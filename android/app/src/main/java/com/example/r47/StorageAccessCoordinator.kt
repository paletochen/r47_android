package com.example.r47

import android.app.Activity
import android.content.Intent
import android.content.SharedPreferences
import android.net.Uri
import android.os.Build
import android.provider.DocumentsContract
import android.util.Log
import android.view.View
import androidx.activity.ComponentActivity
import androidx.activity.result.contract.ActivityResultContracts
import com.google.android.material.dialog.MaterialAlertDialogBuilder
import com.google.android.material.snackbar.Snackbar

internal class StorageAccessCoordinator(
    private val activity: ComponentActivity,
    private val appPreferences: SharedPreferences,
    private val rootView: View,
    private val launchSettings: () -> Unit,
    private val onNativeFileSelected: (Int) -> Unit,
    private val onNativeFileCancelled: () -> Unit,
) {
    companion object {
        private const val TAG = "R47StorageAccess"
    }

    private val saveLauncher = activity.registerForActivityResult(
        ActivityResultContracts.StartActivityForResult()
    ) { result ->
        handleNativeFileResult(
            resultCode = result.resultCode,
            uri = result.data?.data,
            mode = "wt",
        )
    }

    private val loadLauncher = activity.registerForActivityResult(
        ActivityResultContracts.StartActivityForResult()
    ) { result ->
        handleNativeFileResult(
            resultCode = result.resultCode,
            uri = result.data?.data,
            mode = "r",
        )
    }

    fun handleResume() {
        val isFirstRun = appPreferences.getBoolean("first_setup", true)
        val hasWorkDirectory = WorkDirectory.readTreeUriString(activity) != null

        if (isFirstRun && !hasWorkDirectory) {
            showFirstRunDialog()
        } else {
            validateWorkDirectory()
        }
    }

    fun requestNativeFile(isSave: Boolean, defaultName: String, fileType: Int) {
        try {
            val initialUri = WorkDirectory.resolveSubfolder(
                contentResolver = activity.contentResolver,
                treeUriString = WorkDirectory.readTreeUriString(activity),
                fileType = fileType,
            )

            if (isSave) {
                saveLauncher.launch(buildSaveIntent(defaultName, initialUri))
            } else {
                loadLauncher.launch(buildLoadIntent(initialUri))
            }
        } catch (error: Exception) {
            Log.e(TAG, "Failed to launch SAF", error)
            onNativeFileCancelled()
        }
    }

    private fun handleNativeFileResult(resultCode: Int, uri: Uri?, mode: String) {
        if (resultCode != Activity.RESULT_OK || uri == null) {
            onNativeFileCancelled()
            return
        }

        try {
            activity.contentResolver.openFileDescriptor(uri, mode)?.use { fileDescriptor ->
                onNativeFileSelected(fileDescriptor.detachFd())
            } ?: onNativeFileCancelled()
        } catch (error: Exception) {
            Log.e(TAG, "Failed to open selected SAF file", error)
            onNativeFileCancelled()
        }
    }

    private fun buildSaveIntent(defaultName: String, initialUri: Uri?): Intent {
        return Intent(Intent.ACTION_CREATE_DOCUMENT).apply {
            addCategory(Intent.CATEGORY_OPENABLE)
            type = when {
                defaultName.endsWith(".bmp") -> "image/bmp"
                defaultName.endsWith(".rtf") -> "application/rtf"
                defaultName.endsWith(".s47") -> "application/octet-stream"
                defaultName.endsWith(".p47") -> "application/octet-stream"
                defaultName.endsWith(".sav") -> "application/octet-stream"
                else -> "*/*"
            }
            putExtra(Intent.EXTRA_TITLE, defaultName)
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O && initialUri != null) {
                putExtra(DocumentsContract.EXTRA_INITIAL_URI, initialUri)
            }
        }
    }

    private fun buildLoadIntent(initialUri: Uri?): Intent {
        return Intent(Intent.ACTION_OPEN_DOCUMENT).apply {
            addCategory(Intent.CATEGORY_OPENABLE)
            type = "*/*"
            putExtra(
                Intent.EXTRA_MIME_TYPES,
                arrayOf("application/octet-stream", "text/plain"),
            )
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O && initialUri != null) {
                putExtra(DocumentsContract.EXTRA_INITIAL_URI, initialUri)
            }
        }
    }

    private fun showFirstRunDialog() {
        MaterialAlertDialogBuilder(activity)
            .setTitle("Welcome to R47")
            .setMessage(
                "To get started, please select a 'Work Directory'.\n\n" +
                    "This folder will be used to organize your Programs, State files, " +
                    "and Screenshots safely on your device storage."
            )
            .setPositiveButton("Select Folder") { _, _ ->
                appPreferences.edit().putBoolean("first_setup", false).apply()
                launchSettings()
            }
            .setNegativeButton("Later") { _, _ ->
                appPreferences.edit().putBoolean("first_setup", false).apply()
                validateWorkDirectory()
            }
            .setCancelable(false)
            .show()
    }

    private fun validateWorkDirectory() {
        val treeUriString = WorkDirectory.readTreeUriString(activity)
        if (treeUriString == null) {
            showWorkDirectoryMissingSnackbar("Work Directory not set")
            return
        }

        if (!WorkDirectory.isAccessible(activity.contentResolver, treeUriString)) {
            showWorkDirectoryMissingSnackbar("Work Directory is no longer accessible")
        }
    }

    private fun showWorkDirectoryMissingSnackbar(message: String) {
        Snackbar.make(rootView, message, Snackbar.LENGTH_INDEFINITE)
            .setAction("SET") {
                launchSettings()
            }
            .show()
    }
}