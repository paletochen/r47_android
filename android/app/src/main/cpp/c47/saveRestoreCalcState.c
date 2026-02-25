// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"

// This is used for the backup.cfg simulator backup file
// The variable backupVersion is used in the connection
#define BACKUP_VERSION                     1014     // long press f/g
/*
1004     // Replace Norm_Key_00_VAR by the structure Norm_Key_00;
1005     // 2024-09-06 Remove superfluous reporting when old cfg file items are not found in new files
1006     // 2024-11-07 Remove Aspect and add PLOT_PLUS
1007     // Remove all PLSTAT flags incl. PLOT_PLUS...
1008     // 2024-11018 Add lastCenturyHighUsed
1009     // Change matrix headers, add tag
1010     // Change constant format in equation, adding a # prefix
1011     // Added reserve variables UY, LY, UEST, LEST.
*/


// This is used for the state files
#define configFileVersion                  10000023 // long press f/g
#define VersionAllowed                     10000005 // This code will not autoload versions earlier than this
/*
10000001 // arbitrary starting point version 10 000 001
10000002 // 2022-12-05 First release, version 108_08f
10000003 // 2022-12-06 version 108_08h, added LongPressM & LongPressF
10000004 // 2022-12-26 version 108_08n, added lastIntegerBase
10000005 // 2022-01-08 version 108_08q, Pauli changed the real number saver representaiton
...
10000008 // 2023-09-12 version 108.13   Jaco added the missing STOCFG items, remove the unneccary STOCFG items, added the missing STATe file items.
...
10000012 // 5 flags converted from C47
10000013 // Replace Norm_Key_00_VAR by the structure Norm_Key_00; Arbitrary starting point version 10 000 001 of STATE files. Allowable values are 10000000 to 20000000
10000014 // 2024-11-07 configFileVersion                  10000014 // Remove Aspect and add PLOT_PLUS
10000015 // 2024-11    configFileVersion                  10000015 // Remove all PLSTAT flags incl. PLOT_PLUS...
10000016 // 2024-11-18 configFileVersion                  10000016 // Add lastCenturyHighUsed
...
10000020 // Change bcd

Current version defaults all non-loaded settings from previous version files correctly
*/

#define backupFileName (CALCMODEL == USER_C47 ? "backup.cfg" : "backupR47.cfg")

#define LOADDEBUG
#undef LOADDEBUG


uint16_t flushBufferCnt = 0;
#if !defined(TESTSUITE_BUILD)
  #define START_REGISTER_VALUE 860  // 2025/09/06 [DL] reduced fromm 1000 to provide enougth room in tmpRegisterString for config data (840 bytes):
                                    //                 tmpRegisterString is a part of the global tmpString which is 2560 bytes (aux_buf_ptr buffer provided by DMCP)
                                    //                 config string length is 1680 bytes (840 x 2) so tmpRegisterString should start at max at 880 (2560 - 1680)
                                    //                 THIS VALUE NEEDS TO BE RE-EVALUATED IF THE CONFIG DATA LENGTH IS INCREASED
  static uint32_t loadedVersion = 0;
  static char *tmpRegisterString = NULL;

  static void save(const void *buffer, uint32_t size) {
    ioFileWrite(buffer, size);
  }

  static uint32_t restore(void *buffer, uint32_t size) {
    return ioFileRead(buffer, size);
  }
#endif // !TESTSUITE_BUILD


uint8_t convert001090400T001090500(uint8_t parameter, uint8_t offset) { //Example: read from file: RB_F14 (which was 0) and and report RBX_F14 (which is 221) to the program.
  uint8_t output = parameter;
  if(parameter < 20) {
    output = parameter + offset;
  }
  return output;
}

#if !defined(TESTSUITE_BUILD)
// Forced base-10 conversion functions
static int16_t toInt16(const char *str) {
  return (int16_t)strtol(str, NULL, 10);
}

int32_t toInt32(const char *str) {
  return strtol(str, NULL, 10);
}

static uint8_t toUint8(const char *str) {
  return (uint8_t)strtoul(str, NULL, 10);
}

static uint16_t toUint16(const char *str) {
  return (uint16_t)strtoul(str, NULL, 10);
}

static uint32_t toUint32(const char *str) {
  return strtoul(str, NULL, 10);
}

// Floating point conversion functions
float stringToFloat(const char *str) {
  return strtof(str, NULL);
}

//Utility to add angle and polar markers
static void textTag(char *str, const uint8_t angle, const uint8_t polmode) {
  if(angle != amNone) {
    switch(getTagAngularMode(angle)) {
      case amDegree: {
        strcat(str, ":DEG");
        break;
      }
      case amDMS: {
        strcat(str, ":DMS");
        break;
      }
      case amRadian: {
        strcat(str, ":RAD");
        break;
      }
      case amMultPi: {
        strcat(str, ":MULTPI");
        break;
      }
      case amGrad: {
        strcat(str, ":GRAD");
        break;
      }
      case amNone: {
        break;
      }
      default: {
        strcpy(str, ":???");
        break;
      }
    }
  }
  if((polmode & amPolar) == amPolar) {
    strcat(str, "p");
  }
}

// Utility routines to skip stuff
static char *skip_word(const char *str) {
  while(*str != ' ')
    str++;
  return (char *)str;
}

static char *skip_space(const char *str) {
  while(*str == ' ')
    str++;
  return (char *)str;
}

static char *next_word(const char *str) {
  return skip_word(skip_space(str));
}

static char *skip_to_space_newline(const char *str) {
  while(*str != ' ' && *str != '\n' && *str != 0)
    str++;
  return (char *)str;
}

static char *toInt16_next_word(const char *str, int16_t *val) {
    *val = toInt16(str);
    return next_word(str);
}

static void _updateConstantsInEquations(void) {
  for(uint16_t i = 0; i < numberOfFormulae; i++) {
    if(allFormulae[i].pointerToFormulaData != C47_NULL) {
      parseEquation(i, EQUATION_PARSER_MVAR, aimBuffer, tmpString);
      if(lastErrorCode == 0) {   // if equation is valid, check and update constants
        const char *strPtr = (char *)TO_PCMEMPTR(allFormulae[i].pointerToFormulaData);
        strcpy(tmpString, strPtr);
        updateOldConstants = true;
        parseEquation(i, EQUATION_PARSER_MVAR, aimBuffer, tmpString);
        updateOldConstants = false;
        setEquation(i, tmpString);
      }
      else {
        lastErrorCode = 0;
      }
    }
  }
}
#endif

