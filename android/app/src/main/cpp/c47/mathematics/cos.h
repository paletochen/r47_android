// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file cos.h
 ***********************************************/
#if !defined(COS_H)
  #define COS_H

  void fnCos   (uint16_t unusedButMandatoryParameter);

  void cosComplex(const cplx_t *z, cplx_t *res, realContext_t *realContext);
#endif // !COS_H
