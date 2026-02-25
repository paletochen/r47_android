// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"

#define isComplex true
#define isReal    false
static void real34ToDisplayString2(const real34_t *real34, char *displayString, int16_t displayHasNDigits, bool_t limitExponent, bool_t noFix, bool_t frontSpace, bool_t complex, irfracOption_t limitIrfrac);
static void complex34ToDisplayString2(const complex34_t *complex34, char *displayString, int16_t displayHasNDigits, bool_t limitExponent, bool_t frontSpace, const uint16_t tagAngle, const bool_t tagPolar, irfracOption_t limitIrfrac);
static void insertSepsIntoIntegerText(char *displayString);


static void fnDisplayFormatReset(uint16_t displayFormatN) {
  displayFormatDigits = displayFormatN > DSP_MAX ? DSP_MAX : displayFormatN;
  clearSystemFlag(FLAG_FRACT);
  DM_Cycling = 0;
}

/********************************************//**
 * \brief Sets the display format FIX and refreshes the stack
 *
 * \param[in] displayFormatN uint16_t Display format
 * \return void
 ***********************************************/
void fnDisplayFormatFix(uint16_t displayFormatN) {
  fnDisplayFormatReset(displayFormatN);
  displayFormat = DF_FIX;
  fnRefreshState();                              //drJM
}


/********************************************//**
 * \brief Sets the display format SCI and refreshes the stack
 *
 * \param[in] displayFormatN uint16_t Display format
 * \return void
 ***********************************************/
void fnDisplayFormatSci(uint16_t displayFormatN) {
  fnDisplayFormatReset(displayFormatN);
  displayFormat = DF_SCI;
  fnRefreshState();                              //drJM
}


/********************************************//**
 * \brief Sets the display format ENG and refreshes the stack
 *
 * \param[in] displayFormatN uint16_t Display format
 * \return void
 ***********************************************/
void fnDisplayFormatEng(uint16_t displayFormatN) {
  fnDisplayFormatReset(displayFormatN);
  displayFormat = DF_ENG;
  fnRefreshState();                              //drJM
}


/********************************************//**
 * \brief Sets the display format ALL and refreshes the stack
 *
 * \param[in] displayFormatN uint16_t Display format
 * \return void
 ***********************************************/
void fnDisplayFormatAll(uint16_t displayFormatN) {
  fnDisplayFormatReset(displayFormatN);
  displayFormat = DF_ALL;
  fnRefreshState();                              //drJM
}


/********************************************//**
 * \Set SIGFIG mode
 *
 * FROM DISPLAY.C
 ***********************************************/
void fnDisplayFormatSigFig(uint16_t displayFormatN) {
  fnDisplayFormatReset(displayFormatN);
  displayFormat = DF_SF;
  fnRefreshState();
}


/********************************************//**
 * \Set UNIT mode
 *
 * FROM DISPLAY.C
 ***********************************************/
void fnDisplayFormatUnit(uint16_t displayFormatN) {
  fnDisplayFormatReset(displayFormatN);
  displayFormat = DF_UN;
  fnRefreshState();
}


/********************************************//**
 * \brief Sets the number of digits afer the period and refreshes the stack
 *
 * \param[in] displayFormatN uint16_t Display format
 * \return void
 ***********************************************/
void fnDisplayFormatDsp(uint16_t displayFormatN) {
  displayFormatN = displayFormatN > DSP_MAX ? DSP_MAX : displayFormatN;
  displayFormatDigits = displayFormatN;
  clearSystemFlag(FLAG_FRACT);
  fnRefreshState();                              //drJM
}


/********************************************//**
 * \brief Sets the display format for time and refreshes the stack
 *
 * \param[in] displayFormatN uint16_t Display format
 * \return void
 ***********************************************/
void fnDisplayFormatTime(uint16_t displayFormatN) {
  timeDisplayFormatDigits = displayFormatN > DSP_MAX ? DSP_MAX : displayFormatN;
}


/********************************************//**
 * \brief Adds the power of 10 using numeric font to displayString
 *
 * \param[out] displayString char*     Result string
 * \param[in]  exponent int32_t Power of 10 to format
 * \return void
 ***********************************************/
void exponentToDisplayString(int32_t exponent, char *displayString, char *displayValueString, bool_t nimMode) {
  strcpy(displayString, PRODUCT_SIGN);
  displayString += 2;
  strcpy(displayString, STD_SUB_10);
  displayString += 2;
  displayString[0] = 0;

  if(displayValueString != NULL) {
    *displayValueString++ = 'e';
    *displayValueString = 0;
  }

  if(nimMode) {
    if(exponent != 0) {
      supNumberToDisplayString(exponent, displayString, displayValueString, false);
    }
  }
  else {
    supNumberToDisplayString(exponent, displayString, displayValueString, false);
  }
}


void supNumberToDisplayString(int32_t supNumber, char *displayString, char *displayValueString, bool_t insertGap) {
  if(displayValueString != NULL) {
    sprintf(displayValueString, "%" PRId32, supNumber);
  }

  if(supNumber == 0) {
    strcat(displayString, STD_SUP_0);
  }
  else {
    int16_t digitCount=0;
    bool_t greaterThan9999;

    if(supNumber < 0) {
      supNumber = -supNumber;
      strcat(displayString, STD_SUP_MINUS);
      displayString += 2;
    }

    greaterThan9999 = (supNumber > 9999);
    while(supNumber > 0) {
      int16_t digit;

      digit = supNumber % 10;
      supNumber /= 10;

      xcopy(displayString + 2, displayString, stringByteLength(displayString) + 1);

      displayString[0] = STD_SUP_0[0];
      displayString[1] = STD_SUP_0[1];
      displayString[1] += digit;

      if(insertGap && greaterThan9999 && supNumber > 0 && !GROUPLEFT_DISABLED && ((++digitCount) % GROUPWIDTH_LEFT) == 0) {
        if(SEPARATOR_LEFT[1]!=1) {
          xcopy(displayString + 2, displayString, stringByteLength(displayString) + 1);
          displayString[0] = SEPARATOR_LEFT[0];
          displayString[1] = SEPARATOR_LEFT[1];
        }
        else if(SEPARATOR_LEFT[0]!=1) {
          xcopy(displayString + 1, displayString, stringByteLength(displayString) + 1);
          displayString[0] = SEPARATOR_LEFT[0];
        }
      }
    }
  }

  strcat(displayString, STD_SPACE_HAIR);
}


void subNumberToDisplayString(int32_t subNumber, char *displayString, char *displayValueString) {
  if(displayValueString != NULL) {
    sprintf(displayValueString, "%" PRId32, subNumber);
  }

  if(subNumber < 0) {
    subNumber = -subNumber;
    strcat(displayString, STD_SUB_MINUS);
    displayString += 2;
  }

  if(subNumber == 0) {
    strcat(displayString, STD_SUB_0);
  }
  else {
    while(subNumber > 0) {
      int16_t digit = subNumber % 10;
      subNumber /= 10;

      xcopy(displayString + 2, displayString, stringByteLength(displayString) + 1);

      displayString[0] = STD_SUB_0[0];
      displayString[1] = STD_SUB_0[1];
      displayString[1] += digit;
    }
  }
}


void real34ToDisplayString(const real34_t *real34, uint32_t tag, char *displayString, const font_t *font, int16_t maxWidth, int16_t displayHasNDigits, bool_t limitExponent, bool_t frontSpace, irfracOption_t limitIrfrac) {
  uint8_t savedDisplayFormatDigits = displayFormatDigits;
  uint8_t savedDisplayFormat       = displayFormat;
  bool_t  ovrENG = getSystemFlag(FLAG_ENGOVR);

  if(calcMode == CM_PEM) {
    displayFormat = DF_ALL;         // Display all digits for reals in PEM
    clearSystemFlag(FLAG_ENGOVR);   // Default to SCI display in PEM for large reals
  }

  #if (REAL34_WIDTH_TEST == 1)
    maxWidth = largeur;
  #endif // (REAL34_WIDTH_TEST == 1)

  if(displayFormat == DF_SF && checkHP) {        //This portion limits the SIGFIG digits to really n digits, even in the case of SIG3 12345000000000 to be displayed as 1.2340 x 10^5
    displayHasNDigits =  10;
  }

  do {
    if(updateDisplayValueX) {
      displayValueX[0] = 0;
    }
    if(tag == amNone) {
      real34ToDisplayString2(real34, displayString, displayHasNDigits, limitExponent, false, frontSpace, isReal, limitIrfrac);
    }
    else {
      angle34ToDisplayString2(real34, tag, displayString, displayHasNDigits, limitExponent, frontSpace, limitIrfrac);
    }

    if(displayFormat == DF_ALL || displayFormat == DF_SF) {
      if(displayHasNDigits == 2) {
        break;
      }
      displayHasNDigits--;
    }
    else {
      if(displayFormatDigits == 0) {
        break;
      }
      displayFormatDigits--;
    }
  }
  while(stringWidth(displayString, font, true, true) > maxWidth);

  displayFormat       = savedDisplayFormat;
  displayFormatDigits = savedDisplayFormatDigits;
  if(ovrENG) setSystemFlag(FLAG_ENGOVR);
}


/********************************************//**
 * \brief Formats a real
 *
 * \param[out] displayString char* Result string
 * \param[in]  x const real34_t*  Value to format
 * \return void
 ***********************************************/
