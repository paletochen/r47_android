# R47 Android Port: Master Design & Rebuild Guide

**Version:** 3.13 (Automatic Git-Based Versioning)
**Status:** Stable Production Candidate
**Target Platform:** Android (API Level 24+), Optimized for Pixel 10 (16KB Page Size)

---

## 14. Modern Performance & Polish (Groups A, B, C)

### 14.1. Core Loop Efficiency (Group A)
To balance high-speed RPN execution with UI responsiveness:
- **Yield Frequency**: `runProgram` (lblGtoXeq.c) MUST call `yieldToAndroid()` exactly every **10 steps**. yielding every step causes a 10x increase in JNI context-switch overhead, leading to jitter.
- **Native Pre-Init**: The `nativePreInit()` JNI call configures base paths and GMP memory functions BEFORE `initNative()` is called, ensuring the core environment is stable before UI setup.

### 14.2. High-Velocity Graphics (Group B)
To achieve fluid 60fps plotting (e.g. in SPIRAL):
- **Lookup Table Blitting**: LCD-to-RGBA conversion uses a precomputed 256-entry table. Each byte (8 bits) is expanded to 8 pixels in a single memory operation.
- **Bit-Order Awareness**: Core bit-order is right-to-left. Bit 0 (rightmost) must map to the highest memory address in the 8-pixel chunk.
- **Partial Invalidation**: `ReplicaOverlay` calculates a `dirtyRect` covering only changed pixels. Calling `invalidate(dirtyRect)` drastically reduces GPU/CPU draw overhead.

### 14.3. High-Fidelity Audio & Haptics (Group C)
- **Zero-GC Audio Engine**: Audio data is passed via primitive `Long` queues (packed frequency/duration) to avoid `Pair` object allocations. The audio thread reuses a single `ShortArray` buffer, eliminating Garbage Collection stutters.
- **Anti-Pop Envelopes**: A 2ms linear Attack/Decay is applied to all beeper tones to remove digital clicks.
- **Mechanical Haptics**: Uses custom waveforms scaled by a user-defined `hapticIntensity` (0-255). Always guard vibration calls with `intensity > 0` to prevent system crashes.

### 14.4. Storage Resilience & Onboarding
- **Proactive Validation**: `MainActivity.onResume()` validates the "Work Directory" URI accessibility.
- **Self-Healing UI**: If storage is broken or unconfigured, a Snackbar provides a direct "SET" action that triggers the directory picker.
- **First-Run Setup**: A one-time welcome dialog guides users to select their storage folder immediately upon a clean install or factory reset.

---

## 15. System Integrity & Maintenance

### 15.1. Factory Reset Protocol
To ensure a clean slate while preserving user files:
- **Scope**: Deletes `SharedPreferences` and the internal data directory (`files/`, `shared_prefs/`, `cache/`).
- **Safety**: Explicitly preserves the `lib/` directory and NEVER touches the external Work Directory.
- **Process**: Triggers an `intent` restart followed by `Process.killProcess(myPid())` to ensure all static state is purged.

**Goal:** This document captures **every single detail** of the R47 Android port. It serves as both the technical architectural reference and the step-by-step restoration manual for rebuilding the project from a clean master pull.

---

## 1. System Architecture & Threading

The application employs a dual-thread model to decouple the computationally intensive C core from the high-frequency Android UI loop.

### 1.1. Thread Split & Persistence
*   **UI Thread (Main)**:
    *   Manages the `ReplicaOverlay` and standard Android View hierarchy.
    *   Drives the `Choreographer` frame callback (60fps) for display updates.
    *   Handles asynchronous OS events (SAF pickers, Permissions, Insets).
