// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file beta.c
 ***********************************************/

#include "c47.h"

#if !defined(OPTION_ZETA_BETA)
  void fnBeta(uint16_t unusedButMandatoryParameter) {;}

#else //OPTION_ZETA_BETA
static bool_t complexBeta(cplx_t *x, cplx_t *y, cplx_t *r, realContext_t *realContext) {
  if(realCompareLessEqual(&x->Real, const_0)) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot calculate Beta of (%s, %s) with Re(x)<=0", getRegisterDataTypeName(REGISTER_Y, true, false), getRegisterDataTypeName(REGISTER_X, true, false));
      moreInfoOnError("In function complexBeta:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }
  else if(realCompareLessEqual(&y->Real, const_0)) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot calculate Beta of (%s, %s with Re(y)<=0", getRegisterDataTypeName(REGISTER_Y, true, false), getRegisterDataTypeName(REGISTER_X, true, false));
      moreInfoOnError("In function complexBeta:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }

  if(realIsSpecial(&x->Real) || realIsSpecial(&x->Imag) || realIsSpecial(&y->Real) || realIsSpecial(&y->Imag)) {
    if(getSystemFlag(FLAG_SPCRES) && realIsZero(&x->Imag) && realIsZero(&y->Imag) &&
       !realIsNaN(&x->Real) && !realIsNaN(&y->Real) && (realIsInfinite(&x->Real) || realIsInfinite(&y->Real))) {
      realSetZero(&r->Real);
      realSetZero(&r->Imag);
      return true;
    }
    realSetNaN(&r->Real);
    realSetNaN(&r->Imag);
  }
  else {
    C47_LnBetaPositiveComplex(x, y, r);
    expComplex(r, r, realContext);
  }

  if(realIsNaN(&r->Imag) || realIsNaN(&r->Real)) {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot calculate Beta of (%s, %s) out of range", getRegisterDataTypeName(REGISTER_Y, true, false), getRegisterDataTypeName(REGISTER_X, true, false));
      moreInfoOnError("In function complexBeta:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }

  return true;
}

static bool_t realBeta(real_t *x, real_t *y, real_t *r) {
  real_t tReal;

  if(realCompareLessEqual(x, const_0)) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot calculate Beta of (%s, %s) with x<=0", getRegisterDataTypeName(REGISTER_Y, true, false), getRegisterDataTypeName(REGISTER_X, true, false));
      moreInfoOnError("In function realBeta:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }
  else if(realCompareLessEqual(y, const_0)) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot calculate Beta of (%s, %s with Re(y)<=0", getRegisterDataTypeName(REGISTER_Y, true, false), getRegisterDataTypeName(REGISTER_X, true, false));
      moreInfoOnError("In function realBeta:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }

  if(realIsNaN(x) || realIsNaN(y)) {
    realSetNaN(r);
  }
  else if(realIsInfinite(x) || realIsInfinite(y)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      realSetZero(r);
      return true;
    }
    realSetNaN(r);
  }
  else {
    C47_LnBetaPositiveReal(x, y, &tReal);
    realExp(&tReal, r, &ctxtReal75);
  }

  if(realIsNaN(r)) {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot calculate Beta of (%s, %s) out of range", getRegisterDataTypeName(REGISTER_Y, true, false), getRegisterDataTypeName(REGISTER_X, true, false));
      moreInfoOnError("In function realBeta:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }

  return true;
}


static void betaComplex(void) {
  cplx_t x, y, r;

  if(!getRegisterAsComplex(REGISTER_X, &x) || !getRegisterAsComplex(REGISTER_Y, &y)) {
    return;
  }

  if(complexBeta(&x, &y, &r, &ctxtReal75)) {
    convertComplexToResultRegister(&r, REGISTER_X);
  }
}


static void betaReal(void) {
  real_t r, x, y;

  if(!getRegisterAsReal(REGISTER_X, &x) || !getRegisterAsReal(REGISTER_Y, &y)) {
    return;
  }

  if(realBeta(&x, &y, &r)) {
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

#endif // OPTION_ZETA_BETA
