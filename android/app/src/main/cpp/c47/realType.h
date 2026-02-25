// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file realType.h
 */

#if !defined(REALTYPE_H)
  #define REALTYPE_H

#define DEFN_REAL(n)                            \
  typedef struct {                              \
    int32_t digits;                             \
    int32_t exponent;                           \
    uint8_t bits;                               \
    decNumberUnit lsu[(n+DECDPUN-1)/DECDPUN];   \
  } real ## n ## _t

  DEFN_REAL(39);
  DEFN_REAL(51);
  DEFN_REAL(1071);
  DEFN_REAL(2139);
  DEFN_REAL(6147);
  DEFN_REAL(12321);
#undef DEFN_REAL

  typedef decContext realContext_t;
  typedef decQuad    real34_t; // 34 digits and 128 bits = 16 bytes

  typedef struct {
    real34_t real, imag;
  } complex34_t;

  typedef decNumber  real_t;

  #define REAL_SIZE_IN_BLOCKS      TO_BLOCKS(sizeof(real_t))
  #define REAL34_SIZE_IN_BLOCKS    TO_BLOCKS(sizeof(real34_t))
  #define REAL39_SIZE_IN_BLOCKS    TO_BLOCKS(sizeof(real39_t))
  #define REAL51_SIZE_IN_BLOCKS    TO_BLOCKS(sizeof(real51_t))
  #define REAL1071_SIZE_IN_BLOCKS  TO_BLOCKS(sizeof(real1071_t))
  #define REAL2139_SIZE_IN_BLOCKS  TO_BLOCKS(sizeof(real2139_t))
  #define REAL6147_SIZE_IN_BLOCKS  TO_BLOCKS(sizeof(real6147_t))
  #define COMPLEX34_SIZE_IN_BLOCKS TO_BLOCKS(sizeof(complex34_t))

  #define REAL_SIZE_IN_BYTES       TO_BYTES(REAL_SIZE_IN_BLOCKS)
  #define REAL34_SIZE_IN_BYTES     TO_BYTES(REAL34_SIZE_IN_BLOCKS)
  #define REAL39_SIZE_IN_BYTES     TO_BYTES(REAL39_SIZE_IN_BLOCKS)
  #define REAL51_SIZE_IN_BYTES     TO_BYTES(REAL51_SIZE_IN_BLOCKS)
  #define REAL1071_SIZE_IN_BYTES   TO_BYTES(REAL1071_SIZE_IN_BLOCKS)
  #define REAL2139_SIZE_IN_BYTES   TO_BYTES(REAL2139_SIZE_IN_BLOCKS)
  #define REAL6147_SIZE_IN_BYTES   TO_BYTES(REAL6147_SIZE_IN_BLOCKS)
  #define COMPLEX34_SIZE_IN_BYTES  TO_BYTES(COMPLEX34_SIZE_IN_BLOCKS)

  #define VARIABLE_REAL34_DATA(a)                                ((real34_t    *)(a))
  #define VARIABLE_IMAG34_DATA(a)                                ((real34_t    *)((void *)(a) + REAL34_SIZE_IN_BYTES))


//****** added out of place not to clash with coming bf29e4a0 on TaylorMod-import-Mo...
//       will be replaced once the new realtype macros are merged
#if defined(OPTION_CUBIC_159) || defined(OPTION_SQUARE_159) || defined(OPTION_EIGEN_159)
  typedef struct {
    int32_t digits;      // Count of digits in the coefficient; >0
    int32_t exponent;    // Unadjusted exponent, unbiased, in
                         // range: -1999999997 through 999999999
    uint8_t bits;        // Indicator bits (see above)
                         // Coefficient, from least significant unit
    decNumberUnit lsu[(159+DECDPUN-1)/DECDPUN]; // 1071 = 39 + 10*12
  } real159_t; // used for cube root and calculateeigenvalues3x3
  #define REAL159_SIZE_IN_BLOCKS    TO_BLOCKS(sizeof(real159_t))
  #define REAL159_SIZE_IN_BYTES     TO_BYTES(REAL159_SIZE_IN_BLOCKS)
