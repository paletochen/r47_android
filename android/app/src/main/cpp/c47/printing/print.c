// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"


#if defined(IR_PRINTING)

  #define RETURN_IF_PRINT_OFF do { if(!getSystemFlag(FLAG_PRTACT)) { return; } } while(0)
  #define BREAK_IF_EXIT       do { if(key_pop() == KEY_EXIT)       { break;  } } while(0)

  //
  // Alias table for instruction names to get nice prints
  //
  TO_QSPI const nameAlias_t NamesAlias[] = {
  //             item             name
  /*   36 */  { ITM_XexY,        "x<>y"                               },
  /*   58 */  { ITM_SQUARE,      "x^2"                                },
  /*   59 */  { ITM_CUBE,        "x^3"                                },
  /*   60 */  { ITM_YX,          "y^x"                                },
  /*   61 */  { ITM_SQUAREROOTX, STD_SQUARE_ROOT "x"                  },
  /*   62 */  { ITM_CUBEROOT,    STD_SUP_3 STD_SQUARE_ROOT "x"        },
  /*   63 */  { ITM_XTHROOT,     STD_SUP_x STD_SQUARE_ROOT "y"        },
  /*   64 */  { ITM_2X,          "2^x"                                },
  /*   65 */  { ITM_EXP,         "e^x"                                },
  /*   67 */  { ITM_10x,         "10^x"                               },
  /*  127 */  { ITM_Xex,         "x<>"                                },
  /*  981 */  { ITM_ex,          "<>"                                 },
  /* 1539 */  { ITM_M_RR,        "M.R<>R"                             },
  /* 1570 */  { ITM_REexIM,      "Re<>Im"                             },
  /* 1575 */  { ITM_EX1,         "e^x-1"                              },
  /* 1625 */  { ITM_Tex,         "t<>"                                },
  /* 1650 */  { ITM_Yex,         "y<>"                                },
  /* 1651 */  { ITM_Zex,         "z<>"                                },
  /* 1679 */  { ITM_M1X,         "(-1)^x"                             },
  /* 1694 */  { ITM_SHUFFLE,     "<>"                                 },
  /* 1725 */  { ITM_RS,          "RUN"                                },
  /* 1794 */  { ITM_SQRT1PX2,    STD_SQUARE_ROOT "(1+x^2)"            },
  /* 1816 */  { ITM_EE_EXP_TH,   "e^ix"                               },

  /*      */  { LAST_ITEM,       ""                                   }
  };

  //
  // HP-82240 Roman-8 to Unicode table
  //
  TO_QSPI const uint16_t hp82240CharMap[256] = {
  /* 0x00, */ 0x0000, // #NULL
  /* 0x01, */ 0x0000, // #UNDEFINED
  /* 0x02, */ 0x0000, // #UNDEFINED
  /* 0x03, */ 0x0000, // #UNDEFINED
  /* 0x04, */ 0x0004, // #PRINT LINE AND LEAVE PRINT HEAD AT RIGHT
  /* 0x05, */ 0x0000, // #UNDEFINED
  /* 0x06, */ 0x0000, // #UNDEFINED
  /* 0x07, */ 0x0000, // #UNDEFINED
  /* 0x08, */ 0x0000, // #UNDEFINED
  /* 0x09, */ 0x0000, // #UNDEFINED
  /* 0x0A, */ 0x000A, // #LINE FEED
  /* 0x0B, */ 0x0000, // #UNDEFINED
  /* 0x0C, */ 0x0000, // #UNDEFINED
  /* 0x0D, */ 0x0000, // #UNDEFINED
  /* 0x0E, */ 0x0000, // #UNDEFINED
  /* 0x0F, */ 0x0000, // #UNDEFINED

  /* 0x10, */ 0x0000, // #UNDEFINED
  /* 0x11, */ 0x0000, // #UNDEFINED
  /* 0x12, */ 0x0000, // #UNDEFINED
  /* 0x13, */ 0x0000, // #UNDEFINED
  /* 0x14, */ 0x0000, // #UNDEFINED
  /* 0x15, */ 0x0000, // #UNDEFINED
  /* 0x16, */ 0x0000, // #UNDEFINED
  /* 0x17, */ 0x0000, // #UNDEFINED
  /* 0x18, */ 0x0000, // #UNDEFINED
  /* 0x19, */ 0x0000, // #UNDEFINED
  /* 0x1A, */ 0x0000, // #UNDEFINED
  /* 0x1B, */ 0x001B, // #ESCAPE
  /* 0x1C, */ 0x0000, // #UNDEFINED
  /* 0x1D, */ 0x0000, // #UNDEFINED
  /* 0x1E, */ 0x0000, // #UNDEFINED
  /* 0x1F, */ 0x0000, // #UNDEFINED

  /* 0x20, */ 0x0020, // #SPACE
  /* 0x21, */ 0x0021, // #EXCLAMATION MARK
  /* 0x22, */ 0x0022, // #QUOTATION MARK
  /* 0x23, */ 0x0023, // #NUMBER SIGN
  /* 0x24, */ 0x0024, // #DOLLAR SIGN
  /* 0x25, */ 0x0025, // #PERCENT SIGN
  /* 0x26, */ 0x0026, // #AMPERSAND
  /* 0x27, */ 0x0027, // #APOSTROPHE
  /* 0x28, */ 0x0028, // #LEFT PARENTHESIS
  /* 0x29, */ 0x0029, // #RIGHT PARENTHESIS
  /* 0x2A, */ 0x002A, // #ASTERISK
  /* 0x2B, */ 0x002B, // #PLUS SIGN
  /* 0x2C, */ 0x002C, // #COMMA
  /* 0x2D, */ 0x002D, // #HYPHEN-MINUS
  /* 0x2E, */ 0x002E, // #FULL STOP
  /* 0x2F, */ 0x002F, // #SOLIDUS

  /* 0x30, */ 0x0030, // #DIGIT ZERO
  /* 0x31, */ 0x0031, // #DIGIT ONE
  /* 0x32, */ 0x0032, // #DIGIT TWO
  /* 0x33, */ 0x0033, // #DIGIT THREE
  /* 0x34, */ 0x0034, // #DIGIT FOUR
  /* 0x35, */ 0x0035, // #DIGIT FIVE
  /* 0x36, */ 0x0036, // #DIGIT SIX
  /* 0x37, */ 0x0037, // #DIGIT SEVEN
  /* 0x38, */ 0x0038, // #DIGIT EIGHT
  /* 0x39, */ 0x0039, // #DIGIT NINE
  /* 0x3A, */ 0x003A, // #COLON
  /* 0x3B, */ 0x003B, // #SEMICOLON
  /* 0x3C, */ 0x003C, // #LESS-THAN SIGN
  /* 0x3D, */ 0x003D, // #EQUALS SIGN
  /* 0x3E, */ 0x003E, // #GREATER-THAN SIGN
  /* 0x3F, */ 0x003F, // #QUESTION MARK

  /* 0x40, */ 0x0040, // #COMMERCIAL AT
  /* 0x41, */ 0x0041, // #LATIN CAPITAL LETTER A
  /* 0x42, */ 0x0042, // #LATIN CAPITAL LETTER B
  /* 0x43, */ 0x0043, // #LATIN CAPITAL LETTER C
  /* 0x44, */ 0x0044, // #LATIN CAPITAL LETTER D
  /* 0x45, */ 0x0045, // #LATIN CAPITAL LETTER E
  /* 0x46, */ 0x0046, // #LATIN CAPITAL LETTER F
  /* 0x47, */ 0x0047, // #LATIN CAPITAL LETTER G
  /* 0x48, */ 0x0048, // #LATIN CAPITAL LETTER H
  /* 0x49, */ 0x0049, // #LATIN CAPITAL LETTER I
  /* 0x4A, */ 0x004A, // #LATIN CAPITAL LETTER J
  /* 0x4B, */ 0x004B, // #LATIN CAPITAL LETTER K
  /* 0x4C, */ 0x004C, // #LATIN CAPITAL LETTER L
  /* 0x4D, */ 0x004D, // #LATIN CAPITAL LETTER M
  /* 0x4E, */ 0x004E, // #LATIN CAPITAL LETTER N
  /* 0x4F, */ 0x004F, // #LATIN CAPITAL LETTER O

  /* 0x50, */ 0x0050, // #LATIN CAPITAL LETTER P
  /* 0x51, */ 0x0051, // #LATIN CAPITAL LETTER Q
  /* 0x52, */ 0x0052, // #LATIN CAPITAL LETTER R
  /* 0x53, */ 0x0053, // #LATIN CAPITAL LETTER S
  /* 0x54, */ 0x0054, // #LATIN CAPITAL LETTER T
  /* 0x55, */ 0x0055, // #LATIN CAPITAL LETTER U
  /* 0x56, */ 0x0056, // #LATIN CAPITAL LETTER V
  /* 0x57, */ 0x0057, // #LATIN CAPITAL LETTER W
  /* 0x58, */ 0x0058, // #LATIN CAPITAL LETTER X
  /* 0x59, */ 0x0059, // #LATIN CAPITAL LETTER Y
  /* 0x5A, */ 0x005A, // #LATIN CAPITAL LETTER Z
  /* 0x5B, */ 0x005B, // #LEFT SQUARE BRACKET
  /* 0x5C, */ 0x005C, // #REVERSE SOLIDUS
  /* 0x5D, */ 0x005D, // #RIGHT SQUARE BRACKET
  /* 0x5E, */ 0x005E, // #CIRCUMFLEX ACCENT
  /* 0x5F, */ 0x005F, // #LOW LINE

  /* 0x60, */ 0x2019, // #RIGHT SINGLE QUOTATION MARK                  *
  /* 0x61, */ 0x0061, // #LATIN SMALL LETTER A
  /* 0x62, */ 0x0062, // #LATIN SMALL LETTER B
  /* 0x63, */ 0x0063, // #LATIN SMALL LETTER C
  /* 0x64, */ 0x0064, // #LATIN SMALL LETTER D
  /* 0x65, */ 0x0065, // #LATIN SMALL LETTER E
  /* 0x66, */ 0x0066, // #LATIN SMALL LETTER F
  /* 0x67, */ 0x0067, // #LATIN SMALL LETTER G
  /* 0x68, */ 0x0068, // #LATIN SMALL LETTER H
  /* 0x69, */ 0x0069, // #LATIN SMALL LETTER I
  /* 0x6A, */ 0x006A, // #LATIN SMALL LETTER J
  /* 0x6B, */ 0x006B, // #LATIN SMALL LETTER K
  /* 0x6C, */ 0x006C, // #LATIN SMALL LETTER L
  /* 0x6D, */ 0x006D, // #LATIN SMALL LETTER M
  /* 0x6E, */ 0x006E, // #LATIN SMALL LETTER N
  /* 0x6F, */ 0x006F, // #LATIN SMALL LETTER O

  /* 0x70, */ 0x0070, // #LATIN SMALL LETTER P
  /* 0x71, */ 0x0071, // #LATIN SMALL LETTER Q
  /* 0x72, */ 0x0072, // #LATIN SMALL LETTER R
  /* 0x73, */ 0x0073, // #LATIN SMALL LETTER S
  /* 0x74, */ 0x0074, // #LATIN SMALL LETTER T
  /* 0x75, */ 0x0075, // #LATIN SMALL LETTER U
  /* 0x76, */ 0x0076, // #LATIN SMALL LETTER V
  /* 0x77, */ 0x0077, // #LATIN SMALL LETTER W
  /* 0x78, */ 0x0078, // #LATIN SMALL LETTER X
  /* 0x79, */ 0x0079, // #LATIN SMALL LETTER Y
  /* 0x7A, */ 0x007A, // #LATIN SMALL LETTER Z
  /* 0x7B, */ 0x007B, // #LEFT CURLY BRACKET
  /* 0x7C, */ 0x007C, // #VERTICAL LINE
  /* 0x7D, */ 0x007D, // #RIGHT CURLY BRACKET
  /* 0x7E, */ 0x007E, // #TILDE
  /* 0x7F, */ 0x0000, // semi-solid rectangle                          *

  /* 0x80, */ 0X00A0, // #NO-BREAK SPACE
  /* 0x81, */ 0x00F7, // #DIVISION SIGN
  /* 0x82, */ 0x00D7, // #MULTIPLICATION SIGN
  /* 0x83, */ 0x221A, // #SQUARE ROOT
  /* 0x84, */ 0x222B, // #INTEGRAL
  /* 0x85, */ 0x2211, // #N-ARY SUMMATION
  /* 0x86, */ 0x25B6, // #BLACK RIGHT-POINTING TRIANGLE                *
  /* 0x87, */ 0x03C0, // #GREEK SMALL LETTER PI
  /* 0x88, */ 0x2202, // #PARTIAL DIFFERENTIAL
  /* 0x89, */ 0x2264, // #LESS-THAN OR EQUAL TO
  /* 0x8A, */ 0x2265, // #GREATER-THAN OR EQUAL TO
  /* 0x8B, */ 0x2260, // #NOT EQUAL TO
  /* 0x8C, */ 0x03B1, // #GREEK SMALL LETTER ALPHA
  /* 0x8D, */ 0x2192, // #RIGHTWARDS ARROW
  /* 0x8E, */ 0x2190, // #LEFTWARDS ARROW
  /* 0x8F, */ 0x03BC, // #GREEK SMALL LETTER MU

  /* 0x90, */ 0x21B5, // #DOWNWARDS ARROW WITH CORNER LEFTWARDS
  /* 0x91, */ 0x00B0, // #DEGREE SIGN
  /* 0x92, */ 0x00AB, // #LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
  /* 0x93, */ 0x00BB, // #RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
  /* 0x94, */ 0x22A2, // #RIGHT TACK
  /* 0x95, */ 0x2081, // #SUBSCRIPT ONE
  /* 0x96, */ 0x2082, // #SUBSCRIPT TWO
  /* 0x97, */ 0x2162, // #SUPERSCRIPT TWO (REDEFINED)
  /* 0x98, */ 0x2163, // #SUPERSCRIPT THREE (REDEFINED)
  /* 0x99, */ 0x24A4, // #SUBSCRIPT LATIN SMALL LETTER I (REDEFINED)
  /* 0x9A, */ 0x24A5, // #SUBSCRIPT LATIN SMALL LETTER J (REDEFINED)
  /* 0x9B, */ 0x0000, // &two dots                                     *
  /* 0x9C, */ 0x248A, // #SUPERSCRIPT LATIN SMALL LETTER I (REDEFINED)
  /* 0x9D, */ 0x248B, // #SUPERSCRIPT LATIN SMALL LETTER J (REDEFINED)
  /* 0x9E, */ 0x248C, // #SUPERSCRIPT LATIN SMALL LETTER K (REDEFINED)
  /* 0x9F, */ 0x248F, // #SUPERSCRIPT LATIN SMALL LETTER N (REDEFINED)

  /* 0xA0, */ 0x2221, // #MEASURED ANGLE
  /* 0xA1, */ 0x00C0, // #LATIN CAPITAL LETTER A WITH GRAVE
  /* 0xA2, */ 0x00C2, // #LATIN CAPITAL LETTER A WITH CIRCUMFLEX
  /* 0xA3, */ 0x00C8, // #LATIN CAPITAL LETTER E WITH GRAVE
  /* 0xA4, */ 0x00CA, // #LATIN CAPITAL LETTER E WITH CIRCUMFLEX
  /* 0xA5, */ 0x00CB, // #LATIN CAPITAL LETTER E WITH DIAERESIS
  /* 0xA6, */ 0x00CE, // #LATIN CAPITAL LETTER I WITH CIRCUMFLEX
  /* 0xA7, */ 0x00CF, // #LATIN CAPITAL LETTER I WITH DIAERESIS
  /* 0xA8, */ 0x00B4, // #ACUTE ACCENT                                 *
  /* 0xA9, */ 0x0060, // #GRAVE ACCENT                                 *
  /* 0xAA, */ 0x005E, // #CIRCUMFLEX ACCENT                            *
  /* 0xAB, */ 0x00A8, // #DIAERESIS                                    *
  /* 0xAC, */ 0x02DC, // #SMALL TILDE                                  *
  /* 0xAD, */ 0x00D9, // #LATIN CAPITAL LETTER U WITH GRAVE
  /* 0xAE, */ 0x00FB, // #LATIN SMALL LETTER U WITH CIRCUMFLEX
  /* 0xAF, */ 0X20A4, // #LIRA SIGN                                    *

  /* 0xB0, */ 0x00AF, // #MACRON                                       *
  /* 0xB1, */ 0x00DD, // #LATIN CAPITAL LETTER Y WITH ACUTE            *
  /* 0xB2, */ 0x00FD, // #LATIN SMALL LETTER Y WITH ACUTE              *
  /* 0xB3, */ 0x00B0, // #DEGREE SIGN
  /* 0xB4, */ 0x00C7, // #LATIN CAPITAL LETTER C WITH CEDILLA
  /* 0xB5, */ 0x00E7, // #LATIN SMALL LETTER C WITH CEDILLA
  /* 0xB6, */ 0x00D1, // #LATIN CAPITAL LETTER N WITH TILDE
  /* 0xB7, */ 0x00F1, // #LATIN SMALL LETTER N WITH TILDE
  /* 0xB8, */ 0x00A1, // #INVERTED EXCLAMATION MARK                    *
  /* 0xB9, */ 0x00BF, // #INVERTED QUESTION MARK
  /* 0xBA, */ 0x00A4, // #CURRENCY SIGN                                *
  /* 0xBB, */ 0x00A3, // #POUND SIGN
  /* 0xBC, */ 0x00A5, // #YEN SIGN
  /* 0xBD, */ 0x00A7, // #SECTION SIGN
  /* 0xBE, */ 0x0192, // #LATIN SMALL LETTER F WITH HOOK               *
  /* 0xBF, */ 0x00A2, // #CENT SIGN

  /* 0xC0, */ 0x00E2, // #LATIN SMALL LETTER A WITH CIRCUMFLEX
  /* 0xC1, */ 0x00EA, // #LATIN SMALL LETTER E WITH CIRCUMFLEX
  /* 0xC2, */ 0x00F4, // #LATIN SMALL LETTER O WITH CIRCUMFLEX
  /* 0xC3, */ 0x00FB, // #LATIN SMALL LETTER U WITH CIRCUMFLEX
  /* 0xC4, */ 0x00E1, // #LATIN SMALL LETTER A WITH ACUTE
  /* 0xC5, */ 0x00E9, // #LATIN SMALL LETTER E WITH ACUTE
  /* 0xC6, */ 0x00F3, // #LATIN SMALL LETTER O WITH ACUTE
  /* 0xC7, */ 0x00FA, // #LATIN SMALL LETTER U WITH ACUTE
  /* 0xC8, */ 0x00E0, // #LATIN SMALL LETTER A WITH GRAVE
  /* 0xC9, */ 0x00E8, // #LATIN SMALL LETTER E WITH GRAVE
  /* 0xCA, */ 0x00F2, // #LATIN SMALL LETTER O WITH GRAVE
  /* 0xCB, */ 0x00F9, // #LATIN SMALL LETTER U WITH GRAVE
  /* 0xCC, */ 0x00E4, // #LATIN SMALL LETTER A WITH DIAERESIS
  /* 0xCD, */ 0x00EB, // #LATIN SMALL LETTER E WITH DIAERESIS
  /* 0xCE, */ 0x00F6, // #LATIN SMALL LETTER O WITH DIAERESIS
  /* 0xCF, */ 0x00FC, // #LATIN SMALL LETTER U WITH DIAERESIS

  /* 0xD0, */ 0x00C5, // #LATIN CAPITAL LETTER A WITH RING ABOVE
  /* 0xD1, */ 0x00EE, // #LATIN SMALL LETTER I WITH CIRCUMFLEX
  /* 0xD2, */ 0x00D8, // #LATIN CAPITAL LETTER O WITH STROKE
  /* 0xD3, */ 0x00C6, // #LATIN CAPITAL LETTER AE
  /* 0xD4, */ 0x00E5, // #LATIN SMALL LETTER A WITH RING ABOVE
  /* 0xD5, */ 0x00ED, // #LATIN SMALL LETTER I WITH ACUTE
  /* 0xD6, */ 0x00F8, // #LATIN SMALL LETTER O WITH STROKE
  /* 0xD7, */ 0x00E6, // #LATIN SMALL LETTER AE
  /* 0xD8, */ 0x00C4, // #LATIN CAPITAL LETTER A WITH DIAERESIS
  /* 0xD9, */ 0x00EC, // #LATIN SMALL LETTER I WITH GRAVE
  /* 0xDA, */ 0x00D6, // #LATIN CAPITAL LETTER O WITH DIAERESIS
  /* 0xDB, */ 0x00DC, // #LATIN CAPITAL LETTER U WITH DIAERESIS
  /* 0xDC, */ 0x00C9, // #LATIN CAPITAL LETTER E WITH ACUTE
  /* 0xDD, */ 0x00EF, // #LATIN SMALL LETTER I WITH DIAERESIS
  /* 0xDE, */ 0x00DF, // #LATIN SMALL LETTER SHARP S (GERMAN)          *
  /* 0xDF, */ 0x00D4, // #LATIN CAPITAL LETTER O WITH CIRCUMFLEX

  /* 0xE0, */ 0x00C1, // #LATIN CAPITAL LETTER A WITH ACUTE
  /* 0xE1, */ 0x00C3, // #LATIN CAPITAL LETTER A WITH TILDE
  /* 0xE2, */ 0x00E3, // #LATIN SMALL LETTER A WITH TILDE
  /* 0xE3, */ 0x00D0, // #LATIN CAPITAL LETTER ETH (ICELANDIC)         *
  /* 0xE4, */ 0x00F0, // #LATIN SMALL LETTER ETH (ICELANDIC)           *
  /* 0xE5, */ 0x00CD, // #LATIN CAPITAL LETTER I WITH ACUTE
  /* 0xE6, */ 0x00CC, // #LATIN CAPITAL LETTER I WITH GRAVE
  /* 0xE7, */ 0x00D3, // #LATIN CAPITAL LETTER O WITH ACUTE
  /* 0xE8, */ 0x00D2, // #LATIN CAPITAL LETTER O WITH GRAVE
  /* 0xE9, */ 0x00D5, // #LATIN CAPITAL LETTER O WITH TILDE
  /* 0xEA, */ 0x00F5, // #LATIN SMALL LETTER O WITH TILDE
  /* 0xEB, */ 0x0160, // #LATIN CAPITAL LETTER S WITH CARON
  /* 0xEC, */ 0x0161, // #LATIN SMALL LETTER S WITH CARON
  /* 0xED, */ 0x00DA, // #LATIN CAPITAL LETTER U WITH ACUTE
  /* 0xEE, */ 0x0178, // #LATIN CAPITAL LETTER Y WITH DIAERESIS
  /* 0xEF, */ 0x00FF, // #LATIN SMALL LETTER Y WITH DIAERESIS

  /* 0xF0, */ 0x00DE, // #LATIN CAPITAL LETTER THORN (ICELANDIC)       *
  /* 0xF1, */ 0x00FE, // #LATIN SMALL LETTER THORN (ICELANDIC)         *
  /* 0xF2, */ 0x00B7, // #MIDDLE DOT                                   *
  /* 0xF3, */ 0x00B5, // #MICRO SIGN
  /* 0xF4, */ 0x00B6, // #PILCROW SIGN                                 *
  /* 0xF5, */ 0x00BE, // #VULGAR FRACTION THREE QUARTERS               *
  /* 0xF6, */ 0x00AD, // #SOFT HYPHEN                                  *
  /* 0xF7, */ 0x00BC, // #VULGAR FRACTION ONE QUARTER
  /* 0xF8, */ 0x00BD, // #VULGAR FRACTION ONE HALF
  /* 0xF9, */ 0x00AA, // #FEMININE ORDINAL INDICATOR                   *
  /* 0xFA, */ 0x00BA, // #MASCULINE ORDINAL INDICATOR                  *
  /* 0xFB, */ 0x00AB, // #LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
  /* 0xFC, */ 0x25AE, // #BLACK VERTICAL RECTANGLE                     *
  /* 0xFD, */ 0x00BB, // #RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
  /* 0xFE, */ 0x00B1, // #PLUS-MINUS SIGN
  /* 0xFF, */ 0x0000  // #UNDEFINED
  };


  uint8_t charMap(uint16_t charCode) {
    uint8_t i;
    for(i=128; i<255; i++) {
      if(hp82240CharMap[i] == (charCode & ~0x8000)) {
        return (i);
      }
    }
    return 0;
  }


  int16_t findPrinterGlyph(const printerFont_t *font, uint16_t charCode) {
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

    return font->numberOfGlyphs - 1;  // Last character of the font is for charCodes not supported by the font
  }


  uint16_t findMartelGlyph(const martelFont24_t *font, uint16_t charCode) {
    uint16_t first, middle, last;

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

    return 255;  // charCode not supported by the font
  }


  static bool_t _exitKeyPressed() {
    #if defined(DMCP_BUILD)
      int key = C47PopKeyNoBuffer(!DISPLAY_WAIT_FOR_RELEASE) + 1;
      if(key == 36 || key == 33 ) {  // R/S or EXIT
        key_pop();
        clearKeyBuffer();
        return(true);
      }
      else {
        key_pop_all();
        clearKeyBuffer();
        return(false);
      }
    #elif defined(PC_BUILD) // !DMCP_BUILD
      //printf("KeyWaiting keyCode=%u",currentKeyCode);
      return currentKeyCode == 32; //EXIT1 / EXIT key //Do not us gtk_events_pending() as it triggers for timers too
    #endif // PC_BUILD
  }

  static void setPrinterSBI(bool_t status) {
    printerIconEnabled = status;
    setSystemFlagChanged(SETTING_PRINTERICON);
    refreshStatusBar();
  }

