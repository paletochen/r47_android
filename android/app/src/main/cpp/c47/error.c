// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors


/********************************************//**
 * \file error.c
 ***********************************************/

#include "c47.h"


TO_QSPI const char commonBugScreenMessages[NUMBER_OF_BUG_SCREEN_MESSAGES][SIZE_OF_EACH_BUG_SCREEN_MESSAGE] = {
/*  0 */  "In function %s:%d is an unexpected value for %s!",
/*  1 */  "In function %s: unexpected calcMode value (%" PRIu8 ") while processing key %s!",
/*  2 */  "In function %s: no named variables defined!",
/*  3 */  "In function %s: %d is an unexpected value returned by findGlyph!",
/*  4 */  "In function %s: %" PRIu32 " is an unexpected %s value!",
/*  5 */  "In function %s: data type %s is unknown!",
/*  6 */  "In function %s: regist=%" PRId16 " must be less than %d!",
/*  7 */  "In function %s: %s %" PRId16 " is not defined! Must be from 0 to %" PRIu16,
/*  8 */  "In function %s: unexpected case while processing key %s! %" PRIu8 " is an unexpected value for rbrMode.",
};

// One entry per error code, in code order. The pool struct, its initializer and the
// offset table below are all generated from this single list, so they cannot drift.
#define ERROR_MESSAGE_LIST(ENTRY) \
  ENTRY(e000, "No error") /*   0 */ \
  ENTRY(e001, "An argument exceeds the function domain") /*   1 */ \
  ENTRY(e002, "Bad time or date input") /*   2 */ \
  ENTRY(e003, "Undefined op-code") /*   3 */ \
  ENTRY(e004, "Overflow at +" STD_INFINITY) /*   4 */ \
  ENTRY(e005, "Overflow at -" STD_INFINITY) /*   5 */ \
  ENTRY(e006, "No such label found") /*   6 */ \
  ENTRY(e007, "No such function") /*   7 */ \
  ENTRY(e008, "Out of range") /*   8 */ \
  ENTRY(e009, "Illegal digit in integer input for this base") /*   9 */ \
  ENTRY(e010, "Input is too long") /*  10 */ \
  ENTRY(e011, "RAM is full") /*  11 */ \
  ENTRY(e012, "Stack clash") /*  12 */ \
  ENTRY(e013, "Operation is undefined in this mode") /*  13 */ \
  ENTRY(e014, "Word size is too small") /*  14 */ \
  ENTRY(e015, "Too few data points for this statistic") /*  15 */ \
  ENTRY(e016, "Distribution parameter out of valid range") /*  16 */ \
  ENTRY(e017, "I/O error") /*  17 */ \
  ENTRY(e018, "Invalid or corrupted data") /*  18 */ \
  ENTRY(e019, "Flash memory is write protected") /*  19 */ \
  ENTRY(e020, "No root found") /*  20 */ \
  ENTRY(e021, "Matrix mismatch") /*  21 */ \
  ENTRY(e022, "Singular matrix") /*  22 */ \
  ENTRY(e023, "Flash memory is full") /*  23 */ \
  ENTRY(e024, "Invalid input data type for this operation") /*  24 */ \
  ENTRY(e025, "No MVAR found in selected program") /*  25 */ \
  ENTRY(e026, "Please enter a NEW name") /*  26 */ \
  ENTRY(e027, "Cannot delete a predefined item") /*  27 */ \
  ENTRY(e028, "No statistic data present") /*  28 */ \
  ENTRY(e029, "Item to be coded") /*  29 */ \
  ENTRY(e030, "Function to be coded for that data type") /*  30 */ \
  ENTRY(e031, "Input data types do not match") /*  31 */ \
  ENTRY(e032, "This system flag is write protected") /*  32 */ \
  ENTRY(e033, "Output would exceed 508 characters") /*  33 */ \
  ENTRY(e034, "This does not work with an empty string") /*  34 */ \
  ENTRY(e035, "No backup data found") /*  35 */ \
  ENTRY(e036, "Undefined source variable") /*  36 */ \
  ENTRY(e037, "This variable is write protected") /*  37 */ \
  ENTRY(e038, "No matrix indexed") /*  38 */ \
  ENTRY(e039, "Not enough memory for such a matrix") /*  39 */ \
  ENTRY(e040, "No errors for selected model") /*  40 */ \
  ENTRY(e041, "Large " STD_DELTA " and opposite signs, may be a pole") /*  41 */ \
  ENTRY(e042, "Solver reached local extremum, no root") /*  42 */ \
  ENTRY(e043, STD_GREATER_EQUAL "1 initial guess lies out of the domain") /*  43 */ \
  ENTRY(e044, "The function values seem constant") /*  44 */ \
  ENTRY(e045, "Syntax error in this equation") /*  45 */ \
  ENTRY(e046, "This equation formula is too complex") /*  46 */ \
  ENTRY(e047, "This item cannot be assigned here") /*  47 */ \
  ENTRY(e048, "Invalid name") /*  48 */ \
  ENTRY(e049, "Too many variables") /*  49 */ \
  ENTRY(e050, "Non-programmable command, please remove") /*  50 */ \
  ENTRY(e051, "No global label in this program") /*  51 */ \
  ENTRY(e052, "Invalid input data type for polar/rect mode") /*  52 */ \
  ENTRY(e053, "Bad input") /*  53 */ \
  ENTRY(e054, "No program specified") /*  54 */ \
  ENTRY(e055, "Cannot write file ") /*  55 */ \
  ENTRY(e056, "Function has changed, please replace") /*  56 */ \
  ENTRY(e057, "Variable required, please select variable") /*  57 */ \
  ENTRY(e058, "HEX/DEC/OCT/BIN not usable with iCPX") /*  58 */ \
  ENTRY(e059, "Undefined menu name") /*  59 */ \
  ENTRY(e060, "Operation aborted") /*  60 */ \
  ENTRY(e061, "Reserved variable name") /*  61 */ \
  ENTRY(e062, "Invalid register type/angle") /*  62 */ \
  ENTRY(e063, "Printing Is Disabled") /*  63 */ \
  ENTRY(e064, "No string in alpha register") /*  64 */ \
  ENTRY(e065, "No equation defined") /*  65 */ \
  ENTRY(e066, "Nesting too deep") /*  66 */ \
  ENTRY(e067, "IF/WHILE/UNTIL condition missing") /*  67 */ \
  ENTRY(e068, "Not available on this hardware") /*  68 */ \
  ENTRY(e069, "Structure not numbered (do VALID)") /*  69 */ \
  ENTRY(e070, "Structure invalid") /*  70 */ \
  ENTRY(e071, "NEXT not found") /*  71 */ \
  ENTRY(e072, "NEXT without FOR") /*  72 */ \
  ENTRY(e073, "Invalid counter register") /*  73 */ \
  ENTRY(e074, "Step of zero") /*  74 */ \
  ENTRY(e075, "") /*  75 */ \
  ENTRY(e076, "") /*  76 */ \
  ENTRY(e077, "") /*  77 */ \
  ENTRY(e078, "") /*  78 */ \
  ENTRY(e079, "") /*  79 */ \
  ENTRY(e080, "") /*  80 */ \
  ENTRY(e081, "") /*  81 */ \
  ENTRY(e082, "") /*  82 */ \
  ENTRY(e083, "") /*  83 */ \
  ENTRY(e084, "") /*  84 */ \
  ENTRY(e085, "") /*  85 */ \
  ENTRY(e086, "") /*  86 */ \
  ENTRY(e087, "") /*  87 */ \
  ENTRY(e088, "") /*  88 */ \
  ENTRY(e089, "") /*  89 */ \
  ENTRY(e090, "") /*  90 */ \
  ENTRY(e091, "") /*  91 */ \
  ENTRY(e092, "") /*  92 */ \
  ENTRY(e093, "") /*  93 */ \
  ENTRY(e094, "") /*  94 */ \
  ENTRY(e095, "") /*  95 */ \
  ENTRY(e096, "") /*  96 */ \
  ENTRY(e097, "") /*  97 */ \
  ENTRY(e098, "") /*  98 */ \
  ENTRY(e099, "") /*  99 */ \
  ENTRY(e100, " Loading state file ...") /* 100 */ \
  ENTRY(e101, " Saving state file ...") /* 101 */ \
  ENTRY(e102, " Loading stats ...") /* 102 */ \
  ENTRY(e103, " Solving for real/complex root ...") /* 103 */ \
  ENTRY(e104, " Calculating graph coordinates ...") /* 104 */ \
  ENTRY(e105, " Re-calculating sums ... ") /* 105 */ \
  ENTRY(e106, " Solving for real root ...") /* 106 */ \
  ENTRY(e107, "Backup restored") /* 107 */ \
  ENTRY(e108, "State file loaded") /* 108 */ \
  ENTRY(e109, "Programs and equations loaded") /* 109 */ \
  ENTRY(e110, "appended") /* 110 */ \
  ENTRY(e111, "Global and local registers loaded") /* 111 */ \
  ENTRY(e112, "(w/ local flags)") /* 112 */ \
  ENTRY(e113, "System settings loaded") /* 113 */ \
  ENTRY(e114, "Statistical data loaded") /* 114 */ \
  ENTRY(e115, "User variables loaded") /* 115 */ \
  ENTRY(e116, "Program file loaded") /* 116 */ \
  ENTRY(e117, "All global user flags cleared") /* 117 */ \
  ENTRY(e118, "All data, programs and definitions cleared") /* 118 */ \
  ENTRY(e119, "All user menus cleared") /* 119 */ \
  ENTRY(e120, "All user variables cleared") /* 120 */ \
  ENTRY(e121, "All user programs deleted") /* 121 */ \
  ENTRY(e122, "All user menus deleted") /* 122 */ \
  ENTRY(e123, "All user variables deleted") /* 123 */ \
  ENTRY(e124, "Data file loaded") /* 124 */ \
  ENTRY(e125, "Data file saved") /* 125 */ \
  ENTRY(e126, "Not available on the simulator") /* 126 */ \
  ENTRY(e127, "Only available on the simulator") /* 127 */ \
  ENTRY(e128, "Undo failed: likely no memory") /* 128 */

