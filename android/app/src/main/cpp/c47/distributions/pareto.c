// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The C47 Authors

/********************************************//**
 * \file pareto.c
 ***********************************************/

#include "c47.h"

bool_t checkParamGPD(real_t *x, real_t *mu, real_t *sigma, real_t *alpha, bool_t qf) {
  if(!saveLastX())
    return false;

  if(!getRegisterAsReal(REGISTER_X, x)
      || (mu != NULL && !getRegisterAsReal(REGISTER_M, mu))
      || !getRegisterAsReal(REGISTER_S, sigma)
      || !getRegisterAsReal(REGISTER_Q, alpha))
    goto err;

  if(realIsZero(sigma) || realIsNegative(sigma)) {
    displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function checkParamGPD:", "the parameter sigma must be positive", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    goto err;
  }
  if (!qf && realCompareLessThan(x, mu == NULL ? sigma : mu)) {
    displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function checkParamGPD:", "cannot calculate for x < sigma/mu", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    goto err;
  }
  return true;

  err:
  if(getSystemFlag(FLAG_SPCRES)) {
    convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
  }
  return false;
}

// Type I Pareto distribution
void fnParetoP(uint16_t unusedButMandatoryParameter) {
  real_t x, sigma, alpha, t, u;

  if(checkParamGPD(&x, NULL, &sigma, &alpha, false)) {
    realDivide(&sigma, &x, &t, &ctxtReal39);
    realPower(&t, &alpha, &u, &ctxtReal39);
    realDivide(&u, &x, &t, &ctxtReal39);
    realMultiply(&t, &alpha, &x, &ctxtReal39);
    convertRealToResultRegister(&x, REGISTER_X, amNone);
    adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
  }
}

void fnParetoL(uint16_t unusedButMandatoryParameter) {
  real_t x, t, sigma, alpha;

  if(checkParamGPD(&x, NULL, &sigma, &alpha, false)) {
    realDivide(&sigma, &x, &t, &ctxtReal39);
    WP34S_Ln(&t, &sigma, &ctxtReal39);
    realMultiply(&alpha, &sigma, &t, &ctxtReal39);
    realExpM1(&t, &x, &ctxtReal39);
    realChangeSign(&x);
    convertRealToResultRegister(&x, REGISTER_X, amNone);
    adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
  }
}

void fnParetoU(uint16_t unusedButMandatoryParameter) {
  real_t x, sigma, alpha, t;

  if(checkParamGPD(&x, NULL, &sigma, &alpha, false)) {
    realDivide(&sigma, &x, &t, &ctxtReal39);
    realPower(&t, &alpha, &x, &ctxtReal39);
    convertRealToResultRegister(&x, REGISTER_X, amNone);
    adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
  }
}

void fnParetoI(uint16_t unusedButMandatoryParameter) {
  real_t x, t, sigma, alpha;

  if(checkParamGPD(&x, NULL, &sigma, &alpha, true)) {
    if ((realIsNegative(&x) && !realIsZero(&x)) ||
            realCompareGreaterThan(&x, const_1)) {
      realCopy(const_NaN, &x);
    } else {
      realChangeSign(&x);
      WP34S_Ln1P(&x, &t, &ctxtReal39);
      realDivide(&t, &alpha, &x, &ctxtReal39);
      realChangeSign(&x);
      realExp(&x, &t, &ctxtReal39);
      realMultiply(&t, &sigma, &x, &ctxtReal39);
    }
    convertRealToResultRegister(&x, REGISTER_X, amNone);
    adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
  }
}

// Type II Pareto distribution
void fnPareto2P(uint16_t unusedButMandatoryParameter) {
  real_t x, mu, sigma, alpha, t;

  if(checkParamGPD(&x, &mu, &sigma, &alpha, false)) {
    realSubtract(&x, &mu, &t, &ctxtReal39);
    realAdd(&t, &sigma, &x, &ctxtReal39);   // x = x + sigma - mu
    realDivide(&sigma, &x, &t, &ctxtReal39);
    realPower(&t, &alpha, &mu, &ctxtReal39);
    realDivide(&mu, &x, &t, &ctxtReal39);
    realMultiply(&t, &alpha, &x, &ctxtReal39);
    convertRealToResultRegister(&x, REGISTER_X, amNone);
    adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
  }
}

void fnPareto2L(uint16_t unusedButMandatoryParameter) {
  real_t x, mu, sigma, alpha, t;

  if(checkParamGPD(&x, &mu, &sigma, &alpha, false)) {
    realSubtract(&x, &mu, &t, &ctxtReal39);
    realDivide(&t, &sigma, &x, &ctxtReal39);
    WP34S_Ln1P(&x, &t, &ctxtReal39);
    realMultiply(&alpha, &t, &sigma, &ctxtReal39);
    realChangeSign(&sigma);
    realExpM1(&sigma, &x, &ctxtReal39);
    realChangeSign(&x);
    convertRealToResultRegister(&x, REGISTER_X, amNone);
    adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
  }
}

void fnPareto2U(uint16_t unusedButMandatoryParameter) {
  real_t x, mu, sigma, alpha, t;

  if(checkParamGPD(&x, &mu, &sigma, &alpha, false)) {
    realSubtract(&x, &mu, &t, &ctxtReal39);
    realDivide(&t, &sigma, &x, &ctxtReal39);
    WP34S_Ln1P(&x, &t, &ctxtReal39);
    realMultiply(&alpha, &t, &sigma, &ctxtReal39);
    realChangeSign(&sigma);
    realExp(&sigma, &x, &ctxtReal39);
    convertRealToResultRegister(&x, REGISTER_X, amNone);
    adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
  }
}

void fnPareto2I(uint16_t unusedButMandatoryParameter) {
  real_t x, mu, sigma, alpha, t;

  if(checkParamGPD(&x, &mu, &sigma, &alpha, true)) {
    if ((realIsNegative(&x) && !realIsZero(&x)) ||
            realCompareGreaterThan(&x, const_1)) {
      realCopy(const_NaN, &x);
    } else {
      realChangeSign(&x);
      WP34S_Ln1P(&x, &t, &ctxtReal39);
      realDivide(&t, &alpha, &x, &ctxtReal39);
      realExp(&x, &t, &ctxtReal39); // t = (1-x)^(1/alpha)
      realSubtract(&mu, &sigma, &mu, &ctxtReal39);
      realMultiply(&mu, &t, &x, &ctxtReal39);
      realAdd(&x, &sigma, &mu, &ctxtReal39);
      realDivide(&mu, &t, &x, &ctxtReal39);
    }
    convertRealToResultRegister(&x, REGISTER_X, amNone);
    adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
  }
}

