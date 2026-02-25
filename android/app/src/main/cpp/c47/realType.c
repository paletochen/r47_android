// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file realType.c
 ***********************************************/

#include "c47.h"

static void roundToIntAndNormalise(const real_t *r, real_t *i) {
  realToIntegralValue(r, i, DEC_ROUND_DOWN, &ctxtReal39);
  /* Adding zero forces the normalisation of the resulting real which is
   * required for the conversions to integers to work properly
   */
  realAdd(const_0, i, i, &ctxtReal39);
}

int32_t realToInt32C47(const real_t *r) {
  real_t i;

  roundToIntAndNormalise(r, &i);
  return decNumberToInt32(&i, &ctxtReal39);
}

uint32_t realToUint32C47(const real_t *r) {
  real_t i;

  roundToIntAndNormalise(r, &i);
  return decNumberToUInt32(&i, &ctxtReal39);
}

//int64_t realToInt64C47(const real_t *r) {
//  real_t i;
//
//  roundToIntAndNormalise(r, &i);
//  return decNumberToInt64(&i, &ctxtReal39);
//}

uint64_t realToUint64C47(const real_t *r) {
  real_t i;

  roundToIntAndNormalise(r, &i);
  return decNumberToUInt64(&i, &ctxtReal39);
}
