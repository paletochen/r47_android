// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"

#include "c47Extensions/addons.h"
#include "longIntegerType.h"
#include "saveRestoreCalcState.h"
#include "statusBar.h"

//#define JMSHOWCODES
//#define BUFFER_CLICK_DETECTION

#if defined(DMCP_BUILD)
  #include "c47Extensions/inlineTest.h"
  #include "c47Extensions/jm.h"
  #include "c47Extensions/keyboardTweak.h"
#endif

#if !defined(GENERATE_CATALOGS)
  uint16_t lastI = 0;
  uint16_t lastJ = 0;
  int16_t lastFunc = 0;
  int16_t lastParam = 0;
  char    lastTemp[16];
#endif // !GENERATE_CATALOGS
#if defined(PC_BUILD) || defined(TESTSUITE_BUILD)
  bool_t              debugMemAllocation;
  bool                forceTamAlpha;
  uint32_t            deadKey;
  bool_t              testDeadKeys = false;
  bool_t              swapCtrlCode = false;
#endif // PC_BUILD || TESTSUITE_BUILD

const font_t          *fontForShortInteger;
const font_t          *cursorFont;
TO_QSPI const char     baseDigits[63] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
TO_QSPI const char     registerFlagLetters[27] = "XYZTABCDLIJKMNPQRSEFGHOUVW";
void                   (*confirmedFunction)(uint16_t);
TO_QSPI const int      KEY_X[7] = {-1, 66, 133, 200, 267, 333, 400}; // Softkey border positions

uint8_t calcModel = (CALCMODEL == USER_R47 ? USER_R47f_g : CALCMODEL);          //Default set by compiler is "USER_R47" and the profile is changed to USER_R47f_g


// Variables stored in RAM
bool_t                 funcOK;
bool_t                 keyActionProcessed;
bool_t                 fnKeyInCatalog;
bool_t                 hourGlassIconEnabled;
bool_t                 watchIconEnabled;
bool_t                 printerIconEnabled;
bool_t                 serialIOIconEnabled;
bool_t                 shiftF;
bool_t                 shiftG;
bool_t                 lastshiftF = false;
bool_t                 lastshiftG = false;
bool_t                 showContent;
bool_t                 rbr1stDigit;
bool_t                 updateDisplayValueX;
bool_t                 thereIsSomethingToUndo;
bool_t                 lastProgramListEnd;
bool_t                 programListEnd;
bool_t                 pemCursorIsZerothStep;
bool_t                 secTick1;
bool_t                 halfSecTick2;
bool_t                 halfSecTick3;
bool_t                 skippedStackLines = false;
bool_t                 iterations = false;
bool_t                 explicitTaylorIterVisibilitySelection = false;

bool_t                 reDraw = true;
bool_t                 refreshNIMdone = false;
bool_t                 cleanupAfterShift = false;
bool_t                 solverEstimatesUsed = false;
bool_t                 updateOldConstants;


realContext_t          ctxtReal4;    //   limited digits: used for higher speed internal real calcs
realContext_t          ctxtReal34;   //   34 digits
realContext_t          ctxtReal39;   //   39 digits: used for 34 digits intermediate calculations
realContext_t          ctxtReal51;   //   51 digits: used for 34 digits intermediate calculations
realContext_t          ctxtReal75;   //   75 digits: used in SLVQ

subroutineLevels_t       allSubroutineLevels;
subroutineLevelHeader_t *currentSubroutineLevelData;
real_t                  *statisticalSumsPointer;
real_t                  *savedStatisticalSumsPointer;
uint32_t                *ram = NULL;
localFlags_t            *currentLocalFlags;

namedVariableHeader_t *allNamedVariables;
softmenuStack_t        softmenuStack[SOFTMENU_STACK_SIZE];
uint16_t               menuPageNumber;
userMenuItem_t         userMenuItems[18];
userMenuItem_t         userAlphaItems[18];
userMenu_t            *userMenus;
programmableMenu_t     programmableMenu;
calcKey_t              kbd_usr[37];
calcRegister_t         errorMessageRegisterLine;
glyph_t                glyphNotFound = {.charCode = 0x0000, .colsBeforeGlyph = 0, .colsGlyph = 13, .colsAfterGlyph = 0, .rowsGlyph = 19, .data = NULL};

registerHeader_t      *currentLocalRegisters;

#if defined(DMCP_BUILD) && defined(OLD_HW) // The old HW has 32Kb for global variables
  registerHeader_t       globalRegister[NUMBER_OF_GLOBAL_REGISTERS];
  freeMemoryRegion_t     freeMemoryRegions[MAX_FREE_REGIONS];
#else // The new HW has only 16 KB for global variables, so some of them have to be moved elsewhere.
  registerHeader_t      *globalRegister = NULL;
  freeMemoryRegion_t    *freeMemoryRegions = NULL;
#endif // DMCP_BUILD && OLD_HW

#if !defined(DMCP_BUILD)
  freeMemoryRegion_t     allocatedMemoryRegions[MAX_ALLOCATED_REGIONS];
#endif // !DMCP_BUILD

pcg32_random_t         pcg32_global = PCG32_INITIALIZER;
labelList_t           *labelList = NULL;
programList_t         *programList = NULL;
angularMode_t          currentAngularMode;
formulaHeader_t       *allFormulae;

uint8_t               *beginOfCurrentProgram;
uint8_t               *endOfCurrentProgram;
uint8_t               *firstDisplayedStep;
uint8_t               *currentStep;

char                  *tmpString = NULL;
char                  *tmpStringLabelOrVariableName = NULL;
char                  *errorMessage;
char                  *aimBuffer; // aimBuffer is also used for NIM
char                  *nimBufferDisplay;
char                  *tamBuffer;
char                  *userKeyLabel;
char                   asmBuffer[5];
char                   oldTime[8];
char                   dateTimeString[12];
char                   displayValueX[DISPLAY_VALUE_LEN];
//char                   gapCharLeft[3];
//char                   gapCharRadix[3];
//char                   gapCharRight[3];
uint16_t               gapItemLeft;
uint16_t               gapItemRight;
uint16_t               gapItemRadix;
uint16_t               lastCenturyHighUsed = 0;

uint8_t               *lcd_buffer;
uint8_t                numScreensStandardFont;
uint8_t                numScreensNumericFont;
uint8_t                numScreensTinyFont;
uint8_t                currentAsnScr;
uint8_t                currentFntScr;
uint8_t                currentFlgScr;
uint8_t                lastFlgScr;
uint8_t                displayFormat;
uint8_t                displayFormatDigits;
uint8_t                timeDisplayFormatDigits;
uint8_t                shortIntegerWordSize;
uint8_t                significantDigits;
uint8_t                dispBase;
uint8_t                fractionDigits;
uint8_t                shortIntegerMode;
uint8_t                previousCalcMode;
uint8_t                grpGroupingLeft;
uint8_t                grpGroupingGr1LeftOverflow;
uint8_t                grpGroupingGr1Left;
uint8_t                grpGroupingRight;
uint8_t                roundingMode;
uint8_t                calcMode;
uint8_t                nextChar;
uint8_t                displayStack;
uint8_t                cachedDisplayStack;
uint8_t                alphaCase;
uint8_t                numLinesNumericFont;
uint8_t                numLinesStandardFont;
uint8_t                numLinesTinyFont;
uint8_t                cursorEnabled;
uint8_t                nimNumberPart;
uint8_t                nimRealPart;
uint8_t                hexDigits;
uint8_t                lastErrorCode;
uint8_t                previousErrorCode;
uint8_t                temporaryInformation;
uint8_t                rbrMode;
uint8_t                timerCraAndDeciseconds = 128u;
uint8_t                programRunStop;
uint8_t                currentKeyCode;
uint8_t                lastKeyCode;
uint8_t                keyStateCode;
uint8_t                entryStatus;
uint8_t                screenUpdatingMode;
uint8_t               *beginOfProgramMemory;
uint8_t               *firstFreeProgramByte;
bool_t                 statisticalSumsUpdate;

