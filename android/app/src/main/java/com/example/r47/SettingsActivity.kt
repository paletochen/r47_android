package com.example.r47

import android.content.Context
import android.content.Intent
import android.net.Uri
import android.os.Bundle
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.preference.Preference
import androidx.preference.PreferenceFragmentCompat
import androidx.preference.PreferenceManager
import com.google.android.material.dialog.MaterialAlertDialogBuilder

class SettingsActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.settings_activity)
        if (savedInstanceState == null) {
            supportFragmentManager
                .beginTransaction()
                .replace(R.id.settings, SettingsFragment())
                .commit()
        }
        supportActionBar?.setDisplayHomeAsUpEnabled(true)
        title = "Settings"
    }

    override fun onSupportNavigateUp(): Boolean {
        finish()
        return true
    }
}

class SettingsFragment : PreferenceFragmentCompat() {

    private fun formatUriPath(uriPath: String?): String {
        if (uriPath == null) return "Select a folder"
        // Replace /tree/primary: or /tree/1234-ABCD: with /
        return uriPath.replaceFirst("^/tree/.*?:".toRegex(), "/")
    }

    private val treeLauncher = registerForActivityResult(ActivityResultContracts.OpenDocumentTree()) { uri ->
        if (uri != null) {
            requireContext().contentResolver.takePersistableUriPermission(
                uri,
                Intent.FLAG_GRANT_READ_URI_PERMISSION or Intent.FLAG_GRANT_WRITE_URI_PERMISSION
            )
            val prefs = requireContext().getSharedPreferences("R47Prefs", Context.MODE_PRIVATE)
            prefs.edit().putString("work_directory_uri", uri.toString()).apply()
            
            val displayPath = formatUriPath(uri.path)
            findPreference<Preference>("work_directory")?.summary = displayPath
            
            MaterialAlertDialogBuilder(requireContext())
                .setTitle("Work Directory Set")
                .setMessage("Folder selected: $displayPath\nSubfolders (STATE, PROGRAMS, SAVFILES, SCREENS) will be created automatically.")
                .setPositiveButton("OK", null)
                .show()
        }
    }

    private val pdfLauncher = registerForActivityResult(ActivityResultContracts.GetContent()) { uri ->

        if (uri != null) {

            val intent = Intent(Intent.ACTION_VIEW).apply {

                setDataAndType(uri, "application/pdf")

                addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)

            }

            try {

                startActivity(intent)

            } catch (e: Exception) {

                MaterialAlertDialogBuilder(requireContext())

                    .setTitle("No PDF Viewer")

                    .setMessage("No application found to open PDF files.")

                    .setPositiveButton("OK", null)

                    .show()

            }

        }

    }



    override fun onCreatePreferences(savedInstanceState: Bundle?, rootKey: String?) {
        preferenceManager.sharedPreferencesName = "R47Prefs"
        setPreferencesFromResource(R.xml.root_preferences, rootKey)

        // Automatic trigger if coming from validation Snackbar
        if (requireActivity().intent.getBooleanExtra("trigger_work_dir_picker", false)) {
            treeLauncher.launch(null)
        }

        val prefs = requireContext().getSharedPreferences("R47Prefs", Context.MODE_PRIVATE)
        val currentUriStr = prefs.getString("work_directory_uri", null)
        if (currentUriStr != null) {
            try {
                val uri = Uri.parse(currentUriStr)
                findPreference<Preference>("work_directory")?.summary = formatUriPath(uri.path)
            } catch (e: Exception) {
                findPreference<Preference>("work_directory")?.summary = "Select a folder"
            }
        }

        findPreference<Preference>("work_directory")?.setOnPreferenceClickListener {
            treeLauncher.launch(null)
            true
        }

        findPreference<Preference>("factory_reset")?.setOnPreferenceClickListener {
            MaterialAlertDialogBuilder(requireContext())
                .setTitle("Confirm Reset")
                .setMessage("Wipe all internal data and restart app?\n\nNote: This will NOT delete any files in your selected Work Directory (STATE, PROGRAMS, etc.).")
                .setPositiveButton("Reset") { _, _ ->
                    // 1. Clear SharedPreferences in memory first
                    requireContext().getSharedPreferences("R47Prefs", Context.MODE_PRIVATE).edit().clear().commit()
                    requireContext().getSharedPreferences("R47Slots", Context.MODE_PRIVATE).edit().clear().commit()
                    
                    // 2. Delete all files in the app's internal data directory (except lib)
                    val dataDir = requireContext().filesDir.parentFile
                    dataDir?.listFiles()?.forEach { file ->
                        if (file.name != "lib") {
                            file.deleteRecursively()
                        }
                    }

                    // 3. Force a clean restart
                    val intent = requireContext().packageManager.getLaunchIntentForPackage(requireContext().packageName)
                    intent?.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TASK)
                    startActivity(intent)
                    
                    // Kill the process to ensure all static state is cleared
                    android.os.Process.killProcess(android.os.Process.myPid())
                    java.lang.System.exit(0)
                }
                .setNegativeButton("Cancel", null)
                .show()
            true
        }



        findPreference<Preference>("visit_gitlab")?.setOnPreferenceClickListener {

            startActivity(Intent(Intent.ACTION_VIEW, Uri.parse("https://gitlab.com/rpncalculators/c43")))

            true

        }



        findPreference<Preference>("view_wiki")?.setOnPreferenceClickListener {

            startActivity(Intent(Intent.ACTION_VIEW, Uri.parse("https://gitlab.com/rpncalculators/c43/-/wikis/home")))

            true

        }



        findPreference<Preference>("visit_swissmicros")?.setOnPreferenceClickListener {

            startActivity(Intent(Intent.ACTION_VIEW, Uri.parse("https://www.swissmicros.com")))

            true

        }



        findPreference<Preference>("view_manual")?.setOnPreferenceClickListener {

            pdfLauncher.launch("application/pdf")

            true

        }

    }

}
