// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file arccosh.c
 ***********************************************/

#include "c47.h"

static void arccoshCplx(void) {
  cplx_t x, shifted, r;
  REAL_T_PTR(delta, 39);
  REAL_T_PTR(magnitude1, 39);
  REAL_T_PTR(magnitude2, 39);
  REAL_T_PTR(temp1, 39);
  REAL_T_PTR(temp2, 39);

  if(!getRegisterAsComplex(REGISTER_X, &x)) {
    return;
  }

  if(realIsNaN(&x.Real) || realIsNaN(&x.Imag)) {
    realSetNaN(&r.Real);
    realSetNaN(&r.Imag);
  }
  else if(realIsInfinite(&x.Real) || realIsInfinite(&x.Imag)) {
    const bool_t negativeBranch = (!realIsZero(&x.Real) && realIsNegative(&x.Real)) || (realIsZero(&x.Real) && realIsNegative(&x.Imag));

    realSetPlusInfinity(&r.Real);
    if(negativeBranch) {
      realSetNegativeSign(&r.Real);
    }

    realCopy(realIsInfinite(&x.Real) ? const_1 : const_0, temp1);
    if(realIsNegative(&x.Real)) {
      realSetNegativeSign(temp1);
    }
    realCopy(realIsInfinite(&x.Imag) ? const_1 : const_0, temp2);
    if(realIsNegative(&x.Imag) != negativeBranch) {
      realSetNegativeSign(temp2);
    }
    if(realIsZero(&x.Imag)) {
      realSetPositiveSign(temp2);
    }
    C47_WP34S_Atan2(temp2, temp1, &r.Imag, &ctxtReal39);
  }
  else if(realIsZero(&x.Imag)) {
    realCopyAbs(&x.Real, temp1);
    if(realCompareGreaterThan(temp1, const_1)) {
      realArcosh(temp1, &r.Real, &ctxtReal39);
      if(realIsNegative(&x.Real)) {
        realSetNegativeSign(&r.Real);
        realCopy(const39_pi, &r.Imag);
      }
      else {
        realSetZero(&r.Imag);
      }
    }
    else {
      realSetZero(&r.Real);
      realSubtract(const_1, &x.Real, temp1, &ctxtReal39);
      realAdd(const_1, &x.Real, temp2, &ctxtReal39);
      realMultiply(temp1, temp2, temp1, &ctxtReal39);
      realSquareRoot(temp1, temp1, &ctxtReal39);
      C47_WP34S_Atan2(temp1, &x.Real, &r.Imag, &ctxtReal39);
    }
  }
  else {
    const bool_t negativeBranch = (!realIsZero(&x.Real) && realIsNegative(&x.Real)) || (realIsZero(&x.Real) && realIsNegative(&x.Imag));

    // delta = (|z + 1| + |z - 1|) / 2 - 1. Rationalised distance increments preserve delta when its addition to one would round away.
    realCopy(&x.Imag, &shifted.Imag);
    realAdd(&x.Real, const_1, &shifted.Real, &ctxtReal39);
    complexMagnitude(&shifted, magnitude1, &ctxtReal39);
    realCopyAbs(&shifted.Real, temp1);
    realAdd(magnitude1, temp1, magnitude1, &ctxtReal39);

    realSubtract(&x.Real, const_1, &shifted.Real, &ctxtReal39);
    complexMagnitude(&shifted, magnitude2, &ctxtReal39);
    realCopyAbs(&shifted.Real, temp1);
    realAdd(magnitude2, temp1, magnitude2, &ctxtReal39);

    realMultiply(&x.Imag, &x.Imag, delta, &ctxtReal39);
    realDivide(delta, magnitude1, temp1, &ctxtReal39);
    realDivide(delta, magnitude2, temp2, &ctxtReal39);
    realAdd(temp1, temp2, delta, &ctxtReal39);
    realMultiply(delta, const_1on2, delta, &ctxtReal39);

    realCopyAbs(&x.Real, temp1);
    if(realCompareGreaterThan(temp1, const_1)) {
      realSubtract(temp1, const_1, temp1, &ctxtReal39);
      realAdd(delta, temp1, delta, &ctxtReal39);
    }

    realAdd(delta, const_2, temp1, &ctxtReal39);
    realMultiply(delta, temp1, temp1, &ctxtReal39);
    realSquareRoot(temp1, temp1, &ctxtReal39);
    realAdd(delta, temp1, &r.Real, &ctxtReal39);
    WP34S_Ln1P(&r.Real, &r.Real, &ctxtReal39);
    if(negativeBranch) {
      realSetNegativeSign(&r.Real);
    }

    // Scale cos(v) = a / (1 + delta) and sin(v) = sign(real) b / sqrt(delta(delta + 2)) by their positive common denominator for atan2.
    realAdd(delta, const_1, temp2, &ctxtReal39);
    realMultiply(&x.Imag, temp2, temp2, &ctxtReal39);
    if(negativeBranch) {
      realChangeSign(temp2);
    }
    realMultiply(&x.Real, temp1, temp1, &ctxtReal39);
    C47_WP34S_Atan2(temp2, temp1, &r.Imag, &ctxtReal39);
  }

  convertComplexToResultRegister(&r, REGISTER_X);
}


static void arccoshReal(void) {
  real_t x;
  const real_t *r = &x;

  if(!getRegisterAsReal(REGISTER_X, &x)) {
    return;
  }

  if(realCompareLessThan(&x, const_1)) {
    if(getFlag(FLAG_CPXRES)) {
      arccoshCplx();
      return;
    }
    if(getSystemFlag(FLAG_SPCRES)) {
      r = const_NaN;
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function arccoshReal:", "X < 1", "and CPXRES is not set!", NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  else {
    realArcosh(&x, &x, &ctxtReal39);
  }

  convertRealToResultRegister(r, REGISTER_X, amNone);
}



void realArcosh(const real_t *x, real_t *res, realContext_t *realContext) {
  real_t delta, root;

  // arccosh(x) = ln1p((x - 1) + sqrt((x - 1)(x + 1)))
  realSubtract(x, const_1, &delta, realContext);
  realAdd(x, const_1, &root, realContext);
  realMultiply(&delta, &root, &root, realContext);
  realSquareRoot(&root, &root, realContext);
  realAdd(&delta, &root, res, realContext);
  WP34S_Ln1P(res, res, realContext);
}



/********************************************//**
 * \brief regX ==> regL and arccosh(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnArccosh(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&arccoshReal, &arccoshCplx);
}
