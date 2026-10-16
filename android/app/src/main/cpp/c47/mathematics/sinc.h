// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file sinc.h
 ***********************************************/
#if !defined(SINC_H)
  #define SINC_H
  // Coded by JM, based on sin.h

  void fnSinc  (uint16_t unusedButMandatoryParameter);

  void sincComplex(const cplx_t *z, cplx_t *res, realContext_t *realContext);
#endif // !SINC_H
