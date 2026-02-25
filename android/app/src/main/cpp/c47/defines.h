// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#if !defined(DEFINES_H)
#define DEFINES_H


//*********************************
// VARIOUS OPTIONS
//*********************************

#define VERSION1 "0.109.03.01a-internal"       // major release . minor release . tracked build . internal OR un/tracked OR subrelease : Alpha / Beta / RC1

// Version 0.109.02.07b11   Public Release C47 & R47
// Version 0.109.02.07b12   Public Release C47 & R47 launch
// Version 0.109.02.07b13.1 Public Release C47 & R47
// Version 0.109.03.00b0    Public Release C47 & R47
// Version 0.109.03.00a1    Internal C47 & R47
// Version 0.109.03.00b1    Public C47 & R47, with 2 packages for DM42
// Version 0.109.03.00a2    Internal C47 & R47
// Version 0.109.03.00b2    Public C47 & R47


#if !defined(CALCMODEL)
  #define CALCMODEL USER_C47               // USER_C47 or USER_R47
#endif // !CALCMODEL

#undef SAVE_SPACE_DM42_0
#undef SAVE_SPACE_DM42_1
#undef SAVE_SPACE_DM42_3
#undef SAVE_SPACE_DM42_4
#undef SAVE_SPACE_DM42_6
#undef SAVE_SPACE_DM42_8
#undef SAVE_SPACE_DM42_8ASN
#undef SAVE_SPACE_DM42_8F
#undef SAVE_SPACE_DM42_8FL
#undef SAVE_SPACE_DM42_9
#undef SAVE_SPACE_DM42_10
#undef SAVE_SPACE_DM42_11
#undef SAVE_SPACE_DM42_12
#undef SAVE_SPACE_DM42_12PRIME
#undef SAVE_SPACE_DM42_12BESSEL
#undef SAVE_SPACE_DM42_12ORTHO
#undef SAVE_SPACE_DM42_13GRF
#undef SAVE_SPACE_DM42_13GRF_JM
#undef SAVE_SPACE_DM42_14
#undef SAVE_SPACE_DM42_15
#undef SAVE_SPACE_DM42_16
#undef SAVE_SPACE_DM42_17
#undef SAVE_SPACE_DM42_20_TIMER
#undef SAVE_SPACE_DM42_21_HP35
#undef SAVE_SPACE_DM42_22_EDIT1
#undef SAVE_SPACE_DM42_23_EDIT2
#undef SAVE_SPACE_DM42_24_PROFILES
#define LONGPRESS_CFG
#define OPTION_CUBIC_159               //                   // C47 SLVC user function is 159 digits internally;  This is needed for 34 digit input accuracy.
#undef  OPTION_SQUARE_159              // NOT NEEDED AT ALL // C47 SLVQ user function is 159 digits internally; This NOT needed for 34 digit input accuracy. Even the worst case quadratic solve is ok in the standard 75 digits.
#define OPTION_EIGEN_159               //                   // C47 EIGEN user function is 159 digits internally; This is needed for 34 digit input accuracy.
#define OPTION_XFN_1000                // NO DM42           // XFN extended 1000 digit math Functionality; does not work on DM42, due to stack constraint.
#define OPTION_TVM_FORMULAS            //                   // Use analytical formulas where possible
#define OPTION_TVM_NEWTON              //                   // Use additional newton raphson in the brent solver for tvm where possible



#if defined(DMCP_BUILD)

  #define TWO_FILE_PGM                 // Normally TWO_FILE. TWO_FILE means that QSPI is used.

  #define HWM_DM42        1
  #define HWM_DM32        2
  #define HWM_DM42n       3
  #define HARDWARE_MODEL HWM_DM42

  #if defined(NEW_HW) // DMCP5
    #undef TWO_FILE_PGM
    #undef HARDWARE_MODEL
    #define HARDWARE_MODEL HWM_DM42n
  #endif // NEW_HW

//ONE FILE OPERATION needs the original CRC file - see src/c47-dmcp
//  #undef  TWO_FILE_PGM  //See CRC ISSUE - Commented this line to force full QSPI generation
//                        //Also change the file here: src/c47-dmcp/qspi_crc.h for the single file version


//The byte counts are never accurate and depending on build system. Consider general info.
//THESE ARE DMCP COMPILE OPTIONS FOR SINGLE FILE NO QSPI (NOT POSSIBLE ANYMORE ON DM42 OLD HARDWARE)
  #if !defined(TWO_FILE_PGM) && !defined(NEW_HW) //---------THESE ARE THE EXCLUSIONS TO MAKE IT FIT WHILE NOT USING QSPI ON OLD HARDWARE
      #define SAVE_SPACE_DM42_6        //  1376 bytes // ELEC functions
      #define SAVE_SPACE_DM42_8        //  1856 bytes // Register Browser
      #define SAVE_SPACE_DM42_8FL      //  3280 bytes // Flag Browsers
      #define SAVE_SPACE_DM42_8ASN     //  1704 bytes // Assign Browser
      #define SAVE_SPACE_DM42_8F       //  1216 bytes // Font Browsers
      #define SAVE_SPACE_DM42_9        //  6712 bytes // SHOW (use either old SHOW or VIEW, change in code)
      #define SAVE_SPACE_DM42_10       //  3136 bytes // C47 programming ... (not complete removal but disables it anyway)
      #define SAVE_SPACE_DM42_12       //  3288 bytes // SLVC, SLVQ, ELLIPTIC, ZETA, BETA
      #define SAVE_SPACE_DM42_12PRIME  // 27208 bytes // ISPRIME, NEXTPRIME, FACTORS, EULPHI, MATXFACTOR
      #define SAVE_SPACE_DM42_12BESSEL //  5129 bytes // Without BESSEL
      #define SAVE_SPACE_DM42_12ORTHO  //  0768 bytes // Without ORTHO MENU
      #define SAVE_SPACE_DM42_13GRF    // 17472 bytes // Solver & graphics & stat graphics
      #define SAVE_SPACE_DM42_13GRF_JM //  7520 bytes // More graphics
      #define SAVE_SPACE_DM42_14       //   184 bytes // Load programming sample programs testPgms
      #define SAVE_SPACE_DM42_15       // 17592 bytes // Without all distributions, i.e. binomial, cauchy, chi
      #define SAVE_SPACE_DM42_16       //  2168 bytes // Without Norml distribution
      #define SAVE_SPACE_DM42_20_TIMER //  1232 bytes // Without STOPW
      #define SAVE_SPACE_DM42_21_HP35  //   200 bytes // Without config file activations only. Not complete removal.
      #define SAVE_SPACE_DM42_22_EDIT1 //  3256 bytes // Without number editing in X-register. Not complete EDIT removal.
      #define SAVE_SPACE_DM42_23_EDIT2 //  1560 bytes // Without number and function parameter editing in PEM. Not complete EDIT removal.
      #define SAVE_SPACE_DM42_24_PROFILES// 768 bytes // Without any dev profile shortcuts, and no JM, RJ & HP35
      #undef  LONGPRESS_CFG
      #undef  OPTION_CUBIC_159         //  4080 bytes // C47 SLVC function is 159 digits internally
      #undef  OPTION_SQUARE_159        //  2700 bytes // C47 SLVQ function is 159 digits internally
      #undef  OPTION_EIGEN_159         //  5480 bytes // C47 EINEN function is 159 digits internally; note both OPTION_SQUARE_159 & OPTION_CUBIC_159 used by OPTION_EIGEN_159
      #undef  OPTION_XFN_1000          //  4850 bytes // XFN extended 1000 digit math Functionality
      #undef  OPTION_TVM_FORMULAS      //  2320 bytes // Use analytical formulas where possible
      #undef  OPTION_TVM_NEWTON        //             // Use additional newton raphson in the brent solver for tvm where possible
           // DECNUMBER_FASTMUL        // manually include or exclude this option in the Makefile, DECNUMBER_FASTMUL
  #endif // !TWO_FILE_PGM && !NEW_HW

//THESE ARE DMCP COMPILE OPTIONS FOR TWO FILE QSPI
  #if defined(TWO_FILE_PGM) //---------THESE ARE THE EXCLUSIONS TO MAKE IT FIT INTO AVAILABLE FLASH EVEN WHILE USING QSPI

  #undef PACKAGE1_NOBESSEL_NOORTHO
  #undef PACKAGE2_NODISTR
  #undef PACKAGE3_NOBESSEL_NOORTHO_NOFBR
  #undef PACKAGE4_MINIMAL_MATH

  #if DMCP_PACKAGE == 1
  #define PACKAGE1_NOBESSEL_NOORTHO
  #elif DMCP_PACKAGE == 2
  #define PACKAGE2_NODISTR
  #elif DMCP_PACKAGE == 3
  #define PACKAGE3_NOBESSEL_NOORTHO_NOFBR      //More aggressive removals in addition to package 1
  #elif DMCP_PACKAGE == 4
  #define PACKAGE4_MINIMAL_MATH                //Most aggressive removals to pass gitlab pipeline CI release compiles
  #endif



  #if defined(PACKAGE1_NOBESSEL_NOORTHO)   // PACKAGE 1
      //  #define SAVE_SPACE_DM42_6        //  1352 bytes // Without ELEC functions
      //  #define SAVE_SPACE_DM42_8F       //  1216 bytes // Without Font Browsers
    #define SAVE_SPACE_DM42_12BESSEL       //  5168 bytes // Without BESSEL
    #define SAVE_SPACE_DM42_12ORTHO        //  0744 bytes // Without ORTHO MENU
      //  #define SAVE_SPACE_DM42_14       //   184 bytes // Without Load programming sample programs testPgms
      //  #define SAVE_SPACE_DM42_15       // 10056 bytes // Without all distributions, i.e. , cauchy, chi, expo, logis, t, weibull
      //  #define SAVE_SPACE_DM42_16       //  2168 bytes // Without Norml distribution
      //  #define SAVE_SPACE_DM42_17       //  9840 bytes // Without Poisson/Hyper/Binomial/Geometrical/f distributions
    #define SAVE_SPACE_DM42_21_HP35        //     0 bytes // Without config file activations only. Not complete removal
    #define SAVE_SPACE_DM42_24_PROFILES    //   240 bytes // Without any dev profile shortcuts, and no JM, RJ & HP35
    #undef OPTION_TVM_FORMULAS             //  2280 bytes // Use analytical formulas where possible
    #undef OPTION_TVM_NEWTON               //  1864 bytes // Use additional newton raphson in the brent solver for tvm where possible
  #endif

  #if defined(PACKAGE2_NODISTR)            // PACKAGE 2
      //  #define SAVE_SPACE_DM42_6        //  1352 bytes // Without ELEC functions
      //  #define SAVE_SPACE_DM42_8F       //  1216 bytes // Without Font Browsers
      //  #define SAVE_SPACE_DM42_12BESSEL //  5168 bytes // Without BESSEL
      //  #define SAVE_SPACE_DM42_12ORTHO  //  0744 bytes // Without ORTHO MENU
      //  #define SAVE_SPACE_DM42_14       //   184 bytes // Without Load programming sample programs testPgms
    #define SAVE_SPACE_DM42_15             // 10056 bytes // Without all distributions, i.e. , cauchy, chi, expo, logis, t, weibull
      //  #define SAVE_SPACE_DM42_16       //  2168 bytes // Without Norml distribution
    #define SAVE_SPACE_DM42_17             //  9840 bytes // Without Poisson/Hyper/Binomial/Geometrical/f distributions
      //  #define SAVE_SPACE_DM42_21_HP35  //     0 bytes // Without config file activations only. Not complete removal
      //  #define SAVE_SPACE_DM42_24_PROFILES// 240 bytes // Without any dev profile shortcuts, and no JM, RJ & HP35
      #define OPTION_TVM_FORMULAS          //  2280 bytes // Use analytical formulas where possible
      #define OPTION_TVM_NEWTON            //  1864 bytes // Use additional newton raphson in the brent solver for tvm where possible
  #endif


  #if defined(PACKAGE3_NOBESSEL_NOORTHO_NOFBR) // PACKAGE 3
    #define SAVE_SPACE_DM42_6              //  1352 bytes // Without ELEC functions
    #define SAVE_SPACE_DM42_8F             //  1216 bytes // Without Font Browsers
    #define SAVE_SPACE_DM42_12BESSEL       //  5168 bytes // Without BESSEL
    #define SAVE_SPACE_DM42_12ORTHO        //  0744 bytes // Without ORTHO MENU
      //  #define SAVE_SPACE_DM42_14       //   184 bytes // Without Load programming sample programs testPgms
      //  #define SAVE_SPACE_DM42_15       // 10056 bytes // Without all distributions, i.e. , cauchy, chi, expo, logis, t, weibull
      //  #define SAVE_SPACE_DM42_16       //  2168 bytes // Without Norml distribution
      //  #define SAVE_SPACE_DM42_17       //  9840 bytes // Without Poisson/Hyper/Binomial/Geometrical/f distributions
    #define SAVE_SPACE_DM42_21_HP35        //     0 bytes // Without config file activations only. Not complete removal
    #define SAVE_SPACE_DM42_24_PROFILES    //   240 bytes // Without any dev profile shortcuts, and no JM, RJ & HP35
    #undef  OPTION_TVM_FORMULAS            //  2280 bytes // Use analytical formulas where possible
    #define OPTION_TVM_NEWTON              //  1864 bytes // Use additional newton raphson in the brent solver for tvm where possible
  #endif

  #if defined(PACKAGE4_MINIMAL_MATH)       // PACKAGE 4 FOR GITLAB PIPELINE COMPILE
      //  #define SAVE_SPACE_DM42_6        //  1352 bytes // Without ELEC functions
      //  #define SAVE_SPACE_DM42_8F       //  1216 bytes // Without Font Browsers
    #define SAVE_SPACE_DM42_12BESSEL       //  5168 bytes // Without BESSEL
    #define SAVE_SPACE_DM42_12ORTHO        //  0744 bytes // Without ORTHO MENU
    #define SAVE_SPACE_DM42_14             //   184 bytes // Without Load programming sample programs testPgms
    #define SAVE_SPACE_DM42_15             // 10056 bytes // Without all distributions, i.e. , cauchy, chi, expo, logis, t, weibull
    #define SAVE_SPACE_DM42_16             //  2168 bytes // Without Norml distribution
    #define SAVE_SPACE_DM42_17             //  9840 bytes // Without Poisson/Hyper/Binomial/Geometrical/f distributions
      //  #define SAVE_SPACE_DM42_21_HP35  //     0 bytes // Without config file activations only. Not complete removal
      //  #define SAVE_SPACE_DM42_24_PROFILES// 240 bytes // Without any dev profile shortcuts, and no JM, RJ & HP35
    #define OPTION_TVM_FORMULAS            //  2280 bytes // Use analytical formulas where possible
    #undef  OPTION_TVM_NEWTON              //  1864 bytes // Use additional newton raphson in the brent solver for tvm where possible
  #endif


  //Options common to all hardware packages
  //  #define SAVE_SPACE_DM42_8        //  1856 bytes // Without Register Browser
  //  #define SAVE_SPACE_DM42_8FL      //  3280 bytes // Without Flag Browsers
  //  #define SAVE_SPACE_DM42_8ASN     //  1704 bytes // Without Assign Browser
  //  #define SAVE_SPACE_DM42_9        //  6712 bytes // Without SHOW use VIEW
  //  #define SAVE_SPACE_DM42_10       //  3136 bytes // Without C47 programming ... (not complete removal but disables it anyway)
  //  #define SAVE_SPACE_DM42_12       //  3288 bytes // Without SLVC, SLVQ, ELLIPTIC, ZETA, BETA
  //  #define SAVE_SPACE_DM42_12PRIME  // 27208 bytes // Without ISPRIME, NEXTPRIME, FACTORS, EULPHI, MATXFACTOR, NUMTHEORY
  //  #define SAVE_SPACE_DM42_13GRF    // 17472 bytes // Without Solver & graphics & stat graphics
  //  #define SAVE_SPACE_DM42_13GRF_JM //  7520 bytes // Without More graphics (full plot from memory)
    //#define SAVE_SPACE_DM42_20_TIMER //  1232 bytes // Without STOPW
    #define SAVE_SPACE_DM42_22_EDIT1   //  3256 bytes // Without number editing in X-register. Not complete EDIT removal.
    #define SAVE_SPACE_DM42_23_EDIT2   //  1560 bytes // Without number and function parameter editing in PEM. Not complete EDIT removal.
    //#undef  LONGPRESS_CFG            //  1152 bytes // Logic for longpress assignment to the f/g key

  //Large packages developed for DM42/DM42n. Could arguably work on DM42.
      #undef  OPTION_CUBIC_159         //  4080 bytes // C47 SLVC function is 159 digits internally
      #undef  OPTION_SQUARE_159        //  2700 bytes // C47 SLVQ function is 159 digits internally
      #undef  OPTION_EIGEN_159         //  5480 bytes // C47 EINEN function is 159 digits internally; note both OPTION_SQUARE_159 & OPTION_CUBIC_159 used by OPTION_EIGEN_159
      #undef  OPTION_XFN_1000          //  4850 bytes // XFN extended 1000 digit math Functionality

    //#undef  LONGPRESS_CFG            //  1152 bytes // Logic for longpress assignment to the f/g key
           // DECNUMBER_FASTMUL        // manually include or exclude this option in the Makefile, DECNUMBER_FASTMUL
  #endif // TWO_FILE_PGM
