#include "c47.h"
#include <string.h>
#include <stdio.h>
#include <android/log.h>
#include "display.h"
#include "fonts.h"
#include "mathematics/toPolar.h"

#ifndef LOG_TAG
#define LOG_TAG "R47Helpers"
#endif
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

void ascii_clean(char *str) {
    if (!str || !*str) return;
    char tmp[1024];
    char *s = str;
    char *d = tmp;
    while (*s && (d - tmp) < 1023) {
        uint8_t c1 = (uint8_t)s[0];
        uint8_t c2 = (uint8_t)s[1];
        
        if (c1 == 0xA1 && c2 == 0x48) { // STD_op_i
            *d++ = ' '; *d++ = 'i'; *d++ = ' '; s += 2; 
        } else if (c1 == 0xA1 && c2 == 0x49) { // STD_op_j
            *d++ = ' '; *d++ = 'j'; *d++ = ' '; s += 2;
        } else if (c1 == 0x80 && c2 == 0xB7) { // STD_DOT (·)
            *d++ = ' '; s += 2;
        } else if (c1 == 0x80 && c2 == 0xD7) { // STD_CROSS (×)
            *d++ = '*'; s += 2;
        } else if (c1 == 0xA0 && c2 >= 0x80 && c2 <= 0x89) { // STD_SUB_0..9
            *d++ = (c2 - 0x80) + '0'; s += 2;
        } else if (c1 == 0xA0 && c2 == 0x8A) { // STD_SUB_PLUS
            *d++ = '+'; s += 2;
        } else if (c1 == 0xA0 && c2 == 0x8B) { // STD_SUB_MINUS
            *d++ = '-'; s += 2;
        } else if (c1 == 0xA1 && c2 >= 0x60 && c2 <= 0x69) { // STD_SUP_0..9
            *d++ = (c2 - 0x60) + '0'; s += 2;
        } else if (c1 == 0xA1 && c2 == 0x6B) { // STD_SUP_MINUS
            *d++ = '-'; s += 2;
        } else if (c1 == 0xA1 && c2 == 0x6A) { // STD_SUP_PLUS
            *d++ = '+'; s += 2;
        } else if ((c1 == 0xA4 && c2 == 0x69) || (c1 == 0xA4 && c2 == 0x7D)) { // STD_BASE_10 or STD_SUB_10 (x10)
            *d++ = 'e'; s += 2;
        } else if (c1 == 0x82 && c2 == 0xB3) { // STD_SUP_BOLD_r
            *d++ = 'r'; s += 2;
        } else if (c1 == 0x9D && c2 == 0x4D) { // STD_SUP_BOLD_g
            *d++ = 'g'; s += 2;
        } else if (c1 == 0xAC && c2 == 0x66) { // STD_SUP_pir
            *d++ = (char)0x83; *d++ = (char)0xC0; *d++ = 'r'; s += 2;
        } else if (c1 == 0xA0 && (c2 >= 0x03 && c2 <= 0x08)) { // STD_SPACE_EM..STD_SPACE_PUNCTUATION
            *d++ = ' '; s += 2;
        } else if (c1 == 0xA0 && c2 == 0x0A) { // STD_SPACE_HAIR
            *d++ = ' '; s += 2;
        } else {
            *d++ = *s++;
        }
    }
    *d = '\0';
    strcpy(str, tmp);
}

void trimTrailingRadix(char *str) {
    if (!str || !*str) return;
    size_t len = strlen(str);
    while (len > 0 && (str[len-1] == ' ' || str[len-1] == '.' || str[len-1] == ',')) {
        str[--len] = '\0';
    }
    // Also remove any trailing radix marks preceding an angle tag (e.g. "86.°" -> "86°", "45.,deg" -> "45deg")
    char *radixPos;
    while ((radixPos = strpbrk(str, ".,")) != NULL) {
        char *p = radixPos;
        while (*p == '.' || *p == ',' || *p == ' ' || *p == '\t') {
            p++;
        }
        if (*p == 'd' || *p == 'r' || *p == 'g' ||
            (unsigned char)*p == 0x80 ||
            ((unsigned char)*p == 0xC2 && (unsigned char)*(p + 1) == 0xB0)) {
            memmove(radixPos, p, strlen(p) + 1);
        } else {
            break;
        }
    }
}

static void angularUnitToString(angularMode_t angularMode, char *string) {
  switch(angularMode) {
    case amRadian: strcpy(string, "r");        break;
    case amMultPi: strcpy(string, STD_pi "r"); break;
    case amGrad:   strcpy(string, "g");        break;
    case amDegree: strcpy(string, STD_DEGREE); break;
    case amDMS:    strcpy(string, "d.ms");     break;
    case amNone:                               break;
    default:       strcpy(string, "?");
  }
}

