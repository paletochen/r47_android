package org.rpncalculators.r47

import android.content.Context
import android.graphics.Bitmap
import android.graphics.Canvas
import androidx.test.core.app.ApplicationProvider
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Before
import org.junit.Ignore
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner
import org.robolectric.annotation.Config
import org.robolectric.annotation.GraphicsMode

@Ignore("Requires Robolectric native graphics runtime which is unavailable on headless Linux")
@RunWith(RobolectricTestRunner::class)
@Config(sdk = [34])
@GraphicsMode(GraphicsMode.Mode.LEGACY)
class ReplicaOverlayGoldenTest {

    private lateinit var context: Context
    private lateinit var overlay: ReplicaOverlay

    @Before
    fun setUp() {
        context = ApplicationProvider.getApplicationContext()
        overlay = ReplicaOverlay(context)
    }

    @Test
    fun testOverlayInit() {
        assertNotNull(overlay)
    }

    @Test
    fun testLayoutMeasure() {
        overlay.measure(1080, 1920)
        overlay.layout(0, 0, 1080, 1920)
        assertEquals(1080, overlay.width)
        assertEquals(1920, overlay.height)
    }

    @Test
    fun testDraw() {
        overlay.measure(1080, 1920)
        overlay.layout(0, 0, 1080, 1920)
        
        val bitmap = Bitmap.createBitmap(1080, 1920, Bitmap.Config.ARGB_8888)
        val canvas = Canvas(bitmap)
        overlay.draw(canvas)
        
        // Just verify it doesn't crash and produces a bitmap
        assertNotNull(bitmap)
        assertEquals(1080, bitmap.width)
        assertEquals(1920, bitmap.height)
    }
}
