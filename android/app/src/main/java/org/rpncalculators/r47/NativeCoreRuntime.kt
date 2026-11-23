package org.rpncalculators.r47

import android.util.Log
import java.util.concurrent.CountDownLatch
import java.util.concurrent.LinkedBlockingQueue
import java.util.concurrent.TimeUnit

internal interface CoreRuntimeLogger {
    fun i(tag: String, message: String)
    fun w(tag: String, message: String)
    fun e(tag: String, message: String, error: Throwable? = null)
}

internal object AndroidCoreRuntimeLogger : CoreRuntimeLogger {
    override fun i(tag: String, message: String) {
        Log.i(tag, message)
    }

    override fun w(tag: String, message: String) {
        Log.w(tag, message)
    }

    override fun e(tag: String, message: String, error: Throwable?) {
        if (error != null) {
            Log.e(tag, message, error)
        } else {
            Log.e(tag, message)
        }
    }
}

internal class CoreRuntimeSession {
    val lifecycleLock = Any()
    val coreTasks = LinkedBlockingQueue<Runnable>()

    @Volatile
    var isCoreThreadStarted = false

    @Volatile
    var isAppRunning = false

    @Volatile
    var isNativeInitialized = false

    @Volatile
    var coreThreadExitLatch: CountDownLatch? = null

    @Volatile
    var pendingDeferredStopCleanup: (() -> Unit)? = null

    var activeActivityCount = 0

    @Volatile
    var activeRuntime: NativeCoreRuntime? = null

