// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file gev.c
 ***********************************************/

#include "c47.h"

#if !defined(OPTION_DIST_GEV)
  void fnGEVP                      (uint16_t unusedButMandatoryParameter){}
  void fnGEVL                      (uint16_t unusedButMandatoryParameter){}
  void fnGEVR                      (uint16_t unusedButMandatoryParameter){}
  void fnGEVI                      (uint16_t unusedButMandatoryParameter){}
#else

typedef enum {
  GEV_SUPPORT_ERROR,
  GEV_SUPPORT_INSIDE,
  GEV_SUPPORT_BELOW,
  GEV_SUPPORT_ABOVE
} gevSupport_t;

static bool_t gevCheckDomain(bool_t registersValid, real_t *x, real_t *mu, real_t *sigma, real_t *xi, uint16_t unusedParameter) {
  if(!registersValid) {
    return false;
  }
  if(realIsZero(sigma) || realIsNegative(sigma)) {
    displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function gevCheckDomain:", "the parameter sigma must be positive", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }
  return true;
}

static gevSupport_t gevParameters(real_t *x, real_t *mu, real_t *sigma, real_t *xi, bool_t qf) {
  real_t t;

  if(!WP34S_CheckDistributionParameters(x, REGISTER_M, mu, REGISTER_S, sigma, REGISTER_Q, xi, qf, gevCheckDomain)) {
    return GEV_SUPPORT_ERROR;
  }

  // Check support range
  if(qf || realIsZero(xi)) {
    return GEV_SUPPORT_INSIDE;                  // xi = 0, full line
  }
  realDivide(sigma, xi, &t, &ctxtReal39);
  realSubtract(mu, &t, &t, &ctxtReal39);
  if(realIsNegative(xi)) {
    return realCompareGreaterThan(x, &t) ? GEV_SUPPORT_ABOVE : GEV_SUPPORT_INSIDE;
  }

  return realCompareLessThan(x, &t) ? GEV_SUPPORT_BELOW : GEV_SUPPORT_INSIDE;
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
  gevSupport_t support = gevParameters(&x, &mu, &sigma, &xi, false);

  if(support == GEV_SUPPORT_ERROR) {
    return;
  }
  if(support != GEV_SUPPORT_INSIDE) {
    realSetZero(&x);
    goto finish;
  }
  tGEV(&x, &mu, &sigma, &xi, &t);
  if(realIsInfinite(&t)) {
    realSetZero(&x);
  }
  else {
    realAdd(&xi, const_1, &u, &ctxtReal39);
    if(realIsZero(&t) && realIsZero(&u)) {
      realCopy(const_1, &v);
    }
    else {
      PowerReal(&t, &u, &v, &ctxtReal39);     // v = t^(xi+1)
    }
    realDivide(&v, &sigma, &u, &ctxtReal39);  // u = t^(xi+1) / sigma
    realChangeSign(&t);
    realExp(&t, &v, &ctxtReal39);             // v = e^-t
    realMultiply(&v, &u, &x, &ctxtReal39);
  }
finish:
  convertRealToResultRegister(&x, REGISTER_X, amNone);
  adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
}

static void lowerLnGEV(const real_t *x, const real_t *mu, const real_t *sigma, const real_t *xi, real_t *z) {
  tGEV(x, mu, sigma, xi, z);
  realChangeSign(z);
}

void fnGEVL(uint16_t unusedButMandatoryParameter) {
  real_t x, mu, sigma, xi;
  gevSupport_t support = gevParameters(&x, &mu, &sigma, &xi, false);

  if(support == GEV_SUPPORT_ERROR) {
    return;
  }
  if(support == GEV_SUPPORT_BELOW) {
    realSetZero(&x);
  }
  else if(support == GEV_SUPPORT_ABOVE) {
    realCopy(const_1, &x);
  }
  else {
    lowerLnGEV(&x, &mu, &sigma, &xi, &x);
    realExp(&x, &x, &ctxtReal39);
  }
  convertRealToResultRegister(&x, REGISTER_X, amNone);
  adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
}

void fnGEVR(uint16_t unusedButMandatoryParameter) {
  real_t x, mu, sigma, xi;
  gevSupport_t support = gevParameters(&x, &mu, &sigma, &xi, false);

  if(support == GEV_SUPPORT_ERROR) {
    return;
  }
  if(support == GEV_SUPPORT_BELOW) {
    realCopy(const_1, &x);
  }
  else if(support == GEV_SUPPORT_ABOVE) {
    realSetZero(&x);
  }
  else {
    lowerLnGEV(&x, &mu, &sigma, &xi, &x);
    realExpM1(&x, &x, &ctxtReal39);
    realChangeSign(&x);
  }
  convertRealToResultRegister(&x, REGISTER_X, amNone);
  adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
}

void fnGEVI(uint16_t unusedButMandatoryParameter) {
  real_t p, mu, sigma, xi, x, lnp;
  bool_t domainOkay;

  if(gevParameters(&p, &mu, &sigma, &xi, true) == GEV_SUPPORT_ERROR) {
    return;
  }

  domainOkay = realCompareGreaterThan(&p, const_0) && realCompareLessThan(&p, const_1);
  if(!domainOkay && !realIsZero(&xi)) {
    if(realIsNegative(&xi)) {
      domainOkay = realCompareEqual(&p, const_1);
    }
    else {
      domainOkay = realIsZero(&p);
    }
  }

  if(!domainOkay) {
    displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function fnGEVI:", "the argument out of range", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    if(getSystemFlag(FLAG_SPCRES)) {
      convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
    }
    return;
  }

  if(!realIsZero(&xi) && (realIsZero(&p) || realCompareEqual(&p, const_1))) {
    realDivide(&sigma, &xi, &lnp, &ctxtReal39);
    realSubtract(&mu, &lnp, &x, &ctxtReal39);
  }
  else if(realIsZero(&xi)) {
    WP34S_Ln(&p, &lnp, &ctxtReal39);
    realChangeSign(&lnp);
    WP34S_Ln(&lnp, &p, &ctxtReal39);
    realChangeSign(&p);
    realFMA(&sigma, &p, &mu, &x, &ctxtReal39);
  }
  else {
    WP34S_Ln(&p, &lnp, &ctxtReal39);
    realChangeSign(&lnp);
    realMinus(&xi, &p, &ctxtReal39);
    PowerReal(&lnp, &p, &lnp, &ctxtReal39);
    realSubtract(&lnp, const_1, &p, &ctxtReal39);
    realDivide(&p, &xi, &lnp, &ctxtReal39);
    realFMA(&lnp, &sigma, &mu, &x, &ctxtReal39);
  }

  convertRealToResultRegister(&x, REGISTER_X, amNone);
  adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
}

#endif // !OPTION_DIST_GEV
