// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file elec.c
 ***********************************************/

#include "c47.h"

#define ELEC_DUST_FILTER true                                                          // true: treat |Imag| below ELEC_DUST_LIMIT as real (rounding dust ignored for the complex gate); false: strict, any nonzero Imag counts as complex
#define ELEC_DUST_LIMIT  const_1e_37                                                   // imag magnitude at or below this is numerical dust, not a genuine complex result

#if defined(OPTION_ELEC)
  static bool_t elecImagIsDust(const real_t *im) {                                     // true if the imaginary part is to be treated as zero for the complex gate (dust filter), else only true for exact zero
    if(realIsZero(im)) {
      return true;
    }
  #if ELEC_DUST_FILTER
    return realCompareAbsLessThan(im, ELEC_DUST_LIMIT);
  #else
    return false;
  #endif // ELEC_DUST_FILTER
  }

  static bool_t elecInputIsComplex(const calcRegister_t *regs, int n) {                // true if any listed input register carries a genuine (non-dust) imaginary part, signalling the user accepts complex
    for(int i = 0; i < n; i++) {
      if(getRegisterDataType(regs[i]) == dtComplex34 && !real34IsZero(REGISTER_IMAG34_DATA(regs[i]))) {
        return true;
      }
    }
    return false;
  }

  static bool_t elecResultsOk(cplx_t * const *results, int n) {                        // gate computed results against FLAG_CPXRES / FLAG_SPCRES. true: caller may store as computed. false: caller must abort without writing (stack untouched). With CPXRES clear a genuine complex (non-dust Imag) or NaN is refused: SPCRES set substitutes real NaN, else raises out-of-domain. Dust is stored as-is, not scrubbed (user HIDE setting handles scrubbing)
    if(getSystemFlag(FLAG_CPXRES)) {
      return true;                                                                     // complex accepted: store as computed, real parts collapse normally on store
    }
    bool_t bad = false;
    for(int i = 0; i < n; i++) {
      if(!elecImagIsDust(&results[i]->Imag) || realIsNaN(&results[i]->Real) || realIsNaN(&results[i]->Imag)) {
        bad = true;
        break;
      }
    }
    if(!bad) {
      return true;                                                                     // all-real (within dust) and finite: fine for a real-only user
    }
    if(getSystemFlag(FLAG_SPCRES)) {
      for(int i = 0; i < n; i++) {                                                     // SPCRES: deliver NaN, real-typed, in place of the refused complex
        realSetNaN(&results[i]->Real);
        realSetZero(&results[i]->Imag);
      }
      return true;
    }
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In elec transform:", "complex result and CPXRES is not set", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }

  static void elecGetXYZ(cplx_t *x, cplx_t *y, cplx_t *z) {                             // read Z, Y, X registers as complex real_t values
    getRegisterAsComplex(REGISTER_X, x);
    getRegisterAsComplex(REGISTER_Y, y);
    getRegisterAsComplex(REGISTER_Z, z);
  }

  static void elecPutResults(const cplx_t *l, const cplx_t *x, const cplx_t *y, const cplx_t *z) {  // store L, X, Y, Z then collapse any pure-real result back to real34_t
    convertComplexToResultRegister(l, REGISTER_L);
    convertComplexToResultRegister(x, REGISTER_X);
    convertComplexToResultRegister(y, REGISTER_Y);
    convertComplexToResultRegister(z, REGISTER_Z);
    convertComplexRegisterToRealIfZeroImag(REGISTER_X);
    convertComplexRegisterToRealIfZeroImag(REGISTER_Y);
    convertComplexRegisterToRealIfZeroImag(REGISTER_Z);
    convertComplexRegisterToRealIfZeroImag(REGISTER_L);
  }

  /**
   * \brief Form the positive- and negative-sequence rotation operators.
   *
   * The 39-digit constant is sufficient for an isolated rotation.  A wider
   * transform derives the irrational component in its work context because
   * subtraction between sequence terms can expose the constant's low digits.
   */
  static void elecSetAOperators(cplx_t *aOp, cplx_t *aaOp, realContext_t *realContext) { // a = 1 angle 120deg = -1/2 + j*root3/2 ; a^2 is its conjugate
    realCopy(const_1on2, &aOp->Real);
    realChangeSign(&aOp->Real);
    if(realContext->digits <= 39) {
      realCopy(const39_root3on2, &aOp->Imag);
    }
    else {
      realSquareRoot(const_3, &aOp->Imag, realContext);
      realDivide(&aOp->Imag, const_2, &aOp->Imag, realContext);
    }
    realCopy(&aOp->Real, &aaOp->Real);
    realCopy(&aOp->Imag, &aaOp->Imag);
    realChangeSign(&aaOp->Imag);
  }

  /**
   * \brief Add two finite values and retain the rounding residual.
   *
   * The greater-magnitude operand is selected for the FastTwoSum identity.
   */
  static void elecTwoSum(const real_t *a, const real_t *b, real_t *sum, real_t *error, realContext_t *realContext) {
    REAL_T_PTR(z, 75);
    const real_t *larger = a;
    const real_t *smaller = b;

    if(realCompareAbsLessThan(larger, smaller)) {
      larger = b;
      smaller = a;
    }
    realAdd(larger, smaller, sum, realContext);
    realSubtract(sum, larger, z, realContext);
    realSubtract(smaller, z, error, realContext);
  }

  /**
   * \brief Add three real components with a two-term expansion.
   *
   * Special inputs retain the ordinary arithmetic semantics.  Finite inputs
   * use two FastTwoSum stages so that a small term is not discarded before a
   * later cancellation.
   */
  static void elecAddThreeReal(const real_t *a, const real_t *b, const real_t *c, real_t *result, realContext_t *realContext) {
    REAL_T_PTR(sum, 75);
    REAL_T_PTR(total, 75);
    REAL_T_PTR(error1, 75);
    REAL_T_PTR(error2, 75);

    if(realIsSpecial(a) || realIsSpecial(b) || realIsSpecial(c)) {
      realAdd(a, b, result, realContext);
      realAdd(result, c, result, realContext);
      return;
    }
    elecTwoSum(a, b, sum, error1, realContext);
    elecTwoSum(sum, c, total, error2, realContext);
    realAdd(error1, error2, error1, realContext);
    realAdd(total, error1, result, realContext);
  }

  /**
   * \brief Add three complex values component by component.
   */
  static void elecAddThreeComplex(const cplx_t *a, const cplx_t *b, const cplx_t *c, cplx_t *result, realContext_t *realContext) {
    elecAddThreeReal(&a->Real, &b->Real, &c->Real, &result->Real, realContext);
    elecAddThreeReal(&a->Imag, &b->Imag, &c->Imag, &result->Imag, realContext);
  }
#endif // OPTION_ELEC


  void fnDeltaToStar(uint16_t unusedButMandatoryParameter) {                          // Delta to Star; ZYX to ZYX. Cancellation-sensitive work uses ctxtReal75
  #if defined(OPTION_ELEC)
    cplx_t x, y, z, s, xy, yz, zx, rx, ry, rz;
    calcRegister_t inRegs[3] = {REGISTER_X, REGISTER_Y, REGISTER_Z};
    cplx_t *results[3] = {&rx, &ry, &rz};
    if(elecInputIsComplex(inRegs, 3)) {                                                // complex input declares complex intent: latch CPXRES on
      setSystemFlag(FLAG_CPXRES);
    }
    elecGetXYZ(&x, &y, &z);
    elecAddThreeComplex(&x, &y, &z, &s, &ctxtReal75);                                // s = x + y + z
    mulComplexComplex(&z, &x, &zx, &ctxtReal75);                                      // pairwise products
    mulComplexComplex(&x, &y, &xy, &ctxtReal75);
    mulComplexComplex(&y, &z, &yz, &ctxtReal75);
    divComplexComplex(&zx, &s, &rx, &ctxtReal75);                                     // X = (z*x) / s
    divComplexComplex(&xy, &s, &ry, &ctxtReal75);                                     // Y = (x*y) / s
    divComplexComplex(&yz, &s, &rz, &ctxtReal75);                                     // Z = (y*z) / s
    if(!elecResultsOk(results, 3)) {                                                  // abort clean: nothing written, stack intact
      return;
    }
    saveForUndo();                                                                     // commit point: disturb the stack only past the gate
    setSystemFlag(FLAG_ASLIFT);                                                        // 3-in/3-out: results overwrite ZYX in place, force lift so X is not swallowed
    elecPutResults(&x, &rx, &ry, &rz);                                                // L = original X
    temporaryInformation = TI_ABC;
  #endif // OPTION_ELEC
  }

  void fnStarToDelta(uint16_t unusedButMandatoryParameter) {                          // Star to Delta; ZYX to ZYX. Cancellation-sensitive work uses ctxtReal75
  #if defined(OPTION_ELEC)
    cplx_t x, y, z, xy, yz, zx, p, rx, ry, rz;
    calcRegister_t inRegs[3] = {REGISTER_X, REGISTER_Y, REGISTER_Z};
    cplx_t *results[3] = {&rx, &ry, &rz};
    if(elecInputIsComplex(inRegs, 3)) {                                                // complex input declares complex intent: latch CPXRES on
      setSystemFlag(FLAG_CPXRES);
    }
    elecGetXYZ(&x, &y, &z);
    mulComplexComplex(&x, &y, &xy, &ctxtReal75);                                      // p = x*y + y*z + z*x
    mulComplexComplex(&y, &z, &yz, &ctxtReal75);
    mulComplexComplex(&z, &x, &zx, &ctxtReal75);
    elecAddThreeComplex(&xy, &yz, &zx, &p, &ctxtReal75);
    divComplexComplex(&p, &z, &rx, &ctxtReal75);                                      // X = p / z
    divComplexComplex(&p, &x, &ry, &ctxtReal75);                                      // Y = p / x
    divComplexComplex(&p, &y, &rz, &ctxtReal75);                                      // Z = p / y
    if(!elecResultsOk(results, 3)) {                                                  // abort clean: nothing written, stack intact
      return;
    }
    saveForUndo();                                                                     // commit point: disturb the stack only past the gate
    setSystemFlag(FLAG_ASLIFT);                                                        // 3-in/3-out: results overwrite ZYX in place, force lift so X is not swallowed
    elecPutResults(&x, &rx, &ry, &rz);                                                // L = original X
    temporaryInformation = TI_ABBCCA;
  #endif // OPTION_ELEC
  }

  void fnSymToAbc(uint16_t unusedButMandatoryParameter) {                             // Symmetrical components -> ABC; cancellation-sensitive work uses ctxtReal75
  #if defined(OPTION_ELEC)
    cplx_t a2, a1, a0, va, vb, vc, aOp, aaOp, t;
    calcRegister_t inRegs[3] = {REGISTER_X, REGISTER_Y, REGISTER_Z};
    cplx_t *results[3] = {&vc, &vb, &va};
    if(elecInputIsComplex(inRegs, 3)) {                                                // complex input declares complex intent: latch CPXRES on
      setSystemFlag(FLAG_CPXRES);
    }
    elecGetXYZ(&a2, &a1, &a0);                                                        // X = A2 (negative), Y = A1 (positive), Z = A0 (zero) sequence
    elecSetAOperators(&aOp, &aaOp, &ctxtReal75);
    elecAddThreeComplex(&a0, &a1, &a2, &va, &ctxtReal75);                             // Va = A0 + A1 + A2
    mulComplexComplex(&aaOp, &a1, &vb, &ctxtReal75);                                  // Vb = A0 + a^2*A1 + a*A2
    mulComplexComplex(&aOp,  &a2, &t,  &ctxtReal75);
    elecAddThreeComplex(&a0, &vb, &t, &vb, &ctxtReal75);
    mulComplexComplex(&aOp,  &a1, &vc, &ctxtReal75);                                  // Vc = A0 + a*A1 + a^2*A2
    mulComplexComplex(&aaOp, &a2, &t,  &ctxtReal75);
    elecAddThreeComplex(&a0, &vc, &t, &vc, &ctxtReal75);
    if(!elecResultsOk(results, 3)) {                                                  // abort clean: nothing written, stack intact
      return;
    }
    saveForUndo();                                                                     // commit point: disturb the stack only past the gate
    setSystemFlag(FLAG_ASLIFT);                                                        // 3-in/3-out: results overwrite ZYX in place, force lift so X is not swallowed
    elecPutResults(&a2, &vc, &vb, &va);                                              // L = original X (A2)
    temporaryInformation = TI_ABC;
  #endif // OPTION_ELEC
  }

  void fnAbcToSym(uint16_t unusedButMandatoryParameter) {                             // ABC -> symmetrical components; cancellation-sensitive work uses ctxtReal75
  #if defined(OPTION_ELEC)
    cplx_t vc, vb, va, s0, s1, s2, aOp, aaOp, t;
    calcRegister_t inRegs[3] = {REGISTER_X, REGISTER_Y, REGISTER_Z};
    cplx_t *results[3] = {&s2, &s1, &s0};
    if(elecInputIsComplex(inRegs, 3)) {                                                // complex input declares complex intent: latch CPXRES on
      setSystemFlag(FLAG_CPXRES);
    }
    elecGetXYZ(&vc, &vb, &va);                                                        // X = Vc, Y = Vb, Z = Va
    elecSetAOperators(&aOp, &aaOp, &ctxtReal75);
    elecAddThreeComplex(&va, &vb, &vc, &s0, &ctxtReal75);                            // A0 = (Va + Vb + Vc) / 3
    realDivide(&s0.Real, const_3, &s0.Real, &ctxtReal75);
    realDivide(&s0.Imag, const_3, &s0.Imag, &ctxtReal75);
    mulComplexComplex(&aOp,  &vb, &s1, &ctxtReal75);                                  // A1 = (Va + a*Vb + a^2*Vc) / 3
    mulComplexComplex(&aaOp, &vc, &t,  &ctxtReal75);
    elecAddThreeComplex(&va, &s1, &t, &s1, &ctxtReal75);
    realDivide(&s1.Real, const_3, &s1.Real, &ctxtReal75);
    realDivide(&s1.Imag, const_3, &s1.Imag, &ctxtReal75);
    mulComplexComplex(&aaOp, &vb, &s2, &ctxtReal75);                                  // A2 = (Va + a^2*Vb + a*Vc) / 3
    mulComplexComplex(&aOp,  &vc, &t,  &ctxtReal75);
    elecAddThreeComplex(&va, &s2, &t, &s2, &ctxtReal75);
    realDivide(&s2.Real, const_3, &s2.Real, &ctxtReal75);
    realDivide(&s2.Imag, const_3, &s2.Imag, &ctxtReal75);
    if(!elecResultsOk(results, 3)) {                                                  // abort clean: nothing written, stack intact
      return;
    }
    saveForUndo();                                                                     // commit point: disturb the stack only past the gate
    setSystemFlag(FLAG_ASLIFT);                                                        // 3-in/3-out: results overwrite ZYX in place, force lift so X is not swallowed
    elecPutResults(&vc, &s2, &s1, &s0);                                              // L = original X (Vc)
    temporaryInformation = TI_012;
  #endif // OPTION_ELEC
  }



#if defined(OPTION_ELEC)
  static void flipPolar(calcRegister_t regist) {                                      // toggle polar/rectangular DISPLAY of one register. Setting amPolar is a display tag only; fnToRect2 is the actual data conversion and is X-only, so non-X registers are swapped into X around the body and swapped back
    bool_t notX = (regist != REGISTER_X);
    if(notX) {
      fnSwapX(regist);                                                               // bring target into X for the X-only operations below
    }
    {
      cplx_t z;
      if(getRegisterDataType(REGISTER_X) != dtComplex34) {
        if(getRegisterAsComplex(REGISTER_X, &z)) {
          convertComplexToResultRegister(&z, REGISTER_X);                            // promote a real register to complex (rectangular); on read failure leave X unchanged
        }
      }
      else if(getComplexRegisterPolarMode(REGISTER_X) != amPolar) {
        setComplexRegisterPolarMode(REGISTER_X, amPolar);                            // rect -> polar: display tag only, stored data unchanged
      }
      else {
        fnToRect2(NOPARAM);                                                          // polar -> rect: genuine data conversion, X-only
      }
    }
    if(notX) {
      fnSwapX(regist);                                                               // restore: target back to its register, prior X back to X
    }
  }

  static void elecGetTriple(calcRegister_t base, cplx_t *r1, cplx_t *r2, cplx_t *r3) {  // read three consecutive named registers as complex real_t values
    getRegisterAsComplex(base,     r1);
    getRegisterAsComplex(base + 1, r2);
    getRegisterAsComplex(base + 2, r3);
  }

  static void elecStoreTriple(calcRegister_t base, const cplx_t *r1, const cplx_t *r2, const cplx_t *r3) {  // place phase1,phase2,phase3 as stack X,Y,Z then copy to base..base+2. The first placement honours the incoming FLAG_ASLIFT (overwrite X when clear, push when set); the remaining placements always push. The register copy is unconditional, mirroring fn3Sto
    liftStack();
    convertComplexToResultRegister(r3, REGISTER_X);                                   // phase 3 (deepest) into X
    setSystemFlag(FLAG_ASLIFT);
    liftStack();
    convertComplexToResultRegister(r2, REGISTER_X);                                   // phase 2 into X, phase 3 rolls to Y
    setSystemFlag(FLAG_ASLIFT);
    liftStack();
    convertComplexToResultRegister(r1, REGISTER_X);                                   // phase 1 into X, phase 2 -> Y, phase 3 -> Z
    convertComplexRegisterToRealIfZeroImag(REGISTER_X);
    convertComplexRegisterToRealIfZeroImag(REGISTER_Y);
    convertComplexRegisterToRealIfZeroImag(REGISTER_Z);
    setSystemFlag(FLAG_ASLIFT);
    copySourceRegisterToDestRegister(REGISTER_X, base + 0);                           // copy the visible X,Y,Z to the destination register triple
    copySourceRegisterToDestRegister(REGISTER_Y, base + 1);
    copySourceRegisterToDestRegister(REGISTER_Z, base + 2);
  }
#endif // OPTION_ELEC


  #define TripleRegZ1_96 96  //old:rr90
  #define TripleRegZ2_97 97  //old:rr91
  #define TripleRegZ3_98 98  //old:rr92
  #define TripleRegV1_90 90  //old:rr93
  #define TripleRegV2_91 91  //old:rr94
  #define TripleRegV3_92 92  //old:rr95
  #define TripleRegI1_93 93  //old:rr96
  #define TripleRegI2_94 94  //old:rr97
  #define TripleRegI3_95 95  //old:rr98


  void fnTripleZfromVI(uint16_t unusedButMandatoryParameter) {                        // V/I : three-phase Z = V / I ; pushes Z1,Z2,Z3 onto stack (X=Z1) and stores to Z1..Z3 registers. Complex math in real_t (ctxtReal75), in/out via real34_t
  #if defined(OPTION_ELEC)
    cplx_t v1, v2, v3, i1, i2, i3, z1, z2, z3;
    calcRegister_t inRegs[6] = {TripleRegV1_90, TripleRegV2_91, TripleRegV3_92, TripleRegI1_93, TripleRegI2_94, TripleRegI3_95};
    cplx_t *results[3] = {&z1, &z2, &z3};
    if(elecInputIsComplex(inRegs, 6)) {                                                // complex source registers declare complex intent: latch CPXRES on
      setSystemFlag(FLAG_CPXRES);
    }
    elecGetTriple(TripleRegV1_90, &v1, &v2, &v3);                                     // V1..V3
    elecGetTriple(TripleRegI1_93, &i1, &i2, &i3);                                     // I1..I3
    divComplexComplex(&v1, &i1, &z1, &ctxtReal75);                                    // Z1 = V1 / I1
    divComplexComplex(&v2, &i2, &z2, &ctxtReal75);                                    // Z2 = V2 / I2
    divComplexComplex(&v3, &i3, &z3, &ctxtReal75);                                    // Z3 = V3 / I3
    if(!elecResultsOk(results, 3)) {                                                  // abort clean: nothing written, stack and registers intact
      return;
    }
    saveForUndo();                                                                     // commit point: disturb the stack only past the gate
    elecStoreTriple(TripleRegZ1_96, &z1, &z2, &z3);
  #endif // OPTION_ELEC
  }

  void fnTripleVfromIZ(uint16_t unusedButMandatoryParameter) {                        // IZ : three-phase V = I * Z ; pushes V1,V2,V3 onto stack (X=V1) and stores to V1..V3 registers. Complex multiplication uses ctxtReal39
  #if defined(OPTION_ELEC)
    cplx_t i1, i2, i3, z1, z2, z3, v1, v2, v3;
    calcRegister_t inRegs[6] = {TripleRegI1_93, TripleRegI2_94, TripleRegI3_95, TripleRegZ1_96, TripleRegZ2_97, TripleRegZ3_98};
    cplx_t *results[3] = {&v1, &v2, &v3};
    if(elecInputIsComplex(inRegs, 6)) {                                                // complex source registers declare complex intent: latch CPXRES on
      setSystemFlag(FLAG_CPXRES);
    }
    elecGetTriple(TripleRegI1_93, &i1, &i2, &i3);                                     // I1..I3
    elecGetTriple(TripleRegZ1_96, &z1, &z2, &z3);                                     // Z1..Z3
    mulComplexComplex(&i1, &z1, &v1, &ctxtReal39);                                    // V1 = I1 * Z1
    mulComplexComplex(&i2, &z2, &v2, &ctxtReal39);                                    // V2 = I2 * Z2
    mulComplexComplex(&i3, &z3, &v3, &ctxtReal39);                                    // V3 = I3 * Z3
    if(!elecResultsOk(results, 3)) {                                                  // abort clean: nothing written, stack and registers intact
      return;
    }
    saveForUndo();                                                                     // commit point: disturb the stack only past the gate
    elecStoreTriple(TripleRegV1_90, &v1, &v2, &v3);
  #endif // OPTION_ELEC
  }

  void fnTripleIfromVZ(uint16_t unusedButMandatoryParameter) {                        // V/Z : three-phase I = V / Z ; pushes I1,I2,I3 onto stack (X=I1) and stores to I1..I3 registers. Complex math in real_t (ctxtReal75), in/out via real34_t
  #if defined(OPTION_ELEC)
    cplx_t v1, v2, v3, z1, z2, z3, i1, i2, i3;
    calcRegister_t inRegs[6] = {TripleRegV1_90, TripleRegV2_91, TripleRegV3_92, TripleRegZ1_96, TripleRegZ2_97, TripleRegZ3_98};
    cplx_t *results[3] = {&i1, &i2, &i3};
    if(elecInputIsComplex(inRegs, 6)) {                                                // complex source registers declare complex intent: latch CPXRES on
      setSystemFlag(FLAG_CPXRES);
    }
    elecGetTriple(TripleRegV1_90, &v1, &v2, &v3);                                     // V1..V3
    elecGetTriple(TripleRegZ1_96, &z1, &z2, &z3);                                     // Z1..Z3
    divComplexComplex(&v1, &z1, &i1, &ctxtReal75);                                    // I1 = V1 / Z1
    divComplexComplex(&v2, &z2, &i2, &ctxtReal75);                                    // I2 = V2 / Z2
    divComplexComplex(&v3, &z3, &i3, &ctxtReal75);                                    // I3 = V3 / Z3
    if(!elecResultsOk(results, 3)) {                                                  // abort clean: nothing written, stack and registers intact
      return;
    }
    saveForUndo();                                                                     // commit point: disturb the stack only past the gate
    elecStoreTriple(TripleRegI1_93, &i1, &i2, &i3);
  #endif // OPTION_ELEC
  }

  void fnTripleFlipPolar(uint16_t unusedButMandatoryParameter) {                      // toggle polar/rectangular display on stack X, Y, Z (three-phase). Display-only, produces no new complex value, so it carries no complex gate
  #if defined(OPTION_ELEC)
    saveForUndo();
    flipPolar(REGISTER_X);
    flipPolar(REGISTER_Y);
    flipPolar(REGISTER_Z);
  #endif // OPTION_ELEC
  }

  void fnCopyXtoAbc(uint16_t unusedButMandatoryParameter) {                           // Copy/create X -> ABC balanced set. 1-in/3-out: x in Z, Y = a*x, X = a^2*x ; L = original x. Isolated rotations use ctxtReal39
  #if defined(OPTION_ELEC)
    cplx_t x, rx, ry, aOp, aaOp;
    calcRegister_t inRegs[1] = {REGISTER_X};
    cplx_t *results[3] = {&rx, &ry, &x};
    if(elecInputIsComplex(inRegs, 1)) {                                                // complex input declares complex intent: latch CPXRES on
      setSystemFlag(FLAG_CPXRES);
    }
    getRegisterAsComplex(REGISTER_X, &x);                                             // x = source phasor
    elecSetAOperators(&aOp, &aaOp, &ctxtReal39);                                      // a = 1 angle 120deg ; a^2 = 1 angle 240deg
    mulComplexComplex(&aOp,  &x, &ry, &ctxtReal39);                                   // a*x   -> Y
    mulComplexComplex(&aaOp, &x, &rx, &ctxtReal39);                                   // a^2*x -> X
    if(!elecResultsOk(results, 3)) {                                                  // abort clean: nothing written, stack intact
      return;
    }
    saveForUndo();                                                                     // commit point: disturb the stack only past the gate
    fnDrop(NOPARAM);                                                                  // consume the input so the set is built onto the stack below it
    convertComplexToResultRegister(&x, REGISTER_L);                                   // L = original x (last x)
    liftStack();
    convertComplexToResultRegister(&x, REGISTER_X);                                   // x is the base of the set
    setSystemFlag(FLAG_ASLIFT);
    liftStack();
    convertComplexToResultRegister(&ry, REGISTER_X);                                  // a*x into X, x rolls to Y
    setSystemFlag(FLAG_ASLIFT);
    liftStack();
    convertComplexToResultRegister(&rx, REGISTER_X);                                  // a^2*x into X, a*x rolls to Y, x rolls to Z
    convertComplexRegisterToRealIfZeroImag(REGISTER_X);
    convertComplexRegisterToRealIfZeroImag(REGISTER_Y);
    convertComplexRegisterToRealIfZeroImag(REGISTER_Z);
    convertComplexRegisterToRealIfZeroImag(REGISTER_L);
    temporaryInformation = TI_ABC;
  #endif // OPTION_ELEC
  }
