// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file toRect.h
 ***********************************************/
#if !defined(TORECT_H)
  #define TORECT_H

  void fnToRect2               (uint16_t unusedButMandatoryParameter);
  void fnToRect_HP             (uint16_t unusedButMandatoryParameter);
  void fnToRect_CX             (uint16_t unusedButMandatoryParameter);
  void realPolarToRectangular  (const real_t *magnitude, const real_t *theta, real_t *real, real_t *imag, realContext_t *realContext);
  //void real34PolarToRectangular(const real34_t *magnitude34, const real34_t *theta34, real34_t *real34, real34_t *imag34); never used
#endif // !TORECT_H
