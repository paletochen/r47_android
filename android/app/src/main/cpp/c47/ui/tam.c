// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"

#if !defined(TESTSUITE_BUILD)
    TO_QSPI const int16_t StoOperations[][2] = {
      {ITM_ADD,      ITM_STOADD},
      {ITM_SUB,      ITM_STOSUB},
      {ITM_MULT,     ITM_STOMULT},
      {ITM_DIV,      ITM_STODIV},
      {ITM_Max,      ITM_STOMAX},
      {ITM_Min,      ITM_STOMIN},
      {ITM_Config,   ITM_STOCFG},
      {ITM_Stack,    ITM_STOS},
      {ITM_dddEL,    ITM_STOEL},
      {ITM_dddIJ,    ITM_STOIJ},
      {ITM_dddVEL,   ITM_STOVEL},
      {ITM_dddVEL1,  ITM_STOVEL1},
      {ITM_dddVEL2,  ITM_STOVEL2},
      {ITM_dddVEL3,  ITM_STOVEL3},
      {ITM_dddIX,    ITM_INDEX}
    };

    TO_QSPI const int16_t RclOperations[][2] = {
      {ITM_ADD,      ITM_RCLADD},
      {ITM_SUB,      ITM_RCLSUB},
      {ITM_MULT,     ITM_RCLMULT},
      {ITM_DIV,      ITM_RCLDIV},
      {ITM_Max,      ITM_RCLMAX},
      {ITM_Min,      ITM_RCLMIN},
      {ITM_Config,   ITM_RCLCFG},
      {ITM_Stack,    ITM_RCLS},
      {ITM_dddEL,    ITM_RCLEL},
      {ITM_dddIJ,    ITM_RCLIJ},
      {ITM_dddVEL1,  ITM_RCLVEL1},
      {ITM_dddVEL2,  ITM_RCLVEL2},
      {ITM_dddVEL3,  ITM_RCLVEL3},
      {ITM_dddVEL,   ITM_RCLVEL}
    };

    TO_QSPI const int16_t DelitmOperations[][2] = {
      {MNU_PROGS,    ITM_DELITM_PROG},
      {MNU_MENUS,    ITM_DELITM_MENU}
    };

  int16_t tamOperation(void) {
    switch(tam.function) {
      case ITM_STO: {
        for(uint_fast8_t i = 0; i < nbrOfElements(StoOperations); i++) {
          if(tam.currentOperation == StoOperations[i][0]) {
            return StoOperations[i][1];
          }
        }
        return ITM_STO;
      }
      case ITM_RCL: {
        for(uint_fast8_t i = 0; i < nbrOfElements(RclOperations); i++) {
          if(tam.currentOperation == RclOperations[i][0]) {
            return RclOperations[i][1];
          }
        }
        return ITM_RCL;
      }
      case ITM_DELITM: {
        int16_t menu = -softmenu[softmenuStack[0].softmenuId].menuItem;
        for(uint_fast8_t i = 0; i < nbrOfElements(DelitmOperations); i++) {
          if(menu == DelitmOperations[i][0]) {
            return DelitmOperations[i][1];
          }
        }
        return ITM_DELITM;
      }
      default: {
        return tam.function;
      }
    }
  }



  static uint8_t _tamMaxDigits(int16_t max) {
    if(tam.function == ITM_GTOP) {
      return (max < 1000 ? 3 : (max < 10000 ? 4 : 5));
    }
    else {
      return (max < 10 ? 1 : (max < 100 ? 2 : (max < 1000 ? 3 : (max < 10000 ? 4 : 5))));
    }
  }


  static void _tamUpdateBuffer(void) {
    char regists[5];
    char *tbPtr = tamBuffer;
    if(tam.mode == 0) {
      return;
    }

    if(tam.mode == TM_KEY) {
      tbPtr = stringCopy(tbPtr, "KEY ");
      if(tam.keyInputFinished) {
        if(tam.keyIndirect) {
          tbPtr = stringCopy(tbPtr, STD_RIGHT_ARROW);
        }
        if(tam.keyDot) {
          tbPtr = stringCopy(tbPtr, ".");
        }
        if(tam.keyAlpha) {
          tbPtr = stringCopy(tbPtr, STD_LEFT_SINGLE_QUOTE);
          tbPtr = stringCopy(tbPtr, aimBuffer + AIM_BUFFER_LENGTH / 2);
          tbPtr = stringCopy(tbPtr, STD_RIGHT_SINGLE_QUOTE);
        }
        else {
          int16_t v = tam.key;
          for(int i = 1; i >= 0; i--) {
            tbPtr[i] = '0' + (v % 10);
            v /= 10;
          }
          tbPtr += 2;
        }
        if(tam.function == ITM_KEYX) {
          tbPtr = stringCopy(tbPtr, " XEQ ");
        }
        else {
          tbPtr = stringCopy(tbPtr, " GTO ");
        }
      }
    }
    else {
      tbPtr = stringCopy(tbPtr, indexOfItems[tamOperation()].itemCatalogName);
      tbPtr = stringCopy(tbPtr, " ");
    }

    if(tam.mode == TM_SHUFFLE) {
      // Shuffle keeps the source register number for each destination register (X, Y, Z, T) in two bits
      // consecutively, with the 'valid' bit eight above that number
      // E.g. 0000010100001110 would mean that two registers have been entered: T, Z in that order
      regists[4] = 0;
      for(int i=0;i<4;i++) {
        if((tam.value >> (i*2 + 8)) & 1) {
          uint8_t regNum = (tam.value >> (i*2)) & 3;
          regists[i] = (regNum == 3 ? 't' : 'x' + regNum);
        }
        else {
          regists[i] = '_';
        }
      }
      tbPtr = stringCopy(tbPtr, regists);
    }
    else {
      if(tam.indirect) {
        tbPtr = stringCopy(tbPtr, STD_RIGHT_ARROW);
      }
      if(tam.dot) {
        tbPtr = stringCopy(tbPtr, ".");
      }
      if(tam.alpha) {
        tbPtr = stringCopy(tbPtr, STD_LEFT_SINGLE_QUOTE);
        if(aimBuffer[0] == 0) {
          *(tbPtr++) = STD_CURSOR[0];
          *(tbPtr++) = STD_CURSOR[1];
          *(tbPtr) = 0;
        }
        else {
          insertAlphaCursor(0);
          tbPtr = stringCopy(tbPtr, tmpString);
          tbPtr = stringCopy(tbPtr, STD_RIGHT_SINGLE_QUOTE);
        }
      }
      else {
        int16_t max = (tam.indirect ? (tam.dot ? (calcMode == CM_PEM ? 98 : currentNumberOfLocalRegisters-1) : 99)
          : (tam.dot ? (calcMode == CM_PEM ? 98 : ((tam.mode == TM_FLAGR || tam.mode == TM_FLAGW) ? NUMBER_OF_LOCAL_FLAGS - 1 : currentNumberOfLocalRegisters-1)) : tam.max));
        uint8_t maxDigits = _tamMaxDigits(max);
        uint8_t underscores = maxDigits - tam.digitsSoFar;
        int16_t v = tam.value;
        for(int i = tam.digitsSoFar - 1; i >= 0; i--) {
          tbPtr[i] = '0' + (v % 10);
          v /= 10;
        }
        tbPtr += tam.digitsSoFar;
        for(int i = 0; i < underscores; i++) {
          tbPtr[0] = '_';
          tbPtr++;
        }
      }
    }

    tbPtr[0] = 0;
  }



  static void _tamHandleShuffle(uint16_t item) {
    // Shuffle keeps the source register number for each destination register (X, Y, Z, T) in two bits
    // consecutively, with the 'valid' bit eight above that number
    // E.g. 0000010100001110 would mean that two registers have been entered: T, Z in that order
    switch(item) {
      case ITM_REG_X:
      case ITM_REG_Y:
      case ITM_REG_Z:
      case ITM_REG_T: {
        for(int i=0; i<4; i++) {
          if(!((tam.value >> (2*i + 8)) & 1)) {
            uint16_t mask = 3 << (2*i);
            tam.value |= 1 << (2*i + 8);
            tam.value = (tam.value & ~mask) | (((item-ITM_REG_X) << (2*i)) & mask);
            if(i == 3) {
              if(calcMode == CM_PEM) {
                addStepInProgram(tamOperation());
              }
              else {
                reallyRunFunction(tamOperation(), tam.value);
              }
              leaveTamModeIfEnabled();
            }
            break;
          }
        }
        break;
      }
      case ITM_BACKSPACE: {
        // We won't have all four registers at this point as otherwise TAM would already be closed
        for(int i=3; i>=0; i--) {
          if((tam.value >> (2*i + 8)) & 1) {
            tam.value &= ~(1 << (2*i + 8));
            break;
          }
          else if(i == 0) {
            leaveTamModeIfEnabled();
            scrollPemBackwards();
            break;
          }
        }
        break;
      }
    }
  }


  static void _tamProcessInput(uint16_t item) {
    int16_t min, max, min2, max2, dupNum;
    bool_t forceTry = false, tryOoR = false;
    bool_t valueParameter = (tam.function == ITM_GTOP || isFunctionOldParam16(tam.function) || tam.function == ITM_SKIP || tam.function == ITM_BACK);
    char *forcedVar = NULL;

    //printf("**[DL]** _tamProcessInput item %d tam.mode %d\n",item,tam.mode);fflush(stdout);
    // Shuffle is handled completely differently to everything else
    if(tam.mode == TM_SHUFFLE) {
      _tamHandleShuffle(item);
      return;
    }

    min = (tam.dot ? 0 : tam.min);
    max = (tam.dot ? ((tam.mode == TM_FLAGR || tam.mode == TM_FLAGW) ? NUMBER_OF_LOCAL_FLAGS - 1 : (calcMode == CM_PEM ? 98 : currentNumberOfLocalRegisters-1)) : tam.max);
    min2 = (tam.indirect ? 0 : min);
    max2 = (tam.indirect ? (tam.dot ? (calcMode == CM_PEM ? 98 : currentNumberOfLocalRegisters-1) : 99) : max);
    dupNum = 0;
    if((item == ITM_ENTER && !(tam.function == ITM_toINT || tam.function == ITM_HASH_JM)) || (tam.alpha && stringGlyphLength(aimBuffer) > (tam.mode != TM_MENU ? 6 : 8))) {
      forceTry = true;
      if(tam.alpha && calcMode == CM_ASSIGN) {
        assignLeaveAlpha();
        if(itemToBeAssigned == 0) {
          assignGetName1();
        }
        else {
          assignGetName2();
        }
        return;
      }
    }
    else if(item == ITM_BACKSPACE) {
      if(tam.alpha) {
        if(stringByteLength(aimBuffer) != 0) {
          // Delete the character before the cursor
          if(alphaCursor > 0) {
            deleteAlphaCharacter(&alphaCursor);
          }
        }
        else if(tam.mode == TM_NEWMENU) {
          leaveTamModeIfEnabled();
          runFunction(ITM_ASSIGN);
        }
        else {
          // backspaces within AIM are handled by addItemToBuffer, so this is if the aimBuffer is already empty
          tam.alpha = false;
          clearSystemFlag(FLAG_ALPHA);
          popSoftmenu();                     // pop current menu : either MNU_TAMALPHA or an Alpha submenu
          --numberOfTamMenusToPop;
          if(menu(0) == -MNU_TAMALPHA) {     // if back to MNU_TAMALPHA, pop it also to complete exit from tam.alpha
            popSoftmenu();
            --numberOfTamMenusToPop;
          }
          if(calcMode == CM_ASSIGN) {
            leaveTamModeIfEnabled();
            calcModeNormalGui();
          }
          else {
            calcModeTamGui();
          }
        }
      }
      else if(tam.digitsSoFar > 0) {
        if(tam.function == ITM_GTOP && tam.digitsSoFar == 3) {
          max2 = tam.max = max(getNumberOfSteps(), 99);
        }
        if(--tam.digitsSoFar != 0) {
          tam.value /= 10;
        }
        else {
          tam.value = 0;
        }
      }
      else if(tam.function == ITM_GTOP) {
        tam.function = ITM_GTO;
        tam.min = indexOfItems[ITM_GTO].tamMinMax >> TAM_MAX_BITS;
        tam.max = indexOfItems[ITM_GTO].tamMinMax & TAM_MAX_MASK;
      }
      else if(tam.dot) {
        tam.dot = false;
      }
      else if(tam.indirect) {
        tam.indirect = false;
        popSoftmenu();
        if(tam.mode == TM_VALUE || tam.mode == TM_VALUE_CHB) {
          if(tam.function == ITM_DENMAX2) {
            showSoftmenu(-MNU_TAMNONREGMAX);
          }
          else {
            showSoftmenu(-MNU_TAMNONREG);
          }
        }
        else if(tam.mode == TM_REGISTER || tam.mode == TM_M_DIM) {
          showSoftmenu(-MNU_TAM);
        }
        else if(tam.mode == TM_VARONLY) {
          showSoftmenu(-MNU_TAMVARONLY);
        }
        else if(tam.mode == TM_FLAGR || tam.mode == TM_FLAGW) {
          showSoftmenu(-MNU_TAMFLAG);
        }
        else if(tam.mode == TM_STORCL) {
          showSoftmenu(item == ITM_STO ? (currentMenu() == -MNU_TVM ? -MNU_TAMSTO_TVM : -MNU_TAMSTO) : (currentMenu() == -MNU_TVM ? -MNU_TAMRCL_TVM : -MNU_TAMRCL)); // -MNU_TAMSTORCL);
        }
        else if(tam.mode == TM_LABEL || (tam.mode == TM_KEY && tam.keyInputFinished)) {
          showSoftmenu(-MNU_TAMLABEL);
        }
        else if(tam.mode == TM_LBLONLY || (tam.mode == TM_KEY && tam.keyInputFinished)) {
          showSoftmenu(-MNU_TAMLBLONLY);
        }
        else if(tam.mode == TM_SOLVE) {
          if(tam.function == ITM_SOLVE && calcMode == CM_PEM) {
            showSoftmenu(-MNU_TAMVARONLY);
          }
          else {
            showSoftmenu(-MNU_TAMLBLONLY);
          }
        }
        else if(tam.mode == TM_MENU) {
          showSoftmenu(-MNU_TAMMENU);
        }
        else if(tam.mode == TM_CMP) {
          showSoftmenu(-MNU_TAMCMP);
        }
        --numberOfTamMenusToPop;
      }
      else if(tam.currentOperation != tam.function) {
        tam.currentOperation = tam.function;
      }
      else if(tam.mode == TM_KEY && tam.keyInputFinished) {
        tam.value            = tam.key / 10;
        tam.alpha            = tam.keyAlpha;
        tam.dot              = tam.keyDot;
        tam.indirect         = tam.keyIndirect;
        tam.keyInputFinished = false;
        xcopy(aimBuffer, aimBuffer + AIM_BUFFER_LENGTH / 2, 16);
        aimBuffer[0]    = 0;
        tam.key         = 0;
        tam.keyAlpha    = false;
        tam.keyDot      = false;
        tam.keyIndirect = false;
        tam.max         = 21;
        tam.min         = 1;
        tam.digitsSoFar = 1;
        popSoftmenu();
        showSoftmenu(-MNU_TAM); //probably best to leave this fallback as TAM not TAMVARONLY
        --numberOfTamMenusToPop;
        if(tam.alpha) {
          setSystemFlag(FLAG_ALPHA);
          calcModeAim(NOPARAM);
        }
        calcModeTamGui();
      }
      else {
        leaveTamModeIfEnabled();
        scrollPemBackwards();
        if(calcMode == CM_ASSIGN) {
          calcMode = CM_NORMAL;
        }
      }
      return;
    }
    else if(item == MNU_DYNAMIC) {
      forcedVar = dynmenuGetLabelWithDup(dynamicMenuItem, &dupNum);
      if(forcedVar[0] == 0) {
        forcedVar = NULL;
      }
      forceTry = true;
    }
    else if(tam.alpha) {
      // Do nothing if it wasn't enter or backspace as the text input is handled elsewhere
      return;
    }
    else if(!(tam.function == ITM_toINT || tam.function == ITM_HASH_JM) && item == ITM_alpha) {
      bool_t allowAlphaMode = false, beginWithLowercase = false;
      allowAlphaMode = allowAlphaMode || (!tam.digitsSoFar && !tam.dot && !valueParameter && (tam.mode == TM_STORCL || tam.mode == TM_M_DIM || tam.mode == TM_REGISTER || tam.mode == TM_VARONLY || tam.mode == TM_CMP || tam.function == ITM_MVAR));
      allowAlphaMode = allowAlphaMode || (!tam.digitsSoFar && !tam.dot && tam.indirect);
      allowAlphaMode = allowAlphaMode || (!tam.digitsSoFar && !tam.dot && tam.mode == TM_SOLVE && calcMode == CM_PEM);
      beginWithLowercase = allowAlphaMode;
      allowAlphaMode = allowAlphaMode || (!tam.digitsSoFar && !tam.dot && (tam.mode == TM_LABEL || tam.mode == TM_LBLONLY || tam.mode == TM_MENU));
      allowAlphaMode = allowAlphaMode || (!tam.digitsSoFar && !tam.dot && tam.keyInputFinished && tam.mode == TM_KEY);
      allowAlphaMode = allowAlphaMode || (!tam.digitsSoFar && (tam.function == ITM_LBL || tam.function == ITM_GTOP));
      if(allowAlphaMode) {
        tam.alpha = true;
        setSystemFlag(FLAG_ALPHA);
        aimBuffer[0] = 0;
        alphaCursor = 0;
        calcModeAim(NOPARAM);
        if(beginWithLowercase) {
          alphaCase = CAPS_STOetc_DEFAULT;
        }
        else {
          alphaCase = CAPS_TAMother_DEFAULT;
        }
        switch(softmenu[softmenuStack[0].softmenuId].menuItem) {
          case -MNU_TAMCMP      :
          case -MNU_TAMLABEL    :
          case -MNU_TAMLBLONLY  :
          case -MNU_TAM         :
          case -MNU_TAMVARONLY  :
          case -MNU_TAMSTO      :
          case -MNU_TAMRCL      :
          case -MNU_TAMSTO_TVM  :
          case -MNU_TAMRCL_TVM  :
          case -MNU_TAMMENU     :
          case -MNU_TAMINDIRECT :
            showSoftmenu(-MNU_TAMALPHA);
            screenUpdatingMode = SCRUPD_AUTO;
            break;
          default: break;
        }
      }
      return;
    }
    else if(!tam.digitsSoFar && !tam.indirect && tam.mode == TM_FLAGW && (item == ITM_BCD || item == ITM_TOPHEX || item == ITM_CB_LEADING_ZERO || item == ITM_OVERFLOW || item == ITM_CARRY)) {
      if(tam.mode) {
        leaveTamModeIfEnabled();
      }
      hourGlassIconEnabled = false;
      return;
    }
    else if(item==ITM_Max    || item==ITM_Min   || 
            item==ITM_ADD    || item==ITM_SUB   || item==ITM_MULT  || item==ITM_DIV || 
            item==ITM_Config || item==ITM_Stack || item==ITM_dddEL || item==ITM_dddIJ || 
            item == ITM_dddVEL || item == ITM_dddIX || (item >= ITM_dddVEL1 && item <= ITM_dddVEL3)) { // Operation
      if(!tam.digitsSoFar && !tam.indirect) {
        if(tam.function == ITM_GTO) {
          if(item == ITM_Max) { // UP
            if(currentLocalStepNumber == 1) { // We are on 1st step of current program
              if(currentProgramNumber == 1) { // It's the 1st program in memory
                leaveTamModeIfEnabled();      // Nothing to do
                return;
              }
              else { // It isn't the 1st program in memory
                tam.value = programList[currentProgramNumber - 2].step;
              }
            }
            else { // We aren't on 1st step of current program
              tam.value = programList[currentProgramNumber - 1].step;
            }
            reallyRunFunction(ITM_GTOP, tam.value);
            pemCursorIsZerothStep = true;
            leaveTamModeIfEnabled();
            hourGlassIconEnabled = false;
            return;
          }

          if(item == ITM_Min) { // DOWN
            if(currentProgramNumber == numberOfPrograms) { // We are in the last program in memory
              tam.value = programList[currentProgramNumber - 1].step;
              reallyRunFunction(ITM_GTOP, tam.value);      // Go to the first step of the program
              reallyRunFunction(ITM_BST, NOPARAM);         // BST to go to .END.
            }
            else {
              tam.value = programList[currentProgramNumber].step;
              reallyRunFunction(ITM_GTOP, tam.value);
              pemCursorIsZerothStep = true;
            }
            leaveTamModeIfEnabled();
            hourGlassIconEnabled = false;
            return;
          }
        }
        else if(tam.mode == TM_STORCL && tam.currentOperation != ITM_Config && tam.currentOperation != ITM_Stack) {
          if(item == tam.currentOperation) {
            tam.currentOperation = tam.function;
          }

          else if((item >= ITM_STOVEL1 && item <= ITM_STOVEL3) || (item >= ITM_RCLVEL1 && item <= ITM_RCLVEL3)) {
            tam.currentOperation = item;
            switch(calcMode) {
              case CM_MIM: {
                break;
              }
              case CM_PEM: {
                addStepInProgram(item);
                break;
              }
              default: {
                runFunction(item);
              }
            }
            leaveTamModeIfEnabled();
            hourGlassIconEnabled = false;
            return;
          }


          else if(item == ITM_dddVEL || (item >= ITM_dddVEL1 && item <= ITM_dddVEL3) || item == ITM_dddIX) {
            tam.currentOperation = item;
            if(calcMode != CM_MIM
//                && !tam.alpha && !tam.dot
//                && (indexOfItems[tam.function].status & PTP_STATUS) != PTP_SKIP_BACK && (indexOfItems[tam.function].status & PTP_STATUS) != PTP_DECLARE_LABEL
              ) {
              leaveTamModeIfEnabled();
              runFunction(tamOperation());
            }
            return;
          }


          else {
            tam.currentOperation = item;
            if(item == ITM_dddEL || item == ITM_dddIJ) {
              switch(calcMode) {
                case CM_MIM: {
                  mimRunFunction(tamOperation(), NOPARAM);
                  break;
                }
                case CM_PEM: {
                  addStepInProgram(tamOperation());
                  break;
                }
                default: {
                  reallyRunFunction(tamOperation(), NOPARAM);
                }
              }
              leaveTamModeIfEnabled();
              hourGlassIconEnabled = false;
              return;
            }
          }
        }
      }
      return;
    }
    else if((tam.function == ITM_toINT || tam.function == ITM_HASH_JM) && item == ITM_REG_I) {        //    vvvvvv    JM BASE: These are the shortcuts NORMAL MODE
      if(calcMode == CM_PEM) {
        addStepInProgram(ITM_IP);
      }
      else {
        saveForUndo();
        //fnIp(NOPARAM);     // non-round --> retain data type
        fnJM_2SI(NOPARAM);   // round  Real -> LI; LI->SI; SI->LI;
        fnLint(NOPARAM);     // change to long integer output
      }
      leaveTamModeIfEnabled();
      return;
    }
    else if((tam.function == ITM_toINT || tam.function == ITM_HASH_JM)  && ((item == ITM_alpha && calcModel == USER_C47) || (item == ITM_REG_F && isR47FAM))) {
      if(calcMode == CM_PEM) {
        addStepInProgram(ITM_FP);
      }
      else {
        saveForUndo();
        fnFp(NOPARAM);       // retain data type
        fnToReal(NOPARAM);   // change to real fp output
      }
      leaveTamModeIfEnabled();
      return;
    }
    else if((tam.function == ITM_toINT || tam.function == ITM_HASH_JM) && (item == ITM_REG_D || item == ITM_ENTER)) {   //ENTER gives base 10
      tam.value = 10;
      forceTry = true;
    }
    else if((tam.function == ITM_toINT  || tam.function == ITM_HASH_JM) && item == ITM_REG_B) {
      tam.value = 2;
      forceTry = true;
    }
    else if((tam.function == ITM_toINT  || tam.function == ITM_HASH_JM) && item == ITM_REG_H) {
      tam.value = 16;
      forceTry = true;
    }
    else if((tam.function == ITM_toINT  || tam.function == ITM_HASH_JM) && item == ITM_REG_O) {     //JM BASE added OCT (R47 has O on 8, so althopugh this key will not work in R47, it does the same as "8")
      tam.value = 8;
      forceTry = true;
    }
    else if((tam.function == ITM_toINT  || tam.function == ITM_HASH_JM) && item == ITM_REG_I) {     //JM BASE --> Long Integer
      tam.value = 8;
      forceTry = true;
    }
                                                                                                      //    ^^^^^^    JM BASE: These are the shortcuts NORMAL MODE

    else if((tam.mode == TM_LABEL || tam.mode == TM_LBLONLY || (tam.mode == TM_KEY && tam.keyInputFinished)) && !tam.indirect && ITM_a <= item && item <= ITM_l ) {
      tam.value = FIRST_LC_LOCAL_LABEL + item - ITM_a;
      forceTry = true;
      tryOoR = true;
    }

    else if( ((REGISTER_X <= indexOfItems[item].param && indexOfItems[item].param <= REGISTER_W) ||
              (FIRST_NAMED_RESERVED_VARIABLE <= indexOfItems[item].param && indexOfItems[item].param <= LAST_RESERVED_VARIABLE)) &&
              !tam.dot) {
      if(!tam.digitsSoFar && !isFunctionOldParam16(tam.function) && (tam.indirect || (tam.mode != TM_VALUE && tam.mode != TM_VALUE_CHB))) {
        if((tam.mode == TM_LABEL || tam.mode == TM_LBLONLY || (tam.mode == TM_KEY && tam.keyInputFinished)) && !tam.indirect) {
          #define LOCAL_LABEL 0            // Local label from A to J
          #define ALPHA_LABEL 1            // Global single letters alpha labels
          static TO_QSPI const int16_t registerLookup[REGISTER_W - FIRST_LETTERED_REGISTER + 1][2] = {
            [REGISTER_X - FIRST_LETTERED_REGISTER] = {'X', ALPHA_LABEL},
            [REGISTER_Y - FIRST_LETTERED_REGISTER] = {'Y', ALPHA_LABEL},
            [REGISTER_Z - FIRST_LETTERED_REGISTER] = {'Z', ALPHA_LABEL},
            [REGISTER_T - FIRST_LETTERED_REGISTER] = {'T', ALPHA_LABEL},
            [REGISTER_A - FIRST_LETTERED_REGISTER] = {'A', LOCAL_LABEL},
            [REGISTER_B - FIRST_LETTERED_REGISTER] = {'B', LOCAL_LABEL},
            [REGISTER_C - FIRST_LETTERED_REGISTER] = {'C', LOCAL_LABEL},
            [REGISTER_D - FIRST_LETTERED_REGISTER] = {'D', LOCAL_LABEL},
            [REGISTER_L - FIRST_LETTERED_REGISTER] = {'L', LOCAL_LABEL},
            [REGISTER_I - FIRST_LETTERED_REGISTER] = {'I', LOCAL_LABEL},
            [REGISTER_J - FIRST_LETTERED_REGISTER] = {'J', LOCAL_LABEL},
            [REGISTER_K - FIRST_LETTERED_REGISTER] = {'K', LOCAL_LABEL},
            [REGISTER_M - FIRST_LETTERED_REGISTER] = {'M', ALPHA_LABEL},
            [REGISTER_N - FIRST_LETTERED_REGISTER] = {'N', ALPHA_LABEL},
            [REGISTER_P - FIRST_LETTERED_REGISTER] = {'P', ALPHA_LABEL},
            [REGISTER_Q - FIRST_LETTERED_REGISTER] = {'Q', ALPHA_LABEL},
            [REGISTER_R - FIRST_LETTERED_REGISTER] = {'R', ALPHA_LABEL},
            [REGISTER_S - FIRST_LETTERED_REGISTER] = {'S', ALPHA_LABEL},
            [REGISTER_E - FIRST_LETTERED_REGISTER] = {'E', LOCAL_LABEL},
            [REGISTER_F - FIRST_LETTERED_REGISTER] = {'F', LOCAL_LABEL},
            [REGISTER_G - FIRST_LETTERED_REGISTER] = {'G', LOCAL_LABEL},
            [REGISTER_H - FIRST_LETTERED_REGISTER] = {'H', LOCAL_LABEL},
            [REGISTER_O - FIRST_LETTERED_REGISTER] = {'O', ALPHA_LABEL},
            [REGISTER_U - FIRST_LETTERED_REGISTER] = {'U', ALPHA_LABEL},
            [REGISTER_V - FIRST_LETTERED_REGISTER] = {'V', ALPHA_LABEL},
            [REGISTER_W - FIRST_LETTERED_REGISTER] = {'W', ALPHA_LABEL}
          };

          int param = indexOfItems[item].param;
          if(registerLookup[param - FIRST_LETTERED_REGISTER][1] == ALPHA_LABEL) {
            tam.alpha = true;
            aimBuffer[0] = registerLookup[param - FIRST_LETTERED_REGISTER][0];
            aimBuffer[1] = 0;
            forceTry = true;
          }
          else {
            tam.value = FIRST_UC_LOCAL_LABEL - 'A' + registerLookup[param - FIRST_LETTERED_REGISTER][0];
            forceTry = true;
            tryOoR = true;
          }
        }
        else {
          if(calcMode == CM_PEM && indexOfItems[item].param >= FIRST_RESERVED_VARIABLE && indexOfItems[item].param <= LAST_RESERVED_VARIABLE) {
            tam.alpha = true;
            strcpy(aimBuffer, (char *)(allReservedVariables[indexOfItems[item].param - FIRST_RESERVED_VARIABLE].reservedVariableName + 1));
            forceTry = true;
          } else {
            tam.value = indexOfItems[item].param;
            tam.value += 99*(!tam.dot && (tam.mode == TM_FLAGR || tam.mode == TM_FLAGW) && FLAG_M-99 <= tam.value && tam.value <= FLAG_W-99);
printf("tam.value: %d\n",tam.value);
            forceTry = true;
            // Register letters access registers not accessible via number codes, so we shouldn't look at the tam.max value
            // when determining if this is valid
            tryOoR = true;
          }
        }
      }
    }
    else if(item == ITM_0P || item == ITM_1P) {
      reallocateRegister(TEMP_REGISTER_1, dtReal34, 0, amNone);
      real34Copy(item == ITM_1P ? const34_1 : const34_0, REGISTER_REAL34_DATA(TEMP_REGISTER_1));
      if(!tam.digitsSoFar && !isFunctionOldParam16(tam.function) && tam.mode != TM_VALUE && tam.mode != TM_VALUE_CHB) {
        tam.value = TEMP_REGISTER_1;
        forceTry = true;
        // Register letters access registers not accessible via number codes, so we shouldn't look at the tam.max value
        // when determining if this is valid
        tryOoR = true;
      }
    }
    else if(ITM_0 <= item && item <= ITM_9) {
      int16_t digit = item - ITM_0;
      uint8_t maxDigits = _tamMaxDigits(max2);
      // If the number is below our minimum, prevent further entry of digits
      if(tam.function == ITM_GTOP && tam.digitsSoFar == 2) {
        max2 = tam.max = getNumberOfSteps();
        maxDigits = _tamMaxDigits(max2);
      }
      if(!tam.alpha && (tam.value*10 + digit) <= max2 && tam.digitsSoFar < maxDigits) {
        if(tam.digitsSoFar != maxDigits - 1 || (tam.value*10 + digit) >= min2) {
          tam.value = tam.value*10 + digit;
          tam.digitsSoFar++;
          if(tam.digitsSoFar == maxDigits) {
            forceTry = true;
          }
        }
      }
      else if(tam.function == ITM_GTOP) {
        max2 = tam.max = max(getNumberOfSteps(), 99);
        maxDigits = _tamMaxDigits(max2);
      }
    }
    else if(item == ITM_PERIOD) {
      if(tam.function == ITM_LBL) {
        return;
      }
      else if(tam.function == ITM_GTOP) {
        tam.value = programList[numberOfPrograms - 1].step;
        pemCursorIsZerothStep = true;
        reallyRunFunction(ITM_GTOP, tam.value);
        if((*currentStep != 0xff) || (*(currentStep + 1) != 0xff)) {
          currentStep = firstFreeProgramByte;
          insertStepInProgram(ITM_END);
          scanLabelsAndPrograms();
          tam.value = programList[numberOfPrograms - 1].step;
          reallyRunFunction(ITM_GTOP, tam.value);
        }
        leaveTamModeIfEnabled();
        hourGlassIconEnabled = false;
        return;
      }
      else if(!tam.alpha && !tam.digitsSoFar && !tam.dot && !valueParameter) {
        if(tam.function == ITM_GTO || tam.function == ITM_XEQ) {
          tam.function = ITM_GTOP;
          tam.min = 0;
          tam.max = max(getNumberOfSteps(), 99);
        }
        else if(tam.indirect && (currentNumberOfLocalRegisters || calcMode == CM_PEM)) {
          tam.dot = true;
        }
        else if(tam.mode != TM_VALUE && tam.mode != TM_VALUE_CHB && tam.mode != TM_LABEL && tam.mode != TM_LBLONLY && tam.mode != TM_MENU) {
          if(calcMode == CM_PEM || ((tam.mode == TM_FLAGR || tam.mode == TM_FLAGW) && currentLocalFlags != NULL) || ((tam.mode != TM_FLAGR && tam.mode != TM_FLAGW) && currentNumberOfLocalRegisters)) {
            tam.dot = true;
          }
        }
      }
      return;
    }
    else if(item == ITM_INDIRECTION) {
      if(!tam.alpha && !tam.digitsSoFar && !tam.dot && !valueParameter && (indexOfItems[tam.function].status & PTP_STATUS) != PTP_SKIP_BACK && (indexOfItems[tam.function].status & PTP_STATUS) != PTP_DECLARE_LABEL) {
        if(!tam.indirect) {
          popSoftmenu();
          showSoftmenu(-MNU_TAMINDIRECT);
          --numberOfTamMenusToPop;
        }
        tam.indirect = true;
      }
      return;
    }
    else if(indexOfItems[item].func == fnGetSystemFlag && (tam.mode == TM_FLAGR || tam.mode == TM_FLAGW)) {
      // A function key has been pressed that corresponds to a system flag
      tam.value = indexOfItems[item].param;
      tryOoR = true;
      forceTry = true;
    }
    else if(tam.mode == TM_MENU && softmenu[softmenuStack[0].softmenuId].menuItem == -MNU_MENU) {
        tam.value = item;
    }
    else {
      // Do nothing
      return;
    }

    // All operations that may try and evaluate the function shouldn't return but let execution fall through to here

    if(tam.mode == TM_KEY && !tam.keyInputFinished) {
      if(tam.alpha || forcedVar || ((tryOoR || (min2 <= tam.value && tam.value <= max2)) && (forceTry || tam.value*10 > max2))) {
        tam.key              = tam.value;
        tam.keyAlpha         = tam.alpha;
        tam.keyDot           = tam.dot;
        tam.keyIndirect      = tam.indirect;
        tam.keyInputFinished = true;
        xcopy(aimBuffer + AIM_BUFFER_LENGTH / 2, aimBuffer, 16);
        aimBuffer[0]    = 0;
        tam.value       = 0;
        tam.alpha       = false;
        tam.dot         = false;
        tam.indirect    = false;
        tam.max         = 99;
        tam.min         = 0;
        tam.digitsSoFar = 0;
        popSoftmenu();
        showSoftmenu(-MNU_TAMLABEL); // Probably better to have the fallback TAMLABEL, not TAMLBLONLY
        --numberOfTamMenusToPop;
        clearSystemFlag(FLAG_ALPHA);
        calcModeTamGui();
      }
      else if(tam.digitsSoFar == 2 && tam.value == 0) {
        tam.digitsSoFar = 1;
      }
    }
    else if(!tam.alpha && !forcedVar) {
      // Check whether it is possible to add any more digits: if not, execute the function
      if((tryOoR || (min2 <= tam.value && tam.value <= max2)) && (forceTry || tam.value*10 > max2) && ((tam.mode != TM_MENU) || tam.indirect)) {
        int16_t value = tam.value;
        bool_t  tryAllocate = isFunctionAllowingNewVariable(tam.function);
        bool_t  run = true;
        if(tam.dot) {
          value += ((tam.mode == TM_FLAGR || tam.mode == TM_FLAGW) ? FIRST_LOCAL_FLAG : FIRST_LOCAL_REGISTER);
        }
        if(tam.indirect && calcMode != CM_PEM) {
          value = indirectAddressing(value, indirectionType(tam.function), min, max, tryAllocate);
          run = (lastErrorCode == 0);
        }
        if(tam.function == ITM_GTOP) {
          if(tam.digitsSoFar < 3) {
            pemCursorIsZerothStep = false;
            fnGoto(value);
          }
          else {
            pemCursorIsZerothStep = (value == 0);
            if(value == 0) {
              value = 1;
            }
            goToPgmStep(currentProgramNumber, value);
          }
        }
        else if(run) {
          switch(calcMode) {
            case CM_MIM: {
              mimRunFunction(tamOperation(), value);
              break;
            }
            case CM_PEM: {
              addStepInProgram(tamOperation());
              break;
            }
            default: {
              if(tam.mode == TM_MENU) {                        // Leave TAM menu before opening a new menu
                leaveTamModeIfEnabled();
              }
              reallyRunFunction(tamOperation(), value);
            }
          }
        }
        if(tamOperation() == ITM_M_GOTO_ROW) {
          leaveTamModeIfEnabled();
          tamEnterMode(ITM_M_GOTO_COLUMN);
        }
        else {
          leaveTamModeIfEnabled();
        }
      }
      else if(tam.mode == TM_MENU && softmenu[softmenuStack[0].softmenuId].menuItem == -MNU_MENU) {
        int16_t value = tam.value;
        if(calcMode == CM_PEM) {
          addStepInProgram(tamOperation());
          leaveTamModeIfEnabled();
        }
        else {
          leaveTamModeIfEnabled();
          reallyRunFunction(tamOperation(), value);
        }
      }
    }
    else {
      char *buffer = (forcedVar ? forcedVar : aimBuffer);
      bool_t tryAllocate = isFunctionAllowingNewVariable(tam.function);
      int16_t value, value2;
      if(tam.mode == TM_NEWMENU) {
        value = 1;
      }
      else if(tam.mode == TM_LABEL || tam.mode == TM_LBLONLY || tam.mode == TM_SOLVE || (tam.mode == TM_KEY && tam.keyInputFinished) || (tam.mode == TM_DELITM && softmenu[softmenuStack[0].softmenuId].menuItem == -MNU_PROGS)) {
        if(!tam.indirect) {
          value = findNamedLabelWithDuplicate(buffer, dupNum);
        }
        else {
          value = findNamedVariable(buffer);
          if(calcMode != CM_PEM) {
            if(value != INVALID_VARIABLE) {
              value2 = indirectAddressing(value, indirectionType(tam.function), min, max, tryAllocate);
              buffer = REGISTER_STRING_DATA(value);
              dynamicMenuItem = -1;
              value = (value2 != FAILED_INDIRECTION ? value2 : INVALID_VARIABLE);
            }
            else {
              displayCalcErrorMessage(ERROR_UNDEF_SOURCE_VAR, ERR_REGISTER_LINE, REGISTER_X);
              #if (EXTRA_INFO_ON_CALC_ERROR == 1)
                sprintf(errorMessage, "string '%s' is not a named variable", buffer);
                moreInfoOnError("In function _tamProcessInput:", errorMessage, NULL, NULL);
              #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
            }
          }
        }
        if(value == INVALID_VARIABLE && ((tam.function == ITM_XEQ) || (tam.function == ITM_XEQP1))) {  // If no label found then look for XEQ 'function'
          if(!tam.indirect) {                                                                          //  indirection (XEQ -> 'function') not supported
            for(int i = 0; i < LAST_ITEM; ++i) {
              if((indexOfItems[i].status & CAT_STATUS) == CAT_FNCT && compareString(buffer, indexOfItems[i].itemCatalogName, CMP_NAME) == 0) { //change here to slacken the character check for commands: CMP_CLEANED_STRING_ONLY
                leaveTamModeIfEnabled();
                if(calcMode == CM_PEM) {
                  aimBuffer[0] = 0;
                  if(!programListEnd) {
                    scrollPemBackwards();
                  }
                }
                runFunction(i);
                return;
              }
            }
          }
          if(calcMode != CM_PEM) {
            leaveTamModeIfEnabled();
            if(!tam.indirect) {
              displayCalcErrorMessage(ERROR_FUNCTION_NOT_FOUND, ERR_REGISTER_LINE, REGISTER_X);
              #if (EXTRA_INFO_ON_CALC_ERROR == 1)
                sprintf(errorMessage, "string '%s' is neither a named label nor a function name", buffer);
                moreInfoOnError("In function _tamProcessInput:", errorMessage, NULL, NULL);
              #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
            }
            return;
          }
        }
        else if(value == INVALID_VARIABLE && tam.function != ITM_LBL && tam.function != ITM_LBLQ && (calcMode != CM_PEM || tam.mode != TM_SOLVE)) {
          if(calcMode != CM_PEM && getSystemFlag(FLAG_IGN1ER)) {
            clearSystemFlag(FLAG_IGN1ER);
            #if (EXTRA_INFO_ON_CALC_ERROR == 1)
              sprintf(errorMessage, "string '%s' is not a named label", buffer);
              moreInfoOnError("In function _tamProcessInput:", errorMessage, "ignored since IGN1ER was set", NULL);
            #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          }
          else if(calcMode != CM_PEM){
            displayCalcErrorMessage(ERROR_LABEL_NOT_FOUND, ERR_REGISTER_LINE, REGISTER_X);
            #if (EXTRA_INFO_ON_CALC_ERROR == 1)
              sprintf(errorMessage, "string '%s' is not a named label", buffer);
              moreInfoOnError("In function _tamProcessInput:", errorMessage, NULL, NULL);
            #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          }
        }
        else if (calcMode != CM_PEM) {
          reallyRunFunction(tamOperation(), value);
          leaveTamModeIfEnabled();
          return;
        }
      }
      else if(tam.mode == TM_DELITM && softmenu[softmenuStack[0].softmenuId].menuItem == -MNU_MENUS) {
        value = tam.value;
      }
      else if(tryAllocate && !tam.indirect) {
        value = findOrAllocateNamedVariable(buffer);
        //printf("findOrAllocateNamedVariable value=%d lastErrorCode=%d\n",value, lastErrorCode);
      }
      else if((tam.mode == TM_MENU) && !tam.indirect) {
        value = findMenu(buffer);
        tam.value = value;
        if(value == INVALID_MENU && calcMode != CM_PEM) {
          if(getSystemFlag(FLAG_IGN1ER)) {
            clearSystemFlag(FLAG_IGN1ER);
            #if (EXTRA_INFO_ON_CALC_ERROR == 1)
              sprintf(errorMessage, "string '%s' is not a menu name", buffer);
              moreInfoOnError("In function _tamProcessInput:", errorMessage, "ignored since IGN1ER system flag was set", NULL);
            #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          }
          else {
            displayCalcErrorMessage(ERROR_UNDEF_MENU, ERR_REGISTER_LINE, REGISTER_X);
            #if (EXTRA_INFO_ON_CALC_ERROR == 1)
              sprintf(errorMessage, "string '%s' is not a menu name", buffer);
              moreInfoOnError("In function _tamProcessInput:", errorMessage, NULL, NULL);
            #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          }
        }
      }
      else {
        value = findNamedVariable(buffer);
        //printf("findNamedVariable value=%d lastErrorCode=%d\n",value, lastErrorCode);
        if(value == INVALID_VARIABLE && calcMode != CM_PEM) {
          if(getSystemFlag(FLAG_IGN1ER)) {
            clearSystemFlag(FLAG_IGN1ER);
            #if (EXTRA_INFO_ON_CALC_ERROR == 1)
              sprintf(errorMessage, "string '%s' is not a named variable", buffer);
              moreInfoOnError("In function _tamProcessInput:", errorMessage, "ignored since IGN1ER system flag was set", NULL);
            #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          }
          else {
            displayCalcErrorMessage(ERROR_UNDEF_SOURCE_VAR, ERR_REGISTER_LINE, REGISTER_X);
            #if (EXTRA_INFO_ON_CALC_ERROR == 1)
              sprintf(errorMessage, "string '%s' is not a named variable", buffer);
              moreInfoOnError("In function _tamProcessInput:", errorMessage, NULL, NULL);
            #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          }
        }
      }
      if(calcMode == CM_PEM && tam.function != ITM_DELP && lastErrorCode == 0) { //do not add a step of any kind if an error occurred in the processing prior to adding the step. This solves the MVAR and STO of an identified variable name problem.
        addStepInProgram(tamOperation());
      }
      if(tam.mode != TM_NEWMENU) {
        aimBuffer[0] = 0;
      }
      if(tam.indirect && value != INVALID_VARIABLE && calcMode != CM_PEM) {
        value = indirectAddressing(value, indirectionType(tam.function), min, max, tryAllocate);
        if(lastErrorCode != 0) {
          value = INVALID_VARIABLE;
        }
      }
      if(value != INVALID_VARIABLE || tamOperation() == ITM_LBLQ) {
        if(calcMode == CM_MIM) {
          mimRunFunction(tamOperation(), value);
        }
        else if(tam.function == ITM_GTOP) {
          goToGlobalStep(labelList[value - FIRST_LABEL].step);
        }
        else if(tam.function == ITM_DELP) {
          reallyRunFunction(ITM_DELP, value);
        }
        else if(calcMode == CM_PEM) {
          // already done
        }
        else if(tam.mode == TM_MENU) {
          if(value != INVALID_MENU) {
            leaveTamModeIfEnabled();                                // Leave TAM menu before opening a new menu
            reallyRunFunction(tamOperation(), value);
          }
        }
        else {
          reallyRunFunction(tamOperation(), value);
        }
      }
      if(tamOperation() == ITM_M_GOTO_ROW) {
        leaveTamModeIfEnabled();
        tamEnterMode(ITM_M_GOTO_COLUMN);
      }
      else {
        leaveTamModeIfEnabled();
      }
    }
  }



  void tamEnterMode(int16_t func) {
    tam.mode = func == ITM_ASSIGN ? TM_LABEL : func == ITM_USERMODE ? TM_NEWMENU : indexOfItems[func].param; // TM_LABEL should be fine and TM_LBLONLY not needed here
    func = func == ITM_USERMODE ? ITM_ASSIGN : func;
    tam.function = func;
    tam.min = indexOfItems[func].tamMinMax >> TAM_MAX_BITS;
    tam.max = indexOfItems[func].tamMinMax & TAM_MAX_MASK;

    screenUpdatingMode = SCRUPD_AUTO;

    if(tam.max == 16383) { // Only case featuring more than TAM_MAX_BITS bits is GTO.
      tam.max = 32766;
    }

    if(func == ITM_CNST) {
      tam.max = LAST_CONSTANT-FIRST_CONSTANT - 1;
    }

    if(calcMode == CM_NIM) {
      if(func == ITM_toINT || func == ITM_HASH_JM) {
        lastIntegerBase = 0;
        screenUpdatingMode &= ~SCRUPD_MANUAL_STATUSBAR;
        resetShiftState();
        leaveTamModeIfEnabled();
        while(stringByteLength(aimBuffer) > 1 && strchr(aimBuffer,'#') && aimBuffer[strlen(aimBuffer) - 1] != '#') {
          addItemToNimBuffer(ITM_BACKSPACE);
        }
        addItemToNimBuffer(func);
        refreshRegisterLine(REGISTER_X);
        return;
      }
      else {
        closeNim();
      }
    }
    else if(calcMode == CM_PEM && aimBuffer[0] != 0) {
      if(getSystemFlag(FLAG_ALPHA)) {
        pemCloseAlphaInput();
      }
      else {
        pemCloseNumberInput();
      }
      aimBuffer[0] = 0;
      --currentLocalStepNumber;
      currentStep = findPreviousStep(currentStep);
    }
    else if(calcMode == CM_PEM) {
      scrollPemForwards();
    }

    if(func == ITM_ASSIGN) {
      aimBuffer[0] = 0;
    }

    tam.alpha = (func == ITM_ASSIGN);
    alphaCursor = 0;
    tam.currentOperation = tam.function;
    tam.digitsSoFar = 0;
    tam.dot = false;
    tam.indirect = false;
    tam.value = 0;

    tam.key = 0;
    tam.keyAlpha = false;
    tam.keyDot = false;
    tam.keyIndirect = false;
    tam.keyInputFinished = false;

    switch(tam.mode) {
      case TM_VALUE_MAX:                                                 //Changing over to TM_VALUE, only used to add the max button for 0
        if((func != ITM_VIEW && func != ITM_AVIEW) || !catalog || catalog != CATALOG_MVAR) {
          showSoftmenu(-MNU_TAMNONREGMAX);
        }
        tam.mode = TM_VALUE;
        break;
      case TM_VALUE_TRK:                                                 //Changing over to TM_VALUE, only used to add the track button for 9999
        if((func != ITM_VIEW && func != ITM_AVIEW) || !catalog || catalog != CATALOG_MVAR) {
          showSoftmenu(-MNU_TAMNONREGTRK);
        }
        tam.mode = TM_VALUE;
        break;
      case TM_VALUE:
      case TM_VALUE_CHB:
        if((func != ITM_VIEW && func != ITM_AVIEW) || !catalog || catalog != CATALOG_MVAR) {
          showSoftmenu(-MNU_TAMNONREG);
        }
        break;
      case TM_REGISTER:
      case TM_M_DIM:
      case TM_KEY: {
        if((func != ITM_VIEW && func != ITM_AVIEW) || !catalog || catalog != CATALOG_MVAR) {
          showSoftmenu(-MNU_TAM);
        }
        break;
      }
      case TM_VARONLY: {
        showSoftmenu(-MNU_TAMVARONLY);
        break;
      }

      case TM_CMP: {
        showSoftmenu(-MNU_TAMCMP);
        break;
      }

      case TM_FLAGR:
      case TM_FLAGW: {
        showSoftmenu(-MNU_TAMFLAG);
        break;
      }

      case TM_STORCL: {
        if(!catalog || catalog != CATALOG_MVAR) {
          showSoftmenu(func == ITM_STO ? (currentMenu() == -MNU_TVM ? -MNU_TAMSTO_TVM : -MNU_TAMSTO) : (currentMenu() == -MNU_TVM ? -MNU_TAMRCL_TVM : -MNU_TAMRCL)); // -MNU_TAMSTORCL);
        }
        break;
      }

      case TM_SHUFFLE: {
        showSoftmenu(-MNU_TAMSHUFFLE);
        break;
      }

      case TM_LABEL: {
        if(func == ITM_ASSIGN) {
          showSoftmenu(-MNU_TAMALPHA);
        }
        else {
          showSoftmenu(-MNU_TAMLABEL);
        }
        break;
      }

      case TM_LBLONLY: {
        showSoftmenu(-MNU_TAMLBLONLY);
        break;
      }

      case TM_MENU: {
        showSoftmenu(-MNU_TAMMENU);
        break;
      }

      case TM_SOLVE: {
        if(func == ITM_SOLVE && calcMode == CM_PEM) {
          showSoftmenu(-MNU_TAMVARONLY);
        }
        else {
          showSoftmenu(-MNU_TAMLBLONLY);
        }
        break;
      }

      case TM_NEWMENU: {
        showSoftmenu(-MNU_TAMALPHA);
        screenUpdatingMode &= ~SCRUPD_MANUAL_MENU;
        break;
      }

      case TM_DELITM: {
        showSoftmenu(-ITM_DELITM);
        break;
      }

      default: {
        sprintf(errorMessage, commonBugScreenMessages[bugMsgValueFor], "tamEnterMode", tam.mode, "tam.mode");
        displayBugScreen(errorMessage);
        return;
      }
    }

    numberOfTamMenusToPop = 1;
    //numberOfTamMenusToPop = (func == ITM_ASSIGN) || (catalog && catalog == CATALOG_MVAR && (tam.mode == TM_STORCL || func == ITM_VIEW)) ? 0 : 1;

    _tamUpdateBuffer();

    clearSystemFlag(FLAG_ALPHA);

    #if defined(PC_BUILD)
      if(forceTamAlpha) {
        forceTamAlpha = false;
        tamProcessInput(ITM_alpha);  // (DL] enter tam alpha for simulator easy keyboard entry
      }
    #endif // PC_BUILD

    if(tam.mode == TM_NEWMENU) {
      setSystemFlag(FLAG_ALPHA);
      aimBuffer[0] = 0;
      calcModeAim(NOPARAM);
    }
    else {
      calcModeTamGui();
    }
  }



  void leaveTamModeIfEnabled(void) {
    if(!tam.mode) return;
    if(screenUpdatingMode & (SCRUPD_MANUAL_STACK | SCRUPD_SKIP_STACK_ONE_TIME)) {
      clearTamBuffer();
    }

    tam.alpha = false;
    tam.mode = 0;
    catalog = CATALOG_NONE;
    clearSystemFlag(FLAG_ALPHA);

    if(numberOfTamMenusToPop > 0) {
      while(numberOfTamMenusToPop--) {
        popSoftmenu();
      }
    }

    #if defined(PC_BUILD)
      switch(calcMode) {
        case CM_NORMAL:
        case CM_PEM:
        case CM_MIM:
        case CM_TIMER:
        case CM_ASSIGN: {
          calcModeNormalGui();
          break;
        }
        case CM_AIM:
        case CM_EIM: {
          calcModeAimGui();
          break;
        }
      }
    #endif // PC_BUILD

    if(calcMode == CM_PEM) {
      hourGlassIconEnabled = false;
    }
  }



  void tamProcessInput(uint16_t item) {
    _tamProcessInput(item);
    _tamUpdateBuffer();
  }
#endif // !TESTSUITE_BUILD
