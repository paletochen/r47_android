package org.rpncalculators.r47

import kotlin.math.max
import kotlin.math.min

internal data class TopLabelLaneGroupInput(
    val code: Int,
    val centerX: Float,
    val bodyWidth: Float,
    val textWidth: Float,
    val gapWidth: Float,
    val maxShift: Float,
)

internal data class TopLabelLanePlacement(
    val centerShift: Float = 0f,
    val staggerLevel: Int = 0,
    val scale: Float = 1f,
) {
    companion object {
        val DEFAULT = TopLabelLanePlacement()
    }
}

private data class TopLabelLaneGroupState(
    val group: TopLabelLaneGroupInput,
    var staggerLevel: Int = 0,
    var scale: Float = 1f,
    var centerX: Float = group.centerX,
) {
    val minCenterX: Float
        get() = group.centerX - group.maxShift

    val maxCenterX: Float
        get() = group.centerX + group.maxShift

    val totalWidth: Float
        get() = group.textWidth * scale + group.gapWidth

    val left: Float
        get() = centerX - totalWidth / 2f

    val right: Float
        get() = centerX + totalWidth / 2f
}

private data class TopLabelLaneConflict(
    val leftIndex: Int,
    val rightIndex: Int,
)

internal object TopLabelLaneLayout {
    private const val EPSILON = 0.001f
    private const val MAX_RELAXATION_PASSES = 8

    fun solve(groups: List<TopLabelLaneGroupInput>): Map<Int, TopLabelLanePlacement> {
        if (groups.isEmpty()) {
            return emptyMap()
        }

        if (groups.size == 1) {
            return mapOf(groups.single().code to TopLabelLanePlacement.DEFAULT)
        }

        val states = groups.map { TopLabelLaneGroupState(group = it) }
        states.forEach { it.centerX = it.group.centerX }
        var conflicts = resolvePositions(states)

        repeat(states.size) {
            if (conflicts.isEmpty()) {
                return@repeat
            }
            if (!scaleOffenders(states, conflicts)) {
                return@repeat
            }
            conflicts = resolvePositions(states)
        }

        resolvePositions(states)
        return states.associate { state ->
            val centerShift = state.centerX - state.group.centerX
            state.group.code to TopLabelLanePlacement(
                centerShift = centerShift,
                staggerLevel = state.staggerLevel,
                scale = state.scale,
            )
        }
    }

    private fun resolvePositions(states: List<TopLabelLaneGroupState>): List<TopLabelLaneConflict> {
        repeat(max(MAX_RELAXATION_PASSES, states.size * 2)) {
            var changed = false

            for (index in 0 until states.lastIndex) {
                val left = states[index]
                val right = states[index + 1]
                val requiredGap = requiredGap(left, right)
                val gap = right.left - left.right
                if (gap + EPSILON >= requiredGap) {
                    continue
                }

                var shortage = requiredGap - gap
                var leftCapacity = left.centerX - left.minCenterX
                var rightCapacity = right.maxCenterX - right.centerX
                val halfShortage = shortage / 2f

                val leftMove = min(halfShortage, leftCapacity)
                if (leftMove > EPSILON) {
                    left.centerX -= leftMove
                    shortage -= leftMove
                    leftCapacity -= leftMove
                    changed = true
                }

                val rightMove = min(halfShortage, rightCapacity)
                if (rightMove > EPSILON) {
                    right.centerX += rightMove
                    shortage -= rightMove
                    rightCapacity -= rightMove
                    changed = true
                }

                val extraRight = min(shortage, rightCapacity)
                if (extraRight > EPSILON) {
                    right.centerX += extraRight
                    shortage -= extraRight
                    changed = true
                }

                val extraLeft = min(shortage, leftCapacity)
                if (extraLeft > EPSILON) {
                    left.centerX -= extraLeft
                    shortage -= extraLeft
                    changed = true
                }
            }

            if (!changed) {
                return@repeat
            }
        }

        return buildList {
            for (index in 0 until states.lastIndex) {
                val left = states[index]
                val right = states[index + 1]
                val requiredGap = requiredGap(left, right)
                val gap = right.left - left.right
                if (gap + EPSILON < requiredGap) {
                    add(TopLabelLaneConflict(leftIndex = index, rightIndex = index + 1))
                }
            }
        }
    }

    private fun scaleOffenders(
        states: List<TopLabelLaneGroupState>,
        conflicts: List<TopLabelLaneConflict>,
    ): Boolean {
        var scaledAny = false
        for (conflict in conflicts) {
            val left = states[conflict.leftIndex]
            val right = states[conflict.rightIndex]
            if (right.group.textWidth <= EPSILON) {
                continue
            }

            val requiredGap = requiredGap(left, right)
            val maxTotalWidth = max(0f, 2f * (right.maxCenterX - left.right - requiredGap))
            val maxTextWidth = max(0f, maxTotalWidth - right.group.gapWidth)
            var targetScale = maxTextWidth / right.group.textWidth
            targetScale = min(right.scale, targetScale)
            targetScale = max(KeyVisualPolicy.TOP_F_G_LABEL_MIN_SCALE, targetScale)
            if (targetScale + EPSILON < right.scale) {
                right.scale = targetScale
                scaledAny = true
            }
        }
        return scaledAny
    }

    private fun requiredGap(
        left: TopLabelLaneGroupState,
        right: TopLabelLaneGroupState,
    ): Float {
        return KeyVisualPolicy.TOP_F_G_LABEL_HORIZONTAL_GAP * 2f
    }
}
