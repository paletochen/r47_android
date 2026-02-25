// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"

// C47 Layout from Layout_template_automation template: Do not change manually
//This variable is to store in flash memory
TO_QSPI const calcKey_t kbd_std_C47[37] = { //C47 Layout, in the default position without suffix, kbd_std
  //keyID,              primary,              fShifted,             gShifted,             keyLblAim,            primaryAim,           fShiftedAim,          gShiftedAim,          primaryTam
  {21,                  ITM_SIGMAPLUS,        ITM_RI,               ITM_TGLFRT,           ITM_NULL,             ITM_A,                ITM_a,                ITM_SIGMA,            ITM_REG_A           },
  {22,                  ITM_1ONX,             ITM_YX,               ITM_HASH_JM,          ITM_NUMBER_SIGN,      ITM_B,                ITM_b,                ITM_CIRCUMFLEX,       ITM_REG_B           },
  {23,                  ITM_SQUAREROOTX,      ITM_SQUARE,           ITM_ms,               ITM_ROOT_SIGN,        ITM_C,                ITM_c,                ITM_ROOT_SIGN,        ITM_REG_C           },
  {24,                  ITM_LOG10,            ITM_10x,              ITM_dotD,             ITM_NULL,             ITM_D,                ITM_d,                ITM_LG_SIGN,          ITM_REG_D           },
  {25,                  ITM_LN,               ITM_EXP,              ITM_LBL,              ITM_NULL,             ITM_E,                ITM_e,                ITM_LN_SIGN,          ITM_REG_E           },
  {26,                  ITM_XEQ,              ITM_AIM,              ITM_GTO,              ITM_NULL,             ITM_F,                ITM_f,                ITM_alpha,            ITM_alpha           },
  {31,                  ITM_STO,              ITM_MAGNITUDE,        ITM_ARG,              ITM_NULL,             ITM_G,                ITM_g,                ITM_VERTICAL_BAR,     ITM_REG_G           },
  {32,                  ITM_RCL,              ITM_PC,               ITM_DELTAPC,          ITM_NULL,             ITM_H,                ITM_h,                ITM_DELTA,            ITM_REG_H           },
  {33,                  ITM_Rdown,            ITM_CONSTpi,          ITM_XTHROOT,          ITM_NULL,             ITM_I,                ITM_i,                ITM_pi,               ITM_REG_I           },
  {34,                  ITM_sin,              ITM_arcsin,           ITM_op_j,             ITM_NULL,             ITM_J,                ITM_j,                ITM_SIN_SIGN,         ITM_REG_J           },
  {35,                  ITM_cos,              ITM_arccos,           ITM_toREC2,           ITM_NULL,             ITM_K,                ITM_k,                ITM_COS_SIGN,         ITM_REG_K           },
  {36,                  ITM_tan,              ITM_arctan,           ITM_toPOL2,           ITM_NULL,             ITM_L,                ITM_l,                ITM_TAN_SIGN,         ITM_REG_L           },
  {41,                  ITM_ENTER,            KEY_COMPLEX,          -MNU_CPX,             ITM_ENTER,            ITM_ENTER,            ITM_XEDIT,            ITM_CR,               ITM_ENTER           },
  {42,                  ITM_XexY,             ITM_LASTX,            -MNU_STK,             ITM_ex,               ITM_M,                ITM_m,                ITM_ex,               ITM_REG_M           },
  {43,                  ITM_CHS,              -MNU_MODE,            -MNU_TRG_C47,         ITM_PLUS_MINUS,       ITM_N,                ITM_n,                ITM_PLUS_MINUS,       ITM_REG_N           },
  {44,                  ITM_EXPONENT,         -MNU_DISP,            -MNU_EXP,             ITM_NULL,             ITM_O,                ITM_o,                ITM_EEXCHR,           ITM_REG_O           },
  {45,                  ITM_BACKSPACE,        ITM_UNDO,             -MNU_CLR,             ITM_BACKSPACE,        ITM_BACKSPACE,        ITM_CLA,              ITM_CLA,              ITM_BACKSPACE       },
  {51,                  ITM_UP1,              ITM_BST,              ITM_RBR,              ITM_UP1,              ITM_UP1,              CHR_caseUP,           ITM_UP_ARROW,         ITM_UP1             },
  {52,                  ITM_7,                -MNU_EQN,             -MNU_HOME,            ITM_7,                ITM_P,                ITM_p,                ITM_7,                ITM_7               },
  {53,                  ITM_8,                -MNU_ADV,             -MNU_FIN,             ITM_8,                ITM_Q,                ITM_q,                ITM_8,                ITM_8               },
  {54,                  ITM_9,                -MNU_MATX,            -MNU_XFN,             ITM_9,                ITM_R,                ITM_r,                ITM_9,                ITM_9               },
  {55,                  ITM_DIV,              -MNU_STAT,            -MNU_PLOTTING,        ITM_OBELUS,           ITM_S,                ITM_s,                ITM_OBELUS,           ITM_DIV             },
  {61,                  ITM_DOWN1,            ITM_SST,              ITM_FLGSV,            ITM_DOWN1,            ITM_DOWN1,            CHR_caseDN,           ITM_DOWN_ARROW,       ITM_DOWN1           },
  {62,                  ITM_4,                -MNU_BASE,            -MNU_BITS,            ITM_4,                ITM_T,                ITM_t,                ITM_4,                ITM_4               },
  {63,                  ITM_5,                -MNU_UNITCONV,        -MNU_CLK,             ITM_5,                ITM_U,                ITM_u,                ITM_5,                ITM_5               },
  {64,                  ITM_6,                -MNU_FLAGS,           -MNU_PARTS,           ITM_6,                ITM_V,                ITM_v,                ITM_6,                ITM_6               },
  {65,                  ITM_MULT,             -MNU_PROB,            -MNU_INTS,            ITM_CROSS,            ITM_W,                ITM_w,                ITM_CROSS,            ITM_MULT            },
  {71,                  KEY_fg,               ITM_NULL,              ITM_NULL,            KEY_fg,               ITM_NULL,             ITM_NULL,             ITM_NULL,             KEY_fg              },
  {72,                  ITM_1,                ITM_ASSIGN,           -MNU_KEYS,            ITM_1,                ITM_X,                ITM_x,                ITM_1,                ITM_1               },
  {73,                  ITM_2,                ITM_USERMODE,         -MNU_ALPHAFN,         ITM_2,                ITM_Y,                ITM_y,                ITM_2,                ITM_2               },
  {74,                  ITM_3,                -MNU_PFN,             -MNU_LOOP,            ITM_3,                ITM_Z,                ITM_z,                ITM_3,                ITM_3               },
  {75,                  ITM_SUB,              -MNU_PRINT,           -MNU_IO,              ITM_MINUS,            ITM_UNDERSCORE,       ITM_EulerE,           ITM_MINUS,            ITM_SUB             },
  {81,                  ITM_EXIT1,            ITM_OFF,              ITM_SNAP,             ITM_EXIT1,            ITM_EXIT1,            ITM_OFF,              ITM_SNAP,             ITM_EXIT1           },
  {82,                  ITM_0,                ITM_VIEW,             ITM_TIMER,            ITM_0,                ITM_COLON,            ITM_SEMICOLON,        ITM_0,                ITM_0               },
  {83,                  ITM_PERIOD,           ITM_SHOW,             -MNU_INFO,            ITM_PERIOD,           ITM_COMMA,            ITM_NUMBER_SIGN,      ITM_PERIOD,           ITM_PERIOD          },
  {84,                  ITM_RS,               ITM_PR,               -MNU_TEST,            ITM_NULL,             ITM_QUESTION_MARK,    ITM_EXCLAMATION_MARK, ITM_SLASH,            ITM_RS              },
  {85,                  ITM_ADD,              -MNU_CATALOG,         -MNU_CONST,           ITM_PLUS,             ITM_SPACE,            -MNU_AIMCATALOG,      ITM_PLUS,             ITM_ADD             }
  //keyID,              primary,              fShifted,             gShifted,             keyLblAim,            primaryAim,           fShiftedAim,          gShiftedAim,          primaryTam
};














































#if defined(PC_BUILD)
// E47 Layout from Layout_template_automation template: Do not change manually
//This variable is to store in flash memory
TO_QSPI const calcKey_t kbd_std_E47[37] = {
  //keyID,              primary,              fShifted,             gShifted,             keyLblAim,            primaryAim,           fShiftedAim,          gShiftedAim,          primaryTam
  {21,                  ITM_STO,              ITM_RI,               ITM_TGLFRT,           ITM_NULL,             ITM_A,                ITM_a,                ITM_NULL,             ITM_REG_A           },
  {22,                  ITM_RCL,              ITM_XTHROOT,          ITM_HASH_JM,          ITM_NUMBER_SIGN,      ITM_B,                ITM_b,                ITM_NULL,             ITM_REG_B           },
  {23,                  ITM_1ONX,             ITM_YX,               ITM_ms,               ITM_NULL,             ITM_C,                ITM_c,                ITM_NULL,             ITM_REG_C           },
  {24,                  ITM_SQUAREROOTX,      ITM_SQUARE,           ITM_dotD,             ITM_ROOT_SIGN,        ITM_D,                ITM_d,                ITM_ROOT_SIGN,        ITM_REG_D           },
  {25,                  ITM_LOG10,            ITM_10x,              ITM_toREC2,           ITM_NULL,             ITM_E,                ITM_e,                ITM_LG_SIGN,          ITM_NULL            },
  {26,                  ITM_LN,               ITM_EXP,              ITM_toPOL2,           ITM_NULL,             ITM_F,                ITM_f,                ITM_LN_SIGN,          ITM_NULL            },
  {31,                  ITM_SHIFTf,           ITM_NULL,             ITM_NULL,             ITM_SHIFTf,           ITM_NULL,             ITM_NULL,             ITM_NULL,             ITM_SHIFTf          },
  {32,                  ITM_SHIFTg,           ITM_NULL,             ITM_NULL,             ITM_SHIFTg,           ITM_NULL,             ITM_NULL,             ITM_NULL,             ITM_SHIFTg          },
  {33,                  ITM_Rdown,            ITM_Rup,              ITM_CONSTpi,          ITM_NULL,             ITM_G,                ITM_g,                ITM_pi,               ITM_NULL            },
  {34,                  ITM_sin,              ITM_arcsin,           ITM_op_j,             ITM_NULL,             ITM_H,                ITM_h,                ITM_SIN_SIGN,         ITM_NULL            },
  {35,                  ITM_cos,              ITM_arccos,           ITM_MAGNITUDE,        ITM_NULL,             ITM_I,                ITM_i,                ITM_COS_SIGN,         ITM_REG_I           },
  {36,                  ITM_tan,              ITM_arctan,           ITM_ARG,              ITM_NULL,             ITM_J,                ITM_j,                ITM_TAN_SIGN,         ITM_REG_J           },
  {41,                  ITM_ENTER,            KEY_COMPLEX,          -MNU_CPX,             ITM_ENTER,            ITM_ENTER,            ITM_XEDIT,            ITM_CR,               ITM_ENTER           },
  {42,                  ITM_XexY,             ITM_LASTX,            -MNU_STK,             ITM_ex,               ITM_K,                ITM_k,                ITM_ex,               ITM_REG_K           },
  {43,                  ITM_CHS,              ITM_DRG,              -MNU_TRG_C47,         ITM_PLUS_MINUS,       ITM_L,                ITM_l,                ITM_PLUS_MINUS,       ITM_REG_L           },
  {44,                  ITM_EXPONENT,         -MNU_EXP,             -MNU_DISP,            ITM_NULL,             ITM_M,                ITM_m,                ITM_EEXCHR,           ITM_NULL            },
  {45,                  ITM_BACKSPACE,        ITM_UNDO,             -MNU_CLR,             ITM_BACKSPACE,        ITM_BACKSPACE,        ITM_CLA,              ITM_CLA,              ITM_BACKSPACE       },
  {51,                  ITM_XEQ,              ITM_AIM,              ITM_USERMODE,         ITM_NULL,             ITM_NULL,             ITM_NULL,             ITM_alpha,            ITM_alpha           },
  {52,                  ITM_7,                ITM_GTO,              -MNU_HOME,            ITM_7,                ITM_N,                ITM_n,                ITM_7,                ITM_7               },
  {53,                  ITM_8,                -MNU_EQN,             -MNU_ADV,             ITM_8,                ITM_O,                ITM_o,                ITM_8,                ITM_8               },
  {54,                  ITM_9,                -MNU_MATX,            -MNU_XFN,             ITM_9,                ITM_P,                ITM_p,                ITM_9,                ITM_9               },
  {55,                  ITM_DIV,              -MNU_STAT,            -MNU_PLOTTING,        ITM_OBELUS,           ITM_Q,                ITM_q,                ITM_OBELUS,           ITM_DIV             },
  {61,                  ITM_UP1,              ITM_BST,              ITM_RBR,              ITM_UP1,              ITM_UP1,              CHR_caseUP,           ITM_UP_ARROW,         ITM_UP1             },
  {62,                  ITM_4,                -MNU_BASE,            -MNU_BITS,            ITM_4,                ITM_R,                ITM_r,                ITM_4,                ITM_4               },
  {63,                  ITM_5,                -MNU_UNITCONV,        -MNU_CLK,             ITM_5,                ITM_S,                ITM_s,                ITM_5,                ITM_5               },
  {64,                  ITM_6,                -MNU_FLAGS,           -MNU_PARTS,           ITM_6,                ITM_T,                ITM_t,                ITM_6,                ITM_6               },
  {65,                  ITM_MULT,             -MNU_PROB,            -MNU_INTS,            ITM_CROSS,            ITM_U,                ITM_u,                ITM_CROSS,            ITM_MULT            },
  {71,                  ITM_DOWN1,            ITM_SST,              ITM_FLGSV,            ITM_DOWN1,            ITM_DOWN1,            CHR_caseDN,           ITM_DOWN_ARROW,       ITM_DOWN1           },
  {72,                  ITM_1,                ITM_ASSIGN,           -MNU_KEYS,            ITM_1,                ITM_V,                ITM_v,                ITM_1,                ITM_1               },
  {73,                  ITM_2,                -MNU_PREF,            -MNU_ALPHAFN,         ITM_2,                ITM_W,                ITM_w,                ITM_2,                ITM_2               },
  {74,                  ITM_3,                -MNU_PFN,             -MNU_LOOP,            ITM_3,                ITM_X,                ITM_x,                ITM_3,                ITM_3               },
  {75,                  ITM_SUB,              -MNU_FIN,             -MNU_IO,              ITM_MINUS,            ITM_Y,                ITM_y,                ITM_MINUS,            ITM_SUB             },
  {81,                  ITM_EXIT1,            ITM_OFF,              ITM_SNAP,             ITM_EXIT1,            ITM_EXIT1,            ITM_OFF,              ITM_SNAP,             ITM_EXIT1           },
  {82,                  ITM_0,                ITM_VIEW,             ITM_TIMER,            ITM_0,                ITM_Z,                ITM_z,                ITM_0,                ITM_0               },
  {83,                  ITM_PERIOD,           ITM_SHOW,             -MNU_INFO,            ITM_PERIOD,           ITM_COMMA,            ITM_PERIOD,           ITM_PERIOD,           ITM_PERIOD          },
  {84,                  ITM_RS,               ITM_PR,               -MNU_TEST,            ITM_NULL,             ITM_QUESTION_MARK,    ITM_SLASH,            ITM_SLASH,            ITM_RS              },
  {85,                  ITM_ADD,              -MNU_CATALOG,         -MNU_CONST,           ITM_PLUS,             ITM_SPACE,            ITM_PLUS,             ITM_PLUS,             ITM_ADD             }
  //keyID,              primary,              fShifted,             gShifted,             keyLblAim,            primaryAim,           fShiftedAim,          gShiftedAim,          primaryTam
};


