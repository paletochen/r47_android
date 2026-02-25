// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"

//C47 defaults (used in both settings arrays)
#define _gapl   ITM_SPACE_PUNCTUATION
#define _gprl   3
#define _gpr1x  0
#define _gpr1   0
#define _gprr   3
#define _gapr   ITM_SPACE_PUNCTUATION
#define _gaprx  ITM_PERIOD

TO_QSPI static const struct {
    unsigned tdm24 : 1;
    unsigned dmy : 1;
    unsigned mdy : 1;
    unsigned ymd : 1;
    unsigned gregorianDay : 22;
    unsigned gapl : 16;
    unsigned gprl : 4;
    unsigned gpr1x: 4;
    unsigned gpr1 : 4;
    unsigned gprr : 4;
    unsigned gapr : 16;
    unsigned gaprx : 16;
    unsigned us   : 1;
    unsigned woy : 16;


} configSettings[] = {
                /*                         gapl                     gprl  gpr1x  gpr1  gprr     gapr                                   */
                /*   24  D M Y  Gregorian  GAP char                 GRP   GRPx   GRP1  FP.GRP   FP.GAP char               New Radix    */
    [CFG_DFLT  ] = {  1, 0,0,1, 2361222,   _gapl                , _gprl, _gpr1x, _gpr1, _gprr, _gapr                 ,   _gaprx    , 0, ITM_WOY_ISO},    /* 14 Sep 1752 */
    [CFG_CHINA ] = {  1, 0,0,1, 2433191,   ITM_COMMA            ,    4,    0,    0,    4,      ITM_COMMA             ,   ITM_PERIOD, 0, ITM_WOY_ISO},    /*  1 Oct 1949 */
    [CFG_EUROPE] = {  1, 1,0,0, 2299161,   ITM_SPACE_PUNCTUATION,    3,    0,    0,    3,      ITM_SPACE_PUNCTUATION ,   ITM_COMMA , 0, ITM_WOY_ISO},    /* 15 Oct 1582 */
    [CFG_INDIA ] = {  1, 1,0,0, 2361222,   ITM_COMMA            ,    2,    0,    3,    2,      ITM_COMMA             ,   ITM_PERIOD, 0, ITM_WOY_US},    /* 14 Sep 1752 */
    [CFG_JAPAN ] = {  1, 0,0,1, 2405160,   ITM_SPACE_PUNCTUATION,    3,    0,    0,    3,      ITM_SPACE_PUNCTUATION ,   ITM_PERIOD, 0, ITM_WOY_US},    /*  1 Jan 1873 */
    [CFG_UK    ] = {  0, 1,0,0, 2361222,   ITM_SPACE_PUNCTUATION,    3,    0,    0,    3,      ITM_SPACE_PUNCTUATION ,   ITM_PERIOD, 0, ITM_WOY_ISO},    /* 14 Sep 1752 */
    [CFG_USA   ] = {  0, 0,1,0, 2361222,   ITM_COMMA            ,    3,    9,    0,    3,      ITM_NULL              ,   ITM_PERIOD, 1, ITM_WOY_US},    /* 14 Sep 1752 */
};


bool_t isConfigCommon(uint16_t id) {
  uint16_t idx = 99;
  switch(id) {
    case ITM_SETDFLT: idx = CFG_DFLT  ; break;
    case ITM_SETCHN : idx = CFG_CHINA ; break;
    case ITM_SETEUR : idx = CFG_EUROPE; break;
    case ITM_SETIND : idx = CFG_INDIA ; break;
    case ITM_SETJPN : idx = CFG_JAPAN ; break;
    case ITM_SETUK  : idx = CFG_UK    ; break;
    case ITM_SETUSA : idx = CFG_USA   ; break;
    default:;
  }
  bool_t cmp1  = (int32_t)getSystemFlag(FLAG_TDM24) == (int32_t)configSettings[idx].tdm24;
  bool_t cmp2  = (int32_t)getSystemFlag(FLAG_DMY)   == (int32_t)configSettings[idx].dmy;
  bool_t cmp3  = (int32_t)getSystemFlag(FLAG_MDY)   == (int32_t)configSettings[idx].mdy;
  bool_t cmp4  = (int32_t)getSystemFlag(FLAG_YMD)   == (int32_t)configSettings[idx].ymd;
  bool_t cmp5  = (int32_t)firstGregorianDay == (int32_t)configSettings[idx].gregorianDay;
  bool_t cmp6a = ((int32_t)configSettings[idx].woy == ITM_WOY_ISO && (int32_t)firstDayOfWeek == 1 && (int32_t)firstWeekOfYearDay == 4);
  bool_t cmp6b = ((int32_t)configSettings[idx].woy == ITM_WOY_US  && (int32_t)firstDayOfWeek == 7 && (int32_t)firstWeekOfYearDay == 6);
  bool_t cmp6c = ((int32_t)configSettings[idx].woy == ITM_WOY_ME  && (int32_t)firstDayOfWeek == 6 && (int32_t)firstWeekOfYearDay == 5);
  bool_t cmp6  = (cmp6a || cmp6b || cmp6c);
  bool_t cmp7  = (0x3FFF & (uint32_t)gapItemLeft) == (0x3FFF & (int32_t)configSettings[idx].gapl);
  bool_t cmp8  = (int32_t)grpGroupingLeft == (int32_t)configSettings[idx].gprl;
  bool_t cmp9  = (int32_t)grpGroupingGr1LeftOverflow == (int32_t)configSettings[idx].gpr1x;
  bool_t cmp10 = (int32_t)grpGroupingGr1Left == (int32_t)configSettings[idx].gpr1;
  bool_t cmp11 = (int32_t)grpGroupingRight == (int32_t)configSettings[idx].gprr;
  bool_t cmp12 = (0x3FFF & (uint32_t)gapItemRight) == (0x3FFF & (uint32_t)configSettings[idx].gapr);
  bool_t cmp13 = (0x3FFF & (uint32_t)gapItemRadix) == (0x3FFF & (uint32_t)configSettings[idx].gaprx);
  bool_t cmp14 = (int32_t)getSystemFlag(FLAG_US) == (int32_t)configSettings[idx].us;

  //printf("DEBUG: Boolean comparison breakdown (idx=%d)\n", (int32_t)idx);
  //printf("  FLAG_TDM24:               %d == %d → %s\n", (int32_t)getSystemFlag(FLAG_TDM24), (int32_t)configSettings[idx].tdm24, cmp1 ? "TRUE" : "FALSE");
  //printf("  FLAG_DMY:                 %d == %d → %s\n", (int32_t)getSystemFlag(FLAG_DMY), (int32_t)configSettings[idx].dmy, cmp2 ? "TRUE" : "FALSE");
  //printf("  FLAG_MDY:                 %d == %d → %s\n", (int32_t)getSystemFlag(FLAG_MDY), (int32_t)configSettings[idx].mdy, cmp3 ? "TRUE" : "FALSE");
  //printf("  FLAG_YMD:                 %d == %d → %s\n", (int32_t)getSystemFlag(FLAG_YMD), (int32_t)configSettings[idx].ymd, cmp4 ? "TRUE" : "FALSE");
  //printf("  firstGregorianDay:        %d == %d → %s\n", (int32_t)firstGregorianDay, (int32_t)configSettings[idx].gregorianDay, cmp5 ? "TRUE" : "FALSE");
  //printf("  WOY (compound):\n");
  //printf("    ISO (woy=%d fdw=%d fwd=%d): %s\n", (int32_t)configSettings[idx].woy, (int32_t)firstDayOfWeek, (int32_t)firstWeekOfYearDay, cmp6a ? "TRUE" : "FALSE");
  //printf("    US  (woy=%d fdw=%d fwd=%d): %s\n", (int32_t)configSettings[idx].woy, (int32_t)firstDayOfWeek, (int32_t)firstWeekOfYearDay, cmp6b ? "TRUE" : "FALSE");
  //printf("    ME  (woy=%d fdw=%d fwd=%d): %s\n", (int32_t)configSettings[idx].woy, (int32_t)firstDayOfWeek, (int32_t)firstWeekOfYearDay, cmp6c ? "TRUE" : "FALSE");
  //printf("    Combined WOY:           → %s\n", cmp6 ? "TRUE" : "FALSE");
  //printf("  gapItemLeft:              %d == %d → %s\n", (0x3FFF & (uint32_t)gapItemLeft), (0x3FFF & (uint32_t)configSettings[idx].gapl), cmp7 ? "TRUE" : "FALSE");
  //printf("  grpGroupingLeft:          %d == %d → %s\n", (int32_t)grpGroupingLeft, (int32_t)configSettings[idx].gprl, cmp8 ? "TRUE" : "FALSE");
  //printf("  grpGroupingGr1LeftOvrflw: %d == %d → %s\n", (int32_t)grpGroupingGr1LeftOverflow, (int32_t)configSettings[idx].gpr1x, cmp9 ? "TRUE" : "FALSE");
  //printf("  grpGroupingGr1Left:       %d == %d → %s\n", (int32_t)grpGroupingGr1Left, (int32_t)configSettings[idx].gpr1, cmp10 ? "TRUE" : "FALSE");
  //printf("  grpGroupingRight:         %d == %d → %s\n", (int32_t)grpGroupingRight, (int32_t)configSettings[idx].gprr, cmp11 ? "TRUE" : "FALSE");
  //printf("  gapItemRight:             %d == %d → %s\n", (0x3FFF & (uint32_t)gapItemRight), (0x3FFF & (uint32_t)configSettings[idx].gapr), cmp12 ? "TRUE" : "FALSE");
  //printf("  gapItemRadix:             %d == %d → %s\n", (0x3FFF & (uint32_t)gapItemRadix), (0x3FFF & (uint32_t)configSettings[idx].gaprx), cmp13 ? "TRUE" : "FALSE");
  //printf("  FLAG_US:                  %d == %d → %s\n", (int32_t)getSystemFlag(FLAG_US), (int32_t)configSettings[idx].us, cmp14 ? "TRUE" : "FALSE");

  bool_t finalResult = cmp1 && (cmp2 || cmp3 || cmp4) && cmp5 && cmp6 && cmp7 && cmp8 && cmp9 && cmp10 && cmp11 && cmp12 && cmp13 && cmp14;
  //printf("  ═══════════════════════════════════════════\n");
  //printf("  FINAL RESULT:             → %s\n", finalResult ? "TRUE" : "FALSE");

  return finalResult;
}

void configCommon(uint16_t idx) {
  #if !defined(TESTSUITE_BUILD)
    if(checkHP) {
      fnSetC47(0);
    }
  #endif // !TESTSUITE_BUILD

  forceSystemFlag(FLAG_TDM24, configSettings[idx].tdm24);
  forceSystemFlag(FLAG_DMY, configSettings[idx].dmy);
  forceSystemFlag(FLAG_MDY, configSettings[idx].mdy);
  forceSystemFlag(FLAG_YMD, configSettings[idx].ymd);
  firstGregorianDay = configSettings[idx].gregorianDay;
  fnSetWeekOfYearRule(configSettings[idx].woy);
  temporaryInformation = TI_DISP_JULIAN_WOY;

  fnSetGapChar (0 + configSettings[idx].gapl);
  grpGroupingLeft            = configSettings[idx].gprl ;
  grpGroupingGr1LeftOverflow = configSettings[idx].gpr1x;
  grpGroupingGr1Left         = configSettings[idx].gpr1 ;
  grpGroupingRight           = configSettings[idx].gprr ;
  fnSetGapChar (32768+configSettings[idx].gapr);
  fnSetGapChar (49152+configSettings[idx].gaprx);
  forceSystemFlag(FLAG_US, configSettings[idx].us);
}





#define InputDefaultDataType  101    // config_fnInDefault
#define SigFigNumberOfDigits  102    // config_fnDisplayFormatSigFig
#define AllNumberOfDigits     103    // config_fnDisplayFormatAll
#define FixNumberOfDigits     104    // config_fnDisplayFormatFix
#define MymB                  105    // config_BASE_MYM
#define HomeB                 106    // config_BASE_HOME
#define RNG                   107    // config_exponentLimit
#define SDIGS                 108    // config_significantDigits
#define DSTACK                109    // config_displayStack
#define CACHEDDSTACK          110    // config_cachedDisplayStack
#define ADM                   111    // config_currentAngularMode
#define IPGRP                 112    // config_grpGroupingLeft
#define FPGRP                 113    // config_grpGroupingRight
#define IPGRP1                114    // config_grpGroupingGr1Left
#define IPGRP1x               115    // config_grpGroupingGr1LeftOverflow
#define HIDE                  118    // config_exponentHideLimit

#define DenMaX                120    // config_denmax
#define TVMIKnown             121    // tvm
#define TVMIChanges           122    // tvm
#define FDIGS                 123    // config_fractionDigits


#define xxx -10001
#define _Reset   1
#define _HP35    2           //HP35[ENTER]1972
#define _JM      3           //C47JM
#define _RJ      4           //C47RJ
#define _C47     5           //C47[ENTER]2023
#define _DefltSB 6
#define _TVM     7
#define _numberOfGrps 7



