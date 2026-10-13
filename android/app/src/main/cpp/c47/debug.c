// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file debug.c
 ***********************************************/

#include "c47.h"

TO_QSPI const char typeName[7][10 + 1][24 /* 21 characters + 1 sentinel + 2 padding */] = {
  {
    "???",
    "long integer",
    "real34",
    "complex34",
    "time",
    "date",
    "string",
    "real34 matrix",
    "complex34 matrix",
    "short integer",
    "config data",
    //"label",
    //"system integer",
    //"flags",
    //"pgm step",
    //"directory",
  },
  {
    "???                  ",
    "long integer         ",
    "real34               ",
    "complex34            ",
    "time                 ",
    "date                 ",
    "string               ",
    "real34 matrix        ",
    "complex34 matrix     ",
    "short integer        ",
    "config data          ",
    //"label                ",
    //"system integer       ",
    //"flags                ",
    //"pgm step             ",
    //"directory            ",
  },
  {
    "a ???",
    "a long integer",
    "a real34",
    "a complex34",
    "a time",
    "a date",
    "a string",
    "a real34 matrix",
    "a complex34 matrix",
    "a short integer",
    "a config data",
    //"a label",
    //"a system integer",
    //"a flags",
    //"a pgm step",
    //"a directory",
  },
  {
    "a ???                ",
    "a long integer       ",
    "a real34             ",
    "a complex34          ",
    "a time               ",
    "a date               ",
    "a string             ",
    "a real34 matrix      ",
    "a complex34 matrix   ",
    "a short integer      ",
    "a config data        ",
    //"a label              ",
    //"a system integer     ",
    //"a flags              ",
    //"a pgm step           ",
    //"a directory          ",
  },
  {
    "Linear     ",
    "Exponential",
    "Logarithmic",
    "Power      ",
    "Root       ",
    "Hyperbolic ",
    "Parabolic  ",
    "Cauchy peak",
    "Gauss peak ",
    "Orthogonal ",
    "???        "
  },
  {
    "a" STD_SUB_0 STD_SPACE_3_PER_EM "+" STD_SPACE_3_PER_EM "a" STD_SUB_1 "x",
    "a" STD_SUB_0 STD_SPACE_3_PER_EM "e^(a" STD_SUB_1 "x)",
    "a" STD_SUB_0 STD_SPACE_3_PER_EM "+" STD_SPACE_3_PER_EM "a" STD_SUB_1 "ln(x)",
    "a" STD_SUB_0 STD_SPACE_3_PER_EM "x^a" STD_SUB_1 ,
    "a" STD_SUB_0 STD_SPACE_3_PER_EM "a" STD_SUB_1 "^(1/x)",
    "(a" STD_SUB_0 STD_SPACE_3_PER_EM "+" STD_SPACE_3_PER_EM "a" STD_SUB_1 "x)" STD_SUP_MINUS STD_SUP_1,
    "a" STD_SUB_0 STD_SPACE_3_PER_EM "+" STD_SPACE_3_PER_EM "a" STD_SUB_1 "x" STD_SPACE_3_PER_EM "+" STD_SPACE_3_PER_EM "a" STD_SUB_2 "x" STD_SUP_2,
    STD_LEFT_SQUARE_BRACKET "a" STD_SUB_0 "(x+a" STD_SUB_1 ")" STD_SUP_2 "+a" STD_SUB_2 STD_RIGHT_SQUARE_BRACKET STD_SUP_MINUS STD_SUP_1,
    "a" STD_SUB_0 "e^" STD_LEFT_SQUARE_BRACKET  "(x-a" STD_SUB_1 ")" STD_SUP_2 "/a" STD_SUB_2 STD_RIGHT_SQUARE_BRACKET,
    "a" STD_SUB_0 STD_SPACE_3_PER_EM "+" STD_SPACE_3_PER_EM "a" STD_SUB_1 "x",
    "???        "
  },
  {
    "PGM_STOPPED    ",
    "PGM_RUNNING    ",
    "PGM_WAITING    ",
    "PGM_PAUSED     ",
    "PGM_KEY..PAUSED",
    "PGM_RESUMING   ",
    "PGM_SINGLE_STEP",
    "PGM_UNDEFINED  ",
    "???        ",
    "???        ",
    "???        "
  },
};



/********************************************//**
 * \brief Returns the name of a data type
 *
 * \param[in] dt uint16_t Data type
 * \return char*          Name of the data type
 ***********************************************/
