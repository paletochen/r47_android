// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file charString.c
 ***********************************************/

#include "c47.h"

#if !defined(TESTSUITE_BUILD)
  /* Returns the character code from the first glyph of a string.
   *
   * \param[in]     ch     String whose first glyph is to extract
   * \param[in,out] offset Offset which is updated, or null if zero and no update
   * \return Character code for that glyph
   */
  uint16_t charCodeFromString(const char *ch, uint16_t *offset) {
    uint16_t charCode;
    uint16_t loffset = (offset != 0) ? *offset : 0;

    charCode = (uint8_t)ch[loffset++];
    if(charCode &0x0080) {
      charCode = (charCode << 8) | (uint8_t)ch[loffset++];
    }
    if(offset != 0) {
      *offset = loffset;
    }
    return charCode;
  }
#endif //TESTSUITE_BUILD


void convertDigits(char * refstr, char * outstr) {
  uint16_t ii = 0;
  uint16_t oo = 0;
  outstr[0] = 0;

  while(refstr[ii] != 0) {
    switch(refstr[ii]) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': outstr[oo++] = STD_SUB_0[0]; outstr[oo++] = STD_SUB_0[1] + refstr[ii] - '0'; break; //.
      case 't':
      case 'i':
      case 'c':
      case 'k':
      case 'x':
      case 'y':
      case 'a':
      case 's': outstr[oo++] = STD_SUB_a[0]           ; outstr[oo++] = STD_SUB_a[1] + refstr[ii] - 'a'; break;
      case ':': outstr[oo++] = STD_RATIO[0]           ; outstr[oo++] = STD_RATIO[1]           ; break; //:
      case '+': outstr[oo++] = STD_SUB_PLUS[0]        ; outstr[oo++] = STD_SUB_PLUS[1]        ; break; //+
      case '-': outstr[oo++] = STD_SUB_MINUS[0]       ; outstr[oo++] = STD_SUB_MINUS[1]       ; break; //-
      case ',': outstr[oo++] = STD_SINGLE_LOW_QUOTE[0]; outstr[oo++] = STD_SINGLE_LOW_QUOTE[1]; break; //.
      case '/': outstr[oo++] = STD_OBLIQUE3[0]        ; outstr[oo++] = STD_OBLIQUE3[1]        ; break; ///
      case '.':
      default:  outstr[oo++] = refstr[ii];
    }
    ii++;
  }
  outstr[oo] = 0;
}


typedef struct {
  uint16_t Nr;              ///<
  char     *inStr;          ///<
  char     *outStr;         ///<
} replaceTable_t;


  TO_QSPI const replaceTable_t replacementTable[] = {
    //        Nr    InStr          OutStr
              {0,   STD_SUP_MINUS, STD_HP_MINUS},
              {0,   STD_MINUS    , STD_HP_MINUS},
              {0,   STD_SUP_PLUS , STD_HP_PLUS},
              {0,   STD_PLUS     , STD_HP_PLUS},
              {0,   STD_EulerE   , STD_e},
              {0,   STD_op_i     , STD_i},
              {0,   STD_op_j     , STD_j},
              {0,   STD_WDOT     , STD_HP_PERIOD},
              {0,   STD_CROSS    , STD_SPACE_PUNCTUATION},
              {0,   STD_DOT      , STD_SPACE_PUNCTUATION},
              {0,   STD_SUB_10   , STD_SPACE_4_PER_EM},
              {0,   STD_0        , STD_HP_0},
              {0,   STD_SUP_0    , STD_HP_0}
            };

  #if !defined(TESTSUITE_BUILD)
  bool_t replace(uint16_t *charCode) {
    uint_fast16_t n = nbrOfElements(replacementTable);
    for(uint_fast16_t i=0; i<n; i++) {
      if(*charCode == charCodeFromString(replacementTable[i].inStr, 0)) {
        *charCode = charCodeFromString(replacementTable[i].outStr, 0);
        return true;
      }
    }
    return false;
  }
  #endif //TESTSUITE_BUILD


  #if !defined(GENERATE_CATALOGS)
  void charCodeHPReplacement(uint16_t *charCode) {
    #if !defined(TESTSUITE_BUILD)
      if(replace(charCode)) {
      }
      else if(*charCode >= charCodeFromString(STD_1, 0) &&  *charCode <= charCodeFromString(STD_9, 0)) {
        *charCode = *charCode - charCodeFromString(STD_1, 0) + charCodeFromString(STD_HP_1, 0);
      }
      else if(*charCode >= charCodeFromString(STD_SUP_1, 0) &&  *charCode <= charCodeFromString(STD_SUP_9, 0)) {
        *charCode = *charCode - charCodeFromString(STD_SUP_1, 0) + charCodeFromString(STD_HP_1, 0);
      }
    #endif //TESTSUITE_BUILD
  }
  #endif //GENERATE_CATALOGS






void expandConversionName(char *msg1) {   // 2x16+1 character limit, rounded up to 50
  int16_t i = 0;
  int16_t jj = 0;
  char inStr[51];
  xcopy(inStr, msg1, min(50,stringByteLength(msg1)+1));
  inStr[50]=0;
  msg1[0]=0;
  while(inStr[i] != 0) { //replace /U with /kWh; U/ with kWh/; hkm with 100km
    if('h' == inStr[i] && 'k' == inStr[i+1] && 'm' == inStr[i+2]) {    //test beyond end of string is ok, it will not test positive
      msg1[jj++] = '1'; i++;
      msg1[jj++] = '0'; i++;
      msg1[jj++] = '0'; i++;
      msg1[jj++] = 'k';
      msg1[jj++] = 'm';
    }
    else if('/' == inStr[i] && 'E' == inStr[i+1]) {
      msg1[jj++] = '/'; i++;
      msg1[jj++] = 'k'; i++;
      msg1[jj++] = 'W';
      msg1[jj++] = 'h';
    }
    else if('E' == inStr[i] && '/' == inStr[i+1]) {
      msg1[jj++] = 'k'; i++;
      msg1[jj++] = 'W'; i++;
      msg1[jj++] = 'h';
      msg1[jj++] = '/';
    }
    else {
      msg1[jj++]=inStr[i++];
    }
  }
  msg1[jj++]=0;
}