static void real34ToDisplayString2(const real34_t *real34, char *displayString, int16_t displayHasNDigits, bool_t limitExponent, bool_t noFix, bool_t frontSpace, bool_t complex, irfracOption_t limitIrfrac) {
  #undef MAX_DIGITS
  #define MAX_DIGITS 37 // 34 + 1 before (used when rounding from 9.999 to 10.000) + 2 after (used for rounding and ENG display mode)
  #define exponentUNlimit1024max (getSystemFlag(FLAG_PFX_ALL) ? 7 : 5) //1024^7 is the maximum UNIT_1024^n before skipping over to standard unit presentation

  uint8_t charIndex, valueIndex;
  int16_t digitToRound=0;
  uint8_t *bcd;
  int16_t digitsToDisplay=0, numDigits, digitPointer, firstDigit, lastDigit, i, digitCount, digitsToTruncate, exponent;
  int32_t sign;
  bool_t  ovrSCI=false, ovrENG=false, firstDigitAfterPeriod=true;
  real34_t value34;


  //Convert the incoming number in decimal, to the equivalent number in base 1024, multipled by 1000.
  //Example: 1025 -> 1024^1.000140819 -> 1024^.000140819 * 1024^1 -> 1.000976559 * Ki -> 1.001 Ki
  //             ln(1025)/ln(1024) = 1.000140819;

  int32_t exponentUNlimit = 0;
  bool_t flag2To10 = getSystemFlag(FLAG_2TO10);
  bool_t flag2To10_baseunit_integer = false;
  real_t tmp4, tmpIp, tmpFp;
  real34_t real34bak;
  real34Copy(real34, &real34bak);
  if(flag2To10 && displayFormat == DF_UN) {
    real_t x, xx;
    real34ToReal(real34, &x);
    int32ToReal(1024, &tmp4);

    if(!realCompareAbsLessThan(&x, &tmp4)) {

      //x = e^[ ln real34 / ln1024 ]
      bool_t neg = false;
      if(realIsNegative(&x)) {
        realSetPositiveSign(&x);
        neg = true;
      }

      realCopy(&x,&xx);

      //get log base 1024 of real34
      decContext c = ctxtReal39;
      int maxExponent = x.exponent + x.digits;
      c.digits = (SHOWMODE ? 39 : min(75,max(0,maxExponent) + NUMBER_OF_DISPLAY_REAL_CONTEXT_DIGITS));
      WP34S_Ln(&x, &x, &c);                             //x = ln|real34|
      maxExponent = x.exponent + x.digits;
      c.digits = (SHOWMODE ? 39 : min(75,max(0,maxExponent) + NUMBER_OF_DISPLAY_REAL_CONTEXT_DIGITS));
      realDivide(&x, const_ln2, &x, &c);                //ln(1024)=ln( 2^10 )=10ln(2)
      x.exponent--; // x = x / 10
      //printRealToConsole(&x,"log base 1024 of real34 = lnx / ln1024 ","\n");             // x = ln|real34| / ln(1024) = log base 1024 of real34 = 1.00140

      //get IP and FP of this
      realToIntegralValue(&x, &tmpIp, DEC_ROUND_DOWN, &c); // tmpIp = Integer Part log base1024 of Real34    = 1
      int tmpx = realToInt32C47(&tmpIp);
      if(tmpx > exponentUNlimit1024max) {
        goto overRange;
      }
      exponentUNlimit = min(exponentUNlimit1024max, tmpx);
      int32ToReal(exponentUNlimit, &tmpIp);
      realSubtract(&x, &tmpIp, &tmpFp, &c);                // tmpFp = Fractional part log base1024 of Real34    = 0.00140
      //printRealToConsole(&tmpIp, "tmpIp Ip ", "\n");
      //printRealToConsole(&tmpFp, "Fp ", "\n");

      //   = 1000 ^ IP(log base1024 of Real34)
      //   = 1024 ^ IP(log base1024 of Real34)
      // new Real34 = Real34 / 1024^IP * 1000^IP

      // fact = IP§ / IP = (1000/1024)^IP(log base1024 of Real34)
      // new Real34 = Real34 fact

      real_t tmp3, fact;
      int32ToReal(1000, &tmp3);
      int32ToReal(1024, &tmp4);
      realDivide(&tmp3, &tmp4, &fact, &ctxtReal39);
      //printRealToConsole(&fact, "factor = ", "\n");
      realPower(&fact, &tmpIp, &tmp3, &ctxtReal39);
      //printRealToConsole(&tmp3, "factor^IP = ", "\n");
      //printRealToConsole(&xx, "xx = ", "\n");
      realMultiply(&xx, &tmp3, &x, &ctxtReal39);
      //printRealToConsole(&x, "x * fact = ", "\n");

      if(neg) {
        realSetNegativeSign(&x);
      }
      realToReal34(&x, real34);
      //printReal34ToConsole(real34,"---B","\n");

    }
    else {
      flag2To10_baseunit_integer = true;
overRange:
      flag2To10 = false;
    }
  }


  // IRFRAC multiples and fractions of constants
  //   Not checked for reals smaller than 1x10^-6 and integers
  //   Fractions are switched off id MULTPI is used
  //   Checking and showing pure fractions, see table below
  //   Checking and showing fractions with √3, pi, e, √2, see table below
  //   Checking and showing fractions with ɸ, √5, √7, √𝝅, 1/𝝅, 1/e, see table below
  real_t value;

#define oneOverPi "(" STD_pi STD_SUP_MINUS STD_SUP_1 STD_SPACE_HAIR ")"    //OPT1
//#define oneOverPi STD_pi STD_SUP_MINUS STD_SUP_1                         //OPT2 (original)
//#define oneOverPi "(" STD_SUP_1 "/" STD_pi ")"                           //OPT3
//#define oneOverPi STD_pi STD_SUP_MINUS STD_SUP_1 STD_SPACE_4_PER_EM      //OPT4
  //#define oneOverPi "(1/" STD_pi ")"
  //#define oneOverPi "(" STD_SUP_1 "/" STD_SUB_pi ")"   //really not nice
#define oneOverE "(" STD_EulerE STD_SUP_MINUS STD_SUP_1 STD_SPACE_HAIR ")" //OPT1
//#define oneOverE STD_EulerE STD_SUP_MINUS STD_SUP_1                      //OPT2 (original)
//#define oneOverE "(" STD_SUP_1 "/" STD_EulerE ")"                        //OPT3
//#define oneOverE STD_EulerE STD_SUP_MINUS STD_SUP_1 STD_SPACE_4_PER_EM   //OPT4
  //#define oneOverE "(1/" STD_EulerE ")"
  //#define oneOverE "(" STD_SUP_1 "/" STD_SUB_e ")"     //really ungly

  //printf(">>>## flag_proper %u\n",getSystemFlag(FLAG_PROPFR));
  //printReal34ToConsole(real34, "Irfrac: ","\n");
  if(limitIrfrac != NOIRFRAC) {
    if(getSystemFlag(FLAG_IRFRAC) && IrFractionsCurrentStatus != CF_OFF && !real34CompareAbsLessThan(real34,const34_1e_24) && !real34IsAnInteger(real34)) {      // LIMITIRFRAC [Real Matrixes, Complex Matrixes] & LIGHTIRFRAC [Vectors] for USB/BAT/SIM; FULLIRFRAC [Real, Complex] : pure fractions
      const real_t *toleranceIrrational = const_1e_24;
      real_t valueReal, valueRealAbs;
      TO_QSPI static const struct {
        const real_t *cnst;         // 4 bytes
        const char name[14];        // 14 bytes
        char terminator;            // 15 bytes
        const unsigned char option; // 16 bytes containts an irfracOption_t
      } replacements[] = {
          { const_1,        "",                           0, NOIRFRAC },
          { const_rt3,      STD_SQUARE_ROOT STD_SUB_3,    0, LIMITIRFRAC },
          { const_root2,    STD_SQUARE_ROOT STD_SUB_2,    0, LIMITIRFRAC },
          { const_pi,       STD_pi,                       0, LIMITIRFRAC },
          { const_eE,       STD_EulerE,                   0, LIGHTIRFRAC },
          { const_PHI,      STD_phi_m,                    0, LIGHTIRFRAC },
          { const_rt5,      STD_SQUARE_ROOT STD_SUB_5,    0, LIGHTIRFRAC },
          { const_rt7,      STD_SQUARE_ROOT STD_SUB_7,    0, LIGHTIRFRAC },
          { const_rtpi,     STD_SQUARE_ROOT STD_pi,       0, LIGHTIRFRAC },
          { const_1onpi,    oneOverPi,                    0, LIGHTIRFRAC },
          { const_1oneE,    oneOverE,                     0, LIGHTIRFRAC },
          { const_pisq,     "(" STD_pi STD_SUP_2 STD_SPACE_HAIR STD_SPACE_HAIR ")",     0, FULLIRFRAC },
          { const_eEsq,     "(" STD_EulerE STD_SUP_2 STD_SPACE_HAIR STD_SPACE_HAIR ")", 0, FULLIRFRAC },
          { const_1onpisq,  "(" STD_pi STD_SUP_MINUS STD_SUP_2 STD_SPACE_HAIR STD_SPACE_HAIR ")",     0, FULLIRFRAC },
          { const_1oneEsq,  "(" STD_EulerE STD_SUP_MINUS STD_SUP_2 STD_SPACE_HAIR STD_SPACE_HAIR ")", 0, FULLIRFRAC },
      };

      real34ToReal(real34,&valueReal);
      realCopyAbs(&valueReal,&valueRealAbs);
      for (unsigned int i=0; i<nbrOfElements(replacements); i++)
        if ((limitIrfrac >= replacements[i].option && runningOnSimOrUSB) || limitIrfrac == FULLIRFRAC)
          if(checkForAndChange(displayString, &valueReal, &valueRealAbs, replacements[i].cnst, toleranceIrrational, replacements[i].name, frontSpace, complex)) {
            IrFractionsCurrentStatus = CF_NORMAL;
            return;
          }
    }
  }
  IrFractionsCurrentStatus = CF_NORMAL;


  //sigfig
  //printReal34ToConsole(real34," ------- 001 >>>>>"," <<<<<\n");   //JM
  if(displayFormat == DF_SF) {                                 //convert real34 to string, eat away all zeroes from the right and give back to FIX as a real
    exponent = real34GetExponent(real34) + real34Digits(real34) - 1;
    if (abs(exponent) <= displayHasNDigits) {
      char tmpString100[100];                           //cleaning up the REAL
      real34_t reduced;
      real_t tmp1;

      // printReal34ToConsole(real34," ------- 002a >>>>>"," <<<<<\n");   //JM
      real34ToReal(real34, &tmp1);
      decContext c = ctxtReal39;
      c.digits = (SHOWMODE ? 39 : NUMBER_OF_DISPLAY_REAL_CONTEXT_DIGITS);
      roundToSignificantDigits(&tmp1, &tmp1, displayFormatDigits+1, &c); //  &ctxtReal75);
      realToReal34(&tmp1, &reduced);
      // printReal34ToConsole(&reduced," ------- 002b >>>>>"," <<<<<\n");   //JM
      real34Reduce(&reduced, &reduced);
      // printReal34ToConsole(&reduced," ------- 002c >>>>>"," <<<<<\n");   //JM
      real34ToString(&reduced, tmpString100);
      // printf("------- 003 displayFormatDigits=%u >>>>>%s\n",displayFormatDigits, tmpString100);
      int8_t ii=0;
      while(tmpString100[ii]!=0) {                       //skip all zeroes
        while(tmpString100[ii] == '0') {
          if(tmpString100[ii] == 0) break;
          ii++;
        }                                                //counter at first non-'0' or at end
        if(tmpString100[ii] == '.') {
        // printf("------- 004 >>>>%s|, %i\n",tmpString100, ii);

            ii++;                                        //move to first non-'.' and skip all zeroes
            while(tmpString100[ii] == '0') {
              if(tmpString100[ii] == 0) break;
              ii++;
            }                                            //counter at first non-'0' or end, eg. 3.14159265358979E+15
            // printf("------- 004a >>>>%s|, %i, displayFormatDigits=%i\n",tmpString100, ii, displayFormatDigits);

            if(tmpString100[ii] != 0) {
              ii = ii + displayFormatDigits+1;           //2023-06-01 added 1 digit, giving FIX one extra digit for rounding. If it does not work properly, to do rounding here.
              int8_t jj = ii;
              //round here

              while(tmpString100[jj] != 0 && tmpString100[jj] != 'E') {   //find E or first non-zero
                jj++;
              }
              if(tmpString100[jj] == 'E') {              //If E, then move over the exponent to have only the specified significant digts, eg. 3.141E+15
                while(tmpString100[jj] != 0) {
                  tmpString100[ii] = tmpString100[jj];
                  jj++; ii++;
                }
                tmpString100[ii] = 0;
              }
            }
          // printf("------- 005 >>>>%s|\n",tmpString100);
          break;
        }
        else {
          ii++;
        }

      }

      stringToReal(tmpString100,&value,&ctxtReal39);
      // printRealToConsole(&value," ------- 006 >>>>>"," <<<<<\n\n");   //JM
    }
    else {
      real34ToReal(real34, &value);
    }
  }
  else {
    real34ToReal(real34, &value);
  }
  //printRealToConsole(&value," ------- 006 >>>>>"," <<<<<\n\n");   //JM

  if(checkHP) {
    // Forced rounding at 10 digits when HP35 selected, to not risk any guard digits or digit noise in the last digits, see 'decNumberPlus'
    ctxtReal39.digits = min(10,displayHasNDigits);
    realPlus(&value, &value, &ctxtReal39);
    ctxtReal39.digits = 39;
  }

  realToReal34(&value, &value34);
  if(displayFormat == DF_SF) {
    real34Reduce(&value34, &value34);  //JM NEW SIG 2023-03-18
  }
  if(real34IsNegative(real34)) {
    real34SetNegativeSign(&value34);
  }

  char tmpString100[100];
  bcd = (uint8_t *)(tmpString100);
  memset(bcd, 0, MAX_DIGITS);

  sign = real34GetCoefficient(&value34, bcd + 1);
  exponent = real34GetExponent(&value34);

  // Calculate the number of significant digits
  for(digitPointer=1; digitPointer<=MAX_DIGITS-3; digitPointer++) {
    if(bcd[digitPointer] != 0) {
      break;
    }
  }

  if(digitPointer >= MAX_DIGITS-2) { // *real = 0.0
    firstDigit = 0;
    lastDigit  = 0;
    numDigits  = 1;
    exponent   = 0;
  }
  else {
    firstDigit = digitPointer;

    for(digitPointer=MAX_DIGITS-3; digitPointer>=1; digitPointer--) {
      if(bcd[digitPointer] == 0) {
        exponent++;
      }
      else {
        break;
      }
    }
    lastDigit = digitPointer;

    numDigits = lastDigit - firstDigit;
    exponent += numDigits++;
  }
/*
  if(limitExponent && abs(exponent) > exponentLimit) {
    if(exponent > exponentLimit) {
      if(real34IsPositive(&value34)) {
        realToReal34(const_plusInfinity, &value34);
      }
      else {
        realToReal34(const_minusInfinity, &value34);
      }
    }
    else if(exponent < -exponentLimit) {
      real34Zero(&value34);

      bcd = (uint8_t *)(tmpString + 256 - MAX_DIGITS);
      memset(bcd, 0, MAX_DIGITS);

      sign = 0;
      exponent = 0;
      firstDigit = 0;
      lastDigit  = 0;
      numDigits  = 1;
      exponent   = 0;
    }
  }*/

  // printf("value34 (INT)=%i exponent=%i limitExponent=%i (exponentLimit=%i) (exponentHideLimit=%i) \n", real34ToUInt32(&value34),  exponent,limitExponent, exponentLimit, exponentHideLimit);

  if(limitExponent) {
    if(exponent > exponentLimit) {
      if(real34IsPositive(&value34)) {
        if(frontSpace) {
          strcpy(displayString, " " STD_LEFT_SINGLE_QUOTE STD_INFINITY STD_RIGHT_SINGLE_QUOTE);
        }
        else {
          strcpy(displayString, STD_LEFT_SINGLE_QUOTE STD_INFINITY STD_RIGHT_SINGLE_QUOTE);
        }
        if(updateDisplayValueX) {
          strcpy(displayValueX + strlen(displayValueX), "9e9999");
        }
      }
      else if(real34IsNegative(&value34)) {
        strcpy(displayString, "-" STD_LEFT_SINGLE_QUOTE STD_INFINITY STD_RIGHT_SINGLE_QUOTE);
        if(updateDisplayValueX) {
          strcpy(displayValueX + strlen(displayValueX), "-9e9999");
        }
      }
      return;
    }
    else if(exponent < -exponentLimit || (exponentHideLimit != 0 && exponent < -exponentHideLimit)) {
      if(real34IsPositive(&value34)) {
        strcpy(displayString, STD_ALMOST_EQUAL "0");
        if(updateDisplayValueX) {
          strcpy(displayValueX + strlen(displayValueX), "0");
        }
      }
      else if(real34IsNegative(&value34)) {
        strcpy(displayString, STD_ALMOST_EQUAL "-0");
        if(updateDisplayValueX) {
          strcpy(displayValueX + strlen(displayValueX), "-0");
        }
      }
      return;
    }
  }

  if(real34IsInfinite(&value34)) {
    if(real34IsNegative(&value34)) {
      strcpy(displayString, "-" STD_INFINITY);
      if(updateDisplayValueX) {
        strcpy(displayValueX + strlen(displayValueX), "-9e9999");
      }
    }
    else {
      strcpy(displayString, " " STD_INFINITY);
      if(updateDisplayValueX) {
        strcpy(displayValueX + strlen(displayValueX), "9e9999");
      }
    }
    return;
  }

  if(real34IsNaN(&value34)) {
    real34ToString(&value34, displayString);
    if(updateDisplayValueX) {
      real34ToString(&value34, displayValueX + strlen(displayValueX));
    }
    return;
  }

  charIndex = 0;
  valueIndex = (updateDisplayValueX ? strlen(displayValueX) : 0);

  //////////////
  // ALL mode //
  //////////////
  if(displayFormat == DF_ALL) {
    // Number of digits to truncate
    digitsToTruncate = max(numDigits - displayHasNDigits, 0);
    numDigits -= digitsToTruncate;
    lastDigit -= digitsToTruncate;

    // Round the final 9s
    if(bcd[lastDigit+1] == 9) {
      bcd[lastDigit]++;

      // Transfert the carry
      while(bcd[lastDigit] == 10) {
        bcd[lastDigit--] = 0;
        numDigits--;
        bcd[lastDigit]++;
      }

      // Case when 9.9999 rounds to 10.0000
      if(lastDigit < firstDigit) {
        firstDigit--;
        lastDigit = firstDigit;
        numDigits = 1;
        exponent++;
      }
    }

    if(noFix || exponent >= displayHasNDigits ||
        exponent <= -displayHasNDigits ||
        (displayFormatDigits != 0 && exponent < -(int32_t)displayFormatDigits) ||
        (displayFormatDigits == 0 && exponent < numDigits - displayHasNDigits)) { // Display in SCI or ENG format
      digitsToDisplay = min(displayHasNDigits, numDigits - 1);
      digitToRound    = min(firstDigit + digitsToDisplay, lastDigit);
      ovrSCI = !getSystemFlag(FLAG_ENGOVR);
      ovrENG = getSystemFlag(FLAG_ENGOVR);
    }
    else { // display all digits without ten exponent factor
      // Round the displayed number
      if(bcd[lastDigit+1] >= 5) {
        bcd[lastDigit]++;
      }

      // Transfert the carry
      while(bcd[lastDigit] == 10) {
        bcd[lastDigit--] = 0;
        numDigits--;
        bcd[lastDigit]++;
      }

      // Case when 9.9999 rounds to 10.0000
      if(lastDigit < firstDigit) {
        firstDigit--;
        lastDigit = firstDigit;
        numDigits = 1;
        exponent++;
      }

      // Remove trailling zeros
      while(numDigits > 1 && bcd[lastDigit] == 0) {
        lastDigit--;
        numDigits--;
      }

      // The sign
      if(sign) {
        displayString[charIndex++] = '-';
        if(updateDisplayValueX) {
          displayValueX[valueIndex++] = '-';
        }
      }
      else {
        if(frontSpace) {
          displayString[charIndex++] = ' ';
        }
      }

      if(exponent < 0) { // negative exponent
        // first 0 and radix mark
        displayString[charIndex++] = '0';
        if(updateDisplayValueX) {
          displayValueX[valueIndex++] = '0';
        }
        displayString[charIndex] = 0;
        char tt[4];
        if(RADIX34_MARK_STRING[1]!=1) {strcpy(tt,RADIX34_MARK_STRING);}
        else {tt[0] = RADIX34_MARK_STRING[0]; tt[1] = 0;}
        strcat(displayString, tt);
        charIndex += strlen(tt);
        if(updateDisplayValueX) {
          displayValueX[valueIndex++] = '.';
        }

        // Zeros before first significant digit
        for(digitCount=0, i=exponent+1; i<0; i++, digitCount--) {
          if(digitCount != 0 && !GROUPRIGHT_DISABLED && digitCount%(uint16_t)GROUPWIDTH_RIGHT == 0) {
            xcopy(displayString + charIndex,  SEPARATOR_RIGHT, SEPARATOR_RIGHT[0]!=1 ? (SEPARATOR_RIGHT[1]!=1 ? 2 : 1) : 0);
            charIndex +=  ( SEPARATOR_RIGHT[0]!=1 ? (SEPARATOR_RIGHT[1]!=1 ? 2 : 1) : 0);
          }
          displayString[charIndex++] = '0';
          if(updateDisplayValueX) {
            displayValueX[valueIndex++] = '0';
          }
        }

        // Significant digits
        for(digitPointer=firstDigit; digitPointer<firstDigit+min(displayHasNDigits - 1 - exponent, numDigits); digitPointer++, digitCount--) {
          if(digitCount != 0 && !GROUPRIGHT_DISABLED && digitCount%(uint16_t)GROUPWIDTH_RIGHT == 0) {
            xcopy(displayString + charIndex, SEPARATOR_RIGHT,  SEPARATOR_RIGHT[0]!=1 ? (SEPARATOR_RIGHT[1]!=1 ? 2 : 1) : 0);
            charIndex +=  ( SEPARATOR_RIGHT[0]!=1 ? (SEPARATOR_RIGHT[1]!=1 ? 2 : 1) : 0);
          }
          displayString[charIndex++] = '0' + bcd[digitPointer];
          if(updateDisplayValueX) {
            displayValueX[valueIndex++] = '0' + bcd[digitPointer];
          }
        }
      }
      else { // zero or positive exponent
        for(digitCount=exponent, digitPointer=firstDigit; digitPointer<=lastDigit + max(exponent - numDigits + 1, 0); digitPointer++, digitCount--) {

//vvGRP handling
          if(digitCount!=-1 && digitCount!=exponent && GROUPWIDTH_(digitCount)!=0
                            && IS_SEPARATOR_(digitCount)
                            && (GROUP1_OVFL(digitCount, exponent)==0 || bcd[digitPointer-1] >= GROUP1_OVFL(digitCount, exponent) + 1)   ) {
            //printf("GROUPWIDTH_=%2i digitCountNEW=%2i IS_SEPARATOR_=%2i \n",GROUPWIDTH_(digitCount), digitCountNEW(digitCount), IS_SEPARATOR_(digitCount) );
            xcopy(displayString + charIndex, SEPARATOR_(digitCount), 2);
//^^GRP handling

//          if(digitCount != -1 && digitCount != exponent && !GROUPLEFT_DISABLED && modulo(digitCount, (uint16_t)GROUPWIDTH_LEFT) == (uint16_t)GROUPWIDTH_LEFT - 1) {
  //          xcopy(displayString + charIndex, SEPARATOR_LEFT, 2);
            charIndex += 2;
          }

          // Significant digit or zero
          if(digitPointer <= lastDigit) {
            displayString[charIndex++] = '0' + bcd[digitPointer];
            if(updateDisplayValueX) {
              displayValueX[valueIndex++] = '0' + bcd[digitPointer];
            }
          }
          else {
            displayString[charIndex++] = '0';
            if(updateDisplayValueX) {
              displayValueX[valueIndex++] = '0';
            }
          }

          // Radix mark
          if(digitCount == 0) {
            displayString[charIndex] = 0;
            char tt[4];
            if(RADIX34_MARK_STRING[1]!=1) {strcpy(tt,RADIX34_MARK_STRING);}
            else {tt[0] = RADIX34_MARK_STRING[0]; tt[1] = 0;}
            strcat(displayString, tt);
            charIndex += strlen(tt);
            if(updateDisplayValueX) {
              displayValueX[valueIndex++] = '.';
            }
          }
        }
      }

      displayString[charIndex] = 0;
      if(updateDisplayValueX) {
        displayValueX[valueIndex] = 0;
      }
      return;
    }
  }

  //////////////
  // FIX mode //
  //////////////
#undef SIG_VARIABLE_JUMP

  if((displayFormat == DF_FIX || displayFormat == DF_SF || flag2To10_baseunit_integer)) {                        //DF_UN starts here, to override displaying 2^10 values of between 1000 and 1024 as ENG notation
    if(noFix || exponent >= (checkHP ? 10+1 : displayHasNDigits) ||
         #if defined(SIG_VARIABLE_JUMP)
           exponent < -(int32_t)displayFormatDigits ||                                           //allow zero digits .00...1 to track n
         #else
           ((displayFormat == DF_SF) && (exponent < (getSystemFlag(FLAG_ENGOVR) ? -2 : -3))) ||  //in SIG & ENGOVR,  allow 2 zero digits .001 then jump, in SIG & !ENGOVR, allow 3 zero digits .0001 then jump
           ((displayFormat != DF_SF) && (exponent < -(int32_t)(displayFormatDigits))) ||
         #endif //SIG_VARIABLE_JUMP
         ( displayFormat == DF_SF && exponent -(int32_t)displayFormatDigits < -(checkHP ? 10+1 : displayHasNDigits)) ||
         ( displayFormat == DF_SF && !checkHP && exponent -(int32_t)displayFormatDigits > GROUPWIDTH_LEFT1)
      ) { // Display in SCI or ENG format
      digitsToDisplay = min(displayFormatDigits, (checkHP ? 10+1 : displayHasNDigits) - 1);
      digitToRound    = min(firstDigit + digitsToDisplay, lastDigit);
      ovrSCI = !getSystemFlag(FLAG_ENGOVR);
      ovrENG = getSystemFlag(FLAG_ENGOVR);
    }
    else { // display fix number of digits without ten exponent factor
      // Number of digits to truncate

      uint8_t displayFormatDigits_Active;
      if(displayFormat == DF_SF) {
        displayFormatDigits_Active =  min(max(((int16_t)displayFormatDigits+1)-exponent-1,0),255); //Convert SIG to FIX.
      }
      else {
        displayFormatDigits_Active = displayFormatDigits;
      }
      //printf(">>>## displayFormatDigits_Active=%u displayFormatDigits=%u numDigits=%u exponent=%i\n",displayFormatDigits_Active,displayFormatDigits, numDigits, exponent);
      digitsToTruncate = max(numDigits - (int16_t)displayFormatDigits_Active - exponent - 1, 0);   //SIGFIGNEW hackpoint
      numDigits -= digitsToTruncate;
      lastDigit -= digitsToTruncate;

      if(displayFormat == DF_SF && firstDigit + displayFormatDigits <= 34) {
        digitToRound = firstDigit + displayFormatDigits;
      }
      else {
        digitToRound = lastDigit;
      }
      //printf(">>> ###A %d %d %d %d %d |",numDigits, firstDigit, lastDigit, digitToRound, exponent); for(i=firstDigit; i<=lastDigit; i++) {printf("%c",48+bcd[i]);} printf("\n");

      // Round the displayed number
      if(bcd[digitToRound+1] >= 5) {
        bcd[digitToRound]++;
      }

      // Transfer the carry
      while(bcd[digitToRound] == 10) {
        bcd[digitToRound--] = 0;
        if(displayFormat == DF_SF) {
          numDigits--;
        }
        bcd[digitToRound]++;
      }

      if(displayFormat == DF_SF) {
        lastDigit = digitToRound;
      }

      // Case when 9.9999 rounds to 10.0000
      if(digitToRound < firstDigit) {
        firstDigit--;
        lastDigit = firstDigit;
        numDigits = 1;
        if(displayFormat == DF_SF) displayFormatDigits_Active--;
        exponent++;
      }


      //JM SIGFIG - blank out non-sig digits to the right                 //JM SIGFIGNEW vv
      if(displayFormat == DF_SF) {
        if((displayFormatDigits+1)-exponent-1 < 0) {
           for(digitCount = firstDigit + (displayFormatDigits+1); digitCount <= 34; digitCount++) {
            bcd[digitCount] = 0;
           }
        }
      }                                                                   //JM SIGFIG

      // The sign
      if(sign) {
        displayString[charIndex++] = '-';
        if(updateDisplayValueX) {
          displayValueX[valueIndex++] = '-';
        }
      }
      else {
        if(frontSpace) {
          displayString[charIndex++] = ' ';
        }
      }

      if(exponent < 0) { // negative exponent
        // first 0 and radix mark
        displayString[charIndex++] = '0';
        if(updateDisplayValueX) {
          displayValueX[valueIndex++] = '0';
        }
        displayString[charIndex] = 0;
        char tt[4];
        if(RADIX34_MARK_STRING[1]!=1) {strcpy(tt,RADIX34_MARK_STRING);}
        else {tt[0] = RADIX34_MARK_STRING[0]; tt[1] = 0;}
        strcat(displayString, tt);
        charIndex += strlen(tt);
        if(updateDisplayValueX) {
          displayValueX[valueIndex++] = '.';
        }
//WHY is THIS . and displayValueX
        // Zeros before first significant digit
        for(digitCount=0, i=exponent+1; i<0; i++, digitCount--) {
          if(digitCount!=0 && !GROUPRIGHT_DISABLED && digitCount%(uint16_t)GROUPWIDTH_RIGHT==0) {
            xcopy(displayString + charIndex, SEPARATOR_RIGHT,  SEPARATOR_RIGHT[0]!=1 ? (SEPARATOR_RIGHT[1]!=1 ? 2 : 1) : 0);
            charIndex +=  ( SEPARATOR_RIGHT[0]!=1 ? (SEPARATOR_RIGHT[1]!=1 ? 2 : 1) : 0);
          }
          displayString[charIndex++] = '0';
          if(updateDisplayValueX) {
            displayValueX[valueIndex++] = '0';
          }
        }

        // Significant digits
        for(digitPointer=firstDigit; digitPointer<=lastDigit; digitPointer++, digitCount--) {
          if(digitCount!=0 && !GROUPRIGHT_DISABLED && digitCount%(uint16_t)GROUPWIDTH_RIGHT==0) {
            xcopy(displayString + charIndex, SEPARATOR_RIGHT,  SEPARATOR_RIGHT[0]!=1 ? (SEPARATOR_RIGHT[1]!=1 ? 2 : 1) : 0);
            charIndex +=  ( SEPARATOR_RIGHT[0]!=1 ? (SEPARATOR_RIGHT[1]!=1 ? 2 : 1) : 0);
          }
          displayString[charIndex++] = '0' + bcd[digitPointer];
          if(updateDisplayValueX) {
            displayValueX[valueIndex++] = '0' + bcd[digitPointer];
          }
        }

        // Zeros after last significant digit
        for(i=1; i<=(int16_t)displayFormatDigits_Active+exponent+1-numDigits; i++, digitCount--) {   //JM SIGFIGNEW hackpoint
          if(!GROUPRIGHT_DISABLED && digitCount%(uint16_t)GROUPWIDTH_RIGHT==0) {
            xcopy(displayString + charIndex, SEPARATOR_RIGHT,  SEPARATOR_RIGHT[0]!=1 ? (SEPARATOR_RIGHT[1]!=1 ? 2 : 1) : 0);
            charIndex +=  ( SEPARATOR_RIGHT[0]!=1 ? (SEPARATOR_RIGHT[1]!=1 ? 2 : 1) : 0);
          }
          displayString[charIndex++] = '0';
          if(updateDisplayValueX) {
            displayValueX[valueIndex++] = '0';
          }
        }
      }
      else { // zero or positive exponent
  //JM SIGFIGNEW hackpoint
        for(digitCount=exponent, digitPointer=firstDigit; digitPointer<=firstDigit + exponent + (int16_t)displayFormatDigits_Active; digitPointer++, digitCount--) { // This line is for FIX n displaying more than 16 digits. e.g. in FIX 15: 123 456.789 123 456 789 123
        //for(digitCount=exponent, digitPointer=firstDigit; digitPointer<=firstDigit + min(exponent + (int16_t)displayFormatDigits, 15); digitPointer++, digitCount--) { // This line is for fixed number of displayed digits, e.g. in FIX 15: 123 456.789 123 456 8

//vvGRP handling
          //printf(">>>> digitCount=(%2i)digitPointer=(%2i) bcd[digitPointer]=%2u GROUP1_OVFL=%2i GROUPWIDTH_LEFT1=%2u: %i ?? %i :",digitCount, digitPointer, bcd[digitPointer], GROUP1_OVFL(digitCount, exponent), GROUPWIDTH_LEFT1, bcd[digitPointer-1] , GROUP1_OVFL(digitCount, exponent) + 1);
          if(digitCount!=-1 && digitCount!=exponent && GROUPWIDTH_(digitCount)!=0
                            && IS_SEPARATOR_(digitCount)
                            && (GROUP1_OVFL(digitCount, exponent)==0 || bcd[digitPointer-1] >= GROUP1_OVFL(digitCount, exponent) + 1)   ) {
            //printf("GROUPWIDTH_=%2i digitCountNEW=%2i IS_SEPARATOR_=%2i \n",GROUPWIDTH_(digitCount), digitCountNEW(digitCount), IS_SEPARATOR_(digitCount) );
            xcopy(displayString + charIndex, SEPARATOR_(digitCount), 2);
//^^GRP handling

            charIndex += 2;
          }
          //else printf("\n");


          // Significant digit or zero
          if(digitPointer <= lastDigit) {
            displayString[charIndex++] = '0' + bcd[digitPointer];
            if(updateDisplayValueX) {
              displayValueX[valueIndex++] = '0' + bcd[digitPointer];
            }
          }
          else {
            displayString[charIndex++] = '0';
            if(updateDisplayValueX) {
              displayValueX[valueIndex++] = '0';
            }
          }

          // Radix mark
          if(digitCount == 0) {
            displayString[charIndex] = 0;
            char tt[4];
            if(RADIX34_MARK_STRING[1]!=1) {strcpy(tt,RADIX34_MARK_STRING);}
            else {tt[0] = RADIX34_MARK_STRING[0]; tt[1] = 0;}
            strcat(displayString, tt);
            charIndex += strlen(tt);
            if(updateDisplayValueX) {
              displayValueX[valueIndex++] = '.';
            }
          }
        }
      }

      displayString[charIndex] = 0;
      if(updateDisplayValueX) {
        displayValueX[valueIndex] = 0;
      }
      return;
    }
  }

  //////////////
  // SCI mode //
  //////////////
  if(ovrSCI  || displayFormat == DF_SCI) {
    // Round the displayed number
    if(!ovrSCI) {
      digitsToDisplay = displayFormatDigits;
      digitToRound    = min(firstDigit + (int16_t)displayFormatDigits, lastDigit);
    }
    if(bcd[digitToRound + 1] >= 5) {
      bcd[digitToRound]++;
    }

    // Transfert the carry
    while(bcd[digitToRound] == 10) {
      bcd[digitToRound--] = 0;
      numDigits--;
      bcd[digitToRound]++;
    }

    // Case when 9.9999 rounds to 10.0000
    if(digitToRound < firstDigit) {
      firstDigit--;
      numDigits = 1;
      exponent++;
    }

    // Sign
    if(sign) {
      displayString[charIndex++] = '-';
      if(updateDisplayValueX) {
        displayValueX[valueIndex++] = '-';
      }
    }
    else {
      if(frontSpace) {
        displayString[charIndex++] = ' ';
      }
    }

    // First digit
    displayString[charIndex++] = '0' + bcd[firstDigit];
    if(updateDisplayValueX) {
      displayValueX[valueIndex++] = '0' + bcd[firstDigit];
    }

    // Radix mark
    displayString[charIndex] = 0;
    char tt[4];
    if(RADIX34_MARK_STRING[1]!=1) {strcpy(tt,RADIX34_MARK_STRING);}
    else {tt[0] = RADIX34_MARK_STRING[0]; tt[1] = 0;}
    strcat(displayString, tt);
    charIndex += strlen(tt);
    if(updateDisplayValueX) {
      displayValueX[valueIndex++] = '.';
    }

    // Significant digits
    for(digitCount=-1, digitPointer=firstDigit+1; digitPointer<firstDigit+min(numDigits, digitsToDisplay+1); digitPointer++, digitCount--) {
      if(!firstDigitAfterPeriod && !GROUPRIGHT_DISABLED && modulo(digitCount, (uint16_t)GROUPWIDTH_RIGHT) == (uint16_t)GROUPWIDTH_RIGHT - 1) {
        xcopy(displayString + charIndex, SEPARATOR_RIGHT,  SEPARATOR_RIGHT[0]!=1 ? (SEPARATOR_RIGHT[1]!=1 ? 2 : 1) : 0);
        charIndex +=  ( SEPARATOR_RIGHT[0]!=1 ? (SEPARATOR_RIGHT[1]!=1 ? 2 : 1) : 0);
      }
      else {
        firstDigitAfterPeriod = false;
      }

      displayString[charIndex++] = '0' + bcd[digitPointer];
      if(updateDisplayValueX) {
        displayValueX[valueIndex++] = '0' + bcd[digitPointer];
      }
    }

    // The ending zeros
    for(digitPointer=0; digitPointer<=digitsToDisplay-numDigits; digitPointer++, digitCount--) {
      if(!firstDigitAfterPeriod && !GROUPRIGHT_DISABLED && modulo(digitCount, (uint16_t)GROUPWIDTH_RIGHT) == (uint16_t)GROUPWIDTH_RIGHT - 1) {
        xcopy(displayString + charIndex, SEPARATOR_RIGHT,  SEPARATOR_RIGHT[0]!=1 ? (SEPARATOR_RIGHT[1]!=1 ? 2 : 1) : 0);
        charIndex +=  ( SEPARATOR_RIGHT[0]!=1 ? (SEPARATOR_RIGHT[1]!=1 ? 2 : 1) : 0);
      }
      else {
        firstDigitAfterPeriod = false;
      }

      displayString[charIndex++] = '0';
      if(updateDisplayValueX) {
        displayValueX[valueIndex++] = '0';
      }
    }

    displayString[charIndex] = 0;
    if(updateDisplayValueX) {
      displayValueX[valueIndex] = 0;
    }

    if(exponent != 0) {
      if(updateDisplayValueX) {
        exponentToDisplayString(exponent, displayString + charIndex, displayValueX + valueIndex, false);
      }
      else {
        exponentToDisplayString(exponent, displayString + charIndex, NULL,                       false);
      }
    }
    return;
  }

  //////////////
  // ENG mode //
  //////////////
  if(ovrENG || displayFormat == DF_ENG || displayFormat == DF_UN) {
    // Round the displayed number
    if(!ovrENG) {
      digitsToDisplay = displayFormatDigits;
      digitToRound    = min(firstDigit + digitsToDisplay, lastDigit);
    }

    if(bcd[digitToRound + 1] >= 5) {
      bcd[digitToRound]++;
    }

    // Ensure rounding before the radix mark for DSP 0 & DSP 1
    bcd[digitToRound + 1] = 0;
    bcd[digitToRound + 2] = 0;

    // Transfert the carry
    while(bcd[digitToRound] == 10) {
      bcd[digitToRound--] = 0;
      numDigits--;
      bcd[digitToRound]++;
    }

    // Case when 9.9999 rounds to 10.0000
    if(digitToRound < firstDigit) {
      firstDigit--;
      numDigits = 1;
      exponent++;
    }

    // The sign
    if(sign) {
      displayString[charIndex++] = '-';
      if(updateDisplayValueX) {
        displayValueX[valueIndex++] = '-';
      }
    }
    else {
      if(frontSpace){
        displayString[charIndex++] = ' ';
      }
    }

    // Digits before radix mark
    displayString[charIndex++] = '0' + bcd[firstDigit];
    if(updateDisplayValueX) {
      displayValueX[valueIndex++] = '0' + bcd[firstDigit];
    }
    firstDigit++;
    numDigits--;
    digitsToDisplay--;
    while(modulo(exponent, 3) != 0) {
      exponent--;
      displayString[charIndex++] = '0' + bcd[firstDigit];
      if(updateDisplayValueX) {
        displayValueX[valueIndex++] = '0' + bcd[firstDigit];
      }
      firstDigit++;
      numDigits--;
      digitsToDisplay--;
    }

//The digits are not in groups of three, as the underlying ENG display needs no more than 3 digits left of the radix.
//Therefore 1024^ & UN scheme is limited to 1024^5 --> ^6, if larger numbers are required, separators must be added here.

    if(flag2To10 && displayFormat == DF_UN) {
      while(exponent != exponentUNlimit * 3) {
        //printf("%i \n",exponent);
        if(exponent > exponentUNlimit * 3) exponent--; else
        if(exponent < exponentUNlimit * 3) exponent++;
        displayString[charIndex++] = '0' + bcd[firstDigit];
        if(updateDisplayValueX) {
          displayValueX[valueIndex++] = '0' + bcd[firstDigit];
        }
        firstDigit++;
        numDigits--;
        digitsToDisplay--;
      }
    }


    // Radix Mark
    displayString[charIndex] = 0;
    char tt[4];
    if(RADIX34_MARK_STRING[1]!=1) {strcpy(tt,RADIX34_MARK_STRING);}
    else {tt[0] = RADIX34_MARK_STRING[0]; tt[1] = 0;}
    strcat(displayString, tt);
    charIndex += strlen(tt);
    if(updateDisplayValueX) {
      displayValueX[valueIndex++] = '.';
    }

    // Digits after radix mark
    for(digitCount=-1, digitPointer=firstDigit; digitPointer<firstDigit+min(numDigits, digitsToDisplay+1); digitPointer++, digitCount--) {
      if(!firstDigitAfterPeriod && !GROUPRIGHT_DISABLED && modulo(digitCount, (uint16_t)GROUPWIDTH_RIGHT) == (uint16_t)GROUPWIDTH_RIGHT - 1) {
        xcopy(displayString + charIndex, SEPARATOR_RIGHT,  SEPARATOR_RIGHT[0]!=1 ? (SEPARATOR_RIGHT[1]!=1 ? 2 : 1) : 0);
        charIndex +=  ( SEPARATOR_RIGHT[0]!=1 ? (SEPARATOR_RIGHT[1]!=1 ? 2 : 1) : 0);
      }
      else {
        firstDigitAfterPeriod = false;
      }

      displayString[charIndex++] = '0' + bcd[digitPointer];
      if(updateDisplayValueX) {
        displayValueX[valueIndex++] = '0' + bcd[digitPointer];
      }
    }

    // The ending zeros
    for(digitPointer=0; digitPointer<=digitsToDisplay-max(0, numDigits); digitPointer++, digitCount--) {
      if(!firstDigitAfterPeriod && !GROUPRIGHT_DISABLED && modulo(digitCount, (uint16_t)GROUPWIDTH_RIGHT) == (uint16_t)GROUPWIDTH_RIGHT - 1) {
        xcopy(displayString + charIndex, SEPARATOR_RIGHT,  SEPARATOR_RIGHT[0]!=1 ? (SEPARATOR_RIGHT[1]!=1 ? 2 : 1) : 0);
        charIndex +=  ( SEPARATOR_RIGHT[0]!=1 ? (SEPARATOR_RIGHT[1]!=1 ? 2 : 1) : 0);
      }
      else {
        firstDigitAfterPeriod = false;
      }

      displayString[charIndex++] = '0';
      if(updateDisplayValueX) {
        displayValueX[valueIndex++] = '0';
      }
    }

    displayString[charIndex] = 0;
    if(updateDisplayValueX) {
      displayValueX[valueIndex] = 0;
    }

    if(exponent != 0) {
      if(displayFormat != DF_UN) {                                                        //JM UNIT
        if(updateDisplayValueX) {    //DF_ENG
          exponentToDisplayString(exponent, displayString + charIndex, displayValueX + valueIndex, false);
        }
        else {
          exponentToDisplayString(exponent, displayString + charIndex, NULL,                       false);
        }
      }                                                                                 //JM UNIT
      else {  //DF_UN                                                                   //JM UNIT
        exponentToUnitDisplayString(exponent, flag2To10, displayString + charIndex, displayValueX + valueIndex, false);          //JM UNIT
      }                                                                                 //JM UNIT
    }

  }
  if(flag2To10 && displayFormat == DF_UN) {
    real34Copy(&real34bak, real34);
  }
}


