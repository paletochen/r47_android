// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file integerPartLonginteger.c
 ***********************************************/

#include "c47.h"


/********************************************//**
 * \brief regX ==> regL and LINT(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnLint(uint16_t unusedButMandatoryParameter) {
  longInteger_t val;
  uint32_t type = getRegisterDataType(REGISTER_X);

  if(!saveLastX()) {
    return;
  }

  if(getRegisterAsLongInt(REGISTER_X, val, NULL)) {
    convertLongIntegerToLongIntegerRegister(val, REGISTER_X);
    if(type == dtShortInteger) {
      setLastintegerBasetoZero();
    }
  }
  longIntegerFree(val);
}