#if defined(PC_BUILD)
static void convertOldMatrixHeaderToNewMatrixHeader(calcRegister_t regist) {
  //converting old matrix register headers in the form 0xrrrrcccc to 0xttrrrccc including the tag.
  //input expects the old matrix dimensions read into the new header as follows:
  //  memory block contains 0xrrrrcccc and is read into cols=0x0ccc and rows=0x0rrr
  //  this is rewritten to  0x00rrrccc and a amNone tag is applied with Polar off.
  //  resulting in          0x05rrrccc
  //this MUST only be called for version numbers indicating an old backup file, and an old state file.
  //it CANNOT be run on a new matrix header, as the cols read in form 0x0FFF will be shifted by 4 bits to align and will break if already shifted meaning if it already is new format.
  //Old matrixes with rows or cols > 12 bits 0x0FFF will fail. It is however unreasonable to expect such large matrix dimensions of 2^12-1 = 4095.
  if(getRegisterDataType(regist) == dtReal34Matrix || getRegisterDataType(regist) == dtComplex34Matrix) {
    #if defined PC_BUILD
      printf("----------------R%2u Old matrix header: r=%i c=%i \n",regist, (REGISTER_MATRIX_HEADER (regist)->matrixRows), (REGISTER_MATRIX_HEADER (regist)->matrixColumns));
    #endif //PC_BUILD
    uint32_t row = (REGISTER_MATRIX_HEADER(regist)->matrixRows) & 0x0FFF;
    uint32_t col = ((REGISTER_MATRIX_HEADER(regist)->matrixColumns) >> 4) & 0x0FFF;
    #if defined PC_BUILD
      printf("----------------R%2u New matrix header: r=%i c=%i \n",regist, row, col);
    #endif //PC_BUILD
    REGISTER_MATRIX_HEADER(regist)->matrixRows = row;
    REGISTER_MATRIX_HEADER(regist)->matrixColumns = col;
    //printf("R%2u: getRegisterTag=%u REGISTER_MATRIX_HEADER(regist)->mtag=%u\n", regist, getRegisterTag(regist), REGISTER_MATRIX_HEADER(regist)->mtag);
    REGISTER_MATRIX_HEADER(regist)->mtag = amNone; //clear spare bits and clear Polar flag, setting only amNone.
  }
}


  cfgFileParam_t *paramHead=NULL, *paramCurrent;

  static void changeCommaToPeriod(char *str) {
    char *s;

    s = strchr(str, ':') + 1;
    s = strchr(s, ':') + 1;
    while(*s) {
      if(*s == ',') {
        *s = '.';
      }
      s++;
    }
  }

  static void saveStateValue(const void *buffer, uint32_t size, const char *valueName, const char *valueType) {
    char value[200];

    if(!strcmp(valueType, "int64")) {
      sprintf(value, "%s:%s:%" PRIi64 LINEBREAK, valueName, valueType, *(int64_t *)buffer);
      save(value, strlen(value));
    }

    else if(!strcmp(valueType, "uint64")) {
      sprintf(value, "%s:%s:%" PRIu64 LINEBREAK, valueName, valueType, *(uint64_t *)buffer);
      save(value, strlen(value));
    }

    else if(!strcmp(valueType, "int32")) {
      sprintf(value, "%s:%s:%" PRIi32 LINEBREAK, valueName, valueType, *(int32_t *)buffer);
      save(value, strlen(value));
    }

    else if(!strcmp(valueType, "uint32")) {
      sprintf(value, "%s:%s:%" PRIu32 LINEBREAK, valueName, valueType, *(uint32_t *)buffer);
      save(value, strlen(value));
    }

    else if(!strcmp(valueType, "int16")) {
      sprintf(value, "%s:%s:%" PRIi16 LINEBREAK, valueName, valueType, *(int16_t *)buffer);
      save(value, strlen(value));
    }

    else if(!strcmp(valueType, "uint16")) {
      sprintf(value, "%s:%s:%" PRIu16 LINEBREAK, valueName, valueType, *(uint16_t *)buffer);
      save(value, strlen(value));
    }

    else if(!strcmp(valueType, "int8")) {
      sprintf(value, "%s:%s:%" PRIi8 LINEBREAK, valueName, valueType, *(int8_t *)buffer);
      save(value, strlen(value));
    }

    else if(!strcmp(valueType, "uint8")) {
      sprintf(value, "%s:%s:%" PRIu8 LINEBREAK, valueName, valueType, *(uint8_t *)buffer);
      save(value, strlen(value));
    }

    else if(!strcmp(valueType, "float")) {
      sprintf(value, "%s:%s:%.20e" LINEBREAK, valueName, valueType, *(float *)buffer);
      changeCommaToPeriod(value);
      save(value, strlen(value));
    }

    else if(!strcmp(valueType, "double")) {
      sprintf(value, "%s:%s:%.20e" LINEBREAK, valueName, valueType, *(double *)buffer);
      changeCommaToPeriod(value);
      save(value, strlen(value));
    }

    else if(!strcmp(valueType, "real")) {
      sprintf(value, "%s:%s:", valueName, valueType);
      if(realIsInfinite((real_t *)buffer)) {
        if(realIsNegative((real_t *)buffer)) {
          strcpy(value + strlen(value), "-9.9e9999999");
        }
        else {
          strcpy(value + strlen(value), "9.9e9999999");
        }
      }
      else {
        realToString(buffer, value + strlen(value));
      }
      strcat(value, LINEBREAK);
      changeCommaToPeriod(value);
      save(value, strlen(value));
    }

    else if(!strcmp(valueType, "real34")) {
      sprintf(value, "%s:%s:", valueName, valueType);
      if(real34IsInfinite((real34_t *)buffer)) {
        if(real34IsNegative((real34_t *)buffer)) {
          strcpy(value + strlen(value), "-9.9e9999");
        }
        else {
          strcpy(value + strlen(value), "9.9e9999");
        }
      }
      else {
        real34ToString(buffer, value + strlen(value));
      }
      strcat(value, LINEBREAK);
      changeCommaToPeriod(value);
      save(value, strlen(value));
    }

    else if(!strcmp(valueType, "bool")) {
      sprintf(value, "%s:%s:%" PRIu32 LINEBREAK, valueName, valueType, *(bool_t *)buffer);
      save(value, strlen(value));
    }

    else if(!strcmp(valueType, "c47Ptr")) {
      sprintf(value, "%s:%s:%" PRIu32 " (0x%05x)" LINEBREAK, valueName, valueType, *(uint32_t *)buffer, 4 * *(uint32_t *)buffer);
      save(value, strlen(value));
    }

    else if(!strcmp(valueType, "hexDump")) {
      uint32_t addr, b;

      sprintf(value, "%s:%s:%" PRIu32 LINEBREAK, valueName, valueType, size);
      save(value, strlen(value));

      for(addr=0; addr<size; addr+=32) {
        // Hexadecimal dump
        sprintf(value, "%05x  ", addr);
        for(b=0; b<32; b++) {
          if(addr + b < size) {
            sprintf(value + 7 + 3*b, "%02x ", *(uint8_t *)(buffer + addr + b));
          }
          else {
            strcpy(value + 7 + 3*b, "   ");
          }
        }

        // ASCII dump
        strcpy(value + 103, " '");
        for(b=0; b<32; b++) {
          if(addr + b < size) {
            sprintf(value + 105 + b, "%c", *(char *)(buffer + addr + b) >= ' ' && *(char *)(buffer + addr + b) != 0x7f ? *(char *)(buffer + addr + b) : ' ');
          }
          else {
            strcpy(value + 105 + b, " ");
          }
        }
        strcpy(value + 137, "'" LINEBREAK);

        save(value, strlen(value));
      }
    }

    //save and restore screenData is not mandatory
    //else if(!strcmp(valueType, "screenData")) {
    //  char screenRow[401];
    //  screenRow[400] = 0;
    //
    //  sprintf(value, "%s:%s:%dx%d" LINEBREAK, valueName, valueType, SCREEN_WIDTH, SCREEN_HEIGHT);
    //  save(value, strlen(value));
    //
    //  uint32_t addr=0;
    //  for(int y = 0; y < SCREEN_HEIGHT; ++y) {
    //    uint8_t bmpdata = 0;
    //    for(int x = 0; x < SCREEN_WIDTH; ++x) {
    //      bmpdata <<= 1;
    //      if(*((uint32_t *)buffer + y*screenStride + x) == ON_PIXEL) {
    //        bmpdata |= 1;
    //        screenRow[x] = '#';
    //      }
    //      else {
    //        screenRow[x] = ' ';
    //      }
    //      if((x % 8) == 7) {
    //        if(++addr % 50 == 0) {
    //          sprintf(value, "%s|" LINEBREAK, screenRow);
    //          save(value, strlen(value));
    //        }
    //        bmpdata = 0;
    //      }
    //    }
    //  }
    //}

    else {
      printf("ERROR: valueType %s unknown in function saveStateValue!" LINEBREAK, valueType);
    }
  }

  void saveCalc(void) {
    uint32_t ramSizeInBlocks = RAM_SIZE_IN_BLOCKS;
    uint32_t ramPtr, backupVersion = BACKUP_VERSION;
    int ret;

    //prevent demo profiles from being saved
    if(!(    (CALCMODEL == USER_R47 && (calcModel == USER_R47f_g || calcModel == USER_R47fg_g || calcModel == USER_R47fg_bk || calcModel == USER_R47bk_fg))  //this is to prevent any test layout to store the the layount name in the backup file
          || (CALCMODEL == USER_C47 && (calcModel == USER_C47    || calcModel == USER_DM42 ))
        ) )  {
      printf("Not saving the backup cfg file due to a non-permanent layout being selected\n");
      return;
    }


    ret = ioFileOpen(ioPathBackup, ioModeWrite);

    if(ret != FILE_OK) {
      if(ret == FILE_CANCEL) {
        return;
      }
      else {
        printf("Cannot save calc's memory in file backup.bin!\n");
        exit(0);
      }
    }

    if(calcMode == CM_CONFIRMATION) {
      calcMode = previousCalcMode;
      refreshScreen(90);
    }

    printf("Begin of calc's backup\n");

    // The order in which parameters are saved doesn't matter
    // When a parameter is removed, simply remove the corresponding saveStateValue(...) and restoreStateValue(...) lines.
    saveStateValue(&backupVersion,                  sizeof(backupVersion),                                       "backupVersion",                  "uint32");
    saveStateValue(&ramSizeInBlocks,                sizeof(ramSizeInBlocks),                                     "ramSizeInBlocks",                "uint32");
    saveStateValue(&numberOfFreeMemoryRegions,      sizeof(numberOfFreeMemoryRegions),                           "numberOfFreeMemoryRegions",      "int32");
    saveStateValue(freeMemoryRegions,               sizeof(freeMemoryRegion_t) * numberOfFreeMemoryRegions,      "freeMemoryRegions",              "hexDump");
    saveStateValue(&numberOfAllocatedMemoryRegions, sizeof(numberOfAllocatedMemoryRegions),                      "numberOfAllocatedMemoryRegions", "int32");
    saveStateValue(allocatedMemoryRegions,          sizeof(freeMemoryRegion_t) * numberOfAllocatedMemoryRegions, "allocatedMemoryRegions",         "hexDump");
    saveStateValue(globalFlags,                     sizeof(globalFlags),                                         "globalFlags",                    "hexDump");
    saveStateValue(errorMessage,                    ERROR_MESSAGE_LENGTH,                                        "errorMessage",                   "hexDump");
    saveStateValue(aimBuffer,                       AIM_BUFFER_LENGTH,                                           "aimBuffer",                      "hexDump");
    saveStateValue(nimBufferDisplay,                NIM_BUFFER_LENGTH,                                           "nimBufferDisplay",               "hexDump");
    saveStateValue(tamBuffer,                       TAM_BUFFER_LENGTH,                                           "tamBuffer",                      "hexDump");
    saveStateValue(asmBuffer,                       sizeof(asmBuffer),                                           "asmBuffer",                      "hexDump");
    saveStateValue(oldTime,                         sizeof(oldTime),                                             "oldTime",                        "hexDump");
    saveStateValue(dateTimeString,                  sizeof(dateTimeString),                                      "dateTimeString",                 "hexDump");
    saveStateValue(softmenuStack,                   sizeof(softmenuStack),                                       "softmenuStack",                  "hexDump");
    saveStateValue(globalRegister,                  sizeof(registerHeader_t) * NUMBER_OF_GLOBAL_REGISTERS,       "globalRegister",                 "hexDump");
    saveStateValue(kbd_usr,                         sizeof(kbd_usr),                                             "kbd_usr",                        "hexDump");
    saveStateValue(userMenuItems,                   sizeof(userMenuItems),                                       "userMenuItems",                  "hexDump");
    saveStateValue(userAlphaItems,                  sizeof(userAlphaItems),                                      "userAlphaItems",                 "hexDump");
    saveStateValue(lastTemp,                        sizeof(lastTemp),                                            "lastTemp",                       "hexDump");
    saveStateValue(&lastI,                          sizeof(lastI),                                               "lastI",                          "int16");
    saveStateValue(&lastJ,                          sizeof(lastJ),                                               "lastJ",                          "int16");
    saveStateValue(&lastFunc,                       sizeof(lastFunc),                                            "lastFunc",                       "int16");
    saveStateValue(&lastParam,                      sizeof(lastParam),                                           "lastParam",                      "int16");
    saveStateValue(&tam.mode,                       sizeof(tam.mode),                                            "tam.mode",                       "uint16");
    saveStateValue(&tam.function,                   sizeof(tam.function),                                        "tam.function",                   "int16");
    saveStateValue(&tam.alpha,                      sizeof(tam.alpha),                                           "tam.alpha",                      "bool");
    saveStateValue(&tam.currentOperation,           sizeof(tam.currentOperation),                                "tam.currentOperation",           "int16");
    saveStateValue(&tam.dot,                        sizeof(tam.dot),                                             "tam.dot",                        "bool");
    saveStateValue(&tam.indirect,                   sizeof(tam.indirect),                                        "tam.indirect",                   "bool");
    saveStateValue(&tam.digitsSoFar,                sizeof(tam.digitsSoFar),                                     "tam.digitsSoFar",                "int16");
    saveStateValue(&tam.value,                      sizeof(tam.value),                                           "tam.value",                      "int16");
    saveStateValue(&tam.min,                        sizeof(tam.min),                                             "tam.min",                        "int16");
    saveStateValue(&tam.max,                        sizeof(tam.max),                                             "tam.max",                        "int16");
    saveStateValue(&rbrRegister,                    sizeof(rbrRegister),                                         "rbrRegister",                    "int16");
    saveStateValue(&numberOfNamedVariables,         sizeof(numberOfNamedVariables),                              "numberOfNamedVariables",         "int16");
    saveStateValue(&xCursor,                        sizeof(xCursor),                                             "xCursor",                        "uint32");
    saveStateValue(&yCursor,                        sizeof(yCursor),                                             "yCursor",                        "uint32");
    saveStateValue(&firstGregorianDay,              sizeof(firstGregorianDay),                                   "firstGregorianDay",              "uint32");
    saveStateValue(&denMax,                         sizeof(denMax),                                              "denMax",                         "uint32");
    saveStateValue(&lastDenominator,                sizeof(lastDenominator),                                     "lastDenominator",                "uint32");
    saveStateValue(&currentRegisterBrowserScreen,   sizeof(currentRegisterBrowserScreen),                        "currentRegisterBrowserScreen",   "int16");
    saveStateValue(&currentFntScr,                  sizeof(currentFntScr),                                       "currentFntScr",                  "uint8");
    saveStateValue(&currentFlgScr,                  sizeof(currentFlgScr),                                       "currentFlgScr",                  "uint8");
    saveStateValue(&displayFormat,                  sizeof(displayFormat),                                       "displayFormat",                  "uint8");
    saveStateValue(&displayFormatDigits,            sizeof(displayFormatDigits),                                 "displayFormatDigits",            "uint8");
    saveStateValue(&timeDisplayFormatDigits,        sizeof(timeDisplayFormatDigits),                             "timeDisplayFormatDigits",        "uint8");
    saveStateValue(&shortIntegerWordSize,           sizeof(shortIntegerWordSize),                                "shortIntegerWordSize",           "uint8");
    saveStateValue(&significantDigits,              sizeof(significantDigits),                                   "significantDigits",              "uint8");
    saveStateValue(&fractionDigits,                 sizeof(fractionDigits),                                      "fractionDigits",                 "uint8");
    saveStateValue(&shortIntegerMode,               sizeof(shortIntegerMode),                                    "shortIntegerMode",               "uint8");
    saveStateValue(&currentAngularMode,             sizeof(currentAngularMode),                                  "currentAngularMode",             "uint32");
    saveStateValue(&scrLock,                        sizeof(scrLock),                                             "scrLock",                        "uint8");
    saveStateValue(&roundingMode,                   sizeof(roundingMode),                                        "roundingMode",                   "uint8");
    saveStateValue(&calcMode,                       sizeof(calcMode),                                            "calcMode",                       "uint8");
    saveStateValue(&nextChar,                       sizeof(nextChar),                                            "nextChar",                       "uint8");
    saveStateValue(&alphaCase,                      sizeof(alphaCase),                                           "alphaCase",                      "uint8");
    saveStateValue(&hourGlassIconEnabled,           sizeof(hourGlassIconEnabled),                                "hourGlassIconEnabled",           "bool");
    saveStateValue(&watchIconEnabled,               sizeof(watchIconEnabled),                                    "watchIconEnabled",               "bool");
    saveStateValue(&serialIOIconEnabled,            sizeof(serialIOIconEnabled),                                 "serialIOIconEnabled",            "bool");
    saveStateValue(&printerIconEnabled,             sizeof(printerIconEnabled),                                  "printerIconEnabled",             "bool");
    saveStateValue(&programRunStop,                 sizeof(programRunStop),                                      "programRunStop",                 "uint8");
    saveStateValue(&entryStatus,                    sizeof(entryStatus),                                         "entryStatus",                    "uint8");
    saveStateValue(&cursorEnabled,                  sizeof(cursorEnabled),                                       "cursorEnabled",                  "uint8");

    int8_t cf;
         if(cursorFont == &tinyFont)     cf =  1;
    else if(cursorFont == &standardFont) cf =  2;
    else if(cursorFont == &numericFont)  cf =  3;
    else                                 cf = -1;
    saveStateValue(&cf,                             sizeof(cf),                                                  "cursorFont",                     "int8");

    saveStateValue(&rbr1stDigit,                    sizeof(rbr1stDigit),                                         "rbr1stDigit",                    "bool");
    saveStateValue(&shiftF,                         sizeof(shiftF),                                              "shiftF",                         "bool");
    saveStateValue(&shiftG,                         sizeof(shiftG),                                              "shiftG",                         "bool");
    saveStateValue(&rbrMode,                        sizeof(rbrMode),                                             "rbrMode",                        "uint8");
    saveStateValue(&showContent,                    sizeof(showContent),                                         "showContent",                    "bool");
    saveStateValue(&numScreensNumericFont,          sizeof(numScreensNumericFont),                               "numScreensNumericFont",          "uint8");
    saveStateValue(&numLinesNumericFont,            sizeof(numLinesNumericFont),                                 "numLinesNumericFont",            "uint8");
    saveStateValue(&numScreensStandardFont,         sizeof(numScreensStandardFont),                              "numScreensStandardFont",         "uint8");
    saveStateValue(&numLinesStandardFont,           sizeof(numLinesStandardFont),                                "numLinesStandardFont",           "uint8");
    saveStateValue(&numScreensTinyFont,             sizeof(numScreensTinyFont),                                  "numScreensTinyFont",             "uint8");
    saveStateValue(&numLinesTinyFont,               sizeof(numLinesTinyFont),                                    "numLinesTinyFont",               "uint8");
    saveStateValue(&previousCalcMode,               sizeof(previousCalcMode),                                    "previousCalcMode",               "uint8");
    saveStateValue(&lastErrorCode,                  sizeof(lastErrorCode),                                       "lastErrorCode",                  "uint8");
    saveStateValue(&previousErrorCode,              sizeof(previousErrorCode),                                   "previousErrorCode",              "uint8");
    saveStateValue(&nimNumberPart,                  sizeof(nimNumberPart),                                       "nimNumberPart",                  "uint8");
    saveStateValue(&displayStack,                   sizeof(displayStack),                                        "displayStack",                   "uint8");
    saveStateValue(&hexDigits,                      sizeof(hexDigits),                                           "hexDigits",                      "uint8");
    saveStateValue(&errorMessageRegisterLine,       sizeof(errorMessageRegisterLine),                            "errorMessageRegisterLine",       "int16");
    saveStateValue(&shortIntegerMask,               sizeof(shortIntegerMask),                                    "shortIntegerMask",               "uint64");
    saveStateValue(&shortIntegerSignBit,            sizeof(shortIntegerSignBit),                                 "shortIntegerSignBit",            "uint64");
    saveStateValue(&temporaryInformation,           sizeof(temporaryInformation),                                "temporaryInformation",           "uint8");
    saveStateValue(&glyphNotFound,                  sizeof(glyphNotFound),                                       "glyphNotFound",                  "hexDump");
    saveStateValue(&funcOK,                         sizeof(funcOK),                                              "funcOK",                         "bool");
    saveStateValue(&screenChange,                   sizeof(screenChange),                                        "screenChange",                   "bool");
    saveStateValue(&exponentSignLocation,           sizeof(exponentSignLocation),                                "exponentSignLocation",           "int16");
    saveStateValue(&denominatorLocation,            sizeof(denominatorLocation),                                 "denominatorLocation",            "int16");
    saveStateValue(&imaginaryExponentSignLocation,  sizeof(imaginaryExponentSignLocation),                       "imaginaryExponentSignLocation",  "int16");
    saveStateValue(&imaginaryMantissaSignLocation,  sizeof(imaginaryMantissaSignLocation),                       "imaginaryMantissaSignLocation",  "int16");
    saveStateValue(&lineTWidth,                     sizeof(lineTWidth),                                          "lineTWidth",                     "int16");
    saveStateValue(&lastIntegerBase,                sizeof(lastIntegerBase),                                     "lastIntegerBase",                "uint32");
    saveStateValue(&c47MemInBlocks,                 sizeof(c47MemInBlocks),                                      "c47MemInBlocks",                 "uint64");
    saveStateValue(&gmpMemInBytes,                  sizeof(gmpMemInBytes),                                       "gmpMemInBytes",                  "uint64");
    saveStateValue(&catalog,                        sizeof(catalog),                                             "catalog",                        "int16");
    saveStateValue(&lastCatalogPosition,            sizeof(lastCatalogPosition),                                 "lastCatalogPosition",            "int16");
    saveStateValue(displayValueX,                   sizeof(displayValueX),                                       "displayValueX",                  "hexDump");
    saveStateValue(&pcg32_global,                   sizeof(pcg32_global),                                        "pcg32_global",                   "hexDump");
    saveStateValue(&exponentLimit,                  sizeof(exponentLimit),                                       "exponentLimit",                  "int16");
    saveStateValue(&exponentHideLimit,              sizeof(exponentHideLimit),                                   "exponentHideLimit",              "int16");
    saveStateValue(&keyActionProcessed,             sizeof(keyActionProcessed),                                  "keyActionProcessed",             "bool");
    saveStateValue(&systemFlags0,                   sizeof(systemFlags0),                                        "systemFlags",                    "uint64");
    saveStateValue(&systemFlags1,                   sizeof(systemFlags1),                                        "systemFlags1",                   "uint64");
    saveStateValue(&savedSystemFlags0,              sizeof(savedSystemFlags0),                                   "savedSystemFlags",               "uint64");
    saveStateValue(&savedSystemFlags1,              sizeof(savedSystemFlags1),                                   "savedSystemFlags1",              "uint64");
    saveStateValue(&thereIsSomethingToUndo,         sizeof(thereIsSomethingToUndo),                              "thereIsSomethingToUndo",         "bool");
    saveStateValue(&freeProgramBytes,               sizeof(freeProgramBytes),                                    "freeProgramBytes",               "uint16");
    saveStateValue(&firstDisplayedLocalStepNumber,  sizeof(firstDisplayedLocalStepNumber),                       "firstDisplayedLocalStepNumber",  "uint16");
    saveStateValue(&numberOfLabels,                 sizeof(numberOfLabels),                                      "numberOfLabels",                 "uint16");
    saveStateValue(&numberOfPrograms,               sizeof(numberOfPrograms),                                    "numberOfPrograms",               "uint16");
    saveStateValue(&currentLocalStepNumber,         sizeof(currentLocalStepNumber),                              "currentLocalStepNumber",         "uint16");
    saveStateValue(&currentProgramNumber,           sizeof(currentProgramNumber),                                "currentProgramNumber",           "uint16");
    saveStateValue(&lastProgramListEnd,             sizeof(lastProgramListEnd),                                  "lastProgramListEnd",             "bool");
    saveStateValue(&programListEnd,                 sizeof(programListEnd),                                      "programListEnd",                 "bool");
    saveStateValue(&allSubroutineLevels,            sizeof(allSubroutineLevels),                                 "allSubroutineLevels",            "uint32");
    saveStateValue(&pemCursorIsZerothStep,          sizeof(pemCursorIsZerothStep),                               "pemCursorIsZerothStep",          "bool");
    saveStateValue(&skippedStackLines,              sizeof(skippedStackLines),                                   "skippedStackLines",              "bool");
    saveStateValue(&iterations,                     sizeof(iterations),                                          "iterations",                     "bool");
    saveStateValue(&numberOfTamMenusToPop,          sizeof(numberOfTamMenusToPop),                               "numberOfTamMenusToPop",          "int16");
    saveStateValue(&lrSelection,                    sizeof(lrSelection),                                         "lrSelection",                    "uint16");
    saveStateValue(&lrSelectionUndo,                sizeof(lrSelectionUndo),                                     "lrSelectionUndo",                "uint16");
    saveStateValue(&lrChosen,                       sizeof(lrChosen),                                            "lrChosen",                       "uint16");
    saveStateValue(&lrChosenUndo,                   sizeof(lrChosenUndo),                                        "lrChosenUndo",                   "uint16");
    saveStateValue(&lastPlotMode,                   sizeof(lastPlotMode),                                        "lastPlotMode",                   "uint16");
    saveStateValue(&plotSelection,                  sizeof(plotSelection),                                       "plotSelection",                  "uint16");
    saveStateValue(&graph_dx,                       sizeof(graph_dx),                                            "graph_dx",                       "float");
    saveStateValue(&graph_dy,                       sizeof(graph_dy),                                            "graph_dy",                       "float");
    saveStateValue(&roundedTicks,                   sizeof(roundedTicks),                                        "roundedTicks",                   "bool");
    saveStateValue(&PLOT_INTG,                      sizeof(PLOT_INTG),                                           "PLOT_INTG",                      "bool");
    saveStateValue(&PLOT_DIFF,                      sizeof(PLOT_DIFF),                                           "PLOT_DIFF",                      "bool");
    saveStateValue(&PLOT_RMS,                       sizeof(PLOT_RMS),                                            "PLOT_RMS",                       "bool");
    saveStateValue(&PLOT_SHADE,                     sizeof(PLOT_SHADE),                                          "PLOT_SHADE",                     "bool");
    saveStateValue(&PLOT_AXIS,                      sizeof(PLOT_AXIS),                                           "PLOT_AXIS",                      "bool");
    saveStateValue(&PLOT_ZMY,                       sizeof(PLOT_ZMY),                                            "PLOT_ZMY",                       "int8");
    saveStateValue(&PLOT_ZOOM,                      sizeof(PLOT_ZOOM),                                           "PLOT_ZOOM",                      "uint8");
    saveStateValue(&plotmode,                       sizeof(plotmode),                                            "plotmode",                       "int8");
    saveStateValue(&tick_int_x,                     sizeof(tick_int_x),                                          "tick_int_x",                     "float");
    saveStateValue(&tick_int_y,                     sizeof(tick_int_y),                                          "tick_int_y",                     "float");
    saveStateValue(&x_min,                          sizeof(x_min),                                               "x_min",                          "float");
    saveStateValue(&x_max,                          sizeof(x_max),                                               "x_max",                          "float");
    saveStateValue(&y_min,                          sizeof(y_min),                                               "y_min",                          "float");
    saveStateValue(&y_max,                          sizeof(y_max),                                               "y_max",                          "float");
    saveStateValue(&xzero,                          sizeof(xzero),                                               "xzero",                          "uint32");
    saveStateValue(&yzero,                          sizeof(yzero),                                               "yzero",                          "uint32");
    saveStateValue(&regStatsXY,                     sizeof(regStatsXY),                                          "regStatsXY",                     "int16");
    saveStateValue(&matrixIndex,                    sizeof(matrixIndex),                                         "matrixIndex",                    "uint16");
    saveStateValue(&currentViewRegister,            sizeof(currentViewRegister),                                 "currentViewRegister",            "uint16");
    saveStateValue(&currentSolverStatus,            sizeof(currentSolverStatus),                                 "currentSolverStatus",            "uint16");
    saveStateValue(&currentSolverProgram,           sizeof(currentSolverProgram),                                "currentSolverProgram",           "uint16");
    saveStateValue(&currentSolverVariable,          sizeof(currentSolverVariable),                               "currentSolverVariable",          "uint16");
    saveStateValue(&numberOfFormulae,               sizeof(numberOfFormulae),                                    "numberOfFormulae",               "uint16");
    saveStateValue(&currentFormula,                 sizeof(currentFormula),                                      "currentFormula",                 "uint16");
    saveStateValue(&numberOfUserMenus,              sizeof(numberOfUserMenus),                                   "numberOfUserMenus",              "uint16");
    saveStateValue(&currentUserMenu,                sizeof(currentUserMenu),                                     "currentUserMenu",                "uint16");
    saveStateValue(&userKeyLabelSize,               sizeof(userKeyLabelSize),                                    "userKeyLabelSize",               "uint16");
    saveStateValue(&timerCraAndDeciseconds,         sizeof(timerCraAndDeciseconds),                              "timerCraAndDeciseconds",         "uint8");
    saveStateValue(&timerValue,                     sizeof(timerValue),                                          "timerValue",                     "uint32");
    saveStateValue(&timerTotalTime,                 sizeof(timerTotalTime),                                      "timerTotalTime",                 "uint32");
    saveStateValue(&currentInputVariable,           sizeof(currentInputVariable),                                "currentInputVariable",           "uint16");
    saveStateValue(&SAVED_SIGMA_LASTX,              sizeof(SAVED_SIGMA_LASTX),                                   "SAVED_SIGMA_LASTX",              "real");
    saveStateValue(&SAVED_SIGMA_LASTY,              sizeof(SAVED_SIGMA_LASTY),                                   "SAVED_SIGMA_LASTY",              "real");
    saveStateValue(&SAVED_SIGMA_lastAddRem,         sizeof(SAVED_SIGMA_lastAddRem),                              "SAVED_SIGMA_lastAddRem",         "int8");
    saveStateValue(&currentMvarLabel,               sizeof(currentMvarLabel),                                    "currentMvarLabel",               "uint16");
    graphVariabl1 = INVALID_VARIABLE;
    saveStateValue(&graphVariabl1,                  sizeof(graphVariabl1),                                       "graphVariabl1",                  "int16");
    saveStateValue(&plotStatMx,                     sizeof(plotStatMx),                                          "plotStatMx",                     "hexDump");
    saveStateValue(&drawHistogram,                  sizeof(drawHistogram),                                       "drawHistogram",                  "uint8");
    saveStateValue(&statMx,                         sizeof(statMx),                                              "statMx",                         "hexDump");
    saveStateValue(&lrSelectionHistobackup,         sizeof(lrSelectionHistobackup),                              "lrSelectionHistobackup",         "uint16");
    saveStateValue(&lrChosenHistobackup,            sizeof(lrChosenHistobackup),                                 "lrChosenHistobackup",            "uint16");
    saveStateValue(&loBinR,                         sizeof(loBinR),                                              "loBinR",                         "real34");
    saveStateValue(&nBins,                          sizeof(nBins),                                               "nBins",                          "real34");
    saveStateValue(&hiBinR,                         sizeof(hiBinR),                                              "hiBinR",                         "real34");
    saveStateValue(&histElementXorY,                sizeof(histElementXorY),                                     "histElementXorY",                "int16");
    saveStateValue(&screenUpdatingMode,             sizeof(screenUpdatingMode),                                  "screenUpdatingMode",             "uint8");
    //save and restore screenData is not mandatory
    //saveStateValue(screenData,                      0,                                                           "screenData",                     "screenData");
    saveStateValue(&Norm_Key_00.func,               sizeof(Norm_Key_00.func),                                    "Norm_Key_00.func",               "int16");
    saveStateValue(&Norm_Key_00.funcParam,          sizeof(Norm_Key_00.funcParam),                               "Norm_Key_00.funcParam",          "hexDump");
    saveStateValue(&Norm_Key_00.used,               sizeof(Norm_Key_00.used),                                    "Norm_Key_00.used",               "bool");
    saveStateValue(&Input_Default,                  sizeof(Input_Default),                                       "Input_Default",                  "uint8");
    saveStateValue(&T_cursorPos,                    sizeof(T_cursorPos),                                         "T_cursorPos",                    "int16");   //JM ^^
    saveStateValue(&multiEdLines,                   sizeof(multiEdLines),                                        "multiEdLines",                   "uint8");   //JM ^^
    saveStateValue(&current_cursor_x,               sizeof(current_cursor_x  ),                                   "current_cursor_x",              "uint16");   //JM ^^
    saveStateValue(&current_cursor_y,               sizeof(current_cursor_y  ),                                   "current_cursor_y",              "uint16");   //JM ^^
    saveStateValue(&xMultiLineEdOffset,             sizeof(xMultiLineEdOffset),                                   "xMultiLineEdOffset",            "uint8");   //JM ^^
    saveStateValue(&yMultiLineEdOffset,             sizeof(yMultiLineEdOffset),                                   "yMultiLineEdOffset",            "uint8");   //JM ^^
    saveStateValue(&showRegis,                      sizeof(showRegis),                                           "showRegis",                      "int16");   //JM ^^
    saveStateValue(&overrideShowBottomLine,         sizeof(overrideShowBottomLine),                              "overrideShowBottomLine",         "uint8");   //JM ^^
    saveStateValue(&displayStackSHOIDISP,           sizeof(displayStackSHOIDISP),                                "displayStackSHOIDISP",           "uint8");   //JM ^^
    saveStateValue(&ListXYposition,                 sizeof(ListXYposition),                                      "ListXYposition",                 "int16");   //JM ^^
    saveStateValue(&DRG_Cycling,                    sizeof(DRG_Cycling),                                         "DRG_Cycling",                    "uint8");   //JM
    saveStateValue(&lastFlgScr,                     sizeof(lastFlgScr),                                          "lastFlgScr",                     "uint8");   //C47 JM
    saveStateValue(&displayAIMbufferoffset,         sizeof(displayAIMbufferoffset),                              "displayAIMbufferoffset",         "int16");   //C47 JM
    saveStateValue(&bcdDisplaySign,                 sizeof(bcdDisplaySign),                                      "bcdDisplaySign",                 "uint8");   //C47 JM
    saveStateValue(&DM_Cycling,                     sizeof(DM_Cycling),                                          "DM_Cycling",                     "uint8");   //JM
    saveStateValue(&LongPressM,                     sizeof(LongPressM),                                          "LongPressM",                     "uint8");   //JM
    saveStateValue(&LongPressF,                     sizeof(LongPressF),                                          "LongPressF",                     "uint8");   //JM
    saveStateValue(&currentAsnScr,                  sizeof(currentAsnScr),                                       "currentAsnScr",                  "uint8");   //JM
    saveStateValue(&gapItemLeft,                    sizeof(gapItemLeft),                                         "gapItemLeft",                    "uint16");  //JM
    saveStateValue(&gapItemRight,                   sizeof(gapItemRight),                                        "gapItemRight",                   "uint16");  //JM
    saveStateValue(&gapItemRadix,                   sizeof(gapItemRadix),                                        "gapItemRadix",                   "uint16");  //JM
    saveStateValue(&lastCenturyHighUsed,            sizeof(lastCenturyHighUsed),                                 "lastCenturyHighUsed",            "uint16");  //JM
    saveStateValue(&grpGroupingLeft,                sizeof(grpGroupingLeft),                                     "grpGroupingLeft",                "uint8");   //JM
    saveStateValue(&grpGroupingGr1LeftOverflow,     sizeof(grpGroupingGr1LeftOverflow),                          "grpGroupingGr1LeftOverflow",     "uint8");   //JM
    saveStateValue(&grpGroupingGr1Left,             sizeof(grpGroupingGr1Left),                                  "grpGroupingGr1Left",             "uint8");   //JM
    saveStateValue(&grpGroupingRight,               sizeof(grpGroupingRight),                                    "grpGroupingRight",               "uint8");   //JM
    saveStateValue(&firstDayOfWeek,                 sizeof(firstDayOfWeek),                                      "firstDayOfWeek",                 "uint8");
    saveStateValue(&firstWeekOfYearDay,             sizeof(firstWeekOfYearDay),                                  "firstWeekOfYearDay",             "uint8");
    saveStateValue(&dispBase,                       sizeof(dispBase),                                            "dispBase",                       "uint8");   //JM
    saveStateValue(&calcModel,                      sizeof(calcModel),                                           "calcModel",                       "uint8");   //JM

    ramPtr = TO_C47MEMPTR(allNamedVariables);
    saveStateValue(&ramPtr,                         sizeof(ramPtr),                                              "allNamedVariables",              "c47Ptr");

    ramPtr = TO_C47MEMPTR(allFormulae);
    saveStateValue(&ramPtr,                         sizeof(ramPtr),                                              "allFormulae",                    "c47Ptr");

    ramPtr = TO_C47MEMPTR(userMenus);
    saveStateValue(&ramPtr,                         sizeof(ramPtr),                                              "userMenus",                      "c47Ptr");

    ramPtr = TO_C47MEMPTR(userKeyLabel);
    saveStateValue(&ramPtr,                         sizeof(ramPtr),                                              "userKeyLabel",                   "c47Ptr");

    ramPtr = TO_C47MEMPTR(statisticalSumsPointer);
    saveStateValue(&ramPtr,                         sizeof(ramPtr),                                              "statisticalSumsPointer",         "c47Ptr");

    ramPtr = TO_C47MEMPTR(savedStatisticalSumsPointer);
    saveStateValue(&ramPtr,                         sizeof(ramPtr),                                              "savedStatisticalSumsPointer",    "c47Ptr");

    ramPtr = TO_C47MEMPTR(labelList);
    saveStateValue(&ramPtr,                         sizeof(ramPtr),                                              "labelList",                      "c47Ptr");

    ramPtr = TO_C47MEMPTR(programList);
    saveStateValue(&ramPtr,                         sizeof(ramPtr),                                              "programList",                    "c47Ptr");

    ramPtr = TO_C47MEMPTR(currentSubroutineLevelData);
    saveStateValue(&ramPtr,                         sizeof(ramPtr),                                              "currentSubroutineLevelData",     "c47Ptr");

    ramPtr = TO_C47MEMPTR(currentLocalFlags);
    saveStateValue(&ramPtr,                         sizeof(ramPtr),                                              "currentLocalFlags",              "c47Ptr");

    ramPtr = TO_C47MEMPTR(currentLocalRegisters);
    saveStateValue(&ramPtr,                         sizeof(ramPtr),                                              "currentLocalRegisters",          "c47Ptr");

    ramPtr = TO_C47MEMPTR(beginOfProgramMemory);
    saveStateValue(&ramPtr,                         sizeof(ramPtr),                                              "beginOfProgramMemory",           "c47Ptr"); // beginOfProgramMemory pointer to block

    ramPtr = (uint32_t)((void *)beginOfProgramMemory - TO_PCMEMPTR(TO_C47MEMPTR(beginOfProgramMemory)));
    saveStateValue(&ramPtr,                         sizeof(ramPtr),                                              "beginOfProgramMemoryOffset",     "uint32"); // beginOfProgramMemory offset within block

    ramPtr = TO_C47MEMPTR(firstFreeProgramByte);
    saveStateValue(&ramPtr,                         sizeof(ramPtr),                                              "firstFreeProgramByte",           "c47Ptr"); // firstFreeProgramByte pointer to block

    ramPtr = (uint32_t)((void *)firstFreeProgramByte - TO_PCMEMPTR(TO_C47MEMPTR(firstFreeProgramByte)));
    saveStateValue(&ramPtr,                         sizeof(ramPtr),                                              "firstFreeProgramByteOffset",     "uint32"); // firstFreeProgramByte offset within block

    ramPtr = TO_C47MEMPTR(firstDisplayedStep);
    saveStateValue(&ramPtr,                         sizeof(ramPtr),                                              "firstDisplayedStep",             "c47Ptr"); // firstDisplayedStep pointer to block

    ramPtr = (uint32_t)((void *)firstDisplayedStep - TO_PCMEMPTR(TO_C47MEMPTR(firstDisplayedStep)));
    saveStateValue(&ramPtr,                         sizeof(ramPtr),                                              "firstDisplayedStepOffset",       "uint32"); // firstDisplayedStep offset within block

    ramPtr = TO_C47MEMPTR(currentStep);
    saveStateValue(&ramPtr,                         sizeof(ramPtr),                                              "currentStep",                    "c47Ptr"); // currentStep pointer to block

    ramPtr = (uint32_t)((void *)currentStep - TO_PCMEMPTR(TO_C47MEMPTR(currentStep)));
    saveStateValue(&ramPtr,                         sizeof(ramPtr),                                              "currentStepOffset",              "uint32"); // currentStep offset within block

    saveStateValue(ram,                             TO_BYTES(RAM_SIZE_IN_BLOCKS),                                "ram",                            "hexDump");

    // If you create a new parameter, proceed as following:
    //saveStateValue(&newParam,                       sizeof(newParam),                                            "newParam",                       "parameterType");

    ioFileClose();
    printf("End of calc's backup\n");
  }


  static void restoreStateValue(const void *buffer, uint32_t size, const char *valueName, const char *valueType) {
    char value[200], *typePtr, *valuePtr;

    strcpy(value, valueName);
    strcat(value, ":");
    paramCurrent = paramHead;
    while(paramCurrent) {
      if(!strncmp(paramCurrent->param, value, strlen(value))) {
        break;
      }
      paramCurrent = paramCurrent->next;
    }

    if(paramCurrent == NULL) {
      printf("Parameter %s of type %s not found in file %s\n", valueName, valueType, backupFileName);
      printf("Using default value for %s\n", valueName);
      return;
    }

    if((typePtr = strchr(paramCurrent->param, ':')) == NULL) {
      printf("Missing colon (:) after parameter name %s\n", paramCurrent->param);
      printf("Using default value for %s\n", valueName);
      return;
    }
    typePtr++;

    if((valuePtr = strchr(typePtr, ':')) == NULL) {
      printf("Missing colon (:) after parameter type %s\n", paramCurrent->param);
      printf("Using default value for %s\n", valueName);
      return;
    }
    valuePtr++;

    char *p = typePtr;
    int i = 0;
    while(*p != ':') {
      value[i++] = *p++;
    }
    value[i] = 0;

    if(strcmp(valueType, value)) {
      printf("Expected type for parameter %s is %s but %s found\n", valueName, valueType, value);
      return;
    }

    if(!strcmp(valueType, "int64")) {
      *(int64_t *)buffer = stringToInt64(valuePtr);
    }

    else if(!strcmp(valueType, "uint64")) {
      *(uint64_t *)buffer = stringToUint64(valuePtr);
    }

    else if(!strcmp(valueType, "int32")) {
      *(int32_t *)buffer = stringToInt32(valuePtr);
    }

    else if(!strcmp(valueType, "uint32")) {
      *(uint32_t *)buffer = stringToUint32(valuePtr);
    }

    else if(!strcmp(valueType, "int16")) {
      *(int16_t *)buffer = stringToInt16(valuePtr);
    }

    else if(!strcmp(valueType, "uint16")) {
      *(uint16_t *)buffer = stringToUint16(valuePtr);
    }

    else if(!strcmp(valueType, "int8")) {
      *(int8_t *)buffer = stringToInt8(valuePtr);
    }

    else if(!strcmp(valueType, "uint8")) {
      *(uint8_t *)buffer = stringToUint8(valuePtr);
    }

    else if(!strcmp(valueType, "float")) {
      *(float *)buffer = atof(valuePtr);
    }

    else if(!strcmp(valueType, "double")) {
      *(double *)buffer = atof(valuePtr);
    }

    else if(!strcmp(valueType, "real")) {
      stringToReal(valuePtr, (real_t *)buffer, &ctxtReal39);
    }

    else if(!strcmp(valueType, "real34")) {
      stringToReal34(valuePtr, (real34_t *)buffer);
    }

    else if(!strcmp(valueType, "bool")) {
      *(bool_t *)buffer = stringToInt8(valuePtr);
      if(*(bool_t *)buffer != 0) {
        *(bool_t *)buffer = true;
      }
    }

    else if(!strcmp(valueType, "c47Ptr")) {
      *(uint32_t *)buffer = stringToUint32(valuePtr);
    }

    else if(!strcmp(valueType, "hexDump")) {
      uint32_t numberOfBytes = stringToUint32(valuePtr);
      uint8_t hi, lo, *buf = (uint8_t *)buffer;
      uint8_t *v;
      for(uint32_t count=0; count < numberOfBytes; count++, buf++) {
        if(count % 32 == 0) {
          paramCurrent = paramCurrent->next;
          v = (uint8_t *)paramCurrent->param + 7;
        }

        hi = *v - (*v <= '9' ? '0' : 'a' - 10);
        v++;
        lo = *v - (*v <= '9' ? '0' : 'a' - 10);
        v += 2;
        *buf = (hi << 4) | lo;
      }
    }

    //save and restore screenData is not mandatory
    //else if(!strcmp(valueType, "screenData")) {
    //  uint8_t *ls = (uint8_t *)buffer;
    //  for(int y = 0; y < SCREEN_HEIGHT; ++y) {
    //    paramCurrent = paramCurrent->next;
    //    for(int x = 0; x < SCREEN_WIDTH; ++x) {
    //      *ls <<= 1;
    //      if(paramCurrent->param[x] != ' ') {
    //        *ls |= 1;
    //      }
    //      if((x % 8) == 7) {
    //        ls++;
    //      }
    //    }
    //  }
    //}

    else {
      printf("ERROR: valueType %s unknown in function restoreStateValue!" LINEBREAK, valueType);
    }
  }

  void restoreCalc(void) {
    printf("RestoreCalc\n");
    uint32_t ramSizeInBlocks = 0, ramPtr = 0, backupVersion = 0;
    int ret;
    //save and restore screenData is not mandatory
    //uint8_t *loadedScreen = malloc(SCREEN_WIDTH * SCREEN_HEIGHT / 8);
    char oneParam[200];

    doFnReset(CONFIRMED, loadAutoSav);
    ret = ioFileOpen(ioPathBackup, ioModeRead);

    if(ret != FILE_OK ) {
      if(ret == FILE_CANCEL ) {
        return;
      }
      else {
        printf("Cannot restore calc's memory from file %s! Performing RESET\n", backupFileName);
        refreshScreen(91);
        return;
      }
    }

    // Reading all the configuration parameters
    readLine(oneParam);
    paramHead = malloc(sizeof(cfgFileParam_t));
    paramCurrent = paramHead;
    paramCurrent->param = malloc(strlen(oneParam) + 1);
    strcpy(paramCurrent->param, oneParam);
    paramCurrent->next = NULL;
    readLine(oneParam);
    while(!ioEof()) {
      paramCurrent->next = malloc(sizeof(cfgFileParam_t));
      paramCurrent = paramCurrent->next;
      paramCurrent->param = malloc(strlen(oneParam) + 1);
      strcpy(paramCurrent->param, oneParam);
      paramCurrent->next = NULL;
      readLine(oneParam);
    }
    ioFileClose();

    cachedDynamicMenu = 0;
    //configCommon(CFG_DFLT);
    restoreStateValue(&backupVersion,                  sizeof(backupVersion),                                       "backupVersion",                  "uint32");
    restoreStateValue(&ramSizeInBlocks,                sizeof(ramSizeInBlocks),                                     "ramSizeInBlocks",                "uint32");
    if(ramSizeInBlocks != RAM_SIZE_IN_BLOCKS) {
      refreshScreen(92);
      printf("Cannot restore calc's memory from file %s! File %s is from incompatible RAM size.\n", backupFileName, backupFileName);
      printf("       Backup file      Program\n");
      printf("ramSize blocks %6u           %6d\n", ramSizeInBlocks, RAM_SIZE_IN_BLOCKS);
      printf("ramSize bytes  %6u           %6d\n", TO_BYTES(ramSizeInBlocks), TO_BYTES(RAM_SIZE_IN_BLOCKS));
      return;
    }
    else if(backupVersion == 0 || backupVersion < 1011) {
      refreshScreen(92);
      printf("\n");
      char ss[150];
      sprintf(ss, "Cannot restore calc's memory from file %s! File %s has a too low version number.", backupFileName, backupFileName);
      userAbort(ss);
      userAbort("It is proposed that you save a state file from a prior simulator version and import said state file into this version.\n");
      return;
    }

    printf("Begin of calc's restoration, backup version:%i\n", backupVersion);

    // The order in which parameters are restored doesn't matter
    // When a parameter is removed, simply remove the corresponding saveStateValue(...) and restoreStateValue(...) lines.
    restoreStateValue(ram,                             TO_BYTES(RAM_SIZE_IN_BLOCKS),                                "ram",                            "hexDump");
    restoreStateValue(&numberOfFreeMemoryRegions,      sizeof(numberOfFreeMemoryRegions),                           "numberOfFreeMemoryRegions",      "int32");
    restoreStateValue(freeMemoryRegions,               sizeof(freeMemoryRegion_t) * numberOfFreeMemoryRegions,      "freeMemoryRegions",              "hexDump");
    restoreStateValue(&numberOfAllocatedMemoryRegions, sizeof(numberOfAllocatedMemoryRegions),                      "numberOfAllocatedMemoryRegions", "int32");
    restoreStateValue(allocatedMemoryRegions,          sizeof(freeMemoryRegion_t) * numberOfAllocatedMemoryRegions, "allocatedMemoryRegions",         "hexDump");

    restoreStateValue(&ramPtr,                         sizeof(ramPtr),                                              "allNamedVariables",              "c47Ptr");
    allNamedVariables = TO_PCMEMPTR(ramPtr);

    restoreStateValue(&ramPtr,                         sizeof(ramPtr),                                              "allFormulae",                    "c47Ptr");
    allFormulae = TO_PCMEMPTR(ramPtr);

    restoreStateValue(&ramPtr,                         sizeof(ramPtr),                                              "userMenus",                      "c47Ptr");
    userMenus = TO_PCMEMPTR(ramPtr);

    restoreStateValue(&ramPtr,                         sizeof(ramPtr),                                              "userKeyLabel",                   "c47Ptr");
    userKeyLabel = TO_PCMEMPTR(ramPtr);

    restoreStateValue(&ramPtr,                         sizeof(ramPtr),                                              "statisticalSumsPointer",         "c47Ptr");
    statisticalSumsPointer = TO_PCMEMPTR(ramPtr);

    restoreStateValue(&ramPtr,                         sizeof(ramPtr),                                              "savedStatisticalSumsPointer",    "c47Ptr");
    savedStatisticalSumsPointer = TO_PCMEMPTR(ramPtr);

    restoreStateValue(&ramPtr,                         sizeof(ramPtr),                                              "labelList",                      "c47Ptr");
    labelList = TO_PCMEMPTR(ramPtr);

    restoreStateValue(&ramPtr,                         sizeof(ramPtr),                                              "programList",                    "c47Ptr");
    programList = TO_PCMEMPTR(ramPtr);

    restoreStateValue(&ramPtr,                         sizeof(ramPtr),                                              "currentSubroutineLevelData",     "c47Ptr");
    currentSubroutineLevelData = TO_PCMEMPTR(ramPtr);

    restoreStateValue(&ramPtr,                         sizeof(ramPtr),                                              "currentLocalFlags",              "c47Ptr");
    currentLocalFlags = TO_PCMEMPTR(ramPtr);

    restoreStateValue(&ramPtr,                         sizeof(ramPtr),                                              "currentLocalRegisters",          "c47Ptr");
    currentLocalRegisters = TO_PCMEMPTR(ramPtr);

    restoreStateValue(&ramPtr,                         sizeof(ramPtr),                                              "beginOfProgramMemory",           "c47Ptr"); // beginOfProgramMemory pointer to block
    beginOfProgramMemory = TO_PCMEMPTR(ramPtr);

    restoreStateValue(&ramPtr,                         sizeof(ramPtr),                                              "beginOfProgramMemoryOffset",     "uint32"); // beginOfProgramMemory offset within block
    beginOfProgramMemory += ramPtr;

    restoreStateValue(&ramPtr,                         sizeof(ramPtr),                                              "firstFreeProgramByte",           "c47Ptr"); // firstFreeProgramByte pointer to block
    firstFreeProgramByte = TO_PCMEMPTR(ramPtr);

    restoreStateValue(&ramPtr,                         sizeof(ramPtr),                                              "firstFreeProgramByteOffset",     "uint32"); // firstFreeProgramByte offset within block
    firstFreeProgramByte += ramPtr;

    restoreStateValue(&ramPtr,                         sizeof(ramPtr),                                              "firstDisplayedStep",             "c47Ptr"); // firstDisplayedStep pointer to block
    firstDisplayedStep = TO_PCMEMPTR(ramPtr);

    restoreStateValue(&ramPtr,                         sizeof(ramPtr),                                              "firstDisplayedStepOffset",       "uint32"); // firstDisplayedStep offset within block
    firstDisplayedStep += ramPtr;

    restoreStateValue(&ramPtr,                         sizeof(ramPtr),                                              "currentStep",                    "c47Ptr"); // currentStep pointer to block
    currentStep = TO_PCMEMPTR(ramPtr);

    restoreStateValue(&ramPtr,                         sizeof(ramPtr),                                              "currentStepOffset",              "uint32"); // currentStep offset within block
    currentStep += ramPtr;

    restoreStateValue(globalFlags,                     sizeof(globalFlags),                                         "globalFlags",                    "hexDump");
    restoreStateValue(errorMessage,                    ERROR_MESSAGE_LENGTH,                                        "errorMessage",                   "hexDump");
    restoreStateValue(aimBuffer,                       AIM_BUFFER_LENGTH,                                           "aimBuffer",                      "hexDump");
    restoreStateValue(nimBufferDisplay,                NIM_BUFFER_LENGTH,                                           "nimBufferDisplay",               "hexDump");
    restoreStateValue(tamBuffer,                       TAM_BUFFER_LENGTH,                                           "tamBuffer",                      "hexDump");
    restoreStateValue(asmBuffer,                       sizeof(asmBuffer),                                           "asmBuffer",                      "hexDump");
    restoreStateValue(oldTime,                         sizeof(oldTime),                                             "oldTime",                        "hexDump");
    restoreStateValue(dateTimeString,                  sizeof(dateTimeString),                                      "dateTimeString",                 "hexDump");
    restoreStateValue(softmenuStack,                   sizeof(softmenuStack),                                       "softmenuStack",                  "hexDump");
    restoreStateValue(globalRegister,                  sizeof(registerHeader_t) * NUMBER_OF_GLOBAL_REGISTERS,       "globalRegister",                 "hexDump");
    restoreStateValue(kbd_usr,                         sizeof(kbd_usr),                                             "kbd_usr",                        "hexDump");
    restoreStateValue(userMenuItems,                   sizeof(userMenuItems),                                       "userMenuItems",                  "hexDump");
    restoreStateValue(userAlphaItems,                  sizeof(userAlphaItems),                                      "userAlphaItems",                 "hexDump");
    restoreStateValue(lastTemp,                        sizeof(lastTemp),                                            "lastTemp",                       "hexDump");
    lastI = 0;
    lastJ = 0;
    restoreStateValue(&lastI,                          sizeof(lastI),                                               "lastI",                          "int16");
    restoreStateValue(&lastJ,                          sizeof(lastJ),                                               "lastJ",                          "int16");
    restoreStateValue(&lastFunc,                       sizeof(lastFunc),                                            "lastFunc",                       "int16");
    restoreStateValue(&lastParam,                      sizeof(lastParam),                                           "lastParam",                      "int16");
    restoreStateValue(&tam.mode,                       sizeof(tam.mode),                                            "tam.mode",                       "uint16");
    restoreStateValue(&tam.function,                   sizeof(tam.function),                                        "tam.function",                   "int16");
    restoreStateValue(&tam.alpha,                      sizeof(tam.alpha),                                           "tam.alpha",                      "bool");
    restoreStateValue(&tam.currentOperation,           sizeof(tam.currentOperation),                                "tam.currentOperation",           "int16");
    restoreStateValue(&tam.dot,                        sizeof(tam.dot),                                             "tam.dot",                        "bool");
    restoreStateValue(&tam.indirect,                   sizeof(tam.indirect),                                        "tam.indirect",                   "bool");
    restoreStateValue(&tam.digitsSoFar,                sizeof(tam.digitsSoFar),                                     "tam.digitsSoFar",                "int16");
    restoreStateValue(&tam.value,                      sizeof(tam.value),                                           "tam.value",                      "int16");
    restoreStateValue(&tam.min,                        sizeof(tam.min),                                             "tam.min",                        "int16");
    restoreStateValue(&tam.max,                        sizeof(tam.max),                                             "tam.max",                        "int16");
    restoreStateValue(&rbrRegister,                    sizeof(rbrRegister),                                         "rbrRegister",                    "int16");
    restoreStateValue(&numberOfNamedVariables,         sizeof(numberOfNamedVariables),                              "numberOfNamedVariables",         "int16");
    restoreStateValue(&xCursor,                        sizeof(xCursor),                                             "xCursor",                        "uint32");
    restoreStateValue(&yCursor,                        sizeof(yCursor),                                             "yCursor",                        "uint32");
    restoreStateValue(&firstGregorianDay,              sizeof(firstGregorianDay),                                   "firstGregorianDay",              "uint32");
    restoreStateValue(&denMax,                         sizeof(denMax),                                              "denMax",                         "uint32");
    restoreStateValue(&lastDenominator,                sizeof(lastDenominator),                                     "lastDenominator",                "uint32");
    restoreStateValue(&currentRegisterBrowserScreen,   sizeof(currentRegisterBrowserScreen),                        "currentRegisterBrowserScreen",   "int16");
    restoreStateValue(&currentFntScr,                  sizeof(currentFntScr),                                       "currentFntScr",                  "uint8");
    restoreStateValue(&currentFlgScr,                  sizeof(currentFlgScr),                                       "currentFlgScr",                  "uint8");
    restoreStateValue(&displayFormat,                  sizeof(displayFormat),                                       "displayFormat",                  "uint8");
    restoreStateValue(&displayFormatDigits,            sizeof(displayFormatDigits),                                 "displayFormatDigits",            "uint8");
    restoreStateValue(&timeDisplayFormatDigits,        sizeof(timeDisplayFormatDigits),                             "timeDisplayFormatDigits",        "uint8");
    restoreStateValue(&shortIntegerWordSize,           sizeof(shortIntegerWordSize),                                "shortIntegerWordSize",           "uint8");
    restoreStateValue(&significantDigits,              sizeof(significantDigits),                                   "significantDigits",              "uint8");
    fractionDigits = 34;
    restoreStateValue(&fractionDigits,                 sizeof(fractionDigits),                                      "fractionDigits",                 "uint8");
    restoreStateValue(&shortIntegerMode,               sizeof(shortIntegerMode),                                    "shortIntegerMode",               "uint8");
    restoreStateValue(&currentAngularMode,             sizeof(currentAngularMode),                                  "currentAngularMode",             "uint32");

    restoreStateValue(&scrLock,                        sizeof(scrLock),                                             "scrLock",                        "uint8");
    scrLock &= 0x03;

    restoreStateValue(&roundingMode,                   sizeof(roundingMode),                                        "roundingMode",                   "uint8");
    restoreStateValue(&calcMode,                       sizeof(calcMode),                                            "calcMode",                       "uint8");
    restoreStateValue(&nextChar,                       sizeof(nextChar),                                            "nextChar",                       "uint8");
    restoreStateValue(&alphaCase,                      sizeof(alphaCase),                                           "alphaCase",                      "uint8");
    restoreStateValue(&hourGlassIconEnabled,           sizeof(hourGlassIconEnabled),                                "hourGlassIconEnabled",           "bool");
    restoreStateValue(&watchIconEnabled,               sizeof(watchIconEnabled),                                    "watchIconEnabled",               "bool");
    restoreStateValue(&serialIOIconEnabled,            sizeof(serialIOIconEnabled),                                 "serialIOIconEnabled",            "bool");
    restoreStateValue(&printerIconEnabled,             sizeof(printerIconEnabled),                                  "printerIconEnabled",             "bool");
    restoreStateValue(&programRunStop,                 sizeof(programRunStop),                                      "programRunStop",                 "uint8");
    restoreStateValue(&entryStatus,                    sizeof(entryStatus),                                         "entryStatus",                    "uint8");
    restoreStateValue(&cursorEnabled,                  sizeof(cursorEnabled),                                       "cursorEnabled",                  "uint8");

    int8_t cf = 0;
    restoreStateValue(&cf,                             sizeof(cf),                                                  "cursorFont",                     "int8");
         if(cf == 1) cursorFont = &tinyFont;
    else if(cf == 2) cursorFont = &standardFont;
    else if(cf == 3) cursorFont = &numericFont;
    else             cursorFont = NULL;

    restoreStateValue(&rbr1stDigit,                    sizeof(rbr1stDigit),                                         "rbr1stDigit",                    "bool");
    restoreStateValue(&shiftF,                         sizeof(shiftF),                                              "shiftF",                         "bool");
    restoreStateValue(&shiftG,                         sizeof(shiftG),                                              "shiftG",                         "bool");
    restoreStateValue(&rbrMode,                        sizeof(rbrMode),                                             "rbrMode",                        "uint8");
    restoreStateValue(&showContent,                    sizeof(showContent),                                         "showContent",                    "bool");
    restoreStateValue(&numScreensNumericFont,          sizeof(numScreensNumericFont),                               "numScreensNumericFont",          "uint8");
    restoreStateValue(&numLinesNumericFont,            sizeof(numLinesNumericFont),                                 "numLinesNumericFont",            "uint8");
    restoreStateValue(&numScreensStandardFont,         sizeof(numScreensStandardFont),                              "numScreensStandardFont",         "uint8");
    restoreStateValue(&numLinesStandardFont,           sizeof(numLinesStandardFont),                                "numLinesStandardFont",           "uint8");
    restoreStateValue(&numScreensTinyFont,             sizeof(numScreensTinyFont),                                  "numScreensTinyFont",             "uint8");
    restoreStateValue(&numLinesTinyFont,               sizeof(numLinesTinyFont),                                    "numLinesTinyFont",               "uint8");
    restoreStateValue(&previousCalcMode,               sizeof(previousCalcMode),                                    "previousCalcMode",               "uint8");
    restoreStateValue(&lastErrorCode,                  sizeof(lastErrorCode),                                       "lastErrorCode",                  "uint8");
    restoreStateValue(&previousErrorCode,              sizeof(previousErrorCode),                                   "previousErrorCode",              "uint8");
    restoreStateValue(&nimNumberPart,                  sizeof(nimNumberPart),                                       "nimNumberPart",                  "uint8");
    restoreStateValue(&displayStack,                   sizeof(displayStack),                                        "displayStack",                   "uint8");
    restoreStateValue(&hexDigits,                      sizeof(hexDigits),                                           "hexDigits",                      "uint8");
    restoreStateValue(&errorMessageRegisterLine,       sizeof(errorMessageRegisterLine),                            "errorMessageRegisterLine",       "int16");
    restoreStateValue(&shortIntegerMask,               sizeof(shortIntegerMask),                                    "shortIntegerMask",               "uint64");
    restoreStateValue(&shortIntegerSignBit,            sizeof(shortIntegerSignBit),                                 "shortIntegerSignBit",            "uint64");
    restoreStateValue(&temporaryInformation,           sizeof(temporaryInformation),                                "temporaryInformation",           "uint8");

    restoreStateValue(&glyphNotFound,                  sizeof(glyphNotFound),                                       "glyphNotFound",                  "hexDump");
    glyphNotFound.data   = malloc(38);
    xcopy(glyphNotFound.data, "\xff\xf8\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\xff\xf8", 38);

    restoreStateValue(&funcOK,                         sizeof(funcOK),                                              "funcOK",                         "bool");
    restoreStateValue(&screenChange,                   sizeof(screenChange),                                        "screenChange",                   "bool");
    restoreStateValue(&exponentSignLocation,           sizeof(exponentSignLocation),                                "exponentSignLocation",           "int16");
    restoreStateValue(&denominatorLocation,            sizeof(denominatorLocation),                                 "denominatorLocation",            "int16");
    restoreStateValue(&imaginaryExponentSignLocation,  sizeof(imaginaryExponentSignLocation),                       "imaginaryExponentSignLocation",  "int16");
    restoreStateValue(&imaginaryMantissaSignLocation,  sizeof(imaginaryMantissaSignLocation),                       "imaginaryMantissaSignLocation",  "int16");
    restoreStateValue(&lineTWidth,                     sizeof(lineTWidth),                                          "lineTWidth",                     "int16");
    restoreStateValue(&lastIntegerBase,                sizeof(lastIntegerBase),                                     "lastIntegerBase",                "uint32");
    restoreStateValue(&c47MemInBlocks,                 sizeof(c47MemInBlocks),                                      "c47MemInBlocks",                 "uint64");
    restoreStateValue(&gmpMemInBytes,                  sizeof(gmpMemInBytes),                                       "gmpMemInBytes",                  "uint64");
    restoreStateValue(&catalog,                        sizeof(catalog),                                             "catalog",                        "int16");
    if(backupVersion < 1012) { // add FNCS_EIM catalog
      restoreStateValue(&lastCatalogPosition,            sizeof(lastCatalogPosition) - 4,                             "lastCatalogPosition",            "int16");
      lastCatalogPosition[22 /* MNU_FNCS_EIM */]  = 0;
    }
    else {
      restoreStateValue(&lastCatalogPosition,            sizeof(lastCatalogPosition),                                 "lastCatalogPosition",            "int16");
    }
    restoreStateValue(displayValueX,                   sizeof(displayValueX),                                       "displayValueX",                  "hexDump");
    restoreStateValue(&pcg32_global,                   sizeof(pcg32_global),                                        "pcg32_global",                   "hexDump");
    restoreStateValue(&exponentLimit,                  sizeof(exponentLimit),                                       "exponentLimit",                  "int16");
    restoreStateValue(&exponentHideLimit,              sizeof(exponentHideLimit),                                   "exponentHideLimit",              "int16");
    restoreStateValue(&keyActionProcessed,             sizeof(keyActionProcessed),                                  "keyActionProcessed",             "bool");
    restoreStateValue(&systemFlags0,                   sizeof(systemFlags0),                                        "systemFlags",                    "uint64");
    systemFlags1 = 0;
    restoreStateValue(&systemFlags1,                   sizeof(systemFlags1),                                        "systemFlags1",                   "uint64");
    restoreStateValue(&savedSystemFlags0,              sizeof(savedSystemFlags0),                                   "savedSystemFlags",               "uint64");
    savedSystemFlags1 = 0;
    restoreStateValue(&savedSystemFlags1,              sizeof(savedSystemFlags1),                                   "savedSystemFlags1",              "uint64");
    restoreStateValue(&thereIsSomethingToUndo,         sizeof(thereIsSomethingToUndo),                              "thereIsSomethingToUndo",         "bool");
    restoreStateValue(&freeProgramBytes,               sizeof(freeProgramBytes),                                    "freeProgramBytes",               "uint16");
    restoreStateValue(&firstDisplayedLocalStepNumber,  sizeof(firstDisplayedLocalStepNumber),                       "firstDisplayedLocalStepNumber",  "uint16");
    restoreStateValue(&numberOfLabels,                 sizeof(numberOfLabels),                                      "numberOfLabels",                 "uint16");
    restoreStateValue(&numberOfPrograms,               sizeof(numberOfPrograms),                                    "numberOfPrograms",               "uint16");
    restoreStateValue(&currentLocalStepNumber,         sizeof(currentLocalStepNumber),                              "currentLocalStepNumber",         "uint16");
    restoreStateValue(&currentProgramNumber,           sizeof(currentProgramNumber),                                "currentProgramNumber",           "uint16");
    restoreStateValue(&lastProgramListEnd,             sizeof(lastProgramListEnd),                                  "lastProgramListEnd",             "bool");
    restoreStateValue(&programListEnd,                 sizeof(programListEnd),                                      "programListEnd",                 "bool");
    restoreStateValue(&allSubroutineLevels,            sizeof(allSubroutineLevels),                                 "allSubroutineLevels",            "uint32");
    restoreStateValue(&pemCursorIsZerothStep,          sizeof(pemCursorIsZerothStep),                               "pemCursorIsZerothStep",          "bool");
    restoreStateValue(&skippedStackLines,              sizeof(skippedStackLines),                                   "skippedStackLines",              "bool");
    restoreStateValue(&iterations,                     sizeof(iterations),                                          "iterations",                     "bool");
    restoreStateValue(&numberOfTamMenusToPop,          sizeof(numberOfTamMenusToPop),                               "numberOfTamMenusToPop",          "int16");
    restoreStateValue(&lrSelection,                    sizeof(lrSelection),                                         "lrSelection",                    "uint16");
    restoreStateValue(&lrSelectionUndo,                sizeof(lrSelectionUndo),                                     "lrSelectionUndo",                "uint16");
    restoreStateValue(&lrChosen,                       sizeof(lrChosen),                                            "lrChosen",                       "uint16");
    restoreStateValue(&lrChosenUndo,                   sizeof(lrChosenUndo),                                        "lrChosenUndo",                   "uint16");
    restoreStateValue(&lastPlotMode,                   sizeof(lastPlotMode),                                        "lastPlotMode",                   "uint16");
    restoreStateValue(&plotSelection,                  sizeof(plotSelection),                                       "plotSelection",                  "uint16");
    restoreStateValue(&graph_dx,                       sizeof(graph_dx),                                            "graph_dx",                       "float");
    restoreStateValue(&graph_dy,                       sizeof(graph_dy),                                            "graph_dy",                       "float");
    restoreStateValue(&roundedTicks,                   sizeof(roundedTicks),                                        "roundedTicks",                   "bool");
    restoreStateValue(&PLOT_INTG,                      sizeof(PLOT_INTG),                                           "PLOT_INTG",                      "bool");
    restoreStateValue(&PLOT_DIFF,                      sizeof(PLOT_DIFF),                                           "PLOT_DIFF",                      "bool");
    restoreStateValue(&PLOT_RMS,                       sizeof(PLOT_RMS),                                            "PLOT_RMS",                       "bool");
    restoreStateValue(&PLOT_SHADE,                     sizeof(PLOT_SHADE),                                          "PLOT_SHADE",                     "bool");
    restoreStateValue(&PLOT_AXIS,                      sizeof(PLOT_AXIS),                                           "PLOT_AXIS",                      "bool");
    restoreStateValue(&PLOT_ZMY,                       sizeof(PLOT_ZMY),                                            "PLOT_ZMY",                       "int8");
    restoreStateValue(&PLOT_ZOOM,                      sizeof(PLOT_ZOOM),                                           "PLOT_ZOOM",                      "uint8");
    restoreStateValue(&plotmode,                       sizeof(plotmode),                                            "plotmode",                       "int8");
    restoreStateValue(&tick_int_x,                     sizeof(tick_int_x),                                          "tick_int_x",                     "float");
    restoreStateValue(&tick_int_y,                     sizeof(tick_int_y),                                          "tick_int_y",                     "float");
    restoreStateValue(&x_min,                          sizeof(x_min),                                               "x_min",                          "float");
    restoreStateValue(&x_max,                          sizeof(x_max),                                               "x_max",                          "float");
    restoreStateValue(&y_min,                          sizeof(y_min),                                               "y_min",                          "float");
    restoreStateValue(&y_max,                          sizeof(y_max),                                               "y_max",                          "float");
    restoreStateValue(&xzero,                          sizeof(xzero),                                               "xzero",                          "uint32");
    restoreStateValue(&yzero,                          sizeof(yzero),                                               "yzero",                          "uint32");
    restoreStateValue(&regStatsXY,                     sizeof(regStatsXY),                                          "regStatsXY",                     "int16");
    restoreStateValue(&matrixIndex,                    sizeof(matrixIndex),                                         "matrixIndex",                    "uint16");
    restoreStateValue(&currentViewRegister,            sizeof(currentViewRegister),                                 "currentViewRegister",            "uint16");
    restoreStateValue(&currentSolverStatus,            sizeof(currentSolverStatus),                                 "currentSolverStatus",            "uint16");
    restoreStateValue(&currentSolverProgram,           sizeof(currentSolverProgram),                                "currentSolverProgram",           "uint16");
    restoreStateValue(&currentSolverVariable,          sizeof(currentSolverVariable),                               "currentSolverVariable",          "uint16");
    restoreStateValue(&numberOfFormulae,               sizeof(numberOfFormulae),                                    "numberOfFormulae",               "uint16");
    restoreStateValue(&currentFormula,                 sizeof(currentFormula),                                      "currentFormula",                 "uint16");
    restoreStateValue(&numberOfUserMenus,              sizeof(numberOfUserMenus),                                   "numberOfUserMenus",              "uint16");
    restoreStateValue(&currentUserMenu,                sizeof(currentUserMenu),                                     "currentUserMenu",                "uint16");
    restoreStateValue(&userKeyLabelSize,               sizeof(userKeyLabelSize),                                    "userKeyLabelSize",               "uint16");
    restoreStateValue(&timerCraAndDeciseconds,         sizeof(timerCraAndDeciseconds),                              "timerCraAndDeciseconds",         "uint8");
    restoreStateValue(&timerValue,                     sizeof(timerValue),                                          "timerValue",                     "uint32");
    restoreStateValue(&timerTotalTime,                 sizeof(timerTotalTime),                                      "timerTotalTime",                 "uint32");
    restoreStateValue(&currentInputVariable,           sizeof(currentInputVariable),                                "currentInputVariable",           "uint16");
    if(backupVersion < 1002 && currentInputVariable == INVALID_VARIABLE_OLD) {currentInputVariable = INVALID_VARIABLE;}
    restoreStateValue(&SAVED_SIGMA_LASTX,              sizeof(SAVED_SIGMA_LASTX),                                   "SAVED_SIGMA_LASTX",              "real");
    restoreStateValue(&SAVED_SIGMA_LASTY,              sizeof(SAVED_SIGMA_LASTY),                                   "SAVED_SIGMA_LASTY",              "real");
    SAVED_SIGMA_lastAddRem = SIGMA_NONE;
    if(backupVersion <= 1004) {
      restoreStateValue(&SAVED_SIGMA_lastAddRem,         sizeof(SAVED_SIGMA_lastAddRem),                              "SAVED_SIGMA_LAc1",               "int8");     //manual correction as the type allocation was wrong here
    }
    restoreStateValue(&SAVED_SIGMA_lastAddRem,         sizeof(SAVED_SIGMA_lastAddRem),                              "SAVED_SIGMA_lastAddRem",         "int8");     //manual correction as the type allocation was wrong here
    if(SAVED_SIGMA_lastAddRem == -1) SAVED_SIGMA_lastAddRem = SIGMA_MINUS;
  //if(SAVED_SIGMA_lastAddRem == 0 ) SAVED_SIGMA_lastAddRem = SIGMA_NONE;
  //if(SAVED_SIGMA_lastAddRem == 1 ) SAVED_SIGMA_lastAddRem = SIGMA_PLUS;
  //if(SAVED_SIGMA_lastAddRem == 2 ) SAVED_SIGMA_lastAddRem = SIGMA_MINUS;
    restoreStateValue(&currentMvarLabel,               sizeof(currentMvarLabel),                                    "currentMvarLabel",               "uint16");
    if(backupVersion < 1002 && currentMvarLabel == INVALID_VARIABLE_OLD) {currentMvarLabel = INVALID_VARIABLE;}
    restoreStateValue(&graphVariabl1,                  sizeof(graphVariabl1),                                       "graphVariabl1",                  "int16");
    if(backupVersion < 1002 && graphVariabl1 == INVALID_VARIABLE_OLD) {graphVariabl1 = INVALID_VARIABLE;}
    restoreStateValue(&plotStatMx,                     sizeof(plotStatMx),                                          "plotStatMx",                     "hexDump");
    restoreStateValue(&drawHistogram,                  sizeof(drawHistogram),                                       "drawHistogram",                  "uint8");
    restoreStateValue(&statMx,                         sizeof(statMx),                                              "statMx",                         "hexDump");
    restoreStateValue(&lrSelectionHistobackup,         sizeof(lrSelectionHistobackup),                              "lrSelectionHistobackup",         "uint16");
    restoreStateValue(&lrChosenHistobackup,            sizeof(lrChosenHistobackup),                                 "lrChosenHistobackup",            "uint16");
    restoreStateValue(&loBinR,                         sizeof(loBinR),                                              "loBinR",                         "real34");
    restoreStateValue(&nBins,                          sizeof(nBins),                                               "nBins",                          "real34");
    restoreStateValue(&hiBinR,                         sizeof(hiBinR),                                              "hiBinR",                         "real34");
    restoreStateValue(&histElementXorY,                sizeof(histElementXorY),                                     "histElementXorY",                "int16");
    restoreStateValue(&screenUpdatingMode,             sizeof(screenUpdatingMode),                                  "screenUpdatingMode",             "uint8");
    //save and restore screenData is not mandatory
    //restoreStateValue(loadedScreen,                    0,                                                           "screenData",                     "screenData");
    uint8_t fgLN = 255;
    restoreStateValue(&fgLN,                           sizeof(fgLN),                                                "fgLN",                           "uint8");
    fgLN = convert001090400T001090500(fgLN,RBX_FGLNOFF);
    if(fgLN == RBX_FGLNOFF) {                                                                                //This section to deal with any old states containing the old FG system
      clearSystemFlag(FLAG_FGLNLIM);
      clearSystemFlag(FLAG_FGLNFUL);
    } else     if(fgLN == RBX_FGLNLIM) {
      setSystemFlag(FLAG_FGLNLIM);
      clearSystemFlag(FLAG_FGLNFUL);
    } else     if(fgLN == RBX_FGLNFUL) {
      clearSystemFlag(FLAG_FGLNLIM);
      setSystemFlag(FLAG_FGLNFUL);
    }
    restoreStateValue(&Norm_Key_00.func,               sizeof(Norm_Key_00.func),                                    "Norm_Key_00.func",               "int16");
    restoreStateValue(&Norm_Key_00.funcParam,          sizeof(Norm_Key_00.funcParam),                               "Norm_Key_00.funcParam",          "hexDump");
    restoreStateValue(&Norm_Key_00.used,               sizeof(Norm_Key_00.used),                                    "Norm_Key_00.used",               "bool");
    restoreStateValue(&Input_Default,                  sizeof(Input_Default),                                       "Input_Default",                  "uint8");
    IrFractionsCurrentStatus = CF_NORMAL;
    restoreStateValue(&T_cursorPos,                    sizeof(T_cursorPos),                                         "T_cursorPos",                    "int16");   //JM ^^
    restoreStateValue(&multiEdLines,                   sizeof(multiEdLines),                                        "multiEdLines",                   "uint8");   //JM ^^
    restoreStateValue(&current_cursor_x,               sizeof(current_cursor_x  ),                                  "current_cursor_x",               "uint16");   //JM ^^
    restoreStateValue(&current_cursor_y,               sizeof(current_cursor_y  ),                                  "current_cursor_y",               "uint16");   //JM ^^
    restoreStateValue(&xMultiLineEdOffset,             sizeof(xMultiLineEdOffset),                                  "xMultiLineEdOffset",             "uint8");   //JM ^^
    restoreStateValue(&yMultiLineEdOffset,             sizeof(yMultiLineEdOffset),                                  "yMultiLineEdOffset",             "uint8");   //JM ^^
    restoreStateValue(&showRegis,                      sizeof(showRegis),                                           "showRegis",                      "int16");   //JM ^^
    restoreStateValue(&overrideShowBottomLine,         sizeof(overrideShowBottomLine),                              "overrideShowBottomLine",         "uint8");   //JM ^^
    restoreStateValue(&displayStackSHOIDISP,           sizeof(displayStackSHOIDISP),                                "displayStackSHOIDISP",           "uint8");   //JM ^^
    restoreStateValue(&ListXYposition,                 sizeof(ListXYposition),                                      "ListXYposition",                 "int16");   //JM ^^
    restoreStateValue(&DRG_Cycling,                    sizeof(DRG_Cycling),                                         "DRG_Cycling",                    "uint8");   //JM
    restoreStateValue(&lastFlgScr,                     sizeof(lastFlgScr),                                          "lastFlgScr",                     "uint8");   //C47 JM
    restoreStateValue(&displayAIMbufferoffset,         sizeof(displayAIMbufferoffset),                              "displayAIMbufferoffset",         "int16");   //C47 JM
    bool_t bcdDisplay = false;
    restoreStateValue(&bcdDisplay,                     sizeof(bcdDisplay),                                          "bcdDisplay",                     "bool");    //C47 JM
    if(bcdDisplay) setSystemFlag(FLAG_BCD); else clearSystemFlag(FLAG_BCD);
    restoreStateValue(&bcdDisplaySign,                 sizeof(bcdDisplaySign),                                      "bcdDisplaySign",                 "uint8");   //C47 JM
    bcdDisplaySign = convert001090400T001090500(bcdDisplaySign,BCDu);
    restoreStateValue(&DM_Cycling,                     sizeof(DM_Cycling),                                          "DM_Cycling",                     "uint8");   //JM
    restoreStateValue(&LongPressM,                     sizeof(LongPressM),                                          "LongPressM",                     "uint8");   //JM
    LongPressM = convert001090400T001090500(LongPressM,RBX_M14);
    restoreStateValue(&LongPressF,                     sizeof(LongPressF),                                          "LongPressF",                     "uint8");   //JM
    LongPressF = convert001090400T001090500(LongPressF,RBX_F14);
    restoreStateValue(&currentAsnScr,                  sizeof(currentAsnScr),                                       "currentAsnScr",                  "uint8");   //JM
    restoreStateValue(&gapItemLeft,                    sizeof(gapItemLeft),                                         "gapItemLeft",                    "uint16");  //JM
    restoreStateValue(&gapItemRight,                   sizeof(gapItemRight),                                        "gapItemRight",                   "uint16");  //JM
    restoreStateValue(&gapItemRadix,                   sizeof(gapItemRadix),                                        "gapItemRadix",                   "uint16");  //JM
    restoreStateValue(&lastCenturyHighUsed,            sizeof(lastCenturyHighUsed),                                 "lastCenturyHighUsed",            "uint16");  //JM
    restoreStateValue(&grpGroupingLeft,                sizeof(grpGroupingLeft),                                     "grpGroupingLeft",                "uint8");   //JM
    restoreStateValue(&grpGroupingGr1LeftOverflow,     sizeof(grpGroupingGr1LeftOverflow),                          "grpGroupingGr1LeftOverflow",     "uint8");   //JM
    restoreStateValue(&grpGroupingGr1Left,             sizeof(grpGroupingGr1Left),                                  "grpGroupingGr1Left",             "uint8");   //JM
    restoreStateValue(&grpGroupingRight,               sizeof(grpGroupingRight),                                    "grpGroupingRight",               "uint8");   //JM
    restoreStateValue(&firstDayOfWeek,                 sizeof(firstDayOfWeek),                                      "firstDayOfWeek",                 "uint8");
    restoreStateValue(&firstWeekOfYearDay,             sizeof(firstWeekOfYearDay),                                  "firstWeekOfYearDay",             "uint8");
    dispBase = 0;
    restoreStateValue(&dispBase,                       sizeof(dispBase),                                            "dispBase",                       "uint8");   //JM
    calcModel = USER_C47;
    restoreStateValue(&calcModel,                      sizeof(calcModel),                                           "calcModel",                      "uint8");   //JM

    if(backupVersion < 1014) {
      setLongPressFg(calcModel, -MNU_HOME);
    }

    // Ensure valid relations between FLAG_FRACT, FLAG_IRFRAC and FLAG_IRFRQ
    if (getSystemFlag(FLAG_FRACT)) {
      setSystemFlag(FLAG_FRACT);
    }
    else if (getSystemFlag(FLAG_IRFRAC)) {
      setSystemFlag(FLAG_IRFRAC);
    }


    // If you create a new parameter, proceed as following:
    //newParam = 42 // default value for newParam if not found in backup.cgf. This is for compatibility with older versions of backup.cfg.
    //restoreStateValue(&newParam,                       sizeof(newParam),                                            "newParam",                       "parameterType");

    bool_t tmp1 = false;
    if(backupVersion < 1003) {
      restoreStateValue(&tmp1,                           sizeof(tmp1),                                                "constantFractions",              "bool");
      printf("Version number of configfile < 1003, transferring FLAG_IRFRQ.");
      if(tmp1) {
        setSystemFlag(FLAG_IRFRQ);
      }
      else {
        clearSystemFlag(FLAG_IRFRQ);
      }
    }
    if(backupVersion < 1003) {
      restoreStateValue(&tmp1,                           sizeof(tmp1),                                                "constantFractionsOn",            "bool");
      printf("Version number of configfile < 1003, transferring FLAG_IRFRAC.");
      if(tmp1) {
        setSystemFlag(FLAG_IRFRAC);
      }
      else {
        clearSystemFlag(FLAG_IRFRAC);
      }
    }
    if(backupVersion < 1003) {
      restoreStateValue(&tmp1,                           sizeof(tmp1),                                                "eRPN",                           "bool");    //JM vv
      printf("Version number of configfile < 1003, transferring eRPN.");
      if(tmp1) {
        setSystemFlag(FLAG_ERPN);
      }
      else {
        clearSystemFlag(FLAG_ERPN);
      }
    }
    if(backupVersion < 1003) {
      restoreStateValue(&tmp1,                           sizeof(tmp1),                                                "jm_LARGELI",                     "bool");
      printf("Version number of configfile < 1003, transferring jm_LARGELI.");
      if(tmp1) {
        setSystemFlag(FLAG_LARGELI);
      }
      else {
        clearSystemFlag(FLAG_LARGELI);
      }
    }
    if(backupVersion < 1003) {
      restoreStateValue(&tmp1,                           sizeof(tmp1),                                                "CPXMULT",                        "bool");    //JM
      printf("Version number of configfile < 1003, transferring CPXMULT.");
      if(tmp1) {
        setSystemFlag(FLAG_CPXMULT);
      }
      else {
        clearSystemFlag(FLAG_CPXMULT);
      }
    }
    if(backupVersion < 1003) {
      restoreStateValue(&tmp1,                           sizeof(tmp1),                                               "numLock",                        "bool");    //JM ^^
      printf("Version number of configfile < 1003, transferring NUMLOCK.");
      if(tmp1) {
        setSystemFlag(FLAG_NUMLOCK);
      }
      else {
        clearSystemFlag(FLAG_NUMLOCK);
      }
    }
    if(backupVersion < 1003) {
      restoreStateValue(&tmp1,                           sizeof(tmp1),                                              "SI_All",                         "bool");    //JM
      printf("Version number of configfile < 1003, transferring PFX_ALL.");
      if(tmp1) {
        setSystemFlag(FLAG_PFX_ALL);
      }
      else {
        clearSystemFlag(FLAG_PFX_ALL);
      }
    }
    if(backupVersion < 1013) {
      restoreStateValue(&tmp1,                           sizeof(tmp1),                                              "jm_G_DOUBLETAP",                 "bool");    //JM
      printf("Version number of configfile < 1003, transferring FLAG_G_DOUBLETAP.");
      if(tmp1) {
        setSystemFlag(FLAG_G_DOUBLETAP);
      }
      else {
        clearSystemFlag(FLAG_G_DOUBLETAP);
      }
    }
    if(backupVersion < 1013) {
      restoreStateValue(&tmp1,                           sizeof(tmp1),                                              "HOME3",                 "bool");    //JM
      printf("Version number of configfile < 1003, transferring FLAG_HOME_TRIPLE.");
      if(tmp1) {
        setSystemFlag(FLAG_HOME_TRIPLE);
      }
      else {
        clearSystemFlag(FLAG_HOME_TRIPLE);
      }
    }
    if(backupVersion < 1013) {
      restoreStateValue(&tmp1,                           sizeof(tmp1),                                              "MYM3",                 "bool");    //JM
      printf("Version number of configfile < 1003, transferring FLAG_MYM_TRIPLE.");
      if(tmp1) {
        setSystemFlag(FLAG_MYM_TRIPLE);
      }
      else {
        clearSystemFlag(FLAG_MYM_TRIPLE);
      }
    }
    if(backupVersion < 1013) {
      restoreStateValue(&tmp1,                           sizeof(tmp1),                                              "ShiftTimoutMode",                 "bool");    //JM
      printf("Version number of configfile < 1003, transferring FLAG_SHFT_4s.");
      if(tmp1) {
        setSystemFlag(FLAG_SHFT_4s);
      }
      else {
        clearSystemFlag(FLAG_SHFT_4s);
      }
    }
    if(backupVersion < 1013) {
      restoreStateValue(&tmp1,                           sizeof(tmp1),                                              "BASE_HOME",                 "bool");    //JM
      printf("Version number of configfile < 1003, transferring BASE_HOME.");
      if(tmp1) {
        setSystemFlag(FLAG_BASE_HOME);
      }
      else {
        clearSystemFlag(FLAG_BASE_HOME);
      }
    }
    if(backupVersion < 1013) {
      restoreStateValue(&tmp1,                           sizeof(tmp1),                                              "SH_BASE_HOME",                 "bool");    //JM backwards compatible
      printf("Version number of configfile < 1003, transferring SH_BASE_HOME.");
      if(tmp1) {
        setSystemFlag(FLAG_BASE_HOME);
      }
      else {
        clearSystemFlag(FLAG_BASE_HOME);
      }
    }
    if(backupVersion < 1013) {
      restoreStateValue(&tmp1,                           sizeof(tmp1),                                              "BASE_MYM",                 "bool");    //JM
      printf("Version number of configfile < 1003, transferring BASE_MYM.");
      if(tmp1) {
        setSystemFlag(FLAG_BASE_MYM);
      }
      else {
        clearSystemFlag(FLAG_BASE_MYM);
      }
    }
    if(backupVersion < 1013) {
      restoreStateValue(&tmp1,                           sizeof(tmp1),                                              "jm_BASE_SCREEN",                 "bool");    //JM backward compatible
      printf("Version number of configfile < 1003, transferring jm_BASE_SCREEN.");
      if(tmp1) {
        setSystemFlag(FLAG_BASE_MYM);
      }
      else {
        clearSystemFlag(FLAG_BASE_MYM);
      }
    }


    clearScreen(210); // implicit forceSBupdate();



    if(backupVersion < 1009) {                           //register header is already loaded with 32 bits
      printf("Version number of configfile < 1009: chacking all registers for matrix conversion from old 32-bit header to new 32-bit header.");
      int qq = FIRST_GLOBAL_REGISTER;
      while (qq <= LAST_GLOBAL_REGISTER) {
        convertOldMatrixHeaderToNewMatrixHeader(qq);
        qq++;
      }
      qq = FIRST_NAMED_VARIABLE;
      while (qq <= LAST_NAMED_VARIABLE) {
        convertOldMatrixHeaderToNewMatrixHeader(qq);
        qq++;
      }
      qq = FIRST_LOCAL_REGISTER;
      while (qq <= LAST_LOCAL_REGISTER) {
        convertOldMatrixHeaderToNewMatrixHeader(qq);
        qq++;
      }
    }

    if(backupVersion < 1010) {                           //old constant format in equations
      printf("Version number of configfile < 1010: adding # prefix to constants in equations.\n");
      _updateConstantsInEquations();
    }


    // Freeing the space occupied by all the configuration parameters
    paramCurrent = paramHead;
    while(paramHead) {
      paramHead = paramHead->next;
      free(paramCurrent->param);
      free(paramCurrent);
      paramCurrent = paramHead;
    }

    printf("End of calc's restoration\n");fflush(stdout);

    if(temporaryInformation == TI_SHOW_REGISTER_BIG || temporaryInformation == TI_SHOW_REGISTER_SMALL || temporaryInformation == TI_SHOW_REGISTER_TINY || temporaryInformation==TI_SHOW_REGISTER) {
      temporaryInformation = TI_NO_INFO;
    }

    scanLabelsAndPrograms();
    defineCurrentProgramFromGlobalStepNumber(currentLocalStepNumber + abs(programList[currentProgramNumber - 1].step) - 1);
    defineCurrentStep();
    defineFirstDisplayedStep();
    defineCurrentProgramFromCurrentStep();

    //defineCurrentLocalRegisters();

    #if (DEBUG_REGISTER_L == 1)
      refreshRegisterLine(REGISTER_X); // to show L register
    #endif // (DEBUG_REGISTER_L == 1)

    //save and restore screenData is not mandatory
    //for(int y = 0; y < SCREEN_HEIGHT; ++y) {
    //  for(int x = 0; x < SCREEN_WIDTH; x += 8) {
    //    uint8_t bmpdata = *(loadedScreen + (y * SCREEN_WIDTH + x) / 8);
    //    for(int bit = 7; bit >= 0; --bit) {
    //      *(screenData + y * screenStride + x + (7 - bit)) = (bmpdata & (1 << bit)) ? ON_PIXEL : OFF_PIXEL;
    //    }
    //  }
    //}
    //free(loadedScreen);

    if(tam.mode && !tam.alpha) {
      calcModeTamGui();
    }
    else if(tam.mode && tam.alpha) {
      calcModeAimGui();
    }
    else if(   calcMode == CM_NORMAL
            || calcMode == CM_REGISTER_BROWSER
            || calcMode == CM_FLAG_BROWSER
            || calcMode == CM_ASN_BROWSER
            || calcMode == CM_FONT_BROWSER
            || calcMode == CM_PEM
            || calcMode == CM_PLOT_STAT
            || calcMode == CM_GRAPH
            || calcMode == CM_LISTXY) {
      calcModeNormalGui();
    }
    else if(calcMode == CM_MIM) {
      calcModeNormalGui();
      mimRestore();
    }
    else if(calcMode == CM_AIM) {
      calcModeNormalGui();
      calcModeAimGui();
      cursorEnabled = true;
    }
    else if(calcMode == CM_NIM) {
      calcModeNormalGui();
      cursorEnabled = true;
    }
    else if(calcMode == CM_EIM) {
    }
    else if(calcMode == CM_ASSIGN) {
    }
    else if(calcMode == CM_TIMER) {
    }
    else {
      sprintf(errorMessage, commonBugScreenMessages[bugMsgValueFor], "restoreCalc", calcMode, "calcMode");
      displayBugScreen(errorMessage);
    }
    if(catalog) {
      clearSystemFlag(FLAG_ALPHA);
    }

    updateMatrixHeightCache();
    screenUpdatingMode = SCRUPD_AUTO;
    refreshScreen(93);
  }
