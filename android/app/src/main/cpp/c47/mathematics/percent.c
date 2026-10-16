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
  xReal->exponent -= 2;                               // xReal = xReal / 100.0
  realMultiply(xReal, yReal, rReal, realContext);     // rReal = xReal * yReal / 100.0
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
  real34_t exactResult;
  const uint32_t typeX = getRegisterDataType(REGISTER_X);
  const uint32_t typeY = getRegisterDataType(REGISTER_Y);
  const bool_t exactProduct = (typeX == dtLongInteger || typeY == dtLongInteger)
                              && multiplyRegisterValuesToReal34Scaled(REGISTER_X, REGISTER_Y, -2, &exactResult);

  if(!exactProduct && (!getRegisterAsReal(REGISTER_X, &xReal) || !getRegisterAsReal(REGISTER_Y, &yReal))) {
    return;
  }

  if(!saveLastX()) {
    return;
  }

  reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
  if(exactProduct) {
    real34Copy(&exactResult, REGISTER_REAL34_DATA(REGISTER_X));
  }
  else {
    percentReal(&xReal, &yReal, &rReal, &ctxtReal34);
    convertRealToReal34ResultRegister(&rReal, REGISTER_X);
  }
  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
}
