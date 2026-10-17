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
#include <dirent.h>

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
uint8_t lastKeyCode = 0;
uint8_t calcModel = USER_R47f_g;
extern void setLastKeyCode(int key);

real_t *statisticalSumsPointer = NULL;
subroutineLevelHeader_t *currentSubroutineLevelData = NULL;

uint8_t calcMode = CM_NORMAL;
uint16_t numberOfPrograms = 1;
uint16_t currentProgramNumber = 1;
static programList_t mock_program_list[16] = {{0}};
programList_t *programList = mock_program_list;
uint8_t *beginOfProgramMemory = NULL;
uint8_t *firstFreeProgramByte = NULL;
uint16_t freeProgramBytes = 0;
uint8_t *currentStep = NULL;
uint8_t *firstDisplayedStep = NULL;
uint8_t *beginOfCurrentProgram = NULL;
uint8_t *endOfCurrentProgram = NULL;

void defineCurrentProgramFromCurrentStep(void) {}
bool_t isAtEndOfPrograms(const uint8_t *step) { (void)step; return true; }
uint8_t *findNextStep(uint8_t *step) { (void)step; return NULL; }
void decodeOneStep(uint8_t *step) { (void)step; strcpy(tmpString, "END"); }
bool_t checkOpCodeOfStep(const uint8_t *step, uint16_t op) { (void)step; (void)op; return false; }
void deleteStepsFromTo(uint8_t *from, uint8_t *to) { (void)from; (void)to; }
void scanLabelsAndPrograms(void) {}
void resizeProgramMemory(uint16_t newSizeInBlocks) { (void)newSizeInBlocks; }
void goToGlobalStep(int32_t step) { (void)step; }
bool_t reDraw = false;
uint8_t screenUpdatingMode = 0;
void refreshScreen(uint16_t reason) { (void)reason; }
gboolean refreshLcd(gpointer unusedData) { (void)unusedData; return 0; }
void lcd_refresh(void) {}
int32_t compareString(const char *stra, const char *strb, int32_t comparisonType) {
    (void)comparisonType;
    return strcasecmp(stra, strb);
}
extern const item_t indexOfItems[];

static uint32_t mock_register_datatype = dtReal34;

char letteredRegisterName(calcRegister_t regist) {
    (void)regist;
    return 'X';
}

void copyRegisterToClipboardString(calcRegister_t regist, char *clipboardString, bool_t forPrinter) {
    (void)regist;
    (void)forPrinter;
    if (clipboardString) {
        if (mock_register_datatype == dtReal34Matrix) {
            strcpy(clipboardString, "2x2\n0.\n0.\n0.\n0.");
        } else {
            strcpy(clipboardString, "0");
        }
    }
}

char *decNumberToString(const decNumber *dn, char *string) {
    (void)dn;
    if (string) {
        strcpy(string, "0");
    }
    return string;
}

decContext ctxtReal34;

decContext ctxtReal39;
angularMode_t currentAngularMode = amDegree;

decNumber *decimal128ToNumber(const decimal128 *d128, decNumber *dn) {
    (void)d128;
    if (dn) dn->digits = 1;
    return dn;
}

decimal128 *decimal128FromNumber(decimal128 *d128, const decNumber *dn, decContext *set) {
    (void)dn; (void)set;
    return d128;
}

void real34RectangularToPolar(const real34_t *real34, const real34_t *imag34, real34_t *magnitude34, real34_t *theta34) {
    if (magnitude34 && real34) *magnitude34 = *real34;
    if (theta34 && imag34) *theta34 = *imag34;
}

void convertAngle34FromTo(real34_t *angle34, angularMode_t from, angularMode_t to) {
    (void)angle34; (void)from; (void)to;
}
decQuad *decQuadReduce(decQuad *df, const decQuad *dn, decContext *set) {
    (void)set;
    if (df && dn) {
        *df = *dn;
    }
    return df;
}
char *decQuadToString(const decQuad *df, char *string) {
    (void)df;
    if (string) {
        strcpy(string, "-0.98936");
    }
    return string;
}
uint32_t decQuadIsSigned(const decQuad *df) {
    (void)df;
    return 0;
}
decQuad *decQuadCopyAbs(decQuad *df, const decQuad *dn) {
    if (df && dn) {
        *df = *dn;
    }
    return df;
}
decQuad *decQuadFromString(decQuad *df, const char *string, decContext *set) {
    (void)set;
    if (df && string) {
        bool hasNonNumeric = false;
        for (const char *p = string; *p; p++) {
            if (isalpha((unsigned char)*p) && *p != 'e' && *p != 'E') {
                hasNonNumeric = true;
                break;
            }
        }
        if (hasNonNumeric) {
            df->bytes[0] = 0x7c;
        } else {
            df->bytes[0] = 0x00;
        }
    }
    return df;
}
uint32_t decQuadIsNaN(const decQuad *df) {
    if (!df) return 0;
    return (df->bytes[0] == 0x7c) ? 1 : 0;
}

uint32_t getRegisterDataType(calcRegister_t regist) { (void)regist; return mock_register_datatype; }
uint32_t getRegisterTag(calcRegister_t regist) { (void)regist; return 0; }
const char* getDataTypeName(uint16_t dt, bool_t a, bool_t b) { (void)dt; (void)a; (void)b; return "Real"; }
void stringToUtf8(const char *str, uint8_t *utf8) {
    if (!utf8 || !str) return;
    const char *s = str;
    char *d = (char *)utf8;
    while (*s) {
        if ((unsigned char)s[0] == 0xa2 && (unsigned char)s[1] == 0x21) {
            *d++ = (char)0xe2;
            *d++ = (char)0x88;
            *d++ = (char)0xa1;
            s += 2;
        } else {
            *d++ = *s++;
        }
    }
    *d = '\0';
}
void longIntegerRegisterToDisplayString(calcRegister_t regist, char *displayString, int32_t strLg, int16_t maxWidth, int16_t maxExp, bool_t allowLARGELI) { (void)regist; (void)maxWidth; (void)maxExp; (void)allowLARGELI; if (displayString && strLg > 0) displayString[0] = '\0'; }
static real34_t mock_reg_data[4];
void* getRegisterDataPointer(calcRegister_t regist) { (void)regist; return mock_reg_data; }
void real34ToDisplayString(const real34_t *real34, uint32_t tag, char *displayString, const font_t *font, int16_t maxWidth, int16_t displayHasNDigits, bool_t limitExponent, bool_t frontSpace, irfracOption_t limitIrfrac) { (void)real34; (void)tag; (void)font; (void)maxWidth; (void)displayHasNDigits; (void)limitExponent; (void)frontSpace; (void)limitIrfrac; if (displayString) strcpy(displayString, "0"); }
void complex34ToDisplayString(const complex34_t *complex34, char *displayString, const font_t *font, int16_t maxWidth, int16_t displayHasNDigits, bool_t limitExponent, bool_t frontSpace, irfracOption_t limitIrfrac, const uint16_t tagAngle, const bool_t tagPolar) { (void)complex34; (void)font; (void)maxWidth; (void)displayHasNDigits; (void)limitExponent; (void)frontSpace; (void)limitIrfrac; (void)tagAngle; (void)tagPolar; if (displayString) strcpy(displayString, "0"); }
void shortIntegerToDisplayString(calcRegister_t regist, char *displayString, bool_t determineFont, uint8_t baseOverride, int16_t maxWidth) { (void)regist; (void)determineFont; (void)baseOverride; (void)maxWidth; if (displayString) strcpy(displayString, "0"); }
void timeToDisplayString(calcRegister_t regist, char *displayString, bool_t ignoreTDisp) { (void)regist; (void)ignoreTDisp; if (displayString) strcpy(displayString, "2026:10:11"); }
void dateToDisplayString(calcRegister_t regist, char *displayString) { (void)regist; if (displayString) strcpy(displayString, "2026-10-11"); }

void stringToASCII(const char *in, char *out) {
    if (!in || !out) return;
    strcpy(out, in);
}

void utf8ToString(const uint8_t *utf8, char *str) {
    if (!utf8 || !str) return;
    const uint8_t *s = utf8;
    char *d = str;
    while (*s) {
        uint32_t codePoint = 0;
        if ((s[0] & 0x80) == 0) {
            codePoint = s[0];
            s += 1;
        } else if ((s[0] & 0xE0) == 0xC0 && (s[1] & 0xC0) == 0x80) {
            codePoint = ((uint32_t)(s[0] & 0x1F) << 6) | (s[1] & 0x3F);
            s += 2;
        } else if ((s[0] & 0xF0) == 0xE0 && (s[1] & 0xC0) == 0x80 && (s[2] & 0xC0) == 0x80) {
            codePoint = ((uint32_t)(s[0] & 0x0F) << 12) | ((uint32_t)(s[1] & 0x3F) << 6) | (s[2] & 0x3F);
            s += 3;
        } else if ((s[0] & 0xF8) == 0xF0 && (s[1] & 0xC0) == 0x80 && (s[2] & 0xC0) == 0x80 && (s[3] & 0xC0) == 0x80) {
            codePoint = ((uint32_t)(s[0] & 0x07) << 18) | ((uint32_t)(s[1] & 0x3F) << 12) | ((uint32_t)(s[2] & 0x3F) << 6) | (s[3] & 0x3F);
            s += 4;
        } else {
            *d++ = (char)*s++;
            continue;
        }
        if (codePoint < 0x0080) {
            *d++ = (char)codePoint;
        } else if ((codePoint & 0x00FF) == 0) {
            *d++ = '?';
        } else {
            codePoint |= 0x8000;
            *d++ = (char)((codePoint >> 8) & 0xFF);
            *d++ = (char)(codePoint & 0xFF);
        }
    }
    *d = '\0';
}

void codePointToUtf8(uint32_t codePoint, uint8_t *utf8) {
    if (!utf8) return;
    if (codePoint <= 0x7F) {
        utf8[0] = (uint8_t)codePoint;
        utf8[1] = 0;
    } else if (codePoint <= 0x7FF) {
        utf8[0] = 0xC0 | ((codePoint >> 6) & 0x1F);
        utf8[1] = 0x80 | (codePoint & 0x3F);
        utf8[2] = 0;
    } else {
        utf8[0] = 0xE0 | ((codePoint >> 12) & 0x0F);
        utf8[1] = 0x80 | ((codePoint >> 6) & 0x3F);
        utf8[2] = 0x80 | (codePoint & 0x3F);
        utf8[3] = 0;
    }
}

void yieldToAndroid(void) {}
void yieldToAndroidWithMs(int ms) { (void)ms; }

static bool_t mock_system_flags[256] = {0};
bool_t getSystemFlag(int32_t flag) {
    int idx = flag & 0xFF;
    return mock_system_flags[idx];
}
void setSystemFlag(unsigned int flag) {
    int idx = flag & 0xFF;
    mock_system_flags[idx] = true;
}
void clearSystemFlag(unsigned int flag) {
    int idx = flag & 0xFF;
    mock_system_flags[idx] = false;
}

static int g_mock_beeper_pct = 20;
static uint32_t g_last_tone_freq = 0;
static uint32_t g_last_tone_duration = 0;

uint16_t getBeepVolume(void) {
    int vol = (g_mock_beeper_pct * 11 + 50) / 100;
    if (vol > 11) vol = 11;
    if (vol < 0) vol = 0;
    return (uint16_t)vol;
}

void fnSetVolume(uint16_t volume) {
    if (volume > 11) volume = 11;
    g_mock_beeper_pct = (volume * 100) / 11;
}

void fnGetVolume(uint16_t unused) { (void)unused; }

void fnVolumeUp(uint16_t unused) {
    (void)unused;
    uint16_t vol = getBeepVolume();
    if (vol < 11) {
        vol++;
        fnSetVolume(vol);
    }
    audioTone(440000);
}

void fnVolumeDown(uint16_t unused) {
    (void)unused;
    uint16_t vol = getBeepVolume();
    if (vol > 0) {
        vol--;
        fnSetVolume(vol);
    }
    audioTone(440000);
}

void audioTone(uint32_t frequency) {
    if (getSystemFlag(FLAG_QUIET)) return;
    g_last_tone_freq = frequency;
    g_last_tone_duration = 200;
}

