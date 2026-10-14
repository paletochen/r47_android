package org.rpncalculators.r47

import android.content.ClipData
import android.content.ClipboardManager
import android.content.Context
import android.os.Handler
import android.util.Log
import com.google.android.material.dialog.MaterialAlertDialogBuilder
import java.util.Locale

internal enum class PasteAction {
    Key,
    Func
}

internal data class PasteStep(val action: PasteAction, val id: String, val funcId: Int = 0)

internal class DisplayActionController(
    private val context: Context,
    private val mainHandler: Handler,
    private val offerCoreTask: (Runnable) -> Unit,
    private val getXRegisterNative: () -> String,
    private val getAllRegistersNative: () -> String = { "" },
    private val pasteTextNative: (String) -> Unit = {},
    private val sendSimFuncNative: (Int) -> Unit,
    private val sendSimKeyNative: (String, Boolean, Boolean) -> Unit,
    private val enterPiP: () -> Unit,
) {
    companion object {
        private const val TAG = "R47DisplayActions"

        internal fun parsePasteSteps(text: String): List<PasteStep> {
            val steps = mutableListOf<PasteStep>()
            var inExponent = false
            var mantissaNeg = false
            var exponentNeg = false
            var signEmitted = false

            fun emitPendingSigns() {
                if (mantissaNeg && !signEmitted) {
                    steps.add(PasteStep(PasteAction.Key, "35")) // CHS key
                    signEmitted = true
                }
                if (exponentNeg) {
                    steps.add(PasteStep(PasteAction.Key, "35")) // CHS key
                    exponentNeg = false
                }
            }

            var i = 0
            while (i < text.length) {
                val char = text[i]

                // Check for multi-character or degree symbols
                if (char == '°') {
                    emitPendingSigns()
                    steps.add(PasteStep(PasteAction.Func, "", 115)) // ITM_DEG2 (⇒DEG)
                    i++
                    continue
                }

                if (char.equals('d', ignoreCase = true) && i + 2 < text.length &&
                    text[i + 1].equals('e', ignoreCase = true) &&
                    text[i + 2].equals('g', ignoreCase = true)) {
                    emitPendingSigns()
                    steps.add(PasteStep(PasteAction.Func, "", 115)) // ITM_DEG2 (⇒DEG)
                    i += 3
                    continue
                }

                if (char == 'i' || char == 'j') {
                    emitPendingSigns()
                    steps.add(PasteStep(PasteAction.Func, "", if (char == 'i') 1159 else 1160))
                    inExponent = false
                    signEmitted = false
                    i++
                    continue
                }

                if (char == '-') {
                    if (!inExponent && (i == 0 || text[i - 1] == ' ' || text[i - 1] == '(' || text[i - 1] == 'i' || text[i - 1] == 'j')) {
                        mantissaNeg = true
                        i++
                        continue
                    } else if (inExponent && (text[i - 1] == 'e' || text[i - 1] == 'E')) {
                        exponentNeg = true
                        i++
                        continue
                    }
                }

                if (char == 'e' || char == 'E') {
                    if (mantissaNeg && !signEmitted) {
                        steps.add(PasteStep(PasteAction.Key, "35")) // CHS for mantissa
                        signEmitted = true
                    }
                    inExponent = true
                    steps.add(PasteStep(PasteAction.Key, "15")) // EEX
                    i++
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
                    '+' -> 37
                    else -> null
                }

                if (simIdNumber != null) {
                    val simId = String.format(Locale.US, "%02d", simIdNumber)
                    steps.add(PasteStep(PasteAction.Key, simId))
                }
                i++
            }

            emitPendingSigns()
            return steps
        }
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
            .setItems(arrayOf("Copy X Register", "Copy All Registers", "Paste Number", "Enter PiP Mode")) { _, which ->
                when (which) {
                    0 -> copyXToClipboard()
                    1 -> copyAllRegistersToClipboard()
                    2 -> pasteFromClipboard()
                    3 -> enterPiP()
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

    private fun copyAllRegistersToClipboard() {
        offerCoreTask(
            Runnable {
                try {
                    val allRegistersValue = getAllRegistersNative()
                    mainHandler.post {
                        val clipboard = context.getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager
                        val clip = ClipData.newPlainText("R47 All Registers", allRegistersValue)
                        clipboard.setPrimaryClip(clip)
                        android.widget.Toast.makeText(
                            context,
                            "Copied All Registers",
                            android.widget.Toast.LENGTH_SHORT,
                        ).show()
                    }
                } catch (error: Exception) {
                    Log.e(TAG, "Copy all registers error", error)
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
                pasteTextNative(text)
            }
        )
    }
}