TO_QSPI const int32_t Settings[] = {
//variable,                          n/a,        Reset,                          HP35,            JM,                   RJ,                     C47,             DefltSB,         TVM,                  Comment
InputDefaultDataType,                xxx,        xxx,                            ID_DP,           ID_43S,               ID_DP,                  ID_43S,          xxx,             xxx,
SigFigNumberOfDigits,                xxx,        xxx,                            9,               3,                    xxx,                    xxx,             xxx,             xxx,
AllNumberOfDigits,                   xxx,        xxx,                            xxx,             xxx,                  xxx,                    3,               xxx,             xxx,
FixNumberOfDigits,                   xxx,        xxx,                            xxx,             xxx,                  3,                      xxx,             xxx,             xxx,
RNG,                                 xxx,        6145,                           99,              6145,                 6145,                   6145,            xxx,             xxx,
SDIGS,                               xxx,        0,                              16,              0,                    0,                      34,              xxx,             xxx,
FDIGS,                               xxx,        0,                              16,              31,                   0,                      34,              xxx,             xxx,
HIDE,                                xxx,        0,                              0,               31,                   0,                      0,               xxx,             xxx,
DSTACK,                              xxx,        4,                              1,               4,                    4,                      4,               xxx,             xxx,
CACHEDDSTACK,                        xxx,        4,                              1,               4,                    4,                      4,               xxx,             xxx,
ADM,                                 xxx,        amDegree,                       amRadian,        amDegree,             amRadian,               amDegree,        xxx,             xxx,
IPGRP,                               xxx,        3,                              3,               _gprl,                3,                      _gprl,           xxx,             xxx,
FPGRP,                               xxx,        3,                              3,               _gprr,                3,                      _gprr,           xxx,             xxx,
IPGRP1,                              xxx,        0,                              0,               _gpr1,                0,                      _gpr1,           xxx,             xxx,
IPGRP1x,                             xxx,        0,                              0,               _gpr1x,               1,                      _gpr1x,          xxx,             xxx,

3,                                   0,          FLAG_IRFRAC,                    xxx,             xxx,                  xxx,                    xxx,             xxx,             xxx,
3,                                   1,          xxx,                            xxx,             FLAG_IRFRAC,          FLAG_IRFRAC,            xxx,             xxx,             xxx,
3,                                   0,          FLAG_IRFRQ,                     xxx,             xxx,                  xxx,                    xxx,             xxx,             xxx,
3,                                   1,          xxx,                            xxx,             FLAG_IRFRQ,           FLAG_IRFRQ,             xxx,             xxx,             xxx,
3,                                   0,          xxx,                            FLAG_ERPN,       xxx,                  xxx,                    xxx,             xxx,             xxx,
3,                                   1,          FLAG_ERPN,                      xxx,             FLAG_ERPN,            FLAG_ERPN,              FLAG_ERPN,       xxx,             xxx,
3,                                   0,          FLAG_CPXMULT,                   xxx,             xxx,                  xxx,                    xxx,             xxx,             xxx,
3,                                   1,          FLAG_LARGELI,                   xxx,             xxx,                  xxx,                    xxx,             xxx,             xxx,
3,                                   0,          FLAG_PFX_ALL,                   xxx,             xxx,                  xxx,                    xxx,             xxx,             xxx,
3,                                   0,          FLAG_DREAL,                     xxx,             xxx,                  xxx,                    xxx,             xxx,             xxx,
3,                                   1,          xxx,                            xxx,             FLAG_DREAL,           FLAG_DREAL,             xxx,             xxx,             xxx,



DenMaX,                              xxx,        64,                             xxx,             120,                  999,                    64,              xxx,             xxx,
//TVM,                               n/a,        Reset,                          HP35,            JM,                   RJ,                     C47,             DefltSB,         TVM,
TVMIKnown,                           xxx,        0,                              xxx,             xxx,                  xxx,                    xxx,             xxx,             0,                    // Clear flag TVMIKnown
TVMIChanges,                         xxx,        0,                              xxx,             xxx,                  xxx,                    xxx,             xxx,             0,                    // Clear flag TVMIChanges
//TVM,                               n/a,        Reset,                          HP35,            JM,                   RJ,                     C47,             DefltSB,         TVM,
RESERVED_VARIABLE_LX,                xxx,        -10,                            xxx,             xxx,                  xxx,                    xxx,             xxx,             0,
RESERVED_VARIABLE_UX,                xxx,        10,                             xxx,             xxx,                  xxx,                    xxx,             xxx,             0,
RESERVED_VARIABLE_LY,                xxx,        0,                              xxx,             xxx,                  xxx,                    xxx,             xxx,             0,
RESERVED_VARIABLE_UY,                xxx,        0,                              xxx,             xxx,                  xxx,                    xxx,             xxx,             0,
RESERVED_VARIABLE_FV,                xxx,        0,                              xxx,             xxx,                  xxx,                    xxx,             xxx,             0,
RESERVED_VARIABLE_IPONA,             xxx,        0,                              xxx,             xxx,                  xxx,                    xxx,             xxx,             0,
RESERVED_VARIABLE_NPPER,             xxx,        0,                              xxx,             xxx,                  xxx,                    xxx,             xxx,             0,
RESERVED_VARIABLE_PMT,               xxx,        0,                              xxx,             xxx,                  xxx,                    xxx,             xxx,             0,
RESERVED_VARIABLE_PV,                xxx,        0,                              xxx,             xxx,                  xxx,                    xxx,             xxx,             0,
RESERVED_VARIABLE_PPERONA,           xxx,        12,                             xxx,             xxx,                  xxx,                    xxx,             xxx,             12,
RESERVED_VARIABLE_CPERONA,           xxx,        12,                             xxx,             xxx,                  xxx,                    xxx,             xxx,             12,
3,                                   1,          FLAG_ENDPMT,                    xxx,             xxx,                  xxx,                    xxx,             xxx,             FLAG_ENDPMT,          // Set flag  FLAG_ENDPMT
//FLAG,                              set/clear,  Reset,                          HP35,            JM,                   RJ,                     C47,             DefltSB,         TVM,
3,                                   1,          FLAG_MONIT,                     xxx,             xxx,                  xxx,                    xxx,             xxx,             xxx,                  // Set flag  FLAG_MONIT
3,                                   1,          FLAG_HPCONV,                    xxx,             xxx,                  xxx,                    xxx,             xxx,             xxx,                  // Set flag  FLAG_HPCONV
3,                                   0,          xxx,                            xxx,             FLAG_HPCONV,          FLAG_HPCONV,            xxx,             xxx,             xxx,                  // Clear flag FLAG_HPCONV
3,                                   0,          FLAG_CLX_DROP,                  xxx,             FLAG_CLX_DROP,        FLAG_CLX_DROP,          xxx,             xxx,             xxx,                  // Clear flag FLAG_HPCONV
3,                                   1,          FLAG_SH_LONGPRESS,              xxx,             FLAG_SH_LONGPRESS,    FLAG_SH_LONGPRESS,      xxx,             xxx,             xxx,                  // Set flag  FLAG_SH_LONGPRESS
3,                                   0,          xxx,                            xxx,             FLAG_USER,            FLAG_USER,              xxx,             xxx,             xxx,                  // Clear flag FLAG_USER
3,                                   1,          FLAG_SBdate,                    xxx,             xxx,                  xxx,                    xxx,             FLAG_SBdate,     xxx,                  // Set flag  FLAG_SBdate
3,                                   0,          FLAG_SBtime,                    xxx,             FLAG_SBtime,          xxx,                    xxx,             FLAG_SBtime,     xxx,                  // Clear flag FLAG_SBtime
3,                                   0,          FLAG_SBcr,                      xxx,             xxx,                  xxx,                    xxx,             FLAG_SBcr,       xxx,                  // Clear flag FLAG_SBcr
3,                                   1,          xxx,                            xxx,             xxx,                  FLAG_SBcr,              xxx,             xxx,             xxx,                  // Set flag  FLAG_SBcr
3,                                   1,          FLAG_SBcpx,                     xxx,             xxx,                  xxx,                    xxx,             FLAG_SBcpx,      xxx,                  // Set flag  FLAG_SBcpx
3,                                   0,          FLAG_SBang,                     xxx,             xxx,                  xxx,                    xxx,             FLAG_SBang,      xxx,                  // Clear flag FLAG_SBang
3,                                   1,          FLAG_SBfrac,                    xxx,             xxx,                  xxx,                    xxx,             FLAG_SBfrac,     xxx,                  // Set flag  FLAG_SBfrac
3,                                   1,          FLAG_SBint,                     xxx,             xxx,                  xxx,                    xxx,             FLAG_SBint,      xxx,                  // Set flag  FLAG_SBint
3,                                   0,          xxx,                            xxx,             xxx,                  FLAG_SBint,             xxx,             xxx,             xxx,                  // Clear flag FLAG_SBint
3,                                   0,          FLAG_SBmx,                      xxx,             xxx,                  xxx,                    xxx,             FLAG_SBmx,       xxx,                  // Clear flag FLAG_SBmx
3,                                   1,          xxx,                            xxx,             xxx,                  FLAG_SBmx,              xxx,             xxx,             xxx,                  // Set flag  FLAG_SBmx
3,                                   1,          FLAG_SBtvm,                     xxx,             xxx,                  xxx,                    xxx,             FLAG_SBtvm,      xxx,                  // Set flag  FLAG_SBtvm
3,                                   1,          FLAG_SBoc,                      xxx,             xxx,                  xxx,                    xxx,             FLAG_SBoc,       xxx,                  // Set flag  FLAG_SBoc
3,                                   0,          FLAG_SBss,                      xxx,             xxx,                  xxx,                    xxx,             FLAG_SBss,       xxx,                  // Clear flag FLAG_SBss
3,                                   1,          FLAG_SBstpw,                    xxx,             xxx,                  xxx,                    xxx,             FLAG_SBstpw,     xxx,                  // Set flag  FLAG_SBstpw
3,                                   0,          xxx,                            xxx,             xxx,                  FLAG_SBstpw,            xxx,             xxx,             xxx,                  // Clear flag FLAG_SBstpw
3,                                   1,          FLAG_SBser,                     xxx,             xxx,                  xxx,                    xxx,             FLAG_SBser,      xxx,                  // Set flag  FLAG_SBser
3,                                   1,          FLAG_SBprn,                     xxx,             xxx,                  xxx,                    xxx,             FLAG_SBprn,      xxx,                  // Set flag  FLAG_SBprn
3,                                   0,          FLAG_SBbatV,                    xxx,             xxx,                  xxx,                    xxx,             FLAG_SBbatV,     xxx,                  // Clear flag FLAG_SBbatV
3,                                   1,          xxx,                            xxx,             FLAG_SBbatV,          FLAG_SBbatV,            xxx,             xxx,             xxx,                  // Set flag  FLAG_SBbatV
3,                                   0,          FLAG_SBshfR,                    xxx,             xxx,                  xxx,                    xxx,             FLAG_SBshfR,     xxx,                  // Clear flag FLAG_SBshfR
3,                                   1,          FLAG_MULTx,                     xxx,             xxx,                  xxx,                    xxx,             xxx,             xxx,                  // Set flag  FLAG_MULTx
3,                                   0,          xxx,                            xxx,             FLAG_MULTx,           xxx,                    xxx,             xxx,             xxx,                  // Set flag  FLAG_MULTx
3,                                   1,          FLAG_AUTOFF,                    xxx,             xxx,                  xxx,                    xxx,             xxx,             xxx,                  // Set flag  FLAG_AUTOFF
3,                                   1,          FLAG_ENDPMT,                    xxx,             xxx,                  xxx,                    xxx,             xxx,             xxx,                  // Set flag  FLAG_ENDPMT
3,                                   1,          FLAG_HPRP,                      xxx,             xxx,                  xxx,                    xxx,             xxx,             xxx,                  // Set flag  FLAG_HPRP
3,                                   0,          xxx,                            xxx,             FLAG_HPRP,            FLAG_HPRP,              xxx,             xxx,             xxx,                  // Clear flag FLAG_HPRP
3,                                   1,          FLAG_MNUp1,                     xxx,             xxx,                  xxx,                    xxx,             xxx,             xxx,                  // Set flag  FLAG_MNUp1
3,                                   0,          xxx,                            xxx,             FLAG_MNUp1,           FLAG_MNUp1,             xxx,             xxx,             xxx,                  // Clear flag FLAG_MNUp1
3,                                   1,          FLAG_HPBASE,                    xxx,             FLAG_HPBASE,          xxx,                    xxx,             xxx,             xxx,                  // Set flag  FLAG_HPBASE
3,                                   0,          xxx,                            xxx,             xxx,                  FLAG_HPBASE,            xxx,             xxx,             xxx,                  // Clear flag FLAG_HPBASE
3,                                   0,          FLAG_2TO10,                     xxx,             FLAG_2TO10,           FLAG_2TO10,             xxx,             xxx,             xxx,                  // Clear flag FLAG_2TO10
3,                                   0,          xxx,                            xxx,             FLAG_POLAR,           xxx,                    xxx,             xxx,             xxx,                  // Clear flag FLAG_POLAR
3,                                   0,          xxx,                            xxx,             xxx,                  xxx,                    FLAG_CPXj,       xxx,             xxx,                  // Clear flag FLAG_CPXj
3,                                   1,          xxx,                            xxx,             FLAG_CPXj,            xxx,                    xxx,             xxx,             xxx,                  // Set flag  FLAG_CPXj
3,                                   1,          FLAG_CPXRES,                    xxx,             FLAG_CPXRES,          FLAG_CPXRES,            FLAG_CPXRES,     xxx,             xxx,                  // Set flag  FLAG_CPXRES
3,                                   0,          xxx,                            FLAG_CPXRES,     xxx,                  xxx,                    xxx,             xxx,             xxx,                  // Set flag  FLAG_CPXRES
3,                                   1,          FLAG_SPCRES,                    xxx,             FLAG_SPCRES,          FLAG_SPCRES,            FLAG_SPCRES,     xxx,             xxx,                  // Set flag  FLAG_SPCRES
3,                                   0,          xxx,                            FLAG_SPCRES,     xxx,                  xxx,                    xxx,             xxx,             xxx,                  // Set flag  FLAG_SPCRES
3,                                   1,          FLAG_SSIZE8,                    xxx,             FLAG_SSIZE8,          FLAG_SSIZE8,            FLAG_SSIZE8,     xxx,             xxx,                  // Set flag  FLAG_SSIZE8
3,                                   0,          xxx,                            FLAG_SSIZE8,     xxx,                  xxx,                    xxx,             xxx,             xxx,                  // Set flag  FLAG_SSIZE8
3,                                   0,          FLAG_ASLIFT,                    xxx,             xxx,                  xxx,                    xxx,             xxx,             xxx,                  // Clear flag FLAG_ASLIFT
3,                                   1,          xxx,                            xxx,             FLAG_ENGOVR,          xxx,                    xxx,             xxx,             xxx,                  // Clear flag FLAG_ASLIFT
3,                                   1,          FLAG_TOPHEX,                    xxx,             FLAG_TOPHEX,          FLAG_TOPHEX,            FLAG_TOPHEX,     xxx,             xxx,                  // Clear flag FLAG_TOPHEX

//fractions
3,                                   0,          FLAG_DENFIX,                    xxx,             FLAG_DENFIX,          xxx,                    xxx,             xxx,             xxx,                  // Clear flag FLAG_DENFIX
3,                                   0,          FLAG_FRACT,                     xxx,             FLAG_FRACT,           xxx,                    xxx,             xxx,             xxx,                  // Clear flag FLAG_FRACT
3,                                   1,          FLAG_PROPFR,                    xxx,             xxx,                  xxx,                    xxx,             xxx,             xxx,                  // Set flag  FLAG_PROPFR
3,                                   0,          xxx,                            xxx,             FLAG_PROPFR,          FLAG_PROPFR,            xxx,             xxx,             xxx,                  // Set flag  FLAG_PROPFR
3,                                   0,          FLAG_DENANY,                    xxx,             xxx,                  xxx,                    xxx,             xxx,             xxx,                  // Clear flag FLAG_DENANY
3,                                   1,          xxx,                            xxx,             FLAG_DENANY,          FLAG_DENANY,            xxx,             xxx,             xxx,                  // Set flag  FLAG_DENANY
3,                                   0,          FLAG_FRCSRN,                    xxx,             FLAG_FRCSRN,          xxx,                    xxx,             xxx,             xxx,                  // Clear flag FLAG_FRCSRN
3,                                   0,          FLAG_FRCYC,                     xxx,             xxx,                  xxx,                    xxx,             xxx,             xxx,                  // Clear flag FLAG_FRCYC


3,                                   1,          FLAG_G_DOUBLETAP,               xxx,             xxx,                  xxx,                    xxx,             xxx,             xxx,
3,                                   1,          FLAG_HOME_TRIPLE,               xxx,             xxx,                  xxx,                    xxx,             xxx,             xxx,
3,                                   0,          FLAG_MYM_TRIPLE,                xxx,             xxx,                  xxx,                    xxx,             xxx,             xxx,
3,                                   0,          FLAG_SHFT_4s,                   xxx,             xxx,                  xxx,                    xxx,             xxx,             xxx,
3,                                   1,          FLAG_BASE_MYM,                  xxx,             FLAG_BASE_MYM,        FLAG_BASE_MYM,          FLAG_BASE_MYM,   xxx,             xxx,
3,                                   0,          xxx,                            FLAG_BASE_MYM,   xxx,                  xxx,                    xxx,             xxx,             xxx,
3,                                   0,          FLAG_BASE_HOME,                 FLAG_BASE_HOME,  FLAG_BASE_HOME,       FLAG_BASE_HOME,         FLAG_BASE_HOME,  xxx,             xxx,

3,                                   1,          FLAG_FGLNFUL,                   xxx,             FLAG_FGLNFUL,         FLAG_FGLNFUL,           FLAG_FGLNFUL,    xxx,             xxx,
3,                                   0,          xxx,                            FLAG_FGLNFUL,    xxx,                  xxx,                    xxx,             xxx,             xxx,
3,                                   0,          xxx,                            FLAG_FGLNLIM,    xxx,                  xxx,                    xxx,             xxx,             xxx,

3,                                   0,          FLAG_FGGR,                      xxx,             xxx,                  FLAG_FGGR,              FLAG_FGGR,       xxx,             xxx,
3,                                   1,          xxx,                            xxx,             FLAG_FGGR,            xxx,                    xxx,             xxx,             xxx,



//fnSetGapChar,                      n/a,        Reset,                          HP35,            JM,                   RJ,                     C47,             DefltSB,         TVM,
4,                                   xxx,        0+ITM_SPACE_PUNCTUATION,        ITM_NULL,        0+_gapl,              0+ITM_SPACE_4_PER_EM,   0+_gapl,         xxx,             xxx,                  //fnSetGapChar
4,                                   xxx,        32768+ITM_SPACE_PUNCTUATION,    ITM_NULL+32768,  32768+_gapr,          32768+ITM_SPACE_4_PER_EM,32768+_gapr,    xxx,             xxx,                  //fnSetGapChar
4,                                   xxx,        49152+ITM_PERIOD,               ITM_WDOT+49152,  49152+ITM_WDOT,       49152+ITM_WCOMMA,       49152+_gaprx,    xxx,             xxx,                  //fnSetGapChar
0,                                   0,          0,                              0,               0,                    0,                      0,               0,               0,                    //END MARKER

//Setsetting,                        n/a,        Reset,                          HP35,            JM,                   RJ,                     C47,             DefltSB,         TVM,
//2,                                   xxx,        xxx,                            SS_4,            SS_8,                 SS_8,                   SS_8,            xxx,             xxx,                  //SetSetting
//2,                                   xxx,        xxx,                            ITM_CPXRES0,     ITM_CPXRES1,          ITM_CPXRES1,            ITM_CPXRES1,     xxx,             xxx,                  //SetSetting
//2,                                   xxx,        xxx,                            ITM_SPCRES0,     ITM_SPCRES1,          ITM_SPCRES1,            ITM_SPCRES1,     xxx,             xxx,                  //SetSetting

    };