void compressConversionName(char *msg1) {   // 2x16+1 character limit, rounded up to 50
  int16_t i = 0;
  int16_t jj = 0;
  char inStr[51];
  xcopy(inStr, msg1, min(50,stringByteLength(msg1)+1));
  inStr[50]=0;
  msg1[0]=0;
  while(inStr[i] != 0) { //replace 100k with |ook; 100m with |oom; /kWh with /U; kWh/ with U/
    if('1' == inStr[i] && '0' == inStr[i+1] && '0' == inStr[i+2] && ('k' == inStr[i+3] || 'm' == inStr[i+3])) {    //test beyond end of string is ok, it will not test positive
      msg1[jj++] = STD_BINARY_ONE[0];
      msg1[jj++] = STD_BINARY_ONE[1];
      msg1[jj++] = STD_BINARY_ZERO[0];
      msg1[jj++] = STD_BINARY_ZERO[1];
      msg1[jj++] = STD_BINARY_ZERO[0];
      msg1[jj++] = STD_BINARY_ZERO[1];
      msg1[jj++] = inStr[i+3];          //k or m for km or mile
      i += 4;
    }
    else if('/' == inStr[i] && 'k' == inStr[i+1] && 'W' == inStr[i+2] && 'h' == inStr[i+3]) {
      msg1[jj++] = '/';
      msg1[jj++] = 'E';
      i += 4;
    }
    else if('k' == inStr[i] && 'W' == inStr[i+1] && 'h' == inStr[i+2] && '/' == inStr[i+3]) {
      msg1[jj++] = 'E';
      msg1[jj++] = '/';
      i += 4;
    }
    else {
      msg1[jj++]=inStr[i++];
    }
  }
  msg1[jj++]=0;
}



static void _calculateStringWidth(const char *str, const font_t *font, bool_t withLeadingEmptyRows, bool_t withEndingEmptyRows, int16_t *width, const char **resultStr) {
  uint16_t charCode;
  int16_t ch, numPixels, glyphId;
  const glyph_t *glyph;
  bool_t  firstChar;
  #if defined(GENERATE_CATALOGS)
    uint16_t doubling = false;
  #else //GENERATE_CATALOGS
    uint16_t doubling = (font == &numericFont && temporaryInformation == TI_NO_INFO && checkHP) ? DOUBLING : DOUBLINGBASEX;
  #endif //GENERATE_CATALOGS

//  const font_t  *font;  //JM

  glyph = NULL;
  firstChar = true;
  numPixels = 0;
  ch = 0;
  while(str[ch] != 0) {
    charCode = (uint8_t)str[ch++];
    if(charCode & 0x80) { // MSB set
      charCode = (charCode<<8) | (uint8_t)str[ch++];
    }

  #if !defined(GENERATE_CATALOGS)
    if(checkHP && font == &numericFont && HPFONT) {
      charCodeHPReplacement(&charCode);
    }
  #endif //GENERATE_CATALOGS

/*
    font = font1;                             //JM auto font change for enlarged alpha fonts vv
    #if !defined(TESTSUITE_BUILD)
      if(combinationFonts == 2) {
        if(maxiC == 1 && font == &numericFont) {
    #endif // !TESTSUITE_BUILD
    glyphId = findGlyph(font, charCode);
    if(glyphId < 0) {                         //JM if there is not a large glyph, width of the small letter
      font = &standardFont;
    }
    #if !defined(TESTSUITE_BUILD)
        }
      }                                       //JM ^^
    #endif // !TESTSUITE_BUILD
*/
    if(charCode != 1u) {                          //If the special ASCII 01, then skip the width and font not found portions
      glyphId = findGlyph(font, charCode);
      if(glyphId >= 0) {
        glyph = (font->glyphs) + glyphId;
      }
      else if(glyphId == -1) {
        generateNotFoundGlyph(-1, charCode);
        glyph = &glyphNotFound;
      }
      else if(glyphId == -2) {
        generateNotFoundGlyph(-2, charCode);
        glyph = &glyphNotFound;
      }
      else {
        glyph = NULL;
      }

    if(glyph == NULL) {
      #if defined(GENERATE_CATALOGS)
        printf("\n---------------------------------------------------------------------------\n"
                 "In function stringWidth: %d is an unexpected value returned by findGlyph!"
               "/n---------------------------------------------------------------------------\n", glyphId);
      #else // !GENERATE_CATALOGS
        sprintf(errorMessage, commonBugScreenMessages[bugMsgValueReturnedByFindGlyph], "_calculateStringWidth", glyphId);
        displayBugScreen(errorMessage);
      #endif // GENERATE_CATALOGS
      *width = 0;
      return;
    }

      numPixels += (doubling*(glyph->colsGlyph + glyph->colsAfterGlyph)) >> 3;
      if(firstChar) {
        firstChar = false;
        if(withLeadingEmptyRows) {
          numPixels += (doubling*(glyph->colsBeforeGlyph)) >> 3;
        }
      }
      else {
        numPixels += (doubling*(glyph->colsBeforeGlyph)) >> 3;
      }

      if(resultStr != NULL) { // for stringAfterPixels
        if(numPixels > *width) {
          break;
        }
        else {
          *resultStr = str + ch;
        }
      }
    }
  }

  if(glyph != NULL && withEndingEmptyRows == false) {
    numPixels -= (doubling*(glyph->colsAfterGlyph)) >> 3;
    if(resultStr != NULL && numPixels <= *width) { // for stringAfterPixels
      if((**resultStr) & 0x80) {
        *resultStr += 2;
      }
      else if((**resultStr) != 0) {
        *resultStr += 1;
      }
    }
  }
  *width = numPixels;
  return;
}