void complex34ToDisplayString(const complex34_t *complex34, char *displayString, const font_t *font, int16_t maxWidth, int16_t displayHasNDigits, bool_t limitExponent, bool_t frontSpace, irfracOption_t limitIrfrac, const uint16_t tagAngle, const bool_t tagPolar) {
  uint8_t savedDisplayFormatDigits = displayFormatDigits;
  uint8_t saveddisplayFormat       = displayFormat;

  int16_t digitWidth = stringWidth("0", font, false, false);

  if(updateDisplayValueX) {
    displayValueX[0] = 0;
  }

  complex34ToDisplayString2(complex34, displayString, displayHasNDigits, limitExponent, frontSpace, tagAngle, tagPolar, limitIrfrac);
  bool noFix = false;
  // bool overflown = false;
  int16_t overflow = stringWidth(displayString, font, true, true) - maxWidth;
  while(overflow > 0) {
    // overflown = true;
    int16_t overflowDigits = max(overflow / digitWidth / 4, 1);

    //complex34ToDisplayString2(complex34, displayString, displayHasNDigits, limitExponent, frontSpace, tagAngle, tagPolar);
    //printf("#### Xw=%i displayHasNDigits=%u  displayFormatDigits=%u str:%s\n",stringWidth(displayString, font, true, true),displayHasNDigits,displayFormatDigits,displayString);

    if(displayHasNDigits == 2) {
      break;
    }

    if(displayFormat == DF_ALL) {
      displayHasNDigits = max(displayHasNDigits - overflowDigits, 2);
    }
    else {
      if (displayFormat == DF_FIX) {
        if(displayFormatDigits == 0 || noFix) {
          noFix = true;
          displayHasNDigits = max(displayHasNDigits - overflowDigits, 2);
          displayFormatDigits = min(displayHasNDigits - 1, savedDisplayFormatDigits);
        }
        else {
          displayFormatDigits = max(displayFormatDigits - overflowDigits, 0);
        }
      }
      else {
        displayFormatDigits = max(displayFormatDigits - overflowDigits, 3);
        if(displayFormatDigits == 3) {
          displayFormat = DF_ALL;
        }
      }
    }

    if(updateDisplayValueX) {
      displayValueX[0] = 0;
    }

    complex34ToDisplayString2(complex34, displayString, displayHasNDigits, limitExponent, frontSpace, tagAngle, tagPolar, limitIrfrac);
    overflow = stringWidth(displayString, font, true, true) - maxWidth;
  }
  // if (overflown && overflow < -3 * digitWidth) {
  //   printf("oops: %d\n", overflow);
  // }

  displayFormatDigits = savedDisplayFormatDigits;
  displayFormat       = saveddisplayFormat;
}


void strPrepend(char*dest, char*prefix) {
  int16_t ii =stringByteLength(prefix);
  if(ii == 0 || ii > 20) {                  //refuse to work with abnormal prefix lengths
    return;
  }
  while(ii > 0) {
    strcat(dest," ");
    ii--;
  }
  int16_t jj = stringByteLength(dest)-1;
  ii =stringByteLength(prefix);
  while(jj-ii >= 0) {
    dest[jj] = dest[jj-ii];
    jj--;
  }
  ii--;
  while(ii >= 0) {
  dest[ii] = prefix[ii];
  ii--;
  }
}



#if defined(PC_BUILD_TELLTALE)
  static void printTempDisplayString(char *displayString, char *displayString2) {
  printf("Real:");
  int gg = 0;
  while(gg<10){
    if((uint8_t)(displayString[gg] == 0)) break;
    printf("§%s§%c %u\n",displayString, (uint8_t)(displayString[gg]), (uint8_t)(displayString[gg]));
    gg++;
  }
  printf("\nImag:");
   gg = 0;
  while(gg<10){
    if((uint8_t)(displayString2[gg] == 0)) break;
    printf("§%s§%c %u\n",displayString2, (uint8_t)(displayString2[gg]), (uint8_t)(displayString2[gg]));
    gg++;
  }
  printf("\n");
  }
#endif //PC_BUILD_TELLTALE

static void complex34ToDisplayString2(const complex34_t *complex34, char *displayString, int16_t displayHasNDigits, bool_t limitExponent, bool_t frontSpace, const uint16_t tagAngle, const bool_t tagPolar, irfracOption_t limitIrfrac) {
  int16_t imagOffset = 100;
  real34_t real34, imag34, absimag34;
  real_t real, imagIc;

  if(tagPolar) { // polar mode
    real34ToReal(VARIABLE_REAL34_DATA(complex34), &real);
    real34ToReal(VARIABLE_IMAG34_DATA(complex34), &imagIc);

    decContext c = ctxtReal39;
    int maxExponent = max(real.exponent + real.digits, imagIc.exponent + imagIc.digits);
    c.digits = (SHOWMODE ? 39 : min(75,max(0,maxExponent) + NUMBER_OF_DISPLAY_REAL_CONTEXT_DIGITS + 2)); //add 2 guard digits for Taylor etc.
    realRectangularToPolar(&real, &imagIc, &real, &imagIc, &c); // imagIc in radian
    c.digits = (SHOWMODE ? 39 : 3 + NUMBER_OF_DISPLAY_REAL_CONTEXT_DIGITS); //converting from radians to grad is the worst, i.e. x 2E2 / pi, which requires 3 digits accuarcy more
    convertAngleFromTo(&imagIc, amRadian, tagAngle == amNone ? currentAngularMode : tagAngle, &c);

    realToReal34(&real, &real34);
    realToReal34(&imagIc, &imag34);
  }
  else { // rectangular mode
    real34Copy(VARIABLE_REAL34_DATA(complex34), &real34);
    real34Copy(VARIABLE_IMAG34_DATA(complex34), &imag34);
  }

  real34ToDisplayString2(&real34, displayString, displayHasNDigits, limitExponent, false, frontSpace, isComplex, limitIrfrac);
  if(updateDisplayValueX) {
    if(tagPolar) {
      strcat(displayValueX, "j");
    }
    else {
      strcat(displayValueX, "i");
    }
  }

  real34ToDisplayString2(&imag34, displayString + imagOffset, displayHasNDigits, limitExponent, false, !FRONTSPACE, isComplex, limitIrfrac);

  #if defined(PC_BUILD_TELLTALE)
    printTempDisplayString(displayString, displayString + imagOffset);
    printStringToConsole(displayString,"Real$$$: ","\n");
  #endif //PC_BUILD_TELLTALE


  if(!real34IsZero(&real34) && strncmp(displayString + imagOffset, STD_ALMOST_EQUAL, 2) == 0) { //if real is not zero, and almost equal char in front of IM part, transfer it to the Left (Real) side
    displayString[imagOffset] = STD_NOCHAR;    //0x01 is the new 'no char' character
    displayString[imagOffset + 1] = STD_NOCHAR;  //0x01 is the new 'no char' character
    if(strncmp(displayString, STD_ALMOST_EQUAL, 2) != 0) {
      strPrepend(displayString,STD_ALMOST_EQUAL);
    }
  }

  #if defined(PC_BUILD_TELLTALE)
    printTempDisplayString(displayString, displayString + imagOffset);
    printStringToConsole(displayString,"Imag$$$: ","\n");
  #endif //PC_BUILD_TELLTALE


  if(tagPolar) { // polar mode
    strcat(displayString, STD_SPACE_4_PER_EM STD_MEASURED_ANGLE STD_SPACE_4_PER_EM);
    uint16_t kk = stringByteLength(displayString);
    angle34ToDisplayString2(&imag34, tagAngle == amNone ? currentAngularMode : tagAngle, displayString + kk, displayHasNDigits, limitExponent, false, limitIrfrac);
    if(strncmp(displayString + kk, STD_ALMOST_EQUAL, 2) == 0) {          //if almost equal char in front of IM part, transfer it to the Left (Real) side
      displayString[kk] = STD_NOCHAR;    //0x01 is the new 'no char' character
      displayString[kk+1] = STD_NOCHAR;  //0x01 is the new 'no char' character
    }
  }
  else { // rectangular mode
    if(strncmp(displayString + stringByteLength(displayString) - 2, STD_SPACE_HAIR, 2) != 0) {
      strcat(displayString, STD_SPACE_HAIR);
    }

    if(real34IsZero(&real34)) {           // JM
      #if defined(PC_BUILD_TELLTALE)
        char tmp_a[100];
        char tmp_b[100];
        stringToASCII(displayString + imagOffset,tmp_a);
        stringToASCII(displayString,tmp_b);
        printf("AA10 %i Real is zero: §%s§%s§\n", imagOffset + stringByteLength(displayString + imagOffset) - 1, tmp_b, tmp_a);
      #endif //PC_BUILD_TELLTALE
      displayString[0]=0;                 // force a zero real not to display the real part
    }
    else {                                // JM normal full display of the full imag part, + and - shown
      int ii = imagOffset;
      bool_t imagNegative = false;
      //determine if second term is negative
      while(ii < imagOffset + min(4,stringByteLength(displayString + imagOffset))) {    //scan first 4 chars, covering two glyphs

        #if defined(PC_BUILD_TELLTALE)
          char tmp_a[100];
          char tmp_b[100];
          stringToASCII(displayString + imagOffset,tmp_a);
          stringToASCII(displayString,tmp_b);
          printf("AA1: %i/%i Real is non-zero: §%s§%s§ %c § %i §\n", ii, imagOffset + stringByteLength(displayString + imagOffset) - 1, tmp_b, tmp_a, (uint8_t)(displayString[ii]), (uint8_t)(displayString[ii]));
        #endif //PC_BUILD_TELLTALE

        if((displayString[ii] >= '0' && displayString[ii] <= '9')) { // if digit is reached, it is not negative
          break;
        }
        if(displayString[ii]=='-') {
          displayString[ii] = STD_NOCHAR; // blank no space char in beginning of imag
          strcat(displayString, "-");     // add the - to trail the real value
          imagNegative = true;
          break;
        }
        ii = imagOffset + stringNextGlyph(displayString + imagOffset, ii - imagOffset);
      }

      if(!imagNegative) {               //there is no '-', then add a '+' trailing real
        strcat(displayString, "+");
      }
    }

    if(getSystemFlag(FLAG_CPXMULT)) {                  // i x 1.0
      strcat(displayString, COMPLEX_UNIT);
      real34CopyAbs(&imag34, &absimag34);
//      if(!real34CompareEqual(&absimag34, const34_1)) {     //JM force a |imag|=1 not to display. Maybe make it part of IRFRAC.
        strcat(displayString, PRODUCT_SIGN);
        xcopy(strchr(displayString, '\0'), displayString + imagOffset, strlen(displayString + imagOffset) + 1);
//      }
    }

    if(!getSystemFlag(FLAG_CPXMULT)) {                   // 1.0 i
      real34CopyAbs(&imag34, &absimag34);
//      if(!real34CompareEqual(&absimag34, const34_1)) {     //JM force a |imag|=1 not to display.  Maybe make it part of IRFRAC.
        xcopy(strchr(displayString, '\0'), displayString + imagOffset, strlen(displayString + imagOffset) + 1);
//      }
      strcat(displayString, STD_SPACE_HAIR);
      strcat(displayString, STD_SPACE_HAIR);
      strcat(displayString, COMPLEX_UNIT);
    }
  }
}


