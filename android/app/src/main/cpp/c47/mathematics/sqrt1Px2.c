// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file sqrt1Px2.c
 ***********************************************/

#include "c47.h"

void sqrt1Px2Complex(const cplx_t *z, cplx_t *res, realContext_t *realContext) {
  real_t magnitude;
  cplx_t w;

  if(realIsZero(&z->Imag)) {
    if(realIsInfinite(&z->Real)) {
      realSetPlusInfinity(&res->Real);
      realSetZero(&res->Imag);
      return;
    }
    realFMA(&z->Real, &z->Real, const_1, &res->Real, realContext);
    realSquareRoot(&res->Real, &res->Real, realContext);
    realSetZero(&res->Imag);
    return;
  }

  if(realIsSpecial(&z->Real) || realIsSpecial(&z->Imag)) {
    realSetNaN(&res->Real);
    realSetNaN(&res->Imag);
    return;
  }

  // sqrt(1 + a^2 - b^2 + 2 a b i)
  realFMA(&z->Imag, &z->Imag, const__1, &w.Real, realContext);
  realChangeSign(&w.Real);
  realFMA(&z->Real, &z->Real, &w.Real, &w.Real, realContext);
  realAdd(&z->Real, &z->Real, &w.Imag, realContext);
  realMultiply(&w.Imag, &z->Imag, &w.Imag, realContext);

  if(realIsZero(&w.Imag)) {
    sqrtComplex(&w, res, realContext);
  }
  else {
    complexMagnitude(&w, &magnitude, realContext);
    if(realIsNegative(&w.Real)) {
      realSubtract(&magnitude, &w.Real, &res->Imag, realContext);
      realDivideBy2(&res->Imag, realContext);
      realSquareRoot(&res->Imag, &res->Imag, realContext);
      if(realIsNegative(&w.Imag)) {
        realChangeSign(&res->Imag);
      }
      realDivide(&w.Imag, &res->Imag, &res->Real, realContext);
      realDivideBy2(&res->Real, realContext);
    }
    else {
      realAdd(&magnitude, &w.Real, &res->Real, realContext);
      realDivideBy2(&res->Real, realContext);
      realSquareRoot(&res->Real, &res->Real, realContext);
      realDivide(&w.Imag, &res->Real, &res->Imag, realContext);
      realDivideBy2(&res->Imag, realContext);
    }
  }
}




static void sqrt1Px2Real(void) {
  real_t magnitude, x;

  if(!getRegisterAsReal(REGISTER_X, &x)) {
    return;
  }

  if(realIsInfinite(&x) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function sqrt1Px2Real:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input of exp when flag SPCRES is not set", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  if(realIsInfinite(&x)) {
    realSetPlusInfinity(&x);
  }
  else if(realIsSpecial(&x)) {
    realSetNaN(&x);
  }
  else {
    realCopyAbs(&x, &magnitude);
    realFMA(&x, &x, const_1, &x, &ctxtReal39);
    realSquareRoot(&x, &x, &ctxtReal39);
    if(realCompareEqual(&x, &magnitude)) {
      realNextToward(&x, const_plusInfinity, &x, &ctxtReal39);
    }
  }
  convertRealToResultRegister(&x, REGISTER_X, amNone);
}



static void sqrt1Px2Cplx(void) {
  cplx_t z;

  if(!getRegisterAsComplex(REGISTER_X, &z)) {
    return;
  }

  sqrt1Px2Complex(&z, &z, &ctxtReal39);

  convertComplexToResultRegister(&z, REGISTER_X);
}


/********************************************//**
 * \brief regX ==> regL and sqrt1Px2(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnSqrt1Px2(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&sqrt1Px2Real, &sqrt1Px2Cplx);
}


