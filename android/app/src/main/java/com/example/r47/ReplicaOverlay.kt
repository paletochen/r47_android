package com.example.r47

import android.content.Context
import android.graphics.*
import android.util.AttributeSet
import android.view.GestureDetector
import android.view.MotionEvent
import android.view.View
import android.view.ViewGroup
import android.util.Log

class ReplicaOverlay @JvmOverloads constructor(
    context: Context, attrs: AttributeSet? = null, defStyleAttr: Int = 0
) : ViewGroup(context, attrs, defStyleAttr) {

    private val baseWidth = 537f
    private val baseHeight = 1005f
    private var isPiPMode = false
    private var currentSkin: Bitmap? = null
    private var scalingMode = "full_width"
    private var showTouchZones = false

    private val lcdBitmap = Bitmap.createBitmap(400, 240, Bitmap.Config.ARGB_8888)
    private val lastLcdPixels = IntArray(400 * 240)
    private val lcdRect = Rect(0, 0, 400, 240)
    private val lcdDestRect = RectF()
    private val dirtyRect = Rect()
    private val paint = Paint(Paint.FILTER_BITMAP_FLAG)
    private val zonePaint = Paint().apply {
        color = Color.RED
        style = Paint.Style.STROKE
        strokeWidth = 2f * resources.displayMetrics.density
        alpha = 180
    }

    var onPiPKeyEvent: ((Int) -> Unit)? = null
    var onLongPressListener: ((Float, Float) -> Unit)? = null
    var onSettingsTapListener: (() -> Unit)? = null

    private val gestureDetector = GestureDetector(context, object : GestureDetector.SimpleOnGestureListener() {
        override fun onLongPress(e: MotionEvent) {
            onLongPressListener?.invoke(e.x, e.y)
        }
    })

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

    fun setSkin(skinName: String) {
        try {
            val resId = context.resources.getIdentifier(skinName, "drawable", context.packageName)
            currentSkin = BitmapFactory.decodeResource(resources, resId)
            invalidate()
        } catch (e: Exception) {
            Log.e("ReplicaOverlay", "Failed to load skin $skinName", e)
        }
    }

    fun updateLcd(pixels: IntArray) {
        var minX = 400
        var maxX = 0
        var minY = 240
        var maxY = 0
        var changed = false

        for (i in pixels.indices) {
            if (pixels[i] != lastLcdPixels[i]) {
                val x = i % 400
                val y = i / 400
                if (x < minX) minX = x
                if (x > maxX) maxX = x
                if (y < minY) minY = y
                if (y > maxY) maxY = y
                lastLcdPixels[i] = pixels[i]
                changed = true
            }
        }

        if (!changed) return

        lcdBitmap.setPixels(pixels, 0, 400, 0, 0, 400, 240)

        // Calculate screen-space dirty rect
        val w = width.toFloat()
        val scale = getScale(w)
        val offsetX = (w - baseWidth * scale) / 2f
        val offsetY = (height - baseHeight * scale) / 2f

        // Convert LCD coordinates (400x240) to Screen coordinates
        val left = offsetX + (25.5f + (minX.toFloat() / 400f) * 486f) * scale
        val top = offsetY + (67.5f + (minY.toFloat() / 240f) * 266.7f) * scale
        val right = offsetX + (25.5f + ((maxX.toFloat() + 1) / 400f) * 486f) * scale
        val bottom = offsetY + (67.5f + ((maxY.toFloat() + 1) / 240f) * 266.7f) * scale

        dirtyRect.set(left.toInt(), top.toInt(), right.toInt(), bottom.toInt())
        invalidate()
    }

    private fun getScale(w: Float): Float {
        return if (scalingMode == "physical") {
            val metrics = resources.displayMetrics
            val dpi = metrics.xdpi
            (2.83f * dpi) / baseWidth 
        } else {
            w / baseWidth
        }
    }

    override fun onInterceptTouchEvent(ev: MotionEvent): Boolean {
        if (isPiPMode) return false
        gestureDetector.onTouchEvent(ev)
        
        val scale = getScale(width.toFloat())
        val offsetY = (height - baseHeight * scale) / 2f
        val lY = (ev.y - offsetY) / scale
        
        // Intercept touches in the settings area (top bezel)
        if (lY < 67.5f && lY > 0) {
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
        
        val scale = getScale(width.toFloat())
        val offsetY = (height - baseHeight * scale) / 2f
        val lY = (event.y - offsetY) / scale
        
        // If we intercepted this (or no one else took it), and it's in the bezel area
        if (lY < 67.5f && lY > 0) {
            if (event.action == MotionEvent.ACTION_UP) {
                Log.i("ReplicaOverlay", "Settings area tap confirmed on UP")
                onSettingsTapListener?.invoke()
            }
            return true // Always consume touches in the intercept zone
        }
        
        return super.onTouchEvent(event)
    }

    class LayoutParams(val x: Float, val y: Float, val w: Float, val h: Float) : ViewGroup.LayoutParams(0, 0)

    fun addReplicaView(view: View, x: Float, y: Float, w: Float, h: Float) {
        addView(view, LayoutParams(x, y, w, h))
    }

    override fun onMeasure(widthMeasureSpec: Int, heightMeasureSpec: Int) {
        val w = MeasureSpec.getSize(widthMeasureSpec)
        val h = MeasureSpec.getSize(heightMeasureSpec)
        setMeasuredDimension(w, h)

        if (!isPiPMode) {
            val scale = getScale(w.toFloat())
            for (i in 0 until childCount) {
                val child = getChildAt(i)
                val lp = child.layoutParams as LayoutParams
                child.measure(
                    MeasureSpec.makeMeasureSpec((lp.w * scale).toInt(), MeasureSpec.EXACTLY),
                    MeasureSpec.makeMeasureSpec((lp.h * scale).toInt(), MeasureSpec.EXACTLY)
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
        val scale = getScale(w)
        val offsetX = (w - baseWidth * scale) / 2f
        val offsetY = (h - baseHeight * scale) / 2f

        for (i in 0 until childCount) {
            val child = getChildAt(i)
            val lp = child.layoutParams as LayoutParams
            val left = (lp.x * scale + offsetX).toInt()
            val top = (lp.y * scale + offsetY).toInt()
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

        val scale = getScale(w)
        val offsetX = (w - baseWidth * scale) / 2f
        val offsetY = (h - baseHeight * scale) / 2f

        currentSkin?.let {
            val src = Rect(0, 0, it.width, it.height)
            val dst = RectF(offsetX, offsetY, offsetX + baseWidth * scale, offsetY + baseHeight * scale)
            canvas.drawBitmap(it, src, dst, paint)
        }

        lcdDestRect.set(
            offsetX + 25.5f * scale,
            offsetY + 67.5f * scale,
            offsetX + 511.5f * scale,
            offsetY + 334.2f * scale
        )
        canvas.drawBitmap(lcdBitmap, lcdRect, lcdDestRect, paint)

        if (showTouchZones) {
            for (i in 0 until childCount) {
                val child = getChildAt(i)
                canvas.drawRect(
                    child.left.toFloat(), child.top.toFloat(),
                    child.right.toFloat(), child.bottom.toFloat(),
                    zonePaint
                )
            }
            // Show settings zone
            canvas.drawRect(offsetX, offsetY, offsetX + baseWidth * scale, offsetY + 67.5f * scale, zonePaint)
        }

        super.dispatchDraw(canvas)
    }
}