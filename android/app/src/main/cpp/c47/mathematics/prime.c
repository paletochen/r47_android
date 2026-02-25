// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file prime.c
 ***********************************************/

#include "c47.h"

#if defined(SAVE_SPACE_DM42_12PRIME)
  void fnIsPrime      (uint16_t unusedButMandatoryParameter){;}
  void fnNextPrime    (uint16_t unusedButMandatoryParameter){;}
  void fnPrimeFactors (uint16_t unusedButMandatoryParameter){;}
  void fnEvPFacts     (uint16_t unusedButMandatoryParameter){;}
#else


#define maximumPrime 308   //10^308


//debugging method to disable the prime factor parts
#define MONITOR_FACTORS
#undef MONITOR_FACTORS
const bool_t Factors_1_SmallPrimes   = true;
const bool_t Factors_2_PerfectSquare = true;
const bool_t Factors_3_Pollard       = true;


// primes less than 212 for the sieve, the rest of the array to fill one byte primes for the factors trial tests
TO_QSPI const uint8_t smallPrimes[] = {   2,   3,   5,   7,  11,  13,  17,  19,  23,  29,  31,  37,
                                         41,  43,  47,  53,  59,  61,  67,  71,  73,  79,  83,  89,
                                         97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151,
                                        157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211,
                                        223, 227, 229, 233, 239, 241, 251 };

// pre-calced sieve of Eratosthenes for n = 2, 3, 5, 7
TO_QSPI const uint8_t indices[] = {   1,  11,  13,  17,  19,  23,  29,  31,  37,  41,  43,  47,
                                     53,  59,  61,  67,  71,  73,  79,  83,  89,  97, 101, 103,
                                    107, 109, 113, 121, 127, 131, 137, 139, 143, 149, 151, 157,
                                    163, 167, 169, 173, 179, 181, 187, 191, 193, 197, 199, 209 };

// distances between sieve values
TO_QSPI const uint8_t offsets[] = {  10,   2,   4,   2,   4,   6,   2,   6,   4,   2,   4,   6,
                                      6,   2,   6,   4,   2,   6,   4,   6,   8,   4,   2,   4,
                                      2,   4,   8,   6,   4,   6,   2,   4,   6,   2,   6,   6,
                                      4,   2,   4,   6,   2,   6,   4,   2,   4,   2,  10,   2 };


// distances between primes, to add to the 1-byte small prime list; starting at 251+6 ==> 257 .... 1009
TO_QSPI const uint8_t smallPrimes2[] = { 6, 6, 6, 2, 6, 4, 2, 10, 14, 4, 2, 4, 14, 6, 10, 2, 4, 6, 8,
                                         6, 6, 4, 6, 8, 4, 8, 10, 2, 10, 2, 6, 4, 6, 8, 4, 2, 4, 12,
                                         8, 4, 8, 4, 6, 12, 2, 18, 6, 10, 6, 6, 2, 6, 10, 6, 6, 2, 6,
                                         6, 4, 2, 12, 10, 2, 4, 6, 6, 2, 12, 4, 6, 8, 10, 8, 10, 8, 6,
                                         6, 4, 8, 6, 4, 8, 4, 14, 10, 12, 2, 10, 2, 4, 2, 10, 14, 4, 2,
                                         4, 14, 4, 2, 4, 20, 4, 8, 10, 8, 4, 6, 6, 14, 4, 6, 6, 8, 6, 12 };


#define smallPrimeListNumber (nbrOfElements(smallPrimes) + nbrOfElements(smallPrimes2))
uint16_t smallPrimeList(uint16_t index) {
  uint16_t tt = 251;
  if(index < nbrOfElements(smallPrimes)) {
    return smallPrimes[index];
  } else
  if(index < smallPrimeListNumber) {
    uint16_t subIndex = index - nbrOfElements(smallPrimes);
    for(uint16_t ii = 0; ii <= subIndex && ii < nbrOfElements(smallPrimes2); ii++) {
      tt += smallPrimes2[ii];
    }
    return tt;
  } else {
    return 0;
  }
}

//To check if the list of small primes being split into two methods, are contimuous
//void listAllPrimesInList(void) {
//  for (uint i = 0; i < smallPrimeListNumber; i++) {
//    printf("prime: %u : %u\n",i,smallPrimeList(i));
//  }
//}



//------------- Pollard for Factors -------------
// Common status codes returned by factoring algorithms.
typedef enum {
  FACTORS_SETUP,     // Start or restart the factoring algorithm
  FACTORS_ITERATE,   // Perform one or more factoring iterations
  FACTORS_RESET,     // Indicates the input n has changed (reinitialize)
  FACTORS_DONE,      // A nontrivial factor has been found
  FACTORS_FAIL       // Too many attempts or failure to progress
} factors_status_t;

// Result structure returned after a step or setup, for monitoring and diagnostic data.
typedef struct {
  factors_status_t status;
  int iterations_this_call;
  int total_iterations;
  int attempts;
} factors_result_t;

// Internal structure for Pollard's Rho algorithm.
typedef struct {
  longInteger_t n;        // Number to factor
  longInteger_t x, y;     // Floyd's cycle detection: tortoise and hare
  longInteger_t c;        // Constant for the polynomial: f(x) = x^2 + c
  longInteger_t d;        // Current GCD candidate
  longInteger_t tmp;      // Temporary variable for calculations
  int iteration;          // Total iterations performed
  int attempt;            // Number of reinitializations attempted
  pcg32_random_t rng;     // Use PCG32 instead of GMP RNG
} pollard_t;


void pollard_init(pollard_t *self);
void pollard_clear(pollard_t *self);
void pollard_update_n(pollard_t *self, const longInteger_t new_n);
char* pollard_status(factors_status_t st);
factors_result_t pollard_step(pollard_t *self, longInteger_t factor, factors_status_t instruction, int steps);
//------------- Pollard for Factors -------------




void calculateNextPrime(longInteger_t currentNumber, longInteger_t nextPrime);

/*
// Test if a number is prime or not using a Miller-Rabin test
#define QUICK_CHECK (101*101-1)
#define NUMBER_OF_SMALL_PRIMES 25

static bool_t longIntegerIsPrime1(longInteger_t primeCandidate) {
  uint32_t i;
  longInteger_t primeCandidateMinus1, s, temp, smallPrime, mod;

  if(longIntegerCompareUInt(primeCandidate, 2) < 0) {
    return false;
  }

  // Quick check for divisibility by small primes
  for(i=0; i<NUMBER_OF_SMALL_PRIMES; i++) {
    if(longIntegerCompareUInt(primeCandidate, smallPrimes[i]) == 0) {
      return true;
    }
    else if(longIntegerModuloUInt(primeCandidate, smallPrimes[i]) == 0) {
      return false;
    }
  }

  if(longIntegerCompareUInt(primeCandidate, QUICK_CHECK) < 0) {
    return true;
  }

  longIntegerInit(primeCandidateMinus1);
  longIntegerInit(s);
  longIntegerInit(temp);
  longIntegerInit(smallPrime);
  longIntegerInit(mod);
  longIntegerSubtractUInt(primeCandidate, 1, primeCandidateMinus1);
  longIntegerCopy(primeCandidateMinus1, s);

  // Calculate s such as   primeCandidate - 1 = s×2^d and s odd
  while(longIntegerIsEven(s)) {
    longIntegerDivide2Exact(s, s);
  }

  // The loop below should only go from 0 to 12 (primes from 2 to 41) ensuring correct result for candidatePrime < 3 317 044 064 679 887 385 961 981
  // There is a conjecture that when going from 0 to 19 (primes from 2 to 71) the result is correct up to 10^36
  for(i=0; i<NUMBER_OF_SMALL_PRIMES; i++) {
    longIntegerCopy(s, temp);

    uInt32ToLongInteger(smallPrimes[i], smallPrime);
    longIntegerPowerModulo(smallPrime, temp, primeCandidate, mod);
    while(longIntegerCompare(temp, primeCandidateMinus1) != 0 && longIntegerCompareUInt(mod, 1) != 0 && longIntegerCompare(mod, primeCandidateMinus1) != 0) {
      longIntegerPowerUIntModulo(mod, 2, primeCandidate, mod);
      longIntegerMultiply2(temp, temp);
    }

    if(longIntegerCompare(mod, primeCandidateMinus1) != 0 && longIntegerIsEven(temp)) {
      longIntegerFree(primeCandidateMinus1);
      longIntegerFree(s);
      longIntegerFree(temp);
      longIntegerFree(smallPrime);
      longIntegerFree(mod);
      return false;
    }
  }

  longIntegerFree(primeCandidateMinus1);
  longIntegerFree(s);
  longIntegerFree(temp);
  longIntegerFree(smallPrime);
  longIntegerFree(mod);
  return true;
} */

