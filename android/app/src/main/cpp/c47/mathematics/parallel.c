// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file parallel.c
 ***********************************************/

#include "c47.h"


/******************************************************************************************************************************************************************************************/
/* real34 || ...                                                                                                                                                                          */
/******************************************************************************************************************************************************************************************/

static void parallelResultContext(realContext_t *realContext) {
  *realContext = ctxtReal34;
  if(significantDigits > 0 && significantDigits < 34) {
    realContext->digits = significantDigits;
  }
}



static bool_t parallelExactRegister(calcRegister_t regist, real_t *result) {
  int32_t exponent;
  longInteger_t coefficient;
  realContext_t realContext;

  if(!convertRegisterToScaledLongInteger(regist, coefficient, &exponent)) {
    return false;
  }
  parallelResultContext(&realContext);
  convertScaledLongIntegerToReal(coefficient, exponent, result, &realContext);
  longIntegerFree(coefficient);
  return true;
}



static bool_t parallelExactRegisters(real_t *result) {
  int32_t xExponent, yExponent, sumExponent;
  longInteger_t x, y, product, sum;
  realContext_t realContext;

  if(!convertRegisterToScaledLongInteger(REGISTER_X, x, &xExponent)) {
    return false;
  }
  if(!convertRegisterToScaledLongInteger(REGISTER_Y, y, &yExponent)) {
    longIntegerFree(x);
    return false;
  }
  if(longIntegerIsZero(x) || longIntegerIsZero(y)) {
    longIntegerFree(y);
    longIntegerFree(x);
    return false;
  }

  longIntegerInit(product);
  longIntegerMultiplyUnbounded(x, y, product);
  addSubtractScaledLongIntegers(x, xExponent, y, yExponent, false, sum, &sumExponent);
  parallelResultContext(&realContext);
  const bool_t ok = divideScaledLongIntegersToReal(product, xExponent + yExponent, false, sum, sumExponent, result, &realContext);
  longIntegerFree(sum);
  longIntegerFree(product);
  longIntegerFree(y);
  longIntegerFree(x);
  return ok;
}

/********************************************//**
 * \brief Y(real34) || X(real34) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
static void doParallelReal(void) {
  real_t y, x, product;
  real_t exactResult;
  const uint32_t typeX = getRegisterDataType(REGISTER_X);
  const uint32_t typeY = getRegisterDataType(REGISTER_Y);

  if(typeX == dtLongInteger && typeY == dtReal34 && real34IsInfinite(REGISTER_REAL34_DATA(REGISTER_Y))) {
    parallelExactRegister(REGISTER_X, &exactResult);
    convertRealToResultRegister(&exactResult, REGISTER_X, amNone);
    return;
  }
  if(typeY == dtLongInteger && typeX == dtReal34 && real34IsInfinite(REGISTER_REAL34_DATA(REGISTER_X))) {
    parallelExactRegister(REGISTER_Y, &exactResult);
    convertRealToResultRegister(&exactResult, REGISTER_X, amNone);
    return;
  }
  if((typeX == dtLongInteger || typeY == dtLongInteger) && parallelExactRegisters(&exactResult)) {
    convertRealToResultRegister(&exactResult, REGISTER_X, amNone);
    return;
  }

  if(!getRegisterAsReal(REGISTER_X, &x) || !getRegisterAsReal(REGISTER_Y, &y)) {
    return;
  }

  if(realIsNaN(&x) || realIsNaN(&y)) {
    realSetNaN(&x);
  } else if(!realIsZero(&x)) {
    if(realIsZero(&y)) {
      realCopy(&y, &x);
    } else if(realIsInfinite(&x)) {
      if(realIsInfinite(&y)) {
        realSetNaN(&x);
      } else {
        realCopy(&y, &x);
      }
    } else if(!realIsInfinite(&y)) {
      // y || x = xy / (x+y)
      realMultiply(&y, &x, &product, &ctxtReal39);
      realAdd(&y, &x, &y, &ctxtReal39);
      realDivide(&product, &y, &x, &ctxtReal39);
    }
  }
  convertRealToResultRegister(&x, REGISTER_X, amNone);
}



/******************************************************************************************************************************************************************************************/
/* complex34 || ...                                                                                                                                                                       */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(complex34) || X(complex34) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
static void doParallelComplex(void) {
  real_t denominator;
  cplx_t y, x, product, sum;
  bool_t xIsZero, yIsZero, xIsInfinite, yIsInfinite;

  if(!getRegisterAsComplex(REGISTER_X, &x) || !getRegisterAsComplex(REGISTER_Y, &y)) {
    return;
  }

  xIsZero = realIsZero(&x.Real) && realIsZero(&x.Imag);
  yIsZero = realIsZero(&y.Real) && realIsZero(&y.Imag);
  xIsInfinite = realIsInfinite(&x.Real) || realIsInfinite(&x.Imag);
  yIsInfinite = realIsInfinite(&y.Real) || realIsInfinite(&y.Imag);

  if(realIsNaN(&x.Real) || realIsNaN(&x.Imag) || realIsNaN(&y.Real) || realIsNaN(&y.Imag)) {
    realSetNaN(&x.Real);
    realSetNaN(&x.Imag);
  } else if(!xIsZero) {
    if(yIsZero) {
      realCopy(&y.Real, &x.Real);
      realCopy(&y.Imag, &x.Imag);
    } else if(xIsInfinite) {
      if(yIsInfinite) {
        realSetNaN(&x.Real);
        realSetNaN(&x.Imag);
      } else {
        realCopy(&y.Real, &x.Real);
        realCopy(&y.Imag, &x.Imag);
      }
    } else if(!yIsInfinite) {
      // y || x = xy / (x + y)
      mulComplexComplex(&y, &x, &product, &ctxtReal75);
      realAdd(&y.Real, &x.Real, &sum.Real, &ctxtReal75);
      realAdd(&y.Imag, &x.Imag, &sum.Imag, &ctxtReal75);

      realChangeSign(&sum.Imag);
      mulComplexComplex(&product, &sum, &x, &ctxtReal75);
      realMultiply(&sum.Real, &sum.Real, &denominator, &ctxtReal39);
      realFMA(&sum.Imag, &sum.Imag, &denominator, &denominator, &ctxtReal39);
      realDivide(&x.Real, &denominator, &x.Real, &ctxtReal39);
      realDivide(&x.Imag, &denominator, &x.Imag, &ctxtReal39);
    }
  }
  convertComplexToResultRegister(&x, REGISTER_X);
}

/********************************************//**
 * \brief regX ==> regL and regY || regX ==> regX
 * Drops Y, enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter
 * \return void
 ***********************************************/
void fnParallel(uint16_t unusedButMandatoryParameter) {
  processRealComplexDyadicFunction(doParallelReal, doParallelComplex);
}
