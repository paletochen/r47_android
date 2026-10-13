// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file tanh.h
 ***********************************************/
#if !defined(TANH_H)
  #define TANH_H

  void fnTanh   (uint16_t unusedButMandatoryParameter);

  uint8_t TanhComplex(const real_t *xReal, const real_t *xImag, real_t *rReal, real_t *rImag, realContext_t *realContext);
#endif // !TANH_H
