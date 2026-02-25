// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#if !defined(TYPEDEFINITIONS_H)
#define TYPEDEFINITIONS_H

/**
 * Boolean type.
 */
typedef bool bool_t;


/**
 * \union multiplyDivide_t
   * Used for unit conversions.
   */
typedef enum {
  multiply = 0,
  divide = 0x8000,
  invert = 0x4000
} multiplyDivide_t;


/**
 * \struct calcKey_t
   * Structure keeping the informations for one key.
   */
typedef struct {
  int16_t keyId;       ///< ID of the key
  int16_t primary;     ///< ID of the primary function of the key
  int16_t fShifted;    ///< ID of the f shifted function of the key
  int16_t gShifted;    ///< ID of the g shifted function of the key
  int16_t keyLblAim;   ///< ID of the main label of the key
  int16_t primaryAim;  ///< ID of the primary AIM function: latin letters
  int16_t fShiftedAim; ///< ID of the f shifted AIM function:
  int16_t gShiftedAim; ///< ID of the g shifted AIM function: greek letters
  int16_t primaryTam;  ///< ID of the primary TAM function of the key
} calcKey_t;


/**
 * \struct normKey_t
   * Structure keeping the informations for the Norm key.
   */
typedef struct {
  int16_t func;            ///< ID of the function of the Norm key
  char    funcParam[16];   ///< function parameter if required
  bool_t  used;            ///< True when Norm key is used
} normKey_t;


/**
 * \struct glyph_t
 * Structure keeping the informations for one glyph.
 */
typedef struct {
  uint16_t charCode;         ///< Unicode code point
  uint8_t  colsBeforeGlyph;  ///< Number of empty columns before the glyph
  uint8_t  colsGlyph;        ///< Number of columns of the glyph
  uint8_t  colsAfterGlyph;   ///< Number of empty columns afer the glyph
  uint8_t  rowsAboveGlyph;   ///< Number of empty rows above the glyph
  uint8_t  rowsGlyph;        ///< Number of rows of the glyph
  uint8_t  rowsBelowGlyph;   ///< Number of empty rows below the glypg
  int16_t  rank1;            ///< Rank of the replacement glyph
  int16_t  rank2;            ///< Rank of the glyph
  char     *data;            ///< Hexadecimal data representing the glyph.
                             ///< There are rowsGlyph x (colsGlyph rounded up to 8 bit boundary) bytes
} glyph_t;


/**
 * \struct font_t
 * Font description.
 */
typedef struct {
  int8_t  id;              ///< 0=numeric 1=standard
  uint16_t numberOfGlyphs; ///< Number of glyphs in the font
  glyph_t glyphs[];        ///< Pointer to the glyph description structure
} font_t;


/**
 * \struct freeMemoryRegion_t
 * Keeps track of free memory regions.
 */
typedef struct {
  uint16_t blockAddress; ///< Address of the free memory region
  uint16_t sizeInBlocks; ///< Size in blocks of the free memory region
} freeMemoryRegion_t;


/**
 * \enum dataType_t
 * Different data types.
 */
typedef enum {
  dtLongInteger     =  0,  ///< Z arbitrary precision integer
  dtReal34          =  1,  ///< R double precision real (128 bits)
  dtComplex34       =  2,  ///< C double precision complex (2x 128 bits)
  dtTime            =  3,  ///< Time
  dtDate            =  4,  ///< Date in various formats
  dtString          =  5,  ///< Alphanumeric string
  dtReal34Matrix    =  6,  ///< Double precision vector or matrix
  dtComplex34Matrix =  7,  ///< Double precision complex vector or matrix
  dtShortInteger    =  8,  ///< Short integer (64 bit)
  dtConfig          =  9,  ///< Configuration
  //dtLabel           = 10,  ///< Label
  //dtSystemInteger   = 11,  ///< System integer (64 bits)
  //dtFlags           = 12,  ///< Flags
  //dtPgmStep         = 13,  ///< Program step
  //dtDirectory       = 14,  ///< Program
  dtNumbers         = 15,  ///< Numbers (LongInteger, Real34, Complex34)
} dataType_t; // 4 bits (NOT 5 BITS)


