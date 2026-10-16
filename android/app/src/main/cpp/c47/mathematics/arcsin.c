// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file arcsin.c
 ***********************************************/

#include "c47.h"

static void arcsinCplx(void) {
  cplx_t x, r;

  if(!getRegisterAsComplex(REGISTER_X, &x)) {
    return;
  }

  ArcsinComplex(&x, &r, &ctxtReal39);

  convertComplexToResultRegister(&r, REGISTER_X);
}

/********************************************//**
 * \brief The exact angle of arcsin or arccos of ±1/2 and ±1, and of arccos 0, in the current angular mode
 *
 * The angles are 30°, 60°, 90°, 120° and 180° and their negatives. An angle is returned only where the current angular mode writes it exactly: every one
 * in degrees, the multiples of 9° in grads, the multiples of 90° in multiples of π, none in radians. arcsin 0 is left to the caller, so a zero keeps its sign.
 *
 * \param[in]  x      const real_t* the argument, |x| <= 1
 * \param[in]  cosine bool_t        true for arccos, false for arcsin
 * \param[out] res    real_t*       the angle, when the result is true
 * \return bool_t true when res is the exact angle
 ***********************************************/
bool_t exactArcSinCosAngle(const real_t *x, bool_t cosine, real_t *res) {
  int32_t degrees;
  real_t a;

  realCopyAbs(x, &a);
  if(realIsZero(&a) && cosine) {
    degrees = 0;
  }
  else if(realCompareEqual(&a, const_1on2)) {
    degrees = 30;
  }
  else if(realCompareEqual(&a, const_1)) {
    degrees = 90;
  }
  else {
    return false;
  }
  if(realIsNegative(x)) {
    degrees = -degrees;                                          // arcsin is odd
  }
  if(cosine) {
    degrees = 90 - degrees;                                      // arccos x = 90° - arcsin x
  }

  switch(currentAngularMode) {
    case amDegree:
    case amDMS: {
      int32ToReal(degrees, res);
      return true;
    }
    case amGrad: {
      if(degrees % 9 != 0) {
        return false;
      }
      int32ToReal(degrees / 9 * 10, res);
      return true;
    }
    case amMultPi: {
      if(degrees % 90 != 0) {
        return false;
      }
      int32ToReal(degrees / 90 * 5, res);                        // 90° is 0.5 π
      realMultiply(res, const_1on10, res, &ctxtReal39);
      return true;
    }
    default: {
      if(degrees != 0) {
        return false;
      }
      realSetZero(res);
      return true;
    }
  }
}



