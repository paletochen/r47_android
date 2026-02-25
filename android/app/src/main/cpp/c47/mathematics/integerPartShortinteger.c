// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file integerPartShortinteger.c
 ***********************************************/

#include "c47.h"

/********************************************//**
 * \brief regX ==> regL and SINT(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnSint(uint16_t unusedButMandatoryParameter) {
  bool_t sign, overflow, frac;
  uint64_t val;

  if(!saveLastX() || !getRegisterAsShortInt(REGISTER_X, &sign, &val, &overflow, &frac))
    return;
  if (getRegisterDataType(REGISTER_X) != dtShortInteger)
    convertUInt64ToShortIntegerRegister(sign, val, 10, REGISTER_X);
  forceSystemFlag(FLAG_CARRY, frac);
  forceSystemFlag(FLAG_OVERFLOW, overflow);
}