static void formatRegisterString(calcRegister_t regist, char *result, size_t maxLen) {
  static char coreBuf[32768];
  coreBuf[0] = '\0';
  result[0] = '\0';

  if (!ram) {
    snprintf(result, maxLen, "0");
    return;
  }

  uint8_t dataType = getRegisterDataType(regist);
  uint8_t tag = getRegisterTag(regist);

  switch(dataType) {
    case dtReal34: {
      real34_t reduced;
      real34Reduce((const real34_t *)getRegisterDataPointer(regist), &reduced);
      real34ToString(&reduced, coreBuf);
      if(strchr(coreBuf, '.') == NULL && strchr(coreBuf, 'E') == NULL) {
        strcat(coreBuf, ".");
      }
      angularUnitToString(tag & amAngleMask, coreBuf + strlen(coreBuf));
      break;
    }

    case dtComplex34: {
      bool_t isPolar = ((tag & amPolar) == amPolar) || getSystemFlag(FLAG_POLAR);
      if(isPolar) {
        angularMode_t angMode = (tag & amAngleMask);
        if(angMode == amNone) {
          angMode = currentAngularMode;
        }
        real34_t mag34, ang34, reduced;
        const real34_t *real34 = (const real34_t *)getRegisterDataPointer(regist);
        const real34_t *imag34 = real34 + 1;

        real34RectangularToPolar(real34, imag34, &mag34, &ang34);
        convertAngle34FromTo(&ang34, amRadian, angMode);

        char tmpMag[100];
        real34Reduce(&mag34, &reduced);
        real34ToString(&reduced, tmpMag);
        if(strchr(tmpMag, '.') == NULL && strchr(tmpMag, 'E') == NULL) {
          strcat(tmpMag, ".");
        }

        char tmpAng[100];
        real34Reduce(&ang34, &reduced);
        real34ToString(&reduced, tmpAng);
        char unitStr[32] = "";
        angularUnitToString(angMode, unitStr);
        if(unitStr[0] == 0 && strchr(tmpAng, '.') == NULL && strchr(tmpAng, 'E') == NULL) {
          strcat(tmpAng, ".");
        }

        sprintf(coreBuf, "%s " STD_MEASURED_ANGLE " %s%s", tmpMag, tmpAng, unitStr);
      } else {
        real34_t reduced;
        int len;
        char tmpStr[100];

        // Real part
        real34Reduce((const real34_t *)getRegisterDataPointer(regist), &reduced);
        real34ToString(&reduced, tmpStr);
        if(strchr(tmpStr, '.') == NULL && strchr(tmpStr, 'E') == NULL) {
          strcat(tmpStr, ".");
        }
        len = strlen(tmpStr);

        // Imaginary part
        real34Reduce(((const real34_t *)getRegisterDataPointer(regist)) + 1, &reduced);
        if(real34IsNegative(&reduced)) {
          sprintf(coreBuf, "%s - %sx", tmpStr, COMPLEX_UNIT);
          len += 5;
          real34SetPositiveSign(&reduced);
          real34ToString(&reduced, coreBuf + len);
        } else {
          sprintf(coreBuf, "%s + %sx", tmpStr, COMPLEX_UNIT);
          len += 5;
          real34ToString(&reduced, coreBuf + len);
        }
        if(strchr(coreBuf + len, '.') == NULL && strchr(coreBuf + len, 'E') == NULL) {
          strcat(coreBuf + len, ".");
        }
      }
      break;
    }

    case dtString:
      strncpy(coreBuf, REGISTER_STRING_DATA(regist), 1023);
      coreBuf[1023] = '\0';
      break;

    case dtShortInteger:
      shortIntegerToDisplayString(regist, coreBuf, false, 0, SCREEN_WIDTH);
      break;

    case dtLongInteger:
      longIntegerRegisterToDisplayString(regist, coreBuf, sizeof(coreBuf), 9999, 50, true);
      break;

    case dtTime:
      timeToDisplayString(regist, coreBuf, false);
      {
        char *src = coreBuf, *dst = coreBuf;
        while(*src == ' ' || *src == '\t') {
          src++;
        }
        while(*src) {
          if(*src != ' ' && *src != '\t') {
            *dst++ = *src;
          }
          src++;
        }
        *dst = '\0';
      }
      break;

    case dtDate:
      dateToDisplayString(regist, coreBuf);
      break;

    case dtReal34Matrix:
    case dtComplex34Matrix: {
      extern void copyRegisterToClipboardString(calcRegister_t regist, char *clipboardString, bool_t forPrinter);
      copyRegisterToClipboardString(regist, coreBuf, false);
      break;
    }

    default:
      sprintf(coreBuf, "[%s]", getDataTypeName(dataType, false, false));
      break;
  }

  if(dataType != dtReal34Matrix && dataType != dtComplex34Matrix && dataType != dtString) {
    ascii_clean(coreBuf);
    trimTrailingRadix(coreBuf);
  } else {
    ascii_clean(coreBuf);
  }

  // Convert core glyphs to valid UTF-8
  extern void stringToUtf8(const char *str, uint8_t *utf8);
  static char utf8Buf[32768];
  stringToUtf8(coreBuf, (uint8_t *)utf8Buf);
  snprintf(result, maxLen, "%s", utf8Buf);
}