#endif // DMCP_BUILD

//2024-11-26 version 00.109.02.06: 9976 bytes to save - 9840 bytes DM42_17 - 744 byte DM42_12Ortho = 608 bytes in excess of what's needed


#define TEXT_MULTILINE_EDIT         // 5 line buffer
#define MAXLINES 5                  // numner of equavalent lines in small font maximum that is allowed in entry. Entry is hardlocked to multiline 3 lines bif font, but this is still the limit. WP has 2 lines fixed small font.
#define allowShowDigits false       // true to allow typing of double digits to get to register number nn in SHOW.
#define SHOWLineSize    120         // maximum 250
#define SHOWLineMax     (uint8_t )(TMP_STR_LENGTH / SHOWLineSize)

#define LOW_GRAPH_ACC                                                                     //Lowered graph accuracy for EQN graphs
//#undef LOW_GRAPH_ACC
#define significantDigitsForEqnGraphs (significantDigits == 0 ? 12 : significantDigits)   //If 6 is chosen by user, all four types are changes as follows: 34 to SDIGS; 39 to SDIGS+3; 51 to SDIGS+6; 75 to SDIGS+9
#define significantDigitsForScreen    3                                                   //Only for screen coord scaling of the resulting graphic matrix: 34 to 4; 39 to 4+3; 51 to 4+3; 75 to 4+3


//Testing and debugging
  #define    DM42_KEYCLICK              //Add a 1 ms click after key presses and releases, for scope syncing
  #undef     DM42_KEYCLICK
  #define    DM42_POWERMARKS
  #undef     DM42_POWERMARKS
  #define    DM42_POWERMARK_KEYPRESS
  #undef     DM42_POWERMARK_KEYPRESS

  #define    CLICK_REFRESHSCR           //Add a 5 ms click before refresh screen
  #undef     CLICK_REFRESHSCR

  #define    BATTERYTEST                //RNG nnnn is used to force the battery voltage in the simulator
  #undef     BATTERYTEST
  #define    MONITOR_VOLTAGE_INTEGRATOR
  #undef     MONITOR_VOLTAGE_INTEGRATOR


//Debug showFunctionName
#define DEBUG_SHOWNAME
#undef DEBUG_SHOWNAME
#if defined(DEBUG_SHOWNAME)
  #define DEBUGSFN true
#else
  #define DEBUGSFN false
#endif
#define   FN_TIME_DEBUG1
#undef    FN_TIME_DEBUG1

//Verbose options
  #define    VERBOSE_MINIMUM              //Minimal simulator key selections, program commands, refresh modes
//#undef     VERBOSE_MINIMUM
  #define    VERBOSEKEYS
  #undef     VERBOSEKEYS
  #define    VERBOSEKEYS_BUFFERED
  #undef     VERBOSEKEYS_BUFFERED
  #define    VERBOSEKEYS_AUTOCASE         //specifically visualizing the 1 second auto case indication in sim
  #undef     VERBOSEKEYS_AUTOCASE
  #define    MONITOR_CLRSCR
  #undef     MONITOR_CLRSCR
  #define    ANALYSE_REFRESH              //various refresh backtraces
  #undef     ANALYSE_REFRESH
  #define    PC_BUILD_TELLTALE            //JM verbose on PC: jm_show_comment
  #undef     PC_BUILD_TELLTALE
  #define    VERBOSE_DETERMINEITEM
  #undef     VERBOSE_DETERMINEITEM
  #define    VERBOSE_REGISTERS
  #undef     VERBOSE_REGISTERS
  #define    GRAPHDEBUG
  #undef     GRAPHDEBUG


//Verbose STAT
  #define DEBUG_STAT                 0 // PLOT & STATS verbose level can be 0, 1 or 2 (more)
  #if (DEBUG_STAT == 0)
    #undef STATDEBUG
    #undef STATDEBUG_VERBOSE
    #endif // DEBUG_STAT == 0
  #if (DEBUG_STAT == 1)
    #define STATDEBUG
    #undef STATDEBUG_VERBOSE
    #endif // DEBUG_STAT == 1
  #if (DEBUG_STAT == 2)
    #define STATDEBUG
    #define STATDEBUG_VERBOSE
    #endif // DEBUG_STAT == 2

//Debugging
  #if defined(PC_BUILD)
    #define DEBUGUNDO
    #undef DEBUGUNDO
    #define DEBUG_EXECUTE
    #undef DEBUG_EXECUTE
    #define DEBUG_PGM       // backtrace from insert step
    #undef DEBUG_PGM        //
  #else // !PC_BUILD
    #undef DEBUGUNDO
    #undef DEBUG_EXECUTE
    #undef DEBUG_PGM
  #endif // PC_BUILD





#define PAIMDEBUG
#undef PAIMDEBUG


#define VERBOSE_LEVEL -1              //JM -1 no text   0 = very little text; 1 = essential text; 2 = extra debugging: on calc screen

#define VERBOSE_COUNTER               //PI and SIGMA functions
#undef  VERBOSE_COUNTER

#define PC_BUILD_VERBOSE0
#undef PC_BUILD_VERBOSE0

#define PC_BUILD_VERBOSE1            //JM verbose XEQM basic operation on PC
#undef  PC_BUILD_VERBOSE1

#define PC_BUILD_VERBOSE2            //JM verbose XEQM detailed operation on PC, via central jm_show_comment1 function
#undef  PC_BUILD_VERBOSE2

#define VERBOSE_SCREEN               //JM Used at new SHOW. Needs PC_BUILD.
#undef  VERBOSE_SCREEN

#define INLINE_TEST                     //vv dr
#undef INLINE_TEST                    //^^




#define NOMATRIXCURSORS             //JM allow matrix editing to be navigated by up down keys
#undef NOMATRIXCURSORS

//This is to allow the cursors to change the case. Normal on 43S. Off on C47
#define arrowCasechange    false

//This is to allow the creation of a logfile while you type
#define RECORDLOG
#undef  RECORDLOG

//This is to really see what the LCD in the SIM does while programs are running. UGLY.
#define FULLUPDATE
//#undef  FULLUPDATE

//* Key buffer and double clicck detection
#define BUFFER_CLICK_DETECTION    //jm Evaluate the Single/Double/Triple presses
#undef BUFFER_CLICK_DETECTION

#define BUFFER_KEY_COUNT          //dr BUFFER_SIZE has to be at least 8 to become accurate results
#undef BUFFER_KEY_COUNT

#define BUFFER_SIZE 8             //dr muss 2^n betragen (8, 16, 32, 64 ...)
//* Longpress repeat
#define FUNCTION_NOPTIME   800   //JM SCREEN NOP TIMEOUT FOR FIRST 15 FUNCTIONS

#define JM_SHIFT_TIMER     4000  //ms TO_FG_TIMR
#define JM_TO_FG_LONG      580   //ms TO_FG_LONG


//FUNCTION KEYS
//Old timer constants FN
#define JM_FN_DOUBLE_TIMER 150.0   //ms TO_FN_EXEC
#define JM_TO_FN_LONG      400.0                                 * 0.7        //ms TO_FN_LONG  //  450 on 2020-03-13 //temporary factor set JM 2023-10-14
//New timer constants for FN key double press and longpress
#define TIME_FN_124_F_TO_NOP     ((uint32_t)(JM_TO_FN_LONG * 2.0 ))
#define TIME_FN_1234_F_TO_G      ((uint32_t)(JM_TO_FN_LONG       * 2.0))      //temporary factor set JM 2023-10-14
#define TIME_FN_1234_G_TO_NOP    ((uint32_t)(JM_TO_FN_LONG       * 1.8))      //temporary factor set JM 2023-10-14
#define TIME_FN_12XX_TO_F        ((uint32_t)(JM_TO_FN_LONG       * 1.7))      //temporary factor set JM 2023-10-14
#define TIME_FN_DOUBLE_G_TO_NOP  ((uint32_t)(JM_TO_FN_LONG       * 1.6))      //temporary factor set JM 2023-10-14
#define TIME_FN_DOUBLE_RELEASE   ((uint32_t)(JM_FN_DOUBLE_TIMER))



#if defined(DMCP_BUILD)
  #define JM_CLRDROP_TIMER 900   //ms TO_CL_DROP   //DROP
  #define JM_TO_CL_LONG    800   //ms TO_CL_LONG   //CLSTK
  #define JM_TO_3S_CTFF    900   //ms TO_3S_CTFF
#else // !DMCP_BUILD
  #define JM_CLRDROP_TIMER 500   //ms TO_CL_DROP   //DROP
  #define JM_TO_CL_LONG    800   //ms TO_CL_LONG   //CLSTK
  #define JM_TO_3S_CTFF    600   //ms TO_3S_CTFF
#endif // DMCP_BUILD

#define TO_KB_ACTV_MEDIUM 6000
#define TO_KB_ACTV_CURSOR  480
#define TO_KB_ACTV_SHORT    40
#define PROGRAM_KB_ACTV  60000


#define JMSHOWCODES_KB3   // top line right   Single Double Triple
#undef JMSHOWCODES_KB3

//wrapping editor
#define  combinationFontsDefault 2  //JM 0 = no large font; 1 = enlarged standardfont; 2 = combination font enlargement
                                    //JM for text wrapping editor.
                                    //JM Combintionfonts uses large numericfont characters, and if glyph not available then takes standardfont and enlarges it
                                    //JM Otherwise, full enlarged standardfont is used.


//IrFractionsCurrentStatus
#define CF_OFF                   0
#define CF_NORMAL                1


//Input mode                    //JM
#define ID_43S                   0    //JM Input Default
#define ID_DP                    2    //JM Input Default
#define ID_CPXDP                 4    //JM Input Default
#define ID_LI                    7    //JM Input Default


//*********************************
//* General configuration defines *
//*********************************
#define DEBUG_INSTEAD_STATUS_BAR         0 // Debug data instead of the status bar
#define EXTRA_INFO_ON_CALC_ERROR         1 // Print extra information on the console about an error
#define DEBUG_PANEL                      0 // Showing registers, local registers, saved stack registers, flags, statistical sums, ... in a debug panel
#define DEBUG_REGISTER_L                 0 // Showing register L content on the PC GUI
#define SHOW_MEMORY_STATUS               0 // Showing the memory status on the PC GUI
#define MMHG_PA_133_3224                 1 // mmHg to Pa conversion coefficient is 133.3224 and not 133.322387415
#define MAX_LONG_INTEGER_SIZE_IN_BITS    3328 // 1001 decimal digits: 3328 ≃ log2(10^1001)
#define MAX_FACTORIAL                    450  // Auto conversion to Real for > 450
#define SHORT_INTEGER_SIZE_IN_BLOCKS     2 // 2 blocks = 8 bytes = 64 bits
#define SHORT_INTEGER_SIZE_IN_BYTES      8 // 8 bytes = 64 bits
#define ENABLE_SOLVER_PROGRESS           1 // Set to 1 to enable solver progress display (only if called in run mode)
#define USE_MICHALSKI_MOSIG_TANH_SINH    1 // Set to 1 to use Michalski & Mosig tanh-sinh integration
#define USE_NEW_DEI_INTEGRATION_CODE     2 // 0 - use prior code. 1 - use new code. 2 - use new code with split point code.
#define ENABLE_INTEGRATOR_FILE_OUTPUT    0 // 1 for PRINTXY to be done after every evaluation of the formula; Or the complex solver for every iteration;
#define ENABLE_COMPLEXSOLVER_FILE_OUTPUT 0 // 1 for PRINTXY to be done for the complex solver for every iteration; 2 to print the RPN function; Corrupts Reg_K
#define INTEGRATION_TWO_STAGE_EXIT         // If set allows a level to complete before exiting the integrator
#undef  INTEGRATION_TWO_STAGE_EXIT
#define DECNUMDIGITS                    75 // Default number of digits used in the decNumber library
#define BIG_SCREEN_COEF                  1 // 2 = 2 times the standard screen, that is 800x480. Can be a decimal like 1.333
#define SIMULATOR_ON_SCREEN_KEYBOARD     1 // Set to 0 if you don't want an onscreen keyboard in addition to the screen
#define NARROW_SCREEN                    1 // 400x1280 portrait screen
#undef  USECURVES                          // activate spline curve option in the plot menu
#define XFN_EXTENDED_2PI_FOR_MOD         1 // for X_MOD only, if detect precise X_PI 1034 digits, it extends pi to 2139 (or as per contxt up to 6147) in XFN only. Needs to by exact, to 0 ULP difference.

#if (BIG_SCREEN_COEF > 1 && SIMULATOR_ON_SCREEN_KEYBOARD == 1)
  #undef SIMULATOR_ON_SCREEN_KEYBOARD
  #define SIMULATOR_ON_SCREEN_KEYBOARD 0
#endif // BIG_SCREEN_COEF > 1 && SIMULATOR_ON_SCREEN_KEYBOARD == 1

#if !defined(RASPBERRY) && (NARROW_SCREEN == 1)
  #undef NARROW_SCREEN
  #define NARROW_SCREEN 0
#endif // !RASPBERRY && NARROW_SCREEN == 1

#if defined(PC_BUILD) && !defined(RASPBERRY)
  #undef SIMULATOR_ON_SCREEN_KEYBOARD
  #define SIMULATOR_ON_SCREEN_KEYBOARD 1
#endif // PC_BUILD && !RASPBERRY

#define REAL34_WIDTH_TEST 0 // For debugging real34 ALL 0 formating. Use UP/DOWN to shrink or enlarge the available space. The Z register holds the available width.

//Norm_Key_00_VAR, using -1 output for not applicable, purposely out of range
#define Norm_Key_00_key   (calcModel == USER_C47 ? 0 :             calcModel == USER_DM42 ? 0 :             calcModel == USER_R47f_g ? -1 : calcModel == USER_R47bk_fg ? 10 :       calcModel == USER_R47fg_bk ? 11 : -1)
#define Norm_Key_00_keyID (calcModel == USER_C47 ? 21 :            calcModel == USER_DM42 ? 21 :            calcModel == USER_R47f_g ? -1 : calcModel == USER_R47bk_fg ? 35 :       calcModel == USER_R47fg_bk ? 36 : -1)
#define Norm_Key_00_item_in_layout  (calcModel == USER_C47 ? ITM_SIGMAPLUS : calcModel == USER_DM42 ? ITM_SIGMAPLUS : calcModel == USER_R47f_g ? -1 : calcModel == USER_R47bk_fg ? ITM_NULL : calcModel == USER_R47fg_bk ? ITM_NULL : -1)
#define isR47FAM          ((bool_t)(calcModel == USER_R47f_g || calcModel == USER_R47bk_fg || calcModel == USER_R47fg_bk || calcModel == USER_R47fg_g))

#define shortcutProfile   (calcModel == USER_C47 ? USER_C47 : isR47FAM ? USER_R47 : 0)
#define INTEGERSHORTCUTS  ((calcMode == CM_NIM || calcMode == CM_PEM) && (calcModel == USER_C47 || isR47FAM))

#define isArrowUp(code)     ( isR47FAM && code == 22 ) || (!isR47FAM && code == 17 ) // UP
#define isArrowDown(code)   ( isR47FAM && code == 27 ) || (!isR47FAM && code == 22 ) // DN

//fnKeysManagement
#define JM_ASSIGN        28
#define TO_USER          29
#define FROM_USER        30

#define USER_V47         40
#define USER_SHIFTS2     41
#define USER_E47         43

#define USER_DM42        45
#define USER_C47         46
#define USER_D47         47
#define USER_ARESET      48
#define USER_MRESET      49
#define USER_KRESET      50
#define USER_N47         51
#define USER_HRESET      58
#define USER_PRESET      59

#define USER_R47f_g      61
#define USER_R47bk_fg    62
#define USER_R47fg_bk    63
#define USER_R47fg_g     64
#define USER_EXPR        65
#define USER_R47         66  //only used for RB, profile auto selected is USER_R47f_g


//*************************
//* Other defines         *
//*************************
#define CHARS_PER_LINE                            80 // Used in the flag browser application

#define NUMERIC_FONT_HEIGHT                       36 // In pixel. Used in the font browser application
#define STANDARD_FONT_HEIGHT                      22 // In Pixel. Used in the font browser application
#define NUMBER_OF_NUMERIC_FONT_LINES_PER_SCREEN    5 // Used in the font browser application
#define NUMBER_OF_STANDARD_FONT_LINES_PER_SCREEN   8 // Used in the font browser application

#define AIM_BUFFER_LENGTH                       1024 // WP=199 double byte glyphs + trailing 0 + 1 byte to round up to a 4 byte boundary; JM increase from WP43 to 512*2 so as to exceed the 508*2+extras;
#define TAM_BUFFER_LENGTH                         32 // TODO: find the exact maximum needed
#define NIM_BUFFER_LENGTH                        200 // TODO: find the exact maximum needed

#define DEBUG_LINES                               68 // Used in for the debug panel

