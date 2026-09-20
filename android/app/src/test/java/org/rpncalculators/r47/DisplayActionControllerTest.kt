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
    fun testPasteNumberMapping() {
        val sentKeys = mutableListOf<String>()
        val sentFuncs = mutableListOf<Int>()

        val input = "12.5i"
        for (char in input) {
            if (char == 'i' || char == 'j') {
                sentFuncs.add(if (char == 'i') 1159 else 1160)
                continue
            }
            val simId = when (char) {
                '1' -> "28"
                '2' -> "29"
                '.' -> "34"
                '5' -> "24"
                else -> null
            }
            if (simId != null) {
                sentKeys.add(simId)
            }
        }

        assertEquals(listOf("28", "29", "34", "24"), sentKeys)
        assertEquals(listOf(1159), sentFuncs)
    }
}
