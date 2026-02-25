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

TO_QSPI const char errorMessages[NUMBER_OF_ERROR_CODES][SIZE_OF_EACH_ERROR_MESSAGE] = {
/*  0 */  "No error",
/*  1 */  "An argument exceeds the function domain",
/*  2 */  "Bad time or date input",
/*  3 */  "Undefined op-code",
/*  4 */  "Overflow at +" STD_INFINITY,
/*  5 */  "Overflow at -" STD_INFINITY,
/*  6 */  "No such label found",
/*  7 */  "No such function",
/*  8 */  "Out of range",
/*  9 */  "Illegal digit in integer input for this base",
/* 10 */  "Input is too long",
/* 11 */  "RAM is full",
/* 12 */  "Stack clash",
/* 13 */  "Operation is undefined in this mode",
/* 14 */  "Word size is too small",
/* 15 */  "Too few data points for this statistic",
/* 16 */  "Distribution parameter out of valid range",
/* 17 */  "I/O error",
/* 18 */  "Invalid or corrupted data",
/* 19 */  "Flash memory is write protected",
/* 20 */  "No root found",
/* 21 */  "Matrix mismatch",
/* 22 */  "Singular matrix",
/* 23 */  "Flash memory is full",
/* 24 */  "Invalid input data type for this operation",
/* 25 */  "No MVAR found in selected program",
/* 26 */  "Please enter a NEW name",
/* 27 */  "Cannot delete a predefined item",
/* 28 */  "No statistic data present",
/* 29 */  "Item to be coded",
/* 30 */  "Function to be coded for that data type",
/* 31 */  "Input data types do not match",
/* 32 */  "This system flag is write protected",
/* 33 */  "Output would exceed 508 characters",
/* 34 */  "This does not work with an empty string",
/* 35 */  "No backup data found",
/* 36 */  "Undefined source variable",
/* 37 */  "This variable is write protected",
/* 38 */  "No matrix indexed",
/* 39 */  "Not enough memory for such a matrix",
/* 40 */  "No errors for selected model",
/* 41 */  "Large " STD_DELTA " and opposite signs, may be a pole",
/* 42 */  "Solver reached local extremum, no root",
/* 43 */  STD_GREATER_EQUAL "1 initial guess lies out of the domain",
/* 44 */  "The function values seem constant",
/* 45 */  "Syntax error in this equation",
/* 46 */  "This equation formula is too complex",
/* 47 */  "This item cannot be assigned here",
/* 48 */  "Invalid name",
/* 49 */  "Too many variables",
/* 50 */  "Non-programmable command, please remove",
/* 51 */  "No global label in this program",
/* 52 */  "Invalid input data type for polar/rect mode",
/* 53 */  "Bad input", // This error is not in ReM and cannot occur (theoretically).
/* 54 */  "No program specified",
/* 55 */  "Cannot write file ",
/* 56 */  "Function has changed, please replace",
/* 57 */  "Variable required, please select variable",
/* 58 */  "HEX/DEC/OCT/BIN not usable with iCPX",
/* 59 */  "Undefined menu name",
/* 60 */  "Operation aborted",
/* 61 */  "Reserved variable name",
/* 62 */  "Invalid register type/angle",
/* 63 */  "",
/* 64 */  "",
/* 65 */  "",
/* 66 */  "",
/* 67 */  "",
/* 68 */  "",
/* 69 */  "",
/* 70 */  "",
/* 71 */  "",
/* 72 */  "",
/* 73 */  "",
/* 74 */  "",
/* 75 */  "",
/* 76 */  "",
/* 77 */  "",
/* 78 */  "",
/* 79 */  "",
/* 80 */  "",
/* 81 */  "",
/* 82 */  "",
/* 83 */  "",
/* 84 */  "",
/* 85 */  "",
/* 86 */  "",
/* 87 */  "",
/* 88 */  "",
/* 89 */  "",
/* 90 */  "",
/* 91 */  "",
/* 92 */  "",
/* 93 */  "",
/* 94 */  "",
/* 95 */  "",
/* 96 */  "",
/* 97 */  "",
/* 98 */  "",
/* 99 */  "",

//Status output messages for time consuming tasks, to keep user informed
/* 100 */  "  Loading state file ...",
/* 101 */  "  Saving state file ...",
/* 102 */  "  Loading stats ...",
/* 103 */  "  Solving for real/complex root ...",
/* 104 */  "  Calculating graph coordinates ...",
/* 105 */  "  Re-calculating sums ... ",
/* 106 */  "  Solving for real root ...",

//TI Messages (incomplete)
/* 107 */  "Backup restored",
/* 108 */  "State file loaded",
/* 109 */  "Programs and equations loaded",
/* 110 */  "appended",
/* 111 */  "Global and local registers loaded",
/* 112 */  "(w/ local flags)",
/* 113 */  "System settings loaded",
/* 114 */  "Statistical data loaded",
/* 115 */  "User variables loaded",
/* 116 */  "Program file loaded",
/* 117 */  "All global user flags cleared",
/* 118 */  "All data, programs and definitions cleared",
/* 119 */  "All user menus cleared",
/* 120 */  "All user variables cleared",
/* 121 */  "All user programs deleted",
/* 122 */  "All user menus deleted",
/* 123 */  "All user variables deleted",

//ERROR & TI Messages combined, used by either
/* 124 */  "Not available on the simulator",
/* 125 */  "Only available on the simulator",
/* 126 */  "Undo failed: likely no memory",

/* 189 */

};


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
  displayCalcErrorMessage((uint8_t)errorCode, ERR_REGISTER_LINE, REGISTER_X);
}



