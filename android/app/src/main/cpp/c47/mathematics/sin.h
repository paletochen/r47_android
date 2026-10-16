// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file sin.h
 ***********************************************/
#if !defined(SIN_H)
  #define SIN_H

  void sinCosReal(trigType_t trigType);
  void sinCosCplx(trigType_t trigType);
  void fnSin     (uint16_t unusedButMandatoryParameter);
  void multiplyTrigByHyperbolic(const real_t *trigonometric, const real_t *hyperbolic, real_t *res, realContext_t *realContext);
  void sinComplex(const cplx_t *z, cplx_t *res, realContext_t *realContext);
#endif // !SIN_H
