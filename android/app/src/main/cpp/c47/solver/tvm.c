// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file tvm.c
 ***********************************************/

#include "c47.h"


// This mdule as two control defines, controlled from defines.h:
//   OPTION_TVM_FORMULAS
//   OPTION_TVM_NEWTON


#define TVMDEBUG2 //only progress indicators
#undef  TVMDEBUG2



 // Create context for this module only
 static decContext ctxtTvm42;
 static inline void ensureTvmContext(void) {
   if(ctxtTvm42.digits == 0) {
     ctxtTvm42 = ctxtReal39;
     ctxtTvm42.digits = 42;
   }
 }


#if (defined(DMCP_BUILD) && (HARDWARE_MODEL) && (HARDWARE_MODEL == HWM_DM42)) // || defined(PC_BUILD) || defined(TESTSUITE_BUILD)
  #define ctxtTvm          ctxtReal39
  #define ctxtTvmHi         ctxtTvm42  // only some exp/log parts
  #define ctxtSolverTvmHi   ctxtTvm42  // only the exp/log parts
  #define ctxtSolverTvmInv ctxtReal51  // only the inverting of i
#else
  #define ctxtTvm          ctxtReal51
  #define ctxtTvmHi        ctxtReal51  // only some exp/log parts
  #define ctxtSolverTvmHi  ctxtReal51  // only the exp/log parts
  #define ctxtSolverTvmInv ctxtReal51  // only the inverting of i
#endif


static void doubleExp(const real_t *x, real_t *exp, real_t *expm1, realContext_t *realContext) {
  real_t v, w;

  decNumberExp(exp, x, realContext);

  // Code from WP34S_ExpM1 to get accurate result for e^x-1
  realSubtract(exp, const_1, &v, realContext);
  if(realIsZero(&v)) { // |x| is very little
    realCopy(x, expm1);
  }
  else if(realCompareEqual(&v, const__1)) {
    realCopy(const__1, expm1);
  }
  else if(realCompareAbsLessThan(x, const_1on10)) {
    realMultiply(&v, x, &w, realContext);
    WP34S_Ln(exp, &v, realContext);
    realDivide(&w, &v, expm1, realContext);
  }
  else {
    realCopy(&v, expm1);
  }
}

#if defined (OPTION_TVM_FORMULAS)
  #if (EXTRA_INFO_ON_CALC_ERROR == 1)
    const char * const tvmErrorMessages[] = {
      "TVM: Division by zero",                    // 0
      "TVM: Invalid interest rate",               // 1
      "TVM: Invalid number of periods",           // 2
      "TVM: No solution exists",                  // 3
      "TVM: Logarithm of non-positive number",    // 4
      "TVM: Payment frequency cannot be zero",    // 5
      "TVM: Compound frequency cannot be zero",   // 6
      "TVM: Present value cannot be zero",        // 7
      "TCM: Invalid variable requested",          // 8
      "TCM: Exit to proceed to old solver"        // 9
    };
  #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)


static int tvmRangeError(int errorCode) {
  displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
  #if (EXTRA_INFO_ON_CALC_ERROR == 1)
    moreInfoOnError("In function tvmRangeError:", tvmErrorMessages[errorCode], " Out of range error", NULL);
  #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  return errorCode;
}


// Calculate effective interest rate per payment period
// ip = (1 + ic)^(CPER/a / PPER/a) - 1 where ic = (I%/a / 100) / CPER/a
static void calculateEffectiveRate(const real_t *iPercentPerYear,
                                    const real_t *compoundPerYear,
                                    const real_t *paymentPerYear,
                                    real_t *ip,
                                    int *error) {
  ensureTvmContext();
  real_t ic, temp, exponent;

  // Check for zero frequencies
  if(decNumberIsZero((decNumber *)compoundPerYear)) {
    *error = tvmRangeError(6);
    return;
  }
  if(decNumberIsZero((decNumber *)paymentPerYear)) {
    *error = tvmRangeError(5);
    return;
  }

  // ic = (I%/a / 100) / CPER/a
  realDivide(iPercentPerYear, const_100, &ic, &ctxtTvm);
  realDivide(&ic, compoundPerYear, &ic, &ctxtTvm);

  // When CPER/a = PPER/a, ip = ic (shortcut)
  realSubtract(compoundPerYear, paymentPerYear, &temp, &ctxtTvm);
  if(decNumberIsZero((decNumber *)&temp)) {
    realCopy(&ic, ip);
    *error = 0;
    return;
  }

  // exponent = CPER/a / PPER/a
  realDivide(compoundPerYear, paymentPerYear, &exponent, &ctxtTvm);

  // ip = (1 + ic)^exponent - 1
  {
    real_t temp2;
    WP34S_Ln1P(&ic, &temp2, &ctxtTvmHi);                  // temp2 = ln(1 + ic)
    realMultiply(&temp2, &exponent, &temp2, &ctxtTvmHi);  // temp2 = exponent * ln(1 + ic)
    WP34S_ExpM1(&temp2, ip, &ctxtTvmHi);                  // ip = exp(exponent * ln(1 + ic)) - 1
  }

  *error = 0;
}

// Calculate Present Value (PV)
// PV = -(1 + ip*p) * (PMT/ip) * [1 - (1+ip)^(-NPPER)] - FV*(1+ip)^(-NPPER)
// with special case when ip = 0: PV = -PMT*NPPER - FV
int calculatePV(const real_t *fv,
                const real_t *iPercentPerYear,
                const real_t *npper,
                const real_t *paymentPerYear,
                const real_t *pmt,
                const real_t *compoundPerYear,
                const real_t *p,
                real_t *pv) {
  ensureTvmContext();
  real_t ip, temp1, temp2, temp3, negNpper, powerTerm, annuityFactor;
  int error = 0;

  calculateEffectiveRate(iPercentPerYear, compoundPerYear, paymentPerYear, &ip, &error);
  if(error != 0) return error;

  // Check if ip ≈ 0
  if(realCompareAbsLessThan(&ip, const_1e_37)) {
    // PV = -PMT*NPPER - FV
    realFMA(pmt, npper, fv, pv, &ctxtTvm);
    realSetNegativeSign(pv);
    return 0;
  }

  // General case: ip ≠ 0
    // Calculate annuity factor: numerator: 1 - (1+ip)^(-NPPER)
  realCopy(npper, &negNpper);
  realChangeSign(&negNpper);
  WP34S_Ln1P(&ip, &temp2, &ctxtTvm);                  // temp2 = ln(1 + ip)
  realMultiply(&temp2, &negNpper, &temp2, &ctxtTvm);  // temp2 = -NPPER * ln(1 + ip)
  doubleExp(&temp2, &powerTerm, &temp1, &ctxtTvm);
  //realExp(&temp2, &powerTerm, &ctxtTvm);              // powerTerm = (1+ip)^(-NPPER)
  //WP34S_ExpM1(&temp2, &temp1, &ctxtTvm);              // temp1 = (1+ip)^(-NPPER) - 1
  realChangeSign(&temp1);                                // temp1 = 1 - (1+ip)^(-NPPER)

  // Annuity factor = [1 - (1+ip)^(-NPPER)] / ip
  realDivide(&temp1, &ip, &annuityFactor, &ctxtTvm);

  // Possible loss of digits here could compute:
  //    e^(ln1p(ip*p) + ln(PMT * annuityFactor))

  // Payment timing factor = (1 + ip*p)
  realFMA(&ip, p, const_1, &temp2, &ctxtTvm);

  // PV from payments = (1 + ip*p) * PMT * annuityFactor
  realMultiply(&temp2, pmt, &temp1, &ctxtTvm);
  realMultiply(&temp1, &annuityFactor, &temp3, &ctxtTvm);

  // PV from FV = FV * (1+ip)^(-NPPER)
  // PV = -(payment part + FV part)
  realFMA(fv, &powerTerm, &temp3, pv, &ctxtTvm);
  realChangeSign(pv);

  return 0;
}

