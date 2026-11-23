// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file int.c
 ***********************************************/

#include "c47.h"

void fnCheckInteger(uint16_t mode) {
  longInteger_t x;
  bool_t frac;

  if(getRegisterAsLongIntQuiet(REGISTER_X, x, &frac) != ERROR_NONE) {
    compareTypeErrorX();
  }
  else if(frac) {
    SET_TI_TRUE_FALSE(mode == CHECK_INTEGER_FP);
  }
  else {
      lcd_refresh();

    switch(mode) {
      case CHECK_INTEGER: {
        temporaryInformation = TI_TRUE;
        break;
      }

      case CHECK_INTEGER_EVEN: {
        SET_TI_TRUE_FALSE(longIntegerIsEven(x));
        break;
      }

      case CHECK_INTEGER_ODD: {
        SET_TI_TRUE_FALSE(longIntegerIsOdd(x));
        break;
      }

      case CHECK_INTEGER_FP: {
        temporaryInformation = TI_FALSE;
        break;
      }
    }
  }
  longIntegerFree(x);
}
