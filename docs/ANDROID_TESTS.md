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
| **10** | `test_execution_yielding_hooks` | Program execution loops (`Spiralk`, `FACTORs`), `yieldToAndroid` | Verifies that non-blocking execution yield hooks run safely, and recursive `screenMutex` unlocks permit the Android UI thread to acquire the lock and refresh the screen during long loops. |
| **11** | `test_battery_and_disk_info` | `BATT`, `BATT#`, `DISK`, `TI_DISK_INFO` | Verifies that battery voltage and disk storage temporary info screens execute without crashing or null pointer dereferences, and `TI_DISK_INFO` constant (145) is defined. |
| **12** | `test_pip_mode_invariants` | Picture-in-Picture (PiP) window geometry | Verifies that the PiP aspect ratio in `MainActivity.kt` (`Rational(4860, 2667)` ~1.82) precisely matches the R47 hardware bezel ratio. |

---

### Part B: Kotlin Unit Tests (`android/app/src/test/java/org/rpncalculators/r47/`)

| Test Class | Test Method | Target Features / Invariants Tested |
|---|---|---|
| `DisplayActionControllerTest` | `testPipActionTriggersCallback` | Verifies that clicking the PiP button triggers the `onEnterPipRequested` callback. |
| `DisplayActionControllerTest` | `testCopyXRegisterQueuesActionAndReturnsResult` | Verifies that Copy X queues the native action, retrieves the formatted register string, and copies to clipboard. |
| `DisplayActionControllerTest` | `testPasteNumberTranslatesComplexAndDigits` | Verifies that pasted text with complex components ('i', 'j', decimal points, signs) is properly translated into calculator keypad strokes. |
| `WorkDirectoryCategoryTest` | `testCategorySubfolderNames` | Verifies SAF subfolder category integer-to-path mapping: `0: STATE`, `1: PROGRAMS`, `2: SAVFILES`, `3: SCREENS`, `4: DATA`, `5: PRINT`. |
| `WorkDirectoryCategoryTest` | `testAutoSaveFilenamePriorities` | Verifies that `R47auto.sav` takes precedence over legacy `C47auto.sav` when auto-loading state from `SAVFILES/`. |
| `WorkDirectoryCategoryTest` | `testCategoryDisplayFormatting` | Verifies that display paths are formatted cleanly for user presentation without trailing slashes. |

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
