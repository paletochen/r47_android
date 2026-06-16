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

static uint64_t realToInt(const real_t *r, uint64_t magnitudeLimit, enum rounding round, bool_t *error) {
  real_t integer;
  int32_t i;
  uint64_t value=0;

  if(realIsSpecial(r)) {
    return 0;
  }

  realToIntegralValue(r, &integer, round, &ctxtReal39);

  for(i=(integer.digits-1)/DECDPUN; i>=0; i--) {
    value = value*1000 + integer.lsu[i]; // 1000 = 10^DECDPUN
    if(value > magnitudeLimit) {
      return 0;
    }
  }

  for(i=integer.exponent; i>0; i--) {
    value *= 10;
    if(value > magnitudeLimit) {
      return 0;
    }
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
  value = (int64_t)realToInt(r, magnitudeLimit, DEC_ROUND_DOWN, error);
  return sign ? (int32_t)(-value) : (int32_t)value;
}

uint32_t realToUint32C47(const real_t *r, bool_t *error) {
  uint64_t magnitudeLimit;

  if(error != NULL) {
    *error = true;
  }

  if(realIsNegative(r)) {
    return 0;
  }

  magnitudeLimit = (uint64_t)UINT32_MAX; // 4294967295

  return (uint32_t)realToInt(r, magnitudeLimit, DEC_ROUND_DOWN, error);
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