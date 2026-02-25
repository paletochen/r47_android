// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file square.c
 ***********************************************/

#include "c47.h"

static void squareLonI(void) {
  longInteger_t lgInt;

  if(!getRegisterAsLongInt(REGISTER_X, lgInt, NULL)) {
    goto end;
  }

  longIntegerMultiply(lgInt, lgInt, lgInt);
  convertLongIntegerToLongIntegerRegister(lgInt, REGISTER_X);
end:
  longIntegerFree(lgInt);
}

static void squareShoI(void) {
  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = WP34S_intMultiply(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)), *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)));
}

static void squareReal(void) {
  real_t x;

  if(!getRegisterAsReal(REGISTER_X, &x))
    return;

  if(realIsInfinite(&x) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function squareReal:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input of curt when flag D is not set", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  realMultiply(&x, &x, &x, &ctxtReal39);
  convertRealToResultRegister(&x, REGISTER_X, amNone);
}

static void squareCplx(void) {
  real_t a, b;

  if(getRegisterAsComplex(REGISTER_X, &a, &b)) {
    mulComplexComplex(&a, &b, &a, &b, &a, &b, &ctxtReal39);
    convertComplexToResultRegister(&a, &b, REGISTER_X);
  }
}

/********************************************//**
 * \brief regX ==> regL and regX × regX ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter
 * \return void
 ***********************************************/
void fnSquare(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexMonadicFunction(&squareReal, &squareCplx, &squareShoI, &squareLonI);
}