// V47 Layout from Layout_template_automation template: Do not change manually
//This variable is to store in flash memory
TO_QSPI const calcKey_t kbd_std_V47[37] = {
  //keyID,              primary,              fShifted,             gShifted,             keyLblAim,            primaryAim,           fShiftedAim,          gShiftedAim,          primaryTam
  {21,                  ITM_1ONX,             ITM_YX,               ITM_TGLFRT,           ITM_NULL,             ITM_A,                ITM_a,                ITM_CIRCUMFLEX,       ITM_REG_A           },
  {22,                  ITM_SQUAREROOTX,      ITM_SQUARE,           ITM_HASH_JM,          ITM_ROOT_SIGN,        ITM_B,                ITM_b,                ITM_ROOT_SIGN,        ITM_REG_B           },
  {23,                  ITM_LOG10,            ITM_10x,              ITM_ms,               ITM_NULL,             ITM_C,                ITM_c,                ITM_LG_SIGN,          ITM_REG_C           },
  {24,                  ITM_LN,               ITM_EXP,              ITM_dotD,             ITM_NULL,             ITM_D,                ITM_d,                ITM_LN_SIGN,          ITM_REG_D           },
  {25,                  ITM_SHIFTf,           ITM_NULL,             ITM_NULL,             ITM_SHIFTf,           ITM_NULL,             ITM_NULL,             ITM_NULL,             ITM_SHIFTf          },
  {26,                  ITM_SHIFTg,           ITM_NULL,             ITM_NULL,             ITM_SHIFTg,           ITM_NULL,             ITM_NULL,             ITM_NULL,             ITM_SHIFTg          },
  {31,                  ITM_STO,              ITM_MAGNITUDE,        ITM_ARG,              ITM_NULL,             ITM_E,                ITM_e,                ITM_VERTICAL_BAR,     ITM_NULL            },
  {32,                  ITM_RCL,              ITM_PC,               ITM_DELTAPC,          ITM_NULL,             ITM_F,                ITM_f,                ITM_DELTA,            ITM_2HEX            },
  {33,                  ITM_Rdown,            ITM_CONSTpi,          ITM_XTHROOT,          ITM_NULL,             ITM_G,                ITM_g,                ITM_pi,               ITM_NULL            },
  {34,                  ITM_sin,              ITM_arcsin,           ITM_op_j,             ITM_NULL,             ITM_H,                ITM_h,                ITM_SIN_SIGN,         ITM_NULL            },
  {35,                  ITM_cos,              ITM_arccos,           ITM_LBL,              ITM_NULL,             ITM_I,                ITM_i,                ITM_COS_SIGN,         ITM_REG_I           },
  {36,                  ITM_tan,              ITM_arctan,           ITM_GTO,              ITM_NULL,             ITM_J,                ITM_j,                ITM_TAN_SIGN,         ITM_REG_J           },
  {41,                  ITM_ENTER,            KEY_COMPLEX,          -MNU_CPX,             ITM_ENTER,            ITM_ENTER,            ITM_XEDIT,            ITM_CR,               ITM_ENTER           },
  {42,                  ITM_XexY,             ITM_LASTX,            -MNU_STK,             ITM_ex,               ITM_K,                ITM_k,                ITM_ex,               ITM_REG_K           },
  {43,                  ITM_CHS,              -MNU_MODE,            -MNU_TRG_C47,         ITM_PLUS_MINUS,       ITM_L,                ITM_l,                ITM_PLUS_MINUS,       ITM_REG_L           },
  {44,                  ITM_EXPONENT,         -MNU_DISP,            -MNU_EXP,             ITM_NULL,             ITM_M,                ITM_m,                ITM_EEXCHR,           ITM_NULL            },
  {45,                  ITM_BACKSPACE,        ITM_UNDO,             -MNU_CLR,             ITM_BACKSPACE,        ITM_BACKSPACE,        ITM_CLA,              ITM_CLA,              ITM_BACKSPACE       },
  {51,                  ITM_SUB,              -MNU_STAT,            -MNU_PLOTTING,        ITM_MINUS,            ITM_N,                ITM_n,                ITM_MINUS,            ITM_SUB             },
  {52,                  ITM_7,                -MNU_EQN,             -MNU_HOME,            ITM_7,                ITM_O,                ITM_o,                ITM_7,                ITM_7               },
  {53,                  ITM_8,                -MNU_ADV,             -MNU_FIN,             ITM_8,                ITM_P,                ITM_p,                ITM_8,                ITM_8               },
  {54,                  ITM_9,                -MNU_MATX,            -MNU_XFN,             ITM_9,                ITM_Q,                ITM_q,                ITM_9,                ITM_9               },
  {55,                  ITM_XEQ,              ITM_AIM,              ITM_USERMODE,         ITM_NULL,             ITM_R,                ITM_r,                ITM_alpha,            ITM_alpha           },
  {61,                  ITM_ADD,              -MNU_PROB,            -MNU_INTS,            ITM_PLUS,             ITM_S,                ITM_s,                ITM_PLUS,             ITM_ADD             },
  {62,                  ITM_4,                -MNU_BASE,            -MNU_BITS,            ITM_4,                ITM_T,                ITM_t,                ITM_4,                ITM_4               },
  {63,                  ITM_5,                -MNU_UNITCONV,        -MNU_CLK,             ITM_5,                ITM_U,                ITM_u,                ITM_5,                ITM_5               },
  {64,                  ITM_6,                -MNU_FLAGS,           -MNU_PARTS,           ITM_6,                ITM_V,                ITM_v,                ITM_6,                ITM_6               },
  {65,                  ITM_UP1,              ITM_BST,              ITM_RBR,              ITM_UP1,              ITM_UP1,              CHR_caseUP,           ITM_UP_ARROW,         ITM_UP1             },
  {71,                  ITM_MULT,             -MNU_PRINT,           -MNU_IO,              ITM_CROSS,            ITM_W,                ITM_w,                ITM_CROSS,            ITM_MULT            },
  {72,                  ITM_1,                ITM_ASSIGN,           -MNU_KEYS,            ITM_1,                ITM_X,                ITM_x,                ITM_1,                ITM_1               },
  {73,                  ITM_2,                ITM_USERMODE,         -MNU_ALPHAFN,         ITM_2,                ITM_Y,                ITM_y,                ITM_2,                ITM_2               },
  {74,                  ITM_3,                -MNU_PFN,             -MNU_LOOP,            ITM_3,                ITM_Z,                ITM_z,                ITM_3,                ITM_3               },
  {75,                  ITM_DOWN1,            ITM_SST,              ITM_FLGSV,            ITM_DOWN1,            ITM_DOWN1,            CHR_caseDN,           ITM_DOWN_ARROW,       ITM_DOWN1           },
  {81,                  ITM_DIV,              -MNU_CATALOG,         -MNU_CONST,           ITM_OBELUS,           ITM_SPACE,            ITM_OBELUS,           ITM_OBELUS,           ITM_DIV             },
  {82,                  ITM_0,                ITM_VIEW,             ITM_TIMER,            ITM_0,                ITM_COLON,            ITM_SEMICOLON,        ITM_0,                ITM_0               },
  {83,                  ITM_PERIOD,           ITM_SHOW,             -MNU_INFO,            ITM_PERIOD,           ITM_COMMA,            ITM_PERIOD,           ITM_PERIOD,           ITM_PERIOD          },
  {84,                  ITM_RS,               ITM_PR,               -MNU_TEST,            ITM_NULL,             ITM_QUESTION_MARK,    ITM_SLASH,            ITM_SLASH,            ITM_RS              },
  {85,                  ITM_EXIT1,            ITM_OFF,              ITM_SNAP,             ITM_EXIT1,            ITM_EXIT1,            ITM_OFF,              ITM_SNAP,             ITM_EXIT1           }
  //keyID,              primary,              fShifted,             gShifted,             keyLblAim,            primaryAim,           fShiftedAim,          gShiftedAim,          primaryTam
};