#endif //OPTION_CUBIC_159 || OPTION_SQUARE_159 || OPTION_EIGEN_159
//****** added out of place not to clash with coming bf29e4a0 on TaylorMod-import-Mo...


  int32_t  realToInt32C47 (const real_t *r);
  uint32_t realToUint32C47(const real_t *r);
  //int64_t  realToInt64C47 (const real_t *r);
  uint64_t realToUint64C47(const real_t *r);

  #define complex34ChangeSign(operand)                           do {real34ChangeSign((real34_t *)(operand));                               \
                                                                  real34ChangeSign((real34_t *)((void *)(operand) + REAL34_SIZE_IN_BYTES)); \
                                                                 } while(0)
  #define complex34Copy(source, destination)                     do {  *(uint64_t *)(destination)   =   *(uint64_t *)(source);       \
                                                                     *(((uint64_t *)(destination))+1) = *(((uint64_t *)(source))+1); \
                                                                     *(((uint64_t *)(destination))+2) = *(((uint64_t *)(source))+2); \
                                                                     *(((uint64_t *)(destination))+3) = *(((uint64_t *)(source))+3); \
                                                                 } while(0)
  #define int32ToReal34(source, destination)                     decQuadFromInt32         ((real34_t *)(destination), source)
  #define real34Add(operand1, operand2, res)                     decQuadAdd               ((real34_t *)(res), (real34_t *)(operand1), (real34_t *)(operand2), &ctxtReal34)
  #define real34ChangeSign(operand)                              ((real34_t *)(operand))->bytes[15] ^= 0x80
  #define real34Compare(operand1, operand2, res)                 decQuadCompare           ((real34_t *)(res), (real34_t *)(operand1), (real34_t *)(operand2), &ctxtReal34)
  //#define real34Copy(source, destination)                        decQuadCopy            (destination, source)
  //#define real34Copy(source, destination)                        xcopy(destination, source, REAL34_SIZE_IN_BYTES)
  #define real34Copy(source, destination)                        do { *(uint64_t *)(destination)     =   *(uint64_t *)(source);     \
                                                                    *(((uint64_t *)(destination))+1) = *(((uint64_t *)(source))+1); \
                                                                 } while(0)
  #define real34CopyAbs(source, destination)                     decQuadCopyAbs           (destination, source)
  #define real34Digits(source)                                   decQuadDigits            ((real34_t *)(source))
  #define real34Divide(operand1, operand2, res)                  decQuadDivide            ((real34_t *)(res), (real34_t *)(operand1), (real34_t *)(operand2), &ctxtReal34)
  #define real34DivideRemainder(operand1, operand2, res)         decQuadRemainder         ((real34_t *)(res), (real34_t *)(operand1), (real34_t *)(operand2), &ctxtReal34)
  #define real34FMA(factor1, factor2, term, res)                 decQuadFMA               ((real34_t *)(res), (real34_t *)(factor1),  (real34_t *)(factor2),  (real34_t *)(term), &ctxtReal34)
  #define real34GetCoefficient(source, destination)              decQuadGetCoefficient    ((real34_t *)(source), (uint8_t *)(destination))
  #define real34GetExponent(source)                              decQuadGetExponent       ((real34_t *)(source))
  #define real34IsInfinite(source)                               decQuadIsInfinite        ((real34_t *)(source))
  #define real34IsNaN(source)                                    decQuadIsNaN             ((real34_t *)(source))
  #define real34IsNegative(source)                               (((((real34_t *)(source))->bytes[15]) & 0x80) == 0x80)
  #define real34IsPositive(source)                               (((((real34_t *)(source))->bytes[15]) & 0x80) == 0x00)
  #define real34IsSpecial(source)                                (decQuadIsNaN((real34_t *)(source))   || decQuadIsSignaling((real34_t *)(source))   || decQuadIsInfinite((real34_t *)(source)))
  #define real34IsZero(source)                                   decQuadIsZero            ((real34_t *)(source))
  #define real34Minus(operand, res)                              decQuadMinus             ((real34_t *)(res), (real34_t *)(operand), &ctxtReal34)
  #define real34Multiply(operand1, operand2, res)                decQuadMultiply          ((real34_t *)(res), (real34_t *)(operand1), (real34_t *)(operand2), &ctxtReal34)
  #define real34NextMinus(operand, res)                          decQuadNextMinus          ((real34_t *)(res), (real34_t *)(operand), &ctxtReal34)
  #define real34NextPlus(operand, res)                           decQuadNextPlus          ((real34_t *)(res), (real34_t *)(operand), &ctxtReal34)
  #define real34Plus(operand, res)                               decQuadPlus              ((real34_t *)(res), (real34_t *)(operand), &ctxtReal34)
  #define real34SetNegativeSign(operand)                         ((real34_t *)(operand))->bytes[15] |= 0x80
  #define real34SetPositiveSign(operand)                         ((real34_t *)(operand))->bytes[15] &= 0x7F
  #define real34Subtract(operand1, operand2, res)                decQuadSubtract          ((real34_t *)(res), (real34_t *)(operand1), (real34_t *)(operand2), &ctxtReal34)
  #define real34ToInt32(source)                                  decQuadToInt32           ((real34_t *)(source), &ctxtReal34, DEC_ROUND_DOWN)
  #define real34ToIntegralValue(source, destination, mode)       decQuadToIntegralValue   ((real34_t *)(destination), (real34_t *)(source), &ctxtReal34, mode)
  #define real34ToReal(source, destination)                      decQuadToNumber          ((real34_t *)(source), destination)
  #define real34ToString(source, destination)                    decQuadToString          ((real34_t *)(source), destination)
  #define real34ToUInt32(source)                                 decQuadToUInt32          ((real34_t *)(source), &ctxtReal34, DEC_ROUND_DOWN)
  #define real34Reduce(source, destination)                      decQuadReduce            (destination, source, &ctxtReal34)
  #define real34Zero(destination)                                decQuadZero              (destination)
  //#define real34Zero(destination)                                xcopy                    (destination, const34_0, REAL34_SIZE_IN_BYTES)
  /*#define real34Zero(destination)                                do { *(uint64_t *)(destination)     =   *(uint64_t *)const34_0;     \
                                                                    *(((uint64_t *)(destination))+1) = *(((uint64_t *)const34_0)+1); \
                                                                 } while(0)*/
  #define stringToReal34(source, destination)                    decQuadFromString        ((real34_t *)(destination), source, &ctxtReal34)
  #define uInt32ToReal34(source, destination)                    decQuadFromUInt32        ((real34_t *)(destination), source)





  #define int32ToReal(source, destination)                       decNumberFromInt32       (destination, source)
  #define realAdd(operand1, operand2, res, ctxt)                 decNumberAdd             (res, operand1, operand2, ctxt)
  #define realChangeSign(operand)                                (operand)->bits ^= 0x80
  #define realCompare(operand1, operand2, res, ctxt)             decNumberCompare         (res, operand1, operand2, ctxt)
  #define realCopy(source, destination)                          decNumberCopy            (destination, source)
  #define realCopyAbs(source, destination)                       decNumberCopyAbs         (destination, source)
  #define realDivide(operand1, operand2, res, ctxt)              decNumberDivide          (res, operand1, operand2, ctxt)
  #define realDivideBy2(operand, ctxt)                           decNumberDivide          (operand, operand, const_2, ctxt)
  #define realDivideRemainder(operand1, operand2, res, ctxt)     decNumberRemainder       (res, operand1, operand2, ctxt)
  #define realFMA(factor1, factor2, term, res, ctxt)             decNumberFMA             (res, factor1,  factor2,  term, ctxt)
  #define realGetCoefficient(source, destination)                decNumberGetBCD          (source, (uint8_t *)(destination))
  //#define realGetExponent(source)                                ((source)->digits)
  #define realGetExponent(source)                                ((source)->digits + (source)->exponent - 1)
  #define realGetSign(source)                                    (((source)->bits) & 0x80) // 0x80=negative and 0x00=positive
  #define realIsInfinite(source)                                 decNumberIsInfinite      (source)
  #define realIsNaN(source)                                      decNumberIsNaN           (source)
  #define realIsNegative(source)                                 ((((source)->bits) & 0x80) == 0x80)
  #define realIsPositive(source)                                 ((((source)->bits) & 0x80) == 0x00)
  #define realIsSpecial(source)                                  decNumberIsSpecial       (source)
  #define realIsZero(source)                                     decNumberIsZero          (source)
  #define realMinus(operand, res, ctxt)                          decNumberMinus           (res, operand, ctxt)
  #define realMultiply(operand1, operand2, res, ctxt)            decNumberMultiply        (res, operand1, operand2, ctxt)
  #define realNextToward(from, toward, res, ctxt)                decNumberNextToward      (res, from,     toward,   ctxt)
  #define realPlus(operand, res, ctxt)                           decNumberPlus            (res, operand, ctxt)
  #define realPower(operand1, operand2, res, ctxt)               decNumberPower           (res, operand1, operand2, ctxt)
  #define realRescale(operand, res, acc, ctxt)                   decNumberRescale         (res, operand, acc, ctxt)
  #define realSetNegativeSign(operand)                           (operand)->bits |= 0x80
  #define realSetPositiveSign(operand)                           (operand)->bits &= 0x7F
  #define realSquareRoot(operand, res, ctxt)                     decNumberSquareRoot      (res, operand, ctxt)
  #define realSubtract(operand1, operand2, res, ctxt)            decNumberSubtract        (res, operand1, operand2, ctxt)
  #define realToReal34(source, destination)                      decQuadFromNumber        ((real34_t *)(destination), source, &ctxtReal34)
  #define realToString(source, destination)                      decNumberToString        ((real_t *)(source), destination)
  #define realZero(destination)                                  decNumberZero            (destination)
  #define stringToReal(source, destination, ctxt)                decNumberFromString      (destination, source, ctxt)
  #define uInt32ToReal(source, destination)                      decNumberFromUInt32      (destination, source)
#endif // !REALTYPE_H
