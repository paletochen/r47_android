// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file modulo.h
 ***********************************************/
#if !defined(MODULO_H)
  #define MODULO_H

  void fnMod(uint16_t unusedButMandatoryParameter);
  void integerRemainder(bool_t floorMod, bool_t shortResult);   // the body MOD and RMD share over two integers
#endif // !MODULO_H
