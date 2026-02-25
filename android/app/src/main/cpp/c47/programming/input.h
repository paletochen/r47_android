// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file input.h
 ***********************************************/
#if !defined(INPUT_H)
  #define INPUT_H

  void fnInput  (uint16_t regist);
  void fnVarMnu (uint16_t label);
  void fnPause  (uint16_t duration);
  void fnKey    (uint16_t regist);
  void fnKeyType(uint16_t regist);
  void fnPutKey (uint16_t regist);
  void fnEntryQ (uint16_t unusedButMandatoryParameter);
#endif // !INPUT_H