tamState_t             tam;
int16_t                currentRegisterBrowserScreen;
int16_t                lineTWidth;
int16_t                rbrRegister;
int16_t                catalog;
int16_t                lastCatalogPosition[NUMBER_OF_CATALOGS];
int16_t                lastKeyItemDetermined = 0;
bool_t                 lastUserMode = false;         //used in btnReleased and btnFnReleased
int16_t                lastItem = 0;                 //used in btnReleased, for CM_ASN_BROWSER and SHOW/SCREENDUMP
int16_t                showFunctionNameItem;
char *                 showFunctionNameArg;

uint8_t               displayStackSHOIDISP;          //JM SHOIDISP
uint8_t               scrLock;
bool_t                doRefreshSoftMenu;                       //dr
uint8_t               IrFractionsCurrentStatus;
bool_t                tvmIKnown;
bool_t                tvmIChanges;

normKey_t             Norm_Key_00;                             //JM USER NORMAL
uint8_t               Input_Default;                           //JM Input Default
uint8_t               DRG_Cycling = 0;
uint8_t               DM_Cycling = 0;
#if defined(INLINE_TEST)                                             //vv dr
  bool_t                testEnabled;                             //
  uint16_t              testBitset;                              //
#endif // INLINE_TEST                                                       //^^
int16_t                longpressDelayedkey2;         //JM
int16_t                longpressDelayedkey3;         //JM
int16_t                T_cursorPos;                  //JMCURSOR
uint8_t                multiEdLines = 0;
uint8_t                yMultiLineEdOffset = 0;
uint8_t                xMultiLineEdOffset = 0;
uint16_t               current_cursor_x = 0;
uint16_t               current_cursor_y = 0;
int16_t                alphaCursor;                  //DL
int16_t                lastT_cursorPos = 0;
int16_t                displayAIMbufferoffset;       //JMCURSOR
uint16_t               showRegis;                    //JMSHOW
uint8_t                overrideShowBottomLine;
int16_t                ListXYposition;               //JMSHOW
int16_t                JM_auto_doublepress_autodrop_enabled;  //JM TIMER CLRDROP //drop
int16_t                JM_auto_longpress_enabled;    //JM TIMER CLRDROP //clstk
uint8_t                JM_SHIFT_HOME_TIMER1;         //Local to keyboard.c, but defined here
bool_t                 ULFL, ULGL;                   //JM Underline
int16_t                FN_key_pressed, FN_key_pressed_last; //JM LONGPRESS FN
bool_t                 FN_timeouts_in_progress;      //JM LONGPRESS FN
bool_t                 Shft_timeouts;                //JM SHIFT NEW FN
bool_t                 Shft_LongPress_f_g;           //JM SHIFT longpress on f and on g
bool_t                 FN_timed_out_to_NOP_or_Executed; //JM LONGPRESS FN
bool_t                 FN_timed_out_to_RELEASE_EXEC; //JM LONGPRESS FN
bool_t                 FN_handle_timed_out_to_EXEC;
bool_t                 fnAsnDisplayUSER = true;

uint8_t                bcdDisplaySign = 0;
uint8_t                LongPressM = 0;
uint8_t                LongPressF = 0;
uint8_t                last_CM = 255;                //Do extern !!
uint8_t                FN_state; // = ST_0_INIT;
uint8_t                editingLiteralType;

int16_t                exponentSignLocation;
int16_t                denominatorLocation;
int16_t                imaginaryExponentSignLocation;
int16_t                imaginaryMantissaSignLocation;
int16_t                imaginaryDenominatorLocation;
int16_t                exponentLimit;
int16_t                exponentHideLimit;
int16_t                showFunctionNameCounter;
int16_t                dynamicMenuItem;
int16_t               *menu_RAM;
int16_t                numberOfTamMenusToPop;
int16_t                itemToBeAssigned;
int16_t                cachedDynamicMenu;

uint16_t               globalFlags[8];
uint16_t               freeProgramBytes;
uint16_t               glyphRow[NUMBER_OF_GLYPH_ROWS];
uint16_t               firstDisplayedLocalStepNumber;
uint16_t               numberOfLabels;
uint16_t               numberOfPrograms;
uint16_t               numberOfNamedVariables;
uint16_t               currentLocalStepNumber;
uint16_t               currentProgramNumber;
uint16_t               lrSelection;
uint16_t               lrSelectionUndo;
uint16_t               lrChosen;
uint16_t               lrChosenUndo;
uint16_t               lastPlotMode;
uint16_t               plotSelection;
uint16_t               currentViewRegister;
uint16_t               currentSolverStatus;
uint16_t               currentSolverProgram;
uint16_t               currentSolverVariable;
uint16_t               currentSolverNestingDepth;
uint16_t               numberOfFormulae;
uint16_t               currentFormula;
uint16_t               numberOfUserMenus;
uint16_t               currentUserMenu;
uint16_t               userKeyLabelSize;
uint16_t               currentInputVariable = INVALID_VARIABLE;
uint16_t               currentMvarLabel = INVALID_VARIABLE;
#if (REAL34_WIDTH_TEST == 1)
  uint16_t               largeur=200;
#endif // (REAL34_WIDTH_TEST == 1)

int32_t                numberOfFreeMemoryRegions;

#if !defined(DMCP_BUILD)
  int32_t                numberOfAllocatedMemoryRegions;
#endif // !DMCP_BUILD

int32_t                lgCatalogSelection;
calcRegister_t         graphVariabl1;

uint32_t               firstGregorianDay;
uint32_t               denMax;
uint32_t               lastDenominator = 4;
uint32_t               lastIntegerBase;
uint32_t               decodedIntegerBase;
uint32_t               xCursor;
uint32_t               yCursor;
uint32_t               tamOverPemYPos;
uint32_t               timerValue;
uint32_t               timerStartTime = TIMER_APP_STOPPED;
uint32_t               timerTotalTime;
uint32_t               pointerOfFlashPgmLibrary;
uint32_t               sizeOfFlashPgmLibrary;

uint64_t               shortIntegerMask;
uint64_t               shortIntegerSignBit;
uint64_t               systemFlags0;
uint64_t               systemFlags1;
uint64_t               savedSystemFlags0;
uint64_t               savedSystemFlags1;

size_t                 gmpMemInBytes;
size_t                 c47MemInBlocks;

real_t                 SAVED_SIGMA_LASTX;
real_t                 SAVED_SIGMA_LASTY;
int8_t                 SAVED_SIGMA_lastAddRem;

uint16_t               lrSelectionHistobackup;
uint16_t               lrChosenHistobackup;
int16_t                histElementXorY;
real34_t               loBinR;
real34_t               nBins;
real34_t               hiBinR;
char                   statMx[8];
char                   plotStatMx[8];
calcRegister_t         regStatsXY;


