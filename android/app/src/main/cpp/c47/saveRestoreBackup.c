// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file saveRestoreBackup.c
 * Simulator backup.cfg subsystem: saveCalc()/restoreCalc() and their private helpers.
 * Split out of saveRestoreCalcState.c. PC_BUILD (simulator) only.
 */

#include "c47.h"

// This is used for the backup.cfg simulator backup file
// The variable backupVersion is used in the connection
#define BACKUP_VERSION                     1015     // FLAG_SIGZEROS
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

#define backupFileName (CALCMODEL == USER_C47 ? "backup.cfg" : "backupR47.cfg")

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
    #if defined(PC_BUILD)
      printf("----------------R%2u Old matrix header: r=%i c=%i \n", regist, (REGISTER_MATRIX_HEADER (regist)->matrixRows), (REGISTER_MATRIX_HEADER (regist)->matrixColumns));
    #endif //PC_BUILD
    uint32_t row = (REGISTER_MATRIX_HEADER(regist)->matrixRows) & 0x0FFF;
    uint32_t col = ((REGISTER_MATRIX_HEADER(regist)->matrixColumns) >> 4) & 0x0FFF;
    #if defined(PC_BUILD)
      printf("----------------R%2u New matrix header: r=%i c=%i \n", regist, row, col);
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
        realToString((real_t *)buffer, value + strlen(value));
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
        real34ToString((real34_t *)buffer, value + strlen(value));
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
    saveStateValue(lastStateFileOpened,             sizeof(lastStateFileOpened),                                 "lastStateFileOpened",            "hexDump");

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
    saveStateValue(&numScreensNumericFontBold,      sizeof(numScreensNumericFontBold),                           "numScreensNumericFontBold",      "uint8");
    saveStateValue(&numLinesNumericFont,            sizeof(numLinesNumericFont),                                 "numLinesNumericFont",            "uint8");
    saveStateValue(&numLinesNumericFontBold,        sizeof(numLinesNumericFontBold),                             "numLinesNumericFontBold",        "uint8");
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
    saveStateValue(&amortP1,                        sizeof(amortP1),                                             "amortP1",                        "uint16");
    saveStateValue(&amortP2,                        sizeof(amortP2),                                             "amortP2",                        "uint16");
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
    saveStateValue(&calcModel,                      sizeof(calcModel),                                           "calcModel",                      "uint8");   //JM
    saveStateValue(&printerState.print_on,          sizeof(printerState.print_on),                               "printerState.print_on",          "bool");    //DL
    saveStateValue(&printerState.printer_model,     sizeof(printerState.printer_model),                          "printerState.printer_model",     "uint8");   //DL
    saveStateValue(&printerState.delay,             sizeof(printerState.delay),                                  "printerState.delay",             "uint16");  //DL
    saveStateValue(&programmableMenu,               sizeof(programmableMenu),                                    "programmableMenu",               "hexDump");

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

    if(loadTestPrograms) { // if test programs loaded. do not continue loading the backup.cfg file
      refreshScreen(9100);
      return;
    }

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
    restoreStateValue(lastStateFileOpened,             sizeof(lastStateFileOpened),                                 "lastStateFileOpened",            "hexDump");

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
    restoreStateValue(&numScreensNumericFontBold,      sizeof(numScreensNumericFontBold),                           "numScreensNumericFontBold",      "uint8");
    restoreStateValue(&numLinesNumericFont,            sizeof(numLinesNumericFont),                                 "numLinesNumericFont",            "uint8");
    restoreStateValue(&numLinesNumericFontBold,        sizeof(numLinesNumericFontBold),                             "numLinesNumericFontBold",        "uint8");
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
    restoreStateValue(&amortP1,                        sizeof(amortP1),                                             "amortP1",                        "uint16");
    restoreStateValue(&amortP2,                        sizeof(amortP2),                                             "amortP2",                        "uint16");
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
    if(backupVersion < 1002 && currentInputVariable == INVALID_VARIABLE_OLD) {
      currentInputVariable = INVALID_VARIABLE;
    }
    restoreStateValue(&SAVED_SIGMA_LASTX,              sizeof(SAVED_SIGMA_LASTX),                                   "SAVED_SIGMA_LASTX",              "real");
    restoreStateValue(&SAVED_SIGMA_LASTY,              sizeof(SAVED_SIGMA_LASTY),                                   "SAVED_SIGMA_LASTY",              "real");
    SAVED_SIGMA_lastAddRem = SIGMA_NONE;
    if(backupVersion <= 1004) {
      restoreStateValue(&SAVED_SIGMA_lastAddRem,         sizeof(SAVED_SIGMA_lastAddRem),                              "SAVED_SIGMA_LAc1",               "int8");     //manual correction as the type allocation was wrong here
    }
    restoreStateValue(&SAVED_SIGMA_lastAddRem,         sizeof(SAVED_SIGMA_lastAddRem),                              "SAVED_SIGMA_lastAddRem",         "int8");     //manual correction as the type allocation was wrong here
    if(SAVED_SIGMA_lastAddRem == -1) {
      SAVED_SIGMA_lastAddRem = SIGMA_MINUS;
    }
    //if(SAVED_SIGMA_lastAddRem == 0 ) SAVED_SIGMA_lastAddRem = SIGMA_NONE;
    //if(SAVED_SIGMA_lastAddRem == 1 ) SAVED_SIGMA_lastAddRem = SIGMA_PLUS;
    //if(SAVED_SIGMA_lastAddRem == 2 ) SAVED_SIGMA_lastAddRem = SIGMA_MINUS;
    restoreStateValue(&currentMvarLabel,               sizeof(currentMvarLabel),                                    "currentMvarLabel",               "uint16");
    if(backupVersion < 1002 && currentMvarLabel == INVALID_VARIABLE_OLD) {
      currentMvarLabel = INVALID_VARIABLE;
    }
    restoreStateValue(&graphVariabl1,                  sizeof(graphVariabl1),                                       "graphVariabl1",                  "int16");
    if(backupVersion < 1002 && graphVariabl1 == INVALID_VARIABLE_OLD) {
      graphVariabl1 = INVALID_VARIABLE;
    }
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
    fgLN = convert001090400T001090500(fgLN, RBX_FGLNOFF);
    if(fgLN == RBX_FGLNOFF) {                                                                                //This section to deal with any old states containing the old FG system
      clearSystemFlag(FLAG_FGLNLIM);
      clearSystemFlag(FLAG_FGLNFUL);
    }
    else if(fgLN == RBX_FGLNLIM) {
      setSystemFlag(FLAG_FGLNLIM);
      clearSystemFlag(FLAG_FGLNFUL);
    }
    else if(fgLN == RBX_FGLNFUL) {
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
    if(bcdDisplay) {
      setSystemFlag(FLAG_BCD);
    }
    else {
      clearSystemFlag(FLAG_BCD);
    }
    restoreStateValue(&bcdDisplaySign,                 sizeof(bcdDisplaySign),                                      "bcdDisplaySign",                 "uint8");   //C47 JM
    bcdDisplaySign = convert001090400T001090500(bcdDisplaySign, BCDu);
    restoreStateValue(&DM_Cycling,                     sizeof(DM_Cycling),                                          "DM_Cycling",                     "uint8");   //JM
    restoreStateValue(&LongPressM,                     sizeof(LongPressM),                                          "LongPressM",                     "uint8");   //JM
    LongPressM = convert001090400T001090500(LongPressM, RBX_M14);
    restoreStateValue(&LongPressF,                     sizeof(LongPressF),                                          "LongPressF",                     "uint8");   //JM
    LongPressF = convert001090400T001090500(LongPressF, RBX_F14);
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
    printerState.print_on = false;
    restoreStateValue(&printerState.print_on,          sizeof(printerState.print_on),                               "printerState.print_on",          "bool");    //DL
    printerState.trace_done = false;
    printerState.print_blank_line = 0;
    printerState.print_mode = PMODE_DEFAULT;
    printerState.printer_model = PRINTER_HP;
    restoreStateValue(&printerState.printer_model,     sizeof(printerState.printer_model),                          "printerState.printer_model",     "uint8");   //DL
    printerState.delay = getLineDelay();
    restoreStateValue(&printerState.delay,             sizeof(printerState.delay),                                  "printerState.delay",             "uint16");  //DL
    restoreStateValue(&programmableMenu,               sizeof(programmableMenu),                                    "programmableMenu",               "hexDump");

    if(backupVersion < 1014) {
      setLongPressFg(calcModel, -MNU_HOME);
    }
    if(backupVersion < 1015) {
      setSystemFlag(FLAG_SIGZEROS); //SIGZEROS is on per default
    }
    // Ensure valid relations between FLAG_FRACT, FLAG_IRFRAC and FLAG_IRFRQ
    if(getSystemFlag(FLAG_FRACT)) {
      setSystemFlag(FLAG_FRACT);
    }
    else if(getSystemFlag(FLAG_IRFRAC)) {
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
      while(qq <= LAST_GLOBAL_REGISTER) {
        convertOldMatrixHeaderToNewMatrixHeader(qq);
        qq++;
      }
      qq = FIRST_NAMED_VARIABLE;
      while(qq <= LAST_NAMED_VARIABLE) {
        convertOldMatrixHeaderToNewMatrixHeader(qq);
        qq++;
      }
      qq = FIRST_LOCAL_REGISTER;
      while(qq <= LAST_LOCAL_REGISTER) {
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

    printf("End of calc's restoration\n");
    fflush(stdout);

    if(temporaryInformation == TI_SHOW_REGISTER_BIG || temporaryInformation == TI_SHOW_REGISTER_SMALL || temporaryInformation == TI_SHOW_REGISTER_TINY || temporaryInformation==TI_SHOW_REGISTER) {
      temporaryInformation = TI_NO_INFO;
    }

    scanLabelsAndPrograms();
    defineCurrentProgramFromGlobalStepNumber(currentLocalStepNumber + abs(programList[currentProgramNumber - 1].step) - 1);
    defineCurrentStep();
    defineFirstDisplayedStep();
    defineCurrentProgramFromCurrentStep();

    //defineCurrentLocalRegisters();

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