void fnErrorMessage(uint16_t unusedButMandatoryParameter) {
  real34_t r, maxErr;
  uInt32ToReal34(NUMBER_OF_ERROR_CODES, &maxErr);

  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
  switch(getRegisterDataType(REGISTER_X)) {
    case dtLongInteger: {
      convertLongIntegerRegisterToReal34(REGISTER_X, &r);
      break;
    }

    case dtReal34:
      if(getRegisterAngularMode(REGISTER_X) == amNone) {
        real34Copy(REGISTER_REAL34_DATA(REGISTER_X), &r);
        break;
      }
      /* fallthrough */


    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "data type %s cannot be used for this function!", getRegisterDataTypeName(REGISTER_X, false, false));
        moreInfoOnError("In function fnErrorMessage:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  #pragma GCC diagnostic pop

  if(real34CompareLessEqual(const34_1, &r) && real34CompareLessThan(&r, &maxErr)) {
    displayCalcErrorMessage((uint8_t)real34ToUInt32(&r), ERR_REGISTER_LINE, REGISTER_X);
  }
  else {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "the argument is not less than %u or is negative!", NUMBER_OF_ERROR_CODES);
      moreInfoOnError("In function fnErrorMessage:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}



void displayCalcErrorMessage(uint8_t errorCode, calcRegister_t errMessageRegisterLine, calcRegister_t errRegisterLine) {
  if(errorCode >= NUMBER_OF_ERROR_CODES || errorCode == 0) {
    sprintf(errorMessage, commonBugScreenMessages[bugMsgValueFor], "displayCalcErrorMessage", errorCode, "errorCode");
    displayBugScreen(errorMessage);
  }

  else if(errMessageRegisterLine > REGISTER_T || errMessageRegisterLine < REGISTER_X) {
    sprintf(errorMessage, commonBugScreenMessages[bugMsgValueFor], "displayCalcErrorMessage", errMessageRegisterLine, "errMessageRegisterLine");
    sprintf(errorMessage + strlen(errorMessage), "Must be from 100 (register X) to 103 (register T)");
    displayBugScreen(errorMessage);
  }

  else if(errRegisterLine > REGISTER_T || errRegisterLine < REGISTER_X) {
    sprintf(errorMessage, commonBugScreenMessages[bugMsgValueFor], "displayCalcErrorMessage", errRegisterLine, "errRegisterLine");
    sprintf(errorMessage + strlen(errorMessage), "Must be from 100 (register X) to 103 (register T)");
    displayBugScreen(errorMessage);
  }

  else {
    lastErrorCode            = errorCode;
    errorMessageRegisterLine = errMessageRegisterLine;
    screenUpdatingMode = SCRUPD_AUTO;
  }
}


void displayDomainErrorMessage(uint8_t errorCode, calcRegister_t errMessageRegisterLine, calcRegister_t errRegisterLine) {
  const int running = programRunStop == PGM_RUNNING;
  const int spcres = getSystemFlag(FLAG_SPCRES);

  if(!spcres || !running) {
    displayCalcErrorMessage(errorCode, errMessageRegisterLine, errRegisterLine);
  }
  if(spcres) {
    convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
  }
}


#if !defined(TESTSUITE_BUILD)
  void nextWord(const char *str, int16_t *pos, char *word) {
    int16_t i = 0;

    while(str[*pos] != 0 && str[*pos] != ' ') {
      word[i++] = str[(*pos)++];
    }

    word[i] = 0;

    while(str[*pos] == ' ') {
      (*pos)++;
    }
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
      int16_t y, pos;
      char line[100], word[50], message[1000];
      bool_t firstWordOfLine;

      previousCalcMode = calcMode;
      calcMode = CM_BUG_ON_SCREEN;
      clearSystemFlag(FLAG_ALPHA);
      hideCursor();
      cursorEnabled = false;

      lcd_fill_rect(0, 20, SCREEN_WIDTH, 220, LCD_SET_VALUE);

      y = 20;
      showString("This is most likely a bug in the firmware!", &standardFont, 1, y, vmNormal, true, false);
      y += 20;

      strcpy(message, msg);
      strcat(message, " Try to reproduce this and report a bug. Press EXIT to leave." );

      pos = 0;
      line[0] = 0;
      firstWordOfLine = true;

      nextWord(message, &pos, word);
      while(word[0] != 0) {
        if(stringWidth(line, &standardFont, true, true) + (firstWordOfLine ? 0 : 4) + stringWidth(word, &standardFont, true, true) >= SCREEN_WIDTH) { // 4 is the width of STD_SPACE_PUNCTUATION
          showString(line, &standardFont, 1, y, vmNormal, true, false);
          y += 20;
          line[0] = 0;
          firstWordOfLine = true;
        }

        if(firstWordOfLine) {
          strcpy(line, word);
          firstWordOfLine = false;
        }
        else {
          strcat(line, STD_SPACE_PUNCTUATION);
          strcat(line, word);
        }

        nextWord(message, &pos, word);
      }

      if(line[0] != 0) {
        showString(line, &standardFont, 1, y, vmNormal, true, false);
      }
    }
  }
#endif // !TESTSUITE_BUILD



/********************************************//**
 * \brief Data type error, common function
 *
 * \param void
 * \return void
 ***********************************************/
#if (EXTRA_INFO_ON_CALC_ERROR != 1)
  void typeError(void) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
  }
#endif // (EXTRA_INFO_ON_CALC_ERROR != 1)
