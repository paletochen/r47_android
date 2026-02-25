// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file cube.c
 ***********************************************/

#include "c47.h"

static void cubeLonI(void) {
  longInteger_t x, c;

  if(!getRegisterAsLongInt(REGISTER_X, x, NULL)) {
    goto end;
  }

  longIntegerInit(c);
  longIntegerMultiply(x, x, c);
  longIntegerMultiply(c, x, c);
  convertLongIntegerToLongIntegerRegister(c, REGISTER_X);
  longIntegerFree(c);
end:
  longIntegerFree(x);
}

static void cubeShoI(void) {
  uint64_t square;

  // TODO: set the overflow/carry flags properly
  square = WP34S_intMultiply(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)), *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)));
  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = WP34S_intMultiply(square, *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)));
}

static void cubeReal(void) {
  real_t x, xSquared;

  if(!getRegisterAsReal(REGISTER_X, &x))
    return;

  if(realIsInfinite(&x) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function cubeReal:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input of curt when flag D is not set", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  realMultiply(&x, &x, &xSquared, &ctxtReal39);
  realMultiply(&xSquared, &x, &x, &ctxtReal39);
  convertRealToResultRegister(&x, REGISTER_X, amNone);
}

static void cubeCplx(void) {
  real_t a, b, realSquare, imagSquare;

  if(getRegisterAsComplex(REGISTER_X, &a, &b)) {
    mulComplexComplex(&a, &b, &a, &b, &realSquare, &imagSquare, &ctxtReal39);
    mulComplexComplex(&realSquare, &imagSquare, &a, &b, &a, &b, &ctxtReal39);
    convertComplexToResultRegister(&a, &b, REGISTER_X);
  }
}

/********************************************//**
 * \brief regX ==> regL and regX × regX × regX ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter
 * \return void
 ***********************************************/
void fnCube(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexMonadicFunction(&cubeReal, &cubeCplx, &cubeShoI, &cubeLonI);
}

