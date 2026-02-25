// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"



#if defined(PC_BUILD)
  #if defined(PC_BUILD_TELLTALE)
    static char * getCalcModeName1(uint16_t cm) {
      if(cm == CM_NORMAL)                return "normal ";
      if(cm == CM_AIM)                   return "aim    ";
      if(cm == CM_EIM)                   return "eim    ";
      if(cm == CM_PEM)                   return "pem    ";
      if(cm == CM_NIM)                   return "nim    ";
      if(cm == CM_ASSIGN)                return "assign ";
      if(cm == CM_REGISTER_BROWSER)      return "reg.bro";
      if(cm == CM_ASN_BROWSER)           return "asn.bro";
      if(cm == CM_FLAG_BROWSER)          return "flg.bro";
      if(cm == CM_FONT_BROWSER)          return "fnt.bro";
      if(cm == CM_PLOT_STAT)             return "plot.st";
      if(cm == CM_GRAPH)                 return "plot.gr";
      if(cm == CM_ERROR_MESSAGE)         return "err.msg";
      if(cm == CM_BUG_ON_SCREEN)         return "bug.scr";
      if(cm == CM_MIM)                   return "mim    ";
      if(cm == CM_EIM)                   return "eim    ";
      if(cm == CM_TIMER)                 return "timer  ";
      if(cm == CM_CONFIRMATION)          return "confirm";
      if(cm == CM_LISTXY)                return "listxy ";    //JM
      return "???    ";
    }

  static  char * getAlphaCaseName1(uint16_t ac) {
      if(ac == AC_LOWER) return "lower";
      if(ac == AC_UPPER) return "upper";
      return "???  ";
    }
  #endif // PC_BUILD_TELLTALE


  void jm_show_calc_state(char comment[]) {
    #if defined(PC_BUILD_TELLTALE)
      printf("\n%s--------------------------------------------------------------------------------\n",comment);
      printf(".  calcMode: %s   last_CM=%s  AlphaCase=%s  doRefreshSoftMenu=%d    lastErrorCode=%d fnAsnDisplayUSER=%d TI=%u\n",getCalcModeName1(calcMode), getCalcModeName1(last_CM), getAlphaCaseName1(alphaCase), doRefreshSoftMenu,lastErrorCode, fnAsnDisplayUSER, temporaryInformation);
      printf(".  softmenuStack[0].softmenuId=%d      softmenu[softmenuStack[0].softmenuId].menuItem=%d -MNU_ALPHA=%d temporaryInformation=%d currentSolverStatus=%d\n",
                 softmenuStack[0].softmenuId,        softmenu[softmenuStack[0].softmenuId].menuItem,   -MNU_ALPHA,   temporaryInformation, currentSolverStatus);

      printf(".  ");
      int8_t ix=0;
      while(ix < SOFTMENU_STACK_SIZE) {
        printf("(%d)=%5d ", ix, softmenuStack[ix].softmenuId);
        ix++;
      }
      printf("\n");

      printf(".  ");
      ix=0;
      while(ix < SOFTMENU_STACK_SIZE) {
        printf("%9s ", indexOfItems[-softmenu[softmenuStack[ix].softmenuId].menuItem].itemSoftmenuName  );
        ix++;
      }
      printf("\n");
      printf(".  ");
      ix=0;
      while(ix < SOFTMENU_STACK_SIZE) {
        printf("%9s ", getCalcModeName1(softmenuStack[ix].calcMode));
        ix++;
      }
      printf("\n");

      printf(".  (tam.mode=%d, catalog=%d)   \n",
                  tam.mode,    catalog    );
      jm_show_comment("calcstate END:");
    #endif //PC_BUILD_TELLTALE
  }


  void jm_show_comment(char comment[]) {
    #if defined(PC_BUILD_VERBOSE2)
    char tmp[600];
    tmp[0]=0;
      strcat(tmp,"                                                                                                                                                                ");
      tmp[100]=0;
      printf("....%s %s calcMode=%4d last_CM=%4d tam.mode=%5d catalog=%5d Id=%4d Name=%8s f=%d g=%d \n",tmp, comment, calcMode, last_CM, tam.mode, catalog, softmenuStack[0].softmenuId, indexOfItems[-softmenu[softmenuStack[0].softmenuId].menuItem].itemSoftmenuName,shiftF,shiftG);
    #endif // PC_BUILD_VERBOSE2
    //  printf("....%s\n",tmp);
  }
#endif // PC_BUILD



/********************************************//** XXX
 * \brief Set Norm_Key_00
 *
 * \param[in] sigmaAssign uint16_t
 * \return void
 ***********************************************/
