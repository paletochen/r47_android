// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file arcsin.c
 ***********************************************/

#include "c47.h"

static void arcsinCplx(void) {
  real_t xReal, xImag, rReal, rImag;

  if(!getRegisterAsComplex(REGISTER_X, &xReal, &xImag)) {
    return;
  }

  ArcsinComplex(&xReal, &xImag, &rReal, &rImag, &ctxtReal39);

  convertComplexToResultRegister(&rReal, &rImag, REGISTER_X);
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
      arcsinCplx();
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
  else if(!exactArcSinCosAngle(&x, false, &x)) {
    C47_WP34S_Asin(&x, &x, &ctxtReal39);
    convertAngleFromTo(&x, amRadian, currentAngularMode, &ctxtReal39);
  }
  reallocateRegister(REGISTER_X, dtReal34, 0, currentAngularMode);
  convertRealToResultRegister(r, REGISTER_X, currentAngularMode);
}


uint8_t ArcsinComplex(const real_t *xReal, const real_t *xImag, real_t *rReal, real_t *rImag, realContext_t *realContext) {
  real_t a, b;

  realCopy(xReal, &a);
  realCopy(xImag, &b);

  // arcsin(z) = -i.ln(iz + sqrt(1 - z²))
  // calculate sqrt(1 - z²)
  realChangeSign(&b);
  sqrt1Px2Complex(&b, &a, rReal, rImag, realContext);

  // calculate iz + sqrt(1 - z²)
  realAdd(rReal, &b, rReal, realContext);
  realAdd(rImag, &a, rImag, realContext);

  // calculate ln(iz + sqrt(1 - z²))
  lnComplex(rReal, rImag, &a, &b, realContext);

  // calculate = -i.ln(iz + sqrt(1 - z²))
  realChangeSign(&a);

  realCopy(&b, rReal);
  realCopy(&a, rImag);

  return ERROR_NONE;
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
