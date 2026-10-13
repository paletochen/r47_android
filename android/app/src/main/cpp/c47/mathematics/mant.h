// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file mant.h
 ***********************************************/
#if !defined(MANT_H)
  #define MANT_H

  void fnMant   (uint16_t unusedButMandatoryParameter);

  #if (EXTRA_INFO_ON_CALC_ERROR == 1)
    void mantError(void);
  #else // (EXTRA_INFO_ON_CALC_ERROR != 1)
    #define mantError typeError
  #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)

  void mantLonI (void);
  void mantReal (void);
#endif // !MANT_H
