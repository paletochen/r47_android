package com.example.r47

import android.app.Activity
import android.app.PictureInPictureParams
import android.content.Context
import android.content.Intent
import android.graphics.Color
import android.media.AudioManager
import android.net.Uri
import android.os.*
import android.util.Log
import android.util.Rational
import android.view.*
import androidx.activity.result.contract.ActivityResultContracts
import androidx.annotation.Keep
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.WindowCompat
import androidx.core.view.WindowInsetsCompat
import androidx.core.view.WindowInsetsControllerCompat
import com.example.r47.databinding.ActivityMainBinding
import android.content.SharedPreferences
import android.content.res.Configuration

@Keep
class MainActivity : AppCompatActivity(), SharedPreferences.OnSharedPreferenceChangeListener {

    private val TAG = "R47Activity"
    private lateinit var binding: ActivityMainBinding
    private lateinit var replicaOverlay: ReplicaOverlay
    private lateinit var coreRuntime: NativeCoreRuntime
    private lateinit var storageAccessCoordinator: StorageAccessCoordinator
    private lateinit var displayActionController: DisplayActionController
    private val appPreferences by lazy {
        getSharedPreferences(SlotStore.APP_PREFS_NAME, MODE_PRIVATE)
    }
    private val slotStore by lazy { SlotStore(this) }
    
    private val mainHandler = Handler(Looper.getMainLooper())
    private var isHapticEnabled = true
    private var isHighFidelityHapticEnabled = true
    private var hapticIntensity = DEFAULT_HAPTIC_INTENSITY
    private var beeperVolume = 20

    companion object {
        private const val DEFAULT_HAPTIC_INTENSITY = 64
        private const val DEFAULT_CHROME_MODE = ReplicaOverlay.CHROME_MODE_BACKGROUND
        private const val DEFAULT_LCD_MODE = "vintage"
        private const val DEFAULT_SCALING_MODE = "full_width"

        init {
            System.loadLibrary("c47-android")
        }
    }

    private var isMovingToPiP = false

    private var slotsList = mutableListOf<CalculatorSlot>()
    private var currentSlotId = 0
    private var pendingSlotId: Int? = null 

    private var chromeMode = DEFAULT_CHROME_MODE
    private var lcdMode = DEFAULT_LCD_MODE
    private var scalingMode = DEFAULT_SCALING_MODE

    private val VINTAGE_TEXT = 0xFF303030.toInt()
    private val VINTAGE_BG = 0xFFDFF5CC.toInt()
    private val BW_TEXT = Color.BLACK
    private val BW_BG = Color.parseColor("#E0E0E0")
    private val YELLOW_SHIFT = Color.parseColor("#FFBF00")

    private var isBeeperEnabled = true
    private var showTouchZones = false
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

    private fun syncAudioSettings() {
        AudioEngine.updateSettings(isBeeperEnabled, beeperVolume)
    }

    private fun applySelectedSlotUri(uri: Uri, loadState: Boolean) {
        contentResolver.takePersistableUriPermission(
            uri,
            Intent.FLAG_GRANT_READ_URI_PERMISSION or Intent.FLAG_GRANT_WRITE_URI_PERMISSION,
        )

        val id = pendingSlotId ?: return
        slotsList[id].uri = uri.toString()
        slotStore.saveSlots(slotsList)
        currentSlotId = id
        setSlotNative(id)
        slotStore.writeCurrentSlotId(id)

        offerCoreTask {
            if (loadState) {
                loadStateNative()
            } else {
                saveStateNative()
            }
        }

        val verb = if (loadState) "Loaded" else "Created"
        android.widget.Toast.makeText(this, "$verb ${slotsList[id].name}", android.widget.Toast.LENGTH_SHORT).show()
    }

    @Keep
    fun playTone(milliHz: Int, durationMs: Int) {
        AudioEngine.playTone(milliHz, durationMs)
    }

    @Keep
    fun stopTone() {}

