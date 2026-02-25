// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors


//outstanding list:
//=================
//check why ypos-2 must be used to prevent clearing ofnthe ASM line.
//check the "" in some showStringAndClear statements
//look at the "", cases
//'todo' items
// Returnning from a LI SHOW, stack not updated



#include "c47.h"
#if defined(PC_BUILD) && defined(ANALYSE_REFRESH)
  #include <execinfo.h>
#endif // PC_BUILD &&MONITOR_CLRSCR



void drawBattery(uint16_t voltage);

#ifdef PC_BUILD
  void mockupSB(void);
#endif

#if !defined(TESTSUITE_BUILD)

  uint8_t  SBlastIntegerBaseShown = 0xFF;
  uint16_t SBAlphaModeLastShown = 0xFFFF;
  char     SBhourglassShown[2];
  char     alphaOutput[3];
  bool_t   reInstateIntegerModeDisplay;
  bool_t   reInstateOCModeDisplay;

  void forceSBupdate(void) {                   // note set all SB activation/change indicator flags to 'changed'
                                #if defined(ANALYSE_REFRESH)
                                  print_caller(NULL);
                                #endif // ANALYSE_REFRESH
    setAllSystemFlagChanged();
    SBlastIntegerBaseShown = 0xFF;
    SBAlphaModeLastShown = 0xFFFF;
    SBhourglassShown[0]  = 0xFF;
    SBhourglassShown[1]  = 0xFF;               // note terminating 0 never used. Byte comparison done on content only.
    oldTime[0] = 0;
    alphaOutput[0] = 0xFF;
    alphaOutput[1] = 0xFF;
    alphaOutput[2] = 0;
    reInstateIntegerModeDisplay = false;
    reInstateOCModeDisplay = false;
  }


  #define timeHasChanged true
  bool_t timeChanged(void) {
    char timeString[8];
    getTimeString(timeString);
    if(strcmp(timeString, oldTime) != 0 || oldTime[0] == 0) {     // not equal
      strcpy(oldTime, timeString);
      return timeHasChanged;
    } else {
      return !timeHasChanged;                  // returns the strcmp(dateTimeString, oldTime) comparison
    }
  }



