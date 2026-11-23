package org.rpncalculators.r47

import android.view.Choreographer

internal interface DisplayRefreshLoop {
    fun start()
    fun stop()
    fun invalidateKeypadCache() {}
}

internal interface FrameScheduler {
    fun postFrameCallback(callback: Choreographer.FrameCallback)
    fun removeFrameCallback(callback: Choreographer.FrameCallback)
}

internal object ChoreographerFrameScheduler : FrameScheduler {
    override fun postFrameCallback(callback: Choreographer.FrameCallback) {
        Choreographer.getInstance().postFrameCallback(callback)
    }

    override fun removeFrameCallback(callback: Choreographer.FrameCallback) {
        Choreographer.getInstance().removeFrameCallback(callback)
    }
}

internal class NativeDisplayRefreshLoop(
    private val isAppRunning: () -> Boolean,
    private val isNativeInitialized: () -> Boolean,
    private val getDisplayPixels: (IntArray) -> Unit,
    private val getKeypadMetaNative: (Boolean, Boolean) -> IntArray?,
    private val useSceneDrivenKeypadProvider: () -> Boolean,
    private val getKeypadSnapshot: (IntArray) -> KeypadSnapshot?,
    private val onLcdPixels: (IntArray) -> Unit,
    private val onDynamicRefresh: (KeypadSnapshot) -> Unit,
    private val frameScheduler: FrameScheduler = ChoreographerFrameScheduler,
    private val currentTimeMillis: () -> Long = System::currentTimeMillis,
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
                if (currentMeta != null) {
                    val now = currentTimeMillis()
                    val shouldRefreshLabels = now - lastLabelRefresh > 500
                    val keypadStateChanged = !lastKeypadMeta.contentEquals(currentMeta)
                    if (shouldRefreshLabels || keypadStateChanged) {
                        val snapshot = getKeypadSnapshot(currentMeta)
                        if (snapshot != null) {
                            lastKeypadMeta = currentMeta.copyOf()
                            onDynamicRefresh(snapshot)
                            lastLabelRefresh = now
                        }
                    }
                }
            }

            if (isActive && isAppRunning()) {
                frameScheduler.postFrameCallback(this)
            }
        }
    }

    override fun start() {
        if (isActive) {
            return
        }

        isActive = true
        frameScheduler.postFrameCallback(frameCallback)
    }

    override fun stop() {
        if (!isActive) {
            return
        }

        isActive = false
        frameScheduler.removeFrameCallback(frameCallback)
    }

    override fun invalidateKeypadCache() {
        lastKeypadMeta = IntArray(0)
    }
}