/**
 * \enum angularMode_t
 * Angular units.
 */
typedef enum {
  amRadian    =  0, // radian
  amGrad      =  1, // grad
  amDegree    =  2, // degree
  amDMS       =  3, // degrees in dd mm ss.sss...
  amMultPi    =  4, // multiples of pi
  amNone      =  5, // RECT in complex, real, vector
  amSecond    =  6, // not an angular but a time unit: for the routine unified with the real type
  TM_HMS      =  7, // not an angular but a time unit
  amAngleMask = 15,
  amPolar     = 16, // JM bit 4 of the 5 bits is used for Polar in Complex Case

  //removed the idea of adding bits: See registers.h
  //  amPolarCYL  =  64, // 3D Vector: Polar cylindrical.
  //  amPolarSPH  = 128  // 3D Vector: Polar cylindrical.
  //----
  // Replaced with the logic table below, in essence, if a real matrix, and if the matrix is 1x2, 2x1, 1x3 or 3x1 then it is a vector, for which:
  //   - A 2D vector is in Rect, if the angleMode == amNone. The amPolar flag, bit 4, also is clear
  //   - A 3D vector is in Rect, if the angleMode == amNone. The amPolar flag set is Spherical Polar, and reset is Cylindrical.
  //   There are macros for this in defines.h (for matrix type) and registers.h (for registers)

  //using logic to store in a real matrix register, the states: CYL, SPH or RECT. RECT being amNone set and no Polar mode set. This can be reworked into storing the bits. Search for amPolarCYL & amPolarSPH to change.
  //5-bits stored in register header
  //current:   0b ---.-xxx   x : 0-7 angle/time types
  //              ---z-...   z : Polar
  //              ---.-...   - : spare bits

  //Real             0 -000       Real amRadian
  //Real             0 -001       Real amGrad
  //Real             0 -010       Real amDegree
  //Real             0 -011       Real amDMS
  //Real             0 -100       Real amMultPi
  //Real             0 -101       Real amNone
  //Real             0 -110       Real amSecond
  //Real             0 -111       Real TM_HMS

  // Cpx             0 -000       Rect ---
  // Cpx             0 -001       Rect ---
  // Cpx             0 -010       Rect ---
  // Cpx             0 -011       Rect ---
  // Cpx             0 -100       Rect ---
  // Cpx             0 -101       Rect amNone
  // Cpx             0 -110       Rect ---
  // Cpx             0 -111       Rect ---

  // Cpx             1 -000      Polar amRadian
  // Cpx             1 -001      Polar amGrad
  // Cpx             1 -010      Polar amDegree
  // Cpx             1 -011      Polar amDMS
  // Cpx             1 -100      Polar amMultPi
  // Cpx             1 -101            ---
  // Cpx             1 -110            ---
  // Cpx             1 -111            ---

  // RealMx 2D 3D    0 -000       Rect ---
  // RealMx 2D 3D    0 -001       Rect ---
  // RealMx 2D 3D    0 -010       Rect ---
  // RealMx 2D 3D    0 -011       Rect ---
  // RealMx 2D 3D    0 -100       Rect ---
  // RealMx 2D 3D    0 -101       Rect amNone
  // RealMx 2D 3D    0 -110       Rect ---
  // RealMx 2D 3D    0 -111       Rect ---

  // RealMx    3D    0 -000        CYL amRadian
  // RealMx    3D    0 -001        CYL amGrad
  // RealMx    3D    0 -010        CYL amDegree
  // RealMx    3D    0 -011        CYL amDMS
  // RealMx    3D    0 -100        CYL amMultPi
  // RealMx    3D    0 -101            ---
  // RealMx    3D    0 -110            ---
  // RealMx    3D    0 -111            ---

  // RealMx 2D 3D    1 -000    POL SPH amRadian
  // RealMx 2D 3D    1 -001    POL SPH amGrad
  // RealMx 2D 3D    1 -010    POL SPH amDegree
  // RealMx 2D 3D    1 -011    POL SPH amDMS
  // RealMx 2D 3D    1 -100    POL SPH amMultPi
  // RealMx 2D 3D    1 -101            ---
  // RealMx 2D 3D    1 -110            ---
  // RealMx 2D 3D    1 -111            ---


} angularMode_t;


