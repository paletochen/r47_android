// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"

// This is used for the state files
#define configFileVersion                  10000024 // FLAG_SIGZEROS
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


#define LOADDEBUG
#undef LOADDEBUG


uint16_t flushBufferCnt = 0;
#define START_REGISTER_VALUE 860  // 2025/09/06 [DL] reduced fromm 1000 to provide enougth room in tmpRegisterString for config data (840 bytes):
                                  //                 tmpRegisterString is a part of the global tmpString which is 2560 bytes (aux_buf_ptr buffer provided by DMCP)
                                  //                 config string length is 1680 bytes (840 x 2) so tmpRegisterString should start at max at 880 (2560 - 1680)
                                  //                 THIS VALUE NEEDS TO BE RE-EVALUATED IF THE CONFIG DATA LENGTH IS INCREASED
static uint32_t loadedVersion = 0;
static char *tmpRegisterString = NULL;

// When true, complex & complex matrix are written/read in the "(3-i4)" form (data files)
// When false, the backward compatible "re im" form is used. Set/cleared by doSaveDataFile() and doLoadDataFile()
static bool_t dataFileMode = false;

void save(const void *buffer, uint32_t size) {
  ioFileWrite(buffer, size);
}

static uint32_t restore(void *buffer, uint32_t size) {
  return ioFileRead(buffer, size);
}


uint8_t convert001090400T001090500(uint8_t parameter, uint8_t offset) { //Example: read from file: RB_F14 (which was 0) and and report RBX_F14 (which is 221) to the program.
  uint8_t output = parameter;
  if(parameter < 20) {
    output = parameter + offset;
  }
  return output;
}

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

// Lettered-register names for registers FIRST_LETTERED_REGISTER..LAST_SPARE_REGISTER (100..125), in register-number order:
//   100=X 101=Y 102=Z 103=T 104=A 105=B 106=C 107=D 108=L 109=I 110=J 111=K 112=M 113=N 114=P 115=Q 116=R 117=S 118=E 119=F 120=G 121=H 122=O 123=U 124=V 125=W
static const char registerLetters[] = "XYZTABCDLIJKMNPQRSEFGHOUVW";

// Parse a register-name field of the form "Rnnn" (decimal, e.g. "R000", "R100") or the lettered short form "RX".."RW"
// (registers 100..125). Anything else, or a leading 'R' followed by a digit, is read as the decimal register number,
// so existing files written as "Rnnn" remain fully compatible.
static calcRegister_t stringToRegisterNumber(const char *name) {
  if(name[0] == 'R' && name[1] != 0 && (name[1] < '0' || name[1] > '9')) {
    const char *letter = strchr(registerLetters, name[1]);
    if(letter != NULL) {
      return (calcRegister_t)(FIRST_LETTERED_REGISTER + (letter - registerLetters));
    }
  }
  return (calcRegister_t)toInt16(name + 1);
}

// Build a register-name field for writing: the lettered short form "RX".."RW" for registers 100..125, otherwise "Rnnn".
static void registerNumberToString(calcRegister_t regist, char *name) {
  if(regist >= FIRST_LETTERED_REGISTER && regist <= LAST_SPARE_REGISTER) {
    sprintf(name, "R%c", registerLetters[regist - FIRST_LETTERED_REGISTER]);
  }
  else {
    sprintf(name, "R%03" PRId16, (int16_t)regist);
  }
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
  while(*str != ' ' && *str != 0) {
    str++;
  }
  return (char *)str;
}

static char *skip_space(const char *str) {
  while(*str == ' ') {
    str++;
  }
  return (char *)str;
}

static char *next_word(const char *str) {
  return skip_word(skip_space(str));
}

static char *skip_to_space_newline(const char *str) {
  while(*str != ' ' && *str != '\n' && *str != 0) {
    str++;
  }
  return (char *)str;
}

static char *toInt16_next_word(const char *str, int16_t *val) {
  *val = toInt16(str);
  return next_word(str);
}