bool_t                 temporaryFlagRect;
bool_t                 temporaryFlagPolar;
int                    vbatVIntegrated = 3000;
uint32_t               timeLastOp = 0;
uint32_t               timeLastOp0 = 0;
uint32_t               timeLastOp1 = 0;
char                   lastStateFileOpened[stateFileNameVarLength+12];
char                   fileNameSelected[stateFileNameVarLength];

char                   filename_csv[FILENAMELEN]; //JMMAX   //JM_CSV
uint32_t               mem__32;                             //JM_CSV
bool_t                 cancelFilename;

uint8_t                firstDayOfWeek = 1;     // Monday
uint8_t                firstWeekOfYearDay = 4; // Thursday


#if defined(DMCP_BUILD)

#if (CALCMODEL == USER_C47) && (HARDWARE_MODEL == HWM_DM42) // include DM42 QSPI
  IMPORT_BIN(".qspi_dm42", "../c47-dmcp/DM42_qspi_3.x.bin", DM42_qspi);
#endif  // include DM42 QSPI

  #if defined(JMSHOWCODES)                                        //JM Test
    int8_t            telltale_pos;                         //JM Test
    int8_t            telltale_lastkey;                     //JM Test
  #endif // JMSHOWCODES                                      //JM Test
#if defined(BUFFER_CLICK_DETECTION)
  uint32_t            timeStampKey;                         //dr - internal keyBuffer POC
#endif // BUFFER_CLICK_DETECTION
  bool_t              backToDMCP;
//int                  keyAutoRepeat;
//int16_t              previousItem;
  uint32_t             nextTimerRefresh;
  uint32_t             nextScreenRefresh; // timer substitute for refreshLcd(), which does cursor blinking and other stuff
  bool_t               wp43KbdLayout;


#if CALCMODEL == USER_R47 // R47 alpha keyboard mapping
  //Alpha keyboard mapping for DMCP based on the DM41X example from David
  // includes keycodes remapping for the R47
  //
  const char alpha_upper_transl[] = "_" // code 0 unused
  //        +-------+-------+-------+-------+-------+-------+
  //  1- 6  |  x^2  |  SQRT |  1/x  |  y^x  |  LOG  |  LN   |
               "A"     "B"     "C"     "D"     "E"     "F"
  //        +-------+-------+-------+-------+-------+-------+
  //  7-12  |  STO  |  RCL  |  R↓   |  DRG  |   f   |   g   |
               "G"     "H"     "I"     "J"     "@"     "@"
  //        +-------+-------+-------+-------+-------+
  // 13-17  | ENTER | x<>y  |  +/-  |   E   |  <--  |
               "@"     "K"     "M"     "L"     "@"
  //        +-------+-------+-------+-------+-------+
  // 18-22  |  XEQ  |   7   |   8   |   9   |  DIV  |
               "@"     "N"     "O"     "P"     "Q"
  //        +-------+-------+-------+-------+-------+
  // 23-27  |  UP   |   4   |   5   |   6   |  MUL  |
               "@"     "R"     "S"     "T"     "U"
  //        +-------+-------+-------+-------+-------+
  // 28-32  |  DOWN |   1   |   2   |   3   |  SUB  |
               "@"     "V"     "W"     "X"     "Y"
  //        +-------+-------+-------+-------+-------+
  // 33-37  |  ON   |   0   |  DOT  |  R/S  |  ADD  |
               "@"     "Z"     ","     "?"     " "  ;
  //The following lines from the DM41X are not included for the WP43 as
  // there is no alpha character assigned to the function keys
  //        +-------+-------+-------+-------+-------+-------+
  // 38-43  |  Σ+   |  1/x  | SQRT  |  LOG  |  LN   |  PRG  |
  //           "A"     "B"     "C"     "D"     "E"     "@"  ;
#endif // CALCMODEL == USER_R47

