// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file 10pow.c
 ***********************************************/

#include "c47.h"

static void tenPowReal(void);


void realPower10(const real_t *x, real_t *res, realContext_t *realContext) {
  bool_t error;
  int32_t exponent;

  if(!realIsSpecial(x)) {
    exponent = realToInt32C47(x, &error);
    if(!error && exponent <= DECQUAD_Emax && exponent >= DECQUAD_Emin - DECQUAD_Pmax + 1 && exponent < realContext->emax && exponent > realContext->emin) {
      REAL_T_PTR(integerPart, 39);

      int32ToReal(exponent, integerPart);
      realSubtract(x, integerPart, res, realContext);
      if(realIsZero(res)) {
        realSetOne(res);
      }
      else {
        realMultiply(res, const39_ln10, res, realContext);
        realExp(res, res, realContext);
      }
      res->exponent += exponent;
      return;
    }
  }

  realMultiply(x, const39_ln10, res, realContext);
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

  if(!getRegisterAsReal(REGISTER_X, &x)) {
    return;
  }

  if(realIsSpecial(&x) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function intPowReal:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input of 10" STD_SUP_BOLD_x " when flag SPCRES is not set", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  (*powf)(&x, &x, &ctxtReal39);
  convertRealToResultRegister(&x, REGISTER_X, amNone);
}

static void tenPowReal(void) {
  intPowReal(&realPower10);
}


void intPowCplx(void (*powf)(const real_t *x, real_t *res, realContext_t *realContext), const real_t *lnBase) {
  cplx_t x;
  REAL_T_PTR(factor, 39);

  if(!getRegisterAsComplex(REGISTER_X, &x)) {
    return;
  }

  if(realIsZero(&x.Imag)) {
    (*powf)(&x.Real, &x.Real, &ctxtReal39);
    realSetZero(&x.Imag);
    convertComplexToResultRegister(&x, REGISTER_X);
    return;
  }

  (*powf)(&x.Real, factor, &ctxtReal39);
  realMultiply(lnBase, &x.Imag, &x.Imag, &ctxtReal75);
  realPolarToRectangular(const_1, &x.Imag, &x, &ctxtReal75);
  realMultiply(factor, &x.Real, &x.Real, &ctxtReal39);
  realMultiply(factor, &x.Imag, &x.Imag, &ctxtReal39);

  convertComplexToResultRegister(&x, REGISTER_X);
}

static void tenPowCplx(void) {
  cplx_t x;
  REAL_T_PTR(factor, 39);

  if(!getRegisterAsComplex(REGISTER_X, &x)) {
    return;
  }

  if(realIsZero(&x.Imag)) {
    realPower10(&x.Real, &x.Real, &ctxtReal39);
    realSetZero(&x.Imag);
    convertComplexToResultRegister(&x, REGISTER_X);
    return;
  }

  realPower10(&x.Real, factor, &ctxtReal39);
  realMultiply(const75_ln10, &x.Imag, &x.Imag, &ctxtReal75);
  realPolarToRectangular(const_1, &x.Imag, &x, &ctxtReal75);
  realMultiply(factor, &x.Real, &x.Real, &ctxtReal39);
  realMultiply(factor, &x.Imag, &x.Imag, &ctxtReal39);

  convertComplexToResultRegister(&x, REGISTER_X);
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