// List of errors
#define ERROR_NONE                                 0
#define ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN          1
#define ERROR_BAD_TIME_OR_DATE_INPUT               2
#define ERROR_UNDEFINED_OPCODE                     3
#define ERROR_OVERFLOW_PLUS_INF                    4
#define ERROR_OVERFLOW_MINUS_INF                   5
#define ERROR_LABEL_NOT_FOUND                      6
#define ERROR_FUNCTION_NOT_FOUND                   7
#define ERROR_OUT_OF_RANGE                         8
#define ERROR_INVALID_INTEGER_INPUT                9
#define ERROR_INPUT_TOO_LONG                      10
#define ERROR_RAM_FULL                            11
#define ERROR_STACK_CLASH                         12
#define ERROR_OPERATION_UNDEFINED                 13
#define ERROR_WORD_SIZE_TOO_SMALL                 14
#define ERROR_TOO_FEW_DATA                        15
#define ERROR_INVALID_DISTRIBUTION_PARAM          16
#define ERROR_IO                                  17
#define ERROR_INVALID_CORRUPTED_DATA              18
#define ERROR_FLASH_MEMORY_WRITE_PROTECTED        19
#define ERROR_NO_ROOT_FOUND                       20
#define ERROR_MATRIX_MISMATCH                     21
#define ERROR_SINGULAR_MATRIX                     22
#define ERROR_FLASH_MEMORY_FULL                   23
#define ERROR_INVALID_DATA_TYPE_FOR_OP            24
#define ERROR_NO_MVAR_FOUND                       25
#define ERROR_ENTER_NEW_NAME                      26
#define ERROR_CANNOT_DELETE_PREDEF_ITEM           27
#define ERROR_NO_SUMMATION_DATA                   28
#define ERROR_ITEM_TO_BE_CODED                    29
#define ERROR_FUNCTION_TO_BE_CODED                30
#define ERROR_INPUT_DATA_TYPE_NOT_MATCHING        31
#define ERROR_WRITE_PROTECTED_SYSTEM_FLAG         32
#define ERROR_STRING_WOULD_BE_TOO_LONG            33
#define ERROR_EMPTY_STRING                        34
#define ERROR_CANNOT_READ_FILE                    35
#define ERROR_UNDEF_SOURCE_VAR                    36
#define ERROR_WRITE_PROTECTED_VAR                 37
#define ERROR_NO_MATRIX_INDEXED                   38
#define ERROR_NOT_ENOUGH_MEMORY_FOR_NEW_MATRIX    39
#define ERROR_NO_ERRORS_CALCULABLE                40
#define ERROR_LARGE_DELTA_AND_OPPOSITE_SIGN       41
#define ERROR_SOLVER_REACHED_LOCAL_EXTREMUM       42
#define ERROR_INITIAL_GUESS_OUT_OF_DOMAIN         43
#define ERROR_FUNCTION_VALUES_LOOK_CONSTANT       44
#define ERROR_SYNTAX_ERROR_IN_EQUATION            45
#define ERROR_EQUATION_TOO_COMPLEX                46
#define ERROR_CANNOT_ASSIGN_HERE                  47
#define ERROR_INVALID_NAME                        48
#define ERROR_TOO_MANY_VARIABLES                  49 // unlikely
#define ERROR_NON_PROGRAMMABLE_COMMAND            50
#define ERROR_NO_GLOBAL_LABEL                     51
#define ERROR_INVALID_DATA_TYPE_FOR_POLAR_RECT    52
#define ERROR_BAD_INPUT                           53 // This error is not in ReM and cannot occur (theoretically).
#define ERROR_NO_PROGRAM_SPECIFIED                54
#define ERROR_CANNOT_WRITE_FILE                   55
#define ERROR_OLD_ITEM_TO_REPLACE                 56
#define ERROR_VARIABLE_NOT_SELECTED               57
#define ERROR_IPX_INVALID_FOR_SI                  58
#define ERROR_UNDEF_MENU                          59
#define ERROR_SOLVER_ABORT                        60
#define ERROR_RESERVED_VARIABLE_NAME              61
#define ERROR_INVALID_TYPE_XFN                    62
#define LAST_ERROR_MESSAGE                        62

//Status output messages for time consuming tasks, to keep user informed
#define LOADING_STATE_FILE                       100
#define SAVING_STATE_FILE                        101
#define RESTORING_STATS                          102
#define COMPLEX_SOLVER                           103
#define GRAPHING                                 104
#define RECALC_SUMS                              105
#define REAL_SOLVER                              106

//TI Messages (incomplete)
#define TI_Backup_restored                       107
#define TI_State_file_restored                   108
#define TI_Saved_programs_and_equations          109
#define TI_appended                              110
#define TI_Saved_global_and_local_registers      111
#define TI_w_local_flags_restored                112
#define TI_Saved_system_settings_restored        113
#define TI_Saved_statistic_data_restored         114
#define TI_Saved_user_variables_restored         115
#define TI_Program_file_loaded                   116
#define TI_All_user_flags_cleared                117
#define TI_All_data_prgms_cleared                118
#define TI_All_user_menus_cleared                119
#define TI_All_user_vars_cleared                 120
#define TI_All_user_prgms_deleted                121
#define TI_All_user_menus_deleted                122
#define TI_All_user_vars_deleted                 123

//TI & ERROR Messages
#define TI_Not_on_simulator                      124
#define TI_Only_on_simulator                     125
#define ERROR_TI_UNDO_FAILED                     126


#define NUMBER_OF_ERROR_CODES                    127
#define SIZE_OF_EACH_ERROR_MESSAGE                48

#define NUMBER_OF_BUG_SCREEN_MESSAGES             10
#define SIZE_OF_EACH_BUG_SCREEN_MESSAGE          100

////////////////////////////////////////////////////
// Flags are numbered the same way in key
// stroke programs code and in C programs
// unlike registers
#define NUMBER_OF_GLOBAL_FLAGS                   112
#define LAST_GLOBAL_FLAG                         111
#define FIRST_LOCAL_FLAG                         112 // There are 112 global flag from 0 to 111
#define NUMBER_OF_LOCAL_FLAGS                     32
#define LAST_LOCAL_FLAG                          143

// Global flags
#define FLAG_X                                   100
#define FLAG_Y                                   101
#define FLAG_Z                                   102
#define FLAG_T                                   103
#define FLAG_A                                   104
#define FLAG_B                                   105
#define FLAG_C                                   106
#define FLAG_D                                   107
#define FLAG_L                                   108
#define FLAG_I                                   109
#define FLAG_J                                   110
#define FLAG_K                                   111
#define FLAG_M                                   211
#define FLAG_N                                   212
#define FLAG_P                                   213
#define FLAG_Q                                   214
#define FLAG_R                                   215
#define FLAG_S                                   216
#define FLAG_E                                   217
#define FLAG_F                                   218
#define FLAG_G                                   219
#define FLAG_H                                   220
#define FLAG_O                                   221
#define FLAG_U                                   222
#define FLAG_V                                   223
#define FLAG_W                                   224

// System flags
// Bit 15 (MSB) is always set for a system flag
// If bit 14 is set the system flag is read only for the user
#define FLAG_TDM24                            0x8000 // The system flags
#define FLAG_YMD                              0xc001 // MUST be in the same
#define FLAG_DMY                              0xc002 // order as the items
#define FLAG_MDY                              0xc003 // in items.c and items.h
#define FLAG_CPXRES                           0x8004
#define FLAG_CPXj                             0x8005 // And TDM24 MUST be
#define FLAG_POLAR                            0x8006 // the first.
#define FLAG_FRACT                            0x8007
#define FLAG_PROPFR                           0x8008
#define FLAG_DENANY                           0x8009
#define FLAG_DENFIX                           0x800a
#define FLAG_CARRY                            0x800b
#define FLAG_OVERFLOW                         0x800c
#define FLAG_LEAD0                            0x800d
#define FLAG_ALPHA                            0x800e
#define FLAG_alphaCAP                         0xc00f
#define FLAG_RUNTIM                           0xc010
#define FLAG_RUNIO                            0xc011
#define FLAG_PRINTS                           0xc012
#define FLAG_TRACE                            0x8013
#define FLAG_USER                             0x8014
#define FLAG_LOWBAT                           0xc015
#define FLAG_SLOW                             0x8016
#define FLAG_SPCRES                           0x8017
#define FLAG_SSIZE8                           0x8018
#define FLAG_QUIET                            0x8019
#define FLAG_WRAPEND                          0xc01a
#define FLAG_MULTx                            0x801b
#define FLAG_ENGOVR                           0x801c
#define FLAG_GROW                             0x801d
#define FLAG_AUTOFF                           0x801e
#define FLAG_AUTXEQ                           0x801f
#define FLAG_PRTACT                           0x8020
#define FLAG_NUMIN                            0x8021
#define FLAG_ALPIN                            0x8022
#define FLAG_ASLIFT                           0xc023
#define FLAG_IGN1ER                           0x8024
#define FLAG_INTING                           0xc025
#define FLAG_SOLVING                          0xc026
#define FLAG_VMDISP                           0xc027
#define FLAG_USB                              0xc028
#define FLAG_ENDPMT                           0xc029
#define FLAG_FRCSRN                           0x802a
#define FLAG_HPRP                             0x802b
#define FLAG_SBdate                           0x802C
#define FLAG_SBtime                           0x802D
#define FLAG_SBcr                             0x802E
#define FLAG_SBcpx                            0x802F
#define FLAG_SBang                            0x8030
#define FLAG_SBfrac                           0x8031
#define FLAG_SBint                            0x8032
#define FLAG_SBmx                             0x8033
#define FLAG_SBtvm                            0x8034
#define FLAG_SBoc                             0x8035
#define FLAG_SBss                             0x8036
#define FLAG_SBstpw                           0x8037
#define FLAG_SBser                            0x8038
#define FLAG_SBprn                            0x8039
#define FLAG_SBbatV                           0x803A
#define FLAG_SBshfR                           0x803B
#define FLAG_HPBASE                           0x803C
#define FLAG_2TO10                            0x803D
#define FLAG_SH_LONGPRESS                     0x803E
#define FLAG_WRAPEDG                          0xc03F
#define FLAG_MONIT                            0x8040 // MONIT MUST be the first of the second flag word
#define FLAG_FRCYC                            0x8041
#define FLAG_HPCONV                           0x8042 //shifted here from 0x8044
#define FLAG_NUMLOCK                          0x8043
#define FLAG_CPXMULT                          0x8044
#define FLAG_ERPN                             0x8045
#define FLAG_LARGELI                          0x8046
#define FLAG_IRFRAC                           0x8047
#define FLAG_IRFRQ                            0xc048
#define FLAG_PFX_ALL                          0x8049
#define FLAG_DREAL                            0x804A
#define FLAG_CPXPLOT                          0x804B
#define FLAG_SHOWX                            0x804C
#define FLAG_SHOWY                            0x804D
#define FLAG_PBOX                             0x804E
#define FLAG_PCROS                            0x804F //16
#define FLAG_PPLUS                            0x8050
#define FLAG_PLINE                            0x8051
#define FLAG_SCALE                            0x8052
#define FLAG_VECT                             0x8053 //20
#define FLAG_NVECT                            0x8054
#define FLAG_US                               0x8055
#define FLAG_MNUp1                            0x8056
#define FLAG_SBwoy                            0x8057
#define FLAG_TOPHEX                           0x8058
#define FLAG_BCD                              0x8059 //26
#define FLAG_PCURVE                           0x805A //27
#define FLAG_CLX_DROP                         0x805B //28
#define FLAG_BASE_MYM                         0x805C
#define FLAG_G_DOUBLETAP                      0x805D
#define FLAG_BASE_HOME                        0x805E
#define FLAG_MYM_TRIPLE                       0x805F
#define FLAG_HOME_TRIPLE                      0x8060
#define FLAG_SHFT_4s                          0x8061
#define FLAG_FGLNLIM                          0x8062
#define FLAG_FGLNFUL                          0x8063
#define FLAG_FGGR                             0x8064

#define NUMBER_OF_SYSTEM_FLAGS                 64+28+6+3 // We can have a maximum of 128 system flags

                                                     // only used as bit count for setting change detection
#define SETTING_AMODE                         0x0080 // current angle mode
#define SETTING_DMX                           0x0081 // denMax
#define SETTING_SINT_WS                       0x0082 // shortIntegerWordSize
#define SETTING_SINT_MODE                     0x0083 // shortIntegerMode
#define SETTING_WATCHICON                     0x0084 // the bit controlling the watch face icon
#define SETTING_SIOICON                       0x0085 // the bit controlling the serial i/o activity icon
#define SETTING_PRINTERICON                   0x0086 // the bit controlling the IR printer icon


// FLGS and STATUS SCREENS
#define NO_SCREEN                          0  // No screen selected
#define FIRST_SCREEN                       1
#define STATUS_SCREEN                      1  // Flags Status summary screen
#define SYSTEM_FLAGS_SCREEN_1              2  // System Flags 1st screen
#define SYSTEM_FLAGS_SCREEN_2              3  // System Flags 2nd screen
#define GLOBAL_FLAGS_SCREEN                4  // Global Flags 00-99 screen
#define LETTERED_AND_LOCAL_FLAGS_SCREEN    5  // Global Lettered Flags and Local FLags screen
#define LAST_SCREEN                        5



typedef enum {
  LI_ZERO     = 0, // Long integer sign 0
  LI_NEGATIVE = 1, // Long integer sign -
  LI_POSITIVE = 2  // Long integer sign +
} longIntegerSign_t;




// PC GUI
#if NARROW_SCREEN == 1
  #define CSSFILE "res/c47_narrow_screen_pre.css"
  #define GAP                                        6 //JM original GUI legacy
  #define Y_OFFSET_LETTER                           18 //JM original GUI legacy
  #define X_OFFSET_LETTER                            3 //JM original GUI legacy
  #define Y_OFFSET_Aim                              25 //JM original GUI legacy

  #define DELTA_KEYS_X                              67 // Horizontal key step in pixel (row of 6 keys)
  #define DELTA_KEYS_Y                              74 // Vertical key step in pixel
  #define KEY_WIDTH_1                               44 // Width of small keys (STO, RCL, ...)
  #define KEY_WIDTH_2                               54 // Width of large keys (1, 2, 3, ...)
  #define LARGE_KEY_SPACING_1                       12 // Spacing 1st large key --> 7, 4, 1 and 0
  #define LARGE_KEY_SPACING_2                       15 // Spacing next large keys --> 8, 9, ÷, 5, 6, …

  #define X_LEFT_PORTRAIT                           10 // Horizontal offset for a portrait calculator
  #define X_LEFT_LANDSCAPE                         544 // Horizontal offset for a landscape calculator
  #define Y_TOP_PORTRAIT                           267 // Vertical offset for a portrait calculator
  #define Y_TOP_LANDSCAPE                           30 // vertical offset for a landscape calculator
#else // NARROW_SCREEN != 1
  #define CSSFILE "res/c47_pre.css"
  #define GAP                                        6 //JM original GUI legacy
  #define Y_OFFSET_LETTER                           18 //JM original GUI legacy
  #define X_OFFSET_LETTER                            3 //JM original GUI legacy
  #define Y_OFFSET_Aim                              25 //JM original GUI legacy

  #define DELTA_KEYS_X                              78 // Horizontal key step in pixel (row of 6 keys)
  #define DELTA_KEYS_Y                              74 // Vertical key step in pixel
  #define KEY_WIDTH_1                               47 // Width of small keys (STO, RCL, ...)
  #define KEY_WIDTH_2                               56 // Width of large keys (1, 2, 3, ...)
  #define LARGE_KEY_SPACING_1                       18 // Spacing 1st large key --> 7, 4, 1 and 0
  #define LARGE_KEY_SPACING_2                       17 // Spacing next large keys --> 8, 9, ÷, 5, 6, …

  #define X_LEFT_PORTRAIT                           45 // Horizontal offset for a portrait calculator
  #define X_LEFT_LANDSCAPE                         544 // Horizontal offset for a landscape calculator
  #define Y_TOP_PORTRAIT                           376 // Vertical offset for a portrait calculator
  #define Y_TOP_LANDSCAPE                           30 // vertical offset for a landscape calculator
#endif // NARROW_SCREEN == 1


#define TAM_MAX_BITS                              14
#define TAM_MAX_MASK                          0x3fff

// Stack Lift Status (2 bits)
#define SLS_STATUS                            0x0003 // 0000 0011
#define SLS_ENABLED                        ( 0 << 0) // Stack lift enabled after item execution
#define SLS_DISABLED                       ( 1 << 0) // Stack lift disabled after item execution
#define SLS_UNCHANGED                      ( 2 << 0) // Stack lift unchanged after item execution

// Undo Status (2 bits)
#define US_STATUS                             0x000c // 0000 1100
#define US_ENABLED                         ( 0 << 2) // The command saves the stack, the statistical sums, and the system flags for later UNDO
#define US_CANCEL                          ( 1 << 2) // The command cancels the last UNDO data
#define US_UNCHANGED                       ( 2 << 2) // The command leaves the existing UNDO data as is
#define US_ENABL_XEQ                       ( 3 << 2) // Like US_STATUS, but if there is not enough memory for UNDO, deletes UNDO data then continue

