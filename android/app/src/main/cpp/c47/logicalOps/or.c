// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file or.c
 ***********************************************/

#include "c47.h"
#include "and.h"

static void orCplx(void) {
  static TO_QSPI const unsigned char orTable[4] = { 0, 1, 1, 1 };

  dyadicLogicalOp(orTable);
}

static void orShoI(void) {
  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) | *(REGISTER_SHORT_INTEGER_DATA(REGISTER_Y));
  setRegisterShortIntegerBase(REGISTER_X, getRegisterShortIntegerBase(REGISTER_Y));
}

void fnLogicalOr(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexDyadicFunction(NULL, &orCplx, &orShoI, NULL);
}
