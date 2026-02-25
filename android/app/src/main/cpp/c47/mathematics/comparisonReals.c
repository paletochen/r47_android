// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file comparisonReals.c
 ***********************************************/

#include "c47.h"

void convergenceTolerence(real_t *tol)
{
  realCopy(const_1, tol);
  tol->exponent -= (significantDigits == 0 || significantDigits >= 32) ? 32 : significantDigits;
}

void irfractionTolerence(int32_t ii, real_t *tol)
{
  int32ToReal((int32_t)ii,tol);
  tol->exponent -= ((fractionDigits == 0 || fractionDigits >= 34) ? 34 : fractionDigits);
}

void fractionTolerence(real_t *tol)
{
  irfractionTolerence(2, tol);
}


// Helper to make the exponent -99999 when the input is 0
// This is used where the exponent is used to determine convergence close to 0, hence 0 is seen as 10^-99999
int32_t realGetExponentComp(const real_t *val) {
  if(realIsZero(val)) {
    return -999999;
  }
  return realGetExponent(val);
}


bool_t real34CompareAbsGreaterThan(const real34_t *number1, const real34_t *number2) {
  real34_t num1, num2;

  real34CopyAbs(number1, &num1);
  real34CopyAbs(number2, &num2);
  real34Compare(&num1, &num2, &num2);
  return real34IsPositive(&num2) && !real34IsZero(&num2);
}



bool_t real34CompareAbsLessThan(const real34_t *number1, const real34_t *number2) {
  real34_t num1, num2;

  real34CopyAbs(number1, &num1);
  real34CopyAbs(number2, &num2);
  real34Compare(&num1, &num2, &num2);
  return real34IsNegative(&num2) && !real34IsZero(&num2);
}



bool_t real34CompareEqual(const real34_t *number1, const real34_t *number2) {
  real34_t compare;

  real34Compare(number1, number2, &compare);
  return real34IsZero(&compare);
}


bool_t real34CompareAbsEqual(const real34_t *number1, const real34_t *number2) {
  real34_t compare;
  real34_t num1, num2;

  real34CopyAbs(number1, &num1);
  real34CopyAbs(number2, &num2);

  real34Compare(&num1, &num2, &compare);
  return real34IsZero(&compare);
}


bool_t real34CompareGreaterEqual(const real34_t *number1, const real34_t *number2) {
  real34_t compare;

  real34Compare(number1, number2, &compare);
  return real34IsPositive(&compare) || real34IsZero(&compare);
}


bool_t real34CompareGreaterThan(const real34_t *number1, const real34_t *number2) {
  real34_t compare;

  real34Compare(number1, number2, &compare);
  return real34IsPositive(&compare) && !real34IsZero(&compare);
}


bool_t real34CompareLessEqual(const real34_t *number1, const real34_t *number2) {
  real34_t compare;

  real34Compare(number1, number2, &compare);
  return real34IsNegative(&compare) || real34IsZero(&compare);
}



bool_t real34CompareLessThan(const real34_t *number1, const real34_t *number2) {
  real34_t compare;

  real34Compare(number1, number2, &compare);
  return real34IsNegative(&compare) && !real34IsZero(&compare);
}

#if defined(OPTION_CUBIC_159) || defined(OPTION_EIGEN_159)
  #define ALLOW_159
#else
  #undef ALLOW_159
#endif


bool_t realCompareAbsGreaterThan(const real_t *number1, const real_t *number2) {
  realContext_t c = ctxtReal75;
  c.digits = max(max(75, number1->digits), number2->digits);
  #if defined(ALLOW_159)
    real159_t num1, num2;
    const int32_t k = 159;
  #else
    real_t num1, num2;
    const int32_t k = 75;
  #endif //ALLOW_159
  if(c.digits > k) {
    sprintf(errorMessage, "Exceed %d digits :realCompareAbsGreaterThan", (int)k);
    displayBugScreen(errorMessage);
  }
  realCopyAbs(number1, (real_t*)&num1);
  realCopyAbs(number2, (real_t*)&num2);
  realCompare((real_t*)&num1, (real_t*)&num2, (real_t*)&num2, &c);
  return realIsPositive((real_t*)&num2) && !realIsZero((real_t*)&num2);
}


/*
bool_t realCompareAbsGreaterEqual(const real_t *number1, const real_t *number2) {
  real159_t num1, num2;
  realContext_t c = ctxtReal75;
  c.digits = max(max(75, number1->digits), number2->digits);
  if(c.digits > 159) {
    sprintf(errorMessage, "Exceed 159 digits in comparisonReals.c");
    displayBugScreen(errorMessage);
  }
  realCopyAbs(number1, (real_t*)&num1);
  realCopyAbs(number2, (real_t*)&num2);
  realCompare((real_t*)&num1, (real_t*)&num2, (real_t*)&num2, &c);
  return realIsPositive((real_t*)&num2) || realIsZero((real_t*)&num2);
}
*/


