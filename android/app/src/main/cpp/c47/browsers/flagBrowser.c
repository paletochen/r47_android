// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file flagBrowser.c
 ***********************************************/

#include "c47.h"



TO_QSPI const  letteredFlagDisplay_t letteredFlagDisplay[] = {
// Flags X, Y, Z, T, A, B, C, D, L
/* 100 - X */  {.txt = STD_SPACE_6_PER_EM "X", .position = 3        },
/* 101 - Y */  {.txt = STD_SPACE_6_PER_EM "Y", .position = 3 +  1*12},
/* 102 - Z */  {.txt = STD_SPACE_6_PER_EM "Z", .position = 3 +  2*12},
/* 103 - T */  {.txt = STD_SPACE_6_PER_EM "T", .position = 3 +  3*12},
/* 104 - A */  {.txt = STD_SPACE_6_PER_EM "A", .position = 3 +  4*12},
/* 105 - B */  {.txt = STD_SPACE_6_PER_EM "B", .position = 3 +  5*12},
/* 106 - C */  {.txt = STD_SPACE_6_PER_EM "C", .position = 3 +  6*12},
/* 107 - D */  {.txt = STD_SPACE_6_PER_EM "D", .position = 3 +  7*12},
/* 108 - L */  {.txt = STD_SPACE_6_PER_EM "L", .position = 3 +  8*12},

// Flags I, J, K
/* 109 - I */  {.txt = STD_SPACE_6_PER_EM "I", .position = 8 + 10*12},
/* 110 - J */  {.txt = STD_SPACE_6_PER_EM "J", .position = 4 + 11*12},
/* 111 - K */  {.txt = STD_SPACE_6_PER_EM "K", .position = 2 + 12*12},

// Flags M, N, P, Q, R, S
/* 211 - M */  {.txt = STD_SPACE_6_PER_EM "M", .position = 4 + 15*12},
/* 212 - N */  {.txt = STD_SPACE_6_PER_EM "N", .position = 5 + 16*12},
/* 213 - P */  {.txt = STD_SPACE_6_PER_EM "P", .position = 6 + 17*12},
/* 214 - Q */  {.txt = STD_SPACE_6_PER_EM "Q", .position = 6 + 18*12},
/* 215 - R */  {.txt = STD_SPACE_6_PER_EM "R", .position = 6 + 19*12},
/* 216 - S */  {.txt = STD_SPACE_6_PER_EM "S", .position = 6 + 20*12},

// Flags E, F, G, H
/* 217 - E */  {.txt = STD_SPACE_6_PER_EM "E", .position = 9 + 22*12},
/* 218 - F */  {.txt = STD_SPACE_6_PER_EM "F", .position = 9 + 23*12},
/* 219 - G */  {.txt = STD_SPACE_6_PER_EM "G", .position = 8 + 24*12},
/* 220 - H */  {.txt = STD_SPACE_6_PER_EM "H", .position = 8 + 25*12},

// Flags O, U, V, W
/* 221 - O */  {.txt = STD_SPACE_6_PER_EM "O", .position = 9 + 28*12},
/* 222 - U */  {.txt = STD_SPACE_6_PER_EM "U", .position = 9 + 29*12},
/* 223 - V */  {.txt = STD_SPACE_6_PER_EM "V", .position = 8 + 30*12},
/* 224 - W */  {.txt = STD_SPACE_6_PER_EM "W", .position = 8 + 31*12},
};


