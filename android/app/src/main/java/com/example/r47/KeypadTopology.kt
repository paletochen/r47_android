package com.example.r47

internal enum class KeypadKeyFamily {
    SOFTKEY,
    STANDARD,
    ENTER,
    BASE_OPERATOR,
    NUMERIC_MATRIX,
}

internal enum class KeypadLane(
    val usesUpperTouchGrid: Boolean,
    val touchRowIndex: Int,
) {
    SOFTKEY_ROW(true, 0),
    SMALL_ROW_1(true, 1),
    SMALL_ROW_2(true, 2),
    ENTER_ROW(true, 3),
    MATRIX_ROW_1(false, 0),
    MATRIX_ROW_2(false, 1),
    MATRIX_ROW_3(false, 2),
    MATRIX_ROW_4(false, 3),
}

internal data class KeypadSlotSpec(
    val code: Int,
    val family: KeypadKeyFamily,
    val lane: KeypadLane,
    val column: Int,
    val columnSpan: Int = 1,
    val keepLetterSpacerInvisible: Boolean = false,
) {
    val isFunctionKey: Boolean
        get() = family == KeypadKeyFamily.SOFTKEY

    val usesLetterSpacer: Boolean
        get() = when (family) {
            KeypadKeyFamily.STANDARD,
            KeypadKeyFamily.NUMERIC_MATRIX -> true
            KeypadKeyFamily.SOFTKEY,
            KeypadKeyFamily.ENTER,
            KeypadKeyFamily.BASE_OPERATOR -> false
        }
}

