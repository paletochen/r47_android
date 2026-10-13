// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file constants.c List of constant description
 * see: https://physics.nist.gov/cuu/index.html
 ***********************************************/

#include "c47.h"

extern const real_t *realtConstants[NOUC];

/********************************************//**
 * \brief Replaces the X content with the selected
 * constant. Enables \b stack \b lift and refreshes the stack
 *
 * \param[in] constant uint16_t Index of the constant to store in X
 * \return void
 ***********************************************/
void fnConstant(const uint16_t constant) {
  #if (EXTRA_INFO_ON_CALC_ERROR == 1)
    if(constant >= NOUC) {
      sprintf(errorMessage, "parameter constant (%" PRIu16 ") is out of bounds, constant must be less or equal to %" PRId32, constant, NOUC - 1);
      moreInfoOnError("In function fnConstant:", errorMessage, NULL, NULL);
      return;
    }
  #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)

  liftStack();
  currentSolverStatus &= ~SOLVER_STATUS_READY_TO_EXECUTE;

  reallocateRegister(REGISTER_X, dtReal34, 0, amNone);

  realToReal34(realtConstants[constant], REGISTER_REAL34_DATA(REGISTER_X));

  adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);

  setLastintegerBasetoZero();
}



/********************************************//**
 * \brief Replaces the X content with the circumference
 * to diameter ratio pi: 3,1415926. Enables \b stack
 * \b lift and refreshes the stack.
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnPi(uint16_t unusedButMandatoryParameter) {
  liftStack();
  currentSolverStatus &= ~SOLVER_STATUS_READY_TO_EXECUTE;

  convertRealToResultRegister(const39_pi, REGISTER_X, amNone);
  adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);

  setLastintegerBasetoZero();
}
