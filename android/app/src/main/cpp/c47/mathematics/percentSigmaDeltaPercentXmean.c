// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file percentSigmaDeltaPercentXmean.c
 ***********************************************/

#include "c47.h"

//=============================================================================
// Main function
//-----------------------------------------------------------------------------

/********************************************//**
 * \brief regX ==> regL and deltaPercentXmean(regX) ==> regX
 * enables stack lift and refreshes the stack.
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/


void fnPcSigmaDeltaPcXmean(uint16_t unusedButMandatoryParameter) {
  real_t xReal;
  real_t rReal;

  if(!checkMinimumDataPoints(const_1)) {
    displayCalcErrorMessage(ERROR_NO_SUMMATION_DATA, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "There is no statistical data available!");
      moreInfoOnError("In function fnPcSigmaDeltaPcXmean:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  if(!getRegisterAsReal(REGISTER_X, &xReal))
    return;

  if(!saveLastX()) {
    return;
  }
  liftStack();

  if(percentSigma(&xReal, &rReal, &ctxtReal75)) {
    reallocateRegister(REGISTER_Y, dtReal34, 0, amNone);
    convertRealToReal34ResultRegister(&rReal, REGISTER_Y);
  }

  if(deltaPercentXmeanReal(&xReal, &rReal, &ctxtReal75)) {
    reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
    convertRealToReal34ResultRegister(&rReal, REGISTER_X);
  }

  adjustResult(REGISTER_X, false, true, REGISTER_L, -1, -1);
  adjustResult(REGISTER_Y, false, true, REGISTER_L, -1, -1);

  temporaryInformation = TI_PERCD2;
}
