// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file uniform.c
 ***********************************************/

#include "c47.h"

#if !defined(OPTION_DIST_UNFM)
  void fnUniformP           (uint16_t discrete){}
  void fnUniformL           (uint16_t discrete){}
  void fnUniformU           (uint16_t discrete){}
  void fnUniformI           (uint16_t discrete){}
#else

static bool_t uniformCheckDomain(bool_t registersValid, real_t *x, real_t *low, real_t *high, real_t *unusedThird, uint16_t discrete) {
  if(!registersValid) {
    return false;
  }

  if(realIsNaN(x)) {
    displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function uniformCheckDomain:", "given NaN argument", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }
  if(realIsSpecial(low) || realIsSpecial(high)) {
    displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function uniformCheckDomain:", "given non-finite limits", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }
  if(discrete) {
    if(!realIsAnInteger(low)) {
      displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function uniformCheckDomain:", "given non-integer lower limit", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
    if(!realIsAnInteger(high)) {
      displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function uniformCheckDomain:", "given non-integer upper limit", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
  }

  if(realCompareGreaterThan(low, high)) {
    real_t t;
    realCopy(low, &t);
    realCopy(high, low);
    realCopy(&t, high);
  }
  if(!discrete && realCompareEqual(low, high)) {
    displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function uniformCheckDomain:", "equal continuous limits", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }
  return true;
}

static bool_t uniformParameters(real_t *x, real_t *low, real_t *high, real_t *range, int *cmp, uint16_t discrete) {
  if(!WP34S_CheckDistributionParameters(x, REGISTER_M, low, REGISTER_N, high, 0, NULL, discrete, uniformCheckDomain)) {
    return false;
  }
  if(range != NULL) {
    realSubtract(high, low, range, &ctxtReal39);
    if(discrete) {
      realAdd(range, const_1, range, &ctxtReal39);
    }
  }
  if(cmp != NULL) {
    *cmp = realCompareLessThan(x, low) ? -1 : realCompareGreaterThan(x, high) ? 1 : 0;
  }
  return true;
}

void fnUniformP(uint16_t discrete) {
  real_t x, l, h, range, *res = &x;
  int cmp;

  if(uniformParameters(&x, &l, &h, &range, &cmp, discrete)) {
    if(cmp == 0) {
      if(realIsZero(&range)) {
        res = const_1;
      }
      else if(discrete && !realIsAnInteger(&x)) {
        res = const_0;
      }
      else {
        realDivide(const_1, &range, &x, &ctxtReal39);
      }
    }
    else {
      res = const_0;
    }
    convertRealToResultRegister(res, REGISTER_X, amNone);
    adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
  }
}

void fnUniformL(uint16_t discrete) {
  real_t x, low, h, range, *res = &x;
  int cmp;

  if(uniformParameters(&x, &low, &h, &range, &cmp, discrete)) {
    if(cmp < 0) {
      res = const_0;
    }
    else if(cmp > 0 || realIsZero(&range)) {
      res = const_1;
    }
    else {
      const bool_t interior = realCompareLessThan(&x, &h);
      if(discrete) {
        realToIntegralValue(&x, &h, DEC_ROUND_FLOOR, &ctxtReal39);
        realSubtract(&h, &low, &h, &ctxtReal39);
        realAdd(&h, const_1, &h, &ctxtReal39);
      }
      else {
        realSubtract(&x, &low, &h, &ctxtReal39);
      }
      realDivide(&h, &range, &x, &ctxtReal39);
      if(interior && realCompareEqual(&x, const_1)) {
        realNextToward(&x, const_0, &x, &ctxtReal39);
      }
    }
    convertRealToResultRegister(res, REGISTER_X, amNone);
    adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
  }
}

void fnUniformU(uint16_t discrete) {
  real_t x, l, high, range, *res = &x;
  int cmp;

  if(uniformParameters(&x, &l, &high, &range, &cmp, discrete)) {
    if(cmp < 0 || realIsZero(&range)) {
      res = const_1;
    }
    else if(cmp > 0) {
      res = const_0;
    }
    else {
      const bool_t interior = realCompareGreaterThan(&x, &l);
      if(discrete) {
        realToIntegralValue(&x, &l, DEC_ROUND_CEILING, &ctxtReal39);
        realSubtract(&high, &l, &l, &ctxtReal39);
        realAdd(&l, const_1, &l, &ctxtReal39);
      }
      else {
        realSubtract(&high, &x, &l, &ctxtReal39);
      }
      realDivide(&l, &range, &x, &ctxtReal39);
      if(interior && realCompareEqual(&x, const_1)) {
        realNextToward(&x, const_0, &x, &ctxtReal39);
      }
    }
    convertRealToResultRegister(res, REGISTER_X, amNone);
    adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
  }
}

void fnUniformI(uint16_t discrete) {
  real_t x, low, high, t;

  if(uniformParameters(&x, &low, &high, NULL, NULL, discrete)) {
    if(realCompareLessThan(&x, const_0) || realCompareGreaterThan(&x, const_1)) {
      displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function fnUniformI:", "the argument must be 0 <= x <= 1", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      if(getSystemFlag(FLAG_SPCRES)) {
        convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
      }
      return;
    }
    if(realIsZero(&x)) {
      realCopy(&low, &x);
    }
    else if(realCompareEqual(&x, const_1)) {
      realCopy(&high, &x);
    }
    else if(discrete) {
      realSubtract(&high, &low, &t, &ctxtReal75);
      realAdd(&t, const_1, &t, &ctxtReal75);
      realMultiply(&x, &t, &t, &ctxtReal75);
      realToIntegralValue(&t, &t, DEC_ROUND_CEILING, &ctxtReal75);
      realSubtract(&t, const_1, &t, &ctxtReal75);
      realAdd(&low, &t, &x, &ctxtReal75);
    }
    else {
      linpol(&low, &high, &x, &t);
      realCopy(&t, &x);
    }
    convertRealToResultRegister(&x, REGISTER_X, amNone);
    adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
  }
}

#endif // !OPTION_DIST_UNFM
