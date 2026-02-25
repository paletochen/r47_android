// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file fonts.c
 ***********************************************/

#include "c47.h"

// Little hexadecimal font for generating a not found glyph
TO_QSPI const char hexaFont[] = "\x69\x99\x99\x60"  // 0
                               "\x22\x22\x22\x20"  // 1
                               "\xe1\x16\x88\xf0"  // 2
                               "\xe1\x16\x11\xe0"  // 3
                               "\x99\x9f\x11\x10"  // 4
                               "\xf8\x8e\x11\xe0"  // 5
                               "\x68\x8e\x99\x60"  // 6
                               "\xf1\x11\x11\x10"  // 7
                               "\x69\x96\x99\x60"  // 8
                               "\x69\x97\x11\x60"  // 9
                               "\x69\x9f\x99\x90"  // A
                               "\x88\x8e\x99\xe0"  // b
                               "\x78\x88\x88\x70"  // C
                               "\x11\x17\x99\x70"  // d
                               "\xf8\x8e\x88\xf0"  // E
                               "\xf8\x8e\x88\x80"; // F



/********************************************//**
 * \brief Finds a glyph in a font
 *
 * \param[in] font const font_t* Font
 * \param[in] charCode uint16_t  Unicode code point
 * \return int16_t
 *                 * Glyph index
 *                 * -1 when not found in the standard font
 *                 * -2 when not found in the numeric font
 ***********************************************/
int16_t findGlyph(const font_t *font, uint16_t charCode) {
  int16_t first, middle, last;

  first = 0;
  last = font->numberOfGlyphs - 1;

  middle = (first + last) / 2;
  while(last > first + 1) {
    if(charCode < font->glyphs[middle].charCode) {
      last = middle;
    }
    else {
      first = middle;
    }
    middle = (first + last) / 2;
  }

  if(font->glyphs[first].charCode == charCode) {
    return first;
  }

  if(font->glyphs[last].charCode == charCode) {
    return last;
  }

  if(font->id == 1) {
    return -1;
  }

  if(font->id == 0) {
    return -2;
  }
  return 0;
}



/********************************************//**
 * \brief Generates a glyph for a non found glyph containing the hexadecimal Unicoide code point
 *
 * \param[in] font int16_t      Font for which generate the glyph
 * \param[in] charCode uint16_t Unicode code point
 * \return void
 ***********************************************/
void generateNotFoundGlyph(int16_t font, uint16_t charCode) {
  int16_t i;
  uint8_t  nibble1, nibble2;


  if(charCode >= 0x8000) {
    charCode -= 0x8000;
  }

  glyphNotFound.rowsAboveGlyph =  (font==-2 ? 6 : 0); // -1=standardFont
  glyphNotFound.rowsBelowGlyph =  (font==-2 ? 7 : 1); // -2=numericFont

  // Clear the inside of the special glyph
  for(i=4; i<=32; i+=2) {
    if(i == 18) {
      i += 2;
    }

    glyphNotFound.data[i]   &= 0xc2;
    glyphNotFound.data[i+1] &= 0x18;
  }

  // Fill the inside with the hexadecimal value of charCode
  for(i=4; i<=32; i+=2) {
    if(i == 18) {
      i += 2;
    }

    if(i < 18) {
      nibble1 = (i>>1) - 2;
      if(nibble1%2 == 0) {
        nibble2 = (hexaFont[((charCode & 0x0f00) >>  6) + (nibble1>>1)] & 0xf0) >> 4;
        nibble1 = (hexaFont[((charCode & 0xf000) >> 10) + (nibble1>>1)] & 0xf0) >> 4;
      }
      else {
        nibble2 = hexaFont[((charCode & 0x0f00) >>  6) + (nibble1>>1)] & 0x0f;
        nibble1 = hexaFont[((charCode & 0xf000) >> 10) + (nibble1>>1)] & 0x0f;
      }
    }
    else {
      nibble1 = (i>>1) - 10;
      if(nibble1%2 == 0) {
        nibble2 = (hexaFont[((charCode & 0x000f) << 2) + (nibble1>>1)] & 0xf0) >> 4;
        nibble1 = (hexaFont[((charCode & 0x00f0) >> 2) + (nibble1>>1)] & 0xf0) >> 4;
      }
      else {
        nibble2 = hexaFont[((charCode & 0x000f) << 2) + (nibble1>>1)] & 0x0f;
        nibble1 = hexaFont[((charCode & 0x00f0) >> 2) + (nibble1>>1)] & 0x0f;
      }
    }

    glyphNotFound.data[i]   |= nibble1<<2 | nibble2>>3;
    glyphNotFound.data[i+1] |= nibble2<<5;
  }
}
