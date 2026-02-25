// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file lnPOne.c
 ***********************************************/
// Coded by JM, based on ln.c

#include "c47.h"


/**********************************************************************
 * For |z| small, we use the series expansion:
 *
 *  log(1+z) = z - z^2/2 + z^3/3 - z^4/4 + ...
 *
 * Thresholding |z| against 1e-6, means each term after the first will produce
 * approximately six more digits.
 *
 * For |z| larger, we can use the complex Ln function directly without loss.
 */
void lnP1Complex(const real_t *real, const real_t *imag, real_t *lnReal, real_t *lnImag, realContext_t *realContext) {
  real_t sum_real, sum_imag, term_real, term_imag, r, t;
  int n, i;

  complexMagnitude(real, imag, &t, realContext);
  if(realCompareAbsLessThan(&t, const_1e_6)) {
    realCopy(real, &term_real); /* Sequential terms */
    realCopy(imag, &term_imag);
    realCopy(real, &sum_real);  /* Running summation */
    realCopy(imag, &sum_imag);

    /* Calculate the number of iterations for the number of requied digits.
      * The assumption is a worst case of five digits per iteration and
      * always using an odd number of iterations.
      */
    n = (realContext->digits / 5) | 1;
    for(i = 1; i <= n; i++) {
      mulComplexComplex(&term_real, &term_imag, real, imag, &term_real, &term_imag, realContext);
      realChangeSign(&term_real);
      realChangeSign(&term_imag);
      int32ToReal(i + 1, &r);
      realDivide(&term_real, &r, &t, realContext);
      realAdd(&sum_real, &t, &sum_real, realContext);
      realDivide(&term_imag, &r, &t, realContext);
      realAdd(&sum_imag, &t, &sum_imag, realContext);
    }
    realCopy(&sum_real, lnReal);
    realCopy(&sum_imag, lnImag);
  }
  else {
    /* No numeric problems, so just do this directly */
    realAdd(real, const_1, &r, realContext);
    if(realIsZero(&r) && realIsZero(imag)) {
      realCopy(const_minusInfinity, lnReal);
      realZero(lnImag);
    }
    else {
      realRectangularToPolar(&r, imag, lnReal, lnImag, realContext);
      WP34S_Ln(lnReal, lnReal, realContext);
    }
  }
  realAdd(real, const_1, &r, realContext);
}



static void lnP1Real(void) {
  real_t arg, r, x;

  if(!getRegisterAsReal(REGISTER_X, &arg))
    return;

  realAdd(&arg, const_1, &r, &ctxtReal39);
  if(realIsZero(&r)) {
    if(getSystemFlag(FLAG_SPCRES)) {
        realCopy(const_minusInfinity, &x);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function lnP1Real:", "cannot calculate Ln(0) in Ln(1 + x)", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }

  else if(realIsInfinite(&r)) {
    if(!getSystemFlag(FLAG_SPCRES)) {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function lnP1Real:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input of ln(x+1) when flag D is not set", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
    else if(getFlag(FLAG_CPXRES)) {
      if(realIsPositive(&r)) {
        realCopy(const_plusInfinity, &x);
      }
      else {
        convertComplexToResultRegister(const_plusInfinity, const_pi, REGISTER_X);
        return;
      }
    }
    else {
      realCopy(const_NaN, &x);
    }
  }

  else {
    real34ToReal(&r, &x);
    if(realIsPositive(&r)) {
      WP34S_Ln1P(&arg, &x, &ctxtReal39);
     }
    else if(getFlag(FLAG_CPXRES)) {
      lnP1Complex(&arg, const_0, &x, &r, &ctxtReal75);
      convertComplexToResultRegister(&x, const_pi, REGISTER_X);
      return;
    }
    else if(getSystemFlag(FLAG_SPCRES)) {
      realCopy(const_NaN, &x);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function lnP1Real:", "cannot calculate Ln of a negative number when CPXRES is not set!", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  convertRealToResultRegister(&x, REGISTER_X, amNone);
}



static void lnP1Cplx(void) {
  real_t xReal, xImag;

  if (!getRegisterAsComplex(REGISTER_X, &xReal, &xImag))
      return;

  if(realIsZero(&xImag) && realCompareEqual(&xReal, const__1)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      realCopy(const_minusInfinity, &xReal);
      realZero(&xImag);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function lnP1Cplx:", "cannot calculate Ln(0) in Ln(1 + x)", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  else
    lnP1Complex(&xReal, &xImag, &xReal, &xImag, &ctxtReal75);
  convertComplexToResultRegister(&xReal, &xImag, REGISTER_X);
}


/********************************************//**
 * \brief regX ==> regL and lnP1(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnLnP1(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&lnP1Real, &lnP1Cplx);
}
