// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file wp34s.c
 ***********************************************/

#include "c47.h"

/******************************************************
 * This functions are borrowed from the WP34S project
 ******************************************************/


#if !defined(PC_BUILD)
  #undef CACHE_DEBUG
  #undef CACHE_VERIFY
#endif

/****************************************************************************************************
 * Modified for direct handling of 1071 digit contexts.
 * Take care not to use the 1071 version for the DM42 hardware
 ****************************************************************************************************/
#undef DEBUGTAYLOR

#define TaylorIterationMax 1000
#if !defined(PC_BUILD)
  #undef DEBUGTAYLOR
#endif


/**
 * \brief Loads and checks distribution parameters.
 *
 * \param[out] x Argument from register X
 * \param[in] firstRegister Register for the first parameter
 * \param[out] first First parameter, or NULL
 * \param[in] secondRegister Register for the second parameter
 * \param[out] second Second parameter, or NULL
 * \param[in] thirdRegister Register for the third parameter
 * \param[out] third Third parameter, or NULL
 * \param[in] parameter Distribution-specific selector
 * \param[in] domainRule Distribution domain check
 * \return true when the registers and domain are valid
 */
bool_t WP34S_CheckDistributionParameters(real_t *x, calcRegister_t firstRegister, real_t *first, calcRegister_t secondRegister, real_t *second,
                                         calcRegister_t thirdRegister, real_t *third, uint16_t parameter, wp34sDistributionDomainRule_t domainRule) {
  bool_t registersValid;

  if(!saveLastX()) {
    return false;
  }

  registersValid = getRegisterAsReal(REGISTER_X, x) &&
                   (first == NULL || getRegisterAsReal(firstRegister, first)) &&
                   (second == NULL || getRegisterAsReal(secondRegister, second)) &&
                   (third == NULL || getRegisterAsReal(thirdRegister, third));
  if(!registersValid) {
    domainRule(false, x, first, second, third, parameter);
    if(getSystemFlag(FLAG_SPCRES)) {
      convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
    }
    return false;
  }
  if(!domainRule(true, x, first, second, third, parameter)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
    }
    return false;
  }
  return true;
}


#if defined(OPTION_DIST_EXPON) || defined(OPTION_DIST_CAUCH) || defined(OPTION_DIST_LOGIS) || defined(OPTION_DIST_WEIBL) || \
    defined(OPTION_DIST_BINOM) || defined(OPTION_DIST_GEOM)  || defined(OPTION_DIST_POISS) || defined(OPTION_DIST_NBIN)  || \
    defined(OPTION_DIST_CHI2)  || defined(OPTION_DIST_T)     || defined(OPTION_DIST_F)
// One row per distribution, in the order of the DIST_ names. A row whose option is off stays zero, and its NULL kernel is what makes the entry do nothing.
TO_QSPI const distribution_t distributions[DIST_COUNT] = {
  #if defined(OPTION_DIST_EXPON)
    [DIST_EXPON] = {REGISTER_R, 0, 1, false, DIST_INV_NOROOT | DIST_INV_NOROOT_NAN | DIST_ERR_RETURNS, {DIST_CTX_39, DIST_CTX_39, DIST_CTX_39, DIST_CTX_39}, exponentialCheckDomain,
                 {{.one = WP34S_Pdf_Expon}, {.one = WP34S_Cdf_Expon}, {.one = WP34S_Cdfu_Expon}, {.one = WP34S_Qf_Expon}}},
  #endif // OPTION_DIST_EXPON
  #if defined(OPTION_DIST_CAUCH)
    [DIST_CAUCHY] = {REGISTER_M, REGISTER_S, 2, false, DIST_INV_NAN_ARG | DIST_INV_NOROOT | DIST_INV_NOROOT_NAN | DIST_ERR_RETURNS, {DIST_CTX_39, DIST_CTX_39, DIST_CTX_39, DIST_CTX_39}, cauchyCheckDomain,
                 {{.two = WP34S_Pdf_Cauchy}, {.two = WP34S_Cdf_Cauchy}, {.two = WP34S_Cdfu_Cauchy}, {.two = WP34S_Qf_Cauchy}}},
  #endif // OPTION_DIST_CAUCH
  #if defined(OPTION_DIST_LOGIS)
    [DIST_LOGIT] = {REGISTER_M, REGISTER_S, 2, false, DIST_INV_NAN_ARG, {DIST_CTX_39, DIST_CTX_39, DIST_CTX_39, DIST_CTX_39}, logisticCheckDomain,
                 {{.two = WP34S_Pdf_Logit}, {.two = WP34S_Cdf_Logit}, {.two = WP34S_Cdfu_Logit}, {.two = WP34S_Qf_Logit}}},
  #endif // OPTION_DIST_LOGIS
  #if defined(OPTION_DIST_WEIBL)
    [DIST_WEIB] = {REGISTER_Q, REGISTER_S, 2, true, 0, {DIST_CTX_51, DIST_CTX_39, DIST_CTX_39, DIST_CTX_39}, weibullCheckDomain,
                 {{.two = WP34S_Pdf_Weib}, {.two = WP34S_Cdf_Weib}, {.two = WP34S_Cdfu_Weib}, {.two = WP34S_Qf_Weib}}},
  #endif // OPTION_DIST_WEIBL
  #if defined(OPTION_DIST_BINOM)
    [DIST_BINOM] = {REGISTER_P, REGISTER_N, 2, false, DIST_INV_NOROOT | DIST_INV_NOROOT_NAN | DIST_ERR_RETURNS | DIST_PDF_INTEGER_ONLY | DIST_PDF_NAN_IS_ERROR, {DIST_CTX_39, DIST_CTX_39, DIST_CTX_39, DIST_CTX_39}, binomialCheckDomain,
                 {{.two = WP34S_Pdf_Binomial}, {.two = WP34S_Cdf_Binomial}, {.two = WP34S_Cdfu_Binomial}, {.two = WP34S_Qf_Binomial}}},
  #endif // OPTION_DIST_BINOM
  #if defined(OPTION_DIST_GEOM)
    [DIST_GEOM] = {REGISTER_P, 0, 1, false, 0, {DIST_CTX_39, DIST_CTX_39, DIST_CTX_39, DIST_CTX_39}, geometricCheckDomain,
                 {{.one = WP34S_Pdf_Geom}, {.one = WP34S_Cdf_Geom}, {.one = WP34S_Cdfu_Geom}, {.one = WP34S_Qf_Geom}}},
  #endif // OPTION_DIST_GEOM
  #if defined(OPTION_DIST_POISS)
    [DIST_POISSON] = {REGISTER_R, 0, 1, false, DIST_INV_NAN_ARG | DIST_INV_REPORT_FIRST | DIST_INV_NOROOT | DIST_INV_NOROOT_NAN | DIST_ERR_RETURNS, {DIST_CTX_39, DIST_CTX_39, DIST_CTX_39, DIST_CTX_39}, poissonCheckDomain,
                 {{.one = WP34S_Pdf_Poisson}, {.one = WP34S_Cdf_Poisson}, {.one = WP34S_Cdfu_Poisson}, {.one = WP34S_Qf_Poisson}}},
  #endif // OPTION_DIST_POISS
  #if defined(OPTION_DIST_NBIN)
    [DIST_NEGBINOM] = {REGISTER_P, REGISTER_N, 2, false, DIST_INV_NOROOT | DIST_INV_NOROOT_NAN | DIST_ERR_RETURNS | DIST_PDF_INTEGER_ONLY | DIST_PDF_NAN_IS_ERROR, {DIST_CTX_39, DIST_CTX_39, DIST_CTX_39, DIST_CTX_39}, negativeBinomialCheckDomain,
                 {{.two = pdf_NegBinomial}, {.two = cdf_NegBinomial}, {.two = cdfu_NegBinomial}, {.two = qf_NegBinomial}}},
  #endif // OPTION_DIST_NBIN
  #if defined(OPTION_DIST_CHI2)
    [DIST_CHI2] = {REGISTER_M, 0, 1, false, DIST_INV_NAN_ARG | DIST_INV_REPORT_FIRST | DIST_INV_NOROOT | DIST_INV_NOROOT_NAN | DIST_ERR_RETURNS, {DIST_CTX_39, DIST_CTX_39, DIST_CTX_39, DIST_CTX_39}, chi2CheckDomain,
                 {{.one = WP34S_Pdf_Chi2}, {.one = WP34S_Cdf_Chi2}, {.one = WP34S_Cdfu_Chi2}, {.one = WP34S_Qf_Chi2}}},
  #endif // OPTION_DIST_CHI2
  #if defined(OPTION_DIST_T)
    [DIST_T] = {REGISTER_M, 0, 1, false, DIST_INV_NAN_ARG | DIST_INV_REPORT_FIRST | DIST_INV_NOROOT, {DIST_CTX_39, DIST_CTX_39, DIST_CTX_39, DIST_CTX_39}, studentTCheckDomain,
                 {{.one = WP34S_Pdf_T}, {.one = WP34S_Cdf_T}, {.one = WP34S_Cdfu_T}, {.one = WP34S_Qf_T}}},
  #endif // OPTION_DIST_T
  #if defined(OPTION_DIST_F)
    [DIST_F] = {REGISTER_M, REGISTER_N, 2, false, DIST_INV_NAN_ARG | DIST_INV_REPORT_FIRST | DIST_INV_NOROOT | DIST_INV_NOROOT_NAN | DIST_ERR_RETURNS, {DIST_CTX_39, DIST_CTX_39, DIST_CTX_39, DIST_CTX_39}, fCheckDomain,
                 {{.two = WP34S_Pdf_F}, {.two = WP34S_Cdf_F}, {.two = WP34S_Cdfu_F}, {.two = WP34S_Qf_F}}},
  #endif // OPTION_DIST_F
};


// Every entry point of every distribution that takes its parameters from the stat registers. param names the descriptor row and which of the four entries it is,
// and the row's flags state what that distribution does on top of the five lines they all share.
void fnDistribution(uint16_t param) {
  const distribution_t *d = &distributions[param >> 2];
  const uint8_t which = param & 3;
  const uint8_t flags = d->flags;
  real_t val, first, second, ans;
  realContext_t *realContext;

  if(d->kernel[which].two == NULL) {
    return;
  }
  if(!WP34S_CheckDistributionParameters(&val, d->firstRegister, &first, d->secondRegister, d->arity > 1 ? &second : NULL, 0, NULL, 0, d->domainRule)) {
    return;
  }

  if(which == DIST_I && (((flags & DIST_INV_NAN_ARG) && realIsNaN(&val)) || realCompareLessEqual(&val, const_0) || realCompareGreaterEqual(&val, const_1))) {
    if(flags & DIST_INV_REPORT_FIRST) {
      displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function fnDistribution:", "the argument must be 0 < x < 1", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      if(getSystemFlag(FLAG_SPCRES)) {
        convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
      }
    }
    else if(getSystemFlag(FLAG_SPCRES)) {
      convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
    }
    else {
      displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function fnDistribution:", "the argument must be 0 < x < 1", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    return;
  }

  realContext = d->context[which] == DIST_CTX_75 ? &ctxtReal75 : (d->context[which] == DIST_CTX_51 ? &ctxtReal51 : &ctxtReal39);
  if(which == DIST_P && (flags & DIST_PDF_INTEGER_ONLY) && !realIsAnInteger(&val)) {
    realSetZero(&ans);
  }
  else if(d->arity > 1) {
    d->kernel[which].two(&val, d->secondFirst ? &second : &first, d->secondFirst ? &first : &second, &ans, realContext);
  }
  else {
    d->kernel[which].one(&val, &first, &ans, realContext);
  }

  if(realIsNaN(&ans) && which == DIST_I && (flags & DIST_INV_NOROOT)) {
    displayDomainErrorMessage(ERROR_NO_ROOT_FOUND, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function fnDistribution:", "the quantile did not produce a finite result", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    if((flags & DIST_INV_NOROOT_NAN) && getSystemFlag(FLAG_SPCRES)) {
      convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
    }
    if(flags & DIST_ERR_RETURNS) {
      return;
    }
  }
  else if(realIsNaN(&ans) && which == DIST_P && (flags & DIST_PDF_NAN_IS_ERROR)) {
    displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function fnDistribution:", "a parameter is invalid", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else {
    convertRealToResultRegister(&ans, REGISTER_X, amNone);
  }
  adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
}

#else // the packages that take none of these distributions pay for none of this
void fnDistribution(uint16_t unusedButMandatoryParameter) {}
#endif // any of the eleven


/**
 * \brief Tests whether a distribution parameter is positive and finite.
 *
 * \param[in] value Parameter to test
 * \return true when the parameter is positive and finite
 */
bool_t WP34S_IsPositiveFinite(const real_t *value) {
  return !realIsSpecial(value) && realCompareGreaterThan(value, const_0);
}


/**
 * \brief Selects a caller context when it is wider than a fallback context.
 *
 * \param[in] realContext Caller context
 * \param[in] fallbackContext Minimum calculation context
 * \return Selected context
 */
realContext_t *WP34S_AtLeastContext(realContext_t *realContext, realContext_t *fallbackContext) {
  return realContext->digits > fallbackContext->digits ? realContext : fallbackContext;
}


/**
 * \brief Selects p75 when both incomplete-beta shapes enter the concentrated range.
 *
 * \param[in] a First beta shape
 * \param[in] b Second beta shape
 * \return Calculation context
 */
realContext_t *WP34S_BetaCalculationContext(const real_t *a, const real_t *b) {
  real_t threshold;

  WP34S_SetPowerOfTen(5, &threshold);
  return realCompareGreaterEqual(a, &threshold) && realCompareGreaterEqual(b, &threshold) ? &ctxtReal75 : &ctxtReal51;
}


/**
 * \brief Sets a real value to a power of ten.
 *
 * \param[in] exponent Decimal exponent
 * \param[out] res Power of ten
 */
void WP34S_SetPowerOfTen(int32_t exponent, real_t *res) {
  realSetOne(res);
  res->exponent = exponent;
}


/**
 * \brief Retains the strict side of a finite non-endpoint probability.
 *
 * \param[in,out] value Probability to mark
 * \param[in] realContext Arithmetic context
 */
void WP34S_MarkProbability(real_t *value, realContext_t *realContext) {
  if(realIsZero(value)) {
    realNextToward(value, const_1, value, realContext);
  }
  else if(realCompareEqual(value, const_1)) {
    realNextToward(value, const_0, value, realContext);
  }
}


/**
 * \brief Retains the positive side of an underflowed finite value.
 *
 * \param[in,out] value Value to mark
 * \param[in] realContext Arithmetic context
 */
void WP34S_MarkPositive(real_t *value, realContext_t *realContext) {
  if(realIsZero(value)) {
    realNextToward(value, const_1, value, realContext);
  }
}


/**
 * \brief Detects cancellation beyond a selected number of decimal places.
 *
 * \param[in] left Left operand
 * \param[in] right Right operand
 * \param[in] result Subtraction result
 * \param[in] places Maximum permitted cancellation
 * \return true when cancellation exceeds places
 */
bool_t WP34S_HasCancellation(const real_t *left, const real_t *right, const real_t *result, int32_t places) {
  int32_t inputExponent;

  if(realIsZero(left) || realIsZero(right) || realIsSpecial(result)) {
    return false;
  }
  if(realIsZero(result)) {
    return true;
  }
  inputExponent = max(realGetExponent(left), realGetExponent(right));
  return inputExponent - realGetExponent(result) > places;
}


/**
 * \brief Forms a probability, its complement, their logarithms, and their logit without endpoint cancellation.
 *
 * \param[in] probability Probability
 * \param[out] complement Complementary probability
 * \param[out] logProbability Logarithm of the probability
 * \param[out] logComplement Logarithm of the complementary probability
 * \param[out] logit Difference of the logarithms
 * \param[in] realContext Arithmetic context
 */
void WP34S_DistributionProbabilityData(const real_t *probability, real_t *complement, real_t *logProbability, real_t *logComplement, real_t *logit,
                                       realContext_t *realContext) {
  real_t negative;

  realSubtract(const_1, probability, complement, realContext);
  if(realCompareLessEqual(probability, complement)) {
    WP34S_Ln(probability, logProbability, realContext);
    realMinus(probability, &negative, realContext);
    WP34S_Ln1P(&negative, logComplement, realContext);
  }
  else {
    WP34S_Ln(complement, logComplement, realContext);
    realMinus(complement, &negative, realContext);
    WP34S_Ln1P(&negative, logProbability, realContext);
  }
  realSubtract(logProbability, logComplement, logit, realContext);
}


/**
 * \brief Applies real39 positive-underflow rounding to an extended positive value.
 *
 * \param[in,out] value Value to round
 */
void WP34S_AdjustPositiveUnderflow(real_t *value) {
  real_t minimum, halfMinimum;
  bool_t roundUp = false;

  WP34S_SetPowerOfTen(-6176, &minimum);
  if(!realIsZero(value) && realCompareGreaterEqual(value, &minimum)) {
    return;
  }
  realMultiply(&minimum, const_1on2, &halfMinimum, &ctxtReal39);
  switch(roundingModeTable[roundingMode]) {
    case DEC_ROUND_HALF_EVEN:
    case DEC_ROUND_HALF_DOWN:
      roundUp = !realIsZero(value) && realCompareGreaterThan(value, &halfMinimum);
      break;
    case DEC_ROUND_HALF_UP:
      roundUp = !realIsZero(value) && realCompareGreaterEqual(value, &halfMinimum);
      break;
    case DEC_ROUND_UP:
    case DEC_ROUND_CEILING:
      roundUp = true;
      break;
    default:
      break;
  }
  if(roundUp) {
    realCopy(&minimum, value);
  }
  else {
    realSetZero(value);
  }
}


/**
 * \brief Calculates log(1+x)-x without cancellation for a small argument.
 *
 * \param[in] value Argument
 * \param[out] res Result
 * \param[in] realContext Arithmetic context
 */
void WP34S_Log1pMinus(const real_t *value, real_t *res, realContext_t *realContext) {
  real_t term, sum, previous, numerator, denominator;
  int32_t index;

  if(!realCompareAbsLessThan(value, const_1e_6)) {
    WP34S_Ln1P(value, res, realContext);
    realSubtract(res, value, res, realContext);
    return;
  }
  realMultiply(value, value, &term, realContext);
  realMultiply(&term, const_1on2, &term, realContext);
  realChangeSign(&term);
  realCopy(&term, &sum);
  for(index=2; index<100; index++) {
    realCopy(&sum, &previous);
    realMultiply(&term, value, &term, realContext);
    realChangeSign(&term);
    int32ToReal(index, &numerator);
    int32ToReal(index + 1, &denominator);
    realMultiply(&term, &numerator, &term, realContext);
    realDivide(&term, &denominator, &term, realContext);
    realAdd(&sum, &term, &sum, realContext);
    if(realCompareEqual(&sum, &previous)) {
      break;
    }
  }
  realCopy(&sum, res);
}


/**
 * \brief Computes a reduced natural logarithm at an extended precision.
 *
 * \param[in] x Positive argument
 * \param[out] res Result
 * \param[in] realContext Arithmetic context
 */
static void wp34sLnReduced(const real_t *x, real_t *res, realContext_t *realContext) {
  REAL_T_ALLOC(divisor,  realContext->digits);  // the ln series working reals, in a deep call chain under the solver, go on the heap not the DM42 frame
  REAL_T_ALLOC(factor,   realContext->digits);
  REAL_T_ALLOC(previous, realContext->digits);
  REAL_T_ALLOC(square,   realContext->digits);
  REAL_T_ALLOC(sum,      realContext->digits);
  REAL_T_ALLOC(term,     realContext->digits);
  REAL_T_ALLOC(z,        realContext->digits);
  if(divisor == NULL || factor == NULL || previous == NULL || square == NULL || sum == NULL || term == NULL || z == NULL) {
    REAL_T_FREE(divisor,  realContext->digits);
    REAL_T_FREE(factor,   realContext->digits);
    REAL_T_FREE(previous, realContext->digits);
    REAL_T_FREE(square,   realContext->digits);
    REAL_T_FREE(sum,      realContext->digits);
    REAL_T_FREE(term,     realContext->digits);
    REAL_T_FREE(z,        realContext->digits);
    realSetNaN(res);
    return;
  }

  realCopy(x, z);
  realSetOne(factor);
  while(realCompareLessEqual(z, const39_root2on2) || realCompareGreaterEqual(z, const39_root2)) {
    realSquareRoot(z, z, realContext);
    realMultiply(factor, const_2, factor, realContext);
  }
  realSubtract(z, const_1, term, realContext);
  realAdd(z, const_1, sum, realContext);
  realDivide(term, sum, term, realContext);
  realMultiply(term, term, square, realContext);
  realCopy(term, sum);
  int32ToReal(3, divisor);
  for(int32_t n = 0; n < 1000; n++) {
    realMultiply(term, square, term, realContext);
    realDivide(term, divisor, z, realContext);
    realCopy(sum, previous);
    realAdd(sum, z, sum, realContext);
    if(realCompareEqual(sum, previous)) {
      break;
    }
    realAdd(divisor, const_2, divisor, realContext);
  }
  realMultiply(sum, factor, res, realContext);
  realMultiply(res, const_2, res, realContext);
  REAL_T_FREE(divisor,  realContext->digits);
  REAL_T_FREE(factor,   realContext->digits);
  REAL_T_FREE(previous, realContext->digits);
  REAL_T_FREE(square,   realContext->digits);
  REAL_T_FREE(sum,      realContext->digits);
  REAL_T_FREE(term,     realContext->digits);
  REAL_T_FREE(z,        realContext->digits);
}