int16_t stringWidth(const char *str, const font_t *font, bool_t withLeadingEmptyRows, bool_t withEndingEmptyRows) {
  int16_t width = 0;
  _calculateStringWidth(str, font, withLeadingEmptyRows, withEndingEmptyRows, &width, NULL);
  return width;
}


char *stringAfterPixels(const char *str, const font_t *font, int16_t width, bool_t withLeadingEmptyRows, bool_t withEndingEmptyRows) {
  const char *resultStr = str;
  _calculateStringWidth(str, font, withLeadingEmptyRows, withEndingEmptyRows, &width, &resultStr);
  return (char *)resultStr;
}


int16_t stringNextGlyphNoEndCheck_JM(const char *str, int16_t pos) {    //Not checking for beyond terminator. Use only if no risk for pos > length(str)
  int16_t posinc = 0;
  if(str[pos] == 0) return pos;

  if(str[pos] & 0x80) {
    posinc = 2;
    if(str[pos+2] == 0) return pos+2;
  }
  else {
    posinc = 1;
    if(str[pos+1] == 0) return pos+1;
  }

  pos += posinc;
  return pos;
}


int16_t stringNextGlyph(const char *str, int16_t pos) {
  int16_t lg;

  lg = stringByteLength(str);
  if(pos >= lg) {
    return lg;
  }

  pos += (str[pos] & 0x80) ? 2 : 1;

  if(pos >= lg) {
    return lg;
  }
  else {
   return pos;
  }
}


int16_t stringPrevGlyph(const char *str, int16_t pos) {
  int16_t prev = 0;
  int16_t lg;

  lg = stringByteLength(str);
  if(pos >= lg) {
    pos = lg;
  }

  if(pos <= 1) {
    return 0;
  }
  else {
    int16_t cnt = 0;
    while(str[cnt] != 0 && cnt < pos) {
      prev = cnt;
      cnt = stringNextGlyph(str, cnt);
    }
  }
  return prev;
}


bool_t isValidNumber(const char *ss, const char *template){
  if(stringByteLength(ss) != stringByteLength(template)) {
    return false;
  }
  uint16_t nn = stringByteLength(ss);
  while(nn != 0) {
    if((template[nn-1] == '.' && !(ss[nn-1] == '.' || ss[nn-1] == ',')) ||
       (template[nn-1] == 'd' && !(ss[nn-1] >= '0' && ss[nn-1] <= '9')) ||
       (template[nn-1] == 's' && !(ss[nn-1] == '-' || ss[nn-1] == '+')) )  {
      return false;
    }
    nn--;
  }
  return true;
}


int16_t stringPrevNumberGlyph(const char *str, int16_t pos) {
  int16_t pos2 = pos;

  do {
    pos2 = stringPrevGlyph(str,pos2);

    if(('0' <= str[pos2] && str[pos2] <= '9') || str[pos] == '.' || str[pos] == ',') {
      return pos2;
    }
  } while (pos2 != 0);
  return 0;
}



int16_t stringLastGlyph(const char *str) {
  int16_t lastGlyph;

  if(str == NULL) {
    lastGlyph = -1;
  }
  else {
    int16_t lg = stringByteLength(str), next = 0;
    for(lastGlyph=0;;) {
      if(lastGlyph >= lg) {
        next = lg;
      }
      else {
        next += (str[lastGlyph] & 0x80) ? 2 : 1;

        if(next > lg) {
          next = lg;
        }
      }

      if(next == lg) {
        break;
      }

      lastGlyph = next;
    }
  }
  return lastGlyph;
}


//int32_t stringByteLength(const char *str) {
//  int32_t len = 0;
//
//  while(*str != 0) {
//    if(*str & 0x80) {
//      str += 2;
//      len += 2;
//    }
//    else {
//      str++;
//      len++;
//    }
//  }
//  return len;
//}


int32_t stringGlyphLength(const char *str) {
  int32_t len = 0;

  while(*str != 0) {
    if(*str & 0x80) {
      str += 2;
      len++;
    }
    else {
      str++;
      len++;
    }
  }
  return len;
}


void codePointToUtf8(uint32_t codePoint, uint8_t *utf8) { // C47 supports only unicode code points from 0x0000 to 0x7FFF
  if(codePoint <= 0x00007F) {
    utf8[0] = codePoint;
    utf8[1] = 0;
    utf8[2] = 0;
    utf8[3] = 0;
    utf8[4] = 0;
  }

  else if(codePoint <= 0x0007FF) {
    utf8[0] = 0xC0 | (codePoint >> 6);
    utf8[1] = 0x80 | (codePoint &  0x3F);
    utf8[2] = 0;
    utf8[3] = 0;
    utf8[4] = 0;
  }

  else /*if(codePoint <= 0x00FFFF)*/ {
    utf8[0] = 0xE0 |  (codePoint >> 12);
    utf8[1] = 0x80 | ((codePoint >>  6) & 0x3F);
    utf8[2] = 0x80 | ((codePoint      ) & 0x3F);
    utf8[3] = 0;
    utf8[4] = 0;
  }

  /*else if(codePoint <= 0x1FFFFF) {
    utf8[0] = 0xF0 |  (codePoint >> 18);
    utf8[1] = 0x80 | ((codePoint >> 12) & 0x3F);
    utf8[2] = 0x80 | ((codePoint >>  6) & 0x3F);
    utf8[3] = 0x80 | ((codePoint      ) & 0x3F);
    utf8[4] = 0;
  }

  else if(codePoint <= 0x3FFFFFF) {
    utf8[0] = 0xF8 |  (codePoint >> 24);
    utf8[1] = 0x80 | ((codePoint >> 18) & 0x3F);
    utf8[2] = 0x80 | ((codePoint >> 12) & 0x3F);
    utf8[3] = 0x80 | ((codePoint >>  6) & 0x3F);
    utf8[4] = 0x80 | ((codePoint      ) & 0x3F);
    utf8[5] = 0;
  }

  else if(codePoint <= 0x7FFFFFFF) {
    utf8[0] = 0xFC |  (codePoint >> 30);
    utf8[1] = 0x80 | ((codePoint >> 24) & 0x3F);
    utf8[2] = 0x80 | ((codePoint >> 18) & 0x3F);
    utf8[3] = 0x80 | ((codePoint >> 12) & 0x3F);
    utf8[4] = 0x80 | ((codePoint >>  6) & 0x3F);
    utf8[5] = 0x80 | ((codePoint      ) & 0x3F);
    utf8[6] = 0;
  }*/
}