char* getXRegisterString() {
  static char result[32768];
  formatRegisterString(REGISTER_X, result, sizeof(result));
  LOGI("getXRegisterString: Result='%s'", result);
  return result;
}

char* getAllRegistersString(void) {
  static char result[65536];
  char regVal[8192];
  char *ptr = result;
  size_t remaining = sizeof(result);
  result[0] = '\0';

  if(!ram) {
    return "";
  }

  extern char letteredRegisterName(calcRegister_t regist);
  extern void stringToUtf8(const char *str, uint8_t *utf8);

  // 1. Stack registers: LAST_SPARE_REGISTER down to REGISTER_X
  const char *sep = "";
  for(calcRegister_t r = LAST_SPARE_REGISTER; r >= REGISTER_X; r--) {
    formatRegisterString(r, regVal, sizeof(regVal));
    int written = snprintf(ptr, remaining, "%s%c = %s", sep, letteredRegisterName(r), regVal);
    if(written > 0 && (size_t)written < remaining) {
      ptr += written;
      remaining -= written;
    } else {
      break;
    }
    sep = "\n";
  }

  // 2. Global registers: 99 down to 0
  for(int32_t regist = 99; regist >= 0; --regist) {
    formatRegisterString(regist, regVal, sizeof(regVal));
    int written = snprintf(ptr, remaining, "\nR%02d = %s", regist, regVal);
    if(written > 0 && (size_t)written < remaining) {
      ptr += written;
      remaining -= written;
    } else {
      break;
    }
  }

  // 3. Local registers: currentNumberOfLocalRegisters - 1 down to 0
  if(currentSubroutineLevelData != NULL) {
    for(int32_t regist = currentNumberOfLocalRegisters - 1; regist >= 0; --regist) {
      formatRegisterString(FIRST_LOCAL_REGISTER + regist, regVal, sizeof(regVal));
      int written = snprintf(ptr, remaining, "\nR.%02d = %s", regist, regVal);
      if(written > 0 && (size_t)written < remaining) {
        ptr += written;
        remaining -= written;
      } else {
        break;
      }
    }
  }

  // 4. Statistical sums: SR00..SR26
  if(statisticalSumsPointer != NULL) {
    TO_QSPI const char * const StatSumNames[NUMBER_OF_STATISTICAL_SUMS] = {
      /* 0*/ "n             ",
      /* 1*/ STD_SIGMA "(x)          ",
      /* 2*/ STD_SIGMA "(y)          ",
      /* 3*/ STD_SIGMA "(x" STD_SUP_2 ")         ",
      /* 4*/ STD_SIGMA "(x" STD_SUP_2 "y)        ",
      /* 5*/ STD_SIGMA "(y" STD_SUP_2 ")         ",
      /* 6*/ STD_SIGMA "(xy)         ",
      /* 7*/ STD_SIGMA "(ln(x)" STD_CROSS "ln(y))",
      /* 8*/ STD_SIGMA "(ln(x))      ",
      /* 9*/ STD_SIGMA "(ln" STD_SUP_2 "(x))     ",
      /*10*/ STD_SIGMA "(y ln(x))    ",
      /*11*/ STD_SIGMA "(ln(y))      ",
      /*12*/ STD_SIGMA "(ln" STD_SUP_2 "(y))     ",
      /*13*/ STD_SIGMA "(x ln(y))    ",
      /*14*/ STD_SIGMA "(ln(y)/x)    ",
      /*15*/ STD_SIGMA "(x" STD_SUP_2 "/y)       ",
      /*16*/ STD_SIGMA "(1/x)        ",
      /*17*/ STD_SIGMA "(1/x" STD_SUP_2 ")       ",
      /*18*/ STD_SIGMA "(x/y)        ",
      /*19*/ STD_SIGMA "(1/y)        ",
      /*20*/ STD_SIGMA "(1/y" STD_SUP_2 ")       ",
      /*21*/ STD_SIGMA "(x" STD_SUP_3 ")         ",
      /*22*/ STD_SIGMA "(x" STD_SUP_4 ")         ",
      /*23*/ "x min         ",
      /*24*/ "x max         ",
      /*25*/ "y min         ",
      /*26*/ "y max         "
    };

    char sumName[40];
    char sumUtf8[128];
    char tmpRealStr[1024];
    for(int32_t sum = 0; sum < NUMBER_OF_STATISTICAL_SUMS; sum++) {
      strcpy(sumName, StatSumNames[sum]);
      stringToUtf8(sumName, (uint8_t *)sumUtf8);

      realToString(statisticalSumsPointer + sum, tmpRealStr);
      if(strchr(tmpRealStr, '.') == NULL && strchr(tmpRealStr, 'E') == NULL) {
        strcat(tmpRealStr, ".");
      }

      int written = snprintf(ptr, remaining, "\nSR%02d = %s = %s", sum, sumUtf8, tmpRealStr);
      if(written > 0 && (size_t)written < remaining) {
        ptr += written;
        remaining -= written;
      } else {
        break;
      }
    }
  }

  return result;
}