/**
 * \brief Computes a natural logarithm from decimal range reduction and an atanh series.
 *
 * \param[in] x Positive argument
 * \param[out] res Result
 * \param[in] realContext Arithmetic context
 */
void WP34S_LnExtended(const real_t *x, real_t *res, realContext_t *realContext) {
  int32_t decimalExponent;

  if(realCompareGreaterEqual(x, const_1on2) && realCompareLessEqual(x, const_3on2)) {
    wp34sLnReduced(x, res, realContext);
    return;
  }

  REAL_T_ALLOC(exponent,  realContext->digits);  // the ln working reals, in a deep call chain under the solver, go on the heap not the DM42 frame
  REAL_T_ALLOC(logarithm, realContext->digits);
  REAL_T_ALLOC(logTen,    realContext->digits);
  REAL_T_ALLOC(mantissa,  realContext->digits);
  if(exponent == NULL || logarithm == NULL || logTen == NULL || mantissa == NULL) {
    REAL_T_FREE(exponent,  realContext->digits);
    REAL_T_FREE(logarithm, realContext->digits);
    REAL_T_FREE(logTen,    realContext->digits);
    REAL_T_FREE(mantissa,  realContext->digits);
    realSetNaN(res);
    return;
  }

  realCopy(x, mantissa);
  decimalExponent = realGetUnadjustedExponent(mantissa) + realStoredDigits(mantissa);
  realSetUnadjustedExponent(mantissa, -realStoredDigits(mantissa));
  wp34sLnReduced(mantissa, logarithm, realContext);
  wp34sLnReduced(const_10, logTen, realContext);
  int32ToReal(decimalExponent, exponent);
  realFMA(exponent, logTen, logarithm, res, realContext);
  REAL_T_FREE(exponent,  realContext->digits);
  REAL_T_FREE(logarithm, realContext->digits);
  REAL_T_FREE(logTen,    realContext->digits);
  REAL_T_FREE(mantissa,  realContext->digits);
}
static void C47_WP34S_SinCosTanTaylor_temp75   (const real_t *angle, bool_t swap, real_t *sinOut, real_t *cosOut, real_t *tanOut, realContext_t *realContext); // angle in radian
#if defined(OPTION_XFN_1000)
  static void C47_WP34S_SinCosTanTaylor_temp1071 (const real_t *angle, bool_t swap, real_t *sinOut, real_t *cosOut, real_t *tanOut, realContext_t *realContext); // angle in radian
#endif // OPTION_XFN_1000


#if defined(DMCP_BUILD) && HARDWARE_MODEL == HWM_DM42
  #define RADIAN_REDUCTION_DIGITS 2139
  #define RADIAN_REDUCTION_2PI    const2139_2pi    // DM42 reduces with the 2139-digit 2pi; the 12321-digit 2pi NaNs on the DM42 decNumber at this 2139 context
#else
  #define RADIAN_REDUCTION_DIGITS 12321
  #define RADIAN_REDUCTION_2PI    const12321_2pi
#endif // defined(DMCP_BUILD) && HARDWARE_MODEL == HWM_DM42


/**
 * \brief Reduces a nonnegative radian angle to the first octant without shortening before quadrant subtraction.
 *
 * \param[in,out] angle Radian angle on entry and first-octant angle on return
 * \param[in,out] sinNeg Sign selector for sine
 * \param[in,out] cosNeg Sign selector for cosine
 * \param[in,out] swap Exchange selector for sine and cosine
 * \param[in] realContext Context for the shortened first-octant result
 */
static void reduceRadianToFirstOctant(real_t *angle, bool_t *sinNeg, bool_t *cosNeg, bool_t *swap, realContext_t *realContext) {
  REAL_T_ALLOC(work, RADIAN_REDUCTION_DIGITS);
  realContext_t c = *realContext;
  uint32_t quadrant = 0;

  if(work == NULL) {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    realSetNaN(angle);
    return;
  }

  c.digits = RADIAN_REDUCTION_DIGITS;
  realDivideRemainder(angle, RADIAN_REDUCTION_2PI, work, &c);
  if(realIsNaN(work)) {
    realCopy(work, angle);
    REAL_T_FREE(work, RADIAN_REDUCTION_DIGITS);
    return;
  }

  // Four times the remainder identifies its quadrant without shortening a value close to a multiple of pi/2.
  realMultiply(work, const_4, work, &c);
  while(realCompareGreaterEqual(work, RADIAN_REDUCTION_2PI)) {
    realSubtract(work, RADIAN_REDUCTION_2PI, work, &c);
    quadrant++;
  }

  switch(quadrant) {
    case 1:
      *swap = !(*swap);
      *cosNeg = !(*cosNeg);
      break;
    case 2:
      *sinNeg = !(*sinNeg);
      *cosNeg = !(*cosNeg);
      break;
    case 3:
      *swap = !(*swap);
      *sinNeg = !(*sinNeg);
      break;
    default:
      break;
  }

  // The value is now four times an angle in [0, pi/2). Fold its upper half before shortening it.
  realMultiply(work, const_2, work, &c);
  if(realCompareGreaterThan(work, RADIAN_REDUCTION_2PI)) {
    realSubtract(work, RADIAN_REDUCTION_2PI, work, &c);
    realSubtract(RADIAN_REDUCTION_2PI, work, work, &c);
    *swap = !(*swap);
  }
  realDivide(work, const_8, angle, realContext);

  REAL_T_FREE(work, RADIAN_REDUCTION_DIGITS);
}


void reduceAngleToRange(real_t* angle, const real_t** angle45, const real_t** angle90, const real_t** angle180, angularMode_t* angularMode, int32_t savedContextDigits, realContext_t* realContext) {
  switch(*angularMode) {
    case amRadian: {
      if(savedContextDigits >= 1071) {
        *angle45  = const1071_piOn4;
        *angle90  = const1071_piOn2;
        *angle180 = const1071_pi;
      }
      else {
        *angle45  = const75_piOn4;
        *angle90  = const75_piOn2;
        *angle180 = const75_pi;
      }
      mod2Pi(angle, angle, realContext); // mod(angle, 2pi) --> angle
      break;
    }

    case amMultPi: {
      *angle45  = const_1on4;
      *angle90  = const_1on2;
      *angle180 = const_1;
      WP34S_Mod(angle, const_2, angle, realContext); // mod(angle, 2) --> angle
      break;
    }

    case amGrad: {
      *angle45  = const_50;
      *angle90  = const_100;
      *angle180 = const_200;
      WP34S_Mod(angle, const_400, angle, realContext); // mod(angle, 400g) --> angle
      break;
    }

    case amDegree:
    case amDMS: {
      *angle45  = const_45;
      *angle90  = const_90;
      *angle180 = const_180;
      WP34S_Mod(angle, const_360, angle, realContext); // mod(angle, 360°) --> angle
      *angularMode = amDegree;
      break;
    }

    default: {
    }
  }
}


// called from WP34S_Cvt2RadSinCosTan for 75 digits max, by by agm, sin, sinc, cos, tan, multiple elliptic functions, exp (complex), fib, gd, tanh, WP34S_Zeta
// called from C47_WP34S_Cvt2RadSinCosTan for 1071+ XFN
static void doWP34S_SinCosTanTaylor(real_t* angle, bool* sinNeg, bool* cosNeg, bool* swap, real_t* sinOut, real_t* cosOut, real_t* tanOut, angularMode_t angularMode, int32_t savedContextDigits, realContext_t* realContext) {
  const real_t *angle45, *angle90, *angle180;
  angle45  = const_0;
  angle90  = const_0;
  angle180 = const_0;

  #if defined(DEBUGTAYLOR)
   realToString(angle, tmpString);
   //tmpString[80]=0;
   printf("Angle:   %s\n", tmpString);
  #endif //DEBUGTAYLOR

  // sin(-x) = -sin(x), cos(-x) = cos(x)
  if(realIsNegative(angle)) {
    *sinNeg = true;
    realSetPositiveSign(angle);
  }

  if(angularMode == amRadian) {
    reduceRadianToFirstOctant(angle, sinNeg, cosNeg, swap, realContext);
  }
  else {
    reduceAngleToRange(angle, &angle45, &angle90, &angle180, &angularMode, savedContextDigits, realContext);
  }

  #if defined(DEBUGTAYLOR)
   realToString(angle, tmpString);
   //tmpString[80]=0;
   printf("Reduced: %s\n", tmpString);
  #endif //DEBUGTAYLOR

  // sin(180+x) = -sin(x), cos(180+x) = -cos(x)
  if(angularMode != amRadian && realCompareGreaterEqual(angle, angle180)) {        // angle >= 180°
    realSubtract(angle, angle180, angle, realContext); // angle - 180° --> angle
    *sinNeg = !(*sinNeg);
    *cosNeg = !(*cosNeg);
  }

  // sin(90+x) = cos(x), cos(90+x) = -sin(x)
  if(angularMode != amRadian && realCompareGreaterEqual(angle, angle90)) {        // angle >= 90°
    realSubtract(angle, angle90, angle, realContext); // angle - 90° --> angle
    *swap = true;
    *cosNeg = !(*cosNeg);
  }

  // sin(90-x) = cos(x), cos(90-x) = sin(x)
  if(angularMode != amRadian && realCompareEqual(angle, angle45)) { // angle == 45°
    if(sinOut != NULL) {
     realCopy(const39_root2on2, sinOut);
    }
    if(cosOut != NULL) {
      realCopy(const39_root2on2, cosOut);
    }
    if(tanOut != NULL) {
      realSetOne(tanOut);
    }
  }
  else { // angle < 90
    if(angularMode != amRadian && realCompareGreaterThan(angle, angle45)) {        // angle > 45°
      realSubtract(angle90, angle, angle, realContext); // 90° - angle  --> angle
      *swap = !(*swap);
    }
    const bool_t angle30 = (angularMode == amDegree && realCompareEqual(angle, const_30));   // exactly 30°: its sine is 1/2
    if(angularMode != amRadian) {
      convertAngleFromTo(angle, angularMode, amRadian, realContext);
    }
    if(savedContextDigits <= 51) {
      realContext->digits = 51;
    }
    #if defined(OPTION_XFN_1000)
      if(savedContextDigits >= 1071) {
        C47_WP34S_SinCosTanTaylor_temp1071(angle, *swap, (*swap) ? cosOut : sinOut, (*swap) ? sinOut : cosOut, tanOut, realContext); // angle in radian
      }
      else
    #endif // OPTION_XFN_1000
    {
      C47_WP34S_SinCosTanTaylor_temp75(angle, *swap, (*swap) ? cosOut : sinOut, (*swap) ? sinOut : cosOut, tanOut, realContext); // angle in radian
    }
    real_t *sinOfAngle = (*swap) ? cosOut : sinOut;
    if(angle30 && sinOfAngle != NULL) {
      realCopy(const_1on2, sinOfAngle);
    }
  }

  realContext->digits = savedContextDigits;

  if(sinOut != NULL) {
    if(*sinNeg) {
      realSetNegativeSign(sinOut);
      if(tanOut != NULL) {
        realSetNegativeSign(tanOut);
      }
    }
    if(realIsZero(sinOut)) {
      realSetPositiveSign(sinOut);
      if(tanOut != NULL) {
        realSetPositiveSign(tanOut);
      }
    }
    realPlus(sinOut, sinOut, realContext);
  }

  if(cosOut != NULL) {
    if(*cosNeg) {
      realSetNegativeSign(cosOut);
      if(tanOut != NULL) {
        realChangeSign(tanOut);
      }
    }
    if(realIsZero(cosOut)) {
      realSetPositiveSign(cosOut);
    }
    realPlus(cosOut, cosOut, realContext);
  }

  if(tanOut != NULL && realIsZero(cosOut)) {
    realSetPositiveSign(tanOut);
    realPlus(tanOut, tanOut, realContext);
  }
}


// Called directly 75 digits max, by by agm, sin, sinc, cos, tan, multiple elliptic functions, exp (complex), fib, gd, tanh, WP34S_Zeta
//
// Have to be careful here to ensure that every function we call can handle the increased size of the numbers we're using.
static void C47_WP34S_Cvt2RadSinCosTan_75_helper(const real_t *an, angularMode_t angularMode, real_t *sinOut, real_t *cosOut, real_t *tanOut, realContext_t *realContext) {
  bool_t sinNeg = false, cosNeg = false, swap = false;
  real_t angle;
  realContext_t c = *realContext;

  if(realIsNaN(an)) {
    if(sinOut != NULL) {
      realSetNaN(sinOut);
    }
    if(cosOut != NULL) {
      realSetNaN(cosOut);
    }
    if(tanOut != NULL) {
      realSetNaN(tanOut);
    }
   return;
  }

  realCopy(an, &angle);

  int32_t savedContextDigits = c.digits;

  c.digits = 75;

  doWP34S_SinCosTanTaylor(&angle, &sinNeg, &cosNeg, &swap, sinOut, cosOut, tanOut, angularMode, savedContextDigits, &c);
}





static void doTaylorIterations(const real_t *a, real_t* angle, real_t* a2, real_t* t, real_t* j, real_t* z, real_t* sin, real_t* cos, real_t *sinOut, real_t *cosOut, real_t* epsilonOrCompare, const bool_t doEpsilon, const int epsilonDigits, realContext_t *realContext) {
  char tmpEpsilon[16];
  bool_t endSin = (sinOut == NULL), endCos = (cosOut == NULL);
  int i;

  if(doEpsilon) {
    sprintf(tmpEpsilon, "1E-%d", epsilonDigits);
    stringToReal(tmpEpsilon, epsilonOrCompare, realContext);
  }
  realCopy(a, angle);
  realMultiply(angle, angle, a2, realContext);
  realSetOne(j);
  realSetOne(t);
  realSetOne(sin);
  realSetOne(cos);

  for(i=1; !(endSin && endCos) && i<TaylorIterationMax; i++) { // it goes up to 31 max in the test suite
    realAdd(j, const_1, j, realContext);
    realDivide(a2, j, z, realContext);
    realMultiply(t, z, t, realContext);
    realChangeSign(t);
    int tExp = realGetExponent(t);

    if(!endCos) {
      realCopy(cos, z);
      realAdd(cos, t, cos, realContext);
      if(doEpsilon) {
        realCopyAbs(t, z);
      }
      else {
        realCompare(cos, z, epsilonOrCompare, realContext);
      }
      endCos = (!doEpsilon && realIsZero(epsilonOrCompare)) || (doEpsilon && realCompareLessThan(z, epsilonOrCompare));
    }

    realAdd(j, const_1, j, realContext);
    realDivide(t, j, t, realContext);
    tExp = max(tExp, realGetExponent(t));

    if(!endSin) {
      realCopy(sin, z);
      realAdd(sin, t, sin, realContext);
      if(doEpsilon) {
        realCopyAbs(t, z);
      }
      else {
        realCompare(sin, z, epsilonOrCompare, realContext);
      }
      endSin = (!doEpsilon && realIsZero(epsilonOrCompare)) || (doEpsilon && realCompareLessThan(z, epsilonOrCompare));
    }

    if(explicitTaylorIterVisibilitySelection && checkHalfSec()) {
      char ss[100];
      sprintf(ss, "Taylor Iter: %d/%d; Dig: %d/", i, TaylorIterationMax, -(int16_t)tExp);
      ss[40] = 0; //Hard limit to screen display
      #if defined(DEBUGTAYLOR)
        printf("%s%d\n", ss, epsilonDigits);
      #endif //DEBUGTAYLOR
      if(progressHalfSecUpdate_Integer(timed, ss, epsilonDigits, halfSec_clearZ, halfSec_clearT, halfSec_disp)) { //timed
      }
    }
    #if !defined(PC_BUILD)
      if(exitKeyWaiting()) {
        progressHalfSecUpdate_Integer(force+1, "Interrupted Iter:", i, halfSec_clearZ, halfSec_clearT, halfSec_disp);
        displayCalcErrorMessage(ERROR_SOLVER_ABORT, REGISTER_T);
        break;
      }
    #endif //!PC_BUILD

    #if defined(DEBUGTAYLOR)
      if(i > 1 && i % 1 == 0) { //left mod for printing interleaved status
        realToString(sin, tmpString);
        tmpString[80]=0;
        printf("Taylor progress: n=%3d, sin=%s", i, tmpString);
        realToString(cos, tmpString);
        tmpString[80]=0;
        printf(" cos=%s\n", tmpString);
      }
    #endif //DEBUGTAYLOR
  }

  if(realIsZero(cos)) {
    realSetPositiveSign(cos);
  }
  if(realIsZero(sin)) {
    realSetPositiveSign(sin);
  }
  realMultiply(sin, angle, sin, realContext);
  explicitTaylorIterVisibilitySelection = false;
}


// used by normal TRIG, used from externally from bessel.c
// Calculate sin, cos by Taylor series and tan by division
void C47_WP34S_SinCosTanTaylor_temp75(const real_t *a, bool_t swap, real_t *sinOut, real_t *cosOut, real_t *tanOut, realContext_t *realContext) { // a in radian
  bool_t doEpsilon = false;
  int   epsilonDigits;
  // The eight working reals come from the heap, not the frame: eight decNumbers at 60 bytes is 480 of this function's 672 byte frame, and it sits on the integrand path
  // of a plotted integral. The 1071 digit twin below is untouched.
  REAL_T_ALLOC(angle,            75);
  REAL_T_ALLOC(a2,               75);
  REAL_T_ALLOC(t,                75);
  REAL_T_ALLOC(j,                75);
  REAL_T_ALLOC(z,                75);
  REAL_T_ALLOC(sin,              75);
  REAL_T_ALLOC(cos,              75);
  REAL_T_ALLOC(epsilonOrCompare, 75);
  if(angle == NULL || a2 == NULL || t == NULL || j == NULL || z == NULL || sin == NULL || cos == NULL || epsilonOrCompare == NULL) {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    goto freeWork;
  }

  int32_t savedContextDigits = realContext->digits;

  if(realContext->digits > 51) {
    realContext->digits = 75;
    epsilonDigits = 72;           //only applicable if doEpsilon is true, as well as in the iteration display
    doEpsilon = true;
  }
  else {
    realContext->digits = 51;
    epsilonDigits = 39;           //only applicable if doEpsilon is true, as well as in the iteration display
    doEpsilon = false;            //stay compaitble with the old Taylor
  }

  doTaylorIterations(a, angle, a2, t, j, z, sin, cos, sinOut, cosOut, epsilonOrCompare, doEpsilon, epsilonDigits, realContext);

  realContext->digits = savedContextDigits;

  if(sinOut != NULL) {
    realPlus(sin, sinOut, realContext);
  }

  if(cosOut != NULL) {
    realPlus(cos, cosOut, realContext);
  }

  if(tanOut != NULL) {
    if(sinOut == NULL || cosOut == NULL) {
      realSetNaN(tanOut);
    }
    else {
      if(swap) {
        realDivide(cos, sin, tanOut, realContext);
      }
      else {
        realDivide(sin, cos, tanOut, realContext);
      }
    }
  }

freeWork:
  REAL_T_FREE(angle,            75);
  REAL_T_FREE(a2,               75);
  REAL_T_FREE(t,                75);
  REAL_T_FREE(j,                75);
  REAL_T_FREE(z,                75);
  REAL_T_FREE(sin,              75);
  REAL_T_FREE(cos,              75);
  REAL_T_FREE(epsilonOrCompare, 75);
}


