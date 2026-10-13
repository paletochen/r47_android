// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file arctan.h
 ***********************************************/
#if !defined(ARCTAN_H)
  #define ARCTAN_H

  void fnArctan   (uint16_t unusedButMandatoryParameter);

  uint8_t ArctanComplex(real_t *xReal, real_t *xImag, real_t *rReal, real_t *rImag, realContext_t *realContext);
#endif // !ARCTAN_H
