// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file floor.c
 ***********************************************/

#include "c47.h"
#include "integerPart.h"

static void floorReal(void) {
  integerPartReal(DEC_ROUND_FLOOR);
}

static void floorCplx(void) {
  integerPartCplx(DEC_ROUND_FLOOR);
}

/********************************************//**
 * \brief regX ==> regL and floor(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnFloor(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexMonadicFunction(&floorReal, &floorCplx, &integerPartNoOp, &integerPartNoOp);
}
