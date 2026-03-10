# R47 Android Port: Master Design & Rebuild Guide

**Version:** 3.13 (Automatic Git-Based Versioning)
**Status:** Stable Production Candidate
**Target Platform:** Android (API Level 24+), Optimized for Pixel 10 (16KB Page Size)

---

**Goal:** This document captures the technical architecture and specifications of the R47 Android port. It serves as both the technical architectural reference and the restoration manual for the project.

---

## 1. System Architecture & Threading

The application employs a dual-thread model to decouple the computationally intensive C core from the high-frequency Android UI loop.

### 1.1. Thread Split & Persistence

- **UI Thread (Main)**:
  - Manages the `ReplicaOverlay` and standard Android View hierarchy.
  - Drives the `Choreographer` frame callback (60fps) for display updates.
  - Handles asynchronous OS events (SAF pickers, Permissions, Insets).

- **Core Thread (Background)**:
  - **PERSISTENCE**: The engine loop and `isAppRunningShared` flag MUST reside in a static context (`companion object`). This allows the engine to survive Activity recreations during PiP transitions.
  - **Unified Input Queue**: ALL user inputs (touch zones, physical keyboard, PiP events) MUST be routed through the `coreTasks: LinkedBlockingQueue<Runnable>` queue.
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

- **Logical Canvas**: 537 x 1005 pixels.
- **Scaling Rule**: `scale = ViewWidth / 537f`.
- **Positioning**: The hardware body is centered vertically. Areas above/below are filled with `#2B2A29`.

### 2.2. LCD Calibration

- **Viewport**: `(25.5, 67.5)` relative to the logical canvas.
- **Size**: `486 x 266.7` logical pixels.

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

---

## 9. Audio & Beeper Implementation

### 9.1. Timing Accuracy & Note Separation

To ensure beeper tones remain distinct and melodies are paced correctly:
- **Core Thread Cushion**: Use a scheduling cushion in the `_Buzz` JNI call for the Android OS.
- **Silence Tail**: Append silence to each tone buffer in the Kotlin audio thread.

---

## 10. External Keyboard Mapping

The application implements an extensive mapping table for external hardware keyboards, providing simulator parity for RPN shortcuts.

---

## 11. Quick Slot Persistence

To ensure stable state switching, switches occur in a single background thread using atomic sequencing (`Save Old -> Update ID -> Load New`).

---

## 12. Android Compatibility

The native shared library MUST be linked with `-Wl,-z,max-page-size=16384` to support modern Android kernels.

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