#endif // PC_BUILD


char aimBuffer1[400];             //The concurrent use of the global aimBuffer
                                  //does not work. See tmpString.
                                  //Temporary solution is to use a local variable of sufficient length for the target.

#if !defined(TESTSUITE_BUILD)
  static void UI64toString(uint64_t value, char * tmpRegisterString);
  static void registerToSaveString(calcRegister_t regist) {
    longInteger_t lgInt;
    int16_t sign;
    uint64_t value;
    uint32_t base;
    char *str;
    uint8_t *cfg;

    tmpRegisterString = tmpString + START_REGISTER_VALUE;

    switch(getRegisterDataType(regist)) {
      case dtLongInteger: {
        convertLongIntegerRegisterToLongInteger(regist, lgInt);
        longIntegerToAllocatedString(lgInt, tmpRegisterString, TMP_STR_LENGTH - START_REGISTER_VALUE - 1);
        longIntegerFree(lgInt);
        strcpy(aimBuffer1, "LonI");
        break;
      }

      case dtString: {
        stringToUtf8(REGISTER_STRING_DATA(regist), (uint8_t *)(tmpRegisterString));
        strcpy(aimBuffer1, "Stri");
        break;
      }

      case dtShortInteger: {
        convertShortIntegerRegisterToUInt64(regist, &sign, &value);
        base = getRegisterShortIntegerBase(regist);

        char yy[25];
        UI64toString(value, yy);
        sprintf(tmpRegisterString, "%c%s %" PRIu32, sign ? '-' : '+', yy, base);
        strcpy(aimBuffer1, "ShoI");
        break;
      }

      case dtReal34: {
        real34ToString(REGISTER_REAL34_DATA(regist), tmpRegisterString);
        strcpy(aimBuffer1, "Real");
        textTag(aimBuffer1, getRegisterAngularMode(regist), 0);
        break;
      }

      case dtComplex34: {
        real34ToString(REGISTER_REAL34_DATA(regist), tmpRegisterString);
        strcat(tmpRegisterString, " ");
        real34ToString(REGISTER_IMAG34_DATA(regist), tmpRegisterString + strlen(tmpRegisterString));
        strcpy(aimBuffer1, "Cplx");
        textTag(aimBuffer1, getRegisterAngularMode(regist), getComplexRegisterPolarMode(regist));
        break;
      }

      case dtTime: {
        real34ToString(REGISTER_REAL34_DATA(regist), tmpRegisterString);
        strcpy(aimBuffer1, "Time");
        break;
      }

      case dtDate: {
        real34ToString(REGISTER_REAL34_DATA(regist), tmpRegisterString);
        strcpy(aimBuffer1, "Date");
        break;
      }

      case dtReal34Matrix: {
        sprintf(tmpRegisterString, "%" PRIu16 " %" PRIu16, REGISTER_MATRIX_HEADER(regist)->matrixRows, REGISTER_MATRIX_HEADER(regist)->matrixColumns);
        strcpy(aimBuffer1, "Rema");
        textTag(aimBuffer1, isRegisterMatrixVector(regist) ? getVectorRegisterAngularMode(regist) : amNone, isRegisterMatrixVector(regist) ? getVectorRegisterPolarMode(regist) : 0);
        break;
      }

      case dtComplex34Matrix: {
        sprintf(tmpRegisterString, "%" PRIu16 " %" PRIu16, REGISTER_MATRIX_HEADER(regist)->matrixRows, REGISTER_MATRIX_HEADER(regist)->matrixColumns);
        strcpy(aimBuffer1, "Cxma");
        textTag(aimBuffer1, (getRegisterTag(regist) & amPolar) == 0 ? amNone : getRegisterAngularMode(regist), getRegisterTag(regist) & amPolar);
        break;
      }

      case dtConfig: {
        for(str=tmpRegisterString, cfg=(uint8_t *)REGISTER_CONFIG_DATA(regist), value=0; value<sizeof(dtConfigDescriptor_t); value++, cfg++, str+=2) {
          sprintf(str, "%02X", *cfg);
        }
        strcpy(aimBuffer1, "Conf");
        break;
      }

      default: {
        strcpy(tmpRegisterString, "???");
        strcpy(aimBuffer1, "????");
      }
    }
  }


  static void saveMatrixElements(calcRegister_t regist) {
    if(getRegisterDataType(regist) == dtReal34Matrix) {
      for(uint32_t element = 0; element < REGISTER_MATRIX_HEADER(regist)->matrixRows * REGISTER_MATRIX_HEADER(regist)->matrixColumns; ++element) {
        real34ToString(REGISTER_REAL34_MATRIX_ELEMENTS(regist) + element, tmpString);
        strcat(tmpString, "\n");
        save(tmpString, strlen(tmpString));
      }
    }
    else if(getRegisterDataType(regist) == dtComplex34Matrix) {
      for(uint32_t element = 0; element < REGISTER_MATRIX_HEADER(regist)->matrixRows * REGISTER_MATRIX_HEADER(regist)->matrixColumns; ++element) {
        real34ToString(VARIABLE_REAL34_DATA(REGISTER_COMPLEX34_MATRIX_ELEMENTS(regist) + element), tmpString);
        strcat(tmpString, " ");
        real34ToString(VARIABLE_IMAG34_DATA(REGISTER_COMPLEX34_MATRIX_ELEMENTS(regist) + element), tmpString + strlen(tmpString));
        strcat(tmpString, "\n");
        save(tmpString, strlen(tmpString));
      }
    }
  }