*   **Core Thread (Background)**:
    *   **PERSISTENCE**: The engine loop and `isAppRunningShared` flag MUST reside in a static context (`companion object`). This allows the engine to survive Activity recreations during PiP transitions.
    *   **Unified Input Queue**: ALL user inputs (touch zones, physical keyboard, PiP events) MUST be routed through the `coreTasks: LinkedBlockingQueue<Runnable>` queue. The legacy `keyQueue` has been removed. This ensures strict ordering of key events relative to other core tasks (like state saving).
    *   **Synchronous Save on Pause**: To prevent state loss during "swipe-up" force-close, `onPause()` MUST execute `saveStateNative()` synchronously by queuing it to the core thread and waiting via a `CountDownLatch`. This ensures all pending inputs in the queue are processed BEFORE the RAM state is flushed to disk.
    *   **Non-Blocking Program Loops**: To keep the app responsive during long RPN programs, the main execution loop (`runProgram` in `lblGtoXeq.c`) MUST call `yieldToAndroid()`. This allows the UI thread to process stop requests (R/S) without violating display rules (intermediate stack values are not pushed unless `AVIEW` or `PAUSE` is hit).
    *   Runs the native C core loop (`tick`) at a target interval of 10ms.
    *   Blocks synchronously during SAF handshakes to maintain core logic simplicity.
    *   **Activity Re-sync**: When a new Activity instance is created, it MUST call `updateNativeActivityRef()` to ensure the core thread always executes callbacks (SAF, Audio, Quit) on the currently visible instance.

### 1.2. Function Visibility & Linkage
To support Android and GTK automation, specific core functions MUST be made public (remove `static`):
*   **Navigation**: `menuUp()` and `menuDown()` in `src/c47/keyboard.c` MUST be public and declared in `src/c47/softmenus.h`.
*   **State**: `doSave()` in `src/c47/saveRestoreCalcState.c` MUST be public.

### 1.3. Synchronization & JNI Strategy
To prevent Application Not Responding (ANR) errors and deadlocks:
*   **Mutex**: A native `screenMutex` MUST be `PTHREAD_MUTEX_RECURSIVE`.
*   **Minimal Contention (Memcpy-and-Release)**: The UI thread and core thread must never hold the mutex during blocking I/O. Functions like `fnScreenDump` (SNAP) or `getDisplayPixels` MUST lock, `memcpy` data to a local buffer, and unlock IMMEDIATELY before processing.
*   **Locking Strategy**:
    *   **UI Thread Calls**: (`getDisplayPixels`, `saveStateNative`) MUST use `pthread_mutex_trylock`. If busy (e.g. core is saving), the frame is skipped to avoid blocking the main loop.
    *   **Background Calls**: (`getXRegisterNative`, `sendSimKeyNative`) MUST use blocking `pthread_mutex_lock`. Since these are triggered by clipboard or paste actions on background threads in Kotlin, blocking ensures the operation completes without race conditions.
*   **SAF I/O Deadlock Prevention**: The `requestAndroidFile` native helper MUST fully release `screenMutex` (decrementing recursive count to zero) while waiting for the Java result, and re-acquire it afterwards.

### 1.3. Direct JNI Extensions (The Bridge)
Standard key emulation is insufficient for system-level actions. The JNI layer (`native-lib.c`) MUST include:
*   `sendSimMenuNative(menuId)`: Jumps directly to system menus (e.g., `HOME` = -1921, `MyMenu` = -1349).
*   `sendSimFuncNative(funcId)`: Executes core functions directly (e.g., Imaginary `i` = 1159, SI Multipliers 1802-1806) bypassing the keyboard logic for instant results.

---

## 2. Visual Specifications & Scaling

### 2.1. Dynamic Scaling & Layout Sync
*   **Logical Canvas**: 537 x 1005 pixels.
*   **Scaling Rule**: `scale = ViewWidth / 537f`. 
*   **The Maximization Bug**: `ReplicaOverlay` MUST calculate the scale factor using the width provided directly to `onMeasure(w, h)` and `onLayout()`. Do NOT rely on `this.width` or `this.height` during transitions.
*   **Positioning**: The hardware body is centered vertically. Areas above/below are filled with `#2B2A29`.

### 2.2. LCD Calibration
*   **Viewport**: `(25.5, 67.5)` relative to the logical canvas.
*   **Size**: `486 x 266.7` logical pixels.
*   **Rendering**: Pixels are mapped using a color mask (`pix & 0xFFFFFF`) to handle alpha variations.

### 2.3. Touch Grid Calibration (Non-Linear)
To match the photographic skin's perspective and spacing, the following offsets are applied in `setupInteractiveZones()`:
*   **Vertical Row Offsets**:
    *   Row 0: `y + 27f`, `height + 5f` (Shifts up, increased height).
    *   Row 1: `y + 22f`.
    *   Row 2: `y + 12f`.
    *   Row 3 (ENTER): `y + 10f`, expanded downward into Row 4 space.
    *   Row 4 (XEQ): `y + 10f`, height reduced to accommodate ENTER.
    *   Row 7 (Bottom): `y - 10f`, shifted up to touch Row 6 bottom.
