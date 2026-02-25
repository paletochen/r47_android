// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file tvm.h
 ***********************************************/
#if !defined(TVM_H)
  #define TVM_H

  void fnTvmVar      (uint16_t variable);
  void fnTvmBeginMode(uint16_t unusedButMandatoryParameter);
  void fnTvmEndMode  (uint16_t unusedButMandatoryParameter);
  void fnEff         (uint16_t unusedButMandatoryParameter);
  void fnEffToI      (uint16_t unusedButMandatoryParameter);

  void tvmEquation   (calcRegister_t variable, real_t *ioVal, real_t *derivative);
#endif // !TVM_H