void _numerator(uint64_t numer, char *displayString, int16_t *endingZero) {
  // Numerator
  int16_t  u, insertAt, gap;
  insertAt = *endingZero;
  gap = -1;
  do {
    gap++;
    if(gap == GROUPWIDTH_LEFT) {
      gap = 0;
      (*endingZero)++;
      xcopy(displayString + insertAt + 2, displayString + insertAt, (*endingZero)++ - insertAt);
      displayString[insertAt]     = SEPARATOR_FRAC[0];
      displayString[insertAt + 1] = SEPARATOR_FRAC[1];
    }

    u = numer % 10;
    numer /= 10;
    (*endingZero)++;
    xcopy(displayString + insertAt + 2, displayString + insertAt, (*endingZero)++ - insertAt);

    displayString[insertAt]     = STD_SUP_0[0];
    displayString[insertAt + 1] = STD_SUP_0[1];
    displayString[insertAt + 1] += u;
  } while(numer != 0);
}


void _denominator(uint64_t denom, char *displayString, int16_t *endingZero) {
  // Denominator
  int16_t  u, insertAt, gap;
  insertAt = *endingZero;
  gap = -1;
  do {
    gap++;
    if(gap == GROUPWIDTH_LEFT) {
      gap = 0;
      (*endingZero)++;
      xcopy(displayString + insertAt + 2, displayString + insertAt, (*endingZero)++ - insertAt);
      displayString[insertAt]     = SEPARATOR_FRAC[0];
      displayString[insertAt + 1] = SEPARATOR_FRAC[1];
    }

    u = denom % 10;
    denom /= 10;
    (*endingZero)++;
    xcopy(displayString + insertAt + 2, displayString + insertAt, (*endingZero)++ - insertAt);
    displayString[insertAt]     = STD_SUB_0[0];
    displayString[insertAt + 1] = STD_SUB_0[1];
    displayString[insertAt + 1] += u;
  } while(denom != 0);
}



/********************************************//**
 * \brief formats a fraction
 *
 * \param regist calcRegister_t
 * \param displayString char*
 * \return void
 ***********************************************/
void fractionToDisplayString(calcRegister_t regist, char *displayString) {
  int16_t  sign, lessEqualGreater;
  uint64_t intPart, numer, denom;
  int16_t  u, insertAt, endingZero, gap;

  //printf("regist = "); printRegisterToConsole(regist); printf("\n");
  bool_t prependFraction = fraction(regist, &sign, &intPart, &numer, &denom, &lessEqualGreater);

  //printf("result of fraction(...) = %c%" PRIu64 " %" PRIu64 "/%" PRIu64 "\n", sign==-1 ? '-' : ' ', intPart, numer, denom);
  //printf("  prependFraction=%u fractionDigits=%u\n",prependFraction, fractionDigits);


  // Comparison sign
  if(getSystemFlag(FLAG_FRCSRN)) {
    if(lessEqualGreater == -1) {
      sprintf(displayString, "%c" STD_SPACE_PUNCTUATION ">" STD_SPACE_PUNCTUATION, "xyzt"[regist - REGISTER_X]);
    }
    else if(lessEqualGreater ==  0) {
      sprintf(displayString, "%c" STD_SPACE_PUNCTUATION "=" STD_SPACE_PUNCTUATION, "xyzt"[regist - REGISTER_X]);
    }
    else if(lessEqualGreater ==  1) {
      sprintf(displayString, "%c" STD_SPACE_PUNCTUATION "<" STD_SPACE_PUNCTUATION, "xyzt"[regist - REGISTER_X]);
    }
    else {
      strcpy(displayString, "?" STD_SPACE_PUNCTUATION);
      sprintf(errorMessage, commonBugScreenMessages[bugMsgValueFor], "fractionToDisplayString", lessEqualGreater, "lessEqualGreater");
      displayBugScreen(errorMessage);
    }
  }
  else if(prependFraction) {
    if(lessEqualGreater == -1) {
      sprintf(displayString, ">" STD_SPACE_PUNCTUATION);
    }
    else if(lessEqualGreater ==  0) {
      displayString[0] = 0;
    }
    else if(lessEqualGreater ==  1) {
      sprintf(displayString, "<" STD_SPACE_PUNCTUATION);
    }
    else {
      strcpy(displayString, "?" STD_SPACE_PUNCTUATION);
      sprintf(errorMessage, commonBugScreenMessages[bugMsgValueFor], "fractionToDisplayString", lessEqualGreater, "lessEqualGreater");
      displayBugScreen(errorMessage);
    }
  }
  else {
    displayString[0] = 0;
  }


  endingZero = strlen(displayString);

  if(getSystemFlag(FLAG_PROPFR) && intPart != 0) { // a b/c
    if(updateDisplayValueX) {
      sprintf(displayValueX, "%s%" PRIu32 " %" PRIu32 "/%" PRIu32, (sign == -1 ? "-" : ""), (uint32_t)intPart, (uint32_t)numer, (uint32_t)denom);
    }

    if(sign == -1) {
      strcat(displayString, "-");
      endingZero++;
    }

    // Integer part
    insertAt = endingZero;
    gap = -1;
    do {
      gap++;
      if(gap == GROUPWIDTH_LEFT) {
        gap = 0;
        endingZero++;
        xcopy(displayString + insertAt + 2, displayString + insertAt, endingZero++ - insertAt);
        displayString[insertAt]     = SEPARATOR_LEFT[0];
        displayString[insertAt + 1] = SEPARATOR_LEFT[1];
      }

      u = intPart % 10;
      intPart /= 10;
      endingZero++;
      xcopy(displayString + insertAt + 1, displayString + insertAt, endingZero - insertAt);
      displayString[insertAt] = '0' + u;
    } while(intPart != 0);

    strcat(displayString, STD_SPACE_PUNCTUATION);
    endingZero += 2;
  }

  else { // FT_IMPROPER d/c
    if(updateDisplayValueX) {
      sprintf(displayValueX, "%s%" PRIu32 "/%" PRIu32, (sign == -1 ? "-" : ""), (uint32_t)numer, (uint32_t)denom);
    }

    if(sign == -1) {
      strcat(displayString, STD_SUP_MINUS);
      endingZero += 2;
    }
  }


  _numerator(numer, displayString, &endingZero);

  // Fraction bar
  strcat(displayString, "/");
  endingZero++;

  _denominator(denom, displayString, &endingZero);
}


void angle34ToDisplayString2(const real34_t *angle34, uint8_t mode, char *displayString, int16_t displayHasNDigits, bool_t limitExponent, bool_t frontSpace, irfracOption_t limitIrfrac) {
  if(mode == amDMS) {
    char degStr[100];
    uint32_t m, s, fs;
    int16_t sign;
    real34_t angle34Dms;
    real_t angleDms, degrees, minutes, seconds;

    real34FromDegToDms(angle34, &angle34Dms);
    real34ToReal(&angle34Dms, &angleDms);

    sign = realIsNegative(&angleDms);
    realSetPositiveSign(&angleDms);

    // Get the degrees
    realToIntegralValue(&angleDms, &degrees, DEC_ROUND_DOWN, &ctxtReal39);

    // Get the minutes
    realSubtract(&angleDms, &degrees, &angleDms, &ctxtReal39);
    angleDms.exponent += 2; // angleDms = angleDms * 100
    realToIntegralValue(&angleDms, &minutes, DEC_ROUND_DOWN, &ctxtReal39);

    // Get the seconds
    realSubtract(&angleDms, &minutes, &angleDms, &ctxtReal39);
    angleDms.exponent += 2; // angleDms = angleDms * 100
    realToIntegralValue(&angleDms, &seconds, DEC_ROUND_DOWN, &ctxtReal39);

    // Get the fractional seconds
    realSubtract(&angleDms, &seconds, &angleDms, &ctxtReal39);
    angleDms.exponent += 2; // angleDms = angleDms * 100

    fs = realToUint32C47(&angleDms);
    s  = realToUint32C47(&seconds);
    m  = realToUint32C47(&minutes);

    if(fs >= 100) {
      fs -= 100;
      s++;
    }

    if(s >= 60) {
      s -= 60;
      m++;
    }

    if(m >= 60) {
      m -= 60;
      realAdd(&degrees, const_1, &degrees, &ctxtReal39);
    }

    //Change to make proper real number before the °
    real34_t tmp;
    realToReal34(&degrees, &tmp);
    uint8_t savedDisplayFormatDigits = displayFormatDigits;
    uint8_t savedDisplayFormat       = displayFormat;
    //format without decimals
    displayFormatDigits = 0;
    displayFormat = DF_ALL;
    real34ToDisplayString2(&tmp, degStr, displayHasNDigits, limitExponent, false, frontSpace, true, limitIrfrac);
    displayFormatDigits = savedDisplayFormatDigits;
    displayFormat       = savedDisplayFormat;
    //remove the '.' radix indicating it is a real
    int32_t slen = (int32_t)strlen(degStr);
    int32_t mlen = (Rx[0] & 0x80) ? (int32_t)strlen(RADIX34_MARK_STRING) : (int32_t)strlen(Rx);
    const char *marker = (Rx[0] & 0x80) ? RADIX34_MARK_STRING : Rx;
    if(slen >= mlen && strcmp(degStr + slen - mlen, marker) == 0) {
      degStr[slen - mlen] = '\0';
    }


    char tt[4];
    if(RADIX34_MARK_STRING[1]!=1) {strcpy(tt,RADIX34_MARK_STRING);}
    else {tt[0] = RADIX34_MARK_STRING[0]; tt[1] = 0;}

    sprintf(displayString, "%s%s" STD_DEGREE "%s%" PRIu32 STD_RIGHT_SINGLE_QUOTE "%s%" PRIu32 "%s%02" PRIu32 STD_RIGHT_DOUBLE_QUOTE,
                            sign ? "-" : " ",
                              degStr,         m < 10 ? " " : "",
                                                m,                   s < 10 ? " " : "",
                                                                       s,         tt,
                                                                                    fs);
  }
  else if(mode == amMultPi) {
    IrFractionsCurrentStatus = CF_OFF;        //JM
    real34ToDisplayString2(angle34, displayString, displayHasNDigits, limitExponent, mode == amSecond, frontSpace, isReal, limitIrfrac);
    strcat(displayString, STD_SUP_pir);
  }
  else {
    real34ToDisplayString2(angle34, displayString, displayHasNDigits, limitExponent, mode == amSecond, frontSpace, isReal, limitIrfrac);

    if(mode == amRadian) {
      strcat(displayString, STD_SUP_BOLD_r);
    }
    else if(mode == amGrad) {
      strcat(displayString, STD_SUP_BOLD_g);
    }
    else if(mode == amDegree) {
      strcat(displayString, STD_DEGREE);
    }
    else if(mode == amSecond) {
      strcat(displayString, "s");
    }
    else {
      strcat(displayString, "?");
      sprintf(errorMessage, commonBugScreenMessages[bugMsgValueFor], "angle34ToDisplayString2", mode, "mode");
      displayBugScreen(errorMessage);
    }
  }
}



void addBaseNumber(char *displayString, int16_t base) {
  if(base <= 16) {
    strcat(displayString, STD_BASE_2);
    displayString[strlen(displayString) - 1] += base - 2;
  }
  else {
    strcat(displayString, STD_SUB_0);
    displayString[strlen(displayString) - 1] += (base / 10);
    strcat(displayString, STD_SUB_0);
    displayString[strlen(displayString) - 1] += (base % 10);
  }
}


void longIntegerToHexDisplayString(calcRegister_t regist, char *displayString, bool_t determineFont, uint8_t baseOverride, int32_t width) {
  uint16_t i = TMP_STR_LENGTH / 2;
  displayString[0] = 0;
  if(dispBase < 2) {
    return;
  }

  longInteger_t lgInt;
  bool_t sign;
  int32_t digit;

  convertLongIntegerRegisterToLongInteger(regist, lgInt);

  if(longIntegerIsZero(lgInt)) {
    displayString[0] = '0';
    displayString[1] = 0;
    addBaseNumber(displayString, dispBase);
    longIntegerFree(lgInt);
    fontForShortInteger = &numericFont;
    return;
  }

  sign = (lgInt->_mp_size < 0);

  while (!longIntegerIsZero(lgInt)) {
    digit = (int32_t)longIntegerModuloUInt(lgInt, (int32_t)(dispBase));
    longIntegerDivideUInt(lgInt, (int32_t)(dispBase), lgInt);
    displayString[i++] = baseDigits[digit];
  }

  longIntegerFree(lgInt);

  if(sign){
    displayString[i++] = '-';
  }

  for(uint16_t k = i-1, j = 0; k >= TMP_STR_LENGTH / 2; k--, j++) {
    displayString[j] = displayString[k];
    displayString[j+1] = 0;
  }

  if( stringWidth(displayString, &numericFont, false, true) +
      stringWidth(STD_SUB_0 STD_SUB_0, &numericFont, false, true) +
      stringWidth("  X:" STD_INTEGER_Z_SMALL ": ", &standardFont, false, true)
      <= width) {
    fontForShortInteger = &numericFont;
    addBaseNumber(displayString, dispBase);
  }
  else
  if( stringWidth(displayString, &standardFont, false, true) +
      stringWidth(STD_SUB_0 STD_SUB_0, &standardFont, false, true) +
      stringWidth("  X:" STD_INTEGER_Z_SMALL ": ", &standardFont, false, true)
      <= width) {
    fontForShortInteger = &standardFont;
    addBaseNumber(displayString, dispBase);
  }
  else {
    fontForShortInteger = &tinyFont;
    #define lastTinyCharIfTooLong 256
    if( stringWidth(displayString, &tinyFont, true, true) +
        stringWidth(STD_SUB_0 STD_SUB_0, &tinyFont, true, true) //+
        //stringWidth("  X:" STD_INTEGER_Z_SMALL ": ", &tinyFont, false, true)
        >= width * 4) {
      displayString[lastTinyCharIfTooLong + 0] = STD_ELLIPSIS[0];
      displayString[lastTinyCharIfTooLong + 1] = STD_ELLIPSIS[1];
      displayString[lastTinyCharIfTooLong + 2] = 0;
    }
    addBaseNumber(displayString, dispBase);
  }
}


