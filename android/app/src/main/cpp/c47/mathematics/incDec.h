// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file incDec.h
 ***********************************************/
#if !defined(INCDEC_H)
  #define INCDEC_H

  void incDecError(uint16_t regist, uint8_t flag);
  void fnDec      (uint16_t regist);
  void fnInc      (uint16_t regist);
  void incDecLonI (uint16_t regist, uint8_t flag);
  void incDecReal (uint16_t regist, uint8_t flag);
  void incDecCplx (uint16_t regist, uint8_t flag);
  void incDecShoI (uint16_t regist, uint8_t flag);
#endif // !INCDEC_H
