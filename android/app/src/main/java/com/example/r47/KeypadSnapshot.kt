package com.example.r47

internal data class KeypadKeySnapshot(
    val primaryLabel: String,
    val fLabel: String,
    val gLabel: String,
    val letterLabel: String,
    val auxLabel: String,
    val isEnabled: Boolean,
    val styleRole: Int,
    val labelRoles: Int,
    val layoutClass: Int,
    val sceneFlags: Int,
    val overlayState: Int,
    val showValue: Int,
) {
    companion object {
        const val NO_VALUE = -126

        val EMPTY = KeypadKeySnapshot(
            primaryLabel = "",
            fLabel = "",
            gLabel = "",
            letterLabel = "",
            auxLabel = "",
            isEnabled = false,
            styleRole = 0,
            labelRoles = 0,
            layoutClass = 0,
            sceneFlags = 0,
            overlayState = NO_VALUE,
            showValue = NO_VALUE,
        )
    }
}

internal object KeypadSceneContract {
    const val LABEL_PRIMARY = 0
    const val LABEL_F = 1
    const val LABEL_G = 2
    const val LABEL_LETTER = 3
    const val LABEL_AUX = 4

    const val STYLE_DEFAULT = 0
    const val STYLE_SOFTKEY = 1
    const val STYLE_SHIFT_F = 2
    const val STYLE_SHIFT_G = 3
    const val STYLE_SHIFT_FG = 4
    const val STYLE_NUMERIC = 5
    const val STYLE_ALPHA = 6

    const val TEXT_ROLE_NONE = 0
    const val TEXT_ROLE_PRIMARY = 1
    const val TEXT_ROLE_F = 2
    const val TEXT_ROLE_G = 3
    const val TEXT_ROLE_LETTER = 4
    const val TEXT_ROLE_F_UNDERLINE = 5
    const val TEXT_ROLE_G_UNDERLINE = 6
    const val TEXT_ROLE_LONGPRESS = 7
    const val TEXT_ROLE_SOFTKEY = 8

    const val LAYOUT_CLASS_DEFAULT = 0
    const val LAYOUT_CLASS_PACKED = 1
    const val LAYOUT_CLASS_OFFSET = 2
    const val LAYOUT_CLASS_EDGE = 3
    const val LAYOUT_CLASS_ALPHA = 4
    const val LAYOUT_CLASS_TAM = 5
    const val LAYOUT_CLASS_STATIC_SINGLE = 6
    const val LAYOUT_CLASS_SOFTKEY = 7

    const val SCENE_FLAG_SOFTKEY = 1 shl 0
    const val SCENE_FLAG_REVERSE_VIDEO = 1 shl 1
    const val SCENE_FLAG_TOP_LINE = 1 shl 2
    const val SCENE_FLAG_BOTTOM_LINE = 1 shl 3
    const val SCENE_FLAG_SHOW_CB = 1 shl 4
    const val SCENE_FLAG_SHOW_TEXT = 1 shl 5
    const val SCENE_FLAG_SHOW_VALUE = 1 shl 6
    const val SCENE_FLAG_STRIKE_OUT = 1 shl 7
    const val SCENE_FLAG_STRIKE_THROUGH = 1 shl 8
    const val SCENE_FLAG_MENU = 1 shl 9
    const val SCENE_FLAG_PREVIEW_TARGET = 1 shl 10
    const val SCENE_FLAG_DOTTED_ROW = 1 shl 11

    const val OVERLAY_RB_FALSE = 0
    const val OVERLAY_RB_TRUE = 1
    const val OVERLAY_CB_FALSE = 2
    const val OVERLAY_CB_TRUE = 3
    const val OVERLAY_MB_FALSE = 4
    const val OVERLAY_MB_TRUE = 5

    fun labelRole(labelRoles: Int, slot: Int): Int {
        return (labelRoles shr (slot * 4)) and 0xF
    }
}

internal fun KeypadKeySnapshot.labelRole(slot: Int): Int {
    return KeypadSceneContract.labelRole(labelRoles, slot)
}

internal fun KeypadKeySnapshot.hasSceneFlag(flag: Int): Boolean {
    return (sceneFlags and flag) != 0
}