unsigned int charlengths(unsigned int c) {
  static const unsigned char widths[171] = {
    162, 143, 210, 208, 215, 179, 100, 209, 130, 167, 178, 199,
    215,  85, 107, 123, 160, 172, 172,  88, 178, 177, 208, 208,
    166, 208, 179, 173, 209, 215, 130, 165, 170, 172, 172, 165,
    177, 172, 172, 214, 171, 117, 131, 209, 143, 215, 131, 178,
    215, 213, 143, 201, 213, 179, 172, 214, 170, 172, 215, 209,
    215,  95, 129, 129, 172, 172, 172, 172, 129, 207, 215, 136,
    172, 208, 173, 172, 172, 172, 136, 129, 172, 172, 166, 172,
    172, 112, 135, 147, 129, 171, 130, 129, 176, 131, 141, 208,
    115, 143,  82, 140,  50,  93, 129, 129,  57, 128, 165, 129,
    129,  57, 135, 137, 171, 129, 135,  93, 123, 123, 129, 129,
     87, 195, 129, 129, 129, 131,  57, 135, 141, 172, 171, 131,
    207, 165, 202, 177,  93, 167, 178, 135, 165, 124, 129, 177,
    136, 142, 143,  86, 129, 129, 129, 129, 129,  87, 165, 129,
    129, 129, 129, 129, 129, 129, 129, 129, 123, 129, 129, 129,
    129, 129,  21
  };
  static const unsigned char divs[3] = { 1, 6, 36 };
  return (widths[c/3] / divs[c%3]) % 6 + 1;
}

