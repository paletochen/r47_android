// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#if !defined(DEBUG_H)
#define DEBUG_H

#if (DEBUG_INSTEAD_STATUS_BAR == 1)
  char  *getCalcModeName                    (uint16_t cm);
#endif //((DEBUG_INSTEAD_STATUS_BAR == 1)

void   formatReal34Debug                  (char *str, real34_t *real34);
void   formatRealDebug                    (char *str, real_t *real);
void   formatComplex34Debug               (char *str, void *addr);
const char *getDataTypeName               (uint16_t dt, bool_t article, bool_t padWithBlanks);
const char *getRegisterDataTypeName       (calcRegister_t regist, bool_t article, bool_t padWithBlanks);
const char *getRegisterTagName            (calcRegister_t regist, bool_t padWithBlanks);
const char *getShortIntegerModeName       (uint16_t im);
const char *getAngularModeName            (angularMode_t angularMode);
const char *getRunningModeName            (uint16_t mode);
const char *getCurveFitModeName           (uint16_t selection);
const char *getCurveFitModeNames          (uint16_t selection);
const char *getCurveFitModeFormula        (uint16_t selection);
char *eatSpacesEnd                        (const char * ss);
char *eatSpacesMid                        (const char * ss);
//void  debugNIM                            (void); Never used
void stackCheck                           (const unsigned char *begin, const unsigned char *end, int size, const char *where);
void initStackCheck                       (unsigned char *begin, unsigned char *end, int size);
void stackSmashingTest                    (void);
#endif // !DEBUG_H
