// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file and.c
 ***********************************************/

#include "c47.h"
#include "and.h"

void logicalOpResult(bool_t res, uint32_t xtype, uint32_t ytype) {
  if (xtype == dtLongInteger && ytype == dtLongInteger) {
    longInteger_t ires;

    longIntegerInit(ires);
    uInt32ToLongInteger(res ? 1u : 0u, ires);
    convertLongIntegerToLongIntegerRegister(ires, REGISTER_X);
    longIntegerFree(ires);
  } else {
    const real_t *rres = res ? const_1 : const_0;

    if (xtype == dtComplex34 || ytype == dtComplex34)
      convertComplexToResultRegister(rres, const_0, REGISTER_X);
    else
      convertRealToResultRegister(rres, REGISTER_X, amNone);
  }
}

/* table[] indices correspond to:
 *  [0] x=0 y=0
 *  [1] x=1 y=0
 *  [2] x=0 y=1
 *  [3] x=1 y=1
 */
void dyadicLogicalOp(const unsigned char table[4]) {
  real_t xr, xc, yr, yc;
  bool_t x, y, res;

  if (!getRegisterAsComplex(REGISTER_X, &xr, &xc)
      || !getRegisterAsComplex(REGISTER_Y, &yr, &yc))
    return;

  x = !realIsZero(&xr) || !realIsZero(&xc);
  y = !realIsZero(&yr) || !realIsZero(&yc);

  res = table[(x ? 1 : 0) + (y ? 2 : 0)] != 0;
  logicalOpResult(res, getRegisterDataType(REGISTER_X), getRegisterDataType(REGISTER_Y));
}

static void andCplx(void) {
  static const unsigned char andTable[4] = { 0, 0, 0, 1 };

  dyadicLogicalOp(andTable);
}

static void andShoI(void) {
  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) & *(REGISTER_SHORT_INTEGER_DATA(REGISTER_Y));
  setRegisterShortIntegerBase(REGISTER_X, getRegisterShortIntegerBase(REGISTER_Y));
}

/********************************************//**
 * \brief regX ==> regL AND regY ÷ regX ==> regX
 * Drops Y, enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter
 * \return void
 ***********************************************/
void fnLogicalAnd(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexDyadicFunction(NULL, &andCplx, &andShoI, NULL);
}
