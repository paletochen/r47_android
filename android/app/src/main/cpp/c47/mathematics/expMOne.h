// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file expMOne.h
 ***********************************************/
#if !defined(EXPMONE_H)
  #define EXPMONE_H
  // Coded by JM, based on exp.h

  void fnExpM1   (uint16_t unusedButMandatoryParameter);

  void expM1Complex(const real_t *real, const real_t *imag, real_t *resReal, real_t *resImag, realContext_t *realContext);
  void realExpM1(const real_t *x, real_t *res, realContext_t *realContext);

#endif // !EXPMONE_H
