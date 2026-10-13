// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file algdep.c
 ***********************************************/

#include "c47.h"

#if !defined(OPTION_ALGDEP)
  void fnAlgdep (uint16_t maxDegree)                     {}
  void fnLindep (uint16_t unusedButMandatoryParameter)   {}
  const char *algdepPolynomialString(void)               {return "";}
#else

// A register value is exactly N/10^k for an integer N of at most 34 digits, so the lattice entries and the residual check are both exact and no decNumber
// arithmetic appears below the input parse. That removes the rounding question entirely and keeps the shared contexts untouched.

#define ALGDEP_MAX_VECTORS  (ALGDEP_MAX_DEGREE + 1)
#define ALGDEP_MAX_COLS     (ALGDEP_MAX_DEGREE + 2)
#define ALGDEP_POLY_LEN     96

// LLL's potential bounds the swap count. Blowing this means an intermediate divided inexactly, which GMP does not flag, so the reduction would otherwise spin
// forever. A hang on a handheld is indistinguishable from a dead calculator: fail loudly instead.
#define ALGDEP_GUARD(n)     (20000L * (n))

typedef struct {
  longInteger_t b  [ALGDEP_MAX_VECTORS][ALGDEP_MAX_COLS];
  longInteger_t d  [ALGDEP_MAX_VECTORS + 1];
  longInteger_t lam[ALGDEP_MAX_VECTORS + 1][ALGDEP_MAX_VECTORS + 1];
  longInteger_t t0, t1, t2, t3, dotProd, dotAcc;
  longInteger_t coeff[ALGDEP_MAX_VECTORS];
  longInteger_t powN [ALGDEP_MAX_VECTORS];   // N^i, or the scaled vector entries under LINDEP
  longInteger_t p10  [ALGDEP_MAX_VECTORS];   // 10^(kdec*(degree-i)), or the per-entry denominators under LINDEP
  int16_t       n, m;
} algdepLattice_t;

static char algdepPoly[ALGDEP_POLY_LEN] = "";

const char *algdepPolynomialString(void) {
  return algdepPoly;
}


// ---------------------------------------------------------------------------------------------------------------------------------------- lattice lifetime

// Every member up to n/m is a longInteger_t laid out contiguously, so one flat pass initialises and clears them all. The nested per-array loops this replaces
// cost 376 bytes between them for work that is one loop.
#define ALGDEP_MPZ_COUNT (offsetof(algdepLattice_t, n) / sizeof(longInteger_t))

static algdepLattice_t *latticeAlloc(void) {
  algdepLattice_t *L = allocC47Blocks(TO_BLOCKS(sizeof(algdepLattice_t)));
  if(L == NULL) {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    return NULL;
  }
  longInteger_t *p = (longInteger_t *)L;
  for(size_t i = 0; i < ALGDEP_MPZ_COUNT; i++) {
    longIntegerInit(p[i]);
  }
  return L;
}

// Every mpz_init above needs its mpz_clear on every exit path: the test suite fails the run on a GMP leak through gmpMemInBytes.
static void latticeFree(algdepLattice_t *L) {
  longInteger_t *p = (longInteger_t *)L;
  for(size_t i = 0; i < ALGDEP_MPZ_COUNT; i++) {
    longIntegerFree(p[i]);
  }
  freeC47Blocks(L, TO_BLOCKS(sizeof(algdepLattice_t)));
}


// ---------------------------------------------------------------------------------------------------------------------------- integral LLL, Cohen 2.6.7

// Uses its own product and accumulator. The Gram-Schmidt block passes t0 as the destination, so aliasing the scratch here makes divexact divide inexactly and
// the reduction never terminates.
static void latticeDot(algdepLattice_t *L, mpz_ptr r, int i, int j) {
  mpz_set_ui(L->dotAcc, 0);
  for(int t = 0; t < L->m; t++) {
    mpz_mul(L->dotProd, L->b[i - 1][t], L->b[j - 1][t]);
    mpz_add(L->dotAcc, L->dotAcc, L->dotProd);
  }
  mpz_set(r, L->dotAcc);
}

