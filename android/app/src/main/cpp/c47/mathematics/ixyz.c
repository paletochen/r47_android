// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file ixyz.c
 ***********************************************/

#include "c47.h"


/********************************************//**
 * \brief Evaluates the regularised incomplete beta for Real34 inputs.
 ***********************************************/
static bool_t ixyzEvaluate(const real_t *x, const real_t *a, const real_t *b, real_t *res) {
#if defined(OPTION_DIST_F)
  REAL_T_PTR(complement, 75);
  REAL_T_PTR(logComplement, 75);
  REAL_T_PTR(logit, 75);
  REAL_T_PTR(lower, 75);
  REAL_T_PTR(upper, 75);
  realContext_t *realContext = &ctxtReal51;
  real_t largeThreshold;

  realSetOne(&largeThreshold);
  largeThreshold.exponent = 5;
  if(realCompareGreaterEqual(a, &largeThreshold) && realCompareGreaterEqual(b, &largeThreshold)) {
    realContext = &ctxtReal75;
  }
  if(realIsZero(x)) {
    realSetZero(res);
    return true;
  }
  if(realCompareEqual(x, const_1)) {
    realSetOne(res);
    return true;
  }

  realSubtract(const_1, x, complement, realContext);
  WP34S_Ln(x, logit, realContext);
  WP34S_Ln(complement, logComplement, realContext);
  realSubtract(logit, logComplement, logit, realContext);
  if(!distributionBetaPair(a, b, x, complement, logit, lower, upper, realContext)) {
    return false;
  }
  if(realIsZero(lower)) {
    realNextToward(lower, const_1, lower, realContext);
  }
  else if(realCompareEqual(lower, const_1)) {
    realNextToward(lower, const_0, lower, realContext);
  }
  realCopy(lower, res);
  return true;
#else
  WP34S_betai(b, a, x, res, &ctxtReal75);
  return !realIsNaN(res);
#endif // OPTION_DIST_F
}


void fnIxyz(uint16_t unusedButMandatoryParameter) {
  real_t x, a, b, val;

  if(!saveLastX()) {
    return;
  }

  if(getRegisterAsReal(REGISTER_X, &x) && getRegisterAsReal(REGISTER_Y, &a) && getRegisterAsReal(REGISTER_Z, &b)) {
    if(!realIsSpecial(&x) && !realIsSpecial(&a) && !realIsSpecial(&b) &&
       realCompareGreaterEqual(&x, const_0) && realCompareLessEqual(&x, const_1) &&
       realCompareGreaterThan(&a, const_0) && realCompareGreaterThan(&b, const_0) &&
       ixyzEvaluate(&x, &a, &b, &val)) {
      convertRealToResultRegister(&val, REGISTER_X, amNone);
      fnDropY(NOPARAM);
      if(lastErrorCode == ERROR_NONE) {
        fnDropY(NOPARAM);
      }
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "not in 0 " STD_LESS_EQUAL " x " STD_LESS_EQUAL " 1, a > 0, b > 0");
        moreInfoOnError("In function fnIxyz:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }
  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot calculate Ixyz for %s, %s, %s",
        getRegisterDataTypeName(REGISTER_X, true, false),
        getRegisterDataTypeName(REGISTER_Y, true, false),
        getRegisterDataTypeName(REGISTER_Z, true, false));
      moreInfoOnError("In function fnIxyz:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

  adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
}