void Sett(int16_t grp) {
  int16_t ptr = -1;
  real_t realt;

  while(Settings[++ptr*(_numberOfGrps+2) + 0] != 0) {
    if(Settings[  ptr*(_numberOfGrps+2) + 1 + grp] != xxx) {

      #if defined(PC_BUILD) && (VERBOSE_LEVEL > 0)
        if(Settings[ptr*(_numberOfGrps+2) + 0] >= 101 && Settings[ptr*(_numberOfGrps+2) + 0] <= 121) {
          printf("\nSett1:%5d,  +0=%5d, +1=%5d, +1+grp=%5d ",ptr, Settings[ptr*(_numberOfGrps+2) + 0], Settings[ptr*(_numberOfGrps+2) + 1], Settings[ptr*(_numberOfGrps+2) + 1 + grp]);
        }
      #endif //PC_BUILD
      switch (Settings[ptr*(_numberOfGrps+2) + 0]) {
        case InputDefaultDataType : {fnInDefault                  (Settings[ptr*(_numberOfGrps+2) + 1 + grp]);break;}                       // InputDefaultDataType
        case SigFigNumberOfDigits : {fnDisplayFormatSigFig        (Settings[ptr*(_numberOfGrps+2) + 1 + grp]);break;}                       // SigFigNumberOfDigits
        case AllNumberOfDigits    : {fnDisplayFormatAll           (Settings[ptr*(_numberOfGrps+2) + 1 + grp]);break;}                       // AllNumberOfDigits
        case FixNumberOfDigits    : {fnDisplayFormatFix           (Settings[ptr*(_numberOfGrps+2) + 1 + grp]);break;}                       // FixNumberOfDigits
        case RNG                  : {exponentLimit      =         (Settings[ptr*(_numberOfGrps+2) + 1 + grp]);break;}                       // RNG
        case SDIGS                : {significantDigits  =         (Settings[ptr*(_numberOfGrps+2) + 1 + grp]);break;}                       // SDIGS
        case FDIGS                : {fractionDigits  =            (Settings[ptr*(_numberOfGrps+2) + 1 + grp]);break;}                       // FDIGS
        case HIDE                 : {exponentHideLimit  =         (Settings[ptr*(_numberOfGrps+2) + 1 + grp]);break;}                       // HIDE
        case DSTACK               : {displayStack       =         (Settings[ptr*(_numberOfGrps+2) + 1 + grp]);break;}                       // DSTACK
        case CACHEDDSTACK         : {cachedDisplayStack =         (Settings[ptr*(_numberOfGrps+2) + 1 + grp]);break;}                       // CACHEDDSTACK
        case ADM                  : {currentAngularMode =         (Settings[ptr*(_numberOfGrps+2) + 1 + grp]);break;}                       // ADM
        case IPGRP                : {grpGroupingLeft            = (Settings[ptr*(_numberOfGrps+2) + 1 + grp]);break;}                       // IPGRP
        case FPGRP                : {grpGroupingRight           = (Settings[ptr*(_numberOfGrps+2) + 1 + grp]);break;}                       // FPGRP
        case IPGRP1               : {grpGroupingGr1Left         = (Settings[ptr*(_numberOfGrps+2) + 1 + grp]);break;}                       // IPGRP1
        case IPGRP1x              : {grpGroupingGr1LeftOverflow = (Settings[ptr*(_numberOfGrps+2) + 1 + grp]);break;}                       // IPGRP1x
        case DenMaX               : {denMax                     = (Settings[ptr*(_numberOfGrps+2) + 1 + grp]);break;}                       // DenMaX
        case TVMIKnown            : {tvmIKnown                  = (Settings[ptr*(_numberOfGrps+2) + 1 + grp]) == 1 ? true : false;break;}   // TVMIKnown
        case TVMIChanges          : {tvmIChanges                = (Settings[ptr*(_numberOfGrps+2) + 1 + grp]) == 1 ? true : false;break;}   // TVMIChanges

        case RESERVED_VARIABLE_LX     :
        case RESERVED_VARIABLE_UX     :
        case RESERVED_VARIABLE_LY     :
        case RESERVED_VARIABLE_UY     :
        case RESERVED_VARIABLE_FV     :
        case RESERVED_VARIABLE_IPONA  :
        case RESERVED_VARIABLE_NPPER  :
        case RESERVED_VARIABLE_PMT    :
        case RESERVED_VARIABLE_PV     :
        case RESERVED_VARIABLE_PPERONA:
        case RESERVED_VARIABLE_CPERONA: {
            int32ToReal(Settings[ptr*(_numberOfGrps+2) + 1 + grp],&realt);
            reallocateRegister(Settings[ptr*(_numberOfGrps+2) + 0], dtReal34, 0, amNone);
            realToReal34(&realt, REGISTER_REAL34_DATA(Settings[ptr*(_numberOfGrps+2) + 0]));
            #if defined(PC_BUILD) && (VERBOSE_LEVEL > -1)
              printf("Sett1A Register %d = ",Settings[ptr*(_numberOfGrps+2) + 0]);
              printRegisterToConsole(Settings[ptr*(_numberOfGrps+2) + 0]," : ","\n");
            #endif //PC_BUILD
            break;
          }

        case 2 : {
            #if defined(PC_BUILD) && (VERBOSE_LEVEL > 0)
              printf("\nSett2:%5d:%5d\n",ptr,Settings[ptr*(_numberOfGrps+2) + 1 + grp]);
            #endif //PC_BUILD
            SetSetting (Settings[ptr*(_numberOfGrps+2) + 1 + grp]);
            break;
          }

        case 3: {
          #if defined(PC_BUILD) && (VERBOSE_LEVEL > 0)
            printf("\nSett3:%5d:%5d",ptr,Settings[ptr*(_numberOfGrps+2) + 1 + grp]);
          #endif //PC_BUILD
          forceSystemFlag (Settings[ptr*(_numberOfGrps+2) + 1 + grp], Settings[  ptr*(_numberOfGrps+2) + 1 ]);
          break;
          }

        case 4: {
          #if defined(PC_BUILD) && (VERBOSE_LEVEL > 0)
            printf("\nSett4:%5d:%5d",ptr,Settings[ptr*(_numberOfGrps+2) + 1 + grp]);
          #endif //PC_BUILD
          fnSetGapChar (Settings[ptr*(_numberOfGrps+2) + 1 + grp]);
          }
        default:break;
      }
    }
  }
  #if defined(PC_BUILD)
    printf("\n");
  #endif //PC_BUILD
}



  void fnSetHP35(uint16_t unusedButMandatoryParameter) {
    #if !defined(SAVE_SPACE_DM42_21_HP35) && !defined(SAVE_SPACE_DM42_24_PROFILES)
      getDateString(lastStateFileOpened);
      strcat(lastStateFileOpened,": HP35 defaults");
      fnKeyExit(0);                            //Clear pending key input
      fnClrMod(0);                             //Get out of NIM or BASE
      fnStoreConfig(35);                       //Store current config into R35

      fnClearStack(0);                         //Clear stack
      fnPi(0);                                 //Put pi on X

      Sett(_HP35);

      temporaryInformation = TI_NO_INFO;       //Clear any pending TI
      fnRefreshState();
      screenUpdatingMode = SCRUPD_AUTO;
      refreshScreen(160);
    #endif //SAVE_SPACE_DM42_21_HP35
  }


  void fnSetJM(uint16_t unusedButMandatoryParameter){
  #if !defined(SAVE_SPACE_DM42_24_PROFILES)
    fnDrop(NOPARAM);
    fnSquare(0);
    resetOtherConfigurationStuff(true);
    getDateString(lastStateFileOpened);
    strcat(lastStateFileOpened,": Jaco defaults");

    Sett(_JM);

    roundingMode = RM_HALF_UP;
    fnKeysManagement(ITM_RIBBON_C47);

    itemToBeAssigned = -MNU_EE;
    assignToMyMenu(6);
    itemToBeAssigned = ITM_op_j_pol;
    assignToMyMenu(11);
    itemToBeAssigned = -MNU_RIBBONS;
    assignToMyMenu(10);
    itemToBeAssigned = ITM_DREAL;
    assignToMyMenu(9);


    cachedDynamicMenu = 0;

    temporaryInformation = TI_NO_INFO;
    fnRefreshState();
    screenUpdatingMode = SCRUPD_AUTO;
    refreshScreen(161);
  #endif //#!SAVE_SPACE_DM42_24_PROFILES
  }


  void fnSetRJ(uint16_t unusedButMandatoryParameter){
  #if !defined(SAVE_SPACE_DM42_24_PROFILES)
    resetOtherConfigurationStuff(true);
    getDateString(lastStateFileOpened);
    strcat(lastStateFileOpened,": RJvM defaults");

    Sett(_RJ);

    fnKeyExit(0);
    fnDrop(NOPARAM);
    fnSquare(0);
    fnRefreshState();
    screenUpdatingMode = SCRUPD_AUTO;
    refreshScreen(165);
  #endif //!SAVE_SPACE_DM42_24_PROFILES
  }


  void _fnSetC47(uint16_t unusedButMandatoryParameter) {         //Reversing the HP35 settings to C47 defaults
      fnKeyExit(0);
      addItemToBuffer(ITM_EXIT1);
      getDateString(lastStateFileOpened);
      strcat(lastStateFileOpened,": C47 defaults");

      Sett(_C47);

      temporaryInformation = TI_NO_INFO;
      fnRefreshState();

      fnDrop(NOPARAM);
      fnDrop(NOPARAM);
      runFunction(ITM_SQUARE);
      screenUpdatingMode = SCRUPD_AUTO;
      refreshScreen(162);
  }