//Note: in PC_BUILD this is called several times per second, continuously
  bool_t showDateTime(void) {
    if(timeChanged() == !timeHasChanged) {     // creates oldTime here
      return false;
    }
    if(programRunStop == PGM_RUNNING) {
      //printf("RUNNING, NO TIME/DATE SBI PRINTED\n");
      return false;
    }

    #if (DEBUG_INSTEAD_STATUS_BAR == 1)
      return false;
    #endif // (DEBUG_INSTEAD_STATUS_BAR == 1)

    if(!((SBARUPD_Date) || (SBARUPD_Time) || (SBARUPD_WoY))) {
      return false;
    }
    //printf("TIME/DATE SBI PRINTED\n");

    uint32_t x = X_DATE;
    lcd_fill_rect(0, 0, x - 0, 20, LCD_SET_VALUE);

    if(SBARUPD_Date || SBARUPD_WoY) {
      getDateString(dateTimeString);
    }

    if((dateTimeString[0] >= '0' || dateTimeString[0] <= '9')) {                      // not yet initialized, senseless to continue
      if(SBARUPD_Date) {
        x = showString(dateTimeString, &standardFont, x, 0, vmNormal, true, true);
      }
      else {
        lcd_fill_rect(x, 0, X_TIME - x, 20, LCD_SET_VALUE);
        x = X_TIME;
      }
      if(SBARUPD_Time) {
        x = showGlyph(getSystemFlag(FLAG_TDM24) ? " " : STD_SPACE_3_PER_EM, &standardFont, x, 0, vmNormal, true, true, false); // is 0+0+8 pixel wide
        x = showString(oldTime, &standardFont, x, 0, vmNormal, true, false);
      }
      if(SBARUPD_WoY) {
        x = showGlyph(STD_SPACE_3_PER_EM, &standardFont, x, 0, vmNormal, true, true, false);
        getWeekOfYearString(dateTimeString);
        x = showString(dateTimeString, &standardFont, x, 0, vmNormal, true, false);
      }
    }

    lcd_fill_rect(x, 0, X_REAL_COMPLEX - x, 20, LCD_SET_VALUE);


    if(Y_SHIFT == 0 && X_SHIFT < 200) {
      showShiftState();
    }
    return true;
  }


  static void showRealComplexResult(void) {
    if(!(SBARUPD_ComplexResult)) return;
    int32_t x = X_REAL_COMPLEX;
    if (didSystemFlagChange(FLAG_CPXRES)) {
      x = showGlyph(getSystemFlag(FLAG_CPXRES) ? STD_COMPLEX_C : STD_REAL_R, &standardFont, x, 0, vmNormal, true, false, false); // Complex C is 0+8+3 pixel wide
      lcd_fill_rect(x, 0, (SBARUPD_ComplexResult ? X_COMPLEX_MODE : X_COMPLEX_MODE + X_COMPLEX_MODE_ADJ) - x, 20, LCD_SET_VALUE);
    }
  }


  static void showComplexMode(void) {
    if(!(SBARUPD_ComplexMode)) return;
    int32_t x =  SBARUPD_ComplexResult ? X_COMPLEX_MODE : X_COMPLEX_MODE + X_COMPLEX_MODE_ADJ;
    if (didSystemFlagChange(FLAG_POLAR)) {
      x = showGlyph(getSystemFlag(FLAG_POLAR) ? STD_SUN : STD_RIGHT_ANGLE, &standardFont, x, 0, vmNormal, true, true, false); // Sun         is 0+12+2 pixel wide
      lcd_fill_rect(x, 0, X_ANGULAR_MODE - x, 20, LCD_SET_VALUE);
    }
  }


  static void showAngularMode(void) {
    if(!((SBARUPD_AngularModeBasic) | (SBARUPD_AngularMode))) return;

    uint32_t x = X_ANGULAR_MODE;
    if(didSystemFlagChange(SETTING_AMODE)) {

      if(SBARUPD_AngularModeBasic) {
        x = showGlyph(STD_MEASURED_ANGLE, &standardFont, x, 0, vmNormal, true, true, false); // Angle is 0+9 pixel wide
      }

      switch(currentAngularMode) {
        case amRadian: {
          x = showGlyph(STD_SUP_BOLD_r,         &standardFont, x, 0, vmNormal, true, false, false); // r  is 0+6 pixel wide
          break;
        }

        case amMultPi: {
          x = showGlyph(STD_SUP_pir,            &standardFont, x, 0, vmNormal, true, false, false); // pi is 0+9 pixel wide
          break;
        }

        case amGrad: {
          x = showGlyph(STD_SUP_BOLD_g,         &standardFont, x, 0, vmNormal, true, false, false); // g  is 0+6 pixel wide
          break;
        }

        case amDegree: {
          x = showGlyph(STD_DEGREE,             &standardFont, x, 0, vmNormal, true, false, false); // °  is 0+6 pixel wide
          break;
        }

        case amDMS: {
          x = showGlyph(STD_RIGHT_DOUBLE_QUOTE, &standardFont, x, 0, vmNormal, true, false, false); // "  is 0+6 pixel wide
          break;
        }

          default: {
            x = showGlyph(STD_QUESTION_MARK,      &standardFont, x, 0, vmNormal, true, false, false); // ?
          }
      }
      lcd_fill_rect(x, 0, X_FRAC_MODE - x, 20, LCD_SET_VALUE);
    }
  }


  //Share Frac Mode space
  static bool_t showBaseMode(void) {
    if(!(SBARUPD_FractionModeAndBaseMode)) return false;

    bool_t  SBchanged = false;
    if(lastIntegerBase + ((lastIntegerBase >= 2 && didSystemFlagChange(FLAG_TOPHEX)) ? 0x40 : 0) != SBlastIntegerBaseShown) {
      SBlastIntegerBaseShown = lastIntegerBase + ((lastIntegerBase >= 2 && didSystemFlagChange(FLAG_TOPHEX)) ? 0x40 : 0);
      SBchanged = true;
    }

    if(SBchanged) {
      uint32_t x = X_BASE_MODE;
      if(lastIntegerBase >= 2) {
        if(getSystemFlag(FLAG_TOPHEX)) {
          x = showString("#KEY", &standardFont, x, 0 , vmNormal, true, true);
          lcd_fill_rect(x, 0, 26, 20, LCD_SET_VALUE);
          x = showString(STD_SUB_A STD_SUB_MINUS STD_SUB_F,  &standardFont, x, -4 , vmNormal, true, true);
        }
        else {
          x = showString("#BASE", &standardFont, x, 0, vmNormal, true, true);
        }
        lcd_fill_rect(x, 0, X_INT_MX_TVM_MODE - x, 20, LCD_SET_VALUE);
        return true;
      }
    }
    return false;
  }


  #define lowerUnderLine ((calcMode == CM_REGISTER_BROWSER || calcMode == CM_FLAG_BROWSER) ? 0 : 2)   //lower the /1200x a few pixels to create to idea of under the line

  void showFracMode(void) {
    if(!(SBARUPD_FractionModeAndBaseMode)) return;

    #if (DEBUG_INSTEAD_STATUS_BAR == 1)
      return;
    #endif // (DEBUG_INSTEAD_STATUS_BAR == 1)

    if(lastIntegerBase >= 2) {
      showBaseMode();
      return;
    }

    if(didSystemFlagChange(FLAG_FRACT)  || didSystemFlagChange(FLAG_IRFRAC) || didSystemFlagChange(FLAG_PROPFR) || 
       didSystemFlagChange(SETTING_DMX) || didSystemFlagChange(FLAG_DENFIX) || didSystemFlagChange(FLAG_DENANY)) {

      char statusMessage[20];
      uint32_t x = X_FRAC_MODE;

      //a b/c
      char divStr[10];
      if(getSystemFlag(FLAG_FRACT) || getSystemFlag(FLAG_IRFRAC)) {
        if(getSystemFlag(FLAG_PROPFR)) {
          lcd_fill_rect(x, 0, 11, 20, LCD_SET_VALUE);
          raiseString = 3;
          x = showString("a" STD_SPACE_4_PER_EM, &standardFont, x, 0, vmNormal, true, true) - 3;
          lcd_fill_rect(x-11, 17, 11, 2, LCD_SET_VALUE);
        }
        lcd_fill_rect(x, 0, 7+15, 20, LCD_SET_VALUE);
        raiseString = 9;
        x = showString(STD_SUB_b, &standardFont, x, 0, vmNormal, true, true) - 2-2;
      } else {
        lcd_fill_rect(x, 0, 15, 20, LCD_SET_VALUE);      
      }

      int xxSlash = x;
      x += 8;

      if(lowerUnderLine-1 > 0) {
        lcd_fill_rect(xxSlash, 0, x - xxSlash, lowerUnderLine-1, LCD_SET_VALUE);
      }

      compressString = 1;
      if(denMax == 0 || denMax > MAX_DENMAX) {
        sprintf(statusMessage,"max");
      } else {
        sprintf(statusMessage, "%" PRIu32,denMax);
      }
      int xx = x;
      x = showString(statusMessage, &standardFont, x, lowerUnderLine, vmNormal, false, true);
      if(lowerUnderLine > 0) {
        lcd_fill_rect(xx+1, 0, x-xx, lowerUnderLine, LCD_SET_VALUE);
      }

      if(!getSystemFlag(FLAG_IRFRAC) && getSystemFlag(FLAG_DENFIX)) {
        raiseString = 3;
        lcd_fill_rect(++x, 0, 11, 20, LCD_SET_VALUE);
        x = showString(STD_SUB_f, &standardFont, x, lowerUnderLine, vmNormal, true, true);
      }

      if((getSystemFlag(FLAG_IRFRAC)) || (!getSystemFlag(FLAG_IRFRAC) && !getSystemFlag(FLAG_DENFIX) && !getSystemFlag(FLAG_DENANY))) {
        strcpy(divStr,PRODUCT_SIGN);
        lcd_fill_rect(++x, 0, 11, 20, LCD_SET_VALUE);
        raiseString = 2;
        x = showString(divStr, &standardFont, x, lowerUnderLine, vmNormal, true, true) + (getSystemFlag(FLAG_MULTx) ? 0 : 2);
      }

      if(getSystemFlag(FLAG_IRFRAC)) {
        strcpy(divStr,STD_IRRATIONAL_I);
        lcd_fill_rect(x, 0, 9, 20, LCD_SET_VALUE);
        raiseString = 1;
        x = showString(divStr, &standardFont, x, 0, vmNormal, false, false) + 2;
      }

      if((getSystemFlag(FLAG_IRFRAC) || getSystemFlag(FLAG_FRACT)) && (fractionDigits > 0 && fractionDigits < 34)) {
        compressString = 1;
        x = showString(STD_ALMOST_EQUAL, &standardFont, ++x - 1, 0, vmNormal, true, false);
        if(x >= X_INT_MX_TVM_MODE - 1) {
          lcd_fill_rect(X_INT_MX_TVM_MODE - 1, 0, 1, 20, LCD_SET_VALUE);        
        }
      }

      if(x <= X_INT_MX_TVM_MODE) {
        lcd_fill_rect(x, 0, X_INT_MX_TVM_MODE - x, 20, LCD_SET_VALUE);
      }

      plotline2(xxSlash,  18, xxSlash+9,0);

    }
  }


  static int32_t showStringAndClear(const char *str, const font_t *font, uint32_t x, int32_t y,  uint32_t dx, uint32_t dy, videoMode_t videoMode, bool_t showLeadingCols, bool_t showEndingCols) {
     //raiseString = raise;

     if(str[0] == 0) {
       lcd_fill_rect(x, 0, dx, 20, LCD_SET_VALUE);
       return x;
     }

     uint32_t col, row;
     getStringBounds(str, font, &col, &row);

     //show annunciator message
     uint32_t xx = showString(str, font, x, y, videoMode, showLeadingCols, showEndingCols);

     //clear right of message to end of allocated space
     if(xx < x+dx) {
       lcd_fill_rect(xx, max(0,y/*-raise*/), x+dx -xx, max(row,dy)/*-raise*/ + min(0,y), LCD_SET_VALUE);
       //printf("%s ### x=%u y=%i dx=%u dy=%u   xx=%u dd=%i \n", str, x, y, dx, dy, xx, x+dx -xx);
     }
     //clear slither below lifted text
     if(y < 0) {
       lcd_fill_rect(x, (row + y), dx, dx - row, LCD_SET_VALUE);
     }
     //clear slither above dropped text
     else if(y > 0) {
       lcd_fill_rect(x, 0, dx, y-0, LCD_SET_VALUE);
     }
                                #if defined(ANALYSE_REFRESH)
                                  print_caller(NULL);
                                #endif // PC_BUILD && ANALYSE_REFRESH
     return xx;
  }


