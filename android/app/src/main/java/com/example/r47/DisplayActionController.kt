package com.example.r47

import android.content.ClipData
import android.content.ClipboardManager
import android.content.Context
import android.os.Handler
import android.util.Log
import com.google.android.material.dialog.MaterialAlertDialogBuilder
import java.util.Locale

internal class DisplayActionController(
    private val context: Context,
    private val mainHandler: Handler,
    private val offerCoreTask: (Runnable) -> Unit,
    private val getXRegisterNative: () -> String,
    private val sendSimFuncNative: (Int) -> Unit,
    private val sendSimKeyNative: (String, Boolean, Boolean) -> Unit,
    private val enterPiP: () -> Unit,
) {
    companion object {
        private const val TAG = "R47DisplayActions"
    }

    fun bindOverlay(overlay: ReplicaOverlay) {
        overlay.onLongPressListener = { x, y ->
            if (isDisplayLongPress(overlay, x, y)) {
                showDisplayActionsDialog()
            }
        }
    }

    private fun isDisplayLongPress(overlay: ReplicaOverlay, x: Float, y: Float): Boolean {
        return overlay.isPointInLcd(x, y)
    }

    private fun showDisplayActionsDialog() {
        MaterialAlertDialogBuilder(context)
            .setTitle("Display & Clipboard")
            .setItems(arrayOf("Copy X Register", "Paste Number", "Enter PiP Mode")) { _, which ->
                when (which) {
                    0 -> copyXToClipboard()
                    1 -> pasteFromClipboard()
                    2 -> enterPiP()
                }
            }
            .setNegativeButton("Cancel", null)
            .show()
    }

    private fun copyXToClipboard() {
        offerCoreTask(
            Runnable {
                try {
                    val xRegisterValue = getXRegisterNative()
                    mainHandler.post {
                        val clipboard = context.getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager
                        val clip = ClipData.newPlainText("R47 X Register", xRegisterValue)
                        clipboard.setPrimaryClip(clip)
                        android.widget.Toast.makeText(
                            context,
                            "Copied: $xRegisterValue",
                            android.widget.Toast.LENGTH_SHORT,
                        ).show()
                    }
                } catch (error: Exception) {
                    Log.e(TAG, "Copy error", error)
                }
            }
        )
    }

    private fun pasteFromClipboard() {
        val clipboard = context.getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager
        val item = clipboard.primaryClip?.getItemAt(0)
        val text = item?.text?.toString() ?: return

        offerCoreTask(
            Runnable {
                for (char in text) {
                    if (char == 'i' || char == 'j') {
                        sendSimFuncNative(if (char == 'i') 1159 else 1160)
                        Thread.sleep(50)
                        continue
                    }

                    val simIdNumber = when (char.lowercaseChar()) {
                        '0' -> 33
                        '1' -> 28
                        '2' -> 29
                        '3' -> 30
                        '4' -> 23
                        '5' -> 24
                        '6' -> 25
                        '7' -> 18
                        '8' -> 19
                        '9' -> 20
                        '.', ',' -> 34
                        '-' -> 35
                        'e' -> 15
                        '+' -> 37
                        else -> null
                    }

                    if (simIdNumber != null) {
                        val simId = String.format(Locale.US, "%02d", simIdNumber)
                        sendSimKeyNative(simId, false, false)
                        Thread.sleep(50)
                        sendSimKeyNative(simId, false, true)
                        Thread.sleep(50)
                    }
                }
            }
        )
    }
}