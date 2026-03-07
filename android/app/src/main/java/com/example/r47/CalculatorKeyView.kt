package com.example.r47

import android.content.Context
import android.graphics.Color
import android.graphics.Typeface
import android.util.AttributeSet
import android.util.TypedValue
import android.view.Gravity
import android.view.View
import android.widget.TextView
import androidx.constraintlayout.widget.ConstraintLayout

class CalculatorKeyView @JvmOverloads constructor(
    context: Context, attrs: AttributeSet? = null, defStyleAttr: Int = 0
) : ConstraintLayout(context, attrs, defStyleAttr) {

    private val buttonView = View(context)
    val primaryLabel = TextView(context)
    val fLabel = TextView(context)
    val gLabel = TextView(context)
    val alphaLabel = TextView(context)
    val letterLabel = TextView(context)
    
    var keyCode: Int = 0
    private var isFnKey: Boolean = false
    private var baseMainSize = 0f
    private var isShiftedState = false
    private var lastAlphaStateForLayout: Boolean? = null

    init {
        // Critical: Allow drawing outside bounds
        clipChildren = false
        clipToPadding = false
        
        // Letter label (Right side of the view, bottom aligned with button)
        letterLabel.id = View.generateViewId()
        letterLabel.setTextColor(Color.parseColor("#808080"))
        letterLabel.gravity = Gravity.BOTTOM or Gravity.CENTER_HORIZONTAL
        letterLabel.includeFontPadding = false
        letterLabel.maxLines = 1
        val letterParams = LayoutParams(0, 0)
        letterParams.endToEnd = LayoutParams.PARENT_ID
        letterParams.bottomToBottom = LayoutParams.PARENT_ID
        letterParams.matchConstraintPercentWidth = 0.20f
        letterParams.matchConstraintPercentHeight = 0.65f 
        addView(letterLabel, letterParams)

        // Button background view (Left side)
        buttonView.id = View.generateViewId()
        val btnParams = LayoutParams(0, 0)
        btnParams.bottomToBottom = LayoutParams.PARENT_ID
        btnParams.startToStart = LayoutParams.PARENT_ID
        btnParams.endToStart = letterLabel.id
        btnParams.matchConstraintPercentHeight = 0.65f 
        buttonView.setBackgroundResource(R.drawable.calculator_key_background)
        addView(buttonView, btnParams)

        // Primary label (Center of button)
        primaryLabel.id = View.generateViewId()
        primaryLabel.setTextColor(Color.WHITE)
        primaryLabel.gravity = Gravity.CENTER
        primaryLabel.includeFontPadding = false
        primaryLabel.maxLines = 1
        val primaryParams = LayoutParams(0, 0)
        primaryParams.topToTop = buttonView.id
        primaryParams.bottomToBottom = buttonView.id
        primaryParams.startToStart = buttonView.id
        primaryParams.endToEnd = buttonView.id
        addView(primaryLabel, primaryParams)
        
        // F label (Above button)
        fLabel.id = View.generateViewId()
        fLabel.setTextColor(Color.parseColor("#E5AE5A"))
        fLabel.gravity = Gravity.START or Gravity.BOTTOM
        fLabel.includeFontPadding = false
        fLabel.maxLines = 1
        val fParams = LayoutParams(LayoutParams.WRAP_CONTENT, 0)
        fParams.bottomToTop = buttonView.id
        fParams.topToTop = LayoutParams.PARENT_ID
        fParams.startToStart = LayoutParams.PARENT_ID
        fParams.endToStart = gLabel.id
        fParams.horizontalChainStyle = LayoutParams.CHAIN_PACKED
        fParams.bottomMargin = 8 // Push up approx 2mm
        addView(fLabel, fParams)

        // G label (Above button)
        gLabel.id = View.generateViewId()
        gLabel.setTextColor(Color.parseColor("#7EB6BA"))
        gLabel.gravity = Gravity.END or Gravity.BOTTOM
        gLabel.includeFontPadding = false
        gLabel.maxLines = 1
        val gParams = LayoutParams(LayoutParams.WRAP_CONTENT, 0)
        gParams.bottomToTop = buttonView.id
        gParams.topToTop = LayoutParams.PARENT_ID
        gParams.startToEnd = fLabel.id
        gParams.endToEnd = LayoutParams.PARENT_ID
        gParams.bottomMargin = 8 // Push up approx 2mm
        addView(gLabel, gParams)

        // Alpha label (NOT USED inside key)
        alphaLabel.id = View.generateViewId()
        alphaLabel.visibility = View.GONE
        addView(alphaLabel)
        
        isClickable = false
        buttonView.isClickable = false
    }

    override fun onSizeChanged(w: Int, h: Int, oldw: Int, oldh: Int) {
        super.onSizeChanged(w, h, oldw, oldh)
        val btnH = h * 0.65f
        
        var mainLabelFactor = 0.75f
        val smallSize = h * 0.30f 
        val letterSize = btnH * 0.42f
        
        // Specific sizing rules
        val isNumber = keyCode in 18..20 || keyCode in 23..25 || keyCode in 28..30 || keyCode == 34
        val isOperator = keyCode in 21..22 || keyCode in 26..27 || keyCode in 31..32 || keyCode in 35..37
        val isSpecificFunction = keyCode == 18 || keyCode == 33 || keyCode == 36 || keyCode == 15 || keyCode == 16 || keyCode == 10
        
        if (keyCode in 7..9 || keyCode == 14 || keyCode == 18 || keyCode == 36) {
            mainLabelFactor *= 0.85f 
        } else if (isNumber || isOperator) {
            mainLabelFactor *= 1.15f
        } else if (isSpecificFunction) {
            mainLabelFactor *= 0.85f
        } else if (keyCode in 1..6) {
            mainLabelFactor *= 0.95f
        } else if (keyCode == 13 || keyCode == 17 || keyCode == 23 || keyCode == 28) {
            mainLabelFactor *= 0.95f
        }
        
        baseMainSize = btnH * mainLabelFactor
        
        // Initial sync of font size (will be updated again in updateLabels)
        updateFontSize(fOn = false, gOn = false)
        fLabel.setTextSize(TypedValue.COMPLEX_UNIT_PX, smallSize)
        gLabel.setTextSize(TypedValue.COMPLEX_UNIT_PX, smallSize)
        letterLabel.setTextSize(TypedValue.COMPLEX_UNIT_PX, letterSize)
    }

    private fun updateFontSize(fOn: Boolean, gOn: Boolean) {
        isShiftedState = fOn || gOn
        var factor = if (isShiftedState) 0.70f else 1.0f
        
        // Target specific labels per user request: "40% smaller" means 60% size
        if (fOn && keyCode == 14) factor = 0.60f // f-shift for x<->y (LASTx)
        if (gOn && keyCode == 35) factor = 0.60f // g-shift for . (SHOW a b/c)
        
        val size = baseMainSize * factor
        primaryLabel.setTextSize(TypedValue.COMPLEX_UNIT_PX, if (isFnKey) size * 0.8f else size)
    }

    private fun updateLayoutPositioning(alphaOn: Boolean) {
        if (lastAlphaStateForLayout == alphaOn) return
        lastAlphaStateForLayout = alphaOn

        val fParams = fLabel.layoutParams as LayoutParams
        val gParams = gLabel.layoutParams as LayoutParams
        
        // Reset to defaults
        fParams.horizontalChainStyle = LayoutParams.CHAIN_PACKED
        fParams.horizontalBias = 0.5f
        fParams.marginStart = 0
        fParams.marginEnd = 0
        gParams.horizontalBias = 0.5f
        gParams.marginEnd = 0
        gParams.marginStart = 0
        fParams.startToStart = LayoutParams.PARENT_ID
        fParams.endToStart = gLabel.id
        gParams.startToEnd = fLabel.id
        gParams.endToEnd = LayoutParams.PARENT_ID

        if (keyCode == 11 || keyCode == 12) {
            // Static gray labels centered above f/g keys
            if (!alphaOn) {
                fParams.endToStart = LayoutParams.UNSET
                fParams.endToEnd = LayoutParams.PARENT_ID
                fParams.horizontalBias = 0.5f
                gLabel.visibility = View.GONE
                fLabel.setTextColor(Color.parseColor("#808080"))
            } else {
                gLabel.visibility = View.VISIBLE
                fLabel.setTextColor(Color.parseColor("#E5AE5A"))
            }
        } else if (alphaOn) {
            // IN ALPHA MODE: Use Packed style for all keys to avoid wide gaps
            fParams.horizontalChainStyle = LayoutParams.CHAIN_PACKED
            fParams.marginEnd = 6 
            fParams.horizontalBias = 0.5f
        } else {
            // NORMAL MODE: Apply specific positioning rules
            val inst1Codes = intArrayOf(1, 2, 3, 4, 5, 6, 7, 8, 9, 13, 18, 37)
            val inst2Codes = intArrayOf(10, 14, 27, 32, 34, 35) // DRG, x<>y, *, -, 0, .
            val otherOverlapCodes = intArrayOf(20, 21, 22, 24, 25, 26, 29, 30, 31, 36)

            if (keyCode in inst1Codes) {
                fParams.horizontalChainStyle = LayoutParams.CHAIN_PACKED
                fParams.marginEnd = 6 
                fParams.horizontalBias = 0.5f
            } else if (keyCode in inst2Codes) {
                var yellowShift = -4
                var blueShift = 0
                if (keyCode == 14) yellowShift = -40 
                if (keyCode == 10 || keyCode == 35) yellowShift = -30 
                if (keyCode == 34) blueShift = -18 

                fParams.endToStart = LayoutParams.UNSET
                fParams.endToEnd = LayoutParams.UNSET
                fParams.horizontalBias = 0f
                fParams.marginStart = yellowShift
                
                gParams.startToStart = LayoutParams.UNSET
                gParams.startToEnd = LayoutParams.UNSET
                gParams.horizontalBias = 1f
                gParams.marginEnd = -blueShift 
            } else if (keyCode in otherOverlapCodes) {
                fParams.endToStart = LayoutParams.UNSET
                fParams.endToEnd = LayoutParams.UNSET
                fParams.horizontalBias = 0f
                fParams.marginStart = 0
                gParams.startToStart = LayoutParams.UNSET
                gParams.startToEnd = LayoutParams.UNSET
                gParams.horizontalBias = 1f
            }
        }
        
        fLabel.layoutParams = fParams
        gLabel.layoutParams = gParams
    }

    fun setKey(code: Int, isFn: Boolean, typeface: Typeface?) {
        this.keyCode = code
        this.isFnKey = isFn
        primaryLabel.typeface = typeface
        fLabel.typeface = typeface
        gLabel.typeface = typeface
        letterLabel.typeface = typeface
        
        if (isFn) {
            fLabel.visibility = View.GONE
            gLabel.visibility = View.GONE
            letterLabel.visibility = View.GONE
            primaryLabel.visibility = View.GONE
            buttonView.visibility = View.INVISIBLE
        } else {
            buttonView.visibility = View.VISIBLE
            fLabel.visibility = View.VISIBLE
            gLabel.visibility = View.VISIBLE
            letterLabel.visibility = View.VISIBLE
            primaryLabel.visibility = View.VISIBLE
            
            // Initial positioning (Alpha off)
            updateLayoutPositioning(false)

            // Layout Alignment Logic:
            // Keys that should take 100% of container width (no letter spacer)
            if (code == 13 || code == 18 || code == 23 || code == 28 || code == 33) {
                letterLabel.visibility = View.GONE
                val lp = buttonView.layoutParams as LayoutParams
                lp.endToEnd = LayoutParams.PARENT_ID
                buttonView.layoutParams = lp
            } else {
                // Keys that should leave space for a letter (20% width spacer)
                val lp = buttonView.layoutParams as LayoutParams
                lp.endToStart = letterLabel.id
                buttonView.layoutParams = lp
                
                if (code == 17) {
                    // Backspace matches width of 'g' (12) above it by keeping the spacer INVISIBLE
                    letterLabel.visibility = View.INVISIBLE 
                } else {
                    letterLabel.visibility = View.VISIBLE
                }
            }

            // Background Color Logic:
            if (code == 11) {
                buttonView.setBackgroundResource(R.drawable.calculator_key_f_background)
                primaryLabel.setTextColor(Color.BLACK)
            } else if (code == 12) {
                buttonView.setBackgroundResource(R.drawable.calculator_key_g_background)
                primaryLabel.setTextColor(Color.BLACK)
            } else {
                buttonView.setBackgroundResource(R.drawable.calculator_key_background)
                primaryLabel.setTextColor(Color.WHITE)
            }
        }
    }

    fun updateLabels(main: MainActivity) {
        if (isFnKey) {
            primaryLabel.text = main.getSoftkeyLabelNative(keyCode - 37)
        } else {
            val state = main.getKeyboardStateNative()
            var alphaOn = false
            if (state != null && state.size >= 5) {
                val fOn = state[0] != 0
                val gOn = state[1] != 0
                alphaOn = state[4] != 0
                updateFontSize(fOn, gOn || alphaOn) // Treat alpha entry as a shifted state for font scaling
                updateLayoutPositioning(alphaOn) // Switch layout configuration based on mode
            }

            primaryLabel.text = main.getButtonLabelNative(keyCode, 0)

            if (keyCode == 11) {
                fLabel.text = if (alphaOn) main.getButtonLabelNative(keyCode, 1) else "HOME"
                gLabel.text = if (alphaOn) main.getButtonLabelNative(keyCode, 2) else ""
            } else if (keyCode == 12) {
                fLabel.text = if (alphaOn) main.getButtonLabelNative(keyCode, 1) else "CUST"
                gLabel.text = if (alphaOn) main.getButtonLabelNative(keyCode, 2) else ""
            } else {
                fLabel.text = main.getButtonLabelNative(keyCode, 1)
                gLabel.text = main.getButtonLabelNative(keyCode, 2)
            }
            
            if (keyCode == 37) { 
                letterLabel.text = "_"
            } else {
                letterLabel.text = main.getButtonLabelNative(keyCode, 3)
            }
        }
    }
    
    override fun setPressed(pressed: Boolean) {
        super.setPressed(pressed)
        buttonView.isPressed = pressed
    }
}
