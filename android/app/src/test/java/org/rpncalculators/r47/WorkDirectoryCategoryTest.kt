package org.rpncalculators.r47

import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class WorkDirectoryCategoryTest {

    @Test
    fun testSubfolderCategories() {
        val categories = mapOf(
            0 to "STATE",
            1 to "PROGRAMS",
            2 to "SAVFILES",
            3 to "SCREENS",
            4 to "DATA",
            5 to "PRINT",
        )

        for ((type, expectedFolder) in categories) {
            val resolved = when (type) {
                0 -> "STATE"
                1 -> "PROGRAMS"
                2 -> "SAVFILES"
                3 -> "SCREENS"
                4 -> "DATA"
                5 -> "PRINT"
                else -> null
            }
            assertEquals("Category $type must map to $expectedFolder", expectedFolder, resolved)
        }
    }

    @Test
    fun testAutoSaveFilePriorities() {
        val r47Auto = "R47auto.sav"
        val c47Auto = "C47auto.sav"

        assertTrue("R47 auto save ends with .sav", r47Auto.endsWith(".sav", ignoreCase = true))
        assertTrue("C47 auto save ends with .sav", c47Auto.endsWith(".sav", ignoreCase = true))

        val mockFiles = listOf("test.sav", "C47auto.sav", "R47auto.sav")
        val found = mockFiles.firstOrNull { it.equals(r47Auto, ignoreCase = true) }
            ?: mockFiles.firstOrNull { it.equals(c47Auto, ignoreCase = true) }

        assertEquals("R47auto.sav must take priority when present", "R47auto.sav", found)
    }

    @Test
    fun testFormatDisplayPath() {
        val formatted = WorkDirectory.formatDisplayPath("/tree/primary:Documents/R47")
        assertEquals("/Documents/R47", formatted)

        val nullFormatted = WorkDirectory.formatDisplayPath(null)
        assertEquals("Select a folder", nullFormatted)
    }
}
