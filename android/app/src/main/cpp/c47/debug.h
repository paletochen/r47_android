// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#if !defined(DEBUG_H)
#define DEBUG_H

#if (DEBUG_PANEL == 1)
  extern GtkWidget *lbl1[DEBUG_LINES], *lbl2[DEBUG_LINES];
  extern GtkWidget *btnBitFields, *btnFlags, *btnRegisters, *btnLocalRegisters, *btnTmpAndSavedStackRegisters;
  extern GtkWidget *chkHexaString;
  extern int16_t debugWidgetDx, debugWidgetDy;

  void   btnBitFieldsClicked          (GtkWidget* w ,gpointer data);
  void   btnFlagsClicked              (GtkWidget* w ,gpointer data);
  void   btnRegistersClicked          (GtkWidget* w ,gpointer data);
  void   btnLocalRegistersClicked     (GtkWidget* w ,gpointer data);
  void   btnStatisticalSumsClicked    (GtkWidget* w ,gpointer data);
  void   btnNamedVariablesClicked     (GtkWidget* w ,gpointer data);
  void   btnSavedStackRegistersClicked(GtkWidget* w ,gpointer data);
  void   chkHexaStringClicked         (GtkWidget* w ,gpointer data);
  void   refreshDebugPanel            (void);

  char  *getDisplayFormatName               (uint16_t df);
  char  *getTimeFormatName                  (bool_t tf);
  char  *getDateFormatName                  (uint16_t df);
  char  *getBooleanName                     (bool_t b);
  char  *getRbrModeName                     (uint16_t mode);
  char  *getRoundingModeName                (uint16_t rm);
#endif //(DEBUG_PANEL == 1)
#if ((DEBUG_INSTEAD_STATUS_BAR == 1) || (DEBUG_PANEL == 1))
  char  *getCalcModeName                    (uint16_t cm);
#endif //((DEBUG_INSTEAD_STATUS_BAR == 1) || (DEBUG_PANEL == 1))
#if (DEBUG_PANEL == 1)
  char  *getNextCharName                    (uint16_t nc);
  char  *getComplexUnitName                 (bool_t cu);
  char  *getProductSignName                 (bool_t ps);
  char  *getFractionTypeName                (bool_t ft);
  char  *getFractionDenom1ModeName          (bool_t ft);
  char  *getFractionDenom2ModeName          (bool_t ft);
  char  *getRadixMarkName                   (bool_t rm);
  char  *getDisplayOvrName                  (bool_t dio);
  char  *getStackSizeName                   (bool_t ss);
  char  *getComplexModeName                 (bool_t cm);
  char  *getAlphaCaseName                   (uint16_t ac);
  char  *getAlphaSelectionMenuName          (uint16_t alsm);
  char  *getCursorFontName                  (uint16_t cf);
  char  *getSystemFlagName                  (uint16_t sf);
  void   memoryDump                         (bool_t bitFields, bool_t globalFlags, bool_t globalRegisters, bool_t localFlags, bool_t FIRSTLOCALREGISTERs, bool_t otherVars);
#endif // (DEBUG_PANEL == 1)

void   formatReal34Debug                  (char *str, real34_t *real34);
void   formatRealDebug                    (char *str, real_t *real);
void   formatComplex34Debug               (char *str, void *addr);

const char *getDataTypeName               (uint16_t dt, bool_t article, bool_t padWithBlanks);
const char *getRegisterDataTypeName       (calcRegister_t regist, bool_t article, bool_t padWithBlanks);
const char *getRegisterTagName            (calcRegister_t regist, bool_t padWithBlanks);
const char *getShortIntegerModeName       (uint16_t im);
const char *getAngularModeName            (angularMode_t angularMode);
const char *getCurveFitModeName           (uint16_t selection);
const char *getCurveFitModeNames          (uint16_t selection);
const char *getCurveFitModeFormula        (uint16_t selection);
char *eatSpacesEnd                        (const char * ss);
char *eatSpacesMid                        (const char * ss);

//void  debugNIM                            (void); Never used
void dumpScreenToConsole                  (void);

void dumpSubroutineLevelData              (void);
void testRegisters                        (const char *text);
void memoryDump2                          (const char *text);
void stackCheck                           (const unsigned char *begin, const unsigned char *end, int size, const char *where);
void initStackCheck                       (unsigned char *begin, unsigned char *end, int size);
void stackSmashingTest                    (void);
#endif // !DEBUG_H
