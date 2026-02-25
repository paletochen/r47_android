// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file agm.h
 ***********************************************/
#if !defined(AGM_H)
  #define AGM_H

  void   fnAgm         (uint16_t unusedButMandatoryParameter);

  size_t realAgm       (const real_t *a, const real_t *b, real_t *res, realContext_t *realContext);
  size_t complexAgm    (const real_t *ar, const real_t *ai, const real_t *br, const real_t *bi, real_t *resr, real_t *resi, realContext_t *realContext);
  size_t realAgmForE   (const real_t *a, const real_t *b, real_t *c, real_t *res, realContext_t *realContext);
  size_t complexAgmForE(const real_t *ar, const real_t *ai, const real_t *br, const real_t *bi, real_t *cr, real_t *ci, real_t *resr, real_t *resi, realContext_t *realContext);
  size_t realAgmForF   (const real_t *a, const real_t *b, real_t *c, real_t *res, realContext_t *realContext);
  size_t realAgmStep   (const real_t *a, const real_t *b, real_t *res, real_t *aStep, real_t *bStep, size_t bufSize, realContext_t *realContext);
  size_t complexAgmStep(const real_t *ar, const real_t *ai, const real_t *br, const real_t *bi, real_t *resr, real_t *resi, real_t *aStep, real_t *aiStep, real_t *bStep, real_t *biStep, size_t bufSize, realContext_t *realContext);
#endif // !AGM_H
