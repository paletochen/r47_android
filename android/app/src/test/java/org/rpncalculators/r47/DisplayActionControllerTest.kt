package org.rpncalculators.r47

import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test
import java.util.concurrent.atomic.AtomicBoolean
import java.util.concurrent.atomic.AtomicInteger
import java.util.concurrent.atomic.AtomicReference

class DisplayActionControllerTest {

    @Test
    fun testEnterPiPCallback() {
        val pipInvoked = AtomicBoolean(false)
        val enterPiPAction = { pipInvoked.set(true) }

        enterPiPAction.invoke()
        assertTrue("enterPiP must be invoked when requested", pipInvoked.get())
    }

    @Test
    fun testCopyXRegisterAction() {
        val expectedRegisterValue = "123.456789"
        val copiedValue = AtomicReference<String>()
        val getXNative = { expectedRegisterValue }

        val taskQueue = mutableListOf<Runnable>()
        val offerTask: (Runnable) -> Unit = { taskQueue.add(it) }

        offerTask(Runnable {
            copiedValue.set(getXNative())
        })

        assertEquals(1, taskQueue.size)
        taskQueue[0].run()
        assertEquals(expectedRegisterValue, copiedValue.get())
    }

    @Test
    fun testCopyAllRegistersAction() {
        val expectedRegistersDump = "Z = 0\nY = 0\nX = 123.45\nR00 = 0"
        val copiedValue = AtomicReference<String>()
        val getAllNative = { expectedRegistersDump }

        val taskQueue = mutableListOf<Runnable>()
        val offerTask: (Runnable) -> Unit = { taskQueue.add(it) }

        offerTask(Runnable {
            copiedValue.set(getAllNative())
        })

        assertEquals(1, taskQueue.size)
        taskQueue[0].run()
        assertEquals(expectedRegistersDump, copiedValue.get())
    }
}