bool_t realCompareAbsLessThan(const real_t *number1, const real_t *number2) {
  realContext_t c = ctxtReal75;
  c.digits = max(max(75, number1->digits), number2->digits);
  #if defined(ALLOW_159)
    real159_t num1, num2;
    const int32_t k = 159;
  #else
    real_t num1, num2;
    const int32_t k = 75;
  #endif //OPTION_CUBIC_159 || OPTION_EIGEN_159
  if(c.digits > k) {
    sprintf(errorMessage, "Exceed %d digits :realCompareAbsLessThan", (int)k);
    displayBugScreen(errorMessage);
  }
  realCopyAbs(number1, (real_t*)&num1);
  realCopyAbs(number2, (real_t*)&num2);
  realCompare((real_t*)&num1, (real_t*)&num2, (real_t*)&num2, &c);
  return realIsNegative((real_t*)&num2) && !realIsZero((real_t*)&num2);
}



bool_t realCompareEqual(const real_t *number1, const real_t *number2) {
  real_t compare;
  realContext_t c = ctxtReal75;
  c.digits = max(max(75, number1->digits), number2->digits);
  realCompare(number1, number2, &compare, &c);
  return realIsZero(&compare);
}



bool_t realCompareAbsEqual(const real_t *number1, const real_t *number2) {
  realContext_t c = ctxtReal75;
  c.digits = max(max(75, number1->digits), number2->digits);
  #if defined(ALLOW_159)
    real159_t num1, num2;
    const int32_t k = 159;
  #else
    real_t num1, num2;
    const int32_t k = 75;
  #endif //ALLOW_159
  if(c.digits > k) {
    sprintf(errorMessage, "Exceed %d digits :realCompareAbsEqual", (int)k);
    displayBugScreen(errorMessage);
  }
  realCopyAbs(number1, (real_t*)&num1);
  realCopyAbs(number2, (real_t*)&num2);
  realCompare((real_t*)&num1, (real_t*)&num2, (real_t*)&num2, &c);
  return realIsZero((real_t*)&num2);
}



bool_t realCompareGreaterEqual(const real_t *number1, const real_t *number2) {
  real_t compare;
  realContext_t c = ctxtReal75;
  c.digits = max(max(75, number1->digits), number2->digits);
  realCompare(number1, number2, &compare, &c);
  return realIsPositive(&compare) || realIsZero(&compare);
}



bool_t realCompareGreaterThan(const real_t *number1, const real_t *number2) {
  real_t compare;
  realContext_t c = ctxtReal75;
  c.digits = max(max(75, number1->digits), number2->digits);
  realCompare(number1, number2, &compare, &c);
  return realIsPositive(&compare) && !realIsZero(&compare);
}



bool_t realCompareLessEqual(const real_t *number1, const real_t *number2) {
  real_t compare;
  realContext_t c = ctxtReal75;
  c.digits = max(max(75, number1->digits), number2->digits);
  realCompare(number1, number2, &compare, &c);
  return realIsNegative(&compare) || realIsZero(&compare);
}



bool_t realCompareLessThan(const real_t *number1, const real_t *number2) {
  real_t compare;
  realContext_t c = ctxtReal75;
  c.digits = max(max(75, number1->digits), number2->digits);
  realCompare(number1, number2, &compare, &c);
  return realIsNegative(&compare) && !realIsZero(&compare);
}



bool_t real34IsAnInteger(const real34_t *x) {
  real34_t y;

  if(real34IsNaN(x) || real34IsInfinite(x)) {
    return false;
  }

  real34ToIntegralValue(x, &y, DEC_ROUND_DOWN);
  real34Subtract(x, &y, &y);

  return real34CompareEqual(&y, const34_0);
}



bool_t realIsAnInteger(const real_t *x) {
  realContext_t c = ctxtReal75;
  c.digits = max(75, x->digits);
  #if defined(ALLOW_159)
    real159_t y;
    const int32_t k = 159;
  #else
    real_t y;
    const int32_t k = 75;
  #endif //ALLOW_159
  if(c.digits > k) {
    sprintf(errorMessage, "Exceed %d digits :realIsAnInteger", (int)k);
    displayBugScreen(errorMessage);
  }
  if(realIsNaN(x)) {
    return false;
  }

  if(realIsInfinite(x)) {
    return true;
  }

  realToIntegralValue((real_t*)x, (real_t*)&y, DEC_ROUND_DOWN, &c);
  realSubtract((real_t*)x, (real_t*)&y, (real_t*)&y, &c);

  return realCompareEqual((real_t*)&y, const_0);
}



