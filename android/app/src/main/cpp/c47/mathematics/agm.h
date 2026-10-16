// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file agm.h
 ***********************************************/
#if !defined(AGM_H)
  #define AGM_H

  void   fnAgm         (uint16_t unusedButMandatoryParameter);

  size_t realAgm       (const real_t *a, const real_t *b, real_t *res, realContext_t *realContext);
  size_t complexAgm    (const cplx_t *a, const cplx_t *b, cplx_t *res, realContext_t *realContext);
  size_t realAgmForE   (const real_t *a, const real_t *b, real_t *c, real_t *res, realContext_t *realContext);
  size_t complexAgmForE(const cplx_t *a, const cplx_t *b, cplx_t *c, cplx_t *res, realContext_t *realContext);
  size_t realAgmStep   (const real_t *a, const real_t *b, real_t *res, real_t *aStep, real_t *bStep, size_t bufSize, realContext_t *realContext);
  size_t complexAgmStep(const cplx_t *a, const cplx_t *b, cplx_t *res, cplx_t *aStep, cplx_t *bStep, size_t bufSize, realContext_t *realContext);
#endif // !AGM_H