#if defined(OPTION_XFN_1000)
//used only by XFN 1071
static void C47_WP34S_Cvt2RadSinCosTan_1071_helper(const real_t *an, angularMode_t angularMode, real_t *sinOut, real_t *cosOut, real_t *tanOut, realContext_t *realContext) {
  bool_t sinNeg = false, cosNeg = false, swap = false;
  // One 1071 digit decNumber at 724 bytes, of this function's 752 byte frame. Freed on both exits through freeWork.
  REAL_T_ALLOC(angle, 1071);
  if(angle == NULL) {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    goto freeWork;
  }

  if(realIsNaN(an)) {
    if(sinOut != NULL) {
     realSetNaN(sinOut);
    }
    if(cosOut != NULL) {
      realSetNaN(cosOut);
    }
    if(tanOut != NULL) {
      realSetNaN(tanOut);
    }
   goto freeWork;
  }

  realCopy(an, angle);

  int32_t savedContextDigits = realContext->digits;

  doWP34S_SinCosTanTaylor(angle, &sinNeg, &cosNeg, &swap, sinOut, cosOut, tanOut, angularMode, savedContextDigits, realContext);

freeWork:
  REAL_T_FREE(angle, 1071);
  }
#endif // OPTION_XFN_1000


void C47_WP34S_Cvt2RadSinCosTan(const real_t *an, angularMode_t angularMode, real_t *sinOut, real_t *cosOut, real_t *tanOut, realContext_t *realContext) {
  #if defined(OPTION_XFN_1000)
    if(realContext->digits >= 1071) {
      C47_WP34S_Cvt2RadSinCosTan_1071_helper(an, angularMode, sinOut, cosOut, tanOut, realContext);
    }
    else
  #endif // OPTION_XFN_1000
  {
    C47_WP34S_Cvt2RadSinCosTan_75_helper(an, angularMode, sinOut, cosOut, tanOut, realContext);
  }
}


#if defined(OPTION_XFN_1000)
//Used by normal C47 TRIG as well as XFN
// Calculate sin, cos by Taylor series and tan by division, for 1071 contexts
void C47_WP34S_SinCosTanTaylor_temp1071(const real_t *a, bool_t swap, real_t *sinOut, real_t *cosOut, real_t *tanOut, realContext_t *realContext) { // a in radian

  // The eight working reals come from the heap, not the frame: eight 1071 digit decNumbers at 724 bytes is 5792 of this function's 5832 byte frame, on an arm build with
  // OPTION_XFN_1000 forced on. That frame is the stack reason XFN 1000 is kept off the DM42, defines.h:36.
  REAL_T_ALLOC(angle,            1071);
  REAL_T_ALLOC(a2,               1071);
  REAL_T_ALLOC(t,                1071);
  REAL_T_ALLOC(j,                1071);
  REAL_T_ALLOC(z,                1071);
  REAL_T_ALLOC(sin,              1071);
  REAL_T_ALLOC(cos,              1071);
  REAL_T_ALLOC(epsilonOrCompare, 1071);
  if(angle == NULL || a2 == NULL || t == NULL || j == NULL || z == NULL || sin == NULL || cos == NULL || epsilonOrCompare == NULL) {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    goto freeWork;
  }

  doTaylorIterations(a, angle, a2, t, j, z, sin, cos, sinOut, cosOut, epsilonOrCompare, true /*doEpsilon*/, 1040, realContext);

  if(sinOut != NULL) {
    realPlus(sin, sinOut, realContext);
  }
  if(cosOut != NULL) {
    realPlus(cos, cosOut, realContext);
  }
  if(tanOut != NULL) {
    if(sinOut == NULL || cosOut == NULL) {
      realSetNaN(tanOut);
    }
    else {
      if(swap) {
        realDivide(cos, sin, tanOut, realContext);
      }
      else {
        realDivide(sin, cos, tanOut, realContext);
      }
    }
  }

freeWork:
  REAL_T_FREE(angle,            1071);
  REAL_T_FREE(a2,               1071);
  REAL_T_FREE(t,                1071);
  REAL_T_FREE(j,                1071);
  REAL_T_FREE(z,                1071);
  REAL_T_FREE(sin,              1071);
  REAL_T_FREE(cos,              1071);
  REAL_T_FREE(epsilonOrCompare, 1071);
}
#endif // OPTION_XFN_1000


void C47_WP34S_SinCosTanTaylor(const real_t *a, bool_t swap, real_t *sinOut, real_t *cosOut, real_t *tanOut, realContext_t *realContext) { // a in radian
  #if defined(OPTION_XFN_1000)
    if(realContext->digits >= 1071) {
      C47_WP34S_SinCosTanTaylor_temp1071(a, swap, sinOut, cosOut, tanOut, realContext); // a in radian
    }
    else
  #endif // OPTION_XFN_1000
  {
    C47_WP34S_SinCosTanTaylor_temp75(a, swap, sinOut, cosOut, tanOut, realContext); // a in radian
  }
}


static bool_t doAtan(real_t *a, real_t *angle, real_t *a2, real_t *t, real_t *j, real_t *z, const real_t *x, real_t *b, real_t *epsilon, real_t *last, const bool_t doEpsilon, int epsilonDigits, int* doubles, int* invert, int* neg, realContext_t *realContext){
  bool_t conditionToKeepIterating = false;
  char tmpEpsilon[16]; //-- for epsilon string conversion
  //-- use epsilon convergence instead of exact equality
  if(doEpsilon) {
    sprintf(tmpEpsilon, "1E-%d", epsilonDigits);
    stringToReal(tmpEpsilon, epsilon, realContext);
    //-- create const_1on10 equivalent for 1071 precision, temporary use of z - up to for loop below
    //uInt32ToReal(10, z);
    //realDivide(const_1, z, z, realContext);
  }


  *neg = realIsNegative(x);

  if(realIsNaN(x)) {
    realSetNaN(angle);
    return false;
  }

  realCopy(x, a);

  // arrange for a >= 0
  if(*neg) {
    realChangeSign(a);
  }

  // reduce range to 0 <= a < 1, using atan(x) = pi/2 - atan(1/x)
  *invert = realCompareGreaterThan(a, const_1);
  if(*invert) {
    realDivide(const_1, a, a, realContext);
  }

  // Range reduce to small enough limit to use taylor series using:
  //  tan(x/2) = tan(x)/(1+sqrt(1+tan(x)²))
  for(int n=0; n<TaylorIterationMax; n++) {
    if(!doEpsilon && realCompareLessEqual(a, const_1on10)) {
      break;
    }
    else if(doEpsilon && realCompareLessEqual(a, const_1on10)){//  z)) {
      break;
    }

    (*doubles)++;
    // a = a/(1+sqrt(1+a²)) -- at most 3 iterations.
    realMultiply(a, a, b, realContext);
    realAdd(b, const_1, b, realContext);
    realSquareRoot(b, b, realContext);
    realAdd(b, const_1, b, realContext);
    realDivide(a, b, a, realContext);
  }

  // Now Taylor series
  // atan(x) = x(1-x²/3+x⁴/5-x⁶/7...)
  // We calculate pairs of terms and stop when the estimate doesn't change
  uInt32ToReal(3, angle);
  uInt32ToReal(5, j);
  realMultiply(a, a, a2, realContext); // a²
  realCopy(a2, t);
  realDivide(t, angle, angle, realContext); // s = 1-t/3 -- first two terms
  realSubtract(const_1, angle, angle, realContext);

  int i = 0;
  do { // Loop until there is no digits changed
    realCopy(angle, last);

    realMultiply(t, a2, t, realContext);
    realDivide(t, j, z, realContext);
    realAdd(angle, z, angle, realContext);

    realAdd(j, const_2, j, realContext);

    realMultiply(t, a2, t, realContext);
    realDivide(t, j, z, realContext);
    realSubtract(angle, z, angle, realContext);

    realAdd(j, const_2, j, realContext);

    if(doEpsilon) {
      //-- use epsilon convergence test instead of exact equality
      realSubtract(angle, last, b, realContext);
      realCopyAbs(b, b);
      realSubtract(b, epsilon, b, realContext);
      conditionToKeepIterating = realIsPositive(b);
    }
     else {
      realSubtract(angle, last, b, realContext);
      realPlus(b, b, realContext);
      conditionToKeepIterating = !realIsZero(b);
    }


    if(explicitTaylorIterVisibilitySelection && checkHalfSec()) {
      char ss[100];
      sprintf(ss, "Taylor Iter: %d/%d; Dig: %d/", i, TaylorIterationMax, -(int16_t)realGetExponent(b));
      ss[40] = 0; //Hard limit to screen display
      #if defined(DEBUGTAYLOR)
        printf("%s%d\n", ss, epsilonDigits);
      #endif //DEBUGTAYLOR
      if(progressHalfSecUpdate_Integer(timed, ss, epsilonDigits, halfSec_clearZ, halfSec_clearT, halfSec_disp)) { //timed
      }
    }
    #if !defined(PC_BUILD)
      if(exitKeyWaiting()) {
        progressHalfSecUpdate_Integer(force+1, "Interrupted Iter:", i, halfSec_clearZ, halfSec_clearT, halfSec_disp);
        displayCalcErrorMessage(ERROR_SOLVER_ABORT, REGISTER_T);
        break;
      }
    #endif //!PC_BUILD


    #if defined(DEBUGTAYLOR)
      if(i > 1 && i % 1 == 0) { //left mod for printing interleaved status
        realToString(angle, tmpString);
        tmpString[80]=0;
        printf("Taylor progress Atan: n=%3d, angle=%s", i, tmpString);
        realToString(b , tmpString);
        tmpString[80]=0;
        printf(" diff=%s\n", tmpString);
      }
    #endif //DEBUGTAYLOR
    i++;

  } while(conditionToKeepIterating && i < TaylorIterationMax);

  realMultiply(angle, a, angle, realContext);

  while(*doubles) {
    realAdd(angle, angle, angle, realContext);
    (*doubles)--;
  }
  if(*invert) {
    realSubtract(((realContext->digits) > 51) ? const1071_piOn2 : const39_piOn2, angle, angle, realContext);
  }
  if(*neg) {
    realChangeSign(angle);
  }
  return true;
}


// ---------------------------------------------------------------------------
// Generic cache helpers for 1-input and 2-input trig wrappers
//
// Each cache is a single most-recent-call slot. The stored result is treated as
// a pure function of the key: the input value(s), the effective compute
// precision, and the rounding mode. Nothing else in realContext (emax/emin/
// clamp) changes an angle result, whose magnitude is always O(1), and the
// angular-mode conversion happens in the caller AFTER the cache, so the angular
// mode is deliberately not part of the key.
//
// The caches are global single slots. cache*_call therefore writes the key and
// the result together AFTER computing, never a bare key before: a reentrant
// call (a display refresh computing another angle mid-computation, say) can only
// leave its OWN complete, correct pair, which the outer call then overwrites
// with its own - there is no window where a stale key and a fresh result coexist.
// This ordering is the reason the wrappers are safe without any in-progress flag
// or scheduling assumption; do not move cache_prepare before compute. Build with
// CACHE_VERIFY to recompute on every hit and abort on any mismatch, which turns
// the "key is complete" assumption into a checked one.
// ---------------------------------------------------------------------------
typedef struct { bool_t valid; int32_t digits; int32_t round; real_t x, result;    } cache1_t;
typedef struct { bool_t valid; int32_t digits; int32_t round; real_t y, x, result; } cache2_t;

typedef void (*trig1Compute_t)(const real_t *x, real_t *out, realContext_t *ctx);
typedef void (*trig2Compute_t)(const real_t *y, const real_t *x, real_t *out, realContext_t *ctx);

static void cache_commit(bool_t *valid, real_t *cacheResult, const real_t *result) {
  if(!realIsSpecial(result)) { realCopy(result, cacheResult); *valid = true; }
}

// zero sign is part of the key
static bool_t cacheKeyEqual(const real_t *a, const real_t *b) {
  return realCompareIdentical(a, b);
}

#if defined(CACHE_VERIFY)
  static void cacheVerifyFail(const char *which) {
    // stderr is unbuffered: abort() would drop a buffered stdout line.
    fprintf(stderr, "CACHE_VERIFY_MISMATCH in %s: a cache hit disagreed with a fresh recompute\n", which);
    fflush(stderr);
    abort();
  }
#endif // CACHE_VERIFY

// digits is the effective compute precision, not the request
static bool_t cache1_check(cache1_t *c, const real_t *x, real_t *result, int32_t digits, realContext_t *ctx) {
  if(c->valid && digits == c->digits && (int32_t)ctx->round == c->round && cacheKeyEqual(&c->x, x)) {
    realCopy(&c->result, result);
    return true;
  }
  return false;
}
static void cache1_prepare(cache1_t *c, const real_t *x, int32_t digits, realContext_t *ctx) {
  realCopy(x, &c->x);
  c->digits = digits;
  c->round  = (int32_t)ctx->round;
  c->valid  = false;
}

static bool_t cache2_check(cache2_t *c, const real_t *y, const real_t *x, real_t *result, int32_t digits, realContext_t *ctx) {
  if(c->valid && digits == c->digits && (int32_t)ctx->round == c->round && cacheKeyEqual(&c->y, y) && cacheKeyEqual(&c->x, x)) {
    realCopy(&c->result, result);
    return true;
  }
  return false;
}
static void cache2_prepare(cache2_t *c, const real_t *y, const real_t *x, int32_t digits, realContext_t *ctx) {
  realCopy(y, &c->y);
  realCopy(x, &c->x);
  c->digits = digits;
  c->round  = (int32_t)ctx->round;
  c->valid  = false;
}

// One cached 1-input trig call. Copies the input first (so `compute` is safe even
// when the caller aliases in==out), returns a hit, otherwise computes and stores
// key+result together (see the ordering note above). Under CACHE_VERIFY every hit
// is recomputed and compared, so an incomplete key aborts loudly instead of
// returning a stale value.
static void cache1_call(cache1_t *c, trig1Compute_t compute, const char *name, const real_t *x, real_t *out, int32_t effDigits, realContext_t *ctx) {
  real_t localX;
  realCopy(x, &localX);
  if(cache1_check(c, &localX, out, effDigits, ctx)) {
    #if defined(CACHE_DEBUG)
      printf("   %s: quick return for repeated value\n", name);
    #endif // CACHE_DEBUG
    #if defined(CACHE_VERIFY)
      real_t verify;
      compute(&localX, &verify, ctx);
      if(!cacheKeyEqual(&verify, out)) { cacheVerifyFail(name); }
    #endif // CACHE_VERIFY
    return;
  }
  #if defined(CACHE_DEBUG)
    printf("%s: long process calc\n", name);
  #endif // CACHE_DEBUG
  compute(&localX, out, ctx);                 // compute first: any nested call leaves its own complete pair
  cache1_prepare(c, &localX, effDigits, ctx); // then stamp this call's key
  cache_commit(&c->valid, &c->result, out);   // and result, together
  #if !defined(CACHE_DEBUG) && !defined(CACHE_VERIFY)
    (void)name;
  #endif
}

static void cache2_call(cache2_t *c, trig2Compute_t compute, const char *name, const real_t *y, const real_t *x, real_t *out, int32_t effDigits, realContext_t *ctx) {
  real_t localY, localX;
  realCopy(y, &localY);
  realCopy(x, &localX);
  if(cache2_check(c, &localY, &localX, out, effDigits, ctx)) {
    #if defined(CACHE_DEBUG)
      printf("   %s: quick return for repeated value\n", name);
    #endif // CACHE_DEBUG
    #if defined(CACHE_VERIFY)
      real_t verify;
      compute(&localY, &localX, &verify, ctx);
      if(!cacheKeyEqual(&verify, out)) { cacheVerifyFail(name); }
    #endif // CACHE_VERIFY
    return;
  }
  #if defined(CACHE_DEBUG)
    printf("%s: long process calc\n", name);
  #endif // CACHE_DEBUG
  compute(&localY, &localX, out, ctx);                 // compute first (see cache1_call)
  cache2_prepare(c, &localY, &localX, effDigits, ctx); // then stamp key
  cache_commit(&c->valid, &c->result, out);            // and result, together
  #if !defined(CACHE_DEBUG) && !defined(CACHE_VERIFY)
    (void)name;
  #endif
}


// The precision atan actually computes at. Requests above 39 go to the Taylor path at 75.
// At or below 39 the table path honours the request plus a couple of guard digits: about a
// dozen chained operations cost roughly one digit of accumulated rounding, measured as a
// steady 0.8 to 1.0 digits at every precision from 3 up to 39, so computing two beyond the
// request returns what the caller asked for with a digit to spare. The cap at 39 is why
// atan has always returned 38 correct digits to a 39 digit caller, table path or not.
//
// No lower floor: the loss is proportional, not a cliff, so a caller asking for little
// gets little quickly. Graph plotting narrows ctxtReal39 to significantDigits + 3, as low
// as 4, and pays for 6 rather than for 39.
//
// Both compute functions and the cache wrapper must agree on this, or the cache could hand
// a low precision result to a high precision request.
#define ATAN_GUARD_DIGITS 2
static int32_t atanEffectiveDigits(int32_t requestedDigits) {
  if(requestedDigits > 39) {
    return 75;
  }
  requestedDigits += ATAN_GUARD_DIGITS;
  return (requestedDigits > 39) ? 39 : requestedDigits;
}


// atan(j/10) for j = 1..9; the ends are const_0 and const39_piOn4. The step must divide a
// power of ten so that j/step is exact at every working precision: a tenth is, 1/12 is not,
// and its rounding would break the identity the reduction rests on.
#define ATAN_TABLE_STEP 10
static const real_t * const atanTable[ATAN_TABLE_STEP + 1] = {
  const_0,            const39_atan1on10,  const39_atan2on10,  const39_atan3on10,
  const39_atan4on10,  const39_atan5on10,  const39_atan6on10,  const39_atan7on10,
  const39_atan8on10,  const39_atan9on10,  const39_piOn4
};

// P(v) with v = u*u, magnitudes only (the loop below negates v instead of tracking signs).
// Eight of the thirteen were constrained to the Taylor coefficients +-1/(2k+1) and only
// atanP08..12 left free for the fit. Eight constraints still land at 2.9e-41 against a
// 1e-40 budget, so they cost no extra term; a ninth would not fit. Holding the first at
// exactly 1 is what makes atan(x) come out as exactly x for tiny x rather than merely
// rounding to it.
#define ATAN_COEFFICIENTS 13
static const real_t * const atanPoly[ATAN_COEFFICIENTS] = {
  const_1,          const39_1on3,     const_1on5,       const39_1on7,
  const39_1on9,     const39_1on11,    const39_1on13,    const39_1on15,
  const39_atanP08,  const39_atanP09,  const39_atanP10,  const39_atanP11,
  const39_atanP12
};


/********************************************//**
 * \brief atan for requests at or below 39 digits
 *
 * Reduces through the addition formula against a tenth spaced table
 *
 *     a = |x|, and a > 1 is inverted by atan(a) = pi/2 - atan(1/a)
 *     j = round(10a),  c = j/10,  u = (a - c)/(1 + a*c)   so |u| <= 1/20
 *     atan(a) = atanTable[j] + u * P(u*u)
 *
 * which costs one division where the Taylor path spent three square roots, and then
 * finishes with a fixed length polynomial instead of a convergence loop. Horner starts
 * below the top term once u is small enough that the leading terms cannot reach the
 * requested precision, so small arguments stay cheap.
 *
 * Truncation error is 2.9e-41 as the coefficients are stored, an order of magnitude under
 * the 5e-40 the 39 digit table constants contribute anyway, so the polynomial is not what
 * limits the answer. At 39 digits this returns about 38 correct digits, a little better
 * than the Taylor path it replaces.
 ***********************************************/
