// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file fractionalPart.c
 ***********************************************/

#include "c47.h"

static void fpLonI(void) {
  longInteger_t x;

  longIntegerInit(x); // Set to 0
  convertLongIntegerToLongIntegerRegister(x, REGISTER_X);
  longIntegerFree(x);
}

static void fpShoI(void) {
  uint64_t x, y = 0;

  if(shortIntegerMode == SIM_1COMPL || shortIntegerMode == SIM_SIGNMT) {
    /* These two modes support negative 0, so the fractional part of a negative
     * number becomes -0 rather than +0.
     */
    x = *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X));
    if((x & shortIntegerSignBit) != 0)
      y = shortIntegerMode == SIM_1COMPL ? shortIntegerMask : shortIntegerSignBit;
  }
  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = y;
}

static void fpReal(void) {
  real34_t x;

  real34ToIntegralValue(REGISTER_REAL34_DATA(REGISTER_X), &x, DEC_ROUND_DOWN);
  real34Subtract(REGISTER_REAL34_DATA(REGISTER_X), &x, REGISTER_REAL34_DATA(REGISTER_X));
}

void fnFp(uint16_t unusedButMandatoryParameter) {
    processIntRealComplexMonadicFunction(&fpReal, NULL, &fpShoI, &fpLonI);
}
