#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include "c47.h"
#include "fonts.h"

// Helper to check if string matches a glyph and move pointer
static bool match_glyph(const char** input, const char* glyph) {
    if (!glyph || glyph[0] == '\0') return false;
    size_t len = strlen(glyph);
    if (strncmp(*input, glyph, len) == 0) {
        *input += len;
        return true;
    }
    return false;
}

void sanitize_register_string(const char* input, char* output, size_t max_len) {
    const char* in_ptr = input;
    size_t out_idx = 0;
    size_t max_out = (max_len > 0) ? max_len - 1 : 0;
    bool last_was_space = true; 

    // We process the buffer in two potential segments to handle the dual-buffer complex format
    // Real part starts at 0, Imaginary part starts at 100.
    const int segments[] = { 0, 100 };
    
    for (int s = 0; s < 2; s++) {
        in_ptr = input + segments[s];
        
        // If this is the imaginary part, add a separator if we already have data
        if (s == 1 && out_idx > 0 && out_idx < max_out) {
            // Check if the real part was zero. If core sets real to "" for pure imag, out_idx might be 0.
            // But if we have data, we need a separator.
            if (output[out_idx-1] != ' ') {
                output[out_idx++] = ' ';
                last_was_space = true;
            }
        }

        while (*in_ptr != '\0' && out_idx < max_out) {
            // 1. Multi-byte sequences starting with \x80
            if ((unsigned char)in_ptr[0] == 0x80) {
                // Degree Symbol: \x80\xb0
                if ((unsigned char)in_ptr[1] == 0xB0) {
                    if (out_idx + 2 <= max_out) {
                        output[out_idx++] = (char)0xC2; // UTF-8 Degree
                        output[out_idx++] = (char)0xB0;
                    }
                    in_ptr += 2; last_was_space = false; continue;
                }
                // Exponent Multiplier (Dot+10 or Cross+10)
                if (strncmp(in_ptr, "\x80\xb7\xa4\x7d", 4) == 0 || strncmp(in_ptr, "\x80\xd7\xa4\x7d", 4) == 0) {
                    output[out_idx++] = 'e';
                    in_ptr += 4; last_was_space = false; continue;
                }
                // Plus/Minus: \x80\xb1
                if ((unsigned char)in_ptr[1] == 0xB1) {
                    if (out_idx + 3 <= max_out) {
                        output[out_idx++] = '+'; output[out_idx++] = '/'; output[out_idx++] = '-';
                    }
                    in_ptr += 2; last_was_space = false; continue;
                }
                // Dot: \x80\xb7
                if ((unsigned char)in_ptr[1] == 0xB7) {
                    output[out_idx++] = '.';
                    in_ptr += 2; last_was_space = false; continue;
                }
                // Almost Equal: \x80\xa1 -> skip or map to ~
                if ((unsigned char)in_ptr[1] == 0xA1) {
                    in_ptr += 2; continue;
                }
            }

            // 2. Multi-byte sequences starting with \x82 (Bold variants)
            if ((unsigned char)in_ptr[0] == 0x82) {
                // Bold 'r': \x82\xb3 -> SKIP
                if ((unsigned char)in_ptr[1] == 0xB3) {
                    in_ptr += 2; continue;
                }
                // Bold 'f': \x82\x96 -> SKIP
                if ((unsigned char)in_ptr[1] == 0x96) {
                    in_ptr += 2; continue;
                }
            }

            // 3. Multi-byte sequences starting with \xa4 or others (Mode Indicators)
            if (match_glyph(&in_ptr, STD_SUP_r) || match_glyph(&in_ptr, STD_SUP_R) ||
                match_glyph(&in_ptr, STD_SUP_g) || match_glyph(&in_ptr, STD_SUP_G) ||
                match_glyph(&in_ptr, STD_SUP_d) || match_glyph(&in_ptr, STD_SUP_D) ||
                match_glyph(&in_ptr, STD_MODE_F) || match_glyph(&in_ptr, STD_SUP_BOLD_f) ||
                match_glyph(&in_ptr, STD_SUP_BOLD_T) || match_glyph(&in_ptr, STD_MEASURED_ANGLE)) {
                continue;
            }

            // Subscripts/Superscripts -> ASCII
            if (match_glyph(&in_ptr, STD_SUP_0) || match_glyph(&in_ptr, STD_SUB_0)) { output[out_idx++] = '0'; last_was_space = false; continue; }
            if (match_glyph(&in_ptr, STD_SUP_1) || match_glyph(&in_ptr, STD_SUB_1)) { output[out_idx++] = '1'; last_was_space = false; continue; }
            if (match_glyph(&in_ptr, STD_SUP_2) || match_glyph(&in_ptr, STD_SUB_2)) { output[out_idx++] = '2'; last_was_space = false; continue; }
            if (match_glyph(&in_ptr, STD_SUP_3) || match_glyph(&in_ptr, STD_SUB_3)) { output[out_idx++] = '3'; last_was_space = false; continue; }
            if (match_glyph(&in_ptr, STD_SUP_4) || match_glyph(&in_ptr, STD_SUB_4)) { output[out_idx++] = '4'; last_was_space = false; continue; }
            if (match_glyph(&in_ptr, STD_SUP_5) || match_glyph(&in_ptr, STD_SUB_5)) { output[out_idx++] = '5'; last_was_space = false; continue; }
            if (match_glyph(&in_ptr, STD_SUP_6) || match_glyph(&in_ptr, STD_SUB_6)) { output[out_idx++] = '6'; last_was_space = false; continue; }
            if (match_glyph(&in_ptr, STD_SUP_7) || match_glyph(&in_ptr, STD_SUB_7)) { output[out_idx++] = '7'; last_was_space = false; continue; }
            if (match_glyph(&in_ptr, STD_SUP_8) || match_glyph(&in_ptr, STD_SUB_8)) { output[out_idx++] = '8'; last_was_space = false; continue; }
            if (match_glyph(&in_ptr, STD_SUP_9) || match_glyph(&in_ptr, STD_SUB_9)) { output[out_idx++] = '9'; last_was_space = false; continue; }
            if (match_glyph(&in_ptr, STD_SUP_PLUS)) { output[out_idx++] = '+'; last_was_space = false; continue; }
            if (match_glyph(&in_ptr, STD_SUP_MINUS)) { output[out_idx++] = '-'; last_was_space = false; continue; }

            // Constants
            if (match_glyph(&in_ptr, STD_pi) || match_glyph(&in_ptr, STD_PI) || 
                match_glyph(&in_ptr, STD_SUP_pi) || match_glyph(&in_ptr, STD_SUB_pi)) {
                if (out_idx + 2 <= max_out) {
                    output[out_idx++] = (char)0xCF; output[out_idx++] = (char)0x80;
                }
                last_was_space = false; continue;
            }

            // 4. Special Fallbacks
            unsigned char c = (unsigned char)*in_ptr;

            // Degree Symbol (single byte \xb0 or \xa2\x18)
            if (c == 0xB0 || match_glyph(&in_ptr, STD_RING)) {
                if (out_idx + 2 <= max_out) {
                    output[out_idx++] = (char)0xC2; output[out_idx++] = (char)0xB0;
                }
                if (c == 0xB0) in_ptr++;
                last_was_space = false; continue;
            }

            if (match_glyph(&in_ptr, STD_COMMA34)) { output[out_idx++] = '.'; last_was_space = false; continue; }

            // Whitespace & Control: Collapse
            if (c <= 0x20 || match_glyph(&in_ptr, STD_SPACE_HAIR) || match_glyph(&in_ptr, STD_SPACE_4_PER_EM)) {
                if (!last_was_space && out_idx < max_out) {
                    output[out_idx++] = ' ';
                    last_was_space = true;
                }
                if (c <= 0x20) in_ptr++;
                continue;
            }

            // Skip non-printable control characters
            if (c == 0x01) { in_ptr++; continue; }

            // 5. ASCII & Trailing Mode Stripping
            if (c < 0x80) {
                // Strip literal annunciators 'r', 'g', 'd' if they appear at the end of a numeric segment
                // Check if followed by any alphanumeric chars in the REST of this segment
                bool followed_by_data = false;
                const char* check = in_ptr + 1;
                while (*check) {
                    if (isalnum((unsigned char)*check)) { followed_by_data = true; break; }
                    check++;
                }

                if (!followed_by_data && (c == 'r' || c == 'g' || c == 'd')) {
                    in_ptr++; continue;
                }

                output[out_idx++] = *in_ptr++;
                last_was_space = false; continue;
            }

            // Unhandled High-Bit: SKIP
            in_ptr++;
        }
    }

    // Final Trim
    while (out_idx > 0 && output[out_idx-1] == ' ') out_idx--;
    output[out_idx] = '\0';
}