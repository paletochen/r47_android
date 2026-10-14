package org.rpncalculators.r47

import android.content.Context
import android.graphics.*
import android.util.AttributeSet
import android.util.TypedValue
import android.view.GestureDetector
import android.view.MotionEvent
import android.view.View
import android.view.ViewGroup
import android.util.Log
import kotlin.math.roundToInt

class ReplicaOverlay @JvmOverloads constructor(
    context: Context, attrs: AttributeSet? = null, defStyleAttr: Int = 0
) : ViewGroup(context, attrs, defStyleAttr) {

    companion object {
        const val CHROME_MODE_NATIVE = "native"
        const val CHROME_MODE_TEXTURE = "r47_texture"
        const val CHROME_MODE_BACKGROUND = "r47_background"
        const val CHROME_MODE_DAS_KALKULATOR = "das_kalkulator"
        const val CHROME_MODE_SIMULATOR = "r47_background_v2"
    }



    private var isPiPMode = false
    private var showTouchZones = false
    private val chromeLayout = ReplicaChromeLayout(resources)

    private val lcdBitmap = Bitmap.createBitmap(
        R47LcdContract.PIXEL_WIDTH,
        R47LcdContract.PIXEL_HEIGHT,
        Bitmap.Config.ARGB_8888,
    )
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
        chromeLayout.setScalingMode(mode)
        requestLayout()
        invalidate()
    }

    fun setShowTouchZones(show: Boolean) {
        showTouchZones = show
        invalidate()
    }

    fun setChromeMode(mode: String) {
        if (!chromeLayout.setChromeMode(mode)) {
            return
        }
        requestLayout()
        invalidate()
    }

    fun setNativeChrome() {
        setChromeMode(CHROME_MODE_NATIVE)
    }

    private fun currentChromeSpec(): ReplicaChromeSpec = chromeLayout.currentChromeSpec()

    fun isPointInLcd(x: Float, y: Float): Boolean {
        if (width <= 0 || height <= 0) {
            return false
        }

        val spec = currentChromeSpec()
        val projection = chromeLayout.computeProjection(spec, width.toFloat(), height.toFloat())
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
        val projection = chromeLayout.computeProjection(spec, width.toFloat(), height.toFloat())

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

        val projection = chromeLayout.computeProjection(width.toFloat(), height.toFloat())
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

        val projection = chromeLayout.computeProjection(width.toFloat(), height.toFloat())
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

    override fun onMeasure(widthMeasureSpec: Int, heightMeasureSpec: Int) {
        val w = MeasureSpec.getSize(widthMeasureSpec)
        val h = MeasureSpec.getSize(heightMeasureSpec)
        setMeasuredDimension(w, h)

        if (!isPiPMode) {
            val projection = chromeLayout.computeProjection(w.toFloat(), h.toFloat())
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
        val w = (r - l).toFloat()
        val h = (b - t).toFloat()

        if (isPiPMode) {
            for (i in 0 until childCount) {
                getChildAt(i).layout(0, 0, 0, 0)
            }
            return
        }

        val projection = chromeLayout.computeProjection(w, h)

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
        val projection = chromeLayout.computeProjection(layoutSpec, w, h)

        shellRect.set(
            projection.offsetX,
            projection.offsetY,
            projection.offsetX + layoutSpec.shellWidth * projection.scale,
            projection.offsetY + layoutSpec.shellHeight * projection.scale,
        )
        chromeLayout.drawShellBackground(
            canvas,
            layoutSpec,
            shellRect,
            projection.scale,
            bodyPaint,
            paint,
        )

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