void fnSetC47(uint16_t unusedButMandatoryParameter) {
    fnKeyExit(0);
    addItemToBuffer(ITM_EXIT1);
    fnClrMod(0);
    _fnSetC47(0);               //Needs a reset in case for some reason RCLCFG R35 fails, then it resets to defaults
    fnRecallConfig(35);         //restores previously stored C47 config
    lastErrorCode = 0;
    fnRefreshState();
    screenUpdatingMode = SCRUPD_AUTO;
    refreshScreen(167);
  }



void fnClrMod(uint16_t unusedButMandatoryParameter) {        //clear input buffe
  #if defined(PC_BUILD)
    jm_show_comment("^^^^fnClrModa");
  #endif // PC_BUILD
  #if !defined(TESTSUITE_BUILD)
    resetKeytimers();  //JM
    clearSystemFlag(FLAG_FRACT);
    clearSystemFlag(FLAG_IRFRAC);
    clearSystemFlag(FLAG_INTING);
    clearSystemFlag(FLAG_SOLVING);
    programRunStop = PGM_STOPPED;

    if(calcMode == CM_NIM) {
      strcpy(aimBuffer, "+");
      fnKeyBackspace(0);
      //printf("|%s|\n", aimBuffer);
    }
    if(calcMode == CM_ASSIGN) {
      fnKeyExit(0);
    }
    fnKeyExit(0);           //Call fnkeyExit to ensure the correct home screen is brought up, if HOME is selected.
    fnKeyExit(0);           //Second time to ensure not only keys exited, but also modes
    popSoftmenu();
    lastIntegerBase = 0;
    decodedIntegerBase = 0;
    editingLiteralType = 0;
    temporaryInformation = TI_NO_INFO;
    lastErrorCode = 0;
    currentInputVariable = INVALID_VARIABLE;
    dispBase = 0;
    fnExitAllMenus(0);
    if(!checkHP) {
      fnDisplayStack(4);    //Restore to default DSTACK 4
    }
    else {                //Snap out of HP35 mode, and reset all setting needed for that
      _fnSetC47(0);
      fnRecallConfig(35);
      lastErrorCode = 0;
    }
    calcModeNormal();
    hourGlassIconEnabled = false;
    screenUpdatingMode = SCRUPD_AUTO;
    shiftF = false;
    shiftG = false;
    lastshiftF = false;
    lastshiftG = false;
    showShiftState();
    refreshModeGui();
    screenUpdatingMode &= ~SCRUPD_MANUAL_STATUSBAR;
    refreshStatusBar();
    screenUpdatingMode = SCRUPD_AUTO;
    forceSBupdate();
    refreshScreen(166);
    #if defined(PC_BUILD_TELLTALE)
      jm_show_calc_state("fnClrMod end: \n");
    #endif //PC_BUILD_TELLTALE
  #endif // !TESTSUITE_BUILD
}



void fnSetGapChar (uint16_t charParam) {
  //printf(">>>> charParam=%u %u \n", charParam, charParam & 16383);
  if((charParam & 49152) == 0) {                        //+0 for the left hand separator
    gapItemLeft = charParam & 16383;
  }
  else if((charParam & 49152) == 32768) {                        //+32768 for the right hand separator
    gapItemRight = charParam & 16383;
  }
  else if((charParam & 49152) == 49152) {                        //+49152 for the radix separator
    gapItemRadix = charParam & 16383;
  }
  //printf("LT=%s RT=%s RX=%s\n",Lt, Rt, Rx);
  //printf("Post: gapCharL0=%u gapCharL1=%u gapCharR0=%u gapCharR1=%u gapCharRx0=%u gapCharRx1%u  \n", (uint8_t)gapChar1Left[0], (uint8_t)gapChar1Left[1], (uint8_t)gapChar1Right[0], (uint8_t)gapChar1Right[1],  (uint8_t)gapChar1Radix[0], (uint8_t)gapChar1Radix[1]);
}


void fnSettingsDispFormatGrpL   (uint16_t param) {
  grpGroupingLeft = param;
}

void fnSettingsDispFormatGrp1Lo  (uint16_t param) {
  grpGroupingGr1LeftOverflow = param;
}

void fnSettingsDispFormatGrp1L  (uint16_t param) {
  grpGroupingGr1Left = param;
}

void fnSettingsDispFormatGrpR   (uint16_t param) {
  grpGroupingRight = param;
}

void fnMenuGapL (uint16_t unusedButMandatoryParameter) {
  #if !defined(TESTSUITE_BUILD)
    showSoftmenu(-MNU_GAP_L);
  #endif // ! TESTSUITE_BUILD
}

void fnMenuGapRX (uint16_t unusedButMandatoryParameter) {
  #if !defined(TESTSUITE_BUILD)
    showSoftmenu(-MNU_GAP_RX);
  #endif // ! TESTSUITE_BUILD
}

void fnMenuGapR (uint16_t unusedButMandatoryParameter) {
  #if !defined(TESTSUITE_BUILD)
    showSoftmenu(-MNU_GAP_R);
  #endif // ! TESTSUITE_BUILD
}






void fnIntegerMode(uint16_t mode) {
  if(shortIntegerMode != mode) {
    setSystemFlagChanged(SETTING_SINT_MODE);
  }
  shortIntegerMode = mode;
  fnRefreshState();
}



void fnWho(uint16_t unusedButMandatoryParameter) {
  temporaryInformation = TI_WHO;
 }



void fnVersion(uint16_t unusedButMandatoryParameter) {
  temporaryInformation = TI_VERSION;
}



void fnFreeMemory(uint16_t unusedButMandatoryParameter) {
  longInteger_t mem;

  liftStack();

  longIntegerInit(mem);
  uInt32ToLongInteger(getFreeRamMemory(), mem);
  convertLongIntegerToLongIntegerRegister(mem, REGISTER_X);
  longIntegerFree(mem);
  temporaryInformation = TI_BYTES;
}


void fnGetDmx(uint16_t unusedButMandatoryParameter) {
  longInteger_t dmx;

  liftStack();

  longIntegerInit(dmx);
  uInt32ToLongInteger(denMax, dmx);
  convertLongIntegerToLongIntegerRegister(dmx, REGISTER_X);
  longIntegerFree(dmx);
}


void fnGetRoundingMode(uint16_t unusedButMandatoryParameter) {
  longInteger_t rounding;

  liftStack();

  longIntegerInit(rounding);
  uInt32ToLongInteger(roundingMode, rounding);
  convertLongIntegerToLongIntegerRegister(rounding, REGISTER_X);
  longIntegerFree(rounding);
}



void fnSetRoundingMode(uint16_t RM) {
  roundingMode = RM;
}

// "enum rounding" does not match with the specification of WP 43s rounding mode.
// So you need roundingModeTable[roundingMode] rather than roundingMode
// to specify rounding mode in the real number functions.
TO_QSPI const enum rounding roundingModeTable[7] = {
  DEC_ROUND_HALF_EVEN, DEC_ROUND_HALF_UP, DEC_ROUND_HALF_DOWN,
  DEC_ROUND_UP, DEC_ROUND_DOWN, DEC_ROUND_CEILING, DEC_ROUND_FLOOR
};



void fnGetIntegerSignMode(uint16_t unusedButMandatoryParameter) {
  fnRecall(RESERVED_VARIABLE_ISM);
}



void fnGetWordSize(uint16_t unusedButMandatoryParameter) {
  longInteger_t wordSize;

  liftStack();

  longIntegerInit(wordSize);
  uInt32ToLongInteger(shortIntegerWordSize, wordSize);
  convertLongIntegerToLongIntegerRegister(wordSize, REGISTER_X);
  longIntegerFree(wordSize);
  temporaryInformation = TI_BITS;
}



void fnSetWordSize(uint16_t WS) {
  if(shortIntegerWordSize != WS) {
    setSystemFlagChanged(SETTING_SINT_WS);
    screenUpdatingMode &= ~SCRUPD_MANUAL_STATUSBAR;
  }

  bool_t reduceWordSize;
  if(WS == 0) {
    WS = 64;
  }

  reduceWordSize = (WS < shortIntegerWordSize);

  shortIntegerWordSize = WS;

  if(shortIntegerWordSize == 64) {
    shortIntegerMask    = -1;
  }
  else {
    shortIntegerMask    = ((uint64_t)1 << shortIntegerWordSize) - 1;
  }

  shortIntegerSignBit = (uint64_t)1 << (shortIntegerWordSize - 1);
  //printf("shortIntegerMask  =   %08x-%08x\n", (unsigned int)(shortIntegerMask>>32), (unsigned int)(shortIntegerMask&0xffffffff));
  //printf("shortIntegerSignBit = %08x-%08x\n", (unsigned int)(shortIntegerSignBit>>32), (unsigned int)(shortIntegerSignBit&0xffffffff));

  if(reduceWordSize) {
    // reduce the word size of integers on the stack
    for(calcRegister_t regist=REGISTER_X; regist<=getStackTop(); regist++) {
      if(getRegisterDataType(regist) == dtShortInteger) {
        *(REGISTER_SHORT_INTEGER_DATA(regist)) &= shortIntegerMask;
      }
    }

    // reduce the word size of integers in the L register
    if(getRegisterDataType(REGISTER_L) == dtShortInteger) {
      *(REGISTER_SHORT_INTEGER_DATA(REGISTER_L)) &= shortIntegerMask;
    }
  }

  fnRefreshState();                              //dr

}



void fnFreeFlashMemory(uint16_t unusedButMandatoryParameter) {
  longInteger_t flashMem;

  liftStack();

  longIntegerInit(flashMem);
  uInt32ToLongInteger(getFreeFlash(), flashMem);
  convertLongIntegerToLongIntegerRegister(flashMem, REGISTER_X);
  longIntegerFree(flashMem);
}



void fnBatteryVoltage(uint16_t unusedButMandatoryParameter) {
  real_t value;

  liftStack();

  #if defined(PC_BUILD)
    int32ToReal(3100, &value);
  #endif // PC_BUILD

  #if defined(DMCP_BUILD)
//    int32ToReal(get_vbat(), &value);
    int tmpVbat = get_vbat();
    int32ToReal(tmpVbat < vbatVIntegrated ? tmpVbat : vbatVIntegrated, &value);
  #endif // DMCP_BUILD

  temporaryInformation = TI_BATTV;
  value.exponent -= 3; // value = value / 1000
  convertRealToResultRegister(&value, REGISTER_X, amNone);
}



