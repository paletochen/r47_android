# R47 Android Port: Master Design & Rebuild Guide

**Version:** 3.14 (Property-Backed Package And Version Contract)
**Status:** Maintainer reference for the current debug-build Android shell
**Target Platform:** Android (API Level 24+), current checked-in defaults target API 35 and arm64-v8a

---

**Goal:** This document captures the current technical architecture and rebuild contract of the R47 Android port. It serves as the technical reference for the checked-in Android shell and the maintainer rebuild guide for the current debug-build pipeline. Public checkouts contain the Android shell and tracked local overrides; `sync_public.sh` hydrates the authoritative core from upstream before rebuilds.

---

## 0. Package Identity And Version Contract

- The stable checked-in Android package identity MUST be
  `com.example.r47`.
- Debug snapshots MUST install as `com.example.r47.debug` through the
  app-module `applicationIdSuffix ".debug"`.
- The checked-in debug APK currently packages `arm64-v8a` only.
- Release version inputs MUST come from the Gradle properties
  `r47.versionCode` and `r47.versionName`.
- The checked-in defaults are `r47.versionCode=1` and `r47.versionName=0.1.0`.
- Debug builds append `-snapshot.<core>` to `versionName`, where `<core>` comes
  from `r47.coreVersion`.
- `build_android.sh` always passes `-Pr47.coreVersion=<sha>` and may also pass
  `-Pr47.versionCode=<n>` and `-Pr47.versionName=<name>` from the environment
  variables `R47_VERSION_CODE` and `R47_VERSION_NAME`.
- The debug APK output name remains `R47calculator-debug.apk`. That file name is
  a build artifact name, not the app identity.

---

## 1. System Architecture & Threading

The application employs a dual-thread model to decouple the computationally intensive C core from the high-frequency Android UI loop.

### 1.1. Thread Split & Persistence

- **UI Thread (Main)**:
  - Manages the `ReplicaOverlay` and standard Android View hierarchy.
  - Owns the currently visible activity instance and Android OS integration while `NativeCoreRuntime` owns the shared display refresh callback.
  - Handles asynchronous OS events (SAF pickers, Permissions, Insets).

- **Core Thread (Background)**:
  - **PERSISTENCE**: The engine loop, `isAppRunningShared`, and the shared task queue MUST reside in `NativeCoreRuntime` static state. This allows the engine to survive Activity recreations during PiP transitions.
  - **Unified Input Queue**: ALL user inputs (touch zones, physical keyboard, PiP events) MUST be routed through the `NativeCoreRuntime` `coreTasks: LinkedBlockingQueue<Runnable>` queue.
  - **Synchronous Save on Pause**: To prevent state loss during force-close, `onPause()` executes `saveStateNative()` synchronously by queuing it to the core thread and waiting via a `CountDownLatch`.
  - **Non-Blocking Program Loops**: To keep the app responsive during long programs, the main execution loop MUST call `yieldToAndroid()`.
  - Runs the native C core loop (`tick`) at a target interval of 10ms.
  - Blocks synchronously during SAF handshakes to maintain core logic simplicity.
  - **Activity Re-sync**: When a new Activity instance is created, it MUST call `updateNativeActivityRef()` to ensure the core thread always executes callbacks on the currently visible instance.

### 1.2. Function Visibility & Linkage

To support Android automation, specific core functions MUST be public:

- **Navigation**: `menuUp()` and `menuDown()` in `src/c47/keyboard.c` MUST be public and declared in `src/c47/softmenus.h`.
- **State**: `doSave()` in `src/c47/saveRestoreCalcState.c` MUST be public.

### 1.3. Synchronization & JNI Strategy

To prevent Application Not Responding (ANR) errors and deadlocks:

- **Mutex**: A native `screenMutex` MUST be `PTHREAD_MUTEX_RECURSIVE`.
- **Minimal Contention**: The UI thread and core thread must never hold the mutex during blocking I/O. Functions like `getDisplayPixels` MUST lock, `memcpy` data to a local buffer, and unlock IMMEDIATELY before processing.
- **Locking Strategy**:
  - **UI Thread Calls**: (`getDisplayPixels`, `saveStateNative`) MUST use `pthread_mutex_trylock`. If busy, the frame is skipped.
  - **Background Calls**: (`getXRegisterNative`, `sendSimKeyNative`) MUST use blocking `pthread_mutex_lock`.
- **SAF I/O Deadlock Prevention**: The `requestAndroidFile` native helper MUST fully release `screenMutex` while waiting for the Java result.

### 1.4. Advanced Threading & UI Synchronization

- **Timed Yielding (`yieldToAndroidWithMs`)**: The native `yieldToAndroidWithMs(ms)` function MUST be used for intentional delays. It fully unlocks the recursive mutex before sleeping to allow the UI thread to render.
- **PAUSE Implementation**: Long-running pauses are broken into 100ms yielding chunks to allow the UI to capture intermediate states.

---

## 2. Visual Specifications & Scaling

### 2.1. Dynamic Scaling & Layout Sync

