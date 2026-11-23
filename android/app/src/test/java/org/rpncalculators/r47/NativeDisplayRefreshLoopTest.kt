package org.rpncalculators.r47

import android.view.Choreographer
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNull
import org.junit.Assert.assertSame
import org.junit.Test

class NativeDisplayRefreshLoopTest {

    private class ManualFrameScheduler : FrameScheduler {
        var pendingCallback: Choreographer.FrameCallback? = null
        var postCount: Int = 0
        var removeCount: Int = 0

        override fun postFrameCallback(callback: Choreographer.FrameCallback) {
            pendingCallback = callback
            postCount++
        }

        override fun removeFrameCallback(callback: Choreographer.FrameCallback) {
            if (pendingCallback === callback) {
                pendingCallback = null
            }
            removeCount++
        }

        fun runNextFrame(frameTimeNanos: Long) {
            val callback = pendingCallback ?: return
            pendingCallback = null
            callback.doFrame(frameTimeNanos)
        }
    }

    private fun buildSnapshot(softmenuId: Int, shiftF: Boolean = false): KeypadSnapshot {
        val meta = IntArray(324)
        meta[0] = if (shiftF) 1 else 0
        meta[5] = softmenuId
        meta[56] = 5
        return KeypadSnapshot.fromNative(meta, Array(43 * 5) { "K$it" })
    }

    @Test
    fun testBusyCoreNullMetadataSkipsKeypadRefreshWhileDeliveringLcdPixelsAndRecoversImmediately() {
        val scheduler = ManualFrameScheduler()
        var nowMs = 1_000L
        var currentMeta: IntArray? = intArrayOf(0, 0, 0, 0, 0, 10)
        val snapshotByMenu = mapOf(
            10 to buildSnapshot(softmenuId = 10, shiftF = false),
            20 to buildSnapshot(softmenuId = 20, shiftF = true),
        )
        var currentSnapshot: (IntArray) -> KeypadSnapshot? = { meta -> snapshotByMenu[meta[5]] }

        var displayPixelReads = 0
        var lcdUpdates = 0
        var snapshotRequests = 0
        val deliveredSnapshots = mutableListOf<KeypadSnapshot>()

        val refreshLoop = NativeDisplayRefreshLoop(
            isAppRunning = { true },
            isNativeInitialized = { true },
            getDisplayPixels = { pixels ->
                displayPixelReads++
                pixels[0] = displayPixelReads
            },
            getKeypadMetaNative = { _, _ -> currentMeta },
            useSceneDrivenKeypadProvider = { true },
            getKeypadSnapshot = { meta ->
                snapshotRequests++
                currentSnapshot(meta)
            },
            onLcdPixels = { pixels ->
                if (pixels[0] == displayPixelReads) {
                    lcdUpdates++
                }
            },
            onDynamicRefresh = { snapshot -> deliveredSnapshots.add(snapshot) },
            frameScheduler = scheduler,
            currentTimeMillis = { nowMs },
        )

        refreshLoop.start()
        assertEquals(1, scheduler.postCount)

        // Frame 1: initial idle state refreshes both LCD and dynamic keypad.
        scheduler.runNextFrame(16_000_000L)
        assertEquals(1, displayPixelReads)
        assertEquals(1, lcdUpdates)
        assertEquals(1, snapshotRequests)
        assertEquals(1, deliveredSnapshots.size)
        assertSame(snapshotByMenu.getValue(10), deliveredSnapshots.last())

        // Busy core (screenMutex locked): getKeypadMetaNative returns null across frames, even past 500 ms.
        currentMeta = null
        nowMs = 1_016L
        scheduler.runNextFrame(32_000_000L)
        nowMs = 1_650L
        scheduler.runNextFrame(48_000_000L)

        assertEquals(3, displayPixelReads)
        assertEquals(3, lcdUpdates)
        assertEquals("Snapshot builder must not run when metadata is null", 1, snapshotRequests)
        assertEquals("Dynamic keypad must not refresh when metadata is null", 1, deliveredSnapshots.size)

        // Core unlocks with updated keypad metadata: next frame refreshes immediately without waiting another 500 ms.
        currentMeta = intArrayOf(1, 0, 0, 0, 0, 20)
        nowMs = 1_666L
        scheduler.runNextFrame(64_000_000L)

        assertEquals(4, displayPixelReads)
        assertEquals(4, lcdUpdates)
        assertEquals(2, snapshotRequests)
        assertEquals(2, deliveredSnapshots.size)
        assertSame(snapshotByMenu.getValue(20), deliveredSnapshots.last())

        // Immediate subsequent frame (< 500 ms) with unchanged metadata skips redundant snapshot rebuild.
        nowMs = 1_682L
        scheduler.runNextFrame(80_000_000L)
        assertEquals(5, displayPixelReads)
        assertEquals(5, lcdUpdates)
        assertEquals("Unchanged metadata within 500 ms must not rebuild snapshot", 2, snapshotRequests)
        assertEquals(2, deliveredSnapshots.size)
    }

