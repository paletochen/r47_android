// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file percentT.c
 ***********************************************/

#include "c47.h"

//=============================================================================
// %+MG calculation functions
//-----------------------------------------------------------------------------

static bool_t percentTReal(real_t *xReal, real_t *yReal, real_t *rReal, realContext_t *realContext) {
  /*
   * Check x and y
   */
  if(realIsZero(xReal) && realIsZero(yReal) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function percentTReal:", "cannot divide x=0 by y=0", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  } else if(realIsZero(yReal) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function percentTReal:", "cannot divide a real by y=0", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  } else {
    xReal->exponent += 2;                         // xReal = xReal * 100.0
    realDivide(xReal, yReal, rReal, realContext); // rReal = xReal * 100.0 / yReal
  }

  return true;
}

//=============================================================================
// Main function
//-----------------------------------------------------------------------------

/********************************************//**
 * \brief regX ==> regL and PercentT(regX, RegY) ==> regX
 * enables stack lift and refreshes the stack.
 * Calculate x*y/100
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnPercentT(uint16_t unusedButMandatoryParameter) {
  real_t xReal, yReal;
  real_t rReal;
  real34_t exactResult;
  const uint32_t typeX = getRegisterDataType(REGISTER_X);
  const uint32_t typeY = getRegisterDataType(REGISTER_Y);
  const bool_t exactQuotient = (typeX == dtLongInteger || typeY == dtLongInteger)
                               && divideRegisterValuesToReal34Scaled(REGISTER_X, REGISTER_Y, 2, &exactResult);

  if(!exactQuotient && (!getRegisterAsReal(REGISTER_X, &xReal) || !getRegisterAsReal(REGISTER_Y, &yReal))) {
    return;
  }

  if(!saveLastX()) {
    return;
  }

  if(exactQuotient) {
    convertReal34ToReal34Register(&exactResult, REGISTER_X);
  }
  else if(percentTReal(&xReal, &yReal, &rReal, &ctxtReal34)) {
    convertRealToResultRegister(&rReal, REGISTER_X, amNone);
  }

  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);

  temporaryInformation = TI_PERC;
}