void findlengths(unsigned short int posns[257], int smallp) {
  const int mask = smallp ? 256 : 0;
  int i;

  posns[0] = 0;
  for(i=0; i<256; i++) {
    posns[i+1] = posns[i] + charlengths(i + mask) - 1;
  }
}


/* Determine the pixel length of the string if it were displayed
*/
int pixel_length(const char *s, int smallp) {
  int len = 0;
  const int offset = smallp ? 256 : 0;
  while(*s != '\0') {
    #if defined(INCLUDE_FONT_ESCAPE)
    if(s[0] == '\007') {
      len += s[1] & 0x1F;
      s += 3;
      continue;
    }
    #endif // INCLUDE_FONT_ESCAPE
    len += charlengths((unsigned char)*s++ + offset);
  }
  return len;
}


void prepareNewLine(void) {
#if defined(DMCP_BUILD)
  int i = 0;
  clearSystemFlag(FLAG_PRTACT);
  //finish_PRT(); // redraws the PRT annunciator
  printer_advance_buf(PRINT_GRA_LN);
  while(printer_busy_for(PRINT_GRA_LN) && i<50) { // return after 5.0s anyway
    sys_timer_start(0, 100);  // Timer 0: wake up for heartbeat
    sys_sleep();
    sys_timer_disable(0); // stop timer
    i++;
  }
  setSystemFlag(FLAG_PRTACT);
  //finish_PRT();
#endif // !DMCP_BUILD
}


/*
 *  Print to IR
 */
void printIR(uint8_t c) { // prints a single character
  const print_modes_t mode = printerState.print_mode;

  if(c == '\n' && (mode == PMODE_GRAPHICS || mode == PMODE_SMALLGRAPHICS)) {
    // better LF for graphics printing
    sendByteIR(0x04);
  }
  else {
    sendByteIR(c);
  }
  // Reset the CL_DROP timer to avoid missing double press on <- when tracing the CLX instruction
  if(fnTimerGetStatus(TO_CL_DROP) == TMR_RUNNING) {
    fnTimerStart(TO_CL_DROP, TO_CL_DROP, JM_CLRDROP_TIMER);
  }
}


/*
 *  New line
 *  Input: nlMode - define the type of new line sent to the printer
 *                 0: \n    - standard LF
 *                 1: 0x04  - HP 82240 custom LF
 */
void printAdvance(uint8_t nlMode) {
  printerColumn = 0;
  printIR(nlMode ? 0x04 : '\n');
  if(printerState.print_blank_line) {
    printIR(nlMode ? 0x04 : '\n');
  }
  prepareNewLine();
}


/*
 *  New line if tracing is active
 */
static void advanceIfTrace(void) {
  if(printerColumn != 0 && getSystemFlag(FLAG_TRACE)) {
    printAdvance(0);
  }
}

//
//  Move to column
//
void printTab(uint16_t col) { // pixel-aligned column
  if(printerColumn > col) {
    printAdvance(0);
  }
  if(printerColumn < col) {
    uint16_t i, j;
    i = (col - printerColumn) % 7;
    if(i == 0 && printerColumn == 0 && col > 6) {
      i = 7;  // compensate for the first column being not printed
    }
    printerColumn += i;
    if(i) {
      sendByteIR(27);
      sendByteIR(i);
      while(i--) {
        sendByteIR(0);
      }
    }
    j = (col - printerColumn) / 7;
    while(j--) {
      sendByteIR(' ');
    }
    printerColumn = col;
  }
}

//
//  Print a graphic sequence
//
void printGraphic(uint8_t glen, const unsigned char *graphic) {
  if(glen > 0) {
    sendByteIR(27);
    sendByteIR(glen);
    while(glen--) {
      sendByteIR(*graphic++);
    }
  }
}

//
//  Print a 8-bit graphic sequence
//
void printGraphic8(uint8_t glen, const unsigned char *graphic) {
  uint8_t len = glen + ((printerColumn == 0) || (printerColumn >= 160) ? 1 : 2);
  if(glen > 0) {
    sendByteIR(27);    // Set bit image (82240 graphic character printing : ESC Xchar [Ci]
    sendByteIR(len);
    if(printerColumn != 0) {
      sendByteIR(0);              // start with a blank columns
    }
    while(glen--) {
      sendByteIR(*graphic++);
    }
    if(printerColumn < 160) {
      sendByteIR(0);             // end with a blank columns
    }
    printerColumn += len;
  }
}

//
//  Print a 24-bit graphic sequence
//
void printGraphic24(uint8_t glen, const unsigned char *graphic) {
  if(glen > 0) {
    printerColumn += glen;
    sendByteIR(27);    // Set bit image (24 pin double density) : ESC NULL * 33 n1 n2 [d]
    sendByteIR(0);
    sendByteIR(42);
    sendByteIR(33);
    sendByteIR(glen/3);
    sendByteIR(0);
    while(glen--) {
      sendByteIR(*graphic++);
    }
  }
}


//
//  Print a glyph (HP-82240 8-bit graphic)
//
void printGlyph8(uint16_t charCode, const printerFont_t *font) {
  int32_t  glyphId;
  uint8_t  *data;
  const glyphPrinter_t *glyph;

  glyphId = findPrinterGlyph(font, charCode);
  glyph = (font->glyphs) + glyphId;
  data = (uint8_t *)glyph->data;
  printGraphic8(5, data);   // Print the glyph columns
}

//
//  Print a glyph on a Martel printer
//
void printMartelGlyph(uint16_t charCode) {
  int32_t  glyphId;
  uint8_t  *data;
  const glyphMartelPrinter_t *glyph;
  const martelFont24_t *martelFont = &martelFont24;

  glyphId = findMartelGlyph(martelFont, charCode);
  if(glyphId == 255) {
    printGlyph8(charCode, &printerFont8);  // Character not in the Martel high res font
  }
  else {
    glyph = (martelFont->glyphs) + glyphId;
    data = (uint8_t *)glyph->data;
    printGraphic24(48, data);   // Print the glyph columns
    printerColumn += 8;
  }
}

//
//  Print a glyph (Martel 24-bit graphic)
//
void printGlyph24(uint16_t charCode, const font_t *font) {
  uint32_t col, row, row_scaled;
  uint32_t graphic_byte;
  int32_t  glyphId;
  uint8_t  byte, *data;
  const glyph_t *glyph;
  unsigned char graphic[42];  // 3 bytes per column (20 rows) x 14 columns

  memset(graphic, 0, 42);
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
  data = (uint8_t *)glyph->data;

  // Drawing the glyph on 24 rows
  for(row=glyph->rowsGlyph + glyph->rowsBelowGlyph; row>glyph->rowsBelowGlyph; row--) {
    // Drawing the columns of the glyph
    int32_t bit = 7;
    //row_scaled = (row + (row >> 1)) % 25;      // Scale 1.5
    //row_scaled = ((row >> 1) + (row >> 2));    // Scale  .75
    row_scaled = row;                          // Scale 1
    for(col=0; col<glyph->colsGlyph; col++) {
      if(bit == 7) {
        byte = *(data++);
      }
      graphic_byte = col*3 + (24-row_scaled)/8 ;
      if(byte & 0x80) { // MSB set
        graphic[graphic_byte] = graphic[graphic_byte] | (1 << ((row_scaled-1) % 8));  // Set graphic pixel
      }
      else {
        graphic[graphic_byte] = graphic[graphic_byte] & ~(1 << ((row_scaled-1) % 8));  // Clear graphic pixel
      }

      byte <<= 1;

      if(--bit == -1) {
        bit = 7;
      }
    }
  }
  printGraphic24(42, graphic);
  printerColumn += 14;
}

/*
//
//  Determine the length of a string in printer pixels based on the current mode.
//
static int buffer_width(const char *buff) {
  const int mode = printerState.print_mode;
  unsigned int c;
  int l = 0;

  while((c = 0xff & *buff++) != '\0') {
    switch(mode) {
    default:
      l += 7;
      break;

    case PMODE_SMALLGRAPHICS:
      c += 256;
    case PMODE_GRAPHICS:
      l += charlengths(c);
      break;
    }
  }
  return l;
}
*/


//
//  Wrap if line is full
//
static void wrap(int width) {
  if(printerColumn + width > PAPER_WIDTH) {
    printAdvance (0);
    if(width == 7) {
      width = 6;
    }
  }
  printerColumn += width;
}

//
//  Print a complete line using character set translation
//
void printLine(const char *buff, int with_lf) {
  const int mode = printerState.print_mode;
  uint8_t c;
  //unsigned short int posns[257];
  //unsigned char pattern[6];  // Rows
  //unsigned char i, j, m = 0;
  unsigned char graphic[PAPER_WIDTH];  // Columns
  unsigned char glen = 0;
  unsigned char w = 0;

  // Show Print SBI
  setPrinterSBI(true);

  // Print line
  while((c = *((const unsigned char *)buff++)) != '\0') {
    w= 0;
    switch(mode) {
      case PMODE_DEFAULT:      // Mixed character and graphic printing
        if(c == 006 && *buff == 6) {
         // merge small spaces
         continue;
        }
        if(c & 0x80) {  // Unicode
          uint16_t charCode = (c << 8) | *((const unsigned char *)buff++);
          c = charMap(charCode);
          if(c == 0) {  // Not in the 82240 roman character set, need to print graphic
            if(printerState.printer_model == PRINTER_HP) {
              printGlyph8(charCode, &printerFont8);
            }
            else if(printerState.printer_model == PRINTER_MARTEL) {
              //printGlyph24(charCode, &standardFont);
              printMartelGlyph(charCode);
            }
          }
          else {       // Character is in the 82240 roman character set
            w = printerColumn == 0 || printerColumn == 160 ? 6 : 7;
            wrap(w);
            sendByteIR(c);
          }
        }
        else {
          // Use printer character set
          w = printerColumn == 0 || printerColumn == 160 ? 6 : 7;
          printGraphic(glen, graphic);
          glen = 0;
          wrap(w);
          sendByteIR(c);
        }
        break;
    }
  }
  printGraphic(glen, graphic);
  if(with_lf) {
    printAdvance(0);
  }

  // Hide Print SBI
  setPrinterSBI(false);
}

//
//  Print buffer right justified
//
void printJustified(const char *buff) {
  print_modes_t pmode = printerState.print_mode;
  uint16_t len = pmode == PMODE_DEFAULT ? stringGlyphLength(buff) * 7 - 1
                                        : pixel_length(buff, pmode == PMODE_SMALLGRAPHICS);
  uint16_t paperWidth = PAPER_WIDTH;

  if(len >= paperWidth - printerColumn) {
    len = paperWidth - printerColumn;
  }
  if(len > 0) {
    printTab(paperWidth - len);
  }
  printLine(buff, 1);
}

//
//  Print buffer justified on the left half of the paper line
//
void printJustifiedLeft(const char *buff) {
  print_modes_t pmode = printerState.print_mode;
  uint16_t len = pmode == PMODE_DEFAULT ? stringGlyphLength(buff) * 7 - 1
                                        : pixel_length(buff, pmode == PMODE_SMALLGRAPHICS);
  uint16_t paperWidth = (PAPER_WIDTH / 2) - 7;

  if(len >= paperWidth - printerColumn) {
    len = paperWidth - printerColumn;
  }
  if(len > 0) {
    printTab(paperWidth - len);
  }
  printLine(buff, 0);
}

//
// Fit a real string in a 16-character printer compatible string
//

static void _realStringToPrint(char *realString, int16_t max_len) {
  uint16_t i, j, k, pos, expLen;
  uint16_t len = strlen(realString);
  char *from, *to;

  k = 0;
  if(stringGlyphLength(realString) > max_len){        // if string too long for printer
    for(i = 0; i < len; i++) {
      if((realString[i] == 'e') || (realString[i] == 'E')) {
        expLen = len - i;
        from   = &realString[i];
        pos    = 1;
        for(j = 0; j < max_len - expLen - 1; j++) {             // Find the location to move the exponent to fit within max_len characters
          pos = stringNextGlyph(realString, pos);
        }
        to = &realString[pos];
        memmove(to, from, expLen+1);                     // Move exponent within the first max_len characters
        break;
      }
      else if(realString[i] & 0x80) { // Two-byte unicode character
        i++;
      }
      k++;
      if(k >= max_len) {
        realString[i+1] = 0;
        break;
      }
    }
  }
}

//
// Fit a real in a string for printing
//