static void latticeReduceStep(algdepLattice_t *L, int k, int l) {
  mpz_abs(L->t0, L->lam[k][l]);
  mpz_mul_2exp(L->t0, L->t0, 1);
  if(mpz_cmp(L->t0, L->d[l]) <= 0) {
    return;
  }

  mpz_mul_2exp(L->t1, L->lam[k][l], 1);                      // q = nint(lam[k][l] / d[l]), by floor((2*lam + d) / (2*d))
  mpz_add(L->t1, L->t1, L->d[l]);
  mpz_mul_2exp(L->t2, L->d[l], 1);
  mpz_fdiv_q(L->t1, L->t1, L->t2);

  for(int t = 0; t < L->m; t++) {
    mpz_mul(L->t0, L->t1, L->b[l - 1][t]);
    mpz_sub(L->b[k - 1][t], L->b[k - 1][t], L->t0);
  }
  mpz_mul(L->t0, L->t1, L->d[l]);
  mpz_sub(L->lam[k][l], L->lam[k][l], L->t0);
  for(int i = 1; i <= l - 1; i++) {
    mpz_mul(L->t0, L->t1, L->lam[l][i]);
    mpz_sub(L->lam[k][i], L->lam[k][i], L->t0);
  }
}

static void latticeSwap(algdepLattice_t *L, int k, int kmax) {
  for(int t = 0; t < L->m; t++) {
    mpz_swap(L->b[k - 1][t], L->b[k - 2][t]);
  }
  for(int j = 1; j <= k - 2; j++) {
    mpz_swap(L->lam[k][j], L->lam[k - 1][j]);
  }

  mpz_mul(L->t0, L->d[k - 2], L->d[k]);                      // t0 = (d[k-2]*d[k] + lam^2) / d[k-1], the new d[k-1]
  mpz_mul(L->t1, L->lam[k][k - 1], L->lam[k][k - 1]);
  mpz_add(L->t0, L->t0, L->t1);
  mpz_divexact(L->t0, L->t0, L->d[k - 1]);

  for(int i = k + 1; i <= kmax; i++) {
    mpz_set(L->t1, L->lam[i][k]);
    mpz_mul(L->t2, L->d[k], L->lam[i][k - 1]);
    mpz_mul(L->t3, L->lam[k][k - 1], L->t1);
    mpz_sub(L->t2, L->t2, L->t3);
    mpz_divexact(L->lam[i][k], L->t2, L->d[k - 1]);
    mpz_mul(L->t2, L->t0, L->t1);
    mpz_mul(L->t3, L->lam[k][k - 1], L->lam[i][k]);          // deliberately the new lam[i][k]
    mpz_add(L->t2, L->t2, L->t3);
    mpz_divexact(L->lam[i][k - 1], L->t2, L->d[k]);
  }
  mpz_set(L->d[k - 1], L->t0);
}

// Returns false on dependent input or on a guard trip; d[k] == 0 cannot arise from the fnAlgdep lattice, whose identity block makes the rows independent.
static bool_t latticeReduce(algdepLattice_t *L) {
  int  n = L->n, k = 2, kmax = 1;
  long guard = 0;

  mpz_set_ui(L->d[0], 1);
  latticeDot(L, L->d[1], 1, 1);
  if(mpz_sgn(L->d[1]) == 0) {
    return false;
  }

  while(k <= n) {
    if(k > kmax) {
      kmax = k;
      for(int j = 1; j <= k; j++) {
        latticeDot(L, L->t0, k, j);
        for(int i = 1; i <= j - 1; i++) {
          mpz_mul(L->t1, L->d[i], L->t0);
          mpz_mul(L->t2, L->lam[k][i], L->lam[j][i]);
          mpz_sub(L->t1, L->t1, L->t2);
          mpz_divexact(L->t0, L->t1, L->d[i - 1]);
        }
        if(j < k) {
          mpz_set(L->lam[k][j], L->t0);
        }
        else {
          mpz_set(L->d[k], L->t0);
        }
      }
      if(mpz_sgn(L->d[k]) == 0) {
        return false;
      }
    }

    for(;;) {
      if(++guard > ALGDEP_GUARD(n)) {                        // counted here and not in the outer loop: consecutive swaps cycle in this loop alone, so a
        return false;                                        // corrupted lattice would spin here without the outer loop ever seeing it
      }
      latticeReduceStep(L, k, k - 1);
      mpz_mul(L->t0, L->d[k], L->d[k - 2]); mpz_mul_2exp(L->t0, L->t0, 2);         // swap when 4*d[k]*d[k-2] < 3*d[k-1]^2 - 4*lam[k][k-1]^2
      mpz_mul(L->t1, L->d[k - 1], L->d[k - 1]); mpz_mul_ui(L->t1, L->t1, 3);
      mpz_mul(L->t2, L->lam[k][k - 1], L->lam[k][k - 1]); mpz_mul_2exp(L->t2, L->t2, 2);
      mpz_sub(L->t1, L->t1, L->t2);
      if(mpz_cmp(L->t0, L->t1) < 0) {
        latticeSwap(L, k, kmax);
        k = (k - 1 > 2) ? k - 1 : 2;
      }
      else {
        for(int l = k - 2; l >= 1; l--) {
          latticeReduceStep(L, k, l);
        }
        k++;
        break;
      }
    }
  }
  return true;
}