/**
 * \enum commonBugScreenMessageCode_t
 * Common bug screen messages.
 */
typedef enum {
  bugMsgValueFor                 = 0,
  bugMsgCalcModeWhileProcKey     = 1,
  bugMsgNoNamedVariables         = 2,
  bugMsgValueReturnedByFindGlyph = 3,
  bugMsgUnexpectedSValue         = 4,
  bugMsgDataTypeUnknown          = 5,
  bugMsgRegistMustBeLessThan     = 6,
  bugMsgNotDefinedMustBe         = 7,
  bugMsgRbrMode                  = 8,
} commonBugScreenMessageCode_t;


/**
 * \struct dtConfigDescriptor_t
 * Configuration for STOCFG and RCLCFG.
 */
typedef struct {
  uint8_t        shortIntegerMode;                                         //  Sign mode
  uint8_t        shortIntegerWordSize;                                     //  Word size
  uint8_t        displayFormat;                                            //  FIX/SCI…
  uint8_t        displayFormatDigits;                                      //  Display digits for FIX, SCI…
  uint16_t       gapItemLeft;                                              //  GAP details
  uint16_t       gapItemRight;                                             //  GAP details
  uint16_t       gapItemRadix;                                             //  GAP details
  uint8_t        grpGroupingLeft;                                          //  GAP details
  uint8_t        grpGroupingGr1LeftOverflow;                               //  GAP details
  uint8_t        grpGroupingGr1Left;                                       //  GAP details
  uint8_t        grpGroupingRight;                                         //  GAP details
  angularMode_t  currentAngularMode;                                       //  ADM
  uint16_t       lrSelection;                                              //  STAT LR settings
  uint16_t       lrChosen;                                                 //  Last chosen LR
  uint32_t       denMax;                                                   //  DMX
  uint8_t        displayStack;                                             //  dSTACK
  uint32_t       firstGregorianDay;                                        //  First Gregorian Day
  uint8_t        roundingMode;                                             //  RM setting
  uint64_t       systemFlags0;                                             //  All SFL flags
  uint64_t       systemFlags1;                                             //  All SFL flags
  calcKey_t      kbd_usr[37];                                              //  All user keys
  uint8_t        compatibility_byte1;                                      //
  bool_t         compatibility_byte19;                                     //
  bool_t         compatibility_byte27;
  bool_t         compatibility_byte29;
  bool_t         compatibility_byte21;              //Spare Byte           //
  bool_t         compatibility_byte30;
  bool_t         compatibility_byte00;               //Spare Byte          //
  int16_t        compatibility_int1;               //Spare Word            //
  uint8_t        Input_Default;                                            //  iLi/R, iR, iC, iLI
  bool_t         dispBase;                                                 //  dBASE
  bool_t         compatibility_byte31;
  bool_t         compatibility_byte26;
  float          compatibility_float1;              //Spare float          //
  float          compatibility_float2;              //Spare float          //
  normKey_t      Norm_Key_00;                                              //  BlankKey Config
  bool_t         compatibility_byte2;               //Spare Byte           //
  bool_t         compatibility_byte3;               //Spare Byte           //
  bool_t         compatibility_byte4;               //Spare Byte           //
  bool_t         compatibility_byte5;               //Spare Byte           //
  bool_t         compatibility_byte6;               //Spare Byte           //
  bool_t         compatibility_byte7;               //Spare Byte           //
  bool_t         compatibility_byte8;               //Spare Byte           //
  bool_t         compatibility_byte9;               //Spare Byte           //
  bool_t         compatibility_byte10;              //Spare Byte           //
  bool_t         compatibility_byte11;              //Spare Byte           //
  bool_t         compatibility_byte12;              //Spare Byte           //
  bool_t         compatibility_byte13;              //Spare Byte           //
  bool_t         compatibility_byte14;              //Spare Byte           //
  bool_t         compatibility_byte15;              //Spare Byte           //
  int8_t         fractionDigits;                                           //  FDIGS
  int8_t         compatibility_byte23;                                     //
  bool_t         compatibility_byte16;              //Spare Byte           //
  bool_t         compatibility_byte20;              //Spare Byte           //
  bool_t         compatibility_byte17;              //Spare Byte           //
  uint8_t        IrFractionsCurrentStatus;                                 //  Internal control flag for IRFRAC
  bool_t         compatibility_byte18;              //Spare Byte           //
  uint8_t        displayStackSHOIDISP;                                     //  dSI
  bool_t         compatibility_byte25;                                     //  BCD
  bool_t         compatibility_byte24;              //Spare Byte           //
  uint8_t        bcdDisplaySign;                                           //  BCDUNS
  uint8_t        DRG_Cycling;                                              //  Internal control flag for DRG
  uint8_t        DM_Cycling;                                               //  internal control flag for FSE
  bool_t         compatibility_byte22;                                     //
  uint8_t        LongPressM;                                               //  M.124, M.1234, M.14
  uint8_t        LongPressF;                                               //  F.1234, F.14, F.124
  uint32_t       lastDenominator;                                          //  internal control variable for last denominator used
  uint8_t        significantDigits;                                        //  SDIGS
  pcg32_random_t pcg32_global;                                             //  Random seed
  int16_t        exponentLimit;                                            //  RNG
  int16_t        exponentHideLimit;                                        //  HIDE
  uint32_t       lastIntegerBase;                                          //  internal control variable for the last base used. Control BASE mode.
  bool_t         compatibility_byte28;
  uint8_t        timeDisplayFormatDigits;                                  //  TDISP
  uint8_t        reservedForPossibleFutureUse[6]; //additional buffer to pad up to 840 bytes for the descriptor record
  //2025-04-21 Verified all variables above, and in recall.c and store.c
  //Note: Do not change entries going forward to maintain compatibility
  //Should any be added, ensure that the defaults are appropriately added when reading the state file
  //!!! IF THE CONFIG DATA LENGTH IS INCREASED, START_REGISTER_VALUE IN SaveRestoreCalcState NEEDS TO BE RE-EVALUATED !!!
} dtConfigDescriptor_t;