// Item category (4 bits)
#define CAT_STATUS                            0x00f0
#define CAT_NONE                           ( 0 << 4) // None of the others
#define CAT_FNCT                           ( 1 << 4) // Function
#define CAT_MENU                           ( 2 << 4) // Menu
#define CAT_CNST                           ( 3 << 4) // Constant
#define CAT_FREE                           ( 4 << 4) // To identify and find the free items
#define CAT_REGS                           ( 5 << 4) // Registers
#define CAT_RVAR                           ( 6 << 4) // Reserved variable
#define CAT_DUPL                           ( 7 << 4) // Duplicate of another item e.g. acus->m^2
#define CAT_SYFL                           ( 8 << 4) // System flags
#define CAT_AINT                           ( 9 << 4) // Upper case alpha_INTL
#define CAT_aint                           (10 << 4) // Lower case alpha_intl
#define CAT_MNUH                           (11 << 4) // Menu, Hidden, not appearing in the catalogue: Hidden menu, eg. 'DEV', accessible with [XEQ] 'OPENM' 'DEV' or [P.FN] [OPENM] 'DEV'

// EIM (Equation Input Mode) status (1 bit)
#define EIM_STATUS                            0x0100
#define EIM_DISABLED                        (0 << 8) // Function disabled in EIM
#define EIM_ENABLED                         (1 << 8) // Function enabled in EIM

// Parameter Type in Program status (4 bit)
#define PTP_STATUS                            0x1e00 // 0001 1110 0000 0000
#define PTP_NONE                           ( 0 << 9) // No parameters
#define PTP_DECLARE_LABEL                  ( 1 << 9) // These
#define PTP_LABEL                          ( 2 << 9) //   parameter
#define PTP_REGISTER                       ( 3 << 9) //   types
#define PTP_FLAG                           ( 4 << 9) //   must
#define PTP_NUMBER_8                       ( 5 << 9) //   match
#define PTP_NUMBER_16                      ( 6 << 9) //   with
#define PTP_COMPARE                        ( 7 << 9) //   PARAM_*
#define PTP_KEYG_KEYX                      ( 8 << 9) //   defined
#define PTP_SKIP_BACK                      ( 9 << 9) //   below.
#define PTP_NUMBER_8_16                    (10 << 9) //
#define PTP_SHUFFLE                        (11 << 9) //
#define PTP_MENU                           (12 << 9) //
#define PTP_LITERAL                        (13 << 9) // Literal
#define PTP_REM                            (14 << 9) //
#define PTP_DISABLED                       (15 << 9) // Not programmable

// Hourglass enable (2 bits)
#define HG_STATUS                            0x6000  // 0110 0000 0000 0000
#define HG_ENABLED                         ( 0 << 13 ) // Hourglass enabled
#define HG_ENABLED_MX_ONLY                 ( 1 << 13 ) // Hourglass disabled except when matrixes are in X or Y
#define HG_DISABLED                        ( 2 << 13 ) // Hourglass blocked

// EIM function parameter number - Note, if we need a bit here for more important tasks, we can convert this information into an array in equation.c, sized [2,22] so no big loss to do.
#define EIM_INPUT                            0x8000  // 1000 0000 0000 0000
#define EIM_NI_MO                          ( 0 << 15 ) // MONADIC or NILADIC
#define EIM_DY                             ( 1 << 15 ) // DYADIC

#define INC_FLAG                                   0
#define DEC_FLAG                                   1


//Export type
#define MODE_NRM 2
#define MODE_RTF 1


// List of constants
#define FIRST_CONSTANT                        CST_01
#define LAST_CONSTANT                         CST_84

// Local labels
#define FIRST_LOCAL_LABEL        0                             //   0 - 99 and A to L
#define FIRST_UC_LOCAL_LABEL   100                             //   A (first upper case local label
#define LAST_UC_LOCAL_LABEL    111                             //   L (last  upper case local label
#define FIRST_LC_LOCAL_LABEL   112                             //   a (first lower case local label
#define LAST_LOCAL_LABEL       123                             //   0 - 99, A to L and a to l

//Variable names
#define VAR_NO_X        0
#define VAR_NO_Y        1
#define VAR_NO_Z        2
#define VAR_NO_T        3
#define VAR_NO_A        4
#define VAR_NO_B        5
#define VAR_NO_C        6
#define VAR_NO_D        7
#define VAR_NO_L        8
#define VAR_NO_I        9
#define VAR_NO_J       10
#define VAR_NO_K       11
#define VAR_NO_M       12
#define VAR_NO_N       13
#define VAR_NO_P       14
#define VAR_NO_Q       15
#define VAR_NO_R       16
#define VAR_NO_S       17
#define VAR_NO_E       18
#define VAR_NO_F       19
#define VAR_NO_G       20
#define VAR_NO_H       21
#define VAR_NO_O       22
#define VAR_NO_U       23
#define VAR_NO_V       24
#define VAR_NO_W       25
#define VAR_NO_ADM     26
#define VAR_NO_DENMAX  27
#define VAR_NO_ISM     28
#define VAR_NO_REALDF  29
#define VAR_NO_NDEC    30
#define VAR_NO_ACC     31
#define VAR_NO_ULIM    32
#define VAR_NO_LLIM    33
#define VAR_NO_FV      34
#define VAR_NO_IPONA   35
#define VAR_NO_NPPER   36
#define VAR_NO_PPERONA 37
#define VAR_NO_PMT     38
#define VAR_NO_PV      39
#define VAR_NO_GRAMOD  40
#define VAR_NO_UX      41
#define VAR_NO_LX      42
#define VAR_NO_CPERONA 43
#define VAR_NO_UEST    44
#define VAR_NO_LEST    45
#define VAR_NO_UY      46
#define VAR_NO_LY      47


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Register numbering for                                                                           Register numbering in key
// the C47 C program:                                                                               stroke programs (one byte)
// 0…99                                  Global registers from 0 to 99                              0…99
// 100…111                           Lettered global registers from X to L                          100…111
// 112…117         Lettered global registers from M to S: no possibility of indirect access         211…216
// 118…125         Lettered global registers from E to W: no possibility of indirect access         217…224
//                                        25 undefined free registers                               225…249
// 126…134                 saved stack registers (UNDO feature) not user accessible
// 135…136                          temporary registers not user accessible
// 137…249              113 undefined free registers: no possibility of indirect access
//
//                             SYSTEM_FLAG_NUMBER --> Used for system flag access                   250
//                                  VALUE_0 --> Can't remember what this is!                        251
//                                  VALUE_1 --> Can't remember what this is!                        252
//                                           STRING_LABEL_VARIABLE                                  253
//                                             INDIRECT_REGISTER                                    254
//                                             INDIRECT_VARIABLE                                    255
// 256  to 1999                                 named variables
// 2000 to 2029                               reserved variables
// 7000 to 7098                         Local registers from .00 to .98                             112…210

enum REG_NUMBERS { // C program register codes
  FIRST_GLOBAL_REGISTER = 0,                             //   0 - 99 Total 100 registers

  FIRST_LETTERED_REGISTER = 100,
  REGISTER_X = FIRST_LETTERED_REGISTER,                  // 100 9 stack registers
  REGISTER_Y,                                            // 101
  REGISTER_Z,                                            // 102
  REGISTER_T,                                            // 103
  REGISTER_A,                                            // 104
  REGISTER_B,                                            // 105
  REGISTER_C,                                            // 106
  REGISTER_D,                                            // 107
  REGISTER_L,                                            // 108
  REGISTER_I,                                            // 109 3 matrix registers
  REGISTER_J,                                            // 110
  REGISTER_K,                                            // 111
  LAST_LETTERED_REGISTER = REGISTER_K,

  // Statistical parameter registers
  FIRST_STAT_REGISTER,
  REGISTER_M = FIRST_STAT_REGISTER,                      // 112
  REGISTER_N,                                            // 113
  REGISTER_P,                                            // 114
  REGISTER_Q,                                            // 115
  REGISTER_R,                                            // 116
  REGISTER_S,                                            // 117
  LAST_STAT_REGISTER = REGISTER_S,

  // Spare registers
  FIRST_SPARE_REGISTER,
  REGISTER_E = FIRST_SPARE_REGISTER,                     // 118
  REGISTER_F,                                            // 119
  REGISTER_G,                                            // 120
  REGISTER_H,                                            // 121
  REGISTER_O,                                            // 122
  REGISTER_U,                                            // 123
  REGISTER_V,                                            // 124
  REGISTER_W,                                            // 125
  LAST_SPARE_REGISTER = REGISTER_W,

  // Saved stack registers
  FIRST_SAVED_STACK_REGISTER,
  SAVED_REGISTER_X = FIRST_SAVED_STACK_REGISTER,         // 126
  SAVED_REGISTER_Y,                                      // 127
  SAVED_REGISTER_Z,                                      // 128
  SAVED_REGISTER_T,                                      // 129
  SAVED_REGISTER_A,                                      // 130
  SAVED_REGISTER_B,                                      // 131
  SAVED_REGISTER_C,                                      // 132
  SAVED_REGISTER_D,                                      // 133
  SAVED_REGISTER_L,                                      // 134
  LAST_SAVED_STACK_REGISTER = SAVED_REGISTER_L,

  // Temporary registers
  FIRST_TEMP_REGISTER,
  TEMP_REGISTER_1 = FIRST_TEMP_REGISTER,                 // 135
  TEMP_REGISTER_2_SAVED_STATS,                           // 136
  LAST_TEMP_REGISTER = TEMP_REGISTER_2_SAVED_STATS,

  LAST_GLOBAL_REGISTER = TEMP_REGISTER_2_SAVED_STATS,

  // Named variables
  FIRST_NAMED_VARIABLE = 256,                            // 256
  LAST_NAMED_VARIABLE = 1999,                            //1999

  // Reserved variables
  FIRST_RESERVED_VARIABLE,
  RESERVED_VARIABLE_X = FIRST_RESERVED_VARIABLE,         //2000
  RESERVED_VARIABLE_Y,                                   //2001
  RESERVED_VARIABLE_Z,                                   //2002
  RESERVED_VARIABLE_T,                                   //2003
  RESERVED_VARIABLE_A,                                   //2004
  RESERVED_VARIABLE_B,                                   //2005
  RESERVED_VARIABLE_C,                                   //2006
  RESERVED_VARIABLE_D,                                   //2007
  RESERVED_VARIABLE_L,                                   //2008
  RESERVED_VARIABLE_I,                                   //2009
  RESERVED_VARIABLE_J,                                   //2010
  RESERVED_VARIABLE_K,                                   //2011
  RESERVED_VARIABLE_M,                                   //2012
  RESERVED_VARIABLE_N,                                   //2013
  RESERVED_VARIABLE_P,                                   //2014
  RESERVED_VARIABLE_Q,                                   //2015
  RESERVED_VARIABLE_R,                                   //2016
  RESERVED_VARIABLE_S,                                   //2017
  RESERVED_VARIABLE_E,                                   //2018
  RESERVED_VARIABLE_F,                                   //2019
  RESERVED_VARIABLE_G,                                   //2020
  RESERVED_VARIABLE_H,                                   //2021
  RESERVED_VARIABLE_O,                                   //2022
  RESERVED_VARIABLE_U,                                   //2023
  RESERVED_VARIABLE_V,                                   //2024
  RESERVED_VARIABLE_W,                                   //2025

  // Named reserved variables
  FIRST_NAMED_RESERVED_VARIABLE,
  RESERVED_VARIABLE_ADM = FIRST_NAMED_RESERVED_VARIABLE, //2026
  RESERVED_VARIABLE_DENMAX,                              //2027
  RESERVED_VARIABLE_ISM,                                 //2028
  RESERVED_VARIABLE_REALDF,                              //2029
  RESERVED_VARIABLE_NDEC,                                //2030
  RESERVED_VARIABLE_ACC,                                 //2031
  RESERVED_VARIABLE_ULIM,                                //2032
  RESERVED_VARIABLE_LLIM,                                //2033
  RESERVED_VARIABLE_FV,                                  //2034
  RESERVED_VARIABLE_IPONA,                               //2035
  RESERVED_VARIABLE_NPPER,                               //2036
  RESERVED_VARIABLE_PPERONA,                             //2037
  RESERVED_VARIABLE_PMT,                                 //2038
  RESERVED_VARIABLE_PV,                                  //2039
  RESERVED_VARIABLE_GRAMOD,                              //2040
  RESERVED_VARIABLE_UX,                                  //2041
  RESERVED_VARIABLE_LX,                                  //2042
  RESERVED_VARIABLE_CPERONA,                             //2043
  RESERVED_VARIABLE_UEST,                                //2044
  RESERVED_VARIABLE_LEST,                                //2045
  RESERVED_VARIABLE_UY,                                  //2046
  RESERVED_VARIABLE_LY,                                  //2047
  //  RESERVED_SPARES_HERE
  LAST_RESERVED_VARIABLE = RESERVED_VARIABLE_LY,

  INVALID_VARIABLE_OLD = 2043,                           //2043   // Used to fix the backup.cfg loading
  INVALID_VARIABLE = 2199,                               //2199   // Old backup.cfg files will contain currentInputVariable to be 2043, which is fixed

  // Labels
  FIRST_LABEL,                                           //2044
  LAST_LABEL = 6999,                                     //6999

  // Local registers
  FIRST_LOCAL_REGISTER,                                  //7000
  LAST_LOCAL_REGISTER = FIRST_LOCAL_REGISTER + 99 - 1    //7098 total 99 local registers,
};

enum REG_NUMBERS_IN_KS_CODE { // Key Stroke register codes
  FIRST_GLOBAL_REGISTER_IN_KS_CODE = 0,                                       //  0 - 99 Total 100 global registers

  FIRST_LETTERED_REGISTER_IN_KS_CODE = 100,
  REGISTER_X_IN_KS_CODE = FIRST_LETTERED_REGISTER_IN_KS_CODE,                 // 100 9 stack registers
  REGISTER_Y_IN_KS_CODE,                                                      // 101
  REGISTER_Z_IN_KS_CODE,                                                      // 102
  REGISTER_T_IN_KS_CODE,                                                      // 103
  REGISTER_A_IN_KS_CODE,                                                      // 104
  REGISTER_B_IN_KS_CODE,                                                      // 105
  REGISTER_C_IN_KS_CODE,                                                      // 106
  REGISTER_D_IN_KS_CODE,                                                      // 107
  REGISTER_L_IN_KS_CODE,                                                      // 108
  REGISTER_I_IN_KS_CODE,                                                      // 109 3 matrix registers
  REGISTER_J_IN_KS_CODE,                                                      // 110
  REGISTER_K_IN_KS_CODE,                                                      // 111
  LAST_LETTERED_REGISTER_IN_KS_CODE = REGISTER_K_IN_KS_CODE,

  LAST_GLOBAL_REGISTER_IN_KS_CODE = LAST_LETTERED_REGISTER_IN_KS_CODE,

  // Local registers
  FIRST_LOCAL_REGISTER_IN_KS_CODE,                                            // 112
  LAST_LOCAL_REGISTER_IN_KS_CODE = FIRST_LOCAL_REGISTER_IN_KS_CODE + 99 - 1,  // 210, total 99 registers,

  // Statistical parameter registers
  FIRST_STAT_REGISTER_IN_KS_CODE,
  REGISTER_M_IN_KS_CODE = FIRST_STAT_REGISTER_IN_KS_CODE,                     // 211
  REGISTER_N_IN_KS_CODE,                                                      // 212
  REGISTER_P_IN_KS_CODE,                                                      // 213
  REGISTER_Q_IN_KS_CODE,                                                      // 214
  REGISTER_R_IN_KS_CODE,                                                      // 215
  REGISTER_S_IN_KS_CODE,                                                      // 216
  LAST_STAT_REGISTER_IN_KS_CODE = REGISTER_S_IN_KS_CODE,

  // Spare registers
  FIRST_SPARE_REGISTERS_IN_KS_CODE,
  REGISTER_E_IN_KS_CODE = FIRST_SPARE_REGISTERS_IN_KS_CODE,                   // 217
  REGISTER_F_IN_KS_CODE,                                                      // 218
  REGISTER_G_IN_KS_CODE,                                                      // 219
  REGISTER_H_IN_KS_CODE,                                                      // 220
  REGISTER_O_IN_KS_CODE,                                                      // 221
  REGISTER_U_IN_KS_CODE,                                                      // 222
  REGISTER_V_IN_KS_CODE,                                                      // 223
  REGISTER_W_IN_KS_CODE,                                                      // 224
  LAST_SPARE_REGISTERS_IN_KS_CODE = REGISTER_W_IN_KS_CODE,

  // OP parameter special values
  CNST_BEYOND_250       = 250,
  //CNST_BEYOND_500       = 251,
  //CNST_BEYOND_750       = 252,
  SYSTEM_FLAG_NUMBER    = 250,
  VALUE_0               = 251,
  VALUE_1               = 252,
  STRING_LABEL_VARIABLE = 253,
  INDIRECT_REGISTER     = 254,
  INDIRECT_VARIABLE     = 255,
};