const char *getDataTypeName(uint16_t dt, bool_t article, bool_t padWithBlanks) {
  if(article && padWithBlanks) {
    switch(dt) {
      case dtLongInteger:
      case dtTime:
      case dtDate:
      case dtString:
      case dtReal34Matrix:
      case dtComplex34Matrix:
      case dtShortInteger:
      case dtReal34:
      case dtComplex34:
      case dtConfig:
      //case dtLabel:
      //case dtSystemInteger:
      //case dtFlags:
      //case dtPgmStep:
      //case dtDirectory:
      {
        return typeName[3][dt + 1];
      }
      default: {
        return typeName[3][0];
      }
    }
  }
  else if(article && !padWithBlanks) {
    switch(dt) {
      case dtLongInteger:
      case dtTime:
      case dtDate:
      case dtString:
      case dtReal34Matrix:
      case dtComplex34Matrix:
      case dtShortInteger:
      case dtReal34:
      case dtComplex34:
      case dtConfig:
      //case dtLabel:
      //case dtSystemInteger:
      //case dtFlags:
      //case dtPgmStep:
      //case dtDirectory:
      {
        return typeName[2][dt + 1];
      }
      default: {
        return typeName[2][0];
      }
    }
  }
  else if(!article && padWithBlanks) {
    switch(dt) {
      case dtLongInteger:
      case dtTime:
      case dtDate:
      case dtString:
      case dtReal34Matrix:
      case dtComplex34Matrix:
      case dtShortInteger:
      case dtReal34:
      case dtComplex34:
      case dtConfig:
      //case dtLabel:
      //case dtSystemInteger:
      //case dtFlags:
      //case dtPgmStep:
      //case dtDirectory:
      {
        return typeName[1][dt + 1];
      }
      default: {
        return typeName[1][0];
      }
    }
  }
  else if(!article && !padWithBlanks) {
    switch(dt) {
      case dtLongInteger:
      case dtTime:
      case dtDate:
      case dtString:
      case dtReal34Matrix:
      case dtComplex34Matrix:
      case dtShortInteger:
      case dtReal34:
      case dtComplex34:
      case dtConfig:
      //case dtLabel:
      //case dtSystemInteger:
      //case dtFlags:
      //case dtPgmStep:
      //case dtDirectory:
      {
        return typeName[0][dt + 1];
      }
      default: {
        return typeName[0][0];
      }
    }
  }
  else {
    return typeName[0][0];
  }
}



/********************************************//**
 * \brief Returns the name of a data type of a register
 *
 * \param[in] dt calcRegister_t register
 * \return char* Name of the data type
 ***********************************************/
const char *getRegisterDataTypeName(calcRegister_t regist, bool_t article, bool_t padWithBlanks) {
  return getDataTypeName(getRegisterDataType(regist), article, padWithBlanks);
}



const char *getRegisterTagName(calcRegister_t regist, bool_t padWithBlanks) {
  static char base[9];

  switch(getRegisterDataType(regist)) {
    case dtLongInteger: {
      switch(getRegisterTag(regist)) {
        case LI_ZERO:     return "zero    ";
        case LI_NEGATIVE: return "negative";
        case LI_POSITIVE: return "positive";
        default:          return "???     ";
      }
    }

    case dtReal34: {
      switch(getRegisterTag(regist)) {
        case amRadian: return "radian  ";
        case amMultPi: return "multPi  ";
        case amGrad:   return "grad    ";
        case amDegree: return "degree  ";
        case amDMS:    return "dms     ";
        case amNone:   return "none    ";
        default:       return "???     ";
      }
    }

    case dtComplex34: {
      if(getComplexRegisterPolarMode(regist)) {
        return "Polar   ";
      }
      else {
        return "Rect    ";
      }
    }

    case dtString:
    case dtReal34Matrix:
    case dtComplex34Matrix:
    case dtDate:
    case dtTime:
    case dtConfig: {
      switch(getRegisterTag(regist)) {
        case amNone: return "        ";
        default:     return "???     ";
      }
    }

    case dtShortInteger: {
      sprintf(base, "base %2" PRIu32 " ", getRegisterTag(regist));
      return base;
    }

    default: {
      return "???     ";
    }
  }
}




/**********************************************//**
  Obtain the bitnumber for the power of two.
  Combination bits not supported, the first LSB bit scanned, is deemed to tbe the answer.
  The purpose is to decode the following table.

  CF_LINEAR_FITTING                          1  0
  CF_EXPONENTIAL_FITTING                     2  1
  CF_LOGARITHMIC_FITTING                     4  2
  CF_POWER_FITTING                           8  3
  CF_ROOT_FITTING                           16  4
  CF_HYPERBOLIC_FITTING                     32  5
  CF_PARABOLIC_FITTING                      64  6
  CF_CAUCHY_FITTING                        128  7
  CF_GAUSS_FITTING                         256  8
  CF_ORTHOGONAL_FITTING                    512  9
  other                                        10
  Output will be the bit number of the rightmost bit of the input.
 *************************************************/