uint32_t utf8ToCodePoint(const uint8_t *utf8, uint32_t *codePoint) { // C47 supports only unicode code points from 0x0000 to 0x7FFF
  if((*utf8 & 0x80) == 0) {
    *codePoint = *utf8;
    return 1;
  }

  else if((*utf8 & 0xE0) == 0xC0) {
    *codePoint =  (*utf8       & 0x1F) << 6;
    *codePoint |= (*(utf8 + 1) & 0x3F);
    return 2;
  }

  else /*if((*utf8 & 0xF0) == 0xE0)*/ {
    *codePoint =  (*utf8       & 0x0F) << 12;
    *codePoint |= (*(utf8 + 1) & 0x3F) <<  6;
    *codePoint |= (*(utf8 + 2) & 0x3F);
    return 3;
  }

  /*else if((*utf8 & 0xF8) == 0xF0) {
    *codePoint =  (*utf8       & 0x07) << 18;
    *codePoint |= (*(utf8 + 1) & 0x3F) << 12;
    *codePoint |= (*(utf8 + 2) & 0x3F) <<  6;
    *codePoint |= (*(utf8 + 3) & 0x3F);
    return 4;
  }

  else if((*utf8 & 0xFC) == 0xF8) {
    *codePoint =  (*utf8       & 0x03) << 24;
    *codePoint |= (*(utf8 + 1) & 0x3F) << 18;
    *codePoint |= (*(utf8 + 2) & 0x3F) << 12;
    *codePoint |= (*(utf8 + 3) & 0x3F) <<  6;
    *codePoint |= (*(utf8 + 4) & 0x3F);
    return 5;
  }

  else if((*utf8 & 0xFE) == 0xFC) {
    *codePoint =  (*utf8       & 0x01) << 30;
    *codePoint |= (*(utf8 + 1) & 0x3F) << 24;
    *codePoint |= (*(utf8 + 2) & 0x3F) << 18;
    *codePoint |= (*(utf8 + 3) & 0x3F) << 12;
    *codePoint |= (*(utf8 + 4) & 0x3F) <<  6;
    *codePoint |= (*(utf8 + 5) & 0x3F);
    return 6;
  }*/

  return 0;
}


void debug_utf8_string(const char *label, const uint8_t *str, size_t max_len) {
    printf("%s:", label);
    printf("  Hex:   ");
    for (size_t i = 0; i < max_len; i++) {
        printf("%02X ", str[i]);
    }
    printf("; ");

    printf("  Dec:   ");
    for (size_t i = 0; i < max_len; i++) {
        printf("%3d ", str[i]);
    }
    printf("; ");

    printf("  Char:  ");
    for (size_t i = 0; i < max_len; i++) {
        if (str[i] >= 32 && str[i] < 127) {
            printf(" %c  ", str[i]);
        } else {
            printf("    ");
        }
    }
    printf("\n");
}


void stringToUtf8(const char *str, uint8_t *utf8) {
  int16_t len;

  len = stringGlyphLength(str);

  if(len == 0) {
    *utf8 = 0;
    return;
  }

  for(int16_t i=0; i<len; i++) {
    if(*str & 0x80) {
      codePointToUtf8(((uint8_t)*str & 0x7F) * 256u + (uint8_t)str[1], utf8);
      str += 2;
      while(*utf8) {
        utf8++;
      }
    }
    else {
      *utf8 = *str;
      utf8++;
      str++;
      *utf8 = 0;
    }
  }
}

//Alternative stringToUtf8
//void stringToUtf8(const char *str, uint8_t *utf8) {
//    //uint8_t *original_utf8 = utf8;
//    //const char *original_str = str;
//
//    while (*str) {
//        if ((uint8_t)*str & 0x80) {
//            uint16_t high = ((uint16_t)(uint8_t)(*str) & 0x7F);
//            uint16_t low  = (uint8_t)str[1];
//            uint16_t codepoint = (high << 8) | low;
//            if (codepoint <= 0x7F) {
//                *utf8++ = (uint8_t)codepoint;
//            } else if (codepoint <= 0x7FF) {
//                // FIX: use (cp >> 6) & 0x1F to avoid stray upper bits
//                *utf8++ = 0xC0 | ((codepoint >> 6) & 0x1F);
//                *utf8++ = 0x80 | (codepoint & 0x3F);
//            } else {
//                *utf8++ = 0xE0 | ((codepoint >> 12) & 0x0F);
//                *utf8++ = 0x80 | ((codepoint >> 6) & 0x3F);
//                *utf8++ = 0x80 | (codepoint & 0x3F);
//            }
//
//            str += 2;
//        } else {
//            *utf8++ = (uint8_t)*str++;
//        }
//    }
//
//    *utf8 = 0;
//
//    //printf("Original input: ");
//    //size_t input_len = str - original_str + 1;
//    //for (size_t i = 0; i < input_len; i++) {
//    //    printf("%02X ", (unsigned char)original_str[i]);
//    //}
//    //printf("\n");
//    //printf("UTF-8 output:   ");
//    //size_t output_len = utf8 - original_utf8 + 1;
//    //for (size_t i = 0; i < output_len; i++) {
//    //    printf("%02X ", original_utf8[i]);
//    //}
//    //printf("\n");
//}