int convertKeyCode(int key) {
  #if CALCMODEL == USER_R47 // R47 keyboard mapping
    /////////////////////////////////////////////////
    // For key reassignment see:
    // https://technical.swissmicros.com/dmcp/devel/dmcp_devel_manual/#_system_key_table
    //
    // Output of keymap2layout keymap.txt
    //     +-----+-----+-----+-----+-----+-----+
    //  1: | F1  | F2  | F3  | F4  | F5  | F6  |
    //     |38:38|39:39|40:40|41:41|42:42|43:43|
    //    +-----+-----+-----+-----+-----+-----+
    //  2: |Sum+ | 1/x |SQRT | LOG | LN  | XEQ |
    //     | 1: 1| 2: 2| 3: 3| 4: 4| 5: 5| 6: 6|
    //     +-----+-----+-----+-----+-----+-----+
    //  3: | STO | RCL | RDN | SIN |SHIFT| TAN |
    //     | 7: 7| 8: 8| 9: 9|10:10|11:28|12:12|
    //     +-----+-----+-----+-----+-----+-----+
    //  4: |   ENTER   |x<>y |  E  | CHS | <-- |
    //     |   13:13   |14:14|15:16|16:15|17:17|
    //     +-----------+-----+-----+-----+-----+
    //  5: |  COS |   7  |   8  |   9  |  DIV  |
    //     | 18:11| 19:19| 20:20| 21:21| 22:22 |
    //     +------+------+------+------+-------+
    //  6: |  UP  |   4  |   5  |   6  |  MUL  |
    //     | 23:18| 24:24| 25:25| 26:26| 27:27 |
    //     +------+------+------+------+-------+
    //  7: | DOWN |   1  |   2  |   3  |  SUB  |
    //     | 28:23| 29:29| 30:30| 31:31| 32:32 |
    //     +------+------+------+------+-------+
    //  8: | EXIT |   0  |  DOT |  RUN |  ADD  |
    //     | 33:33| 34:34| 35:35| 36:36| 37:37 |
    //     +------+------+------+------+-------+

    //The switch instruction below is implemented as follows e.g. for the up arrow key on the R47 layout:
    //  the output of keymap2layout for this key is UP 23:18 so we need the line:
    //    case 18: key = 23; break;
    switch(key) {               // Original
    //case  1: key =  1; break; // SUM+
    //case  2: key =  2; break; // 1/x
    //case  3: key =  3; break; // SQRT
    //case  4: key =  4; break; // LOG
    //case  5: key =  5; break; // LN
    //case  6: key =  6; break; // XEQ
    //case  7: key =  7; break; // STO
    //case  8: key =  8; break; // RCL
    //case  9: key =  9; break; // RDN
    //case 10: key = 10; break; // SIN
      case 11: key = 18; break; // COS
    //case 12: key = 12; break; // TAN
    //case 13: key = 13; break; // ENTER
    //case 14: key = 14; break; // x<>y
      case 15: key = 16; break; // +/-
      case 16: key = 15; break; // E
    //case 17: key = 17; break; // <--
      case 18: key = 23; break; // UP
    //case 19: key = 19; break; // 7
    //case 20: key = 20; break; // 8
    //case 21: key = 21; break; // 9
    //case 22: key = 18; break; // /
      case 23: key = 28; break; // DOWN
    //case 24: key = 24; break; // 4
    //case 25: key = 25; break; // 5
    //case 26: key = 26; break; // 6
    //case 27: key = 27; break; // x
      case 28: key = 11; break; // SHIFT
    //case 29: key = 29; break; // 1
    //case 30: key = 30; break; // 2
    //case 31: key = 31; break; // 3
    //case 32: key = 32; break; // -
    //case 33: key = 33; break; // EXIT
    //case 34: key = 34; break; // 0
    //case 35: key = 35; break; // .
    //case 36: key = 36; break; // R/S
    //case 37: key = 37; break; // +
      default: {
      }
    }
  #else // Default
                                                  if(wp43KbdLayout) {
                                                    /////////////////////////////////////////////////
                                                    // For key reassignment see:
                                                    // https://technical.swissmicros.com/dmcp/devel/dmcp_devel_manual/#_system_key_table
                                                    //
                                                    // Output of keymap2layout keymap.txt
                                                    //
                                                    //    +-----+-----+-----+-----+-----+-----+
                                                    // 1: | F1  | F2  | F3  | F4  | F5  | F6  |
                                                    //    |38:38|39:39|40:40|41:41|42:42|43:43|
                                                    //    +-----+-----+-----+-----+-----+-----+
                                                    // 2: | 1/x |Sum+ | SIN | LN  | LOG |SQRT |
                                                    //    | 1: 2| 2: 1| 3:10| 4: 5| 5: 4| 6: 3|
                                                    //    +-----+-----+-----+-----+-----+-----+
                                                    // 3: | STO | RCL | RDN | COS | TAN |SHIFT|
                                                    //    | 7: 7| 8: 8| 9: 9|10:11|11:12|12:28|
                                                    //    +-----+-----+-----+-----+-----+-----+
                                                    // 4: |   ENTER   |x<>y | CHS |  E  | <-- |
                                                    //    |   13:13   |14:14|15:15|16:16|17:17|
                                                    //    +-----------+-----+-----+-----+-----+
                                                    // 5: |  DIV |   7  |   8  |   9  |  XEQ  |
                                                    //    | 18:22| 19:19| 20:20| 21:21| 22: 6 |
                                                    //    +------+------+------+------+-------+
                                                    // 6: |  MUL |   4  |   5  |   6  |  UP   |
                                                    //    | 23:27| 24:24| 25:25| 26:26| 27:18 |
                                                    //    +------+------+------+------+-------+
                                                    // 7: |  SUB |   1  |   2  |   3  | DOWN  |
                                                    //    | 28:32| 29:29| 30:30| 31:31| 32:23 |
                                                    //    +------+------+------+------+-------+
                                                    // 8: |  ADD |   0  |  DOT |  RUN | EXIT  |
                                                    //    | 33:37| 34:34| 35:35| 36:36| 37:33 |
                                                    //    +------+------+------+------+-------+

                                                    //The switch instruction below is implemented as follows e.g. for the up arrow key on the WP43 layout:
                                                    //  the output of keymap2layout for this key is UP 27:18 so we need the line:
                                                    //    case 18: key = 27; break;
                                                    switch(key) {               // Original
                                                      case  1: key =  2; break; // SUM+
                                                      case  2: key =  1; break; // 1/x
                                                      case  3: key =  6; break; // SQRT
                                                      case  4: key =  5; break; // LOG
                                                      case  5: key =  4; break; // LN
                                                      case  6: key = 22; break; // XEQ
                                                    //case  7: key =  7; break; // STO
                                                    //case  8: key =  8; break; // RCL
                                                    //case  9: key =  9; break; // RDN
                                                      case 10: key =  3; break; // SIN
                                                      case 11: key = 10; break; // COS
                                                      case 12: key = 11; break; // TAN
                                                    //case 13: key = 13; break; // ENTER
                                                    //case 14: key = 14; break; // x<>y
                                                    //case 15: key = 15; break; // +/-
                                                    //case 16: key = 16; break; // E
                                                    //case 17: key = 17; break; // <--
                                                      case 18: key = 27; break; // UP
                                                    //case 19: key = 19; break; // 7
                                                    //case 20: key = 20; break; // 8
                                                    //case 21: key = 21; break; // 9
                                                      case 22: key = 18; break; // /
                                                      case 23: key = 32; break; // DOWN
                                                    //case 24: key = 24; break; // 4
                                                    //case 25: key = 25; break; // 5
                                                    //case 26: key = 26; break; // 6
                                                      case 27: key = 23; break; // x
                                                      case 28: key = 12; break; // SHIFT
                                                    //case 29: key = 29; break; // 1
                                                    //case 30: key = 30; break; // 2
                                                    //case 31: key = 31; break; // 3
                                                      case 32: key = 28; break; // -
                                                      case 33: key = 37; break; // EXIT
                                                    //case 34: key = 34; break; // 0
                                                    //case 35: key = 35; break; // .
                                                    //case 36: key = 36; break; // R/S
                                                      case 37: key = 33; break; // +
                                                      default: {
                                                      }
                                                    }
                                                  }
  #endif // CALCMODEL == USER_R47
  return key;
}

                                                #if defined(INLINE_TEST)
                                                //#define TMR_OBSERVE
                                                #endif // INLINE_TEST

  void program_main(void) {
    int key = 0;
    char charKey[3];
                                                #if defined(BUFFER_KEY_COUNT)
                                                    int keyCount = 0;                                       //dr - internal keyBuffer POC
                                                #endif // BUFFER_KEY_COUNT
                                                #if defined(BUFFER_CLICK_DETECTION)
                                                    timeStampKey = (uint32_t)sys_current_ms();              //dr - internal keyBuffer POC
                                                #endif // BUFFER_CLICK_DETECTION
                                                    /*bool_t wp43KbdLayout, inFastRefresh = 0, inDownUpPress = 0, repeatDownUpPress = 0*/;  // removed autorepeat stuff   //dr - no keymap is used

                                                  //uint32_t now, previousRefresh, nextAutoRepeat = 0;      // removed autorepeat stuff

    c47MemInBlocks = 0;
    gmpMemInBytes = 0;
    mp_set_memory_functions(allocGmp, reallocGmp, freeGmp);

    #if CALCMODEL == USER_R47 // R47 meyboard mapping
      key_to_alpha_table = alpha_upper_transl; //Remap the alpha keyboard layout for DMCP dialogs
      //SET_ST(STAT_ALPHA_TAB_Fn);             // Alpha key table includes F keys - This doesn't apply to the R47
    #endif // CALCMODEL == USER_R47

    // initialize lcd_buffer mainly used in hal/lcd.c
    lcd_buffer = lcd_line_addr(0)-2;
    lcd_clear_buf();
                                                #if defined(NOKEYMAP)                                             //vv dr - no keymap is used
                                                    lcd_putsAt(t24, 4, "Press the bottom left key."); lcd_refresh();
                                                    while(key != 33 && key != 37) {
                                                      key = key_pop();
                                                      while(key == -1) {
                                                        sys_sleep();
                                                        key = key_pop();
                                                      }
                                                    }

                                                    wp43KbdLayout = (key == 37); // bottom left key
                                                    key = 0;

                                                  lcd_clear_buf();
                                                #endif // NOKEYMAP                                           //^^
    doFnReset(CONFIRMED, loadAutoSav);
    refreshScreen(71);

                                                #if defined(JMSHOWCODES)                                        //JM test
                                                  telltale_lastkey = 0;                                   //JM test
                                                  telltale_pos = 0;                                       //JM test
                                                #endif // JMSHOWCODES                                                  //JM test

                                                  #if 0
                                                    longInteger_t li;
                                                    uint32_t addr, min, max, *ptr;

                                                    min = 1;
                                                    max = 100000000;
                                                    while(min+1 < max) {
                                                      ptr = malloc((max + min) >> 1);
                                                      if(ptr) {
                                                        free(ptr);
                                                        min = (max + min) >> 1;
                                                      }
                                                      else {
                                                        max = (max + min) >> 1;
                                                      }
                                                    }

                                                    ptr = malloc(min);
                                                    xcopy(&addr, &ptr, 4);
                                                    free(ptr);
                                                    longIntegerInit(li);
                                                    uInt32ToLongInteger(addr, li);
                                                    convertLongIntegerToShortIntegerRegister(li, 16, 50);

                                                    uInt32ToLongInteger(min, li);
                                                    convertLongIntegerToShortIntegerRegister(li, 10, 51);

                                                    ptr = (uint32_t *)qspi_user_addr();
                                                    xcopy(&addr, &ptr, 4);
                                                    uInt32ToLongInteger(addr, li);
                                                    convertLongIntegerToShortIntegerRegister(li, 16, 52);

                                                    addr = (uint32_t)qspi_user_size(); // QSPI user size in bytes
                                                    uInt32ToLongInteger(addr, li);
                                                    convertLongIntegerToShortIntegerRegister(li, 10, 53);

                                                    ptr = (uint32_t *)&ram;
                                                    xcopy(&addr, &ptr, 4);
                                                    uInt32ToLongInteger(addr, li);
                                                    convertLongIntegerToShortIntegerRegister(li, 16, 54);

                                                    ptr = (uint32_t *)&indexOfItems;
                                                    xcopy(&addr, &ptr, 4);
                                                    uInt32ToLongInteger(addr, li);
                                                    convertLongIntegerToShortIntegerRegister(li, 16, 55);

                                                    ptr = (uint32_t *)ppgm_fp;
                                                    xcopy(&addr, &ptr, 4);
                                                    uInt32ToLongInteger(addr, li);
                                                    convertLongIntegerToShortIntegerRegister(li, 16, 56);

                                                    ptr = (uint32_t *)get_reset_state_file();
                                                    xcopy(&addr, &ptr, 4);
                                                    uInt32ToLongInteger(addr, li);
                                                    convertLongIntegerToShortIntegerRegister(li, 16, 57);

                                                    addr = 0x38; // RESET_STATE_FILE_SIZE;
                                                    uInt32ToLongInteger(addr, li);
                                                    convertLongIntegerToShortIntegerRegister(li, 10, 58);

                                                    ptr = (uint32_t *)aux_buf_ptr();
                                                    xcopy(&addr, &ptr, 4);
                                                    uInt32ToLongInteger(addr, li);
                                                    convertLongIntegerToShortIntegerRegister(li, 16, 59);

                                                    addr = AUX_BUF_SIZE;
                                                    uInt32ToLongInteger(addr, li);
                                                    convertLongIntegerToShortIntegerRegister(li, 10, 60);

                                                    ptr = (uint32_t *)write_buf_ptr();
                                                    xcopy(&addr, &ptr, 4);
                                                    uInt32ToLongInteger(addr, li);
                                                    convertLongIntegerToShortIntegerRegister(li, 16, 61);

                                                    addr = (uint32_t)write_buf_size();
                                                    uInt32ToLongInteger(addr, li);
                                                    convertLongIntegerToShortIntegerRegister(li, 10, 62);

                                                    addr = (uint32_t)get_hw_id();
                                                    uInt32ToLongInteger(addr, li);
                                                    convertLongIntegerToShortIntegerRegister(li, 10, 63);

                                                    ptr = (uint32_t *)resizeProgramMemory;
                                                    xcopy(&addr, &ptr, 4);
                                                    uInt32ToLongInteger(addr, li);
                                                    convertLongIntegerToShortIntegerRegister(li, 16, 64);

                                                    addr = (uint32_t)sizeof(char);
                                                    uInt32ToLongInteger(addr, li);
                                                    convertLongIntegerToShortIntegerRegister(li, 10, 65);

                                                    addr = (uint32_t)sizeof(short);
                                                    uInt32ToLongInteger(addr, li);
                                                    convertLongIntegerToShortIntegerRegister(li, 10, 66);

                                                    addr = (uint32_t)sizeof(int);
                                                    uInt32ToLongInteger(addr, li);
                                                    convertLongIntegerToShortIntegerRegister(li, 10, 67);

                                                    addr = (uint32_t)sizeof(long);
                                                    uInt32ToLongInteger(addr, li);
                                                    convertLongIntegerToShortIntegerRegister(li, 10, 68);

                                                    addr = (uint32_t)sizeof(long long);
                                                    uInt32ToLongInteger(addr, li);
                                                    convertLongIntegerToShortIntegerRegister(li, 10, 69);

                                                    addr = (uint32_t)sizeof(void *);
                                                    uInt32ToLongInteger(addr, li);
                                                    convertLongIntegerToShortIntegerRegister(li, 10, 70);

                                                    longIntegerFree(li);
                                                  #endif // 1

    backToDMCP = false;

    lcd_forced_refresh();                                   //JM
                                                    //previousRefresh = sys_current_ms();
    nextScreenRefresh = sys_current_ms() + SCREEN_REFRESH_PERIOD;
                                                    //now = sys_current_ms();
                                                      //runner_key_tout_init(0); // Enables fast auto repeat

    fnTimerReset();
    fnTimerConfig(TO_FG_LONG, refreshFn, TO_FG_LONG);
    fnTimerConfig(TO_CL_LONG, refreshFn, TO_CL_LONG);
    fnTimerConfig(TO_FG_TIMR, refreshFn, TO_FG_TIMR);
    fnTimerConfig(TO_FN_LONG, refreshFn, TO_FN_LONG);
    fnTimerConfig(TO_FN_EXEC, execFnTimeout, 0);
    fnTimerConfig(TO_3S_CTFF, shiftCutoff, TO_3S_CTFF);
    fnTimerConfig(TO_CL_DROP, fnTimerDummy1, TO_CL_DROP);
    fnTimerConfig(TO_AUTO_REPEAT, execAutoRepeat, 0);
    fnTimerConfig(TO_TIMER_APP, execTimerApp, 0);
    fnTimerConfig(TO_ASM_ACTIVE, refreshFn, TO_ASM_ACTIVE);
    fnTimerConfig(TO_KB_ACTV, fnTimerEndOfActivity, TO_KB_ACTV);
//--fnTimerConfig(TO_SHOW_NOP, execNOPTimeout, TO_SHOW_NOP);
    nextTimerRefresh = SCREEN_REFRESH_PERIOD;

    // Status flags:
    //   ST(STAT_PGM_END)   - Indicates that program should go to off state (set by auto off timer)
    //   ST(STAT_SUSPENDED) - Program signals it is ready for off and doesn't need to be woken-up again
    //   ST(STAT_OFF)       - Program in off state (OS goes to sleep and only [EXIT] key can wake it up again)
    //   ST(STAT_RUNNING)   - OS doesn't sleep in this mode
    //   SET_ST(STAT_CLK_WKUP_SECONDS);
    SET_ST(STAT_CLK_WKUP_ENABLE); // Enable wakeup each minute (for clock update)


    //** ** ** MAIN LOOP START ** ** **
    while(!backToDMCP) {
                          #if defined(DM42_POWERMARK_KEYPRESS)
                            powerMarkerMsF(1,1000);
                          #endif //DM42_POWERMARK_KEYPRESS
                                               //    char rrr[100];
                                               //    int ii = sys_auto_off_cnt();
                                               //    sprintf(rrr, "time left: %d",(uint16_t)ii);
                                               //    print_linestr(rrr,true);

      if(ST(STAT_PGM_END) && ST(STAT_SUSPENDED)) { // Already in off mode and suspended
        CLR_ST(STAT_RUNNING);
                            #if defined(DM42_POWERMARKS)
                              powerMarkerMsF(15,1000);
                            #endif //DM42_POWERMARKS
        sys_sleep();
      }
      else if(!ST(STAT_PGM_END) && key_empty() && emptyKeyBuffer()) {          // Just wait if no keys available.
        CLR_ST(STAT_RUNNING);

        if(nextTimerRefresh == 0) {                                            // no timeout available
                                                  #if defined(TMR_OBSERVE)
                                                    if(fnTestBitIsSet(2) == true) {
                                                      showString("key_empty()", &standardFont, 20, 40, vmNormal, false, false);
                                                      refreshLcd();
                                                      lcd_refresh_wait();
                                                    }
                                                  #endif // TMR_OBSERVE
          sys_sleep();
        }
        else {                                                                 // timeout available
          //uint32_t timeoutTime = max(1, nextTimerRefresh - sys_current_ms());
          uint32_t timeoutTime = sys_current_ms();
          if(nextTimerRefresh > timeoutTime) {
            timeoutTime = nextTimerRefresh - timeoutTime;
          }
          else {
                                                  #if defined(TMR_OBSERVE)
                                                    if(fnTestBitIsSet(3) == true) {
                                                      char snum[50];
                                                      itoa(timeoutTime - nextTimerRefresh, snum, 10);
                                                      showString(snum, &standardFont, 20, 120, vmNormal, false, false);
                                                    }
                                                  #endif // TMR_OBSERVE
            timeoutTime = 1;
          }
                                                  // char rrr[100];
                                                  // sprintf(rrr, "nextTimerRefresh: %lu",nextTimerRefresh);
                                                  // print_linestr(rrr,true);
                                                  // rrr[0]=0;
                                                  // print_linestr(rrr,false);
                                                  // sprintf(rrr, "timeoutTime: %lu",timeoutTime);
                                                  // print_linestr(rrr,false);

          if(fnTimerGetStatus(TO_KB_ACTV) == TMR_RUNNING) {
            timeoutTime = min(timeoutTime, 40);
          }
          if(fnTimerGetStatus(TO_FN_EXEC) == TMR_RUNNING) {
            timeoutTime = min(timeoutTime, 15);
          }
          if(fnTimerGetStatus(TO_CL_DROP) == TMR_RUNNING || fnTimerGetStatus(TO_FN_LONG) == TMR_RUNNING || fnTimerGetStatus(TO_CL_LONG) == TMR_RUNNING) {
            timeoutTime = min(timeoutTime, 10);
          }

          uint32_t sleepTime = SCREEN_REFRESH_PERIOD;
          sleepTime = min(sleepTime, timeoutTime);
          sys_timer_start(TIMER_IDX_REFRESH_SLEEP, max(sleepTime, 1));         // wake up for screen refresh

                                                  #if defined(TMR_OBSERVE)
                                                    if(fnTestBitIsSet(1) == true) {
                                                      char snum[50];
                                                      itoa(sleepTime, snum, 10);
                                                      strcat(snum, " ");
                                                      for(int8_t i = TMR_NUMBER -1; i>=0; i--) {
                                                        char digit[2] = "_";
                                                        if(fnTimerGetStatus(i) == TMR_RUNNING) { itoa(i, digit, 16); }
                                                        strcat(snum, digit);
                                                      }
                                                      showString(snum, &standardFont, 20, 40, vmNormal, false, false);
                                                    }
                                                  #endif // TMR_OBSERVE

                              #if defined(DM42_POWERMARKS)
                                powerMarkerMsF(10,1000);
                              #endif //DM42_POWERMARKS
                          #if defined(DM42_POWERMARK_KEYPRESS)
                            powerMarkerMsF(max(sleepTime, 1),8000);
                          #endif //DM42_POWERMARK_KEYPRESS
          sys_sleep();
                          #if defined(DM42_POWERMARK_KEYPRESS)
                            powerMarkerMsF(1,1000);
                          #endif //DM42_POWERMARK_KEYPRESS
          sys_timer_disable(TIMER_IDX_REFRESH_SLEEP);
        }


                                                  //      sys_timer_start(TIMER_IDX_SCREEN_REFRESH, max(1, nextScreenRefresh - now));  // wake up for screen refresh
                                                  //      if(inDownUpPress) {
                                                  //        sys_timer_start(TIMER_IDX_AUTO_REPEAT, max(1, nextAutoRepeat - now)); // wake up for key auto-repeat
                                                  //      }
                                                  //      sys_sleep();
                                                  //      sys_timer_disable(TIMER_IDX_SCREEN_REFRESH);
                                                  //      if(inDownUpPress) {
                                                  //        repeatDownUpPress = (sys_current_ms() > nextAutoRepeat);
                                                  //        sys_timer_disable(TIMER_IDX_AUTO_REPEAT);
                                                  //      }
      }

                                                  //now = sys_current_ms();

      // =======================
      // Externally forced LCD repaint
      if(ST(STAT_CLK_WKUP_FLAG)) {
                            #if defined(DM42_POWERMARKS)
                              powerMarkerMsF(5,10000);
                            #endif //DM42_POWERMARKS
        if(!ST(STAT_OFF) && (nextTimerRefresh == 0)) {

                                                  #if defined(TMR_OBSERVE)
                                                    if(fnTestBitIsSet(1) == true) {
                                                      showString("CLK_WKUP_FLAG", &standardFont, 20, 40, vmNormal, false, false);
                                                    }
                                                  #endif // TMR_OBSERVE
          updateVbatIntegrated(true);
          refreshLcd();
          lcd_refresh_wait();
        }
        CLR_ST(STAT_CLK_WKUP_FLAG);
        continue;
      }
      if(ST(STAT_POWER_CHANGE)) {
                            #if defined(DM42_POWERMARKS)
                              powerMarkerMsF(7,10000);
                            #endif //DM42_POWERMARKS
        showHideUsbLowBattery();
        refreshLcd();
        lcd_refresh_wait();
        if(!ST(STAT_OFF) && (fnTimerGetStatus(TO_KB_ACTV) != TMR_RUNNING)) {
          fnTimerStart(TO_KB_ACTV, TO_KB_ACTV, 40);
        }
        CLR_ST(STAT_POWER_CHANGE);
        continue;
      }

      // Wakeup in off state or going to sleep
      if(ST(STAT_PGM_END) || ST(STAT_SUSPENDED)) {
        if(!ST(STAT_SUSPENDED)) {
          // Going to off mode
          lcd_set_buf_cleared(0); // Mark no buffer change region
          draw_power_off_image(1);

          LCD_power_off(0);
          SET_ST(STAT_SUSPENDED);
          SET_ST(STAT_OFF);
        }
        // Already in OFF -> just continue to sleep above
        continue;
      }

      // Well, we are woken-up
      SET_ST(STAT_RUNNING);

      // Clear suspended state, because now we are definitely reached the active state
      CLR_ST(STAT_SUSPENDED);

      // Get up from OFF state
      if(ST(STAT_OFF)) {
        LCD_power_on();
        rtc_wakeup_delay(); // Ensure that RTC readings after power off will be OK

        CLR_ST(STAT_OFF);

        if(!lcd_get_buf_cleared()) {
          lcd_forced_refresh(); // Just redraw from LCD buffer
        }

        if(getSystemFlag(FLAG_AUTXEQ)) { // Run the program if AUTXEQ is set
          clearSystemFlag(FLAG_AUTXEQ);
          if(programRunStop != PGM_RUNNING) {
            screenUpdatingMode = SCRUPD_AUTO;
            runFunction(ITM_RS);
          }
          refreshScreen(72);
        }
      }

      dmcpResetAutoOff();

                                                  #if defined(NOKEYMAP)
                                                    // Fetch the key
                                                    //  < 0 -> No key event
                                                    //  > 0 -> Key pressed
                                                    // == 0 -> Key released
                                                    key = key_pop();

                                                    //key = runner_get_key_delay(&keyAutoRepeat,
                                                    //                           50,                            // timeout - this should be the fastest period between loops
                                                    //                           KEY_AUTOREPEAT_FIRST_PERIOD,  // time before the first autorepeat
                                                    //                           KEY_AUTOREPEAT_PERIOD,        // time between subsequent autorepeats
                                                    //                           KEY_AUTOREPEAT_FIRST_PERIOD); // should be the same as time before first autorepeat
                                                    //key = runner_get_key(&keyAutoRepeat);

                                                    key = convertKeyCode(key);
                                                    //The 3 lines below to see in the top left screen corner the pressed keycode
                                                    //char sysLastKeyCh[5];
                                                    //sprintf(sysLastKeyCh, " %02d", key);
                                                    //showString(sysLastKeyCh, &standardFont, 0, 0, vmReverse, true, true);
                                                    //The line below to emit a beep
                                                    //while(get_beep_volume() < 11) beep_volume_up(); start_buzzer_freq(220000); sys_delay(200); stop_buzzer();
                                                  #endif // NOKEYMAP

                                                  #if defined(AUTOREPEAT_WP43S)
                                                    // Increase the refresh rate if we are in an UP/DOWN key press so we pick up auto key repeats
                                                    if(key == 18 || key == 23) {
                                                      //inDownUpPress = 1;
                                                      //nextAutoRepeat = now + KEY_AUTOREPEAT_FIRST_PERIOD;
                                                      if(fnTimerGetStatus(TO_AUTO_REPEAT) != TMR_RUNNING && (!shiftF || calcMode == CM_PEM) && !shiftG && (currentSoftmenuScrolls() || (calcMode != CM_NORMAL && calcMode != CM_NIM && calcMode != CM_AIM))) {
                                                        fnTimerStart(TO_AUTO_REPEAT, key, KEY_AUTOREPEAT_FIRST_PERIOD);
                                                      }
                                                    }
                                                    else if(key == 0) {
                                                      //inDownUpPress = 0;
                                                      //repeatDownUpPress = 0;
                                                      //nextAutoRepeat = 0;
                                                      fnTimerStop(TO_AUTO_REPEAT);
                                                    }
                                                    //else if(repeatDownUpPress) {
                                                    //  keyAutoRepeat = 1;
                                                    //  key = 0;
                                                    //  nextAutoRepeat = now + KEY_AUTOREPEAT_PERIOD;
                                                    //  repeatDownUpPress = 0;
                                                    //}

                                                    //if(keyAutoRepeat) {
                                                    //  if(key == 27 || key == 32) { // UP or DOWN keys
                                                    //    //beep(2200, 50);
                                                    //    key = 0; // to trigger btnReleased
                                                    //  }
                                                    //  else {
                                                    //    key = -1;
                                                    //  }
                                                    //}
                                                  #endif // AUTOREPEAT_WP43S


      uint8_t outKey;
      keyBuffer_pop();

                                                  #if defined(BUFFER_CLICK_DETECTION)
                                                    uint32_t timeSpan_1;
                                                    uint32_t timeSpan_B;
                                                    #if defined(BUFFER_KEY_COUNT)
                                                uint8_t outKeyCount;
                                                if(outKeyBuffer(&outKey, &outKeyCount, &timeStampKey, &timeSpan_1, &timeSpan_B) == BUFFER_SUCCESS) {
                                                  key = outKey;
                                                  keyCount = outKeyCount;
                                                  //if(outKeyCount > 0) {
                                                  //  do someting
                                                  //}
                                                }
                                                #else // !BUFFER_KEY_COUNT
                                                  if(outKeyBuffer(&outKey, &timeStampKey, &timeSpan_1, &timeSpan_B) == BUFFER_SUCCESS) {
                                                    key = outKey;
                                                  }
                                                #endif // BUFFER_KEY_COUNT
                                                #else // !BUFFER_CLICK_DETECTION
                                                #if defined(BUFFER_KEY_COUNT)
                                                  uint8_t outKeyCount;
                                                  if(outKeyBuffer(&outKey, &outKeyCount) == BUFFER_SUCCESS) {
                                                    key = outKey;
                                                    keyCount = outKeyCount;
                                                  }
                                                #else // !BUFFER_KEY_COUNT
      if(outKeyBuffer(&outKey) == BUFFER_SUCCESS) {
        key = outKey;
      }
                                                #endif // BUFFER_KEY_COUNT
                                                #endif // BUFFER_CLICK_DETECTION
      else {
        key = -1;
      }                                                     //^^


                                                  #if defined(AUTOREPEAT_C47)
                                                  if(key == 18 || key == 23) {
                                                    //if(!shiftF || calcMode == CM_PEM) && !shiftG && (currentSoftmenuScrolls() || (calcMode != CM_NORMAL && calcMode != CM_NIM && calcMode != CM_AIM)) {
                                                    if(fnTimerGetStatus(TO_AUTO_REPEAT) != TMR_RUNNING) {
                                                      fnTimerStart(TO_AUTO_REPEAT, key, KEY_AUTOREPEAT_FIRST_PERIOD);
                                                    }
                                                    //}
                                                    }
                                                    else if(key == 0) {
                                                      fnTimerStop(TO_AUTO_REPEAT);
                                                    }
                                                  #endif // AUTOREPEAT_C47

      if(key == 44) { //DISP for special SCREEN DUMP key code. To be 16 but shift decoding already done to 44 in DMCP
        standardScreenDump();
        wait_for_key_release(0);
        charKey[0]=0;
        charKey[1]=0;
        resetShiftState();
        resetKeytimers();
        key = -1;

        keyBuffer_pop();
        uint8_t outKey;
        while (!emptyKeyBuffer()) {
          outKeyBuffer(&outKey);
        }
        lastItem = SCREENDUMP;
      }

                                                  #if defined(JMSHOWCODES)
                                                    fnDisplayStack(1);
                                                    //Show key codes
                                                    if(sys_last_key()!=telltale_lastkey) {
                                                      telltale_lastkey = sys_last_key();
                                                      telltale_pos++;
                                                      telltale_pos = telltale_pos & 0x03;
                                                      char aaa[100];
                                                      #if defined(BUFFER_CLICK_DETECTION)
                                                        sprintf   (aaa,"k=%d d=%ld  d=%ld",key, timeSpan_1, timeSpan_B);
                                                      #endif
                                                      showString(aaa, &standardFont, 300, Y_POSITION_OF_REGISTER_X_LINE - REGISTER_LINE_HEIGHT*(REGISTER_T - REGISTER_X), vmNormal, true, true);
                                                      sprintf   (aaa,"Rel=%d, nop=%d, St=%d, Key=%d, FN_kp=%d   ",FN_timed_out_to_RELEASE_EXEC, FN_timed_out_to_NOP_or_Executed, FN_state, sys_last_key(), FN_key_pressed);
                                                      showString(aaa, &standardFont, 1, Y_POSITION_OF_REGISTER_X_LINE - REGISTER_LINE_HEIGHT*(REGISTER_Z - REGISTER_X), vmNormal, true, true);
                                                      #if defined(BUFFER_CLICK_DETECTION)
                                                        sprintf   (aaa,"%4d(%4ld)(%4ld)<<",sys_last_key(),timeSpan_1,timeSpan_B);
                                                      #endif
                                                      showString(aaa, &standardFont, telltale_pos*90+ 1, Y_POSITION_OF_REGISTER_X_LINE - REGISTER_LINE_HEIGHT*(REGISTER_Y - REGISTER_X), vmNormal, true, true);
                                                    }
                                                  #endif // JMSHOWCODES

      if(38 <= key && key <=43) { // Function key
                            #if defined(DM42_POWERMARK_KEYPRESS)
                              powerMarkerMsF(1,4000);
                            #endif //DM42_POWERMARK_BEGIN_WHILE
        sprintf(charKey, "%c", key+11);
        btnFnPressed(charKey);
                            #if defined(DM42_KEYCLICK)
                              keyClick(3);
                            #endif //DM42_KEYCLICK
      //lcd_refresh_dma();
      }
      else if(1 <= key && key <= 37) { // Not a function key
                            #if defined(DM42_POWERMARK_KEYPRESS)
                              powerMarkerMsF(1,4000);
                            #endif //DM42_POWERMARK_BEGIN_WHILE
        sprintf(charKey, "%02u", key - 1);
        btnPressed(charKey);
                            #if defined(DM42_KEYCLICK)
                              keyClick(1);
                            #endif //DM42_KEYCLICK
      //lcd_refresh_dma();
      }

                                                  #if defined(FN_RELEASE_CODE_WP43S)
                                                        else if(key == 0) { // Autorepeat of UP/DOWN or key released
                                                          if(charKey[1] == 0) { // Last key pressed was one of the 6 function keys
                                                            btnFnReleased(charKey);
                                                          }
                                                          else { // Last key pressed was not one of the 6 function keys
                                                            //beep(440, 50);
                                                            btnReleased(charKey);
                                                            if(calcMode == CM_PEM && shiftF && ((charKey[0] == '1' && charKey[1] == '7') || (charKey[0] == '2' && charKey[1] == '2'))) {    //JM C47
                                                              shiftF = false;
                                                              refreshScreen(73);
                                                            }
                                                          }
                                                  //      keyAutoRepeat = 0;
                                                          lcd_refresh();
                                                        }
                                                  #endif // FN_RELEASE_CODE_WP43S

      else if(key == 0 && charKey[1] == 0) {            //JM, key=0 is release, therefore there must have been a press before that. If the press was a FN key, FN_key_pressed > 0 when it comes back here for release.
                            #if defined(DM42_POWERMARK_KEYPRESS)
                              powerMarkerMsF(1,4000);
                            #endif //DM42_POWERMARK_BEGIN_WHILE
        btnFnReleased(charKey);                                //    in short, it can only execute FN release after there was a FN press.
                            #if defined(DM42_KEYCLICK)
                              keyClick(4);
                            #endif //DM42_KEYCLICK
      //lcd_refresh_dma();
      }
      else if(key == 0) {
                            #if defined(DM42_POWERMARK_KEYPRESS)
                              powerMarkerMsF(1,4000);
                            #endif //DM42_POWERMARK_BEGIN_WHILE
        btnReleased(charKey);
                            #if defined(DM42_KEYCLICK)
                              keyClick(2);
                            #endif //DM42_KEYCLICK
        if(calcMode == CM_PEM && shiftF && ( (calcModel == USER_C47 && ((charKey[0] == '1' && charKey[1] == '7') || (charKey[0] == '2' && charKey[1] == '2')))
                                            || (calcModel == USER_R47 && ((charKey[0] == '2' && charKey[1] == '2') || (charKey[0] == '2' && charKey[1] == '7')))
                                                 )) {
          shiftF = false;
          refreshScreen(74);
        }
        //lcd_refresh_dma();
      }

      if(key >= 0) {                                        //dr
        lcd_refresh_dma();
        if(key > 0) {
          fnTimerStart(TO_KB_ACTV, TO_KB_ACTV, TO_KB_ACTV_MEDIUM);     // Key pressed
        }
        else if(cursorEnabled == true) {
                            #if defined(DM42_KEYCLICK)
                              keyClick(6);
                            #endif //DM42_KEYCLICK
          fnTimerStart(TO_KB_ACTV, TO_KB_ACTV, TO_KB_ACTV_CURSOR);
        }
        else
        {
                            #if defined(DM42_KEYCLICK)
                              keyClick(7);
                            #endif //DM42_KEYCLICK
          fnTimerStart(TO_KB_ACTV, TO_KB_ACTV, skippedStackLines ? TO_KB_ACTV_MEDIUM/5 : TO_KB_ACTV_SHORT); // Key released
        }
      }

                                                  //    // Compute refresh period
                                                  //    if(showFunctionNameCounter > 0) {
                                                  //      inFastRefresh = 1;
                                                  //      nextScreenRefresh = previousRefresh + FAST_SCREEN_REFRESH_PERIOD;
                                                  //    }
                                                  //    else {
                                                  //      inFastRefresh = 0;
                                                  //    }

      uint32_t now = sys_current_ms();

      if(nextTimerRefresh != 0 && nextTimerRefresh <= now) {
        refreshTimer();                                     // Executes pending timer jobs
                          #if defined(DM42_POWERMARK_KEYPRESS)
                            powerMarkerMsF(5,4000);
                          #endif //DM42_POWERMARK_KEYPRESS
      }
      now = sys_current_ms();
      if(nextScreenRefresh <= now) {
        nextScreenRefresh += SCREEN_REFRESH_PERIOD;
        if(nextScreenRefresh < now) {
          nextScreenRefresh = now + SCREEN_REFRESH_PERIOD;  // we were out longer than expected; just skip ahead.
        }
        if((calcMode != CM_TIMER) || (fnTimerGetStatus(TO_TIMER_APP) != TMR_RUNNING)) {
                          #if defined(DM42_POWERMARK_KEYPRESS)
                            powerMarkerMsF(10,1000);
                          #endif //DM42_POWERMARK_KEYPRESS
          refreshLcd();
          if(key >= 0) {
            lcd_refresh();
          }
          else {
            lcd_refresh_wait();
          }
        }
      }
    }
  fnSaveAuto(NOPARAM);
  }
#endif // DMCP_BUILD
