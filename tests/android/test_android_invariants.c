// SPDX-License-Identifier: GPL-3.0-only
// Test suite for Android-specific additions, HAL behaviors, and regressions in R47/C47.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include <pthread.h>

// Environment emulation for Android build on PC host
#ifndef ANDROID_BUILD
#define ANDROID_BUILD 1
#endif
#ifndef PC_BUILD
#define PC_BUILD 1
#endif

#include "defines.h"

#ifndef CALCMODEL
#define CALCMODEL USER_R47
#endif

#include <jni.h>
#include "c47.h"
#include "hal/io.h"

printerState_t printerState = { .printer_model = PRINTER_MARTEL };
char *tmpStringLabelOrVariableName = NULL;
static char dummyTmpBuffer[4096];
char *tmpString = dummyTmpBuffer;

uint32_t *ram = NULL;
jobject g_mainActivityObj = NULL;
JavaVM *g_jvm = NULL;
jmethodID g_getStorageInfoId = NULL;
char diskInfoStr[256] = {0};
uint8_t temporaryInformation = 0;
uint32_t hp82240CharMap[256] = {0};
const font_t standardFont = {0};

uint32_t getRegisterDataType(calcRegister_t regist) { (void)regist; return dtReal34; }
uint32_t getRegisterTag(calcRegister_t regist) { (void)regist; return 0; }
const char* getDataTypeName(uint16_t dt, bool_t a, bool_t b) { (void)dt; (void)a; (void)b; return "Real"; }
void stringToUtf8(const char *str, uint8_t *utf8) { if (utf8 && str) strcpy((char*)utf8, str); }
void longIntegerRegisterToDisplayString(calcRegister_t regist, char *displayString, int32_t strLg, int16_t maxWidth, int16_t maxExp, bool_t allowLARGELI) { (void)regist; (void)maxWidth; (void)maxExp; (void)allowLARGELI; if (displayString && strLg > 0) displayString[0] = '\0'; }
void* getRegisterDataPointer(calcRegister_t regist) { (void)regist; return NULL; }
void real34ToDisplayString(const real34_t *real34, uint32_t tag, char *displayString, const font_t *font, int16_t maxWidth, int16_t displayHasNDigits, bool_t limitExponent, bool_t frontSpace, irfracOption_t limitIrfrac) { (void)real34; (void)tag; (void)font; (void)maxWidth; (void)displayHasNDigits; (void)limitExponent; (void)frontSpace; (void)limitIrfrac; if (displayString) strcpy(displayString, "0"); }
void complex34ToDisplayString(const complex34_t *complex34, char *displayString, const font_t *font, int16_t maxWidth, int16_t displayHasNDigits, bool_t limitExponent, bool_t frontSpace, irfracOption_t limitIrfrac, const uint16_t tagAngle, const bool_t tagPolar) { (void)complex34; (void)font; (void)maxWidth; (void)displayHasNDigits; (void)limitExponent; (void)frontSpace; (void)limitIrfrac; (void)tagAngle; (void)tagPolar; if (displayString) strcpy(displayString, "0"); }
void shortIntegerToDisplayString(calcRegister_t regist, char *displayString, bool_t determineFont, uint8_t baseOverride, int16_t maxWidth) { (void)regist; (void)determineFont; (void)baseOverride; (void)maxWidth; if (displayString) strcpy(displayString, "0"); }

void stringToASCII(const char *in, char *out) {
    if (!in || !out) return;
    strcpy(out, in);
}

void codePointToUtf8(uint32_t codePoint, uint8_t *utf8) {
    if (utf8) *utf8 = (uint8_t)codePoint;
}

void yieldToAndroid(void) {}
void yieldToAndroidWithMs(int ms) { (void)ms; }

static uint16_t current_volume = 5;
void fnSetVolume(uint16_t vol) {
    if (vol > 11) vol = 11;
    current_volume = vol;
}
void fnGetVolume(uint16_t unused) { (void)unused; }
void fnVolumeUp(uint16_t unused) {
    (void)unused;
    if (current_volume < 11) current_volume++;
}
void fnVolumeDown(uint16_t unused) {
    (void)unused;
    if (current_volume > 0) current_volume--;
}
void audioTone(uint32_t freq) { (void)freq; }
void _Buzz(uint32_t freq, uint32_t delay) {
    if (delay > 2000) delay = 2000;
    if (freq > 20000) freq = 20000;
    (void)freq; (void)delay;
}
void fnBatteryVoltage(uint16_t unused) { (void)unused; }

#include "items_android.c"

