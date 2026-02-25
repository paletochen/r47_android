// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#if !defined(C47_H)
  #define C47_H

  #pragma GCC diagnostic ignored "-Wunused-parameter"

  #if defined(LINUX)
    #define _XOPEN_SOURCE                800 // see: https://stackoverflow.com/questions/5378778/what-does-d-xopen-source-do-mean
  #endif // LINUX

  #include <assert.h>
  #include <ctype.h>
  #include <errno.h>
  #include <inttypes.h>
  #include <libgen.h>
  #include <math.h>
  #include <stdbool.h>
  #include <stddef.h>
  #include <stdint.h>
  #include <stdlib.h>
  #include <stdio.h>
  #include <string.h>
  #include <sys/stat.h>
  #include <time.h>
  #include <unistd.h>

  #if defined(DMCP_BUILD) // this is due to the libc_nano added in 999acb23 which does not have hh support; also added nano to the new hardware in 2fde900
    #undef PRIu8
    #define PRIu8 "u"
    #undef PRIi8
    #define PRIi8 "i"
  #endif // DMCP_BUILD && OLD_HW

  #if !defined(GENERATE_CATALOGS) && !defined(GENERATE_CONSTANTS) && !defined(GENERATE_TESTPGMS)
    #include <gmp.h>

    #if defined(PC_BUILD)
      #ifndef ANDROID_BUILD
#include <gtk/gtk.h>
#endif
      #ifndef ANDROID_BUILD
#include <gdk/gdk.h>
#endif
    #endif // PC_BUILD

    #if defined(WIN32)
      #include <locale.h>
    #endif // WIN32

    #if defined(DMCP_BUILD)
      #define DBG_PRINT

      #if !defined(DBG_PRINT)
        #define printf(...)
      #endif

      #include <dmcp.h>
    #endif // DMCP_BUILD

    #include "defines.h"

    #include "decContext.h"
    #include "decNumber.h"
    #include "decQuad.h"
    #include "decimal128.h"

    #include "mathematics/pcg_basic.h"

    #include "realType.h"
    #include "typeDefinitions.h"
    #include "longIntegerType.h"

    #include "assign.h"
    #include "browsers/browsers.h"
    #include "bufferize.h"
    #include "c47Extensions/c47Extensions.h"
    #include "calcMode.h"
    #include "charString.h"
    #include "config.h"
    #include "constantPointers.h"
    #include "constants.h"
    #include "conversionAngles.h"
    #include "conversionUnits.h"
    #include "core/freeList.h"
    #include "curveFitting.h"
    #include "dateTime.h"
    #include "debug.h"
    #include "display.h"
    #include "distributions/distributions.h"
    #include "error.h"
    #include "flags.h"
    #include "fonts.h"
    #include "fractions.h"
    #include "hal/audio.h"
    #include "hal/gui.h"
    #include "hal/io.h"
    #include "hal/lcd.h"
    #include "integers.h"
    #include "items.h"
    #include "keyboard.h"
    #include "logicalOps/logicalOps.h"
    #include "mathematics/mathematics.h"
    #include "memory.h"
    #include "plotstat.h"
    #include "programming/programming.h"
    #include "recall.h"
    #include "registers.h"
    #include "registerValueConversions.h"
    #include "saveRestoreCalcState.h"
    #include "saveRestorePrograms.h"
    #include "screen.h"
    #include "softmenus.h"
    #include "solver/solver.h"
    #include "sort.h"
    #include "stack.h"
    #include "stats.h"
    #include "statusBar.h"
    #include "store.h"
    #include "stringFuncs.h"
    #include "timer.h"
    #include "ui/matrixEditor.h"
    #include "ui/tam.h"
    #include "ui/tone.h"

    #if defined(TESTSUITE_BUILD)
      #include "testSuite.h"
    #endif //TESTSUITE_BUILD
  #endif // PC_BUILD || DMCP_BUILD || TESTSUITE_BUILD

  #if defined(GENERATE_CATALOGS) || defined(GENERATE_TESTPGMS)
    #include <gmp.h>

    #ifndef ANDROID_BUILD
