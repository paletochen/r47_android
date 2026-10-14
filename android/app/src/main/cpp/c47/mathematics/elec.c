// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file elec.c
 ***********************************************/

#include "c47.h"

#define ELEC_DUST_FILTER true                                                          // true: treat |Imag| below ELEC_DUST_LIMIT as real (rounding dust ignored for the complex gate); false: strict, any nonzero Imag counts as complex
#define ELEC_DUST_LIMIT  const_1e_37                                                   // imag magnitude at or below this is rounding dust from ctxtReal39, not a genuine complex result

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
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In elec transform:", "complex result and CPXRES is not set", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }

  static void elecGetXYZ(cplx_t *x, cplx_t *y, cplx_t *z) {                             // read Z, Y, X registers as complex (ctxtReal39 internals, real34_t in)
    getRegisterAsComplex(REGISTER_X, CPLX(*x));
    getRegisterAsComplex(REGISTER_Y, CPLX(*y));
    getRegisterAsComplex(REGISTER_Z, CPLX(*z));
  }

  static void elecPutResults(const cplx_t *l, const cplx_t *x, const cplx_t *y, const cplx_t *z) {  // store L, X, Y, Z then collapse any pure-real result back to real34_t
    convertComplexToResultRegister(CPLX(*l), REGISTER_L);
    convertComplexToResultRegister(CPLX(*x), REGISTER_X);
    convertComplexToResultRegister(CPLX(*y), REGISTER_Y);
    convertComplexToResultRegister(CPLX(*z), REGISTER_Z);
    convertComplexRegisterToRealIfZeroImag(REGISTER_X);
    convertComplexRegisterToRealIfZeroImag(REGISTER_Y);
    convertComplexRegisterToRealIfZeroImag(REGISTER_Z);
    convertComplexRegisterToRealIfZeroImag(REGISTER_L);
  }

  static void elecSetAOperators(cplx_t *aOp, cplx_t *aaOp) {                            // a = 1 angle 120deg = -1/2 + j*root3/2 ; a^2 = conjugate of a = -1/2 - j*root3/2
    realCopy(const_1on2, &aOp->Real);
    realChangeSign(&aOp->Real);
    realCopy(const39_root3on2, &aOp->Imag);
    realCopy(&aOp->Real, &aaOp->Real);
    realCopy(&aOp->Imag, &aaOp->Imag);
    realChangeSign(&aaOp->Imag);
  }