uint32_t getFreeFlash(void) {
  return 123456789u;
}



void fnGetSignificantDigits(uint16_t unusedButMandatoryParameter) {
  longInteger_t sigDigits;

  liftStack();

  longIntegerInit(sigDigits);
  uInt32ToLongInteger(significantDigits == 0 ? 34 : significantDigits, sigDigits);
  convertLongIntegerToLongIntegerRegister(sigDigits, REGISTER_X);
  longIntegerFree(sigDigits);
}



void fnSetSignificantDigits(uint16_t S) {
   significantDigits = S;
   if(significantDigits == 0) {
     significantDigits = 34;
   }
 }



void fnSetBaseNr(uint16_t S) {
   dispBase = S;
   if(dispBase == 1) {
     dispBase = 0;
   }
 }

void fnGetFractionDigits(uint16_t unusedButMandatoryParameter) {
  longInteger_t sigDigits;

  liftStack();

  longIntegerInit(sigDigits);
  uInt32ToLongInteger(fractionDigits == 0 ? 34 : fractionDigits, sigDigits);
  convertLongIntegerToLongIntegerRegister(sigDigits, REGISTER_X);
  longIntegerFree(sigDigits);
}



void fnSetFractionDigits(uint16_t S) {
   fractionDigits = S;
   if(fractionDigits == 0) {
     fractionDigits = 34;
   }
 }



void fnRoundingMode(uint16_t RM) {
  if(RM < nbrOfElements(roundingModeTable)) {
    roundingMode = RM;
    ctxtReal34.round = roundingModeTable[RM];
  }
  else {
    sprintf(errorMessage, commonBugScreenMessages[bugMsgValueFor], "fnRoundingMode", RM, "RM");
    strcat(errorMessage, "Must be from 0 to 6");
    displayBugScreen(errorMessage);
  }
}



void fnAngularMode(uint16_t am) {
  if(am != currentAngularMode) {
    setSystemFlagChanged(SETTING_AMODE);
  }
  currentAngularMode = am;
  fnRefreshState();
}



void fnFractionType(uint16_t unusedButMandatoryParameter) {
  #define STATE_offbc    0b0000  // B
  #define STATE_bc       0b0001  //1
  #define STATE_offabc   0b0010  // B
  #define STATE_abc      0b0011  //3
  #define STATE_offr_bc  0b0100  //12
//                         0101    C if b8==0 the b4=0
  #define STATE_offr_abc 0b0110  //14
//                         0111    C if b8==0 the b4=0
//                         1000  A
//                         1001
//                         1010  A
//                         1011
  #define STATE_exfr_bc  0b1100  //12
//                         1101
  #define STATE_exfr_abc 0b1110  //14
//                         1111
  #define STATE         ((getSystemFlag(FLAG_IRFRAC) ? 8:0) +  \
                         (getSystemFlag(FLAG_IRFRQ ) ? 4:0) +  \
                         (getSystemFlag(FLAG_PROPFR) ? 2:0) +  \
                         (getSystemFlag(FLAG_FRACT)  ? 1:0))
  uint8_t state = STATE;
  //printf("%u ",state);

  if(getSystemFlag(FLAG_FRCYC)) {
    switch(state) {
      case STATE_offbc       : state = STATE_bc;        break;
      case STATE_offabc      : state = STATE_abc;       break;
      case STATE_offr_bc     : state = STATE_exfr_bc;   break;
      case STATE_offr_abc    : state = STATE_exfr_abc;  break;

      case STATE_bc          : state = STATE_exfr_abc;  break;                    // 0b0001 -->
      case STATE_abc         : state = STATE_bc;        break;                    // 0b0011 -->
      case STATE_exfr_bc     : state = STATE_abc;       break;                    // 0b1100 -->
      case STATE_exfr_abc    : state = STATE_exfr_bc;   break;                    // 0b1110 -->
      default                : state = STATE_abc;       break;                    //
    }

    if((state & 8)) setSystemFlag(FLAG_IRFRAC); else clearSystemFlag(FLAG_IRFRAC);
    if((state & 4)) setSystemFlag(FLAG_IRFRQ ); else clearSystemFlag(FLAG_IRFRQ );
    if((state & 2)) setSystemFlag(FLAG_PROPFR); else clearSystemFlag(FLAG_PROPFR);
    if((state & 1)) setSystemFlag(FLAG_FRACT);  else clearSystemFlag(FLAG_FRACT);
    //printf("--> %u --> %u\n",state, STATE);
  }
  else {
    flipSystemFlag(getSystemFlag(FLAG_IRFRQ ) ? FLAG_IRFRAC : FLAG_FRACT);
  }
}


/* Confirmation messages */
TO_QSPI const confirmationTI_t confirmationTI[] = {
    {.item = ITM_DELALL,      .string = "Delete all?"                  },
    {.item = ITM_CLFALL,      .string = "Clear all flags?"             },
    {.item = ITM_DELPALL,     .string = "Delete all programs?"         },
    {.item = ITM_CLREGS,      .string = "Clear registers?"             },
    {.item = ITM_RESET,       .string = "Reset?"                       },
    {.item = ITM_DELBKUP,     .string = "Delete both backup files?"    },
    {.item = ITM_CLMALL,      .string = "Clear all user menus?"        },
    {.item = ITM_CLVALL,      .string = "Clear all user variables?"    },
    {.item = ITM_DELMALL,     .string = "Delete all user menus?"       },
    {.item = ITM_DELVALL,     .string = "Delete all user variables?"   },
    {.item = 0,               .string = "Are you sure?"                }          // Default TI for items requiring confirmation but not listed in this table
};

uint16_t getConfirmationTiId(void) {
  uint16_t id;
  uint16_t item = 0;
  for(id=0; id<LAST_ITEM; id++) {
    if(indexOfItems[id].func == confirmedFunction) {
      item = id;
      break;
    }
  }
  id = 0;
  while(confirmationTI[id].item != 0) {
    if(confirmationTI[id].item == item) {
      break;
    }
    id++;
  }
  return id;
}

void setConfirmationMode(void (*func)(uint16_t)) {
#if !defined(TESTSUITE_BUILD)
  previousCalcMode = calcMode;
  cursorEnabled = false;
  calcMode = CM_CONFIRMATION;
  clearSystemFlag(FLAG_ALPHA);
  confirmedFunction = func;
  temporaryInformation = TI_ARE_YOU_SURE;
  showSoftmenu(-MNU_YESNO);
#endif // !TESTSUITE_BUILD
}


void fnConfirmationYes(uint16_t unusedButMandatoryParameter) {
#if !defined(TESTSUITE_BUILD)
  if(calcMode == CM_CONFIRMATION) {
      calcMode = previousCalcMode;
      popSoftmenu();                // Pop MNU_YESNO
      confirmedFunction(CONFIRMED);
  }
#endif // !TESTSUITE_BUILD
}


 void fnConfirmationNo(uint16_t unusedButMandatoryParameter) {
#if !defined(TESTSUITE_BUILD)
  if(calcMode == CM_CONFIRMATION) {
      calcMode = previousCalcMode;
      popSoftmenu();                // Pop MNU_YESNO
  }
#endif // !TESTSUITE_BUILD
}


void fnRange(uint16_t R) {
  exponentLimit = R;
  if(exponentLimit < 99) exponentLimit = 99;
}



void fnGetRange(uint16_t unusedButMandatoryParameter) {
  longInteger_t range;

  liftStack();

  longIntegerInit(range);
  uInt32ToLongInteger(exponentLimit, range);
  convertLongIntegerToLongIntegerRegister(range, REGISTER_X);
  longIntegerFree(range);
}



void fnHide(uint16_t H) {
  exponentHideLimit = H;
  if(exponentHideLimit > 0 && exponentHideLimit < 12) {
    exponentHideLimit = 12;
  }
}



void fnGetHide(uint16_t unusedButMandatoryParameter) {
  longInteger_t range;

  liftStack();

  longIntegerInit(range);
  uInt32ToLongInteger(exponentHideLimit, range);
  convertLongIntegerToLongIntegerRegister(range, REGISTER_X);
  longIntegerFree(range);
}


void fnGetLastErr(uint16_t unusedButMandatoryParameter) {
  longInteger_t err;

  liftStack();

  longIntegerInit(err);
  uInt32ToLongInteger(previousErrorCode, err);
  convertLongIntegerToLongIntegerRegister(err, REGISTER_X);
  longIntegerFree(err);
}


void initSimEqMatABX(void) {
  matrixHeader_t *matrixHeader;

  allocateNamedVariable("Mat_A", dtReal34Matrix, REAL34_SIZE_IN_BLOCKS + TO_BLOCKS(sizeof(matrixHeader_t)));
  matrixHeader = getRegisterDataPointer(FIRST_NAMED_VARIABLE);
  matrixHeader->matrixRows = 1;
  matrixHeader->matrixColumns = 1;
  real34Zero(REAL34_MATRIX_ELEMENTS_AFTER_MATRIX_HEADER(matrixHeader));

  allocateNamedVariable("Mat_B", dtReal34Matrix, REAL34_SIZE_IN_BLOCKS + TO_BLOCKS(sizeof(matrixHeader_t)));
  matrixHeader = getRegisterDataPointer(FIRST_NAMED_VARIABLE + 1);
  matrixHeader->matrixRows = 1;
  matrixHeader->matrixColumns = 1;
  real34Zero(REAL34_MATRIX_ELEMENTS_AFTER_MATRIX_HEADER(matrixHeader));

  allocateNamedVariable("Mat_X", dtReal34Matrix, REAL34_SIZE_IN_BLOCKS + TO_BLOCKS(sizeof(matrixHeader_t)));
  matrixHeader = getRegisterDataPointer(FIRST_NAMED_VARIABLE + 2);
  matrixHeader->matrixRows = 1;
  matrixHeader->matrixColumns = 1;
  real34Zero(REAL34_MATRIX_ELEMENTS_AFTER_MATRIX_HEADER(matrixHeader));
}


void fnClAll(uint16_t confirmation) {
                                    #if defined(PC_BUILD) && (VERBOSE_LEVEL > -1)
                                      printf("fnClAll\n");
                                    #endif
  if(confirmation == NOT_CONFIRMED) {
    setConfirmationMode(fnClAll);
  }
  else {
    calcRegister_t regist;

    fnClPAll(CONFIRMED);  // Clears all the programs
    fnClSigma(CONFIRMED); // Clears and releases the memory of all statistical sums
    if(savedStatisticalSumsPointer != NULL) {
      freeC47Blocks(savedStatisticalSumsPointer, NUMBER_OF_STATISTICAL_SUMS * REAL_SIZE_IN_BLOCKS);
    }

    // Clear local registers
    allocateLocalRegisters(0);

    // Clear registers including stack, I, J, K, L, MNP QRS, EFHH OUVW, saved stack and temp
    for(regist=FIRST_GLOBAL_REGISTER; regist<=LAST_GLOBAL_REGISTER; regist++) {
      clearRegister(regist);
    }
    thereIsSomethingToUndo = false;

    // Clear user menus
    fnExitAllMenus(NOPARAM);
    fnDeleteUserMenus(CONFIRMED);             // Delete all user menus and user menus assignments

    if(isR47FAM) {
      fnRESET_MyM(ITM_RIBBON_R47);            // Reset Menu MyMenu
    }
    else {
      fnRESET_MyM(ITM_RIBBON_C47);            // Reset Menu MyMenu
    }

    fnRESET_Mya();                            // Reset Menu MyAlpha
    #if !defined(TESTSUITE_BUILD)
                                    #if defined(PC_BUILD) && (VERBOSE_LEVEL > -1)
                                      printf("createHOME\n");
                                    #endif
      createHOME();                             // Reset Menu HOME
                                    #if defined(PC_BUILD) && (VERBOSE_LEVEL > -1)
                                      printf("createPFN CL\n");
                                    #endif
      createPFN();                              // Reset Menu P.FN
    #endif // !TESTSUITE_BUILD

    // Clear All Key assignments
    fnKeysManagement(USER_KRESET);
    initUserKeyArgument();

    // Delete named variables
    fnDeleteAllVariables(CONFIRMED);

    // Clear global flags
    fnClFAll(CONFIRMED);

    temporaryInformation = TI_RESET;
    if(programRunStop == PGM_WAITING) {
      programRunStop = PGM_STOPPED;
    }
  }
}



