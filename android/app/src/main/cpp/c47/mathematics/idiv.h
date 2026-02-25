// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file idiv.h
 ***********************************************/
#if !defined(IDIV_H)
  #define IDIV_H

  void fnIDiv(uint16_t unusedButMandatoryParameter);

  #if (EXTRA_INFO_ON_CALC_ERROR == 1)
    void idivError(void);
  #else // (EXTRA_INFO_ON_CALC_ERROR != 1)
    #define idivError typeError
  #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)

  //      RegYRegX
  void idivLonILonI(void);
  void idivLonIShoI(void);
  void idivLonIReal(void);

  //      RegYRegX
  void idivShoILonI(void);
  void idivShoIShoI(void);
  void idivShoIReal(void);

  //      RegYRegX
  void idivRealLonI(void);
  void idivRealShoI(void);
  void idivRealReal(void);
#endif // !IDIV_H