*   **Horizontal Column Offsets**:
    *   **Top 3 Rows**: Boundary 1-2 and 2-3 shifted +8 units (widening left columns). Column 3 shrunk from right to restore Column 4 position.
    *   **Numeric Block (Rows 4-7)**: Boundaries 1-2, 2-3, and 3-4 extended to touch the next column's left side, eliminating dead gaps.

---

## 3. Character Encoding & Clipboard

### 3.1. Encoding Artifact Prevention (The "䊰" Bug)
Double-encoding occurs if symbols are converted to UTF-8 before the core `stringToUtf8` function.
*   **Rule**: `ascii_clean` in `android_helpers.c` MUST NOT convert core symbols (like `\x80\xb0` degree) to UTF-8. 
*   **Flattening Table**: `ascii_clean` MUST map the following byte sequences to standard ASCII:
    *   `\xa1\x48` → ` i ` (Imaginary i)
    *   `\xa1\x49` → ` j ` (Imaginary j)
    *   `\xa4\x69` or `\xa4\x7d` → `e` (Scientific Notation)
    *   `\xa0\x80`..`\xa0\x89` → `0`..`9` (Subscript Digits)
    *   `\xa1\x60`..`\xa1\x69` → `0`..`9` (Superscript Digits)
    *   `\xa1\x6a`/`\xa1\x6b` → `+`/`-` (Superscript Signs)
    *   `\xa0\x8a`/`\xa0\x8b` → `+`/`-` (Subscript Signs)
    *   `\x80\xd7` → `*` (Cross multiplication)
    *   `\x80\xb7` → ` ` (Dot/Separator)
    *   `\x82\xb3` → `r` (Bold r)
    *   `\x9d\x4d` → `g` (Bold g)
*   **Final Call**: Every register string returned to JNI MUST pass through `stringToUtf8` exactly once at the end of the helper chain.

### 3.2. RPN Paste Mapping
*   **Digits 0-9**: 33, 28, 29, 30, 23, 24, 25, 18, 19, 20.
*   **Imaginary i/j**: Map to `sendSimFuncNative` with IDs 1159 and 1160 respectively.
*   **Plus '+'**: Map to `ENTER` (37) to facilitate RPN complex number entry (e.g. `1+2i` pasted as `1`, `ENTER`, `2`, `i`).
*   **Locale**: `String.format` calls for simulated key IDs MUST use `java.util.Locale.US`.

---

## 4. File System & Storage Access Framework (SAF)

### 4.1. Intercepted Native Commands
Redirected to SAF: `LOADST`, `SAVEST`, `LOAD`, `SAVE`, `WRITEP`, `READP`, `XPORTP`, `SNAP`.

### 4.2. Work Directory & Organized Storage
To maintain parity with the hardware and simulator folder structures, users can select a "Work Directory" in preferences.
- **Parent Folder Selection**: Uses `ACTION_OPEN_DOCUMENT_TREE` to let the user pick a root folder (e.g., `/Downloads/R47`). The resulting URI is persisted with long-term permissions.
- **Automatic Subfolder Creation**: Upon selecting a work directory, the app automatically creates or resolves four standard subfolders: `STATE`, `PROGRAMS`, `SAVFILES`, and `SCREENS`.
- **Context-Aware SAF Pickers**: The `requestFile` bridge now accepts a `fileType` category ID. When a Work Directory is set, the SAF file picker uses `DocumentsContract.EXTRA_INITIAL_URI` to open directly in the relevant subfolder:
    - **Category 0 (STATE)**: For `.s47` state files.
    - **Category 1 (PROGRAMS)**: For `.p47`, `.rtf`, and `.txt` program files.
    - **Category 2 (SAVFILES)**: For `.sav` manual save files.
    - **Category 3 (SCREENS)**: For `.bmp` screenshots (`SNAP`).
- **Fallback**: If no Work Directory is selected, SAF pickers open in the system default location (usually Recents).

### 4.3. High-Latency Mitigation (Buffering)
SAF descriptors are slow.
*   **Implementation**: Data is assembled in a local 16KB memory buffer and written to the SAF descriptor in a single block operation.
*   **Persistence**: Implementations MUST call `fflush()` and `fsync(fileno(fp))` before closing.

