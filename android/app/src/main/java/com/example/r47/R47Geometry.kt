package com.example.r47

internal object R47ReferenceGeometry {
    // Measured reference-frame geometry shared with the Python contract tests.
    const val LOGICAL_CANVAS_WIDTH = 1820f
    const val LOGICAL_CANVAS_HEIGHT = 3403f

    const val STANDARD_LEFT = 134f
    const val STANDARD_PITCH = 272f
    const val STANDARD_KEY_WIDTH = 192f
    const val MATRIX_FIRST_VISIBLE_LEFT = 465f
    const val MATRIX_PITCH = 331f
    const val MATRIX_KEY_WIDTH = 228f
    const val ENTER_WIDTH = 462f
    const val ROW_HEIGHT = 144f
    const val ROW_STEP = 260f
    const val SOFTKEY_ROW_TOP = 1290f
    const val FIRST_SMALL_ROW_TOP = 1550f
    const val ENTER_ROW_TOP = 2070f
    const val FIRST_LARGE_ROW_TOP = 2330f
}

internal object R47AndroidChromeGeometry {
    // Android-only chrome values, already rebased into reference-space coordinates.
    const val NATIVE_SHELL_DRAW_CORNER_RADIUS = 80f

    const val SCALED_MODE_FIT_TRIM_LEFT = 42f
    const val SCALED_MODE_FIT_TRIM_TOP = 49f
    const val SCALED_MODE_FIT_TRIM_RIGHT = 42f
    const val SCALED_MODE_FIT_TRIM_BOTTOM = 56f

    const val TOP_BEZEL_SETTINGS_TAP_HEIGHT = 229f
    const val LCD_WINDOW_LEFT = 86f
    const val LCD_WINDOW_TOP = 229f
    const val LCD_WINDOW_WIDTH = 1648f
    const val LCD_WINDOW_HEIGHT = 903f

    const val NON_SOFTKEY_VIEW_HEIGHT = 236f
}

internal object R47LcdContract {
    const val PIXEL_WIDTH = 400
    const val PIXEL_HEIGHT = 240
    const val PIXEL_COUNT = PIXEL_WIDTH * PIXEL_HEIGHT
}