// The messages used to live in a fixed-width [NUMBER_OF_ERROR_CODES][SIZE_OF_EACH_ERROR_MESSAGE]
// array, spending the width of the longest message on every row. They are now packed back to
// back in one struct of exactly-sized char arrays (char alignment is 1, so there is no padding),
// with a per-code offset table computed by the compiler through offsetof.
typedef struct {
  #define ERROR_MESSAGE_FIELD(name, text) char name[sizeof(text)];
  ERROR_MESSAGE_LIST(ERROR_MESSAGE_FIELD)
  #undef ERROR_MESSAGE_FIELD
} errorMessagePool_t;

TO_QSPI static const errorMessagePool_t errorMessagePool = {
  #define ERROR_MESSAGE_INIT(name, text) text,
  ERROR_MESSAGE_LIST(ERROR_MESSAGE_INIT)
  #undef ERROR_MESSAGE_INIT
};

TO_QSPI static const uint16_t errorMessageOffset[NUMBER_OF_ERROR_CODES] = {
  #define ERROR_MESSAGE_OFFSET(name, text) offsetof(errorMessagePool_t, name),
  ERROR_MESSAGE_LIST(ERROR_MESSAGE_OFFSET)
  #undef ERROR_MESSAGE_OFFSET
};

const char *errorMessageOf(uint8_t errorCode) {
  return ((const char *)&errorMessagePool) + errorMessageOffset[errorCode];
}



