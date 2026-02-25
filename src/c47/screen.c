// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"
#include "version.h"

#if defined(PC_BUILD) && defined(ANALYSE_REFRESH)
  #include <execinfo.h>
#endif //PC_BUILD

#if !defined(TESTSUITE_BUILD)
  static void refreshRegisterLineRestoreT(void);
  static void _refreshPemScreen(void);
#endif //TESTSUITE_BUILD


//#define DEBUGCLEARS


//undefine FIXED_FN_NAME_SHIFT to let the function name move to the left edge with the shift
//   The default is to keep the left offset as it looks prettier, arguably
  #define FIXED_FN_NAME_SHIFT

  #define shiftOffset        17
  #define noShiftOffset      0
  #if defined(FIXED_FN_NAME_SHIFT)
    #define funcNameOffset_x shiftOffset
  #else
    #define funcNameOffset_x (Y_SHIFT ? shiftOffset : noShiftOffset)
  #endif
  #define isShiftOffset      funcNameOffset_x == shiftOffset
  #define funcNameOffset_str isShiftOffset ? "  " : ""


void setLastintegerBasetoZero(void) {
  if(lastIntegerBase != 0) {
    lastIntegerBase = 0;
    screenUpdatingMode = SCRUPD_AUTO;
    screenUpdatingMode |= SCRUPD_SKIP_STATUSBAR_ONE_TIME;
    refreshNIMdone = false;
    refreshScreen(56);
  }
  fnRefreshState();                                                //JMNIM
}

bool_t blockMonitoring = false;


#if !defined(TESTSUITE_BUILD)
  #define spc STD_SPACE
  #define spc1 STD_SPACE STD_SPACE_3_PER_EM

  #if (CALCMODEL == USER_R47)
    #define MODELTEXT "R47"
  #else
    #define MODELTEXT "C47"
  #endif

  TO_QSPI static const char whoStr1[] = "C47 & R47 Development since 2019" spc "by" spc1
                                       "\n"
                                       "Ben" spc "GB," spc1
                                       "D" spc "A" spc "CA," spc1
                                       "Dani" spc "CH," spc1
                                       "Didier" spc "FR," spc1
                                       "\n"
                                       "H" STD_a_RING "kon" spc "NO," spc1
                                       "Jaco" spc "ZA," spc1
                                       "Martin" spc "FR," spc1
                                       "Mihail" spc "JP," spc1
                                       "\n"
                                       "Pauli" spc "AU," spc1
                                       "RJvM" spc "NL," spc1
                                       "Walter" spc "DE.";



   TO_QSPI static const char disclaimerStr[]     = "  " MODELTEXT " firmware is free, open source and \n  neither provided nor supported by \n  SwissMicros. Press a key to continue.";

   TO_QSPI static const char versionStr[]        = "  " MODELTEXT " " VERSION_STRING ".";
  #if defined(PC_BUILD)

    TO_QSPI static const char versionStr2[]     = "  " MODELTEXT " Sim " VERSION1 ", dated " __DATE__ ".";
  #else // !PC_BUILD
    #if defined(TWO_FILE_PGM)
      TO_QSPI static const char versionStr2[]   = "  " MODELTEXT " QSPI " VERSION1 ", dated " __DATE__ ".";
    #else // !TWO_FILE_PGM
      #if !defined(TWO_FILE_PGM)
        TO_QSPI static const char versionStr2[] = "  " MODELTEXT " No QSPI " VERSION1 ", dated " __DATE__ ".";
      #endif // !TWO_FILE_PGM
    #endif // TWO_FILE_PGM
  #endif // PC_BUILD

  /* Names of day of week */
typedef struct {
  char     itemName[30];
} nstr;

  TO_QSPI static const nstr nameOfWday_en[8] = { {"invalid day of week"},                                   {"Monday"},            {"Tuesday"},                     {"Wednesday"},               {"Thursday"},           {"Friday"},             {"Saturday"},             {"Sunday"}};
  /*
  TO_QSPI static const char *nameOfWday_de[8] = {"ung" STD_u_DIARESIS "ltiger Wochentag",                 "Montag",            "Dienstag",                    "Mittwoch",                "Donnerstag",         "Freitag",            "Samstag",              "Sonntag"};
  TO_QSPI static const char *nameOfWday_fr[8] = {"jour de la semaine invalide",                           "lundi",             "mardi",                       "mercredi",                "jeudi",              "vendredi",           "samedi",               "dimanche"};
  TO_QSPI static const char *nameOfWday_es[8] = {"d" STD_i_ACUTE "a inv" STD_a_ACUTE "lido de la semana", "lunes",             "martes",                      "mi" STD_e_ACUTE "rcoles", "jueves",             "viernes",            "s" STD_a_ACUTE "bado", "domingo"};
  TO_QSPI static const char *nameOfWday_it[8] = {"giorno della settimana non valido",                     "luned" STD_i_GRAVE, "marted" STD_i_GRAVE,          "mercoled" STD_i_GRAVE,    "gioved" STD_i_GRAVE, "venerd" STD_i_GRAVE, "sabato",               "domenica"};
  TO_QSPI static const char *nameOfWday_pt[8] = {"dia inv" STD_a_ACUTE "lido da semana",                  "segunda-feira",     "ter" STD_c_CEDILLA "a-feira", "quarta-feira",            "quinta-feira",       "sexta-feira",        "s" STD_a_ACUTE "bado", "domingo"};
  */
#endif // !TESTSUITE_BUILD

