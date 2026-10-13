// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file cosh.c
 ***********************************************/

#include "c47.h"

static void coshReal(void) {
  sinhCoshReal(trigCos);
}



static void coshCplx(void) {
  sinhCoshCplx(trigCos);
}



/********************************************//**
 * \brief regX ==> regL and cosh(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnCosh(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&coshReal, &coshCplx);
}