/*
int16_t realIdenticalDigits(real_t *a, real_t *b) {
  int16_t counter, smallest;

  if(realGetExponent(a) != realGetExponent(b)) {
    return 0;
  }

  realGetCoefficient(a, tmpString);
  realGetCoefficient(b, tmpString + TMP_STR_LENGTH/2);
  smallest = min(a->digits, b->digits);
  counter = 0;

  while(counter < smallest && tmpString[counter] == tmpString[TMP_STR_LENGTH/2 + counter]) {
    counter++;
  }

  return counter;
}
*/




//   #if defined FUTURE_USE_IRFRAC
//   // realCompareRoundedEqualConstant
//   
//   // Compares number to constant (2pi) at appropriate precision to avoid comparing a dirty rounding digit.
//   // If both operands have equal digit counts and both fit within 1071 digits, compares at full
//   // precision (no rounding occurs). If lengths differ, both are rounded to one digit less than the
//   // shorter before comparison to avoid the dirty rounding digit. If nn > 0, verifies first nn digits
//   // match (accounting for trailing zeros stripped by decNumber). If nn == 0, compares all input
//   // digits. Returns false for an input number +-5 digits longer/shorter than the target, allowing
//   // for up to 5 trailing zeros either side. Returns true if equal, false otherwise.
//   
//   // This portion identifies pi(1034) and if found, extends pi precision to 2139 digits.
//   
//   // Test: Using XPI, and adding and subtracting 1 ULP to verify recognition
//   //
//   // pi(1034)-Delta: is NOT recognized
//   //   R11 = -1.238094855978690282734422681276933E-1000 <=== note the last digit
//   //   R10 = 6283185307179586476925286766559005768394338798...398
//   //   R09 = 1E-999
//   //
//   // pi(1034)+Delta: : is NOT recognized
//   //   R08 = -1.238094855978690282734422681276931E-1000 <=== note the last digit
//   //   R07 = 6283185307179586476925286766559005768394338798...398
//   //   R06 = 1E-999
//   //
//   // Delta
//   //   R05 = 0.
//   //   R04 = 1
//   //   R03 = 1E-1033
//   //
//   // pi(1034): : is recognized
//   //   R02 = -1.238094855978690282734422681276932E-1000 <=== note the last digit
//   //   R01 = 6283185307179586476925286766559005768394338798...398
//   //   R00 = 1E-999
//   
//   
//   bool_t realCompareRoundedEqualConstant(const real_t *number, const real_t *constant, int32_t nn) {
//     int32_t checkDigits = (nn > 0) ? nn : number->digits;
//   
//     if(abs(number->digits - checkDigits) > 5) {
//       return false;
//     }
//   
//     if(realGetExponent(number) != realGetExponent(constant)) {
//       return false;
//     }
//   
//     real1071_t pii, n1;
//     realContext_t c = ctxtReal75;
//   
//     // Extract constant at appropriate precision
//     int32_t piPrecision = (number->digits == checkDigits && checkDigits <= 1071) ? checkDigits : 1071;
//     c.digits = piPrecision;
//     realPlus(constant, (real_t *)&pii, &c);
//     realGetCoefficient((real_t *)&pii, tmpString);
//   
//     // Check trailing zeros if needed
//     if(number->digits < checkDigits) {
//       for(int32_t i = number->digits; i < min(checkDigits, number->digits + 3); i++) {
//         if(tmpString[i] != '0') {
//           return false;
//         }
//       }
//       checkDigits = number->digits;
//     } else if(nn > 0 && number->digits > checkDigits) {
//       return false;
//     }
//   
//     // Extract input at appropriate precision
//     if(number->digits == checkDigits && checkDigits <= 1071) {
//       realGetCoefficient(number, tmpString + TMP_STR_LENGTH/2);
//       c.digits = checkDigits;
//     } else {
//       int32_t minDigits = min(number->digits, 1071);
//       c.digits = minDigits > 1 ? minDigits - 1 : 1;
//       realPlus(number, (real_t *)&n1, &c);
//       realGetCoefficient((real_t *)&n1, tmpString + TMP_STR_LENGTH/2);
//     }
//   
//     // Compare strings
//     int32_t compareDigits = min(checkDigits, c.digits);
//     for(int32_t i = 0; i < compareDigits; i++) {
//       if(tmpString[i] != tmpString[TMP_STR_LENGTH/2 + i]) {
//         return false;
//       }
//     }
//   
//     return true;
//   }
//   #endif //!FUTURE_USE_IRFRAC


