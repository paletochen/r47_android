// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file exp.h
 ***********************************************/
#if !defined(EXP_H)
  #define EXP_H

  void fnExp   (uint16_t unusedButMandatoryParameter);

  void realExp(const real_t *rhs, real_t *res, realContext_t *set);
  bool_t realExpLimitCheck(const real_t *x, real_t *res, const real_t *zero);
  void expComplex(const real_t *real, const real_t *imag, real_t *resReal, real_t *resImag, realContext_t *realContext);
#endif // !EXP_H
