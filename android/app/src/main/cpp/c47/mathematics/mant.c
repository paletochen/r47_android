// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file mant.c
 ***********************************************/

#include "c47.h"

TO_QSPI void (* const mant[NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS])(void) = {
// regX ==> 1            2         3          4          5          6          7          8           9             10
//          Long integer Real34    complex34  Time       Date       String     Real34 mat Complex34 m Short integer Config data
            mantLonI,    mantReal, mantError, mantError, mantError, mantError, mantError, mantError,  mantError,    mantError
};



/********************************************//**
 * \brief Data type error in mant
 *
 * \param void
 * \return void
 ***********************************************/
#if (EXTRA_INFO_ON_CALC_ERROR == 1)
  void mantError(void) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
    sprintf(errorMessage, "cannot calculate MANT for %s", getRegisterDataTypeName(REGISTER_X, true, false));
    moreInfoOnError("In function mantError:", errorMessage, NULL, NULL);
  }
#endif // (EXTRA_INFO_ON_CALC_ERROR == 1)



/********************************************//**
 * \brief regX ==> regL and mant(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnMant(uint16_t unusedButMandatoryParameter) {
  if(!saveLastX()) {
    return;
  }

  mant[getRegisterDataType(REGISTER_X)]();

  adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
}



void mantLonI(void) {
  real_t x;

  convertLongIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
  x.exponent = 1 - x.digits;
  reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
  convertRealToReal34ResultRegister(&x, REGISTER_X);
}



void mantReal(void) {
  if(real34IsNaN(REGISTER_REAL34_DATA(REGISTER_X))) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function mantReal:", "cannot use NaN as X input of MANT", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  real_t x;

  real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &x);
  x.exponent = 1 - x.digits;
  convertRealToReal34ResultRegister(&x, REGISTER_X);
  setRegisterAngularMode(REGISTER_X, amNone);
}
