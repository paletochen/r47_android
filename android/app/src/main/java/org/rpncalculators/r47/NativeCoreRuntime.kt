package org.rpncalculators.r47

import android.util.Log
import android.view.Choreographer
import java.util.concurrent.CountDownLatch
import java.util.concurrent.LinkedBlockingQueue
import java.util.concurrent.TimeUnit

internal class NativeCoreRuntime(
    private val filesDirPath: String,
    private val currentSlotIdProvider: () -> Int,
    private val nativePreInit: (String) -> Unit,
    private val initNative: (String, Int) -> Unit,
    private val updateNativeActivityRef: () -> Unit,
    private val tick: () -> Unit,
    private val saveStateNative: () -> Unit,
    private val forceRefreshNative: () -> Unit,
    private val getDisplayPixels: (IntArray) -> Unit,
    private val getKeypadMetaNative: (Boolean, Boolean) -> IntArray,
    private val useSceneDrivenKeypadProvider: () -> Boolean,
    private val getKeypadSnapshot: (IntArray) -> KeypadSnapshot,
    private val onLcdPixels: (IntArray) -> Unit,
    private val onDynamicRefresh: (KeypadSnapshot) -> Unit,
    private val displayRefreshLoop: DisplayRefreshLoop = NativeDisplayRefreshLoop(
        isAppRunning = { isAppRunningShared },
        isNativeInitialized = { isNativeInitializedShared },
        getDisplayPixels = getDisplayPixels,
        getKeypadMetaNative = getKeypadMetaNative,
        useSceneDrivenKeypadProvider = useSceneDrivenKeypadProvider,
        getKeypadSnapshot = getKeypadSnapshot,
        onLcdPixels = onLcdPixels,
        onDynamicRefresh = onDynamicRefresh,
    )
) {
    companion object {
        private const val TAG = "R47CoreRuntime"

        private val coreTasks = LinkedBlockingQueue<Runnable>()

        @Volatile
        private var isCoreThreadStarted = false

        @Volatile
        private var isAppRunningShared = false

        @Volatile
        private var isNativeInitializedShared = false

        @Volatile
        private var coreThreadExitLatch: CountDownLatch? = null

        @Volatile
        private var activeActivityCount = 0

        fun isAppRunning(): Boolean = isAppRunningShared

        internal fun resetSharedState() {
            coreTasks.clear()
            isCoreThreadStarted = false
            isAppRunningShared = false
            isNativeInitializedShared = false
            activeActivityCount = 0
        }

    }



    fun attach() {
        activeActivityCount++
        Log.i(TAG, "attach: activeActivityCount=$activeActivityCount")
        isAppRunningShared = true
        startOrAttachCoreThread()
        displayRefreshLoop.start()
    }

    fun dispose(stopApp: Boolean, onActualStop: (() -> Unit)? = null) {
        displayRefreshLoop.stop()
        activeActivityCount--
        Log.i(TAG, "dispose: activeActivityCount=$activeActivityCount stopApp=$stopApp")
        if (stopApp) {
            if (activeActivityCount <= 0) {
                Log.i(TAG, "No active activities left, stopping core thread")
                isAppRunningShared = false
                coreTasks.clear()
                waitForCoreThreadToExit()
                onActualStop?.invoke()
            } else {
                Log.i(TAG, "Still have active activities ($activeActivityCount), keeping core thread running")
            }
        }
    }



    private fun waitForCoreThreadToExit() {
        val latch = coreThreadExitLatch ?: return
        try {
            Log.i(TAG, "Waiting for core thread to exit...")
            if (!latch.await(500, TimeUnit.MILLISECONDS)) {
                Log.w(TAG, "Timeout waiting for core thread to exit")
            } else {
                Log.i(TAG, "Core thread exited cleanly")
            }
        } catch (error: InterruptedException) {
            Log.e(TAG, "Interrupted while waiting for core thread to exit", error)
        } finally {
            coreThreadExitLatch = null
        }
    }


    fun offerTask(task: Runnable) {
        if (isAppRunningShared) {
            coreTasks.offer(task)
        }
    }

    fun processCoreTasks() {
        drainCoreTasks()
    }

    fun requestForceRefresh() {
        if (isNativeInitializedShared) {
            offerTask(Runnable { forceRefreshNative() })
        }
    }

    fun saveStateOnPause(autoSaveEnabled: Boolean, timeoutSeconds: Long = 2) {
        if (!autoSaveEnabled || !isNativeInitializedShared) {
            return
        }

        val latch = CountDownLatch(1)
        offerTask(
            Runnable {
                try {
                    saveStateNative()
                } finally {
                    latch.countDown()
                }
            }
        )

        try {
            if (!latch.await(timeoutSeconds, TimeUnit.SECONDS)) {
                Log.w(TAG, "Timed out waiting for state save on pause")
            }
        } catch (error: InterruptedException) {
            Log.e(TAG, "Interrupted while waiting for state save", error)
        }
    }

    private fun startOrAttachCoreThread() {
        if (!isCoreThreadStarted) {
            isCoreThreadStarted = true
            coreThreadExitLatch = CountDownLatch(1)
            Thread {
                try {
                    Log.i(TAG, "Core thread starting; nativeInitialized=$isNativeInitializedShared")
                    if (!isNativeInitializedShared) {
                        nativePreInit(filesDirPath)
                        initNative(filesDirPath, currentSlotIdProvider())
                        isNativeInitializedShared = true
                    } else {
                        updateNativeActivityRef()
                    }

                    var lastTickLog = 0L
                    while (isAppRunningShared) {
                        val now = System.currentTimeMillis()
                        if (now - lastTickLog > 5000) {
                            Log.i(TAG, "Core thread heartbeat")
                            lastTickLog = now
                        }

                        drainCoreTasks()
                        tick()
                        Thread.sleep(10)
                    }
                    Log.i(TAG, "Core thread exiting")
                } catch (error: Exception) {
                    Log.e(TAG, "Native core thread crashed", error)
                } finally {
                    isCoreThreadStarted = false
                    coreThreadExitLatch?.countDown()
                }
            }.start()
        } else {
            Log.i(TAG, "Core thread already running; updating activity ref")
            updateNativeActivityRef()
        }
    }




    private fun drainCoreTasks() {
        var task = coreTasks.poll()
        while (task != null) {
            try {
                task.run()
            } catch (error: Exception) {
                Log.e(TAG, "Core task failed", error)
            }
            task = coreTasks.poll()
        }
    }
}