//sharing space with Integermode
  static bool_t showMatrixMode(void) {
    if(!(SBARUPD_MatrixMode)) return false;
    bool_t enable = calcMode == CM_MIM;// || didSystemFlagChange(FLAG_GROW);
    if(enable) {
      reInstateOCModeDisplay = true;
      reInstateIntegerModeDisplay = true;
      compressString = 1;
      showStringAndClear(getSystemFlag(FLAG_GROW) ? "grow" : "wrap", &standardFont, X_INT_MX_TVM_MODE, 0, X_ALPHA_MODE - X_INT_MX_TVM_MODE, 20, vmNormal, true, true);
    }
    return enable;
  }

//sharing space with Integermode
  static bool_t showTvmMode(void) {
    if(!(SBARUPD_TVMMode)) return false;
    bool_t enable = softmenu[softmenuStack[0].softmenuId].menuItem == -MNU_TVM || softmenu[softmenuStack[0].softmenuId].menuItem == -MNU_FIN;
    if(enable) {
      reInstateOCModeDisplay = true;
      reInstateIntegerModeDisplay = true;
      compressString = 1;
      showStringAndClear(getSystemFlag(FLAG_ENDPMT) ? "END " : "BEG ", &standardFont, X_INT_MX_TVM_MODE, 0, X_ALPHA_MODE - X_INT_MX_TVM_MODE, 20, vmNormal, true, true);
    }
    return enable;
  }

  static bool_t showIntegerMode(void) {
    bool_t aa = didSystemFlagChange(SETTING_SINT_WS);    //note, separately to prevent compiler short circuiting second term
    bool_t bb = didSystemFlagChange(SETTING_SINT_MODE);
    if(!(SBARUPD_IntegerMode)) return false;
    if( aa || bb || reInstateIntegerModeDisplay) {
      reInstateIntegerModeDisplay = false;
      char statusMessage[10];
      sprintf(statusMessage, "%s%" PRIu8 ":%c", shortIntegerWordSize <= 9 ? " " : "", shortIntegerWordSize, shortIntegerMode==SIM_1COMPL?'1':(shortIntegerMode==SIM_2COMPL?'2':(shortIntegerMode==SIM_UNSIGN?'u':(shortIntegerMode==SIM_SIGNMT?'s':'?'))));
      strcat(statusMessage," ");
      showStringAndClear(statusMessage, &standardFont, X_INT_MX_TVM_MODE, 0, X_OVERFLOW_CARRY - X_INT_MX_TVM_MODE, 20, vmNormal, true, true);
      return true;
    } else {
      return false;
    }
  }

  static bool_t showOverflowCarry(void) {
    bool_t aa = didSystemFlagChange(FLAG_OVERFLOW);    //note, separately to prevent compiler short circuiting second term
    bool_t bb = didSystemFlagChange(FLAG_CARRY);
    if(!(SBARUPD_OCCarryMode)) return false;
    if (aa || bb || reInstateOCModeDisplay) {
      reInstateOCModeDisplay = false;
      showStringAndClear(STD_OVERFLOW_CARRY, &standardFont, X_OVERFLOW_CARRY, 0, 6 /*X_ALPHA_MODE - X_OVERFLOW_CARRY*/, 20, vmNormal, true, true);
      if(!getSystemFlag(FLAG_OVERFLOW)) { // Overflow flag is cleared
        lcd_fill_rect(X_OVERFLOW_CARRY, 2, 6, 7, LCD_SET_VALUE);
      }
      if(!getSystemFlag(FLAG_CARRY)) { // Carry flag is cleared
        lcd_fill_rect(X_OVERFLOW_CARRY, 12, 6, 7, LCD_SET_VALUE);
      }
      return true;
    } else {
      return false;
    }
  }

  static void clearINT_MX_TVM_MODE(void) {
    compressString = 1;
    showStringAndClear("    ", &standardFont, X_INT_MX_TVM_MODE, 0, X_ALPHA_MODE - X_INT_MX_TVM_MODE, 20, vmNormal, true, true);
  }