static void WP34S_Atan_table_compute(const real_t *x, real_t *angle, realContext_t *realContext) {
  REAL_T_ALLOC(a, 39);   // also the accumulator further down
  REAL_T_ALLOC(u, 39);
  REAL_T_ALLOC(v, 39);
  REAL_T_ALLOC(c, 39);
  bool_t neg, invert;
  int32_t j, k, top, exponent, savedContextDigits;

  // The four 39 digit heap reals are smaller than the declared real_t. GCC's bounds checker gives warning; example fix from graph.c
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Warray-bounds"

  if(a == NULL || u == NULL || v == NULL || c == NULL) {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    goto freeWork;
  }

  if(realIsNaN(x)) {
    realSetNaN(angle);
    goto freeWork;
  }

  savedContextDigits  = realContext->digits;
  realContext->digits = atanEffectiveDigits(savedContextDigits);

  neg = realIsNegative(x);                        // kept: the result is negated again at the end
  realCopyAbs(x, a);

  invert = realCompareGreaterThan(a, const_1);
  if(invert) {
    realDivide(const_1, a, a, realContext);   // 1/inf is 0, which lands on j = 0 and gives pi/2
  }

  // j = round(10a), always in [0, 10] and used directly as the index into atanTable. c is
  // borrowed for the scaled copy here and holds the table centre from the next block on.
  // Multiplying by ten in a decimal representation is nothing but a shift of the exponent,
  // so the scaling is free and exact; the rounding is left to realToIntegralValue. a is |x|
  // with anything above 1 inverted, and rounding 1/a can reach 1 but never pass it, so 10a
  // stays inside [0, 10] and no clamp is needed. realToInt32C47 cannot break that either:
  // every one of its failure paths returns 0.
  realCopy(a, c);
  c->exponent++;
  realToIntegralValue(c, c, DEC_ROUND_HALF_UP, realContext);
  j = realToInt32C47(c, NULL);

  // c = j/10, exact for the same reason and for j = 0 too, since a zero stays a zero
  int32ToReal(j, c);
  c->exponent--;
  realMultiply(a, c, v, realContext);
  realAdd(v, const_1, v, realContext);
  realSubtract(a, c, u, realContext);
  realDivide(u, v, u, realContext);

  realMultiply(u, u, v, realContext);        // v = u*u

  // v^k drops below the last digit being computed once k * |log10 v| passes that many
  // digits, so every coefficient above that index is dead weight. The + 6 keeps the cut a
  // few digits clear of the rounding noise, at the cost of at most one extra term.
  top = ATAN_COEFFICIENTS - 1;
  if(realIsZero(v)) {
    top = 0;
  }
  else {
    exponent = -realGetExponent(v);
    if(exponent > 0) {
      int32_t needed = (realContext->digits + 6 + exponent - 1) / exponent;
      if(needed < top) {
        top = needed;
      }
    }
  }

  // The series alternates: c0 - c1*v + c2*v^2 - ... = c0 - v*(c1 - v*(c2 - ...)). Negating v
  // turns every Horner step into the same acc*(-v) + c operation, so atanPoly can hold plain
  // magnitudes and neither the loop nor its seed needs to track the sign.
  realChangeSign(v);
  realCopy(atanPoly[top], a);
  for(k = top - 1; k >= 0; k--) {
    realMultiply(a, v, a, realContext);
    realAdd(a, atanPoly[k], a, realContext);
  }
  realMultiply(a, u, a, realContext);
  realAdd(a, atanTable[j], angle, realContext);

  if(invert) {
    realSubtract(const39_piOn2, angle, angle, realContext);
  }
  if(neg) {
    realChangeSign(angle);
  }

  realContext->digits = savedContextDigits;
  #pragma GCC diagnostic pop

freeWork:
  REAL_T_FREE(a, 39);
  REAL_T_FREE(u, 39);
  REAL_T_FREE(v, 39);
  REAL_T_FREE(c, 39);
}


// The Taylor series with sqrt halving, now reached only for requests above 39 digits.
static void WP34S_Atan_75_compute(const real_t *x, real_t *angle, realContext_t *realContext) {
  bool_t doEpsilon = false;
  // The eight working reals come from the heap, not the frame: 480 of this function's 656 bytes, on the integrand path of a plotted integral. The 1071 digit twin below
  // keeps its stack buffers untouched.
  REAL_T_ALLOC(a,       75);
  REAL_T_ALLOC(b,       75);
  REAL_T_ALLOC(a2,      75);
  REAL_T_ALLOC(t,       75);
  REAL_T_ALLOC(j,       75);
  REAL_T_ALLOC(z,       75);
  REAL_T_ALLOC(last,    75);
  REAL_T_ALLOC(epsilon, 75);   //-- added epsilon for convergence;
  int doubles = 0;
  int invert;
  int neg;
  int32_t savedContextDigits = realContext->digits;
  int epsilonDigits;

  if(realContext->digits > 39) {
    realContext->digits = 75;
    epsilonDigits = 72;           //only applicable if doEpsilon is true, as well as in the iteration display
    doEpsilon = true;
  }
  else {
    realContext->digits = 39;
    epsilonDigits = 39;           //only applicable if doEpsilon is true, as well as in the iteration display
    doEpsilon = false;            //stay compaitble with the old Taylor
  }

  if(a == NULL || b == NULL || a2 == NULL || t == NULL || j == NULL || z == NULL || last == NULL || epsilon == NULL) {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    realContext->digits = savedContextDigits;
    goto freeWork;
  }

  if(!doAtan( a, angle, a2, t, j, z, x, b, epsilon, last,
              doEpsilon, epsilonDigits,
              &doubles, &invert, &neg,
              realContext)) {
    realContext->digits = savedContextDigits;
    goto freeWork; //NaN
  }
  realContext->digits = savedContextDigits;

freeWork:
  REAL_T_FREE(a,       75);
  REAL_T_FREE(b,       75);
  REAL_T_FREE(a2,      75);
  REAL_T_FREE(t,       75);
  REAL_T_FREE(j,       75);
  REAL_T_FREE(z,       75);
  REAL_T_FREE(last,    75);
  REAL_T_FREE(epsilon, 75);
}


// Cached wrapper for WP34S_Atan. Returns the previous result when the input,
// effective precision and rounding mode all match; see cache1_call.
static cache1_t atanCache;
static void WP34S_Atan_75_helper(const real_t *x, real_t *angle, realContext_t *realContext) {
  #if defined(CACHE_DEBUG)
    print_caller("WP34S_Atan");             // traced here, not in cache1_call: print_caller reports its own caller
  #endif // CACHE_DEBUG
  int32_t effDigits = atanEffectiveDigits(realContext->digits); // precision actually computed at, not the request
  cache1_call(&atanCache,
              (effDigits > 39) ? WP34S_Atan_75_compute : WP34S_Atan_table_compute,
              "WP34S_Atan", x, angle, effDigits, realContext);
}


#if defined(OPTION_XFN_1000)
static void C47do_WP34S_Atan_1071_helper(const real_t *x, real_t *angle, realContext_t *realContext) {
  // The eight working reals come from the heap, not the frame: eight 1071 digit decNumbers at 724 bytes is 5792 of this function's 5864 byte frame, the largest frame in
  // the build. Freed on both exits through freeWork.
  REAL_T_ALLOC(a,       1071);
  REAL_T_ALLOC(b,       1071);
  REAL_T_ALLOC(a2,      1071);
  REAL_T_ALLOC(t,       1071);
  REAL_T_ALLOC(j,       1071);
  REAL_T_ALLOC(z,       1071);
  REAL_T_ALLOC(last,    1071);
  REAL_T_ALLOC(epsilon, 1071);
  if(a == NULL || b == NULL || a2 == NULL || t == NULL || j == NULL || z == NULL || last == NULL || epsilon == NULL) {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    goto freeWork;
  }
  int doubles = 0;
  int invert;
  int neg;
  if(!doAtan( a, angle, a2, t, j, z, x, b, epsilon, last, true, 1040, &doubles, &invert, &neg, realContext)) {
    goto freeWork; //NaN
  }

freeWork:
  REAL_T_FREE(a,       1071);
  REAL_T_FREE(b,       1071);
  REAL_T_FREE(a2,      1071);
  REAL_T_FREE(t,       1071);
  REAL_T_FREE(j,       1071);
  REAL_T_FREE(z,       1071);
  REAL_T_FREE(last,    1071);
  REAL_T_FREE(epsilon, 1071);
}
#endif // OPTION_XFN_1000


void C47_WP34S_Atan(const real_t *x, real_t *angle, realContext_t *realContext) {
  #if defined(OPTION_XFN_1000)
    if(realContext->digits >= 1071) {
      C47do_WP34S_Atan_1071_helper(x, angle, realContext);
    }
    else
  #endif // OPTION_XFN_1000
  {
    WP34S_Atan_75_helper(x, angle, realContext);
  }
}


#define _pi(d)     (d > 51 ? (d > 75 ? const1071_pi     : const75_pi)     : const39_pi)
#define _piOn2(d)  (d > 51 ? (d > 75 ? const1071_piOn2  : const75_piOn2)  : const39_piOn2)
#define _piOn4(d)  (d > 51 ? (d > 75 ? const1071_piOn4  : const75_piOn4)  : const39_piOn4)
#define _3piOn4(d) (d > 51 ? (d > 75 ? const1071_3piOn4 : const75_3piOn4) : const39_3piOn4)

static bool_t doAtan2(const real_t *y, const real_t *x, real_t *atan, real_t *r, real_t *t, realContext_t *realContext) {
  const bool_t xNeg = realIsNegative(x);
  const bool_t yNeg = realIsNegative(y);

  if(realIsNaN(x) || realIsNaN(y)) {
    realSetNaN(atan);
    return false;
  }

  if(realCompareEqual(y, const_0)) {
    if(yNeg) {
      if(realCompareEqual(x, const_0)) {
        if(xNeg) {
          realMinus( _pi(realContext->digits), atan, realContext);
        }
        else {
          realCopy(y, atan);
        }
      }
      else if(xNeg) {
        realMinus( _pi(realContext->digits), atan, realContext);
      }
      else {
        realCopy(y, atan);
      }
    }
    else {
      if(realCompareEqual(x, const_0)) {
        if(xNeg) {
          realCopy( _pi(realContext->digits), atan);
        }
        else {
          realSetZero(atan);
        }
      }
      else if(xNeg) {
        realCopy( _pi(realContext->digits), atan);
      }
      else {
        realSetZero(atan);
      }
    }
    return true;
  }

  if(realCompareEqual(x, const_0)) {
    realCopy( _piOn2(realContext->digits), atan);
    if(yNeg) {
      realSetNegativeSign(atan);
    }
    return true;
  }

  if(realIsInfinite(x)) {
    if(xNeg) {
      if(realIsInfinite(y)) {
        realCopy( _3piOn4(realContext->digits), atan);
        if(yNeg) {
          realSetNegativeSign(atan);
        }
      }
      else {
        realCopy( _pi(realContext->digits), atan);
        if(yNeg) {
          realSetNegativeSign(atan);
        }
      }
    }
    else {
      if(realIsInfinite(y)) {
        realCopy( _piOn4(realContext->digits), atan);
        if(yNeg) {
          realSetNegativeSign(atan);
        }
      }
      else {
        realSetZero(atan);
        if(yNeg) {
          realSetNegativeSign(atan);
        }
      }
    }
    return true;
  }

  if(realIsInfinite(y)) {
    realCopy( _piOn2(realContext->digits), atan);
    if(yNeg) {
      realSetNegativeSign(atan);
    }
    return true;
  }

  realDivide(y, x, t, realContext);
  C47_WP34S_Atan(t, r, realContext);
  if(xNeg) {
    realCopy( _pi(realContext->digits), t);
    if(yNeg) {
     realSetNegativeSign(t);
    }
  }
  else {
    realSetZero(t);
  }

  realAdd(r, t, atan, realContext);
  if(realCompareEqual(atan, const_0) && yNeg) {
    realSetNegativeSign(atan);
  }
  return true;
}


static void WP34S_Atan2_75_compute(const real_t *y, const real_t *x, real_t *atan, realContext_t *realContext) {
  real_t r, t;
  int32_t savedContextDigits = realContext->digits;
  if(realContext->digits > 75) {
    realContext->digits = 75;
  }
  if(!doAtan2(y, x, atan, &r, &t, realContext)) {
    realContext->digits = savedContextDigits;
    return; //NaN
  }
  realContext->digits = savedContextDigits;
}


// Cached wrapper for WP34S_Atan2. Returns the previous result when both inputs,
// the effective precision and rounding mode match; see cache2_call.
static cache2_t atan2Cache;
static void WP34S_Atan2_75_helper(const real_t *y, const real_t *x, real_t *atan, realContext_t *realContext) {
  #if defined(CACHE_DEBUG)
    print_caller("WP34S_Atan2");            // traced here, not in cache2_call: print_caller reports its own caller
  #endif // CACHE_DEBUG
  int32_t effDigits = (realContext->digits > 75) ? 75 : realContext->digits; // precision WP34S_Atan2_75_compute computes at
  cache2_call(&atan2Cache, WP34S_Atan2_75_compute, "WP34S_Atan2", y, x, atan, effDigits, realContext);
}


#if defined(OPTION_XFN_1000)
static void C47do_WP34S_Atan2_1071_helper(const real_t *y, const real_t *x, real_t *atan, realContext_t *realContext) {
  // Two 1071 digit decNumbers at 724 bytes is 1448 of this function's 1464 byte frame. Freed on both exits through freeWork.
  REAL_T_ALLOC(r, 1071);
  REAL_T_ALLOC(t, 1071);
  if(r == NULL || t == NULL) {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    goto freeWork;
  }
  if(!doAtan2(y, x, atan, r, t, realContext)) {
    goto freeWork; //NaN
  }

freeWork:
  REAL_T_FREE(r, 1071);
  REAL_T_FREE(t, 1071);
}
#endif // OPTION_XFN_1000


void C47_WP34S_Atan2(const real_t *y, const real_t *x, real_t *atan, realContext_t *realContext) {
  #if defined(OPTION_XFN_1000)
    if(realContext->digits >= 1071) {
      C47do_WP34S_Atan2_1071_helper(y, x, atan, realContext);
    }
    else
  #endif // OPTION_XFN_1000
  {
    WP34S_Atan2_75_helper(y, x, atan, realContext);
  }
}


static bool_t doAsin(const real_t *x, real_t *angle, real_t *abx, real_t *z, realContext_t *realContext) {
  if(realIsNaN(x)) {
    realSetNaN(angle);
    return false;
  }
  realCopyAbs(x, abx);
  if(realCompareGreaterThan(abx, const_1)) {
    realSetNaN(angle);
    return false;
  }
  // angle = atan2(x, sqrt((1-|x|)*(1+|x|)))
  realSubtract(const_1, abx, z, realContext);
  realAdd(const_1, abx, abx, realContext);
  realMultiply(z, abx, z, realContext);
  realSquareRoot(z, z, realContext);
  C47_WP34S_Atan2(x, z, angle, realContext);
  return true;
}


static void WP34S_Asin_75_compute(const real_t *x, real_t *angle, realContext_t *realContext) {
  real_t abx, z;
  int32_t savedContextDigits = realContext->digits;
  if(realContext->digits > 75) {
    realContext->digits = 75;
  }
  if(!doAsin(x, angle, &abx, &z, realContext)) {
    realContext->digits = savedContextDigits;
    return; //NaN
  }
  realContext->digits = savedContextDigits;
}


// Cached wrapper for WP34S_Asin. See cache1_call.
static cache1_t asinCache;
static void WP34S_Asin_75_helper(const real_t *x, real_t *angle, realContext_t *realContext) {
  #if defined(CACHE_DEBUG)
    print_caller("WP34S_Asin");             // traced here, not in cache1_call: print_caller reports its own caller
  #endif // CACHE_DEBUG
  int32_t effDigits = (realContext->digits > 75) ? 75 : realContext->digits; // precision WP34S_Asin_75_compute computes at
  cache1_call(&asinCache, WP34S_Asin_75_compute, "WP34S_Asin", x, angle, effDigits, realContext);
}


#if defined(OPTION_XFN_1000)
static void C47do_WP34S_Asin_1071_helper(const real_t *x, real_t *angle, realContext_t *realContext) {
  // Two 1071 digit decNumbers at 724 bytes is 1448 of this function's 1464 byte frame. Freed on both exits through freeWork.
  REAL_T_ALLOC(abx, 1071);
  REAL_T_ALLOC(z,   1071);
  if(abx == NULL || z == NULL) {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    goto freeWork;
  }
  if(!doAsin(x, angle, abx, z, realContext)) {
    goto freeWork; //NaN
  }

freeWork:
  REAL_T_FREE(abx, 1071);
  REAL_T_FREE(z,   1071);
}
#endif // OPTION_XFN_1000


void C47_WP34S_Asin(const real_t *x, real_t *angle, realContext_t *realContext) {
  #if defined(OPTION_XFN_1000)
    if(realContext->digits >= 1071) {
      C47do_WP34S_Asin_1071_helper(x, angle, realContext);
    }
    else
  #endif // OPTION_XFN_1000
  {
    WP34S_Asin_75_helper(x, angle, realContext);
  }
}



static bool_t doAcos(const real_t *x, real_t *angle, real_t *abx, real_t *z, realContext_t *realContext) {
  if(realIsNaN(x)) {
    realSetNaN(angle);
    return false;
  }
  realCopyAbs(x, abx);
  if(realCompareGreaterThan(abx, const_1)) {
    realSetNaN(angle);
    return false;
  }
  // angle = atan2(sqrt((1-|x|)*(1+|x|)), x)
  realSubtract(const_1, abx, z, realContext);
  realAdd(const_1, abx, abx, realContext);
  realMultiply(z, abx, z, realContext);
  realSquareRoot(z, z, realContext);
  C47_WP34S_Atan2(z, x, angle, realContext);
  return true;
}


static void WP34S_Acos_75_compute(const real_t *x, real_t *angle, realContext_t *realContext) {
  real_t abx, z;
  int32_t savedContextDigits = realContext->digits;
  if(realContext->digits > 75) {
    realContext->digits = 75;
  }
  if(!doAcos(x, angle, &abx, &z, realContext)) {
    realContext->digits = savedContextDigits;
    return; //NaN
  }
  realContext->digits = savedContextDigits;
}


// Cached wrapper for WP34S_Acos. See cache1_call.
static cache1_t acosCache;
static void WP34S_Acos_75_helper(const real_t *x, real_t *angle, realContext_t *realContext) {
  #if defined(CACHE_DEBUG)
    print_caller("WP34S_Acos");             // traced here, not in cache1_call: print_caller reports its own caller
  #endif // CACHE_DEBUG
  int32_t effDigits = (realContext->digits > 75) ? 75 : realContext->digits; // precision WP34S_Acos_75_compute computes at
  cache1_call(&acosCache, WP34S_Acos_75_compute, "WP34S_Acos", x, angle, effDigits, realContext);
}


#if defined(OPTION_XFN_1000)
static void C47do_WP34S_Acos_1071_helper(const real_t *x, real_t *angle, realContext_t *realContext) {
  // Two 1071 digit decNumbers at 724 bytes is 1448 of this function's 1464 byte frame. Freed on both exits through freeWork.
  REAL_T_ALLOC(abx, 1071);
  REAL_T_ALLOC(z,   1071);
  if(abx == NULL || z == NULL) {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    goto freeWork;
  }
  if(!doAcos(x, angle, abx, z, realContext)) {
    goto freeWork; //NaN
  }

freeWork:
  REAL_T_FREE(abx, 1071);
  REAL_T_FREE(z,   1071);
}
#endif // OPTION_XFN_1000


void C47_WP34S_Acos(const real_t *x, real_t *angle, realContext_t *realContext) {
  #if defined(OPTION_XFN_1000)
    if(realContext->digits >= 1071) {
      C47do_WP34S_Acos_1071_helper(x, angle, realContext);
    }
    else
  #endif // OPTION_XFN_1000
  {
    WP34S_Acos_75_helper(x, angle, realContext);
  }
}



/**
 * \brief Evaluates the real Lanczos coefficient sum.
 *
 * \param[in] x Argument
 * \param[in] offset Denominator offset
 * \param[out] res Coefficient sum
 * \param[in] realContext Calculation context
 */
void WP34S_LanczosSum(const real_t *x, const real_t *offset, real_t *res, realContext_t *realContext) {
  real_t denominator, term;
  int32_t index;

  realSetZero(res);
  realAdd(x, offset, &denominator, realContext);
  for(index=28; index>=0; index--) {
    realDivide((real_t *)(((uint8_t *)const75_gammaC01) + index*REAL_SIZE_IN_BYTES(75)), &denominator, &term, realContext);
    realAdd(res, &term, res, realContext);
    realSubtract(&denominator, const_1, &denominator, realContext);
  }
  realAdd(res, const75_gammaC00, res, realContext);
}