#define NUMBER_OF_GLOBAL_REGISTERS      (LAST_GLOBAL_REGISTER          - FIRST_GLOBAL_REGISTER          + 1) // 137 = 100 numbered + 12 lettered + 6 stat + 8 spare + 9 saved_stach + 2 temp
#define NUMBER_OF_LETTERED_REGISTERS    (LAST_LETTERED_REGISTER        - FIRST_LETTERED_REGISTER        + 1) // 12 lettered from X to K
#define NUMBER_OF_STAT_REGISTERS        (LAST_STAT_REGISTER            - FIRST_STAT_REGISTER            + 1) // 6 lettered from M to S
#define NUMBER_OF_SPARE_REGISTERS       (LAST_SPARE_REGISTER           - FIRST_SPARE_REGISTER           + 1) // 8 lettered from E to W
#define NUMBER_OF_SAVED_STACK_REGISTERS (LAST_SAVED_STACK_REGISTER     - FIRST_SAVED_STACK_REGISTER     + 1) // 9
#define NUMBER_OF_TEMP_REGISTERS        (LAST_TEMP_REGISTER            - FIRST_TEMP_REGISTER            + 1) // 2
#define NUMBER_OF_LOCAL_REGISTERS       (LAST_LOCAL_REGISTER           - FIRST_LOCAL_REGISTER           + 1) // 99 from .00 to .98

#define NUMBER_OF_RESERVED_VARIABLES    (LAST_RESERVED_VARIABLE        - FIRST_RESERVED_VARIABLE        + 1) // 41
#define NUMBER_OF_LETTERED_VARIABLES    (FIRST_NAMED_RESERVED_VARIABLE - FIRST_RESERVED_VARIABLE)            // 26

#define RBR_INCDEC1 10
#define LAST_GLOBAL_REGISTER_SCREEN LAST_SPARE_REGISTER - modulo(LAST_SPARE_REGISTER, RBR_INCDEC1)

#define FAILED_INDIRECTION                      9999

/* Convertion from a key stroke program register code to a C register number
 */
static inline int16_t regKStoC(const uint8_t regKS) {
  return   (int16_t)regKS
         - (FIRST_STAT_REGISTER_IN_KS_CODE  <= regKS && regKS <= LAST_SPARE_REGISTERS_IN_KS_CODE) * NUMBER_OF_LOCAL_REGISTERS
         + (FIRST_LOCAL_REGISTER_IN_KS_CODE <= regKS && regKS <= LAST_LOCAL_REGISTER_IN_KS_CODE)  * (FIRST_LOCAL_REGISTER - FIRST_LOCAL_REGISTER_IN_KS_CODE);
}

/* Convertion from a C register number to a key stroke program register code
 */
static inline uint8_t regCtoKS(const int16_t regC) {
  return (uint8_t)(  regC
                   + (FIRST_STAT_REGISTER  <= regC && regC <= LAST_SPARE_REGISTER) * NUMBER_OF_LOCAL_REGISTERS
                   - (FIRST_LOCAL_REGISTER <= regC && regC <= LAST_LOCAL_REGISTER) * (FIRST_LOCAL_REGISTER - FIRST_LOCAL_REGISTER_IN_KS_CODE));
}

// If one of the 4 next defines is changed: change also Y_POSITION_OF_???_LINE below
#define AIM_REGISTER_LINE                 REGISTER_X
#define TAM_REGISTER_LINE                 REGISTER_T
#define NIM_REGISTER_LINE                 REGISTER_X // MUST be REGISTER_X
#define ERR_REGISTER_LINE                 REGISTER_Z
#define TRUE_FALSE_REGISTER_LINE          REGISTER_Z

// If one of the 4 next defines is changed: change also ???_REGISTER_LINE above
#define Y_POSITION_OF_AIM_LINE        Y_POSITION_OF_REGISTER_X_LINE
#define Y_POSITION_OF_TAM_LINE        Y_POSITION_OF_REGISTER_T_LINE
#define Y_POSITION_OF_NIM_LINE        Y_POSITION_OF_REGISTER_X_LINE
#define Y_POSITION_OF_ERR_LINE        Y_POSITION_OF_REGISTER_Z_LINE
#define Y_POSITION_OF_TRUE_FALSE_LINE Y_POSITION_OF_REGISTER_Z_LINE



#define SCREEN_WIDTH                             400 // Width of the screen
#define SCREEN_HEIGHT                            240 // Height of the screen
#define ON_PIXEL                            0x303030 // blue red green
#define OFF_PIXEL                           0xe0e0e0 // blue red green
#define SOFTMENU_STACK_SIZE                        8
#define TEMPORARY_INFO_OFFSET                      6 // Vertical offset for temporary informations. I find 4 looks better
#define REGISTER_LINE_HEIGHT                      36

#define Y_POSITION_OF_REGISTER_T_LINE             24 // 135 - REGISTER_LINE_HEIGHT*(registerNumber - REGISTER_X)
#define Y_POSITION_OF_REGISTER_Z_LINE             60
#define Y_POSITION_OF_REGISTER_Y_LINE             96
#define Y_POSITION_OF_REGISTER_X_LINE            132

#define NUMBER_OF_DYNAMIC_SOFTMENUS               22
#define SOFTMENU_HEIGHT                           23


// Status bar updating mode
#define SBARUPD_Date                            (getSystemFlag(FLAG_SBdate ))
#define SBARUPD_Time                            (getSystemFlag(FLAG_SBtime ))
#define SBARUPD_WoY                             (getSystemFlag(FLAG_SBwoy  ))
#define SBARUPD_ComplexResult                   (getSystemFlag(FLAG_SBcr   ))
#define SBARUPD_ComplexMode                     (getSystemFlag(FLAG_SBcpx  ))
#define SBARUPD_AngularModeBasic                (getSystemFlag(FLAG_SBang  ))
#define SBARUPD_AngularMode                     ( 1                         )
#define SBARUPD_FractionModeAndBaseMode         (getSystemFlag(FLAG_SBfrac ))
#define SBARUPD_IntegerMode                     (getSystemFlag(FLAG_SBint  ))
#define SBARUPD_MatrixMode                      (getSystemFlag(FLAG_SBmx   ))
#define SBARUPD_TVMMode                         (getSystemFlag(FLAG_SBtvm  ))
#define SBARUPD_OCCarryMode                     (getSystemFlag(FLAG_SBoc   ))
#define SBARUPD_AlphaMode                       ( 1                         )
#define SBARUPD_HourGlass                       ( 1                         )
#define SBARUPD_StackSize                       (getSystemFlag(FLAG_SBss   ))
#define SBARUPD_StopWatch                       (getSystemFlag(FLAG_SBstpw ))
#define SBARUPD_SerialIO                        (getSystemFlag(FLAG_SBser  ))
#define SBARUPD_Printer                         (getSystemFlag(FLAG_SBprn  ))
#define SBARUPD_UserMode                        ( 1                         )
#define SBARUPD_Battery                         ( 1                         )
#define SBARUPD_BatVoltage                      (getSystemFlag(FLAG_SBbatV ))
#define SBAR_SHIFT                              (getSystemFlag(FLAG_SBshfR ))




// Horizontal offsets in the status bar
#define X_DATE                           ((SBARUPD_Time || SBARUPD_WoY) ? 1 : 25)
#define X_TIME                                     45 // note: this is used only if DATE is not displayed, otherwise TIME is printed directly next to date's end
#define X_REAL_COMPLEX        (X_TIME             +91)// note: this is for both dow or time, not both
#define X_HOURGLASS_GRAPHS    (X_REAL_COMPLEX     + 4)//
#define X_COMPLEX_MODE        (X_HOURGLASS_GRAPHS + 6)//
#define X_COMPLEX_MODE_ADJ               -8           // note: auto moved left if REAL_COMPLEX is not present
#define X_ANGULAR_MODE        (X_COMPLEX_MODE     +14)//
#define X_FRAC_MODE           (X_ANGULAR_MODE     +27)//
#define X_BASE_MODE           (X_FRAC_MODE        + 0)//
#define X_INT_MX_TVM_MODE     (X_BASE_MODE        +77)//
#define X_OVERFLOW_CARRY      (X_INT_MX_TVM_MODE  +30)//
#define X_ALPHA_MODE          (X_OVERFLOW_CARRY   +10)//
#define X_HOURGLASS           (X_ALPHA_MODE       +11)//
#define X_SSIZE_BEGIN         (X_HOURGLASS        +14)//
#define X_ASM                 (X_SSIZE_BEGIN      +11)//
#define X_STOPWATCH           (X_ASM              + 4)//
#define X_SERIAL_IO           (X_STOPWATCH        +18)// note: I/O and Printing (soft or hard) cannot happen at the same time
#define X_PRINTER             (X_SERIAL_IO        + 0)//
#define X_USER_MODE           (X_PRINTER          +14)//
#define X_BATTERY             (X_USER_MODE        +13)//
#define DX_BATTERY                                  8  // <=2.054 V - minimum bar (one fine line)
#define DY_BATTERY                                 20  // >=3.045 V - maximum bars (tip of battery against the edge)
                                                       // f/g icon either in T-line left; or if date or time is removed, it moves up top left; or if SBAR_SHIFT is active, it goes top right, next to U
#define X_SHIFT_L                                   0
#define X_SHIFT_R                                (X_PRINTER - 1)
#define X_SHIFT                                  (getSystemFlag(FLAG_SBshfR) ? X_SHIFT_R : X_SHIFT_L)
#define Y_SHIFT_LO                               (Y_POSITION_OF_REGISTER_T_LINE)
#define Y_SHIFT                                  (((!SBARUPD_Date || !(SBARUPD_Time || SBARUPD_WoY)) && !SBAR_SHIFT) ? 0 : (SBAR_SHIFT ? 0 : Y_SHIFT_LO ))



#define TIMER_IDX_REFRESH_SLEEP                    0 // use timer 0 to wake up for screen refresh
//#define TIMER_IDX_AUTO_REPEAT                    1 // use timer 1 to wake up for key auto-repeat

#define TMR_NUMBER                                11

// timer
#define TO_FG_LONG                                 0
#define TO_CL_LONG                                 1
#define TO_FG_TIMR                                 2
#define TO_FN_LONG                                 3
#define TO_FN_EXEC                                 4
#define TO_3S_CTFF                                 5
#define TO_CL_DROP                                 6
#define TO_AUTO_REPEAT                             7
#define TO_TIMER_APP                               8
#define TO_ASM_ACTIVE                              9
#define TO_KB_ACTV                                 10



#if defined(PC_BUILD)
  #if defined(LINUX)
    #define LINEBREAK                           "\n"
  #elif defined(WIN32)
    #define LINEBREAK                         "\n\r"
  #elif defined(OSX)
    #define LINEBREAK                         "\r\n"
  #else // Unsupported OS
    #error Only Linux, MacOS, and Windows MINGW64 are supported for now
  #endif // OS
#else // !PC_BUILD
  #define LINEBREAK                           "\n\r"                       //JM
#endif // PC_BUILD

#define NUMBER_OF_DISPLAY_REAL_CONTEXT_DIGITS     ((displayFormat == DF_ALL || getSystemFlag(FLAG_2TO10)) ? NUMBER_OF_DISPLAY_DIGITS + 1 : displayFormatDigits + 2) //used for time consuming functions, divides, etc.
#define NUMBER_OF_DISPLAY_DIGITS                  20
#define NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS     10

#if defined(DMCP_BUILD) && defined(OLD_HW) // The old HW has about 64Kb for user usable RAM
  #define MAX_FREE_REGIONS                        50 // Maximum number of free memory regions
#else // The new HW has about 512 KB for user usable RAM, 8 times more.
  #define MAX_FREE_REGIONS                       200 // Maximum number of free memory regions
#endif // DMCP_BUILD && OLD_HW

#if !defined(DMCP_BUILD)
  #define MAX_ALLOCATED_REGIONS                 5000 // Maximum number of allocated memory regions
#endif // !DMCP_BUILD

// On/Off 1 bit
#define OFF                                        0
#define ON                                         1

// Short integer mode 2 bits
#define SIM_UNSIGN                                 0
#define SIM_1COMPL                                 1
#define SIM_2COMPL                                 2
#define SIM_SIGNMT                                 3

// Display format 2 bits
#define DF_ALL                                     0
#define DF_FIX                                     1
#define DF_SCI                                     2
#define DF_ENG                                     3
#define DF_SF                                      4   //JM
#define DF_UN                                      5   //JM

// Date format 2 bits
#define DF_DMY                                     0
#define DF_YMD                                     1
#define DF_MDY                                     2

// Curve fitting 10 bits
#define CF_LINEAR_FITTING                          1
#define CF_EXPONENTIAL_FITTING                     2
#define CF_LOGARITHMIC_FITTING                     4
#define CF_POWER_FITTING                           8
#define CF_ROOT_FITTING                           16
#define CF_HYPERBOLIC_FITTING                     32
#define CF_PARABOLIC_FITTING                      64
#define CF_CAUCHY_FITTING                        128
#define CF_GAUSS_FITTING                         256
#define CF_ORTHOGONAL_FITTING                    512
#define orOrtho(a)                                 (a==0 ? CF_ORTHOGONAL_FITTING : a)

  // Plot curve fitting 4 bits
#define PLOT_ORTHOF                                0
#define PLOT_NXT                                   1
#define PLOT_REV                                   2
#define PLOT_LR                                    3
#define PLOT_START                                 4
#define PLOT_NOTHING                               5
#define PLOT_GRAPH                                 6
#define H_PLOT                                     7
#define H_NORM                                     8

// Rounding mode 3 bits
#define RM_HALF_EVEN                               0
#define RM_HALF_UP                                 1
#define RM_HALF_DOWN                               2
#define RM_UP                                      3
#define RM_DOWN                                    4
#define RM_CEIL                                    5
#define RM_FLOOR                                   6

// Calc mode 5 bits
#define CM_NORMAL                                  0 // Normal operation
#define CM_AIM                                     1 // Alpha input mode
#define CM_NIM                                     2 // Numeric input mode
#define CM_PEM                                     3 // Program entry mode
#define CM_ASSIGN                                  4 // Assign mode
#define CM_REGISTER_BROWSER                        5 // Register browser
#define CM_FLAG_BROWSER                            6 // Flag browser
#define CM_FONT_BROWSER                            7 // Font browser
#define CM_PLOT_STAT                               8 // Plot stats mode
#define CM_ERROR_MESSAGE                           9 // Error message in one of the register lines
#define CM_BUG_ON_SCREEN                          10 // Bug message on screen
#define CM_CONFIRMATION                           11 // Waiting for confirmation or canceling
#define CM_MIM                                    12 // Matrix input mode tbd reorder
#define CM_EIM                                    13 // Equation input mode
#define CM_TIMER                                  14 // Timer application
#define CM_GRAPH                                  15 // Plot graph mode
#define CM_NO_UNDO                                16 // Running functions without undo affected
#define CM_ASN_BROWSER                            17 // Assignments browser   //JM
#define CM_LISTXY                                 18 // Display stat list   //JM

// Next character in AIM 2 bits
#define NC_NORMAL                                  0
#define NC_SUBSCRIPT                               1
#define NC_SUPERSCRIPT                             2

// Alpha case 1 bit
#define AC_UPPER                                   0
#define AC_LOWER                                   1
#define plainTextMode                              (bool_t)( calcMode == CM_AIM   || ((calcMode == CM_PEM  || calcMode == CM_ASSIGN) && getSystemFlag(FLAG_ALPHA)))
#define labelText                                  (bool_t)((tam.mode == TM_MENU || tam.mode == TM_LABEL || tam.mode == TM_LBLONLY || tam.mode == TM_STORCL || tam.alpha) && getSystemFlag(FLAG_ALPHA))
//#define plainText                                  (bool_t)( calcMode == CM_AIM   || calcMode == CM_EIM    || (calcMode == CM_PEM    && getSystemFlag(FLAG_ALPHA) && !tam.mode))
#define noCapsLockSync                             0
#define onlyCapsLockSync                           1
#define allKeysCapsLockSync                        2
#define CAPS_EQN_DEFAULT                           AC_LOWER
#define CAPS_AIM_DEFAULT                           AC_UPPER
#define CAPS_ASMcat_DEFAULT                        AC_UPPER
#define CAPS_STOetc_DEFAULT                        AC_LOWER
#define CAPS_TAMother_DEFAULT                      AC_UPPER

// TAM mode
#define TM_VALUE                               10001 // TM_VALUE must be the 1st in this list
#define TM_VALUE_CHB                           10002 // same as TM_VALUE but for ->INT (#) change base
#define TM_REGISTER                            10003
#define TM_FLAGR                               10004
#define TM_FLAGW                               10005
#define TM_STORCL                              10006
#define TM_M_DIM                               10007
#define TM_SHUFFLE                             10008
#define TM_LABEL                               10009
#define TM_SOLVE                               10010
#define TM_NEWMENU                             10011
#define TM_KEY                                 10012
#define TM_INTEGRATE                           10013
#define TM_DELITM                              10014
#define TM_VALUE_MAX                           10015
#define TM_VALUE_TRK                           10016
#define TM_MENU                                10017
#define TM_LBLONLY                             10018
#define TM_VARONLY                             10019
#define TM_CMP                                 10020 // TM_CMP must be the last in this list

// gamma function type
#define GAMMA_XYLOWER                              0
#define GAMMA_P                                    1
#define GAMMA_XYUPPER                              2
#define GAMMA_Q                                    3


