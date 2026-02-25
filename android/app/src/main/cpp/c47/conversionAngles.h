// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file conversionAngles.h
 ***********************************************/
#if !defined(CONVERSIONANGLES_H)
#define CONVERSIONANGLES_H

void     fnCvtToCurrentAngularMode   (uint16_t fromAngularMode);
void     fnCvtFromCurrentAngularModeRegister(calcRegister_t regist1, uint16_t toAngularMode); //JM
void     fnCvtFromCurrentAngularMode (uint16_t toAngularMode);
/*
void     fnCvtDmsToCurrentAngularMode(uint16_t unusedButMandatoryParameter);
void     fnCvtDegToRad               (uint16_t unusedButMandatoryParameter);
void     fnCvtDegToDms               (uint16_t unusedButMandatoryParameter);
void     fnCvtDmsToDeg               (uint16_t unusedButMandatoryParameter);
void     fnCvtRadToDeg               (uint16_t unusedButMandatoryParameter);
void     fnCvtRadToMultPi            (uint16_t unusedButMandatoryParameter);
void     fnCvtMultPiToRad            (uint16_t unusedButMandatoryParameter);
*/
void     convertAngle34FromTo        (real34_t *angle34, angularMode_t fromAngularMode, angularMode_t toAngularMode);
void     convertAngleFromTo          (real_t *angle, angularMode_t fromAngularMode, angularMode_t toAngularMode, realContext_t *realContext);
void     checkDms34                  (real34_t *angle34Dms);
uint32_t getInfiniteComplexAngle     (real_t *x, real_t *y);
void     setInfiniteComplexAngle     (uint32_t angle, real_t *x, real_t *y);
void     real34FromDmsToDeg          (const real34_t *angleDms, real34_t *angleDec);
void     real34FromDegToDms          (const real34_t *angleDec, real34_t *angleDms);
#endif // !CONVERSIONANGLES_H
