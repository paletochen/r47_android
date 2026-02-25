// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file integerPart.c
 ***********************************************/

#include "c47.h"

static void doIP(real_t *x, enum rounding mode) {
  if (realIsSpecial(x)) {
    if (!getSystemFlag(FLAG_SPCRES))
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
  } else
    realToIntegralValue(x, x, mode, &ctxtReal39);
}


void integerPartNoOp(void) {
}

void integerPartReal(enum rounding mode) {
  real_t x;

  if (getRegisterAsReal(REGISTER_X, &x)) {
    doIP(&x, mode);
    convertRealToResultRegister(&x, REGISTER_X, amNone);
  }
}

void integerPartCplx(enum rounding mode) {
  real_t a, b;

  if (getRegisterAsComplex(REGISTER_X, &a, &b)) {
    doIP(&a, mode);
    doIP(&b, mode);
    convertComplexToResultRegister(&a, &b, REGISTER_X);
  }
}

static void ipReal(void) {
  integerPartReal(DEC_ROUND_DOWN);
}

static void ipCplx(void) {
  integerPartCplx(DEC_ROUND_DOWN);
}


/********************************************//**
 * \brief regX ==> regL and IP(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnIp(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexMonadicFunction(&ipReal, &ipCplx, &integerPartNoOp, &integerPartNoOp);
}