static bool_t getIntArg(longInteger_t x, calcRegister_t regist) {
  bool_t fractional;

  if(!getRegisterAsLongInt(regist, x, &fractional)) {
    if(getRegisterDataType(regist) != dtReal34Matrix) {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "The input value is invalid for storage into a longinteger!");
        moreInfoOnError("In function getIntArg:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    return false;
  }

  if(fractional) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "The input value is fractional and invalid for storage into a longinteger!");
      moreInfoOnError("In function getIntArg:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }
  return true;
}


void fnIsPrime(uint16_t unusedButMandatoryParameter) {
  #if !defined(SAVE_SPACE_DM42_12PRIME)
    longInteger_t tmp, primeCandidate;

    if(!getIntArg(primeCandidate, REGISTER_X)) {
      goto abort1;
    }

    longIntegerInit(tmp);
    longIntegerPowerUIntUInt(10,maximumPrime,tmp);
    longIntegerSubtract(primeCandidate, tmp, tmp);   // (primeCandidate - 10^300) positive is too large
    if(longIntegerIsPositive(tmp)) {
      badDomainError(REGISTER_X);
      goto abort2;
    }

    SET_TI_TRUE_FALSE(longIntegerIsPositive(primeCandidate) && longIntegerIsPrime(primeCandidate) != 0);

  abort2:
    longIntegerFree(tmp);
  abort1:
    longIntegerFree(primeCandidate);
  #endif // !SAVE_SPACE_DM42_12PRIME
}

void SQUFOF(longInteger_t result, const longInteger_t N, const real34_t lastAdded);
void complete_factorization1(const longInteger_t N);
void complete_factorization2(const longInteger_t N);


void fnNextPrime(uint16_t unusedButMandatoryParameter) {
  #if !defined(SAVE_SPACE_DM42_12PRIME)
    real_t x;
    longInteger_t tmp, currentNumber, nextPrime;

    if(getRegisterDataType(REGISTER_X) == dtReal34) {    //Allow decimals to be rounded down, to be able to get the next prime despite being decimal input;
      if(!getRegisterAsReal(REGISTER_X, &x)) {
        return;
      }
      longIntegerInit(currentNumber);
      convertRealToLongInteger(&x, currentNumber, DEC_ROUND_DOWN);
    }
    else {
      if(!getIntArg(currentNumber, REGISTER_X)) {
        goto abort1;
      }
    }

    longIntegerInit(tmp);
    longIntegerPowerUIntUInt(10, maximumPrime, tmp);
    longIntegerSubtract(currentNumber, tmp, tmp);   // (primeCandidate - 10^300) positive is too large
    if(longIntegerIsPositive(tmp)) {
      badDomainError(REGISTER_X);
      goto abort2;
    }


    if(!saveLastX()) {
      goto abort2;
    }

    if(!longIntegerIsPositive(currentNumber)) {
      uInt32ToLongInteger(1u, currentNumber);
    }


    longIntegerInit(nextPrime);
    //longIntegerNextPrime(currentNumber, nextPrime);
    calculateNextPrime(currentNumber, nextPrime);

    if(getRegisterDataType(REGISTER_L) == dtShortInteger) {
      convertLongIntegerToShortIntegerRegister(nextPrime, getRegisterShortIntegerBase(REGISTER_L), REGISTER_X);
    }
    else {
      convertLongIntegerToLongIntegerRegister(nextPrime, REGISTER_X);
    }

    longIntegerFree(nextPrime);
  abort2:
    longIntegerFree(tmp);
  abort1:
    longIntegerFree(currentNumber);
  #endif // !SAVE_SPACE_DM42_12PRIME
}


/*
// Baillie-PSW primality test in python found here:
// https://codegolf.stackexchange.com/questions/10701/fastest-code-to-find-the-next-prime
bool_t isStrongProbablePrime(longInteger_t primeCandidate) {
  longInteger_t two, d, x, primeCandidateMinus1;
  int32_t r, s;

  longIntegerInit(d);
  longIntegerSubtractUInt(primeCandidate, 1, d);       // d = primeCandidate - 1;
  longIntegerInit(primeCandidateMinus1);
  longIntegerCopy(d, primeCandidateMinus1);            // primeCandidateMinus1 = primeCandidate - 1
  s = 0;
  while(longIntegerIsEven(d)) {
    s++;
    longIntegerDivide2(d, d);                          // d >>= 1;
  }

  longIntegerInit(two);
  uInt32ToLongInteger(2u, two);                           // two = 2
  longIntegerInit(x);
  longIntegerPowerModulo(two, d, primeCandidate, x);   // x = pow(2, d, primeCandidate);
  longIntegerFree(two);
  longIntegerFree(d);

  if(longIntegerCompareUInt(x, 1) == 0 || longIntegerCompare(x, primeCandidateMinus1) == 0) {
    longIntegerFree(x);
    longIntegerFree(primeCandidateMinus1);
    return true;
  }

  for(r=1; r<s; r++) {
    longIntegerPowerUIntModulo(x, 2, primeCandidate, x); // x = (x * x) % primeCandidate;
    if(longIntegerCompareUInt(x, 1) == 0) {
      longIntegerFree(x);
      longIntegerFree(primeCandidateMinus1);
      return false;
    }
    else if(longIntegerCompare(x, primeCandidateMinus1) == 0) {
      longIntegerFree(x);
      longIntegerFree(primeCandidateMinus1);
      return true;
    }
  }

  longIntegerFree(x);
  longIntegerFree(primeCandidateMinus1);
  return false;
}

// Lucas probable prime
// assumes D = 1 (mod 4), (D|primeCandidate) = -1
bool_t isLucasProbablePrime(longInteger_t primeCandidate, longInteger_t D) {
  longInteger_t Q, s, t, primeCandidatePlus1, inv2, oldU, U, V, q;
  uint32_t r = 0;

  longIntegerInit(Q);                             // Q = 0;
  uInt32ToLongInteger(1u, Q);                     // Q = 1;
  longIntegerSubtract(Q, D, Q);                   // Q = 1 - D;
  longIntegerDivideUInt(Q, 4, Q);                 // Q = (1 - D) >> 2

  // primeCandidate + 1 = 2**r*s where s is odd
  longIntegerInit(s);                             // s = 0;
  longIntegerAddUInt(primeCandidate, 1, s);       // s = primeCandidate + 1
  longIntegerCopy(s, primeCandidatePlus1);        // primeCandidatePlus1 = primeCandidate + 1
  while(longIntegerIsEven(s)) {
    r++;
    longIntegerDivide2(s, s);
  }

  // calculate the bit reversal of (odd) s
  // e.g. 19 (10011) <=> 25 (11001)
  longIntegerInit(t);                             // t = 0;
  while(longIntegerIsPositive(s)) {
    if(longIntegerIsOdd(s)) {
      longIntegerAddUInt(t, 1, t);                // t++;
      longIntegerSubtractUInt(s, 1, s);           // s--;
    }
    else {
      longIntegerMultiply2(t, t);                 // t <<= 1;
      longIntegerDivide2(s, s);                   // s >>= 1;
    }
  }

  // use the same bit reversal process to calculate the sth Lucas number
  // keep track of q = Q**primeCandidate as we go
  longIntegerInit(U);                             // U = 0;
  longIntegerInit(V);                             // V = 0;
  uInt32ToLongInteger(2u, V);                     // V = 2;
  longIntegerInit(q);                             // q = 0;
  uInt32ToLongInteger(1u, q);                     // q = 1;
  // mod_inv(2, primeCandidate)
  longIntegerDivide2(primeCandidatePlus1, inv2);  //inv2 = primeCandidatePlus1 >> 1;
  while(longIntegerIsPositive(t)) {
    if(longIntegerIsOdd(t)) {
      // U, V of primeCandidate+1
      longIntegerCopy(U, oldU);                   // oldU = U;
      longIntegerAdd(oldU, V, U);                 // U =  (oldU + V);
      longIntegerMultiply(U, inv2, U);            // U =  (oldU + V) * inv2;
      longIntegerModulo(U, primeCandidate, U);    // U = ((oldU + V) * inv2) % primeCandidate;

      longIntegerMultiply(D, oldU, oldU);         // oldU = D * oldU;
      longIntegerAdd(oldU, V, V);                 // V =   D * oldU + V;
      longIntegerMultiply(V, inv2, V);            // V =  (D * oldU + V) * inv2;
      longIntegerModulo(V, primeCandidate, V);    // V = ((D * oldU + V) * inv2) % primeCandidate;

      longIntegerMultiply(q, Q, q);               // q = q * Q;
      longIntegerModulo(q, primeCandidate, q);    // q = (q * Q) % primeCandidate;
      longIntegerSubtractUInt(t, 1, t);           // t--;
    }
    else {
      // U, V of primeCandidate * 2
      longIntegerMultiply(U, V, U);               // U =  U * V;
      longIntegerModulo(U, primeCandidate, U);    // U = (U * V) % primeCandidate;

      longIntegerMultiply(V, V, V);               // V = V * V;
      longIntegerMultiply2(q, oldU);              // oldU = 2 * q;
      longIntegerSubtract(V, oldU, V);            // V =  V * V - 2 * q;
      longIntegerModulo(V, primeCandidate, V);    // V = (V * V - 2 * q) % primeCandidate;

      longIntegerMultiply(q, q, q);               // q = q * q;
      longIntegerModulo(q, primeCandidate, q);    // q = (q * q) % primeCandidate;

      longIntegerDivide2(t, t);                   // t >>= 1;
    }
  }

  // double s until we have the 2**r*sth Lucas number
  while(r > 0) {
    longIntegerMultiply(U, V, U);                 // U =  U * V;
    longIntegerModulo(U, primeCandidate, U);      // U = (U * V) % primeCandidate;

    longIntegerMultiply(V, V, V);                 // V = V * V;
    longIntegerMultiply2(q, oldU);                // oldU = 2 * q;
    longIntegerSubtract(V, oldU, V);              // V =  V * V - 2 * q;
    longIntegerModulo(V, primeCandidate, V);      // V = (V * V - 2 * q) % primeCandidate;

    longIntegerMultiply(q, q, q);                 // q = q * q;
    longIntegerModulo(q, primeCandidate, q);      // q = (q * q) % primeCandidate;

    r--;
  }

  // primality check
  // if primeCandidate is prime, primeCandidate divides the primeCandidate+1st Lucas number, given the assumptions
  return (longIntegerIsZero(U));
}


// an 'almost certain' primality check
bool_t longIntegerIsPrime2(longInteger_t primeCandidate) {
  longInteger_t primeCandidateMinus1, primeCandidateMinus1on2, a, s, temp;
  uint32_t i, j, pc;

  if(longIntegerCompareUInt(primeCandidate, smallPrimes[46]+1) <= 0) {  //smallPrimes[46]+1 = 212
    longIntegerToUInt32(primeCandidate, pc);
    for(i=0; i<nbrOfElements(smallPrimes); i++) {
      if(smallPrimes[i] == pc) {
        return true;
      }
    }
    return false;
  }

  for(i=0; i<nbrOfElements(smallPrimes); i++) {
    if(longIntegerModuloUInt(primeCandidate, smallPrimes[i]) == 0) {
      return false;
    }
  }

  // if primeCandidate is a 32-bit integer, perform full trial division
  if(longIntegerCompareUInt(primeCandidate, 0xffffffff) <= 0) {
    i = smallPrimes[46]; //smallPrimes[46] = 211
    longIntegerToUInt32(primeCandidate, pc);
    while(i*i < pc) {
      for(j=0; j<nbrOfElements(offsets); j++) {
        i += offsets[j];
        if(pc % i == 0) {
          return false;
        }
      }
    }

    return true;
  }

  // Baillie-PSW: this is technically a probabalistic test, but there are no known pseudoprimes
  if(!isStrongProbablePrime(primeCandidate)) {
    return false;
  }

  longIntegerSubtractUInt(primeCandidate, 1, primeCandidateMinus1); // primeCandidateMinus1 = primeCandidate - 1;
  longIntegerDivide2(primeCandidate, primeCandidateMinus1on2);      // primeCandidateMinus1on2 = (primeCandidate - 1) >> 1
  uInt32ToLongInteger(2u, s);
  uInt32ToLongInteger(5u, a);
  longIntegerPowerModulo(a, primeCandidateMinus1on2, primeCandidate, temp); // temp = Legendre symbol resulting in primeCandidate-1 if a is a non-residue, instead of -1
  while(longIntegerCompare(temp, primeCandidateMinus1) != 0) {
    longIntegerChangeSign(s);     // s = -s;
    longIntegerSubtract(s, a, a); // a = s - a;
    longIntegerPowerModulo(a, primeCandidateMinus1on2, primeCandidate, temp);
  }

  return isLucasProbablePrime(primeCandidate, a);
} */

// Next prime strictly larger than currentNumber
//void nextPrime(longInteger_t currentNumber, longInteger_t nextPrime) {
void calculateNextPrime(longInteger_t currentNumber, longInteger_t nextPrime) {
  uint32_t cn, i, x, s, e, m, o;
  #if !defined(TESTSUITE_BUILD)
    int32_t loop = 0;
  #endif //TESTSUITE_BUILD


  if(longIntegerCompareUInt(currentNumber, 2) < 0) {
    uInt32ToLongInteger(2u, nextPrime);
    return;
  }

  // first odd larger than currentNumber
  longIntegerAddUInt(currentNumber, 1, currentNumber);
  if(longIntegerIsEven(currentNumber)) {
    longIntegerAddUInt(currentNumber, 1, currentNumber);
  }


  //replaced the above with a faster integer only sequential prime elimination
  //check if the next odd number is a small prime
  if (longIntegerCompareUInt(currentNumber, smallPrimeList(smallPrimeListNumber - 1) + 1) < 0) {
    if (mpz_fits_ulong_p(currentNumber)) {
      longIntegerToUInt32(currentNumber, cn);
      while (true) {
        for (i = 0; i < smallPrimeListNumber; i++) {
          if (smallPrimeList(i) == cn) {
            uInt32ToLongInteger(cn, nextPrime);
            return;
          }
        }
        cn += 2;
      }
    }
    //note: not coming out of here except by returning out of the loop
  }


  // find our position in the sieve rotation via binary search
  // The sieve algorithm uses a wheel of size 210 (the product of the first four primes:
  // 2×3×5×7 = 210). The rotation pattern (or reduced residue system) under mod 210
  // consists of numbers < 210 that are coprime to 210, and these are indexed in indices[].
  // There are 48 such numbers, 47 when starting from 2 as we do.

  // below:
  // startpoint inclusive
  // endpoint exclusive
  // midpoint: will be incremented until endpoint is reached

  // uses the first 47 entries of the smallPrimes list, not needing the actual list, using the indices
  x = longIntegerModuloUInt(currentNumber, 210);
  s = 0;
  e = 47;
  m = 24;
  while(m != e) {
    if(indices[m] < x) {
      s = m;
      m = (s + e + 1) >> 1;
    }
    else {
      e = m;
      m = (s + e) >> 1;
    }
  }

  //nextPrime = currentNumber + indices[m] - x;
  longIntegerAddUInt(currentNumber, indices[m] - x, nextPrime);
  while(true) {
    for(o=m; o<m+48; o++) {
      //if(longIntegerIsPrime2(nextPrime)) {
      if(longIntegerIsPrime(nextPrime)) {
        return;
      }
      longIntegerAddUInt(nextPrime, offsets[o % 48], nextPrime);

      #if !defined(TESTSUITE_BUILD)
        if(monitorExit(&loop, "Iter: ")) {
          displayCalcErrorMessage(ERROR_SOLVER_ABORT, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
          return;
        }
      #endif //!TESTSUITE_BUILD
    }
  }
}


#if !defined (TESTSUITE_BUILD)
  #if !defined(SAVE_SPACE_DM42_12PRIME)
    static void _showProgress(const real34_t *ss, longInteger_t nextp) {
      real34_t rr;
      clearRegisterLine(REGISTER_Z, true, true);
      clearRegisterLine(REGISTER_Y, true, true);
      clearRegisterLine(REGISTER_X, true, true);
      uint8_t savedDisplayFormatDigits = displayFormatDigits;
      displayFormatDigits = 0;
      strcpy(tmpString,"Last:   ");
      real34ToDisplayString(ss, amNone, tmpString+5, &standardFont, 400-6*18, 34, !LIMITEXP, FRONTSPACE, NOIRFRAC);
      showString(tmpString, &standardFont, 1, Y_POSITION_OF_REGISTER_Y_LINE + 6, vmNormal, true, true);
      convertLongIntegerToReal34(nextp, &rr);

      strcpy(tmpString,"Test:   ");
      real34ToDisplayString(&rr, amNone, tmpString+5, &standardFont, 400-6*18, 34, !LIMITEXP, FRONTSPACE, NOIRFRAC);
      showString(tmpString, &standardFont, 1, Y_POSITION_OF_REGISTER_Z_LINE + 6, vmNormal, true, true);

      refreshRegisterLine(REGISTER_X);

      displayFormatDigits = savedDisplayFormatDigits;
    }
  #endif //SAVE_SPACE_DM42_12PRIME
#endif //TESTSUITE_BUILD





// *** This is used with the Euler sigma function
void longIntegerSumPowers(longInteger_t base, longInteger_t exponent, uint32_t k, longInteger_t result) {
  longInteger_t count, sum, pwr, tmp, tmpbase;
  longIntegerInit(count);
  longIntegerInit(sum);
  longIntegerInit(pwr);
  longIntegerInit(tmp);
  longIntegerInit(tmpbase);
  uInt32ToLongInteger(0u, sum);
  longIntegerCopy(exponent, count);

  while(!longIntegerIsNegative(count)) {
    //printLongIntegerToConsole(count,"  count:"," \n");
    if(k == 0) {                                    // Divisor Count is the generalized sigma function, with k = 0
      uInt32ToLongInteger(0u, tmp);
    }
    else {                                          // Euler's sigma function is the generalized sigma function, with k = 1
      longIntegerCopy(count, tmp);
    }
    if(k >= 2) {                                    // Generalized sigma function, with k > 1
      longIntegerMultiplyUInt(tmp, k, tmp);
    }
    longIntegerCopy(base, tmpbase);
    longIntegerPower(tmpbase, tmp, pwr);
    longIntegerCopy(pwr, tmp);
    longIntegerAdd(sum, tmp, sum);
    //printLongIntegerToConsole(pwr,"  pwr:"," ");
    //printLongIntegerToConsole(sum,"  sum:","\n");
    longIntegerSubtractUInt(count, 1, count);
  }

  longIntegerCopy(sum, result);
  longIntegerFree(tmpbase);
  longIntegerFree(tmp);
  longIntegerFree(pwr);
  longIntegerFree(sum);
  longIntegerFree(count);
}



/*
 * This is the inverse function of fnPrimeFactors.
 * Given a matrix as produced by fnPrimeFactors, this function expands out
 * all the prime factors with their exponents, and returns in the X register
 * the result as a long integer.
 */
static void _doFnEvPFacts     (uint16_t param) {
  #if !defined(SAVE_SPACE_DM42_12PRIME)
    real_t factorR, factorI, baseR, expR, prodR, prodI;

    //parameter X required for k
    int32_t pwr = param;
    real_t x;
    longInteger_t currentNumber;
    if(param == M_SIGMA_k) {
      if(getRegisterDataType(REGISTER_X) == dtReal34) { //Allow decimals to be rounded down, to be able to get the next prime despite being decimal input;
        if(!getRegisterAsReal(REGISTER_X, &x)) {
          goto abort;
        }
        pwr = realToInt32C47(&x);
        fnDrop(NOPARAM);
      }
      else {
        if(!getIntArg(currentNumber, REGISTER_X)) {
          longIntegerFree(currentNumber);
          goto abort;
        }
        longIntegerToInt32(currentNumber, pwr);
        if(pwr > 3321 || pwr < 0) {
          longIntegerFree(currentNumber);
          goto abort;
        }
        fnDrop(NOPARAM);
        longIntegerFree(currentNumber);
      }
    }

    if(getRegisterDataType(REGISTER_X) == dtReal34Matrix) {
      real34Matrix_t matrix;

      linkToRealMatrixRegister(REGISTER_X, &matrix);
      uint16_t rows = REGISTER_MATRIX_HEADER(REGISTER_X)->matrixRows;
      uint16_t cols = REGISTER_MATRIX_HEADER(REGISTER_X)->matrixColumns;
      if(rows == 2 && cols >= 1) {
        // Only operate if factorisation matrix has two rows and at least one column
        longInteger_t prod, factor, tmp_prod, p_li, k_li;
        longIntegerInit(prod);
        longIntegerInit(factor);
        longIntegerInit(tmp_prod);
        uInt32ToLongInteger(1u, prod);
        realCopy(const_1,&prodR);
        #define sumTypeInteger 0
        #define sumTypeReal    1
        #define sumTypeComplex 2
        uint8_t sumType = sumTypeInteger;
        for(uint16_t j=0;  j<cols; ++j) {
          if(real34IsAnInteger(&matrix.matrixElements[j]) && real34IsAnInteger(&matrix.matrixElements[cols+j]) && sumType == sumTypeInteger) {
            convertReal34ToLongInteger(&matrix.matrixElements[j], p_li, RM_HALF_UP);
            convertReal34ToLongInteger(&matrix.matrixElements[cols+j], k_li, RM_HALF_UP);
            //printLongIntegerToConsole(p_li,"base:","  ");
            //printLongIntegerToConsole(k_li,"exp:","\n");
            switch(param){
              case M_FACTORS: longIntegerPower(p_li, k_li, factor); break;
              case M_SIGMA_0:
              case M_SIGMA_1:
              case M_SIGMA_k: longIntegerSumPowers(p_li, k_li, pwr, factor); break;
              default:;
            }
            longIntegerFree(p_li);
            longIntegerFree(k_li);
            //printLongIntegerToConsole(factor,"factor:","\n");
            longIntegerCopy(prod, tmp_prod);
            longIntegerMultiply(tmp_prod, factor, prod);
          }
          else {
            if(sumType == sumTypeInteger) {
              convertLongIntegerToReal(prod, &prodR, &ctxtReal39);
              sumType = sumTypeReal;
            }
            real34ToReal(&matrix.matrixElements[j], &baseR);
            real34ToReal(&matrix.matrixElements[cols+j], &expR);
            if(!(realIsNegative(&baseR) && !realIsAnInteger(&expR)) && sumType == sumTypeReal) {
              PowerReal(&baseR, &expR, &factorR, &ctxtReal39);
              realMultiply(&prodR, &factorR, &prodR, &ctxtReal39);
            }
            else if(getFlag(FLAG_CPXRES)) {
              if(sumType == sumTypeReal) {
                realCopy(const_0,&prodI);
                sumType = sumTypeComplex;
              }
              if(sumType == sumTypeComplex) {
                PowerComplex(&baseR, const_0, &expR, const_0, &factorR, &factorI, &ctxtReal39);
                mulComplexComplex(&prodR, &prodI, &factorR, &factorI, &prodR, &prodI, &ctxtReal39);
              }
            }
            else {
              displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
              #if (EXTRA_INFO_ON_CALC_ERROR == 1)
                moreInfoOnError("In function _doFnEvPFacts:", "cannot do complex results if CPXRES is not set", NULL, NULL);
              #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
              longIntegerFree(prod);
              longIntegerFree(factor);
              longIntegerFree(tmp_prod);
              return;
            }
          }
        }

        switch(sumType) {
          case sumTypeInteger: {
            convertLongIntegerToLongIntegerRegister(prod, REGISTER_X);
            break;
          }
          case sumTypeReal: {
            convertRealToResultRegister(&prodR, REGISTER_X, amNone);
            break;
          }
          case sumTypeComplex: {
            convertComplexToResultRegister(&prodR, &prodI, REGISTER_X);
            break;
          }
          default: break;
        }
        adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
        longIntegerFree(prod);
        longIntegerFree(factor);
        longIntegerFree(tmp_prod);
      }
      else {
        displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X); // Invalid input data type for this operation
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "Only 2" STD_CROSS "n matrix supported: %" PRIu32 STD_CROSS "%" PRIu32 " matrix", rows, cols);
          moreInfoOnError("In function _doFnEvPFacts:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        goto return10;
      }
    }
    else {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X); // Invalid input data type for this operation
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "2" STD_CROSS "n matrix required.");
        moreInfoOnError("In function _doFnEvPFacts:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      goto return10;
    }
    return10:
    refreshScreen(253);
    return;

    abort:
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function _doFnEvPFacts:", "cannot do Euler sigma function due to parameter issue", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    goto return10;

  #endif //SAVE_SPACE_DM42_12PRIME
}