// ------------------------------------------------------------------------------------------------------------------------------------------- input parse

// x = n / 10^k exactly, k >= 0. decNumberToString emits plain decimal or an E form; both are handled here rather than assuming the plain one.
static bool_t realToScaledInteger(const real_t *x, longInteger_t n, int32_t *k) {
  char    str[128], digits[128];
  int32_t di = 0, frac = 0, expo = 0, expSign = 1;
  bool_t  neg = false, seenDot = false, seenExp = false;

  realToString(x, str);

  const char *p = str;
  if(*p == '-') {
    neg = true;
    p++;
  }
  else if(*p == '+') {
    p++;
  }

  for(; *p; p++) {
    if(*p == '.') {
      seenDot = true;
      continue;
    }
    if(*p == 'E' || *p == 'e') {
      seenExp = true;
      p++;
      if(*p == '-') {
        expSign = -1;
        p++;
      }
      else if(*p == '+') {
        p++;
      }
      for(; *p >= '0' && *p <= '9'; p++) {
        expo = expo * 10 + (*p - '0');
      }
      break;
    }
    if(*p < '0' || *p > '9') {
      return false;                                          // NaN, Infinity, or anything else that is not a finite number
    }
    if(di < (int32_t)sizeof(digits) - 1) {
      digits[di++] = *p;
    }
    if(seenDot) {
      frac++;
    }
  }
  digits[di] = 0;
  if(di == 0) {
    return false;
  }
  if(seenExp) {
    expo *= expSign;
  }

  stringToLongInteger(digits, 10, n);
  if(neg) {
    longIntegerChangeSign(n);
  }

  *k = frac - expo;
  if(*k < 0) {
    longInteger_t scale;
    longIntegerInit(scale);
    longIntegerPowerUIntUInt(10, (uint32_t)(-*k), scale);
    mpz_mul(n, n, scale);
    longIntegerFree(scale);
    *k = 0;
  }
  return true;
}


// -------------------------------------------------------------------------------------------------------------------------------------------- the search

typedef struct {
  bool_t  found;
  int32_t degree;
  int32_t margin;
} algdepOutcome_t;


// Scores one reduced row against the acceptance rule, common to both commands: the i-th value is powN[i]*p10[i], which fnAlgdep fills with N^i and
// 10^(kdec*(degree-i)) and fnLindep fills with the entry and its denominator. Sharing it is not only smaller, it is what stops the two paths drifting apart:
// the residual-is-zero case was fixed once in each of them before this existed.
// Reuses t0..t3 and dotProd, which are free once the reduction has finished.
static __attribute__((noinline)) bool_t scoreCandidate(algdepLattice_t *L, int32_t cand, int32_t count, int32_t *marginOut) {
  #define RESIDUAL L->t0
  #define TYPICAL  L->t1
  #define HEIGHT   L->t2
  #define TERM     L->t3
  #define ABSCOEF  L->dotProd
  bool_t allZero = true;

  mpz_set_ui(RESIDUAL, 0);
  mpz_set_ui(TYPICAL, 0);
  mpz_set_ui(HEIGHT, 0);

  for(int32_t i = 0; i < count; i++) {
    if(mpz_sgn(L->b[cand][i]) != 0) {
      allZero = false;
    }
    mpz_mul(TERM, L->powN[i], L->p10[i]);                    // the i-th value, scaled to the common denominator

    mpz_mul(ABSCOEF, L->b[cand][i], TERM);
    mpz_add(RESIDUAL, RESIDUAL, ABSCOEF);                    // exact: the relation evaluated at the input, times that denominator

    mpz_abs(ABSCOEF, L->b[cand][i]);
    mpz_abs(TERM, TERM);
    mpz_mul(TERM, TERM, ABSCOEF);
    mpz_add(TYPICAL, TYPICAL, TERM);                         // the size a random combination of this height would reach

    mpz_abs(ABSCOEF, L->b[cand][i]);
    if(mpz_cmp(ABSCOEF, HEIGHT) > 0) {
      mpz_set(HEIGHT, ABSCOEF);
    }
  }
  if(allZero || mpz_sgn(TYPICAL) == 0) {
    return false;
  }

  int32_t confidence = (mpz_sgn(RESIDUAL) == 0)
                     ? ALGDEP_MAX_CONFIDENCE
                     : (int32_t)longIntegerBase10Digits(TYPICAL) - (int32_t)longIntegerBase10Digits(RESIDUAL);
  if(confidence > ALGDEP_MAX_CONFIDENCE) {
    confidence = ALGDEP_MAX_CONFIDENCE;
  }

  *marginOut = confidence - count * (int32_t)longIntegerBase10Digits(HEIGHT);
  return *marginOut >= ALGDEP_MARGIN;
  #undef RESIDUAL
  #undef TYPICAL
  #undef HEIGHT
  #undef TERM
  #undef ABSCOEF
}


