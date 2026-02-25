// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file w_inverse.c
 ***********************************************/

#include "c47.h"

static void wInvReal(void) {
  real_t x, res;

  if(!getRegisterAsReal(REGISTER_X, &x))
    return;

  WP34S_InverseW(&x, &res, &ctxtReal39);
  convertRealToResultRegister(&res, REGISTER_X, amNone);
}

static void wInvCplx(void) {
  real_t xr, xi, resr, resi;

  if(!getRegisterAsComplex(REGISTER_X, &xr, &xi))
    return;
  WP34S_InverseComplexW(&xr, &xi, &resr, &resi, &ctxtReal39);
  convertComplexToResultRegister(&resr, &resi, REGISTER_X);
}

/********************************************//**
 * \brief regX ==> regL and W^(-1)(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnWinverse(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&wInvReal, &wInvCplx);

}