#endif // !TESTSUITE_BUILD


static void doSave(uint16_t saveType);

  void fnSaveAuto(uint16_t unusedButMandatoryParameter) {
  #ifdef DMCP_BUILD
    doSave(autoSave);
  #endif //DMCP_BUILD
  }


void fnSave(uint16_t saveMode) {
  if(saveMode == SM_MANUAL_SAVE) {
    doSave(manualSave);
  }
  else if(saveMode == SM_STATE_SAVE) {
    doSave(stateSave);
  }
}

void doSave(uint16_t saveType) {
#if !defined(TESTSUITE_BUILD)
  printStatus(0, errorMessages[SAVING_STATE_FILE],force);
  ioFilePath_t path;
  char tmpString[3000];           //The concurrent use of the global tmpString
                                  //as target does not work while the source is at
                                  //tmpRegisterString = tmpString + START_REGISTER_VALUE;
                                  //Temporary solution is to use a local variable of sufficient length for the target.

  int ret;
  calcRegister_t regist;
  uint32_t i;
  char yy1[35], yy2[35];

#if defined(DMCP_BUILD)
  // Don't pass through if the power is insufficient
  if( power_check_screen() ) return;
#endif // DMCP_BUILD

  if(saveType == autoSave) {
    path = ioPathAutoSave;
  }
  else if(saveType == manualSave) {
    path = ioPathManualSave;
  }
  else {
    path = ioPathSaveStateFile;
  }

  ret = ioFileOpen(path, ioModeWrite);

  if(ret != FILE_OK ) {
    if(ret == FILE_CANCEL ) {
      return;
    }
    else {
      #if !defined(DMCP_BUILD)
        printf("Cannot SAVE in file C47.sav!\n");
      #endif // !DMCP_BUILD
      displayCalcErrorMessage(ERROR_CANNOT_WRITE_FILE, ERR_REGISTER_LINE, REGISTER_X);
      return;
    }
  }

  // SAV file version number
  //printHalfSecUpdate_Integer(force+1, "Version",configFileVersion);
  hourGlassIconEnabled = true;
  showHideHourGlass();

  sprintf(tmpString, "SAVE_FILE_REVISION\n%" PRIu8 "\n", (uint8_t)0);
  save(tmpString, strlen(tmpString));
  #if CALCMODEL ==  USER_R47               // Identify which model is creating the save file
    sprintf(tmpString, "R47_save_file_00\n%" PRIu32 "\n", (uint32_t)configFileVersion);
  #else
    sprintf(tmpString, "C47_save_file_00\n%" PRIu32 "\n", (uint32_t)configFileVersion);
  #endif // !CALCMODEL
  save(tmpString, strlen(tmpString));


  // Global registers
  sprintf(tmpString, "GLOBAL_REGISTERS\n%" PRIu16 "\n", (uint16_t)(LAST_GLOBAL_REGISTER+1));
  save(tmpString, strlen(tmpString));
  for(regist=FIRST_GLOBAL_REGISTER; regist<=LAST_GLOBAL_REGISTER; regist++) {
    registerToSaveString(regist);
    sprintf(tmpString, "R%03" PRId16 "\n%s\n%s\n", regist, aimBuffer1, tmpRegisterString);
    save(tmpString, strlen(tmpString));
    saveMatrixElements(regist);
  }

  // Global flags
  strcpy(tmpString, "GLOBAL_FLAGS\n");
  save(tmpString, strlen(tmpString));
  sprintf(tmpString, "%" PRIu16 " %" PRIu16 " %" PRIu16 " %" PRIu16 " %" PRIu16 " %" PRIu16 " %" PRIu16 " %" PRIu16 "\n",
                       globalFlags[0],
                                   globalFlags[1],
                                               globalFlags[2],
                                                           globalFlags[3],
                                                                       globalFlags[4],
                                                                                   globalFlags[5],
                                                                                               globalFlags[6],
                                                                                                           globalFlags[7]);
  save(tmpString, strlen(tmpString));

  // Local registers
  sprintf(tmpString, "LOCAL_REGISTERS\n%" PRIu8 "\n", currentNumberOfLocalRegisters);
  save(tmpString, strlen(tmpString));
  for(i=0; i<currentNumberOfLocalRegisters; i++) {
    registerToSaveString(FIRST_LOCAL_REGISTER + i);
    sprintf(tmpString, "R.%02" PRIu32 "\n%s\n%s\n", i, aimBuffer1, tmpRegisterString);
    save(tmpString, strlen(tmpString));
    saveMatrixElements(FIRST_LOCAL_REGISTER + i);
  }

  // Local flags
  if(currentLocalRegisters) {
    sprintf(tmpString, "LOCAL_FLAGS\n%" PRIu32 "\n", *currentLocalFlags);
    save(tmpString, strlen(tmpString));
  }

  // Named variables
  sprintf(tmpString, "NAMED_VARIABLES\n%" PRIu16 "\n", numberOfNamedVariables);
  save(tmpString, strlen(tmpString));
  for(i=0; i<numberOfNamedVariables; i++) {
    registerToSaveString(FIRST_NAMED_VARIABLE + i);
    stringToUtf8((char *)allNamedVariables[i].variableName + 1, (uint8_t *)tmpString);
    sprintf(tmpString + strlen(tmpString), "\n%s\n%s\n", aimBuffer1, tmpRegisterString);
    save(tmpString, strlen(tmpString));
    saveMatrixElements(FIRST_NAMED_VARIABLE + i);
  }

  // Statistical sums
  sprintf(tmpString, "STATISTICAL_SUMS\n%" PRIu16 "\n", (uint16_t)(statisticalSumsPointer ? NUMBER_OF_STATISTICAL_SUMS : 0));
  save(tmpString, strlen(tmpString));
  for(i=0; i<(statisticalSumsPointer ? NUMBER_OF_STATISTICAL_SUMS : 0); i++) {
    realToString(statisticalSumsPointer + i , tmpRegisterString);
    sprintf(tmpString, "%s\n", tmpRegisterString);
    save(tmpString, strlen(tmpString));
  }

  // System flags
  UI64toString(systemFlags0, yy1);
  sprintf(tmpString, "SYSTEM_FLAGS\n%s\n", yy1);
  save(tmpString, strlen(tmpString));
  UI64toString(systemFlags1, yy1);
  sprintf(tmpString, "SYSTEM_FLAGS1\n%s\n", yy1);
  save(tmpString, strlen(tmpString));

  // Keyboard assignments
  sprintf(tmpString, "KEYBOARD_ASSIGNMENTS\n37\n");
  save(tmpString, strlen(tmpString));
  for(i=0; i<37; i++) {
    sprintf(tmpString, "%" PRId16 " %" PRId16 " %" PRId16 " %" PRId16 " %" PRId16 " %" PRId16 " %" PRId16 " %" PRId16 " %" PRId16 "\n",
                         kbd_usr[i].keyId,
                                     kbd_usr[i].primary,
                                                 kbd_usr[i].fShifted,
                                                             kbd_usr[i].gShifted,
                                                                         kbd_usr[i].keyLblAim,
                                                                                     kbd_usr[i].primaryAim,
                                                                                                 kbd_usr[i].fShiftedAim,
                                                                                                             kbd_usr[i].gShiftedAim,
                                                                                                                         kbd_usr[i].primaryTam);
    save(tmpString, strlen(tmpString));
  }
  if(loadedVersion < 10000023) {
    setLongPressFg(calcModel, -MNU_HOME); // This setting wil be overridden if loadedVersion < 1000022, by backwar old setting HOME3 and MYM3 settings in OTHER_CONFIGURATION_STUFF
  }

  // Keyboard arguments
  sprintf(tmpString, "KEYBOARD_ARGUMENTS\n");
  save(tmpString, strlen(tmpString));

  uint32_t num = 0;
  for(i = 0; i < 37 * 6; ++i) {
    if(*(getNthString((uint8_t *)userKeyLabel, i)) != 0) {
      ++num;
    }
  }
  sprintf(tmpString, "%" PRIu32 "\n", num);
  save(tmpString, strlen(tmpString));

  for(i = 0; i < 37 * 6; ++i) {
    if(*(getNthString((uint8_t *)userKeyLabel, i)) != 0) {
      sprintf(tmpString, "%" PRIu32 " ", i);
      stringToUtf8((char *)getNthString((uint8_t *)userKeyLabel, i), (uint8_t *)tmpString + strlen(tmpString));
      strcat(tmpString, "\n");
      save(tmpString, strlen(tmpString));
    }
  }

  // MyMenu
  sprintf(tmpString, "MYMENU\n18\n");
  save(tmpString, strlen(tmpString));
  for(i=0; i<18; i++) {
    sprintf(tmpString, "%" PRId16, userMenuItems[i].item);
    if(userMenuItems[i].argumentName[0] != 0) {
      strcat(tmpString, " ");
      stringToUtf8(userMenuItems[i].argumentName, (uint8_t *)tmpString + strlen(tmpString));
    }
    strcat(tmpString, "\n");
    save(tmpString, strlen(tmpString));
  }

  // MyAlpha
  sprintf(tmpString, "MYALPHA\n18\n");
  save(tmpString, strlen(tmpString));
  for(i=0; i<18; i++) {
    sprintf(tmpString, "%" PRId16, userAlphaItems[i].item);
    if(userAlphaItems[i].argumentName[0] != 0) {
      strcat(tmpString, " ");
      stringToUtf8(userAlphaItems[i].argumentName, (uint8_t *)tmpString + strlen(tmpString));
    }
    strcat(tmpString, "\n");
    save(tmpString, strlen(tmpString));
  }

  // User menus
  sprintf(tmpString, "USER_MENUS\n");
  save(tmpString, strlen(tmpString));
  sprintf(tmpString, "%" PRIu16 "\n", numberOfUserMenus);
  save(tmpString, strlen(tmpString));
  for(uint32_t j = 0; j < numberOfUserMenus; ++j) {
    stringToUtf8(userMenus[j].menuName, (uint8_t *)tmpString);
    strcat(tmpString, "\n18\n");
    save(tmpString, strlen(tmpString));
    for(i=0; i<18; i++) {
      sprintf(tmpString, "%" PRId16, userMenus[j].menuItem[i].item);
      if(userMenus[j].menuItem[i].argumentName[0] != 0) {
        strcat(tmpString, " ");
        stringToUtf8(userMenus[j].menuItem[i].argumentName, (uint8_t *)tmpString + strlen(tmpString));
      }
      strcat(tmpString, "\n");
      save(tmpString, strlen(tmpString));
    }
  }

  // Programs
  uint16_t currentSizeInBlocks = RAM_SIZE_IN_BLOCKS - TO_C47MEMPTR(beginOfProgramMemory);
  sprintf(tmpString, "PROGRAMS\n%" PRIu16 "\n", currentSizeInBlocks);
  save(tmpString, strlen(tmpString));

  sprintf(tmpString, "%" PRIu32 "\n%" PRIu32 "\n", (uint32_t)TO_C47MEMPTR(currentStep), (uint32_t)((void *)currentStep - TO_PCMEMPTR(TO_C47MEMPTR(currentStep)))); // currentStep block pointer + offset within block
  save(tmpString, strlen(tmpString));

  sprintf(tmpString, "%" PRIu32 "\n%" PRIu32 "\n", (uint32_t)TO_C47MEMPTR(firstFreeProgramByte), (uint32_t)((void *)firstFreeProgramByte - TO_PCMEMPTR(TO_C47MEMPTR(firstFreeProgramByte)))); // firstFreeProgramByte block pointer + offset within block
  save(tmpString, strlen(tmpString));

  sprintf(tmpString, "%" PRIu16 "\n", freeProgramBytes);
  save(tmpString, strlen(tmpString));

  for(i=0; i<currentSizeInBlocks; i++) {
    sprintf(tmpString, "%" PRIu32 "\n", *(((uint32_t *)(beginOfProgramMemory)) + i));
    save(tmpString, strlen(tmpString));
  }

  // Equations
  sprintf(tmpString, "EQUATIONS\n%" PRIu16 "\n", numberOfFormulae);
  save(tmpString, strlen(tmpString));

  for(i=0; i<numberOfFormulae; i++) {
    stringToUtf8(TO_PCMEMPTR(allFormulae[i].pointerToFormulaData), (uint8_t *)tmpString);
    strcat(tmpString, "\n");
    save(tmpString, strlen(tmpString));
  }

  // Other configuration stuff
        sprintf(tmpString, "OTHER_CONFIGURATION_STUFF\n00\n");
        save(tmpString, strlen(tmpString));
        save(tmpString, strlen(tmpString));     //this extra content line + 00 line, is a repeat, is historical, is not used.
                                                //keep here, this line is now used since 2025-11-16 to short circuit the configuration reset, if END_OTHER_PARAM detected here.

        sprintf(tmpString, "firstGregorianDay\n%"          PRIu32 "\n",     firstGregorianDay);            save(tmpString, strlen(tmpString));
        sprintf(tmpString, "denMax\n%"                     PRIu32 "\n",     denMax);                       save(tmpString, strlen(tmpString));
        sprintf(tmpString, "lastDenominator\n%"            PRIu32 "\n",     lastDenominator);              save(tmpString, strlen(tmpString));
        sprintf(tmpString, "displayFormat\n%"              PRIu8  "\n",     displayFormat);                save(tmpString, strlen(tmpString));
        sprintf(tmpString, "displayFormatDigits\n%"        PRIu8  "\n",     displayFormatDigits);          save(tmpString, strlen(tmpString));
        sprintf(tmpString, "timeDisplayFormatDigits\n%"    PRIu8  "\n",     timeDisplayFormatDigits);      save(tmpString, strlen(tmpString));
        sprintf(tmpString, "shortIntegerWordSize\n%"       PRIu8  "\n",     shortIntegerWordSize);         save(tmpString, strlen(tmpString));
        sprintf(tmpString, "shortIntegerMode\n%"           PRIu8  "\n",     shortIntegerMode);             save(tmpString, strlen(tmpString));
        sprintf(tmpString, "significantDigits\n%"          PRIu8  "\n",     significantDigits);            save(tmpString, strlen(tmpString));
        sprintf(tmpString, "fractionDigits\n%"             PRIu8  "\n",     fractionDigits);               save(tmpString, strlen(tmpString));
        sprintf(tmpString, "currentAngularMode\n%"         PRIu8  "\n",     (uint8_t)currentAngularMode);  save(tmpString, strlen(tmpString));
        sprintf(tmpString, "gapItemLeft\n%"                PRIu16 "\n",     gapItemLeft);                  save(tmpString, strlen(tmpString));
        sprintf(tmpString, "gapItemRight\n%"               PRIu16 "\n",     gapItemRight);                 save(tmpString, strlen(tmpString));
        sprintf(tmpString, "gapItemRadix\n%"               PRIu16 "\n",     gapItemRadix);                 save(tmpString, strlen(tmpString));
        sprintf(tmpString, "lastCenturyHighUsed\n%"        PRIu16 "\n",     lastCenturyHighUsed);          save(tmpString, strlen(tmpString));
        sprintf(tmpString, "grpGroupingLeft\n%"            PRIu8  "\n",     grpGroupingLeft);              save(tmpString, strlen(tmpString));
        sprintf(tmpString, "grpGroupingGr1LeftOverflow\n%" PRIu8  "\n",     grpGroupingGr1LeftOverflow);   save(tmpString, strlen(tmpString));
        sprintf(tmpString, "grpGroupingGr1Left\n%"         PRIu8  "\n",     grpGroupingGr1Left);           save(tmpString, strlen(tmpString));
        sprintf(tmpString, "grpGroupingRight\n%"           PRIu8  "\n",     grpGroupingRight);             save(tmpString, strlen(tmpString));
        sprintf(tmpString, "roundingMode\n%"               PRIu8  "\n",     roundingMode);                 save(tmpString, strlen(tmpString));
        sprintf(tmpString, "displayStack\n%"               PRIu8  "\n",     displayStack);                 save(tmpString, strlen(tmpString));
        UI64toString(pcg32_global.state, yy1);
        UI64toString(pcg32_global.inc, yy2);
        sprintf(tmpString, "rngState\n%s %s\n", yy1, yy2);                                                 save(tmpString, strlen(tmpString));
        sprintf(tmpString, "exponentLimit\n%"              PRId16  "\n",    exponentLimit);                save(tmpString, strlen(tmpString));
        sprintf(tmpString, "exponentHideLimit\n%"          PRId16  "\n",    exponentHideLimit);            save(tmpString, strlen(tmpString));
        sprintf(tmpString, "bestF\n%"                      PRIu16  "\n",    lrSelection);                  save(tmpString, strlen(tmpString));
        sprintf(tmpString, "dispBase\n%"                   PRIu8  "\n",     (uint8_t)dispBase);            save(tmpString, strlen(tmpString));
        sprintf(tmpString, "calcModel\n%"                  PRId16  "\n",    calcModel);                    save(tmpString, strlen(tmpString));
        sprintf(tmpString, "Norm_Key_00.func\n%"           PRId16 "\n",     Norm_Key_00.func);             save(tmpString, strlen(tmpString));
        //prevent empty string from being written to config file.
        sprintf(tmpString, "Norm_Key_00.funcParam\n"       "%s"   "\n",     (Norm_Key_00.funcParam[0]==0) ? "NoNormKeyParamDef" : Norm_Key_00.funcParam); save(tmpString, strlen(tmpString));
        sprintf(tmpString, "Norm_Key_00.used\n%"           PRIu8  "\n",     (uint8_t)Norm_Key_00.used);    save(tmpString, strlen(tmpString));
        sprintf(tmpString, "Input_Default\n%"              PRIu8  "\n",     Input_Default);                save(tmpString, strlen(tmpString));
        sprintf(tmpString, "displayStackSHOIDISP\n%"       PRIu8  "\n",     displayStackSHOIDISP);         save(tmpString, strlen(tmpString));
        sprintf(tmpString, "bcdDisplaySign\n%"             PRIu8  "\n",     bcdDisplaySign);               save(tmpString, strlen(tmpString));
        sprintf(tmpString, "DRG_Cycling\n%"                PRIu8  "\n",     DRG_Cycling);                  save(tmpString, strlen(tmpString));
        sprintf(tmpString, "DM_Cycling\n%"                 PRIu8  "\n",     DM_Cycling);                   save(tmpString, strlen(tmpString));
        sprintf(tmpString, "LongPressM\n%"                 PRIu8  "\n",     (uint8_t)LongPressM);          save(tmpString, strlen(tmpString));
        sprintf(tmpString, "LongPressF\n%"                 PRIu8  "\n",     (uint8_t)LongPressF);          save(tmpString, strlen(tmpString));
        sprintf(tmpString, "lastIntegerBase\n%"            PRIu8  "\n",     (uint8_t)lastIntegerBase);     save(tmpString, strlen(tmpString));
        sprintf(tmpString, "lrChosen\n%"                   PRIu16 "\n",     lrChosen);                     save(tmpString, strlen(tmpString));
        sprintf(tmpString, "graph_dx\n"                    "%f"   "\n",     graph_dx);                     save(tmpString, strlen(tmpString));
        sprintf(tmpString, "graph_dy\n"                    "%f"   "\n",     graph_dy);                     save(tmpString, strlen(tmpString));
        sprintf(tmpString, "roundedTicks\n%"               PRIu8  "\n",     (uint8_t)roundedTicks);        save(tmpString, strlen(tmpString));
        sprintf(tmpString, "PLOT_INTG\n%"                  PRIu8  "\n",     (uint8_t)PLOT_INTG);           save(tmpString, strlen(tmpString));
        sprintf(tmpString, "PLOT_DIFF\n%"                  PRIu8  "\n",     (uint8_t)PLOT_DIFF);           save(tmpString, strlen(tmpString));
        sprintf(tmpString, "PLOT_RMS\n%"                   PRIu8  "\n",     (uint8_t)PLOT_RMS);            save(tmpString, strlen(tmpString));
        sprintf(tmpString, "PLOT_SHADE\n%"                 PRIu8  "\n",     (uint8_t)PLOT_SHADE);          save(tmpString, strlen(tmpString));
        sprintf(tmpString, "PLOT_AXIS\n%"                  PRIu8  "\n",     (uint8_t)PLOT_AXIS);           save(tmpString, strlen(tmpString));
        sprintf(tmpString, "PLOT_ZMY\n%"                   PRIu8  "\n",     PLOT_ZMY);                     save(tmpString, strlen(tmpString));
        sprintf(tmpString, "firstDayOfWeek\n%"             PRIu8  "\n",     firstDayOfWeek);               save(tmpString, strlen(tmpString));
        sprintf(tmpString, "firstWeekOfYearDay\n%"         PRIu8  "\n",     firstWeekOfYearDay);           save(tmpString, strlen(tmpString));
        sprintf(tmpString, "END_OTHER_PARAM\n");                                                           save(tmpString, strlen(tmpString));

  ioFileClose();

  hourGlassIconEnabled = false;
  temporaryInformation = TI_SAVED;
#endif // !TESTSUITE_BUILD
}



