// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file gcd.c
 ***********************************************/

#include "c47.h"

static void _longIntegerGcd(longInteger_t liY, longInteger_t liX, longInteger_t liA) {
  if(longIntegerIsZero(liY) && longIntegerIsZero(liX)) {
    #if !defined(TESTSUITE_BUILD)
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function _longIntegerGcd:", "(0, 0) is not in the function domain.", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    #endif //TESTSUITE_BUILD
  } else {
    longIntegerGcd(liY, liX, liA);
  }
}

static void gcdInt(void) {
  longInteger_t liX, liY;
  bool_t fracX, fracY;

  if(!getRegisterAsLongInt(REGISTER_Y, liY, &fracY)) {
    goto end1;
  }
  if(!getRegisterAsLongInt(REGISTER_X, liX, &fracX)) {
    goto end2;
  }

  if (fracX) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    goto end2;
  }
  if (fracY) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_Y);
    goto end2;
  }

  longIntegerSetPositiveSign(liY);
  longIntegerSetPositiveSign(liX);
  _longIntegerGcd(liY, liX, liX);
  convertLongIntegerToLongIntegerRegister(liX, REGISTER_X);
end2:
  longIntegerFree(liX);
end1:
  longIntegerFree(liY);
}

static void gcdShoI(void) {
  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = WP34S_intGCD(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_Y)), *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)));
}

/********************************************//**
 * \brief regX ==> regL and GDC(regY, regX) ==> regX
 * Drops Y, enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnGcd(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexDyadicFunction(&gcdInt, NULL, &gcdShoI, &gcdInt);
}

