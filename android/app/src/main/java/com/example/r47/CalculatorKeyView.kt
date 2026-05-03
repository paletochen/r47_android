package com.example.r47

import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.RectF
import android.graphics.Typeface
import android.util.AttributeSet
import android.util.TypedValue
import android.view.Gravity
import android.view.View
import android.widget.TextView
import androidx.constraintlayout.widget.ConstraintLayout
import kotlin.math.abs

internal data class KeypadFontSet(
    val standard: Typeface?,
    val numeric: Typeface?,
    val tiny: Typeface?,
)

private data class MainKeyStyleSpec(
    val fontSize: Float,
    val primaryTextColor: Int,
    val idleFillColor: Int,
    val pressedFillColor: Int,
)

private object KeyVisualPolicy {
    // Android presentation tuning values. These are UI policy, not measured hardware geometry.
    const val MAIN_KEY_DRAW_CORNER_RADIUS = 20f
    const val SOFTKEY_DRAW_CORNER_RADIUS = MAIN_KEY_DRAW_CORNER_RADIUS
    const val MAIN_KEY_BODY_OPTICAL_WIDTH_DELTA = 6f
    const val FOURTH_LABEL_X_OFFSET_FROM_MAIN_KEY_BODY_RIGHT = 16f
    const val FOURTH_LABEL_Y_OFFSET_FROM_MAIN_KEY_BODY_TOP = 80f
    const val DEFAULT_PRIMARY_LEGEND_TEXT_SIZE = 76f
    const val NUMERIC_PRIMARY_LEGEND_TEXT_SIZE = 114f
    const val SHIFT_STYLE_PRIMARY_LEGEND_TEXT_SIZE = 94f
    const val TOP_F_G_LABEL_TEXT_SIZE = 64f
    const val FOURTH_LABEL_TEXT_SIZE = 66f
    const val TOP_F_G_LABEL_HORIZONTAL_GAP = 10f
    const val TOP_F_G_LABEL_VERTICAL_LIFT = 86f
    const val SOFTKEY_DECOR_STROKE_WIDTH = 2f
    const val SOFTKEY_OUTER_INSET = 2f
    const val SOFTKEY_PREVIEW_LINE_SIDE_INSET = 10f
    const val SOFTKEY_PREVIEW_LINE_BOTTOM_INSET = 4f
    const val SOFTKEY_VALUE_TEXT_SIZE_RATIO = 0.18f
    const val SOFTKEY_VALUE_WIDTH_RATIO = 0.34f
    const val SOFTKEY_VALUE_RIGHT_INSET = 7f
    const val SOFTKEY_VALUE_TOP_INSET = 6f
    const val SOFTKEY_OVERLAY_CENTER_RIGHT_INSET = 10f
    const val SOFTKEY_OVERLAY_CENTER_BOTTOM_INSET = 12f
    const val SOFTKEY_PRIMARY_TOP_RATIO = 0.28f
    const val SOFTKEY_PRIMARY_SIDE_INSET = 8f
    const val SOFTKEY_PRIMARY_RIGHT_RESERVE_WITH_OVERLAY = 16f
    const val SOFTKEY_AUX_TEXT_SIZE_RATIO = 0.16f
    const val SOFTKEY_AUX_SIDE_INSET = 12f
    const val SOFTKEY_AUX_BOTTOM_INSET = 11f
    const val SOFTKEY_STRIKE_SIDE_INSET = 8f
    const val SOFTKEY_STRIKE_OUT_SIDE_INSET = 7f
    const val SOFTKEY_STRIKE_OUT_VERTICAL_INSET = 10f
    const val SOFTKEY_OVERLAY_SIZE = 7f
    const val SOFTKEY_OVERLAY_MARK_HALF_EXTENT_RATIO = 0.7f
    const val SOFTKEY_OVERLAY_MARK_DOT_RATIO = 0.28f
    const val SOFTKEY_OVERLAY_CHECK_LEFT_X = 3f
    const val SOFTKEY_OVERLAY_CHECK_MID_X = 1f
    const val SOFTKEY_OVERLAY_CHECK_DELTA_Y = 3f
    const val SOFTKEY_OVERLAY_CHECK_RIGHT_X = 4f
    const val SOFTKEY_OVERLAY_MB_HALF_WIDTH = 6.5f
    const val SOFTKEY_OVERLAY_MB_HALF_HEIGHT = 5.2f
    const val SOFTKEY_OVERLAY_MB_CORNER_RADIUS = 3f
    const val SOFTKEY_OVERLAY_MB_TEXT_SIZE_RATIO = 0.12f
    const val SOFTKEY_OVERLAY_MB_TEXT_MAX_WIDTH = 9f
    const val SOFTKEY_OVERLAY_MB_TEXT_BASELINE_OFFSET = 0.5f
    const val SOFTKEY_OVERLAY_MB_UNDERLINE_START_X = 1f
    const val SOFTKEY_OVERLAY_MB_UNDERLINE_END_X = 6f
    const val SOFTKEY_OVERLAY_MB_UNDERLINE_Y = 4f
    const val FITTED_TEXT_MIN_SCALE = 0.58f
}

