// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file tanh.h
 ***********************************************/
#if !defined(TANH_H)
  #define TANH_H

  void fnTanh   (uint16_t unusedButMandatoryParameter);

  uint8_t TanhComplex(const cplx_t *x, cplx_t *r, realContext_t *realContext);
#endif // !TANH_H
