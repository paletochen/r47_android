#include "c47.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <strings.h>
#include <ctype.h>
#include <android/log.h>
#include "display.h"
#include "fonts.h"
#include "mathematics/toPolar.h"
#include "programming/manage.h"
#include "programming/decode.h"
#include "programming/lblGtoXeq.h"
#include "memory.h"
#include "sort.h"

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

void c47StringToReadableUtf8(const char *src, char *dest, size_t destSize) {
  if(!src || !dest || destSize == 0) {
    if(dest && destSize > 0) {
      dest[0] = '\0';
    }
    return;
  }

  char *d = dest;
  char *end = dest + destSize - 1;

  while(*src && d < end) {
    uint8_t b1 = (uint8_t)*src;
    if(b1 & 0x80) {
      uint8_t b2 = (uint8_t)*(src + 1);
      if(b2 == 0) {
        *d++ = '?';
        src++;
        continue;
      }

      // 1. Multi-byte combinations in C47 font
      if(b1 == 0x80 && (b2 == 0xd7 || b2 == 0xb7) && (uint8_t)*(src + 2) == 0xa4 && (uint8_t)*(src + 3) == 0x7d) {
        // PRODUCT_SIGN STD_SUB_10 followed by superscript exponent digits in _decodeNumeral -> "e..."
        const char *pExp = src + 4;
        if((uint8_t)pExp[0] == 0xa1 && ((uint8_t)pExp[1] == 0x6b || ((uint8_t)pExp[1] >= 0x60 && (uint8_t)pExp[1] <= 0x69))) {
          if(d < end) {
            *d++ = 'e';
          }
          src += 4;
          if((uint8_t)src[0] == 0xa1 && (uint8_t)src[1] == 0x6b) {
            if(d < end) {
              *d++ = '-';
            }
            src += 2;
          }
          while((uint8_t)src[0] == 0xa1 && (uint8_t)src[1] >= 0x60 && (uint8_t)src[1] <= 0x69) {
            if(d < end) {
              *d++ = (char)('0' + ((uint8_t)src[1] - 0x60));
            }
            src += 2;
          }
          continue;
        }
      }
      if(b1 == 0xa2 && b2 == 0x1c && (uint8_t)*(src + 2) == 0x82 && (uint8_t)*(src + 3) == 0x32) {
        // STD_xTH_ROOT STD_y_UNDER_ROOT -> "ˣ√y"
        const char *rep = "ˣ√y";
        size_t rlen = strlen(rep);
        if(d + rlen <= end) {
          memcpy(d, rep, rlen);
          d += rlen;
        }
        src += 4;
        continue;
      }
      if(b1 == 0xa2 && b2 == 0x1a && (uint8_t)*(src + 2) == 0x83 && (uint8_t)*(src + 3) == 0x7f) {
        // STD_SQUARE_ROOT STD_x_UNDER_ROOT -> "√x"
        const char *rep = "√x";
        size_t rlen = strlen(rep);
        if(d + rlen <= end) {
          memcpy(d, rep, rlen);
          d += rlen;
        }
        src += 4;
        continue;
      }
      if(b1 == 0xa2 && b2 == 0x1b && (uint8_t)*(src + 2) == 0x83 && (uint8_t)*(src + 3) == 0x7f) {
        // STD_CUBE_ROOT STD_x_UNDER_ROOT -> "∛x"
        const char *rep = "∛x";
        size_t rlen = strlen(rep);
        if(d + rlen <= end) {
          memcpy(d, rep, rlen);
          d += rlen;
        }
        src += 4;
        continue;
      }

      // 2. Two-byte mappings
      const char *rep = NULL;
      if(b1 == 0xa2 && b2 == 0x1c) {
        rep = "ˣ√";
      }
      else if(b1 == 0x82 && b2 == 0x32) {
        rep = "y";
      }
      else if(b1 == 0x83 && b2 == 0x7f) {
        rep = "x";
      }
      else if(b1 == 0xa2 && b2 == 0x1a) {
        rep = "√";
      }
      else if(b1 == 0xa2 && b2 == 0x1b) {
        rep = "∛";
      }
      else if(b1 == 0x82 && b2 == 0xe3) {
        rep = "ˣ";
      }
      else if(b1 == 0xa1 && b2 >= 0x60 && b2 <= 0x69) {
        static const char *const sups[10] = {"⁰", "¹", "²", "³", "⁴", "⁵", "⁶", "⁷", "⁸", "⁹"};
        rep = sups[b2 - 0x60];
      }
      else if(b1 == 0xa1 && b2 == 0x6a) {
        rep = "⁺";
      }
      else if(b1 == 0xa1 && b2 == 0x6b) {
        rep = "⁻";
      }
      else if(b1 == 0xa0 && b2 >= 0x80 && b2 <= 0x89) {
        static const char *const subs[10] = {"₀", "₁", "₂", "₃", "₄", "₅", "₆", "₇", "₈", "₉"};
        rep = subs[b2 - 0x80];
      }
      else if(b1 == 0xa0 && b2 == 0x8a) {
        rep = "₊";
      }
      else if(b1 == 0xa0 && b2 == 0x8b) {
        rep = "₋";
      }
      else if(b1 == 0xa1 && b2 == 0xc4) {
        rep = "⇄";
      }
      else if(b1 == 0xa1 && b2 == 0x94) {
        rep = "↔";
      }
      else if(b1 == 0xa1 && b2 == 0x90) {
        rep = "←";
      }
      else if(b1 == 0xa1 && b2 == 0x91) {
        rep = "↑";
      }
      else if(b1 == 0xa1 && b2 == 0x92) {
        rep = "→";
      }
      else if(b1 == 0xa1 && b2 == 0x93) {
        rep = "↓";
      }
      else if(b1 == 0x80 && b2 == 0xd7) {
        rep = "×";
      }
      else if(b1 == 0x80 && b2 == 0xf7) {
        rep = "÷";
      }
      else if(b1 == 0xa2 && b2 == 0x04) {
        rep = "≠";
      }
      else if(b1 == 0xa2 && b2 == 0x02) {
        rep = "≤";
      }
      else if(b1 == 0xa2 && b2 == 0x03) {
        rep = "≥";
      }
      else if(b1 == 0xa2 && b2 == 0x06) {
        rep = "≈";
      }
      else if((b1 == 0x83 && b2 == 0xc0) || (b1 == 0xac && b2 == 0x64) || (b1 == 0xac && b2 == 0x65)) {
        rep = "π";
      }
      else if(b1 == 0x83 && (b2 == 0xb1 || b2 == 0x80)) {
        rep = "α";
      }
      else if(b1 == 0xa1 && b2 == 0x47) {
        rep = "e";
      }
      else if(b1 == 0xa2 && b2 == 0x1e) {
        rep = "∞";
      }
      else if(b1 == 0x80 && b2 == 0xb1) {
        rep = "±";
      }
      else if(b1 == 0x80 && b2 == 0xb0) {
        rep = "°";
      }
      else if(b1 == 0xa2 && b2 == 0x2b) {
        rep = "∫";
      }
      else if(b1 == 0xa1 && b2 == 0xb5) {
        rep = "↵";
      }
      else if(b1 == 0xa4 && b2 >= 0x60 && b2 <= 0x6f) {
        static const char *const bases[16] = {
          "#1", "#2", "#3", "#4", "#5", "#6", "#7", "#8",
          "#9", "#10", "#11", "#12", "#13", "#14", "#15", "#16"
        };
        rep = bases[b2 - 0x60];
      }
      else if(b1 == 0xa4 && b2 == 0xea) {
        rep = "#0";
      }
      else if(b1 == 0xa3 && b2 == 0x99) {
        rep = "🖨";
      }
      else if(b1 == 0xa9 && b2 == 0xf0) {
        rep = "yˣ";
      }
      else if(b1 == 0xa9 && b2 == 0xf1) {
        rep = "_f^g";
      }
      else if(b1 == 0xa1 && b2 == 0x48) {
        rep = "i";
      }
      else if(b1 == 0xa1 && b2 == 0x49) {
        rep = "j";
      }
      else if(b1 == 0x84 && b2 == 0x52) {
        rep = "_d⁻¹";
      }
      else if(b1 == 0x83 && b2 == 0x79) {
        rep = "x̂";
      }
      else if(b1 == 0xa2 && b2 == 0x95) {
        rep = "⊕";
      }
      else if(b1 == 0x83 && b2 == 0xd5) {
        rep = "ϕ";
      }
      else if(b1 == 0x83 && b2 == 0xc6) {
        rep = "φ";
      }
      else if(b1 == 0x83 && b2 == 0x96) {
        rep = "ζ";
      }
      else if(b1 == 0xa4 && b2 == 0x2e) {
        rep = "_(US)";
      }
      else if(b1 == 0xa4 && b2 == 0x2d) {
        rep = "_(UK)";
      }
      else if(b1 == 0xa4 && b2 == 0x8f) {
        rep = "ⁿ";
      }
      else if(b1 == 0xa1 && b2 == 0x1d) {
        rep = "ℝ";
      }
      else if(b1 == 0xa1 && b2 == 0x02) {
        rep = "ℂ";
      }
      else if(b1 == 0xa1 && b2 == 0x24) {
        rep = "ℤ";
      }
      else if((b1 == 0x80 && b2 == 0xb7) || (b1 == 0xa2 && b2 == 0xc5)) {
        rep = "⋅";
      }
      else if(b1 == 0xa0 && b2 >= 0x03 && b2 <= 0x0a) {
        rep = " ";
      }

      if(rep != NULL) {
        size_t rlen = strlen(rep);
        if(d + rlen <= end) {
          memcpy(d, rep, rlen);
          d += rlen;
        }
        src += 2;
        continue;
      }

      uint32_t codePoint = ((uint32_t)(b1 & 0x7F) << 8) | b2;
      uint8_t utfTmp[8];
      codePointToUtf8(codePoint, utfTmp);
      size_t tlen = strlen((char *)utfTmp);
      if(d + tlen <= end) {
        memcpy(d, utfTmp, tlen);
        d += tlen;
      }
      src += 2;
    }
    else {
      *d++ = *src++;
    }
  }

  *d = '\0';
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

  // Convert core glyphs to valid readable UTF-8
  static char utf8Buf[32768];
  c47StringToReadableUtf8(coreBuf, utf8Buf, sizeof(utf8Buf));
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

bool isProgramMode(void) {
  return (calcMode == CM_PEM);
}

const char *stripProgramLinePrefix(const char *line) {
  while(*line == ' ' || *line == '\t') {
    line++;
  }
  const char *p = line;
  while(*p >= '0' && *p <= '9') {
    p++;
  }
  if(p > line) {
    if(*p == ':') {
      p++;
      while(*p == ' ' || *p == '\t') {
        p++;
      }
      return p;
    }
    if(*p == '.' && (p - line >= 2) && (*(p + 1) == ' ' || *(p + 1) == '\t')) {
      const char *after = p + 1;
      while(*after == ' ' || *after == '\t') {
        after++;
      }
      if(*after != '\0') {
        return after;
      }
    }
    if(line[0] == '0' && (p - line >= 2) && (*p == ' ' || *p == '\t')) {
      const char *next = p;
      while(*next == ' ' || *next == '\t') {
        next++;
      }
      if((*next >= 'A' && *next <= 'Z') || (*next >= 'a' && *next <= 'z') ||
         *next == '"' || *next == '\'' || (uint8_t)*next >= 0x80) {
        return next;
      }
    }
  }
  return line;
}

static bool hasProgramLinePrefix(const char *line) {
  while(*line == ' ' || *line == '\t') {
    line++;
  }
  const char *stripped = stripProgramLinePrefix(line);
  return (stripped != line);
}

static bool isProgramHeaderLine(const char *s);

static void normalizeProgramText(const char *src, char *dest, size_t destSize);

static bool documentHasLineNumbers(const char *text) {
  if(text == NULL) {
    return false;
  }
  const char *p = text;
  while(*p) {
    const char *lineStart = p;
    while(*p && *p != '\n' && *p != '\r') {
      p++;
    }
    size_t lineLen = (size_t)(p - lineStart);
    if(*p == '\r') {
      p++;
    }
    if(*p == '\n') {
      p++;
    }
    if(lineLen == 0) {
      continue;
    }
    char rawLine[1024];
    size_t copyLen = (lineLen < sizeof(rawLine) - 1) ? lineLen : sizeof(rawLine) - 1;
    memcpy(rawLine, lineStart, copyLen);
    rawLine[copyLen] = '\0';

    char temp[1024];
    normalizeProgramText(rawLine, temp, sizeof(temp));

    const char *s = temp;
    while(*s == ' ' || *s == '\t') {
      s++;
    }
    if(*s == '\0') {
      continue;
    }
    if(*s == '@' || *s == '#' || *s == ';' || (s[0] == '/' && s[1] == '/')) {
      continue;
    }
    if(*s == '{') {
      continue;
    }
    if(isProgramHeaderLine(s)) {
      continue;
    }

    return hasProgramLinePrefix(s);
  }
  return false;
}

char *getCurrentProgramString(void) {
  if(calcMode != CM_PEM) {
    return strdup("");
  }
  defineCurrentProgramFromCurrentStep();
  if(numberOfPrograms == 0) {
    return strdup("");
  }
  uint8_t *step = programList[currentProgramNumber - 1].instructionPointer;
  if(step == NULL || isAtEndOfPrograms(step)) {
    return strdup("");
  }

  size_t capacity = 8192;
  size_t length = 0;
  char *buffer = (char *)malloc(capacity);
  if(buffer == NULL) {
    return strdup("");
  }
  buffer[0] = '\0';

  char utf8Buf[TMP_STR_LENGTH * 2];
  char lineBuf[TMP_STR_LENGTH * 2 + 32];

  for(uint16_t line = 1; line <= 9999; ++line) {
    decodeOneStep(step);
    c47StringToReadableUtf8(tmpString, utf8Buf, sizeof(utf8Buf));
    int written = snprintf(lineBuf, sizeof(lineBuf), "%04u: %s\n", line, utf8Buf);
    if(written > 0) {
      if(length + (size_t)written + 1 > capacity) {
        capacity = (capacity * 2) + (size_t)written + 1024;
        char *newBuf = (char *)realloc(buffer, capacity);
        if(newBuf == NULL) {
          break;
        }
        buffer = newBuf;
      }
      memcpy(buffer + length, lineBuf, (size_t)written);
      length += (size_t)written;
      buffer[length] = '\0';
    }
    if(isAtEndOfProgram(step) || (*step == 255 && *(step + 1) == 255)) {
      break;
    }
    uint8_t *next = findNextStep(step);
    if(next == NULL || next == step) {
      break;
    }
    step = next;
  }
  return buffer;
}

typedef struct {
  uint8_t bytes[260];
  uint16_t size;
} EncodedStep;


static bool isHexOrDecDigitByte(uint8_t c) {
  return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

static void normalizeProgramText(const char *src, char *dest, size_t destSize) {
  if(dest == NULL || destSize == 0) {
    return;
  }
  if(src == NULL) {
    dest[0] = '\0';
    return;
  }

  // Pass 1: unwrap C47 RTF line markup ({\rtf1...}, \par, \sub, \super, \nosupersub, \uNNNN?) if present
  const char *trimSrc = src;
  while(*trimSrc == ' ' || *trimSrc == '\t') {
    trimSrc++;
  }
  if(strncmp(trimSrc, "{\\rtf", 5) == 0 || strcmp(trimSrc, "}}") == 0) {
    dest[0] = '\0';
    return;
  }

  char rtfBuf[1024];
  size_t ri = 0;
  const uint8_t *r = (const uint8_t *)src;
  while(*r && ri < sizeof(rtfBuf) - 6) {
    if(r[0] == '\\') {
      if(strncmp((const char *)r, "\\par", 4) == 0 &&
         (r[4] == '\0' || r[4] == ' ' || r[4] == '\t' || r[4] == '\r' || r[4] == '\n' || r[4] == '\\')) {
        r += 4;
        continue;
      }
      if(strncmp((const char *)r, "\\sub ", 5) == 0) {
        r += 5;
        continue;
      }
      if(strncmp((const char *)r, "\\super ", 7) == 0) {
        r += 7;
        continue;
      }
      if(strncmp((const char *)r, "\\nosupersub ", 12) == 0) {
        r += 12;
        continue;
      }
      if(strncmp((const char *)r, "\\nosupersub", 11) == 0 && (r[11] == '\0' || r[11] == '\\')) {
        r += 11;
        continue;
      }
      if(r[1] == 'u' && ((r[2] >= '0' && r[2] <= '9') || r[2] == '-')) {
        const uint8_t *numP = r + 2;
        bool neg = false;
        if(*numP == '-') {
          neg = true;
          numP++;
        }
        int32_t uval = 0;
        while(*numP >= '0' && *numP <= '9') {
          uval = uval * 10 + (*numP - '0');
          numP++;
        }
        if(*numP == '?') {
          numP++;
          if(neg) {
            uval = -uval;
          }
          uint32_t cp = (uint32_t)((uint16_t)(uval & 0xffff));
          uint8_t utfTmp[8];
          codePointToUtf8(cp, utfTmp);
          size_t tlen = strlen((char *)utfTmp);
          if(ri + tlen < sizeof(rtfBuf) - 1) {
            memcpy(&rtfBuf[ri], utfTmp, tlen);
            ri += tlen;
          }
          r = numP;
          continue;
        }
      }
    }
    rtfBuf[ri++] = (char)*r++;
  }
  rtfBuf[ri] = '\0';

  // Pass 2: normalize Unicode spaces, Mathematical Italic characters, and C47 RTF glyphs
  size_t di = 0;
  const uint8_t *s = (const uint8_t *)rtfBuf;
  uint8_t quoteChar = 0;
  while(*s && di < destSize - 6) {
    // Strip zero-width characters (U+FEFF BOM, U+200B..U+200D)
    if(s[0] == 0xef && s[1] == 0xbb && s[2] == 0xbf) {
      s += 3;
      continue;
    }
    if(s[0] == 0xe2 && s[1] == 0x80 && s[2] >= 0x8b && s[2] <= 0x8d) {
      s += 3;
      continue;
    }

    // Track string/label quotes so string contents preserve literal spaces
    if(quoteChar != 0) {
      if(quoteChar == 0x99 || quoteChar == 0x9d) {
        if(s[0] == 0xe2 && s[1] == 0x80 && s[2] == quoteChar) {
          quoteChar = 0;
          dest[di++] = (char)*s++;
          dest[di++] = (char)*s++;
          dest[di++] = (char)*s++;
          continue;
        }
      }
      else if(s[0] == quoteChar) {
        if(s[1] == '\0' || s[1] == ' ' || s[1] == '\t' || s[1] == '\r' || s[1] == '\n' ||
           (s[1] == 0xc2 && s[2] == 0xa0) ||
           (s[1] == 0xe2 && s[2] == 0x80 && ((s[3] >= 0x80 && s[3] <= 0x8a) || s[3] == 0xaf))) {
          quoteChar = 0;
        }
      }
    }
    else {
      if(s[0] == '\'' || s[0] == '"') {
        quoteChar = s[0];
        dest[di++] = (char)*s++;
        continue;
      }
      else if(s[0] == 0xe2 && s[1] == 0x80 && (s[2] == 0x98 || s[2] == 0x9c)) {
        quoteChar = (s[2] == 0x98) ? 0x99 : 0x9d;
        dest[di++] = (char)*s++;
        dest[di++] = (char)*s++;
        dest[di++] = (char)*s++;
        continue;
      }

      // Normalize Unicode spaces outside quotes
      if(s[0] == 0xc2 && s[1] == 0xa0) { // U+00A0 NO-BREAK SPACE
        dest[di++] = ' ';
        s += 2;
        continue;
      }
      if(s[0] == 0xa0 && s[1] >= 0x03 && s[1] <= 0x0a) { // C47 font spaces
        if(s[1] == 0x08 && di > 0 && isHexOrDecDigitByte((uint8_t)dest[di - 1]) && isHexOrDecDigitByte(s[2])) {
          dest[di++] = (char)0xe2;
          dest[di++] = (char)0x80;
          dest[di++] = (char)0x88;
        }
        else {
          dest[di++] = ' ';
        }
        s += 2;
        continue;
      }
      if(s[0] == 0xe1 && s[1] == 0x9a && s[2] == 0x80) { // U+1680
        dest[di++] = ' ';
        s += 3;
        continue;
      }
      if(s[0] == 0xe2 && s[1] == 0x81 && s[2] == 0x9f) { // U+205F
        dest[di++] = ' ';
        s += 3;
        continue;
      }
      if(s[0] == 0xe3 && s[1] == 0x80 && s[2] == 0x80) { // U+3000
        dest[di++] = ' ';
        s += 3;
        continue;
      }
      if(s[0] == 0xe2 && s[1] == 0x80 && ((s[2] >= 0x80 && s[2] <= 0x8a) || s[2] == 0xaf)) {
        if((s[2] == 0x88 || s[2] == 0x89 || s[2] == 0xaf) &&
           di > 0 && isHexOrDecDigitByte((uint8_t)dest[di - 1]) && isHexOrDecDigitByte(s[3])) {
          // Digit grouping space inside a number (e.g. 900,677 7 or 1 000.77)
          dest[di++] = (char)0xe2;
          dest[di++] = (char)0x80;
          dest[di++] = (char)0x88;
          s += 3;
          continue;
        }
        if(s[2] == 0x84 || s[2] == 0x85) {
          // U+2004 / U+2005 inside a C47 command name (e.g. i LI/ℝ, PLT f, RWDOT ⋅): keep if preceded by a command letter
          if(di > 0 && dest[di - 1] != ' ' && dest[di - 1] != '\t' && dest[di - 1] != ':' && dest[di - 1] != '.') {
            dest[di++] = (char)*s++;
            dest[di++] = (char)*s++;
            dest[di++] = (char)*s++;
            continue;
          }
        }
        dest[di++] = ' ';
        s += 3;
        continue;
      }

      // C47 RTF scientific notation: ×⑽ (C3 97 E2 91 BD) or *⑽ (2A E2 91 BD) -> 'e'
      if(((s[0] == 0xc3 && s[1] == 0x97 && s[2] == 0xe2 && s[3] == 0x91 && s[4] == 0xbd) ||
          (s[0] == '*' && s[1] == 0xe2 && s[2] == 0x91 && s[3] == 0xbd)) &&
         di > 0 && ((dest[di - 1] >= '0' && dest[di - 1] <= '9') || dest[di - 1] == '.' || dest[di - 1] == ',')) {
        s += (s[0] == '*') ? 4 : 5;
        dest[di++] = 'e';
        if(s[0] == 0xe2 && ((s[1] == 0x85 && s[2] == 0xab) || (s[1] == 0x81 && s[2] == 0xbb))) { // Ⅻ or ⁻
          dest[di++] = '-';
          s += 3;
        }
        else if(s[0] == 0xe2 && ((s[1] == 0x85 && s[2] == 0xaa) || (s[1] == 0x81 && s[2] == 0xba))) { // Ⅺ or ⁺
          dest[di++] = '+';
          s += 3;
        }
        continue;
      }

      // C47 RTF base suffixes: ①..⑯ (U+2460..U+246F: E2 91 A0..AF) and ⓪ (U+24EA: E2 93 AA)
      if(s[0] == 0xe2 && s[1] == 0x91 && s[2] >= 0xa0 && s[2] <= 0xaf &&
         di > 0 && isHexOrDecDigitByte((uint8_t)dest[di - 1])) {
        int baseNum = (s[2] - 0xa0) + 1;
        dest[di++] = '#';
        if(baseNum >= 10) {
          dest[di++] = '1';
          dest[di++] = (char)('0' + (baseNum - 10));
        }
        else {
          dest[di++] = (char)('0' + baseNum);
        }
        s += 3;
        continue;
      }
      if(s[0] == 0xe2 && s[1] == 0x93 && s[2] == 0xaa &&
         di > 0 && isHexOrDecDigitByte((uint8_t)dest[di - 1])) {
        dest[di++] = '#';
        dest[di++] = '0';
        s += 3;
        continue;
      }
    }

    // Mathematical Italic Capital A..Z (U+1D434..U+1D44D: F0 9D 90 B4..BF and F0 9D 91 80..8D)
    if(s[0] == 0xf0 && s[1] == 0x9d && s[2] == 0x90 && s[3] >= 0xb4 && s[3] <= 0xbf) {
      dest[di++] = (char)('A' + (s[3] - 0xb4));
      s += 4;
    }
    else if(s[0] == 0xf0 && s[1] == 0x9d && s[2] == 0x91 && s[3] >= 0x80 && s[3] <= 0x8d) {
      dest[di++] = (char)('M' + (s[3] - 0x80));
      s += 4;
    }
    // Mathematical Italic Small a..z (U+1D44E..U+1D467: F0 9D 91 8E..A7, except h at U+210E)
    else if(s[0] == 0xf0 && s[1] == 0x9d && s[2] == 0x91 && s[3] >= 0x8e && s[3] <= 0xa7) {
      dest[di++] = (char)('a' + (s[3] - 0x8e));
      s += 4;
    }
    else if(s[0] == 0xe2 && s[1] == 0x84 && s[2] == 0x8e) { // ℎ (U+210E)
      dest[di++] = 'h';
      s += 3;
    }
    else if(s[0] == 0xe2 && s[1] == 0x85 && s[2] == 0x87) { // ⅇ (U+2147, C47 STD_EulerE)
      dest[di++] = 'e';
      s += 3;
    }
    else if(s[0] == 0xf0 && s[1] == 0x9d && s[2] == 0x92 && (s[3] == 0xbe || s[3] == 0xbf)) { // 𝒾, 𝒿 (U+1D4BE..U+1D4BF)
      dest[di++] = (s[3] == 0xbe) ? 'i' : 'j';
      s += 4;
    }
    else if(s[0] == 0xe2 && s[1] == 0x85 && (s[2] == 0x88 || s[2] == 0x89)) { // ⅈ, ⅉ (U+2148..U+2149)
      dest[di++] = (s[2] == 0x88) ? 'i' : 'j';
      s += 3;
    }
    else if(s[0] == 0xf0 && s[1] == 0x9d && s[2] == 0x9c && s[3] == 0x8b) { // 𝜋 (U+1D70B)
      dest[di++] = 0xcf;
      dest[di++] = 0x80;
      s += 4;
    }
    else if(s[0] == 0xe2 && s[1] == 0x88 && s[2] == 0x92) { // − (minus)
      dest[di++] = '-';
      s += 3;
    }
    else if(s[0] == 0xe2 && s[1] == 0x80 && (s[2] == 0x93 || s[2] == 0x94)) { // – or —
      dest[di++] = '-';
      s += 3;
    }
    else if(*s == '\r') {
      dest[di++] = ' ';
      s++;
    }
    else {
      dest[di++] = (char)*s++;
    }
  }
  dest[di] = '\0';
}

static void flattenSubSupCommand(const char *src, char *dest, size_t destSize) {
  if(destSize == 0) {
    return;
  }
  size_t di = 0;
  const uint8_t *s = (const uint8_t *)src;
  while(*s && di < destSize - 6) {
    // Strip rejig subscript wrapper "_(" and matching ")"
    if(s[0] == '_' && s[1] == '(') {
      s += 2;
      while(*s && *s != ')' && di < destSize - 6) {
        if(*s != '_' && *s != '^' && *s != ' ' && *s != '\t') {
          dest[di++] = (char)*s;
        }
        s++;
      }
      if(*s == ')') {
        s++;
      }
      continue;
    }
    if(s[0] == '_' || s[0] == '^' || s[0] == ' ' || s[0] == '\t') {
      s++;
      continue;
    }
    // Strip C47 font spaces (0xA0 0x03..0x0A) and UTF-8 spaces
    if(s[0] == 0xa0 && s[1] >= 0x03 && s[1] <= 0x0a) {
      s += 2;
      continue;
    }
    if(s[0] == 0xe2 && s[1] == 0x80 && ((s[2] >= 0x80 && s[2] <= 0x8a) || s[2] == 0xaf)) {
      s += 3;
      continue;
    }
    if(s[0] == 0xc2 && s[1] == 0xa0) {
      s += 2;
      continue;
    }
    // C47 2-byte font glyphs for subscripts, superscripts, and special symbols
    if(s[0] == 0xa9 && s[1] == 0xf0) { // STD_YX
      dest[di++] = 'y';
      dest[di++] = 'x';
      s += 2;
    }
    else if(s[0] == 0xa9 && s[1] == 0xf1) { // STD_fg
      dest[di++] = 'f';
      dest[di++] = 'g';
      s += 2;
    }
    else if(s[0] == 0xa1 && s[1] == 0x48) { // STD_op_i
      dest[di++] = 'i';
      s += 2;
    }
    else if(s[0] == 0xa1 && s[1] == 0x49) { // STD_op_j
      dest[di++] = 'j';
      s += 2;
    }
    else if(s[0] == 0x84 && s[1] == 0x52) { // STD_D_MINUS1
      dest[di++] = 'd';
      dest[di++] = '-';
      dest[di++] = '1';
      s += 2;
    }
    else if(s[0] == 0x83 && s[1] == 0x79) { // STD_x_CIRC
      dest[di++] = 'x';
      dest[di++] = 'h';
      dest[di++] = 'a';
      dest[di++] = 't';
      s += 2;
    }
    else if(s[0] == 0xa4 && s[1] == 0x2e) { // STD_US
      dest[di++] = 'U';
      dest[di++] = 'S';
      s += 2;
    }
    else if(s[0] == 0xa4 && s[1] == 0x2d) { // STD_UK
      dest[di++] = 'U';
      dest[di++] = 'K';
      s += 2;
    }
    else if((s[0] == 0x83 && (s[1] == 0xc6 || s[1] == 0xd5)) ||
            (s[0] == 0xcf && (s[1] == 0x86 || s[1] == 0x95))) { // STD_phi, STD_phi_m, φ, ϕ
      dest[di++] = 'p';
      dest[di++] = 'h';
      dest[di++] = 'i';
      s += 2;
    }
    else if((s[0] == 0x83 && (s[1] == 0x96 || s[1] == 0xb6)) ||
            (s[0] == 0xce && (s[1] == 0x96 || s[1] == 0xb6))) { // STD_ZETA, STD_zeta, Ζ, ζ
      dest[di++] = 'z';
      dest[di++] = 'e';
      dest[di++] = 't';
      dest[di++] = 'a';
      s += 2;
    }
    else if((s[0] == 0x80 && s[1] == 0xb7) || (s[0] == 0xa2 && s[1] == 0xc5) ||
            (s[0] == 0xc2 && s[1] == 0xb7) || (s[0] == 0xe2 && s[1] == 0x8b && s[2] == 0x85)) { // STD_DOT, STD_WDOT, ·, ⋅
      dest[di++] = '.';
      s += (s[0] == 0xe2) ? 3 : 2;
    }
    else if(s[0] == 0xa4 && s[1] >= 0x82 && s[1] <= 0x9b) { // STD_SUP_a..z
      dest[di++] = (char)('a' + (s[1] - 0x82));
      s += 2;
    }
    else if(s[0] == 0xa4 && s[1] >= 0x9c && s[1] <= 0xb5) { // STD_SUB_a..z
      dest[di++] = (char)('a' + (s[1] - 0x9c));
      s += 2;
    }
    else if(s[0] == 0xa4 && s[1] >= 0xb6 && s[1] <= 0xcf) { // STD_SUP_A..Z
      dest[di++] = (char)('A' + (s[1] - 0xb6));
      s += 2;
    }
    else if(s[0] == 0xa4 && s[1] >= 0xd0 && s[1] <= 0xe9) { // STD_SUB_A..Z
      dest[di++] = (char)('A' + (s[1] - 0xd0));
      s += 2;
    }
    else if(s[0] == 0xa0 && s[1] >= 0x80 && s[1] <= 0x89) { // STD_SUB_0..9
      dest[di++] = (char)('0' + (s[1] - 0x80));
      s += 2;
    }
    else if(s[0] == 0xa0 && s[1] == 0x8a) { // STD_SUB_PLUS
      dest[di++] = '+';
      s += 2;
    }
    else if(s[0] == 0xa0 && s[1] == 0x8b) { // STD_SUB_MINUS
      dest[di++] = '-';
      s += 2;
    }
    else if(s[0] == 0xa1 && s[1] >= 0x60 && s[1] <= 0x69) { // STD_SUP_0..9
      dest[di++] = (char)('0' + (s[1] - 0x60));
      s += 2;
    }
    else if(s[0] == 0xa1 && s[1] == 0x6a) { // STD_SUP_PLUS
      dest[di++] = '+';
      s += 2;
    }
    else if(s[0] == 0xa1 && s[1] == 0x6b) { // STD_SUP_MINUS
      dest[di++] = '-';
      s += 2;
    }
    else if(s[0] == 0x82 && s[1] == 0xe3) { // STD_SUP_BOLD_x
      dest[di++] = 'x';
      s += 2;
    }
    else if((s[0] == 0xa1 && s[1] == 0x92) || (s[0] == 0xe2 && s[1] == 0x86 && s[2] == 0x92)) { // →
      dest[di++] = '-';
      dest[di++] = '>';
      s += (s[0] == 0xa1) ? 2 : 3;
    }
    else if((s[0] == 0xa1 && s[1] == 0xd2) || (s[0] == 0xe2 && s[1] == 0x87 && s[2] == 0x92)) { // ⇒
      dest[di++] = '=';
      dest[di++] = '>';
      s += (s[0] == 0xa1) ? 2 : 3;
    }
    else if((s[0] == 0xa1 && s[1] == 0xc4) || (s[0] == 0xe2 && s[1] == 0x87 && s[2] == 0x84)) { // ⇄
      dest[di++] = '<';
      dest[di++] = '>';
      s += (s[0] == 0xa1) ? 2 : 3;
    }
    // UTF-8 subscript digits ₀..₉ (E2 82 80..89), ₊ (8A), ₋ (8B)
    else if(s[0] == 0xe2 && s[1] == 0x82 && s[2] >= 0x80 && s[2] <= 0x89) {
      dest[di++] = (char)('0' + (s[2] - 0x80));
      s += 3;
    }
    else if(s[0] == 0xe2 && s[1] == 0x82 && s[2] == 0x8a) {
      dest[di++] = '+';
      s += 3;
    }
    else if(s[0] == 0xe2 && s[1] == 0x82 && s[2] == 0x8b) {
      dest[di++] = '-';
      s += 3;
    }
    // UTF-8 superscript digits and signs: ⁰ (E2 81 B0), ¹ (C2 B9), ² (C2 B2), ³ (C2 B3), ⁴..⁹ (E2 81 B4..B9), ⁺ (BA), ⁻ (BB), ⁿ (BF)
    else if(s[0] == 0xe2 && s[1] == 0x81 && s[2] == 0xb0) {
      dest[di++] = '0';
      s += 3;
    }
    else if(s[0] == 0xc2 && s[1] == 0xb9) {
      dest[di++] = '1';
      s += 2;
    }
    else if(s[0] == 0xc2 && s[1] == 0xb2) {
      dest[di++] = '2';
      s += 2;
    }
    else if(s[0] == 0xc2 && s[1] == 0xb3) {
      dest[di++] = '3';
      s += 2;
    }
    else if(s[0] == 0xe2 && s[1] == 0x81 && s[2] >= 0xb4 && s[2] <= 0xb9) {
      dest[di++] = (char)('0' + (s[2] - 0xb0));
      s += 3;
    }
    else if(s[0] == 0xe2 && s[1] == 0x81 && s[2] == 0xba) {
      dest[di++] = '+';
      s += 3;
    }
    else if(s[0] == 0xe2 && s[1] == 0x81 && s[2] == 0xbb) {
      dest[di++] = '-';
      s += 3;
    }
    else if(s[0] == 0xe2 && s[1] == 0x81 && s[2] == 0xbf) {
      dest[di++] = 'n';
      s += 3;
    }
    // UTF-8 subscript letters ₐ ₑ ₒ ₓ ₔ ₕ ₖ ₗ ₘ ₙ ₚ ₛ ₜ (E2 82 90..9C)
    else if(s[0] == 0xe2 && s[1] == 0x82 && s[2] >= 0x90 && s[2] <= 0x9c) {
      static const char subMap[13] = {'a', 'e', 'o', 'x', 'e', 'h', 'k', 'l', 'm', 'n', 'p', 's', 't'};
      dest[di++] = subMap[s[2] - 0x90];
      s += 3;
    }
    // UTF-8 subscript ᵢ ᵣ ᵤ ᵥ (E1 B5 A2..A5)
    else if(s[0] == 0xe1 && s[1] == 0xb5 && s[2] >= 0xa2 && s[2] <= 0xa5) {
      static const char subMap2[4] = {'i', 'r', 'u', 'v'};
      dest[di++] = subMap2[s[2] - 0xa2];
      s += 3;
    }
    // UTF-8 superscript ᵀ (E1 B5 80), ᴼ (E1 B4 BC), ᴾ (E1 B4 BE), ˣ (CB A3), ʸ (CB B8)
    else if(s[0] == 0xe1 && s[1] == 0xb5 && s[2] == 0x80) {
      dest[di++] = 'T';
      s += 3;
    }
    else if(s[0] == 0xe1 && s[1] == 0xb4 && s[2] == 0xbc) {
      dest[di++] = 'O';
      s += 3;
    }
    else if(s[0] == 0xe1 && s[1] == 0xb4 && s[2] == 0xbe) {
      dest[di++] = 'P';
      s += 3;
    }
    else if(s[0] == 0xcb && s[1] == 0xa3) {
      dest[di++] = 'x';
      s += 2;
    }
    else if(s[0] == 0xcb && s[1] == 0xb8) {
      dest[di++] = 'y';
      s += 2;
    }
    else if(s[0] == 0xcc && s[1] == 0x82) { // Combining circumflex (x̂)
      dest[di++] = 'h';
      dest[di++] = 'a';
      dest[di++] = 't';
      s += 2;
    }
    else if(s[0] >= 0x80 && s[0] < 0xc0 && s[1] != 0) {
      // Other 2-byte C47 font glyph -> convert to UTF-8
      uint32_t cp = ((uint32_t)(s[0] & 0x7f) << 8) | s[1];
      uint8_t utfTmp[8];
      codePointToUtf8(cp, utfTmp);
      size_t tlen = strlen((char *)utfTmp);
      if(di + tlen < destSize) {
        memcpy(&dest[di], utfTmp, tlen);
        di += tlen;
      }
      s += 2;
    }
    else if(s[0] >= 0xc0) {
      // Multi-byte UTF-8 character: copy complete sequence so continuation bytes (0x80..0xbf) are not split
      size_t ulen = 1;
      if((s[0] & 0xe0) == 0xc0 && s[1] != 0) {
        ulen = 2;
      }
      else if((s[0] & 0xf0) == 0xe0 && s[1] != 0 && s[2] != 0) {
        ulen = 3;
      }
      else if((s[0] & 0xf8) == 0xf0 && s[1] != 0 && s[2] != 0 && s[3] != 0) {
        ulen = 4;
      }
      for(size_t k = 0; k < ulen && di < destSize - 1; k++) {
        dest[di++] = (char)*s++;
      }
    }
    else {
      dest[di++] = (char)*s++;
    }
  }
  dest[di] = '\0';
}

static int findItemOpcode(const char *cmd) {
  if(cmd == NULL || cmd[0] == '\0') {
    return -1;
  }
  if(strcasecmp(cmd, "VARMENU") == 0) {
    return 1630; // ITM_VARMNU
  }

  // Printer commands (Print..., PRT..., 🖨..., \xa3\x99...)
  const char *prtSfx = NULL;
  if(strncasecmp(cmd, "Print", 5) == 0) {
    prtSfx = cmd + 5;
  }
  else if(strncasecmp(cmd, "PRT", 3) == 0) {
    prtSfx = cmd + 3;
  }
  else if((uint8_t)cmd[0] == 0xf0 && (uint8_t)cmd[1] == 0x9f && (uint8_t)cmd[2] == 0x96 && (uint8_t)cmd[3] == 0xa8) { // 🖨
    prtSfx = cmd + 4;
  }
  else if((uint8_t)cmd[0] == 0xa3 && (uint8_t)cmd[1] == 0x99) { // STD_PRINTER
    prtSfx = cmd + 2;
  }

  if(prtSfx != NULL) {
    while(*prtSfx == ' ' || *prtSfx == '\t') {
      prtSfx++;
    }
    if(strcasecmp(prtSfx, "ON") == 0) {
      return 2687; // ITM_PRON (STD_PRINTER "ON")
    }
    if(strcasecmp(prtSfx, "OFF") == 0) {
      return 2688; // ITM_PROFF (STD_PRINTER "OFF")
    }
    if(strcasecmp(prtSfx, "ADV") == 0) {
      return 1708; // ITM_PRINTER_ADV (STD_PRINTER "ADV")
    }
    if(strcasecmp(prtSfx, "LCD") == 0) {
      return 1711; // fnP_LCD (STD_PRINTER "LCD")
    }
    if(strcasecmp(prtSfx, "PROG") == 0) {
      return 1713; // fnExportProgram (STD_PRINTER "PROG")
    }
    if(strcasecmp(prtSfx, "REGS") == 0) {
      return 1715; // fnP_All_Regs (STD_PRINTER "REGS")
    }
    if(strcasecmp(prtSfx, "STK") == 0) {
      return 1716; // fnP_All_Regs (STD_PRINTER "STK")
    }
    if(strcasecmp(prtSfx, "r") == 0) {
      return 1714; // fnP_Regs (STD_PRINTER "r")
    }
    if(strcasecmp(prtSfx, "x") == 0) {
      return 1676; // fnP_All_Regs (STD_PRINTER "x")
    }
    if(strcasecmp(prtSfx, "xy") == 0) {
      return 1707; // fnP_All_Regs (STD_PRINTER "xy")
    }
    if(strcasecmp(prtSfx, "USER") == 0) {
      return 1718; // fnP_User (STD_PRINTER "USER")
    }
    if(strcasecmp(prtSfx, "ALLr") == 0) {
      return 1799; // fnP_All_Regs (STD_PRINTER "ALLr")
    }
    if(strcmp(prtSfx, "α") == 0 || strcasecmp(prtSfx, "Alpha") == 0 || strcmp(prtSfx, "\x83\xb1") == 0 || strcmp(prtSfx, "\x83\x80") == 0) {
      return 2682; // fnP_Alpha (STD_PRINTER STD_alpha)
    }
    if(strcasecmp(prtSfx, "CHAR") == 0) {
      return 2695; // fnP_Char (STD_PRINTER "CHAR")
    }
    if(strcasecmp(prtSfx, "TAB") == 0) {
      return 2696; // fnP_Tab (STD_PRINTER "TAB")
    }
    if(strcmp(prtSfx, "#") == 0) {
      return 2697; // fnP_Byte (STD_PRINTER "#")
    }
    if(strcasecmp(prtSfx, "ITEMS") == 0) {
      return 2699; // fnP_PrintAllItems (STD_PRINTER "ITEMS")
    }
    if(strcasecmp(prtSfx, "LIST") == 0) {
      return 2693; // fnP_PrinterList (STD_PRINTER "LIST")
    }
    if(strcasecmp(prtSfx, "XFN") == 0) {
      return 2694; // S18_fnP_All_Regs (STD_PRINTER "XFN")
    }
    if(strcasecmp(prtSfx, "DLAY") == 0) {
      return 1710; // fnP_SetDelay (STD_PRINTER "DLAY")
    }
    if(strcasecmp(prtSfx, "DLAY#") == 0) {
      return 2686; // fnP_GetDelay (STD_PRINTER "DLAY#")
    }
    if(strcasecmp(prtSfx, "TRACE") == 0) {
      return 482; // FLAG_TRACE (STD_PRINTER "TRACE")
    }
    if(strcasecmp(prtSfx, "ACT") == 0) {
      return 495; // FLAG_PRTACT (STD_PRINTER "ACT")
    }
    if(strcasecmp(prtSfx, "EN") == 0) {
      return 2290; // FLAG_PRTEN (STD_PRINTER "EN")
    }
    if(strcasecmp(prtSfx, "NORM") == 0) {
      return 2291; // FLAG_NORM (STD_PRINTER "NORM")
    }
    if(strcmp(prtSfx, "Σ") == 0 || strcasecmp(prtSfx, "Sigma") == 0 || strcmp(prtSfx, "\x83\xa3") == 0) {
      return 1720; // fnP_Sigma (STD_PRINTER STD_SIGMA)
    }
    if(*prtSfx == '\0') {
      return 961; // ITM_PRINTER
    }
  }

  if(strcasecmp(cmd, "ABS") == 0 || strcmp(cmd, "|x|") == 0 || strcasecmp(cmd, ">ABS<") == 0) {
    return ITM_MAGNITUDE; // 105
  }
  if(strcasecmp(cmd, "NEWMAT") == 0) {
    return 1536; // ITM_M_NEW / fnNewMatrix ("M.NEW")
  }
  if(strcmp(cmd, "*") == 0 || strcmp(cmd, "×") == 0 || strcmp(cmd, "\x80\xd7") == 0) {
    return ITM_MULT;
  }
  if(strcmp(cmd, "/") == 0 || strcmp(cmd, "÷") == 0 || strcmp(cmd, "\x80\xf7") == 0) {
    return ITM_DIV;
  }
  if(strcmp(cmd, "+") == 0) {
    return ITM_ADD;
  }
  if(strcmp(cmd, "-") == 0) {
    return ITM_SUB;
  }
  if(strcasecmp(cmd, "X<>Y") == 0 || strcasecmp(cmd, "X<->Y") == 0 || strcmp(cmd, "x⇄y") == 0 || strcmp(cmd, "x↔y") == 0 || strcasecmp(cmd, "SWAP") == 0) {
    return 36; // fnSwapXY
  }
  if(strcasecmp(cmd, "x<>") == 0 || strcasecmp(cmd, "x⇄") == 0 || strcasecmp(cmd, "x><") == 0) {
    return 127; // ITM_Xex
  }
  if(strcasecmp(cmd, "y<>") == 0 || strcasecmp(cmd, "y⇄") == 0 || strcasecmp(cmd, "y><") == 0) {
    return 1650; // ITM_Yex
  }
  if(strcasecmp(cmd, "z<>") == 0 || strcasecmp(cmd, "z⇄") == 0 || strcasecmp(cmd, "z><") == 0) {
    return 1651; // ITM_Zex
  }
  if(strcasecmp(cmd, "t<>") == 0 || strcasecmp(cmd, "t⇄") == 0 || strcasecmp(cmd, "t><") == 0) {
    return 1625; // ITM_Tex
  }
  if(strcmp(cmd, "<>") == 0 || strcmp(cmd, "⇄") == 0 || strcasecmp(cmd, "SHUFFLE") == 0) {
    return 1694; // ITM_SHUFFLE
  }
  if(strcasecmp(cmd, "DROP") == 0 || strcasecmp(cmd, "DROPX") == 0) {
    return ITM_DROP; // 37
  }
  if(strcasecmp(cmd, "DROPY") == 0) {
    return 1544; // ITM_DROPY
  }
  if(strcasecmp(cmd, "ENTER") == 0 || strcasecmp(cmd, "ENTER↑") == 0 || strcasecmp(cmd, "ENTER^") == 0) {
    return ITM_ENTER;
  }
  if(strcasecmp(cmd, "RTN") == 0) {
    return ITM_RTN;
  }
  if(strcasecmp(cmd, "END") == 0 || strcasecmp(cmd, ".END.") == 0) {
    return ITM_END;
  }
  if(strcasecmp(cmd, "PI") == 0 || strcmp(cmd, "π") == 0) {
    return ITM_CONSTpi; // 109 (NOT ITM_PI 618 which is PTP_DISABLED Greek letter Π)
  }
  if(strcmp(cmd, "\xf0\x9d\x9d\x85") == 0 || strcmp(cmd, "#π") == 0 || strcasecmp(cmd, "#pi") == 0) {
    return 212; // OP_CNST_PI (#π)
  }
  if(strcmp(cmd, "ˣ√y") == 0 || strcmp(cmd, "x√y") == 0 || strcmp(cmd, "∜Ȳ") == 0 ||
     strcasecmp(cmd, "root(x)(y)") == 0 || strcasecmp(cmd, "root(x,y)") == 0 ||
     strcasecmp(cmd, "xthroot") == 0 || strcasecmp(cmd, "xroot") == 0) {
    return ITM_XTHROOT; // 63
  }
  if(strcmp(cmd, "√x") == 0 || strcmp(cmd, "√Ϳ") == 0 || strcasecmp(cmd, "sqrt") == 0 || strcasecmp(cmd, "sqrt(x)") == 0) {
    return ITM_SQUAREROOTX; // 61
  }
  if(strcmp(cmd, "∛x") == 0 || strcmp(cmd, "∛Ϳ") == 0 || strcasecmp(cmd, "cbrt") == 0 || strcasecmp(cmd, "cbrt(x)") == 0) {
    return ITM_CUBEROOT; // 62
  }
  if(strcmp(cmd, "x²") == 0 || strcasecmp(cmd, "x^2") == 0 || strcasecmp(cmd, "x↑2") == 0 || strcmp(cmd, "xⅢ") == 0 || strcasecmp(cmd, "sqr") == 0) {
    return ITM_SQUARE; // 58
  }
  if(strcmp(cmd, "x³") == 0 || strcasecmp(cmd, "x^3") == 0 || strcmp(cmd, "xⅣ") == 0 || strcasecmp(cmd, "cube") == 0) {
    return ITM_CUBE; // 59
  }
  if(strcmp(cmd, "yˣ") == 0 || strcasecmp(cmd, "y^x") == 0) {
    return ITM_YX; // 60
  }
  if(strcmp(cmd, "2ˣ") == 0 || strcasecmp(cmd, "2^x") == 0) {
    return ITM_2X; // 64
  }
  if(strcmp(cmd, "eˣ") == 0 || strcasecmp(cmd, "e^x") == 0 || strcasecmp(cmd, "exp") == 0 || strcasecmp(cmd, "exp(x)") == 0) {
    return ITM_EXP; // 65
  }
  if(strcmp(cmd, "10ˣ") == 0 || strcasecmp(cmd, "10^x") == 0) {
    return ITM_10x; // 67
  }
  if(strcmp(cmd, "(-1)ˣ") == 0 || strcasecmp(cmd, "(-1)^x") == 0) {
    return ITM_M1X; // 1679
  }
  if(strcasecmp(cmd, "1/X") == 0 || strcasecmp(cmd, "1/x") == 0 || strcasecmp(cmd, "inv") == 0) {
    return ITM_1ONX; // 566
  }
  if(strcasecmp(cmd, "log₂(x)") == 0 || strcasecmp(cmd, "log2(x)") == 0 || strcasecmp(cmd, "lb(x)") == 0 ||
     strcasecmp(cmd, "log₂") == 0 || strcasecmp(cmd, "log2") == 0 || strcasecmp(cmd, "lb") == 0) {
    return 68; // ITM_LOG2
  }
  if(strcasecmp(cmd, "ln(x)") == 0 || strcasecmp(cmd, "ln") == 0) {
    return 69; // ITM_LN
  }
  if(strcasecmp(cmd, "log₁₀(x)") == 0 || strcasecmp(cmd, "log10(x)") == 0 || strcasecmp(cmd, "lg(x)") == 0 ||
     strcasecmp(cmd, "log(x)") == 0 || strcasecmp(cmd, "log₁₀") == 0 || strcasecmp(cmd, "log10") == 0 ||
     strcasecmp(cmd, "lg") == 0 || strcasecmp(cmd, "log") == 0) {
    return 71; // ITM_LOG10
  }
  if(strcasecmp(cmd, "logₓ(y)") == 0 || strcasecmp(cmd, "log_x(y)") == 0 || strcasecmp(cmd, "logxy") == 0) {
    return 72; // ITM_LOGXY
  }
  if(strcasecmp(cmd, "cos(x)") == 0 || strcasecmp(cmd, "cos") == 0) {
    return 74; // ITM_COS
  }
  if(strcasecmp(cmd, "cosh(x)") == 0 || strcasecmp(cmd, "cosh") == 0) {
    return 75; // ITM_COSH
  }
  if(strcasecmp(cmd, "sin(x)") == 0 || strcasecmp(cmd, "sin") == 0) {
    return 76; // ITM_SIN
  }
  if(strcasecmp(cmd, "sinh(x)") == 0 || strcasecmp(cmd, "sinh") == 0) {
    return 78; // ITM_SINH
  }
  if(strcasecmp(cmd, "tan(x)") == 0 || strcasecmp(cmd, "tan") == 0) {
    return 79; // ITM_TAN
  }
  if(strcasecmp(cmd, "tanh(x)") == 0 || strcasecmp(cmd, "tanh") == 0) {
    return 80; // ITM_TANH
  }
  if(strcasecmp(cmd, "cos⁻¹(x)") == 0 || strcasecmp(cmd, "cos^-1(x)") == 0 || strcasecmp(cmd, "cos⁻¹") == 0 ||
     strcasecmp(cmd, "acos(x)") == 0 || strcasecmp(cmd, "arccos(x)") == 0 || strcasecmp(cmd, "acos") == 0 || strcasecmp(cmd, "arccos") == 0) {
    return 81; // ITM_ARCCOS
  }
  if(strcasecmp(cmd, "cosh⁻¹(x)") == 0 || strcasecmp(cmd, "cosh^-1(x)") == 0 || strcasecmp(cmd, "cosh⁻¹") == 0 ||
     strcasecmp(cmd, "acosh(x)") == 0 || strcasecmp(cmd, "arccosh(x)") == 0 || strcasecmp(cmd, "acosh") == 0 ||
     strcasecmp(cmd, "arccosh") == 0 || strcasecmp(cmd, "arcosh") == 0) {
    return 82; // ITM_ARCCOSH
  }
  if(strcasecmp(cmd, "sin⁻¹(x)") == 0 || strcasecmp(cmd, "sin^-1(x)") == 0 || strcasecmp(cmd, "sin⁻¹") == 0 ||
     strcasecmp(cmd, "asin(x)") == 0 || strcasecmp(cmd, "arcsin(x)") == 0 || strcasecmp(cmd, "asin") == 0 || strcasecmp(cmd, "arcsin") == 0) {
    return 83; // ITM_ARCSIN
  }
  if(strcasecmp(cmd, "sinh⁻¹(x)") == 0 || strcasecmp(cmd, "sinh^-1(x)") == 0 || strcasecmp(cmd, "sinh⁻¹") == 0 ||
     strcasecmp(cmd, "asinh(x)") == 0 || strcasecmp(cmd, "arcsinh(x)") == 0 || strcasecmp(cmd, "asinh") == 0 ||
     strcasecmp(cmd, "arcsinh") == 0 || strcasecmp(cmd, "arsinh") == 0) {
    return 84; // ITM_ARCSINH
  }
  if(strcasecmp(cmd, "tan⁻¹(x)") == 0 || strcasecmp(cmd, "tan^-1(x)") == 0 || strcasecmp(cmd, "tan⁻¹") == 0 ||
     strcasecmp(cmd, "atan(x)") == 0 || strcasecmp(cmd, "arctan(x)") == 0 || strcasecmp(cmd, "atan") == 0 || strcasecmp(cmd, "arctan") == 0) {
    return 85; // ITM_ARCTAN
  }
  if(strcasecmp(cmd, "tanh⁻¹(x)") == 0 || strcasecmp(cmd, "tanh^-1(x)") == 0 || strcasecmp(cmd, "tanh⁻¹") == 0 ||
     strcasecmp(cmd, "atanh(x)") == 0 || strcasecmp(cmd, "arctanh(x)") == 0 || strcasecmp(cmd, "atanh") == 0 ||
     strcasecmp(cmd, "arctanh") == 0 || strcasecmp(cmd, "artanh") == 0) {
    return 86; // ITM_ARCTANH
  }
  if(strcasecmp(cmd, "⌈x⌉") == 0 || strcasecmp(cmd, "ceil(x)") == 0 || strcasecmp(cmd, "ceil") == 0) {
    return 87; // ITM_CEIL
  }
  if(strcasecmp(cmd, "⌊x⌋") == 0 || strcasecmp(cmd, "floor(x)") == 0 || strcasecmp(cmd, "floor") == 0 || strcasecmp(cmd, "intg") == 0) {
    return 88; // ITM_FLOOR
  }
  if(strcasecmp(cmd, "x!") == 0 || strcasecmp(cmd, "fact(x)") == 0 || strcasecmp(cmd, "fact") == 0) {
    return 108; // ITM_XFACT
  }
  if(strcasecmp(cmd, "Γ(x)") == 0 || strcasecmp(cmd, "gamma(x)") == 0 || strcasecmp(cmd, "gamma") == 0) {
    return 1664; // ITM_GAMMAX
  }
  if(strcasecmp(cmd, "ShortInt") == 0 || strcasecmp(cmd, "SINT") == 0) {
    return 66; // ITM_SINT
  }
  if(strcasecmp(cmd, "LongInt") == 0 || strcasecmp(cmd, "LINT") == 0) {
    return 120; // ITM_LINT
  }
  if(strcasecmp(cmd, "ShortInt?") == 0 || strcasecmp(cmd, "SINT?") == 0) {
    return 2400;
  }
  if(strcasecmp(cmd, "LongInt?") == 0 || strcasecmp(cmd, "LINT?") == 0) {
    return 2398;
  }
  if(strcasecmp(cmd, "Number?") == 0 || strcasecmp(cmd, "NUMBR?") == 0) {
    return 2399;
  }
  if(strcmp(cmd, "𝔻?") == 0 || strcasecmp(cmd, "Date?") == 0) {
    return 2397;
  }
  if(strcmp(cmd, "𝕋?") == 0 || strcasecmp(cmd, "Time?") == 0) {
    return 2401;
  }
  if(strcmp(cmd, "ℂ?") == 0 || strcasecmp(cmd, "CPX?") == 0) {
    return 26;
  }
  if(strcmp(cmd, "ℝ?") == 0 || strcasecmp(cmd, "REAL?") == 0) {
    return 29;
  }
  if(strcmp(cmd, "±∞?") == 0 || strcasecmp(cmd, "+-INF?") == 0) {
    return 32;
  }
  if(strcasecmp(cmd, "⇒DEG") == 0 || strcasecmp(cmd, "->DEG") == 0 || strcasecmp(cmd, "=>DEG") == 0) {
    return 115;
  }
  if(strcasecmp(cmd, "⇒D.MS") == 0 || strcasecmp(cmd, "->D.MS") == 0 || strcasecmp(cmd, "=>D.MS") == 0 ||
     strcasecmp(cmd, "⇒DMS") == 0 || strcasecmp(cmd, "->DMS") == 0 || strcasecmp(cmd, "=>DMS") == 0) {
    return 116;
  }
  if(strcasecmp(cmd, "⇒GRAD") == 0 || strcasecmp(cmd, "->GRAD") == 0 || strcasecmp(cmd, "=>GRAD") == 0) {
    return 117;
  }
  if(strcasecmp(cmd, "⇒MULπ") == 0 || strcasecmp(cmd, "->MULpi") == 0 || strcasecmp(cmd, "=>MULpi") == 0 ||
     strcasecmp(cmd, "->MULπ") == 0 || strcasecmp(cmd, "=>MULπ") == 0) {
    return 118;
  }
  if(strcasecmp(cmd, "⇒RAD") == 0 || strcasecmp(cmd, "->RAD") == 0 || strcasecmp(cmd, "=>RAD") == 0) {
    return 119;
  }
  if(strcasecmp(cmd, "→REAL") == 0 || strcasecmp(cmd, "->REAL") == 0) {
    return 1691;
  }
  if(strcasecmp(cmd, "sinc(x)") == 0 || strcasecmp(cmd, "sinc") == 0) {
    return 1500;
  }
  if(strcasecmp(cmd, "ln(1+x)") == 0 || strcasecmp(cmd, "ln1p") == 0) {
    return 1614;
  }
  if(strcasecmp(cmd, "atan2(x,y)") == 0 || strcasecmp(cmd, "atan2") == 0) {
    return 1775;
  }
  if(strcmp(cmd, "R↑") == 0 || strcasecmp(cmd, "R^") == 0 || strcasecmp(cmd, "RUP") == 0 || strcasecmp(cmd, "ROLLUP") == 0) {
    return 39; // fnRollUp
  }
  if(strcmp(cmd, "R↓") == 0 || strcasecmp(cmd, "Rv") == 0 || strcasecmp(cmd, "RDN") == 0 || strcasecmp(cmd, "ROLLDOWN") == 0) {
    return 40; // fnRollDown
  }
  if(strcmp(cmd, "STO+") == 0) {
    return 45; // fnStoreAdd
  }
  if(strcmp(cmd, "STO-") == 0) {
    return 46; // fnStoreSub
  }
  if(strcmp(cmd, "STO×") == 0 || strcmp(cmd, "STO*") == 0) {
    return 47; // fnStoreMult
  }
  if(strcmp(cmd, "STO÷") == 0 || strcmp(cmd, "STO/") == 0) {
    return 48; // fnStoreDiv
  }
  if(strcmp(cmd, "RCL+") == 0) {
    return 52; // fnRecallAdd
  }
  if(strcmp(cmd, "RCL-") == 0) {
    return 53; // fnRecallSub
  }
  if(strcmp(cmd, "RCL×") == 0 || strcmp(cmd, "RCL*") == 0) {
    return 54; // fnRecallMult
  }
  if(strcmp(cmd, "RCL÷") == 0 || strcmp(cmd, "RCL/") == 0) {
    return 55; // fnRecallDiv
  }
  if(strcasecmp(cmd, "Draw_y^x") == 0 || strcasecmp(cmd, "Draw yˣ") == 0 || strcasecmp(cmd, "Drawyˣ") == 0) {
    return 2372;
  }
  if(strcasecmp(cmd, "PLTf") == 0 || strcasecmp(cmd, "PLT f") == 0) {
    return 2734;
  }
  if(strcasecmp(cmd, "X.SWAP") == 0) {
    return 2419;
  }
  if(strcmp(cmd, "CLΣ") == 0 || strcasecmp(cmd, "CLSIGMA") == 0 || strcasecmp(cmd, "CLSUM") == 0) {
    return ITM_CLSIGMA;
  }
  if(strcmp(cmd, "Σ+") == 0 || strcasecmp(cmd, "SIGMA+") == 0 || strcasecmp(cmd, "SUM+") == 0) {
    return ITM_SIGMAPLUS;
  }
  if(strcmp(cmd, "Σ-") == 0 || strcasecmp(cmd, "SIGMA-") == 0 || strcasecmp(cmd, "SUM-") == 0) {
    return ITM_SIGMAMINUS;
  }
  if(strcasecmp(cmd, "RAD") == 0) {
    return 1557;
  }
  if(strcasecmp(cmd, "GRAD") == 0) {
    return 1480;
  }
  if(strcasecmp(cmd, "2^x") == 0 || strcasecmp(cmd, "2^X") == 0 || strcasecmp(cmd, "2ˣ") == 0) {
    return ITM_2X;
  }
  if(strcasecmp(cmd, "10^x") == 0 || strcasecmp(cmd, "10^X") == 0 || strcasecmp(cmd, "10ˣ") == 0) {
    return ITM_10x;
  }
  if(strcasecmp(cmd, "y^x") == 0 || strcasecmp(cmd, "y^X") == 0 || strcasecmp(cmd, "yˣ") == 0) {
    return ITM_YX;
  }
  if(strcasecmp(cmd, "1/x") == 0 || strcasecmp(cmd, "1/X") == 0) {
    return ITM_1ONX;
  }
  if(strcasecmp(cmd, "→REC") == 0 || strcasecmp(cmd, "->REC") == 0 ||
     strcasecmp(cmd, "→RECT") == 0 || strcasecmp(cmd, "->RECT") == 0) {
    return 1850; // ITM_toRECT2
  }
  if(strcasecmp(cmd, "→POL") == 0 || strcasecmp(cmd, "->POL") == 0 ||
     strcasecmp(cmd, "→POLAR") == 0 || strcasecmp(cmd, "->POLAR") == 0) {
    return 1849; // ITM_toPOL2
  }
  if(strcasecmp(cmd, "deg→rad") == 0 || strcasecmp(cmd, "deg->rad") == 0) {
    return 2096; // ITM_DEGtoRAD
  }
  if(strcasecmp(cmd, "rad→deg") == 0 || strcasecmp(cmd, "rad->deg") == 0) {
    return 2097; // ITM_RADtoDEG
  }
  if(strcasecmp(cmd, "deg→grad") == 0 || strcasecmp(cmd, "deg->grad") == 0) {
    return 2098; // ITM_DEGtoGRAD
  }
  if(strcasecmp(cmd, "grad→deg") == 0 || strcasecmp(cmd, "grad->deg") == 0) {
    return 2099; // ITM_GRADtoDEG
  }
  if(strcasecmp(cmd, "grad→rad") == 0 || strcasecmp(cmd, "grad->rad") == 0) {
    return 2100; // ITM_GRADtoRAD
  }
  if(strcasecmp(cmd, "rad→grad") == 0 || strcasecmp(cmd, "rad->grad") == 0) {
    return 2101; // ITM_RADtoGRAD
  }
  if(strcmp(cmd, "∫^x_y") == 0 || strcmp(cmd, "∫yx") == 0 || strcmp(cmd, "∫xy") == 0 || strcmp(cmd, "∫yˣ") == 0) {
    return 1690; // ITM_INTEGRAL_YX
  }
  if(strcmp(cmd, "Σₙ") == 0 || strcmp(cmd, "∑ₙ") == 0 || strcmp(cmd, "Σ_n") == 0 || strcmp(cmd, "∑_n") == 0) {
    return 1672; // fnProgrammableSum
  }
  if(strcmp(cmd, "∏ₙ") == 0 || strcmp(cmd, "Πₙ") == 0 || strcmp(cmd, "∏_n") == 0 || strcmp(cmd, "Π_n") == 0) {
    return 1671; // fnProgrammableProduct
  }
  if(strcmp(cmd, "iΣₙ") == 0 || strcmp(cmd, "i∑ₙ") == 0 || strcmp(cmd, "iΣ_n") == 0 || strcmp(cmd, "i∑_n") == 0) {
    return 1755; // fnProgSumI
  }
  if(strcmp(cmd, "i∏ₙ") == 0 || strcmp(cmd, "iΠₙ") == 0 || strcmp(cmd, "i∏_n") == 0 || strcmp(cmd, "iΠ_n") == 0) {
    return 1754; // fnProgProdI
  }
  if(strcmp(cmd, "∞Σₙ") == 0 || strcmp(cmd, "∞∑ₙ") == 0 || strcmp(cmd, "∞Σ_n") == 0 || strcmp(cmd, "∞∑_n") == 0) {
    return 2755; // fnInfSum
  }
  if(strcasecmp(cmd, "setEUR") == 0) {
    return 1593; // ITM_SETEUR
  }
  if(strcasecmp(cmd, "setUK") == 0) {
    return 1594; // ITM_SETUK
  }
  if(strcasecmp(cmd, "setUS") == 0) {
    return 1595; // ITM_SETUSA
  }
  if(strcasecmp(cmd, "setIND") == 0) {
    return 1591; // ITM_SETIND
  }
  if(strcasecmp(cmd, "setCHN") == 0) {
    return 1592; // ITM_SETCHN
  }
  if(strcasecmp(cmd, "setJPN") == 0) {
    return 1596; // ITM_SETJPN
  }
  if(strcmp(cmd, "ISPC␣") == 0) {
    return 2119; // ITM_ISPC ("ISPC" STD_OPEN_BOX)
  }
  if(strcmp(cmd, "FSPC␣") == 0) {
    return 2132; // ITM_FSPC ("FSPC" STD_OPEN_BOX)
  }
  if(strcasecmp(cmd, "RPER.") == 0) {
    return 2139; // ITM_RPER ("RPER.")
  }
  if(strcasecmp(cmd, "→INT") == 0 || strcasecmp(cmd, "->INT") == 0) {
    return 1872; // fnChangeBaseMNU (STD_RIGHT_ARROW "INT")
  }
  if(strcasecmp(cmd, "x≤-0?") == 0 || strcasecmp(cmd, "x<=-0?") == 0) {
    return 2850; // fnCheckLessEqualMinusZero
  }
  if(strcasecmp(cmd, "x≥+0?") == 0 || strcasecmp(cmd, "x>=+0?") == 0) {
    return 2851; // fnCheckGreaterEqualPlusZero
  }
  if(strcasecmp(cmd, "STRING?") == 0) {
    return 31; // ITM_STRI
  }
  if(strcasecmp(cmd, "LASTT?") == 0) {
    return 1677; // ITM_LASTT ("LASTT#")
  }
  if(strcasecmp(cmd, "x_circ") == 0 || strcasecmp(cmd, "xhat") == 0) {
    return 1643; // fnXIsFny (STD_x_CIRC)
  }
  if(strcasecmp(cmd, "INC") == 0) {
    return 92; // ITM_INCR
  }

  char c47Cmd[128];
  extern void utf8ToString(const uint8_t *utf8, char *str);
  utf8ToString((const uint8_t *)cmd, c47Cmd);

  for(int i = 0; i < LAST_ITEM; ++i) {
    if((indexOfItems[i].status & PTP_STATUS) != PTP_DISABLED) {
      if(indexOfItems[i].itemCatalogName[0] != '\0') {
        if(compareString(cmd, indexOfItems[i].itemCatalogName, CMP_COMMAND) == 0 ||
           compareString(c47Cmd, indexOfItems[i].itemCatalogName, CMP_COMMAND) == 0) {
          return i;
        }
        char catReadable[128];
        c47StringToReadableUtf8(indexOfItems[i].itemCatalogName, catReadable, sizeof(catReadable));
        if(strcasecmp(cmd, catReadable) == 0) {
          return i;
        }
      }
      if(indexOfItems[i].itemSoftmenuName[0] != '\0') {
        if(compareString(cmd, indexOfItems[i].itemSoftmenuName, CMP_COMMAND) == 0 ||
           compareString(c47Cmd, indexOfItems[i].itemSoftmenuName, CMP_COMMAND) == 0) {
          return i;
        }
        char smReadable[128];
        c47StringToReadableUtf8(indexOfItems[i].itemSoftmenuName, smReadable, sizeof(smReadable));
        if(strcasecmp(cmd, smReadable) == 0) {
          return i;
        }
      }
    }
  }

  // Fallback: match flattened subscript/superscript/arrow command representations (e.g. →RECT_(HP), FORₓ, NEXTₓ)
  char flatCmd[128];
  flattenSubSupCommand(cmd, flatCmd, sizeof(flatCmd));
  if(flatCmd[0] != '\0') {
    for(int i = 0; i < LAST_ITEM; ++i) {
      if((indexOfItems[i].status & PTP_STATUS) != PTP_DISABLED) {
        if(indexOfItems[i].itemCatalogName[0] != '\0') {
          char flatCat[128];
          flattenSubSupCommand(indexOfItems[i].itemCatalogName, flatCat, sizeof(flatCat));
          if(strcasecmp(flatCmd, flatCat) == 0) {
            return i;
          }
        }
        if(indexOfItems[i].itemSoftmenuName[0] != '\0') {
          char flatSm[128];
          flattenSubSupCommand(indexOfItems[i].itemSoftmenuName, flatSm, sizeof(flatSm));
          if(strcasecmp(flatCmd, flatSm) == 0) {
            return i;
          }
        }
      }
    }
  }
  return -1;
}

static void readableUtf8ToC47String(const char *src, char *dest, size_t destSize) {
  if(dest == NULL || destSize == 0) {
    return;
  }
  if(src == NULL) {
    dest[0] = '\0';
    return;
  }
  char preBuf[512];
  size_t pi = 0;
  const uint8_t *s = (const uint8_t *)src;
  while(*s && pi < sizeof(preBuf) - 4) {
    if(s[0] == 0xe2 && s[1] == 0x81 && s[2] == 0xb0) { // ⁰ -> U+2160 (STD_SUP_0)
      preBuf[pi++] = (char)0xe2;
      preBuf[pi++] = (char)0x85;
      preBuf[pi++] = (char)0xa0;
      s += 3;
    }
    else if(s[0] == 0xc2 && s[1] == 0xb9) { // ¹ -> U+2161 (STD_SUP_1)
      preBuf[pi++] = (char)0xe2;
      preBuf[pi++] = (char)0x85;
      preBuf[pi++] = (char)0xa1;
      s += 2;
    }
    else if(s[0] == 0xc2 && s[1] == 0xb2) { // ² -> U+2162 (STD_SUP_2)
      preBuf[pi++] = (char)0xe2;
      preBuf[pi++] = (char)0x85;
      preBuf[pi++] = (char)0xa2;
      s += 2;
    }
    else if(s[0] == 0xc2 && s[1] == 0xb3) { // ³ -> U+2163 (STD_SUP_3)
      preBuf[pi++] = (char)0xe2;
      preBuf[pi++] = (char)0x85;
      preBuf[pi++] = (char)0xa3;
      s += 2;
    }
    else if(s[0] == 0xe2 && s[1] == 0x81 && s[2] >= 0xb4 && s[2] <= 0xb9) { // ⁴..⁹ -> U+2164..U+2169
      preBuf[pi++] = (char)0xe2;
      preBuf[pi++] = (char)0x85;
      preBuf[pi++] = (char)(0xa0 + (s[2] - 0xb0));
      s += 3;
    }
    else if(s[0] == 0xe2 && s[1] == 0x81 && s[2] == 0xba) { // ⁺ -> U+216A (STD_SUP_PLUS)
      preBuf[pi++] = (char)0xe2;
      preBuf[pi++] = (char)0x85;
      preBuf[pi++] = (char)0xaa;
      s += 3;
    }
    else if(s[0] == 0xe2 && s[1] == 0x81 && s[2] == 0xbb) { // ⁻ -> U+216B (STD_SUP_MINUS)
      preBuf[pi++] = (char)0xe2;
      preBuf[pi++] = (char)0x85;
      preBuf[pi++] = (char)0xab;
      s += 3;
    }
    else if(s[0] == 0xf0 && s[1] == 0x9f && s[2] == 0x96 && s[3] == 0xa8) { // 🖨 -> U+2399 (STD_PRINTER)
      preBuf[pi++] = (char)0xe2;
      preBuf[pi++] = (char)0x8e;
      preBuf[pi++] = (char)0x99;
      s += 4;
    }
    else {
      preBuf[pi++] = (char)*s++;
    }
  }
  preBuf[pi] = '\0';

  char c47Tmp[512];
  extern void utf8ToString(const uint8_t *utf8, char *str);
  utf8ToString((const uint8_t *)preBuf, c47Tmp);
  size_t len = strlen(c47Tmp);
  if(len >= destSize) {
    len = destSize - 1;
  }
  memcpy(dest, c47Tmp, len);
  dest[len] = '\0';
}

static bool tryParseNumericOrLocalRegister(const char *s, bool allowBareDigits, uint8_t *outCode) {
  if(s == NULL || *s == '\0') {
    return false;
  }
  // R00..R99 or R0..R9
  if((s[0] == 'R' || s[0] == 'r') && s[1] >= '0' && s[1] <= '9') {
    if(s[2] == '\0') {
      *outCode = (uint8_t)(s[1] - '0');
      return true;
    }
    if(s[2] >= '0' && s[2] <= '9' && s[3] == '\0') {
      *outCode = (uint8_t)((s[1] - '0') * 10 + (s[2] - '0'));
      return true;
    }
  }
  // R.00..R.98 or .00...98
  const char *dotPart = NULL;
  if(s[0] == '.') {
    dotPart = s + 1;
  }
  else if((s[0] == 'R' || s[0] == 'r') && s[1] == '.') {
    dotPart = s + 2;
  }
  if(dotPart != NULL && dotPart[0] >= '0' && dotPart[0] <= '9') {
    int num = -1;
    if(dotPart[1] == '\0') {
      num = dotPart[0] - '0';
    }
    else if(dotPart[1] >= '0' && dotPart[1] <= '9' && dotPart[2] == '\0') {
      num = (dotPart[0] - '0') * 10 + (dotPart[1] - '0');
    }
    if(num >= 0 && num <= 98) {
      *outCode = (uint8_t)(FIRST_LOCAL_REGISTER_IN_KS_CODE + num);
      return true;
    }
  }
  // Bare 00..99 or 0..9
  if(allowBareDigits && s[0] >= '0' && s[0] <= '9') {
    if(s[1] == '\0') {
      *outCode = (uint8_t)(s[0] - '0');
      return true;
    }
    if(s[1] >= '0' && s[1] <= '9' && s[2] == '\0') {
      *outCode = (uint8_t)((s[0] - '0') * 10 + (s[1] - '0'));
      return true;
    }
  }
  return false;
}

static bool parseRegisterArg(const char *arg, uint8_t *outCode, char *outVar, uint8_t *outVarLen) {
  while(*arg == ' ' || *arg == '\t') {
    arg++;
  }
  if(*arg == '\0') {
    return false;
  }

  if(*arg == '"' || *arg == '\'' || ((uint8_t)arg[0] == 0xe2 && (uint8_t)arg[1] == 0x80 && ((uint8_t)arg[2] == 0x98 || (uint8_t)arg[2] == 0x9c))) {
    const char *start = arg + 1;
    uint8_t endUtf8 = 0;
    char endChar = 0;
    if((uint8_t)arg[0] == 0xe2) {
      start = arg + 3;
      endUtf8 = ((uint8_t)arg[2] == 0x98) ? 0x99 : 0x9d;
    }
    else {
      endChar = *arg;
    }
    const char *end = start;
    if(endUtf8 != 0) {
      while(*end) {
        if((uint8_t)end[0] == 0xe2 && (uint8_t)end[1] == 0x80 && (uint8_t)end[2] == endUtf8) {
          if(end[3] == '\0' || end[3] == ' ' || end[3] == '\t') {
            break;
          }
        }
        end++;
      }
    }
    else {
      while(*end) {
        if(*end == endChar) {
          if(end[1] == '\0' || end[1] == ' ' || end[1] == '\t') {
            break;
          }
        }
        end++;
      }
    }
    size_t rawLen = (size_t)(end - start);
    char rawInner[64];
    if(rawLen >= sizeof(rawInner)) {
      rawLen = sizeof(rawInner) - 1;
    }
    memcpy(rawInner, start, rawLen);
    rawInner[rawLen] = '\0';

    // Heal quoted register names such as 'R09', ‘R11’, 'R.00', '.00' into register codes
    if(tryParseNumericOrLocalRegister(rawInner, false, outCode)) {
      *outVarLen = 0;
      return true;
    }

    char c47Inner[64];
    readableUtf8ToC47String(rawInner, c47Inner, sizeof(c47Inner));
    size_t len = strlen(c47Inner);
    if(len > 15) {
      len = 15;
    }
    memcpy(outVar, c47Inner, len);
    outVar[len] = '\0';
    *outVarLen = (uint8_t)len;
    *outCode = STRING_LABEL_VARIABLE;
    return true;
  }

  if((arg[0] == '-' && arg[1] == '>') || ((uint8_t)arg[0] == 0xe2 && (uint8_t)arg[1] == 0x86 && (uint8_t)arg[2] == 0x92) ||
     strncasecmp(arg, "IND ", 4) == 0 || strncasecmp(arg, "IND\t", 4) == 0) {
    const char *target = arg;
    if(arg[0] == '-' && arg[1] == '>') {
      target += 2;
    }
    else if((uint8_t)arg[0] == 0xe2) {
      target += 3;
    }
    else {
      target += 4;
    }
    while(*target == ' ' || *target == '\t') {
      target++;
    }
    uint8_t subCode = 0;
    char subVar[32];
    uint8_t subLen = 0;
    if(!parseRegisterArg(target, &subCode, subVar, &subLen)) {
      return false;
    }
    if(subCode == STRING_LABEL_VARIABLE) {
      *outCode = INDIRECT_VARIABLE;
      memcpy(outVar, subVar, subLen);
      outVar[subLen] = '\0';
      *outVarLen = subLen;
      return true;
    }
    if(subCode == INDIRECT_REGISTER || subCode == INDIRECT_VARIABLE) {
      return false;
    }
    *outCode = INDIRECT_REGISTER;
    outVar[0] = (char)subCode;
    *outVarLen = 1;
    return true;
  }

  if(tryParseNumericOrLocalRegister(arg, true, outCode)) {
    *outVarLen = 0;
    return true;
  }

  if(strcasecmp(arg, "X") == 0) {
    *outCode = REGISTER_X_IN_KS_CODE;
    *outVarLen = 0;
    return true;
  }
  if(strcasecmp(arg, "Y") == 0) {
    *outCode = REGISTER_Y_IN_KS_CODE;
    *outVarLen = 0;
    return true;
  }
  if(strcasecmp(arg, "Z") == 0) {
    *outCode = REGISTER_Z_IN_KS_CODE;
    *outVarLen = 0;
    return true;
  }
  if(strcasecmp(arg, "T") == 0) {
    *outCode = REGISTER_T_IN_KS_CODE;
    *outVarLen = 0;
    return true;
  }

  if(strlen(arg) == 1 && arg[0] >= 'A' && arg[0] <= 'D') {
    *outCode = (uint8_t)(REGISTER_A_IN_KS_CODE + (arg[0] - 'A'));
    *outVarLen = 0;
    return true;
  }
  if(strlen(arg) == 1 && arg[0] >= 'a' && arg[0] <= 'd') {
    *outCode = (uint8_t)(REGISTER_A_IN_KS_CODE + (arg[0] - 'a'));
    *outVarLen = 0;
    return true;
  }
  if(strcasecmp(arg, "L") == 0) {
    *outCode = REGISTER_L_IN_KS_CODE;
    *outVarLen = 0;
    return true;
  }
  if(strlen(arg) == 1 && arg[0] >= 'I' && arg[0] <= 'K') {
    *outCode = (uint8_t)(REGISTER_I_IN_KS_CODE + (arg[0] - 'I'));
    *outVarLen = 0;
    return true;
  }
  if(strlen(arg) == 1 && arg[0] >= 'i' && arg[0] <= 'k') {
    *outCode = (uint8_t)(REGISTER_I_IN_KS_CODE + (arg[0] - 'i'));
    *outVarLen = 0;
    return true;
  }
  if(strcasecmp(arg, "M") == 0) { *outCode = REGISTER_M_IN_KS_CODE; *outVarLen = 0; return true; }
  if(strcasecmp(arg, "N") == 0) { *outCode = REGISTER_N_IN_KS_CODE; *outVarLen = 0; return true; }
  if(strcasecmp(arg, "P") == 0) { *outCode = REGISTER_P_IN_KS_CODE; *outVarLen = 0; return true; }
  if(strcasecmp(arg, "Q") == 0) { *outCode = REGISTER_Q_IN_KS_CODE; *outVarLen = 0; return true; }
  if(strcasecmp(arg, "R") == 0) { *outCode = REGISTER_R_IN_KS_CODE; *outVarLen = 0; return true; }
  if(strcasecmp(arg, "S") == 0) { *outCode = REGISTER_S_IN_KS_CODE; *outVarLen = 0; return true; }

  if(strcasecmp(arg, "E") == 0) { *outCode = REGISTER_E_IN_KS_CODE; *outVarLen = 0; return true; }
  if(strcasecmp(arg, "F") == 0) { *outCode = REGISTER_F_IN_KS_CODE; *outVarLen = 0; return true; }
  if(strcasecmp(arg, "G") == 0) { *outCode = REGISTER_G_IN_KS_CODE; *outVarLen = 0; return true; }
  if(strcasecmp(arg, "H") == 0) { *outCode = REGISTER_H_IN_KS_CODE; *outVarLen = 0; return true; }
  if(strcasecmp(arg, "O") == 0) { *outCode = REGISTER_O_IN_KS_CODE; *outVarLen = 0; return true; }
  if(strcasecmp(arg, "U") == 0) { *outCode = REGISTER_U_IN_KS_CODE; *outVarLen = 0; return true; }
  if(strcasecmp(arg, "V") == 0) { *outCode = REGISTER_V_IN_KS_CODE; *outVarLen = 0; return true; }
  if(strcasecmp(arg, "W") == 0) { *outCode = REGISTER_W_IN_KS_CODE; *outVarLen = 0; return true; }

  char c47Var[64];
  readableUtf8ToC47String(arg, c47Var, sizeof(c47Var));
  size_t len = strlen(c47Var);
  if(len > 15) {
    len = 15;
  }
  memcpy(outVar, c47Var, len);
  outVar[len] = '\0';
  *outVarLen = (uint8_t)len;
  *outCode = STRING_LABEL_VARIABLE;
  return true;
}

static bool parseLabelArg(const char *arg, uint8_t *outCode, char *outName, uint8_t *outNameLen) {
  while(*arg == ' ' || *arg == '\t') {
    arg++;
  }
  if(*arg == '\0') {
    return false;
  }

  if((arg[0] == '-' && arg[1] == '>') || ((uint8_t)arg[0] == 0xe2 && (uint8_t)arg[1] == 0x86 && (uint8_t)arg[2] == 0x92) ||
     strncasecmp(arg, "IND ", 4) == 0 || strncasecmp(arg, "IND\t", 4) == 0) {
    const char *target = arg;
    if(arg[0] == '-' && arg[1] == '>') {
      target += 2;
    }
    else if((uint8_t)arg[0] == 0xe2) {
      target += 3;
    }
    else {
      target += 4;
    }
    while(*target == ' ' || *target == '\t') {
      target++;
    }
    uint8_t subCode = 0;
    char subVar[32];
    uint8_t subLen = 0;
    if(!parseRegisterArg(target, &subCode, subVar, &subLen)) {
      return false;
    }
    if(subCode == STRING_LABEL_VARIABLE) {
      *outCode = INDIRECT_VARIABLE;
      memcpy(outName, subVar, subLen);
      outName[subLen] = '\0';
      *outNameLen = subLen;
      return true;
    }
    if(subCode == INDIRECT_REGISTER || subCode == INDIRECT_VARIABLE) {
      return false;
    }
    *outCode = INDIRECT_REGISTER;
    outName[0] = (char)subCode;
    *outNameLen = 1;
    return true;
  }

  if(arg[0] == ':') {
    const char *start = arg + 1;
    const char *end = strchr(start, ':');
    if(!end) {
      end = start + strlen(start);
    }
    size_t rawLen = (size_t)(end - start);
    char rawName[64];
    if(rawLen >= sizeof(rawName)) {
      rawLen = sizeof(rawName) - 1;
    }
    memcpy(rawName, start, rawLen);
    rawName[rawLen] = '\0';

    char c47Name[64];
    readableUtf8ToC47String(rawName, c47Name, sizeof(c47Name));
    size_t len = strlen(c47Name);
    if(len > 15) {
      len = 15;
    }
    memcpy(outName, c47Name, len);
    outName[len] = '\0';
    *outNameLen = (uint8_t)len;
    *outCode = LOCAL_LABEL_VARIABLE;
    return true;
  }

  if(*arg == '"' || *arg == '\'' || ((uint8_t)arg[0] == 0xe2 && (uint8_t)arg[1] == 0x80 && ((uint8_t)arg[2] == 0x98 || (uint8_t)arg[2] == 0x9c))) {
    const char *start = arg + 1;
    uint8_t endUtf8 = 0;
    char endChar = 0;
    if((uint8_t)arg[0] == 0xe2) {
      start = arg + 3;
      endUtf8 = ((uint8_t)arg[2] == 0x98) ? 0x99 : 0x9d;
    }
    else {
      endChar = *arg;
    }
    const char *end = start;
    if(endUtf8 != 0) {
      while(*end) {
        if((uint8_t)end[0] == 0xe2 && (uint8_t)end[1] == 0x80 && (uint8_t)end[2] == endUtf8) {
          if(end[3] == '\0' || end[3] == ' ' || end[3] == '\t') {
            break;
          }
        }
        end++;
      }
    }
    else {
      while(*end) {
        if(*end == endChar) {
          if(end[1] == '\0' || end[1] == ' ' || end[1] == '\t') {
            break;
          }
        }
        end++;
      }
    }
    size_t rawLen = (size_t)(end - start);
    char rawName[64];
    if(rawLen >= sizeof(rawName)) {
      rawLen = sizeof(rawName) - 1;
    }
    memcpy(rawName, start, rawLen);
    rawName[rawLen] = '\0';

    char c47Name[64];
    readableUtf8ToC47String(rawName, c47Name, sizeof(c47Name));
    size_t len = strlen(c47Name);
    if(len > 15) {
      len = 15;
    }
    memcpy(outName, c47Name, len);
    outName[len] = '\0';
    *outNameLen = (uint8_t)len;
    *outCode = STRING_LABEL_VARIABLE;
    return true;
  }

  if(strlen(arg) == 1 && arg[0] >= 'A' && arg[0] <= 'L') {
    *outCode = (uint8_t)(100 + (arg[0] - 'A'));
    *outNameLen = 0;
    return true;
  }

  if(strlen(arg) == 1 && arg[0] >= 'a' && arg[0] <= 'l') {
    *outCode = (uint8_t)(FIRST_LC_LOCAL_LABEL + (arg[0] - 'a'));
    *outNameLen = 0;
    return true;
  }

  if(arg[0] >= '0' && arg[0] <= '9') {
    int num = atoi(arg);
    if(num >= 0 && num <= 99) {
      *outCode = (uint8_t)num;
      *outNameLen = 0;
      return true;
    }
  }

  char c47Name[64];
  readableUtf8ToC47String(arg, c47Name, sizeof(c47Name));
  size_t len = strlen(c47Name);
  if(len > 15) {
    len = 15;
  }
  memcpy(outName, c47Name, len);
  outName[len] = '\0';
  *outNameLen = (uint8_t)len;
  *outCode = STRING_LABEL_VARIABLE;
  return true;
}

static bool parseComparisonArg(const char *arg, uint8_t *outCode, char *outVar, uint8_t *outVarLen) {
  while(*arg == ' ' || *arg == '\t') {
    arg++;
  }
  if(strcmp(arg, "0") == 0 || strcmp(arg, "0.0") == 0 || strcmp(arg, "0.") == 0) {
    *outCode = VALUE_0;
    *outVarLen = 0;
    return true;
  }
  if(strcmp(arg, "1") == 0 || strcmp(arg, "1.0") == 0 || strcmp(arg, "1.") == 0) {
    *outCode = VALUE_1;
    *outVarLen = 0;
    return true;
  }
  return parseRegisterArg(arg, outCode, outVar, outVarLen);
}

static void stripProgramComment(char *s) {
  if(s == NULL || *s == '\0') {
    return;
  }
  char quoteChar = 0;
  char *p = s;
  while(*p) {
    if(quoteChar != 0) {
      if((uint8_t)quoteChar == 0x99 || (uint8_t)quoteChar == 0x9d) {
        if((uint8_t)p[0] == 0xe2 && (uint8_t)p[1] == 0x80 && (uint8_t)p[2] == (uint8_t)quoteChar) {
          quoteChar = 0;
          p += 2;
        }
      }
      else if(*p == quoteChar) {
        if(*(p + 1) == '\0' || *(p + 1) == ' ' || *(p + 1) == '\t' || *(p + 1) == '\r' || *(p + 1) == '\n') {
          quoteChar = 0;
        }
      }
    }
    else {
      if(*p == '\'' || *p == '"') {
        quoteChar = *p;
      }
      else if((uint8_t)p[0] == 0xe2 && (uint8_t)p[1] == 0x80 && ((uint8_t)p[2] == 0x98 || (uint8_t)p[2] == 0x9c)) {
        quoteChar = ((uint8_t)p[2] == 0x98) ? 0x99 : 0x9d;
        p += 2;
      }
      else if(*p == '{') {
        char *closing = strchr(p, '}');
        if(closing != NULL) {
          memset(p, ' ', (size_t)(closing - p + 1));
          p = closing;
        }
        else {
          *p = '\0';
          break;
        }
      }
      else if(*p == ';' || *p == '@' || (p[0] == '/' && p[1] == '/') ||
              (*p == '#' && (p == s || *(p - 1) == ' ' || *(p - 1) == '\t') &&
               (*(p + 1) == ' ' || *(p + 1) == '\t' || *(p + 1) == '\0' ||
                !((*p + 1 >= '0' && *(p + 1) <= '9') || *(p + 1) == 'b' || *(p + 1) == 'B' || *(p + 1) == 'o' || *(p + 1) == 'O' || *(p + 1) == 'd' || *(p + 1) == 'D' || *(p + 1) == 'h' || *(p + 1) == 'H')))) {
        *p = '\0';
        break;
      }
    }
    p++;
  }
  size_t len = strlen(s);
  while(len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t' || s[len - 1] == '\r' || s[len - 1] == '\n')) {
    s[--len] = '\0';
  }
}

static bool isProgramHeaderLine(const char *s) {
  while(*s == ' ' || *s == '\t') {
    s++;
  }
  if(*s == '\0') {
    return false;
  }
  if(strncasecmp(s, "C47", 3) == 0 || strncasecmp(s, "R47", 3) == 0) {
    return true;
  }
  if(strncasecmp(s, "PROGRAM_FILE_FORMAT", 19) == 0 || strncasecmp(s, "PROGRAM FILE FORMAT", 19) == 0) {
    return true;
  }
  if(strncasecmp(s, "Title:", 6) == 0 || strncasecmp(s, "Author:", 7) == 0 ||
     strncasecmp(s, "Date:", 5) == 0 || strncasecmp(s, "Version:", 8) == 0 ||
     strncasecmp(s, "Created:", 8) == 0 || strncasecmp(s, "Tested:", 7) == 0 ||
     strncasecmp(s, "Input:", 6) == 0 || strncasecmp(s, "Output:", 7) == 0 ||
     strncasecmp(s, "Index:", 6) == 0 || strncasecmp(s, "Description:", 12) == 0 ||
     strncasecmp(s, "File:", 5) == 0 || strncasecmp(s, "Notes:", 6) == 0) {
    return true;
  }
  if(strncmp(s, "===", 3) == 0 || strncmp(s, "---", 3) == 0 || strncmp(s, "***", 3) == 0) {
    return true;
  }
  return false;
}


static bool parseFlagArg(const char *arg, uint8_t *outCode, uint8_t *outSysFlag, bool *isSysFlag) {
  *isSysFlag = false;
  *outSysFlag = 0;
  while(*arg == ' ' || *arg == '\t') {
    arg++;
  }
  if(*arg == '\0') {
    return false;
  }
  if(*arg == '"' || *arg == '\'' || ((uint8_t)arg[0] == 0xe2 && (uint8_t)arg[1] == 0x80 && ((uint8_t)arg[2] == 0x98 || (uint8_t)arg[2] == 0x9c))) {
    const char *start = arg + 1;
    uint8_t endUtf8 = 0;
    char endChar = 0;
    if((uint8_t)arg[0] == 0xe2) {
      start = arg + 3;
      endUtf8 = ((uint8_t)arg[2] == 0x98) ? 0x99 : 0x9d;
    }
    else {
      endChar = *arg;
    }
    const char *end = start;
    if(endUtf8 != 0) {
      while(*end) {
        if((uint8_t)end[0] == 0xe2 && (uint8_t)end[1] == 0x80 && (uint8_t)end[2] == endUtf8) {
          if(end[3] == '\0' || end[3] == ' ' || end[3] == '\t') {
            break;
          }
        }
        end++;
      }
    }
    else {
      while(*end) {
        if(*end == endChar) {
          if(end[1] == '\0' || end[1] == ' ' || end[1] == '\t') {
            break;
          }
        }
        end++;
      }
    }
    size_t rawLen = (size_t)(end - start);
    char rawName[64];
    if(rawLen >= sizeof(rawName)) {
      rawLen = sizeof(rawName) - 1;
    }
    memcpy(rawName, start, rawLen);
    rawName[rawLen] = '\0';

    if(strcasecmp(rawName, "HOME.3") == 0) {
      *outCode = SYSTEM_FLAG_NUMBER;
      *outSysFlag = (uint8_t)(SFL_HOME_TRIPLE - SFL_MONIT + 64);
      *isSysFlag = true;
      return true;
    }
    if(strcasecmp(rawName, "MyM.3") == 0) {
      *outCode = SYSTEM_FLAG_NUMBER;
      *outSysFlag = (uint8_t)(SFL_MYM_TRIPLE - SFL_MONIT + 64);
      *isSysFlag = true;
      return true;
    }
    if(strcasecmp(rawName, "HP.RP") == 0) {
      *outCode = SYSTEM_FLAG_NUMBER;
      *outSysFlag = (uint8_t)(SFL_HPRP - SFL_TDM24);
      *isSysFlag = true;
      return true;
    }

    char c47Name[64];
    readableUtf8ToC47String(rawName, c47Name, sizeof(c47Name));
    char flatRaw[64];
    flattenSubSupCommand(rawName, flatRaw, sizeof(flatRaw));
    for(int k = 0; k < 64; k++) {
      int item = SFL_TDM24 + k;
      if(item < LAST_ITEM && indexOfItems[item].itemSoftmenuName[0] != '\0') {
        char flatSm[64];
        flattenSubSupCommand(indexOfItems[item].itemSoftmenuName, flatSm, sizeof(flatSm));
        if(compareString(c47Name, indexOfItems[item].itemSoftmenuName, CMP_COMMAND) == 0 ||
           strcasecmp(rawName, indexOfItems[item].itemSoftmenuName) == 0 ||
           (flatRaw[0] != '\0' && strcasecmp(flatRaw, flatSm) == 0)) {
          *outCode = SYSTEM_FLAG_NUMBER;
          *outSysFlag = (uint8_t)k;
          *isSysFlag = true;
          return true;
        }
      }
    }
    for(int k = 64; (SFL_MONIT + k - 64) < LAST_ITEM && k < 128; k++) {
      int item = SFL_MONIT + k - 64;
      if(indexOfItems[item].itemSoftmenuName[0] != '\0') {
        char flatSm[64];
        flattenSubSupCommand(indexOfItems[item].itemSoftmenuName, flatSm, sizeof(flatSm));
        if(compareString(c47Name, indexOfItems[item].itemSoftmenuName, CMP_COMMAND) == 0 ||
           strcasecmp(rawName, indexOfItems[item].itemSoftmenuName) == 0 ||
           (flatRaw[0] != '\0' && strcasecmp(flatRaw, flatSm) == 0)) {
          *outCode = SYSTEM_FLAG_NUMBER;
          *outSysFlag = (uint8_t)k;
          *isSysFlag = true;
          return true;
        }
      }
    }
    if(rawName[0] != '\0' && rawName[0] != '"' && rawName[0] != '\'' && (uint8_t)rawName[0] != 0xe2) {
      return parseFlagArg(rawName, outCode, outSysFlag, isSysFlag);
    }
    return false;
  }

  // Local flag .00..31, R.00..R.31, or F.00..F.31
  const char *dotPart = NULL;
  if(arg[0] == '.') {
    dotPart = arg + 1;
  }
  else if((arg[0] == 'R' || arg[0] == 'r' || arg[0] == 'F' || arg[0] == 'f') && arg[1] == '.') {
    dotPart = arg + 2;
  }
  if(dotPart != NULL && dotPart[0] >= '0' && dotPart[0] <= '9') {
    if(dotPart[1] == '\0' || (dotPart[1] >= '0' && dotPart[1] <= '9' && dotPart[2] == '\0')) {
      int num = atoi(dotPart);
      if(num >= 0 && num <= 31) {
        *outCode = (uint8_t)(FIRST_LOCAL_FLAG + num);
        return true;
      }
    }
    return false;
  }

  // Global flag F00..F99 or F0..F9 (rejig syntax)
  if((arg[0] == 'F' || arg[0] == 'f') && arg[1] >= '0' && arg[1] <= '9') {
    if(arg[2] == '\0') {
      *outCode = (uint8_t)(arg[1] - '0');
      return true;
    }
    if(arg[2] >= '0' && arg[2] <= '9' && arg[3] == '\0') {
      *outCode = (uint8_t)((arg[1] - '0') * 10 + (arg[2] - '0'));
      return true;
    }
    return false;
  }

  // Global flag 00..99 or R00..R99
  uint8_t regCode = 0;
  if(tryParseNumericOrLocalRegister(arg, true, &regCode) && regCode <= 99) {
    *outCode = regCode;
    return true;
  }

  // Lettered flags X..K and M..W
  uint8_t letCode = 0;
  char dummyVar[32];
  uint8_t dummyLen = 0;
  if(strlen(arg) == 1 && parseRegisterArg(arg, &letCode, dummyVar, &dummyLen) &&
     ((FLAG_X <= letCode && letCode <= FLAG_K) || (FLAG_M <= letCode && letCode <= FLAG_W))) {
    *outCode = letCode;
    return true;
  }
  return false;
}

static const char *extractArgToken(const char *p, char *out, size_t outSize) {
  while(*p == ' ' || *p == '\t') {
    p++;
  }
  if(*p == '\0') {
    out[0] = '\0';
    return p;
  }
  size_t oi = 0;
  if(strncasecmp(p, "IND ", 4) == 0) {
    while(*p && *p != ' ' && *p != '\t' && oi < outSize - 2) {
      out[oi++] = *p++;
    }
    while((*p == ' ' || *p == '\t') && oi < outSize - 2) {
      out[oi++] = *p++;
    }
  }
  else if((p[0] == '-' && p[1] == '>' && (p[2] == ' ' || p[2] == '\t')) ||
          ((uint8_t)p[0] == 0xe2 && (uint8_t)p[1] == 0x86 && (uint8_t)p[2] == 0x92 && (p[3] == ' ' || p[3] == '\t'))) {
    if(p[0] == '-') {
      out[oi++] = *p++;
      out[oi++] = *p++;
    }
    else {
      out[oi++] = *p++;
      out[oi++] = *p++;
      out[oi++] = *p++;
    }
    while((*p == ' ' || *p == '\t') && oi < outSize - 2) {
      out[oi++] = *p++;
    }
  }
  if(*p == '"' || *p == '\'') {
    char q = *p++;
    if(oi < outSize - 2) {
      out[oi++] = q;
    }
    while(*p) {
      if(*p == q && (p[1] == '\0' || p[1] == ' ' || p[1] == '\t')) {
        break;
      }
      if(oi < outSize - 2) {
        out[oi++] = *p;
      }
      p++;
    }
    if(*p == q) {
      if(oi < outSize - 1) {
        out[oi++] = *p;
      }
      p++;
    }
    out[oi] = '\0';
    return p;
  }
  if((uint8_t)p[0] == 0xe2 && (uint8_t)p[1] == 0x80 && ((uint8_t)p[2] == 0x98 || (uint8_t)p[2] == 0x9c)) {
    uint8_t endByte = ((uint8_t)p[2] == 0x98) ? 0x99 : 0x9d;
    if(oi < outSize - 4) {
      out[oi++] = *p++;
      out[oi++] = *p++;
      out[oi++] = *p++;
    }
    else {
      p += 3;
    }
    while(*p) {
      if((uint8_t)p[0] == 0xe2 && (uint8_t)p[1] == 0x80 && (uint8_t)p[2] == endByte &&
         (p[3] == '\0' || p[3] == ' ' || p[3] == '\t')) {
        break;
      }
      if(oi < outSize - 4) {
        out[oi++] = *p;
      }
      p++;
    }
    if((uint8_t)p[0] == 0xe2 && (uint8_t)p[1] == 0x80 && (uint8_t)p[2] == endByte) {
      if(oi < outSize - 4) {
        out[oi++] = *p++;
        out[oi++] = *p++;
        out[oi++] = *p++;
      }
      else {
        p += 3;
      }
    }
    out[oi] = '\0';
    return p;
  }
  if(*p == ':') {
    out[oi++] = *p++;
    while(*p && *p != ':' && oi < outSize - 2) {
      out[oi++] = *p++;
    }
    if(*p == ':') {
      out[oi++] = *p++;
    }
    out[oi] = '\0';
    return p;
  }
  while(*p && *p != ' ' && *p != '\t' && oi < outSize - 1) {
    out[oi++] = *p++;
  }
  out[oi] = '\0';
  return p;
}

static const char *tryParseComparison(const char *p, EncodedStep *outStep, char *errMsg, size_t errLen) {
  if(p == NULL || (p[0] != 'x' && p[0] != 'X')) {
    return NULL;
  }
  uint16_t op = 0;
  size_t opSymbolLen = 0;
  if(strncmp(p + 1, "<=", 2) == 0) {
    op = 17;
    opSymbolLen = 2;
  }
  else if((uint8_t)p[1] == 0xe2 && (uint8_t)p[2] == 0x89 && (uint8_t)p[3] == 0xa4) { // ≤
    op = 17;
    opSymbolLen = 3;
  }
  else if(strncmp(p + 1, ">=", 2) == 0) {
    op = 18;
    opSymbolLen = 2;
  }
  else if((uint8_t)p[1] == 0xe2 && (uint8_t)p[2] == 0x89 && (uint8_t)p[3] == 0xa5) { // ≥
    op = 18;
    opSymbolLen = 3;
  }
  else if(strncmp(p + 1, "!=", 2) == 0) {
    op = 12;
    opSymbolLen = 2;
  }
  else if((uint8_t)p[1] == 0xe2 && (uint8_t)p[2] == 0x89 && (uint8_t)p[3] == 0xa0) { // ≠
    op = 12;
    opSymbolLen = 3;
  }
  else if(p[1] == '#') {
    op = 12;
    opSymbolLen = 1;
  }
  else if(strncmp(p + 1, "==", 2) == 0) {
    op = 11;
    opSymbolLen = 2;
  }
  else if(p[1] == '=') {
    op = 11;
    opSymbolLen = 1;
  }
  else if(p[1] == '<') {
    if(p[2] == '>') {
      const char *q = p + 3;
      while(*q == ' ' || *q == '\t') {
        q++;
      }
      if(*q == '?') {
        op = 12; // x<>? or x<> ? is x≠?
        opSymbolLen = 2;
      }
      else {
        return NULL; // x<> is swap
      }
    }
    else {
      op = 16;
      opSymbolLen = 1;
    }
  }
  else if(p[1] == '>') {
    op = 19;
    opSymbolLen = 1;
  }
  else if((uint8_t)p[1] == 0xe2 && (uint8_t)p[2] == 0x89 && (uint8_t)p[3] == 0x88) { // ≈
    op = 15;
    opSymbolLen = 3;
  }
  else if(strncasecmp(p + 1, "approx", 6) == 0) {
    op = 15;
    opSymbolLen = 6;
  }
  else {
    return NULL;
  }

  const char *argP = p + 1 + opSymbolLen;
  if(strncmp(argP, "-0?", 3) == 0 || strncmp(argP, "+0?", 3) == 0) {
    return NULL; // Handled by 0-arg opcodes x=-0?, x=+0?, x≤-0?, x≥+0?
  }
  while(*argP == ' ' || *argP == '\t') {
    argP++;
  }
  if(*argP == '?') {
    argP++;
    while(*argP == ' ' || *argP == '\t') {
      argP++;
    }
  }

  char rawOperand[64];
  argP = extractArgToken(argP, rawOperand, sizeof(rawOperand));
  size_t oi = strlen(rawOperand);
  while(oi > 0 && rawOperand[oi - 1] == '?') {
    rawOperand[--oi] = '\0';
  }

  if(rawOperand[0] == '\0') {
    snprintf(errMsg, errLen, "Comparison requires operand");
    return NULL;
  }

  uint8_t cmpCode = 0;
  char varBuf[32];
  uint8_t varLen = 0;
  if(!parseComparisonArg(rawOperand, &cmpCode, varBuf, &varLen)) {
    snprintf(errMsg, errLen, "Invalid comparison operand '%s'", rawOperand);
    return NULL;
  }

  outStep->bytes[0] = (uint8_t)op;
  if(cmpCode == STRING_LABEL_VARIABLE || cmpCode == INDIRECT_VARIABLE) {
    outStep->bytes[1] = cmpCode;
    outStep->bytes[2] = varLen;
    memcpy(&outStep->bytes[3], varBuf, varLen);
    outStep->size = (uint16_t)(3 + varLen);
  }
  else if(cmpCode == INDIRECT_REGISTER) {
    outStep->bytes[1] = INDIRECT_REGISTER;
    outStep->bytes[2] = (uint8_t)varBuf[0];
    outStep->size = 3;
  }
  else {
    outStep->bytes[1] = cmpCode;
    outStep->size = 2;
  }
  return argP;
}

static bool isValidRealNumericLiteral(const char *s, size_t len) {
  if(s == NULL || len == 0) {
    return false;
  }
  size_t i = 0;
  if(s[i] == '+' || s[i] == '-') {
    i++;
  }
  bool hasDigits = false;
  bool hasDot = false;
  bool hasExponent = false;

  while(i < len) {
    uint8_t c = (uint8_t)s[i];
    if(c >= '0' && c <= '9') {
      hasDigits = true;
      i++;
    }
    else if(c == '.' || c == ',') {
      if(hasDot || hasExponent) {
        return false;
      }
      hasDot = true;
      i++;
    }
    else if(c == 'e' || c == 'E' ||
            (i + 4 < len && strncmp(&s[i], "*10^", 4) == 0) ||
            (i + 5 < len && (uint8_t)s[i] == 0xc3 && (uint8_t)s[i + 1] == 0x97 && strncmp(&s[i + 2], "10^", 3) == 0)) {
      if(hasExponent || !hasDigits) {
        return false;
      }
      hasExponent = true;
      if(c == 'e' || c == 'E') {
        i++;
      }
      else if(s[i] == '*') {
        i += 4;
      }
      else {
        i += 5;
      }
      if(i < len && (s[i] == '+' || s[i] == '-')) {
        i++;
      }
      if(i + 1 == len && (s[i] == '.' || s[i] == ',')) {
        i++;
        break;
      }
      if(i >= len || s[i] < '0' || s[i] > '9') {
        return false;
      }
    }
    else if(c == 0xe2 && i + 2 < len && (uint8_t)s[i + 1] == 0x80 && (uint8_t)s[i + 2] >= 0x80 && (uint8_t)s[i + 2] <= 0x8a) {
      // UTF-8 thin space / grouping space (U+2000..U+200A)
      i += 3;
    }
    else if(c == 0xe2 && i + 2 < len && (uint8_t)s[i + 1] == 0x80 && (uint8_t)s[i + 2] == 0xaf) {
      // Narrow no-break space (U+202F)
      i += 3;
    }
    else if(c == 0xc2 && i + 1 < len && (uint8_t)s[i + 1] == 0xa0) {
      // Non-breaking space (U+00A0)
      i += 2;
    }
    else {
      return false;
    }
  }

  return hasDigits;
}

static void normalizeRealTokenString(const char *start, size_t rawLen, bool stripTrailingDot, char *normNum, size_t normSize) {
  size_t ni = 0;
  for(size_t i = 0; i < rawLen && ni < normSize - 1; i++) {
    if((uint8_t)start[i] == 0xe2 && i + 2 < rawLen && (uint8_t)start[i + 1] == 0x80 && (uint8_t)start[i + 2] >= 0x80 && (uint8_t)start[i + 2] <= 0x8a) {
      i += 2;
      continue;
    }
    if((uint8_t)start[i] == 0xe2 && i + 2 < rawLen && (uint8_t)start[i + 1] == 0x80 && (uint8_t)start[i + 2] == 0xaf) {
      i += 2;
      continue;
    }
    if((uint8_t)start[i] == 0xc2 && i + 1 < rawLen && (uint8_t)start[i + 1] == 0xa0) {
      i += 1;
      continue;
    }
    if(i + 4 < rawLen && strncmp(&start[i], "*10^", 4) == 0) {
      if(stripTrailingDot && ni > 0 && normNum[ni - 1] == '.') {
        ni--;
      }
      normNum[ni++] = 'e';
      i += 3;
      continue;
    }
    if(i + 5 < rawLen && (uint8_t)start[i] == 0xc3 && (uint8_t)start[i + 1] == 0x97 && strncmp(&start[i + 2], "10^", 3) == 0) {
      if(stripTrailingDot && ni > 0 && normNum[ni - 1] == '.') {
        ni--;
      }
      normNum[ni++] = 'e';
      i += 4;
      continue;
    }
    normNum[ni++] = (start[i] == ',') ? '.' : start[i];
  }
  if(ni >= 2 && (normNum[ni - 2] == 'e' || normNum[ni - 2] == 'E') && normNum[ni - 1] == '.') {
    normNum[ni - 1] = '0';
  }
  if(stripTrailingDot && ni > 1 && normNum[ni - 1] == '.') {
    ni--;
  }
  normNum[ni] = '\0';
}

static bool tryParseComplexNumericLiteral(const char *s, size_t len, char *outCplx, size_t outSize) {
  if(s == NULL || len < 4 || outSize < 8) {
    return false;
  }
  size_t sepIdx = 0;
  for(size_t i = 1; i < len - 1; i++) {
    if((s[i] == '+' || s[i] == '-') && s[i - 1] != 'e' && s[i - 1] != 'E' && s[i - 1] != '^') {
      sepIdx = i;
    }
  }
  if(sepIdx == 0) {
    return false;
  }
  if(!isValidRealNumericLiteral(s, sepIdx)) {
    return false;
  }
  char sign = s[sepIdx];
  const char *imagStart = s + sepIdx + 1;
  size_t imagLen = len - (sepIdx + 1);
  if(imagLen < 2) {
    return false;
  }
  char unit = 0;
  const char *magStart = NULL;
  size_t magLen = 0;
  char lastC = imagStart[imagLen - 1];
  if(lastC == 'i' || lastC == 'I' || lastC == 'j' || lastC == 'J') {
    unit = (lastC == 'j' || lastC == 'J') ? 'j' : 'i';
    magStart = imagStart;
    magLen = imagLen - 1;
  }
  else if(imagStart[0] == 'i' || imagStart[0] == 'I' || imagStart[0] == 'j' || imagStart[0] == 'J') {
    unit = (imagStart[0] == 'j' || imagStart[0] == 'J') ? 'j' : 'i';
    magStart = imagStart + 1;
    magLen = imagLen - 1;
    if(magLen > 0 && *magStart == '*') {
      magStart++;
      magLen--;
    }
    else if(magLen > 1 && (uint8_t)magStart[0] == 0xc3 && (uint8_t)magStart[1] == 0x97) {
      magStart += 2;
      magLen -= 2;
    }
  }
  if(unit == 0 || magLen == 0 || !isValidRealNumericLiteral(magStart, magLen)) {
    return false;
  }
  char normReal[60];
  char normImag[60];
  normalizeRealTokenString(s, sepIdx, true, normReal, sizeof(normReal));
  normalizeRealTokenString(magStart, magLen, true, normImag, sizeof(normImag));
  snprintf(outCplx, outSize, "%s%c%c%s", normReal, sign, unit, normImag);
  return true;
}

static bool parseStructuredCommandWithSubscript(const char *cmd, int *outOp, int *outSubscript) {
  if(cmd == NULL || cmd[0] == '\0') {
    return false;
  }
  static const struct {
    const char *name;
    int op;
  } structOps[] = {
    {"ENDIF",  ITM_ENDIF},  // 2922
    {"ENDDO",  ITM_ENDDO},  // 2926
    {"REPEAT", ITM_REPEAT}, // 2935
    {"UNTIL",  ITM_UNTIL},  // 2936
    {"WHILE",  ITM_WHILE},  // 2925
    {"ELSE",   ITM_ELSE},   // 2921
    {"SKIP",   ITM_SKIP},   // 1603
    {"BACK",   ITM_BACK},   // 1412
    {"IF",     ITM_IF},     // 2920
    {"DO",     ITM_DO}      // 2924
  };

  for(size_t i = 0; i < sizeof(structOps) / sizeof(structOps[0]); i++) {
    size_t nlen = strlen(structOps[i].name);
    if(strncasecmp(cmd, structOps[i].name, nlen) == 0) {
      const char *s = cmd + nlen;
      if(*s == '\0') {
        *outOp = structOps[i].op;
        *outSubscript = -1;
        return true;
      }
      if(*s == '_') {
        s++;
      }
      int val = 0;
      bool hasDigits = false;
      while(*s) {
        if((uint8_t)s[0] == 0xe2 && (uint8_t)s[1] == 0x82 && (uint8_t)s[2] >= 0x80 && (uint8_t)s[2] <= 0x89) {
          val = val * 10 + ((uint8_t)s[2] - 0x80);
          hasDigits = true;
          s += 3;
        }
        else if((uint8_t)s[0] == 0xa0 && (uint8_t)s[1] >= 0x80 && (uint8_t)s[1] <= 0x89) {
          val = val * 10 + ((uint8_t)s[1] - 0x80);
          hasDigits = true;
          s += 2;
        }
        else if((uint8_t)s[0] == 0xe2 && (uint8_t)s[1] == 0x81 && (uint8_t)s[2] == 0xb0) {
          val = val * 10 + 0;
          hasDigits = true;
          s += 3;
        }
        else if((uint8_t)s[0] == 0xc2 && (uint8_t)s[1] == 0xb9) {
          val = val * 10 + 1;
          hasDigits = true;
          s += 2;
        }
        else if((uint8_t)s[0] == 0xc2 && (uint8_t)s[1] == 0xb2) {
          val = val * 10 + 2;
          hasDigits = true;
          s += 2;
        }
        else if((uint8_t)s[0] == 0xc2 && (uint8_t)s[1] == 0xb3) {
          val = val * 10 + 3;
          hasDigits = true;
          s += 2;
        }
        else if((uint8_t)s[0] == 0xe2 && (uint8_t)s[1] == 0x81 && (uint8_t)s[2] >= 0xb4 && (uint8_t)s[2] <= 0xb9) {
          val = val * 10 + ((uint8_t)s[2] - 0xb0);
          hasDigits = true;
          s += 3;
        }
        else if(*s >= '0' && *s <= '9') {
          val = val * 10 + (*s - '0');
          hasDigits = true;
          s++;
        }
        else {
          hasDigits = false;
          break;
        }
      }
      if(hasDigits && *s == '\0') {
        *outOp = structOps[i].op;
        *outSubscript = val;
        return true;
      }
    }
  }
  return false;
}

static const char *parseInstructionToken(const char *p, EncodedStep *outStep, char *errMsg, size_t errLen) {
  while(*p == ' ' || *p == '\t') {
    p++;
  }
  if(*p == '\0') {
    return p;
  }
  outStep->size = 0;

  // 1. END or .END.
  if((strncasecmp(p, "END", 3) == 0 && (p[3] == ' ' || p[3] == '\t' || p[3] == '\0')) ||
     (strncasecmp(p, ".END.", 5) == 0 && (p[5] == ' ' || p[5] == '\t' || p[5] == '\0'))) {
    outStep->bytes[0] = (uint8_t)((ITM_END >> 8) | 0x80);
    outStep->bytes[1] = (uint8_t)(ITM_END & 0xff);
    outStep->size = 2;
    return p + ((p[0] == '.') ? 5 : 3);
  }

  // 2. String literal: "...", '...', ‘...’, or “...”
  bool isString = false;
  const char *strStart = NULL;
  size_t rawLen = 0;
  if(*p == '"' || *p == '\'') {
    char q = *p++;
    strStart = p;
    while(*p) {
      if(*p == q) {
        if(*(p + 1) == q) {
          p += 2;
          continue;
        }
        if(*(p + 1) == '\0' || *(p + 1) == ' ' || *(p + 1) == '\t') {
          break;
        }
      }
      p++;
    }
    rawLen = (size_t)(p - strStart);
    if(*p == q) {
      p++;
    }
    isString = true;
  }
  else if((uint8_t)p[0] == 0xe2 && (uint8_t)p[1] == 0x80 && ((uint8_t)p[2] == 0x98 || (uint8_t)p[2] == 0x9c)) {
    uint8_t endByte = ((uint8_t)p[2] == 0x98) ? 0x99 : 0x9d;
    p += 3;
    strStart = p;
    while(*p) {
      if((uint8_t)p[0] == 0xe2 && (uint8_t)p[1] == 0x80 && (uint8_t)p[2] == endByte) {
        break;
      }
      p++;
    }
    rawLen = (size_t)(p - strStart);
    if((uint8_t)p[0] == 0xe2 && (uint8_t)p[1] == 0x80 && (uint8_t)p[2] == endByte) {
      p += 3;
    }
    isString = true;
  }

  if(isString) {
    char rawStr[256];
    size_t copyLen = (rawLen < sizeof(rawStr) - 1) ? rawLen : sizeof(rawStr) - 1;
    memcpy(rawStr, strStart, copyLen);
    rawStr[copyLen] = '\0';

    char c47Str[256];
    readableUtf8ToC47String(rawStr, c47Str, sizeof(c47Str));
    size_t c47Len = strlen(c47Str);
    if(c47Len > 120) {
      c47Len = 120;
    }
    outStep->bytes[0] = (uint8_t)ITM_LITERAL;
    outStep->bytes[1] = (uint8_t)STRING_LABEL_VARIABLE;
    outStep->bytes[2] = (uint8_t)c47Len;
    memcpy(&outStep->bytes[3], c47Str, c47Len);
    outStep->size = (uint16_t)(3 + c47Len);
    return p;
  }

  // 3. Short integer with base (e.g. 1111#2, FF#16, 77#8, 0₁₆, 3₁₆)
  const char *tokEnd = p;
  const char *hashPos = NULL;
  while(*tokEnd && *tokEnd != ' ' && *tokEnd != '\t') {
    if(*tokEnd == '#') {
      hashPos = tokEnd;
    }
    tokEnd++;
  }
  const char *digitEnd = hashPos;
  uint8_t subBase = 0;
  if(digitEnd == NULL && tokEnd > p + 3) {
    // Check for trailing UTF-8 subscript digits ₀..₉ (E2 82 80..89) or C47 subscript digits (A0 80..89)
    const char *scan = p;
    const char *subStart = NULL;
    int bAcc = 0;
    while(scan < tokEnd) {
      if(scan + 2 < tokEnd && (uint8_t)scan[0] == 0xe2 && (uint8_t)scan[1] == 0x82 && (uint8_t)scan[2] >= 0x80 && (uint8_t)scan[2] <= 0x89) {
        if(subStart == NULL) {
          subStart = scan;
          bAcc = 0;
        }
        bAcc = bAcc * 10 + ((uint8_t)scan[2] - 0x80);
        scan += 3;
      }
      else if(scan + 1 < tokEnd && (uint8_t)scan[0] == 0xa0 && (uint8_t)scan[1] >= 0x80 && (uint8_t)scan[1] <= 0x89) {
        if(subStart == NULL) {
          subStart = scan;
          bAcc = 0;
        }
        bAcc = bAcc * 10 + ((uint8_t)scan[1] - 0x80);
        scan += 2;
      }
      else {
        subStart = NULL;
        bAcc = 0;
        scan++;
      }
    }
    if(subStart != NULL && subStart > p && bAcc >= 2 && bAcc <= 16) {
      const char *firstD = (*p == '+' || *p == '-') ? (p + 1) : p;
      if(firstD < subStart && *firstD >= '0' && *firstD <= '9') {
        digitEnd = subStart;
        subBase = (uint8_t)bAcc;
      }
    }
  }
  if(digitEnd != NULL && digitEnd > p && (subBase != 0 || digitEnd + 1 < tokEnd)) {
    bool validBase = (subBase != 0);
    if(!validBase) {
      const char *bptr = digitEnd + 1;
      if((*bptr >= '0' && *bptr <= '9') || *bptr == 'b' || *bptr == 'B' ||
         *bptr == 'o' || *bptr == 'O' || *bptr == 'd' || *bptr == 'D' ||
         *bptr == 'h' || *bptr == 'H') {
        validBase = true;
      }
    }
    if(validBase) {
      const char *s = p;
      if(*s == '+' || *s == '-') {
        s++;
      }
      bool allHex = (s < digitEnd);
      while(s < digitEnd) {
        if((*s >= '0' && *s <= '9') || (*s >= 'A' && *s <= 'F') || (*s >= 'a' && *s <= 'f')) {
          s++;
        }
        else if((uint8_t)s[0] == 0xe2 && (uint8_t)s[1] == 0x80 && (uint8_t)s[2] >= 0x80 && (uint8_t)s[2] <= 0x8a) {
          s += 3;
        }
        else if(*s == '.' || *s == ',' || *s == '_') {
          s++;
        }
        else {
          allHex = false;
          break;
        }
      }
      if(allHex) {
        uint8_t base = subBase;
        if(base == 0) {
          if(strcasecmp(digitEnd + 1, "b") == 0) {
            base = 2;
          }
          else if(strcasecmp(digitEnd + 1, "o") == 0) {
            base = 8;
          }
          else if(strcasecmp(digitEnd + 1, "d") == 0) {
            base = 10;
          }
          else if(strcasecmp(digitEnd + 1, "h") == 0) {
            base = 16;
          }
          else {
            int bval = atoi(digitEnd + 1);
            base = (uint8_t)((bval >= 1 && bval <= 16) ? bval : 2);
          }
        }

        char digits[128];
        size_t dlen = 0;
        s = p;
        if(*s == '+' || *s == '-') {
          if(*s == '-') {
            digits[dlen++] = '-';
          }
          s++;
        }
        while(s < digitEnd && dlen < sizeof(digits) - 1) {
          if((*s >= '0' && *s <= '9') || (*s >= 'A' && *s <= 'F') || (*s >= 'a' && *s <= 'f')) {
            digits[dlen++] = *s;
            s++;
          }
          else if((uint8_t)s[0] == 0xe2 && (uint8_t)s[1] == 0x80 && (uint8_t)s[2] >= 0x80 && (uint8_t)s[2] <= 0x8a) {
            s += 3;
          }
          else {
            s++;
          }
        }
        digits[dlen] = '\0';
        if(dlen == 0 || (dlen == 1 && digits[0] == '-')) {
          digits[dlen++] = '0';
          digits[dlen] = '\0';
        }

        outStep->bytes[0] = (uint8_t)ITM_LITERAL;
        outStep->bytes[1] = (uint8_t)STRING_SHORT_INTEGER;
        outStep->bytes[2] = base;
        outStep->bytes[3] = (uint8_t)dlen;
        memcpy(&outStep->bytes[4], digits, dlen);
        outStep->size = (uint16_t)(4 + dlen);
        return tokEnd;
      }
    }
  }

  // 4. Real or Complex numeric literal: e.g. 4, -5, +1.2, 1e-3, 1.*10^3, 200.+120.i
  const char *chk = p;
  if(*chk == '+' || *chk == '-') {
    chk++;
  }
  if((*chk >= '0' && *chk <= '9') || (*chk == '.' && *(chk + 1) >= '0' && *(chk + 1) <= '9')) {
    const char *tokEnd = p;
    while(*tokEnd && *tokEnd != ' ' && *tokEnd != '\t') {
      tokEnd++;
    }
    size_t tokLen = (size_t)(tokEnd - p);
    char cplxStr[128];
    if(tryParseComplexNumericLiteral(p, tokLen, cplxStr, sizeof(cplxStr))) {
      size_t clen = strlen(cplxStr);
      outStep->bytes[0] = (uint8_t)ITM_LITERAL;
      outStep->bytes[1] = (uint8_t)STRING_COMPLEX34;
      outStep->bytes[2] = (uint8_t)clen;
      memcpy(&outStep->bytes[3], cplxStr, clen);
      outStep->size = (uint16_t)(3 + clen);
      return tokEnd;
    }
    if(isValidRealNumericLiteral(p, tokLen)) {
      const char *start = p;
      p = tokEnd;
      char normNum[256];
      normalizeRealTokenString(start, tokLen, false, normNum, sizeof(normNum));
      size_t ni = strlen(normNum);
      if(ni > 250) {
        ni = 250;
      }
      bool isReal = (strchr(normNum, '.') != NULL || strchr(normNum, 'e') != NULL || strchr(normNum, 'E') != NULL);
      outStep->bytes[0] = (uint8_t)ITM_LITERAL;
      outStep->bytes[1] = (uint8_t)(isReal ? STRING_REAL34 : STRING_LONG_INTEGER);
      outStep->bytes[2] = (uint8_t)ni;
      memcpy(&outStep->bytes[3], normNum, ni);
      outStep->size = (uint16_t)(3 + ni);
      return p;
    }
  }

  // 4. Comparisons: starting with x or X (e.g. x<y?, x=0?, x=? 0, x<> ? Y)
  if((*p == 'x' || *p == 'X') &&
     strncmp(p, "x!", 2) != 0 && strncmp(p, "X!", 2) != 0 &&
     strncmp(p, "x²", 3) != 0 && strncmp(p, "x³", 3) != 0 &&
     strncmp(p, "x^2", 3) != 0 && strncmp(p, "x^3", 3) != 0 &&
     strncmp(p, "x↑2", 5) != 0 && strncmp(p, "X↑2", 5) != 0 &&
     strncmp(p, "x⇄", 4) != 0 && strncasecmp(p, "x><", 3) != 0 &&
     strncasecmp(p, "x=+0?", 5) != 0 && strncasecmp(p, "x=-0?", 5) != 0 &&
     strncasecmp(p, "xroot", 5) != 0 && strncasecmp(p, "xthroot", 7) != 0 &&
     strncasecmp(p, "x.swap", 6) != 0 && strncasecmp(p, "xeq", 3) != 0) {
    const char *nextP = tryParseComparison(p, outStep, errMsg, errLen);
    if(nextP != NULL) {
      return nextP;
    }
  }

  // 5. LBL, GTO, XEQ, MVAR
  if(strcasecmp(p, "LBL") == 0 || strncasecmp(p, "LBL ", 4) == 0 || strncasecmp(p, "LBL\t", 4) == 0 || strncasecmp(p, "LBL:", 4) == 0) {
    p += 3;
    char argToken[64];
    p = extractArgToken(p, argToken, sizeof(argToken));
    uint8_t codeParam = 0;
    char nameBuf[32];
    uint8_t nameLen = 0;
    if(!parseLabelArg(argToken, &codeParam, nameBuf, &nameLen)) {
      snprintf(errMsg, errLen, "Invalid LBL argument '%s'", argToken);
      return NULL;
    }
    if(codeParam == INDIRECT_REGISTER || codeParam == INDIRECT_VARIABLE) {
      snprintf(errMsg, errLen, "LBL cannot have indirect argument '%s'", argToken);
      return NULL;
    }
    outStep->bytes[0] = (uint8_t)ITM_LBL;
    if(codeParam == STRING_LABEL_VARIABLE || codeParam == LOCAL_LABEL_VARIABLE) {
      outStep->bytes[1] = codeParam;
      outStep->bytes[2] = nameLen;
      memcpy(&outStep->bytes[3], nameBuf, nameLen);
      outStep->size = (uint16_t)(3 + nameLen);
    }
    else {
      outStep->bytes[1] = codeParam;
      outStep->size = 2;
    }
    return p;
  }

  if(strncasecmp(p, "GTO ", 4) == 0 || strncasecmp(p, "GTO\t", 4) == 0 || strncasecmp(p, "GTO:", 4) == 0 ||
     strncasecmp(p, "XEQ ", 4) == 0 || strncasecmp(p, "XEQ\t", 4) == 0 || strncasecmp(p, "XEQ:", 4) == 0) {
    uint16_t op = (strncasecmp(p, "GTO", 3) == 0) ? ITM_GTO : ITM_XEQ;
    p += 3;
    char argToken[64];
    p = extractArgToken(p, argToken, sizeof(argToken));
    uint8_t codeParam = 0;
    char nameBuf[32];
    uint8_t nameLen = 0;
    if(!parseLabelArg(argToken, &codeParam, nameBuf, &nameLen)) {
      snprintf(errMsg, errLen, "Invalid %s argument '%s'", (op == ITM_GTO) ? "GTO" : "XEQ", argToken);
      return NULL;
    }
    outStep->bytes[0] = (uint8_t)op;
    if(codeParam == STRING_LABEL_VARIABLE || codeParam == LOCAL_LABEL_VARIABLE || codeParam == INDIRECT_VARIABLE) {
      outStep->bytes[1] = codeParam;
      outStep->bytes[2] = nameLen;
      memcpy(&outStep->bytes[3], nameBuf, nameLen);
      outStep->size = (uint16_t)(3 + nameLen);
    }
    else if(codeParam == INDIRECT_REGISTER) {
      outStep->bytes[1] = INDIRECT_REGISTER;
      outStep->bytes[2] = (uint8_t)nameBuf[0];
      outStep->size = 3;
    }
    else {
      outStep->bytes[1] = codeParam;
      outStep->size = 2;
    }
    return p;
  }

  if(strncasecmp(p, "MVAR ", 5) == 0 || strncasecmp(p, "MVAR\t", 5) == 0 || strncasecmp(p, "MVAR'", 5) == 0 || strncasecmp(p, "MVAR\"", 5) == 0) {
    p += 4;
    char argToken[64];
    p = extractArgToken(p, argToken, sizeof(argToken));
    uint8_t codeParam = 0;
    char nameBuf[32];
    uint8_t nameLen = 0;
    if(!parseLabelArg(argToken, &codeParam, nameBuf, &nameLen)) {
      snprintf(errMsg, errLen, "Invalid MVAR argument '%s'", argToken);
      return NULL;
    }
    if(codeParam == INDIRECT_REGISTER || codeParam == INDIRECT_VARIABLE) {
      snprintf(errMsg, errLen, "MVAR cannot have indirect argument '%s'", argToken);
      return NULL;
    }
    outStep->bytes[0] = (uint8_t)((ITM_MVAR >> 8) | 0x80);
    outStep->bytes[1] = (uint8_t)(ITM_MVAR & 0xff);
    outStep->bytes[2] = STRING_LABEL_VARIABLE;
    outStep->bytes[3] = nameLen;
    memcpy(&outStep->bytes[4], nameBuf, nameLen);
    outStep->size = (uint16_t)(4 + nameLen);
    return p;
  }

  // 6. General command
  char cmd[64];
  size_t ci = 0;
  while(*p && *p != ' ' && *p != '\t' && ci < sizeof(cmd) - 1) {
    cmd[ci++] = *p++;
  }
  cmd[ci] = '\0';

  int structOp = -1;
  int structSubscript = -1;
  bool isStructWithSub = parseStructuredCommandWithSubscript(cmd, &structOp, &structSubscript);

  int op = -1;
  if(isStructWithSub && structSubscript > 0) {
    op = structOp;
  }
  else {
    op = findItemOpcode(cmd);
    if(op < 0 && isStructWithSub) {
      op = structOp;
    }
  }
  if(op < 0) {
    snprintf(errMsg, errLen, "Unknown command '%s'", cmd);
    return NULL;
  }

  uint16_t ptp = indexOfItems[op].status & PTP_STATUS;
  if(ptp == PTP_NONE) {
    char peekArg[64];
    extractArgToken(p, peekArg, sizeof(peekArg));
    if(peekArg[0] != '\0') {
      if(op == 1833) { // DEC -> DECR when followed by a register operand
        uint8_t dummyCode = 0;
        char dummyBuf[32];
        uint8_t dummyLen = 0;
        if(parseRegisterArg(peekArg, &dummyCode, dummyBuf, &dummyLen)) {
          op = 91; // ITM_DECR
          ptp = indexOfItems[op].status & PTP_STATUS;
        }
      }
      else if(op == 1956 || op == 1957 || op == 1999 || op == 2000 || op == 2001 || op == 2002) {
        bool isShiftArg = ((peekArg[0] >= '0' && peekArg[0] <= '9') ||
                           strncmp(peekArg, "->", 2) == 0 ||
                           ((uint8_t)peekArg[0] == 0xe2 && (uint8_t)peekArg[1] == 0x86 && (uint8_t)peekArg[2] == 0x92) ||
                           strncasecmp(peekArg, "IND ", 4) == 0 ||
                           strncasecmp(peekArg, "IND\t", 4) == 0);
        if(isShiftArg) {
          if(op == 1956) {
            op = 411; // RLCn
          }
          else if(op == 1957) {
            op = 413; // RRCn
          }
          else if(op == 1999) {
            op = 414; // SLn
          }
          else if(op == 2000) {
            op = 415; // SRn
          }
          else if(op == 2001) {
            op = 410; // RLn
          }
          else if(op == 2002) {
            op = 412; // RRn
          }
          ptp = indexOfItems[op].status & PTP_STATUS;
        }
      }
    }
  }

  uint16_t opBytes = 0;
  if(op < 128) {
    outStep->bytes[0] = (uint8_t)op;
    opBytes = 1;
  }
  else {
    outStep->bytes[0] = (uint8_t)((op >> 8) | 0x80);
    outStep->bytes[1] = (uint8_t)(op & 0xff);
    opBytes = 2;
  }

  if(ptp == PTP_NONE) {
    outStep->size = opBytes;
    return p;
  }

  if(ptp == PTP_SKIP_BACK && isStructWithSub && structSubscript >= 0) {
    outStep->bytes[opBytes] = (uint8_t)(structSubscript & 0xff);
    outStep->size = (uint16_t)(opBytes + 1);
    return p;
  }

  char argToken[256];
  p = extractArgToken(p, argToken, sizeof(argToken));
  if(argToken[0] == '\0') {
    if(ptp == PTP_SKIP_BACK && isStructWithSub) {
      outStep->bytes[opBytes] = 0;
      outStep->size = (uint16_t)(opBytes + 1);
      return p;
    }
    snprintf(errMsg, errLen, "%s requires an argument", cmd);
    return NULL;
  }

  if(ptp == PTP_REGISTER) {
    uint8_t regCode = 0;
    char varBuf[32];
    uint8_t varLen = 0;
    if(!parseRegisterArg(argToken, &regCode, varBuf, &varLen)) {
      snprintf(errMsg, errLen, "%s requires a register or variable operand", cmd);
      return NULL;
    }
    if(regCode == STRING_LABEL_VARIABLE || regCode == INDIRECT_VARIABLE) {
      outStep->bytes[opBytes] = regCode;
      outStep->bytes[opBytes + 1] = varLen;
      memcpy(&outStep->bytes[opBytes + 2], varBuf, varLen);
      outStep->size = (uint16_t)(opBytes + 2 + varLen);
    }
    else if(regCode == INDIRECT_REGISTER) {
      outStep->bytes[opBytes] = INDIRECT_REGISTER;
      outStep->bytes[opBytes + 1] = (uint8_t)varBuf[0];
      outStep->size = (uint16_t)(opBytes + 2);
    }
    else {
      outStep->bytes[opBytes] = regCode;
      outStep->size = (uint16_t)(opBytes + 1);
    }
    return p;
  }

  if(ptp == PTP_LABEL || ptp == PTP_DECLARE_LABEL || ptp == PTP_MENU) {
    uint8_t codeParam = 0;
    char nameBuf[32];
    uint8_t nameLen = 0;
    if(!parseLabelArg(argToken, &codeParam, nameBuf, &nameLen)) {
      snprintf(errMsg, errLen, "%s requires a label/menu operand", cmd);
      return NULL;
    }
    if(ptp == PTP_DECLARE_LABEL && (codeParam == INDIRECT_REGISTER || codeParam == INDIRECT_VARIABLE)) {
      snprintf(errMsg, errLen, "%s cannot have indirect argument '%.120s'", cmd, argToken);
      return NULL;
    }
    if(codeParam == STRING_LABEL_VARIABLE || codeParam == LOCAL_LABEL_VARIABLE || codeParam == INDIRECT_VARIABLE) {
      outStep->bytes[opBytes] = codeParam;
      outStep->bytes[opBytes + 1] = nameLen;
      memcpy(&outStep->bytes[opBytes + 2], nameBuf, nameLen);
      outStep->size = (uint16_t)(opBytes + 2 + nameLen);
    }
    else if(codeParam == INDIRECT_REGISTER) {
      outStep->bytes[opBytes] = INDIRECT_REGISTER;
      outStep->bytes[opBytes + 1] = (uint8_t)nameBuf[0];
      outStep->size = (uint16_t)(opBytes + 2);
    }
    else {
      outStep->bytes[opBytes] = codeParam;
      outStep->size = (uint16_t)(opBytes + 1);
    }
    return p;
  }

  if(ptp == PTP_COMPARE) {
    uint8_t cmpCode = 0;
    char varBuf[32];
    uint8_t varLen = 0;
    if(!parseComparisonArg(argToken, &cmpCode, varBuf, &varLen)) {
      snprintf(errMsg, errLen, "%s requires a comparison operand (0, 1, or register)", cmd);
      return NULL;
    }
    if(cmpCode == STRING_LABEL_VARIABLE || cmpCode == INDIRECT_VARIABLE) {
      outStep->bytes[opBytes] = cmpCode;
      outStep->bytes[opBytes + 1] = varLen;
      memcpy(&outStep->bytes[opBytes + 2], varBuf, varLen);
      outStep->size = (uint16_t)(opBytes + 2 + varLen);
    }
    else if(cmpCode == INDIRECT_REGISTER) {
      outStep->bytes[opBytes] = INDIRECT_REGISTER;
      outStep->bytes[opBytes + 1] = (uint8_t)varBuf[0];
      outStep->size = (uint16_t)(opBytes + 2);
    }
    else {
      outStep->bytes[opBytes] = cmpCode;
      outStep->size = (uint16_t)(opBytes + 1);
    }
    return p;
  }

  bool isIndirect = (strncmp(argToken, "->", 2) == 0 ||
                     ((uint8_t)argToken[0] == 0xe2 && (uint8_t)argToken[1] == 0x86 && (uint8_t)argToken[2] == 0x92) ||
                     strncasecmp(argToken, "IND ", 4) == 0 ||
                     strncasecmp(argToken, "IND\t", 4) == 0);

  if(ptp == PTP_FLAG) {
    if(isIndirect) {
      uint8_t regCode = 0;
      char varBuf[32];
      uint8_t varLen = 0;
      if(!parseRegisterArg(argToken, &regCode, varBuf, &varLen)) {
        snprintf(errMsg, errLen, "%s invalid indirect argument '%.120s'", cmd, argToken);
        return NULL;
      }
      if(regCode == INDIRECT_VARIABLE) {
        outStep->bytes[opBytes] = INDIRECT_VARIABLE;
        outStep->bytes[opBytes + 1] = varLen;
        memcpy(&outStep->bytes[opBytes + 2], varBuf, varLen);
        outStep->size = (uint16_t)(opBytes + 2 + varLen);
        return p;
      }
      else if(regCode == INDIRECT_REGISTER) {
        outStep->bytes[opBytes] = INDIRECT_REGISTER;
        outStep->bytes[opBytes + 1] = (uint8_t)varBuf[0];
        outStep->size = (uint16_t)(opBytes + 2);
        return p;
      }
      else {
        snprintf(errMsg, errLen, "%s invalid indirect argument '%.120s'", cmd, argToken);
        return NULL;
      }
    }
    uint8_t flagCode = 0;
    uint8_t sysFlagCode = 0;
    bool isSysFlag = false;
    if(!parseFlagArg(argToken, &flagCode, &sysFlagCode, &isSysFlag)) {
      snprintf(errMsg, errLen, "%s requires a valid flag operand", cmd);
      return NULL;
    }
    if(isSysFlag) {
      outStep->bytes[opBytes] = SYSTEM_FLAG_NUMBER;
      outStep->bytes[opBytes + 1] = sysFlagCode;
      outStep->size = (uint16_t)(opBytes + 2);
    }
    else {
      outStep->bytes[opBytes] = flagCode;
      outStep->size = (uint16_t)(opBytes + 1);
    }
    return p;
  }

  if(ptp == PTP_SHUFFLE) {
    if(isIndirect) {
      uint8_t regCode = 0;
      char varBuf[32];
      uint8_t varLen = 0;
      if(!parseRegisterArg(argToken, &regCode, varBuf, &varLen)) {
        snprintf(errMsg, errLen, "%s invalid indirect argument '%.120s'", cmd, argToken);
        return NULL;
      }
      if(regCode == INDIRECT_VARIABLE) {
        outStep->bytes[opBytes] = INDIRECT_VARIABLE;
        outStep->bytes[opBytes + 1] = varLen;
        memcpy(&outStep->bytes[opBytes + 2], varBuf, varLen);
        outStep->size = (uint16_t)(opBytes + 2 + varLen);
        return p;
      }
      else if(regCode == INDIRECT_REGISTER) {
        outStep->bytes[opBytes] = INDIRECT_REGISTER;
        outStep->bytes[opBytes + 1] = (uint8_t)varBuf[0];
        outStep->size = (uint16_t)(opBytes + 2);
        return p;
      }
    }
    if(strlen(argToken) != 4) {
      snprintf(errMsg, errLen, "%s requires a 4-letter xyzt shuffle pattern", cmd);
      return NULL;
    }
    uint8_t shufByte = 0;
    for(int si = 0; si < 4; si++) {
      char sc = argToken[si];
      uint8_t bits = 0;
      if(sc == 'x' || sc == 'X') {
        bits = 0;
      }
      else if(sc == 'y' || sc == 'Y') {
        bits = 1;
      }
      else if(sc == 'z' || sc == 'Z') {
        bits = 2;
      }
      else if(sc == 't' || sc == 'T') {
        bits = 3;
      }
      else {
        snprintf(errMsg, errLen, "%s invalid shuffle register '%c'", cmd, sc);
        return NULL;
      }
      shufByte |= (uint8_t)(bits << (si * 2));
    }
    outStep->bytes[opBytes] = shufByte;
    outStep->size = (uint16_t)(opBytes + 1);
    return p;
  }

  if(ptp == PTP_REM) {
    const char *rs = argToken;
    size_t rlen = strlen(rs);
    if(rlen >= 2 && ((rs[0] == '\'' && rs[rlen - 1] == '\'') || (rs[0] == '"' && rs[rlen - 1] == '"'))) {
      rs++;
      rlen -= 2;
    }
    else if(rlen >= 6 && (uint8_t)rs[0] == 0xe2 && (uint8_t)rs[1] == 0x80 &&
            (((uint8_t)rs[2] == 0x98 && (uint8_t)rs[rlen - 3] == 0xe2 && (uint8_t)rs[rlen - 2] == 0x80 && (uint8_t)rs[rlen - 1] == 0x99) ||
             ((uint8_t)rs[2] == 0x9c && (uint8_t)rs[rlen - 3] == 0xe2 && (uint8_t)rs[rlen - 2] == 0x80 && (uint8_t)rs[rlen - 1] == 0x9d))) {
      rs += 3;
      rlen -= 6;
    }
    char rawRem[256];
    if(rlen >= sizeof(rawRem)) {
      rlen = sizeof(rawRem) - 1;
    }
    memcpy(rawRem, rs, rlen);
    rawRem[rlen] = '\0';
    char c47Rem[256];
    readableUtf8ToC47String(rawRem, c47Rem, sizeof(c47Rem));
    size_t c47Len = strlen(c47Rem);
    if(c47Len > 120) {
      c47Len = 120;
    }
    outStep->bytes[opBytes] = STRING_LABEL_VARIABLE;
    outStep->bytes[opBytes + 1] = (uint8_t)c47Len;
    memcpy(&outStep->bytes[opBytes + 2], c47Rem, c47Len);
    outStep->size = (uint16_t)(opBytes + 2 + c47Len);
    return p;
  }

  if(ptp == PTP_NUMBER_8 || ptp == PTP_SKIP_BACK || ptp == PTP_NUMBER_8_16 || ptp == PTP_KEYG_KEYX) {
    if(isIndirect) {
      uint8_t regCode = 0;
      char varBuf[32];
      uint8_t varLen = 0;
      if(!parseRegisterArg(argToken, &regCode, varBuf, &varLen)) {
        snprintf(errMsg, errLen, "%s invalid indirect argument '%.120s'", cmd, argToken);
        return NULL;
      }
      if(regCode == INDIRECT_VARIABLE) {
        outStep->bytes[opBytes] = INDIRECT_VARIABLE;
        outStep->bytes[opBytes + 1] = varLen;
        memcpy(&outStep->bytes[opBytes + 2], varBuf, varLen);
        outStep->size = (uint16_t)(opBytes + 2 + varLen);
        return p;
      }
      else if(regCode == INDIRECT_REGISTER) {
        outStep->bytes[opBytes] = INDIRECT_REGISTER;
        outStep->bytes[opBytes + 1] = (uint8_t)varBuf[0];
        outStep->size = (uint16_t)(opBytes + 2);
        return p;
      }
      else {
        snprintf(errMsg, errLen, "%s invalid indirect argument '%.120s'", cmd, argToken);
        return NULL;
      }
    }
    int num = atoi(argToken);
    if(ptp == PTP_NUMBER_8_16 && num >= 250) {
      outStep->bytes[opBytes] = (uint8_t)CNST_BEYOND_250;
      outStep->bytes[opBytes + 1] = (uint8_t)(num - 250);
      outStep->size = (uint16_t)(opBytes + 2);
    }
    else {
      outStep->bytes[opBytes] = (uint8_t)(num & 0xff);
      outStep->size = (uint16_t)(opBytes + 1);
    }
    return p;
  }

  if(ptp == PTP_NUMBER_16) {
    if(isIndirect) {
      uint8_t regCode = 0;
      char varBuf[32];
      uint8_t varLen = 0;
      if(!parseRegisterArg(argToken, &regCode, varBuf, &varLen)) {
        snprintf(errMsg, errLen, "%s invalid indirect argument '%.120s'", cmd, argToken);
        return NULL;
      }
      if(regCode == INDIRECT_VARIABLE) {
        outStep->bytes[opBytes] = INDIRECT_VARIABLE;
        outStep->bytes[opBytes + 1] = varLen;
        memcpy(&outStep->bytes[opBytes + 2], varBuf, varLen);
        outStep->size = (uint16_t)(opBytes + 2 + varLen);
        return p;
      }
      else if(regCode == INDIRECT_REGISTER) {
        outStep->bytes[opBytes] = INDIRECT_REGISTER;
        outStep->bytes[opBytes + 1] = (uint8_t)varBuf[0];
        outStep->size = (uint16_t)(opBytes + 2);
        return p;
      }
      else {
        snprintf(errMsg, errLen, "%s invalid indirect argument '%.120s'", cmd, argToken);
        return NULL;
      }
    }
    int num = atoi(argToken);
    extern bool_t isFunctionOldParam16(uint16_t func);
    if(isFunctionOldParam16(op)) {
      outStep->bytes[opBytes] = (uint8_t)(num & 0xff);
      outStep->bytes[opBytes + 1] = (uint8_t)((num >> 8) & 0xff);
    }
    else {
      outStep->bytes[opBytes] = (uint8_t)((num >> 8) & 0xff);
      outStep->bytes[opBytes + 1] = (uint8_t)(num & 0xff);
    }
    outStep->size = (uint16_t)(opBytes + 2);
    return p;
  }

  outStep->size = opBytes;
  return p;
}

char *parseAndPasteProgram(const char *codeText) {
  if(codeText == NULL || *codeText == '\0') {
    return strdup("Empty clipboard content");
  }

  bool hasLineNumbers = documentHasLineNumbers(codeText);
  bool reachedFirstStep = !hasLineNumbers;

  EncodedStep *steps = (EncodedStep *)malloc(16384 * sizeof(EncodedStep));
  if(steps == NULL) {
    return strdup("Out of memory");
  }
  int stepCount = 0;
  char errMsg[256];
  errMsg[0] = '\0';

  const char *p = codeText;
  int lineNum = 1;
  while(*p) {
    const char *lineStart = p;
    while(*p && *p != '\n' && *p != '\r') {
      p++;
    }
    size_t lineLen = (size_t)(p - lineStart);
    if(lineLen >= 1024) {
      lineLen = 1023;
    }
    char lineBuf[1024];
    memcpy(lineBuf, lineStart, lineLen);
    lineBuf[lineLen] = '\0';

    if(*p == '\r') {
      p++;
    }
    if(*p == '\n') {
      p++;
    }

    char normLine[1024];
    normalizeProgramText(lineBuf, normLine, sizeof(normLine));

    if(!reachedFirstStep) {
      if(hasProgramLinePrefix(normLine)) {
        reachedFirstStep = true;
      }
      else {
        lineNum++;
        continue;
      }
    }

    const char *prefixed = hasLineNumbers ? stripProgramLinePrefix(normLine) : normLine;
    char cleanBuf[1024];
    strncpy(cleanBuf, prefixed, sizeof(cleanBuf) - 1);
    cleanBuf[sizeof(cleanBuf) - 1] = '\0';
    stripProgramComment(cleanBuf);

    if(stepCount == 0 && isProgramHeaderLine(cleanBuf)) {
      lineNum++;
      continue;
    }

    if(hasLineNumbers && stepCount > 0 && !hasProgramLinePrefix(normLine)) {
      EncodedStep *last = &steps[stepCount - 1];
      if(last->size == 2 && last->bytes[0] == (uint8_t)((ITM_END >> 8) | 0x80) && last->bytes[1] == (uint8_t)(ITM_END & 0xff)) {
        lineNum++;
        continue;
      }
    }

    const char *lp = cleanBuf;
    while(*lp) {
      while(*lp == ' ' || *lp == '\t') {
        lp++;
      }
      if(*lp == '\0') {
        break;
      }
      if(stepCount >= 16384) {
        free(steps);
        return strdup("Program too large (maximum 16384 steps)");
      }
      EncodedStep *curStep = &steps[stepCount];
      const char *nextLp = parseInstructionToken(lp, curStep, errMsg, sizeof(errMsg));
      if(nextLp == NULL) {
        char finalErr[300];
        snprintf(finalErr, sizeof(finalErr), "Line %d: %s", lineNum, errMsg);
        free(steps);
        return strdup(finalErr);
      }
      lp = nextLp;
      if(curStep->size > 0) {
        bool isCurEnd = (curStep->size == 2 &&
                         curStep->bytes[0] == (uint8_t)((ITM_END >> 8) | 0x80) &&
                         curStep->bytes[1] == (uint8_t)(ITM_END & 0xff));
        bool isPrevEnd = (stepCount > 0 &&
                          steps[stepCount - 1].size == 2 &&
                          steps[stepCount - 1].bytes[0] == (uint8_t)((ITM_END >> 8) | 0x80) &&
                          steps[stepCount - 1].bytes[1] == (uint8_t)(ITM_END & 0xff));
        if(!(isCurEnd && isPrevEnd)) {
          stepCount++;
        }
      }
    }
    lineNum++;
  }

  if(stepCount == 0) {
    free(steps);
    return strdup("No executable program steps found");
  }

  bool endsWithEnd = false;
  if(stepCount > 0) {
    EncodedStep *last = &steps[stepCount - 1];
    if(last->size == 2 && last->bytes[0] == (uint8_t)((ITM_END >> 8) | 0x80) && last->bytes[1] == (uint8_t)(ITM_END & 0xff)) {
      endsWithEnd = true;
    }
  }
  if(!endsWithEnd && stepCount < 16384) {
    steps[stepCount].bytes[0] = (uint8_t)((ITM_END >> 8) | 0x80);
    steps[stepCount].bytes[1] = (uint8_t)(ITM_END & 0xff);
    steps[stepCount].size = 2;
    stepCount++;
  }

  if(calcMode != CM_PEM) {
    free(steps);
    return strdup("Calculator is not in PRGM mode");
  }

  defineCurrentProgramFromCurrentStep();

  if(programList == NULL || firstFreeProgramByte == NULL) {
    free(steps);
    return NULL;
  }

  uint8_t *startPtr = programList[currentProgramNumber - 1].instructionPointer;
  uint8_t *endPtr = endOfCurrentProgram - ((currentProgramNumber == numberOfPrograms) ? 2 : 0);

  if(startPtr != NULL && endPtr != NULL && startPtr < endPtr) {
    deleteStepsFromTo(startPtr, endPtr);
    scanLabelsAndPrograms();
    defineCurrentProgramFromCurrentStep();
    startPtr = programList[currentProgramNumber - 1].instructionPointer;
  }

  if(startPtr == NULL) {
    free(steps);
    return NULL;
  }

  uint32_t totalBytes = 0;
  for(int i = 0; i < stepCount; i++) {
    totalBytes += steps[i].size;
  }

  if(totalBytes > 0) {
    if(freeProgramBytes < totalBytes) {
      uint8_t *oldBeginOfProgramMemory = beginOfProgramMemory;
      uint32_t programSizeInBlocks = RAM_SIZE_IN_BLOCKS - TO_C47MEMPTR(beginOfProgramMemory);
      uint32_t newProgramSizeInBlocks = TO_BLOCKS(TO_BYTES(programSizeInBlocks) - freeProgramBytes + totalBytes);
      freeProgramBytes += TO_BYTES(newProgramSizeInBlocks - programSizeInBlocks);
      resizeProgramMemory(newProgramSizeInBlocks);
      currentStep           = currentStep           - oldBeginOfProgramMemory + beginOfProgramMemory;
      firstDisplayedStep    = firstDisplayedStep    - oldBeginOfProgramMemory + beginOfProgramMemory;
      beginOfCurrentProgram = beginOfCurrentProgram - oldBeginOfProgramMemory + beginOfProgramMemory;
      endOfCurrentProgram   = endOfCurrentProgram   - oldBeginOfProgramMemory + beginOfProgramMemory;
      startPtr              = startPtr              - oldBeginOfProgramMemory + beginOfProgramMemory;
    }

    for(uint8_t *pos = firstFreeProgramByte + 1 + totalBytes; pos > startPtr; --pos) {
      *pos = *(pos - totalBytes);
    }

    uint8_t *dest = startPtr;
    for(int i = 0; i < stepCount; i++) {
      memcpy(dest, steps[i].bytes, steps[i].size);
      dest += steps[i].size;
    }

    firstFreeProgramByte += totalBytes;
    freeProgramBytes -= totalBytes;
    endOfCurrentProgram += totalBytes;

    *(firstFreeProgramByte)     = 0xffu;
    *(firstFreeProgramByte + 1) = 0xffu;

    scanLabelsAndPrograms();
    goToGlobalStep(programList[currentProgramNumber - 1].step);
  }

  free(steps);

  reDraw = true;
  screenUpdatingMode = SCRUPD_AUTO;
  refreshScreen(1);
  refreshLcd(NULL);
  lcd_refresh();

  return NULL;
}