class CalculatorKeyView @JvmOverloads constructor(
    context: Context, attrs: AttributeSet? = null, defStyleAttr: Int = 0
) : ConstraintLayout(context, attrs, defStyleAttr) {

    companion object {
        private val defaultPrimaryColor = Color.WHITE
        private val defaultPrimaryDarkColor = Color.BLACK
        private val fAccentColor = Color.rgb(242, 171, 94)
        private val fPressedColor = Color.rgb(246, 196, 141)
        private val gAccentColor = Color.rgb(131, 183, 223)
        private val gPressedColor = Color.rgb(171, 207, 234)
        private val fgAccentColor = fAccentColor
        private val fgPressedColor = fPressedColor
        private val alphaAccentColor = Color.parseColor("#E36C50")
        private val letterColor = Color.parseColor("#A5A5A5")
        private val fourthLabelColor = Color.rgb(223, 223, 223)
        private val longPressColor = Color.parseColor("#D4D8DD")
        private val mainKeyFillColor = Color.rgb(64, 64, 64)
        private val mainKeyPressedColor = Color.parseColor("#744A2E")
        private val softkeyReverseColor = Color.rgb(96, 96, 96)
        private val softkeyReversePressedColor = mainKeyPressedColor
        private val softkeyLightTextColor = Color.parseColor("#F4F7F9")
        private val softkeyMetaLightColor = Color.parseColor("#C9D0D6")
        private val softkeyValueLightColor = Color.parseColor("#F0C77A")
        private val softkeyPreviewColor = Color.parseColor("#E5AE5A")
        private const val MAIN_KEY_BODY_HEIGHT_FRACTION_OF_VIEW = 0.610169f
        private const val STANDARD_KEY_FOURTH_LABEL_STRIP_WIDTH_FRACTION = 0.294118f
        private const val MATRIX_KEY_FOURTH_LABEL_STRIP_WIDTH_FRACTION = 0.311178f
        private const val TEXT_ANCHOR_CENTER = 0
        private const val TEXT_ANCHOR_TOP = 1
        private const val TEXT_ANCHOR_BOTTOM = 2
    }

    private val buttonView = View(context)
    val primaryLabel = TextView(context)
    val fLabel = TextView(context)
    val gLabel = TextView(context)
    val alphaLabel = TextView(context)
    val letterLabel = TextView(context)
    
    var keyCode: Int = 0
    private var isFnKey: Boolean = false
    private var lastLayoutClass: Int? = null
    private var usesLetterSpacer = true
    private var keepLetterSpacerInvisible = false
    private var fontSet = KeypadFontSet(null, null, null)
    private var softkeyState = KeypadKeySnapshot.EMPTY
    private var mainKeyState = KeypadKeySnapshot.EMPTY
    private var currentShiftFOn = false
    private var currentShiftGOn = false
    private var designCellWidth = R47ReferenceGeometry.STANDARD_PITCH
    private var designButtonWidth = R47ReferenceGeometry.STANDARD_KEY_WIDTH
    private var drawKeySurfaces = true
    private val softkeyRect = RectF()
    private val mainKeyRect = RectF()
    private val mainKeyFillPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        style = Paint.Style.FILL
    }
    private val softkeyFillPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        style = Paint.Style.FILL
    }
    private val softkeyTextPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        textAlign = Paint.Align.CENTER
    }
    private val softkeyAuxPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        textAlign = Paint.Align.CENTER
    }
    private val softkeyValuePaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        textAlign = Paint.Align.RIGHT
    }
    private val softkeyDecorPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        style = Paint.Style.STROKE
        strokeCap = Paint.Cap.ROUND
        strokeWidth = KeyVisualPolicy.SOFTKEY_DECOR_STROKE_WIDTH
    }
    private val softkeyDotPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        style = Paint.Style.FILL
    }
    private val faceplateOffsetUpdater = Runnable { updateFaceplateOffsets() }

    init {
        // Critical: Allow drawing outside bounds
        clipChildren = false
        clipToPadding = false
        setWillNotDraw(false)
        
        // Letter label (Right side of the view, bottom aligned with button)
        letterLabel.id = View.generateViewId()
        letterLabel.setTextColor(fourthLabelColor)
        letterLabel.gravity = Gravity.START or Gravity.TOP
        letterLabel.includeFontPadding = false
        letterLabel.maxLines = 1
        val letterParams = LayoutParams(0, 0)
        letterParams.topToTop = buttonView.id
        letterParams.endToEnd = LayoutParams.PARENT_ID
        letterParams.bottomToBottom = LayoutParams.PARENT_ID
        letterParams.matchConstraintPercentWidth = STANDARD_KEY_FOURTH_LABEL_STRIP_WIDTH_FRACTION
        letterParams.matchConstraintPercentHeight = MAIN_KEY_BODY_HEIGHT_FRACTION_OF_VIEW
        addView(letterLabel, letterParams)

        // Button background view (Left side)
        buttonView.id = View.generateViewId()
        val btnParams = LayoutParams(0, 0)
        btnParams.topToTop = LayoutParams.PARENT_ID
        btnParams.bottomToBottom = LayoutParams.PARENT_ID
        btnParams.startToStart = LayoutParams.PARENT_ID
        btnParams.endToStart = letterLabel.id
        btnParams.verticalBias = 0f
        btnParams.matchConstraintPercentHeight = MAIN_KEY_BODY_HEIGHT_FRACTION_OF_VIEW
        buttonView.setBackgroundColor(Color.TRANSPARENT)
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
        fLabel.setTextColor(fAccentColor)
        fLabel.gravity = Gravity.START or Gravity.TOP
        fLabel.includeFontPadding = false
        fLabel.maxLines = 1
        val fParams = LayoutParams(LayoutParams.WRAP_CONTENT, 0)
        fParams.topToTop = buttonView.id
        fParams.bottomToBottom = buttonView.id
        fParams.startToStart = LayoutParams.PARENT_ID
        fParams.endToStart = gLabel.id
        fParams.horizontalChainStyle = LayoutParams.CHAIN_PACKED
        addView(fLabel, fParams)

        // G label (Above button)
        gLabel.id = View.generateViewId()
        gLabel.setTextColor(gAccentColor)
        gLabel.gravity = Gravity.END or Gravity.TOP
        gLabel.includeFontPadding = false
        gLabel.maxLines = 1
        val gParams = LayoutParams(LayoutParams.WRAP_CONTENT, 0)
        gParams.topToTop = buttonView.id
        gParams.bottomToBottom = buttonView.id
        gParams.startToEnd = fLabel.id
        gParams.endToEnd = LayoutParams.PARENT_ID
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
        updateFontSize(currentShiftFOn, currentShiftGOn)
        updateFaceplateOffsets()
    }

    private fun updateFontSize(fOn: Boolean, gOn: Boolean) {
        currentShiftFOn = fOn
        currentShiftGOn = gOn

        if (isFnKey || width <= 0) {
            return
        }

        val referenceCellToViewWidthScale = if (designCellWidth > 0f) width.toFloat() / designCellWidth else 1f
        val primarySize = mainKeyStyleSpec(mainKeyState.styleRole).fontSize * referenceCellToViewWidthScale

        primaryLabel.setTextSize(TypedValue.COMPLEX_UNIT_PX, primarySize)
        primaryLabel.textScaleX = 1f
        fLabel.setTextSize(TypedValue.COMPLEX_UNIT_PX, KeyVisualPolicy.TOP_F_G_LABEL_TEXT_SIZE * referenceCellToViewWidthScale)
        gLabel.setTextSize(TypedValue.COMPLEX_UNIT_PX, KeyVisualPolicy.TOP_F_G_LABEL_TEXT_SIZE * referenceCellToViewWidthScale)
        letterLabel.setTextSize(TypedValue.COMPLEX_UNIT_PX, KeyVisualPolicy.FOURTH_LABEL_TEXT_SIZE * referenceCellToViewWidthScale)
        primaryLabel.translationX = 0f
    }

    private fun resetLabelLayout() {
        val fParams = fLabel.layoutParams as LayoutParams
        val gParams = gLabel.layoutParams as LayoutParams

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

        fLabel.layoutParams = fParams
        gLabel.layoutParams = gParams
        scheduleFaceplateOffsetUpdate()
    }

    private fun scheduleFaceplateOffsetUpdate() {
        removeCallbacks(faceplateOffsetUpdater)
        post(faceplateOffsetUpdater)
    }

    private fun updateFaceplateOffsets() {
        if (isFnKey || width <= 0) {
            return
        }

        val referenceCellToViewWidthScale = if (designCellWidth > 0f) width.toFloat() / designCellWidth else 1f
        val topFgLabelVerticalLift = KeyVisualPolicy.TOP_F_G_LABEL_VERTICAL_LIFT * referenceCellToViewWidthScale
        val topFgLabelHorizontalGap = KeyVisualPolicy.TOP_F_G_LABEL_HORIZONTAL_GAP * referenceCellToViewWidthScale
        val buttonWidth = buttonView.width.toFloat()
        if (buttonWidth <= 0f || buttonView.height <= 0) {
            return
        }

        val referenceBodyToViewWidthScale = if (designButtonWidth > 0f) buttonWidth / designButtonWidth else 1f
        updateMainKeySurfaceRect(mainKeyRect, referenceBodyToViewWidthScale)
        val buttonCenterX = mainKeyRect.centerX()
        val rawButtonCenterX = buttonView.left + buttonWidth / 2f
        primaryLabel.translationX = buttonCenterX - rawButtonCenterX
        val fWidth = maxOf(fLabel.width, fLabel.measuredWidth).toFloat()
        val gWidth = maxOf(gLabel.width, gLabel.measuredWidth).toFloat()
        val hasFLabel = fLabel.visibility == View.VISIBLE && fLabel.text.isNotBlank()
        val hasGLabel = hasFLabel && gLabel.visibility == View.VISIBLE && gLabel.text.isNotBlank()
        val groupWidth = when {
            hasGLabel -> fWidth + topFgLabelHorizontalGap + gWidth
            hasFLabel -> fWidth
            else -> 0f
        }
        val groupLeft = buttonCenterX - groupWidth / 2f

        when {
            hasGLabel -> {
                val fLeft = groupLeft
                val gLeft = groupLeft + fWidth + topFgLabelHorizontalGap
                fLabel.translationX = fLeft - fLabel.left.toFloat()
                gLabel.translationX = gLeft - gLabel.left.toFloat()
            }

            hasFLabel -> {
                fLabel.translationX = groupLeft - fLabel.left.toFloat()
                gLabel.translationX = 0f
            }

            else -> {
                fLabel.translationX = 0f
                gLabel.translationX = 0f
            }
        }

        fLabel.translationY = -topFgLabelVerticalLift - fLabel.top.toFloat()
        gLabel.translationY = -topFgLabelVerticalLift - gLabel.top.toFloat()

        updateFourthLabelOffset(referenceCellToViewWidthScale)
    }

    private fun updateFourthLabelOffset(referenceCellToViewWidthScale: Float) {
        val letterLeft =
            mainKeyRect.right +
                KeyVisualPolicy.FOURTH_LABEL_X_OFFSET_FROM_MAIN_KEY_BODY_RIGHT * referenceCellToViewWidthScale
        val letterTop =
            KeyVisualPolicy.FOURTH_LABEL_Y_OFFSET_FROM_MAIN_KEY_BODY_TOP * referenceCellToViewWidthScale
        letterLabel.translationX = letterLeft - letterLabel.left.toFloat()
        letterLabel.translationY = letterTop - letterLabel.top.toFloat()
    }

    private fun updateMainKeySurfaceRect(targetRect: RectF, referenceBodyToViewWidthScale: Float) {
        val inset = referenceBodyToViewWidthScale
        val widthBonus = KeyVisualPolicy.MAIN_KEY_BODY_OPTICAL_WIDTH_DELTA * referenceBodyToViewWidthScale
        val halfWidthBonus = widthBonus * 0.5f
        targetRect.set(
            (buttonView.left.toFloat() + inset - halfWidthBonus).coerceAtLeast(inset),
            buttonView.top.toFloat() + inset,
            (buttonView.right.toFloat() - inset + halfWidthBonus).coerceAtMost(width.toFloat() - inset),
            buttonView.bottom.toFloat() - inset,
        )
    }

    private fun updateLayoutPositioning(layoutClass: Int) {
        if (lastLayoutClass == layoutClass) return
        lastLayoutClass = layoutClass

        resetLabelLayout()
        val fParams = fLabel.layoutParams as LayoutParams
        val gParams = gLabel.layoutParams as LayoutParams

        when (layoutClass) {
            KeypadSceneContract.LAYOUT_CLASS_STATIC_SINGLE -> {
                fParams.endToStart = LayoutParams.UNSET
                fParams.endToEnd = LayoutParams.PARENT_ID
                fParams.horizontalBias = 0.5f
                gLabel.visibility = View.GONE
            }
        }

        fLabel.layoutParams = fParams
        gLabel.layoutParams = gParams
        scheduleFaceplateOffsetUpdate()
    }

    internal fun setKey(slot: KeypadSlotSpec, fonts: KeypadFontSet) {
        this.keyCode = slot.code
        this.isFnKey = slot.isFunctionKey
        this.fontSet = fonts
        primaryLabel.typeface = fonts.standard
        fLabel.typeface = fonts.standard
        gLabel.typeface = fonts.standard
        letterLabel.typeface = fonts.standard
        
        if (slot.isFunctionKey) {
            softkeyState = KeypadKeySnapshot.EMPTY
            mainKeyState = KeypadKeySnapshot.EMPTY
            fLabel.visibility = View.GONE
            gLabel.visibility = View.GONE
            letterLabel.visibility = View.GONE
            primaryLabel.visibility = View.GONE
            buttonView.visibility = View.INVISIBLE
            invalidate()
        } else {
            buttonView.visibility = View.VISIBLE
            fLabel.visibility = View.VISIBLE
            gLabel.visibility = View.VISIBLE
            letterLabel.visibility = View.VISIBLE
            primaryLabel.visibility = View.VISIBLE

            lastLayoutClass = null
            resetLabelLayout()
            configureMainKeySurface(slot.family)
            usesLetterSpacer = slot.usesLetterSpacer
            keepLetterSpacerInvisible = slot.keepLetterSpacerInvisible

            if (!slot.usesLetterSpacer) {
                letterLabel.visibility = View.GONE
                val lp = buttonView.layoutParams as LayoutParams
                lp.endToStart = LayoutParams.UNSET
                lp.endToEnd = LayoutParams.PARENT_ID
                buttonView.layoutParams = lp
            } else {
                val lp = buttonView.layoutParams as LayoutParams
                lp.endToStart = letterLabel.id
                lp.endToEnd = LayoutParams.UNSET
                buttonView.layoutParams = lp
                letterLabel.visibility = if (keepLetterSpacerInvisible) View.INVISIBLE else View.VISIBLE
            }

            primaryLabel.setTextColor(defaultPrimaryColor)
            mainKeyState = KeypadKeySnapshot.EMPTY
        }
    }

    internal fun setDrawKeySurfaces(draw: Boolean) {
        if (drawKeySurfaces == draw) {
            return
        }
        drawKeySurfaces = draw
        invalidate()
    }

    private fun mainKeyStyleSpec(styleRole: Int): MainKeyStyleSpec {
        return when (styleRole) {
            KeypadSceneContract.STYLE_SHIFT_F -> MainKeyStyleSpec(
                fontSize = KeyVisualPolicy.SHIFT_STYLE_PRIMARY_LEGEND_TEXT_SIZE,
                primaryTextColor = defaultPrimaryDarkColor,
                idleFillColor = fAccentColor,
                pressedFillColor = fPressedColor,
            )

            KeypadSceneContract.STYLE_SHIFT_G -> MainKeyStyleSpec(
                fontSize = KeyVisualPolicy.SHIFT_STYLE_PRIMARY_LEGEND_TEXT_SIZE,
                primaryTextColor = defaultPrimaryDarkColor,
                idleFillColor = gAccentColor,
                pressedFillColor = gPressedColor,
            )

            KeypadSceneContract.STYLE_SHIFT_FG -> MainKeyStyleSpec(
                fontSize = KeyVisualPolicy.SHIFT_STYLE_PRIMARY_LEGEND_TEXT_SIZE,
                primaryTextColor = defaultPrimaryDarkColor,
                idleFillColor = fgAccentColor,
                pressedFillColor = fgPressedColor,
            )

            KeypadSceneContract.STYLE_ALPHA -> MainKeyStyleSpec(
                fontSize = KeyVisualPolicy.DEFAULT_PRIMARY_LEGEND_TEXT_SIZE,
                primaryTextColor = defaultPrimaryDarkColor,
                idleFillColor = alphaAccentColor,
                pressedFillColor = alphaAccentColor,
            )

            KeypadSceneContract.STYLE_NUMERIC -> MainKeyStyleSpec(
                fontSize = KeyVisualPolicy.NUMERIC_PRIMARY_LEGEND_TEXT_SIZE,
                primaryTextColor = defaultPrimaryColor,
                idleFillColor = mainKeyFillColor,
                pressedFillColor = mainKeyPressedColor,
            )

            else -> MainKeyStyleSpec(
                fontSize = KeyVisualPolicy.DEFAULT_PRIMARY_LEGEND_TEXT_SIZE,
                primaryTextColor = defaultPrimaryColor,
                idleFillColor = mainKeyFillColor,
                pressedFillColor = mainKeyPressedColor,
            )
        }
    }

    private fun configureMainKeySurface(family: KeypadKeyFamily) {
        val letterRatio: Float
        when (family) {
            KeypadKeyFamily.STANDARD -> {
                designCellWidth = R47ReferenceGeometry.STANDARD_PITCH
                designButtonWidth = R47ReferenceGeometry.STANDARD_KEY_WIDTH
                letterRatio = STANDARD_KEY_FOURTH_LABEL_STRIP_WIDTH_FRACTION
            }

            KeypadKeyFamily.ENTER -> {
                designCellWidth = R47ReferenceGeometry.ENTER_WIDTH
                designButtonWidth = R47ReferenceGeometry.ENTER_WIDTH
                letterRatio = 0f
            }

            KeypadKeyFamily.NUMERIC_MATRIX -> {
                designCellWidth = R47ReferenceGeometry.MATRIX_PITCH
                designButtonWidth = R47ReferenceGeometry.MATRIX_KEY_WIDTH
                letterRatio = MATRIX_KEY_FOURTH_LABEL_STRIP_WIDTH_FRACTION
            }

            KeypadKeyFamily.BASE_OPERATOR -> {
                designCellWidth = R47ReferenceGeometry.STANDARD_KEY_WIDTH
                designButtonWidth = R47ReferenceGeometry.STANDARD_KEY_WIDTH
                letterRatio = 0f
            }

            KeypadKeyFamily.SOFTKEY -> error("Softkeys use the dedicated function-key drawing path")
        }

        val buttonParams = buttonView.layoutParams as LayoutParams
        val letterParams = letterLabel.layoutParams as LayoutParams
        letterParams.topToTop = buttonView.id
        letterParams.bottomToBottom = buttonView.id
        letterParams.matchConstraintPercentWidth = letterRatio
        letterParams.matchConstraintPercentHeight = MAIN_KEY_BODY_HEIGHT_FRACTION_OF_VIEW

        if (letterRatio > 0f) {
            buttonParams.endToStart = letterLabel.id
            buttonParams.endToEnd = LayoutParams.UNSET
        } else {
            buttonParams.endToStart = LayoutParams.UNSET
            buttonParams.endToEnd = LayoutParams.PARENT_ID
        }

        buttonView.layoutParams = buttonParams
        letterLabel.layoutParams = letterParams
        scheduleFaceplateOffsetUpdate()
    }

    private fun primaryTypefaceFor(): Typeface? {
        return fontSet.standard
    }

    private fun applyLabelRole(labelView: TextView, role: Int, defaultColor: Int) {
        var paintFlags = labelView.paintFlags and Paint.UNDERLINE_TEXT_FLAG.inv()
        if (
            role == KeypadSceneContract.TEXT_ROLE_F_UNDERLINE ||
                role == KeypadSceneContract.TEXT_ROLE_G_UNDERLINE
        ) {
            paintFlags = paintFlags or Paint.UNDERLINE_TEXT_FLAG
        }
        labelView.paintFlags = paintFlags
        labelView.typeface = when (labelView) {
            primaryLabel -> primaryTypefaceFor()
            fLabel, gLabel, letterLabel -> fontSet.standard
            else -> fontSet.standard
        }
        labelView.setTextColor(
            when (role) {
                KeypadSceneContract.TEXT_ROLE_LONGPRESS -> longPressColor
                else -> defaultColor
            }
        )
    }

    private fun applySceneStyling(keyState: KeypadKeySnapshot) {
        primaryLabel.setTextColor(mainKeyStyleSpec(keyState.styleRole).primaryTextColor)
        primaryLabel.typeface = primaryTypefaceFor()
        applyLabelRole(
            fLabel,
            keyState.labelRole(KeypadSceneContract.LABEL_F),
            if (keyState.layoutClass == KeypadSceneContract.LAYOUT_CLASS_STATIC_SINGLE) letterColor else fAccentColor,
        )
        applyLabelRole(
            gLabel,
            keyState.labelRole(KeypadSceneContract.LABEL_G),
            gAccentColor,
        )
        applyLabelRole(
            letterLabel,
            keyState.labelRole(KeypadSceneContract.LABEL_LETTER),
            fourthLabelColor,
        )
    }

    private fun applyLabelVisibility(keyState: KeypadKeySnapshot) {
        primaryLabel.visibility = View.VISIBLE
        val hasFLabel = keyState.fLabel.isNotBlank()
        fLabel.visibility = if (hasFLabel) View.VISIBLE else View.INVISIBLE
        gLabel.visibility = if (hasFLabel && keyState.gLabel.isNotBlank()) View.VISIBLE else View.INVISIBLE
        if (!usesLetterSpacer) {
            letterLabel.visibility = View.GONE
        } else if (keepLetterSpacerInvisible || keyState.letterLabel.isBlank()) {
            letterLabel.visibility = View.INVISIBLE
        } else {
            letterLabel.visibility = View.VISIBLE
        }

        if (keyState.layoutClass == KeypadSceneContract.LAYOUT_CLASS_STATIC_SINGLE) {
            gLabel.visibility = View.GONE
        }
    }

    private fun applyEnabledState(enabled: Boolean) {
        isEnabled = enabled
        alpha = if (isFnKey || enabled) 1f else 0.45f
        if (!isFnKey) {
            buttonView.alpha = if (enabled) 1f else 0.45f
            primaryLabel.alpha = if (enabled) 1f else 0.6f
            fLabel.alpha = if (enabled) 1f else 0.6f
            gLabel.alpha = if (enabled) 1f else 0.6f
            letterLabel.alpha = if (enabled) 1f else 0.6f
        }
    }

    internal fun updateLabels(snapshot: KeypadSnapshot) {
        val keyState = snapshot.keyStateFor(keyCode)
        applyEnabledState(keyState.isEnabled)

        if (isFnKey) {
            softkeyState = keyState
            mainKeyState = KeypadKeySnapshot.EMPTY
            contentDescription = buildSoftkeyContentDescription(keyState)
            invalidate()
        } else {
            mainKeyState = keyState
            primaryLabel.text = keyState.primaryLabel
            fLabel.text = keyState.fLabel
            gLabel.text = keyState.gLabel
            letterLabel.text = keyState.letterLabel
            currentShiftFOn = snapshot.shiftF
            currentShiftGOn = snapshot.shiftG || snapshot.alphaOn
            updateFontSize(currentShiftFOn, currentShiftGOn)
            updateLayoutPositioning(keyState.layoutClass)
            applySceneStyling(keyState)
            applyLabelVisibility(keyState)
            scheduleFaceplateOffsetUpdate()
            contentDescription = buildString {
                append(keyState.primaryLabel)
                if (keyState.fLabel.isNotBlank()) {
                    append(", f ")
                    append(keyState.fLabel)
                }
                if (keyState.fLabel.isNotBlank() && keyState.gLabel.isNotBlank()) {
                    append(", g ")
                    append(keyState.gLabel)
                }
            }
        }
    }
    
    override fun setPressed(pressed: Boolean) {
        super.setPressed(pressed)
        invalidate()
    }

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)
        if (isFnKey) {
            drawSoftkey(canvas)
        } else {
            drawMainKey(canvas)
        }
    }

    private fun drawMainKey(canvas: Canvas) {
        if (buttonView.width <= 0 || buttonView.height <= 0) {
            return
        }

        val keyState = mainKeyState
        val referenceBodyToViewWidthScale = if (designButtonWidth > 0f) {
            buttonView.width.toFloat() / designButtonWidth
        } else {
            1f
        }
        val cornerRadius = KeyVisualPolicy.MAIN_KEY_DRAW_CORNER_RADIUS * referenceBodyToViewWidthScale
        updateMainKeySurfaceRect(mainKeyRect, referenceBodyToViewWidthScale)

        val styleSpec = mainKeyStyleSpec(keyState.styleRole)
        val fillColor = if (isPressed) styleSpec.pressedFillColor else styleSpec.idleFillColor

        if (drawKeySurfaces) {
            drawKeyChrome(
                canvas = canvas,
                rect = mainKeyRect,
                fillPaint = mainKeyFillPaint,
                fillColor = fillColor,
                cornerRadius = cornerRadius,
            )
        }
    }

    private fun drawSoftkey(canvas: Canvas) {
        val keyState = softkeyState
        val reverseVideo = keyState.hasSceneFlag(KeypadSceneContract.SCENE_FLAG_REVERSE_VIDEO)
        val showText = keyState.hasSceneFlag(KeypadSceneContract.SCENE_FLAG_SHOW_TEXT) &&
            keyState.auxLabel.isNotBlank()
        val showValue = keyState.hasSceneFlag(KeypadSceneContract.SCENE_FLAG_SHOW_VALUE) &&
            keyState.showValue != KeypadKeySnapshot.NO_VALUE
        val showOverlay = keyState.hasSceneFlag(KeypadSceneContract.SCENE_FLAG_SHOW_CB) &&
            keyState.overlayState >= 0

        softkeyRect.set(
            KeyVisualPolicy.SOFTKEY_OUTER_INSET,
            KeyVisualPolicy.SOFTKEY_OUTER_INSET,
            width - KeyVisualPolicy.SOFTKEY_OUTER_INSET,
            height - KeyVisualPolicy.SOFTKEY_OUTER_INSET,
        )

        val fillColor = when {
            reverseVideo && isPressed -> softkeyReversePressedColor
            reverseVideo -> softkeyReverseColor
            isPressed -> mainKeyPressedColor
            else -> mainKeyFillColor
        }
        val decorColor = if (reverseVideo) softkeyLightTextColor else defaultPrimaryColor
        val primaryTextColor = if (reverseVideo) softkeyLightTextColor else defaultPrimaryColor
        val metaTextColor = if (reverseVideo) softkeyMetaLightColor else letterColor
        val valueTextColor = softkeyValueLightColor

        softkeyDecorPaint.color = decorColor
        softkeyDotPaint.color = decorColor

        val softkeySurfaceScale = width / R47ReferenceGeometry.STANDARD_KEY_WIDTH
        val cornerRadius = KeyVisualPolicy.SOFTKEY_DRAW_CORNER_RADIUS * softkeySurfaceScale
        if (drawKeySurfaces) {
            drawKeyChrome(
                canvas = canvas,
                rect = softkeyRect,
                fillPaint = softkeyFillPaint,
                fillColor = fillColor,
                cornerRadius = cornerRadius,
            )
        }
        if (drawKeySurfaces && keyState.hasSceneFlag(KeypadSceneContract.SCENE_FLAG_PREVIEW_TARGET)) {
            softkeyDecorPaint.color = softkeyPreviewColor
            canvas.drawLine(
                softkeyRect.left + KeyVisualPolicy.SOFTKEY_PREVIEW_LINE_SIDE_INSET,
                softkeyRect.bottom - KeyVisualPolicy.SOFTKEY_PREVIEW_LINE_BOTTOM_INSET,
                softkeyRect.right - KeyVisualPolicy.SOFTKEY_PREVIEW_LINE_SIDE_INSET,
                softkeyRect.bottom - KeyVisualPolicy.SOFTKEY_PREVIEW_LINE_BOTTOM_INSET,
                softkeyDecorPaint,
            )
            softkeyDecorPaint.color = decorColor
        }

        if (showValue) {
            val valueText = formatSoftkeyValue(keyState.showValue)
            if (valueText.isNotBlank()) {
                drawFittedText(
                    canvas = canvas,
                    text = valueText,
                    paint = softkeyValuePaint,
                    typeface = fontSet.numeric ?: fontSet.tiny ?: fontSet.standard,
                    baseSize = height * KeyVisualPolicy.SOFTKEY_VALUE_TEXT_SIZE_RATIO,
                    maxWidth = softkeyRect.width() * KeyVisualPolicy.SOFTKEY_VALUE_WIDTH_RATIO,
                    x = softkeyRect.right - KeyVisualPolicy.SOFTKEY_VALUE_RIGHT_INSET,
                    anchorY = softkeyRect.top + KeyVisualPolicy.SOFTKEY_VALUE_TOP_INSET,
                    color = valueTextColor,
                    align = Paint.Align.RIGHT,
                    verticalAnchor = TEXT_ANCHOR_TOP,
                )
            }
        }

        if (showOverlay) {
            drawSoftkeyOverlay(
                canvas = canvas,
                overlayState = keyState.overlayState,
                centerX = softkeyRect.right - KeyVisualPolicy.SOFTKEY_OVERLAY_CENTER_RIGHT_INSET,
                centerY = softkeyRect.bottom - KeyVisualPolicy.SOFTKEY_OVERLAY_CENTER_BOTTOM_INSET,
                color = decorColor,
            )
        }

        if (keyState.primaryLabel.isNotBlank()) {
            val primaryCenterY = if (showText) {
                softkeyRect.top + (softkeyRect.height() * KeyVisualPolicy.SOFTKEY_PRIMARY_TOP_RATIO)
            } else {
                softkeyRect.centerY()
            }
            val reservedRight = when {
                showOverlay -> KeyVisualPolicy.SOFTKEY_PRIMARY_RIGHT_RESERVE_WITH_OVERLAY
                else -> KeyVisualPolicy.SOFTKEY_PRIMARY_SIDE_INSET
            }
            drawFittedText(
                canvas = canvas,
                text = keyState.primaryLabel,
                paint = softkeyTextPaint,
                typeface = fontSet.standard,
                baseSize = KeyVisualPolicy.DEFAULT_PRIMARY_LEGEND_TEXT_SIZE * softkeySurfaceScale,
                maxWidth = softkeyRect.width() - reservedRight - KeyVisualPolicy.SOFTKEY_PRIMARY_SIDE_INSET,
                x = softkeyRect.centerX(),
                anchorY = primaryCenterY,
                color = primaryTextColor,
            )
        }

        if (showText) {
            drawFittedText(
                canvas = canvas,
                text = keyState.auxLabel,
                paint = softkeyAuxPaint,
                typeface = fontSet.tiny ?: fontSet.standard,
                baseSize = height * KeyVisualPolicy.SOFTKEY_AUX_TEXT_SIZE_RATIO,
                maxWidth = softkeyRect.width() - KeyVisualPolicy.SOFTKEY_AUX_SIDE_INSET,
                x = softkeyRect.centerX(),
                anchorY = softkeyRect.bottom - KeyVisualPolicy.SOFTKEY_AUX_BOTTOM_INSET,
                color = metaTextColor,
                verticalAnchor = TEXT_ANCHOR_BOTTOM,
            )
        }

        if (keyState.hasSceneFlag(KeypadSceneContract.SCENE_FLAG_STRIKE_THROUGH)) {
            canvas.drawLine(
                softkeyRect.left + KeyVisualPolicy.SOFTKEY_STRIKE_SIDE_INSET,
                softkeyRect.centerY(),
                softkeyRect.right - KeyVisualPolicy.SOFTKEY_STRIKE_SIDE_INSET,
                softkeyRect.centerY(),
                softkeyDecorPaint,
            )
        }
        if (keyState.hasSceneFlag(KeypadSceneContract.SCENE_FLAG_STRIKE_OUT)) {
            canvas.drawLine(
                softkeyRect.left + KeyVisualPolicy.SOFTKEY_STRIKE_OUT_SIDE_INSET,
                softkeyRect.top + KeyVisualPolicy.SOFTKEY_STRIKE_OUT_VERTICAL_INSET,
                softkeyRect.right - KeyVisualPolicy.SOFTKEY_STRIKE_OUT_SIDE_INSET,
                softkeyRect.bottom - KeyVisualPolicy.SOFTKEY_STRIKE_OUT_VERTICAL_INSET,
                softkeyDecorPaint,
            )
        }
    }

    private fun drawKeyChrome(
        canvas: Canvas,
        rect: RectF,
        fillPaint: Paint,
        fillColor: Int,
        cornerRadius: Float,
    ) {
        fillPaint.color = fillColor
        canvas.drawRoundRect(rect, cornerRadius, cornerRadius, fillPaint)
    }

    private fun drawSoftkeyOverlay(
        canvas: Canvas,
        overlayState: Int,
        centerX: Float,
        centerY: Float,
        color: Int,
    ) {
        val size = KeyVisualPolicy.SOFTKEY_OVERLAY_SIZE
        softkeyDecorPaint.color = color

        when (overlayState) {
            KeypadSceneContract.OVERLAY_RB_FALSE -> {
                canvas.drawCircle(centerX, centerY, size * KeyVisualPolicy.SOFTKEY_OVERLAY_MARK_HALF_EXTENT_RATIO, softkeyDecorPaint)
            }
            KeypadSceneContract.OVERLAY_RB_TRUE -> {
                canvas.drawCircle(centerX, centerY, size * KeyVisualPolicy.SOFTKEY_OVERLAY_MARK_HALF_EXTENT_RATIO, softkeyDecorPaint)
                canvas.drawCircle(centerX, centerY, size * KeyVisualPolicy.SOFTKEY_OVERLAY_MARK_DOT_RATIO, softkeyDotPaint.apply { this.color = color })
            }
            KeypadSceneContract.OVERLAY_CB_FALSE -> {
                canvas.drawRect(
                    centerX - size * KeyVisualPolicy.SOFTKEY_OVERLAY_MARK_HALF_EXTENT_RATIO,
                    centerY - size * KeyVisualPolicy.SOFTKEY_OVERLAY_MARK_HALF_EXTENT_RATIO,
                    centerX + size * KeyVisualPolicy.SOFTKEY_OVERLAY_MARK_HALF_EXTENT_RATIO,
                    centerY + size * KeyVisualPolicy.SOFTKEY_OVERLAY_MARK_HALF_EXTENT_RATIO,
                    softkeyDecorPaint,
                )
            }
            KeypadSceneContract.OVERLAY_CB_TRUE -> {
                canvas.drawRect(
                    centerX - size * KeyVisualPolicy.SOFTKEY_OVERLAY_MARK_HALF_EXTENT_RATIO,
                    centerY - size * KeyVisualPolicy.SOFTKEY_OVERLAY_MARK_HALF_EXTENT_RATIO,
                    centerX + size * KeyVisualPolicy.SOFTKEY_OVERLAY_MARK_HALF_EXTENT_RATIO,
                    centerY + size * KeyVisualPolicy.SOFTKEY_OVERLAY_MARK_HALF_EXTENT_RATIO,
                    softkeyDecorPaint,
                )
                canvas.drawLine(
                    centerX - KeyVisualPolicy.SOFTKEY_OVERLAY_CHECK_LEFT_X,
                    centerY,
                    centerX - KeyVisualPolicy.SOFTKEY_OVERLAY_CHECK_MID_X,
                    centerY + KeyVisualPolicy.SOFTKEY_OVERLAY_CHECK_DELTA_Y,
                    softkeyDecorPaint,
                )
                canvas.drawLine(
                    centerX - KeyVisualPolicy.SOFTKEY_OVERLAY_CHECK_MID_X,
                    centerY + KeyVisualPolicy.SOFTKEY_OVERLAY_CHECK_DELTA_Y,
                    centerX + KeyVisualPolicy.SOFTKEY_OVERLAY_CHECK_RIGHT_X,
                    centerY - KeyVisualPolicy.SOFTKEY_OVERLAY_CHECK_DELTA_Y,
                    softkeyDecorPaint,
                )
            }
            KeypadSceneContract.OVERLAY_MB_FALSE,
            KeypadSceneContract.OVERLAY_MB_TRUE -> {
                val rect = RectF(
                    centerX - KeyVisualPolicy.SOFTKEY_OVERLAY_MB_HALF_WIDTH,
                    centerY - KeyVisualPolicy.SOFTKEY_OVERLAY_MB_HALF_HEIGHT,
                    centerX + KeyVisualPolicy.SOFTKEY_OVERLAY_MB_HALF_WIDTH,
                    centerY + KeyVisualPolicy.SOFTKEY_OVERLAY_MB_HALF_HEIGHT,
                )
                canvas.drawRoundRect(
                    rect,
                    KeyVisualPolicy.SOFTKEY_OVERLAY_MB_CORNER_RADIUS,
                    KeyVisualPolicy.SOFTKEY_OVERLAY_MB_CORNER_RADIUS,
                    softkeyDecorPaint,
                )
                drawFittedText(
                    canvas = canvas,
                    text = "M",
                    paint = softkeyAuxPaint,
                    typeface = fontSet.tiny ?: fontSet.standard,
                    baseSize = height * KeyVisualPolicy.SOFTKEY_OVERLAY_MB_TEXT_SIZE_RATIO,
                    maxWidth = KeyVisualPolicy.SOFTKEY_OVERLAY_MB_TEXT_MAX_WIDTH,
                    x = centerX,
                    anchorY = centerY - KeyVisualPolicy.SOFTKEY_OVERLAY_MB_TEXT_BASELINE_OFFSET,
                    color = color,
                )
                if (overlayState == KeypadSceneContract.OVERLAY_MB_TRUE) {
                    canvas.drawLine(
                        centerX + KeyVisualPolicy.SOFTKEY_OVERLAY_MB_UNDERLINE_START_X,
                        centerY + KeyVisualPolicy.SOFTKEY_OVERLAY_MB_UNDERLINE_Y,
                        centerX + KeyVisualPolicy.SOFTKEY_OVERLAY_MB_UNDERLINE_END_X,
                        centerY + KeyVisualPolicy.SOFTKEY_OVERLAY_MB_UNDERLINE_Y,
                        softkeyDecorPaint,
                    )
                }
            }
        }
    }

    private fun drawFittedText(
        canvas: Canvas,
        text: String,
        paint: Paint,
        typeface: Typeface?,
        baseSize: Float,
        maxWidth: Float,
        x: Float,
        anchorY: Float,
        color: Int,
        align: Paint.Align = Paint.Align.CENTER,
        verticalAnchor: Int = TEXT_ANCHOR_CENTER,
    ) {
        if (text.isBlank()) {
            return
        }

        paint.typeface = typeface
        paint.textAlign = align
        paint.color = color
        paint.textSize = baseSize

        val measured = paint.measureText(text)
        if (measured > maxWidth && measured > 0f) {
            paint.textSize =
                (baseSize * (maxWidth / measured)).coerceAtLeast(baseSize * KeyVisualPolicy.FITTED_TEXT_MIN_SCALE)
        }

        val metrics = paint.fontMetrics
        val baseline = when (verticalAnchor) {
            TEXT_ANCHOR_TOP -> anchorY - metrics.ascent
            TEXT_ANCHOR_BOTTOM -> anchorY - metrics.descent
            else -> anchorY - ((metrics.ascent + metrics.descent) / 2f)
        }

        canvas.drawText(text, x, baseline, paint)
    }

    private fun formatSoftkeyValue(showValue: Int): String {
        return when (showValue) {
            KeypadKeySnapshot.NO_VALUE,
            -127 -> ""
            else -> {
                val prefix = if (showValue < 0) "-" else ""
                prefix + abs(showValue).toString()
            }
        }
    }

    private fun buildSoftkeyContentDescription(keyState: KeypadKeySnapshot): String {
        return buildString {
            append(keyState.primaryLabel)
            if (keyState.hasSceneFlag(KeypadSceneContract.SCENE_FLAG_SHOW_TEXT) && keyState.auxLabel.isNotBlank()) {
                append(", ")
                append(keyState.auxLabel)
            }
            val valueText = formatSoftkeyValue(keyState.showValue)
            if (valueText.isNotBlank()) {
                append(", ")
                append(valueText)
            }
        }
    }
}