void addTestPrograms(void) {
  uint32_t numberOfBytesUsed, numberOfBytesForTheTestPrograms = TO_BYTES(TO_BLOCKS(19000));

  resizeProgramMemory(TO_BLOCKS(numberOfBytesForTheTestPrograms));
  firstDisplayedStep            = beginOfProgramMemory;
  currentStep                   = beginOfProgramMemory;
  currentLocalStepNumber        = 1;
  firstDisplayedLocalStepNumber = 0;

  #if defined(DMCP_BUILD)
    if(f_open(ppgm_fp, "testPgms.bin", FA_READ) != FR_OK) {
      *(beginOfProgramMemory)     = 255; // .END.
      *(beginOfProgramMemory + 1) = 255; // .END.
      firstFreeProgramByte = beginOfProgramMemory;
      freeProgramBytes = numberOfBytesForTheTestPrograms - 2;
    }
    else {
      UINT bytesRead;
      f_read(ppgm_fp, &numberOfBytesUsed,   sizeof(numberOfBytesUsed), &bytesRead);
      f_read(ppgm_fp, beginOfProgramMemory, numberOfBytesUsed,         &bytesRead);
      f_close(ppgm_fp);

      firstFreeProgramByte = beginOfProgramMemory + (numberOfBytesUsed - 2);
      freeProgramBytes = numberOfBytesForTheTestPrograms - numberOfBytesUsed;
    }

    scanLabelsAndPrograms();
  #else // !DMCP_BUILD
    FILE *testPgms;

    testPgms = fopen("res/testPgms/testPgms.bin", "rb");
    if(testPgms == NULL) {
      printf("Cannot open file res/testPgms/testPgms.bin\n");
      *(beginOfProgramMemory)     = 255; // .END.
      *(beginOfProgramMemory + 1) = 255; // .END.
      firstFreeProgramByte = beginOfProgramMemory;
      freeProgramBytes = numberOfBytesForTheTestPrograms - 2;
    }
    else {
      ignoreReturnedValue(fread(&numberOfBytesUsed, 1, sizeof(numberOfBytesUsed), testPgms));
      printf("%u bytes\n", numberOfBytesUsed);
      if(numberOfBytesUsed > numberOfBytesForTheTestPrograms) {
        printf("Increase allocated memory for programs! File config.c 1st line of function addTestPrograms\n");
        fclose(testPgms);
        exit(0);
      }
      ignoreReturnedValue(fread(beginOfProgramMemory, 1, numberOfBytesUsed, testPgms));
      fclose(testPgms);

      firstFreeProgramByte = beginOfProgramMemory + (numberOfBytesUsed - 2);
      freeProgramBytes = numberOfBytesForTheTestPrograms - numberOfBytesUsed;
    }

    printf("freeProgramBytes = %u\n", freeProgramBytes);

    scanLabelsAndPrograms();
    #if !defined(TESTSUITE_BUILD)
      leavePem();
    #endif // !TESTSUITE_BUILD
    printf("freeProgramBytes = %u\n", freeProgramBytes);
    //listPrograms();
    //listLabelsAndPrograms();
  #endif // DMCP_BUILD
}



void restoreStats(void){
  if(lrChosen !=65535) {
    lrChosen = lrChosenHistobackup;
  }
  if(lrSelection !=65535) {
    lrSelection = lrSelectionHistobackup;
  }
  strcpy(statMx,"STATS");
  lrSelectionHistobackup = 65535;
  lrChosenHistobackup = 65535;
  calcSigma(0);
}


    typedef struct {              //JM VALUES DEMO
      uint8_t  itemType;
      uint16_t  count;
      char     *itemName;
    } numberstr;

    TO_QSPI const numberstr indexOfStrings[] = {
      {0,10, "Reg 11,12 & 13 have: The 3 cubes = 3."},
      {1,11, "569936821221962380720"},
      {1,12, "-569936821113563493509"},
      {1,13, "-472715493453327032"},

      {0,14, "Reg 15, 16 & 17 have: The 3 cubes = 42."},
      {1,15, "-80538738812075974"},
      {1,16, "80435758145817515"},
      {1,17, "12602123297335631"},

      {0,18, "37 digits of pi, Reg19 / Reg20."},
      {1,19, "2646693125139304345"},
      {1,20, "842468587426513207"},

      {0,21, "Primes: Carol"},
      {1,22, "18014398241046527"},

      {0,23, "Primes: Kynea"},
      {1,24, "18446744082299486207"},

      {0,25, "Primes: repunit"},
      {1,26, "7369130657357778596659"},

      {0,27, "Primes: Woodal"},
      {1,28, "195845982777569926302400511"},

      {0,29, "Primes: Woodal"},
      {1,30, "4776913109852041418248056622882488319"},

      {0,31, "Primes: Woodal"},
      {1,32, "225251798594466661409915431774713195745814267044878909733007331390393510002687"},

      {0,33, "pi.(10^100) (101 digits) longinteger"},
      {1,34, "31415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170680"},

      //35 reserved for HP35/C47 swaps config save

      {0,36, "100 primes' product 2x3x...x541"},
      {1,37, "4711930799906184953162487834760260422020574773409675520188634839616415335845034221205289256705544681972439104097777157991804380284218315038719444943990492579030720635990538452312528339864352999310398481791730017201031090"},

      {0,38, "Heart:16" STD_CROSS "(sin(t)^3)+i" STD_CROSS "(13" STD_CROSS "cos(t)-5" STD_CROSS "cos(2" STD_CROSS "t)-2" STD_CROSS "cos(3" STD_CROSS "t)-cos(4" STD_CROSS "t))"},

//temporary test values

//      {0,39, "Carmichael number 1"},
//      {1,40, "2090544338452850336677669058614524913253916538940295276440923713893889746693477362455627182089820344446945934218698530379666521"},
//             // m = 117278425593238765610521319698123670218170
//
//      {0,41, "Carmichael number 2"},
//      {1,42, "1114914577280124301342981138310574878929503042734089"},
//             // m = 9510693751439811
//
//      {0,43, "Carmichael number 3"},
//      {1,44, "3215031751"},
//            // 151 × 21291601
//            // 151 × 751 × 28351

    };





    TO_QSPI const numberstr indexOfMsgs[] = {
      {0,USER_C47,      "C47: Classic single shift (DM42/DM42n base)"  },
      {0,USER_R47f_g,   "R47v0 L.Shift is " STD_f   ", R.Shift is " STD_g },
      {0,USER_R47bk_fg, "R47v3 L.Shift is " STD_BOX ", R.Shift is " STD_fg },
      {0,USER_R47fg_bk, "R47v1 L.Shift is " STD_fg  ", R.Shift is " STD_BOX},
      {0,USER_R47fg_g,  "R47v2 L.Shift is " STD_fg  ", R.Shift is " STD_g  },
      {0,USER_DM42,    "DM42: Final Compatibility layout"                },
      {0,USER_HRESET,  "HOME menu reset to default"                      },
      {0,USER_PRESET,  "P.FN menu reset to default"                      },
      {0,USER_KRESET,  "Key assignments cleaned"                         },
      {0,USER_MRESET,  "MyMenu menu cleaned"                             },
      {0,USER_ARESET,  "My" STD_alpha " menu cleaned"                    },
      {0,ITM_RIBBON_ENG  , "MyMenu primary F-key engineering ribbon"     },
      {0,ITM_RIBBON_FIN  , "MyMenu primary F-key financial ribbon"       },
      {0,ITM_RIBBON_FIN2 , "MyMenu primary F-key financial ribbon 2"     },
      {0,ITM_RIBBON_CPX  , "MyMenu primary F-key complex ribbon"         },
      {0,ITM_RIBBON_SAV  , "MyMenu primary F-key save/load ribbon"       },
      {0,ITM_RIBBON_SAV2 , "MyMenu primary F-key save/load ribbon 2"     },
      {0,ITM_RIBBON_C47  , "MyMenu primary C47 F-key ribbon"             },
      {0,ITM_RIBBON_C47PL, "MyMenu primary C47 Plus F-key ribbon"        },
      {0,ITM_RIBBON_R47  , "MyMenu primary R47 F-key ribbon"             },
      {0,ITM_RIBBON_R47PL, "MyMenu primary R47 Plus F-key ribbon"        },
      {0,100,"Error List"}
    };



uint16_t searchMsg(uint16_t idStr) {
  uint_fast16_t n = nbrOfElements(indexOfMsgs);
  uint_fast16_t i;
  for(i=0; i<n; i++) {
    if(indexOfMsgs[i].count == idStr) {
       break;
    }
  }
  return i;
}


void fnShowVersion(uint16_t option) {  //KEYS VERSION LOADED
  strcpy(errorMessage, indexOfMsgs[searchMsg(option)].itemName);
  temporaryInformation = TI_KEYS;
}


void fnResetTVM (uint16_t unusedButMandatoryParameter) {
  Sett(_TVM);
}



void defaultStatusBar(void) {
  Sett(_DefltSB);
}


void resetOtherConfigurationStuff(bool_t allowUserKeys) {
  cancelFilename = true;
  lastStateFileOpened[0]=0;

  firstGregorianDay = 2361222 /* 14 Sept 1752 */;
  displayFormat = DF_ALL;
  displayFormatDigits = 3;
  timeDisplayFormatDigits = 0;

  shortIntegerMode = SIM_2COMPL;                              //64:2
  fnSetWordSize(64);
  roundingMode = RM_HALF_EVEN;
  pcg32_srandom(0x1963073019931121ULL, 0x1995062319981019ULL); // RNG initialisation
  exponentHideLimit = 0;
  lastCenturyHighUsed = 0;
  lrSelection = CF_LINEAR_FITTING;
  lrSelectionUndo = lrSelection;                               //Not saved in file, but reset

  IrFractionsCurrentStatus = CF_NORMAL;
  if(allowUserKeys) {
    Norm_Key_00.func  = Norm_Key_00_item_in_layout;               //JM NORM MODE SIGMA REPLACEMENT KEY
    Norm_Key_00.funcParam[0] = 0;
    Norm_Key_00.used = false;
  }
  Input_Default =  ID_43S;
  displayStackSHOIDISP = 2;            //See if the refresh is needed. fnShoiXRepeats(2); //displayStackSHOIDISP
  bcdDisplaySign = BCDu;
  DRG_Cycling = 0;
  DM_Cycling = 0;
  LongPressM = RBX_M1234;
  LongPressF = RBX_F124;
  lastIntegerBase = 0;
  decodedIntegerBase = 0;
  timeLastOp = 0;
  timeLastOp0 = 0;
  timeLastOp1 = 0;
  dispBase = 0;

  #if !defined(TESTSUITE_BUILD) && !defined(GENERATE_CATALOGS)
    lastI = 0;
    lastI = 0;
    lastFunc    = 0;
    lastParam   = 0;
    lastTemp[0] = 0;
  #endif // !TESTSUITE_BUILD && !GENERATE_CATALOGS

  blockMonitoring = false;
}


void setLongPressFg(int calcModel0, int16_t menuItem) {
  int16_t keyCode = 9999;
  switch(calcModel0) {
    case USER_R47f_g:
           keyCode = 9999;
           break;
    case USER_R47bk_fg:
           keyCode = 11;
           break;
    case USER_R47fg_bk:
           keyCode = 10;
           break;
    case USER_R47fg_g:
           keyCode = 10;
           break;
    case USER_C47:
           keyCode = 27;
           break;
    case USER_DM42:
           keyCode = 27;
           break;
    default:;
  }
  if(keyCode != 9999) {
    calcKey_t *key_assign_test = kbd_usr + keyCode;
    key_assign_test->fShifted = menuItem;
  }
}



typedef struct {
  char str2[180];
} nstr2;
TO_QSPI const nstr2 msg2[] = {
      { "\xff\xf8\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\xff\xf8" }
   };


void fnReset(uint16_t confirmation) {
  doFnReset(confirmation, doNotLoadAutoSav);
}

