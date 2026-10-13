// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file ln.h
 ***********************************************/
#if !defined(LN_H)
  #define LN_H

  void fnLn   (uint16_t unusedButMandatoryParameter);
  void lnComplex(const real_t *real, const real_t *imag, real_t *lnReal, real_t *lnImag, realContext_t *realContext);
#endif // !LN_H