//todo remove SETT_AlphaMode replace with didSystemFlagChange
  #define SETT_AlphaMode (uint16_t)((nextChar         ) \
                                  + (scrLock      << 10) \
                                  + (alphaCase    << 12) \
                                  + (shiftF       << 13) \
                                  + (shiftG       << 14))
 
  void showHideAlphaMode(void) {
    #if (DEBUG_INSTEAD_STATUS_BAR == 1)
      return;
    #endif // (DEBUG_INSTEAD_STATUS_BAR == 1)

    bool_t textModeIconDisplay = ((plainTextMode || calcMode == CM_EIM || (catalog && catalog != CATALOG_MVAR) || (tam.mode != 0 && tam.alpha)));
    bool_t toSwitchOff         = !textModeIconDisplay && alphaOutput[0] != 0;

    if(!(SBARUPD_AlphaMode) || calcMode == CM_GRAPH) return;
    bool_t SBchanged;
    SBchanged = false;
    if(SBAlphaModeLastShown != SETT_AlphaMode) {
       SBAlphaModeLastShown = SETT_AlphaMode;
       SBchanged = true;
    }
    if(didSystemFlagChange(FLAG_alphaCAP) || didSystemFlagChange(FLAG_NUMLOCK) || SBchanged || toSwitchOff || textModeIconDisplay) {

      int status=0;
      uint8_t nChar;
      if(scrLock == NC_NORMAL) {
        nChar = nextChar;
      }
      else {
        nChar = scrLock;
      }
      if(textModeIconDisplay) {
        #if defined (PC_BUILD)
          if(deadKey != 0) {
            status = 20;
          } else
        #endif

        if(plainTextMode) {                  //TODO this flag's purpose must be checked
          if(alphaCase == AC_UPPER) {
            setSystemFlag(FLAG_alphaCAP);
          } else {
            clearSystemFlag(FLAG_alphaCAP);
          }
        }

        if (getSystemFlag(FLAG_NUMLOCK) && !shiftF && !shiftG) {
          if (alphaCase == AC_UPPER) {
                                                                        status =  3 - (nChar == NC_SUBSCRIPT ? 2 : nChar == NC_SUPERSCRIPT ? 1 : 0);
          } else if (alphaCase == AC_LOWER) {
                                                                        status =  6 - (nChar == NC_SUBSCRIPT ? 2 : nChar == NC_SUPERSCRIPT ? 1 : 0);
          }
        } else if (alphaCase == AC_LOWER && shiftF) {
                                                                        status = 12 - (nChar == NC_SUBSCRIPT ? 2 : nChar == NC_SUPERSCRIPT ? 1 : 0); //A
        } else if (alphaCase == AC_UPPER && shiftF) {
                                                                        status = 18 - (nChar == NC_SUBSCRIPT ? 2 : nChar == NC_SUPERSCRIPT ? 1 : 0); //a
        } else { //at this point shiftF is false
          if (alphaCase == AC_UPPER) { //UPPER
            if (shiftG) {
                                                                        status =  3 - (nChar == NC_SUBSCRIPT ? 2 : nChar == NC_SUPERSCRIPT ? 1 : 0);
            } else if (!shiftG && !shiftF && !getSystemFlag(FLAG_NUMLOCK)) {
                                                                        status = 12 - (nChar == NC_SUBSCRIPT ? 2 : nChar == NC_SUPERSCRIPT ? 1 : 0);
            }
          } else if (alphaCase == AC_LOWER) { //LOWER
            if (shiftG) {
                                                                        status =  3 - (nChar == NC_SUBSCRIPT ? 2 : nChar == NC_SUPERSCRIPT ? 1 : 0);
            } else if (!shiftG && !shiftF && !getSystemFlag(FLAG_NUMLOCK)) {
                                                                        status = 18 - (nChar == NC_SUBSCRIPT ? 2 : nChar == NC_SUPERSCRIPT ? 1 : 0);
            }
          }
        }
      }
      int32_t ypos = 0;
      alphaOutput[0]=0;
      switch(status) {
        case  1: strcpy(alphaOutput, STD_SUB_N); ypos =  -2; break;
        case  2: strcpy(alphaOutput, STD_SUP_N); ypos =   0; break;
        case  3: strcpy(alphaOutput, STD_num  ); ypos =   0; break;
        case  4: strcpy(alphaOutput, STD_SUB_n); ypos =  -2; break;
        case  5: strcpy(alphaOutput, STD_SUP_n); ypos =  -2; break;
        case  6: strcpy(alphaOutput, STD_n    ); ypos =   0; break;
        case 10: strcpy(alphaOutput, STD_SUB_A); ypos =  -2; break;
        case 11: strcpy(alphaOutput, STD_SUP_A); ypos =   0; break;
        case 12: strcpy(alphaOutput, STD_A    ); ypos =   0; break;
        case 16: strcpy(alphaOutput, STD_SUB_a); ypos =  -2; break;
        case 17: strcpy(alphaOutput, STD_SUP_a); ypos =  -2; break;
        case 18: strcpy(alphaOutput, STD_a    ); ypos =   0; break;
        #if defined(PC_BUILD)
        case 20: strcpy(alphaOutput, STD_BOX  ); ypos =   0; break; //deadkey
        #endif //PC_BUILD
          default:;
      }
      showStringAndClear(alphaOutput, &standardFont, X_ALPHA_MODE, ypos, X_HOURGLASS - X_ALPHA_MODE, 18, vmNormal, true, true);
    }
  }





//todo check if it properly works on hardware with running programs
  void showHideHourGlass(void) {
    if(!(SBARUPD_HourGlass)) {
      return;
    }

    #if (DEBUG_INSTEAD_STATUS_BAR == 1)
      return;
    #endif // (DEBUG_INSTEAD_STATUS_BAR == 1)

    if(screenUpdatingMode & SCRUPD_MANUAL_STATUSBAR) {      // force statusbar display for these modes
      switch(calcMode) {
        case CM_PEM:
        case CM_REGISTER_BROWSER:
        case CM_FLAG_BROWSER:
        case CM_ASN_BROWSER:
        case CM_FONT_BROWSER:
        case CM_PLOT_STAT:
        case CM_CONFIRMATION:
        case CM_MIM:
        case CM_TIMER:
        case CM_GRAPH: {
          screenUpdatingMode &= ~SCRUPD_MANUAL_STATUSBAR;
          break;
        }

        default: {
          return;
        }
      }
    }

    char statusMessage[4];
    statusMessage[0]=0;
    statusMessage[1]=0;
    statusMessage[2]=0;
    statusMessage[3]=0;
    int32_t offs = 0;
    int32_t yoffs = 0;
    switch(programRunStop) {
      case PGM_WAITING: {
        strcpy(statusMessage,STD_NEG_EXCLAMATION_MARK);
        offs = 0;
        break;
      }
      case PGM_RUNNING: {
        sprintf(statusMessage, STD_SPACE_HAIR STD_P);
        offs = +1;
        break;
      }
      default: {
        if(hourGlassIconEnabled) {
          strcpy(statusMessage,STD_HOURGLASS_WH);
          offs = +1;
          yoffs = +5;
        }
      }
    }
    //statusMessage is now 0/!/P/H
    #define XX  (GRAPHMODE ? X_HOURGLASS_GRAPHS : X_HOURGLASS)
    #define XXX (GRAPHMODE ? ((SBARUPD_ComplexResult ? X_COMPLEX_MODE : X_COMPLEX_MODE + X_COMPLEX_MODE_ADJ)) : X_SSIZE_BEGIN)
    if((SBhourglassShown[0] != statusMessage[0] || SBhourglassShown[1] != statusMessage[1])  ) {
      SBhourglassShown[0] = statusMessage[0];
      SBhourglassShown[1] = statusMessage[1];
      showStringAndClear(statusMessage, &standardFont, XX + offs, yoffs, XXX - XX, 20, vmNormal, false, false);
      force_SBrefresh(force);
    }
  }


  static void showStackSize(void) {
    if(!(SBARUPD_StackSize)) return;
    if(didSystemFlagChange(FLAG_SSIZE8)) {
      showStringAndClear(getSystemFlag(FLAG_SSIZE8) ? STD_SPACE_6_PER_EM STD_8 : STD_SPACE_6_PER_EM STD_4, &standardFont, X_SSIZE_BEGIN, 0, X_ASM - X_SSIZE_BEGIN, 20, getSystemFlag(FLAG_ERPN) ? vmNormal : vmReverse, false, true);
    }
  }

