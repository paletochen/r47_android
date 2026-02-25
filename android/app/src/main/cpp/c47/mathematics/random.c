// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file random.c
 ***********************************************/

#include "c47.h"

//////////////////////////////////////////////////////////
// Implementation an optimal random integer in a range function.
//
// Essentially it boils down to incrementally generating a fixed point
// number on the interval [0, 1) and multiplying this number by the upper
// range limit.  Once it is certain what the fractional part contributes to
// the integral part of the product, the algorithm has produced a definitive
// result.
//
// Refer: https://github.com/apple/swift/pull/39143 for a fuller description
// of the algorithm.
static uint32_t boundedRand(uint32_t s) { // random integer in [0 , s)
    uint32_t i, f;      // integer and fractional parts
    uint32_t f2, rand;  // extra fractional part and random material
    uint64_t prod;      // temporary holding double width product

    rand = pcg32_random_r(&pcg32_global);

    // We are generating a fixed point number on the interval [0, 1).
    // Multiplying this by the range gives us a number on [0, upper).
    // The high word of the multiplication result represents the integral
    // part we want.  The lower word is the fractional part.  We can early exit if
    // if the fractional part is small enough that no carry from the next lower
    // word can cause an overflow and carry into the integer part.  This
    // happens when the fractional part is bounded by 2^32 - upper which
    // can be simplified to just -upper (as an unsigned integer).
    prod = (uint64_t)s * rand;
    i = prod >> 32;
    f = prod & 0xffffffff;
    if (f <= 1 + ~s)    // 1+~s == -s but compilers whine
        return i;

    // We're in the position where the carry from the next word *might* cause
    // a carry to the integral part.  The process here is to generate the next
    // word, multiply it by the range and add that to the current word.  If
    // it overflows, the carry propagates to the integer part (return i+1).
    // If it can no longer overflow regardless of further lower order bits,
    // we are done (return i).  If there is still a chance of overflow, we
    // repeat the process with the next lower word.
    //
    // Each *bit* of randomness has a probability of one half of terminating
    // this process, so each each word beyond the first has a probability
    // of 2^-32 of not terminating the process.  That is, we're extremely
    // likely to stop very rapidly.
    for (int j = 0; j < 10; j++) {
        rand = pcg32_random_r(&pcg32_global);
        prod = (uint64_t)s * rand;
        f2 = prod >> 32;
        f += f2;
        // On overflow, add the carry to our result
        if (f < f2)
            return i + 1;
        // For not all 1 bits, there is no carry so return the result
        if (f != 0xffffffff)
            return i;
        // setup for the next word of randomness
        f = prod & 0xffffffff;
    }
    // If we get here, we've consumed 32 * 10 + 32 bits
    // with no firm decision, this gives a bias with probability < 2^-(320),
    // which is likely acceptable.
    return i;
}


void realRandomU01(real_t *res) {
  real_t t;

  uInt32ToReal(boundedRand(100000000),  res);
  uInt32ToReal(boundedRand(100000000),  &t);
  realFMA(const_1e8, res, &t, res, &ctxtReal39);
  uInt32ToReal(boundedRand(1000000000), &t);
  realFMA(const_1e9, res, &t, res, &ctxtReal39);
  uInt32ToReal(boundedRand(1000000000), &t);
  realFMA(const_1e9, res, &t, res, &ctxtReal39);
  res->exponent -= 34;
}


static void doIntRandomI(void) {
  longInteger_t regX, regY, mini, maxi;
  uint32_t maxRand;
  int32_t cmp;
  bool_t frac, init_minmax = false;

  saveForUndo();
  thereIsSomethingToUndo = true;

  if(!getRegisterAsLongInt(REGISTER_X, regX, &frac) || frac) {
    goto err1;
  }
  if(!getRegisterAsLongInt(REGISTER_Y, regY, &frac) || frac) {
    goto err2;
  }

  cmp = longIntegerCompare(regX, regY);
  if(cmp == 0) {
    goto end;
  }

  init_minmax = true;
  longIntegerInit(mini);
  longIntegerInit(maxi);
  if(cmp < 0) {
    longIntegerCopy(regX, mini);
    longIntegerCopy(regY, maxi);
  }
  else {
    longIntegerCopy(regX, maxi);
    longIntegerCopy(regY, mini);
  }

  longIntegerSubtract(maxi, mini, regX);
  if(longIntegerCompareUInt(regX, 0xFFFFFFFE) >= 0) { // 2^32 - 2
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function doIntRandomI:", "cannot RANI# with |X - Y| >= 2^32", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    fnUndo(0);
    goto err3;
  }

  longIntegerToUInt32(regX, maxRand);
  maxRand = boundedRand(maxRand + 1);
  longIntegerAddUInt(mini, maxRand, regX);

end:
  convertLongIntegerToLongIntegerRegister(regX, REGISTER_X);
  adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);

err3:
  if(init_minmax) {
    longIntegerFree(maxi);
    longIntegerFree(mini);
  }
err2:
  longIntegerFree(regY);
err1:
  longIntegerFree(regX);
}

static void doRealRandomI(void) {
  real_t regX, regY, t, u, *lower;

  if (!getRegisterAsReal(REGISTER_X, &regX) || !getRegisterAsReal(REGISTER_Y, &regY))
    return;

  realSubtract(&regX, &regY, &t, &ctxtReal39);
  if(realIsZero(&t))
    goto end;

  if(realIsNegative(&t)) {
    realChangeSign(&t);
    lower = &regX;
  }
  else {
    lower = &regY;
  }

  realRandomU01(&u);
  realFMA(&u, &t, lower, &regX, &ctxtReal39);
end:
  convertRealToResultRegister(&regX, REGISTER_X, amNone);
}

void fnRandomI(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexDyadicFunction(&doRealRandomI, NULL, NULL, &doIntRandomI);
}


/////////////////////////////////////////////////////////////////////////////
// Method for pseudo random number generation: http://www.pcg-random.org/

void fnRandom(uint16_t unusedButMandatoryParameter) {
  real_t r;

  realRandomU01(&r);
  liftStack();
  reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
  convertRealToReal34ResultRegister(&r, REGISTER_X);
  adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
}

void fnSeed(uint16_t unusedButMandatoryParameter) {
  uint64_t seed=0, sequ=0;
  real_t regX;
  unsigned char *p = (unsigned char *)regX.lsu;

  if (!saveLastX())
    return;

  memset(&regX, 0, sizeof(regX));
  if (!getRegisterAsReal(REGISTER_X, &regX))
    return;
  fnDrop(NOPARAM);

  memcpy(&seed, p, sizeof(seed));
  memcpy(&sequ, p + sizeof(seed), sizeof(sequ));

  if(seed == 0 && sequ == 0) {
    #if defined(TESTSUITE_BUILD)
      seed = 0xDeadBeef;
      sequ = 0xBadCafeFace;
    #else // !TESTSUITE_BUILD
      seed = (((uint64_t)getUptimeMs()) << 32) + (uint64_t)getFreeRamMemory();
      sequ = (((uint64_t)getUptimeMs()) << 32) + (uint64_t)getFreeFlash();
    #endif // TESTSUITE_BUILD
  }

  pcg32_srandom(seed, sequ);
}