// Builds the lattice for one degree, reduces it, and tests every reduced row. A candidate is reported only when the residual beats a random integer
// combination of the same height by ALGDEP_MARGIN digits more than the relation costs to write down.
static void trySingleDegree(algdepLattice_t *L, const longInteger_t bigN, int32_t kdec, int32_t degree, algdepOutcome_t *out) {
  longInteger_t scale, den, num, quo, term;

  L->n = (int16_t)(degree + 1);
  L->m = (int16_t)(degree + 2);

  longIntegerInit(scale); longIntegerInit(den); longIntegerInit(num); longIntegerInit(quo); longIntegerInit(term);

  longIntegerPowerUIntUInt(10, ALGDEP_SCALE, scale);
  for(int i = 0; i <= degree; i++) {                         // N^i and 10^(kdec*(degree-i)) depend only on i, so they are built once here and read back in the
    mpz_pow_ui(L->powN[i], bigN, (unsigned long)i);          // candidate loop below. Recomputing them per candidate cost (degree+1) squared big-integer
    longIntegerPowerUIntUInt(10, (uint32_t)(kdec * (degree - i)), L->p10[i]);   // exponentiations instead of (degree+1).
  }
  for(int i = 0; i <= degree; i++) {
    for(int j = 0; j <= degree; j++) {
      mpz_set_ui(L->b[i][j], (i == j) ? 1 : 0);
    }
    longIntegerPowerUIntUInt(10, (uint32_t)(kdec * i), den);
    mpz_mul(num, scale, L->powN[i]);
    mpz_mul_2exp(quo, num, 1); mpz_add(quo, quo, den);       // nint(num/den)
    mpz_mul_2exp(term, den, 1);
    mpz_fdiv_q(quo, quo, term);
    mpz_set(L->b[i][degree + 1], quo);
  }

  if(!latticeReduce(L)) {
    goto cleanup;
  }

  for(int32_t cand = 0; cand <= degree; cand++) {
    int32_t margin = 0;
    if(!scoreCandidate(L, cand, degree + 1, &margin)) {
      continue;
    }
    mpz_set_ui(term, 0);                                     // primitive part, then a positive leading coefficient
    for(int i = 0; i <= degree; i++) {
      longIntegerGcd(term, L->b[cand][i], term);
    }
    for(int i = 0; i <= degree; i++) {
      if(mpz_sgn(term) != 0) {
        mpz_divexact(L->coeff[i], L->b[cand][i], term);
      }
      else {
        mpz_set(L->coeff[i], L->b[cand][i]);
      }
    }
    if(mpz_sgn(L->coeff[degree]) < 0) {
      for(int i = 0; i <= degree; i++) {
        mpz_neg(L->coeff[i], L->coeff[i]);
      }
    }
    out->found   = true;
    out->degree  = degree;
    out->margin  = margin;
    break;
  }

cleanup:
  longIntegerFree(scale); longIntegerFree(den); longIntegerFree(num); longIntegerFree(quo); longIntegerFree(term);
}

