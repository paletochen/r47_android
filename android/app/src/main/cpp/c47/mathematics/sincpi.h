// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file sinc.h
 ***********************************************/
#if !defined(SINCPI_H)
  #define SINCPI_H
  // Coded by JM, based on sinc.h

  void fnSincpi(uint16_t unusedButMandatoryParameter);

  void sincpiComplex(const real_t *real, const real_t *imag, real_t *resReal, real_t *resImag, realContext_t *realContext);
#endif // !SINCPI_H
