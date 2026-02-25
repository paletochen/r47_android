// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file integerPart.h
 ***********************************************/
#if !defined(INTEGERPART_H)
  #define INTEGERPART_H

  void fnIp   (uint16_t unusedButMandatoryParameter);

  /* Helpers for ceil, floor and here */
  void integerPartNoOp(void);
  void integerPartReal(enum rounding mode);
  void integerPartCplx(enum rounding mode);
#endif // !INTEGERPART_H
