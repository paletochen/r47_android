// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file not.c
 ***********************************************/

#include "c47.h"

static void notCplx(void) {
  real_t xr, xc;
  bool_t res;
  uint32_t xtype = getRegisterDataType(REGISTER_X);

  if (!getRegisterAsComplex(REGISTER_X, &xr, &xc))
    return;
  res = realIsZero(&xr) && realIsZero(&xc) ? true : false;
  logicalOpResult(res, xtype, xtype);
}

static void notShoI(void) {
  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = ~(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_X))) & shortIntegerMask;
}

/********************************************//**
 * \brief regX ==> regL and not(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnLogicalNot(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexMonadicFunction(NULL, &notCplx, &notShoI, NULL);
}
