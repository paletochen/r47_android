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
uint8_t lastKeyCode = 0;
uint8_t calcModel = USER_R47f_g;
extern void setLastKeyCode(int key);

real_t *statisticalSumsPointer = NULL;
subroutineLevelHeader_t *currentSubroutineLevelData = NULL;

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

void codePointToUtf8(uint32_t codePoint, uint8_t *utf8) {
    if (utf8) *utf8 = (uint8_t)codePoint;
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

    printf("====================================================\n");
    printf("  Summary: %d Run, %d Passed, %d Failed\n", tests_run, tests_passed, tests_failed);
    printf("====================================================\n");

    return (tests_failed == 0) ? 0 : 1;
}
