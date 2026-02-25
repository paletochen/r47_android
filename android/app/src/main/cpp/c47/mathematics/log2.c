// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"
#include "log10.h"

static void log2LonI(void) {
  logxyLonI(const_ln2);
}

static void log2ShoI(void) {
  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = WP34S_intLog2(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)));
}

static void log2Real(void) {
  logxyReal(const_ln2);
}

static void log2Cplx(void) {
  logxyCplx(const_ln2);
}

/********************************************//**
 * \brief regX ==> regL and log2(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnLog2(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexMonadicFunction(&log2Real, &log2Cplx, &log2ShoI, &log2LonI);
}
