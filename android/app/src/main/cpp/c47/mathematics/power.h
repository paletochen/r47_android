// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file power.h
 ***********************************************/
#if !defined(POWER_H)
  #define POWER_H

  void fnPower    (uint16_t unusedButMandatoryParameter);

  void PowerReal(const real_t *y, const real_t *x, real_t *res, realContext_t *realContext);
  bool_t realIntegerPowerExact(const real_t *x, const real_t *n, real_t *res);
  bool_t realExactRoot(const real_t *x, const real_t *n, real_t *root);
  uint8_t PowerComplex(const real_t *yReal, const real_t *yImag, const real_t *xReal, const real_t *xImag, real_t *rReal, real_t *rImag, realContext_t *realContext);
  void longIntegerPower(longInteger_t base, longInteger_t exponent, longInteger_t result);
#endif // !POWER_H
