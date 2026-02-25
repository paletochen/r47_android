// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file lnbeta.c
 ***********************************************/

#include "c47.h"


/*
 * This function checks argument for LnGamma function.
 * Returns true if function can be calculated, false otherwise. In case of error the function sets the appropriate value
 * into REGISTER_X and display the error message.
 */
#define RESULT_TYPE_UNKNOWN 0
#define RESULT_TYPE_REAL    1
#define RESULT_TYPE_COMPLEX 2

static bool_t _checkLnGammaArgs(int8_t *resultType, real_t *xReal, realContext_t *realContext) {
  bool_t result = true;
  *resultType = RESULT_TYPE_UNKNOWN;

  if(realIsInfinite(xReal)) {
    if(!getSystemFlag(FLAG_SPCRES)) {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      EXTRA_INFO_MESSAGE("_checkLnGammaArgs", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input of lnbeta when flag D is not set");
    }
    else {
      realToReal34((real34IsPositive(xReal) ? const_plusInfinity : const_NaN), REGISTER_REAL34_DATA(REGISTER_X));
    }

    result = false;
  }
  else if(realCompareLessEqual(xReal, const_0)) { // x <= 0
    if(realIsAnInteger(xReal)) {
      if(!getSystemFlag(FLAG_SPCRES)) {
        displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
        EXTRA_INFO_MESSAGE("_checkLnGammaArgs", "cannot use a negative integer as X input of lnbeta when flag D is not set");
      }
      else {
        reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
        convertRealToReal34ResultRegister(const_NaN, REGISTER_X);
      }

      result = false;
    }
    else { // x is negative and not an integer
      real_t tmp;

      realMinus(xReal, &tmp, realContext); // tmp = -x
      WP34S_Mod(&tmp, const_2, &tmp, realContext); // tmp = ?

      if(realCompareGreaterThan(&tmp, const_1)) { // the result is a real
          *resultType = RESULT_TYPE_REAL;
      }
      else { // the result is a complex
        if(getFlag(FLAG_CPXRES)) {
          *resultType = RESULT_TYPE_COMPLEX;
        }
        else { // Domain error
          displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
          EXTRA_INFO_MESSAGE("_checkLnGammaArgs", "cannot use a as X input of lnbeta if gamma(X)<0 when flag I is not set");
          result = false;
        }
      }
    }
  }
  else {
      *resultType = RESULT_TYPE_REAL;
  }

  return result;
}



static void _lnGammaReal(real_t *xReal, real_t *rReal, realContext_t *realContext) {
  WP34S_LnGamma(xReal, rReal, realContext);
}



static void _lnGammaComplex(real_t *xReal, real_t *rReal, real_t *rImag, realContext_t *realContext) {
  realCopy(xReal, rImag);
  WP34S_Gamma(xReal, xReal, realContext);
  realSetPositiveSign(xReal);
  WP34S_Ln(xReal, xReal, realContext);
  realCopy(xReal, rReal);
  realToIntegralValue(rImag, rImag, DEC_ROUND_FLOOR, realContext);
  realMultiply(rImag, const_pi, rImag, realContext);
}



static void _lnBetaComplex(real_t *xReal, real_t *xImag, real_t *yReal, real_t *yImag, real_t *rReal, real_t *rImag, realContext_t *realContext) {
  // LnBeta(x, y) := LnGamma(x) + LnGamma(y) - LnGamma(x+y)
  real_t tReal, tImag;

  complexLnGamma(xReal, xImag, &tReal, &tImag, realContext); // t = LnGamma(x)
  complexLnGamma(yReal, yImag, rReal, rImag, realContext);   // r = LnGamma(y)

  realAdd(rReal, &tReal, rReal, realContext); // r = LnGamma(x) + LnGamma(y)
  realAdd(rImag, &tImag, rImag, realContext);

  realAdd(xReal, yReal, &tReal, realContext); // t = x + y
  realAdd(xImag, yImag, &tImag, realContext);

  complexLnGamma(&tReal, &tImag, &tReal, &tImag, realContext); // t = LnGamma(x + y);

  realSubtract(rReal, &tReal, rReal, realContext); // r = LnGamma(x) + LnGamma(y) - LnGamma(x + y);
  realSubtract(rImag, &tImag, rImag, realContext);
}



static bool_t _lnBetaReal(real_t *xReal, real_t *yReal, real_t *rReal, real_t *rImag, realContext_t *realContext) {
  int8_t xflag, yflag, sflag;

  realAdd(xReal, yReal, rReal, realContext);  // r = x+y

  if(_checkLnGammaArgs(&xflag, xReal, realContext)
      && _checkLnGammaArgs(&yflag, yReal, realContext)
      && _checkLnGammaArgs(&sflag, rReal, realContext)) {
    real_t gxReal, gxImag;  // LnGamma(x)
    real_t gyReal, gyImag;  // LnGamma(y)
    real_t gsReal, gsImag;  // LnGamma(x+y)

    if(xflag==RESULT_TYPE_REAL) {
      _lnGammaReal(xReal, &gxReal, realContext);
      realCopy(const_0, &gxImag);
    }
    else {
      _lnGammaComplex(xReal, &gxReal, &gxImag, realContext);
    }

    if(yflag==RESULT_TYPE_REAL) {
      _lnGammaReal(yReal, &gyReal, realContext);
      realCopy(const_0, &gyImag);
    }
    else {
      _lnGammaComplex(yReal, &gyReal, &gyImag, realContext);
    }

    if(sflag==RESULT_TYPE_REAL) {
      _lnGammaReal(rReal, &gsReal, realContext);
      realCopy(const_0, &gsImag);
    }
    else {
      _lnGammaComplex(rReal, &gsReal, &gsImag, realContext);
    }

    realAdd(&gxReal, &gyReal, rReal, realContext); // r = LnGamma(x) + LnGamma(y)
    realAdd(&gxImag, &gyImag, rImag, realContext);

    realSubtract(rReal, &gsReal, rReal, realContext); // r = LnGamma(x) + LnGamma(y) - LnGamma(x+y)
    realSubtract(rImag, &gsImag, rImag, realContext);

    return true;
  }

  return false;
}


void LnBeta(const real_t *x, const real_t *y, real_t *res, realContext_t *realContext) {
  real_t rReal, rImag;
  real_t xx, yy;

  realCopy(x, &xx); realCopy(y, &yy);
  if(_lnBetaReal(&xx, &yy, &rReal, &rImag, realContext) && realIsZero(&rImag)) {
    realCopy(&rReal, res);
  }
  else {
    realCopy(const_NaN, res);
  }
}


static void lnbetaReal(void) {
  real_t x, y, rReal, rImag;

  if (getRegisterAsReal(REGISTER_X, &x) && getRegisterAsReal(REGISTER_Y, &y))
    if(_lnBetaReal(&x, &y, &rReal, &rImag, &ctxtReal39)) {
      if(realIsZero(&rImag))
        convertRealToResultRegister(&rReal, REGISTER_X, amNone);
      else
        convertComplexToResultRegister(&rReal, &rImag, REGISTER_X);
    }
}

static void lnbetaCplx(void)  {
  real_t xReal, xImag, yReal, yImag, rReal, rImag;

  if (getRegisterAsComplex(REGISTER_X, &xReal, &xImag)
          && getRegisterAsComplex(REGISTER_Y, &yReal, &yImag)) {
    _lnBetaComplex(&xReal, &xImag, &yReal, &yImag, &rReal, &rImag, &ctxtReal39);
    convertComplexToResultRegister(&rReal, &rImag, REGISTER_X);
  }
}

/********************************************//**
 * \brief regX ==> regL and lnBeta(regX, RegY) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnLnBeta(uint16_t unusedButMandatoryParameter) {
    processRealComplexDyadicFunction(&lnbetaReal, &lnbetaCplx);
}
