package com.example.r47

import android.graphics.Typeface
import android.view.MotionEvent
import android.view.View

internal object ReplicaKeypadLayout {
    private const val CHROME_MODE_TEXTURE = "r47_texture"
    private const val SOFTKEY_WIDTH = R47ReferenceGeometry.STANDARD_KEY_WIDTH
    private const val SOFTKEY_HEIGHT = R47ReferenceGeometry.ROW_HEIGHT
    private const val NON_SOFTKEY_VIEW_HEIGHT = R47AndroidChromeGeometry.NON_SOFTKEY_VIEW_HEIGHT
    private val UPPER_COLUMN_BOUNDARIES = floatArrayOf(94.0f, 366.0f, 638.0f, 910.0f, 1182.0f, 1454.0f, 1726.0f)
    private val UPPER_ROW_BOUNDARIES = floatArrayOf(1232.0f, 1492.0f, 1752.0f, 2012.0f, 2272.0f)
    private val LOWER_COLUMN_BOUNDARIES = floatArrayOf(82.5f, 413.5f, 744.5f, 1075.5f, 1406.5f, 1737.5f)
    private val LOWER_ROW_BOUNDARIES = floatArrayOf(2272.0f, 2532.0f, 2792.0f, 3052.0f, 3312.0f)

    private data class TouchZoneSpec(
        val code: Int,
        val x: Float,
        val y: Float,
        val width: Float,
        val height: Float,
    )

    private val baseTouchZones = buildBaseTouchZones()
    private val baseTouchZonesByCode = baseTouchZones.associateBy { it.code }

    fun rebuild(
        activity: MainActivity,
        overlay: ReplicaOverlay,
        chromeMode: String,
        performHapticClick: () -> Unit,
        dispatchKey: (Int) -> Unit,
    ) {
        overlay.removeAllViews()
        if (chromeMode == CHROME_MODE_TEXTURE) {
            addClassicKeypad(activity, overlay, performHapticClick, dispatchKey)
        } else {
            addDynamicKeypad(activity, overlay, chromeMode, performHapticClick, dispatchKey)
        }
    }

    fun updateDynamicKeys(
        overlay: ReplicaOverlay,
        snapshot: KeypadSnapshot,
    ) {
        for (index in 0 until overlay.childCount) {
            val child = overlay.getChildAt(index)
            if (child is CalculatorKeyView) {
                child.updateLabels(snapshot)
            }
        }
    }

    private fun addClassicKeypad(
        activity: MainActivity,
        overlay: ReplicaOverlay,
        performHapticClick: () -> Unit,
        dispatchKey: (Int) -> Unit,
    ) {
        for (touchZone in baseTouchZones) {
            addTouchZone(
                activity = activity,
                overlay = overlay,
                code = touchZone.code,
                performHapticClick = performHapticClick,
                dispatchKey = dispatchKey,
            )
        }
    }

    private fun addDynamicKeypad(
        activity: MainActivity,
        overlay: ReplicaOverlay,
        chromeMode: String,
        performHapticClick: () -> Unit,
        dispatchKey: (Int) -> Unit,
    ) {
        val fonts = KeypadFontSet(
            standard = loadTypeface(activity, "fonts/C47__StandardFont.ttf"),
            numeric = loadTypeface(activity, "fonts/C47__NumericFont.ttf"),
            tiny = loadTypeface(activity, "fonts/C47__TinyFont.ttf"),
        )
        val initialSnapshot = activity.currentKeypadSnapshot().takeIf {
            it.sceneContractVersion > 0
        }

        KeypadTopology.orderedSlots().forEach { slot ->
            val y = when (slot.lane) {
                KeypadLane.SOFTKEY_ROW -> R47ReferenceGeometry.SOFTKEY_ROW_TOP
                KeypadLane.SMALL_ROW_1 -> R47ReferenceGeometry.FIRST_SMALL_ROW_TOP
                KeypadLane.SMALL_ROW_2 -> R47ReferenceGeometry.FIRST_SMALL_ROW_TOP + R47ReferenceGeometry.ROW_STEP
                KeypadLane.ENTER_ROW -> R47ReferenceGeometry.ENTER_ROW_TOP
                KeypadLane.MATRIX_ROW_1 -> R47ReferenceGeometry.FIRST_LARGE_ROW_TOP
                KeypadLane.MATRIX_ROW_2 -> R47ReferenceGeometry.FIRST_LARGE_ROW_TOP + R47ReferenceGeometry.ROW_STEP
                KeypadLane.MATRIX_ROW_3 -> R47ReferenceGeometry.FIRST_LARGE_ROW_TOP + R47ReferenceGeometry.ROW_STEP * 2f
                KeypadLane.MATRIX_ROW_4 -> R47ReferenceGeometry.FIRST_LARGE_ROW_TOP + R47ReferenceGeometry.ROW_STEP * 3f
            }
            val x = when (slot.family) {
                KeypadKeyFamily.SOFTKEY,
                KeypadKeyFamily.STANDARD,
                KeypadKeyFamily.ENTER,
                KeypadKeyFamily.BASE_OPERATOR -> R47ReferenceGeometry.STANDARD_LEFT + R47ReferenceGeometry.STANDARD_PITCH * slot.column

                KeypadKeyFamily.NUMERIC_MATRIX ->
                    R47ReferenceGeometry.MATRIX_FIRST_VISIBLE_LEFT +
                        R47ReferenceGeometry.MATRIX_PITCH * (slot.column - 1)
            }
            val width = when (slot.family) {
                KeypadKeyFamily.SOFTKEY -> SOFTKEY_WIDTH
                KeypadKeyFamily.STANDARD -> R47ReferenceGeometry.STANDARD_PITCH
                KeypadKeyFamily.ENTER -> R47ReferenceGeometry.ENTER_WIDTH
                KeypadKeyFamily.BASE_OPERATOR -> R47ReferenceGeometry.STANDARD_KEY_WIDTH
                KeypadKeyFamily.NUMERIC_MATRIX -> R47ReferenceGeometry.MATRIX_PITCH
            }
            val height = if (slot.family == KeypadKeyFamily.SOFTKEY) {
                SOFTKEY_HEIGHT
            } else {
                NON_SOFTKEY_VIEW_HEIGHT
            }
            addKey(
                activity = activity,
                overlay = overlay,
                fonts = fonts,
                initialSnapshot = initialSnapshot,
                slot = slot,
                chromeMode = chromeMode,
                x = x,
                y = y,
                width = width,
                height = height,
                performHapticClick = performHapticClick,
                dispatchKey = dispatchKey,
            )
        }
    }

