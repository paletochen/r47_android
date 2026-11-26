# R47 Android Automated Test Suite & Invariants Catalog

This document catalogs all automated regression and invariant tests implemented for the Android port of R47/C47. Run these tests before any commit, push, or upstream merge to verify that all Android-specific enhancements, hardware bridges, and UI integrations function as intended.

---

## 1. Quick Start: Running Tests

To run the complete verification suite (both Native C Invariants and Kotlin Unit Tests):

```bash
./test_android.sh
```

To run individual suites:

- **Native C Invariants:**
  ```bash
  make -C tests/android clean && make -C tests/android
  ```

- **Android Kotlin Unit Tests:**
  ```bash
  cd android && ./gradlew testDebugUnitTest
  ```

---

## 2. Test Catalog for Inspiration

### Part A: Native C Invariant Tests (`tests/android/test_android_invariants.c`)

| # | Test Name | Target Features / Invariants Tested | Purpose / Regression Guard |
|---|---|---|---|
| **1** | `test_menu_item_availability_android` | `ITM_VOL`, `ITM_VOLPLUS`, `ITM_VOLMINUS`, `ITM_VOLQ`, `ITM_BATT`, `ITM_DISK`, `ITM_BUZZ`, `ITM_PLAY`, `ITM_SAVEAUT`, `ITM_ACTUSB` | Verifies that Android-enabled hardware functions are not struck out or blocked by upstream `PC_BUILD` checks in `items.c` (`itemNotAvail` / `itemERRTIVal`). |
| **2** | `test_saf_default_filenames_and_categories` | `EXPreg`, `IMPORTr`, `EXPstk`, `EXPltr`, `EXPnrg`, `EXPxfnx`, `SAVE`, `LOAD`, `READP`, `WRITEP` | Verifies that file operations default to clean names (`data.d47`, `state.s47`) without scratchpad contamination from `_decodeNumeral`, and route to correct SAF subfolder categories (`4` for DATA, `0` for STATE). |
| **3** | `test_snap_screenshot_bmp_format` | `SNAP`, monochrome 1-bit BMP generation | Verifies SNAP timestamp filename format (`SNAP_YYYYMMDD_HHMMSS.bmp`), correct 24-character length, BMP file size header calculations, and 400x240 screen dimensions. |
| **4** | `test_copy_x_register_conversions` | Copy X Register, `ascii_clean`, `trimTrailingRadix`, `getXRegisterString` | Verifies translation of calculator UTF-8 glyphs into standard ASCII (imaginary units `i`/`j`, scientific `e`, superscripts/subscripts), trailing radix stripping, and null RAM safety. |
| **5** | `test_printer_ir_virtual_output` | Virtual Thermal IR Printer, `sendByteIR`, `PRN_XFN` | Verifies line buffering in `hal/print_ir.c` (output dispatched upon `\n`), and checks that the extended register printing constant `PRN_XFN` (item 2694) is defined. |
| **6** | `test_audio_and_volume_bridge` | Beeper volume keys, `_Buzz`, `audioTone`, `FLAG_QUIET` | Verifies volume boundary clamping (0..11), step increments/decrements, frequency bounds (<=20 kHz), duration clamping (<=2000 ms), and silence under `FLAG_QUIET`. |
| **7** | `test_matrix_show_10x10_screen_fit` | Matrix `f-SHOW`, `OPTION_MX_SHOW`, dynamic font scaling | Verifies that `OPTION_MX_SHOW` is enabled, user display format stashing variables exist, and 10 rows fit completely within the 240px LCD canvas using the reduced 19px font height. |
| **8** | `test_softmenu_rendering_bitblt24` | Softmenu grid rendering, `bitblt24`, `BLT_OR`, `BLT_SET` | Verifies bitblt raster logic: ensures border bitmasks are preserved under `BLT_OR` even when `fill == BLT_SET`, preventing disappearance of softkey outlines. |
| **9** | `test_state_save_and_restore_commands` | `SAVEAUT`, `LOAD`, `SAVE`, `LOADST`, `SAVEST`, `READP`, `WRITEP`, `XPORTP` | Verifies that all state save/load and program transfer commands are active and mapped to valid execution functions. |
| **10** | `test_execution_yielding_hooks` | Program execution (`Spiralk`, `FACTORs`), `yieldToAndroid` | Verifies that non-blocking execution yield hooks execute cleanly and recursive `screenMutex` unlocks permit the Android UI thread to acquire the lock and refresh the screen during long computations. |
| **11** | `test_dmcp_key_queue_invariants` | `hal/keys.c` DMCP key FIFO queue, `wait_for_key_release`, `sys_sleep`, `sys_delay`, `sys_timer_start`, shutdown wakeup | Directly links and verifies production `hal/keys.c` key-ID parsing/formatting, 63-press + 1-release FIFO capacity, `keyPopForMainLoop` vs `key_pop` isolation (including during shutdown), `wait_for_key_release`, `sys_sleep` timer clamping, `sys_delay` slicing, recursive `screenMutex` unwinding, and post-drain `key_empty` shutdown termination. |
| **12** | `test_power_off_and_system_exit_invariants` | `fnOff` (`f-shift EXIT` / `OFF`), `backToSystem` (`SYSTEM`), `handlePendingPowerOffLocked`, `keyDispatchNow`, `dispatchTouchKeyGesture` / `dispatchSimKeyGesture` | Compiles upstream `fnOff` (`src/c47/calcMode.c`) and `backToSystem` (`src/c47/config.c`) against production `hal/keys.c` and `hal/lcd.c`, verifying flag clearing (`STAT_PGM_END`, `STAT_SUSPENDED`, `STAT_OFF`, `backToDMCP`), `keyDispatchNow` immediate stop and FIFO/ring-buffer flush on `OFF`, cross-source gesture rollover and invalid-keycode release, state persistence via `saveCalc`, single-shot `triggerQuit` idempotency, and external `notifyCoreShutdownRequested` isolation. |
| **13** | `test_lcd_pixel_mutex_concurrency_under_locked_screen_mutex` | `hal/lcd.c` `lcdPixelMutex`, `copyDirtyDisplayPixels`, `setLcdColors`, `lcd_refresh_lines`, UI-thread `screenMutex` trylock | Links production `hal/lcd.c` against a multi-threaded `pthread` harness owning `screenMutex` continuously for ~80 ms (simulating `FACTORS`), verifying `<5 ms` `EBUSY` trylock latency, live tear-free 96,000-pixel frame delivery via `copyDirtyDisplayPixels`, race-free `setLcdColors` palette remapping (including `LCD_write_line` overlays), and `lcd_refresh_lines` row bounds clamping. |
| **14** | `test_battery_and_disk_info` | `BATT`, `BATT#`, `DISK`, `TI_DISK_INFO` | Verifies that battery voltage and disk storage temporary info screens execute without crashing or null pointer dereferences, and `TI_DISK_INFO` (`254`) is assigned consistently across core and HAL units. |
| **15** | `test_pip_mode_invariants` | Picture-in-Picture (PiP) window geometry | Verifies that the PiP aspect ratio in `MainActivity.kt` (`Rational(4860, 2667)` ~1.82) precisely matches the R47 hardware bezel ratio. |
| **16** | `test_r47_last_key_code_mapping` | `lastKeyCode`, `KEY?`, R47 layout keycode parity | Verifies that `setLastKeyCode` maps R47 key layout indices (UP=23, DOWN=28, SHIFT=11, COS=18, +/-=16, E=15) to standard HP-42S/DM42 keycodes (UP=51, DOWN=61), ensuring HP-42S programs (e.g. Almanac) correctly navigate views without premature termination. |
| **17** | `test_program_copy_and_line_stripping` | `CM_PEM` program copy/paste, `stripProgramLinePrefix`, `parseAndPasteProgram` | Verifies stripping of numbered step prefixes and two-pass compilation of RPN program text into bytecode. |
| **18** | `test_c47_character_set_conversion_and_math_ops` | C47 UTF-8 glyphs, RTF unwrapping, 90 `.p47u`/`.txt` + 69 `.rtf` programs | Verifies byte-level round-trip compilation and glyph parity across the repository's RPN program corpus. |
| **19** | `test_rejig_aliases_and_insertion_shift` | `rejig` aliases (`TICKS`, `CLV`, `DIE`, `⎙`), `PTP_DISABLED` rejection, `parseAndPasteProgram` insertion shift | Verifies exact bytecode compilation of `rejig` aliases, rejection of non-programmable printer prefixes, and `PROT_NONE` guard-page bounds safety during mid-memory program insertion. |
| **20** | `test_matrix_paste_block_bounds` | `isMatrixBlockCountValid`, `tryPasteMatrix`, `ERROR_NOT_ENOUGH_MEMORY_FOR_NEW_MATRIX` | Verifies rejection of zero dimensions, dimensions `> 0xFFFFu` (including `1UL << 32` 64-bit wraparound inputs), real matrix bounds (`16383x1` valid vs `16384x1` and `200x200` invalid), complex matrix bounds (`8191x1` valid vs `8192x1` and `91x91` invalid), signed non-matrix text (`-2x3`), and `FLAG_ASLIFT` / `mtag` handling. |
| **21** | `test_program_long_literals_and_glyph_clamping` | `parseAndPasteProgram`, `normalizeProgramText`, `STRING_LONG_INTEGER`, `STRING_REAL34`, `STRING_LABEL_VARIABLE` | Verifies compilation of 255-digit `STRING_LONG_INTEGER` literals (including `>2000`-byte RTF `\u8200?` lines), 255-character `STRING_REAL34` scientific literals (`1e555...`, decoded `1.e555...`, and `>5000`-byte RTF `\super` lines), and C47 1-or-2-byte glyph-boundary clamping up to 255 bytes without splitting a 2-byte glyph. |