/**
 * \union registerHeader_t
 * 32 bits describing the register.
 */
typedef union {
  uint32_t descriptor;
  struct {
    unsigned pointerToRegisterData : 16; ///< uint32_t      Memory block number
    unsigned dataType              :  4; ///< uint32_t      dtLongInteger, dtReal34, ...
    unsigned tag                   :  5; ///< angularMode_t Short integer base, real34 angular mode, or long integer sign; complex34 angle mode + Polar
    unsigned readOnly              :  1; ///< uint32_t      The register or variable is readOnly if this field is 1 (used for system named variables)
    unsigned notUsed               :  6; ///< uint32_t      6 bits free
  };
} registerHeader_t;


/**
 * \typedef matrixHeader_t
 * Header for matrix datatype.
 */
typedef struct {
  unsigned matrixRows    : 12;   ///< uint32_t      Number of rows in the matrix
  unsigned matrixColumns : 12;   ///< uint32_t      Number of columns in the matrix
  unsigned mtag          :  6;   ///< angularMode_t tag;
  unsigned notUsed       :  2;   ///< uint32_t      2 bits free
} matrixHeader_t;


/**
 * \struct strLgIntHeader_t
 * Header for string and long integer data
 */
typedef struct {
  uint16_t dataMaxLengthInBlocks; ///< Max size in blocks of the long integer or the string including trailing 0
  uint16_t unused;                ///< Unused
} strLgIntHeader_t;


typedef uint32_t localFlags_t;


/**
 * \struct subroutineLevels_t
 * allSubroutineLevels data structure
 */
typedef struct {
  uint16_t numberOfSubroutineLevels;    ///< Number of subroutine levels
  uint16_t ptrToSubroutineLevel0Header; ///< Pointer to subroutine level 0 data
} subroutineLevels_t;


/**
 * \union subroutineLevelHeader_t
 * Header for a subroutine level
 */