// NIM number part
#define NP_EMPTY                                   0
#define NP_INT_10                                  1 // Integer base 10
#define NP_INT_16                                  2 // Integer base > 10
#define NP_INT_BASE                                3 // Integer: the base
#define NP_REAL_FLOAT_PART                         4 // Decimal part of the real
#define NP_REAL_EXPONENT                           5 // Ten exponent of the real
#define NP_FRACTION_DENOMINATOR                    6 // Denominator of the fraction
#define NP_COMPLEX_INT_PART                        7 // Integer part of the complex imaginary part
#define NP_COMPLEX_FLOAT_PART                      8 // Decimal part of the complex imaginary part
#define NP_COMPLEX_EXPONENT                        9 // Ten exponent of the complex imaginary part
#define NP_HP32SII_DENOMINATOR                    10 // Denominator of the fraction (HP32SII mode)
#define NP_COMPLEX_FRACTION_DENOMINATOR           11 // Denominator of the complex imaginary part fraction
#define NP_COMPLEX_HP32SII_DENOMINATOR            12 // Denominator of the complex imaginary part fraction (HP32SII mode)

// Temporary information
#define TI_NO_INFO                                 0
#define TI_RADIUS_THETA                            1
#define TI_RADIUS_THETA_SWAPPED                    2
#define TI_THETA_RADIUS                            3
#define TI_X_Y                                     4
#define TI_X_Y_SWAPPED                             5
#define TI_RE_IM                                   6
#define TI_STATISTIC_SUMS                          7
#define TI_RESET                                   8
#define TI_ARE_YOU_SURE                            9
#define TI_VERSION                                10
#define TI_WHO                                    11
#define TI_FALSE                                  12
#define TI_TRUE                                   13 // MUST be (TI_FALSE + 1)
#define TI_SHOW_REGISTER                          14
#define TI_VIEW_REGISTER                          15
#define TI_SUMX_SUMY                              16
#define TI_MEANX_MEANY                            17
#define TI_MEANX                                  18
#define TI_GEOMMEANX_GEOMMEANY                    19
#define TI_WEIGHTEDMEANX                          20
#define TI_HARMMEANX_HARMMEANY                    21
#define TI_RMSMEANX_RMSMEANY                      22
#define TI_WEIGHTEDSAMPLSTDDEV                    23
#define TI_WEIGHTEDPOPLSTDDEV                     24
#define TI_WEIGHTEDSTDERR                         25
#define TI_SAMPLSTDDEV                            26
#define TI_POPLSTDDEV                             27
#define TI_STDERR                                 28
#define TI_GEOMSAMPLSTDDEV                        29
#define TI_GEOMPOPLSTDDEV                         30
#define TI_GEOMSTDERR                             31
#define TI_SAVED                                  32
#define TI_BACKUP_RESTORED                        33
#define TI_XMIN_YMIN                              34
#define TI_XMAX_YMAX                              35
#define TI_DAY_OF_WEEK                            36
#define TI_SXY                                    37
#define TI_COV                                    38
#define TI_CORR                                   39
#define TI_SMI                                    40
#define TI_LR                                     41
#define TI_CALCX                                  42
#define TI_CALCY                                  43
#define TI_CALCX2                                 44
#define TI_STATISTIC_LR                           45
#define TI_STATISTIC_HISTO                        46
#define TI_SA                                     47
#define TI_INACCURATE                             48
#define TI_UNDO_DISABLED                          49
//#define TI_VIEW                                 50
#define TI_SOLVER_VARIABLE                        51
#define TI_SOLVER_FAILED                          52
#define TI_ACC                                    53
#define TI_ULIM                                   54
#define TI_LLIM                                   55
#define TI_INTEGRAL                               56
#define TI_1ST_DERIVATIVE                         57
#define TI_2ND_DERIVATIVE                         58
#define TI_KEYS                                   59
#define TI_MEDIANX_MEDIANY                        60
#define TI_Q1X_Q1Y                                61
#define TI_Q3X_Q3Y                                62
#define TI_MADX_MADY                              63
#define TI_IQRX_IQRY                              64
#define TI_RANGEX_RANGEY                          65
#define TI_PCTILEX_PCTILEY                        66
#define TI_CONV_MENU_STR                          67
#define TI_PERC                                   68
#define TI_PERCD                                  69
#define TI_PERCD2                                 70
#define TI_STATEFILE_RESTORED                     71
#define TI_ABC                                    72  //JM EE
#define TI_ABBCCA                                 73  //JM EE
#define TI_012                                    74  //JM EE
#define TI_SHOW_REGISTER_BIG                      75  //JM_SHOW
#define TI_SHOW_REGISTER_SMALL                    76
#define TI_SHOW_REGISTER_TINY                     77
#define TI_BATTV                                  78
#define TI_FROM_DMS                               79
#define TI_FROM_MS_TIME                           80
#define TI_FROM_MS_DEG                            81
#define TI_FROM_HMS                               82
#define TI_DISP_JULIAN                            83
#define TI_FROM_DATEX                             84
#define TI_LAST_CONST_CATNAME                     85
#define TI_PROGRAM_LOADED                         86  //DL
#define TI_PROGRAMS_RESTORED                      87  //DL
#define TI_REGISTERS_RESTORED                     88  //DL
#define TI_SETTINGS_RESTORED                      89  //DL
#define TI_SUMS_RESTORED                          90  //DL
#define TI_VARIABLES_RESTORED                     91  //DL
#define TI_SCATTER_SMI                            92
#define TI_SHOWNOTHING                            93
#define TI_COPY_FROM_SHOW                         94
#define TI_DATA_LOSS                              95
#define TI_CLEAR_ALL_FLAGS                        96
#define TI_CLEAR_ALL_MENUS                        97  //DL
#define TI_CLEAR_ALL_VARIABLES                    98  //DL
#define TI_DEL_ALL_PRGMS                          99
#define TI_DEL_ALL_MENUS                         100  //DL
#define TI_DEL_ALL_VARIABLES                     101  //DL
#define TI_ROOTS2                                102
#define TI_ROOTS3                                103
#define TI_IJ                                    104
#define TI_I                                     105
#define TI_J                                     106
#define TI_MIJ                                   107
#define TI_BYTES                                 108
#define TI_BITS                                  109
#define TI_SOLVER_VARIABLE_RESULT                110
#define TI_DATA_NEG_OVRFL                        111
#define TI_LASTSTATEFILE                         112
#define TI_FUNCTION                              113
#define TI_STORCL                                114
#define TI_TVM_EFF                               115
#define TI_TVM_IA                                116
#define TI_NOT_AVAILABLE                         117
#define TI_DISP_WOY                              118
#define TI_DISP_JULIAN_WOY                       119
#define TI_WOY                                   120
#define TI_WOY_RULE                              121
#define TI_MIJEQ                                 122
#define TI_REGTYPE                               123
#define TI_LR_A0                                 124
#define TI_LR_A1                                 125
#define TI_LR_A2                                 126

#define SET_TI_TRUE_FALSE(condition)               do { temporaryInformation = TI_FALSE + (condition); } while(0) // TI_TRUE must be TI_FALSE + 1

// Register browser mode
#define RBR_GLOBAL                                 0 // Global registers are browsed
#define RBR_LOCAL                                  1 // Local registers are browsed
#define RBR_NAMED                                  2 // Named variables are browsed

// Debug window
#define DBG_BIT_FIELDS                             0
#define DBG_FLAGS                                  1
#define DBG_REGISTERS                              2
#define DBG_LOCAL_REGISTERS                        3
#define DBG_STATISTICAL_SUMS                       4
#define DBG_NAMED_VARIABLES                        5
#define DBG_TMP_SAVED_STACK_REGISTERS              6

// alpha selection menus
#define CATALOG_NONE                               0 // CATALOG_NONE must be 0
#define CATALOG_CNST                               1
#define CATALOG_FCNS                               2
#define CATALOG_MENU                               3
#define CATALOG_SYFL                               4
#define CATALOG_AINT                               5
#define CATALOG_aint                               6
#define CATALOG_PROG                               7
#define CATALOG_VAR                                8
#define CATALOG_MATRS                              9
#define CATALOG_STRINGS                           10
#define CATALOG_DATES                             11
#define CATALOG_TIMES                             12
#define CATALOG_ANGLES                            13
#define CATALOG_SINTS                             14
#define CATALOG_LINTS                             15
#define CATALOG_REALS                             16
#define CATALOG_CPXS                              17
#define CATALOG_MVAR                              18
#define CATALOG_CONFIGS                           19
#define CATALOG_ALLVARS                           20
#define CATALOG_NUMBRS                            21
#define CATALOG_FCNS_EIM                          22
#define NUMBER_OF_CATALOGS                        23

// String comparison type
#define CMP_BINARY                                 0
#define CMP_CLEANED_STRING_ONLY                    1
#define CMP_EXTENSIVE                              2
#define CMP_NAME                                   3

// Indirect parameter mode
#define INDPM_PARAM                                0
#define INDPM_REGISTER                             1
#define INDPM_FLAG                                 2
#define INDPM_LABEL                                3
#define INDPM_MENU                                 4

// Combination / permutation
#define CP_PERMUTATION                             0
#define CP_COMBINATION                             1

// Gudermannian
#define GD_DIRECT_FUNCTION                         0
#define GD_INVERSE_FUNCTION                        1

// Program running mode
#define PGM_STOPPED                                0
#define PGM_RUNNING                                1
#define PGM_WAITING                                2
#define PGM_PAUSED                                 3
#define PGM_KEY_PRESSED_WHILE_PAUSED               4
#define PGM_RESUMING                               5
#define PGM_SINGLE_STEP                            6
#define PGM_UNDEFINED                            255

// Save mode
#define SM_MANUAL_SAVE                             0
#define SM_STATE_SAVE                              1

// Load mode
#define LM_ALL                                     0
#define LM_PROGRAMS                                1
#define LM_REGISTERS                               2
#define LM_NAMED_VARIABLES                         3
#define LM_SUMS                                    4
#define LM_SYSTEM_STATE                            5
#define LM_REGISTERS_PARTIAL                       6
#define LM_STATE_LOAD                              7

// Screen updating mode
#define SCRUPD_AUTO                             0x00
#define SCRUPD_MANUAL_STATUSBAR                 0x01       //0000 0001
#define SCRUPD_MANUAL_STACK                     0x02       //0000 0010
#define SCRUPD_MANUAL_MENU                      0x04       //0000 0100
#define SCRUPD_MANUAL_SHIFT_STATUS              0x08       //0000 1000
#define SCRUPD_SKIP_STATUSBAR_ONE_TIME          0x10       //0001 0000   16d
#define SCRUPD_SKIP_STACK_ONE_TIME              0x20       //0010 0000   32d
#define SCRUPD_SKIP_MENU_ONE_TIME               0x40       //0100 0000   64d
//#define SCRUPD_SHIFT_STATUS                     0x80
#define SCRUPD_ONE_TIME_FLAGS                   0xf0

// Statistical sums TODO: optimize size of SIGMA_N, _X, _Y, _XMIN, _XMAX, _YMIN, and _YMAX. Thus, saving 2×(7×60 - 4 - 6×16) = 640 bytes
#define SUM_X                                      1
#define SUM_Y                                      2
#define SUM_X2                                     3
#define SUM_X2Y                                    4
#define SUM_Y2                                     5
#define SUM_XY                                     6
#define SUM_lnXlnY                                 7
#define SUM_lnX                                    8
#define SUM_ln2X                                   9
#define SUM_YlnX                                  10
#define SUM_lnY                                   11
#define SUM_ln2Y                                  12
#define SUM_XlnY                                  13
#define SUM_X2lnY                                 14
#define SUM_lnYonX                                15
#define SUM_X2onY                                 16
#define SUM_1onX                                  17
#define SUM_1onX2                                 18
#define SUM_XonY                                  19
#define SUM_1onY                                  20
#define SUM_1onY2                                 21
#define SUM_X3                                    22
#define SUM_X4                                    23
#define SUM_XMIN                                  24
#define SUM_XMAX                                  25
#define SUM_YMIN                                  26
#define SUM_YMAX                                  27

#define NUMBER_OF_STATISTICAL_SUMS                28
#define SIGMA_N      (statisticalSumsPointer             ) // could be a 32 bit unsigned integer. No, this must be old. SIGMA_N is a Real.
#define SIGMA_X      (statisticalSumsPointer + SUM_X     ) // could be a real34. No, this must be old. SIGMA_** is a Real.
#define SIGMA_Y      (statisticalSumsPointer + SUM_Y     ) // could be a real34. No, this must be old. SIGMA_** is a Real.
#define SIGMA_X2     (statisticalSumsPointer + SUM_X2    )
#define SIGMA_X2Y    (statisticalSumsPointer + SUM_X2Y   )
#define SIGMA_Y2     (statisticalSumsPointer + SUM_Y2    )
#define SIGMA_XY     (statisticalSumsPointer + SUM_XY    )
#define SIGMA_lnXlnY (statisticalSumsPointer + SUM_lnXlnY)
#define SIGMA_lnX    (statisticalSumsPointer + SUM_lnX   )
#define SIGMA_ln2X   (statisticalSumsPointer + SUM_ln2X  )
#define SIGMA_YlnX   (statisticalSumsPointer + SUM_YlnX  )
#define SIGMA_lnY    (statisticalSumsPointer + SUM_lnY   )
#define SIGMA_ln2Y   (statisticalSumsPointer + SUM_ln2Y  )
#define SIGMA_XlnY   (statisticalSumsPointer + SUM_XlnY  )
#define SIGMA_X2lnY  (statisticalSumsPointer + SUM_X2lnY )
#define SIGMA_lnYonX (statisticalSumsPointer + SUM_lnYonX)
#define SIGMA_X2onY  (statisticalSumsPointer + SUM_X2onY )
#define SIGMA_1onX   (statisticalSumsPointer + SUM_1onX  )
#define SIGMA_1onX2  (statisticalSumsPointer + SUM_1onX2 )
#define SIGMA_XonY   (statisticalSumsPointer + SUM_XonY  )
#define SIGMA_1onY   (statisticalSumsPointer + SUM_1onY  )
#define SIGMA_1onY2  (statisticalSumsPointer + SUM_1onY2 )
#define SIGMA_X3     (statisticalSumsPointer + SUM_X3    )
#define SIGMA_X4     (statisticalSumsPointer + SUM_X4    )
#define SIGMA_XMIN   (statisticalSumsPointer + SUM_XMIN  ) // could be a real34. No, this must be old. SIGMA_** is a Real.
#define SIGMA_XMAX   (statisticalSumsPointer + SUM_XMAX  ) // could be a real34. No, this must be old. SIGMA_** is a Real.
#define SIGMA_YMIN   (statisticalSumsPointer + SUM_YMIN  ) // could be a real34. No, this must be old. SIGMA_** is a Real.
#define SIGMA_YMAX   (statisticalSumsPointer + SUM_YMAX  ) // could be a real34. No, this must be old. SIGMA_** is a Real.

#define MAX_NUMBER_OF_GLYPHS_IN_STRING           508 //WP=196: Change to 512 less 3, Also change error message 33, and AIM_BUFFER_LENGTH, and MAXLINES
#define NUMBER_OF_GLYPH_ROWS                     239 //Used in the font browser application

#define YY_OFF                                     2 // 2 is off and gets transferred to bit 15 (32768 + YY)
#define YY_TRACKING                                1 // 1 gets transferred to bit 14 (16384 + YY)
#define MAX_DENMAX                              9999 // Biggest denominator in fraction display mode selector, and annunciator.
                                                     // The value 0 gets converted to MAX_INTERNAL_DENMAX
#define MAX_INTERNAL_DENMAX                    32500 // Biggest denominator in fraction display mode

#if defined(DMCP_BUILD)
  #define SCREEN_REFRESH_PERIOD                  160 // 500 // in milliseconds //JM timeout for lcd refresh in ms 125
  #define FAST_SCREEN_REFRESH_PERIOD             100 // in milliseconds
#else // !DMCP_BUILD
  #define SCREEN_REFRESH_PERIOD                  100 // 500 // in milliseconds //JM timeout for lcd refresh in ms 100
  #define FAST_SCREEN_REFRESH_PERIOD             100 // in milliseconds
#endif // DMCP_BUILD
#define KEY_AUTOREPEAT_FIRST_PERIOD              400 // in milliseconds
#define KEY_AUTOREPEAT_PERIOD                    200 // in milliseconds
#define TIMER_APP_PERIOD                         100 // in milliseconds

#define RAM_SIZE_IN_BLOCKS_OLD_HW              (16384) // MUST be < 2^16 - 1   (65535 = 0xffff excluded because it's the value of the C47_NULL pointer)
#define RAM_SIZE_IN_BLOCKS_NEW_HW              (65534) // MUST be < 2^16 - 1   (65535 = 0xffff excluded because it's the value of the C47_NULL pointer)
#if defined(DMCP_BUILD) && defined(NEW_HW) // DMCP5
  #define RAM_SIZE_IN_BLOCKS                   RAM_SIZE_IN_BLOCKS_NEW_HW
