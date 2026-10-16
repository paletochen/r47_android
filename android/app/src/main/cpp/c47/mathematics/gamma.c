// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file gamma.c
 ***********************************************/

#include "c47.h"

static int checkGammaDomain(const real_t *xReal, real_t *out) {
  if(realIsSpecial(xReal)) {
    if(realIsNaN(xReal)) {
      realSetNaN(out);
    }
    else if(!getSystemFlag(FLAG_SPCRES)) {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function checkGammaDomain:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input of gamma when flag SPCRES is not set", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else {
      realCopy(realIsPositive(xReal) ? const_plusInfinity : const_NaN, out);
    }
    return 0;
  }
  if(realCompareLessEqual(xReal, const_0) && realIsAnInteger(xReal)) {
    if(!getSystemFlag(FLAG_SPCRES)) {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function checkGammaDomain:", "cannot use a negative integer as X input of gamma when flag SPCRES is not set", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else {
      realSetNaN(out);
    }
    return 0;
  }
  return 1;
}

static void gammaReal(void) {
  real_t x;

  if(!getRegisterAsReal(REGISTER_X, &x)) {
    return;
  }

  if(checkGammaDomain(&x, &x)) {
    WP34S_Gamma(&x, &x, &ctxtReal39);
  }
  convertRealToResultRegister(&x, REGISTER_X, amNone);
}



/**
 * ln|Gamma(x)| at 75 digits for a negative x that is not an integer; returns whether Gamma(x) is negative. Where it is, ln|Gamma(x)| is lnGamma(x + 1) - ln(-x),
 * since Gamma(x + 1) = x Gamma(x) is then positive. Next to the zeros of ln|Gamma| on the negative axis the terms of the reflection cancel, so all of it is at 75 digits.
 */
static bool_t lnAbsGammaNegative(const real_t *x, real_t *res) {
  real_t xin, t;

  realCopy(x, &xin);
  realMinus(&xin, &t, &ctxtReal75);
  WP34S_Mod(&t, const_2, &t, &ctxtReal75);
  if(realCompareGreaterThan(&t, const_1)) {
    WP34S_LnGamma(&xin, res, &ctxtReal75);
    return false;
  }
  realAdd(&xin, const_1, &t, &ctxtReal75);
  WP34S_LnGamma(&t, res, &ctxtReal75);
  realMinus(&xin, &t, &ctxtReal75);
  WP34S_Ln(&t, &t, &ctxtReal75);
  realSubtract(res, &t, res, &ctxtReal75);
  return true;
}



static void lnGammaReal(void) {
  real_t xReal, xImag;

  if(!getRegisterAsReal(REGISTER_X, &xReal)) {
    return;
  }
  setRegisterAngularMode(REGISTER_X, amNone);

  if(!checkGammaDomain(&xReal, &xReal)) {
    goto end;
  }

  if(realCompareLessEqual(&xReal, const_0)) { // x <= 0
    // x is negative and not an integer
    realCopy(&xReal, &xImag); // x.imag is used as a temp variable here
    if(!lnAbsGammaNegative(&xImag, &xReal)) { // the result is a real
      convertRealToReal34ResultRegister(&xReal, REGISTER_X);
    }
    else { // the result is a complex
      if(getFlag(FLAG_CPXRES)) { // We can calculate a complex
        reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
        realToIntegralValue(&xImag, &xImag, DEC_ROUND_FLOOR, &ctxtReal39);
        realMultiply(&xImag, const39_pi, &xImag, &ctxtReal39);
        convertComplexPartsToResultRegister(&xReal, &xImag, REGISTER_X);
      }
      else { // Domain error
        displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
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
  cplx_t z;

  if(!getRegisterAsComplex(REGISTER_X, &z)) {
    return;
  }
  WP34S_ComplexGamma(&z, &z, &ctxtReal39);
  convertComplexToResultRegister(&z, REGISTER_X);
}



// Abramowitz & Stegun §6.1.41, leading terms only: (z-1/2)ln(z) - z + (1/2)ln(2pi)
// Not a general-purpose lnGamma: complexLnGamma() uses this solely as a reference to
// pick the 2*pi winding integer for the imaginary part of the Lanczos result, so the
// imaginary part only needs to be within pi of the true lnGamma. The leading terms
// achieve that for every z off the real axis: the imaginary-part error stays below
// ~pi/2 (approached near the negative real axis and, as |z| -> 0, tending to
// arg(z)/2), a full factor of 2 inside the margin. The asymptotic corrections
// 1/(12z) - 1/(360z^3) + 1/(1260z^5) - ... are intentionally omitted: where |z| is
// large enough for the truncated series to be accurate their total stays orders of
// magnitude below the ~pi decision margin, and for small |z| the divergent tail
// grows past pi (~6000 at z = 0.1i) and corrupts the winding choice.
static void complexLnGamma_Stirling(const cplx_t *x, cplx_t *r, realContext_t *realContext) {
  cplx_t t;

  lnComplex(x, r, realContext);
  realSubtract(&x->Real, const_1on2, &t.Real, realContext);
  realCopy(&x->Imag, &t.Imag);
  mulComplexComplex(&t, r, r, realContext);

  realSubtract(&r->Real, &x->Real, &r->Real, realContext);
  realSubtract(&r->Imag, &x->Imag, &r->Imag, realContext);

  realAdd(&r->Real, const39_ln2piOn2, &r->Real, realContext);
}



void complexLnGamma(const cplx_t *x, cplx_t *r, realContext_t *realContext) {
  if(realIsZero(&x->Imag) && realIsNegative(&x->Real) && !realIsAnInteger(&x->Real)) {
    // On the cut along the negative axis, the value from above: ln|Gamma(x)| + i pi floor(x)
    realToIntegralValue(&x->Real, &r->Imag, DEC_ROUND_FLOOR, realContext);
    realMultiply(&r->Imag, const75_pi, &r->Imag, realContext);
    lnAbsGammaNegative(&x->Real, &r->Real);
  }
  else if(realIsZero(&x->Imag)) {
    WP34S_LnGamma(&x->Real, &r->Real, realContext);
    realCopy(realIsNaN(&r->Real) ? const_NaN : const_0, &r->Imag);
  }
  else {
    cplx_t z, t;
    realCopy(&x->Real, &z.Real);
    realCopy(&x->Imag, &z.Imag);
    complexLnGamma_Stirling(&z, &t, &ctxtReal39);
    WP34S_ComplexLnGamma(&z, r, &ctxtReal39);
    realSubtract(&t.Imag, &r->Imag, &t.Imag, &ctxtReal39);
    realDivide(&t.Imag, const39_2pi, &t.Imag, &ctxtReal39);
    realToIntegralValue(&t.Imag, &t.Imag, DEC_ROUND_HALF_EVEN, &ctxtReal39);
    realFMA(const39_2pi, &t.Imag, &r->Imag, &r->Imag, &ctxtReal39);
  }
}



static void lnGammaCplx(void) {
  cplx_t z, r;

  if(!getRegisterAsComplex(REGISTER_X, &z)) {
    return;
  }
  complexLnGamma(&z, &r, &ctxtReal39);
  convertComplexToResultRegister(&r, REGISTER_X);
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
