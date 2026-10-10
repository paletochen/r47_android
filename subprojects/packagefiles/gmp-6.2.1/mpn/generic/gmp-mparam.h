// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

// Replaces the empty mpn/generic/gmp-mparam.h of the calculator build, which configures --disable-assembly and so takes the generic path. A C47 long integer is at most
// MAX_LONG_INTEGER_SIZE_IN_BITS, 3328 bits or 104 limbs, and GMP uses the algorithms named here only for a product of two factors of 100 limbs or more, a division whose
// divisor and quotient both reach 60 limbs, a modular power whose modulus reaches 100 limbs, or larger operands still. C47 uses GMP's modular power only in the primality
// tests behind PRIME?, NEXTP and FACTORS, which refuse an argument above 10^300. GMP takes a threshold of MP_SIZE_T_MAX as never: ABOVE_THRESHOLD folds it at compile
// time, so the branch and the object behind it leave the link. Toom-3, schoolbook division, Lehmer GCD and REDC_1 remain, and each is exact at every size. Every other
// threshold keeps its gmp-impl.h default.

#define MUL_TOOM44_THRESHOLD           MP_SIZE_T_MAX
#define MUL_TOOM6H_THRESHOLD           MP_SIZE_T_MAX
#define MUL_TOOM8H_THRESHOLD           MP_SIZE_T_MAX
#define MUL_TOOM32_TO_TOOM43_THRESHOLD MP_SIZE_T_MAX
#define MUL_TOOM32_TO_TOOM53_THRESHOLD MP_SIZE_T_MAX
#define MUL_TOOM42_TO_TOOM53_THRESHOLD MP_SIZE_T_MAX
#define MUL_TOOM42_TO_TOOM63_THRESHOLD MP_SIZE_T_MAX
#define MUL_TOOM43_TO_TOOM54_THRESHOLD MP_SIZE_T_MAX
#define SQR_TOOM4_THRESHOLD            MP_SIZE_T_MAX
#define SQR_TOOM6_THRESHOLD            MP_SIZE_T_MAX
#define SQR_TOOM8_THRESHOLD            MP_SIZE_T_MAX
#define MUL_FFT_MODF_THRESHOLD         MP_SIZE_T_MAX
#define MUL_FFT_THRESHOLD              MP_SIZE_T_MAX
#define SQR_FFT_MODF_THRESHOLD         MP_SIZE_T_MAX
#define SQR_FFT_THRESHOLD              MP_SIZE_T_MAX
#define MULLO_MUL_N_THRESHOLD          MP_SIZE_T_MAX
#define SQRLO_SQR_THRESHOLD            MP_SIZE_T_MAX
#define DC_DIV_QR_THRESHOLD            MP_SIZE_T_MAX
#define DC_DIVAPPR_Q_THRESHOLD         MP_SIZE_T_MAX
#define DC_BDIV_QR_THRESHOLD           MP_SIZE_T_MAX
#define DC_BDIV_Q_THRESHOLD            MP_SIZE_T_MAX
#define MU_DIV_QR_THRESHOLD            MP_SIZE_T_MAX
#define MU_DIVAPPR_Q_THRESHOLD         MP_SIZE_T_MAX
#define MUPI_DIV_QR_THRESHOLD          MP_SIZE_T_MAX
#define MU_BDIV_QR_THRESHOLD           MP_SIZE_T_MAX
#define MU_BDIV_Q_THRESHOLD            MP_SIZE_T_MAX
#define INV_NEWTON_THRESHOLD           MP_SIZE_T_MAX
#define INV_APPR_THRESHOLD             MP_SIZE_T_MAX
#define BINV_NEWTON_THRESHOLD          MP_SIZE_T_MAX
#define REDC_1_TO_REDC_N_THRESHOLD     MP_SIZE_T_MAX
#define REDC_2_TO_REDC_N_THRESHOLD     MP_SIZE_T_MAX
#define GCD_DC_THRESHOLD               MP_SIZE_T_MAX
#define GCDEXT_DC_THRESHOLD            MP_SIZE_T_MAX
#define HGCD_THRESHOLD                 MP_SIZE_T_MAX
#define HGCD_APPR_THRESHOLD            MP_SIZE_T_MAX
#define HGCD_REDUCE_THRESHOLD          MP_SIZE_T_MAX
#define SET_STR_DC_THRESHOLD           MP_SIZE_T_MAX
#define SET_STR_PRECOMPUTE_THRESHOLD   MP_SIZE_T_MAX
