package org.rpncalculators.r47

import android.content.ContentResolver
import android.content.Context
import android.media.MediaScannerConnection
import android.net.Uri
import android.os.Environment
import android.os.ParcelFileDescriptor
import android.provider.DocumentsContract
import android.util.Log

import java.io.File
import java.io.FileOutputStream
import java.util.concurrent.ConcurrentHashMap

object WorkDirectory {
    private const val TAG = "R47WorkDir"

    const val PREFS_NAME = SlotStore.APP_PREFS_NAME
    const val KEY_TREE_URI = "work_directory_uri"

    internal data class WriteSession(
        val uri: Uri,
        val mode: String,
        val stageFile: File,
    )

    private val documentUriCache = ConcurrentHashMap<String, Uri>()
    private val activeWriteSessions = ConcurrentHashMap<Int, WriteSession>()

    fun openWriteStagingFd(context: Context?, uri: Uri, mode: String): Int {
        return try {
            val cacheDir = context?.cacheDir
            val stageFile = if (cacheDir != null) {
                File.createTempFile("r47_saf_", ".tmp", cacheDir)
            } else {
                File.createTempFile("r47_saf_", ".tmp")
            }
            val pfd = ParcelFileDescriptor.open(
                stageFile,
                ParcelFileDescriptor.MODE_READ_WRITE or
                    ParcelFileDescriptor.MODE_CREATE or
                    ParcelFileDescriptor.MODE_TRUNCATE,
            )
            val fd = pfd.detachFd()
            activeWriteSessions[fd] = WriteSession(uri = uri, mode = mode, stageFile = stageFile)
            fd
        } catch (error: Exception) {
            Log.e(TAG, "openWriteStagingFd failed for $uri (mode=$mode)", error)
            -1
        }
    }

    internal fun resolveFileSystemPathFromDocumentId(docId: String?, defaultPrimaryRoot: String = "/storage/emulated/0"): String? {
        if (docId.isNullOrEmpty()) {
            return null
        }
        val parts = docId.split(":", limit = 2)
        if (parts.size != 2) {
            return null
        }
        val volume = parts[0]
        val relPath = parts[1].trimStart('/')
        return if (volume.equals("primary", ignoreCase = true)) {
            if (relPath.isEmpty()) defaultPrimaryRoot else "$defaultPrimaryRoot/$relPath"
        } else {
            if (relPath.isEmpty()) "/storage/$volume" else "/storage/$volume/$relPath"
        }
    }

    internal fun resolveFileSystemPath(uri: Uri): String? {
        val docId = try {
            DocumentsContract.getDocumentId(uri)
        } catch (_: Exception) {
            try {
                DocumentsContract.getTreeDocumentId(uri)
            } catch (_: Exception) {
                null
            }
        } ?: return null

        val primaryRoot = try {
            Environment.getExternalStorageDirectory()?.absolutePath
        } catch (_: Exception) {
            null
        } ?: "/storage/emulated/0"

        return resolveFileSystemPathFromDocumentId(docId, primaryRoot)
    }

    fun notifyFileUpdated(context: Context, fd: Int) {
        val session = activeWriteSessions.remove(fd) ?: return
        val uri = session.uri
        try {
            val openMode = if (session.mode.contains("a")) "wa" else "wt"
            val outStream = try {
                context.contentResolver.openOutputStream(uri, openMode)
            } catch (error: Exception) {
                if (openMode == "wt") {
                    context.contentResolver.openOutputStream(uri, "w")
                } else {
                    throw error
                }
            }
            outStream?.use { out ->
                session.stageFile.inputStream().use { input ->
                    input.copyTo(out)
                }
                out.flush()
                if (out is FileOutputStream) {
                    try {
                        out.fd.sync()
                    } catch (_: Exception) {
                    }
                }
            }
            Log.i(TAG, "notifyFileUpdated: committed ${session.stageFile.length()} bytes to $uri (mode=${session.mode})")
        } catch (error: Exception) {
            Log.e(TAG, "notifyFileUpdated: failed committing staged write to $uri", error)
        } finally {
            try {
                session.stageFile.delete()
            } catch (_: Exception) {
            }
        }

        try {
            context.contentResolver.notifyChange(uri, null)
        } catch (_: Exception) {
        }

        val realPath = resolveFileSystemPath(uri)
        if (realPath != null) {
            try {
                val file = File(realPath)
                Log.i(TAG, "notifyFileUpdated: triggering MediaScanner for $realPath (size=${file.length()})")
                MediaScannerConnection.scanFile(
                    context.applicationContext,
                    arrayOf(realPath),
                    null,
                ) { scannedPath, scannedUri ->
                    Log.i(TAG, "MediaScanner finished for $scannedPath -> $scannedUri")
                }
            } catch (error: Exception) {
                Log.w(TAG, "notifyFileUpdated: MediaScanner failed for $realPath: ${error.message}")
            }
        }
    }

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

    const val KEY_PENDING_AUTOLOAD = "pending_autoload_autosave"