static void doFnEvPFacts (uint16_t param) {
  longInteger_t xx;
  int32_t k = 1;

  /* process M_SIGMA_p1 as M_SIGMA_1; process M_SIGMA_pk as M_SIGMA_k*/
  if(param == M_SIGMA_p1 || param == M_SIGMA_pk) {
    if(param == M_SIGMA_pk) {
      if(!getIntArg(xx, REGISTER_X)) {
        goto end;
      }
      longIntegerToInt32(xx, k);
      fnSwapXY(NOPARAM);                                           // get matrix to X
      end:
      longIntegerFree(xx);
    }

    //expect matrix in X
    real34Matrix_t xx;
    convertReal34MatrixRegisterToReal34Matrix(REGISTER_X, &xx);    // temporary store matrix, as the temp register is being used already
    longInteger_t y, x, z, tmp;
    _doFnEvPFacts(M_FACTORS);                                      // longinteger register output
    convertLongIntegerRegisterToLongInteger(REGISTER_X, y);
    longIntegerInit(z);
    longIntegerInit(tmp);
    int32ToLongInteger(k, z);
    longIntegerPower(y, z, tmp);
    longIntegerCopy(tmp,y);                                        // y is the number to be subtracted
    convertReal34MatrixToReal34MatrixRegister(&xx, REGISTER_X);    // restore matrix

    switch(param) {
      case M_SIGMA_p1:
        _doFnEvPFacts(M_SIGMA_1);                                  // longinteger register output
        break;
      case M_SIGMA_pk:
        fnSwapXY(NOPARAM);                                         // restore matrix to y
        _doFnEvPFacts(M_SIGMA_k);                                  // longinteger register output
        break;
      default:;
    }
    convertLongIntegerRegisterToLongInteger(REGISTER_X, x);
    longIntegerSubtract(x, y, x);
    convertLongIntegerToLongIntegerRegister(x, REGISTER_X);
    longIntegerFree(tmp);
    longIntegerFree(z);
    longIntegerFree(y);
    longIntegerFree(x);
  } else {
  /* process M_SIGMA_0, M_SIGMA_1, M_SIGMA_k */
    _doFnEvPFacts(param);
  }
}

