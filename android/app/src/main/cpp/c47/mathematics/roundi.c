// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file roundi.c
 ***********************************************/

#include "c47.h"

TO_QSPI void (* const Roundi[NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS])(void) = {
// regX ==> 1            2           3            4            5            6            7           8            9             10
//          Long integer Real34      Complex34    Time         Date         String       Real34 mat  Complex34 m  Short integer Config data
            doNothing,   roundiReal, roundiError, roundiError, roundiError, roundiError, roundiRema, roundiError, doNothing,   roundiError
};



/********************************************//**
 * \brief Data type error in roundi
 *
 * \param void
 * \return void
 ***********************************************/
#if (EXTRA_INFO_ON_CALC_ERROR == 1)
  void roundiError(void) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    sprintf(errorMessage, "cannot calculate ROUNDI for %s", getRegisterDataTypeName(REGISTER_X, true, false));
    moreInfoOnError("In function roundiError:", errorMessage, NULL, NULL);
  }
#endif // (EXTRA_INFO_ON_CALC_ERROR == 1)



/********************************************//**
 * \brief regX ==> regL and roundi(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnRoundi(uint16_t unusedButMandatoryParameter) {
  if(!saveLastX()) {
    return;
  }

  Roundi[getRegisterDataType(REGISTER_X)]();

  adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
}



void roundiRema(void) {
  real34Matrix_t x;
  uint32_t elements;

  linkToRealMatrixRegister(REGISTER_X, &x);
  elements = x.header.matrixRows * x.header.matrixColumns;

  for(uint32_t i = 0; i < elements; i++) {
    if(real34IsSpecial(x.matrixElements + i)) {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      return;
    }
  }

  for(uint32_t i = 0; i < elements; i++) {
    real34ToIntegralValue(x.matrixElements + i, x.matrixElements + i, DEC_ROUND_HALF_UP);
  }
}



void roundiReal(void) {
  longInteger_t rounded;

  if(real34IsNaN(REGISTER_REAL34_DATA(REGISTER_X))) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function roundiReal:", "cannot use NaN as X input of ROUNDI", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  if(real34IsInfinite(REGISTER_REAL34_DATA(REGISTER_X))) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function roundiReal:", "cannot use ±∞ as an input of ROUNDI", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  convertReal34ToLongInteger(REGISTER_REAL34_DATA(REGISTER_X), rounded, DEC_ROUND_HALF_UP);
  if(longIntegerBits(rounded) > MAX_LONG_INTEGER_SIZE_IN_BITS) {
    displayCalcErrorMessage(real34IsNegative(REGISTER_REAL34_DATA(REGISTER_X)) ? ERROR_OVERFLOW_MINUS_INF : ERROR_OVERFLOW_PLUS_INF, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "The rounded integer needs %" PRIu32 " bits, exceeding the %" PRId16 " bit limit!", longIntegerBits(rounded), (uint16_t)MAX_LONG_INTEGER_SIZE_IN_BITS);
      moreInfoOnError("In function roundiReal:", errorMessage, "", "");
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    longIntegerFree(rounded);
    return;
  }

  convertLongIntegerToLongIntegerRegister(rounded, REGISTER_X);
  longIntegerFree(rounded);
}
