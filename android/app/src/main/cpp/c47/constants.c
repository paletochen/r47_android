// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file constants.c List of constant description
 * see: https://physics.nist.gov/cuu/index.html
 ***********************************************/

#include "c47.h"

/********************************************//**
 * \brief Replaces the X content with the selected
 * constant. Enables \b stack \b lift and refreshes the stack
 *
 * \param[in] constant uint16_t Index of the constant to store in X
 * \return void
 ***********************************************/
void fnConstant(const uint16_t constant) {
  liftStack();
  currentSolverStatus &= ~SOLVER_STATUS_READY_TO_EXECUTE;

  reallocateRegister(REGISTER_X, dtReal34, 0, amNone);

  if(constant < NUMBER_OF_CONSTANTS_39) { // 39 digit constants
    realToReal34((real_t *)(constants + constant * REAL39_SIZE_IN_BYTES), REGISTER_REAL34_DATA(REGISTER_X));

  }
  else if(constant < NUMBER_OF_CONSTANTS_39 + NUMBER_OF_CONSTANTS_51) { // 51 digit constants (gamma coefficients)
    realToReal34((real_t *)(constants + NUMBER_OF_CONSTANTS_39 * REAL39_SIZE_IN_BYTES
                                      + (constant - NUMBER_OF_CONSTANTS_39) * REAL51_SIZE_IN_BYTES), REGISTER_REAL34_DATA(REGISTER_X));
  }
  else if(constant < NUMBER_OF_CONSTANTS_39 + NUMBER_OF_CONSTANTS_51 + NUMBER_OF_CONSTANTS_6147) { // 6147 digit constant
    realToReal34((real_t *)(constants + NUMBER_OF_CONSTANTS_39 * REAL39_SIZE_IN_BYTES
                                      + NUMBER_OF_CONSTANTS_51 * REAL51_SIZE_IN_BYTES
                                      + (constant - NUMBER_OF_CONSTANTS_39 - NUMBER_OF_CONSTANTS_51) * REAL6147_SIZE_IN_BYTES), REGISTER_REAL34_DATA(REGISTER_X));
  }
  else if(constant < NUMBER_OF_CONSTANTS_39 + NUMBER_OF_CONSTANTS_51 + NUMBER_OF_CONSTANTS_1071 + NUMBER_OF_CONSTANTS_2139) { // 1071 digit constant
    realToReal34((real_t *)(constants + NUMBER_OF_CONSTANTS_39 * REAL39_SIZE_IN_BYTES
                                      + NUMBER_OF_CONSTANTS_51 * REAL51_SIZE_IN_BYTES
                                      + NUMBER_OF_CONSTANTS_1071 * REAL1071_SIZE_IN_BYTES
                                      + (constant - NUMBER_OF_CONSTANTS_39 - NUMBER_OF_CONSTANTS_51 - NUMBER_OF_CONSTANTS_1071) * REAL1071_SIZE_IN_BYTES), REGISTER_REAL34_DATA(REGISTER_X));
  }
  else { // 34 digit constants
    real34Copy((real34_t *)(constants + NUMBER_OF_CONSTANTS_39 * REAL39_SIZE_IN_BYTES
                                      + NUMBER_OF_CONSTANTS_51 * REAL51_SIZE_IN_BYTES
                                      + NUMBER_OF_CONSTANTS_1071 * REAL1071_SIZE_IN_BYTES
                                      + NUMBER_OF_CONSTANTS_2139 * REAL2139_SIZE_IN_BYTES
                                      + NUMBER_OF_CONSTANTS_6147 * REAL6147_SIZE_IN_BYTES
                                      + (constant - NUMBER_OF_CONSTANTS_39 - NUMBER_OF_CONSTANTS_51 - NUMBER_OF_CONSTANTS_1071 - NUMBER_OF_CONSTANTS_2139 - NUMBER_OF_CONSTANTS_6147) * REAL34_SIZE_IN_BYTES), REGISTER_REAL34_DATA(REGISTER_X));
  }

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

  convertRealToResultRegister(const_pi, REGISTER_X, amNone);
  adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);

  setLastintegerBasetoZero();
}
