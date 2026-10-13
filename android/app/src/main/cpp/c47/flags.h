// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file flags.h
 ***********************************************/
#if !defined(FLAGS_H)
  #define FLAGS_H

  bool_t getFlag                 (uint16_t flag);
  void   fnGetSystemFlag         (uint16_t systemFlag);
  void   fnSetFlag               (uint16_t flag);
  void   fnClearFlag             (uint16_t flag);
  void   fnFlipFlag              (uint16_t flag);
  void   fnClFAll                (uint16_t confirmation);
  void   fnIsFlagClear           (uint16_t flag);
  void   fnIsFlagSet             (uint16_t flag);
  void   fnIsFlagClearClear      (uint16_t flag);
  void   fnIsFlagSetClear        (uint16_t flag);
  void   fnIsFlagClearSet        (uint16_t flag);
  void   fnIsFlagSetSet          (uint16_t flag);
  void   fnIsFlagClearFlip       (uint16_t flag);
  void   fnIsFlagSetFlip         (uint16_t flag);
  void   fnIsFlagSetFlip         (uint16_t flag);
  void   SetSetting              (uint16_t jmConfig);

  void   setSystemFlag           (unsigned int sf);
  void   clearSystemFlag         (unsigned int sf);
  bool_t getSystemFlag           (int32_t sf);
  bool_t didSystemFlagChange     (int32_t sf);
  void   setAllSystemFlagChanged (void);
  void   setSystemFlagChanged    (int32_t sf);
  void   flipSystemFlag          (unsigned int sf);
  void   forceSystemFlag         (unsigned int sf, int set);

#endif // !FLAGS_H