#if !defined(TESTSUITE_BUILD)
void readLine(char *line) {
  if(!ioEof()) {
    restore(line, 1);
    while((*line == '\n' || *line == '\r') && !ioEof()) {
      restore(line, 1);
    }

    while(*line != '\n' && *line != '\r' && !ioEof()) {
      restore(++line, 1);
    }
  }

  *line = 0;
}

void read2Lines(char *line1, char *line2) {  // Needed to capture empty lines due to empty strings saved from registers
  char eol1,eol2;

  if(!ioEof()) {
    restore(line1, 1);
    while((*line1 == '\n' || *line1 == '\r') && !ioEof()) {
      restore(line1, 1);
    }

    while(*line1 != '\n' && *line1 != '\r' && !ioEof()) {
      restore(++line1, 1);
    }
  }
  eol1 = *line1;
  *line1 = 0;

  if(!ioEof()) {
    restore(line2, 1);
    eol2 = *line2;
    if((((eol1 == '\n') && (eol2 ==  '\n')) || ((eol1 == '\r') && (eol2 ==  '\r'))) && !ioEof()) {   // empty string between two CR or two LF
      *line2 = 0;
      return;
    }
    if((((eol1 == '\r') && (eol2 ==  '\n')) || ((eol1 == '\n') && (eol2 ==  '\r'))) && !ioEof()) {   // end line is CRLF or LFCR
      restore(line2, 1);
      if(((*line2 == '\n') || (*line2 == '\r')) && !ioEof()) {     // empty string after CRLF or LFCR
        *line2 = 0;
        return;
      }
    }

    while(*line2 != '\n' && *line2 != '\r' && !ioEof()) {
      restore(++line2, 1);
    }
  }

  *line2 = 0;
}



