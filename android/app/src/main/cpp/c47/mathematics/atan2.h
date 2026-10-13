// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file atan2.h
 ***********************************************/
#if !defined(ATAN2_H)
  #define ATAN2_H

  void fnAtan2      (uint16_t unusedButMandatoryParameter);

  #if (EXTRA_INFO_ON_CALC_ERROR == 1)
    void atan2Error   (void);
  #else // (EXTRA_INFO_ON_CALC_ERROR == 1)
    #define atan2Error typeError
  #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)

  void atan2RealReal(void);
  void atan2RemaReal(void);
  void atan2LonIRema(void);
  void atan2RealRema(void);
  void atan2RemaRema(void);
#endif // !ATAN2_H
