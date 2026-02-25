// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file erf.c
 ***********************************************/

#include "c47.h"

static void erfReal(void) {
  real_t x;

  if(!getRegisterAsReal(REGISTER_X, &x))
    return;
  WP34S_Erf(&x, &x, &ctxtReal39);
  convertRealToResultRegister(&x, REGISTER_X, amNone);
}


/********************************************//**
 * \brief regX ==> regL and erf(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnErf(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&erfReal, NULL);
}
