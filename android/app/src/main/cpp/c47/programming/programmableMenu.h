// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file programmableMenu.h
 ***********************************************/
#if !defined(PROGRAMMABLEMENU_H)
  #define PROGRAMMABLEMENU_H

  void fnKeyGtoXeq       (uint16_t keyNum);
  void fnKeyGto          (uint16_t unusedButMandatoryParameter);
  void fnKeyXeq          (uint16_t unusedButMandatoryParameter);
  void fnProgrammableMenu(uint16_t unusedButMandatoryParameter);
  void fnClearMenu       (uint16_t unusedButMandatoryParameter);

  void keyGto            (uint16_t keyNum, uint16_t label);
  void keyXeq            (uint16_t keyNum, uint16_t label);
#endif // !PROGRAMMABLEMENU_H