- **Logical Canvas**: The live runtime uses one measured reference canvas, `1820 x 3403`. The checked-in `r47_texture.webp` and `r47_background.webp` assets share the same measured widths across density buckets: mdpi `360`, hdpi `540`, xhdpi `720`, xxhdpi `1080`, xxxhdpi `1440`.
- **Scaling Rule**: `ReplicaOverlay` fits the active shell contract inside the available window. `full_width` fits the trimmed visible frame (`42 / 49 / 42 / 56` logical-canvas units), while `physical` fits the full shell and caps that scale by the resolved shell bitmap width divided by `R47ReferenceGeometry.LOGICAL_CANVAS_WIDTH`.
- **Chrome Contract**: `chrome_mode` selects between the default `r47_texture` classic shell with hidden touch zones, the background-backed `r47_background` shell that keeps scene-driven labels and softkey text without Android-painted key surfaces, and the native-drawn chrome mode.
- **Positioning**: The active projected frame is centered inside the view, and the logical shell is offset by the active trim when `full_width` is selected. Areas outside the rounded shell remain filled by the activity background.

### 2.2. LCD Calibration

- **Live LCD Window**: All chrome modes use the same logical LCD window from
  `R47AndroidChromeGeometry`: left `86`, top `229`, width `1648`, height `903`.
- **LCD Buffer**: The native display buffer remains `400 x 240`; Android draws
  that bitmap into the projected logical LCD window.
- **Readability Bias**: The live Android LCD window is larger than the measured
  physical LCD opening on the reference image. That is intentional app UI
  policy, not a geometry parsing error.

---

## 3. Character Encoding & Clipboard

### 3.1. Encoding Artifact Prevention

Double-encoding occurs if symbols are converted to UTF-8 before the core `stringToUtf8` function.

- **Rule**: `ascii_clean` in `android_helpers.c` MUST NOT convert core symbols to UTF-8 prematurely.
- Every register string returned to JNI MUST pass through `stringToUtf8` exactly once at the end of the helper chain.

### 3.2. RPN Paste Mapping

- **Plus '+'**: Map to `ENTER` (37) to facilitate RPN complex number entry.
- **Locale**: `String.format` calls for simulated key IDs MUST use `java.util.Locale.US`.

---

## 4. File System & Storage Access Framework (SAF)

### 4.1. Work Directory & Organized Storage

To maintain parity with the hardware and simulator structures, users can select a "Work Directory" in preferences.

- **Subfolder Structure**: The app resolves standard subfolders: `STATE`, `PROGRAMS`, `SAVFILES`, and `SCREENS`.
- **Context-Aware SAF Pickers**: The `requestFile` bridge uses category IDs to open pickers directly in relevant subfolders.
- **Native Base Path**: `nativePreInit()` passes `filesDir.absolutePath` into `set_android_base_path()`. The native HAL must use that runtime path instead of assuming a package-specific internal directory.

### 4.2. High-Latency Mitigation (Buffering)

SAF descriptors are slow. Data is assembled in a local 16KB buffer and written in a single block operation.

---

## 5. Persistence & Lifecycle

### 5.1. Non-Destructive Save State

- **Buffer Backup Strategy**: `saveStateNative` uses a backup buffer to prevent transient system messages from being persisted in the visual state.
- **Force Dirty on Load**: Upon state restoration, all LCD rows are marked as dirty to force an immediate refresh.

---

## 6. Interactive Picture-in-Picture (PiP)

- **Aspect Ratio**: `4860:2667`.
- **Interactivity**: Touch events on the LCD map horizontally to keys F1-F6 in PiP mode.

---

## 7. Android Lifecycle & System UI

### 7.1. Full Screen Mode

A "Full Screen Mode" toggle in preferences handles immersive vs. normal window configurations, accommodating different Android navigation styles.

### 7.2. EXIT Logic

The `quitApp()` function respects the `force_close_on_exit` preference, allowing for either minimizing the app or terminating the process.

---

## 8. Robust Synchronization Strategy

To maintain custom work while pulling latest upstream changes, the `sync_public.sh` script MUST follow these rules:

1. **Backup All Custom Assets**: The `android/` directory is the primary asset requiring persistence.
2. **Upstream Reset**: The script pulls the math core from the authoritative source and populates the workspace.
3. **Local Patch Restoration**: Immediately after the upstream pull, the script re-applies the Android Port modifications. This ensures that the optimized code takes precedence over the generic core.

### 8.1. Android Native Staging

- `build_android.sh` remains the top-level Android debug-build entry point.
- `build_android.sh` MUST resolve one worker count from `R47_BUILD_JOBS`, then
  `CMAKE_BUILD_PARALLEL_LEVEL`, then the host CPU count, export that value as
  `CMAKE_BUILD_PARALLEL_LEVEL`, and thread it through `make`, `NINJAFLAGS`, and
  `gradlew --max-workers`.
- `build_android.sh` MAY pass `R47_SOURCE_REPOSITORY_URL` through as the Gradle
  property `r47.sourceRepositoryUrl` so redistributed APKs can point the About
  screen at the Android fork source for the build they convey.
