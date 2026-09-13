package org.rpncalculators.r47

import android.content.ContentResolver
import android.content.Context
import android.net.Uri
import android.provider.DocumentsContract
import android.util.Log

import java.util.concurrent.ConcurrentHashMap

object WorkDirectory {
    private const val TAG = "R47WorkDir"

    const val PREFS_NAME = SlotStore.APP_PREFS_NAME
    const val KEY_TREE_URI = "work_directory_uri"

    private val documentUriCache = ConcurrentHashMap<String, Uri>()

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
            4 -> "DATA"
            5 -> "PRINT"
            else -> null
        }
    }

    fun openDirectDocumentFd(
        contentResolver: ContentResolver,
        treeUriString: String?,
        fileType: Int,
        fileName: String,
        mode: String,
    ): Int {
        if (treeUriString.isNullOrEmpty()) {
            return -1
        }

        return try {
            val cacheKey = "$treeUriString|$fileType|$fileName"
            var docUri: Uri? = documentUriCache[cacheKey]

            if (docUri == null) {
                val folderUri = resolveSubfolder(contentResolver, treeUriString, fileType) ?: return -1
                val treeUri = Uri.parse(treeUriString)
                val folderDocId = DocumentsContract.getDocumentId(folderUri)
                val childrenUri = DocumentsContract.buildChildDocumentsUriUsingTree(treeUri, folderDocId)

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
                        if (cursor.getString(0) == fileName) {
                            docUri = DocumentsContract.buildDocumentUriUsingTree(treeUri, cursor.getString(1))
                            break
                        }
                    }
                }

                if (docUri == null) {
                    if (!mode.contains("w")) {
                        return -1
                    }
                    val mimeType = when {
                        fileName.endsWith(".txt") -> "text/plain"
                        fileName.endsWith(".sav") -> "application/octet-stream"
                        fileName.endsWith(".tsv") -> "text/tab-separated-values"
                        else -> "application/octet-stream"
                    }
                    docUri = DocumentsContract.createDocument(
                        contentResolver,
                        folderUri,
                        mimeType,
                        fileName,
                    ) ?: return -1
                }

                documentUriCache[cacheKey] = docUri!!
            }

            val pfd = contentResolver.openFileDescriptor(docUri!!, mode) ?: run {
                documentUriCache.remove(cacheKey)
                return -1
            }
            pfd.detachFd()
        } catch (error: Exception) {
            val cacheKey = "$treeUriString|$fileType|$fileName"
            documentUriCache.remove(cacheKey)
            Log.e(TAG, "Failed openDirectDocumentFd for $fileName (fileType=$fileType, mode=$mode)", error)
            -1
        }
    }

    fun deleteDirectDocument(
        contentResolver: ContentResolver,
        treeUriString: String?,
        fileType: Int,
        fileName: String,
    ): Boolean {
        if (treeUriString.isNullOrEmpty()) {
            return false
        }

        val cacheKey = "$treeUriString|$fileType|$fileName"
        documentUriCache.remove(cacheKey)

        return try {
            val folderUri = resolveSubfolder(contentResolver, treeUriString, fileType) ?: return false
            val treeUri = Uri.parse(treeUriString)
            val folderDocId = DocumentsContract.getDocumentId(folderUri)
            val childrenUri = DocumentsContract.buildChildDocumentsUriUsingTree(treeUri, folderDocId)

            var docUri: Uri? = null
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
                    if (cursor.getString(0) == fileName) {
                        docUri = DocumentsContract.buildDocumentUriUsingTree(treeUri, cursor.getString(1))
                        break
                    }
                }
            }

            if (docUri != null) {
                DocumentsContract.deleteDocument(contentResolver, docUri!!)
            } else {
                false
            }
        } catch (error: Exception) {
            Log.e(TAG, "Failed deleteDirectDocument for $fileName in fileType=$fileType", error)
            false
        }
    }
}