#elif defined(DMCP_BUILD) && !defined(NEW_HW) // DMCP
  #define RAM_SIZE_IN_BLOCKS                   RAM_SIZE_IN_BLOCKS_OLD_HW
#else // !DMCP_BUILD
  #define RAM_SIZE_IN_BLOCKS                   RAM_SIZE_IN_BLOCKS_NEW_HW
#endif // DMCP_BUILD

#define CONFIG_SIZE_IN_BLOCKS                  TO_BLOCKS(sizeof(dtConfigDescriptor_t))
#define CONFIG_SIZE_IN_BYTES                   TO_BYTES(CONFIG_SIZE_IN_BLOCKS)

#define FLASH_PGM_PAGE_SIZE                      512
#define FLASH_PGM_NUMBER_OF_PAGES                 64

// Type of constant stored in a program
#define BINARY_SHORT_INTEGER                       1
#define BINARY_LONG_INTEGER                        2
#define BINARY_REAL34                              3
#define BINARY_COMPLEX34                           4
#define BINARY_DATE                                5
#define BINARY_TIME                                6
#define STRING_SHORT_INTEGER                       7
#define STRING_LONG_INTEGER                        8
#define STRING_REAL34                              9
#define STRING_COMPLEX34                          10
#define STRING_TIME                               11
#define STRING_DATE                               12
//#define BINARY_ANGLE_RADIAN                       13
//#define BINARY_ANGLE_GRAD                         14
//#define BINARY_ANGLE_DEGREE                       15
//#define BINARY_ANGLE_DMS                          16
//#define BINARY_ANGLE_MULTPI                       17
#define STRING_ANGLE_RADIAN                       18
#define STRING_ANGLE_GRAD                         19
#define STRING_ANGLE_DEGREE                       20
#define STRING_ANGLE_DMS                          21
#define STRING_ANGLE_MULTPI                       22

// OP parameter type
#define PARAM_DECLARE_LABEL                        1
#define PARAM_LABEL                                2
#define PARAM_REGISTER                             3
#define PARAM_FLAG                                 4
#define PARAM_NUMBER_8                             5
#define PARAM_NUMBER_16                            6
#define PARAM_COMPARE                              7
#define PARAM_KEYG_KEYX                            8
#define PARAM_SKIP_BACK                            9
#define PARAM_NUMBER_8_16                         10
#define PARAM_SHUFFLE                             11
#define PARAM_MENU                                12

#define CHECK_INTEGER                              0
#define CHECK_INTEGER_EVEN                         1
#define CHECK_INTEGER_ODD                          2
#define CHECK_INTEGER_FP                           3

#define OPMOD_MULTIPLY                             0
#define OPMOD_POWER                                1

#define ORTHOPOLY_HERMITE_H                        0
#define ORTHOPOLY_HERMITE_HE                       1
#define ORTHOPOLY_LAGUERRE_L                       2
#define ORTHOPOLY_LAGUERRE_L_ALPHA                 3
#define ORTHOPOLY_LEGENDRE_P                       4
#define ORTHOPOLY_CHEBYSHEV_T                      5
#define ORTHOPOLY_CHEBYSHEV_U                      6

#define QF_NEWTON_F                                0
#define QF_NEWTON_POISSON                          1
#define QF_NEWTON_BINOMIAL                         2
#define QF_NEWTON_GEOMETRIC                        3
#define QF_NEWTON_NEGBINOM                         4
#define QF_NEWTON_HYPERGEOMETRIC                   5

#define QF_DISCRETE_CDF_POISSON                    0
#define QF_DISCRETE_CDF_BINOMIAL                   1
#define QF_DISCRETE_CDF_GEOMETRIC                  2
#define QF_DISCRETE_CDF_NEGBINOM                   3
#define QF_DISCRETE_CDF_HYPERGEOMETRIC             4

#define SOLVER_STATUS_READY_TO_EXECUTE             0x0001 // 0000 0000 0000 --01
#define SOLVER_STATUS_INTERACTIVE                  0x0002 // 0000 0000 0000 --10

#define SOLVER_STATUS_EQUATION_MODE                0x200c // --1- ---- ---- 1100
#define SOLVER_STATUS_EQUATION_SOLVER              0x0000 // --0- ---- ---- 00--
#define SOLVER_STATUS_EQUATION_INTEGRATE           0x0004 // --0- ---- ---- 01--
#define SOLVER_STATUS_EQUATION_1ST_DERIVATIVE      0x0008 // --0- ---- ---- 10--
#define SOLVER_STATUS_EQUATION_2ND_DERIVATIVE      0x000C // --0- ---- ---- 11--
#define SOLVER_STATUS_EQUATION_GRAPHER             0x2000 // --1- ---- ---- 00--

#define SOLVER_STATUS_SINGLE_VARIABLE              0x0010 // 00-0 --00 ---1 ----
#define SOLVER_STATUS_USES_FORMULA                 0x0100 // 00-0 --01 ---0 ----
#define SOLVER_STATUS_MVAR_BEING_OPENED            0x0200 // 00-0 --10 ---0 ----
#define SOLVER_STATUS_TVM_APPLICATION              0x1000 // 00-1 ---- ---0 ----

#define IS_EQN_INTEGRATE (((currentSolverStatus & SOLVER_STATUS_EQUATION_MODE) == SOLVER_STATUS_EQUATION_INTEGRATE) && \
                            currentSolverStatus & SOLVER_STATUS_INTERACTIVE)
#define IS_EQN_2NDDER     ((currentSolverStatus & SOLVER_STATUS_USES_FORMULA) && \
                           (currentSolverStatus & SOLVER_STATUS_INTERACTIVE) &&  \
                          ((currentSolverStatus & SOLVER_STATUS_EQUATION_MODE) == SOLVER_STATUS_EQUATION_2ND_DERIVATIVE))
#define IS_EQN_1STDER     ((currentSolverStatus & SOLVER_STATUS_USES_FORMULA) && \
                           (currentSolverStatus & SOLVER_STATUS_INTERACTIVE) && \
                          ((currentSolverStatus & SOLVER_STATUS_EQUATION_MODE) == SOLVER_STATUS_EQUATION_1ST_DERIVATIVE))

#define SOLVER_RESULT_NORMAL                       0
#define SOLVER_RESULT_SIGN_REVERSAL                1
#define SOLVER_RESULT_EXTREMUM                     2
#define SOLVER_RESULT_BAD_GUESS                    3
#define SOLVER_RESULT_CONSTANT                     4
#define SOLVER_RESULT_OTHER_FAILURE                5
#define SOLVER_RESULT_ABORTED                      6
#define SOLVER_RESULT_CONJUGATES                 200

#define ASSIGN_NAMED_VARIABLES                 10000
#define ASSIGN_LABELS                          12000
#define ASSIGN_RESERVED_VARIABLES                  (ASSIGN_NAMED_VARIABLES + FIRST_RESERVED_VARIABLE - FIRST_NAMED_VARIABLE)
#define ASSIGN_USER_MENU                     (-10000)
#define ASSIGN_CLEAR                         (-32768)

#define TIMER_APP_STOPPED                          0xFFFFFFFFu

#if !defined(DMCP_BUILD)
  #define TO_QSPI
#else // DMCP_BUILD
  #define beep(frequence, length)            do { while(get_beep_volume() < 11) beep_volume_up(); start_buzzer_freq(frequence * 1000); sys_delay(length); stop_buzzer(); } while(0)
  #undef TO_QSPI
  #if defined(TWO_FILE_PGM)
    #define TO_QSPI                          __attribute__ ((section(".qspi")))
  #else // !TWO_FILE_PGM
    #define TO_QSPI
  #endif // TWO_FILE_PGM
#endif // !DMCP_BUILD

#if defined(DMCP_BUILD) && defined(NEW_HW) // DMCP5
  #undef TO_QSPI
  #define TO_QSPI
#endif // DMCP_BUILD && NEW_HW

//******************************
//* Macros replacing functions *
//******************************
#if (EXTRA_INFO_ON_CALC_ERROR == 0) || defined(TESTSUITE_BUILD) || defined(DMCP_BUILD)
  #define EXTRA_INFO_MESSAGE(function, msg)
#else // EXTRA_INFO_ON_CALC_ERROR != 0 && !TESTSUITE_BUILD && !DMCP_BUILD
  #define EXTRA_INFO_MESSAGE(function, msg)  do { sprintf(errorMessage, msg); moreInfoOnError("In function ", function, errorMessage, NULL); } while(0)
#endif // EXTRA_INFO_ON_CALC_ERROR == 0 || TESTSUITE_BUILD || DMCP_BUILD

#define isSystemFlagWriteProtected(sf)       ((sf & 0x4000) != 0)
#define shortIntegerIsZero(op)               (((*(uint64_t *)(op)) == 0) || (shortIntegerMode == SIM_SIGNMT && (((*(uint64_t *)(op)) == 1u<<((uint64_t)shortIntegerWordSize-1)))))
#define getStackTop()                        (getSystemFlag(FLAG_SSIZE8) ? REGISTER_D : REGISTER_T)
#define freeRegisterData(regist)             freeC47Blocks(getRegisterDataPointer(regist), getRegisterFullSizeInBlocks(regist))
#define storeToDtConfigDescriptor(config)    (configToStore->config = config)
#define recallFromDtConfigDescriptor(config) (config = configToRecall->config)

#define BPB                                  2 // 2^BPB = number of bytes per block
#define BYTES_PER_BLOCK                      (1 << BPB)
#define TO_BLOCKS(n)                         ((((uint32_t)n) + (BYTES_PER_BLOCK - 1)) >> BPB)
#define TO_BYTES(n)                          (((uint32_t)n) << BPB)

#define C47_NULL                             65535 // NULL pointer
#define TO_PCMEMPTR(p)                       ((void *)((p) == C47_NULL ? NULL : ram + (p)))
#define TO_C47MEMPTR(p)                      ((p) == NULL ? C47_NULL : (uint16_t)((uint32_t *)(p) - ram))

#define min(a,b)                             ((a)<(b)?(a):(b))
#define max(a,b)                             ((a)>(b)?(a):(b))
#define rmd(n, d)                            ((n)%(d))                                                       // rmd(n,d) = n - d*idiv(n,d)   where idiv is the division with decimal part truncature
#define mod(n, d)                            (((n)%(d) + (d)) % (d))                                         // mod(n,d) = n - d*floor(n/d)  where floor(a) is the biggest integer <= a
//#define modulo(n, d)                         ((n)%(d)<0 ? ((d)<0 ? (n)%(d) - (d) : (n)%(d) + (d)) : (n)%(d)) // modulo(n,d) = rmd(n,d) (+ |d| if rmd(n,d)<0)  thus the result is always >= 0
#define modulo(n, d)                         ((n)%(d)<0 ? (n)%(d)+(d) : (n)%(d))                             // This version works only if d > 0
#define nbrOfElements(x)                     (sizeof(x) / sizeof((x)[0]))                                    //dr

#define PROBMENU                             showingProbMenu()

#define BASEMODEACTIVE                       (!PROBMENU && (lastIntegerBase != 0 || softmenu[softmenuStack[0].softmenuId].menuItem == -MNU_BASE || dispBase > 0))

#define XXFNMODEACTIVE                       (!SHOWMODE && !GRAPHMODE && softmenu[softmenuStack[0].softmenuId].menuItem == -MNU_XXFCNS && calcMode != CM_NIM &&\
                                             ( (getRegisterDataType(REGISTER_X) == dtReal34 || getRegisterDataType(REGISTER_X) == dtLongInteger) ||\
                                               (getRegisterDataType(REGISTER_T) == dtReal34 || getRegisterDataType(REGISTER_T) == dtLongInteger)) )
                                               //PROBMENU not needed, as a specific menu is required for XXFN

#define DBASEMODE                            (!SHOWMODE && !GRAPHMODE && !PROBMENU && !XXFNMODEACTIVE && dispBase >= 2)

#define BASEMODEREGISTERX                    (BASEMODEACTIVE && \
                                              displayStackSHOIDISP != 0 && \
                                              ( \
                                                (calcMode == CM_NORMAL && getRegisterDataType(REGISTER_X) == dtShortInteger) || \
                                                (calcMode == CM_NIM && getRegisterDataType(REGISTER_Y) == dtShortInteger)   ||\
                                                (calcMode == CM_NORMAL && getRegisterDataType(REGISTER_X) == dtLongInteger)) \
                                              )
#define inputAngleMode3r(r)                  ((registerIsNoAngle(r+1) && registerIsNoAngle(r+2)) ? (!registerIsNoAngle(r) ? getRegisterAngularMode(r) : amNone) : amNone)
#define registerIsNoAngle(r)                 ((getRegisterDataType(r  ) == dtReal34 && getRegisterAngularMode(r) == amNone) || getRegisterDataType(r) == dtLongInteger)
#define inputIsNoAngle3r(r)                  ( registerIsNoAngle(r  )   || !registerIsNoAngle(r+1)  || !registerIsNoAngle(r+2))
#define inputAngleError3r(r)                 (!registerIsNoAngle(r+1)   || !registerIsNoAngle(r+2))
#define isXFNregisterValid3r(r)              ((getRegisterDataType(r  ) == dtReal34 || getRegisterDataType(r  ) == dtLongInteger) &&\
                                              (getRegisterDataType(r+1) == dtReal34 || getRegisterDataType(r+1) == dtLongInteger) &&\
                                              (getRegisterDataType(r+2) == dtReal34 || getRegisterDataType(r+2) == dtLongInteger) &&\
                                              !inputAngleError3r(r))
#define isXFNShowing(r)                      (menu(0) == -MNU_SHOW && menu(1) == -MNU_XXFCNS && isXFNregisterValid3r(r))



#define SHOWMODE                             (calcMode == CM_NORMAL && (temporaryInformation == TI_SHOW_REGISTER || temporaryInformation == TI_SHOW_REGISTER_BIG || temporaryInformation == TI_SHOW_REGISTER_SMALL || temporaryInformation == TI_SHOW_REGISTER_TINY || temporaryInformation == TI_SHOWNOTHING))
#define GRAPHMODE                            (calcMode == CM_PLOT_STAT || calcMode == CM_GRAPH)

#define COMPLEX_UNIT                         (getSystemFlag(FLAG_CPXj)   ? STD_op_j  : STD_op_i)  //Do not change back to single byte character - code must also change accordingly
#define PRODUCT_SIGN                         (getSystemFlag(FLAG_MULTx)  ? STD_CROSS : STD_DOT)

#define RADIX34_MARK_CHAR                    (gapChar1Radix[0] == ',' || (gapChar1Radix[0] == STD_WCOMMA[0] && gapChar1Radix[1] == STD_WCOMMA[1]) ? ',' : '.') //map comma and wide comma to comma, and dot and period and wdot and wperiod to period
#define RADIX34_MARK_STRING                  (gapChar1Radix)
#define RADIX34_MARK_DEC_ITM                 (RADIX34_MARK_CHAR == '.' ? ITM_PERIOD : ITM_COMMA)
#define RADIX34_MARK_NOT_DEC_ITM             (RADIX34_MARK_CHAR == '.' ? ITM_COMMA : ITM_PERIOD)

#define groupingGap                          ((uint8_t)(grpGroupingLeft)) //ADD HERE THE CONDITIONS FOR NIL SEPS

#define Lt                                   (gapItemLeft  == 0 ? (char*) "\1\1\0" : (char*)indexOfItems[gapItemLeft ].itemSoftmenuName) // Actual separator character
#define Rt                                   (gapItemRight == 0 ? (char*) "\1\1\0" : (char*)indexOfItems[gapItemRight].itemSoftmenuName) // Actual separator character
#define Rx                                   ((char*)indexOfItems[gapItemRadix].itemSoftmenuName) // Actual separator character
#define gapChar1Left                         (Lt[0] != 0 && Lt[1] == 0 && Lt[2] == 0 ? \
                                                ( Lt[0] == ','  ? (char*) ",\1\0"  :   \
                                                  Lt[0] == '.'  ? (char*) ".\1\0"  :   \
                                                  Lt[0] == '\'' ? (char*) "\'\1\0" :   \
                                                  Lt[0] == '_'  ? (char*) "_\1\0"  : Lt ) : Lt )  //set second character to skip character 0x01
#define gapChar1Right                        (Rt[0] != 0 && (Rt[1] == 0 || (Rt[1] != 0 && Rt[2] == 0)) ? \
                                                ( Rt[0] == ','  ? (char*) ",\1\0"  :   \
                                                  Rt[0] == '.'  ? (char*) ".\1\0"  :   \
                                                  Rt[0] == '\'' ? (char*) "\'\1\0" :   \
                                                  Rt[0] == '_'  ? (char*) "_\1\0"  : Rt ) : Rt )  //set second character to skip character 0x01
#define gapChar1Radix                        (Rx[0] != 0 && (Rx[1] == 0 || (Rx[1] != 0 && Rx[2] == 0)) ? \
                                                ( Rx[0] == ','  ? (char*) ",\1\0"  :   \
                                                  Rx[0] == '.'  ? (char*) ".\1\0"  : Rx ) : Rx )  //set second character to skip character 0x01

#define GROUPLEFT_DISABLED                   (GROUPWIDTH_LEFT == 0 || gapItemLeft == 0)
#define GROUPRIGHT_DISABLED                  (GROUPWIDTH_RIGHT == 0 || gapItemRight == 0)
#define SEPARATOR_LEFT                       (gapChar1Left)
#define SEPARATOR_RIGHT                      (gapChar1Right)

