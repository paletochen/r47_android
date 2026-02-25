// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file decode.h
 ***********************************************/
#if !defined(DECODE_H)
  #define DECODE_H

  extern const char           baseChars[];
  extern const char           angleChars[];

  void decodeOneStep          (uint8_t *step);
  void decodeOneStep_XPORT    (uint8_t *step);
  void listPrograms           (void);
  void listLabelsAndPrograms  (void);
#endif // !DECODE_H