#if defined(PC_BUILD)
  gboolean drawScreen(GtkWidget *widget, cairo_t *cr, gpointer data) {
    cairo_surface_t *imageSurface;

    imageSurface = cairo_image_surface_create_for_data((unsigned char *)screenData, CAIRO_FORMAT_RGB24, SCREEN_WIDTH, SCREEN_HEIGHT, screenStride * 4);
    #if (BIG_SCREEN_COEF != 1)
      cairo_scale(cr, BIG_SCREEN_COEF, BIG_SCREEN_COEF);
    #endif // BIG_SCREEN_COEF != 1
    cairo_set_source_surface(cr, imageSurface, 0, 0);
    cairo_surface_mark_dirty(imageSurface);
    #if (BIG_SCREEN_COEF != 1)
      cairo_pattern_set_filter(cairo_get_source(cr), CAIRO_FILTER_FAST);
    #endif // BIG_SCREEN_COEF != 1
    cairo_paint(cr);
    cairo_surface_destroy(imageSurface);

    return FALSE;
  }


  void copyScreenToClipboard(void) {
    cairo_surface_t *imageSurface;
    GtkClipboard *clipboard;

    clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_clear(clipboard);
    gtk_clipboard_set_text(clipboard, "", 0); //JM FOUND TIP TO PROPERLY CLEAR CLIPBOARD: https://stackoverflow.com/questions/2418487/clear-the-system-clipboard-using-the-gtk-lib-in-c/2419673#2419673

    imageSurface = cairo_image_surface_create_for_data((unsigned char *)screenData, CAIRO_FORMAT_RGB24, SCREEN_WIDTH, SCREEN_HEIGHT, screenStride * 4);
    gtk_clipboard_set_image(clipboard, gdk_pixbuf_get_from_surface(imageSurface, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
  }


  #define CLIPSTR 30000                                         //JMCSV
#else // !PC_BUILD
  #define CLIPSTR TMP_STR_LENGTH                              //JMCSV
#endif // PC_BUILD                                            //JMCSV

#if defined(PC_BUILD) || defined(DMCP_BUILD)        //JMCSV
  static void angularUnitToString(angularMode_t angularMode, char *string) {
    switch(angularMode) {
      case amRadian: strcpy(string, "r");        break;
      case amMultPi: strcpy(string, STD_pi);     break;
      case amGrad:   strcpy(string, "g");        break;
      case amDegree: strcpy(string, STD_DEGREE); break;
      case amDMS:    strcpy(string, "d.ms");     break;
      case amNone:                               break;
      default:       strcpy(string, "?");
    }
  }

  void copyRegisterToClipboardString(calcRegister_t regist, char *clipboardString) {
    longInteger_t lgInt;
    int16_t base, sign, n;
    uint64_t shortInt;
    char string[CLIPSTR];

    switch(getRegisterDataType(regist)) {
      case dtLongInteger:
        convertLongIntegerRegisterToLongInteger(regist, lgInt);
        longIntegerToAllocatedString(lgInt, string, CLIPSTR);
        longIntegerFree(lgInt);
        break;

      case dtTime:
        timeToDisplayString(regist, string, false);
        break;

      case dtDate:
        dateToDisplayString(regist, string);
        break;

      case dtString:
        xcopy(string, REGISTER_STRING_DATA(regist), stringByteLength(REGISTER_STRING_DATA(regist)) + 1);
        break;

      case dtReal34Matrix: {
        matrixHeader_t *matrixHeader = REGISTER_MATRIX_HEADER(regist);
        real34_t *real34 = REGISTER_REAL34_MATRIX_ELEMENTS(regist);
        real34_t reduced;
        uint32_t rows, columns, len;

        rows = matrixHeader->matrixRows;
        columns = matrixHeader->matrixColumns;
        sprintf(string, "%" PRIu32 "x%" PRIu32, rows, columns);

        for(uint32_t i=0; i<rows*columns; i++) {
          strcat(string, LINEBREAK);
          len = strlen(string);

          real34Reduce(real34++, &reduced);
          real34ToString(&reduced, string + len);

          if(strchr(string + len, '.') == NULL && strchr(string + len, 'E') == NULL) {
            strcat(string + len, ".");
          }
        }
        break;
      }

      case dtComplex34Matrix: {
        matrixHeader_t *matrixHeader = REGISTER_MATRIX_HEADER(regist);
        complex34_t *complex34 = REGISTER_COMPLEX34_MATRIX_ELEMENTS(regist);
        real34_t reduced;
        uint32_t rows, columns, len;

        rows = matrixHeader->matrixRows;
        columns = matrixHeader->matrixColumns;
        sprintf(string, "%" PRIu32 "x%" PRIu32, rows, columns);

        for(uint32_t i=0; i<rows*columns; i++, complex34++) {
          strcat(string, LINEBREAK);
          len = strlen(string);

          // Real part
          real34Reduce((real34_t *)complex34, &reduced);
          real34ToString(&reduced, string + len);
          if(strchr(string + len, '.') == NULL && strchr(string + len, 'E') == NULL) {
            strcat(string + len, ".");
          }
          len = strlen(string);

          // Imaginary part
          real34Reduce(((real34_t *)complex34) + 1, &reduced);
          if(real34IsNegative(&reduced)) {
            sprintf(string + len, " - %sx", COMPLEX_UNIT);
            len += 5;
            real34SetPositiveSign(&reduced);
            real34ToString(&reduced, string + len);
          }
          else {
            sprintf(string + len + strlen(string + len), " + %sx", COMPLEX_UNIT);
            len += 5;
            real34ToString(&reduced, string + len);
          }
          if(strchr(string + len, '.') == NULL && strchr(string + len, 'E') == NULL) {
            strcat(string + len, ".");
          }
        }
        break;
      }

      case dtShortInteger:
        convertShortIntegerRegisterToUInt64(regist, &sign, &shortInt);
        base = getRegisterShortIntegerBase(regist);

        n = ERROR_MESSAGE_LENGTH - 100;
        sprintf(errorMessage + n--, "#%d (word size = %u)", base, shortIntegerWordSize);

        if(shortInt == 0) {
          errorMessage[n--] = '0';
        }
        else {
          while(shortInt != 0) {
            errorMessage[n--] = baseDigits[shortInt % base];
            shortInt /= base;
          }
          if(sign) {
            errorMessage[n--] = '-';
          }
        }
        n++;

        strcpy(string, errorMessage + n);
        break;

      case dtReal34: {
        real34_t reduced;

        real34Reduce(REGISTER_REAL34_DATA(regist), &reduced);
        real34ToString(&reduced, string);
        if(strchr(string, '.') == NULL && strchr(string, 'E') == NULL) {
          strcat(string, ".");
        }
        angularUnitToString(getRegisterAngularMode(regist), string + strlen(string));
        break;
      }

      case dtComplex34: {
        real34_t reduced;
        int len;
        char tmpStr[100];

        // Real part
        real34Reduce(REGISTER_REAL34_DATA(regist), &reduced);
        real34ToString(&reduced, tmpStr);
        if(strchr(tmpStr, '.') == NULL && strchr(tmpStr, 'E') == NULL) {
          strcat(tmpStr, ".");
        }
        len = strlen(tmpStr);

        // Imaginary part
        real34Reduce(REGISTER_IMAG34_DATA(regist), &reduced);
        if(real34IsNegative(&reduced)) {
          sprintf(string, "%s - %sx", tmpStr, COMPLEX_UNIT);
          len += 5;
          real34SetPositiveSign(&reduced);
          real34ToString(&reduced, string + len);
        }
        else {
          sprintf(string, "%s + %sx", tmpStr, COMPLEX_UNIT);
          len += 5;
          real34ToString(&reduced, string + len);
        }
        if(strchr(string + len, '.') == NULL && strchr(string + len, 'E') == NULL) {
          strcat(string + len, ".");
        }
        break;
      }

      case dtConfig:
        xcopy(string, "Configuration data", 19);
        break;

      default:
        sprintf(string, "In function copyRegisterXToClipboard, the data type %" PRIu32 " is unknown! Please try to reproduce and submit a bug.", getRegisterDataType(regist));
    }

    stringToUtf8(string, (uint8_t *)clipboardString);
  }
#endif // PC_BUILD || DMCP_BUILD                              //JMCSV

#define checkHPoffset (checkHP && temporaryInformation == TI_NO_INFO ? 50:0)

char letteredRegisterName(calcRegister_t regist) {
  return registerFlagLetters[regist - FIRST_LETTERED_REGISTER];
}


#if defined(PC_BUILD)                                         //JMCSV
  void copyRegisterXToClipboard(void) {
    GtkClipboard *clipboard;
    char clipboardString[CLIPSTR];

    clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_clear(clipboard);
    gtk_clipboard_set_text(clipboard, "", 0); //JM FOUND TIP TO PROPERLY CLEAR CLIPBOARD: https://stackoverflow.com/questions/2418487/clear-the-system-clipboard-using-the-gtk-lib-in-c/2419673#2419673

    copyRegisterToClipboardString(REGISTER_X, clipboardString);
    gtk_clipboard_set_text(clipboard, clipboardString, -1);
  }


  void copyStackRegistersToClipboardString(char *clipboardString, calcRegister_t lastRegist) {
    char *ptr = clipboardString;
    const char *sep = "";

    for (calcRegister_t r = lastRegist; r >= REGISTER_X; r--) {
      ptr += sprintf(ptr, "%s%c = ", sep, letteredRegisterName(r));
      copyRegisterToClipboardString(r, ptr);
      ptr = strchr(ptr, '\0');
      sep = LINEBREAK;
    }
  }


  void copyStackRegistersToClipboard(void) {
    GtkClipboard *clipboard;
    char clipboardString[10000];

    clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_clear(clipboard);
    gtk_clipboard_set_text(clipboard, "", 0); //JM FOUND TIP TO PROPERLY CLEAR CLIPBOARD: https://stackoverflow.com/questions/2418487/clear-the-system-clipboard-using-the-gtk-lib-in-c/2419673#2419673

    copyStackRegistersToClipboardString(clipboardString, REGISTER_K);

    gtk_clipboard_set_text(clipboard, clipboardString, -1);
  }


  void copyAllRegistersToClipboard(void) {
    GtkClipboard *clipboard;
    char clipboardString[15000], *ptr = clipboardString;

    clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_clear(clipboard);
    gtk_clipboard_set_text(clipboard, "", 0); //JM FOUND TIP TO PROPERLY CLEAR CLIPBOARD: https://stackoverflow.com/questions/2418487/clear-the-system-clipboard-using-the-gtk-lib-in-c/2419673#2419673

    copyStackRegistersToClipboardString(ptr, LAST_SPARE_REGISTER);

    for(int32_t regist=99; regist>=0; --regist) {
      ptr += strlen(ptr);
      sprintf(ptr, LINEBREAK "R%02d = ", regist);
      ptr += strlen(ptr);
      copyRegisterToClipboardString(regist, ptr);
    }

    for(int32_t regist=currentNumberOfLocalRegisters-1; regist>=0; --regist) {
      ptr += strlen(ptr);
      sprintf(ptr, LINEBREAK "R.%02d = ", regist);
      ptr += strlen(ptr);
      copyRegisterToClipboardString(FIRST_LOCAL_REGISTER + regist, ptr);
    }

    if(statisticalSumsPointer != NULL) {
      TO_QSPI const char * const StatSumNames[NUMBER_OF_STATISTICAL_SUMS] = {
        /* 0*/ "n             ",
        /* 1*/ STD_SIGMA "(x)          ",
        /* 2*/ STD_SIGMA "(y)          ",
        /* 3*/ STD_SIGMA "(x" STD_SUP_2 ")         ",
        /* 4*/ STD_SIGMA "(x" STD_SUP_2 "y)        ",
        /* 5*/ STD_SIGMA "(y" STD_SUP_2 ")         ",
        /* 6*/ STD_SIGMA "(xy)         ",
        /* 7*/ STD_SIGMA "(ln(x)" STD_CROSS "ln(y))",
        /* 8*/ STD_SIGMA "(ln(x))      ",
        /* 9*/ STD_SIGMA "(ln" STD_SUP_2 "(x))     ",
        /*10*/ STD_SIGMA "(y ln(x))    ",
        /*11*/ STD_SIGMA "(ln(y))      ",
        /*12*/ STD_SIGMA "(ln" STD_SUP_2 "(y))     ",
        /*13*/ STD_SIGMA "(x ln(y))    ",
        /*14*/ STD_SIGMA "(ln(y)/x)    ",
        /*15*/ STD_SIGMA "(x" STD_SUP_2 "/y)       ",
        /*16*/ STD_SIGMA "(1/x)        ",
        /*17*/ STD_SIGMA "(1/x" STD_SUP_2 ")       ",
        /*18*/ STD_SIGMA "(x/y)        ",
        /*19*/ STD_SIGMA "(1/y)        ",
        /*20*/ STD_SIGMA "(1/y" STD_SUP_2 ")       ",
        /*21*/ STD_SIGMA "(x" STD_SUP_3 ")         ",
        /*22*/ STD_SIGMA "(x" STD_SUP_4 ")         ",
        /*23*/ "x min         ",
        /*24*/ "x max         ",
        /*25*/ "y min         ",
        /*26*/ "y max         "
      };

      char sumName[40];
      for(int32_t sum=0; sum<NUMBER_OF_STATISTICAL_SUMS; sum++) {
        ptr += strlen(ptr);
        strcpy(sumName, StatSumNames[sum]);

        sprintf(ptr, LINEBREAK "SR%02d = ", sum);
        ptr += strlen(ptr);
        stringToUtf8(sumName, (uint8_t *)ptr);
        ptr += strlen(ptr);
        strcpy(ptr, " = ");
        ptr += strlen(ptr);
        realToString(statisticalSumsPointer + sum, tmpString);
        if(strchr(tmpString, '.') == NULL && strchr(tmpString, 'E') == NULL) {
          strcat(tmpString, ".");
        }
        strcpy(ptr, tmpString);
      }
    }

    gtk_clipboard_set_text(clipboard, clipboardString, -1);
  }


  #define cursorCycle 3                      //JM cursor vv
  int8_t cursorBlinkCounter;                 //JM cursor ^^
  gboolean refreshLcd(gpointer unusedData) { // This function is called every SCREEN_REFRESH_PERIOD ms by a GTK timer
    // Cursor blinking
    static bool_t cursorBlink=true;

    if(cursorEnabled) {
      if(++cursorBlinkCounter > cursorCycle) {         //JM cursor vv
        cursorBlinkCounter = 0;
        if(cursorBlink && !checkHP) {
          showGlyph(STD_CURSOR, cursorFont, xCursor, yCursor - checkHPoffset, vmNormal, true, false, false);
        }                                              //JM cursor ^^
        else {
          hideCursor();
        }
      cursorBlink = !cursorBlink;
      }
    }

    // Function name display
    if(showFunctionNameCounter > 0) {
      showFunctionNameCounter -= SCREEN_REFRESH_PERIOD;
      if(showFunctionNameCounter <= 0) {
        hideFunctionName();
        tmpString[0] = 0;
        showFunctionName(ITM_NOP, 0, "SF:R");
      }
    }

    // Update date and time
    showDateTime();

    lcd_refresh();
    refresh_gui();

    return TRUE;
  }
#elif defined(DMCP_BUILD)
  #define cursorCycle 3                      //JM cursor vv
  int8_t cursorBlinkCounter;                 //JM cursor ^^
  void refreshLcd(void) { // This function is called roughly every SCREEN_REFRESH_PERIOD ms from the main loop
    // Cursor blinking
    static bool_t cursorBlink=true;

    if(cursorEnabled) {
      if(++cursorBlinkCounter > cursorCycle) {         //JM cursor vv
        cursorBlinkCounter = 0;
        if(cursorBlink && !checkHP) {
          showGlyph(STD_CURSOR, cursorFont, xCursor, yCursor - checkHPoffset, vmNormal, true, false, false);
        }                                              //JM cursor ^^
        else {
          hideCursor();
        }
        //cursorBlink = !cursorBlink;
      }
    }

    // Function name display
    if(showFunctionNameCounter>0) {
      showFunctionNameCounter -= FAST_SCREEN_REFRESH_PERIOD;
      if(showFunctionNameCounter <= 0) {
        hideFunctionName();
        tmpString[0] = 0;
        showFunctionName(ITM_NOP, 0, "SF:R");
      }
    }

    // Update date and time
    if(showDateTime()) {
      dmcpResetAutoOff();
      fnPollTimerApp();
    }
    checkBattery();
  }
#endif // PC_BUILD DMCP_BUILD


void execTimerApp(uint16_t timerType) {
  fnTimerStart(TO_TIMER_APP, TO_TIMER_APP, TIMER_APP_PERIOD);
  fnUpdateTimerApp();
}


#if !defined(TESTSUITE_BUILD)
  void refreshFn(uint16_t timerType) {                        //vv dr - general timeout handler
    if(timerType == TO_FG_LONG) Shft_handler();
    if(timerType == TO_CL_LONG) LongpressKey_handler();
    if(timerType == TO_FG_TIMR) Shft_stop();
    if(timerType == TO_FN_LONG) FN_handler();
    if(timerType == TO_ASM_ACTIVE) {
      if(catalog) {
        resetAlphaSelectionBuffer();
      }
    }
  }                                                           //^^


  void toggle6UnderLines(int16_t y) {
      if((maxfgLines(y) || (getSystemFlag(FLAG_FGLNFUL)))) {
        underline_softkey(0b111111u, y);
      } else {
        underline_softkey(0, 3);
      }
  }


  void show_f_jm(void) {
    if(!FN_timeouts_in_progress && calcMode != CM_ASN_BROWSER) {
        toggle6UnderLines(1);
    }
  }


  void show_g_jm(void) {
    if(!FN_timeouts_in_progress && calcMode != CM_ASN_BROWSER) {
        toggle6UnderLines(2);
    }
  }


  void clear_fg_jm(void) {
    if(!FN_timeouts_in_progress) {        //Cancel lines
      underline_softkey(0, 3);
    }
  }

  static inline uint16_t getLine_buffer_bit(int x) {
    return 415-x;
  }

  uint16_t yUnderlined = 3;
  void underline_softkey(uint16_t xSoftkeyMask, uint16_t ySoftkey) {
    if(calcMode == CM_REGISTER_BROWSER || calcMode == CM_FLAG_BROWSER || calcMode == CM_FONT_BROWSER || (!getSystemFlag(FLAG_FGLNFUL) && !getSystemFlag(FLAG_FGLNLIM))  ) {
      return;
    }
    xSoftkeyMask &= (GRAPHMODE?0b000011u:0b111111u);
    bool_t greyType = getSystemFlag(FLAG_FGGR);
    uint8_t lineCount, maxLine;
    lineCount = greyType ? SOFTMENU_HEIGHT - 3 : 3;
    if (yUnderlined <= 2) {
      maxLine = 239 - SOFTMENU_HEIGHT * (yUnderlined);
      // reset display to the buffer without shade
      lcd_refresh_lines (maxLine-lineCount, lineCount);
    }
    yUnderlined = ySoftkey;
    if(ySoftkey > 2) {
      return;
    }
    uint8_t temp_line[LCD_LINE_BUF_SIZE], tempByte, xBg[6], xIndex, line;
    uint16_t j, buff_bit, colIncrease = greyType ? 5 : 2;
    maxLine = 238 - SOFTMENU_HEIGHT * (ySoftkey);
    // Get current background from corner pixels
    for (xIndex = 0;xIndex < 6; xIndex++) {
      buff_bit = getLine_buffer_bit(KEY_X[xIndex]+1);
      xBg[xIndex] = (lcd_buffer[52 * (maxLine+1) + buff_bit/8]>>mod(buff_bit,8)) & 1u;
    }
    // Draw shade pattern without changing lcd_buffer
    for (line = maxLine - lineCount + 1; line <= maxLine; line++) {
      memcpy(temp_line, &lcd_buffer[52 * line] , LCD_LINE_BUF_SIZE);
      for (xIndex = 0; xIndex < 6; xIndex++) {
        if (xSoftkeyMask>>xIndex & 1u) {
          j = KEY_X[xIndex] + 1;
          j += greyType ? mod(2*line-j,5) : mod(j+line,2);
          for (; j < KEY_X[xIndex + 1]; j += colIncrease) {
            buff_bit = getLine_buffer_bit(j);
            tempByte = temp_line[buff_bit / 8];
            if (xBg[xIndex]){
              tempByte = tempByte & ~(1u<<mod(buff_bit,8));
            } else {
              tempByte = tempByte | (1u<<mod(buff_bit,8));
            }
            temp_line[buff_bit/8] = tempByte;
          }
        }
      }
      temp_line[0] = 0;
      LCD_write_line (temp_line);
    }
  }


  void FN_handler_StepToF(uint32_t time) {
    shiftF = true;        //S_shF();                  //   New shift state
    shiftG = false;
    showShiftState();
    if(calcMode != CM_PEM) {
      refreshRegisterLineRestoreT(); //clearRegisterLine(Y_POSITION_OF_REGISTER_T_LINE - 4, REGISTER_LINE_HEIGHT); //JM FN clear the previous shift function name
    }
    char *varCatalogItem = "SF:F";
    int16_t Dyn = nameFunction(FN_key_pressed-37, shiftF, shiftG);
    if(dynamicMenuItem > -1 && !DEBUGSFN) {
      varCatalogItem = dynmenuGetLabel(dynamicMenuItem);
    }
    showFunctionName(Dyn,0, varCatalogItem);
    FN_timed_out_to_RELEASE_EXEC = true;
    underline_softkey(1<<(FN_key_pressed-38), 1);
    fnTimerStart(TO_FN_LONG, TO_FN_LONG, time);          //dr
  }


  void FN_handler_StepToG(uint32_t time) {
    shiftF = false;
    shiftG = true;
    showShiftState();
    if(calcMode != CM_PEM) {
      refreshRegisterLineRestoreT(); //clearRegisterLine(Y_POSITION_OF_REGISTER_T_LINE - 4, REGISTER_LINE_HEIGHT); //JM FN clear the previous shift function name
    }
    char *varCatalogItem = "SF:G";
    int16_t Dyn = nameFunction(FN_key_pressed-37, shiftF, shiftG);
    if(dynamicMenuItem > -1 && !DEBUGSFN) {
      varCatalogItem = dynmenuGetLabel(dynamicMenuItem);
    }
    showFunctionName(Dyn,0, varCatalogItem);
    FN_timed_out_to_RELEASE_EXEC = true;
    underline_softkey(1<<(FN_key_pressed-38), 2);
    fnTimerStart(TO_FN_LONG, TO_FN_LONG, time);          //dr
  }


  void FN_handler_StepToNOP(void) {
    if(calcMode != CM_PEM) {
      refreshRegisterLineRestoreT(); //clearRegisterLine(Y_POSITION_OF_REGISTER_T_LINE - 4, REGISTER_LINE_HEIGHT); //JM FN clear the previous shift function name
    }
    showFunctionName(ITM_NOP, 0, "SF:N");
    FN_timed_out_to_NOP_or_Executed = true;
    underline_softkey(1<<(FN_key_pressed-38), 3);   //  Purposely select row 3 which does not exist, just to activate the 'clear previous line'
    FN_timeouts_in_progress = false;
    fnTimerStop(TO_FN_LONG);                                      //dr
  }


  void FN_handler(void) {                     //JM FN LONGPRESS vv Handler FN Key shift longpress handler
                                              //   Processing cycles here while the key is pressed, that is, after PRESS #1, waiting for RELEASE #2
    if((FN_state == ST_1_PRESS1 || FN_state == ST_3_PRESS2) && FN_timeouts_in_progress && (FN_key_pressed != 0) && !IS_BASEBLANK_(softmenuStack[0].softmenuId) ) {
      if(fnTimerGetStatus(TO_FN_LONG) == TMR_COMPLETED) {
        FN_handle_timed_out_to_EXEC = false;
        if(!shiftF && !shiftG) {                              //From No_Shift State 1
          if(LongPressF == RBX_F1234) {
            FN_handler_StepToF(TIME_FN_1234_F_TO_G);           //To F State 2
          }
          else if(LongPressF == RBX_F124) {
            FN_handler_StepToF(TIME_FN_124_F_TO_NOP);            //To F State 2
          }
          else if(LongPressF == RBX_F14) {
            FN_handler_StepToNOP();                           //To NOP State 4
          }

          #if defined(FN_TIME_DEBUG1)
            printf("Handler 1, KEY=%d =%i\n",FN_key_pressed,nameFunction(FN_key_pressed-37, shiftF, shiftG));
          #endif // FN_TIME_DEBUG1
        }
        else if(shiftF && !shiftG) {                          //From F State 2
          if(LongPressF == RBX_F1234) {
            FN_handler_StepToG(TIME_FN_1234_G_TO_NOP);            //To G State 3
          }
          else if(LongPressF == RBX_F124) {
            FN_handler_StepToNOP();                           //To NOP State 4
          }
          #if defined(FN_TIME_DEBUG1)
            printf("Handler 2, KEY=%d =%i\n",FN_key_pressed,nameFunction(FN_key_pressed-37, shiftF, shiftG));
          #endif // FN_TIME_DEBUG1
        }
        else if((!shiftF && shiftG) || (shiftF && shiftG)) {  //From G: 3 (or illegal state FG)
          FN_handler_StepToNOP();                             //To NOP State 4
          #if defined(FN_TIME_DEBUG1)
            printf("Handler 3, KEY=%d =%i\n",FN_key_pressed,nameFunction(FN_key_pressed-37, shiftF, shiftG));
          #endif // FN_TIME_DEBUG1
        }
      }
    }
  }


#if defined(LONGPRESS_CFG)   // only when allowed by LONGPRESS_CFG
  static void _assignLongPressKey(int keyCode) {
    char kc[4] = {};
    kc[0] = (keyCode / 10) + '0';
    kc[1] = (keyCode % 10) + '0';
    kc[2] = 0;
    shiftF = false;
    shiftG = true;
    assignToKey(kc);
    itemToBeAssigned = 0;
    leaveTamModeIfEnabled();
    keyActionProcessed = true;
    calcMode = previousCalcMode;
    shiftF = shiftG = false;
  }

  void _executeItem(int16_t item, int keyCode) {
    char *funcParam = "";

    keyStateCode = (getSystemFlag(FLAG_ALPHA) ? 3 : 0) + 2;
    funcParam = (char *)getNthString((uint8_t *)userKeyLabel, keyCode * 6 + keyStateCode);
    if(item == ITM_RCL && getSystemFlag(FLAG_USER) && funcParam[0] != 0) {
      calcRegister_t var = findNamedVariable(funcParam);
      if(var != INVALID_VARIABLE) {
        if(calcMode == CM_PEM) {  // Insert user variable recall in program
          insertUserItemInProgram(item, funcParam);
        }
        else {                    // Execute item
          reallyRunFunction(item, var);
        }
      }
      else {
        displayCalcErrorMessage(ERROR_UNDEF_SOURCE_VAR, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
           sprintf(errorMessage, "string '%s' is not a named variable", funcParam);
           moreInfoOnError("In function _executeItem:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
    }
    else if(item == ITM_XEQ && getSystemFlag(FLAG_USER) && funcParam[0] != 0) {
      calcRegister_t label = findNamedLabel(funcParam);
      if(label != INVALID_VARIABLE) {
        if(calcMode == CM_PEM) {  // Insert user program call in program
          insertUserItemInProgram(item, funcParam);
        }
        else {                    // Execute item
          reallyRunFunction(item, label);
        }
      }
      else {
        displayCalcErrorMessage(ERROR_LABEL_NOT_FOUND, ERR_REGISTER_LINE, REGISTER_X);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            sprintf(errorMessage, "string '%s' is not a named label", funcParam);
            moreInfoOnError("In function _executeItem:", errorMessage, NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
    }
    else {
      runFunction(item);
    }
  }
#endif // LONGPRESS_CFG

  void Shft_handler() {
    if(Shft_LongPress_f_g) {
      if(fnTimerGetStatus(TO_FG_LONG) == TMR_COMPLETED) {
        Shft_LongPress_f_g = false;
        fnTimerStop(TO_3S_CTFF);
        fnTimerStop(TO_FG_LONG);
      #if defined(LONGPRESS_CFG)   // only when allowed by LONGPRESS_CFG
        int keyCode;

        int16_t item;
        if((shiftF || shiftG)                                                           // this is for R47 ShiftF or shiftG, adding long press assignments - DL 2025/12/03
            && (calcMode != CM_EIM) && (calcMode != CM_MIM)) {  // f and g longpress temporarily disabled in EIM and MIM
          //leaveTamModeIfEnabled();
          keyCode = (shiftF ? 10 : 11);  // R47 'f' or 'g' keyCode
          calcKey_t *key = kbd_usr + keyCode;
          item = key->gShifted;
          if((calcMode == CM_ASSIGN) && (itemToBeAssigned !=0)) {
            if(previousCalcMode != CM_AIM) {   // No long press assignments in AIM
              _assignLongPressKey(keyCode);
            }
            shiftF = 0;
            shiftG = 0;
          }
          else if(tam.alpha || !tam.mode){
            if(calcMode == CM_NIM && item != ITM_ms && item != ITM_CC && item != ITM_op_j && item != ITM_op_j_pol && item != ITM_dotD
               && item != ITM_HASH_JM && item != ITM_toINT && item != ITM_BACKSPACE && indexOfItems[item].func != addItemToBuffer) {
              delayCloseNim = false;
              closeNim();
              screenUpdatingMode &= ~SCRUPD_MANUAL_MENU;
            }
            if(getSystemFlag(FLAG_USER) && (calcMode != CM_AIM) && (calcMode != CM_EIM) && !getSystemFlag(FLAG_ALPHA) && (item > 0)) {

              if((calcMode == CM_NIM  || (calcMode == CM_PEM && aimBuffer[0] != 0 && !getSystemFlag(FLAG_ALPHA)))
                  && (item == ITM_HASH_JM || item == ITM_toINT )) {
                  processKeyAction(item);
              }
              else if(calcMode != CM_PEM && indexOfItems[item].func == addItemToBuffer) {
                addItemToNimBuffer(item);
              }
              else {
                _executeItem(item,keyCode);
              }

            }
            else {
              char *funcParam = "";
              keyStateCode = (getSystemFlag(FLAG_ALPHA) ? 3 : 0) + 2;
              funcParam = (char *)getNthString((uint8_t *)userKeyLabel, keyCode * 6 + keyStateCode);
              setCurrentUserMenu(item, funcParam);
              if(shiftF) {
                if(getSystemFlag(FLAG_ALPHA) && ((currentMenu() == -MNU_MyAlpha) || (currentMenu() == -MNU_AIMCATALOG) || isAlphabeticSoftmenu())) {
                  popSoftmenu();
                }
                //showSoftmenu((calcMode == CM_AIM) || ((calcMode == CM_ASSIGN) && (previousCalcMode == CM_AIM)) || tam.alpha ? -MNU_ALPHA :
                //             (getSystemFlag(FLAG_USER) && (key->gShifted != ITM_NULL) ? key->gShifted : -MNU_HOME));

                if(tam.alpha) {
                  showSoftmenu(-MNU_TAMALPHA);
                }
                else if((calcMode == CM_AIM) || getSystemFlag(FLAG_ALPHA) || ((calcMode == CM_ASSIGN) && (previousCalcMode == CM_AIM))) {
                  showSoftmenu(-MNU_ALPHA);
                }
                else if(getSystemFlag(FLAG_USER) && (key->gShifted != ITM_NULL)) {
                  showSoftmenu(key->gShifted);
                }
                else {
                  showSoftmenu(-MNU_HOME);
                }
                showSoftmenuCurrentPart();
              }
              else {
                bool_t baseOverrideOnce = true;
                BASE_OVERRIDEONCE = baseOverrideOnce;
                if((calcMode == CM_AIM) || getSystemFlag(FLAG_ALPHA) || ((calcMode == CM_ASSIGN) && (previousCalcMode == CM_AIM)) || tam.alpha) {
                  showSoftmenu(-MNU_MyAlpha);
                }
                else if(getSystemFlag(FLAG_USER) && (key->gShifted != ITM_NULL)) {
                  showSoftmenu(key->gShifted);
                }
                else if(getSystemFlag(FLAG_BASE_MYM) || getSystemFlag(FLAG_BASE_HOME)) {
                  showSoftmenu(-MNU_MyMenu);
                }
                else {
                  baseOverrideOnce = false;
                  BASE_OVERRIDEONCE = baseOverrideOnce;
                  fnExitAllMenus(0);               // If MyMb and HOMEb are both clear, return to the blank base menu display
                }
                BASE_OVERRIDEONCE = baseOverrideOnce;
                showSoftmenuCurrentPart();
                BASE_OVERRIDEONCE = baseOverrideOnce;            //for upcoming refresh*
              }
            }
          }
          else if(tam.mode && indexOfItems[item].func == addItemToBuffer) {
            addItemToBuffer(item);
          }
          shiftF = 0;
          shiftG = 0;
          screenUpdatingMode = SCRUPD_AUTO;
          refreshScreen(23);
        }
      #endif // LONGPRESS_CFG
        shiftF = 0;
        shiftG = 0;
        showShiftState();
        if((calcMode == CM_AIM) || (calcMode == CM_EIM)) {
          calcModeAimGui();
        }
      }
    }
    else if(Shft_timeouts) {
      if(fnTimerGetStatus(TO_FG_LONG) == TMR_COMPLETED) {
        fnTimerStop(TO_3S_CTFF);
        if(!shiftF && !shiftG) {
          shiftF = true;
          fnTimerStart(TO_FG_LONG, TO_FG_LONG, JM_TO_FG_LONG);
          showShiftState();
        }
        else if(shiftF && !shiftG) {
          shiftG = true;
          shiftF = false;
          fnTimerStart(TO_FG_LONG, TO_FG_LONG, JM_TO_FG_LONG);
          showShiftState();
        }
        else if((!shiftF && shiftG) || (shiftF && shiftG)) {
          Shft_timeouts = false;
          resetShiftState();                                       //force into no shift state, i.e. to wait
          if((calcMode == CM_ASSIGN) && (itemToBeAssigned !=0)) {
            #if defined(LONGPRESS_CFG)   // only when allowed by LONGPRESS_CFG
              int keyCode = (calcModel == USER_R47bk_fg) ? 11 : (calcModel == USER_R47fg_bk || calcModel == USER_R47fg_g) ? 10 : (calcModel == USER_C47 || calcModel == USER_DM42) ? 27 : 9999;
              //shiftF = 1;
              if(previousCalcMode != CM_AIM) {   // No long press assignments in AIM
                _assignLongPressKey(keyCode);
              }
            #endif // LONGPRESS_CFG
            shiftF = 0;
            shiftG = 0;
            screenUpdatingMode = SCRUPD_AUTO;
            refreshScreen(23);
          }
          else {
            openHOMEorMyM(keypress_long_f);
          }
        }
      }
    }
  }


//longpress DOT function during AIM EIM and PEM alpha:

//2.  2025-08-02
//All currentKeyCode checking below should be replaced with last item checking.
//  Although it does not really matter because this section is for non-assignable, i.e. hardware keys

//3. 2025-08-02
// The 'exclude ENTER and BACKSPACE' section below has UP and DN longpress added. This must be checked anyway in PEM.
//   ... || (calcMode == CM_PEM && getSystemFlag(FLAG_ALPHA) && !tam.mode) to be added to 'if((calcMode == CM_AIM || calcMode == CM_EIM) &&'



  void LongpressKey_handler() {
    if(fnTimerGetStatus(TO_CL_LONG) == TMR_COMPLETED) {
      if(JM_auto_longpress_enabled != 0) {
        char *funcParam;
        int keyStateCode = (getSystemFlag(FLAG_ALPHA) ? 3 : 0) + ((LongPressM == RBX_M124) ? 1 : longpressDelayedkey3 ? 1 : 2);
        funcParam = (char *)getNthString((uint8_t *)userKeyLabel, currentKeyCode * 6 + keyStateCode);

        if(calcMode == CM_NORMAL && programRunStop == PGM_STOPPED && (isArrowUp(currentKeyCode))) {
          aimBuffer[0] = 0;
          ++currentLocalStepNumber;
          currentStep = findNextStep(currentStep);
          refreshRegisterLine(REGISTER_T);
        }
        else if(calcMode == CM_NORMAL && programRunStop == PGM_SINGLE_STEP && (isArrowDown(currentKeyCode))) {
          programRunStop = PGM_STOPPED;
          refreshRegisterLine(REGISTER_T);
        }

        //printf("LongpressKey_handler = %d %s currentKeyCode=%d\n",JM_auto_longpress_enabled, indexOfItems[abs(JM_auto_longpress_enabled)].itemCatalogName, currentKeyCode);
        if((calcMode == CM_AIM || calcMode == CM_EIM || tam.alpha) && !( (currentKeyCode == 16 || currentKeyCode == 12)) && JM_auto_longpress_enabled != ITM_CLRMOD && JM_auto_longpress_enabled > 0) {  //using keyboard positions, as these cannot be re-assigned. It should not work with re-assigned keys on different places.
                                                                 // Exclude  BACKSP                   ENTER
          if(JM_auto_longpress_enabled == ITM_NOP) {
            return;
          }
          if(isArrowUp(currentKeyCode) || isArrowDown(currentKeyCode)) {
            // stub for code to process on up1/down longpress
            return;
          }

          fnKeyBackspace(NOPARAM);
          addItemToBuffer(JM_auto_longpress_enabled);
          FN_timeouts_in_progress = false;
          fnTimerStop(TO_FN_LONG);
          if(calcMode == CM_AIM) {
            refreshRegisterLine(AIM_REGISTER_LINE);   //TO DISPLAY KEYPRESS DIRECTLY AFTER PRESS, NOT ONLY UPON RELEASE
          } else
          if(calcMode == CM_EIM || tam.alpha) {
            screenUpdatingMode &= ~(SCRUPD_MANUAL_MENU | SCRUPD_SKIP_MENU_ONE_TIME);
            refreshScreen(131);
          }
          return;
        }
        else if((funcParam[0] != 0) && ((JM_auto_longpress_enabled == -MNU_DYNAMIC) || (JM_auto_longpress_enabled == ITM_XEQ) || (JM_auto_longpress_enabled == ITM_RCL))) { // For user menu, prog or variable a-feirassignment
          showFunctionName(JM_auto_longpress_enabled, JM_TO_CL_LONG + 50, funcParam);     //Add a marginal amout of time to prevent racing of end conditions.
        }
        else if(funcParam[0] == 0 && (JM_auto_longpress_enabled == ITM_XEQ || JM_auto_longpress_enabled == ITM_GTO)) {  //from KEYA-F longpress
          showFunctionName(JM_auto_longpress_enabled, JM_TO_CL_LONG + 50, funcParam);     //Add a marginal amout of time to prevent racing of end conditions.
        }
        else {
          showFunctionName(JM_auto_longpress_enabled, JM_TO_CL_LONG + 50, "SF:LL");     //Add a marginal amout of time to prevent racing of end conditions.
        }
        JM_auto_longpress_enabled = 0;                                       //showFunctionName must not time out longer than the timer that is started below

        //Setup up next long press activation possibility
        if(longpressDelayedkey2) {
          JM_auto_longpress_enabled = longpressDelayedkey2;
          longpressDelayedkey2 = 0;
        }
        else if(longpressDelayedkey3) {
          JM_auto_longpress_enabled = longpressDelayedkey3;
          longpressDelayedkey3 = 0;
        }
        else {
          JM_auto_longpress_enabled = ITM_NOP;
        }
        if(JM_auto_longpress_enabled) {
          fnTimerStart(TO_CL_LONG, TO_CL_LONG, JM_TO_CL_LONG);
        }
      }
    }
  }


  void Shft_stop() {
    Shft_timeouts = false;
    resetShiftState();                        //force into no shift state, i.e. to wait
  }


  //JM: If maxiC is set, then, if a glyph is not found in numericfont, it will be fetched and enlarged from standardfont
  uint8_t  combinationFonts = combinationFontsDefault;
  uint8_t  miniC = 0;                                                              //JM miniature letters
  uint8_t  maxiC = 0;                                                              //JM ENLARGE letters. Use Numericfont & combinationFontsDefault=2;
  bool_t   noShow = false;                                                         //JM
  uint8_t  displaymode = stdNoEnlarge;


uint16_t str2dec(char* ch) {
  //uint16_t ll = ch[1], hh = ch[0],
  uint16_t res = (uint8_t)ch[1] + (((uint8_t)ch[0]) << 8);
  //printf("= %u *256+ %u = %u\n", (uint8_t)hh, (uint8_t)ll, (uint16_t)res);
return res;
}

//bool_t ratherUseEnlargement(uint16_t charCode) {
//  return ((bool_t) (
//    ((charCode >= str2dec(STD_SUP_BOLD_f)) && (charCode <= str2dec(STD_SUP_BOLD_h))) ||
//    ( charCode == str2dec(STD_SUP_BOLD_r)) ||
//    ( charCode == str2dec(STD_SUP_BOLD_x)) ||
//
//    ((charCode >= str2dec(STD_SUB_f)) && (charCode <= str2dec(STD_SUB_h))) ||
//    ( charCode == str2dec(STD_SUB_r)) ||
//    ( charCode == str2dec(STD_SUB_t))
//    ));
//}


  uint8_t  boldString = 0;
  uint8_t  compressString = 0;
  uint8_t  raiseString = 0;

  uint32_t showGlyphCode(uint16_t charCode, const font_t *font, uint32_t x, uint32_t y, videoMode_t videoMode, bool_t showLeadingCols, bool_t showEndingCols, bool_t noPreClear) {
    uint32_t col, row, xGlyph, endingCols;
    int32_t  glyphId;
    int8_t   byte, *data;
    const glyph_t *glyph;

    if(charCode == STD_NOCHAR) {
      return x; //This is special usage of the 01 ASCII code, to ignore the code and return with nothing printed
    }

    bool_t enlarge = false;                                   //JM ENLARGE vv
    if(combinationFonts == stdnumEnlarge || combinationFonts == numHalf) {
      if(maxiC == 1 && font == &numericFont) {                //JM allow enlargements
        glyphId = findGlyph(font, charCode);
        //printf("DDDDDD %d %d --- %u\n",glyphId, charCode, ratherUseEnlargement(charCode));
        if(glyphId < 0) {// || ratherUseEnlargement(charCode)) {           //JM if there is not a large glyph, enlarge the small letter
          enlarge = true;
          font = &standardFont;
        }
      }
    }
    else if(combinationFonts == 1) {
      if(maxiC == 1 && font == &standardFont) {                //JM allow enlargements
        enlarge = true;
      }                                                       //JM ENLARGE ^^
    }

    #if !defined(GENERATE_CATALOGS)
      if(checkHP && font == &numericFont && HPFONT) {
        charCodeHPReplacement(&charCode);
      }
    #endif //GENERATE_CATALOGS

    glyphId = findGlyph(font, charCode);
    if(glyphId >= 0) {
      glyph = (font->glyphs) + glyphId;
    }
    else if(glyphId == -1) {
      generateNotFoundGlyph(-1, charCode);
      glyph = &glyphNotFound;
    }
    else if(glyphId == -2) {
      generateNotFoundGlyph(-2, charCode);
      glyph = &glyphNotFound;
    }
    else {
      glyph = NULL;
    }

    if(glyph == NULL) {
      sprintf(errorMessage, commonBugScreenMessages[bugMsgValueReturnedByFindGlyph], "showGlyphCode", glyphId);
      displayBugScreen(errorMessage);
      return 0;
    }

    int32_t yy = (int32_t)y; //get unsigned integer that was negative, back to signed integer negative
    data = (int8_t *)glyph->data;
    uint32_t y0 = y;                                                   //JMmini 0-reference
    xGlyph      = showLeadingCols ? glyph->colsBeforeGlyph : 0;
    endingCols  = showEndingCols ? glyph->colsAfterGlyph : 0;

    bool_t numDouble = font == &numericFont && checkHP && temporaryInformation == TI_NO_INFO; //&& charCodeFromString(STD_MODE_G, 0)!=charCode && charCodeFromString(STD_MODE_G, 0)!=charCode; //this also triggers the vertical doubling
    uint16_t doubling = numDouble ? DOUBLING : DOUBLINGBASEX;      //this is the horizontal factor, 8 is normal, so 16 is double

    // Clearing the space needed by the glyph
    bool_t rep_enlarge = numDouble || (enlarge && combinationFonts != 0);                //JM ENLARGE
    uint32_t yNewMaxDx = (rep_enlarge ? 2 : 1) * (((glyph->rowsAboveGlyph + glyph->rowsGlyph + glyph->rowsBelowGlyph) >> miniC) - (rep_enlarge ? 4 : 0));
    if(!noShow && !noPreClear) {
      lcd_fill_rect(x, max(0,yy), (uint32_t)(doubling * ((xGlyph + glyph->colsGlyph + endingCols) >> miniC)) >> 3, max(0, (int32_t)(yNewMaxDx) + (yy<0 ? yy:0)), (videoMode == vmNormal ? LCD_SET_VALUE : LCD_EMPTY_VALUE));  //JMmini
    }
    if(displaymode == numHalf) {
      y += (uint32_t)(glyph->rowsAboveGlyph*REDUCT_A/REDUCT_B*(rep_enlarge ? 2 : 1));
    }
    else {
      y += glyph->rowsAboveGlyph*(rep_enlarge ? 2 : 1);
    }        //JM REDUCE and DOUBLE
    //x += xGlyph; //JM

    // Choose pencil
    void (*setPixel)(uint32_t,uint32_t) = (videoMode == vmNormal)?&setBlackPixel:&setWhitePixel;
    // Drawing the glyph
    for(row=0; row<glyph->rowsGlyph; row++, y++) {
      if(displaymode == numHalf) {
        if((int)((REDUCT_A*row+REDUCT_OFF)) % REDUCT_B == 0) {
          y--;
        }
      }                           //JM REDUCE
      // Drawing the columns of the glyph
      for(col=0; col<glyph->colsGlyph; col++) {
        if (!(col%8)) {
          byte = *(data++);
          if(miniC!=0) {
            byte = (uint8_t)byte | (((uint8_t)byte) << 1);           //JMmini
          }
        }

        if(byte & 0x80 && !noShow) { // MSB set
          uint32_t x1 = x+((((doubling * (xGlyph+col)) >> miniC)) >> 3);
          uint32_t x2 = x1;
          uint32_t y1 = min(SCREEN_HEIGHT-1, max(0,yy + (int32_t)min(yNewMaxDx,  ((y-y0) >> miniC))));
          uint32_t y2 = min(SCREEN_HEIGHT-1, max(0,yy + (int32_t)min(yNewMaxDx,1+((y-y0) >> miniC))));
          if(x2 > 0) {
            x2--;
          }
          setPixel(x1,y1);
          if(boldString == 1) {
            setPixel(x1+1,y1);
          }
          if(numDouble) {
            setPixel(x2,y1);
          }
          if(rep_enlarge) {
            setPixel(x1,y2);
            if(numDouble) {
              setPixel(x2,y2);
            }
          }
        }

        byte <<= 1;
      }
      if(rep_enlarge && row!=3 && row!=6 && row!=9 && row!=12) {
        y++; //JM ENLARGE vv do not advance the row counter for four rows, to match the row height of the enlarge font
      }
    }
    return x + boldString + (((doubling * (xGlyph + glyph->colsGlyph + endingCols)) >> miniC) >> 3);        //JMmini
  }


  uint32_t showGlyph(const char *ch, const font_t *font, uint32_t x, uint32_t y, videoMode_t videoMode, bool_t showLeadingCols, bool_t showEndingCols, bool_t noPreClear) {
    return showGlyphCode(charCodeFromString(ch, 0), font, x, y, videoMode, showLeadingCols, showEndingCols, noPreClear);
  }


  /* Finds the cols and rows for a glyph.
   *
   * \param[in]     ch     const char*   String whose first glyph is to find the bounds for
   * \param[in,out] offset uint16_t*     Offset for string or null if zero should be used
   * \param[in]     font   const font_t* Font to use
   * \param[out]    col    uint32_t*     Number of columns for the glyph
   * \param[out]    row    uint32_t*     Number of rows for the glyph
   */
  static void getGlyphBounds(const char *ch, uint16_t *offset, const font_t *font, uint32_t *col, uint32_t *row) {
    int32_t        glyphId;
    const glyph_t *glyph;

    glyphId = findGlyph(font, charCodeFromString(ch, offset));
    if(glyphId < 0) {
      sprintf(errorMessage, commonBugScreenMessages[bugMsgValueReturnedByFindGlyph], "getGlyphBounds", glyphId);
      displayBugScreen(errorMessage);
      return;
    }
    glyph = (font->glyphs) + glyphId;
    *col = glyph->colsBeforeGlyph + glyph->colsGlyph + glyph->colsAfterGlyph;
    *row = glyph->rowsAboveGlyph + glyph->rowsGlyph + glyph->rowsBelowGlyph;
  }


  /* Finds the cols and rows for a string if showing leading and ending columns.
   *
   * \param[in]  ch   const char*   String to find the bounds of
   * \param[in]  font const font_t* Font to use
   * \param[out] col  uint32_t*     Number of columns for the string
   * \param[out] row  uint32_t*     Number of rows for the string
   */
  void getStringBounds(const char *string, const font_t *font, uint32_t *col, uint32_t *row) {
    uint16_t ch = 0;
    uint32_t lcol, lrow;
    lcol = 0;
    lrow = 0;
    *col = 0;
    *row = 0;

    while(string[ch] != 0) {
      getGlyphBounds(string, &ch, font, &lcol, &lrow);
      *col += lcol;
      if(lrow > *row) {
        *row = lrow;
      }
    }
  }



  static uint32_t _doShowString(const char *string, const font_t *font, uint32_t x, uint32_t y, char **resStr, uint32_t width, videoMode_t videoMode, bool_t showLeadingCols, bool_t showEndingCols, bool_t LF) {
    uint16_t ch, lg;
    bool_t   slc, sec;
    uint32_t prevX = x;
    uint32_t orgX = x;

    lg = stringByteLength(string);

    ch = 0;
    while(string[ch] != 0) {
      if(lg == 1 || (lg == 2 && (string[0] & 0x80))) {// The string is 1 glyph long
        slc = showLeadingCols;
        sec = showEndingCols;
      }
      else if(ch == 0) {// First glyph
        slc = showLeadingCols;
        sec = true;
      }
      else if(ch == lg-1 || (ch == lg-2 && (string[ch] & 0x80))) {// Last glyph
        slc = true;
        sec = showEndingCols;
      }
      else {// Glyph between first and last glyph
        slc = true;
        sec = true;
      }

      if(LF && x > SCREEN_WIDTH - 20 && !noShow) {                      //auto LF when line is full
        noShow = true;
        uint16_t tmp = ch;
        if(x + showGlyphCode(charCodeFromString(string, &tmp), font, 0, 0, videoMode, slc, sec, false) - compressString > SCREEN_WIDTH) {
          x = orgX;
          prevX = x;
          y += (font == &tinyFont ? 8 : 20);
        }
        noShow = false;
      }

      x = showGlyphCode(charCodeFromString(string, &ch), font, x, y - raiseString, videoMode, slc, sec, false) - compressString;
      if(resStr) { // for stringAfterPixelsC47
        if(x > width) {
          if(!showEndingCols) {
            uint32_t tmpX = x;
            ch = *resStr - string;
            x = showGlyphCode(charCodeFromString(string, &ch), font, prevX, y - raiseString, videoMode, true, false, false) - compressString;
            if(x <= width) {
              *resStr = (char *)(string + ch);
            }
            x = tmpX;
          }
          break;
        }
        else {
          *resStr = (char *)(string + ch);
          prevX = x;
        }
      }
      uint16_t tmp = ch;                                     //LF after 0x0A is recognized (/n)
      while(LF && (charCodeFromString(string, &tmp) == 0x0A)) {   //do not touch character pointer
        charCodeFromString(string, &ch);                       //increment character pointer to skip 0x0A
        x = orgX;
        prevX = x;
        y += (font == &tinyFont ? 8 : 20);
      }
    }
    compressString = 0;        //JM compressString
    raiseString = 0;
    return x;
  }


  uint32_t showStringEnhanced(const char *string, const font_t *font, uint32_t x, uint32_t y, videoMode_t videoMode, bool_t showLeadingCols, bool_t showEndingCols, uint8_t compress1, uint8_t raise1, uint8_t noShow1, uint8_t boldString1, bool_t lf) {
    boldString = boldString1;
    compressString = compress1;
    raiseString = raise1;
    noShow = noShow1;
    uint32_t tmp = _doShowString(string, font, x, y, NULL, 0, videoMode, showLeadingCols, showEndingCols, lf);
    boldString = 0;
    compressString = 0;
    raiseString = 0;
    noShow = 0;
    return tmp;
  }


  uint32_t showString(const char *string, const font_t *font, uint32_t x, uint32_t y, videoMode_t videoMode, bool_t showLeadingCols, bool_t showEndingCols) {
    return _doShowString(string, font, x, y, NULL, 0, videoMode, showLeadingCols, showEndingCols, NO_LF);
  }


  static char *_stringAfterPixels(const char *string, const font_t *font, uint32_t width, bool_t showLeadingCols, bool_t showEndingCols) {
    char *resStr = (char *)string;
    _doShowString(string, font, 0, 0, &resStr, width, vmNormal, showLeadingCols, showEndingCols, NO_LF);
    return resStr;
  }


  static uint32_t _showStringWithLimit(const char *string, const font_t *font, uint32_t limitWidth, bool_t showLeadingCols, bool_t showEndingCols) {
    char *resStr = (char *)string;
    return _doShowString(string, font, 0, 0, &resStr, limitWidth, vmNormal, showLeadingCols, showEndingCols, NO_LF);
  }


  static void _setStringMode(int mode, int comp, const font_t **fontPtr) {
    compressString = comp;
    displaymode = mode;             // miniC and maxiC to be depreciated in favour of displaymode
    switch(mode) {
      case stdNoEnlarge:  miniC = 0 ; maxiC = 0; combinationFonts = combinationFontsDefault; *fontPtr = &standardFont; break;
      case stdEnlarge:    miniC = 0 ; maxiC = 1; combinationFonts = stdEnlarge;              *fontPtr = &standardFont; break;
      case stdnumEnlarge: miniC = 0 ; maxiC = 1; combinationFonts = stdnumEnlarge;           *fontPtr = &numericFont;  break;
      case numSmall:      miniC = 1 ; maxiC = 0; combinationFonts = combinationFontsDefault; *fontPtr = &numericFont;  break;
      case numHalf:       miniC = 0 ; maxiC = 1; combinationFonts = numHalf;                 *fontPtr = &numericFont;  break;
      default:                                                                               *fontPtr = NULL;
    }
  }


  static void _resetStringMode(void) {
    miniC = 0;
    maxiC = 0;
    compressString = 0;
    noShow = false;
    displaymode = stdNoEnlarge;
  }


  uint32_t showStringC47(const char *string, int mode, int comp, uint32_t x, uint32_t y, videoMode_t videoMode, bool_t showLeadingCols, bool_t showEndingCols ) {
    int combinationFontsM = combinationFonts;
    if(combinationFontsDefault == 0) {
      mode = stdNoEnlarge;
    }

    const font_t *font;
    _setStringMode(mode, comp, &font);
    if(font) {
      x = showString(string, font, x, y, videoMode, showLeadingCols, showEndingCols );
    }
    else {
      x = 0;
    }

    combinationFonts = combinationFontsM; _resetStringMode();
    return x;
  }

  char *stringAfterPixelsC47(const char *string, int mode, int comp, uint32_t width, bool_t withLeadingEmptyRows, bool_t withEndingEmptyRows) {
    int combinationFontsM = combinationFonts;
    char *resStr = (char *)string;
    if(combinationFontsDefault == 0) {
      mode = stdNoEnlarge;
    }

    const font_t *font;
    noShow = true;
    _setStringMode(mode, comp, &font);
    if(font) {
      resStr = _stringAfterPixels(string, font, width, withLeadingEmptyRows, withEndingEmptyRows );
    }
    else {
      resStr = (char *)string;
    }

    combinationFonts = combinationFontsM; _resetStringMode();
    return resStr;
  }

  uint32_t stringWidthWithLimitC47(const char *string, int mode, int comp, uint32_t limitWidth, bool_t withLeadingEmptyRows, bool_t withEndingEmptyRows) {
    int combinationFontsM = combinationFonts;
    uint32_t x = 0;
    if(combinationFontsDefault == 0) {
      mode = stdNoEnlarge;
    }

    const font_t *font;
    noShow = true;
    _setStringMode(mode, comp, &font);
    if(font) {
      x = _showStringWithLimit(string, font, limitWidth, withLeadingEmptyRows, withEndingEmptyRows );
    }
    else {
      x = 0;
    }

    combinationFonts = combinationFontsM; _resetStringMode();
    return x;
  }


  uint32_t  stringWidthC47(const char *str, int mode, int comp, bool_t withLeadingEmptyRows, bool_t withEndingEmptyRows){
     noShow = true;
     return showStringC47(str, mode, comp, 0, 0, vmNormal, withLeadingEmptyRows, withEndingEmptyRows );
     //noShow = false; //no need to redo
  }

  void drawSinglePixelFullWidthLine(int y) {
    lcd_fill_rect(0, y, SCREEN_WIDTH, 1, LCD_EMPTY_VALUE);
  }


  void showBottomLine(void) {
    int yoff = 0;
    if(!( (temporaryInformation == TI_SHOW_REGISTER_SMALL && tmpString[5*SHOWLineSize] != 0) ||
          (temporaryInformation == TI_SHOW_REGISTER_TINY && tmpString[14*SHOWLineSize] != 0)      )    //The softmenu space is not used
      || (overrideShowBottomLine > 0) ) {


      if(overrideShowBottomLine > 0) {
        yoff = SCREEN_HEIGHT - REGISTER_LINE_HEIGHT*(overrideShowBottomLine)/10.0f;   // 40 means 4.0 registers from bottom
      }
      else {
        yoff = SCREEN_HEIGHT - REGISTER_LINE_HEIGHT*2;
      }

      int offs = (temporaryInformation == TI_SHOW_REGISTER_BIG ? - 2 : temporaryInformation == TI_SHOW_REGISTER_SMALL ? 0 : temporaryInformation == TI_SHOW_REGISTER_TINY ? 0 : 0);

      drawSinglePixelFullWidthLine(yoff + offs); //HERE SHOW LINE SMALL & TINY

      overrideShowBottomLine = 0;
    }
  }


  void showDispSmall(uint16_t offset, uint8_t _h1) {
    #define line_small 21
    #define line_tiny  10
    const uint32_t line_hMultiLineEdOffset = Y_POSITION_OF_REGISTER_T_LINE;
    if(tmpString[offset]) {
      uint32_t w = stringWidth(tmpString + offset, temporaryInformation == TI_SHOW_REGISTER_SMALL ? &standardFont : &tinyFont, true, true);
      showString(tmpString + offset, temporaryInformation == TI_SHOW_REGISTER_SMALL ? &standardFont : &tinyFont, SCREEN_WIDTH - w, line_hMultiLineEdOffset + (temporaryInformation == TI_SHOW_REGISTER_SMALL ? line_small : line_tiny) * _h1, vmNormal, true, true);
      #if defined(VERBOSE_SCREEN) && defined(PC_BUILD)
        printf("^^^^NEW SHOW: %s\n", tmpString + offset);
      #endif // VERBOSE_SCREEN && PC_BUILD
    }
  }


  void showDisp(uint16_t offset, uint8_t _h1) {
    #define line_h1 37
    const uint32_t line_hMultiLineEdOffset = Y_POSITION_OF_REGISTER_T_LINE - 3;

    uint32_t w = stringWidthWithLimitC47(tmpString + offset, stdnumEnlarge, nocompress, SCREEN_WIDTH, true, true);
    if(w < SCREEN_WIDTH) {
      showStringC47(tmpString + offset, stdnumEnlarge, nocompress,  SCREEN_WIDTH - w, line_hMultiLineEdOffset + line_h1 * _h1, vmNormal, true, true);
      return;
    }

    w = stringWidthWithLimitC47(tmpString + offset, stdEnlarge, nocompress, SCREEN_WIDTH, true, true);
    if(w < SCREEN_WIDTH) {
      showStringC47(tmpString + offset, stdEnlarge, nocompress,  SCREEN_WIDTH - w, line_hMultiLineEdOffset + line_h1 * _h1, vmNormal, true, true);
      return;
    }

    w = stringWidthWithLimitC47(tmpString + offset, stdNoEnlarge, nocompress, SCREEN_WIDTH, true, true);
    if(w < SCREEN_WIDTH) {
      showStringC47(tmpString + offset, stdNoEnlarge, nocompress,  SCREEN_WIDTH - w, line_hMultiLineEdOffset + line_h1 * _h1, vmNormal, true, true);
      return;
    }

    w = stringWidthWithLimitC47(tmpString + offset, numSmall, nocompress, SCREEN_WIDTH, true, true);
    if(w < SCREEN_WIDTH) {
      showStringC47(tmpString + offset, numSmall, nocompress,  SCREEN_WIDTH - w, line_hMultiLineEdOffset + line_h1 * _h1, vmNormal, true, true);
      return;
    }

    w = stringWidthWithLimitC47(tmpString + offset, numSmall, DO_compress, SCREEN_WIDTH, true, true);
    if(w < SCREEN_WIDTH) {
      showStringC47(tmpString + offset, numSmall, DO_compress,  SCREEN_WIDTH - w, line_hMultiLineEdOffset + line_h1 * _h1, vmNormal, true, true);
      return;
    }

    w = stringWidth(tmpString + offset + 2, &standardFont, true, true);
    if(w < SCREEN_WIDTH) {
      showString(tmpString + offset + 2, &standardFont,  SCREEN_WIDTH - w, line_hMultiLineEdOffset + line_h1 * _h1, vmNormal, true, true);
      return;
    }
  }


  #if defined(TEXT_MULTILINE_EDIT)
    uint32_t showStringEdC47(uint32_t lastline, int16_t offset, int16_t edcursor, const char *string, uint32_t x, uint32_t y, videoMode_t videoMode, bool_t showLeadingCols, bool_t showEndingCols, bool_t noshow1) {
      uint16_t ch, charCode, lg;
      int16_t  glyphId;
      bool_t   slc, sec;
      uint32_t  numPixels, orglastlines, tmpxy;
      const    glyph_t *glyph;
      uint8_t  yincr;
      const    font_t *font;

      //combinationFonts = 0;

      if(combinationFonts == 2) {
        font = &numericFont;                             //JM ENLARGE
      }
      else {
        font = &standardFont;                            //JM ENLARGE
      }

      lg = stringByteLength(string + offset);

      //see original size jumping code: c7de947 108_02 2022-08-31
      yincr         = 35;       //JM ENLARGE 21   distasnce between editing wrapped lines
      xMultiLineEdOffset      = 0;    //pixels 40
      if(stringWidth(string + offset, &numericFont, showLeadingCols, showEndingCols) > SCREEN_WIDTH - 50 ) {  //jump from large letters to small letters
        multiEdLines = 3;
        yMultiLineEdOffset = 1;
        screenUpdatingMode &= ~SCRUPD_MANUAL_STACK;
        last_CM = calcMode; //ignore this method of prioritising refreshes. This method is sunsetting.
      }
      else {
        multiEdLines = 2;              //jump back to small letters
        yMultiLineEdOffset = 3;
        screenUpdatingMode &= ~SCRUPD_MANUAL_STACK;
        last_CM = calcMode; //ignore this method of prioritising refreshes. This method is sunsetting.
      }

      if(checkHP) {
        multiEdLines = 1;
        yMultiLineEdOffset = 1;
        screenUpdatingMode &= ~SCRUPD_MANUAL_STACK;
        last_CM = calcMode; //ignore this method of prioritising refreshes. This method is sunsetting.
        yincr = 1;
      }

      orglastlines = lastline;

      if(lastline > yMultiLineEdOffset) {
        x = xMultiLineEdOffset;
        y = (yincr-1) + yMultiLineEdOffset * (yincr-1);
      }

      //****************************************
      ch = offset;
      while(string[ch] != 0) {
        //printf("%3d:%3d ",ch,(uint8_t)string[ch]);

        if(lg == 1 || (lg == 2 && (string[offset] & 0x80))) {// The string is 1 glyph long
          slc = showLeadingCols;
          sec = showEndingCols;
        }
        else if(ch == 0) {// First glyph
          slc = showLeadingCols;
          sec = true;
        }
        else if(ch == lg-1 || (ch == lg-2 && (string[ch] & 0x80))) {// Last glyph
          slc = true;
          sec = showEndingCols;
        }
        else {// Glyph between first and last glyph
          slc = true;
          sec = true;
        }

        //printf("^^^^ ch=%d edcursor=%d string[ch]=%d \n",ch, edcursor, string[ch]);

        if(ch == edcursor) {                 //draw cursor
          current_cursor_x = x;
          current_cursor_y = y;
          tmpxy = y-1;
          while(tmpxy < y + (yincr+1)) {
            if(!noshow1) {
              setBlackPixel(x,tmpxy);
            }
            if(!noshow1) {
              setBlackPixel(x+1,tmpxy);
            }
            tmpxy++;
          }
          x += 2;
        }

        charCode = (uint8_t)string[ch++];                         //get glyph code
        if(charCode & 0x80) {// MSB set?
          charCode = (charCode<<8) | (uint8_t)string[ch++];
        }
        glyph = NULL;
        glyphId = findGlyph(font, charCode);
        if(glyphId >= 0) {
          glyph = (font->glyphs) + glyphId;
        }
        else if(glyphId == -1) {                   //JMvv
          generateNotFoundGlyph(-1, charCode);
          glyph = &glyphNotFound;
        }
        else if(glyphId == -2) {
          generateNotFoundGlyph(-2, charCode);
          glyph = &glyphNotFound;
        }
        else {
          glyph = NULL;
        }                                         //JM^^

        numPixels = 0;

        numPixels += glyph->colsGlyph + glyph->colsAfterGlyph + glyph->colsBeforeGlyph;    // get width of glyph
        //printf(">>> lastline=%d string[ch]=%d x=%d numPixels=%d", lastline, string[ch], x, numPixels);
        if(string[ch]== 0) {
          numPixels += 8;
        }
        //printf("±±± lastline=%d string[ch]=%d x=%d numPixels=%d\n", lastline, string[ch], x, numPixels);
        #define ALLOW_PIXELS_FOR_CURSOR 12
        if(x + numPixels > SCREEN_WIDTH-1-ALLOW_PIXELS_FOR_CURSOR && lastline == orglastlines) {
          x = xMultiLineEdOffset;
          y += yincr;
          lastline--;
        }
        else if(x + numPixels > SCREEN_WIDTH-1-ALLOW_PIXELS_FOR_CURSOR && lastline > 1) {
          x = 1;
          y += yincr;
          lastline--;
        }
        else if(x + numPixels > SCREEN_WIDTH-1-ALLOW_PIXELS_FOR_CURSOR && lastline <= 1) {
          xCursor = x;
          yCursor = y;
          return x;
        }
        #undef ALLOW_PIXELS_FOR_CURSOR

        maxiC = 1;                                                                            //JM
        if(y!=(uint32_t)(-100)) {
          x = showGlyphCode(charCode, font, x, y - raiseString, videoMode, slc, sec, false) - compressString;        //JM compressString
        }
        maxiC = 0;                                                                            //JM
      }
      //printf("\n");

      xCursor = x;
      yCursor = y;
      compressString = 0;                                                                     //JM compressString
      raiseString = 0;
      return xCursor;
    }
  #endif // TEXT_MULTILINE_EDIT
                                                          //JMCURSOR ^^


  void findOffset(void) {             //C47 JM
    int32_t strWidth = (int32_t)stringWidthC47(aimBuffer, combinationFonts, nocompress, true, true);
    strWidth -= SCREEN_WIDTH * multiEdLines - 45;
    if(strWidth < 0) {
      strWidth = 0;
    }
    char *offset = stringAfterPixelsC47(aimBuffer, combinationFonts, nocompress, strWidth, true, true);
    displayAIMbufferoffset = offset - aimBuffer;
    incOffset();
  }


  void incOffset(void) {             //C47 JM
    if( (int32_t)stringWidthC47(aimBuffer + displayAIMbufferoffset, combinationFonts ,nocompress, true, true) -
        (int32_t)stringWidthC47(aimBuffer + T_cursorPos, combinationFonts ,nocompress, true, true)
        > SCREEN_WIDTH * multiEdLines - 45
        ) {
      displayAIMbufferoffset = stringNextGlyph(aimBuffer, displayAIMbufferoffset);
    }
  }


#define blockForcedRefreshes false

  static bool_t _force_refresh(uint8_t mode) {
    #if defined(ANALYSE_REFRESH) && defined(PC_BUILD)
      printf("# force = %i",mode == force);
    #endif //ANALYSE_REFRESH
    uint16_t now = 0;
    bool_t itIsTime = false;
    if(mode != force || blockForcedRefreshes) {
      now = (uint16_t)(getUptimeMs() >> 4);           // ms/16
      itIsTime = ((now >> 6) & 0x0001) == secTick1;     // ms/1024, that is every second, flips secTick1
      if(itIsTime) {
        secTick1 = !secTick1;
      }
    }

    if(((mode == force && !blockForcedRefreshes) || itIsTime) && getSystemFlag(FLAG_MONIT)) {  //Restrict refresh to once per period. Use this minimally, due to extreme slow response.
      #if defined(ANALYSE_REFRESH) && defined(PC_BUILD)
        printf("-\n");
      #endif //ANALYSE_REFRESH
      return true;
    }

    else {
      #if defined(ANALYSE_REFRESH) && defined(PC_BUILD)
        printf("not updated =%i %i\n", now, itIsTime);
      #endif //ANALYSE_REFRESH
    }
    return false;
  }


  void force_refresh(uint8_t mode) {
                                        #if defined(ANALYSE_REFRESH)
                                          print_caller(NULL);
                                        #endif //ANALYSE_REFRESH
    if(_force_refresh(mode)) {
      _lcdRefresh();
    }
    return;
  }

  void force_SBrefresh(uint8_t mode) {
                                        #if defined(ANALYSE_REFRESH)
                                          print_caller(NULL);
                                        #endif //ANALYSE_REFRESH
    if(_force_refresh(mode)) {
      _lcdSBRefresh();
    }
    return;
  }


  static void force_Registerrefresh(calcRegister_t regist, bool_t clearTop, bool_t clearBottom) {
    if(REGISTER_X <= regist && regist <= REGISTER_T) {
      uint32_t yStart = Y_POSITION_OF_REGISTER_X_LINE - REGISTER_LINE_HEIGHT*(regist - REGISTER_X);
      uint32_t height = 32;

      if(clearTop) {
        yStart -= 4;
        height += 4;
      }

      if(clearBottom) {
        height += 4;
        if(regist == REGISTER_X) {
          height += 3;
        }
      }

      _lcdBandRefresh(yStart, height);
    }
  }


  static bool_t _printHalfSecUpdate_Integer(uint8_t mode, char *txt, int32_t loop, bool_t clearZ, bool_t clearT, bool_t disp) {
    char tmps[100];
    bool_t ret_value = false;

    if((mode != timed && !blockForcedRefreshes) || ((((uint16_t)(getUptimeMs()) >> 10) & 0x0001)) == halfSecTick3) { //1.024 second refresh interval
      halfSecTick3 = !halfSecTick3;
      ret_value = true;
      #if defined(DMCP_BUILD)
        dmcpResetAutoOff();
      #endif //DMCP_BUILD

      //refreshScreen();   //to update stack
      if(clearT && !blockMonitoring) {
        clearRegisterLine(REGISTER_T, true, true);
      }
      if(clearZ && !blockMonitoring && mode > force) {   //force = 1
        clearRegisterLine(REGISTER_Z, true, true);
      }

      //lcd_refresh();
      fnTimerStart(TO_KB_ACTV, TO_KB_ACTV, TO_KB_ACTV_MEDIUM); //PROGRAM_KB_ACTV
      if(disp && !blockMonitoring) {
        sprintf(tmps, "%s %" PRIi32 "  ", txt, loop);
        showString(tmps, &standardFont, 20, /*145-7*/ Y_POSITION_OF_REGISTER_T_LINE + mode * 20, vmNormal, false, false);  //note: displays info 1 line down, if "force" parameter is set
      }

      _lcdRefresh();

    }
    return ret_value;
  }

  bool_t progressHalfSecUpdate_Integer(uint8_t mode, char *txt, int32_t loop, bool_t clearZ, bool_t clearT, bool_t disp) {//further optimisation, not to even set up the 100 byte array or call getUptimeMs if progress monitor is not selected
    if(!getSystemFlag(FLAG_MONIT)) {
      return false;
    }
    return _printHalfSecUpdate_Integer(mode, txt, loop, clearZ, clearT, disp);
  }


  bool_t checkHalfSec(void) {
    if(!getSystemFlag(FLAG_MONIT)) {
      return false;
    }
    if(((((uint16_t)(getUptimeMs()) >> 10) & 0x0001)) == halfSecTick2) { //1.024 second refresh interval
      halfSecTick2 = !halfSecTick2;
      #if defined(DMCP_BUILD)
        dmcpResetAutoOff();
      #endif //DMCP_BUILD
      return true;
    }
    return false;
  }



  bool_t monitorExit(int32_t *loop, char* str) {
    (*loop)++;
    if(checkHalfSec()) {
      if(progressHalfSecUpdate_Integer(timed, str, *loop, halfSec_clearZ, halfSec_clearT, halfSec_disp)) { //timed
        //no additional 1 sec monitoring here
      }
    }
    if(exitKeyWaiting()) {
      progressHalfSecUpdate_Integer(force+1, "Interrupted: ",*loop, halfSec_clearZ, halfSec_clearT, halfSec_disp);
      return true;
    }
    return false;
  }


  void hideCursor(void) {
    if(cursorEnabled) {
      if(cursorFont == &standardFont) {
        lcd_fill_rect(xCursor, yCursor + 10,  6,  6, LCD_SET_VALUE);
      }
      else {
        if(checkHP) {
          uint32_t ccol, crow;
          getGlyphBounds(STD_CURSOR, 0, cursorFont, &ccol, &crow);
          lcd_fill_rect(xCursor, yCursor - checkHPoffset, ccol, crow, LCD_SET_VALUE);
          //lcd_fill_rect(xCursor, yCursor + 15 - checkHPoffset, 13*2, 13*2, LCD_SET_VALUE);
        }
        else {
          lcd_fill_rect(xCursor, yCursor + 15, 13, 13, LCD_SET_VALUE);
        }
      }
    }
  }


  static void stats_param_display(const char *name, calcRegister_t reg, char *prefix, char *tmpString, calcRegister_t rowReg) {
    int16_t prefixWidth;
    char regS[16], *p;
    real_t t;
    real34_t u;
    uint32_t angleMode;

    if(name == NULL || !(rowReg == REGISTER_Y || rowReg == REGISTER_Z || rowReg == REGISTER_T)) {
      return;
    }
    clearRegisterLine(rowReg, true, true);

    if(reg == RESERVED_VARIABLE_UEST) {
      sprintf(prefix, "Upper =");
      strcpy(regS,name);
    } else if (reg == RESERVED_VARIABLE_LEST) {
      sprintf(prefix, "Lower =");
      strcpy(regS,name);
    } else {
        strcpy(regS, "Reg_");
        regS[3] = letteredRegisterName(reg);
        sprintf(prefix, "= %s =", name);
    }
    showString(regS, &standardFont, 19, Y_POSITION_OF_REGISTER_X_LINE - REGISTER_LINE_HEIGHT*(rowReg - REGISTER_X) + 6, vmNormal, true, true);
    prefixWidth = showString(prefix, &standardFont, 19 + (17+28), Y_POSITION_OF_REGISTER_X_LINE - REGISTER_LINE_HEIGHT*(rowReg - REGISTER_X) + 6, vmNormal, true, true);

    if(getRegisterAsRealQuiet(reg, &t)) {
      angleMode = getRegisterDataType(reg) == dtReal34 ? getRegisterAngularMode(reg) : amNone;
      realToReal34(&t, &u);
      real34ToDisplayString(&u, angleMode, tmpString, &numericFont, SCREEN_WIDTH - prefixWidth, NUMBER_OF_DISPLAY_DIGITS, LIMITEXP, FRONTSPACE, NOIRFRAC);
      p = tmpString;
    }
    else {
      p = "invalid";
    }

    showString(p, &numericFont, SCREEN_WIDTH - stringWidth(p, &numericFont, false, true), Y_POSITION_OF_REGISTER_X_LINE - REGISTER_LINE_HEIGHT*(rowReg - REGISTER_X), vmNormal, false, true);
  }

  #define PRIORITY_itemCatalogName true
  #define PRIORITY_itemSoftmenuName false
  const char* pickValidItemFromItems(int16_t item, bool_t priority) {
    bool_t takeCat = false;
    if (priority == PRIORITY_itemCatalogName) {
      if ((indexOfItems[abs(item)].itemCatalogName)[0] != 0) {
        takeCat = true;
      }
    } else { // PRIORITY_itemSoftmenuName
      if ((indexOfItems[abs(item)].itemSoftmenuName)[0] == 0) {
        takeCat = true;
      }
    }
    if (takeCat) {
      return indexOfItems[abs(item)].itemCatalogName;
    } else {
      return indexOfItems[abs(item)].itemSoftmenuName;
    }
  }

  bool_t showingProbMenu(void) {
    int cur = -softmenu[softmenuStack[0].softmenuId].menuItem;

    return (cur >= PROBMENUSTART1 && cur <= PROBMENUEND1) ||
           (cur >= PROBMENUSTART2 && cur <= PROBMENUEND2);
  }

//#define DEBUG_SHOWNAME
  void showFunctionName(int16_t itm, int16_t delayInMs, const char *arg) {
    int16_t item = (int16_t)itm;
    //printf("---Function par:%4u %4u-- converted %4u--arg:|%s|-=-\n", itm, (int16_t)itm, item, arg );
    char functionName[64];
    char padding[25];          //(2+0)+(15+0)+(7+0)+1 = 25
    functionName[0] = 0;
    showFunctionNameArg = NULL;

    #if defined(DEBUG_SHOWNAME)
      if(item < LAST_ITEM && (item == ITM_XEQ || item != ITM_RCL)) {
        stringCopy(functionName + stringByteLength(functionName), pickValidItemFromItems(item, PRIORITY_itemCatalogName));
        stringCopy(functionName + stringByteLength(functionName), ":");
      }
      if(item < LAST_ITEM && (item == ITM_RCL || item != ITM_XEQ)) {
        stringCopy(functionName + stringByteLength(functionName), pickValidItemFromItems(item, PRIORITY_itemSoftmenuName));
        stringCopy(functionName + stringByteLength(functionName), ":");
      }
      if(arg != NULL) {
        stringCopy(functionName + stringByteLength(functionName), arg);
        stringCopy(functionName + stringByteLength(functionName), ":");
      }
      if(dynamicMenuItem > -1) {
        stringCopy(functionName + stringByteLength(functionName), dynmenuGetLabel(dynamicMenuItem));
      }

    #else //DEBUG_SHOWNAME
      if((item == ITM_XEQ) || (item == ITM_RCL)) {
        if(arg != NULL) stringCopy(functionName, arg);
        showFunctionNameArg = (char *)arg;                          // Needed when executing a program or a variable from a long pressed key
        if(functionName[0]==0) {
          stringCopy(functionName, indexOfItems[abs(item)].itemCatalogName);
        }
      }
      else if(item == -MNU_DYNAMIC) {
        if(arg != NULL) stringCopy(functionName, arg);
        showFunctionNameArg = (char *)arg;                        // Needed when executing a user menu from a long pressed key
      }
      else if(item >= FIRST_CONSTANT && item <= LAST_CONSTANT) {
        stringCopy(functionName, pickValidItemFromItems(item, PRIORITY_itemSoftmenuName));
      }
      else if(item < LAST_ITEM && item != MNU_DYNAMIC) {
        stringCopy(functionName, pickValidItemFromItems(item, PRIORITY_itemCatalogName));
      }
      else if(dynamicMenuItem > -1) {
        stringCopy(functionName, dynmenuGetLabel(dynamicMenuItem));
      }
    #endif //DEBUG_SHOWNAME
      //printf("---|%s|---\n", functionName);

    showFunctionNameItem = item;
    showFunctionNameCounter = delayInMs;


    if(tam.alpha && ((item == ITM_BACKSPACE) || (item == ITM_T_LEFT_ARROW) || (item == ITM_T_RIGHT_ARROW))) {               // For smooth display in tam.alpha
      return;
    }

    if(functionName[0] != 0)
    {
      bool_t overLapPossible = (calcMode == CM_PEM);
      padding[0] = 0;
      if(overLapPossible) {
        stringCopy(padding, " ");
      }
      #define typWidth 120 //stringWidth(" WWWWWW     ", &standardFont, true, true);
      stringCopy(padding + stringByteLength(padding), functionName);
      stringCopy(padding + stringByteLength(padding), "       ");
      if(calcMode == CM_ASSIGN || ((PROBMENU || XXFNMODEACTIVE || stringWidth(padding, &standardFont, true, true) + 1 /*JM 20*/ + lineTWidth > SCREEN_WIDTH) && calcMode != CM_PEM)) {
        clearRegisterLine(REGISTER_T, true, false);
      }
      // Clear SHIFT f and SHIFT g in case they were present (otherwise they will be obscured by the function name)
      clearShiftState();
      int xx = showString(padding, &standardFont, funcNameOffset_x, Y_POSITION_OF_REGISTER_T_LINE + 6, vmNormal, true, true);      //JM
      if(overLapPossible) {
        plotrect(funcNameOffset_x, Y_POSITION_OF_REGISTER_T_LINE + 6, max(xx,funcNameOffset_x + typWidth), Y_POSITION_OF_REGISTER_T_LINE + 6 + STANDARD_FONT_HEIGHT - 1);
        if(xx < funcNameOffset_x + typWidth) lcd_fill_rect(xx, Y_POSITION_OF_REGISTER_T_LINE + 6 + 1, funcNameOffset_x + typWidth - xx, STANDARD_FONT_HEIGHT - 2, LCD_SET_VALUE);
      }
    }
    if(temporaryInformation != TI_NO_INFO) {
      temporaryInformation = TI_NO_INFO;
      lastErrorCode = 0;
      screenUpdatingMode &= ~SCRUPD_MANUAL_STACK;
    }
  }


  void hideFunctionName(void) {
    if(tmpString[0] != 0 || calcMode!=CM_AIM) {
      if(calcMode != CM_PEM) {
        if(!tam.alpha || (showFunctionNameItem != ITM_BACKSPACE &&               // For smooth display in tam.alpha
                          showFunctionNameItem != ITM_T_LEFT_ARROW &&
                          showFunctionNameItem != ITM_T_RIGHT_ARROW &&
                          showFunctionNameItem != ITM_NULL)) {
          refreshRegisterLineRestoreT();                                                //JM DO NOT CHANGE BACK TO CLEARING ONLY A SHORT PIECE. CHANGED IN TWEAKED AS WELL>
          force_Registerrefresh(REGISTER_T, true, true);
        }
      } else {
        _refreshPemScreen();
        //force reset is done at _refreshPemScreen
      }
// this seems to cause an undue delay for large matrices, and I cannot see why it should be reprinted (and the cached heights updated
//      if(getRegisterDataType(REGISTER_X) == dtReal34Matrix || getRegisterDataType(REGISTER_X) == dtReal34Matrix) {
//        refreshRegisterLine(REGISTER_X);
//      }
    }
    showFunctionNameItem = 0;
    showFunctionNameCounter = 0;
  }


  void clearRect(uint32_t g_line_x, uint32_t g_line_y) {
    uint32_t fcol, frow;
    getGlyphBounds(" ", 0, &standardFont, &fcol, &frow);
    lcd_fill_rect((uint32_t) g_line_x, (uint32_t) g_line_y, SCREEN_WIDTH-g_line_x-1, frow, LCD_SET_VALUE);
  }


  void clearRegisterLine(calcRegister_t regist, bool_t clearTop, bool_t clearBottom) {
    if(REGISTER_X <= regist && regist <= REGISTER_T) {
      uint32_t yStart = Y_POSITION_OF_REGISTER_X_LINE - REGISTER_LINE_HEIGHT*(regist - REGISTER_X);
      uint32_t height = 32;

      if(clearTop) {
        yStart -= 4;
        height += 4;
      }

      if(clearBottom) {
        height += 4;
        if(regist == REGISTER_X) {
          height += 3;
        }
      }

      lcd_fill_rect(0, yStart, SCREEN_WIDTH, height, LCD_SET_VALUE);
    }
  }


  static void do_viewRegName(calcRegister_t regist,  char *prefix, int16_t *prefixWidth, char* endChar) { //using "=" for VIEW
    //printf("========================== %i %s regist=%i %s %i\n", lastFuncNo(), lastFuncCatalogName(), regist, prefix, *prefixWidth);
    if(lastFuncNo() == ITM_AVIEW || lastFuncNo() == ITM_PROMPT) {
      if(isShiftOffset) {
        strcpy(prefix, "  ");
        *prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
      }
      else {
        prefix[0] = 0;
        *prefixWidth = 1;
      }
      return;
    }

    if(regist < REGISTER_X) {
      sprintf(prefix, "%sR%02" PRIu16 STD_SPACE_4_PER_EM "%s" STD_SPACE_4_PER_EM, funcNameOffset_str, regist, endChar);
    }
    else if(regist <= LAST_SPARE_REGISTER) {
      sprintf(prefix, "%s%c" STD_SPACE_4_PER_EM "%s" STD_SPACE_4_PER_EM, funcNameOffset_str, letteredRegisterName(regist), endChar);
    }
    else if(regist >= FIRST_LOCAL_REGISTER && regist <= LAST_LOCAL_REGISTER) {
      sprintf(prefix, "%sR.%02" PRIu16 STD_SPACE_4_PER_EM "%s" STD_SPACE_4_PER_EM, funcNameOffset_str, (uint16_t)(regist - FIRST_LOCAL_REGISTER), endChar);
    }
    else if(FIRST_NAMED_VARIABLE <= regist && regist <= LAST_NAMED_VARIABLE) {
      if(isShiftOffset) {
        strcpy(prefix, "  ");
      }
      strcpy(prefix + (isShiftOffset ? 2 : 0), STD_LEFT_SINGLE_QUOTE);
      memcpy(prefix + (isShiftOffset ? 4 : 2), allNamedVariables[regist - FIRST_NAMED_VARIABLE].variableName + 1, allNamedVariables[regist - FIRST_NAMED_VARIABLE].variableName[0]);
      sprintf(prefix + (isShiftOffset ? 4 : 2) + allNamedVariables[regist - FIRST_NAMED_VARIABLE].variableName[0], STD_RIGHT_SINGLE_QUOTE STD_SPACE_4_PER_EM "%s" STD_SPACE_4_PER_EM, endChar);
    }
    else if(FIRST_RESERVED_VARIABLE <= regist && regist <= LAST_RESERVED_VARIABLE) {
      if(isShiftOffset) {
        strcpy(prefix, "  ");
      }
      strcpy(prefix + (isShiftOffset ? 2 : 0), STD_LEFT_SINGLE_QUOTE);
      memcpy(prefix + (isShiftOffset ? 4 : 2), allReservedVariables[regist - FIRST_RESERVED_VARIABLE].reservedVariableName + 1, allReservedVariables[regist - FIRST_RESERVED_VARIABLE].reservedVariableName[0]);
      sprintf(prefix + (isShiftOffset ? 4 : 2) + allReservedVariables[regist - FIRST_RESERVED_VARIABLE].reservedVariableName[0], STD_RIGHT_SINGLE_QUOTE STD_SPACE_4_PER_EM "%s" STD_SPACE_4_PER_EM, endChar);
    }
    else {
      sprintf(prefix, "?" STD_SPACE_4_PER_EM "%s" STD_SPACE_4_PER_EM, endChar);
    }
    *prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
  }

  static void viewRegName(char *prefix, int16_t *prefixWidth) { //using "=" for VIEW
    do_viewRegName(currentViewRegister, prefix, prefixWidth, "=");
  }

  void viewRegName2(char *prefix, int16_t *prefixWidth) { //using ":" for SHOW
    do_viewRegName(showRegis, prefix, prefixWidth, ":" );
  }

  static void nameRegis(calcRegister_t regist, char *prefix) {
    int16_t prefixWidth;
    do_viewRegName(regist, prefix, &prefixWidth, "");
  }

  static void viewStoRcl(char *prefix, int16_t *prefixWidth) {
    do_viewRegName(lastSTORCL(), prefix, prefixWidth, ":");
    if(prefix[0]=='?') {
      prefix[0] = 0;
      prefixWidth = 0;
    }
  }



//create substrings in tmpString by replacing the separator character with a [0]
void createSubstrings(uint8_t number) {
  if(!(lastFuncNo() == ITM_AVIEW || lastFuncNo() == ITM_PROMPT)) {
    return;
  }
  //printf("\n\ncreateSubstrings(%i): tmpString: %s\n",number, tmpString);
  uint16_t nn = 0;
  uint16_t counter = 0;
  uint16_t mm = stringByteLength(tmpString);
  while(nn <= mm){
    //printf("#%u tmpString[nn]=%u",nn,(uint8_t)(tmpString[nn]));
    if(tmpString[nn] == STD_CR[0] && tmpString[nn+1] == STD_CR[1]) {
      tmpString[nn++] = 32;
      tmpString[nn  ] = 0;
      //printf("ZERO\n");
      if(++counter == number) break;
    }
    else if(tmpString[nn] & 0x80) {
      nn++;
    }
    nn++;
    //printf("\nSSS %u @ %u ; ",counter, nn);
    //printStringToConsole(tmpString,">>","<<\n");
  }
  //printf("TTT %u nn=%u\n",counter,nn);
  tmpString[  nn] = 0;
  while(counter < number && number <= 4) {   //allow up to 5 sub-strings
    tmpString[++nn] = 0;
    counter++;
  }
  //printStringToConsole(tmpString,"String: ","\n");
  //printStringToConsole((char *)getNthString((uint8_t *)tmpString,0),"createSubstrings: substring 0: ","\n");
  //printStringToConsole((char *)getNthString((uint8_t *)tmpString,1),"createSubstrings: substring 1: ","\n");
  //printStringToConsole((char *)getNthString((uint8_t *)tmpString,2),"createSubstrings: substring 2: ","\n");
  //printStringToConsole((char *)getNthString((uint8_t *)tmpString,3),"createSubstrings: substring 3: ","\n");
  return;
}




  static void userTI(int16_t viewRegister, int16_t refreshRegist, char *prefix, int16_t *prefixWidth) {
    if(!(lastFuncNo() == ITM_AVIEW || lastFuncNo() == ITM_PROMPT)) {
      return;
    }
    //printf("TTTT:refreshRegist:%i %i string:%s type:%i\n",viewRegister, refreshRegist, tmpString, getRegisterDataType(viewRegister));
    if(temporaryInformation == TI_VIEW_REGISTER && getRegisterDataType(viewRegister) == dtString) {
      //tmpString[0]=0;
      xcopy(tmpString, REGISTER_STRING_DATA(viewRegister), stringByteLength(REGISTER_STRING_DATA(viewRegister)) + 1);
      createSubstrings(4);
      if(refreshRegist == REGISTER_T) {
        char *string1 = "";
        string1 = (char *)getNthString((uint8_t *)tmpString,0);
        xcopy(tmpString + (isShiftOffset ? 2 : 0), string1, stringByteLength(string1) + 1);
        if(isShiftOffset) {
          tmpString[0] = 32;
          tmpString[1] = 32;
        }
        //printStringToConsole(tmpString,"--userTI substring 0: ","\n");
      }
      else if(refreshRegist == REGISTER_X) {
        char *string1 = "";
        string1 = (char *)getNthString((uint8_t *)tmpString,1);
        xcopy(prefix, string1, stringByteLength(string1) + 1);
        //printStringToConsole(prefix,"--userTI substring 1: ","\n");
        *prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
      }
      else if(refreshRegist == REGISTER_Y) {
        char *string1 = "";
        string1 = (char *)getNthString((uint8_t *)tmpString,2);
        xcopy(prefix, string1, stringByteLength(string1) + 1);
        //printStringToConsole(prefix,"--userTI substring 2: ","\n");
        *prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
      }
      else if(refreshRegist == REGISTER_Z) {
        char *string1 = "";
        string1 = (char *)getNthString((uint8_t *)tmpString,3);
        xcopy(prefix, string1, stringByteLength(string1) + 1);
        //printStringToConsole(prefix,"--userTI substring 3: ","\n");
        *prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
      }
    }
  }


  static void elecTI(int16_t regist, char *prefix, int16_t *prefixWidth) {
    if(temporaryInformation == TI_ABC) {
      if(regist == REGISTER_X) {
        strcpy(prefix, "c" STD_SPACE_FIGURE ":");
        *prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
      }
      else if(regist == REGISTER_Y) {
        strcpy(prefix, "b" STD_SPACE_FIGURE ":");
        *prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
      }
      else if(regist == REGISTER_Z) {
        strcpy(prefix, "a" STD_SPACE_FIGURE ":");
        *prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
      }
    }
    else if(temporaryInformation == TI_ABBCCA) {
      if(regist == REGISTER_X) {
        strcpy(prefix, "bc" STD_SPACE_FIGURE ":");
        *prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
      }
      else if(regist == REGISTER_Y) {
        strcpy(prefix, "ab" STD_SPACE_FIGURE ":");
        *prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
      }
      else if(regist == REGISTER_Z) {
        strcpy(prefix, "ca" STD_SPACE_FIGURE ":");
        *prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
      }
    }
    else if(temporaryInformation == TI_012) {
      if(regist == REGISTER_X) {
        strcpy(prefix, "sym2" STD_SPACE_FIGURE ":");
        *prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
      }
      else if(regist == REGISTER_Y) {
        strcpy(prefix, "sym1" STD_SPACE_FIGURE ":");
        *prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
      }
      else if(regist == REGISTER_Z) {
        strcpy(prefix, "sym0" STD_SPACE_FIGURE ":");
        *prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
      }
    }
  }



  static void inputRegName(char *prefix, int16_t *prefixWidth) {
    if((currentInputVariable & 0x3fff) < REGISTER_X) {
      sprintf(prefix, " R%02" PRIu16 "?", (uint16_t)(currentInputVariable & 0x3fff));
    }
    else if((currentInputVariable & 0x3fff) <= LAST_SPARE_REGISTER) {
      sprintf(prefix, " %c?", letteredRegisterName(currentInputVariable & 0x3fff));
    }
    else if(((currentInputVariable & 0x3fff) >= FIRST_LOCAL_REGISTER) && (currentInputVariable & 0x3fff) <= LAST_LOCAL_REGISTER) {
      sprintf(prefix, " R.%02" PRIu16 "?", (uint16_t)((currentInputVariable & 0x3fff) - FIRST_LOCAL_REGISTER));
    }
    else if(FIRST_NAMED_VARIABLE <= (currentInputVariable & 0x3fff) && (currentInputVariable & 0x3fff) <= LAST_NAMED_VARIABLE) {
      memcpy(prefix, allNamedVariables[(currentInputVariable & 0x3fff) - FIRST_NAMED_VARIABLE].variableName + 1, allNamedVariables[(currentInputVariable & 0x3fff) - FIRST_NAMED_VARIABLE].variableName[0]);
      strcpy(prefix + allNamedVariables[(currentInputVariable & 0x3fff) - FIRST_NAMED_VARIABLE].variableName[0], "?");
    }
    else if(FIRST_RESERVED_VARIABLE <= (currentInputVariable & 0x3fff) && (currentInputVariable & 0x3fff) <= LAST_RESERVED_VARIABLE) {
      memcpy(prefix, allReservedVariables[(currentInputVariable & 0x3fff) - FIRST_RESERVED_VARIABLE].reservedVariableName + 1, allReservedVariables[(currentInputVariable & 0x3fff) - FIRST_RESERVED_VARIABLE].reservedVariableName[0]);
      strcpy(prefix + allReservedVariables[(currentInputVariable & 0x3fff) - FIRST_RESERVED_VARIABLE].reservedVariableName[0], "?");
    }
    else {
      sprintf(prefix, "??");
    }
    *prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
  }


  static void _fnShowRecallTI(char * prefix, int16_t *prefixWidth) {
    viewRegName2(prefix + sprintf(prefix, "SHOW RCL"), prefixWidth);
    *prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
    temporaryInformation = TI_NO_INFO;
    screenUpdatingMode |= SCRUPD_SKIP_STACK_ONE_TIME;
  }


  void updateMatrixHeightCache(void) {
    int16_t prefixWidth = 0;
    char prefix[200];

    cachedDisplayStack = 4;

    if(getRegisterDataType(REGISTER_X) == dtReal34Matrix || (calcMode == CM_MIM && getRegisterDataType(matrixIndex) == dtReal34Matrix)) {
      real34Matrix_t matrix;

      if(temporaryInformation == TI_VIEW_REGISTER) {
        viewRegName(prefix, &prefixWidth);
      }
      if(temporaryInformation == TI_NO_INFO && currentInputVariable != INVALID_VARIABLE) {
        inputRegName(prefix, &prefixWidth);
      }
      if(calcMode == CM_MIM) {
        matrix = openMatrixMIMPointer.realMatrix;
      }
      else {
        linkToRealMatrixRegister(REGISTER_X, &matrix);
      }
      const uint16_t rows = matrix.header.matrixRows;
      const uint16_t cols = matrix.header.matrixColumns;
      bool_t smallFont = (rows >= 5);
      int16_t dummyVal[MATRIX_MAX_COLUMNS * (MATRIX_MAX_ROWS + 1) + 1] = {};

      bool_t allElementsInColAreIntegers[MATRIX_MAX_COLUMNS] = {};
      for(int j = 0; j < min(cols,MATRIX_MAX_COLUMNS); j++) {
        allElementsInColAreIntegers[j]=true;
        for(int i = 0; i < rows; i++) {
          if(!real34IsAnInteger(&matrix.matrixElements[i*cols+j])) {
            allElementsInColAreIntegers[j]=false;
            break;
          }
        }
      }

      const int16_t mtxWidth = getRealMatrixColumnWidths(&matrix, prefixWidth, &numericFont, dummyVal, dummyVal + MATRIX_MAX_COLUMNS, dummyVal + (MATRIX_MAX_ROWS + 1) * MATRIX_MAX_COLUMNS, cols > MATRIX_MAX_COLUMNS ? MATRIX_MAX_COLUMNS : cols, allElementsInColAreIntegers);
      if(abs(mtxWidth) > MATRIX_LINE_WIDTH) {
        smallFont = true;
      }
      if(rows == 2 && cols > 1 && !smallFont) {
        cachedDisplayStack = 3;
      }
      if(rows == 3 && cols > 1) {
        cachedDisplayStack = smallFont ? 3 : 2;
      }
      if(rows == 4 && cols > 1) {
        cachedDisplayStack = smallFont ? 2 : 1;
      }
      if(rows >= 5 && cols > 1) {
        cachedDisplayStack = 2;
      }
      if(calcMode == CM_MIM) {
        cachedDisplayStack -= 2;
      }
      if(cachedDisplayStack > 4) { // in case of overflow
        cachedDisplayStack = 0;
      }
    }
    else if(getRegisterDataType(REGISTER_X) == dtComplex34Matrix || (calcMode == CM_MIM && getRegisterDataType(matrixIndex) == dtComplex34Matrix)) {
      complex34Matrix_t matrix;
      if(temporaryInformation == TI_VIEW_REGISTER) {
        viewRegName(prefix, &prefixWidth);
      }
      if(temporaryInformation == TI_NO_INFO && currentInputVariable != INVALID_VARIABLE) {
        inputRegName(prefix, &prefixWidth);
      }
      if(calcMode == CM_MIM) {
        matrix = openMatrixMIMPointer.complexMatrix;
      }
      else {
        linkToComplexMatrixRegister(REGISTER_X, &matrix);
      }
      const uint16_t rows = matrix.header.matrixRows;
      const uint16_t cols = matrix.header.matrixColumns;
      bool_t smallFont = (rows >= 5);
      int16_t dummyVal[MATRIX_MAX_COLUMNS * (MATRIX_MAX_ROWS * 2 + 3) + 1] = {};
      const int16_t mtxWidth = getComplexMatrixColumnWidths(&matrix, prefixWidth, &numericFont, dummyVal, dummyVal + MATRIX_MAX_COLUMNS, dummyVal + MATRIX_MAX_COLUMNS * 2, dummyVal + MATRIX_MAX_COLUMNS * 3, dummyVal + MATRIX_MAX_COLUMNS * (MATRIX_MAX_ROWS + 3), dummyVal + MATRIX_MAX_COLUMNS * (MATRIX_MAX_ROWS * 2 + 3), cols > MATRIX_MAX_COLUMNS ? MATRIX_MAX_COLUMNS : cols,  getComplexRegisterAngularMode(REGISTER_X), getComplexRegisterPolarMode(REGISTER_X) == amPolar);
      if(mtxWidth > MATRIX_LINE_WIDTH) {
        smallFont = true;
      }
      if(rows == 2 && cols > 1 && !smallFont) {
        cachedDisplayStack = 3;
      }
      if(rows == 3 && cols > 1) {
        cachedDisplayStack = smallFont ? 3 : 2;
      }
      if(rows == 4 && cols > 1) {
        cachedDisplayStack = smallFont ? 2 : 1;
      }
      if(rows >= 5 && cols > 1) {
        cachedDisplayStack = 2;
      }
      if(calcMode == CM_MIM) {
        cachedDisplayStack -= 2;
      }
      if(cachedDisplayStack > 4) { // in case of overflow
        cachedDisplayStack = 0;
      }
    }

    if(calcMode == CM_MIM && matrixIndex == REGISTER_X) {
      cachedDisplayStack += 1;
    }
  }


  void displayTemporaryInformationOnX(char *prefix) {
    int16_t       w, prefixWidth;
    uint8_t       savedTempInformation;

    prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
    savedTempInformation = temporaryInformation;
    temporaryInformation = TI_NO_INFO;
    refreshRegisterLine(REGISTER_T);
    refreshRegisterLine(REGISTER_Z);
    refreshRegisterLine(REGISTER_Y);
    refreshRegisterLine(REGISTER_X);
    temporaryInformation = savedTempInformation;

    if(getRegisterDataType(REGISTER_X) == dtReal34) {
      clearRegisterLine(REGISTER_X, true, true);
      if(getSystemFlag(FLAG_FRACT)) {
        fractionToDisplayString(REGISTER_X, tmpString);
      }
      else {
        real34ToDisplayString(REGISTER_REAL34_DATA(REGISTER_X), getRegisterAngularMode(REGISTER_X), tmpString, &numericFont, SCREEN_WIDTH - prefixWidth, NUMBER_OF_DISPLAY_DIGITS, LIMITEXP, FRONTSPACE, LIMITIRFRAC);
      }
      w = stringWidth(tmpString, &numericFont, false, true);
      showString(prefix, &standardFont, 1, Y_POSITION_OF_REGISTER_X_LINE + 6, vmNormal, true, true);
      showString(tmpString, &numericFont, SCREEN_WIDTH - w, Y_POSITION_OF_REGISTER_X_LINE, vmNormal, false, true);
    }
    else if(getRegisterDataType(REGISTER_X) == dtComplex34) {
      clearRegisterLine(REGISTER_X, true, true);
      complex34ToDisplayString(REGISTER_COMPLEX34_DATA(REGISTER_X), tmpString, &numericFont, SCREEN_WIDTH - prefixWidth, NUMBER_OF_DISPLAY_DIGITS, LIMITEXP, FRONTSPACE, LIMITIRFRAC, getComplexRegisterAngularMode(REGISTER_X),  getComplexRegisterPolarMode(REGISTER_X) == amPolar);
      w = stringWidth(tmpString, &numericFont, false, true);
      showString(prefix, &standardFont, 1, Y_POSITION_OF_REGISTER_X_LINE + 6, vmNormal, true, true);
      showString(tmpString, &numericFont, SCREEN_WIDTH - w, Y_POSITION_OF_REGISTER_X_LINE, vmNormal, false, true);
    }
    else if(getRegisterDataType(REGISTER_X) == dtLongInteger) {
      clearRegisterLine(REGISTER_X, true, true);
      longIntegerRegisterToDisplayString(REGISTER_X, tmpString, TMP_STR_LENGTH, SCREEN_WIDTH - prefixWidth, 50, true);
      w = stringWidth(tmpString, &numericFont, false, true);
      showString(prefix, &standardFont, 1, Y_POSITION_OF_REGISTER_X_LINE + 6, vmNormal, true, true);
      if(w <= SCREEN_WIDTH-prefixWidth) {
        showString(tmpString, &numericFont, SCREEN_WIDTH - w, Y_POSITION_OF_REGISTER_X_LINE, vmNormal, false, true);
      }
      else {
        w = stringWidth(tmpString, &standardFont, false, true);
        if(w > SCREEN_WIDTH-prefixWidth) {
          //errorMoreInfo("Long integer representation too wide!\n%s", tmpString);
          strcpy(tmpString, "Long integer representation too wide!");
        }
        w = stringWidth(tmpString, &standardFont, false, true);
        showString(tmpString, &standardFont, SCREEN_WIDTH - w, Y_POSITION_OF_REGISTER_X_LINE + 6, vmNormal, false, true);
      }
    }
    else {
        showString(prefix, &standardFont, 1, Y_POSITION_OF_REGISTER_X_LINE + 6, vmNormal, true, true);
    }
  }


  void _displayIJ(calcRegister_t regist, char *prefix, int16_t *prefixWidth) {
    if(lastErrorCode != 0) {
      return;
    }
    real_t iir,jjr;

    int32_t iii, jji;
    bool_t bb;
    // uses errorMessage string to store old I & J
    iii = lastI;
    jji = lastJ;
    if(iii == 0xFFFF || jji == 0xFFFF) {
      bb = getRegisterAsRealQuiet(REGISTER_I, &iir) && getRegisterAsRealQuiet(REGISTER_J, &jjr);
      iii=realToUint32C47(&iir);
      jji=realToUint32C47(&jjr);
    } else {
      bb = true;
    }


    if(bb) {
      if(0 <= iii && iii < 200 && 0 <= jji && jji < 200) {
        prefix[0] = 0;
        *prefixWidth = 0;
        char tmp[16];
        nameRegis(matrixIndex, tmp);
// M[Ir Jc]=INDEXname[1, 2]=
        if(regist == REGISTER_X && temporaryInformation == TI_MIJEQ) {
          sprintf(prefix,STD_MU "[I" STD_SUB_r STD_SPACE_4_PER_EM "J" STD_SUB_c "]=%s[%u" STD_SPACE_3_PER_EM "%u]%s",tmp, (uint8_t)iii,(uint8_t)jji,(temporaryInformation == TI_MIJEQ ? "=" : ""));
        }
// M[Ir Jc]=INDEXname[1, 2]
        else if(regist == REGISTER_X && temporaryInformation == TI_MIJ) {
          sprintf(prefix,STD_MU "[I" STD_SUB_r STD_SPACE_4_PER_EM "J" STD_SUB_c "]=%s[%u" STD_SPACE_3_PER_EM "%u]",tmp, (uint8_t)iii,(uint8_t)jji);
        }
//R00 [Ir=1 Jc=1]: Jc=
        else if(regist == REGISTER_X && ((iii != 0 && temporaryInformation == TI_I) || (jji != 0 && temporaryInformation == TI_J))) {
         sprintf(prefix,"%s[I" STD_SUB_r "=%u" STD_SPACE_4_PER_EM "J" STD_SUB_c "=%u]%s",tmp, (uint8_t)iii,(uint8_t)jji, temporaryInformation == TI_I ? ": I" STD_SUB_r "=" : ": J" STD_SUB_c "=");
        }
//R00: Ir=
//R00: Jr=
        else if(iii != 0 && jji != 0) {
          if(regist == REGISTER_Y) {
            sprintf(prefix,STD_MU STD_SPACE_4_PER_EM "%s:I" STD_SUB_r "=",tmp);
          } else if(regist == REGISTER_X) {
            sprintf(prefix,STD_MU STD_SPACE_4_PER_EM "%s:J" STD_SUB_c "=",tmp);
          }
        }
        *prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
      }
    }
  }

  static void __displaySolver(calcRegister_t regist, char *prefix, int16_t *prefixWidth, int16_t no) {
      char noo[32];
      real_t t;
      uint16_t variableNo = currentSolverVariable - FIRST_RESERVED_VARIABLE;
      switch(no) {
        case  2: strcpy(noo,STD_SUB_p STD_SUB_r STD_SUB_e STD_SUB_v " =");
                   break;
        case  1: strcpy(noo," =" );
                   if(getRegisterAsRealQuiet(REGISTER_T, &t)) {
                     if(!realIsSpecial(&t) && realIsAnInteger(&t) && realToInt32C47(&t) == 200) {
                      strcat(noo," (conjugates)");
                     }
                   }
                   break;
        default: strcpy(noo," =" );
                   break;
      }
      if(currentSolverVariable >= FIRST_RESERVED_VARIABLE) {
        memcpy(prefix, allReservedVariables[variableNo].reservedVariableName + 1, allReservedVariables[variableNo].reservedVariableName[0]);
        strcpy(prefix + allReservedVariables[variableNo].reservedVariableName[0], noo);
        strcat(prefix + allReservedVariables[variableNo].reservedVariableName[0],&varDescr[variableNo].Desc[0]);
      }
      else {
        memcpy(prefix, allNamedVariables[currentSolverVariable - FIRST_NAMED_VARIABLE].variableName + 1, allNamedVariables[currentSolverVariable - FIRST_NAMED_VARIABLE].variableName[0]);
        strcpy(prefix + allNamedVariables[currentSolverVariable - FIRST_NAMED_VARIABLE].variableName[0], noo);
      }
      *prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
  }

  void _displaySolverOutput(calcRegister_t regist, char *prefix, int16_t *prefixWidth) {
    if(regist == REGISTER_X || regist == REGISTER_Y) {
      __displaySolver(regist, prefix, prefixWidth, regist - REGISTER_X +1);
    }
    else if(regist == REGISTER_Z) {
      strcpy(prefix, "Accuracy " STD_ALMOST_EQUAL);
      *prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
    }
    if(regist == REGISTER_T) {
      if(funcNameOffset_x == shiftOffset) {
        strcpy(prefix,"  ");
      } else {
        prefix[0]=0;
      }
      strcat(prefix, "Result Code =");
      *prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
    }
  }

  void _displaySolverInput(calcRegister_t regist, char *prefix, int16_t *prefixWidth) {
    if(regist == REGISTER_X) {
      __displaySolver(regist, prefix, prefixWidth, -1);
    }
  }

  #define noLine false
  static void _displaySigmaPlus(calcRegister_t regist, char *prefix, int16_t *prefixWidth, bool_t doLine) {
    int32_t w = realToInt32C47(SIGMA_N);
    if(regist == REGISTER_X) {
      sprintf(prefix, "%03" PRId32 " data point", w);
      if(w > 1) {
        stringCopy(prefix + stringByteLength(prefix), "s");
      }
      *prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
      if(doLine) {
        drawSinglePixelFullWidthLine(Y_POSITION_OF_REGISTER_Y_LINE - 2);
      }
    }
  }


  void _displayRegType(calcRegister_t regist, char *prefix, int16_t *prefixWidth) {
    if(regist == REGISTER_X) {
      real_t t;
      getRegisterAsRealQuiet(REGISTER_X, &t);
      int32_t ii = realToInt32C47(&t);
      realMultiply(&t, const_100, &t, &ctxtReal39);
      int32_t jj = realToInt32C47(&t) - 100*ii;
      char sss[30];
      sss[0]=0;
      switch (ii) {
        case 0 : strcpy(sss,"LongInteger"); break;
        case 1 : strcpy(sss,"Real"); break;
        case 2 : strcpy(sss,"Complex"); break;
        case 3 : strcpy(sss,"Time"); break;
        case 4 : strcpy(sss,"Date"); break;
        case 5 : strcpy(sss,"String"); break;
        case 6 : strcpy(sss,"RealMatrix"); break;
        case 7 : strcpy(sss,"ComplexMatrix");  break;
        case 8 : strcpy(sss,"ShortInteger"); break;
        case 9 : strcpy(sss,"Config"); break;
        default: break;
      }
      if(ii == 8) {
        strcat(sss,", base");
      } else {
        switch (jj) {
          case 10 : strcat(sss,", MUL" STD_pi); break;
          case 20 : strcat(sss,", DMS"); break;
          case 30 : strcat(sss,", Degree"); break;
          case 40 : strcat(sss,", Grad"); break;
          case 50 : strcat(sss,", Radian"); break;
          default:break;
        }
      }
      sprintf(prefix, "%s", sss);
      *prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
    }
  }



static bool_t displayTrueFalse(calcRegister_t regist) {
      char sss[10];
      #define clearOffset -2
      if(temporaryInformation == TI_FALSE) {
        if(clearOffset != 0) {
          sprintf(sss, "     ");
          showString(sss, &standardFont, 1, Y_POSITION_OF_TRUE_FALSE_LINE + 6 + clearOffset, vmNormal, true, true); //blank a little higher, 2 pixel
        }
        sprintf(sss, "false");
        showString(sss, &standardFont, 1, Y_POSITION_OF_TRUE_FALSE_LINE + 6, vmNormal, true, true);
        return true;
      }

      else if(temporaryInformation == TI_TRUE) {
        if(clearOffset != 0) {
          sprintf(sss, "    ");
          showString(sss, &standardFont, 1, Y_POSITION_OF_TRUE_FALSE_LINE + 6 + clearOffset, vmNormal, true, true); //blank a little higher, 2 pixel
        }
        sprintf(sss, "true");
        showString(sss, &standardFont, 1, Y_POSITION_OF_TRUE_FALSE_LINE + 6, vmNormal, true, true);
        return true;
      }
      return false;
  }



  void displayBaseMode(calcRegister_t regist) {
     #define BASEMODE_OFFSET_X 2
     #define BASEMODE_OFFSET_Y 2
     calcRegister_t Register_X = calcMode == CM_NIM ? REGISTER_Y : REGISTER_X;

     //JM SHOIDISP // use the top part of the screen for HEX and BIN    //JM vv SHOIDISP
     //DISP_TI=3    T=16    T=16    T=16
     //DISP_TI=2            Z=10    T=2
     //DISP_TI=1                    Z=10
     if(BASEMODEREGISTERX && regist == REGISTER_X && lastErrorCode == 0) {


       if(getRegisterDataType(REGISTER_X) == dtShortInteger) {

         //handle Reg Pos Y
         if(displayStack == 1 && calcMode != CM_NIM) {
           shortIntegerToDisplayString(Register_X, tmpString, true, dispBase == 0 ? 10 : 16);
           if(lastErrorCode == 0 && stringWidth(tmpString, fontForShortInteger, false, true) + stringWidth("  X: ", &standardFont, false, true) <= SCREEN_WIDTH) {
             showString("  X: ", &standardFont, 0, Y_POSITION_OF_REGISTER_X_LINE - REGISTER_LINE_HEIGHT*(REGISTER_Y - REGISTER_X) + (fontForShortInteger == &standardFont ? 6 : 0), vmNormal, false, true);
           }
           showString(tmpString, fontForShortInteger, SCREEN_WIDTH - stringWidth(tmpString, fontForShortInteger, false, true), Y_POSITION_OF_REGISTER_X_LINE - REGISTER_LINE_HEIGHT*(REGISTER_Y - REGISTER_X) + (fontForShortInteger == &standardFont ? 6 : 0), vmNormal, false, true);
         }


         //handle reg pos Z
         if((displayStack == 1 && calcMode != CM_NIM) || displayStack == 2){
           shortIntegerToDisplayString(Register_X, tmpString, true, displayStack == 1 ? 2 : (dispBase == 0 ? 10 : 16));
           if(lastErrorCode == 0 && stringWidth(tmpString, fontForShortInteger, false, true) + stringWidth("  X: ", &standardFont, false, true) <= SCREEN_WIDTH) {
             showString("  X: ", &standardFont, 0, Y_POSITION_OF_REGISTER_X_LINE - REGISTER_LINE_HEIGHT*(REGISTER_Z - REGISTER_X) + (fontForShortInteger == &standardFont ? 6 : 0), vmNormal, false, true);
           }
           showString(tmpString, fontForShortInteger, SCREEN_WIDTH - stringWidth(tmpString, fontForShortInteger, false, true), Y_POSITION_OF_REGISTER_X_LINE - REGISTER_LINE_HEIGHT*(REGISTER_Z - REGISTER_X) + (fontForShortInteger == &standardFont ? 6 : 0), vmNormal, false, true);
         }


         //handle reg pos T
         if((displayStack == 1 && calcMode != CM_NIM) || displayStack == 2 || displayStack == 3) {
           shortIntegerToDisplayString(Register_X, tmpString, true,  dispBase == 0 ? (!getSystemFlag(FLAG_BCD) ? 16 : 1) : dispBase); // base 1 is BCD, #10
           if(lastErrorCode == 0 && stringWidth(tmpString, fontForShortInteger, false, true) + stringWidth("  X: ", &standardFont, false, true) <= SCREEN_WIDTH) {
             showString("  X: ", &standardFont, 0 + BASEMODE_OFFSET_X, Y_POSITION_OF_REGISTER_X_LINE - REGISTER_LINE_HEIGHT*(REGISTER_T - REGISTER_X) + (fontForShortInteger == &standardFont ? 6 : 0) + BASEMODE_OFFSET_Y, vmNormal, false, true);
           }
           showString(tmpString, fontForShortInteger, SCREEN_WIDTH - stringWidth(tmpString, fontForShortInteger, false, true), Y_POSITION_OF_REGISTER_X_LINE - REGISTER_LINE_HEIGHT*(REGISTER_T - REGISTER_X) + (fontForShortInteger == &standardFont ? 6 : 0), vmNormal, false, true);
         }

       }
       else if(getRegisterDataType(REGISTER_X) == dtLongInteger && !solverEstimatesUsed) {
         //handle longinteger in pos T
         if((displayStack == 1 && calcMode != CM_NIM) || displayStack == 2 || displayStack == 3) {
           longIntegerToHexDisplayString(REGISTER_X, tmpString, true,  dispBase == 0 ? (!getSystemFlag(FLAG_BCD) ? 16 : 1) : dispBase, SCREEN_WIDTH - (isShiftOffset ? 10 : 0)); // base 1 is BCD, #10
           bool_t   printFirstCol = fontForShortInteger == &tinyFont;
           bool_t   printWillFit = stringWidth(tmpString, fontForShortInteger, printFirstCol, true) + stringWidth("  X:" STD_INTEGER_Z ": ", &standardFont, false, true) <= SCREEN_WIDTH - (isShiftOffset ? 10 : 0);
           uint32_t xoff = printWillFit ? SCREEN_WIDTH - (isShiftOffset ? 10 : 0) - stringWidth(tmpString, fontForShortInteger, printFirstCol, true) - 3 : (isShiftOffset ? 10 : 0);
           if(lastErrorCode == 0 && printWillFit) {
             showString("  X:" STD_INTEGER_Z ": ", &standardFont, 0 + BASEMODE_OFFSET_X, Y_POSITION_OF_REGISTER_X_LINE - REGISTER_LINE_HEIGHT*(REGISTER_T - REGISTER_X) + (fontForShortInteger == &standardFont ? 6 : 0) + BASEMODE_OFFSET_Y, vmNormal, false, true);
           }
           showStringEnhanced(tmpString, fontForShortInteger, xoff, Y_POSITION_OF_REGISTER_X_LINE - REGISTER_LINE_HEIGHT*(REGISTER_T - REGISTER_X) + (fontForShortInteger == &standardFont ? 6 : 0), vmNormal, printFirstCol, true, NO_compress, NO_raise, DO_Show, NO_Bold, DO_LF);
         }
       }

       if(displayStack == 3) {
         drawSinglePixelFullWidthLine(Y_POSITION_OF_REGISTER_Z_LINE - 2);
       }
       else if(displayStack == 2) {
         drawSinglePixelFullWidthLine(Y_POSITION_OF_REGISTER_Y_LINE - 2);
       }
       else if(displayStack == 1) {
         drawSinglePixelFullWidthLine(Y_POSITION_OF_REGISTER_X_LINE - 2);
       }
     }                                                                 //JM ^^
  }


  // Calculates a shortened real, using only reaal34
  bool_t registerFMA(calcRegister_t regist, real_t* tmp1, real_t* tmp2, real34_t* tmp3, angularMode_t* angle, realContext_t *c) {
    if(getRegisterDataType(regist) == dtShortInteger || getRegisterDataType(regist+1) == dtShortInteger || getRegisterDataType(regist+2) == dtShortInteger) {  //check for SI, because getRegisterAsRealQuiet will accept SI as leagl number.
      return false;
    }
    if(!getRegisterAsRealQuiet(regist, tmp1)) {
      return false;
    }
    if(getRegisterDataType(regist) == dtReal34) {
      *angle = getRegisterAngularMode(regist);
    } else {
      *angle = amNone;
    }
    if(!getRegisterAsRealQuiet(regist+1, tmp2)) {
      return false;
    }
    realMultiply(tmp1, tmp2, tmp1, c);

    if(!getRegisterAsRealQuiet(regist+2, tmp2)) {
      return false;
    }
    realAdd(tmp1, tmp2, tmp1, c);
    realToReal34(tmp1, tmp3);
    return true;
  }

#define LRWidth 140

static void displayLRtemporaryInformation(char *prefix1, char *prefix2, char *prefix, const char *label, bool_t prefixPre, bool_t prefixPost, int16_t *prefixWidth) {
   strcpy(prefix, prefix1);
   strcat(prefix, getCurveFitModeFormula(lrChosen));
   strcat(prefix, prefix2);
     while(stringWidth(prefix, &standardFont, prefixPre, prefixPost) + 1 < LRWidth) {
    strcat(prefix, STD_SPACE_6_PER_EM);
  }
  strcat(prefix,label);
  strcat(prefix, STD_SPACE_4_PER_EM "=" STD_SPACE_HAIR);
  *prefixWidth = stringWidth(prefix, &standardFont, prefixPre, prefixPost) + 1;
}

  #define RESTORE_T true

  static void _refreshRegisterLine(calcRegister_t regist, bool_t restoreRegisterT) {
    int32_t w = 0;
    int16_t wLastBaseNumeric, wLastBaseStandard, prefixWidth = 0, lineWidth = 0;
    bool_t prefixPre = true;
    bool_t prefixPost = true;
    const uint8_t origDisplayStack = displayStack;
    bool_t distModeActive = false;

    char prefix[200], lastBase[20];

    #ifdef DMCP_BUILD
      keyBuffer_pop();                                            // This causes key updates while the longer time processing register updates happen
      if( !skippedStackLines && (calcMode == CM_NORMAL || calcMode == CM_MIM) &&
          !(regist == REGISTER_X)&&// || regist == REGISTER_Y) &&
          !runningOnSimOrUSB &&                                   // Automatically, when on battery (hence low processor), change to skip long processing register printing, recovering the fragmented screen here: See timer.c fnTimerEndOfActivity()
          !emptyKeyBuffer() &&
          key_empty() == 1
          ) {
        skippedStackLines = true;
        return;
      }
    #endif //DMCP

                                      #if defined(PC_BUILD) && defined(MONITOR_CLRSCR)
                                        printf(">>> refreshRegisterLine   register=%u screenUpdatingMode=%d temporaryInformation=%u BASEMODEACTIVE=%u, lastIntegerBase=%u\n", regist, screenUpdatingMode, temporaryInformation, BASEMODEACTIVE, lastIntegerBase);
                                        #if defined(ANALYSE_REFRESH)
                                          print_caller(NULL);
                                        #endif //ANALYSE_REFRESH
                                      #endif // PC_BUILD &&MONITOR_CLRSCR

    if(BASEMODEREGISTERX && !SHOWMODE && displayStack != 4-displayStackSHOIDISP) { //JMSHOI
      if(getRegisterDataType(REGISTER_X) == dtShortInteger) {
        fnDisplayStack(4-displayStackSHOIDISP);
      }
      else {
        fnDisplayStack(3);
      }
    } else {
      if(XXFNMODEACTIVE) {
        fnDisplayStack(3);
      }
    }


    #if (DEBUG_PANEL == 1)
      if(programRunStop != PGM_RUNNING) {
        refreshDebugPanel();
      }
    #endif // (DEBUG_PANEL == 1)

    if((temporaryInformation == TI_SHOW_REGISTER || SHOWMODE) && regist == REGISTER_X) {     //JM top frame of the SHOW window  //HERE SHOW LINE
      drawSinglePixelFullWidthLine(Y_POSITION_OF_REGISTER_T_LINE-4);
    }

    if((calcMode != CM_BUG_ON_SCREEN) && !GRAPHMODE && (calcMode != CM_LISTXY)) {               //JM
      if(temporaryInformation != TI_SHOW_REGISTER_BIG) {                        //JMSHOW
        clearRegisterLine(regist, true, (regist != REGISTER_Y));
      }                                                                         //JMSHOW

      #if defined(PC_BUILD)
        #if (DEBUG_REGISTER_L == 1 || SHOW_MEMORY_STATUS == 1)
          char tmpStr[1000];
        #endif // (DEBUG_REGISTER_L == 1 || SHOW_MEMORY_STATUS == 1)
        #if (DEBUG_REGISTER_L == 1)
          char string1[1000], string2[1000], *p;
          uint16_t i;

          strcpy(string1, "L = ");

          if(getRegisterDataType(REGISTER_L) == dtReal34) {
            strcat(string1, "real34 = ");
            formatReal34Debug(string2, (real34_t *)getRegisterDataPointer(REGISTER_L));
            strcat(string2, " ");
            strcat(string2, getAngularModeName(getRegisterAngularMode(REGISTER_L)));
          }

          else if(getRegisterDataType(REGISTER_L) == dtComplex34) {
            strcat(string1, "complex34 = ");
            formatComplex34Debug(string2, getRegisterDataPointer(REGISTER_L));
          }

          else if(getRegisterDataType(REGISTER_L) == dtString) {
            strcat(string1, "string = ");
            for(i=0, p=REGISTER_STRING_DATA(REGISTER_L); i<=stringByteLength(REGISTER_STRING_DATA(REGISTER_L)); i++, p++) {
              string2[i] = *p;
            }
          }

          else if(getRegisterDataType(REGISTER_L) == dtShortInteger) {
            strcat(string1, "short integer = ");
            shortIntegerToDisplayString(REGISTER_L, string2, false, noBaseOverride);
            strcat(string2, STD_SPACE_3_PER_EM);
            strcat(string2, getShortIntegerModeName(shortIntegerMode));
          }

          else if(getRegisterDataType(REGISTER_L) == dtLongInteger) {
            strcat(string1, "long integer = ");
            longIntegerRegisterToDisplayString(REGISTER_L, string2, sizeof(string2), SCREEN_WIDTH, 50, true);
          }

          else if(getRegisterDataType(REGISTER_L) == dtTime) {
            strcat(string1, "time = ");
            formatReal34Debug(string2, (real34_t *)getRegisterDataPointer(REGISTER_L));
          }

          else if(getRegisterDataType(REGISTER_L) == dtDate) {
            strcat(string1, "date = ");
            formatReal34Debug(string2, (real34_t *)getRegisterDataPointer(REGISTER_L));
          }

          else if(getRegisterDataType(REGISTER_L) == dtReal34Matrix) {
            sprintf(&string1[strlen(string1)], "real34 %" PRIu16 STD_CROSS "%" PRIu16 " matrix = ", REGISTER_MATRIX_HEADER(REGISTER_L)->matrixRows, REGISTER_MATRIX_HEADER(REGISTER_L)->matrixColumns);
            formatReal34Debug(string2, REGISTER_REAL34_MATRIX_ELEMENTS(REGISTER_L));
          }

          else if(getRegisterDataType(REGISTER_L) == dtComplex34Matrix) {
            sprintf(&string1[strlen(string1)], "complex34 %" PRIu16 STD_CROSS "%" PRIu16 " matrix = ", REGISTER_MATRIX_HEADER(REGISTER_L)->matrixRows, REGISTER_MATRIX_HEADER(REGISTER_L)->matrixColumns);
            formatComplex34Debug(string2, REGISTER_COMPLEX34_MATRIX_ELEMENTS(REGISTER_L));
          }

          else if(getRegisterDataType(REGISTER_L) == dtConfig) {
            strcat(string1, "Configuration data");
            string2[0] = 0;
          }

          else {
            sprintf(string2, "data type %s not supported for now!", getRegisterDataTypeName(REGISTER_L, false, false));
          }

          stringToUtf8(string1, (uint8_t *)tmpStr);
          stringToUtf8(string2, (uint8_t *)tmpStr + 500);

          gtk_label_set_label(GTK_LABEL(lblRegisterL1), tmpStr);
          gtk_label_set_label(GTK_LABEL(lblRegisterL2), tmpStr + 500);
          gtk_widget_show(lblRegisterL1);
          gtk_widget_show(lblRegisterL2);
        #endif // (DEBUG_REGISTER_L == 1)
        #if (SHOW_MEMORY_STATUS == 1)
          char string[1000];

          sprintf(string, "%" PRIu32 " bytes free (%" PRId32 " region%s), C47 %" PRIu32 " bytes, GMP %" PRIu32 " bytes -> should always be 0", getFreeRamMemory(), numberOfFreeMemoryRegions, numberOfFreeMemoryRegions==1 ? "" : "s", (uint32_t)TO_BYTES((uint64_t)c47MemInBlocks), (uint32_t)gmpMemInBytes);
          stringToUtf8(string, (uint8_t *)tmpStr);
          gtk_label_set_label(GTK_LABEL(lblMemoryStatus), tmpStr);
          gtk_widget_show(lblMemoryStatus);
        #endif // (SHOW_MEMORY_STATUS == 1)
      #endif // PC_BUILD

      #if defined(VERBOSE_SCREEN) && defined(PC_BUILD)
        printf("^^^^Display Register: %d temporaryInformation: %d\n", regist, temporaryInformation);
      #endif //VERBOSE_SCREEN

      if(getRegisterDataType(REGISTER_X) == dtReal34Matrix || (calcMode == CM_MIM && getRegisterDataType(matrixIndex) == dtReal34Matrix)) {
        displayStack = cachedDisplayStack;
      }
      else if(getRegisterDataType(REGISTER_X) == dtComplex34Matrix || (calcMode == CM_MIM && getRegisterDataType(matrixIndex) == dtComplex34Matrix)) {
        displayStack = cachedDisplayStack;
      }

      if(temporaryInformation == TI_STATISTIC_LR && (getRegisterDataType(REGISTER_X) != dtReal34)) {
        if(regist == REGISTER_X) {
          if(orOrtho(lrSelection) == CF_ORTHOGONAL_FITTING) {
            sprintf(tmpString, "L.R. selected to OrthoF");
          }
          else {
            sprintf(tmpString, "L.R. selected to %03" PRIu16 ".", (uint16_t)((lrSelection) & 0x01FF));
          }
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            sprintf(errorMessage, "BestF is set, but will not work until REAL data points are used.");
            moreInfoOnError("In function _refreshRegisterLine:", errorMessage, errorMessages[ERROR_INVALID_DATA_TYPE_FOR_OP], NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          w = stringWidth(tmpString, &standardFont, true, true);
          showString(tmpString, &standardFont, SCREEN_WIDTH - w, Y_POSITION_OF_REGISTER_X_LINE + 6, vmNormal, true, true);
        }
      }

      else if(temporaryInformation == TI_BATTV && regist == REGISTER_X) {
        sprintf(prefix, "V" STD_SPACE_FIGURE "=");
        displayTemporaryInformationOnX(prefix);
      }

      else if(temporaryInformation == TI_BYTES && regist == REGISTER_X) {
        sprintf(prefix, "Bytes" STD_SPACE_FIGURE "=");
        displayTemporaryInformationOnX(prefix);
      }
      else if(temporaryInformation == TI_BITS && regist == REGISTER_X) {
        sprintf(prefix, "Bits" STD_SPACE_FIGURE "=");
        displayTemporaryInformationOnX(prefix);
      }

      else if(temporaryInformation == TI_ARE_YOU_SURE && regist == REGISTER_X) {
        uint16_t id = getConfirmationTiId();
        showString(confirmationTI[id].string, &standardFont, 1, Y_POSITION_OF_REGISTER_X_LINE + 6, vmNormal, true, true);
      }

      else if(temporaryInformation == TI_WHO) {
        if(regist == REGISTER_Z || regist == REGISTER_Y || regist == REGISTER_X) { //Force repainting it 3 times to get it painted properly over three lines
          showStringEnhanced(whoStr1, &standardFont, 1, Y_POSITION_OF_REGISTER_X_LINE - REGISTER_LINE_HEIGHT*2 + 6, vmNormal, true, true, NO_compress, NO_raise, DO_Show, NO_Bold, DO_LF);
        }
      }

      else if(temporaryInformation == TI_VERSION && regist == REGISTER_X) {
        clearRegisterLine(REGISTER_T, true, true);
        clearRegisterLine(REGISTER_Z, true, true);
        clearRegisterLine(REGISTER_Y, true, true);
        clearRegisterLine(REGISTER_X, true, true);
        showStringEnhanced(versionStr2,    &standardFont, 1, Y_POSITION_OF_REGISTER_T_LINE + 6, vmNormal, true, true, NO_compress, NO_raise, DO_Show, NO_Bold, DO_LF);
        showStringEnhanced(versionStr,     &standardFont, 1, Y_POSITION_OF_REGISTER_Z_LINE + 6, vmNormal, true, true, NO_compress, NO_raise, DO_Show, NO_Bold, DO_LF);
        showStringEnhanced(disclaimerStr,  &standardFont, 1, Y_POSITION_OF_REGISTER_Y_LINE + 6, vmNormal, true, true, NO_compress, NO_raise, DO_Show, NO_Bold, DO_LF);
      }

      else if(temporaryInformation == TI_DISP_JULIAN) {
        real34_t j;
        char tmpStr2[20];
        uInt32ToReal34(firstGregorianDay, &j);
        julianDayToInternalDate(&j,REGISTER_REAL34_DATA(TEMP_REGISTER_1));
        dateToDisplayString(TEMP_REGISTER_1, tmpStr2);
        sprintf(tmpString, "First Gregorian day set: %s", tmpStr2);
        //sprintf(tmpString, "1st Gregorian day set: %s (JD %" PRId32 ")", tmpStr2, firstGregorianDay);
        showString(tmpString, &standardFont, 1, Y_POSITION_OF_REGISTER_X_LINE + TEMPORARY_INFO_OFFSET + 6, vmNormal, true, true);
      }

      else if(temporaryInformation == TI_DISP_WOY) {
        sprintf(tmpString, "Week of Year rule set: %s.%s",
          nameOfWday_en[firstDayOfWeek].itemName,
          nameOfWday_en[firstWeekOfYearDay].itemName);
        showString(tmpString, &standardFont, 1, Y_POSITION_OF_REGISTER_X_LINE + TEMPORARY_INFO_OFFSET + 6, vmNormal, true, true);
      }

      else if(temporaryInformation == TI_DISP_JULIAN_WOY) {
        real34_t j;
        char tmpStr2[20];
        uInt32ToReal34(firstGregorianDay, &j);
        julianDayToInternalDate(&j,REGISTER_REAL34_DATA(TEMP_REGISTER_1));
        dateToDisplayString(TEMP_REGISTER_1, tmpStr2);
        sprintf(tmpString, "First Gregorian day set: %s", tmpStr2);
        //sprintf(tmpString, "1st Gregorian day set: %s (JD %" PRId32 ")", tmpStr2, firstGregorianDay);
        showString(tmpString, &standardFont, 1, Y_POSITION_OF_REGISTER_Y_LINE + TEMPORARY_INFO_OFFSET + 6, vmNormal, true, true);
        sprintf(tmpString, "Week of Year rule set: %s.%s",
          nameOfWday_en[firstDayOfWeek].itemName,
          nameOfWday_en[firstWeekOfYearDay].itemName);
        showString(tmpString, &standardFont, 1, Y_POSITION_OF_REGISTER_X_LINE + TEMPORARY_INFO_OFFSET + 6, vmNormal, true, true);
      }

      else if(temporaryInformation == TI_WOY && regist == REGISTER_X) {
        sprintf(prefix, "Week of Year" STD_SPACE_FIGURE "=");
        displayTemporaryInformationOnX(prefix);
      }

      else if(temporaryInformation == TI_WOY_RULE && regist == REGISTER_X) {
        sprintf(prefix, "%s.%s",
          nameOfWday_en[firstDayOfWeek].itemName,
          nameOfWday_en[firstWeekOfYearDay].itemName);
        displayTemporaryInformationOnX(prefix);
      }

      else if(temporaryInformation == TI_KEYS && regist == REGISTER_X) {
        showString(errorMessage, &standardFont, 1, Y_POSITION_OF_REGISTER_X_LINE + 6, vmNormal, true, true);
      }

      else if(regist == TRUE_FALSE_REGISTER_LINE && displayTrueFalse(regist)) {
      }

      else if(temporaryInformation == TI_RESET && regist == REGISTER_X) {
        sprintf(tmpString, "%s", errorMessages[TI_All_data_prgms_cleared]);
        w = stringWidth(tmpString, &standardFont, true, true);
        showString(tmpString, &standardFont, SCREEN_WIDTH - w, Y_POSITION_OF_REGISTER_X_LINE + 6, vmNormal, true, true);
      }

      else if(temporaryInformation == TI_SAVED && regist == REGISTER_X) {
        sprintf(prefix, "Saved");
        displayTemporaryInformationOnX(prefix);
      }

      else if(temporaryInformation == TI_DEL_ALL_PRGMS && regist == REGISTER_X) {
        sprintf(tmpString, "%s", errorMessages[TI_All_user_prgms_deleted]);
        w = stringWidth(tmpString, &standardFont, true, true);
        showString(tmpString, &standardFont, SCREEN_WIDTH - w, Y_POSITION_OF_REGISTER_X_LINE + 6, vmNormal, true, true);
      }

      else if(temporaryInformation == TI_CLEAR_ALL_FLAGS && regist == REGISTER_X) {
        sprintf(tmpString, "%s", errorMessages[TI_All_user_flags_cleared]);
        w = stringWidth(tmpString, &standardFont, true, true);
        showString(tmpString, &standardFont, SCREEN_WIDTH - w, Y_POSITION_OF_REGISTER_X_LINE + 6, vmNormal, true, true);
      }

      else if(temporaryInformation == TI_CLEAR_ALL_MENUS && regist == REGISTER_X) {
        sprintf(tmpString, "%s", errorMessages[TI_All_user_menus_cleared]);
        w = stringWidth(tmpString, &standardFont, true, true);
        showString(tmpString, &standardFont, SCREEN_WIDTH - w, Y_POSITION_OF_REGISTER_X_LINE + 6, vmNormal, true, true);
      }

      else if(temporaryInformation == TI_CLEAR_ALL_VARIABLES && regist == REGISTER_X) {
        sprintf(tmpString, "%s", errorMessages[TI_All_user_vars_cleared]);
        w = stringWidth(tmpString, &standardFont, true, true);
        showString(tmpString, &standardFont, SCREEN_WIDTH - w, Y_POSITION_OF_REGISTER_X_LINE + 6, vmNormal, true, true);
      }

      else if(temporaryInformation == TI_DEL_ALL_MENUS && regist == REGISTER_X) {
        sprintf(tmpString, "%s", errorMessages[TI_All_user_menus_deleted]);
        w = stringWidth(tmpString, &standardFont, true, true);
        showString(tmpString, &standardFont, SCREEN_WIDTH - w, Y_POSITION_OF_REGISTER_X_LINE + 6, vmNormal, true, true);
      }

      else if(temporaryInformation == TI_DEL_ALL_VARIABLES && regist == REGISTER_X) {
        sprintf(tmpString, "%s", errorMessages[TI_All_user_vars_deleted]);
        w = stringWidth(tmpString, &standardFont, true, true);
        showString(tmpString, &standardFont, SCREEN_WIDTH - w, Y_POSITION_OF_REGISTER_X_LINE + 6, vmNormal, true, true);
      }

      #if defined(PC_BUILD)
      else if(temporaryInformation == TI_NOT_AVAILABLE && regist == REGISTER_X) {
        sprintf(prefix, "%s", errorMessages[TI_Not_on_simulator]);
        displayTemporaryInformationOnX(prefix);
      }
      #elif defined(DMCP_BUILD)
      else if(temporaryInformation == TI_NOT_AVAILABLE && regist == REGISTER_X) {
        sprintf(prefix, "%s", errorMessages[TI_Only_on_simulator]);
        displayTemporaryInformationOnX(prefix);
      }
      #endif // PC_BUILD/DMCP_BUILD

      else if(temporaryInformation == TI_BACKUP_RESTORED && regist == REGISTER_X) {
        clearRegisterLine(REGISTER_X, true, true);
        clearRegisterLine(REGISTER_Y, true, true);
        clearRegisterLine(REGISTER_Z, true, true);
        clearRegisterLine(REGISTER_T, true, true);
        showString(errorMessages[TI_Backup_restored], &standardFont, 1, Y_POSITION_OF_REGISTER_Z_LINE + 6, vmNormal, true, true);
        showStringEnhanced(versionStr,  &standardFont, 1, Y_POSITION_OF_REGISTER_X_LINE + 6, vmNormal, true, true, NO_compress, NO_raise, DO_Show, NO_Bold, DO_LF);
        showStringEnhanced(versionStr2, &standardFont, 1, Y_POSITION_OF_REGISTER_Y_LINE + 6, vmNormal, true, true, NO_compress, NO_raise, DO_Show, NO_Bold, DO_LF);
      }

      else if(temporaryInformation == TI_STATEFILE_RESTORED && regist == REGISTER_X) {
        sprintf(prefix, "%s", errorMessages[TI_State_file_restored]);
        displayTemporaryInformationOnX(prefix);
      }

      else if(temporaryInformation == TI_PROGRAMS_RESTORED && regist == REGISTER_X) {
        sprintf(prefix, "                                ");
        displayTemporaryInformationOnX(prefix);
        sprintf(prefix, "%s", errorMessages[TI_Saved_programs_and_equations]);
        showString(prefix, &standardFont, 1, Y_POSITION_OF_REGISTER_X_LINE - 3, vmNormal, true, true);
        sprintf(prefix, "%s", errorMessages[TI_appended]);
        showString(prefix, &standardFont, 1, Y_POSITION_OF_REGISTER_X_LINE + 17, vmNormal, true, true);
     }

      else if(temporaryInformation == TI_REGISTERS_RESTORED && regist == REGISTER_X) {
        sprintf(prefix, "                                  ");
        displayTemporaryInformationOnX(prefix);
        sprintf(prefix, "%s", errorMessages[TI_Saved_global_and_local_registers]);
        showString(prefix, &standardFont, 1, Y_POSITION_OF_REGISTER_X_LINE - 3, vmNormal, true, true);
        sprintf(prefix, "%s", errorMessages[TI_w_local_flags_restored]);
        showString(prefix, &standardFont, 1, Y_POSITION_OF_REGISTER_X_LINE + 17, vmNormal, true, true);
      }

      else if(temporaryInformation == TI_SETTINGS_RESTORED && regist == REGISTER_X) {
        sprintf(prefix, "%s", errorMessages[TI_Saved_system_settings_restored]);
        displayTemporaryInformationOnX(prefix);
      }

      else if(temporaryInformation == TI_SUMS_RESTORED && regist == REGISTER_X) {
        sprintf(prefix, "%s", errorMessages[TI_Saved_statistic_data_restored]);
        displayTemporaryInformationOnX(prefix);
      }

      else if(temporaryInformation == TI_VARIABLES_RESTORED && regist == REGISTER_X) {
        sprintf(prefix, "%s", errorMessages[TI_Saved_user_variables_restored]);
        displayTemporaryInformationOnX(prefix);
      }

      else if(temporaryInformation == TI_PROGRAM_LOADED && regist == REGISTER_X) {
        sprintf(prefix, "%s", errorMessages[TI_Program_file_loaded]);
        displayTemporaryInformationOnX(prefix);
      }

      else if(temporaryInformation == TI_UNDO_DISABLED && regist == REGISTER_X) {
        showString(errorMessages[ERROR_TI_UNDO_FAILED], &standardFont, 1, Y_POSITION_OF_REGISTER_X_LINE + 6, vmNormal, true, true);
      }



      // NEW SHOW                                                                  //JMSHOW vv
      else if(temporaryInformation == TI_SHOW_REGISTER_SMALL || temporaryInformation == TI_SHOW_REGISTER) {
        switch(regist) {
          case REGISTER_X:{
              clearScreenOld(!clrStatusBar, clrRegisterLines, clrSoftkeys);
              int16_t nn = 0;
              while (nn <= 9) {
                showDispSmall( nn * SHOWLineSize, nn);          // L1
                nn++;
              }
              showBottomLine();
              break;
            }
            default: ;
          }
        }


      else if(temporaryInformation == TI_SHOW_REGISTER_TINY) {
        switch(regist) {
          case REGISTER_X:{
              clearScreenOld(!clrStatusBar, clrRegisterLines, clrSoftkeys);
              int16_t nn = 0;
              while (nn<=SCREEN_HEIGHT/line_tiny && nn<SHOWLineMax) {
                showDispSmall( nn * SHOWLineSize, nn);          // L1
                nn++;
              }
              showBottomLine();
              break;
            }
            default: ;
          }
        }


      else if(temporaryInformation == TI_SHOW_REGISTER_BIG) {
        if(regist == REGISTER_T) {
            int16_t nn = 0;
            while (nn<=5) {
              showDisp( nn * SHOWLineSize, nn);
              nn++;
            }
            showBottomLine();
          }
      }

//Keepin original until tested, for easy of putting back:
//      else if((!checkHP && restoreRegisterT == RESTORE_T && regist == REGISTER_T) ||
//               regist < REGISTER_X + min(displayStack, origDisplayStack) ||
//               (lastErrorCode != 0 && regist == errorMessageRegisterLine) ||
//               (temporaryInformation == TI_VIEW_REGISTER && regist == REGISTER_T)) {
//Cautiously removing the first condition above, as it should only be active when display stack allows; and this AND condition will be overridden by the second OR condition.

      else if( (regist < REGISTER_X + min(displayStack, origDisplayStack)) ||
               (lastErrorCode != 0 && regist == errorMessageRegisterLine) ||
               (temporaryInformation == TI_VIEW_REGISTER && regist == REGISTER_T) ) {
        prefixWidth = 0;
        const int16_t baseY = Y_POSITION_OF_REGISTER_X_LINE - REGISTER_LINE_HEIGHT*(regist - REGISTER_X + ((restoreRegisterT == RESTORE_T) ? 0 : ((temporaryInformation == TI_VIEW_REGISTER && regist == REGISTER_T) ? 0 : (getRegisterDataType(REGISTER_X) == dtReal34Matrix || getRegisterDataType(REGISTER_X) == dtComplex34Matrix) ? 4 - displayStack : 0)));
                                        #if defined(PC_BUILD)
                                        if(baseY < 0) {
                                          printf("ILLEGAL BASE VALUE baseY<0 : baseY=%i regist=%u regist-REGISTER_X=%u cachedDisplayStack=%u displayStack=%u\n",  baseY, regist, regist-REGISTER_X, cachedDisplayStack, displayStack);
                                          #if defined(ANALYSE_REFRESH)
                                            print_caller(NULL);
                                          #endif //PC_BUILD && ANALYSE_REFRESH
                                        }
                                        #endif //PC_BUILD
        calcRegister_t origRegist = regist;
        if(temporaryInformation == TI_VIEW_REGISTER && regist == REGISTER_T) {
          if(FIRST_RESERVED_VARIABLE <= currentViewRegister && currentViewRegister < LAST_RESERVED_VARIABLE && allReservedVariables[currentViewRegister - FIRST_RESERVED_VARIABLE].header.pointerToRegisterData == C47_NULL) {
            copySourceRegisterToDestRegister(currentViewRegister, TEMP_REGISTER_1);
            regist = TEMP_REGISTER_1;
          }
          else {
            regist = currentViewRegister;
          }
        }

        if(regist == REGISTER_X && currentInputVariable != INVALID_VARIABLE) {
          inputRegName(prefix, &prefixWidth);
        }


        // STATISTICAL DISTR & SOLVER
        if(regist == REGISTER_X && lastErrorCode == 0 && calcMode != CM_PEM &&
            ( (PROBMENU) ||
              (currentMenu() == -MNU_Solver_TOOL && solverEstimatesUsed && temporaryInformation != TI_SOLVER_VARIABLE_RESULT)
            )) {
          const char *r_i = NULL, *r_j = NULL, *r_k = NULL;
          calcRegister_t register_i = REGISTER_X, register_j = REGISTER_X, register_k = REGISTER_X;

          switch(currentMenu()) {
            case -MNU_Solver_TOOL:
              r_i = indexOfItems[VAR_UEST].itemCatalogName; register_i = RESERVED_VARIABLE_UEST;
              r_j = indexOfItems[VAR_LEST].itemCatalogName; register_j = RESERVED_VARIABLE_LEST;
              break;
            case -MNU_PARETO:
              r_i = STD_mu;                 register_i = REGISTER_M;
              r_j = STD_sigma;              register_j = REGISTER_S;
              r_k = STD_alpha;              register_k = REGISTER_Q;
              break;
            case -MNU_GEV:
              r_i = STD_mu;                 register_i = REGISTER_M;
              r_j = STD_sigma;              register_j = REGISTER_S;
              r_k = STD_xi;                 register_k = REGISTER_Q;
              break;
            case -MNU_BINOM:
              r_i = STD_p;                  register_i = REGISTER_P;
              r_j = STD_n;                  register_j = REGISTER_N;
              break;
            case -MNU_CAUCH:
              r_i = STD_x STD_SUB_0;        register_i = REGISTER_M;
              r_j = STD_gamma;              register_j = REGISTER_S;
              break;
            case -MNU_WEIBL:
              r_i = STD_k;                  register_i = REGISTER_Q;
              r_j = STD_lambda;             register_j = REGISTER_S;
              break;
            case -MNU_CHI2:
            case -MNU_T:
              r_i = STD_nu;                 register_i = REGISTER_M;
              break;
            case -MNU_EXPON:
            case -MNU_POISS:
              r_i = STD_lambda;             register_i = REGISTER_R;
              break;
            case -MNU_F:
              r_i = STD_d STD_SUB_1;        register_i = REGISTER_M;
              r_j = STD_d STD_SUB_2;        register_j = REGISTER_N;
              break;
            case -MNU_GEOM:
              r_i = STD_p;                  register_i = REGISTER_P;
              break;
            case -MNU_HYPER:
              r_i = STD_N;                  register_i = REGISTER_M;
              r_j = STD_n;                  register_j = REGISTER_N;
              r_k = STD_K;                  register_k = REGISTER_Q;
              break;
            case -MNU_LOGIS:
              r_j = STD_s;                  register_j = REGISTER_S;
              r_i = STD_mu;                 register_i = REGISTER_M;
              break;
            case -MNU_NORML:
              r_j = STD_sigma;              register_j = REGISTER_S;
              r_i = STD_mu;                 register_i = REGISTER_M;
              break;
            case -MNU_UNIFORM:
            case -MNU_DISUNIFORM:
              r_i = STD_a;                  register_i = REGISTER_M;
              r_j = STD_b;                  register_j = REGISTER_N;
              break;
            default: ;
          }

          if(r_i != NULL || r_j != NULL || r_k != NULL) {
            stats_param_display(r_i, register_i, prefix, tmpString, REGISTER_T);
            stats_param_display(r_j, register_j, prefix, tmpString, REGISTER_Z);
            stats_param_display(r_k, register_k, prefix, tmpString, REGISTER_Y);

            prefix[0]=0;
            tmpString[0]=0;
            uint8_t ii = 255;
            if(r_i != NULL) {
              ii = Y_POSITION_OF_REGISTER_Z_LINE;
              fnDisplayStack(3);
              distModeActive = true;
            }
            if(r_j != NULL) {
              ii = Y_POSITION_OF_REGISTER_Y_LINE;
              fnDisplayStack(2);
              distModeActive = true;
            }
            if(r_k != NULL) {
              ii = Y_POSITION_OF_REGISTER_X_LINE;
              fnDisplayStack(1);
              distModeActive = true;
            }
            if(distModeActive) {
              drawSinglePixelFullWidthLine(ii - 2);
              if(displayStack != origDisplayStack) {
//                refreshScreen(81);                                //recurse into refreshScreen
              }
            }
          }
        }

        // XXFN DISPLAY
        if(regist == REGISTER_X && XXFNMODEACTIVE) {
          int tmpY = Y_POSITION_OF_REGISTER_X_LINE - REGISTER_LINE_HEIGHT*(REGISTER_T - REGISTER_X);

          angularMode_t angle;
          real_t tmp1, tmp2;
          real34_t tmp3;
          #define FMA_X 19-3
          #define FMA_T -1-3
          uint8_t savedDisplayFormat = displayFormat, savedDisplayFormatDigits = displayFormatDigits;
          displayFormat = DF_ALL;
          displayFormatDigits = 19;

          {
            sprintf(tmpString, "X%sY+Z=", PRODUCT_SIGN);
            int xx = showString(tmpString, &standardFont, (isShiftOffset ? 20 : 0), tmpY + FMA_X, vmNormal, false, true);
              if(isXFNregisterValid3r(REGISTER_X + (calcMode == CM_NIM ? 1 : 0)) && registerFMA(REGISTER_X + (calcMode == CM_NIM ? 1 : 0), &tmp1, &tmp2, &tmp3, &angle, &ctxtReal39)) {
                tmpString[0] = 0;
                real34ToDisplayString(&tmp3, angle, tmpString, &standardFont, SCREEN_WIDTH - (isShiftOffset ? 20 : 0) - xx, 34, LIMITEXP, FRONTSPACE, NOIRFRAC);
              } else {
                sprintf(tmpString, "%s ", errorMessages[ERROR_INVALID_TYPE_XFN]);
              }
              showString(tmpString, &standardFont, SCREEN_WIDTH - stringWidth(tmpString, &standardFont, false, true), tmpY + FMA_X, vmNormal, false, true);
          }
          if(getSystemFlag(FLAG_SSIZE8)) {
            sprintf(tmpString, "T%sA+B=", PRODUCT_SIGN);
            int xx = showString(tmpString, &standardFont, (isShiftOffset ? 20 : 0), tmpY + FMA_T, vmNormal, false, true);
              if(isXFNregisterValid3r(REGISTER_T + (calcMode == CM_NIM ? 1 : 0)) && registerFMA(REGISTER_T + (calcMode == CM_NIM ? 1 : 0), &tmp1, &tmp2, &tmp3, &angle, &ctxtReal39)) {
                tmpString[0] = 0;
                real34ToDisplayString(&tmp3, angle, tmpString, &standardFont, SCREEN_WIDTH - (isShiftOffset ? 20 : 0) - xx, 34, LIMITEXP, FRONTSPACE, NOIRFRAC);
              } else {
                sprintf(tmpString, "%s ", errorMessages[ERROR_INVALID_TYPE_XFN]);
              }
              showString(tmpString, &standardFont, SCREEN_WIDTH - stringWidth(tmpString, &standardFont, false, true), tmpY + FMA_T , vmNormal, false, true);
          }
          displayFormat = savedDisplayFormat;
          displayFormatDigits = savedDisplayFormatDigits;
          drawSinglePixelFullWidthLine(Y_POSITION_OF_REGISTER_Z_LINE - 2);
          fnDisplayStack(3);
        }



        if(lastErrorCode != 0 && regist == errorMessageRegisterLine) {
          if(stringWidth(errorMessages[lastErrorCode], &standardFont, true, true) <= SCREEN_WIDTH - 1) {
            if(lastErrorCode == ERROR_RESERVED_VARIABLE_NAME) {
              sprintf(tmpString, "%s: %s", errorMessages[lastErrorCode],errorMessage);

              showString(tmpString, &standardFont, 1, Y_POSITION_OF_REGISTER_X_LINE - REGISTER_LINE_HEIGHT*(regist - REGISTER_X) + 6, vmNormal, true, true);
            }
            else {
              showString(errorMessages[lastErrorCode], &standardFont, 1, Y_POSITION_OF_REGISTER_X_LINE - REGISTER_LINE_HEIGHT*(regist - REGISTER_X) + 6, vmNormal, true, true);
            }
          }
          else {
            #if (EXTRA_INFO_ON_CALC_ERROR == 1)
              sprintf(errorMessage, "Error message %" PRIu8 " is too wide!", lastErrorCode);
              moreInfoOnError("In function _refreshRegisterLine:", errorMessage, errorMessages[lastErrorCode], NULL);
            #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
            sprintf(tmpString, "Error message %" PRIu8 " is too wide!", lastErrorCode);
            w = stringWidth(tmpString, &standardFont, true, true);
            showString(tmpString, &standardFont, SCREEN_WIDTH - w, Y_POSITION_OF_REGISTER_X_LINE - REGISTER_LINE_HEIGHT*(regist - REGISTER_X) + 6, vmNormal, true, true);
          }
        }

        else if(regist == NIM_REGISTER_LINE && calcMode == CM_NIM) {
          if(lastIntegerBase != 0) {
            lastBase[0] = '#';
            if(lastIntegerBase > 9) {
              lastBase[1] = '1';
              lastBase[2] = '0' + (lastIntegerBase - 10);
              lastBase[3] = 0;
            }
            else {
              lastBase[1] = '0' + lastIntegerBase;
              lastBase[2] = 0;
            }
            wLastBaseNumeric  = stringWidth(lastBase, &numericFont,  true, true);
            wLastBaseStandard = stringWidth(lastBase, &standardFont, true, true);
          }
          else if(aimBuffer[0] != 0 && aimBuffer[strlen(aimBuffer)-1]=='/') {
            char *lb = lastBase;

            uint32_t iDigit = pow(10,(int)log10(lastDenominator)+1);
            uint32_t iDigit1;
            while(iDigit >= 10) {
              iDigit1 = iDigit / 10;
              if(lastDenominator >= iDigit1) {
                //printf("%i %i %i\n",lastDenominator, iDigit, iDigit1);
                *(lb++) = STD_SUB_0[0];
                *(lb++) = STD_SUB_0[1] + (lastDenominator % iDigit) / (iDigit1);
              }
              iDigit = iDigit1;
            }

    //        if(lastDenominator >= 100000) {
    //          *(lb++) = STD_SUB_0[0];
    //          *(lb++) = STD_SUB_0[1] + (lastDenominator / 100000);
    //        }
    //        if(lastDenominator >= 10000) {
    //          *(lb++) = STD_SUB_0[0];
    //          *(lb++) = STD_SUB_0[1] + (lastDenominator % 100000 / 10000);
    //        }
    //        if(lastDenominator >= 1000) {
    //          *(lb++) = STD_SUB_0[0];
    //          *(lb++) = STD_SUB_0[1] + (lastDenominator % 10000 / 1000);
    //        }
    //        if(lastDenominator >= 100) {
    //          *(lb++) = STD_SUB_0[0];
    //          *(lb++) = STD_SUB_0[1] + (lastDenominator % 1000 / 100);     // 210 % 1000 / 100 ==> 210/100 = 2
    //        }
    //        if(lastDenominator >= 10) {
    //          *(lb++) = STD_SUB_0[0];
    //          *(lb++) = STD_SUB_0[1] + (lastDenominator % 100 / 10);       // 50 % 100 / 10  ==> 50 / 10 = 5
    //        }
    //        *(lb++) = STD_SUB_0[0];
    //        *(lb++) = STD_SUB_0[1] + (lastDenominator % 10);

            *(lb++) = 0;
            wLastBaseNumeric  = stringWidth(lb, &numericFont,  true, true);    //fixed to lb
            wLastBaseStandard = stringWidth(lb, &standardFont, true, true);    //fixed to lb
          }
          else {
            wLastBaseNumeric  = 0;
            wLastBaseStandard = 0;
          }

          displayBaseMode(regist);
          //printStringToConsole(nimBufferDisplay,"XX: nimBufferDisplay:","\n");
          //printStringToConsole(lastBase,        "YY: lastBase:","\n");
          displayNim(nimBufferDisplay, lastBase, wLastBaseNumeric, wLastBaseStandard);
        }

        else if(regist == AIM_REGISTER_LINE && calcMode == CM_AIM && !tam.mode) {
          //JMCURSOR vv
          #if defined(TEXT_MULTILINE_EDIT)
            int16_t tmplen = stringByteLength(aimBuffer);
            if(T_cursorPos > tmplen) {T_cursorPos = tmplen;}     //Do range checking in case the cursor starts off outside of range
            if(T_cursorPos < 0)      {T_cursorPos = tmplen;}     //Do range checking in case the cursor starts off outside of range
            showStringEdC47(multiEdLines ,displayAIMbufferoffset, T_cursorPos, aimBuffer, 1, Y_POSITION_OF_NIM_LINE - 3 - checkHPoffset, vmNormal, true, true, false);  //display up to the cursor

            if(T_cursorPos == tmplen) {
              cursorEnabled = true;
            }
            else {
              cursorEnabled = false;
            }
            if(combinationFonts == 2) {
              cursorFont = &numericFont;                             //JM ENLARGE
            }
            else {
              cursorFont = &standardFont;                            //JM ENLARGE
            }
            //JMCURSOR  ^^
          #else // !TEXT_MULTILINE_EDIT

            // JM Removed and replaced with JMCURSOR vv
            if(stringWidth(aimBuffer, &standardFont, true, true) < SCREEN_WIDTH - 8) { // 8 is the standard font cursor width
              xCursor = showString(aimBuffer, &standardFont, 1, Y_POSITION_OF_NIM_LINE + 6, vmNormal, true, true);
              yCursor = Y_POSITION_OF_NIM_LINE + 6;
              cursorFont = &standardFont;
            }
            else {
              char *aimw;
              w = stringByteLength(aimBuffer) + 1;
              xcopy(tmpString,        aimBuffer, w);
              xcopy(tmpString + 1500, aimBuffer, w);
              aimw = stringAfterPixels(tmpString, &standardFont, SCREEN_WIDTH - 2, true, true);
              w = aimw - tmpString;
              *aimw = 0;

              if(stringWidth(tmpString + 1500 + w, &standardFont, true, true) >= SCREEN_WIDTH - 8) { // 8 is the standard font cursor width
                fnKeyBackspace(0); // back space
              }
              else {
                showString(tmpString, &standardFont, 1, Y_POSITION_OF_NIM_LINE - 3, vmNormal, true, true);

                xCursor = showString(tmpString + 1500 + w, &standardFont, 1, Y_POSITION_OF_NIM_LINE + 18, vmNormal, true, true);
                yCursor = Y_POSITION_OF_NIM_LINE + 18;
                cursorFont = &standardFont;
              }
            }
            // JM Removed and replaced with JMCURSOR ^^
          #endif // TEXT_MULTILINE_EDIT
        }




/*Main type dtReal34 FLAG_FRACT*/
        else if(getSystemFlag(FLAG_FRACT)
                    && (    getRegisterDataType(regist) == dtReal34
                         && (
                                real34CompareAbsLessThan(REGISTER_REAL34_DATA(regist), const34_1e6)
                             || real34IsZero(REGISTER_REAL34_DATA(regist))
                            )
                       )
               ) {

          if(temporaryInformation == TI_VIEW_REGISTER && origRegist == REGISTER_T) {
            viewRegName(prefix, &prefixWidth);
          }
          else if(temporaryInformation == TI_VIEW_REGISTER) {          //X, Y, & Z, not T
            userTI(currentViewRegister, regist, prefix, &prefixWidth);
          }

          fractionToDisplayString(regist, tmpString);

          w = stringWidth(tmpString, &numericFont, false, true);
          lineWidth = w;

          if(prefixWidth > 0) {
            if(temporaryInformation == TI_INTEGRAL && regist == REGISTER_X) {
              showString(prefix, &numericFont, 1, baseY - checkHPoffset, vmNormal, prefixPre, prefixPost);
            }
            else {
              showString(prefix, &standardFont, 1, baseY - checkHPoffset + TEMPORARY_INFO_OFFSET, vmNormal, prefixPre, prefixPost);
            }
          }

          if(w <= SCREEN_WIDTH) {
            showString(tmpString, &numericFont, SCREEN_WIDTH - w, baseY - checkHPoffset, vmNormal, false, true);
          }
          else {
            w = stringWidth(tmpString, &standardFont, false, true);
            lineWidth = w;
            if(w > SCREEN_WIDTH) {
              #if (EXTRA_INFO_ON_CALC_ERROR == 1)
                moreInfoOnError("In function _refreshRegisterLine:", "Fraction representation too wide!", tmpString, NULL);
              #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
              strcpy(tmpString, "Fraction representation too wide!");
              w = stringWidth(tmpString, &standardFont, false, true);
              lineWidth = w;
            }
            showString(tmpString, &standardFont, SCREEN_WIDTH - w, baseY, vmNormal, false, true);
          }
        }





/*Main type dtReal34*/
        else if(getRegisterDataType(regist) == dtReal34) {
          if(temporaryInformation == TI_COPY_FROM_SHOW && regist == REGISTER_X) {
            _fnShowRecallTI(prefix, &prefixWidth);
          }

          else if(temporaryInformation == TI_THETA_RADIUS) {
            if(regist == REGISTER_Y) {
              strcpy(prefix, "r =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            else if(regist == REGISTER_X) {
              strcpy(prefix, STD_theta_m " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_RADIUS_THETA) {
            if(regist == REGISTER_X) {
              strcpy(prefix, "r =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            else if(regist == REGISTER_Y) {
              strcpy(prefix, STD_theta_m " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_RADIUS_THETA_SWAPPED) {
            if(regist == REGISTER_Y) {
              strcpy(prefix, "r =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            else if(regist == REGISTER_X) {
              strcpy(prefix, STD_theta_m " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_PERC) {
            if(regist == REGISTER_X) {
              strcpy(prefix, " % :");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_PERCD) {
            if(regist == REGISTER_X) {
              strcpy(prefix, STD_DELTA "% :");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_PERCD2) {
            if(regist == REGISTER_Y) {
              strcpy(prefix, " % :");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            if(regist == REGISTER_X) {
              strcpy(prefix, STD_DELTA "% :");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_X_Y) {
            if(regist == REGISTER_X) {
              strcpy(prefix, "x : Re =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            else if(regist == REGISTER_Y) {
              strcpy(prefix, "y : Im =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_X_Y_SWAPPED) {
            if(regist == REGISTER_Y) {
              strcpy(prefix, "x : Re =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            else if(regist == REGISTER_X) {
              strcpy(prefix, "y : Im =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_RE_IM) {
            if(regist == REGISTER_X) {
              strcpy(prefix, "Im" STD_SPACE_FIGURE "=");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            else if(regist == REGISTER_Y) {
              strcpy(prefix, "Re" STD_SPACE_FIGURE "=");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_SUMX_SUMY) {
            if(regist == REGISTER_X) {
              strcpy(prefix, STD_SIGMA "x =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            else if(regist == REGISTER_Y) {
              strcpy(prefix, STD_SIGMA "y =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_XMIN_YMIN) {
            if(regist == REGISTER_X) {
              strcpy(prefix, "x" STD_SUB_m STD_SUB_i STD_SUB_n " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            else if(regist == REGISTER_Y) {
              strcpy(prefix, "y" STD_SUB_m STD_SUB_i STD_SUB_n " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_XMAX_YMAX) {
            if(regist == REGISTER_X) {
              strcpy(prefix, "x" STD_SUB_m STD_SUB_a STD_SUB_x " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            else if(regist == REGISTER_Y) {
              strcpy(prefix, "y" STD_SUB_m STD_SUB_a STD_SUB_x " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_SA) {
            if(regist == REGISTER_X) {
              strcpy(prefix, "s(a" STD_SUB_0 ") =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            else if(regist == REGISTER_Y) {
              strcpy(prefix, "s(a" STD_SUB_1 ") =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_MEANX_MEANY || temporaryInformation == TI_MEANX) {
            if(regist == REGISTER_X) {
              strcpy(prefix, STD_x_BAR " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            else if(regist == REGISTER_Y && temporaryInformation != TI_MEANX) {
              strcpy(prefix, STD_y_BAR " =");
               prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
           }

          else if(temporaryInformation == TI_PCTILEX_PCTILEY) {
            if(regist == REGISTER_X) {
              strcpy(prefix, "pctile" STD_SUB_x " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            else if(regist == REGISTER_Y) {
              strcpy(prefix, "pctile" STD_SUB_y " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_MEDIANX_MEDIANY) {
            if(regist == REGISTER_X) {
              strcpy(prefix, "md" STD_SUB_x " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            else if(regist == REGISTER_Y) {
              strcpy(prefix, "md" STD_SUB_y " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_Q1X_Q1Y) {
            if(regist == REGISTER_X) {
              strcpy(prefix, "Q" STD_SUB_1 STD_SPACE_3_PER_EM STD_SUB_x " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            else if(regist == REGISTER_Y) {
              strcpy(prefix, "Q" STD_SUB_1 STD_SPACE_3_PER_EM STD_SUB_y " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_Q3X_Q3Y) {
            if(regist == REGISTER_X) {
              strcpy(prefix, "Q" STD_SUB_3 STD_SPACE_3_PER_EM STD_SUB_x " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            else if(regist == REGISTER_Y) {
              strcpy(prefix, "Q" STD_SUB_3 STD_SPACE_3_PER_EM STD_SUB_y " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_MADX_MADY) {
            if(regist == REGISTER_X) {
              strcpy(prefix, "mad" STD_SUB_x " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            else if(regist == REGISTER_Y) {
              strcpy(prefix, "mad" STD_SUB_y " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_IQRX_IQRY) {
            if(regist == REGISTER_X) {
              strcpy(prefix, "iqr" STD_SUB_x " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            else if(regist == REGISTER_Y) {
              strcpy(prefix, "iqr" STD_SUB_y " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_RANGEX_RANGEY) {
            if(regist == REGISTER_X) {
              strcpy(prefix, "rg" STD_SUB_x " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            else if(regist == REGISTER_Y) {
              strcpy(prefix, "rg" STD_SUB_y " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_SAMPLSTDDEV) {
            if(regist == REGISTER_X) {
              strcpy(prefix, "s" STD_SUB_x " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            else if(regist == REGISTER_Y) {
              strcpy(prefix, "s" STD_SUB_y " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_POPLSTDDEV) {
            if(regist == REGISTER_X) {
              strcpy(prefix, STD_sigma STD_SUB_x " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            else if(regist == REGISTER_Y) {
              strcpy(prefix, STD_sigma STD_SUB_y " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_STDERR) {
            if(regist == REGISTER_X) {
              strcpy(prefix, "s" STD_SUB_m STD_SUB_x " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            else if(regist == REGISTER_Y) {
              strcpy(prefix, "s" STD_SUB_m STD_SUB_y " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_GEOMMEANX_GEOMMEANY) {
            if(regist == REGISTER_X) {
              strcpy(prefix, STD_x_BAR STD_SUB_G " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            else if(regist == REGISTER_Y) {
              strcpy(prefix, STD_y_BAR STD_SUB_G " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_GEOMSAMPLSTDDEV) {
            if(regist == REGISTER_X) {
              strcpy(prefix, STD_epsilon STD_SUB_x " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            else if(regist == REGISTER_Y) {
              strcpy(prefix, STD_epsilon STD_SUB_y " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_GEOMPOPLSTDDEV) {
            if(regist == REGISTER_X) {
              strcpy(prefix, STD_epsilon STD_SUB_p STD_SUB_x " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            else if(regist == REGISTER_Y) {
              strcpy(prefix, STD_epsilon STD_SUB_p STD_SUB_y " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_GEOMSTDERR) {
            if(regist == REGISTER_X) {
              strcpy(prefix, STD_epsilon STD_SUB_m STD_SUB_x " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            else if(regist == REGISTER_Y) {
              strcpy(prefix, STD_epsilon STD_SUB_m STD_SUB_y " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_WEIGHTEDMEANX) {
            if(regist == REGISTER_X) {
              strcpy(prefix, STD_x_BAR STD_SUB_w " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_WEIGHTEDSAMPLSTDDEV) {
            if(regist == REGISTER_X) {
              strcpy(prefix, "s" STD_SUB_w " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_WEIGHTEDPOPLSTDDEV) {
            if(regist == REGISTER_X) {
              strcpy(prefix, STD_sigma STD_SUB_w " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_WEIGHTEDSTDERR) {
            if(regist == REGISTER_X) {
              strcpy(prefix, "s" STD_SUB_m STD_SUB_w " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_STATISTIC_HISTO) {
            if(regist == REGISTER_X) {
              strcpy(prefix,STD_UP_ARROW "BIN" STD_SPACE_FIGURE ":");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            else
            if(regist == REGISTER_Y) {
              strcpy(prefix,STD_DOWN_ARROW "BIN" STD_SPACE_FIGURE ":");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            else if(regist == REGISTER_Z) {
              strcpy(prefix,"nBINS" STD_SPACE_FIGURE ":");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_ROOTS3) {
            if(regist == REGISTER_X || regist == REGISTER_Y || regist == REGISTER_Z) {
              strcpy(prefix,"Root" STD_SPACE_FIGURE ":");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            #ifdef DISCRIMINANT
            if(regist == REGISTER_T) {
              strcpy(prefix,STD_UP_ARROW "  Discr." STD_SPACE_FIGURE ":");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            #endif //DISCRIMINANT
          }

          else if(temporaryInformation == TI_ROOTS2) {
            if(regist == REGISTER_X || regist == REGISTER_Y) {
              strcpy(prefix,"Root" STD_SPACE_FIGURE ":");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            #ifdef DISCRIMINANT
            if(regist == REGISTER_Z) {
              strcpy(prefix,STD_UP_ARROW "Discr." STD_SPACE_FIGURE ":");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            #endif //DISCRIMINANT
          }
         else if (temporaryInformation == TI_LR_A0) {
            if(regist == REGISTER_X)
              displayLRtemporaryInformation("y" STD_SPACE_4_PER_EM "=" STD_SPACE_4_PER_EM, ":" STD_SPACE_4_PER_EM, prefix, "a" STD_SUB_0, prefixPre, prefixPost, &prefixWidth);
         } else if (temporaryInformation == TI_LR_A1) {
            if(regist == REGISTER_X)
              displayLRtemporaryInformation("y" STD_SPACE_4_PER_EM "=" STD_SPACE_4_PER_EM, ":" STD_SPACE_4_PER_EM, prefix, "a" STD_SUB_1, prefixPre, prefixPost, &prefixWidth);
         } else if (temporaryInformation == TI_LR_A2) {
            if(regist == REGISTER_X)
              displayLRtemporaryInformation("y" STD_SPACE_4_PER_EM "=" STD_SPACE_4_PER_EM, ":" STD_SPACE_4_PER_EM, prefix, "a" STD_SUB_2, prefixPre, prefixPost, &prefixWidth);
         } 
          //L.R. Display
          else if(temporaryInformation == TI_LR && lrChosen != 0) {
            bool_t prefixPre = false;
            bool_t prefixPost = false;

            if(lrChosen == CF_CAUCHY_FITTING || lrChosen == CF_GAUSS_FITTING || lrChosen == CF_PARABOLIC_FITTING){
              if(regist == REGISTER_X) {
                displayLRtemporaryInformation("", "", prefix, "a" STD_SUB_0, prefixPre, prefixPost, &prefixWidth);
              }
              else if(regist == REGISTER_Y) {
                strcpy(prefix,"y" STD_SPACE_4_PER_EM "=" STD_SPACE_4_PER_EM);
                while(stringWidth(prefix, &standardFont, prefixPre, prefixPost) + 1 < LRWidth) {
                  strcat(prefix,STD_SPACE_6_PER_EM);
                }
                strcat(prefix, "a" STD_SUB_1 STD_SPACE_4_PER_EM "=" STD_SPACE_HAIR);
                prefixWidth = stringWidth(prefix, &standardFont, prefixPre, prefixPost) + 1;
              }
              else if(regist == REGISTER_Z) {
                strcpy(prefix, eatSpacesEnd(getCurveFitModeName(lrChosen)));
                if(lrCountOnes(lrSelection)>1) {
                  strcat(prefix,lrChosen == 0 ? "" : STD_SUP_ASTERISK);
                }
                while(stringWidth(prefix, &standardFont, prefixPre, prefixPost) + 1 < LRWidth) {
                  strcat(prefix,STD_SPACE_6_PER_EM);
                }
                strcat(prefix, "a" STD_SUB_2 STD_SPACE_4_PER_EM "=" STD_SPACE_HAIR);
                prefixWidth = stringWidth(prefix, &standardFont, prefixPre, prefixPost) + 1;
              }
            }
            else {
                if(regist == REGISTER_X) {
                  displayLRtemporaryInformation("y" STD_SPACE_4_PER_EM "=" STD_SPACE_4_PER_EM, "", prefix, "a" STD_SUB_0, prefixPre, prefixPost, &prefixWidth);
                }
                else if(regist == REGISTER_Y) {
                strcpy(prefix, eatSpacesEnd(getCurveFitModeName(lrChosen)));
                if(lrCountOnes(lrSelection)>1) {
                  strcat(prefix,lrChosen == 0 ? "" : STD_SUP_ASTERISK);
                }
                while(stringWidth(prefix, &standardFont, prefixPre, prefixPost) + 1 < LRWidth) {
                  strcat(prefix,STD_SPACE_6_PER_EM);
                }
                strcat(prefix, "a" STD_SUB_1 STD_SPACE_4_PER_EM "=" STD_SPACE_HAIR);
                prefixWidth = stringWidth(prefix, &standardFont, prefixPre, prefixPost) + 1;
              }
            }
          }

          //else if(temporaryInformation == TI_SXY) {
          //  if(regist == REGISTER_X) {
          //    strcpy(prefix, "s" STD_SUB_x STD_SUB_y " =");
          //    prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
          //  }
          //}

          //else if(temporaryInformation == TI_COV) {
          //  if(regist == REGISTER_X) {
          //    strcpy(prefix, "s" STD_SUB_m STD_SUB_w " =");
          //    prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
          //  }
          //}

          else if(temporaryInformation == TI_CALCY) {
            if(regist == REGISTER_X) {
              prefix[0] = 0;
              if(lrChosen != 0) {
                strcpy(prefix,eatSpacesEnd(getCurveFitModeName(lrChosen)));
                if(lrCountOnes(lrSelection)>1) {
                  strcat(prefix,STD_SUP_ASTERISK);
                }
                strcat(prefix, STD_SPACE_FIGURE);
              }
              strcat(prefix, STD_y_CIRC " =");
              prefixWidth = stringWidth(prefix, &standardFont, false,false) + 1;
            }
          }

          else if(temporaryInformation == TI_CALCX) {
            if(regist == REGISTER_X) {
              prefix[0] = 0;
              if(lrChosen != 0) {
                strcpy(prefix,eatSpacesEnd(getCurveFitModeName(lrChosen)));
                if(lrCountOnes(lrSelection)>1) {
                  strcat(prefix,STD_SUP_ASTERISK);
                }
                strcat(prefix, STD_SPACE_FIGURE);
              }
              strcat(prefix, STD_x_CIRC " =");
              prefixWidth = stringWidth(prefix, &standardFont, false,false) + 1;
            }
          }

          else if(temporaryInformation == TI_CALCX2) {
            if(regist == REGISTER_X) {
              prefix[0] = 0;
              if(lrChosen != 0) {
                strcpy(prefix,eatSpacesEnd(getCurveFitModeName(lrChosen)));
                if(lrCountOnes(lrSelection)>1) {
                  strcat(prefix,STD_SUP_ASTERISK);
                }
                strcat(prefix, STD_SPACE_FIGURE);
              }
              strcat(prefix, STD_x_CIRC STD_SUB_1 " =" );
              prefixWidth = stringWidth(prefix, &standardFont, false,false) + 1;
            }
            else {
              if(regist == REGISTER_Y) {
                prefix[0] = 0;
                if(lrChosen != 0) {
                  strcpy(prefix,eatSpacesEnd(getCurveFitModeName(lrChosen)));
                  if(lrCountOnes(lrSelection)>1) {
                    strcat(prefix,STD_SUP_ASTERISK);
                  }
                  strcat(prefix, STD_SPACE_FIGURE);
                }
                strcat(prefix, STD_x_CIRC STD_SUB_2 " =");
                prefixWidth = stringWidth(prefix, &standardFont, false,false) + 1;
              }
            }
          }

          else if(temporaryInformation == TI_CORR) {
            if(regist == REGISTER_X) {
              prefix[0] = 0;
              if(lrChosen != 0) {
                strcpy(prefix,eatSpacesEnd(getCurveFitModeName(lrChosen)));
                if(lrCountOnes(lrSelection)>1) {
                  strcat(prefix,STD_SUP_ASTERISK);
                }
                strcat(prefix, STD_SPACE_FIGURE);
              }
              strcat(prefix, "r =");
              prefixWidth = stringWidth(prefix, &standardFont, false,false) + 1;
            }
          }

          else if(temporaryInformation == TI_SMI) {
            if(regist == REGISTER_X) {
              strcpy(prefix, "s" STD_SUB_m STD_SUB_i " =");
              prefixWidth = stringWidth(prefix, &standardFont, false,false) + 1;
            }
          }
          //L.R. Display

          else if(temporaryInformation == TI_HARMMEANX_HARMMEANY) {
            if(regist == REGISTER_X) {
              strcpy(prefix, STD_x_BAR STD_SUB_H " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            else if(regist == REGISTER_Y) {
              strcpy(prefix, STD_y_BAR STD_SUB_H " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_RMSMEANX_RMSMEANY) {
            if(regist == REGISTER_X) {
              strcpy(prefix, STD_x_BAR STD_SUB_R STD_SUB_M STD_SUB_S " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            else if(regist == REGISTER_Y) {
              strcpy(prefix, STD_y_BAR STD_SUB_R STD_SUB_M STD_SUB_S " =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_STATISTIC_SUMS) {
            _displaySigmaPlus(regist, prefix, &prefixWidth, !noLine);
          }

          else if(temporaryInformation == TI_STATISTIC_LR) {
             if(regist == REGISTER_X) {
               if(orOrtho(lrSelection) == CF_ORTHOGONAL_FITTING) {
                 sprintf(prefix, "L.R. selected to OrthoF");
               }
               else {
                 sprintf(prefix, "L.R. selected to %03" PRIu16, (uint16_t)((lrSelection) & 0x01FF));
               }
               prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
               //drawSinglePixelFullWidthLine(Y_POSITION_OF_REGISTER_Y_LINE - 2);
             }
           }

          else if(temporaryInformation == TI_SOLVER_VARIABLE_RESULT) {
            _displaySolverOutput(regist, prefix, &prefixWidth);
          }
          else if(temporaryInformation == TI_SOLVER_VARIABLE) {
            _displaySolverInput(regist, prefix, &prefixWidth);
          }

          else if(temporaryInformation == TI_ACC) {
            if(regist == REGISTER_X) {
              sprintf(prefix, "ACC" STD_SPACE_FIGURE ":");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_ULIM) {
            if(regist == REGISTER_X) {
              sprintf(prefix, STD_UP_ARROW " Upper limit" STD_SPACE_FIGURE ":");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_LLIM) {
            if(regist == REGISTER_X) {
              sprintf(prefix, STD_DOWN_ARROW " Lower limit" STD_SPACE_FIGURE ":");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_INTEGRAL) {
            if(regist == REGISTER_X) {
              sprintf(prefix, STD_INTEGRAL STD_ALMOST_EQUAL);
              prefixWidth = stringWidth(prefix, &numericFont, true, true) + 1;
            }
            else if(regist == REGISTER_Y) {
              strcpy(prefix, "Accuracy " STD_ALMOST_EQUAL);
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_FUNCTION) {
            if(regist == REGISTER_X) {
              sprintf(prefix, "f =");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_1ST_DERIVATIVE) {
            if(regist == REGISTER_X) {
              sprintf(prefix, "%sf'" STD_ALMOST_EQUAL, errorMessage);
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_2ND_DERIVATIVE) {
            if(regist == REGISTER_X) {
              sprintf(prefix, "%sf\"" STD_ALMOST_EQUAL, errorMessage);
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_VIEW_REGISTER && origRegist == REGISTER_T) {
            viewRegName(prefix, &prefixWidth);
          }

          else if(temporaryInformation == TI_VIEW_REGISTER) {          //X, Y, & Z, not T
            userTI(currentViewRegister, regist, prefix, &prefixWidth);
          }

          else if(temporaryInformation == TI_CONV_MENU_STR && regist == REGISTER_X) {    //convert menu
                strcpy(prefix," ");
                strcat(prefix, errorMessage);
                strcat(prefix, ":");
                prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
          }

          else if(temporaryInformation == TI_ABC || temporaryInformation == TI_ABBCCA || temporaryInformation == TI_012) {                             //JM EE \/
            elecTI(regist, prefix, &prefixWidth);
          }

          else if(temporaryInformation == TI_FROM_DMS) {
            if(regist == REGISTER_X) {
              strcpy(prefix, "decimal" STD_DEGREE ":");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_FROM_HMS) {
            if(regist == REGISTER_X) {
              strcpy(prefix, "decimal h:");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_FROM_MS_TIME) {
            if(regist == REGISTER_X) {
              strcpy(prefix, "hh.mmss:");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_FROM_MS_DEG) {
            if(regist == REGISTER_X) {
              strcpy(prefix, "dd.mmss:");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_TVM_EFF && regist == REGISTER_X) {
            strcpy(prefix, "EFF%/a = EFF%YR = EAR =");
            prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
          }

          else if(temporaryInformation == TI_TVM_IA && regist == REGISTER_X) {
            strcpy(prefix, "I%/a = I%YR = NAR =");
            prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
          }

          else if(temporaryInformation == TI_FROM_DATEX) {
            if(regist == REGISTER_X) {
              if(getSystemFlag(FLAG_DMY)) {
                strcpy(prefix, "dd.mmyyyy:");
              }
              else if(getSystemFlag(FLAG_MDY)) {
                strcpy(prefix, "mm.ddyyyy:");
              }
              else { // YMD
                strcpy(prefix, "yyyy.mmdd:");
              }
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }

          else if(temporaryInformation == TI_LAST_CONST_CATNAME || temporaryInformation == TI_SCATTER_SMI) {
            if(regist == REGISTER_X) {
              strcpy(prefix, lastFuncSoftmenuName());
              if(prefix[0] != 0) {
                strcat(prefix,  " ");
                if(compareString(lastFuncSoftmenuName(), lastFuncCatalogName(), CMP_BINARY) != 0) {
                  char prefix_[16];
                  prefix_[0]=0;
                  strcat(prefix_, lastFuncCatalogName());
                  if(prefix_[0] != 0) {
                    strcat(prefix,prefix_);
                  }
                }
                if(prefix[0] != 0) {
                  strcat(prefix,  " = ");
                }
                prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
              }
            }
          }
          else if((regist == REGISTER_X && (temporaryInformation == TI_MIJ || temporaryInformation == TI_MIJEQ)) || ((regist == REGISTER_X || regist == REGISTER_Y) && temporaryInformation == TI_IJ) || (regist == REGISTER_X && (temporaryInformation == TI_I || temporaryInformation == TI_J))) {
            _displayIJ(regist, prefix, &prefixWidth);
          }
          else if(temporaryInformation == TI_STORCL && regist == REGISTER_X) {
            viewStoRcl(prefix, &prefixWidth);
          }
          else if(temporaryInformation == TI_REGTYPE) {
            _displayRegType(regist, prefix, &prefixWidth);
          }


          if(prefixWidth > 0 && temporaryInformation != TI_VIEW_REGISTER) {
            if(regist == REGISTER_X) {
              showString(prefix, &standardFont, 1, Y_POSITION_OF_REGISTER_X_LINE + TEMPORARY_INFO_OFFSET, vmNormal, true, true);
            }
            else if(regist == REGISTER_Y) {
              showString(prefix, &standardFont, 1, Y_POSITION_OF_REGISTER_Y_LINE + TEMPORARY_INFO_OFFSET, vmNormal, true, true);
            }
            else if(regist == REGISTER_Z) {
              showString(prefix, &standardFont, 1, Y_POSITION_OF_REGISTER_Z_LINE + TEMPORARY_INFO_OFFSET, vmNormal, true, true);
            }
          }
                                                                      //JM EE ^

          real34ToDisplayString(REGISTER_REAL34_DATA(regist), getRegisterAngularMode(regist), tmpString, &numericFont, SCREEN_WIDTH - prefixWidth, NUMBER_OF_DISPLAY_DIGITS, LIMITEXP, FRONTSPACE, FULLIRFRAC);

          w = stringWidth(tmpString, &numericFont, false, true);
          lineWidth = w;
          if(prefixWidth > 0) {
            if(temporaryInformation == TI_INTEGRAL && regist == REGISTER_X) {
              showString(prefix, &numericFont, 1, baseY - checkHPoffset, vmNormal, prefixPre, prefixPost);
            }
            else {
              showString(prefix, &standardFont, 1, baseY - checkHPoffset + TEMPORARY_INFO_OFFSET, vmNormal, prefixPre, prefixPost);
            }
          }
          showString(tmpString, &numericFont, (temporaryInformation == TI_VIEW_REGISTER && origRegist == REGISTER_T) ? prefixWidth : SCREEN_WIDTH - w, baseY - checkHPoffset, vmNormal, false, true);
        }



/*Main type dtComplex34*/
          //JM else if(getRegisterDataType(regist) == dtComplex34) {                                                                                                      //JM EE Removed and replaced with the below
          //JM complex34ToDisplayString(REGISTER_COMPLEX34_DATA(regist), tmpString, &numericFont, SCREEN_WIDTH, NUMBER_OF_DISPLAY_DIGITS, true, STD_SPACE_PUNCTUATION);   //JM EE Removed and replaced with the below
        else if(getRegisterDataType(regist) == dtComplex34) {
          if(temporaryInformation == TI_COPY_FROM_SHOW && regist == REGISTER_X) {
            _fnShowRecallTI(prefix, &prefixWidth);
          }

          else if(temporaryInformation == TI_SOLVER_VARIABLE_RESULT) {
            _displaySolverOutput(regist, prefix, &prefixWidth);
          }
          else if(temporaryInformation == TI_SOLVER_VARIABLE) {
            _displaySolverInput(regist, prefix, &prefixWidth);
          }
          else if(temporaryInformation == TI_VIEW_REGISTER && origRegist == REGISTER_T) {
              viewRegName(prefix, &prefixWidth);
          }
          else if(temporaryInformation == TI_VIEW_REGISTER) {          //X, Y, & Z, not T
            userTI(currentViewRegister, regist, prefix, &prefixWidth);
          }
          else if(temporaryInformation == TI_ABC || temporaryInformation == TI_ABBCCA || temporaryInformation == TI_012) {                             //JM EE \/
            elecTI(regist, prefix, &prefixWidth);
          }

          else if(temporaryInformation == TI_ROOTS3) {
            if(regist == REGISTER_X || regist == REGISTER_Y || regist == REGISTER_Z) {
              strcpy(prefix,"Root" STD_SPACE_FIGURE ":");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            #ifdef DISCRIMINANT
            if(regist == REGISTER_T) {
              strcpy(prefix,STD_UP_ARROW "Discr." STD_SPACE_FIGURE ":");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            #endif //DISCRIMINANT
          }
          else if(temporaryInformation == TI_ROOTS2) {
            if(regist == REGISTER_X || regist == REGISTER_Y) {
              strcpy(prefix,"Root" STD_SPACE_FIGURE ":");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            #ifdef DISCRIMINANT
            if(regist == REGISTER_Z) {
              strcpy(prefix,STD_UP_ARROW "Discr." STD_SPACE_FIGURE ":");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
            #endif //DISCRIMINANT
          }
          else if((regist == REGISTER_X && (temporaryInformation == TI_MIJ || temporaryInformation == TI_MIJEQ)) || ((regist == REGISTER_X || regist == REGISTER_Y) && temporaryInformation == TI_IJ) || (regist == REGISTER_X && (temporaryInformation == TI_I || temporaryInformation == TI_J))) {
            _displayIJ(regist, prefix, &prefixWidth);
          }
          else if(temporaryInformation == TI_STORCL && regist == REGISTER_X) {
            viewStoRcl(prefix, &prefixWidth);
          }


          if(prefixWidth > 0) {
            if(regist == REGISTER_X) {
              showString(prefix, &standardFont, 1, Y_POSITION_OF_REGISTER_X_LINE + TEMPORARY_INFO_OFFSET, vmNormal, true, true);
            }
            else if(regist == REGISTER_Y) {
              showString(prefix, &standardFont, 1, Y_POSITION_OF_REGISTER_Y_LINE + TEMPORARY_INFO_OFFSET, vmNormal, true, true);
            }
            else if(regist == REGISTER_Z) {
              showString(prefix, &standardFont, 1, Y_POSITION_OF_REGISTER_Z_LINE + TEMPORARY_INFO_OFFSET, vmNormal, true, true);
            }
          }
                                                                       //JM EE ^
          complex34ToDisplayString(REGISTER_COMPLEX34_DATA(regist), tmpString, &numericFont, SCREEN_WIDTH - prefixWidth, NUMBER_OF_DISPLAY_DIGITS, LIMITEXP, FRONTSPACE, FULLIRFRAC, getComplexRegisterAngularMode(regist),  getComplexRegisterPolarMode(regist) == amPolar);

          w = stringWidth(tmpString, &numericFont, false, true);
          lineWidth = w;
          if(prefixWidth > 0) {
            showString(prefix, &standardFont, 1, baseY + TEMPORARY_INFO_OFFSET, vmNormal, prefixPre, prefixPost);
          }
          showString(tmpString, &numericFont, (temporaryInformation == TI_VIEW_REGISTER && origRegist == REGISTER_T) ? prefixWidth : SCREEN_WIDTH - w, baseY - checkHPoffset, vmNormal, false, true);
        }




/*Main type dtString*/
        else if(getRegisterDataType(regist) == dtString) {
          if(temporaryInformation == TI_COPY_FROM_SHOW && regist == REGISTER_X) {
            _fnShowRecallTI(prefix, &prefixWidth);
          }

          else if(temporaryInformation == TI_VIEW_REGISTER && origRegist == REGISTER_T) {
            viewRegName(prefix, &prefixWidth);
          }
          else if(temporaryInformation == TI_VIEW_REGISTER) {          //X, Y, & Z, not T
            userTI(currentViewRegister, regist, prefix, &prefixWidth);
          }
          else if(temporaryInformation == TI_STORCL && regist == REGISTER_X) {
            viewStoRcl(prefix, &prefixWidth);
          }
          else if(temporaryInformation == TI_LASTSTATEFILE) {
               clearRegisterLine(REGISTER_Y, true, false);
               strcpy(prefix,"Last full state file loaded:");
               showString(prefix, &standardFont, 1, Y_POSITION_OF_REGISTER_Y_LINE, vmNormal, prefixPre, prefixPost);
               prefix[0]=0;
               prefixWidth = 0;
          }
          else if(isShiftOffset && regist == REGISTER_T) {
             strcpy(prefix,"  ");
             prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
          }


          if(prefixWidth > 0) {
            showString(prefix, &standardFont, 1, baseY + TEMPORARY_INFO_OFFSET, vmNormal, prefixPre, prefixPost);
          }

          //JM REGISTER STRING LARGE FONTS
          #if defined(STACK_X_STR_LRG_FONT)
            //This is for X
            w = stringWidthWithLimitC47(REGISTER_STRING_DATA(regist), stdnumEnlarge, nocompress, SCREEN_WIDTH, false, true);
            if(temporaryInformation != TI_VIEW_REGISTER && regist == REGISTER_X && w < SCREEN_WIDTH) {
              lineWidth = w; //slighly incorrect if special characters are there as well.
              showStringC47(REGISTER_STRING_DATA(regist), stdnumEnlarge, nocompress, SCREEN_WIDTH - w, Y_POSITION_OF_REGISTER_X_LINE + 6 - checkHPoffset, vmNormal, false, true);
            }
            else                                                                   //JM
          #endif // STACK_X_STR_LRG_FONT

          #if defined(STACK_X_STR_MED_FONT)
            //This is for X
            if(temporaryInformation != TI_VIEW_REGISTER && regist == REGISTER_X && (w = stringWidthWithLimitC47(REGISTER_STRING_DATA(regist), numHalf, nocompress, SCREEN_WIDTH, false, true)) < SCREEN_WIDTH) {
              lineWidth = w;
              showStringC47(REGISTER_STRING_DATA(regist), numHalf, nocompress, SCREEN_WIDTH - w, Y_POSITION_OF_REGISTER_X_LINE + 6 - checkHPoffset, vmNormal, false, true);
            }
            else                                                                   //JM
          #endif //STACK_X_STR_MED_FONT

          #if defined(STACK_STR_MED_FONT)
            //This is for Y, Z & T
            if(regist >= REGISTER_Y && regist <= REGISTER_T && (w = stringWidthWithLimitC47(REGISTER_STRING_DATA(regist), numHalf, nocompress, SCREEN_WIDTH - prefixWidth, false, true)) < SCREEN_WIDTH - prefixWidth) {
              lineWidth = w;
              showStringC47(REGISTER_STRING_DATA(regist), numHalf, nocompress, SCREEN_WIDTH - w, baseY + 6 - checkHPoffset, vmNormal, false, true);
            }
            else                                                                   //JM
          #endif // STACK_STR_MED_FONT
            //JM ^^ large fonts


          {
            //printf("^^^^#### combinationFonts=%d maxiC=%d miniC=%d displaymode=%d\n", combinationFonts, maxiC, miniC, displaymode);
            w = stringWidth(REGISTER_STRING_DATA(regist), &standardFont, false, true);
            if(w >= SCREEN_WIDTH - prefixWidth) {
              char *tmpStrW;
              if(regist == REGISTER_X || (temporaryInformation == TI_VIEW_REGISTER && origRegist == REGISTER_T)) {
                xcopy(tmpString, REGISTER_STRING_DATA(regist), stringByteLength(REGISTER_STRING_DATA(regist)) + 1);
                if(temporaryInformation == TI_VIEW_REGISTER && origRegist == REGISTER_T) {
                  createSubstrings(1);
                }
                tmpStrW = stringAfterPixels(tmpString, &standardFont, SCREEN_WIDTH - prefixWidth - 1, false, true);
                *tmpStrW = 0;
                w = stringWidth(tmpString, &standardFont, false, true);
                if(temporaryInformation == TI_VIEW_REGISTER && origRegist == REGISTER_T) {
                  showString(tmpString, &standardFont, prefixWidth     , Y_POSITION_OF_REGISTER_T_LINE - 3, vmNormal, false, true);
                }
                else {
                  showString(tmpString, &standardFont, SCREEN_WIDTH - w, Y_POSITION_OF_REGISTER_X_LINE - 3 - checkHPoffset, vmNormal, false, true);
                }

                w = stringByteLength(tmpString);
                xcopy(tmpString, REGISTER_STRING_DATA(regist), stringByteLength(REGISTER_STRING_DATA(regist)) + 1);
                if(temporaryInformation == TI_VIEW_REGISTER && origRegist == REGISTER_T) {
                  createSubstrings(1);
                }
                xcopy(tmpString, tmpString + w, stringByteLength(tmpString + w) + 1);

                w = stringWidth(tmpString, &standardFont, false, true);
                if(w >= SCREEN_WIDTH - prefixWidth) {
                  tmpStrW = stringAfterPixels(tmpString, &standardFont, SCREEN_WIDTH - prefixWidth - 14 - 1, false, true); // 14 is the width of STD_ELLIPSIS
                  xcopy(tmpStrW, STD_ELLIPSIS, 3);
                  w = stringWidth(tmpString, &standardFont, false, true);
                }
                if(temporaryInformation == TI_VIEW_REGISTER && origRegist == REGISTER_T) {
                  showString(tmpString, &standardFont, prefixWidth     , Y_POSITION_OF_REGISTER_T_LINE + 18, vmNormal, false, true);
                }
                else {
                  showString(tmpString, &standardFont, SCREEN_WIDTH - w, Y_POSITION_OF_REGISTER_X_LINE + 18 - checkHPoffset, vmNormal, false, true);
                }
              }
              else {
                xcopy(tmpString, REGISTER_STRING_DATA(regist), stringByteLength(REGISTER_STRING_DATA(regist)) + 1);
                tmpStrW = stringAfterPixels(tmpString, &standardFont, SCREEN_WIDTH - prefixWidth - 14 - 1, false, true); // 14 is the width of STD_ELLIPSIS
                xcopy(tmpStrW, STD_ELLIPSIS, 3);
                w = stringWidth(tmpString, &standardFont, false, true);
                lineWidth = w;
                showString(tmpString, &standardFont, SCREEN_WIDTH - w, baseY + 6 - checkHPoffset, vmNormal, false, true);
              }
            }
            else {
              lineWidth = w;
              xcopy(tmpString, REGISTER_STRING_DATA(regist), stringByteLength(REGISTER_STRING_DATA(regist)) + 1);
              if(temporaryInformation == TI_VIEW_REGISTER && origRegist == REGISTER_T) {
                createSubstrings(1);
              }
              if(temporaryInformation == TI_VIEW_REGISTER && origRegist == REGISTER_T) {
                showString(tmpString, &standardFont, prefixWidth     , baseY + TEMPORARY_INFO_OFFSET, vmNormal, false, true);
              }
              else {
                showString(tmpString, &standardFont, SCREEN_WIDTH - w, baseY + 6                    - checkHPoffset, vmNormal, false, true);
              }
            }
          }
        }


/*Main type dtShortInteger*/
        else if(getRegisterDataType(regist) == dtShortInteger) {
          {
            shortIntegerToDisplayString(regist, tmpString, true, noBaseOverride);
            showString(tmpString, fontForShortInteger, SCREEN_WIDTH - stringWidth(tmpString, fontForShortInteger, false, true), baseY + (fontForShortInteger == &standardFont ? 6 : 0) - (fontForShortInteger == &numericFont ? checkHPoffset : 0), vmNormal, false, true);

            if(regist == REGISTER_X) {
              displayBaseMode(regist);
              displayTrueFalse(regist);
            }
            if(temporaryInformation == TI_VIEW_REGISTER && origRegist == REGISTER_T) {
              lcd_fill_rect(0,Y_POSITION_OF_REGISTER_T_LINE, 50, REGISTER_LINE_HEIGHT, LCD_SET_VALUE);
            }
          }

          if(!(temporaryInformation == TI_NO_INFO && currentInputVariable != INVALID_VARIABLE)) {
            prefix[0] = 0;
          }
          tmpString[0]=0;
          if(regist == REGISTER_X && (temporaryInformation == TI_DATA_LOSS || temporaryInformation == TI_DATA_NEG_OVRFL)) {
            // show Overflow indication for current X register operation
            shortIntegerToDisplayString(regist, tmpString, true, noBaseOverride);
            if(temporaryInformation == TI_DATA_LOSS) {
              sprintf(prefix, "Ovrfl>%ubits:", shortIntegerWordSize);
            }
            else if(temporaryInformation == TI_DATA_NEG_OVRFL) {
              sprintf(prefix, "Ovrfl<0:");
            }
            prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            if(prefixWidth + stringWidth(tmpString, fontForShortInteger, true, true) + 1 > SCREEN_WIDTH) {
              sprintf(prefix, "OF");
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }
          else if(temporaryInformation == TI_VIEW_REGISTER && origRegist == REGISTER_T) {
            viewRegName(prefix, &prefixWidth);
          }
          else if(temporaryInformation == TI_VIEW_REGISTER) {          //X, Y, & Z, not T
            userTI(currentViewRegister, regist, prefix, &prefixWidth);
          }

          else if(temporaryInformation == TI_COPY_FROM_SHOW && regist == REGISTER_X) {
            _fnShowRecallTI(prefix, &prefixWidth);
          }
          else if(temporaryInformation == TI_STATISTIC_SUMS) {
            _displaySigmaPlus(regist, prefix, &prefixWidth, !noLine);
          }
          else if(temporaryInformation == TI_STORCL && regist == REGISTER_X) {
            viewStoRcl(prefix, &prefixWidth);
          }
          if(prefixWidth > 0) {
            if(regist == REGISTER_X) {
              showString(prefix, &standardFont, 1,
                baseY + TEMPORARY_INFO_OFFSET, vmNormal, prefixPre, prefixPost);
            }
            if(tmpString[0] != 0) {
              shortIntegerToDisplayString(regist, tmpString, true, noBaseOverride);
            }
            showString(tmpString, fontForShortInteger, SCREEN_WIDTH - stringWidth(tmpString, fontForShortInteger, false, true), baseY + (fontForShortInteger == &standardFont ? 6 : 0) - (fontForShortInteger == &numericFont ? checkHPoffset : 0), vmNormal, false, true);
          }
      }

/*Main type dtLongInteger*/
        else if(getRegisterDataType(regist) == dtLongInteger) {
          if(!(temporaryInformation == TI_NO_INFO && currentInputVariable != INVALID_VARIABLE)) {
            prefix[0] = 0;
          }

          if(DBASEMODE) displayBaseMode(regist);

          if(temporaryInformation == TI_COPY_FROM_SHOW && regist == REGISTER_X) {
            _fnShowRecallTI(prefix, &prefixWidth);
          }

          else if(temporaryInformation == TI_SOLVER_VARIABLE) {
            _displaySolverInput(regist, prefix, &prefixWidth);
          }
          else if((regist == REGISTER_X && (temporaryInformation == TI_MIJ || temporaryInformation == TI_MIJEQ)) || ((regist == REGISTER_X || regist == REGISTER_Y) && temporaryInformation == TI_IJ) || (regist == REGISTER_X && (temporaryInformation == TI_I || temporaryInformation == TI_J))) {
            _displayIJ(regist, prefix, &prefixWidth);
          }
          else if(temporaryInformation == TI_STORCL && regist == REGISTER_X) {
            viewStoRcl(prefix, &prefixWidth);
          }
          else if(temporaryInformation == TI_REGTYPE) {
            _displayRegType(regist, prefix, &prefixWidth);
          }
          else if(temporaryInformation == TI_ABC || temporaryInformation == TI_ABBCCA || temporaryInformation == TI_012) {                             //JM EE \/
            elecTI(regist, prefix, &prefixWidth);
          }
          else if(temporaryInformation == TI_STATISTIC_SUMS) {
            _displaySigmaPlus(regist, prefix, &prefixWidth, !noLine);
          }
          else if(temporaryInformation == TI_VIEW_REGISTER && origRegist == REGISTER_T) {
            viewRegName(prefix, &prefixWidth);
          }
          else if(temporaryInformation == TI_VIEW_REGISTER) {          //X, Y, & Z, not T
            userTI(currentViewRegister, regist, prefix, &prefixWidth);
          }
          else if(temporaryInformation == TI_DAY_OF_WEEK) {
            if(regist == REGISTER_X) {
              int day;
              longInteger_t li;
              getRegisterAsLongInt(REGISTER_X, li, NULL); // Cannot fail as REGISTER_X is a dtLongInteger
              longIntegerToInt32(li, day);
              longIntegerFree(li);
              if(day < 1 || day > 7) {
                day = 0;
              }
              strcpy(prefix,"[ISO day] ");
              strcat(prefix, nameOfWday_en[day].itemName);
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }


          //shift longinter prefix on by two space if interfering with the shift indicator, when SB_TIME is selected
          if(regist == REGISTER_T && isShiftOffset) {
           int len = strlen(prefix);
           if(len + 2 < 200) {
             if(prefix[0] == 0) {
               strcpy(prefix,"  ");
               prefixWidth += 20; //stringWidth("  ", &standardFont, true, true) - 2;
             } else {
               for(int i = len; i >= 0; i--) {
                 prefix[i + 2] = prefix[i];
               }
               prefix[0] = ' ';
               prefix[1] = ' ';
               prefixWidth += 20; //stringWidth("  ", &standardFont, true, true) - 2;
              }
            }
          }

        //This section to display long integers as reals
          if(getSystemFlag(FLAG_DREAL)) {
            strcpy(tmpString,STD_INTEGER_Z_SMALL ": ");// STD_SPACE_4_PER_EM);
            w = stringWidth(tmpString, getSystemFlag(FLAG_LARGELI) ? &numericFont : &standardFont, false, true);
            int16_t tlen =stringByteLength(tmpString);
            longIntegerRegisterToRealDisplayString(regist, tmpString+tlen, TMP_STR_LENGTH-tlen, SCREEN_WIDTH - prefixWidth - w, 0, toRemoveTrailingRadix);
            tmpString[TMP_STR_LENGTH-1] = tmpString[tlen];
          }

        //for the 2^10 UNIT diplay, display long integers in real string, with the Ti suffic
          else if(getSystemFlag(FLAG_2TO10) && displayFormat == DF_UN) {
            strcpy(tmpString,STD_INTEGER_Z_SMALL ": ");// STD_SPACE_4_PER_EM);
            w = stringWidth(tmpString, &standardFont, false, true);
            int16_t tlen =stringByteLength(tmpString);
            longIntegerRegisterToRealDisplayString(regist, tmpString+tlen, TMP_STR_LENGTH-tlen, SCREEN_WIDTH - prefixWidth - w, 1024, !toRemoveTrailingRadix);
            tmpString[TMP_STR_LENGTH-1] = tmpString[tlen];
          }

        //normal longinteger handling
          else {
            longIntegerRegisterToDisplayString(regist, tmpString, TMP_STR_LENGTH, SCREEN_WIDTH - prefixWidth, 50, getSystemFlag(FLAG_LARGELI));
            tmpString[TMP_STR_LENGTH-1] = tmpString[0];

            {
              //This section to display too long integers as reals, re-doing the above string in Real. The wastage is minor compared to the Real processing that will follow
              uint16_t pos;
              if(findTwoChars(tmpString, (uint8_t)PRODUCT_SIGN[0], (uint8_t)PRODUCT_SIGN[1], &pos)) {
                strcpy(tmpString,STD_INTEGER_Z_SMALL ": ");// STD_SPACE_4_PER_EM);
                w = stringWidth(tmpString, getSystemFlag(FLAG_LARGELI) ? &numericFont : &standardFont, false, true);
                int16_t tlen =stringByteLength(tmpString);
                uint8_t savedDisplayFormat = displayFormat, savedDisplayFormatDigits = displayFormatDigits;
                displayFormatDigits = 20;
                displayFormat = DF_SCI;
                longIntegerRegisterToRealDisplayString(regist, tmpString+tlen, TMP_STR_LENGTH-tlen, SCREEN_WIDTH - prefixWidth - w, 0, toRemoveTrailingRadix);
                displayFormat = savedDisplayFormat; displayFormatDigits = savedDisplayFormatDigits;
                tmpString[TMP_STR_LENGTH-1] = tmpString[tlen];
              }
            }

          }




          w = stringWidth(tmpString, &numericFont, false, true);
          lineWidth = w;
          if(prefixWidth > 0) {
            showString(prefix, &standardFont, 1, baseY + TEMPORARY_INFO_OFFSET, vmNormal, prefixPre, prefixPost);
          }
          if(w <= SCREEN_WIDTH) {
            showString(tmpString, &numericFont, (temporaryInformation == TI_VIEW_REGISTER && origRegist == REGISTER_T) ? prefixWidth : SCREEN_WIDTH - w, baseY - checkHPoffset, vmNormal, false, true);
          }
          else {
            w = stringWidth(tmpString, &standardFont, false, true);
            if(w > SCREEN_WIDTH) {
              #if (EXTRA_INFO_ON_CALC_ERROR == 1)
                moreInfoOnError("In function _refreshRegisterLine:", "Long integer representation too wide!", tmpString, NULL);
              #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
              strcpy(tmpString, "Long integer representation too wide!");
            }
            w = stringWidth(tmpString, &standardFont, false, true);
            lineWidth = w;
            showString(tmpString, &standardFont, (temporaryInformation == TI_VIEW_REGISTER && origRegist == REGISTER_T) ? prefixWidth : SCREEN_WIDTH - w, baseY + 6, vmNormal, false, true);
          }
        }

/*Main type dtTime*/
        else if(getRegisterDataType(regist) == dtTime) {
          if(temporaryInformation == TI_COPY_FROM_SHOW && regist == REGISTER_X) {
            _fnShowRecallTI(prefix, &prefixWidth);
          }

          else if(temporaryInformation == TI_VIEW_REGISTER && origRegist == REGISTER_T) {
            viewRegName(prefix, &prefixWidth);
          }
          else if(temporaryInformation == TI_VIEW_REGISTER) {          //X, Y, & Z, not T
            userTI(currentViewRegister, regist, prefix, &prefixWidth);
          }
          timeToDisplayString(regist, tmpString, false);
          w = stringWidth(tmpString, &numericFont, false, true);
          if(prefixWidth > 0) {
            showString(prefix, &standardFont, 1, baseY + TEMPORARY_INFO_OFFSET, vmNormal, prefixPre, prefixPost);
          }
          showString(tmpString, &numericFont, (temporaryInformation == TI_VIEW_REGISTER && origRegist == REGISTER_T) ? prefixWidth : SCREEN_WIDTH - w, baseY - checkHPoffset, vmNormal, false, true);
        }

/*Main type dtDate*/
        else if(getRegisterDataType(regist) == dtDate) {
          if(temporaryInformation == TI_COPY_FROM_SHOW && regist == REGISTER_X) {
            _fnShowRecallTI(prefix, &prefixWidth);
          }
          else if(temporaryInformation != TI_VIEW_REGISTER /*== TI_DAY_OF_WEEK*/) { // Change to ignore TI_DAY_OF_WEEK as TI, and permanently display the weekday on registers X, Y & Z
            if(regist >= REGISTER_X && regist <= REGISTER_T) {
              if(!(temporaryInformation == TI_NO_INFO && currentInputVariable != INVALID_VARIABLE) || regist != REGISTER_X) {
                prefix[0] = 0;
              }
              if(isShiftOffset && regist == REGISTER_T){
                strcpy(prefix,"  ");
              }
              strcat(prefix, nameOfWday_en[getJulianDayOfWeek(regist)].itemName);
              prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
            }
          }
          else if(temporaryInformation == TI_VIEW_REGISTER && origRegist == REGISTER_T) {
            viewRegName(prefix, &prefixWidth);
            strcat(prefix, nameOfWday_en[getJulianDayOfWeek(regist)].itemName);
            strcat(prefix," ");
            prefixWidth = stringWidth(prefix, &standardFont, true, true) + 1;
          }
          else if(temporaryInformation == TI_VIEW_REGISTER) {          //X, Y, & Z, not T
            userTI(currentViewRegister, regist, prefix, &prefixWidth);
          }

          dateToDisplayString(regist, tmpString);
          w = stringWidth(tmpString, &numericFont, false, true);
          if(prefixWidth > 0) {
            showString(prefix, &standardFont, 1, baseY + TEMPORARY_INFO_OFFSET, vmNormal, prefixPre, prefixPost);
          }
          showString(tmpString, &numericFont, (temporaryInformation == TI_VIEW_REGISTER && origRegist == REGISTER_T) ? prefixWidth : SCREEN_WIDTH - w, baseY - checkHPoffset, vmNormal, false, true);
        }

/*Main type dtConfig*/
        else if(getRegisterDataType(regist) == dtConfig) {
          if(temporaryInformation == TI_COPY_FROM_SHOW && regist == REGISTER_X) {
            _fnShowRecallTI(prefix, &prefixWidth);
          }
          if(temporaryInformation == TI_VIEW_REGISTER && origRegist == REGISTER_T) {
            viewRegName(prefix, &prefixWidth);
          }
          else if(temporaryInformation == TI_VIEW_REGISTER) {          //X, Y, & Z, not T
            userTI(currentViewRegister, regist, prefix, &prefixWidth);
          }
          xcopy(tmpString, "Configuration data", 19);
          w = stringWidth(tmpString, &numericFont, false, true);
          lineWidth = w;
          if(prefixWidth > 0) {
            showString(prefix, &standardFont, 1, baseY + TEMPORARY_INFO_OFFSET, vmNormal, prefixPre, prefixPost);
          }
          showString(tmpString, &numericFont, (temporaryInformation == TI_VIEW_REGISTER && origRegist == REGISTER_T) ? prefixWidth : SCREEN_WIDTH - w, baseY - checkHPoffset, vmNormal, false, true);
        }

/*Main type dtReal34Matrix*/
        else if(getRegisterDataType(regist) == dtReal34Matrix) {
          if((origRegist == REGISTER_X && calcMode != CM_MIM) || (temporaryInformation == TI_VIEW_REGISTER && origRegist == REGISTER_T)) {
            real34Matrix_t matrix;
            prefixWidth = 0; prefix[0] = 0;
            linkToRealMatrixRegister(regist, &matrix);
            if(temporaryInformation == TI_VIEW_REGISTER && origRegist == REGISTER_T) {
              viewRegName(prefix, &prefixWidth);
            }
            showRealMatrix(&matrix, prefixWidth, toDisplayVectorMatrix);
            if(lastErrorCode != 0) {
              refreshRegisterLine(errorMessageRegisterLine);
            }
            else if((regist == REGISTER_X && (temporaryInformation == TI_MIJ || temporaryInformation == TI_MIJEQ)) || ((regist == REGISTER_X || regist == REGISTER_Y) && temporaryInformation == TI_IJ) || (regist == REGISTER_X && (temporaryInformation == TI_I || temporaryInformation == TI_J))) {
              _displayIJ(regist, prefix, &prefixWidth);
            }
            else if(temporaryInformation == TI_STORCL && regist == REGISTER_X) {
              viewStoRcl(prefix, &prefixWidth);
            }
            else if(temporaryInformation == TI_VIEW_REGISTER && regist == REGISTER_X) {          //X, not T
              userTI(currentViewRegister, regist, prefix, &prefixWidth);
            }
            else if(temporaryInformation == TI_STATISTIC_SUMS) {
              _displaySigmaPlus(regist, prefix, &prefixWidth, noLine);
            }
            else if(temporaryInformation == TI_NO_INFO && currentInputVariable != INVALID_VARIABLE) {
              inputRegName(prefix, &prefixWidth);
            }

            if(temporaryInformation == TI_TRUE || temporaryInformation == TI_FALSE) {
              refreshRegisterLine(TRUE_FALSE_REGISTER_LINE);
            }
            if(prefixWidth > 0) {
              showString(prefix, &standardFont, 1, baseY + TEMPORARY_INFO_OFFSET, vmNormal, prefixPre, prefixPost);
            }
          }
          else {
            if(temporaryInformation == TI_VIEW_REGISTER) {          // Y, & Z, not T
              userTI(currentViewRegister, regist, prefix, &prefixWidth);
            }

            if(prefixWidth > 0) {
              showString(prefix, &standardFont, 1, baseY + TEMPORARY_INFO_OFFSET, vmNormal, prefixPre, prefixPost);
            }


            char preserveErrorMessage[ERROR_MESSAGE_LENGTH];
            xcopy(preserveErrorMessage,errorMessage,ERROR_MESSAGE_LENGTH);   // maintain the errormessage string, which is used for TI's earlier.
            if(!vectorToDisplayString(regist, tmpString)) {                  //   errorMessage string used
              real34MatrixToDisplayString(regist, tmpString);
            }
            xcopy(errorMessage,preserveErrorMessage,ERROR_MESSAGE_LENGTH);   // maintain the errormessage string, which is used for TI's earlier.



            w = stringWidth(tmpString, &numericFont, false, true);
            lineWidth = w;
            if(w > SCREEN_WIDTH - 1) {
              w = stringWidth(tmpString, &standardFont, false, true);
              //Iteration to place ellipsis by eating away the left hand digtis not needed. This will be needed, if the maximum vector digits is increased to more than 9 fixed digits
              showString(tmpString, &standardFont, SCREEN_WIDTH - w - 0 + 2, baseY, vmNormal, false, true);
            } else {
              showString(tmpString, &numericFont, SCREEN_WIDTH - w - 1, baseY, vmNormal, false, true);
            }
          }

          if(temporaryInformation == TI_INACCURATE && regist == REGISTER_X) {
            showString("This result may be inaccurate", &standardFont, 1, Y_POSITION_OF_ERR_LINE, vmNormal, true, true);
          }
        }

/*Main type dtComplex34Matrix*/
        else if(getRegisterDataType(regist) == dtComplex34Matrix) {
          if((origRegist == REGISTER_X && calcMode != CM_MIM) || (temporaryInformation == TI_VIEW_REGISTER && origRegist == REGISTER_T)) {
            complex34Matrix_t matrix;
            prefixWidth = 0; prefix[0] = 0;
            linkToComplexMatrixRegister(regist, &matrix);
            if(temporaryInformation == TI_VIEW_REGISTER && origRegist == REGISTER_T) {
              viewRegName(prefix, &prefixWidth);
            }
            showComplexMatrix(&matrix, prefixWidth, getComplexRegisterAngularMode(regist), getComplexRegisterPolarMode(regist) == amPolar);
            if(lastErrorCode != 0) {
              refreshRegisterLine(errorMessageRegisterLine);
            }
            else if((regist == REGISTER_X && (temporaryInformation == TI_MIJ || temporaryInformation == TI_MIJEQ)) || ((regist == REGISTER_X || regist == REGISTER_Y) && temporaryInformation == TI_IJ) || (regist == REGISTER_X && (temporaryInformation == TI_I || temporaryInformation == TI_J))) {
              _displayIJ(regist, prefix, &prefixWidth);
            }
            else if(temporaryInformation == TI_STORCL && regist == REGISTER_X) {
              viewStoRcl(prefix, &prefixWidth);
            }
            else if(temporaryInformation == TI_VIEW_REGISTER && regist == REGISTER_X) {          //X, not T
              userTI(currentViewRegister, regist, prefix, &prefixWidth);
            }
            else if(temporaryInformation == TI_NO_INFO && currentInputVariable != INVALID_VARIABLE) {
              inputRegName(prefix, &prefixWidth);
            }

            if(temporaryInformation == TI_TRUE || temporaryInformation == TI_FALSE) {
              refreshRegisterLine(TRUE_FALSE_REGISTER_LINE);
            }
            if(prefixWidth > 0) {
              showString(prefix, &standardFont, 1, baseY + TEMPORARY_INFO_OFFSET, vmNormal, prefixPre, prefixPost);
            }
          }
          else {
            if(temporaryInformation == TI_VIEW_REGISTER) {          // Y, & Z, not T
              userTI(currentViewRegister, regist, prefix, &prefixWidth);
            }

            if(prefixWidth > 0) {
              showString(prefix, &standardFont, 1, baseY + TEMPORARY_INFO_OFFSET, vmNormal, prefixPre, prefixPost);
            }
            complex34MatrixToDisplayString(regist, tmpString);
            w = stringWidth(tmpString, &numericFont, false, true);
            lineWidth = w;
            showString(tmpString, &numericFont, SCREEN_WIDTH - w - 2, baseY, vmNormal, false, true);
          }

          if(temporaryInformation == TI_INACCURATE && regist == REGISTER_X) {
            showString("This result may be inaccurate", &standardFont, 1, Y_POSITION_OF_ERR_LINE, vmNormal, true, true);
          }
        }

        else {
          sprintf(tmpString, "Displaying %s: to be coded!", getRegisterDataTypeName(regist, true, false));
          showString(tmpString, &standardFont, SCREEN_WIDTH - stringWidth(tmpString, &standardFont, false, true), baseY + 6, vmNormal, false, true);
        }

        if(temporaryInformation == TI_VIEW_REGISTER && origRegist == REGISTER_X) {
          regist = REGISTER_X;
        }
      }

      if(regist == REGISTER_T) {
        lineTWidth = lineWidth;
      }
    }

    if(getRegisterDataType(REGISTER_X) == dtReal34Matrix || getRegisterDataType(REGISTER_X) == dtComplex34Matrix || calcMode == CM_MIM || distModeActive || BASEMODEACTIVE || XXFNMODEACTIVE) {
      displayStack = origDisplayStack;
    }
  }

  void refreshRegisterLine(calcRegister_t regist) {
    _refreshRegisterLine(regist, !RESTORE_T);
  }

  static void refreshRegisterLineRestoreT(void) {
    _refreshRegisterLine(REGISTER_T, RESTORE_T);
  }

  static void _showAngularModeGlyph(angularMode_t angularMode, const font_t *font, uint32_t x, uint32_t y) {
    switch(angularMode) {
      case amMultPi: {
        showString(STD_SUP_pir, font, x, y, vmNormal, true, true);
        break;
      }
      case amRadian: {
        showString(STD_SUP_BOLD_r, font, x, y, vmNormal, true, true);
        break;
      }
      case amGrad: {
        showString(STD_SUP_BOLD_g, font, x, y, vmNormal, true, true);
        break;
      }
      case amDegree: {
        showString(STD_DEGREE, font, x, y, vmNormal, true, true);
        break;
      }
      case amSecond: {
        showString("s", font, x, y, vmNormal, true, true);
        break;
      }
      default: {
      }
    }
  }


  void displayNim(const char *nim, const char *lastBase, int16_t wLastBaseNumeric, int16_t wLastBaseStandard) {
    int16_t w;
    angularMode_t xangularMode = getRegisterAngularMode(REGISTER_X);
    if(stringWidth(nim, &numericFont, true, true) + wLastBaseNumeric <= SCREEN_WIDTH - 16) { // 16 is the numeric font cursor width
      xCursor = showString(nim, &numericFont, 0, Y_POSITION_OF_NIM_LINE - checkHPoffset, vmNormal, true, true);
      yCursor = Y_POSITION_OF_NIM_LINE;
      cursorFont = &numericFont;

      if(lastIntegerBase != 0 || (aimBuffer[0] != 0 && aimBuffer[strlen(aimBuffer)-1]=='/')) {
        showString(lastBase, &numericFont, xCursor + 16, Y_POSITION_OF_NIM_LINE - checkHPoffset, vmNormal, true, true);
      }
      else if((getRegisterDataType(REGISTER_X) == dtReal34) && (xangularMode < amNone)) {
        _showAngularModeGlyph(xangularMode, &numericFont, xCursor + 16, Y_POSITION_OF_NIM_LINE - checkHPoffset);
      }
    }
    else if(stringWidth(nim, &standardFont, true, true) + wLastBaseStandard <= SCREEN_WIDTH - 8) { // 8 is the standard font cursor width
      xCursor = showString(nim, &standardFont, 0, Y_POSITION_OF_NIM_LINE + 6, vmNormal, true, true);
      yCursor = Y_POSITION_OF_NIM_LINE + 6;
      cursorFont = &standardFont;

      if(lastIntegerBase != 0 || (aimBuffer[0] != 0 && aimBuffer[strlen(aimBuffer)-1]=='/')) {
        showString(lastBase, &standardFont, xCursor + 8, Y_POSITION_OF_NIM_LINE + 6, vmNormal, true, true);
      }
      else if((getRegisterDataType(REGISTER_X) == dtReal34) && (xangularMode < amNone)) {
        _showAngularModeGlyph(xangularMode, &standardFont, xCursor + 8, Y_POSITION_OF_NIM_LINE + 6);
      }
    }
    else {
      char *nimw;
      w = stringByteLength(nim) + 1;
      xcopy(tmpString,        nim, w);
      xcopy(tmpString + 1500, nim, w);
      nimw = stringAfterPixels(tmpString, &standardFont, SCREEN_WIDTH - 1, true, true);
      w = nimw - tmpString;
      *nimw = 0;

      if(stringWidth(tmpString + 1500 + w, &standardFont, true, true) + wLastBaseStandard > SCREEN_WIDTH - 8) { // 8 is the standard font cursor width
        addItemToNimBuffer(ITM_BACKSPACE);
      }
      else {
        showString(tmpString, &standardFont, 0, Y_POSITION_OF_NIM_LINE - 3, vmNormal, true, true);

        xCursor = showString(tmpString + 1500 + w, &standardFont, 0, Y_POSITION_OF_NIM_LINE + 18, vmNormal, true, true);
        yCursor = Y_POSITION_OF_NIM_LINE + 18;
        cursorFont = &standardFont;

        if(lastIntegerBase != 0 || (aimBuffer[0] != 0 && aimBuffer[strlen(aimBuffer)-1] == '/')) {
          showString(lastBase, &standardFont, xCursor + 8, Y_POSITION_OF_NIM_LINE + 18, vmNormal, true, true);
        }
        else if((getRegisterDataType(REGISTER_X) == dtReal34) && (xangularMode < amNone)) {
          _showAngularModeGlyph(xangularMode, &standardFont, xCursor + 8, Y_POSITION_OF_NIM_LINE + 18);
        }
      }
    }
  }


  void clearTamBuffer(void) {
    if(temporaryInformation == TI_SHOWNOTHING) return; //to allow a matrix being dispayed without clearing the tam line through it

    if(shiftF || shiftG) {
      //lcd_fill_rect(18, Y_POSITION_OF_TAM_LINE, 120, 32, LCD_SET_VALUE);
      lcd_fill_rect(funcNameOffset_x, Y_POSITION_OF_TAM_LINE, SCREEN_WIDTH - funcNameOffset_x, 32, LCD_SET_VALUE); //JM Clear the whole t-register instead of only 120+18 oixels
    }
    else {
      //lcd_fill_rect(0, Y_POSITION_OF_TAM_LINE, 138, 32, LCD_SET_VALUE);
      lcd_fill_rect(0, Y_POSITION_OF_TAM_LINE, SCREEN_WIDTH, 32, LCD_SET_VALUE); //JM Clear the whole t-register instead of 138
    }
  }

  //conditions where an extra space in T register display is not possible, to prevent for the f/g indicator to clash, we reduce the size of the f/g indicator
  #define useSmallShifts (isShiftOffset && calcMode == CM_NORMAL\
                                       &&  ( ((!BASEMODEACTIVE || displayStackSHOIDISP == 0) &&  getRegisterDataType(REGISTER_T) == dtShortInteger && getRegisterShortIntegerBase(REGISTER_T) < 4)       ||\
                                              ((dispBase > 0)                               && (getRegisterDataType(REGISTER_X) == dtShortInteger || getRegisterDataType(REGISTER_X) == dtLongInteger))   \
                                           ) )
  #define displayF (useSmallShifts ? STD_f : STD_MODE_F)
  #define displayG (useSmallShifts ? STD_g : STD_MODE_G)

  void clearShiftState(void) {
    uint32_t fcol, frow, gcol, grow;
    getGlyphBounds(displayF, 0, &standardFont, &fcol, &frow);
    getGlyphBounds(displayG, 0, &standardFont, &gcol, &grow);
    lcd_fill_rect(X_SHIFT, Y_SHIFT, (fcol > gcol ? fcol : gcol), (frow > grow ? frow : grow), LCD_SET_VALUE);
  }

  void showShiftStateF(void) {
        showGlyph(displayF, &standardFont, X_SHIFT, Y_SHIFT, vmNormal, true, true, false); // f is pixel 4+8+3 wide
  }

  void showShiftStateG(void) {
        showGlyph(displayG, &standardFont, X_SHIFT, Y_SHIFT, vmNormal, true, true, false); // g is pixel 4+10+1 wide
  }


  void displayShiftAndTamBuffer(void) {
    if(calcMode == CM_ASSIGN) {
      updateAssignTamBuffer();
    }

    if(calcMode != CM_ASSIGN || itemToBeAssigned == 0 || tam.alpha) {
      if(shiftF) {
        showShiftStateF();
      }
      else if(shiftG) {
        showShiftStateG();
      }
    }

    if(tam.mode || calcMode == CM_ASSIGN) {
      if(calcMode == CM_PEM) { // Variable line to display TAM informations
        lcd_fill_rect(45+20, tamOverPemYPos, 168, 20, LCD_SET_VALUE);
        showString(tamBuffer, &standardFont, 75+20, tamOverPemYPos, vmNormal,  false, false);
      }
      else { // Fixed line to display TAM informations
        clearTamBuffer();
        showString(tamBuffer, &standardFont, funcNameOffset_x, Y_POSITION_OF_TAM_LINE + 6, vmNormal, true, true);
      }
    }
  }



  void closeShowMenu(void) {
    if(currentMenu() == -MNU_SHOW) {
      popSoftmenu();
    }
    showRegis = 9999;
    calcMode = CM_NORMAL;
    screenUpdatingMode = SCRUPD_AUTO;
    temporaryInformation = TI_NO_INFO;
    refreshScreen(137);
  }


  void reallyClearStatusBar(uint8_t info) {
    lcd_fill_rect(0, 0, (GRAPHMODE ? SCREEN_WIDTH / 3 : SCREEN_WIDTH), Y_POSITION_OF_REGISTER_T_LINE, LCD_SET_VALUE);
    force_SBrefresh(force);
    forceSBupdate();
    refreshStatusBar();
  }


  static void _selectiveClearScreen(void) {
    if(screenUpdatingMode == SCRUPD_AUTO) {
      #if defined(PC_BUILD) && defined(MONITOR_CLRSCR)
        printf("   >>> _selectiveClearScreen: lcd_fill_rect clear all\n");
      #endif // PC_BUILD && MONITOR_CLRSCR
      clearScreen(6);
      refreshStatusBar();
      refreshNIMdone = false;
    }
    else {
      if(!(screenUpdatingMode & (SCRUPD_MANUAL_STATUSBAR | SCRUPD_SKIP_STATUSBAR_ONE_TIME))) {
        #if defined(PC_BUILD) && defined(MONITOR_CLRSCR)
          printf("   >>> _selectiveClearScreen: SCRUPD_MANUAL_STATUSBAR\n");
        #endif // PC_BUILD &&MONITOR_CLRSCR
        clearScreenStatusBar(7);
      }
      else if(!(screenUpdatingMode & SCRUPD_MANUAL_STATUSBAR)) {
        refreshStatusBar();
      }


      //now clear stack area, first the left graph info area, then the actual area covered by the graph if not in graph mode
      if(!(screenUpdatingMode & (SCRUPD_MANUAL_STACK | SCRUPD_SKIP_STACK_ONE_TIME))) {
        #if defined(PC_BUILD) && defined(MONITOR_CLRSCR)
          printf("   >>> _selectiveClearScreen: lcd_fill_rect SCRUPD_MANUAL_STACK | SCRUPD_SKIP_STACK_ONE_TIME\n");
        #endif // PC_BUILD && MONITOR_CLRSCR
        lcd_fill_rect(  LeftGraphInfoX,    topLeftGraphInfoY,          widthGraphInfoBox,    heightGraphInfoBox,   LCD_SET_VALUE);
        refreshNIMdone = false;
        if(!GRAPHMODE) {                                                                                                                   // in GRAPHMODE, protect the square graph area
          lcd_fill_rect(widthGraphInfoBox, topLeftGraphInfoY,          widthGraphInclBorder, heightGraphInfoBox,   LCD_SET_VALUE);
        }
      }


      //now clear menu area, first the left graph info area, then the actual area covered by the graph if not in graph mode
      if(!(screenUpdatingMode & (SCRUPD_MANUAL_MENU | SCRUPD_SKIP_MENU_ONE_TIME))) {
        #if defined(PC_BUILD) && defined(MONITOR_CLRSCR)
          printf("   >>> _selectiveClearScreen: lcd_fill_rect SCRUPD_MANUAL_MENU | SCRUPD_SKIP_MENU_ONE_TIME\n");
        #endif // PC_BUILD && MONITOR_CLRSCR
        lcd_fill_rect(  LeftGraphInfoX,    topLeftMenuInclBorderY,     widthGraphInfoBox,    menuHeightInclBorder, LCD_SET_VALUE);
        if(!GRAPHMODE || menu(0) == -MNU_PLOT_FUNC) {                                                                                      // not in GRAPHMODE, clear the little triangle area indicating more menus
          lcd_fill_rect(LeftGraphInfoX,    topLeftMenuInclBorderY - 3, 20,                   6,                    LCD_SET_VALUE);
        }
        if(!GRAPHMODE) {                                                                                                                   // in GRAPHMODE, protect the square graph area
          lcd_fill_rect(widthGraphInfoBox, topLeftMenuInclBorderY,     widthGraphInclBorder, menuHeightInclBorder, LCD_SET_VALUE);
        }
      }
    }
  }


//#if !defined(TESTSUITE_BUILD)
//  void clearScreenOld(bool_t clearStatusBar, bool_t clearRegisterLines, bool_t clearSoftkeys) {      //JMOLD
//    if(clearStatusBar) {
//      lcd_fill_rect(0, 0, SCREEN_WIDTH, 20, 0);
//    }
//    if(clearRegisterLines) {
//      lcd_fill_rect(0, 20, SCREEN_WIDTH, 151, 0);
//    }
//    if(clearSoftkeys) {
//      clear_ul(); //JMUL
//      lcd_fill_rect(0, 171, SCREEN_WIDTH, 69, 0);
//      lcd_fill_rect(0, 171-5, 20, 5, 0);
//    }
//  }                                                       //JM ^^
//#endif // !TESTSUITE_BUILD


  #if !defined(TESTSUITE_BUILD)  //clearScreenOld(clrStatusBar, clrRegisterLines, clrSoftkeys);
    void clearScreenOld(bool_t clearStatusBar, bool_t clearRegisterLines, bool_t clearSoftkeys) {  //clrStatusBar, clrRegisterLines, clrSoftkeys
                                        #if defined(PC_BUILD) && defined(MONITOR_CLRSCR)
                                          printf("       clearScreenOld calcMode=%u clearStatusBar=%u, clearRegisterLines=%u, clearSoftkeys=%u\n",calcMode, clearStatusBar, clearRegisterLines, clearSoftkeys);
                                        #endif // PC_BUILD &&MONITOR_CLRSCR
                                        #if defined(ANALYSE_REFRESH)
                                          print_caller(NULL);
                                        #endif //ANALYSE_REFRESH
      uint8_t origScreenUpdatingMode = screenUpdatingMode;
      screenUpdatingMode = SCRUPD_AUTO;
      if(clearStatusBar) {
        screenUpdatingMode &= ~SCRUPD_MANUAL_STATUSBAR;
        screenUpdatingMode |=  SCRUPD_MANUAL_STACK;
        screenUpdatingMode |=  SCRUPD_MANUAL_MENU;
        _selectiveClearScreen();
      }
      if(clearRegisterLines) {
        screenUpdatingMode |=  SCRUPD_MANUAL_STATUSBAR;
        screenUpdatingMode &= ~SCRUPD_MANUAL_STACK;
        screenUpdatingMode |=  SCRUPD_MANUAL_MENU;
        _selectiveClearScreen();
      }
      if(clearSoftkeys) {
        screenUpdatingMode |=  SCRUPD_MANUAL_STATUSBAR;
        screenUpdatingMode |=  SCRUPD_MANUAL_STACK;
        screenUpdatingMode &= ~SCRUPD_MANUAL_MENU;
        _selectiveClearScreen();
      }
      screenUpdatingMode = origScreenUpdatingMode;
    }



    void clearScreenGraphs(uint8_t source, bool_t clearTextArea, bool_t clearGraphArea) {
      #if defined(PC_BUILD) && defined(MONITOR_CLRSCR)
        printf("       clearScreenGraphs(%u) clearTextArea=%u, clearGraphArea=%u \n",source, clearTextArea, clearGraphArea);
      #endif // PC_BUILD &&MONITOR_CLRSCR
      uint8_t origCalcMode = calcMode;
      if(clearTextArea) {
        calcMode = CM_GRAPH; //in GRAPHMODE, protect the square graph area
      }
      if(clearGraphArea) {
        reDraw = true;
        calcMode = CM_NORMAL;
      }
      clearScreenOld(clrStatusBar, clrRegisterLines, clrSoftkeys);
      screenUpdatingMode |= SCRUPD_MANUAL_MENU;
      screenUpdatingMode |= SCRUPD_MANUAL_STACK;
      screenUpdatingMode |= SCRUPD_MANUAL_STATUSBAR;
      calcMode = origCalcMode;
    }
  #endif // !TESTSUITE_BUILD


  static void _refreshPemScreen(void) {
      #if defined(PC_BUILD) && defined(MONITOR_CLRSCR)
        printf(">>> BEGIN _refreshPemScreen calcMode=%d previousCalcMode=%d screenUpdatingMode=%d skippedStackLines=%u\n", calcMode, previousCalcMode, screenUpdatingMode, skippedStackLines);    //JMYY
      #endif // PC_BUILD &&MONITOR_CLRSCR
      skippedStackLines = false;                                    // See timer.c skippedStackLines
      #if defined(DMCP_BUILD)
        keyBuffer_pop();                                            // This causes key updates while the longer time processing register updates happen
        if( !runningOnSimOrUSB &&                             // Automatically, when on battery (hence low processor), change to skip long processing register printing, recovering the fragmented screen here: See timer.c fnTimerEndOfActivity()
            !emptyKeyBuffer() &&
            key_empty() == 1
            ) {
          skippedStackLines = true;
          return;
        }
      #endif //DMCP_BUILD

      #if defined(DMCP_BUILD)
        if(!runningOnSimOrUSB) {
          // partial clearscreen, no menu update, no statusbar update on battery
          if(doRefreshSoftMenu || !(screenUpdatingMode & (SCRUPD_MANUAL_MENU | SCRUPD_SKIP_MENU_ONE_TIME))) {  // battery powered
            clearScreenOld(!clrStatusBar, !clrRegisterLines, clrSoftkeys);                // battery powered
            showSoftmenuCurrentPart();                                                    // battery powered
          }                                                                               // battery powered

          clearScreenOld(!clrStatusBar, clrRegisterLines, !clrSoftkeys);                  // battery powered
          fnPem(NOPARAM);                                                                 // battery powered
          displayShiftAndTamBuffer();                                                     // battery powered

          if(!(screenUpdatingMode & SCRUPD_MANUAL_STATUSBAR)) {                           // battery powered
            clearScreenOld(clrStatusBar, !clrRegisterLines, !clrSoftkeys);                // battery powered
            refreshStatusBar();                                                           // battery powered
          }                                                                               // battery powered
        }
        else {
          clearScreen(7);                                                                  // USB powered
          showSoftmenuCurrentPart();                                                      // USB powered
          fnPem(NOPARAM);                                                                 // USB powered
          displayShiftAndTamBuffer();                                                     // USB powered
          refreshStatusBar();                                                             // USB powered
        }
      #elif defined(PC_BUILD)
          #define TEST_BATTERY_POWERED_SIMULATION
          #if defined (TEST_USB_POWERED_SIMULATION)
            clearScreen(8);                                                                // this tests the USB powered option on sim
            showSoftmenuCurrentPart();                                                    // this tests the USB powered option on sim
            fnPem(NOPARAM);                                                               // this tests the USB powered option on sim
            displayShiftAndTamBuffer();                                                   // this tests the USB powered option on sim
            refreshStatusBar();                                                           // this tests the USB powered option on sim

          #elif defined (TEST_BATTERY_POWERED_SIMULATION)
            if(doRefreshSoftMenu || !(screenUpdatingMode & (SCRUPD_MANUAL_MENU | SCRUPD_SKIP_MENU_ONE_TIME))) {  // this tests the battery powered option on sim
              clearScreenOld(!clrStatusBar, !clrRegisterLines, clrSoftkeys);              // this tests the battery powered option on sim
              showSoftmenuCurrentPart();                                                  // this tests the battery powered option on sim
            }                                                                             // this tests the battery powered option on sim
            clearScreenOld(!clrStatusBar, clrRegisterLines, !clrSoftkeys);                // this tests the battery powered option on sim
            fnPem(NOPARAM);                                                               // this tests the battery powered option on sim
            displayShiftAndTamBuffer();                                                   // this tests the battery powered option on sim
            if(!(screenUpdatingMode & SCRUPD_MANUAL_STATUSBAR)) {                         // this tests the battery powered option on sim
              clearScreenOld(clrStatusBar, !clrRegisterLines, !clrSoftkeys);              // this tests the battery powered option on sim
              refreshStatusBar();                                                         // this tests the battery powered option on sim
            }                                                                             // this tests the battery powered option on sim
          #endif //TEST_BATTERY_POWERED_SIMULATION

      #endif//!DMCP_BUILD PC_BUILD
    doRefreshSoftMenu = false;

    force_refresh(force);
  }


  static void _refreshNormalScreen(void) {
                              #if defined(PC_BUILD) && defined(ANALYSE_REFRESH)
                                printf(">>> BEGIN _refreshNormalScreen calcMode=%d previousCalcMode=%d screenUpdatingMode=%d\n", calcMode, previousCalcMode, screenUpdatingMode);    //JMYY
                                print_caller(NULL);
                              #endif // PC_BUILD &&MONITOR_CLRSCR
        if(calcMode != CM_NIM) refreshNIMdone = false;

        if(calcMode == CM_NORMAL && screenUpdatingMode != SCRUPD_AUTO && temporaryInformation == TI_SHOWNOTHING) {
          goto RETURN_NORMAL;
        }

        if(BASEMODEREGISTERX) {
          screenUpdatingMode &= ~SCRUPD_MANUAL_MENU;
          screenUpdatingMode &= ~(SCRUPD_MANUAL_STACK | SCRUPD_SKIP_STACK_ONE_TIME);
          if(calcMode == CM_NIM) {
            refreshNIMdone = false;
          }
        }

        if(BASEMODEACTIVE) {
          showFracMode();
//          screenUpdatingMode &= ~SCRUPD_MANUAL_STATUSBAR;
        }
        if(calcMode == CM_CONFIRMATION) {
          screenUpdatingMode = SCRUPD_AUTO;
          screenUpdatingMode |= SCRUPD_SKIP_STATUSBAR_ONE_TIME;
        }
        else if(calcMode == CM_MIM) {
          screenUpdatingMode = (aimBuffer[0] == 0) ? SCRUPD_AUTO : (SCRUPD_MANUAL_STACK | SCRUPD_MANUAL_SHIFT_STATUS);
          screenUpdatingMode |= SCRUPD_SKIP_STATUSBAR_ONE_TIME;
        }
        else if(calcMode == CM_TIMER) {
          screenUpdatingMode = SCRUPD_AUTO; //SCRUPD_MANUAL_STACK | SCRUPD_MANUAL_SHIFT_STATUS;
        }
        else if(calcMode == CM_EIM) {
          screenUpdatingMode &= ~SCRUPD_MANUAL_MENU;
          screenUpdatingMode |= SCRUPD_MANUAL_STACK;
        }
        else if(SHOWMODE) {
          screenUpdatingMode &= ~(SCRUPD_MANUAL_STACK | SCRUPD_MANUAL_MENU);
        }
        else if(calcMode == CM_PEM) {
          screenUpdatingMode |= SCRUPD_MANUAL_STATUSBAR;
        }
        //else if(temporaryInformation == TI_SHOWNOTHING) {
        //  screenUpdatingMode |= (SCRUPD_MANUAL_MENU | SCRUPD_MANUAL_STACK);
        //}
        else if((calcMode == CM_NORMAL) && ((getRegisterDataType(REGISTER_X) == dtReal34Matrix) || getRegisterDataType(REGISTER_X) == dtComplex34Matrix) ) {
          screenUpdatingMode &= ~(SCRUPD_MANUAL_STACK | SCRUPD_SKIP_STACK_ONE_TIME);
        }

        _selectiveClearScreen();
        //printf("##> AAAA screenUpdatingMode  MANUAL STACK=%u SKIP MENU ONCE=%u \n",screenUpdatingMode & SCRUPD_MANUAL_STACK, screenUpdatingMode & SCRUPD_SKIP_STACK_ONE_TIME);

        // The ordering of the 4 lines below is important for SHOW (temporaryInformation == TI_SHOW_REGISTER)
        if((calcMode != CM_NIM || (skippedStackLines && calcMode == CM_NIM)) && !(screenUpdatingMode & (SCRUPD_MANUAL_STACK | SCRUPD_SKIP_STACK_ONE_TIME))) {
          if(calcMode != CM_AIM) {
            if(calcMode != CM_TIMER && !tam.alpha && temporaryInformation != TI_VIEW_REGISTER) {
              refreshRegisterLine(REGISTER_T);
            }
            //printf("##> BBBB 4lines Normal Mode\n");
            refreshRegisterLine(REGISTER_Z);
            refreshRegisterLine(REGISTER_Y);
            refreshRegisterLine(REGISTER_X);
            if(temporaryInformation == TI_VIEW_REGISTER) {
              clearRegisterLine(REGISTER_T, true, true);
              refreshRegisterLine(REGISTER_T);
            }
            if(SHOWMODE) {
              screenUpdatingMode |= SCRUPD_MANUAL_MENU; //done with clearing and printing over the menu area, now protecting the menu area
            }
          }
          else {
            //printf("##> CCCC 4lines ALPHA Mode\n");
            if(yMultiLineEdOffset == 3) {
              refreshRegisterLine(REGISTER_T);
              refreshRegisterLine(REGISTER_Z);
              refreshRegisterLine(REGISTER_Y);
            }
            refreshRegisterLine(REGISTER_X);
          }

        }
        else if(calcMode == CM_NIM) {
          #if defined(PC_BUILD) && defined(MONITOR_CLRSCR)
            printf(">>>>      _refreshNormalScreen NIM_LINE: calcMode=%u  programRunStop=%d lastErrorCode=%u screenUpdatingMode=%u\n",calcMode, programRunStop, lastErrorCode, screenUpdatingMode);
          #endif // PC_BUILD &&MONITOR_CLRSCR
          if(!refreshNIMdone) {
            #if defined(PC_BUILD) && defined(MONITOR_CLRSCR)
              printf(">>>>      _refreshNormalScreen NIM: FULL\n");
            #endif // PC_BUILD &&MONITOR_CLRSCR
            refreshRegisterLine(REGISTER_T);
            refreshRegisterLine(REGISTER_Z);
            refreshRegisterLine(REGISTER_Y);
            refreshNIMdone = true;
          }

          refreshRegisterLine(NIM_REGISTER_LINE);
        }
        //printf("##><\n");


        if(calcMode == CM_ASN_BROWSER) {
          fnAsnViewer(NOPARAM);
          calcModeNormal();
          calcMode = CM_ASN_BROWSER;
        }

        if(calcMode == CM_MIM) {
          showMatrixEditor();
        }
        if(calcMode == CM_TIMER) {
          fnShowTimerApp();
        }

        if(currentSolverStatus & SOLVER_STATUS_INTERACTIVE) {
          bool_t mvarMenu = false;
          for(int i = 0; i < SOFTMENU_STACK_SIZE; i++) {
            if(menu(i) == -MNU_MVAR) {
              mvarMenu = true;
              break;
            }
          }
          if(!mvarMenu) {
            if(currentSolverStatus & SOLVER_STATUS_USES_FORMULA) {
              if((currentSolverStatus & SOLVER_STATUS_EQUATION_MODE) == SOLVER_STATUS_EQUATION_INTEGRATE) {
                showSoftmenu(-MNU_Sf);
              }
              else {
                showSoftmenu(-MNU_Solver);
              }
            }
            else {
              currentMvarLabel = INVALID_VARIABLE;
              showSoftmenu(-MNU_MVAR);
            }
          }
        }
        if(calcMode == CM_EIM) {
          bool_t mvarMenu = false;
          for(int i = 0; i < SOFTMENU_STACK_SIZE; i++) {
            if(menu(i) == -MNU_EQ_EDIT) {
              mvarMenu = true;
              break;
            }
          }
          if(!mvarMenu) {
            showSoftmenu(-MNU_EQ_EDIT);
          }
        }

        if(!(screenUpdatingMode & SCRUPD_MANUAL_SHIFT_STATUS)) {
          if(screenUpdatingMode & (SCRUPD_MANUAL_STACK | SCRUPD_SKIP_STACK_ONE_TIME)) {
            clearShiftState();
          }
          displayShiftAndTamBuffer();
        }
        if(!(screenUpdatingMode & (SCRUPD_MANUAL_MENU | SCRUPD_SKIP_MENU_ONE_TIME))) {
          showSoftmenuCurrentPart();
          #if defined(DMCP_BUILD)
            lcd_refresh_dma();             //If this is not here, menu generation is not reliable, and presses are missed. Not sure why.
          #endif //DMCP_BUILD
        }
        if(programRunStop == PGM_STOPPED || programRunStop == PGM_WAITING) {
          hourGlassIconEnabled = false;
        }

        //if(!(screenUpdatingMode & SCRUPD_MANUAL_STATUSBAR)) {
        //  refreshStatusBar();
        //}
        refreshStatusBar();

        #if (REAL34_WIDTH_TEST == 1)
          for(int y=Y_POSITION_OF_REGISTER_Y_LINE; y<Y_POSITION_OF_REGISTER_Y_LINE + 2*REGISTER_LINE_HEIGHT; y++ ) {
            setBlackPixel(SCREEN_WIDTH - largeur - 1, y);
          }
        #endif // (REAL34_WIDTH_TEST == 1)


//2023-07-26 this is new and to be tested for stability
        RETURN_NORMAL:
        screenUpdatingMode |= SCRUPD_MANUAL_STATUSBAR | SCRUPD_MANUAL_STACK | SCRUPD_MANUAL_MENU;

        #if defined(PC_BUILD) && defined(MONITOR_CLRSCR)
          printf(">>> END of _refreshNormalScreen calcMode=%d previousCalcMode=%d screenUpdatingMode=%d\n", calcMode, previousCalcMode, screenUpdatingMode);    //JMYY
        #endif // PC_BUILD &&MONITOR_CLRSCR
  }

  #if defined(PC_BUILD)
    char* get_binary_bits(uint64_t n, int bits) {
      static char buffer[80]; // 64 bits + 15 spaces + null terminator
      if (bits <= 0 || bits > 64) {
          return NULL;
      }
      int pos = 0;
      for (int i = bits - 1; i >= 0; i--) {
          buffer[pos++] = (n & (1ULL << i)) ? '1' : '0';
          if (i % 4 == 0 && i != 0) {
              buffer[pos++] = ' ';
          }
      }
      buffer[pos] = '\0';
      return buffer;
    }
  #endif //PC_BUILD


  int16_t refreshScreenCounter = 0;        //JM

  void refreshScreen(uint16_t source) {
                              #if defined(ANALYSE_REFRESH)
                                print_caller(NULL);
                              #endif

                              #if defined(DMCP_BUILD) && defined(CLICK_REFRESHSCR)
                                powerMarkerMsF(3,10000);
                              #endif // DMCP_BUILD && CLICK_REFRESHSCR

    if(calcMode!=CM_AIM && calcMode!=CM_NIM && calcMode!=CM_PLOT_STAT && calcMode!=CM_GRAPH && calcMode!=CM_LISTXY && last_CM != 240) {  //240 specifically to prefent this
      last_CM = 254;  //JM Force NON-CM_AIM and NON-CM_NIM to refresh to be compatible to 43S
    }
    else {
      if(last_CM == 240) {
        last_CM = calcMode;
      }
    }
                             #if defined(PC_BUILD) && defined(MONITOR_CLRSCR)
                               jm_show_calc_state("refreshScreen");
                             #endif // PC_BUILD

                             #if defined(PC_BUILD) && defined(VERBOSE_MINIMUM)
                               char ttt[500] = "";
                               char sss[500] = "";
                               strcpy(sss,get_binary_bits(screenUpdatingMode,8));
                               convertUInt64ToShortIntegerRegister(0, screenUpdatingMode, 2, TEMP_REGISTER_1 );
                               shortIntegerToDisplayString(TEMP_REGISTER_1, ttt, false, noBaseOverride);
                               stringToASCII(ttt,sss);
                               strcpy(ttt,"");
                               if(screenUpdatingMode == 0) strcat(ttt,"AUTO "); else {
                                 if((screenUpdatingMode & 0x40)) strcat(ttt,"SkpMEN ");
                                 if((screenUpdatingMode & 0x20)) strcat(ttt,"SkpSTK ");

                                 if(!(screenUpdatingMode & 0x08)) strcat(ttt,"SHFT ");
                                 if(!(screenUpdatingMode & 0x04)) strcat(ttt,"MENU ");
                                 if(!(screenUpdatingMode & 0x02)) strcat(ttt, "STK ");
                                 if(!(screenUpdatingMode & 0x01)) strcat(ttt, "STS ");
                               }
                               int16_t m = softmenuStack[0].softmenuId;
                               char uuu[100];
                               stringToASCII(indexOfItems[currentMenu() > 0 ? currentMenu() : -currentMenu()].itemSoftmenuName, uuu);

                               printf("   refrsh(%5u): Cnt=%3d %s CM=%2d scr..upd:%3d=%12s=>%26s TI=%4u CL=%s tam:%5i MENUid=%2d:%4i:%s\n",
                                  source, refreshScreenCounter++,
                                  (last_CM != calcMode) ? "OVR" : "   ",
                                  calcMode,
                                  screenUpdatingMode, sss, ttt,
                                  temporaryInformation,
                                  (alphaCase == AC_LOWER)?"LO":"UP",
                                  tam.mode,
                                  m, currentMenu(), uuu);
                               fflush(stdout);
                             #endif // PC_BUILD


    switch(currentMenu()) {
      case -MNU_GEV:
      case -MNU_BINOM:
      case -MNU_CAUCH:
      case -MNU_WEIBL:
      case -MNU_CHI2:
      case -MNU_T:
      case -MNU_EXPON:
      case -MNU_POISS:
      case -MNU_F:
      case -MNU_GEOM:
      case -MNU_HYPER:
      case -MNU_LOGIS:
      case -MNU_NORML: screenUpdatingMode = SCRUPD_AUTO;
                       screenUpdatingMode |= SCRUPD_SKIP_STATUSBAR_ONE_TIME;
                       break;
      default: ;
    }

    switch(calcMode) {
      case CM_FLAG_BROWSER:
        last_CM = calcMode;
        clearScreen(9);
        flagBrowser(NOPARAM);
        refreshStatusBar();
        force_refresh(force);
        break;

      case CM_FONT_BROWSER:
        last_CM = calcMode;
        clearScreen(10);
        fontBrowser(NOPARAM);
        refreshStatusBar();
        force_refresh(force);
        break;

      case CM_REGISTER_BROWSER:
        last_CM = calcMode;
        clearScreen(11);
        registerBrowser(NOPARAM);
        refreshStatusBar();
        force_refresh(force);
        break;

      case CM_PEM:
        screenUpdatingMode &= ~SCRUPD_MANUAL_MENU;
        _refreshPemScreen();
        break;


      case CM_CONFIRMATION: {
        if(previousCalcMode == CM_PEM) {
          _refreshPemScreen();
        }
        else {
          _refreshNormalScreen();
        }
        break;
      }

      case CM_ASN_BROWSER:
      case CM_NORMAL:
      case CM_AIM:
      case CM_NIM:
      case CM_MIM:
      case CM_EIM:
      case CM_ASSIGN:
      case CM_ERROR_MESSAGE:
      case CM_TIMER:
//printf("screenUpdatingMode1=%u\n",screenUpdatingMode);
        if((doRefreshSoftMenu && !SHOWMODE) || calcMode == CM_ASSIGN) {
          screenUpdatingMode &= ~SCRUPD_MANUAL_MENU ;
        }


        ////printf("screenUpdatingMode2=%u calcmode=%u last_CM=%u\n",screenUpdatingMode, calcMode, last_CM);
        //if(last_CM != calcMode) {
        //  if(!SHOWMODE) screenUpdatingMode &= ~SCRUPD_MANUAL_MENU ;
        //  screenUpdatingMode &= ~SCRUPD_MANUAL_STACK ;
        //  //printf("screenUpdatingMode3=%u calcmode=%u last_CM=%u\n",screenUpdatingMode, calcMode, last_CM);
        //}

        else if(calcMode == CM_MIM) {
          screenUpdatingMode = (aimBuffer[0] == 0) ? SCRUPD_AUTO : (SCRUPD_MANUAL_STACK | SCRUPD_MANUAL_SHIFT_STATUS);
          screenUpdatingMode |= SCRUPD_SKIP_STATUSBAR_ONE_TIME;
        }
        else if(calcMode == CM_TIMER) {
          screenUpdatingMode = SCRUPD_MANUAL_STACK | SCRUPD_MANUAL_SHIFT_STATUS;
        }
//printf("screenUpdatingMode4=%u calcmode=%u last_CM=%u\n",screenUpdatingMode, calcMode, last_CM);


        _refreshNormalScreen();
        break;

      case CM_LISTXY:
          doRefreshSoftMenu = false;
          displayShiftAndTamBuffer();
          refreshStatusBar();
          fnStatList();
          hourGlassIconEnabled = false;
          refreshStatusBar();
          force_refresh(force);
        break;

      case CM_GRAPH:
          doRefreshSoftMenu = false;
          graph_plotmem();
          displayShiftAndTamBuffer();
          showSoftmenuCurrentPart();
          hourGlassIconEnabled = true;
          refreshStatusBar();
          hourGlassIconEnabled = false;
          showHideHourGlass();
          refreshStatusBar();
          force_refresh(force);
        break;

      case CM_PLOT_STAT:
          doRefreshSoftMenu = false;
          graphPlotstat(plotSelection);
          displayShiftAndTamBuffer();
          showSoftmenuCurrentPart();
          hourGlassIconEnabled = true;
          refreshStatusBar();
          graphDrawLRline(plotSelection);
          if(lastErrorCode != ERROR_NONE) {
            if(currentMenu() == -MNU_HPLOT || currentMenu() == -MNU_PLOT_ASSESS || currentMenu() == -MNU_HPLOT || currentMenu() == -MNU_PLOT_SCATR) {
              popSoftmenu();
              calcMode = CM_NORMAL;
              refreshScreen(84);
            }
          }
          hourGlassIconEnabled = false;
          showHideHourGlass();
          refreshStatusBar();
          force_refresh(force);
        break;

      default: ;
    }

    doRefreshSoftMenu = false;
    #if !defined(DMCP_BUILD)
      refreshLcd(NULL);
    #endif // !DMCP_BUILD
  }
#endif // !TESTSUITE_BUILD


void fnSNAP(uint16_t unusedButMandatoryParameter) {
  #ifdef PC_BUILD
    printf("fnSNAP!\n");
  #endif
  resetShiftState();                  //JM To avoid f or g top left of the screen, clear to make sure
  refreshScreen(80);

  #if defined(PC_BUILD)  //added the xcopy commands needed for hardware, to better duplicate the hardware standardScreenDump
    xcopy(tmpString, errorMessage, ERROR_MESSAGE_LENGTH + AIM_BUFFER_LENGTH + NIM_BUFFER_LENGTH + TAM_BUFFER_LENGTH);       //backup portion of the "message buffer" area in DMCP used by ERROR..AIM..NIM buffers, to the tmpstring area in DMCP. DMCP uses this area during create_screenshot.
    fnScreenDump(0);
    xcopy(errorMessage, tmpString, ERROR_MESSAGE_LENGTH + AIM_BUFFER_LENGTH + NIM_BUFFER_LENGTH + TAM_BUFFER_LENGTH);        //   This total area must be less than the tmpString storage area, which it is.
  #elif defined(DMCP_BUILD)
    standardScreenDump();
  #endif

  char ss[TAM_BUFFER_LENGTH];
  xcopy(ss, tamBuffer, TAM_BUFFER_LENGTH);      //Backup the TamBuffer, in case we are in a TAM screen when doing screenshot
  if(calcMode == CM_AIM) {
    fnP_Alpha();     //print alpha
  }
  else {
    fnP_All_Regs(PRN_STK); //print stack
  }
  xcopy(tamBuffer, ss, TAM_BUFFER_LENGTH);      //Backup the TamBuffer, in case we are in a TAM screen when doing screenshot


  screenUpdatingMode |= SCRUPD_SKIP_STACK_ONE_TIME | SCRUPD_SKIP_MENU_ONE_TIME;
}


void fnScreenDump(uint16_t unusedButMandatoryParameter) {
  #if defined(ANDROID_BUILD)
    char bmpFileName[32];
    time_t rawTime;
    struct tm *timeInfo;
    time(&rawTime);
    timeInfo = localtime(&rawTime);
    strftime(bmpFileName, sizeof(bmpFileName), "SNAP_%Y%m%d_%H%M%S.bmp", timeInfo);

    int fd = requestAndroidFile(1, bmpFileName, 3); // 3 = SCREENS category
    if (fd < 0) return;

    FILE *bmp = fdopen(fd, "wb");
    if (!bmp) {
        close(fd);
        return;
    }
  #elif defined(PC_BUILD)
    FILE *bmp;
    char bmpFileName[22];
    time_t rawTime;
    struct tm *timeInfo;

    time(&rawTime);
    timeInfo = localtime(&rawTime);

    strftime(bmpFileName, 22, "%Y%m%d-%H%M%S00.bmp", timeInfo);
    bmp = fopen(bmpFileName, "wb");
  #else
    return;
  #endif

  #if defined(PC_BUILD) || defined(ANDROID_BUILD)
    int32_t x, y;
    uint32_t uint32;
    uint16_t uint16;
    uint8_t  uint8 = 0;

    fwrite("BM", 1, 2, bmp);        // Offset 0x00  0  BMP header

    uint32 = (SCREEN_WIDTH/8 * SCREEN_HEIGHT) + 610;
    fwrite(&uint32, 1, 4, bmp);     // Offset 0x02  2  File size

    uint32 = 0;
    fwrite(&uint32, 1, 4, bmp);     // Offset 0x06  6  unused

    uint32 = 0x00000082;
    fwrite(&uint32, 1, 4, bmp);     // Offset 0x0a 10  Offset where the bitmap data can be found

    uint32 = 0x0000006c;
    fwrite(&uint32, 1, 4, bmp);     // Offset 0x0e 14  Number of bytes in DIB header

    uint32 = SCREEN_WIDTH;
    fwrite(&uint32, 1, 4, bmp);     // Offset 0x12 18  Bitmap width

    uint32 = SCREEN_HEIGHT;
    fwrite(&uint32, 1, 4, bmp);     // Offset 0x16 22  Bitmap height

    uint16 = 0x0001;
    fwrite(&uint16, 1, 2, bmp);     // Offset 0x1a 26  Number of planes

    uint16 = 0x0001;
    fwrite(&uint16, 1, 2, bmp);     // Offset 0x1c 28  Number of bits per pixel

    uint32 = 0;
    fwrite(&uint32, 1, 4, bmp);     // Offset 0x1e 30  Compression

    uint32 = 0x000030c0;
    fwrite(&uint32, 1, 4, bmp);     // Offset 0x22 34  Size of bitmap data (including padding)

    uint32 = 0x00001a7c; // 6780 pixels/m
    fwrite(&uint32, 1, 4, bmp);     // Offset 0x26 38  Horizontal print resolution

    uint32 = 0x00001a7c; // 6780 pixels/m
    fwrite(&uint32, 1, 4, bmp);     // Offset 0x2a 42  Vertical print resolution

    uint32 = 0x00000002;
    fwrite(&uint32, 1, 4, bmp);     // Offset 0x2e 46  Number of colors in the palette

    uint32 = 0x00000002;
    fwrite(&uint32, 1, 4, bmp);     // Offset 0x32 50  Number of important colors

    uint32 = 0x73524742;
    fwrite(&uint32, 1, 4, bmp);     // Offset 0x36  ???

    uint32 = 0;
    fwrite(&uint32, 1, 4, bmp);     // Offset 0x3a  ???
    fwrite(&uint32, 1, 4, bmp);     // Offset 0x3e  ???
    fwrite(&uint32, 1, 4, bmp);     // Offset 0x42  ???
    fwrite(&uint32, 1, 4, bmp);     // Offset 0x46  ???
    fwrite(&uint32, 1, 4, bmp);     // Offset 0x4a  ???
    fwrite(&uint32, 1, 4, bmp);     // Offset 0x4e  ???
    fwrite(&uint32, 1, 4, bmp);     // Offset 0x52  ???
    fwrite(&uint32, 1, 4, bmp);     // Offset 0x56  ???
    fwrite(&uint32, 1, 4, bmp);     // Offset 0x5a  ???
    fwrite(&uint32, 1, 4, bmp);     // Offset 0x5e  ???
    fwrite(&uint32, 1, 4, bmp);     // Offset 0x62  ???
    fwrite(&uint32, 1, 4, bmp);     // Offset 0x66  ???

    uint32 = 0x00000002;
    fwrite(&uint32, 1, 4, bmp);     // Offset 0x6a  ???

    uint32 = 0;
    fwrite(&uint32, 1, 4, bmp);     // Offset 0x6e  ???
    fwrite(&uint32, 1, 4, bmp);     // Offset 0x72  ???
    fwrite(&uint32, 1, 4, bmp);     // Offset 0x76  ???

    uint32 = 0x00dff5cc; // light green
    fwrite(&uint32, 1, 4, bmp);     // Offset 0x7a  RGB color for 0

    uint32 = 0;
    fwrite(&uint32, 1, 4, bmp);     // Offset 0x7e  RGB color for 1

    // Offset 0x82  bit map data
    uint16 = 0;
    for(y=SCREEN_HEIGHT-1; y>=0; y--) {
      uint8 = 0;
      for(x=0; x<SCREEN_WIDTH; x++) {
        uint8 <<= 1;
        uint32_t pix = *(screenData + y*screenStride + x);
        #if defined(ANDROID_BUILD)
          // Android uses ARGB. Mask alpha and check for 'on' color.
          // Background colors are: Vintage=0xFFDFF5CC, B&W=0xFFE0E0E0
          if ((pix & 0xFFFFFF) != 0xDFF5CC && (pix & 0xFFFFFF) != 0xE0E0E0) {
             uint8 |= 1;
          }
        #else
          if(pix == ON_PIXEL) {
            uint8 |= 1;
          }
        #endif

        if((x % 8) == 7) {
          fwrite(&uint8, 1, 1, bmp);
          uint8 = 0;
        }
      }
      fwrite(&uint16, 1, 2, bmp); // Padding
    }


    fclose(bmp);
    screenUpdatingMode |= SCRUPD_SKIP_STACK_ONE_TIME | SCRUPD_SKIP_MENU_ONE_TIME;
  #endif // PC_BUILD
}


#if !defined(TESTSUITE_BUILD)
  static int32_t _getPositionFromRegister(calcRegister_t regist, int16_t maxValuePlusOne) {
    int32_t value;

    if(getRegisterDataType(regist) == dtReal34) {
      real34_t maxValue34;

      int32ToReal34(maxValuePlusOne, &maxValue34);
      if(real34CompareLessThan(REGISTER_REAL34_DATA(regist), const34_0) || real34CompareLessEqual(&maxValue34, REGISTER_REAL34_DATA(regist))) {
        displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
        #if defined(PC_BUILD)
          real34ToString(REGISTER_REAL34_DATA(regist), errorMessage);
          sprintf(tmpString, "x %" PRId16 " = %s:", regist, errorMessage);
          moreInfoOnError("In function _getPositionFromRegister:", tmpString, "this value is negative or too big!", NULL);
        #endif // PC_BUILD
        return -1;
      }
      value = real34ToInt32(REGISTER_REAL34_DATA(regist));
    }

    else if(getRegisterDataType(regist) == dtLongInteger) {
      longInteger_t lgInt;

      convertLongIntegerRegisterToLongInteger(regist, lgInt);
      if(longIntegerCompareUInt(lgInt, 0) < 0 || longIntegerCompareUInt(lgInt, maxValuePlusOne) >= 0) {
        displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
        #if defined(PC_BUILD)
          longIntegerToAllocatedString(lgInt, errorMessage, ERROR_MESSAGE_LENGTH);
          sprintf(tmpString, "register %" PRId16 " = %s:", regist, errorMessage);
          moreInfoOnError("In function _getPositionFromRegister:", tmpString, "this value is negative or too big!", NULL);
        #endif // PC_BUILD
        longIntegerFree(lgInt);
        return -1;
      }
      longIntegerToUInt32(lgInt, value);
      longIntegerFree(lgInt);
    }

    else {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
      #if defined(PC_BUILD)
        sprintf(errorMessage, "register %" PRId16 " is %s:", regist, getRegisterDataTypeName(regist, true, false));
        moreInfoOnError("In function _getPositionFromRegister:", errorMessage, "not suited for addressing!", NULL);
      #endif // PC_BUILD
      return -1;
    }

    return value;
  }

  static void getPixelPos(int32_t *x, int32_t *y) {
    *x = _getPositionFromRegister(REGISTER_X, SCREEN_WIDTH);
    *y = _getPositionFromRegister(REGISTER_Y, SCREEN_HEIGHT);
  }
#endif // !TESTSUITE_BUILD

void fnClLcd(uint16_t unusedButMandatoryParameter) {
  #if !defined(TESTSUITE_BUILD)
    int32_t x, y;
    getPixelPos(&x, &y);
    if(lastErrorCode == ERROR_NONE) {
      screenUpdatingMode |= SCRUPD_MANUAL_STATUSBAR | SCRUPD_MANUAL_STACK | SCRUPD_MANUAL_MENU | SCRUPD_MANUAL_SHIFT_STATUS;
      lcd_fill_rect(x, 0, SCREEN_WIDTH - x, SCREEN_HEIGHT - y, LCD_SET_VALUE);
    }
  #endif // !TESTSUITE_BUILD
}


void fnPixel(uint16_t unusedButMandatoryParameter) {
  #if !defined(TESTSUITE_BUILD)
    int32_t x, y;
    getPixelPos(&x, &y);
    if(lastErrorCode == ERROR_NONE) {
      screenUpdatingMode |= SCRUPD_MANUAL_STACK | SCRUPD_MANUAL_MENU | SCRUPD_MANUAL_SHIFT_STATUS;
        if((SCREEN_HEIGHT - y - 1) <= Y_POSITION_OF_REGISTER_T_LINE) {
          screenUpdatingMode |= SCRUPD_MANUAL_STATUSBAR;
        }
      setBlackPixel(x, SCREEN_HEIGHT - y - 1);
    }
  #endif // !TESTSUITE_BUILD
}

void fnPoint(uint16_t unusedButMandatoryParameter) {
  #if !defined(TESTSUITE_BUILD)
    int32_t x, y;
    getPixelPos(&x, &y);
    if(lastErrorCode == ERROR_NONE) {
      screenUpdatingMode |= SCRUPD_MANUAL_STACK | SCRUPD_MANUAL_MENU | SCRUPD_MANUAL_SHIFT_STATUS;
      if((SCREEN_HEIGHT - y - 2) <= Y_POSITION_OF_REGISTER_T_LINE) {
        screenUpdatingMode |= SCRUPD_MANUAL_STATUSBAR;
      }
      lcd_fill_rect(x - 1, SCREEN_HEIGHT - y - 2, 3, 3, LCD_EMPTY_VALUE);
    }
  #endif // !TESTSUITE_BUILD
}

void fnAGraph(uint16_t regist) {
  #if !defined(TESTSUITE_BUILD)
    int32_t x, y;
    uint32_t gramod;
    longInteger_t liGramod;
    getPixelPos(&x, &y);
    convertLongIntegerRegisterToLongInteger(RESERVED_VARIABLE_GRAMOD, liGramod);
    longIntegerToUInt32(liGramod, gramod);
    longIntegerFree(liGramod);
    if(lastErrorCode == ERROR_NONE) {
      if(getRegisterDataType(regist) == dtShortInteger) {
        uint64_t val;
        int16_t sign;
        const uint8_t savedShortIntegerMode = shortIntegerMode;

        screenUpdatingMode |= SCRUPD_MANUAL_STACK | SCRUPD_MANUAL_MENU | SCRUPD_MANUAL_SHIFT_STATUS;
        if((SCREEN_HEIGHT - y - 1 - (int)shortIntegerWordSize) <= Y_POSITION_OF_REGISTER_T_LINE) {
          screenUpdatingMode |= SCRUPD_MANUAL_STATUSBAR;
        }
        shortIntegerMode = SIM_UNSIGN;
        convertShortIntegerRegisterToUInt64(regist, &sign, &val);
        shortIntegerMode = savedShortIntegerMode;
        for(uint32_t i = 0; i < shortIntegerWordSize; ++i) {
          switch(gramod) {
            case 1: if(!(val & 1)) setWhitePixel(x, SCREEN_HEIGHT - y - 1 - i); /* fallthrough */
            case 0: if(val & 1)    setBlackPixel(x, SCREEN_HEIGHT - y - 1 - i); break;
            case 2: if(val & 1)    setWhitePixel(x, SCREEN_HEIGHT - y - 1 - i); break;
            case 3: if(val & 1)    flipPixel(x, SCREEN_HEIGHT - y - 1 - i);     break;
          }
          val >>= 1;
        }

        fnInc(REGISTER_X);
      }

      else {
        displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
        #if defined(PC_BUILD)
          sprintf(errorMessage, "register %" PRId16 " is %s:", regist, getRegisterDataTypeName(regist, true, false));
          moreInfoOnError("In function fnAGraph:", errorMessage, "not suited for addressing!", NULL);
        #endif // PC_BUILD
      }
    }
  #endif // !TESTSUITE_BUILD
}


void insertAlphaCursor(uint16_t startAt) {
  #if !defined(TESTSUITE_BUILD)
    char       *bufPtr = tmpString + startAt;
    const char *strPtr = aimBuffer;
    uint16_t    strLength = 0;
//    int16_t     strWidth = 0;
//    int16_t     glyphWidth = 0;

    *bufPtr       = 0;

    if(alphaCursor == 0) {
      *bufPtr       = STD_CURSOR[0];
      *(bufPtr + 1) = STD_CURSOR[1];
      *(bufPtr + 2) = 0;
//      glyphWidth = stringWidth(bufPtr, &standardFont, true, true);
//      strWidth += glyphWidth;
      bufPtr += 2;
    }

    while((*strPtr) != 0) {
      ++strLength;
      *bufPtr = *strPtr;

      /* Double-byte characters */
      if((*strPtr) & 0x80) {
        *(bufPtr + 1) = *(strPtr + 1);
        *(bufPtr + 2) = 0;
        bufPtr += 2;
      }

      /* Single-byte characters */
      else {
      *(bufPtr + 1) = 0;
      bufPtr += 1;
      }

      /* Cursor */
      if(strLength == alphaCursor) {
        *bufPtr       = STD_CURSOR[0];
        *(bufPtr + 1) = STD_CURSOR[1];
        *(bufPtr + 2) = 0;
//        glyphWidth = stringWidth(bufPtr, &standardFont, true, true);
//        strWidth += glyphWidth;
        bufPtr += 2;
      }

      /* Next character */
      strPtr += ((*strPtr) & 0x80) ? 2 : 1;
    }
  #endif // !TESTSUITE_BUILD
}
