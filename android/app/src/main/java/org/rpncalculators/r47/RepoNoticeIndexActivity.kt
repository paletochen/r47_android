package org.rpncalculators.r47

import android.os.Bundle
import android.view.View
import android.view.ViewGroup
import android.widget.ArrayAdapter
import android.widget.ListView
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.google.android.material.dialog.MaterialAlertDialogBuilder
import org.json.JSONObject
import java.io.IOException

class RepoNoticeIndexActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_repo_notice_index)
        supportActionBar?.setDisplayHomeAsUpEnabled(true)
        title = getString(R.string.repo_notice_catalog_title)

        val entries = try {
            loadEntries()
        } catch (e: IOException) {
            showMissingAndFinish(getString(R.string.repo_notice_catalog_missing_message))
            return
        }

        val noticeList = findViewById<ListView>(R.id.notice_list)
        noticeList.adapter = object : ArrayAdapter<RepoNoticeEntry>(
            this,
            android.R.layout.simple_list_item_2,
            android.R.id.text1,
            entries
        ) {
            override fun getView(position: Int, convertView: View?, parent: ViewGroup): View {
                val view = super.getView(position, convertView, parent)
                val entry = getItem(position) ?: return view
                view.findViewById<TextView>(android.R.id.text1).text = entry.title
                view.findViewById<TextView>(android.R.id.text2).apply {
                    text = entry.summary
                    visibility = if (entry.summary.isBlank()) View.GONE else View.VISIBLE
                }
                return view
            }
        }

        noticeList.setOnItemClickListener { _, _, position, _ ->
            val entry = entries[position]
            startActivity(
                NoticeAssetActivity.createIntent(
                    this,
                    entry.title,
                    entry.assetPath,
                    entry.contentType
                )
            )
        }
    }

    override fun onSupportNavigateUp(): Boolean {
        finish()
        return true
    }

    private fun loadEntries(): List<RepoNoticeEntry> {
        val rawText = assets.open("repo-notices/NOTICE-INDEX.json").bufferedReader().use { it.readText() }
        val jsonObject = JSONObject(rawText)
        val entriesArray = jsonObject.getJSONArray("entries")
        val entries = mutableListOf<RepoNoticeEntry>()

        for (index in 0 until entriesArray.length()) {
            val entry = entriesArray.getJSONObject(index)
            entries += RepoNoticeEntry(
                title = entry.optString("title"),
                summary = entry.optString("summary"),
                assetPath = entry.optString("assetPath"),
                contentType = entry.optString("contentType", "text/plain")
            )
        }

        return entries
    }

    private fun showMissingAndFinish(message: String) {
        MaterialAlertDialogBuilder(this)
            .setTitle(R.string.repo_notice_catalog_title)
            .setMessage(message)
            .setPositiveButton(R.string.gpl_license_dialog_close) { _, _ ->
                finish()
            }
            .setOnDismissListener {
                finish()
            }
            .show()
    }

    private data class RepoNoticeEntry(
        val title: String,
        val summary: String,
        val assetPath: String,
        val contentType: String
    )
}