static void _real34ToPrintString(real34_t *real34, uint16_t amMode, char *realString, int16_t maxWidth) {
  uint8_t grpGroupingLeftOld  = grpGroupingLeft;
  uint8_t grpGroupingRightOld = grpGroupingRight;

  grpGroupingLeft  = 0;
  grpGroupingRight = 0;
  real34ToDisplayString(real34, amMode, realString, &standardFont, maxWidth, 16, true, false, true);
  grpGroupingRight = grpGroupingRightOld;
  grpGroupingLeft  = grpGroupingLeftOld;
  uint16_t i, j;
  uint16_t len = strlen(realString);

  j = 0;
  for(i = 0; i < len; i++) {
    switch((uint8_t) realString[i]) {
      case 0x80:
        if(realString[i+1] == STD_CROSS[1]) {  // Exponent found
          realString[j++] = 'E';    // replace by E
          i += 3; // Skip x10
          if(realString[i+1] == '+') {
            i++;  // Skip the + sign in the exponent
          }
        }
        else if(realString[i+1] == STD_DEGREE[1]) {  // Degree found
          realString[j++] = 0X80;          // #DEGREE SIGN
          realString[j++] = 0XB0;          // #DEGREE SIGN
          i++;
        }
        else {
          i++;
        }
        break;

      case 0xa0:
        if(realString[i+1] <= 0x0f) {      // Special space codes
          i++;                            // ignore them
        }
        else {  // Just pass all other unicode characters starting with 0xa0
          realString[j++] = realString[i];
          realString[j++] = realString[i+1];
          i++;
        }
        break;

      case 0xa1:
        if(realString[i+1] == STD_SUP_MINUS[1]) {  // Exponent sign found
          realString[j] = '-';    // replace by normal minus
          j++;
          i++;
        }
        else if((realString[i+1] >= STD_SUP_0[1]) && (realString[i+1] <= STD_SUP_9[1])) {
          realString[j] = realString[i+1]- 0x30;  // replace superscript digits by normal digits
          j++;
          i++;
        }
        else {
          i++;
        }
        break;

      case 0xac:
        if((realString[i+1] == STD_SUP_pir[1]) && (printerState.printer_model == PRINTER_HP))  {
          realString[j++] = STD_SUP_pi[0];       // replace sup pi r by sup pi
          realString[j++] = STD_SUP_pi[1];
          i++;
        }
        else {
          realString[j++] = realString[i];
          realString[j++] = realString[i+1];
          i++;
        }
        break;

      case 0x81:
      case 0x82:
      case 0x83:
      case 0x9d:
      case 0x9e:
      case 0xa2:
      case 0xa3:
      case 0xa4:
      case 0xa5:
      case 0xa6:
      case 0xa7:
      case 0xa9:
      case 0xab:
        realString[j++] = realString[i];
        realString[j++] = realString[i+1];
        i++;     // Just pass all other unicode characters
        break;

      default:
        realString[j] = realString[i];
        j++;
        break;
    }
  }
  realString[j] = 0;
}

//
// Fit a complex in a string for printing
//

static void _complex34ToPrintString(real34_t *registReal34, real34_t *registImag34, uint16_t tagAngle, uint16_t tagPolar, char *realString) {
  int16_t max_len = 17;
  real_t real, imagIc;
  real34_t real34, imag34;

  if(tagPolar) {  // polar mode
    temporaryFlagPolar = true;
    real34ToReal(registReal34, &real);
    real34ToReal(registImag34, &imagIc);

    decContext c = ctxtReal39;
    int maxExponent = max(real.exponent + real.digits, imagIc.exponent + imagIc.digits);
    c.digits = (SHOWMODE ? 39 : max(0, maxExponent) + NUMBER_OF_DISPLAY_REAL_CONTEXT_DIGITS + 2); //add 2 guard digits for Taylor etc.
    realRectangularToPolar(&real, &imagIc, &real, &imagIc, &c); // imagIc in radian
    c.digits = (SHOWMODE ? 39 : 3 + NUMBER_OF_DISPLAY_REAL_CONTEXT_DIGITS); //converting from radians to grad is the worst, i.e. x 2E2 / pi, which requires 3 digits accuarcy more
    convertAngleFromTo(&imagIc, amRadian, tagAngle, &c);

    realToReal34(&real, &real34);
    realToReal34(&imagIc, &imag34);
  }
  else { // rectangular mode
    real34Copy(registReal34, &real34);
    real34Copy(registImag34, &imag34);
  }

  // Real part
  _real34ToPrintString(&real34, amNone, tmpString, max_len * 9);
  _realStringToPrint(tmpString, max_len);                 // Adjust the real part lenght (same as a standard real)

  // Separator
  if(tagPolar) {
    strcat(tmpString, STD_MEASURED_ANGLE);
    tagAngle = (tagAngle == amNone ? currentAngularMode : tagAngle);
  }
  else {
    strcat(tmpString, "+i");
    strcat(tmpString, PRODUCT_SIGN);
    tagAngle = amNone;
  }

  // Imaginary part
  char * imaginaryPart = tmpString + strlen(tmpString);
  _real34ToPrintString(&imag34, tagAngle, imaginaryPart, max_len * 9);
  _realStringToPrint(imaginaryPart, max_len + (tagPolar ? 1 : 0));             // Adjust the imaginary part length
}

//
//  Print a single register
//
void printReg(uint16_t regist, const char *label, bool_t eq, print_area_t where, bool prSigma) {
  uint16_t tagAngle;
  uint16_t tagPolar;
  real34_t *real34;
  real34_t *imag34;
  int16_t max_len   = ((where == LINE_FULL) || (where == LINE_NOLF) ?  17 : 11);
  //int16_t max_width = ((where == LINE_FULL) || (where == LINE_NOLF) ? 153 : 99);
  if(prSigma) {
    max_len   -= 1;
    //max_width -= 9;
  }

  if(label != NULL) {
    printLine(label, 0);
    if(eq) {
      printLine("=", 0);
    }
  }

//  dtLongInteger     =  0,  ///< Z arbitrary precision integer                    DONE
//  dtReal34          =  1,  ///< R double precision real (128 bits)               DONE (fractions are printed as reals, not as fractions)
//  dtComplex34       =  2,  ///< C double precision complex (2x 128 bits)         DONE
//  dtTime            =  3,  ///< Time                                             DONE
//  dtDate            =  4,  ///< Date in various formats                          DONE
//  dtString          =  5,  ///< Alphanumeric string                              Need to complete the printerFont8
//  dtReal34Matrix    =  6,  ///< Double precision vector or matrix                DONE
//  dtComplex34Matrix =  7,  ///< Double precision complex vector or matrix        DONE
//  dtShortInteger    =  8,  ///< Short integer (64 bit)                           DONE
//  dtConfig          =  9,  ///< Configuration                                    DONE, print "Config. data"

  switch(getRegisterDataType((calcRegister_t) regist)) {
    case dtString:
      strcpy(tmpString, REGISTER_STRING_DATA(regist));
      if(where == LINE_FULL) {
        addChrBothSides(34, tmpString);   //Add quotes only for standard print reg, not for print XY
      }
      break;

    case dtReal34:
      _real34ToPrintString(REGISTER_REAL34_DATA(regist), getRegisterAngularMode(regist), tmpString, max_len * 9);
      _realStringToPrint(tmpString, max_len);             // Fit the number on a single line
      break;

    case dtComplex34:
      tagAngle = getComplexRegisterAngularMode(regist);
      tagPolar = getComplexRegisterPolarMode(regist);
      real34 = REGISTER_REAL34_DATA(regist);
      imag34 = REGISTER_IMAG34_DATA(regist);

      _complex34ToPrintString(real34, imag34, tagAngle, tagPolar, tmpString);
      break;

    case dtReal34Matrix: {
      matrixHeader_t *matrixHeader = REGISTER_MATRIX_HEADER(regist);
      real34_t *real34 = REGISTER_REAL34_MATRIX_ELEMENTS(regist);
      real34_t reduced;
      uint16_t rows, columns;
      uint16_t i, j;

      rows = matrixHeader->matrixRows;
      columns = matrixHeader->matrixColumns;
      sprintf(tmpString, "[ %" PRIu16 "x%" PRIu16 " Matrix ]", rows, columns);  //Matrix header
      printJustified(tmpString);
      for(i = 1; i <= rows; i++) {
        for(j = 1; j <= columns; j++) {
          sprintf(tmpString, "%" PRIu16 ":%" PRIu16 "=", i, j);  //Matrix element
          printLine(tmpString, 0);
          real34Reduce(real34++, &reduced);
          _real34ToPrintString(&reduced, amNone, tmpString, max_len * 9);
          _realStringToPrint(tmpString, max_len);             // Fit the number on a single line
          printJustified(tmpString);
        }
      }
      return;
    }

    case dtComplex34Matrix: {
      complex34Matrix_t cpxMatrix;
      complex34Matrix_t *matrix = &cpxMatrix;
      real34_t *real34, *imag34;
      uint16_t rows, cols;
      uint16_t i, j;
      uint16_t tagAngle = getComplexRegisterAngularMode(regist);
      uint16_t tagPolar = getComplexRegisterPolarMode(regist);

      linkToComplexMatrixRegister(regist, matrix);
      rows = matrix->header.matrixRows;
      cols = matrix->header.matrixColumns;
      sprintf(tmpString, "[ %" PRIu16 "x%" PRIu16 " Cpx Matrix ]", rows, cols);  //Matrix header
      printJustified(tmpString);
      for(i = 0; i < rows; i++) {
        for(j = 0; j < cols; j++) {
          sprintf(tmpString, "%" PRIu16 ":%" PRIu16 "=", i+1, j+1);  //Matrix element
          printLine(tmpString, 0);
          real34 = VARIABLE_REAL34_DATA(&matrix->matrixElements[i*cols+j]);
          imag34 = VARIABLE_IMAG34_DATA(&matrix->matrixElements[i*cols+j]);
          _complex34ToPrintString(real34, imag34, tagAngle, tagPolar, tmpString);
          printJustified(tmpString);
        }
      }
      return;
    }

    case dtShortInteger: {
      int16_t base, n;
      uint64_t shortInt;
      shortInt = *(REGISTER_SHORT_INTEGER_DATA(regist)) & shortIntegerMask;
      base = getRegisterShortIntegerBase(regist);
      n = ERROR_MESSAGE_LENGTH - 100;
      sprintf(errorMessage + n--, "#%02d", base);
      if(shortInt == 0) {
        errorMessage[n--] = '0';
      }
      else {
        while(shortInt != 0) {
          errorMessage[n--] = baseDigits[shortInt % base];
          shortInt /= base;
        }
      }
      n++;
      strcpy(tmpString, errorMessage + n);
      break;
    }

    default:
      copyRegisterToClipboardString(regist, tmpString, true);
      break;
  }

  if(label == NULL && (where == LINE_FULL || (where == LINE_NOLF))) {     // Padding for PRX
    uint16_t i, padding;
    uint16_t glen = stringGlyphLength(tmpString);
    if((where == LINE_NOLF) && (glen < 17)) {
      padding = 17 - glen;
      printTab(padding * 7 - 1);
    }

    if(glen > 17) {
      glen = glen % 24;
      padding = (glen <= 17 ? 17 - glen : 24 - (glen - 17));
      for(i=0; i < padding; i++) {
        strcat(tmpString, " ");  // pad string to ensure "***" will be right aligned
      }
    }

    if(where == LINE_FULL) {
      strcpy(tmpString + strlen(tmpString), "    ***");   // End line with "    ***" as on the HP-41 and 42
    }
  }

  switch(where) {
    case LINE_FULL:
    case LINE_RIGHT:
      printJustified(tmpString);
      break;
    case LINE_LEFT:
      printJustifiedLeft(tmpString);
      break;
    case LINE_NOLF:
    case LINE_ASIS:
      printLine(tmpString, 0);
      break;
    default:
      printJustified(tmpString);
  }
}


//
//  Print the contents of an Alpha register, terminated by a LF
//
void printAlpha(const char *Alpha, printArgument_t arg) {
  RETURN_IF_PRINT_OFF;
  advanceIfTrace();
  if(arg == PRINT_ALPHA_JUST) {
    printJustified(Alpha);
  }
  else {
    printLine(Alpha, arg == PRINT_ALPHA);
  }
}

//
//  Send a LF to the printer
//
void print_lf(void) {
  RETURN_IF_PRINT_OFF;
  advanceIfTrace();
  printAdvance(0);
}