#if defined(PC_BUILD)
  /********************************************//**
   * \brief Displays an error message like a pop up
   *
   * \param[in] m1 const char* 1st part of the message
   * \param[in] m2 const char* 2nd part of the message
   * \param[in] m3 const char* 3rd part of the message
   * \param[in] m4 const char* 4th part of the message
   * \return void
   ***********************************************/
  void moreInfoOnError(const char *m1, const char *m2, const char *m3, const char *m4) {
    uint8_t utf8m1[2000], utf8m2[2000], utf8m3[2000], utf8m4[2000];

    if(m1 == NULL) {
      stringToUtf8("No error message available!", utf8m1);
      printf("\n%s\n", utf8m1);
    }
    else if(m2 == NULL) {
      stringToUtf8(m1, utf8m1);
      printf("\n%s\n\n", utf8m1);
    }
    else if(m3 == NULL) {
      stringToUtf8(m1, utf8m1);
      stringToUtf8(m2, utf8m2);
      printf("\n%s\n%s\n\n", utf8m1, utf8m2);
    }
    else if(m4 == NULL) {
      stringToUtf8(m1, utf8m1);
      stringToUtf8(m2, utf8m2);
      stringToUtf8(m3, utf8m3);
      printf("\n%s\n%s\n%s\n\n", utf8m1, utf8m2, utf8m3);
    }
    else {
      stringToUtf8(m1, utf8m1);
      stringToUtf8(m2, utf8m2);
      stringToUtf8(m3, utf8m3);
      stringToUtf8(m4, utf8m4);
      printf("\n%s\n%s\n%s\n%s\n\n", utf8m1, utf8m2, utf8m3, utf8m4);
    }
  }
