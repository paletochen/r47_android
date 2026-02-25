#include "c47.h"
#include <string.h>
#include <stdio.h>
#include <android/log.h>
#include "display.h"
#include "fonts.h"

#define LOG_TAG "R47Helpers"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static void ascii_clean(char *str) {
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
        } else if (c1 == 0xA0 && c2 == 0x0A) { // STD_SPACE_HAIR
            *d++ = ' '; s += 2;
        } else {
            *d++ = *s++;
        }
    }
    *d = '\0';
    strcpy(str, tmp);
}

static void trimTrailingRadix(char *str) {
    if (!str || !*str) return;
    size_t len = strlen(str);
    while (len > 0 && (str[len-1] == ' ' || str[len-1] == '.' || str[len-1] == ',')) {
        str[--len] = '\0';
    }
}

char* getXRegisterString() {
    static char result[2048]; // Increased size for UTF-8 conversion
    char coreBuf[1024];
    coreBuf[0] = '\0';
    
    if (!ram) return "0";

    calcRegister_t regist = REGISTER_X;
    uint8_t dataType = getRegisterDataType(regist);
    uint8_t tag = getRegisterTag(regist);
    
    switch(dataType) {
        case dtReal34:
            real34ToDisplayString((const real34_t *)getRegisterDataPointer(regist), tag, coreBuf, &standardFont, 9999, 34, false, false, NOIRFRAC);
            break;
            
        case dtComplex34:
            complex34ToDisplayString((const complex34_t *)getRegisterDataPointer(regist), coreBuf, &standardFont, 9999, 34, false, false, NOIRFRAC, tag & amAngleMask, (tag & amPolar) != 0);
            break;
            
        case dtString:
            strncpy(coreBuf, REGISTER_STRING_DATA(regist), 1023);
            coreBuf[1023] = '\0';
            break;
            
        case dtShortInteger:
            shortIntegerToDisplayString(regist, coreBuf, false, 0);
            break;
            
        case dtLongInteger:
            longIntegerRegisterToDisplayString(regist, coreBuf, sizeof(coreBuf), 9999, 50, true);
            break;
            
        default:
            sprintf(coreBuf, "[%s]", getDataTypeName(dataType, false, false));
            break;
    }
    
    ascii_clean(coreBuf);
    trimTrailingRadix(coreBuf);

    // Convert core glyphs to valid UTF-8
    extern void stringToUtf8(const char *str, uint8_t *utf8);
    stringToUtf8(coreBuf, (uint8_t*)result);

    LOGI("getXRegisterString: Result='%s'", result);
    return result;
}