/******************************************************************************
 * isRegisterMatrixFactors
 * Validates 2 rows, >= 1 cols matrix as likely prime factors for use in the phi and sigma functions
 *   Row 1: [p1, p2, ..., pn]  factors >= 1 (or -1 in first column for optional sign)
 *   Row 2: [e1, e2, ..., en]  exponents >= 0
 * Parameters: reg - register to validate, isNegative output is true if [-1,1] sign column is present
 * Returns: true if valid factorization matrix
 ******************************************************************************/
static bool_t isRegisterMatrixFactors(calcRegister_t reg, bool_t *isNegative) {
  const uint32_t type = getRegisterDataType(reg);
  *isNegative = false;

  if (type == dtReal34Matrix) {
    const matrixHeader_t *head = REGISTER_MATRIX_HEADER(reg);
    const uint16_t cols = head->matrixColumns;
    real34_t *elems = REGISTER_REAL34_MATRIX_ELEMENTS(reg);
    real_t x;
    bool_t mustBeOne = false;
    unsigned int i;

    if (head->matrixRows != 2 || cols < 1)  // changed to allow single column, -1^1 or m^n
      return false;
    for (i = 0; i < cols; i++) {
      real34ToReal(elems + i + 0 * cols, &x);
      if (!realIsAnInteger(&x))
        return false;
      if (realCompareLessEqual(&x, const_0)) {// changed to allow any factor >= 1, to include 1^n
        if (i != 0)
          return false;
        if (!realCompareEqual(&x, const__1))
          return false;
        mustBeOne = true;
      }

      real34ToReal(elems + i + 1 * cols, &x);
      if (!realIsAnInteger(&x))
        return false;
      if (realCompareLessThan(&x, const_0))  // change to const_0 to allow n^0 (exponents ≥ 0). (0^0 per definition will not occur as 0 factor is not allowed).
        return false;
      if (mustBeOne) {
        if (!realCompareEqual(&x, const_1))
          return false;
        *isNegative = true;
        mustBeOne = false;
      }
    }
    return true;
  }

  return false;
}

static void fnEulPhi(uint16_t unusedButMandatoryParameter);
static bool_t performPrimeFactorization(bool_t doSaveLastX);


/******************************************************************************
 * ensureFactorizationMatrix
 * Validates register contains either:
 *   - A valid factorization matrix, or
 *   - A non-negative long integer
 * Optionally factorizes the integer into a matrix.
 * Parameters:
 *   reg - register to validate
 *   allowNegative - if false, rejects negative matrices
 *   doFactorizeNow - if true, factorizes integer; if false, only validates
 *   wasAlreadyMatrix - output: true if was already a matrix
 * Returns: true if valid and non-negative, false otherwise (displays errors)
 ******************************************************************************/
static bool_t ensureFactorizationMatrix(calcRegister_t reg, bool_t allowNegative, bool_t doFactorizeNow, bool_t *wasAlreadyMatrix) {
  bool_t isNegative;
  *wasAlreadyMatrix = isRegisterMatrixFactors(reg, &isNegative);
  if(!*wasAlreadyMatrix) {
    if(getRegisterDataType(reg) == dtReal34Matrix) {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, reg);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "The input value is not a valid matrix!");
        moreInfoOnError("In function ensureFactorizationMatrix 01:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
    //check long integer for validity and positive
    longInteger_t x;
      if(!getIntArg(x, reg)) {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, reg);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "The input value is neither a longinteger nor a valid matrix!");
        moreInfoOnError("In function ensureFactorizationMatrix 02:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      longIntegerFree(x);
      return false;
    }
    if(longIntegerIsNegative(x)) {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, reg);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "The input value is negative and therefore out of the domain!");
        moreInfoOnError("In function ensureFactorizationMatrix 03:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      longIntegerFree(x);
      return false;
    }
    longIntegerFree(x);
  }
  if(*wasAlreadyMatrix && !allowNegative && isNegative) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, reg);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "The matrix input value is negative and therefore out of the domain!");
      moreInfoOnError("In function ensureFactorizationMatrix 04:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }
  // Factorize the integer if requested
  if(doFactorizeNow && !*wasAlreadyMatrix) {
    if(reg == REGISTER_Y) {
      fnSwapXY(NOPARAM);
      performPrimeFactorization(false);
      fnSwapXY(NOPARAM);
    }
    else {
      performPrimeFactorization(false);
    }
  }
  return true;
}



void fnEvPFacts(uint16_t param) {
  bool_t isValidMxX      = false;
  bool_t isValidMxY      = false;
  bool_t needsFactorizeX = false;

  // Validate (before saveLastX)
  if(param != M_FACTORS) {
    if(!ensureFactorizationMatrix(REGISTER_X, false, false, &isValidMxX)) {
      goto return10;
    }
    needsFactorizeX = !isValidMxX && (param == M_SIGMA_0 || param == M_SIGMA_1 || param == M_SIGMA_p1);
  }

  if(!saveLastX()) {
    goto return10;
  }
  saveForUndo();

  // Factorize X if needed (after saveLastX)
  if(needsFactorizeX) {
    if(!ensureFactorizationMatrix(REGISTER_X, false, true, &isValidMxX)) {
      goto return10;
    }
  }

  //pre-processing
  switch(param) {
    case M_SIGMA_0  :  // 0  // k = 0                      ; monadic; x has input number
    case M_SIGMA_1  :  // 1  // k = 1                      ; monadic; x has input number
    case M_SIGMA_p1 :  // 3  // k = 1 proper               ; monadic; x has input number
      ; //nothing - X already ensured to be factorization matrix
      break;
    case M_SIGMA_k  :  // 2  // k > 1                      ; dyadic; x has k; y has input number
    case M_SIGMA_pk :  // 4  // k > 1 proper genereralized ; dyadic; x has k; y has input number
      if(!ensureFactorizationMatrix(REGISTER_Y, false, false, &isValidMxY)) {
        goto return10;
      }
      if(!isValidMxY) {
        if(!ensureFactorizationMatrix(REGISTER_Y, false, true, &isValidMxY)) {
          goto return10;
        }
      }
      break;
    case M_FACTORS  :  // 5
      ; //nothing
      break;
    case M_PHI_EUL  :  // 6
      ; //nothing
      break;
    default:;
  }

  //processing
  switch(param) {
    case M_SIGMA_0  :  // 0  // k = 0                      ; monadic; x has input number
    case M_SIGMA_1  :  // 1  // k = 1                      ; monadic; x has input number
    case M_SIGMA_k  :  // 2  // k > 1                      ; dyadic; x has k; y has input number
    case M_SIGMA_p1 :  // 3  // k = 1 proper               ; monadic; x has input number
    case M_SIGMA_pk :  // 4  // k > 1 proper genereralized ; dyadic; x has k; y has input number
    case M_FACTORS  :  // 5
      if(lastErrorCode == 0) {
        doFnEvPFacts(param);
      } else {
        #if defined(PC_BUILD)
          printf("fnEvPFacts 07: Error passed through: lastErrorCode=%d\n",lastErrorCode);
        #endif
      }
      break;
    case M_PHI_EUL  :  // 6
        fnEulPhi(NOPARAM);
      break;
    default:;
  }

  return10:
  screenUpdatingMode &= ~SCRUPD_MANUAL_STACK;
  refreshScreen(254);
}



/*
 * This function accepts a long integer in the X register and computes
 * the Euler phi function on it.  The result is a new long integer.
 *
 * Example:
 *
 *   Input X register:  1500
 *   Output X register:  400
 *
 * Formula for euler phi function phi(n):
 * determine prime factorisation of n:  p[0]**k[0], p[1]**k[1], p[2]**k[2], ... p[r]**k[r] where the p[i] are distinct
 * phi(n) = n * prod[i=0...r]((p[i]-1)/p[i])
 *
 * This is done by grabbing the value n as long integer from the X register.
 * Then fnPrimeFactors is computed, producing a factorisation matrix.
 * The prime factors are extracted from this matrix and used to complete
 * the calculation.
 */
static void fnEulPhi(uint16_t unusedButMandatoryParameter) {
  #if !defined(SAVE_SPACE_DM42_12PRIME)
    bool_t isMxXNegative = false;
    longInteger_t x;
    bool_t useMatrix = isRegisterMatrixFactors(REGISTER_X, &isMxXNegative);
    real34Matrix_t xx;

    if(useMatrix) {
      convertReal34MatrixRegisterToReal34Matrix(REGISTER_X, &xx);
      doFnEvPFacts(M_FACTORS);
    }

    if(!getIntArg(x, REGISTER_X)) {
      if(useMatrix) {
        convertReal34MatrixToReal34MatrixRegister(&xx, REGISTER_X);    // restore matrix after matrix element operations
      }
      goto return1;
    }

    if(useMatrix) {
      convertReal34MatrixToReal34MatrixRegister(&xx, REGISTER_X);    // restore matrix
    }

    longInteger_t phi_x, p_li, p_li_less_1, phi_x_tmp, phi_x_tmp_b;
    longIntegerInit(phi_x);
    longIntegerInit(phi_x_tmp);
    longIntegerInit(phi_x_tmp_b);
    real34Matrix_t matrix;
    // Check for the trivial case x = 0        (*)
    if(longIntegerIsZero(x)) {
      longIntegerCopy(x, phi_x);
      goto returnValue;
    }
    longIntegerSubtractUInt(x, 1, phi_x_tmp);
    // Check for the trivial case x = 1        (**)
    if(longIntegerIsZero(phi_x_tmp)) {
      longIntegerCopy(x, phi_x);
      goto returnValue;
    }

    {
      if(!useMatrix) {
        fnPrimeFactors(unusedButMandatoryParameter);
      }
      if(getRegisterDataType(REGISTER_X) == dtReal34Matrix) {
        // Only operate if we got back a Real 34 Matrix from fnPrimeFactors
        linkToRealMatrixRegister(REGISTER_X, &matrix);
        uint16_t rows = REGISTER_MATRIX_HEADER(REGISTER_X)->matrixRows;
        uint16_t cols = REGISTER_MATRIX_HEADER(REGISTER_X)->matrixColumns;
        if (rows == 2 && cols >= 1) {
          // Only operate if factorisation matrix has two rows and at least one column
          longIntegerCopy(x, phi_x);
          for (uint16_t j = 0;  j < cols; ++j) {
            real34_t p = matrix.matrixElements[j];
            convertReal34ToLongInteger(&p, p_li, RM_HALF_UP);
            longIntegerInit(p_li_less_1);
            longIntegerSubtractUInt(p_li, 1, p_li_less_1);
            if(j == 0 && !longIntegerIsPositive(p_li_less_1)) {   //ensure 0 is returned is the first factor <= 1. This is achieved above, see (*), (**), (***)
              uInt32ToLongInteger(0u, phi_x);
              longIntegerFree(p_li);
              longIntegerFree(p_li_less_1);
              break;
            }
            longIntegerCopy(phi_x, phi_x_tmp);
            longIntegerDivide(phi_x_tmp, p_li, phi_x_tmp_b);
            longIntegerMultiply(phi_x_tmp_b, p_li_less_1, phi_x);
            longIntegerFree(p_li);
            longIntegerFree(p_li_less_1);
          }
        }
        else {
          goto return2; //matrix dimensions wrong. An error would have been issued by fnPrimeFactors
        }
      }
      else {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "The intermediate prime factor matrix could not be found.");
        moreInfoOnError("In function fnEulPhi:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      goto return2;
      }
    }

    returnValue:
    convertLongIntegerToLongIntegerRegister(phi_x, REGISTER_X);
    adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);

    return2:
    longIntegerFree(phi_x);
    longIntegerFree(phi_x_tmp);
    longIntegerFree(phi_x_tmp_b);

    return1:
    longIntegerFree(x);

  #endif //SAVE_SPACE_DM42_12PRIME
}