    private fun addKey(
        activity: MainActivity,
        overlay: ReplicaOverlay,
        fonts: KeypadFontSet,
        initialSnapshot: KeypadSnapshot?,
        slot: KeypadSlotSpec,
        chromeMode: String,
        x: Float,
        y: Float,
        width: Float,
        height: Float,
        performHapticClick: () -> Unit,
        dispatchKey: (Int) -> Unit,
    ) {
        val keyView = CalculatorKeyView(activity)
        keyView.setKey(slot, fonts)
        keyView.setDrawKeySurfaces(chromeMode != ReplicaOverlay.CHROME_MODE_BACKGROUND)
        initialSnapshot?.let { keyView.updateLabels(it) }
        addTouchZone(
            activity = activity,
            overlay = overlay,
            code = slot.code,
            performHapticClick = performHapticClick,
            dispatchKey = dispatchKey,
            pressedView = keyView,
        )
        keyView.setOnTouchListener(
            createTouchListener(
                code = slot.code,
                performHapticClick = performHapticClick,
                dispatchKey = dispatchKey,
                pressedView = keyView,
            )
        )
        overlay.addReplicaView(keyView, x, y, width, height)
    }

    private fun addTouchZone(
        activity: MainActivity,
        overlay: ReplicaOverlay,
        code: Int,
        performHapticClick: () -> Unit,
        dispatchKey: (Int) -> Unit,
        pressedView: View? = null,
    ) {
        val touchZoneSpec = touchZoneFor(code)
        val touchZone = View(activity).apply {
            isFocusable = false
            isFocusableInTouchMode = false
            importantForAccessibility = View.IMPORTANT_FOR_ACCESSIBILITY_NO
            background = null
            setOnTouchListener(
                createTouchListener(
                    code = code,
                    performHapticClick = performHapticClick,
                    dispatchKey = dispatchKey,
                    pressedView = pressedView,
                )
            )
        }

        overlay.addReplicaView(
            touchZone,
            touchZoneSpec.x,
            touchZoneSpec.y,
            touchZoneSpec.width,
            touchZoneSpec.height,
            showTouchZone = true,
        )
    }

    private fun loadTypeface(activity: MainActivity, assetPath: String): Typeface? {
        return try {
            Typeface.createFromAsset(activity.assets, assetPath)
        } catch (_: Exception) {
            null
        }
    }

    private fun createTouchListener(
        code: Int,
        performHapticClick: () -> Unit,
        dispatchKey: (Int) -> Unit,
        pressedView: View? = null,
    ): View.OnTouchListener {
        return View.OnTouchListener { view, event ->
            val feedbackView = pressedView ?: view
            if (!feedbackView.isEnabled) {
                feedbackView.isPressed = false
                return@OnTouchListener true
            }

            when (event.action) {
                MotionEvent.ACTION_DOWN -> {
                    feedbackView.isPressed = true
                    performHapticClick()
                    dispatchKey(code)
                }

                MotionEvent.ACTION_UP, MotionEvent.ACTION_CANCEL -> {
                    feedbackView.isPressed = false
                    dispatchKey(0)
                }
            }
            true
        }
    }

    private fun touchZoneFor(code: Int): TouchZoneSpec {
        return baseTouchZonesByCode.getValue(code)
    }

    private fun buildBaseTouchZones(): List<TouchZoneSpec> {
        return KeypadTopology.orderedSlots().map { slot ->
            val rowBounds = if (slot.lane.usesUpperTouchGrid) {
                UPPER_ROW_BOUNDARIES
            } else {
                LOWER_ROW_BOUNDARIES
            }
            val columnBounds = if (slot.lane.usesUpperTouchGrid) {
                UPPER_COLUMN_BOUNDARIES
            } else {
                LOWER_COLUMN_BOUNDARIES
            }
            TouchZoneSpec(
                code = slot.code,
                x = columnBounds[slot.column],
                y = rowBounds[slot.lane.touchRowIndex],
                width = columnBounds[slot.column + slot.columnSpan] - columnBounds[slot.column],
                height = rowBounds[slot.lane.touchRowIndex + 1] - rowBounds[slot.lane.touchRowIndex],
            )
        }
    }
}