// Sweeps upward so the first hit is the minimal polynomial and not a multiple of it.
static bool_t algdepSearch(algdepLattice_t *L, const real_t *x, int32_t maxDegree, algdepOutcome_t *out) {
  longInteger_t bigN;
  int32_t       kdec = 0;

  out->found = false;
  longIntegerInit(bigN);
  if(!realToScaledInteger(x, bigN, &kdec)) {
    longIntegerFree(bigN);
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function algdepSearch:", "the value in X is not finite.", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }

  if(mpz_sgn(bigN) == 0) {                                   // zero is the root of x. Its row scores typical == 0, which scoreCandidate cannot tell from an
    mpz_set_ui(L->coeff[0], 0);                              // empty candidate, so it is answered here instead of being lost in the sweep.
    mpz_set_ui(L->coeff[1], 1);
    out->found   = true;
    out->degree  = 1;
    out->margin  = ALGDEP_MAX_CONFIDENCE;
    longIntegerFree(bigN);
    return true;
  }

  // A relation of degree d and height H costs (d+1)log10(H) + d*log10|x| digits to specify, and only 34 are available. The magnitude term alone can exceed the
  // budget: for x = 1e400 no degree is possible at all. Testing it here refuses in constant time instead of reducing a lattice to reach the same answer, and it
  // keeps 10^(kdec*i) bounded, which matters because GMP aborts rather than failing when an allocation cannot be met.
  int32_t magnitude = (int32_t)longIntegerBase10Digits(bigN) - kdec;
  if(magnitude < 0) {
    magnitude = -magnitude;
  }

  for(int32_t degree = 1; degree <= maxDegree && !out->found; degree++) {
    if(magnitude > 0 && degree * magnitude > ALGDEP_MAX_CONFIDENCE - ALGDEP_MARGIN) {
      break;                                                 // every higher degree is worse, so the sweep is finished
    }
    trySingleDegree(L, bigN, kdec, degree, out);
    if(exitKeyWaiting()) {
      break;
    }
  }
  longIntegerFree(bigN);
  return out->found;
}


// ------------------------------------------------------------------------------------------------------------------------------------- polynomial string

// Renders the recovered relation as x^3 - x - 1 rather than a column of numbers. Superscript digits and the minus sign are existing two byte glyphs.
static void buildPolynomialString(algdepLattice_t *L, int32_t degree) {
  char    number[64];
  int32_t at = 0;
  bool_t  first = true;

  algdepPoly[0] = 0;
  for(int32_t i = degree; i >= 0; i--) {
    if(mpz_sgn(L->coeff[i]) == 0) {
      continue;
    }
    bool_t negative = mpz_sgn(L->coeff[i]) < 0;

    if(first) {
      if(negative) {
        xcopy(algdepPoly + at, "-", 1);
        at += 1;
      }
    }
    else {
      xcopy(algdepPoly + at, negative ? " - " : " + ", 3);
      at += 3;
    }

    mpz_abs(L->t0, L->coeff[i]);
    if(mpz_cmp_ui(L->t0, 1) != 0 || i == 0) {                // a unit coefficient is written only when it is the constant term
      longIntegerToString(L->t0, 10, number);
      int32_t len = (int32_t)stringByteLength(number);
      if(at + len >= ALGDEP_POLY_LEN - 8) {
        at = 0;
        break;
      }
      xcopy(algdepPoly + at, number, len);
      at += len;
    }
    if(i >= 1) {
      if(at + 2 >= ALGDEP_POLY_LEN - 8) {
        at = 0;
        break;
      }
      xcopy(algdepPoly + at, "x", 1);
      at += 1;
    }
    if(i >= 2) {                                             // exponent 1 is implied, so it is never printed
      char    sup[8];
      int32_t si = 0, e = i;
      if(e >= 10) {
        sup[si++] = '\xa1';
        sup[si++] = (char)(0x60 + e / 10);
      }
      sup[si++] = '\xa1';
      sup[si++] = (char)(0x60 + e % 10);
      if(at + si >= ALGDEP_POLY_LEN - 2) {
        at = 0;
        break;
      }
      xcopy(algdepPoly + at, sup, si);
      at += si;
    }
    first = false;
  }
  algdepPoly[at] = 0;
}


// ------------------------------------------------------------------------------------------------------------------------------------------- the commands

static void reportNoRelation(const char *function, const char *what) {
  displayCalcErrorMessage(ERROR_NO_ROOT_FOUND, ERR_REGISTER_LINE);
  #if (EXTRA_INFO_ON_CALC_ERROR == 1)
    moreInfoOnError(function, what, "to the precision the input carries.", NULL);
  #else // EXTRA_INFO_ON_CALC_ERROR != 1
    (void)function; (void)what;
  #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
}

// The result writes deliberately skip adjustResult: its SDIGS rounding would turn an exact seven digit coefficient into noise at SDIGS 6, with no error and
// no warning. Nothing else in it applies here either: the result is real, and every error path returns before a result is written. The search itself never
// reads significantDigits, so no context needs forcing. algdep_cov.txt pins the exactness at SD=6.
void fnAlgdep(uint16_t maxDegree) {
  real_t           x;
  algdepLattice_t *L;
  algdepOutcome_t  out;

  if(maxDegree < 1 || maxDegree > ALGDEP_MAX_DEGREE) {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
    return;
  }
  if(!getRegisterAsReal(REGISTER_X, &x)) {
    return;
  }
  if(realIsSpecial(&x)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    return;
  }
  if(!saveLastX()) {                                         // the input is consumed, so LASTx carries it, as fnSlvq's does; a refusal is an error, and runFunction's undo()
    // restores LASTx with the rest of the stack, so on that path LASTx keeps what it had
    return;
  }
  if((L = latticeAlloc()) == NULL) {
    return;
  }

  bool_t found = algdepSearch(L, &x, (int32_t)maxDegree, &out);

  if(!found) {
    latticeFree(L);
    reportNoRelation("In function fnAlgdep:", "no integer polynomial of the requested degree has this value as a root,");
    return;
  }

  buildPolynomialString(L, out.degree);

  // Highest degree first, the coefficient-vector convention SLVP, SLVQ and SLVC read: SLVC consumes X as it stands,
  // and for a quadratic v3->zyx lands a, b, c in Z, Y, X, which is the stack order SLVQ reads.
  real34Matrix_t matrix;
  liftStack();
  if(initMatrixRegister(REGISTER_X, 1, (uint16_t)(out.degree + 1), false)) {
    linkToRealMatrixRegister(REGISTER_X, &matrix);
    for(int32_t i = 0; i <= out.degree; i++) {
      convertLongIntegerToReal34(L->coeff[out.degree - i], &matrix.matrixElements[i]);
    }
    temporaryInformation = TI_ALGDEP_POLY;                   // only when the matrix write succeeded, so a RAM full error keeps its message line
  }
  reallocateRegister(REGISTER_Y, dtReal34, 0, amNone);
  int32ToReal34(out.margin, REGISTER_REAL34_DATA(REGISTER_Y));

  latticeFree(L);
}


// Integer relation over a supplied vector: same kernel, but the lattice column carries the vector entries rather than the powers of one value. The result
// is a row, as fnAlgdep's is, whichever way the input vector lay.
void fnLindep(uint16_t unusedButMandatoryParameter) {
  real34Matrix_t   matrix;
  algdepLattice_t *L;

  if(getRegisterDataType(REGISTER_X) != dtReal34Matrix) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    return;
  }
  linkToRealMatrixRegister(REGISTER_X, &matrix);

  uint16_t count = (matrix.header.matrixRows == 1) ? matrix.header.matrixColumns
                 : (matrix.header.matrixColumns == 1) ? matrix.header.matrixRows
                 : 0;
  if(count < 2 || count > ALGDEP_MAX_VECTORS) {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function fnLindep:", "expects a row or column vector of 2 to 11 elements.", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }
  // The entries are validated before LASTx is written, so a vector carrying a NaN or an infinity refuses with the same domain error and the same untouched
  // stack as fnAlgdep refuses a special scalar. maxK is the common denominator the residual is measured over; the lattice column does not use it.
  longInteger_t entryN;
  longIntegerInit(entryN);
  bool_t  finite = true;
  int32_t maxK = 0, maxMag = -0x7FFFFFFF, minMag = 0x7FFFFFFF;
  for(uint16_t i = 0; i < count && finite; i++) {
    real_t  v;
    int32_t k;
    real34ToReal(&matrix.matrixElements[i], &v);
    if(realIsSpecial(&v) || !realToScaledInteger(&v, entryN, &k)) {
      finite = false;
    }
    else {
      if(k > maxK) {
        maxK = k;
      }
      int32_t mag = (int32_t)longIntegerBase10Digits(entryN) - k;   // log10 of the entry, not its count of decimal places: 1 and 1.414... are both magnitude 1
      if(mag > maxMag) {
        maxMag = mag;
      }
      if(mag < minMag) {
        minMag = mag;
      }
    }
  }
  longIntegerFree(entryN);
  if(!finite) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function fnLindep:", "every entry of the vector must be finite.", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }
  if(!saveLastX()) {                                         // the vector in X is consumed, so LASTx carries it, as fnAlgdep's does; a refusal undoes it the same way
    return;
  }
  // Entries spread over more decades than the digit budget cannot share a relation of any usable height, so this refuses the impossible case and bounds the
  // powers of ten the construction would otherwise follow. It is the fnLindep twin of the magnitude test in algdepSearch; placed here it also spares the
  // lattice allocation.
  if(maxMag - minMag > ALGDEP_MAX_CONFIDENCE - ALGDEP_MARGIN) {
    reportNoRelation("In function fnLindep:", "no integer relation stands out among entries this far apart in magnitude,");
    return;
  }
  if((L = latticeAlloc()) == NULL) {
    return;
  }

  longInteger_t scale, den, num, quo, term;
  longIntegerInit(scale); longIntegerInit(den); longIntegerInit(num); longIntegerInit(quo); longIntegerInit(term);
  longIntegerPowerUIntUInt(10, ALGDEP_SCALE, scale);

  L->n = (int16_t)count;
  L->m = (int16_t)(count + 1);
  for(uint16_t i = 0; i < count; i++) {
    real_t  v;
    int32_t k = 0;
    real34ToReal(&matrix.matrixElements[i], &v);
    realToScaledInteger(&v, L->powN[i], &k);                 // cannot fail, the entries were validated above; parsed into powN once, the candidate loop reads it back
    longIntegerPowerUIntUInt(10, (uint32_t)(maxK - k), L->p10[i]);
    for(uint16_t j = 0; j < count; j++) {
      mpz_set_ui(L->b[i][j], (i == j) ? 1 : 0);
    }
    longIntegerPowerUIntUInt(10, (uint32_t)k, den);          // nint(10^S * v_i), exactly as trySingleDegree builds its column: scaling the column any harder
    mpz_mul(num, scale, L->powN[i]);                         // than the identity block stops LLL feeling the cost of a large coefficient, and it then finds
    mpz_mul_2exp(quo, num, 1); mpz_add(quo, quo, den);       // the exact relation that always exists among truncated decimals
    mpz_mul_2exp(term, den, 1);
    mpz_fdiv_q(quo, quo, term);
    mpz_set(L->b[i][count], quo);
  }

  bool_t found = false;
  int32_t margin = 0;
  if(latticeReduce(L)) {
    for(uint16_t cand = 0; cand < count && !found; cand++) {
      if(!scoreCandidate(L, (int32_t)cand, (int32_t)count, &margin)) {
        continue;
      }
      for(uint16_t i = 0; i < count; i++) {
        mpz_set(L->coeff[i], L->b[cand][i]);
      }
      for(uint16_t i = 0; i < count; i++) {                  // first non-zero coefficient positive, so the reported relation is deterministic
        if(mpz_sgn(L->coeff[i]) != 0) {
          if(mpz_sgn(L->coeff[i]) < 0) {
            for(uint16_t j = 0; j < count; j++) {
              mpz_neg(L->coeff[j], L->coeff[j]);
            }
          }
          break;
        }
      }
      found = true;
    }
  }

  longIntegerFree(scale); longIntegerFree(den); longIntegerFree(num); longIntegerFree(quo); longIntegerFree(term);

  if(!found) {
    latticeFree(L);
    reportNoRelation("In function fnLindep:", "no integer relation stands out among the vector entries,");
    return;
  }

  real34Matrix_t result;
  liftStack();
  if(initMatrixRegister(REGISTER_X, 1, count, false)) {
    linkToRealMatrixRegister(REGISTER_X, &result);
    for(uint16_t i = 0; i < count; i++) {
      convertLongIntegerToReal34(L->coeff[i], &result.matrixElements[i]);
    }
  }
  reallocateRegister(REGISTER_Y, dtReal34, 0, amNone);
  int32ToReal34(margin, REGISTER_REAL34_DATA(REGISTER_Y));
  latticeFree(L);
}

#endif // OPTION_ALGDEP
