// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors


/********************************************//**
 * \file xeqm.c
 ***********************************************/

#include "c47.h"

void fnXSWAP (uint16_t mode) {
  #define isEdit (mode > 0)
  #define isSwap (!isEdit)

    if(calcMode == CM_EIM || calcMode == CM_AIM) {
      if(calcMode==CM_AIM) fnSwapXY(0);
      //convert X to string if needed
      int type_x = getRegisterDataType(REGISTER_X);
      if(type_x == dtString && stringByteLength(REGISTER_STRING_DATA(REGISTER_X)) >= AIM_BUFFER_LENGTH) {
        if(calcMode==CM_AIM) fnSwapXY(0);                                           //swap back before returning with nothing done
        return;
      }
      if(type_x == dtReal34 || type_x == dtComplex34 || type_x == dtLongInteger || type_x == dtShortInteger || type_x == dtTime || type_x == dtDate) {
        //Backup Y; Use Y as temp to add to X; Convert number in X to string; Restore Y; Leave X as string
        copySourceRegisterToDestRegister(REGISTER_Y, TEMP_REGISTER_1);              //Save Y to temp register
        char tmp[2];
        tmp[0] = 0;
        int16_t len = stringByteLength(tmp) + 1;
        reallocateRegister(REGISTER_Y, dtString, TO_BLOCKS(len), amNone);           //Make blank string in Y
        xcopy(REGISTER_STRING_DATA(REGISTER_Y), tmp, len);
        addition[type_x][getRegisterDataType(REGISTER_Y)]();                        //Convert X (number) to string in X
        adjustResult(REGISTER_X, false, false, -1, -1, -1);

        copySourceRegisterToDestRegister(TEMP_REGISTER_1, REGISTER_Y);              //restore Y
        clearRegister(TEMP_REGISTER_1);                                             //Clear in case it was a really long longinteger
        //resulting in a converted string in X, with Y unchanged
      }
      if(getRegisterDataType(REGISTER_X) != dtString) {                             //somehow failed to convert then return with whatever was done in X
        if(calcMode==CM_AIM) fnSwapXY(0);                                           //  This could be optimized to still restore the original X register if it had failed to convert
        return;
      }

      if(isSwap) {
        //Save aimbuffer to TEMP1 as a string register
        int16_t len = stringByteLength(aimBuffer) + 1;
        reallocateRegister(TEMP_REGISTER_1, dtString, TO_BLOCKS(len), amNone);
        xcopy(REGISTER_STRING_DATA(TEMP_REGISTER_1), aimBuffer, len);
      }
      //In essence, after conversions,
      //If X is string shorter than buffer max, copy X to aimbuffer
      //If X is no string, ignore, then aimbuffer remains unchanged.
      if(getRegisterDataType(REGISTER_X) == dtString) {
        if(stringByteLength(REGISTER_STRING_DATA(REGISTER_X)) < AIM_BUFFER_LENGTH) {
          strcpy(aimBuffer, REGISTER_STRING_DATA(REGISTER_X));

          if(isSwap) {
            //copy aimbuffer to X
            copySourceRegisterToDestRegister(TEMP_REGISTER_1, REGISTER_X);
          }
          //Set cursors
          if(calcMode==CM_AIM) {
            fnSwapXY(0);
            T_cursorPos = stringByteLength(aimBuffer);
            if(isEdit) {
              fnDrop(NOPARAM);
            }
          }
          else { //EIM
            xCursor = stringGlyphLength(aimBuffer);
          }
          refreshRegisterLine(REGISTER_X);        //make sure that the multi line editor check is done
          last_CM = 253;
          refreshScreen(64);
        }
      }
      clearRegister(TEMP_REGISTER_1);
    }

    else if(calcMode == CM_NORMAL && getRegisterDataType(REGISTER_X) == dtString) {
      if(stringByteLength(REGISTER_STRING_DATA(REGISTER_X)) < AIM_BUFFER_LENGTH) {
        if(getSystemFlag(FLAG_ERPN)) {      //JM NEWERPN
          setSystemFlag(FLAG_ASLIFT);            //JM NEWERPN OVERRIDE SLS, AS ERPN ENTER ALWAYS HAS SLS SET
        }                                        //JM NEWERPN
        strcpy(aimBuffer, REGISTER_STRING_DATA(REGISTER_X));
        T_cursorPos = stringByteLength(aimBuffer);
        fnDrop(NOPARAM);
        #if !defined(TESTSUITE_BUILD)
          resetShiftState();
          calcModeAim(NOPARAM); // Alpha Input Mode
          showSoftmenu(-MNU_ALPHA);
        #endif // !TESTSUITE_BUILD
      }
    }
    else if(calcMode == CM_NORMAL && getRegisterDataType(REGISTER_X) != dtString) {
      char line1[TMP_STR_LENGTH];
      line1[0] = 0;
      strcpy(line1, " ");
      int16_t len = stringByteLength(line1);
      if(getSystemFlag(FLAG_ERPN)) {      //JM NEWERPN
        setSystemFlag(FLAG_ASLIFT);            //JM NEWERPN OVERRIDE SLS, AS ERPN ENTER ALWAYS HAS SLS SET
      }                                        //JM NEWERPN
      liftStack();
      reallocateRegister(REGISTER_X, dtString, TO_BLOCKS(len), amNone);
      strcpy(REGISTER_STRING_DATA(REGISTER_X), line1);
      fnXSWAP(0);
    }

  last_CM = 252;
  refreshScreen(63);
  last_CM = 251;
  refreshScreen(0);
}