//** factorization **********************************************************************************************************
// Shanks algorithm, based on the demo on the wikipedia page
// https://en.wikipedia.org/wiki/Shanks%27s_square_forms_factorization
// jaymos 2025

    const int multipliers[] = {
        1, 3, 5, 7, 11, 13,
        3*5, 3*7, 3*11, 3*13,
        5*7, 5*11, 5*13,
        7*11, 7*13,
        11*13,
        3*5*7, 3*5*11, 3*5*13,
        3*7*11, 3*7*13, 3*11*13,
        5*7*11, 5*7*13,
        3*5*7*11
    };

    bool_t addFactorsToTSV = false;

    #if !defined(TESTSUITE_BUILD)
      static void keepFileNameAlive(void) {
        if(addFactorsToTSV) {
          preventFilenameTimeout();
        }
      }
    #endif //TESTSUITE_BUILD


    // Fast perfect square check using 32-bit integer sqrt
    static int is_perfect_square_uint32(uint32_t n, uint32_t* sqrt_out) {
        uint32_t r = (uint32_t)(sqrt((double)n));
        if (r * r == n) {
            if (sqrt_out) *sqrt_out = r;
            return 1;
        }
        if ((r + 1) * (r + 1) == n) {
            if (sqrt_out) *sqrt_out = r + 1;
            return 1;
        }
        return 0;
    }

    // Check if a number is a perfect square using GMP
    // Efficient and correct: fast path for small numbers, fallback for large
    static int longIntegerIsPerfectSquareCheckAndDo(const longInteger_t n, longInteger_t r) {
        if (mpz_fits_uint_p(n)) {
            uint32_t small = (uint32_t)mpz_get_ui(n);
            uint32_t sqrt_small;
            if (is_perfect_square_uint32(small, &sqrt_small)) {
                uInt32ToLongInteger(sqrt_small, r);
                return 1;
            }
            return 0;
        }
        // GMP fallback for larger integers
        if (longIntegerPerfectSquare(n)) {
            longIntegerSquareRoot(n, r);
            return 1;
        }
        return 0;
    }


    int32_t loopp;

    void SQUFOF(longInteger_t result, const longInteger_t N, const real34_t lastAdded) {
      uint32_t k;
      longInteger_t BB, LL, ii, D, Po, P, Pprev, Q, Qprev, q, b, r, s, temp1, temp2, temp3, gcd_result;
      longIntegerInit(BB);
      longIntegerInit(LL);
      longIntegerInit(ii);
      longIntegerInit(D);
      longIntegerInit(Po);
      longIntegerInit(P);
      longIntegerInit(Pprev);
      longIntegerInit(Q);
      longIntegerInit(Qprev);
      longIntegerInit(q);
      longIntegerInit(b);
      longIntegerInit(r);
      longIntegerInit(s);
      longIntegerInit(temp1);
      longIntegerInit(temp2);
      longIntegerInit(temp3);
      longIntegerInit(gcd_result);

      //Pollard simultaneous analysis setup
      pollard_t pollardData;
      factors_status_t instruction = FACTORS_SETUP;
      factors_result_t PollardResult;
      longInteger_t n, pollardFactor;
      if(Factors_3_Pollard) {
        longIntegerInit(n);
        longIntegerInit(pollardFactor);
        longIntegerCopy(N, n);
        pollard_init(&pollardData);
        pollard_update_n(&pollardData, n);
      }

      // Check if N is a perfect square
      if (longIntegerIsPerfectSquareCheckAndDo(N, s)) {
        longIntegerCopy(s, result);
        goto cleanup;
      }

      // Calculate s = sqrt(N)
      longIntegerSquareRoot(N, s);

      for (k = 0; k < nbrOfElements(multipliers); k++) {
          // D = multiplier[k] * N  (just N here)
          longIntegerMultiplyUInt(N, multipliers[k], D);

          // Po = Pprev = P = sqrt(D)
          longIntegerSquareRoot(D, Po);
          longIntegerCopy(Po, Pprev);
          longIntegerCopy(Po, P);

          // Qprev = 1
          uInt32ToLongInteger(1, Qprev);

          // Q = D - Po*Po
          longIntegerMultiply(Po, Po, temp1);
          longIntegerSubtract(D, temp1, Q);
          if (longIntegerSign(Q) == 0) {
              continue; // Q is zero; no factor found
          }

          // LL = 2 * sqrt(2*s)
          longIntegerMultiplyUInt(s, 2, temp1);
          longIntegerSquareRoot(temp1, temp2);
          longIntegerMultiplyUInt(temp2, 2, LL);

          // BB = 3 * LL
          longIntegerMultiplyUInt(LL, 3, BB);

          // Initialize i as longInteger_t for comparison with BB
          uInt32ToLongInteger(2, ii);

          while (longIntegerCompare(ii, BB) < 0) {
              #if !defined(TESTSUITE_BUILD)
                loopp++;
                if(checkHalfSec()) {
                  keepFileNameAlive();
                  if(progressHalfSecUpdate_Integer(timed, "Factors: Shanks/Pollard" STD_UP_ARROW " n =",loopp, halfSec_clearZ, halfSec_clearT, halfSec_disp)) { //timed
                    _showProgress(&lastAdded, n);
                    force_refresh(force);
                  }
                }
                if(exitKeyWaiting() || programRunStop == PGM_WAITING) {
                  progressHalfSecUpdate_Integer(force+1, "Interrupted: ",loopp, halfSec_clearZ, halfSec_clearT, halfSec_disp);
                  programRunStop = PGM_WAITING;
                  break;
                }
              #endif //!TESTSUITE_BUILD


           //Pollard simultaneous analysis - interject a few steps
              //printf("While: PollardIter %u : %s\n",PollardResult.status, pollard_status(PollardResult.status));
              if(Factors_3_Pollard && (instruction == FACTORS_ITERATE || instruction == FACTORS_SETUP)) {
                PollardResult = pollard_step(&pollardData, pollardFactor, instruction, 10);
                #if defined(MONITOR_FACTORS)
                  printf("   Factor loops: %15d | Pollard steps: %10d | Attempts: %3d | Status: %4d     \r",loopp, PollardResult.total_iterations, PollardResult.attempts, PollardResult.status);
                  fflush(stdout);
                #endif //MONITOR_FACTORS
                if (PollardResult.status == FACTORS_DONE) {
                  #if defined(MONITOR_FACTORS)
                    gmp_printf("   Pollard found factor: %Zd                    \n", pollardFactor);
                  #endif //MONITOR_FACTORS
                  longIntegerCopy(pollardFactor, result);
                  goto cleanup;
                } else if (PollardResult.status == FACTORS_FAIL) {
                  #if defined(MONITOR_FACTORS)
                    printf("   Pollard failed after %d attempts.                \n", PollardResult.attempts);
                  #endif //MONITOR_FACTORS
                }
                instruction = PollardResult.status;
              }
              //Pollard end

              // b = (Po + P) / Q
              longIntegerAdd(Po, P, temp1);
              longIntegerDivide(temp1, Q, b);

              // P = b*Q - P
              longIntegerMultiply(b, Q, temp1);
              longIntegerSubtract(temp1, P, temp2);
              longIntegerCopy(temp2, P);
              // q = Q
              longIntegerCopy(Q, q);
              // Q = Qprev + b*(Pprev - P)
              longIntegerSubtract(Pprev, P, temp1);
              longIntegerMultiply(b, temp1, temp2);
              longIntegerAdd(Qprev, temp2, Q);
              // r = sqrt(Q)
              longIntegerSquareRoot(Q, r);
              // Check if i is even and r*r == Q
              if (longIntegerIsEven(ii) && longIntegerIsPerfectSquareCheckAndDo(Q, temp1)) {
                  break;
              }

              // Qprev = q; Pprev = P
              longIntegerCopy(q, Qprev);
              longIntegerCopy(P, Pprev);

              // Increment i
              longIntegerAddUInt(ii, 1, ii);
          }

          #if defined(MONITOR_FACTORS)
            printf("\n");
            fflush(stdout);
          #endif //MONITOR_FACTORS

          if (longIntegerCompare(ii, BB) >= 0) {
              continue;
          }

          // b = (Po - P) / r
          longIntegerSubtract(Po, P, temp1);
          longIntegerDivide(temp1, r, b);

          // Pprev = P = b*r + P
          longIntegerMultiply(b, r, temp1);
          longIntegerAdd(temp1, P, temp2);
          longIntegerCopy(temp2, Pprev);
          longIntegerCopy(temp2, P);

          // Qprev = r
          longIntegerCopy(r, Qprev);

          // Q = (D - Pprev*Pprev) / Qprev
          longIntegerMultiply(Pprev, Pprev, temp1);
          longIntegerSubtract(D, temp1, temp2);
          longIntegerDivide(temp2, Qprev, Q);

          do {
              #if !defined(TESTSUITE_BUILD)
                loopp++;
                if(checkHalfSec()) {
                  keepFileNameAlive();
                  if(progressHalfSecUpdate_Integer(timed, "Factors: Shanks/Pollard" STD_UP_ARROW " n =",loopp, halfSec_clearZ, halfSec_clearT, halfSec_disp)) { //timed
                    _showProgress(&lastAdded, n);
                    force_refresh(force);
                  }
                }
                if(exitKeyWaiting()  || programRunStop == PGM_WAITING) {
                  progressHalfSecUpdate_Integer(force+1, "Interrupted: ",loopp, halfSec_clearZ, halfSec_clearT, halfSec_disp);
                  programRunStop = PGM_WAITING;
                  break;
                }
              #endif //!TESTSUITE_BUILD
              // b = (Po + P) / Q
              longIntegerAdd(Po, P, temp1);
              longIntegerDivide(temp1, Q, b);

              // Pprev = P
              longIntegerCopy(P, Pprev);

              // P = b*Q - P
              longIntegerMultiply(b, Q, temp1);
              longIntegerSubtract(temp1, P, P);

              // q = Q
              longIntegerCopy(Q, q);

              // Q = Qprev + b*(Pprev - P)
              longIntegerSubtract(Pprev, P, temp1);
              longIntegerMultiply(b, temp1, temp2);
              longIntegerAdd(Qprev, temp2, Q);

              // Qprev = q
              longIntegerCopy(q, Qprev);

          } while (longIntegerCompare(P, Pprev) != 0);

          #if !defined(TESTSUITE_BUILD)
            if(exitKeyWaiting()  || programRunStop == PGM_WAITING) {
              progressHalfSecUpdate_Integer(force+1, "Interrupted: ",loopp, halfSec_clearZ, halfSec_clearT, halfSec_disp);
              programRunStop = PGM_WAITING;
              break;
            }
          #endif //TESTSUITE_BUILD

          // r = gcd(N, Qprev)
          longIntegerGcd(N, Qprev, r);

          // Check if r != 1 and r != N
          if (longIntegerCompareUInt(r, 1) != 0 && longIntegerCompare(r, N) != 0) {
              longIntegerCopy(r, result);
              goto cleanup;
          }
      }

      // No factor found
      uInt32ToLongInteger(0, result);

cleanup:
      //Pollard simultaneous analysis cleanup
      if(Factors_3_Pollard) {
        pollard_clear(&pollardData);
        longIntegerFree(n);
        longIntegerFree(pollardFactor);
      }
      longIntegerFree(BB);
      longIntegerFree(LL);
      longIntegerFree(ii);
      longIntegerFree(D);
      longIntegerFree(Po);
      longIntegerFree(P);
      longIntegerFree(Pprev);
      longIntegerFree(Q);
      longIntegerFree(Qprev);
      longIntegerFree(q);
      longIntegerFree(b);
      longIntegerFree(r);
      longIntegerFree(s);
      longIntegerFree(temp1);
      longIntegerFree(temp2);
      longIntegerFree(temp3);
      longIntegerFree(gcd_result);
    }