    private fun applyLcdMode(mode: String) {
        lcdMode = mode
        if (mode == DEFAULT_LCD_MODE) setLcdColors(VINTAGE_TEXT, VINTAGE_BG)
        else setLcdColors(BW_TEXT.toInt(), BW_BG.toInt())
    }

    private fun normalizeChromeMode(mode: String?): String {
        return when {
            mode == null -> DEFAULT_CHROME_MODE
            mode == ReplicaOverlay.CHROME_MODE_NATIVE ||
                mode == ReplicaOverlay.CHROME_MODE_TEXTURE ||
                mode == ReplicaOverlay.CHROME_MODE_BACKGROUND -> mode
            else -> DEFAULT_CHROME_MODE
        }
    }

    private fun applyChromeMode(mode: String) {
        chromeMode = normalizeChromeMode(mode)
        if (::replicaOverlay.isInitialized) {
            replicaOverlay.setChromeMode(chromeMode)
            setupInteractiveZones()
            if (::coreRuntime.isInitialized && chromeMode != ReplicaOverlay.CHROME_MODE_TEXTURE) {
                updateDynamicKeys()
            }
        }
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
            "haptic_intensity" -> hapticIntensity = prefs.getInt(key, DEFAULT_HAPTIC_INTENSITY)
            "beeper_volume" -> {
                beeperVolume = prefs.getInt(key, 20)
                syncAudioSettings()
            }
            "keep_screen_on" -> {
                val enabled = prefs.getBoolean(key, false)
                if (enabled) window?.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
                else window?.clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
            }
            "beeper_enabled" -> {
                isBeeperEnabled = prefs.getBoolean(key, true)
                syncAudioSettings()
            }
            "lcd_mode" -> {
                lcdMode = prefs.getString(key, DEFAULT_LCD_MODE) ?: DEFAULT_LCD_MODE
                applyLcdMode(lcdMode)
            }
            "chrome_mode" -> {
                applyChromeMode(prefs.getString(key, DEFAULT_CHROME_MODE) ?: DEFAULT_CHROME_MODE)
            }
            "scaling_mode" -> {
                scalingMode = prefs.getString(key, DEFAULT_SCALING_MODE) ?: DEFAULT_SCALING_MODE
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
    private val activeKeyIdMap = mutableMapOf<Int, PhysicalKeyboardAction>()

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

        PhysicalKeyboardMapper.resolve(keyCode, event)?.let { action ->
            activeKeyIdMap[keyCode] = action
            when (action) {
                is PhysicalKeyboardAction.NativeKey -> {
                    offerCoreTask { sendSimKeyNative(action.id, action.isFunctionKey, false) }
                }
                is PhysicalKeyboardAction.Shortcut -> {
                    PhysicalKeyboardShortcuts.dispatch(
                        action,
                        ::offerCoreTask,
                        ::sendSimKeyNative,
                        ::sendSimMenuNative,
                    )
                }
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
        activeKeyIdMap.remove(keyCode)?.let { action ->
            if (action is PhysicalKeyboardAction.NativeKey) {
                offerCoreTask { sendSimKeyNative(action.id, action.isFunctionKey, true) }
            }
            return true
        }
        return super.onKeyUp(keyCode, event)
    }

    internal fun currentKeypadSnapshot(meta: IntArray? = null): KeypadSnapshot {
        val resolvedMeta = meta ?: getKeypadMetaNative(true)
        return KeypadSnapshot.fromNative(
            resolvedMeta,
            getKeypadLabelsNative(true),
        )
    }

    private fun updateDynamicKeys(snapshot: KeypadSnapshot? = null) {
        val resolvedSnapshot = snapshot ?: currentKeypadSnapshot()
        ReplicaKeypadLayout.updateDynamicKeys(replicaOverlay, resolvedSnapshot)
    }

    private fun offerCoreTask(task: Runnable) {
        coreRuntime.offerTask(task)
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        volumeControlStream = AudioManager.STREAM_MUSIC
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        
        val prefs = appPreferences
        prefs.registerOnSharedPreferenceChangeListener(this)

        storageAccessCoordinator = StorageAccessCoordinator(
            activity = this,
            appPreferences = prefs,
            rootView = binding.root,
            launchSettings = {
                startActivity(Intent(this, SettingsActivity::class.java).apply {
                    putExtra("trigger_work_dir_picker", true)
                })
            },
            onNativeFileSelected = ::onFileSelectedNative,
            onNativeFileCancelled = ::onFileCancelledNative,
        )

        displayActionController = DisplayActionController(
            context = this,
            mainHandler = mainHandler,
            offerCoreTask = ::offerCoreTask,
            getXRegisterNative = ::getXRegisterNative,
            sendSimFuncNative = ::sendSimFuncNative,
            sendSimKeyNative = ::sendSimKeyNative,
            enterPiP = ::enterPiP,
        )
        
        window.attributes.layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES
        val isFullscreen = prefs.getBoolean("fullscreen_mode", true)
        applyFullscreenMode(isFullscreen)

        replicaOverlay = binding.replicaOverlay
        replicaOverlay.onPiPKeyEvent = { code ->
            offerCoreTask { sendKey(code) }
        }
        
        isHapticEnabled = prefs.getBoolean("haptic_enabled", true)
        isHighFidelityHapticEnabled = prefs.getBoolean("haptic_hifi_enabled", true)
        hapticIntensity = prefs.getInt("haptic_intensity", DEFAULT_HAPTIC_INTENSITY)
        beeperVolume = prefs.getInt("beeper_volume", 20)
        val storedChromeMode = prefs.getString("chrome_mode", DEFAULT_CHROME_MODE)
        chromeMode = normalizeChromeMode(storedChromeMode)
        if (storedChromeMode != chromeMode) {
            prefs.edit().putString("chrome_mode", chromeMode).apply()
        }
        lcdMode = prefs.getString("lcd_mode", DEFAULT_LCD_MODE) ?: DEFAULT_LCD_MODE
        scalingMode = prefs.getString("scaling_mode", DEFAULT_SCALING_MODE) ?: DEFAULT_SCALING_MODE
        isBeeperEnabled = prefs.getBoolean("beeper_enabled", true)
        showTouchZones = prefs.getBoolean("show_touch_zones", false)
        if (prefs.getBoolean("keep_screen_on", false)) window?.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        syncAudioSettings()
        
        replicaOverlay.setChromeMode(chromeMode)
        setupInteractiveZones()
        slotsList = slotStore.loadSlots()
        currentSlotId = slotStore.readCurrentSlotId()

        coreRuntime = NativeCoreRuntime(
            filesDirPath = filesDir.absolutePath,
            currentSlotIdProvider = { currentSlotId },
            nativePreInit = ::nativePreInit,
            initNative = ::initNative,
            updateNativeActivityRef = ::updateNativeActivityRef,
            tick = ::tick,
            saveStateNative = ::saveStateNative,
            forceRefreshNative = ::forceRefreshNative,
            getDisplayPixels = ::getDisplayPixels,
            getKeypadMetaNative = ::getKeypadMetaNative,
            useSceneDrivenKeypadProvider = { true },
            getKeypadSnapshot = ::currentKeypadSnapshot,
            onLcdPixels = { pixels -> replicaOverlay.updateLcd(pixels) },
            onDynamicRefresh = ::updateDynamicKeys,
        )
        
        replicaOverlay.post {
            replicaOverlay.setShowTouchZones(showTouchZones)
            replicaOverlay.setScalingMode(scalingMode)
            applyLcdMode(lcdMode)
            if (chromeMode != ReplicaOverlay.CHROME_MODE_TEXTURE) {
                updateDynamicKeys()
            }
        }

        displayActionController.bindOverlay(replicaOverlay)
        
        replicaOverlay.onSettingsTapListener = {
            Log.i(TAG, "Settings tap received in MainActivity")
            startActivity(Intent(this, SettingsActivity::class.java))
        }

        coreRuntime.attach()
        AudioEngine.start { NativeCoreRuntime.isAppRunning() }
    }

    private external fun updateNativeActivityRef()

    override fun onDestroy() { 
        Log.i(TAG, "onDestroy: isFinishing=$isFinishing")
        coreRuntime.dispose(stopApp = isFinishing)
        if (isFinishing) {
            AudioEngine.stop()
        }
        appPreferences.unregisterOnSharedPreferenceChangeListener(this)
        super.onDestroy() 
    }

    override fun onResume() {
        super.onResume()
        coreRuntime.requestForceRefresh()
        storageAccessCoordinator.handleResume()
    }

    override fun onPause() {
        super.onPause()
        val isEnteringPiP = isMovingToPiP || (if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) isInPictureInPictureMode else false)
        Log.i(TAG, "onPause: isEnteringPiP=$isEnteringPiP isMovingToPiP=$isMovingToPiP")
        
        if (!isEnteringPiP && appPreferences.getBoolean("auto_save_minimize", true)) {
            Log.i(TAG, "Auto-saving state on pause (synchronous via core thread)...")
            coreRuntime.saveStateOnPause(autoSaveEnabled = true)
        }
    }

    private val slotCreateLauncher = registerForActivityResult(ActivityResultContracts.StartActivityForResult()) { result ->
        if (result.resultCode == Activity.RESULT_OK) {
            result.data?.data?.let { uri ->
                applySelectedSlotUri(uri, loadState = false)
            }
        }
    }

    private val slotLoadLauncher = registerForActivityResult(ActivityResultContracts.StartActivityForResult()) { result ->
        if (result.resultCode == Activity.RESULT_OK) {
            result.data?.data?.let { uri ->
                applySelectedSlotUri(uri, loadState = true)
            }
        }
    }

    private fun switchSlot(id: Int) {
        if (id !in slotsList.indices || id == currentSlotId) return
        
        val targetName = slotsList[id].name
        offerCoreTask {
            try {
                saveStateNative()
                currentSlotId = id
                setSlotNative(id)
                loadStateNative()
                mainHandler.post {
                    slotStore.writeCurrentSlotId(id)
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
            storageAccessCoordinator.requestNativeFile(isSave, defaultName, fileType)
        }
    }

    @Keep
    fun quitApp() {
        Log.i(TAG, "quitApp called from native")
        mainHandler.post { 
            val forceClose = appPreferences.getBoolean("force_close_on_exit", false)
            if (forceClose) {
                finishAndRemoveTask()
            } else {
                moveTaskToBack(true)
            }
        }
    }

    @Keep
    fun processCoreTasks() {
        coreRuntime.processCoreTasks()
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
        ReplicaKeypadLayout.rebuild(
            activity = this,
            overlay = replicaOverlay,
            chromeMode = chromeMode,
            performHapticClick = ::performHapticClick,
            dispatchKey = { keyCode -> offerCoreTask { sendKey(keyCode) } },
        )
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
    
    // Legacy keypad getters kept for bridge compatibility.
    private external fun getButtonLabelNative(keyCode: Int, type: Int, isDynamic: Boolean): String
    private external fun getSoftkeyLabelNative(fnKeyIndex: Int): String
    private external fun getKeyboardStateNative(): IntArray // returns [shiftF, shiftG, calcMode, userMode, alphaFlag]

    // Snapshot keypad APIs used by the default Android-native keypad.
    private external fun getKeypadMetaNative(isDynamic: Boolean): IntArray
    private external fun getKeypadLabelsNative(isDynamic: Boolean): Array<String>

    @Keep fun onFileSelected(fd: Int) { onFileSelectedNative(fd) }
    @Keep fun onFileCancelled() { onFileCancelledNative() }
    private external fun onFileSelectedNative(fd: Int)
    private external fun onFileCancelledNative()
}