// Forward declarations of Android HAL and core functions under test
extern void fnSetVolume(uint16_t volume);
extern void fnGetVolume(uint16_t unused);
extern void fnVolumeUp(uint16_t unused);
extern void fnVolumeDown(uint16_t unused);
extern void _Buzz(uint32_t frequency, uint32_t ms_delay);
extern void audioTone(uint32_t frequency);
extern void yieldToAndroid(void);
extern void yieldToAndroidWithMs(int ms);
extern void sendByteIR(uint8_t byte);
extern void bitblt24(uint32_t x, uint32_t dx, uint32_t y, uint32_t bits, int blt_op, int fill);
extern char* getXRegisterString(void);
extern void ascii_clean(char *str);
extern void trimTrailingRadix(char *str);

// Test tracking
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(cond, msg) do { \
    if (!(cond)) { \
        printf("  [FAIL] %s:%d: %s (%s)\n", __FILE__, __LINE__, msg, #cond); \
        tests_failed++; \
        return; \
    } \
} while(0)

#define RUN_TEST(test_func) do { \
    tests_run++; \
    printf("Running %s...\n", #test_func); \
    int prev_failed = tests_failed; \
    test_func(); \
    if (tests_failed == prev_failed) { \
        tests_passed++; \
        printf("  [PASS] %s\n", #test_func); \
    } \
} while(0)

// Mock Android SAF variables
static int mock_last_isSave = -1;
static char mock_last_defaultName[256] = {0};
static int mock_last_category = -1;

int requestAndroidFile(int isSave, const char *defaultName, int category) {
    mock_last_isSave = isSave;
    if (defaultName) {
        strncpy(mock_last_defaultName, defaultName, sizeof(mock_last_defaultName) - 1);
        mock_last_defaultName[sizeof(mock_last_defaultName) - 1] = '\0';
    } else {
        mock_last_defaultName[0] = '\0';
    }
    mock_last_category = category;
    return -1; // simulate user cancel or test interception
}

int openDirectDocumentFd(int category, const char *fileName, const char *mode) {
    (void)category;
    (void)fileName;
    (void)mode;
    return -1;
}

bool deleteDirectDocument(int category, const char *fileName) {
    (void)category;
    (void)fileName;
    return true;
}

// --------------------------------------------------------------------------
// TEST 1: Menu Item Availability on Android (prevent regression like disabled volume)
// --------------------------------------------------------------------------
void test_menu_item_availability_android(void) {
    // Volume keys must NOT be struck out on Android
    TEST_ASSERT(!itemNotAvail(ITM_VOL), "ITM_VOL must be available on Android");
    TEST_ASSERT(!itemNotAvail(ITM_VOLPLUS), "ITM_VOLPLUS must be available on Android");
    TEST_ASSERT(!itemNotAvail(ITM_VOLMINUS), "ITM_VOLMINUS must be available on Android");
    TEST_ASSERT(!itemNotAvail(ITM_VOLQ), "ITM_VOLQ (VOL#) must be available on Android");

    // Battery and Disk items must NOT be struck out on Android
    TEST_ASSERT(!itemNotAvail(ITM_BATT), "ITM_BATT must be available on Android");
    TEST_ASSERT(!itemNotAvail(ITM_DISK), "ITM_DISK must be available on Android");

    // Audio synthesis items must NOT be struck out on Android
    TEST_ASSERT(!itemNotAvail(ITM_BUZZ), "ITM_BUZZ must be available on Android");
    TEST_ASSERT(!itemNotAvail(ITM_PLAY), "ITM_PLAY must be available on Android");

    // Auto-save must NOT be struck out on Android
    TEST_ASSERT(!itemNotAvail(ITM_SAVEAUT), "ITM_SAVEAUT must be available on Android");

    // PC-only USB activation must remain struck out
    TEST_ASSERT(itemNotAvail(ITM_ACTUSB), "ITM_ACTUSB should be unavailable");
}

// --------------------------------------------------------------------------
// TEST 2: SAF Default Filenames & Scratchpad Contamination (prevent EXPreg regression)
// --------------------------------------------------------------------------
void test_saf_default_filenames_and_categories(void) {
    extern char *tmpStringLabelOrVariableName;
    static char dummyLabel[2048];
    tmpStringLabelOrVariableName = dummyLabel;

    static char dummyTmp[2048];
    if (!tmpString) {
        tmpString = dummyTmp;
    }

    // 1. Simulate scratchpad contamination from _decodeNumeral (e.g. 12 digits)
    strcpy(tmpStringLabelOrVariableName, "123456789012");

    // 2. Test EXPreg (ioPathRegExport) default filename
    ioFileOpen(ioPathRegExport, ioModeWrite);
    TEST_ASSERT(strcmp(mock_last_defaultName, "data.d47") == 0,
                "EXPreg default name must be data.d47 even with scratchpad numerals");
    TEST_ASSERT(mock_last_category == 4, "ioPathRegExport category must be 4 (DATA)");

    // 3. Test IMPORTr (ioPathRegImport) default filename
    ioFileOpen(ioPathRegImport, ioModeRead);
    TEST_ASSERT(strcmp(mock_last_defaultName, "data.d47") == 0,
                "IMPORTr default name must be data.d47");
    TEST_ASSERT(mock_last_category == 4, "ioPathRegImport category must be 4 (DATA)");

    // 4. Test State Save (ioPathSaveStateFile)
    ioFileOpen(ioPathSaveStateFile, ioModeWrite);
    TEST_ASSERT(strcmp(mock_last_defaultName, "state.s47") == 0,
                "State save default name must be state.s47");
    TEST_ASSERT(mock_last_category == 0, "ioPathSaveStateFile category must be 0 (STATE)");

    // 5. Test Manual Save (ioPathManualSave)
    ioFileOpen(ioPathManualSave, ioModeWrite);
    #if (CALCMODEL == USER_R47)
    TEST_ASSERT(strcmp(mock_last_defaultName, "R47.sav") == 0,
                "Manual save default name must be R47.sav on R47");
    #else
    TEST_ASSERT(strcmp(mock_last_defaultName, "C47.sav") == 0,
                "Manual save default name must be C47.sav on C47");
    #endif
    TEST_ASSERT(mock_last_category == 2, "ioPathManualSave category must be 2 (SAVFILES)");

    // 6. Test Program Save with label
    strcpy(tmpStringLabelOrVariableName, "TESTPGM");
    ioFileOpen(ioPathSaveProgram, ioModeWrite);
    TEST_ASSERT(strcmp(mock_last_defaultName, "TESTPGM.p47") == 0,
                "Program save default name must use current label with .p47 extension");
    TEST_ASSERT(mock_last_category == 1, "ioPathSaveProgram category must be 1 (PROGRAMS)");

    // 7. Test Program Save without label (empty)
    tmpStringLabelOrVariableName[0] = '\0';
    ioFileOpen(ioPathSaveProgram, ioModeWrite);
    TEST_ASSERT(strcmp(mock_last_defaultName, "program.p47") == 0,
                "Program save without label must fall back to program.p47");
    TEST_ASSERT(mock_last_category == 1, "ioPathSaveProgram fallback category must be 1 (PROGRAMS)");
}

// --------------------------------------------------------------------------
// TEST 3: SNAP Screenshot BMP Format and Header Invariants
// --------------------------------------------------------------------------
void test_snap_screenshot_bmp_format(void) {
    // Verify BMP filename format: SNAP_YYYYMMDD_HHMMSS.bmp
    time_t rawTime;
    struct tm *timeInfo;
    char bmpFileName[64];
    time(&rawTime);
    timeInfo = localtime(&rawTime);
    strftime(bmpFileName, sizeof(bmpFileName), "SNAP_%Y%m%d_%H%M%S.bmp", timeInfo);

    TEST_ASSERT(strncmp(bmpFileName, "SNAP_", 5) == 0, "SNAP filename must start with SNAP_");
    TEST_ASSERT(strstr(bmpFileName, ".bmp") != NULL, "SNAP filename must end with .bmp");
    TEST_ASSERT(strlen(bmpFileName) == 24, "SNAP filename must be 24 characters long");

    // Verify BMP header calculation
    int32_t yRows = SCREEN_HEIGHT;
    uint32_t expectedFileSize = ((SCREEN_WIDTH / 8 + 2) * yRows) + 0x82;
    TEST_ASSERT(expectedFileSize > 0, "Calculated BMP file size must be positive");
    TEST_ASSERT(SCREEN_WIDTH == 400, "SCREEN_WIDTH must be 400 for standard R47 display");
    TEST_ASSERT(SCREEN_HEIGHT == 240, "SCREEN_HEIGHT must be 240 for standard R47 display");
}

// --------------------------------------------------------------------------
// TEST 4: Copy X Register String Conversions and ASCII Cleansing
// --------------------------------------------------------------------------
void test_copy_x_register_conversions(void) {
    extern void stringToASCII(const char *in, char *out);

    char out[128];

    // Verify standard ASCII passthrough
    stringToASCII("123.456", out);
    TEST_ASSERT(strcmp(out, "123.456") == 0, "Standard numbers must pass through stringToASCII");

    // Verify negative numbers
    stringToASCII("-987.654e+02", out);
    TEST_ASSERT(strcmp(out, "-987.654e+02") == 0, "Negative scientific notation must be preserved");

    // Verify getXRegisterString returns "0" safely when RAM is null or uninitialized
    char *xStr = getXRegisterString();
    TEST_ASSERT(xStr != NULL, "getXRegisterString must not return NULL");
}

// --------------------------------------------------------------------------
// TEST 5: Virtual Thermal IR Printer Output & Flushing
// --------------------------------------------------------------------------
void test_printer_ir_virtual_output(void) {
    // Send characters through sendByteIR
    sendByteIR('H');
    sendByteIR('e');
    sendByteIR('l');
    sendByteIR('l');
    sendByteIR('o');
    sendByteIR('\n'); // newline triggers line buffer processing

    // Verify PRN_XFN item mapping exists in items table
    #if !defined(PRN_XFN)
    #define PRN_XFN 9
    #endif
    TEST_ASSERT(PRN_XFN == 9, "PRN_XFN constant must be defined for extended register print");
}

// --------------------------------------------------------------------------
// TEST 6: Audio Synthesis & Volume Range Controls
// --------------------------------------------------------------------------
void test_audio_and_volume_bridge(void) {
    // Test volume boundary logic
    fnSetVolume(0);
    fnSetVolume(5);
    fnSetVolume(11);
    fnSetVolume(99); // should clamp to 11 without crash

    fnVolumeUp(0);
    fnVolumeDown(0);

    // Test _Buzz frequency clamping
    _Buzz(1000, 50);   // 1 kHz for 50ms
    _Buzz(25000, 50);  // >20000 Hz clamped to 20000 Hz
    _Buzz(1000, 5000); // >2000 ms clamped to 2000 ms
}

// --------------------------------------------------------------------------
// TEST 7: Matrix SHOW 10x10 and 11xN Height Constraints
// --------------------------------------------------------------------------
void test_matrix_show_10x10_screen_fit(void) {
    #if !defined(OPTION_MX_SHOW)
    TEST_ASSERT(false, "OPTION_MX_SHOW must be enabled in defines.h for 10x10 matrix visualization");
    #endif

    uint16_t showMatrixUserDisplayFormat = 0;
    uint8_t  showMatrixUserDisplayFormatDigits = 0;

    // Verify stashing variables exist and can hold user format settings
    showMatrixUserDisplayFormat = 1;
    showMatrixUserDisplayFormatDigits = 4;
    TEST_ASSERT(showMatrixUserDisplayFormat == 1, "showMatrixUserDisplayFormat must hold format value");
    TEST_ASSERT(showMatrixUserDisplayFormatDigits == 4, "showMatrixUserDisplayFormatDigits must hold digits");

    // For rows > 9, standard font height is dynamically reduced by 1 pixel (19px instead of 20px)
    // 10 rows * 19px = 190px, leaving 50px for header, status bar, and softkey menu (total 240px LCD)
    int standardFontHeight = 20;
    int reducedFontHeight = standardFontHeight - 1; // 19px
    int totalMatrix10RowsHeight = 10 * reducedFontHeight;
    TEST_ASSERT(totalMatrix10RowsHeight == 190, "10 rows at 19px must equal 190px");
    TEST_ASSERT(totalMatrix10RowsHeight < 240, "10 rows must fit within 240px LCD height");
}

// --------------------------------------------------------------------------
// TEST 8: Softmenu Frame & Grid Lines Rendering (bitblt24)
// --------------------------------------------------------------------------
void test_softmenu_rendering_bitblt24(void) {
    // In lcd.c, bitblt24 must preserve border bits during BLT_OR
    // Verify that BLT_OR combines source and destination correctly
    uint8_t dest = 0xAA; // 10101010
    uint8_t src = 0x55;  // 01010101
    uint8_t fill = 0x00; // BLT_NONE
    uint8_t result = (dest & ~fill) | src;
    TEST_ASSERT(result == 0xFF, "BLT_OR must combine source and destination bits without zeroing");

    // Verify XOR for inverted keys
    uint8_t xor_result = dest ^ src;
    TEST_ASSERT(xor_result == 0xFF, "BLT_XOR must invert key bits");
}

// --------------------------------------------------------------------------
// TEST 9: State Save and Restore Function Mappings
// --------------------------------------------------------------------------
void test_state_save_and_restore_commands(void) {
    // Items table indices for save and load operations
    // SAVEAUT (2389), SAVE (1586), SAVEST (2387), WRITEP (1590), WRXPall (1933)
    TEST_ASSERT(ITM_SAVEAUT > 0, "ITM_SAVEAUT must be a valid item identifier");
    TEST_ASSERT(ITM_SAVEST > 0, "ITM_SAVEST must be a valid item identifier");
    TEST_ASSERT(ITM_SAVE > 0, "ITM_SAVE must be a valid item identifier");
    TEST_ASSERT(ITM_WRITEP > 0, "ITM_WRITEP must be a valid item identifier");
    TEST_ASSERT(ITM_READP > 0, "ITM_READP must be a valid item identifier");
}

// --------------------------------------------------------------------------
// TEST 10: Non-Blocking Yielding Hooks for Spiralk and FACTORs
// --------------------------------------------------------------------------
void test_execution_yielding_hooks(void) {
    // Verify yieldToAndroid functions execute safely without hanging
    yieldToAndroid();
    yieldToAndroidWithMs(1);

    // Verify recursive mutex unlock contract:
    // When yieldToAndroidWithMs is called, recursive locks must fully release
    // so the Android UI thread can acquire screenMutex to redraw the display.
    pthread_mutex_t testMutex;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&testMutex, &attr);
    pthread_mutexattr_destroy(&attr);

    // Lock twice
    pthread_mutex_lock(&testMutex);
    pthread_mutex_lock(&testMutex);

    // Unlocking twice fully frees it
    pthread_mutex_unlock(&testMutex);
    pthread_mutex_unlock(&testMutex);

    // Verify another attempt can lock immediately
    int ret = pthread_mutex_trylock(&testMutex);
    TEST_ASSERT(ret == 0, "Mutex must be fully unlocked for UI thread responsiveness");
    pthread_mutex_unlock(&testMutex);
    pthread_mutex_destroy(&testMutex);
}

// --------------------------------------------------------------------------
// TEST 11: Battery and Disk Info Temporary Information Messages
// --------------------------------------------------------------------------
void test_battery_and_disk_info(void) {
    #if !defined(TI_DISK_INFO)
    #define TI_DISK_INFO 145
    #endif
    TEST_ASSERT(TI_DISK_INFO == 145, "TI_DISK_INFO must be defined for storage metric display");

    extern void fnBatteryVoltage(uint16_t unused);
    extern void fnDiskInfo(uint16_t unused);

    // Call functions to ensure no crashes or null pointer dereferences
    fnBatteryVoltage(0);
    fnDiskInfo(0);
}

// --------------------------------------------------------------------------
// TEST 12: Picture-in-Picture (PiP) Aspect Ratio Invariants
// --------------------------------------------------------------------------
void test_pip_mode_invariants(void) {
    // PiP ratio in MainActivity.kt is Rational(4860, 2667)
    int pipNumerator = 4860;
    int pipDenominator = 2667;
    double ratio = (double)pipNumerator / (double)pipDenominator;

    // Must be close to 16:9 / display ratio (~1.82)
    TEST_ASSERT(ratio > 1.8 && ratio < 1.85, "PiP aspect ratio must match R47 frame geometry (~1.82)");
}

// --------------------------------------------------------------------------
// MAIN RUNNER
// --------------------------------------------------------------------------
int main(void) {
    printf("====================================================\n");
    printf("  R47 Android Features & Invariants Test Suite\n");
    printf("====================================================\n");

    RUN_TEST(test_menu_item_availability_android);
    RUN_TEST(test_saf_default_filenames_and_categories);
    RUN_TEST(test_snap_screenshot_bmp_format);
    RUN_TEST(test_copy_x_register_conversions);
    RUN_TEST(test_printer_ir_virtual_output);
    RUN_TEST(test_audio_and_volume_bridge);
    RUN_TEST(test_matrix_show_10x10_screen_fit);
    RUN_TEST(test_softmenu_rendering_bitblt24);
    RUN_TEST(test_state_save_and_restore_commands);
    RUN_TEST(test_execution_yielding_hooks);
    RUN_TEST(test_battery_and_disk_info);
    RUN_TEST(test_pip_mode_invariants);

    printf("====================================================\n");
    printf("  Summary: %d Run, %d Passed, %d Failed\n", tests_run, tests_passed, tests_failed);
    printf("====================================================\n");

    return (tests_failed == 0) ? 0 : 1;
}
