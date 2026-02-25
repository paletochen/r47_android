// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file random.h
 ***********************************************/
#if !defined(RANDOM_H)
  #define RANDOM_H

  void     fnRandom   (uint16_t unusedButMandatoryParameter);
  void     fnRandomI  (uint16_t unusedButMandatoryParameter);
  void     fnSeed     (uint16_t unusedButMandatoryParameter);
  void realRandomU01(real_t *res);
#endif // !RANDOM_H