static void UI64toString(uint64_t value, char * tmpRegisterString) {
  uint32_t v0,v1;

  v0 = value & 0xffffffff;
  v1 = value >> 32;
  if(v1 != 0) {
    sprintf(tmpRegisterString, "0x%" PRIx32 "%08" PRIx32, v1, v0);
  }
  else {
    sprintf(tmpRegisterString, "0x%" PRIx32, v0);
  }
}
#endif // !TESTSUITE_BUILD

#define stringToIntFunc(name, type)               \
  type name(const char *str) {                    \
    return (type)strtol(str, NULL, 0);            \
  }

#define stringToUintFunc(name, type)              \
  type name(const char *str) {                    \
    return (type)strtoul(str, NULL, 0);           \
  }

stringToUintFunc(stringToUint8,  uint8_t)
stringToUintFunc(stringToUint16, uint16_t)
stringToUintFunc(stringToUint32, uint32_t)

uint64_t stringToUint64(const char *str) {
  return strtoull(str, NULL, 0);
}

stringToIntFunc(stringToInt8,  int8_t)
stringToIntFunc(stringToInt16, int16_t)
stringToIntFunc(stringToInt32, int32_t)

int64_t stringToInt64(const char *str) {
  return strtoll(str, NULL, 0);
}


#if !defined(TESTSUITE_BUILD)
  static void restoreRegister(calcRegister_t regist, char *type, char *value) {
    uint32_t tag = amNone;

    //printf("restoreRegister:%i %s %s\n",regist, type, value);

    if(type[4] == ':') {
      if(type[5] == 'R') {
        tag = amRadian;
      }
      else if(type[5] == 'M') {
        tag = amMultPi;
      }
      else if(type[5] == 'G') {
        tag = amGrad;
      }
      else if(type[5] == 'D' && type[6] == 'E') {
        tag = amDegree;
      }
      else if(type[5] == 'D' && type[6] == 'M') {
        tag = amDMS;
      }
      else {
        tag = amNone;
      }

      if(type[stringByteLength(type)-1] == 'p') {
        tag |= amPolar;
      }
      type[4] = 0;
      //printf("               :%i %s %s\n",regist, type, value);
    }


    if(strcmp(type, "Real") == 0) {
      reallocateRegister(regist, dtReal34, 0, tag);
      stringToReal34(value, REGISTER_REAL34_DATA(regist));
    }

    else if(strcmp(type, "Time") == 0) {
      reallocateRegister(regist, dtTime, 0, amNone);
      stringToReal34(value, REGISTER_REAL34_DATA(regist));
    }

    else if(strcmp(type, "Date") == 0) {
      reallocateRegister(regist, dtDate, 0, amNone);
      stringToReal34(value, REGISTER_REAL34_DATA(regist));
    }

    else if(strcmp(type, "LonI") == 0) {
      longInteger_t lgInt;

      longIntegerInit(lgInt);
      stringToLongInteger(value, 10, lgInt);
      convertLongIntegerToLongIntegerRegister(lgInt, regist);
      longIntegerFree(lgInt);
    }

    else if(strcmp(type, "Stri") == 0) {
      int32_t len;

      utf8ToString((uint8_t *)value, errorMessage);
      len = stringByteLength(errorMessage) + 1;
      reallocateRegister(regist, dtString, TO_BLOCKS(len), amNone);
      xcopy(REGISTER_STRING_DATA(regist), errorMessage, len);
    }

    else if(strcmp(type, "ShoI") == 0) {
      uint16_t sign = (value[0] == '-' ? 1 : 0);
      uint64_t val  = stringToUint64(value + 1);

      value = next_word(value);
      uint32_t base = toUint32(value);

      convertUInt64ToShortIntegerRegister(sign, val, base, regist);
    }

    else if(strcmp(type, "Cplx") == 0) {
      char *imaginaryPart;

      reallocateRegister(regist, dtComplex34, 0, tag);
      imaginaryPart = skip_word(value);
      *(imaginaryPart++) = 0;
      stringToReal34(value, REGISTER_REAL34_DATA(regist));
      stringToReal34(imaginaryPart, REGISTER_IMAG34_DATA(regist));
    }

  #if !defined(TESTSUITE_BUILD)
    else if(strcmp(type, "Rema") == 0) {
      char *numOfCols;
      uint16_t rows, cols;

      numOfCols = skip_word(value);
      *(numOfCols++) = 0;
      rows = toUint16(value);
      cols = toUint16(numOfCols);
      reallocateRegister(regist, dtReal34Matrix, REAL34_SIZE_IN_BLOCKS * rows * cols, tag);
      REGISTER_MATRIX_HEADER(regist)->matrixRows = rows;
      REGISTER_MATRIX_HEADER(regist)->matrixColumns = cols;
      //printf("     %i %i %i\n",rows, cols, tag);
    }

    else if(strcmp(type, "Cxma") == 0) {
      char *numOfCols;
      uint16_t rows, cols;

      numOfCols = skip_word(value);
      *(numOfCols++) = 0;
      rows = toUint16(value);
      cols = toUint16(numOfCols);
      reallocateRegister(regist, dtComplex34Matrix, COMPLEX34_SIZE_IN_BLOCKS * rows * cols, tag);
      REGISTER_MATRIX_HEADER(regist)->matrixRows = rows;
      REGISTER_MATRIX_HEADER(regist)->matrixColumns = cols;
    }
  #endif // TESTSUITE_BUILD

    else if(strcmp(type, "Conf") == 0) {
      char *cfg;

      reallocateRegister(regist, dtConfig, 0, amNone);
      for(cfg=(char *)REGISTER_CONFIG_DATA(regist), tag=0;  tag < (loadedVersion < 10000008 ? 896 : sizeof(dtConfigDescriptor_t)); tag++, value+=2, cfg++) {
        *cfg = ((*value >= 'A' ? *value - 'A' + 10 : *value - '0') << 4) | (*(value + 1) >= 'A' ? *(value + 1) - 'A' + 10 : *(value + 1) - '0');
      }
      if(loadedVersion < 10000008) {
        // For earlier version config files of 896 desxcriptor length, the above Write into the register must only be up to the old descriptor content.
        // We add the defaults for the new portion of the new descriptor in the following string.
        static const unsigned char tmpvalue[] = {
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0xF7, 0x77, 0xDC, 0x2C, 0x2B, 0x84, 0x2A, 0x1C,
          0x33, 0x20, 0x30, 0x33, 0x46, 0x0C, 0x2A, 0x33,
          0x01, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        };
        memcpy(cfg, tmpvalue, sizeof(tmpvalue));
      }
    }

    else {
      sprintf(errorMessage, "In function restoreRegister: Data: Reg %d, type %s, value %s to be coded!", (int16_t)regist, type, value);
      displayBugScreen(errorMessage);
    }
  }


  static void restoreMatrixData(calcRegister_t regist) {
    #if !defined(TESTSUITE_BUILD)
    uint16_t rows, cols;
    uint32_t i;

    if(getRegisterDataType(regist) == dtReal34Matrix) {
      rows = REGISTER_MATRIX_HEADER(regist)->matrixRows;
      cols = REGISTER_MATRIX_HEADER(regist)->matrixColumns;

      for(i = 0; i < rows * cols; ++i) {
        readLine(tmpString);
        stringToReal34(tmpString, REGISTER_REAL34_MATRIX_ELEMENTS(regist) + i);
      }
    }

    if(getRegisterDataType(regist) == dtComplex34Matrix) {
      rows = REGISTER_MATRIX_HEADER(regist)->matrixRows;
      cols = REGISTER_MATRIX_HEADER(regist)->matrixColumns;

      for(i = 0; i < rows * cols; ++i) {
        char *imaginaryPart;

        readLine(tmpString);
        imaginaryPart = skip_word(tmpString);
        *(imaginaryPart++) = 0;
        stringToReal34(tmpString,     VARIABLE_REAL34_DATA(REGISTER_COMPLEX34_MATRIX_ELEMENTS(regist) + i));
        stringToReal34(imaginaryPart, VARIABLE_IMAG34_DATA(REGISTER_COMPLEX34_MATRIX_ELEMENTS(regist) + i));
      }
    }
    #endif // !TESTSUITE_BUILD
  }


  static void skipMatrixData(char *type, char *value) {
    #if !defined(TESTSUITE_BUILD)
    uint16_t rows, cols;
    uint32_t i;
    char *numOfCols;

    if(strcmp(type, "Rema") == 0 || strcmp(type, "Cxma") == 0) {
      numOfCols = skip_word(value);
      *(numOfCols++) = 0;
      rows = toUint16(value);
      cols = toUint16(numOfCols);

      for(i = 0; i < rows * cols; ++i) {
        readLine(tmpString);
      }
    }
    #endif // !TESTSUITE_BUILD
  }


#if defined(LOADDEBUG)
  static void debugPrintf(int s1, const char * s2, const char * s3) {
    #if defined(PC_BUILD)
      printf("%i %s %s\n", s1, s2, s3);
    #else
      char yy[1000];
      sprintf(yy,"%i %s %s\n", s1, s2, s3);
//      printHalfSecUpdate_Integer(force+1, yy, 0);
//      print_linestr(yy,false);

    #endif //!PC_BUILD
  }
