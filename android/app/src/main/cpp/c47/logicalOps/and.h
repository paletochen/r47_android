// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file and.h
 ***********************************************/
#if !defined(AND_H)
  #define AND_H

  void fnLogicalAnd(uint16_t unusedButMandatoryParameter);

  void dyadicLogicalOp(const unsigned char table[4]);
  void logicalOpResult(bool_t res, uint32_t xtype, uint32_t ytype);
#endif // !AND_H
