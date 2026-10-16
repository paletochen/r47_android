// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file erfc.c
 ***********************************************/

#include "c47.h"

// erfc remains grouped with OPTION_DIST_NORML in the build configuration.
#if defined(OPTION_DIST_NORML)
static void erfcReal(void) {
  real_t x;

  if(!getRegisterAsReal(REGISTER_X, &x)) {
    return;
  }

  WP34S_Erfc(&x, &x, &ctxtReal51);
  convertRealToResultRegister(&x, REGISTER_X, amNone);
}
#endif // OPTION_DIST_NORML

/********************************************//**
 * \brief regX ==> regL and erfc(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnErfc(uint16_t unusedButMandatoryParameter) {
#if defined(OPTION_DIST_NORML)
  processRealComplexMonadicFunction(&erfcReal, NULL);
#endif // OPTION_DIST_NORML
}