    fun reset(timeoutMs: Long = 500L) {
        val latchToAwait: CountDownLatch?
        synchronized(lifecycleLock) {
            isAppRunning = false
            coreTasks.clear()
            pendingDeferredStopCleanup = null
            latchToAwait = coreThreadExitLatch
        }
        latchToAwait?.await(timeoutMs, TimeUnit.MILLISECONDS)
        synchronized(lifecycleLock) {
            if (coreThreadExitLatch === latchToAwait && latchToAwait?.count == 0L) {
                coreThreadExitLatch = null
            }
            if (coreThreadExitLatch == null) {
                isCoreThreadStarted = false
            }
            isNativeInitialized = false
            activeActivityCount = 0
            activeRuntime = null
        }
    }
}

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
    private val getKeypadMetaNative: (Boolean, Boolean) -> IntArray?,
    private val useSceneDrivenKeypadProvider: () -> Boolean,
    private val getKeypadSnapshot: (IntArray) -> KeypadSnapshot?,
    private val onLcdPixels: (IntArray) -> Unit,
    private val onDynamicRefresh: (KeypadSnapshot) -> Unit,
    private val requestNativeShutdown: () -> Unit = {},
    private val session: CoreRuntimeSession = defaultSession,
    private val logger: CoreRuntimeLogger = AndroidCoreRuntimeLogger,
    private val shutdownWaitTimeoutMs: Long = 500L,
    private val displayRefreshLoop: DisplayRefreshLoop = NativeDisplayRefreshLoop(
        isAppRunning = { session.isAppRunning },
        isNativeInitialized = { session.isNativeInitialized },
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
        private val defaultSession = CoreRuntimeSession()

        fun isAppRunning(): Boolean = defaultSession.isAppRunning

        internal fun resetSharedState() {
            defaultSession.reset()
        }
    }

    fun attach() {
        val dyingLatch: CountDownLatch? = synchronized(session.lifecycleLock) {
            // If a prior shutdown timed out and its thread is still winding down, cancel any deferred release
            // and wait briefly so attach() never collides with a dying thread.
            session.pendingDeferredStopCleanup = null
            if (!session.isAppRunning && session.isCoreThreadStarted) session.coreThreadExitLatch else null
        }
        dyingLatch?.await(shutdownWaitTimeoutMs, TimeUnit.MILLISECONDS)

        var shouldUpdateActivityRef = false
        synchronized(session.lifecycleLock) {
            session.activeActivityCount++
            session.activeRuntime = this
            session.pendingDeferredStopCleanup = null
            logger.i(TAG, "attach: activeActivityCount=${session.activeActivityCount}")
            session.isAppRunning = true
            if (!session.isCoreThreadStarted) {
                startCoreThreadLocked()
            } else {
                shouldUpdateActivityRef = true
            }
        }
        if (shouldUpdateActivityRef) {
            logger.i(TAG, "Core thread already running; updating activity ref")
            updateNativeActivityRef()
        }
        displayRefreshLoop.start()
    }

    fun dispose(stopApp: Boolean, onActualStop: (() -> Unit)? = null): Boolean {
        displayRefreshLoop.stop()
        val latchToAwait: CountDownLatch?
        val shouldRequestShutdown: Boolean
        synchronized(session.lifecycleLock) {
            if (session.activeActivityCount > 0) {
                session.activeActivityCount--
            }
            logger.i(TAG, "dispose: activeActivityCount=${session.activeActivityCount} stopApp=$stopApp")
            if (!stopApp || session.activeActivityCount > 0) {
                if (stopApp) {
                    logger.i(TAG, "Still have active activities (${session.activeActivityCount}), keeping core thread running")
                }
                return true
            }
            logger.i(TAG, "No active activities left, stopping core thread")
            session.isAppRunning = false
            session.coreTasks.clear()
            shouldRequestShutdown = session.isNativeInitialized
            latchToAwait = session.coreThreadExitLatch
        }

        if (shouldRequestShutdown) {
            requestNativeShutdown()
        }

        val exitedCleanly = waitForCoreThreadToExit(latchToAwait)
        if (exitedCleanly) {
            onActualStop?.invoke()
        } else if (onActualStop != null) {
            var runNow = false
            synchronized(session.lifecycleLock) {
                if (!session.isCoreThreadStarted && !session.isAppRunning) {
                    runNow = true
                } else if (!session.isAppRunning) {
                    session.pendingDeferredStopCleanup = onActualStop
                }
            }
            if (runNow) {
                onActualStop.invoke()
            }
        }
        return exitedCleanly
    }

    private fun waitForCoreThreadToExit(latch: CountDownLatch?): Boolean {
        if (latch == null) {
            return true
        }
        return try {
            logger.i(TAG, "Waiting for core thread to exit...")
            val exited = latch.await(shutdownWaitTimeoutMs, TimeUnit.MILLISECONDS)
            if (!exited) {
                logger.w(TAG, "Timeout waiting for core thread to exit")
            } else {
                logger.i(TAG, "Core thread exited cleanly")
            }
            exited
        } catch (error: InterruptedException) {
            Thread.currentThread().interrupt()
            logger.e(TAG, "Interrupted while waiting for core thread to exit", error)
            false
        }
    }

    fun offerTask(task: Runnable) {
        if (session.isAppRunning) {
            session.coreTasks.offer(task)
        }
    }

    fun processCoreTasks() {
        drainCoreTasks()
    }

    fun requestForceRefresh() {
        if (session.isNativeInitialized) {
            val target = session.activeRuntime ?: this
            offerTask(Runnable { target.forceRefreshNative() })
        }
    }

    fun invalidateKeypadCache() {
        displayRefreshLoop.invalidateKeypadCache()
    }

    fun saveStateOnPause(autoSaveEnabled: Boolean, timeoutSeconds: Long = 2) {
        if (!autoSaveEnabled || !session.isNativeInitialized) {
            return
        }

        val target = session.activeRuntime ?: this
        val latch = CountDownLatch(1)
        offerTask(
            Runnable {
                try {
                    target.saveStateNative()
                } finally {
                    latch.countDown()
                }
            }
        )

        try {
            if (!latch.await(timeoutSeconds, TimeUnit.SECONDS)) {
                logger.w(TAG, "Timed out waiting for state save on pause")
            }
        } catch (error: InterruptedException) {
            Thread.currentThread().interrupt()
            logger.e(TAG, "Interrupted while waiting for state save", error)
        }
    }

    private fun startCoreThreadLocked() {
        val threadLatch = CountDownLatch(1)
        session.isCoreThreadStarted = true
        session.coreThreadExitLatch = threadLatch
        Thread {
            try {
                val initialOwner = session.activeRuntime ?: this@NativeCoreRuntime
                logger.i(TAG, "Core thread starting; nativeInitialized=${session.isNativeInitialized}")
                if (!session.isNativeInitialized) {
                    initialOwner.nativePreInit(initialOwner.filesDirPath)
                    initialOwner.initNative(initialOwner.filesDirPath, initialOwner.currentSlotIdProvider())
                    session.isNativeInitialized = true
                } else {
                    initialOwner.updateNativeActivityRef()
                }

                var lastTickLog = 0L
                while (session.isAppRunning) {
                    val now = System.currentTimeMillis()
                    if (now - lastTickLog > 5000) {
                        logger.i(TAG, "Core thread heartbeat")
                        lastTickLog = now
                    }

                    drainCoreTasks()
                    val currentOwner = session.activeRuntime ?: initialOwner
                    currentOwner.tick()
                    Thread.sleep(10)
                }
                logger.i(TAG, "Core thread exiting")
            } catch (error: Exception) {
                logger.e(TAG, "Native core thread crashed", error)
            } finally {
                val deferredCleanup: (() -> Unit)? = synchronized(session.lifecycleLock) {
                    session.isCoreThreadStarted = false
                    if (session.coreThreadExitLatch === threadLatch) {
                        session.coreThreadExitLatch = null
                    }
                    val cleanup = if (!session.isAppRunning) session.pendingDeferredStopCleanup else null
                    session.pendingDeferredStopCleanup = null
                    threadLatch.countDown()
                    cleanup
                }
                deferredCleanup?.invoke()
            }
        }.start()
    }

    private fun drainCoreTasks() {
        var task = session.coreTasks.poll()
        while (task != null) {
            try {
                task.run()
            } catch (error: Exception) {
                logger.e(TAG, "Core task failed", error)
            }
            task = session.coreTasks.poll()
        }
    }
}
