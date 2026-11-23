package org.rpncalculators.r47

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit
import java.util.concurrent.atomic.AtomicBoolean
import java.util.concurrent.atomic.AtomicInteger

class NativeCoreRuntimeTest {

    private val noopRefreshLoop = object : DisplayRefreshLoop {
        override fun start() {}
        override fun stop() {}
    }

    private val noopLogger = object : CoreRuntimeLogger {
        override fun i(tag: String, message: String) {}
        override fun w(tag: String, message: String) {}
        override fun e(tag: String, message: String, error: Throwable?) {}
    }

    private fun createRuntime(
        session: CoreRuntimeSession,
        initNative: (String, Int) -> Unit = { _, _ -> },
        updateNativeActivityRef: () -> Unit = {},
        tick: () -> Unit = {},
        requestNativeShutdown: () -> Unit = {},
        shutdownWaitTimeoutMs: Long = 500L,
    ): NativeCoreRuntime = NativeCoreRuntime(
        filesDirPath = "/tmp",
        currentSlotIdProvider = { 0 },
        nativePreInit = {},
        initNative = initNative,
        updateNativeActivityRef = updateNativeActivityRef,
        tick = tick,
        saveStateNative = {},
        forceRefreshNative = {},
        getDisplayPixels = {},
        getKeypadMetaNative = { _, _ -> IntArray(0) },
        useSceneDrivenKeypadProvider = { false },
        getKeypadSnapshot = { throw UnsupportedOperationException("unused in noop refresh loop") },
        onLcdPixels = {},
        onDynamicRefresh = {},
        requestNativeShutdown = requestNativeShutdown,
        session = session,
        logger = noopLogger,
        shutdownWaitTimeoutMs = shutdownWaitTimeoutMs,
        displayRefreshLoop = noopRefreshLoop,
    )

    @Test
    fun testInitialState() {
        val session = CoreRuntimeSession()
        assertFalse(session.isAppRunning)
    }

    @Test
    fun testResetSharedState() {
        NativeCoreRuntime.resetSharedState()
        assertFalse(NativeCoreRuntime.isAppRunning())
    }

    @Test
    fun testDisposeStopAppWakesBlockedCoreThreadAndUpdatesRotatedOwnerCallbacks() {
        val session = CoreRuntimeSession()
        val initDone = CountDownLatch(1)
        val rotatedTickObserved = CountDownLatch(1)
        val taskEntered = CountDownLatch(1)
        val shutdownWakeLatch = CountDownLatch(1)
        val shutdownCount = AtomicInteger(0)
        val actualStopCalled = AtomicBoolean(false)

        val firstActivityRuntime = createRuntime(
            session = session,
            initNative = { _, _ -> initDone.countDown() },
            requestNativeShutdown = {
                shutdownCount.incrementAndGet()
                shutdownWakeLatch.countDown()
            },
        )
        val rotatedActivityRuntime = createRuntime(
            session = session,
            tick = { rotatedTickObserved.countDown() },
            requestNativeShutdown = {
                shutdownCount.incrementAndGet()
                shutdownWakeLatch.countDown()
            },
        )

        firstActivityRuntime.attach()
        assertTrue("initNative must execute on core thread", initDone.await(2, TimeUnit.SECONDS))

        // Configuration change: second Activity attaches before first Activity disposes with stopApp = false
        rotatedActivityRuntime.attach()
        assertTrue(firstActivityRuntime.dispose(stopApp = false))
        assertEquals(0, shutdownCount.get())
        assertTrue("Core thread must delegate tick() to rotated Activity runtime", rotatedTickObserved.await(2, TimeUnit.SECONDS))

        // Enqueue a task simulating PSE 99 that unblocks only when requestNativeShutdown fires
        rotatedActivityRuntime.offerTask(Runnable {
            taskEntered.countDown()
            shutdownWakeLatch.await(2, TimeUnit.SECONDS)
        })
        assertTrue("Blocked core task must start running", taskEntered.await(2, TimeUnit.SECONDS))

        val exitedCleanly = rotatedActivityRuntime.dispose(
            stopApp = true,
            onActualStop = { actualStopCalled.set(true) },
        )

        assertTrue("Core thread must exit cleanly within latch budget", exitedCleanly)
        assertEquals(1, shutdownCount.get())
        assertTrue("onActualStop must execute after core thread exit", actualStopCalled.get())
        assertFalse(session.isAppRunning)
    }

    @Test
    fun testSameProcessRelaunchAfterDisposeStopAppReattachesCleanly() {
        val session = CoreRuntimeSession()
        val initCount = AtomicInteger(0)
        val updateRefDone = CountDownLatch(1)
        val relaunchedTaskDone = CountDownLatch(1)
        val shutdownRequested = AtomicBoolean(false)

        val runtime = createRuntime(
            session = session,
            initNative = { _, _ -> initCount.incrementAndGet() },
            updateNativeActivityRef = {
                shutdownRequested.set(false)
                updateRefDone.countDown()
            },
            requestNativeShutdown = { shutdownRequested.set(true) },
        )

        runtime.attach()
        val firstSessionReady = CountDownLatch(1)
        runtime.offerTask(Runnable { firstSessionReady.countDown() })
        assertTrue(firstSessionReady.await(2, TimeUnit.SECONDS))
        assertTrue(runtime.dispose(stopApp = true))
        assertTrue(shutdownRequested.get())

        runtime.attach()
        assertTrue("updateNativeActivityRef must run on warm-process relaunch", updateRefDone.await(2, TimeUnit.SECONDS))
        assertFalse("Warm-process relaunch must clear prior shutdown latch", shutdownRequested.get())
        assertEquals(1, initCount.get())

        runtime.offerTask(Runnable { relaunchedTaskDone.countDown() })
        assertTrue("Tasks on relaunched core thread must execute normally", relaunchedTaskDone.await(2, TimeUnit.SECONDS))
        assertTrue(runtime.dispose(stopApp = true))
    }

    @Test
    fun testShutdownTimeoutDefersCleanupUntilSlowTaskFinishes() {
        val session = CoreRuntimeSession()
        val slowTaskEntered = CountDownLatch(1)
        val releaseSlowTask = CountDownLatch(1)
        val cleanupExecuted = CountDownLatch(1)
        val cleanupRanWhileTaskActive = AtomicBoolean(false)
        val taskStillActive = AtomicBoolean(false)

        val runtime = createRuntime(
            session = session,
            shutdownWaitTimeoutMs = 40L,
        )

        runtime.attach()
        runtime.offerTask(Runnable {
            taskStillActive.set(true)
            slowTaskEntered.countDown()
            releaseSlowTask.await(2, TimeUnit.SECONDS)
            taskStillActive.set(false)
        })
        assertTrue(slowTaskEntered.await(2, TimeUnit.SECONDS))

        val exitedWithinBudget = runtime.dispose(
            stopApp = true,
            onActualStop = {
                if (taskStillActive.get()) {
                    cleanupRanWhileTaskActive.set(true)
                }
                cleanupExecuted.countDown()
            },
        )

        assertFalse("dispose must report timeout when slow step exceeds budget", exitedWithinBudget)
        assertEquals("onActualStop must not run while native core thread is still executing", 1L, cleanupExecuted.count)

        releaseSlowTask.countDown()
        assertTrue("Deferred onActualStop must run once core thread exits", cleanupExecuted.await(2, TimeUnit.SECONDS))
        assertFalse("onActualStop must never overlap active native execution", cleanupRanWhileTaskActive.get())
    }
}
