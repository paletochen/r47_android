// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file comparisonReals.h
 ***********************************************/
#if !defined(COMPARISONREALS_H)
#define COMPARISONREALS_H

// Return a convergence tolerance based on the current display setting
void convergenceTolerence(real_t *tol);
void fractionTolerence(real_t *tol);
void irfractionTolerence(int32_t ii, real_t *tol);
int32_t realGetExponentComp(const real_t *val);

bool_t  real34CompareAbsGreaterThan (const real34_t *number1, const real34_t *number2);
bool_t  real34CompareAbsGreaterEqual(const real34_t *number1, const real34_t *number2);
bool_t  real34CompareAbsLessThan    (const real34_t *number1, const real34_t *number2);
bool_t  real34CompareEqual          (const real34_t *number1, const real34_t *number2);
bool_t  real34CompareAbsEqual       (const real34_t *number1, const real34_t *number2);
bool_t  real34CompareGreaterEqual   (const real34_t *number1, const real34_t *number2);
bool_t  real34CompareGreaterThan    (const real34_t *number1, const real34_t *number2); //never used: JM returned
bool_t  real34CompareLessEqual      (const real34_t *number1, const real34_t *number2);
bool_t  real34CompareLessThan       (const real34_t *number1, const real34_t *number2);

bool_t  realCompareAbsGreaterThan   (const real_t *number1, const real_t *number2);
//bool_t  realCompareAbsGreaterEqual  (const real_t *number1, const real_t *number2); never used
bool_t  realCompareAbsLessThan      (const real_t *number1, const real_t *number2);
bool_t  realCompareEqual            (const real_t *number1, const real_t *number2);
bool_t  realCompareAbsEqual         (const real_t *number1, const real_t *number2);
bool_t  realCompareGreaterEqual     (const real_t *number1, const real_t *number2);
bool_t  realCompareGreaterThan      (const real_t *number1, const real_t *number2);
bool_t  realCompareLessEqual        (const real_t *number1, const real_t *number2);
bool_t  realCompareLessThan         (const real_t *number1, const real_t *number2);

bool_t  real34IsAnInteger           (const real34_t *x);
bool_t  realIsAnInteger             (const real_t *x);
//int16_t realIdenticalDigits         (real_t *a, real_t *b);
//bool_t  realCompareRoundedEqualConstant(const real_t *number, const real_t *constant, int32_t nn);

#endif // !COMPARISONREALS_H
