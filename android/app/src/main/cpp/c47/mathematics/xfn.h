// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#if !defined(XFN_H)
#define XFN_H

bool_t  exitKeyWaiting(void);
#define DISPLAY_WAIT_FOR_RELEASE true
int     C47PopKeyNoBuffer(bool_t displayWaitForRelease);


void C47Cvt2RadSinCosTan2(real1071_t *an, angularMode_t angularMode, real1071_t *sinOut, real1071_t *cosOut, real1071_t *tanOut, realContext_t *realContext, int acc);
void C47radSinCosTanTaylor(real1071_t *an, bool_t swapTemp, real1071_t *sinOut, real1071_t *cosOut, real1071_t *tanOut, realContext_t *realContext, int accNumberDigits);

bool_t registerFMAOutputString(calcRegister_t regist, char* prefix, char *displayString);
void fnXXfn         (uint16_t function);
void fnXXfn_ToDEG   (uint16_t registerNo);
void fnXXfn_ToRAD   (uint16_t registerNo);
void fnXXfn_sin     (uint16_t registerNo);
void fnXXfn_cos     (uint16_t registerNo);
void fnXXfn_tan     (uint16_t registerNo);
void fnXXfn_pi      (uint16_t registerNo);
void fnXXfn_atan2   (uint16_t registerNo);
void fnXXfn_arcsin  (uint16_t registerNo);
void fnXXfn_arccos  (uint16_t registerNo);
void fnXXfn_arctan  (uint16_t registerNo);
void fnXXfn_LN      (uint16_t registerNo);
void fnXXfn_LOG     (uint16_t registerNo);
void fnXXfn_EXP     (uint16_t registerNo);
void fnXXfn_10X     (uint16_t registerNo);
void fnXXfn_POWER   (uint16_t registerNo);
void fnXXfn_SQRT    (uint16_t registerNo);
void fnXXfn_1ONX    (uint16_t registerNo);
void fnXXfn_ADD     (uint16_t registerNo);
void fnXXfn_SUB     (uint16_t registerNo);
void fnXXfn_MULT    (uint16_t registerNo);
void fnXXfn_DIV     (uint16_t registerNo);
void fnXXfn_MOD     (uint16_t registerNo);
void fnXXfn_MODANG  (uint16_t registerNo);
void fnXXfn_TO      (uint16_t registerNo);
void fnXXfn_DRG     (uint16_t registerNo);
void fnXXfn_SQR     (uint16_t registerNo);
void fnXXfn_YRTX    (uint16_t registerNo);
void fnXXfn_RDP     (uint16_t digits);
void fnXXfn_RSD     (uint16_t digits);
void fnXXfn_CHS     (uint16_t registerNo);

#endif // !XFN_H