void utf8ToString(const uint8_t *utf8, char *str) {
  uint32_t codePoint;

  while(*utf8) {
    utf8 += utf8ToCodePoint(utf8, &codePoint);
    if(codePoint < 0x0080) {
      *(str++) = codePoint;
    }
    else {
      codePoint |= 0x8000;
      *(str++) = codePoint >> 8;
      *(str++) = codePoint & 0x00FF;
    }
  }
  *str = 0;
}


typedef struct {
  char     *item_in;            ///<
  char     *item_out;           ///<
} function_t2;

TO_QSPI const function_t2 indexOfStringsASCII[] = {
              //number                  replacement string
//XPORTP CODE 2023-07-15
              {STD_NOT,                       "<>"},
              {STD_DEGREE,                    "deg"},
              {STD_PLUS_MINUS,                "+-"},
              {STD_A_GRAVE,                   "`A"},
              {STD_CROSS,                     "*"},
              {STD_U_GRAVE,                   "`U"},
              {STD_a_GRAVE,                   "`a"},
              {STD_DIVIDE,                    "/"},
              {STD_u_GRAVE,                   "`u"},
              {STD_A_BREVE,                   "`A"},
              {STD_a_BREVE,                   "`a"},
              {STD_E_MACRON,                  "`E"},
              {STD_e_MACRON,                  "`e"},
              {STD_I_MACRON,                  "`I"},
              {STD_i_MACRON,                  "`i"},
              {STD_I_BREVE,                   "`I"},
              {STD_i_BREVE,                   "`i"},
              {STD_U_BREVE,                   "`U"},
              {STD_u_BREVE,                   "`u"},
              {STD_Y_CIRC,                    "`Y"},
              {STD_y_CIRC,                    "`y"},
              {STD_y_BAR,                     "y_mean"},
              {STD_x_BAR,                     "x_mean"},
              {STD_x_CIRC,                    "x_circ"},
              {STD_ALPHA,                     "Alpha"},
              {STD_BETA,                      "Beta"},
              {STD_GAMMA,                     "Gamma"},
              {STD_DELTA,                     "Delta"},
              {STD_EPSILON,                   "Epsilon"},
              {STD_ZETA,                      "Zeta"},
              {STD_PI,                        "Pi"},
              {STD_SIGMA,                     "Sigma"},
              {STD_PHI,                       "Phi"},
              {STD_CHI,                       "Chi"},
              {STD_PSI,                       "Psi"},
              {STD_alpha,                     "alpha"},
              {STD_beta,                      "beta"},
              {STD_gamma,                     "gamma"},
              {STD_delta,                     "delta"},
              {STD_epsilon,                   "epsilon"},
              {STD_zeta,                      "zeta"},
              {STD_lambda,                    "lambda"},
              {STD_mu,                        "mu"},
              {STD_pi,                        "pi"},
              {STD_SUB_pi,                    "pi"},
              {STD_SUP_pi,                    "pi"},
              {STD_sigma,                     "sigma"},
              {STD_phi,                       "phi"},
              {STD_phi_m,                     "phi"},
              {STD_theta,                     "theta"},
              {STD_theta_m,                   "theta"},
              {STD_chi,                       "chi"},
              {STD_psi,                       "psi"},
              {STD_omega,                     "omega"},
              {STD_ELLIPSIS,                  "…"},
              {STD_BINARY_ONE,                "1"},
              {STD_SUB_E_OUTLINE,             "`E"},
              {STD_SUB_PLUS,                  "`+"},
              {STD_SUB_MINUS,                 "`-"},
              {STD_SUP_ASTERISK,              "`*"},
              {STD_SUP_INFINITY,              "inf"},
              {STD_SUB_INFINITY,              "inf"},
              {STD_PLANCK,                    "Planck"},
              {STD_PLANCK_2PI,                "Planck2pi"},
              {STD_SUP_PLUS,                  "+"},
              {STD_SUP_MINUS,                 "-"},
              {STD_BINARY_ZERO,               "0"},
              {STD_INFINITY,                  "inf"},
              {STD_MEASURED_ANGLE,            "angle"},
              {STD_INTEGRAL,                  "integral"},
              {STD_ALMOST_EQUAL,              "approx"},
              {STD_NOT_EQUAL,                 "<>"},
              {STD_LESS_EQUAL,                "<="},
              {STD_GREATER_EQUAL,             ">="},
              {STD_SUB_EARTH,                 "`earth"},
              {STD_SUB_alpha,                 "`alpha"},
              {STD_SUB_delta,                 "`delta"},
              {STD_SUB_mu,                    "`mu"},
              {STD_SUB_SUN,                   "`sun"},
              {STD_PRINTER,                   "Print"},
              {STD_UK,                        "UK"},
              {STD_US,                        "US"},
              {STD_GAUSS_BLACK_L,             "GAUSS_BL_L "},
              {STD_GAUSS_WHITE_R,             "GAUSS_WH_R "},
              {STD_GAUSS_WHITE_L,             "GAUSS_WH_L "},
              {STD_GAUSS_BLACK_R,             "GAUSS_BL_R "},
              {STD_SUB_10,                    "10^"},
              {STD_EulerE,                    "e"},
              {STD_RIGHT_OVER_LEFT_ARROW,     "<>"},
              {STD_SQUARE_ROOT,               "root"},
//diverse
              {STD_RIGHT_SINGLE_QUOTE,        "\'"},
              {STD_RIGHT_DOUBLE_QUOTE,        "\""},
              {STD_op_i,                      "i"},
              {STD_op_j,                      "j"},
              {STD_BINARY_ONE,                "1"},
              {STD_BINARY_ZERO,               "0"},
              {STD_CR,                        "|"},
//seps
              {STD_RIGHT_TACK,                "\'"},
              {STD_WDOT,                      "."},
              {STD_DOT,                       "."},
              {STD_WPERIOD,                   "."},
              {STD_WCOMMA,                    ","},
              {STD_NQUOTE,                    "\'"},
              {STD_INV_BRIDGE,                ","},
              {STD_SPACE_EM,                  " "},
              {STD_SPACE_3_PER_EM,            " "},
              {STD_SPACE_4_PER_EM,            " "},
              {STD_SPACE_6_PER_EM,            " "},
              {STD_SPACE_FIGURE,              " "},
              {STD_SPACE_PUNCTUATION,         " "},
              {STD_SPACE_HAIR,                " "},
//Types used in commands
              {STD_TIME_T,                    "T"},
              {STD_DATE_D,                    "D"},
              {STD_COMPLEX_C,                 "C"},
              {STD_INTEGER_Z,                 "Z"},
              {STD_INTEGER_Z_SMALL,           "Z"},
              {STD_NATURAL_N,                 "N"},
              {STD_RATIONAL_Q,                "Q"},
              {STD_IRRATIONAL_I,              "I"},
              {STD_REAL_R,                    "R"},
              {STD_u_BAR,                     "u_vec"},
              {STD_v_BAR,                     "v_vec"},
              {STD_w_BAR,                     "w_vec"},

              {STD_RIGHT_DOUBLE_ARROW ,       ">>"},

              {STD_RIGHT_DASHARROW    ,       "->"},
              {STD_RIGHT_ARROW        ,       "->"},
              {STD_RIGHT_SHORT_ARROW  ,       "->"},

              {STD_LEFT_DASHARROW     ,       "<-"},
              {STD_LEFT_ARROW         ,       "<-"},

              {STD_UP_DASHARROW       ,       "^" },
              {STD_UP_ARROW           ,       "^" },
              {STD_HOLLOW_UP_ARROW    ,       "^" },

              {STD_DOWN_DASHARROW     ,       "v" },
              {STD_DOWN_ARROW         ,       "v" },
              {STD_HOLLOW_DOWN_ARROW  ,       "v" },

              {STD_LEFT_RIGHT_ARROWS  ,       "><"},
              {STD_SUP_pir            ,       "pi"},

};