// Calculate Future Value (FV)
// FV = -PV*(1+ip)^NPPER - (1+ip*p) * (PMT/ip) * [(1+ip)^NPPER - 1]
// Special case when ip = 0: FV = -PV - PMT*NPPER
int calculateFV(const real_t *pv,
                const real_t *iPercentPerYear,
                const real_t *npper,
                const real_t *paymentPerYear,
                const real_t *pmt,
                const real_t *compoundPerYear,
                const real_t *p,
                real_t *fv) {
  ensureTvmContext();
  real_t ip, temp1, temp2, temp3, powerTerm, annuityFactor;
  int error = 0;

  calculateEffectiveRate(iPercentPerYear, compoundPerYear, paymentPerYear, &ip, &error);
  if(error != 0) return error;

  // Check if ip ≈ 0 (use special formula)
  if(realCompareAbsLessThan(&ip, const_1e_37)) {
    // FV = -PV - PMT*NPPER
    realFMA(pmt, npper, pv, fv, &ctxtTvm);
    realSetNegativeSign(fv);
    return 0;
  }

  // General case: ip ≠ 0
  // Calculate (1 + ip)^NPPER
  {
    real_t temp_ln;
    WP34S_Ln1P(&ip, &temp_ln, &ctxtTvm);                 // temp_ln = ln(1 + ip)
    realMultiply(&temp_ln, npper, &temp_ln, &ctxtTvm);   // temp_ln = npper * ln(1 + ip)
    doubleExp(&temp_ln, &powerTerm, &temp2, &ctxtTvm);
    //realExp(&temp_ln, &powerTerm, &ctxtTvm);             // powerTerm = (1+ip)^npper
    //WP34S_ExpM1(&temp_ln, &temp2, &ctxtTvm);             // temp2 = (1+ip)^npper - 1
  }

  // FV from PV = -PV * (1+ip)^NPPER
  realMultiply(pv, &powerTerm, &temp1, &ctxtTvm);
  realSetNegativeSign(&temp1);

  // Annuity factor = [(1+ip)^NPPER - 1] / ip
  realDivide(&temp2, &ip, &annuityFactor, &ctxtTvm);

  // Payment timing factor = (1 + ip*p)
  realFMA(&ip, p, const_1, &temp3, &ctxtTvm);

  // FV from payments = (1 + ip*p) * PMT * annuityFactor
  // FV = PV part - payment part
  realMultiply(&temp3, pmt, &temp2, &ctxtTvm);
  // fv = temp1 - (temp2 x &annuityFactor) = - ((temp2 x &annuityFactor) - temp1) = - [ (temp2 x &annuityFactor) + (-temp1) ] = - FMA[ temp2, &annuityFactor, - temp1 ]
  realChangeSign(&temp1);
  realFMA(&temp2, &annuityFactor, &temp1, fv, &ctxtTvm);
  realChangeSign(fv);

  return 0;
}

// Calculate Payment (PMT)
// PMT = -[PV + FV*(1+ip)^(-NPPER)] * ip / [(1+ip*p) * (1-(1+ip)^(-NPPER))]
// Special case when ip = 0: PMT = -(PV + FV) / NPPER
int calculatePMT(const real_t *pv,
                 const real_t *fv,
                 const real_t *iPercentPerYear,
                 const real_t *npper,
                 const real_t *paymentPerYear,
                 const real_t *compoundPerYear,
                 const real_t *p,
                 real_t *pmt) {
  ensureTvmContext();
  real_t ip, temp1, temp2, temp3, negNpper, powerTerm, numerator, denominator;
  int error = 0;

  // Check for zero periods
  if(decNumberIsZero((decNumber *)npper)) {
    return tvmRangeError(2);
  }

  calculateEffectiveRate(iPercentPerYear, compoundPerYear, paymentPerYear, &ip, &error);
  if(error != 0) return error;

  // Check if ip ≈ 0 (use special formula)
  if(realCompareAbsLessThan(&ip, const_1e_37)) {
    // PMT = -(PV + FV) / NPPER
    realAdd(pv, fv, &temp1, &ctxtTvm);
    realDivide(&temp1, npper, pmt, &ctxtTvm);
    realSetNegativeSign(pmt);
    return 0;
  }

  // General case: ip ≠ 0
  // Calculate (1 + ip)^(-NPPER)
  realCopy(npper, &negNpper);
  realChangeSign(&negNpper);
   {
    real_t temp_ln;
    WP34S_Ln1P(&ip, &temp_ln, &ctxtTvm);                    // temp_ln = ln(1 + ip)
    realMultiply(&temp_ln, &negNpper, &temp_ln, &ctxtTvm);  // temp_ln = -npper * ln(1 + ip)
    doubleExp(&temp_ln, &powerTerm, &temp3, &ctxtTvm);
    //realExp(&temp_ln, &powerTerm, &ctxtTvm);                // powerTerm = (1+ip)^(-npper)
    //WP34S_ExpM1(&temp_ln, &temp3, &ctxtTvm);                // temp3 = (1+ip)^(-npper) - 1
    realChangeSign(&temp3);                                    // temp3 = 1 - (1+ip)^(-npper)
  }


  // Numerator = -[PV + FV*(1+ip)^(-NPPER)] * ip
  realFMA(fv, &powerTerm, pv, &temp2, &ctxtTvm);
  realMultiply(&temp2, &ip, &numerator, &ctxtTvm);
  realChangeSign(&numerator);

  // Denominator = (1+ip*p) * [1-(1+ip)^(-NPPER)]
  realFMA(&ip, p, const_1, &temp2, &ctxtTvm);
  realMultiply(&temp2, &temp3, &denominator, &ctxtTvm);

  // Check for zero denominator
  if(decNumberIsZero((decNumber *)&denominator)) {
    return tvmRangeError(0);
  }

  // PMT = numerator / denominator
  realDivide(&numerator, &denominator, pmt, &ctxtTvm);

  return 0;
}