### 4.3. SNAP / Screenshot Capture (Android)
The `SNAP` command (fnScreenDump) and documentation menu dump (fnMenuDump) require specialized handling for the Android ARGB8888 buffer:
- **Pixel Mapping**: Use direct mapping (`y_phys = y`, `x_phys = x`). The core engine already iterates rows bottom-to-top, which matches the BMP file format requirements. Flipping the axes manually causes 180-degree rotation.
- **Color Detection**: When comparing against `ON_PIXEL`, the alpha channel MUST be ignored: `if ((pix & 0xFFFFFF) == ON_PIXEL)`. This ensures consistent text detection regardless of transparency levels.
- **Thread Safety**: Always `memcpy` the `screenData` into a `localScreen` buffer while holding the `screenMutex`, then release the mutex before performing the (slow) BMP disk write to avoid UI stutters.

---

## 5. Persistence & Lifecycle

### 5.1. Non-Destructive Save State
To prevent transient messages (like "Saving state file...") from being stuck on screen upon resume:
- **Buffer Backup Strategy**: In `saveStateNative`, backup the clean `lcd_buffer` (pixels), print the message, refresh the PHYSICAL display, then RESTORE the clean pixels to the INTERNAL buffer before calling `saveCalc()`.
- **Buffer Preservation**: The `lcd_buffer`, `programmableMenu`, and `cachedDynamicMenu` must be persisted in the state file (Core Version 10000024) to ensure perfect visual parity.
- **Force Dirty on Load**: Upon state restoration, all LCD rows MUST be marked as dirty (`lcd_buffer[r * 52] = 1u`) to force an immediate push to the display view.
- **Avoid** `calcModeNormal()` or `temporaryInformation = TI_NO_INFO` in native lifecycle hooks to preserve NIM/AIM mode integrity.

### 5.2. Fortify SIGABRT Prevention
*   **Rule**: All native file pointers MUST be checked: `if (fp != NULL) { fwrite(...); }`. `fclose(NULL)` and `fwrite(..., NULL)` are fatal on Android.

---

## 6. Interactive Picture-in-Picture (PiP)

### 6.1. Aspect Ratio & Transitions
*   **Ratio**: `4860:2667`.
*   **Deferred Updates**: All UI changes during PiP transitions MUST be wrapped in `mainHandler.post`.

### 6.2. LCD Interactivity
*   In PiP, touch events on the LCD map horizontally: `X / width * 6` maps to keys F1-F6.

---

## 7. Android Lifecycle & System UI

### 7.1. Full Screen Mode & 3-Button Navigation
To accommodate users who prefer the Android 3-button navigation bar over gestures, a "Full Screen Mode" toggle is provided in the preferences (`fullscreen_mode`).
- **Implementation**: The `applyFullscreenMode(isFullscreen)` method in `MainActivity.kt` handles the transition.
- **Immersive (ON)**: Calls `WindowCompat.setDecorFitsSystemWindows(window, false)`, hides system bars using `WindowInsetsControllerCompat`, and sets `FLAG_LAYOUT_NO_LIMITS`. This allows the content to fill the entire screen, including cutout areas.
- **Normal (OFF)**: Calls `WindowCompat.setDecorFitsSystemWindows(window, true)`, shows system bars, and clears `FLAG_LAYOUT_NO_LIMITS`. This forces the Android framework to inset the `ReplicaOverlay`, ensuring the 3-button bar is visible and non-overlapping.
- **Adaptive Centering**: The `ReplicaOverlay` centering logic (`offsetX`/`offsetY`) automatically recalculates based on the reduced available width/height when bars are visible, keeping the calculator perfectly positioned.

### 7.2. View Attachment & Insets
Accessing the `WindowInsetsController` or system bars before the view is attached causes a `NullPointerException`.
*   **Rule**: System bar `hide()` and appearance configuration MUST be deferred until `onAttachedToWindow` or wrapped in `mainHandler.post` within `onCreate`.

### 7.2. Settings Menu Accessibility
*   **Touch Interception**: `ReplicaOverlay.onInterceptTouchEvent` MUST return `true` for taps in the top bezel area (`lY < 67.5f`) to ensure the Android settings/context menu can be triggered reliably even when core touch processing is active.