TO_QSPI const function_t2 indexOfStringsRTF[] = {              //Only STD codes 2 bytes, i.e. ASCII > 128 will arrive here
              //number                  replacement string
              {STD_BINARY_ONE,                "1"},
              {STD_BINARY_ZERO,               "0"},
           //   {STD_A_MACRON,                  "\x81\x00"},
           //   {STD_FOR_ALL,                   "\xa2\x00"}
};

  static char *stringAppend(char *dest, const char *source) {  //functio0n re-instated only for the export, where 0x00 is possible
    const size_t l = stringByteLength(source);
    return (char *)memcpy(dest, source, l + 1) + l;
  }

  static bool_t _getText(uint8_t a1, uint8_t a2, char *str) {
    //printf("_getText %c%c %u %u : ",(uint8_t)a1,(uint8_t)a2,(uint8_t)a1,(uint8_t)a2);
    str[0] = 0;
    uint_fast16_t n = nbrOfElements(indexOfStringsASCII);
    for(uint_fast16_t i=0; i<n; i++) {
      if((uint8_t)a1 == (uint8_t)(indexOfStringsASCII[i].item_in[0]) && (uint8_t)a2 == (uint8_t)(indexOfStringsASCII[i].item_in[1])) {
        //printf("(%u):%u %u %s\n", i,(uint8_t)(indexOfStringsASCII[i].item_in[0]), (uint8_t)(indexOfStringsASCII[i].item_in[1]),indexOfStringsASCII[i].item_out);
        stringAppend(str,indexOfStringsASCII[i].item_out);
        break;
      }
    }
    return str[0] != 0;
  }

  static bool_t _getTextRTF(uint8_t a1, uint8_t a2, char *str) {
    //printf("_getTextRTF %u %u : ",(uint8_t)a1,(uint8_t)a2);
    str[0] = 0;
    uint_fast16_t n = nbrOfElements(indexOfStringsRTF);
    for(uint_fast16_t i=0; i<n; i++) {
      if((uint8_t)a1 == (uint8_t)(indexOfStringsRTF[i].item_in[0]) && (uint8_t)a2 == (uint8_t)(indexOfStringsRTF[i].item_in[1])) {
        //printf("(%u):%u %u %s\n", i,(uint8_t)(indexOfStringsRTF[i].item_in[0]), (uint8_t)(indexOfStringsRTF[i].item_in[1]),indexOfStringsRTF[i].item_out);
        stringAppend(str,indexOfStringsRTF[i].item_out);
        break;
      }
    }
    return str[0] != 0;
  }


