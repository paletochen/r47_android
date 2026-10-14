// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file arcsin.h
 ***********************************************/
#if !defined(ARCSIN_H)
  #define ARCSIN_H

  void fnArcsin   (uint16_t unusedButMandatoryParameter);

  uint8_t ArcsinComplex(const real_t *xReal, const real_t *xImag, real_t *rReal, real_t *rImag, realContext_t *realContext);
#endif // !ARCSIN_H