//sharing space with stopwatch, so ASM does not come when the stopwatch is on
  void light_ASB_icon(void) {
    if(!(SBARUPD_AlphaMode) || calcMode == CM_GRAPH) return;
    #if (DEBUG_INSTEAD_STATUS_BAR == 1)
      return;
    #endif // (DEBUG_INSTEAD_STATUS_BAR == 1)
    lcd_fill_rect(X_ALPHA_MODE,18,9,2,LCD_EMPTY_VALUE);        //underline the alha mode character, AND show the asmBuffer as well
    //compressString = 1; //do not use compress, as the far edges of the letter get cut off
    if(!watchIconEnabled) {
      showStringAndClear(asmBuffer, &standardFont, X_ASM, 0, X_SERIAL_IO - X_ASM, 20, vmNormal, true, false);
    }
    if(programRunStop != PGM_RUNNING) {
      force_SBrefresh(force);
    }
  }


//sharing space with stopwatch, so ASM does not come when the stopwatch is on
  void kill_ASB_icon(void) {
    if(!(SBARUPD_AlphaMode) || calcMode == CM_GRAPH) return;
    #if (DEBUG_INSTEAD_STATUS_BAR == 1)
      return;
    #endif // (DEBUG_INSTEAD_STATUS_BAR == 1)
    lcd_fill_rect(X_ALPHA_MODE,18,9,2,LCD_SET_VALUE);        //underline the alha mode character, AND show the asmBuffer as well
    if(!watchIconEnabled) {
      lcd_fill_rect(X_ASM, 0, X_SERIAL_IO - X_ASM, 20, LCD_SET_VALUE);
    }
    if(programRunStop != PGM_RUNNING) {
      force_SBrefresh(force);
    }
  }


  static void showHideWatch(void) {
    if(!(SBARUPD_StopWatch)) return;

    #if !defined(TESTSUITE_BUILD)
      if(watchIconEnabled != (timerStartTime != TIMER_APP_STOPPED)) {
        setSystemFlagChanged(SETTING_WATCHICON);
        watchIconEnabled = !watchIconEnabled;
      }
    #endif // !TESTSUITE_BUILD

    if(didSystemFlagChange(SETTING_WATCHICON)) {
      showStringAndClear(watchIconEnabled ? STD_TIMER : "", &standardFont, X_STOPWATCH, 0, X_SERIAL_IO - X_STOPWATCH, 20, vmNormal, true, false );
    }
  }


//NOTE for FUTURE: WHEN printerIconEnabled changed, do setSystemFlagChanged(SETTING_SIOICON) otherwise it will not display the icon. Search for the example of SETTING_WATCHICON & watchIconEnabled
  static void showHideSerialIO(void) {
    if(!(SBARUPD_SerialIO)) return;
    if(didSystemFlagChange(SETTING_SIOICON)) {
        showStringAndClear(serialIOIconEnabled ? STD_SERIAL_IO : "", &standardFont, X_SERIAL_IO, 0, X_PRINTER - X_SERIAL_IO, 20, vmNormal, true, false );
    }
  }


//NOTE for FUTURE: WHEN printerIconEnabled changed, do setSystemFlagChanged(SETTING_PRINTERICON) otherwise it will not display the icon. Search for the example of SETTING_WATCHICON & watchIconEnabled
  static void showHidePrinter(void) {
    if(didSystemFlagChange(SETTING_PRINTERICON)) {
        showStringAndClear(printerIconEnabled ? STD_PRINTER : "", &standardFont, X_PRINTER, 0, X_USER_MODE - X_PRINTER, 20, vmNormal, true, false );
      }
  }


  static void showHideUserMode(void) {
    if(!(SBARUPD_UserMode)) return;
    if(didSystemFlagChange(FLAG_USER)) {
        showStringAndClear(getSystemFlag(FLAG_USER) ? STD_USER_MODE : "", &standardFont, X_USER_MODE, 0, X_BATTERY - X_USER_MODE, 20, vmNormal, false, false );
        refreshModeGui();
    }
  }


//todo make it check the last voltage plotted, and bypass if nothing has changed
void drawBattery(uint16_t voltage) {
  #if (DEBUG_INSTEAD_STATUS_BAR == 1)
    return;
  #endif // (DEBUG_INSTEAD_STATUS_BAR == 1)
  lcd_fill_rect(X_BATTERY, 0, 11, 20, LCD_SET_VALUE);
  uint16_t vv = (uint16_t)(min(max(voltage - 2000,0),3100) / (float)(((float)3100 - 2000.0f)/(float)(DY_BATTERY))); //draw a battery, full at 3.1V empty at 2V
  for(uint16_t ii = min(vv-1,DY_BATTERY-1); ii <= DY_BATTERY-1; ii++) {
    if(ii%2 == 0) { //draw outline
      setBlackPixel(ii < DY_BATTERY-3 ?  X_BATTERY + 0 : X_BATTERY + 2                           ,(DY_BATTERY-1)-ii);
      setBlackPixel(ii < DY_BATTERY-3 ?  X_BATTERY + DX_BATTERY + 0 : X_BATTERY + DX_BATTERY - 2 ,(DY_BATTERY-1)-ii);
    }
  }
  for(uint16_t ii = 0; ii <= min(vv,DY_BATTERY-1); ii++) { //draw voltage
    for(uint16_t jj = 0; jj <= DX_BATTERY; jj++) {
      if(min(vv,DY_BATTERY)-ii > (voltage > 2750 ? 2 : 1) || (jj>1 && jj<DX_BATTERY-1)) {
        setBlackPixel(X_BATTERY + jj, (DY_BATTERY-1)-ii);
      }
    }
  }
}