void stringToRTF(const char *str, char *ascii) {
  int16_t len;
  uint8_t a1, a2;
  char aa[32];
  char bb[2];
  int8_t supsub = 0;


  len = stringGlyphLength(str);

  if(len == 0) {
    *ascii = 0;
    return;
  }

  for(int16_t i=0; i<len; i++) {  // C47 supports only unicode code points from 0x0000 to 0x7FFF
    if(*str & 0x80) {
      bb[1] = 0;
      bb[0] = 0;

      a1=(uint8_t)*str;
      str++;
      a2=(uint8_t)*str;
      str++;

      //replacement table TO BE PLAINTEXT OUTPUT
      if(_getTextRTF(a1, a2, aa)) {
        int16_t j = 0;
        while(aa[j] != 0) {
          *ascii = aa[j++];
          ascii++;
        }
        ascii--;
      }

      else
      //RANGE SUP/SUB/BASE TO BE PLAINTEXT OUTPUT
      if((a1==(uint8_t)(STD_SUP_0   [0]) && (a2>=(uint8_t)(STD_SUP_0   [1]) && a2<=(uint8_t)(STD_SUP_9  [1]))) ) {supsub = +1; bb[0] = ('0'+a2)-(uint8_t)(STD_SUP_0 [1]);} else
      if((a1==(uint8_t)(STD_SUP_a   [0]) && (a2>=(uint8_t)(STD_SUP_a   [1]) && a2<=(uint8_t)(STD_SUP_z  [1]))) ) {supsub = +1; bb[0] = ('a'+a2)-(uint8_t)(STD_SUP_a [1]);} else
      if((a1==(uint8_t)(STD_SUP_A   [0]) && (a2>=(uint8_t)(STD_SUP_A   [1]) && a2<=(uint8_t)(STD_SUP_Z  [1]))) ) {supsub = +1; bb[0] = ('A'+a2)-(uint8_t)(STD_SUP_A [1]);} else
      if((a1==(uint8_t)(STD_SUB_0   [0]) && (a2>=(uint8_t)(STD_SUB_0   [1]) && a2<=(uint8_t)(STD_SUB_9  [1]))) ) {supsub = -1; bb[0] = ('0'+a2)-(uint8_t)(STD_SUB_0 [1]);} else
      if((a1==(uint8_t)(STD_SUB_a   [0]) && (a2>=(uint8_t)(STD_SUB_a   [1]) && a2<=(uint8_t)(STD_SUB_z  [1]))) ) {supsub = -1; bb[0] = ('a'+a2)-(uint8_t)(STD_SUB_a [1]);} else
      if((a1==(uint8_t)(STD_SUB_A   [0]) && (a2>=(uint8_t)(STD_SUB_A   [1]) && a2<=(uint8_t)(STD_SUB_Z  [1]))) ) {supsub = -1; bb[0] = ('A'+a2)-(uint8_t)(STD_SUB_A [1]);} else
//  ssss    if((a1==(uint8_t)(STD_BASE_0  [0]) && (a2==(uint8_t)(STD_BASE_0  [1])                                 )) ) {                        *ascii = ('0');} else
//      if((a1==(uint8_t)(STD_BASE_1  [0]) && (a2>=(uint8_t)(STD_BASE_1  [1]) && a2<=(uint8_t)(STD_BASE_9 [1]))) ) {*ascii = '#';  ascii++; *ascii = ('1'+a2)-(uint8_t)(STD_BASE_1[1]);} else
//      if((a1==(uint8_t)(STD_BASE_10 [0]) && (a2>=(uint8_t)(STD_BASE_10 [1]) && a2<=(uint8_t)(STD_BASE_16[1]))) ) {*ascii = '#';  ascii++; *ascii =  '1'; ascii++; *ascii = ('0'+a2)-(uint8_t)(STD_BASE_10[1]);} else

      { sprintf(aa,"\\u%i?",((a1 & 0x7F) << 8) | a2);
        //printf("§%s§\n",aa);

        int16_t j = 0;
        while(aa[j] != 0) {
          *ascii = aa[j++];
          ascii++;
        }
        ascii--;
      }


      if(bb[0] != 0) {
        if(supsub == +1) {
          strcpy(aa,"\\super ");
        } else if(supsub == -1) {
          strcpy(aa,"\\sub ");
        }
        int16_t j = 0;
        while(aa[j] != 0) {
          *ascii = aa[j++];
          ascii++;
        }


        *ascii = bb[0];
        ascii++;


        strcpy(aa,"\\nosupersub ");
        j = 0;
        while(aa[j] != 0) {
          *ascii = aa[j++];
          ascii++;
        }
        ascii--;
      }


    }
    else {
      *ascii = *str;
      str++;
    }
    ascii++;
    *ascii = 0;
  }
}