// N47 Layout from Layout_template_automation template: Do not change manually
//This variable is to store in flash memory
TO_QSPI const calcKey_t kbd_std_N47[37] = {
  //keyID,              primary,              fShifted,             gShifted,             keyLblAim,            primaryAim,           fShiftedAim,          gShiftedAim,          primaryTam
  {21,                  ITM_sin,              ITM_arcsin,           ITM_TGLFRT,           ITM_NULL,             ITM_A,                ITM_a,                ITM_SIN_SIGN,         ITM_REG_A           },
  {22,                  ITM_cos,              ITM_arccos,           ITM_HASH_JM,          ITM_NUMBER_SIGN,      ITM_B,                ITM_b,                ITM_COS_SIGN,         ITM_REG_B           },
  {23,                  ITM_tan,              ITM_arctan,           ITM_ms,               ITM_NULL,             ITM_C,                ITM_c,                ITM_TAN_SIGN,         ITM_REG_C           },
  {24,                  ITM_1ONX,             ITM_YX,               ITM_dotD,             ITM_NULL,             ITM_D,                ITM_d,                ITM_CIRCUMFLEX,       ITM_REG_D           },
  {25,                  ITM_SQUAREROOTX,      ITM_SQUARE,           ITM_RI,               ITM_ROOT_SIGN,        ITM_E,                ITM_e,                ITM_ROOT_SIGN,        ITM_NULL            },
  {26,                  ITM_UP1,              ITM_BST,              ITM_RBR,              ITM_UP1,              ITM_UP1,              CHR_caseUP,           ITM_UP_ARROW,         ITM_UP1             },
  {31,                  ITM_STO,              ITM_CONSTpi,          ITM_PC,               ITM_NULL,             ITM_F,                ITM_f,                ITM_pi,               ITM_NULL            },
  {32,                  ITM_RCL,              ITM_MAGNITUDE,        ITM_ARG,              ITM_NULL,             ITM_G,                ITM_g,                ITM_VERTICAL_BAR,     ITM_2HEX            },
  {33,                  ITM_Rdown,            ITM_XTHROOT,          ITM_Rup,              ITM_NULL,             ITM_H,                ITM_h,                ITM_NULL,             ITM_NULL            },
  {34,                  ITM_LOG10,            ITM_10x,              ITM_toREC2,           ITM_NULL,             ITM_I,                ITM_i,                ITM_LN_SIGN,          ITM_REG_I           },
  {35,                  ITM_LN,               ITM_EXP,              ITM_toPOL2,           ITM_NULL,             ITM_J,                ITM_j,                ITM_NULL,             ITM_REG_J           },
  {36,                  ITM_DOWN1,            ITM_SST,              ITM_FLGSV,            ITM_DOWN1,            ITM_DOWN1,            CHR_caseDN,           ITM_DOWN_ARROW,       ITM_DOWN1           },
  {41,                  ITM_ENTER,            KEY_COMPLEX,          -MNU_CPX,             ITM_ENTER,            ITM_ENTER,            ITM_XEDIT,            ITM_CR,               ITM_ENTER           },
  {42,                  ITM_XexY,             ITM_LASTX,            -MNU_STK,             ITM_ex,               ITM_K,                ITM_k,                ITM_ex,               ITM_REG_K           },
  {43,                  ITM_CHS,              ITM_DRG,              -MNU_TRG_C47,         ITM_PLUS_MINUS,       ITM_L,                ITM_l,                ITM_PLUS_MINUS,       ITM_REG_L           },
  {44,                  ITM_EXPONENT,         -MNU_EXP,             -MNU_DISP,            ITM_NULL,             ITM_M,                ITM_m,                ITM_EEXCHR,           ITM_2OCT            },
  {45,                  ITM_BACKSPACE,        ITM_UNDO,             -MNU_CLR,             ITM_BACKSPACE,        ITM_BACKSPACE,        ITM_CLA,              ITM_CLA,              ITM_BACKSPACE       },
  {51,                  ITM_XEQ,              ITM_AIM,              ITM_USERMODE,         ITM_NULL,             ITM_N,                ITM_n,                ITM_alpha,            ITM_alpha           },
  {52,                  ITM_7,                ITM_GTO,              -MNU_HOME,            ITM_7,                ITM_O,                ITM_o,                ITM_7,                ITM_7               },
  {53,                  ITM_8,                -MNU_EQN,             -MNU_ADV,             ITM_8,                ITM_P,                ITM_p,                ITM_8,                ITM_8               },
  {54,                  ITM_9,                -MNU_MATX,            -MNU_XFN,             ITM_9,                ITM_Q,                ITM_q,                ITM_9,                ITM_9               },
  {55,                  ITM_DIV,              -MNU_STAT,            -MNU_PLOTTING,        ITM_OBELUS,           ITM_R,                ITM_r,                ITM_OBELUS,           ITM_DIV             },
  {61,                  ITM_SHIFTf,           ITM_NULL,             ITM_NULL,             ITM_SHIFTf,           ITM_NULL,             ITM_NULL,             ITM_NULL,             ITM_SHIFTf          },
  {62,                  ITM_4,                -MNU_BASE,            -MNU_BITS,            ITM_4,                ITM_S,                ITM_s,                ITM_4,                ITM_4               },
  {63,                  ITM_5,                -MNU_UNITCONV,        -MNU_CLK,             ITM_5,                ITM_T,                ITM_t,                ITM_5,                ITM_5               },
  {64,                  ITM_6,                -MNU_FLAGS,           -MNU_PARTS,           ITM_6,                ITM_U,                ITM_u,                ITM_6,                ITM_6               },
  {65,                  ITM_MULT,             -MNU_PROB,            -MNU_INTS,            ITM_CROSS,            ITM_V,                ITM_v,                ITM_CROSS,            ITM_MULT            },
  {71,                  ITM_SHIFTg,           ITM_NULL,             ITM_NULL,             ITM_SHIFTg,           ITM_NULL,             ITM_NULL,             ITM_NULL,             ITM_SHIFTg          },
  {72,                  ITM_1,                ITM_ASSIGN,           -MNU_KEYS,            ITM_1,                ITM_W,                ITM_w,                ITM_1,                ITM_1               },
  {73,                  ITM_2,                -MNU_PREF,            -MNU_ALPHAFN,         ITM_2,                ITM_X,                ITM_x,                ITM_2,                ITM_2               },
  {74,                  ITM_3,                -MNU_PFN,             -MNU_LOOP,            ITM_3,                ITM_Y,                ITM_y,                ITM_3,                ITM_3               },
  {75,                  ITM_SUB,              -MNU_FIN,             -MNU_IO,              ITM_MINUS,            ITM_NULL,             ITM_MINUS,            ITM_MINUS,            ITM_SUB             },
  {81,                  ITM_EXIT1,            ITM_OFF,              ITM_SNAP,             ITM_EXIT1,            ITM_EXIT1,            ITM_OFF,              ITM_SNAP,             ITM_EXIT1           },
  {82,                  ITM_0,                ITM_VIEW,             ITM_TIMER,            ITM_0,                ITM_Z,                ITM_z,                ITM_0,                ITM_0               },
  {83,                  ITM_PERIOD,           ITM_SHOW,             -MNU_INFO,            ITM_PERIOD,           ITM_COMMA,            ITM_PERIOD,           ITM_PERIOD,           ITM_PERIOD          },
  {84,                  ITM_RS,               ITM_PR,               -MNU_TEST,            ITM_NULL,             ITM_QUESTION_MARK,    ITM_SLASH,            ITM_SLASH,            ITM_RS              },
  {85,                  ITM_ADD,              -MNU_CATALOG,         -MNU_CONST,           ITM_PLUS,             ITM_SPACE,            ITM_PLUS,             ITM_PLUS,             ITM_ADD             }
  //keyID,              primary,              fShifted,             gShifted,             keyLblAim,            primaryAim,           fShiftedAim,          gShiftedAim,          primaryTam
};


// D47 Layout from Layout_template_automation template: Do not change manually
//This variable is to store in flash memory
TO_QSPI const calcKey_t kbd_std_D47[37] = {
  //keyID,              primary,              fShifted,             gShifted,             keyLblAim,            primaryAim,           fShiftedAim,          gShiftedAim,          primaryTam
  {21,                  ITM_LN,               ITM_EXP,              ITM_TGLFRT,           ITM_NULL,             ITM_A,                ITM_a,                ITM_LN_SIGN,          ITM_REG_A           },
  {22,                  ITM_LOG10,            ITM_10x,              ITM_HASH_JM,          ITM_NUMBER_SIGN,      ITM_B,                ITM_b,                ITM_LG_SIGN,          ITM_REG_B           },
  {23,                  ITM_SQUAREROOTX,      ITM_SQUARE,           ITM_ms,               ITM_ROOT_SIGN,        ITM_C,                ITM_c,                ITM_ROOT_SIGN,        ITM_REG_C           },
  {24,                  ITM_sin,              ITM_arcsin,           ITM_dotD,             ITM_NULL,             ITM_D,                ITM_d,                ITM_SIN_SIGN,         ITM_REG_D           },
  {25,                  ITM_cos,              ITM_arccos,           ITM_toREC2,           ITM_NULL,             ITM_E,                ITM_e,                ITM_COS_SIGN,         ITM_NULL            },
  {26,                  ITM_tan,              ITM_arctan,           ITM_toPOL2,           ITM_NULL,             ITM_F,                ITM_f,                ITM_TAN_SIGN,         ITM_NULL            },
  {31,                  ITM_STO,              ITM_RI,               ITM_PC,               ITM_NULL,             ITM_G,                ITM_g,                ITM_NULL,             ITM_NULL            },
  {32,                  ITM_RCL,              ITM_MAGNITUDE,        ITM_ARG,              ITM_NULL,             ITM_H,                ITM_h,                ITM_VERTICAL_BAR,     ITM_2HEX            },
  {33,                  ITM_Rdown,            ITM_XTHROOT,          ITM_Rup,              ITM_NULL,             ITM_I,                ITM_i,                ITM_NULL,             ITM_REG_I           },
  {34,                  ITM_1ONX,             ITM_YX,               ITM_CONSTpi,          ITM_NULL,             ITM_J,                ITM_j,                ITM_CIRCUMFLEX,       ITM_REG_J           },
  {35,                  ITM_SHIFTf,           ITM_NULL,             ITM_NULL,             ITM_SHIFTf,           ITM_NULL,             ITM_NULL,             ITM_NULL,             ITM_SHIFTf          },
  {36,                  ITM_SHIFTg,           ITM_NULL,             ITM_NULL,             ITM_SHIFTg,           ITM_NULL,             ITM_NULL,             ITM_NULL,             ITM_SHIFTg          },
  {41,                  ITM_ENTER,            KEY_COMPLEX,          -MNU_CPX,             ITM_ENTER,            ITM_ENTER,            ITM_XEDIT,            ITM_CR,               ITM_ENTER           },
  {42,                  ITM_XexY,             ITM_LASTX,            -MNU_STK,             ITM_ex,               ITM_K,                ITM_k,                ITM_ex,               ITM_REG_K           },
  {43,                  ITM_CHS,              ITM_DRG,              -MNU_TRG_R47,         ITM_PLUS_MINUS,       ITM_L,                ITM_l,                ITM_PLUS_MINUS,       ITM_REG_L           },
  {44,                  ITM_EXPONENT,         -MNU_EXP,             -MNU_DISP,            ITM_NULL,             ITM_M,                ITM_m,                ITM_EEXCHR,           ITM_NULL            },
  {45,                  ITM_BACKSPACE,        ITM_UNDO,             -MNU_CLR,             ITM_BACKSPACE,        ITM_BACKSPACE,        ITM_CLA,              ITM_CLA,              ITM_BACKSPACE       },
  {51,                  ITM_XEQ,              ITM_AIM,              ITM_USERMODE,         ITM_NULL,             ITM_NULL,             ITM_NULL,             ITM_alpha,            ITM_alpha           },
  {52,                  ITM_7,                ITM_GTO,              -MNU_HOME,            ITM_7,                ITM_N,                ITM_n,                ITM_7,                ITM_7               },
  {53,                  ITM_8,                -MNU_EQN,             -MNU_ADV,             ITM_8,                ITM_O,                ITM_o,                ITM_8,                ITM_8               },
  {54,                  ITM_9,                -MNU_MATX,            -MNU_XFN,             ITM_9,                ITM_P,                ITM_p,                ITM_9,                ITM_9               },
  {55,                  ITM_DIV,              -MNU_STAT,            -MNU_PLOTTING,        ITM_OBELUS,           ITM_Q,                ITM_q,                ITM_OBELUS,           ITM_DIV             },
  {61,                  ITM_UP1,              ITM_BST,              ITM_RBR,              ITM_UP1,              ITM_UP1,              CHR_caseUP,           ITM_UP_ARROW,         ITM_UP1             },
  {62,                  ITM_4,                -MNU_BASE,            -MNU_BITS,            ITM_4,                ITM_R,                ITM_r,                ITM_4,                ITM_4               },
  {63,                  ITM_5,                -MNU_UNITCONV,        -MNU_CLK,             ITM_5,                ITM_S,                ITM_s,                ITM_5,                ITM_5               },
  {64,                  ITM_6,                -MNU_FLAGS,           -MNU_PARTS,           ITM_6,                ITM_T,                ITM_t,                ITM_6,                ITM_6               },
  {65,                  ITM_MULT,             -MNU_PROB,            -MNU_INTS,            ITM_CROSS,            ITM_U,                ITM_u,                ITM_CROSS,            ITM_MULT            },
  {71,                  ITM_DOWN1,            ITM_SST,              ITM_FLGSV,            ITM_DOWN1,            ITM_DOWN1,            CHR_caseDN,           ITM_DOWN_ARROW,       ITM_DOWN1           },
  {72,                  ITM_1,                ITM_ASSIGN,           -MNU_KEYS,            ITM_1,                ITM_V,                ITM_v,                ITM_1,                ITM_1               },
  {73,                  ITM_2,                -MNU_PREF,            -MNU_ALPHAFN,         ITM_2,                ITM_W,                ITM_w,                ITM_2,                ITM_2               },
  {74,                  ITM_3,                -MNU_PFN,             -MNU_LOOP,            ITM_3,                ITM_X,                ITM_x,                ITM_3,                ITM_3               },
  {75,                  ITM_SUB,              -MNU_FIN,             -MNU_IO,              ITM_MINUS,            ITM_Y,                ITM_y,                ITM_MINUS,            ITM_SUB             },
  {81,                  ITM_EXIT1,            ITM_OFF,              ITM_SNAP,             ITM_EXIT1,            ITM_EXIT1,            ITM_OFF,              ITM_SNAP,             ITM_EXIT1           },
  {82,                  ITM_0,                ITM_VIEW,             ITM_TIMER,            ITM_0,                ITM_Z,                ITM_z,                ITM_0,                ITM_0               },
  {83,                  ITM_PERIOD,           ITM_SHOW,             -MNU_INFO,            ITM_PERIOD,           ITM_COMMA,            ITM_PERIOD,           ITM_PERIOD,           ITM_PERIOD          },
  {84,                  ITM_RS,               ITM_PR,               -MNU_TEST,            ITM_NULL,             ITM_QUESTION_MARK,    ITM_SLASH,            ITM_SLASH,            ITM_RS              },
  {85,                  ITM_ADD,              -MNU_CATALOG,         -MNU_CONST,           ITM_PLUS,             ITM_SPACE,            ITM_PLUS,             ITM_PLUS,             ITM_ADD             }
  //keyID,              primary,              fShifted,             gShifted,             keyLblAim,            primaryAim,           fShiftedAim,          gShiftedAim,          primaryTam
};
#endif //PC_BUILD