//todo make it bypass if nothing has changed
  #if defined(DMCP_BUILD)
    void showHideUsbLowBattery(void) {
      if(!(SBARUPD_Battery)) {
        // Clear the space used by the USB / LOWBAT glyph
        lcd_fill_rect(X_BATTERY, 0, 11, 20, LCD_SET_VALUE);
        return;
      }
      if(getSystemFlag(FLAG_USB)) {
        showGlyph(STD_USB_SYMBOL, &standardFont, X_BATTERY, 0, vmNormal, true, false, false); // is 0+9+2 pixel wide
      }
      else {
        if(SBARUPD_BatVoltage) {
          drawBattery(min(get_vbat(), vbatVIntegrated));
        }
        else if(getSystemFlag(FLAG_LOWBAT)) {
          showGlyph(STD_BATTERY, &standardFont, X_BATTERY, 0, vmNormal, true, false, false); // is 0+10+1 pixel wide
        }
        else {
          // Clear the space used by the USB / LOWBAT glyph
          lcd_fill_rect(X_BATTERY, 0, 11, 20, LCD_SET_VALUE);
        }
      }
    }
  #endif // DMCP_BUILD


  void refreshStatusBar(void) {
                                #if defined(ANALYSE_REFRESH)
                                  print_caller(NULL);
                                #endif // ANALYSE_REFRESH
    if(screenUpdatingMode & SCRUPD_MANUAL_STATUSBAR) {      // force statusbar display for these modes
      switch(calcMode) {
        case CM_PEM:
        case CM_REGISTER_BROWSER:
        case CM_FLAG_BROWSER:
        case CM_ASN_BROWSER:
        case CM_FONT_BROWSER:
        case CM_PLOT_STAT:
        case CM_CONFIRMATION:
        case CM_MIM:
        case CM_TIMER:
        case CM_GRAPH: {
          screenUpdatingMode &= ~SCRUPD_MANUAL_STATUSBAR;
          break;
        }

        default: {
          return;
        }
      }
    }

    #if (DEBUG_INSTEAD_STATUS_BAR == 1)
      char statusMessage[100];
      char catalogstr[10];
      sprintf(catalogstr,"%d",catalog);
      sprintf(statusMessage, "%s%s %s %s m:%s i:%d ti:%u er:%u lp:%u %u ", 
        /*    */ catalog ? "asm:" : "", 
        /*    */ catalog ? catalogstr : "", 
        /*    */ tam.mode ? "tam" : "", 
        /*    */ getCalcModeName(calcMode),
        /* m  */ indexOfItems[-softmenu[softmenuStack[0].softmenuId].menuItem].itemCatalogName, 
        /* i  */ softmenuStack[0].firstItem, 
        /* ti  */ temporaryInformation, 
        /* er */ lastErrorCode, 
        /* lp */ lastParam,
        /*    */ programRunStop
        );
      showString(statusMessage, &standardFont, X_DATE, 0, vmNormal, true, true);
    #else // DEBUG_INSTEAD_STATUS_BAR != 1


      if(GRAPHMODE) {
        lcd_fill_rect(0, 0, 158, 20, LCD_SET_VALUE);
      }
      showDateTime();
      if(Y_SHIFT==0 && X_SHIFT<200) {
        showShiftState();
      }
      if(GRAPHMODE) {                      // With graph displayed, only update the time, as the other items are clashing with the graph display screen
        showHideHourGlass();
        return;
      }
      showRealComplexResult();
      showComplexMode();
      showAngularMode();
      showFracMode();

      if(!showMatrixMode()) {
        if(!showTvmMode()) {
          bool_t aa = showIntegerMode();    //note, separately to prevent compiler short circuiting second term
          bool_t bb = showOverflowCarry();
          if(!aa && !bb && (!(SBARUPD_IntegerMode) || !(SBARUPD_OCCarryMode))) {
            clearINT_MX_TVM_MODE();
          }
        }
      }

      showHideAlphaMode();
      showHideHourGlass();
      showStackSize();
      showHideWatch();
      if(fnTimerGetStatus(TO_ASM_ACTIVE) == TMR_RUNNING && (plainTextMode || labelText || catalog) ) {
        light_ASB_icon();
      }
      else {
        kill_ASB_icon();
      }
      showHideSerialIO();
      showHidePrinter();
      if(Y_SHIFT==0 && X_SHIFT >300) {
        showShiftState();
      }
      showHideUserMode();
      #if defined(DMCP_BUILD)
        showHideUsbLowBattery();
      #else // !DMCP_BUILD
        showHideStackLift();
      #endif // DMCP_BUILD
    #endif // DEBUG_INSTEAD_STATUS_BAR == 1
  }



  #if !defined(DMCP_BUILD)
    void showHideStackLift(void) {

      #if defined(BATTERYTEST)
        drawBattery(exponentLimit); //test battery indicator
        return;                     //test battery indicator
      #endif

      if(getSystemFlag(FLAG_ASLIFT)) {
        // Draw S
        setBlackPixel(392,  1);
        setBlackPixel(393,  1);
        setBlackPixel(394,  1);
        setBlackPixel(391,  2);
        setBlackPixel(395,  2);
        setBlackPixel(391,  3);
        setBlackPixel(392,  4);
        setBlackPixel(393,  4);
        setBlackPixel(394,  4);
        setBlackPixel(395,  5);
        setBlackPixel(391,  6);
        setBlackPixel(395,  6);
        setBlackPixel(392,  7);
        setBlackPixel(393,  7);
        setBlackPixel(394,  7);

        // Draw L
        setBlackPixel(391, 10);
        setBlackPixel(391, 11);
        setBlackPixel(391, 12);
        setBlackPixel(391, 13);
        setBlackPixel(391, 14);
        setBlackPixel(391, 15);
        setBlackPixel(391, 16);
        setBlackPixel(392, 16);
        setBlackPixel(393, 16);
        setBlackPixel(394, 16);
        setBlackPixel(395, 16);
      }
      else {
        // Erase S
        setWhitePixel(392,  1);
        setWhitePixel(393,  1);
        setWhitePixel(394,  1);
        setWhitePixel(391,  2);
        setWhitePixel(395,  2);
        setWhitePixel(391,  3);
        setWhitePixel(392,  4);
        setWhitePixel(393,  4);
        setWhitePixel(394,  4);
        setWhitePixel(395,  5);
        setWhitePixel(391,  6);
        setWhitePixel(395,  6);
        setWhitePixel(392,  7);
        setWhitePixel(393,  7);
        setWhitePixel(394,  7);

        // Erase L
        setWhitePixel(391, 10);
        setWhitePixel(391, 11);
        setWhitePixel(391, 12);
        setWhitePixel(391, 13);
        setWhitePixel(391, 14);
        setWhitePixel(391, 15);
        setWhitePixel(391, 16);
        setWhitePixel(392, 16);
        setWhitePixel(393, 16);
        setWhitePixel(394, 16);
        setWhitePixel(395, 16);
      }
    }
  #endif // !DMCP_BUILD