void stringToASCII(const char *str, char *ascii) {
  int16_t len;
  uint8_t a1, a2;
  char aa[32];

  len = stringGlyphLength(str);

  if(len == 0) {
    *ascii = 0;
    return;
  }

  for(int16_t i=0; i<len; i++) {  // C47 supports only unicode code points from 0x0000 to 0x7FFF
    if(*str & 0x80) {
      a1=(uint8_t)*str;
      str++;
      a2=(uint8_t)*str;
      str++;

      //replacement table
      if(_getText(a1, a2, aa)) {
        int16_t j = 0;
        while(aa[j] != 0) {
          *ascii = aa[j++];
          ascii++;
        }
        ascii--;
      }

      else
      //RANGE SUP/SUB/BASE
      if((a1==(uint8_t)(STD_SUP_0            [0]) && (a2>=(uint8_t)(STD_SUP_0            [1]) && a2<=(uint8_t)(STD_SUP_9  [1]))) ) {*ascii = ('0'+a2)-(uint8_t)(STD_SUP_0 [1]);} else
      if((a1==(uint8_t)(STD_SUP_a            [0]) && (a2>=(uint8_t)(STD_SUP_a            [1]) && a2<=(uint8_t)(STD_SUP_z  [1]))) ) {*ascii = ('a'+a2)-(uint8_t)(STD_SUP_a [1]);} else
      if((a1==(uint8_t)(STD_SUP_A            [0]) && (a2>=(uint8_t)(STD_SUP_A            [1]) && a2<=(uint8_t)(STD_SUP_Z  [1]))) ) {*ascii = ('A'+a2)-(uint8_t)(STD_SUP_A [1]);} else
      if((a1==(uint8_t)(STD_SUB_0            [0]) && (a2>=(uint8_t)(STD_SUB_0            [1]) && a2<=(uint8_t)(STD_SUB_9  [1]))) ) {*ascii = ('0'+a2)-(uint8_t)(STD_SUB_0 [1]);} else
      if((a1==(uint8_t)(STD_SUB_a            [0]) && (a2>=(uint8_t)(STD_SUB_a            [1]) && a2<=(uint8_t)(STD_SUB_z  [1]))) ) {*ascii = ('a'+a2)-(uint8_t)(STD_SUB_a [1]);} else
      if((a1==(uint8_t)(STD_SUB_A            [0]) && (a2>=(uint8_t)(STD_SUB_A            [1]) && a2<=(uint8_t)(STD_SUB_Z  [1]))) ) {*ascii = ('A'+a2)-(uint8_t)(STD_SUB_A [1]);} else
      if((a1==(uint8_t)(STD_BASE_1           [0]) && (a2>=(uint8_t)(STD_BASE_1           [1]) && a2<=(uint8_t)(STD_BASE_9 [1]))) ) {*ascii = '#';  ascii++; *ascii = ('1'+a2)-(uint8_t)(STD_BASE_1[1]);} else
      if((a1==(uint8_t)(STD_BASE_10          [0]) && (a2>=(uint8_t)(STD_BASE_10          [1]) && a2<=(uint8_t)(STD_BASE_16[1]))) ) {*ascii = '#';  ascii++; *ascii =  '1'; ascii++; *ascii = ('0'+a2)-(uint8_t)(STD_BASE_10[1]);} else
      //RANGE INTERNATIONAL AND EXTENDED ASCII
      if(a1>=0x81 && a1<=0x83) *ascii = '_'; else //All international characters use 0x00 which is not otherwise used in C47
      //RANGE QUOTES
      if(a1==(uint8_t)(STD_LEFT_SINGLE_QUOTE[0]) && (a2>=(uint8_t)(STD_LEFT_SINGLE_QUOTE[1]) && a2<=(uint8_t)(STD_SINGLE_HIGH_QUOTE[1])) ) *ascii = '\''; else
      if(a1==(uint8_t)(STD_LEFT_DOUBLE_QUOTE[0]) && (a2>=(uint8_t)(STD_LEFT_DOUBLE_QUOTE[1]) && a2<=(uint8_t)(STD_DOUBLE_HIGH_QUOTE[1])) ) *ascii = '"'; else
      {
        #ifdef PC_BUILD
          printf("Not decoded, replace with _: --a1=%u--a2=%u\n",a1,a2);
        #endif// PC_BUILD
        *ascii = 0x5F;    // underscore
      }
    }
    else {
      *ascii = *str;
      str++;
    }
    ascii++;
    *ascii = 0;
  }
}


void stringToFileNameChars(const char *str, char *ascii) {
  int16_t len;
  len = stringGlyphLength(str);

  if(len == 0) {
    *ascii = 0;
    return;
  }

  for(int16_t i=0; i<len; i++) {
    if(((uint8_t)(*str) & 0x80) != 0) {
      *ascii = '_';
      str++;
      str++;
      ascii++;
    }
    else if((uint8_t)(*str) < 0x20 || *str == '/' || *str == '\\') {
      *ascii = '_';
      str++;
      ascii++;
    }
    else if(*str == '|' || *str == '?' || *str == '*' || *str == ':' || *str == '<' || *str == '>') {
      *ascii = '-';
      str++;
      ascii++;
    }
    else if(*str == '\"') {
      *ascii = '\'';
      str++;
      ascii++;
    }
    else {
      *ascii = *str;
      str++;
       ascii++;
    }
    *ascii = 0;
  }
}



void *xcopy(void *dest, const void *source, int n) {
  char       *pDest   = (char *)dest;
  const char *pSource = (char *)source;

  if(pSource > pDest) {
    while(n--) {
      *pDest++ = *pSource++;
    }
  }
  else if(pSource < pDest) {
    while(n--) {
      pDest[n] = pSource[n];
    }
  }

  return dest;
}


void addChrBothSides(uint8_t t, char * str) {
  char tt[4];
  tt[0] = t;
  tt[1] = 0;
  xcopy(str + 1, str, stringByteLength(str) + 1);
  str[0] = t;
  strcat(str, tt);
}


void addStrBothSides(char * str, char * str_b, char * str_e) {
  xcopy(str + stringByteLength(str_b), str, stringByteLength(str) + 1);
  xcopy(str, str_b, stringByteLength(str_b));
  xcopy(str + stringByteLength(str), str_e, stringByteLength(str_e) + 1);
}


#if defined(__MINGW64__)
  char *stringCopy(char *dest, const char *source) {
    const uint32_t l = stringByteLength(source);
    return (char *)xcopy(dest, source, l + 1) + l;
  }
#endif // __MINGW64__


#if !defined(DMCP_BUILD)
void strReplace(char *haystack, const char *needle, const char *newNeedle) {
  ////////////////////////////////////////////////////////
  // There MUST be enough memory allocated to *haystack //
  // when strlen(newNeedle) > strlen(needle)            //
  ////////////////////////////////////////////////////////
  char *str, *needleLocation;
  int  needleLg;

  while(strstr(haystack, needle) != NULL) {
    needleLg = strlen(needle);
    needleLocation = strstr(haystack, needle);
    str = malloc(strlen(needleLocation + needleLg) + 1);
    #if defined(PC_BUILD) && !defined(GENERATE_CATALOGS)
      if(str == NULL) {
        moreInfoOnError("In function strReplace:", "error allocating memory for str!", NULL, NULL);
        exit(1);
      }
    #endif // PC_BUILD && !GENERATE_CATALOGS

    strcpy(str, needleLocation + needleLg);
    *strstr(haystack, needle) = 0;
    strcat(haystack, newNeedle);
    strcat(haystack, str);
    free(str);
  }
}
#endif // !DMCP_BUILD


bool_t findTwoChars(const char *tmpString, uint8_t char1, uint8_t char2, uint16_t *position) {
  for(uint16_t i = 0; tmpString[i] != '\0' && tmpString[i + 1] != '\0'; i++) {
    if((uint8_t)tmpString[i] == char1 && (uint8_t)tmpString[i + 1] == char2) {
      *position = i;
      return true;
    }
  }
  return false;
}