void shortIntegerToDisplayString(calcRegister_t regist, char *displayString, bool_t determineFont, uint8_t baseOverride) {
  int16_t i, j, k, unit, gap, digit, bitsPerDigit, maxDigits, base;
  uint64_t orgnumber, number, sign;

//JM Pre-load X:
char str3[3];
j = 0;
str3[j] = displayString[j]; j++;
str3[j] = displayString[j]; j++;
str3[j] = displayString[j];

  number  = *(REGISTER_SHORT_INTEGER_DATA(regist));
  orgnumber = number;

  bool_t bcdFlag = false;   //JM BCDvv Base 1 is reserved for BCD, 10
  if(baseOverride == 1) {
    base = 10;
    bcdFlag = true;
  }                         ////JMBCD ^^ Base 1 is reserved for BCD, 10
  else if(baseOverride == 0) {
    base = getRegisterTag(regist);
    if(base <= 1 || base >= 17) {
      sprintf(errorMessage, commonBugScreenMessages[bugMsgValueFor], "shortIntegerToDisplayString", base, "base");
      displayBugScreen(errorMessage);
      base = 10;
    }
  }
  else {
    base = baseOverride;
  }


  //number &= shortIntegerMask;

  if(shortIntegerMode == SIM_UNSIGN || base == 2 || base == 4 || base == 8 || base == 16) {
    sign = 0;
  }
  else {
    sign = number & shortIntegerSignBit;
  }

  if(sign) {
    if(shortIntegerMode == SIM_2COMPL) {
      number |= ~shortIntegerMask;
      number = ~number + 1;
    }
    else if(shortIntegerMode == SIM_1COMPL) {
      number = ~number;
    }
    else if(shortIntegerMode == SIM_SIGNMT) {
      number &= ~shortIntegerSignBit;
    }
    else {
      sprintf(errorMessage, commonBugScreenMessages[bugMsgValueFor], "shortIntegerToDisplayString", shortIntegerMode, "shortIntegerMode");
      displayBugScreen(errorMessage);
    }

    number &= shortIntegerMask;
  }

  i = ERROR_MESSAGE_LENGTH / 2;

  if(number == 0) {
    if(base == 10 && bcdFlag) {           //JM BCDvv
      char ss[5];
      if(bcdDisplaySign == BCD9c) {
        strcpy(displayString + i, "1001");
        strcpy(ss,STD_BASE_9);
      }
      else
      if(bcdDisplaySign == BCD10c) {
        strcpy(displayString + i, "0000");
        strcpy(ss,STD_SUB_o);
      }
      else {
        strcpy(displayString + i, "0000");
        strcpy(ss,STD_SUB_o);
      }
      i += 4;
      digit = 1;
      displayString[i++] = ss[1];
      displayString[i++] = ss[0];
    }
    else {                              //JM BCD^^
      displayString[i++] = '0';
      digit = 1;
    }
  }
  else {
    digit = 0;
  }

  if(GROUPLEFT_DISABLED) {
    gap = 0;
  }
  else {
    if(base == 2) {
      gap = 4;
    }
    else if(base == 4 || base == 16) {
      gap = 2;
    }
    else if(base == 8) {
      gap = 3;
    }
    else if(base == 10 && bcdFlag) {
      gap = 1;
    }
    else if(base <= 16) {
      gap = 3;
    }
    else { //for large base display, use 0-9, a-z, A-Z.
      gap = 4;
    }
  }

  uint8_t firstNonZero = 0;
  while(number) {
    if(gap != 0 && digit != 0 && digit%gap == 0) {
      displayString[i++] = ' ';
    }
    digit++;

    unit = number % base;
    number /= base;
    if(unit != 0) {firstNonZero++;}

    if(bcdFlag && base == 10) {                //JM BCDVV conversion - Note base 17 is code for BCD display of base 10
      if(bcdDisplaySign == BCD9c) {
        unit = 9 - unit;
      }
      else
      if(bcdDisplaySign == BCD10c) {           //see https://madformath.com/calculators/digital-systems/complement-calculators/10-s-complement-calculator-alternative/10-s-complement-calculator-alternative
        if(firstNonZero == 0) {
          unit = 0;
        }
        else
        if(firstNonZero == 1) {
          unit = 9 - unit + 1;
          firstNonZero++;
        }
        else {
          unit = 9 - unit;
          firstNonZero++;
        }
      }

      displayString[i++] = '0' + ((unit & 0x0001) );
      displayString[i++] = '0' + ((unit & 0x0002) >> 1);
      displayString[i++] = '0' + ((unit & 0x0004) >> 2);
      displayString[i++] = '0' + ((unit & 0x0008) >> 3);

      if((orgnumber & 0x0FFFFFFFFFFFFFFF) <= 99999999999) {    //JM BCD add decimal to each BCD nibble
        char ss[5];
        if(unit == 0) {
          strcpy(ss,STD_SUB_o);
          displayString[i++] = ss[1];
          displayString[i++] = ss[0];
        }
        else {
          strcpy(ss,STD_BASE_1);
          displayString[i++] = ss[1] + unit - 1;
          displayString[i++] = ss[0];
        }
      }
    }
    else {                                   //JM BCD^^
      displayString[i++] = baseDigits[unit];
    }                                          //JM

  }

  // Add leading zeros
  if(getSystemFlag(FLAG_LEAD0)) {
    if(base ==  2) {
      bitsPerDigit = 1;
    }
    else if(base ==  4) {
      bitsPerDigit = 2;
    }
    else if(base ==  8) {
      bitsPerDigit = 3;
    }
    else if(base == 16) {
      bitsPerDigit = 4;
    }
    else {
     bitsPerDigit = 0;
    }

    if(bitsPerDigit != 0) {
      maxDigits = shortIntegerWordSize / bitsPerDigit;
      if(shortIntegerWordSize % bitsPerDigit) {
        maxDigits++;
      }

      while(digit < maxDigits) {
        if(gap != 0 && digit%gap == 0) {
          displayString[i++] = ' ';
        }
        digit++;

        displayString[i++] = '0';
      }
    }
  }

  if(sign) {
    displayString[i++] = '-';
  }

//JM SHOW //ONLY ADD REGISTER NAME IF IT IS A LETTERED REGISTER - NO SPACE FOR MORE
if( str3[0] >= 'A' && str3[0] <= 'Z' && str3[1] == ':' && str3[2] == ' ' && !(base == 2 && orgnumber > 0x3FFF))
{
  displayString[i++] = str3[2];
  displayString[i++] = str3[1];
  displayString[i++] = str3[0];
}
if( str3[1] >= '0' && str3[1] <= '9' && str3[2] >= '0' && str3[2] <= '9' && str3[0] == 'R' && !(base == 2 && orgnumber > 0x3FFF))
{
  displayString[i++] = ':';
  displayString[i++] = str3[2];
  displayString[i++] = str3[1];
}

  if(determineFont) { // The font is not yet determined
    // 1st try: numeric font digits from 30 to 39
    fontForShortInteger = &numericFont;

    for(k=i-1, j=0; k>=ERROR_MESSAGE_LENGTH / 2; k--, j++) {
      if(displayString[k] == ' ') {
        displayString[j++] = STD_SPACE_PUNCTUATION[0];
        displayString[j]   = STD_SPACE_PUNCTUATION[1];
      }
      else {
        displayString[j] = displayString[k];
      }
    }
    displayString[j] = 0;

    if(bcdFlag && base == 10) {                              //JM BCD id display instead of base 10
      strcat(displayString, STD_SUB_b STD_SUB_c STD_SUB_d);
    }
    else addBaseNumber(displayString, base);

    if(stringWidth(displayString, fontForShortInteger, false, false) < SCREEN_WIDTH) {
      return;
    }

    // 2nd try: numeric font digits from 2487 to 2490
    for(k=i-1, j=0; k>=ERROR_MESSAGE_LENGTH / 2; k--, j++) {
      if(displayString[k] == ' ') {
        displayString[j++] = STD_SPACE_PUNCTUATION[0];
        displayString[j]   = STD_SPACE_PUNCTUATION[1];
      }
      else if(0x30 <= displayString[k] && displayString[k] <= 0x39) {
        displayString[j++] = NUM_0_b[0];
        displayString[j]   = NUM_0_b[1] - '0' + displayString[k];
      }
      else {
        displayString[j] = displayString[k];
      }
    }
    displayString[j] = 0;

    if(bcdFlag && base == 10) {                             //JM BCD id display instead of base 10
      strcat(displayString, STD_SUB_b STD_SUB_c STD_SUB_d);
    }
    else addBaseNumber(displayString, base);


    if(stringWidth(displayString, fontForShortInteger, false, false) < SCREEN_WIDTH) {
      return;
    }

    // 3rd try: standard font digits from 30 to 39
    fontForShortInteger = &standardFont;

    for(k=i-1, j=0; k>=ERROR_MESSAGE_LENGTH / 2; k--, j++) {
      if(displayString[k] == ' ') {
        displayString[j++] = STD_SPACE_PUNCTUATION[0];
        displayString[j]   = STD_SPACE_PUNCTUATION[1];
      }
      else {
       displayString[j] = displayString[k];
      }
    }
    displayString[j] = 0;

    if(bcdFlag && base == 10) {                             //JM BCD id display instead of base 10
      strcat(displayString, STD_SUB_b STD_SUB_c STD_SUB_d);
    }
    else addBaseNumber(displayString, base);

    if(/*temporaryInformation == TI_SHOW_REGISTER_BIG ||*/ stringWidth(displayString, fontForShortInteger, false, false) < SCREEN_WIDTH) {     //JMSHOW
      return;
    }

    // 4th and last try: standard font digits 220e and 2064 (binary)
    for(k=i-1, j=0; k>=ERROR_MESSAGE_LENGTH / 2; k--, j++) {
      if(displayString[k] == ' ') {
        displayString[j++] = STD_SPACE_PUNCTUATION[0];
        displayString[j]   = STD_SPACE_PUNCTUATION[1];
      }
      else if(displayString[k] == '0') {
        displayString[j++] = STD_BINARY_ZERO[0];
        displayString[j]   = STD_BINARY_ZERO[1];
      }
      else if(displayString[k] == '1') {
        displayString[j++] = STD_BINARY_ONE[0];
        displayString[j]   = STD_BINARY_ONE[1];
      }
      else {
        displayString[j] = displayString[k];
      }
    }
    displayString[j] = 0;

    if(bcdFlag && base == 10) {                             //JM BCD id display instead of base 10
      strcat(displayString, STD_SUB_b STD_SUB_c STD_SUB_d);
    }
    else addBaseNumber(displayString, base);

    if(stringWidth(displayString, fontForShortInteger, false, false) < SCREEN_WIDTH) {
      return;
    }

    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function shortIntegerToDisplayString: the integer data representation is too wide (1)!", displayString, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)

    strcpy(displayString, "Integer data representation too wide!");
  }

  else { // the font is already determined (standard font)
    fontForShortInteger = &standardFont;

    // 1st try: standard font digits from 30 to 39
    for(k=i-1, j=0; k>=ERROR_MESSAGE_LENGTH / 2; k--, j++) {
      if(displayString[k] == ' ') {
        displayString[j++] = STD_SPACE_PUNCTUATION[0];
        displayString[j]   = STD_SPACE_PUNCTUATION[1];
      }
      else {
        displayString[j] = displayString[k];
      }
    }
    displayString[j] = 0;

    if(bcdFlag && base == 10) {                             //JM BCD id display instead of base 10
      strcat(displayString, STD_SUB_b STD_SUB_c STD_SUB_d);
    }
    else addBaseNumber(displayString, base);

    if(stringWidth(displayString, fontForShortInteger, false, false) < SCREEN_WIDTH) {
      return;
    }

    // 2nd and last try: standard font digits 220e and 2064 (binary)
    for(k=i-1, j=0; k>=ERROR_MESSAGE_LENGTH / 2; k--, j++) {
      if(displayString[k] == ' ') {
        displayString[j++] = STD_SPACE_PUNCTUATION[0];
        displayString[j]   = STD_SPACE_PUNCTUATION[1];
      }
      else if(displayString[k] == '0') {
        displayString[j++] = STD_BINARY_ZERO[0];
        displayString[j]   = STD_BINARY_ZERO[1];
      }
      else if(displayString[k] == '1') {
        displayString[j++] = STD_BINARY_ONE[0];
        displayString[j]   = STD_BINARY_ONE[1];
      }
      else {
       displayString[j] = displayString[k];
      }
    }
    displayString[j] = 0;

    if(bcdFlag && base == 10) {                             //JM BCD id display instead of base 10
      strcat(displayString, STD_SUB_b STD_SUB_c STD_SUB_d);
    }
    else addBaseNumber(displayString, base);

    if(stringWidth(displayString, fontForShortInteger, false, false) < SCREEN_WIDTH) {
      return;
    }

    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function shortIntegerToDisplayString: the integer data representation is too wide (2)!", displayString, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)

    strcpy(displayString, "Integer data representation to wide!");
  }
}


void longIntegerRegisterToDisplayString(calcRegister_t regist, char *displayString, int32_t strLg, int16_t max_Width, int16_t maxExp, bool_t allowLARGELI) { //JM mod max_Width;   //JM added last parameter: Allow LARGELI
  longInteger_t lgInt;
  convertLongIntegerRegisterToLongInteger(regist, lgInt);
  longIntegerToDisplayString(lgInt, displayString, strLg, max_Width, maxExp, allowLARGELI);
  longIntegerFree(lgInt);
}



void longIntegerRegisterToRealDisplayString(calcRegister_t regist, char *displayString, int32_t strLg, int16_t maxWidth, int32_t minimum, bool_t removeTrailingRadix) {    //This function depends on real34ToDisplayString2, which depends on the getSystemFlag(FLAG_2TO10) && displayFormat == DF_UN to be set
  longInteger_t lgInt;
  convertLongIntegerRegisterToLongInteger(regist, lgInt);
  longIntegerToAllocatedString(lgInt, displayString, strLg);
  longIntegerFree(lgInt);
  real_t tmp4, tmpReal;
  real34_t tmpReal34;
  stringToReal(displayString, &tmpReal, &ctxtReal39);
  int32ToReal(minimum,&tmp4);
  if(minimum == 0 || !realCompareAbsLessThan(&tmpReal, &tmp4)) {
    realToReal34(&tmpReal, &tmpReal34);
    //real34ToDisplayString2(&tmpReal34, displayString,                            34, 100, false, false, isReal);
    real34ToDisplayString(&tmpReal34, amNone, displayString, getSystemFlag(FLAG_LARGELI) ? &numericFont : &standardFont, maxWidth,  34, LIMITEXP, !FRONTSPACE, NOIRFRAC);


    if(removeTrailingRadix) {
      int lastGlyphPosition = stringLastGlyph(displayString);
      //check the radix. Two options, a single byte or two-byte radix. Delete the radix if at the right edge of the string.
      if(displayString[lastGlyphPosition]==RADIX34_MARK_STRING[0] && (displayString[lastGlyphPosition+1]==RADIX34_MARK_STRING[1] || RADIX34_MARK_STRING[1]==1)) {
        displayString[lastGlyphPosition] = 0;
      }
    }
  }
}


//this procedure is meant to be integrated into the integer parts of Real, Integer, sup & sub text.
//TODO. It needs careful consideration and will save some bytes

static void insertSepsIntoIntegerText(char *displayString){
  //IPGRP IPGRP1 IPGRP1x handling
  if(!GROUPLEFT_DISABLED && GROUPWIDTH_LEFT1 > 0) {
    //Handle IPGRP1, bearing in mind with a negative number, the digits start one deeper
    int16_t len = strlen(displayString);
    //printf("len %u %u %u %u\n",len, displayString[0], displayString[1], displayString[2]);
    int16_t digitOne = displayString[0] == '-' ? 1 : 0;
    int16_t Grp1 = ( (GROUPWIDTH_LEFT1X > 0)
                  && (displayString[digitOne] - 48 <= GROUPWIDTH_LEFT1X)
                  && (len - digitOne == GROUPWIDTH_LEFT1 + 1) ? GROUPWIDTH_LEFT1 + 1 : GROUPWIDTH_LEFT1);
    Grp1 = len >= (checkHP ? 10 : 20) ? GROUPWIDTH_LEFT : Grp1;
    int16_t i = len - Grp1;
    if(i > 0 && (i != 1 || displayString[0] != '-')) {
      if(SEPARATOR_LEFT[1]!=1) {
        xcopy(displayString + i + 2, displayString + i, len - i + 1);
        displayString[i] = SEPARATOR_LEFT[0];
        displayString[i + 1] = SEPARATOR_LEFT[1];
      }
      else if(SEPARATOR_LEFT[0] != 1) {
        xcopy(displayString + i + 1, displayString + i, len - i + 1);
        displayString[i] = SEPARATOR_LEFT[0];
      }

      //Handle repeating IPGRP
      len = strlen(displayString);
      for(i=len - GROUPWIDTH_LEFT - (Grp1 + (SEPARATOR_LEFT[1] == 1 ? 1 : 2)); i>0; i-=GROUPWIDTH_LEFT) {
        if(i != 1 || displayString[0] != '-') {
          if(SEPARATOR_LEFT[1] != 1) {
            xcopy(displayString + i + 2, displayString + i, len - i + 1);
            displayString[i] = SEPARATOR_LEFT[0];
            displayString[i + 1] = SEPARATOR_LEFT[1];
            len += 2;
          }
          else if(SEPARATOR_LEFT[0] != 1) {
            xcopy(displayString + i + 1, displayString + i, len - i + 1);
            displayString[i] = SEPARATOR_LEFT[0];
            len += 1;
          }
        }
      }
    }
  }
}

void longIntegerToDisplayString(longInteger_t lgInt, char *displayString, int32_t strLg, int16_t max_Width, int16_t maxExp, bool_t allowLARGELI) { //JM mod max_Width;   //JM added last parameter: Allow LARGELI
  int16_t exponentStep,exponentStep1;
  uint32_t exponentShift, exponentShiftLimit;
  int16_t maxWidth;                                         //JM align longints

  if(longIntegerIsNegative(lgInt)) {maxWidth = max_Width;}  //JM align longints
  else {maxWidth = max_Width - 8;}                          //JM align longints

  exponentShift = (longIntegerBits(lgInt) - 1) * 0.3010299956639811952137;
  exponentStep = (GROUPWIDTH_LEFT  == 0 || (SEPARATOR_LEFT[0]==1 && SEPARATOR_LEFT[1]==1) ? 1 : GROUPWIDTH_LEFT);   //TOCHECK
  exponentStep1= (                         (SEPARATOR_LEFT[0]==1 && SEPARATOR_LEFT[1]==1) ? 1 : GROUPWIDTH_LEFT1);  //TOCHECK

  exponentShift = (exponentShift / exponentStep  + 1) * exponentStep;
  exponentShiftLimit = ((maxExp-exponentStep1) / exponentStep + 1) * exponentStep;
  //printf("exponentShift=%i exponentShiftLimit=%i\n",exponentShift,exponentShiftLimit);
  if(exponentShift > exponentShiftLimit) {
    exponentShift -= exponentShiftLimit;

    // Why do the following lines not work (for a big exponentShift) instead of the for loop below ?
    //longIntegerInitSizeInBits(divisor, longIntegerBits(lgInt));
    //longIntegerPowerUIntUInt(10u, exponentShift, divisor);
    //longIntegerDivide(lgInt, divisor, lgInt);
    //longIntegerFree(divisor);
    for(int32_t i=(int32_t)exponentShift; i>=1; i--) {
      if(i >= 9) {
        longIntegerDivideUInt(lgInt, 1000000000, lgInt);
        i -= 8;
      }
      else if(i >= 4) {
        longIntegerDivideUInt(lgInt,      10000, lgInt);
        i -= 3;
      }
      else {
        longIntegerDivideUInt(lgInt,         10, lgInt);
      }
    }
  }
  else {
    exponentShift = 0;
  }

  //printf("B exponentShift=%i exponentShiftLimit=%i\n",exponentShift,exponentShiftLimit);
  longIntegerToAllocatedString(lgInt, displayString, strLg);


  if(updateDisplayValueX) {
    strcpy(displayValueX, displayString);
  }

  insertSepsIntoIntegerText(displayString);

  //for any exponent display, further manipulation of GRP is not needed
  if(stringWidth(displayString, allowLARGELI && getSystemFlag(FLAG_LARGELI) ? &numericFont : &standardFont, false, false) > maxWidth) {      //JM
    char exponentString[14], lastRemovedDigit;
    int16_t lastChar, stringStep, tenExponent;

    stringStep = (GROUPLEFT_DISABLED ? 1 : GROUPWIDTH_LEFT + (SEPARATOR_LEFT[1] == 1 ? 1 : 2));
    tenExponent = exponentStep + exponentShift;
    lastChar = strlen(displayString) - stringStep;
    lastRemovedDigit = displayString[lastChar + (SEPARATOR_LEFT[1] == 1 ? 1 : 2)];
    displayString[lastChar] = 0;
    if(updateDisplayValueX) {
      displayValueX[strlen(displayValueX) - max(GROUPWIDTH_LEFT, 1)] = 0;
    }
    exponentString[0] = 0;
    exponentToDisplayString(tenExponent, exponentString, NULL, false);
    while(stringWidth(displayString,   allowLARGELI && getSystemFlag(FLAG_LARGELI) ? &numericFont : &standardFont, false, true) + stringWidth(exponentString,   allowLARGELI && getSystemFlag(FLAG_LARGELI) ? &numericFont : &standardFont, true, false) > maxWidth) {  //JM getSystemFlag(FLAG_LARGELI)
      lastChar -= stringStep;
      tenExponent += exponentStep;
      lastRemovedDigit = displayString[lastChar + (SEPARATOR_LEFT[1] == 1 ? 1 : 2)];
      displayString[lastChar] = 0;
      if(updateDisplayValueX) {
        displayValueX[strlen(displayValueX) - max(GROUPWIDTH_LEFT, 1)] = 0;
      }
      exponentString[0] = 0;
      exponentToDisplayString(tenExponent, exponentString, NULL, false);
    }

    if(lastRemovedDigit >= '5') { // Round up
      lastChar = strlen(displayString) - 1;
      displayString[lastChar]++;
      while(displayString[lastChar] > '9') {
        displayString[lastChar--] = '0';
        while(lastChar >= 0 && (displayString[lastChar] < '0' || displayString[lastChar] > '9')) {
          lastChar--;
        }
        if(lastChar >= 0) {
          displayString[lastChar]++;
        }
        else { // We are rounding up from 9999... to 10000...
          lastChar = (displayString[0] == '-' ? 1 : 0);
          xcopy(displayString + lastChar + 1, displayString + lastChar, strlen(displayString) + 1);
          displayString[lastChar] = '1';
          if(SEPARATOR_LEFT[1] != 1) {
            if(!GROUPLEFT_DISABLED && displayString[lastChar + GROUPWIDTH_LEFT + 2] == SEPARATOR_LEFT[1]) { // We need to insert a new goup SEPARATOR_LEFT
              xcopy(displayString + lastChar + 3, displayString + lastChar + 1, strlen(displayString));
              displayString[lastChar + 1] = SEPARATOR_LEFT[0];
              displayString[lastChar + 2] = SEPARATOR_LEFT[1];
            }
          }
          else if(SEPARATOR_LEFT[0] !=1 ){
            if(!GROUPLEFT_DISABLED && displayString[lastChar + GROUPWIDTH_LEFT + 1] == SEPARATOR_LEFT[0]) { // We need to insert a new goup SEPARATOR_LEFT
              xcopy(displayString + lastChar + 2, displayString + lastChar + 1, strlen(displayString));
              displayString[lastChar + 1] = SEPARATOR_LEFT[0];
            }
          }

          // Has the string become too long?
          if(stringWidth(displayString,   allowLARGELI && getSystemFlag(FLAG_LARGELI) ? &numericFont : &standardFont, false, true) + stringWidth(exponentString,   allowLARGELI && getSystemFlag(FLAG_LARGELI) ? &numericFont : &standardFont, true, false) > maxWidth) {   //JM getSystemFlag(FLAG_LARGELI)
            lastChar = strlen(displayString) - stringStep;
            tenExponent += exponentStep;
            displayString[lastChar] = 0;
            if(updateDisplayValueX) {
              displayValueX[strlen(displayValueX) - max(GROUPWIDTH_LEFT, 1)] = 0;
            }
            exponentString[0] = 0;
            exponentToDisplayString(tenExponent, exponentString, NULL, false);
          }
        }
      }

      if(updateDisplayValueX) {
        lastChar = strlen(displayValueX) - 1;
        displayValueX[lastChar]++;
        while(lastChar>0 && '0' <= displayValueX[lastChar - 1] && displayValueX[lastChar - 1] <= '9' && displayValueX[lastChar] > '9') {
          displayValueX[lastChar--] = '0';
          displayValueX[lastChar]++;
        }

        if(displayValueX[lastChar] > '9') { // We are rounding 9999... to 10000...
          xcopy(displayValueX + 1, displayValueX, strlen(displayValueX) + 1);
          displayValueX[lastChar++] = '1';
          displayValueX[lastChar] = '0';
        }
      }
    }

    strcat(displayString, exponentString);

    if(updateDisplayValueX) {
      sprintf(displayValueX + strlen(displayValueX), "e%d", tenExponent);
    }
  }
}


