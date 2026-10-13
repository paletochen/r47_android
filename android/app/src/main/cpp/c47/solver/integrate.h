// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file integrate.h
 ***********************************************/
#if !defined(INTEGRATE_H)
  #define INTEGRATE_H

  void fnPgmInt     (uint16_t label);
  void fnIntegrate  (uint16_t labelOrVariable);
  void fnIntegrateYX(uint16_t labelOrVariable);
  void fnIntVar     (uint16_t unusedButMandatoryParameter);

  void integrate  (calcRegister_t regist, const real_t *a, const real_t *b, real_t *acc, real_t *res, realContext_t *realContext);
#endif // !INTEGRATE_H
