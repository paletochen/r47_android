package com.example.r47

import android.content.Context
import android.graphics.*
import android.util.AttributeSet
import android.util.TypedValue
import android.view.GestureDetector
import android.view.MotionEvent
import android.view.View
import android.view.ViewGroup
import android.util.Log
import kotlin.math.min
import kotlin.math.roundToInt

class ReplicaOverlay @JvmOverloads constructor(
    context: Context, attrs: AttributeSet? = null, defStyleAttr: Int = 0
) : ViewGroup(context, attrs, defStyleAttr) {

    companion object {
        const val CHROME_MODE_NATIVE = "native"
        const val CHROME_MODE_TEXTURE = "r47_texture"
        const val CHROME_MODE_BACKGROUND = "r47_background"
    }

    private data class Projection(val scale: Float, val offsetX: Float, val offsetY: Float)
    private data class ChromeSpec(
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
        val drawNativeChrome: Boolean = false,
    )

    private val baseChromeSpec = ChromeSpec(
        mode = CHROME_MODE_NATIVE,
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
        mode = CHROME_MODE_NATIVE,
        drawNativeChrome = true,
    )
    private val backgroundChromeSpec = baseChromeSpec.copy(
        mode = CHROME_MODE_BACKGROUND,
        imageResId = R.drawable.r47_background,
    )
    private val textureChromeSpec = baseChromeSpec.copy(
        mode = CHROME_MODE_TEXTURE,
        imageResId = R.drawable.r47_texture,
    )

    private var isPiPMode = false
    private var chromeMode = CHROME_MODE_NATIVE
    private var scalingMode = "full_width"
    private var showTouchZones = false

    private val lcdBitmap = Bitmap.createBitmap(
        R47LcdContract.PIXEL_WIDTH,
        R47LcdContract.PIXEL_HEIGHT,
        Bitmap.Config.ARGB_8888,
    )
    private val chromeBitmapCache = mutableMapOf<Int, Bitmap?>()
    private var resolvedShellBitmapWidthCache: Float? = null
    private val lastLcdPixels = IntArray(R47LcdContract.PIXEL_COUNT)
    private val lcdRect = Rect(0, 0, R47LcdContract.PIXEL_WIDTH, R47LcdContract.PIXEL_HEIGHT)
    private val lcdDestRect = RectF()
    private val dirtyRect = Rect()
    private val paint = Paint(Paint.FILTER_BITMAP_FLAG)
    private val shellRect = RectF()
    private val bodyPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = Color.rgb(32, 32, 32)
    }
    private val zonePaint = Paint().apply {
        color = Color.RED
        style = Paint.Style.STROKE
        strokeWidth = 2f * resources.displayMetrics.density
        alpha = 180
    }

    var onPiPKeyEvent: ((Int) -> Unit)? = null
    var onLongPressListener: ((Float, Float) -> Unit)? = null
    var onSettingsTapListener: (() -> Unit)? = null
    
    private val gestureDetector: GestureDetector

    init {
        setBackgroundColor(Color.BLACK)
        // Allow drawing outside individual key boundaries
        clipChildren = false
        clipToPadding = false
        
        gestureDetector = GestureDetector(context, object : GestureDetector.SimpleOnGestureListener() {
            override fun onLongPress(e: MotionEvent) {
                onLongPressListener?.invoke(e.x, e.y)
            }
        })
    }

    private fun dp(value: Float): Float {
        return TypedValue.applyDimension(
            TypedValue.COMPLEX_UNIT_DIP,
            value,
            resources.displayMetrics,
        )
    }

    fun setPiPMode(enabled: Boolean) {
        isPiPMode = enabled
        requestLayout()
        invalidate()
    }

    fun setScalingMode(mode: String) {
        scalingMode = mode
        requestLayout()
        invalidate()
    }

    fun setShowTouchZones(show: Boolean) {
        showTouchZones = show
        invalidate()
    }

    fun setChromeMode(mode: String) {
        val resolvedMode = resolveChromeSpec(mode).mode
        if (chromeMode == resolvedMode) {
            return
        }
        chromeMode = resolvedMode
        requestLayout()
        invalidate()
    }

    fun setNativeChrome() {
        setChromeMode(CHROME_MODE_NATIVE)
    }

    private fun resolveChromeSpec(mode: String): ChromeSpec {
        return when {
            mode == CHROME_MODE_TEXTURE -> textureChromeSpec
            mode == "image" -> backgroundChromeSpec
            mode.startsWith(CHROME_MODE_BACKGROUND) -> backgroundChromeSpec
            else -> nativeChromeSpec
        }
    }

    private fun currentChromeSpec(): ChromeSpec {
        return resolveChromeSpec(chromeMode)
    }

    private fun chromeBitmapFor(spec: ChromeSpec): Bitmap? {
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
        if (widths.size > 1) {
            Log.w("ReplicaOverlay", "Shell drawable width mismatch: $widths")
        }

        resolvedShellBitmapWidthCache = resolvedWidth
        return resolvedWidth
    }

    fun isPointInLcd(x: Float, y: Float): Boolean {
        if (width <= 0 || height <= 0) {
            return false
        }

        val spec = currentChromeSpec()
        val projection = computeShellProjection(spec, width.toFloat(), height.toFloat())
        val localX = (x - projection.offsetX) / projection.scale
        val localY = (y - projection.offsetY) / projection.scale

        return localX >= spec.lcdWindowLeft &&
            localX <= spec.lcdWindowLeft + spec.lcdWindowWidth &&
            localY >= spec.lcdWindowTop &&
            localY <= spec.lcdWindowTop + spec.lcdWindowHeight
    }

    fun updateLcd(pixels: IntArray) {
        val pixelWidth = R47LcdContract.PIXEL_WIDTH
        val pixelHeight = R47LcdContract.PIXEL_HEIGHT
        val pixelWidthF = pixelWidth.toFloat()
        val pixelHeightF = pixelHeight.toFloat()

        var minX = pixelWidth
        var maxX = 0
        var minY = pixelHeight
        var maxY = 0
        var changed = false

        for (i in pixels.indices) {
            if (pixels[i] != lastLcdPixels[i]) {
                val x = i % pixelWidth
                val y = i / pixelWidth
                if (x < minX) minX = x
                if (x > maxX) maxX = x
                if (y < minY) minY = y
                if (y > maxY) maxY = y
                lastLcdPixels[i] = pixels[i]
                changed = true
            }
        }

        if (!changed) return

    lcdBitmap.setPixels(pixels, 0, pixelWidth, 0, 0, pixelWidth, pixelHeight)

        // Calculate screen-space dirty rect
        val spec = currentChromeSpec()
        val projection = computeShellProjection(spec, width.toFloat(), height.toFloat())

        // Convert LCD coordinates to Screen coordinates
        val left = projection.offsetX + (spec.lcdWindowLeft + (minX.toFloat() / pixelWidthF) * spec.lcdWindowWidth) * projection.scale
        val top = projection.offsetY + (spec.lcdWindowTop + (minY.toFloat() / pixelHeightF) * spec.lcdWindowHeight) * projection.scale
        val right = projection.offsetX + (spec.lcdWindowLeft + ((maxX.toFloat() + 1f) / pixelWidthF) * spec.lcdWindowWidth) * projection.scale
        val bottom = projection.offsetY + (spec.lcdWindowTop + ((maxY.toFloat() + 1f) / pixelHeightF) * spec.lcdWindowHeight) * projection.scale

        dirtyRect.set(left.toInt(), top.toInt(), right.toInt(), bottom.toInt())
        postInvalidateOnAnimation(dirtyRect.left, dirtyRect.top, dirtyRect.right, dirtyRect.bottom)
    }

    override fun onInterceptTouchEvent(ev: MotionEvent): Boolean {
        if (isPiPMode) return false
        gestureDetector.onTouchEvent(ev)

        val projection = computeShellProjection(width.toFloat(), height.toFloat())
        val lY = (ev.y - projection.offsetY) / projection.scale
        val spec = currentChromeSpec()

        // Intercept touches in the settings area (top bezel)
        if (lY < spec.topBezelSettingsTapHeight && lY > 0) {
            return true
        }

        return false
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        gestureDetector.onTouchEvent(event)

        if (isPiPMode) {
            val fKey = (event.x / width * 6).toInt() + 38
            if (event.action == MotionEvent.ACTION_DOWN) {
                onPiPKeyEvent?.invoke(fKey)
            } else if (event.action == MotionEvent.ACTION_UP || event.action == MotionEvent.ACTION_CANCEL) {
                onPiPKeyEvent?.invoke(0)
            }
            return true
        }

        val projection = computeShellProjection(width.toFloat(), height.toFloat())
        val lY = (event.y - projection.offsetY) / projection.scale
        val spec = currentChromeSpec()

        // If we intercepted this (or no one else took it), and it's in the bezel area
        if (lY < spec.topBezelSettingsTapHeight && lY > 0) {
            if (event.action == MotionEvent.ACTION_UP) {
                Log.i("ReplicaOverlay", "Settings area tap received")
                onSettingsTapListener?.invoke()
            }
            return true
        }

        return super.onTouchEvent(event)
    }

    class LayoutParams(
        val logicalX: Float,
        val logicalY: Float,
        val logicalWidth: Float,
        val logicalHeight: Float,
        val showTouchZone: Boolean = false,
    ) : ViewGroup.LayoutParams(0, 0)

    fun addReplicaView(
        view: View,
        x: Float,
        y: Float,
        w: Float,
        h: Float,
        showTouchZone: Boolean = false,
    ) {
        addView(view, LayoutParams(x, y, w, h, showTouchZone))
    }

    private fun computeShellProjection(spec: ChromeSpec, availableWidth: Float, availableHeight: Float): Projection {
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
                resolvedShellBitmapWidthForCurrentDensity() / R47ReferenceGeometry.LOGICAL_CANVAS_WIDTH
            min(oneToOneProjectionScaleCap, fitScale)
        } else {
            fitScale
        }
        val offsetX = (availableWidth - fitWidth * scale) / 2f - fitLeft * scale
        val offsetY = (availableHeight - fitHeight * scale) / 2f - fitTop * scale
        return Projection(scale, offsetX, offsetY)
    }

    private fun computeShellProjection(availableWidth: Float, availableHeight: Float): Projection {
        return computeShellProjection(currentChromeSpec(), availableWidth, availableHeight)
    }

    private fun drawNativeShellChrome(
        canvas: Canvas,
        rect: RectF,
        cornerRadius: Float,
    ) {
        canvas.drawRoundRect(rect, cornerRadius, cornerRadius, bodyPaint)
    }

    override fun onMeasure(widthMeasureSpec: Int, heightMeasureSpec: Int) {
        val w = MeasureSpec.getSize(widthMeasureSpec)
        val h = MeasureSpec.getSize(heightMeasureSpec)
        setMeasuredDimension(w, h)

        if (!isPiPMode) {
            val projection = computeShellProjection(w.toFloat(), h.toFloat())
            for (i in 0 until childCount) {
                val child = getChildAt(i)
                val lp = child.layoutParams as LayoutParams
                val childWidth = (lp.logicalWidth * projection.scale).roundToInt().coerceAtLeast(0)
                val childHeight = (lp.logicalHeight * projection.scale).roundToInt().coerceAtLeast(0)
                child.measure(
                    MeasureSpec.makeMeasureSpec(childWidth, MeasureSpec.EXACTLY),
                    MeasureSpec.makeMeasureSpec(childHeight, MeasureSpec.EXACTLY)
                )
            }
        }
    }

    override fun onLayout(changed: Boolean, l: Int, t: Int, r: Int, b: Int) {
        if (isPiPMode) {
            for (i in 0 until childCount) {
                getChildAt(i).layout(0, 0, 0, 0)
            }
            return
        }

        val w = (r - l).toFloat()
        val h = (b - t).toFloat()
        val projection = computeShellProjection(w, h)

        for (i in 0 until childCount) {
            val child = getChildAt(i)
            val lp = child.layoutParams as LayoutParams
            val left = (projection.offsetX + lp.logicalX * projection.scale).roundToInt()
            val top = (projection.offsetY + lp.logicalY * projection.scale).roundToInt()
            child.layout(left, top, left + child.measuredWidth, top + child.measuredHeight)
        }
    }

    override fun dispatchDraw(canvas: Canvas) {
        val w = width.toFloat()
        val h = height.toFloat()

        if (isPiPMode) {
            lcdDestRect.set(0f, 0f, w, h)
            canvas.drawBitmap(lcdBitmap, lcdRect, lcdDestRect, paint)
            return
        }

        val layoutSpec = currentChromeSpec()
        val projection = computeShellProjection(layoutSpec, w, h)

        shellRect.set(
            projection.offsetX,
            projection.offsetY,
            projection.offsetX + layoutSpec.shellWidth * projection.scale,
            projection.offsetY + layoutSpec.shellHeight * projection.scale,
        )
        val backgroundBitmap = chromeBitmapFor(layoutSpec)
        if (backgroundBitmap != null) {
            canvas.drawBitmap(backgroundBitmap, null, shellRect, paint)
        } else {
            drawNativeShellChrome(
                canvas = canvas,
                rect = shellRect,
                cornerRadius =
                    R47AndroidChromeGeometry.NATIVE_SHELL_DRAW_CORNER_RADIUS * projection.scale,
            )
        }

        lcdDestRect.set(
            projection.offsetX + layoutSpec.lcdWindowLeft * projection.scale,
            projection.offsetY + layoutSpec.lcdWindowTop * projection.scale,
            projection.offsetX + (layoutSpec.lcdWindowLeft + layoutSpec.lcdWindowWidth) * projection.scale,
            projection.offsetY + (layoutSpec.lcdWindowTop + layoutSpec.lcdWindowHeight) * projection.scale
        )
        canvas.drawBitmap(lcdBitmap, lcdRect, lcdDestRect, paint)

        if (showTouchZones) {
            for (i in 0 until childCount) {
                val child = getChildAt(i)
                val lp = child.layoutParams as? LayoutParams ?: continue
                if (!lp.showTouchZone) {
                    continue
                }
                canvas.drawRect(
                    child.left.toFloat(), child.top.toFloat(),
                    child.right.toFloat(), child.bottom.toFloat(),
                    zonePaint
                )
            }
            // Show settings zone
            canvas.drawRect(
                projection.offsetX,
                projection.offsetY,
                projection.offsetX + layoutSpec.shellWidth * projection.scale,
                projection.offsetY + layoutSpec.topBezelSettingsTapHeight * projection.scale,
                zonePaint,
            )
        }

        super.dispatchDraw(canvas)
    }
}