#if !defined(TESTSUITE_BUILD)


  /********************************************//**
   * \brief The flag browser application
   *
   * \param[in] unusedButMandatoryParameter uint16_t
   * \return void
   ***********************************************/
  void flagBrowser(uint16_t init) {
  #if !defined(SAVE_SPACE_DM42_8FL)
    static int16_t line;
    int16_t f,i;
    bool_t firstFlag;

    hourGlassIconEnabled = false;

    if(calcMode != CM_FLAG_BROWSER) {
      if(calcMode == CM_AIM) {
        hideCursor();
        cursorEnabled = false;
      }

      previousCalcMode = calcMode;
      calcMode = CM_FLAG_BROWSER;
      clearSystemFlag(FLAG_ALPHA);
      currentFlgScr = init;                      //STATUS_SCREEN for flag STATUS, GLOBAL_FLAGS_SCREEN for FLGS
      refreshScreen(190);                        //Restart once, clearing screen and all, restarting flag browser, now in the correct mode
    }

    if(currentFlgScr < FIRST_SCREEN) currentFlgScr = LAST_SCREEN;
    if(currentFlgScr > LAST_SCREEN)  currentFlgScr = FIRST_SCREEN;

    if(currentFlgScr == STATUS_SCREEN) { // Init new style
      char flagNumber[4];

      line = 0;

      // Free memory
      //sprintf(tmpString + CHARS_PER_LINE * line++, "%" PRIu32 " bytes free in RAM, %" PRIu32 " in flash.", getFreeRamMemory(), getFreeFlash());
      sprintf(tmpString + CHARS_PER_LINE * line++, "%" PRIu32 " bytes free in RAM.", getFreeRamMemory());

      // Global flags
      sprintf(tmpString + CHARS_PER_LINE * line++, "Global user flags set:");

      tmpString[CHARS_PER_LINE * line] = 0;
      firstFlag = true;
      for(f=0; f<=FLAG_W; f++) {
        if(f>FLAG_K && f<FLAG_M) {
          continue;
        }

        if(getFlag(f)) {
          if(f < 10) {
            flagNumber[0] = '0' + f;
            flagNumber[1] = 0;
          }
          else if(f < 100) {
            flagNumber[0] = '0' + f/10;
            flagNumber[1] = '0' + f%10;
            flagNumber[2] = 0;
          }
          else {
            flagNumber[0] = registerFlagLetters[f <= FLAG_K ? f-FLAG_X : f-(FLAG_M-12)];
            flagNumber[1] = 0;
          }

          if(stringWidth(tmpString + CHARS_PER_LINE * line, &standardFont, true, true) + stringWidth(flagNumber, &standardFont, true, false) <= SCREEN_WIDTH - 1 - 8) { // SPACE is 8 pixel wide
            if(!firstFlag) {
              strcat(tmpString + CHARS_PER_LINE * line, " ");
            }
            else {
              firstFlag = false;
            }
            strcat(tmpString + CHARS_PER_LINE * line, flagNumber);
          }
          else {
            xcopy(tmpString + CHARS_PER_LINE * ++line, flagNumber, 4);
          }
        }
      }

      if(currentNumberOfLocalFlags == 0) {
        sprintf(tmpString + CHARS_PER_LINE * ++line, "No local flags or registers allocated.");
      }
      else {
        // Local flags
        sprintf(tmpString + CHARS_PER_LINE * ++line, "Local user flags set:");
        tmpString[CHARS_PER_LINE * ++line] = 0;
        firstFlag = true;
        for(f=0; f<NUMBER_OF_LOCAL_FLAGS; f++) {
          if(getFlag(FIRST_LOCAL_FLAG + f)) {
            if(f < 10) {
              flagNumber[0] = '0' + f;
              flagNumber[1] = 0;
            }
            else {
              flagNumber[0] = '0' + f/10;
              flagNumber[1] = '0' + f%10;
              flagNumber[2] = 0;
            }

            if(stringWidth(tmpString + CHARS_PER_LINE * line, &standardFont, true, true) + stringWidth(flagNumber, &standardFont, true, false) <= SCREEN_WIDTH - 1 - 8) { // SPACE is 8 pixel wide
              if(!firstFlag) {
                strcat(tmpString + CHARS_PER_LINE * line, " ");
              }
              else {
                firstFlag = false;
              }
              strcat(tmpString + CHARS_PER_LINE * line, flagNumber);
            }
            else {
              xcopy(tmpString + CHARS_PER_LINE * ++line, flagNumber, 4);
            }
          }
        }
      }

      // Empty line
      tmpString[CHARS_PER_LINE * ++line] = 0;

      // Rounding mode
      strcpy(tmpString + CHARS_PER_LINE * ++line, "RM=");
      switch(roundingMode) {
        case RM_HALF_EVEN: {
          strcat(tmpString + CHARS_PER_LINE * line, STD_ONE_HALF "E");
          break;
        }

        case RM_HALF_UP: {
          strcat(tmpString + CHARS_PER_LINE * line, STD_ONE_HALF STD_UP_ARROW);
          break;
        }

        case RM_HALF_DOWN: {
          strcat(tmpString + CHARS_PER_LINE * line, STD_ONE_HALF STD_DOWN_ARROW);
          break;
        }

        case RM_UP: {
          strcat(tmpString + CHARS_PER_LINE * line, STD_LEFT_ARROW "0" STD_RIGHT_ARROW);
          break;
        }

        case RM_DOWN: {
          strcat(tmpString + CHARS_PER_LINE * line, STD_RIGHT_ARROW "0" STD_LEFT_ARROW);
          break;
        }

        case RM_CEIL: {
          strcat(tmpString + CHARS_PER_LINE * line, STD_MAT_TL "x" STD_MAT_TR);
          break;
        }

        case RM_FLOOR: {
          strcat(tmpString + CHARS_PER_LINE * line, STD_MAT_BL "x" STD_MAT_BR);
          break;
        }

        default: {
          strcat(tmpString + CHARS_PER_LINE * line, "???");
        }
      }

      // Significant digits
      strcat(tmpString + CHARS_PER_LINE * line, "  SDIGS=");
      uint8_t sd = (significantDigits == 0 ? 34 : significantDigits);
      if(sd < 10) {
        flagNumber[0] = '0' + sd;
        flagNumber[1] = 0;
      }
      else {
        flagNumber[0] = '0' + sd/10;
        flagNumber[1] = '0' + sd%10;
        flagNumber[2] = 0;
      }
      strcat(tmpString + CHARS_PER_LINE * line, flagNumber);

      // ULP of X
      switch(getRegisterDataType(REGISTER_X)) {
        case dtLongInteger:
        case dtShortInteger: {
          strcat(tmpString + CHARS_PER_LINE * line, "  ULP of reg X = 1");
          break;
        }

        case dtReal34: {
          if(real34IsInfinite(REGISTER_REAL34_DATA(REGISTER_X))) {
            strcat(tmpString + CHARS_PER_LINE * line, "  ULP of reg X = " STD_INFINITY);
          }
          else {
            real34_t x34;
            real34NextPlus(REGISTER_REAL34_DATA(REGISTER_X), &x34);
            if(real34IsInfinite(&x34)) {
              real34NextMinus(REGISTER_REAL34_DATA(REGISTER_X), &x34);
              real34Subtract(REGISTER_REAL34_DATA(REGISTER_X), &x34, &x34);
            }
            else {
              real34Subtract(&x34, REGISTER_REAL34_DATA(REGISTER_X), &x34);
            }
            strcat(tmpString + CHARS_PER_LINE * line, "  ULP of reg X = 10");
            supNumberToDisplayString(real34GetExponent(&x34), tmpString + CHARS_PER_LINE * line + strlen(tmpString + CHARS_PER_LINE * line), NULL, false);
          }
          break;
        }

        default: {
        }
      }
      tmpString[CHARS_PER_LINE * ++line] = 0;
    }


    if(currentFlgScr == STATUS_SCREEN) {
      for(f=0; f<min(9, line); f++) {
        showString(tmpString + CHARS_PER_LINE * f, &standardFont, 1, 22*f + 43, vmNormal, true, false);
      }
    }

    if(currentFlgScr == SYSTEM_FLAGS_SCREEN_1 || currentFlgScr == SYSTEM_FLAGS_SCREEN_2) {  // System Flags 00 to 59
      extern int16_t menu_SYSFL[];
      uint16_t systemFlag;
      uint16_t param;
      uint16_t x1, x2, y1, fOffset = currentFlgScr == SYSTEM_FLAGS_SCREEN_1?0:60;
      for(f=0; f<=59; f++) {
        if (f+fOffset > NUMBER_OF_SYSTEM_FLAGS - 1) {
          break;
        }
        systemFlag = menu_SYSFL[f+fOffset];
        param = indexOfItems[systemFlag].param;
        x1 = KEY_X[f%6]+2;
        x2 = KEY_X[f%6+1]-1;
        y1 = 22*(f/6)+66-1-44;
        if(getSystemFlag(param)) {
          lcd_fill_rect(
            x1, 
            y1,  
            x2-x1,
            22*(f/6)+66+20-1-44-y1+1,
            LCD_EMPTY_VALUE);
        }
        sprintf(tmpString, "%s", indexOfItems[systemFlag].itemCatalogName);
        showString(tmpString, &standardFont, (x1 + x2 - stringWidth(tmpString, &standardFont, false, false))/2, y1, getSystemFlag(param) ? vmReverse : vmNormal, true, true); //JM-44
      }
    }

    if(currentFlgScr == GLOBAL_FLAGS_SCREEN) { // Global flags from 0 to 99
      //clearScreen(false, true, true);

      for(f=0; f<=99/*79*/; f++) {                                          //JM 99
        if(getFlag(f)) {
          lcd_fill_rect(40*(f%10)+1,22*(f/10)+66-1-44,  40*(f%10)+39-(40*(f%10)+1),22*(f/10)+66+20-1-44-(22*(f/10)+66-1-44)+1,LCD_EMPTY_VALUE);
        }
        sprintf(tmpString, "%d", f);
        showString(tmpString, &standardFont, 40*(f%10) + 19 - stringWidth(tmpString, &standardFont, false, false)/2, 22*(f/10)+66-1-44, getFlag(f) ? vmReverse : vmNormal, true, true); //JM-44
      }
    }

    if(currentFlgScr == LETTERED_AND_LOCAL_FLAGS_SCREEN) { // Flags from FLAG_X to FLAG_W, local registers and local flags
      //clearScreen(false, true, true);

      showString("Global flag status (continued):", &standardFont, 1, 22-1, vmNormal, true, true);

      // Lettered flags
      for(f=FLAG_X; f<=FLAG_W; f++) {
        f = (f == FLAG_K + 1 ? FLAG_M : f);            // Skip from FLAG_K (111) to FLAG_M (211)
        i = (f <= FLAG_K ? f-FLAG_X : f-FLAG_X - 99);  // Index in the flag display table
        showString(letteredFlagDisplay[i].txt, &standardFont, letteredFlagDisplay[i].position, 43, getFlag(f) ? vmReverse : vmNormal, true, true);
      }

      showString("[" STD_SPACE_6_PER_EM "  100..108   " STD_SPACE_3_PER_EM STD_SPACE_6_PER_EM "109..111  " STD_SPACE_3_PER_EM "211..216  217..220" STD_SPACE_6_PER_EM " 221..224]"
                 , &standardFont, 1, 44+66-1-44, vmNormal, true, true);

      if(currentNumberOfLocalFlags == 0) {
        sprintf(tmpString, "No local flags or registers allocated.");
        showString(tmpString, &standardFont, 1, 109, vmNormal, true, true);
      }
      else {
        // Local Registers
        sprintf(tmpString, "%" PRIu8 " local register%s allocated.", currentNumberOfLocalRegisters, currentNumberOfLocalRegisters > 1 ? "s": "");
        showString(tmpString, &standardFont, 1, 109, vmNormal, true, true);
        showString("Local flag status:", &standardFont, 1, 153, vmNormal, true, true);


        // Local Flags
        for(f=0; f<NUMBER_OF_LOCAL_FLAGS; f++) {
          if(getFlag(FIRST_LOCAL_FLAG + f)) {
            lcd_fill_rect(25*(f%16)+1, 22*(f/16)+175, 22, 21,  LCD_EMPTY_VALUE);
          }

          sprintf(tmpString, "%d", f);
          showString(tmpString, &standardFont, 25*(f%16)+5+4*(f<=9), 22*(f/16)+175, getFlag(FIRST_LOCAL_FLAG + f) ? vmReverse : vmNormal, true, true);     //JM-44
        }
      }
    }
    lastFlgScr = currentFlgScr;
  #endif // !SAVE_SPACE_DM42_8FL
  }
#endif // !TESTSUITE_BUILD