typedef struct {
  int16_t  returnProgramNumber;       ///< return program number >0 if in RAM and <0 if in FLASH
  uint16_t returnLocalStep;           ///< Return local step number in program number
  uint8_t  numberOfLocalFlags;        ///< Number of allocated local flags
  uint8_t  numberOfLocalRegisters;    ///< Number of allocated local registers
  uint16_t subroutineLevel;           ///< Subroutine level
  uint16_t ptrToNextLevel;            ///< Pointer to next level of subroutine data
  uint16_t ptrToPreviousLevel;        ///< Pointer to previous level of subroutine data
} subroutineLevelHeader_t;


/**
 * \struct namedVariableHeader_t
 * Header for named variables.
 */
typedef struct {
  registerHeader_t header;  ///< Header
  uint8_t variableName[16]; ///< Variable name
} namedVariableHeader_t;

typedef struct {
  char     Desc[28];
} reservedVariableDescStr_t;


/**
 * \struct reservedVariableHeader_t
 * Header for system named variables.
 */
typedef struct {
  registerHeader_t header;         ///< Header
  uint8_t reservedVariableName[8]; ///< Variable name
} reservedVariableHeader_t;


/**
 * \struct formulaHeader_t
 * Header for EQN formulae.
 */
typedef struct {
  uint16_t pointerToFormulaData; ///< Memory block number
  uint8_t  sizeInBlocks;         ///< Size of allocated memory block
  uint8_t  unused;               ///< Padding
} formulaHeader_t;


/**
 * \enum videoMode_t
 * Video mode: normal video or reverse video.
 */
typedef enum {
  vmNormal,  ///< Normal mode: black on white background
  vmReverse  ///< Reverse mode: white on black background
} videoMode_t; // 1 bit


/**
 * \struct softmenu_t
   * Structure keeping the informations for one softmenu.
   */
typedef struct {
  int16_t menuItem;           ///< ID of the menu. The item is always negative and -item must be in the indexOfItems area
  int16_t numItems;           ///< Number of items in the softmenu (must be a multiple of 6 for now)
  const int16_t *softkeyItem; ///< Pointer to the first item of the menu
} softmenu_t;


/**
 * \struct dynamicSoftmenu_t
   * Structure keeping the informations for one variable softmenu.
   */
typedef struct {
  int16_t menuItem;           ///< ID of the menu. The item is always negative and -item must be in the indexOfItems area
  int16_t numItems;           ///< Number of items in the dynamic softmenu (must be a multiple of 6 for now)
  uint8_t *menuContent;       ///< Pointer to the menu content
} dynamicSoftmenu_t;


/**
 * \struct softmenuStack_t
   * Stack of softmenus.
   */
typedef struct {
  int16_t softmenuId; ///< Softmenu ID = rank in dynamicSoftmenu or softmenu
  int16_t firstItem;  ///< Current first item on the screen (unshifted F1 = bottom left)
  int16_t userMenuId; ///< Id of the user menu when softmenuId is a
  uint8_t calcMode;   ///< Parent mode
} softmenuStack_t;


/**
 * \typedef calcRegister_t
 * \brief A type for calculator registers
*/
typedef int16_t calcRegister_t;

/**
 * \struct real34Matrix_t
 * A type for real34Matrix.
 */
typedef struct {
   matrixHeader_t  header;
   real34_t       *matrixElements;
} real34Matrix_t;

/**
 * \struct complex34Matrix_t
 * A type for complex34Matrix.
 */
typedef struct {
   matrixHeader_t  header;
   complex34_t    *matrixElements;
} complex34Matrix_t;

/**
 * \union any34Matrix_t
 * Stores either real34Matrix_t or complex34Matrix_t.
 */
typedef union {
   matrixHeader_t    header;
   real34Matrix_t    realMatrix;
   complex34Matrix_t complexMatrix;
} any34Matrix_t;

/**
 * \struct confirmationTI_t
 * Structure keeping the TI message for items requiring confirmation.
 */
typedef struct {
  int16_t  item;                   ///< Item ID
  char     string[30];    ///< Confirmation message
} confirmationTI_t;

/**
 * \struct item_t
 * Structure keeping the information for one item.
 */
