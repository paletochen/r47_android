// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file atan2.c
 ***********************************************/

#include "c47.h"

TO_QSPI void (* const arctan2[NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS][NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS])(void) = {
// regX |    regY ==>   1              2              3           4           5           6           7              8           9             10
//      V               Long integer   Real34         Complex34   Time        Date        String      Real34 mat     Complex34 m Short integer Config data
/*  1 Long integer  */ {atan2RealReal, atan2RealReal, atan2Error, atan2Error, atan2Error, atan2Error, atan2RemaReal, atan2Error, atan2Error,   atan2Error},
/*  2 Real34        */ {atan2RealReal, atan2RealReal, atan2Error, atan2Error, atan2Error, atan2Error, atan2RemaReal, atan2Error, atan2Error,   atan2Error},
/*  3 Complex34     */ {atan2Error,    atan2Error,    atan2Error, atan2Error, atan2Error, atan2Error, atan2Error,    atan2Error, atan2Error,   atan2Error},
/*  4 Time          */ {atan2Error,    atan2Error,    atan2Error, atan2Error, atan2Error, atan2Error, atan2Error,    atan2Error, atan2Error,   atan2Error},
/*  5 Date          */ {atan2Error,    atan2Error,    atan2Error, atan2Error, atan2Error, atan2Error, atan2Error,    atan2Error, atan2Error,   atan2Error},
/*  6 String        */ {atan2Error,    atan2Error,    atan2Error, atan2Error, atan2Error, atan2Error, atan2Error,    atan2Error, atan2Error,   atan2Error},
/*  7 Real34 mat    */ {atan2LonIRema, atan2RealRema, atan2Error, atan2Error, atan2Error, atan2Error, atan2RemaRema, atan2Error, atan2Error,   atan2Error},
/*  8 Complex34 mat */ {atan2Error,    atan2Error,    atan2Error, atan2Error, atan2Error, atan2Error, atan2Error,    atan2Error, atan2Error,   atan2Error},
/*  9 Short integer */ {atan2Error,    atan2Error,    atan2Error, atan2Error, atan2Error, atan2Error, atan2RemaReal, atan2Error, atan2Error,   atan2Error},
/* 10 Config data   */ {atan2Error,    atan2Error,    atan2Error, atan2Error, atan2Error, atan2Error, atan2Error,    atan2Error, atan2Error,   atan2Error}
};



/********************************************//**
 * \brief Data type error in arctan
 *
 * \param void
 * \return void
 ***********************************************/
#if (EXTRA_INFO_ON_CALC_ERROR == 1)
  void atan2Error(void) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
    sprintf(errorMessage, "cannot calculate atan2 for %s and %s", getRegisterDataTypeName(REGISTER_Y, true, false), getRegisterDataTypeName(REGISTER_X, true, false));
    moreInfoOnError("In function fnAtan2:", errorMessage, NULL, NULL);
  }
#endif // (EXTRA_INFO_ON_CALC_ERROR == 1)



/********************************************//**
 * \brief regX ==> regL and atan2(regY, regX) ==> regX
 * Drops Y, enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnAtan2(uint16_t unusedButMandatoryParameter) {
  if(!saveLastX()) {
    return;
  }

  arctan2[getRegisterDataType(REGISTER_X)][getRegisterDataType(REGISTER_Y)]();

  adjustResult(REGISTER_X, true, true, REGISTER_X, REGISTER_Y, -1);
}



void atan2RealReal(void) {
  real_t y, x;

  if(!getRegisterAsReal(REGISTER_X, &x) || !getRegisterAsReal(REGISTER_Y, &y))
    return;

  if(realIsZero(&y) && realIsZero(&x) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function atan2RealReal:", "X = 0 and Y = 0", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }
  WP34S_Atan2(&y, &x, &x, &ctxtReal39);
  convertAngleFromTo(&x, amRadian, currentAngularMode, &ctxtReal39);
  reallocateRegister(REGISTER_X, dtReal34, 0, currentAngularMode);
  convertRealToReal34ResultRegister(&x, REGISTER_X);
}



void atan2RemaRema(void) {
  real34Matrix_t y, x;

  linkToRealMatrixRegister(REGISTER_Y, &y);
  linkToRealMatrixRegister(REGISTER_X, &x);

  if(y.header.matrixRows == x.header.matrixRows && y.header.matrixColumns == x.header.matrixColumns) {
    for(int i = 0; i < x.header.matrixRows * x.header.matrixColumns; ++i) {
      real_t yy, xx;
      real34ToReal(&y.matrixElements[i], &yy);
      real34ToReal(&x.matrixElements[i], &xx);
      if(realIsZero(&yy) && realIsZero(&xx) && !getSystemFlag(FLAG_SPCRES)) {
        displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function atan2RemaRema:", "X = 0 and Y = 0", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return;
      }
      WP34S_Atan2(&yy, &xx, &xx, &ctxtReal39);
      convertAngleFromTo(&xx, amRadian, currentAngularMode, &ctxtReal39);
      roundToSignificantDigits(&xx, &xx, significantDigits == 0 ? 34 : significantDigits, &ctxtReal75);
      realToReal34(&xx, &x.matrixElements[i]);
    }
  }
  else {
    displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot calculate atan2 with %d" STD_CROSS "%d-matrix and %d" STD_CROSS "%d-matrix",
              x.header.matrixRows, x.header.matrixColumns,
              y.header.matrixRows, y.header.matrixColumns);
      moreInfoOnError("In function atan2RemaRema:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}



void atan2RemaReal(void) {
  elementwiseRemaReal(atan2RealReal);
}



void atan2RealRema(void) {
  real_t y;
  real34Matrix_t x;

  real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &y);
  linkToRealMatrixRegister(REGISTER_X, &x);

  for(int i = 0; i < x.header.matrixRows * x.header.matrixColumns; ++i) {
    real_t xx;
    real34ToReal(&x.matrixElements[i], &xx);
    if(realIsZero(&y) && realIsZero(&xx) && !getSystemFlag(FLAG_SPCRES)) {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function atan2RealRema:", "X = 0 and Y = 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
    WP34S_Atan2(&y, &xx, &xx, &ctxtReal39);
    convertAngleFromTo(&xx, amRadian, currentAngularMode, &ctxtReal39);
    roundToSignificantDigits(&xx, &xx, significantDigits == 0 ? 34 : significantDigits, &ctxtReal75);
    realToReal34(&xx, &x.matrixElements[i]);
  }
}



void atan2LonIRema(void) {
  convertLongIntegerRegisterToReal34Register(REGISTER_Y, REGISTER_Y);
  atan2RealRema();
}
