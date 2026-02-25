// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file beta.c
 ***********************************************/

#include "c47.h"

#if !defined(SAVE_SPACE_DM42_12)
static bool_t complexBeta(real_t *xReal, real_t *xImag, real_t *yReal, real_t *yImag, real_t *rReal, real_t *rImag, realContext_t *realContext) {
  // Beta(x, y) := Gamma(x) * Gamma(y) / Gamma(x+y)
  real_t tReal, tImag;

  if(realCompareLessEqual(xReal, const_0)) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot calculate Beta of (%s, %s) with Re(x)<=0", getRegisterDataTypeName(REGISTER_Y, true, false), getRegisterDataTypeName(REGISTER_X, true, false));
      moreInfoOnError("In function complexBeta:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }
  else if(realCompareLessEqual(yReal, const_0)) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot calculate Beta of (%s, %s with Re(y)<=0", getRegisterDataTypeName(REGISTER_Y, true, false), getRegisterDataTypeName(REGISTER_X, true, false));
      moreInfoOnError("In function complexBeta:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }

  WP34S_ComplexGamma(xReal, xImag, &tReal, &tImag, realContext);              // t = Gamma(x)
  WP34S_ComplexGamma(yReal, yImag, rReal, rImag, realContext);                // r = Gamma(y)

  mulComplexComplex(rReal, rImag, &tReal, &tImag, rReal, rImag, realContext); // r = Gamma(x) * Gamma(y)

  realAdd(xReal, yReal, &tReal, realContext);                             // t = x + y
  realAdd(xImag, yImag, &tImag, realContext);

  WP34S_ComplexGamma(&tReal, &tImag, &tReal, &tImag, realContext);            // t = Gamma(x + y);
  divComplexComplex(rReal, rImag, &tReal, &tImag, rReal, rImag, realContext); // r = Gamma(x) * Gamma(y) / Gamma(x + y);

  if(realIsNaN(rImag) || realIsNaN(rReal)) {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot calculate Beta of (%s, %s) out of range", getRegisterDataTypeName(REGISTER_Y, true, false), getRegisterDataTypeName(REGISTER_X, true, false));
      moreInfoOnError("In function complexBeta:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }

  return true;
}

static bool_t realBeta(real_t *x, real_t *y, real_t *r, realContext_t *realContext) {
  // Beta(x, y) := Gamma(x) * Gamma(y) / Gamma(x+y)
  real_t tReal;

  if(realCompareLessEqual(x, const_0)) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot calculate Beta of (%s, %s) with x<=0", getRegisterDataTypeName(REGISTER_Y, true, false), getRegisterDataTypeName(REGISTER_X, true, false));
      moreInfoOnError("In function realBeta:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }
  else if(realCompareLessEqual(y, const_0)) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot calculate Beta of (%s, %s with Re(y)<=0", getRegisterDataTypeName(REGISTER_Y, true, false), getRegisterDataTypeName(REGISTER_X, true, false));
      moreInfoOnError("In function realBeta:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }

  WP34S_Gamma(x, &tReal, realContext);              // t = Gamma(x)
  WP34S_Gamma(y, r, realContext);                // r = Gamma(y)

  realMultiply(r, &tReal, r, realContext); // r = Gamma(x) * Gamma(y)

  realAdd(x, y, &tReal, realContext);                             // t = x + y

  WP34S_Gamma(&tReal, &tReal, realContext);            // t = Gamma(x + y);
  realDivide(r, &tReal, r, realContext); // r = Gamma(x) * Gamma(y) / Gamma(x + y);

  if(realIsNaN(r)) {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot calculate Beta of (%s, %s) out of range", getRegisterDataTypeName(REGISTER_Y, true, false), getRegisterDataTypeName(REGISTER_X, true, false));
      moreInfoOnError("In function realBeta:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }

  return true;
}


static void betaComplex(void) {
  real_t xReal, xImag, yReal, yImag, rReal, rImag;

  if(!getRegisterAsComplex(REGISTER_X, &xReal, &xImag) || !getRegisterAsComplex(REGISTER_Y, &yReal, &yImag))
    return;

  if(complexBeta(&xReal, &xImag, &yReal, &yImag, &rReal, &rImag, &ctxtReal75))
    convertComplexToResultRegister(&rReal, &rImag, REGISTER_X);
}


static void betaReal(void) {
  real_t r, x, y;

  if(!getRegisterAsReal(REGISTER_X, &x) || !getRegisterAsReal(REGISTER_Y, &y))
    return;

  if(realBeta(&x, &y, &r, &ctxtReal39)) {
    convertRealToResultRegister(&r, REGISTER_X, amNone);
  }
}


/********************************************//**
 * \brief regX ==> regL and beta(regX, RegY) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnBeta(uint16_t unusedButMandatoryParameter) {
  processRealComplexDyadicFunction(&betaReal, &betaComplex);
}

#endif // !SAVE_SPACE_DM42_12
