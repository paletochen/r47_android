// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file erfc.c
 ***********************************************/

#include "c47.h"

static void erfcReal(void) {
  real_t x;

  if(!getRegisterAsReal(REGISTER_X, &x))
    return;
  WP34S_Erfc(&x, &x, &ctxtReal39);
  convertRealToResultRegister(&x, REGISTER_X, amNone);
}

/********************************************//**
 * \brief regX ==> regL and erfc(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnErfc(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&erfcReal, NULL);
}