#define gapItemFrac                          (gapItemLeft == 0 ? 0 : ITM_SPACE_4_PER_EM)          // Fractions only get to have narrow space or nothing
#define SEPARATOR_FRAC                       (gapItemFrac  == 0 ? (char*) "\1\1\0" : (char*)indexOfItems[gapItemFrac].itemSoftmenuName) // Actual separator character

#define checkHP                              (significantDigits <= 16 && displayStack == 1 && exponentLimit == 99 && Input_Default == ID_DP && (calcMode == CM_NORMAL || calcMode == CM_NIM))
#define REPLACEFONT                          // Use HP 7-segment font
#ifdef REPLACEFONT
  #define DOUBLING_A                         15u // 16=is double; 14 is 1.75*; 12=1.5*; 10=1.25* (8 is the per unit norm horizontal factor, A/B)
  #define DOUBLINGBASE_B                     8u
  #define REDUCT_A1                          4   // Reduction vertical ratio A/B
  #define REDUCT_B1                          4
  #define REDUCT_OFFSET1                     0   // Reduction vertical offset
  #define HPFONT1                            true
#else
  #define DOUBLING_A                         14u // 16=is double; 14 is 1.75*; 12=1.5*; 10=1.25* (8 is the per unit norm horizontal factor, A/B)
  #define DOUBLINGBASE_B                     8u
  #define REDUCT_A1                          3   // Reduction ratio A/B
  #define REDUCT_B1                          4
  #define REDUCT_OFFSET1                     3   // Reduction offset
  #define HPFONT1                            false
#endif
#define DOUBLING                             (checkHP ? DOUBLING_A     : 6u)
#define DOUBLINGBASEX                        (checkHP ? DOUBLINGBASE_B : 8u)
#define REDUCT_A                             (checkHP ? REDUCT_A1      : 3)
#define REDUCT_B                             (checkHP ? REDUCT_B1      : 4)
#define REDUCT_OFF                           (checkHP ? REDUCT_OFFSET1 : 3)
#define HPFONT                               (checkHP ? HPFONT1        : false)


#define GROUPWIDTH_LEFT                      (grpGroupingLeft)
#define GROUPWIDTH_LEFT1                     ((grpGroupingGr1Left        == 0 ? (uint16_t)grpGroupingLeft : (uint16_t)grpGroupingGr1Left))
#define GROUPWIDTH_LEFT1X                    (grpGroupingGr1LeftOverflow)
#define GROUP1_OVFL(digitCount, exp)         ( (grpGroupingGr1LeftOverflow > 0 && exp == GROUPWIDTH_LEFT1 && digitCount+1 == GROUPWIDTH_LEFT1  ? grpGroupingGr1LeftOverflow:0 ) )
#define GROUPWIDTH_RIGHT                     (grpGroupingRight)
#define SEPARATOR_(digitCount)               (digitCount >= 0 ? SEPARATOR_LEFT : SEPARATOR_RIGHT)
#define GROUPWIDTH_(digitCount)              (digitCount >= 0 ? GROUPWIDTH_LEFT : GROUPWIDTH_RIGHT)
#define digitCountNEW(digitCount)            (  digitCount+1 > GROUPWIDTH_LEFT1 ? digitCount - GROUPWIDTH_LEFT1 : digitCount  )  //remaining digits to divide up into groups. "+1" due to the fact the the digit is recognized now, but only added below. So the sep gets added before the digit down below.
#define IS_SEPARATOR_(digitCount)            (   (digitCount+1 == GROUPWIDTH_LEFT1) \
                                              || ((digitCount+1  > GROUPWIDTH_LEFT1 || digitCount < 0) \
                                                  && (modulo(digitCountNEW(digitCount), (uint16_t)GROUPWIDTH_(digitCount)) == (uint16_t)GROUPWIDTH_(digitCount) - 1)) )

#define BLOCK_DOUBLEPRESS_MENU(menu, x, y)   ( \
                                               (menu == -MNU_ALPHA     && y == 0 && (x == 4 || x == 5)) || \
                                               (menu == -MNU_M_EDIT    && y == 0 && (x == 4 || x == 5)) || \
                                               (menu == -MNU_EQ_EDIT   && y == 0 && (x == 4 || x == 5)) || \
                                               (menu == -MNU_TAMALPHA  && y == 0 && (x == 4 || x == 5)) \
                                             )


#define IS_SIM_ARROW_ALLOWED_IN_MENU(menu, key) ( \
                                               (menu == -MNU_ALPHA   && (key == GDK_KEY_Up || key == GDK_KEY_Down || key == GDK_KEY_Left || key == GDK_KEY_Right) ) || \
                                               (menu == -MNU_M_EDIT  && (key == GDK_KEY_Up || key == GDK_KEY_Down || key == GDK_KEY_Left || key == GDK_KEY_Right) ) || \
                                               (menu == -MNU_EQ_EDIT && (                                            key == GDK_KEY_Left || key == GDK_KEY_Right) ) \
                                             )

#define IS_BASEBLANK_(menuId)                (menuId==0 && !getSystemFlag(FLAG_BASE_MYM) && !getSystemFlag(FLAG_BASE_HOME))

#define currentReturnProgramNumber           (currentSubroutineLevelData->returnProgramNumber   )
#define currentReturnLocalStep               (currentSubroutineLevelData->returnLocalStep       )
#define currentNumberOfLocalFlags            (currentSubroutineLevelData->numberOfLocalFlags    )
#define currentNumberOfLocalRegisters        (currentSubroutineLevelData->numberOfLocalRegisters)
#define currentSubroutineLevel               (currentSubroutineLevelData->subroutineLevel       )
#define currentPtrToNextLevel                (currentSubroutineLevelData->ptrToNextLevel        )
#define currentPtrToPreviousLevel            (currentSubroutineLevelData->ptrToPreviousLevel    )

#define LOCAL_FLAGS_AFTER_SUBROUTINE_LEVEL_HEADER(ptr)     ((localFlags_t     *)((subroutineLevelHeader_t  *)ptr + 1))
#define LOCAL_REGISTER_HEADERS_AFTER_LOCAL_FLAGS(ptr)      ((registerHeader_t *)((localFlags_t             *)ptr + 1))
#define REAL34_MATRIX_ELEMENTS_AFTER_MATRIX_HEADER(ptr)    ((real34_t         *)((matrixHeader_t           *)ptr + 1))
#define COMPLEX34_MATRIX_ELEMENTS_AFTER_MATRIX_HEADER(ptr) ((real34_t         *)((matrixHeader_t           *)ptr + 1))

#define isMatrix2dVector(rows,cols)          ((rows == 1 && cols == 2) || (rows == 2 && cols == 1))
#define isMatrix3dVector(rows,cols)          ((rows == 1 && cols == 3) || (rows == 3 && cols == 1))
#define isMatrixVector(rows,cols)            ((isMatrix3dVector(rows,cols) || isMatrix2dVector(rows,cols)))
#define getTagAngularMode(tag)               ( tag & amAngleMask)
#define is2dVectorPolar(tag)                 ((tag & amPolar) == amPolar)
#define is3dVectorPolarSPHCYL(tag)           ((tag & amPolar) == amPolar)
#define is3dVectorPolarSPH(tag)              (((getTagAngularMode(tag)) != amNone) &&  is3dVectorPolarSPHCYL(tag))
#define is3dVectorPolarCYL(tag)              (((getTagAngularMode(tag)) != amNone) && !is3dVectorPolarSPHCYL(tag))

#define isMatrix3dVectorSPH(rows,cols,tag)   (isMatrix3dVector(rows,cols) && is3dVectorPolarSPH(tag))
#define isMatrix3dVectorCYL(rows,cols,tag)   (isMatrix3dVector(rows,cols) && is3dVectorPolarCYL(tag))
#define isMatrix2dVectorPOL(rows,cols,tag)   (isMatrix2dVector(rows,cols) && is2dVectorPolar(tag))




#if defined(DMCP_BUILD)
  #define runningOnSimOrUSB getSystemFlag(FLAG_USB)    // used to compromise on complexity to increase speed
#else //!DMCP_BUILD
  #define runningOnSimOrUSB true
#endif //!DMCP_BUILD

#if !defined(PC_BUILD) && !defined(DMCP_BUILD)
  #error One of PC_BUILD and DMCP_BUILD must be defined
#endif // !PC_BUILD && !DMCP_BUILD

#if defined(PC_BUILD) && defined(DMCP_BUILD)
  #error Only one of PC_BUILD and DMCP_BUILD must be defined
#endif // PC_BUILD && DMCP_BUILD

#if !defined(OS32BIT) && !defined(OS64BIT)
  #error One of OS32BIT and OS64BIT must be defined
#endif // !OS32BIT && !OS64BIT

#if defined(OS32BIT) && defined(OS64BIT)
  #error Only one of OS32BIT and OS64BIT must be defined
#endif // OS32BIT && OS64BIT

#if defined(PC_BUILD)
  #if defined(WIN32) // No DEBUG_PANEL mode for Windows
    #undef  DEBUG_PANEL
    #define DEBUG_PANEL 0
  #endif // WIN32
  #if defined(RASPBERRY) // No DEBUG_PANEL mode for Raspberry Pi
    #undef  DEBUG_PANEL
    #define DEBUG_PANEL 0
  #endif // RASPBERRY
#endif // PC_BUILD

#if defined(DMCP_BUILD) || (SIMULATOR_ON_SCREEN_KEYBOARD == 0)
  #undef  DEBUG_PANEL
  #define DEBUG_PANEL 0
  #undef  DEBUG_REGISTER_L
  #define DEBUG_REGISTER_L 0
  #undef  SHOW_MEMORY_STATUS
  #define SHOW_MEMORY_STATUS 0
  #undef  EXTRA_INFO_ON_CALC_ERROR
  #define EXTRA_INFO_ON_CALC_ERROR 0
#endif // DMCP_BUILD || SIMULATOR_ON_SCREEN_KEYBOARD == 0

#if defined(TESTSUITE_BUILD) && !defined(GENERATE_CATALOGS)
  #undef  PC_BUILD
  #undef  DMCP_BUILD
  #undef  DEBUG_PANEL
  #define DEBUG_PANEL 0
  #undef  DEBUG_REGISTER_L
  #define DEBUG_REGISTER_L 0
  #undef  SHOW_MEMORY_STATUS
  #define SHOW_MEMORY_STATUS 0
  #undef  EXTRA_INFO_ON_CALC_ERROR
  #define EXTRA_INFO_ON_CALC_ERROR 0
  #define addItemToBuffer fnNop
  #define fnOff           fnNop
  #define fnAim           fnNop
  #define asnBrowser      fnNop
  #define registerBrowser fnNop
  #define flagBrowser     fnNop
  #define fontBrowser     fnNop
  #define flagBrowser_old fnNop       //JM
  #define refreshRegisterLine(a)  do {} while(0)
  #define displayBugScreen(a)     do { printf("\n-----------------------------------------------------------------------\n"); printf("%s\n", a); printf("\n-----------------------------------------------------------------------\n"); } while(0)
  #define showHideHourGlass()     do {} while(0)
  #define refreshScreen(a)        do {} while(0)
  #define refreshLcd(a)           do {} while(0)
  #define initFontBrowser()       do {} while(0)
#endif // TESTSUITE_BUILD && !GENERATE_CATALOGS

/* Turn off -Wunused-result for a specific function call */
#define ignoreReturnedValue(function) (__extension__ ({ __typeof__ (function) __x = (function); (void) __x; }))

#define TMP_STR_LENGTH         2560
#define WRITE_BUFFER_LEN       4096
#define ERROR_MESSAGE_LENGTH    512
#define DISPLAY_VALUE_LEN        80

#define FILENAMELEN              40

//************************
//* Macros for debugging *
//************************
#define COLOR_DEFAULT "\033[0m"
#define COLOR_RED     "\033[1;31m"
#define COLOR_GREEN   "\033[1;92m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_CYAN    "\033[1;36m"
#define COLOR_BLUE    "\033[1;38;5;12m"
#define debugf(a) do { fprintf(stderr, "%sdebug:%s %s %s(%s %s:%d)%s\n", COLOR_GREEN,  a, COLOR_DEFAULT, COLOR_CYAN, __FUNCTION__, __FILE__, __LINE__, COLOR_DEFAULT);fflush(stderr); } while(0)
#define errorf(a) do { fprintf(stderr, "%serror:%s %s %s(%s %s:%d)%s\n", COLOR_YELLOW, a, COLOR_DEFAULT, COLOR_CYAN, __FUNCTION__, __FILE__, __LINE__, COLOR_DEFAULT);fflush(stderr); } while(0)
#define abortf(a) do { fprintf(stderr, "%sabort: %s(%s %s:%d)%s\n",      COLOR_RED,                      COLOR_CYAN, __FUNCTION__, __FILE__, __LINE__, COLOR_DEFAULT);perror(a);fflush(stderr);abort(); } while(0)
#define userAbort(a) do { fprintf(stderr, "%serror:%s %s \n", COLOR_YELLOW, a, COLOR_DEFAULT);fflush(stderr); } while(0)

// To time a piece of code (not on DM42 hardware), you can use the following code snippet:
// struct timespec stopwatch_start, stopwatch_stop;
// clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stopwatch_start);
// : piece of code
// : to time
// clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stopwatch_stop);
// printf("Duration = %11.6fs\n", stopwatch_stop.tv_sec + stopwatch_stop.tv_nsec /1e9 - stopwatch_start.tv_sec - stopwatch_start.tv_nsec /1e9);

#define TEST_REG(r, comment)                                                                                       \
  do {                                                                                                             \
    if(globalRegister[r].dataPointer >= 500) {                                                                     \
      uint32_t a, b;                                                                                               \
      a = 1;                                                                                                       \
      b = 0;                                                                                                       \
      printf("\n=====> BAD  REGISTER %d DATA POINTER: %u <===== %s\n", r, globalRegister[r].dataPointer, comment); \
      globalRegister[r].dataType = a/b;                                                                            \
    }                                                                                                              \
    else {                                                                                                         \
      printf("\n=====> good register %d data pointer: %u <===== %s\n", r, globalRegister[r].dataPointer, comment); \
    }                                                                                                              \
  } while(0)

#define PRINT_LI(lint, comment)                            \
  do {                                                     \
    int i;                                                 \
    printf("\n%s", comment);                               \
    if((lint)->_mp_size == 0) printf(" lint=0");           \
    else if((lint)->_mp_size < 0) printf(" lint=-");       \
    else printf(" lint=+");                                \
    for(i=0; i<abs((lint)->_mp_size); i++) {               \
      printf("%" PRIu64, (uint64)((lint)->_mp_d[i]));      \
    }                                                      \
    printf("  _mp_alloc=%dlimbs=", (lint)->_mp_alloc);     \
    printf("%lubytes", LIMB_SIZE * (lint)->_mp_alloc);     \
    printf(" _mp_size=%dlimbs=", abs((lint)->_mp_size));   \
    printf("%lubytes", LIMB_SIZE * abs((lint)->_mp_size)); \
    printf(" PCaddress=%p", (lint)->_mp_d);                \
    printf(" 47address=%d", TO_C47MEMPTR((lint)->_mp_d));  \
    printf("\n");                                          \
  } while(0)


#define PRINT_LI_REG(reg, comment)                                                                   \
  do {                                                                                               \
    int i;                                                                                           \
    mp_limb_t *p;                                                                                    \
    printf("\n%s", comment);                                                                         \
    if(getRegisterLongIntegerSign(reg) == LI_ZERO) printf("lint=0");                                 \
    else if(getRegisterLongIntegerSign(reg) == LI_NEGATIVE) printf("lint=-");                        \
    else printf("lint=+");                                                                           \
    for(i=*REGISTER_DATA_MAX_LEN(reg)/LIMB_SIZE, p=REGISTER_LONG_INTEGER_DATA(reg); i>0; i--, p++) { \
      printf("%lu ", *p);                                                                            \
    }                                                                                                \
    printf(" maxLen=%dbytes=", *REGISTER_DATA_MAX_LEN(reg));                                         \
    printf("%lulimbs", *REGISTER_DATA_MAX_LEN(reg) / LIMB_SIZE);                                     \
    printf("\n");                                                                                    \
  } while(0)

#if defined(DMCP_BUILD)
  /* Import a binary file - from https://elm-chan.org/junk/32bit/binclude.html */
  #define IMPORT_BIN(sect, file, sym) asm (\
      ".section " #sect "\n"                  /* Change section */\
      ".balign 4\n"                           /* Word alignment */\
      ".global " #sym "\n"                    /* Export the object address */\
      #sym ":\n"                              /* Define the object label */\
      ".incbin \"" file "\"\n"                /* Import the file */\
      ".global _sizeof_" #sym "\n"            /* Export the object size */\
      ".set _sizeof_" #sym ", . - " #sym "\n" /* Define the object size */\
      ".balign 4\n"                           /* Word alignment */\
      ".section \".text\"\n")                 /* Restore section */
#endif // DMCP_BUILD

#endif // !DEFINES_H