bool delCol1RealMatrixX(void) {
    real34Matrix_t mat;
    linkToRealMatrixRegister(REGISTER_X, &mat);
    uint16_t rows = mat.header.matrixRows;
    uint16_t cols = mat.header.matrixColumns;
    if (!mat.matrixElements || cols <= 1) return false;

    // every element in column 0 is exactly 1.0 or 0.0
    bool removeFirstCol = true;
    for (uint16_t i = 0; i < rows; ++i) {
        if (!real34CompareEqual(&mat.matrixElements[i * cols + 0], const34_1) && !real34CompareEqual(&mat.matrixElements[i * cols + 0], const34_0)) {
            removeFirstCol = false;
            break;
        }
    }
    if (!removeFirstCol) {
        // nothing to delete, first column is all 1.0 or 0.0
        return true;
    }


    // Build a temp matrix with one fewer column:
    real34Matrix_t tmp;
    if (!realMatrixInit(&tmp, rows, cols - 1)) {
        displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
        return false;
    }
    for (uint16_t i = 0; i < rows; i++) {
        for (uint16_t j = 1; j < cols; j++) {
            real34Copy(
                &mat.matrixElements[i*cols + j],
                &tmp.matrixElements[i*(cols-1) + (j-1)]
            );
        }
    }
    // Re-init the register and free old buffer
    initMatrixRegister(REGISTER_X, rows, cols - 1, false);

    // Copy the temp data back into the freshly-allocated register
    real34_t *dest = REGISTER_REAL34_MATRIX_ELEMENTS(REGISTER_X);
    size_t total = (size_t)rows * (cols - 1);
    for (size_t k = 0; k < total; k++) {
        real34Copy(&tmp.matrixElements[k], &dest[k]);
    }
    realMatrixFree(&tmp);
    return true;
}




#define MAX_FACTORS 110
#define MAXIMUM_QUEUE_SIZE 100 // must be even. Worst well on 1000, which costs > 8000 bytes just for the empty longints
typedef struct FactorAdder
{
  uint16_t nExpons;
  uint16_t expons[MAX_FACTORS];
} FactorAdder_t;


#if !defined(SAVE_SPACE_DM42_12PRIME)
  static void initFactorAdder(FactorAdder_t *faddr) {
    faddr->nExpons = 0;
  };