uint16_t LogBaseTwoOfPowersOfTwo(uint16_t selection) {
  uint16_t jj = orOrtho(selection) & 0x03FF;
  jj = jj ^ (jj & (jj - 1));
  uint16_t r = (jj & 0xAAAA) != 0;
  r |= ((jj & 0xFF00) != 0) << 3;
  r |= ((jj & 0xF0F0) != 0) << 2;
  r |= ((jj & 0xCCCC) != 0) << 1;
  return r;
}



/********************************************//**
 * \param[in] am uint16_t curvefitting mode
 * \return char*          Name of the running mode
 ***********************************************/
const char *getRunningModeName(uint16_t mode) {
  return typeName[6][mode];
}



/********************************************//**
 * \brief Returns the single name of a curvefitting mode, or ??? if multiple names are defined in bits
 * \param[in] am uint16_t curvefitting mode
 * \return char*          Name of the curvefitting mode
 ***********************************************/
const char * getCurveFitModeName(uint16_t selection) {          //Can be only one bit. ??? if invalid.
  return typeName[4][LogBaseTwoOfPowersOfTwo(selection)];
}


/********************************************//**
 * \brief Remove trailing spaces from the curvefitting mode name
 *
 ***********************************************/
char *eatSpacesEnd(const char * ss) {
  static char tmp_names[20];
  int8_t ix;

  strcpy(tmp_names, ss);
  ix = stringByteLength(ss)-1;
  while(ix > 0) {
    if(ss[ix] == ' ') {
      tmp_names[ix] = 0;
    }
    else {
      break;
    }
    ix--;
  }
  return tmp_names;
}


/********************************************//**
 * \brief Remove spaces from the curvefitting mode name
 *
 ***********************************************/
char *eatSpacesMid(const char * ss) {
  static char tmp_names[20];
  char tt[50];
  int8_t ix = 0, iy = 0;

  strcpy(tt, ss);
  tmp_names[0] = 0;
  while(tt[ix] != 0 && ix < 50) {
    if(tt[ix] != ' ') {
      tmp_names[iy++] = tt[ix];
    }
    ix++;
  }
  tmp_names[iy] = 0;
  tmp_names[iy++] = 0;
  return tmp_names;
}


/********************************************//**
 * \brief Returns all selected names of the curve fit types
 * \note that a single bit EXCLUDES a method
 *
 * \param[in] dt uint16_t Data type
 * \return char*          Name of the curvefit type
 ***********************************************/
const char *getCurveFitModeNames(uint16_t selection) {
  #if defined(PC_BUILD)
  uint16_t ix;

  errorMessage[0] = 0;
  for(ix = 0; ix < 10; ix++) {
    if(selection & (1 << ix)) {
      strcat(errorMessage, errorMessage[0] == 0 ? "" : " ");
      strcat(errorMessage, eatSpacesEnd(getCurveFitModeName(1 << ix)));
    }
  }
  if(errorMessage[0] == 0) {
    return "???        ";
  }
  return errorMessage;
  #else // defined(PC_BUILD)
  return "";
  #endif // defined(PC_BUILD)
}


/********************************************//**
 * \brief Returns the formula of a curvefitting mode
 *
 * \param[in] am uint16_t curvefitting mode
 * \return char*          Formula of the curvefitting mode
 ***********************************************/
const char *getCurveFitModeFormula(uint16_t selection) {          //Can be only one bit. ??? if invalid.
  return typeName[5][LogBaseTwoOfPowersOfTwo(orOrtho(selection) & 0x03FF)];
}



/********************************************//**
 * \brief Returns the name of a angular mode
 *
 * \param[in] am uint16_t Angular mode
 * \return char*          Name of the angular mode
 ***********************************************/
const char *getAngularModeName(angularMode_t angularMode) {
  switch(angularMode) {
    case amRadian: return "radian";
    case amMultPi: return "multPi";
    case amGrad:   return "grad  ";
    case amDegree: return "degree";
    case amDMS:    return "d.ms  ";
    case amNone:   return "none  ";
    default:       return "???   ";
  }
}


/********************************************//**
 * \brief Returns the name of an integer mode
 *
 * \param[in] im uint16_t Integer mode
 * \return char*          Name of the integer mode
 ***********************************************/