typedef struct {
  void     (*func)(uint16_t); ///< Function called to execute the item
  uint16_t param;             ///< 1st parameter to the above function
  char     itemCatalogName [16]; ///< Name of the item in the catalogs and in programs
  char     itemSoftmenuName[16]; ///< Representation of the item in the menus and on the keyboard
  uint16_t tamMinMax;         ///< Minimal value (2 bits) and maximal value (14 bits) for TAM argument
  //uint16_t tamMin;            ///< Minimal value for TAM argument
  //uint16_t tamMax;            ///< Maximal value for TAM argument
  uint16_t status;            ///< Catalog, stack lift status and undo status
  //char     catalog;           ///< Catalog in which the item is located: see #define CAT_* in defines.h
  //uint8_t  stackLiftStatus;   ///< Stack lift status after item execution.
  //uint8_t  undoStatus;        ///< Undo status after item execution.
} item_t;


/**
 * \struct userMenuItem_t
 * Structure keeping the information for one item of MyMenu and MyAlpha.
 */
typedef struct {
  int16_t  item;               ///< Item ID
  int16_t  unused;             ///< Padding
  char     argumentName[16];   ///< Name of variable or label
} userMenuItem_t;

/**
 * \struct userMenu_t
 * Structure keeping the information for a user-defined menu.
 */
typedef struct {
  char           menuName[16];  ///< Name of menu
  userMenuItem_t menuItem[18];  ///< Items
} userMenu_t;


/**
 * \struct programmableMenu_t
 * Structure keeping the information for programmable menu.
 */
typedef struct {
  char           itemName[18][16]; ///< Name of items
  uint16_t       itemParam[21];    ///< Parameter, XEQ if MSB set, GTO if MSB clear
  uint16_t       unused;           ///< Padding
} programmableMenu_t;


/**
 * \struct labelList_t
 * Structure keeping the information for a program label.
 */
typedef struct {
  int16_t  program;             ///< Program id
  int32_t  step;                ///< Step number of the label: <0 for a local label and >0 for a global label
  uint8_t *labelPointer;        ///< Pointer to the byte after the 0x01 op code (LBL)
  uint8_t *instructionPointer;  ///< Pointer to the instruction following the label
} labelList_t;


/**
 * \struct programList_t
 * Structure keeping the information for a program.
 */
typedef struct {
  int32_t  step;                ///< (Step number + 1) of the program begin
  uint8_t *instructionPointer;  ///< Pointer to the program begin
} programList_t;


/**
 * \struct tamState_t
 * State for TAM mode. Most of this state is internal and so not documented.
 */
typedef struct {
  /**
   * The mode used by TAM processing. If non-zero then TAM mode is active.
   * This should be used to determine whether the calculator is in TAM mode:
   *
   *     if(tam.mode) {
   *       // the calculator is in TAM mode
   *     }
   */
  uint16_t   mode;
  int16_t    function;
  /**
   * Whether input is a string rather than a number. For example, a named variable.
   * If the calculator is in alpha mode then additional details apply. See
   * ::tamProcessInput for further details.
   */
  bool_t     alpha;
  int16_t    currentOperation;
  bool_t     dot;
  bool_t     indirect;
  int16_t    digitsSoFar;
  int16_t    value;
  int16_t    min;
  int16_t    max;
  /**
   * Only used for KEYG and KEYX
   */
  int16_t    key;
  bool_t     keyAlpha;
  bool_t     keyDot;
  bool_t     keyIndirect;
  bool_t     keyInputFinished;
} tamState_t;


/**
 * \struct letteredFlagDisplay_t
   * Structure keeping the informations to display the lettered flags status.
   */
typedef struct {
  char      txt[4];              ///< lettered flag text to be displayed
  uint32_t  position;            ///< X position of the text
} letteredFlagDisplay_t;


  #if defined(PC_BUILD)
  /**
   * \struct calcKeyboard_t
   * Structure keeping key images, image sizes, and image locations.
   */
  typedef struct {
    int x, y;
    int width[4], height[4];
    GtkWidget *keyImage[4];
  } calcKeyboard_t;

  struct cfgFileParam {
    char *param;
    struct cfgFileParam *next;
  };
  typedef struct cfgFileParam cfgFileParam_t;
  #endif // PC_BUILD
#endif // TYPEDEFINITIONS_H
