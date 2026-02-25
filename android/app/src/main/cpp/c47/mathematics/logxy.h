// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file logxy.h
 ***********************************************/
#if !defined(LOGXY_H)
  #define LOGXY_H

  void fnLogXY   (uint16_t unusedButMandatoryParameter);

  #if (EXTRA_INFO_ON_CALC_ERROR == 1)
    void logxyError(void);
  #else // (EXTRA_INFO_ON_CALC_ERROR != 1)
    #define logxyError typeError
  #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)

  void logxyLonILonI(void);
  void logxyRealLonI(void);
  void logxyCplxLonI(void);
  void logxyRemaLonI(void);
  void logxyCxmaLonI(void);
  void logxyShoILonI(void);

  void logxyLonIReal(void);
  void logxyRealReal(void);
  void logxyCplxReal(void);
  void logxyRemaReal(void);
  void logxyCxmaReal(void);
  void logxyShoIReal(void);

  void logxyLonICplx(void);
  void logxyRealCplx(void);
  void logxyCplxCplx(void);
  void logxyRemaCplx(void);
  void logxyCxmaCplx(void);
  void logxyShoICplx(void);

  void logxyLonIShoI(void);
  void logxyRealShoI(void);
  void logxyCplxShoI(void);
  void logxyRemaShoI(void);
  void logxyCxmaShoI(void);
  void logxyShoIShoI(void);
#endif // !LOGXY_H
