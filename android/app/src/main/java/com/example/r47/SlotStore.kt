package com.example.r47

import android.content.Context

data class CalculatorSlot(val id: Int, var name: String, var uri: String?)

class SlotStore(private val context: Context) {
    companion object {
        const val APP_PREFS_NAME = "R47Prefs"
        const val SLOT_PREFS_NAME = "R47Slots"

        private const val KEY_SLOT_COUNT = "slot_count"
        private const val KEY_CURRENT_SLOT = "current_slot_id"
    }

    fun loadSlots(): MutableList<CalculatorSlot> {
        val prefs = slotPrefs()
        val count = prefs.getInt(KEY_SLOT_COUNT, 0)
        if (count == 0) {
            return mutableListOf(CalculatorSlot(0, "Slot 1 (Standard)", null)).also {
                saveSlots(it)
            }
        }

        return MutableList(count) { index ->
            val name = prefs.getString("slot_${index}_name", "Slot ${index + 1}") ?: "Slot ${index + 1}"
            val uri = prefs.getString("slot_${index}_uri", null)
            CalculatorSlot(index, name, uri)
        }
    }

    fun saveSlots(slots: List<CalculatorSlot>) {
        val editor = slotPrefs().edit()
        editor.clear()
        editor.putInt(KEY_SLOT_COUNT, slots.size)
        for (slot in slots) {
            editor.putString("slot_${slot.id}_name", slot.name)
            editor.putString("slot_${slot.id}_uri", slot.uri)
        }
        editor.apply()
    }

    fun readCurrentSlotId(): Int {
        return appPrefs().getInt(KEY_CURRENT_SLOT, 0)
    }

    fun writeCurrentSlotId(slotId: Int) {
        appPrefs().edit().putInt(KEY_CURRENT_SLOT, slotId).apply()
    }

    private fun appPrefs() = context.getSharedPreferences(APP_PREFS_NAME, Context.MODE_PRIVATE)

    private fun slotPrefs() = context.getSharedPreferences(SLOT_PREFS_NAME, Context.MODE_PRIVATE)
}