internal data class KeypadSnapshot(
    val keyboardState: KeyboardStateSnapshot,
    val sceneContractVersion: Int,
    val softmenuId: Int,
    val softmenuFirstItem: Int,
    val softmenuItemCount: Int,
    val softmenuVisibleRowOffset: Int,
    val softmenuPage: Int,
    val softmenuPageCount: Int,
    val softmenuHasPreviousPage: Boolean,
    val softmenuHasNextPage: Boolean,
    val softmenuDottedRow: Int,
    val functionPreviewActive: Boolean,
    val functionPreviewKeyCode: Int,
    val functionPreviewRow: Int,
    val functionPreviewState: Int,
    val functionPreviewTimeoutActive: Boolean,
    val functionPreviewReleaseExec: Boolean,
    val functionPreviewNopOrExecuted: Boolean,
    private val keyStates: List<KeypadKeySnapshot>,
) {
    val shiftF: Boolean
        get() = keyboardState.shiftF

    val shiftG: Boolean
        get() = keyboardState.shiftG

    val calcMode: Int
        get() = keyboardState.calcMode

    val userModeEnabled: Boolean
        get() = keyboardState.userModeEnabled

    val alphaOn: Boolean
        get() = keyboardState.alphaOn

    fun keyStateFor(code: Int): KeypadKeySnapshot {
        if (code !in 1..KEY_COUNT) {
            return KeypadKeySnapshot.EMPTY
        }
        return keyStates[code - 1]
    }

    companion object {
        private const val KEY_COUNT = 43
        private const val LABELS_PER_KEY = 5

        private const val META_SHIFT_F = 0
        private const val META_SHIFT_G = 1
        private const val META_CALC_MODE = 2
        private const val META_USER_MODE = 3
        private const val META_ALPHA = 4
        private const val META_SOFTMENU_ID = 5
        private const val META_SOFTMENU_FIRST_ITEM = 6
        private const val META_SOFTMENU_ITEM_COUNT = 7
        private const val META_SOFTMENU_VISIBLE_ROW = 8
        private const val META_SOFTMENU_PAGE = 9
        private const val META_SOFTMENU_PAGE_COUNT = 10
        private const val META_SOFTMENU_HAS_PREVIOUS = 11
        private const val META_SOFTMENU_HAS_NEXT = 12
        private const val META_KEY_ENABLED_OFFSET = 13
        private const val META_CONTRACT_VERSION = META_KEY_ENABLED_OFFSET + KEY_COUNT
        private const val META_SOFTMENU_DOTTED_ROW = META_CONTRACT_VERSION + 1
        private const val META_FN_PREVIEW_ACTIVE = META_SOFTMENU_DOTTED_ROW + 1
        private const val META_FN_PREVIEW_KEY = META_FN_PREVIEW_ACTIVE + 1
        private const val META_FN_PREVIEW_ROW = META_FN_PREVIEW_KEY + 1
        private const val META_FN_PREVIEW_STATE = META_FN_PREVIEW_ROW + 1
        private const val META_FN_PREVIEW_TIMEOUT_ACTIVE = META_FN_PREVIEW_STATE + 1
        private const val META_FN_PREVIEW_RELEASE_EXEC = META_FN_PREVIEW_TIMEOUT_ACTIVE + 1
        private const val META_FN_PREVIEW_NOP_OR_EXECUTED = META_FN_PREVIEW_RELEASE_EXEC + 1
        private const val META_STYLE_ROLE_OFFSET = META_FN_PREVIEW_NOP_OR_EXECUTED + 1
        private const val META_LABEL_ROLE_OFFSET = META_STYLE_ROLE_OFFSET + KEY_COUNT
        private const val META_LAYOUT_CLASS_OFFSET = META_LABEL_ROLE_OFFSET + KEY_COUNT
        private const val META_SCENE_FLAGS_OFFSET = META_LAYOUT_CLASS_OFFSET + KEY_COUNT
        private const val META_OVERLAY_STATE_OFFSET = META_SCENE_FLAGS_OFFSET + KEY_COUNT
        private const val META_SHOW_VALUE_OFFSET = META_OVERLAY_STATE_OFFSET + KEY_COUNT
        private const val META_LENGTH = META_SHOW_VALUE_OFFSET + KEY_COUNT

        private val EMPTY_KEYS = List(KEY_COUNT) { KeypadKeySnapshot.EMPTY }

        val EMPTY = KeypadSnapshot(
            keyboardState = KeyboardStateSnapshot.EMPTY,
            sceneContractVersion = 0,
            softmenuId = 0,
            softmenuFirstItem = 0,
            softmenuItemCount = 0,
            softmenuVisibleRowOffset = 0,
            softmenuPage = 0,
            softmenuPageCount = 0,
            softmenuHasPreviousPage = false,
            softmenuHasNextPage = false,
            softmenuDottedRow = -1,
            functionPreviewActive = false,
            functionPreviewKeyCode = 0,
            functionPreviewRow = -1,
            functionPreviewState = 0,
            functionPreviewTimeoutActive = false,
            functionPreviewReleaseExec = false,
            functionPreviewNopOrExecuted = false,
            keyStates = EMPTY_KEYS,
        )

        fun fromNative(meta: IntArray?, labels: Array<String>?): KeypadSnapshot {
            if (meta == null || meta.size < META_LENGTH) {
                return EMPTY
            }

            val resolvedLabels = labels ?: emptyArray()
            val keyStates = List(KEY_COUNT) { index ->
                val labelIndex = index * LABELS_PER_KEY
                KeypadKeySnapshot(
                    primaryLabel = resolvedLabels.getOrElse(labelIndex) { "" },
                    fLabel = resolvedLabels.getOrElse(labelIndex + 1) { "" },
                    gLabel = resolvedLabels.getOrElse(labelIndex + 2) { "" },
                    letterLabel = resolvedLabels.getOrElse(labelIndex + 3) { "" },
                    auxLabel = resolvedLabels.getOrElse(labelIndex + 4) { "" },
                    isEnabled = meta[META_KEY_ENABLED_OFFSET + index] != 0,
                    styleRole = meta[META_STYLE_ROLE_OFFSET + index],
                    labelRoles = meta[META_LABEL_ROLE_OFFSET + index],
                    layoutClass = meta[META_LAYOUT_CLASS_OFFSET + index],
                    sceneFlags = meta[META_SCENE_FLAGS_OFFSET + index],
                    overlayState = meta[META_OVERLAY_STATE_OFFSET + index],
                    showValue = meta[META_SHOW_VALUE_OFFSET + index],
                )
            }

            return KeypadSnapshot(
                keyboardState = KeyboardStateSnapshot.fromMeta(meta),
                sceneContractVersion = meta[META_CONTRACT_VERSION],
                softmenuId = meta[META_SOFTMENU_ID],
                softmenuFirstItem = meta[META_SOFTMENU_FIRST_ITEM],
                softmenuItemCount = meta[META_SOFTMENU_ITEM_COUNT],
                softmenuVisibleRowOffset = meta[META_SOFTMENU_VISIBLE_ROW],
                softmenuPage = meta[META_SOFTMENU_PAGE],
                softmenuPageCount = meta[META_SOFTMENU_PAGE_COUNT],
                softmenuHasPreviousPage = meta[META_SOFTMENU_HAS_PREVIOUS] != 0,
                softmenuHasNextPage = meta[META_SOFTMENU_HAS_NEXT] != 0,
                softmenuDottedRow = meta[META_SOFTMENU_DOTTED_ROW],
                functionPreviewActive = meta[META_FN_PREVIEW_ACTIVE] != 0,
                functionPreviewKeyCode = meta[META_FN_PREVIEW_KEY],
                functionPreviewRow = meta[META_FN_PREVIEW_ROW],
                functionPreviewState = meta[META_FN_PREVIEW_STATE],
                functionPreviewTimeoutActive = meta[META_FN_PREVIEW_TIMEOUT_ACTIVE] != 0,
                functionPreviewReleaseExec = meta[META_FN_PREVIEW_RELEASE_EXEC] != 0,
                functionPreviewNopOrExecuted = meta[META_FN_PREVIEW_NOP_OR_EXECUTED] != 0,
                keyStates = keyStates,
            )
        }
    }
}