#endif //LOADDEBUG


  uint16_t strcmp2(char* inStr, char* in2Str) {       //special comparison, only to accommodate incorrect separator saves in versions 10000005-6.
    if(strcmp(inStr, in2Str) == 0) {
      return 0;
    }
    if(stringByteLength(inStr) != stringByteLength(in2Str)+1 || stringByteLength(inStr) > 50) {     //if length mismatch;
      return 1;
    }
    char tmps[60];
    tmps[0]=32;
    tmps[1]=0;
    strcat(tmps, in2Str);
    if(strcmp(inStr, tmps) == 0) {
      return 0;
    }
    return 1;
  }

  uint16_t savedCalcModel = 0;
  static bool_t restoreOneSection(uint16_t loadMode, uint16_t s, uint16_t n, uint16_t d, bool_t allowUserKeys) {
    int16_t i, numberOfRegs;
    calcRegister_t regist;
    char *str;
    #if defined(LOADDEBUG)
      char line[1000];
    #endif //LOADDEBUG

    cancelFilename = true;
    hourGlassIconEnabled = true;
    showHideHourGlass();
    readLine(tmpString);
    #if defined(LOADDEBUG)
      sprintf(line,", loadMode:%d, %s\n",loadMode,tmpString);
      debugPrintf(0, "-", line);
    #endif //LOADDEBUG

    if(strcmp(tmpString, "GLOBAL_REGISTERS") == 0) {
      readLine(tmpString); // Number of global registers
      numberOfRegs = toInt16(tmpString);
      for(i=0; i<numberOfRegs; i++) {
        readLine(tmpString); // Register number
        regist = toInt16(tmpString + 1);
        read2Lines(aimBuffer,tmpString); // Register data type & Register value

        if(loadMode == LM_ALL || (loadMode == LM_REGISTERS && regist < REGISTER_X) || (loadMode == LM_REGISTERS_PARTIAL && regist >= s && regist < (s + n))) {
          #if defined(LOADDEBUG)
            sprintf(line,", register=%i loadMode:%d, ['%s'] = %s", regist - s + d, loadMode, aimBuffer, tmpString);
            debugPrintf(1, "-", line);
          #endif //LOADDEBUG
          restoreRegister(loadMode == LM_REGISTERS_PARTIAL ? (regist - s + d) : regist, aimBuffer, tmpString);
          restoreMatrixData(loadMode == LM_REGISTERS_PARTIAL ? (regist - s + d) : regist);
        }
        else {
          skipMatrixData(aimBuffer, tmpString);
        }
      }
    }

    else if(strcmp(tmpString, "GLOBAL_FLAGS") == 0) {
      readLine(tmpString); // Global flags
      if(loadMode == LM_ALL || loadMode == LM_SYSTEM_STATE) {
        #if defined(LOADDEBUG)
          sprintf(line,", loadMode:%d, %s\n",loadMode,tmpString);
          debugPrintf(2, "-", tmpString);
        #endif //LOADDEBUG
        str = tmpString;
        for (i = 0; i < 7; i++) {
          globalFlags[i] = toUint16(str);
          str = next_word(str);
        }
        globalFlags[i] = toUint16(str);
      }
    }

    else if(strcmp(tmpString, "LOCAL_REGISTERS") == 0) {
      readLine(tmpString); // Number of local registers
      numberOfRegs = toInt16(tmpString);
      if(loadMode == LM_ALL || loadMode == LM_REGISTERS) {
        #if defined(LOADDEBUG)
          sprintf(line,", loadMode:%d, %s\n",loadMode,tmpString);
          debugPrintf(3, "A", tmpString);
        #endif //LOADDEBUG
        allocateLocalRegisters(numberOfRegs);
      }

      if((loadMode != LM_ALL && loadMode != LM_REGISTERS) || lastErrorCode == ERROR_NONE) {
        #if defined(LOADDEBUG)
          sprintf(line,", loadMode:%d, %s\n",loadMode,tmpString);
          debugPrintf(3, "B", tmpString);
        #endif //LOADDEBUG
        for(i=0; i<numberOfRegs; i++) {
          readLine(tmpString); // Register number
          regist = toInt16(tmpString + 2) + FIRST_LOCAL_REGISTER;
          read2Lines(aimBuffer,tmpString); // Register data type & Register value

          if(loadMode == LM_ALL || loadMode == LM_REGISTERS) {
            #if defined(LOADDEBUG)
              sprintf(line,", loadMode:%d, %s\n",loadMode,tmpString);
              debugPrintf(3, "C", tmpString);
            #endif //LOADDEBUG
            restoreRegister(regist, aimBuffer, tmpString);
            restoreMatrixData(regist);
          }
          else {
            skipMatrixData(aimBuffer, tmpString);
          }
        }
      }
    }

    else if(strcmp(tmpString, "LOCAL_FLAGS") == 0) {
      #if defined(LOADDEBUG)
        sprintf(line,", loadMode:%d, %s\n",loadMode,tmpString);
        debugPrintf(4, "A", tmpString);
      #endif //LOADDEBUG
      readLine(tmpString); // LOCAL_FLAGS
      if(loadMode == LM_ALL || loadMode == LM_REGISTERS) {
        #if defined(LOADDEBUG)
          sprintf(line,", loadMode:%d, %s\n",loadMode,tmpString);
          debugPrintf(4, "B", tmpString);
        #endif //LOADDEBUG
        *currentLocalFlags = toUint32(tmpString);
      }
    }

    else if(strcmp(tmpString, "NAMED_VARIABLES") == 0) {
      #if defined(LOADDEBUG)
        sprintf(line,", loadMode:%d, %s\n",loadMode,tmpString);
        debugPrintf(20, "A", tmpString);
      #endif //LOADDEBUG
      readLine(tmpString); // Number of named variables
      numberOfRegs = toInt16(tmpString);
      for(i=0; i<numberOfRegs; i++) {
        readLine(errorMessage); // Variable name
        read2Lines(aimBuffer,tmpString); // Variable data type & Variable value

        if(( loadMode == LM_ALL ||
             loadMode == LM_NAMED_VARIABLES ||
            (loadMode == LM_SUMS && ((strcmp(errorMessage, "STATS") == 0) || (strcmp(errorMessage, "HISTO") == 0))) ) &&

           !(loadMode == LM_NAMED_VARIABLES && ((strcmp(errorMessage, "STATS") == 0) || (strcmp(errorMessage, "HISTO") == 0)))
          ) {

          #if defined(LOADDEBUG)
            sprintf(line,", loadMode:%d, %s\n",loadMode,tmpString);
            debugPrintf(20, "B", tmpString);
          #endif //LOADDEBUG
          char *varName = errorMessage + strlen(errorMessage) + 1;
          utf8ToString((uint8_t *)errorMessage, varName);
          regist = findOrAllocateNamedVariable(varName);
          if(regist != INVALID_VARIABLE) {
            restoreRegister(regist, aimBuffer, tmpString);
            restoreMatrixData(regist);
          }
          else {
            skipMatrixData(aimBuffer, tmpString);
          }
        }
        else {
          skipMatrixData(aimBuffer, tmpString);
        }
      }
    }

    else if(strcmp(tmpString, "STATISTICAL_SUMS") == 0) {
      readLine(tmpString); // Number of statistical sums
      numberOfRegs = toInt16(tmpString);
      if(numberOfRegs > 0 && (loadMode == LM_ALL || loadMode == LM_SUMS)) {
        #if defined(LOADDEBUG)
          sprintf(line,", loadMode:%d, %s\n",loadMode,tmpString);
          debugPrintf(6, "A", tmpString);
        #endif //LOADDEBUG

        initStatisticalSums();
        reLoadStatisticalSums();

        for(i=0; i<numberOfRegs; i++) {
          readLine(tmpString); // statistical sum
          if(statisticalSumsPointer) { // likely
            if(loadMode == LM_ALL || loadMode == LM_SUMS) {
              #if defined(LOADDEBUG)
                sprintf(line,", loadMode:%d, %s\n",loadMode,tmpString);
                debugPrintf(6, "B", tmpString);
              #endif //LOADDEBUG
              stringToReal(tmpString, statisticalSumsPointer + i, &ctxtReal75);
            }
          }
        }
      }
    }

    else if(strcmp(tmpString, "SYSTEM_FLAGS") == 0) {
      readLine(tmpString); // Global flags
      if(loadMode == LM_ALL || loadMode == LM_SYSTEM_STATE) {
        #if defined(LOADDEBUG)
          sprintf(line,", loadMode:%d, %s\n",loadMode,tmpString);
          debugPrintf(7, "-", tmpString);
        #endif //LOADDEBUG
        systemFlags0 = stringToUint64(tmpString);
        systemFlags1 = 0;
        if(loadedVersion < 10000006) {
          defaultStatusBar(); //clear systemflags for early version config files
        }
        if(loadedVersion < 10000009) {
          setSystemFlag(FLAG_MONIT); //Monitoring is on per default
        }
      }
    }

    else if(strcmp(tmpString, "SYSTEM_FLAGS1") == 0) {
      readLine(tmpString); // Global flags
      if(loadMode == LM_ALL || loadMode == LM_SYSTEM_STATE) {
        #if defined(LOADDEBUG)
          sprintf(line,", loadMode:%d, %s\n",loadMode,tmpString);
          debugPrintf(7, "-", tmpString);
        #endif //LOADDEBUG
        systemFlags1 = stringToUint64(tmpString);
        if(loadedVersion < 10000006) {
          defaultStatusBar(); //clear systemflags for early version config files
        }
        if(loadedVersion < 10000009) {
          setSystemFlag(FLAG_MONIT); //Monitoring is on per default
        }
        if(loadedVersion < 10000012) {
          clearSystemFlag(FLAG_IRFRAC); //restore previously used manually stored flags in OTHER STUFF below
          clearSystemFlag(FLAG_IRFRQ);  //restore previously used manually stored flags in OTHER STUFF below
        }

        // Ensure valid relations between FLAG_FRACT, FLAG_IRFRAC and FLAG_IRFRQ
        if (getSystemFlag(FLAG_FRACT)) {
          setSystemFlag(FLAG_FRACT);
        }
        else if (getSystemFlag(FLAG_IRFRAC)) {
          setSystemFlag(FLAG_IRFRAC);
        }
      }
    }

    else if(strcmp(tmpString, "KEYBOARD_ASSIGNMENTS") == 0) {
      readLine(tmpString); // Number of keys
      numberOfRegs = toInt16(tmpString);
      for(i=0; i<numberOfRegs; i++) {
        readLine(tmpString); // key
        if(allowUserKeys && (loadMode == LM_ALL || loadMode == LM_SYSTEM_STATE)) { // Restore Keyboard Assignments only if they were save on the same calc model (C47->C47 or R47->R47)
          #if defined(LOADDEBUG)
            sprintf(line,", loadMode:%d, %s\n",loadMode,tmpString);
            debugPrintf(8, "-", tmpString);
          #endif //LOADDEBUG
          str = toInt16_next_word(tmpString, &kbd_usr[i].keyId);
          str = toInt16_next_word(str, &kbd_usr[i].primary);
          str = toInt16_next_word(str, &kbd_usr[i].fShifted);
          str = toInt16_next_word(str, &kbd_usr[i].gShifted);
          str = toInt16_next_word(str, &kbd_usr[i].keyLblAim);
          str = toInt16_next_word(str, &kbd_usr[i].primaryAim);
          str = toInt16_next_word(str, &kbd_usr[i].fShiftedAim);
          str = toInt16_next_word(str, &kbd_usr[i].gShiftedAim);
          str = toInt16_next_word(str, &kbd_usr[i].primaryTam);
        }
      }
    }

    else if(strcmp(tmpString, "KEYBOARD_ARGUMENTS") == 0) {
      readLine(tmpString); // Number of keys
      numberOfRegs = toInt16(tmpString);
      if(allowUserKeys && (loadMode == LM_ALL || loadMode == LM_SYSTEM_STATE)) {
        #if defined(LOADDEBUG)
          sprintf(line,", loadMode:%d, %s\n",loadMode,tmpString);
          debugPrintf(9, "A", tmpString);
        #endif //LOADDEBUG
        freeC47Blocks(userKeyLabel, TO_BLOCKS(userKeyLabelSize));
        userKeyLabelSize = 37/*keys*/ * 6/*states*/ * 1/*byte terminator*/ + 1/*byte sentinel*/;
        userKeyLabel = allocC47Blocks(TO_BLOCKS(userKeyLabelSize));
        memset(userKeyLabel,   0, TO_BYTES(TO_BLOCKS(userKeyLabelSize)));
      }

      for(i=0; i<numberOfRegs; i++) {
        readLine(tmpString); // key
        // Restore Keyboard Arguments only if they were save on the same calc model (C47->C47 or R47->R47)
        if(allowUserKeys && (loadMode == LM_ALL || loadMode == LM_SYSTEM_STATE)) {
          #if defined(LOADDEBUG)
            sprintf(line,", loadMode:%d, %s\n",loadMode,tmpString);
            debugPrintf(9, "B", tmpString);
          #endif //LOADDEBUG
          str = tmpString;
          uint16_t key = toUint16(str);
          userMenuItems[i].argumentName[0] = 0;

          str = skip_to_space_newline(str);
          if(*str == ' ') {
            str = skip_space(str);
            if((*str != '\n') && (*str != 0)) {
              utf8ToString((uint8_t *)str, tmpString + TMP_STR_LENGTH / 2);
              setUserKeyArgument(key, tmpString + TMP_STR_LENGTH / 2);
            }
          }
        }
      }
    }

    else if(strcmp(tmpString, "MYMENU") == 0) {
      readLine(tmpString); // Number of keys
      numberOfRegs = toInt16(tmpString);
      for(i=0; i<numberOfRegs; i++) {
        readLine(tmpString); // key
        if(loadMode == LM_ALL || loadMode == LM_SYSTEM_STATE) {
          #if defined(LOADDEBUG)
            sprintf(line,", loadMode:%d, %s\n",loadMode,tmpString);
            debugPrintf(10, "-", tmpString);
          #endif //LOADDEBUG
          str = tmpString;
          userMenuItems[i].item            = toInt16(str);
          userMenuItems[i].argumentName[0] = 0;

          str = skip_to_space_newline(str);
          if(*str == ' ') {
            str = skip_space(str);
            if((*str != '\n') && (*str != 0)) {
              utf8ToString((uint8_t *)str, userMenuItems[i].argumentName);
            }
          }
        }
      }
    }

    else if(strcmp(tmpString, "MYALPHA") == 0) {
      readLine(tmpString); // Number of keys
      numberOfRegs = toInt16(tmpString);
      for(i=0; i<numberOfRegs; i++) {
        readLine(tmpString); // key
        if(loadMode == LM_ALL || loadMode == LM_SYSTEM_STATE) {
          #if defined(LOADDEBUG)
            sprintf(line,", loadMode:%d, %s\n",loadMode,tmpString);
            debugPrintf(11, "-", tmpString);
          #endif //LOADDEBUG
          str = tmpString;
          userAlphaItems[i].item            = toInt16(str);
          userAlphaItems[i].argumentName[0] = 0;

          str = skip_to_space_newline(str);
          if(*str == ' ') {
            str = skip_space(str);
            if((*str != '\n') && (*str != 0)) {
              utf8ToString((uint8_t *)str, userAlphaItems[i].argumentName);
            }
          }
        }
      }
    }

    else if(strcmp(tmpString, "USER_MENUS") == 0) {
      readLine(tmpString); // Number of keys
      int16_t numberOfMenus = toInt16(tmpString);
      for(int32_t j=0; j<numberOfMenus; j++) {
        readLine(tmpString);
        int16_t target = -1;
        if(loadMode == LM_ALL || loadMode == LM_SYSTEM_STATE) {
          #if defined(LOADDEBUG)
            sprintf(line,", loadMode:%d, %s\n",loadMode,tmpString);
            debugPrintf(12, "-", tmpString);
          #endif //LOADDEBUG
          utf8ToString((uint8_t *)tmpString, tmpString + TMP_STR_LENGTH / 2);
          for(i = 0; i < numberOfUserMenus; ++i) {
            if(compareString(tmpString + TMP_STR_LENGTH / 2, userMenus[i].menuName, CMP_NAME) == 0) {
              target = i;
            }
          }
          if(target == -1) {
            createMenu(tmpString + TMP_STR_LENGTH / 2);
            target = numberOfUserMenus - 1;
          }
        }

        readLine(tmpString);
        numberOfRegs = toInt16(tmpString);
        for(i=0; i<numberOfRegs; i++) {
          readLine(tmpString); // key
          if(loadMode == LM_ALL || loadMode == LM_SYSTEM_STATE) {
            #if defined(LOADDEBUG)
              sprintf(line,", loadMode:%d, %s\n",loadMode,tmpString);
              debugPrintf(13, "-", tmpString);
            #endif //LOADDEBUG
            str = tmpString;
            userMenus[target].menuItem[i].item            = toInt16(str);
            userMenus[target].menuItem[i].argumentName[0] = 0;

            str = skip_to_space_newline(str);
            if(*str == ' ') {
              str = skip_space(str);
              if((*str != '\n') && (*str != 0)) {
                utf8ToString((uint8_t *)str, userMenus[target].menuItem[i].argumentName);
              }
            }
          }
        }
      }
    }

    else if(strcmp(tmpString, "PROGRAMS") == 0) {
      #if defined(LOADDEBUG)
        if(loadMode == LM_ALL || loadMode == LM_PROGRAMS) {
          sprintf(line,", loadMode:%d, %s\n",loadMode,tmpString);
          debugPrintf(14, "-", tmpString);
        }
      #endif //LOADDEBUG
      uint16_t numberOfBlocks;
      uint16_t oldSizeInBlocks = RAM_SIZE_IN_BLOCKS - TO_C47MEMPTR(beginOfProgramMemory);
      uint8_t *oldFirstFreeProgramByte = firstFreeProgramByte;
      uint16_t oldFreeProgramBytes = freeProgramBytes;

      readLine(tmpString); // Number of blocks
      numberOfBlocks = toUint16(tmpString);
      if(loadMode == LM_ALL) {
        resizeProgramMemory(numberOfBlocks);
      }
      else if(loadMode == LM_PROGRAMS) {
        resizeProgramMemory(oldSizeInBlocks + numberOfBlocks);
        oldFirstFreeProgramByte = beginOfProgramMemory + TO_BYTES(oldSizeInBlocks) - oldFreeProgramBytes - 2;
      }

      readLine(tmpString); // currentStep (pointer to block)
      if(loadMode == LM_ALL) {
        currentStep = TO_PCMEMPTR(toUint32(tmpString));
      }
      readLine(tmpString); // currentStep (offset in bytes within block)
      if(loadMode == LM_ALL) {
        currentStep += toUint32(tmpString);
      }
      else if(loadMode == LM_PROGRAMS) {
        if(programList[currentProgramNumber - 1].step > 0) {
          currentStep           -= TO_BYTES(numberOfBlocks);
          firstDisplayedStep    -= TO_BYTES(numberOfBlocks);
          beginOfCurrentProgram -= TO_BYTES(numberOfBlocks);
          endOfCurrentProgram   -= TO_BYTES(numberOfBlocks);
        }
      }

      readLine(tmpString); // firstFreeProgramByte (pointer to block)
      if(loadMode == LM_ALL || loadMode == LM_PROGRAMS) {
        firstFreeProgramByte = TO_PCMEMPTR(toUint32(tmpString));
      }
      readLine(tmpString); // firstFreeProgramByte (offset in bytes within block)
      if(loadMode == LM_ALL || loadMode == LM_PROGRAMS) {
        firstFreeProgramByte += toUint32(tmpString);
      }

      readLine(tmpString); // freeProgramBytes
      if(loadMode == LM_ALL || loadMode == LM_PROGRAMS) {
        freeProgramBytes = toUint16(tmpString);
      }


      if (firstFreeProgramByte + freeProgramBytes != beginOfProgramMemory + TO_BYTES(numberOfBlocks) - 2) {
        uint32_t diff = TO_BYTES(RAM_SIZE_IN_BLOCKS_NEW_HW - RAM_SIZE_IN_BLOCKS_OLD_HW);
        #if defined(DMCP_BUILD) && defined(OLD_HW)
          if ((firstFreeProgramByte + freeProgramBytes - diff == beginOfProgramMemory + TO_BYTES(numberOfBlocks) - 2)) {
            currentStep -= diff;
            firstFreeProgramByte -= diff;
          }
        #else
          if ((firstFreeProgramByte + freeProgramBytes + diff == beginOfProgramMemory + TO_BYTES(numberOfBlocks) - 2)) {
            currentStep += diff;
            firstFreeProgramByte += diff;
          }
        #endif
      }

      if(loadMode == LM_PROGRAMS) { // .END. to END
        freeProgramBytes += oldFreeProgramBytes;
        if((oldFirstFreeProgramByte >= (beginOfProgramMemory + 2)) && isAtEndOfProgram(oldFirstFreeProgramByte - 2)) {
        }
        else {
          if(oldFreeProgramBytes + freeProgramBytes < 2) {
            uint16_t tmpFreeProgramBytes = freeProgramBytes;
            resizeProgramMemory(oldSizeInBlocks + numberOfBlocks + 1);
            oldFirstFreeProgramByte -= 4;
            freeProgramBytes = tmpFreeProgramBytes + 4;
            if(programList[currentProgramNumber - 1].step > 0) {
            currentStep           -= 4;
            firstDisplayedStep    -= 4;
            beginOfCurrentProgram -= 4;
            endOfCurrentProgram   -= 4;
            }
          }
          *(oldFirstFreeProgramByte    ) = (ITM_END >> 8) | 0x80;
          *(oldFirstFreeProgramByte + 1) =  ITM_END       & 0xff;
          freeProgramBytes -= 2;
          oldFirstFreeProgramByte += 2;
        }
      }

      #if defined(LOADDEBUG)
        if(loadMode == LM_ALL || loadMode == LM_PROGRAMS) {
          printf("Before loading programs:\n");
          printf("  beginOfProgramMemory    = *8b %4u %16p\n",*beginOfProgramMemory,    (void*)(((uint32_t *)(beginOfProgramMemory)) ));
          printf("  firstFreeProgramByte    = *8b %4u %16p\n",*firstFreeProgramByte,    (void*)(((uint32_t *)(firstFreeProgramByte)) ));
          printf("  oldFirstFreeProgramByte = *8b %4u %16p\n",*oldFirstFreeProgramByte, (void*)(((uint32_t *)(oldFirstFreeProgramByte)) ));
          printf("\n  freeProgramBytes        = 16b %u\n", freeProgramBytes);
          printf("  numberOfBlocks          = 16b %u, on dot per block: ", numberOfBlocks);
        }
      #endif // LOADDEBUG

      for(i=0; i<numberOfBlocks; i++) {

        #if defined(LOADDEBUG)
          printf(".");
        #endif // LOADDEBUG

        readLine(tmpString); // One block
        if(loadMode == LM_ALL) {
          *(((uint32_t *)(beginOfProgramMemory)) + i) = toUint32(tmpString);
        }
        else if(loadMode == LM_PROGRAMS) {
          uint32_t tmpBlock = toUint32(tmpString);
          xcopy(oldFirstFreeProgramByte + TO_BYTES(i), (uint8_t *)(&tmpBlock), 4);
        }
      }

      #if defined(LOADDEBUG)
        if(loadMode == LM_ALL || loadMode == LM_PROGRAMS) {
          printf("\n");
          printf("After loading programs:\n");
          printf("  beginOfProgramMemory    = *8b %4u %16p\n",*beginOfProgramMemory,    (void*)(((uint32_t *)(beginOfProgramMemory)) ));
          printf("  firstFreeProgramByte    = *8b %4u %16p\n",*firstFreeProgramByte,    (void*)(((uint32_t *)(firstFreeProgramByte)) ));
          printf("  oldFirstFreeProgramByte = *8b %4u %16p\n",*oldFirstFreeProgramByte, (void*)(((uint32_t *)(oldFirstFreeProgramByte)) ));
        }
      #endif // LOADDEBUG

      if(loadMode == LM_ALL || loadMode == LM_PROGRAMS) {
        scanLabelsAndPrograms();
      }
    }

    else if(strcmp(tmpString, "EQUATIONS") == 0) {
      uint16_t formulae = 0;

      if(loadMode == LM_ALL || loadMode == LM_PROGRAMS) {
        #if defined(LOADDEBUG)
          sprintf(line,", loadMode:%d, %s\n",loadMode,tmpString);
          debugPrintf(15, "A", tmpString);
        #endif //LOADDEBUG
      }

      readLine(tmpString); // Number of formulae
      formulae = toUint16(tmpString);
      if(formulae > 0 && (loadMode == LM_ALL || loadMode == LM_PROGRAMS)) {
        #if defined(LOADDEBUG)
          sprintf(line,", loadMode:%d, %s; formulae = %d\n",loadMode,tmpString, formulae);
          debugPrintf(15, "AA", "Resetting equations");
        #endif //LOADDEBUG
        for(i = numberOfFormulae; i > 0; --i) {
          deleteEquation(i - 1);
        }

        #if defined(LOADDEBUG)
          sprintf(line,", loadMode:%d, %s\n",loadMode,tmpString);
          debugPrintf(15, "B", tmpString);
        #endif //LOADDEBUG
        allFormulae = allocC47Blocks(TO_BLOCKS(sizeof(formulaHeader_t)) * formulae);
        numberOfFormulae = formulae;
        currentFormula = 0;
        for(i = 0; i < formulae; i++) {
          allFormulae[i].pointerToFormulaData = C47_NULL;
          allFormulae[i].sizeInBlocks = 0;
        }

        for(i = 0; i < formulae; i++) {
          readLine(tmpString); // One formula
          if(loadMode == LM_ALL || loadMode == LM_PROGRAMS) {
            #if defined(LOADDEBUG)
              sprintf(line,", loadMode:%d, %s\n",loadMode,tmpString);
              debugPrintf(15, "C", tmpString);
            #endif //LOADDEBUG
            utf8ToString((uint8_t *)tmpString, tmpString + TMP_STR_LENGTH / 2);
            setEquation(i, tmpString + TMP_STR_LENGTH / 2);
          }
        }
        if(loadedVersion < 10000021) {  // Old constant format, need to update constants in equation with # prefix
          _updateConstantsInEquations();
        }
      }
    }

    else if(strcmp(tmpString, "OTHER_CONFIGURATION_STUFF") == 0) {
      if(loadMode == LM_ALL || loadMode == LM_SYSTEM_STATE) {
        #if defined(LOADDEBUG)
          sprintf(line,", loadMode:%d, %s\n",loadMode,tmpString);
          debugPrintf(16, "A", tmpString);
        #endif //LOADDEBUG
      }
      readLine(tmpString);                           // Read number params is not used anymore, reading until end of file or "END_OTHER_PARAM"; leaving it to read the old parameter in old files
      //numberOfRegs = toInt16(tmpString);           // Number of regs or params is not used anymore

      readLine(aimBuffer); //param                   // Reading duplicated OTHER_CONFIGURATION_STUFF
      if(strcmp(aimBuffer,"END_OTHER_PARAM") == 0) { // This is used for short form key-only state files, which specify that a setting reset is not to occur
        #if defined(LOADDEBUG)
          debugPrintf(16, "A", "Ending OTHER_CONFIGURATION_STUFF prior to RESET");
        #endif //LOADDEBUG
        goto END_CONFIG;                             // If this is END_OTHER_PARAM, loading is aborted before the config RESET, got break
      } else {
        resetOtherConfigurationStuff(allowUserKeys); // Ensure all the configuration stuff below is reset prior to loading. That ensures if missing settings, that the proper defaults are set.
      }
      readLine(tmpString); //value 00                // Reading second (duplicated 00)

      i = 0;
      while(i < 255) {                                                           //adjust for absolute maximum number of OTHER CONFIGUARTION SETTINGS
        readLine(aimBuffer); // param
        if(strcmp(aimBuffer,"END_OTHER_PARAM") == 0 || aimBuffer[0] == 0) break; //to END_CONFIG break the reading loop for end of file (zero length read, or in all later files END_OTHER_PARAM)
        readLine(tmpString); // value
        if(loadMode == LM_ALL || loadMode == LM_SYSTEM_STATE) {
          #if defined(LOADDEBUG)
            sprintf(line,", loadMode:%d, %s\n",loadMode,tmpString);
            char aa[333];
            sprintf(aa,"B|%u|%s",i,aimBuffer);
            debugPrintf(16, aa, tmpString);
          #endif //LOADDEBUG

          uint8_t fgLN = 255;

          if(strcmp(aimBuffer, "firstGregorianDay") == 0) {
            firstGregorianDay = toUint32(tmpString);
          }
          else if(strcmp(aimBuffer, "denMax") == 0) {
            denMax = toUint32(tmpString);
            if(denMax == 1 || denMax > MAX_DENMAX) {
              denMax = MAX_DENMAX;
            }
          }
          else if(strcmp(aimBuffer, "lastDenominator") == 0) {
            lastDenominator = toUint32(tmpString);
            if(lastDenominator < 1 || lastDenominator > MAX_DENMAX) {
              lastDenominator = 4;
            }
          }
          else if(strcmp(aimBuffer, "displayFormat"               ) == 0) { displayFormat       = toUint8(tmpString);   }
          else if(strcmp(aimBuffer, "displayFormatDigits"         ) == 0) { displayFormatDigits = toUint8(tmpString);   }
          else if(strcmp(aimBuffer, "timeDisplayFormatDigits"     ) == 0) { timeDisplayFormatDigits = toUint8(tmpString); }
          else if(strcmp(aimBuffer, "shortIntegerWordSize"        ) == 0) { shortIntegerWordSize = toUint8(tmpString);  }
          else if(strcmp(aimBuffer, "shortIntegerMode"            ) == 0) { shortIntegerMode     = toUint8(tmpString);  }
          else if(strcmp(aimBuffer, "significantDigits"           ) == 0) { significantDigits    = toUint8(tmpString);  }
          else if(strcmp(aimBuffer, "fractionDigits"              ) == 0) { fractionDigits       = toUint8(tmpString);  }
          else if(strcmp(aimBuffer, "currentAngularMode"          ) == 0) { currentAngularMode   = toUint8(tmpString);  }
          else if(strcmp(aimBuffer, "groupingGap"                 ) == 0) { //backwards compatible loading old config files
            configCommon(CFG_DFLT);
            grpGroupingLeft = toUint8(tmpString);                     //Changed from groupingGap to remain compatible
            grpGroupingRight = grpGroupingLeft;
          }
          else if(strcmp2(aimBuffer, "gapItemLeft"                ) == 0) { gapItemLeft          = toUint16(tmpString); }            //This is to correct a bug in version 00000005-6, to be compatible to the old files
          else if(strcmp2(aimBuffer, "gapItemRight"               ) == 0) { gapItemRight         = toUint16(tmpString); }            //This is to correct a bug in version 00000005-6, to be compatible to the old files
          else if(strcmp2(aimBuffer, "gapItemRadix"               ) == 0) { gapItemRadix         = toUint16(tmpString); }            //This is to correct a bug in version 00000005-6, to be compatible to the old files
          else if(strcmp2(aimBuffer, "lastCenturyHighUsed"        ) == 0) { lastCenturyHighUsed  = toUint16(tmpString); }            //This is to correct a bug in version 00000005-6, to be compatible to the old files
          else if(strcmp2(aimBuffer, "grpGroupingLeft"            ) == 0) { grpGroupingLeft      = toUint8(tmpString);  }            //This is to correct a bug in version 00000005-6, to be compatible to the old files
          else if(strcmp2(aimBuffer, "grpGroupingGr1LeftOverflow" ) == 0) { grpGroupingGr1LeftOverflow = toUint8(tmpString);  }      //This is to correct a bug in version 00000005-6, to be compatible to the old files
          else if(strcmp2(aimBuffer, "grpGroupingGr1Left"         ) == 0) { grpGroupingGr1Left   = toUint8(tmpString);  }            //This is to correct a bug in version 00000005-6, to be compatible to the old files
          else if(strcmp2(aimBuffer, "grpGroupingRight"           ) == 0) { grpGroupingRight     = toUint8(tmpString);  }            //This is to correct a bug in version 00000005-6, to be compatible to the old files
          else if(strcmp(aimBuffer, "roundingMode"                ) == 0) { roundingMode         = toUint8(tmpString);  }
          else if(strcmp(aimBuffer, "displayStack"                ) == 0) { displayStack         = toUint8(tmpString);  }
          else if(strcmp(aimBuffer, "rngState"                    ) == 0) {
            pcg32_global.state = stringToUint64(tmpString);
            str = next_word(tmpString);
            pcg32_global.inc = stringToUint64(str);
          }
          else if(strcmp(aimBuffer, "exponentLimit"               ) == 0) { exponentLimit         = toInt16(tmpString); }
          else if(strcmp(aimBuffer, "exponentHideLimit"           ) == 0) { exponentHideLimit     = toInt16(tmpString); }
          else if(strcmp(aimBuffer, "notBestF"                    ) == 0) { lrSelection           = toUint16(tmpString);}
          else if(strcmp(aimBuffer, "bestF"                       ) == 0) { lrSelection           = toUint16(tmpString);}
          else if(strcmp(aimBuffer, "fgLN" ) == 0 || strcmp(aimBuffer, "jm_FG_LINE" ) == 0) {                                        //This section to deal with any old states containing the old FG system
            fgLN = convert001090400T001090500(toUint8(tmpString),RBX_FGLNOFF);
            if(fgLN == RBX_FGLNOFF) {
              clearSystemFlag(FLAG_FGLNLIM);
              clearSystemFlag(FLAG_FGLNFUL);
            } else     if(fgLN == RBX_FGLNLIM) {
              setSystemFlag(FLAG_FGLNLIM);
              clearSystemFlag(FLAG_FGLNFUL);
            } else     if(fgLN == RBX_FGLNFUL) {
              clearSystemFlag(FLAG_FGLNLIM);
              setSystemFlag(FLAG_FGLNFUL);
            }
          }             //Keep compatible with old setting
          else if(strcmp(aimBuffer, "HOME3"                       ) == 0) {
            if(loadedVersion < 10000022) {
              forceSystemFlag(FLAG_HOME_TRIPLE, toUint8(tmpString) != 0);
              setLongPressFg(calcModel, -MNU_HOME);
            } //Keep compatible by repeating, even though setting is now in systemflags
          }
          else if(strcmp(aimBuffer, "MYM3"                        ) == 0) {
            if(loadedVersion < 10000022) {
              forceSystemFlag(FLAG_MYM_TRIPLE, toUint8(tmpString) != 0);
              setLongPressFg(calcModel, -MNU_MyMenu);
            } //Keep compatible by repeating, even though setting is now in systemflags
          }
          else if(strcmp(aimBuffer, "dispBase"                    ) == 0) { dispBase              = toUint8(tmpString); }
          else if(strcmp(aimBuffer, "ShiftTimoutMode"             ) == 0) {
            if(loadedVersion < 10000022) {
              forceSystemFlag(FLAG_SHFT_4s, toUint8(tmpString) != 0);
            } //Keep compatible by repeating, even though setting is now in systemflags
          }
          else if(strcmp(aimBuffer, "SH_BASE_HOME"                ) == 0) {//Keep compatible with old name by repeating it
            if(loadedVersion < 10000022) {
              forceSystemFlag(FLAG_BASE_HOME, toUint8(tmpString) != 0);
            } //Keep compatible by repeating, even though setting is now in systemflags
          }
          else if(strcmp(aimBuffer, "BASE_HOME"                   ) == 0) {
            if(loadedVersion < 10000022) {
              forceSystemFlag(FLAG_BASE_HOME, toUint8(tmpString) != 0);
            } //Keep compatible by repeating, even though setting is now in systemflags
          }
          else if(allowUserKeys && (strcmp(aimBuffer, "Norm_Key_00_VAR"             ) == 0)) {
            // Old state file, before changing Norm_Key_00_VAR to the Norm_Key_00 structure
            if(Norm_Key_00_key != -1) {
              Norm_Key_00.func  = toUint16(tmpString);   // only the function is restored, assuming no param
              Norm_Key_00.used  = Norm_Key_00.func != kbd_std[Norm_Key_00_key].primary;
            } else {
              Norm_Key_00.used = false;
            }
          }
          else if(allowUserKeys && (strcmp(aimBuffer, "Norm_Key_00.func"            ) == 0)) { Norm_Key_00.func      = toUint16(tmpString); }
          else if(strcmp(aimBuffer, "Norm_Key_00.funcParam"       ) == 0) {      //  Workaround keeping old state files and new state files working, due to a blank string possibility which breaks the loading (on Mac sim at least).
              if(strcmp(tmpString, "Norm_Key_00.used") == 0) {                     //check if the next setting is erroneously read as data for the text data string 'funcParam'. In the old state file, a blank string was saved as param, which causes the single line read to fail, and the next setting name read as data.
                  if(allowUserKeys) {
                    Norm_Key_00.funcParam[0]=0;                                    //  - old file compatibility: If next setting name is found as data, clear it.
                    Norm_Key_00.used = 0;                                          //  - populate the the next setting to default 0,  as the read has already currupted the sequence
                  }
                  readLine(tmpString);                                             //  - read the next data line as a dummy and throw away, as it also has corrupted the sequence
              } else if(allowUserKeys &&
                      (strcmp(tmpString, "NoNormKeyParamDef") == 0)) {             //if no data sequence corrution, check for the new keyword for a blank stirng. Note the keyword is longer than the 16 chars max of param strings. Hence the 'NoNormKeyParamDef' is unique and cannot be data.
                  Norm_Key_00.funcParam[0]=0;                                      //  - if the code word for a blank string, blank the string.
              } else if(allowUserKeys) {                                           //  - New state files will have 'NoNormKeyParamDef' if no NRM+ XEQ paramater is present.
                  strcpy(Norm_Key_00.funcParam,tmpString);                         //Otherwise proceed and use the data as normal
              }
          }
          else if(allowUserKeys && (strcmp(aimBuffer, "Norm_Key_00.used"            ) == 0)) { Norm_Key_00.used      = toUint8(tmpString) != 0; }

          else if(allowUserKeys && (strcmp(aimBuffer, "calcModel"                   ) == 0)) {
            uint16_t calcModelRead = toUint16(tmpString);
            if(savedCalcModel == USER_R47 && (calcModelRead == USER_R47f_g || calcModelRead == USER_R47fg_g || calcModelRead == USER_R47fg_bk || calcModelRead == USER_R47bk_fg)) {
              calcModel = calcModelRead;
            } else
            if(savedCalcModel == USER_C47 && (calcModelRead == USER_C47    || calcModelRead == USER_DM42 )) {
              calcModel = calcModelRead;
            }
          }

          else if(strcmp(aimBuffer, "Input_Default"               ) == 0) { Input_Default         = toUint8(tmpString); }
          else if(strcmp(aimBuffer, "jm_BASE_SCREEN"              ) == 0) {        //Keep compatible by repeating
            if(loadedVersion < 10000022) {
              forceSystemFlag(FLAG_BASE_MYM, toUint8(tmpString) != 0);
            } //Keep compatible by repeating, even though setting is now in systemflags
          }
          else if(strcmp(aimBuffer, "BASE_MYM"                    ) == 0) {
            if(loadedVersion < 10000022) {
              forceSystemFlag(FLAG_BASE_MYM, toUint8(tmpString) != 0);
            } //Keep compatible by repeating, even though setting is now in systemflags
          }
          else if(strcmp(aimBuffer, "jm_G_DOUBLETAP"              ) == 0) {
            if(loadedVersion < 10000022) {
              forceSystemFlag(FLAG_G_DOUBLETAP, toUint8(tmpString) != 0);
            } //Keep compatible by repeating, even though setting is now in systemflags
          }

          else if(strcmp(aimBuffer, "displayStackSHOIDISP"        ) == 0) { displayStackSHOIDISP  = toUint8(tmpString); }
          else if(strcmp(aimBuffer, "bcdDisplay"                  ) == 0) {
            if(loadedVersion < 10000020) {
              forceSystemFlag(FLAG_BCD, toUint8(tmpString) != 0);
            } //Keep compatible by repeating, even though setting is now in systemflags
          }
          else if(strcmp(aimBuffer, "topHex"                      ) == 0) {
            if(loadedVersion < 10000019) {
              forceSystemFlag(FLAG_TOPHEX, toUint8(tmpString) != 0);
            } //Keep compatible by repeating, even though setting is now in systemflags
          }
          else if(strcmp(aimBuffer, "bcdDisplaySign"              ) == 0) { bcdDisplaySign        = convert001090400T001090500(toUint8(tmpString),BCDu); }
          else if(strcmp(aimBuffer, "DRG_Cycling"                 ) == 0) { DRG_Cycling           = toUint8(tmpString); }
          else if(strcmp(aimBuffer, "DM_Cycling"                  ) == 0) { DM_Cycling            = toUint8(tmpString); }
          else if(strcmp(aimBuffer, "LongPressM"                  ) == 0) { LongPressM            = convert001090400T001090500(toUint8(tmpString),RBX_M14); }                  //10000003
          else if(strcmp(aimBuffer, "LongPressF"                  ) == 0) { LongPressF            = convert001090400T001090500(toUint8(tmpString),RBX_F14); }                  //10000003
          else if(strcmp(aimBuffer, "lastIntegerBase"             ) == 0) { lastIntegerBase       = toUint8(tmpString); }                  //10000004
          else if(strcmp(aimBuffer, "lrChosen"                    ) == 0) { lrChosen              = toUint16(tmpString);}
          else if(strcmp(aimBuffer, "graph_dx"                    ) == 0) { graph_dx              = stringToFloat(tmpString); }
          else if(strcmp(aimBuffer, "graph_dy"                    ) == 0) { graph_dy              = stringToFloat(tmpString); }
          else if(strcmp(aimBuffer, "roundedTicks"                ) == 0) { roundedTicks          = toUint8(tmpString) != 0; }
          else if(strcmp(aimBuffer, "PLOT_INTG"                   ) == 0) { PLOT_INTG             = toUint8(tmpString) != 0; }
          else if(strcmp(aimBuffer, "PLOT_DIFF"                   ) == 0) { PLOT_DIFF             = toUint8(tmpString) != 0; }
          else if(strcmp(aimBuffer, "PLOT_RMS"                    ) == 0) { PLOT_RMS              = toUint8(tmpString) != 0; }
          else if(strcmp(aimBuffer, "PLOT_SHADE"                  ) == 0) { PLOT_SHADE            = toUint8(tmpString) != 0; }
          else if(strcmp(aimBuffer, "PLOT_AXIS"                   ) == 0) { PLOT_AXIS             = toUint8(tmpString) != 0; }
          else if(strcmp(aimBuffer, "PLOT_ZMY"                    ) == 0) { PLOT_ZMY              = toUint8(tmpString); }
          else if(strcmp(aimBuffer, "firstDayOfWeek"              ) == 0) { firstDayOfWeek        = toUint8(tmpString); }
          else if(strcmp(aimBuffer, "firstWeekOfYearDay"          ) == 0) { firstWeekOfYearDay    = toUint8(tmpString); }
          else if(strcmp(aimBuffer, "jm_LARGELI"                  ) == 0) {
            if(loadedVersion < 10000012) {
              forceSystemFlag(FLAG_LARGELI, toUint8(tmpString) != 0);
            } //Keep compatible by repeating, even though setting is now in systemflags
          }
          else if(strcmp(aimBuffer, "constantFractions"           ) == 0) {
            if(loadedVersion < 10000012) {
              forceSystemFlag(FLAG_IRFRQ, toUint8(tmpString) != 0);
            } //Keep compatible by repeating, even though setting is now in systemflags
          }
          else if(strcmp(aimBuffer, "constantFractionsOn"         ) == 0) {
            if(loadedVersion < 10000012) {
              forceSystemFlag(FLAG_IRFRAC, toUint8(tmpString) != 0);
            } //Keep compatible by repeating, even though setting is now in systemflags
          }
          else if(strcmp(aimBuffer, "eRPN"                        ) == 0) {
            if(loadedVersion < 10000012) {
              forceSystemFlag(FLAG_ERPN, toUint8(tmpString) != 0);
            } //Keep compatible by repeating, even though setting is now in systemflags
          }
          else if(strcmp(aimBuffer, "CPXMult"                     ) == 0) {
            if(loadedVersion < 10000012) {
              forceSystemFlag(FLAG_CPXMULT, toUint8(tmpString) != 0);
            } //Keep compatible by repeating, even though setting is now in systemflags
          }
          else if(strcmp(aimBuffer, "SI_All"                      ) == 0) {
            if(loadedVersion < 10000012) {
              forceSystemFlag(FLAG_PFX_ALL, toUint8(tmpString) != 0);
            } //Keep compatible by repeating, even though setting is now in systemflags
          }



          hourGlassIconEnabled = false;

        }
      i++;
      }
END_CONFIG:
      hourGlassIconEnabled = false;
      return false; //Signal that this was the last section loaded and no more sections to follow
      #if defined(LOADDEBUG)
        debugPrintf(17, "END1", "");
      #endif // LOADDEBUG
    }
    hourGlassIconEnabled = false;
    return true; //Signal to continue loading the next section
    #if defined(LOADDEBUG)
      debugPrintf(18, "END2", "");
    #endif // LOADDEBUG
    clearScreen(211); // implicit forceSBupdate();
  }
