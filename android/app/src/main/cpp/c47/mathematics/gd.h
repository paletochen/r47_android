// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file fib.h
 ***********************************************/
#if !defined(GD_H)
  #define GD_H

  void fnGd   (uint16_t unusedButMandatoryParameter);
  void fnInvGd(uint16_t unusedButMandatoryParameter);

  uint8_t GudermannianReal(const real_t *x, real_t *res, realContext_t *realContext);
  uint8_t GudermannianComplex(const real_t *xReal, const real_t *xImag, real_t *resReal, real_t *resImag, realContext_t *realContext);

  uint8_t InverseGudermannianReal(const real_t *x, real_t *res, realContext_t *realContext);
  uint8_t InverseGudermannianComplex(const real_t *xReal, const real_t *xImag, real_t *resReal, real_t *resImag, realContext_t *realContext);
#endif // !GD_H