#include <gtk/gtk.h>
#endif
    #ifndef ANDROID_BUILD
#include <gdk/gdk.h>
#endif

    #include "defines.h"

    #include "decContext.h"
    #include "decNumber.h"
    #include "decQuad.h"
    #include "decimal128.h"

    #include "mathematics/pcg_basic.h"

    #include "realType.h"
    #include "typeDefinitions.h"
    #include "longIntegerType.h"

    #include "c47Extensions/c47Extensions.h"
    #include "charString.h"
    #include "config.h"
    #include "conversionUnits.h"
    #include "display.h"
    #include "fonts.h"
    #include "items.h"
    #include "mathematics/mathematics.h"
    #include "solver/solver.h"
    #include "sort.h"
    #include "stats.h"
  #endif // GENERATE_CATALOGS

  #if defined(GENERATE_CONSTANTS)
    #ifndef ANDROID_BUILD
#include <gtk/gtk.h>
#endif
    #ifndef ANDROID_BUILD
#include <gdk/gdk.h>
#endif

    #include "defines.h"

    #include "decContext.h"
    #include "decNumber.h"
    #include "decQuad.h"
    #include "decimal128.h"

    #include "mathematics/pcg_basic.h"

    #include "realType.h"
    #include "typeDefinitions.h"

  #endif // GENERATE_CONSTANTS

  #if defined(GENERATE_TESTPGMS)
    #ifndef ANDROID_BUILD
#include <gtk/gtk.h>
#endif
    #ifndef ANDROID_BUILD