    fun setPendingAutoLoad(context: Context, pending: Boolean) {
        context.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)
            .edit()
            .putBoolean(KEY_PENDING_AUTOLOAD, pending)
            .apply()
    }

    fun consumePendingAutoLoad(context: Context): Boolean {
        val prefs = context.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)
        val pending = prefs.getBoolean(KEY_PENDING_AUTOLOAD, false)
        if (pending) {
            prefs.edit().putBoolean(KEY_PENDING_AUTOLOAD, false).apply()
        }
        return pending
    }

    fun ensureAllSubfolders(context: Context, treeUriString: String? = readTreeUriString(context)) {
        if (treeUriString.isNullOrEmpty()) {
            return
        }
        val contentResolver = context.contentResolver
        for (type in 0..5) {
            try {
                resolveSubfolder(contentResolver, treeUriString, type)
            } catch (error: Exception) {
                Log.e(TAG, "Failed ensuring subfolder for type $type", error)
            }
        }
    }

    fun findAutoSaveFile(contentResolver: ContentResolver, treeUriString: String?): String? {
        if (treeUriString.isNullOrEmpty()) {
            return null
        }
        return try {
            val folderUri = resolveSubfolder(contentResolver, treeUriString, 2) ?: return null
            val treeUri = Uri.parse(treeUriString)
            val folderDocId = DocumentsContract.getDocumentId(folderUri)
            val childrenUri = DocumentsContract.buildChildDocumentsUriUsingTree(treeUri, folderDocId)

            var foundName: String? = null
            contentResolver.query(
                childrenUri,
                arrayOf(DocumentsContract.Document.COLUMN_DISPLAY_NAME),
                null,
                null,
                null,
            )?.use { cursor ->
                while (cursor.moveToNext()) {
                    val name = cursor.getString(0)
                    if (name.equals("R47auto.sav", ignoreCase = true)) {
                        foundName = "R47auto.sav"
                        break
                    } else if (name.equals("C47auto.sav", ignoreCase = true) && foundName == null) {
                        foundName = "C47auto.sav"
                    }
                }
            }
            foundName
        } catch (error: Exception) {
            Log.e(TAG, "Failed finding auto-save file in SAVFILES", error)
            null
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
                    val name = cursor.getString(0)
                    if (name.equals(subfolderName, ignoreCase = true) ||
                        (fileType == 2 && name.equals("SAVEFILES", ignoreCase = true))) {
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

            if (docUri != null) {
                if (mode.contains("w") || mode.contains("a")) {
                    val exists = try {
                        contentResolver.openFileDescriptor(docUri!!, "r")?.use { true } ?: false
                    } catch (_: Exception) {
                        false
                    }
                    if (exists) {
                        val fd = openWriteStagingFd(null, docUri!!, mode)
                        if (fd >= 0) {
                            Log.i(TAG, "openDirectDocumentFd: staged cached $fileName with fd $fd (fileType=$fileType, mode=$mode)")
                            return fd
                        }
                    }
                } else {
                    val cachedPfd = try {
                        contentResolver.openFileDescriptor(docUri!!, mode)
                    } catch (_: Exception) {
                        null
                    }
                    if (cachedPfd != null) {
                        val fd = cachedPfd.detachFd()
                        Log.i(TAG, "openDirectDocumentFd: opened cached $fileName with fd $fd (fileType=$fileType, mode=$mode)")
                        return fd
                    }
                }
                Log.w(TAG, "openDirectDocumentFd: cached URI stale for $fileName, resolving fresh")
                documentUriCache.remove(cacheKey)
                docUri = null
            }

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
                    if (cursor.getString(0).equals(fileName, ignoreCase = true)) {
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
                    fileName.endsWith(".txt", ignoreCase = true) -> "text/plain"
                    fileName.endsWith(".sav", ignoreCase = true) -> "application/octet-stream"
                    fileName.endsWith(".tsv", ignoreCase = true) -> "text/tab-separated-values"
                    fileName.endsWith(".csv", ignoreCase = true) -> "text/comma-separated-values"
                    else -> "application/octet-stream"
                }
                docUri = DocumentsContract.createDocument(
                    contentResolver,
                    folderUri,
                    mimeType,
                    fileName,
                ) ?: DocumentsContract.createDocument(
                    contentResolver,
                    folderUri,
                    "text/plain",
                    fileName,
                ) ?: return -1
            }

            documentUriCache[cacheKey] = docUri!!

            if (mode.contains("w") || mode.contains("a")) {
                val fd = openWriteStagingFd(null, docUri!!, mode)
                if (fd < 0) {
                    documentUriCache.remove(cacheKey)
                    return -1
                }
                Log.i(TAG, "openDirectDocumentFd: staged $fileName with fd $fd (fileType=$fileType, mode=$mode)")
                return fd
            }

            val pfd = contentResolver.openFileDescriptor(docUri!!, mode) ?: run {
                documentUriCache.remove(cacheKey)
                Log.e(TAG, "openDirectDocumentFd: openFileDescriptor returned null for $fileName (mode=$mode)")
                return -1
            }
            val fd = pfd.detachFd()
            Log.i(TAG, "openDirectDocumentFd: opened $fileName with fd $fd (fileType=$fileType, mode=$mode)")
            fd
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
                    if (cursor.getString(0).equals(fileName, ignoreCase = true)) {
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