void fnSigmaAssign(uint16_t sigmaAssign) {             //DONE
  if(Norm_Key_00_key != -1) {
    int16_t tt = (int16_t)sigmaAssign;
    Norm_Key_00.func = tt - 16384;
    Norm_Key_00.funcParam[0] = 0;
    Norm_Key_00.used = Norm_Key_00.func != kbd_std[Norm_Key_00_key].primary;
    fnRefreshState();                                 //drJM
    fnClearFlag(FLAG_USER);
  } else {
    Norm_Key_00.used = false;
    displayCalcErrorMessage(ERROR_CANNOT_ASSIGN_HERE, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
    #if defined(PC_BUILD)
      moreInfoOnError("In function fnSigmaAssign: ", "the NRM key is not available.",NULL, NULL);
    #endif // PC_BUILD
  }

}



void flipPolar(void) {
  real_t zReal, zImag;
  if(getRegisterDataType(REGISTER_X) != dtComplex34) {
    if(!getRegisterAsComplex(REGISTER_X, &zReal, &zImag)) {
      return;
    }
    convertComplexToResultRegister(&zReal, &zImag, REGISTER_X);
    return;
  }

  if(getComplexRegisterPolarMode(REGISTER_X) != amPolar) {
    setComplexRegisterPolarMode(REGISTER_X, amPolar);
  } else {
    fnToRect2(NOPARAM);
//      setComplexRegisterPolarMode(REGISTER_X, ~amPolar);
  //    setComplexRegisterAngularMode(REGISTER_X, amNone);
  }
}


uint16_t nprimes = 0;
/********************************************//**
 * RPN PROGRAM.
 *
 * \param[in] JM_OPCODE uint16_t
 * \return void
 ***********************************************/
void fnJM(uint16_t JM_OPCODE) {
  #define JMTEMP    TEMP_REGISTER_1 // 98
  #define JM_TEMP_I REGISTER_I // 97
  #define JM_TEMP_J REGISTER_J // 96
  #define JM_TEMP_K REGISTER_K // 95

  #if !defined(SAVE_SPACE_DM42_6)
    if(JM_OPCODE == 6) {                                         // Delta to Star   ZYX to ZYX; destroys IJKL & JMTEMP
      saveForUndo();
      setSystemFlag(FLAG_ASLIFT);
      copySourceRegisterToDestRegister(REGISTER_X, JM_TEMP_I);   // STO I
      copySourceRegisterToDestRegister(REGISTER_Y, JM_TEMP_J);   // STO J
      copySourceRegisterToDestRegister(REGISTER_Z, JM_TEMP_K);   // STO K
      fnAdd(0);                                                  // +
      fnSwapXY(0);                                               // X<>Y

      fnAdd(0);                                                  // +
      copySourceRegisterToDestRegister(REGISTER_X, JMTEMP);      // STO JMTEMP
      fnRCL(JM_TEMP_K);                                          // RCL I
      fnRCL(JM_TEMP_J);                                          // RCL J     // z = (zx yz) / (x+y+z)
      fnMultiply(0);                                             // *
      fnSwapXY(0);                                               // X<>Y
      fnDivide(0);                                               // /

      fnRCL(JMTEMP);                                             // RCL JMTEMP
      fnRCL(JM_TEMP_I);                                          // RCL J
      fnRCL(JM_TEMP_J);                                          // RCL K     // y = (xy yz) / (x+y+z)
      fnMultiply(0);                                             // *
      fnSwapXY(0);                                               // X<>Y
      fnDivide(0);                                               // /

      fnRCL(JMTEMP);                                             // RCL JMTEMP
      fnRCL(JM_TEMP_I);                                          // RCL I
      fnRCL(JM_TEMP_K);                                          // RCL K     // z = (xy zx) / (x+y+z)
      fnMultiply(0);                                             // *
      fnSwapXY(0);                                               // X<>Y
      fnDivide(0);                                               // /

      copySourceRegisterToDestRegister(JM_TEMP_I, REGISTER_L);   // STO

      temporaryInformation = TI_ABC;

      adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
      adjustResult(REGISTER_Y, false, true, REGISTER_Y, -1, -1);
      adjustResult(REGISTER_Z, false, true, REGISTER_Z, -1, -1);
    }

    else if(JM_OPCODE == 7) {                                    // Star to Delta ZYX to ZYX; destroys IJKL & JMTEMP
      saveForUndo();
      setSystemFlag(FLAG_ASLIFT);
      copySourceRegisterToDestRegister(REGISTER_X, JM_TEMP_I);   // STO I
      copySourceRegisterToDestRegister(REGISTER_Y, JM_TEMP_J);   // STO J
      copySourceRegisterToDestRegister(REGISTER_Z, JM_TEMP_K);   // STO K

      fnMultiply(0);                          //IJ               // *
      fnSwapXY(0);
      fnRCL(JM_TEMP_I);                                          // RCL J
      fnMultiply(0);                          //IK               // *
      fnAdd(0);
      fnRCL(JM_TEMP_J);                                          // RCL J
      fnRCL(JM_TEMP_K);                                          // RCL K
      fnMultiply(0);                          //JK               // *
      fnAdd(0);
      copySourceRegisterToDestRegister(REGISTER_X, JMTEMP);      // STO JMTEMP
                                                                //
      fnRCL(JM_TEMP_J);                                          //      zx = () / y
      fnDivide(0);                                               //

      fnRCL(JMTEMP);                                             // RCL JMTEMP
      fnRCL(JM_TEMP_I);                                          //      yz = () / x
      fnDivide(0);                                               //

      fnRCL(JMTEMP);                                             // RCL JMTEMP
      fnRCL(JM_TEMP_K);                                          //      xy = () / z
      fnDivide(0);                                               //

      copySourceRegisterToDestRegister(JM_TEMP_I, REGISTER_L);   // STO

      temporaryInformation = TI_ABBCCA;
      adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
      adjustResult(REGISTER_Y, false, true, REGISTER_Y, -1, -1);
      adjustResult(REGISTER_Z, false, true, REGISTER_Z, -1, -1);
    }

    else if(JM_OPCODE == 8) {                                          //SYMMETRICAL COMP to ABC   ZYX to ZYX; destroys IJKL
      saveForUndo();
      setSystemFlag(FLAG_ASLIFT);
      copySourceRegisterToDestRegister(REGISTER_X, JM_TEMP_I);   // STO I  //A2
      copySourceRegisterToDestRegister(REGISTER_Y, JM_TEMP_J);   // STO J  //A1
      copySourceRegisterToDestRegister(REGISTER_Z, JM_TEMP_K);   // STO K  //A0
      fnAdd(0);                                                  // +
      fnAdd(0);                                                  // + Va = Vao + Va1 +Va2

      setSystemFlag(FLAG_ASLIFT);
      //liftStack();
      fn_cnst_op_a(0);
      fnRCL(JM_TEMP_I);                                          // A2
      fnMultiply(0);                                             // * a
      setSystemFlag(FLAG_ASLIFT);
      //liftStack();
      fn_cnst_op_aa(0);
      fnRCL(JM_TEMP_J);                                          // A1
      fnMultiply(0);                                             // * aa
      fnAdd(0);                                                  // +
      fnRCL(JM_TEMP_K);                                          // A0
      fnAdd(0);                                                  // + Vb = Vao + aaVa1 +aVa2

      setSystemFlag(FLAG_ASLIFT);
      //liftStack();
      fn_cnst_op_aa(0);
      fnRCL(JM_TEMP_I);                                          // A2
      fnMultiply(0);                                             // * a
      setSystemFlag(FLAG_ASLIFT);
      //liftStack();
      fn_cnst_op_a(0);
      fnRCL(JM_TEMP_J);                                          // A1
      fnMultiply(0);                                             // * aa
      fnAdd(0);                                                  // +
      fnRCL(JM_TEMP_K);                                          // A0
      fnAdd(0);                                                  // + Vb = Vao + aaVa1 +aVa2

      copySourceRegisterToDestRegister(JM_TEMP_I, REGISTER_L);  // STO

      temporaryInformation = TI_ABC;
    }

    else if(JM_OPCODE == 9) {                                   //ABC to SYMMETRICAL COMP   ZYX to ZYX; destroys IJKL & JMTEMP
      saveForUndo();
      setSystemFlag(FLAG_ASLIFT);
      copySourceRegisterToDestRegister(REGISTER_X, JM_TEMP_I);  // STO I  //c
      copySourceRegisterToDestRegister(REGISTER_Y, JM_TEMP_J);  // STO J  //b
      copySourceRegisterToDestRegister(REGISTER_Z, JM_TEMP_K);  // STO K  //a
      fnAdd(0);                                                 // +
      fnAdd(0);                                                 // + Va0 = (Va + Vb +Vc)/3
      liftStack();
      reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
      stringToReal34("3", REGISTER_REAL34_DATA(REGISTER_X));
      stringToReal34("0", REGISTER_IMAG34_DATA(REGISTER_X));
      copySourceRegisterToDestRegister(REGISTER_X, JMTEMP);     // STO
      fnDivide(0);

      setSystemFlag(FLAG_ASLIFT);
      //liftStack();
      fn_cnst_op_a(0);
      fnRCL(JM_TEMP_J);                                         // VB
      fnMultiply(0);                                            // * a
      setSystemFlag(FLAG_ASLIFT);
      //liftStack();
      fn_cnst_op_aa(0);
      fnRCL(JM_TEMP_I);                                         // VC
      fnMultiply(0);                                            // * aa
      fnAdd(0);                                                 // +
      fnRCL(JM_TEMP_K);                                         // VA
      fnAdd(0);                                                 // + V1 = (VA +aVb +aaVc) /3
      fnRCL(JMTEMP);                                            // 3
      fnDivide(0);                                              // /

      setSystemFlag(FLAG_ASLIFT);
      //liftStack();
      fn_cnst_op_aa(0);
      fnRCL(JM_TEMP_J);                                         // VB
      fnMultiply(0);                                            // * a
      setSystemFlag(FLAG_ASLIFT);
      //liftStack();
      fn_cnst_op_a(0);
      fnRCL(JM_TEMP_I);                                         // VC
      fnMultiply(0);                                            // * aa
      fnAdd(0);                                                 // +
      fnRCL(JM_TEMP_K);                                         // VA
      fnAdd(0);                                                 // + V1 = (VA +aVb +aaVc) /3
      fnRCL(JMTEMP);                                            // 3
      fnDivide(0);                                              // /

      copySourceRegisterToDestRegister(JM_TEMP_I, REGISTER_L);  // STO

      temporaryInformation = TI_012;
    }


    #define TripleRegZ1_96 96  //old:rr90
    #define TripleRegZ2_97 97  //old:rr91
    #define TripleRegZ3_98 98  //old:rr92
    #define TripleRegV1_90 90  //old:rr93
    #define TripleRegV2_91 91  //old:rr94
    #define TripleRegV3_92 92  //old:rr95
    #define TripleRegI1_93 93  //old:rr96
    #define TripleRegI2_94 94  //old:rr97
    #define TripleRegI3_95 95  //old:rr98


    else if(JM_OPCODE == 17) {                                  // V/I
      saveForUndo();
      fnRCL(TripleRegV3_92);  //rr95
      fnRCL(TripleRegI3_95);  //rr98
      fnDivide(0);
      fnRCL(TripleRegV2_91);  //rr94
      fnRCL(TripleRegI2_94);  //rr97
      fnDivide(0);
      fnRCL(TripleRegV1_90);  //rr93
      fnRCL(TripleRegI1_93);  //rr96
      fnDivide(0);
      fn3Sto(TripleRegZ1_96);
    }
    else if(JM_OPCODE == 18) {                                  // IZ
      saveForUndo();
      fnRCL(TripleRegI3_95);  //rr98
      fnRCL(TripleRegZ3_98);  //rr92
      fnMultiply(0);
      fnRCL(TripleRegI2_94);  //rr97
      fnRCL(TripleRegZ2_97);  //rr91
      fnMultiply(0);
      fnRCL(TripleRegI1_93);  //rr96
      fnRCL(TripleRegZ1_96);  //rr91
      fnMultiply(0);
      fn3Sto(TripleRegV1_90);
    }
    else if(JM_OPCODE == 19) {                                  // V/Z
      saveForUndo();
      fnRCL(TripleRegV3_92);  //rr95
      fnRCL(TripleRegZ3_98);  //rr92
      fnDivide(0);
      fnRCL(TripleRegV2_91);  //rr94
      fnRCL(TripleRegZ2_97);  //rr91
      fnDivide(0);
      fnRCL(TripleRegV1_90);  //rr93
      fnRCL(TripleRegZ1_96);  //rr90
      fnDivide(0);
      fn3Sto(TripleRegI1_93);
    }

    else if(JM_OPCODE == 20) {                                  //Copy Create X>ABC
      saveForUndo();
      setSystemFlag(FLAG_ASLIFT);
      copySourceRegisterToDestRegister(REGISTER_X, JM_TEMP_I);

      fnRCL(JM_TEMP_I);                                         //
      setSystemFlag(FLAG_ASLIFT);
      //liftStack();
      fn_cnst_op_a(0);
      fnMultiply(0);

      fnRCL(JM_TEMP_I);                                         //
      setSystemFlag(FLAG_ASLIFT);
      //liftStack();
      fn_cnst_op_aa(0);
      copySourceRegisterToDestRegister(REGISTER_X, JM_TEMP_J);
      fnMultiply(0);

      temporaryInformation = TI_ABC;
    }

    else if(JM_OPCODE == 21) {                                  // V/Z
      saveForUndo();
      flipPolar();
      fnSwapX(REGISTER_Y);
      flipPolar();
      fnSwapX(REGISTER_Y);
      fnSwapX(REGISTER_Z);
      flipPolar();
      fnSwapX(REGISTER_Z);
    }


  #endif // !SAVE_SPACE_DM42_6
  // Item 255 is NOP
}


