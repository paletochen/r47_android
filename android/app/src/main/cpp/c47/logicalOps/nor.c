// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file nor.c
 ***********************************************/

#include "c47.h"
#include "and.h"

static void norCplx(void) {
  static TO_QSPI const unsigned char norTable[4] = { 1, 0, 0, 0 };

  dyadicLogicalOp(norTable);
}

static void norShoI(void) {
  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = ~(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) | *(REGISTER_SHORT_INTEGER_DATA(REGISTER_Y))) & shortIntegerMask;
  setRegisterShortIntegerBase(REGISTER_X, getRegisterShortIntegerBase(REGISTER_Y));
}

void fnLogicalNor(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexDyadicFunction(NULL, &norCplx, &norShoI, NULL);
}