//
//  Print a single character or control code
//
void cmdPrint(uint16_t arg, printArgument_t op) {
  char buff[4];
  char *line;

  if(!getSystemFlag(FLAG_PRTACT)) {
    if(getSystemFlag(FLAG_PRTEN) || ((programRunStop != PGM_RUNNING) && (programRunStop != PGM_SINGLE_STEP))) {
      displayCalcErrorMessage(ERROR_PRINTING_DISABLED, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
      #if defined(PC_BUILD)
        sprintf(errorMessage, "Printing is disabled");
        moreInfoOnError("In function cmdPrint:", errorMessage, NULL, NULL);
      #endif // PC_BUILD
    }
    return;
  }

  switch(op) {
    case PRINT_BYTE:
      // Transparent printing of bytes
      printIR(arg); // might be a line feed, so...
      if(arg == '\n' || arg == 0x04) {
        prepareNewLine();
      }
    break;

    case PRINT_CHAR:
      // Character printing, should depend on mode
      line = buff;
      if(arg & 0xff00) {
        *line++ = (arg | 0x8000) >> 8;
      }
      *line++ = arg & 0xff;
      *line = '\0';
      printLine(buff, 0);
      break;

    case PRINT_TAB:
      // Move to specific column
      printTab(arg);
      break;

    default:
      break;
  }
}

//
//  Set print mode
//
void setPrintMode(uint8_t mode) {
  // Set print mode: ESC NULL  !  n
  //                 27   0   33  n
  sendByteIR(27);
  sendByteIR( 0);
  sendByteIR(33);
  sendByteIR(mode);
}

uint8_t reverse(uint8_t b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

//
//  Print LCD screen
//
void printLcd(void) {
  int32_t x, y;
  uint8_t * line_addr = 0;
  uint8_t data;
  int16_t i;
  int32_t offset;

  //setPrintMode(0x01);  // 48 chars/line
  sendByteIR('\n');
  //for(x=0; x<SCREEN_WIDTH/8; x++) {
  for(x=16; x>=0; x--) {
    //New graphic line
    printerColumn = 0;
    printTab(30);  // Center screen shot
    sendByteIR( 27);    // Set bit image (24 pin double density) : ESC NULL * 33 n1 n2 [d]
    sendByteIR(  0);
    sendByteIR( 42);
    sendByteIR( 33);
    sendByteIR(242);
    sendByteIR(  0);
    // print bottom line
    sendByteIR((x == 16 ? 0x1f : 0xff));
    sendByteIR(0xff);
    sendByteIR((x == 0 ? 0xf1 : 0xff));
    // print screen line
    for(y= SCREEN_HEIGHT-1; y>=0; y--) {
      #if defined(DMCP_BUILD)
        line_addr = lcd_line_addr(y);
      #elif defined(PC_BUILD) // !DMCP_BUILD
        // To be coded
      #endif // DMCP_BUILD

      for(i=2; i>= 0; i--) {
        offset = 3*x + i;
        if((offset > 0) && (offset < 50 )) {
          data =  ((((~*(line_addr + offset)) << 4) & 0xf0) | (((~*(line_addr + offset -1)) >> 4) & 0x0f));
        }
        else if(offset == 0) {
          data =  (((~*(line_addr + offset)) << 4) & 0xf0)  | 0x08;    // Print with right vertical line
        }
        else {
          data =  0x10 | (((~*(line_addr + offset -1)) >> 4) & 0x0f);  // Print with left vertical line
        }
        sendByteIR(data);
      }
    }
    // print top line
    sendByteIR((x == 16 ? 0x1f : 0xff));
    sendByteIR(0xff);
    sendByteIR((x == 0 ? 0xf1 : 0xff));
    sendByteIR('\n');
    //print_lf();
  }

  setPrintMode(0x00);  // Default 24 chars/line
  print_lf();
}

/*
//
//  Return the width of alpha to x
//
void cmdprintwidth(enum nilop op) {
  setX_int_sgn(buffer_width(Alpha), 0);
}
*/


static bool_t _printRegRange(uint16_t firstRegisterNo, uint16_t lastRegisterNo) {
  uint16_t regist;
  currentKeyCode = 255;
  if(firstRegisterNo <= lastRegisterNo) {
    for(regist = firstRegisterNo; regist <= lastRegisterNo; regist++) {
      fnP_Regs (regist);
      if(_exitKeyPressed()) {
        return true;
      }
    }
  }
  else {
    for(regist = firstRegisterNo; regist >= lastRegisterNo; regist--) {
      fnP_Regs (regist);
      if(_exitKeyPressed()) {
        return true;
      }
    }
  }
  currentKeyCode = 255;
  return false;
}

//
//  Trace error function
//
void printTraceErrorFunction(int16_t func, char *errorString) {
  if((calcMode != CM_NORMAL) || ((tam.mode) && ((func == ITM_BACKSPACE) || (func == ITM_EXIT1)))) {
    return;  // do Trace only in normal mode
  }
  if(getSystemFlag(FLAG_TRACE) && getSystemFlag(FLAG_PRTACT)) {   // Trace mode and printer active
    nameAlias(func, tmpString);
    strcat(tmpString, " ");
    strcat(tmpString, errorString);
    printJustified(tmpString);
    leaveTamModeIfEnabled();

    #if defined(PC_BUILD) && defined(MONITOR_IRPRINT)
      printf("**[DL]** Trace: %s\n", tmpString);
      fflush(stdout);
    #endif // PC_BUILD
  }
}

//
//  Trace errors
//
void printTraceError(char *errorString) {
  if(getSystemFlag(FLAG_TRACE) && getSystemFlag(FLAG_PRTACT)) {   // Trace mode and printer active
    printLine(errorString, 1);

    #if defined(PC_BUILD) && defined(MONITOR_IRPRINT)
      printf("**[DL]** Trace: %s\n", errorString);
      fflush(stdout);
    #endif // PC_BUILD
  }
}

//
//  Trace the X register
//
void printTraceX(uint16_t where) {
  if((getSystemFlag(FLAG_TRACE)|| getSystemFlag(FLAG_NORM)) && getSystemFlag(FLAG_PRTACT)) {   // Trace or Norm mode and printer active
    if(getSystemFlag(FLAG_TRACE)&& getSystemFlag(FLAG_NORM) && (where == LINE_FULL)) {
      _printRegRange(getSystemFlag(FLAG_SSIZE8) ? REGISTER_D : REGISTER_T, REGISTER_X);  // Trace with stack
      print_lf();
    }
    else {
      printReg(REGISTER_X, NULL, false, where, false);  // Print register X without name header
    }

    #if defined(PC_BUILD) && defined(MONITOR_IRPRINT)
      printf("**[DL]** Trace: %s\n", tmpString);
      fflush(stdout);
    #endif // PC_BUILD
  }
}

//
//  Trace a Matrix editing element
//
void printTraceMatElement(uint16_t where) {
  if(getSystemFlag(FLAG_TRACE) && getSystemFlag(FLAG_PRTACT)) {   // Trace mode and printer active
    const int16_t i = getIRegisterAsInt(true);
    const int16_t j = getJRegisterAsInt(true);

    if(getRegisterDataType(matrixIndex) == dtReal34Matrix) {
      real34Matrix_t mat;
      real34Matrix_t *matrix = &mat;
      convertReal34MatrixRegisterToReal34Matrix(matrixIndex, &mat);
      reallocateRegister(TEMP_REGISTER_1, dtReal34, 0, amNone);
      real34Copy(&matrix->matrixElements[i * matrix->header.matrixColumns + j], REGISTER_REAL34_DATA(TEMP_REGISTER_1));
    }
    else {
      complex34Matrix_t mat;
      complex34Matrix_t *matrix = &mat;
      convertComplex34MatrixRegisterToComplex34Matrix(matrixIndex, &mat);
      reallocateRegister(TEMP_REGISTER_1, dtComplex34, 0, amNone);
      complex34Copy(&matrix->matrixElements[i * matrix->header.matrixColumns + j], REGISTER_COMPLEX34_DATA(TEMP_REGISTER_1));
    }
    printReg(TEMP_REGISTER_1, NULL, false, where, false);  // Print temporary register 1 without name header

    #if defined(PC_BUILD) && defined(MONITOR_IRPRINT)
      printf("**[DL]** printTraceMatElement where %d Trace: %s\n", where, tmpString);
      fflush(stdout);
    #endif // PC_BUILD
  }
}

//
//  Trace a string
//
void printTraceString(char *string, uint16_t where) {
  int16_t lenInBytes = stringByteLength(string) + 1;

  if((getSystemFlag(FLAG_TRACE)|| getSystemFlag(FLAG_NORM)) && getSystemFlag(FLAG_PRTACT)) {   // Trace or Norm mode and printer active
    reallocateRegister(TEMP_REGISTER_1, dtString, TO_BLOCKS(lenInBytes), amNone);
    xcopy(REGISTER_STRING_DATA(TEMP_REGISTER_1), string, lenInBytes);
    printReg(TEMP_REGISTER_1, NULL, false, where, false);  // Print register X without name header

    #if defined(PC_BUILD) && defined(MONITOR_IRPRINT)
      printf("**[DL]** printTraceString Trace: %s\n", tmpString);
      fflush(stdout);
    #endif // PC_BUILD
  }
}

//
//  Trace the temporary information
//
void printTraceTI() {
  if(calcMode != CM_NORMAL) {
    return;  // do Trace only in normal mode
  }

  if(getSystemFlag(FLAG_TRACE) && getSystemFlag(FLAG_PRTACT)) {   // Trace mode and printer active
    if((programRunStop != PGM_RUNNING) && (programRunStop != PGM_SINGLE_STEP)) { // Not executing a program instruction
      tmpString[0] = 0;
      switch(temporaryInformation) {
        case TI_FALSE:
          sprintf(tmpString, "false");
          break;
        case TI_TRUE:
          sprintf(tmpString, "true" );
          break;
      }
      if(tmpString[0] != 0) {
        printLine(tmpString, 1);
        #if defined(PC_BUILD) && defined(MONITOR_IRPRINT)
          printf("**[DL]** Trace: %s\n", tmpString);
          fflush(stdout);
        #endif // PC_BUILD
      }
    }
  }
}

void _getRegisterLabel(uint16_t registerNo, char *label) {
  label[0] = 0;
  if(REGISTER_X <= registerNo && registerNo <= REGISTER_W) {
    label[0] = letteredRegisterName((calcRegister_t)registerNo);
    label[1] = 0;
  }
  else if(registerNo < REGISTER_X) {
    sprintf(label, "R%02d", registerNo);
  }
  else if(FIRST_LOCAL_REGISTER <= registerNo && registerNo <= LAST_LOCAL_REGISTER) {
    sprintf(label, "R.%03d", registerNo-100);
  }
  else if(FIRST_NAMED_VARIABLE <= registerNo && registerNo <= LAST_NAMED_VARIABLE) {
    sprintf(label, "%s", (char *)allNamedVariables[registerNo - FIRST_NAMED_VARIABLE].variableName + 1);
  }
  else if(FIRST_NAMED_RESERVED_VARIABLE <= registerNo && registerNo <= LAST_RESERVED_VARIABLE) {
    sprintf(label, "%s", (char *)allReservedVariables[registerNo - FIRST_RESERVED_VARIABLE].reservedVariableName + 1);
  }
}

//
//  INPUT and PROMPT printing
//
void printInputPrompt(uint16_t func, uint16_t regist) {
  if((getSystemFlag(FLAG_TRACE) || getSystemFlag(FLAG_NORM)) && getSystemFlag(FLAG_PRTACT)) {   // Trace or Norm mode and printer active
    if(getSystemFlag(FLAG_PRTEN) || ((programRunStop != PGM_RUNNING) && (programRunStop != PGM_SINGLE_STEP) && (programRunStop != PGM_WAITING))) { // No printing in a program if PRTEN cleared
      if(func == ITM_PROMPT) {
        printReg(regist, NULL, false, LINE_ASIS, false);  // Print register left justified without name header
        print_lf();
        #if defined(PC_BUILD) && defined(MONITOR_IRPRINT)
          printf("**[DL]** Trace: %s\n", tmpString);
          fflush(stdout);
        #endif // PC_BUILD
      }
      else {  // ITM_INPUT
        char label[16];
        uint16_t len;
        _getRegisterLabel(regist, label);
        len = strlen(label);
        label[len++] = '?';
        label[len] = 0;
        printReg(regist, label, false, LINE_ASIS, false);  // Print register left justified with name header
        print_lf();
        #if defined(PC_BUILD) && defined(MONITOR_IRPRINT)
          printf("**[DL]** Trace: %s\n", tmpString);
          fflush(stdout);
        #endif // PC_BUILD
        tmpString[0] = 0;
      }
    }
  }
}

//
//  VIEW and AVIEW printing
//
void printViewAview(uint16_t func, uint16_t regist) {
  if(getSystemFlag(FLAG_PRTACT)) {   // Man, Trace or Norm mode and printer active
    if(getSystemFlag(FLAG_PRTEN) || ((programRunStop != PGM_RUNNING) && (programRunStop != PGM_SINGLE_STEP))) { // No printing in a program if PRTEN cleared
      if(func == ITM_VIEW) {
        char label[16];
        _getRegisterLabel(regist, label);
        printReg(regist, label, true, LINE_ASIS, false);  // Print register left justified with name header
        print_lf();
        #if defined(PC_BUILD) && defined(MONITOR_IRPRINT)
          printf("**[DL]** Trace: %s=%s\n", label, tmpString);
          fflush(stdout);
        #endif // PC_BUILD
      }
      else {  // ITM_AVIEW
        printReg(regist, NULL, false, LINE_ASIS, false);  // Print register left justified without name header
        print_lf();
        #if defined(PC_BUILD) && defined(MONITOR_IRPRINT)
          printf("**[DL]** Trace: %s\n", tmpString);
          fflush(stdout);
        #endif // PC_BUILD
      }
    }
  }
  else {
    if(getSystemFlag(FLAG_PRTEN) && (programRunStop == PGM_RUNNING)) { // Stop in a program if PRTEN and not PRTACT
      fnStopProgram(NOPARAM);
    }
  }
}


//
//  Trace an instruction
//
void printTrace(int16_t func, uint16_t param) {
  char traceBuffer[32];

  printerState.trace_done = true;
  if(((calcMode != CM_NORMAL) && (calcMode != CM_MIM)) || ((tam.mode) && ((func == ITM_BACKSPACE) || (func == ITM_EXIT1)))) {
    return;  // Trace only in normal mode
  }

  if((getSystemFlag(FLAG_TRACE) || getSystemFlag(FLAG_NORM))&& getSystemFlag(FLAG_PRTACT)) {    // Trace or Norm mode and printer active
    if((programRunStop != PGM_RUNNING) && (programRunStop != PGM_SINGLE_STEP)) { // Not executing a program instruction
      if(func < 0) {   // Menu
        if(func == -MNU_DYNAMIC) {
          printJustified(userMenus[currentUserMenu].menuName);    // User Menu
          #if defined(PC_BUILD) && defined(MONITOR_IRPRINT)
            printf("**[DL]** Trace: %s\n", userMenus[currentUserMenu].menuName);
            fflush(stdout);
          #endif // PC_BUILD
        }
        else {
          printJustified(indexOfItems[-func].itemSoftmenuName);   // Predefined Menu
          #if defined(PC_BUILD) && defined(MONITOR_IRPRINT)
            printf("**[DL]** Trace: %s\n", indexOfItems[-func].itemSoftmenuName);
            fflush(stdout);
          #endif // PC_BUILD
        }
      }
      else if((func != ITM_PRINTERADV) && (func != ITM_PRINTERX)) {           // Function (don't trace printer ADV nor PRX)
        char buffer[16];
        xcopy(buffer, tmpString, 16);    // Save tmpString content for dynamic menus
        nameAlias(func, tmpString);
        if((indexOfItems[func].func != addItemToBuffer) && (indexOfItems[func].param != NOPARAM) && ((indexOfItems[func].status & PTP_STATUS) != PTP_NONE)) {
          traceBuffer[0] = 0;
          if(tam.indirect) {
            strcat(tmpString, " " STD_RIGHT_ARROW);
            param = tam.value0;
          }
          if((tam.mode == TM_VALUE) & !tam.indirect) {   // Needs to be the first if to correctly catch values
            uint16_t tamMax = indexOfItems[func].tamMinMax & TAM_MAX_MASK;
            if(tamMax <= 9) {
              sprintf(traceBuffer, " %u", param);
            }
            else if(tamMax <= 99) {
              sprintf(traceBuffer, " %02u", param);
            }
            else if(tamMax <= 999) {
              sprintf(traceBuffer, " %03u", param);
            }
            else {
              sprintf(traceBuffer, " %04u", param);
            }
          }
          else if((func == ITM_OPEN_MENU) && !tam.indirect) {
            if(param == MNU_DYNAMIC) {
              sprintf(traceBuffer, " " STD_LEFT_SINGLE_QUOTE "%s" STD_RIGHT_SINGLE_QUOTE, buffer);
            }
            else {
              sprintf(traceBuffer, " " STD_LEFT_SINGLE_QUOTE "%s" STD_RIGHT_SINGLE_QUOTE, indexOfItems[tam.value].itemCatalogName);
            }
          }
          else if(tam.mode == TM_SHUFFLE) {
            sprintf(traceBuffer, " %c%c%c%c", shuffleReg[ param & 0x03      ],
                                              shuffleReg[(param & 0x0c) >> 2],
                                              shuffleReg[(param & 0x30) >> 4],
                                              shuffleReg[(param & 0xc0) >> 6]);
          }
          else if((param >= FIRST_NAMED_VARIABLE) && (param <= LAST_NAMED_VARIABLE)) {
            if(!tam.indirect) {
              strcat(tmpString, " ");
            }
            strcat(tmpString, STD_LEFT_SINGLE_QUOTE);
            strcat(tmpString, (char *)allNamedVariables[param - FIRST_NAMED_VARIABLE].variableName + 1);
            strcat(tmpString, STD_RIGHT_SINGLE_QUOTE);
          }
          else if((param >= FIRST_NAMED_RESERVED_VARIABLE) && (param <= LAST_RESERVED_VARIABLE)) {
            if(!tam.indirect) {
              strcat(tmpString, " ");
            }
            strcat(tmpString, STD_LEFT_SINGLE_QUOTE);
            strcat(tmpString, (char *)allReservedVariables[param - FIRST_RESERVED_VARIABLE].reservedVariableName + 1);
            strcat(tmpString, STD_RIGHT_SINGLE_QUOTE);
          }
          else if((tam.mode == TM_LABEL || tam.mode == TM_LBLONLY || func == ITM_XEQ) && !tam.indirect) {
            if(param < 99) { // Local label from 00 to 99
              sprintf(traceBuffer, " %02u", param);
            }
            else if(param <= LAST_UC_LOCAL_LABEL) { // Local label from A to L
              sprintf(traceBuffer, " %c", 'A' + (param - 100));
            }
            else if(param <= LAST_LOCAL_LABEL) { // Local label from a to l
              sprintf(traceBuffer, " %c", 'a' + (param - FIRST_LC_LOCAL_LABEL));
            }
            else if((param >= FIRST_LABEL) && (param <= LAST_LABEL)) { // Alpha labels
              strcat(tmpString, " " STD_LEFT_SINGLE_QUOTE);
              uint16_t strLength = stringByteLength(tmpString);
              xcopy(tmpString + strLength, labelList[param - FIRST_LABEL].labelPointer + 1, *(labelList[param - FIRST_LABEL].labelPointer));
              tmpString[strLength + *(labelList[param - FIRST_LABEL].labelPointer)] = 0;
              strcat(tmpString, STD_RIGHT_SINGLE_QUOTE);
            }
          }
          else if((tam.mode == TM_FLAGR || tam.mode == TM_FLAGW) && !tam.indirect) {
            if(param < FLAG_X) { // Global flag from 00 to 99
              sprintf(traceBuffer, " %02u", param);
            }
            else if(FLAG_X <= param && param <= FLAG_K) { // Lettered flag from X to K
              sprintf(traceBuffer, " %c", registerFlagLetters[param - FLAG_X]);
            }
            else if(param <= LAST_LOCAL_FLAG) { // Local flag from .00 to .31
              sprintf(traceBuffer, " .%02d", param - FIRST_LOCAL_FLAG);
            }
            else if(param < FLAG_M) { // Local flag from .32 to .98 are illegal
              sprintf(traceBuffer, " %02u", param);
            }
            else if(param <= FLAG_W) { // Lettered flag from M to S and E to W
              sprintf(traceBuffer, " %c", registerFlagLetters[param - FLAG_M + (FLAG_K - FLAG_X + 1)]);
            }
            else if(param & 0x8000) { // System flag
              param &= 0x3fff;
              if(param < 64) {
                sprintf(traceBuffer, " " STD_LEFT_SINGLE_QUOTE "%s" STD_RIGHT_SINGLE_QUOTE, indexOfItems[param + SFL_TDM24].itemSoftmenuName);
              }
              else {
                sprintf(traceBuffer, " " STD_LEFT_SINGLE_QUOTE "%s" STD_RIGHT_SINGLE_QUOTE, indexOfItems[param + SFL_MONIT - 64].itemSoftmenuName);
              }
            }
          }
          else if((tam.mode == TM_CMP) && (param == TEMP_REGISTER_1) && !tam.indirect) {
            sprintf(traceBuffer, real34IsZero(REGISTER_REAL34_DATA(TEMP_REGISTER_1)) ? " 0." : " 1.");
          }
          else {
            if(param < 99) { // Global register from 00 to 99
              if(!tam.indirect) {
                strcat(tmpString, " ");
              }
              sprintf(traceBuffer, "%02u", param);
            }
            else if(param <= REGISTER_K) { // Lettered register from X to K
              if(!tam.indirect) {
                strcat(tmpString, " ");
              }
              sprintf(traceBuffer, "%s", indexOfItems[ITM_REG_X + param - REGISTER_X].itemSoftmenuName);
            }
            else if(param <= REGISTER_W) { // Lettered register from M to S and E to W
              if(!tam.indirect) {
                strcat(tmpString, " ");
              }
              sprintf(traceBuffer, "%s", indexOfItems[ITM_REG_M + param - REGISTER_M].itemSoftmenuName);
            }
            else if((param >= FIRST_LOCAL_REGISTER) && (param <= LAST_LOCAL_REGISTER)) { // Local register from .00 to .98
              if(!tam.indirect) {
                strcat(tmpString, " ");
              }
              sprintf(traceBuffer, ".%02d", param - FIRST_LOCAL_REGISTER);
            }
          }
          strcat(tmpString, traceBuffer);
        }
        else if((func == ITM_MENU) && (dynamicMenuItem >= 0)) {
          xcopy(tmpString, programmableMenu.itemName[dynamicMenuItem], 16);
        }
        uint16_t width = stringGlyphLength(tmpString) * 7 - 1;
        if(printerColumn + width > PAPER_WIDTH) {
          printAdvance(0);
        }
        printJustified(tmpString);

        #if defined(PC_BUILD) && defined(MONITOR_IRPRINT)
          printf("**[DL]** Trace: %s\n", tmpString);
          fflush(stdout);
        #endif // PC_BUILD
      }
    }
    else if(getSystemFlag(FLAG_TRACE)) {  // Program running or single stepping - Trace mode only
      decodeOneStep_ALIAS(currentStep);
      if(func == ITM_LBL) {
        printAdvance(0); // Skip one line before printing the label
        sprintf(traceBuffer, " %02d", currentLocalStepNumber);
        strcat(traceBuffer, STD_BLACK_RIGHT_TRIANGLE);
        strcat(traceBuffer, tmpString);
        printJustified(traceBuffer);     // Current step & step number
        #if defined(PC_BUILD) && defined(MONITOR_IRPRINT)
          printf("**[DL]** Trace: %s\n", traceBuffer);
          fflush(stdout);
        #endif // PC_BUILD
      }
      else if(func >= 0) {  // Don't trace menu names during program execution
        printJustified(tmpString);    // Current step
        #if defined(PC_BUILD) && defined(MONITOR_IRPRINT)
          printf("**[DL]** Trace: %s\n", tmpString);
          fflush(stdout);
        #endif // PC_BUILD
      }
    }
  }
}


void nameAlias(uint16_t op, char *nameOp) {
  uint16_t i = 0;
  while(NamesAlias[i].item != LAST_ITEM) {
    if(NamesAlias[i].item == op) {
      strcpy(nameOp, NamesAlias[i].name);
      return;
    }
    i++;
  }
  strcpy(nameOp, indexOfItems[op].itemCatalogName);
}


//
//  Print a program listing
//  Start at the PC location
//
void printProgram(bool_t list, uint16_t lines) {
  ///////////////////////////////////////////////////////////////////////////////////////
  // For details, see fnPem(). This is a modified copy.
  //
  currentKeyCode = 255;
  uint16_t line, firstLine, lastLine;
  uint8_t *step, *nextStep;
  currentKeyCode = 255;
  RETURN_IF_PRINT_OFF;
  advanceIfTrace();

  firstDisplayedLocalStepNumber = 0;
  if(!list) {
    defineFirstDisplayedStep();
    step = firstDisplayedStep;
  }
  else {
    step = currentStep;
  }
  programListEnd           = false;
  lastProgramListEnd       = false;

  print_lf();

  if(!list) {  // Print Program (pr_PROG)
    // Time and date header line
    if(getSystemFlag(FLAG_TRACE)) { // Compact program format
      printLine(" ", 0);            // add a space before the header
    }
    getTimeString(tmpString);
    printLine(tmpString, 0);
    printLine(" ", 0);
    getDateString(tmpString);
    printLine(tmpString, 1);
    print_lf();

    if(firstDisplayedLocalStepNumber == 0) {
      if(getSystemFlag(FLAG_TRACE)) { // Compact program format
        printLine(" ", 0);            // add a space before the first line
      }
      sprintf(tmpString, "00 { %" PRIu32 "-Byte Prgm }", _getProgramSize());
      printLine(tmpString, 1);
      firstLine = 1;
    }
    else {
      firstLine = 0;
    }
    lastLine = 9999;
  }
  else {       // Print listing (pr_LIST)
    firstLine = currentLocalStepNumber;
    lastLine = firstLine + lines - 1;
  }

  int lineOffset = 0, lineOffsetTam = 0;

  bool_t  isLabel;
  bool_t  startOfLine = true;

  for(line=firstLine; line<=lastLine; line++) {
    nextStep = findNextStep(step);
    isLabel = (*step == ITM_LBL);

    //Line Number
    if(getSystemFlag(FLAG_TRACE)) {   // Compact program format
      if(isLabel) {
        if((line != 01) && !startOfLine) {
          printAdvance(0); // Print pending line
        }
        printAdvance(0); // Skip one line before printing the label
        sprintf(tmpString, " %02d" , firstDisplayedLocalStepNumber + line - lineOffset + lineOffsetTam);
        strcat(tmpString, STD_BLACK_RIGHT_TRIANGLE);
        printLine(tmpString, 0);
      }
      else if(!startOfLine) {
        if(printerColumn + 14 <= PAPER_WIDTH) {
          printLine("  ", 0);
        }
        else {
          printAdvance(0);
        }
      }
    }
    else {
      sprintf(tmpString, "%02d" , firstDisplayedLocalStepNumber + line - lineOffset + lineOffsetTam);
      strcat(tmpString, (isLabel ? STD_BLACK_RIGHT_TRIANGLE : " "));
      printLine(tmpString, 0);
    }

    //Decode instruction
    decodeOneStep_ALIAS(step);

    if(getSystemFlag(FLAG_TRACE)) {   // Compact program format
      if(printerColumn + stringGlyphLength(tmpString)*7 > PAPER_WIDTH + 2 ) {
        printAdvance(0);
      }
      printLine(tmpString, isLabel);
      startOfLine = isLabel;
    }
    else {
      printLine(tmpString, 1);
    }

    if(isAtEndOfProgram(step)) {
      programListEnd = true;
      if(*nextStep == 255 && *(nextStep + 1) == 255) {
        lastProgramListEnd = true;
      }
      break;
    }
    if((*step == 255) && (*(step + 1) == 255)) {
      programListEnd = true;
      lastProgramListEnd = true;
      break;
    }
    step = nextStep;
    if(_exitKeyPressed()) {
      break;
    }
  }
  if(getSystemFlag(FLAG_TRACE)) { // Compact program format
      printAdvance(0);            // print remaining buffer content
  }
}


static uint16_t _getUnicodeValue(calcRegister_t regist) {
  int32_t value;

  if(getRegisterDataType(regist) == dtReal34) {
    real34_t maxValue34;

    int32ToReal34(0x8000, &maxValue34);
    if(real34CompareLessThan(REGISTER_REAL34_DATA(regist), const34_0) || real34CompareLessEqual(&maxValue34, REGISTER_REAL34_DATA(regist))) {
      displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
      #if defined(PC_BUILD)
        real34ToString(REGISTER_REAL34_DATA(regist), errorMessage);
        sprintf(tmpString, "x %" PRId16 " = %s:", regist, errorMessage);
        moreInfoOnError("In function _getPositionFromRegister:", tmpString, "this value is negative or too big!", NULL);
      #endif // PC_BUILD
      return -1;
    }
    value = real34ToInt32(REGISTER_REAL34_DATA(regist));
  }

  else if(getRegisterDataType(regist) == dtLongInteger) {
    longInteger_t lgInt;

    convertLongIntegerRegisterToLongInteger(regist, lgInt);
    if(longIntegerCompareUInt(lgInt, 0) < 0 || longIntegerCompareUInt(lgInt, 0x8000) >= 0) {
      displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
      #if defined(PC_BUILD)
        longIntegerToAllocatedString(lgInt, errorMessage, ERROR_MESSAGE_LENGTH);
        sprintf(tmpString, "register %" PRId16 " = %s:", regist, errorMessage);
        moreInfoOnError("In function _getPositionFromRegister:", tmpString, "this value is negative or too big!", NULL);
      #endif // PC_BUILD
      longIntegerFree(lgInt);
      return -1;
    }
    longIntegerToUInt32(lgInt, value);
    longIntegerFree(lgInt);
  }

  else if(getRegisterDataType(regist) == dtShortInteger) {
    longInteger_t lgInt;

    longIntegerInit(lgInt);
    convertShortIntegerRegisterToLongInteger(regist, lgInt);
    longIntegerToUInt32(lgInt, value);
    longIntegerFree(lgInt);
  }

  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
    #if defined(PC_BUILD)
      sprintf(errorMessage, "register %" PRId16 " is %s:", regist, getRegisterDataTypeName(regist, true, false));
      moreInfoOnError("In function _getPositionFromRegister:", errorMessage, "not suited for addressing!", NULL);
    #endif // PC_BUILD
    return -1;
  }

  return value;
}
#endif // IR_PRINTING


//********************************************************
// PRINTER FUNCTIONS
//********************************************************

// Printer On/Off
void fnP_PrinterOnOff(uint16_t op) {
  //#if defined(IR_PRINTING)
    if(op == PRON) {
      printerState.print_on = true;
      setSystemFlag(FLAG_PRTACT);
      fnSetFlag(FLAG_PRTEN);
    }
    else if(op == PROFF) {
      printerState.print_on = false;
      clearSystemFlag(FLAG_PRTACT);
      fnClearFlag(FLAG_PRTEN);
    }
  //#endif //IR_PRINTING
}

// Printer Mode
void fnP_PrinterMode(uint16_t mode) {
  //#if defined(IR_PRINTING)
    if(mode == MAN) {
      fnClearFlag(FLAG_NORM);
      fnClearFlag(FLAG_TRACE);
    }
    else if(mode == NORM) {
      fnSetFlag(FLAG_NORM);
      fnClearFlag(FLAG_TRACE);
    }
    else if(mode == TRACE) {
      fnClearFlag(FLAG_NORM);
      fnSetFlag(FLAG_TRACE);
    }
    else if(mode == STRACE) {
      fnSetFlag(FLAG_NORM);
      fnSetFlag(FLAG_TRACE);
    }
  //#endif //IR_PRINTING
}

// Printer model selection
void fnSetPrinter(uint16_t model) {
  //#if defined(IR_PRINTING)
    printerState.printer_model = model;
  //#endif //IR_PRINTING
}

// Get printer line delay
void fnP_GetDelay(uint16_t unusedButMandatoryParameter) {
  #if defined(IR_PRINTING)
    longInteger_t delay;

    liftStack();

    longIntegerInit(delay);
    int32ToLongInteger(getLineDelay(), delay);
    convertLongIntegerToLongIntegerRegister(delay, REGISTER_X);
    longIntegerFree(delay);
  #endif //IR_PRINTING
}

// Set printer line delay
void fnP_SetDelay(uint16_t delay) {
  #if defined(IR_PRINTING)
    printerState.delay = delay;
    setLineDelay(delay);
  #endif //IR_PRINTING
}

// Printer paper advance
void fnP_Advance(uint16_t unusedButMandatoryParameter) {
  #if defined(IR_PRINTING)  // Show Print SBI
    setPrinterSBI(true);
    print_lf();
    setPrinterSBI(false);
  #endif //IR_PRINTING
}

// Print program list
void fnP_PrinterList(uint16_t lines) {
  #if defined(IR_PRINTING)
    printProgram(LIST, lines);
  #endif //IR_PRINTING
}

// Print byte
void fnP_Byte(uint16_t byte) {
  #if defined(IR_PRINTING)
    setPrinterSBI(true);
    cmdPrint(byte, PRINT_BYTE);
    setPrinterSBI(false);
  #endif //IR_PRINTING
}

// Print a character using character set translation
void fnP_Char(uint16_t registerNo) {
  #if defined(IR_PRINTING)
    uint16_t character;
    setPrinterSBI(true);
    character = _getUnicodeValue(registerNo);
    cmdPrint(character, PRINT_CHAR);
    setPrinterSBI(false);
  #endif //IR_PRINTING
}


// Print Tab
void fnP_Tab(uint16_t column) {
  #if defined(IR_PRINTING)
    setPrinterSBI(true);
    cmdPrint(column, PRINT_TAB);
    setPrinterSBI(false);
  #endif //IR_PRINTING
}


// Print User
void fnP_User(uint16_t unusedButMandatoryParameter) {
  #if defined(IR_PRINTING)
    char label[16];
    currentKeyCode = 255;

    if(!getSystemFlag(FLAG_PRTACT)) {
      if(getSystemFlag(FLAG_PRTEN) || ((programRunStop != PGM_RUNNING) && (programRunStop != PGM_SINGLE_STEP))) {
        displayCalcErrorMessage(ERROR_PRINTING_DISABLED, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
        #if defined(PC_BUILD)
          sprintf(errorMessage, "Printing is disabled");
          moreInfoOnError("In function fnP_User:", errorMessage, NULL, NULL);
        #endif // PC_BUILD
      }
      return;
    }
    // Print User variables
    calcRegister_t variable;
    bool_t userVariableFound = false;
    for(int i=0; i<numberOfNamedVariables; i++) {
      xcopy(label, allNamedVariables[i].variableName + 1, allNamedVariables[i].variableName[0]);
      label[allNamedVariables[i].variableName[0]] = 0;
      if((compareString(label, "STATS", CMP_NAME) == 0) || (compareString(label, "HISTO", CMP_NAME) == 0) || (compareString(label, "Mat_A", CMP_NAME) == 0) ||
         (compareString(label, "Mat_B", CMP_NAME) == 0) || (compareString(label, "Mat_X", CMP_NAME) == 0)) { // Don't include "STATS", "HISTO", "Mat_A", "Mat_B" and "Mat_X"
        continue;
      }
      else {
        variable =  findNamedVariable(label);
        printReg(variable, label, true, LINE_FULL, false );
        userVariableFound = true;
        if(_exitKeyPressed()) {
          return;
        }
      }
      if(_exitKeyPressed()) {
        return;
      }
    }

    if(userVariableFound) {
      print_lf();
    }

    // Print User Program Labels
    uint8_t *nextStep, *step = beginOfProgramMemory;
    uint16_t programNumber = 1;
    bool_t firstProgramLabel = true;
    while(!isAtEndOfPrograms(step)) { // .END.
      nextStep = findNextStep(step);
      if(checkOpCodeOfStep(step, ITM_LBL)) { // LBL
        if(*(step + 1) > LAST_LOCAL_LABEL) { // Global label
          xcopy(label, step + 3, *(step+2));
          label[*(step+2)] = 0;
          printLine("LBL " STD_LEFT_SINGLE_QUOTE, 0);
          printLine(label, 0);
          printLine(STD_RIGHT_SINGLE_QUOTE, !firstProgramLabel);
          if(firstProgramLabel) {
            sprintf(tmpString, "Prgm #%" PRIu16 "/%" PRIu16 "", programNumber, numberOfPrograms);
            printJustified(tmpString);
            firstProgramLabel = false;
          }
        }
      }

      if(isAtEndOfProgram(step)) { // END
        printLine("END", !firstProgramLabel);
        if(firstProgramLabel) {
          sprintf(tmpString, "Prgm #%" PRIu16 "/%" PRIu16 "", programNumber, numberOfPrograms);
          printJustified(tmpString);
        }
        programNumber++;
        firstProgramLabel = true;
      }

      step = nextStep;

      if(_exitKeyPressed()) {
        return;
      }
    }

    printLine(".END.", 1);

  #endif //IR_PRINTING*
}

// Print LCD
void fnP_LCD(uint16_t unusedButMandatoryParameter) {
  if(getSystemFlag(FLAG_PRTACT)) {  // Print to the printer)
    #if defined(IR_PRINTING)
    return; // Not yet working for the 82240 printer
    setPrinterSBI(true);
    resetShiftState();                  //JM To avoid f or g top left of the screen, clear to make sure
    refreshScreen(80);
    printLcd();
    setPrinterSBI(false);
    #endif //IR_PRINTING
  }
  else {                             // SNAP
    fnSNAP(NOPARAM);
  }
}


// Print Alpha string
void fnP_Alpha(uint16_t registerNo) {
  if(getSystemFlag(FLAG_PRTACT)) {  // Print to the printer)
  #if defined(IR_PRINTING)
    if(getRegisterDataType(registerNo) == dtString) {
      printAlpha(REGISTER_STRING_DATA(registerNo), PRINT_ALPHA);
    }
  #endif //IR_PRINTING
  }
  else {                             // Print to file
    if(calcMode != CM_AIM) {
      #if defined(DMCP_BUILD)
        beep(440, 50);
        beep(4400, 50);
        beep(440, 50);
      #endif // DMCP_BUILD
      return;
    }
    xcopy(tmpString, aimBuffer, ERROR_MESSAGE_LENGTH + AIM_BUFFER_LENGTH + NIM_BUFFER_LENGTH);       //backup portion of the "message buffer" area in DMCP used by ERROR..AIM..NIM buffers, to the tmpstring area in DMCP. DMCP uses this area during create_screenshot.
    create_filename(".REGS.TSV");

    #if (VERBOSE_LEVEL >= 1)
      clearScreen(2);
      print_linestr("Output Aim Buffer to drive:", true);
      print_linestr(filename_csv, false);
    #endif // VERBOSE_LEVEL >= 1

    tmpString_csv_out(5);          //aimBuffer now already copied to tmpString
    xcopy(aimBuffer, tmpString, ERROR_MESSAGE_LENGTH + AIM_BUFFER_LENGTH + NIM_BUFFER_LENGTH);        //   This total area must be less than the tmpString storage area, which it is.
    //print_linestr(aimBuffer, false);
  }
}



void fnP_Regs (uint16_t registerNo) {
  if(getSystemFlag(FLAG_PRTACT)) {  // Print to the printer
  #if defined(IR_PRINTING)
    char label[16];
    label[0] = 0;
    if(REGISTER_X <= registerNo && registerNo <= REGISTER_W) {
      label[0] = letteredRegisterName((calcRegister_t)registerNo);
      label[1] = 0;
      }
    else if(registerNo < REGISTER_X) {
      sprintf(label, "R%02d", registerNo);
    }
    else if(FIRST_LOCAL_REGISTER <= registerNo && registerNo <= LAST_LOCAL_REGISTER) {
      sprintf(label, "R.%03d", registerNo-100);
    }
    else if(FIRST_NAMED_VARIABLE <= registerNo && registerNo <= LAST_NAMED_VARIABLE) {
      sprintf(label, "%s", (char *)allNamedVariables[registerNo - FIRST_NAMED_VARIABLE].variableName + 1);
    }
    else if(FIRST_NAMED_RESERVED_VARIABLE <= registerNo && registerNo <= LAST_RESERVED_VARIABLE) {
      sprintf(label, "%s", (char *)allReservedVariables[registerNo - FIRST_RESERVED_VARIABLE].reservedVariableName + 1);
    }
    printReg(registerNo, label, true, LINE_FULL, false );
  #endif //IR_PRINTING
  }
  else {                             // Print to file
    if(calcMode != CM_NORMAL) {
      #if defined(DMCP_BUILD)
        beep(440, 50);
        beep(4400, 50);
        beep(440, 50);
      #endif // DMCP_BUILD
      return;
    }

    create_filename(".REGS.TSV");

    #if (VERBOSE_LEVEL >= 1)
      clearScreen(3);
      print_linestr("Output regs to drive:", true);
      print_linestr(filename_csv, false);
    #endif // VERBOSE_LEVEL >= 1

    stackregister_csv_out((int16_t)registerNo, (int16_t)registerNo, !ONELINE);
  }
}

TO_QSPI const summationRegisterName_t summationRegisterName[NUMBER_OF_STATISTICAL_SUMS] = {
          { "n"                             },
          { STD_SIGMA "X"                   },
          { STD_SIGMA "Y"                   },
          { STD_SIGMA "X" STD_SUP_2         },
          { STD_SIGMA "X" STD_SUP_2 "Y"     },
          { STD_SIGMA "Y" STD_SUP_2         },
          { STD_SIGMA "XY"                  },
          { STD_SIGMA "lnXlnY"              },
          { STD_SIGMA "lnX"                 },
          { STD_SIGMA "ln" STD_SUP_2 "X"    },
          { STD_SIGMA "Y"  STD_DOT  "lnX"   },
          { STD_SIGMA "lnY"                 },
          { STD_SIGMA "ln" STD_SUP_2 "Y"    },
          { STD_SIGMA "X"  STD_DOT  "lnY"   },
          { STD_SIGMA "X" STD_SUP_2 "lnY"   },
          { STD_SIGMA "lnY/X"               },
          { STD_SIGMA "X" STD_SUP_2 "/Y"    },
          { STD_SIGMA "1/X"                 },
          { STD_SIGMA "1/X" STD_SUP_2       },
          { STD_SIGMA "X/Y"                 },
          { STD_SIGMA "1/Y"                 },
          { STD_SIGMA "1/Y" STD_SUP_2       },
          { STD_SIGMA "X" STD_SUP_3         },
          { STD_SIGMA "X" STD_SUP_4         },
          { STD_SIGMA "Xmin"                },
          { STD_SIGMA "Xmax"                },
          { STD_SIGMA "Ymin"                },
          { STD_SIGMA "Ymax"                }
};

void fnP_Sigma(uint16_t unusedButMandatoryParameter) {
  currentKeyCode = 255;
  if(statisticalSumsPointer != NULL) {
    if(getSystemFlag(FLAG_PRTACT)) {  // Print to the printer
    #if defined(IR_PRINTING)
      uint16_t regist;
      if(!getSystemFlag(FLAG_PRTEN) && ((programRunStop == PGM_RUNNING) || (programRunStop == PGM_SINGLE_STEP))) {
        displayCalcErrorMessage(ERROR_PRINTING_DISABLED, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
        #if defined(PC_BUILD)
          sprintf(errorMessage, "Printing is disabled");
          moreInfoOnError("In function fnP_Sigma:", errorMessage, NULL, NULL);
        #endif // PC_BUILD
        return;
      }
      for(regist = 0; regist < NUMBER_OF_STATISTICAL_SUMS; regist++) {
        convertRealToResultRegister(statisticalSumsPointer + regist, TEMP_REGISTER_1, amNone);
        printReg(TEMP_REGISTER_1, summationRegisterName[regist].name, true, LINE_FULL, true );
        if(_exitKeyPressed()) {
          return;
        }
      }
    #endif //IR_PRINTING
    }
    else {                             // Print to file
    }
  }
  else {
    displayCalcErrorMessage(ERROR_NO_SUMMATION_DATA, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function fnP_Sigma:", "There is no statistical data available!", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


void fnP_All_Regs(uint16_t option) {
  if(getSystemFlag(FLAG_PRTACT)) {  // Print to the printer
  #if defined(IR_PRINTING)
    bool_t exited;
    uint16_t s, n;
    switch(option) {
      case PRN_ALL:
        exited = _printRegRange(REGISTER_X, REGISTER_W);  // Lettered registers
        if(exited) {
          return;
        }
        exited = _printRegRange(0, 99);                   // Global registers
        if(exited) {
          return;
        }
        if(currentNumberOfLocalRegisters > 0){
          exited = _printRegRange(FIRST_LOCAL_REGISTER, FIRST_LOCAL_REGISTER + currentNumberOfLocalRegisters - 1);  // Local registers
          if(exited) {
            return;
          }
        }
        if(numberOfNamedVariables > 0) {
          exited = _printRegRange(FIRST_NAMED_VARIABLE, FIRST_NAMED_VARIABLE + numberOfNamedVariables - 1);  // Named Variablewss
          if(exited) {
            return;
          }
        }
        break;

      case PRN_REGS:
        if((lastErrorCode = getRegParam(NULL, &s, &n, NULL)) == ERROR_NONE) {
          _printRegRange(s, (s + n) -1);
        }
        else {
          displayCalcErrorMessage(lastErrorCode, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
        }
        break;

      case PRN_Xr:
        printReg(REGISTER_X, NULL, false, LINE_FULL, false );  // Print register X without name header
        break;

      case PRN_STK:
        _printRegRange(getSystemFlag(FLAG_SSIZE8) ? REGISTER_D : REGISTER_T, REGISTER_X);
        break;

      case PRN_XYr:
        switch(getRegisterDataType(REGISTER_X)) {
          case dtLongInteger:
          case dtReal34:
          case dtShortInteger:
          case dtString:
          case dtDate:
          case dtTime: {
            switch(getRegisterDataType(REGISTER_Y)) {
              case dtLongInteger:
              case dtReal34:
              case dtShortInteger:
              case dtString:
              case dtDate:
              case dtTime: {
                printReg(REGISTER_X, NULL, false, LINE_LEFT, false );   // Print register X on the left half of the line
                printReg(REGISTER_Y, NULL, false, LINE_RIGHT, false );  // Print register Y on the right half of the line
                return;
              }
              default: {
                displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_Y);
                #if (EXTRA_INFO_ON_CALC_ERROR == 1)
                  sprintf(errorMessage, "invalid data type %s for register Y", getRegisterDataTypeName(REGISTER_Y, true, false));
                  moreInfoOnError("In function fnP_All_Regs(PRN_XYr):", errorMessage, NULL, NULL);
                #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
                return;
              }
            }
            return;
          }


          case dtReal34Matrix: {
            real34Matrix_t    x;
            convertReal34MatrixRegisterToReal34Matrix(REGISTER_X, &x);

            if(x.header.matrixColumns == 2) {
              for(int i = 0; i < x.header.matrixRows; ++i) {
                reallocateRegister(TEMP_REGISTER_1, dtReal34, REAL34_SIZE_IN_BYTES, amNone);
                real34Copy(&x.matrixElements[i*2], REGISTER_REAL34_DATA(TEMP_REGISTER_1));
                printReg(TEMP_REGISTER_1, NULL, false, LINE_LEFT, false );    // Print row i col 1 on the left half of the line
                real34Copy(&x.matrixElements[i*2+1], REGISTER_REAL34_DATA(TEMP_REGISTER_1));
                printReg(TEMP_REGISTER_1, NULL, false, LINE_RIGHT, false );   // Print row i col 2 on the right half of the line
              }
            }
            else if(x.header.matrixRows == 2) {
              for(int i = 0; i < x.header.matrixColumns; ++i) {
                reallocateRegister(TEMP_REGISTER_1, dtReal34, REAL34_SIZE_IN_BYTES, amNone);
                real34Copy(&x.matrixElements[i], REGISTER_REAL34_DATA(TEMP_REGISTER_1));
                printReg(TEMP_REGISTER_1, NULL, false, LINE_LEFT, false );   // Print row 1 col i on the left half of the line
                real34Copy(&x.matrixElements[i + x.header.matrixColumns], REGISTER_REAL34_DATA(TEMP_REGISTER_1));
                printReg(TEMP_REGISTER_1, NULL, false, LINE_RIGHT, false );    // Print row 2 col i on the right half of the line
              }
            }
            else {
              displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE, REGISTER_X);
              #if (EXTRA_INFO_ON_CALC_ERROR == 1)
                sprintf(errorMessage, "cannot print xy when matrix size is %d x %d", x.header.matrixRows, x.header.matrixColumns);
                moreInfoOnError("In function fnP_All_Regs(PRN_XYr):", errorMessage, NULL, NULL);
              #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
            }
            return;
          }

          case dtComplex34Matrix: {
            complex34Matrix_t    xc;
            convertComplex34MatrixRegisterToComplex34Matrix(REGISTER_X, &xc);

            if(xc.header.matrixColumns == 2) {
              for(int i = 0; i < xc.header.matrixRows; ++i) {
                reallocateRegister(TEMP_REGISTER_1, dtComplex34, 0, amNone);
                complex34Copy(&xc.matrixElements[i*2], REGISTER_COMPLEX34_DATA(TEMP_REGISTER_1));
                printReg(TEMP_REGISTER_1, NULL, false, LINE_LEFT, false );    // Print row i col 1 on the left half of the line
                complex34Copy(&xc.matrixElements[i*2+1], REGISTER_COMPLEX34_DATA(TEMP_REGISTER_1));
                printReg(TEMP_REGISTER_1, NULL, false, LINE_RIGHT, false );   // Print row i col 2 on the right half of the line
              }
            }
            else if(xc.header.matrixRows == 2) {
              for(int i = 0; i < xc.header.matrixColumns; ++i) {
                reallocateRegister(TEMP_REGISTER_1, dtComplex34, 0, amNone);
                complex34Copy(&xc.matrixElements[i], REGISTER_COMPLEX34_DATA(TEMP_REGISTER_1));
                printReg(TEMP_REGISTER_1, NULL, false, LINE_LEFT, false );   // Print row 1 col i on the left half of the line
                complex34Copy(&xc.matrixElements[i + xc.header.matrixColumns], REGISTER_COMPLEX34_DATA(TEMP_REGISTER_1));
                printReg(TEMP_REGISTER_1, NULL, false, LINE_RIGHT, false );    // Print row 2 col i on the right half of the line
              }
            }
            else {
              displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE, REGISTER_X);
              #if (EXTRA_INFO_ON_CALC_ERROR == 1)
                sprintf(errorMessage, "cannot print xy when matrix size is %d x %d", xc.header.matrixRows, xc.header.matrixColumns);
                moreInfoOnError("In function fnP_All_Regs(PRN_XYr):", errorMessage, NULL, NULL);
              #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
            }
            return;
          }

          default: {
            displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
            #if (EXTRA_INFO_ON_CALC_ERROR == 1)
              sprintf(errorMessage, "invalid data type %s for register X", getRegisterDataTypeName(REGISTER_X, true, false));
              moreInfoOnError("In function fnP_All_Regs(PRN_XYr):", errorMessage, NULL, NULL);
            #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
            return;
          }
        }
        break;

      default: ;
    }
  #endif //IR_PRINTING
  }
  else {                             // Print to file
    if(calcMode != CM_NORMAL && calcMode != CM_NO_UNDO) {
      #if defined(DMCP_BUILD)
        beep(440, 50);
        beep(4400, 50);
        beep(440, 50);
      #endif // DMCP_BUILD
      return;
    }

    create_filename(".REGS.TSV");

    #if (VERBOSE_LEVEL >= 1)
      clearScreen(4);
      print_linestr("Output regs to drive:", true);
      print_linestr(filename_csv, false);
    #endif // VERBOSE_LEVEL >= 1

    uint16_t s, n;
    switch(option) {
      case PRN_ALL:
        stackregister_csv_out(REGISTER_X, REGISTER_W, !ONELINE);
        stackregister_csv_out(0, 99, !ONELINE);
        if(currentNumberOfLocalRegisters > 0) {
          stackregister_csv_out(FIRST_LOCAL_REGISTER, FIRST_LOCAL_REGISTER + currentNumberOfLocalRegisters - 1, !ONELINE);
        }
        if(numberOfNamedVariables > 0) {
          stackregister_csv_out(FIRST_NAMED_VARIABLE, FIRST_NAMED_VARIABLE + numberOfNamedVariables - 1, !ONELINE);
        }
        break;

      case PRN_REGS:
        if((lastErrorCode = getRegParam(NULL, &s, &n, NULL)) == ERROR_NONE) {
          stackregister_csv_out(s, (s + n) -1 , !ONELINE);
        }
        else {
          displayCalcErrorMessage(lastErrorCode, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
        }
        break;

      case PRN_STK:
        if(getSystemFlag(FLAG_SSIZE8)) {
          stackregister_csv_out(REGISTER_X, REGISTER_D, !ONELINE);
        }
        else {
          stackregister_csv_out(REGISTER_X, REGISTER_T, !ONELINE);
        }
        break;

      case PRN_GLOBALr:
        stackregister_csv_out(0, 99, !ONELINE);
        break;

      case PRN_LOCALr:
        if(currentNumberOfLocalRegisters > 0) {
          stackregister_csv_out(FIRST_LOCAL_REGISTER, FIRST_LOCAL_REGISTER + currentNumberOfLocalRegisters - 1, !ONELINE);
        }
        break;

      case PRN_NAMEDr:
        if(numberOfNamedVariables > 0) {
          stackregister_csv_out(FIRST_NAMED_VARIABLE, FIRST_NAMED_VARIABLE + numberOfNamedVariables - 1, !ONELINE);
        }
        break;

      case PRN_Xr:
        stackregister_csv_out(REGISTER_X, REGISTER_X, !ONELINE);
        break;

      case PRN_TMP:
        stackregister_csv_out(TEMP_REGISTER_1, TEMP_REGISTER_1, !ONELINE);
        break;

      case PRN_XYr:
        stackregister_csv_out(REGISTER_X, REGISTER_Y, ONELINE);
        break;

      default: ;
    }
  }
}


//
//  Print all items (test function)
//
void fnP_PrintAllItems (uint16_t unusedButMandatoryParameter) {
  #if defined(PC_BUILD) && defined(IR_PRINTING)
    int32_t item;
    currentKeyCode = 255;
    if(getSystemFlag(FLAG_PRTACT)) {
      sprintf(tmpString, "item catname  menuname");
      printLine(tmpString, 1);
      for(item=1; item<LAST_ITEM; item++) {
        sprintf(tmpString, "%4d ", item);
        strcat(tmpString, indexOfItems[item].itemCatalogName);
        printLine(tmpString, 0);
        printTab(97);
        sprintf(tmpString, "%s ", indexOfItems[item].itemSoftmenuName);
        printLine(tmpString, 1);
        if(_exitKeyPressed()) {
          break;
        }
      }
      temporaryInformation = TI_PRINT_COMPLETE;
    }
  #endif //PC_BUILD && IR_PRINTING
}

