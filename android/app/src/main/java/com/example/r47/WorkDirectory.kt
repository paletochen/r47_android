package com.example.r47

import android.content.ContentResolver
import android.content.Context
import android.net.Uri
import android.provider.DocumentsContract
import android.util.Log

object WorkDirectory {
    private const val TAG = "R47WorkDir"

    const val PREFS_NAME = SlotStore.APP_PREFS_NAME
    const val KEY_TREE_URI = "work_directory_uri"

    fun readTreeUriString(context: Context): String? {
        return context.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)
            .getString(KEY_TREE_URI, null)
    }

    fun writeTreeUriString(context: Context, uri: Uri) {
        context.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)
            .edit()
            .putString(KEY_TREE_URI, uri.toString())
            .apply()
    }

    fun formatDisplayPath(uriPath: String?): String {
        if (uriPath == null) {
            return "Select a folder"
        }

        return uriPath.replaceFirst("^/tree/.*?:".toRegex(), "/")
    }

    fun isAccessible(contentResolver: ContentResolver, treeUriString: String?): Boolean {
        if (treeUriString.isNullOrEmpty()) {
            return false
        }

        val treeUri = try {
            Uri.parse(treeUriString)
        } catch (error: Exception) {
            Log.w(TAG, "Invalid work directory URI: ${error.message}")
            return false
        }

        return try {
            val hasPermission = contentResolver.persistedUriPermissions.any {
                it.uri == treeUri && it.isWritePermission
            }
            if (!hasPermission) {
                return false
            }

            val documentId = DocumentsContract.getTreeDocumentId(treeUri)
            val childrenUri = DocumentsContract.buildChildDocumentsUriUsingTree(treeUri, documentId)
            contentResolver.query(
                childrenUri,
                arrayOf(DocumentsContract.Document.COLUMN_DOCUMENT_ID),
                null,
                null,
                null
            )?.use {
                true
            } ?: false
        } catch (error: Exception) {
            Log.w(TAG, "Work directory validation failed: ${error.message}")
            false
        }
    }

    fun resolveSubfolder(
        contentResolver: ContentResolver,
        treeUriString: String?,
        fileType: Int,
    ): Uri? {
        if (treeUriString.isNullOrEmpty()) {
            return null
        }

        val treeUri = try {
            Uri.parse(treeUriString)
        } catch (error: Exception) {
            Log.e(TAG, "Invalid work directory URI", error)
            return null
        }

        val subfolderName = subfolderNameFor(fileType) ?: return treeUri

        return try {
            val rootDocId = DocumentsContract.getTreeDocumentId(treeUri)
            val rootDocumentUri = DocumentsContract.buildDocumentUriUsingTree(treeUri, rootDocId)
            val childrenUri = DocumentsContract.buildChildDocumentsUriUsingTree(treeUri, rootDocId)
            var folderUri: Uri? = null

            contentResolver.query(
                childrenUri,
                arrayOf(
                    DocumentsContract.Document.COLUMN_DISPLAY_NAME,
                    DocumentsContract.Document.COLUMN_DOCUMENT_ID,
                ),
                null,
                null,
                null,
            )?.use { cursor ->
                while (cursor.moveToNext()) {
                    if (cursor.getString(0) == subfolderName) {
                        folderUri = DocumentsContract.buildDocumentUriUsingTree(treeUri, cursor.getString(1))
                        break
                    }
                }
            }

            if (folderUri == null) {
                folderUri = DocumentsContract.createDocument(
                    contentResolver,
                    rootDocumentUri,
                    DocumentsContract.Document.MIME_TYPE_DIR,
                    subfolderName,
                )
            }

            folderUri ?: treeUri
        } catch (error: Exception) {
            Log.e(TAG, "Error resolving subfolder $subfolderName", error)
            treeUri
        }
    }

    private fun subfolderNameFor(fileType: Int): String? {
        return when (fileType) {
            0 -> "STATE"
            1 -> "PROGRAMS"
            2 -> "SAVFILES"
            3 -> "SCREENS"
            else -> null
        }
    }
}