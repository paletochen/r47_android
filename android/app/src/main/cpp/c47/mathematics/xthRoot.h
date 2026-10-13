// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file xthRoot.h
 ***********************************************/
#if !defined(XTHROOT_H)
  #define XTHROOT_H

  void fnXthRoot      (uint16_t unusedButMandatoryParameter);

  void xthRootReal(const real_t *yy, const real_t *xx, realContext_t *realContext);
#endif // !XTHROOT_H