const char *getShortIntegerModeName(uint16_t im) {
  switch(im) {
    case SIM_1COMPL: return "1compl";
    case SIM_2COMPL: return "2compl";
    case SIM_SIGNMT: return "signmt";
    case SIM_UNSIGN: return "unsign";
    default:         return "???   ";
  }
}


/* Never used
void debugNIM(void) {
  switch(nimNumberPart) {
    case NP_EMPTY:                printf("nimNumberPart = NP_EMPTY               \n");
    case NP_INT_10:               printf("nimNumberPart = NP_INT_10              \n");
    case NP_INT_16:               printf("nimNumberPart = NP_INT_16              \n");
    case NP_INT_BASE:             printf("nimNumberPart = NP_INT_BASE            \n");
    case NP_REAL_FLOAT_PART:      printf("nimNumberPart = NP_REAL_FLOAT_PART     \n");
    case NP_REAL_EXPONENT:        printf("nimNumberPart = NP_REAL_EXPONENT       \n");
    case NP_FRACTION_DENOMINATOR: printf("nimNumberPart = NP_FRACTION_DENOMINATOR\n");
    case NP_COMPLEX_INT_PART:     printf("nimNumberPart = NP_COMPLEX_INT_PART    \n");
    case NP_COMPLEX_FLOAT_PART:   printf("nimNumberPart = NP_COMPLEX_FLOAT_PART  \n");
    case NP_COMPLEX_EXPONENT:     printf("nimNumberPart = NP_COMPLEX_EXPONENT    \n");
    default:                      printf("nimNumberPart = NP_???                 \n");
  }
}
*/


#if (DEBUG_INSTEAD_STATUS_BAR == 1)
  /********************************************//**
   * \brief Returns the name of a calc mode
   *
   * \param[in] cm uint16_t Calc mode
   * \return char*          Name of the calc mode
   ***********************************************/
  char *getCalcModeName(uint16_t cm) {
    switch(cm) {
      case CM_NORMAL:           return "normal ";
      case CM_AIM:              return "aim    ";
      case CM_EIM:              return "eim    ";
      case CM_PEM:              return "pem    ";
      case CM_NIM:              return "nim    ";
      case CM_ASSIGN:           return "assign ";
      case CM_REGISTER_BROWSER: return "reg.bro";
      case CM_ASN_BROWSER:      return "asn.bro";
      case CM_FLAG_BROWSER:     return "flg.bro";
      case CM_FONT_BROWSER:     return "fnt.bro";
      case CM_PLOT_STAT:        return "plot.st";
      case CM_GRAPH:            return "plot.gr";
      case CM_ERROR_MESSAGE:    return "err.msg";
      case CM_BUG_ON_SCREEN:    return "bug.scr";
      case CM_TIMER:            return "timer  ";
      case CM_LISTXY:           return "listxy ";
      default:                  return "???    ";
    }
  }
#endif //((DEBUG_INSTEAD_STATUS_BAR == 1)


#if defined(PC_BUILD )
  ///////////////////////////////
  // Stack smashing detection
  void stackCheck(const unsigned char *begin, const unsigned char *end, int size, const char *where) {
     int i, corrupted = 0;

     for(i=0; i<size; i++) {
       if(*(begin + i) != 0xaa) {
         printf("Stack begin corrupted: begin[%d]=0x%02x at %s\n", i, begin[i], where);
         corrupted = 1;
       }
     }

     for(i=0; i<size; i++) {
       if(*(end + i) != 0xaa) {
         printf("Stack end corrupted: end[%d]=0x%02x at %s\n", i, end[i], where);
         corrupted = 1;
       }
     }

     if(corrupted) {
       exit(0xBAD);
     }
  }


  void initStackCheck(unsigned char *begin, unsigned char *end, int size) {
    memset(begin, 0xaa, size);
    memset(end,   0xaa, size);
  }

  //////////////////////////////////////////////////
  // Example of stack smashing tests in a function
  void stackSmashingTest(void) {
                                                unsigned char stackEnd[10000]; // First declaration
    int v1, v2, v3;
                                                unsigned char stackBegin[10000]; // Last declaration

                                                initStackCheck(stackBegin, stackEnd, 10000);

    v1 = 1;
                                                stackCheck(stackBegin, stackEnd, 10000, "after v1 = ...");
    v2 = v1 + 1;
                                                stackCheck(stackBegin, stackEnd, 10000, "after v2 = ...");
    v3 = v2 + 2;
                                                stackCheck(stackBegin, stackEnd, 10000, "after v3 = ...");
    printf("v1=%d v2=%d v3=%d\n", v1, v2, v3);
                                                stackCheck(stackBegin, stackEnd, 10000, "after printf(...");
  }
#endif // PC_BUILD