// Calculate Number of Payment Periods (NPPER)
// Case 1: PMT = 0: NPPER = ln(-FV/PV) / ln(1+ip)
// Case 2: PMT ≠ 0, ip ≠ 0: NPPER = ln(A/B) / ln(1+ip)
//         where A = -FV*ip + PMT*(1+ip*p)
//               B = PV*ip + PMT*(1+ip*p)
// Case 3: ip = 0: NPPER = -(PV + FV) / PMT
int calculateNPPER(const real_t *pv,
                   const real_t *fv,
                   const real_t *iPercentPerYear,
                   const real_t *paymentPerYear,
                   const real_t *pmt,
                   const real_t *compoundPerYear,
                   const real_t *p,
                   real_t *npper) {
  ensureTvmContext();
  real_t ip, temp1, temp2, a, b, ratio, lnRatio, lnBase;
  int error = 0;

  calculateEffectiveRate(iPercentPerYear, compoundPerYear, paymentPerYear, &ip, &error);
  if(error != 0) return error;

  // Check if ip ≈ 0 (use special formula)
  if(realCompareAbsLessThan(&ip, const_1e_37)) {
    // Case 3: NPPER = -(PV + FV) / PMT
    if(decNumberIsZero((decNumber *)pmt)) {
      return tvmRangeError(0);
    }
    realAdd(pv, fv, &temp1, &ctxtTvm);
    realDivide(&temp1, pmt, npper, &ctxtTvm);
    realSetNegativeSign(npper);
    return 0;
  }

  // Check if PMT = 0 (simple compound interest)
  if(decNumberIsZero((decNumber *)pmt)) {
    // Case 1: NPPER = ln(-FV/PV) / ln(1+ip)
    // Check for PV = 0 before division
    if(decNumberIsZero((decNumber *)pv)) {
      return tvmRangeError(7);
    }

    realDivide(fv, pv, &ratio, &ctxtTvm);
    realSetNegativeSign(&ratio);

    // Check for non-positive argument to ln
    if(!realIsPositive(&ratio)) {
      return tvmRangeError(4);
    }

    WP34S_Ln(&ratio, &lnRatio, &ctxtTvm);
    WP34S_Ln1P(&ip, &lnBase, &ctxtTvm);

    // Check for zero denominator (should not happen for valid ip, but check anyway)
    if(decNumberIsZero((decNumber *)&lnBase)) {
      return tvmRangeError(0);
    }

    realDivide(&lnRatio, &lnBase, npper, &ctxtTvm);
    return 0;
  }

  // Case 2: PMT ≠ 0, ip ≠ 0
  // Calculate A = -FV*ip + PMT*(1+ip*p)
  realMultiply(fv, &ip, &temp1, &ctxtTvm);
  realSetNegativeSign(&temp1);
  realFMA(&ip, p, const_1, &temp2, &ctxtTvm);
  realFMA(pmt, &temp2, &temp1, &a, &ctxtTvm);

  // Calculate B = PV*ip + PMT*(1+ip*p)
  realMultiply(pv, &ip, &temp1, &ctxtTvm);
  //realFMA(&ip, p, const_1, &temp2, &ctxtTvm);
  realFMA(pmt, &temp2, &temp1, &b, &ctxtTvm);

  // Check for zero denominator
  if(decNumberIsZero((decNumber *)&b)) {
    return tvmRangeError(0);
  }

  // Calculate ratio = A / B
  realDivide(&a, &b, &ratio, &ctxtTvm);

  // Check for non-positive argument to ln
  if(!realIsPositive(&ratio)) {
    return tvmRangeError(3);  // No solution
  }

  // NPPER = ln(A/B) / ln(1+ip)
  WP34S_Ln(&ratio, &lnRatio, &ctxtTvm);
  WP34S_Ln1P(&ip, &lnBase, &ctxtTvm);

  // Check for zero denominator
  if(decNumberIsZero((decNumber *)&lnBase)) {
    return tvmRangeError(0);
  }

  realDivide(&lnRatio, &lnBase, npper, &ctxtTvm);

  return 0;
}

// Calculate Payment Periods per Annum (PPER/a)
// From: ip = (1 + ic)^(CPER/a / PPER/a) - 1
// Where: ic = (I%/a / 100) / CPER/a
// Solve: PPER/a = CPER/a * ln(1 + ic) / ln(1 + ip)
// NOTE: This requires knowing the effective rate ip, which comes from
//       solving the main TVM equation. For simple cases (PMT=0), we can
//       calculate it directly. For complex cases, use I%/a solver first.
int calculatePPER(const real_t *pv,
                  const real_t *fv,
                  const real_t *iPercentPerYear,
                  const real_t *npper,
                  const real_t *pmt,
                  const real_t *compoundPerYear,
                  const real_t *p,
                  real_t *paymentPerYear) {
  ensureTvmContext();
  real_t ic, temp1, temp2, lnBase, lnTarget, ratio;
  real_t ip_effective;

  // Check for zero compounding frequency
  if(decNumberIsZero((decNumber *)compoundPerYear)) {
    return tvmRangeError(6);
  }

  // Calculate ic = (I%/a / 100) / CPER/a
  realDivide(iPercentPerYear, const_100, &ic, &ctxtTvm);
  realDivide(&ic, compoundPerYear, &ic, &ctxtTvm);

  // Calculate effective interest rate ip from the TVM parameters
  // For PMT = 0 (simple compound interest): ip = (-FV/PV)^(1/NPPER) - 1

  if(decNumberIsZero((decNumber *)pmt)) {
    // Simple case: compound interest only
    if(decNumberIsZero((decNumber *)pv)) {
      return tvmRangeError(7);
    }
    if(decNumberIsZero((decNumber *)npper)) {
      return tvmRangeError(2);
    }

    // ip = (-FV/PV)^(1/NPPER) - 1
    realDivide(fv, pv, &temp1, &ctxtTvm);
    realSetNegativeSign(&temp1);

    if(!realIsPositive(&temp1)) {
      return tvmRangeError(3);
    }

    realDivide(const_1, npper, &temp2, &ctxtTvm);
    {
      real_t temp_ln;
      WP34S_Ln(&temp1, &temp_ln, &ctxtTvm);
      realMultiply(&temp_ln, &temp2, &temp_ln, &ctxtTvm);
      WP34S_ExpM1(&temp_ln, &ip_effective, &ctxtTvm);
    }

  } else {
    // Complex case: with payments
    // This requires solving the full TVM equation for ip, which is iterative
    // User should use their I%/a solver first, then calculate PPER from result
    // For now, we'll return an error indicating this limitation
    return tvmRangeError(3);  // Use I%/a solver first for payment cases
  }

  // Check if ic ≈ 0
  if(realCompareAbsLessThan(&ic, const_1e_37)) {
    // When ic ≈ 0, any PPER/a works (indeterminate)
    return tvmRangeError(3);
  }

  // Check if ip ≈ ic (special case: PPER/a = CPER/a)
  realSubtract(&ip_effective, &ic, &temp1, &ctxtTvm);
  if(realCompareAbsLessThan(&temp1, const_1e_37)) {
    realCopy(compoundPerYear, paymentPerYear);
    return 0;
  }

  // General case: PPER/a = CPER/a * ln(1 + ic) / ln(1 + ip)
  realAdd(&ic, const_1, &temp1, &ctxtTvm);
  if(!realIsPositive(&temp1)) {
    return tvmRangeError(4);
  }
  WP34S_Ln1P(&ic, &lnBase, &ctxtTvm);

  realAdd(&ip_effective, const_1, &temp1, &ctxtTvm);
  if(!realIsPositive(&temp1)) {
    return tvmRangeError(4);
  }
  WP34S_Ln1P(&ip_effective, &lnTarget, &ctxtTvm);

  if(decNumberIsZero((decNumber *)&lnTarget)) {
    return tvmRangeError(0);
  }

  // PPER/a = CPER/a * ln(1+ic) / ln(1+ip)
  realDivide(&lnBase, &lnTarget, &ratio, &ctxtTvm);
  realMultiply(compoundPerYear, &ratio, paymentPerYear, &ctxtTvm);

  if(!realIsPositive(paymentPerYear)) {
    return tvmRangeError(3);
  }

  return 0;
}