// DM42 Layout from Layout_template_automation template: Do not change manually
//This variable is to store in flash memory
TO_QSPI const calcKey_t kbd_std_DM42[37] = {
  //keyID,              primary,              fShifted,             gShifted,             keyLblAim,            primaryAim,           fShiftedAim,          gShiftedAim,          primaryTam
  {21,                  ITM_SIGMAPLUS,        ITM_SIGMAMINUS,       ITM_TGLFRT,           ITM_NULL,             ITM_A,                ITM_a,                ITM_SIGMA,            ITM_REG_A           },
  {22,                  ITM_1ONX,             ITM_YX,               ITM_HASH_JM,          ITM_NUMBER_SIGN,      ITM_B,                ITM_b,                ITM_CIRCUMFLEX,       ITM_REG_B           },
  {23,                  ITM_SQUAREROOTX,      ITM_SQUARE,           ITM_ms,               ITM_ROOT_SIGN,        ITM_C,                ITM_c,                ITM_ROOT_SIGN,        ITM_REG_C           },
  {24,                  ITM_LOG10,            ITM_10x,              ITM_dotD,             ITM_NULL,             ITM_D,                ITM_d,                ITM_LG_SIGN,          ITM_REG_D           },
  {25,                  ITM_LN,               ITM_EXP,              ITM_LBL,              ITM_NULL,             ITM_E,                ITM_e,                ITM_LN_SIGN,          ITM_NULL            },
  {26,                  ITM_XEQ,              ITM_GTO,              ITM_RTN,              ITM_NULL,             ITM_F,                ITM_f,                ITM_alpha,            ITM_alpha           },
  {31,                  ITM_STO,              KEY_COMPLEX,          ITM_ARG,              ITM_NULL,             ITM_G,                ITM_g,                ITM_VERTICAL_BAR,     ITM_NULL            },
  {32,                  ITM_RCL,              ITM_PC,               ITM_DELTAPC,          ITM_NULL,             ITM_H,                ITM_h,                ITM_DELTA,            ITM_2HEX            },
  {33,                  ITM_Rdown,            ITM_CONSTpi,          ITM_XTHROOT,          ITM_NULL,             ITM_I,                ITM_i,                ITM_pi,               ITM_REG_I           },
  {34,                  ITM_sin,              ITM_arcsin,           ITM_op_j,             ITM_NULL,             ITM_J,                ITM_j,                ITM_SIN_SIGN,         ITM_REG_J           },
  {35,                  ITM_cos,              ITM_arccos,           ITM_toREC2,           ITM_NULL,             ITM_K,                ITM_k,                ITM_COS_SIGN,         ITM_REG_K           },
  {36,                  ITM_tan,              ITM_arctan,           ITM_toPOL2,           ITM_NULL,             ITM_L,                ITM_l,                ITM_TAN_SIGN,         ITM_REG_L           },
  {41,                  ITM_ENTER,            ITM_AIM,              -MNU_CPX,             ITM_ENTER,            ITM_ENTER,            ITM_XEDIT,            ITM_CR,               ITM_ENTER           },
  {42,                  ITM_XexY,             ITM_LASTX,            -MNU_STK,             ITM_ex,               ITM_M,                ITM_m,                ITM_ex,               ITM_NULL            },
  {43,                  ITM_CHS,              -MNU_MODE,            -MNU_TRG_C47,         ITM_PLUS_MINUS,       ITM_N,                ITM_n,                ITM_PLUS_MINUS,       ITM_NULL            },
  {44,                  ITM_EXPONENT,         -MNU_DISP,            -MNU_EXP,             ITM_NULL,             ITM_O,                ITM_o,                ITM_EEXCHR,           ITM_2OCT            },
  {45,                  ITM_BACKSPACE,        -MNU_CLR,             ITM_UNDO,             ITM_BACKSPACE,        ITM_BACKSPACE,        ITM_CLA,              ITM_CLA,              ITM_BACKSPACE       },
  {51,                  ITM_UP1,              ITM_BST,              ITM_RBR,              ITM_UP1,              ITM_UP1,              CHR_caseUP,           ITM_UP_ARROW,         ITM_UP1             },
  {52,                  ITM_7,                -MNU_EQN,             -MNU_HOME,            ITM_7,                ITM_P,                ITM_p,                ITM_7,                ITM_7               },
  {53,                  ITM_8,                -MNU_ADV,             -MNU_FIN,             ITM_8,                ITM_Q,                ITM_q,                ITM_8,                ITM_8               },
  {54,                  ITM_9,                -MNU_MATX,            -MNU_XFN,             ITM_9,                ITM_R,                ITM_r,                ITM_9,                ITM_9               },
  {55,                  ITM_DIV,              -MNU_STAT,            -MNU_PLOTTING,        ITM_OBELUS,           ITM_S,                ITM_s,                ITM_OBELUS,           ITM_DIV             },
  {61,                  ITM_DOWN1,            ITM_SST,              ITM_FLGSV,            ITM_DOWN1,            ITM_DOWN1,            CHR_caseDN,           ITM_DOWN_ARROW,       ITM_DOWN1           },
  {62,                  ITM_4,                -MNU_BASE,            -MNU_BITS,            ITM_4,                ITM_T,                ITM_t,                ITM_4,                ITM_4               },
  {63,                  ITM_5,                -MNU_UNITCONV,        -MNU_CLK,             ITM_5,                ITM_U,                ITM_u,                ITM_5,                ITM_5               },
  {64,                  ITM_6,                -MNU_FLAGS,           -MNU_PARTS,           ITM_6,                ITM_V,                ITM_v,                ITM_6,                ITM_6               },
  {65,                  ITM_MULT,             -MNU_PROB,            -MNU_INTS,            ITM_CROSS,            ITM_W,                ITM_w,                ITM_CROSS,            ITM_MULT            },
  {71,                  KEY_fg,               ITM_NULL,              ITM_NULL,            KEY_fg,               ITM_NULL,             ITM_NULL,             ITM_NULL,             KEY_fg              },
  {72,                  ITM_1,                ITM_ASSIGN,           -MNU_KEYS,            ITM_1,                ITM_X,                ITM_x,                ITM_1,                ITM_1               },
  {73,                  ITM_2,                ITM_USERMODE,         -MNU_ALPHAFN,         ITM_2,                ITM_Y,                ITM_y,                ITM_2,                ITM_2               },
  {74,                  ITM_3,                -MNU_PFN,             -MNU_LOOP,            ITM_3,                ITM_Z,                ITM_z,                ITM_3,                ITM_3               },
  {75,                  ITM_SUB,              -MNU_PRINT,           -MNU_IO,              ITM_MINUS,            ITM_UNDERSCORE,       ITM_MINUS,            ITM_MINUS,            ITM_SUB             },
  {81,                  ITM_EXIT1,            ITM_OFF,              ITM_SNAP,             ITM_EXIT1,            ITM_EXIT1,            ITM_OFF,              ITM_SNAP,             ITM_EXIT1           },
  {82,                  ITM_0,                -MNU_BLUE_C47,        ITM_TIMER,            ITM_0,                ITM_COLON,            ITM_SEMICOLON,        ITM_0,                ITM_0               },
  {83,                  ITM_PERIOD,           ITM_SHOW,             -MNU_INFO,            ITM_PERIOD,           ITM_COMMA,            ITM_NUMBER_SIGN,      ITM_PERIOD,           ITM_PERIOD          },
  {84,                  ITM_RS,               ITM_PR,               -MNU_TEST,            ITM_NULL,             ITM_QUESTION_MARK,    ITM_EXCLAMATION_MARK, ITM_SLASH,            ITM_RS              },
  {85,                  ITM_ADD,              -MNU_CATALOG,         -MNU_CONST,           ITM_PLUS,             ITM_SPACE,            -MNU_AIMCATALOG,      ITM_PLUS,             ITM_ADD             }
  //keyID,              primary,              fShifted,             gShifted,             keyLblAim,            primaryAim,           fShiftedAim,          gShiftedAim,          primaryTam
};















































// R47 Layout from Layout_template_automation template: Do not change manually
//This variable is to store in flash memory
TO_QSPI const calcKey_t kbd_std_R47f_g[37] = {
  //keyID,              primary,              fShifted,             gShifted,             keyLblAim,            primaryAim,           fShiftedAim,          gShiftedAim,          primaryTam
  {21,                  ITM_SQUARE,           ITM_op_j,             ITM_toREC2,           ITM_NULL,             ITM_A,                ITM_a,                ITM_op_i_char,        ITM_REG_A           },
  {22,                  ITM_SQUAREROOTX,      ITM_op_j_pol,         ITM_toPOL2,           ITM_ROOT_SIGN,        ITM_B,                ITM_b,                ITM_ROOT_SIGN,        ITM_REG_B           },
  {23,                  ITM_1ONX,             ITM_XFACT,            ITM_ms,               ITM_NULL,             ITM_C,                ITM_c,                ITM_EXCLAMATION_MARK, ITM_REG_C           },
  {24,                  ITM_YX,               ITM_XTHROOT,          ITM_dotD,             ITM_NULL,             ITM_D,                ITM_d,                ITM_CIRCUMFLEX,       ITM_REG_D           },
  {25,                  ITM_LOG10,            ITM_10x,              ITM_RI,               ITM_NULL,             ITM_E,                ITM_e,                ITM_EulerE,           ITM_REG_E           },
  {26,                  ITM_LN,               ITM_EXP,              ITM_HASH_JM,          ITM_NUMBER_SIGN,      ITM_F,                ITM_f,                ITM_NUMBER_SIGN,      ITM_REG_F           },
  {31,                  ITM_STO,              ITM_MAGNITUDE,        ITM_ARG,              ITM_NULL,             ITM_G,                ITM_g,                ITM_VERTICAL_BAR,     ITM_REG_G           },
  {32,                  ITM_RCL,              ITM_PC,               ITM_DELTAPC,          ITM_NULL,             ITM_H,                ITM_h,                ITM_DELTA,            ITM_REG_H           },
  {33,                  ITM_Rdown,            ITM_CONSTpi,          ITM_Rup,              ITM_NULL,             ITM_I,                ITM_i,                ITM_pi,               ITM_REG_I           },
  {34,                  ITM_DRG,              ITM_USERMODE,         ITM_ASSIGN,           ITM_NULL,             ITM_J,                ITM_j,                ITM_op_j_char,        ITM_REG_J           },
  {35,                  ITM_SHIFTf,           ITM_NULL,             ITM_NULL,             ITM_SHIFTf,           ITM_NULL,             ITM_NULL,             ITM_NULL,             ITM_SHIFTf          },
  {36,                  ITM_SHIFTg,           ITM_NULL,             ITM_NULL,             ITM_SHIFTg,           ITM_NULL,             ITM_NULL,             ITM_NULL,             ITM_SHIFTg          },
  {41,                  ITM_ENTER,            KEY_COMPLEX,          -MNU_CPX,             ITM_ENTER,            ITM_ENTER,            ITM_XEDIT,            ITM_CR,               ITM_ENTER           },
  {42,                  ITM_XexY,             ITM_LASTX,            -MNU_STK,             ITM_ex,               ITM_K,                ITM_k,                ITM_ex,               ITM_REG_K           },
  {43,                  ITM_CHS,              -MNU_DISP,            -MNU_TRG_R47,         ITM_PLUS_MINUS,       ITM_L,                ITM_l,                ITM_PLUS_MINUS,       ITM_REG_L           },
  {44,                  ITM_EXPONENT,         -MNU_PREFIX,          -MNU_EXP,             ITM_NULL,             ITM_M,                ITM_m,                ITM_EEXCHR,           ITM_REG_M           },
  {45,                  ITM_BACKSPACE,        ITM_UNDO,             -MNU_CLR,             ITM_BACKSPACE,        ITM_BACKSPACE,        ITM_CLA,              ITM_CLA,              ITM_BACKSPACE       },
  {51,                  ITM_XEQ,              ITM_AIM,              ITM_GTO,              ITM_NULL,             ITM_UNDERSCORE,       ITM_omega,            ITM_alpha,            ITM_alpha           },
  {52,                  ITM_7,                ITM_sin,              ITM_arcsin,           ITM_7,                ITM_N,                ITM_n,                ITM_7,                ITM_7               },
  {53,                  ITM_8,                ITM_cos,              ITM_arccos,           ITM_8,                ITM_O,                ITM_o,                ITM_8,                ITM_8               },
  {54,                  ITM_9,                ITM_tan,              ITM_arctan,           ITM_9,                ITM_P,                ITM_p,                ITM_9,                ITM_9               },
  {55,                  ITM_DIV,              -MNU_STAT,            -MNU_PLOTTING,        ITM_OBELUS,           ITM_Q,                ITM_q,                ITM_OBELUS,           ITM_DIV             },
  {61,                  ITM_UP1,              ITM_BST,              ITM_RBR,              ITM_UP1,              ITM_UP1,              CHR_caseUP,           ITM_UP_ARROW,         ITM_UP1             },
  {62,                  ITM_4,                -MNU_BASE,            -MNU_BITS,            ITM_4,                ITM_R,                ITM_r,                ITM_4,                ITM_4               },
  {63,                  ITM_5,                -MNU_INTS,            -MNU_PARTS,           ITM_5,                ITM_S,                ITM_s,                ITM_5,                ITM_5               },
  {64,                  ITM_6,                -MNU_MATX,            -MNU_XFN,             ITM_6,                ITM_T,                ITM_t,                ITM_6,                ITM_6               },
  {65,                  ITM_MULT,             -MNU_EQN,             -MNU_ADV,             ITM_CROSS,            ITM_U,                ITM_u,                ITM_CROSS,            ITM_MULT            },
  {71,                  ITM_DOWN1,            ITM_SST,              ITM_FLGSV,            ITM_DOWN1,            ITM_DOWN1,            CHR_caseDN,           ITM_DOWN_ARROW,       ITM_DOWN1           },
  {72,                  ITM_1,                -MNU_PREF,            -MNU_KEYS,            ITM_1,                ITM_V,                ITM_v,                ITM_1,                ITM_1               },
  {73,                  ITM_2,                -MNU_UNITCONV,        -MNU_CLK,             ITM_2,                ITM_W,                ITM_w,                ITM_2,                ITM_2               },
  {74,                  ITM_3,                -MNU_FLAGS,           -MNU_ALPHAFN,         ITM_3,                ITM_X,                ITM_x,                ITM_3,                ITM_3               },
  {75,                  ITM_SUB,              -MNU_PROB,            -MNU_FIN,             ITM_MINUS,            ITM_Y,                ITM_y,                ITM_MINUS,            ITM_SUB             },
  {81,                  ITM_EXIT1,            ITM_OFF,              -MNU_INFO,            ITM_EXIT1,            ITM_EXIT1,            ITM_OFF,              ITM_SNAP,             ITM_EXIT1           },
  {82,                  ITM_0,                ITM_VIEW,             -MNU_IO,              ITM_0,                ITM_Z,                ITM_z,                ITM_0,                ITM_0               },
  {83,                  ITM_PERIOD,           ITM_SHOW,             ITM_TGLFRT,           ITM_PERIOD,           ITM_COMMA,            ITM_SEMICOLON,        ITM_PERIOD,           ITM_PERIOD          },
  {84,                  ITM_RS,               ITM_PR,               -MNU_PFN,             ITM_NULL,             ITM_QUESTION_MARK,    ITM_COLON,            ITM_SLASH,            ITM_RS              },
  {85,                  ITM_ADD,              -MNU_CATALOG,         -MNU_CONST,           ITM_PLUS,             ITM_SPACE,            -MNU_AIMCATALOG,      ITM_PLUS,             ITM_ADD             }
  //keyID,              primary,              fShifted,             gShifted,             keyLblAim,            primaryAim,           fShiftedAim,          gShiftedAim,          primaryTam
};