internal object KeypadTopology {
    private val orderedSlots = listOf(
        KeypadSlotSpec(38, KeypadKeyFamily.SOFTKEY, KeypadLane.SOFTKEY_ROW, 0),
        KeypadSlotSpec(39, KeypadKeyFamily.SOFTKEY, KeypadLane.SOFTKEY_ROW, 1),
        KeypadSlotSpec(40, KeypadKeyFamily.SOFTKEY, KeypadLane.SOFTKEY_ROW, 2),
        KeypadSlotSpec(41, KeypadKeyFamily.SOFTKEY, KeypadLane.SOFTKEY_ROW, 3),
        KeypadSlotSpec(42, KeypadKeyFamily.SOFTKEY, KeypadLane.SOFTKEY_ROW, 4),
        KeypadSlotSpec(43, KeypadKeyFamily.SOFTKEY, KeypadLane.SOFTKEY_ROW, 5),

        KeypadSlotSpec(1, KeypadKeyFamily.STANDARD, KeypadLane.SMALL_ROW_1, 0),
        KeypadSlotSpec(2, KeypadKeyFamily.STANDARD, KeypadLane.SMALL_ROW_1, 1),
        KeypadSlotSpec(3, KeypadKeyFamily.STANDARD, KeypadLane.SMALL_ROW_1, 2),
        KeypadSlotSpec(4, KeypadKeyFamily.STANDARD, KeypadLane.SMALL_ROW_1, 3),
        KeypadSlotSpec(5, KeypadKeyFamily.STANDARD, KeypadLane.SMALL_ROW_1, 4),
        KeypadSlotSpec(6, KeypadKeyFamily.STANDARD, KeypadLane.SMALL_ROW_1, 5),

        KeypadSlotSpec(7, KeypadKeyFamily.STANDARD, KeypadLane.SMALL_ROW_2, 0),
        KeypadSlotSpec(8, KeypadKeyFamily.STANDARD, KeypadLane.SMALL_ROW_2, 1),
        KeypadSlotSpec(9, KeypadKeyFamily.STANDARD, KeypadLane.SMALL_ROW_2, 2),
        KeypadSlotSpec(10, KeypadKeyFamily.STANDARD, KeypadLane.SMALL_ROW_2, 3),
        KeypadSlotSpec(11, KeypadKeyFamily.STANDARD, KeypadLane.SMALL_ROW_2, 4),
        KeypadSlotSpec(12, KeypadKeyFamily.STANDARD, KeypadLane.SMALL_ROW_2, 5),

        KeypadSlotSpec(13, KeypadKeyFamily.ENTER, KeypadLane.ENTER_ROW, 0, columnSpan = 2),
        KeypadSlotSpec(14, KeypadKeyFamily.STANDARD, KeypadLane.ENTER_ROW, 2),
        KeypadSlotSpec(15, KeypadKeyFamily.STANDARD, KeypadLane.ENTER_ROW, 3),
        KeypadSlotSpec(16, KeypadKeyFamily.STANDARD, KeypadLane.ENTER_ROW, 4),
        KeypadSlotSpec(
            17,
            KeypadKeyFamily.STANDARD,
            KeypadLane.ENTER_ROW,
            5,
            keepLetterSpacerInvisible = true,
        ),

        KeypadSlotSpec(18, KeypadKeyFamily.BASE_OPERATOR, KeypadLane.MATRIX_ROW_1, 0),
        KeypadSlotSpec(19, KeypadKeyFamily.NUMERIC_MATRIX, KeypadLane.MATRIX_ROW_1, 1),
        KeypadSlotSpec(20, KeypadKeyFamily.NUMERIC_MATRIX, KeypadLane.MATRIX_ROW_1, 2),
        KeypadSlotSpec(21, KeypadKeyFamily.NUMERIC_MATRIX, KeypadLane.MATRIX_ROW_1, 3),
        KeypadSlotSpec(22, KeypadKeyFamily.NUMERIC_MATRIX, KeypadLane.MATRIX_ROW_1, 4),

        KeypadSlotSpec(23, KeypadKeyFamily.BASE_OPERATOR, KeypadLane.MATRIX_ROW_2, 0),
        KeypadSlotSpec(24, KeypadKeyFamily.NUMERIC_MATRIX, KeypadLane.MATRIX_ROW_2, 1),
        KeypadSlotSpec(25, KeypadKeyFamily.NUMERIC_MATRIX, KeypadLane.MATRIX_ROW_2, 2),
        KeypadSlotSpec(26, KeypadKeyFamily.NUMERIC_MATRIX, KeypadLane.MATRIX_ROW_2, 3),
        KeypadSlotSpec(27, KeypadKeyFamily.NUMERIC_MATRIX, KeypadLane.MATRIX_ROW_2, 4),

        KeypadSlotSpec(28, KeypadKeyFamily.BASE_OPERATOR, KeypadLane.MATRIX_ROW_3, 0),
        KeypadSlotSpec(29, KeypadKeyFamily.NUMERIC_MATRIX, KeypadLane.MATRIX_ROW_3, 1),
        KeypadSlotSpec(30, KeypadKeyFamily.NUMERIC_MATRIX, KeypadLane.MATRIX_ROW_3, 2),
        KeypadSlotSpec(31, KeypadKeyFamily.NUMERIC_MATRIX, KeypadLane.MATRIX_ROW_3, 3),
        KeypadSlotSpec(32, KeypadKeyFamily.NUMERIC_MATRIX, KeypadLane.MATRIX_ROW_3, 4),

        KeypadSlotSpec(33, KeypadKeyFamily.BASE_OPERATOR, KeypadLane.MATRIX_ROW_4, 0),
        KeypadSlotSpec(34, KeypadKeyFamily.NUMERIC_MATRIX, KeypadLane.MATRIX_ROW_4, 1),
        KeypadSlotSpec(35, KeypadKeyFamily.NUMERIC_MATRIX, KeypadLane.MATRIX_ROW_4, 2),
        KeypadSlotSpec(36, KeypadKeyFamily.NUMERIC_MATRIX, KeypadLane.MATRIX_ROW_4, 3),
        KeypadSlotSpec(37, KeypadKeyFamily.NUMERIC_MATRIX, KeypadLane.MATRIX_ROW_4, 4),
    )

    private val slotsByCode = orderedSlots.associateBy { it.code }
    private val slotsByLane = orderedSlots.groupBy { it.lane }

    fun orderedSlots(): List<KeypadSlotSpec> = orderedSlots

    fun slotFor(code: Int): KeypadSlotSpec = slotsByCode.getValue(code)

    fun slotsForLane(lane: KeypadLane): List<KeypadSlotSpec> = slotsByLane.getValue(lane)
}