// Calculate Compounding Periods per Annum (CPER/a)
// From: ip = (1 + ic)^(CPER/a / PPER/a) - 1
// Where: ic = (I%/a / 100) / CPER/a
// This is transcendental in CPER/a, requires iterative solution.
// NOTE: For simple cases (PMT=0), we calculate ip directly.
//       For complex cases, use I%/a solver first.
int calculateCPER(const real_t *pv,
                  const real_t *fv,
                  const real_t *iPercentPerYear,
                  const real_t *npper,
                  const real_t *paymentPerYear,
                  const real_t *pmt,
                  const real_t *p,
                  real_t *compoundPerYear) {
  ensureTvmContext();
  real_t ip, ic, temp1, temp2;
  real_t exponent, test_ip, error_val, tolerance;
  int iterations = 0;
  const int maxIterations = 100;

  // Check for zero payment frequency
  if(decNumberIsZero((decNumber *)paymentPerYear)) {
    return tvmRangeError(5);
  }

  // Calculate effective interest rate per payment period (ip)
  if(decNumberIsZero((decNumber *)pmt)) {
    // Simple case: compound interest only
    if(decNumberIsZero((decNumber *)pv)) {
      return tvmRangeError(7);
    }
    if(decNumberIsZero((decNumber *)npper)) {
      return tvmRangeError(2);
    }

    // ip = (-FV/PV)^(1/NPPER) - 1
    realDivide(fv, pv, &temp1, &ctxtTvm);
    realSetNegativeSign(&temp1);

    if(!realIsPositive(&temp1)) {
      return tvmRangeError(3);
    }

    realDivide(const_1, npper, &temp2, &ctxtTvm);
    {
      real_t temp_ln;
      WP34S_Ln(&temp1, &temp_ln, &ctxtTvm);
      realMultiply(&temp_ln, &temp2, &temp_ln, &ctxtTvm);
      WP34S_ExpM1(&temp_ln, &ip, &ctxtTvm);
    }

  } else {
    // Complex case - use I%/a solver first
    return tvmRangeError(9);
  }

  // Iterative solution for CPER/a
  // Equation: (1 + (I%/a/100)/CPER)^(CPER/PPER) - 1 = ip

  stringToReal("1e-36", &tolerance, &ctxtTvm);

  // Initial guess: CPER/a = PPER/a
  realCopy(paymentPerYear, compoundPerYear);

  for(iterations = 0; iterations < maxIterations; iterations++) {
    // ic = (I%/a / 100) / CPER/a
    realDivide(iPercentPerYear, const_100, &ic, &ctxtTvm);
    realDivide(&ic, compoundPerYear, &ic, &ctxtTvm);

    // exponent = CPER/a / PPER/a
    realDivide(compoundPerYear, paymentPerYear, &exponent, &ctxtTvm);

    // test_ip = (1 + ic)^exponent - 1
    {
      real_t temp_exp;
      WP34S_Ln1P(&ic, &temp_exp, &ctxtTvm);                    // temp_exp = ln(1 + ic)
      realMultiply(&temp_exp, &exponent, &temp_exp, &ctxtTvm); // temp_exp = exponent * ln(1 + ic)
      WP34S_ExpM1(&temp_exp, &test_ip, &ctxtTvm);              // test_ip = (1+ic)^exponent - 1
    }

    // error = test_ip - ip
    realSubtract(&test_ip, &ip, &error_val, &ctxtTvm);

    // Check convergence
    if(realCompareAbsLessThan(&error_val, &tolerance)) {
      return 0;
    }

    {
      // Newton-Raphson update with damping
      real_t temp2;
      realDivide(&error_val, &ip, &temp1, &ctxtTvm);
      realChangeSign(&temp1);
      realFMA(&temp1, const_1on2, const_1, &temp2, &ctxtTvm);
      realMultiply(compoundPerYear, &temp2, compoundPerYear, &ctxtTvm);
    }
    // Keep CPER/a positive
    if(!realIsPositive(compoundPerYear)) {
      realCopy(paymentPerYear, compoundPerYear);
    }
  }

  // Failed to converge
  return tvmRangeError(3);
}



  TO_QSPI static const char bugScreenNotForTvmVar[] = "In function solveTvmVariable51: this variable is not intended for TVM application!";


// Solve for the specified TVM variable, returns: 0 on success, error code on failure
int solveTvmVariable51(uint16_t variable) {
  ensureTvmContext();
  real_t fv, iA, nPer, pperA, cperA, pmt, pv, p;
  real_t result;
  int error = 0;

  real34ToReal(REGISTER_REAL34_DATA(RESERVED_VARIABLE_FV),      &fv);     // Future value
  real34ToReal(REGISTER_REAL34_DATA(RESERVED_VARIABLE_IPONA),   &iA);     // Interest percentage per annum
  real34ToReal(REGISTER_REAL34_DATA(RESERVED_VARIABLE_NPPER),   &nPer);   // Number of periods
  real34ToReal(REGISTER_REAL34_DATA(RESERVED_VARIABLE_PPERONA), &pperA);  // Payment periods per annum
  real34ToReal(REGISTER_REAL34_DATA(RESERVED_VARIABLE_CPERONA), &cperA);  // Compounding periods per annum
  real34ToReal(REGISTER_REAL34_DATA(RESERVED_VARIABLE_PMT),     &pmt);    // Payment
  real34ToReal(REGISTER_REAL34_DATA(RESERVED_VARIABLE_PV),      &pv);     // Present value

  if(getSystemFlag(FLAG_ENDPMT)) {
    int32ToReal(0, &p);  // END mode: p=0
  } else {
    int32ToReal(1, &p);  // BEGIN mode: p=1
  }

  switch(variable) {
    case RESERVED_VARIABLE_PV:
      error = calculatePV(&fv, &iA, &nPer, &pperA, &pmt, &cperA, &p, &result);
      if(!error) realToReal34(&result, REGISTER_REAL34_DATA(RESERVED_VARIABLE_PV)      );     // Present value
      break;

    case RESERVED_VARIABLE_FV:
      error = calculateFV(&pv, &iA, &nPer, &pperA, &pmt, &cperA, &p, &result);
      if(!error) realToReal34(&result, REGISTER_REAL34_DATA(RESERVED_VARIABLE_FV)      );     // Future value
      break;

    case RESERVED_VARIABLE_PMT:
      error = calculatePMT(&pv, &fv, &iA, &nPer, &pperA, &cperA, &p, &result);
      if(!error) realToReal34(&result, REGISTER_REAL34_DATA(RESERVED_VARIABLE_PMT)     );    // Payment
      break;

    case RESERVED_VARIABLE_NPPER:
      error = calculateNPPER(&pv, &fv, &iA, &pperA, &pmt, &cperA, &p, &result);
      if(!error) realToReal34(&result, REGISTER_REAL34_DATA(RESERVED_VARIABLE_NPPER)   );   // Number of periods
      break;

    case RESERVED_VARIABLE_PPERONA:
      error = calculatePPER(&pv, &fv, &iA, &nPer, &pmt, &cperA, &p, &result);
      if(!error) realToReal34(&result, REGISTER_REAL34_DATA(RESERVED_VARIABLE_PPERONA) );  // Payment periods per annum
      break;

    case RESERVED_VARIABLE_CPERONA:
      error = calculateCPER(&pv, &fv, &iA, &nPer, &pperA, &pmt, &p, &result);
      if(!error) realToReal34(&result, REGISTER_REAL34_DATA(RESERVED_VARIABLE_CPERONA) );  // Compounding periods per annum
      break;

    default:
      displayBugScreen(bugScreenNotForTvmVar);
      return 8;
  }

  if(error != 0) {
    //Not stopping for an error, but letting it through to the old solver for erroring and/or solving
    //displayCalcErrorMessage(ERROR_NO_ROOT_FOUND, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function solveTvmVariable51:", tvmErrorMessages[error], " Cannot compute TVM equation with current parameters", NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return error;
  }

  reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
  convertRealToReal34ResultRegister(&result, REGISTER_X);
  setSystemFlag(FLAG_ASLIFT);
  return 0;
}
#endif //OPTION_TVM_FORMULAS