#include <gdk/gdk.h>
#endif

    #include "defines.h"

    #include "decContext.h"
    #include "decNumber.h"
    #include "decQuad.h"
    #include "decimal128.h"

    #include "mathematics/pcg_basic.h"

    #include "realType.h"
    #include "typeDefinitions.h"

    #include "fonts.h"
    #include "items.h"
  #endif // GENERATE_TESTPGMS

  // Variables for the simulator
  #if !defined(GENERATE_CATALOGS) &&  !defined(GENERATE_TESTPGMS)
    extern uint16_t lastI;
    extern uint16_t lastJ;
    extern int16_t lastFunc;
    extern int16_t lastParam;
    extern char    lastTemp[16];
  #endif // !GENERATE_CATALOGS

  #if defined(PC_BUILD) || defined(TESTSUITE_BUILD)
    extern bool_t               debugMemAllocation;
    extern bool                 forceTamAlpha;
    extern uint32_t             deadKey;
    extern bool_t               testDeadKeys;
    extern bool_t               swapCtrlCode;
  #endif // PC_BUILD || TESTSUITE_BUILD

  #if defined(PC_BUILD)
    extern bool_t               calcLandscape;
    extern bool_t               calcAutoLandscapePortrait;
    extern GtkWidget           *screen;
    extern GtkWidget           *frmCalc;
    extern int16_t              screenStride;
    extern int16_t              debugWindow;
    extern uint32_t            *screenData;
    extern bool_t               screenChange;
    extern char                 debugString[10000];
    #if (DEBUG_REGISTER_L == 1)
      extern GtkWidget         *lblRegisterL1;
      extern GtkWidget         *lblRegisterL2;
    #endif // (DEBUG_REGISTER_L == 1)
    #if (SHOW_MEMORY_STATUS == 1)
      extern GtkWidget         *lblMemoryStatus;
    #endif // (SHOW_MEMORY_STATUS == 1)
    extern calcKeyboard_t       calcKeyboard[43];
    extern int                  currentBezel; // 0=normal, 1=AIM, 2=TAM
  #endif //PC_BUILD


  extern uint8_t calcModel;

  extern uint8_t             *lcd_buffer;
  extern const int           KEY_X[7];

  // Variables stored in FLASH
  extern const item_t                    indexOfItems[];
  extern const reservedVariableHeader_t  allReservedVariables[];
  extern const reservedVariableDescStr_t varDescr[];
  extern const char                      commonBugScreenMessages[NUMBER_OF_BUG_SCREEN_MESSAGES][SIZE_OF_EACH_BUG_SCREEN_MESSAGE];
  extern const char                      errorMessages[NUMBER_OF_ERROR_CODES][SIZE_OF_EACH_ERROR_MESSAGE];
  extern const calcKey_t                 kbd_std_C47[37];
  extern const calcKey_t                 kbd_std_DM42[37];
  extern const calcKey_t                 kbd_std_R47[37];
  extern const calcKey_t                 kbd_std_R47f_g[37];
  extern const calcKey_t                 kbd_std_R47bk_fg[37];
  extern const calcKey_t                 kbd_std_R47fg_bk[37];
  extern const calcKey_t                 kbd_std_R47fg_g[37];

  #if defined(PC_BUILD)
    #define kbd_std                      (calcModel == USER_C47 ? kbd_std_C47 : calcModel == USER_DM42 ? kbd_std_DM42 : calcModel == USER_R47f_g ? kbd_std_R47f_g : calcModel == USER_R47bk_fg ? kbd_std_R47bk_fg : calcModel == USER_R47fg_bk ? kbd_std_R47fg_bk : calcModel == USER_R47fg_g ? kbd_std_R47fg_g : \
                                          calcModel == USER_E47 ? kbd_std_E47 : calcModel == USER_D47 ?  kbd_std_D47 :  calcModel == USER_V47 ? kbd_std_V47 : calcModel == USER_N47 ?     kbd_std_N47 : calcModel == USER_DM42 ?     kbd_std_DM42 :    kbd_std_C47)
  #else //!PC_BUILD
    #define kbd_std                      (calcModel == USER_C47 ? kbd_std_C47 : calcModel == USER_DM42 ? kbd_std_DM42 : calcModel == USER_R47f_g ? kbd_std_R47f_g : calcModel == USER_R47bk_fg ? kbd_std_R47bk_fg : calcModel == USER_R47fg_bk ? kbd_std_R47fg_bk : calcModel == USER_R47fg_g ? kbd_std_R47fg_g : kbd_std_C47)
  #endif //!PC_BUILD

  #if defined(PC_BUILD)
    extern const calcKey_t                 kbd_std_WP43[37];
    extern const calcKey_t                 kbd_std_D47[37];
    extern const calcKey_t                 kbd_std_V47[37];
    extern const calcKey_t                 kbd_std_E47[37];
    extern const calcKey_t                 kbd_std_N47[37];
  #endif // PC_BUILD
  extern const font_t                    standardFont, numericFont, tinyFont;
  extern const font_t                   *fontForShortInteger;
  extern const font_t                   *cursorFont;
  extern const char                      baseDigits[63];
  extern const char                      registerFlagLetters[27];
  extern any34Matrix_t                   openMatrixMIMPointer;
  extern uint16_t                        matrixIndex;
  extern void                            (* const addition[NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS][NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS])(void);
  extern void                            (* const subtraction[NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS][NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS])(void);
  extern void                            (* const multiplication[NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS][NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS])(void);
  extern void                            (* const division[NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS][NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS])(void);
  extern void                            (*confirmedFunction)(uint16_t);
  extern const softmenu_t                softmenu[];
  extern const confirmationTI_t          confirmationTI[];

  #define gammaLanczosCoefficients       ((real51_t *)const_gammaC01)

  // Variables stored in RAM
  extern bool_t                 fnAsnDisplayUSER;
  extern bool_t                 funcOK;
  extern bool_t                 keyActionProcessed;
  extern bool_t                 fnKeyInCatalog;
  extern bool_t                 hourGlassIconEnabled;
  extern bool_t                 watchIconEnabled;
  extern bool_t                 printerIconEnabled;
  extern bool_t                 serialIOIconEnabled;
  extern bool_t                 shiftF;
  extern bool_t                 shiftG;
  extern bool_t                 lastshiftF;
  extern bool_t                 lastshiftG;
  extern bool_t                 showContent;
  extern bool_t                 rbr1stDigit;
  extern bool_t                 updateDisplayValueX;
  extern bool_t                 thereIsSomethingToUndo;
  extern bool_t                 lastProgramListEnd;
  extern bool_t                 programListEnd;
  extern bool_t                 pemCursorIsZerothStep;
  extern bool_t                 secTick1;
  extern bool_t                 halfSecTick2;
  extern bool_t                 halfSecTick3;
  extern bool_t                 skippedStackLines;
  extern bool_t                 iterations;
  extern bool_t                 explicitTaylorIterVisibilitySelection;

  extern bool_t                 reDraw;
  extern bool_t                 refreshNIMdone;
  extern bool_t                 cleanupAfterShift;
  extern bool_t                 solverEstimatesUsed;
  extern bool_t                 updateOldConstants;


  extern realContext_t          ctxtReal4;    //   Limited digits: used for high speed internal calcs
  extern realContext_t          ctxtReal34;   //   34 digits
  extern realContext_t          ctxtReal39;   //   39 digits: used for 34 digits intermediate calculations
  extern realContext_t          ctxtReal51;   //   51 digits: used for 34 digits intermediate calculations
  extern realContext_t          ctxtReal75;   //   75 digits: used in SLVQ

  extern dynamicSoftmenu_t      dynamicSoftmenu[NUMBER_OF_DYNAMIC_SOFTMENUS];

  extern subroutineLevels_t       allSubroutineLevels;
  extern subroutineLevelHeader_t *currentSubroutineLevelData;
  extern real_t                  *statisticalSumsPointer;
  extern real_t                  *savedStatisticalSumsPointer;
  extern uint32_t                *ram;
  extern localFlags_t            *currentLocalFlags;

  extern namedVariableHeader_t *allNamedVariables;
  extern softmenuStack_t        softmenuStack[SOFTMENU_STACK_SIZE];
  extern uint16_t               menuPageNumber;
  extern userMenuItem_t         userMenuItems[18];
  extern userMenuItem_t         userAlphaItems[18];
  extern userMenu_t            *userMenus;
  extern programmableMenu_t     programmableMenu;
  extern calcKey_t              kbd_usr[37];
  extern calcRegister_t         errorMessageRegisterLine;
  extern glyph_t                glyphNotFound;

  extern registerHeader_t      *currentLocalRegisters;

  #if defined(DMCP_BUILD) && defined(OLD_HW) // The old HW has 32Kb for global variables
    extern registerHeader_t       globalRegister[NUMBER_OF_GLOBAL_REGISTERS];
    extern freeMemoryRegion_t     freeMemoryRegions[MAX_FREE_REGIONS];
  #else // The new HW has only 16 KB for global variables, so some of them have to be moved elsewhere.
    extern registerHeader_t      *globalRegister;
    extern freeMemoryRegion_t    *freeMemoryRegions;
  #endif // DMCP_BUILD && OLD_HW

  #if !defined(DMCP_BUILD)
    extern freeMemoryRegion_t     allocatedMemoryRegions[MAX_ALLOCATED_REGIONS];
  #endif // !DMCP_BUILD

  extern pcg32_random_t         pcg32_global;
  extern labelList_t           *labelList;
  extern labelList_t           *flashLabelList;
  extern programList_t         *programList;
  extern programList_t         *flashProgramList;
  extern angularMode_t          currentAngularMode;
  extern formulaHeader_t       *allFormulae;

  extern uint8_t               *beginOfCurrentProgram;
  extern uint8_t               *endOfCurrentProgram;
  extern uint8_t               *firstDisplayedStep;
  extern uint8_t               *currentStep;

  extern char                  *tmpString;
  extern char                  *tmpStringLabelOrVariableName;
  extern char                  *errorMessage;
  extern char                  *aimBuffer; // aimBuffer is also used for NIM
  extern char                  *nimBufferDisplay;

  /**
   * Buffer for output of TAM current state. After calling ::tamProcessInput this
   * buffer is updated to the latest TAM state and should be redrawn to the relevant
   * part of the screen.
   */
  extern char                  *tamBuffer;
  extern char                  *userKeyLabel;
  extern char                   asmBuffer[5];
  extern char                   oldTime[8];
  extern char                   dateTimeString[12];
  extern char                   displayValueX[DISPLAY_VALUE_LEN];
  extern uint16_t               gapItemLeft;
  extern uint16_t               gapItemRight;
  extern uint16_t               gapItemRadix;
  extern uint16_t               lastCenturyHighUsed;
  extern uint8_t                numScreensStandardFont;
  extern uint8_t                numScreensNumericFont;
  extern uint8_t                numScreensTinyFont;
  extern uint8_t                currentAsnScr;
  extern uint8_t                currentFntScr;
  extern uint8_t                currentFlgScr;
  extern uint8_t                lastFlgScr;
  extern uint8_t                displayFormat;
  extern uint8_t                displayFormatDigits;
  extern uint8_t                timeDisplayFormatDigits;
  extern uint8_t                shortIntegerWordSize;
  extern uint8_t                significantDigits;
  extern uint8_t                dispBase;
  extern uint8_t                fractionDigits;
  extern uint8_t                shortIntegerMode;
  extern uint8_t                previousCalcMode;
  extern uint8_t                grpGroupingLeft;
  extern uint8_t                grpGroupingGr1LeftOverflow;
  extern uint8_t                grpGroupingGr1Left;
  extern uint8_t                grpGroupingRight;
  extern uint8_t                roundingMode;
  extern uint8_t                calcMode;
  extern uint8_t                nextChar;
  extern uint8_t                displayStack;
  extern uint8_t                cachedDisplayStack;
  extern uint8_t                displayStackSHOIDISP;         //JM SHOIDISP
  extern uint8_t                scrLock;
  extern uint8_t                alphaCase;
  extern uint8_t                numLinesNumericFont;
  extern uint8_t                numLinesStandardFont;
  extern uint8_t                numLinesTinyFont;
  extern uint8_t                cursorEnabled;
  extern uint8_t                nimNumberPart;
  extern uint8_t                nimRealPart;
  extern uint8_t                hexDigits;
  extern uint8_t                lastErrorCode;
  extern uint8_t                previousErrorCode;
  extern uint8_t                temporaryInformation;
  extern uint8_t                rbrMode;
  extern uint8_t                timerCraAndDeciseconds;
  extern uint8_t                programRunStop;
  extern uint8_t                currentKeyCode;
  extern uint8_t                lastKeyCode;
  extern uint8_t                keyStateCode;
  extern uint8_t                entryStatus; // 0x01 for the entry flag, backed up to 0x02 for undo
  extern uint8_t                screenUpdatingMode;
  extern uint8_t               *beginOfProgramMemory;
  extern uint8_t               *firstFreeProgramByte;
  extern bool_t                 statisticalSumsUpdate;

  /**
   * Instance of the internal state for TAM.
   */
  extern tamState_t             tam;
  extern int16_t                currentRegisterBrowserScreen;
  extern int16_t                lineTWidth;
  extern int16_t                rbrRegister;
  extern int16_t                catalog;
  extern int16_t                lastCatalogPosition[NUMBER_OF_CATALOGS];
  extern int16_t                lastKeyItemDetermined;
  extern bool_t                 lastUserMode;                 //used in btnReleased and btnFnReleased
  extern int16_t                lastItem;                     //used in btnReleased, for CM_ASN_BROWSER and SHOW/SCREENDUMP
  extern int16_t                showFunctionNameItem;
  extern char *                 showFunctionNameArg;
  extern int16_t                exponentSignLocation;
  extern int16_t                denominatorLocation;
  extern int16_t                imaginaryExponentSignLocation;
  extern int16_t                imaginaryMantissaSignLocation;
  extern int16_t                imaginaryDenominatorLocation;
  extern int16_t                exponentLimit;
  extern int16_t                exponentHideLimit;
  extern int16_t                showFunctionNameCounter;
  extern int16_t                dynamicMenuItem;
  extern int16_t               *menu_RAM;
  extern int16_t                numberOfTamMenusToPop;
  extern int16_t                itemToBeAssigned;
  extern int16_t                cachedDynamicMenu;

  extern uint16_t               globalFlags[8];
  extern int16_t                longpressDelayedkey2;         //JM
  extern int16_t                longpressDelayedkey3;         //JM
  extern int16_t                T_cursorPos;                  //JMCURSOR
  extern uint8_t                multiEdLines;
  extern uint8_t                yMultiLineEdOffset;
  extern uint8_t                xMultiLineEdOffset;
  extern uint16_t               current_cursor_x;
  extern uint16_t               current_cursor_y;
  extern int16_t                alphaCursor;                  //DL
  extern int16_t                lastT_cursorPos;
  extern int16_t                displayAIMbufferoffset;       //JMCURSOR
  extern uint16_t               showRegis;                    //JMSHOW
  extern uint8_t                overrideShowBottomLine;
  extern int16_t                ListXYposition;               //JM
  extern uint8_t                DRG_Cycling;                  //JM
  extern uint8_t                DM_Cycling;                   //JM
  extern int16_t                JM_auto_doublepress_autodrop_enabled;  //JM TIMER CLRDROP //drop
  extern int16_t                JM_auto_longpress_enabled;    //JM TIMER CLRDROP //clstk
  extern uint8_t                JM_SHIFT_HOME_TIMER1;         //Local to keyboard.c, but defined here
  extern bool_t                 ULFL, ULGL;                   //JM Underline
  extern int16_t                FN_key_pressed, FN_key_pressed_last; //JM LONGPRESS FN
  extern bool_t                 FN_timeouts_in_progress;      //JM LONGPRESS FN
  extern bool_t                 Shft_timeouts;                //JM SHIFT NEW FN
  extern bool_t                 Shft_LongPress_f_g;           //JM SHIFT longpress on f and on g
  extern bool_t                 FN_timed_out_to_NOP_or_Executed; //JM LONGPRESS FN
  extern bool_t                 FN_timed_out_to_RELEASE_EXEC; //JM LONGPRESS FN
  extern bool_t                 FN_handle_timed_out_to_EXEC;
  extern uint8_t                bcdDisplaySign;
  extern uint8_t                LongPressM;
  extern uint8_t                LongPressF;
  extern uint8_t                last_CM;                      //Do extern !!
  extern uint8_t                FN_state; // = ST_0_INIT;
  extern uint8_t                editingLiteralType;

  // Variables from jm.h
  extern normKey_t              Norm_Key_00;                  //JM USER NORMAL
  extern uint8_t                Input_Default;                //JM Input Default
  extern uint8_t                IrFractionsCurrentStatus;     //JM
  extern bool_t                 tvmIKnown;
  extern bool_t                 tvmIChanges;


  extern uint16_t               glyphRow[NUMBER_OF_GLYPH_ROWS];
  extern uint16_t               freeProgramBytes;
  extern uint16_t               firstDisplayedLocalStepNumber;
  extern uint16_t               numberOfLabels;
  extern uint16_t               numberOfPrograms;
  extern uint16_t               numberOfNamedVariables;
  extern uint16_t               currentLocalStepNumber;
  extern uint16_t               currentProgramNumber;
  extern uint16_t               lrSelection;
  extern uint16_t               lrSelectionUndo;
  extern uint16_t               lrChosen;
  extern uint16_t               lrChosenUndo;
  extern uint16_t               lastPlotMode;
  extern uint16_t               plotSelection;
  extern uint16_t               currentViewRegister;
  extern uint16_t               currentSolverStatus;
  extern uint16_t               currentSolverProgram;
  extern uint16_t               currentSolverVariable;
  extern uint16_t               currentSolverNestingDepth;
  extern uint16_t               numberOfFormulae;
  extern uint16_t               currentFormula;
  extern uint16_t               numberOfUserMenus;
  extern uint16_t               currentUserMenu;
  extern uint16_t               userKeyLabelSize;
  extern uint16_t               currentInputVariable;
  extern uint16_t               currentMvarLabel;
  #if (REAL34_WIDTH_TEST == 1)
    extern uint16_t               largeur;
  #endif // (REAL34_WIDTH_TEST == 1)

  extern int32_t                numberOfFreeMemoryRegions;

  #if !defined(DMCP_BUILD)
    extern int32_t                numberOfAllocatedMemoryRegions;
  #endif // !DMCP_BUILD

  extern int32_t                lgCatalogSelection;
  extern calcRegister_t         graphVariabl1;

  extern uint32_t               firstGregorianDay;
  extern uint32_t               denMax;
  extern uint32_t               lastDenominator;
  extern uint32_t               lastIntegerBase;
  extern uint32_t               decodedIntegerBase;
  extern uint32_t               xCursor;
  extern uint32_t               yCursor;
  extern uint32_t               tamOverPemYPos;
  extern uint32_t               timerValue;
  extern uint32_t               timerStartTime;
  extern uint32_t               timerTotalTime;
  extern uint32_t               pointerOfFlashPgmLibrary;
  extern uint32_t               sizeOfFlashPgmLibrary;

  extern uint64_t               shortIntegerMask;
  extern uint64_t               shortIntegerSignBit;
  extern uint64_t               systemFlags0;
  extern uint64_t               systemFlags1;
  extern uint64_t               savedSystemFlags0;
  extern uint64_t               savedSystemFlags1;

  extern size_t                 gmpMemInBytes;
  extern size_t                 c47MemInBlocks;

  extern real_t                 SAVED_SIGMA_LASTX;
  extern real_t                 SAVED_SIGMA_LASTY;
  extern int8_t                 SAVED_SIGMA_lastAddRem;

  extern uint16_t               lrSelectionHistobackup;
  extern uint16_t               lrChosenHistobackup;
  extern int16_t                histElementXorY;
  extern real34_t               loBinR;
  extern real34_t               nBins ;
  extern real34_t               hiBinR;
  extern char                   statMx[8];
  extern char                   plotStatMx[8];
  extern calcRegister_t         regStatsXY;


  extern bool_t                 temporaryFlagRect;
  extern bool_t                 temporaryFlagPolar;
  extern int                    vbatVIntegrated;

  extern uint32_t               timeLastOp;
  extern uint32_t               timeLastOp0;
  extern uint32_t               timeLastOp1;

  #define stateFileNameVarLength 20
  extern  char                  lastStateFileOpened[stateFileNameVarLength+12];
  extern  char                  fileNameSelected[stateFileNameVarLength];

  extern char         filename_csv[FILENAMELEN]; //JMMAX                //JM_CSV
  extern uint32_t     mem__32;                                          //JM_CSV
  extern bool_t       cancelFilename;

  extern uint8_t                firstDayOfWeek;
  extern uint8_t                firstWeekOfYearDay;

  #if defined(DMCP_BUILD)
    extern bool_t              backToDMCP;
  #if defined(BUFFER_CLICK_DETECTION)
    extern uint32_t            timeStampKey;                                      //dr - internal keyBuffer POC
  #endif // BUFFER_CLICK_DETECTION
  //extern int                  keyAutoRepeat; // Key repetition
  //extern int16_t              previousItem;
  extern uint32_t             nextTimerRefresh;

  int                         convertKeyCode(int key);
  #endif // DMCP_BUILD
#endif // !C47_H
