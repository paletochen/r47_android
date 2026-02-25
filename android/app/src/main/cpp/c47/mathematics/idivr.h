// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file idivr.h
 ***********************************************/
#if !defined(IDIVR_H)
  #define IDIVR_H

  void fnIDivR(uint16_t unusedButMandatoryParameter);

  #if (EXTRA_INFO_ON_CALC_ERROR == 1)
    void idivrError(void);
  #else // (EXTRA_INFO_ON_CALC_ERROR != 1)
    #define idivrError typeError
  #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)

  //      RegYRegX
  void idivrLonILonI(void);
  void idivrLonIShoI(void);
  void idivrLonIReal(void);

  //      RegYRegX
  void idivrShoILonI(void);
  void idivrShoIShoI(void);
  void idivrShoIReal(void);

  //      RegYRegX
  void idivrRealLonI(void);
  void idivrRealShoI(void);
  void idivrRealReal(void);
#endif // !IDIVR_H
