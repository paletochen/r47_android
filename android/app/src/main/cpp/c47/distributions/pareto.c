// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The C47 Authors

/********************************************//**
 * \file pareto.c
 ***********************************************/

#include "c47.h"

 #if !defined(OPTION_DIST_PRTO)
  void fnParetoP   (uint16_t unusedButMandatoryParameter){}
  void fnParetoL   (uint16_t unusedButMandatoryParameter){}
  void fnParetoU   (uint16_t unusedButMandatoryParameter){}
  void fnParetoI   (uint16_t unusedButMandatoryParameter){}
  void fnPareto2P  (uint16_t unusedButMandatoryParameter){}
  void fnPareto2L  (uint16_t unusedButMandatoryParameter){}
  void fnPareto2U  (uint16_t unusedButMandatoryParameter){}
  void fnPareto2I  (uint16_t unusedButMandatoryParameter){}
#else


/********************************************//**
 * \brief Validates the variate and parameters for a Pareto calculation.
 ***********************************************/
static bool_t paretoCheckDomain(bool_t registersValid, real_t *x, real_t *mu, real_t *sigma, real_t *alpha, uint16_t qf) {
  if(!registersValid) {
    return false;
  }

  if(realIsNaN(x) || (qf && realIsInfinite(x))) {
    if(!getSystemFlag(FLAG_SPCRES)) {
      displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function paretoCheckDomain:", "the argument is not finite", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    return false;
  }
  if((mu != NULL && realIsSpecial(mu)) || realIsSpecial(sigma) || realCompareLessEqual(sigma, const_0) ||
     realIsSpecial(alpha) || realCompareLessEqual(alpha, const_0)) {
    displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function paretoCheckDomain:", "location, scale, or shape parameter is invalid", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }
  if(!qf && realCompareLessThan(x, mu == NULL ? sigma : mu)) {
    if(!getSystemFlag(FLAG_SPCRES)) {
      displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function paretoCheckDomain:", "the argument is below the distribution support", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    return false;
  }
  return true;
}

static bool_t paretoParameters(real_t *x, real_t *mu, real_t *sigma, real_t *alpha, bool_t qf) {
  return WP34S_CheckDistributionParameters(x, REGISTER_M, mu, REGISTER_S, sigma, REGISTER_Q, alpha, qf, paretoCheckDomain);
}


/********************************************//**
 * \brief Forms the logarithm of the Pareto survival probability.
 ***********************************************/
static void paretoLogSurvival(const real_t *distance, const real_t *sigma, const real_t *alpha, real_t *res, realContext_t *realContext) {
  REAL_T_PTR(relative, realContext->digits);

  realDivide(distance, sigma, relative, realContext);
  WP34S_Ln1P(relative, res, realContext);
  realMultiply(res, alpha, res, realContext);
  realChangeSign(res);
}


/********************************************//**
 * \brief Evaluates a Pareto density from its nonnegative support distance.
 ***********************************************/
static void paretoDensity(const real_t *distance, const real_t *sigma, const real_t *alpha, real_t *res, realContext_t *realContext) {
  REAL_T_PTR(denominator, realContext->digits);
  REAL_T_PTR(logSurvival, realContext->digits);

  if(realIsInfinite(distance)) {
    realSetZero(res);
    return;
  }
  paretoLogSurvival(distance, sigma, alpha, logSurvival, realContext);
  realExp(logSurvival, res, realContext);
  realMultiply(res, alpha, res, realContext);
  realAdd(distance, sigma, denominator, realContext);
  realDivide(res, denominator, res, realContext);
  if(realIsZero(res)) {
    realNextToward(res, const_1, res, realContext);
  }
}


/********************************************//**
 * \brief Evaluates a Pareto lower probability from its support distance.
 ***********************************************/
static void paretoLower(const real_t *distance, const real_t *sigma, const real_t *alpha, real_t *res, realContext_t *realContext) {
  paretoLogSurvival(distance, sigma, alpha, res, realContext);
  WP34S_ExpM1(res, res, realContext);
  realChangeSign(res);
  if(!realIsZero(distance) && !realIsInfinite(distance)) {
    WP34S_MarkProbability(res, realContext);
  }
}


/********************************************//**
 * \brief Evaluates a Pareto upper probability from its support distance.
 ***********************************************/
static void paretoUpper(const real_t *distance, const real_t *sigma, const real_t *alpha, real_t *res, realContext_t *realContext) {
  paretoLogSurvival(distance, sigma, alpha, res, realContext);
  realExp(res, res, realContext);
  if(!realIsZero(distance) && !realIsInfinite(distance)) {
    WP34S_MarkProbability(res, realContext);
  }
}


/********************************************//**
 * \brief Evaluates log(1-p) without a finite-width subtraction near p=0.
 ***********************************************/
static void paretoLogOneMinusExtended(const real_t *probability, real_t *res, realContext_t *realContext) {
  REAL_T_PTR(argument, realContext->digits);
  REAL_T_PTR(denominator, realContext->digits);
  REAL_T_PTR(factor, realContext->digits);
  REAL_T_PTR(previous, realContext->digits);
  REAL_T_PTR(square, realContext->digits);
  REAL_T_PTR(sum, realContext->digits);
  REAL_T_PTR(term, realContext->digits);
  REAL_T_PTR(value, realContext->digits);
  uint16_t index;

  if(realCompareLessEqual(probability, const_1on2)) {
    realSubtract(const_2, probability, denominator, realContext);
    realDivide(probability, denominator, argument, realContext);
    realChangeSign(argument);
    realSetOne(factor);
  }
  else {
    realSubtract(const_1, probability, value, realContext);
    realSetOne(factor);
    while(realCompareLessEqual(value, const39_root2on2)) {
      realSquareRoot(value, value, realContext);
      realAdd(factor, factor, factor, realContext);
    }
    realSubtract(value, const_1, argument, realContext);
    realAdd(value, const_1, denominator, realContext);
    realDivide(argument, denominator, argument, realContext);
  }

  realMultiply(argument, argument, square, realContext);
  realCopy(argument, term);
  realCopy(argument, sum);
  for(index = 3; index < 2001; index += 2) {
    realMultiply(term, square, term, realContext);
    int32ToReal(index, denominator);
    realDivide(term, denominator, value, realContext);
    realCopy(sum, previous);
    realAdd(sum, value, sum, realContext);
    if(realCompareEqual(sum, previous)) {
      break;
    }
  }
  realAdd(sum, sum, sum, realContext);
  realMultiply(sum, factor, res, realContext);
}


/********************************************//**
 * \brief Evaluates exp(x)-1 for nonnegative x at extended precision.
 ***********************************************/
static void paretoExpM1Extended(const real_t *x, real_t *res, realContext_t *realContext) {
  REAL_T_PTR(denominator, realContext->digits);
  REAL_T_PTR(previous, realContext->digits);
  REAL_T_PTR(term, realContext->digits);
  uint16_t index;

  if(realCompareLessThan(x, const_1on10)) {
    realCopy(x, term);
    realCopy(x, res);
    for(index = 2; index < 1001; index++) {
      realMultiply(term, x, term, realContext);
      int32ToReal(index, denominator);
      realDivide(term, denominator, term, realContext);
      realCopy(res, previous);
      realAdd(res, term, res, realContext);
      if(realCompareEqual(res, previous)) {
        break;
      }
    }
  }
  else {
    realExp(x, res, realContext);
    realSubtract(res, const_1, res, realContext);
  }
}


/********************************************//**
 * \brief Computes the type-II quantile terms at one work precision.
 ***********************************************/
static void paretoTypeTwoQuantileAtContext(const real_t *probability, const real_t *mu, const real_t *sigma, const real_t *alpha,
                                           real_t *scaled, real_t *value, realContext_t *realContext, bool_t extendedLog) {
  REAL_T_PTR(delta, realContext->digits);
  REAL_T_PTR(logSurvival, realContext->digits);

  if(extendedLog) {
    paretoLogOneMinusExtended(probability, logSurvival, realContext);
  }
  else {
    realMinus(probability, logSurvival, realContext);
    WP34S_Ln1P(logSurvival, logSurvival, realContext);
  }
  realDivide(logSurvival, alpha, logSurvival, realContext);
  realChangeSign(logSurvival);
  if(extendedLog) {
    paretoExpM1Extended(logSurvival, delta, realContext);
  }
  else {
    WP34S_ExpM1(logSurvival, delta, realContext);
  }
  realMultiply(sigma, delta, scaled, realContext);
  realFMA(sigma, delta, mu, value, realContext);
}


/********************************************//**
 * \brief Computes a type-II Pareto quantile with adaptive cancellation work.
 ***********************************************/
static void paretoTypeTwoQuantile(const real_t *probability, const real_t *mu, const real_t *sigma, const real_t *alpha, real_t *res) {
  REAL_T_PTR(scaled, 75);
  REAL_T_PTR(value, 75);

  paretoTypeTwoQuantileAtContext(probability, mu, sigma, alpha, scaled, value, &ctxtReal51, false);
  if(WP34S_HasCancellation(mu, scaled, value, 8)) {
    paretoTypeTwoQuantileAtContext(probability, mu, sigma, alpha, scaled, value, &ctxtReal75, false);
    if(WP34S_HasCancellation(mu, scaled, value, 30)) {
      REAL_T_PTR(extendedScaled, 159);
      REAL_T_PTR(extendedValue, 159);
      realContext_t extendedContext = ctxtReal75;

      extendedContext.digits = 159;
      extendedContext.status = 0;
      paretoTypeTwoQuantileAtContext(probability, mu, sigma, alpha, extendedScaled, extendedValue, &extendedContext, true);
      realPlus(extendedValue, res, &ctxtReal75);
      return;
    }
  }
  realCopy(value, res);
}


// Type I Pareto distribution
void fnParetoP(uint16_t unusedButMandatoryParameter) {
  real_t x, sigma, alpha, distance;

  if(paretoParameters(&x, NULL, &sigma, &alpha, false)) {
    realSubtract(&x, &sigma, &distance, &ctxtReal51);
    paretoDensity(&distance, &sigma, &alpha, &x, &ctxtReal51);
    convertRealToResultRegister(&x, REGISTER_X, amNone);
    adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
  }
}

void fnParetoL(uint16_t unusedButMandatoryParameter) {
  real_t x, distance, sigma, alpha;

  if(paretoParameters(&x, NULL, &sigma, &alpha, false)) {
    realSubtract(&x, &sigma, &distance, &ctxtReal51);
    paretoLower(&distance, &sigma, &alpha, &x, &ctxtReal51);
    convertRealToResultRegister(&x, REGISTER_X, amNone);
    adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
  }
}

void fnParetoU(uint16_t unusedButMandatoryParameter) {
  real_t x, distance, sigma, alpha;

  if(paretoParameters(&x, NULL, &sigma, &alpha, false)) {
    realSubtract(&x, &sigma, &distance, &ctxtReal51);
    paretoUpper(&distance, &sigma, &alpha, &x, &ctxtReal51);
    convertRealToResultRegister(&x, REGISTER_X, amNone);
    adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
  }
}

void fnParetoI(uint16_t unusedButMandatoryParameter) {
  real_t x, delta, logSurvival, sigma, alpha;

  if(paretoParameters(&x, NULL, &sigma, &alpha, true)) {
    if(realCompareLessThan(&x, const_0) || realCompareGreaterThan(&x, const_1)) {
      if(getSystemFlag(FLAG_SPCRES)) {
        convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
      }
      else {
        displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      }
      return;
    }
    if(realIsZero(&x)) {
      realCopy(&sigma, &x);
    }
    else if(realCompareEqual(&x, const_1)) {
      realSetPlusInfinity(&x);
      convertRealToResultRegister(&x, REGISTER_X, amNone);
      return;
    }
    else {
      realMinus(&x, &logSurvival, &ctxtReal51);
      WP34S_Ln1P(&logSurvival, &logSurvival, &ctxtReal51);
      realDivide(&logSurvival, &alpha, &logSurvival, &ctxtReal51);
      realChangeSign(&logSurvival);
      WP34S_ExpM1(&logSurvival, &delta, &ctxtReal51);
      realFMA(&sigma, &delta, &sigma, &x, &ctxtReal51);
      if(realCompareEqual(&x, &sigma)) {
        realNextToward(&x, const_plusInfinity, &x, &ctxtReal51);
      }
      else if(realIsInfinite(&x)) {
        realNextToward(&x, const_0, &x, &ctxtReal51);
      }
    }
    convertRealToResultRegister(&x, REGISTER_X, amNone);
    adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
  }
}

// Type II Pareto distribution
void fnPareto2P(uint16_t unusedButMandatoryParameter) {
  real_t x, mu, sigma, alpha, distance;

  if(paretoParameters(&x, &mu, &sigma, &alpha, false)) {
    realSubtract(&x, &mu, &distance, &ctxtReal51);
    paretoDensity(&distance, &sigma, &alpha, &x, &ctxtReal51);
    convertRealToResultRegister(&x, REGISTER_X, amNone);
    adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
  }
}

void fnPareto2L(uint16_t unusedButMandatoryParameter) {
  real_t x, mu, sigma, alpha, distance;

  if(paretoParameters(&x, &mu, &sigma, &alpha, false)) {
    realSubtract(&x, &mu, &distance, &ctxtReal51);
    paretoLower(&distance, &sigma, &alpha, &x, &ctxtReal51);
    convertRealToResultRegister(&x, REGISTER_X, amNone);
    adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
  }
}

void fnPareto2U(uint16_t unusedButMandatoryParameter) {
  real_t x, mu, sigma, alpha, distance;

  if(paretoParameters(&x, &mu, &sigma, &alpha, false)) {
    realSubtract(&x, &mu, &distance, &ctxtReal51);
    paretoUpper(&distance, &sigma, &alpha, &x, &ctxtReal51);
    convertRealToResultRegister(&x, REGISTER_X, amNone);
    adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
  }
}

void fnPareto2I(uint16_t unusedButMandatoryParameter) {
  real_t x, mu, sigma, alpha;

  if(paretoParameters(&x, &mu, &sigma, &alpha, true)) {
    if(realCompareLessThan(&x, const_0) || realCompareGreaterThan(&x, const_1)) {
      if(getSystemFlag(FLAG_SPCRES)) {
        convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
      }
      else {
        displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      }
      return;
    }
    if(realIsZero(&x)) {
      realCopy(&mu, &x);
    }
    else if(realCompareEqual(&x, const_1)) {
      realSetPlusInfinity(&x);
      convertRealToResultRegister(&x, REGISTER_X, amNone);
      return;
    }
    else {
      paretoTypeTwoQuantile(&x, &mu, &sigma, &alpha, &x);
      if(realCompareEqual(&x, &mu)) {
        realNextToward(&x, const_plusInfinity, &x, &ctxtReal75);
      }
      else if(realIsInfinite(&x)) {
        realNextToward(&x, const_0, &x, &ctxtReal75);
      }
    }
    convertRealToResultRegister(&x, REGISTER_X, amNone);
    adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
  }
}

#endif // !OPTION_DIST_PRTO
