// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file sin.h
 ***********************************************/
#if !defined(SIN_H)
  #define SIN_H

  void fnSin   (uint16_t unusedButMandatoryParameter);

  void sinComplex(const real_t *real, const real_t *imag, real_t *resReal, real_t *resImag, realContext_t *realContext);
#endif // !SIN_H
