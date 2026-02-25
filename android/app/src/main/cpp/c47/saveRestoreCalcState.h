// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file saveRestoreCalcState.h
 */
#if !defined(SAVERESTORECALCSTATE_H)
  #define SAVERESTORECALCSTATE_H


  #define autoLoad   0
  #define manualLoad 1
  #define stateLoad  2
  #define autoSave   3
  #define manualSave 4
  #define stateSave  5

  void     saveCalc      (void);
  void     restoreCalc   (void);

  void     fnSave        (uint16_t unusedButMandatoryParameter);
  void     fnLoad        (uint16_t loadMode);
  void     fnSaveAuto    (uint16_t unusedButMandatoryParameter);
  void     fnLoadAuto    (void);
  void     fnLoadedFile  (uint16_t unusedButMandatoryParameter);
  uint8_t  stringToUint8 (const char *str);
  uint16_t stringToUint16(const char *str);
  uint32_t stringToUint32(const char *str);
  uint64_t stringToUint64(const char *str);
  int8_t   stringToInt8  (const char *str);
  int16_t  stringToInt16 (const char *str);
  int32_t  stringToInt32 (const char *str);
  int64_t  stringToInt64 (const char *str);
  float    stringToFloat (const char *str);

  int32_t  toInt32 (const char *str);

  void     readLine      (char *line);
  void     read2Lines    (char *line1, char *line2);

  void     doLoad        (uint16_t loadMode, uint16_t s, uint16_t n, uint16_t d, uint16_t loadType);

  void     fnDeleteBackup(uint16_t confirmation);

#endif // !SAVERESTORECALCSTATE_H
