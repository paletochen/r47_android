// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#if !defined(SCREEN_H)
#define SCREEN_H

  extern bool_t blockMonitoring;
  bool_t   registerFMA(calcRegister_t regist, real_t* tmp1, real_t* tmp2, real34_t* tmp3, angularMode_t* angle, realContext_t *c);

  void     setLastintegerBasetoZero           (void);
  extern bool_t   doRefreshSoftMenu;                                                                              //dr
  void     _executeItem(int16_t item, int keyCode);
  void     FN_handler();                                                                                          //JM LONGPRESS
  void     Shft_handler();                                                                                        //JM LONGPRESS f/g
  void     LongpressKey_handler();                                                                                //JM LONGPRESS CLX
  void     Shft_stop();                                                                                           //JM reset shift after  4s
  void     closeShowMenu(void);
  void     reallyClearStatusBar(uint8_t info);

  void     clearScreenOld(bool_t clearStatusBar, bool_t clearRegisterLines, bool_t clearSoftkeys);               //JMOLD
  void     clearScreenGraphs(uint8_t source, bool_t clearTextArea, bool_t clearGraphArea);
  #define  clrStatusBar true
  #define  clrRegisterLines true
  #define  clrSoftkeys true
  #define  clrTextArea true
  #define  clrGraphArea true

  void     fnScreenDump                       (uint16_t unusedButMandatoryParameter);
  void     fnSNAP                             (uint16_t unusedButMandatoryParameter);


void       fnClLcd                            (uint16_t unusedButMandatoryParameter);
void       fnPixel                            (uint16_t unusedButMandatoryParameter);
void       fnPoint                            (uint16_t unusedButMandatoryParameter);
void       fnAGraph                           (uint16_t regist);
void       insertAlphaCursor                  (uint16_t startAt);

void       drawSinglePixelFullWidthLine       (int y);

