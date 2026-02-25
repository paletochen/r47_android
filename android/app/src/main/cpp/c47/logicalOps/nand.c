// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file nand.c
 ***********************************************/

#include "c47.h"
#include "and.h"

static void nandCplx(void) {
  static TO_QSPI const unsigned char nandTable[4] = { 1, 1, 1, 0 };

  dyadicLogicalOp(nandTable);
}

static void nandShoI(void) {
  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = ~(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) & *(REGISTER_SHORT_INTEGER_DATA(REGISTER_Y))) & shortIntegerMask;
  setRegisterShortIntegerBase(REGISTER_X, getRegisterShortIntegerBase(REGISTER_Y));
}

void fnLogicalNand(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexDyadicFunction(NULL, &nandCplx, &nandShoI, NULL);
}
