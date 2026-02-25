// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#if !defined(DISPLAY_H)
#define DISPLAY_H

// The maximum argument to FIX, SCI, ...
#define DSP_MAX     19

void fnDisplayFormatFix                (uint16_t displayFormatN);
void fnDisplayFormatSci                (uint16_t displayFormatN);
void fnDisplayFormatEng                (uint16_t displayFormatN);
void fnDisplayFormatAll                (uint16_t displayFormatN);
void fnDisplayFormatDsp                (uint16_t displayFormatN);
void fnDisplayFormatTime               (uint16_t displayFormatN);
void fnDisplayFormatSigFig             (uint16_t displayFormatN);
void fnDisplayFormatUnit               (uint16_t displayFormatN);
void fnShow                            (uint16_t unusedButMandatoryParameter);
void mimShowElement                    (void);
void fnView                            (uint16_t regist);
void fnAview                           (uint16_t regist);
void fnPrompt                          (uint16_t regist);

typedef enum {
  NOIRFRAC    = 0,
  LIMITIRFRAC = 1,
  LIGHTIRFRAC = 2,
  FULLIRFRAC  = 3
} irfracOption_t;
#define LIMITEXP    true
#define FRONTSPACE  true
void real34ToDisplayString             (const real34_t *real34, uint32_t tag, char *displayString, const font_t *font, int16_t maxWidth, int16_t displayHasNDigits, bool_t limitExponent, bool_t frontSpace, irfracOption_t limitIrfrac);
void dateToDisplayString               (calcRegister_t regist, char *displayString);
void complex34ToDisplayString          (const complex34_t *complex34, char *displayString, const font_t *font, int16_t maxWidth, int16_t displayHasNDigits, bool_t limitExponent, bool_t frontSpace, irfracOption_t limitIrfrac, const uint16_t tagAngle, const bool_t tagPolar);
void angle34ToDisplayString2           (const real34_t *angle34, uint8_t mode, char *displayString, int16_t displayHasNDigits, bool_t limitExponent, bool_t frontSpace, irfracOption_t limitIrfrac);
void _numerator                        (uint64_t numer, char *displayString, int16_t *endingZero);
void _denominator                      (uint64_t denom, char *displayString, int16_t *endingZero);
void fractionToDisplayString           (calcRegister_t regist, char *displayString);

void realToSci(real_t* num, char* dispString);

#define noBaseOverride 0
void shortIntegerToDisplayString       (calcRegister_t regist, char *displayString, bool_t determineFont, uint8_t baseOverride);
#define toRemoveTrailingRadix true
void longIntegerRegisterToRealDisplayString     (calcRegister_t regist, char *displayString, int32_t strLg, int16_t maxWidth, int32_t minimum, bool_t removeTrailingRadix);
void longIntegerToDisplayString        (longInteger_t lgInt, char *displayString, int32_t strLg, int16_t max_Width, int16_t maxExp, bool_t allowLARGELI);     //JM mod max_Width;   //JM added last parameter: Allow LARGELI
void longIntegerToHexDisplayString     (calcRegister_t regist, char *displayString, bool_t determineFont, uint8_t baseOverride, int32_t width);
void longIntegerRegisterToDisplayString(calcRegister_t regist, char *displayString, int32_t strLg, int16_t maxWidth, int16_t maxExp, bool_t allowLARGELI);    //JM added last parameter: Allow LARGELI
void dateToDisplayString               (calcRegister_t regist, char *displayString);
void timeToDisplayString               (calcRegister_t regist, char *displayString, bool_t ignoreTDisp);
void real34MatrixToDisplayString       (calcRegister_t regist, char *displayString);
bool_t vectorToDisplayString           (calcRegister_t regist, char *displayString);
void complex34MatrixToDisplayString    (calcRegister_t regist, char *displayString);
void exponentToDisplayString           (int32_t exponent, char *displayString, char *displayValueString, bool_t nimMode);
void supNumberToDisplayString          (int32_t supNumber, char *displayString, char *displayValueString, bool_t insertGap);
void subNumberToDisplayString          (int32_t subNumber, char *displayString, char *displayValueString);
void longIntegerToAllocatedString      (const longInteger_t lgInt, char *str, int32_t strLen);
void fnC47Show                         (uint16_t fnShow_param);    //JMSHOW

#endif // !DISPLAY_H
