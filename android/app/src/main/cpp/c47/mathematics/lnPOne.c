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
void lnP1Complex(const cplx_t *z, cplx_t *ln, realContext_t *realContext) {
  real_t r, t;
  cplx_t sum, term;
  int n, i;

  complexMagnitude(z, &t, realContext);
  if(realCompareAbsLessThan(&t, const_1e_6)) {
    realContext_t seriesContext = *realContext;
    if(seriesContext.digits > 51) {
      seriesContext.digits = 51;
    }
    realCopy(&z->Real, &term.Real); /* Sequential terms */
    realCopy(&z->Imag, &term.Imag);
    realCopy(&z->Real, &sum.Real);  /* Running summation */
    realCopy(&z->Imag, &sum.Imag);

    /* Calculate the number of iterations for the number of requied digits.
      * The assumption is a worst case of five digits per iteration and
      * always using an odd number of iterations.
      */
    n = (seriesContext.digits / 5) | 1;
    for(i = 1; i <= n; i++) {
      mulComplexComplex(&term, z, &term, &seriesContext);
      realChangeSign(&term.Real);
      realChangeSign(&term.Imag);
      int32ToReal(i + 1, &r);
      realDivide(&term.Real, &r, &t, &seriesContext);
      realAdd(&sum.Real, &t, &sum.Real, &seriesContext);
      realDivide(&term.Imag, &r, &t, &seriesContext);
      realAdd(&sum.Imag, &t, &sum.Imag, &seriesContext);
    }
    realCopy(&sum.Real, &ln->Real);
    realCopy(&sum.Imag, &ln->Imag);
  }
  else {
    /* No numeric problems, so just do this directly */
    realAdd(&z->Real, const_1, &term.Real, realContext);  // term = 1 + z
    realCopy(&z->Imag, &term.Imag);
    if(realIsZero(&term.Real) && realIsZero(&term.Imag)) {
      realSetMinusInfinity(&ln->Real);
      realSetZero(&ln->Imag);
    }
    else {
      realCopy(&z->Imag, &sum.Imag);
      realRectangularToPolar(&term, &ln->Mag, &ln->Arg, realContext);
      if(realCompareGreaterThan(&ln->Mag, const_1on2) && realCompareLessThan(&ln->Mag, const_2)) {
        realSubtract(&term.Real, const_1, &t, realContext);
        realAdd(&term.Real, const_1, &sum.Real, realContext);
        realMultiply(&t, &sum.Real, &sum.Real, realContext);
        realFMA(&sum.Imag, &sum.Imag, &sum.Real, &sum.Real, realContext);
        WP34S_Ln1P(&sum.Real, &ln->Real, realContext);
        realMultiply(&ln->Real, const_1on2, &ln->Real, realContext);
      }
      else {
        WP34S_Ln(&ln->Mag, &ln->Real, realContext);
      }
    }
  }
  realAdd(&z->Real, const_1, &r, realContext);
}



static void lnP1Real(void) {
  real_t arg, r, x;

  if(!getRegisterAsReal(REGISTER_X, &arg)) {
    return;
  }

  realAdd(&arg, const_1, &r, &ctxtReal39);
  if(realIsZero(&r)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      realSetMinusInfinity(&x);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function lnP1Real:", "cannot calculate Ln(0) in Ln(1 + x)", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }

  else if(realIsInfinite(&r)) {
    if(!getSystemFlag(FLAG_SPCRES)) {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function lnP1Real:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input of ln(x+1) when flag SPCRES is not set", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
    else if(realIsPositive(&r)) {
      realSetPlusInfinity(&x);
    }
    else if(getFlag(FLAG_CPXRES)) {
      convertComplexPartsToResultRegister(const_plusInfinity, const39_pi, REGISTER_X);
      return;
    }
    else {
      realSetNaN(&x);
    }
  }

  else {
    if(realIsPositive(&r)) {
      WP34S_Ln1P(&arg, &x, &ctxtReal39);
     }
    else if(getFlag(FLAG_CPXRES)) {
      cplx_t z;

      realCopy(&arg, &z.Real);
      realSetZero(&z.Imag);
      lnP1Complex(&z, &z, &ctxtReal75);
      convertComplexPartsToResultRegister(&z.Real, const39_pi, REGISTER_X);
      return;
    }
    else if(getSystemFlag(FLAG_SPCRES)) {
      realSetNaN(&x);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function lnP1Real:", "cannot calculate Ln of a negative number when CPXRES is not set!", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  convertRealToResultRegister(&x, REGISTER_X, amNone);
}



static void lnP1Cplx(void) {
  cplx_t x;

  if(!getRegisterAsComplex(REGISTER_X, &x)) {
    return;
  }

  if(realIsZero(&x.Imag) && realCompareEqual(&x.Real, const__1)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      realSetMinusInfinity(&x.Real);
      realSetZero(&x.Imag);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function lnP1Cplx:", "cannot calculate Ln(0) in Ln(1 + x)", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  else {
    lnP1Complex(&x, &x, &ctxtReal75);
  }
  convertComplexToResultRegister(&x, REGISTER_X);
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
