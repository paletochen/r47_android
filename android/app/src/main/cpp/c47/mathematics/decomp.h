// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file decomp.h
 ***********************************************/
#if !defined(DECOMP_H)
  #define DECOMP_H

  void fnDecomp(uint16_t unusedButMandatoryParameter);

  #if (EXTRA_INFO_ON_CALC_ERROR == 1)
    void decompError(void);
  #else // (EXTRA_INFO_ON_CALC_ERROR == 1)
    #define decompError typeError
  #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)

  void decompLonI(void);
  void decompReal(void);
#endif // !DECOMP_H