static void WP34S_Calc_Gamma_LnGamma_Lanczos(const real_t *xin, real_t *res, bool_t calculateLnGamma, realContext_t *realContext) {
  real_t r, s, t, u, v, x;
  int32_t savedContextDigits;

  savedContextDigits = realContext->digits;
  if(realContext->digits < 75) {
    realContext->digits = 75;
  }

  realSubtract(xin, const_1, &x, realContext);
  WP34S_LanczosSum(&x, const_29, &s, realContext);
  WP34S_Ln(&s, &s, realContext);

  //  r = z + g + 0.5;
  realAdd(&x, const_gammaR, &r, realContext); // const_gammaR is g + 0.5

  //  r = log(R[0][0]) + (z+0.5) * log(r) - r;
  WP34S_Ln(&r, &u, realContext);
  realAdd(&x, const_1on2, &t, realContext);
  realMultiply(&u, &t, &v, realContext);

  realSubtract(&v, &r, &u, realContext);

  if(calculateLnGamma) {
    realAdd(&u, &s, &x, realContext);
  }
  else {
    realAdd(&u, &s, &x, realContext);
    realExp(&x, &x, realContext);
  }

  realContext->digits = savedContextDigits;

  realPlus(&x, res, realContext);
}


// common code for the [GAMMA] and LN[GAMMA]
static void WP34S_Gamma_LnGamma(const real_t *xin, const bool_t calculateLnGamma, real_t *res, realContext_t *realContext) {
  real_t x, t;
  bool_t reflect = false;

  // Check for special cases
  if(realIsSpecial(xin)) {
    if(realIsInfinite(xin) && realIsPositive(xin)) {
      realSetPlusInfinity(res);
      return;
    }

    realSetNaN(res);
    return;
  }

  // Handle x approximately zero case
  if(realCompareAbsLessThan(xin, const_1e_24)) {
    if(realIsZero(xin)) {
      realSetNaN(res);
      return;
    }
    realDivide(const_1, xin, &x, realContext);
    realSubtract(&x, const39_egamma, res, realContext);
    if(calculateLnGamma) {
      WP34S_Ln(res, res, realContext);
    }
    return;
  }

  // Correct our argument and begin the inversion if it is negative
  if(realCompareLessEqual(xin, const_0)) {
    reflect = true;
    realSubtract(const_1, xin, &t, realContext); // t = 1 - xin
    if(realIsAnInteger(&t)) {
      realSetNaN(res);
      return;
    }
  }
  else {
    // Provide a fast path evaluation for positive integer arguments that aren't too large
    // The threshold for overflow is 205! (i.e. 204! is within range and 205! isn't).
    if(realIsAnInteger(xin) && realCompareLessEqual(xin, const_205)) {
      realSubtract(xin, const_1, &x, realContext); // x = xin - 1
      realSetOne(res);
      while(realCompareGreaterEqual(&x, const_2)) {
        realMultiply(res, &x, res, realContext);
        realSubtract(&x, const_1, &x, realContext);
      }
      if(calculateLnGamma) {
        WP34S_Ln(res, res, realContext);
      }
      return;
    }
    realCopy(xin, &t); // t = xin
  }

  WP34S_Calc_Gamma_LnGamma_Lanczos(&t, res, calculateLnGamma, realContext); // t=1-xin or t=xin and res=gamma(xin) or res=lngamma(xin) or res=gamma(1-xin) or res=lngamma(1-xin)

  if(reflect) {
    // Reduce to [-0.5, 0.5] before multiplying by pi, preserving the full distance from a pole without forming an angle close to an integer multiple of pi first.
    WP34S_Mod(xin, const_2, &t, realContext);
    if(realCompareGreaterThan(&t, const_1on2)) {
      if(realCompareGreaterThan(&t, const_3on2)) {
        realSubtract(&t, const_2, &t, realContext);
      }
      else {
        realSubtract(const_1, &t, &t, realContext);
      }
    }
    else if(realIsNegative(&t)) {
      realMinus(&t, &x, realContext);
      if(realCompareGreaterThan(&x, const_3on2)) {
        realAdd(&t, const_2, &t, realContext);
      }
      else if(realCompareGreaterThan(&x, const_1on2)) {
        realSubtract(const__1, &t, &t, realContext);
      }
    }
    realMultiply(&t, const75_pi, &t, realContext);                            // t = xin·pi
    C47_WP34S_SinCosTanTaylor_temp75(&t, false, &x, NULL, NULL, realContext); // x = sin(xin·pi)

    if(calculateLnGamma) {
      realDivide(const75_pi, &x, &t, realContext);                            // t = pi / sin(pi·xin)
      WP34S_Ln(&t, &t, realContext);                                          // t = ln(pi / sin(pi·xin))
      realSubtract(&t, res, res, realContext);                                // res = ln(pi / sin(pi·xin)) - lngamma(1-xin)
    }
    else {
      realMultiply(&x, res, &t, realContext);                                 // t = sin(pi·xin) × gamma(1-xin)
      realDivide(const75_pi, &t, res, realContext);                           // res = pi / (sin(pi·xin)·gamma(1-xin))
    }
  }
}


void WP34S_Factorial(const real_t *xin, real_t *res, realContext_t *realContext) {
  real_t x;

  realAdd(xin, const_1, &x, realContext);
  WP34S_Gamma_LnGamma(&x, false, res, realContext);
}


void WP34S_Gamma(const real_t *xin, real_t *res, realContext_t *realContext) {
  real_t x;

  realCopy(xin, &x);
  WP34S_Gamma_LnGamma(&x, false, res, realContext);
}


void WP34S_LnGamma(const real_t *xin, real_t *res, realContext_t *realContext) {
  real_t x;

  realCopy(xin, &x);
  WP34S_Gamma_LnGamma(&x, true, res, realContext);
}


/* Natural logarithm.
 *
 * Take advantage of the fact that we store our numbers in the form: m * 10^e
 * so log(m * 10^e) = log(m) + e * log(10)
 * do this so that m is always in the range 0.1 <= m < 2.  However if the number
 * is already in the range 0.5 .. 1.5, this step is skipped.
 *
 * Then use the fact that ln(x²) = 2 * ln(x) to range reduce the mantissa
 * into 1/sqrt(2) <= m < 2.
 *
 * Finally, apply the series expansion:
 *   ln(x) = 2(a+a³/3+a⁵/5+...) where a=(x-1)/(x+1)
 * which converges quickly for an argument near unity.
 */
void WP34S_Ln(const real_t *xin, real_t *res, realContext_t *realContext) {
  real_t z, t, f, n, m, i, v, w, e;
  int32_t expon;

  if(realIsSpecial(xin)) {
    if(realIsNaN(xin) || realIsNegative(xin)) {
      realSetNaN(res);
      return;
    }

    realSetPlusInfinity(res);
    return;
  }

  if(realCompareLessEqual(xin, const_0)) {
    if(realIsNegative(xin)) {
      realSetNaN(res);
      return;
    }

    realSetMinusInfinity(res);
    return;
  }

  realCopy(xin, &z);
  realCopy(const_2, &f);
  realSubtract(xin, const_1, &t, realContext);
  realCopy(&t, &v);
  realSetPositiveSign(&v);
  if(realCompareGreaterThan(&v, const_1on2)) {
    expon = z.exponent + z.digits;
    z.exponent = -z.digits;
  }
  else {
    expon = 0;
  }

  /* The too high case never happens
  while(dn_le(const_2, &z)) {
    realMultiply(&f, const_2, &f, realContext);
    realSquareRoot(&z, &z, realContext);
  } */

  // Range reduce the value by repeated square roots.
  // Making the constant here larger will reduce the number of later
  // iterations at the expense of more square root operations.
  while(realCompareLessEqual(&z, const39_root2on2)) {
    realMultiply(&f, const_2, &f, realContext);
    realSquareRoot(&z, &z, realContext);
  }

  realAdd(&z, const_1, &t, realContext);
  realSubtract(&z, const_1, &v, realContext);
  realDivide(&v, &t, &n, realContext);
  realCopy(&n, &v);
  realMultiply(&v, &v, &m, realContext);
  realCopy(const_3, &i);

  int32ToReal(1 - realContext->digits, &t); // t is the exponent
  realPower10(&t, &z, realContext); // z is the max error

  for(;;) {
    realMultiply(&m, &n, &n, realContext);
    realDivide(&n, &i, &e, realContext);
    realAdd(&v, &e, &w, realContext);
    if(WP34S_RelativeError(&w, &v, &z, realContext)) {
      break;
    }
    realCopy(&w, &v);
    realAdd(&i, const_2, &i, realContext);
  }

  realMultiply(&f, &w, res, realContext);
  if(expon == 0) {
    return;
  }

  int32ToReal(expon, &e);
  realMultiply(&e, realContext->digits > 39 ? const75_ln10 : const39_ln10, &w, realContext);
  realAdd(res, &w, res, realContext);
}


void WP34S_Log(const real_t *xin, const real_t *base, real_t *res, realContext_t *realContext) {
  real_t y;

  if(realIsSpecial(xin)) {
    if(realIsNaN(xin) || realIsNegative(xin)) {
      realSetNaN(res);
      return;
    }

    realSetPlusInfinity(res);
    return;
  }

  WP34S_Ln(xin, &y, realContext);

  realDivide(&y, base, res, realContext);
}


/* never used
void WP34S_Log2(const real_t *xin, real_t *res, realContext_t *realContext) {
  WP34S_Log(xin, const39_ln2, res, realContext);
}
*/


void WP34S_Log10(const real_t *xin, real_t *res, realContext_t *realContext) {
  WP34S_Log(xin, const39_ln10, res, realContext);
}


void WP34S_Logxy(const real_t *yin, const real_t *xin, real_t *res, realContext_t *realContext) {
  real_t lx;

  WP34S_Ln(xin, &lx, realContext);
  WP34S_Log(yin, &lx, res, realContext);
}


bool_t WP34S_RelativeError(const real_t *x, const real_t *y, const real_t *tol, realContext_t *realContext) {
  real_t a;

  if(realIsZero(x)) {
    realCopyAbs(y, &a);
    return realCompareLessThan(&a, tol);
  }

  realSubtract(x, y, &a, realContext);
  realDivide(&a, x, &a, realContext);
  realSetPositiveSign(&a);
  return realCompareLessThan(&a, tol);
}


bool_t WP34S_AbsoluteError(const real_t *x, const real_t *y, const real_t *tol, realContext_t *realContext) {
  real_t a;
  realSubtract(x, y, &a, realContext);
  return realCompareAbsLessThan(&a, tol);
}


bool_t WP34S_ComplexRelativeError(const cplx_t *x, const cplx_t *y, const real_t *tol, realContext_t *realContext) {
  cplx_t a;

  if(realIsZero(&x->Real) && realIsZero(&x->Imag)) {
    complexMagnitude(y, &a.Real, realContext);
  }
  else {
    realSubtract(&x->Real, &y->Real, &a.Real, realContext);
    realSubtract(&x->Imag, &y->Imag, &a.Imag, realContext);
    divComplexComplex(&a, x, &a, realContext);
    complexMagnitude(&a, &a.Real, realContext);
  }
  return realCompareLessThan(&a.Real, tol);
}


bool_t WP34S_ComplexAbsError(const cplx_t *x, const cplx_t *y, const real_t *tol, realContext_t *realContext) {
  real_t r;
  cplx_t a;

  realSubtract(&x->Real, &y->Real, &a.Real, realContext), realSubtract(&x->Imag, &y->Imag, &a.Imag, realContext);
  complexMagnitude(&a, &r, realContext);
  return realCompareLessThan(&r, tol);
}


/* Hyperbolic functions.
 * We start with a utility routine that calculates sinh and cosh.
 * We do the sihn as (e^x - 1) (e^x + 1) / (2 e^x) for numerical stability
 * reasons if the value of x is smallish.
 */
void WP34S_SinhCosh(const real_t *x, real_t *sinhOut, real_t *coshOut, realContext_t *realContext) {
  real_t t, u, v;

  if(realIsNaN(x)) {
    if(sinhOut != NULL) {
      realSetNaN(sinhOut);
    }
    if(coshOut != NULL) {
      realSetNaN(coshOut);
    }
    return;
  }

  if(sinhOut != NULL) {
    if(realCompareAbsLessThan(x, const_1on2)) {
      WP34S_ExpM1(x, &u, realContext);                         // u = e^x - 1
      realMultiply(&u, const_1on2, &t, realContext);           // t = (e^x - 1) / 2

      realAdd(&u, const_1, &u, realContext);                   // u = e^x
      realDivide(&t, &u, &v, realContext);                     // v = (e^x - 1) / 2e^x

      realAdd(&u, const_1, &u, realContext);                   // u = e^x + 1
      realMultiply(&u, &v, sinhOut, realContext);              // sinhOut = (e^x - 1)(e^x + 1) / 2e^x
    }
    else {
      realExp(x, &u, realContext);                             // u = e^x
      realDivide(const_1, &u, &v, realContext);                // v = e^-x
      realSubtract(&u, &v, sinhOut, realContext);              // sinhOut = (e^x + e^-x)
      realMultiply(sinhOut, const_1on2, sinhOut, realContext); // sinhOut = (e^x + e^-x)/2
    }
  }
  if(coshOut != NULL) {
   realExp(x, &u, realContext);                                // u = e^x
   realDivide(const_1, &u, &v, realContext);                   // v = e^-x
   realAdd(&u, &v, coshOut, realContext);                      // coshOut = (e^x + e^-x)
   realMultiply(coshOut, const_1on2, coshOut, realContext);    // coshOut = (e^x + e^-x)/2
  }
}


void WP34S_Tanh(const real_t *x, real_t *res, realContext_t *realContext) {
  bool_t negative = realIsNegative(x);

  if(realIsNaN(x)) {
    realSetNaN(res);
  }
  else {
    real_t a, b;

    realCopyAbs(x, &a);
    realAdd(&a, &a, &a, realContext);
    realSetNegativeSign(&a);               // a = -2|x|
    WP34S_ExpM1(&a, &b, realContext);      // b = exp(-2|x|) - 1
    realChangeSign(&b);                    // b = 1 - exp(-2|x|)
    realSubtract(const_2, &b, &a, realContext); // a = 1 + exp(-2|x|)
    realDivide(&b, &a, res, realContext);
    if(negative) {
      realSetNegativeSign(res);
    }
  }
}


/********************************************//**
 * \brief x + x³/d or x - x³/d of an odd function for a nonzero |x| below 1E-18, kept on its side of x
 *
 * Where x³/d is below the last digit of the context, the sum equals x; one step from x keeps it above or below |x|, so rounding it to 34 digits gives
 * the exact value rounded once. The series runs on |x| and the result takes the sign of x.
 *
 * \param[in]  x           const real_t*  a nonzero real, |x| below 1E-18
 * \param[in]  divisor     const real_t*  d
 * \param[in]  above       bool_t         true for x + x³/d, false for x - x³/d
 * \param[out] res         real_t*        the result
 * \param[in]  realContext realContext_t* the working context
 * \return void
 ***********************************************/
void WP34S_TinyOddSeries(const real_t *x, const real_t *divisor, bool_t above, real_t *res, realContext_t *realContext) {
  real_t a, t, s;
  const bool_t negative = realIsNegative(x);

  realCopyAbs(x, &a);
  realMultiply(&a, &a, &t, realContext);
  realMultiply(&t, &a, &t, realContext);
  realDivide(&t, divisor, &t, realContext);
  if(above) {
    realAdd(&a, &t, &s, realContext);
  }
  else {
    realSubtract(&a, &t, &s, realContext);
  }
  if(realCompareEqual(&s, &a)) {
    realNextToward(&a, above ? const_plusInfinity : const_0, &s, realContext);
  }
  realCopy(&s, res);
  if(negative) {
    realSetNegativeSign(res);
  }
}


void WP34S_ArcSinh(const real_t *x, real_t *res, realContext_t *realContext) {
  real_t a, s;
  const bool_t negative = realIsNegative(x);

  if(realIsSpecial(x)) {
    realCopy(x, res);
    return;
  }
  if(realIsZero(x)) {
    if(negative) {
      realDivide(const__1, const_plusInfinity, res, realContext);
    } else {
      realSetZero(res);
    }
    return;
  }

  realCopyAbs(x, &a);
  if(realGetExponent(&a) < -18) {         // arsinh x = x - x³/6 to 72 digits, below |x| by less than half a unit at 34 digits
    WP34S_TinyOddSeries(&a, const_6, false, res, realContext);
    if(negative) {
      realSetNegativeSign(res);
    }
    return;
  }
  realFMA(&a, &a, const_1, &s, realContext);
  realSquareRoot(&s, &s, realContext);
  realAdd(&s, const_1, &s, realContext);
  realDivide(&a, &s, &s, realContext);
  realAdd(&s, const_1, &s, realContext);
  realMultiply(&a, &s, &s, realContext);
  WP34S_Ln1P(&s, res, realContext);
  if(negative) {
    realSetNegativeSign(res);
  }
}



/* never used
void WP34S_ArcCosh(const real_t *xin, real_t *res, realContext_t *realContext) {
  real_t x, z;

  realCopy(xin, &x);
  realMultiply(&x, &x, res, realContext);      // res = x²
  realSubtract(res, const_1, &z, realContext); // z = x² - 1
  realSquareRoot(&z, res, realContext);        // res = sqrt(x²-1)
  realAdd(res, &x, &z, realContext);           // z = x + sqrt(x²-1)
  WP34S_Ln(&z, res, realContext);              // res = ln(x + sqrt(x²-1))
}
*/


void WP34S_ArcTanh(const real_t *x, real_t *res, realContext_t *realContext) {
  real_t y, z;
  const bool_t negative = realIsNegative(x);

  if(realIsNaN(x)) {
    realSetNaN(res);
    return;
  }
  if(realIsZero(x)) {
    if(negative) {
      realDivide(const__1, const_plusInfinity, res, realContext);
    } else {
      realSetZero(res);
    }
    return;
  }

  WP34S_Ln1P(x, &y, realContext);
  realMinus(x, &z, realContext);
  WP34S_Ln1P(&z, &z, realContext);
  realSubtract(&y, &z, &y, realContext);
  realMultiply(&y, const_1on2, res, realContext);
}


/* ln(1+x) */
void WP34S_Ln1P(const real_t *x, real_t *res, realContext_t *realContext) {
  real_t u, v, w;

  if(realIsSpecial(x) || realIsZero(x)) {
    realCopy(x, res);
  }
  else {
    realAdd(x, const_1, &u, realContext);       // u = x+1
    realSubtract(&u, const_1, &v, realContext); // v = x
    if(realIsZero(&v)) {
      realCopy(x, res);
    }
    else {
      realDivide(x, &v, &w, realContext);
      WP34S_Ln(&u, &v, realContext);
      realMultiply(&v, &w, res, realContext);
    }
  }
}


/* exp(x)-1 */
void WP34S_ExpM1(const real_t *x, real_t *res, realContext_t *realContext) {
  real_t u, v, w;

  if(!realExpLimitCheck(x, res, const__1)) {
    return;
  }

  realExp(x, &u, realContext);
  realSubtract(&u, const_1, &v, realContext);
  if(realIsZero(&v)) { // |x| is very little
    realCopy(x, res);
  }
  else if(realCompareEqual(&v, const__1)) {
    realSetNegativeOne(res);
  }
  else if(realCompareAbsLessThan(x, const_1on10)) {
    realMultiply(&v, x, &w, realContext);
    WP34S_Ln(&u, &v, realContext);
    realDivide(&w, &v, res, realContext);
  }
  else {
    realCopy(&v, res);
  }
}


static void WP34S_CalcComplexLnGamma_Lanczos(const cplx_t *z, cplx_t *res, realContext_t *realContext) {
  cplx_t r, s, t, u, v;
  int k;
  int32_t savedContextDigits;

  savedContextDigits = realContext->digits;
  if(realContext->digits < 75) {
    realContext->digits = 75;
  }

  realSetZero(&u.Real);
  realSetZero(&u.Imag);
  realAdd(&z->Real, const_29, &t.Real, realContext);
  realCopy(&z->Imag, &t.Imag);
  for(k=28; k>=0; k--) {
    divRealComplex((real_t *)(((uint8_t *)const75_gammaC01) + k*REAL_SIZE_IN_BYTES(75)), &t, &s, realContext);
    realSubtract(&t.Real, const_1, &t.Real, realContext);
    realAdd(&u.Real, &s.Real, &u.Real, realContext);
    realAdd(&u.Imag, &s.Imag, &u.Imag, realContext);
  }
  realAdd(&u.Real, const75_gammaC00, &t.Real, realContext);
  realCopy(&u.Imag, &t.Imag);
  lnComplex(&t, &s, realContext);                          // (s1, s2)

  realAdd(&z->Real, const_gammaR, &r.Real, realContext);
  realCopy(&z->Imag, &r.Imag);
  lnComplex(&r, &u, realContext);

  realAdd(&z->Real, const_1on2, &t.Real, realContext);
  realCopy(&z->Imag, &t.Imag);
  mulComplexComplex(&t, &u, &v, realContext);

  realSubtract(&v.Real, &r.Real, &u.Real, realContext);
  realSubtract(&v.Imag, &z->Imag, &u.Imag, realContext);

  realContext->digits = savedContextDigits;

  realAdd(&u.Real, &s.Real, &res->Real, realContext);
  realAdd(&u.Imag, &s.Imag, &res->Imag, realContext);
}