void doFnReset(uint16_t confirmation, bool_t autoSav) {
                                    #if defined(PC_BUILD) && (VERBOSE_LEVEL > -1)
                                      printf("doFnReset\n");
                                    #endif
   if(confirmation == NOT_CONFIRMED) {
    setConfirmationMode(fnReset);
  }
  else {
    void *memPtr;

    if(ram == NULL) {
      ram = (uint32_t *)malloc(TO_BYTES(RAM_SIZE_IN_BLOCKS));
    }
    memset(ram, 0, TO_BYTES(RAM_SIZE_IN_BLOCKS));

    #if !defined(DMCP_BUILD) || !defined(OLD_HW)
      if(globalRegister == NULL) {
        globalRegister = malloc(sizeof(registerHeader_t) * NUMBER_OF_GLOBAL_REGISTERS);
        freeMemoryRegions = malloc(sizeof(freeMemoryRegion_t) * MAX_FREE_REGIONS);
      }
    #endif // DMCP_BUILD && OLD_HW

    freeMemoryRegions[0].blockAddress = TO_C47MEMPTR(ram + allReservedVariables[LAST_RESERVED_VARIABLE - FIRST_RESERVED_VARIABLE].header.pointerToRegisterData + REAL34_SIZE_IN_BLOCKS);
    freeMemoryRegions[0].sizeInBlocks = RAM_SIZE_IN_BLOCKS - freeMemoryRegions[0].blockAddress - 1; // - 1: one block for an empty program

    numberOfFreeMemoryRegions = 1;

    #if !defined(DMCP_BUILD)
      numberOfAllocatedMemoryRegions = 0;
    #endif // !DMCP_BUILD

    if(tmpString == NULL) {
      #if defined(DMCP_BUILD)
         tmpString        = aux_buf_ptr();   // 2560 byte buffer provided by DMCP
         errorMessage     = write_buf_ptr(); // 4096 byte buffer provided by DMCP
       #else // !DMCP_BUILD
         tmpString        = (char *)malloc(TMP_STR_LENGTH);    // 2560
         errorMessage     = (char *)malloc(WRITE_BUFFER_LEN);  // 4096
       #endif // DMCP_BUILD

                                                                              // errorMessage     from    0 to (4095       )
       aimBuffer        = errorMessage + ERROR_MESSAGE_LENGTH;   // + 512     // aimBuffer        from  512 to (512  + 1024) or 1536
       nimBufferDisplay = aimBuffer + AIM_BUFFER_LENGTH;         // +1024     // nimBufferDisplay from 1536 to (1536 +  200) or 1736
       tamBuffer        = nimBufferDisplay + NIM_BUFFER_LENGTH;  // + 200     // tamBuffer        from 1736 to (1736 +   32) or 1768
                                          // TAM_BUFFER_LENGTH   // +  32

       tmpStringLabelOrVariableName = tmpString + 1000;
    }
    memset(tmpString,        0, TMP_STR_LENGTH);
    memset(errorMessage,     0, ERROR_MESSAGE_LENGTH);
    memset(aimBuffer,        0, AIM_BUFFER_LENGTH);
    memset(nimBufferDisplay, 0, NIM_BUFFER_LENGTH);
    memset(tamBuffer,        0, TAM_BUFFER_LENGTH);

    // Empty program initialization
    beginOfProgramMemory          = (uint8_t *)(ram + (RAM_SIZE_IN_BLOCKS - 1)); // Last block of RAM
    currentStep                   = beginOfProgramMemory;
    firstFreeProgramByte          = beginOfProgramMemory + 2;
    firstDisplayedStep            = beginOfProgramMemory;
    firstDisplayedLocalStepNumber = 0;
    labelList                     = NULL;
    programList                   = NULL;
    *(beginOfProgramMemory + 0) = (ITM_END >> 8) | 0x80;
    *(beginOfProgramMemory + 1) =  ITM_END       & 0xff;
    *(beginOfProgramMemory + 2) = 255; // .END.
    *(beginOfProgramMemory + 3) = 255; // .END.
    freeProgramBytes            = 0;

    scanLabelsAndPrograms();

    // "Not found glyph" initialization
    if(glyphNotFound.data == NULL) {
      glyphNotFound.data = malloc(38);
    }
    //xcopy(glyphNotFound.data, "\xff\xf8\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\x80\x08\xff\xf8", 38);
    xcopy(glyphNotFound.data, msg2[0].str2, 38);

    // Initialization of user key assignments
    xcopy(kbd_usr, kbd_std, sizeof(kbd_std));
    //setLongPressFg(calcModel, (calcModel == USER_R47bk_fg ? -MNU_MyMenu : -MNU_HOME));
    // initialize 9 real34 reserved variables: ACC, ↑Lim, ↓Lim, FV, i%/a, NPPER, PPER/a, PMT, and PV
    for(int i=VAR_NO_ACC; i<=VAR_NO_CPERONA; i++) {
      real34Zero((real34_t *)TO_PCMEMPTR(allReservedVariables[i].header.pointerToRegisterData));
    }

    // initialize 1 long integer reserved variables: GRAMOD
    strLgIntHeader_t *ptr = TO_PCMEMPTR(allReservedVariables[VAR_NO_GRAMOD].header.pointerToRegisterData);
    #if defined(OS64BIT)
      (ptr++)->dataMaxLengthInBlocks = TO_BLOCKS(8);
      *(int64_t *)ptr = 0;
    #else // !OS64BIT
      (ptr++)->dataMaxLengthInBlocks = TO_BLOCKS(4);
      *(int32_t *)ptr = 0;
    #endif // OS64BIT


    // initialize the global registers
    #if defined(DMCP_BUILD) && defined(OLD_HW)
      memset(globalRegister, 0, sizeof(globalRegister));
    #endif // DMCP_BUILD && OLD_HW
    for(calcRegister_t regist=FIRST_GLOBAL_REGISTER; regist<=LAST_GLOBAL_REGISTER; regist++) {
      setRegisterDataType(regist, dtReal34, amNone);
      memPtr = allocC47Blocks(REAL34_SIZE_IN_BLOCKS);
      setRegisterDataPointer(regist, memPtr);
      real34Zero(memPtr);
    }

    // Clear global flags
    memset(globalFlags, 0, sizeof(globalFlags));

    // allocate space for the local register list
    allSubroutineLevels.numberOfSubroutineLevels = 1;
    currentSubroutineLevelData = allocC47Blocks(3);
    allSubroutineLevels.ptrToSubroutineLevel0Header = TO_C47MEMPTR(currentSubroutineLevelData);
    currentReturnProgramNumber = 0;
    currentReturnLocalStep = 0;
    currentNumberOfLocalRegisters = 0; // No local register
    currentNumberOfLocalFlags = 0; // No local flags
    currentSubroutineLevel = 0;
    currentPtrToNextLevel = C47_NULL;
    currentPtrToPreviousLevel = C47_NULL;
    currentLocalFlags = NULL;
    currentLocalRegisters = NULL;

    // allocate space for the named variable list
    numberOfNamedVariables = 0;
    allNamedVariables = NULL;

    initSimEqMatABX();

    #if !defined(TESTSUITE_BUILD)
      matrixIndex = INVALID_VARIABLE; // Unset matrix index
    #endif // !TESTSUITE_BUILD


    #if defined(PC_BUILD)
      debugWindow = DBG_REGISTERS;
    #endif // PC_BUILD

    decContextDefault(&ctxtReal34, DEC_INIT_DECQUAD);

    decContextDefault(&ctxtReal4, DEC_INIT_DECSINGLE);
    ctxtReal4.digits = 6;
    ctxtReal4.traps  = 0;

    decContextDefault(&ctxtReal39, DEC_INIT_DECQUAD);
    ctxtReal39.digits = 39;
    ctxtReal39.emax   = 999999;
    ctxtReal39.emin   = -999999;
    ctxtReal39.traps  = 0;

    decContextDefault(&ctxtReal51, DEC_INIT_DECQUAD);
    ctxtReal51.digits = 51;
    ctxtReal51.emax   = 999999;
    ctxtReal51.emin   = -999999;
    ctxtReal51.traps  = 0;

    decContextDefault(&ctxtReal75, DEC_INIT_DECQUAD);
    ctxtReal75.digits = 75;
    ctxtReal75.emax   = 999999;
    ctxtReal75.emin   = -999999;
    ctxtReal75.traps  = 0;

    resetOtherConfigurationStuff(true);

    statisticalSumsPointer = NULL;
    savedStatisticalSumsPointer = NULL;
    statisticalSumsUpdate = true;
    lrChosen    = 0;
    lrChosenUndo = 0;
    lastPlotMode = PLOT_NOTHING;
    plotSelection = 0;
    drawHistogram = 0;
    realZero(&SAVED_SIGMA_LASTX);
    realZero(&SAVED_SIGMA_LASTY);
    SAVED_SIGMA_lastAddRem = SIGMA_NONE;

    plotStatMx[0] = 0;
    regStatsXY = INVALID_VARIABLE;
    real34Zero(&loBinR);
    real34Zero(&nBins );
    real34Zero(&hiBinR);
    histElementXorY = -1;


    x_min = -10;
    x_max = 10;
    y_min = 0;
    y_max = 1;



    systemFlags0 = 0;
    systemFlags1 = 0;
    clearScreen(202); // implicit forceSBupdate();

    Sett(_Reset);
    //Statusbar default setup   DATE noTIME noCR noANGLE [ADM] FRAC INT MATX TVM CARRY noSS WATCH SERIAL PRN BATVOLT noSHIFTR

    configCommon(CFG_DFLT);

    hourGlassIconEnabled = false;
    watchIconEnabled = false;
    serialIOIconEnabled = false;
    printerIconEnabled = false;
    thereIsSomethingToUndo = false;
    pemCursorIsZerothStep = true;
    tam.alpha = false;
    fnKeyInCatalog = false;
    shiftF = false;
    shiftG = false;
    lastshiftF = false;
    lastshiftG = false;
    secTick1 = false;
    halfSecTick2 = false;
    halfSecTick3 = false;
    skippedStackLines = false;
    iterations = false;
    explicitTaylorIterVisibilitySelection = false;
    updateOldConstants = false;
    programRunStop = PGM_STOPPED;

    ctxtReal34.round = DEC_ROUND_HALF_EVEN;

    initFontBrowser();
    currentAsnScr = 1;
    currentFlgScr = NO_SCREEN;
    lastFlgScr    = NO_SCREEN;
    currentRegisterBrowserScreen = 9999;

    memset(softmenuStack, 0, sizeof(softmenuStack)); // This works because the ID of MyMenu is 0
    menuPageNumber = 1;                              // Set default menu page number for OPENM

    aimBuffer[0] = 0;
    lastErrorCode = 0;
    previousErrorCode = 0;

    #if !defined(TESTSUITE_BUILD)
      resetAlphaSelectionBuffer();
    #endif // !TESTSUITE_BUILD

    #if defined(TESTSUITE_BUILD)
      calcMode = CM_NORMAL;
    #else // TESTSUITE_BUILD
      if(calcMode == CM_MIM) {
        mimFinalize();
      }

      clearScreen(10); //WE HAVE TO FIND THE BEST PLACE FOR A FULL SCREEN CLEAR, JUST BEFORE THE CALCULATOR STARTS
      calcModeNormal();
    #endif // !TESTSUITE_BUILD

    #if defined(PC_BUILD) || defined(TESTSUITE_BUILD)
      debugMemAllocation = true;
      forceTamAlpha = false;
      deadKey = 0;
    #endif // PC_BUILD || TESTSUITE_BUILD


    tam.mode = 0;
    catalog = CATALOG_NONE;
    memset(lastCatalogPosition, 0, NUMBER_OF_CATALOGS * sizeof(lastCatalogPosition[0]));
    lastDenominator = 4;
    temporaryInformation = TI_RESET;

    currentInputVariable = INVALID_VARIABLE;
    currentMvarLabel = INVALID_VARIABLE;
    lastKeyCode = 0;
    entryStatus = 0;
    lastUserMode = false;   //used in btnReleased and btnFnReleased
    lastItem = 0;           //used in btnReleased, for CM_ASN_BROWSER and SHOW/SCREENDUMP

    memset(userMenuItems,  0, sizeof(userMenuItem_t) * 18);
    memset(userAlphaItems, 0, sizeof(userMenuItem_t) * 18);
    userMenus = NULL;
    numberOfUserMenus = 0;
    currentUserMenu = 0;

    initUserKeyArgument();

                                    #if defined(PC_BUILD) && (VERBOSE_LEVEL > -1)
                                      printf("fnClearMenu\n");
                                    #endif
    fnClearMenu(NOPARAM);

    #if !defined(TESTSUITE_BUILD)
      calcModeNormal();
      if(getSystemFlag(FLAG_BASE_HOME)) showSoftmenu(-MNU_HOME); //JM Reset to BASE MENU HOME;
    #endif // !TESTSUITE_BUILD

    showRegis = 9999;                                          //JMSHOW
    overrideShowBottomLine = 0;

    graph_reset();

    ListXYposition = 0;

    fnClrMod(0);


    displayAIMbufferoffset = 0;
    T_cursorPos = 0;
    yMultiLineEdOffset = 0;
    xMultiLineEdOffset = 0;
    current_cursor_x = 0;
    current_cursor_y = 0;
    lastT_cursorPos = 0;


                                    #if defined(PC_BUILD) && (VERBOSE_LEVEL > -1)
                                      printf("Doing A.RESET, M.RESET & K.RESET\n");
                                    #endif
    //    calcMode = CM_BUG_ON_SCREEN; this also removes the false start on MyMenu error

                                    #if defined(PC_BUILD) && (VERBOSE_LEVEL > -1)
                                      printf("USER_PRESET\n");
                                    #endif
    fnKeysManagement(USER_PRESET);                                      //JM USER
                                   #if defined(PC_BUILD) && (VERBOSE_LEVEL > -1)
                                     printf("USER_HRESET\n");
                                   #endif
    fnKeysManagement(USER_HRESET);                                      //JM USER
                                   #if defined(PC_BUILD) && (VERBOSE_LEVEL > -1)
                                     printf("USER_ARESET\n");
                                   #endif
    fnKeysManagement(USER_ARESET);                                      //JM USER
                                   #if defined(PC_BUILD) && (VERBOSE_LEVEL > -1)
                                     printf("USER_MRESET\n");
                                   #endif
    fnKeysManagement(USER_MRESET);                                      //JM USER

    if(isR47FAM) {
                                   #if defined(PC_BUILD) && (VERBOSE_LEVEL > -1)
                                     printf("USER_MR47\n");
                                   #endif
      fnKeysManagement(ITM_RIBBON_R47);                  // Reset Menu MyMenu Ribbon
    }
    else {
                                   #if defined(PC_BUILD) && (VERBOSE_LEVEL > -1)
                                     printf("USER_MC47\n");
                                   #endif
      fnKeysManagement(ITM_RIBBON_C47);                  // Reset Menu MyMenu Ribbon
    }

    #if !defined(TESTSUITE_BUILD)
      showSoftmenu(-MNU_MyMenu);                                   //this removes the false start on MyMenu error
    #endif // !TESTSUITE_BUILD
                                   #if defined(PC_BUILD) && (VERBOSE_LEVEL > -1)
                                     printf("fnKeysManagement\n");
                                   #endif
    fnKeysManagement(USER_KRESET);                                      //JM USER
    temporaryInformation = TI_NO_INFO;
    screenUpdatingMode = SCRUPD_AUTO;
    refreshScreen(163);

    // The following lines are test data
    #if !defined(SAVE_SPACE_DM42_14)
                                   #if defined(PC_BUILD) && (VERBOSE_LEVEL > -1)
                                     printf("addTestPrograms\n");
                                   #endif
      addTestPrograms();
    #endif // !SAVE_SPACE_DM42_14

    // Equation formulae
    allFormulae = NULL;
    numberOfFormulae = 0;
    currentFormula = 0;

    currentSolverStatus = 0;
    currentSolverProgram = 0xffffu;
    currentSolverVariable = INVALID_VARIABLE;
    currentSolverNestingDepth = 0;

    graphVariabl1 = 0;

    // Timer application
    timerCraAndDeciseconds = 0x80u;
    timerValue             = 0u;
    timerStartTime         = TIMER_APP_STOPPED;
    timerTotalTime         = 0u;

    #if (DEBUG_PANEL == 1)
      debugWindow = DBG_REGISTERS;
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkHexaString), false);
      refreshDebugPanel();
    #endif // (DEBUG_PANEL == 1)

    #if defined(DMCP_BUILD)
      //Check and update current power status (USB / LOWBAT)
      checkBattery();
    #endif // DMCP_BUILD


                                   #if defined(PC_BUILD) && (VERBOSE_LEVEL > -1)
                                     printf("Populate test data\n");
                                   #endif
    //JM TEMPORARY TEST DATA IN REGISTERS
    uint16_t n = nbrOfElements(indexOfStrings);
    for(uint16_t i=0; i<n; i++) {
      if(indexOfStrings[i].itemType == 0) {
        fnStrtoX(indexOfStrings[i].itemName);
      }
      else if(indexOfStrings[i].itemType == 1) {
        fnStrInputLongint(indexOfStrings[i].itemName);
      }
      fnStore(indexOfStrings[i].count);
      fnDrop(NOPARAM);
    }

                                   #if defined(PC_BUILD) && (VERBOSE_LEVEL > -1)
                                     printf("version\n");
                                   #endif
    #if !defined(TESTSUITE_BUILD)
      runFunction(ITM_VERS);
    #endif // !TESTSUITE_BUILD


    //Autoloading of C47Auto.sav
    #if defined(DMCP_BUILD)
      if(autoSav == loadAutoSav) {
        fnLoadAuto();
      }
    #endif // DMCP_BUILD

    #if !defined(TESTSUITE_BUILD)
      showSoftmenuCurrentPart();
    #endif // !TESTSUITE_BUILD
    doRefreshSoftMenu = true;     //jm dr
    screenUpdatingMode = SCRUPD_AUTO;
    refreshScreen(164);

    fnClearFlag(FLAG_USER);
    fnRefreshState();
  }
}