#endif // OPTION_ELEC


  void fnDeltaToStar(uint16_t unusedButMandatoryParameter) {                          // Delta to Star; ZYX to ZYX. Complex math in real_t (ctxtReal39), in/out via real34_t
  #if defined(OPTION_ELEC)
    cplx_t x, y, z, s, xy, yz, zx, rx, ry, rz;
    calcRegister_t inRegs[3] = {REGISTER_X, REGISTER_Y, REGISTER_Z};
    cplx_t *results[3] = {&rx, &ry, &rz};
    if(elecInputIsComplex(inRegs, 3)) {                                                // complex input declares complex intent: latch CPXRES on
      setSystemFlag(FLAG_CPXRES);
    }
    elecGetXYZ(&x, &y, &z);
    addComplex(CPLX(x), CPLX(y), CPLX(s), &ctxtReal39);                               // s = x + y + z
    addComplex(CPLX(s), CPLX(z), CPLX(s), &ctxtReal39);
    mulComplexComplex(CPLX(z), CPLX(x), CPLX(zx), &ctxtReal39);                       // pairwise products
    mulComplexComplex(CPLX(x), CPLX(y), CPLX(xy), &ctxtReal39);
    mulComplexComplex(CPLX(y), CPLX(z), CPLX(yz), &ctxtReal39);
    divComplexComplex(CPLX(zx), CPLX(s), CPLX(rx), &ctxtReal39);                      // X = (z*x) / s
    divComplexComplex(CPLX(xy), CPLX(s), CPLX(ry), &ctxtReal39);                      // Y = (x*y) / s
    divComplexComplex(CPLX(yz), CPLX(s), CPLX(rz), &ctxtReal39);                      // Z = (y*z) / s
    if(!elecResultsOk(results, 3)) {                                                  // abort clean: nothing written, stack intact
      return;
    }
    saveForUndo();                                                                     // commit point: disturb the stack only past the gate
    setSystemFlag(FLAG_ASLIFT);                                                        // 3-in/3-out: results overwrite ZYX in place, force lift so X is not swallowed
    elecPutResults(&x, &rx, &ry, &rz);                                                // L = original X
    temporaryInformation = TI_ABC;
  #endif // OPTION_ELEC
  }

  void fnStarToDelta(uint16_t unusedButMandatoryParameter) {                          // Star to Delta; ZYX to ZYX. Complex math in real_t (ctxtReal39), in/out via real34_t
  #if defined(OPTION_ELEC)
    cplx_t x, y, z, p, t, rx, ry, rz;
    calcRegister_t inRegs[3] = {REGISTER_X, REGISTER_Y, REGISTER_Z};
    cplx_t *results[3] = {&rx, &ry, &rz};
    if(elecInputIsComplex(inRegs, 3)) {                                                // complex input declares complex intent: latch CPXRES on
      setSystemFlag(FLAG_CPXRES);
    }
    elecGetXYZ(&x, &y, &z);
    mulComplexComplex(CPLX(x), CPLX(y), CPLX(p), &ctxtReal39);                        // p = x*y + y*z + z*x
    mulComplexComplex(CPLX(y), CPLX(z), CPLX(t), &ctxtReal39);
    addComplex(CPLX(p), CPLX(t), CPLX(p), &ctxtReal39);
    mulComplexComplex(CPLX(z), CPLX(x), CPLX(t), &ctxtReal39);
    addComplex(CPLX(p), CPLX(t), CPLX(p), &ctxtReal39);
    divComplexComplex(CPLX(p), CPLX(z), CPLX(rx), &ctxtReal39);                       // X = p / z
    divComplexComplex(CPLX(p), CPLX(x), CPLX(ry), &ctxtReal39);                       // Y = p / x
    divComplexComplex(CPLX(p), CPLX(y), CPLX(rz), &ctxtReal39);                       // Z = p / y
    if(!elecResultsOk(results, 3)) {                                                  // abort clean: nothing written, stack intact
      return;
    }
    saveForUndo();                                                                     // commit point: disturb the stack only past the gate
    setSystemFlag(FLAG_ASLIFT);                                                        // 3-in/3-out: results overwrite ZYX in place, force lift so X is not swallowed
    elecPutResults(&x, &rx, &ry, &rz);                                                // L = original X
    temporaryInformation = TI_ABBCCA;
  #endif // OPTION_ELEC
  }

  void fnSymToAbc(uint16_t unusedButMandatoryParameter) {                             // Symmetrical components -> ABC; ZYX to ZYX. Complex math in real_t (ctxtReal39), in/out via real34_t
  #if defined(OPTION_ELEC)
    cplx_t a2, a1, a0, va, vb, vc, aOp, aaOp, t;
    calcRegister_t inRegs[3] = {REGISTER_X, REGISTER_Y, REGISTER_Z};
    cplx_t *results[3] = {&vc, &vb, &va};
    if(elecInputIsComplex(inRegs, 3)) {                                                // complex input declares complex intent: latch CPXRES on
      setSystemFlag(FLAG_CPXRES);
    }
    elecGetXYZ(&a2, &a1, &a0);                                                        // X = A2 (negative), Y = A1 (positive), Z = A0 (zero) sequence
    elecSetAOperators(&aOp, &aaOp);
    addComplex(CPLX(a0), CPLX(a1), CPLX(va), &ctxtReal39);                            // Va = A0 + A1 + A2
    addComplex(CPLX(va), CPLX(a2), CPLX(va), &ctxtReal39);
    mulComplexComplex(CPLX(aaOp), CPLX(a1), CPLX(vb), &ctxtReal39);                   // Vb = A0 + a^2*A1 + a*A2
    mulComplexComplex(CPLX(aOp),  CPLX(a2), CPLX(t),  &ctxtReal39);
    addComplex(CPLX(vb), CPLX(t),  CPLX(vb), &ctxtReal39);
    addComplex(CPLX(vb), CPLX(a0), CPLX(vb), &ctxtReal39);
    mulComplexComplex(CPLX(aOp),  CPLX(a1), CPLX(vc), &ctxtReal39);                   // Vc = A0 + a*A1 + a^2*A2
    mulComplexComplex(CPLX(aaOp), CPLX(a2), CPLX(t),  &ctxtReal39);
    addComplex(CPLX(vc), CPLX(t),  CPLX(vc), &ctxtReal39);
    addComplex(CPLX(vc), CPLX(a0), CPLX(vc), &ctxtReal39);
    if(!elecResultsOk(results, 3)) {                                                  // abort clean: nothing written, stack intact
      return;
    }
    saveForUndo();                                                                     // commit point: disturb the stack only past the gate
    setSystemFlag(FLAG_ASLIFT);                                                        // 3-in/3-out: results overwrite ZYX in place, force lift so X is not swallowed
    elecPutResults(&a2, &vc, &vb, &va);                                              // L = original X (A2)
    temporaryInformation = TI_ABC;
  #endif // OPTION_ELEC
  }

  void fnAbcToSym(uint16_t unusedButMandatoryParameter) {                             // ABC -> symmetrical components; ZYX to ZYX. Complex math in real_t (ctxtReal39), in/out via real34_t
  #if defined(OPTION_ELEC)
    cplx_t vc, vb, va, s0, s1, s2, aOp, aaOp, t;
    calcRegister_t inRegs[3] = {REGISTER_X, REGISTER_Y, REGISTER_Z};
    cplx_t *results[3] = {&s2, &s1, &s0};
    if(elecInputIsComplex(inRegs, 3)) {                                                // complex input declares complex intent: latch CPXRES on
      setSystemFlag(FLAG_CPXRES);
    }
    elecGetXYZ(&vc, &vb, &va);                                                        // X = Vc, Y = Vb, Z = Va
    elecSetAOperators(&aOp, &aaOp);
    addComplex(CPLX(va), CPLX(vb), CPLX(s0), &ctxtReal39);                            // A0 = (Va + Vb + Vc) / 3
    addComplex(CPLX(s0), CPLX(vc), CPLX(s0), &ctxtReal39);
    realDivide(&s0.Real, const_3, &s0.Real, &ctxtReal39);
    realDivide(&s0.Imag, const_3, &s0.Imag, &ctxtReal39);
    mulComplexComplex(CPLX(aOp),  CPLX(vb), CPLX(s1), &ctxtReal39);                   // A1 = (Va + a*Vb + a^2*Vc) / 3
    mulComplexComplex(CPLX(aaOp), CPLX(vc), CPLX(t),  &ctxtReal39);
    addComplex(CPLX(s1), CPLX(t),  CPLX(s1), &ctxtReal39);
    addComplex(CPLX(s1), CPLX(va), CPLX(s1), &ctxtReal39);
    realDivide(&s1.Real, const_3, &s1.Real, &ctxtReal39);
    realDivide(&s1.Imag, const_3, &s1.Imag, &ctxtReal39);
    mulComplexComplex(CPLX(aaOp), CPLX(vb), CPLX(s2), &ctxtReal39);                   // A2 = (Va + a^2*Vb + a*Vc) / 3
    mulComplexComplex(CPLX(aOp),  CPLX(vc), CPLX(t),  &ctxtReal39);
    addComplex(CPLX(s2), CPLX(t),  CPLX(s2), &ctxtReal39);
    addComplex(CPLX(s2), CPLX(va), CPLX(s2), &ctxtReal39);
    realDivide(&s2.Real, const_3, &s2.Real, &ctxtReal39);
    realDivide(&s2.Imag, const_3, &s2.Imag, &ctxtReal39);
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
      real_t zReal, zImag;
      if(getRegisterDataType(REGISTER_X) != dtComplex34) {
        if(getRegisterAsComplex(REGISTER_X, &zReal, &zImag)) {
          convertComplexToResultRegister(&zReal, &zImag, REGISTER_X);                // promote a real register to complex (rectangular); on read failure leave X unchanged
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

  static void elecGetTriple(calcRegister_t base, cplx_t *r1, cplx_t *r2, cplx_t *r3) {  // read three consecutive named registers (base, base+1, base+2) as complex, ctxtReal39 internals
    getRegisterAsComplex(base,     CPLX(*r1));
    getRegisterAsComplex(base + 1, CPLX(*r2));
    getRegisterAsComplex(base + 2, CPLX(*r3));
  }

  static void elecStoreTriple(calcRegister_t base, const cplx_t *r1, const cplx_t *r2, const cplx_t *r3) {  // place phase1,phase2,phase3 as stack X,Y,Z then copy to base..base+2. The first placement honours the incoming FLAG_ASLIFT (overwrite X when clear, push when set); the remaining placements always push. The register copy is unconditional, mirroring fn3Sto
    liftStack();
    convertComplexToResultRegister(CPLX(*r3), REGISTER_X);                            // phase 3 (deepest) into X
    setSystemFlag(FLAG_ASLIFT);
    liftStack();
    convertComplexToResultRegister(CPLX(*r2), REGISTER_X);                            // phase 2 into X, phase 3 rolls to Y
    setSystemFlag(FLAG_ASLIFT);
    liftStack();
    convertComplexToResultRegister(CPLX(*r1), REGISTER_X);                            // phase 1 into X, phase 2 -> Y, phase 3 -> Z
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


  void fnTripleZfromVI(uint16_t unusedButMandatoryParameter) {                        // V/I : three-phase Z = V / I ; pushes Z1,Z2,Z3 onto stack (X=Z1) and stores to Z1..Z3 registers. Complex math in real_t (ctxtReal39), in/out via real34_t
  #if defined(OPTION_ELEC)
    cplx_t v1, v2, v3, i1, i2, i3, z1, z2, z3;
    calcRegister_t inRegs[6] = {TripleRegV1_90, TripleRegV2_91, TripleRegV3_92, TripleRegI1_93, TripleRegI2_94, TripleRegI3_95};
    cplx_t *results[3] = {&z1, &z2, &z3};
    if(elecInputIsComplex(inRegs, 6)) {                                                // complex source registers declare complex intent: latch CPXRES on
      setSystemFlag(FLAG_CPXRES);
    }
    elecGetTriple(TripleRegV1_90, &v1, &v2, &v3);                                     // V1..V3
    elecGetTriple(TripleRegI1_93, &i1, &i2, &i3);                                     // I1..I3
    divComplexComplex(CPLX(v1), CPLX(i1), CPLX(z1), &ctxtReal39);                     // Z1 = V1 / I1
    divComplexComplex(CPLX(v2), CPLX(i2), CPLX(z2), &ctxtReal39);                     // Z2 = V2 / I2
    divComplexComplex(CPLX(v3), CPLX(i3), CPLX(z3), &ctxtReal39);                     // Z3 = V3 / I3
    if(!elecResultsOk(results, 3)) {                                                  // abort clean: nothing written, stack and registers intact
      return;
    }
    saveForUndo();                                                                     // commit point: disturb the stack only past the gate
    elecStoreTriple(TripleRegZ1_96, &z1, &z2, &z3);
  #endif // OPTION_ELEC
  }

  void fnTripleVfromIZ(uint16_t unusedButMandatoryParameter) {                        // IZ : three-phase V = I * Z ; pushes V1,V2,V3 onto stack (X=V1) and stores to V1..V3 registers. Complex math in real_t (ctxtReal39), in/out via real34_t
  #if defined(OPTION_ELEC)
    cplx_t i1, i2, i3, z1, z2, z3, v1, v2, v3;
    calcRegister_t inRegs[6] = {TripleRegI1_93, TripleRegI2_94, TripleRegI3_95, TripleRegZ1_96, TripleRegZ2_97, TripleRegZ3_98};
    cplx_t *results[3] = {&v1, &v2, &v3};
    if(elecInputIsComplex(inRegs, 6)) {                                                // complex source registers declare complex intent: latch CPXRES on
      setSystemFlag(FLAG_CPXRES);
    }
    elecGetTriple(TripleRegI1_93, &i1, &i2, &i3);                                     // I1..I3
    elecGetTriple(TripleRegZ1_96, &z1, &z2, &z3);                                     // Z1..Z3
    mulComplexComplex(CPLX(i1), CPLX(z1), CPLX(v1), &ctxtReal39);                     // V1 = I1 * Z1
    mulComplexComplex(CPLX(i2), CPLX(z2), CPLX(v2), &ctxtReal39);                     // V2 = I2 * Z2
    mulComplexComplex(CPLX(i3), CPLX(z3), CPLX(v3), &ctxtReal39);                     // V3 = I3 * Z3
    if(!elecResultsOk(results, 3)) {                                                  // abort clean: nothing written, stack and registers intact
      return;
    }
    saveForUndo();                                                                     // commit point: disturb the stack only past the gate
    elecStoreTriple(TripleRegV1_90, &v1, &v2, &v3);
  #endif // OPTION_ELEC
  }

  void fnTripleIfromVZ(uint16_t unusedButMandatoryParameter) {                        // V/Z : three-phase I = V / Z ; pushes I1,I2,I3 onto stack (X=I1) and stores to I1..I3 registers. Complex math in real_t (ctxtReal39), in/out via real34_t
  #if defined(OPTION_ELEC)
    cplx_t v1, v2, v3, z1, z2, z3, i1, i2, i3;
    calcRegister_t inRegs[6] = {TripleRegV1_90, TripleRegV2_91, TripleRegV3_92, TripleRegZ1_96, TripleRegZ2_97, TripleRegZ3_98};
    cplx_t *results[3] = {&i1, &i2, &i3};
    if(elecInputIsComplex(inRegs, 6)) {                                                // complex source registers declare complex intent: latch CPXRES on
      setSystemFlag(FLAG_CPXRES);
    }
    elecGetTriple(TripleRegV1_90, &v1, &v2, &v3);                                     // V1..V3
    elecGetTriple(TripleRegZ1_96, &z1, &z2, &z3);                                     // Z1..Z3
    divComplexComplex(CPLX(v1), CPLX(z1), CPLX(i1), &ctxtReal39);                     // I1 = V1 / Z1
    divComplexComplex(CPLX(v2), CPLX(z2), CPLX(i2), &ctxtReal39);                     // I2 = V2 / Z2
    divComplexComplex(CPLX(v3), CPLX(z3), CPLX(i3), &ctxtReal39);                     // I3 = V3 / Z3
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

  void fnCopyXtoAbc(uint16_t unusedButMandatoryParameter) {                           // Copy/create X -> ABC balanced set. 1-in/3-out: x in Z, Y = a*x, X = a^2*x ; L = original x. The input is consumed (dropped) before the set is built, so the first placement honours the incoming FLAG_ASLIFT. Complex math in real_t (ctxtReal39), in/out via real34_t
  #if defined(OPTION_ELEC)
    cplx_t x, rx, ry, aOp, aaOp;
    calcRegister_t inRegs[1] = {REGISTER_X};
    cplx_t *results[3] = {&rx, &ry, &x};
    if(elecInputIsComplex(inRegs, 1)) {                                                // complex input declares complex intent: latch CPXRES on
      setSystemFlag(FLAG_CPXRES);
    }
    getRegisterAsComplex(REGISTER_X, CPLX(x));                                        // x = source phasor
    elecSetAOperators(&aOp, &aaOp);                                                   // a = 1 angle 120deg ; a^2 = 1 angle 240deg
    mulComplexComplex(CPLX(aOp),  CPLX(x), CPLX(ry), &ctxtReal39);                    // a*x   -> Y
    mulComplexComplex(CPLX(aaOp), CPLX(x), CPLX(rx), &ctxtReal39);                    // a^2*x -> X
    if(!elecResultsOk(results, 3)) {                                                  // abort clean: nothing written, stack intact
      return;
    }
    saveForUndo();                                                                     // commit point: disturb the stack only past the gate
    fnDrop(NOPARAM);                                                                  // consume the input so the set is built onto the stack below it
    convertComplexToResultRegister(CPLX(x), REGISTER_L);                              // L = original x (last x)
    liftStack();
    convertComplexToResultRegister(CPLX(x), REGISTER_X);                              // x is the base of the set
    setSystemFlag(FLAG_ASLIFT);
    liftStack();
    convertComplexToResultRegister(CPLX(ry), REGISTER_X);                             // a*x into X, x rolls to Y
    setSystemFlag(FLAG_ASLIFT);
    liftStack();
    convertComplexToResultRegister(CPLX(rx), REGISTER_X);                             // a^2*x into X, a*x rolls to Y, x rolls to Z
    convertComplexRegisterToRealIfZeroImag(REGISTER_X);
    convertComplexRegisterToRealIfZeroImag(REGISTER_Y);
    convertComplexRegisterToRealIfZeroImag(REGISTER_Z);
    convertComplexRegisterToRealIfZeroImag(REGISTER_L);
    temporaryInformation = TI_ABC;
  #endif // OPTION_ELEC
  }