- `build_android.sh` MUST NOT default the Android source commit to the
  synchronized upstream core hash. When `R47_SOURCE_COMMIT` is omitted, Gradle
  MUST infer the checked-out Android repo `HEAD` for `assets/SOURCE`.
- `build_android.sh` MAY also pass `R47_UPSTREAM_SOURCE_REPOSITORY_URL` and
  `R47_UPSTREAM_SOURCE_COMMIT` so the packaged `SOURCE` manifest records the
  synchronized upstream core revision ahead of the Android fork metadata.
- `build_android.sh` MUST pass the pinned xlsxio repository URL and commit
  through as `r47.xlsxioSourceRepositoryUrl` and `r47.xlsxioSourceCommit` so
  the packaged APK provenance matches the host toolchain that generated the
  fonts.
- After `make sim`, it MUST delegate native staging to `android/stage_native_sources.sh`.
- That staging step copies the synced `src/c47` tree, `dep/decNumberICU`, generated files, and mini-gmp inputs into `android/app/src/main/cpp`.
- The app-module Gradle build MUST generate `assets/COPYING`,
  `assets/LICENSE.txt`, and `assets/SOURCE`, with `COPYING` copied from the
  repo root, `LICENSE.txt` carrying the pinned xlsxio MIT license text, and
  `SOURCE` recording the Android repository URL plus commit plus the xlsxio
  repository URL plus commit for the packaged APK.
- When `r47.upstreamSourceRepositoryUrl` and `r47.upstreamSourceCommit` are
  supplied, `assets/SOURCE` MUST record that synchronized upstream core
  revision first and then the Android and xlsxio provenance lines.
- The default Android source URL is inferred from `git remote origin` when
  available, with a fallback of `https://github.com/ppigazzini/r47_android`.
  Distributors remain responsible for overriding it when the shipped APK
  corresponds to a different public Android source location.
- The Android GitHub Actions workflow in `.github/workflows/android-ci.yml`
  owns the main-branch snapshot lane. It MUST poll upstream daily, skip the
  scheduled release path when the resolved upstream commit already has a GitHub
  release tag in this repository, and keep publication downstream of successful
  simulator and Android build jobs.
- Android compatibility for upstream GTK, GDK, and Cairo includes MUST live in tracked Android stub headers under `android/app/src/main/cpp/c47-android/stubs` plus `android_mocks.h`. Do not reintroduce post-copy `sed` rewrites of staged sources.
- The About-version preference summary MUST come from the Gradle property `r47.coreVersion`, not from build-time edits of tracked XML resources.

---

## 9. Audio & Beeper Implementation

### 9.1. Timing Accuracy & Note Separation

To ensure beeper tones remain distinct and melodies are paced correctly:

- **Core Thread Cushion**: Use a scheduling cushion in the `_Buzz` JNI call for the Android OS.
- **Silence Tail**: Append silence to each tone buffer in the Kotlin audio thread.

---

## 10. External Keyboard Mapping

The application implements an extensive mapping table for external hardware keyboards. It covers many RPN shortcuts and calculator key IDs, but it remains an Android-specific contract rather than full GTK simulator shortcut parity.

---

## 11. Quick Slot Persistence

To ensure stable state switching, switches occur in a single background thread using atomic sequencing (`Save Old -> Update ID -> Load New`).

---

## 12. Android Compatibility

The native shared library MUST follow the supported Android NDK flexible-page-size
contract. In this repo, `android/app/build.gradle` passes
`-DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=ON` into CMake, and CI verifies both APK
zip alignment and ELF `LOAD` segment alignment. Do not rely on a project-local
hardcoded `-Wl,-z,max-page-size=16384` flag as the only contract.

With AGP 8.7.3 and NDK 29, the repo intentionally uses the preferred
uncompressed-native-library path and no longer carries
`useLegacyPackaging true`. Treat the current CI zip and ELF alignment checks as
the artifact-level evidence that justifies that simpler packaging contract.

---

## 13. Modern Performance & Polish

### 13.1. Core Loop Efficiency

- **Yield Frequency**: Programs call `yieldToAndroid()` every 10 steps to balance speed and responsiveness.
- **Native Pre-Init**: `nativePreInit()` configures the environment before UI setup.

### 13.2. High-Velocity Graphics

- **Lookup Table Blitting**: LCD-to-RGBA conversion uses a precomputed table for efficient pixel expansion.
- **Partial Invalidation**: `ReplicaOverlay` calculates a `dirtyRect` covering only changed pixels to reduce draw overhead.

### 13.3. High-Fidelity Audio & Haptics

- **Zero-GC Audio Engine**: Audio data is passed via primitive queues to avoid object allocations.
- **Anti-Pop Envelopes**: A 2ms linear Attack/Decay is applied to remove digital clicks.

---

## 14. System Integrity & Maintenance

### 14.1. Factory Reset Protocol

- **Scope**: Deletes application data while preserving user files in the external Work Directory.
- **Process**: Triggers an `intent` restart to ensure all static state is purged.