#ifdef DMCP_BUILD

  void dmcpResetAutoOff(void) {
    // Key is ready -> clear auto off timer
    if(!key_empty() || !emptyKeyBuffer() || ((calcMode == CM_TIMER) && timerStartTime != TIMER_APP_STOPPED) || !getSystemFlag(FLAG_AUTOFF) || getSystemFlag(FLAG_RUNTIM) || programRunStop == PGM_RUNNING || (nextTimerRefresh != 0)) {
      reset_auto_off();
    }
  }


  int16_t loop=0;
  int16_t loop2=0;
  int updateVbatIntegrated(bool_t minutePulse) {
    if(getSystemFlag(FLAG_USB)) {
        return 3100;
    }
    int tmpVbat = get_vbat();
    if(tmpVbat > 1500 && tmpVbat < 3100) {
      if(tmpVbat < vbatVIntegrated) {
        vbatVIntegrated = tmpVbat;                                                        //immediately assume the lowest possibe value measured
        loop = 0;
      }
      else if(tmpVbat > vbatVIntegrated) {
        #ifndef MONITOR_VOLTAGE_INTEGRATOR
          //During monitoring do not force a reset to normal and high voltage
          if(tmpVbat > 2900) {                                                           //if high enough, reset
            vbatVIntegrated = tmpVbat;
          loop = 0;
          }
          else
        #endif
        if(vbatVIntegrated < tmpVbat && minutePulse) {                                    // Every min if vbatTIntegrated is lower than actual V, then creep closer
          vbatVIntegrated = vbatVIntegrated + max(1,((tmpVbat - vbatVIntegrated) >> 4));  //   (2500 - 2350) >> 4 = 9 increase every minute
        }
      }
    }
    else {
      vbatVIntegrated = tmpVbat;
      loop = 0;
    }

    #ifdef MONITOR_VOLTAGE_INTEGRATOR
      //Monitoring for voltage integrator
      if(minutePulse) {
        char aaa[120];
        sprintf(aaa,"         V=%i VI=%i loop=%i cnt=%i   ",tmpVbat, vbatVIntegrated, loop++, loop2++);
        print_numberstr(aaa,true);
        convertDoubleToReal34RegisterPush((double)vbatVIntegrated, REGISTER_X);
        uint8_t min = rtc_read_min();
        convertDoubleToReal34RegisterPush((double)min, REGISTER_X);
        fnSigmaAddRem(SIGMA_PLUS);
        fnDrop(NOPARAM);
        fnDrop(NOPARAM);
      }
    #endif

    return tmpVbat; //returning the direct battery voltage; to enable the selective usage of the integrator
  }


  void checkBattery(void) {
    if(usb_powered() == 1) {
      if(!getSystemFlag(FLAG_USB)) {
        setSystemFlag(FLAG_USB);
        clearSystemFlag(FLAG_LOWBAT);
        showHideUsbLowBattery();
      }
    }
    else {
      if(getSystemFlag(FLAG_USB)) {
        clearSystemFlag(FLAG_USB);
        showHideUsbLowBattery();
      }

      int tmpVbat = updateVbatIntegrated(false);

      if(tmpVbat < 2100 || vbatVIntegrated < 2100) { //shutdown from the new integrator system. The indicator uses the integrator.
        if(!getSystemFlag(FLAG_LOWBAT)) {
          setSystemFlag(FLAG_LOWBAT);
          showHideUsbLowBattery();
        }
        SET_ST(STAT_PGM_END);
      }
      else if(tmpVbat < 2500 || vbatVIntegrated < 2500) {
        if(!getSystemFlag(FLAG_LOWBAT)) {
          setSystemFlag(FLAG_LOWBAT);
          showHideUsbLowBattery();
        }
      }
      else {
        if(getSystemFlag(FLAG_LOWBAT)) {
          clearSystemFlag(FLAG_LOWBAT);
          showHideUsbLowBattery();
        }
      }
    }
  }
#endif //DMCP_BUILD

/* not used anymore, replaced by DMCP and ActUSB
*/
void backToSystem(uint16_t confirmation) {
  if(confirmation == NOT_CONFIRMED) {
    setConfirmationMode(backToSystem);
  }
  else {
    cancelFilename = true;
    #if defined(PC_BUILD)
      fnOff(NOPARAM);
    #endif // PC_BUILD

    #if defined(DMCP_BUILD)
      backToDMCP = true;
    #endif // DMCP_BUILD
  }
}

void runDMCPmenu(uint16_t confirmation) {
  #if defined(DMCP_BUILD)
    if(confirmation == NOT_CONFIRMED) {
      setConfirmationMode(runDMCPmenu);
    }
    else {
      cancelFilename = true;
//      #if defined(PC_BUILD)  //for consistency with backToSystem
//        fnOff(NOPARAM);
//      #endif // PC_BUILD
      run_menu_item_sys(MI_DMCP_MENU);
      clearScreen(200);
    }
  #endif //!PC_BUILD
}

void activateUSBdisk(uint16_t confirmation) {
  #if defined(DMCP_BUILD)
    if(confirmation == NOT_CONFIRMED) {
      setConfirmationMode(activateUSBdisk);
    }
    else {
      cancelFilename = true;
      run_menu_item_sys(MI_MSC);
      clearScreen(201);
    }
  #endif //!PC_BUILD
}



/********************************************//**
 * \brief Sets/resets KEYS MANAGEMENT
 *
 * \param[in] jmConfig uint16_t
 * \return void
 ***********************************************/
void fnKeysManagement(uint16_t choice) {
  switch(choice) {
    //---KEYS SIGMA+ ALLOCATIONS: COPY SIGMA+ USER MODE primary to -> ALLMODE
    //-----------------------------------------------------------------------
    case TO_USER:
//      if(Norm_Key_00.func != ITM_SHIFTf && Norm_Key_00.func != ITM_SHIFTg && Norm_Key_00.func != KEY_fg) {  //This line removed: it prevents f, g, fg in USER on the Norm_Key
        if(Norm_Key_00_key != -1) {
          kbd_usr[Norm_Key_00_key].primary = Norm_Key_00.func;
          setUserKeyArgument(Norm_Key_00_key * 6 , Norm_Key_00.funcParam);
          fnRefreshState();
          fnSetFlag(FLAG_USER);
        } else {
          Norm_Key_00.used = false;
          displayCalcErrorMessage(ERROR_CANNOT_ASSIGN_HERE, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
          #if defined(PC_BUILD)
            moreInfoOnError("In function fnKeysManagement: TO_USER", "the NRM key is not available.",NULL, NULL);
          #endif // PC_BUILD
        }
//      }
      break;

    case FROM_USER:
      if(Norm_Key_00_key != -1) {
        Norm_Key_00.func = kbd_usr[Norm_Key_00_key].primary;
        Norm_Key_00.funcParam[0] = 0;
        Norm_Key_00.used = Norm_Key_00.func != kbd_std[Norm_Key_00_key].primary;
        char *funcParam = (char *)getNthString((uint8_t *)userKeyLabel, Norm_Key_00_key * 6);
        if((funcParam[0] != 0) && ((Norm_Key_00.func == -MNU_DYNAMIC)|| (Norm_Key_00.func == ITM_XEQ) || (Norm_Key_00.func == ITM_RCL)))  {
          strcpy(Norm_Key_00.funcParam, (char *)getNthString((uint8_t *)userKeyLabel, Norm_Key_00_key * 6));       // name of a user menu, program or variable assigned to the Norm key
        }
        fnRefreshState();
        fnClearFlag(FLAG_USER);
      } else {
        Norm_Key_00.used = false;
        displayCalcErrorMessage(ERROR_CANNOT_ASSIGN_HERE, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
        #if defined(PC_BUILD)
          moreInfoOnError("In function fnKeysManagement: FROM_USER", "the NRM key is not available.",NULL, NULL);
        #endif // PC_BUILD
      }
      break;

    case USER_R47f_g:
    case USER_R47bk_fg:
    case USER_R47fg_bk:
    case USER_R47fg_g:
    case USER_C47:
    case USER_DM42:
      calcModel = choice;
      fnClearFlag(FLAG_USER);
      fnKeysManagement(USER_KRESET);                      // Reset all user keys when a permanent layout is changed, Reset +NRM when a permanent layout is changed
      if(choice == USER_R47bk_fg) {                       // MyMenu is the long press default for R47bk_fg
        fnClearFlag(FLAG_HOME_TRIPLE);
        fnSetFlag(FLAG_MYM_TRIPLE);
      }
      else {                                              // HOME is the long press default for all other options
        fnSetFlag(FLAG_HOME_TRIPLE);
        fnClearFlag(FLAG_MYM_TRIPLE);
      }
      fnShowVersion(choice);
      break;


    case USER_KRESET:
      fnShowVersion(choice);
      xcopy(kbd_usr, kbd_std, sizeof(kbd_std_C47));       // sizeof does not work when using the define for kbd_std
      Norm_Key_00.func = Norm_Key_00_item_in_layout;
      Norm_Key_00.funcParam[0] = 0;
      Norm_Key_00.used = false;
      //setLongPressFg(calcModel, (calcModel == USER_R47bk_fg ? -MNU_MyMenu : -MNU_HOME));
      fnRefreshState();
      fnClearFlag(FLAG_USER);
      break;

    case USER_HRESET:
      #if !defined(TESTSUITE_BUILD)
        createHOME();
        showSoftmenu(-MNU_HOME);
        fnShowVersion(choice);
      #endif // !TESTSUITE_BUILD
      break;

    case USER_PRESET:
      #if !defined(TESTSUITE_BUILD)
        createPFN();
        showSoftmenu(-MNU_PFN);
        fnShowVersion(choice);
      #endif // !TESTSUITE_BUILD
      break;

    case USER_MRESET:
      fnRESET_MyM(0);
      fnShowVersion(choice);
      break;

    case USER_ARESET:
      fnRESET_Mya();
      fnShowVersion(choice);
      break;

    case ITM_RIBBON_CPX  :
    case ITM_RIBBON_FIN  :
    case ITM_RIBBON_FIN2 :
    case ITM_RIBBON_ENG  :
    case ITM_RIBBON_SAV  :
    case ITM_RIBBON_SAV2 :
    case ITM_RIBBON_C47  :
    case ITM_RIBBON_C47PL:
    case ITM_RIBBON_R47  :
    case ITM_RIBBON_R47PL:
      fnRESET_MyM(choice);
      fnShowVersion(choice);
      #if !defined(TESTSUITE_BUILD)
        showSoftmenu(-MNU_MyMenu);
      #endif // !TESTSUITE_BUILD
      break;


    default:
      break;
  }
}
