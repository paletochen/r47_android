// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file addition.h
 ***********************************************/
#if !defined(ADDITION_H)
#define ADDITION_H

void fnAdd      (uint16_t unusedButMandatoryParameter);

// A pair of routines that is the same under addition and under subtraction shares one body, which takes the operation. The long integer bodies take the two
// conversions as well, so one body serves the long and short integer pairs in both orders.
typedef void (*longIntegerOp_t)         (longInteger_t opY, longInteger_t opX, longInteger_t result);
typedef void (*registerToLongInteger_t) (calcRegister_t regist, longInteger_t lgInt);
void addSubRemaReal(real34Op_t op, void (*realRealf)(void));
void addSubCxmaReal(real34Op_t op, void (*cplxRealf)(void));
void addSubRealRema(real34Op_t op, void (*realRealf)(void));
void addSubCxmaCplx(real34Op_t op);
void addSubCplxCxma(real34Op_t op);
void addSubCplxCplx(real34Op_t op);
void addSubCplxShoI(real34Op_t op);
void addSubDateLonI(real34Op_t op);
void addSubTimeLonI(real34Op_t op, calcRegister_t lonI);
void addSubIntInt(longIntegerOp_t op, registerToLongInteger_t cvtY, registerToLongInteger_t cvtX);
// The long integer and real34 pairs of add, subtract and multiply share two bodies, one per operand order, each taking the fast path and the real
// operation. Multiplication commutes, so its fast path has no use for the operand order.
typedef bool_t (*longIntegerReal34Op_t) (calcRegister_t lonI, const real34_t *r, bool_t integerFirst, real34_t *res);
void   lonIRealOp     (longIntegerReal34Op_t fast, realOp_t op);
void   realLonIOp     (longIntegerReal34Op_t fast, realOp_t op);
bool_t addFastLonIReal(calcRegister_t lonI, const real34_t *r, bool_t integerFirst, real34_t *res);
bool_t subFastLonIReal(calcRegister_t lonI, const real34_t *r, bool_t integerFirst, real34_t *res);
bool_t mulFastLonIReal(calcRegister_t lonI, const real34_t *r, bool_t unusedIntegerFirst, real34_t *res);
void addComplex(const cplx_t *a, const cplx_t *b, cplx_t *res, realContext_t *realContext);
#if defined(OPTION_CUBIC_159) || defined(OPTION_SQUARE_159) || defined(OPTION_EIGEN_159)
  void addComplex159(const cplx159_t *a, const cplx159_t *b, cplx159_t *res, realContext_t *realContext);
#endif // OPTION_CUBIC_159 || OPTION_SQUARE_159 || OPTION_EIGEN_159

#if (EXTRA_INFO_ON_CALC_ERROR == 1)
void addError   (void);
#else // (EXTRA_INFO_ON_CALC_ERROR == 1)
#define addError typeError
#endif // (EXTRA_INFO_ON_CALC_ERROR == 1)

//      RegYRegX
void addLonILonI(void);
  void addLonIRema(void);
  void addLonICxma(void);
void addLonITime(void);
void addLonIDate(void);
void addLonIShoI(void);
void addLonIReal(void);
void addLonICplx(void);

//      RegYRegX
void addTimeLonI(void);
void addTimeTime(void);
void addTimeReal(void);

//      RegYRegX
void addDateLonI(void);
void addDateReal(void);

//      RegYRegX
void addStriLonI(void);
void addStriTime(void);
void addStriStri(void);
void addStriRema(void);
void addStriCxma(void);
void addStriDate(void);
void addStriShoI(void);
void addStriReal(void);
void addStriCplx(void);
void addRegYStri(void);   //JM

//      RegYRegX
  void addRemaLonI(void);
void addRemaRema(void);
void addRemaCxma(void);
  void addRemaShoI(void);
  void addRemaReal(void);
  void addRemaCplx(void);

//      RegYRegX
  void addCxmaLonI(void);
void addCxmaRema(void);
void addCxmaCxma(void);
  void addCxmaShoI(void);
  void addCxmaReal(void);
  void addCxmaCplx(void);

//      RegYRegX
void addShoILonI(void);
  void addShoIRema(void);
  void addShoICxma(void);
void addShoIShoI(void);
void addShoIReal(void);
void addShoICplx(void);

//      RegYRegX
void addRealLonI(void);
  void addRealRema(void);
  void addRealCxma(void);
void addRealTime(void);
void addRealDate(void);
void addRealShoI(void);
void addRealReal(void);
void addRealCplx(void);

//      RegYRegX
void addCplxLonI(void);
  void addCplxRema(void);
  void addCplxCxma(void);
void addCplxShoI(void);
void addCplxReal(void);
void addCplxCplx(void);
#endif // !ADDITION_H