#endif // PC_BUILD



void fnRaiseError(uint16_t errorCode) {
  displayCalcErrorMessage((uint8_t)errorCode, ERR_REGISTER_LINE);
}



void fnErrorMessage(uint16_t unusedButMandatoryParameter) {
  real34_t maxErr;
  uint32_t errorCode = 0;
  uInt32ToReal34(NUMBER_OF_ERROR_CODES, &maxErr);

  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
  switch(getRegisterDataType(REGISTER_X)) {
    case dtLongInteger: {
      longInteger_t value;

      convertLongIntegerRegisterToLongInteger(REGISTER_X, value);
      if(longIntegerCompareUInt(value, 1) >= 0 && longIntegerCompareUInt(value, NUMBER_OF_ERROR_CODES) < 0) {
        longIntegerToUInt32(value, errorCode);
      }
      longIntegerFree(value);
      break;
    }

    case dtReal34:
      if(getRegisterAngularMode(REGISTER_X) == amNone) {
        const real34_t *value = REGISTER_REAL34_DATA(REGISTER_X);

        if(real34CompareLessEqual(const34_1, value) && real34CompareLessThan(value, &maxErr)) {
          errorCode = real34ToUInt32(value);
        }
        break;
      }
      /* fallthrough */


    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "data type %s cannot be used for this function!", getRegisterDataTypeName(REGISTER_X, false, false));
        moreInfoOnError("In function fnErrorMessage:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  #pragma GCC diagnostic pop

  if(errorCode != 0) {
    displayCalcErrorMessage((uint8_t)errorCode, ERR_REGISTER_LINE);
  }
  else {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "the argument is not less than %u or is negative!", NUMBER_OF_ERROR_CODES);
      moreInfoOnError("In function fnErrorMessage:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}



void displayCalcErrorMessage(uint8_t errorCode, calcRegister_t errMessageRegisterLine) {
  if(errorCode >= NUMBER_OF_ERROR_CODES || errorCode == 0) {
    sprintf(errorMessage, commonBugScreenMessages[bugMsgValueFor], "displayCalcErrorMessage", errorCode, "errorCode");
    displayBugScreen(errorMessage);
  }

  else if(errMessageRegisterLine > REGISTER_T || errMessageRegisterLine < REGISTER_X) {
    sprintf(errorMessage, commonBugScreenMessages[bugMsgValueFor], "displayCalcErrorMessage", errMessageRegisterLine, "errMessageRegisterLine");
    sprintf(errorMessage + strlen(errorMessage), "Must be from 100 (register X) to 103 (register T)");
    displayBugScreen(errorMessage);
  }

  else {
    lastErrorCode            = errorCode;
    errorMessageRegisterLine = errMessageRegisterLine;
    screenUpdatingMode = SCRUPD_AUTO;

    #if defined(OPTION_IR_PRINTING)
      if((tam.mode != 0) && (errorCode != ERROR_LABEL_NOT_FOUND) &&   !printerState.trace_done) {
        printTrace(tam.function, tam.value);
      }
      if(!getSystemFlag(FLAG_IGN1ER)) {
        if(lastErrorCode == ERROR_RESERVED_VARIABLE_NAME) {
          sprintf(tmpString, "%s: %s", errorMessageOf(lastErrorCode), errorMessage);
        }
        else {
          sprintf(tmpString, "%s", errorMessageOf(lastErrorCode));
        }
        printTraceError(tmpString);
      }
    #endif //OPTION_IR_PRINTING
  }
}


void displayDomainErrorMessage(uint8_t errorCode, calcRegister_t errMessageRegisterLine) {
  const int spcres = getSystemFlag(FLAG_SPCRES);

  if(!spcres) {
    displayCalcErrorMessage(errorCode, errMessageRegisterLine);
  }
  if(spcres) {
    convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
  }
}


  /********************************************//**
   * \brief Copies the next word of a NULL terminated list of strings, walked as one flow,
   *        into word. Separators, part boundaries and a broken trailing lead byte are
   *        skipped on the way in. A lead byte with no partner byte can only be the string's
   *        own NUL terminator, since a genuine second byte of value 0 would end the C
   *        string there too - so it unambiguously marks a message truncated mid glyph, and
   *        the orphan is dropped rather than read as part of a word.
   *
   *        Words are stepped a whole glyph at a time, never a byte: the second byte of a
   *        two byte glyph is free to be anything, including the 0x20 of a separator, so a
   *        byte comparison would split such a glyph and leave a lead byte with no partner
   *        for the font scan to read past.
   *
   *        A word too long for word is stopped on a glyph boundary and its remainder is
   *        left for the next call, which returns it as a word of its own.
   *
   * \param[in] part const char* const* The strings, NULL terminated
   * \param[in,out] p int16_t* Which string is being read, moved on past exhausted parts
   * \param[in,out] pos int16_t* Offset into that string, left just past the word
   * \param[out] word char* Receives the word, NUL terminated
   * \param[in] wordSize int16_t Size of word, terminator included
   * \return bool_t true if a word was copied, false when the list is exhausted
   ***********************************************/
  static bool_t nextWord(const char *const *part, int16_t *p, int16_t *pos, char *word, int16_t wordSize) {
    const int16_t maxLen = wordSize - 1;
    const char *str;
    int16_t start;

    for(;;) {
      str = part[*p];

      if(str == NULL) {
        return false;
      }

      if(str[*pos] == 0) {
        (*p)++;
        *pos = 0;
        continue;
      }

      if(str[*pos] == ' ') {
        (*pos)++;
        continue;
      }

      if((str[*pos] & 0x80) && str[*pos + 1] == 0) {
        (*p)++;
        *pos = 0;
        continue;
      }

      break;
    }

    start = *pos;

    while(str[*pos] != 0 && str[*pos] != ' ') {
      const int16_t step = (str[*pos] & 0x80) ? 2 : 1;

      if(step == 2 && str[*pos + 1] == 0) {
        break;
      }

      if(*pos - start + step > maxLen) {
        break;
      }

      *pos += step;
    }

    xcopy(word, str + start, *pos - start);
    word[*pos - start] = 0;
    return true;
  }


  /********************************************//**
   * \brief Displays an error message.
   *        When this happens it's likely a bug!
   *
   * \param[in] message char* The message
   * \return void
   ***********************************************/
  void displayBugScreen(const char *msg) {
    if(calcMode != CM_BUG_ON_SCREEN) {
      static const char *const bugScreenTail = "Try to reproduce this and report a bug. Press EXIT to leave.";
      const char *part[3];
      int16_t y, pos, p, x;
      char word[100];

      previousCalcMode = calcMode;
      calcMode = CM_BUG_ON_SCREEN;
      clearSystemFlag(FLAG_ALPHA);
      hideCursor();
      cursorEnabled = false;

      lcd_fill_rect(0, 20, SCREEN_WIDTH, 220, LCD_SET_VALUE);

      y = 20;
      showString("This is most likely a bug in the firmware!", &standardFont, 1, y, vmNormal, true, false);
      y += 20;

      part[0] = msg;
      part[1] = bugScreenTail;
      part[2] = NULL;

      // One word at a time, drawn where it lands: only the word is ever buffered, so the
      // line exists as an x position rather than as a string. The gap between words is the
      // separator's width added to x, so no separator is ever stored or measured.
      #define SEP_WIDTH 4   // the width of STD_SPACE_PUNCTUATION in standardFont

      p = 0;
      pos = 0;
      x = 1;

      while(nextWord(part, &p, &pos, word, (int16_t)sizeof(word))) {
        const int16_t w = stringWidth(word, &standardFont, true, true);

        // The first word of a line is kept however wide it is: the next line is no wider.
        if(x != 1 && x + w >= SCREEN_WIDTH) {
          y += 20;
          x = 1;
        }

        showString(word, &standardFont, x, y, vmNormal, true, false);
        x += w + SEP_WIDTH;
      }

      #undef SEP_WIDTH
    }
  }



/********************************************//**
 * \brief Data type error, common function
 *
 * \param void
 * \return void
 ***********************************************/
#if (EXTRA_INFO_ON_CALC_ERROR != 1)
  void typeError(void) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
  }
#endif // (EXTRA_INFO_ON_CALC_ERROR != 1)
