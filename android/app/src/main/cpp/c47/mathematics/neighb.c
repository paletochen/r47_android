// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file neighb.c
 ***********************************************/

#include "c47.h"

static void neighbShoI(void) {
  bool_t subtract, negx, negy;
  uint64_t x, y;

  if (getRegisterAsShortInt(REGISTER_X, &negx, &x, NULL, NULL) && getRegisterAsShortInt(REGISTER_Y, &negy, &y, NULL, NULL)) {
    if (negx != negy)
      subtract = negy;
    else if (x == y)
      return;
    else if (negx)
      subtract = y > x;
    else
      subtract = y < x;

    if (subtract)
      *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = WP34S_intSubtract(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)), 1);
    else
      *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = WP34S_intAdd(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)), 1);
  }
}

static void neighbLonI(void) {
  longInteger_t x, y;

  if(!getRegisterAsLongInt(REGISTER_X, x, NULL)) {
    goto end1;
  }
  if(!getRegisterAsLongInt(REGISTER_Y, y, NULL)) {
    goto end2;
  }

  const int32_t cmp = longIntegerCompare(y, x);

  if(cmp != 0) {
    int32ToLongInteger(cmp > 0 ? 1 : -1, y);
    longIntegerAdd(x, y, x);
  }
  convertLongIntegerToLongIntegerRegister(x, REGISTER_X);

end2:
  longIntegerFree(y);
end1:
  longIntegerFree(x);
}

static void neighbReal(void) {
  real_t x, y;
  angularMode_t xAngularMode = amNone;

  if (!getRegisterAsReal(REGISTER_X, &x) || !getRegisterAsReal(REGISTER_Y, &y))
    return;
  if (getRegisterDataType(REGISTER_X) == dtReal34)
    xAngularMode = getRegisterAngularMode(REGISTER_X);
  realNextToward(&x, &y, &x, &ctxtReal34);
  convertRealToResultRegister(&x, REGISTER_X, xAngularMode);
}

void fnNeighb(uint16_t unusedButMandatoryParameter) {
    processIntRealComplexDyadicFunction(&neighbReal, NULL, &neighbShoI, &neighbLonI);
}