char       letteredRegisterName(calcRegister_t regist);

  #if defined(PC_BUILD)
  /**
   * Draws the calc's screen on the PC window widget.
   *
   * \param[in] widget Not used
   * \param[in] cr
   * \param[in] data   Not used
   */
  gboolean drawScreen                         (GtkWidget *widget, cairo_t *cr, gpointer data);
  void     copyScreenToClipboard              (void);
  void     copyRegisterXToClipboard           (void);
  void     copyStackRegistersToClipboard      (void);
  void     copyAllRegistersToClipboard        (void);
  void     copyRegisterToClipboardString      (calcRegister_t regist, char *clipboardString);

  /**
   * Refreshes calc's screen.
   * This function is called every SCREEN_REFRESH_PERIOD ms by a GTK timer.
   * - make the cursor blink if needed
   * - refresh date and time in the status bar if needed
   * - refresh the whole screen if needed
   *
   * \param[in] unusedData Not used
   * \return What will happen next?
   *   - true  = timer will call this function again
   *   - false = timer stops calling this function
   */
  gboolean refreshLcd                         (gpointer unusedData);
  #endif // PC_BUILD

  #if defined(DMCP_BUILD)
    void     copyRegisterToClipboardString      (calcRegister_t regist, char *clipboardString);                   //JMCSV Added for textfiles
    void     refreshLcd                         (void);
  #endif // DMCP_BUILD

  void     execTimerApp                         (uint16_t timerType);

  #define  force 1
  #define  timed 0
  #define  halfSec_clearZ true
  #define  halfSec_clearT true
  #define  halfSec_disp true

  #define LeftGraphInfoX         0
  #define topLeftGraphInfoY      Y_POSITION_OF_REGISTER_T_LINE-4
  #define widthGraphInfoBox      SCREEN_WIDTH - 240 - 2
  #define heightGraphInfoBox     240 - Y_POSITION_OF_REGISTER_T_LINE - SOFTMENU_HEIGHT * 3+4
  #define widthGraphInclBorder   240 + 2
  #define menuHeightInclBorder   SOFTMENU_HEIGHT * 3
  #define topLeftMenuInclBorderY 240 - menuHeightInclBorder
  #if defined(TESTSUITE_BUILD)
      #define clearScreen(cnt)                        do { } while(0)
      #define clearScreenExcludingStatusBar(cnt)      do { } while(0)
      #define clearScreenStatusBar(cnt)               do { } while(0)
  #else //!TESTSUITE_BUILD
    #if defined(MONITOR_CLRSCR)
      #define clearScreen(cnt)                        do { lcd_fill_rect(0,  0, SCREEN_WIDTH, 240, LCD_SET_VALUE); forceSBupdate(); printf("CLEARFULLSCREEN Macro %u\n",        (uint16_t)cnt);} while(0)  //set last to undefined to force first refresh condition to be true
      #define clearScreenExcludingStatusBar(cnt)      do { lcd_fill_rect(0, 20, SCREEN_WIDTH, 220, LCD_SET_VALUE);                  printf("CLEARFULLSCREEN EXCL SB Macro %u\n",(uint16_t)cnt);} while(0)  //set last to undefined to force first refresh condition to be true
      #define clearScreenStatusBar(cnt)               do { lcd_fill_rect(0,  0, calcMode == CM_GRAPH ? widthGraphInfoBox : SCREEN_WIDTH, 20,  LCD_SET_VALUE); forceSBupdate(); printf("CLEARSB Macro %u\n", (uint16_t)cnt);} while(0)  //set last to undefined to force first refresh condition to be true
    #else //!MONITOR_CLRSCR
      #define clearScreen(cnt)                        do { lcd_fill_rect(0,  0, SCREEN_WIDTH, 240, LCD_SET_VALUE); forceSBupdate();} while(0)  //set last to undefined to force first refresh condition to be true
      #define clearScreenExcludingStatusBar(cnt)      do { lcd_fill_rect(0, 20, SCREEN_WIDTH, 220, LCD_SET_VALUE);                 } while(0)  //set last to undefined to force first refresh condition to be true
      #define clearScreenStatusBar(cnt)               do { lcd_fill_rect(0,  0, calcMode == CM_GRAPH ? widthGraphInfoBox : SCREEN_WIDTH, 20,  LCD_SET_VALUE); forceSBupdate();} while(0)  //set last to undefined to force first refresh condition to be true
    #endif //MONITOR_CLRSCR
  #endif //!TESTSUITE_BUILD


  #if !defined(TESTSUITE_BUILD)

  void     refreshFn                            (uint16_t timerType);                                           //dr - general timeout handler
  extern uint8_t  compressString;                                                                               //global flags for character control: compressString
  extern uint8_t  raiseString;                                                                                  //global flags for character control: raiseString
  extern uint8_t  multiEdLines;
  extern uint16_t current_cursor_x;
  extern uint16_t current_cursor_y;


  //Stack string large font display
  #define STACK_X_STR_LRG_FONT
  #define STACK_STR_MED_FONT
  #define STACK_X_STR_MED_FONT
  #undef  STACK_X_STR_MED_FONT //not needed as the full and half fonts are the same width

  //mode
  #define stdNoEnlarge     0                                                                                    //JM vv compress, enlarge, small fonts
  #define stdEnlarge       1  //used in screen.c only
  #define stdnumEnlarge    2
  #define numSmall         3  //used in screen.c only
  #define numHalf          4  //used in screen.c only

  //showStringEnhanced, showStringC47
  #define DO_LF            true
  #define NO_LF            false
  #define DO_compress      1
  #define NO_compress      0
  #define nocompress       0
  #define DO_raise         1
  #define NO_raise         0
  #define DO_Show          0
  #define NO_Show          1
  #define DO_Bold          1
  #define NO_Bold          0


  uint32_t showStringC47                      (const char *string, int mode, int comp, uint32_t x, uint32_t y, videoMode_t videoMode, bool_t showLeadingCols, bool_t showEndingCols );
  uint32_t stringWidthC47                     (const char *str,    int mode, int comp,                                                bool_t withLeadingEmptyRows, bool_t withEndingEmptyRows);
  char *stringAfterPixelsC47                  (const char *str,    int mode, int comp, uint32_t width,                                bool_t withLeadingEmptyRows, bool_t withEndingEmptyRows);
  uint32_t stringWidthWithLimitC47            (const char *str,    int mode, int comp, uint32_t limitWidth,                           bool_t withLeadingEmptyRows, bool_t withEndingEmptyRows); // like stringWidthC47 but don't check anymore after once exceeded limitWidth

  uint32_t showStringEdC47                    (uint32_t lastline, int16_t offset, int16_t edcursor, const char *string, uint32_t x, uint32_t y, videoMode_t videoMode, bool_t showLeadingCols, bool_t showEndingCols, bool_t noshow1);
  void findOffset(void);
  void incOffset(void);

  void     show_f_jm                          (void);
  void     show_g_jm                          (void);
  void     clear_fg_jm                        (void);
  void     underline_softkey                  (uint16_t xSoftkeyMask, uint16_t ySoftKey);
  void     force_refresh                      (uint8_t mode);
  void     force_SBrefresh                    (uint8_t mode);
  bool_t   progressHalfSecUpdate_Integer      (uint8_t mode, char *txt, int32_t loop, bool_t clearZ, bool_t clearT, bool_t disp);
  bool_t   monitorExit                        (int32_t *loop, char* str);
  bool_t   checkHalfSec                       (void);

  void     refreshScreen                      (uint16_t source);
  bool_t   showingProbMenu                    (void);

  /**
   * Displays a 0 terminated string.
   *
   * \param[in] string          String whose first glyph is to display
   * \param[in] font            Font to use
   * \param[in] x               x coordinate where to display the glyph
   * \param[in] y               y coordinate where to display the glyph
   * \param[in] videoMode       Video mode normal or reverse
   * \param[in] showLeadingCols Display the leading empty columns
   * \param[in] showEndingCols  Display the ending empty columns
   * \return x coordinate for the next glyph
   */
  uint32_t showStringEnhanced                 (const char *string, const font_t *font, uint32_t x, uint32_t y, videoMode_t videoMode, bool_t showLeadingCols, bool_t showEndingCols, uint8_t compress1, uint8_t raise1, uint8_t noShow1, uint8_t boldString1, bool_t lf);
  uint32_t showString                         (const char *str,   const font_t *font, uint32_t x, uint32_t y, videoMode_t videoMode, bool_t showLeadingCols, bool_t showEndingCols);
  void getStringBounds                        (const char *string, const font_t *font, uint32_t *col, uint32_t *row);

  /**
   * Displays the first glyph of a string.
   *
   * \param[in] ch              String whose first glyph is to display
   * \param[in] font            Font to use
   * \param[in] x               x coordinate where to display the glyph
   * \param[in] y               y coordinate where to display the glyph
   * \param[in] videoMode       Video mode normal or reverse
   * \param[in] showLeadingCols Display the leading empty columns
   * \param[in] showEndingCols  Display the ending empty columns
   * \return x coordinate for the next glyph
   */
  uint32_t showGlyph                          (const char *ch,    const font_t *font, uint32_t x, uint32_t y, videoMode_t videoMode, bool_t showLeadingCols, bool_t showEndingCols, bool_t noPreClear);

  /**
   * Displays a glyph using it's Unicode code point.
   *
   * \param[in] charCode        Unicode code point of the glyph to display
   * \param[in] font            Font to use
   * \param[in] x               x coordinate where to display the glyph
   * \param[in] y               y coordinate where to display the glyph
   * \param[in] videoMode       Video mode normal or reverse
   * \param[in] showLeadingCols Display the leading empty columns
   * \param[in] showEndingCols  Display the ending empty columns
   * \return x coordinate for the next glyph
   */
  uint32_t showGlyphCode                      (uint16_t charCode, const font_t *font, uint32_t x, uint32_t y, videoMode_t videoMode, bool_t showLeadingCols, bool_t showEndingCols, bool_t noPreClear);

  /**
   * Hides the cursor.
   */
  void     hideCursor                         (void);

  /**
   * Displays the function name.
   * The function name of the currently pressed button is shown in the
   * upper left corner of the T register line.
   *
   * \param[in] item     Item ID to show
   * \param[in] counter  number of 1/10 seconds until NOP
   */
  void     showFunctionName                   (int16_t itm, int16_t delayInMs, const char * arg);

  /**
   * Hides the function name.
   * The function name in the upper left corner of the T register line is hidden
   * and the counter is cleared.
   */
  void     hideFunctionName                   (void);


  /**
   * Clears one register line.
   *
   * \param[in] yStart y coordinate from where starting to clear
   */
  void     clearRect                          (uint32_t g_line_x, uint32_t g_line_y);

  void     clearRegisterLine                  (calcRegister_t regist, bool_t clearTop, bool_t clearBottom);

  /**
   * Updates matrix height cache.
   */
  void     updateMatrixHeightCache            (void);

  /**
   * Displays one register line.
   *
   * \param[in] regist Register line to display
   */
  void     refreshRegisterLine                (calcRegister_t regist);

  void     viewRegName2(char *prefix, int16_t *prefixWidth); //register name + ":" for SHOW
  void     displayNim                         (const char *nim, const char *lastBase, int16_t wLastBaseNumeric, int16_t wLastBaseStandard);
  void     clearTamBuffer                     (void);
  void     clearShiftState                    (void);
  void     showShiftStateF                    (void);
  void     showShiftStateG                    (void);
  void     displayShiftAndTamBuffer           (void);
  #endif // !TESTSUITE_BUILD
#endif // !SCREEN_H
