// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file cos.h
 ***********************************************/
#if !defined(COS_H)
  #define COS_H

  void fnCos   (uint16_t unusedButMandatoryParameter);

  void cosComplex(const real_t *real, const real_t *imag, real_t *resReal, real_t *resImag, realContext_t *realContext);
#endif // !COS_H