### 7.3. EXIT Logic: Minimize vs. Force Close
The `quitApp()` function in `MainActivity.kt` handles the native `fnOff` callback (triggered by `f-shift + EXIT`).
- **Logic**: It reads the `force_close_on_exit` preference.
- **Minimize**: Calls `moveTaskToBack(true)`.
- **Force Close**: Calls `finishAndRemoveTask()` to terminate the process and remove it from recents.
- **State Safety**: The core calls `saveCalc()` before invoking this callback.

### 7.4. Haptic Feedback
To provide tactile confirmation:
*   **Implementation**: Uses `performHapticFeedback(HapticFeedbackConstants.VIRTUAL_KEY)` on button views.
*   **Waveform**: Uses `VibrationEffect` waveform (Click + small bounce) for mechanical feel.

---

## 8. Robust Synchronization Strategy

To maintain custom work while pulling latest upstream changes, the `sync_repo.sh` (v3.1) script MUST follow these rules:
1.  **Backup All Custom Assets**: Directories `android/`, `manual/`, `.agent/`, `pdfs/`, `PROGRAMS/`, and `STATE/` are backed up before any git reset.
2.  **Backup Tooling Scripts**: Specialized root scripts like `generate_exports.py`, `generate_images.py`, `tag2ver.py`, and `build_android.sh` are preserved.
3.  **Master-Relative Patching**: Local core changes are captured via `git diff origin/master` to include both staged and unstaged work.
4.  **Tracked-Only Restoration**: Patches are re-applied using `git apply --include="src/*"` to avoid redundant modification of the already-restored custom directories.
5.  **Clean Restore**: Target directories are `rm -rf`'d before restoration to eliminate stale upstream artifacts.

---

## 9. Audio & Beeper Implementation

### 9.1. Timing Accuracy & Note Separation
To ensure `BEEP` melodies match the original hardware speed and notes remain distinct:
- **Core Thread**: Use `usleep((ms_delay + 10) * 1000)` in `_Buzz` to provide a 10ms scheduling cushion.
- **Audio Thread (Kotlin)**: Append 20ms of silence to each tone buffer in `audioThread` by increasing `numSamples` calculation: `(durationMs + 20) * sampleRate / 1000`.

### 9.2. Tone Parameter Handling
The `TONE` command requires an explicit exception in the core's register shortcut logic to support register indices (X, Y, Z, T).
- **Mandatory Change**: In `src/c47/ui/tam.c`, the condition for register shortcuts MUST include `|| tam.function == ITM_TONE`.
- **Note**: `ITM_TONE` MUST NOT be added to `isFunctionOldParam16()` as it interferes with immediate execution.

### 9.3. Link to System Volume
To support the `system_volume_linked` preference:
- **Stream Redirection**: `volumeControlStream = AudioManager.STREAM_MUSIC` MUST be set in `onCreate`.
- **Amplitude Scaling**: In the `audioThread`, if linked, samples MUST be scaled: `32767 * (currentVolume / maxVolume)`. Otherwise, use fixed `8192`.
- **AudioManager**: Use `getSystemService(Context.AUDIO_SERVICE) as AudioManager` to poll current volume levels inside the audio loop.

---

## 10. External Keyboard & Simulator Parity

### 10.1. Input Logic
*   **Modifier Logic**: External `Shift` and `Ctrl` act as modifiers while held. They trigger `f-shift`/`g-shift` ONLY upon release, and ONLY if no other key was pressed during the hold (Tap logic).
*   **activeKeyIdMap**: To prevent "stuck" keys, the app tracks the exact Simulator ID sent during `onKeyDown` and uses it for `onKeyUp`.

