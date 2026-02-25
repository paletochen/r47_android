// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file gamma.c
 ***********************************************/

#include "c47.h"

static int checkGammaDomain(const real_t *xReal, real_t *out) {
  if(realIsSpecial(xReal)) {
    if (realIsNaN(xReal))
      realCopy(const_NaN, out);
    else if(!getSystemFlag(FLAG_SPCRES)) {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function checkGammaDomain:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input of gamma when flag D is not set", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else
      realCopy(realIsPositive(xReal) ? const_plusInfinity : const_NaN, out);
    return 0;
  }
  if(realCompareLessEqual(xReal, const_0) && realIsAnInteger(xReal)) {
    if(!getSystemFlag(FLAG_SPCRES)) {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function checkGammaDomain:", "cannot use a negative integer as X input of gamma when flag D is not set", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else {
      realCopy(const_NaN, out);
    }
    return 0;
  }
  return 1;
}

static void gammaReal(void) {
  real_t x;

  if(!getRegisterAsReal(REGISTER_X, &x))
    return;

  if (checkGammaDomain(&x, &x))
    WP34S_Gamma(&x, &x, &ctxtReal39);
  convertRealToResultRegister(&x, REGISTER_X, amNone);
}



static void lnGammaReal(void) {
  real_t xReal, xImag;

  if(!getRegisterAsReal(REGISTER_X, &xReal))
    return;
  setRegisterAngularMode(REGISTER_X, amNone);

  if (!checkGammaDomain(&xReal, &xReal))
    goto end;

  if(realCompareLessEqual(&xReal, const_0)) { // x <= 0
    // x is negative and not an integer
    realMinus(&xReal, &xImag, &ctxtReal39); // x.imag is used as a temp variable here
    WP34S_Mod(&xImag, const_2, &xImag, &ctxtReal39);
    if(realCompareGreaterThan(&xImag, const_1)) { // the result is a real
      WP34S_LnGamma(&xReal, &xReal, &ctxtReal39);
      convertRealToReal34ResultRegister(&xReal, REGISTER_X);
    }
    else { // the result is a complex
      if(getFlag(FLAG_CPXRES)) { // We can calculate a complex
        real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &xImag);
        reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
        WP34S_Gamma(&xReal, &xReal, &ctxtReal39);
        realSetPositiveSign(&xReal);
        WP34S_Ln(&xReal, &xReal, &ctxtReal39);
        realToIntegralValue(&xImag, &xImag, DEC_ROUND_FLOOR, &ctxtReal39);
        realMultiply(&xImag, const_pi, &xImag, &ctxtReal39);
        convertComplexToResultRegister(&xReal, &xImag, REGISTER_X);
      }
      else { // Domain error
        displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function lnGammaReal:", "cannot use a as X input of lngamma if gamma(X)<0 when flag I is not set", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
    }
    return;
  }

  WP34S_LnGamma(&xReal, &xReal, &ctxtReal39);
end:
  convertRealToResultRegister(&xReal, REGISTER_X, amNone);
}



static void gammaCplx(void) {
  real_t zReal, zImag;

  if(!getRegisterAsComplex(REGISTER_X, &zReal, &zImag))
    return;
  WP34S_ComplexGamma(&zReal, &zImag, &zReal, &zImag, &ctxtReal39);
  convertComplexToResultRegister(&zReal, &zImag, REGISTER_X);
}



// Abramowitz & Stegun §6.1.41
// (only approximation needed here)
static void complexLnGamma_Stirling(const real_t *xReal, const real_t *xImag, real_t *rReal, real_t *rImag, realContext_t *realContext) {
  // (z-1/2)ln(z) - z + (1/2)ln(2pi) + 1/(12z) - 1/(360z^3) + 1/(1260z^5) - 1/(1680z^7) ...
  real_t zReal, zImag, z2Real, z2Imag, zxReal, zxImag, tReal, tImag;
  realCopy(xReal, &zReal); realCopy(xImag, &zImag);
  lnComplex(&zReal, &zImag, rReal, rImag, realContext);
  realSubtract(&zReal, const_1on2, &tReal, realContext);
  mulComplexComplex(&tReal, &zImag, rReal, rImag, rReal, rImag, realContext);

  realSubtract(rReal, &zReal, rReal, realContext); realSubtract(rImag, &zImag, rImag, realContext);

  realAdd(rReal, const_ln2piOn2, rReal, realContext);

  realMultiply(const_12, &zReal, &tReal, realContext); realMultiply(const_12, &zImag, &tImag, realContext);
  divRealComplex(const_1, &tReal, &tImag, &tReal, &tImag, realContext);
  realAdd(rReal, &zReal, rReal, realContext); realAdd(rImag, &zImag, rImag, realContext);

  mulComplexComplex(&zReal, &zImag, &zReal, &zImag, &z2Real, &z2Imag, realContext);
  mulComplexComplex(&zReal, &zImag, &z2Real, &z2Imag, &zxReal, &zxImag, realContext);
  realMultiply(const_360, &zxReal, &tReal, realContext); realMultiply(const_360, &zxImag, &tImag, realContext);
  divRealComplex(const_1, &tReal, &tImag, &tReal, &tImag, realContext);
  realSubtract(rReal, &zReal, rReal, realContext); realSubtract(rImag, &zImag, rImag, realContext);

  mulComplexComplex(&zxReal, &zxImag, &z2Real, &z2Imag, &zxReal, &zxImag, realContext);
  realMultiply(const_1260, &zxReal, &tReal, realContext); realMultiply(const_1260, &zxImag, &tImag, realContext);
  divRealComplex(const_1, &tReal, &tImag, &tReal, &tImag, realContext);
  realAdd(rReal, &zReal, rReal, realContext); realAdd(rImag, &zImag, rImag, realContext);

  mulComplexComplex(&zxReal, &zxImag, &z2Real, &z2Imag, &zxReal, &zxImag, realContext);
  realMultiply(const_1680, &zxReal, &tReal, realContext); realMultiply(const_1680, &zxImag, &tImag, realContext);
  divRealComplex(const_1, &tReal, &tImag, &tReal, &tImag, realContext);
  realSubtract(rReal, &zReal, rReal, realContext); realSubtract(rImag, &zImag, rImag, realContext);
}



void complexLnGamma(const real_t *xReal, const real_t *xImag, real_t *rReal, real_t *rImag, realContext_t *realContext) {
  if(realIsZero(xImag)) {
    WP34S_LnGamma(xReal, rReal, realContext);
    realCopy(realIsNaN(rReal) ? const_NaN : const_0, rImag);
  }
  else {
    real_t zReal, zImag, tImag;
    realCopy(xReal, &zReal); realCopy(xImag, &zImag);
    complexLnGamma_Stirling(&zReal, &zImag, rReal, &tImag, &ctxtReal39);
    WP34S_ComplexLnGamma(&zReal, &zImag, rReal, rImag, &ctxtReal39);
    realSubtract(&tImag, rImag, &tImag, &ctxtReal39);
    realDivide(&tImag, const_2pi, &tImag, &ctxtReal39);
    realToIntegralValue(&tImag, &tImag, DEC_ROUND_HALF_EVEN, &ctxtReal39);
    realFMA(const_2pi, &tImag, rImag, rImag, &ctxtReal39);
  }
}



static void lnGammaCplx(void) {
  real_t zReal, zImag, rReal, rImag;

  if(!getRegisterAsComplex(REGISTER_X, &zReal, &zImag))
    return;
  complexLnGamma(&zReal, &zImag, &rReal, &rImag, &ctxtReal39);
  convertComplexToResultRegister(&rReal, &rImag, REGISTER_X);
}


/********************************************//**
 * \brief regX ==> regL and Gamma(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnGamma(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&gammaReal, &gammaCplx);
}

/********************************************//**
 * \brief regX ==> regL and lnGamma(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnLnGamma(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&lnGammaReal, &lnGammaCplx);
}
