// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file xor.c
 ***********************************************/

#include "c47.h"
#include "and.h"

static void xorCplx(void) {
  static TO_QSPI const unsigned char xorTable[4] = { 0, 1, 1, 0 };

  dyadicLogicalOp(xorTable);
}

static void xorShoI(void) {
  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) ^ *(REGISTER_SHORT_INTEGER_DATA(REGISTER_Y));
  setRegisterShortIntegerBase(REGISTER_X, getRegisterShortIntegerBase(REGISTER_Y));
}

/********************************************//**
 * \brief regX ==> regL XOR regY ÷ regX ==> regX
 * Drops Y, enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter
 * \return void
 ***********************************************/
void fnLogicalXor(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexDyadicFunction(NULL, &xorCplx, &xorShoI, NULL);
}
