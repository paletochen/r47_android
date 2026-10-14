// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file ceil.c
 ***********************************************/

#include "c47.h"
#include "integerPart.h"

static void ceilReal(void) {
  integerPartReal(DEC_ROUND_CEILING);
}

static void ceilCplx(void) {
  integerPartCplx(DEC_ROUND_CEILING);
}

/********************************************//**
 * \brief regX ==> regL and ceil(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnCeil(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexMonadicFunction(&ceilReal, &ceilCplx, &integerPartNoOp, &integerPartNoOp);
}