#endif // !TESTSUITE_BUILD




void doLoad(uint16_t loadMode, uint16_t s, uint16_t n, uint16_t d, uint16_t loadType) {
  #if !defined(TESTSUITE_BUILD)
  savedCalcModel = 0;
  ioFilePath_t path;
  int ret;
  #if defined(LOADDEBUG)
    char yy[1000];
    sprintf(yy, "%d",loadMode);
    debugPrintf(-1, "LoadMode", yy);
  #endif // LOADDEBUG

  if(loadType == autoLoad) {
    path = ioPathAutoSave;
  }
  else if(loadType == manualLoad) {
    path = ioPathManualSave;
  }
  else {
    path = ioPathLoadStateFile;
  }

  #if defined(LOADDEBUG)
    sprintf(yy, "%u",path);
    debugPrintf(-1, "Path:", yy);
  #endif // LOADDEBUG


  ret = ioFileOpen(path, ioModeRead);

  if(ret != FILE_OK ) {
    if(ret == FILE_CANCEL ) {
      return;
    }
    else {
      displayCalcErrorMessage(ERROR_CANNOT_READ_FILE, ERR_REGISTER_LINE, REGISTER_X);
      return;
    }
  }

  if(loadMode == LM_ALL) {
    while(currentSubroutineLevel > 0) {
      fnReturn(0);
    }
  }


  // SAVE_FILE_REVISION
  // 0
  // C47_save_file_00 or R47_save_file_00
  // 10000003

  // Allow older versions for autoloaded sav file
  //  while doing no check on manual loading. This may allow manual loading of older files at risk
  loadedVersion = 0;
  {
    readLine(tmpString);
    if(strcmp(tmpString, "SAVE_FILE_REVISION") == 0) {
      readLine(aimBuffer); // internal rev number (ignore now)
      readLine(aimBuffer); // param
      readLine(tmpString); // value

      // Determine on which calculator the file was saved
      if(strcmp(aimBuffer, "C47_save_file_00") == 0) {
        savedCalcModel = USER_C47;
      }
      else if(strcmp(aimBuffer, "R47_save_file_00") == 0) {
        savedCalcModel = USER_R47;
      }

      if(savedCalcModel == USER_C47 || savedCalcModel == USER_R47) {
        loadedVersion = toUint32(tmpString);
        if(loadedVersion < 10000000 || loadedVersion > 20000000) {
          loadedVersion = 0;
        }
      }
    }
  }

  bool_t enableLoad = false;
  switch(loadType) {
    case manualLoad: {
      switch(loadMode) {
        case LM_ALL: {
          enableLoad = true;
        }
        break;
        case LM_PROGRAMS:
        case LM_REGISTERS:
        case LM_SYSTEM_STATE:
        case LM_SUMS:
        case LM_NAMED_VARIABLES: {
          enableLoad = true;
        }
        break;
        default:break;
      }
    }
    break;
    case stateLoad: {
      if (loadMode == LM_ALL) {
        enableLoad = true;
      }
    }
    break;
    case autoLoad: {
      if (((loadMode == LM_ALL) && (loadedVersion >= VersionAllowed) && (loadedVersion <= configFileVersion)) ){
        enableLoad = true;
      }
    }
    break;
    default:break;
  }
  if(enableLoad) {
    bool_t allowUserKeys = false;
    #if CALCMODEL == USER_C47
      allowUserKeys = (savedCalcModel == USER_C47);
    #elif CALCMODEL == USER_R47
      allowUserKeys = (savedCalcModel == USER_R47);
    #endif  // CALCMODEL
    while(restoreOneSection(loadMode, s, n, d, allowUserKeys)) {
    }

    // Set the user primary functions for the R47 yellow and blue shift keys to their standard default value
    //   to avoid discrepancies after loading key assignments
    if(calcModel == USER_R47f_g || calcModel == USER_R47fg_g || calcModel == USER_R47fg_bk || calcModel == USER_R47bk_fg) {
      for(int i = 10; i <= 11; i++) {        // R47 Yellow and Blue Shift keys
        kbd_usr[i].primary    = kbd_std[i].primary;
        kbd_usr[i].keyLblAim  = kbd_std[i].keyLblAim;
        kbd_usr[i].primaryAim = kbd_std[i].primaryAim;
        kbd_usr[i].primaryTam = kbd_std[i].primaryTam;
      }
    }
  }

  lastErrorCode = ERROR_NONE;
  previousErrorCode = lastErrorCode;

  ioFileClose();

    //-------------------------------------------------------------------------------------------------
  // This is where user is informed about versions incompatibilities and changes to loaded data occur
  // The code  below is an example of a version mismatch handling
  // The string passed to show_warning() can be the same if it fits on the HW display (7 lines of ~32
  // characters and standard ASCII characters), or two differents strings can used as shown below
  //-------------------------------------------------------------------------------------------------
  //
  //Code example:
  //
  //if(loadMode == LM_ALL) {
  //  if(loadedVersion <= 88) { // Program incompatibility
  //  #if defined(PC_BUILD)
  //    sprintf(tmpString,"****Program binary incompatibility****\n"
  //                      "x→α now followed by a destination register\n"
  //                      "Loaded x→α in RAM will be replaced by NOP\n"
  //                      "CAVEAT: x→α in Flash will not be replaced so it may cause crash\n");
  //  #endif // PC_BUILD
  //  #if defined(DMCP_BUILD)
  //    sprintf(tmpString,"**Program binary incompatibility**\n"
  //                      "x->a now uses a destination register\n"
  //                      "x->a in RAM will be replaced by NOP\n"
  //                      "CAVEAT: x->a in Flash will not be\n"
  //                      "replaced so it may cause crash\n");
  //  #endif // DMCP_BUILD
  //  #if !defined(TESTSUITE_BUILD)
  //    show_warning(tmpString);
  //  #endif // TESTSUITE_BUILD
  //
  //    int globalStep = 1;
  //    uint8_t *step = beginOfProgramMemory;
  //
  //    while(!isAtEndOfPrograms(step)) { // .END.
  //      if(checkOpCodeOfStep(step, ITM_XtoALPHA)) { // x->alpha
  //        step[0] = (ITM_NOP >> 8) | 0x80;
  //        step[1] =  ITM_NOP       & 0xff;
  //        printf("x→α found at global step %d\n", globalStep);
  //      }
  //      ++globalStep;
  //      step = findNextStep(step);
  //    }
  //  }
  //}

  #if defined(DMCP_BUILD)
    sys_timer_disable(TIMER_IDX_REFRESH_SLEEP);
    sys_timer_start(TIMER_IDX_REFRESH_SLEEP,1000);
    fnTimerStart(TO_KB_ACTV, TO_KB_ACTV, TO_KB_ACTV_MEDIUM); //PROGRAM_KB_ACTV
  #endif // DMCP_BUILD


  #if !defined(TESTSUITE_BUILD)
    if(loadType == manualLoad && loadMode == LM_ALL) {
      temporaryInformation = TI_BACKUP_RESTORED;
      getDateString(lastStateFileOpened);
      strcat(lastStateFileOpened,": ");
      stringCopy(lastStateFileOpened + stringByteLength(lastStateFileOpened), fileNameSelected);
    }
    else if((loadType == autoLoad) && (loadedVersion >= VersionAllowed) && (loadedVersion <= configFileVersion) && (loadMode == LM_ALL)) {
      temporaryInformation = TI_BACKUP_RESTORED;
      getDateString(lastStateFileOpened);
      strcat(lastStateFileOpened,": ");
      stringCopy(lastStateFileOpened + stringByteLength(lastStateFileOpened), fileNameSelected);
    }
    else if(loadType == stateLoad) {
      temporaryInformation = TI_STATEFILE_RESTORED;
      getDateString(lastStateFileOpened);
      strcat(lastStateFileOpened,": ");
      stringCopy(lastStateFileOpened + stringByteLength(lastStateFileOpened), fileNameSelected);
    }
    else if(loadMode == LM_PROGRAMS) {
      temporaryInformation = TI_PROGRAMS_RESTORED;
    }
    else if(loadMode == LM_REGISTERS) {
      temporaryInformation = TI_REGISTERS_RESTORED;
    }
    else if(loadMode == LM_REGISTERS) {
      temporaryInformation = TI_REGISTERS_RESTORED;
    }
    else if(loadMode == LM_SYSTEM_STATE) {
      temporaryInformation = TI_SETTINGS_RESTORED;
    }
    else if(loadMode == LM_SUMS) {
      temporaryInformation = TI_SUMS_RESTORED;
    }
    else if(loadMode == LM_NAMED_VARIABLES) {
      temporaryInformation = TI_VARIABLES_RESTORED;
    }
    cachedDynamicMenu = 0;
  #endif // !TESTSUITE_BUILD
#endif // !TESTSUITE_BUILD
}



void fnLoad(uint16_t loadMode) {
  printStatus(0, errorMessages[LOADING_STATE_FILE],force);
  if(loadMode == LM_STATE_LOAD) {
    doLoad(LM_ALL, 0, 0, 0, stateLoad);
  }
  else {
    doLoad(loadMode, 0, 0, 0, manualLoad);
  }
  fnClearFlag(FLAG_USER);
  screenUpdatingMode &= ~SCRUPD_MANUAL_MENU;
  refreshScreen(94);
}

void fnLoadAuto(void) {
  doLoad(LM_ALL, 0, 0, 0, autoLoad);
  fnClearFlag(FLAG_USER);
  screenUpdatingMode &= ~SCRUPD_MANUAL_MENU;
  refreshScreen(95);
}


void fnLoadedFile (uint16_t unusedButMandatoryParameter) {
  liftStack();

  int16_t len = stringByteLength(lastStateFileOpened) + 1;
  reallocateRegister(REGISTER_X, dtString, TO_BLOCKS(len), amNone);
  xcopy(REGISTER_STRING_DATA(REGISTER_X), lastStateFileOpened , len);
  temporaryInformation = TI_LASTSTATEFILE;
}



#undef BACKUP

void fnDeleteBackup(uint16_t confirmation) {
  if(confirmation == NOT_CONFIRMED) {
    setConfirmationMode(fnDeleteBackup);
  }
  else {
    #if defined(DMCP_BUILD)
      FRESULT result;
      sys_disk_write_enable(1);
      result = f_unlink("SAVFILES\\C47.sav");
      if(result != FR_OK && result != FR_NO_FILE && result != FR_NO_PATH) {
        displayCalcErrorMessage(ERROR_IO, ERR_REGISTER_LINE, REGISTER_X);
      }
      result = f_unlink("SAVFILES\\C47auto.sav");
      if(result != FR_OK && result != FR_NO_FILE && result != FR_NO_PATH) {
        displayCalcErrorMessage(ERROR_IO, ERR_REGISTER_LINE, REGISTER_X);
      }
      sys_disk_write_enable(0);
    #else // !DMCP_BUILD
      int result = remove("SAVFILES/C47.sav");
      if(result == -1) {
        #if !defined(TESTSUITE_BUILD)
          int e = errno;
          if(e != ENOENT) {
            displayCalcErrorMessage(ERROR_IO, ERR_REGISTER_LINE, REGISTER_X);
            #if (EXTRA_INFO_ON_CALC_ERROR == 1)
              sprintf(errorMessage, "removing the backup failed with error code %d", e);
              moreInfoOnError("In function fnDeleteBackup:", errorMessage, NULL, NULL);
            #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          }
        #endif // !TESTSUITE_BUILD
      }
    #endif // DMCP_BUILD
  }
}