static void arcsinReal(void) {
  real_t x;
  const real_t *r = &x;

  if(!getRegisterAsReal(REGISTER_X, &x)) {
    return;
  }

  if(realCompareAbsGreaterThan(&x, const_1)) {
    if(getFlag(FLAG_CPXRES)) {
      cplx_t z;

      realCopy(&x, &z.Real);
      realSetZero(&z.Imag);
      if(!realIsNegative(&x)) {
        realSetNegativeSign(&z.Imag);
      }
      ArcsinComplex(&z, &z, &ctxtReal39);
      convertComplexToResultRegister(&z, REGISTER_X);
      return;
    }
    else if(getSystemFlag(FLAG_SPCRES)) {
      r = const_NaN;
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function arcsinReal:", "|X| > 1", "and CPXRES is not set!", NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  else if(currentAngularMode == amRadian && !realIsZero(&x) && realGetExponent(&x) < -18) {   // arcsin x = x + x³/6 to 72 digits, above |x| by less than half a unit at 34 digits
    WP34S_TinyOddSeries(&x, const_6, true, &x, &ctxtReal75);
  }
  else if(!exactArcSinCosAngle(&x, false, &x)) {
    C47_WP34S_Asin(&x, &x, &ctxtReal39);
    convertAngleFromTo(&x, amRadian, currentAngularMode, &ctxtReal39);
  }
  reallocateRegister(REGISTER_X, dtReal34, 0, currentAngularMode);
  convertRealToResultRegister(r, REGISTER_X, currentAngularMode);
}


/********************************************//**
 * \brief calculate principal arcsin or arccos components
 *
 * \param[in] z argument
 * \param[out] r result
 * \param[in] calculateAcos select arccos instead of arcsin
 * \param[in] realContext working context
 * \return error code
 ***********************************************/
uint8_t ArcsinAcosComplex(const cplx_t *z, cplx_t *r, bool_t calculateAcos, realContext_t *realContext) {
  real_t x, xAbs, y, yAbs, ySquared, q, d, t;
  bool_t negativeImaginaryInput;
  const real_t *pi = realContext->digits > 39 ? const75_pi : const39_pi;
  const real_t *piOn2 = realContext->digits > 39 ? const75_piOn2 : const39_piOn2;
  const real_t *piOn4 = realContext->digits > 39 ? const75_piOn4 : const39_piOn4;
  const real_t *pi3On4 = realContext->digits > 39 ? const75_3piOn4 : const39_3piOn4;

  realCopy(&z->Real, &x);
  realCopy(&z->Imag, &y);

  if(realIsNaN(&x) || realIsNaN(&y)) {
    realSetNaN(&r->Real);
    realSetNaN(&r->Imag);
    return ERROR_NONE;
  }

  negativeImaginaryInput = realIsNegative(&y);
  // On either real-axis cut, an exact zero retains the established continuation selected by the real component rather than the imaginary zero sign.
  if(realIsZero(&y) && realCompareAbsGreaterThan(&x, const_1)) {
    negativeImaginaryInput = !realIsNegative(&x);
  }

  if(realIsInfinite(&x) || realIsInfinite(&y)) {
    if(realIsInfinite(&x) && realIsInfinite(&y)) {
      realCopy(realIsNegative(&x) && calculateAcos ? pi3On4 : piOn4, &r->Real);
      if(realIsNegative(&x) && !calculateAcos) {
        realSetNegativeSign(&r->Real);
      }
    } else if(realIsInfinite(&x)) {
      if(calculateAcos) {
        if(realIsNegative(&x)) {
          realCopy(pi, &r->Real);
        } else {
          realSetZero(&r->Real);
        }
      } else {
        realCopy(piOn2, &r->Real);
        if(realIsNegative(&x)) {
          realSetNegativeSign(&r->Real);
        }
      }
    } else if(calculateAcos) {
      realCopy(piOn2, &r->Real);
    } else {
      realSetZero(&r->Real);
      if(realIsNegative(&x)) {
        realSetNegativeSign(&r->Real);
      }
    }

    realCopy(const_plusInfinity, &r->Imag);
    if((calculateAcos && !negativeImaginaryInput) || (!calculateAcos && negativeImaginaryInput)) {
      realSetNegativeSign(&r->Imag);
    }
    return ERROR_NONE;
  }

  // For asin(x+iy)=a+ib, t=sinh(b)² is the nonnegative root of t²-(x²+y²-1)t-y²=0; acos uses pi/2-a and -b from the same magnitudes.
  realCopyAbs(&x, &xAbs);
  realCopyAbs(&y, &yAbs);
  realMultiply(&yAbs, &yAbs, &ySquared, realContext);
  realSubtract(&xAbs, const_1, &q, realContext);
  realAdd(&xAbs, const_1, &d, realContext);
  realMultiply(&q, &d, &q, realContext);
  realFMA(&yAbs, &yAbs, &q, &q, realContext);

  // d=sqrt(q²+4y²); the q<0 expression uses d-q instead of q+d, avoiding subtraction of nearly equal operands near the finite real-axis segment.
  realMultiply(&ySquared, const_4, &d, realContext);
  realFMA(&q, &q, &d, &d, realContext);
  realSquareRoot(&d, &d, realContext);
  if(realIsNegative(&q)) {
    realSubtract(&d, &q, &r->Imag, realContext);
    realMultiply(&ySquared, const_2, &t, realContext);
    realDivide(&t, &r->Imag, &t, realContext);
  } else {
    realAdd(&q, &d, &t, realContext);
    realMultiply(&t, const_1on2, &t, realContext);
  }

  realSquareRoot(&t, &ySquared, realContext);
  WP34S_ArcSinh(&ySquared, &r->Imag, realContext);
  if((calculateAcos && !negativeImaginaryInput) || (!calculateAcos && negativeImaginaryInput)) {
    realSetNegativeSign(&r->Imag);
  }

  realAdd(&t, const_1, &r->Real, realContext);
  realSquareRoot(&r->Real, &r->Real, realContext);
  if(realIsZero(&t)) {
    realSubtract(const_1, &xAbs, &q, realContext);
    realAdd(const_1, &xAbs, &d, realContext);
    realMultiply(&q, &d, &q, realContext);
    realSquareRoot(&q, &q, realContext);
    C47_WP34S_Atan2(calculateAcos ? &q : &x, calculateAcos ? &x : &q, &r->Real, realContext);
  } else {
    realMultiply(&x, &ySquared, &q, realContext);
    realMultiply(&yAbs, &r->Real, &d, realContext);
    C47_WP34S_Atan2(calculateAcos ? &d : &q, calculateAcos ? &q : &d, &r->Real, realContext);
  }

  return ERROR_NONE;
}


uint8_t ArcsinComplex(const cplx_t *x, cplx_t *r, realContext_t *realContext) {
  return ArcsinAcosComplex(x, r, false, realContext);
}


/********************************************//**
 * \brief regX ==> regL and arcsin(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnArcsin(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&arcsinReal, &arcsinCplx);

}
