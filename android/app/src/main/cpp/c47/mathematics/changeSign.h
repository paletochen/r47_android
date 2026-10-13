// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file changeSign.h
 ***********************************************/
#if !defined(CHANGESIGN_H)
  #define CHANGESIGN_H

  void fnChangeSign(uint16_t unusedButMandatoryParameter);

  static inline void chsComplex(real_t *aReal, real_t *aImag) {
    realChangeSign(aReal);
    realChangeSign(aImag);
  }

  //      RegX
  void chsReal     (void);
  void chsCplx     (void);
  void chsShoI     (void);
#endif // !CHANGESIGN_H
