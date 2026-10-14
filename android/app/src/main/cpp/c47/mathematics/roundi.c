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
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
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
  elementwiseRema(roundiReal);
}



void roundiReal(void) {
  if(real34IsNaN(REGISTER_REAL34_DATA(REGISTER_X))) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function roundiReal:", "cannot use NaN as X input of ROUNDI", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  if(real34IsInfinite(REGISTER_REAL34_DATA(REGISTER_X))) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function roundiReal:", "cannot use ±∞ as an input of ROUNDI", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  int32_t exponent = real34GetExponent(REGISTER_REAL34_DATA(REGISTER_X));
  if(exponent > (int32_t)(0.301029995663 * MAX_LONG_INTEGER_SIZE_IN_BITS)) {
    displayCalcErrorMessage(!real34CompareLessThan(REGISTER_REAL34_DATA(REGISTER_X), const34_0) ? ERROR_OVERFLOW_PLUS_INF : ERROR_OVERFLOW_MINUS_INF, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Converting a real exponent of %" PRId32 " would result in a value exceeding %" PRId16 " bits!", exponent, (uint16_t)MAX_LONG_INTEGER_SIZE_IN_BITS);
      moreInfoOnError("In function roundiReal:", errorMessage, "", "");
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

  convertReal34ToLongIntegerRegister(REGISTER_REAL34_DATA(REGISTER_X), REGISTER_X, DEC_ROUND_HALF_UP);
}