static void WP34S_ComplexGammaLnGamma(const cplx_t *z, const bool_t calculateLnGamma, cplx_t *res, realContext_t *realContext) {
  cplx_t sinPiZ, t, u, x;
  bool_t reflect = false;
  bool_t negateSin = false;

  // Check for special cases
  if(realIsSpecial(&z->Real) || realIsSpecial(&z->Imag)) {
    if(realIsNaN(&z->Real) || realIsNaN(&z->Imag)) {
      realSetNaN(&res->Real);
      realSetNaN(&res->Imag);
    }
    else {
      if(realIsInfinite(&z->Real)) {
        if(realIsInfinite(&z->Imag) || realIsNegative(&z->Real)) {
          realSetNaN(&res->Real);
          realSetNaN(&res->Imag);
        }
        else {
          realSetPlusInfinity(&res->Real);
          realSetZero(&res->Imag);
        }
      }
      else {
        realSetZero(&res->Real);
        realSetZero(&res->Imag);
      }
    }
    return;
  }

  // Correct our argument and begin the inversion if it is negative
  if(realIsNegative(&z->Real)) {
    reflect = true;
    realSubtract(const_1, &z->Real, &t.Real, realContext);
    if(realIsZero(&z->Imag) && realIsAnInteger(&t.Real)) {
      realSetNaN(&res->Real);
      realSetNaN(&res->Imag);
      return;
    }
    realSubtract(&t.Real, const_1, &x.Real, realContext);
    realMinus(&z->Imag, &x.Imag, realContext);
  }
  else {
    realSubtract(&z->Real, const_1, &x.Real, realContext);
    realCopy(&z->Imag, &x.Imag);
  }

  // Sum the series
  WP34S_CalcComplexLnGamma_Lanczos(&x, res, realContext);
  if(!calculateLnGamma) {
    expComplex(res, res, realContext);
  }

  // Finally invert if we started with a negative argument
  if(reflect) {
    // Reduce the real component before multiplying by pi and record the parity of the removed integer, which determines the sign of both sine components.
    WP34S_Mod(&z->Real, const_2, &t.Real, realContext);
    if(realCompareGreaterThan(&t.Real, const_1on2)) {
      if(realCompareGreaterThan(&t.Real, const_3on2)) {
        realSubtract(&t.Real, const_2, &t.Real, realContext);
      }
      else {
        realSubtract(&t.Real, const_1, &t.Real, realContext);
        negateSin = true;
      }
    }
    else if(realIsNegative(&t.Real)) {
      realMinus(&t.Real, &u.Real, realContext);
      if(realCompareGreaterThan(&u.Real, const_3on2)) {
        realAdd(&t.Real, const_2, &t.Real, realContext);
      }
      else if(realCompareGreaterThan(&u.Real, const_1on2)) {
        realAdd(&t.Real, const_1, &t.Real, realContext);
        negateSin = true;
      }
    }
    realMultiply(&t.Real, const39_pi, &t.Real, realContext);
    realMultiply(&z->Imag, const39_pi, &t.Imag, realContext);
    sinComplex(&t, &sinPiZ, realContext);
    if(negateSin) {
      realMinus(&sinPiZ.Real, &sinPiZ.Real, realContext);
      realMinus(&sinPiZ.Imag, &sinPiZ.Imag, realContext);
    }
    if(!calculateLnGamma) {
      mulComplexComplex(&sinPiZ, res, &u, realContext);
      divRealComplex(const39_pi, &u, res, realContext);
    }
    else {
      divRealComplex(const39_pi, &sinPiZ, &u, realContext);
      lnComplex(&u, &t, realContext);
      realSubtract(&t.Real, &res->Real, &res->Real, realContext);
      realSubtract(&t.Imag, &res->Imag, &res->Imag, realContext);
    }
  }
}


void WP34S_ComplexGamma(const cplx_t *zin, cplx_t *res, realContext_t *realContext) {
  cplx_t z;

  realCopy(&zin->Real, &z.Real);
  realCopy(&zin->Imag, &z.Imag);
  WP34S_ComplexGammaLnGamma(&z, false, res, realContext);
}


void WP34S_ComplexLnGamma(const cplx_t *zin, cplx_t *res, realContext_t *realContext) {
  cplx_t z;

  realCopy(&zin->Real, &z.Real);
  realCopy(&zin->Imag, &z.Imag);
  WP34S_ComplexGammaLnGamma(&z, true, res, realContext);
}


static void doMod(const real_t *x, const real_t *y, real_t *res, realContext_t *realContext,
                  unsigned int digits, real_t *out) {
  realContext_t c = *realContext;

  c.digits = digits;
  realDivideRemainder(x, y, out, &c);
  realPlus(out, res, realContext);
}


/**
 * \brief Calculates the exact truncated remainder of two finite real34 values with 68-digit modular arithmetic.
 *
 * \param[in] x Dividend
 * \param[in] y Divisor
 * \param[out] res Remainder
 */
void WP34S_Real34Remainder(const real_t *x, const real_t *y, real_t *res) {
  REAL_T_PTR(a, 34);
  REAL_T_PTR(b, 34);
  REAL_T_PTR(comparison, 1);
  REAL_T_PTR(factor, 34);
  REAL_T_PTR(product, 68);
  REAL_T_PTR(remainder, 34);
  realContext_t c = ctxtReal75;
  int32_t exponentA, exponentB, shift;

  if(realIsSpecial(x) || realIsSpecial(y) || realIsZero(y)) {
    c.digits = 39;
    realDivideRemainder(x, y, res, &c);
    return;
  }

  realCopyAbs(x, a);
  realCopyAbs(y, b);
  realCompare(a, b, comparison, &c);
  if(realIsNegative(comparison)) {
    realCopy(x, res);
    return;
  }

  c.digits = 68;
  exponentA = realGetUnadjustedExponent(a);
  exponentB = realGetUnadjustedExponent(b);
  realSetUnadjustedExponent(a, 0);
  realSetUnadjustedExponent(b, 0);

  if(exponentA < exponentB) {
    realSetUnadjustedExponent(b, exponentB - exponentA);
    realDivideRemainder(a, b, remainder, &c);
    realAdjustExponent(remainder, exponentA);
  }
  else {
    shift = exponentA - exponentB;
    realDivideRemainder(a, b, remainder, &c);
    realDivideRemainder(const_10, b, factor, &c);

    while(shift != 0) {
      if((shift & 1) != 0) {
        realMultiply(remainder, factor, product, &c);
        realDivideRemainder(product, b, remainder, &c);
      }
      shift >>= 1;
      if(shift != 0) {
        realMultiply(factor, factor, product, &c);
        realDivideRemainder(product, b, factor, &c);
      }
    }
    realAdjustExponent(remainder, exponentB);
  }

  if(realIsNegative(x)) {
    realSetNegativeSign(remainder);
  }
  realCopy(remainder, res);
}


#if 0
// Original e900193 fixes 2025-08 Pauli
// When the memory allocation below is in place, 1E700 SIN (REAL), as well as 700 10^x SIN (LI) results in -NaN
void WP34S_Mod_Pauli(const real_t *x, const real_t *y, real_t *res, realContext_t *realContext) {
#if defined(DMCP_BUILD) && HARDWARE_MODEL == HWM_DM42
  unsigned int digits = 6147;
  const size_t blocks = TO_BLOCKS(REAL_SIZE_IN_BYTES(6147));
  real_t *tofree = allocC47Blocks(blocks);
  REAL_T_PTR(small, 2139); // Fallback size
  real_t *out;

  out = tofree == NULL ? small : tofree;
  doMod(x, y, res, realContext, digits, out);
  freeC47Blocks(out, blocks);
#else
  REAL_T_PTR(temp, 6147);

  doMod(x, y, res, realContext, 6147, temp);
#endif // DMCP_BUILD && HARDWARE_MODEL == HWM_DM42
}


// Original e900193 fixes 2025-08 Pauli
// When the memory allocation below is in place, 1E700 SIN (REAL), as well as 700 10^x SIN (LI) results in -NaN
void WP34S_BigMod_Pauli(const real_t *x, const real_t *y, real_t *res, realContext_t *realContext) {
#if defined(DMCP_BUILD) && HARDWARE_MODEL == HWM_DM42
  unsigned int digits = 12321;
  const size_t blocks = TO_BLOCKS(REAL_SIZE_IN_BYTES(12321));
  real_t *tofree = allocC47Blocks(blocks);
  REAL_T_PTR(small, 2139); // Fallback size
  real_t *out;

  out = tofree == NULL ? small : tofree;
  doMod(x, y, res, realContext, digits, out);
  freeC47Blocks(out, blocks);
#else
  REAL_T_PTR(temp, 12321);

  doMod(x, y, res, realContext, 12321, temp);
#endif // DMCP_BUILD && HARDWARE_MODEL == HWM_DM42
}
#endif // 0


// long integer handling: see longIntegerAngleReduction in registerValueConversions.c, case amRadian. A long integer angle never arrives here as an integer. There,
// longIntegerToString writes it base 10 into tmpString and decNumberFromString reads that into a 2139 digit real, which is the x this function reduces against
// const2139_2pi, and the result is rounded down to real75 in the same place. Angles over 1001 base 10 digits are refused before the conversion, and that context sits
// at 2139 digits because 6147 overran the stack.
// The reduction buffer comes from the heap, not the frame: 1436 bytes of this function's 1488 byte frame on the DM42, 8224 on every other build. Argument reduction runs
// inside the integrand of a plotted integral, so this frame sits under two engine frames.
void WP34S_Mod(const real_t *x, const real_t *y, real_t *res, realContext_t *realContext) {
#if defined(DMCP_BUILD) && HARDWARE_MODEL == HWM_DM42
  REAL_T_ALLOC(small, 2139); // Fallback size
  if(small == NULL) {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    realSetNaN(res);
  }
  else {
    doMod(x, y, res, realContext, 2139, small);
  }
  REAL_T_FREE(small, 2139);
#else
  REAL_T_ALLOC(temp, 12321);
  if(temp == NULL) {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    realSetNaN(res);
  }
  else {
    doMod(x, y, res, realContext, 6147, temp);
  }
  REAL_T_FREE(temp, 12321);
#endif // DMCP_BUILD && HARDWARE_MODEL == HWM_DM42
}


// Same buffer sizes as WP34S_Mod above, taken the same way. This is the one mod2Pi calls.
void WP34S_BigMod(const real_t *x, const real_t *y, real_t *res, realContext_t *realContext) {
#if defined(DMCP_BUILD) && HARDWARE_MODEL == HWM_DM42
  REAL_T_ALLOC(small, 2139); // Fallback size
  if(small == NULL) {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    realSetNaN(res);
  }
  else {
    doMod(x, y, res, realContext, 2139, small);
  }
  REAL_T_FREE(small, 2139);
#else
  REAL_T_ALLOC(temp, 12321);
  if(temp == NULL) {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    realSetNaN(res);
  }
  else {
    doMod(x, y, res, realContext, 12321, temp);                //printf("\n******  ****** NOT MATCHED 2pi !! ****** ******\n");
  }
  REAL_T_FREE(temp, 12321);
#endif // DMCP_BUILD && HARDWARE_MODEL == HWM_DM42
}


void mod2Pi(const real_t *x, real_t *res, realContext_t *realContext) {
#if defined(DMCP_BUILD) && HARDWARE_MODEL == HWM_DM42
  WP34S_BigMod(x, const2139_2pi, res, realContext);     // DM42 reduces with the 2139-digit 2pi; the 12321-digit 2pi NaNs on the DM42 decNumber at this 2139 context
#else
  WP34S_BigMod(x, const12321_2pi, res, realContext);
#endif // defined(DMCP_BUILD) && HARDWARE_MODEL == HWM_DM42
}

TO_QSPI static const real_t *const igammaTemmeC0[] = {
  const39_igammaC0_00, const39_igammaC0_01, const39_igammaC0_02,
  const39_igammaC0_03, const39_igammaC0_04, const39_igammaC0_05,
  const39_igammaC0_06, const39_igammaC0_07, const39_igammaC0_08,
  const39_igammaC0_09, const39_igammaC0_10, const39_igammaC0_11,
  const39_igammaC0_12, const39_igammaC0_13, const39_igammaC0_14,
  const39_igammaC0_15, const39_igammaC0_16, const39_igammaC0_17,
  const39_igammaC0_18, const39_igammaC0_19, const39_igammaC0_20,
  const39_igammaC0_21, const39_igammaC0_22, const39_igammaC0_23,
  const39_igammaC0_24, const39_igammaC0_25, const39_igammaC0_26,
  const39_igammaC0_27, const39_igammaC0_28, const39_igammaC0_29,
  const39_igammaC0_30
};

TO_QSPI static const real_t *const igammaTemmeC1[] = {
  const39_igammaC1_00, const39_igammaC1_01, const39_igammaC1_02,
  const39_igammaC1_03, const39_igammaC1_04, const39_igammaC1_05,
  const39_igammaC1_06, const39_igammaC1_07, const39_igammaC1_08,
  const39_igammaC1_09, const39_igammaC1_10, const39_igammaC1_11,
  const39_igammaC1_12, const39_igammaC1_13, const39_igammaC1_14,
  const39_igammaC1_15, const39_igammaC1_16, const39_igammaC1_17,
  const39_igammaC1_18, const39_igammaC1_19, const39_igammaC1_20,
  const39_igammaC1_21, const39_igammaC1_22, const39_igammaC1_23,
  const39_igammaC1_24, const39_igammaC1_25, const39_igammaC1_26,
  const39_igammaC1_27, const39_igammaC1_28
};

TO_QSPI static const real_t *const igammaTemmeC2[] = {
  const39_igammaC2_00, const39_igammaC2_01, const39_igammaC2_02,
  const39_igammaC2_03, const39_igammaC2_04, const39_igammaC2_05,
  const39_igammaC2_06, const39_igammaC2_07, const39_igammaC2_08,
  const39_igammaC2_09, const39_igammaC2_10, const39_igammaC2_11,
  const39_igammaC2_12, const39_igammaC2_13, const39_igammaC2_14,
  const39_igammaC2_15, const39_igammaC2_16, const39_igammaC2_17,
  const39_igammaC2_18, const39_igammaC2_19, const39_igammaC2_20,
  const39_igammaC2_21, const39_igammaC2_22, const39_igammaC2_23,
  const39_igammaC2_24, const39_igammaC2_25, const39_igammaC2_26
};

TO_QSPI static const real_t *const igammaTemmeC3[] = {
  const39_igammaC3_00, const39_igammaC3_01, const39_igammaC3_02,
  const39_igammaC3_03, const39_igammaC3_04, const39_igammaC3_05,
  const39_igammaC3_06, const39_igammaC3_07, const39_igammaC3_08,
  const39_igammaC3_09, const39_igammaC3_10, const39_igammaC3_11,
  const39_igammaC3_12, const39_igammaC3_13, const39_igammaC3_14,
  const39_igammaC3_15, const39_igammaC3_16, const39_igammaC3_17,
  const39_igammaC3_18, const39_igammaC3_19, const39_igammaC3_20,
  const39_igammaC3_21, const39_igammaC3_22, const39_igammaC3_23,
  const39_igammaC3_24
};

TO_QSPI static const real_t *const igammaTemmeC4[] = {
  const39_igammaC4_00, const39_igammaC4_01, const39_igammaC4_02,
  const39_igammaC4_03, const39_igammaC4_04, const39_igammaC4_05,
  const39_igammaC4_06, const39_igammaC4_07, const39_igammaC4_08,
  const39_igammaC4_09, const39_igammaC4_10, const39_igammaC4_11,
  const39_igammaC4_12, const39_igammaC4_13, const39_igammaC4_14,
  const39_igammaC4_15, const39_igammaC4_16, const39_igammaC4_17,
  const39_igammaC4_18, const39_igammaC4_19, const39_igammaC4_20,
  const39_igammaC4_21, const39_igammaC4_22
};

TO_QSPI static const real_t *const igammaTemmeC5[] = {
  const39_igammaC5_00, const39_igammaC5_01, const39_igammaC5_02,
  const39_igammaC5_03, const39_igammaC5_04, const39_igammaC5_05,
  const39_igammaC5_06, const39_igammaC5_07, const39_igammaC5_08,
  const39_igammaC5_09, const39_igammaC5_10, const39_igammaC5_11,
  const39_igammaC5_12, const39_igammaC5_13, const39_igammaC5_14,
  const39_igammaC5_15, const39_igammaC5_16
};

TO_QSPI static const real_t *const igammaTemmeC6[] = {
  const39_igammaC6_00, const39_igammaC6_01, const39_igammaC6_02,
  const39_igammaC6_03, const39_igammaC6_04, const39_igammaC6_05,
  const39_igammaC6_06, const39_igammaC6_07, const39_igammaC6_08,
  const39_igammaC6_09, const39_igammaC6_10, const39_igammaC6_11,
  const39_igammaC6_12, const39_igammaC6_13, const39_igammaC6_14
};

TO_QSPI static const real_t *const igammaTemmeC7[] = {
  const39_igammaC7_00, const39_igammaC7_01, const39_igammaC7_02,
  const39_igammaC7_03, const39_igammaC7_04, const39_igammaC7_05,
  const39_igammaC7_06, const39_igammaC7_07, const39_igammaC7_08,
  const39_igammaC7_09, const39_igammaC7_10, const39_igammaC7_11,
  const39_igammaC7_12
};

static void igammaTemmePolynomial(const real_t *const coefficients[], int32_t count, const real_t *z, real_t *res, realContext_t *realContext) {
  real_t coefficient;

  realCopy(coefficients[count - 1], res);
  for(int32_t i = count - 2; i >= 0; i--) {
    realCopy(coefficients[i], &coefficient);
    realFMA(res, z, &coefficient, res, realContext);
  }
}


static void igammaTemmePhi(const real_t *sigma, real_t *res, realContext_t *realContext) {
  real_t factor, nReal, nPlusOne, previous, term;

  realMultiply(sigma, sigma, &term, realContext);
  realDivide(&term, const_2, &term, realContext);
  realCopy(&term, res);
  realMinus(sigma, &factor, realContext);
  for(int32_t n = 2; n < 1000; n++) {
    realMultiply(&term, &factor, &term, realContext);
    int32ToReal(n, &nReal);
    realMultiply(&term, &nReal, &term, realContext);
    int32ToReal(n + 1, &nPlusOne);
    realDivide(&term, &nPlusOne, &term, realContext);
    realCopy(res, &previous);
    realAdd(res, &term, res, realContext);
    if(realCompareEqual(res, &previous)) {
      break;
    }
  }
}


static void igammaTemmeTail(const real_t *a, const real_t *x, real_t *res, realContext_t *realContext) {
  real_t correction, exponential, inverseA, phi, scale, sigma, workspace[8], y, z;

  realSubtract(x, a, &sigma, realContext);
  realDivide(&sigma, a, &sigma, realContext);
  igammaTemmePhi(&sigma, &phi, realContext);
  realMultiply(a, &phi, &y, realContext);
  realMultiply(&phi, const_2, &z, realContext);
  realSquareRoot(&z, &z, realContext);
  if(realCompareLessThan(x, a)) {
    realChangeSign(&z);
  }

  igammaTemmePolynomial(igammaTemmeC0, 31, &z, &workspace[0], realContext);
  igammaTemmePolynomial(igammaTemmeC1, 29, &z, &workspace[1], realContext);
  igammaTemmePolynomial(igammaTemmeC2, 27, &z, &workspace[2], realContext);
  igammaTemmePolynomial(igammaTemmeC3, 25, &z, &workspace[3], realContext);
  igammaTemmePolynomial(igammaTemmeC4, 23, &z, &workspace[4], realContext);
  igammaTemmePolynomial(igammaTemmeC5, 17, &z, &workspace[5], realContext);
  igammaTemmePolynomial(igammaTemmeC6, 15, &z, &workspace[6], realContext);
  igammaTemmePolynomial(igammaTemmeC7, 13, &z, &workspace[7], realContext);

  realDivide(const_1, a, &inverseA, realContext);
  realCopy(&workspace[7], &correction);
  for(int32_t i = 6; i >= 0; i--) {
    realFMA(&correction, &inverseA, &workspace[i], &correction, realContext);
  }
  realMinus(&y, &exponential, realContext);
  realExp(&exponential, &exponential, realContext);
  realMultiply(&correction, &exponential, &correction, realContext);
  realMultiply(const39_2pi, a, &scale, realContext);
  realSquareRoot(&scale, &scale, realContext);
  realDivide(&correction, &scale, &correction, realContext);
  if(realCompareLessThan(x, a)) {
    realChangeSign(&correction);
  }

  // Q(1/2,y) is erfc(sqrt(y)); this recursive call cannot select the large-shape expansion.
  WP34S_GammaP(&y, const_1on2, res, realContext, true, true);
  realDivide(res, const_2, res, realContext);
  realAdd(res, &correction, res, realContext);
}


