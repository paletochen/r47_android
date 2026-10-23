// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file realType.c
 ***********************************************/

#include "c47.h"

//int64_t realToInt64C47(const real_t *r) {
//  real_t i;
//
//  realToIntegralValue(r, i, DEC_ROUND_DOWN, &ctxtReal39); // After this call, it's guaranteed that i is an integer and i->exponent >= 0
//  decNumberQuantize(i, i, const_0, &ctxtReal39); // After this call, it's guaranteed that the value of i is not changed and i->exponent == 0 (const_0 is used only because it's exponent is 0)
//  return decNumberToInt64(&i, &ctxtReal39);
//}

//uint64_t realToUint64C47(const real_t *r) { // This function is only used once
//  real_t i;
//
//  realToIntegralValue(r, &i, DEC_ROUND_DOWN, &ctxtReal39); // After this call, it's guaranteed that i is an integer and i->exponent >= 0
//  decNumberQuantize(&i, &i, const_0, &ctxtReal39); // After this call, it's guaranteed that the value of i is not changed and i->exponent == 0 (const_0 is used only because it's exponent is 0)
//  return decNumberToUInt64(&i, &ctxtReal39);
//}

#if DECDPUN != 3
  #error DECDPUN must be 3
#endif

#if (defined __GNUC__ && __GNUC__ + (__GNUC_MINOR__ >= 3) > 4) || (defined __clang__ && __clang_major__ >= 3)
  // Self freeing
void auto_free(void *p) {
  void *q = *(void**)p;
  if (q != NULL)
    free(q);
}
#endif  // (defined __GNUC__ && __GNUC__ + (__GNUC_MINOR__ >= 3) > 4) || (defined __clang__ && __clang_major__ >= 3)


static uint64_t realToInt(const real_t *r, uint64_t magnitudeLimit, bool_t *error) {
  TO_QSPI static const uint16_t powersOfTen[DECDPUN] = {1, 10, 100};
  int32_t magnitudeDigits=1;
  int64_t integerDigits;
  uint64_t value=0;

  if(realIsSpecial(r)) {
    return 0;
  }

  for(uint64_t limit=magnitudeLimit; limit>=10; limit/=10) {
    magnitudeDigits++;
  }
  integerDigits = (int64_t)r->digits + r->exponent;
  if(!realIsZero(r) && integerDigits > magnitudeDigits) {
    return 0;
  }
  if(integerDigits <= 0) {
    if(error != NULL) {
      *error = false;
    }
    return 0;
  }

  for(int64_t position=integerDigits-1; position>=0; position--) {
    const int64_t coefficientPosition = position - r->exponent;
    const uint16_t digit = coefficientPosition < 0 ? 0
                                                   : r->lsu[coefficientPosition/DECDPUN] / powersOfTen[coefficientPosition%DECDPUN] % 10;
    if(value > (magnitudeLimit - digit)/10) {
      return 0;
    }
    value = value*10 + digit;
  }

  if(error != NULL) {
    *error = false;
  }
  return value;
}

int32_t realToInt32C47(const real_t *r, bool_t *error) {
  uint64_t magnitudeLimit;
  bool_t sign;
  int64_t value;

  sign = realIsNegative(r);

  magnitudeLimit = (uint64_t)INT32_MAX + (uint64_t)sign; // 2147483647 or 2147483648

  if(error != NULL) {
    *error = true;
  }
  value = (int64_t)realToInt(r, magnitudeLimit, error);
  return sign ? (int32_t)(-value) : (int32_t)value;
}

uint32_t realToUint32C47(const real_t *r, bool_t *error) {
  uint64_t magnitudeLimit;

  if(error != NULL) {
    *error = true;
  }

  if(realIsNegative(r) && !realIsZero(r)) {
    return 0;
  }

  magnitudeLimit = (uint64_t)UINT32_MAX; // 4294967295

  return (uint32_t)realToInt(r, magnitudeLimit, error);
}

bool_t realIsZero(const real_t *r) {
  return r->lsu[0] == 0 && r->digits == 1 && (r->bits & DECSPECIAL) == 0;
}

void realSetZero(real_t *r) {
  r->bits     = 0;
  r->exponent = 0;
  r->digits   = 1;
  r->lsu[0]   = 0;
}

void realSetOne(real_t *r) {
  r->bits     = 0;
  r->exponent = 0;
  r->digits   = 1;
  r->lsu[0]   = 1;
}

void realSetNegativeOne(real_t *r) {
  r->bits     = DECNEG;
  r->exponent = 0;
  r->digits   = 1;
  r->lsu[0]   = 1;
}

void realSetNaN(real_t *r) {
  r->bits     = DECNAN;
  r->exponent = 0;
  r->digits   = 1;
  r->lsu[0]   = 0;
}

void realSetPlusInfinity(real_t *r) {
  r->bits     = DECINF;
  r->exponent = 0;
  r->digits   = 1;
  r->lsu[0]   = 0;
}

void realSetMinusInfinity(real_t *r) {
  r->bits     = DECINF | DECNEG;
  r->exponent = 0;
  r->digits   = 1;
  r->lsu[0]   = 0;
}

void copyComplex34(const real34_t *source, real34_t *destination) {
  real34Copy(source,     destination);
  real34Copy(source + 1, destination + 1);
}

void setReal34NegativeSign(real34_t *r) {
  decQuadCopyNegate(r, decQuadCopyAbs(r, r));
}


// A routine that is the same under addition and under subtraction takes one of these. real34Add and real34Subtract are macros and cannot be pointed at.
void real34AddOp(const real34_t *y, const real34_t *x, real34_t *res) {
  real34Add(y, x, res);
}

void real34SubtractOp(const real34_t *y, const real34_t *x, real34_t *res) {
  real34Subtract(y, x, res);
}

void realAddOp(const real_t *y, const real_t *x, real_t *res, realContext_t *realContext) {
  realAdd(y, x, res, realContext);
}

void realSubtractOp(const real_t *y, const real_t *x, real_t *res, realContext_t *realContext) {
  realSubtract(y, x, res, realContext);
}

void realMultiplyOp(const real_t *y, const real_t *x, real_t *res, realContext_t *realContext) {
  realMultiply(y, x, res, realContext);
}
