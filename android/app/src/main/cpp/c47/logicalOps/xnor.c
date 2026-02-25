// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file xnor.c
 ***********************************************/

#include "c47.h"
#include "and.h"

static void xnorCplx(void) {
  static TO_QSPI const unsigned char xnorTable[4] = { 1, 0, 0, 1 };

  dyadicLogicalOp(xnorTable);
}

static void xnorShoI(void) {
  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = ~(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) ^ *(REGISTER_SHORT_INTEGER_DATA(REGISTER_Y))) & shortIntegerMask;
  setRegisterShortIntegerBase(REGISTER_X, getRegisterShortIntegerBase(REGISTER_Y));
}

void fnLogicalXnor(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexDyadicFunction(NULL, &xnorCplx, &xnorShoI, NULL);
}