static void gser(const real_t *a, const real_t *x, const real_t *gln, real_t *res, realContext_t *realContext) {
  real_t ap, del, sum, t, u;
  int32_t i;

  if(realCompareLessEqual(x, const_0)) {
    realSetZero(res);
    return;
  }
  realCopy(a, &ap);
  realDivide(const_1, a, &sum, realContext);
  realCopy(&sum, &del);
  for(i=0; i<1000; i++) {
    realAdd(&ap, const_1, &ap, realContext);
    realDivide(x, &ap, &t, realContext);
    realMultiply(&del, &t, &del, realContext);
    realAdd(&sum, &del, &t, realContext);
    if(realCompareEqual(&sum, &t)) {
      break;
    }
    realCopy(&t, &sum);
  }
  WP34S_Ln(x, &t, realContext);
  realMultiply(&t, a, &u, realContext);
  realSubtract(&u, x, &t, realContext);
  realSubtract(&t, gln, &u, realContext);
  realExp(&u, &t, realContext);
  realMultiply(&sum, &t, res, realContext);
  return;
}


static void gcheckSmall(real_t *v, realContext_t *realContext) {
  if(realCompareAbsLessThan(v, const_1e_10000)) {
    realCopy(const_1e_10000, v);
  }
}


static void gcf(const real_t *a, const real_t *x, const real_t *gln, real_t *res, realContext_t *realContext) {
  real_t an, b, c, d, h, t, u, v, i;
  int32_t n;

  realAdd(x, const_1, &t, realContext);
  realSubtract(&t, a, &b, realContext);    // b = (x+1) - a
  gcheckSmall(&b, realContext);
  realSetPlusInfinity(&c);
  realDivide(const_1, &b, &d, realContext);
  realCopy(&d, &h);
  realSetZero(&i);
  for(n=0; n<1000; n++) {
    realAdd(&i, const_1, &i, realContext);
    realSubtract(a, &i, &t, realContext);   // t = a-i
    realMultiply(&i, &t, &an, realContext); // an = -i (i-a)
    realAdd(&b, const_2, &b, realContext);
    realMultiply(&an, &d, &t, realContext);
    realAdd(&t, &b, &v, realContext);
    gcheckSmall(&v, realContext);
    realDivide(const_1, &v, &d, realContext);
    realDivide(&an, &c, &t, realContext);
    realAdd(&b, &t, &c, realContext);
    gcheckSmall(&c, realContext);
    realMultiply(&d, &c, &t, realContext);
    realMultiply(&h, &t, &u, realContext);
    if(realCompareEqual(&u, &h)) {
      break;
    }
    realCopy(&u, &h);
  }
  WP34S_Ln(x, &t, realContext);
  realMultiply(&t, a, &u, realContext);
  realSubtract(&u, x, &t, realContext);
  realSubtract(&t, gln, &u, realContext);
  realExp(&u, &t, realContext);
  realMultiply(&t, &h, res, realContext);
  return;
}


static void gserUpper(const real_t *a, const real_t *x, const real_t *lga, real_t *res, realContext_t *realContext, bool_t regularised) {
  real_t ap, difference, divisor, logPower, logX, power, previous, ratio, scale, series, term;

  WP34S_Ln(x, &logX, realContext);
  realAdd(a, const_1, &ap, &ctxtReal75);
  if(realCompareLessEqual(a, const_1e_37)) {
    realMinus(const39_egamma, &ratio, realContext);
  }
  else {
    WP34S_LnGamma(&ap, &ratio, &ctxtReal75);
    realDivide(&ratio, a, &ratio, realContext);
  }
  realSubtract(&ratio, &logX, &difference, realContext);
  realMultiply(a, &difference, &difference, realContext);
  WP34S_ExpM1(&difference, &difference, realContext);
  realDivide(&difference, a, &difference, realContext);

  realDivide(x, &ap, &term, realContext);
  realCopy(&term, &series);
  for(int32_t n = 1; n < 1000; n++) {
    realMultiply(&term, x, &term, realContext);
    realChangeSign(&term);
    realMultiply(&term, &ap, &term, realContext);
    int32ToReal(n + 1, &divisor);
    realDivide(&term, &divisor, &term, realContext);
    realAdd(&ap, const_1, &ap, realContext);
    realDivide(&term, &ap, &term, realContext);
    realCopy(&series, &previous);
    realAdd(&series, &term, &series, realContext);
    if(realCompareEqual(&series, &previous)) {
      break;
    }
  }
  realAdd(&difference, &series, &series, realContext);
  realMultiply(a, &logX, &logPower, realContext);
  realExp(&logPower, &power, realContext);
  realMultiply(&power, &series, res, realContext);
  if(regularised) {
    realMinus(lga, &scale, realContext);
    realExp(&scale, &scale, realContext);
    realMultiply(res, &scale, res, realContext);
  }
}


void WP34S_GammaP(const real_t *x, const real_t *a, real_t *res, realContext_t *realContext, bool_t upper, bool_t regularised) {
  int32_t savedContextDigits;
  bool_t centralTransition;
  real_t lga, sigma, threshold, z;

  if(realIsNegative(x) || realCompareLessEqual(a, const_0) || realIsNaN(x) || realIsNaN(a) || realIsInfinite(a)) {
    realSetNaN(res);
    return;
  }

  if(realIsInfinite(x)) {
    if(upper) {
      realSetZero(res);
      return;
    }

    if(regularised) {
      realSetOne(res);
      return;
    }
    WP34S_Gamma(a, res, realContext);
    return;
  }

  if(realCompareGreaterEqual(a, const_9000)) {
    realSubtract(x, a, &sigma, realContext);
    realDivide(&sigma, a, &sigma, realContext);
    realMultiply(&sigma, &sigma, &sigma, realContext);
    realDivide(const_200, a, &threshold, realContext);
    centralTransition = realCompareLessThan(&sigma, &threshold);
    if(centralTransition || realCompareLessEqual(&sigma, const_1on40000)) {
      if(!regularised) {
        realSetPlusInfinity(res);
        return;
      }
      savedContextDigits = realContext->digits;
      if(!centralTransition && savedContextDigits < 51) {
        realContext->digits = 51;
      }
      igammaTemmeTail(a, x, res, realContext);
      realContext->digits = savedContextDigits;
      if(upper != realCompareGreaterEqual(x, a)) {
        realSubtract(const_1, res, res, realContext);
      }
      return;
    }
  }

  realAdd(a, const_1, &lga, realContext);
  realCompare(x, &lga, &z, realContext);
  if(regularised) {
    WP34S_LnGamma(a, &lga, realContext);
  }
  else {
    realSetZero(&lga);
  }
  if(upper && !realIsZero(x) && realCompareLessEqual(x, const_1) && realCompareLessEqual(a, const_1)) {
    gserUpper(a, x, &lga, res, realContext, regularised);
    return;
  }
  if(realIsNegative(&z)) {
    /* Deal with a difficult case by using the other expansion */
    if(realCompareGreaterThan(a, const_9000)) {
      realCopy(const_995on1000, &z);
      realMultiply(a, &z, &z, realContext);
      if(realCompareGreaterThan(x, &z)) {
        goto use_cf;
      }
    }

    gser(a, x, &lga, res, realContext);
    if(upper) {
      goto invert;
    }
  }
  else {
    use_cf:
    gcf(a, x, &lga, res, realContext);
    if(!upper) {
      goto invert;
    }
  }
  return;

  invert:
  if(regularised) {
    realSubtract(const_1, res, res, realContext);
    return;
  }
  WP34S_Gamma(a, &z, realContext);
  realSubtract(&z, res, res, realContext);
  return;
}


static void erfSeries(const real_t *x, real_t *res, realContext_t *realContext) {
  real_t square, term, sum, previous, numerator, denominator, rootPi;
  const real_t *pi = realContext->digits > 39 ? const75_pi : const39_pi;

  realMultiply(x, x, &square, realContext);
  realChangeSign(&square);
  realSetOne(&term);
  realSetOne(&sum);
  for(int32_t n = 1; n < 1000; n++) {
    realMultiply(&term, &square, &term, realContext);
    int32ToReal(2 * n - 1, &numerator);
    realMultiply(&term, &numerator, &term, realContext);
    int32ToReal(n * (2 * n + 1), &denominator);
    realDivide(&term, &denominator, &term, realContext);
    realCopy(&sum, &previous);
    realAdd(&sum, &term, &sum, realContext);
    if(realCompareEqual(&sum, &previous)) {
      break;
    }
  }
  realMultiply(x, &sum, res, realContext);
  realMultiply(res, const_2, res, realContext);
  realSquareRoot(pi, &rootPi, realContext);
  realDivide(res, &rootPi, res, realContext);
}


// erf and erfc were embedded library functions on WP 34S
void WP34S_Erf(const real_t *x, real_t *res, realContext_t *realContext) {
  real_t p, q;
  const real_t *pi = realContext->digits > 39 ? const75_pi : const39_pi;
  bool_t negative;

  if(realIsNaN(x)) {
    realCopy(x, res);
    return;
  }

  if(realIsInfinite(x)) {
    int32ToReal(realIsNegative(x) ? -1 : 1, res);
    return;
  }

  if(realIsZero(x)) {
    if(realIsNegative(x)) {
      realDivide(const__1, const_plusInfinity, res, realContext);
    }
    else {
      realSetZero(res);
    }
    return;
  }

  negative = realIsNegative(x);
  if(realCompareAbsLessThan(x, const_1on10)) {
    erfSeries(x, res, realContext);
  }
  else {
    realMultiply(x, x, &p, realContext);
    WP34S_GammaP(&p, const_1on2, &p, realContext, false, false);
    realSquareRoot(pi, &q, realContext);
    realDivide(&p, &q, res, realContext);
    if(negative) {
      realChangeSign(res);
    }
  }
  if(realCompareAbsEqual(res, const_1)) {
    realNextToward(res, const_0, res, realContext);
  }
}


#if defined(OPTION_DIST_NORML)   // erfc remains part of the normal-distribution build option
void WP34S_Erfc(const real_t *x, real_t *res, realContext_t *realContext) {
  real_t p, q;
  const real_t *pi = realContext->digits > 39 ? const75_pi : const39_pi;
  bool_t negative;

  if(realIsNaN(x)) {
    realCopy(x, res);
    return;
  }
  if(realIsInfinite(x)) {
    realCopy(realIsNegative(x) ? const_2 : const_0, res);
    return;
  }
  if(realIsZero(x)) {
    realSetOne(res);
    return;
  }

  negative = realIsNegative(x);
  if(realCompareAbsLessThan(x, const_1on10)) {
    WP34S_Erf(x, &p, realContext);
    realSubtract(const_1, &p, res, realContext);
    if(realCompareEqual(res, const_1)) {
      realNextToward(res, negative ? const_2 : const_0, res, realContext);
    }
    return;
  }

  realMultiply(x, x, &p, realContext);
  WP34S_GammaP(&p, const_1on2, &p, realContext, true, false);
  realSquareRoot(pi, &q, realContext);
  realDivide(&p, &q, &p, realContext);
  if(negative) {
    realSubtract(const_2, &p, res, realContext);
    if(realCompareEqual(res, const_2)) {
      realNextToward(res, const_0, res, realContext);
    }
  }
  else {
    realCopy(&p, res);
    if(realIsZero(res)) {
      realNextToward(res, const_1, res, realContext);
    }
  }
}
#endif // OPTION_DIST_NORML


static void betaFractionCheckScale(real_t *value) {
  bool_t negative;

  if(realCompareAbsLessThan(value, const_1e_10000)) {
    negative = realIsNegative(value);
    realCopy(const_1e_10000, value);
    if(negative) {
      realSetNegativeSign(value);
    }
  }
}


static void betaFractionStep(const real_t *coefficient, real_t *d, real_t *c, realContext_t *realContext) {
  real_t term, value;

  realFMA(coefficient, d, const_1, &value, realContext);
  betaFractionCheckScale(&value);
  realDivide(const_1, &value, d, realContext);
  realDivide(coefficient, c, &term, realContext);
  realAdd(&term, const_1, c, realContext);
  betaFractionCheckScale(c);
}


/**
 * \brief Evaluates an incomplete-beta continued fraction.
 *
 * \param[in] a First beta parameter
 * \param[in] b Second beta parameter
 * \param[in] x Evaluation point
 * \param[in] convergenceExponent Relative convergence exponent
 * \param[in] iterationMax Maximum iteration count
 * \param[out] res Continued fraction
 * \param[in] realContext Calculation context
 * \return true when the continued fraction converges
 */
bool_t WP34S_BetaFraction(const real_t *a, const real_t *b, const real_t *x, int32_t convergenceExponent, int32_t iterationMax, real_t *res,
                          realContext_t *realContext) {
  real_t coefficient, c, d, ab, am1, ap1, m, twiceM, old, term, tolerance, u, v, w;
  int32_t index, monitor = 0;

  realAdd(a, const_1, &ap1, realContext);
  realSubtract(a, const_1, &am1, realContext);
  realAdd(a, b, &ab, realContext);
  realSetOne(&c);
  realDivide(x, &ap1, &term, realContext);
  realFMA(&term, &ab, const__1, &term, realContext);
  realChangeSign(&term);
  betaFractionCheckScale(&term);
  realDivide(const_1, &term, &d, realContext);
  realCopy(&d, res);
  realSetZero(&m);
  realSetOne(&tolerance);
  tolerance.exponent = convergenceExponent;

  for(index=0; index<iterationMax; index++) {
    realCopy(res, &old);
    realAdd(&m, const_1, &m, realContext);
    realMultiply(&m, const_2, &twiceM, realContext);

    realSubtract(b, &m, &term, realContext);
    realMultiply(&term, &m, &u, realContext);
    realMultiply(&u, x, &term, realContext);
    realAdd(&am1, &twiceM, &u, realContext);
    realAdd(a, &twiceM, &v, realContext);
    realMultiply(&u, &v, &w, realContext);
    realDivide(&term, &w, &coefficient, realContext);
    betaFractionStep(&coefficient, &d, &c, realContext);
    realMultiply(res, &d, &term, realContext);
    realMultiply(&term, &c, res, realContext);

    realAdd(a, &m, &term, realContext);
    realAdd(&ab, &m, &u, realContext);
    realMultiply(&term, &u, &w, realContext);
    realMultiply(&w, x, &w, realContext);
    realChangeSign(&w);
    realAdd(a, &twiceM, &term, realContext);
    realAdd(&ap1, &twiceM, &u, realContext);
    realMultiply(&term, &u, &v, realContext);
    realDivide(&w, &v, &coefficient, realContext);
    betaFractionStep(&coefficient, &d, &c, realContext);
    realMultiply(&d, &c, &term, realContext);
    realMultiply(res, &term, res, realContext);

    if(realCompareEqual(&old, res) || WP34S_RelativeError(&old, res, &tolerance, realContext)) {
      return true;
    }
    if(monitorExit(&monitor, "Iter: ")) {
      return false;
    }
  }
  return false;
}


static void check_low(real_t *d) {
  real_t real_1e_32;

  realSetOne(&real_1e_32);
  real_1e_32.exponent -= 32;
  if(realCompareAbsLessThan(d, &real_1e_32)) {
    realCopy(d, &real_1e_32);
  }
}


static void ib_step(const real_t *aa, real_t *d, real_t *c, realContext_t *realContext) {
  real_t t, u;

  realMultiply(aa, d, &t, realContext);
  realAdd(&t, const_1, &u, realContext);   // d = 1+aa*d
  check_low(&u);
  realDivide(const_1, &u, d, realContext);
  realDivide(aa, c, &t, realContext);
  realAdd(&t, const_1, c, realContext);    // c = 1+aa/c
  check_low(c);
}


static void betacf(const real_t *a, const real_t *b, const real_t *x, real_t *r, realContext_t *realContext) {
  real_t aa, c, d, apb, am1, ap1, m, m2, oldr;
  int i;
  real_t t, u, v, w;
  int32_t loop = 0;

  realAdd(a, const_1, &ap1, realContext);        // ap1 = 1+a
  realSubtract(a, const_1, &am1, realContext);   // am1 = a-1
  realAdd(a, b, &apb, realContext);              // apb = a+b
  realSetOne(&c);                                // c = 1
  realDivide(x, &ap1, &t, realContext);
  realMultiply(&t, &apb, &u, realContext);
  realSubtract(const_1, &u, &t, realContext);    // t = 1-apb*x/ap1
  check_low(&t);
  realDivide(const_1, &t, &d, realContext);      // d = 1/t
  realCopy(&d, r);    // res = d
  realSetZero(&m);
  for(i=0; i<500; i++) {
    realCopy(r, &oldr);
    realAdd(&m, const_1, &m, realContext);       // m = i+1
    realMultiply(&m, const_2, &m2, realContext);
    realSubtract(b, &m, &t, realContext);
    realMultiply(&t, &m, &u, realContext);
    realMultiply(&u, x, &t, realContext);        // t = m*(b-m)*x
    realAdd(&am1, &m2, &u, realContext);
    realAdd(a, &m2, &v, realContext);
    realMultiply(&u, &v, &w, realContext);       // w = (am1+m2)*(a+m2)
    realDivide(&t, &w, &aa, realContext);        // aa = t/w
    ib_step(&aa, &d, &c, realContext);
    realMultiply(r, &d, &t, realContext);
    realMultiply(&t, &c, r, realContext);        // r = r*d*c
    realAdd(a, &m, &t, realContext);
    realAdd(&apb, &m, &u, realContext);
    realMultiply(&t, &u, &w, realContext);
    realMultiply(&w, x, &t, realContext);
    realMinus(&t, &w, realContext);              // w = -(a+m)*(apb+m)*x
    realAdd(a, &m2, &t, realContext);
    realAdd(&ap1, &m2, &u, realContext);
    realMultiply(&t, &u, &v, realContext);       // v = (a+m2)*(ap1+m2)
    realDivide(&w, &v, &aa, realContext);        // aa = w/v
    ib_step(&aa, &d, &c, realContext);
    realMultiply(&d, &c, &v, realContext);
    realMultiply(r, &v, r, realContext);         // r *= d*c
    if(realCompareEqual(&oldr, r)) {
      break;
    }
    if(monitorExit(&loop, "Iter: ")) {
      break;
    }
  }
}


// Regularised incomplete beta function Ix(a, b)
void WP34S_betai(const real_t *b, const real_t *a, const real_t *x, real_t *res, realContext_t *realContext) {
  real_t t, u, v, w, y;
  int32_t limit = 0;

  realCompare(const_1, x, &t, realContext);
  if(realIsNegative(x) || realIsNegative(&t)) {
    realSetNaN(res);
   return;
  }

  if(realIsZero(x) || realIsZero(&t)) {
    limit = 1;
  }
  else {
    LnBeta(a, b, &u, realContext);
    WP34S_Ln(x, &v, realContext);              // v = ln(x)
    realMultiply(a, &v, &t, realContext);
    realSubtract(&t, &u, &v, realContext);     // v = lng(...)+a.ln(x)
    realSubtract(const_1, x, &y, realContext); // y = 1-x
    WP34S_Ln(&y, &u, realContext);             // u = ln(1-x)
    realMultiply(&u, b, &t, realContext);
    realAdd(&t, &v, &u, realContext);          // u = lng(...)+a.ln(x)+b.ln(1-x)
    realExp(&u, &w, realContext);
  }

  realAdd(a, b, &v, realContext);
  realAdd(&v, const_2, &u, realContext);       // u = a+b+2
  realAdd(a, const_1, &t, realContext);        // t = a+1
  realDivide(&t, &u, &v, realContext);         // u = (a+1)/(a+b+2)
  if(realCompareLessThan(x, &v)) {
    if(limit) {
      realSetZero(res);
    }
    else {
      betacf(a, b, x, &t, realContext);
      realDivide(&t, a, &u, realContext);
      realMultiply(&w, &u, res, realContext);
    }
  }
  else {
    if(limit) {
      realSetOne(res);
    }
    else {
      betacf(b, a, &y, &t, realContext);
      realDivide(&t, b, &u, realContext);
      realMultiply(&w, &u, &t, realContext);
      realSubtract(const_1, &t, res, realContext);
    }
  }
}