    @Test
    fun testNullSnapshotDuringTransientContentionDoesNotAdvanceLastKeypadMeta() {
        val scheduler = ManualFrameScheduler()
        var nowMs = 2_000L
        val metaInitial = intArrayOf(0, 0, 0, 0, 0, 1)
        val metaShifted = intArrayOf(1, 0, 0, 0, 0, 1)
        var currentMeta: IntArray? = metaInitial
        val snapshotInitial = buildSnapshot(softmenuId = 1, shiftF = false)
        val snapshotShifted = buildSnapshot(softmenuId = 1, shiftF = true)
        var allowLabels = true

        val deliveredSnapshots = mutableListOf<KeypadSnapshot>()

        val refreshLoop = NativeDisplayRefreshLoop(
            isAppRunning = { true },
            isNativeInitialized = { true },
            getDisplayPixels = {},
            getKeypadMetaNative = { _, _ -> currentMeta },
            useSceneDrivenKeypadProvider = { true },
            getKeypadSnapshot = { meta ->
                if (!allowLabels) {
                    null
                } else if (meta[0] == 1) {
                    snapshotShifted
                } else {
                    snapshotInitial
                }
            },
            onLcdPixels = {},
            onDynamicRefresh = { snapshot -> deliveredSnapshots.add(snapshot) },
            frameScheduler = scheduler,
            currentTimeMillis = { nowMs },
        )

        refreshLoop.start()
        scheduler.runNextFrame(16_000_000L)
        assertEquals(1, deliveredSnapshots.size)
        assertSame(snapshotInitial, deliveredSnapshots[0])

        // Metadata changes to shifted state, then core locks screenMutex before getKeypadLabelsNative runs -> null snapshot.
        currentMeta = metaShifted
        allowLabels = false
        nowMs = 2_016L
        scheduler.runNextFrame(32_000_000L)
        assertEquals("Null snapshot must skip dynamic refresh", 1, deliveredSnapshots.size)

        // Next frame 16 ms later (< 500 ms) with identical metaShifted and labels now available must refresh immediately.
        allowLabels = true
        nowMs = 2_032L
        scheduler.runNextFrame(48_000_000L)
        assertEquals("Preserved lastKeypadMeta must trigger immediate retry on next frame", 2, deliveredSnapshots.size)
        assertSame(snapshotShifted, deliveredSnapshots[1])
    }

    @Test
    fun testStopRemovesCallbackAndIgnoresLateFrame() {
        val scheduler = ManualFrameScheduler()
        var lcdUpdates = 0

        val refreshLoop = NativeDisplayRefreshLoop(
            isAppRunning = { true },
            isNativeInitialized = { true },
            getDisplayPixels = {},
            getKeypadMetaNative = { _, _ -> null },
            useSceneDrivenKeypadProvider = { true },
            getKeypadSnapshot = { null },
            onLcdPixels = { lcdUpdates++ },
            onDynamicRefresh = {},
            frameScheduler = scheduler,
        )

        refreshLoop.start()
        val capturedCallback = scheduler.pendingCallback
        refreshLoop.stop()

        assertEquals(1, scheduler.removeCount)
        assertNull(scheduler.pendingCallback)

        capturedCallback?.doFrame(16_000_000L)
        assertEquals(0, lcdUpdates)
        assertNull(scheduler.pendingCallback)
    }

    @Test
    fun testInvalidateKeypadCacheForcesImmediateRefreshOnNextFrameEvenWhenMetaUnchanged() {
        val scheduler = ManualFrameScheduler()
        var nowMs = 3_000L
        val steadyMeta = intArrayOf(0, 0, 0, 0, 0, 10)
        var currentMeta: IntArray? = steadyMeta
        val snapshotA = buildSnapshot(softmenuId = 10, shiftF = false)
        val snapshotB = buildSnapshot(softmenuId = 10, shiftF = true)
        var activeSnapshot = snapshotA
        val deliveredSnapshots = mutableListOf<KeypadSnapshot>()

        val refreshLoop = NativeDisplayRefreshLoop(
            isAppRunning = { true },
            isNativeInitialized = { true },
            getDisplayPixels = {},
            getKeypadMetaNative = { _, _ -> currentMeta },
            useSceneDrivenKeypadProvider = { true },
            getKeypadSnapshot = { activeSnapshot },
            onLcdPixels = {},
            onDynamicRefresh = { snapshot -> deliveredSnapshots.add(snapshot) },
            frameScheduler = scheduler,
            currentTimeMillis = { nowMs },
        )

        refreshLoop.start()
        scheduler.runNextFrame(16_000_000L)
        assertEquals(1, deliveredSnapshots.size)
        assertSame(snapshotA, deliveredSnapshots[0])

        // Simulate keypad rebuild or preference change while screenMutex is locked (currentMeta == null).
        currentMeta = null
        activeSnapshot = snapshotB
        refreshLoop.invalidateKeypadCache()
        nowMs = 3_016L
        scheduler.runNextFrame(32_000_000L)
        assertEquals(1, deliveredSnapshots.size)

        // Core unlocks with identical metadata < 500 ms later: invalidated cache must trigger immediate refresh.
        currentMeta = steadyMeta
        nowMs = 3_032L
        scheduler.runNextFrame(48_000_000L)
        assertEquals(2, deliveredSnapshots.size)
        assertSame(snapshotB, deliveredSnapshots[1])
    }
}
