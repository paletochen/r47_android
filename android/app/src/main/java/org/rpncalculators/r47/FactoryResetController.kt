package org.rpncalculators.r47

import android.app.AlarmManager
import android.app.PendingIntent
import android.content.Context
import android.content.Intent
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import java.io.File

internal class FactoryResetController(
    private val activity: AppCompatActivity,
    private val onResetRequested: () -> Unit,
    private val onDestroyFactoryReset: () -> Unit,
    private val onDestroyFinish: () -> Unit,
) {
    companion object {
        private const val TAG = "R47FactoryReset"
        private const val ACTION_FACTORY_RESET = "org.rpncalculators.r47.action.FACTORY_RESET"
        private const val FACTORY_RESET_RESTART_DELAY_MS = 250L
        private const val FACTORY_RESET_RESTART_REQUEST_CODE = 4701

        fun createIntent(context: Context): Intent {
            return Intent(context, MainActivity::class.java).apply {
                action = ACTION_FACTORY_RESET
                addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP or Intent.FLAG_ACTIVITY_SINGLE_TOP)
            }
        }
    }

    var isResetInProgress = false
        private set

    fun isFactoryResetIntent(intent: Intent?): Boolean {
        return intent?.action == ACTION_FACTORY_RESET
    }

    fun handleResetRequest() {
        if (isResetInProgress) {
            return
        }

        val relaunchIntent = activity.packageManager.getLaunchIntentForPackage(activity.packageName)?.apply {
            addFlags(Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TASK)
        }

        if (relaunchIntent == null) {
            Log.e(TAG, "Factory reset requested without a launch intent")
            return
        }

        isResetInProgress = true
        scheduleRestart(relaunchIntent)
        onResetRequested()
        activity.finishAffinity()
        activity.finishAndRemoveTask()
    }

    fun handleDestroy(shouldStopApp: Boolean) {
        if (isResetInProgress) {
            onDestroyFactoryReset()
            clearInternalAppData()
        } else if (shouldStopApp) {
            onDestroyFinish()
        }
    }

    private fun scheduleRestart(relaunchIntent: Intent) {
        val alarmManager = activity.getSystemService(AlarmManager::class.java) ?: return
        val restartIntent = PendingIntent.getActivity(
            activity,
            FACTORY_RESET_RESTART_REQUEST_CODE,
            relaunchIntent,
            PendingIntent.FLAG_CANCEL_CURRENT or PendingIntent.FLAG_IMMUTABLE,
        )
        val triggerAtMillis = System.currentTimeMillis() + FACTORY_RESET_RESTART_DELAY_MS
        alarmManager.setExactAndAllowWhileIdle(AlarmManager.RTC, triggerAtMillis, restartIntent)
    }

    private fun clearInternalAppData() {
        activity.deleteSharedPreferences(SlotStore.APP_PREFS_NAME)
        activity.deleteSharedPreferences(SlotStore.SLOT_PREFS_NAME)
        activity.deleteSharedPreferences(WorkDirectory.PREFS_NAME)
        activity.databaseList().forEach { activity.deleteDatabase(it) }
        clearDirectoryContents(activity.filesDir)
        clearDirectoryContents(activity.cacheDir)
        clearDirectoryContents(activity.codeCacheDir)
        clearDirectoryContents(activity.noBackupFilesDir)
    }

    private fun clearDirectoryContents(directory: File?) {
        directory?.listFiles()?.forEach { child ->
            if (!child.deleteRecursively()) {
                Log.w(TAG, "Factory reset could not delete ${child.absolutePath}")
            }
        }
    }
}