### 10.2. Final Verified Mapping Table
| Key | Shifted / Sequence | R47 Function (Core ID) | Technical Implementation |
| :--- | :--- | :--- | :--- |
| `q` / `Q` | `sqrt(x)` / `x^2` | 01 / 00 | Symmetric Key ID |
| `w` / `W` | `x<->y` / `LASTx` | 13 / SEQ_LASTX | Sequence (f-13) |
| `e` / `E` | `EEX` / **Constant `e`** | 15 / SEQ_ECONST | Complex Macro (f-CAT->CNST->f-e) |
| `r` / `R` | `RCL` / `->REC` | 07 / SEQ_toREC | Sequence (g-00) |
| `t` / `T` | `TAN` / `ATAN` | SEQ_TAN / SEQ_ATAN | Sequence (f-20 / g-20) |
| `y` / `Y` | `xroot(y)` / `y^x` | SEQ_XTHROOT / 03 | Sequence (f-03) |
| `u` / `U` | `undo` / `USER` | SEQ_UNDO / SEQ_USER | Sequence (f-16 / f-09) |
| `i` / `I` | `i` / `DISP` | SEQ_IMAG_J / SEQ_DISP | Sequence (f-00 / f-14) |
| `o` / `O` | `LOG` / `10^x` | 04 / SEQ_10X | Sequence (f-04) |
| `p` / `P` | `pi` / `->POL` | SEQ_PI / SEQ_toPOL | Sequence (f-08 / g-01) |
| `a` / `A` | `SIGMA+` / `ANGLE` | SEQ_SIGMAP / SEQ_ANGLE | Sequence (f-STAT-F1 / g-06) |
| `s` / `S` | `SIN` / `ASIN` | SEQ_SIN / SEQ_ASIN | Sequence (f-18 / g-18) |
| `d` / `D` | `Rdown` / `Rup` | 08 / SEQ_RUP | Sequence (g-08) |
| `f` / `F` | `f-shift` / `PREFIX` | 10 / SEQ_PREFIX | Sequence (f-15) |
| `g` / `G` | `g-shift` / `GTO` | 11 / SEQ_GTO | Sequence (g-17) |
| `h` / `H` | null / `[HOME]` | - / SEQ_HOME | Direct JNI Menu (-1921) |
| `j` / `J` | `j` / `EXP` | SEQ_IMAG_J / SEQ_EXP | Sequence (f-00 / g-15) |
| `k` / `K` | `ipol` / `STK` | SEQ_IMAG_POL / SEQ_STK | Sequence (f-01 / g-13) |
| `l` / `L` | `LN` / `exp(x)` | 05 / SEQ_EXP_E | Sequence (f-05) |
| `z` / `Z` | `R/S` / `|x|` | 35 / SEQ_ABS | Key ID / Sequence (f-06) |
| `x` / `X` | `XEQ` / `COMPLEX` | 17 / SEQ_COMPLEX | Key ID / Sequence (f-12) |
| `c` / `C` | `COS` / `ACOS` | SEQ_COS / SEQ_ACOS | Sequence (f-19 / g-19) |
| `v` / `V` | `1/x` / `1/x` | 02 / 02 | Duplicate ID |
| `b` / `B` | `LBL` / `[MyMenu]` | SEQ_LBL / SEQ_MYMENU | Sequence / Direct JNI (-1349) |
| `n` / `N` | `CHS` / `PRGM` | 14 / SEQ_PRGM | Key ID / Sequence (f-35) |
| `m` / `M` | `STO` / `[PREF]` | 06 / SEQ_PREF | Key ID / Sequence (f-28) |
| `,` / `<` | `.` / `RTN` | 34 / SEQ_RTN | Key ID / Sequence (g-35) |
| `.` / `>` | `.` / `DRG` | 34 / SEQ_DRG | Key ID / Sequence (09) |
| `'` | `[alpha]` | SEQ_ALPHA | Sequence (f-17) |
| `=` | `.d` | SEQ_DOTD | Sequence (g-34) |
| `7` / `&` | `7` / `->I` | 18 / SEQ_toI | Key ID / Sequence (g-04) |
| `F7-F11` | SI Multipliers | - | Direct JNI Func (1802-1806) |

---

## 11. Quick Slot Persistence & Atomic Switching

To ensure stable state switching:
*   **Atomic Sequencing**: Switches MUST occur in a single background thread: `[Save Old Slot] -> [Update ID] -> [Load New Slot]`.
*   **Situational Refresh**: Loading triggers `doFnReset`, `doLoad(autoLoad)`, re-scanning programs (`scanLabelsAndPrograms`), and a full screen redraw.
*   **Concurrency**: Uses an `AtomicBoolean` (`isNativeBusy`) to prevent overlapping state operations.

---

## 12. Android 15+ Compatibility (16KB Pages)

The native shared library MUST be linked with `-Wl,-z,max-page-size=16384` to support modern Android kernels and high-performance hardware like the Pixel 10.