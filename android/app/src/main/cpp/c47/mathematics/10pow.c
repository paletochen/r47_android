// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file 10pow.c
 ***********************************************/

#include "c47.h"

static void tenPowReal(void);


void realPower10(const real_t *x, real_t *res, realContext_t *realContext) {
  realMultiply(x, const_ln10, res, realContext);
  realExp(res, res, realContext);
}


static void tenPowLonI(void) {
  int32_t exponentSign;
  longInteger_t base, exponent;

  longIntegerInit(base);
  uInt32ToLongInteger(10u, base);
  convertLongIntegerRegisterToLongInteger(REGISTER_X, exponent);

  longIntegerSetPositiveSign(base);

  exponentSign = longIntegerSign(exponent);
  longIntegerSetPositiveSign(exponent);

  if(longIntegerIsZero(exponent)) {
    uInt32ToLongInteger(1u, base);
    convertLongIntegerToLongIntegerRegister(base, REGISTER_X);
    longIntegerFree(base);
    longIntegerFree(exponent);
    return;
  }
  else if(exponentSign == -1) {
    longIntegerFree(base);
    longIntegerFree(exponent);
    tenPowReal();
    return;
  }

  longInteger_t power;

  longIntegerInit(power);
  uInt32ToLongInteger(1u, power);

  while(!longIntegerIsZero(exponent) && lastErrorCode == 0) {
    if(longIntegerIsOdd(exponent)) {
     longIntegerMultiply(power, base, power);
    }

    longIntegerDivideUInt(exponent, 2, exponent);

    if(!longIntegerIsZero(exponent)) {
      longIntegerSquare(base, base);
    }
  }

  convertLongIntegerToLongIntegerRegister(power, REGISTER_X);

  longIntegerFree(power);
  longIntegerFree(base);
  longIntegerFree(exponent);
}


static void tenPowShoI(void) {
  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = WP34S_int10pow(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)));
}



void intPowReal(void (*powf)(const real_t *x, real_t *res, realContext_t *realContext)) {
  real_t x;

  if (!getRegisterAsReal(REGISTER_X, &x))
    return;

  if(realIsSpecial(&x) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function intPowReal:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input of 10" STD_SUP_BOLD_x " when flag D is not set", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  (*powf)(&x, &x, &ctxtReal39);
  convertRealToResultRegister(&x, REGISTER_X, amNone);
}

static void tenPowReal(void) {
  intPowReal(&realPower10);
}


void intPowCplx(const real_t *lnBase) {
  real_t a, b, factor;

  if (!getRegisterAsComplex(REGISTER_X, &a, &b))
    return;

  // ln(10) * (a + bi) --> (a + bi)
  realMultiply(lnBase, &a, &a, &ctxtReal39);
  realMultiply(lnBase, &b, &b, &ctxtReal39);

  // exp(ln(10) * (a + bi)) --> (a + bi)
  realExp(&a, &factor, &ctxtReal39);
  realPolarToRectangular(const_1, &b, &a, &b, &ctxtReal39);
  realMultiply(&factor, &a, &a, &ctxtReal39);
  realMultiply(&factor, &b, &b, &ctxtReal39);

  convertComplexToResultRegister(&a, &b, REGISTER_X);
}

static void tenPowCplx(void) {
  intPowCplx(const_ln10);
}


/********************************************//**
 * \brief regX ==> regL and 10^regX ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fn10Pow(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexMonadicFunction(&tenPowReal, &tenPowCplx, &tenPowShoI, &tenPowLonI);
}
