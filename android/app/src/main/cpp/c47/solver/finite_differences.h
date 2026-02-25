
// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"

#define MAX_ORDER   7
#define MAX_F_EVAL  (2 * MAX_ORDER + 1)

typedef struct {
    const char *const desc;
    uint8_t n;      // Number of steps from the x, each step is by h in either direction
    uint8_t order;  // Order of the derivative (1 or 2 only at present)
    uint8_t denom; // Denominator of the finite different equation
    uint8_t coeff[/*2*n+1*/]; // Coefficients for each function evaluation point
} FINITE_DIFF_COEFF;


TO_QSPI static const FINITE_DIFF_COEFF d1_3_central = {
  "d1_3_central", 1, 1, 1, { 2, 0, 3 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_2_lower_middle = {
  "d1_2_lower_middle", 1, 1, 3, { 2, 3, 0 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_2_upper_middle = {
  "d1_2_upper_middle", 1, 1, 3, { 0, 2, 3 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_5_central = {
  "d1_5_central", 2, 1, 4, { 3, 5, 0, 6, 2 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_3_lower_middle = {
  "d1_3_lower_middle", 2, 1, 1, { 3, 7, 8, 0, 0 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_3_upper_middle = {
  "d1_3_upper_middle", 2, 1, 1, { 0, 0, 9, 10, 2 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_2_lower = {
  "d1_2_lower", 2, 1, 3, { 2, 3, 0, 0, 0 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_2_upper = {
  "d1_2_upper", 2, 1, 3, { 0, 0, 0, 2, 3 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_7_central = {
  "d1_7_central", 3, 1, 11, { 2, 12, 13, 0, 14, 15, 3 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_4_lower_middle = {
  "d1_4_lower_middle", 3, 1, 16, { 17, 12, 18, 19, 0, 0, 0 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_4_upper_middle = {
  "d1_4_upper_middle", 3, 1, 16, { 0, 0, 0, 20, 21, 15, 1 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_3_lower = {
  "d1_3_lower", 3, 1, 1, { 8, 5, 22, 0, 0, 0, 0 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_3_upper = {
  "d1_3_upper", 3, 1, 1, { 0, 0, 0, 0, 23, 6, 9 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_9_central = {
  "d1_9_central", 4, 1, 24, { 8, 25, 26, 27, 0, 28, 29, 30, 9 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_5_lower_middle = {
  "d1_5_lower_middle", 4, 1, 4, { 8, 31, 32, 33, 34, 0, 0, 0, 0 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_5_upper_middle = {
  "d1_5_upper_middle", 4, 1, 4, { 0, 0, 0, 0, 35, 36, 37, 38, 9 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_4_lower = {
  "d1_4_lower", 4, 1, 16, { 20, 39, 40, 41, 0, 0, 0, 0, 0 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_4_upper = {
  "d1_4_upper", 4, 1, 16, { 0, 0, 0, 0, 0, 42, 43, 44, 19 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_11_central = {
  "d1_11_central", 5, 1, 45, { 17, 34, 46, 47, 48, 0, 49, 50, 51, 35, 1 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_6_lower_middle = {
  "d1_6_lower_middle", 5, 1, 11, { 52, 53, 54, 55, 56, 57, 0, 0, 0, 0, 0 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_6_upper_middle = {
  "d1_6_upper_middle", 5, 1, 11, { 0, 0, 0, 0, 0, 58, 55, 56, 59, 60, 4 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_5_lower = {
  "d1_5_lower", 5, 1, 4, { 34, 61, 62, 63, 64, 0, 0, 0, 0, 0, 0 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_5_upper = {
  "d1_5_upper", 5, 1, 4, { 0, 0, 0, 0, 0, 0, 65, 66, 67, 68, 35 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_13_central = {
  "d1_13_central", 6, 1, 69, { 22, 70, 71, 72, 73, 74, 0, 75, 76, 77, 78, 79, 23 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_7_lower_middle = {
  "d1_7_lower_middle", 6, 1, 11, { 80, 70, 81, 82, 83, 84, 85, 0, 0, 0, 0, 0, 0 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_7_upper_middle = {
  "d1_7_upper_middle", 6, 1, 11, { 0, 0, 0, 0, 0, 0, 86, 87, 88, 89, 90, 79, 91 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_6_lower = {
  "d1_6_lower", 6, 1, 11, { 58, 92, 93, 94, 95, 96, 0, 0, 0, 0, 0, 0, 0 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_6_upper = {
  "d1_6_upper", 6, 1, 11, { 0, 0, 0, 0, 0, 0, 0, 97, 98, 99, 100, 101, 57 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_15_central = {
  "d1_15_central", 7, 1, 102, { 103, 104, 105, 106, 107, 108, 109, 0, 110, 111, 112, 113, 114, 115, 116 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_8_lower_middle = {
  "d1_8_lower_middle", 7, 1, 117, { 118, 119, 120, 121, 122, 123, 124, 125, 0, 0, 0, 0, 0, 0, 0 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_8_upper_middle = {
  "d1_8_upper_middle", 7, 1, 117, { 0, 0, 0, 0, 0, 0, 0, 126, 127, 128, 129, 130, 131, 132, 11 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_7_lower = {
  "d1_7_lower", 7, 1, 11, { 85, 133, 134, 135, 136, 137, 138, 0, 0, 0, 0, 0, 0, 0, 0 }
};

TO_QSPI static const FINITE_DIFF_COEFF d1_7_upper = {
  "d1_7_upper", 7, 1, 11, { 0, 0, 0, 0, 0, 0, 0, 0, 139, 140, 141, 142, 143, 144, 86 }
};

TO_QSPI static const FINITE_DIFF_COEFF d2_3_central = {
  "d2_3_central", 1, 2, 3, { 3, 17, 3 }
};

TO_QSPI static const FINITE_DIFF_COEFF d2_5_central = {
  "d2_5_central", 2, 2, 4, { 2, 38, 145, 38, 2 }
};

TO_QSPI static const FINITE_DIFF_COEFF d2_3_lower_middle = {
  "d2_3_lower_middle", 2, 2, 3, { 3, 17, 3, 0, 0 }
};

TO_QSPI static const FINITE_DIFF_COEFF d2_3_upper_middle = {
  "d2_3_upper_middle", 2, 2, 3, { 0, 0, 3, 17, 3 }
};

TO_QSPI static const FINITE_DIFF_COEFF d2_7_central = {
  "d2_7_central", 3, 2, 146, { 1, 147, 148, 132, 148, 147, 1 }
};

TO_QSPI static const FINITE_DIFF_COEFF d2_4_lower_middle = {
  "d2_4_lower_middle", 3, 2, 3, { 2, 10, 23, 1, 0, 0, 0 }
};

TO_QSPI static const FINITE_DIFF_COEFF d2_4_upper_middle = {
  "d2_4_upper_middle", 3, 2, 3, { 0, 0, 0, 1, 23, 10, 2 }
};

TO_QSPI static const FINITE_DIFF_COEFF d2_3_lower = {
  "d2_3_lower", 3, 2, 3, { 3, 17, 3, 0, 0, 0, 0 }
};

TO_QSPI static const FINITE_DIFF_COEFF d2_3_upper = {
  "d2_3_upper", 3, 2, 3, { 0, 0, 0, 0, 3, 17, 3 }
};

TO_QSPI static const FINITE_DIFF_COEFF d2_9_central = {
  "d2_9_central", 4, 2, 149, { 15, 150, 151, 152, 153, 152, 151, 150, 15 }
};

TO_QSPI static const FINITE_DIFF_COEFF d2_5_lower_middle = {
  "d2_5_lower_middle", 4, 2, 4, { 19, 154, 155, 156, 157, 0, 0, 0, 0 }
};

TO_QSPI static const FINITE_DIFF_COEFF d2_5_upper_middle = {
  "d2_5_upper_middle", 4, 2, 4, { 0, 0, 0, 0, 157, 156, 155, 154, 19 }
};

TO_QSPI static const FINITE_DIFF_COEFF d2_4_lower = {
  "d2_4_lower", 4, 2, 3, { 17, 158, 5, 8, 0, 0, 0, 0, 0 }
};

TO_QSPI static const FINITE_DIFF_COEFF d2_4_upper = {
  "d2_4_upper", 4, 2, 3, { 0, 0, 0, 0, 0, 8, 5, 158, 17 }
};

TO_QSPI static const FINITE_DIFF_COEFF d2_11_central = {
  "d2_11_central", 5, 2, 159, { 6, 160, 161, 162, 163, 164, 163, 162, 161, 160, 6 }
};

TO_QSPI static const FINITE_DIFF_COEFF d2_6_lower_middle = {
  "d2_6_lower_middle", 5, 2, 4, { 91, 165, 166, 66, 167, 14, 0, 0, 0, 0, 0 }
};

TO_QSPI static const FINITE_DIFF_COEFF d2_6_upper_middle = {
  "d2_6_upper_middle", 5, 2, 4, { 0, 0, 0, 0, 0, 14, 167, 66, 166, 165, 91 }
};

TO_QSPI static const FINITE_DIFF_COEFF d2_5_lower = {
  "d2_5_lower", 5, 2, 4, { 157, 168, 169, 170, 171, 0, 0, 0, 0, 0, 0 }
};

TO_QSPI static const FINITE_DIFF_COEFF d2_5_upper = {
  "d2_5_upper", 5, 2, 4, { 0, 0, 0, 0, 0, 0, 171, 170, 169, 168, 157 }
};

TO_QSPI static const FINITE_DIFF_COEFF d2_13_central = {
  "d2_13_central", 6, 2, 172, { 173, 174, 76, 175, 176, 177, 178, 177, 176, 175, 76, 174, 173 }
};

TO_QSPI static const FINITE_DIFF_COEFF d2_7_lower_middle = {
  "d2_7_lower_middle", 6, 2, 146, { 57, 179, 180, 181, 182, 183, 184, 0, 0, 0, 0, 0, 0 }
};

TO_QSPI static const FINITE_DIFF_COEFF d2_7_upper_middle = {
  "d2_7_upper_middle", 6, 2, 146, { 0, 0, 0, 0, 0, 0, 184, 183, 182, 181, 180, 179, 57 }
};

TO_QSPI static const FINITE_DIFF_COEFF d2_6_lower = {
  "d2_6_lower", 6, 2, 4, { 13, 185, 186, 187, 188, 189, 0, 0, 0, 0, 0, 0, 0 }
};

TO_QSPI static const FINITE_DIFF_COEFF d2_6_upper = {
  "d2_6_upper", 6, 2, 4, { 0, 0, 0, 0, 0, 0, 0, 189, 188, 187, 186, 185, 13 }
};

TO_QSPI static const FINITE_DIFF_COEFF d2_15_central = {
  "d2_15_central", 7, 2, 190, { 191, 192, 193, 194, 195, 196, 197, 198, 197, 196, 195, 194, 193, 192, 191 }
};

TO_QSPI static const FINITE_DIFF_COEFF d2_8_lower_middle = {
  "d2_8_lower_middle", 7, 2, 146, { 199, 144, 200, 201, 202, 203, 204, 205, 0, 0, 0, 0, 0, 0, 0 }
};

TO_QSPI static const FINITE_DIFF_COEFF d2_8_upper_middle = {
  "d2_8_upper_middle", 7, 2, 146, { 0, 0, 0, 0, 0, 0, 0, 205, 204, 203, 202, 201, 200, 144, 199 }
};

TO_QSPI static const FINITE_DIFF_COEFF d2_7_lower = {
  "d2_7_lower", 7, 2, 146, { 184, 206, 207, 208, 209, 210, 211, 0, 0, 0, 0, 0, 0, 0, 0 }
};

TO_QSPI static const FINITE_DIFF_COEFF d2_7_upper = {
  "d2_7_upper", 7, 2, 146, { 0, 0, 0, 0, 0, 0, 0, 0, 211, 210, 209, 208, 207, 206, 184 }
};

TO_QSPI static const FINITE_DIFF_COEFF *const d1_derivatives[] = {
  &d1_15_central, &d1_13_central, &d1_11_central, &d1_9_central, 
  &d1_8_lower_middle, &d1_8_upper_middle, &d1_7_central, &d1_7_lower_middle, 
  &d1_7_upper_middle, &d1_7_lower, &d1_7_upper, &d1_6_lower_middle, 
  &d1_6_upper_middle, &d1_6_lower, &d1_6_upper, &d1_5_central, 
  &d1_5_lower_middle, &d1_5_upper_middle, &d1_5_lower, &d1_5_upper, 
  &d1_4_lower_middle, &d1_4_upper_middle, &d1_4_lower, &d1_4_upper, 
  &d1_3_central, &d1_3_lower_middle, &d1_3_upper_middle, &d1_3_lower, 
  &d1_3_upper, &d1_2_lower_middle, &d1_2_upper_middle, &d1_2_lower, 
  &d1_2_upper, NULL
};

TO_QSPI static const FINITE_DIFF_COEFF *const d2_derivatives[] = {
  &d2_15_central, &d2_13_central, &d2_11_central, &d2_9_central, 
  &d2_8_lower_middle, &d2_8_upper_middle, &d2_7_central, &d2_7_lower_middle, 
  &d2_7_upper_middle, &d2_7_lower, &d2_7_upper, &d2_6_lower_middle, 
  &d2_6_upper_middle, &d2_6_lower, &d2_6_upper, &d2_5_central, 
  &d2_5_lower_middle, &d2_5_upper_middle, &d2_5_lower, &d2_5_upper, 
  &d2_4_lower_middle, &d2_4_upper_middle, &d2_4_lower, &d2_4_upper, 
  &d2_3_central, &d2_3_lower_middle, &d2_3_upper_middle, &d2_3_lower, 
  &d2_3_upper, NULL
};

TO_QSPI static const FINITE_DIFF_COEFF *const *const finite_difference_table[] = {
  d1_derivatives, 
  d2_derivatives, 
};

TO_QSPI static const int32_t fdValues[212] = {
         0,         2,        -1,         1,        12,        -8,         8,        -4,
         3,        -3,         4,        60,         9,       -45,        45,        -9,
         6,        -2,       -18,        11,       -11,        18,         5,        -5,
       840,       -32,       168,      -672,       672,      -168,        32,       -16,
        36,       -48,        25,       -25,        48,       -36,        16,        42,
       -57,        26,       -26,        57,       -42,      2520,      -150,       600,
     -2100,      2100,      -600,       150,       -12,        75,      -200,       300,
      -300,       137,      -137,       200,       -75,      -122,       234,      -214,
        77,       -77,       214,      -234,       122,     27720,       -72,       495,
     -2200,      7425,    -23760,     23760,     -7425,      2200,      -495,        72,
        10,       225,      -400,       450,      -360,       147,      -147,       360,
      -450,       400,      -225,       -10,       810,     -1980,      2540,     -1755,
       522,      -522,      1755,     -2540,      1980,      -810,    360360,       -15,
       245,     -1911,      9555,    -35035,    105105,   -315315,    315315,   -105105,
     35035,     -9555,      1911,      -245,        15,       420,       -60,       490,
     -1764,      3675,     -4900,      4410,     -2940,      1089,     -1089,      2940,
     -4410,      4900,     -3675,      1764,      -490,     -1019,      3015,     -4920,
      4745,     -2637,       669,      -669,      2637,     -4745,      4920,     -3015,
      1019,       -30,       180,       -27,       270,      5040,       128,     -1008,
      8064,    -14350,       -56,       114,      -104,        35,         7,     25200,
      -125,      1000,     -6000,     42000,    -73766,        61,      -156,      -154,
      -164,       294,      -236,        71,    831600,       -50,       864,     44000,
   -222750,   1425600,  -2480478,      -972,      2970,     -5080,      5265,     -3132,
       812,       260,      -614,       744,      -461,       116,  75675600,       900,
    -17150,    160524,  -1003275,   4904900, -22072050, 132432300,-228812298,      -126,
     -3618,      7380,     -9490,      7911,     -4014,       938,     -5547,     16080,
    -25450,     23340,    -11787,      2552,
};