// R47bk_fg Layout from Layout_template_automation template: Do not change manually
//This variable is to store in flash memory
TO_QSPI const calcKey_t kbd_std_R47bk_fg[37] = {
  //keyID,              primary,              fShifted,             gShifted,             keyLblAim,            primaryAim,           fShiftedAim,          gShiftedAim,          primaryTam
  {21,                  ITM_SQUARE,           ITM_op_j,             ITM_toREC2,           ITM_NULL,             ITM_A,                ITM_a,                ITM_op_i_char,        ITM_REG_A           },
  {22,                  ITM_SQUAREROOTX,      ITM_op_j_pol,         ITM_toPOL2,           ITM_ROOT_SIGN,        ITM_B,                ITM_b,                ITM_ROOT_SIGN,        ITM_REG_B           },
  {23,                  ITM_1ONX,             ITM_XFACT,            ITM_ms,               ITM_NULL,             ITM_C,                ITM_c,                ITM_EXCLAMATION_MARK, ITM_REG_C           },
  {24,                  ITM_YX,               ITM_XTHROOT,          ITM_dotD,             ITM_NULL,             ITM_D,                ITM_d,                ITM_CIRCUMFLEX,       ITM_REG_D           },
  {25,                  ITM_LOG10,            ITM_10x,              ITM_RI,               ITM_NULL,             ITM_E,                ITM_e,                ITM_EulerE,           ITM_REG_E           },
  {26,                  ITM_LN,               ITM_EXP,              ITM_HASH_JM,          ITM_NUMBER_SIGN,      ITM_F,                ITM_f,                ITM_NUMBER_SIGN,      ITM_REG_F           },
  {31,                  ITM_STO,              ITM_MAGNITUDE,        ITM_ARG,              ITM_NULL,             ITM_G,                ITM_g,                ITM_VERTICAL_BAR,     ITM_REG_G           },
  {32,                  ITM_RCL,              ITM_PC,               ITM_DELTAPC,          ITM_NULL,             ITM_H,                ITM_h,                ITM_DELTA,            ITM_REG_H           },
  {33,                  ITM_Rdown,            ITM_CONSTpi,          ITM_Rup,              ITM_NULL,             ITM_I,                ITM_i,                ITM_pi,               ITM_REG_I           },
  {34,                  ITM_DRG,              ITM_USERMODE,         ITM_ASSIGN,           ITM_NULL,             ITM_J,                ITM_j,                ITM_op_j_char,        ITM_REG_J           },
  {35,                  ITM_NULL,             ITM_NULL,             ITM_NULL,             ITM_NULL,             ITM_NULL,             ITM_NULL,             ITM_NULL,             ITM_NULL            },
  {36,                  KEY_fg,               ITM_NULL,             ITM_NULL,             KEY_fg,               ITM_NULL,             ITM_NULL,             ITM_NULL,             KEY_fg              },
  {41,                  ITM_ENTER,            KEY_COMPLEX,          -MNU_CPX,             ITM_ENTER,            ITM_ENTER,            ITM_XEDIT,            ITM_CR,               ITM_ENTER           },
  {42,                  ITM_XexY,             ITM_LASTX,            -MNU_STK,             ITM_ex,               ITM_K,                ITM_k,                ITM_ex,               ITM_REG_K           },
  {43,                  ITM_CHS,              -MNU_DISP,            -MNU_TRG_R47,         ITM_PLUS_MINUS,       ITM_L,                ITM_l,                ITM_PLUS_MINUS,       ITM_REG_L           },
  {44,                  ITM_EXPONENT,         -MNU_PREFIX,          -MNU_EXP,             ITM_NULL,             ITM_M,                ITM_m,                ITM_EEXCHR,           ITM_REG_M           },
  {45,                  ITM_BACKSPACE,        ITM_UNDO,             -MNU_CLR,             ITM_BACKSPACE,        ITM_BACKSPACE,        ITM_CLA,              ITM_CLA,              ITM_BACKSPACE       },
  {51,                  ITM_XEQ,              ITM_AIM,              ITM_GTO,              ITM_NULL,             ITM_UNDERSCORE,       ITM_omega,            ITM_alpha,            ITM_alpha           },
  {52,                  ITM_7,                ITM_sin,              ITM_arcsin,           ITM_7,                ITM_N,                ITM_n,                ITM_7,                ITM_7               },
  {53,                  ITM_8,                ITM_cos,              ITM_arccos,           ITM_8,                ITM_O,                ITM_o,                ITM_8,                ITM_8               },
  {54,                  ITM_9,                ITM_tan,              ITM_arctan,           ITM_9,                ITM_P,                ITM_p,                ITM_9,                ITM_9               },
  {55,                  ITM_DIV,              -MNU_STAT,            -MNU_PLOTTING,        ITM_OBELUS,           ITM_Q,                ITM_q,                ITM_OBELUS,           ITM_DIV             },
  {61,                  ITM_UP1,              ITM_BST,              ITM_RBR,              ITM_UP1,              ITM_UP1,              CHR_caseUP,           ITM_UP_ARROW,         ITM_UP1             },
  {62,                  ITM_4,                -MNU_BASE,            -MNU_BITS,            ITM_4,                ITM_R,                ITM_r,                ITM_4,                ITM_4               },
  {63,                  ITM_5,                -MNU_INTS,            -MNU_PARTS,           ITM_5,                ITM_S,                ITM_s,                ITM_5,                ITM_5               },
  {64,                  ITM_6,                -MNU_MATX,            -MNU_XFN,             ITM_6,                ITM_T,                ITM_t,                ITM_6,                ITM_6               },
  {65,                  ITM_MULT,             -MNU_EQN,             -MNU_ADV,             ITM_CROSS,            ITM_U,                ITM_u,                ITM_CROSS,            ITM_MULT            },
  {71,                  ITM_DOWN1,            ITM_SST,              ITM_FLGSV,            ITM_DOWN1,            ITM_DOWN1,            CHR_caseDN,           ITM_DOWN_ARROW,       ITM_DOWN1           },
  {72,                  ITM_1,                -MNU_PREF,            -MNU_KEYS,            ITM_1,                ITM_V,                ITM_v,                ITM_1,                ITM_1               },
  {73,                  ITM_2,                -MNU_UNITCONV,        -MNU_CLK,             ITM_2,                ITM_W,                ITM_w,                ITM_2,                ITM_2               },
  {74,                  ITM_3,                -MNU_FLAGS,           -MNU_ALPHAFN,         ITM_3,                ITM_X,                ITM_x,                ITM_3,                ITM_3               },
  {75,                  ITM_SUB,              -MNU_PROB,            -MNU_FIN,             ITM_MINUS,            ITM_Y,                ITM_y,                ITM_MINUS,            ITM_SUB             },
  {81,                  ITM_EXIT1,            ITM_OFF,              -MNU_INFO,            ITM_EXIT1,            ITM_EXIT1,            ITM_OFF,              ITM_SNAP,             ITM_EXIT1           },
  {82,                  ITM_0,                ITM_VIEW,             -MNU_IO,              ITM_0,                ITM_Z,                ITM_z,                ITM_0,                ITM_0               },
  {83,                  ITM_PERIOD,           ITM_SHOW,             ITM_TGLFRT,           ITM_PERIOD,           ITM_COMMA,            ITM_SEMICOLON,        ITM_PERIOD,           ITM_PERIOD          },
  {84,                  ITM_RS,               ITM_PR,               -MNU_PFN,             ITM_NULL,             ITM_QUESTION_MARK,    ITM_COLON,            ITM_SLASH,            ITM_RS              },
  {85,                  ITM_ADD,              -MNU_CATALOG,         -MNU_CONST,           ITM_PLUS,             ITM_SPACE,            -MNU_AIMCATALOG,      ITM_PLUS,             ITM_ADD             }
  //keyID,              primary,              fShifted,             gShifted,             keyLblAim,            primaryAim,           fShiftedAim,          gShiftedAim,          primaryTam
};




// R47fg_bk Layout from Layout_template_automation template: Do not change manually
//This variable is to store in flash memory
TO_QSPI const calcKey_t kbd_std_R47fg_bk[37] = {
  //keyID,              primary,              fShifted,             gShifted,             keyLblAim,            primaryAim,           fShiftedAim,          gShiftedAim,          primaryTam
  {21,                  ITM_SQUARE,           ITM_op_j,             ITM_toREC2,           ITM_NULL,             ITM_A,                ITM_a,                ITM_op_i_char,        ITM_REG_A           },
  {22,                  ITM_SQUAREROOTX,      ITM_op_j_pol,         ITM_toPOL2,           ITM_ROOT_SIGN,        ITM_B,                ITM_b,                ITM_ROOT_SIGN,        ITM_REG_B           },
  {23,                  ITM_1ONX,             ITM_XFACT,            ITM_ms,               ITM_NULL,             ITM_C,                ITM_c,                ITM_EXCLAMATION_MARK, ITM_REG_C           },
  {24,                  ITM_YX,               ITM_XTHROOT,          ITM_dotD,             ITM_NULL,             ITM_D,                ITM_d,                ITM_CIRCUMFLEX,       ITM_REG_D           },
  {25,                  ITM_LOG10,            ITM_10x,              ITM_RI,               ITM_NULL,             ITM_E,                ITM_e,                ITM_EulerE,           ITM_REG_E           },
  {26,                  ITM_LN,               ITM_EXP,              ITM_HASH_JM,          ITM_NUMBER_SIGN,      ITM_F,                ITM_f,                ITM_NUMBER_SIGN,      ITM_REG_F           },
  {31,                  ITM_STO,              ITM_MAGNITUDE,        ITM_ARG,              ITM_NULL,             ITM_G,                ITM_g,                ITM_VERTICAL_BAR,     ITM_REG_G           },
  {32,                  ITM_RCL,              ITM_PC,               ITM_DELTAPC,          ITM_NULL,             ITM_H,                ITM_h,                ITM_DELTA,            ITM_REG_H           },
  {33,                  ITM_Rdown,            ITM_CONSTpi,          ITM_Rup,              ITM_NULL,             ITM_I,                ITM_i,                ITM_pi,               ITM_REG_I           },
  {34,                  ITM_DRG,              ITM_USERMODE,         ITM_ASSIGN,           ITM_NULL,             ITM_J,                ITM_j,                ITM_op_j_char,        ITM_REG_J           },
  {35,                  KEY_fg,               ITM_NULL,             ITM_NULL,             KEY_fg,               ITM_NULL,             ITM_NULL,             ITM_NULL,             KEY_fg              },
  {36,                  ITM_NULL,             ITM_NULL,             ITM_NULL,             ITM_NULL,             ITM_NULL,             ITM_NULL,             ITM_NULL,             ITM_NULL            },
  {41,                  ITM_ENTER,            KEY_COMPLEX,          -MNU_CPX,             ITM_ENTER,            ITM_ENTER,            ITM_XEDIT,            ITM_CR,               ITM_ENTER           },
  {42,                  ITM_XexY,             ITM_LASTX,            -MNU_STK,             ITM_ex,               ITM_K,                ITM_k,                ITM_ex,               ITM_REG_K           },
  {43,                  ITM_CHS,              -MNU_DISP,            -MNU_TRG_R47,         ITM_PLUS_MINUS,       ITM_L,                ITM_l,                ITM_PLUS_MINUS,       ITM_REG_L           },
  {44,                  ITM_EXPONENT,         -MNU_PREFIX,          -MNU_EXP,             ITM_NULL,             ITM_M,                ITM_m,                ITM_EEXCHR,           ITM_REG_M           },
  {45,                  ITM_BACKSPACE,        ITM_UNDO,             -MNU_CLR,             ITM_BACKSPACE,        ITM_BACKSPACE,        ITM_CLA,              ITM_CLA,              ITM_BACKSPACE       },
  {51,                  ITM_XEQ,              ITM_AIM,              ITM_GTO,              ITM_NULL,             ITM_UNDERSCORE,       ITM_omega,            ITM_alpha,            ITM_alpha           },
  {52,                  ITM_7,                ITM_sin,              ITM_arcsin,           ITM_7,                ITM_N,                ITM_n,                ITM_7,                ITM_7               },
  {53,                  ITM_8,                ITM_cos,              ITM_arccos,           ITM_8,                ITM_O,                ITM_o,                ITM_8,                ITM_8               },
  {54,                  ITM_9,                ITM_tan,              ITM_arctan,           ITM_9,                ITM_P,                ITM_p,                ITM_9,                ITM_9               },
  {55,                  ITM_DIV,              -MNU_STAT,            -MNU_PLOTTING,        ITM_OBELUS,           ITM_Q,                ITM_q,                ITM_OBELUS,           ITM_DIV             },
  {61,                  ITM_UP1,              ITM_BST,              ITM_RBR,              ITM_UP1,              ITM_UP1,              CHR_caseUP,           ITM_UP_ARROW,         ITM_UP1             },
  {62,                  ITM_4,                -MNU_BASE,            -MNU_BITS,            ITM_4,                ITM_R,                ITM_r,                ITM_4,                ITM_4               },
  {63,                  ITM_5,                -MNU_INTS,            -MNU_PARTS,           ITM_5,                ITM_S,                ITM_s,                ITM_5,                ITM_5               },
  {64,                  ITM_6,                -MNU_MATX,            -MNU_XFN,             ITM_6,                ITM_T,                ITM_t,                ITM_6,                ITM_6               },
  {65,                  ITM_MULT,             -MNU_EQN,             -MNU_ADV,             ITM_CROSS,            ITM_U,                ITM_u,                ITM_CROSS,            ITM_MULT            },
  {71,                  ITM_DOWN1,            ITM_SST,              ITM_FLGSV,            ITM_DOWN1,            ITM_DOWN1,            CHR_caseDN,           ITM_DOWN_ARROW,       ITM_DOWN1           },
  {72,                  ITM_1,                -MNU_PREF,            -MNU_KEYS,            ITM_1,                ITM_V,                ITM_v,                ITM_1,                ITM_1               },
  {73,                  ITM_2,                -MNU_UNITCONV,        -MNU_CLK,             ITM_2,                ITM_W,                ITM_w,                ITM_2,                ITM_2               },
  {74,                  ITM_3,                -MNU_FLAGS,           -MNU_ALPHAFN,         ITM_3,                ITM_X,                ITM_x,                ITM_3,                ITM_3               },
  {75,                  ITM_SUB,              -MNU_PROB,            -MNU_FIN,             ITM_MINUS,            ITM_Y,                ITM_y,                ITM_MINUS,            ITM_SUB             },
  {81,                  ITM_EXIT1,            ITM_OFF,              -MNU_INFO,            ITM_EXIT1,            ITM_EXIT1,            ITM_OFF,              ITM_SNAP,             ITM_EXIT1           },
  {82,                  ITM_0,                ITM_VIEW,             -MNU_IO,              ITM_0,                ITM_Z,                ITM_z,                ITM_0,                ITM_0               },
  {83,                  ITM_PERIOD,           ITM_SHOW,             ITM_TGLFRT,           ITM_PERIOD,           ITM_COMMA,            ITM_SEMICOLON,        ITM_PERIOD,           ITM_PERIOD          },
  {84,                  ITM_RS,               ITM_PR,               -MNU_PFN,             ITM_NULL,             ITM_QUESTION_MARK,    ITM_COLON,            ITM_SLASH,            ITM_RS              },
  {85,                  ITM_ADD,              -MNU_CATALOG,         -MNU_CONST,           ITM_PLUS,             ITM_SPACE,            -MNU_AIMCATALOG,      ITM_PLUS,             ITM_ADD             }
  //keyID,              primary,              fShifted,             gShifted,             keyLblAim,            primaryAim,           fShiftedAim,          gShiftedAim,          primaryTam
};




