package org.rpncalculators.r47

import android.view.Choreographer

internal interface DisplayRefreshLoop {
    fun start()
    fun stop()
}

internal class NativeDisplayRefreshLoop(
    private val isAppRunning: () -> Boolean,
    private val isNativeInitialized: () -> Boolean,
    private val getDisplayPixels: (IntArray) -> Unit,
    private val getKeypadMetaNative: (Boolean, Boolean) -> IntArray,
    private val useSceneDrivenKeypadProvider: () -> Boolean,
    private val getKeypadSnapshot: (IntArray) -> KeypadSnapshot,
    private val onLcdPixels: (IntArray) -> Unit,
    private val onDynamicRefresh: (KeypadSnapshot) -> Unit,
) : DisplayRefreshLoop {
    private val lcdPixels = IntArray(R47LcdContract.PIXEL_COUNT)
    private var lastLabelRefresh = 0L
    private var lastKeypadMeta = IntArray(0)
    private var isActive = false

    private val frameCallback = object : Choreographer.FrameCallback {
        override fun doFrame(frameTimeNanos: Long) {
            if (!isActive || !isAppRunning()) {
                return
            }

            if (isNativeInitialized()) {
                getDisplayPixels(lcdPixels)
                if (lcdPixels.isNotEmpty()) {
                    onLcdPixels(lcdPixels)
                }

                val currentMeta = getKeypadMetaNative(useSceneDrivenKeypadProvider(), useSceneDrivenKeypadProvider())
                val now = System.currentTimeMillis()
                val shouldRefreshLabels = now - lastLabelRefresh > 500
                val keypadStateChanged = !lastKeypadMeta.contentEquals(currentMeta)
                if (shouldRefreshLabels || keypadStateChanged) {
                    lastKeypadMeta = currentMeta.copyOf()
                    onDynamicRefresh(getKeypadSnapshot(currentMeta))
                    lastLabelRefresh = now
                }
            }

            if (isActive && isAppRunning()) {
                Choreographer.getInstance().postFrameCallback(this)
            }
        }
    }

    override fun start() {
        if (isActive) {
            return
        }

        isActive = true
        Choreographer.getInstance().postFrameCallback(frameCallback)
    }

    override fun stop() {
        if (!isActive) {
            return
        }

        isActive = false
        Choreographer.getInstance().removeFrameCallback(frameCallback)
    }
}
