package com.example.r47

import android.app.Activity
import android.app.PictureInPictureParams
import android.content.Context
import android.content.Intent
import android.graphics.Color
import android.media.AudioAttributes
import android.media.AudioFormat
import android.media.AudioTrack
import android.media.AudioManager
import android.net.Uri
import android.os.*
import android.util.Log
import android.util.Rational
import android.view.*
import android.widget.Button
import androidx.activity.result.contract.ActivityResultContracts
import androidx.annotation.Keep
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.WindowCompat
import androidx.core.view.WindowInsetsCompat
import androidx.core.view.WindowInsetsControllerCompat
import com.example.r47.databinding.ActivityMainBinding
import java.util.concurrent.LinkedBlockingQueue
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit
import android.content.ClipboardManager
import android.content.ClipData
import android.content.SharedPreferences
import android.content.res.Configuration
import android.provider.DocumentsContract
import androidx.preference.PreferenceManager
import com.google.android.material.dialog.MaterialAlertDialogBuilder

@Keep
class MainActivity : AppCompatActivity(), SharedPreferences.OnSharedPreferenceChangeListener {

    private val TAG = "R47Activity"
    private lateinit var binding: ActivityMainBinding
    private lateinit var replicaOverlay: ReplicaOverlay
    
    private val mainHandler = Handler(Looper.getMainLooper())
    private val lcdPixels = IntArray(400 * 240)
    private var lastTouchX = 0f
    private var lastTouchY = 0f
    private var isHapticEnabled = true
    private var isHighFidelityHapticEnabled = true
    private var hapticIntensity = 180
    private var beeperVolume = 20
    
    private val isNativeBusy = java.util.concurrent.atomic.AtomicBoolean(false)
    private val coreTasks = LinkedBlockingQueue<Runnable>()

    companion object {
        private var isCoreThreadStarted = false
        private var isAppRunningShared = false
        private var isNativeInitialized = false
        private val audioQueue = LinkedBlockingQueue<Long>()
        private var audioThread: Thread? = null
        
        init {
            System.loadLibrary("c47-android")
        }
    }

    private var isMovingToPiP = false

    data class Slot(val id: Int, var name: String, var uri: String?)
    private var slotsList = mutableListOf<Slot>()
    private var currentSlotId = 0
    private var pendingSlotId: Int? = null 

    private var currentSkin = "r47_texture"
    private var lcdMode = "vintage"
    private var scalingMode = "full_width"

    private val VINTAGE_TEXT = 0xFF303030.toInt()
    private val VINTAGE_BG = 0xFFDFF5CC.toInt()
    private val BW_TEXT = Color.BLACK
    private val BW_BG = Color.parseColor("#E0E0E0")
    private val YELLOW_SHIFT = Color.parseColor("#FFBF00")

    private var isBeeperEnabled = true
    private var showTouchZones = false

    private val saveLauncher = registerForActivityResult(ActivityResultContracts.StartActivityForResult()) { result ->
        if (result.resultCode == Activity.RESULT_OK) {
            val uri = result.data?.data
            if (uri != null) {
                try {
                    val pfd = contentResolver.openFileDescriptor(uri, "wt")
                    if (pfd != null) onFileSelectedNative(pfd.detachFd())
                    else onFileCancelledNative()
                } catch (e: Exception) { onFileCancelledNative() }
            } else onFileCancelledNative()
        } else onFileCancelledNative()
    }

    private val loadLauncher = registerForActivityResult(ActivityResultContracts.StartActivityForResult()) { result ->
        if (result.resultCode == Activity.RESULT_OK) {
            val uri = result.data?.data
            if (uri != null) {
                try {
                    val pfd = contentResolver.openFileDescriptor(uri, "r")
                    if (pfd != null) onFileSelectedNative(pfd.detachFd())
                    else onFileCancelledNative()
                } catch (e: Exception) { onFileCancelledNative() }
            } else onFileCancelledNative()
        } else onFileCancelledNative()
    }

    private fun performHapticClick() {
        if (!isHapticEnabled || hapticIntensity <= 0) return
        val vibrator = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            val vibratorManager = getSystemService(Context.VIBRATOR_MANAGER_SERVICE) as? VibratorManager
            vibratorManager?.defaultVibrator
        } else {
            @Suppress("DEPRECATION")
            getSystemService(Context.VIBRATOR_SERVICE) as? Vibrator
        } ?: return

