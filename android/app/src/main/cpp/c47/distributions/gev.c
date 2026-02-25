// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file gev.c
 ***********************************************/

#include "c47.h"

static bool_t checkParamGEV(real_t *x, real_t *mu, real_t *sigma, real_t *xi, bool_t qf) {
  real_t t;

  if(!saveLastX())
    return false;

  if(!getRegisterAsReal(REGISTER_X, x)
      || !getRegisterAsReal(REGISTER_M, mu)
      || !getRegisterAsReal(REGISTER_S, sigma)
      || !getRegisterAsReal(REGISTER_Q, xi))
    goto err;

  if(realIsNegative(x)) {
    displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function checkParamGEV:", "cannot calculate for x < 0", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    goto err;
  }
  else if(realIsZero(sigma) || realIsNegative(sigma)) {
    displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function checkParamGEV:", "the parameter sigma must be positive", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    goto err;
  }

  // Check support range
  if(qf || realIsZero(xi))
    return true;                                // xi = 0, full line
  realSubtract(mu, sigma, &t, &ctxtReal39);
  realDivide(&t, xi, &t, &ctxtReal39);
  if(realIsNegative(xi))
    return realCompareLessEqual(x, &t);         // xi < 0, (-inf, (mu-sigma)/xi]
  return realCompareGreaterEqual(x, &t);        // xi > 0, [(mu-sigma)/xi, +inf)

  err:
  if(getSystemFlag(FLAG_SPCRES)) {
    convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
  }
  return false;
}

static void tGEV(const real_t *x, const real_t *mu, const real_t *sigma, const real_t *xi, real_t *t) {
  real_t z;

  realSubtract(x, mu, &z, &ctxtReal39);
  realDivide(&z, sigma, &z, &ctxtReal39);   // z = (x - mu) / sigma

  if(!realIsZero(xi)) {
    realMultiply(&z, xi, &z, &ctxtReal39);    // z = xi (x - mu) / sigma
    WP34S_Ln1P(&z, &z, &ctxtReal39);
    realDivide(&z, xi, &z, &ctxtReal39);
  }
  realChangeSign(&z);
  realExp(&z, t, &ctxtReal39);
}

void fnGEVP(uint16_t unusedButMandatoryParameter) {
  real_t x, mu, sigma, xi, t, u, v;

  if(!checkParamGEV(&x, &mu, &sigma, &xi, false))
    return;
  tGEV(&x, &mu, &sigma, &xi, &t);
  realAdd(&t, const_1, &u, &ctxtReal39);
  realPower(&t, &u, &v, &ctxtReal39);       // v= t^(xi+1)
  realDivide(&v, &sigma, &u, &ctxtReal39);   // u = t^(xi+1) / sigma
  realChangeSign(&t);
  realExp(&t, &v, &ctxtReal39);             // v = e^-t
  realMultiply(&v, &u, &x, &ctxtReal39);
  convertRealToResultRegister(&x, REGISTER_X, amNone);
  adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
}

static void lowerLnGEV(const real_t *x, const real_t *mu, const real_t *sigma, const real_t *xi, real_t *z) {
  tGEV(x, mu, sigma, xi, z);
  realChangeSign(z);
}

void fnGEVL(uint16_t unusedButMandatoryParameter) {
  real_t x, mu, sigma, xi;

  if(!checkParamGEV(&x, &mu, &sigma, &xi, false))
    return;
  lowerLnGEV(&x, &mu, &sigma, &xi, &x);
  realExp(&x, &x, &ctxtReal39);
  convertRealToResultRegister(&x, REGISTER_X, amNone);
  adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
}

void fnGEVR(uint16_t unusedButMandatoryParameter) {
  real_t x, mu, sigma, xi;

  if(!checkParamGEV(&x, &mu, &sigma, &xi, false))
    return;
  lowerLnGEV(&x, &mu, &sigma, &xi, &x);
  realExpM1(&x, &x, &ctxtReal39);
  realChangeSign(&x);
  convertRealToResultRegister(&x, REGISTER_X, amNone);
  adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
}

void fnGEVI(uint16_t unusedButMandatoryParameter) {
  real_t p, mu, sigma, xi, x, lnp;
  bool_t domainOkay;

  if(!checkParamGEV(&p, &mu, &sigma, &xi, true))
    return;

  domainOkay = realCompareGreaterThan(&p, const_0) || realCompareLessThan(&p, const_1);
  if(!domainOkay && !realIsZero(&xi)) {
    if(realIsNegative(&xi)) {
      domainOkay = realCompareEqual(&p, const_1);
    }
    else {
      domainOkay = realIsZero(&p);
    }
  }

  if(!domainOkay) {
    displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function fnGEVI:", "the argument out of range", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    if(getSystemFlag(FLAG_SPCRES)) {
      convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
    }
    return;
  }

  WP34S_Ln(&p, &lnp, &ctxtReal39);
  realChangeSign(&lnp);
  if(realIsZero(&xi)) {
    WP34S_Ln(&lnp, &p, &ctxtReal39);
    realChangeSign(&p);
    realFMA(&sigma, &p, &mu, &x, &ctxtReal39);
  }
  else {
    realMinus(&xi, &p, &ctxtReal39);
    PowerReal(&lnp, &p, &lnp, &ctxtReal39);
    realSubtract(&lnp, const_1, &p, &ctxtReal39);
    realDivide(&p, &xi, &lnp, &ctxtReal39);
    realFMA(&lnp, &sigma, &mu, &x, &ctxtReal39);
  }

  convertRealToResultRegister(&x, REGISTER_X, amNone);
  adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
}

