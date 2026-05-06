package org.rpncalculators.r47

import android.content.Context
import android.content.Intent
import android.os.Bundle
import android.view.View
import android.webkit.WebView
import android.widget.ScrollView
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.google.android.material.dialog.MaterialAlertDialogBuilder
import java.io.IOException

class NoticeAssetActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_notice_asset)
        supportActionBar?.setDisplayHomeAsUpEnabled(true)

        val titleText = intent.getStringExtra(EXTRA_TITLE)
            ?: getString(R.string.repo_notice_asset_fallback_title)
        title = titleText

        val assetPath = intent.getStringExtra(EXTRA_ASSET_PATH)
        if (assetPath.isNullOrBlank()) {
            showMissingAndFinish(getString(R.string.repo_notice_asset_missing_message))
            return
        }

        val contentType = intent.getStringExtra(EXTRA_CONTENT_TYPE) ?: "text/plain"
        val textPrefix = intent.getStringExtra(EXTRA_TEXT_PREFIX)
        val textContainer = findViewById<ScrollView>(R.id.notice_text_container)
        val textView = findViewById<TextView>(R.id.notice_text)
        val webView = findViewById<WebView>(R.id.notice_webview)

        val content = try {
            assets.open(assetPath).bufferedReader().use { it.readText() }
        } catch (e: IOException) {
            showMissingAndFinish(getString(R.string.repo_notice_asset_missing_message))
            return
        }

        if (contentType.contains("html", ignoreCase = true)) {
            textContainer.visibility = View.GONE
            webView.visibility = View.VISIBLE
            webView.loadDataWithBaseURL(
                "file:///android_asset/$assetPath",
                content,
                "text/html",
                "UTF-8",
                null
            )
        } else {
            webView.visibility = View.GONE
            textContainer.visibility = View.VISIBLE
            textView.text = if (textPrefix.isNullOrBlank()) {
                content
            } else {
                buildString {
                    append(textPrefix)
                    append("\n\n")
                    append(content)
                }
            }
        }
    }

    override fun onSupportNavigateUp(): Boolean {
        finish()
        return true
    }

    private fun showMissingAndFinish(message: String) {
        MaterialAlertDialogBuilder(this)
            .setTitle(R.string.repo_notice_asset_fallback_title)
            .setMessage(message)
            .setPositiveButton(R.string.gpl_license_dialog_close) { _, _ ->
                finish()
            }
            .setOnDismissListener {
                finish()
            }
            .show()
    }

    companion object {
        private const val EXTRA_TITLE = "title"
        private const val EXTRA_ASSET_PATH = "asset_path"
        private const val EXTRA_CONTENT_TYPE = "content_type"
        private const val EXTRA_TEXT_PREFIX = "text_prefix"

        fun createIntent(
            context: Context,
            title: String,
            assetPath: String,
            contentType: String,
            textPrefix: String? = null,
        ): Intent {
            return Intent(context, NoticeAssetActivity::class.java).apply {
                putExtra(EXTRA_TITLE, title)
                putExtra(EXTRA_ASSET_PATH, assetPath)
                putExtra(EXTRA_CONTENT_TYPE, contentType)
                putExtra(EXTRA_TEXT_PREFIX, textPrefix)
            }
        }
    }
}
