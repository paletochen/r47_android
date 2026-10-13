// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file isumprod.h
 ***********************************************/
#if !defined(ISUMPROD_H)
  #define ISUMPROD_H

//====================================
// Triadic function integer only sum and product functions
// Z : Longinteger counter step increment
// Y : Longinteger start value (inclusive)
// X : Longinteger end value (inclusive)
//====================================

  void fnProgrammableiSum    (uint16_t label);
  void fnProgrammableiProduct(uint16_t label);
#endif // !ISUMPROD_H