// R47fg_g Layout from Layout_template_automation template: Do not change manually
//This variable is to store in flash memory
TO_QSPI const calcKey_t kbd_std_R47fg_g[37] = {
  //keyID,              primary,              fShifted,             gShifted,             keyLblAim,            primaryAim,           fShiftedAim,          gShiftedAim,          primaryTam
  {21,                  ITM_SQUARE,           ITM_op_j,             ITM_toREC2,           ITM_NULL,             ITM_A,                ITM_a,                ITM_op_i_char,        ITM_REG_A           },
  {22,                  ITM_SQUAREROOTX,      ITM_op_j_pol,         ITM_toPOL2,           ITM_ROOT_SIGN,        ITM_B,                ITM_b,                ITM_ROOT_SIGN,        ITM_REG_B           },
  {23,                  ITM_1ONX,             ITM_XFACT,            ITM_ms,               ITM_NULL,             ITM_C,                ITM_c,                ITM_EXCLAMATION_MARK, ITM_REG_C           },
  {24,                  ITM_YX,               ITM_XTHROOT,          ITM_dotD,             ITM_NULL,             ITM_D,                ITM_d,                ITM_CIRCUMFLEX,       ITM_REG_D           },
  {25,                  ITM_LOG10,            ITM_10x,              ITM_RI,               ITM_NULL,             ITM_E,                ITM_e,                ITM_EulerE,           ITM_REG_E           },
  {26,                  ITM_LN,               ITM_EXP,              ITM_HASH_JM,          ITM_NUMBER_SIGN,      ITM_F,                ITM_f,                ITM_NUMBER_SIGN,      ITM_REG_F           },
  {31,                  ITM_STO,              ITM_MAGNITUDE,        ITM_ARG,              ITM_NULL,             ITM_G,                ITM_g,                ITM_VERTICAL_BAR,     ITM_REG_G           },
  {32,                  ITM_RCL,              ITM_PC,               ITM_DELTAPC,          ITM_NULL,             ITM_H,                ITM_h,                ITM_DELTA,            ITM_REG_H           },
  {33,                  ITM_Rdown,            ITM_CONSTpi,          ITM_Rup,              ITM_NULL,             ITM_I,                ITM_i,                ITM_pi,               ITM_REG_I           },
  {34,                  ITM_DRG,              ITM_USERMODE,         ITM_ASSIGN,           ITM_NULL,             ITM_J,                ITM_j,                ITM_op_j_char,        ITM_REG_J           },
  {35,                  KEY_fg,               ITM_NULL,             ITM_NULL,             KEY_fg,               ITM_NULL,             ITM_NULL,             ITM_NULL,             KEY_fg              },
  {36,                  ITM_SHIFTg,           ITM_NULL,             ITM_NULL,             ITM_SHIFTg,           ITM_NULL,             ITM_NULL,             ITM_NULL,             ITM_SHIFTg          },
  {41,                  ITM_ENTER,            KEY_COMPLEX,          -MNU_CPX,             ITM_ENTER,            ITM_ENTER,            ITM_XEDIT,            ITM_CR,               ITM_ENTER           },
  {42,                  ITM_XexY,             ITM_LASTX,            -MNU_STK,             ITM_ex,               ITM_K,                ITM_k,                ITM_ex,               ITM_REG_K           },
  {43,                  ITM_CHS,              -MNU_DISP,            -MNU_TRG_R47,         ITM_PLUS_MINUS,       ITM_L,                ITM_l,                ITM_PLUS_MINUS,       ITM_REG_L           },
  {44,                  ITM_EXPONENT,         -MNU_PREFIX,          -MNU_EXP,             ITM_NULL,             ITM_M,                ITM_m,                ITM_EEXCHR,           ITM_REG_M           },
  {45,                  ITM_BACKSPACE,        ITM_UNDO,             -MNU_CLR,             ITM_BACKSPACE,        ITM_BACKSPACE,        ITM_CLA,              ITM_CLA,              ITM_BACKSPACE       },
  {51,                  ITM_XEQ,              ITM_AIM,              ITM_GTO,              ITM_NULL,             ITM_UNDERSCORE,       ITM_omega,            ITM_alpha,            ITM_alpha           },
  {52,                  ITM_7,                ITM_sin,              ITM_arcsin,           ITM_7,                ITM_N,                ITM_n,                ITM_7,                ITM_7               },
  {53,                  ITM_8,                ITM_cos,              ITM_arccos,           ITM_8,                ITM_O,                ITM_o,                ITM_8,                ITM_8               },
  {54,                  ITM_9,                ITM_tan,              ITM_arctan,           ITM_9,                ITM_P,                ITM_p,                ITM_9,                ITM_9               },
  {55,                  ITM_DIV,              -MNU_STAT,            -MNU_PLOTTING,        ITM_OBELUS,           ITM_Q,                ITM_q,                ITM_OBELUS,           ITM_DIV             },
  {61,                  ITM_UP1,              ITM_BST,              ITM_RBR,              ITM_UP1,              ITM_UP1,              CHR_caseUP,           ITM_UP_ARROW,         ITM_UP1             },
  {62,                  ITM_4,                -MNU_BASE,            -MNU_BITS,            ITM_4,                ITM_R,                ITM_r,                ITM_4,                ITM_4               },
  {63,                  ITM_5,                -MNU_INTS,            -MNU_PARTS,           ITM_5,                ITM_S,                ITM_s,                ITM_5,                ITM_5               },
  {64,                  ITM_6,                -MNU_MATX,            -MNU_XFN,             ITM_6,                ITM_T,                ITM_t,                ITM_6,                ITM_6               },
  {65,                  ITM_MULT,             -MNU_EQN,             -MNU_ADV,             ITM_CROSS,            ITM_U,                ITM_u,                ITM_CROSS,            ITM_MULT            },
  {71,                  ITM_DOWN1,            ITM_SST,              ITM_FLGSV,            ITM_DOWN1,            ITM_DOWN1,            CHR_caseDN,           ITM_DOWN_ARROW,       ITM_DOWN1           },
  {72,                  ITM_1,                -MNU_PREF,            -MNU_KEYS,            ITM_1,                ITM_V,                ITM_v,                ITM_1,                ITM_1               },
  {73,                  ITM_2,                -MNU_UNITCONV,        -MNU_CLK,             ITM_2,                ITM_W,                ITM_w,                ITM_2,                ITM_2               },
  {74,                  ITM_3,                -MNU_FLAGS,           -MNU_ALPHAFN,         ITM_3,                ITM_X,                ITM_x,                ITM_3,                ITM_3               },
  {75,                  ITM_SUB,              -MNU_PROB,            -MNU_FIN,             ITM_MINUS,            ITM_Y,                ITM_y,                ITM_MINUS,            ITM_SUB             },
  {81,                  ITM_EXIT1,            ITM_OFF,              -MNU_INFO,            ITM_EXIT1,            ITM_EXIT1,            ITM_OFF,              ITM_SNAP,             ITM_EXIT1           },
  {82,                  ITM_0,                ITM_VIEW,             -MNU_IO,              ITM_0,                ITM_Z,                ITM_z,                ITM_0,                ITM_0               },
  {83,                  ITM_PERIOD,           ITM_SHOW,             ITM_TGLFRT,           ITM_PERIOD,           ITM_COMMA,            ITM_SEMICOLON,        ITM_PERIOD,           ITM_PERIOD          },
  {84,                  ITM_RS,               ITM_PR,               -MNU_PFN,             ITM_NULL,             ITM_QUESTION_MARK,    ITM_COLON,            ITM_SLASH,            ITM_RS              },
  {85,                  ITM_ADD,              -MNU_CATALOG,         -MNU_CONST,           ITM_PLUS,             ITM_SPACE,            -MNU_AIMCATALOG,      ITM_PLUS,             ITM_ADD             }
  //keyID,              primary,              fShifted,             gShifted,             keyLblAim,            primaryAim,           fShiftedAim,          gShiftedAim,          primaryTam
};



void fnAssign(uint16_t mode) {
  if(mode) {
    createMenu(aimBuffer);
    aimBuffer[0] = 0;
  }
  else {
    previousCalcMode = calcMode;
    calcMode = CM_ASSIGN;
    itemToBeAssigned = 0;
    updateAssignTamBuffer();
  }
}



void removeUserItemAssignments(int16_t userItem, char *userItemName) {
  bool_t deleteAllItems = false;

  itemToBeAssigned = ITM_NULL;
  if(userItemName[0] == 0) {
    deleteAllItems = true;
  }

  // Predefined configurable menus
  for(int i = 0; i < 18; ++i) {
    // MyMenu
    if((userMenuItems[i].item == userItem) && (userMenuItems[i].argumentName[0] != 0) &&
       ((deleteAllItems || compareString(userMenuItems[i].argumentName, userItemName, CMP_NAME) == 0))) {
      assignToMyMenu(i);
    }
    // MyAlpha
    if((userAlphaItems[i].item == userItem) && (userAlphaItems[i].argumentName[0] != 0) &&
       (deleteAllItems || (compareString(userAlphaItems[i].argumentName, userItemName, CMP_NAME) == 0))) {
      assignToMyAlpha(i);
    }
  }
  // User-defined menus
  for(int i = 0; i < numberOfUserMenus; ++i) {
    for(int j = 0; j < 18; ++j) {
      if((userMenus[i].menuItem[j].item == userItem) && (userMenus[i].menuItem[j].argumentName[0] != 0) &&
       (deleteAllItems || (compareString(userMenus[i].menuItem[j].argumentName, userItemName, CMP_NAME) == 0))) {
        _assignItem(&userMenus[i].menuItem[j]);
      }
    }
  }
  // Keys
  calcKey_t *key;
  char lbl[22];
  bool_t f = shiftF;
  bool_t g = shiftG;
  char kc[4] = {};
  for(int i = 0; i < 37; ++i) {
    key = kbd_usr + i;
    kc[0] = (i / 10) + '0';
    kc[1] = (i % 10) + '0';
    kc[2] = 0;
    if(key->primary == userItem) {
      stringToUtf8((char *)getNthString((uint8_t *)userKeyLabel, i*6),(uint8_t *)lbl);
      if((lbl[0] != 0) && (deleteAllItems || (compareString(lbl,userItemName, CMP_NAME) == 0))) {
        shiftF = false;
        shiftG = false;
        assignToKey(kc);
        #if defined(PC_BUILD)
          //printf("**[DL]** remove primary key %s assignment\n",kc);
        #endif //PC_BUILD
      }
    }
    if(key->fShifted == userItem) {
      stringToUtf8((char *)getNthString((uint8_t *)userKeyLabel, i*6+1),(uint8_t *)lbl);
      if((lbl[0] != 0) && (deleteAllItems || (compareString(lbl,userItemName, CMP_NAME) == 0))) {
        shiftF = true;
        shiftG = false;
        assignToKey(kc);
      }
    }
    if(key->gShifted == userItem) {
      stringToUtf8((char *)getNthString((uint8_t *)userKeyLabel, i*6+2),(uint8_t *)lbl);
      if((lbl[0] != 0) && (deleteAllItems || (compareString(lbl,userItemName, CMP_NAME) == 0))) {
        shiftF = false;
        shiftG = true;
        assignToKey(kc);
      }
    }
  }
  shiftF = f;
  shiftG = g;
}