void longIntegerToAllocatedString(const longInteger_t lgInt, char *str, int32_t strLen) {
  int32_t numberOfDigits, stringLen, counter;
  longInteger_t x;

  str[0] = '0';
  str[1] = 0;
  if(lgInt->_mp_size == 0) {
    return;
  }

  //numberOfDigits = longIntegerBase10Digits(lgInt); // GMP documentation says the result can be 1 to big
  numberOfDigits = mpz_sizeinbase(lgInt, 10); // GMP documentation says the result can be 1 to big
  if(lgInt->_mp_size < 0) {
    stringLen = numberOfDigits + 2; // 1 for the trailing 0 and 1 for the minus sign
    str[0] = '-';
  }
  else {
    stringLen = numberOfDigits + 1; // 1 for the trailing 0
  }

  if(strLen < stringLen) {
    sprintf(errorMessage, "In function longIntegerToAllocatedString: the string str (%" PRId32 " bytes) is too small to hold the base 10 representation of lgInt, %" PRId32 " are needed!", strLen, stringLen);
    displayBugScreen(errorMessage);
    return;
  }

  str[stringLen - 1] = 0;

  //longIntegerInitSizeInBits(x, longIntegerBits(lgInt));
  mpz_init2(x, mpz_sizeinbase(lgInt, 2));

  //longIntegerAddUInt(lgInt, 0, x);
  mpz_add_ui(x, lgInt, 0);

  //longIntegerSetPositiveSign(x);
  x->_mp_size =  abs(x->_mp_size);


  stringLen -= 2; // set stringLen to the last digit of the base 10 representation
  counter = numberOfDigits;
  //while(!longIntegerIsZero(x)) {
  while(x->_mp_size != 0) {
    str[stringLen--] = '0' + mpz_tdiv_ui(x, 10);

    //longIntegerDivideUInt(x, 10, x);
    mpz_tdiv_q_ui(x, x, 10);

    counter--;
  }

  if(counter == 1) { // digit was 1 too big
    xcopy(str + stringLen, str + stringLen + 1, numberOfDigits);
  }

  //longIntegerFree(x);
  mpz_clear(x);
}


void dateToDisplayString(calcRegister_t regist, char *displayString) {
  real34_t j, y, yy, m, d;
  uint64_t yearval64;
  char sign[] = {0, 0};

  internalDateToJulianDay(REGISTER_REAL34_DATA(regist), &j);
  decomposeJulianDay(&j, &y, &m, &d);
  if(real34IsNegative(&y)) {
    sign[0] = '-';
  }
  real34CopyAbs(&y, &y);
  real34CopyAbs(&y, &yy);
  real34DivideRemainder(&y, const34_2p32, &y);
  real34Divide(&yy, const34_2p32, &yy);
  real34ToIntegralValue(&yy, &yy, DEC_ROUND_DOWN);
  yearval64 = (((uint64_t)real34ToUInt32(&yy) << 32) | ((uint64_t)real34ToUInt32(&y)));

  if(getSystemFlag(FLAG_DMY)) {
    sprintf(displayString, "%02" PRIu32 ".%02" PRIu32 ".%s%04" PRIu32, real34ToUInt32(&d), real34ToUInt32(&m), sign, (uint32_t)yearval64);
  }
  else if(getSystemFlag(FLAG_MDY)) {
    sprintf(displayString, "%02" PRIu32 "/%02" PRIu32 "/%s%04" PRIu32, real34ToUInt32(&m), real34ToUInt32(&d), sign, (uint32_t)yearval64);
  }
  else { // YMD
    sprintf(displayString, "%s%04" PRIu32 "-%02" PRIu32 "-%02" PRIu32, sign, (uint32_t)yearval64, real34ToUInt32(&m), real34ToUInt32(&d));
  }
}


void timeToDisplayString(calcRegister_t regist, char *displayString, bool_t ignoreTDisp) {
  real_t real, value, tmp, h, m, s;
  longInteger_t hli;
  int32_t sign, i;
  uint32_t digits, tDigits = 0u, bDigits, m32, s32;
  char digitBuf[16], digitBuf2[48];
  char* bufPtr;
  bool_t isValid12hTime = false, isAfternoon = false;
  uint8_t savedDisplayFormat = displayFormat, savedDisplayFormatDigits = displayFormatDigits;

  real34ToReal(REGISTER_REAL34_DATA(regist), &real);
  sign = realIsNegative(&real);

  // Short time (displayed like SCI/ENG)
  if(timeDisplayFormatDigits == 0) {
    realDivide(const_1, const_1000, &value, &ctxtReal39);
  }
  else if((timeDisplayFormatDigits == 1) || (timeDisplayFormatDigits == 2)) {
    realCopy(const_60, &value);
  }
  else {
    realCopy(const_1, &value);
    for(i = 3; i < timeDisplayFormatDigits; ++i) {
      --value.exponent;
      if(i == 5) {
        break;
      }
    }
  }
  if(realCompareAbsLessThan(&real, const_1)) {
    realCopy(const_1, &tmp), tmp.exponent -= 33;
    realDivideRemainder(&real, &tmp, &tmp, &ctxtReal39);
  }
  else {
    realZero(&tmp);
  }
  if(realCompareAbsLessThan(&real, &value) || (ignoreTDisp && (!realIsZero(&tmp)))) {
    if(ignoreTDisp || (timeDisplayFormatDigits == 0)) {
      displayFormat = DF_ALL;
      displayFormatDigits = 0;
    }
    else {
      displayFormat = getSystemFlag(FLAG_ENGOVR) ? DF_ENG : DF_SCI;
      displayFormatDigits = 3;
    }
    real34ToDisplayString(REGISTER_REAL34_DATA(regist), amSecond, displayString, &standardFont, 2000, ignoreTDisp ? 34 : 16, !LIMITEXP, !FRONTSPACE, NOIRFRAC);
    displayFormatDigits = savedDisplayFormatDigits;
    displayFormat = savedDisplayFormat;
    return;
  }
  displayFormatDigits = savedDisplayFormatDigits;
  displayFormat = savedDisplayFormat;

  // Hours
  realDivide(&real, const_3600, &h, &ctxtReal39);
  realSetPositiveSign(&h);
  realToIntegralValue(&h, &h, DEC_ROUND_DOWN, &ctxtReal39);

  // Pre-rounding
  if(!ignoreTDisp) {
    switch(timeDisplayFormatDigits) {
      case 0: {
        int32ToReal(86400, &value);
        if((!sign) && (!getSystemFlag(FLAG_TDM24)) && realCompareLessThan(&real, &value)) {
          isValid12hTime = true;
        }
        for(bDigits = 0; bDigits < (isValid12hTime ? 14 : 16); ++bDigits) {
          if(realCompareAbsLessThan(&h, const_100)) {
            break;
          }
          ++value.exponent;
        }
        tDigits = isValid12hTime ? 13 : 15;
        isValid12hTime = false;
        goto do_rounding;
      }
      case 1:
      case 2: { // round to minutes
        realDivide(&real, const_60, &real, &ctxtReal39);
        realToIntegralValue(&real, &real, DEC_ROUND_DOWN, &ctxtReal39);
        realMultiply(&real, const_60, &real, &ctxtReal39);
        break;
      }
      default: { // round to seconds, milliseconds, microseconds, ...
        tDigits = timeDisplayFormatDigits + 1;
        bDigits = 4u;
      do_rounding:
        for(digits = bDigits; digits < tDigits; ++digits) {
          ++real.exponent;
        }
        realToIntegralValue(&real, &real, timeDisplayFormatDigits == 0 ? DEC_ROUND_HALF_UP : DEC_ROUND_DOWN, &ctxtReal39);
        for(digits = bDigits; digits < tDigits; ++digits) {
          --real.exponent;
        }
        tDigits = 0u;
      }
    }
  }
  realSetPositiveSign(&real);

  // Seconds
  realToIntegralValue(&real, &s, DEC_ROUND_DOWN, &ctxtReal39);
  realSubtract(&real, &s, &real, &ctxtReal39); // Fractional part
  // Minutes
  realDivide(&s, const_60, &m, &ctxtReal39);
  realToIntegralValue(&m, &m, DEC_ROUND_DOWN, &ctxtReal39);
  realDivideRemainder(&s, const_60, &s, &ctxtReal39);
  realDivideRemainder(&m, const_60, &m, &ctxtReal39);
  // 12-hour time
  if((!getSystemFlag(FLAG_TDM24)) && (!sign)) {
    int32ToReal(24, &value);
    if(realCompareLessThan(&h, &value)) {
      isValid12hTime = true;
      int32ToReal(12, &value);
      if(realCompareGreaterEqual(&h, &value)) {
        isAfternoon = true;
        if(!realCompareLessEqual(&h, &value)) {
          realSubtract(&h, &value, &h, &ctxtReal39);
        }
      }
      else if(realIsZero(&h)) {
        realAdd(&h, &value, &h, &ctxtReal39);
      }
    }
  }

  // Display Hours
  strcpy(displayString, sign ? "-" : " ");
  longIntegerInit(hli);
  convertRealToLongInteger(&h, hli, DEC_ROUND_DOWN);
  longIntegerToAllocatedString(hli, digitBuf2, sizeof(digitBuf2));
  longIntegerFree(hli);

  bufPtr = digitBuf2;
  digitBuf[1] = '\0';
  for(digits = strlen(digitBuf2); digits > 0; --digits){
    digitBuf[0] = *bufPtr++;
    strcat(displayString, digitBuf);
    if((digits % GROUPWIDTH_LEFT == 1) && (digits > 1)) {
      char tt[4]; tt[0]=0;
      if(SEPARATOR_LEFT[1]!=1) {
        strcpy(tt,SEPARATOR_LEFT);
      }
      else if(SEPARATOR_LEFT[0]!=1) {
        tt[0] = SEPARATOR_LEFT[0];
        tt[1] = 0;
      }
      strcat(displayString, tt);
    }
    ++tDigits;
  }

  if((!ignoreTDisp) && (timeDisplayFormatDigits == 1 || timeDisplayFormatDigits == 2 || (++tDigits) > (isValid12hTime ? 16 : 18))) {
    // Display Minutes
    m32 = realToUint32C47(&m);
    sprintf(digitBuf, ":%02" PRIu32, m32);
    strcat(displayString, digitBuf);
  }

  else {
    // Display MM:SS
    m32 = realToUint32C47(&m);
    s32 = realToUint32C47(&s);
    sprintf(digitBuf, ":%02" PRIu32 ":%02" PRIu32, m32, s32);
    strcat(displayString, digitBuf);

    // Display fractional part of seconds
    digits = 0u;
    realZero(&value);
    while(1) {
      realSubtract(&real, &value, &real, &ctxtReal39);
      if(ignoreTDisp || (timeDisplayFormatDigits == 0)) {
        if(realIsZero(&real)) {
          break;
        }
      }
      else {
        if(digits + 4 > timeDisplayFormatDigits) {
          break;
        }
      }
      if((!ignoreTDisp) && ((++tDigits) > (isValid12hTime ? 16 : 18))) {
        break;
      }
      real.exponent++; // real = real * 10
      realToIntegralValue(&real, &value, DEC_ROUND_DOWN, &ctxtReal39);

      if(digits == 0u) {
        char tt[4];
        if(RADIX34_MARK_STRING[1]!=1) {strcpy(tt,RADIX34_MARK_STRING);}
        else {tt[0] = RADIX34_MARK_STRING[0]; tt[1] = 0;}
        strcat(displayString, tt);
      }
      else if(digits % GROUPWIDTH_RIGHT == 0u) {
        char tt[4]; tt[0]=0;
        if(SEPARATOR_RIGHT[1]!=1) {strcpy(tt,SEPARATOR_RIGHT);}
        else if(SEPARATOR_RIGHT[0]!=1) {tt[0] = SEPARATOR_RIGHT[0]; tt[1] = 0;}
        strcat(displayString, tt);
      }

      s32 = realToUint32C47(&value);
      sprintf(digitBuf, "%" PRIu32, s32);
      strcat(displayString, digitBuf);
      ++digits;
    }
  }

  // for 12-hour time
  if(isAfternoon) {
    strcat(displayString, "p.m.");
  }
  else if(isValid12hTime) {
    strcat(displayString, "a.m.");
  }
}


void real34MatrixToDisplayString(calcRegister_t regist, char *displayString) { // [n×n Matrix]
  matrixHeader_t *matrixHeader = REGISTER_MATRIX_HEADER(regist);
  sprintf(displayString, "[%" PRIu16 STD_CROSS "%" PRIu16" Matrix]", matrixHeader->matrixRows, matrixHeader->matrixColumns);
}


void complex34MatrixToDisplayString(calcRegister_t regist, char *displayString) {
  matrixHeader_t* matrixHeader = REGISTER_MATRIX_HEADER(regist);
  sprintf(displayString, "[%" PRIu16 STD_CROSS "%" PRIu16 " " STD_COMPLEX_C " Matrix]", matrixHeader->matrixRows, matrixHeader->matrixColumns);
}


#if !defined(TESTSUITE_BUILD)
bool_t vectorToDisplayString(calcRegister_t regist, char *displayString) {
  if(getRegisterDataType(regist) == dtReal34Matrix) {
    matrixHeader_t *matrixHeader = REGISTER_MATRIX_HEADER(regist);
    if(isMatrixVector(matrixHeader->matrixRows, matrixHeader->matrixColumns)) {
      real34Matrix_t matrix;
      int16_t prefixWidth = 0;
      linkToRealMatrixRegister(regist, &matrix);
      showRealMatrix(&matrix, prefixWidth, !toDisplayVectorMatrix);
      sprintf(displayString, "%s", errorMessage);
      //if(stringWidth(tmpString, &numericFont, true, true) + 1 > SCREEN_WIDTH) return false;     //this is to revert to [4x4 Matrix] if the digits in the default standard font is too wide. Not needed as it is managed by reducing the font
      return true;
    }
  }
  return false;
}


static void _complex34ToShowTmpString(const real34_t *r, const real34_t *i) {
  int16_t last;
  real34_t real34;

  // Real part
  real34ToDisplayString(r, amNone, tmpString, &standardFont, 2000, 34, !LIMITEXP, !FRONTSPACE, NOIRFRAC);

  // +/- i×
  real34Copy(i, &real34);
  last = SHOWLineSize;
  while(tmpString[last]) {
    last++;
  }
  xcopy(tmpString + last++, (real34IsNegative(&real34) ? "-" : "+"), 1);
  xcopy(tmpString + last++, COMPLEX_UNIT, 2);
  xcopy(tmpString + last, PRODUCT_SIGN, 3);

  // Imaginary part
  real34SetPositiveSign(&real34);
  real34ToDisplayString(&real34, amNone, tmpString + 2*SHOWLineSize, &standardFont, 2000, 34, !LIMITEXP, !FRONTSPACE, NOIRFRAC);

  if(stringWidth(tmpString + SHOWLineSize, &standardFont, true, true) + stringWidth(tmpString + 2*SHOWLineSize, &standardFont, true, true) <= SCREEN_WIDTH) {
    last = SHOWLineSize;
    while(tmpString[last]) {
      last++;
    }
    xcopy(tmpString + last, tmpString + 2*SHOWLineSize, strlen(tmpString + 2*SHOWLineSize) + 1);
    tmpString[2*SHOWLineSize] = 0;
  }

  if(stringWidth(tmpString, &standardFont, true, true) + stringWidth(tmpString + SHOWLineSize, &standardFont, true, true) <= SCREEN_WIDTH) {
    last = 0;
    while(tmpString[last]) {
      last++;
    }
    xcopy(tmpString + last, tmpString + SHOWLineSize, strlen(tmpString + SHOWLineSize) + 1);
    xcopy(tmpString + SHOWLineSize,  tmpString + 2*SHOWLineSize, strlen(tmpString + 2*SHOWLineSize) + 1);
    tmpString[2*SHOWLineSize] = 0;
  }
}
#endif //TESTSUITE_BUILD


void mimShowElement(void) {
  #if !defined(TESTSUITE_BUILD)
    uint8_t savedDisplayFormat = displayFormat, savedDisplayFormatDigits = displayFormatDigits;

    int16_t i = getIRegisterAsInt(true);
    int16_t j = getJRegisterAsInt(true);

    displayFormat = DF_ALL;
    displayFormatDigits = 0;

    uint8_t ix;
    for(ix=0; ix<=SHOWLineMax; ix++) { //L1 ... L7
      tmpString[ix*SHOWLineSize]=0;
    }

    temporaryInformation = TI_SHOW_REGISTER;

    if(getRegisterDataType(matrixIndex) == dtReal34Matrix) {
      real34ToDisplayString(&openMatrixMIMPointer.realMatrix.matrixElements[i * openMatrixMIMPointer.header.matrixColumns + j], amNone, tmpString, &standardFont, 2000, 34, !LIMITEXP, !FRONTSPACE, NOIRFRAC);
    }

    else {
      _complex34ToShowTmpString(VARIABLE_REAL34_DATA(&openMatrixMIMPointer.complexMatrix.matrixElements[i * openMatrixMIMPointer.header.matrixColumns + j]),
                                VARIABLE_IMAG34_DATA(&openMatrixMIMPointer.complexMatrix.matrixElements[i * openMatrixMIMPointer.header.matrixColumns + j]));
    }

    displayFormat = savedDisplayFormat;
    displayFormatDigits = savedDisplayFormatDigits;
  #endif
}


#if !defined(TESTSUITE_BUILD)
#if !defined(SAVE_SPACE_DM42_9)

static void RegName(void) {    //JM using standard reg name, using showRegis, not using prefixWidth
  int16_t tmp;
  viewRegName2(tmpString + 2100, &tmp);
  //printf("|%s|%d|\n",tmpString + 2100, 2100+stringByteLength(tmpString + 2100));
}


static void SHOW_reset(void){
  uint8_t ix;
  for(ix=0; ix<=SHOWLineMax; ix++) { //L1 ... L7
    tmpString[ix*SHOWLineSize]=0;
  }

  temporaryInformation = TI_SHOW_REGISTER_SMALL;
  RegName();
}


static void checkAndEat(int16_t *source, int16_t last, int16_t *dest) {
  uint8_t ix;
  if(*source < last && !GROUPLEFT_DISABLED) {                  //Not in the last line
    for(ix=0; ix<=3; ix++) {
      if(!(tmpString[(*dest)-2] & 0x80)) {(*dest)--; (*source)--;} //Eat away characters at the end to line up the last space
    }
    tmpString[*dest] = 0;
  }
}


static void printXSHOW(int16_t am, int16_t d, int16_t df, int16_t dfd, int16_t dt, bool_t tagPolar) {
  displayFormat = df;
  displayFormatDigits = dfd;
  int16_t last, source, dest, ww;
  RegName();
  ww = stringWidth(tmpString + 2100, &numericFont, true, true);

  if(dt == dtReal34) {
    real34_t real34;
    real34Copy(REGISTER_REAL34_DATA(showRegis), &real34);
    convertAngle34FromTo(&real34, getRegisterAngularMode(showRegis), am);
    real34ToDisplayString(&real34, am, tmpString + 2100 + stringByteLength(tmpString + 2100), &numericFont, SCREEN_WIDTH - ww - 8*2, 34, !LIMITEXP, !FRONTSPACE, NOIRFRAC);
  }
  else if(dt == dtComplex34) {
    complex34ToDisplayString(REGISTER_COMPLEX34_DATA(showRegis), tmpString + 2100 + stringByteLength(tmpString + 2100), &numericFont,SCREEN_WIDTH - ww - 8*2, 34 ,!LIMITEXP, !FRONTSPACE, NOIRFRAC, getComplexRegisterAngularMode(showRegis), tagPolar);
  }

  if(stringWidth(tmpString + 2100, &numericFont, true, true) > SCREEN_WIDTH) tmpString[2100] = 0; //clear the line if it comes out too long by some fluke


  last = 2100 + stringByteLength(tmpString + 2100);
  source = 2100;
  dest = d;
  while(source < last && stringWidth(tmpString + d, &numericFont, true, true) <= SCREEN_WIDTH - 8*2) {
    tmpString[dest] = tmpString[source];
    if(tmpString[dest] & 0x80) {
      tmpString[++dest] = tmpString[++source];
    }
    source++;
    tmpString[++dest] = 0;
  }
}


static void dispM(uint16_t regist, char * prefix) {
  uint32_t prefixWidth = 0;
  const int16_t baseY = 20;
  bool_t prefixPre = false;
  bool_t prefixPost = false;
  prefixWidth = stringWidth(prefix, &standardFont, true, true);
  temporaryInformation = TI_SHOWNOTHING;
  if(getRegisterDataType(regist) == dtReal34Matrix) {
    real34Matrix_t matrix;
    linkToRealMatrixRegister(regist, &matrix);
    showRealMatrix(&matrix, prefixWidth,toDisplayVectorMatrix);
    //printf("#### tmpString=%s prefix=%s prefixWidth=%u lastErrorCode=%u temporaryInformation=%u\n",tmpString,prefix,prefixWidth,lastErrorCode, temporaryInformation);
    if(lastErrorCode != 0) {
      refreshRegisterLine(errorMessageRegisterLine);
    }
    if(prefixWidth > 0) {
      showString(prefix, &standardFont, 1, baseY, vmNormal, prefixPre, prefixPost);
    }
    if(temporaryInformation == TI_INACCURATE && regist == REGISTER_X) {
      showString("This result may be inaccurate", &standardFont, 1, Y_POSITION_OF_ERR_LINE, vmNormal, true, true);
    }
  }
  else if(getRegisterDataType(regist) == dtComplex34Matrix) {
    complex34Matrix_t matrix;
    linkToComplexMatrixRegister(regist, &matrix);
    showComplexMatrix(&matrix, prefixWidth, getComplexRegisterAngularMode(regist), getComplexRegisterPolarMode(regist) == amPolar);
    //printf("#### tmpString=%s prefix=%s prefixWidth=%u lastErrorCode=%u temporaryInformation=%u\n",tmpString,prefix,prefixWidth,lastErrorCode, temporaryInformation);
    if(lastErrorCode != 0) {
      refreshRegisterLine(errorMessageRegisterLine);
    }
    if(prefixWidth > 0) {
      showString(prefix, &standardFont, 1, baseY, vmNormal, prefixPre, prefixPost);
    }
    if(temporaryInformation == TI_INACCURATE && regist == REGISTER_X) {
      showString("This result may be inaccurate", &standardFont, 1, Y_POSITION_OF_ERR_LINE, vmNormal, true, true);
    }
  }
}
#endif //SAVE_SPACE_DM42_9



