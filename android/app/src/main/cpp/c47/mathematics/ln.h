// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file ln.h
 ***********************************************/
#if !defined(LN_H)
  #define LN_H

  void fnLn   (uint16_t unusedButMandatoryParameter);
  void lnComplex(const cplx_t *z, cplx_t *ln, realContext_t *realContext);
  void lnComplexDirect(const cplx_t *z, cplx_t *ln);
#endif // !LN_H
