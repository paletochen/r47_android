// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file round.h
 ***********************************************/
#if !defined(ROUND_H)
  #define ROUND_H

  void fnRound   (uint16_t unusedButMandatoryParameter);

  #if (EXTRA_INFO_ON_CALC_ERROR == 1)
    void roundError(void);
  #else // (EXTRA_INFO_ON_CALC_ERROR != 1)
    #define roundError typeError
  #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)

  void roundTime (void);
  void roundDate (void);
  void roundRema (void);
  void roundCxma (void);
  void roundReal (void);
  void roundCplx (void);
#endif // !ROUND_H