---

### Part B: Kotlin Unit Tests (`android/app/src/test/java/org/rpncalculators/r47/`)

| Test Class | Test Method | Target Features / Invariants Tested |
|---|---|---|
| `DisplayActionControllerTest` | `testEnterPiPCallback` | Verifies that clicking the PiP button triggers the `onEnterPipRequested` callback. |
| `DisplayActionControllerTest` | `testCopyXRegisterAction` | Verifies that Copy X queues the native action, retrieves the formatted register string, and copies to clipboard. |
| `DisplayActionControllerTest` | `testCopyAllRegistersAction` | Verifies that Copy All Registers queues the native action and retrieves the multi-register text snapshot. |
| `DisplayActionControllerTest` | `testPasteNumberMapping` | Verifies that pasted text with complex components ('i', 'j', decimal points, signs) is translated into calculator keypad strokes. |
| `WorkDirectoryCategoryTest` | `testSubfolderCategories` | Verifies SAF subfolder category integer-to-path mapping: `0: STATE`, `1: PROGRAMS`, `2: SAVFILES`, `3: SCREENS`, `4: DATA`, `5: PRINT`. |
| `WorkDirectoryCategoryTest` | `testAutoSaveFilePriorities` | Verifies that `R47auto.sav` takes precedence over legacy `C47auto.sav` when auto-loading state from `SAVFILES/`. |
| `WorkDirectoryCategoryTest` | `testFormatDisplayPath` | Verifies that display paths are formatted cleanly for user presentation without trailing slashes. |
| `AudioEngineTest` | `testVolumeClampingAndSettings` / `testBidirectionalVolumeMappingSteps0To11` | Verifies 0..100% clamping, 0..11 C47 volume step round-trip mapping, and mute behavior at step 0. |
| `NativeCoreRuntimeTest` | `testDisposeStopAppWakesBlockedCoreThreadAndUpdatesRotatedOwnerCallbacks` | Verifies rotated Activity callback delegation across configuration changes and bounded core-thread shutdown wakeup via `requestNativeShutdown`. |
| `NativeCoreRuntimeTest` | `testSameProcessRelaunchAfterDisposeStopAppReattachesCleanly` | Verifies that warm-process relaunch invokes `updateNativeActivityRef` to clear the prior shutdown latch without re-running `initNative`. |
| `NativeCoreRuntimeTest` | `testShutdownTimeoutDefersCleanupUntilSlowTaskFinishes` | Verifies that when a slow step outlasts the shutdown wait budget, `onActualStop` (`releaseNativeRuntime`) is deferred until the core thread exits. |
| `NativeDisplayRefreshLoopTest` | `testBusyCoreNullMetadataSkipsKeypadRefreshWhileDeliveringLcdPixelsAndRecoversImmediately` | Verifies that when `getKeypadMetaNative` returns `null` during busy core operations (`screenMutex` locked, even past 500 ms), `NativeDisplayRefreshLoop` continues 60 fps LCD pixel delivery without blocking and refreshes dynamic keypad labels on the first unlocked frame. |
| `NativeDisplayRefreshLoopTest` | `testNullSnapshotDuringTransientContentionDoesNotAdvanceLastKeypadMeta` | Verifies that when `getKeypadSnapshot` returns `null` due to transient `getKeypadLabelsNative` contention, `lastKeypadMeta` is preserved so the next frame retries immediately. |
| `NativeDisplayRefreshLoopTest` | `testStopRemovesCallbackAndIgnoresLateFrame` | Verifies that `stop()` removes the scheduled `FrameCallback` and ignores late frame callbacks. |
| `NativeDisplayRefreshLoopTest` | `testInvalidateKeypadCacheForcesImmediateRefreshOnNextFrameEvenWhenMetaUnchanged` | Verifies that `invalidateKeypadCache()` resets `lastKeypadMeta` so dynamic keypad rebuilds or preference changes during `screenMutex` contention refresh immediately on the first unlocked frame without waiting 500 ms. |

---

## 3. How to Add a New Test

### Adding a Native C Test (`tests/android/test_android_invariants.c`):
1. Open `tests/android/test_android_invariants.c`.
2. Define your test function:
   ```c
   void test_my_new_feature(void) {
       // Perform checks using TEST_ASSERT(condition, failure_message);
       TEST_ASSERT(my_condition == true, "Feature must satisfy invariant");
   }
   ```
3. Register your test in `main()`:
   ```c
   RUN_TEST(test_my_new_feature);
   ```
4. Run `./test_android.sh` to verify.

### Adding an Android Kotlin Unit Test:
1. Create or open a test file under `android/app/src/test/java/org/rpncalculators/r47/`.
2. Add a `@Test` method using standard JUnit 4:
   ```kotlin
   @Test
   fun testMyAndroidFeature() {
       assertEquals(expected, actual)
   }
   ```
3. Run `./test_android.sh` to verify.
