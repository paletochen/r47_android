// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file dot.h
 ***********************************************/
#if !defined(DOT_H)
  #define DOT_H

  void fnDot(uint16_t unusedButMandatoryParameter);
  void dotCplx    (const real_t *xReal, const real_t *xImag, const real_t *yReal, const real_t *yImag, real_t *rReal, realContext_t *realContext);
#endif // !DOT_H