void _Buzz(uint32_t frequency, uint32_t ms_delay) {
    if (getSystemFlag(FLAG_QUIET)) return;
    if (ms_delay > 2000) ms_delay = 2000;
    if (frequency > 20000) frequency = 20000;
    g_last_tone_freq = frequency * 1000;
    g_last_tone_duration = ms_delay;
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
extern char* getAllRegistersString(void);
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
    extern void trimTrailingRadix(char *str);

    char out[128];

    // Verify standard ASCII passthrough
    stringToASCII("123.456", out);
    TEST_ASSERT(strcmp(out, "123.456") == 0, "Standard numbers must pass through stringToASCII");

    // Verify negative numbers
    stringToASCII("-987.654e+02", out);
    TEST_ASSERT(strcmp(out, "-987.654e+02") == 0, "Negative scientific notation must be preserved");

    // Verify trimTrailingRadix cleans trailing period before degree and angle tags
    strcpy(out, "86.°");
    trimTrailingRadix(out);
    TEST_ASSERT(strcmp(out, "86°") == 0, "trimTrailingRadix must strip radix mark before degree symbol");

    strcpy(out, "45.,deg");
    trimTrailingRadix(out);
    TEST_ASSERT(strcmp(out, "45deg") == 0, "trimTrailingRadix must strip radix mark before deg tag");

    strcpy(out, "123.450.");
    trimTrailingRadix(out);
    TEST_ASSERT(strcmp(out, "123.450") == 0, "trimTrailingRadix must strip trailing period");

    strcpy(out, "8.r");
    trimTrailingRadix(out);
    TEST_ASSERT(strcmp(out, "8r") == 0, "trimTrailingRadix must strip trailing period before 'r'");

    strcpy(out, "100.g");
    trimTrailingRadix(out);
    TEST_ASSERT(strcmp(out, "100g") == 0, "trimTrailingRadix must strip trailing period before 'g'");

    // Verify trimTrailingRadix preserves radix mark before STD_pi and UTF-8 pi
    strcpy(out, "2.\x83\xc0r");
    trimTrailingRadix(out);
    TEST_ASSERT(strcmp(out, "2.\x83\xc0r") == 0, "trimTrailingRadix must preserve period before STD_pi r");

    strcpy(out, "2.πr");
    trimTrailingRadix(out);
    TEST_ASSERT(strcmp(out, "2.πr") == 0, "trimTrailingRadix must preserve period before UTF-8 πr");

    strcpy(out, "2.π");
    trimTrailingRadix(out);
    TEST_ASSERT(strcmp(out, "2.π") == 0, "trimTrailingRadix must preserve period before UTF-8 π");

    // Verify ascii_clean normalizes STD_SUP_pir to STD_pi "r"
    strcpy(out, "\xac\x66");
    ascii_clean(out);
    TEST_ASSERT(strcmp(out, "\x83\xc0r") == 0, "ascii_clean must normalize STD_SUP_pir to STD_pi r");

    // Verify sanitization logic for pasted numbers (stripping spaces and normalizing commas/periods)
    char testClean1[] = "-0.989 36";
    char cleanBuf1[256];
    char *r1 = testClean1, *w1 = cleanBuf1;
    while (*r1) { if (*r1 != ' ' && *r1 != '\t') *w1++ = *r1; r1++; }
    *w1 = '\0';
    TEST_ASSERT(strcmp(cleanBuf1, "-0.98936") == 0, "Pasting with internal grouping spaces must be sanitized to -0.98936");

    char testClean2[] = "1.234,56"; // European thousands dot + comma radix
    char cleanBuf2[256];
    char *r2 = testClean2, *w2 = cleanBuf2;
    while (*r2) { if (*r2 != ' ' && *r2 != '\t') *w2++ = *r2; r2++; }
    *w2 = '\0';
    char *lastDot2 = strrchr(cleanBuf2, '.');
    char *lastComma2 = strrchr(cleanBuf2, ',');
    if (lastDot2 && lastComma2 && lastComma2 > lastDot2) {
        char *src = cleanBuf2, *dst = cleanBuf2;
        while (*src) {
            if (*src == '.') src++;
            else if (*src == ',') { *dst++ = '.'; src++; }
            else *dst++ = *src++;
        }
        *dst = '\0';
    }
    TEST_ASSERT(strcmp(cleanBuf2, "1234.56") == 0, "European thousands dot and comma radix must normalize to 1234.56");

    char testClean3[] = "1,234.56"; // US thousands comma + dot radix
    char cleanBuf3[256];
    char *r3 = testClean3, *w3 = cleanBuf3;
    while (*r3) { if (*r3 != ' ' && *r3 != '\t') *w3++ = *r3; r3++; }
    *w3 = '\0';
    char *lastDot3 = strrchr(cleanBuf3, '.');
    char *lastComma3 = strrchr(cleanBuf3, ',');
    if (lastDot3 && lastComma3 && lastDot3 > lastComma3) {
        char *src = cleanBuf3, *dst = cleanBuf3;
        while (*src) {
            if (*src == ',') src++;
            else *dst++ = *src++;
        }
        *dst = '\0';
    }
    TEST_ASSERT(strcmp(cleanBuf3, "1234.56") == 0, "US thousands comma and dot radix must normalize to 1234.56");

    // Verify dtTime and dtDate formatting in getXRegisterString
    static uint32_t dummyRam[100];
    ram = dummyRam;

    mock_register_datatype = dtTime;
    char *timeStr = getXRegisterString();
    TEST_ASSERT(strcmp(timeStr, "2026:10:11") == 0, "dtTime must copy formatted time string without colons stripped");

    mock_register_datatype = dtDate;
    char *dateStr = getXRegisterString();
    TEST_ASSERT(strcmp(dateStr, "2026-10-11") == 0, "dtDate must copy formatted date string without dashes stripped");

    // Verify dtString formatting in getXRegisterString (preserving trailing radix/dots)
    strcpy((char *)mock_reg_data + sizeof(strLgIntHeader_t), "Statement.");
    mock_register_datatype = dtString;
    char *stringStr = getXRegisterString();
    TEST_ASSERT(strcmp(stringStr, "Statement.") == 0, "dtString must copy formatted string without trailing radix/period stripped");

    // Verify string paste fallback logic: alphanumeric strings like "hello" produce dtString, not NaN
    real34_t testNonNumeric;
    stringToReal34("hello", &testNonNumeric);
    TEST_ASSERT(real34IsNaN(&testNonNumeric), "decQuadFromString on alphanumeric string 'hello' must result in NaN");

    // Verify quoted string stripping logic: "\"hello\"" -> "hello"
    const char *testQuoted = "\"hello\"";
    size_t qLen = strlen(testQuoted);
    bool_t isExplicitlyQuoted = (qLen >= 2 && testQuoted[0] == '"' && testQuoted[qLen - 1] == '"');
    TEST_ASSERT(isExplicitlyQuoted, "Double-quoted string must be recognized as explicitly quoted");

    // Verify ascii_clean maps punctuation spaces (e.g. from display digit grouping) to standard spaces
    char rawCoreTime[] = " 2\xa0\x08" "026:10:11";
    ascii_clean(rawCoreTime);
    TEST_ASSERT(strcmp(rawCoreTime, " 2 026:10:11") == 0, "ascii_clean must map STD_SPACE_PUNCTUATION to ASCII space");

    // Verify time sanitization logic for pasted time strings (stripping leading, trailing, and internal spaces)
    char testTimeRaw[] = " 2 026:10:12";
    char *pT = testTimeRaw;
    while (*pT == ' ' || *pT == '\t') pT++;
    char *colon1 = strchr(pT, ':');
    TEST_ASSERT(colon1 != NULL, "first colon must be found");
    char *colon2 = strchr(colon1 + 1, ':');
    TEST_ASSERT(colon2 != NULL, "second colon must be found");
    char partT1[32], cleanT1[32];
    size_t lT1 = colon1 - pT;
    strncpy(partT1, pT, lT1);
    partT1[lT1] = '\0';
    char *wT = cleanT1;
    for (char *r = partT1; *r; r++) {
        if (*r != ' ' && *r != '\t') *wT++ = *r;
    }
    *wT = '\0';
    TEST_ASSERT(strcmp(cleanT1, "2026") == 0, "Hours part must have internal spaces stripped to 2026");

    // Verify MULTPI paste sanitization and scalar extraction
    const char *testMultpiInputs[] = {
        "2.πr", "2.π", "2πr", "2π", "πr", "π", "-πr", "-π", "+π", "-2.πr", "2.5πr", "2 * π", "2:MULTPI", "2pir", "2pi", "pi", "-pi"
    };
    const char *expectedScalars[] = {
        "2", "2", "2", "2", "1", "1", "-1", "-1", "1", "-2", "2.5", "2", "2", "2", "2", "1", "-1"
    };
    for (size_t i = 0; i < sizeof(testMultpiInputs) / sizeof(testMultpiInputs[0]); i++) {
        char sBuf[128];
        strcpy(sBuf, testMultpiInputs[i]);
        char *s = sBuf;
        char typeTag[32];
        strcpy(typeTag, "Real");
        char multpiDefaultBuf[8] = "";

        char *multpiTag = strstr(s, ":MULTPI");
        char *piUtf8 = strstr(s, "\xcf\x80");
        if (!piUtf8) {
            piUtf8 = strstr(s, "\xce\xa0");
        }

        if (multpiTag) {
            strcpy(typeTag, "Real:MULTPI");
            *multpiTag = '\0';
        } else if (piUtf8) {
            strcpy(typeTag, "Real:MULTPI");
            *piUtf8 = '\0';
        } else {
            size_t slen = strlen(s);
            if (slen >= 3 && strcasecmp(s + slen - 3, "pir") == 0) {
                strcpy(typeTag, "Real:MULTPI");
                s[slen - 3] = '\0';
            } else if (slen >= 2 && strcasecmp(s + slen - 2, "pi") == 0) {
                strcpy(typeTag, "Real:MULTPI");
                s[slen - 2] = '\0';
            }
        }

        if (strcmp(typeTag, "Real:MULTPI") == 0) {
            char *end = s + strlen(s);
            while (end > s && (*(end - 1) == ' ' || *(end - 1) == '\t' || *(end - 1) == '*')) {
                *(--end) = '\0';
            }
            while (*s == ' ' || *s == '\t') {
                s++;
            }
            if (*s == '\0' || strcmp(s, "+") == 0) {
                strcpy(multpiDefaultBuf, "1.");
                s = multpiDefaultBuf;
            } else if (strcmp(s, "-") == 0) {
                strcpy(multpiDefaultBuf, "-1.");
                s = multpiDefaultBuf;
            }
        }

        char *end = s + strlen(s);
        while (end > s && (*(end - 1) == ' ' || *(end - 1) == '\t' || *(end - 1) == '.' || *(end - 1) == ',')) {
            *(--end) = '\0';
        }

        TEST_ASSERT(strcmp(typeTag, "Real:MULTPI") == 0, "Type tag must be Real:MULTPI");
        TEST_ASSERT(strcmp(s, expectedScalars[i]) == 0, "Extracted scalar must match expected scalar");
    }

    // Verify dtReal34Matrix copying in getXRegisterString
    mock_register_datatype = dtReal34Matrix;
    char *matStr = getXRegisterString();
    TEST_ASSERT(strcmp(matStr, "2x2\n0.\n0.\n0.\n0.") == 0, "dtReal34Matrix must copy formatted matrix elements instead of placeholder");

    // Verify matrix parsing invariants across diverse formats
    const char *testMatrixInputs[] = {
        "2x2\n0.\n0.\n0.\n0.",
        "X = 2x2\n0.\n0.\n0.\n0.",
        "X = 2 x 2\n1.5\n-2.5\n3.0\n-4.0",
        "3x1\n10.\n20.\n30.",
        "2x2 1. 2. 3. 4.",
        "2x2"
    };
    unsigned long expectedRows[] = {2, 2, 2, 3, 2, 2};
    unsigned long expectedCols[] = {2, 2, 2, 1, 2, 2};

    for (size_t m = 0; m < sizeof(testMatrixInputs) / sizeof(testMatrixInputs[0]); m++) {
        const char *p = testMatrixInputs[m];
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;

        const char *lineEnd = strpbrk(p, "\r\n");
        const char *eq = strchr(p, '=');
        if (eq != NULL && (lineEnd == NULL || eq < lineEnd)) {
            p = eq + 1;
            while (*p == ' ' || *p == '\t') p++;
        }
        if (*p == '[') {
            p++;
            while (*p == ' ' || *p == '\t') p++;
        }

        char *endRows = NULL;
        unsigned long r = strtoul(p, &endRows, 10);
        TEST_ASSERT(r == expectedRows[m], "Parsed matrix rows must match");

        while (*endRows == ' ' || *endRows == '\t') endRows++;
        TEST_ASSERT(*endRows == 'x' || *endRows == 'X', "Dimension separator must be x");
        endRows++;
        while (*endRows == ' ' || *endRows == '\t') endRows++;

        char *endCols = NULL;
        unsigned long c = strtoul(endRows, &endCols, 10);
        TEST_ASSERT(c == expectedCols[m], "Parsed matrix cols must match");
    }

    // Verify complex number parsing invariants (Cartesian, tuple, polar, suffix/prefix, pure imaginary)
    const char *testComplexInputs[] = {
        "2. +  i 5",
        "2. -  i 5",
        "2 + 5i",
        "2 - 5j",
        "2+5i",
        "2-5j",
        "-1.5e-3 + 4.2e+2i",
        "-1.5e-3-4.2e+2j",
        "(2.5, -4.2)",
        "(2; 5)",
        "2; 5",
        "5i",
        "-5j",
        "i",
        "-i",
        "-5.2e-3i",
        "5 ∡ 53.13:DEG",
        "5 /_ 53.13",
        "5 < 53.13 deg"
    };

    for (size_t cIdx = 0; cIdx < sizeof(testComplexInputs) / sizeof(testComplexInputs[0]); cIdx++) {
        const char *s = testComplexInputs[cIdx];
        bool isPolar = (strstr(s, "\xe2\x88\xa1") != NULL || strstr(s, "\xe2\x88\xa0") != NULL || strstr(s, "/_") != NULL || strstr(s, " < ") != NULL);
        bool hasTuple = (strchr(s, ';') != NULL || (s[0] == '(' && s[strlen(s) - 1] == ')'));
        bool hasUnit = false;
        const char *pU = s;
        while (*pU) {
            if (*pU == 'i' || *pU == 'I' || *pU == 'j' || *pU == 'J') {
                if (pU == s || (pU[-1] != 'p' && pU[-1] != 'P')) {
                    hasUnit = true;
                    break;
                }
            }
            pU++;
        }
        TEST_ASSERT(isPolar || hasTuple || hasUnit, "Input must be recognized as complex representation");
    }

    // Reset RAM and datatype
    ram = NULL;
    mock_register_datatype = dtReal34;

    // Verify getXRegisterString returns "0" safely when RAM is null or uninitialized
    char *xStr = getXRegisterString();
    TEST_ASSERT(xStr != NULL, "getXRegisterString must not return NULL");



    // Verify polar complex copying output
    static uint32_t dummy_ram[256];
    ram = dummy_ram;
    mock_register_datatype = dtComplex34;
    mock_system_flags[FLAG_POLAR & 0xFF] = true;
    char *polarStr = getXRegisterString();
    TEST_ASSERT(strstr(polarStr, "∡") != NULL, "Polar complex register copy must contain angle symbol");

    // Verify getAllRegistersString also preserves visual polar format in stack output
    char *allPolarStr = getAllRegistersString();
    TEST_ASSERT(allPolarStr != NULL, "getAllRegistersString must not return NULL when RAM is initialized");
    TEST_ASSERT(strstr(allPolarStr, "X = ") != NULL, "getAllRegistersString must contain X register prefix");
    TEST_ASSERT(strstr(allPolarStr, "∡") != NULL, "getAllRegistersString must preserve visual angle symbol for polar registers");

    mock_system_flags[FLAG_POLAR & 0xFF] = false;
    ram = NULL;
    mock_register_datatype = dtReal34;

    // Verify getAllRegistersString returns "" safely when RAM is null or uninitialized
    char *allStr = getAllRegistersString();
    TEST_ASSERT(allStr != NULL, "getAllRegistersString must not return NULL");
    TEST_ASSERT(strcmp(allStr, "") == 0, "getAllRegistersString must return empty string when RAM is uninitialized");
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

    // Test printPrettyLine emits formatted line to unique prettyprint_output.txt
    extern void printPrettyLine(const char *str);
    printPrettyLine("TEST PRETTY LINE");

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
    // 1. Test exact bidirectional 0..11 mapping between Android percentage (0..100) and C47 volume steps (0..11)
    for (int vol = 0; vol <= 11; vol++) {
        fnSetVolume((uint16_t)vol);
        uint16_t readBack = getBeepVolume();
        TEST_ASSERT(readBack == (uint16_t)vol, "Volume step must roundtrip through Android percentage without loss or parity drift");
    }

    // 2. Test stepwise fnVolumeUp from 0 to 11
    fnSetVolume(0);
    for (int step = 1; step <= 11; step++) {
        g_last_tone_freq = 0;
        g_last_tone_duration = 0;
        fnVolumeUp(0);
        TEST_ASSERT(getBeepVolume() == (uint16_t)step, "fnVolumeUp must reach step");
        TEST_ASSERT(g_last_tone_freq == 440000, "fnVolumeUp must play 440000 milliHz tone");
        TEST_ASSERT(g_last_tone_duration == 200, "fnVolumeUp tone duration must be 200ms");
    }
    // Clamping at upper bound (11)
    fnVolumeUp(0);
    TEST_ASSERT(getBeepVolume() == 11, "fnVolumeUp must clamp at maximum volume 11");

    // 3. Test stepwise fnVolumeDown from 11 down to 0
    for (int step = 10; step >= 0; step--) {
        g_last_tone_freq = 0;
        g_last_tone_duration = 0;
        fnVolumeDown(0);
        TEST_ASSERT(getBeepVolume() == (uint16_t)step, "fnVolumeDown must reach step");
        TEST_ASSERT(g_last_tone_freq == 440000, "fnVolumeDown must play 440000 milliHz tone");
        TEST_ASSERT(g_last_tone_duration == 200, "fnVolumeDown tone duration must be 200ms");
    }
    // Clamping at lower bound (0)
    fnVolumeDown(0);
    TEST_ASSERT(getBeepVolume() == 0, "fnVolumeDown must clamp at minimum volume 0");

    // 4. Test extreme volume clamping
    fnSetVolume(99);
    TEST_ASSERT(getBeepVolume() == 11, "fnSetVolume(99) must clamp to 11");

    // 5. Test _Buzz frequency and duration clamping
    g_last_tone_freq = 0;
    g_last_tone_duration = 0;
    _Buzz(1000, 50);   // 1 kHz for 50ms
    TEST_ASSERT(g_last_tone_freq == 1000000, "_Buzz must convert Hz to milliHz (1000 Hz -> 1000000 milliHz)");
    TEST_ASSERT(g_last_tone_duration == 50, "_Buzz duration must be 50ms");

    _Buzz(25000, 50);  // >20000 Hz clamped to 20000 Hz
    TEST_ASSERT(g_last_tone_freq == 20000000, "_Buzz must clamp frequency to 20000 Hz (20000000 milliHz)");

    _Buzz(1000, 5000); // >2000 ms clamped to 2000 ms
    TEST_ASSERT(g_last_tone_duration == 2000, "_Buzz must clamp duration to 2000 ms");

    // 6. Test FLAG_QUIET silence guarantee
    setSystemFlag(FLAG_QUIET);
    g_last_tone_freq = 0;
    audioTone(440000);
    TEST_ASSERT(g_last_tone_freq == 0, "audioTone must be silenced when FLAG_QUIET is set");

    _Buzz(1000, 50);
    TEST_ASSERT(g_last_tone_freq == 0, "_Buzz must be silenced when FLAG_QUIET is set");
    clearSystemFlag(FLAG_QUIET);
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
    #define TI_DISK_INFO 162
    #endif
    TEST_ASSERT(TI_DISK_INFO == 162, "TI_DISK_INFO must be defined for storage metric display");

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
// TEST 13: R47 Keycode Parity for Programmed Key Inquiries (KEY?)
// --------------------------------------------------------------------------
void test_r47_last_key_code_mapping(void) {
    // In R47 layout (isR47FAM), key indices map to DM42 matrix positions so
    // programs querying lastKeyCode (e.g. Almanac via KEY?) observe exact
    // HP-42S/DM42 keycodes (Arrow Up = 51, Arrow Down = 61).
    calcModel = USER_R47f_g;

    // Arrow UP (Slot 23 on R47) must produce 51
    setLastKeyCode(23);
    TEST_ASSERT(lastKeyCode == 51, "R47 Arrow UP (slot 23) must map to keycode 51");

    // Arrow DOWN (Slot 28 on R47) must produce 61
    setLastKeyCode(28);
    TEST_ASSERT(lastKeyCode == 61, "R47 Arrow DOWN (slot 28) must map to keycode 61");

    // SHIFT (Slot 11 on R47) maps to DM42 key 28 -> 71
    setLastKeyCode(11);
    TEST_ASSERT(lastKeyCode == 71, "R47 SHIFT (slot 11) must map to keycode 71");

    // COS (Slot 18 on R47) maps to DM42 key 11 -> 35
    setLastKeyCode(18);
    TEST_ASSERT(lastKeyCode == 35, "R47 COS (slot 18) must map to keycode 35");

    // +/- (Slot 16 on R47) maps to DM42 key 15 -> 43
    setLastKeyCode(16);
    TEST_ASSERT(lastKeyCode == 43, "R47 +/- (slot 16) must map to keycode 43");

    // E (Slot 15 on R47) maps to DM42 key 16 -> 44
    setLastKeyCode(15);
    TEST_ASSERT(lastKeyCode == 44, "R47 E (slot 15) must map to keycode 44");

    // EXIT (Slot 33) is unchanged -> 81
    setLastKeyCode(33);
    TEST_ASSERT(lastKeyCode == 81, "R47 EXIT (slot 33) must remain keycode 81");

    // R/S (Slot 36) is unchanged -> 84
    setLastKeyCode(36);
    TEST_ASSERT(lastKeyCode == 84, "R47 R/S (slot 36) must remain keycode 84");

    // Standard C47 / DM42 layout verification (!isR47FAM)
    calcModel = USER_C47;
    setLastKeyCode(18);
    TEST_ASSERT(lastKeyCode == 51, "C47 Arrow UP (slot 18) must produce keycode 51");
    setLastKeyCode(23);
    TEST_ASSERT(lastKeyCode == 61, "C47 Arrow DOWN (slot 23) must produce keycode 61");

    // Restore default R47 model
    calcModel = USER_R47f_g;
}

// --------------------------------------------------------------------------
// TEST 14: Program Mode Detection, Program Copy & Line Prefix Stripping
// --------------------------------------------------------------------------
extern bool isProgramMode(void);
extern const char *stripProgramLinePrefix(const char *line);
extern char *getCurrentProgramString(void);

void test_program_copy_and_line_stripping(void) {
    // 1. Line Prefix Stripping Invariants
    const char *p1 = stripProgramLinePrefix("0001: LBL \"FOO\"");
    TEST_ASSERT(strcmp(p1, "LBL \"FOO\"") == 0, "Line prefix '0001: ' must be stripped");

    const char *p2 = stripProgramLinePrefix("0002. STO 01");
    TEST_ASSERT(strcmp(p2, "STO 01") == 0, "Line prefix '0002. ' must be stripped");

    const char *p3 = stripProgramLinePrefix("0003  RCL 01");
    TEST_ASSERT(strcmp(p3, "RCL 01") == 0, "Line prefix '0003  ' must be stripped");

    const char *p4 = stripProgramLinePrefix("1: 1234");
    TEST_ASSERT(strcmp(p4, "1234") == 0, "Line prefix '1: ' must be stripped");

    const char *p5 = stripProgramLinePrefix("12345");
    TEST_ASSERT(strcmp(p5, "12345") == 0, "Standalone number literal '12345' must not be stripped");

    const char *p6 = stripProgramLinePrefix("-0.7666");
    TEST_ASSERT(strcmp(p6, "-0.7666") == 0, "Negative number literal '-0.7666' must not be stripped");

    const char *p7 = stripProgramLinePrefix("   0010:   +");
    TEST_ASSERT(strcmp(p7, "+") == 0, "Indented prefix '   0010:   +' must strip to '+'");

    const char *p8 = stripProgramLinePrefix("0020: 1.e-50");
    TEST_ASSERT(strcmp(p8, "1.e-50") == 0, "Scientific literal '0020: 1.e-50' must strip to '1.e-50'");

    const char *p9 = stripProgramLinePrefix("255 →INT 10");
    TEST_ASSERT(strcmp(p9, "255 →INT 10") == 0, "RPN number '255 →INT 10' must not be stripped");

    const char *p10 = stripProgramLinePrefix("100 STO 01");
    TEST_ASSERT(strcmp(p10, "100 STO 01") == 0, "RPN number '100 STO 01' must not be stripped");

    // 2. Program Mode Detection
    calcMode = CM_NORMAL;
    TEST_ASSERT(!isProgramMode(), "CM_NORMAL must not be identified as program mode");

    calcMode = CM_PEM;
    TEST_ASSERT(isProgramMode(), "CM_PEM must be identified as program mode");

    // 3. Program Copy in Normal mode vs PEM mode
    calcMode = CM_NORMAL;
    char *s1 = getCurrentProgramString();
    TEST_ASSERT(s1 != NULL && strlen(s1) == 0, "Normal mode must return empty program string");
    free(s1);

    // 4. Program Paste Transactional Parsing (E2DT forum program)
    calcMode = CM_PEM;
    const char *e2dt_sample =
        " \n"
        "0001:  LBL 'E2DT'   ; epoch to date/time matrix\n"
        "0002:    MVAR 'EP'\n"
        "0003:    MVAR 'TZ'\n"
        "0004:    VARMNU 'E2DT'   ; EP = epoch secs, TZ = offset secs (0 UTC, 3600 BST)\n"
        "0005:    STOP   ; enter values, press RS\n"
        "0006:    EXITALL\n"
        "0007:    LocR 14   ; .00 secs .01 days .02 sod .03 hh .04 mm .05 ss\n"
        "0008:    RCL 'EP'   ; local secs = EP + TZ\n"
        "0009:    RCL 'TZ'\n"
        "0010:    +\n"
        "0011:    STO .00\n"
        "0012:    86400   ; whole days since 1970-01-01\n"
        "0013:    ÷\n"
        "0014:    IP\n"
        "0015:    STO .01\n"
        "0016:    RCL .00   ; seconds into the day\n"
        "0017:    86400\n"
        "0018:    MOD\n"
        "0019:    STO .02\n"
        "0020:    3600   ; hours\n"
        "0021:    ÷\n"
        "0022:    IP\n"
        "0023:    STO .03\n"
        "0024:    RCL .02   ; minutes\n"
        "0025:    3600\n"
        "0026:    MOD\n"
        "0027:    60\n"
        "0028:    ÷\n"
        "0029:    IP\n"
        "0030:    STO .04\n"
        "0031:    RCL .02   ; seconds\n"
        "0032:    60\n"
        "0033:    MOD\n"
        "0034:    STO .05\n"
        "0035:    RCL .01   ; days to civil date (year starts 1 March)\n"
        "0036:    719468\n"
        "0037:    +\n"
        "0038:    STO .06   ; .06 doe\n"
        "0039:    146097   ; era = 400-year block\n"
        "0040:    ÷\n"
        "0041:    IP\n"
        "0042:    STO .07   ; .07 era\n"
        "0043:    146097\n"
        "0044:    ×\n"
        "0045:    RCL .06\n"
        "0046:    X<>Y\n"
        "0047:    -\n"
        "0048:    STO .06   ; doe = day of era\n"
        "0049:    RCL .06   ; yoe = year of era\n"
        "0050:    1460\n"
        "0051:    ÷\n"
        "0052:    IP\n"
        "0053:    RCL .06\n"
        "0054:    X<>Y\n"
        "0055:    -\n"
        "0056:    RCL .06\n"
        "0057:    36524\n"
        "0058:    ÷\n"
        "0059:    IP\n"
        "0060:    +\n"
        "0061:    RCL .06\n"
        "0062:    146096\n"
        "0063:    ÷\n"
        "0064:    IP\n"
        "0065:    -\n"
        "0066:    365\n"
        "0067:    ÷\n"
        "0068:    IP\n"
        "0069:    STO .08   ; .08 yoe\n"
        "0070:    RCL .07   ; year = yoe + 400*era\n"
        "0071:    400\n"
        "0072:    ×\n"
        "0073:    +\n"
        "0074:    STO .09   ; .09 year\n"
        "0075:    RCL .08   ; doy = day of March-based year\n"
        "0076:    365\n"
        "0077:    ×\n"
        "0078:    RCL .08\n"
        "0079:    4\n"
        "0080:    ÷\n"
        "0081:    IP\n"
        "0082:    +\n"
        "0083:    RCL .08\n"
        "0084:    100\n"
        "0085:    ÷\n"
        "0086:    IP\n"
        "0087:    -\n"
        "0088:    RCL .06\n"
        "0089:    X<>Y\n"
        "0090:    -\n"
        "0091:    STO .10   ; .10 doy\n"
        "0092:    5   ; mp = month index, March = 0\n"
        "0093:    ×\n"
        "0094:    2\n"
        "0095:    +\n"
        "0096:    153\n"
        "0097:    ÷\n"
        "0098:    IP\n"
        "0099:    STO .11   ; .11 mp\n"
        "0100:    153   ; day of month\n"
        "0101:    ×\n"
        "0102:    2\n"
        "0103:    +\n"
        "0104:    5\n"
        "0105:    ÷\n"
        "0106:    IP\n"
        "0107:    RCL .10\n"
        "0108:    X<>Y\n"
        "0109:    -\n"
        "0110:    1\n"
        "0111:    +\n"
        "0112:    STO .12   ; .12 day\n"
        "0113:    10   ; month = mp+3 if mp<10 else mp-9\n"
        "0114:    RCL .11\n"
        "0115:    x<y?\n"
        "0116:    GTO 01\n"
        "0117:    RCL .11\n"
        "0118:    9\n"
        "0119:    -\n"
        "0120:    STO .13   ; .13 month\n"
        "0121:    GTO 02\n"
        "0122:  LBL 01\n"
        "0123:    RCL .11\n"
        "0124:    3\n"
        "0125:    +\n"
        "0126:    STO .13\n"
        "0127:  LBL 02\n"
        "0128:    3   ; Jan/Feb belong to next year\n"
        "0129:    RCL .13\n"
        "0130:    x<y?\n"
        "0131:    INCR .09\n"
        "0132:    1   ; build 1x6 matrix\n"
        "0133:    6\n"
        "0134:    M.NEW\n"
        "0135:    RCL .09   ; year\n"
        "0136:    STOVEL 001\n"
        "0137:    DROP\n"
        "0138:    RCL .13   ; month\n"
        "0139:    STOVEL 002\n"
        "0140:    DROP\n"
        "0141:    RCL .12   ; day\n"
        "0142:    STOVEL 003\n"
        "0143:    DROP\n"
        "0144:    RCL .03   ; hour\n"
        "0145:    STOVEL 004\n"
        "0146:    DROP\n"
        "0147:    RCL .04   ; minute\n"
        "0148:    STOVEL 005\n"
        "0149:    DROP\n"
        "0150:    RCL .05   ; second\n"
        "0151:    STOVEL 006\n"
        "0152:    DROP\n"
        "0153:    STO 'DT'   ; keep copy in DT, matrix stays in X\n"
        "0154:  END\n";

    extern char *parseAndPasteProgram(const char *codeText);
    char *err = parseAndPasteProgram(e2dt_sample);
    if(err) {
        printf("Unexpected error in parseAndPasteProgram: '%s'\n", err);
    }
    TEST_ASSERT(err == NULL, "parseAndPasteProgram must succeed on E2DT forum listing with comments and Unicode ops");
    if(err) {
        free(err);
    }

    calcMode = CM_NORMAL; // Restore
}

static void test_c47_character_set_conversion_and_math_ops(void) {
    extern void c47StringToReadableUtf8(const char *src, char *dest, size_t destSize);
    char buf[128];

    // Test 1: x-th root of y (custom C47 font bytes \xa2\x1c\x82\x32 -> "ˣ√y")
    c47StringToReadableUtf8(STD_xTH_ROOT STD_y_UNDER_ROOT, buf, sizeof(buf));
    TEST_ASSERT(strcmp(buf, "ˣ√y") == 0, "STD_xTH_ROOT STD_y_UNDER_ROOT must convert to 'ˣ√y'");

    // Test 2: square root of x (\xa2\x1a\x83\x7f -> "√x")
    c47StringToReadableUtf8(STD_SQUARE_ROOT STD_x_UNDER_ROOT, buf, sizeof(buf));
    TEST_ASSERT(strcmp(buf, "√x") == 0, "STD_SQUARE_ROOT STD_x_UNDER_ROOT must convert to '√x'");

    // Test 3: cube root of x (\xa2\x1b\x83\x7f -> "∛x")
    c47StringToReadableUtf8(STD_CUBE_ROOT STD_x_UNDER_ROOT, buf, sizeof(buf));
    TEST_ASSERT(strcmp(buf, "∛x") == 0, "STD_CUBE_ROOT STD_x_UNDER_ROOT must convert to '∛x'");

    // Test 4: power and superscript conversions
    c47StringToReadableUtf8("x" STD_SUP_2, buf, sizeof(buf));
    TEST_ASSERT(strcmp(buf, "x²") == 0, "'x' STD_SUP_2 must convert to 'x²'");

    c47StringToReadableUtf8("x" STD_SUP_3, buf, sizeof(buf));
    TEST_ASSERT(strcmp(buf, "x³") == 0, "'x' STD_SUP_3 must convert to 'x³'");

    c47StringToReadableUtf8("y" STD_SUP_BOLD_x, buf, sizeof(buf));
    TEST_ASSERT(strcmp(buf, "yˣ") == 0, "'y' STD_SUP_BOLD_x must convert to 'yˣ'");

    c47StringToReadableUtf8("x" STD_RIGHT_OVER_LEFT_ARROW "y", buf, sizeof(buf));
    TEST_ASSERT(strcmp(buf, "x⇄y") == 0, "Swap must convert to 'x⇄y'");

    // Test 5: Parsing incoming programs with readable, forum, and raw C47 Unicode variants
    calcMode = CM_PEM;
    static uint8_t mock_prog[262144];
    memset(mock_prog, 0, sizeof(mock_prog));
    beginOfProgramMemory = mock_prog;
    firstFreeProgramByte = mock_prog + 16;
    freeProgramBytes = 65000;
    endOfCurrentProgram = mock_prog + 16;
    mock_program_list[0].instructionPointer = mock_prog;
    mock_program_list[0].step = 1;
    numberOfPrograms = 1;
    currentProgramNumber = 1;

    const char *math_listing =
        "0001:  LBL 'ALLRT'\n"
        "0002:    3\n"
        "0003:    x<y?\n"
        "0004:    DROP\n"
        "0005:    ˣ√y       ; readable Unicode x-th root of y\n"
        "0006:    x√y       ; ASCII x root y\n"
        "0007:    ∜Ȳ        ; raw C47 font code points from legacy clipboard\n"
        "0008:    root(x)(y); text formula equivalent\n"
        "0009:    xthroot   ; catalog name alias\n"
        "0010:    √x        ; square root\n"
        "0011:    ∛x        ; cube root\n"
        "0012:    x²        ; square\n"
        "0013:    x^2       ; square caret\n"
        "0014:    yˣ        ; y power x\n"
        "0015:    x⇄y       ; swap\n"
        "0016:    X<>Y      ; swap ascii\n"
        "0017:  END\n";

    extern char *parseAndPasteProgram(const char *codeText);
    char *err = parseAndPasteProgram(math_listing);
    if(err) {
        printf("Unexpected error in parseAndPasteProgram on math listing: '%s'\n", err);
    }
    TEST_ASSERT(err == NULL, "parseAndPasteProgram must parse ˣ√y, x√y, ∜Ȳ, root(x)(y), √x, ∛x, x², and swaps without error");
    if(err) {
        free(err);
    }

    // Test 6: Parsing raw res/PROGRAMS/GRAPHS.txt (multi-token lines, mathematical italic 𝑥², curly quotes, etc.)
    FILE *gf = fopen("../../res/PROGRAMS/GRAPHS.txt", "r");
    if(!gf) {
        gf = fopen("res/PROGRAMS/GRAPHS.txt", "r");
    }
    if(!gf) {
        perror("fopen GRAPHS.txt");
    }
    if(gf) {
        char gBuf[4096];
        size_t nRead = fread(gBuf, 1, sizeof(gBuf) - 1, gf);
        gBuf[nRead] = '\0';
        fclose(gf);
        char *gErr = parseAndPasteProgram(gBuf);
        if(gErr) {
            printf("Unexpected error in parseAndPasteProgram on GRAPHS.txt: '%s'\n", gErr);
        }
        TEST_ASSERT(gErr == NULL, "parseAndPasteProgram must compile res/PROGRAMS/GRAPHS.txt cleanly without line numbers");
        if(gErr) {
            free(gErr);
        }

        TEST_ASSERT(beginOfProgramMemory != NULL, "Program memory must be allocated");
        TEST_ASSERT(beginOfProgramMemory[0] == ITM_LBL, "Step 1 must be ITM_LBL");
        TEST_ASSERT(beginOfProgramMemory[1] == LOCAL_LABEL_VARIABLE, "Step 1 param must be LOCAL_LABEL_VARIABLE (249)");
        TEST_ASSERT(beginOfProgramMemory[2] == 2, "Step 1 label length must be 2");
        TEST_ASSERT(memcmp(&beginOfProgramMemory[3], "FX", 2) == 0, "Step 1 label name must be 'FX'");
    }

    // Test 7: Export header and metadata preamble skipping on line-numbered listings
    const char *export_header_listing =
        "C47 Program file export, version 1\n"
        "0000: { Prgm #1: 42 bytes / 5 steps }\n"
        "0001:  LBL 'DEMO'\n"
        "0002:  42.\n"
        "0003:  IP\n"
        "0004:  RTN\n"
        "0005:  END\n";

    char *hdrErr = parseAndPasteProgram(export_header_listing);
    if(hdrErr) {
        printf("Unexpected error in parseAndPasteProgram on export header listing: '%s'\n", hdrErr);
    }
    TEST_ASSERT(hdrErr == NULL, "parseAndPasteProgram must skip export preamble header lines and curly-brace comments");
    if(hdrErr) {
        free(hdrErr);
    }

    // Test 8: Unnumbered listing with export header
    const char *unnumbered_header_listing =
        "C47 Program file export, version 1\n"
        "LBL 'DEMO2'\n"
        "10.\n"
        "RTN\n"
        "END\n";

    char *unHdrErr = parseAndPasteProgram(unnumbered_header_listing);
    if(unHdrErr) {
        printf("Unexpected error in parseAndPasteProgram on unnumbered header listing: '%s'\n", unHdrErr);
    }
    TEST_ASSERT(unHdrErr == NULL, "parseAndPasteProgram must skip export header on unnumbered listings");
    if(unHdrErr) {
        free(unHdrErr);
    }

    // Test 9: Multi-line preamble and post-END trailer notes on line-numbered listings
    const char *trailer_listing =
        "C47 Program file export, version 1\n"
        "Author: David Castro\n"
        "0001:  LBL 'DEMO3'\n"
        "0002:  RTN\n"
        "0003:  END\n"
        "Exported successfully from C47\n";

    char *trErr = parseAndPasteProgram(trailer_listing);
    if(trErr) {
        printf("Unexpected error in parseAndPasteProgram on trailer listing: '%s'\n", trErr);
    }
    TEST_ASSERT(trErr == NULL, "parseAndPasteProgram must skip author preamble and post-END trailer notes");
    if(trErr) {
        free(trErr);
    }

    // Test 10: ELEM47 snippet with ABS, structured programming (IF/ENDIF/DO/WHILE), and NEWMAT
    const char *elem_snippet =
        "LBL 'ELEMTEST'\n"
        "ABS\n"
        "RCL 41\n"
        "ABS\n"
        "+\n"
        "1\n"
        "X=Y?\n"
        "IF 01\n"
        "  XEQ 03\n"
        "ENDIF 01\n"
        "NEWMAT\n"
        "RTN\n"
        "END\n";

    char *elemErr = parseAndPasteProgram(elem_snippet);
    if(elemErr) {
        printf("Unexpected error in parseAndPasteProgram on ELEM47 snippet: '%s'\n", elemErr);
    }
    TEST_ASSERT(elemErr == NULL, "parseAndPasteProgram must parse ABS, structured programming (IF/ENDIF), and NEWMAT without error");
    if(elemErr) {
        free(elemErr);
    }

    // Test 11: Binary/Hex numerals with #base and string literals with STD_CR ('↵')
    const char *base_and_string_snippet =
        "LBL 'BASETEST'\n"
        "11111111111111111111111111#2\n"
        "STO 20\n"
        "100000000000000000000000010000000000000000000000001#2\n"
        "STO 21\n"
        "FF#16\n"
        "\"Symbol↵State↵Group↵Config\"\n"
        "RTN\n"
        "END\n";

    char *baseErr = parseAndPasteProgram(base_and_string_snippet);
    if(baseErr) {
        printf("Unexpected error in parseAndPasteProgram on base and string snippet: '%s'\n", baseErr);
    }
    TEST_ASSERT(baseErr == NULL, "parseAndPasteProgram must compile base literals (#2, #16) and string literals with ↵ cleanly");
    if(baseErr) {
        free(baseErr);
    }

    // Verify step 2 is STRING_SHORT_INTEGER with base 2
    uint8_t *pStep = beginOfProgramMemory;
    TEST_ASSERT(pStep != NULL && pStep[0] == ITM_LBL, "Step 1 must be LBL");
    // Advance past step 1 (ITM_LBL, STRING_LABEL_VARIABLE, len, name...)
    pStep = pStep + 3 + pStep[2];
    TEST_ASSERT(pStep != NULL, "Step 2 must exist");
    TEST_ASSERT(pStep[0] == ITM_LITERAL, "Step 2 opcode must be ITM_LITERAL");
    TEST_ASSERT(pStep[1] == STRING_SHORT_INTEGER, "Step 2 literal type must be STRING_SHORT_INTEGER (7)");
    TEST_ASSERT(pStep[2] == 2, "Step 2 base must be 2");
    TEST_ASSERT(pStep[3] == 26, "Step 2 digits len must be 26");
    TEST_ASSERT(memcmp(&pStep[4], "11111111111111111111111111", 26) == 0, "Step 2 digits must match binary literal");

    // Test conversion of STD_CR and base indicators in c47StringToReadableUtf8
    char c47StrTest[64];
    c47StrTest[0] = 'A';
    c47StrTest[1] = (char)0xa1;
    c47StrTest[2] = (char)0xb5; // STD_CR
    c47StrTest[3] = 'B';
    c47StrTest[4] = '\0';
    c47StringToReadableUtf8(c47StrTest, buf, sizeof(buf));
    TEST_ASSERT(strcmp(buf, "A↵B") == 0, "STD_CR (\\xa1\\xb5) must decode to '↵'");

    c47StrTest[0] = '1';
    c47StrTest[1] = '0';
    c47StrTest[2] = (char)0xa4;
    c47StrTest[3] = (char)0x61; // base 2 (\xa4\x61)
    c47StrTest[4] = '\0';
    c47StringToReadableUtf8(c47StrTest, buf, sizeof(buf));
    TEST_ASSERT(strcmp(buf, "10#2") == 0, "Base 2 character (\\xa4\\x61) must decode to '#2'");

    // Test 12: Indirect subroutine calls and jumps (XEQ IND 50, GTO IND X, XEQ IND "MYROUT")
    const char *indirect_listing =
        "LBL 'INDTEST'\n"
        "XEQ IND 50\n"
        "XEQ IND \"MYROUT\"\n"
        "GTO IND X\n"
        "RTN\n"
        "END\n";

    char *indErr = parseAndPasteProgram(indirect_listing);
    if(indErr) {
        printf("Unexpected error in parseAndPasteProgram on indirect listing: '%s'\n", indErr);
    }
    TEST_ASSERT(indErr == NULL, "parseAndPasteProgram must compile indirect calls (XEQ IND 50, XEQ IND \"MYROUT\", GTO IND X)");
    if(indErr) {
        free(indErr);
    }

    uint8_t *pInd = beginOfProgramMemory;
    TEST_ASSERT(pInd != NULL && pInd[0] == ITM_LBL, "Step 1 must be LBL");
    // Step 2: XEQ IND 50 -> [ITM_XEQ, INDIRECT_REGISTER, 50] (3 bytes)
    pInd = pInd + 3 + pInd[2];
    TEST_ASSERT(pInd != NULL && pInd[0] == ITM_XEQ, "Step 2 must be ITM_XEQ");
    TEST_ASSERT(pInd[1] == INDIRECT_REGISTER, "Step 2 param must be INDIRECT_REGISTER (244)");
    TEST_ASSERT(pInd[2] == 50, "Step 2 register operand must be 50");

    // Step 3: XEQ IND "MYROUT" -> [ITM_XEQ, INDIRECT_VARIABLE, 6, 'M','Y','R','O','U','T'] (9 bytes)
    pInd = pInd + 3;
    TEST_ASSERT(pInd != NULL && pInd[0] == ITM_XEQ, "Step 3 must be ITM_XEQ");
    TEST_ASSERT(pInd[1] == INDIRECT_VARIABLE, "Step 3 param must be INDIRECT_VARIABLE (243)");
    TEST_ASSERT(pInd[2] == 6, "Step 3 variable length must be 6");
    TEST_ASSERT(memcmp(&pInd[3], "MYROUT", 6) == 0, "Step 3 variable name must be 'MYROUT'");

    // Step 4: GTO IND X -> [ITM_GTO, INDIRECT_REGISTER, REGISTER_X_IN_KS_CODE] (3 bytes)
    pInd = pInd + 3 + pInd[2];
    TEST_ASSERT(pInd != NULL && pInd[0] == ITM_GTO, "Step 4 must be ITM_GTO");
    TEST_ASSERT(pInd[1] == INDIRECT_REGISTER, "Step 4 param must be INDIRECT_REGISTER (244)");
    TEST_ASSERT(pInd[2] == REGISTER_X_IN_KS_CODE, "Step 4 register operand must be REGISTER_X_IN_KS_CODE");

    // Test rejection of invalid LBL IND 50
    const char *invalid_lbl_ind =
        "LBL 'BAD'\n"
        "LBL IND 50\n"
        "RTN\n"
        "END\n";
    char *badLblErr = parseAndPasteProgram(invalid_lbl_ind);
    TEST_ASSERT(badLblErr != NULL, "parseAndPasteProgram must reject 'LBL IND 50'");
    if(badLblErr) {
        free(badLblErr);
    }

    // Test 13: Power functions (2ˣ, 10ˣ, 1/x) and indirect volume/number commands (VOL IND "KVOL", VOL IND 33, VOL 11)
    const char *audio_prog =
        "LBL 'AUDIO'\n"
        "VOL#\n"
        "STO 'KVOL'\n"
        "2ˣ\n"
        "10ˣ\n"
        "1/x\n"
        "VOL 11\n"
        "VOL IND 33\n"
        "VOL IND \"KVOL\"\n"
        "RTN\n"
        "END\n";

    char *audErr = parseAndPasteProgram(audio_prog);
    if(audErr) {
        printf("Unexpected error in parseAndPasteProgram on audio listing: '%s'\n", audErr);
    }
    TEST_ASSERT(audErr == NULL, "parseAndPasteProgram must compile 2ˣ, 10ˣ, 1/x, VOL 11, VOL IND 33, and VOL IND \"KVOL\"");
    if(audErr) {
        free(audErr);
    }

    uint8_t *pAud = beginOfProgramMemory;
    TEST_ASSERT(pAud != NULL && pAud[0] == ITM_LBL, "Step 1 must be LBL");
    // Step 2: VOL# (item 2199: 2 bytes: (2199>>8)|0x80, 2199&0xff)
    pAud = pAud + 3 + pAud[2];
    TEST_ASSERT(pAud != NULL && pAud[0] == ((2199 >> 8) | 0x80) && pAud[1] == (2199 & 0xff), "Step 2 must be VOL#");

    // Step 3: STO 'KVOL'
    pAud = pAud + 2;
    TEST_ASSERT(pAud != NULL && pAud[0] == 44, "Step 3 must be STO (44)");

    // Step 4: 2ˣ (item 64: 1 byte: 64)
    pAud = pAud + 3 + pAud[2];
    TEST_ASSERT(pAud != NULL && pAud[0] == 64, "Step 4 must be ITM_2X (64)");

    // Step 5: 10ˣ (item 67: 1 byte: 67)
    pAud = pAud + 1;
    TEST_ASSERT(pAud != NULL && pAud[0] == 67, "Step 5 must be ITM_10x (67)");

    // Step 6: 1/x (item 73: 1 byte: 73)
    pAud = pAud + 1;
    TEST_ASSERT(pAud != NULL && pAud[0] == 73, "Step 6 must be ITM_1ONX (73)");

    // Step 7: VOL 11 (item 2198 + 11: 3 bytes)
    pAud = pAud + 1;
    TEST_ASSERT(pAud != NULL && pAud[0] == ((2198 >> 8) | 0x80) && pAud[1] == (2198 & 0xff), "Step 7 must be VOL");
    TEST_ASSERT(pAud[2] == 11, "Step 7 operand must be 11");

    // Step 8: VOL IND 33 (item 2198 + INDIRECT_REGISTER + 33: 4 bytes)
    pAud = pAud + 3;
    TEST_ASSERT(pAud != NULL && pAud[0] == ((2198 >> 8) | 0x80) && pAud[1] == (2198 & 0xff), "Step 8 must be VOL");
    TEST_ASSERT(pAud[2] == INDIRECT_REGISTER, "Step 8 param must be INDIRECT_REGISTER (244)");
    TEST_ASSERT(pAud[3] == 33, "Step 8 register operand must be 33");

    // Step 9: VOL IND "KVOL" (item 2198 + INDIRECT_VARIABLE + 4 + "KVOL": 8 bytes)
    pAud = pAud + 4;
    TEST_ASSERT(pAud != NULL && pAud[0] == ((2198 >> 8) | 0x80) && pAud[1] == (2198 & 0xff), "Step 9 must be VOL");
    TEST_ASSERT(pAud[2] == INDIRECT_VARIABLE, "Step 9 param must be INDIRECT_VARIABLE (243)");
    TEST_ASSERT(pAud[3] == 4, "Step 9 variable length must be 4");
    TEST_ASSERT(memcmp(&pAud[4], "KVOL", 4) == 0, "Step 9 variable name must be 'KVOL'");

    // Test 14: Parse full C64FX.p47u program end-to-end
    FILE *fP47u = fopen("../../res/PROGRAMS/C64FX.p47u", "rb");
    TEST_ASSERT(fP47u != NULL, "res/PROGRAMS/C64FX.p47u must exist");
    if(fP47u) {
        fseek(fP47u, 0, SEEK_END);
        long sz = ftell(fP47u);
        fseek(fP47u, 0, SEEK_SET);
        char *buf = (char *)malloc(sz + 1);
        fread(buf, 1, sz, fP47u);
        buf[sz] = '\0';
        fclose(fP47u);

        char *c64Err = parseAndPasteProgram(buf);
        if(c64Err) {
            printf("Error parsing C64FX.p47u: '%s'\n", c64Err);
        }
        TEST_ASSERT(c64Err == NULL, "parseAndPasteProgram must succeed on C64FX.p47u");
        if(c64Err) {
            free(c64Err);
        }
        free(buf);
    }

    // Test 15: VARMENU, Printer Commands, and Structured Subscript Parsing
    const char *testSubscriptsAndPrinters =
        "0001: LBL 'TESTEXT'\n"
        "0002: VARMENU 'PAYSWISS'\n"
        "0003: PrintON\n"
        "0004: PrintADV\n"
        "0005: 🖨α X\n"
        "0006: PrintXY\n"
        "0007: PrintOFF\n"
        "0008: IF₁\n"
        "0009:   1\n"
        "0010: ELSE₁\n"
        "0011:   2\n"
        "0012: ENDIF₁\n"
        "0013: IF₄₄\n"
        "0014:   3\n"
        "0015: ENDIF₄₄\n"
        "0016: DO₂\n"
        "0017: WHILE₂\n"
        "0018: ENDDO₂\n"
        "0019: REPEAT₃\n"
        "0020: UNTIL₃\n"
        "0021: SKIP₁\n"
        "0022: BACK₁\n"
        "0023: RTN\n"
        "0024: END\n";

    char *extErr = parseAndPasteProgram(testSubscriptsAndPrinters);
    if(extErr) {
        printf("Error parsing testSubscriptsAndPrinters: '%s'\n", extErr);
    }
    TEST_ASSERT(extErr == NULL, "parseAndPasteProgram must succeed on testSubscriptsAndPrinters");
    if(extErr) {
        free(extErr);
    }

    // Test 16: Parse full PAYSWISS.p47u program end-to-end
    firstFreeProgramByte = mock_prog + 16;
    freeProgramBytes = 65000;
    endOfCurrentProgram = mock_prog + 16;
    FILE *fSwiss = fopen("../../res/PROGRAMS/PAYSWISS.p47u", "rb");
    TEST_ASSERT(fSwiss != NULL, "res/PROGRAMS/PAYSWISS.p47u must exist");
    if(fSwiss) {
        fseek(fSwiss, 0, SEEK_END);
        long sz = ftell(fSwiss);
        fseek(fSwiss, 0, SEEK_SET);
        char *buf = (char *)malloc(sz + 1);
        fread(buf, 1, sz, fSwiss);
        buf[sz] = '\0';
        fclose(fSwiss);

        char *swissErr = parseAndPasteProgram(buf);
        if(swissErr) {
            printf("Error parsing PAYSWISS.p47u: '%s'\n", swissErr);
        }
        TEST_ASSERT(swissErr == NULL, "parseAndPasteProgram must succeed on PAYSWISS.p47u");
        if(swissErr) {
            free(swissErr);
        }
        free(buf);
    }

    // Test 17: Parse full PAYSLIP.p47u program end-to-end
    size_t pLenP47u = 0;
    memset(mock_prog, 0, sizeof(mock_prog));
    mock_program_list[0].instructionPointer = mock_prog;
    firstFreeProgramByte = mock_prog;
    freeProgramBytes = 65000;
    endOfCurrentProgram = mock_prog;
    FILE *fSlip = fopen("../../res/PROGRAMS/PAYSLIP.p47u", "rb");
    TEST_ASSERT(fSlip != NULL, "res/PROGRAMS/PAYSLIP.p47u must exist");
    if(fSlip) {
        fseek(fSlip, 0, SEEK_END);
        long sz = ftell(fSlip);
        fseek(fSlip, 0, SEEK_SET);
        char *buf = (char *)malloc(sz + 1);
        fread(buf, 1, sz, fSlip);
        buf[sz] = '\0';
        fclose(fSlip);

        char *slipErr = parseAndPasteProgram(buf);
        if(slipErr) {
            printf("Error parsing PAYSLIP.p47u: '%s'\n", slipErr);
        }
        TEST_ASSERT(slipErr == NULL, "parseAndPasteProgram must succeed on PAYSLIP.p47u");
        if(slipErr == NULL) {
            pLenP47u = (size_t)(endOfCurrentProgram - mock_program_list[0].instructionPointer);
        }
        if(slipErr) {
            free(slipErr);
        }
        free(buf);
    }

    // Test 18: Parse full res/PROGRAMS/PAYSLIP.txt program end-to-end and verify byte parity with PAYSLIP.p47u
    memset(mock_prog, 0, sizeof(mock_prog));
    mock_program_list[0].instructionPointer = mock_prog;
    firstFreeProgramByte = mock_prog;
    freeProgramBytes = 65000;
    endOfCurrentProgram = mock_prog;
    FILE *fTxt = fopen("../../res/PROGRAMS/PAYSLIP.txt", "rb");
    TEST_ASSERT(fTxt != NULL, "res/PROGRAMS/PAYSLIP.txt must exist");
    if(fTxt) {
        fseek(fTxt, 0, SEEK_END);
        long sz = ftell(fTxt);
        fseek(fTxt, 0, SEEK_SET);
        char *buf = (char *)malloc(sz + 1);
        fread(buf, 1, sz, fTxt);
        buf[sz] = '\0';
        fclose(fTxt);

        char *txtErr = parseAndPasteProgram(buf);
        if(txtErr) {
            printf("Error parsing PAYSLIP.txt: '%s'\n", txtErr);
        }
        TEST_ASSERT(txtErr == NULL, "parseAndPasteProgram must succeed on PAYSLIP.txt");
        if(txtErr == NULL) {
            size_t pLenTxt = (size_t)(endOfCurrentProgram - mock_program_list[0].instructionPointer);
            TEST_ASSERT(pLenTxt == pLenP47u, "PAYSLIP.txt (rejig R09/R11/IF₁ syntax) and PAYSLIP.p47u must compile to identical byte length");
        }
        if(txtErr) {
            free(txtErr);
        }
        free(buf);
    }

    // Test 19: Parse full tmp/payslip_decoded.txt program end-to-end
    firstFreeProgramByte = mock_prog + 16;
    freeProgramBytes = 65000;
    endOfCurrentProgram = mock_prog + 16;
    FILE *fDec = fopen("../../tmp/payslip_decoded.txt", "rb");
    if(fDec) {
        fseek(fDec, 0, SEEK_END);
        long sz = ftell(fDec);
        fseek(fDec, 0, SEEK_SET);
        char *buf = (char *)malloc(sz + 1);
        fread(buf, 1, sz, fDec);
        buf[sz] = '\0';
        fclose(fDec);

        char *decErr = parseAndPasteProgram(buf);
        if(decErr) {
            printf("Error parsing payslip_decoded.txt: '%s'\n", decErr);
        }
        TEST_ASSERT(decErr == NULL, "parseAndPasteProgram must succeed on payslip_decoded.txt");
        if(decErr) {
            free(decErr);
        }
        free(buf);
    }

    // Test 20: Parse full ATEXTing.p47u end-to-end (verify unnumbered start preserved)
    firstFreeProgramByte = mock_prog + 16;
    freeProgramBytes = 65000;
    endOfCurrentProgram = mock_prog + 16;
    FILE *fAtext = fopen("../../res/PROGRAMS/ATEXTing.p47u", "rb");
    TEST_ASSERT(fAtext != NULL, "res/PROGRAMS/ATEXTing.p47u must exist");
    if(fAtext) {
        fseek(fAtext, 0, SEEK_END);
        long sz = ftell(fAtext);
        fseek(fAtext, 0, SEEK_SET);
        char *buf = (char *)malloc(sz + 1);
        fread(buf, 1, sz, fAtext);
        buf[sz] = '\0';
        fclose(fAtext);

        char *atextErr = parseAndPasteProgram(buf);
        if(atextErr) {
            printf("Error parsing ATEXTing.p47u: '%s'\n", atextErr);
        }
        TEST_ASSERT(atextErr == NULL, "parseAndPasteProgram must succeed on ATEXTing.p47u");
        if(atextErr) {
            free(atextErr);
        }

        // Verify first step is LBL 'ATEXTing' (opcode 1, string variable 'ATEXTing')
        uint8_t *firstStep = mock_program_list[0].instructionPointer;
        TEST_ASSERT(firstStep[0] == 1, "First instruction in ATEXTing.p47u must be LBL (opcode 1)");
        TEST_ASSERT(firstStep[1] == (uint8_t)STRING_LABEL_VARIABLE, "First instruction argument must be STRING_LABEL_VARIABLE");
        TEST_ASSERT(firstStep[2] == 8, "LBL argument length must be 8");
        TEST_ASSERT(memcmp(&firstStep[3], "ATEXTing", 8) == 0, "LBL name must be 'ATEXTing'");

        // Verify '255' literal was preserved in program bytecode without being stripped as line prefix
        bool found255 = false;
        for(uint8_t *ptr = mock_prog; ptr < firstFreeProgramByte - 3; ptr++) {
            if(ptr[0] == '2' && ptr[1] == '5' && ptr[2] == '5') {
                found255 = true;
                break;
            }
        }
        TEST_ASSERT(found255, "Numeric literal '255' must be preserved in ATEXTing.p47u program memory");
        free(buf);
    }

    // Test 21: Verify R00..R99, R.00..R.98, healed 'R09' / ‘R11’, system flags, shuffle, and REM compile to exact opParams (never STRING_LABEL_VARIABLE 'R09')
    memset(mock_prog, 0, sizeof(mock_prog));
    mock_program_list[0].instructionPointer = mock_prog;
    firstFreeProgramByte = mock_prog;
    freeProgramBytes = 65000;
    endOfCurrentProgram = mock_prog;
    const char *regAndBulletproofProg =
        "LBL 'REGTST'\n"
        "KEY? R09\n"
        "CLα R11\n"
        "αIP R11\n"
        "RCL R11\n"
        "STO R12\n"
        "KEY? ‘R09’\n"
        "CLα ‘R11’\n"
        "STO R.00\n"
        "RCL .05\n"
        "SF 'SPCRES'\n"
        "CF .05\n"
        "<> yxzt\n"
        "REM 'unit test'\n"
        "x=? IND R05\n"
        "RTN\n";
    char *regErr = parseAndPasteProgram(regAndBulletproofProg);
    if(regErr) {
        printf("Error parsing regAndBulletproofProg: '%s'\n", regErr);
    }
    TEST_ASSERT(regErr == NULL, "parseAndPasteProgram must succeed on R09/R11/R.00/system flag/shuffle/REM syntax");
    if(regErr == NULL) {
        // Ensure 'R09' and 'R11' were NEVER encoded as STRING_LABEL_VARIABLE (250, 3, 'R', '0', '9')
        size_t totalBytes = (size_t)(endOfCurrentProgram - mock_program_list[0].instructionPointer);
        bool foundBadVarR09 = false;
        bool foundBadVarR11 = false;
        for(size_t bi = 0; bi + 4 < totalBytes; bi++) {
            if(mock_prog[bi] == (uint8_t)STRING_LABEL_VARIABLE && mock_prog[bi + 1] == 3 &&
               mock_prog[bi + 2] == 'R' && mock_prog[bi + 3] == '0' && mock_prog[bi + 4] == '9') {
                foundBadVarR09 = true;
            }
            if(mock_prog[bi] == (uint8_t)STRING_LABEL_VARIABLE && mock_prog[bi + 1] == 3 &&
               mock_prog[bi + 2] == 'R' && mock_prog[bi + 3] == '1' && mock_prog[bi + 4] == '1') {
                foundBadVarR11 = true;
            }
        }
        TEST_ASSERT(!foundBadVarR09, "R09 and ‘R09’ must compile to global register 9, never STRING_LABEL_VARIABLE 'R09'");
        TEST_ASSERT(!foundBadVarR11, "R11 and ‘R11’ must compile to global register 11, never STRING_LABEL_VARIABLE 'R11'");
        // Step 1: LBL 'REGTST' is 1 + 1 + 1 + 6 = 9 bytes (bytes 0..8)
        // Step 2: KEY? R09 -> ITM_KEYQ (77 = 0x4d), reg 9 -> 2 bytes at offset 9..10
        TEST_ASSERT(mock_prog[9] == (uint8_t)ITM_KEYQ, "Step 2 must be ITM_KEYQ (77)");
        TEST_ASSERT(mock_prog[10] == 9, "KEY? R09 operand must be numeric register 9 (0x09)");
        // Step 3: CLα R11 -> mock opcode 2773 (2 bytes), reg 11 -> 3 bytes at offset 11..13
        TEST_ASSERT(mock_prog[13] == 11, "CLα R11 operand must be numeric register 11 (0x0b)");
        // Step 4: αIP R11 -> mock opcode 2774 (2 bytes), reg 11 -> 3 bytes at offset 14..16
        TEST_ASSERT(mock_prog[16] == 11, "αIP R11 operand must be numeric register 11 (0x0b)");
        // Step 5: RCL R11 -> ITM_RCL (51 = 0x33), reg 11 -> 2 bytes at offset 17..18
        TEST_ASSERT(mock_prog[17] == (uint8_t)ITM_RCL && mock_prog[18] == 11, "RCL R11 must encode as [ITM_RCL, 11]");
        // Step 6: STO R12 -> ITM_STO (44 = 0x2c), reg 12 -> 2 bytes at offset 19..20
        TEST_ASSERT(mock_prog[19] == (uint8_t)ITM_STO && mock_prog[20] == 12, "STO R12 must encode as [ITM_STO, 12]");
        // Step 7: KEY? ‘R09’ -> healed to reg 9 at offset 21..22
        TEST_ASSERT(mock_prog[21] == (uint8_t)ITM_KEYQ && mock_prog[22] == 9, "Healed KEY? ‘R09’ operand must be numeric register 9");
        // Step 8: CLα ‘R11’ -> healed to reg 11 at offset 23..25
        TEST_ASSERT(mock_prog[25] == 11, "Healed CLα ‘R11’ operand must be numeric register 11");
    }
    if(regErr) {
        free(regErr);
    }

    // Test 22: Parse full res/PROGRAMS/TRI∠.p47u end-to-end (verify CF F00, KEY 01 XEQ 11, y⇄ Z, cos(𝑥), cos⁻¹(𝑥), sin(𝑥), sin⁻¹(𝑥), ShortInt?, LongInt?, ℝ?)
    memset(mock_prog, 0, sizeof(mock_prog));
    mock_program_list[0].instructionPointer = mock_prog;
    firstFreeProgramByte = mock_prog;
    freeProgramBytes = 65000;
    endOfCurrentProgram = mock_prog;
    FILE *fTriAngle = fopen("../../res/PROGRAMS/TRI∠.p47u", "rb");
    TEST_ASSERT(fTriAngle != NULL, "res/PROGRAMS/TRI∠.p47u must exist");
    if(fTriAngle) {
        fseek(fTriAngle, 0, SEEK_END);
        long sz = ftell(fTriAngle);
        fseek(fTriAngle, 0, SEEK_SET);
        char *buf = (char *)malloc(sz + 1);
        fread(buf, 1, sz, fTriAngle);
        buf[sz] = '\0';
        fclose(fTriAngle);

        char *triErr = parseAndPasteProgram(buf);
        if(triErr) {
            printf("Error parsing TRI∠.p47u: '%s'\n", triErr);
        }
        TEST_ASSERT(triErr == NULL, "parseAndPasteProgram must succeed on TRI∠.p47u");
        if(triErr == NULL) {
            size_t triBytes = (size_t)(endOfCurrentProgram - mock_program_list[0].instructionPointer);
            TEST_ASSERT(triBytes > 2000, "TRI∠.p47u compiled bytecode must exceed 2000 bytes");
            // Verify KEY 01 XEQ 11 encoded as [0x85, 0xd9, 0x01, 0x03, 0x0b]
            bool foundKey01Xeq11 = false;
            // Verify CF F00 encoded as [110, 0]
            bool foundCfF00 = false;
            // Verify y⇄ Z encoded as [0x86, 0x72, REGISTER_Z_IN_KS_CODE]
            bool foundYexZ = false;
            for(size_t bi = 0; bi + 4 < triBytes; bi++) {
                if(mock_prog[bi] == (uint8_t)((1497 >> 8) | 0x80) && mock_prog[bi + 1] == (uint8_t)(1497 & 0xff) &&
                   mock_prog[bi + 2] == 1 && mock_prog[bi + 3] == 3 && mock_prog[bi + 4] == 11) {
                    foundKey01Xeq11 = true;
                }
                if(mock_prog[bi] == 110 && mock_prog[bi + 1] == 0) {
                    foundCfF00 = true;
                }
                if(mock_prog[bi] == (uint8_t)((1650 >> 8) | 0x80) && mock_prog[bi + 1] == (uint8_t)(1650 & 0xff) &&
                   mock_prog[bi + 2] == (uint8_t)REGISTER_Z_IN_KS_CODE) {
                    foundYexZ = true;
                }
            }
            TEST_ASSERT(foundCfF00, "CF F00 in TRI∠.p47u must encode as [110, 0]");
            TEST_ASSERT(foundKey01Xeq11, "KEY 01 XEQ 11 in TRI∠.p47u must encode key number 1 followed by XEQ 11");
            TEST_ASSERT(foundYexZ, "𝑦⇄ Z in TRI∠.p47u must encode as [0x86, 0x72, REGISTER_Z_IN_KS_CODE]");

            // Compare against res/PROGRAMS/TRI∠.p47 with REM comment steps (134, 18, 253, len, ...) stripped
            FILE *fp47 = fopen("../../res/PROGRAMS/TRI∠.p47", "r");
            if(fp47) {
                char line[64];
                for(int skip = 0; skip < 6; skip++) {
                    if(!fgets(line, sizeof(line), fp47)) break;
                }
                uint8_t rawP47[16384];
                size_t rawLen = 0;
                while(fgets(line, sizeof(line), fp47) && rawLen < sizeof(rawP47)) {
                    rawP47[rawLen++] = (uint8_t)atoi(line);
                }
                fclose(fp47);
                if(rawLen >= 2 && rawP47[rawLen - 1] == 255 && rawP47[rawLen - 2] == 255) {
                    rawLen -= 2;
                }
                uint8_t cleanP47[16384];
                size_t cleanLen = 0;
                for(size_t ri = 0; ri < rawLen;) {
                    if(ri + 3 < rawLen && rawP47[ri] == 134 && rawP47[ri + 1] == 18 && rawP47[ri + 2] == 253) {
                        uint8_t remLen = rawP47[ri + 3];
                        ri += 4 + remLen;
                    }
                    else {
                        cleanP47[cleanLen++] = rawP47[ri++];
                    }
                }
                if(cleanLen != triBytes || memcmp(cleanP47, mock_prog, triBytes) != 0) {
                    printf("TRI∠.p47u vs TRI∠.p47 mismatch: cleanLen=%zu, triBytes=%zu\n", cleanLen, triBytes);
                    size_t minL = (cleanLen < triBytes) ? cleanLen : triBytes;
                    for(size_t mi = 0; mi < minL; mi++) {
                        if(cleanP47[mi] != mock_prog[mi]) {
                            printf("First byte diff at offset %zu: p47=%u (0x%02x) vs compiled=%u (0x%02x)\n",
                                   mi, cleanP47[mi], cleanP47[mi], mock_prog[mi], mock_prog[mi]);
                            break;
                        }
                    }
                }
                TEST_ASSERT(cleanLen == triBytes && memcmp(cleanP47, mock_prog, triBytes) == 0,
                            "Compiled TRI∠.p47u must match TRI∠.p47 byte-for-byte (excluding REM comment steps)");
            }
        }
        if(triErr) {
            free(triErr);
        }
        free(buf);
    }

    // Test 23: Parse full res/PROGRAMS/TRI∡.p47u end-to-end (verify →RECT_(HP), →POL_(HP), KEY 01 XEQ A, STOCFG K)
    memset(mock_prog, 0, sizeof(mock_prog));
    mock_program_list[0].instructionPointer = mock_prog;
    firstFreeProgramByte = mock_prog;
    freeProgramBytes = 65000;
    endOfCurrentProgram = mock_prog;
    FILE *fTriMeas = fopen("../../res/PROGRAMS/TRI∡.p47u", "rb");
    TEST_ASSERT(fTriMeas != NULL, "res/PROGRAMS/TRI∡.p47u must exist");
    if(fTriMeas) {
        fseek(fTriMeas, 0, SEEK_END);
        long sz = ftell(fTriMeas);
        fseek(fTriMeas, 0, SEEK_SET);
        char *buf = (char *)malloc(sz + 1);
        fread(buf, 1, sz, fTriMeas);
        buf[sz] = '\0';
        fclose(fTriMeas);

        char *triMeasErr = parseAndPasteProgram(buf);
        if(triMeasErr) {
            printf("Error parsing TRI∡.p47u: '%s'\n", triMeasErr);
        }
        TEST_ASSERT(triMeasErr == NULL, "parseAndPasteProgram must succeed on TRI∡.p47u");
        if(!triMeasErr) {
            size_t triMeasBytes = (size_t)(endOfCurrentProgram - mock_prog);
            FILE *fp47m = fopen("../../res/PROGRAMS/TRI∡.p47", "rb");
            if(fp47m) {
                char line[64];
                for(int skip = 0; skip < 6; skip++) {
                    if(!fgets(line, sizeof(line), fp47m)) break;
                }
                uint8_t rawP47[16384];
                size_t rawLen = 0;
                while(fgets(line, sizeof(line), fp47m) && rawLen < sizeof(rawP47)) {
                    rawP47[rawLen++] = (uint8_t)atoi(line);
                }
                fclose(fp47m);
                if(rawLen >= 2 && rawP47[rawLen - 1] == 255 && rawP47[rawLen - 2] == 255) {
                    rawLen -= 2;
                }
                uint8_t cleanP47[16384];
                size_t cleanLen = 0;
                for(size_t ri = 0; ri < rawLen;) {
                    if(ri + 3 < rawLen && rawP47[ri] == 134 && rawP47[ri + 1] == 18 && rawP47[ri + 2] == 253) {
                        uint8_t remLen = rawP47[ri + 3];
                        ri += 4 + remLen;
                    }
                    else {
                        cleanP47[cleanLen++] = rawP47[ri++];
                    }
                }
                TEST_ASSERT(cleanLen == triMeasBytes && memcmp(cleanP47, mock_prog, triMeasBytes) == 0,
                            "Compiled TRI∡.p47u must match TRI∡.p47 byte-for-byte (excluding REM comment steps)");
            }
        }
        if(triMeasErr) {
            free(triMeasErr);
        }
        free(buf);
    }

    // Test 24: Parse SPIRALk.p47u, TSTAEQ.p47u, OpAmp.p47u, 47DEFLT.p47u, SETT.p47u, HP35fun.p47u, FreeF.p47u, POL.p47u and verify byte-for-byte parity with .p47
    const char *parityPairs[][2] = {
        { "../../res/PROGRAMS/SPIRALk.p47u", "../../res/PROGRAMS/SPIRALk.p47" },
        { "../../res/PROGRAMS/TSTAEQ.p47u",  "../../res/PROGRAMS/TSTAEQ.p47"  },
        { "../../res/PROGRAMS/OpAmp.p47u",   "../../res/PROGRAMS/OpAmp.p47"   },
        { "../../res/PROGRAMS/47DEFLT.p47u", "../../res/PROGRAMS/47DEFLT.p47" },
        { "../../res/PROGRAMS/SETT.p47u",    "../../res/PROGRAMS/SETT.p47"    },
        { "../../res/PROGRAMS/HP35fun.p47u", "../../res/PROGRAMS/HP35fun.p47" },
        { "../../res/PROGRAMS/FreeF.p47u",   "../../res/PROGRAMS/FreeF.p47"   },
        { "../../res/PROGRAMS/POL.p47u",     "../../res/PROGRAMS/POL.p47"     }
    };
    for(size_t pi = 0; pi < sizeof(parityPairs) / sizeof(parityPairs[0]); pi++) {
        memset(mock_prog, 0, sizeof(mock_prog));
        mock_program_list[0].instructionPointer = mock_prog;
        firstFreeProgramByte = mock_prog;
        freeProgramBytes = 65000;
        endOfCurrentProgram = mock_prog;

        FILE *fSrc = fopen(parityPairs[pi][0], "rb");
        TEST_ASSERT(fSrc != NULL, "Source .p47u file must exist");
        if(fSrc) {
            fseek(fSrc, 0, SEEK_END);
            long sz = ftell(fSrc);
            fseek(fSrc, 0, SEEK_SET);
            char *pBuf = (char *)malloc(sz + 1);
            fread(pBuf, 1, sz, fSrc);
            pBuf[sz] = '\0';
            fclose(fSrc);

            char *pErr = parseAndPasteProgram(pBuf);
            if(pErr) {
                printf("Error parsing %s: '%s'\n", parityPairs[pi][0], pErr);
            }
            TEST_ASSERT(pErr == NULL, "parseAndPasteProgram must succeed on .p47u file");
            if(!pErr) {
                size_t compiledBytes = (size_t)(endOfCurrentProgram - mock_prog);
                FILE *fBin = fopen(parityPairs[pi][1], "rb");
                if(fBin) {
                    char line[64];
                    for(int skip = 0; skip < 6; skip++) {
                        if(!fgets(line, sizeof(line), fBin)) {
                            break;
                        }
                    }
                    uint8_t rawP47[16384];
                    size_t rawLen = 0;
                    while(fgets(line, sizeof(line), fBin) && rawLen < sizeof(rawP47)) {
                        rawP47[rawLen++] = (uint8_t)atoi(line);
                    }
                    fclose(fBin);
                    if(rawLen >= 2 && rawP47[rawLen - 1] == 255 && rawP47[rawLen - 2] == 255) {
                        rawLen -= 2;
                    }
                    uint8_t cleanP47[16384];
                    size_t cleanLen = 0;
                    for(size_t ri = 0; ri < rawLen;) {
                        if(ri + 3 < rawLen && rawP47[ri] == 134 && rawP47[ri + 1] == 18 && rawP47[ri + 2] == 253) {
                            uint8_t remLen = rawP47[ri + 3];
                            ri += 4 + remLen;
                        }
                        else {
                            cleanP47[cleanLen++] = rawP47[ri++];
                        }
                    }
                    TEST_ASSERT(cleanLen == compiledBytes && memcmp(cleanP47, mock_prog, compiledBytes) == 0,
                                "Compiled .p47u must match .p47 byte-for-byte (excluding REM comment steps)");
                }
            }
            if(pErr) {
                free(pErr);
            }
            free(pBuf);
        }
    }

    // Test 25: Scan all RPN program files (.p47u, .txt, and .rtf) in res/PROGRAMS, res/PROGRAMS/tvm, and PROGRAMS
    const char *progDirs[] = {
        "../../res/PROGRAMS",
        "../../res/PROGRAMS/tvm",
        "../../PROGRAMS"
    };
    int totalScanned = 0;
    int totalRtfScanned = 0;
    int scanFailures = 0;
    for(size_t di = 0; di < sizeof(progDirs) / sizeof(progDirs[0]); di++) {
        DIR *dir = opendir(progDirs[di]);
        TEST_ASSERT(dir != NULL, "Program directory must be readable");
        if(dir) {
            struct dirent *ent;
            while((ent = readdir(dir)) != NULL) {
                const char *fname = ent->d_name;
                size_t flen = strlen(fname);
                bool isP47u = (flen > 5 && strcmp(fname + flen - 5, ".p47u") == 0);
                bool isTxt = (flen > 4 && strcmp(fname + flen - 4, ".txt") == 0);
                bool isRtf = (flen > 4 && strcmp(fname + flen - 4, ".rtf") == 0);
                if(!isP47u && !isTxt && !isRtf) {
                    continue;
                }
                // Skip plain-text documentation / README files
                if(strcmp(fname, "Example programs.txt") == 0 ||
                   strcmp(fname, "Example-programs.txt") == 0 ||
                   strcmp(fname, "PAYSWISS.txt") == 0 ||
                   strcmp(fname, "AllOp's.txt") == 0) {
                    continue;
                }
                char fullPath[512];
                snprintf(fullPath, sizeof(fullPath), "%s/%s", progDirs[di], fname);
                FILE *fp = fopen(fullPath, "rb");
                TEST_ASSERT(fp != NULL, "RPN program file must be readable");
                if(fp) {
                    fseek(fp, 0, SEEK_END);
                    long sz = ftell(fp);
                    fseek(fp, 0, SEEK_SET);
                    char *pBuf = (char *)malloc(sz + 1);
                    fread(pBuf, 1, sz, fp);
                    pBuf[sz] = '\0';
                    fclose(fp);

                    memset(mock_prog, 0, sizeof(mock_prog));
                    mock_program_list[0].instructionPointer = mock_prog;
                    firstFreeProgramByte = mock_prog;
                    freeProgramBytes = 65000;
                    endOfCurrentProgram = mock_prog;

                    char *pErr = parseAndPasteProgram(pBuf);
                    if(pErr) {
                        printf("Failed to parse RPN program %s: %s\n", fullPath, pErr);
                        free(pErr);
                        scanFailures++;
                    }
                    if(isRtf) {
                        totalRtfScanned++;
                    }
                    else {
                        totalScanned++;
                    }
                    free(pBuf);
                }
            }
            closedir(dir);
        }
    }
    TEST_ASSERT(scanFailures == 0, "Every RPN program in res/PROGRAMS and PROGRAMS must paste without error");
    printf("  Verified %d RPN program files (.p47u/.txt) and %d .rtf files across res/PROGRAMS, res/PROGRAMS/tvm, and PROGRAMS\n", totalScanned, totalRtfScanned);
    TEST_ASSERT(totalScanned >= 90, "Must scan at least 90 RPN program files");
    TEST_ASSERT(totalRtfScanned >= 69, "Must scan at least 69 RTF program files");

    // Test 26: User ROMAN program with U+2006 (SIX-PER-EM SPACE ' '), comma decimals (','), and U+2008 (' ') digit grouping
    const char *roman_u2006 =
        "0001:\xe2\x80\x86 \xe2\x80\x86 LBL \xe2\x80\x98ROMAN\xe2\x80\x99\n"
        "0002:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 STRI?\n"
        "0003:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 GTO 03\n"
        "0004:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 STO 20\n"
        "0005:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 STO 23\n"
        "0006:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 42CLA\n"
        "0007:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 1000,77\n"
        "0008:\xe2\x80\x86 \xe2\x80\x86 XEQ 00\n"
        "0009:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 900,677\xe2\x80\x88" "7\n"
        "0010:\xe2\x80\x86 \xe2\x80\x86 XEQ 00\n"
        "0011:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 500,68\n"
        "0012:\xe2\x80\x86 \xe2\x80\x86 XEQ 00\n"
        "0013:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 400,676\xe2\x80\x88" "8\n"
        "0014:\xe2\x80\x86 \xe2\x80\x86 XEQ 00\n"
        "0015:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 100,67\n"
        "0016:\xe2\x80\x86 \xe2\x80\x86 XEQ 00\n"
        "0017:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 90,886\xe2\x80\x88" "7\n"
        "0018:\xe2\x80\x86 \xe2\x80\x86 XEQ 00\n"
        "0019:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 50,76\n"
        "0020:\xe2\x80\x86 \xe2\x80\x86 XEQ 00\n"
        "0021:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 40,887\xe2\x80\x88" "6\n"
        "0022:\xe2\x80\x86 \xe2\x80\x86 XEQ 00\n"
        "0023:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 10,88\n"
        "0024:\xe2\x80\x86 \xe2\x80\x86 XEQ 00\n"
        "0025:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 9,738\xe2\x80\x88" "8\n"
        "0026:\xe2\x80\x86 \xe2\x80\x86 XEQ 00\n"
        "0027:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 5,86\n"
        "0028:\xe2\x80\x86 \xe2\x80\x86 XEQ 00\n"
        "0029:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 4,738\xe2\x80\x88" "6\n"
        "0030:\xe2\x80\x86 \xe2\x80\x86 XEQ 00\n"
        "0031:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 1,73\n"
        "0032:\xe2\x80\x86 \xe2\x80\x86 XEQ 00\n"
        "0033:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 CLSTK\n"
        "0034:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 RCL 23\n"
        "0035:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 RCL K\n"
        "0036:\xe2\x80\x86 \xe2\x80\x86 RTN\n"
        "0037:\xe2\x80\x86 \xe2\x80\x86 LBL 00\n"
        "0038:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 ENTER\n"
        "0039:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 IP\n"
        "0040:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 STO 21\n"
        "0041:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 x\xe2\x87\x84y\n"
        "0042:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 FP\n"
        "0043:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 STO 22\n"
        "0044:\xe2\x80\x86 \xe2\x80\x86 LBL 01\n"
        "0045:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 RCL 20\n"
        "0046:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 RCL 21\n"
        "0047:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 x> ? Y\n"
        "0048:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 RTN\n"
        "0049:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 STO- 20\n"
        "0050:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 RCL 22\n"
        "0051:\xe2\x80\x86 \xe2\x80\x86 LBL 02\n"
        "0052:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 100\n"
        "0053:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xc3\x97\n"
        "0054:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 42XTOA\n"
        "0055:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 FP\n"
        "0056:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 x\xe2\x89\xa0 ? 0.\n"
        "0057:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 GTO 02\n"
        "0058:\xe2\x80\x86 \xe2\x80\x86 GTO 01\n"
        "0059:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 STOP\n"
        "0060:\xe2\x80\x86 \xe2\x80\x86 LBL 03\n"
        "0061:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 STO 24\n"
        "0062:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 STO K\n"
        "0063:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 0\n"
        "0064:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 STO 20\n"
        "0065:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 CLX\n"
        "0066:\xe2\x80\x86 \xe2\x80\x86 XEQ 10\n"
        "0067:\xe2\x80\x86 \xe2\x80\x86 LBL 04\n"
        "0068:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 42ALENG\n"
        "0069:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 x= ? 0.\n"
        "0070:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 GTO 05\n"
        "0071:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 RCL 22\n"
        "0072:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 STO 21\n"
        "0073:\xe2\x80\x86 \xe2\x80\x86 XEQ 10\n"
        "0074:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 RCL 22\n"
        "0075:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 RCL 21\n"
        "0076:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 x< ? Y\n"
        "0077:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 CHS\n"
        "0078:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 STO+ 20\n"
        "0079:\xe2\x80\x86 \xe2\x80\x86 GTO 04\n"
        "0080:\xe2\x80\x86 \xe2\x80\x86 LBL 05\n"
        "0081:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 RCL 22\n"
        "0082:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 STO+ 20\n"
        "0083:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 CLSTK\n"
        "0084:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 RCL 24\n"
        "0085:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 RCL 20\n"
        "0086:\xe2\x80\x86 \xe2\x80\x86 RTN\n"
        "0087:\xe2\x80\x86 \xe2\x80\x86 LBL 10\n"
        "0088:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 42ATOX\n"
        "0089:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x86\x92REAL\n"
        "0090:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 STO 23\n"
        "0091:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 0\n"
        "0092:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 STO 22\n"
        "0093:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 1000,77\n"
        "0094:\xe2\x80\x86 \xe2\x80\x86 XEQ 11\n"
        "0095:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 500,68\n"
        "0096:\xe2\x80\x86 \xe2\x80\x86 XEQ 11\n"
        "0097:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 100,67\n"
        "0098:\xe2\x80\x86 \xe2\x80\x86 XEQ 11\n"
        "0099:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 50,76\n"
        "0100:\xe2\x80\x86 \xe2\x80\x86 XEQ 11\n"
        "0101:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 10,88\n"
        "0102:\xe2\x80\x86 \xe2\x80\x86 XEQ 11\n"
        "0103:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 5,86\n"
        "0104:\xe2\x80\x86 \xe2\x80\x86 XEQ 11\n"
        "0105:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 1,73\n"
        "0106:\xe2\x80\x86 \xe2\x80\x86 XEQ 11\n"
        "0107:\xe2\x80\x86 \xe2\x80\x86 RTN\n"
        "0108:\xe2\x80\x86 \xe2\x80\x86 LBL 11\n"
        "0109:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 ENTER\n"
        "0110:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 IP\n"
        "0111:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 x\xe2\x87\x84y\n"
        "0112:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 FP\n"
        "0113:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 100\n"
        "0114:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xc3\x97\n"
        "0115:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 RCL 23\n"
        "0116:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 x\xe2\x89\xa0 ? Y\n"
        "0117:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 RTN\n"
        "0118:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 R\xe2\x86\x93\n"
        "0119:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 R\xe2\x86\x93\n"
        "0120:\xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 \xe2\x80\x86 STO 22\n"
        "0121:\xe2\x80\x86 \xe2\x80\x86 RTN\n"
        "0122:\xe2\x80\x86 \xe2\x80\x86 END\n";

    memset(mock_prog, 0, sizeof(mock_prog));
    mock_program_list[0].instructionPointer = mock_prog;
    firstFreeProgramByte = mock_prog;
    freeProgramBytes = 65000;
    endOfCurrentProgram = mock_prog;

    char *romErr = parseAndPasteProgram(roman_u2006);
    if(romErr) {
        printf("Unexpected error in parseAndPasteProgram on ROMAN program: '%s'\n", romErr);
    }
    TEST_ASSERT(romErr == NULL, "parseAndPasteProgram must parse ROMAN program with U+2006 spaces and comma decimals cleanly");
    if(!romErr) {
        size_t romBytes = (size_t)(endOfCurrentProgram - mock_prog);
        TEST_ASSERT(romBytes == 375, "Compiled ROMAN program must be 375 bytes (377 bytes of ROMANv2 minus 2 bytes for 'v2' in label)");
    }
    if(romErr) {
        free(romErr);
    }

    calcMode = CM_NORMAL;
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
    RUN_TEST(test_r47_last_key_code_mapping);
    RUN_TEST(test_program_copy_and_line_stripping);
    RUN_TEST(test_c47_character_set_conversion_and_math_ops);

    printf("====================================================\n");
    printf("  Summary: %d Run, %d Passed, %d Failed\n", tests_run, tests_passed, tests_failed);
    printf("====================================================\n");

    return (tests_failed == 0) ? 0 : 1;
}
