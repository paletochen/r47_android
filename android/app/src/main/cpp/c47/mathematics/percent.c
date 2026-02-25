// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file percent.c
 ***********************************************/

#include "c47.h"

//=============================================================================
// Percent calculation functions
//-----------------------------------------------------------------------------

static void percentReal(real_t *xReal, real_t *yReal, real_t *rReal, realContext_t *realContext) {
  realMultiply(xReal, yReal, rReal, realContext);     // rReal = xReal * yReal
  rReal->exponent -= 2;                               // rReal = rReal / 100.0
}

//=============================================================================
// Main function
//-----------------------------------------------------------------------------

/********************************************//**
 * \brief regX ==> regL and Percent(regX, RegY) ==> regX
 * enables stack lift and refreshes the stack.
 * Calculate x*y/100
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnPercent(uint16_t unusedButMandatoryParameter) {
  real_t xReal, yReal;
  real_t rReal;

  if(!getRegisterAsReal(REGISTER_X, &xReal) || !getRegisterAsReal(REGISTER_Y, &yReal))
    return;

  if(!saveLastX())
    return;

  percentReal(&xReal, &yReal, &rReal, &ctxtReal34);

  reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
  convertRealToReal34ResultRegister(&rReal, REGISTER_X);
  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
}