#undef MONITOR_SHOW

static void prepLongintIntoLines(int16_t *last, int16_t *source, int16_t *dest, const font_t *fontToUse, int16_t maxWidth, int16_t numberOfLines, int16_t *startingLine) {
        /* checktmpStringSep macro:           */
        /* previous 2 bytes = double byte sep */
        /* previous byte = single byte sep    */
        /* previous 2 bytes are STD_SPACE_FIGURE used in XFN to compact the display */
#define checktmpStringSep(SEP, a) ( !(   (SEP[1] != 1 && tmpString[a-2] == SEP[0] &&              tmpString[a-1] == SEP[1]             ) \
                                      || (SEP[1] == 1 && tmpString[a-1] == SEP[0]                                                      ) \
                                      || (               tmpString[a-2] == STD_SPACE_FIGURE[0] && tmpString[a-1] == STD_SPACE_FIGURE[1]) \
                                    ))
  char *SEP = SEPARATOR_LEFT;
  int8_t GRPWID = GROUPWIDTH_LEFT;
  bool_t GRP_DISABLED = GROUPLEFT_DISABLED;

  if(strchr(errorMessage, '.') || strstr(errorMessage,RADIX34_MARK_STRING)) {
    //in XFN decimal mode, a decimal point changes the seps to the right hand style
    SEP = SEPARATOR_RIGHT;
    GRPWID = GROUPWIDTH_RIGHT;
    GRP_DISABLED = GROUPRIGHT_DISABLED;
  }
  int16_t Width_0 = stringWidth(SEP, fontToUse, true, true);
  #if defined(MONITOR_SHOW)
    printf("000: source=%d %d %d [%d] %d %d\n", *source, errorMessage[*source-2], errorMessage[*source-1], errorMessage[*source], errorMessage[*source+1], errorMessage[*source+2]);
    printf("Width_0 = %d\n",Width_0);
  #endif //MONITOR_SHOW

  int16_t d;
  *dest = 0;
  int16_t sourceReturn = 0;
  for(d=0; d <= (numberOfLines)*SHOWLineSize ; d+=SHOWLineSize) {   //0 to (n-1)+1 one more that the displayed strings, to detect run-over
    tmpString[d] = 0;
  }
  for(d = (*startingLine)*SHOWLineSize; d <= (*startingLine + (numberOfLines-1+1))*SHOWLineSize; d += SHOWLineSize) { //0 to (n-1)+1 one more that the displayed strings, to detect run-over
    int16_t dCounter = d - (*startingLine)*SHOWLineSize;
    //printf("dCounter=%i d=%i startingLine=%i last=%i source=%i dest=%i ...",dCounter,d,*startingLine,*last,*source,*dest);
    *dest = dCounter;


    while((*source < *last) && (*dest < TMP_STR_LENGTH - 6)) {
      // Add the character(s)
      tmpString[*dest] = errorMessage[*source];
      int bytesToAdd = 1;

      if(tmpString[*dest] & 0x80) {
        tmpString[++*dest] = errorMessage[++*source];
        bytesToAdd = 2;
        #if defined(MONITOR_SHOW)
          printf("(%u)\n",(uint8_t)((tmpString + (*dest))[0]));
        #endif
      }
      tmpString[++*dest] = 0;
      (*source)++;

      // Check width validity
      int16_t currentWidth = stringWidth(tmpString + dCounter, fontToUse, true, true);
      int16_t allowedWidth = maxWidth - (dCounter == 0 ? 0 : Width_0);

      if(currentWidth >= allowedWidth) {
        // Width exceeded - remove added character(s) and break
        *dest -= bytesToAdd;
        *source -= bytesToAdd;
        tmpString[*dest] = 0;
        break;
      }
      // Width is valid

      #if defined(MONITOR_SHOW)
        printf("dCounter = %d, Width_0 =%d, %s : Width=%d <> %d\n", dCounter, Width_0, tmpString + dCounter, currentWidth, allowedWidth);
        printf("02--->d=%i startingLine=%i last=%i source=%i dest=%i wid=%i??maxwid=%i <<:%u ",d,*startingLine,*last,*source,*dest,currentWidth, allowedWidth, *dest < TMP_STR_LENGTH - 6);
        printf("03    ==>%c (%u)",((tmpString + (*dest-1))[0]), (uint8_t)((tmpString + (*dest-1))[0]));
        if(((uint8_t)((tmpString + (*dest-1))[0]) & 0x80) == 0) {printf("\n");}
      #endif
    }


    #if defined(MONITOR_SHOW)
      printf("  --->d=%i wid=%i\n",d,(int16_t)(stringWidth(tmpString + dCounter, fontToUse, true, true)));
    #endif
    uint8_t cnt = GRPWID+1;
    while(cnt-- != 0 && *source < *last && !GRP_DISABLED ) { //Eat away characters at the end to line, up to and excluding the last seperator.
      if(checktmpStringSep(SEP, *dest)) {                    //try the actual sep char, or any other double byte unicode character to split it there. That excludes all ligit digits
        (*dest)--;                                           //line does not end on separator, so reduce the characters until it does
        (*source)--;
      }
      else {
        (*dest)--;    //line ends on a seperator so reduce only the target and let the next line begins onthe number, not separator
        (*source)--;
        if(SEP[0] & 0x80 && SEP[1] != 1) { //line ends on a double byte seperator
          (*dest)--;
          (*source)--;
        }
        break;
      }
    }
    //source sits on the next, not yet printed digit


    #if defined(MONITOR_SHOW)
      printf("AAA: source=%d %d %d [%d] %d %d\n", (uint8_t)*source, (uint8_t)errorMessage[*source-2], (uint8_t)errorMessage[*source-1], (uint8_t)errorMessage[*source], (uint8_t)errorMessage[*source+1], (uint8_t)errorMessage[*source+2]);
      printf("AAA: dest  =%d %d %d [%d] %d %d\n", (uint8_t)*dest  , (uint8_t)tmpString[*dest  -2],    (uint8_t)tmpString[*dest  -1],    (uint8_t)tmpString[*dest  ],    (uint8_t)tmpString[*dest  +1],    (uint8_t)tmpString[*dest  +2]);
      printf("---: d=%d (*startingLine + (numberOfLines-1))*SHOWLineSize=%d\n", d, (*startingLine + (numberOfLines-1))*SHOWLineSize);
    #endif //MONITOR_SHOW

    tmpString[*dest] = 0;
    if(d == (*startingLine + (numberOfLines-1))*SHOWLineSize) sourceReturn = *source; //numberOfLines-1 is the last visible line

    //printf("source=%i dest=%i [..3]=%i %i %i\n",*source,*dest,tmpString[*dest-2],tmpString[*dest-1],tmpString[*dest-0]);
    //printf(">>>AA %u %u |%s|\n", d, (uint8_t)tmpString[d], tmpString+d);
    //printf(">>>BB source=%i last=%i dest=%i\n", *source, *last, *dest);
  }

  *source = sourceReturn;

  #if defined(MONITOR_SHOW)
    printf("###%s###\n",errorMessage + *dest);
    printf("BBB: source=%d %d %d [%d] %d %d\n", (uint8_t)*source, (uint8_t)errorMessage[*source-2], (uint8_t)errorMessage[*source-1], (uint8_t)errorMessage[*source], (uint8_t)errorMessage[*source+1], (uint8_t)errorMessage[*source+2]);
    printf("BBB: dest  =%d %d %d [%d] %d %d\n", (uint8_t)*dest  , (uint8_t)tmpString[*dest  -2],    (uint8_t)tmpString[*dest  -1],    (uint8_t)tmpString[*dest  ],    (uint8_t)tmpString[*dest  +1],    (uint8_t)tmpString[*dest  +2]);
  #endif //MONITOR_SHOW
}

#endif //TESTSUITE_BUILD



void realToSci(real_t* num, char* dispString) {
   char *p, *radix = Rx, *sep = SEPARATOR_RIGHT;
   int neg, exp, mi = 0, i = 1, d = 0;
   int sepGroup = GROUPWIDTH_RIGHT;

   if(realGetExponent(num) > 672 || num->digits > 672 ) { //tighten up the spacing if it gets to a longer string
     sep = STD_SPACE_FIGURE;
   }

    exp = realGetExponent(num);
    realToString(num, dispString + 1500);
    if(realIsZero(num)) {
      sprintf(dispString, "0%s0", radix);
      return;
    }

    neg = ((dispString + 1500)[0] == '-');
    p = (dispString + 1500) + neg;

    while(*p && (*p < '0' || *p > '9')) p++;    // skips to first digit

    if(*p == '0' && *(p+1) == '.') {            // handle "0.ddd..." format
      p += 2;                                    // skip "0."
      while(*p == '0') p++;                      // skip all leading zeros after decimal
    }
    dispString[mi++] = neg ? '-' : ' ';         // inserts - if prior determined
    dispString[mi++] = *p++;                    // copies first digit incr and continue
    if(*p == '.') p++;                          // if 2nd char is . skip it
    if(*p != 'E') {                             // as long as current (2nd/3rd) char is not at the end E meaning 1E, it must have been the 1., so continue to add the proper radix
      dispString[mi++] = radix[0];              // add first half of radix
      if(radix[0] & 0x80 && radix[1] && radix[1] != '\1') {
        dispString[mi++] = radix[1];            // add 2nd half of radix if second half > 1
      }
    }

    while(*p && *p != 'E' && i < 1000) {        // add seps
      if(*p >= '0' && *p <= '9') {
        if(d > 0 && d % sepGroup == 0 && !GROUPRIGHT_DISABLED) {
          dispString[mi++] = sep[0];
          if(sep[0] & 0x80 && sep[1] && sep[1] != '\1') dispString[mi++] = sep[1];
        }
        dispString[mi++] = *p;
        i++;
        d++;
      }
      p++;
    }

    // Remove trailing zeros and separators from the right, until first non-zero or decimal is reached
    while( mi > 1 &&
          ((dispString[mi-1] == '0') ||
           (dispString[mi-2] == sep[0] && sep[1] != '\0' && sep[1] != '\1' && dispString[mi-1] == sep[1]) ||
           (dispString[mi-1] == sep[0] && sep[1] != '\0' && sep[1] != '\1' && dispString[mi  ] == sep[1])
          )
         ) mi--;
    if(mi > 0 && (dispString[mi-1] == radix[0] && (radix[1] == '\0' || radix[1] == '\1' || (radix[1] != '\0' && radix[1] != '\1' && dispString[mi-1] == radix[1])) )) mi--;
    if(mi > 1 && (dispString[mi-2] == radix[0] && (                                        (radix[1] != '\0' && radix[1] != '\1' && dispString[mi-1] == radix[1])) )) mi -= 2;

    dispString[mi] = '\0';
    char tt[32];
    exponentToDisplayString(exp, tt, NULL, false);
    sprintf(dispString + mi, "%s", tt);
}



int16_t startingLine = 0;
int16_t IntShowMode = 0;
int16_t source = 0;
#define SHOWAUTO 0
#define SHOWSML 1
#define SHOWTNY 2