        if (!vibrator.hasVibrator()) return

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val effect = if (isHighFidelityHapticEnabled) {
                // Modern powerful haptics: Custom waveform scaled by intensity
                VibrationEffect.createWaveform(
                    longArrayOf(0, 10, 20, 5),
                    intArrayOf(0, hapticIntensity, 0, hapticIntensity / 2),
                    -1
                )
            } else {
                // Standard mode: Simple one-shot pulse scaled by intensity
                VibrationEffect.createOneShot(15, hapticIntensity)
            }
            vibrator.vibrate(effect)
        } else {
            @Suppress("DEPRECATION")
            vibrator.vibrate(15)
        }
    }

    private fun startAudioThread() {
        if (audioThread != null && audioThread!!.isAlive) return
        audioThread = Thread {
            Log.i(TAG, "Starting Audio Thread (Zero-GC)...")
            val sampleRate = 44100
            val minBufSize = AudioTrack.getMinBufferSize(sampleRate, AudioFormat.CHANNEL_OUT_MONO, AudioFormat.ENCODING_PCM_16BIT)
            val audioTrack = AudioTrack.Builder()
                .setAudioAttributes(AudioAttributes.Builder()
                    .setUsage(AudioAttributes.USAGE_MEDIA)
                    .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
                    .build())
                .setAudioFormat(AudioFormat.Builder()
                    .setEncoding(AudioFormat.ENCODING_PCM_16BIT)
                    .setSampleRate(sampleRate)
                    .setChannelMask(AudioFormat.CHANNEL_OUT_MONO)
                    .build())
                .setBufferSizeInBytes(Math.max(minBufSize, 4096))
                .setTransferMode(AudioTrack.MODE_STREAM)
                .build()
            
            // Reusable buffer to prevent GC allocations
            val bufferSize = 48000 // Approx 1 sec buffer
            val buffer = ShortArray(bufferSize) 
            
            audioTrack.play()
            try {
                while (isAppRunningShared) {
                    val packed = audioQueue.take() // Blocking wait
                    val frequency = (packed shr 32).toInt()
                    val durationMs = (packed and 0xFFFFFFFF).toInt()

                    // Amplitude scaled by beeperVolume (0-100) -> (0-32767)
                    // We use a safe max of ~16384 to avoid heavy distortion on square waves
                    val amplitude: Short = (beeperVolume * 163.84).toInt().toShort()
                    
                    // Add 20ms silence tail for separation
                    val totalSamples = ((durationMs + 20) * sampleRate / 1000)
                    val noteSamples = (durationMs * sampleRate / 1000)
                    
                    // Safety check for buffer size
                    val actualSamples = if (totalSamples > bufferSize) bufferSize else totalSamples
                    
                    // Generate Square Wave with Anti-Pop Envelope
                    val period = if (frequency > 0) sampleRate / frequency else 0
                    
                    if (period > 0) {
                        for (i in 0 until actualSamples) {
                            if (i >= noteSamples) {
                                buffer[i] = 0 // Silence tail
                            } else {
                                var sample = if ((i % period) < (period / 2)) amplitude else (-amplitude).toShort()
                                
                                // Simple 2ms Attack/Decay to remove clicks
                                val rampSamples = 88 // ~2ms at 44.1kHz
                                if (i < rampSamples) {
                                    sample = (sample * i / rampSamples).toShort()
                                } else if (i > noteSamples - rampSamples) {
                                    sample = (sample * (noteSamples - i) / rampSamples).toShort()
                                }
                                buffer[i] = sample
                            }
                        }
                    } else {
                        // Pure silence (rest)
                        for (i in 0 until actualSamples) buffer[i] = 0
                    }
                    
                    audioTrack.write(buffer, 0, actualSamples)
                }
            } catch (e: InterruptedException) {
                // Expected on exit
            } catch (e: Exception) {
                Log.e(TAG, "Audio thread error", e)
            } finally {
                try { audioTrack.stop(); audioTrack.release() } catch (e: Exception) {}
                Log.i(TAG, "Audio Thread stopped.")
            }
        }.apply { priority = Thread.MAX_PRIORITY; start() }
    }

    @Keep
    fun playTone(milliHz: Int, durationMs: Int) {
        if (!isBeeperEnabled || milliHz <= 0) return
        val frequency = Math.max(1, milliHz / 1000)
        // Pack frequency and duration into a single Long to avoid object allocation
        val packed = (frequency.toLong() shl 32) or (durationMs.toLong() and 0xFFFFFFFF)
        audioQueue.offer(packed)
    }

    @Keep
    fun stopTone() {}

    private fun copyXToClipboard() {
        coreTasks.offer {
            try {
                val xVal = getXRegisterNative()
                mainHandler.post {
                    val clipboard = getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager
                    val clip = ClipData.newPlainText("R47 X Register", xVal)
                    clipboard.setPrimaryClip(clip)
                    android.widget.Toast.makeText(this@MainActivity, "Copied: $xVal", android.widget.Toast.LENGTH_SHORT).show()
                }
            } catch (e: Exception) { Log.e(TAG, "Copy error", e) }
        }
    }

    private fun pasteFromClipboard() {
        val clipboard = getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager
        val item = clipboard.primaryClip?.getItemAt(0)
        val text = item?.text?.toString() ?: return
        coreTasks.offer {
            for (char in text) {
                if (char == 'i' || char == 'j') {
                    sendSimFuncNative(if (char == 'i') 1159 else 1160)
                    Thread.sleep(50)
                    continue
                }
                // Correct mappings for R47 6-column grid
                val simIdNum = when (char.lowercaseChar()) {
                    '0' -> 33; '1' -> 28; '2' -> 29; '3' -> 30; '4' -> 23
                    '5' -> 24; '6' -> 25; '7' -> 18; '8' -> 19; '9' -> 20
                    '.', ',' -> 34
                    '-' -> 35 // CHS
                    'e' -> 15 // EEX
                    '+' -> 37 // ENTER
                    else -> null
                }
                if (simIdNum != null) {
                    val simId = String.format(java.util.Locale.US, "%02d", simIdNum)
                    sendSimKeyNative(simId, false, false) // Pressed
                    Thread.sleep(50)
                    sendSimKeyNative(simId, false, true) // Released
                    Thread.sleep(50)
                }
            }
        }
    }

    private fun applyLcdMode(mode: String) {
        lcdMode = mode
        if (mode == "vintage") setLcdColors(VINTAGE_TEXT, VINTAGE_BG)
        else setLcdColors(BW_TEXT.toInt(), BW_BG.toInt())
    }

    private fun applyFullscreenMode(isFullscreen: Boolean) {
        val win = window ?: return
        try {
            WindowCompat.setDecorFitsSystemWindows(win, !isFullscreen)
            val decorView = win.decorView
            WindowInsetsControllerCompat(win, decorView).apply {
                if (isFullscreen) {
                    systemBarsBehavior = WindowInsetsControllerCompat.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
                    hide(WindowInsetsCompat.Type.systemBars())
                    win.addFlags(WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS)
                } else {
                    show(WindowInsetsCompat.Type.systemBars())
                    win.clearFlags(WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS)
                }
            }
        } catch (e: Exception) { Log.e(TAG, "Failed to apply fullscreen mode", e) }
    }

    override fun onSharedPreferenceChanged(prefs: SharedPreferences?, key: String?) {
        if (prefs == null || key == null) return
        when (key) {
            "haptic_enabled" -> isHapticEnabled = prefs.getBoolean(key, true)
            "haptic_hifi_enabled" -> isHighFidelityHapticEnabled = prefs.getBoolean(key, true)
            "haptic_intensity" -> hapticIntensity = prefs.getInt(key, 180)
            "beeper_volume" -> beeperVolume = prefs.getInt(key, 20)
            "keep_screen_on" -> {
                val enabled = prefs.getBoolean(key, false)
                if (enabled) window?.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
                else window?.clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
            }
            "beeper_enabled" -> isBeeperEnabled = prefs.getBoolean(key, true)
            "current_skin" -> {
                currentSkin = prefs.getString(key, "r47_texture") ?: "r47_texture"
                replicaOverlay.setSkin(currentSkin)
            }
            "lcd_mode" -> {
                lcdMode = prefs.getString(key, "vintage") ?: "vintage"
                applyLcdMode(lcdMode)
            }
            "scaling_mode" -> {
                scalingMode = prefs.getString(key, "full_width") ?: "full_width"
                replicaOverlay.setScalingMode(scalingMode)
            }
            "show_touch_zones" -> {
                showTouchZones = prefs.getBoolean(key, false)
                replicaOverlay.setShowTouchZones(showTouchZones)
            }
            "fullscreen_mode" -> {
                applyFullscreenMode(prefs.getBoolean(key, true))
            }
        }
    }

    private var isPhysicalShiftHeld = false
    private var isPhysicalCtrlHeld = false
    private var interceptedWhileHeld = false
    private val activeKeyIdMap = mutableMapOf<Int, Pair<String, Boolean>>()

    private fun getSimIdFromEvent(keyCode: Int, event: KeyEvent?): Pair<String, Boolean>? {
        if (event == null) return null
        if (keyCode in listOf(KeyEvent.KEYCODE_SHIFT_LEFT, KeyEvent.KEYCODE_SHIFT_RIGHT, KeyEvent.KEYCODE_CTRL_LEFT, KeyEvent.KEYCODE_CTRL_RIGHT)) return null

        val unicode = event.getUnicodeChar(event.metaState)
        val char = unicode.toChar()
        val charId = when (char) {
            '+' -> "36"; '-' -> "31"; '*' -> "26"; '/' -> "21"; '.' -> "34"; '=' -> "SEQ_DOTD"
            '0' -> "33"; '1' -> "28"; '2' -> "29"; '3' -> "30"; '4' -> "23"
            '5' -> "24"; '6' -> "25"; '7' -> "18"; '&' -> "SEQ_toI"; '8' -> "19"; '9' -> "20"
            '%' -> "SEQ_PERCENT"; '!' -> "SEQ_FACTORIAL"
            '@' -> "SEQ_DOTD"; '#' -> "SEQ_HASH"; '$' -> "SEQ_MS"; '^' -> "03"
            'q' -> "01"; 'Q' -> "00"
            'w' -> "13"; 'W' -> "SEQ_LASTX"
            'e' -> "15"; 'E' -> "SEQ_ECONST"
            'r' -> "07"; 'R' -> "SEQ_toREC"
            't' -> "SEQ_TAN"; 'T' -> "SEQ_ATAN"
            'y' -> "SEQ_XTHROOT"; 'Y' -> "03"
            'u' -> "SEQ_UNDO"; 'U' -> "SEQ_USER"
            'i' -> "SEQ_IMAG_J"; 'I' -> "SEQ_DISP"
            'o' -> "04"; 'O' -> "SEQ_10X"
            'p' -> "SEQ_PI"; 'P' -> "SEQ_toPOL"
            'a' -> "SEQ_SIGMAP"; 'A' -> "SEQ_ANGLE"
            's' -> "SEQ_SIN"; 'S' -> "SEQ_ASIN"
            'd' -> "08"; 'D' -> "SEQ_RUP"
            'f' -> "10"; 'F' -> "SEQ_PREFIX"
            'g' -> "11"; 'G' -> "SEQ_GTO"
            'h' -> null; 'H' -> "SEQ_HOME"
            'j' -> "SEQ_IMAG_J"; 'J' -> "SEQ_EXP"
            'k' -> "SEQ_IMAG_POL"; 'K' -> "SEQ_STK"
            'l' -> "05"; 'L' -> "SEQ_EXP_E"
            'z' -> "35"; 'Z' -> "SEQ_ABS"
            'x' -> "17"; 'X' -> "SEQ_COMPLEX"
            'c' -> "SEQ_COS"; 'C' -> "SEQ_ACOS"
            'v' -> "02"; 'V' -> "02"
            'b' -> "SEQ_LBL"; 'B' -> "SEQ_MYMENU"
            'n' -> "14"; 'N' -> "SEQ_PRGM"
            'm' -> "06"; 'M' -> "SEQ_PREF"
            ',' -> "34"; '<' -> "SEQ_RTN"
            '>' -> "SEQ_DRG"
            ':' -> "SEQ_TGLFRT"; '\'' -> "SEQ_ALPHA"
            '"' -> "SEQ_HASH"; '\\' -> "35"; '|' -> "SEQ_ABS"
            else -> null
        }
        if (charId != null) return Pair(charId, false)

        return when (keyCode) {
            KeyEvent.KEYCODE_ENTER, KeyEvent.KEYCODE_NUMPAD_ENTER -> Pair("12", false)
            KeyEvent.KEYCODE_ESCAPE -> Pair("32", false)
            KeyEvent.KEYCODE_DEL, KeyEvent.KEYCODE_FORWARD_DEL -> Pair("16", false)
            KeyEvent.KEYCODE_TAB -> Pair("13", false)
            KeyEvent.KEYCODE_DPAD_UP -> Pair("22", false)
            KeyEvent.KEYCODE_DPAD_DOWN -> Pair("27", false)
            KeyEvent.KEYCODE_F1 -> Pair("1", true); KeyEvent.KEYCODE_F2 -> Pair("2", true)
            KeyEvent.KEYCODE_F3 -> Pair("3", true); KeyEvent.KEYCODE_F4 -> Pair("4", true)
            KeyEvent.KEYCODE_F5 -> Pair("5", true); KeyEvent.KEYCODE_F6 -> Pair("6", true)
            KeyEvent.KEYCODE_F7 -> Pair("SEQ_SI_n", false); KeyEvent.KEYCODE_F8 -> Pair("SEQ_SI_u", false)
            KeyEvent.KEYCODE_F9 -> Pair("SEQ_SI_m", false); KeyEvent.KEYCODE_F10 -> Pair("SEQ_SI_k", false)
            KeyEvent.KEYCODE_F11 -> Pair("SEQ_SI_M", false)
            KeyEvent.KEYCODE_NUMPAD_ADD -> Pair("36", false)
            KeyEvent.KEYCODE_NUMPAD_SUBTRACT -> Pair("31", false)
            KeyEvent.KEYCODE_NUMPAD_MULTIPLY -> Pair("26", false)
            KeyEvent.KEYCODE_NUMPAD_DIVIDE -> Pair("21", false)
            else -> null
        }
    }

    override fun onKeyDown(keyCode: Int, event: KeyEvent?): Boolean {
        if (keyCode == KeyEvent.KEYCODE_VOLUME_UP || keyCode == KeyEvent.KEYCODE_VOLUME_DOWN) return false
        if (keyCode == KeyEvent.KEYCODE_SHIFT_LEFT || keyCode == KeyEvent.KEYCODE_SHIFT_RIGHT) {
            isPhysicalShiftHeld = true; interceptedWhileHeld = false; return true
        }
        if (keyCode == KeyEvent.KEYCODE_CTRL_LEFT || keyCode == KeyEvent.KEYCODE_CTRL_RIGHT) {
            isPhysicalCtrlHeld = true; interceptedWhileHeld = false; return true
        }
        if (isPhysicalShiftHeld || isPhysicalCtrlHeld) interceptedWhileHeld = true
        if (event?.repeatCount ?: 0 > 0) return true

        getSimIdFromEvent(keyCode, event)?.let { (id, isFn) ->
            activeKeyIdMap[keyCode] = id to isFn
            when (id) {
                "SEQ_PERCENT" -> coreTasks.offer {
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true)
                    Thread.sleep(50); sendSimKeyNative("07", false, false); Thread.sleep(50); sendSimKeyNative("07", false, true)
                }
                "SEQ_FACTORIAL" -> coreTasks.offer {
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true)
                    Thread.sleep(50); sendSimKeyNative("02", false, false); Thread.sleep(50); sendSimKeyNative("02", false, true)
                }
                "SEQ_DOTD" -> coreTasks.offer {
                    sendSimKeyNative("11", false, false); Thread.sleep(50); sendSimKeyNative("11", false, true)
                    Thread.sleep(50); sendSimKeyNative("03", false, false); Thread.sleep(50); sendSimKeyNative("03", false, true)
                }
                "SEQ_HASH" -> coreTasks.offer {
                    sendSimKeyNative("11", false, false); Thread.sleep(50); sendSimKeyNative("11", false, true)
                    Thread.sleep(50); sendSimKeyNative("05", false, false); Thread.sleep(50); sendSimKeyNative("05", false, true)
                }
                "SEQ_MS" -> coreTasks.offer {
                    sendSimKeyNative("11", false, false); Thread.sleep(50); sendSimKeyNative("11", false, true)
                    Thread.sleep(50); sendSimKeyNative("02", false, false); Thread.sleep(50); sendSimKeyNative("02", false, true)
                }
                "SEQ_LASTX" -> coreTasks.offer {
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true)
                    Thread.sleep(50); sendSimKeyNative("13", false, false); Thread.sleep(50); sendSimKeyNative("13", false, true)
                }
                "SEQ_ECONST" -> coreTasks.offer {
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true); Thread.sleep(50)
                    sendSimKeyNative("36", false, false); Thread.sleep(50); sendSimKeyNative("36", false, true); Thread.sleep(100)
                    sendSimKeyNative("2", true, false); Thread.sleep(50); sendSimKeyNative("2", true, true); Thread.sleep(100)
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true); Thread.sleep(50)
                    sendSimKeyNative("2", true, false); Thread.sleep(50); sendSimKeyNative("2", true, true)
                }
                "SEQ_toREC" -> coreTasks.offer {
                    sendSimKeyNative("11", false, false); Thread.sleep(50); sendSimKeyNative("11", false, true)
                    Thread.sleep(50); sendSimKeyNative("00", false, false); Thread.sleep(50); sendSimKeyNative("00", false, true)
                }
                "SEQ_TAN" -> coreTasks.offer {
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true)
                    Thread.sleep(50); sendSimKeyNative("20", false, false); Thread.sleep(50); sendSimKeyNative("20", false, true)
                }
                "SEQ_ATAN" -> coreTasks.offer {
                    sendSimKeyNative("11", false, false); Thread.sleep(50); sendSimKeyNative("11", false, true)
                    Thread.sleep(50); sendSimKeyNative("20", false, false); Thread.sleep(50); sendSimKeyNative("20", false, true)
                }
                "SEQ_XTHROOT" -> coreTasks.offer {
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true)
                    Thread.sleep(50); sendSimKeyNative("03", false, false); Thread.sleep(50); sendSimKeyNative("03", false, true)
                }
                "SEQ_UNDO" -> coreTasks.offer {
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true)
                    Thread.sleep(50); sendSimKeyNative("16", false, false); Thread.sleep(50); sendSimKeyNative("16", false, true)
                }
                "SEQ_USER" -> coreTasks.offer {
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true)
                    Thread.sleep(50); sendSimKeyNative("09", false, false); Thread.sleep(50); sendSimKeyNative("09", false, true)
                }
                "SEQ_IMAG_J" -> coreTasks.offer {
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true)
                    Thread.sleep(50); sendSimKeyNative("00", false, false); Thread.sleep(50); sendSimKeyNative("00", false, true)
                }
                "SEQ_DISP" -> coreTasks.offer {
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true)
                    Thread.sleep(50); sendSimKeyNative("14", false, false); Thread.sleep(50); sendSimKeyNative("14", false, true)
                }
                "SEQ_10X" -> coreTasks.offer {
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true)
                    Thread.sleep(50); sendSimKeyNative("04", false, false); Thread.sleep(50); sendSimKeyNative("04", false, true)
                }
                "SEQ_PI" -> coreTasks.offer {
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true)
                    Thread.sleep(50); sendSimKeyNative("08", false, false); Thread.sleep(50); sendSimKeyNative("08", false, true)
                }
                "SEQ_toI" -> coreTasks.offer {
                    sendSimKeyNative("11", false, false); Thread.sleep(50); sendSimKeyNative("11", false, true)
                    Thread.sleep(50); sendSimKeyNative("04", false, false); Thread.sleep(50); sendSimKeyNative("04", false, true)
                }
                "SEQ_HOME" -> coreTasks.offer { sendSimMenuNative(-1921) }
                "SEQ_MYMENU" -> coreTasks.offer { sendSimMenuNative(-1349) }
                "SEQ_toPOL" -> coreTasks.offer {
                    sendSimKeyNative("11", false, false); Thread.sleep(50); sendSimKeyNative("11", false, true)
                    Thread.sleep(50); sendSimKeyNative("01", false, false); Thread.sleep(50); sendSimKeyNative("01", false, true)
                }
                "SEQ_IMAG_POL" -> coreTasks.offer {
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true)
                    Thread.sleep(50); sendSimKeyNative("01", false, false); Thread.sleep(50); sendSimKeyNative("01", false, true)
                }
                "SEQ_ALPHA" -> coreTasks.offer {
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true)
                    Thread.sleep(50); sendSimKeyNative("17", false, false); Thread.sleep(50); sendSimKeyNative("17", false, true)
                }
                "SEQ_SIGMAP" -> coreTasks.offer {
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true); Thread.sleep(50)
                    sendSimKeyNative("21", false, false); Thread.sleep(50); sendSimKeyNative("21", false, true); Thread.sleep(100)
                    sendSimKeyNative("1", true, false); Thread.sleep(50); sendSimKeyNative("1", true, true)
                }
                "SEQ_ANGLE" -> coreTasks.offer {
                    sendSimKeyNative("11", false, false); Thread.sleep(50); sendSimKeyNative("11", false, true)
                    Thread.sleep(50); sendSimKeyNative("06", false, false); Thread.sleep(50); sendSimKeyNative("06", false, true)
                }
                "SEQ_SIN" -> coreTasks.offer {
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true)
                    Thread.sleep(50); sendSimKeyNative("18", false, false); Thread.sleep(50); sendSimKeyNative("18", false, true)
                }
                "SEQ_ASIN" -> coreTasks.offer {
                    sendSimKeyNative("11", false, false); Thread.sleep(50); sendSimKeyNative("11", false, true)
                    Thread.sleep(50); sendSimKeyNative("18", false, false); Thread.sleep(50); sendSimKeyNative("18", false, true)
                }
                "SEQ_RUP" -> coreTasks.offer {
                    sendSimKeyNative("11", false, false); Thread.sleep(50); sendSimKeyNative("11", false, true)
                    Thread.sleep(50); sendSimKeyNative("08", false, false); Thread.sleep(50); sendSimKeyNative("08", false, true)
                }
                "SEQ_PREFIX" -> coreTasks.offer {
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true)
                    Thread.sleep(50); sendSimKeyNative("15", false, false); Thread.sleep(50); sendSimKeyNative("15", false, true)
                }
                "SEQ_GTO" -> coreTasks.offer {
                    sendSimKeyNative("11", false, false); Thread.sleep(50); sendSimKeyNative("11", false, true)
                    Thread.sleep(50); sendSimKeyNative("17", false, false); Thread.sleep(50); sendSimKeyNative("17", false, true)
                }
                "SEQ_EXP" -> coreTasks.offer {
                    sendSimKeyNative("11", false, false); Thread.sleep(50); sendSimKeyNative("11", false, true)
                    Thread.sleep(50); sendSimKeyNative("15", false, false); Thread.sleep(50); sendSimKeyNative("15", false, true)
                }
                "SEQ_COMPLEX" -> coreTasks.offer {
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true)
                    Thread.sleep(50); sendSimKeyNative("12", false, false); Thread.sleep(50); sendSimKeyNative("12", false, true)
                }
                "SEQ_STK" -> coreTasks.offer {
                    sendSimKeyNative("11", false, false); Thread.sleep(50); sendSimKeyNative("11", false, true)
                    Thread.sleep(50); sendSimKeyNative("13", false, false); Thread.sleep(50); sendSimKeyNative("13", false, true)
                }
                "SEQ_EXP_E" -> coreTasks.offer {
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true)
                    Thread.sleep(50); sendSimKeyNative("05", false, false); Thread.sleep(50); sendSimKeyNative("05", false, true)
                }
                "SEQ_COS" -> coreTasks.offer {
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true)
                    Thread.sleep(50); sendSimKeyNative("19", false, false); Thread.sleep(50); sendSimKeyNative("19", false, true)
                }
                "SEQ_ACOS" -> coreTasks.offer {
                    sendSimKeyNative("11", false, false); Thread.sleep(50); sendSimKeyNative("11", false, true)
                    Thread.sleep(50); sendSimKeyNative("19", false, false); Thread.sleep(50); sendSimKeyNative("19", false, true)
                }
                "SEQ_LBL" -> coreTasks.offer {
                    sendSimKeyNative("11", false, false); Thread.sleep(50); sendSimKeyNative("11", false, true)
                    Thread.sleep(50); sendSimKeyNative("05", false, false); Thread.sleep(50); sendSimKeyNative("05", false, true)
                }
                "SEQ_PRGM" -> coreTasks.offer {
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true)
                    Thread.sleep(50); sendSimKeyNative("35", false, false); Thread.sleep(50); sendSimKeyNative("35", false, true)
                }
                "SEQ_PREF" -> coreTasks.offer {
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true)
                    Thread.sleep(50); sendSimKeyNative("28", false, false); Thread.sleep(50); sendSimKeyNative("28", false, true)
                }
                "SEQ_RTN" -> coreTasks.offer {
                    sendSimKeyNative("11", false, false); Thread.sleep(50); sendSimKeyNative("11", false, true)
                    Thread.sleep(50); sendSimKeyNative("35", false, false); Thread.sleep(50); sendSimKeyNative("35", false, true)
                }
                "SEQ_DRG" -> coreTasks.offer { sendSimKeyNative("09", false, false) }
                "SEQ_SI_n" -> coreTasks.offer {
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true); Thread.sleep(50)
                    sendSimKeyNative("15", false, false); Thread.sleep(50); sendSimKeyNative("15", false, true); Thread.sleep(100)
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true); Thread.sleep(50)
                    sendSimKeyNative("3", true, false); Thread.sleep(50); sendSimKeyNative("3", true, true)
                }
                "SEQ_SI_u" -> coreTasks.offer {
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true); Thread.sleep(50)
                    sendSimKeyNative("15", false, false); Thread.sleep(50); sendSimKeyNative("15", false, true); Thread.sleep(100)
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true); Thread.sleep(50)
                    sendSimKeyNative("2", true, false); Thread.sleep(50); sendSimKeyNative("2", true, true)
                }
                "SEQ_SI_m" -> coreTasks.offer {
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true); Thread.sleep(50)
                    sendSimKeyNative("15", false, false); Thread.sleep(50); sendSimKeyNative("15", false, true); Thread.sleep(100)
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true); Thread.sleep(50)
                    sendSimKeyNative("1", true, false); Thread.sleep(50); sendSimKeyNative("1", true, true)
                }
                "SEQ_SI_k" -> coreTasks.offer {
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true); Thread.sleep(50)
                    sendSimKeyNative("15", false, false); Thread.sleep(50); sendSimKeyNative("15", false, true); Thread.sleep(100)
                    sendSimKeyNative("1", true, false); Thread.sleep(50); sendSimKeyNative("1", true, true)
                }
                "SEQ_SI_M" -> coreTasks.offer {
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true); Thread.sleep(50)
                    sendSimKeyNative("15", false, false); Thread.sleep(50); sendSimKeyNative("15", false, true); Thread.sleep(100)
                    sendSimKeyNative("2", true, false); Thread.sleep(50); sendSimKeyNative("2", true, true)
                }
                "SEQ_TGLFRT" -> coreTasks.offer {
                    sendSimKeyNative("11", false, false); Thread.sleep(50); sendSimKeyNative("11", false, true)
                    Thread.sleep(50); sendSimKeyNative("34", false, false); Thread.sleep(50); sendSimKeyNative("34", false, true)
                }
                "SEQ_AIM" -> coreTasks.offer {
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true)
                    Thread.sleep(50); sendSimKeyNative("17", false, false); Thread.sleep(50); sendSimKeyNative("17", false, true)
                }
                "SEQ_ABS" -> coreTasks.offer {
                    sendSimKeyNative("10", false, false); Thread.sleep(50); sendSimKeyNative("10", false, true)
                    Thread.sleep(50); sendSimKeyNative("06", false, false); Thread.sleep(50); sendSimKeyNative("06", false, true)
                }
                else -> coreTasks.offer { sendSimKeyNative(id, isFn, false) }
            }
            return true
        }
        return super.onKeyDown(keyCode, event)
    }

    override fun onKeyUp(keyCode: Int, event: KeyEvent?): Boolean {
        if (keyCode == KeyEvent.KEYCODE_VOLUME_UP || keyCode == KeyEvent.KEYCODE_VOLUME_DOWN) return false
        if (keyCode == KeyEvent.KEYCODE_SHIFT_LEFT || keyCode == KeyEvent.KEYCODE_SHIFT_RIGHT) {
            if (!interceptedWhileHeld) { sendSimKeyNative("10", false, false); sendSimKeyNative("10", false, true) }
            isPhysicalShiftHeld = false; return true
        }
        if (keyCode == KeyEvent.KEYCODE_CTRL_LEFT || keyCode == KeyEvent.KEYCODE_CTRL_RIGHT) {
            if (!interceptedWhileHeld) { sendSimKeyNative("11", false, false); sendSimKeyNative("11", false, true) }
            isPhysicalCtrlHeld = false; return true
        }
        activeKeyIdMap.remove(keyCode)?.let { (id, isFn) ->
            if (!id.startsWith("SEQ_")) coreTasks.offer { sendSimKeyNative(id, isFn, true) }
            return true
        }
        return super.onKeyUp(keyCode, event)
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        volumeControlStream = AudioManager.STREAM_MUSIC
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        
        val prefs = getSharedPreferences("R47Prefs", MODE_PRIVATE)
        prefs.registerOnSharedPreferenceChangeListener(this)
        
        window.attributes.layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES
        val isFullscreen = prefs.getBoolean("fullscreen_mode", true)
        applyFullscreenMode(isFullscreen)

        replicaOverlay = binding.replicaOverlay
        replicaOverlay.onPiPKeyEvent = { code ->
            coreTasks.offer { sendKey(code) }
        }
        setupInteractiveZones()
        loadSlots()
        
        isHapticEnabled = prefs.getBoolean("haptic_enabled", true)
        isHighFidelityHapticEnabled = prefs.getBoolean("haptic_hifi_enabled", true)
        hapticIntensity = prefs.getInt("haptic_intensity", 180)
        beeperVolume = prefs.getInt("beeper_volume", 20)
        currentSlotId = prefs.getInt("current_slot_id", 0)
        currentSkin = prefs.getString("current_skin", "r47_texture") ?: "r47_texture"
        lcdMode = prefs.getString("lcd_mode", "vintage") ?: "vintage"
        scalingMode = prefs.getString("scaling_mode", "full_width") ?: "full_width"
        isBeeperEnabled = prefs.getBoolean("beeper_enabled", true)
        showTouchZones = prefs.getBoolean("show_touch_zones", false)
        if (prefs.getBoolean("keep_screen_on", false)) window?.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        
        replicaOverlay.post {
            replicaOverlay.setSkin(currentSkin)
            replicaOverlay.setShowTouchZones(showTouchZones)
            replicaOverlay.setScalingMode(scalingMode)
            applyLcdMode(lcdMode)
        }

        replicaOverlay.onLongPressListener = { x, y ->
            val scale = replicaOverlay.width.toFloat() / 537f
            val offX = (replicaOverlay.width - 537f * scale) / 2f
            val offY = (replicaOverlay.height - 1005f * scale) / 2f
            val lX = (x - offX) / scale
            val lY = (y - offY) / scale
            if (lX in 25.5f..511.5f && lY in 67.5f..334.2f) {
                MaterialAlertDialogBuilder(this@MainActivity)
                    .setTitle("Display & Clipboard")
                    .setItems(arrayOf("Copy X Register", "Paste Number", "Enter PiP Mode")) { _, which ->
                        when (which) {
                            0 -> copyXToClipboard()
                            1 -> pasteFromClipboard()
                            2 -> enterPiP()
                        }
                    }.setNegativeButton("Cancel", null)
                    .show()
            }
        }
        
        replicaOverlay.onSettingsTapListener = {
            Log.i(TAG, "Settings tap received in MainActivity")
            startActivity(Intent(this, SettingsActivity::class.java))
        }
        
        isAppRunningShared = true
        startAudioThread()
        
        // PERSISTENT THREAD RESTART LOGIC
        if (!isCoreThreadStarted) {
            isCoreThreadStarted = true
            Thread {
                try {
                    Log.i(TAG, "Core Thread Starting... isNativeInitialized=$isNativeInitialized")
                    if (!isNativeInitialized) {
                        nativePreInit(filesDir.absolutePath)
                        initNative(filesDir.absolutePath, currentSlotId)
                        isNativeInitialized = true
                    } else {
                        updateNativeActivityRef()
                    }
                    var lastTickLog = 0L
                    while (isAppRunningShared) {
                        val now = System.currentTimeMillis()
                        if (now - lastTickLog > 5000) {
                            Log.i(TAG, "Core Thread Heartbeat: Loop active")
                            lastTickLog = now
                        }

                        // 1. Process tasks from coreTasks (Consolidated mechanism)
                        var task = coreTasks.poll()
                        while (task != null) {
                            try { task.run() } catch (e: Exception) { Log.e(TAG, "Task failed", e) }
                            task = coreTasks.poll()
                        }

                        tick()
                        Thread.sleep(10)
                    }
                    Log.i(TAG, "Core Thread exiting (isAppRunningShared is false)")
                } catch (e: Exception) { 
                    Log.e(TAG, "Native core thread crashed", e) 
                } finally {
                    isCoreThreadStarted = false
                }
            }.start()
        } else {
            Log.i(TAG, "Core Thread already running, updating Activity ref.")
            updateNativeActivityRef()
        }
        
        Choreographer.getInstance().postFrameCallback(object : Choreographer.FrameCallback {
            override fun doFrame(frameTimeNanos: Long) {
                if (isAppRunningShared) {
                    if (isNativeInitialized) { 
                        getDisplayPixels(lcdPixels)
                        if (lcdPixels.isNotEmpty()) {
                            replicaOverlay.updateLcd(lcdPixels)
                        }
                    }
                    Choreographer.getInstance().postFrameCallback(this)
                }
            }
        })
    }

    private external fun updateNativeActivityRef()

    override fun onDestroy() { 
        Log.i(TAG, "onDestroy: isFinishing=$isFinishing")
        if (isFinishing) {
            isAppRunningShared = false
        }
        getSharedPreferences("R47Prefs", MODE_PRIVATE).unregisterOnSharedPreferenceChangeListener(this)
        super.onDestroy() 
    }

    override fun onResume() {
        super.onResume()
        if (isNativeInitialized) {
            coreTasks.offer {
                forceRefreshNative()
            }
        }
        
        val prefs = getSharedPreferences("R47Prefs", MODE_PRIVATE)
        val isFirstRun = prefs.getBoolean("first_run_setup", true)
        val hasDir = prefs.getString("work_directory_uri", null) != null

        if (isFirstRun && !hasDir) {
            showFirstRunDialog()
        } else {
            validateWorkDirectory()
        }
    }

    private fun showFirstRunDialog() {
        MaterialAlertDialogBuilder(this)
            .setTitle("Welcome to R47")
            .setMessage("To get started, please select a 'Work Directory'.\n\nThis folder will be used to organize your Programs, State files, and Screenshots safely on your device storage.")
            .setPositiveButton("Select Folder") { _, _ ->
                getSharedPreferences("R47Prefs", MODE_PRIVATE).edit().putBoolean("first_run_setup", false).apply()
                startActivity(Intent(this, SettingsActivity::class.java).apply {
                    putExtra("trigger_work_dir_picker", true)
                })
            }
            .setNegativeButton("Later") { _, _ ->
                getSharedPreferences("R47Prefs", MODE_PRIVATE).edit().putBoolean("first_run_setup", false).apply()
                validateWorkDirectory()
            }
            .setCancelable(false)
            .show()
    }

    private fun validateWorkDirectory() {
        val prefs = getSharedPreferences("R47Prefs", MODE_PRIVATE)
        val treeUriStr = prefs.getString("work_directory_uri", null)
        
        if (treeUriStr == null) {
            showWorkDirMissingSnackbar("Work Directory not set")
            return
        }

        val treeUri = Uri.parse(treeUriStr)
        var isAccessible = false
        try {
            // Check if we still have persisted permissions
            val persistedPerms = contentResolver.persistedUriPermissions
            val hasPerm = persistedPerms.any { it.uri == treeUri && it.isWritePermission }
            
            if (hasPerm) {
                // Try to actually query the directory to ensure it wasn't deleted
                val documentId = DocumentsContract.getTreeDocumentId(treeUri)
                val childrenUri = DocumentsContract.buildChildDocumentsUriUsingTree(treeUri, documentId)
                contentResolver.query(childrenUri, arrayOf(DocumentsContract.Document.COLUMN_DOCUMENT_ID), null, null, null)?.use {
                    isAccessible = true 
                }
            }
        } catch (e: Exception) {
            Log.w(TAG, "Work directory validation failed: ${e.message}")
        }

        if (!isAccessible) {
            showWorkDirMissingSnackbar("Work Directory is no longer accessible")
        }
    }

    private fun showWorkDirMissingSnackbar(message: String) {
        com.google.android.material.snackbar.Snackbar.make(
            binding.root,
            message,
            com.google.android.material.snackbar.Snackbar.LENGTH_INDEFINITE
        ).setAction("SET") {
            startActivity(Intent(this, SettingsActivity::class.java).apply {
                putExtra("trigger_work_dir_picker", true)
            })
        }.show()
    }

    override fun onPause() {
        super.onPause()
        val isEnteringPiP = isMovingToPiP || (if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) isInPictureInPictureMode else false)
        Log.i(TAG, "onPause: isEnteringPiP=$isEnteringPiP isMovingToPiP=$isMovingToPiP")
        
        if (!isEnteringPiP && getSharedPreferences("R47Prefs", MODE_PRIVATE).getBoolean("auto_save_minimize", true)) {
            Log.i(TAG, "Auto-saving state on pause (synchronous via core thread)...")
            val latch = CountDownLatch(1)
            coreTasks.offer {
                try {
                    saveStateNative()
                } finally {
                    latch.countDown()
                }
            }
            try {
                // Wait up to 2 seconds for the core thread to finish pending keys and save
                if (!latch.await(2, TimeUnit.SECONDS)) {
                    Log.w(TAG, "Timed out waiting for state save on pause")
                }
            } catch (e: InterruptedException) {
                Log.e(TAG, "Interrupted while waiting for state save", e)
            }
        }
    }

    private fun loadSlots() {
        val prefs = getSharedPreferences("R47Slots", MODE_PRIVATE)
        val count = prefs.getInt("slot_count", 0)
        slotsList.clear()
        if (count == 0) {
            slotsList.add(Slot(0, "Slot 1 (Standard)", null))
            saveSlots()
        } else {
            for (i in 0 until count) {
                val name = prefs.getString("slot_${i}_name", "Slot ${i+1}") ?: "Slot ${i+1}"
                val uri = prefs.getString("slot_${i}_uri", null)
                slotsList.add(Slot(i, name, uri))
            }
        }
        currentSlotId = getSharedPreferences("R47Prefs", MODE_PRIVATE).getInt("current_slot_id", 0)
    }

    private fun saveSlots() {
        val prefs = getSharedPreferences("R47Slots", MODE_PRIVATE).edit()
        prefs.putInt("slot_count", slotsList.size)
        for (i in slotsList.indices) {
            prefs.putString("slot_${i}_name", slotsList[i].name)
            prefs.putString("slot_${i}_uri", slotsList[i].uri)
        }
        prefs.apply()
    }

    private val slotCreateLauncher = registerForActivityResult(ActivityResultContracts.StartActivityForResult()) { result ->
        if (result.resultCode == Activity.RESULT_OK) {
            result.data?.data?.let { uri ->
                contentResolver.takePersistableUriPermission(uri, Intent.FLAG_GRANT_READ_URI_PERMISSION or Intent.FLAG_GRANT_WRITE_URI_PERMISSION)
                val id = pendingSlotId ?: return@let
                slotsList[id].uri = uri.toString()
                saveSlots()
                currentSlotId = id
                setSlotNative(id)
                getSharedPreferences("R47Prefs", MODE_PRIVATE).edit().putInt("current_slot_id", id).apply()
                coreTasks.offer {
                    saveStateNative()
                }
                android.widget.Toast.makeText(this, "Created ${slotsList[id].name}", android.widget.Toast.LENGTH_SHORT).show()
            }
        }
    }

    private val slotLoadLauncher = registerForActivityResult(ActivityResultContracts.StartActivityForResult()) { result ->
        if (result.resultCode == Activity.RESULT_OK) {
            result.data?.data?.let { uri ->
                contentResolver.takePersistableUriPermission(uri, Intent.FLAG_GRANT_READ_URI_PERMISSION or Intent.FLAG_GRANT_WRITE_URI_PERMISSION)
                val id = pendingSlotId ?: return@let
                slotsList[id].uri = uri.toString()
                saveSlots()
                currentSlotId = id
                setSlotNative(id)
                getSharedPreferences("R47Prefs", MODE_PRIVATE).edit().putInt("current_slot_id", id).apply()
                coreTasks.offer {
                    loadStateNative()
                }
                android.widget.Toast.makeText(this, "Loaded ${slotsList[id].name}", android.widget.Toast.LENGTH_SHORT).show()
            }
        }
    }

    private fun switchSlot(id: Int) {
        if (id !in slotsList.indices || id == currentSlotId) return
        
        val targetName = slotsList[id].name
        coreTasks.offer {
            try {
                saveStateNative()
                currentSlotId = id
                setSlotNative(id)
                loadStateNative()
                mainHandler.post {
                    getSharedPreferences("R47Prefs", MODE_PRIVATE).edit().putInt("current_slot_id", id).apply()
                    android.widget.Toast.makeText(this, "Switched to $targetName", android.widget.Toast.LENGTH_SHORT).show()
                }
            } catch (e: Exception) {
                Log.e(TAG, "Slot switch failed", e)
            }
        }
    }

    @Keep
    fun requestFile(isSave: Boolean, defaultName: String, fileType: Int) {
        mainHandler.post {
            try {
                val initialUri = getWorkDirectorySubfolder(fileType)
                if (isSave) {
                    val intent = Intent(Intent.ACTION_CREATE_DOCUMENT).apply {
                        addCategory(Intent.CATEGORY_OPENABLE)
                        type = when {
                            defaultName.endsWith(".bmp") -> "image/bmp"
                            defaultName.endsWith(".rtf") -> "application/rtf"
                            defaultName.endsWith(".s47") -> "application/octet-stream"
                            defaultName.endsWith(".p47") -> "application/octet-stream"
                            defaultName.endsWith(".sav") -> "application/octet-stream"
                            else -> "*/*"
                        }
                        putExtra(Intent.EXTRA_TITLE, defaultName)
                        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O && initialUri != null) {
                            putExtra(DocumentsContract.EXTRA_INITIAL_URI, initialUri)
                        }
                    }
                    saveLauncher.launch(intent)
                } else {
                    val intent = Intent(Intent.ACTION_OPEN_DOCUMENT).apply {
                        addCategory(Intent.CATEGORY_OPENABLE)
                        type = "*/*"
                        val mimeTypes = arrayOf("application/octet-stream", "text/plain")
                        putExtra(Intent.EXTRA_MIME_TYPES, mimeTypes)
                        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O && initialUri != null) {
                            putExtra(DocumentsContract.EXTRA_INITIAL_URI, initialUri)
                        }
                    }
                    loadLauncher.launch(intent)
                }
            } catch (e: Exception) { 
                Log.e(TAG, "Failed to launch SAF", e)
                onFileCancelledNative() 
            }
        }
    }

    @Keep
    fun quitApp() {
        Log.i(TAG, "quitApp called from native")
        mainHandler.post { 
            val forceClose = getSharedPreferences("R47Prefs", MODE_PRIVATE).getBoolean("force_close_on_exit", false)
            if (forceClose) {
                finishAndRemoveTask()
            } else {
                moveTaskToBack(true)
            }
        }
    }

    @Keep
    fun processCoreTasks() {
        var task = coreTasks.poll()
        while (task != null) {
            try { task.run() } catch (e: Exception) { Log.e(TAG, "Task failed", e) }
            task = coreTasks.poll()
        }
    }

    private fun getWorkDirectorySubfolder(fileType: Int): Uri? {
        val prefs = getSharedPreferences("R47Prefs", MODE_PRIVATE)
        val treeUriStr = prefs.getString("work_directory_uri", null) ?: return null
        val treeUri = Uri.parse(treeUriStr)
        
        val subfolderName = when (fileType) {
            0 -> "STATE"
            1 -> "PROGRAMS"
            2 -> "SAVFILES"
            3 -> "SCREENS"
            else -> null
        } ?: return treeUri

        try {
            val rootDocId = DocumentsContract.getTreeDocumentId(treeUri)
            val resolver = contentResolver
            
            // Check if subfolder exists
            val childrenUri = DocumentsContract.buildChildDocumentsUriUsingTree(treeUri, rootDocId)
            var folderUri: Uri? = null
            
            resolver.query(childrenUri, arrayOf(DocumentsContract.Document.COLUMN_DISPLAY_NAME, DocumentsContract.Document.COLUMN_DOCUMENT_ID), null, null, null)?.use { cursor ->
                while (cursor.moveToNext()) {
                    if (cursor.getString(0) == subfolderName) {
                        folderUri = DocumentsContract.buildDocumentUriUsingTree(treeUri, cursor.getString(1))
                        break
                    }
                }
            }

            if (folderUri == null) {
                // Create subfolder
                folderUri = DocumentsContract.createDocument(resolver, DocumentsContract.buildDocumentUriUsingTree(treeUri, rootDocId), DocumentsContract.Document.MIME_TYPE_DIR, subfolderName)
            }
            return folderUri
        } catch (e: Exception) {
            Log.e(TAG, "Error resolving subfolder $subfolderName", e)
            return treeUri
        }
    }

    private fun enterPiP() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            isMovingToPiP = true
            val params = PictureInPictureParams.Builder()
                .setAspectRatio(Rational(4860, 2667)) 
                .build()
            enterPictureInPictureMode(params)
        }
    }

    override fun onPictureInPictureModeChanged(isInPictureInPictureMode: Boolean, newConfig: Configuration) {
        super.onPictureInPictureModeChanged(isInPictureInPictureMode, newConfig)
        Log.i(TAG, "onPictureInPictureModeChanged: isInPictureInPictureMode=$isInPictureInPictureMode")
        isMovingToPiP = false
        mainHandler.post {
            Log.i(TAG, "Updating overlay for pipMode=$isInPictureInPictureMode")
            replicaOverlay.setPiPMode(isInPictureInPictureMode)
        }
    }

    private fun setupInteractiveZones() {
        val X_LEFT = 22.5f; val Y_TOP = 339.5f; val DY = 82f; val TOTAL_GRID_W = 492f 
        val rowSpecs = arrayOf(
            intArrayOf(38, 6, 6),
            intArrayOf(1,  6, 6),
            intArrayOf(7,  6, 6),
            intArrayOf(13, 5, 6),
            intArrayOf(18, 5, 5),
            intArrayOf(23, 5, 5),
            intArrayOf(28, 5, 5),
            intArrayOf(33, 5, 5)
        )
        for (r in 0 until 8) {
            val spec = rowSpecs[r]; val codeStart = spec[0]; val count = spec[1]; val gridCols = spec[2]; val dx = TOTAL_GRID_W / gridCols
            var y = Y_TOP + (r * DY); var kH = DY - 10
            when(r) { 0 -> { y += 27f; kH += 5f }; 1 -> y += 22f; 2 -> y += 12f; 3 -> y += 10f; 4 -> { y += 10f; kH -= 10f }; 7 -> y -= 10f }
            for (c in 0 until count) {
                val code = codeStart + c; var actualX = X_LEFT + (c * dx); var actualW = dx * 0.90f
                if (r < 3) {
                    val shift = 8f
                    when (c) { 0 -> actualW += shift; 1 -> { actualX += shift }; 2 -> { actualX += shift; actualW -= shift } }
                }
                if (r == 3) { if (c == 0) actualW = dx * 2 * 0.95f else actualX = X_LEFT + ((c + 1) * dx) }
                if (r >= 4) {
                    when (c) { 0 -> actualW = dx * 0.95f; 1 -> { actualX = X_LEFT + dx * 0.95f; actualW = dx * 1.05f }; 2, 3 -> actualW = dx }
                }
                val keyBtn = Button(this); keyBtn.background = null 
                keyBtn.isFocusable = false; keyBtn.isFocusableInTouchMode = false
                keyBtn.setOnTouchListener { btn, event ->
                    Log.d(TAG, "Button $code touch: ${event.action}")
                    lastTouchX = event.rawX; lastTouchY = event.rawY
                    when (event.action) {
                        MotionEvent.ACTION_DOWN -> { 
                            btn.isPressed = true
                            performHapticClick()
                            coreTasks.offer { sendKey(code) } 
                        }
                        MotionEvent.ACTION_UP, MotionEvent.ACTION_CANCEL -> { 
                            btn.isPressed = false
                            coreTasks.offer { sendKey(0) } 
                        }
                    }
                    true
                }
                replicaOverlay.addReplicaView(keyBtn, actualX, y, actualW, kH)
            }
        }
    }

    private external fun nativePreInit(storagePath: String)
    private external fun initNative(storagePath: String, slotId: Int)
    private external fun tick()
    private external fun sendKey(keyCode: Int)
    private external fun sendSimKeyNative(keyId: String, isFn: Boolean, isRelease: Boolean)
    private external fun sendSimMenuNative(menuId: Int)
    private external fun sendSimFuncNative(funcId: Int)
    private external fun saveStateNative()
    private external fun loadStateNative()
    private external fun forceRefreshNative()
    private external fun setSlotNative(slot: Int)
    private external fun getXRegisterNative(): String
    private external fun getDisplayPixels(pixels: IntArray)
    private external fun setLcdColors(text: Int, bg: Int)
    @Keep fun onFileSelected(fd: Int) { onFileSelectedNative(fd) }
    @Keep fun onFileCancelled() { onFileCancelledNative() }
    private external fun onFileSelectedNative(fd: Int)
    private external fun onFileCancelledNative()
}