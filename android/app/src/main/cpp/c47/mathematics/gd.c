// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file gd.c
 ***********************************************/

#include "c47.h"

static void gdError(bool_t gd, uint8_t errorCode) {
  displayCalcErrorMessage(errorCode, ERR_REGISTER_LINE);
  #if (EXTRA_INFO_ON_CALC_ERROR == 1)
    if(gd) {
      sprintf(errorMessage, "cannot calculate gd(%s)", getRegisterDataTypeName(REGISTER_X, false, false));
      moreInfoOnError("In function fnGd:", errorMessage, NULL, NULL);
    }
    else {
      sprintf(errorMessage, "cannot calculate invGd(%s)", getRegisterDataTypeName(REGISTER_X, false, false));
      moreInfoOnError("In function fnInvGd:", errorMessage, NULL, NULL);
    }
  #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
}

static void gdReal(bool_t gd) {
  real_t x;
  uint8_t errorCode;

  if(!getRegisterAsReal(REGISTER_X, &x)) {
    return;
  }

  errorCode = gd ? GudermannianReal(&x, &x, &ctxtReal39) : InverseGudermannianReal(&x, &x, &ctxtReal39);

  if(errorCode != ERROR_NONE) {
    gdError(gd, errorCode);
  }
  else {
    convertRealToResultRegister(&x, REGISTER_X, amNone);
  }
}

static void gdCplx(bool_t gd) {
  cplx_t x;
  uint8_t errorCode;

  if(!getRegisterAsComplex(REGISTER_X, &x)) {
    return;
  }

  errorCode = gd ? GudermannianComplex(&x, &x, &ctxtReal39)
                 : InverseGudermannianComplex(&x, &x, &ctxtReal39);

  if(errorCode != ERROR_NONE) {
    gdError(gd, errorCode);
  }
  else {
    convertComplexToResultRegister(&x, REGISTER_X);
  }
}

uint8_t GudermannianReal(const real_t *x, real_t *res, realContext_t *realContext) {
  const real_t *piOn2 = realContext->digits > 39 ? const75_piOn2 : const39_piOn2;

  if(realIsZero(x)) {
    if(realIsNegative(x)) {
      realDivide(const__1, const_plusInfinity, res, realContext);
    }
    else {
      realSetZero(res);
    }
  }
  else if(realIsInfinite(x)) {
    if(realIsPositive(x)) {
      realCopy(piOn2, res);
    }
    else {
      realCopy(piOn2, res);
      realChangeSign(res);
    }
  }
  else {
    // Gd(x) = 2*atan(tanh(x/2)).
    realMultiply(x, const_1on2, res, realContext);
    WP34S_Tanh(res, res, realContext);
    C47_WP34S_Atan(res, res, realContext);
    realMultiply(res, const_2, res, realContext);
  }

  return ERROR_NONE;
}

uint8_t GudermannianComplex(const cplx_t *x, cplx_t *res, realContext_t *realContext) {
  const real_t *twoPi = realContext->digits > 39 ? const75_2pi : const39_2pi;
  real_t cosine;
  cplx_t tangent;
  bool_t nonnegativeReal = !realIsNegative(&x->Real);

  realMultiply(&x->Real, const_1on2, &tangent.Real, realContext);
  realMultiply(&x->Imag, const_1on2, &tangent.Imag, realContext);
  TanhComplex(&tangent, &tangent, realContext);
  C47_WP34S_Cvt2RadSinCosTan(&x->Imag, amRadian, NULL, &cosine, NULL, realContext);
  ArctanComplex(&tangent, res, realContext);
  realMultiply(&res->Real, const_2, &res->Real, realContext);
  realMultiply(&res->Imag, const_2, &res->Imag, realContext);

  // The defining branch differs from 2*atan(tanh(z/2)) by -2*pi when Re(z) is nonnegative and cos(Im(z)) is negative; both tests precede aliased output.
  if(nonnegativeReal && realIsNaN(&cosine)) {
    // cos(Im(z)) is NaN for an infinite or NaN Im(z), where the branch is unknown: gd(+inf + iy) is pi/2 or -3pi/2 by the sign of cos(y).
    realSetNaN(&res->Real);
    realSetNaN(&res->Imag);
  }
  else if(nonnegativeReal && realIsNegative(&cosine)) {
    realSubtract(&res->Real, twoPi, &res->Real, realContext);
  }

  return ERROR_NONE;
}

uint8_t InverseGudermannianReal(const real_t *x, real_t *res, realContext_t *realContext) {
  const real_t *piOn2 = realContext->digits > 39 ? const75_piOn2 : const39_piOn2;
  uint8_t result = ERROR_NONE;

  if(realIsNaN(x)) {
    realSetNaN(res);
    return result;
  }

  /*
   * InvGd(x) = Ln(Tan(x/2 + PI/4))
   */
  if(realCompareAbsLessThan(x, piOn2)) {
    if(realIsZero(x)) {
      if(realIsNegative(x)) {
        realDivide(const__1, const_plusInfinity, res, realContext);
      }
      else {
        realSetZero(res);
      }
    }
    else {
      real_t sin, cos;

      // InvGd(x) = asinh(tan(x)), -pi/2 < x < pi/2.
      C47_WP34S_Cvt2RadSinCosTan(x, amRadian, &sin, &cos, res, realContext);
      ArcsinhReal(res, res, realContext);
    }
  }
  else {
    result = ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN;
  }

  return result;
}

uint8_t InverseGudermannianComplex(const cplx_t *x, cplx_t *res, realContext_t *realContext) {
  real_t sine, cosine, sinhImag, modulus;

  // With z = a + ib and c = cos a + i sinh b, InvGd(z) = asinh(sin a / |c|) + i arg(c): asinh(tan z) where cos a > 0, and i pi - asinh(tan z), or -i pi - asinh(tan z)
  // for b < 0, where cos a < 0. For a large |b|, tan z comes within e^(-2|b|) of +-i, the branch points of asinh, and asinh(tan z) loses those digits.
  C47_WP34S_Cvt2RadSinCosTan(&x->Real, amRadian, &sine, &cosine, NULL, realContext);
  WP34S_SinhCosh(&x->Imag, &sinhImag, NULL, realContext);
  realMultiply(&cosine, &cosine, &modulus, realContext);
  realFMA(&sinhImag, &sinhImag, &modulus, &modulus, realContext);
  realSquareRoot(&modulus, &modulus, realContext);
  realDivide(&sine, &modulus, &res->Real, realContext);
  ArcsinhReal(&res->Real, &res->Real, realContext);
  C47_WP34S_Atan2(&sinhImag, &cosine, &res->Imag, realContext);

  return ERROR_NONE;
}

static void doGdReal(void) {
  gdReal(true);
}

static void doInvGdReal(void) {
  gdReal(false);
}

static void doGdCplx(void) {
  gdCplx(true);
}

static void doInvGdCplx(void) {
  gdCplx(false);
}

/********************************************//**
 * \brief regX ==> regL and gd(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnGd(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&doGdReal, &doGdCplx);
}

/********************************************//**
 * \brief regX ==> regL and invGd(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnInvGd(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&doInvGdReal, &doInvGdCplx);
}