//  static void initFactorCreateFromMatrix(FactorAdder_t *faddr) {
//    faddr->nExpons = 0;
//    if(!real34CompareAbsEqual(REGISTER_REAL34_MATRIX_ELEMENTS(REGISTER_X)+0,const34_1)) {
//      uint16_t cols = REGISTER_MATRIX_HEADER(REGISTER_X)->matrixColumns;
//      uint16_t rows = REGISTER_MATRIX_HEADER(REGISTER_X)->matrixRows;
//      for(int j = cols-1-1; j >= 0 ; j--) {
//        for(int i = rows-1; i >= 0; i--) {
//          real34Copy(REGISTER_REAL34_MATRIX_ELEMENTS(REGISTER_X)+i*cols+j,REGISTER_REAL34_MATRIX_ELEMENTS(REGISTER_X)+i*(cols)+j+1);
//        }
//      }
//      real34Copy(const34_1, REGISTER_REAL34_MATRIX_ELEMENTS(REGISTER_X)+0);
//      real34Copy(const34_1, REGISTER_REAL34_MATRIX_ELEMENTS(REGISTER_X)+cols+0);
//      faddr->expons[faddr->nExpons] = 1;
//      (faddr->nExpons)++;
//    }
//    while(faddr->nExpons < REGISTER_MATRIX_HEADER(REGISTER_X)->matrixColumns ) {
//      faddr->expons[faddr->nExpons] = real34ToUInt32(REGISTER_REAL34_MATRIX_ELEMENTS(REGISTER_X)+REGISTER_MATRIX_HEADER(REGISTER_X)->matrixColumns + faddr->nExpons);
//      (faddr->nExpons)++;
//    }
//  };


  // An integer matrix is maintained with the exponents only, and dumped to the visible matrix only when needed

  void dumpExponents(calcRegister_t regist, FactorAdder_t *faddr, uint16_t dumpForFewerThan) {
      uint16_t cols = REGISTER_MATRIX_HEADER(regist)->matrixColumns;
      uint16_t rows = REGISTER_MATRIX_HEADER(regist)->matrixRows;

                                            #ifdef MONITOR_FACTORS
                                              printf("\ndumpExponents:\n");
                                              printRegisterToConsole(regist,"Matrix: ","\n");
                                              printf("  Exponent Array: faddr->nExpons=%d\n", faddr->nExpons);
                                              for(int ii = 0; ii < faddr->nExpons; ii++) {
                                                printf("%d:%d ",ii, faddr->expons[ii]);
                                              }
                                              printf("\n");
                                              fflush(stdout);
                                            #endif //MONITOR_FACTORS

    if(faddr->nExpons != cols || rows != 2 || getRegisterDataType(REGISTER_X) != dtReal34Matrix) {
       #if !defined(TESTSUITE_BUILD)
         displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
         #if (EXTRA_INFO_ON_CALC_ERROR == 1)
           sprintf(errorMessage, "Incorrect matrix counters %" PRIu32 STD_CROSS "%" PRIu32 " matrix vs. array %d", rows, cols, faddr->nExpons);
           moreInfoOnError("In function dumpExponents:", errorMessage, NULL, NULL);
         #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
         return;
       #endif // !TESTSUITE_BUILD
    }
                                            #ifdef MONITOR_FACTORS
                                              printf("dumpExponents:  fill exponents:  faddr->nExpons==%u dumpForFewerThan=%u\n", faddr->nExpons, dumpForFewerThan);
                                              printf("--a:  rows==%" PRIu16 ", cols==%" PRIu16 "\n", rows, cols);
                                              fflush(stdout);
                                            #endif //MONITOR_FACTORS
    for( uint16_t i = 0;  i < min(faddr->nExpons, dumpForFewerThan);  ++i ) {
                                            #ifdef MONITOR_FACTORS
                                              printf("--b:  adding expon at faddr->nExpons==%u, i==%u, val %u, ind %u\n", faddr->nExpons, i, faddr->expons[i], faddr->nExpons+i);
                                              fflush(stdout);
                                            #endif //MONITOR_FACTORS
      uInt32ToReal34(faddr->expons[i], REGISTER_REAL34_MATRIX_ELEMENTS(regist) + faddr->nExpons+i);
    }
  }


  static void pushLongIntegerToJK(longInteger_t factor) {
    copySourceRegisterToDestRegister(REGISTER_H, REGISTER_G);
    copySourceRegisterToDestRegister(REGISTER_K, REGISTER_H);
    convertLongIntegerToLongIntegerRegister(factor, REGISTER_K);
  }


  static bool_t addFactor(longInteger_t factor, calcRegister_t regist, const real34_t *lastAdded,FactorAdder_t *faddr) {
    //printLongIntegerToConsole(factor,"-->","\n");

    if(addFactorsToTSV) {
      convertLongIntegerToLongIntegerRegister(factor, TEMP_REGISTER_1);
      fnP_All_Regs(PRN_TMP);
    }

    pushLongIntegerToJK(factor);

                                            #ifdef MONITOR_FACTORS
                                              printf("--c:  addFactor()\n");
                                              fflush(stdout);
                                            #endif //MONITOR_FACTORS
    if(getRegisterDataType(regist) != dtReal34Matrix) {
                                            #ifdef MONITOR_FACTORS
                                              uint16_t cols = REGISTER_MATRIX_HEADER(regist)->matrixColumns;
                                              uint16_t rows = REGISTER_MATRIX_HEADER(regist)->matrixRows;
                                              printf("addFactor 1:\n");
                                              printf("--a:  rows==%" PRIu16 ", cols==%" PRIu16 "\n", rows, cols);
                                              fflush(stdout);
                                            #endif //MONITOR_FACTORS
      //Initialize Memory for Matrix
      if(initMatrixRegister(regist, 2, 1, false)) {
                                             #ifdef MONITOR_FACTORS
                                              uint16_t cols = REGISTER_MATRIX_HEADER(regist)->matrixColumns;
                                              uint16_t rows = REGISTER_MATRIX_HEADER(regist)->matrixRows;
                                              printf("addFactor 2:\n");
                                              printf("--a:  rows==%" PRIu16 ", cols==%" PRIu16 "\n", rows, cols);
                                              fflush(stdout);
                                            #endif //MONITOR_FACTORS
        setSystemFlag(FLAG_ASLIFT);
      }
      else {
                                            #ifdef MONITOR_FACTORS
                                              uint16_t cols = REGISTER_MATRIX_HEADER(regist)->matrixColumns;
                                              uint16_t rows = REGISTER_MATRIX_HEADER(regist)->matrixRows;
                                              printf("addFactor 3:\n");
                                              printf("--a:  rows==%" PRIu16 ", cols==%" PRIu16 "\n", rows, cols);
                                              fflush(stdout);
                                            #endif //MONITOR_FACTORS

        if(lastErrorCode != 0) goto returnFalse;
        displayCalcErrorMessage(ERROR_NOT_ENOUGH_MEMORY_FOR_NEW_MATRIX, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          uint16_t cols_ = REGISTER_MATRIX_HEADER(regist)->matrixColumns;
          uint16_t rows_ = REGISTER_MATRIX_HEADER(regist)->matrixRows;
          sprintf(errorMessage, "Not enough memory for a rows:%" PRIu32 STD_CROSS " cols:%" PRIu32 " matrix", rows_, cols_);
          moreInfoOnError("In function addFactor 001:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        goto returnFalse;
      }
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }



    uint16_t rows = REGISTER_MATRIX_HEADER(regist)->matrixRows;
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      uint16_t cols = REGISTER_MATRIX_HEADER(regist)->matrixColumns;
    #endif //(EXTRA_INFO_ON_CALC_ERROR == 1)
    if(rows > 2) {
       #if !defined(TESTSUITE_BUILD)
         displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
         #if (EXTRA_INFO_ON_CALC_ERROR == 1)
           sprintf(errorMessage, "Incorrect matrix dimensions %" PRIu32 STD_CROSS "%" PRIu32 " matrix", rows, cols);
           moreInfoOnError("In function addFactor:", errorMessage, NULL, NULL);
         #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
       #endif // !TESTSUITE_BUILD
       goto returnFalse;
    }

    if( faddr->nExpons == 0 ) {
      faddr->nExpons = 1;  // has to be 1 now, as we have this factor
      faddr->expons[(faddr->nExpons)-1] = 1;
    }
    uint16_t wkgCols = faddr->nExpons;
                                            #ifdef MONITOR_FACTORS
                                              gmp_printf("--d:  factor==%Zd, rows==%u, cols==%u, nExpons==%u, wkgCols==%u\n",factor, (uint16_t)rows, (uint16_t)cols, faddr->nExpons, wkgCols);
                                              fflush(stdout);
                                            #endif //MONITOR_FACTORS
    if(!redimMatrixRegister(regist, rows, wkgCols, ITM_M_DIM)) {
      if(lastErrorCode != 0) goto returnFalse;
      #if !defined(TESTSUITE_BUILD)
        displayCalcErrorMessage(ERROR_NOT_ENOUGH_MEMORY_FOR_NEW_MATRIX, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "Not enough memory for a %" PRIu32 STD_CROSS "%" PRIu32 " matrix", rows, cols);
          moreInfoOnError("In function addFactor 002:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      #endif // !TESTSUITE_BUILD
      goto returnFalse;
    }

    int counter = faddr->nExpons;
    uint16_t n = rows*counter;
    uint16_t c = n/2;
                                            #ifdef MONITOR_FACTORS
                                              printf("faddr->nExpons=%d n=%d c=%d\n",faddr->nExpons, n, c);
                                              fflush(stdout);
                                            #endif //MONITOR_FACTORS
    real34_t factorR;
    convertLongIntegerToReal34(factor, &factorR);
    //search for existing factors
    while(counter >= 0 && !real34CompareEqual(&factorR, REGISTER_REAL34_MATRIX_ELEMENTS(regist) + counter)) {
      counter--;
    }

    //increment exponent if found
    if( longIntegerSign(factor) != 0 && counter >= 0 && !real34CompareAbsEqual(REGISTER_REAL34_MATRIX_ELEMENTS(regist) + counter,const34_1) ) {
      ++(faddr->expons[counter]);
                                              #ifdef MONITOR_FACTORS
                                                printf("--e:   use existing:  created expons %u at %u\n",faddr->expons[(faddr->nExpons)-1], (faddr->nExpons)-1);
                                              #endif //MONITOR_FACTORS
    }
    else {
      bool_t incNExpons = real34CompareAbsEqual(&factorR,const34_1) ? false : true;
      if( !incNExpons ) {
        c = 0;
      }
                                              #ifdef MONITOR_FACTORS
                                                printf("--f:   restart:  n==%u, c==%u, incNExpons==%d\n", n, c, incNExpons);
                                              #endif //MONITOR_FACTORS
      real34Copy(&factorR, REGISTER_REAL34_MATRIX_ELEMENTS(regist) + c);
      real34Copy(&factorR, lastAdded);
      if( incNExpons ) {
        if( faddr->nExpons < MAX_FACTORS ) {
            (faddr->nExpons)++;
        }
        else {
          #if !defined(TESTSUITE_BUILD)
            displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
            #if (EXTRA_INFO_ON_CALC_ERROR == 1)
              sprintf(errorMessage, "Maximum number of factors exceeded %" PRIu32 STD_CROSS "%" PRIu32 " matrix", rows, cols);
              moreInfoOnError("In function addFactor 003:", errorMessage, NULL, NULL);
            #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          #endif // !TESTSUITE_BUILD
        goto returnFalse;
        }

        ++wkgCols;
        faddr->expons[faddr->nExpons-1] = 1;
        if(!redimMatrixRegister(regist, rows, wkgCols, ITM_M_DIM)) {
          if(lastErrorCode != 0) goto returnFalse;
          #if !defined(TESTSUITE_BUILD)
            displayCalcErrorMessage(ERROR_NOT_ENOUGH_MEMORY_FOR_NEW_MATRIX, ERR_REGISTER_LINE, REGISTER_X);
            #if (EXTRA_INFO_ON_CALC_ERROR == 1)
              sprintf(errorMessage, "Not enough memory for a %" PRIu32 STD_CROSS "%" PRIu32 " matrix", rows, cols);
              moreInfoOnError("In function addFactor 004", errorMessage, NULL, NULL);
            #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
            #endif // !TESTSUITE_BUILD
            goto returnFalse;
        }
      }
      n = rows*(faddr->nExpons);
      c = n/2;
    }
    dumpExponents(REGISTER_X, faddr, 13);
    updateMatrixHeightCache();
    refreshRegisterLine(REGISTER_X);
    #if defined (PC_BUILD) //Note clear the correct number of lines to ensure no old register debris remains on screen. This is only required on SIM as the hardware clears the screen presumably
      if(cachedDisplayStack <= 3) refreshRegisterLine(REGISTER_Y);
      if(cachedDisplayStack <= 2) refreshRegisterLine(REGISTER_Z);
      if(cachedDisplayStack <= 1) refreshRegisterLine(REGISTER_T);
    #endif //PC_BUILD

    return true;

returnFalse:
    updateMatrixHeightCache();
    screenUpdatingMode &= ~(SCRUPD_MANUAL_STACK | SCRUPD_SKIP_STACK_ONE_TIME);
    refreshScreen(301);
    return false;
  }
#endif //SAVE_SPACE_DM42_12PRIME


static void printTitles(longInteger_t input) {
  if(addFactorsToTSV) {
    #if !defined(TESTSUITE_BUILD)
      create_filename("");
      fnStrtoReg(filename_csv, TEMP_REGISTER_1);
      cancelFilename = true;
      fnP_All_Regs(PRN_TMP);

      convertLongIntegerToLongIntegerRegister(input, TEMP_REGISTER_1);
      fnP_All_Regs(PRN_TMP);

      char filename[50];
      strcpy(filename,"FACTORS:");
      fnStrtoReg(filename, TEMP_REGISTER_1);
      fnP_All_Regs(PRN_TMP);
    #endif //TESTSUITE_BUILD
  }
}


// Helper that extracts from REGISTER_X, validates, and performs factorization
// Parameter doSaveLastX: if true, calls saveLastX() before proceeding
// Returns true on success, false on failure
// All error reporting happens inside this function
static bool_t performPrimeFactorization(bool_t doSaveLastX) {
  iterations = true;
  addFactorsToTSV = false;
  loopp = 0;
  currentKeyCode = 255;
  real34_t lastAdded;

  longInteger_t currentNumber, tmp, temp1;

  longIntegerInit(tmp);
  longIntegerInit(temp1);

  if(!getIntArg(currentNumber, REGISTER_X)) {
    goto abort;
  }

  if(longIntegerIsZero(currentNumber)) {
    badDomainError(REGISTER_X);
    goto abort;
  }

  int32_t sign = longIntegerIsNegative(currentNumber);
  if(sign) {
    longIntegerSetPositiveSign(currentNumber);
  }

  longIntegerPowerUIntUInt(10,maximumPrime,tmp);
  longIntegerSubtract(currentNumber, tmp, tmp);   // (primeCandidate - 10^300) positive is too large
  if(longIntegerIsPositive(tmp)) {
    badDomainError(REGISTER_X);
    goto abort;
  }

  if(doSaveLastX && !saveLastX())
    goto abort;

  int32ToReal34(0,&lastAdded);
  FactorAdder_t faddr;
  initFactorAdder(&faddr);

  //determine if the TSV file is to be written
  longInteger_t lgInt;
  longIntegerInit(lgInt);
  stringToLongInteger("9999999999999999999999999999999999", 10, lgInt);
  addFactorsToTSV = (longIntegerCompare(currentNumber, lgInt) > 0);
  longIntegerFree(lgInt);
  printTitles(currentNumber);


  //capture the sign in the output matrix
  int32ToLongInteger(sign ? -1 : 1, tmp);
  if(!addFactor(tmp, REGISTER_X, &lastAdded, &faddr)) {
    goto abort;
  }


  longInteger_t queue[MAXIMUM_QUEUE_SIZE];
  int queue_start = 0, queue_end = 0;

  //initialize and prep second part
  longInteger_t temp, factor, quotient, tempPrePrimeRun;
  longIntegerInit(temp);
  longIntegerInit(factor);
  longIntegerInit(quotient);
  longIntegerInit(tempPrePrimeRun);
  // Initialize queue with the input number
  for(int i=0; i<MAXIMUM_QUEUE_SIZE; i++) {
    longIntegerInit(queue[i]);
  }

  #if !defined(TESTSUITE_BUILD)
    clearScreenOld(!clrStatusBar, clrRegisterLines, clrSoftkeys);
    force_refresh(force);
  #endif //TESTSUITE_BUILD

  // original command, prior to pre-run of primes. Retain here to test without the pre-run block
  //   longIntegerCopy(currentNumber, queue[queue_end++]);

  if(Factors_1_SmallPrimes) {
    // first do a pre-run, to do small prime checking
    for (uint16_t i = 0; i < smallPrimeListNumber; i++) {
      uint16_t smallP = smallPrimeList(i);
      while (mpz_divisible_ui_p(currentNumber, smallP)) {
        #if !defined(TESTSUITE_BUILD)
          loopp++;
          if(checkHalfSec()) {
            keepFileNameAlive();
            if(progressHalfSecUpdate_Integer(timed, "Factors: Small prime trial: p =",smallP, halfSec_clearZ, halfSec_clearT, halfSec_disp)) { //timed
              _showProgress(&lastAdded, currentNumber);
              dumpExponents(REGISTER_X, &faddr, 13);
              force_refresh(force);
            }
          }
          if(exitKeyWaiting() || programRunStop == PGM_WAITING) {
            progressHalfSecUpdate_Integer(force+1, "Interrupted: ",loopp, halfSec_clearZ, halfSec_clearT, halfSec_disp);
            programRunStop = PGM_WAITING;
            break;
          }
        #endif //!TESTSUITE_BUILD
                            #if defined(MONITOR_FACTORS)
                              printf("\nPrime factor: %u -> PrePrimeRun; Remaining currentNumber -> queue: ", smallP);
                            #endif //MONITOR_FACTORS
        mpz_divexact_ui(currentNumber, currentNumber, smallP);
                            #if defined(MONITOR_FACTORS)
                              mpz_out_str(stdout, 10, currentNumber);
                            #endif //MONITOR_FACTORS
        uInt32ToLongInteger((unsigned long)(smallP), tempPrePrimeRun);
        if(!addFactor(tempPrePrimeRun, REGISTER_X, &lastAdded, &faddr)) {
          goto cleanup;
        }
                            #if defined(MONITOR_FACTORS)
                              printf("\n");
                            #endif //MONITOR_FACTORS
      }
    } // end of small prime loop
  }




  // Early multiplier check using 32-bit perfect square test
  static const uint32_t multipliers[] = {
  1,   // Always include 1
  3, 5, 7, 11,
  15, 21, 33, 35, 55, 65, 77, 91, // 2-way composite square-free products
  105 }; // 3-way composite: 3*5*7

  if(Factors_2_PerfectSquare) {
    for(uint16_t i=0; i<nbrOfElements(multipliers); i++) {
      #if !defined(TESTSUITE_BUILD)
        loopp++;
        if(checkHalfSec()) {
          keepFileNameAlive();
          if(progressHalfSecUpdate_Integer(timed, "Factors: Perfect Sq trial: p =",multipliers[i], halfSec_clearZ, halfSec_clearT, halfSec_disp)) { //timed
            _showProgress(&lastAdded, currentNumber);
            dumpExponents(REGISTER_X, &faddr, 13);
            force_refresh(force);
          }
        }
        if(exitKeyWaiting() || programRunStop == PGM_WAITING) {
          progressHalfSecUpdate_Integer(force+1, "Interrupted: ",loopp, halfSec_clearZ, halfSec_clearT, halfSec_disp);
          programRunStop = PGM_WAITING;
          break;
        }
      #endif //!TESTSUITE_BUILD
      if(mpz_fits_uint_p(currentNumber)) {
        uint32_t k = multipliers[i];
        uint32_t n32;
        longIntegerToUInt32(currentNumber, n32);
        uint64_t kn = (uint64_t)k * (uint64_t)n32;
        #if defined(MONITOR_FACTORS)
          printf("Early squares trial: currentNumber: %" PRIu32 ", sqtest: %" PRIu32 " trial square: %" PRIu64 "\n",n32, k, kn);
          fflush(stdout);
        #endif
        uint32_t root;
        if(is_perfect_square_uint32(kn, &root)) {
          // Skip trivial perfect square 1 * 1 = 1
          if(!(k == 1 && root == 1)) {
            #if defined(MONITOR_FACTORS)
              printf("Perfect square detected early: %u * %u = %" PRIu64 "\n", k, n32, kn);
              fflush(stdout);
            #endif
            // Use gcd to extract non-trivial factor safely
            longInteger_t gcd;
            longIntegerInit(gcd);
            mpz_gcd_ui(gcd, currentNumber, root);
            if (longIntegerCompareUInt(gcd, 1) > 0) {
                // Inject a trivial factor
                if (!addFactor(gcd, REGISTER_X, &lastAdded, &faddr)) {
                    longIntegerFree(gcd);
                    goto cleanup;
                }
                mpz_divexact(currentNumber, currentNumber, gcd);
                longIntegerFree(gcd);
                break;
            }
            longIntegerFree(gcd);
          }
        }
      }
    }
  }

  //SQUFOF SHANKS & POLLARD RHO FACTORISATION START
  longIntegerCopy(currentNumber, queue[queue_end]);
  queue_end = (queue_end + 1) % MAXIMUM_QUEUE_SIZE;
                          #if defined(MONITOR_FACTORS)
                            printf("Factorizing: ");
                            mpz_out_str(stdout, 10, currentNumber);
                            printf("\nFactors found: ");
                          #endif //MONITOR_FACTORS

  longInteger_t current;
  longIntegerInit(current);
  while (queue_start != queue_end) {
    longIntegerCopy(queue[queue_start], current);
    queue_start = (queue_start + 1) % MAXIMUM_QUEUE_SIZE;
                        #if defined(MONITOR_FACTORS)
                          printf("loopp=%d queue_start=%d queue_end=%d\n",loopp, queue_start, queue_end);
                          mpz_out_str(stdout, 10, current);
                          printf(" ");
                        #endif //MONITOR_FACTORS

    #if !defined(TESTSUITE_BUILD)
      loopp++;
      if(checkHalfSec()) {
        keepFileNameAlive();
        if(progressHalfSecUpdate_Integer(timed, "Factors: Shanks/Pollard: n =",loopp, halfSec_clearZ, halfSec_clearT, halfSec_disp)) { //timed
          _showProgress(&lastAdded, current);
          dumpExponents(REGISTER_X, &faddr, 13);
          force_refresh(force);
        }
      }
      if(exitKeyWaiting() || programRunStop == PGM_WAITING) {
        progressHalfSecUpdate_Integer(force+1, "Interrupted: ",loopp, halfSec_clearZ, halfSec_clearT, halfSec_disp);
        programRunStop = PGM_WAITING;
        break;
      }
    #endif //!TESTSUITE_BUILD


    // Skip if current is 1
    if(longIntegerCompareUInt(current, 1) == 0) {
                        #if defined(MONITOR_FACTORS)
                          printf("Skip, current is 1\n");
                        #endif //MONITOR_FACTORS
      continue;
    }

    // Check if current is prime
    if(longIntegerProbabPrime(current)) {
                      #if defined(MONITOR_FACTORS)
                        mpz_out_str(stdout, 10, current);
                        printf(" ");
                      #endif //MONITOR_FACTORS
      if(!addFactor(current, REGISTER_X, &lastAdded, &faddr)) {
        goto doneWhile;
      }
                      #if defined(MONITOR_FACTORS)
                        printf("Skip, current is prime\n");
                      #endif //MONITOR_FACTORS
      continue;
    }

    // Attempt to find a factor using SQUFOF
    SQUFOF(factor, current, lastAdded);
    if (longIntegerCompareUInt(factor, 0) == 0 || longIntegerCompare(factor, current) == 0) {
      // SQUFOF failed; treat current as prime
                      #if defined(MONITOR_FACTORS)
                        printf("SQUFOF failed; treat current as prime: ");
                        mpz_out_str(stdout, 10, current);
                        printf(" ");
                      #endif //MONITOR_FACTORS
      if(!addFactor(current, REGISTER_X, &lastAdded, &faddr)) {
        goto doneWhile;
      }
                      #if defined(MONITOR_FACTORS)
                        printf("current considered prime, next\n");
                      #endif //MONITOR_FACTORS
      continue;
    }

    // Divide current by the found factor
    mpz_divexact(quotient, current, factor);

    // Enqueue the factor and quotient for further factorization
    int next_end = (queue_end + 1) % MAXIMUM_QUEUE_SIZE;
    int next_next_end = (next_end + 1) % MAXIMUM_QUEUE_SIZE;
    if (next_next_end != queue_start) {
                        #if defined(MONITOR_FACTORS)
                          printf("Enqueueing: %u -> %u : factor: ",queue_end, queue_start);
                          mpz_out_str(stdout, 10, factor);
                          printf(" quotient: ");
                          mpz_out_str(stdout, 10, quotient);
                          printf("\n");
                        #endif //MONITOR_FACTORS
      longIntegerCopy(factor, queue[queue_end]);
      queue_end = next_end;
      longIntegerCopy(quotient, queue[queue_end]);
      queue_end = next_next_end;
    }
    else {
      if(lastErrorCode != 0) break;
      displayCalcErrorMessage(ERROR_NOT_ENOUGH_MEMORY_FOR_NEW_MATRIX, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Not enough memory for a %" PRIu32 STD_CROSS "%" PRIu32 " matrix", 1, 1);
        moreInfoOnError("In function performPrimeFactorization:  Queue overflow:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      break;
    }
  }

doneWhile:
  longIntegerFree(current);

cleanup:
  longIntegerFree(temp);
  longIntegerFree(factor);
  longIntegerFree(quotient);
  longIntegerFree(tempPrePrimeRun);
  for(int i=0; i<MAXIMUM_QUEUE_SIZE; i++) {
    longIntegerFree(queue[i]);
  }

  dumpExponents(REGISTER_X, &faddr, 65535);
  delCol1RealMatrixX();

abort:
  uInt32ToLongInteger(1u, tmp);
  convertLongIntegerToLongIntegerRegister(tmp, TEMP_REGISTER_1);
  longIntegerFree(tmp);
  longIntegerFree(temp1);
  longIntegerFree(currentNumber);
  cancelFilename = true;
  iterations = false;
  return false;
}


void fnPrimeFactors(uint16_t unusedButMandatoryParameter) {
  performPrimeFactorization(true);  // Call with saveLastX enabled
}

//-------------------------------------------------------------------
/*
 * Pollard's Rho Integer Factorization with perfect power detection
 *
 * Based on: https://en.wikipedia.org/wiki/Pollard%27s_rho_algorithm
 * jaymos 2025
 * The point of this approach of coding is to let Pollard's algo operate step wise,
 *   interjected while doing the Shanks algo. Shanks was done first, and found to get
 *   stuck with relatively simple factor not being found. Pollard will in PARALLEL
 *   scan the current number and whichever Shanks or Pollard finding a factor,
 *   will run the addFactor function, and continue to look in the same way sharing the
 *   processor.
 * The idea comes from 'linear programming' in the 80's afaik, where a simplistic
 * single threaded industrial controller or PLC does 'multi-tasking' without formal time
 * slicing and mult-threadedness.
 *
 * In summary, Shanks was written, then Pollard was written to fill in and find factors
 * in parallel without breaking the Shanks factor finding loop.
 */

/*
 * Print pollard status to screen
 */
char* pollard_status(factors_status_t st) {
  switch(st){
    case FACTORS_SETUP:   return("SETUP  "); break;
    case FACTORS_ITERATE: return("ITERATE"); break;
    case FACTORS_RESET:   return("RESET  "); break;
    case FACTORS_DONE:    return("DONE   "); break;
    case FACTORS_FAIL:    return("FAIL   "); break;
    default:return("");break;
  }
}
/*
 * Polynomial function used in Pollard's Rho:
 * f(x) = x^2 + c mod n
 */
static void f(longInteger_t result, longInteger_t x, longInteger_t c, const longInteger_t n) {
  longIntegerSquare(x, result);                     // x^2
  longIntegerAdd(result, c, result);                // + c
  longIntegerModulo(result, n, result);             // mod n
}

/*
 * Initializes the Pollard state structure. Must be called before using the step or update functions.
 */
void pollard_init(pollard_t *self) {
  longIntegerInit(self->n);
  longIntegerInit(self->x);
  longIntegerInit(self->y);
  longIntegerInit(self->c);
  longIntegerInit(self->d);
  longIntegerInit(self->tmp);
  self->iteration = 0;
  self->attempt = 0;
  #define max_iterations 1000000      // Reset current attempt after this many iterations
  #define max_attempts   1000         // Max allowed retried attempts before FAIL. 1000 meaning literally infinity for the hardware speed ...
  #define maxIter (self->attempt <= 25 ? max_iterations / 10 : (self->attempt <= 50 ? max_iterations : max_iterations * 4))

  // Deterministically seed the RNG for repeatability.
  // These values are the defaults specified by the algorithm, so pretty uninteresting.
  // It doesn't matter much what values are used so long as the inc is odd.
  self->rng.state = 0x853c49e6748fea9bull;
  self->rng.inc = 0xda3e39cb94b95bdbull;
}

void mpz_urandomm_pcg32(mpz_t rop, pcg32_random_t* rng, const mpz_t n) { // Get bit size of n to determine how many random bits we need. Generate enough random bits (use multiple PCG32 calls if needed)
  size_t n_bits = mpz_sizeinbase(n, 2);
  mpz_t temp;
  mpz_init(temp);
  size_t bits_generated = 0;
  while (bits_generated < n_bits + 32) {  // Extra 32 bits for better distribution
      uint32_t random_val = pcg32_random_r(rng);
      mpz_mul_2exp(temp, temp, 32);
      mpz_add_ui(temp, temp, random_val);
      bits_generated += 32;
  }
  mpz_mod(rop, temp, n);
  mpz_clear(temp);
}
/*
 * Frees all GMP variables and RNG state.
 */
void pollard_clear(pollard_t *self) {
  longIntegerFree(self->n);
  longIntegerFree(self->x);
  longIntegerFree(self->y);
  longIntegerFree(self->c);
  longIntegerFree(self->d);
  longIntegerFree(self->tmp);
}
/*
 * Updates the target number to factor (n), and resets the internal state automatically.
 */
void pollard_update_n(pollard_t *self, const longInteger_t new_n) {
  longIntegerCopy(new_n, self->n);
  self->attempt = 0;
  self->iteration = 0;

  uInt32ToLongInteger(1, self->c);        // Start with f(x) = x² + 1
  mpz_urandomm_pcg32(self->x, &self->rng, self->n);
  longIntegerCopy(self->x, self->y);
  uInt32ToLongInteger(1, self->d);
}
/*
 * Performs a number 'steps' of Pollard's Rho iterations within one main iteration.
 *   Currently 10 steps of pollard to one step of Shanks; Shanks being on the back burner.
 * Returns a result struct with status and diagnostic info.
 * - If a factor is found, status will be FACTORS_DONE.
 * - If more work is needed, returns FACTORS_ITERATE or FACTORS_SETUP.
 */
factors_result_t pollard_step(pollard_t *self, longInteger_t factor, factors_status_t instruction, int steps) {
  factors_result_t result;
  result.status = FACTORS_ITERATE;
  result.iterations_this_call = 0;
  result.total_iterations = self->iteration;
  result.attempts = self->attempt;
  // First-time or forced re-setup
if (instruction == FACTORS_RESET || (instruction == FACTORS_SETUP && self->iteration >= maxIter)) {
    if (self->attempt++ >= max_attempts) {
      result.status = FACTORS_FAIL;
      return result;
    }

  // Try different polynomial function based on attempt number
  if (self->attempt % 3 == 0) {
    uInt32ToLongInteger(1, self->c);      // f(x) = x² + 1
  } else if (self->attempt % 3 == 1) {
    uInt32ToLongInteger(2, self->c);      // f(x) = x² + 2
  } else {
    mpz_urandomm_pcg32(self->c, &self->rng, self->n); // Random c
  }
    mpz_urandomm_pcg32(self->x, &self->rng, self->n);
    longIntegerCopy(self->x, self->y);
    uInt32ToLongInteger(1, self->d);
    self->iteration = 0;
    result.status = FACTORS_ITERATE;
    return result;
  }
  // Perform up to `steps` iterations
  if (instruction == FACTORS_ITERATE) {
    for (int i = 0; i < steps; ++i) {
      if (++self->iteration >= maxIter) {
        result.status = FACTORS_SETUP; // Too long without result — reseed
        break;
      }
      // Floyd's cycle detection: tortoise and hare
      f(self->x, self->x, self->c, self->n);         // tortoise: 1 step
      f(self->y, self->y, self->c, self->n);         // hare: 2 steps
      f(self->y, self->y, self->c, self->n);
      longIntegerSubtract(self->x, self->y, self->tmp);
      mpz_abs(self->tmp, self->tmp);
      longIntegerGcd(self->tmp, self->n, self->d);
      // Factor found!
      if (longIntegerCompareUInt(self->d, 1) > 0 && longIntegerCompare(self->d, self->n) < 0) {
        longIntegerCopy(self->d, factor);
        result.status = FACTORS_DONE;
        break;
      }
      // Fail — retry from new seed
      if (longIntegerCompare(self->d, self->n) == 0) {
        result.status = FACTORS_SETUP;
        break;
      }
      result.iterations_this_call++;
    }
  }
  result.total_iterations = self->iteration;
  result.attempts = self->attempt;
  return result;
}

#endif // SAVE_SPACE_DM42_12PRIME