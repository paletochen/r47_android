// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file lnPOne.h
 ***********************************************/
#if !defined(LNPONE_H)
  #define LNPONE_H
  // Coded by JM, based on ln.h

  void fnLnP1   (uint16_t unusedButMandatoryParameter);
  void lnP1Complex(const cplx_t *z, cplx_t *ln, realContext_t *realContext);
#endif // !LNPONE_H