void fnC47Show(uint16_t fnShow_param) {
#if !defined(SAVE_SPACE_DM42_9)
  #if !defined(TESTSUITE_BUILD)
    uint8_t savedDisplayFormat = displayFormat, savedDisplayFormatDigits = displayFormatDigits;
    uint64_t ssf0 = systemFlags0;
    uint64_t ssf1 = systemFlags1;
    bool_t thereIsANextLine;
    int16_t dest = 0, last = 0, d, i, offset, bytesProcessed, aa, bb, cc, dd, aa2 = 0, aa3 = 0, aa4 = 0, numberOfLines = 0;
    uint64_t nn;

    displayFormat = DF_ALL;
    displayFormatDigits = 0;
    clearSystemFlag(FLAG_IRFRAC);


    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
    switch(fnShow_param) {
      case NOPARAM:
               showSoftmenu(-MNU_SHOW); //continue, don't use 'break'
               source = 0;
               showRegis = REGISTER_X;
               startingLine = 0;
               IntShowMode = SHOWAUTO;
               break;

      case ITM_RS: //change page on SHOW LI, if in StandardFont
               if(getRegisterDataType(showRegis) == dtLongInteger || isXFNShowing(showRegis)) {
                 if(IntShowMode == SHOWTNY) {
                   startingLine = 0;
                   source = 0;
                   IntShowMode = SHOWAUTO;
                 }
                 else
                 if(IntShowMode != SHOWSML) {
                   startingLine = 0;
                   source = 0;
                   IntShowMode = SHOWSML;
                 }
                 else {
                   startingLine += 10;
                   if(startingLine >= 30 || source == last) {
                     startingLine = 0;
                     source = 0;
                     IntShowMode = SHOWTNY;
                   }
                 }
               }
               break;

      case ITM_UP1: //was 1
               source = 0;
               startingLine = 0;
               IntShowMode = SHOWAUTO;
               if(showRegis == 9999) {
                 showRegis = REGISTER_X;
               }
               else {
                 if(showRegis >= FIRST_NAMED_VARIABLE + numberOfNamedVariables - 1) {
                   showRegis = FIRST_GLOBAL_REGISTER;
                 }
                 else if(showRegis == LAST_SPARE_REGISTER) {
                   showRegis = FIRST_NAMED_VARIABLE;
                 }
                 else {
                   showRegis++;
                 }

                 while(!regInRange(showRegis) && showRegis < FIRST_NAMED_VARIABLE + numberOfNamedVariables) {
                   showRegis++;
                 }
               }
               #if defined(PC_BUILD) && defined(MONITOR_CLRSCR)
                 printf("R=%u TI=%u\n",showRegis, temporaryInformation);
               #endif // PC_BUILD && MONITOR_CLRSCR
               break;

      case ITM_DOWN1: //was 2
               source = 0;
               startingLine = 0;
               IntShowMode = SHOWAUTO;
               if(showRegis == 9999) {
                 showRegis = REGISTER_X;
               }
               else {
                 if(showRegis == FIRST_GLOBAL_REGISTER) {
                   showRegis = FIRST_NAMED_VARIABLE + numberOfNamedVariables - 1;
                 }
                 else if(showRegis == FIRST_NAMED_VARIABLE) {
                   showRegis = LAST_SPARE_REGISTER;
                 }
                 else {
                   showRegis--;
                 }
                 while(!regInRange(showRegis) && showRegis != FIRST_GLOBAL_REGISTER) {
                   showRegis--;
                 }
               }
               #if defined(PC_BUILD) && defined(MONITOR_CLRSCR)
                 printf("R=%u TI=%u\n",showRegis, temporaryInformation);
               #endif // PC_BUILD && MONITOR_CLRSCR
               break;
      case ITM_NOP:                                       //Allow fnView to enter a value into Show without changing the register number
               break;
      default:
        break;
    }
    #pragma GCC diagnostic pop

// printf("fnC47Show: fnShow_param=%i startingLine=%i IntShowMode=%i source=%i\n",fnShow_param, startingLine, IntShowMode, source);


    #if !defined(TESTSUITE_BUILD)
      #if defined(PC_BUILD) && defined(MONITOR_CLRSCR)
        printf(">>> ---- clearScreenOld from display.c fnC47Show\n");
      #endif // PC_BUILD && MONITOR_CLRSCR
        //      clearScreenOld(!clrStatusBar, clrRegisterLines, !clrSoftkeys); //Clear screen content while NEW SHOW
        refreshScreen(153);
    #endif // !TESTSUITE_BUILD

    SHOW_reset();




    #define dtXFN 100
    int selection = getRegisterDataType(showRegis);

    if(isXFNShowing(showRegis)) {
      switch(showRegis) {
        case 90:
        case 93:
        case 96:
        case REGISTER_X:
        case REGISTER_T:
          selection = dtXFN;
          break;
        default:;
      }
    }



    switch(selection) {
      case dtXFN : {
          //XFN Format is real, constructed into a string, into the LI display function
          strcpy(errorMessage,tmpString + 2100);
          if(registerFMAOutputString(showRegis, showRegis == REGISTER_X ? "XY+Z = " :
                                                showRegis == REGISTER_T ? "TA+B = " :
                                                                          "FMA: ",     errorMessage + stringByteLength(errorMessage))) {
            goto XFNentryPoint;
          }
          break;
        }

      case dtLongInteger:

        #if defined(VERBOSE_SCREEN) && defined(PC_BUILD)
          printf("SHOW:Longint\n");
        #endif // VERBOSE_SCREEN && PC_BUILD

        strcpy(errorMessage,tmpString + 2100);
        longIntegerRegisterToDisplayString(showRegis, errorMessage + stringByteLength(tmpString + 2100), WRITE_BUFFER_LEN, 25*SCREEN_WIDTH, /*10*50-3*/ 1010, false);  //JM added last parameter: AglyphNumberow LARGELI
XFNentryPoint:

        last = stringByteLength(errorMessage);
        int16_t glyphNumber = stringGlyphLength(errorMessage);

        //printf("glyphNumber %i\n",glyphNumber);

        //LARGE font, one page
        if(IntShowMode == SHOWAUTO){
          if(glyphNumber < 170){ //using 170 as an obvious cutoff. Longer than 170 definately wont work. Shorter must be tried.
            temporaryInformation = TI_SHOW_REGISTER_BIG;
            numberOfLines = 6;
            startingLine = 0;
            int16_t sourcemem = source;
            int16_t destmem = dest;
            prepLongintIntoLines(&last, &source, &dest, &numericFont, SCREEN_WIDTH, numberOfLines, &startingLine);
            //printf("001 ll=%i source=%i last=%i\n",glyphNumber, source, last);
            if(tmpString[numberOfLines*SHOWLineSize] == 0) {
              break;
            }
            source = sourcemem;
            dest = destmem;
            IntShowMode = SHOWSML; // if not broken out, go smaller
          }
          else {
            IntShowMode = SHOWSML;  // if >170 chars, go smaller
          }
        }


        //STANDARD font, three pages
        if(IntShowMode == SHOWSML) {
          SHOW_reset();
          temporaryInformation = TI_SHOW_REGISTER_SMALL;
          numberOfLines = 10;
          prepLongintIntoLines(&last, &source, &dest, &standardFont, SCREEN_WIDTH, numberOfLines, &startingLine);
          if(tmpString[0] != 0) {
            goto goBreak1; //break if first line first character is non-terminator and display
          }
          else {
            startingLine = 0;
            source = 0;
            dest = 0;
            IntShowMode = SHOWTNY;
          }
        }


        //TINY font, one page
        if(IntShowMode == SHOWTNY) {
          SHOW_reset();
          temporaryInformation = TI_SHOW_REGISTER_TINY;
          numberOfLines = min(21,SHOWLineMax);
          startingLine = 0;
          prepLongintIntoLines(&last, &source, &dest, &tinyFont, SCREEN_WIDTH, numberOfLines, &startingLine);

goBreak1:

          if(tmpString[numberOfLines*SHOWLineSize]!=0) {                                         // The long integer is too long for the last display string

            int16_t ii = stringLastGlyph(tmpString + (numberOfLines-1)*SHOWLineSize);            // last char of last display string; note that dest is the terminator
            source = stringPrevNumberGlyph(errorMessage,source);                                 // ssource is still pointing to the next unprinted digit: hift source by one, to match the dest

            if(IntShowMode == SHOWSML) {
                while(ii > 10 && (int16_t)((stringWidth(tmpString + (numberOfLines-1)*SHOWLineSize, &standardFont, true, true) + stringWidth(STD_ELLIPSIS, &standardFont, true, true) + stringWidth(STD_SPACE_6_PER_EM, &standardFont, true, true)) >= SCREEN_WIDTH)) {
                  source = stringPrevNumberGlyph(errorMessage,source);
                  tmpString[+ (numberOfLines-1)*SHOWLineSize + ii] = 0;
                  ii = stringPrevNumberGlyph(tmpString + (numberOfLines-1)*SHOWLineSize,ii);
                }
            }

            xcopy(tmpString + (numberOfLines-1)*SHOWLineSize + ii, STD_ELLIPSIS, 2);        // * Ellipsis needes 6perEM space to line up with two digitss
            ii += 2;                                                                        // *
            xcopy(tmpString + (numberOfLines-1)*SHOWLineSize + ii, STD_SPACE_6_PER_EM, 2);  // *
            ii += 2;                                                                        // *
            tmpString[(numberOfLines-1)*SHOWLineSize + ii++] = 0;                           // *
          }
        }
        break;

      case dtReal34:
        #if defined(VERBOSE_SCREEN) && defined(PC_BUILD)
          printf("SHOW:Real\n");
        #endif // VERBOSE_SCREEN && PC_BUILD
        temporaryInformation = TI_SHOW_REGISTER_BIG;
        int16_t angleM = getRegisterAngularMode(showRegis);
        if(angleM == amDMS) angleM = amDegree;
        real34ToDisplayString(REGISTER_REAL34_DATA(showRegis), angleM, tmpString + 2100+stringByteLength(tmpString + 2100), &numericFont, SCREEN_WIDTH * 2, 34, !LIMITEXP, !FRONTSPACE, NOIRFRAC);
        last = 2100 + stringByteLength(tmpString + 2100);
        source = 2100;
        for(d=0; d<=3*SHOWLineSize ; d+=SHOWLineSize) {
          dest = d;
          tmpString[d] = 0;
          while(source < last && stringWidth(tmpString + d, &numericFont, true, true) <= SCREEN_WIDTH - 8*2-5) {
            //check if a full triplet of digits will fit otherwise break line
            if((tmpString[source] == SEPARATOR_LEFT[0] && (SEPARATOR_LEFT[1]==1 ? true : tmpString[source + 1] == SEPARATOR_LEFT[1])     ) ||
               (tmpString[source] == SEPARATOR_RIGHT[0] && (SEPARATOR_RIGHT[1]==1 ? true : tmpString[source + 1] == SEPARATOR_RIGHT[1]) )) {
              aa = source;
              if(tmpString[aa] & 0x80) aa += 2;
              if(tmpString[aa] & 0x80) aa += 2;
              if(tmpString[aa] & 0x80) aa += 2;
              if(tmpString[aa] & 0x80) aa += 2;
              char tmpString20[20];
              tmpString20[0]=0;
              xcopy(tmpString20, tmpString + source, aa - source);
              tmpString20[aa - source]=0;
              if(stringWidth(tmpString + d, &numericFont, true, true) + stringWidth(tmpString20, &numericFont, true, true) > SCREEN_WIDTH - 8*2-5) break;
            }

            tmpString[dest] = tmpString[source];
            if(tmpString[dest] & 0x80) {
              tmpString[++dest] = tmpString[++source];
            }
            source++;
            tmpString[++dest] = 0;
          }
        }

        if(getRegisterAngularMode(showRegis) != amNone) {
          aa = 0;
          bb = 0;
          cc = 0;
          dd = 0;
          switch(getRegisterAngularMode(showRegis)) {
            case amDegree: aa = amDMS;    bb = amRadian; cc = amMultPi; dd = amGrad;   break;
            case amRadian: aa = amMultPi; bb = amDegree; cc = amDMS;    dd = amGrad;   break;
            case amGrad:   aa = amDegree; bb = amDMS;    cc = amRadian; dd = amMultPi; break;
            case amMultPi: aa = amRadian; bb = amDegree; cc = amDMS;    dd = amGrad;   break;
            case amDMS:    aa = amDMS;    bb = amRadian; cc = amMultPi; dd = amGrad;   break;  //note amDMS displays the same way as amDegree, to show all 34 digits in the top line
            default: ;
          }

          overrideShowBottomLine = 40;     //from bototm, total 5
          printXSHOW(aa, 2*SHOWLineSize, displayFormat, displayFormatDigits, dtReal34, false);
          printXSHOW(bb, 3*SHOWLineSize, displayFormat, displayFormatDigits, dtReal34, false);
          printXSHOW(cc, 4*SHOWLineSize, displayFormat, displayFormatDigits, dtReal34, false);
          printXSHOW(dd, 5*SHOWLineSize, displayFormat, displayFormatDigits, dtReal34, false);
        }
        else {
          overrideShowBottomLine = 30;    // from bottom, total 5
          setSystemFlag(FLAG_ENGOVR);
          printXSHOW(amNone, 3*SHOWLineSize, DF_SF, 6, dtReal34, false);
          printXSHOW(amNone, 4*SHOWLineSize, DF_UN, 3, dtReal34, false);
          printXSHOW(amNone, 5*SHOWLineSize, DF_SCI, 3, dtReal34, false);
        }
        break;

      case dtComplex34:
        #if defined(VERBOSE_SCREEN) && defined(PC_BUILD)
          printf("SHOW:Complex\n");
        #endif // VERBOSE_SCREEN && PC_BUILD
        temporaryInformation = TI_SHOW_REGISTER_BIG;

        complex34ToDisplayString(REGISTER_COMPLEX34_DATA(showRegis), tmpString, &numericFont,2000, 34, LIMITEXP, FRONTSPACE, NOIRFRAC, getComplexRegisterAngularMode(showRegis), getComplexRegisterPolarMode(showRegis));
        for(i=stringByteLength(tmpString) - 1; i>0; i--) {
          if(tmpString[i] == 0x08) { //change punctuation space to EM4
            tmpString[i] = 0x05;
          }
        }

        //copy result into destination 2100 (label already in 2100-2102)
        last = 2100;
        while(tmpString[last]) last++;
        xcopy(tmpString + last, tmpString + 0,  strlen(tmpString + 0) + 1);
        tmpString[0] = 0;


        int32_t strWid = stringWidth(tmpString + 2100, &numericFont, true, true);
        d = 2100;
        int16_t hadFirstRealDigit = 0;
        while ( d < 2100 + stringByteLength(tmpString + 2100)) {
          if(hadFirstRealDigit == 0 && tmpString[d] >= '0' && tmpString[d] <='9') {
            hadFirstRealDigit = d - 2100;
          }
          if(hadFirstRealDigit > 0 && (tmpString[d] == '+' || tmpString[d] == '-')) break;
          d = 2100 + stringNextGlyph(tmpString + 2100,d - 2100);
        }
        int8_t tmpp = tmpString[d];
        tmpString[d] = 0;
        int32_t strWidReal = stringWidth(tmpString + 2100, &numericFont, true, true);
        tmpString[d] = tmpp;
        int32_t strWidImag = stringWidth(tmpString + d, &numericFont, true, true);
        int32_t strWidCur = 0;
        bool_t changedOverToImag = false;

        //printf("\ntmpString: %i %s %i %i %i\n",hadFirstRealDigit, tmpString + 2100, strWid, strWidReal, strWidImag);
        //printStringToConsole(tmpString + 2100,"tmpStr ","\n");

        //write 2100+ into four lines, 0+ to 750+
        last = 2100 + stringByteLength(tmpString + 2100);
        source = 2100;
        for(d=0; d<=3*SHOWLineSize ; d+=SHOWLineSize) {
          dest = d;
          tmpString[d] = 0;
          while(source < last && stringWidth(tmpString + d, &numericFont, true, true) <= SCREEN_WIDTH - 8*2-5) {
            tmpString[dest] = tmpString[source];
            if(tmpString[dest] & 0x80) {
              tmpString[++dest] = tmpString[++source];
            }
            source++;
            tmpString[++dest] = 0;

            #define strWidLim ((55*(changedOverToImag ? (strWidImag) : (strWidReal)))/100)

            strWidCur = stringWidth(tmpString + d, &numericFont, true, true);

            if( (strWidCur >= SCREEN_WIDTH-60 && strWidCur > strWidLim) && (uint8_t)tmpString[source-2] == 160 && (uint8_t)tmpString[source-1]==5) break;   // breaking on seps
            if(d == 0 && source - 2100 > hadFirstRealDigit && strWid > SCREEN_WIDTH && ( !(getComplexRegisterPolarMode(showRegis) == amPolar) && (tmpString[source]=='+' || tmpString[source]=='-'))) break;     // break before the + -)
            if( source - 2100 > hadFirstRealDigit && ((uint8_t)tmpString[source]=='+' || (uint8_t)tmpString[source+1]=='-')) changedOverToImag = true;
            if(d>0 &&
              (    ( !(getComplexRegisterPolarMode(showRegis) == amPolar) && (tmpString[source]=='+' || tmpString[source]=='-'))       //break before the + -
                   || (stringWidth(tmpString + d, &numericFont, true, true) >= (SCREEN_WIDTH*4)/5 && (uint8_t)tmpString[source]==RADIX34_MARK_STRING[0] && (uint8_t)tmpString[source+1]==RADIX34_MARK_STRING[1])  //break before the radix
                   || (getSystemFlag(FLAG_CPXMULT) && (uint8_t)tmpString[source]==COMPLEX_UNIT[0] && (uint8_t)tmpString[source+1]==COMPLEX_UNIT[1])  //break before the complex operator
              ) ) {
              break;
            }
          }
        }

        overrideShowBottomLine = 20;   //2 from bottom, total 5
        setSystemFlag(FLAG_ENGOVR);
        printXSHOW(amNone, 4*SHOWLineSize, DF_SF, 4, dtComplex34, true);
        printXSHOW(amNone, 5*SHOWLineSize, DF_SF, 4, dtComplex34, false);

        //if(tmpString[300]==0) {                          //shift up if line is empty
        //  //vv new       strcpy(tmpString + 300, tmpString + 600);
        //  xcopy(tmpString + 300, tmpString + 600,  min(300,strlen(tmpString + 600) + 1));
        //  //vv new        strcpy(tmpString + 600, tmpString + 900);
        //  xcopy(tmpString + 600, tmpString + 900,  min(300,strlen(tmpString + 900) + 1));
        //  tmpString[900] = 0;
        //}

        //if(tmpString[600]==0) {                          //shift up if line is empty
        //  //vv new        strcpy(tmpString + 600, tmpString + 900);
        //  xcopy(tmpString + 600, tmpString + 900,  min(300,strlen(tmpString + 900) + 1));
        //  tmpString[900] = 0;
        //}

        break;

      case dtShortInteger:
        #if defined(VERBOSE_SCREEN) && defined(PC_BUILD)
          printf("SHOW:Shortint\n");
        #endif // VERBOSE_SCREEN && PC_BUILD
        temporaryInformation = TI_SHOW_REGISTER_BIG;

        shortIntegerToDisplayString(showRegis, tmpString + 2100, true, noBaseOverride); //jm include X:
  /*
        if(getRegisterTag(showRegis) == 2) {
          source = 2100;
          dest = 2400;
          while(tmpString[source] !=0 ) {
            if((uint8_t)(tmpString[source]) == 160 && (uint8_t)(tmpString[source+1]) == 39) {
              source++;
              tmpString[dest++]=49;
            }
            else
              if((uint8_t)(tmpString[source]) == 162 && (uint8_t)(tmpString[source+1]) == 14) {
                source++;
                tmpString[dest++]=48;
              }
              else {
                tmpString[dest++] = tmpString[source];
              }
            source++;
          }
          tmpString[dest]=0;
        }
  */
  //      else {
          strcpy(tmpString + 2400,tmpString + 2100);
  //      }

        last = 2400 + stringByteLength(tmpString + 2400);
        source = 2400;
        tmpString[0]=0;
        for(d=0; d<=3*SHOWLineSize ; d+=SHOWLineSize) {
          dest = d;
          tmpString[d] = 0;
          if(dest != 0){strcat(tmpString + dest,"  ");dest+=2;}               //space below the T:
          while(source < last) {
            tmpString[dest] = tmpString[source];
            if(tmpString[dest] & 0x80) {
              tmpString[++dest] = tmpString[++source];
            }
            source++;
            tmpString[++dest] = 0;
          }
          checkAndEat(&source, last, &dest);
        }

        convertShortIntegerRegisterToUInt64(showRegis, &aa, &nn);
        aa = getRegisterTag(showRegis);

        switch(aa) {
          case  2: aa2=10;  aa3= 8;  aa4=16; break;   //Keeping the 2 8 16 sequence where possible
          case  4: aa2= 2;  aa3= 8;  aa4=16; break;   //Keeping the 2 8 16 sequence where possible
          case  8: aa2= 2;  aa3=10;  aa4=16; break;   //Keeping the 2 8 16 sequence where possible
          case 10: aa2= 2;  aa3= 8;  aa4=16; break;   //Keeping the 2 8 16 sequence where possible
          case 16: aa2= 2;  aa3= 8;  aa4=10; break;   //Keeping the 2 8 16 sequence where possible

          case  3:
          case  5:
          case  6:
          case  7:
          case  9:
          case 11:
          case 12:
          case 13:
          case 14:
          case 15: aa2=10;  aa3= 8;  aa4=16; break;
        }

        if(aa2){
          setRegisterTag(showRegis,aa2);
          RegName();
          shortIntegerToDisplayString(showRegis, tmpString + 2100, true, noBaseOverride);
          strcpy(tmpString + 2400,tmpString + 2100);
          last = 2400 + stringByteLength(tmpString + 2400);
          source = 2400;
          tmpString[SHOWLineSize]=0;
          for(d=SHOWLineSize; d<=3*SHOWLineSize ; d+=SHOWLineSize) {
            dest = d;
            tmpString[0] = 0;
            if(dest != SHOWLineSize){strcat(tmpString + dest,"  ");dest+=2;}               //space below the T:
            while(source < last) {
              tmpString[dest] = tmpString[source];
              if(tmpString[dest] & 0x80) {
                tmpString[++dest] = tmpString[++source];
              }
              source++;
              tmpString[++dest] = 0;
            }
            checkAndEat(&source, last, &dest);
          }
        }
        if(aa3){
          RegName();
          setRegisterTag(showRegis,aa3);
          shortIntegerToDisplayString(showRegis, tmpString + 2100, true, noBaseOverride);
          strcpy(tmpString + 2400,tmpString + 2100);
          last = 2400 + stringByteLength(tmpString + 2400);
          source = 2400;
          tmpString[2*SHOWLineSize]=0;
          for(d=2*SHOWLineSize; d<=3*SHOWLineSize ; d+=SHOWLineSize) {
            dest = d;
            tmpString[d] = 0;
            if(dest != 2*SHOWLineSize){strcat(tmpString + dest,"  ");dest+=2;}               //space below the T:
            while(source < last) {
              tmpString[dest] = tmpString[source];
              if(tmpString[dest] & 0x80) {
                tmpString[++dest] = tmpString[++source];
              }
              source++;
              tmpString[++dest] = 0;
            }
            checkAndEat(&source, last, &dest);
          }
        }
        if(aa4){
          RegName();
          setRegisterTag(showRegis,aa4);
          shortIntegerToDisplayString(showRegis, tmpString + 2100, true, noBaseOverride);
          strcpy(tmpString + 2400,tmpString + 2100);
          last = 2400 + stringByteLength(tmpString + 2400);
          source = 2400;
          tmpString[3*SHOWLineSize]=0;
          for(d=3*SHOWLineSize; d<=3*SHOWLineSize ; d+=SHOWLineSize) {
            tmpString[d] = 0;
            dest = d;
            if(dest != 3*SHOWLineSize){strcat(tmpString + dest,"  ");dest+=2;}               //space below the T:
            while(source < last) {
              tmpString[dest] = tmpString[source];
              if(tmpString[dest] & 0x80) {
                tmpString[++dest] = tmpString[++source];
              }
              source++;
              tmpString[++dest] = 0;
            }
            checkAndEat(&source, last, &dest);
          }
        }
        setRegisterTag(showRegis,aa);
        break;

      case dtTime:
        //SHOW_reset();
        strcpy(tmpString, tmpString + 2100);
        temporaryInformation = TI_SHOW_REGISTER_BIG;
        timeToDisplayString(showRegis, tmpString + stringByteLength(tmpString + 2100), true);
        break;

      case dtDate:
        //SHOW_reset();
        strcpy(tmpString, tmpString + 2100);
        temporaryInformation = TI_SHOW_REGISTER_BIG;
        dateToDisplayString(showRegis, tmpString + stringByteLength(tmpString + 2100));
        break;


      case dtString:
        #if defined(VERBOSE_SCREEN) && defined(PC_BUILD)
          printf("SHOW:String\n");
        #endif // VERBOSE_SCREEN && PC_BUILD

        SHOW_reset();
        temporaryInformation = TI_SHOW_REGISTER_BIG; //First try one line of big font.
        offset = 0;
        thereIsANextLine = true;
        bytesProcessed = 2100;
        strcat(tmpString + 2100, "'");
        strcat(tmpString + 2100, REGISTER_STRING_DATA(showRegis));//, stringByteLength(REGISTER_STRING_DATA(showRegis)) + 4+1);
        strcat(tmpString + 2100, "'");
        #if defined(VERBOSE_SCREEN) && defined(PC_BUILD)
          uint32_t tmp = 0;
          printf("^^^0 %4u", tmp);
          printf("^^^^$$ %s %d\n", tmpString + 2100, stringWidthC47(tmpString + 2100, stdnumEnlarge, nocompress, false, true));
        #endif // VERBOSE_SCREEN && PC_BUILD
        while(thereIsANextLine) {
          char *strw;
          xcopy(tmpString + offset, tmpString + bytesProcessed, stringByteLength(tmpString + bytesProcessed) + 1);
          thereIsANextLine = false;
          strw = stringAfterPixelsC47(tmpString + offset, stdnumEnlarge, nocompress, SCREEN_WIDTH - 1, false, true);
          if(*strw != 0) {
            *strw = 0;
            thereIsANextLine = true;
            #if defined(VERBOSE_SCREEN) && defined(PC_BUILD)
              printf("^^^A %4u",tmp++);
              printf("^^^^$$ %s %d\n",tmpString + offset,stringWidthC47(tmpString + offset, stdnumEnlarge, nocompress, false, true));
            #endif // VERBOSE_SCREEN && PC_BUILD
          }
          bytesProcessed += stringByteLength(tmpString + offset);
          offset += SHOWLineSize;
          tmpString[offset] = 0;
        }
        if(offset <= 4*SHOWLineSize) break; //else continue on the small font


        SHOW_reset();
        temporaryInformation = TI_SHOW_REGISTER_SMALL;
        offset = 0;
        thereIsANextLine = true;
        bytesProcessed = 2100;
        strcat(tmpString + 2100, "'");
        strcat(tmpString + 2100, REGISTER_STRING_DATA(showRegis));//, stringByteLength(REGISTER_STRING_DATA(showRegis)) + 4+1);
        strcat(tmpString + 2100, "'");
        #if defined(VERBOSE_SCREEN) && defined(PC_BUILD)
          uint32_t tmp2 = 0;
        #endif // VERBOSE_SCREEN && PC_BUILD
        while(thereIsANextLine) {
          char *remainingString;
          xcopy(tmpString + offset, tmpString + bytesProcessed, stringByteLength(tmpString + bytesProcessed) + 1);
          thereIsANextLine = false;
          #if defined(VERBOSE_SCREEN) && defined(PC_BUILD)
            tmp =0;
          #endif // VERBOSE_SCREEN && PC_BUILD
          remainingString = stringAfterPixels(tmpString + offset, &standardFont, SCREEN_WIDTH, false, true);
          if(*remainingString != 0) {
            *remainingString = 0;
            thereIsANextLine = true;
            #if defined(VERBOSE_SCREEN) && defined(PC_BUILD)
              printf("^^^B %4u %4u",tmp2, tmp++);
              printf("^^^^$$ %s %d\n",tmpString + offset,stringWidth(tmpString + offset, &standardFont, false, true));
            #endif // VERBOSE_SCREEN && PC_BUILD
          }
          #if defined(VERBOSE_SCREEN) && defined(PC_BUILD)
            tmp2++;
          #endif // VERBOSE_SCREEN && PC_BUILD
          bytesProcessed += stringByteLength(tmpString + offset);
          offset += SHOWLineSize;
          tmpString[offset] = 0;
        }
        break;


      case dtConfig:
        temporaryInformation = TI_SHOW_REGISTER_BIG;
        xcopy(tmpString, "Configuration data", 19);
        break;

      case dtReal34Matrix:
      case dtComplex34Matrix:
        clearScreenOld(!clrStatusBar, clrRegisterLines, clrSoftkeys);
        dispM(showRegis, tmpString + 2100);                   //then display the matrix
        drawSinglePixelFullWidthLine(Y_POSITION_OF_REGISTER_T_LINE-4);
        temporaryInformation = TI_SHOWNOTHING;                //then tell the system it is in show nothing mode,
        if(programRunStop == PGM_RUNNING) {   //this needs to be checked - maybe needed for all show items not only here
          refreshScreen(150);
          fnPause(10);
          temporaryInformation = TI_NO_INFO;
        }
        break;


      default:
        temporaryInformation = TI_NO_INFO;
        displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, showRegis);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "cannot SHOW %s%s", tmpString + 2100, getRegisterDataTypeName(showRegis, true, false));
          moreInfoOnError("In function fnC47Show:", errorMessage, NULL, NULL);
        #endif
        return;
    }


    displayFormat = savedDisplayFormat;
    displayFormatDigits = savedDisplayFormatDigits;
    systemFlags0 = ssf0;
    systemFlags1 = ssf1;

    #if defined(VERBOSE_SCREEN) && defined(PC_BUILD)
      printf("SHOW:Done |%s|\n",tmpString);
    #endif

  #endif // !TESTSUITE_BUILD
#else
    fnView(REGISTER_X); // Re-direct to use VIEW instead. No more accuracy though
#endif // !SAVE_SPACE_DM42_9
}

void _view(uint16_t regist) {
  if(regInRange(regist)) {
    currentViewRegister = regist;
    temporaryInformation = TI_VIEW_REGISTER;
    if(programRunStop == PGM_RUNNING) {
      refreshScreen(151);
//      temporaryInformation = TI_NO_INFO;  //JM removed to signal to STOP, so that STOP does not clear the screen after VIEW
    }
  }
}

void fnView(uint16_t regist) {
  _view(regist);
}

void fnAview(uint16_t regist) {
  _view(regist);
}

void fnPrompt(uint16_t regist) {
  _view(regist);
  fnStopProgram(NOPARAM);
}
