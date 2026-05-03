package com.example.r47

internal data class KeyboardStateSnapshot(
    val shiftF: Boolean,
    val shiftG: Boolean,
    val calcMode: Int,
    val userModeEnabled: Boolean,
    val alphaOn: Boolean,
) {
    companion object {
        val EMPTY = KeyboardStateSnapshot(
            shiftF = false,
            shiftG = false,
            calcMode = 0,
            userModeEnabled = false,
            alphaOn = false,
        )

        fun fromNative(state: IntArray?): KeyboardStateSnapshot {
            if (state == null || state.size < 5) {
                return EMPTY
            }

            return KeyboardStateSnapshot(
                shiftF = state[0] != 0,
                shiftG = state[1] != 0,
                calcMode = state[2],
                userModeEnabled = state[3] != 0,
                alphaOn = state[4] != 0,
            )
        }

        fun fromMeta(meta: IntArray?): KeyboardStateSnapshot {
            return fromNative(meta)
        }
    }
}