void fnDeleteMenu(uint16_t id) {
  if(id >= numberOfUserMenus) {
    displayCalcErrorMessage(ERROR_CANNOT_DELETE_PREDEF_ITEM, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
    return;
  }
  else {
    removeUserItemAssignments(-MNU_DYNAMIC,userMenus[id].menuName);   // Remove assignments before deleting the user menu
    #if !defined(TESTSUITE_BUILD)
      removeUserMenuFromStack(id);                                    // Remove user menu from the stack before deleting it
    #endif // !TESTSUITE_BUILD
    if(numberOfUserMenus == 1) {
      freeC47Blocks(userMenus, TO_BLOCKS(sizeof(userMenu_t)));
      userMenus = NULL;
      numberOfUserMenus = 0;
    }
    else if(numberOfUserMenus > 1) {
      if(id < numberOfUserMenus - 1) {
        xcopy(userMenus + id, userMenus + id + 1, sizeof(userMenu_t) * (numberOfUserMenus - id - 1));
      }
      reduceC47Blocks(userMenus, TO_BLOCKS(sizeof(userMenu_t)) * numberOfUserMenus, TO_BLOCKS(sizeof(userMenu_t)) * (numberOfUserMenus - 1));
      --numberOfUserMenus;
    }
  }
  if(currentUserMenu > id) {
    --currentUserMenu;
  }
  #if !defined(TESTSUITE_BUILD)
    else if(currentUserMenu == id) {
      showSoftmenu(-MNU_DYNAMIC);
      popSoftmenu();
    }
  #endif // !TESTSUITE_BUILD
}



void fnDeleteUserMenus(uint16_t confirmation) {
  if((confirmation == NOT_CONFIRMED) && (programRunStop != PGM_RUNNING)) {
    setConfirmationMode(fnDeleteUserMenus);
  }
  else {
    removeUserItemAssignments(-MNU_DYNAMIC,"");           // Remove all user menus assignments
    #if !defined(TESTSUITE_BUILD)
      removeUserMenuFromStack(numberOfUserMenus);         // Remove all user menus from the stack before deleting them
    #endif // !TESTSUITE_BUILD
    freeC47Blocks(userMenus, TO_BLOCKS(sizeof(userMenu_t)) * numberOfUserMenus);
    userMenus = NULL;
    numberOfUserMenus = 0;
    #if !defined(TESTSUITE_BUILD)
      createHOME();
      createPFN();
    #endif // !TESTSUITE_BUILD
    if(programRunStop != PGM_RUNNING) {
      temporaryInformation = TI_DEL_ALL_MENUS;
    }
    else {
      temporaryInformation = TI_NO_INFO;
    }
    screenUpdatingMode = SCRUPD_AUTO;
  }
}



void fnClearUserMenus(uint16_t confirmation) {
  int i;
  if((confirmation == NOT_CONFIRMED) && (programRunStop != PGM_RUNNING)) {
    setConfirmationMode(fnClearUserMenus);
  }
  else {
    for(i=0; i<numberOfUserMenus; i++) {
      memset(userMenus[i].menuItem, 0, 18 * sizeof(userMenuItem_t));
    }
    #if !defined(TESTSUITE_BUILD)
      createHOME();
      createPFN();
    #endif // !TESTSUITE_BUILD
    if(programRunStop != PGM_RUNNING) {
      temporaryInformation = TI_CLEAR_ALL_MENUS;
    }
    else {
      temporaryInformation = TI_NO_INFO;
    }
    screenUpdatingMode = SCRUPD_AUTO;
  }
}



void updateAssignTamBuffer(void) {
  char *tbPtr = tamBuffer;
  tbPtr = stringCopy(tbPtr, "ASSIGN ");

  if(itemToBeAssigned == 0) {
    if(tam.alpha) {
      tbPtr = stringCopy(tbPtr, STD_LEFT_SINGLE_QUOTE);
      if(aimBuffer[0] == 0) {
        tbPtr = stringCopy(tbPtr, STD_CURSOR);
      }
      else {
        stringCopy(tmpString, aimBuffer);
        insertAlphaCursor(0);
        tbPtr = stringCopy(tbPtr, tmpString);
        tbPtr = stringCopy(tbPtr, STD_RIGHT_SINGLE_QUOTE);
      }
    }
    else {
      tbPtr = stringCopy(tbPtr, "_");
    }
  }
  else if(itemToBeAssigned == ASSIGN_CLEAR) {
    tbPtr = stringCopy(tbPtr, "NULL");
  }
  else if(itemToBeAssigned >= ASSIGN_LABELS) {
    uint8_t *lblPtr = labelList[itemToBeAssigned - ASSIGN_LABELS].labelPointer;
    uint32_t count = *(lblPtr++);
    for(uint32_t i=0; i<count; ++i) {
      *(tbPtr++) = *(lblPtr++);
    }
  }
  else if(itemToBeAssigned >= ASSIGN_RESERVED_VARIABLES) {
    tbPtr = stringCopy(tbPtr, (char *)allReservedVariables[itemToBeAssigned - ASSIGN_RESERVED_VARIABLES].reservedVariableName + 1);
  }
  else if(itemToBeAssigned >= ASSIGN_NAMED_VARIABLES) {
    tbPtr = stringCopy(tbPtr, (char *)allNamedVariables[itemToBeAssigned - ASSIGN_NAMED_VARIABLES].variableName + 1);
  }
  else if(itemToBeAssigned <= ASSIGN_USER_MENU) {
    tbPtr = stringCopy(tbPtr, userMenus[-(itemToBeAssigned - ASSIGN_USER_MENU)].menuName);
  }
  else if(itemToBeAssigned < 0) {
    tbPtr = stringCopy(tbPtr, indexOfItems[-itemToBeAssigned].itemCatalogName);
  }
  else if(indexOfItems[itemToBeAssigned].itemCatalogName[0] == 0) {
    tbPtr = stringCopy(tbPtr, indexOfItems[itemToBeAssigned].itemSoftmenuName);
  }
  else {
    tbPtr = stringCopy(tbPtr, indexOfItems[itemToBeAssigned].itemCatalogName);
  }

  tbPtr = stringCopy(tbPtr, " ");
  if(itemToBeAssigned != 0 && tam.alpha) {
    tbPtr = stringCopy(tbPtr, STD_LEFT_SINGLE_QUOTE);
    if(aimBuffer[0] == 0) {
      tbPtr = stringCopy(tbPtr, STD_CURSOR);
    }
    else {
      stringCopy(tmpString, aimBuffer);
      insertAlphaCursor(0);
      tbPtr = stringCopy(tbPtr, tmpString);
      tbPtr = stringCopy(tbPtr, STD_RIGHT_SINGLE_QUOTE);
    }
  }
  else if(itemToBeAssigned != 0 && shiftF) {
    tbPtr = stringCopy(tbPtr, STD_SUP_BOLD_f STD_CURSOR);
  }
  else if(itemToBeAssigned != 0 && shiftG) {
    tbPtr = stringCopy(tbPtr, STD_SUP_BOLD_g STD_CURSOR);
  }
  else {
    tbPtr = stringCopy(tbPtr, "_");
  }
}


void _assignItem(userMenuItem_t *menuItem) {
  const uint8_t *lblPtr = NULL;
  uint32_t l = 0;
  if(itemToBeAssigned == ASSIGN_CLEAR) {
    menuItem->item            = ITM_NULL;
    menuItem->argumentName[0] = 0;
  }
  else if(itemToBeAssigned >= ASSIGN_LABELS) {
    lblPtr                    = labelList[itemToBeAssigned - ASSIGN_LABELS].labelPointer;
    menuItem->item            = ITM_XEQ;
    //printf("**[DL]** menuItem->item %d lblPtr %s\n",menuItem->item,lblPtr);fflush(stdout);
  }
  else if(itemToBeAssigned >= ASSIGN_RESERVED_VARIABLES) {
    lblPtr                    = allReservedVariables[itemToBeAssigned - ASSIGN_RESERVED_VARIABLES].reservedVariableName;
    menuItem->item            = ITM_RCL;
  }
  else if(itemToBeAssigned >= ASSIGN_NAMED_VARIABLES) {
    lblPtr                    = allNamedVariables[itemToBeAssigned - ASSIGN_NAMED_VARIABLES].variableName;
    menuItem->item            = ITM_RCL;
  }
  else if(itemToBeAssigned <= ASSIGN_USER_MENU) {
    lblPtr                    = (uint8_t *)userMenus[-(itemToBeAssigned - ASSIGN_USER_MENU)].menuName;
    menuItem->item            = -MNU_DYNAMIC;
    xcopy(menuItem->argumentName, (char *)lblPtr, stringByteLength((char *)lblPtr) + 1);
    lblPtr                    = NULL;
  }
  else {
    menuItem->item            = itemToBeAssigned;
    menuItem->argumentName[0] = 0;
  }
  if(lblPtr) {
    l = (uint32_t)(*(lblPtr++));
    xcopy(menuItem->argumentName, (char *)lblPtr, l);
    menuItem->argumentName[l] = 0;
  }
}


void assignToMyMenu(uint16_t position) {
  if(position < 18) {
    _assignItem(&userMenuItems[position]);
  }
  cachedDynamicMenu = 0;
  refreshScreen(20);
}


void assignToMyAlpha(uint16_t position) {
  if(position < 18) {
    _assignItem(&userAlphaItems[position]);
  }
  cachedDynamicMenu = 0;
  refreshScreen(21);
}


void assignToUserMenu(uint16_t position) {
  if(position < 18) {
    _assignItem(&userMenus[currentUserMenu].menuItem[position]);
  }
  cachedDynamicMenu = 0;
  refreshScreen(22);
}


static int _typeOfFunction(int16_t func) {
  switch(func) {
    case ITM_NULL:      return 0;

    case ITM_EXIT1:
    case ITM_ENTER:
    case ITM_UP1:
    case ITM_DOWN1:
    case ITM_BACKSPACE: return 1;

    case ITM_0:
    case ITM_1:
    case ITM_2:
    case ITM_3:
    case ITM_4:
    case ITM_5:
    case ITM_6:
    case ITM_7:
    case ITM_8:
    case ITM_9:
    case ITM_PERIOD:
    case ITM_ADD:
    case ITM_SUB:
    case ITM_MULT:
    case ITM_DIV:       return 2;

    case ITM_A:
    case ITM_B:
    case ITM_C:
    case ITM_D:
    case ITM_E:
    case ITM_H:
    case ITM_I:
    case ITM_J:
    case ITM_K:
    case ITM_L:
    case ITM_O:         return 3;

    default:            return 4;
  }
}


static bool_t _assignTamAlpha(calcKey_t *key, uint16_t item) {
  switch(item) {
    case ITM_A: key->primaryTam = ITM_REG_A; return true;
    case ITM_B: key->primaryTam = ITM_REG_B; return true;
    case ITM_C: key->primaryTam = ITM_REG_C; return true;
    case ITM_D: key->primaryTam = ITM_REG_D; return true;
    case ITM_E: key->primaryTam = ITM_REG_E; return true;
    case ITM_H: key->primaryTam = ITM_REG_H; return true;
    case ITM_I: key->primaryTam = ITM_REG_I; return true;
    case ITM_J: key->primaryTam = ITM_REG_J; return true;
    case ITM_K: key->primaryTam = ITM_REG_K; return true;
    case ITM_L: key->primaryTam = ITM_REG_L; return true;
    case ITM_O: key->primaryTam = ITM_REG_O; return true;
    case ITM_M: key->primaryTam = ITM_REG_M; return true;
    case ITM_N: key->primaryTam = ITM_REG_N; return true;
    case ITM_P: key->primaryTam = ITM_REG_P; return true;
    case ITM_R: key->primaryTam = ITM_REG_R; return true;
    case ITM_S: key->primaryTam = ITM_REG_S; return true;
    case ITM_T: key->primaryTam = ITM_REG_T; return true;
    default:                                 return false;
  }
}


static bool_t _assignTamNum(calcKey_t *key, uint16_t item) {
  if(_typeOfFunction(item) == 2) {
    key->primaryTam = item;
    return true;
  }
  else {
    return false;
  }
}


void assignToKey(const char *data) {
  int keyCode = (*data - '0')*10 + *(data+1) - '0';
  calcKey_t *key = kbd_usr + keyCode;
  userMenuItem_t tmpMenuItem;
  keyStateCode = ((previousCalcMode) == CM_AIM ? 3 : 0) + (shiftG ? 2 : shiftF ? 1 : 0);
  const calcKey_t *stdKey = kbd_std + keyCode;

  _assignItem(&tmpMenuItem);
  switch(_typeOfFunction(tmpMenuItem.item)) {
    case 0:
      switch(keyStateCode) {
        case 5: key->gShiftedAim = stdKey->gShiftedAim; break;
        case 4: key->fShiftedAim = stdKey->fShiftedAim; break;
        case 3: key->primaryAim  = stdKey->primaryAim;
                key->primaryTam  = stdKey->primaryTam;
                break;
        case 2: key->gShifted    = stdKey->gShifted;    break;
        case 1: key->fShifted    = stdKey->fShifted;    break;
        case 0: key->primary     = stdKey->primary;
                key->primaryTam  = stdKey->primaryTam;
                _assignTamAlpha(key, key->primaryAim);
      }
      break;

    case 1:
      switch(keyStateCode) {
        case 5:
        case 2: key->gShiftedAim = key->gShifted                   = tmpMenuItem.item; break;
        case 4:
        case 1: key->fShiftedAim = key->fShifted                   = tmpMenuItem.item; break;
        case 3:
        case 0: key->primaryAim  = key->primary  = key->primaryTam = tmpMenuItem.item; break;
      }
      break;

    case 2:
      switch(keyStateCode) {
        case 5:  key->gShiftedAim = tmpMenuItem.item; break;
        case 4:
          key->fShiftedAim = tmpMenuItem.item;
          switch(tmpMenuItem.item) {
            case ITM_PLUS:      key->primary = ITM_ADD;  break;
            case ITM_MINUS:     key->primary = ITM_SUB;  break;
            case ITM_CROSS:
            case ITM_DOT:
            case ITM_PROD_SIGN: key->primary = ITM_MULT; break;
            case ITM_SLASH:     key->primary = ITM_DIV;  break;
            default:            key->primary = tmpMenuItem.item;
          }
          _assignTamNum(key, key->primary);
          break;
        case 3: key->primaryAim = tmpMenuItem.item;   break;
        case 2: key->gShifted = tmpMenuItem.item;     break;
        case 1: key->fShifted = tmpMenuItem.item;     break;
        case 0: {
          key->primary     = key->primaryTam = tmpMenuItem.item;
          switch(tmpMenuItem.item) {
            case ITM_ADD:  key->fShiftedAim = ITM_PLUS;      break;
            case ITM_SUB:  key->fShiftedAim = ITM_MINUS;     break;
            case ITM_MULT: key->fShiftedAim = ITM_PROD_SIGN; break;
            case ITM_DIV:  key->fShiftedAim = ITM_SLASH;     break;
            default:       key->fShiftedAim = tmpMenuItem.item;
          }
        }
      }
      break;

    case 3:
      switch(keyStateCode) {
        case 5: key->gShiftedAim = tmpMenuItem.item; break;
        case 4: key->fShiftedAim = tmpMenuItem.item; break;
        case 3: key->primaryAim  = tmpMenuItem.item;
                _assignTamAlpha(key, tmpMenuItem.item);
                break;
        case 2: key->gShifted    = tmpMenuItem.item; break;
        case 1: key->fShifted    = tmpMenuItem.item; break;
        case 0: key->primary     = tmpMenuItem.item;
                _assignTamAlpha(key, key->primaryAim) || (key->primaryTam = ITM_NULL);
      }
      break;

    default:
      switch(keyStateCode) {
        case 5: key->gShiftedAim = tmpMenuItem.item; break;
        case 4: key->fShiftedAim = tmpMenuItem.item; break;
        case 3: key->primaryAim  = tmpMenuItem.item;
                _assignTamAlpha(key, key->primaryAim) || _assignTamNum(key, key->primary) || (key->primaryTam = ITM_NULL);
                break;
        case 2: key->gShifted = tmpMenuItem.item;    break;
        case 1: key->fShifted = tmpMenuItem.item;    break;
        case 0: key->primary = tmpMenuItem.item;
                _assignTamNum(key, key->primary) || _assignTamAlpha(key, key->primaryAim) || (key->primaryTam = ITM_NULL);
      }
  }

  if(keyCode == 5) { // alpha
    key->primaryTam  = stdKey->primaryTam;
  }

  setUserKeyArgument(keyCode * 6 + keyStateCode, tmpMenuItem.argumentName);
}


void initUserKeyArgument(void) {
  userKeyLabelSize = 37/*keys*/ * 6/*states*/ * 1/*byte terminator*/ + 1/*byte sentinel*/;
  userKeyLabel = allocC47Blocks(TO_BLOCKS(userKeyLabelSize));
  memset(userKeyLabel,   0, TO_BYTES(TO_BLOCKS(userKeyLabelSize)));
}


void setUserKeyArgument(uint16_t position, const char *name) {
  char *userKeyLabelPtr1 = (char *)getNthString((uint8_t *)userKeyLabel, position);
  char *userKeyLabelPtr2 = (char *)getNthString((uint8_t *)userKeyLabel, position + 1);
  char *userKeyLabelPtr3 = (char *)getNthString((uint8_t *)userKeyLabel, 37 * 6);
  uint16_t newUserKeyLabelSize = userKeyLabelSize - stringByteLength(userKeyLabelPtr1) + stringByteLength(name);
  char *newUserKeyLabel = allocC47Blocks(TO_BLOCKS(newUserKeyLabelSize));
  char *newUserKeyLabelPtr = newUserKeyLabel;

  xcopy(newUserKeyLabelPtr, userKeyLabel, (int)(userKeyLabelPtr1 - userKeyLabel));
  newUserKeyLabelPtr += (int)(userKeyLabelPtr1 - userKeyLabel);
  xcopy(newUserKeyLabelPtr, name, stringByteLength(name));
  newUserKeyLabelPtr += stringByteLength(name);
  *(newUserKeyLabelPtr++) = 0;
  xcopy(newUserKeyLabelPtr, userKeyLabelPtr2, (int)(userKeyLabelPtr3 - userKeyLabelPtr2));
  newUserKeyLabelPtr += (int)(userKeyLabelPtr3 - userKeyLabelPtr2);
  *(newUserKeyLabelPtr++) = 0;

  freeC47Blocks(userKeyLabel, TO_BLOCKS(userKeyLabelSize));
  userKeyLabel = newUserKeyLabel;
  userKeyLabelSize = newUserKeyLabelSize;
}


void createMenu(const char *name) {
  if(validateName(name)) {
    if(isUniqueMenuName(name)) {
      if(numberOfUserMenus == 0) {
        userMenus = allocC47Blocks(TO_BLOCKS(sizeof(userMenu_t)));
      }
      else {
        userMenus = reallocC47Blocks(userMenus, TO_BLOCKS(sizeof(userMenu_t)) * numberOfUserMenus, TO_BLOCKS(sizeof(userMenu_t)) * (numberOfUserMenus + 1));
      }
      memset(userMenus + numberOfUserMenus, 0, sizeof(userMenu_t));
      #ifdef PC_BUILD
      if(name == NULL) {
        abortf("The parameter name is NULL!\n");
      }
      if(stringByteLength(name) + 1 > (int32_t)sizeof(userMenus[0].menuName)) {
        char tmp[1000];
        sprintf(tmp, "The string \"name\" <%s> is too long (%" PRId32 "+1 bytes) to be copied to userMenus[numberOfUserMenus].menuName (%" PRIi32 " bytes)\n",
                                           name,              stringByteLength(name),                                                    (int32_t)sizeof(userMenus[0].menuName));
        abortf(tmp);
      }
      #endif // PC_PUILD
      xcopy(userMenus[numberOfUserMenus].menuName, name, stringByteLength(name) + 1);
      ++numberOfUserMenus;
    }
    else {
      displayCalcErrorMessage(ERROR_ENTER_NEW_NAME, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "the name %s", name);
        moreInfoOnError("In function createMenu:", errorMessage, "is already in use!", NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }
  else {
    displayCalcErrorMessage(ERROR_INVALID_NAME, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function createMenu:", "the menu", name, "does not follow the naming convention");
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


void assignEnterAlpha(void) {
  #if !defined(TESTSUITE_BUILD)
    tam.alpha = true;
    setSystemFlag(FLAG_ALPHA);
    aimBuffer[0] = 0;
    tamEnterMode(ITM_ASSIGN);
    calcModeAim(NOPARAM);
  #endif // !TESTSUITE_BUILD
}


void assignLeaveAlpha(void) {
  #if !defined(TESTSUITE_BUILD)
    tam.alpha = false;
    clearSystemFlag(FLAG_ALPHA);
    leaveTamModeIfEnabled();
    alphaCursor = 0;
    calcModeNormalGui();
  #endif // !TESTSUITE_BUILD
}


void assignGetName1(void) {
  if(compareString(aimBuffer, "ENTER", CMP_NAME) == 0) {
    itemToBeAssigned = ITM_ENTER;
  }
  else if(compareString(aimBuffer, "EXIT", CMP_NAME) == 0) {
    itemToBeAssigned = ITM_EXIT1;
  }
  else if(compareString(aimBuffer, "USER", CMP_NAME) == 0) {
    itemToBeAssigned = ITM_USERMODE;
  }
  else if(compareString(aimBuffer, "UP", CMP_NAME) == 0) {
    itemToBeAssigned = ITM_UP1;
  }
  else if(compareString(aimBuffer, "DOWN", CMP_NAME) == 0) {
    itemToBeAssigned = ITM_DOWN1;
  }
  else if(compareString(aimBuffer, "BKSPC", CMP_NAME) == 0) {
    itemToBeAssigned = ITM_BACKSPACE;
  }
  /*else if(compareString(aimBuffer, STD_alpha, CMP_NAME) == 0) {
    itemToBeAssigned = ITM_AIM;
  }
  else if(compareString(aimBuffer, "f", CMP_NAME) == 0) {
    itemToBeAssigned = ITM_SHIFTf;
  }
  else if(compareString(aimBuffer, "g", CMP_NAME) == 0) {
    itemToBeAssigned = ITM_SHIFTg;
  }*/
  else if(aimBuffer[0] == 0) {
    itemToBeAssigned = ITM_NULL;
  }
  else {
    itemToBeAssigned = ASSIGN_CLEAR;

    // user-defined menus
    for(int i=0; i<numberOfUserMenus; ++i) {
      if(compareString(aimBuffer, userMenus[i].menuName, CMP_NAME) == 0) {
        itemToBeAssigned = ASSIGN_USER_MENU - i;
        break;
      }
    }

    // preset menus
    if(itemToBeAssigned == ASSIGN_CLEAR) {
      for(int i=0; softmenu[i].menuItem!=0; ++i) {
        if(compareString(aimBuffer, indexOfItems[-softmenu[i].menuItem].itemCatalogName, CMP_NAME) == 0) {
          itemToBeAssigned = softmenu[i].menuItem;
          break;
        }
      }
    }

    // programs
    if(itemToBeAssigned == ASSIGN_CLEAR) {
      itemToBeAssigned = findNamedLabel(aimBuffer);
      if(itemToBeAssigned == INVALID_VARIABLE) {
        itemToBeAssigned = ASSIGN_CLEAR;
      }
      else {
        itemToBeAssigned = itemToBeAssigned - FIRST_LABEL + ASSIGN_LABELS;
      }
    }

    // functions
    if(itemToBeAssigned == ASSIGN_CLEAR) {
      for(int i=0; i<LAST_ITEM; ++i) {
        if((indexOfItems[i].status & CAT_STATUS) == CAT_FNCT && compareString(aimBuffer, indexOfItems[i].itemCatalogName, CMP_NAME) == 0) {
          itemToBeAssigned = i;
          break;
        }
      }
    }
  }
}


static bool_t _assignToKey(int16_t keyFunc) {
  keyStateCode = (previousCalcMode) == CM_AIM ? 3 : 0;

  for(int i=0; i<3; ++i) {
    for(int j=0; j<37; ++j) {
      const calcKey_t *key = (getSystemFlag(FLAG_USER) ? kbd_usr : kbd_std) + j;
      int16_t kf = 0;
      switch(keyStateCode + i) {
        case 5: kf = key->gShiftedAim; break;
        case 4: kf = key->fShiftedAim; break;
        case 3: kf = key->primaryAim;  break;
        case 2: kf = key->gShifted;    break;
        case 1: kf = key->fShifted;    break;
        case 0: kf = key->primary;     break;
      }
      if(keyFunc == kf && (!getSystemFlag(FLAG_USER) || *getNthString((uint8_t *)userKeyLabel, j * 6 + keyStateCode + i) == 0)) {
        char kc[4] = {};
        kc[0] = (j / 10) + '0';
        kc[1] = (j % 10) + '0';
        kc[2] = 0;
        shiftF = (i == 1);
        shiftG = (i == 2);
        assignToKey(kc);
        return true;
      }
    }
  }
  return false;
}


void assignGetName2(void) {
  bool_t result = false;
  if(compareString(aimBuffer, "ENTER", CMP_NAME) == 0) {
    result = _assignToKey(ITM_ENTER);
  }
  else if(compareString(aimBuffer, "EXIT", CMP_NAME) == 0) {
    result = _assignToKey(ITM_EXIT1);
  }
  else if(compareString(aimBuffer, "USER", CMP_NAME) == 0) {
    result = _assignToKey(ITM_USERMODE);
  }
  else if(compareString(aimBuffer, "UP", CMP_NAME) == 0) {
    result = _assignToKey(ITM_UP1);
  }
  else if(compareString(aimBuffer, "DOWN", CMP_NAME) == 0) {
    result = _assignToKey(ITM_DOWN1);
  }
  else if(compareString(aimBuffer, "BKSPC", CMP_NAME) == 0) {
    result = _assignToKey(ITM_BACKSPACE);
  }
  /*else if(compareString(aimBuffer, STD_alpha, CMP_NAME) == 0) {
    result = _assignToKey(ITM_AIM);
  }
  else if(compareString(aimBuffer, "f", CMP_NAME) == 0) {
    result = _assignToKey(ITM_SHIFTf);
  }
  else if(compareString(aimBuffer, "g", CMP_NAME) == 0) {
    result = _assignToKey(ITM_SHIFTg);
  }*/
  calcMode = previousCalcMode;
  shiftF = shiftG = false;
  refreshScreen(23);

  if(!result) {
    displayCalcErrorMessage(ERROR_CANNOT_ASSIGN_HERE, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
    #if defined(PC_BUILD)
      moreInfoOnError("In function assignGetName2:", aimBuffer, "is invalid name.", NULL);
    #endif // PC_BUILD
  }
}
