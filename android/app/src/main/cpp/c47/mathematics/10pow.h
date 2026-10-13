// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file 10pow.h
 ***********************************************/
#if !defined(FILE_10POW_H)
  #define FILE_10POW_H

  void fn10Pow    (uint16_t unusedButMandatoryParameter);
  void realPower10(const real_t *x, real_t *res, realContext_t *realContext);
  void intPowReal(void (*powf)(const real_t *x, real_t *res, realContext_t *realContext));
  void intPowCplx(const real_t *lnBase);
#endif // !FILE_10POW_H