void WP34S_Bernoulli(const real_t *x, real_t *res, bool_t bn_star, realContext_t *realContext) {
  real_t p;

  if((!realIsAnInteger(x)) || realCompareLessThan(x, const_0)) {
    realSetNaN(res);
    return;
  }
  if(realIsZero(x)) {// Bn_0
    realCopy(const_1, res);
    return;
  }
  if(!bn_star) {
    if(realCompareEqual(x, const_1)) { // zeta_0
      realCopy(const_1on2, res);
      realChangeSign(res);
      return;
    }
    else if(realMultiply(x, const_1on2, &p, realContext), (!realIsAnInteger(&p))) { // Bn_odd
      realSetZero(res);
      return;
    }
    realCopy(x, &p);
  }
  else {
    realMultiply(x, const_2, &p, realContext);
  }

  if(realGetExponent(&p) >= realContext->digits) {
    realSetNaN(res);
    return;
  }

  // bernoulli
  realSubtract(&p, const_1, &p, realContext);
  realChangeSign(&p);
  WP34S_Zeta(&p, &p, realContext);
  realMultiply(&p, x, &p, realContext);
  realChangeSign(&p);

  if(bn_star) {
    realMultiply(&p, const_2, &p, realContext);
    realSetPositiveSign(&p);
  }
  realCopy(&p, res);
}


/**************************************************************************/
/* Zeta function implementation based on Jean-Marc Baillard's from:
 * http://hp41programs.yolasite.com/zeta.php
 * This is the same algorithm as the C version uses, just with fewer terms and
 * with the constants computed on the fly.
 */

static void zeta_calc(const real_t *x, real_t *reg1, real_t *reg7, real_t *res, realContext_t *realContext) {
  real_t p, q, r, s, reg0, reg3, reg4, reg5, reg6;
  const real_t *ln2 = realContext->digits > 39 ? const75_ln2 : const39_ln2;
  int32_t loop = 0;

  int32ToReal(60/*44*/, &reg0);
  int32ToReal(60/*44*/, &reg3);
  int32ToReal(1, &reg4);
  int32ToReal(1, &reg5);
  int32ToReal(-1, &reg6);
  realSetZero(&p);
  do { // zeta_loop
    realMinus(reg1, &q, realContext);
    realPower(&reg0, &q, &q, realContext);
    realMultiply(&reg5, &q, &q, realContext);
    realChangeSign(&reg6);
    realMultiply(&q, &reg6, &q, realContext);
    realAdd(&p, &q, &p, realContext);
    realMultiply(&reg0, const_2, &q, realContext);
    realMultiply(&q, &reg0, &q, realContext);
    realSubtract(&q, &reg0, &q, realContext);
    realMultiply(&q, &reg4, &q, realContext);
    realMultiply(&reg3, &reg3, &r, realContext);
    realSubtract(&reg0, const_1, &s, realContext);
    realMultiply(&s, &s, &s, realContext);
    realSubtract(&r, &s, &r, realContext);
    realMultiply(&r, const_2, &r, realContext);
    realDivide(&q, &r, &q, realContext);
    realCopy(&q, &reg4);
    realAdd(&q, &reg5, &reg5, realContext);
    realSubtract(&reg0, const_1, &reg0, realContext);
    if(monitorExit(&loop, "Iter: ")) {
      break;
    }
  } while(realCompareGreaterThan(&reg0, const_0));
  realDivide(&p, &reg5, &p, realContext);
  realSubtract(const_1, reg1, &r, realContext);
  realMultiply(ln2, &r, &r, realContext);
  WP34S_ExpM1(&r, &q, realContext);
  realDivide(&p, &q, res, realContext);
}


void WP34S_Zeta(const real_t *x, real_t *res, realContext_t *realContext) {
  real_t p, q, r, reg1, reg7;
  const real_t *pi = realContext->digits > 39 ? const75_pi : const39_pi;
  const real_t *twoPi = realContext->digits > 39 ? const75_2pi : const39_2pi;

  if(realIsZero(x)) {
    realCopy(const_1on2, res);
    realChangeSign(res);
    return;
  }
  if(realCompareEqual(x, const_1)) {
    realSetPlusInfinity(res);
    return;
  }
  if(realIsNegative(x) && realIsAnInteger(x)) {
    realMultiply(x, const_1on2, &p, realContext);
    if(realIsAnInteger(&p)) {
      realSetZero(res);
      return;
    }
  }

  // zeta_int
  realCopy(x, &reg1);
  realCopy(x, &reg7);
  if(realCompareGreaterThan(x, const_1on2)) {
    zeta_calc(x, &reg1, &reg7, res, realContext);
  }
  else { // zeta_neg
    realSubtract(const_1, x, &q, realContext);
    realCopy(&q, &reg1);
    zeta_calc(&q, &reg1, &reg7, &p, realContext);
    C47_WP34S_Asin(const_1, &q, realContext);
    realMultiply(&q, &reg7, &q, realContext);
    C47_WP34S_Cvt2RadSinCosTan(&q, amRadian, &r, NULL, NULL, realContext);
    realMultiply(&p, &r, &p, realContext);
    realDivide(&p, pi, &p, realContext);
    realPower(twoPi, &reg7, &q, realContext);
    realMultiply(&p, &q, &p, realContext);
    realCopy(&reg1, &q);
    WP34S_Gamma(&q, &r, realContext);
    realMultiply(&r, &p, res, realContext);
  }
}


/**************************************************************************/
/* A rough guess is made and then Newton's method is used to attain
 * convergence.  The C version (in ../trunk/unused/lambertW.c) works
 * similarily but uses Halley's method which converges a cubically
 * instead of quadratically.
 *
 * This code is based on a discussion on the MoHPC.  Search the archives
 * for "Lambert W (HP-41)" starting on the 2 Sept 2012.  The actual code
 * is Dieter's from a message posted on the 23rd of Sept 2012.
 */
void WP34S_LambertW(const real_t *x, real_t *res, bool_t negativeBranch, realContext_t *realContext) {
  real_t e, p, q, r, reg0, reg1, reg2;
  bool_t converged = false;
  const real_t *tolerance = realContext->digits <= 39 ? const_1e_37 : const_1e_49;

  if(realIsNaN(x)) {
    realSetNaN(res);
    return;
  }
  if(realIsInfinite(x)) {
    if(!negativeBranch && realIsPositive(x)) {
      realSetPlusInfinity(res);
    }
    else {
      realSetNaN(res);
    }
    return;
  }
  if(realIsZero(x)) {
    if(negativeBranch) {
      realSetMinusInfinity(res);
    }
    else {
      realCopy(x, res);
    }
    return;
  }
  if(negativeBranch && realIsPositive(x)) {
    realSetNaN(res);
    return;
  }

  // LamW0_common
  realCopy(x, &reg0);
  int32ToReal(7, &reg1);
  int32ToReal(negativeBranch ? 25 : 35, &p), p.exponent -= 2, realChangeSign(&p);
  if(realCompareLessEqual(&reg0, &p)) {
    realExp(const_1, &e, realContext);
    realMultiply(&reg0, &e, &q, realContext);
    realAdd(&q, const_1, &q, realContext);
    realCopy(&q, &reg2);
    realMultiply(&q, const_2, &q, realContext);
    realSquareRoot(&q, &r, realContext);
    if(negativeBranch) {
      realChangeSign(&r);
    }
    realMultiply(&q, const39_1on3, &q, realContext);
    realSubtract(&r, &q, &q, realContext);

    // Newton iteration for W+1
    do { //LamW0_wp1_newton
      // FILL and x close to -1/e
      realMinus(&q, &p, realContext);
      WP34S_ExpM1(&p, &p, realContext);
      realMultiply(&p, &reg0, &p, realContext);
      realMultiply(&p, &e, &p, realContext);
      realSubtract(&p, &q, &p, realContext);
      realAdd(&p, &reg2, &p, realContext);
      realDivide(&p, &q, &p, realContext);
      realAdd(&p, &q, &p, realContext);
      if(converged) {
        break;
      }
      if(WP34S_AbsoluteError(&p, &q, tolerance, realContext)) { // test if absolute error
        converged = true;
      }
      realSubtract(&reg1, const_1, &reg1, realContext);
      realCopy(&p, &q);
    } while(realCompareGreaterThan(&reg1, const_0));
    // LamW0_converged
    realSubtract(&p, const_1, res, realContext);
  }
  else {// LamW0_normal
    if(negativeBranch) {// LamW0_smallx
      realMinus(&reg0, &q, realContext);
      WP34S_Ln(&q, &q, realContext);
      realMinus(&q, &r, realContext);
      WP34S_Ln(&r, &r, realContext);
      realSubtract(&q, &r, &q, realContext);
    }
    else {
      WP34S_Ln1P(&reg0, &q, realContext);
      if(realCompareGreaterThan(&q, const_1)) {
        WP34S_Ln(&q, &r, realContext);
        realSubtract(&q, &r, &q, realContext);
      }
    }
    // Newton-Halley iteration for W
    // x not close to -1/e
    do { // LamW0_halley
      realExp(&q, &r, realContext);
      realDivide(&reg0, &r, &r, realContext);
      realSubtract(&q, &r, &r, realContext);
      realAdd(&q, const_1, &p, realContext);
      realDivide(&r, &p, &r, realContext);
      realDivide(const_1, &p, &p, realContext);
      realAdd(&p, const_1, &p, realContext);
      realMultiply(&p, &r, &p, realContext);
      realMultiply(&p, const_1on2, &p, realContext);
      realChangeSign(&p);
      realAdd(&p, const_1, &p, realContext);
      realDivide(&r, &p, &r, realContext);
      realSubtract(&q, &r, &r, realContext);
      // R Q Q
      if(converged) {
        break;
      }
      if(WP34S_RelativeError(&r, &q, tolerance, realContext)) { // test if relative error
        converged = true;
      }
      realSubtract(&reg1, const_1, &reg1, realContext);
      realCopy(&r, &q);
    } while(realCompareGreaterThan(&reg1, const_0));
    // LamW0_finish
    realCopy(&r, res);
  }
}


/**************************************************************************/
/* The positive branch of the complex W function.
 *
 * This code is based on Jean-Marc Baillard's HP-41 version from:
 * http://hp41programs.yolasite.com/lambertw.php
 *
 * Register use:
 * .00/.01 z
 * .02/.03 w
 * .04/.05 temporary
 */
void WP34S_ComplexLambertW(const cplx_t *x, cplx_t *res, realContext_t *realContext) {
  real_t d, dPrev;
  cplx_t p, q, z, w, t, delta;
  const real_t *piConstant = realContext->digits <= 39 ? const39_pi : const75_pi;
  const real_t *piOn2Constant = realContext->digits <= 39 ? const39_piOn2 : const75_piOn2;
  const real_t *tolerance = const_1e_37;
  const bool_t negativeImaginarySide = realIsNegative(&x->Imag);
  int32_t iteration = 0;
  int32_t flat = 0;

  if(realIsNaN(&x->Real) || realIsNaN(&x->Imag) || (realIsInfinite(&x->Real) && realIsInfinite(&x->Imag))) {
    realSetNaN(&res->Real);
    realSetNaN(&res->Imag);
    return;
  }
  if(realIsInfinite(&x->Real)) {
    realSetPlusInfinity(&res->Real);
    if(realIsPositive(&x->Real)) {
      realSetZero(&res->Imag);
    }
    else {
      realCopy(piConstant, &res->Imag);
    }
    if(negativeImaginarySide) {
      realSetNegativeSign(&res->Imag);
    }
    return;
  }
  if(realIsInfinite(&x->Imag)) {
    realSetPlusInfinity(&res->Real);
    realCopy(piOn2Constant, &res->Imag);
    if(negativeImaginarySide) {
      realSetNegativeSign(&res->Imag);
    }
    return;
  }
  if(realIsZero(&x->Real) && realIsZero(&x->Imag)) {
    realCopy(&x->Real, &res->Real);
    realCopy(&x->Imag, &res->Imag);
    return;
  }

  realCopy(&x->Real, &z.Real), realCopy(&x->Imag, &z.Imag);
  realSetOne(&w.Real);
  realSetOne(&w.Imag);
  realAdd(&x->Real, const_1, &p.Real, realContext);
  realCopy(&x->Imag, &p.Imag);
  if(realIsZero(&z.Imag) && realIsNegative(&z.Real) && realCompareGreaterEqual(&z.Real, const__1)) {
    // Close to -1/e, the series is very slow to converge
    realSetOne(&p.Real);
    realCopy(negativeImaginarySide ? const__1 : const_1, &p.Imag);
  }
  else if(!realIsZero(&p.Real) || !realIsZero(&p.Imag)) {
    lnComplex(&p, &p, realContext);
    realCopy(&p.Real, &w.Real);
    realCopy(&p.Imag, &w.Imag);
  }
  realCopy(const_plusInfinity, &dPrev);
  while(1) { // LamW_cloop
    if(monitorExit(&iteration, "Iter: ")) {
      break;
    }

    expComplex(&p, &q, realContext);
    realCopy(&q.Real, &t.Real);
    realCopy(&q.Imag, &t.Imag);
    mulComplexComplex(&q, &w, &q, realContext);
    realAdd(&t.Real, &q.Real, &t.Real, realContext);
    realAdd(&t.Imag, &q.Imag, &t.Imag, realContext);
    realSubtract(&q.Real, &z.Real, &q.Real, realContext);
    realSubtract(&q.Imag, &z.Imag, &q.Imag, realContext);
    divComplexComplex(&q, &t, &q, realContext);
    realSubtract(&w.Real, &q.Real, &w.Real, realContext);
    realSubtract(&w.Imag, &q.Imag, &w.Imag, realContext);
    if(realIsSpecial(&w.Real) || realIsSpecial(&w.Imag)) {
      // The iteration has broken down, so further evaluation cannot recover.
      realSetNaN(&w.Real);
      realSetNaN(&w.Imag);
      break;
    }
    if(WP34S_RelativeError(&w.Real, &p.Real, tolerance, realContext) && WP34S_RelativeError(&w.Imag, &p.Imag, tolerance, realContext)) {
      break;
    }
    realSubtract(&w.Real, &p.Real, &delta.Real, realContext), realSubtract(&w.Imag, &p.Imag, &delta.Imag, realContext);
    complexMagnitude(&delta, &d, realContext);
    // Near the branch point the step settles into a two-cycle at the working precision.
    if(realCompareGreaterEqual(&d, &dPrev) && WP34S_ComplexRelativeError(&w, &p, const_1e_34, realContext)) {
      flat++;
      if(flat >= 2) {
        break;
      }
    }
    else {
      flat = 0;
    }
    realCopy(&d, &dPrev);
    realCopy(&w.Real, &p.Real);
    realCopy(&w.Imag, &p.Imag);
  }
  if(realIsZero(&x->Imag) && negativeImaginarySide) {
    realSetNegativeSign(&w.Imag);
  }
  realCopy(&w.Real, &res->Real);
  realCopy(&w.Imag, &res->Imag);
}


/**************************************************************************/
/* The inverse W function in both real and complex domains.
 */


void WP34S_InverseW(const real_t *x, real_t *res, realContext_t *realContext) {
  real_t p;

  if(realIsInfinite(x) && realIsNegative(x)) {
    realSetZero(res);
    realSetNegativeSign(res);
    return;
  }
  realExp(x, &p, realContext);
  realMultiply(&p, x, res, realContext);
}


void WP34S_InverseComplexW(const cplx_t *x, cplx_t *res, realContext_t *realContext) {
  cplx_t p;

  if(realIsZero(&x->Imag)) {
    WP34S_InverseW(&x->Real, &res->Real, realContext);
    realSetZero(&res->Imag);
    return;
  }
  if(realIsSpecial(&x->Real) || realIsSpecial(&x->Imag)) {
    realSetNaN(&res->Real);
    realSetNaN(&res->Imag);
    return;
  }
  expComplex(x, &p, realContext);
  mulComplexComplex(&p, x, res, realContext);
}


// Orthogonal Polynomials, common function
void WP34S_OrthoPoly(uint16_t kind, const real_t *rX, const real_t *rN, const real_t *rParam, real_t *res, realContext_t *realContext) {
#if defined(OPTION_ORTHO)
  real_t a, b, c, d, i;
  real_t rT0, rT1, incB;
  real_t p, q;
  bool_t incA = false, incC = false;

  // ortho_default
  if(realIsSpecial(rX) || (!realIsAnInteger(rN)) || realIsNegative(rN)) {
    realSetNaN(res);
    return;
  }
  if(realIsZero(rN)) {
    realSetOne(res);
    return;
  }
  // Here we are free from the limitation of ISG since the code is ported to C.
  //if(realCompareGreaterEqual(rN, const_1000)) {
  //  realSetNaN(res);
  //  return;
  //}
  realSetOne(&rT0);
  // Now initialise everything else
  realCopy(const_2, &i);
  realCopy(const_2, &d);
  realSetOne(&c);
  realSetOne(&b);
  realCopy(rX, &rT1);
  realMultiply(rX, const_2, &a, realContext);

  // We must initialise this too
  realSetZero(&incB);

  switch(kind) {
  /**************************************************************************/
  /* Legendre's Pn                                                          */
    case ORTHOPOLY_LEGENDRE_P: {
      realAdd(&a, rX, &a, realContext);
      realMultiply(rX, const_2, &d, realContext);
      goto ortho_allinc;
    }

  /**************************************************************************/
  /* Chebychev's Tn                                                         */
    case ORTHOPOLY_CHEBYSHEV_T: {
      break;
    }

  /**************************************************************************/
  /* Chebychev's Un                                                         */
    case ORTHOPOLY_CHEBYSHEV_U: {
      realAdd(&rT1, rX, &rT1, realContext);
      break;
    }

  /**************************************************************************/
  /* Laguerre's Ln and Ln with parameter alpha                              */
    case ORTHOPOLY_LAGUERRE_L:
    case ORTHOPOLY_LAGUERRE_L_ALPHA: {
      // laguerre_common
      if(realIsSpecial(rParam) || realCompareLessEqual(rParam, const__1)) {
        realSetNaN(res);
        return;
      }
      realAdd(&b, rParam, &b, realContext);
      realAdd(rParam, const_3, &a, realContext);
      realSubtract(&a, rX, &a, realContext);
      realSubtract(rParam, rX, &p, realContext);
      realSubtract(const_1, rX, &q, realContext);
      if(realCompareAbsLessThan(&p, &q)) {
        realAdd(&p, const_1, &rT1, realContext);
      }
      else {
        realAdd(&q, rParam, &rT1, realContext);
      }
      ortho_allinc:
      incA = true;
      realSetOne(&incB);
      incC = true;
      break;
    }

  /**************************************************************************/
  /* Hermite's He (Hn)                                                      */
    case ORTHOPOLY_HERMITE_HE: {
      realCopy(rX, &a);
      realSetOne(&incB);
      break;
    }

  /**************************************************************************/
  /* Hermite's H  (Hnp)                                                     */
    case ORTHOPOLY_HERMITE_H: {
      realAdd(&rT1, rX, &rT1, realContext);
      realCopy(const_2, &b);
      realCopy(const_2, &incB);
      break;
    }
  }

  /**************************************************************************/
  /* Common evaluation code                                                 */
  /* Everything is assumed properly set up at this point.                   */
  /* ortho_common                                                           */
  while(realCompareLessEqual(&i, rN)) { // ortho_loop
    realMultiply(&rT1, &a, &p, realContext);
    realMultiply(&rT0, &b, &q, realContext);
    realCopy(&rT1, &rT0);
    realSubtract(&p, &q, &rT1, realContext);

    if(incC) {
      realAdd(&c, const_1, &c, realContext);
      realDivide(&rT1, &c, &rT1, realContext);
    } // ortho_noC
    if(incA) {
      realAdd(&a, &d, &a, realContext);
    } // ortho_noA
    realAdd(&b, &incB, &b, realContext);
    realAdd(&i, const_1, &i, realContext);
  }
  realCopy(&rT1, res);
#endif //defined(OPTION_ORTHO)
}
