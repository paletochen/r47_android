// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file roundi.h
 ***********************************************/
#if !defined(ROUNDI_H)
  #define ROUNDI_H

  void fnRoundi   (uint16_t unusedButMandatoryParameter);

  #if (EXTRA_INFO_ON_CALC_ERROR == 1)
    void roundiError(void);
  #else // (EXTRA_INFO_ON_CALC_ERROR != 1)
    #define roundiError typeError
  #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)

  void roundiRema (void);
  void roundiReal (void);

#endif // !ROUNDI_H
