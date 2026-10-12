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
        // 1. Complex number 12.5i
        val complexSteps = DisplayActionController.parsePasteSteps("12.5i")
        val complexKeys = complexSteps.filter { it.action == PasteAction.Key }.map { it.id }
        val complexFuncs = complexSteps.filter { it.action == PasteAction.Func }.map { it.funcId }
        assertEquals(listOf("28", "29", "34", "24"), complexKeys)
        assertEquals(listOf(1159), complexFuncs)

        // 2. Negative decimal number -0.7666
        val negSteps = DisplayActionController.parsePasteSteps("-0.7666")
        val negKeys = negSteps.filter { it.action == PasteAction.Key }.map { it.id }
        // Digits 0, ., 7, 6, 6, 6 then CHS (35)
        assertEquals(listOf("33", "34", "18", "25", "25", "25", "35"), negKeys)

        // 3. Degree angle: 86°
        val degSteps = DisplayActionController.parsePasteSteps("86°")
        val degKeys = degSteps.filter { it.action == PasteAction.Key }.map { it.id }
        val degFuncs = degSteps.filter { it.action == PasteAction.Func }.map { it.funcId }
        assertEquals(listOf("19", "25"), degKeys)
        assertEquals(listOf(115), degFuncs)

        // 4. Negative with scientific exponent: -1.2e-5
        val sciSteps = DisplayActionController.parsePasteSteps("-1.2e-5")
        val sciKeys = sciSteps.filter { it.action == PasteAction.Key }.map { it.id }
        // 1, ., 2, CHS (35), EEX (15), 5, CHS (35)
        assertEquals(listOf("28", "34", "29", "35", "15", "24", "35"), sciKeys)
    }
}
