// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file ln.c
 ***********************************************/

#include "c47.h"




void lnComplex(const real_t *real, const real_t *imag, real_t *lnReal, real_t *lnImag, realContext_t *realContext) {
  if(realIsZero(real) && realIsZero(imag)) {
    realCopy(const_minusInfinity, lnReal);
    realZero(lnImag);
  }
  else {
    realRectangularToPolar(real, imag, lnReal, lnImag, realContext);
    WP34S_Ln(lnReal, lnReal, realContext);
  }
}



/**********************************************************************
 * In all the functions below:
 * if X is a number then X = a + ib
 * The variables a and b are used for intermediate calculations
 ***********************************************************************/

void lnReal(void) {
  real_t x;

  if(!getRegisterAsReal(REGISTER_X, &x))
    return;

  if(realIsZero(&x)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      realCopy(const_minusInfinity, &x);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function lnReal:", "cannot calculate Ln(0)", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  else if(realIsInfinite(&x)) {
    if(!getSystemFlag(FLAG_SPCRES)) {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function lnReal:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input of ln when flag D is not set", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
    else if (realIsPositive(&x))
      realCopy(const_plusInfinity, &x);
    else if(getFlag(FLAG_CPXRES)) {
      convertComplexToResultRegister(const_plusInfinity, const_pi, REGISTER_X);
      return;
    } else
      realCopy(const_NaN, &x);
  } else {
    if(realIsPositive(&x)) {
      WP34S_Ln(&x, &x, &ctxtReal39);
     }
    else if(getFlag(FLAG_CPXRES)) {
      realSetPositiveSign(&x);
      WP34S_Ln(&x, &x, &ctxtReal39);
      convertComplexToResultRegister(&x, const_pi, REGISTER_X);
      return;
    }
    else if(getSystemFlag(FLAG_SPCRES)) {
      realCopy(const_NaN, &x);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function lnReal:", "cannot calculate Ln of a negative number when CPXRES is not set!", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  convertRealToResultRegister(&x, REGISTER_X, amNone);
}


void lnCplx(void) {
    real_t xReal, xImag;

  if(!getRegisterAsComplex(REGISTER_X, &xReal, &xImag))
      return;

  if(realIsZero(&xReal) && realIsZero(&xImag)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      realCopy(const_minusInfinity, &xReal);
      realZero(&xImag);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function lnCplx:", "cannot calculate Ln(0)", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  } else
    lnComplex(&xReal, &xImag, &xReal, &xImag, &ctxtReal39);
  convertComplexToResultRegister(&xReal, &xImag, REGISTER_X);
}


/********************************************//**
 * \brief regX ==> regL and ln(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnLn(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&lnReal, &lnCplx);
}
