package org.rpncalculators.r47

import android.content.res.Resources
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.Canvas
import android.graphics.Paint
import android.graphics.RectF
import kotlin.math.min

internal data class ReplicaProjection(val scale: Float, val offsetX: Float, val offsetY: Float)

internal data class ReplicaChromeSpec(
    val mode: String,
    val shellWidth: Float,
    val shellHeight: Float,
    val topBezelSettingsTapHeight: Float,
    val lcdWindowLeft: Float,
    val lcdWindowTop: Float,
    val lcdWindowWidth: Float,
    val lcdWindowHeight: Float,
    val scaledModeFitTrimLeft: Float = 0f,
    val scaledModeFitTrimTop: Float = 0f,
    val scaledModeFitTrimRight: Float = 0f,
    val scaledModeFitTrimBottom: Float = 0f,
    val imageResId: Int? = null,
)

internal class ReplicaChromeLayout(
    private val resources: Resources,
) {
    private val baseChromeSpec = ReplicaChromeSpec(
        mode = ReplicaOverlay.CHROME_MODE_NATIVE,
        shellWidth = R47ReferenceGeometry.LOGICAL_CANVAS_WIDTH,
        shellHeight = R47ReferenceGeometry.LOGICAL_CANVAS_HEIGHT,
        topBezelSettingsTapHeight = R47AndroidChromeGeometry.TOP_BEZEL_SETTINGS_TAP_HEIGHT,
        lcdWindowLeft = R47AndroidChromeGeometry.LCD_WINDOW_LEFT,
        lcdWindowTop = R47AndroidChromeGeometry.LCD_WINDOW_TOP,
        lcdWindowWidth = R47AndroidChromeGeometry.LCD_WINDOW_WIDTH,
        lcdWindowHeight = R47AndroidChromeGeometry.LCD_WINDOW_HEIGHT,
        scaledModeFitTrimLeft = R47AndroidChromeGeometry.SCALED_MODE_FIT_TRIM_LEFT,
        scaledModeFitTrimTop = R47AndroidChromeGeometry.SCALED_MODE_FIT_TRIM_TOP,
        scaledModeFitTrimRight = R47AndroidChromeGeometry.SCALED_MODE_FIT_TRIM_RIGHT,
        scaledModeFitTrimBottom = R47AndroidChromeGeometry.SCALED_MODE_FIT_TRIM_BOTTOM,
    )
    private val nativeChromeSpec = baseChromeSpec.copy(
        mode = ReplicaOverlay.CHROME_MODE_NATIVE,
    )
    private val backgroundChromeSpec = baseChromeSpec.copy(
        mode = ReplicaOverlay.CHROME_MODE_BACKGROUND,
        imageResId = R.drawable.r47_background,
    )
    private val textureChromeSpec = baseChromeSpec.copy(
        mode = ReplicaOverlay.CHROME_MODE_TEXTURE,
        imageResId = R.drawable.r47_texture,
    )
    private val dasKalkulatorChromeSpec = baseChromeSpec.copy(
        mode = ReplicaOverlay.CHROME_MODE_DAS_KALKULATOR,
        imageResId = R.drawable.r47_das_kalkulator,
    )



    private val chromeBitmapCache = mutableMapOf<Int, Bitmap?>()
    private var resolvedShellBitmapWidthCache: Float? = null
    private var chromeMode = ReplicaOverlay.CHROME_MODE_NATIVE
    private var scalingMode = "full_width"

    fun setChromeMode(mode: String): Boolean {
        val resolvedMode = resolveChromeSpec(mode).mode
        if (chromeMode == resolvedMode) {
            return false
        }

        chromeMode = resolvedMode
        return true
    }

    fun setScalingMode(mode: String) {
        scalingMode = mode
    }

    fun currentChromeSpec(): ReplicaChromeSpec {
        return resolveChromeSpec(chromeMode)
    }

    fun computeProjection(availableWidth: Float, availableHeight: Float): ReplicaProjection {
        return computeProjection(currentChromeSpec(), availableWidth, availableHeight)
    }

    fun computeProjection(
        spec: ReplicaChromeSpec,
        availableWidth: Float,
        availableHeight: Float,
    ): ReplicaProjection {
        val fitLeft = if (scalingMode == "physical") 0f else spec.scaledModeFitTrimLeft
        val fitTop = if (scalingMode == "physical") 0f else spec.scaledModeFitTrimTop
        val fitWidth = if (scalingMode == "physical") {
            spec.shellWidth
        } else {
            spec.shellWidth - spec.scaledModeFitTrimLeft - spec.scaledModeFitTrimRight
        }
        val fitHeight = if (scalingMode == "physical") {
            spec.shellHeight
        } else {
            spec.shellHeight - spec.scaledModeFitTrimTop - spec.scaledModeFitTrimBottom
        }
        val fitScale = min(availableWidth / fitWidth, availableHeight / fitHeight)
        val scale = if (scalingMode == "physical") {
            val oneToOneProjectionScaleCap =
                resolvedShellBitmapWidthForCurrentDensity() /
                    R47ReferenceGeometry.LOGICAL_CANVAS_WIDTH
            min(oneToOneProjectionScaleCap, fitScale)
        } else {
            fitScale
        }
        val offsetX = (availableWidth - fitWidth * scale) / 2f - fitLeft * scale
        val offsetY = (availableHeight - fitHeight * scale) / 2f - fitTop * scale
        return ReplicaProjection(scale, offsetX, offsetY)
    }

    fun drawShellBackground(
        canvas: Canvas,
        spec: ReplicaChromeSpec,
        rect: RectF,
        projectionScale: Float,
        bodyPaint: Paint,
        bitmapPaint: Paint,
    ) {
        val backgroundBitmap = chromeBitmapFor(spec)
        if (backgroundBitmap != null) {
            canvas.drawBitmap(backgroundBitmap, null, rect, bitmapPaint)
            return
        }

        val cornerRadius =
            R47AndroidChromeGeometry.NATIVE_SHELL_DRAW_CORNER_RADIUS * projectionScale
        canvas.drawRoundRect(rect, cornerRadius, cornerRadius, bodyPaint)
    }

    private fun resolveChromeSpec(mode: String): ReplicaChromeSpec {
        return when {
            mode == ReplicaOverlay.CHROME_MODE_TEXTURE -> textureChromeSpec
            mode == ReplicaOverlay.CHROME_MODE_DAS_KALKULATOR -> dasKalkulatorChromeSpec
            mode == "image" -> backgroundChromeSpec
            mode.startsWith(ReplicaOverlay.CHROME_MODE_BACKGROUND) -> backgroundChromeSpec
            else -> nativeChromeSpec
        }
    }


    private fun chromeBitmapFor(spec: ReplicaChromeSpec): Bitmap? {
        val resId = spec.imageResId ?: return null
        return chromeBitmapCache.getOrPut(resId) {
            BitmapFactory.decodeResource(resources, resId)
        }
    }

    private fun decodeResourceWidth(resId: Int): Float? {
        val options = BitmapFactory.Options().apply {
            inJustDecodeBounds = true
        }
        BitmapFactory.decodeResource(resources, resId, options)
        return options.outWidth.takeIf { it > 0 }?.toFloat()
    }

    private fun resolvedShellBitmapWidthForCurrentDensity(): Float {
        resolvedShellBitmapWidthCache?.let { return it }

        val widths = listOfNotNull(
            decodeResourceWidth(R.drawable.r47_background),
            decodeResourceWidth(R.drawable.r47_texture),
        ).distinct()

        val resolvedWidth = widths.firstOrNull() ?: R47ReferenceGeometry.LOGICAL_CANVAS_WIDTH
        resolvedShellBitmapWidthCache = resolvedWidth
        return resolvedWidth
    }
}
