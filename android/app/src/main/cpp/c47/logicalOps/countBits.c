// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file countBits.c
 ***********************************************/

#include "c47.h"

/********************************************//**
 * \brief regX ==> regL and countBits(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnCountBits(uint16_t unusedButMandatoryParameter) {
  uint64_t w;
  uint32_t base;

  if (!getRegisterAsRawShortInt(REGISTER_X, &w, &base) || !saveLastX())
    return;

#if 0
  // https://en.wikipedia.org/wiki/Hamming_weight
  w -= (w >> 1) & 0x5555555555555555;
  w = (w & 0x3333333333333333) + ((w >> 2) & 0x3333333333333333);
  w = (w + (w >> 4)) & 0x0f0f0f0f0f0f0f0f;
  w = (w * 0x0101010101010101) >> 56;
#else
  w = __builtin_popcountll(w);
#endif
  convertUInt64ToShortIntegerRegister(0, w, base, REGISTER_X);
}