void _updateConstantsInEquations(void) {
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
//#endif

#define isXFN true
char aimBuffer1[400];             //The concurrent use of the global aimBuffer
                                  //does not work. See tmpString.
                                  //Temporary solution is to use a local variable of sufficient length for the target.

  static void UI64toString(uint64_t value, char * tmpRegisterString);
  static void registerToSaveString(calcRegister_t regist, bool_t isXFNRegister) {
    longInteger_t lgInt;
    int16_t sign;
    uint64_t value;
    uint32_t base;
    char *str;
    uint8_t *cfg;

    tmpRegisterString = tmpString + START_REGISTER_VALUE;


#if defined(OPTION_XFN_1000)
    if(isXFNRegister) {
      if(registerFMAOutputPlainString(regist, "", tmpRegisterString)) {
        strcpy(aimBuffer1, "RXFN");
        angularMode_t am;
        if(getAngleModeForRegister3r(regist, &am)) {
          textTag(aimBuffer1, am, 0);
        }
      } else {
        aimBuffer1[0] = 0;
        tmpRegisterString[0] = 0;
      }
      return;
    }
#endif //OPTION_XFN_1000

    // else do the standard register branch

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
        if(dataFileMode) {
          sprintf(tmpRegisterString, "%c%s#%" PRIu32, sign ? '-' : '+', yy, base);
        }
        else {
          sprintf(tmpRegisterString, "%c%s %" PRIu32, sign ? '-' : '+', yy, base);
        }
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
        if(dataFileMode) {
          char reStr[100], imStr[100], *imValue;
          char imSign = '+';
          real34ToString(REGISTER_REAL34_DATA(regist), reStr);
          real34ToString(REGISTER_IMAG34_DATA(regist), imStr);
          imValue = imStr;
          if(imStr[0] == '-') {                                                  // the imaginary sign is between the real part and the 'i', so strip it from the imaginary value
            imSign = '-';
            imValue++;
          }
          else if(imStr[0] == '+') {
            imValue++;
          }
          sprintf(tmpRegisterString, "(%s%ci%s)", reStr, imSign, imValue);       // e.g. real 3, imag -4 -> "(3-i4)"
        }
        else {
          real34ToString(REGISTER_REAL34_DATA(regist), tmpRegisterString);
          strcat(tmpRegisterString, " ");
          real34ToString(REGISTER_IMAG34_DATA(regist), tmpRegisterString + strlen(tmpRegisterString));
        }
        strcpy(aimBuffer1, "Cplx");
        textTag(aimBuffer1, getRegisterAngularMode(regist), getComplexRegisterPolarMode(regist));
        break;
      }

      case dtTime: {
        if(dataFileMode) {
          copySourceRegisterToDestRegister(regist, TEMP_REGISTER_1);
          fnFrom_msRegister(TEMP_REGISTER_1);                                    // dtTime -> HHMMSS-coded real, in place, so the live register is untouched
          real34ToString(REGISTER_REAL34_DATA(TEMP_REGISTER_1), tmpRegisterString);
          strcpy(aimBuffer1, "THMS");
        }
        else {
          real34ToString(REGISTER_REAL34_DATA(regist), tmpRegisterString);
          strcpy(aimBuffer1, "Time");
        }
        break;
      }

      case dtDate: {
        if(dataFileMode) {
          copySourceRegisterToDestRegister(regist, TEMP_REGISTER_1);
          convertDateRegisterToReal34Register(TEMP_REGISTER_1, TEMP_REGISTER_1); // dtDate -> real in the current date-format field order, in place
          real34ToString(REGISTER_REAL34_DATA(TEMP_REGISTER_1), tmpRegisterString);
          if(getSystemFlag(FLAG_YMD)) {                                          // record the field order so the loader can reproduce it
            strcpy(aimBuffer1, "DYMD");
          }
          else if(getSystemFlag(FLAG_MDY)) {
            strcpy(aimBuffer1, "DMDY");
          }
          else {
            strcpy(aimBuffer1, "DDMY");
          }
        }
        else {
          real34ToString(REGISTER_REAL34_DATA(regist), tmpRegisterString);
          strcpy(aimBuffer1, "Date");
        }
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
    #define MATRIX_ELEMENT_SEPARATOR "\t"          // in-row separator for data-file export; change to " " for a single space
    uint16_t cols;

    if(getRegisterDataType(regist) == dtReal34Matrix) {
      cols = REGISTER_MATRIX_HEADER(regist)->matrixColumns;
      for(uint32_t element = 0; element < REGISTER_MATRIX_HEADER(regist)->matrixRows * REGISTER_MATRIX_HEADER(regist)->matrixColumns; ++element) {
        real34ToString(REGISTER_REAL34_MATRIX_ELEMENTS(regist) + element, tmpString);
        if(dataFileMode) {
          strcat(tmpString, ((element % cols) == (uint32_t)(cols - 1)) ? "\n" : MATRIX_ELEMENT_SEPARATOR);   // one matrix row per line, elements separated within the row
        }
        else {
          strcat(tmpString, "\n");
        }
        save(tmpString, strlen(tmpString));
      }
    }
    else if(getRegisterDataType(regist) == dtComplex34Matrix) {
      cols = REGISTER_MATRIX_HEADER(regist)->matrixColumns;
      for(uint32_t element = 0; element < REGISTER_MATRIX_HEADER(regist)->matrixRows * REGISTER_MATRIX_HEADER(regist)->matrixColumns; ++element) {
        if(dataFileMode) {
          char reStr[100], imStr[100], *imValue;
          char imSign = '+';
          real34ToString(VARIABLE_REAL34_DATA(REGISTER_COMPLEX34_MATRIX_ELEMENTS(regist) + element), reStr);
          real34ToString(VARIABLE_IMAG34_DATA(REGISTER_COMPLEX34_MATRIX_ELEMENTS(regist) + element), imStr);
          imValue = imStr;
          if(imStr[0] == '-') {                                                  // the imaginary sign is emitted between the real part and the 'i', so strip it from the imaginary value
            imSign = '-';
            imValue++;
          }
          else if(imStr[0] == '+') {
            imValue++;
          }
          sprintf(tmpString, "(%s%ci%s)", reStr, imSign, imValue);               // parenthesised so element boundaries are unambiguous under free-form whitespace
          strcat(tmpString, ((element % cols) == (uint32_t)(cols - 1)) ? "\n" : MATRIX_ELEMENT_SEPARATOR);   // one matrix row per line, elements separated within the row
        }
        else {
          real34ToString(VARIABLE_REAL34_DATA(REGISTER_COMPLEX34_MATRIX_ELEMENTS(regist) + element), tmpString);
          strcat(tmpString, " ");
          real34ToString(VARIABLE_IMAG34_DATA(REGISTER_COMPLEX34_MATRIX_ELEMENTS(regist) + element), tmpString + strlen(tmpString));
          strcat(tmpString, "\n");
        }
        save(tmpString, strlen(tmpString));
      }
    }
    #undef MATRIX_ELEMENT_SEPARATOR
  }


bool_t fnSaveDataRegisters(uint16_t *beginR, uint16_t *endR, char *registerName, bool_t isXFNRegister) {
  // Appends a register section to the already-open file: header, count, then per register id/name line, type line, 
  // value line, and matrix element lines. registerName != NULL saves that one named variable (beginR/endR ignored);
  // NULL saves the range *beginR..*endR inclusive. Lettered registers (100..125) use the short "RX".."RW" form. Returns false on invalid arguments.
  char tmpString[3000];           // Local target buffer. registerToSaveString() emits the value through the global
                                  // tmpRegisterString (== global tmpString + START_REGISTER_VALUE), so a separate
                                  // local target is required here while that source is live, exactly as in doSave().
  char regName[16];
  calcRegister_t regist;

  if(registerName != NULL) {
    regist = findNamedVariable(registerName);   // read-only lookup: saving must not allocate a missing variable
    if(regist == INVALID_VARIABLE) {
      return false;
    }
    sprintf(tmpString, "NAMED_VARIABLES\n%" PRIu16 "\n", (uint16_t)1);
    save(tmpString, strlen(tmpString));
    registerToSaveString(regist, !isXFN);
    sprintf(tmpString, "%s\n%s\n%s\n", registerName, aimBuffer1, tmpRegisterString);
    save(tmpString, strlen(tmpString));
    saveMatrixElements(regist);
    return true;
  }

  if(beginR == NULL || endR == NULL || *endR < *beginR) {
    return false;
  }

  sprintf(tmpString, "GLOBAL_REGISTERS\n%" PRIu16 "\n", (uint16_t)(*endR - *beginR + 1));
  save(tmpString, strlen(tmpString));
  for(regist = (calcRegister_t)*beginR; regist <= (calcRegister_t)*endR; regist++) {
    registerToSaveString(regist, isXFNRegister);
    registerNumberToString(regist, regName);
    sprintf(tmpString, "%s\n%s\n%s\n", regName, aimBuffer1, tmpRegisterString);
    save(tmpString, strlen(tmpString));
    saveMatrixElements(regist); ///only executes if really a matrix otherwise returns
  }
  return true;
}

static void doSaveDataFile(uint16_t *beginR, uint16_t *endR, char *registerName, bool_t isXFNRegister) {
  ioFilePath_t path;
  int ret;
  char header[40];

  path = ioPathRegExport;
  ret = ioFileOpen(path, ioModeWrite);
  if(ret != FILE_OK) {
    if(ret == FILE_CANCEL) {
      screenUpdatingMode = SCRUPD_AUTO;
      refreshScreen(2996);
      return;
    }
    else {
      displayCalcErrorMessage(ERROR_CANNOT_WRITE_FILE, ERR_REGISTER_LINE, REGISTER_X);
      return;
    }
  }

  hourGlassIconEnabled = true;
  showHideHourGlass();

  dataFileMode = true;                                                           // write complex values in the compact human-readable form for the duration of this file
  sprintf(header, "DATA_FILE_REVISION\n%" PRIu8 "\n", (uint8_t)0);
  save(header, strlen(header));

  fnSaveDataRegisters(beginR, endR, registerName, isXFNRegister);

  dataFileMode = false;
  ioFileClose();
  temporaryInformation = TI_DATA_SAVED;   // confirm this constant

  screenUpdatingMode = SCRUPD_AUTO;
  refreshScreen(2997);

}

void fnSaveStackRegisters(uint16_t unusedButMandatoryParameter) {
  uint16_t beginR = REGISTER_X;
  uint16_t endR   = REGISTER_X + (getSystemFlag(FLAG_SSIZE8) ? 7 : 3);
  doSaveDataFile(&beginR, &endR, NULL, !isXFN);
}

void fnSaveLetteredRegisters(uint16_t unusedButMandatoryParameter) {
  uint16_t beginR = REGISTER_X;
  uint16_t endR   = REGISTER_W;
  doSaveDataFile(&beginR, &endR, NULL, !isXFN);
}

void fnSaveNRegisters(uint16_t N) {
  if(N <= 125) {
    uint16_t beginR = 0;
    uint16_t endR   = N - 1;
    doSaveDataFile(&beginR, &endR, NULL, !isXFN);
  } else {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "number of registers out of range: %d", N);
      moreInfoOnError("In function fnSaveNRegisters:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}

void fnSaveRegister(uint16_t regist) {
  if(FIRST_NAMED_VARIABLE <= regist && regist < FIRST_NAMED_VARIABLE + numberOfNamedVariables) {
    char varName[16];
    stringToUtf8((char *)allNamedVariables[regist - FIRST_NAMED_VARIABLE].variableName + 1, (uint8_t *)varName);
    doSaveDataFile(NULL, NULL, varName, !isXFN);                                    // named variable: save by name
  } else if(regist < LAST_SPARE_REGISTER) {
    uint16_t beginR = regist;
    uint16_t endR   = regist;
    doSaveDataFile(&beginR, &endR, NULL, !isXFN);                                   // numbererd or lettered register: save by number
  } else {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "register number out of range: %d", regist);
      moreInfoOnError("In function fnSaveRegister:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}

void fnSaveXFNRegister(uint16_t unusedButMandatoryParameter) {
  const uint16_t regist = REGISTER_X;
  if(regist < LAST_SPARE_REGISTER - 2) {
    uint16_t beginR = regist;
    uint16_t endR   = regist;
    doSaveDataFile(&beginR, &endR, NULL, isXFN);
  } else {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "register number out of range: %d", regist);
      moreInfoOnError("In function fnSaveRegister:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


static void doSave(uint16_t saveType);

  void fnSaveAuto(uint16_t unusedButMandatoryParameter) {
  #if defined(DMCP_BUILD)
    doSave(autoSave);
  #endif // DMCP_BUILD
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
  printStatus(0, errorMessages[SAVING_STATE_FILE], force);
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
  if( power_check_screen() ) {
    return;
  }
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
    registerToSaveString(regist, !isXFN);
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
    registerToSaveString(FIRST_LOCAL_REGISTER + i, !isXFN);
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
    registerToSaveString(FIRST_NAMED_VARIABLE + i, !isXFN);
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
        sprintf(tmpString, "amortP1\n%"                    PRIu16 "\n",     amortP1);                      save(tmpString, strlen(tmpString));
        sprintf(tmpString, "amortP2\n%"                    PRIu16 "\n",     amortP2);                      save(tmpString, strlen(tmpString));
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
        sprintf(tmpString, "printerOn\n%"                  PRIu8  "\n",     printerState.print_on);        save(tmpString, strlen(tmpString));
        sprintf(tmpString, "printerModel\n%"               PRIu8  "\n",     printerState.printer_model);   save(tmpString, strlen(tmpString));
        sprintf(tmpString, "printerLineDelay\n%"           PRIu16 "\n",     printerState.delay);           save(tmpString, strlen(tmpString));
        sprintf(tmpString, "END_OTHER_PARAM\n");                                                           save(tmpString, strlen(tmpString));

  ioFileClose();

  hourGlassIconEnabled = false;
  temporaryInformation = TI_SAVED;
}



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
  char eol1, eol2;

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
  uint32_t v0, v1;

  v0 = value & 0xffffffff;
  v1 = value >> 32;
  if(v1 != 0) {
    sprintf(tmpRegisterString, "0x%" PRIx32 "%08" PRIx32, v1, v0);
  }
  else {
    sprintf(tmpRegisterString, "0x%" PRIx32, v0);
  }
}

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


  static void dataFileCommaToPeriod(char *str) {
    while(*str) {
      if(*str == ',') {
        *str = '.';
      }
      str++;
    }
  }


  // Function to standardize new input to the old state file format, any new complex, into the old "re im" form. 
  // Accepts (3-i4) | 3-i4 | +3+i4 | -3-i4 | (+3+i4) | (-3-i4) | 3 -4 | 3 4 | -2.5 1e3 | -2.5e-3+i4 | (-2.5e-3-i4) | i4 | (i4) | 
  //         ( 3 - i4 ) | (1.5e2-i2.5e-3) | 0+i0 | -7+i0 | (3 -i4) | 3 -i4 | +3 +i4 | -3 -i4 | ( 3 - i 4 ) | +3 + i4 | 3 - i 4 | (-2.5e-3 - i 4)
  //         (parentheses optional, leading sign optional)
  // as well as the state file form  "3 -4"  (backward compatibility). Form and free white space is enabled by 'i' (which can never occur in a real number string). 
  static void standardiseComplex(const char *src, char *dest) {
    char work[200];
    char *w = work;
    char *iPos;
    bool_t isIform = (strchr(src, 'i') != NULL);

    // Copy src into a writable buffer, dropping parentheses. For the 'i' form drop all whitespace too; for the stock
    // space form keep internal whitespace (it is the real/imaginary separator) and only trim the ends below.
    while(*src != 0) {
      if(*src == '(' || *src == ')') {
        src++;
        continue;
      }
      if(isIform && (*src == ' ' || *src == '\t' || *src == '\n' || *src == '\r')) {
        src++;
        continue;
      }
      *(w++) = *src;
      src++;
    }
    *w = 0;

    if(!isIform) {
      char *start = work;
      while(*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r') {           // trim leading whitespace
        start++;
      }
      while(w > start && (w[-1] == ' ' || w[-1] == '\t' || w[-1] == '\n' || w[-1] == '\r')) { // trim trailing whitespace
        *(--w) = 0;
      }
      strcpy(dest, start);                                                       // already the stock "re im" form
      return;
    }

    iPos = strchr(work, 'i');
    if(iPos == work) {
      sprintf(dest, "0 +%s", iPos + 1);                                          // leading 'i' (e.g. "i4" meaning 0+i4): real part is zero
      return;
    }

    {
      char imSign = *(iPos - 1);                                                 // the character right before 'i' is the imaginary sign
      char *imMag = iPos + 1;                                                    // the imaginary magnitude follows the 'i'
      size_t realLen = (size_t)(iPos - 1 - work);                                // the real part is everything before that sign
      char realStr[200];

      if(imSign != '+' && imSign != '-') {                                       // malformed: treat the char before 'i' as part of the real and assume '+'
        imSign = '+';
        realLen = (size_t)(iPos - work);
      }
      xcopy(realStr, work, realLen);
      realStr[realLen] = 0;
      sprintf(dest, "%s %c%s", realStr, imSign, imMag);
    }
  }


  // Read the next whitespace-delimited token from the open file, skipping any leading whitespace (spaces, tabs, CR,
  // LF). Used by the data-file matrix reader so that matrix element layout (one per line, all on one line, or any
  // mix) is irrelevant. NotE: readLine()'s use of restore()/ioEof().
  static char *readToken(char *tok) {
    char *p = tok;

    if(!ioEof()) {
      restore(p, 1);
      while((*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') && !ioEof()) {
        restore(p, 1);
      }
      while(*p != ' ' && *p != '\t' && *p != '\n' && *p != '\r' && !ioEof()) {
        restore(++p, 1);
      }
    }
    *p = 0;
    return tok;
  }


  // Read the next complex matrix element token, skipping leading whitespace. A complex element is either a
  // parenthesised group "( ... )" (read up to and including the closing parenthesis, may span newlines) or a bare
  // whitespace-free 'i' form token.
  static char *readComplexToken(char *tok) {
    char *p = tok;

    if(!ioEof()) {
      restore(p, 1);
      while((*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') && !ioEof()) {
        restore(p, 1);
      }
      if(*p == '(') {
        while(*p != ')' && !ioEof()) {
          restore(++p, 1);
        }
        if(*p == ')') {
          p++;
        }
      }
      else {
        while(*p != ' ' && *p != '\t' && *p != '\n' && *p != '\r' && !ioEof()) {
          restore(++p, 1);
        }
      }
    }
    *p = 0;
    return tok;
  }


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
      if(dataFileMode) {
        dataFileCommaToPeriod(value);
      }
      stringToReal34(value, REGISTER_REAL34_DATA(regist));
    }

#if defined(OPTION_XFN_1000)
    else if(strcmp(type, "RXFN") == 0) {
      if(regist != REGISTER_X) {
        displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "XFN import only to REGISTER_X (stack), got register %d", (int)regist);
          moreInfoOnError("In function restoreRegister:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return;
      }

      REAL_T_PTR(paramX, 1071);
      REAL_T_PTR(paramY, 1071);
      REAL_T_PTR(paramTemp, 1071);
      realContext_t c = ctxtReal75;
      c.digits = 1034;
      angularMode_t angleMode = tag;
      angularMode_t tmpAngle;

      if(dataFileMode) {
        dataFileCommaToPeriod(value);
      }
      stringToReal(value, paramX, &c);

      processResultantLongReal(REGISTER_X, 0, FT_EXTERNAL, paramX, paramY, paramTemp, &angleMode, &tmpAngle);
    }
#endif //OPTION_XFN_1000

    else if(strcmp(type, "Time") == 0) {
      reallocateRegister(regist, dtTime, 0, amNone);
      stringToReal34(value, REGISTER_REAL34_DATA(regist));
    }

    else if(strcmp(type, "Date") == 0) {
      reallocateRegister(regist, dtDate, 0, amNone);
      stringToReal34(value, REGISTER_REAL34_DATA(regist));
    }

    else if(strcmp(type, "THMS") == 0) {
      reallocateRegister(TEMP_REGISTER_1, dtReal34, 0, amNone);
      if(dataFileMode) {
        dataFileCommaToPeriod(value);
      }
      stringToReal34(value, REGISTER_REAL34_DATA(TEMP_REGISTER_1));
      hmmssInRegisterToSeconds(TEMP_REGISTER_1);
      copySourceRegisterToDestRegister(TEMP_REGISTER_1, regist);
    }

    else if(strcmp(type, "DYMD") == 0 || strcmp(type, "DDMY") == 0 || strcmp(type, "DMDY") == 0) {
      bool_t savedYMD = getSystemFlag(FLAG_YMD);                                 // the type code gives the field order; force the matching flag for the conversion, then restore the user's flags
      bool_t savedMDY = getSystemFlag(FLAG_MDY);
      bool_t savedDMY = getSystemFlag(FLAG_DMY);

      clearSystemFlag(FLAG_YMD);
      clearSystemFlag(FLAG_MDY);
      clearSystemFlag(FLAG_DMY);
      if(strcmp(type, "DYMD") == 0) {
        setSystemFlag(FLAG_YMD);
      }
      else if(strcmp(type, "DMDY") == 0) {
        setSystemFlag(FLAG_MDY);
      }
      else {
        setSystemFlag(FLAG_DMY);
      }

      reallocateRegister(TEMP_REGISTER_1, dtReal34, 0, amNone);
      if(dataFileMode) {
        dataFileCommaToPeriod(value);
      }
      stringToReal34(value, REGISTER_REAL34_DATA(TEMP_REGISTER_1));
      convertReal34RegisterToDateRegister(TEMP_REGISTER_1, TEMP_REGISTER_1, false);
      copySourceRegisterToDestRegister(TEMP_REGISTER_1, regist);

      clearSystemFlag(FLAG_YMD);
      clearSystemFlag(FLAG_MDY);
      clearSystemFlag(FLAG_DMY);
      if(savedYMD) {
        setSystemFlag(FLAG_YMD);
      }
      if(savedMDY) {
        setSystemFlag(FLAG_MDY);
      }
      if(savedDMY) {
        setSystemFlag(FLAG_DMY);
      }
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

      value = strchr(value, '#') ? strchr(value, '#') + 1 : next_word(value);    // accept "+255#16" or the old "+255 16"
      uint32_t base = toUint32(value);

      convertUInt64ToShortIntegerRegister(sign, val, base, regist);
    }

    else if(strcmp(type, "Cplx") == 0) {
      char *imaginaryPart;
      char stdTmp[200];

      reallocateRegister(regist, dtComplex34, 0, tag);
      if(dataFileMode) {
        standardiseComplex(value, stdTmp);                                       // accept (3-i4) / 3-i4 / +3+i4 / stock "re im"; emit the stock "re im" form for the parser below
        dataFileCommaToPeriod(stdTmp);
        value = stdTmp;
      }
      imaginaryPart = skip_word(value);
      *(imaginaryPart++) = 0;
      stringToReal34(value, REGISTER_REAL34_DATA(regist));
      stringToReal34(imaginaryPart, REGISTER_IMAG34_DATA(regist));
    }

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
    uint16_t rows, cols;
    uint32_t i;

    if(getRegisterDataType(regist) == dtReal34Matrix) {
      rows = REGISTER_MATRIX_HEADER(regist)->matrixRows;
      cols = REGISTER_MATRIX_HEADER(regist)->matrixColumns;

      for(i = 0; i < rows * cols; ++i) {
        if(dataFileMode) {
          readToken(tmpString);                                                  // any whitespace (spaces and/or newlines) separates elements
          dataFileCommaToPeriod(tmpString);
        }
        else {
          readLine(tmpString);
        }
        stringToReal34(tmpString, REGISTER_REAL34_MATRIX_ELEMENTS(regist) + i);
      }
    }

    if(getRegisterDataType(regist) == dtComplex34Matrix) {
      rows = REGISTER_MATRIX_HEADER(regist)->matrixRows;
      cols = REGISTER_MATRIX_HEADER(regist)->matrixColumns;

      for(i = 0; i < rows * cols; ++i) {
        char *imaginaryPart;

        if(dataFileMode) {
          char stdTmp[200];

          readComplexToken(tmpString);                                           // one parenthesised "(re-iIM)" group (or bare i form) per element, free-form whitespace between elements
          standardiseComplex(tmpString, stdTmp);
          dataFileCommaToPeriod(stdTmp);
          strcpy(tmpString, stdTmp);
        }
        else {
          readLine(tmpString);
        }
        imaginaryPart = skip_word(tmpString);
        *(imaginaryPart++) = 0;
        stringToReal34(tmpString,     VARIABLE_REAL34_DATA(REGISTER_COMPLEX34_MATRIX_ELEMENTS(regist) + i));
        stringToReal34(imaginaryPart, VARIABLE_IMAG34_DATA(REGISTER_COMPLEX34_MATRIX_ELEMENTS(regist) + i));
      }
    }
  }


  static void skipMatrixData(char *type, char *value) {
    uint16_t rows, cols;
    uint32_t i;
    char *numOfCols;

    if(strcmp(type, "Rema") == 0 || strcmp(type, "Cxma") == 0) {
      numOfCols = skip_word(value);
      *(numOfCols++) = 0;
      rows = toUint16(value);
      cols = toUint16(numOfCols);

      for(i = 0; i < rows * cols; ++i) {
        if(dataFileMode) {
          if(strcmp(type, "Cxma") == 0) {                                        // skip exactly as restoreMatrixData() reads, or the file position desyncs
            readComplexToken(tmpString);
          }
          else {
            readToken(tmpString);
          }
        }
        else {
          readLine(tmpString);
        }
      }
    }
  }


#if defined(LOADDEBUG)
  static void debugPrintf(int s1, const char * s2, const char * s3) {
    #if defined(PC_BUILD)
      printf("%i %s %s\n", s1, s2, s3);
    #else
      char yy[1000];
      sprintf(yy, "%i %s %s\n", s1, s2, s3);
//      printHalfSecUpdate_Integer(force+1, yy, 0);
//      print_linestr(yy, false);

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


  // Datafile: Reads the next line, first skipping any comment blocks: a "Cmnt" line followed by one line of comment text, repeated.
  // Used where a register id or variable name is expected, so comments can sit between entries without
  // affecting the entry count.
  static void readLineSkippingComments(char *line) {
    readLine(line);
    while(strcmp(line, "Cmnt") == 0) {
      readLine(line);                           // drop the comment text line
      readLine(line);                           // next line: another Cmnt, or the real id / name
    }
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
      sprintf(line, ", loadMode:%d, %s\n", loadMode, tmpString);
      debugPrintf(0, "-", line);
    #endif //LOADDEBUG

    if(strcmp(tmpString, "GLOBAL_REGISTERS") == 0) {
      readLine(tmpString); // Number of global registers
      numberOfRegs = toInt16(tmpString);
      for(i=0; i<numberOfRegs; i++) {
        readLineSkippingComments(tmpString); // Register number, skipping any comments
        regist = stringToRegisterNumber(tmpString);
        read2Lines(aimBuffer, tmpString); // Register data type & Register value

        if(loadMode == LM_ALL || (loadMode == LM_REGISTERS && regist < REGISTER_X) || (loadMode == LM_REGISTERS_PARTIAL && regist >= s && regist < (s + n))) {
          #if defined(LOADDEBUG)
            sprintf(line, ", register=%i loadMode:%d, ['%s'] = %s", regist - s + d, loadMode, aimBuffer, tmpString);
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
          sprintf(line, ", loadMode:%d, %s\n", loadMode, tmpString);
          debugPrintf(2, "-", tmpString);
        #endif //LOADDEBUG
        str = tmpString;
        for(i = 0; i < 7; i++) {
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
          sprintf(line, ", loadMode:%d, %s\n", loadMode, tmpString);
          debugPrintf(3, "A", tmpString);
        #endif //LOADDEBUG
        allocateLocalRegisters(numberOfRegs);
      }

      if((loadMode != LM_ALL && loadMode != LM_REGISTERS) || lastErrorCode == ERROR_NONE) {
        #if defined(LOADDEBUG)
          sprintf(line, ", loadMode:%d, %s\n", loadMode, tmpString);
          debugPrintf(3, "B", tmpString);
        #endif //LOADDEBUG
        for(i=0; i<numberOfRegs; i++) {
          readLine(tmpString); // Register number
          regist = toInt16(tmpString + 2) + FIRST_LOCAL_REGISTER;
          read2Lines(aimBuffer, tmpString); // Register data type & Register value

          if(loadMode == LM_ALL || loadMode == LM_REGISTERS) {
            #if defined(LOADDEBUG)
              sprintf(line, ", loadMode:%d, %s\n", loadMode, tmpString);
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
        sprintf(line, ", loadMode:%d, %s\n", loadMode, tmpString);
        debugPrintf(4, "A", tmpString);
      #endif //LOADDEBUG
      readLine(tmpString); // LOCAL_FLAGS
      if(loadMode == LM_ALL || loadMode == LM_REGISTERS) {
        #if defined(LOADDEBUG)
          sprintf(line, ", loadMode:%d, %s\n", loadMode, tmpString);
          debugPrintf(4, "B", tmpString);
        #endif //LOADDEBUG
        *currentLocalFlags = toUint32(tmpString);
      }
    }

    else if(strcmp(tmpString, "NAMED_VARIABLES") == 0) {
      #if defined(LOADDEBUG)
        sprintf(line, ", loadMode:%d, %s\n", loadMode, tmpString);
        debugPrintf(20, "A", tmpString);
      #endif //LOADDEBUG
      readLine(tmpString); // Number of named variables
      numberOfRegs = toInt16(tmpString);
      for(i=0; i<numberOfRegs; i++) {
        readLine(errorMessage); // Variable name
        read2Lines(aimBuffer, tmpString); // Variable data type & Variable value

        if(( loadMode == LM_ALL ||
             loadMode == LM_NAMED_VARIABLES ||
            (loadMode == LM_SUMS && ((strcmp(errorMessage, "STATS") == 0) || (strcmp(errorMessage, "HISTO") == 0))) ) &&

           !(loadMode == LM_NAMED_VARIABLES && ((strcmp(errorMessage, "STATS") == 0) || (strcmp(errorMessage, "HISTO") == 0)))
          ) {

          #if defined(LOADDEBUG)
            sprintf(line, ", loadMode:%d, %s\n", loadMode, tmpString);
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

    else if(strcmp(tmpString, "Cmnt") == 0) {
      readLine(tmpString);                      // discard the single comment line that follows
      return true;
    }

    else if(strcmp(tmpString, "STATISTICAL_SUMS") == 0) {
      readLine(tmpString); // Number of statistical sums
      numberOfRegs = toInt16(tmpString);
      if(numberOfRegs > 0 && (loadMode == LM_ALL || loadMode == LM_SUMS)) {
        #if defined(LOADDEBUG)
          sprintf(line, ", loadMode:%d, %s\n", loadMode, tmpString);
          debugPrintf(6, "A", tmpString);
        #endif //LOADDEBUG

        initStatisticalSums();
        reLoadStatisticalSums();

        for(i=0; i<numberOfRegs; i++) {
          readLine(tmpString); // statistical sum
          if(statisticalSumsPointer) { // likely
            if(loadMode == LM_ALL || loadMode == LM_SUMS) {
              #if defined(LOADDEBUG)
                sprintf(line, ", loadMode:%d, %s\n", loadMode, tmpString);
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
          sprintf(line, ", loadMode:%d, %s\n", loadMode, tmpString);
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
          sprintf(line, ", loadMode:%d, %s\n", loadMode, tmpString);
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
        if(loadedVersion < 10000024) {
          setSystemFlag(FLAG_SIGZEROS); //SIGZEROS is on per default
        }

        // Ensure valid relations between FLAG_FRACT, FLAG_IRFRAC and FLAG_IRFRQ
        if(getSystemFlag(FLAG_FRACT)) {
          setSystemFlag(FLAG_FRACT);
        }
        else if(getSystemFlag(FLAG_IRFRAC)) {
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
            sprintf(line, ", loadMode:%d, %s\n", loadMode, tmpString);
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
          sprintf(line, ", loadMode:%d, %s\n", loadMode, tmpString);
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
            sprintf(line, ", loadMode:%d, %s\n", loadMode, tmpString);
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
            sprintf(line, ", loadMode:%d, %s\n", loadMode, tmpString);
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
            sprintf(line, ", loadMode:%d, %s\n", loadMode, tmpString);
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
            sprintf(line, ", loadMode:%d, %s\n", loadMode, tmpString);
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
              sprintf(line, ", loadMode:%d, %s\n", loadMode, tmpString);
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
          sprintf(line, ", loadMode:%d, %s\n", loadMode, tmpString);
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


      if(firstFreeProgramByte + freeProgramBytes != beginOfProgramMemory + TO_BYTES(numberOfBlocks) - 2) {
        uint32_t diff = TO_BYTES(RAM_SIZE_IN_BLOCKS_NEW_HW - RAM_SIZE_IN_BLOCKS_OLD_HW);
        #if defined(DMCP_BUILD) && defined(OLD_HW)
          if((firstFreeProgramByte + freeProgramBytes - diff == beginOfProgramMemory + TO_BYTES(numberOfBlocks) - 2)) {
            currentStep -= diff;
            firstFreeProgramByte -= diff;
          }
        #else
          if((firstFreeProgramByte + freeProgramBytes + diff == beginOfProgramMemory + TO_BYTES(numberOfBlocks) - 2)) {
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
          printf("  beginOfProgramMemory    = *8b %4u %16p\n", *beginOfProgramMemory,    (void*)(((uint32_t *)(beginOfProgramMemory)) ));
          printf("  firstFreeProgramByte    = *8b %4u %16p\n", *firstFreeProgramByte,    (void*)(((uint32_t *)(firstFreeProgramByte)) ));
          printf("  oldFirstFreeProgramByte = *8b %4u %16p\n", *oldFirstFreeProgramByte, (void*)(((uint32_t *)(oldFirstFreeProgramByte)) ));
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
          printf("  beginOfProgramMemory    = *8b %4u %16p\n", *beginOfProgramMemory,    (void*)(((uint32_t *)(beginOfProgramMemory)) ));
          printf("  firstFreeProgramByte    = *8b %4u %16p\n", *firstFreeProgramByte,    (void*)(((uint32_t *)(firstFreeProgramByte)) ));
          printf("  oldFirstFreeProgramByte = *8b %4u %16p\n", *oldFirstFreeProgramByte, (void*)(((uint32_t *)(oldFirstFreeProgramByte)) ));
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
          sprintf(line, ", loadMode:%d, %s\n", loadMode, tmpString);
          debugPrintf(15, "A", tmpString);
        #endif //LOADDEBUG
      }

      readLine(tmpString); // Number of formulae
      formulae = toUint16(tmpString);
      if(formulae > 0 && (loadMode == LM_ALL || loadMode == LM_PROGRAMS)) {
        #if defined(LOADDEBUG)
          sprintf(line, ", loadMode:%d, %s; formulae = %d\n", loadMode, tmpString, formulae);
          debugPrintf(15, "AA", "Resetting equations");
        #endif //LOADDEBUG
        for(i = numberOfFormulae; i > 0; --i) {
          deleteEquation(i - 1);
        }

        #if defined(LOADDEBUG)
          sprintf(line, ", loadMode:%d, %s\n", loadMode, tmpString);
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
              sprintf(line, ", loadMode:%d, %s\n", loadMode, tmpString);
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
          sprintf(line, ", loadMode:%d, %s\n", loadMode, tmpString);
          debugPrintf(16, "A", tmpString);
        #endif //LOADDEBUG
      }
      readLine(tmpString);                            // Read number params is not used anymore, reading until end of file or "END_OTHER_PARAM"; leaving it to read the old parameter in old files
      //numberOfRegs = toInt16(tmpString);            // Number of regs or params is not used anymore

      readLine(aimBuffer); //param                    // Reading duplicated OTHER_CONFIGURATION_STUFF
      if(strcmp(aimBuffer, "END_OTHER_PARAM") == 0) { // This is used for short form key-only state files, which specify that a setting reset is not to occur
        #if defined(LOADDEBUG)
          debugPrintf(16, "A", "Ending OTHER_CONFIGURATION_STUFF prior to RESET");
        #endif //LOADDEBUG
        goto END_CONFIG;                              // If this is END_OTHER_PARAM, loading is aborted before the config RESET, got break
      }
      else {
        resetOtherConfigurationStuff(allowUserKeys);  // Ensure all the configuration stuff below is reset prior to loading. That ensures if missing settings, that the proper defaults are set.
      }
      readLine(tmpString); //value 00                 // Reading second (duplicated 00)

      i = 0;
      while(i < 255) {                                                            //adjust for absolute maximum number of OTHER CONFIGUARTION SETTINGS
        readLine(aimBuffer); // param
        if(strcmp(aimBuffer, "END_OTHER_PARAM") == 0 || aimBuffer[0] == 0) {
          break; //to END_CONFIG break the reading loop for end of file (zero length read, or in all later files END_OTHER_PARAM)
        }
        readLine(tmpString); // value
        if(loadMode == LM_ALL || loadMode == LM_SYSTEM_STATE) {
          #if defined(LOADDEBUG)
            sprintf(line, ", loadMode:%d, %s\n", loadMode, tmpString);
            char aa[333];
            sprintf(aa, "B|%u|%s", i, aimBuffer);
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
          else if(strcmp(aimBuffer, "displayFormat"               ) == 0) { displayFormat           = toUint8(tmpString); }
          else if(strcmp(aimBuffer, "displayFormatDigits"         ) == 0) { displayFormatDigits     = toUint8(tmpString); }
          else if(strcmp(aimBuffer, "timeDisplayFormatDigits"     ) == 0) { timeDisplayFormatDigits = toUint8(tmpString); }
          else if(strcmp(aimBuffer, "shortIntegerWordSize"        ) == 0) { shortIntegerWordSize    = toUint8(tmpString); }
          else if(strcmp(aimBuffer, "shortIntegerMode"            ) == 0) { shortIntegerMode        = toUint8(tmpString); }
          else if(strcmp(aimBuffer, "significantDigits"           ) == 0) { significantDigits       = toUint8(tmpString); }
          else if(strcmp(aimBuffer, "fractionDigits"              ) == 0) { fractionDigits          = toUint8(tmpString); }
          else if(strcmp(aimBuffer, "currentAngularMode"          ) == 0) { currentAngularMode      = toUint8(tmpString); }
          else if(strcmp(aimBuffer, "groupingGap"                 ) == 0) { //backwards compatible loading old config files
            configCommon(CFG_DFLT);
            grpGroupingLeft = toUint8(tmpString);                     //Changed from groupingGap to remain compatible
            grpGroupingRight = grpGroupingLeft;
          }
          else if(strcmp2(aimBuffer, "gapItemLeft"                ) == 0) { gapItemLeft                = toUint16(tmpString); } // This is to correct a bug in version 00000005-6, to be compatible to the old files
          else if(strcmp2(aimBuffer, "gapItemRight"               ) == 0) { gapItemRight               = toUint16(tmpString); } // This is to correct a bug in version 00000005-6, to be compatible to the old files
          else if(strcmp2(aimBuffer, "gapItemRadix"               ) == 0) { gapItemRadix               = toUint16(tmpString); } // This is to correct a bug in version 00000005-6, to be compatible to the old files
          else if(strcmp2(aimBuffer, "lastCenturyHighUsed"        ) == 0) { lastCenturyHighUsed        = toUint16(tmpString); } // This is to correct a bug in version 00000005-6, to be compatible to the old files
          else if(strcmp2(aimBuffer, "grpGroupingLeft"            ) == 0) { grpGroupingLeft            = toUint8(tmpString);  } // This is to correct a bug in version 00000005-6, to be compatible to the old files
          else if(strcmp2(aimBuffer, "grpGroupingGr1LeftOverflow" ) == 0) { grpGroupingGr1LeftOverflow = toUint8(tmpString);  } // This is to correct a bug in version 00000005-6, to be compatible to the old files
          else if(strcmp2(aimBuffer, "grpGroupingGr1Left"         ) == 0) { grpGroupingGr1Left         = toUint8(tmpString);  } // This is to correct a bug in version 00000005-6, to be compatible to the old files
          else if(strcmp2(aimBuffer, "grpGroupingRight"           ) == 0) { grpGroupingRight           = toUint8(tmpString);  } // This is to correct a bug in version 00000005-6, to be compatible to the old files
          else if(strcmp(aimBuffer, "roundingMode"                ) == 0) { roundingMode               = toUint8(tmpString);  }
          else if(strcmp(aimBuffer, "displayStack"                ) == 0) { displayStack               = toUint8(tmpString);  }
          else if(strcmp(aimBuffer, "rngState"                    ) == 0) {
            pcg32_global.state = stringToUint64(tmpString);
            str = next_word(tmpString);
            pcg32_global.inc = stringToUint64(str);
          }
          else if(strcmp(aimBuffer, "exponentLimit"               ) == 0) { exponentLimit         = toInt16(tmpString); }
          else if(strcmp(aimBuffer, "exponentHideLimit"           ) == 0) { exponentHideLimit     = toInt16(tmpString); }
          else if(strcmp(aimBuffer, "notBestF"                    ) == 0) { lrSelection           = toUint16(tmpString);}
          else if(strcmp(aimBuffer, "bestF"                       ) == 0) { lrSelection           = toUint16(tmpString);}
          else if(strcmp(aimBuffer, "fgLN" ) == 0 || strcmp(aimBuffer, "jm_FG_LINE" ) == 0) {                                   // This section to deal with any old states containing the old FG system
            fgLN = convert001090400T001090500(toUint8(tmpString), RBX_FGLNOFF);
            if(fgLN == RBX_FGLNOFF) {
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
          }             //Keep compatible with old setting
          else if(strcmp(aimBuffer, "HOME3") == 0) {
            if(loadedVersion < 10000022) {
              forceSystemFlag(FLAG_HOME_TRIPLE, toUint8(tmpString) != 0); //Keep compatible by repeating, even though setting is now in systemflags
              setLongPressFg(calcModel, -MNU_HOME);
            }
          }
          else if(strcmp(aimBuffer, "MYM3") == 0) {
            if(loadedVersion < 10000022) {
              forceSystemFlag(FLAG_MYM_TRIPLE, toUint8(tmpString) != 0); //Keep compatible by repeating, even though setting is now in systemflags
              setLongPressFg(calcModel, -MNU_MyMenu);
            }
          }
          else if(strcmp(aimBuffer, "dispBase") == 0) {
            dispBase = toUint8(tmpString);
          }
          else if(strcmp(aimBuffer, "ShiftTimoutMode") == 0) {
            if(loadedVersion < 10000022) { //Keep compatible by repeating, even though setting is now in systemflags
              forceSystemFlag(FLAG_SHFT_4s, toUint8(tmpString) != 0);
            }
          }
          else if(strcmp(aimBuffer, "SH_BASE_HOME") == 0) { //Keep compatible with old name by repeating it
            if(loadedVersion < 10000022) { //Keep compatible by repeating, even though setting is now in systemflags
              forceSystemFlag(FLAG_BASE_HOME, toUint8(tmpString) != 0);
            }
          }
          else if(strcmp(aimBuffer, "BASE_HOME") == 0) {
            if(loadedVersion < 10000022) { //Keep compatible by repeating, even though setting is now in systemflags
              forceSystemFlag(FLAG_BASE_HOME, toUint8(tmpString) != 0);
            }
          }
          else if(allowUserKeys && (strcmp(aimBuffer, "Norm_Key_00_VAR") == 0)) {
            // Old state file, before changing Norm_Key_00_VAR to the Norm_Key_00 structure
            if(Norm_Key_00_key != -1) {
              Norm_Key_00.func  = toUint16(tmpString);   // only the function is restored, assuming no param
              Norm_Key_00.used  = Norm_Key_00.func != kbd_std[Norm_Key_00_key].primary;
            }
            else {
              Norm_Key_00.used = false;
            }
          }
          else if(allowUserKeys && (strcmp(aimBuffer, "Norm_Key_00.func") == 0)) {
            Norm_Key_00.func = toUint16(tmpString);
          }
          else if(strcmp(aimBuffer, "Norm_Key_00.funcParam") == 0) {                //  Workaround keeping old state files and new state files working, due to a blank string possibility which breaks the loading (on Mac sim at least).
            if(strcmp(tmpString, "Norm_Key_00.used") == 0) {                        //check if the next setting is erroneously read as data for the text data string 'funcParam'. In the old state file, a blank string was saved as param, which causes the single line read to fail, and the next setting name read as data.
              if(allowUserKeys) {
                Norm_Key_00.funcParam[0]=0;                                         //  - old file compatibility: If next setting name is found as data, clear it.
                Norm_Key_00.used = 0;                                               //  - populate the the next setting to default 0,  as the read has already currupted the sequence
              }
              readLine(tmpString);                                                  //  - read the next data line as a dummy and throw away, as it also has corrupted the sequence
            }
            else if(allowUserKeys && strcmp(tmpString, "NoNormKeyParamDef") == 0) { //if no data sequence corrution, check for the new keyword for a blank stirng. Note the keyword is longer than the 16 chars max of param strings. Hence the 'NoNormKeyParamDef' is unique and cannot be data.
              Norm_Key_00.funcParam[0]=0;                                           //  - if the code word for a blank string, blank the string.
            }
            else if(allowUserKeys) {                                                //  - New state files will have 'NoNormKeyParamDef' if no NRM+ XEQ paramater is present.
              strcpy(Norm_Key_00.funcParam, tmpString);                             //Otherwise proceed and use the data as normal
            }
          }
          else if(allowUserKeys && strcmp(aimBuffer, "Norm_Key_00.used") == 0) {
            Norm_Key_00.used      = toUint8(tmpString) != 0;
          }

          else if(allowUserKeys && strcmp(aimBuffer, "calcModel") == 0) {
            uint16_t calcModelRead = toUint16(tmpString);
            if(savedCalcModel == USER_R47 && (calcModelRead == USER_R47f_g || calcModelRead == USER_R47fg_g || calcModelRead == USER_R47fg_bk || calcModelRead == USER_R47bk_fg)) {
              calcModel = calcModelRead;
            }
            else if(savedCalcModel == USER_C47 && (calcModelRead == USER_C47    || calcModelRead == USER_DM42 )) {
              calcModel = calcModelRead;
            }
          }

          else if(strcmp(aimBuffer, "Input_Default") == 0) {
            Input_Default = toUint8(tmpString);
          }
          else if(strcmp(aimBuffer, "jm_BASE_SCREEN") == 0) {        //Keep compatible by repeating
            if(loadedVersion < 10000022) {
              forceSystemFlag(FLAG_BASE_MYM, toUint8(tmpString) != 0);
            } //Keep compatible by repeating, even though setting is now in systemflags
          }
          else if(strcmp(aimBuffer, "BASE_MYM") == 0) {
            if(loadedVersion < 10000022) {
              forceSystemFlag(FLAG_BASE_MYM, toUint8(tmpString) != 0);
            } //Keep compatible by repeating, even though setting is now in systemflags
          }
          else if(strcmp(aimBuffer, "jm_G_DOUBLETAP") == 0) {
            if(loadedVersion < 10000022) {
              forceSystemFlag(FLAG_G_DOUBLETAP, toUint8(tmpString) != 0);
            } //Keep compatible by repeating, even though setting is now in systemflags
          }

          else if(strcmp(aimBuffer, "displayStackSHOIDISP") == 0) {
            displayStackSHOIDISP  = toUint8(tmpString);
          }
          else if(strcmp(aimBuffer, "bcdDisplay") == 0) {
            if(loadedVersion < 10000020) {
              forceSystemFlag(FLAG_BCD, toUint8(tmpString) != 0);
            } //Keep compatible by repeating, even though setting is now in systemflags
          }
          else if(strcmp(aimBuffer, "topHex"                      ) == 0) {
            if(loadedVersion < 10000019) {
              forceSystemFlag(FLAG_TOPHEX, toUint8(tmpString) != 0);
            } //Keep compatible by repeating, even though setting is now in systemflags
          }
          else if(strcmp(aimBuffer, "bcdDisplaySign"              ) == 0) { bcdDisplaySign        = convert001090400T001090500(toUint8(tmpString), BCDu); }
          else if(strcmp(aimBuffer, "DRG_Cycling"                 ) == 0) { DRG_Cycling           = toUint8(tmpString); }
          else if(strcmp(aimBuffer, "DM_Cycling"                  ) == 0) { DM_Cycling            = toUint8(tmpString); }
          else if(strcmp(aimBuffer, "LongPressM"                  ) == 0) { LongPressM            = convert001090400T001090500(toUint8(tmpString), RBX_M14); } //10000003
          else if(strcmp(aimBuffer, "LongPressF"                  ) == 0) { LongPressF            = convert001090400T001090500(toUint8(tmpString), RBX_F14); } //10000003
          else if(strcmp(aimBuffer, "lastIntegerBase"             ) == 0) { lastIntegerBase       = toUint8(tmpString); }                                      //10000004
          else if(strcmp(aimBuffer, "amortP1"                     ) == 0) { amortP1               = toUint16(tmpString);}
          else if(strcmp(aimBuffer, "amortP2"                     ) == 0) { amortP2               = toUint16(tmpString);}
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
        #if defined(IR_PRINTING)
          else if(strcmp(aimBuffer, "printerOn"                   ) == 0) { printerState.print_on = toUint8(tmpString); }
          else if(strcmp(aimBuffer, "printerModel"                ) == 0) { printerState.printer_model    = toUint8(tmpString); }
          else if(strcmp(aimBuffer, "printerLineDelay"            ) == 0) { printerState.delay    = toUint16(tmpString); setLineDelay(printerState.delay);}
        #endif //IR_PRINTING
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



static void doLoadDataFile(uint16_t loadMode, uint16_t s, uint16_t n, uint16_t d) {
  ioFilePath_t path;
  int ret;

  path = ioPathRegImport;
  ret = ioFileOpen(path, ioModeRead);
  if(ret != FILE_OK) {
    if(ret == FILE_CANCEL) {
      screenUpdatingMode = SCRUPD_AUTO;
      refreshScreen(2998);
      return;
    }
    else {
      displayCalcErrorMessage(ERROR_CANNOT_READ_FILE, ERR_REGISTER_LINE, REGISTER_X);
      return;
    }
  }

  hourGlassIconEnabled = true;
  showHideHourGlass();

  dataFileMode = true;                                                           // accept compact complex form and free-form matrix whitespace while reading this file

  // Line 1 must be "DATA_FILE_REVISION" and line 2 must be exactly "0"
  readLine(tmpString);                      // "DATA_FILE_REVISION"
  if(strcmp(tmpString, "DATA_FILE_REVISION") != 0) {
    sprintf(tmpString, " \nThis is not a C47/R47 data file\n \nIt will not be loaded.");
    show_warning(tmpString);
    dataFileMode = false;
    ioFileClose();
    return;
  }
  readLine(tmpString);                      // revision number
  if(strcmp(tmpString, "0") != 0) {
    sprintf(tmpString, " \n   !!! Data file revision not supported !!!\nNot compatible with current version\n \nIt will not be loaded.");
    show_warning(tmpString);
    dataFileMode = false;
    ioFileClose();
    return;
  }

  liftStack();

  while(!ioEof()) {                                                             // Loop on ioEof(): restoreOneSection() only returns false after SYSTEM_STATE, absent from data files.
    restoreOneSection(loadMode, s, n, d, false);
  }

  dataFileMode = false;
  ioFileClose();
  temporaryInformation = TI_DATA_LOADED;

  screenUpdatingMode = SCRUPD_AUTO;
  refreshScreen(2999);

}


void fnLoadRegisters(uint16_t unusedButMandatoryParameter) {
  doLoadDataFile(LM_ALL, 0, 0, 0);
}



void doLoad(uint16_t loadMode, uint16_t s, uint16_t n, uint16_t d, uint16_t loadType) {
  savedCalcModel = 0;
  ioFilePath_t path;
  int ret;
  #if defined(LOADDEBUG)
    char yy[1000];
    sprintf(yy, "%d", loadMode);
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
    sprintf(yy, "%u", path);
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
      if(loadMode == LM_ALL) {
        enableLoad = true;
      }
    }
    break;
    case autoLoad: {
      if(((loadMode == LM_ALL) && (loadedVersion >= VersionAllowed) && (loadedVersion <= configFileVersion)) ){
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
  //    sprintf(tmpString, "****Program binary incompatibility****\n"
  //                       "x→α now followed by a destination register\n"
  //                       "Loaded x→α in RAM will be replaced by NOP\n"
  //                       "CAVEAT: x→α in Flash will not be replaced so it may cause crash\n");
  //  #endif // PC_BUILD
  //  #if defined(DMCP_BUILD)
  //    sprintf(tmpString, "**Program binary incompatibility**\n"
  //                       "x->a now uses a destination register\n"
  //                       "x->a in RAM will be replaced by NOP\n"
  //                       "CAVEAT: x->a in Flash will not be\n"
  //                       "replaced so it may cause crash\n");
  //  #endif // DMCP_BUILD
  //    show_warning(tmpString);
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
    sys_timer_start(TIMER_IDX_REFRESH_SLEEP, 1000);
    fnTimerStart(TO_KB_ACTV, TO_KB_ACTV, TO_KB_ACTV_MEDIUM); //PROGRAM_KB_ACTV
  #endif // DMCP_BUILD


    if(loadType == manualLoad && loadMode == LM_ALL) {
      temporaryInformation = TI_BACKUP_RESTORED;
      getDateString(lastStateFileOpened);
      strcat(lastStateFileOpened, ": ");
      stringCopy(lastStateFileOpened + stringByteLength(lastStateFileOpened), fileNameSelected);
    }
    else if((loadType == autoLoad) && (loadedVersion >= VersionAllowed) && (loadedVersion <= configFileVersion) && (loadMode == LM_ALL)) {
      temporaryInformation = TI_BACKUP_RESTORED;
      getDateString(lastStateFileOpened);
      strcat(lastStateFileOpened, ": ");
      stringCopy(lastStateFileOpened + stringByteLength(lastStateFileOpened), fileNameSelected);
    }
    else if(loadType == stateLoad) {
      temporaryInformation = TI_STATEFILE_RESTORED;
      getDateString(lastStateFileOpened);
      strcat(lastStateFileOpened, ": ");
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
}



void fnLoad(uint16_t loadMode) {
  printStatus(0, errorMessages[LOADING_STATE_FILE], force);
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
        int e = errno;
        if(e != ENOENT) {
          displayCalcErrorMessage(ERROR_IO, ERR_REGISTER_LINE, REGISTER_X);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            sprintf(errorMessage, "removing the backup failed with error code %d", e);
            moreInfoOnError("In function fnDeleteBackup:", errorMessage, NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        }
      }
    #endif // DMCP_BUILD
  }
}

