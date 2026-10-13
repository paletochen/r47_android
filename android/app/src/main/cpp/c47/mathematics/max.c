// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file max.c
 ***********************************************/

#include "c47.h"

void fnMax(uint16_t unusedButMandatoryParameter) {
  if(!saveLastX()) {
    return;
  }

  registerMax(REGISTER_X, REGISTER_Y, REGISTER_X);

  adjustResult(REGISTER_X, true, false, REGISTER_X, -1, -1);
}