#if defined(TESTSUITE_BUILD)
  #define testing true
#else
  #define testing false
#endif //TESTSUITE_BUILD


  TO_QSPI static const char bugScreenNotForTvm[] = "In function fnTvmVar: this variable is not intended for TVM application!";

void fnTvmVar(uint16_t variable) {
    ensureTvmContext();
    #if defined (PC_BUILD) && defined (TVMDEBUG2)
      printf("fnTvmVar solver starting with: variable = %d ", variable);
      switch(variable){
        case RESERVED_VARIABLE_FV      : printf("RESERVED_VARIABLE_FV     \n");break;
        case RESERVED_VARIABLE_IPONA   : printf("RESERVED_VARIABLE_IPONA  \n");break;
        case RESERVED_VARIABLE_NPPER   : printf("RESERVED_VARIABLE_NPPER  \n");break;
        case RESERVED_VARIABLE_PPERONA : printf("RESERVED_VARIABLE_PPERONA\n");break;
        case RESERVED_VARIABLE_CPERONA : printf("RESERVED_VARIABLE_CPERONA\n");break;
        case RESERVED_VARIABLE_PMT     : printf("RESERVED_VARIABLE_PMT    \n");break;
        case RESERVED_VARIABLE_PV      : printf("RESERVED_VARIABLE_PV     \n");break;
        default:;
      }
      printRegisterToConsole(RESERVED_VARIABLE_NPPER, "N=", ", ");
      printRegisterToConsole(RESERVED_VARIABLE_IPONA, "I%/a=", ", ");
      printRegisterToConsole(RESERVED_VARIABLE_PV, "PV=", ", ");
      printRegisterToConsole(RESERVED_VARIABLE_PMT, "PMT=", ", ");
      printRegisterToConsole(RESERVED_VARIABLE_FV, "FV=", ", ");
      printRegisterToConsole(RESERVED_VARIABLE_PPERONA, "pp/a=", ", ");
      printRegisterToConsole(RESERVED_VARIABLE_CPERONA, "cp/a=", ", ");
      printf("END=%d\n", getSystemFlag(FLAG_ENDPMT));
    #endif

    switch(variable) {
      case RESERVED_VARIABLE_FV:
      case RESERVED_VARIABLE_IPONA:
      case RESERVED_VARIABLE_NPPER:
      case RESERVED_VARIABLE_PPERONA:
      case RESERVED_VARIABLE_CPERONA:
      case RESERVED_VARIABLE_PMT:
      case RESERVED_VARIABLE_PV: {
        currentSolverStatus |= SOLVER_STATUS_TVM_APPLICATION;
        currentSolverVariable = variable;
        tvmIKnown = false;

        /* Calculate */
        if(currentSolverStatus & SOLVER_STATUS_READY_TO_EXECUTE || programRunStop == PGM_RUNNING || programRunStop == PGM_PAUSED || testing) {
          real34_t y, x, resZ, resY, resX;
          real34Copy(const_0, &resZ);
          real34Copy(const_0, &resY);
          real34Copy(const_1, &resX);
          saveForUndo();
          thereIsSomethingToUndo = true;
          liftStack();
          tvmIKnown = false;

          #if defined(OPTION_TVM_FORMULAS)
            if(variable != RESERVED_VARIABLE_IPONA) {
              int err = solveTvmVariable51(variable);
              if( err == 0) {
                #if defined(PC_BUILD) && defined (TVMDEBUG2)
                  printf("Success analytical TVM\n");
                #endif //PC_BUILD
                temporaryInformation = TI_SOLVER_VARIABLE;
                return;   // Try analytic solution, if successful, return
              }
              else {
                lastErrorCode = 0;  // Failure in analytical solver section, error is on the PC screen but continue to retry using the old solver without erroring
                #if defined(PC_BUILD) && defined (TVMDEBUG2)
                  printf("Clearing analytical TVM error to continue to iterative solver\n");
                #endif //PC_BUILD
              }
            }
          #endif //OPTION_TVM_FORMULAS

          switch(variable) {
            case RESERVED_VARIABLE_IPONA:
            case RESERVED_VARIABLE_PPERONA:
            case RESERVED_VARIABLE_CPERONA: {
              tvmIChanges = true;
              break;
            }
            default: {
              tvmIChanges = false;
            }
          }

          real34Multiply(REGISTER_REAL34_DATA(variable), const34_2, &y);
          real34Multiply(REGISTER_REAL34_DATA(variable), const34_1on2, &x);

          switch(variable) {
            case RESERVED_VARIABLE_PV: {
              if(real34CompareAbsLessThan(REGISTER_REAL34_DATA(RESERVED_VARIABLE_PV),const34_1e_4)) {
                real34Multiply(REGISTER_REAL34_DATA(RESERVED_VARIABLE_FV), const34_2, &y);
                real34Multiply(REGISTER_REAL34_DATA(RESERVED_VARIABLE_FV), const34_1on2, &x);
              }
              break;
            }
            case RESERVED_VARIABLE_FV: {
              if(real34CompareAbsLessThan(REGISTER_REAL34_DATA(RESERVED_VARIABLE_FV),const34_1e_4)) {
                real34Multiply(REGISTER_REAL34_DATA(RESERVED_VARIABLE_PV), const34_2, &y);
                real34Multiply(REGISTER_REAL34_DATA(RESERVED_VARIABLE_PV), const34_1on2, &x);
              }
              break;
            }
            case RESERVED_VARIABLE_IPONA: {
              if(real34CompareAbsLessThan(REGISTER_REAL34_DATA(variable), const34_1on10)) {       //if interest rate p.a. < 0.1% then default to a reasonable 3% to 7% strarting condition
                real34Copy(const34_3, &y); //3
                real34Copy(const34_7, &x); //7
              }
              break;
            }

            case RESERVED_VARIABLE_NPPER: {
              if(real34CompareLessThan(REGISTER_REAL34_DATA(variable), const34_1)) {
                real34Copy(const34_2, &y);
                real34Copy(const34_1, &x);
              }
              break;
            }

            case RESERVED_VARIABLE_CPERONA:
            case RESERVED_VARIABLE_PPERONA: {
              if(real34CompareLessThan(REGISTER_REAL34_DATA(variable), const34_3)) {
                real34Copy(const34_24, &y);
                real34Copy(const34_9, &x);
              }
              break;
            }

            case RESERVED_VARIABLE_PMT: {
              //no special x y cases
              break;
            }
            default:break;
          }

          if((variable == RESERVED_VARIABLE_PV || variable == RESERVED_VARIABLE_FV) &&
            !real34IsZero(REGISTER_REAL34_DATA(RESERVED_VARIABLE_PV)) &&
            !real34IsZero(REGISTER_REAL34_DATA(RESERVED_VARIABLE_FV)) &&
            real34IsNegative(REGISTER_REAL34_DATA(RESERVED_VARIABLE_PV)) == real34IsNegative(REGISTER_REAL34_DATA(RESERVED_VARIABLE_FV))) {
              real34ChangeSign(&y);
              real34ChangeSign(&x);
          }

          if(real34CompareAbsLessThan(&x,const34_1e_4) && real34CompareAbsLessThan(&y,const34_1e_4)) { //still after all the tries, if x & y are still both below 0.0001, then change x to 1 (keeping y = 0)
            real34Copy(const34_1, &x);
          }

          uint16_t iter = 0;
          real34_t xx, yy;

          #define nIter 6
          while(iter++ < nIter && !real34CompareEqual(&resX, &resY)) {
            real34Copy(&x, &xx);
            real34Copy(&y, &yy);
            #if defined (PC_BUILD) && defined (TVMDEBUG2)
              printReal34ToConsole(&x,"iter x:","\n");
              printReal34ToConsole(&y,"iter y:","\n");
            #endif //PC_BUILD
            uint16_t solveResult = solver(variable, &y, &x, &resZ, &resY, &resX);
            #if defined(PC_BUILD) && defined (TVMDEBUG2)
              printf("Overall solve iteration: iter=%u/%u solveResult=%u\n",iter, nIter, solveResult);
            #endif //PC_BUILD
            if(solveResult == SOLVER_RESULT_NORMAL) {
              #if defined(PC_BUILD)  && defined (TVMDEBUG2)
                printReal34ToConsole(&resZ,"  solver results: resZ:","\n");
                printReal34ToConsole(&resY,"  solver results: resY:","\n");
                printReal34ToConsole(&resX,"  solver results: resX:","\n");
              #endif //PC_BUILD
              reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
              real34Copy(&resX, REGISTER_REAL34_DATA(REGISTER_X));
              temporaryInformation = TI_SOLVER_VARIABLE;
              thereIsSomethingToUndo = false;
              break;
            }
            else if(solveResult == SOLVER_RESULT_CONSTANT) { // in cases with prevented infinite loop
              iter = nIter;
              break;
            }
            else if(solveResult == SOLVER_RESULT_ABORTED) { // solver aborted
              iter = nIter;
              if(exitKeyWaiting()) {
                break;
              }
            }
            else {
              if(real34IsNegative(&xx)) {
                real34Subtract(&xx, const34_2, &x);
              } else {
                real34Add(&xx, const34_1, &x);
              }
              if(real34IsNegative(&yy)) {
                real34Subtract(&yy, const34_1on2, &y);
              }
              else {
                real34Add(&yy, const34_2, &y);
              }
              real34Multiply(&x, const34_3, &x);
              if((iter+1)%3 == 0) {
                real34ChangeSign(&x);
              }
              real34Multiply(&y, const34_2, &y);
            }
          }

          if(iter == nIter) {
            if(lastErrorCode != ERROR_SOLVER_ABORT) {
              displayCalcErrorMessage(ERROR_NO_ROOT_FOUND, ERR_REGISTER_LINE, REGISTER_X);
            }
            #if (EXTRA_INFO_ON_CALC_ERROR == 1)
              moreInfoOnError("In function fnTvmVar:", "cannot compute TVM equation", "with current parameters", NULL);
            #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          }

          adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
          setSystemFlag(FLAG_ASLIFT);
        }

        /* Store parameters */
        else {
          fnToReal(NOPARAM);
          if(lastErrorCode == ERROR_NONE) {
            reallyRunFunction(ITM_STO, variable);
            if(variable == RESERVED_VARIABLE_PPERONA) {
              reallyRunFunction(ITM_STO, RESERVED_VARIABLE_CPERONA);
            }
            currentSolverStatus |= SOLVER_STATUS_READY_TO_EXECUTE;
            temporaryInformation = TI_SOLVER_VARIABLE;
          }
          adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
          setSystemFlag(FLAG_ASLIFT);
        }
        break;
      }

      default: {
        displayBugScreen(bugScreenNotForTvm);
      }
    }
}



void fnTvmBeginMode(uint16_t unusedButMandatoryParameter) {
  clearSystemFlag(FLAG_ENDPMT);
}

void fnTvmEndMode(uint16_t unusedButMandatoryParameter) {
  setSystemFlag(FLAG_ENDPMT);
}


void fnEff(uint16_t unusedButMandatoryParameter) {
  ensureTvmContext();
  real_t iA, cperA, tmp;
  //no need to use tvmIKnown or tvmIChanges, as this is a simplistic output only, which takes the current cperA & iA and produces the effective rate. There is no situation where there is no values in these
    //   EFF = 100({[iA / 100cperA] + 1} ^ cperA - 1)

  if(getRegisterAsRealQuiet(RESERVED_VARIABLE_IPONA, &iA) &&
     getRegisterAsRealQuiet(RESERVED_VARIABLE_CPERONA, &cperA) &&
     !realIsZero(&cperA)) {

    saveForUndo();
    thereIsSomethingToUndo = true;
    liftStack();

    iA.exponent -= 2; // iA = iA / 100
    realDivide(&iA, &cperA, &tmp, &ctxtReal39);
    {
      real_t temp2;
      WP34S_Ln1P(&tmp, &temp2, &ctxtTvm);              // temp2 = ln(1 + tmp)
      realMultiply(&temp2, &cperA, &temp2, &ctxtTvm);  // temp2 = cperA * ln(1 + tmp)
      WP34S_ExpM1(&temp2, &tmp, &ctxtTvm);             // tmp = exp(cperA * ln(1 + tmp)) - 1
    }
    tmp.exponent += 2; // tmp = tmp * 100

    reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
    convertRealToReal34ResultRegister(&tmp, REGISTER_X);
    temporaryInformation = TI_TVM_EFF;
  } else {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function fnEff:", "cannot compute EFF%/a ", "with parameter cp/a = 0", NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


void fnEffToI(uint16_t unusedButMandatoryParameter) {
  ensureTvmContext();
  real_t iEFF, tmp, cperA, t2;
  //no need to use tvmIKnown or tvmIChanges, as this is a simplistic output only, which takes the current cperA & iA and produces the effective rate. There is no situation where there is no values in these
    // iA = {[(EFF / 100 + 1) ^ (1/cperA) ] - 1 } 100 * cperA

  if(getRegisterAsRealQuiet(REGISTER_X, &iEFF) &&
     getRegisterAsRealQuiet(RESERVED_VARIABLE_CPERONA, &cperA) &&
     !realIsZero(&cperA) &&
     realIsPositive(&iEFF)) {

    saveForUndo();
    thereIsSomethingToUndo = true;

    iEFF.exponent -= 2; // iEFF = iEFF / 100
    WP34S_Ln1P(&iEFF, &tmp, &ctxtReal39);

    realDivide(&tmp, &cperA, &t2, &ctxtReal39);
    WP34S_ExpM1(&t2, &tmp, &ctxtReal39);
    tmp.exponent += 2; // tmp = tmp * 100
    realMultiply(&tmp, &cperA, &tmp, &ctxtReal39);

    realToReal34(&tmp, REGISTER_REAL34_DATA(RESERVED_VARIABLE_IPONA));
    reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
    convertRealToReal34ResultRegister(&tmp, REGISTER_X);

    temporaryInformation = TI_TVM_IA;
  } else {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function fnEffToI:", "cannot compute I%/a ", "with parameters n = 0 & EFF/a < 0 ", NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}




void tvmEquation(calcRegister_t variable, real_t *ioVal, real_t *derivative) {
  // printf("tvmEquation start: variable = %d\n", variable);
  // printRealToConsole(ioVal,"ioVal: "," derivative: ");
  // if(derivative != NULL) printRealToConsole(ioVal,"ioVal: ","\n"); else printf("NULL\n");
  // printf("Context ctxtTvm         : %d digits\n", ctxtTvm.digits);
  // printf("Context ctxtTvmHi       : %d digits\n", ctxtTvmHi.digits);
  // printf("Context ctxtSolverTvmHi : %d digits\n", ctxtSolverTvmHi.digits);
  // printf("Context ctxtSolverTvmInv: %d digits\n", ctxtSolverTvmInv.digits);
  // switch(variable){
  //   case RESERVED_VARIABLE_FV      : printf("RESERVED_VARIABLE_FV     \n");break;
  //   case RESERVED_VARIABLE_IPONA   : printf("RESERVED_VARIABLE_IPONA  \n");break;
  //   case RESERVED_VARIABLE_NPPER   : printf("RESERVED_VARIABLE_NPPER  \n");break;
  //   case RESERVED_VARIABLE_PPERONA : printf("RESERVED_VARIABLE_PPERONA\n");break;
  //   case RESERVED_VARIABLE_CPERONA : printf("RESERVED_VARIABLE_CPERONA\n");break;
  //   case RESERVED_VARIABLE_PMT     : printf("RESERVED_VARIABLE_PMT    \n");break;
  //   case RESERVED_VARIABLE_PV      : printf("RESERVED_VARIABLE_PV     \n");break;
  //   default:;
  // }

  // Force recalculation of i when computing derivative
  if(derivative != NULL && variable == RESERVED_VARIABLE_IPONA) {
    tvmIKnown = false;
  }

  ensureTvmContext();
  real_t fv, iA, nPer, pperA, cperA, pmt, pv;
  real_t val, r, k;
  static real_t i;

  if(variable != RESERVED_VARIABLE_FV     ) {real34ToReal(REGISTER_REAL34_DATA(RESERVED_VARIABLE_FV),      &fv);   }  //future value
  if(variable != RESERVED_VARIABLE_IPONA  ) {real34ToReal(REGISTER_REAL34_DATA(RESERVED_VARIABLE_IPONA),   &iA);   }  //interest percentage per annum
  if(variable != RESERVED_VARIABLE_NPPER  ) {real34ToReal(REGISTER_REAL34_DATA(RESERVED_VARIABLE_NPPER),   &nPer); }  //number of periods
  if(variable != RESERVED_VARIABLE_PPERONA) {real34ToReal(REGISTER_REAL34_DATA(RESERVED_VARIABLE_PPERONA), &pperA);}  //payment periods per annum
  if(variable != RESERVED_VARIABLE_CPERONA) {real34ToReal(REGISTER_REAL34_DATA(RESERVED_VARIABLE_CPERONA), &cperA);}  //compounding periods per annum
  if(variable != RESERVED_VARIABLE_PMT    ) {real34ToReal(REGISTER_REAL34_DATA(RESERVED_VARIABLE_PMT),     &pmt);  }  //payment
  if(variable != RESERVED_VARIABLE_PV     ) {real34ToReal(REGISTER_REAL34_DATA(RESERVED_VARIABLE_PV),      &pv);   }  //present value

  // override the new real_t variable being solved, with the incoming full digit real_t value from the solver, overriding the priod 34-digit from the reserved value.
  switch(variable) {
    case RESERVED_VARIABLE_FV:     realCopy(ioVal, &fv);    break;
    case RESERVED_VARIABLE_IPONA:  realCopy(ioVal, &iA);    break;
    case RESERVED_VARIABLE_NPPER:  realCopy(ioVal, &nPer);  break;
    case RESERVED_VARIABLE_PPERONA:realCopy(ioVal, &pperA); break;
    case RESERVED_VARIABLE_CPERONA:realCopy(ioVal, &cperA); break;
    case RESERVED_VARIABLE_PMT:    realCopy(ioVal, &pmt);   break;
    case RESERVED_VARIABLE_PV:     realCopy(ioVal, &pv);    break;
  }

  /*
    The plan is to find an interest rate iM which,
    when compounded pperA times in a year, gives iAER.
    (AER stands for annual effective rate.)
    iAER can be found like this:
    1 + iAER = (1 + (iA/100)/cperA)^cperA
    Also,
    1 + iAER = (1 + iM) ^ pperA.
    So
    iM = (1 + (iA/100)/cperA)^(cperA/pperA) - 1.
    If cperA = pperA this is just iM = (iA/100)/pperA.

    Define r = cperA / pperA. r = 1 corresponds to the "normal" case.
    In terms of r,
    iM = (1 + iA/100/pperA/r)^r - 1.
    This reduces to iM = (iA/100)/pperA when r=1.
    When pperA is set cperA is also set to the same value. So if no value
    is entered for cperA, r will be 1. We can test for this and short-circuit
    the calculation.
    If no value is entered for pperA - perhaps it's being solved for so it's zero
    Can I avoid repeating this calculation each time the TVM equation is called?
    The calculation of i depends on: iA; pperA; cperA. iA could change if i is being solved for;
    similarly, the other two could change as well if being solved for.
    This function doesn't know what is being solved for.
    It takes a lot of iterations to converge but without the calculation of i each time
    these iterations pass much more quickly. So I do need to do something here.
   */

  if((!tvmIKnown) || (tvmIChanges)) { // if i hasn't been found yet or i changes each time
    realDivide(&iA, const_100, &i, &ctxtTvm);
    realDivide(&i, &pperA, &i, &ctxtTvm);
    // i is now (iA / 100) / pperA.
    // This is the "normal" value of i when cperA = pperA.

    realDivide(&cperA, &pperA, &r, &ctxtTvm); // r = cperA / pperA

    if(!(realIsZero(&r) || realCompareEqual(const_1, &r)) ) { // not normal case
      realDivide(&i, &r, &i, &ctxtTvm);
      {
        // Converted to: (1+i)^r = exp(r * ln(1+i))
        real_t temp;
        WP34S_Ln1P(&i, &temp, &ctxtSolverTvmHi);             // temp = ln(1 + i)
        realMultiply(&temp, &r, &temp, &ctxtSolverTvmHi);    // temp = r * ln(1 + i)
        WP34S_ExpM1(&temp, &i, &ctxtSolverTvmHi);            // i = exp(r * ln(1 + i)) - 1
      } // i = (1 + (i/pperA)/r)^r - 1
    }
    tvmIKnown = true;
  }

  // early return for i = 0: exact limit avoids 0/0 in annuity; f = -PV + n*PMT - FV (k -> 1 for both modes)
  if(realIsZero(&i)) {
    realFMA(&nPer, &pmt, &pv, &val, &ctxtTvm);
    realSubtract(&val, &fv, &val, &ctxtTvm);
    realCopy(&val, ioVal);
    // result returned via ioVal; X-register write commented out (may be needed on solver abort)
    //reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
    //convertRealToReal34ResultRegister(&val, REGISTER_X);
    #if defined(OPTION_TVM_NEWTON)
    if(derivative != NULL) {
      switch(variable) {
        case RESERVED_VARIABLE_PMT:   realCopy(&nPer,   derivative); break;  // df/dPMT = N
        case RESERVED_VARIABLE_PV:    realCopy(const_1,  derivative); break;  // df/dPV = 1
        case RESERVED_VARIABLE_FV:    realCopy(const__1, derivative); break;  // df/dFV = -1
        case RESERVED_VARIABLE_NPPER: realCopy(&pmt,    derivative); break;  // df/dN = PMT
        default:                      realCopy(const_NaN,derivative); break;  // IPONA: NaN -> Brent fallback
      }
    }
    #endif
    return;
  }

  {
    // Compute (1+i)^(-nPer) and 1-(1+i)^(-nPer) avoiding cancellation
    real_t temp, neg_nPer, i1negN, oneMinusI1negN;
    realCopy(&nPer, &neg_nPer);
    realChangeSign(&neg_nPer);
    WP34S_Ln1P(&i, &temp, &ctxtSolverTvmHi);
    realMultiply(&temp, &neg_nPer, &temp, &ctxtSolverTvmHi);

    doubleExp(&temp, &i1negN, &oneMinusI1negN, &ctxtSolverTvmHi);
    //realExp(&temp, &i1negN, &ctxtSolverTvmHi);  // For FV term
    //WP34S_ExpM1(&temp, &oneMinusI1negN, &ctxtSolverTvmHi);  // For PMT term
    realChangeSign(&oneMinusI1negN);  // oneMinusI1negN = 1-(1+i)^(-n)

    // Save k for derivative
    if(getSystemFlag(FLAG_ENDPMT)) {
      realCopy(const_1, &k);
    }
    else {
      realAdd(const_1, &i, &k, &ctxtTvm);
    }

    // f(i) = PV + PMT * k * [1 - (1+i)^(-n)] / i + FV * (1+i)^(-n)
    realCopy(&pv, &val);

    real_t term2;
    realCopy(&oneMinusI1negN, &term2);
    realDivide(&term2, &i, &term2, &ctxtSolverTvmInv);
    realMultiply(&term2, &k, &term2, &ctxtTvm);
    realFMA(&term2, &pmt, &val, &val, &ctxtTvm);
    realFMA(&fv, &i1negN, &val, &val, &ctxtTvm);
  }
  realCopy(&val, ioVal);
  // result returned via ioVal; X-register write commented out (may be needed on solver abort)
  //reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
  //convertRealToReal34ResultRegister(&val, REGISTER_X);

#if defined(OPTION_TVM_NEWTON)
    if(derivative != NULL && variable == RESERVED_VARIABLE_IPONA) {
      real_t one_plus_i, one_plus_i_neg_N, one_plus_i_neg_N_m1;
      real_t term1, term2, factor_deriv, i_squared;
      realAdd(const_1, &i, &one_plus_i, &ctxtTvm);
      // Compute (1+i)^(-N) and (1+i)^(-N-1)
      real_t temp, neg_nPer, one_minus;
      realCopy(&nPer, &neg_nPer);
      realChangeSign(&neg_nPer);
      WP34S_Ln1P(&i, &temp, &ctxtSolverTvmHi);
      realMultiply(&temp, &neg_nPer, &temp, &ctxtSolverTvmHi);
      doubleExp(&temp, &one_plus_i_neg_N, &one_minus, &ctxtSolverTvmHi);
      //realExp(&temp, &one_plus_i_neg_N, &ctxtSolverTvmHi);
      //WP34S_ExpM1(&temp, &one_minus, &ctxtSolverTvmHi);
      realChangeSign(&one_minus);
      realDivide(&one_plus_i_neg_N, &one_plus_i, &one_plus_i_neg_N_m1, &ctxtTvm);
      // term1 = N * (1+i)^(-N-1) / i
      realMultiply(&nPer, &one_plus_i_neg_N_m1, &term1, &ctxtTvm);
      realDivide(&term1, &i, &term1, &ctxtSolverTvmInv);
      // term2 = [1 - (1+i)^(-N)] / i^2
      realMultiply(&i, &i, &i_squared, &ctxtSolverTvmInv);
      realDivide(&one_minus, &i_squared, &term2, &ctxtSolverTvmInv);
      // factor_deriv = term1 - term2
      realSubtract(&term1, &term2, &factor_deriv, &ctxtTvm);
      // For BEGIN: factor_deriv = factor_deriv * (1+i) + (1-(1+i)^(-N))/i
      if(!getSystemFlag(FLAG_ENDPMT)) {
        realMultiply(&factor_deriv, &one_plus_i, &factor_deriv, &ctxtTvm);
        real_t extra;
        realDivide(&one_minus, &i, &extra, &ctxtSolverTvmInv);
        realAdd(&factor_deriv, &extra, &factor_deriv, &ctxtTvm);
      }
      // derivative = PMT * factor_deriv - N * FV * (1+i)^(-N-1)
      realMultiply(&pmt, &factor_deriv, derivative, &ctxtTvm);
      real_t fv_term;
      realMultiply(&nPer, &fv, &fv_term, &ctxtTvm);
      realChangeSign(&fv_term);
      realFMA(&fv_term, &one_plus_i_neg_N_m1, derivative, derivative, &ctxtTvm);
      // Scale derivative for I%/a variable: df/d(I%/a) = df/di × 1/(100×pperA)
      realDivide(derivative, const_100, derivative, &ctxtTvm);
      realDivide(derivative, &pperA, derivative, &ctxtTvm);
    }
    
    if(derivative != NULL && variable == RESERVED_VARIABLE_NPPER) {
      // df/dN = (1+i)^(-N) * ln(1+i) * [PMT*k/i - FV]
      real_t one_plus_i_neg_N, ln1pi, pmt_term;

      // Compute (1+i)^(-N)
      real_t temp, neg_nPer;
      realCopy(&nPer, &neg_nPer);
      realChangeSign(&neg_nPer);
      WP34S_Ln1P(&i, &ln1pi, &ctxtSolverTvmHi);
      realMultiply(&ln1pi, &neg_nPer, &temp, &ctxtSolverTvmHi);
      realExp(&temp, &one_plus_i_neg_N, &ctxtSolverTvmHi);
      // pmt_term = PMT*k/i - FV
      realDivide(&pmt, &i, &pmt_term, &ctxtSolverTvmInv);
      realChangeSign(&fv);
      realFMA(&pmt_term, &k, &fv, &pmt_term, &ctxtTvm);
      // derivative = (1+i)^(-N) * ln(1+i) * pmt_term
      realMultiply(&one_plus_i_neg_N, &ln1pi, derivative, &ctxtTvm);
      realMultiply(derivative, &pmt_term, derivative, &ctxtTvm);
    }
    
    if(derivative != NULL && variable == RESERVED_VARIABLE_PV) {
      // df/dPV = 1
      realCopy(const_1, derivative);
    }
    
    if(derivative != NULL && variable == RESERVED_VARIABLE_PMT) {
      // df/dPMT = k * [1-(1+i)^(-N)]/i
      real_t one_minus;

      // Compute (1+i)^(-N)
      real_t temp, neg_nPer;
      realCopy(&nPer, &neg_nPer);
      realChangeSign(&neg_nPer);
      WP34S_Ln1P(&i, &temp, &ctxtSolverTvmHi);
      realMultiply(&temp, &neg_nPer, &temp, &ctxtSolverTvmHi);
      // derivative = k * [1-(1+i)^(-N)]/i
      WP34S_ExpM1(&temp, &one_minus, &ctxtSolverTvmHi);
      realChangeSign(&one_minus);
      realDivide(&one_minus, &i, derivative, &ctxtSolverTvmInv);
      realMultiply(derivative, &k, derivative, &ctxtTvm);
    }
    
    if(derivative != NULL && variable == RESERVED_VARIABLE_FV) {
      // df/dFV = (1+i)^(-N)
      real_t temp, neg_nPer;
      realCopy(&nPer, &neg_nPer);
      realChangeSign(&neg_nPer);
      WP34S_Ln1P(&i, &temp, &ctxtSolverTvmHi);
      realMultiply(&temp, &neg_nPer, &temp, &ctxtSolverTvmHi);
      realExp(&temp, derivative, &ctxtSolverTvmHi);
    }
  #endif //OPTION_TVM_NEWTON

}