#ifdef PC_BUILD
  void mockupSB(void) {
    uint32_t x = 0;
    uint32_t xx = 0;
    char statusMessage[100];
    #define L0 0
    #define L1 20
    #define L2 40
    #define L3 60
    #define L4 80
    #define L5 100
    #define L6 120
    #define L7 140
    #define L8 160
    #define L9 200
    #define L10 220

    //All status bar printing functions copied frm the individual sections to creat a statusbar mockup

      clearScreen(100);

      getTimeString(oldTime);
      getDateString(dateTimeString);
      x = showString(dateTimeString, &standardFont, x, L1, vmNormal, true, true);
      xx = x;
      x = showGlyph(getSystemFlag(FLAG_TDM24) ? " " : STD_SPACE_3_PER_EM, &standardFont, x, L2, vmNormal, true, true, false); // is 0+0+8 pixel wide
      x = showString(oldTime, &standardFont, x, L2, vmNormal, true, false);

      x = showGlyph(STD_SPACE_3_PER_EM, &standardFont, xx, L3, vmNormal, true, true, false);
      getWeekOfYearString(dateTimeString);
      x = showString(dateTimeString, &standardFont, x, L3, vmNormal, true, false);

      showGlyph(STD_MODE_F, &standardFont, X_SHIFT_L, L0, vmNormal, true, true, false);   // f is pixel 4+8+3 wide
      showGlyph(STD_MODE_F, &standardFont, X_SHIFT_L, L2, vmNormal, true, true, false);   // f is pixel 4+8+3 wide
      showGlyph(STD_MODE_G, &standardFont, X_SHIFT_R, L0, vmNormal, true, true, false);   // g is pixel 4+10+1 wide

      x = X_REAL_COMPLEX;
      x = showGlyph(STD_COMPLEX_C, &standardFont, x, 0, vmNormal, true, false, false); // Complex C is 0+8+3 pixel wide
      x = X_REAL_COMPLEX;
      x = showGlyph(STD_REAL_R,    &standardFont, x, L1, vmNormal, true, false, false); // Complex C is 0+8+3 pixel wide

      x =  X_COMPLEX_MODE;
      showGlyph(STD_SUN, &standardFont, x, 0, vmNormal, true, true, false); // Sun         is 0+12+2 pixel wide
      showGlyph(STD_RIGHT_ANGLE, &standardFont, x, L1, vmNormal, true, true, false); // Sun         is 0+12+2 pixel wide
      x =  X_COMPLEX_MODE + X_COMPLEX_MODE_ADJ;
      showGlyph(STD_SUN, &standardFont, x, L3, vmNormal, true, true, false); // Sun         is 0+12+2 pixel wide
      showGlyph(STD_RIGHT_ANGLE, &standardFont, x, L4, vmNormal, true, true, false); // Sun         is 0+12+2 pixel wide

      x = X_ANGULAR_MODE;
      x = showGlyph(STD_MEASURED_ANGLE, &standardFont, x, L0, vmNormal, true, true, false); // Angle is 0+9 pixel wide
      showGlyph(STD_SUP_pir,            &standardFont, x, L0, vmNormal, true, false, false); // pi is 0+9 pixel wide
      showGlyph(STD_SUP_BOLD_r,         &standardFont, x, L1, vmNormal, true, false, false); // r  is 0+6 pixel wide
      showGlyph(STD_SUP_BOLD_g,         &standardFont, x, L2, vmNormal, true, false, false); // g  is 0+6 pixel wide
      showGlyph(STD_DEGREE,             &standardFont, x, L3, vmNormal, true, false, false); // °  is 0+6 pixel wide
      showGlyph(STD_RIGHT_DOUBLE_QUOTE, &standardFont, x, L4, vmNormal, true, false, false); // "  is 0+6 pixel wide
      showGlyph(STD_SUP_pir,            &standardFont, x, L5, vmNormal, true, false, false); // pi is 0+9 pixel wide

      x = X_FRAC_MODE;
      x = showString("#KEY", &standardFont, x, L5 , vmNormal, true, true);
      x = showString(STD_SUB_A STD_SUB_MINUS STD_SUB_F,  &standardFont, x, L5-4 , vmNormal, true, true);
      x = X_FRAC_MODE;
      x = showString("#BASE", &standardFont, x, L4, vmNormal, true, true);

      x = X_FRAC_MODE;                    //vJM
      raiseString = 3;
      x = showString("a" STD_SPACE_4_PER_EM, &standardFont, x, 0, vmNormal, true, true) - 3;
      raiseString = 9;
      x = showString(STD_SUB_b, &standardFont, x, 0, vmNormal, true, true) - 2-2;

    char divStr[10];
    compressString = 1;
    xx = x;
      x = showGlyph("/", &standardFont, x, lowerUnderLine-1, vmNormal, false, true, true);
      x = showGlyph("/", &standardFont, xx+4, lowerUnderLine-1-9, vmNormal, false, true, true)-5;
    lcd_fill_rect(xx, 0, x-xx, lowerUnderLine-1, LCD_SET_VALUE);

    compressString = 1;
    sprintf(statusMessage,"max");
    xx = x;
    x = showString(statusMessage, &standardFont, x, lowerUnderLine, vmNormal, false, true);
    raiseString = 3;
    x = showString(STD_SUB_f, &standardFont, ++x, lowerUnderLine, vmNormal, true, true);
 
    compressString = 1;
    sprintf(statusMessage, "%" PRIu32,9999);
    x = showString(statusMessage, &standardFont, xx, L1+lowerUnderLine, vmNormal, false, true);
    compressString = 1;
    x = showString(statusMessage, &standardFont, xx, L2+lowerUnderLine, vmNormal, false, true);

    strcpy(divStr,STD_CROSS);
    raiseString = 2;
    xx = x;
    x = showString(divStr, &standardFont, ++x, L1+lowerUnderLine, vmNormal, true, true) + (0);
    strcpy(divStr,STD_IRRATIONAL_I);
    raiseString = 1;
    x = max(0,((int32_t)x)-1);
    x = showString(divStr, &standardFont, x, L1, vmNormal, false, false) + 2;

    strcpy(divStr,STD_DOT);
    raiseString = 2;
    x = showString(divStr, &standardFont, ++xx,   L2+lowerUnderLine, vmNormal, true, true) + (2);
    strcpy(divStr,STD_IRRATIONAL_I);
    raiseString = 1;
    x = max(0,((int32_t)x)-1);
    x = showString(divStr, &standardFont, x  , L2, vmNormal, false, false) + 2;

    compressString = 1;
    x = showString(STD_ALMOST_EQUAL, &standardFont, ++x - 1, 0, vmNormal, true, false);


    sprintf(statusMessage, "%s%" PRIu8 ":%c", shortIntegerWordSize <= 9 ? " " : "", shortIntegerWordSize, shortIntegerMode==SIM_1COMPL?'1':(shortIntegerMode==SIM_2COMPL?'2':(shortIntegerMode==SIM_UNSIGN?'u':(shortIntegerMode==SIM_SIGNMT?'s':'?'))));
    x = showString(statusMessage, &standardFont, X_INT_MX_TVM_MODE, 0, vmNormal, true, true);

    x = X_INT_MX_TVM_MODE;
    compressString = 1;
    x = showString("grow", &standardFont, x, L1, vmNormal, true, true);
    x = X_INT_MX_TVM_MODE;
    compressString = 1;
    x = showString("wrap", &standardFont, x, L2, vmNormal, true, true);
    compressString = 1;
    showString("END"    , &standardFont, X_INT_MX_TVM_MODE,  L3, vmNormal, true, true);   //normal
    compressString = 1;
    showString("BEG"    , &standardFont, X_INT_MX_TVM_MODE,  L4, vmNormal, true, true);   //normal

    showGlyph(STD_OVERFLOW_CARRY, &standardFont, X_OVERFLOW_CARRY, 0, vmNormal, true, false, false); // STD_OVERFLOW_CARRY is 0+6+3 pixel wide

    showString(STD_A    , &standardFont, X_ALPHA_MODE,  L0, vmNormal, true, false);   //normal
    showString(STD_SUB_A, &standardFont, X_ALPHA_MODE,  L1, vmNormal, true, false);   //normal
    showString(STD_SUP_A, &standardFont, X_ALPHA_MODE,  L2, vmNormal, true, false);   //normal
    showString(STD_a,     &standardFont, X_ALPHA_MODE,  L3, vmNormal, true, false);   //normal
    showString(STD_SUB_a, &standardFont, X_ALPHA_MODE,  L4, vmNormal, true, false);   //normal
    showString(STD_SUP_a, &standardFont, X_ALPHA_MODE,  L5, vmNormal, true, false);   //normal
    showString(STD_num STD_SUB_N STD_SUP_N,   &standardFont, X_ALPHA_MODE,  L6, vmNormal, true, false);   //normal
    showString(STD_n STD_SUB_n STD_SUP_n,     &standardFont, X_ALPHA_MODE,  L7, vmNormal, true, false);   //normal
    showString(STD_BOX,   &standardFont, X_ALPHA_MODE,  L8, vmNormal, true, false);   //normal

    showGlyph(STD_HOURGLASS_WH,            &standardFont, X_HOURGLASS  +1   ,     L1, vmNormal, true, false, false); // is 0+11+3 pixel wide //Shift the hourglass to a visible part of the status bar
    showGlyph(STD_NEG_EXCLAMATION_MARK, &standardFont, X_HOURGLASS       ,     L3, vmNormal, true, false, false);
    showGlyph(STD_P,                    &standardFont, X_HOURGLASS  +2   ,     L2, vmNormal, true, false, false);
    showGlyph(STD_HOURGLASS_WH,            &standardFont, X_HOURGLASS_GRAPHS  , L2, vmNormal, true, false, false); // is 0+11+3 pixel wide //Shift the hourglass to a visible part of the status bar

    strcpy(asmBuffer,"MM");
    compressString = 1;             //^JM
    showString(asmBuffer, &standardFont, X_ASM, L1, vmNormal, true, false);
    asmBuffer[0]=0;

    x = X_SSIZE_BEGIN;
    x = showGlyph(STD_SPACE_6_PER_EM, &standardFont, x, 0, vmNormal, true, true, false); // is 0+6+2 pixel wide
    x = showGlyph(STD_4, &standardFont, x, 0, vmNormal, true, true, false); // is 0+6+2 pixel wide
    x = X_SSIZE_BEGIN;
    char dd[6];
    sprintf(dd,STD_SPACE_6_PER_EM STD_8);
    showString(dd, &standardFont, x, L1, vmReverse, false, true);
    showString(dd, &standardFont, x, L2, vmReverse, false, true);
    showString(dd, &standardFont, x, L3, vmReverse, false, true);

      showGlyph(STD_TIMER,     &standardFont, X_STOPWATCH, L2, vmNormal, true, false, false); // is 0+13+1 pixel wide
      showGlyph(STD_SERIAL_IO, &standardFont, X_SERIAL_IO, L1, vmNormal, true, false, false); // is 0+8+3 pixel wide
      showGlyph(STD_PRINTER,   &standardFont, X_PRINTER,   L2, vmNormal, true, false, false); // is 0+12+3 pixel wide
      showGlyph(STD_USER_MODE, &standardFont, X_USER_MODE, L0, vmNormal, false, false, false); // STD_USER_MODE is 0+12+2 pixel wide
      showGlyph(STD_USER_MODE, &standardFont, X_USER_MODE, L1, vmNormal, false, false, false); // STD_USER_MODE is 0+12+2 pixel wide
      showGlyph(STD_USER_MODE, &standardFont, X_USER_MODE, L2, vmNormal, false, false, false); // STD_USER_MODE is 0+12+2 pixel wide

      light_ASB_icon();
      drawBattery(exponentLimit); //test battery indicator
      showGlyph(STD_BATTERY, &standardFont, X_BATTERY, L1, vmNormal, true, false, false); // is 0+10+1 pixel wide
      calcMode = CM_GRAPH;
  }
#endif //PC_BUILD

#endif // !TESTSUITE_BUILD
