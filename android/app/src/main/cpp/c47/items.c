// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"

void itemToBeCoded(uint16_t unusedButMandatoryParameter) {
  funcOK = false;
}

  void fnOldItemError(uint16_t unusedButMandatoryParameter) {
    #if !defined(TESTSUITE_BUILD) && !defined(GENERATE_CATALOGS) &&  !defined(GENERATE_TESTPGMS)
      displayCalcErrorMessage(ERROR_OLD_ITEM_TO_REPLACE, ERR_REGISTER_LINE, REGISTER_X);
    #endif // !TESTSUITE_BUILD
  }


//#if !defined(GENERATE_CATALOGS)
//void fnToBeCoded(void) {
//  displayCalcErrorMessage(ERROR_FUNCTION_TO_BE_CODED, ERR_REGISTER_LINE, REGISTER_X);
//  #if defined(PC_BUILD)
//    moreInfoOnError("Function to be coded", "for that data type(s)!", NULL, NULL);
//  #endif // PC_BUILD
//}
//#endif // !GENERATE_CATALOGS

void fnNop(uint16_t unusedButMandatoryParameter) {
}

void doNothing(void) {
}



#ifdef DMCP_BUILD
  #define notAvail TI_Only_on_simulator
#else //PC_BUILD
  #define notAvail TI_Not_on_simulator
#endif //PC_BUILD

#define  _TO_ITM_NONE 0
#define  _TO_ITM_ERR  1
#define  _TO_ITM_TI   2

static uint8_t itemERRTIVal(int16_t itemNr) {
  switch(max(itemNr,-itemNr)) {
    #ifdef DMCP_BUILD
      case ITM_WRXPALL   :
                          return  _TO_ITM_ERR;
    #elif PC_BUILD
      case ITM_SAVEAUT  :
      case ITM_SETDAT   :
      case ITM_SETTIM   :
      case ITM_SYSTEM2  :
      case ITM_ACTUSB   :
                          return  _TO_ITM_ERR;
    #endif //PC_BUILD

    #ifdef PC_BUILD
      case ITM_DISK     :
      case ITM_BUZZ     :
      case ITM_PLAY     :
      case ITM_VOL      :
      case ITM_VOLMINUS :
      case ITM_VOLPLUS  :
      case ITM_VOLQ     :
      case ITM_BATT     :  return  _TO_ITM_TI;
    #endif //PC_BUILD
      default           :  return  _TO_ITM_NONE;
    }
}



//Items in here are both struck through in the softmenu, and are prevented from running, including TAM if in use, and TI_NOT_AVAILABE.
bool_t itemNotAvail(int16_t itemNr) {
#ifdef DMCP_BUILD
  return (itemERRTIVal(itemNr) !=  _TO_ITM_NONE);
#elif PC_BUILD
  if(itemERRTIVal(itemNr) !=  _TO_ITM_NONE) {
    #if (VERBOSE_LEVEL >= 0)
      printf("Item %i Softkey item not available in simulator, not executing and/or struck through.\n",itemNr);
    #endif
    return true;
  } else {
    return false;
  }
#else //!DMCP_BUILD && !PC_BUILD
  return false;
#endif //!DMCP_BUILD && !PC_BUILD
}

bool_t isFunctionOldParam16(uint16_t func) {
    return (func == ITM_BESTF_OLD ||
            func == ITM_RNG_OLD ||
            func == ITM_YY_DFLT_OLD ||
            func == ITM_DENMAX2_OLD);
}

#if !defined(GENERATE_CATALOGS) &&  !defined(GENERATE_TESTPGMS)
  uint16_t indirectionType(uint16_t func) {
    switch(indexOfItems[func].param) {
      case TM_FLAGR   :
      case TM_FLAGW   : {
        return INDPM_FLAG;
      }
      case TM_STORCL  :
      case TM_REGISTER:
      case TM_CMP     :
      case TM_M_DIM   : {
        return INDPM_REGISTER;
      }
      case TM_LBLONLY :
      case TM_LABEL   : {
        return INDPM_LABEL;
      }
      case TM_MENU    : {
        return INDPM_MENU;
      }
      case TM_SOLVE   : {
        if(func == ITM_SOLVE) {
          return (programRunStop == PGM_RUNNING ? INDPM_REGISTER : INDPM_LABEL);
        }
        else if(func == ITM_PGMSLV) {
          return INDPM_LABEL;
        }
        else {
          return INDPM_PARAM;
        }
      }
      default:          {
        return INDPM_PARAM;
      }
    }
  }

  char *lastFuncCatalogName(void) {
    if(lastFunc == ITM_VERS || lastFunc == NOPARAM) return "";
    if(lastFunc == ITM_CNST) {
      if(lastParam <= LAST_CONSTANT-FIRST_CONSTANT - 1) {                 //less 1 for the ITM_CNST inside the range (historically)
        int16_t addOffset = (lastParam > indexOfItems[ITM_CNST - 1].param ? 1 : 0);
        strcpy(lastTemp, indexOfItems[lastParam + 128 + addOffset].itemCatalogName);
      }
      else {
        lastTemp[0] = 0;
      }
    }
    else {
      strcpy(lastTemp, indexOfItems[lastFunc].itemCatalogName);
    }
    return lastTemp;
  }

  char *lastFuncSoftmenuName(void) {
    if(lastFunc == ITM_VERS || lastFunc == NOPARAM) return "";
    if(lastFunc == ITM_CNST) {
      if(lastParam <= LAST_CONSTANT-FIRST_CONSTANT - 1) {                 //less 1 for the ITM_CNST inside the range (historically)
        int16_t addOffset = (lastParam > indexOfItems[ITM_CNST - 1].param ? 1 : 0);
        strcpy(lastTemp, indexOfItems[lastParam + 128 + addOffset].itemSoftmenuName);
      }
      else {
        lastTemp[0] = 0;
      }
    }
    else {
      strcpy(lastTemp, indexOfItems[lastFunc].itemSoftmenuName);
    }
    return lastTemp;
  }

  int16_t lastSTORCL(void) {
    return lastParam;
  }

  int16_t lastFuncNo(void) {
    return lastFunc;
  }

  char *getItemCatalogName(int16_t itemNr) {
    char *itemName;

    if(abs(itemNr) <= LAST_ITEM) {                         // Predefined item
      itemName = (char *)indexOfItems[abs(itemNr)].itemCatalogName;
    }
    else if(itemNr >= ASSIGN_LABELS) {                     // User program
      uint8_t *lblPtr = labelList[itemNr - ASSIGN_LABELS].labelPointer;
      uint32_t count = *(lblPtr++);
      char    *tbPtr = tmpStringLabelOrVariableName;
      for(uint32_t i=0; i<count; ++i) {
        *(tbPtr++) = *(lblPtr++);
      }
      *(tbPtr) = 0;
      itemName = tmpStringLabelOrVariableName;
    }
    else if(itemNr >= ASSIGN_RESERVED_VARIABLES) {         // Reserved variable
      itemName = (char *)(allReservedVariables[itemNr - ASSIGN_RESERVED_VARIABLES].reservedVariableName + 1);
    }
    else if(itemNr >= ASSIGN_NAMED_VARIABLES) {            // User variable
      itemName = (char *)(allNamedVariables[itemNr - ASSIGN_NAMED_VARIABLES].variableName + 1);
    }
    else if(itemNr <= ASSIGN_USER_MENU) {                  // User menu
      itemName = (char *)userMenus[ASSIGN_USER_MENU - itemNr].menuName;
    }
    else {                                                 // unknown item, return empty string
      tmpStringLabelOrVariableName[0] = 0;
      itemName = tmpStringLabelOrVariableName;
    }

    return itemName;
  }

  function_t getItemFunc(int16_t itemNr) {
    void     (*func)(uint16_t);

    if(abs(itemNr) <= LAST_ITEM) {                         // Predefined item
      func = indexOfItems[itemNr].func;
    }
    else {                                                 // Any other user or reserved item
      func = addItemToBuffer;
    }

    return func;
  }


  void reallyRunFunction(int16_t func, uint16_t param) {
    #if defined(PC_BUILD) && defined(DEBUG_EXECUTE)
      printf("   >>>  reallyRunFunction: CM=%3u %5i%8s%8s\n",calcMode, func, indexOfItems[abs(func)].itemCatalogName, indexOfItems[abs(func)].itemSoftmenuName);
    #endif // PC_BUILD

    //do not store parameters for the commands ending or stopping a program as that is killing the prior RCL TI
    bool_t funcIsProgramStopControl = (func == ITM_END || func == ITM_RTN || func == ITM_STOP || func == ITM_RTNP1);
    if(!funcIsProgramStopControl) {
      lastFunc = func;
      lastParam = param;
    }

    #if !defined(TESTSUITE_BUILD)
      if(func != ITM_SOLVE_VAR && (calcMode == CM_NORMAL || calcMode == CM_NIM) &&
          (currentMenu() == -MNU_MVAR) &&
          (currentSolverStatus == 258 || currentSolverStatus == 259)) {  //allow interactive functions to clear the SolverReady flag
        currentSolverStatus &= ~SOLVER_STATUS_READY_TO_EXECUTE;
      }
      if(indexOfItems[func].func != fnTvmVar && (calcMode == CM_NORMAL || calcMode == CM_NIM) &&
          currentMenu() == -MNU_TVM &&
          (currentSolverStatus & SOLVER_STATUS_TVM_APPLICATION)) {       //clear execute flag, to prioritise entry, on all keys except the actual variable keys
        currentSolverStatus &= ~SOLVER_STATUS_READY_TO_EXECUTE;
      }
    #endif //TESTSUITE_BUILD

    if(func != ITM_CLX) { //JM Do not reset for backspace, because the timers need to run after the first action, CLX
      resetKeytimers();  //JM
    }
    if(func != ITM_DRG) { //JM Reset DRG cycling flag for any command except DRG
      DRG_Cycling = 0;  //JM
    }

    if((indexOfItems[func].status & US_STATUS) == US_ENABLED || (indexOfItems[func].status & US_STATUS) == US_ENABL_XEQ) {
      if((programRunStop != PGM_RUNNING || getSystemFlag(FLAG_IGN1ER))  && calcMode != CM_GRAPH && calcMode != CM_NO_UNDO && !getSystemFlag(FLAG_SOLVING) && !getSystemFlag(FLAG_INTING)) {
        #if defined(DEBUGUNDO)
          printf(">>> saveForUndo from reallyRunFunction: %i, %s, calcMode = %i ",func, indexOfItems[func].itemCatalogName, calcMode);
        #endif // DEBUGUNDO
        saveForUndo();
      }

      if(lastErrorCode == ERROR_RAM_FULL) {
        if((indexOfItems[func].status & US_STATUS) == US_ENABLED || calcMode == CM_CONFIRMATION) {
          displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            moreInfoOnError("In function reallyRunFunction:", "there is not enough memory to save for undo!", NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          return;
        }
        else {
          lastErrorCode = ERROR_NONE;
          temporaryInformation = TI_UNDO_DISABLED;
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            moreInfoOnError("In function reallyRunFunction:", "there is not enough memory to save for undo!", NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        }
      }
    }
    else if(((indexOfItems[func].status & US_STATUS) == US_CANCEL) && calcMode != CM_NO_UNDO && !GRAPHMODE){
      thereIsSomethingToUndo = false;
    }

    if(programRunStop != PGM_RUNNING) { //NORMAL MODE
      #if defined(PC_BUILD)
        char tmp[200]; sprintf(tmp,"^^^^reallyRunFunction func=%d param=%d\n",func, param); jm_show_comment(tmp);
        //printf("---#### Before function %s\n",tmp);
      #endif // PC_BUILD

      if((indexOfItems[func].status & HG_STATUS) == HG_ENABLED || (((indexOfItems[func].status & HG_STATUS) == HG_ENABLED_MX_ONLY) && isXYRegisterMatrix)) {
          hourGlassIconEnabled = true;
          screenUpdatingMode &= ~SCRUPD_MANUAL_STATUSBAR;
          showHideHourGlass();
      }

      if(func == ITM_GTO || func == ITM_XEQ || func == ITM_GTOP) {
        while(currentSubroutineLevel > 0) {
          fnReturn(0);
        }
        fnReturn(0); // 1 more time to clean local registers
      }

/* Full refresh included in showHideHourGlass above, so removinf it here to save time
      #if defined(DMCP_BUILD)
        lcd_refresh();
      #else // !DMCP_BUILD
        refreshLcd(NULL);
      #endif // DMCP_BUILD
*/

    screenUpdatingMode = SCRUPD_AUTO;
    }

    else { //PGM_RUNNING MODE
      if(func == ITM_GTO || func == ITM_XEQ || func == ITM_GTOP) {
        screenUpdatingMode &= ~SCRUPD_MANUAL_STATUSBAR;
        showHideHourGlass();
      }
      #if defined(PC_BUILD)
        force_refresh(timed); //Added this to enable 0.5 second refresh and break during running
      #endif //PC_BUILD
    }



    #if defined(PC_BUILD) && defined(VERBOSE_MINIMUM)// || defined(DEBUG_EXECUTE)
      char ss1[30], ss2[30];
      stringToASCII(indexOfItems[abs(func)].itemCatalogName, ss1);
      stringToASCII(indexOfItems[abs(func)].itemSoftmenuName, ss2);
      // printf("   >>    reallyRunFunction: %5i%8s§%8s  %5i  SBI:%s\n",func, ss1, ss2, param, programRunStop == PGM_WAITING ? "W" : programRunStop == PGM_RUNNING ? "P" : hourGlassIconEnabled ? "HG" : "??");
      fflush(stdout);
    #endif // PC_BUILD



    if((programRunStop != PGM_RUNNING && func != ITM_LASTT) || func == ITM_XEQ || timeLastOp0 == 0) {    //The first manual command and XEQ (re)starts the timer by setting timeLastOp0
      LastOpTimerReStart(func);                                                                          //    You don't want the timer to always be read before every command when in a program. It would be wasteful. During program execution, LASTT? laps the counter.
    }
    else if(programRunStop == PGM_RUNNING && func == ITM_LASTT) {                                        //When running, the timing is provided from LASTT? to LASTT?
      LastOpTimerLap(func);
    }

#if !defined(TESTSUITE_BUILD)
    // mark the previous I and J, when STOSEQ and RCLSEQ are being used
    real_t iir,jjr;
    if((func == ITM_RCLELPLUS || func == ITM_STOELPLUS) && isMatrixIndexed() && getRegisterAsRealQuiet(REGISTER_I, &iir) && getRegisterAsRealQuiet(REGISTER_J, &jjr)) {
      lastI=realToUint32C47(&iir);
      lastJ=realToUint32C47(&jjr);
    } else {
      lastI = 0xFFFF;
      lastJ = 0xFFFF;
    }
#endif //TESTSUITE_BUILD


    //**RunFunction
    if(!itemNotAvail(func)) {
      indexOfItems[func].func(param);
    } else {
        if(itemERRTIVal(func) ==  _TO_ITM_TI) {
          temporaryInformation = TI_NOT_AVAILABLE;
        }
        else if(itemERRTIVal(func) ==  _TO_ITM_ERR) {
          displayCalcErrorMessage(notAvail, ERR_REGISTER_LINE, REGISTER_X);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            sprintf(errorMessage, "Not Available");
            moreInfoOnError("In function reallyRunFunction:", errorMessage, NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        }
      screenUpdatingMode = SCRUPD_AUTO;
    }

    switch(func) { //list the items for which LastERR will not be updated, i.e it will keep the prior error
        case ITM_LASTERR:
        case ITM_GTO:
        case ITM_XEQ:
        case ITM_BACK:
        case ITM_CASE:
        case ITM_SKIP:
        default: previousErrorCode = lastErrorCode;
             break;
    }

    #if defined(PC_BUILD) && defined(DEBUG_EXECUTE)
      printf("   >>>  reallyRunFunction: CM=%3u                                SBI:%s\n", calcMode, programRunStop == PGM_WAITING ? "W" : programRunStop == PGM_RUNNING ? "P" : hourGlassIconEnabled ? "HG" : "??");
    #endif // PC_BUILD

    if(programRunStop != PGM_RUNNING) {                                                                  //stores the last time to timeLastOp, if not running
      LastOpTimerLap(func);
    }

    if(funcIsProgramStopControl) {
      screenUpdatingMode &= ~SCRUPD_MANUAL_STATUSBAR;
      if(currentSubroutineLevel == 0) {
        #if !defined(TESTSUITE_BUILD)
          forceSBupdate();
        #endif //TESTSUITE_BUILD
        screenUpdatingMode = SCRUPD_AUTO;
      }
    }

    switch(func) {                              //functions to cause a graph redraw
      case ITM_DRAW:       //EQN Draw
      case ITM_DRAW_LU:    //Xup Xdn
      case VAR_LX:
      case VAR_UX:

      case ITM_PLOT_STAT:  //Plot menu
      case ITM_PLINE:
      case ITM_DIFF:
      case ITM_RMS:
      case ITM_PCROS:
      case ITM_PPLUS:
      case ITM_NVECT:
      case ITM_SNAP:
      case ITM_NULL:
      case ITM_SCALE:
      case ITM_INTG:
      case ITM_SHADE:
      case ITM_PBOX:
      case ITM_PCURVE:
      case ITM_VECT:
      case ITM_SHOWX:
      case ITM_SHOWY:
      case ITM_MZOOMY:
      case ITM_PZOOMY:
      case ITM_LISTXY:
      case ITM_PLOTRST:


      case ITM_PLOT_ASSESS:
      case ITM_PLOT_NXT:
      case ITM_PLOT_REV:

      case ITM_PLOT_SCATR:
      case ITM_PLOT_CENTRL:
      case ITM_SMI:

      case ITM_HPLOT:        //HPLOT
      case ITM_HNORM:
      case ITM_PLOTZOOM:

        reDraw = true;
    }


    if(temporaryInformation == TI_LAST_CONST_CATNAME && (currentSolverStatus & 0x000F) != 0) {
      temporaryInformation = TI_NO_INFO;
    }
    else
    if(((FIRST_CONSTANT <= func && func <= LAST_CONSTANT) || func == ITM_CNST) && calcMode == CM_NORMAL) {   //including ITM_CNST inside the range (historically)
      temporaryInformation = TI_LAST_CONST_CATNAME;
    }
    else
    if(calcMode == CM_NORMAL && !getSystemFlag(FLAG_INTING) && !getSystemFlag(FLAG_SOLVING)) {
#if !defined(TESTSUITE_BUILD)

      //bool_t inMatrixMenu = (tam.mode == 0 ? softmenu[softmenuStack[0].softmenuId].menuItem : softmenu[softmenuStack[1].softmenuId].menuItem) == -MNU_MATX;
      bool_t inRegisterRange = (param <= LAST_LETTERED_REGISTER ||
                       (FIRST_STAT_REGISTER  <= param && param <= LAST_STAT_REGISTER) ||
                       (FIRST_SPARE_REGISTER <= param && param <= LAST_SPARE_REGISTER));
      bool_t inReservedRange =      (FIRST_NAMED_RESERVED_VARIABLE <= param && param <= LAST_RESERVED_VARIABLE);
      bool_t inNameRegisterRange =  (FIRST_NAMED_VARIABLE <= param && param <= LAST_NAMED_VARIABLE);
      bool_t inLocalRegisters =     (param >= FIRST_LOCAL_REGISTER && param < FIRST_LOCAL_REGISTER + currentNumberOfLocalRegisters);
      bool_t isMatrix = (func == ITM_RCL || func == ITM_STO) ? ((inRegisterRange || inReservedRange || inNameRegisterRange || inLocalRegisters) ? (getRegisterDataType(param) == dtReal34Matrix || getRegisterDataType(param) == dtComplex34Matrix) : false) : false;
      uint16_t rr;
      calcRegister_t regStats = FAILED_INDIRECTION;
      if(inNameRegisterRange) regStats = findNamedVariable("STATS");

      switch(func) {
        case VAR_UEST        : solverEstimatesUsed = true; break;
        case VAR_LEST        : solverEstimatesUsed = true; break;
        case VAR_UX          :
        case VAR_LX          :
        case VAR_UY          :
        case VAR_LY          : temporaryInformation = TI_STORCL; break;
        case ITM_STO         :
        case ITM_RCL         : temporaryInformation = \
                               (param == REGISTER_I) && isMatrixIndexed() ? TI_I : \
                               (param == REGISTER_J) && isMatrixIndexed() ? TI_J : \
                               (inNameRegisterRange) ? ((isStatsMatrixN(&rr, regStats) && param == regStats) ? TI_STATISTIC_SUMS : TI_STORCL) : \
                               (isMatrix) ? TI_STORCL : \
                               (inReservedRange || inRegisterRange || inLocalRegisters) ? TI_STORCL : \
                               TI_NO_INFO ; break;
        case ITM_RCLELPLUS   :
        case ITM_RCLEL       :
        case ITM_STOELPLUS   :
        case ITM_STOEL       : if(isMatrixIndexed()) temporaryInformation = TI_MIJEQ;   break;

        case ITM_INDEX       :
        case ITM_IPLUS       :
        case ITM_IMINUS      :
        case ITM_JPLUS       :
        case ITM_JMINUS      : if(isMatrixIndexed()) temporaryInformation = TI_MIJ;   break;

        case ITM_RCLIJ       :
        case ITM_STOIJ       : if(isMatrixIndexed()) temporaryInformation = TI_IJ;    break;
        default:;
      }

      //TI for conversion menus
      if(lastErrorCode == ERROR_NONE && temporaryInformation == TI_NO_INFO) {
        switch(softmenu[softmenuStack[0].softmenuId].menuItem) {
          case -MNU_CONVE :
          case -MNU_CONVP :
          case -MNU_CONVFP:
          case -MNU_CONVM :
          case -MNU_CONVX :
          case -MNU_CONVV :
          case -MNU_CONVA :
          case -MNU_CONVS :
          case -MNU_CONVANG :
          case -MNU_MISC :
          case -MNU_CONVHUM :
          case -MNU_CONVYMMV :
          case -MNU_CONVCHEF : {
            errorMessage[0]=0;
            strcat(errorMessage,indexOfItems[func].itemCatalogName);
            temporaryInformation = TI_NO_INFO;
            int16_t i = 0;
            while(errorMessage[i+1] != 0) {
              if(STD_RIGHT_ARROW[0] == errorMessage[i] && (STD_RIGHT_ARROW[1] == errorMessage[i+1] || STD_RIGHT_SHORT_ARROW[1] == errorMessage[i+1])) {
                temporaryInformation = TI_CONV_MENU_STR;
                errorMessage[i++] = 0;
                errorMessage[i++] = 0;
                break;
              }
              i++;
            }
            int16_t j = 0;
            errorMessage[j] = 0;
            while(errorMessage[i] != 0) {
              errorMessage[j++] =  errorMessage[i++];
            }
            errorMessage[j] = 0;
            expandConversionName(errorMessage);
            break;
          }
          default:break;
        }
      }
#endif //TESTSUITE_BUILD
    }


    if(lastErrorCode != 0) {
      if(getSystemFlag(FLAG_IGN1ER)) {
        if(thereIsSomethingToUndo) {
          undo();
        }
        lastErrorCode = ERROR_NONE;
      }
      else {
        if(thereIsSomethingToUndo &&
          !((func == ITM_EIGVAL || func == ITM_EIGVEC) && lastErrorCode == ERROR_SOLVER_ABORT)) {
          undo();
        }
      }
      clearSystemFlag(FLAG_IGN1ER);
    }
    else {
      if((indexOfItems[func].status & SLS_STATUS) == SLS_DISABLED) {
        clearSystemFlag(FLAG_ASLIFT);
      }
      else if((indexOfItems[func].status & SLS_STATUS) == SLS_ENABLED) {
        setSystemFlag(FLAG_ASLIFT);
      }
    }


    #if defined(DMCP_BUILD)
      updateVbatIntegrated(false);              //Check the battery directly after a task so that the worst case voltage is recorded
    #endif


    if(programRunStop != PGM_RUNNING) {
      updateMatrixHeightCache();
      cachedDynamicMenu = 0;
      #if defined(PC_BUILD)
        refreshLcd(NULL);
      #endif // PC_BUILD
    }

    #if defined(PC_BUILD)
      if(gmpMemInBytes != 0 && !getSystemFlag(FLAG_SOLVING) && !iterations) {
        char str[30], txt[200];
        sprintf(txt, "gmpMemInBytes should be 0 but it is not! gmpMemInBytes = %zu. Check to ensure allocated long integers have been freed.", gmpMemInBytes);
        stringToASCII(indexOfItems[abs(func)].itemSoftmenuName, str);
        errorf(txt);
        fprintf(stderr, "This happened after %s\n", str);
        fflush(stderr);
      }
    #endif // PC_BUILD
  }

  void runFunction(int16_t func) {
    #if defined(PC_BUILD) && defined(DEBUG_EXECUTE)
      printf("   >>>RunFunction: %5i%8s%8s\n",func, indexOfItems[abs(func)].itemCatalogName, indexOfItems[abs(func)].itemSoftmenuName);
    #endif // PC_BUILD
    funcOK = true;

    #if defined(PC_BUILD)
      if(func >= LAST_ITEM) {
        sprintf(errorMessage, "item (%" PRId16 ") must be below LAST_ITEM", func);
        moreInfoOnError("In function runFunction:", errorMessage, NULL, NULL);
      }
    #endif // PC_BUILD

    if(programRunStop != PGM_RUNNING) {
      if(func == ITM_RCL && dynamicMenuItem > -1) {
        char *varCatalogItem = dynmenuGetLabel(dynamicMenuItem);
        if (strcmp(varCatalogItem, "RCL") != 0) {
          calcRegister_t var = findNamedVariable(varCatalogItem);
          if(var != INVALID_VARIABLE) {
            if(calcMode == CM_PEM) {
              insertUserItemInProgram(func, varCatalogItem);
            }
            else {
              reallyRunFunction(func, var);
            }
          }
          else {
            displayCalcErrorMessage(ERROR_UNDEF_SOURCE_VAR, ERR_REGISTER_LINE, REGISTER_X);
            #if (EXTRA_INFO_ON_CALC_ERROR == 1)
              sprintf(errorMessage, "string '%s' is not a named variable", varCatalogItem);
              moreInfoOnError("In function runFunction:", errorMessage, NULL, NULL);
            #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          }
          return;
        }
      }
      if(func == ITM_XEQ && dynamicMenuItem > -1) {
        char *varCatalogItem = dynmenuGetLabel(dynamicMenuItem);
        if (strcmp(varCatalogItem, "XEQ") != 0) {
          calcRegister_t label = findNamedLabel(varCatalogItem);
          if(label != INVALID_VARIABLE) {
            if(calcMode == CM_PEM) {
              insertUserItemInProgram(func, varCatalogItem);
            }
            else {
              reallyRunFunction(func, label);
            }
          }
          else {
            displayCalcErrorMessage(ERROR_LABEL_NOT_FOUND, ERR_REGISTER_LINE, REGISTER_X);
            #if (EXTRA_INFO_ON_CALC_ERROR == 1)
              sprintf(errorMessage, "string '%s' is not a named label", varCatalogItem);
              moreInfoOnError("In function runFunction:", errorMessage, NULL, NULL);
            #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          }
          return;
        }
      }
      if(tam.mode == 0 && TM_VALUE <= indexOfItems[func].param && indexOfItems[func].param <= TM_CMP && (calcMode != CM_PEM || aimBuffer[0] == 0 || nimNumberPart != NP_INT_BASE)) {
        #if defined(VERBOSEKEYS)
          printf("items.c: runfunction (before tamEnterMode): %i, %s\n", softmenu[softmenuStack[0].softmenuId].menuItem, indexOfItems[-softmenu[softmenuStack[0].softmenuId].menuItem].itemSoftmenuName);
        #endif // VERBOSEKEYS


        //**Start TAM function
        if(!itemNotAvail(func)) {
          tamEnterMode(func);
        } else {
          if(itemERRTIVal(func) ==  _TO_ITM_TI) {
            temporaryInformation = TI_NOT_AVAILABLE;
          }
          else if(itemERRTIVal(func) ==  _TO_ITM_ERR) {
            displayCalcErrorMessage(notAvail, ERR_REGISTER_LINE, REGISTER_X);
            #if (EXTRA_INFO_ON_CALC_ERROR == 1)
              sprintf(errorMessage, "Not Available");
              moreInfoOnError("In function runFunction:", errorMessage, NULL, NULL);
            #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          }
          screenUpdatingMode = SCRUPD_AUTO;
        }


        #if defined(VERBOSEKEYS)
          printf("items.c: runfunction (after tamEnterMode): %i, %s\n", softmenu[softmenuStack[0].softmenuId].menuItem, indexOfItems[-softmenu[softmenuStack[0].softmenuId].menuItem].itemSoftmenuName);
        #endif // VERBOSEKEYS
        return;
      }

      if(calcMode == CM_PEM) {
        bool_t doNotAddStep = false;
        switch(func) {
          case ITM_EXIT1:
          case ITM_CLRMOD:
          case ITM_SNAP:
          case ITM_NOP:
          case ITM_BASEMENU:
                doNotAddStep = currentKeyCode == 32; break;
          case ITM_T_UP_ARROW:
          case ITM_T_DOWN_ARROW:
          case ITM_T_LLEFT_ARROW:
          case ITM_T_RRIGHT_ARROW:
          case ITM_T_LEFT_ARROW:
          case ITM_T_RIGHT_ARROW:
          case ITM_ASSIGN:
          case ITM_XSWAP:
          case ITM_XPARSE:
          case CHR_case:
          case CHR_num:
          case ITM_SCR:
          case ITM_USERMODE:
                doNotAddStep =  getSystemFlag(FLAG_ALPHA); break;
          case ITM_EDIT:
                doNotAddStep = !getSystemFlag(FLAG_ALPHA); break;
          default:;
        }

        #if defined(VERBOSEKEYS)
          printf("$$ PEM:    func=%d  showFunctionNameItem1=%d doNotAddStep=%d tam.mode=%d getSystemFlag(FLAG_ALPHA)=%d tam.alpha=%d\n", func, showFunctionNameItem, doNotAddStep, tam.mode, getSystemFlag(FLAG_ALPHA), tam.alpha);
          fflush(stdout);
        #endif // VERBOSEKEYS

        if( !tam.mode && ((func != ITM_BACKSPACE && (!catalog || catalog == CATALOG_MVAR || fnKeyInCatalog) && !doNotAddStep) || func == ITM_ENTER )){
          #if defined(VERBOSEKEYS)
            printf("$$         items.c: runfunction: add step (before addStepInProgram) func=%i\n",func);
            fflush(stdout);
          #endif // VERBOSEKEYS
          addStepInProgram(func);
          return;
        }
      }
    }

    #if defined(PC_BUILD)
      char tmp[200]; sprintf(tmp,"^^^^ReallyRunFunction func=%d\n",func); jm_show_comment(tmp);
    #endif // PC_BUILD

    reallyRunFunction(func, indexOfItems[func].param);

    if(!funcOK) {
      displayCalcErrorMessage(ERROR_ITEM_TO_BE_CODED, ERR_REGISTER_LINE, REGISTER_X);
      #if defined(PC_BUILD)
        sprintf(errorMessage, "%" PRId16 " = %s", func, indexOfItems[func].itemCatalogName);
        moreInfoOnError("In function runFunction:", "Item not implemented", errorMessage, "to be coded");
      #endif // PC_BUILD
    }
  }
#endif // !GENERATE_CATALOGS && !GENERATE_TESTPGMS

#if defined(GENERATE_CATALOGS) || defined(GENERATE_TESTPGMS)
  void fnAsnViewer                 (uint16_t unusedButMandatoryParameter) {}
  void registerBrowser             (uint16_t unusedButMandatoryParameter) {}
  void flagBrowser                 (uint16_t unusedButMandatoryParameter) {}
  void fontBrowser                 (uint16_t unusedButMandatoryParameter) {}
  void fnPow10                     (uint16_t unusedButMandatoryParameter) {}
  void fnIntegerMode               (uint16_t unusedButMandatoryParameter) {}
  void fnConstant                  (uint16_t unusedButMandatoryParameter) {}
  void fnInvert                    (uint16_t unusedButMandatoryParameter) {}
  void fn2Pow                      (uint16_t unusedButMandatoryParameter) {}
  void fn10Pow                     (uint16_t unusedButMandatoryParameter) {}
  void fnCubeRoot                  (uint16_t unusedButMandatoryParameter) {}
  void fnMagnitude                 (uint16_t unusedButMandatoryParameter) {}
  void fnAgm                       (uint16_t unusedButMandatoryParameter) {}
  void fnDisplayFormatAll          (uint16_t unusedButMandatoryParameter) {}
  void fnDisplayFormatFix          (uint16_t unusedButMandatoryParameter) {}
  void fnDisplayFormatSci          (uint16_t unusedButMandatoryParameter) {}
  void fnDisplayFormatEng          (uint16_t unusedButMandatoryParameter) {}
  void fnDisplayFormatDsp          (uint16_t unusedButMandatoryParameter) {}
  void fnDisplayFormatTime         (uint16_t unusedButMandatoryParameter) {}
  void fnArccos                    (uint16_t unusedButMandatoryParameter) {}
  void fnArccosh                   (uint16_t unusedButMandatoryParameter) {}
  void fnArcsin                    (uint16_t unusedButMandatoryParameter) {}
  void fnArcsinh                   (uint16_t unusedButMandatoryParameter) {}
  void fnArctan                    (uint16_t unusedButMandatoryParameter) {}
  void fnArctanh                   (uint16_t unusedButMandatoryParameter) {}
  void fnAtan2                     (uint16_t unusedButMandatoryParameter) {}
  void fnCos                       (uint16_t unusedButMandatoryParameter) {}
  void fnCosh                      (uint16_t unusedButMandatoryParameter) {}
  void fnSin                       (uint16_t unusedButMandatoryParameter) {}
  void fnSinc                      (uint16_t unusedButMandatoryParameter) {}
  void fnSincpi                    (uint16_t unusedButMandatoryParameter) {}
  void fnSinh                      (uint16_t unusedButMandatoryParameter) {}
  void fnTan                       (uint16_t unusedButMandatoryParameter) {}
  void fnTanh                      (uint16_t unusedButMandatoryParameter) {}
  void fnDrop                      (uint16_t unusedButMandatoryParameter) {}
  void fnDropY                     (uint16_t unusedButMandatoryParameter) {}
  void fnBatteryVoltage            (uint16_t unusedButMandatoryParameter) {}
  void fnCurveFitting              (uint16_t unusedButMandatoryParameter) {}
  void fnCeil                      (uint16_t unusedButMandatoryParameter) {}
  void fnFloor                     (uint16_t unusedButMandatoryParameter) {}
  void fnClearFlag                 (uint16_t unusedButMandatoryParameter) {}
  void fnFlipFlag                  (uint16_t unusedButMandatoryParameter) {}
  void fnSetFlag                   (uint16_t unusedButMandatoryParameter) {}
  void fnClAll                     (uint16_t unusedButMandatoryParameter) {}
  void fnClX                       (uint16_t unusedButMandatoryParameter) {}
  void fnClFAll                    (uint16_t unusedButMandatoryParameter) {}
  void fnClPAll                    (uint16_t unusedButMandatoryParameter) {}
  void fnClSigma                   (uint16_t unusedButMandatoryParameter) {}
  void fnClearStack                (uint16_t unusedButMandatoryParameter) {}
  void fnClearRegisters            (uint16_t unusedButMandatoryParameter) {}
  void fnTimeFormat                (uint16_t unusedButMandatoryParameter) {}
  void fnSetDateFormat             (uint16_t unusedButMandatoryParameter) {}
  void fnComplexUnit               (uint16_t unusedButMandatoryParameter) {}
  void fnComplexMode               (uint16_t unusedButMandatoryParameter) {}
  void fnComplexResult             (uint16_t unusedButMandatoryParameter) {}
  void fnConjugate                 (uint16_t unusedButMandatoryParameter) {}
  void fnAngularMode               (uint16_t unusedButMandatoryParameter) {}
  void fnDenMode                   (uint16_t unusedButMandatoryParameter) {}
  void fnDenMax                    (uint16_t unusedButMandatoryParameter) {}
  void fnErf                       (uint16_t unusedButMandatoryParameter) {}
  void fnErfc                      (uint16_t unusedButMandatoryParameter) {}
  void fnExp                       (uint16_t unusedButMandatoryParameter) {}
  void fnExpM1                     (uint16_t unusedButMandatoryParameter) {}
  void fnExpt                      (uint16_t unusedButMandatoryParameter) {}
  void fnMant                      (uint16_t unusedButMandatoryParameter) {}
  void fnCxToRe                    (uint16_t unusedButMandatoryParameter) {}
  void fnReToCx                    (uint16_t unusedButMandatoryParameter) {}
  void fnFillStack                 (uint16_t unusedButMandatoryParameter) {}
  void fnIsFlagClear               (uint16_t unusedButMandatoryParameter) {}
  void fnIsFlagClearClear          (uint16_t unusedButMandatoryParameter) {}
  void fnIsFlagClearFlip           (uint16_t unusedButMandatoryParameter) {}
  void fnIsFlagClearSet            (uint16_t unusedButMandatoryParameter) {}
  void fnIsFlagSet                 (uint16_t unusedButMandatoryParameter) {}
  void fnIsFlagSetClear            (uint16_t unusedButMandatoryParameter) {}
  void fnIsFlagSetFlip             (uint16_t unusedButMandatoryParameter) {}
  void fnIsFlagSetSet              (uint16_t unusedButMandatoryParameter) {}
  void fnKeyEnter                  (uint16_t unusedButMandatoryParameter) {}
  void fnKeyExit                   (uint16_t unusedButMandatoryParameter) {}
  void fnKeyUp                     (uint16_t unusedButMandatoryParameter) {}
  void fnKeyDown                   (uint16_t unusedButMandatoryParameter) {}
  void fnKeyDotD                   (uint16_t unusedButMandatoryParameter) {}
  void fnKeyCC                     (uint16_t unusedButMandatoryParameter) {}
  void fnKeyBackspace              (uint16_t unusedButMandatoryParameter) {}
  void fnKeyAngle                  (uint16_t unusedButMandatoryParameter) {}
  void fnDisplayStack              (uint16_t unusedButMandatoryParameter) {}
  void fnFreeFlashMemory           (uint16_t unusedButMandatoryParameter) {}
  void fnFreeMemory                (uint16_t unusedButMandatoryParameter) {}
  void fnFp                        (uint16_t unusedButMandatoryParameter) {}
  void fnIp                        (uint16_t unusedButMandatoryParameter) {}
  void allocateLocalRegisters      (uint16_t unusedButMandatoryParameter) {}
  void fnLeadingZeros              (uint16_t unusedButMandatoryParameter) {}
  void fnNeighb                    (uint16_t unusedButMandatoryParameter) {}
  void fnGcd                       (uint16_t unusedButMandatoryParameter) {}
  void fnMin                       (uint16_t unusedButMandatoryParameter) {}
  void fnMax                       (uint16_t unusedButMandatoryParameter) {}
  void fnStatSum                   (uint16_t unusedButMandatoryParameter) {}
  void fnIsPrime                   (uint16_t unusedButMandatoryParameter) {}
  void fnIsLeap                    (uint16_t unusedButMandatoryParameter) {}
  void fnCheckType                 (uint16_t unusedButMandatoryParameter) {}
  void fnCheckNumber               (uint16_t unusedButMandatoryParameter) {}
  void fnCheckAngle                (uint16_t unusedButMandatoryParameter) {}
  void fnCheckSpecial              (uint16_t unusedButMandatoryParameter) {}
  void fnCheckNaN                  (uint16_t unusedButMandatoryParameter) {}
  void fnCheckInfinite             (uint16_t unusedButMandatoryParameter) {}
  void fnCheckPlusZero             (uint16_t unusedButMandatoryParameter) {}
  void fnCheckMinusZero            (uint16_t unusedButMandatoryParameter) {}
  void fnCheckMatrix               (uint16_t unusedButMandatoryParameter) {}
  void fnCheckMatrixSquare         (uint16_t unusedButMandatoryParameter) {}
  void fnCheckReIsZero             (uint16_t unusedButMandatoryParameter) {}
  void fnCheckImIsZero             (uint16_t unusedButMandatoryParameter) {}
  void fnCheckReNotZero            (uint16_t unusedButMandatoryParameter) {}
  void fnCheckImNotZero            (uint16_t unusedButMandatoryParameter) {}
  void fnCheckIsVect2d             (uint16_t unusedButMandatoryParameter) {}
  void fnCheckIsVect3d             (uint16_t unusedButMandatoryParameter) {}
  void fnRandom                    (uint16_t unusedButMandatoryParameter) {}
  void fnRandomI                   (uint16_t unusedButMandatoryParameter) {}
  void fnImaginaryPart             (uint16_t unusedButMandatoryParameter) {}
  void fnRecall                    (uint16_t unusedButMandatoryParameter) {}
  void fnRecallConfig              (uint16_t unusedButMandatoryParameter) {}
  void fnRecallElement             (uint16_t unusedButMandatoryParameter) {}
  void fnRecallVElement            (uint16_t unusedButMandatoryParameter) {}
  void fnRecallElementPlus         (uint16_t unusedButMandatoryParameter) {}
  void fnRecallIJ                  (uint16_t unusedButMandatoryParameter) {}
  void fnRecallStack               (uint16_t unusedButMandatoryParameter) {}
  void fnRecallAdd                 (uint16_t unusedButMandatoryParameter) {}
  void fnRecallSub                 (uint16_t unusedButMandatoryParameter) {}
  void fnRecallMult                (uint16_t unusedButMandatoryParameter) {}
  void fnRecallDiv                 (uint16_t unusedButMandatoryParameter) {}
  void fnRecallMin                 (uint16_t unusedButMandatoryParameter) {}
  void fnRecallMax                 (uint16_t unusedButMandatoryParameter) {}
  void fnRadixMark                 (uint16_t unusedButMandatoryParameter) {}
  void fnReset                     (uint16_t unusedButMandatoryParameter) {}
  void fnRealPart                  (uint16_t unusedButMandatoryParameter) {}
  void fnRmd                       (uint16_t unusedButMandatoryParameter) {}
  void fnRound                     (uint16_t unusedButMandatoryParameter) {}
  void fnRoundi                    (uint16_t unusedButMandatoryParameter) {}
  void fnRsd                       (uint16_t unusedButMandatoryParameter) {}
  void fnRdp                       (uint16_t unusedButMandatoryParameter) {}
  void fnRollDown                  (uint16_t unusedButMandatoryParameter) {}
  void fnRollUp                    (uint16_t unusedButMandatoryParameter) {}
  void fnSeed                      (uint16_t unusedButMandatoryParameter) {}
  void fnSetDate                   (uint16_t unusedButMandatoryParameter) {}
  void fnSetTime                   (uint16_t unusedButMandatoryParameter) {}
  void configCommon                (uint16_t unusedButMandatoryParameter) {}
  void fnLcm                       (uint16_t unusedButMandatoryParameter) {}
  void fnSign                      (uint16_t unusedButMandatoryParameter) {}
  void fnSlvq                      (uint16_t unusedButMandatoryParameter) {}
  void fnSlvc                      (uint16_t unusedButMandatoryParameter) {}
  void fnGetIntegerSignMode        (uint16_t unusedButMandatoryParameter) {}
  void fnLog2                      (uint16_t unusedButMandatoryParameter) {}
  void fnLog10                     (uint16_t unusedButMandatoryParameter) {}
  void fnLn                        (uint16_t unusedButMandatoryParameter) {}
  void fnLogXY                     (uint16_t unusedButMandatoryParameter) {}
  void fnLnP1                      (uint16_t unusedButMandatoryParameter) {}
  void fnLnGamma                   (uint16_t unusedButMandatoryParameter) {}
  void fnLnBeta                    (uint16_t unusedButMandatoryParameter) {}
  void fnBeta                      (uint16_t unusedButMandatoryParameter) {}
  void fnIxyz                      (uint16_t unusedButMandatoryParameter) {}
  void fnGamma                     (uint16_t unusedButMandatoryParameter) {}
  void fnGammaX                    (uint16_t unusedButMandatoryParameter) {}
  void fnBesselJ                   (uint16_t unusedButMandatoryParameter) {}
  void fnBesselY                   (uint16_t unusedButMandatoryParameter) {}
  void fnZeta                      (uint16_t unusedButMandatoryParameter) {}
  void fnBn                        (uint16_t unusedButMandatoryParameter) {}
  void fnBnStar                    (uint16_t unusedButMandatoryParameter) {}
  void fnWnegative                 (uint16_t unusedButMandatoryParameter) {}
  void fnWpositive                 (uint16_t unusedButMandatoryParameter) {}
  void fnWinverse                  (uint16_t unusedButMandatoryParameter) {}
  void fnHermite                   (uint16_t unusedButMandatoryParameter) {}
  void fnHermiteP                  (uint16_t unusedButMandatoryParameter) {}
  void fnLaguerre                  (uint16_t unusedButMandatoryParameter) {}
  void fnLaguerreAlpha             (uint16_t unusedButMandatoryParameter) {}
  void fnLegendre                  (uint16_t unusedButMandatoryParameter) {}
  void fnChebyshevT                (uint16_t unusedButMandatoryParameter) {}
  void fnChebyshevU                (uint16_t unusedButMandatoryParameter) {}
  void fnIDiv                      (uint16_t unusedButMandatoryParameter) {}
  void fnIDivR                     (uint16_t unusedButMandatoryParameter) {}
  void fnMirror                    (uint16_t unusedButMandatoryParameter) {}
  void fnZip                       (uint16_t unusedButMandatoryParameter) {}
  void fnUnzip                     (uint16_t unusedButMandatoryParameter) {}
  void fnMod                       (uint16_t unusedButMandatoryParameter) {}
  void fnMulMod                    (uint16_t unusedButMandatoryParameter) {}
  void fnExpMod                    (uint16_t unusedButMandatoryParameter) {}
  void fnPower                     (uint16_t unusedButMandatoryParameter) {}
  void fnPi                        (uint16_t unusedButMandatoryParameter) {}
  void fnUserMode                  (uint16_t unusedButMandatoryParameter) {}
  void fnParallel                  (uint16_t unusedButMandatoryParameter) {}
  void fnSquareRoot                (uint16_t unusedButMandatoryParameter) {}
  void fnSubtract                  (uint16_t unusedButMandatoryParameter) {}
  void fnChangeSign                (uint16_t unusedButMandatoryParameter) {}
  void fnM1Pow                     (uint16_t unusedButMandatoryParameter) {}
  void runDMCPmenu                 (uint16_t unusedButMandatoryParameter) {}
  void fnMultiply                  (uint16_t unusedButMandatoryParameter) {}
  void fnDblDivide                 (uint16_t unusedButMandatoryParameter) {}
  void fnDblDivideRemainder        (uint16_t unusedButMandatoryParameter) {}
  void fnDblMultiply               (uint16_t unusedButMandatoryParameter) {}
  void fnChangeBase                (uint16_t unusedButMandatoryParameter) {}
  void fnDivide                    (uint16_t unusedButMandatoryParameter) {}
  void fnAdd                       (uint16_t unusedButMandatoryParameter) {}
  void fnSigmaAddRem               (uint16_t unusedButMandatoryParameter) {}
  void fnXEqualsTo                 (uint16_t unusedButMandatoryParameter) {}
  void fnXNotEqual                 (uint16_t unusedButMandatoryParameter) {}
  void fnXAlmostEqual              (uint16_t unusedButMandatoryParameter) {}
  void fnXLessThan                 (uint16_t unusedButMandatoryParameter) {}
  void fnXLessEqual                (uint16_t unusedButMandatoryParameter) {}
  void fnXGreaterThan              (uint16_t unusedButMandatoryParameter) {}
  void fnXGreaterEqual             (uint16_t unusedButMandatoryParameter) {}
  void fnIsConverged               (uint16_t unusedButMandatoryParameter) {}
  void fnGetLocR                   (uint16_t unusedButMandatoryParameter) {}
  void fnSwapRealImaginary         (uint16_t unusedButMandatoryParameter) {}
  void fnSetRoundingMode           (uint16_t unusedButMandatoryParameter) {}
  void fnGetRoundingMode           (uint16_t unusedButMandatoryParameter) {}
  void fnSetWordSize               (uint16_t unusedButMandatoryParameter) {}
  void fnGetWordSize               (uint16_t unusedButMandatoryParameter) {}
  void fnGetStackSize              (uint16_t unusedButMandatoryParameter) {}
  void fnStackSize                 (uint16_t unusedButMandatoryParameter) {}
  void fnStore                     (uint16_t unusedButMandatoryParameter) {}
  void fnStoreConfig               (uint16_t unusedButMandatoryParameter) {}
  void fnStoreElement              (uint16_t unusedButMandatoryParameter) {}
  void fnStoreVElement             (uint16_t unusedButMandatoryParameter) {}
  void fnStoreElementPlus          (uint16_t unusedButMandatoryParameter) {}
  void fnStoreIJ                   (uint16_t unusedButMandatoryParameter) {}
  void fnStoreStack                (uint16_t unusedButMandatoryParameter) {}
  void fnStoreAdd                  (uint16_t unusedButMandatoryParameter) {}
  void fnStoreSub                  (uint16_t unusedButMandatoryParameter) {}
  void fnStoreMult                 (uint16_t unusedButMandatoryParameter) {}
  void fnStoreDiv                  (uint16_t unusedButMandatoryParameter) {}
  void fnStoreMax                  (uint16_t unusedButMandatoryParameter) {}
  void fnStoreMin                  (uint16_t unusedButMandatoryParameter) {}
  void fnUlp                       (uint16_t unusedButMandatoryParameter) {}
  void fnUnitVector                (uint16_t unusedButMandatoryParameter) {}
  void fnVersion                   (uint16_t unusedButMandatoryParameter) {}
  void fnSquare                    (uint16_t unusedButMandatoryParameter) {}
  void fnCube                      (uint16_t unusedButMandatoryParameter) {}
  void fnFactorial                 (uint16_t unusedButMandatoryParameter) {}
  void fnSwapX                     (uint16_t unusedButMandatoryParameter) {}
  void fnSwapY                     (uint16_t unusedButMandatoryParameter) {}
  void fnSwapZ                     (uint16_t unusedButMandatoryParameter) {}
  void fnSwapT                     (uint16_t unusedButMandatoryParameter) {}
  void fnSwapXY                    (uint16_t unusedButMandatoryParameter) {}
  void fnShuffle                   (uint16_t unusedButMandatoryParameter) {}
  void fnWho                       (uint16_t unusedButMandatoryParameter) {}
  void fnGetSignificantDigits      (uint16_t unusedButMandatoryParameter) {}
  void fnSetSignificantDigits      (uint16_t unusedButMandatoryParameter) {}
  void fnSetBaseNr                 (uint16_t unusedButMandatoryParameter) {}
  void fnGetFractionDigits         (uint16_t unusedButMandatoryParameter) {}
  void fnSetFractionDigits         (uint16_t unusedButMandatoryParameter) {}
  void fnSdl                       (uint16_t unusedButMandatoryParameter) {}
  void fnSdr                       (uint16_t unusedButMandatoryParameter) {}
  void fnCvtToCurrentAngularMode   (uint16_t unusedButMandatoryParameter) {}
  void fnCvtDbRatio                (uint16_t unusedButMandatoryParameter) {}
  void fnCvtRatioDb                (uint16_t unusedButMandatoryParameter) {}
  void fnCvtCToF                   (uint16_t unusedButMandatoryParameter) {}
  void fnCvtFToC                   (uint16_t unusedButMandatoryParameter) {}
  void fnCvtFromCurrentAngularMode (uint16_t unusedButMandatoryParameter) {}
  void fnCvtDmsToCurrentAngularMode(uint16_t unusedButMandatoryParameter) {}
  void fnCvtHMSHR                  (uint16_t unusedButMandatoryParameter) {}
  void fnCvtDegRad                 (uint16_t unusedButMandatoryParameter) {}
  void fnCvtDegGrad                (uint16_t unusedButMandatoryParameter) {}
  void fnCvtGradRad                (uint16_t unusedButMandatoryParameter) {}
  void addItemToBuffer             (uint16_t unusedButMandatoryParameter) {}
  void fnOff                       (uint16_t unusedButMandatoryParameter) {}
  void fnAim                       (uint16_t unusedButMandatoryParameter) {}
  void fnView                      (uint16_t unusedButMandatoryParameter) {}
  void fnAview                     (uint16_t unusedButMandatoryParameter) {}
  void fnPrompt                    (uint16_t unusedButMandatoryParameter) {}
  void fnLastX                     (uint16_t unusedButMandatoryParameter) {}
  void fnCyx                       (uint16_t unusedButMandatoryParameter) {}
  void fnPyx                       (uint16_t unusedButMandatoryParameter) {}
  void fnDateToJulian              (uint16_t unusedButMandatoryParameter) {}
  void fnToDate                    (uint16_t unusedButMandatoryParameter) {}
  void fnYYDflt                    (uint16_t unusedButMandatoryParameter) {}
  void fnDateTo                    (uint16_t unusedButMandatoryParameter) {}
  void fnXToDate                   (uint16_t unusedButMandatoryParameter) {}
  void fnYear                      (uint16_t unusedButMandatoryParameter) {}
  void fnMonth                     (uint16_t unusedButMandatoryParameter) {}
  void fnDay                       (uint16_t unusedButMandatoryParameter) {}
  void fnWday                      (uint16_t unusedButMandatoryParameter) {}
  void fnToHr                      (uint16_t unusedButMandatoryParameter) {}
  void fnHRtoTM                    (uint16_t unusedButMandatoryParameter) {}
  void fnHMStoTM                   (uint16_t unusedButMandatoryParameter) {}
  void fnFrom_ymd                  (uint16_t unusedButMandatoryParameter) {}
  void fnToReal                    (uint16_t unusedButMandatoryParameter) {}
  void fnDec                       (uint16_t unusedButMandatoryParameter) {}
  void fnInc                       (uint16_t unusedButMandatoryParameter) {}
  void fnIse                       (uint16_t unusedButMandatoryParameter) {}
  void fnIsg                       (uint16_t unusedButMandatoryParameter) {}
  void fnIsz                       (uint16_t unusedButMandatoryParameter) {}
  void fnDse                       (uint16_t unusedButMandatoryParameter) {}
  void fnDsl                       (uint16_t unusedButMandatoryParameter) {}
  void fnDsz                       (uint16_t unusedButMandatoryParameter) {}
  void fncountBits                 (uint16_t unusedButMandatoryParameter) {}
  void fnLogicalNot                (uint16_t unusedButMandatoryParameter) {}
  void fnLogicalAnd                (uint16_t unusedButMandatoryParameter) {}
  void fnLogicalNand               (uint16_t unusedButMandatoryParameter) {}
  void fnLogicalOr                 (uint16_t unusedButMandatoryParameter) {}
  void fnLogicalNor                (uint16_t unusedButMandatoryParameter) {}
  void fnLogicalXor                (uint16_t unusedButMandatoryParameter) {}
  void fnLogicalXnor               (uint16_t unusedButMandatoryParameter) {}
  void fnDecomp                    (uint16_t unusedButMandatoryParameter) {}
  void fnPlotStat                  (uint16_t unusedButMandatoryParameter) {}
  void fnSumXY                     (uint16_t unusedButMandatoryParameter) {}
  void fnMeanXY                    (uint16_t unusedButMandatoryParameter) {}
  void fnMeanX                     (uint16_t unusedButMandatoryParameter) {}
  void fnGeometricMeanXY           (uint16_t unusedButMandatoryParameter) {}
  void fnWeightedMeanX             (uint16_t unusedButMandatoryParameter) {}
  void fnHarmonicMeanXY            (uint16_t unusedButMandatoryParameter) {}
  void fnRMSMeanXY                 (uint16_t unusedButMandatoryParameter) {}
  void fnWeightedSampleStdDev      (uint16_t unusedButMandatoryParameter) {}
  void fnWeightedPopulationStdDev  (uint16_t unusedButMandatoryParameter) {}
  void fnWeightedStandardError     (uint16_t unusedButMandatoryParameter) {}
  void fnSampleStdDev              (uint16_t unusedButMandatoryParameter) {}
  void fnPopulationStdDev          (uint16_t unusedButMandatoryParameter) {}
  void fnStandardError             (uint16_t unusedButMandatoryParameter) {}
  void fnGeometricSampleStdDev     (uint16_t unusedButMandatoryParameter) {}
  void fnGeometricPopulationStdDev (uint16_t unusedButMandatoryParameter) {}
  void fnGeometricStandardError    (uint16_t unusedButMandatoryParameter) {}
  void fnPopulationCovariance      (uint16_t unusedButMandatoryParameter) {}
  void fnSampleCovariance          (uint16_t unusedButMandatoryParameter) {}
  void fnCoefficientDetermination  (uint16_t unusedButMandatoryParameter) {}
  void fnMinExpStdDev              (uint16_t unusedButMandatoryParameter) {}
  void fnMedianXY                  (uint16_t unusedButMandatoryParameter) {}
  void fnLowerQuartileXY           (uint16_t unusedButMandatoryParameter) {}
  void fnUpperQuartileXY           (uint16_t unusedButMandatoryParameter) {}
  void fnMADXY                     (uint16_t unusedButMandatoryParameter) {}
  void fnIQRXY                     (uint16_t unusedButMandatoryParameter) {}
  void fnLINPOL                    (uint16_t unusedButMandatoryParameter) {}
  void fnPercentileXY              (uint16_t unusedButMandatoryParameter) {}
  void fnPlotCloseSmi              (uint16_t unusedButMandatoryParameter) {}
  void fnMaskl                     (uint16_t unusedButMandatoryParameter) {}
  void fnMaskr                     (uint16_t unusedButMandatoryParameter) {}
  void fnAsr                       (uint16_t unusedButMandatoryParameter) {}
  void fnCb                        (uint16_t unusedButMandatoryParameter) {}
  void fnSb                        (uint16_t unusedButMandatoryParameter) {}
  void fnFb                        (uint16_t unusedButMandatoryParameter) {}
  void fnBs                        (uint16_t unusedButMandatoryParameter) {}
  void fnBc                        (uint16_t unusedButMandatoryParameter) {}
  void fnSl                        (uint16_t unusedButMandatoryParameter) {}
  void fnRl                        (uint16_t unusedButMandatoryParameter) {}
  void fnRlc                       (uint16_t unusedButMandatoryParameter) {}
  void fnSr                        (uint16_t unusedButMandatoryParameter) {}
  void fnRr                        (uint16_t unusedButMandatoryParameter) {}
  void fnRrc                       (uint16_t unusedButMandatoryParameter) {}
  void fnLj                        (uint16_t unusedButMandatoryParameter) {}
  void fnRj                        (uint16_t unusedButMandatoryParameter) {}
  void fnCountBits                 (uint16_t unusedButMandatoryParameter) {}
  void fnSwapEndian                (uint16_t unusedButMandatoryParameter) {}
  void fnNextPrime                 (uint16_t unusedButMandatoryParameter) {}
  void fnPrimeFactors              (uint16_t unusedButMandatoryParameter) {}
  void fnEvPFacts                  (uint16_t unusedButMandatoryParameter) {}
  void fnArg                       (uint16_t unusedButMandatoryParameter) {}
  void fnRange                     (uint16_t unusedButMandatoryParameter) {}
  void fnGetRange                  (uint16_t unusedButMandatoryParameter) {}
  void fnHide                      (uint16_t unusedButMandatoryParameter) {}
  void fnGetHide                   (uint16_t unusedButMandatoryParameter) {}
  void fnGetLastErr                (uint16_t unusedButMandatoryParameter) {}
  void fnDot                       (uint16_t unusedButMandatoryParameter) {}
  void fnCross                     (uint16_t unusedButMandatoryParameter) {}
  void fnPercent                   (uint16_t unusedButMandatoryParameter) {}
  void fnPercentMRR                (uint16_t unusedButMandatoryParameter) {}
  void fnPercentT                  (uint16_t unusedButMandatoryParameter) {}
  void fnPercentSigma              (uint16_t unusedButMandatoryParameter) {}
  void fnPercentPlusMG             (uint16_t unusedButMandatoryParameter) {}
  void fnDeltaPercent              (uint16_t unusedButMandatoryParameter) {}
  void fnXthRoot                   (uint16_t unusedButMandatoryParameter) {}
  void fnGetSystemFlag             (uint16_t unusedButMandatoryParameter) {}
  void fnFractionType              (uint16_t unusedButMandatoryParameter) {}
  void fnAlphaLeng                 (uint16_t unusedButMandatoryParameter) {}
  void fnAlphaSR                   (uint16_t unusedButMandatoryParameter) {}
  void fnAlphaSL                   (uint16_t unusedButMandatoryParameter) {}
  void fnAlphaRR                   (uint16_t unusedButMandatoryParameter) {}
  void fnAlphaRL                   (uint16_t unusedButMandatoryParameter) {}
  void fnAlphaPos                  (uint16_t unusedButMandatoryParameter) {}
  void fnXToAlpha                  (uint16_t unusedButMandatoryParameter) {}
  void fnAlphaToX                  (uint16_t unusedButMandatoryParameter) {}
  void fnTicks                     (uint16_t unusedButMandatoryParameter) {}
  void fnLastT                     (uint16_t unusedButMandatoryParameter) {}
  void fnSetFirstGregorianDay      (uint16_t unusedButMandatoryParameter) {}
  void fnGetFirstGregorianDay      (uint16_t unusedButMandatoryParameter) {}
  void fnDate                      (uint16_t unusedButMandatoryParameter) {}
  void fnTime                      (uint16_t unusedButMandatoryParameter) {}
  void fnTone                      (uint16_t unusedButMandatoryParameter) {}
  void fnBeep                      (uint16_t unusedButMandatoryParameter) {}
  void fnSave                      (uint16_t unusedButMandatoryParameter) {}
  void fnSaveAuto                  (uint16_t unusedButMandatoryParameter) {}
  void fnLoad                      (uint16_t unusedButMandatoryParameter) {}
  void fnSaveProgram               (uint16_t unusedButMandatoryParameter) {}
  void fnSaveAllPrograms           (uint16_t unusedButMandatoryParameter) {}
  void fnExportProgram             (uint16_t unusedButMandatoryParameter) {}
  void fnLoadProgram               (uint16_t unusedButMandatoryParameter) {}
  void fnDeleteBackup              (uint16_t unusedButMandatoryParameter) {}
  void fnUndo                      (uint16_t unusedButMandatoryParameter) {}
  void fnXmax                      (uint16_t unusedButMandatoryParameter) {}
  void fnXmin                      (uint16_t unusedButMandatoryParameter) {}
  void fnRangeXY                   (uint16_t unusedButMandatoryParameter) {}
  void fnFib                       (uint16_t unusedButMandatoryParameter) {}
  void fnGd                        (uint16_t unusedButMandatoryParameter) {}
  void fnInvGd                     (uint16_t unusedButMandatoryParameter) {}
  void fnClP                       (uint16_t unusedButMandatoryParameter) {}
  void fnPem                       (uint16_t unusedButMandatoryParameter) {}
  void fnGoto                      (uint16_t unusedButMandatoryParameter) {}
  void fnGotoDot                   (uint16_t unusedButMandatoryParameter) {}
  void fnExecute                   (uint16_t unusedButMandatoryParameter) {}
  void fnReturn                    (uint16_t unusedButMandatoryParameter) {}
  void fnRunProgram                (uint16_t unusedButMandatoryParameter) {}
  void fnStopProgram               (uint16_t unusedButMandatoryParameter) {}
  void fnBst                       (uint16_t unusedButMandatoryParameter) {}
  void fnSst                       (uint16_t unusedButMandatoryParameter) {}
  void fnBack                      (uint16_t unusedButMandatoryParameter) {}
  void fnSkip                      (uint16_t unusedButMandatoryParameter) {}
  void fnCase                      (uint16_t unusedButMandatoryParameter) {}
  void fnCheckLabel                (uint16_t unusedButMandatoryParameter) {}
  void fnIsTopRoutine              (uint16_t unusedButMandatoryParameter) {}
  void fnRaiseError                (uint16_t unusedButMandatoryParameter) {}
  void fnErrorMessage              (uint16_t unusedButMandatoryParameter) {}
  void fnRegClr                    (uint16_t unusedButMandatoryParameter) {}
  void fnRegCopy                   (uint16_t unusedButMandatoryParameter) {}
  void fnRegSort                   (uint16_t unusedButMandatoryParameter) {}
  void fnRegSwap                   (uint16_t unusedButMandatoryParameter) {}
  void fnInput                     (uint16_t unusedButMandatoryParameter) {}
  void fnClCVar                    (uint16_t unusedButMandatoryParameter) {}
  void fnVarMnu                    (uint16_t unusedButMandatoryParameter) {}
  void fnDynamicMenu               (uint16_t unusedButMandatoryParameter) {}
  void fnPause                     (uint16_t unusedButMandatoryParameter) {}
  void fnKey                       (uint16_t unusedButMandatoryParameter) {}
  void fnKeyType                   (uint16_t unusedButMandatoryParameter) {}
  void fnPutKey                    (uint16_t unusedButMandatoryParameter) {}
  void fnKeyGtoXeq                 (uint16_t unusedButMandatoryParameter) {}
  void fnKeyGto                    (uint16_t unusedButMandatoryParameter) {}
  void fnKeyXeq                    (uint16_t unusedButMandatoryParameter) {}
  void fnProgrammableMenu          (uint16_t unusedButMandatoryParameter) {}
  void fnClearMenu                 (uint16_t unusedButMandatoryParameter) {}
  void fnExitAllMenus              (uint16_t unusedButMandatoryParameter) {}
  void fnEntryQ                    (uint16_t unusedButMandatoryParameter) {}
  void fnCheckInteger              (uint16_t unusedButMandatoryParameter) {}
  void fnNormalP                   (uint16_t unusedButMandatoryParameter) {}
  void fnNormalL                   (uint16_t unusedButMandatoryParameter) {}
  void fnNormalR                   (uint16_t unusedButMandatoryParameter) {}
  void fnNormalI                   (uint16_t unusedButMandatoryParameter) {}
  void fnT_P                       (uint16_t unusedButMandatoryParameter) {}
  void fnT_L                       (uint16_t unusedButMandatoryParameter) {}
  void fnT_R                       (uint16_t unusedButMandatoryParameter) {}
  void fnT_I                       (uint16_t unusedButMandatoryParameter) {}
  void fnChi2P                     (uint16_t unusedButMandatoryParameter) {}
  void fnChi2L                     (uint16_t unusedButMandatoryParameter) {}
  void fnChi2R                     (uint16_t unusedButMandatoryParameter) {}
  void fnChi2I                     (uint16_t unusedButMandatoryParameter) {}
  void fnStdNormalP                (uint16_t unusedButMandatoryParameter) {}
  void fnStdNormalL                (uint16_t unusedButMandatoryParameter) {}
  void fnStdNormalR                (uint16_t unusedButMandatoryParameter) {}
  void fnStdNormalI                (uint16_t unusedButMandatoryParameter) {}
  void fnF_P                       (uint16_t unusedButMandatoryParameter) {}
  void fnF_L                       (uint16_t unusedButMandatoryParameter) {}
  void fnF_R                       (uint16_t unusedButMandatoryParameter) {}
  void fnF_I                       (uint16_t unusedButMandatoryParameter) {}
  void fnProcessLR                 (uint16_t unusedButMandatoryParameter) {}
  void fnPlotZoom                  (uint16_t unusedButMandatoryParameter) {}
  void fnYIsFnx                    (uint16_t unusedButMandatoryParameter) {}
  void fnXIsFny                    (uint16_t unusedButMandatoryParameter) {}
  void fnStatSa                    (uint16_t unusedButMandatoryParameter) {}
  void fnCurveFittingLR            (uint16_t unusedButMandatoryParameter) {}
  void fnLogNormalP                (uint16_t unusedButMandatoryParameter) {}
  void fnLogNormalL                (uint16_t unusedButMandatoryParameter) {}
  void fnLogNormalR                (uint16_t unusedButMandatoryParameter) {}
  void fnLogNormalI                (uint16_t unusedButMandatoryParameter) {}
  void fnCauchyP                   (uint16_t unusedButMandatoryParameter) {}
  void fnCauchyL                   (uint16_t unusedButMandatoryParameter) {}
  void fnCauchyR                   (uint16_t unusedButMandatoryParameter) {}
  void fnCauchyI                   (uint16_t unusedButMandatoryParameter) {}
  void fnExponentialP              (uint16_t unusedButMandatoryParameter) {}
  void fnExponentialL              (uint16_t unusedButMandatoryParameter) {}
  void fnExponentialR              (uint16_t unusedButMandatoryParameter) {}
  void fnExponentialI              (uint16_t unusedButMandatoryParameter) {}
  void fnLogisticP                 (uint16_t unusedButMandatoryParameter) {}
  void fnLogisticL                 (uint16_t unusedButMandatoryParameter) {}
  void fnLogisticR                 (uint16_t unusedButMandatoryParameter) {}
  void fnLogisticI                 (uint16_t unusedButMandatoryParameter) {}
  void fnParetoP                   (uint16_t unusedButMandatoryParameter) {}
  void fnParetoL                   (uint16_t unusedButMandatoryParameter) {}
  void fnParetoU                   (uint16_t unusedButMandatoryParameter) {}
  void fnParetoI                   (uint16_t unusedButMandatoryParameter) {}
  void fnPareto2P                  (uint16_t unusedButMandatoryParameter) {}
  void fnPareto2L                  (uint16_t unusedButMandatoryParameter) {}
  void fnPareto2U                  (uint16_t unusedButMandatoryParameter) {}
  void fnPareto2I                  (uint16_t unusedButMandatoryParameter) {}
  void fnWeibullP                  (uint16_t unusedButMandatoryParameter) {}
  void fnWeibullL                  (uint16_t unusedButMandatoryParameter) {}
  void fnWeibullR                  (uint16_t unusedButMandatoryParameter) {}
  void fnWeibullI                  (uint16_t unusedButMandatoryParameter) {}
  void fnGEVP                      (uint16_t unusedButMandatoryParameter) {}
  void fnGEVL                      (uint16_t unusedButMandatoryParameter) {}
  void fnGEVR                      (uint16_t unusedButMandatoryParameter) {}
  void fnGEVI                      (uint16_t unusedButMandatoryParameter) {}
  void fnNegBinomialP              (uint16_t unusedButMandatoryParameter) {}
  void fnNegBinomialL              (uint16_t unusedButMandatoryParameter) {}
  void fnNegBinomialR              (uint16_t unusedButMandatoryParameter) {}
  void fnNegBinomialI              (uint16_t unusedButMandatoryParameter) {}
  void fnGeometricP                (uint16_t unusedButMandatoryParameter) {}
  void fnGeometricL                (uint16_t unusedButMandatoryParameter) {}
  void fnGeometricR                (uint16_t unusedButMandatoryParameter) {}
  void fnGeometricI                (uint16_t unusedButMandatoryParameter) {}
  void fnHypergeometricP           (uint16_t unusedButMandatoryParameter) {}
  void fnHypergeometricL           (uint16_t unusedButMandatoryParameter) {}
  void fnHypergeometricR           (uint16_t unusedButMandatoryParameter) {}
  void fnHypergeometricI           (uint16_t unusedButMandatoryParameter) {}
  void fnBinomialP                 (uint16_t unusedButMandatoryParameter) {}
  void fnBinomialL                 (uint16_t unusedButMandatoryParameter) {}
  void fnBinomialR                 (uint16_t unusedButMandatoryParameter) {}
  void fnBinomialI                 (uint16_t unusedButMandatoryParameter) {}
  void fnPoissonP                  (uint16_t unusedButMandatoryParameter) {}
  void fnPoissonL                  (uint16_t unusedButMandatoryParameter) {}
  void fnPoissonR                  (uint16_t unusedButMandatoryParameter) {}
  void fnPoissonI                  (uint16_t unusedButMandatoryParameter) {}
  void fnUniformP                  (uint16_t unusedButMandatoryParameter) {}
  void fnUniformL                  (uint16_t unusedButMandatoryParameter) {}
  void fnUniformU                  (uint16_t unusedButMandatoryParameter) {}
  void fnUniformI                  (uint16_t unusedButMandatoryParameter) {}
  void fnNewMatrix                 (uint16_t unusedButMandatoryParameter) {}
  void fnEditMatrix                (uint16_t unusedButMandatoryParameter) {}
  void fnOldMatrix                 (uint16_t unusedButMandatoryParameter) {}
  void fnGoToElement               (uint16_t unusedButMandatoryParameter) {}
  void fnGoToRow                   (uint16_t unusedButMandatoryParameter) {}
  void fnGoToColumn                (uint16_t unusedButMandatoryParameter) {}
  void fnSetGrowMode               (uint16_t unusedButMandatoryParameter) {}
  void fnIncDecI                   (uint16_t unusedButMandatoryParameter) {}
  void fnIncDecJ                   (uint16_t unusedButMandatoryParameter) {}
  void fnInsRow                    (uint16_t unusedButMandatoryParameter) {}
  void fnDelRow                    (uint16_t unusedButMandatoryParameter) {}
  void fnInsCol                    (uint16_t unusedButMandatoryParameter) {}
  void fnDelCol                    (uint16_t unusedButMandatoryParameter) {}
  void fnAddRow                    (uint16_t unusedButMandatoryParameter) {}
  void fnAddCol                    (uint16_t unusedButMandatoryParameter) {}
  void fnSetMatrixDimensions       (uint16_t unusedButMandatoryParameter) {}
  void fnSetMatrixDimensionsGr     (uint16_t unusedButMandatoryParameter) {}
  void fnGetMatrixDimensions       (uint16_t unusedButMandatoryParameter) {}
  void fnTranspose                 (uint16_t unusedButMandatoryParameter) {}
  void fnLuDecomposition           (uint16_t unusedButMandatoryParameter) {}
  void fnDeterminant               (uint16_t unusedButMandatoryParameter) {}
  void fnInvertMatrix              (uint16_t unusedButMandatoryParameter) {}
  void fnEuclideanNorm             (uint16_t unusedButMandatoryParameter) {}
  void fnRowSum                    (uint16_t unusedButMandatoryParameter) {}
  void fnRowNorm                   (uint16_t unusedButMandatoryParameter) {}
  void fnVectorAngle               (uint16_t unusedButMandatoryParameter) {}
  void fnIndexMatrix               (uint16_t unusedButMandatoryParameter) {}
  void fnGetMatrix                 (uint16_t unusedButMandatoryParameter) {}
  void fnPutMatrix                 (uint16_t unusedButMandatoryParameter) {}
  void fnSwapRows                  (uint16_t unusedButMandatoryParameter) {}
  void fnSimultaneousLinearEquation(uint16_t unusedButMandatoryParameter) {}
  void fnEditLinearEquationMatrixA (uint16_t unusedButMandatoryParameter) {}
  void fnEditLinearEquationMatrixB (uint16_t unusedButMandatoryParameter) {}
  void fnEditLinearEquationMatrixX (uint16_t unusedButMandatoryParameter) {}
  void fnQrDecomposition           (uint16_t unusedButMandatoryParameter) {}
  void fnEigenvalues               (uint16_t unusedButMandatoryParameter) {}
  void fnEigenvectors              (uint16_t unusedButMandatoryParameter) {}
  void fnJacobiSn                  (uint16_t unusedButMandatoryParameter) {}
  void fnJacobiCn                  (uint16_t unusedButMandatoryParameter) {}
  void fnJacobiDn                  (uint16_t unusedButMandatoryParameter) {}
  void fnJacobiAmplitude           (uint16_t unusedButMandatoryParameter) {}
  void fnEllipticK                 (uint16_t unusedButMandatoryParameter) {}
  void fnEllipticE                 (uint16_t unusedButMandatoryParameter) {}
  void fnEllipticPi                (uint16_t unusedButMandatoryParameter) {}
  void fnEllipticFphi              (uint16_t unusedButMandatoryParameter) {}
  void fnEllipticEphi              (uint16_t unusedButMandatoryParameter) {}
  void fnJacobiZeta                (uint16_t unusedButMandatoryParameter) {}
  void fnPgmSlv                    (uint16_t unusedButMandatoryParameter) {}
  void fnSolve                     (uint16_t unusedButMandatoryParameter) {}
  void fnSolveVar                  (uint16_t unusedButMandatoryParameter) {}
  void fnPgmInt                    (uint16_t unusedButMandatoryParameter) {}
  void fnIntegrate                 (uint16_t unusedButMandatoryParameter) {}
  void fnIntegrateYX               (uint16_t unusedButMandatoryParameter) {}
  void fnIntVar                    (uint16_t unusedButMandatoryParameter) {}
  void fn1stDeriv                  (uint16_t unusedButMandatoryParameter) {}
  void fn2ndDeriv                  (uint16_t unusedButMandatoryParameter) {}
  void fn1stDerivEq                (uint16_t unusedButMandatoryParameter) {}
  void fn2ndDerivEq                (uint16_t unusedButMandatoryParameter) {}
  void fnEqDelete                  (uint16_t unusedButMandatoryParameter) {}
  void fnEqEdit                    (uint16_t unusedButMandatoryParameter) {}
  void fnEqNew                     (uint16_t unusedButMandatoryParameter) {}
  void fnEqCursorLeft              (uint16_t unusedButMandatoryParameter) {}
  void fnEqCursorRight             (uint16_t unusedButMandatoryParameter) {}
  void fnEqCalc                    (uint16_t unusedButMandatoryParameter) {}
  void fnProgrammableSum           (uint16_t unusedButMandatoryParameter) {}
  void fnProgrammableProduct       (uint16_t unusedButMandatoryParameter) {}
  void fnProgrammableiSum          (uint16_t unusedButMandatoryParameter) {}
  void fnProgrammableiProduct      (uint16_t unusedButMandatoryParameter) {}
  void fnTvmVar                    (uint16_t unusedButMandatoryParameter) {}
  void fnTvmBeginMode              (uint16_t unusedButMandatoryParameter) {}
  void fnTvmEndMode                (uint16_t unusedButMandatoryParameter) {}
  void fnAssign                    (uint16_t unusedButMandatoryParameter) {}
  void fnItemTimerApp              (uint16_t unusedButMandatoryParameter) {}
  void fnAddTimerApp               (uint16_t unusedButMandatoryParameter) {}
  void fnAddLapTimerApp            (uint16_t unusedButMandatoryParameter) {}
  void fnRegAddTimerApp            (uint16_t unusedButMandatoryParameter) {}
  void fnRegAddLapTimerApp         (uint16_t unusedButMandatoryParameter) {}
  void fnDecisecondTimerApp        (uint16_t unusedButMandatoryParameter) {}
  void fnResetTimerApp             (uint16_t unusedButMandatoryParameter) {}
  void fnRecallTimerApp            (uint16_t unusedButMandatoryParameter) {}
  void fnStartStopTimerApp         (uint16_t unusedButMandatoryParameter) {}
  void fnEqSolvGraph               (uint16_t unusedButMandatoryParameter) {}
  void graph_eqn                   (uint16_t unusedButMandatoryParameter) {}
  void fnClLcd                     (uint16_t unusedButMandatoryParameter) {}
  void fnPixel                     (uint16_t unusedButMandatoryParameter) {}
  void fnPoint                     (uint16_t unusedButMandatoryParameter) {}
  void fnAGraph                    (uint16_t unusedButMandatoryParameter) {}
  void fnSetLoBin                  (uint16_t unusedButMandatoryParameter) {}
  void fnSetHiBin                  (uint16_t unusedButMandatoryParameter) {}
  void fnSetNBins                  (uint16_t unusedButMandatoryParameter) {}
  void fnConvertStatsToHisto       (uint16_t unusedButMandatoryParameter) {}
  void fnSqrt1Px2                  (uint16_t unusedButMandatoryParameter) {}
  void fnDeleteVariable            (uint16_t unusedButMandatoryParameter) {}
  void fnDeleteMenu                (uint16_t unusedButMandatoryParameter) {}
  void activateUSBdisk             (uint16_t unusedButMandatoryParameter) {}

  void fnJM                       (uint16_t unusedButMandatoryParameter) {}           //vv JM
  void SetSetting                 (uint16_t unusedButMandatoryParameter) {}
  void fnDisplayFormatSigFig      (uint16_t unusedButMandatoryParameter) {}
  void fnDisplayFormatUnit        (uint16_t unusedButMandatoryParameter) {}
  void fnDisplayFormatCycle       (uint16_t unusedButMandatoryParameter) {}
  void fnKeysManagement           (uint16_t unusedButMandatoryParameter) {}
  void fnSigmaAssign              (uint16_t unusedButMandatoryParameter) {}
  void fnGetSigmaAssignToX        (uint16_t unusedButMandatoryParameter) {}
  void fnInDefault                (uint16_t unusedButMandatoryParameter) {}
  void fnJM_2SI                   (uint16_t unusedButMandatoryParameter) {}
  void fnTo_ms                    (uint16_t unusedButMandatoryParameter) {}
  void fnFrom_ms                  (uint16_t unusedButMandatoryParameter) {}
  void fnC47Show                  (uint16_t unusedButMandatoryParameter) {}
  void fnP_All_Regs               (uint16_t unusedButMandatoryParameter) {}
  void fnP_Regs                   (uint16_t unusedButMandatoryParameter) {}
  void fnToPolar2                 (uint16_t unusedButMandatoryParameter) {}
  void fnToRect2                  (uint16_t unusedButMandatoryParameter) {}
  void fnToPolar_HP               (uint16_t unusedButMandatoryParameter) {}
  void fnToRect_HP                (uint16_t unusedButMandatoryParameter) {}
  void fnToPolar_CX               (uint16_t unusedButMandatoryParameter) {}
  void fnToRect_CX                (uint16_t unusedButMandatoryParameter) {}
  void fnMultiplySI               (uint16_t unusedButMandatoryParameter) {}
  void fn_cnst_op_j               (uint16_t unusedButMandatoryParameter) {}
  void fn_cnst_op_j_pol           (uint16_t unusedButMandatoryParameter) {}
  void fn_cnst_op_aa              (uint16_t unusedButMandatoryParameter) {}
  void fn_cnst_op_a               (uint16_t unusedButMandatoryParameter) {}
  void fn_cnst_0_cpx              (uint16_t unusedButMandatoryParameter) {}
  void fn_cnst_1_cpx              (uint16_t unusedButMandatoryParameter) {}
  void fnListXY                   (uint16_t unusedButMandatoryParameter) {}
  void flagBrowser_old            (uint16_t unusedButMandatoryParameter) {}
  void fnSetInlineTest            (uint16_t unusedButMandatoryParameter) {}           //vv dr
  void fnSetInlineTestXToBs       (uint16_t unusedButMandatoryParameter) {}
  void fnGetInlineTestBsToX       (uint16_t unusedButMandatoryParameter) {}
  void fnSysFreeMem               (uint16_t unusedButMandatoryParameter) {}           //^^
  void fnT_ARROW                  (uint16_t unusedButMandatoryParameter) {}
  void fnXSWAP                    (uint16_t unusedButMandatoryParameter) {}
  void fnAngularModeJM            (uint16_t unusedButMandatoryParameter) {}
  void fnChangeBaseJM             (uint16_t unusedButMandatoryParameter) {}
  void fnChangeBaseMNU            (uint16_t unusedButMandatoryParameter) {}
  void fnByteShortcutsS           (uint16_t unusedButMandatoryParameter) {}  //JM POC BASE2
  void fnByteShortcutsU           (uint16_t unusedButMandatoryParameter) {}  //JM POC BASE2
  void fnClrMod                   (uint16_t unusedButMandatoryParameter) {}  //JM POC BASE2
  void fnShoiXRepeats             (uint16_t unusedButMandatoryParameter) {}  //JM SHOIDISP
  void fnDumpMenus                (uint16_t unusedButMandatoryParameter) {}  //JM
  void fnCFGsettings              (uint16_t unusedButMandatoryParameter) {}
  void fnPline                    (uint16_t unusedButMandatoryParameter) {}
  void fnPcros                    (uint16_t unusedButMandatoryParameter) {}
  void fnPplus                    (uint16_t unusedButMandatoryParameter) {}
  void fnPbox                     (uint16_t unusedButMandatoryParameter) {}
  void fnPcurve                   (uint16_t unusedButMandatoryParameter) {}
  void fnPintg                    (uint16_t unusedButMandatoryParameter) {}
  void fnPdiff                    (uint16_t unusedButMandatoryParameter) {}
  void fnPrms                     (uint16_t unusedButMandatoryParameter) {}
  void fnPvect                    (uint16_t unusedButMandatoryParameter) {}
  void fnPNvect                   (uint16_t unusedButMandatoryParameter) {}
  void fnScale                    (uint16_t unusedButMandatoryParameter) {}
  void fnPx                       (uint16_t unusedButMandatoryParameter) {}
  void fnPy                       (uint16_t unusedButMandatoryParameter) {}
  void fnPshade                   (uint16_t unusedButMandatoryParameter) {}
  void fnComplexPlot              (uint16_t unusedButMandatoryParameter) {}
  void fnPMzoom                   (uint16_t unusedButMandatoryParameter) {}
  void fnCla                      (uint16_t unusedButMandatoryParameter) {}
  void fnCln                      (uint16_t unusedButMandatoryParameter) {}
  void fnClGrf                    (uint16_t unusedButMandatoryParameter) {}
  void fnMinute                   (uint16_t unusedButMandatoryParameter) {}
  void fnSecond                   (uint16_t unusedButMandatoryParameter) {}
  void fnHrDeg                    (uint16_t unusedButMandatoryParameter) {}
  void fnToTime                   (uint16_t unusedButMandatoryParameter) {}
  void fnTimeTo                   (uint16_t unusedButMandatoryParameter) {}
  void fnDRG                      (uint16_t unusedButMandatoryParameter) {}
  void fnPlotReset                (uint16_t unusedButMandatoryParameter) {}
  void fnCurveFittingReset        (uint16_t unusedButMandatoryParameter) {}
  void fnCurveFitting_T           (uint16_t unusedButMandatoryParameter) {}
  void fnSHIFTf                   (uint16_t unusedButMandatoryParameter) {}
  void fnSHIFTg                   (uint16_t unusedButMandatoryParameter) {}
  void fnSHIFTfg                  (uint16_t unusedButMandatoryParameter) {}
  void graph_stat                 (uint16_t unusedButMandatoryParameter) {}
  void fnSafeReset                (uint16_t unusedButMandatoryParameter) {}
  void fnSetBCD                   (uint16_t unusedButMandatoryParameter) {}
  void fnLongPressSwitches        (uint16_t unusedButMandatoryParameter) {}
  void fnSetSI_All                (uint16_t unusedButMandatoryParameter) {}
  void fnJulianToDateTime         (uint16_t unusedButMandatoryParameter) {}
  void fnDateTimeToJulian         (uint16_t unusedButMandatoryParameter) {}
  void fnEulersFormula            (uint16_t unusedButMandatoryParameter) {}
  void fnSNAP                     (uint16_t unusedButMandatoryParameter) {}
  void fnPcSigmaDeltaPcXmean      (uint16_t unusedButMandatoryParameter) {}
  void fnDeltaPercentXmean        (uint16_t unusedButMandatoryParameter) {}
  void setFGLSettings             (uint16_t unusedButMandatoryParameter) {}
  void fnSettingsDispFormatGrpL   (uint16_t unusedButMandatoryParameter) {}
  void fnSettingsDispFormatGrp1L  (uint16_t unusedButMandatoryParameter) {}
  void fnSettingsDispFormatGrp1Lo (uint16_t unusedButMandatoryParameter) {}
  void fnSettingsDispFormatGrpR   (uint16_t unusedButMandatoryParameter) {}
  void fnSetGapChar               (uint16_t unusedButMandatoryParameter) {}
  void fnMenuGapL                 (uint16_t unusedButMandatoryParameter) {}
  void fnMenuGapR                 (uint16_t unusedButMandatoryParameter) {}
  void fnMenuGapRX                (uint16_t unusedButMandatoryParameter) {}
  void fnDiskInfo                 (uint16_t unusedButMandatoryParameter) {}
  void fnLint                     (uint16_t unusedButMandatoryParameter) {}
  void fnSint                     (uint16_t unusedButMandatoryParameter) {}
  void fnSetVolume                (uint16_t unusedButMandatoryParameter) {}
  void fnGetVolume                (uint16_t unusedButMandatoryParameter) {}
  void fnVolumeUp                 (uint16_t unusedButMandatoryParameter) {}
  void fnVolumeDown               (uint16_t unusedButMandatoryParameter) {}
  void fnBuzz                     (uint16_t unusedButMandatoryParameter) {}
  void fnPlay                     (uint16_t unusedButMandatoryParameter) {}
  void fnUnitConvert              (uint16_t unusedButMandatoryParameter) {}
  void fnKmletok100K              (uint16_t unusedButMandatoryParameter) {}
  void fnK100Ktokmk               (uint16_t unusedButMandatoryParameter) {}
  void fnL100Tomgus               (uint16_t unusedButMandatoryParameter) {}
  void fnMgeustok100M             (uint16_t unusedButMandatoryParameter) {}
  void fnK100Ktok100M             (uint16_t unusedButMandatoryParameter) {}
  void fnL100Tomguk               (uint16_t unusedButMandatoryParameter) {}
  void fnMgeuktok100M             (uint16_t unusedButMandatoryParameter) {}
  void fnK100Mtomik               (uint16_t unusedButMandatoryParameter) {}
  void fnBaseMenu                 (uint16_t unusedButMandatoryParameter) {}
  void fnExecutePlusSkip          (uint16_t unusedButMandatoryParameter) {}
  void fnRecallPlusSkip           (uint16_t unusedButMandatoryParameter) {}
  void fnGetDmx                   (uint16_t unusedButMandatoryParameter) {}
  void fnClearUserMenus           (uint16_t unusedButMandatoryParameter) {}   //DL
  void fnClearAllVariables        (uint16_t unusedButMandatoryParameter) {}   //DL
  void fnDeleteUserMenus          (uint16_t unusedButMandatoryParameter) {}   //DL
  void fnDeleteAllVariables       (uint16_t unusedButMandatoryParameter) {}   //DL
  void fnConfirmationYes          (uint16_t unusedButMandatoryParameter) {}   //DL
  void fnConfirmationNo           (uint16_t unusedButMandatoryParameter) {}   //DL
  void fnLoadedFile               (uint16_t unusedButMandatoryParameter) {}
  void fnResetTVM                 (uint16_t unusedButMandatoryParameter) {}
  void fnEff                      (uint16_t unusedButMandatoryParameter) {}
  void fnEffToI                   (uint16_t unusedButMandatoryParameter) {}
  void pemAlphaEdit               (uint16_t unusedButMandatoryParameter) {}
  void fnOpenMenu                 (uint16_t unusedButMandatoryParameter) {}   //DL
  void fnGetMenu                  (uint16_t unusedButMandatoryParameter) {}   //DL
  void fn_cnst_op_A               (uint16_t unusedButMandatoryParameter) {}
  void fnConvertStkToMx           (uint16_t unusedButMandatoryParameter) {}
  void fnConvertMxToStk           (uint16_t unusedButMandatoryParameter) {}
  void fnSetCountDownTimerApp     (uint16_t unusedButMandatoryParameter) {}
  void fnGetType                  (uint16_t unusedButMandatoryParameter) {}
  void fnPseudoMenu               (uint16_t unusedButMandatoryParameter) {}
  void fnVectorDist               (uint16_t unusedButMandatoryParameter) {}
  void V3RectoToSph               (uint16_t unusedButMandatoryParameter) {}
  void V3RectoToCyl               (uint16_t unusedButMandatoryParameter) {}
  void fnComplexToVector          (uint16_t unusedButMandatoryParameter) {}
  void fnWeekOfYear               (uint16_t unusedButMandatoryParameter) {}
  void fnSetWeekOfYearRule        (uint16_t unusedButMandatoryParameter) {}
  void fnGetWeekOfYearRule        (uint16_t unusedButMandatoryParameter) {}
  void fnEdit                     (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn                     (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_sin                 (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_cos                 (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_tan                 (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_pi                  (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_atan2               (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_arcsin              (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_arccos              (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_arctan              (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_LN                  (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_LOG                 (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_EXP                 (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_10X                 (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_POWER               (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_SQRT                (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_1ONX                (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_ADD                 (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_SUB                 (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_MULT                (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_DIV                 (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_MOD                 (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_MODANG              (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_TO                  (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_ToDEG               (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_ToRAD               (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_STO                 (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_RCL                 (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_DRG                 (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_SQR                 (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_YRTX                (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_DUPX                (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_RDP                 (uint16_t unusedButMandatoryParameter) {}
  void fnXXfn_RSD                 (uint16_t unusedButMandatoryParameter) {}
  void fn3Sto                     (uint16_t unusedButMandatoryParameter) {}
  void fn3Rcl                     (uint16_t unusedButMandatoryParameter) {}
  void fnDupN                     (uint16_t unusedButMandatoryParameter) {}
  void fnSwapN                    (uint16_t unusedButMandatoryParameter) {}
  void fnDropN                    (uint16_t unusedButMandatoryParameter) {}
  void fn2Sto                     (uint16_t unusedButMandatoryParameter) {}
  void fn2Rcl                     (uint16_t unusedButMandatoryParameter) {}
  void fnSetHP35                  (uint16_t unusedButMandatoryParameter) {}
  void fnSetC47                   (uint16_t unusedButMandatoryParameter) {}
  void fnSetJM                    (uint16_t unusedButMandatoryParameter) {}
  void fnSetRJ                    (uint16_t unusedButMandatoryParameter) {}

#endif // GENERATE_CATALOGS || defined(GENERATE_TESTPGMS)

#ifdef TESTSUITE_BUILD
  void fnDiskInfo                 (uint16_t unusedButMandatoryParameter) {}
#endif //TEST_SUITE


#define PER_    STD_PER
#define RANGE_  STD_SUB_R STD_SUB_A STD_SUB_N STD_SUB_G STD_SUB_E    // Strings to maintain table columns below
#define SUPSUB_ STD_SUP_S STD_SUP_U STD_SUP_P STD_SUB_S STD_SUB_U STD_SUB_B
//#define TM      "T" STD_SUB_M
//#define DT      "D" STD_SUB_T
#define TM      STD_TIME_T
#define DT      STD_DATE_D
#if defined(USECURVES)
  #define conditionalPCURVE fnPcurve
#else
  #define conditionalPCURVE itemToBeCoded
#endif
#if defined(OPTION_XFN_1000)
  #define S18_fnEdit         fnEdit
  #define S18_fnXXfn         fnXXfn
  #define S18_fnXXfn_sin     fnXXfn_sin
  #define S18_fnXXfn_cos     fnXXfn_cos
  #define S18_fnXXfn_tan     fnXXfn_tan
  #define S18_fnXXfn_pi      fnXXfn_pi
  #define S18_fnXXfn_atan2   fnXXfn_atan2
  #define S18_fnXXfn_arcsin  fnXXfn_arcsin
  #define S18_fnXXfn_arccos  fnXXfn_arccos
  #define S18_fnXXfn_arctan  fnXXfn_arctan
  #define S18_fnXXfn_LN      fnXXfn_LN
  #define S18_fnXXfn_LOG     fnXXfn_LOG
  #define S18_fnXXfn_EXP     fnXXfn_EXP
  #define S18_fnXXfn_10X     fnXXfn_10X
  #define S18_fnXXfn_POWER   fnXXfn_POWER
  #define S18_fnXXfn_SQRT    fnXXfn_SQRT
  #define S18_fnXXfn_1ONX    fnXXfn_1ONX
  #define S18_fnXXfn_ADD     fnXXfn_ADD
  #define S18_fnXXfn_SUB     fnXXfn_SUB
  #define S18_fnXXfn_MULT    fnXXfn_MULT
  #define S18_fnXXfn_DIV     fnXXfn_DIV
  #define S18_fnXXfn_MOD     fnXXfn_MOD
  #define S18_fnXXfn_TO      fnXXfn_TO
  #define S18_fnXXfn_MODANG  fnXXfn_MODANG
  #define S18_fnXXfn_ToDEG   fnXXfn_ToDEG
  #define S18_fnXXfn_ToRAD   fnXXfn_ToRAD
  #define S18_MENU           CAT_MENU
  #define S18_FNCT           CAT_FNCT
  #define S18_fnXXfn_STO     fnXXfn_STO
  #define S18_fnXXfn_RCL     fnXXfn_RCL
  #define S18_fnXXfn_DRG     fnXXfn_DRG
  #define S18_fnXXfn_SQR     fnXXfn_SQR
  #define S18_fnXXfn_YRTX    fnXXfn_YRTX
  #define S18_fnXXfn_RDP     fnXXfn_RDP
  #define S18_fnXXfn_RSD     fnXXfn_RSD
#else //OPTION_XFN_1000
  #define S18_fnEdit         itemToBeCoded
  #define S18_fnXXfn         itemToBeCoded
  #define S18_fnXXfn_sin     itemToBeCoded
  #define S18_fnXXfn_cos     itemToBeCoded
  #define S18_fnXXfn_tan     itemToBeCoded
  #define S18_fnXXfn_pi      itemToBeCoded
  #define S18_fnXXfn_atan2   itemToBeCoded
  #define S18_fnXXfn_arcsin  itemToBeCoded
  #define S18_fnXXfn_arccos  itemToBeCoded
  #define S18_fnXXfn_arctan  itemToBeCoded
  #define S18_fnXXfn_LN      itemToBeCoded
  #define S18_fnXXfn_LOG     itemToBeCoded
  #define S18_fnXXfn_EXP     itemToBeCoded
  #define S18_fnXXfn_10X     itemToBeCoded
  #define S18_fnXXfn_POWER   itemToBeCoded
  #define S18_fnXXfn_SQRT    itemToBeCoded
  #define S18_fnXXfn_1ONX    itemToBeCoded
  #define S18_fnXXfn_ADD     itemToBeCoded
  #define S18_fnXXfn_SUB     itemToBeCoded
  #define S18_fnXXfn_MULT    itemToBeCoded
  #define S18_fnXXfn_DIV     itemToBeCoded
  #define S18_fnXXfn_MOD     itemToBeCoded
  #define S18_fnXXfn_TO      itemToBeCoded
  #define S18_fnXXfn_MODANG  itemToBeCoded
  #define S18_fnXXfn_ToDEG   itemToBeCoded
  #define S18_fnXXfn_ToRAD   itemToBeCoded
  #define S18_MENU           CAT_NONE
  #define S18_FNCT           CAT_NONE
  #define S18_fnXXfn_STO     itemToBeCoded
  #define S18_fnXXfn_RCL     itemToBeCoded
  #define S18_fnXXfn_DRG     itemToBeCoded
  #define S18_fnXXfn_SQR     itemToBeCoded
  #define S18_fnXXfn_YRTX    itemToBeCoded
  #define S18_fnXXfn_RDP     itemToBeCoded
  #define S18_fnXXfn_RSD     itemToBeCoded
#endif //OPTION_XFN_1000



/* Helper macro for multiplicative unit conversions */
#define UNIT_CONV(unit, invert, cat, menu)      \
            { fnUnitConvert,                unit | invert,              cat,                                            menu,                                           (0 << TAM_MAX_BITS) |    0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         }

TO_QSPI const item_t indexOfItems[] = {

//This list is generated (manually) from items3.xlsx, EXPORT.C

//jj
//            function                      parameter                    item in catalog                                item in softmenu                               TAM min                 max  CATALOG    stackLift       UNDO status    EIM status     In program
/*    0 */  { itemToBeCoded,                NOPARAM,                     "",                                            "0000",                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         }, // ITM_NULL

// Items from 1 to 127 are 1 byte OP codes
/*    1 */  { fnNop,                        TM_LABEL,                    "LBL",                                         "LBL",                                         (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DECLARE_LABEL| HG_ENABLED         },
/*    2 */  { fnGoto,                       TM_LABEL,                    "GTO",                                         "GTO",                                         (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_LABEL        | HG_ENABLED         },
/*    3 */  { fnExecute,                    TM_LABEL,                    "XEQ",                                         "XEQ",                                         (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_LABEL        | HG_ENABLED         },
/*    4 */  { fnReturn,                     0,                           "RTN",                                         "RTN",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*    5 */  { fnIse,                        TM_REGISTER,                 "ISE",                                         "ISE",                                         (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/*    6 */  { fnIsg,                        TM_REGISTER,                 "ISG",                                         "ISG",                                         (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/*    7 */  { fnIsz,                        TM_REGISTER,                 "ISZ",                                         "ISZ",                                         (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/*    8 */  { fnDse,                        TM_REGISTER,                 "DSE",                                         "DSE",                                         (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/*    9 */  { fnDsl,                        TM_REGISTER,                 "DSL",                                         "DSL",                                         (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/*   10 */  { fnDsz,                        TM_REGISTER,                 "DSZ",                                         "DSZ",                                         (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/*   11 */  { fnXEqualsTo,                  TM_CMP,                      "x= ?",                                        "x= ?",                                        (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_COMPARE      | HG_ENABLED         },
/*   12 */  { fnXNotEqual,                  TM_CMP,                      "x" STD_NOT_EQUAL " ?",                        "x" STD_NOT_EQUAL " ?",                        (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_COMPARE      | HG_ENABLED         },
/*   13 */  { fnCheckPlusZero,              NOPARAM,                     "x=+0?",                                       "x=+0?",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*   14 */  { fnCheckMinusZero,             NOPARAM,                     "x=-0?",                                       "x=-0?",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*   15 */  { fnXAlmostEqual,               TM_CMP,                      "x" STD_ALMOST_EQUAL " ?",                     "x" STD_ALMOST_EQUAL " ?",                     (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_COMPARE      | HG_ENABLED         },
/*   16 */  { fnXLessThan,                  TM_CMP,                      "x< ?",                                        "x< ?",                                        (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_COMPARE      | HG_ENABLED         },
/*   17 */  { fnXLessEqual,                 TM_CMP,                      "x" STD_LESS_EQUAL " ?",                       "x" STD_LESS_EQUAL " ?",                       (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_COMPARE      | HG_ENABLED         },
/*   18 */  { fnXGreaterEqual,              TM_CMP,                      "x" STD_GREATER_EQUAL " ?",                    "x" STD_GREATER_EQUAL " ?",                    (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_COMPARE      | HG_ENABLED         },
/*   19 */  { fnXGreaterThan,               TM_CMP,                      "x> ?",                                        "x> ?",                                        (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_COMPARE      | HG_ENABLED         },
/*   20 */  { fnIsFlagClear,                TM_FLAGR,                    "FC?",                                         "FC?",                                         (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_FLAG         | HG_ENABLED         },
/*   21 */  { fnIsFlagSet,                  TM_FLAGR,                    "FS?",                                         "FS?",                                         (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_FLAG         | HG_ENABLED         },
/*   22 */  { fnCheckInteger,               CHECK_INTEGER_EVEN,          "EVEN?",                                       "EVEN?",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*   23 */  { fnCheckInteger,               CHECK_INTEGER_ODD,           "ODD?",                                        "ODD?",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*   24 */  { fnCheckInteger,               CHECK_INTEGER_FP,            "FP?",                                         "FP?",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*   25 */  { fnCheckInteger,               CHECK_INTEGER,               "INT?",                                        "INT?",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*   26 */  { fnCheckType,                  dtComplex34,                 STD_COMPLEX_C "?",                             STD_COMPLEX_C "?",                             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*   27 */  { fnCheckMatrix,                NOPARAM,                     "MATR?",                                       "MATR?",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*   28 */  { fnCheckNaN,                   NOPARAM,                     "NaN?",                                        "NaN?",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*   29 */  { fnCheckType,                  dtReal34,                    STD_REAL_R "?",                                STD_REAL_R "?",                                (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*   30 */  { fnCheckSpecial,               NOPARAM,                     "SPEC?",                                       "SPEC?",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*   31 */  { fnCheckType,                  dtString,                    "STRI?",                                       "STRI?",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*   32 */  { fnCheckInfinite,              NOPARAM,                     STD_PLUS_MINUS STD_INFINITY "?",               STD_PLUS_MINUS STD_INFINITY "?",               (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*   33 */  { fnIsPrime,                    NOPARAM,                     "PRIME?",                                      "PRIME?",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_CANCEL    | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*   34 */  { fnIsTopRoutine,               NOPARAM,                     "TOP?",                                        "TOP?",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*   35 */  { fnKeyEnter,                   NOPARAM/*#JM#*/,             "ENTER",                                       "ENTER",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_DISABLED        },
/*   36 */  { fnSwapXY,                     NOPARAM,                     "x" STD_RIGHT_OVER_LEFT_ARROW "y",             "x" STD_RIGHT_OVER_LEFT_ARROW "y",             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABL_XEQ | EIM_DISABLED | PTP_NONE         | HG_ENABLED_MX_ONLY },
/*   37 */  { fnDrop,                       NOPARAM,                     "DROPx",                                       "DROPx",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*   38 */  { fnPause,                      TM_VALUE,                    "PAUSE",                                       "PAUSE",                                       (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/*   39 */  { fnRollUp,                     NOPARAM,                     "R" STD_UP_ARROW,                              "R" STD_UP_ARROW,                              (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABL_XEQ | EIM_DISABLED | PTP_NONE         | HG_DISABLED        },
/*   40 */  { fnRollDown,                   NOPARAM,                     "R" STD_DOWN_ARROW,                            "R" STD_DOWN_ARROW,                            (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABL_XEQ | EIM_DISABLED | PTP_NONE         | HG_DISABLED        },
/*   41 */  { fnClX,                        NOPARAM,                     "CLX",                                         "CLX",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_DISABLED  | US_ENABL_XEQ | EIM_DISABLED | PTP_NONE         | HG_DISABLED        },
/*   42 */  { fnFillStack,                  NOPARAM,                     "FILL",                                        "FILL",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*   43 */  { fnInput,                      TM_REGISTER,                 "INPUT",                                       "INPUT",                                       (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_CANCEL    | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/*   44 */  { fnStore,                      TM_STORCL,                   "STO",                                         "STO",                                         (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_DISABLED        },
/*   45 */  { fnStoreAdd,                   TM_REGISTER,                 "STO+",                                        "STO+",                                        (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/*   46 */  { fnStoreSub,                   TM_REGISTER,                 "STO-",                                        "STO-",                                        (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/*   47 */  { fnStoreMult,                  TM_REGISTER,                 "STO" STD_CROSS,                               "STO" STD_CROSS,                               (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/*   48 */  { fnStoreDiv,                   TM_REGISTER,                 "STO" STD_DIVIDE,                              "STO" STD_DIVIDE,                              (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/*   49 */  { fnCyx,                        NOPARAM,                     "COMB",                                        "C" STD_SUB_y STD_SUB_x,                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/*   50 */  { fnPyx,                        NOPARAM,                     "PERM",                                        "P" STD_SUB_y STD_SUB_x,                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/*   51 */  { fnRecall,                     TM_STORCL,                   "RCL",                                         "RCL",                                         (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_DISABLED        },
/*   52 */  { fnRecallAdd,                  TM_REGISTER,                 "RCL+",                                        "RCL+",                                        (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/*   53 */  { fnRecallSub,                  TM_REGISTER,                 "RCL-",                                        "RCL-",                                        (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/*   54 */  { fnRecallMult,                 TM_REGISTER,                 "RCL" STD_CROSS,                               "RCL" STD_CROSS,                               (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/*   55 */  { fnRecallDiv,                  TM_REGISTER,                 "RCL" STD_DIVIDE,                              "RCL" STD_DIVIDE,                              (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/*   56 */  { fnIsConverged,                TM_VALUE,                    "CONVG?",                                      "CONVG?",                                      (0 << TAM_MAX_BITS) |     7, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/*   57 */  { fnEntryQ,                     NOPARAM,                     "ENTRY?",                                      "ENTRY?",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*   58 */  { fnSquare,                     NOPARAM,                     "x" STD_SUP_2,                                 "x" STD_SUP_2,                                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED_MX_ONLY },
/*   59 */  { fnCube,                       NOPARAM,                     "x" STD_SUP_3,                                 "x" STD_SUP_3,                                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*   60 */  { fnPower,                      NOPARAM,                     "y" STD_SUP_BOLD_x,                            "y" STD_SUP_BOLD_x,                            (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*   61 */  { fnSquareRoot,                 NOPARAM,                     STD_SQUARE_ROOT STD_x_UNDER_ROOT,              STD_SQUARE_ROOT STD_x_UNDER_ROOT,              (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/*   62 */  { fnCubeRoot,                   NOPARAM,                     STD_CUBE_ROOT STD_x_UNDER_ROOT,                STD_CUBE_ROOT STD_x_UNDER_ROOT,                (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/*   63 */  { fnXthRoot,                    NOPARAM,                     STD_xTH_ROOT STD_y_UNDER_ROOT,                 STD_xTH_ROOT STD_y_UNDER_ROOT,                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/*   64 */  { fn2Pow,                       NOPARAM,                     "2" STD_SUP_BOLD_x,                            "2" STD_SUP_BOLD_x,                            (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*   65 */  { fnExp,                        NOPARAM,                     STD_EulerE STD_SUP_BOLD_x,                     STD_EulerE STD_SUP_BOLD_x,                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/*   66 */  { fnSint,                       NOPARAM,                     "SINT",                                        "SINT",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*   67 */  { fn10Pow,                      NOPARAM,                     "10" STD_SUP_BOLD_x,                           "10" STD_SUP_BOLD_x,                           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED_MX_ONLY },
/*   68 */  { fnLog2,                       NOPARAM/*#JM#*/,             "LB",                                          "LB",                                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED_MX_ONLY },
/*   69 */  { fnLn,                         NOPARAM/*#JM#*/,             "LN",                                          "LN",                                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED_MX_ONLY },//JM3 change ln to LN
/*   70 */  { fnStopProgram,                NOPARAM,                     "STOP",                                        "R/S",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*   71 */  { fnLog10,                      NOPARAM/*#JM#*/,             "LOG",                                         "LOG",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED_MX_ONLY },//JM Change lg to LOG
/*   72 */  { fnLogXY,                      NOPARAM/*#JM#*/,             "LOG" STD_SUB_x "y",                           "LOG" STD_SUB_x "y",                           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/*   73 */  { fnInvert,                     NOPARAM,                     "1/x",                                         "1/x",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED_MX_ONLY },
/*   74 */  { fnCos,                        NOPARAM/*#JM#*/,             "COS",                                         "COS",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED_MX_ONLY },//JM
/*   75 */  { fnCosh,                       NOPARAM,                     "cosh",                                        "cosh",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED_MX_ONLY },
/*   76 */  { fnSin,                        NOPARAM/*#JM#*/,             "SIN",                                         "SIN",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED_MX_ONLY },//JM3
/*   77 */  { fnKey,                        TM_REGISTER,                 "KEY?",                                        "KEY?",                                        (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_UNCHANGED | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/*   78 */  { fnSinh,                       NOPARAM,                     "sinh",                                        "sinh",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED_MX_ONLY },
/*   79 */  { fnTan,                        NOPARAM/*#JM#*/,             "TAN",                                         "TAN",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED_MX_ONLY },//JM3
/*   80 */  { fnTanh,                       NOPARAM,                     "tanh",                                        "tanh",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED_MX_ONLY },
/*   81 */  { fnArccos,                     NOPARAM/*#JM#*/,             "ARCCOS",                                      "ACOS",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED_MX_ONLY },//JM
/*   82 */  { fnArccosh,                    NOPARAM,                     "arcosh",                                      "arcosh",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED_MX_ONLY },
/*   83 */  { fnArcsin,                     NOPARAM/*#JM#*/,             "ARCSIN",                                      "ASIN",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED_MX_ONLY },//JM
/*   84 */  { fnArcsinh,                    NOPARAM,                     "arsinh",                                      "arsinh",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED_MX_ONLY },
/*   85 */  { fnArctan,                     NOPARAM/*#JM#*/,             "ARCTAN",                                      "ATAN",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED_MX_ONLY },//JM
/*   86 */  { fnArctanh,                    NOPARAM,                     "artanh",                                      "artanh",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED_MX_ONLY },
/*   87 */  { fnCeil,                       NOPARAM,                     "ceil",                                        "ceil",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/*   88 */  { fnFloor,                      NOPARAM,                     "floor",                                       "floor",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/*   89 */  { fnGcd,                        NOPARAM,                     "GCD",                                         "GCD",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*   90 */  { fnLcm,                        NOPARAM,                     "LCM",                                         "LCM",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*   91 */  { fnDec,                        TM_REGISTER,                 "DECR",                                        "DECR",                                        (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/*   92 */  { fnInc,                        TM_REGISTER,                 "INCR",                                        "INCR",                                        (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/*   93 */  { fnIp,                         NOPARAM,                     "IP",                                          "IP",                                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*   94 */  { fnFp,                         NOPARAM,                     "FP",                                          "FP",                                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*   95 */  { fnAdd,                        ITM_ADD,                     "+",                                           "+",                                           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED_MX_ONLY },
/*   96 */  { fnSubtract,                   ITM_SUB,                     "-",                                           "-",                                           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED_MX_ONLY },
/*   97 */  { fnChangeSign,                 ITM_CHS/*#JM#*/,             "CHS",                                         "CHS",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED_MX_ONLY },//JM Change +/- to CHS
/*   98 */  { fnMultiply,                   ITM_MULT,                    STD_CROSS,                                     STD_CROSS,                                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED_MX_ONLY },
/*   99 */  { fnDivide,                     ITM_DIV/*#JM#*/,             STD_DIVIDE,                                    STD_DIVIDE,                                    (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED_MX_ONLY },
/*  100 */  { fnIDiv,                       NOPARAM,                     "IDIV",                                        "IDIV",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/*  101 */  { fnView,                       TM_REGISTER,                 "VIEW",                                        "VIEW",                                        (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/*  102 */  { fnMod,                        NOPARAM,                     "MOD",                                         "MOD",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/*  103 */  { fnMax,                        NOPARAM,                     "MAX",                                         "MAX",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/*  104 */  { fnMin,                        NOPARAM,                     "MIN",                                         "MIN",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/*  105 */  { fnMagnitude,                  NOPARAM,                     "|x|",                                         "|x|",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/*  106 */  { fnNeighb,                     NOPARAM,                     "NEIGHB",                                      "NEIGHB",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  107 */  { fnNextPrime,                  NOPARAM,                     "NEXTP",                                       "NEXTP",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  108 */  { fnFactorial,                  NOPARAM,                     "x!",                                          "x!",                                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/*  109 */  { fnPi,                         NOPARAM,                     STD_pi,                                        STD_pi,                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_DISABLED        },
/*  110 */  { fnClearFlag,                  TM_FLAGW,                    "CF",                                          "CF",                                          (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_FLAG         | HG_ENABLED         },
/*  111 */  { fnSetFlag,                    TM_FLAGW,                    "SF",                                          "SF",                                          (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_FLAG         | HG_ENABLED         },
/*  112 */  { fnFlipFlag,                   TM_FLAGW,                    "FF",                                          "FF",                                          (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_FLAG         | HG_ENABLED         },
/*  113 */  { fnCheckMatrixSquare,          NOPARAM,                     "M.SQR?",                                      "M.SQR?",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  114 */  { itemToBeCoded,                NOPARAM,                     "LITE",                                        "LITE",                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_LITERAL      | HG_ENABLED         }, // Literal in a PGM
/*  115 */  { fnAngularModeJM,              amDegree,                    STD_RIGHT_DOUBLE_ARROW "DEG",                  STD_RIGHT_DOUBLE_ARROW "DEG",                  (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  116 */  { fnAngularModeJM,              amDMS /*#JM#*/,              STD_RIGHT_DOUBLE_ARROW "D.MS",                 STD_RIGHT_DOUBLE_ARROW "D.MS",                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  117 */  { fnAngularModeJM,              amGrad,                      STD_RIGHT_DOUBLE_ARROW "GRAD",                 STD_RIGHT_DOUBLE_ARROW "GRAD",                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  118 */  { fnAngularModeJM,              amMultPi,                    STD_RIGHT_DOUBLE_ARROW "MUL" STD_pi,           STD_RIGHT_DOUBLE_ARROW "MUL" STD_pi,           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  119 */  { fnAngularModeJM,              amRadian,                    STD_RIGHT_DOUBLE_ARROW "RAD",                  STD_RIGHT_DOUBLE_ARROW "RAD",                  (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  120 */  { fnLint,                       NOPARAM,                     "LINT",                                        "LINT",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  121 */  { fnSetRoundingMode,            TM_VALUE,                    "RMODE",                                       "RMODE",                                       (0 << TAM_MAX_BITS) |     6, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/*  122 */  { fnRmd,                        NOPARAM,                     "RMD",                                         "RMD",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/*  123 */  { fnLogicalNot,                 NOPARAM,                     "NOT",                                         "NOT",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  124 */  { fnLogicalAnd,                 NOPARAM,                     "AND",                                         "AND",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  125 */  { fnLogicalOr,                  NOPARAM,                     "OR",                                          "OR",                                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  126 */  { fnLogicalXor,                 NOPARAM,                     "XOR",                                         "XOR",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  127 */  { fnSwapX,                      TM_REGISTER,                 "x" STD_RIGHT_OVER_LEFT_ARROW,                 "x" STD_RIGHT_OVER_LEFT_ARROW,                 (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABL_XEQ | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },


// Items from 128 to ... are 2 byte OP codes
// Constants
/*  128 */  { fnConstant,                   0,                           "a" STD_SUB_g,                                 "yr.gregor",                                   (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  129 */  { fnConstant,                   1,                           "r" STD_SUB_B STD_SUB_o STD_SUB_h STD_SUB_r,   "rad.bohr",                                    (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  130 */  { fnConstant,                   2,                           "a" STD_SUB_M STD_SUB_o STD_SUB_o STD_SUB_n,   "orb.moon",                                    (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  131 */  { fnConstant,                   3,                           "a" STD_SUB_EARTH,                             "orb.earth",                                   (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  132 */  { fnConstant,                   4,                           "c",                                           "lightspeed",                                  (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  133 */  { fnConstant,                   5,                           "c" STD_SUB_1,                                 "c.radiatn1",                                  (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  134 */  { fnConstant,                   6,                           "c" STD_SUB_2,                                 "c.radiatn2",                                  (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  135 */  { fnConstant,                   7,                           "e",                                           "charge.elem",                                 (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  136 */  { fnConstant,                   8,                           STD_EulerE,                                    STD_EulerE ".euler",                           (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  137 */  { fnConstant,                   9,                           "F",                                           "c.faraday",                                   (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  138 */  { fnConstant,                   10,                          "F" STD_SUB_alpha,                             STD_alpha ".feigenbm",                         (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  139 */  { fnConstant,                   11,                          "F" STD_SUB_delta,                             STD_delta ".feigenbm",                         (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  140 */  { fnConstant,                   12,                          "G",                                           "c.grav.nwt",                                  (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  141 */  { fnConstant,                   13,                          "G" STD_SUB_0,                                 "cond.quant",                                  (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  142 */  { fnConstant,                   14,                          "G" STD_SUB_C,                                 "c.catalan",                                   (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  143 */  { fnConstant,                   15,                          "g" STD_SUB_e,                                 "gfact.elec",                                  (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  144 */  { fnConstant,                   16,                          "GM" STD_SUB_EARTH,                            "c.grav.geo",                                  (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  145 */  { fnConstant,                   17,                          "g" STD_SUB_EARTH,                             "acc.earth",                                   (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  146 */  { fnConstant,                   18,                          STD_PLANCK,                                    "c.planck",                                    (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  147 */  { fnConstant,                   19,                          STD_PLANCK_2PI,                                "red.planck",                                  (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  148 */  { fnConstant,                   20,                          "k" STD_SUB_B,                                 "c.boltzmn",                                   (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  149 */  { fnConstant,                   21,                          "K" STD_SUB_J,                                 "c.josephsn",                                  (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  150 */  { fnConstant,                   22,                          "l" STD_SUB_P STD_SUB_L,                       "len.planck",                                  (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  151 */  { fnConstant,                   23,                          "m" STD_SUB_e,                                 "mass.elec",                                   (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  152 */  { fnConstant,                   24,                          "M" STD_SUB_M STD_SUB_o STD_SUB_o STD_SUB_n,   "mass.moon",                                   (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  153 */  { fnConstant,                   25,                          "m" STD_SUB_n,                                 "mass.neu",                                    (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  154 */  { fnConstant,                   26,                          "m" STD_SUB_n "/m" STD_SUB_p,                  "r.neu.prot",                                  (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  155 */  { fnConstant,                   27,                          "m" STD_SUB_p,                                 "mass.prot",                                   (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  156 */  { fnConstant,                   28,                          "m" STD_SUB_P STD_SUB_L,                       "mass.planck",                                 (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  157 */  { fnConstant,                   29,                          "m" STD_SUB_p "/m" STD_SUB_e,                  "r.prot.elec",                                 (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  158 */  { fnConstant,                   30,                          "m" STD_SUB_u,                                 "mass.atom",                                   (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  159 */  { fnConstant,                   31,                          "m" STD_SUB_u "c" STD_SUP_2,                   "energy.atom",                                 (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  160 */  { fnConstant,                   32,                          "m" STD_SUB_mu,                                "mass.muon",                                   (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  161 */  { fnConstant,                   33,                          "M" STD_SUB_SUN,                               "mass.sun",                                    (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  162 */  { fnConstant,                   34,                          "M" STD_SUB_EARTH,                             "mass.earth",                                  (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  163 */  { fnConstant,                   35,                          "N" STD_SUB_A,                                 "nr.avogadro",                                 (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  164 */  { fnConstant,                   36,                          "NaN",                                         "not.a.nr",                                    (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  165 */  { fnConstant,                   37,                          "p" STD_SUB_0,                                 "press.atm",                                   (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  166 */  { fnConstant,                   38,                          "R",                                           "c.mol.gas",                                   (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  167 */  { fnConstant,                   39,                          "r" STD_SUB_e,                                 "rad.elec",                                    (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  168 */  { fnConstant,                   40,                          "R" STD_SUB_K,                                 "c.klitzing",                                  (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  169 */  { fnConstant,                   41,                          "R" STD_SUB_M STD_SUB_o STD_SUB_o STD_SUB_n,   "rad.moon",                                    (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  170 */  { fnConstant,                   42,                          "R" STD_SUB_INFINITY,                          "c.rydberg",                                   (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  171 */  { fnConstant,                   43,                          "R" STD_SUB_SUN,                               "rad.sun",                                     (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  172 */  { fnConstant,                   44,                          "R" STD_SUB_EARTH,                             "rad.earth",                                   (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  173 */  { fnConstant,                   45,                          "Sa" STD_SUB_EARTH,                            "majax.earth",                                 (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  174 */  { fnConstant,                   46,                          "Sb" STD_SUB_EARTH,                            "minax.earth",                                 (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  175 */  { fnConstant,                   47,                          "Se" STD_SUP_2,                                "sq.eccent1",                                  (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  176 */  { fnConstant,                   48,                          "Se'" STD_SUP_2,                               "sq.eccent2",                                  (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  177 */  { fnConstant,                   49,                          "Sf" STD_SUP_MINUS STD_SUP_1,                  "f.flatteng",                                  (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  178 */  { fnConstant,                   50,                          "T" STD_SUB_0,                                 "temp.stand",                                  (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  179 */  { fnConstant,                   51,                          "T" STD_SUB_P,                                 "temp.planck",                                 (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  180 */  { fnConstant,                   52,                          "t" STD_SUB_P STD_SUB_L,                       "time.planck",                                 (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  181 */  { fnConstant,                   53,                          "V" STD_SUB_m,                                 "volume.gas",                                  (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  182 */  { fnConstant,                   54,                          "Z" STD_SUB_0,                                 "imped.vac",                                   (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  183 */  { fnConstant,                   55,                          STD_alpha STD_SUB_F,                           "c.finestruc",                                 (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  184 */  { fnConstant,                   56,                          "K" STD_SUB_0,                                 "c.khinchin",                                  (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  185 */  { fnConstant,                   57,                          STD_gamma STD_SUB_E STD_SUB_M,                 "c.eul.masc",                                  (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  186 */  { fnConstant,                   58,                          STD_gamma STD_SUB_p,                           "r.gyro.prot",                                 (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  187 */  { fnConstant,                   59,                          STD_DELTA STD_nu STD_SUB_C STD_SUB_s,          "frq.hypf.cs",                                 (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  188 */  { fnConstant,                   60,                          STD_epsilon STD_SUB_0,                         "epermt.vac",                                  (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  189 */  { fnConstant,                   61,                          STD_lambda STD_SUB_C,                          "wavln.elec",                                  (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  190 */  { fnConstant,                   62,                          STD_lambda STD_SUB_C STD_SUB_n,                "wavln.neu",                                   (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  191 */  { fnConstant,                   63,                          STD_lambda STD_SUB_C STD_SUB_p,                "wavln.prot",                                  (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  192 */  { fnConstant,                   64,                          STD_mu STD_SUB_0,                              "mpermb.vac",                                  (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  193 */  { fnConstant,                   65,                          STD_mu STD_SUB_B,                              "magn.both",                                   (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  194 */  { fnConstant,                   66,                          STD_mu STD_SUB_e,                              "mgmom.elec",                                  (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  195 */  { fnConstant,                   67,                          STD_mu STD_SUB_e "/" STD_mu STD_SUB_B,         "r.elec.bohr",                                 (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  196 */  { fnConstant,                   68,                          STD_mu STD_SUB_n,                              "magmom.neu",                                  (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  197 */  { fnConstant,                   69,                          STD_mu STD_SUB_p,                              "mgmom.prot",                                  (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  198 */  { fnConstant,                   70,                          STD_mu STD_SUB_u,                              "magn.nucl",                                   (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  199 */  { fnConstant,                   71,                          STD_mu STD_SUB_mu,                             "mgmom.muon",                                  (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  200 */  { fnConstant,                   72,                          STD_sigma STD_SUB_B,                           "c.stephbol",                                  (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  201 */  { fnConstant,                   73,                          STD_phi_m,                                     "r.golden",                                    (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  202 */  { fnConstant,                   74,                          STD_PHI STD_SUB_0,                             "fluxq.magn",                                  (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  203 */  { fnConstant,                   75,                          STD_omega STD_SUB_EARTH,                       "vangl.earth",                                 (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  204 */  { fnConstant,                   76,                          "-" STD_INFINITY,                              "inf.minus",                                   (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  205 */  { fnConstant,                   77,                          STD_INFINITY,                                  "inf.plus",                                    (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  206 */  { itemToBeCoded,                78,                          "#",                                           "zero",                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  207 */  { fnConstant,                   TM_VALUE,                    "CNST",                                        "CNST",                                        (0 << TAM_MAX_BITS) |   215, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8_16  | HG_ENABLED         }, // tam.max is replaced in tam.c : tamEnterMode()
/*  208 */  { fnConstant,                   79,                          STD_xi STD_SUB_B,                              "exp.bbone",                                   (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  209 */  { fnConstant,                   80,                          STD_delta STD_SUB_S,                           "r.silver",                                    (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  210 */  { fnConstant,                   81,                          STD_mu STD_SUB_G,                              "c.gerver",                                    (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  211 */  { fnConstant,                   82,                          STD_tau,                                       "c.circle",                                    (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  212 */  { fnConstant,                   83,                          STD_pi,                                        "pi",                                          (0 << TAM_MAX_BITS) |     0, CAT_CNST | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  213 */  { itemToBeCoded,                NOPARAM,                     "0213",                                        "0213",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  214 */  { itemToBeCoded,                NOPARAM,                     "0214",                                        "0214",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  215 */  { itemToBeCoded,                NOPARAM,                     "0215",                                        "0215",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  216 */  { itemToBeCoded,                NOPARAM,                     "0216",                                        "0216",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  217 */  { itemToBeCoded,                NOPARAM,                     "0217",                                        "0217",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  218 */  { itemToBeCoded,                NOPARAM,                     "0218",                                        "0218",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  219 */  { itemToBeCoded,                NOPARAM,                     "0219",                                        "0219",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },


// Conversions
/*  220 */  { fnCvtCToF,                    NOPARAM,                     STD_DEGREE "C" STD_RIGHT_ARROW STD_DEGREE "F", STD_DEGREE "C" STD_RIGHT_ARROW STD_DEGREE "F", (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  221 */  { fnCvtFToC,                    NOPARAM,                     STD_DEGREE "F" STD_RIGHT_ARROW STD_DEGREE "C", STD_DEGREE "F" STD_RIGHT_ARROW STD_DEGREE "C", (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  222 */  { fnCvtDbRatio,                 10,                          "dB" STD_RIGHT_ARROW "pr",                     "dB" STD_RIGHT_ARROW,                          (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  223 */  UNIT_CONV(constFactorFt2Hectare, multiply,  "ft"  STD_SUP_2 STD_RIGHT_ARROW "ha",               "ft"  STD_SUP_2 STD_RIGHT_ARROW "ha"),
/*  224 */  UNIT_CONV(constFactorFt2Hectare,   divide,  "ha" STD_RIGHT_ARROW "ft"  STD_SUP_2,               "ha" STD_RIGHT_ARROW "ft"  STD_SUP_2),
/*  225 */  { fnCvtDbRatio,                 20,                          "dB" STD_RIGHT_ARROW "fr",                     "dB" STD_RIGHT_ARROW,                          (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  226 */  UNIT_CONV(constFactorFt2M2,     multiply,   "ft"  STD_SUP_2 STD_RIGHT_ARROW "m" STD_SUP_2,      "ft"  STD_SUP_2 STD_RIGHT_ARROW "m" STD_SUP_2),
/*  227 */  UNIT_CONV(constFactorFt2M2,       divide,   "m" STD_SUP_2 STD_RIGHT_ARROW "ft"  STD_SUP_2,      "m" STD_SUP_2 STD_RIGHT_ARROW "ft"  STD_SUP_2),
/*  228 */  { fnCvtRatioDb,                 10,                          "pr" STD_RIGHT_ARROW "dB",                     "pwr",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  229 */  UNIT_CONV(constFactorHectareKm2,   divide,  "ha" STD_RIGHT_ARROW "km" STD_SUP_2,                "ha" STD_RIGHT_ARROW "km" STD_SUP_2),
/*  230 */  UNIT_CONV(constFactorHectareKm2, multiply,  "km" STD_SUP_2 STD_RIGHT_ARROW "ha",                "km" STD_SUP_2 STD_RIGHT_ARROW "ha"),
/*  231 */  { fnCvtRatioDb,                 20,                          "fr" STD_RIGHT_ARROW "dB",                     "fld",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  232 */  UNIT_CONV(constFactorGlukFzuk,  multiply,   "gal" STD_UK STD_RIGHT_ARROW "floz" STD_UK,         "gal" STD_UK STD_RIGHT_ARROW                                ),
/*  233 */  UNIT_CONV(constFactorFzukGluk,    divide,   "floz" STD_UK STD_RIGHT_ARROW "gal" STD_UK,         "floz" STD_UK STD_RIGHT_ARROW                               ),
/*  234 */  UNIT_CONV(constFactorAcreHa,    multiply,   "acre" STD_RIGHT_ARROW "ha",                        "acre" STD_RIGHT_ARROW),
/* 0235 */  UNIT_CONV(constFactorMlIn3,       divide,   "m" STD_litre STD_RIGHT_ARROW "in" STD_SUP_3,       "m" STD_litre STD_RIGHT_ARROW                               ),
/*  236 */  UNIT_CONV(constFactorAcreHa,      divide,   "ha" STD_RIGHT_ARROW "acre",                        "ha" STD_RIGHT_ARROW),
/* 0237 */  UNIT_CONV(constFactorIn3Ml,     multiply,   "in" STD_SUP_3 STD_RIGHT_ARROW "m" STD_litre,       "in" STD_SUP_3 STD_RIGHT_ARROW                              ),
/*  238 */  UNIT_CONV(constFactorAcreusHa,  multiply,   "acre" STD_US STD_RIGHT_ARROW "ha",                 "acre" STD_US STD_RIGHT_ARROW),
/* 0239 */  UNIT_CONV(constFactorFt3Gluk,   multiply,   "ft" STD_SUP_3 STD_RIGHT_ARROW "gal" STD_UK,        "ft" STD_SUP_3 STD_RIGHT_ARROW                              ),
/*  240 */  UNIT_CONV(constFactorAcreusHa,    divide,   "ha" STD_RIGHT_ARROW "acre" STD_US,                 "ha" STD_RIGHT_ARROW),
/* 0241 */  UNIT_CONV(constFactorGlukFt3,     divide,   "gal" STD_UK STD_RIGHT_ARROW "ft" STD_SUP_3,        "gal" STD_UK STD_RIGHT_ARROW                                ),
/*  242 */  UNIT_CONV(constFactorAtmPa,       divide,   "Pa" STD_RIGHT_ARROW "atm",                         "Pa" STD_RIGHT_ARROW "atm"),
/*  243 */  UNIT_CONV(constFactorAtmPa,     multiply,   "atm" STD_RIGHT_ARROW "Pa",                         "atm" STD_RIGHT_ARROW "Pa"),
/*  244 */  UNIT_CONV(constFactorAuM,       multiply,   "au" STD_RIGHT_ARROW "m",                           "au" STD_RIGHT_ARROW "m"),
/*  245 */  UNIT_CONV(constFactorAuM,         divide,   "m" STD_RIGHT_ARROW "au",                           "m" STD_RIGHT_ARROW "au"),
/*  246 */  UNIT_CONV(constFactorBarPa,     multiply,   "bar" STD_RIGHT_ARROW "Pa",                         "bar" STD_RIGHT_ARROW "Pa"),
/*  247 */  UNIT_CONV(constFactorBarPa,       divide,   "Pa" STD_RIGHT_ARROW "bar",                         "Pa" STD_RIGHT_ARROW "bar"),
/*  248 */  UNIT_CONV(constFactorBtuJ,      multiply,   "Btu" STD_RIGHT_ARROW "J",                          "Btu" STD_RIGHT_ARROW "J"),
/*  249 */  UNIT_CONV(constFactorBtuJ,        divide,   "J" STD_RIGHT_ARROW "Btu",                          "J" STD_RIGHT_ARROW "Btu"),
/*  250 */  UNIT_CONV(constFactorCalJ,      multiply,   "cal" STD_RIGHT_ARROW "J",                          "cal" STD_RIGHT_ARROW "J"),
/*  251 */  UNIT_CONV(constFactorCalJ,        divide,   "J" STD_RIGHT_ARROW "cal",                          "J" STD_RIGHT_ARROW "cal"),
/*  252 */  UNIT_CONV(constFactorLbfftNm,   multiply,   "lbf" STD_DOT "ft" STD_RIGHT_ARROW "Nm",            "lbf" STD_DOT "ft" STD_RIGHT_ARROW),
/* 0253 */  UNIT_CONV(constFactorLFt3,        divide,   STD_litre STD_RIGHT_ARROW "ft" STD_SUP_3,           STD_litre STD_RIGHT_ARROW                                   ),
/*  254 */  UNIT_CONV(constFactorLbfftNm,     divide,   "Nm" STD_RIGHT_ARROW "lbf" STD_DOT "ft",            "Nm" STD_RIGHT_ARROW),
/* 0255 */  UNIT_CONV(constFactorFt3L,      multiply,   "ft" STD_SUP_3 STD_RIGHT_ARROW STD_litre,           "ft" STD_SUP_3 STD_RIGHT_ARROW                              ),
/*  256 */  UNIT_CONV(constFactorCwtKg,     multiply,   "cwt" STD_RIGHT_ARROW "kg",                         "cwt" STD_RIGHT_ARROW "kg"),
/*  257 */  UNIT_CONV(constFactorCwtKg,       divide,   "kg" STD_RIGHT_ARROW "cwt",                         "kg" STD_RIGHT_ARROW "cwt"),
/*  258 */  UNIT_CONV(constFactorFtM,       multiply,   "ft"  STD_RIGHT_ARROW "m",                          "ft"  STD_RIGHT_ARROW "m"),
/*  259 */  UNIT_CONV(constFactorFtM,         divide,   "m" STD_RIGHT_ARROW "ft" ,                          "m" STD_RIGHT_ARROW "ft" ),
/*  260 */  UNIT_CONV(constFactorSfeetM,    multiply,   "survey ft" STD_US STD_RIGHT_ARROW "m",             "survey ft" STD_US STD_RIGHT_ARROW),
/* 0261 */  UNIT_CONV(constFactorLQtus,     multiply,   STD_litre STD_RIGHT_ARROW "qt" STD_US,              STD_litre STD_RIGHT_ARROW                                   ),
/* 0262 */  UNIT_CONV(constFactorQtusL,       divide,   "qt" STD_US STD_RIGHT_ARROW STD_litre,              "qt" STD_US STD_RIGHT_ARROW                                 ),
/*  263 */  UNIT_CONV(constFactorSfeetM,      divide,   "m" STD_RIGHT_ARROW "survey ft" STD_US,             "m" STD_RIGHT_ARROW),
/*  264 */  UNIT_CONV(constFactorFlozukIn3,   divide,   "in"  STD_SUP_3 STD_RIGHT_ARROW "floz" STD_UK,      "in"  STD_SUP_3 STD_RIGHT_ARROW),
/*  265 */  UNIT_CONV(constFactorFlozukIn3, multiply,   "floz" STD_UK STD_RIGHT_ARROW "in"  STD_SUP_3,      "floz" STD_UK STD_RIGHT_ARROW),
/*  266 */  UNIT_CONV(constFactorFlozukMl,  multiply,   "floz" STD_UK STD_RIGHT_ARROW "m" STD_litre,        "floz" STD_UK STD_RIGHT_ARROW),
/*  267 */  UNIT_CONV(constFactorFlozusIn3,   divide,   "in"  STD_SUP_3 STD_RIGHT_ARROW "floz" STD_US,      "in"  STD_SUP_3 STD_RIGHT_ARROW),
/*  268 */  UNIT_CONV(constFactorFlozukMl,    divide,   "m" STD_litre STD_RIGHT_ARROW "floz" STD_UK,        "m" STD_litre STD_RIGHT_ARROW),
/*  269 */  UNIT_CONV(constFactorFlozusIn3, multiply,   "floz" STD_US STD_RIGHT_ARROW "in"  STD_SUP_3,      "floz" STD_US STD_RIGHT_ARROW),
/*  270 */  UNIT_CONV(constFactorFlozusMl,  multiply,   "floz" STD_US  STD_RIGHT_ARROW "m" STD_litre,       "floz" STD_US  STD_RIGHT_ARROW),
/*  271 */  UNIT_CONV(constFactorFt3toGalUS,multiply,   "ft"  STD_SUP_3 STD_RIGHT_ARROW "gal" STD_US,       "ft"  STD_SUP_3 STD_RIGHT_ARROW),
/*  272 */  UNIT_CONV(constFactorFlozusMl,    divide,   "m" STD_litre STD_RIGHT_ARROW "floz" STD_US ,       "m" STD_litre STD_RIGHT_ARROW),
/*  273 */  UNIT_CONV(constFactorFt3toGalUS,   divide,  "gal" STD_US STD_RIGHT_ARROW "ft"  STD_SUP_3,       "gal" STD_US STD_RIGHT_ARROW),
/*  274 */  UNIT_CONV(constFactorGalukL,    multiply,   "gal" STD_UK STD_RIGHT_ARROW STD_litre,             "gal" STD_UK STD_RIGHT_ARROW STD_litre),
/*  275 */  UNIT_CONV(constFactorGalukL,      divide,   STD_litre STD_RIGHT_ARROW "gal" STD_UK,             STD_litre STD_RIGHT_ARROW "gal" STD_UK),
/*  276 */  UNIT_CONV(constFactorGalusL,    multiply,   "gal" STD_US STD_RIGHT_ARROW STD_litre,             "gal" STD_US STD_RIGHT_ARROW STD_litre),
/*  277 */  UNIT_CONV(constFactorGalusL,      divide,   STD_litre STD_RIGHT_ARROW "gal" STD_US,             STD_litre STD_RIGHT_ARROW "gal" STD_US),
/*  278 */  UNIT_CONV(constFactorHpeW,      multiply,   "hp" STD_SUB_E STD_RIGHT_ARROW "W",                 "hp" STD_SUB_E STD_RIGHT_ARROW "W"),
/*  279 */  UNIT_CONV(constFactorHpeW,        divide,   "W" STD_RIGHT_ARROW "hp" STD_SUB_E,                 "W" STD_RIGHT_ARROW "hp" STD_SUB_E),
/*  280 */  UNIT_CONV(constFactorHpmW,      multiply,   "hp" STD_SUB_M STD_RIGHT_ARROW "W",                 "hp" STD_SUB_M STD_RIGHT_ARROW "W"),
/*  281 */  UNIT_CONV(constFactorHpmW,        divide,   "W" STD_RIGHT_ARROW "hp" STD_SUB_M,                 "W" STD_RIGHT_ARROW "hp" STD_SUB_M),
/*  282 */  UNIT_CONV(constFactorHpukW,     multiply,   "hp" STD_UK STD_RIGHT_ARROW "W",                    "hp" STD_UK STD_RIGHT_ARROW "W"),
/*  283 */  UNIT_CONV(constFactorHpukW,       divide,   "W" STD_RIGHT_ARROW "hp" STD_UK,                    "W" STD_RIGHT_ARROW "hp" STD_UK),
/*  284 */  UNIT_CONV(constFactorInhgPa,    multiply,   "inHg" STD_RIGHT_ARROW "Pa",                        "inHg"  STD_RIGHT_ARROW),
/* 0285 */  UNIT_CONV(constFactorCupcFzus,  multiply,   "cup" STD_US  STD_RIGHT_ARROW "floz" STD_US,        "cup" STD_US  STD_RIGHT_ARROW                              ),
/*  286 */  UNIT_CONV(constFactorInhgPa,      divide,   "Pa" STD_RIGHT_ARROW "inHg",                        "Pa" STD_RIGHT_ARROW),
/* 0287 */  UNIT_CONV(constFactorCupcMl,    multiply,   "cup" STD_US  STD_RIGHT_ARROW "m" STD_litre,        "cup" STD_US  STD_RIGHT_ARROW                              ),
/*  288 */  UNIT_CONV(constFactorInchMm,    multiply,   "in"  STD_RIGHT_ARROW "mm",                         "in"  STD_RIGHT_ARROW "mm"),
/*  289 */  UNIT_CONV(constFactorInchMm,      divide,   "mm" STD_RIGHT_ARROW "in" ,                         "mm" STD_RIGHT_ARROW "in" ),
/*  290 */  UNIT_CONV(constFactorWhJ,       multiply,   "Wh" STD_RIGHT_ARROW "J",                           "Wh" STD_RIGHT_ARROW "J"),
/*  291 */  UNIT_CONV(constFactorWhJ,         divide,   "J" STD_RIGHT_ARROW "Wh",                           "J" STD_RIGHT_ARROW "Wh"),
/*  292 */  UNIT_CONV(constFactorLbKg,        divide,   "kg" STD_RIGHT_ARROW "lb" ,                         "kg" STD_RIGHT_ARROW "lb" ),
/*  293 */  UNIT_CONV(constFactorLbKg,      multiply,   "lb"  STD_RIGHT_ARROW "kg",                         "lb"  STD_RIGHT_ARROW "kg"),
/*  294 */  UNIT_CONV(constFactorOzG,         divide,   "g" STD_RIGHT_ARROW "oz",                           "g" STD_RIGHT_ARROW "oz"),
/*  295 */  UNIT_CONV(constFactorOzG,       multiply,   "oz" STD_RIGHT_ARROW "g",                           "oz" STD_RIGHT_ARROW "g"),
/*  296 */  UNIT_CONV(constFactorShortcwtKg,  divide,   "kg" STD_RIGHT_ARROW "short cwt",                   "kg" STD_RIGHT_ARROW),
/* 0297 */  UNIT_CONV(constFactorCupukFzuk, multiply,   "cup" STD_UK STD_RIGHT_ARROW "floz" STD_UK,         "cup" STD_UK STD_RIGHT_ARROW                               ),
/*  298 */  UNIT_CONV(constFactorShortcwtKg,multiply,   "short cwt" STD_RIGHT_ARROW "kg",                   "short cwt" STD_RIGHT_ARROW),
/* 0299 */  UNIT_CONV(constFactorCupukMl,   multiply,   "cup" STD_UK STD_RIGHT_ARROW "m" STD_litre,         "cup" STD_UK STD_RIGHT_ARROW                               ),
/*  300 */  UNIT_CONV(constFactorStoneKg,     divide,   "kg" STD_RIGHT_ARROW "stone",                       "kg" STD_RIGHT_ARROW),
/* 0301 */  UNIT_CONV(constFactorFzukCupuk,   divide,   "floz" STD_UK STD_RIGHT_ARROW "cup" STD_UK,         "floz" STD_UK STD_RIGHT_ARROW                              ),
/*  302 */  UNIT_CONV(constFactorStoneKg,   multiply,   "stone" STD_RIGHT_ARROW "kg",                       "stone" STD_RIGHT_ARROW),
/* 0303 */  UNIT_CONV(constFactorFzukTbspuk,multiply,   "floz" STD_UK STD_RIGHT_ARROW "tbsp" STD_UK,        "floz" STD_UK STD_RIGHT_ARROW                              ),
/*  304 */  UNIT_CONV(constFactorShorttonKg,  divide,   "kg" STD_RIGHT_ARROW "short ton",                   "kg" STD_RIGHT_ARROW),
/* 0305 */  UNIT_CONV(constFactorFzukTspuk, multiply,   "floz" STD_UK STD_RIGHT_ARROW "tsp" STD_UK,         "floz" STD_UK STD_RIGHT_ARROW                              ),
/* 0306 */  UNIT_CONV(constFactorFzusCupc,    divide,   "floz" STD_US STD_RIGHT_ARROW "cup" STD_US ,         "floz" STD_US STD_RIGHT_ARROW                             ),
/*  307 */  UNIT_CONV(constFactorShorttonKg,multiply,   "short ton" STD_RIGHT_ARROW "kg",                   "short ton" STD_RIGHT_ARROW),
/* 0308 */  UNIT_CONV(constFactorFzusTbspc, multiply,   "floz" STD_US STD_RIGHT_ARROW "Tbsp" STD_US ,       "floz" STD_US STD_RIGHT_ARROW                              ),
/* 0309 */  UNIT_CONV(constFactorFzusTspc,  multiply,   "floz" STD_US STD_RIGHT_ARROW "tsp" STD_US ,         "floz" STD_US STD_RIGHT_ARROW                             ),
/*  310 */  UNIT_CONV(constFactorTonKg,       divide,   "kg" STD_RIGHT_ARROW "ton",                         "kg" STD_RIGHT_ARROW "ton"),
/*  311 */  UNIT_CONV(constFactorLiangKg,   multiply,   "kg" STD_RIGHT_ARROW "li" STD_a_BREVE "ng",         "kg" STD_RIGHT_ARROW),
/* 0312 */  UNIT_CONV(constFactorMlCupc,      divide,   "m" STD_litre STD_RIGHT_ARROW "cup" STD_US ,        "m" STD_litre STD_RIGHT_ARROW                              ),
/*  313 */  UNIT_CONV(constFactorTonKg,     multiply,   "ton" STD_RIGHT_ARROW "kg",                         "ton" STD_RIGHT_ARROW "kg"),
/*  314 */  UNIT_CONV(constFactorLiangKg,     divide,   "li" STD_a_BREVE "ng" STD_RIGHT_ARROW "kg",         "li" STD_a_BREVE "ng" STD_RIGHT_ARROW),
/* 0315 */  UNIT_CONV(constFactorMlCupuk,     divide,   "m" STD_litre STD_RIGHT_ARROW "cup" STD_UK,         "m" STD_litre STD_RIGHT_ARROW                              ),
/*  316 */  UNIT_CONV(constFactorTrozG,       divide,   "g" STD_RIGHT_ARROW "tr" STD_SPACE_3_PER_EM "oz",   "g" STD_RIGHT_ARROW),
/* 0317 */  UNIT_CONV(constFactorMlPintlq,    divide,   "m" STD_litre STD_RIGHT_ARROW "pint" STD_US,        "m" STD_litre STD_RIGHT_ARROW                              ),
/*  318 */  UNIT_CONV(constFactorTrozG,     multiply,   "tr" STD_SPACE_3_PER_EM "oz" STD_RIGHT_ARROW "g",   "tr" STD_SPACE_3_PER_EM "oz" STD_RIGHT_ARROW),
/* 0319 */  UNIT_CONV(constFactorMlPintuk,    divide,   "m" STD_litre STD_RIGHT_ARROW "pint" STD_UK,        "m" STD_litre STD_RIGHT_ARROW                              ),
/*  320 */  UNIT_CONV(constFactorLbfN,      multiply,   "lbf" STD_RIGHT_ARROW "N",                          "lbf" STD_RIGHT_ARROW "N"),
/*  321 */  UNIT_CONV(constFactorLbfN,        divide,   "N" STD_RIGHT_ARROW "lbf",                          "N" STD_RIGHT_ARROW "lbf"),
/*  322 */  UNIT_CONV(constFactorLyM,       multiply,   "ly" STD_RIGHT_ARROW "m",                           "ly" STD_RIGHT_ARROW "m"),
/*  323 */  UNIT_CONV(constFactorLyM,         divide,   "m" STD_RIGHT_ARROW "ly",                           "m" STD_RIGHT_ARROW "ly"),
/*  324 */  UNIT_CONV(constFactorMmhgPa,    multiply,   "mmHg" STD_RIGHT_ARROW "Pa",                        "mmHg" STD_RIGHT_ARROW),
/* 0325 */  UNIT_CONV(constFactorMlQt,        divide,   "m" STD_litre STD_RIGHT_ARROW "qt" STD_UK,          "m" STD_litre STD_RIGHT_ARROW                              ),
/*  326 */  UNIT_CONV(constFactorMmhgPa,      divide,   "Pa" STD_RIGHT_ARROW "mmHg",                        "Pa" STD_RIGHT_ARROW),
/* 0327 */  UNIT_CONV(constFactorMlQtus,      divide,   "m" STD_litre STD_RIGHT_ARROW "qt" STD_US,          "m" STD_litre STD_RIGHT_ARROW                              ),
/*  328 */  UNIT_CONV(constFactorMiKm,      multiply,   "mi"  STD_RIGHT_ARROW "km",                         "mi"  STD_RIGHT_ARROW "km"),
/*  329 */  UNIT_CONV(constFactorMiKm,        divide,   "km" STD_RIGHT_ARROW "mi" ,                         "km" STD_RIGHT_ARROW "mi" ),
/*  330 */  UNIT_CONV(constFactorNmiKm,       divide,   "km" STD_RIGHT_ARROW "nmi",                         "km" STD_RIGHT_ARROW "nmi"),
/*  331 */  UNIT_CONV(constFactorNmiKm,     multiply,   "nmi" STD_RIGHT_ARROW "km",                         "nmi" STD_RIGHT_ARROW "km"),
/*  332 */  UNIT_CONV(constFactorPcM,         divide,   "m" STD_RIGHT_ARROW "pc",                           "m" STD_RIGHT_ARROW "pc"),
/*  333 */  UNIT_CONV(constFactorPcM,       multiply,   "pc" STD_RIGHT_ARROW "m",                           "pc" STD_RIGHT_ARROW "m"),
/*  334 */  UNIT_CONV(constFactorPointMm,     divide,   "mm" STD_RIGHT_ARROW "point",                       "mm" STD_RIGHT_ARROW),
/* 0335 */  UNIT_CONV(constFactorMlTbspc,     divide,   "m" STD_litre STD_RIGHT_ARROW "Tbsp" STD_US ,       "m" STD_litre STD_RIGHT_ARROW                              ),
/*  336 */  UNIT_CONV(constFactorMileM,     multiply,   "mi"  STD_RIGHT_ARROW "m",                          "mi"  STD_RIGHT_ARROW "m"),
/*  337 */  UNIT_CONV(constFactorPointMm,   multiply,   "point" STD_RIGHT_ARROW "mm",                       "point" STD_RIGHT_ARROW),
/* 0338 */  UNIT_CONV(constFactorMlTbspuk,    divide,   "m" STD_litre STD_RIGHT_ARROW "tbsp" STD_UK,        "m" STD_litre STD_RIGHT_ARROW                              ),
/*  339 */  UNIT_CONV(constFactorMileM,       divide,   "m" STD_RIGHT_ARROW "mi" ,                          "m" STD_RIGHT_ARROW "mi" ),
/*  340 */  UNIT_CONV(constFactorYardM,       divide,   "m" STD_RIGHT_ARROW "yd" ,                          "m" STD_RIGHT_ARROW "yd" ),
/*  341 */  UNIT_CONV(constFactorYardM,     multiply,   "yd"  STD_RIGHT_ARROW "m",                          "yd"  STD_RIGHT_ARROW "m"),
/*  342 */  UNIT_CONV(constFactorPsiPa,     multiply,   "psi" STD_RIGHT_ARROW "Pa",                         "psi" STD_RIGHT_ARROW "Pa"),
/*  343 */  UNIT_CONV(constFactorPsiPa,       divide,   "Pa" STD_RIGHT_ARROW "psi",                         "Pa" STD_RIGHT_ARROW "psi"),
/*  344 */  UNIT_CONV(constFactorTorrPa,      divide,   "Pa" STD_RIGHT_ARROW "torr",                        "Pa" STD_RIGHT_ARROW),
/* 0345 */  UNIT_CONV(constFactorMlTspc,      divide,   "m" STD_litre STD_RIGHT_ARROW "tsp" STD_US ,        "m" STD_litre STD_RIGHT_ARROW                              ),
/*  346 */  UNIT_CONV(constFactorTorrPa,    multiply,   "torr" STD_RIGHT_ARROW "Pa",                        "torr" STD_RIGHT_ARROW),
/* 0347 */  UNIT_CONV(constFactorMlTspuk,     divide,   "m" STD_litre STD_RIGHT_ARROW "tsp" STD_UK,         "m" STD_litre STD_RIGHT_ARROW                              ),
/*  348 */  UNIT_CONV(constFactorYearS,       divide,   "s" STD_RIGHT_ARROW "year",                         "s" STD_RIGHT_ARROW "year"),
/*  349 */  UNIT_CONV(constFactorYearS,     multiply,   "year" STD_RIGHT_ARROW "s",                         "year" STD_RIGHT_ARROW "s"),
/*  350 */  UNIT_CONV(constFactorCaratG,    multiply,   "carat" STD_RIGHT_ARROW "g",                        "carat" STD_RIGHT_ARROW),
/* 0351 */  UNIT_CONV(constFactorPintlqMl,  multiply,   "pint" STD_US STD_RIGHT_ARROW "m" STD_litre,        "pint" STD_US STD_RIGHT_ARROW                          ),
/*  352 */  UNIT_CONV(constFactorJinKg,       divide,   "j" STD_i_MACRON "n" STD_RIGHT_ARROW "kg",          "j" STD_i_MACRON "n" STD_RIGHT_ARROW "kg"),
/*  353 */  UNIT_CONV(constFactorCaratG,      divide,   "g" STD_RIGHT_ARROW "carat",                        "g" STD_RIGHT_ARROW),
/* 0354 */  UNIT_CONV(constFactorPintukMl,  multiply,   "pint" STD_UK STD_RIGHT_ARROW "m" STD_litre,        "pint" STD_UK STD_RIGHT_ARROW                              ),
/*  355 */  UNIT_CONV(constFactorJinKg,     multiply,   "kg" STD_RIGHT_ARROW "j" STD_i_MACRON "n",          "kg" STD_RIGHT_ARROW "j" STD_i_MACRON "n"),
/*  356 */  UNIT_CONV(constFactorQuartL,    multiply,   "qt" STD_UK STD_RIGHT_ARROW STD_litre,              "qt" STD_UK STD_RIGHT_ARROW STD_litre),
/*  357 */  UNIT_CONV(constFactorQuartL,      divide,   STD_litre STD_RIGHT_ARROW "qt" STD_UK,              STD_litre STD_RIGHT_ARROW "qt" STD_UK),
/*  358 */  UNIT_CONV(constFactorFathomM,   multiply,   "fathom" STD_RIGHT_ARROW "m",                       "fathom" STD_RIGHT_ARROW),
/* 0359 */  UNIT_CONV(constFactorQtMl,      multiply,   "qt" STD_UK STD_RIGHT_ARROW "m" STD_litre,          "qt" STD_UK STD_RIGHT_ARROW                                ),
/*  360 */  UNIT_CONV(constFactorNMiM,      multiply,   "nmi" STD_RIGHT_ARROW "m",                          "nmi" STD_RIGHT_ARROW "m"),
/*  361 */  UNIT_CONV(constFactorFathomM,     divide,   "m" STD_RIGHT_ARROW "fathom",                       "m" STD_RIGHT_ARROW),
/* 0362 */  UNIT_CONV(constFactorQtusMl,    multiply,   "qt" STD_US STD_RIGHT_ARROW "m" STD_litre,          "qt" STD_US STD_RIGHT_ARROW                                ),
/*  363 */  UNIT_CONV(constFactorNMiM,        divide,   "m" STD_RIGHT_ARROW "nmi",                          "m" STD_RIGHT_ARROW "nmi"),
/*  364 */  UNIT_CONV(constFactorBarrelM3,  multiply,   "barrel" STD_RIGHT_ARROW "m" STD_SUP_3,             "barrel" STD_RIGHT_ARROW),
/* 0365 */  UNIT_CONV(constFactorTbspcFzus,   divide,   "Tbsp" STD_US  STD_RIGHT_ARROW "floz" STD_US,       "Tbsp" STD_US  STD_RIGHT_ARROW                             ),
/*  366 */  UNIT_CONV(constFactorBarrelM3,    divide,   "m" STD_SUP_3 STD_RIGHT_ARROW "barrel",             "m" STD_SUP_3 STD_RIGHT_ARROW),
/*  367 */  { fnCvtHMSHR,                   divide,                      "HMS" STD_RIGHT_ARROW "HR",                    "HMS" STD_RIGHT_ARROW "HR",                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  368 */  { fnCvtHMSHR,                   multiply,                    "HR" STD_RIGHT_ARROW "HMS",                    "HR" STD_RIGHT_ARROW "HMS",                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 0369 */  UNIT_CONV(constFactorTbspcMl,   multiply,   "Tbsp" STD_US  STD_RIGHT_ARROW "m" STD_litre,       "Tbsp" STD_US  STD_RIGHT_ARROW                             ),
/*  370 */  UNIT_CONV(constFactorHectareM2, multiply,   "ha" STD_RIGHT_ARROW "m" STD_SUP_2,                 "ha" STD_RIGHT_ARROW "m" STD_SUP_2),
/*  371 */  UNIT_CONV(constFactorHectareM2,   divide,   "m" STD_SUP_2 STD_RIGHT_ARROW "ha",                 "m" STD_SUP_2 STD_RIGHT_ARROW "ha"),
/*  372 */  UNIT_CONV(constFactorMuM2,        divide,   "m" STD_u_BREVE STD_RIGHT_ARROW "m" STD_SUP_2,      "m" STD_u_BREVE STD_RIGHT_ARROW "m" STD_SUP_2),
/*  373 */  UNIT_CONV(constFactorMuM2,      multiply,   "m" STD_SUP_2 STD_RIGHT_ARROW "m" STD_u_BREVE,      "m" STD_SUP_2 STD_RIGHT_ARROW "m" STD_u_BREVE),
/*  374 */  UNIT_CONV(constFactorLiM,       multiply,   "l" STD_i_BREVE STD_RIGHT_ARROW "m",                "l" STD_i_BREVE STD_RIGHT_ARROW "m"),
/*  375 */  UNIT_CONV(constFactorLiM,         divide,   "m" STD_RIGHT_ARROW "l" STD_i_BREVE,                "m" STD_RIGHT_ARROW "l" STD_i_BREVE),
/*  376 */  UNIT_CONV(constFactorChiM,        divide,   "ch" STD_i_BREVE STD_RIGHT_ARROW "m",               "ch" STD_i_BREVE STD_RIGHT_ARROW "m"),
/*  377 */  UNIT_CONV(constFactorChiM,      multiply,   "m" STD_RIGHT_ARROW "ch" STD_i_BREVE,               "m" STD_RIGHT_ARROW "ch" STD_i_BREVE),
/*  378 */  UNIT_CONV(constFactorYinM,        divide,   "y" STD_i_BREVE "n" STD_RIGHT_ARROW "m",            "y" STD_i_BREVE "n" STD_RIGHT_ARROW "m"),
/*  379 */  UNIT_CONV(constFactorYinM,      multiply,   "m" STD_RIGHT_ARROW "y" STD_i_BREVE "n",            "m" STD_RIGHT_ARROW "y" STD_i_BREVE "n"),
/*  380 */  UNIT_CONV(constFactorCunM,        divide,   "c" STD_u_GRAVE "n" STD_RIGHT_ARROW "m",            "c" STD_u_GRAVE "n" STD_RIGHT_ARROW "m"),
/*  381 */  UNIT_CONV(constFactorCunM,      multiply,   "m" STD_RIGHT_ARROW "c" STD_u_GRAVE "n",            "m" STD_RIGHT_ARROW "c" STD_u_GRAVE "n"),
/*  382 */  UNIT_CONV(constFactorZhangM,      divide,   "zh" STD_a_GRAVE "ng" STD_RIGHT_ARROW "m",          "zh" STD_a_GRAVE "ng" STD_RIGHT_ARROW),
/* 0383 */  UNIT_CONV(constFactorTbspukFzuk,  divide,   "tbsp" STD_UK STD_RIGHT_ARROW "floz" STD_UK,        "tbsp" STD_UK STD_RIGHT_ARROW                              ),
/*  384 */  UNIT_CONV(constFactorZhangM,    multiply,   "m" STD_RIGHT_ARROW "zh" STD_a_GRAVE "ng",          "m" STD_RIGHT_ARROW),
/* 0385 */  UNIT_CONV(constFactorTbspukMl,  multiply,   "tbsp" STD_UK STD_RIGHT_ARROW "m" STD_litre,        "tbsp" STD_UK STD_RIGHT_ARROW                              ),
/*  386 */  UNIT_CONV(constFactorFenM,        divide,   "f" STD_e_MACRON "n" STD_RIGHT_ARROW "m",           "f" STD_e_MACRON "n" STD_RIGHT_ARROW "m"),
/*  387 */  UNIT_CONV(constFactorFenM,      multiply,   "m" STD_RIGHT_ARROW "f" STD_e_MACRON "n",           "m" STD_RIGHT_ARROW "f" STD_e_MACRON "n"),
/*  388 */  UNIT_CONV(constFactorMi2Km2,    multiply,   "mi" STD_SUP_2 STD_RIGHT_ARROW "km" STD_SUP_2,      "mi" STD_SUP_2 STD_RIGHT_ARROW "km" STD_SUP_2),
/*  389 */  UNIT_CONV(constFactorMi2Km2,      divide,   "km" STD_SUP_2 STD_RIGHT_ARROW "mi" STD_SUP_2,      "km" STD_SUP_2 STD_RIGHT_ARROW "mi" STD_SUP_2),
/*  390 */  UNIT_CONV(constFactorNmi2Km2,   multiply,   "nmi" STD_SUP_2 STD_RIGHT_ARROW "km" STD_SUP_2,     "nmi" STD_SUP_2 STD_RIGHT_ARROW "km" STD_SUP_2),
/*  391 */  UNIT_CONV(constFactorNmi2Km2,     divide,   "km" STD_SUP_2 STD_RIGHT_ARROW "nmi" STD_SUP_2,     "km" STD_SUP_2 STD_RIGHT_ARROW "nmi" STD_SUP_2),
/* 0392 */  UNIT_CONV(constFactorTspcFzus,    divide,   "tsp" STD_US  STD_RIGHT_ARROW "floz" STD_US,        "tsp" STD_US  STD_RIGHT_ARROW                              ),
/* 0393 */  UNIT_CONV(constFactorTspcMl,    multiply,   "tsp" STD_US  STD_RIGHT_ARROW "m" STD_litre,        "tsp" STD_US  STD_RIGHT_ARROW                              ),
/* 0394 */  UNIT_CONV(constFactorTspukFzuk,   divide,   "tsp" STD_UK STD_RIGHT_ARROW "floz" STD_UK,         "tsp" STD_UK STD_RIGHT_ARROW                               ),
/* 0395 */  UNIT_CONV(constFactorTspukMl,   multiply,   "tsp" STD_UK STD_RIGHT_ARROW "m" STD_litre,         "tsp" STD_UK STD_RIGHT_ARROW                               ),


// Flag, bit, rotation, and logical OPs
/*  396 */  { fnIsFlagClearClear,           TM_FLAGW,                    "FC?C",                                        "FC?C",                                        (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_FLAG         | HG_ENABLED         },
/*  397 */  { fnIsFlagClearSet,             TM_FLAGW,                    "FC?S",                                        "FC?S",                                        (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_FLAG         | HG_ENABLED         },
/*  398 */  { fnIsFlagClearFlip,            TM_FLAGW,                    "FC?F",                                        "FC?F",                                        (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_FLAG         | HG_ENABLED         },
/*  399 */  { fnIsFlagSetClear,             TM_FLAGW,                    "FS?C",                                        "FS?C",                                        (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_FLAG         | HG_ENABLED         },
/*  400 */  { fnIsFlagSetSet,               TM_FLAGW,                    "FS?S",                                        "FS?S",                                        (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_FLAG         | HG_ENABLED         },
/*  401 */  { fnIsFlagSetFlip,              TM_FLAGW,                    "FS?F",                                        "FS?F",                                        (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_FLAG         | HG_ENABLED         },
/*  402 */  { fnLogicalNand,                NOPARAM,                     "NAND",                                        "NAND",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  403 */  { fnLogicalNor,                 NOPARAM,                     "NOR",                                         "NOR",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  404 */  { fnLogicalXnor,                NOPARAM,                     "XNOR",                                        "XNOR",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  405 */  { fnBs,                         TM_VALUE,                    "BS?",                                         "BS?",                                         (0 << TAM_MAX_BITS) |    63, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/*  406 */  { fnBc,                         TM_VALUE,                    "BC?",                                         "BC?",                                         (0 << TAM_MAX_BITS) |    63, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/*  407 */  { fnCb,                         TM_VALUE,                    "CB",                                          "CB",                                          (0 << TAM_MAX_BITS) |    63, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/*  408 */  { fnSb,                         TM_VALUE,                    "SB",                                          "SB",                                          (0 << TAM_MAX_BITS) |    63, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/*  409 */  { fnFb,                         TM_VALUE,                    "FB",                                          "FB",                                          (0 << TAM_MAX_BITS) |    63, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/*  410 */  { fnRl,                         TM_VALUE,                    "RLn",                                         "RLn",                                         (0 << TAM_MAX_BITS) |    63, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/*  411 */  { fnRlc,                        TM_VALUE,                    "RLCn",                                        "RLCn",                                        (0 << TAM_MAX_BITS) |    63, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/*  412 */  { fnRr,                         TM_VALUE,                    "RRn",                                         "RRn",                                         (0 << TAM_MAX_BITS) |    63, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/*  413 */  { fnRrc,                        TM_VALUE,                    "RRCn",                                        "RRCn",                                        (0 << TAM_MAX_BITS) |    63, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/*  414 */  { fnSl,                         TM_VALUE,                    "SLn",                                         "SLn",                                         (0 << TAM_MAX_BITS) |    63, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/*  415 */  { fnSr,                         TM_VALUE,                    "SRn",                                         "SRn",                                         (0 << TAM_MAX_BITS) |    63, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/*  416 */  { fnAsr,                        TM_VALUE,                    "ASRn",                                        "ASRn",                                        (0 << TAM_MAX_BITS) |    63, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/*  417 */  { fnLj,                         NOPARAM,                     "LJ",                                          "LJ",                                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  418 */  { fnRj,                         NOPARAM,                     "RJ",                                          "RJ",                                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  419 */  { fnMaskl,                      TM_VALUE,                    "MASKL",                                       "MASKL",                                       (0 << TAM_MAX_BITS) |    64, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/*  420 */  { fnMaskr,                      TM_VALUE,                    "MASKR",                                       "MASKR",                                       (0 << TAM_MAX_BITS) |    64, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/*  421 */  { fnMirror,                     NOPARAM,                     "MIRROR",                                      "MIRROR",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  422 */  { fnCountBits,                  NOPARAM,                     "#B",                                          "#B",                                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  423 */  { fnSdl,                        TM_VALUE,                    "SDL",                                         "SDL",                                         (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/*  424 */  { fnSdr,                        TM_VALUE,                    "SDR",                                         "SDR",                                         (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/*  425 */  { fnZip,                        NOPARAM,                     "ZIP",                                         "ZIP",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  426 */  { fnUnzip,                      NOPARAM,                     "UNZIP",                                       "UNZIP",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  427 */  { itemToBeCoded,                NOPARAM,                     "0427",                                        "0427",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  428 */  { itemToBeCoded,                NOPARAM,                     "0428",                                        "0428",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  429 */  { itemToBeCoded,                NOPARAM,                     "0429",                                        "0429",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  430 */  { itemToBeCoded,                NOPARAM,                     "0430",                                        "0430",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  431 */  { itemToBeCoded,                NOPARAM,                     "0431",                                        "0431",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  432 */  { itemToBeCoded,                NOPARAM,                     "0432",                                        "0432",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },


// Statistical sums
/*  433 */  { fnSigmaAddRem,                SIGMA_PLUS,                  STD_SIGMA "+",                                 STD_SIGMA "+",                                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_DISABLED  | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  434 */  { fnSigmaAddRem,                SIGMA_MINUS,                 STD_SIGMA "-",                                 STD_SIGMA "-",                                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_DISABLED  | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  435 */  { fnStatSum,                    0,                           "n" STD_SIGMA,                                 "n",                                           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  436 */  { fnStatSum,                    SUM_X,                       STD_SIGMA "x",                                 STD_SIGMA "x",                                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  437 */  { fnStatSum,                    SUM_Y,                       STD_SIGMA "y",                                 STD_SIGMA "y",                                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  438 */  { fnStatSum,                    SUM_X2,                      STD_SIGMA "x" STD_SUP_2,                       STD_SIGMA "x" STD_SUP_2,                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  439 */  { fnStatSum,                    SUM_X2Y,                     STD_SIGMA "x" STD_SUP_2 "y",                   STD_SIGMA "x" STD_SUP_2 "y",                   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  440 */  { fnStatSum,                    SUM_Y2,                      STD_SIGMA "y" STD_SUP_2,                       STD_SIGMA "y" STD_SUP_2,                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  441 */  { fnStatSum,                    SUM_XY,                      STD_SIGMA "xy",                                STD_SIGMA "xy",                                (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  442 */  { fnStatSum,                    SUM_lnXlnY,                  STD_SIGMA "lnx" STD_DOT STD_SPACE_6_PER_EM "lny", STD_SIGMA "lnx" STD_DOT STD_SPACE_6_PER_EM "lny",                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  443 */  { fnStatSum,                    SUM_lnX,                     STD_SIGMA "lnx",                               STD_SIGMA "lnx",                                                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  444 */  { fnStatSum,                    SUM_ln2X,                    STD_SIGMA "ln" STD_SUP_2 "x",                  STD_SIGMA "ln" STD_SUP_2 "x",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  445 */  { fnStatSum,                    SUM_YlnX,                    STD_SIGMA "y" STD_DOT STD_SPACE_6_PER_EM "lnx", STD_SIGMA "y" STD_DOT STD_SPACE_6_PER_EM "lnx",                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  446 */  { fnStatSum,                    SUM_lnY,                     STD_SIGMA "lny",                               STD_SIGMA "lny",                                                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  447 */  { fnStatSum,                    SUM_ln2Y,                    STD_SIGMA "ln" STD_SUP_2 "y",                  STD_SIGMA "ln" STD_SUP_2 "y",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  448 */  { fnStatSum,                    SUM_XlnY,                    STD_SIGMA "x" STD_DOT STD_SPACE_6_PER_EM "lny", STD_SIGMA "x" STD_DOT STD_SPACE_6_PER_EM "lny",                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  449 */  { fnStatSum,                    SUM_X2lnY,                   STD_SIGMA "x" STD_SUP_2 STD_DOT STD_SPACE_6_PER_EM "lny", STD_SIGMA "x" STD_SUP_2 STD_DOT STD_SPACE_6_PER_EM "lny", (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  450 */  { fnStatSum,                    SUM_lnYonX,                  STD_SIGMA "x" STD_SUP_MINUS STD_SUP_1 STD_DOT STD_SPACE_6_PER_EM "lny", STD_SIGMA "x" STD_SUP_MINUS STD_SUP_1 STD_DOT STD_SPACE_6_PER_EM "lny", (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  451 */  { fnStatSum,                    SUM_X2onY,                   STD_SIGMA "x" STD_SUP_2 "y" STD_SUP_MINUS STD_SUP_1, STD_SIGMA "x" STD_SUP_2 "y" STD_SUP_MINUS STD_SUP_1, (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  452 */  { fnStatSum,                    SUM_1onX,                    STD_SIGMA "x" STD_SUP_MINUS STD_SUP_1,         STD_SIGMA "x" STD_SUP_MINUS STD_SUP_1,         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  453 */  { fnStatSum,                    SUM_1onX2,                   STD_SIGMA "x" STD_SUP_MINUS STD_SUP_2,         STD_SIGMA "x" STD_SUP_MINUS STD_SUP_2,         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  454 */  { fnStatSum,                    SUM_XonY,                    STD_SIGMA "xy" STD_SUP_MINUS STD_SUP_1,        STD_SIGMA "xy" STD_SUP_MINUS STD_SUP_1,        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  455 */  { fnStatSum,                    SUM_1onY,                    STD_SIGMA "y" STD_SUP_MINUS STD_SUP_1,         STD_SIGMA "y" STD_SUP_MINUS STD_SUP_1,         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  456 */  { fnStatSum,                    SUM_1onY2,                   STD_SIGMA "y" STD_SUP_MINUS STD_SUP_2,         STD_SIGMA "y" STD_SUP_MINUS STD_SUP_2,         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  457 */  { fnStatSum,                    SUM_X3,                      STD_SIGMA "x" STD_SUP_3,                       STD_SIGMA "x" STD_SUP_3,                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  458 */  { fnStatSum,                    SUM_X4,                      STD_SIGMA "x" STD_SUP_4,                       STD_SIGMA "x" STD_SUP_4,                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/*  459 */  { itemToBeCoded,                NOPARAM,                     "0459",                                        "0459",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  460 */  { itemToBeCoded,                NOPARAM,                     "0460",                                        "0460",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  461 */  { itemToBeCoded,                NOPARAM,                     "0461",                                        "0461",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  462 */  { itemToBeCoded,                NOPARAM,                     "0462",                                        "0462",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },


// System flags
/*  463 */  { fnGetSystemFlag,              FLAG_TDM24,                  "TDM24",                                       "TDM24",                                       (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // The system flags,
/*  464 */  { fnGetSystemFlag,              FLAG_YMD,                    "YMD",                                         "YMD",                                         (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // items from 453 to 493,
/*  465 */  { fnGetSystemFlag,              FLAG_DMY,                    "DMY",                                         "DMY",                                         (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // MUST be in the same
/*  466 */  { fnGetSystemFlag,              FLAG_MDY,                    "MDY",                                         "MDY",                                         (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // order as the flag
/*  467 */  { fnGetSystemFlag,              FLAG_CPXRES,                 "CPXRES",                                      "CPXRES",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // number (8 LSB) defined
/*  468 */  { fnGetSystemFlag,              FLAG_CPXj,                   "CPX" STD_op_j,                                "CPX" STD_op_j,                                (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // in defines.h
/*  469 */  { fnGetSystemFlag,              FLAG_POLAR,                  "POLAR",                                       "POLAR",                                       (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  470 */  { fnGetSystemFlag,              FLAG_FRACT,                  "FRACT",                                       "FRACT",                                       (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // And TDM24 MUST be
/*  471 */  { fnGetSystemFlag,              FLAG_PROPFR,                 "PROPFR",                                      "PROPFR",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // the first.
/*  472 */  { fnGetSystemFlag,              FLAG_DENANY,                 "DENANY",                                      "DENANY",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  473 */  { fnGetSystemFlag,              FLAG_DENFIX,                 "DENFIX",                                      "DENFIX",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  474 */  { fnGetSystemFlag,              FLAG_CARRY,                  "CARRY",                                       "CARRY",                                       (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  475 */  { fnGetSystemFlag,              FLAG_OVERFLOW,               "OVERFL",                                      "OVERFL",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  476 */  { fnGetSystemFlag,              FLAG_LEAD0,                  "LEAD.0",                                      "LEAD.0",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  477 */  { fnGetSystemFlag,              FLAG_ALPHA,                  "ALPHA",                                       "ALPHA",                                       (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  478 */  { fnGetSystemFlag,              FLAG_alphaCAP,               STD_alpha "CAP",                               STD_alpha "CAP",                               (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  479 */  { fnGetSystemFlag,              FLAG_RUNTIM,                 "RUNTIM",                                      "RUNTIM",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  480 */  { fnGetSystemFlag,              FLAG_RUNIO,                  "RUNIO",                                       "RUNIO",                                       (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  481 */  { fnGetSystemFlag,              FLAG_PRINTS,                 "PRINTS",                                      "PRINTS",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  482 */  { fnGetSystemFlag,              FLAG_TRACE,                  "TRACE",                                       "TRACE",                                       (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  483 */  { fnGetSystemFlag,              FLAG_USER,                   "USER",                                        "USER",                                        (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  484 */  { fnGetSystemFlag,              FLAG_LOWBAT,                 "LOWBAT",                                      "LOWBAT",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  485 */  { fnGetSystemFlag,              FLAG_SLOW,                   "SLOW",                                        "SLOW",                                        (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  486 */  { fnGetSystemFlag,              FLAG_SPCRES,                 "SPCRES",                                      "SPCRES",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  487 */  { fnGetSystemFlag,              FLAG_SSIZE8,                 "SSIZE8",                                      "SSIZE8",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  488 */  { fnGetSystemFlag,              FLAG_QUIET,                  "QUIET",                                       "QUIET",                                       (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  489 */  { fnGetSystemFlag,              FLAG_WRAPEND,                "WRPEND",                                      "WRPEND",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  490 */  { fnGetSystemFlag,              FLAG_MULTx,                  "MULT" STD_CROSS,                              "MULT" STD_CROSS,                              (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  491 */  { fnGetSystemFlag,              FLAG_ENGOVR,                 "ENGOVR",                                      "ENGOVR",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  492 */  { fnGetSystemFlag,              FLAG_GROW,                   "GROW",                                        "GROW",                                        (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  493 */  { fnGetSystemFlag,              FLAG_AUTOFF,                 "AUTOFF",                                      "AUTOFF",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  494 */  { fnGetSystemFlag,              FLAG_AUTXEQ,                 "AUTXEQ",                                      "AUTXEQ",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  495 */  { fnGetSystemFlag,              FLAG_PRTACT,                 "PRTACT",                                      "PRTACT",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  496 */  { fnGetSystemFlag,              FLAG_NUMIN,                  "NUM.IN",                                      "NUM.IN",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  497 */  { fnGetSystemFlag,              FLAG_ALPIN,                  "ALP.IN",                                      "ALP.IN",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  498 */  { fnGetSystemFlag,              FLAG_ASLIFT,                 "ASLIFT",                                      "ASLIFT",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  499 */  { fnGetSystemFlag,              FLAG_IGN1ER,                 "IGN1ER",                                      "IGN1ER",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  500 */  { fnGetSystemFlag,              FLAG_INTING,                 "INTING",                                      "INTING",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  501 */  { fnGetSystemFlag,              FLAG_SOLVING,                "SOLVING",                                     "SOLVING",                                     (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  502 */  { fnGetSystemFlag,              FLAG_VMDISP,                 "VMDISP",                                      "VMDISP",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  503 */  { fnGetSystemFlag,              FLAG_USB,                    "USB",                                         "USB",                                         (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  504 */  { fnGetSystemFlag,              FLAG_ENDPMT,                 "ENDPMT",                                      "ENDPMT",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  505 */  { fnGetSystemFlag,              FLAG_FRCSRN,                 "FRCSRN",                                      "FRCSRN",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  506 */  { fnGetSystemFlag,              FLAG_HPRP,                   "RP" STD_SUB_H STD_SUB_P,                      "RP" STD_SUB_H STD_SUB_P,                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  507 */  { fnGetSystemFlag,              FLAG_SBdate,                 "SBdate",                                      "SBdate",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  508 */  { fnGetSystemFlag,              FLAG_SBtime,                 "SBtime",                                      "SBtime",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  509 */  { fnGetSystemFlag,              FLAG_SBcr,                   "SBcr",                                        "SBcr",                                        (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  510 */  { fnGetSystemFlag,              FLAG_SBcpx,                  "SBcpx",                                       "SBcpx",                                       (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  511 */  { fnGetSystemFlag,              FLAG_SBang,                  "SBang",                                       "SBang",                                       (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  512 */  { fnGetSystemFlag,              FLAG_SBfrac,                 "SBfrac",                                      "SBfrac",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  513 */  { fnGetSystemFlag,              FLAG_SBint,                  "SBint",                                       "SBint",                                       (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  514 */  { fnGetSystemFlag,              FLAG_SBmx,                   "SBmx",                                        "SBmx",                                        (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  515 */  { fnGetSystemFlag,              FLAG_SBtvm,                  "SBtvm",                                       "SBtvm",                                       (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  516 */  { fnGetSystemFlag,              FLAG_SBoc,                   "SBoc",                                        "SBoc",                                        (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  517 */  { fnGetSystemFlag,              FLAG_SBss,                   "SBss",                                        "SBss",                                        (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  518 */  { fnGetSystemFlag,              FLAG_SBstpw,                 "SBstw",                                       "SBstw",                                       (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  519 */  { fnGetSystemFlag,              FLAG_SBser,                  "SBser",                                       "SBser",                                       (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  520 */  { fnGetSystemFlag,              FLAG_SBprn,                  "SBprn",                                       "SBprn",                                       (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  521 */  { fnGetSystemFlag,              FLAG_SBbatV,                 "SBbatV",                                      "SBbatV",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  522 */  { fnGetSystemFlag,              FLAG_SBshfR,                 "SBshfR",                                      "SBshfR",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  523 */  { fnGetSystemFlag,              FLAG_HPBASE,                 "BASE" STD_SUB_H STD_SUB_P,                    "BASE" STD_SUB_H STD_SUB_P,                    (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  524 */  { fnGetSystemFlag,              FLAG_2TO10,                  "1024" STD_SUP_n,                              "1024" STD_SUP_n,                              (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  525 */  { fnGetSystemFlag,              FLAG_SH_LONGPRESS,           "KEY.LP",                                      "KEY.LP",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  526 */  { fnGetSystemFlag,              FLAG_WRAPEDG,                "WRPEDG",                                      "WRPEDG",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },



// Bufferized items
/*  527 */  { addItemToBuffer,              REGISTER_X,                  "X",                                           "X",                                           (0 << TAM_MAX_BITS) |     0, CAT_REGS | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // The
/*  528 */  { addItemToBuffer,              REGISTER_Y,                  "Y",                                           "Y",                                           (0 << TAM_MAX_BITS) |     0, CAT_REGS | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // order
/*  529 */  { addItemToBuffer,              REGISTER_Z,                  "Z",                                           "Z",                                           (0 << TAM_MAX_BITS) |     0, CAT_REGS | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // of these
/*  530 */  { addItemToBuffer,              REGISTER_T,                  "T",                                           "T",                                           (0 << TAM_MAX_BITS) |     0, CAT_REGS | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // 12 lines
/*  531 */  { addItemToBuffer,              REGISTER_A,                  "A",                                           "A",                                           (0 << TAM_MAX_BITS) |     0, CAT_REGS | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // Must be
/*  532 */  { addItemToBuffer,              REGISTER_B,                  "B",                                           "B",                                           (0 << TAM_MAX_BITS) |     0, CAT_REGS | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // kept as
/*  533 */  { addItemToBuffer,              REGISTER_C,                  "C",                                           "C",                                           (0 << TAM_MAX_BITS) |     0, CAT_REGS | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // is.
/*  534 */  { addItemToBuffer,              REGISTER_D,                  "D",                                           "D",                                           (0 << TAM_MAX_BITS) |     0, CAT_REGS | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // Do not
/*  535 */  { addItemToBuffer,              REGISTER_L,                  "L",                                           "L",                                           (0 << TAM_MAX_BITS) |     0, CAT_REGS | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // put them
/*  536 */  { addItemToBuffer,              REGISTER_I,                  "I",                                           "I",                                           (0 << TAM_MAX_BITS) |     0, CAT_REGS | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // in a
/*  537 */  { addItemToBuffer,              REGISTER_J,                  "J",                                           "J",                                           (0 << TAM_MAX_BITS) |     0, CAT_REGS | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // different
/*  538 */  { addItemToBuffer,              REGISTER_K,                  "K",                                           "K",                                           (0 << TAM_MAX_BITS) |     0, CAT_REGS | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // order!
/*  539 */  { addItemToBuffer,              ITM_INDIRECTION,             STD_RIGHT_ARROW,                               STD_RIGHT_ARROW,                               (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  540 */  { addItemToBuffer,              ITM_0,                       "",                                            "0",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  541 */  { addItemToBuffer,              ITM_1,                       "",                                            "1",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  542 */  { addItemToBuffer,              ITM_2,                       "",                                            "2",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  543 */  { addItemToBuffer,              ITM_3,                       "",                                            "3",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  544 */  { addItemToBuffer,              ITM_4,                       "",                                            "4",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  545 */  { addItemToBuffer,              ITM_5,                       "",                                            "5",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  546 */  { addItemToBuffer,              ITM_6,                       "",                                            "6",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  547 */  { addItemToBuffer,              ITM_7,                       "",                                            "7",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  548 */  { addItemToBuffer,              ITM_8,                       "",                                            "8",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  549 */  { addItemToBuffer,              ITM_9,                       "",                                            "9",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  550 */  { addItemToBuffer,              ITM_A,                       "A",                                           "A",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  551 */  { addItemToBuffer,              ITM_B,                       "B",                                           "B",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  552 */  { addItemToBuffer,              ITM_C,                       "C",                                           "C",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  553 */  { addItemToBuffer,              ITM_D,                       "D",                                           "D",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  554 */  { addItemToBuffer,              ITM_E,                       "E",                                           "E",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  555 */  { addItemToBuffer,              ITM_F,                       "F",                                           "F",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  556 */  { addItemToBuffer,              ITM_G,                       "G",                                           "G",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  557 */  { addItemToBuffer,              ITM_H,                       "H",                                           "H",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  558 */  { addItemToBuffer,              ITM_I,                       "I",                                           "I",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  559 */  { addItemToBuffer,              ITM_J,                       "J",                                           "J",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  560 */  { addItemToBuffer,              ITM_K,                       "K",                                           "K",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  561 */  { addItemToBuffer,              ITM_L,                       "L",                                           "L",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  562 */  { addItemToBuffer,              ITM_M,                       "M",                                           "M",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  563 */  { addItemToBuffer,              ITM_N,                       "N",                                           "N",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  564 */  { addItemToBuffer,              ITM_O,                       "O",                                           "O",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  565 */  { addItemToBuffer,              ITM_P,                       "P",                                           "P",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  566 */  { addItemToBuffer,              ITM_Q,                       "Q",                                           "Q",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  567 */  { addItemToBuffer,              ITM_R,                       "R",                                           "R",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  568 */  { addItemToBuffer,              ITM_S,                       "S",                                           "S",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  569 */  { addItemToBuffer,              ITM_T,                       "T",                                           "T",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  570 */  { addItemToBuffer,              ITM_U,                       "U",                                           "U",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  571 */  { addItemToBuffer,              ITM_V,                       "V",                                           "V",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  572 */  { addItemToBuffer,              ITM_W,                       "W",                                           "W",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  573 */  { addItemToBuffer,              ITM_X,                       "X",                                           "X",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  574 */  { addItemToBuffer,              ITM_Y,                       "Y",                                           "Y",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  575 */  { addItemToBuffer,              ITM_Z,                       "Z",                                           "Z",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  576 */  { addItemToBuffer,              ITM_a,                       "a",                                           "a",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  577 */  { addItemToBuffer,              ITM_b,                       "b",                                           "b",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  578 */  { addItemToBuffer,              ITM_c,                       "c",                                           "c",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  579 */  { addItemToBuffer,              ITM_d,                       "d",                                           "d",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  580 */  { addItemToBuffer,              ITM_e,                       "e",                                           "e",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  581 */  { addItemToBuffer,              ITM_f,                       "f",                                           "f",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  582 */  { addItemToBuffer,              ITM_g,                       "g",                                           "g",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  583 */  { addItemToBuffer,              ITM_h,                       "h",                                           "h",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  584 */  { addItemToBuffer,              ITM_i,                       "i",                                           "i",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  585 */  { addItemToBuffer,              ITM_j,                       "j",                                           "j",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  586 */  { addItemToBuffer,              ITM_k,                       "k",                                           "k",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  587 */  { addItemToBuffer,              ITM_l,                       "l",                                           "l",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  588 */  { addItemToBuffer,              ITM_m,                       "m",                                           "m",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  589 */  { addItemToBuffer,              ITM_n,                       "n",                                           "n",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  590 */  { addItemToBuffer,              ITM_o,                       "o",                                           "o",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  591 */  { addItemToBuffer,              ITM_p,                       "p",                                           "p",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  592 */  { addItemToBuffer,              ITM_q,                       "q",                                           "q",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  593 */  { addItemToBuffer,              ITM_r,                       "r",                                           "r",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  594 */  { addItemToBuffer,              ITM_s,                       "s",                                           "s",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  595 */  { addItemToBuffer,              ITM_t,                       "t",                                           "t",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  596 */  { addItemToBuffer,              ITM_u,                       "u",                                           "u",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  597 */  { addItemToBuffer,              ITM_v,                       "v",                                           "v",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  598 */  { addItemToBuffer,              ITM_w,                       "w",                                           "w",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  599 */  { addItemToBuffer,              ITM_x,                       "x",                                           "x",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  600 */  { addItemToBuffer,              ITM_y,                       "y",                                           "y",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  601 */  { addItemToBuffer,              ITM_z,                       "z",                                           "z",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  602 */  { addItemToBuffer,              ITM_ALPHA,                   "",                                            STD_ALPHA,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  603 */  { addItemToBuffer,              ITM_BETA,                    "",                                            STD_BETA,                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  604 */  { addItemToBuffer,              ITM_GAMMA,                   "",                                            STD_GAMMA,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  605 */  { addItemToBuffer,              ITM_DELTA,                   "",                                            STD_DELTA,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  606 */  { addItemToBuffer,              ITM_EPSILON,                 "",                                            STD_EPSILON,                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  607 */  { addItemToBuffer,              ITM_ZETA,                    "",                                            STD_ZETA,                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  608 */  { addItemToBuffer,              ITM_ETA,                     "",                                            STD_ETA,                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  609 */  { addItemToBuffer,              ITM_THETA,                   "",                                            STD_THETA,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  610 */  { addItemToBuffer,              ITM_IOTA,                    "",                                            STD_IOTA,                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  611 */  { addItemToBuffer,              ITM_IOTA_DIALYTIKA,          "",                                            STD_IOTA_DIALYTIKA,                            (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  612 */  { addItemToBuffer,              ITM_KAPPA,                   "",                                            STD_KAPPA,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  613 */  { addItemToBuffer,              ITM_LAMBDA,                  "",                                            STD_LAMBDA,                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  614 */  { addItemToBuffer,              ITM_MU,                      "",                                            STD_MU,                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  615 */  { addItemToBuffer,              ITM_NU,                      "",                                            STD_NU,                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  616 */  { addItemToBuffer,              ITM_XI,                      "",                                            STD_XI,                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  617 */  { addItemToBuffer,              ITM_OMICRON,                 "",                                            STD_OMICRON,                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  618 */  { addItemToBuffer,              ITM_PI,                      "",                                            STD_PI,                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  619 */  { addItemToBuffer,              ITM_RHO,                     "",                                            STD_RHO,                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  620 */  { addItemToBuffer,              ITM_SIGMA,                   "",                                            STD_SIGMA,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  621 */  { addItemToBuffer,              ITM_TAU,                     "",                                            STD_TAU,                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  622 */  { addItemToBuffer,              ITM_UPSILON,                 "",                                            STD_UPSILON,                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  623 */  { addItemToBuffer,              ITM_UPSILON_DIALYTIKA,       "",                                            STD_UPSILON_DIALYTIKA,                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  624 */  { addItemToBuffer,              ITM_PHI,                     "",                                            STD_PHI,                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  625 */  { addItemToBuffer,              ITM_CHI,                     "",                                            STD_CHI,                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  626 */  { addItemToBuffer,              ITM_PSI,                     "",                                            STD_PSI,                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  627 */  { addItemToBuffer,              ITM_OMEGA,                   "",                                            STD_OMEGA,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  628 */  { addItemToBuffer,              ITM_alpha,                   "",                                            STD_alpha,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  629 */  { addItemToBuffer,              ITM_beta,                    "",                                            STD_beta,                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  630 */  { addItemToBuffer,              ITM_gamma,                   "",                                            STD_gamma,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  631 */  { addItemToBuffer,              ITM_delta,                   "",                                            STD_delta,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  632 */  { addItemToBuffer,              ITM_epsilon,                 "",                                            STD_epsilon,                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  633 */  { addItemToBuffer,              ITM_zeta,                    "",                                            STD_zeta,                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  634 */  { addItemToBuffer,              ITM_eta,                     "",                                            STD_eta,                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  635 */  { addItemToBuffer,              ITM_theta,                   "",                                            STD_theta,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  636 */  { addItemToBuffer,              ITM_iota,                    "",                                            STD_iota,                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  637 */  { addItemToBuffer,              ITM_iota_DIALYTIKA,          "",                                            STD_iota_DIALYTIKA,                            (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  638 */  { addItemToBuffer,              ITM_kappa,                   "",                                            STD_kappa,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  639 */  { addItemToBuffer,              ITM_lambda,                  "",                                            STD_lambda,                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  640 */  { addItemToBuffer,              ITM_mu,                      "",                                            STD_mu,                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  641 */  { addItemToBuffer,              ITM_nu,                      "",                                            STD_nu,                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  642 */  { addItemToBuffer,              ITM_xi,                      "",                                            STD_xi,                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  643 */  { addItemToBuffer,              ITM_omicron,                 "",                                            STD_omicron,                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  644 */  { addItemToBuffer,              ITM_pi,                      "",                                            STD_pi,                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  645 */  { addItemToBuffer,              ITM_rho,                     "",                                            STD_rho,                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  646 */  { addItemToBuffer,              ITM_sigma,                   "",                                            STD_sigma,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  647 */  { addItemToBuffer,              ITM_tau,                     "",                                            STD_tau,                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  648 */  { addItemToBuffer,              ITM_upsilon,                 "",                                            STD_upsilon,                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  649 */  { addItemToBuffer,              ITM_upsilon_DIALYTIKA,       "",                                            STD_upsilon_DIALYTIKA,                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  650 */  { addItemToBuffer,              ITM_phi,                     "",                                            STD_phi,                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  651 */  { addItemToBuffer,              ITM_chi,                     "",                                            STD_chi,                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  652 */  { addItemToBuffer,              ITM_psi,                     "",                                            STD_psi,                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  653 */  { addItemToBuffer,              ITM_omega,                   "",                                            STD_omega,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  654 */  { addItemToBuffer,              ITM_alpha_TONOS,             "",                                            STD_alpha_TONOS,                               (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  655 */  { addItemToBuffer,              ITM_epsilon_TONOS,           "",                                            STD_epsilon_TONOS,                             (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  656 */  { addItemToBuffer,              ITM_eta_TONOS,               "",                                            STD_eta_TONOS,                                 (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  657 */  { addItemToBuffer,              ITM_iotaTON,                 "",                                            STD_iota_TONOS,                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  658 */  { addItemToBuffer,              ITM_iota_DIALYTIKA_TONOS,    "",                                            STD_iota_DIALYTIKA_TONOS,                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  659 */  { addItemToBuffer,              ITM_omicron_TONOS,           "",                                            STD_omicron_TONOS,                             (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  660 */  { addItemToBuffer,              ITM_sigma_end,               "",                                            STD_sigma_end,                                 (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  661 */  { addItemToBuffer,              ITM_upsilon_TONOS,           "",                                            STD_upsilon_TONOS,                             (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  662 */  { addItemToBuffer,              ITM_upsilon_DIALYTIKA_TONOS, "",                                            STD_upsilon_DIALYTIKA_TONOS,                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  663 */  { addItemToBuffer,              ITM_omega_TONOS,             "",                                            STD_omega_TONOS,                               (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  664 */  { addItemToBuffer,              ITM_A_MACRON,                STD_A_MACRON,                                  STD_A_MACRON,                                  (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  665 */  { addItemToBuffer,              ITM_A_ACUTE,                 STD_A_ACUTE,                                   STD_A_ACUTE,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  666 */  { addItemToBuffer,              ITM_A_BREVE,                 STD_A_BREVE,                                   STD_A_BREVE,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  667 */  { addItemToBuffer,              ITM_A_GRAVE,                 STD_A_GRAVE,                                   STD_A_GRAVE,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  668 */  { addItemToBuffer,              ITM_A_DIARESIS,              STD_A_DIARESIS,                                STD_A_DIARESIS,                                (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  669 */  { addItemToBuffer,              ITM_A_TILDE,                 STD_A_TILDE,                                   STD_A_TILDE,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  670 */  { addItemToBuffer,              ITM_A_CIRC,                  STD_A_CIRC,                                    STD_A_CIRC,                                    (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  671 */  { addItemToBuffer,              ITM_A_RING,                  STD_A_RING,                                    STD_A_RING,                                    (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  672 */  { addItemToBuffer,              ITM_AE,                      STD_AE,                                        STD_AE,                                        (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  673 */  { addItemToBuffer,              ITM_A_OGONEK,                STD_A_OGONEK,                                  STD_A_OGONEK,                                  (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  674 */  { addItemToBuffer,              ITM_C_ACUTE,                 STD_C_ACUTE,                                   STD_C_ACUTE,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  675 */  { addItemToBuffer,              ITM_C_CARON,                 STD_C_CARON,                                   STD_C_CARON,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  676 */  { addItemToBuffer,              ITM_C_CEDILLA,               STD_C_CEDILLA,                                 STD_C_CEDILLA,                                 (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  677 */  { addItemToBuffer,              ITM_D_STROKE,                STD_D_STROKE,                                  STD_D_STROKE,                                  (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  678 */  { addItemToBuffer,              ITM_D_CARON,                 STD_D_CARON,                                   STD_D_CARON,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  679 */  { addItemToBuffer,              ITM_E_MACRON,                STD_E_MACRON,                                  STD_E_MACRON,                                  (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  680 */  { addItemToBuffer,              ITM_E_ACUTE,                 STD_E_ACUTE,                                   STD_E_ACUTE,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  681 */  { addItemToBuffer,              ITM_E_BREVE,                 STD_E_BREVE,                                   STD_E_BREVE,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  682 */  { addItemToBuffer,              ITM_E_GRAVE,                 STD_E_GRAVE,                                   STD_E_GRAVE,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  683 */  { addItemToBuffer,              ITM_E_DIARESIS,              STD_E_DIARESIS,                                STD_E_DIARESIS,                                (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  684 */  { addItemToBuffer,              ITM_E_CIRC,                  STD_E_CIRC,                                    STD_E_CIRC,                                    (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  685 */  { addItemToBuffer,              ITM_E_OGONEK,                STD_E_OGONEK,                                  STD_E_OGONEK,                                  (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  686 */  { addItemToBuffer,              ITM_G_BREVE,                 STD_G_BREVE,                                   STD_G_BREVE,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  687 */  { addItemToBuffer,              ITM_I_MACRON,                STD_I_MACRON,                                  STD_I_MACRON,                                  (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  688 */  { addItemToBuffer,              ITM_I_ACUTE,                 STD_I_ACUTE,                                   STD_I_ACUTE,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  689 */  { addItemToBuffer,              ITM_I_BREVE,                 STD_I_BREVE,                                   STD_I_BREVE,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  690 */  { addItemToBuffer,              ITM_I_GRAVE,                 STD_I_GRAVE,                                   STD_I_GRAVE,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  691 */  { addItemToBuffer,              ITM_I_DIARESIS,              STD_I_DIARESIS,                                STD_I_DIARESIS,                                (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  692 */  { addItemToBuffer,              ITM_I_CIRC,                  STD_I_CIRC,                                    STD_I_CIRC,                                    (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  693 */  { addItemToBuffer,              ITM_I_OGONEK,                STD_I_OGONEK,                                  STD_I_OGONEK,                                  (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  694 */  { addItemToBuffer,              ITM_I_DOT,                   STD_I_DOT,                                     STD_I_DOT,                                     (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  695 */  { addItemToBuffer,              ITM_I_DOTLESS,               "I",                                           "I",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  696 */  { addItemToBuffer,              ITM_L_STROKE,                STD_L_STROKE,                                  STD_L_STROKE,                                  (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  697 */  { addItemToBuffer,              ITM_L_ACUTE,                 STD_L_ACUTE,                                   STD_L_ACUTE,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  698 */  { addItemToBuffer,              ITM_L_APOSTROPHE,            STD_L_APOSTROPHE,                              STD_L_APOSTROPHE,                              (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  699 */  { addItemToBuffer,              ITM_N_ACUTE,                 STD_N_ACUTE,                                   STD_N_ACUTE,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  700 */  { addItemToBuffer,              ITM_N_CARON,                 STD_N_CARON,                                   STD_N_CARON,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  701 */  { addItemToBuffer,              ITM_N_TILDE,                 STD_N_TILDE,                                   STD_N_TILDE,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  702 */  { addItemToBuffer,              ITM_O_MACRON,                STD_O_MACRON,                                  STD_O_MACRON,                                  (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  703 */  { addItemToBuffer,              ITM_O_ACUTE,                 STD_O_ACUTE,                                   STD_O_ACUTE,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  704 */  { addItemToBuffer,              ITM_O_BREVE,                 STD_O_BREVE,                                   STD_O_BREVE,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  705 */  { addItemToBuffer,              ITM_O_GRAVE,                 STD_O_GRAVE,                                   STD_O_GRAVE,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  706 */  { addItemToBuffer,              ITM_O_DIARESIS,              STD_O_DIARESIS,                                STD_O_DIARESIS,                                (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  707 */  { addItemToBuffer,              ITM_O_TILDE,                 STD_O_TILDE,                                   STD_O_TILDE,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  708 */  { addItemToBuffer,              ITM_O_CIRC,                  STD_O_CIRC,                                    STD_O_CIRC,                                    (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  709 */  { addItemToBuffer,              ITM_O_STROKE,                STD_O_STROKE,                                  STD_O_STROKE,                                  (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  710 */  { addItemToBuffer,              ITM_OE,                      STD_OE,                                        STD_OE,                                        (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  711 */  { addItemToBuffer,              ITM_S_SHARP,                 STD_s_SHARP,                                   STD_s_SHARP,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  712 */  { addItemToBuffer,              ITM_S_ACUTE,                 STD_S_ACUTE,                                   STD_S_ACUTE,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  713 */  { addItemToBuffer,              ITM_S_CARON,                 STD_S_CARON,                                   STD_S_CARON,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  714 */  { addItemToBuffer,              ITM_S_CEDILLA,               STD_S_CEDILLA,                                 STD_S_CEDILLA,                                 (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  715 */  { addItemToBuffer,              ITM_T_CARON,                 STD_T_CARON,                                   STD_T_CARON,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  716 */  { addItemToBuffer,              ITM_T_CEDILLA,               STD_T_CEDILLA,                                 STD_T_CEDILLA,                                 (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  717 */  { addItemToBuffer,              ITM_U_MACRON,                STD_U_MACRON,                                  STD_U_MACRON,                                  (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  718 */  { addItemToBuffer,              ITM_U_ACUTE,                 STD_U_ACUTE,                                   STD_U_ACUTE,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  719 */  { addItemToBuffer,              ITM_U_BREVE,                 STD_U_BREVE,                                   STD_U_BREVE,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  720 */  { addItemToBuffer,              ITM_U_GRAVE,                 STD_U_GRAVE,                                   STD_U_GRAVE,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  721 */  { addItemToBuffer,              ITM_U_DIARESIS,              STD_U_DIARESIS,                                STD_U_DIARESIS,                                (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  722 */  { addItemToBuffer,              ITM_U_TILDE,                 STD_U_TILDE,                                   STD_U_TILDE,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  723 */  { addItemToBuffer,              ITM_U_CIRC,                  STD_U_CIRC,                                    STD_U_CIRC,                                    (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  724 */  { addItemToBuffer,              ITM_U_RING,                  STD_U_RING,                                    STD_U_RING,                                    (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  725 */  { addItemToBuffer,              ITM_W_CIRC,                  STD_W_CIRC,                                    STD_W_CIRC,                                    (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  726 */  { addItemToBuffer,              ITM_Y_CIRC,                  STD_Y_CIRC,                                    STD_Y_CIRC,                                    (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  727 */  { addItemToBuffer,              ITM_Y_ACUTE,                 STD_Y_ACUTE,                                   STD_Y_ACUTE,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  728 */  { addItemToBuffer,              ITM_Y_DIARESIS,              STD_Y_DIARESIS,                                STD_Y_DIARESIS,                                (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  729 */  { addItemToBuffer,              ITM_Z_ACUTE,                 STD_Z_ACUTE,                                   STD_Z_ACUTE,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  730 */  { addItemToBuffer,              ITM_Z_CARON,                 STD_Z_CARON,                                   STD_Z_CARON,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  731 */  { addItemToBuffer,              ITM_Z_DOT,                   STD_Z_DOT,                                     STD_Z_DOT,                                     (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  732 */  { addItemToBuffer,              ITM_a_MACRON,                STD_a_MACRON,                                  STD_a_MACRON,                                  (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  733 */  { addItemToBuffer,              ITM_a_ACUTE,                 STD_a_ACUTE,                                   STD_a_ACUTE,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  734 */  { addItemToBuffer,              ITM_a_BREVE,                 STD_a_BREVE,                                   STD_a_BREVE,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  735 */  { addItemToBuffer,              ITM_a_GRAVE,                 STD_a_GRAVE,                                   STD_a_GRAVE,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  736 */  { addItemToBuffer,              ITM_a_DIARESIS,              STD_a_DIARESIS,                                STD_a_DIARESIS,                                (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  737 */  { addItemToBuffer,              ITM_a_TILDE,                 STD_a_TILDE,                                   STD_a_TILDE,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  738 */  { addItemToBuffer,              ITM_a_CIRC,                  STD_a_CIRC,                                    STD_a_CIRC,                                    (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  739 */  { addItemToBuffer,              ITM_a_RING,                  STD_a_RING,                                    STD_a_RING,                                    (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  740 */  { addItemToBuffer,              ITM_ae,                      STD_ae,                                        STD_ae,                                        (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  741 */  { addItemToBuffer,              ITM_a_OGONEK,                STD_a_OGONEK,                                  STD_a_OGONEK,                                  (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  742 */  { addItemToBuffer,              ITM_c_ACUTE,                 STD_c_ACUTE,                                   STD_c_ACUTE,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  743 */  { addItemToBuffer,              ITM_c_CARON,                 STD_c_CARON,                                   STD_c_CARON,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  744 */  { addItemToBuffer,              ITM_c_CEDILLA,               STD_c_CEDILLA,                                 STD_c_CEDILLA,                                 (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  745 */  { addItemToBuffer,              ITM_d_STROKE,                STD_d_STROKE,                                  STD_d_STROKE,                                  (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  746 */  { addItemToBuffer,              ITM_d_APOSTROPHE,            STD_d_APOSTROPHE,                              STD_d_APOSTROPHE,                              (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  747 */  { addItemToBuffer,              ITM_e_MACRON,                STD_e_MACRON,                                  STD_e_MACRON,                                  (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  748 */  { addItemToBuffer,              ITM_e_ACUTE,                 STD_e_ACUTE,                                   STD_e_ACUTE,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  749 */  { addItemToBuffer,              ITM_e_BREVE,                 STD_e_BREVE,                                   STD_e_BREVE,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  750 */  { addItemToBuffer,              ITM_e_GRAVE,                 STD_e_GRAVE,                                   STD_e_GRAVE,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  751 */  { addItemToBuffer,              ITM_e_DIARESIS,              STD_e_DIARESIS,                                STD_e_DIARESIS,                                (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  752 */  { addItemToBuffer,              ITM_e_CIRC,                  STD_e_CIRC,                                    STD_e_CIRC,                                    (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  753 */  { addItemToBuffer,              ITM_e_OGONEK,                STD_e_OGONEK,                                  STD_e_OGONEK,                                  (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  754 */  { addItemToBuffer,              ITM_g_BREVE,                 STD_g_BREVE,                                   STD_g_BREVE,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  755 */  { addItemToBuffer,              ITM_h_STROKE,                "",                                            STD_h_STROKE,                                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  756 */  { addItemToBuffer,              ITM_i_MACRON,                STD_i_MACRON,                                  STD_i_MACRON,                                  (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  757 */  { addItemToBuffer,              ITM_i_ACUTE,                 STD_i_ACUTE,                                   STD_i_ACUTE,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  758 */  { addItemToBuffer,              ITM_i_BREVE,                 STD_i_BREVE,                                   STD_i_BREVE,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  759 */  { addItemToBuffer,              ITM_i_GRAVE,                 STD_i_GRAVE,                                   STD_i_GRAVE,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  760 */  { addItemToBuffer,              ITM_i_DIARESIS,              STD_i_DIARESIS,                                STD_i_DIARESIS,                                (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  761 */  { addItemToBuffer,              ITM_i_CIRC,                  STD_i_CIRC,                                    STD_i_CIRC,                                    (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  762 */  { addItemToBuffer,              ITM_i_OGONEK,                STD_i_OGONEK,                                  STD_i_OGONEK,                                  (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  763 */  { addItemToBuffer,              ITM_i_DOT,                   "i",                                           "i",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  764 */  { addItemToBuffer,              ITM_i_DOTLESS,               STD_i_DOTLESS,                                 STD_i_DOTLESS,                                 (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  765 */  { addItemToBuffer,              ITM_l_STROKE,                STD_l_STROKE,                                  STD_l_STROKE,                                  (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  766 */  { addItemToBuffer,              ITM_l_ACUTE,                 STD_l_ACUTE,                                   STD_l_ACUTE,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  767 */  { addItemToBuffer,              ITM_l_APOSTROPHE,            STD_l_APOSTROPHE,                              STD_l_APOSTROPHE,                              (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  768 */  { addItemToBuffer,              ITM_n_ACUTE,                 STD_n_ACUTE,                                   STD_n_ACUTE,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  769 */  { addItemToBuffer,              ITM_n_CARON,                 STD_n_CARON,                                   STD_n_CARON,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  770 */  { addItemToBuffer,              ITM_n_TILDE,                 STD_n_TILDE,                                   STD_n_TILDE,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  771 */  { addItemToBuffer,              ITM_o_MACRON,                STD_o_MACRON,                                  STD_o_MACRON,                                  (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  772 */  { addItemToBuffer,              ITM_o_ACUTE,                 STD_o_ACUTE,                                   STD_o_ACUTE,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  773 */  { addItemToBuffer,              ITM_o_BREVE,                 STD_o_BREVE,                                   STD_o_BREVE,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  774 */  { addItemToBuffer,              ITM_o_GRAVE,                 STD_o_GRAVE,                                   STD_o_GRAVE,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  775 */  { addItemToBuffer,              ITM_o_DIARESIS,              STD_o_DIARESIS,                                STD_o_DIARESIS,                                (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  776 */  { addItemToBuffer,              ITM_o_TILDE,                 STD_o_TILDE,                                   STD_o_TILDE,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  777 */  { addItemToBuffer,              ITM_o_CIRC,                  STD_o_CIRC,                                    STD_o_CIRC,                                    (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  778 */  { addItemToBuffer,              ITM_o_STROKE,                STD_o_STROKE,                                  STD_o_STROKE,                                  (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  779 */  { addItemToBuffer,              ITM_oe,                      STD_oe,                                        STD_oe,                                        (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  780 */  { addItemToBuffer,              ITM_r_CARON,                 STD_r_CARON,                                   STD_r_CARON,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  781 */  { addItemToBuffer,              ITM_r_ACUTE,                 STD_r_ACUTE,                                   STD_r_ACUTE,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  782 */  { addItemToBuffer,              ITM_s_SHARP,                 STD_s_SHARP,                                   STD_s_SHARP,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  783 */  { addItemToBuffer,              ITM_s_ACUTE,                 STD_s_ACUTE,                                   STD_s_ACUTE,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  784 */  { addItemToBuffer,              ITM_s_CARON,                 STD_s_CARON,                                   STD_s_CARON,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  785 */  { addItemToBuffer,              ITM_s_CEDILLA,               STD_s_CEDILLA,                                 STD_s_CEDILLA,                                 (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  786 */  { addItemToBuffer,              ITM_t_APOSTROPHE,            STD_t_APOSTROPHE,                              STD_t_APOSTROPHE,                              (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  787 */  { addItemToBuffer,              ITM_t_CEDILLA,               STD_t_CEDILLA,                                 STD_t_CEDILLA,                                 (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  788 */  { addItemToBuffer,              ITM_u_MACRON,                STD_u_MACRON,                                  STD_u_MACRON,                                  (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  789 */  { addItemToBuffer,              ITM_u_ACUTE,                 STD_u_ACUTE,                                   STD_u_ACUTE,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  790 */  { addItemToBuffer,              ITM_u_BREVE,                 STD_u_BREVE,                                   STD_u_BREVE,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  791 */  { addItemToBuffer,              ITM_u_GRAVE,                 STD_u_GRAVE,                                   STD_u_GRAVE,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  792 */  { addItemToBuffer,              ITM_u_DIARESIS,              STD_u_DIARESIS,                                STD_u_DIARESIS,                                (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  793 */  { addItemToBuffer,              ITM_u_TILDE,                 STD_u_TILDE,                                   STD_u_TILDE,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  794 */  { addItemToBuffer,              ITM_u_CIRC,                  STD_u_CIRC,                                    STD_u_CIRC,                                    (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  795 */  { addItemToBuffer,              ITM_u_RING,                  STD_u_RING,                                    STD_u_RING,                                    (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  796 */  { addItemToBuffer,              ITM_w_CIRC,                  STD_w_CIRC,                                    STD_w_CIRC,                                    (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  797 */  { addItemToBuffer,              ITM_x_BAR,                   "",                                            STD_x_BAR,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  798 */  { addItemToBuffer,              ITM_x_CIRC,                  "",                                            STD_x_CIRC,                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  799 */  { addItemToBuffer,              ITM_y_BAR,                   "",                                            STD_y_BAR,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  800 */  { addItemToBuffer,              ITM_y_CIRC,                  STD_y_CIRC,                                    STD_y_CIRC,                                    (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  801 */  { addItemToBuffer,              ITM_y_ACUTE,                 STD_y_ACUTE,                                   STD_y_ACUTE,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  802 */  { addItemToBuffer,              ITM_y_DIARESIS,              STD_y_DIARESIS,                                STD_y_DIARESIS,                                (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  803 */  { addItemToBuffer,              ITM_z_ACUTE,                 STD_z_ACUTE,                                   STD_z_ACUTE,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  804 */  { addItemToBuffer,              ITM_z_CARON,                 STD_z_CARON,                                   STD_z_CARON,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  805 */  { addItemToBuffer,              ITM_z_DOT,                   STD_z_DOT,                                     STD_z_DOT,                                     (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  806 */  { addItemToBuffer,              ITM_SPACE,                   "",                                            STD_SPACE,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  807 */  { addItemToBuffer,              ITM_EXCLAMATION_MARK,        "",                                            STD_EXCLAMATION_MARK,                          (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  808 */  { addItemToBuffer,              ITM_DOUBLE_QUOTE,            "",                                            STD_DOUBLE_QUOTE,                              (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  809 */  { addItemToBuffer,              ITM_NUMBER_SIGN,             "",                                            STD_NUMBER_SIGN,                               (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  810 */  { addItemToBuffer,              ITM_DOLLAR,                  "",                                            STD_DOLLAR,                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  811 */  { addItemToBuffer,              ITM_PERCENT,                 "",                                            STD_PERCENT,                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  812 */  { addItemToBuffer,              ITM_AMPERSAND,               "",                                            STD_AMPERSAND,                                 (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  813 */  { addItemToBuffer,              ITM_QUOTE,                   "",                                            STD_QUOTE,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  814 */  { addItemToBuffer,              ITM_LEFT_PARENTHESIS,        "",                                            STD_LEFT_PARENTHESIS,                          (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  815 */  { addItemToBuffer,              ITM_RIGHT_PARENTHESIS,       "",                                            STD_RIGHT_PARENTHESIS,                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  816 */  { addItemToBuffer,              ITM_ASTERISK,                "",                                            STD_ASTERISK,                                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  817 */  { addItemToBuffer,              ITM_PLUS,                    "",                                            STD_PLUS,                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  818 */  { addItemToBuffer,              ITM_COMMA,                   "",                                            STD_COMMA,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  819 */  { addItemToBuffer,              ITM_MINUS,                   "",                                            STD_MINUS,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  820 */  { addItemToBuffer,              ITM_PERIOD,                  "",                                            STD_PERIOD,                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  821 */  { addItemToBuffer,              ITM_SLASH,                   "",                                            STD_SLASH,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  822 */  { addItemToBuffer,              ITM_COLON,                   "",                                            STD_COLON,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  823 */  { addItemToBuffer,              ITM_SEMICOLON,               "",                                            STD_SEMICOLON,                                 (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  824 */  { addItemToBuffer,              ITM_LESS_THAN,               "",                                            STD_LESS_THAN,                                 (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  825 */  { addItemToBuffer,              ITM_EQUAL,                   "",                                            STD_EQUAL,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  826 */  { addItemToBuffer,              ITM_GREATER_THAN,            "",                                            STD_GREATER_THAN,                              (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  827 */  { addItemToBuffer,              ITM_QUESTION_MARK,           "",                                            STD_QUESTION_MARK,                             (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  828 */  { addItemToBuffer,              ITM_AT,                      "",                                            STD_AT,                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  829 */  { addItemToBuffer,              ITM_LEFT_SQUARE_BRACKET,     "",                                            STD_LEFT_SQUARE_BRACKET,                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  830 */  { addItemToBuffer,              ITM_BACK_SLASH,              "",                                            STD_BACK_SLASH,                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  831 */  { addItemToBuffer,              ITM_RIGHT_SQUARE_BRACKET,    "",                                            STD_RIGHT_SQUARE_BRACKET,                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  832 */  { addItemToBuffer,              ITM_CIRCUMFLEX,              "",                                            STD_CIRCUMFLEX,                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  833 */  { addItemToBuffer,              ITM_UNDERSCORE,              "",                                            STD_UNDERSCORE,                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  834 */  { addItemToBuffer,              ITM_LEFT_CURLY_BRACKET,      "",                                            STD_LEFT_CURLY_BRACKET,                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  835 */  { addItemToBuffer,              ITM_PIPE,                    "",                                            STD_PIPE,                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  836 */  { addItemToBuffer,              ITM_RIGHT_CURLY_BRACKET,     "",                                            STD_RIGHT_CURLY_BRACKET,                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  837 */  { addItemToBuffer,              ITM_TILDE,                   "",                                            STD_TILDE,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  838 */  { addItemToBuffer,              ITM_INVERTED_EXCLAMATION_MARK, "",                                          STD_INVERTED_EXCLAMATION_MARK,                 (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  839 */  { addItemToBuffer,              ITM_CENT,                    "",                                            STD_CENT,                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  840 */  { addItemToBuffer,              ITM_POUND,                   "",                                            STD_POUND,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  841 */  { addItemToBuffer,              ITM_YEN,                     "",                                            STD_YEN,                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  842 */  { addItemToBuffer,              ITM_SECTION,                 "",                                            STD_SECTION,                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  843 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_OVERFLOW_CARRY,                            (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  844 */  { addItemToBuffer,              ITM_LEFT_DOUBLE_ANGLE,       "",                                            STD_LEFT_DOUBLE_ANGLE,                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  845 */  { addItemToBuffer,              ITM_NOT,                     "",                                            STD_NOT,                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  846 */  { addItemToBuffer,              ITM_DEGREE,                  "",                                            STD_DEGREE,                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  847 */  { addItemToBuffer,              ITM_PLUS_MINUS,              "",                                            STD_PLUS_MINUS,                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  848 */  { addItemToBuffer,              ITM_MICRO,                   "",                                            STD_MICRO,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  849 */  { addItemToBuffer,              ITM_DOT,                     "",                                            STD_DOT,                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  850 */  { addItemToBuffer,              ITM_RIGHT_DOUBLE_ANGLE,      "",                                            STD_RIGHT_DOUBLE_ANGLE,                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  851 */  { addItemToBuffer,              ITM_ONE_HALF,                "",                                            STD_ONE_HALF,                                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  852 */  { addItemToBuffer,              ITM_ONE_QUARTER,             "",                                            STD_ONE_QUARTER,                               (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  853 */  { addItemToBuffer,              ITM_INVERTED_QUESTION_MARK,  "",                                            STD_INVERTED_QUESTION_MARK,                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  854 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_ETH,                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  855 */  { addItemToBuffer,              ITM_CROSS,                   "",                                            STD_CROSS,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  856 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_eth,                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  857 */  { addItemToBuffer,              ITM_OBELUS,                  "",                                            STD_DIVIDE,                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  858 */  { addItemToBuffer,              ITM_E_DOT,                   STD_E_DOT,                                     STD_E_DOT,                                     (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  859 */  { addItemToBuffer,              ITM_e_DOT,                   STD_e_DOT,                                     STD_e_DOT,                                     (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  860 */  { addItemToBuffer,              ITM_E_CARON,                 STD_E_CARON,                                   STD_E_CARON,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  861 */  { addItemToBuffer,              ITM_e_CARON,                 STD_e_CARON,                                   STD_e_CARON,                                   (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  862 */  { addItemToBuffer,              ITM_R_ACUTE,                 STD_R_ACUTE,                                   STD_R_ACUTE,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  863 */  { addItemToBuffer,              ITM_R_CARON,                 STD_R_CARON,                                   STD_R_CARON,                                   (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  864 */  { addItemToBuffer,              ITM_U_OGONEK,                STD_U_OGONEK,                                  STD_U_OGONEK,                                  (0 << TAM_MAX_BITS) |     0, CAT_AINT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  865 */  { addItemToBuffer,              ITM_u_OGONEK,                STD_u_OGONEK,                                  STD_u_OGONEK,                                  (0 << TAM_MAX_BITS) |     0, CAT_aint | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  866 */  { addItemToBuffer,              ITM_y_UNDER_ROOT,            "",                                            STD_y_UNDER_ROOT,                              (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  867 */  { addItemToBuffer,              ITM_x_UNDER_ROOT,            "",                                            STD_x_UNDER_ROOT,                              (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  868 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SPACE_EM,                                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  869 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SPACE_3_PER_EM,                            (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  870 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SPACE_4_PER_EM,                            (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  871 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SPACE_6_PER_EM,                            (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  872 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SPACE_FIGURE,                              (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  873 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SPACE_PUNCTUATION,                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  874 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SPACE_HAIR,                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  875 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_LEFT_SINGLE_QUOTE,                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  876 */  { addItemToBuffer,              ITM_RIGHT_SINGLE_QUOTE,      "",                                            STD_RIGHT_SINGLE_QUOTE,                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  877 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SINGLE_LOW_QUOTE,                          (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  878 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SINGLE_HIGH_QUOTE,                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  879 */  { addItemToBuffer,              ITM_LEFT_DOUBLE_QUOTE,       "",                                            STD_LEFT_DOUBLE_QUOTE,                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  880 */  { addItemToBuffer,              ITM_RIGHT_DOUBLE_QUOTE,      "",                                            STD_RIGHT_DOUBLE_QUOTE,                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  881 */  { addItemToBuffer,              ITM_DOUBLE_LOW_QUOTE,        "",                                            STD_DOUBLE_LOW_QUOTE,                          (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  882 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_DOUBLE_HIGH_QUOTE,                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  883 */  { addItemToBuffer,              ITM_ELLIPSIS,                "",                                            STD_ELLIPSIS,                                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  884 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_BINARY_ONE,                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  885 */  { addItemToBuffer,              ITM_EURO,                    "",                                            STD_EURO,                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  886 */  { addItemToBuffer,              ITM_COMPLEX_C,               "",                                            STD_COMPLEX_C,                                 (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  887 */  { addItemToBuffer,              ITM_PLANCK,                  "",                                            STD_PLANCK,                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  888 */  { addItemToBuffer,              ITM_PLANCK_2PI,              "",                                            STD_PLANCK_2PI,                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  889 */  { addItemToBuffer,              ITM_NATURAL_N,               "",                                            STD_NATURAL_N,                                 (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  890 */  { addItemToBuffer,              ITM_RATIONAL_Q,              "",                                            STD_RATIONAL_Q,                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  891 */  { addItemToBuffer,              ITM_REAL_R,                  "",                                            STD_REAL_R,                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  892 */  { addItemToBuffer,              ITM_LEFT_ARROW,              "",                                            STD_LEFT_ARROW,                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  893 */  { addItemToBuffer,              ITM_UP_ARROW,                "",                                            STD_UP_ARROW,                                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  894 */  { addItemToBuffer,              ITM_RIGHT_ARROW,             "",                                            STD_RIGHT_ARROW,                               (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  895 */  { addItemToBuffer,              ITM_DOWN_ARROW,              "",                                            STD_DOWN_ARROW,                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  896 */  { addItemToBuffer,              ITM_SERIAL_IO,               "",                                            STD_SERIAL_IO,                                 (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  897 */  { addItemToBuffer,              ITM_RIGHT_SHORT_ARROW,       "",                                            STD_RIGHT_SHORT_ARROW,                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  898 */  { addItemToBuffer,              ITM_LEFT_RIGHT_ARROWS,       "",                                            STD_LEFT_RIGHT_ARROWS,                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  899 */  { addItemToBuffer,              ITM_BST_SIGN,                "",                                            STD_BST,                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  900 */  { addItemToBuffer,              ITM_SST_SIGN,                "",                                            STD_SST,                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  901 */  { addItemToBuffer,              ITM_HAMBURGER,               "",                                            STD_HAMBURGER,                                 (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  902 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_UNDO,                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  903 */  { addItemToBuffer,              ITM_FOR_ALL,                 "",                                            STD_FOR_ALL,                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  904 */  { addItemToBuffer,              ITM_COMPLEMENT,              "",                                            STD_COMPLEMENT,                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  905 */  { addItemToBuffer,              ITM_PARTIAL_DIFF,            "",                                            STD_PARTIAL_DIFF,                              (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  906 */  { addItemToBuffer,              ITM_THERE_EXISTS,            "",                                            STD_THERE_EXISTS,                              (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  907 */  { addItemToBuffer,              ITM_THERE_DOES_NOT_EXIST,    "",                                            STD_THERE_DOES_NOT_EXIST,                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  908 */  { addItemToBuffer,              ITM_EMPTY_SET,               "",                                            STD_EMPTY_SET,                                 (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  909 */  { addItemToBuffer,              ITM_INCREMENT,               "",                                            STD_INCREMENT,                                 (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  910 */  { addItemToBuffer,              ITM_NABLA,                   "",                                            STD_NABLA,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  911 */  { addItemToBuffer,              ITM_ELEMENT_OF,              "",                                            STD_ELEMENT_OF,                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  912 */  { addItemToBuffer,              ITM_NOT_ELEMENT_OF,          "",                                            STD_NOT_ELEMENT_OF,                            (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  913 */  { addItemToBuffer,              ITM_CONTAINS,                "",                                            STD_CONTAINS,                                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  914 */  { addItemToBuffer,              ITM_DOES_NOT_CONTAIN,        "",                                            STD_DOES_NOT_CONTAIN,                          (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  915 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_BINARY_ZERO,                               (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  916 */  { addItemToBuffer,              ITM_PRODUCT,                 "",                                            STD_PRODUCT,                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  917 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_MINUS_PLUS,                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  918 */  { addItemToBuffer,              ITM_RING,                    "",                                            STD_RING,                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  919 */  { addItemToBuffer,              ITM_BULLET,                  "",                                            STD_BULLET,                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  920 */  { addItemToBuffer,              ITM_SQUARE_ROOT,             "",                                            STD_SQUARE_ROOT,                               (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  921 */  { addItemToBuffer,              ITM_CUBEROOT_SIGN,           "",                                            STD_CUBE_ROOT,                                 (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  922 */  { addItemToBuffer,              ITM_xTH_ROOT,                "",                                            STD_xTH_ROOT,                                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  923 */  { addItemToBuffer,              ITM_PROPORTIONAL,            "",                                            STD_PROPORTIONAL,                              (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  924 */  { addItemToBuffer,              ITM_INFINITY,                "",                                            STD_INFINITY,                                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  925 */  { addItemToBuffer,              ITM_RIGHT_ANGLE,             "",                                            STD_RIGHT_ANGLE,                               (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  926 */  { addItemToBuffer,              ITM_IRRATIONAL_I,            "",                                            STD_IRRATIONAL_I,                              (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  927 */  { addItemToBuffer,              ITM_MEASURED_ANGLE,          "",                                            STD_MEASURED_ANGLE,                            (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  928 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_DIVIDES,                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  929 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_DOES_NOT_DIVIDE,                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  930 */  { addItemToBuffer,              ITM_PARALLEL_SIGN,           "",                                            STD_PARALLEL,                                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  931 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_NOT_PARALLEL,                              (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  932 */  { addItemToBuffer,              ITM_AND,                     "",                                            STD_AND,                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  933 */  { addItemToBuffer,              ITM_OR,                      "",                                            STD_OR,                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  934 */  { addItemToBuffer,              ITM_INTERSECTION,            "",                                            STD_INTERSECTION,                              (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  935 */  { addItemToBuffer,              ITM_UNION,                   "",                                            STD_UNION,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  936 */  { addItemToBuffer,              ITM_INTEGRAL_SIGN,           "",                                            STD_INTEGRAL,                                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  937 */  { addItemToBuffer,              ITM_DOUBLE_INTEGRAL,         "",                                            STD_DOUBLE_INTEGRAL,                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  938 */  { addItemToBuffer,              ITM_CONTOUR_INTEGRAL,        "",                                            STD_CONTOUR_INTEGRAL,                          (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  939 */  { addItemToBuffer,              ITM_SURFACE_INTEGRAL,        "",                                            STD_SURFACE_INTEGRAL,                          (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  940 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_RATIO,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  941 */  { addItemToBuffer,              ITM_CHECK_MARK,              "",                                            STD_CHECK_MARK,                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  942 */  { addItemToBuffer,              ITM_ASYMPOTICALLY_EQUAL,     "",                                            STD_ASYMPOTICALLY_EQUAL,                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  943 */  { addItemToBuffer,              ITM_ALMOST_EQUAL,            "",                                            STD_ALMOST_EQUAL,                              (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  944 */  { addItemToBuffer,              ITM_COLON_EQUALS,            "",                                            STD_COLON_EQUALS,                              (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  945 */  { addItemToBuffer,              ITM_CORRESPONDS_TO,          "",                                            STD_CORRESPONDS_TO,                            (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  946 */  { addItemToBuffer,              ITM_ESTIMATES,               "",                                            STD_ESTIMATES,                                 (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  947 */  { addItemToBuffer,              ITM_NOT_EQUAL,               "",                                            STD_NOT_EQUAL,                                 (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  948 */  { addItemToBuffer,              ITM_IDENTICAL_TO,            "",                                            STD_IDENTICAL_TO,                              (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  949 */  { addItemToBuffer,              ITM_LESS_EQUAL,              "",                                            STD_LESS_EQUAL,                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  950 */  { addItemToBuffer,              ITM_GREATER_EQUAL,           "",                                            STD_GREATER_EQUAL,                             (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  951 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_MUCH_LESS,                                 (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  952 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_MUCH_GREATER,                              (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  953 */  { addItemToBuffer,              ITM_SUN,                     "",                                            STD_SUN,                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  954 */  { addItemToBuffer,              ITM_TRANSPOSED,              "",                                            STD_TRANSPOSED,                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  955 */  { addItemToBuffer,              ITM_PERPENDICULAR,           "",                                            STD_PERPENDICULAR,                             (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  956 */  { addItemToBuffer,              ITM_XOR,                     "",                                            STD_XOR,                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  957 */  { addItemToBuffer,              ITM_NAND,                    "",                                            STD_NAND,                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  958 */  { addItemToBuffer,              ITM_NOR,                     "",                                            STD_NOR,                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  959 */  { addItemToBuffer,              ITM_WATCH,                   "",                                            STD_WATCH,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  960 */  { addItemToBuffer,              ITM_HOURGLASS,               "",                                            STD_HOURGLASS,                                 (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  961 */  { addItemToBuffer,              ITM_PRINTER,                 "",                                            STD_PRINTER,                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  962 */  { addItemToBuffer,              ITM_MAT_TL,                  "",                                            STD_MAT_TL,                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  963 */  { addItemToBuffer,              ITM_MAT_ML,                  "",                                            STD_MAT_ML,                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  964 */  { addItemToBuffer,              ITM_MAT_BL,                  "",                                            STD_MAT_BL,                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  965 */  { addItemToBuffer,              ITM_MAT_TR,                  "",                                            STD_MAT_TR,                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  966 */  { addItemToBuffer,              ITM_MAT_MR,                  "",                                            STD_MAT_MR,                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  967 */  { addItemToBuffer,              ITM_MAT_BR,                  "",                                            STD_MAT_BR,                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  968 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_OBLIQUE1,                                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  969 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_OBLIQUE2,                                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  970 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_OBLIQUE3,                                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  971 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_OBLIQUE4,                                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  972 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_CURSOR,                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  973 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_PERIOD34,                                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  974 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_COMMA34,                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  975 */  { addItemToBuffer,              ITM_BATTERY,                 "",                                            STD_BATTERY,                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  976 */  { addItemToBuffer,              ITM_PGM_BEGIN,               "",                                            STD_PGM_BEGIN,                                 (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  977 */  { addItemToBuffer,              ITM_USER_MODE,               "",                                            STD_USER_MODE,                                 (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  978 */  { addItemToBuffer,              ITM_UK,                      "",                                            STD_UK,                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  979 */  { addItemToBuffer,              ITM_US,                      "",                                            STD_US,                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  980 */  { addItemToBuffer,              ITM_NEG_EXCLAMATION_MARK,    "",                                            STD_NEG_EXCLAMATION_MARK,                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  981 */  { addItemToBuffer,              ITM_ex,                      "",                                            STD_RIGHT_OVER_LEFT_ARROW,                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  982 */  { itemToBeCoded,                NOPARAM,                     "0982",                                        "0982",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  983 */  { itemToBeCoded,                NOPARAM,                     "0983",                                        "0983",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  984 */  { itemToBeCoded,                NOPARAM,                     "0984",                                        "0984",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  985 */  { itemToBeCoded,                NOPARAM,                     "0985",                                        "0985",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  986 */  { itemToBeCoded,                NOPARAM,                     "0986",                                        "0986",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  987 */  { itemToBeCoded,                NOPARAM,                     "0987",                                        "0987",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  988 */  { addItemToBuffer,              ITM_0P,                      "0.",                                          "0.",                                          (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  989 */  { addItemToBuffer,              ITM_1P,                      "1.",                                          "1.",                                          (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  990 */  { addItemToBuffer,              ITM_EXPONENT/*jmok*/,        "",                                            "EEX",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_DISABLED        },
/*  991 */  { addItemToBuffer,              ITM_litre,                   "",                                            STD_litre,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  992 */  { fnGoToRow,                    TM_VALUE,                    "GOTO Row",                                    "GOTO",                                        (0 << TAM_MAX_BITS) |  9999, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  993 */  { fnGoToColumn,                 TM_VALUE,                    "GOTO Column",                                 "GOTO",                                        (0 << TAM_MAX_BITS) |  9999, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  994 */  { fnSolveVar,                   NOPARAM,                     "SOLVE Var",                                   "SOLVE",                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  995 */  { fnEqCursorLeft,               NOPARAM,                     "",                                            STD_LEFT_ARROW,                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  996 */  { fnEqCursorRight,              NOPARAM,                     "",                                            STD_RIGHT_ARROW,                               (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  997 */  { addItemToBuffer,              ITM_PAIR_OF_PARENTHESES,     "",                                            "( )",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  998 */  { addItemToBuffer,              ITM_VERTICAL_BAR,            "",                                            "|",                                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/*  999 */  { addItemToBuffer,              ITM_ALOG_SIGN,               "",                                            STD_EulerE STD_SUP_BOLD_x,                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1000 */  { addItemToBuffer,              ITM_ROOT_SIGN,               "",                                            STD_SQUARE_ROOT,                               (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1001 */  { addItemToBuffer,              ITM_TIMER_SYMBOL,            "",                                            STD_TIMER,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1002 */  { fnIntVar,                     NOPARAM,                     STD_INTEGRAL "fdx VarL", /*L*/                 STD_INTEGRAL "fdx E", /*E*/                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1003 */  { addItemToBuffer,              ITM_SUP_PLUS,                "",                                            STD_SUP_PLUS,                                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1004 */  { addItemToBuffer,              ITM_SUP_MINUS,               "",                                            STD_SUP_MINUS,                                 (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1005 */  { itemToBeCoded,                NOPARAM,                     "1005",                                        "1005",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1006 */  { addItemToBuffer,              ITM_SUP_INFINITY,            "",                                            STD_SUP_INFINITY,                              (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1007 */  { addItemToBuffer,              ITM_SUP_ASTERISK,            "",                                            STD_SUP_ASTERISK,                              (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1008 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_0,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1009 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_1,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1010 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_2,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1011 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_3,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1012 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_4,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1013 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_5,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1014 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_6,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1015 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_7,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1016 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_8,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1017 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_9,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1018 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_A,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1019 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_B,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1020 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_C,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1021 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_D,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1022 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_E,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1023 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_F,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1024 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_G,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1025 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_H,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1026 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_I,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1027 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_J,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1028 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_K,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1029 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_L,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1030 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_M,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1031 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_N,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1032 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_O,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1033 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_P,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1034 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_Q,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1035 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_R,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1036 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_S,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1037 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_T,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1038 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_U,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1039 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_V,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1040 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_W,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1041 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_X,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1042 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_Y,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1043 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_Z,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1044 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_a,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1045 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_b,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1046 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_c,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1047 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_d,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1048 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_e,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1049 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_f,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1050 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_g,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1051 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_h,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1052 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_i,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1053 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_j,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1054 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_k,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1055 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_l,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1056 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_m,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1057 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_n,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1058 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_o,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1059 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_p,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1060 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_q,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1061 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_r,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1062 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_s,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1063 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_t,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1064 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_u,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1065 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_v,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1066 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_w,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1067 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_x,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1068 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_y,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1069 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_z,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1070 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_alpha,                                 (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1071 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_delta,                                 (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1072 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_mu,                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1073 */  { addItemToBuffer,              ITM_SUB_SUN,                 "",                                            STD_SUB_SUN,                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1074 */  { addItemToBuffer,              ITM_SUB_EARTH,               "",                                            STD_SUB_EARTH,                                 (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1075 */  { addItemToBuffer,              ITM_SUB_PLUS,                "",                                            STD_SUB_PLUS,                                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1076 */  { addItemToBuffer,              ITM_SUB_MINUS,               "",                                            STD_SUB_MINUS,                                 (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1077 */  { addItemToBuffer,              ITM_SUB_INFINITY,            "",                                            STD_SUB_INFINITY,                              (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1078 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_10,                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1079 */  { addItemToBuffer,              ITM_SUB_E_OUTLINE,           "",                                            STD_SUB_E_OUTLINE,                             (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1080 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_0,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1081 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_1,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1082 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_2,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1083 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_3,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1084 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_4,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1085 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_5,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1086 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_6,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1087 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_7,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1088 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_8,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1089 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_9,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1090 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_A,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1091 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_B,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1092 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_C,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1093 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_D,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1094 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_E,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1095 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_F,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1096 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_G,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1097 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_H,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1098 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_I,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1099 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_J,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1100 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_K,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1101 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_L,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1102 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_M,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1103 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_N,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1104 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_O,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1105 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_P,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1106 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_Q,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1107 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_R,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1108 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_S,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1109 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_T,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1110 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_U,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1111 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_V,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1112 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_W,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1113 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_X,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1114 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_Y,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1115 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_Z,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1116 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_a,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1117 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_b,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1118 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_c,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1119 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_d,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1120 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_e,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1121 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_f,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1122 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_g,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1123 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_h,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1124 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_i,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1125 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_j,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1126 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_k,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1127 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_l,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1128 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_m,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1129 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_n,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1130 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_o,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1131 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_p,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1132 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_q,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1133 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_r,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1134 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_s,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1135 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_t,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1136 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_u,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1137 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_v,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1138 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_w,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1139 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_x,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1140 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_y,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1141 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUB_z,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1142 */  { addItemToBuffer,              ITM_SUB_pi,                  "",                                            STD_SUB_pi,                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1143 */  { addItemToBuffer,              ITM_SUP_pi,                  "",                                            STD_SUP_pi,                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1144 */  { addItemToBuffer,              ITM_CB_OFF,                  "",                                            STD_CB_OFF,                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1145 */  { addItemToBuffer,              ITM_CB_ON,                   "",                                            STD_CB_ON,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1146 */  { addItemToBuffer,              ITM_RB_OFF,                  "",                                            STD_RB_OFF,                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1147 */  { addItemToBuffer,              ITM_RB_ON ,                  "",                                            STD_RB_ON,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1148 */  { addItemToBuffer,              ITM_ALTERN_CURRENT,          "",                                            STD_AC,                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1149 */  { addItemToBuffer,              ITM_ANGLE,                   "",                                            STD_ANGLE,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1150 */  { addItemToBuffer,              ITM_BATTERYHALF,             "",                                            STD_BATTERYHALF,                               (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1151 */  { addItemToBuffer,              ITM_DIA_OFF,                 "",                                            STD_DIA_OFF,                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1152 */  { addItemToBuffer,              ITM_CYCLIC,                  "",                                            STD_CYCLIC,                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1153 */  { addItemToBuffer,              ITM_DIRECT_CURRENT,          "",                                            STD_DC,                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1154 */  { addItemToBuffer,              ITM_DOWN_DASHARROW,          "",                                            STD_DOWN_DASHARROW,                            (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1155 */  { addItemToBuffer,              ITM_EulerE,                  "",                                            STD_EulerE,                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1156 */  { addItemToBuffer,              ITM_INTEGER_Z,               "",                                            STD_INTEGER_Z,                                 (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1157 */  { addItemToBuffer,              ITM_LEFT_DASHARROW,          "",                                            STD_LEFT_DASHARROW,                            (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1158 */  { addItemToBuffer,              ITM_NOT_SUBSET_OF,           "",                                            STD_NOT_SUBSET_OF,                             (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1159 */  { addItemToBuffer,              ITM_op_i_char,               "",                                            STD_op_i,                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1160 */  { addItemToBuffer,              ITM_op_j_char,               "",                                            STD_op_j,                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1161 */  { addItemToBuffer,              ITM_POLAR_char,              "",                                            STD_SUN,                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1162 */  { addItemToBuffer,              ITM_DIA_ON,                  "",                                            STD_DIA_ON,                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1163 */  { addItemToBuffer,              ITM_RIGHT_DASHARROW,         "",                                            STD_RIGHT_DASHARROW,                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1164 */  { addItemToBuffer,              ITM_RIGHT_DOUBLE_ARROW,      "",                                            STD_RIGHT_DOUBLE_ARROW,                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1165 */  { addItemToBuffer,              ITM_RIGHT_TACK,              "",                                            STD_RIGHT_TACK,                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1166 */  { addItemToBuffer,              NOPARAM,                     "1166",                                        "1166",                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1167 */  { addItemToBuffer,              ITM_SUBSET_OF,               "",                                            STD_SUBSET_OF,                                 (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1168 */  { addItemToBuffer,              ITM_SUM_char,                "",                                            STD_SUM,                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1169 */  { addItemToBuffer,              ITM_UP_DASHARROW,            "",                                            STD_UP_DASHARROW,                              (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1170 */  { addItemToBuffer,              ITM_USB_SYMBOL,              "",                                            STD_USB_SYMBOL,                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1171 */  { addItemToBuffer,              ITM_LEFT_RIGHT_DOUBLE_ARROW, "",                                            STD_LEFT_RIGHT_DOUBLE_ARROW,                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1172 */  { addItemToBuffer,              ITM_CR,                      "",                                            STD_CR,                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1173 */  { addItemToBuffer,              ITM_TIME_T,                  "",                                            STD_TIME_T,                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1174 */  { addItemToBuffer,              ITM_DATE_D,                  "",                                            STD_DATE_D,                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },


// Reserved variables
/* 1175 */  { addItemToBuffer,              REGISTER_X,                  "X",                                           "X",                                           (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // Do
/* 1176 */  { addItemToBuffer,              REGISTER_Y,                  "Y",                                           "Y",                                           (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // not
/* 1177 */  { addItemToBuffer,              REGISTER_Z,                  "Z",                                           "Z",                                           (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // change
/* 1178 */  { addItemToBuffer,              REGISTER_T,                  "T",                                           "T",                                           (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // the
/* 1179 */  { addItemToBuffer,              REGISTER_A,                  "A",                                           "A",                                           (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // order
/* 1180 */  { addItemToBuffer,              REGISTER_B,                  "B",                                           "B",                                           (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // of
/* 1181 */  { addItemToBuffer,              REGISTER_C,                  "C",                                           "C",                                           (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // this
/* 1182 */  { addItemToBuffer,              REGISTER_D,                  "D",                                           "D",                                           (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // 3
/* 1183 */  { addItemToBuffer,              REGISTER_L,                  "L",                                           "L",                                           (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // 0
/* 1184 */  { addItemToBuffer,              REGISTER_I,                  "I",                                           "I",                                           (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // reserved
/* 1185 */  { addItemToBuffer,              REGISTER_J,                  "J",                                           "J",                                           (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // variables
/* 1186 */  { addItemToBuffer,              REGISTER_K,                  "K",                                           "K",                                           (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // !
/* 1187 */  { itemToBeCoded,                NOPARAM,                     "ADM",                                         "ADM",                                         (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1188 */  { itemToBeCoded,                NOPARAM,                     "D.MAX",                                       "D.MAX",                                       (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1189 */  { itemToBeCoded,                NOPARAM,                     "ISM",                                         "ISM",                                         (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1190 */  { itemToBeCoded,                NOPARAM,                     "REALDF",                                      "REALDF",                                      (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1191 */  { itemToBeCoded,                NOPARAM,                     "#DEC",                                        "#DEC",                                        (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1192 */  { fnIntegrate,                  RESERVED_VARIABLE_ACC,       "ACC",                                         "ACC",                                         (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1193 */  { fnIntegrate,                  RESERVED_VARIABLE_ULIM,      STD_UP_ARROW "Lim",                            STD_UP_ARROW "Lim",                            (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1194 */  { fnIntegrate,                  RESERVED_VARIABLE_LLIM,      STD_DOWN_ARROW "Lim",                          STD_DOWN_ARROW "Lim",                          (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1195 */  { fnTvmVar,                     RESERVED_VARIABLE_FV,        "FV",                                          "FV",                                          (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1196 */  { fnTvmVar,                     RESERVED_VARIABLE_IPONA,     "I/a",                                         "I/a",                                         (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1197 */  { fnTvmVar,                     RESERVED_VARIABLE_NPPER,     "n",                                           "n",                                           (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1198 */  { fnTvmVar,                     RESERVED_VARIABLE_PPERONA,   "pp/a",                                        "pp/a",                                        (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1199 */  { fnTvmVar,                     RESERVED_VARIABLE_PMT,       "PMT",                                         "PMT",                                         (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1200 */  { fnTvmVar,                     RESERVED_VARIABLE_PV,        "PV",                                          "PV",                                          (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1201 */  { itemToBeCoded,                NOPARAM,                     "GRAMOD",                                      "GRAMOD",                                      (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1202 */  { fnEditLinearEquationMatrixA,  NOPARAM,                     "Mat_A",                                       "Mat A",                                       (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1203 */  { fnEditLinearEquationMatrixB,  NOPARAM,                     "Mat_B",                                       "Mat B",                                       (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1204 */  { fnEditLinearEquationMatrixX,  NOPARAM,                     "Mat_X",                                       "Mat X",                                       (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1205 */  { fnStore,                      RESERVED_VARIABLE_UX,        STD_UP_ARROW "X",                              STD_UP_ARROW "X",                              (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1206 */  { fnStore,                      RESERVED_VARIABLE_LX,        STD_DOWN_ARROW "X",                            STD_DOWN_ARROW "X",                            (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },


// Probability distributions
/* 1207 */  { itemToBeCoded,                NOPARAM,                     "Binom:",                                      "Binom:",                                      (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1208 */  { fnBinomialP,                  NOPARAM,                     "Binom" STD_SUB_p,                             "Binom" STD_SUB_p,                             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1209 */  { fnBinomialL,                  NOPARAM,                     "Binom" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,   "Binom" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1210 */  { fnBinomialR,                  NOPARAM,                     "Binom" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,   "Binom" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1211 */  { fnBinomialI,                  NOPARAM,                     "Binom" STD_SUP_MINUS STD_SUP_1,               "Binom" STD_SUP_MINUS STD_SUP_1,               (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1212 */  { itemToBeCoded,                NOPARAM,                     "Cauch:",                                      "Cauch:",                                      (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1213 */  { fnCauchyP,                    NOPARAM,                     "Cauch" STD_SUB_p,                             "Cauch" STD_SUB_p,                             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1214 */  { fnCauchyL,                    NOPARAM,                     "Cauch" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,   "Cauch" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1215 */  { fnCauchyR,                    NOPARAM,                     "Cauch" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,   "Cauch" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1216 */  { fnCauchyI,                    NOPARAM,                     "Cauch" STD_SUP_MINUS STD_SUP_1,               "Cauch" STD_SUP_MINUS STD_SUP_1,               (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1217 */  { itemToBeCoded,                NOPARAM,                     "Expon:",                                      "Expon:",                                      (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1218 */  { fnExponentialP,               NOPARAM,                     "Expon" STD_SUB_p,                             "Expon" STD_SUB_p,                             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1219 */  { fnExponentialL,               NOPARAM,                     "Expon" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,   "Expon" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1220 */  { fnExponentialR,               NOPARAM,                     "Expon" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,   "Expon" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1221 */  { fnExponentialI,               NOPARAM,                     "Expon" STD_SUP_MINUS STD_SUP_1,               "Expon" STD_SUP_MINUS STD_SUP_1,               (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1222 */  { itemToBeCoded,                NOPARAM,                     "F:",                                          "F:",                                          (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1223 */  { fnF_P,                        NOPARAM,                     "F" STD_SUB_p "(x)",                           "F" STD_SUB_p "(x)",                           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1224 */  { fnF_L,                        NOPARAM,                     "F" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R "(x)", "F" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R "(x)", (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1225 */  { fnF_R,                        NOPARAM,                     "F" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R "(x)", "F" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R "(x)", (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1226 */  { fnF_I,                        NOPARAM,                     "F" STD_SUP_MINUS STD_SUP_1 "(p)",             "F" STD_SUP_MINUS STD_SUP_1 "(p)",             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1227 */  { itemToBeCoded,                NOPARAM,                     "Geom:",                                       "Geom:",                                       (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1228 */  { fnGeometricP,                 NOPARAM,                     "Geom" STD_SUB_p,                              "Geom" STD_SUB_p,                              (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1229 */  { fnGeometricL,                 NOPARAM,                     "Geom" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,    "Geom" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,    (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1230 */  { fnGeometricR,                 NOPARAM,                     "Geom" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,    "Geom" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,    (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1231 */  { fnGeometricI,                 NOPARAM,                     "Geom" STD_SUP_MINUS STD_SUP_1,                "Geom" STD_SUP_MINUS STD_SUP_1,                (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1232 */  { itemToBeCoded,                NOPARAM,                     "Hyper:",                                      "Hyper:",                                      (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1233 */  { fnHypergeometricP,            NOPARAM,                     "Hyper" STD_SUB_p,                             "Hyper" STD_SUB_p,                             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1234 */  { fnHypergeometricL,            NOPARAM,                     "Hyper" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,   "Hyper" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1235 */  { fnHypergeometricR,            NOPARAM,                     "Hyper" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,   "Hyper" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1236 */  { fnHypergeometricI,            NOPARAM,                     "Hyper" STD_SUP_MINUS STD_SUP_1,               "Hyper" STD_SUP_MINUS STD_SUP_1,               (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1237 */  { itemToBeCoded,                NOPARAM,                     "DISTR",                                       "DISTR",                                       (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1238 */  { fnLogNormalP,                 NOPARAM,                     "LgNrm" STD_SUB_p,                             "LgNrm" STD_SUB_p,                             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1239 */  { fnLogNormalL,                 NOPARAM,                     "LgNrm" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,   "LgNrm" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1240 */  { fnLogNormalR,                 NOPARAM,                     "LgNrm" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,   "LgNrm" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1241 */  { fnLogNormalI,                 NOPARAM,                     "LgNrm" STD_SUP_MINUS STD_SUP_1,               "LgNrm" STD_SUP_MINUS STD_SUP_1,               (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1242 */  { itemToBeCoded,                NOPARAM,                     "Logis:",                                      "Logis:",                                      (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1243 */  { fnLogisticP,                  NOPARAM,                     "Logis" STD_SUB_p,                             "Logis" STD_SUB_p,                             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1244 */  { fnLogisticL,                  NOPARAM,                     "Logis" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,   "Logis" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1245 */  { fnLogisticR,                  NOPARAM,                     "Logis" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,   "Logis" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1246 */  { fnLogisticI,                  NOPARAM,                     "Logis" STD_SUP_MINUS STD_SUP_1,               "Logis" STD_SUP_MINUS STD_SUP_1,               (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1247 */  { itemToBeCoded,                NOPARAM,                     "GEV:",                                        "GEV:",                                        (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1248 */  { fnNegBinomialP,               NOPARAM,                     "NBin" STD_SUB_p,                              "NBin" STD_SUB_p,                              (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1249 */  { fnNegBinomialL,               NOPARAM,                     "NBin" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,    "NBin" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,    (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1250 */  { fnNegBinomialR,               NOPARAM,                     "NBin" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,    "NBin" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,    (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1251 */  { fnNegBinomialI,               NOPARAM,                     "NBin" STD_SUP_MINUS STD_SUP_1,                "NBin" STD_SUP_MINUS STD_SUP_1,                (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1252 */  { itemToBeCoded,                NOPARAM,                     "Norml:",                                      "Norml:",                                      (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1253 */  { fnNormalP,                    NOPARAM,                     "Norml" STD_SUB_p,                             "Norml" STD_SUB_p,                             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1254 */  { fnNormalL,                    NOPARAM,                     "Norml" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,   "Norml" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1255 */  { fnNormalR,                    NOPARAM,                     "Norml" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,   "Norml" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1256 */  { fnNormalI,                    NOPARAM,                     "Norml" STD_SUP_MINUS STD_SUP_1,               "Norml" STD_SUP_MINUS STD_SUP_1,               (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1257 */  { itemToBeCoded,                NOPARAM,                     "Poiss:",                                      "Poiss:",                                      (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1258 */  { fnPoissonP,                   NOPARAM,                     "Poiss" STD_SUB_p,                             "Poiss" STD_SUB_p,                             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1259 */  { fnPoissonL,                   NOPARAM,                     "Poiss" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,   "Poiss" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1260 */  { fnPoissonR,                   NOPARAM,                     "Poiss" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,   "Poiss" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1261 */  { fnPoissonI,                   NOPARAM,                     "Poiss" STD_SUP_MINUS STD_SUP_1,               "Poiss" STD_SUP_MINUS STD_SUP_1,               (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1262 */  { itemToBeCoded,                NOPARAM,                     "t:",                                          "t:",                                          (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1263 */  { fnT_P,                        NOPARAM,                     "t" STD_SUB_p "(x)",                           "t" STD_SUB_p "(x)",                           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1264 */  { fnT_L,                        NOPARAM,                     "t" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R "(x)", "t" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R "(x)", (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1265 */  { fnT_R,                        NOPARAM,                     "t" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R "(x)", "t" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R "(x)", (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1266 */  { fnT_I,                        NOPARAM,                     "t" STD_SUP_MINUS STD_SUP_1 "(p)",             "t" STD_SUP_MINUS STD_SUP_1 "(p)",             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1267 */  { itemToBeCoded,                NOPARAM,                     "Weibl:",                                      "Weibl:",                                      (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1268 */  { fnWeibullP,                   NOPARAM,                     "Weibl" STD_SUB_p,                             "Weibl" STD_SUB_p,                             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1269 */  { fnWeibullL,                   NOPARAM,                     "Weibl" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,   "Weibl" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1270 */  { fnWeibullR,                   NOPARAM,                     "Weibl" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,   "Weibl" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1271 */  { fnWeibullI,                   NOPARAM,                     "Weibl" STD_SUP_MINUS STD_SUP_1,               "Weibl" STD_SUP_MINUS STD_SUP_1,               (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1272 */  { itemToBeCoded,                NOPARAM,                     STD_chi STD_SUP_2 ":",                         STD_chi STD_SUP_2 ":",                         (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1273 */  { fnChi2P,                      NOPARAM,                     STD_chi STD_SUP_2 STD_SUB_p "(x)",             STD_chi STD_SUP_2 STD_SUB_p "(x)",             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1274 */  { fnChi2L,                      NOPARAM,                     STD_chi STD_SUP_2 STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R "(x)", STD_chi STD_SUP_2 STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R "(x)", (0 << TAM_MAX_BITS) | 0, CAT_FNCT | SLS_ENABLED | US_ENABLED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1275 */  { fnChi2R,                      NOPARAM,                     STD_chi STD_SUP_2 STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R "(x)", STD_chi STD_SUP_2 STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R "(x)", (0 << TAM_MAX_BITS) | 0, CAT_FNCT | SLS_ENABLED | US_ENABLED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1276 */  { fnChi2I,                      NOPARAM,                     "(" STD_chi STD_SUP_2 ")" STD_SUP_MINUS STD_SUP_1, "(" STD_chi STD_SUP_2 ")" STD_SUP_MINUS STD_SUP_1, (0 << TAM_MAX_BITS) | 0, CAT_FNCT | SLS_ENABLED | US_ENABLED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1277 */  { itemToBeCoded,                NOPARAM,                     STD_PHI ":",                                   STD_PHI ":",                                   (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1278 */  { fnStdNormalP,                 NOPARAM,                     STD_phi_m STD_SUB_p,                           STD_phi_m STD_SUB_p,                           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1279 */  { fnStdNormalL,                 NOPARAM,                     STD_PHI STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,   STD_PHI STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1280 */  { fnStdNormalR,                 NOPARAM,                     STD_PHI STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,   STD_PHI STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1281 */  { fnStdNormalI,                 NOPARAM,                     STD_PHI STD_SUP_MINUS STD_SUP_1,               STD_PHI STD_SUP_MINUS STD_SUP_1,               (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1282 */  { fnGEVP,                       NOPARAM,                     "GEV" STD_SUB_p,                               "GEV" STD_SUB_p,                               (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1283 */  { fnGEVL,                       NOPARAM,                     "GEV" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,     "GEV" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1284 */  { fnGEVR,                       NOPARAM,                     "GEV" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,     "GEV" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1285 */  { fnGEVI,                       NOPARAM,                     "GEV" STD_SUP_MINUS STD_SUP_1,                 "GEV" STD_SUP_MINUS STD_SUP_1,                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1286 */  { itemToBeCoded,                NOPARAM,                     "Pareto:",                                     "Pareto:",                                     (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1287 */  { fnParetoP,                    NOPARAM,                     "Prto" STD_SUB_p,                              "Prto" STD_SUB_p,                              (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1288 */  { fnParetoL,                    NOPARAM,                     "Prto" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,    "Prto" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,    (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1289 */  { fnParetoU,                    NOPARAM,                     "Prto" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,    "Prto" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,    (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1290 */  { fnParetoI,                    NOPARAM,                     "Prto" STD_SUP_MINUS STD_SUP_1,                "Prto" STD_SUP_MINUS STD_SUP_1,                (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1291 */  { fnPareto2P,                   NOPARAM,                     "Prto2" STD_SUB_p,                             "Prto2" STD_SUB_p,                             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1292 */  { fnPareto2L,                   NOPARAM,                     "Prto2" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,   "Prto2" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1293 */  { fnPareto2U,                   NOPARAM,                     "Prto2" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,   "Prto2" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1294 */  { fnPareto2I,                   NOPARAM,                     "Prto2" STD_SUP_MINUS STD_SUP_1,               "Prto2" STD_SUP_MINUS STD_SUP_1,               (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1295 */  { itemToBeCoded,                NOPARAM,                     "1295",                                        "1295",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1296 */  { itemToBeCoded,                NOPARAM,                     "1296",                                        "1296",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },


// Curve fitting
/* 1297 */  { fnCurveFitting,               TM_VALUE,                    ">BestF<",                                     ">BestF<",                                     (0 << TAM_MAX_BITS) |   511, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_16    | HG_ENABLED         },
/* 1298 */  { fnCurveFitting_T,             CF_EXPONENTIAL_FITTING,      "ExpF",                                        "ExpF",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1299 */  { fnCurveFitting_T,             CF_LINEAR_FITTING,           "LinF",                                        "LinF",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1300 */  { fnCurveFitting_T,             CF_LOGARITHMIC_FITTING,      "LogF",                                        "LogF",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1301 */  { fnCurveFitting_T,             CF_ORTHOGONAL_FITTING,       "OrthoF",                                      "OrthoF",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1302 */  { fnCurveFitting_T,             CF_POWER_FITTING,            "PowerF",                                      "PowerF",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1303 */  { fnCurveFitting_T,             CF_GAUSS_FITTING,            "GaussF",                                      "GaussF",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1304 */  { fnCurveFitting_T,             CF_CAUCHY_FITTING,           "CauchF",                                      "CauchF",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1305 */  { fnCurveFitting_T,             CF_PARABOLIC_FITTING,        "ParabF",                                      "ParabF",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1306 */  { fnCurveFitting_T,             CF_HYPERBOLIC_FITTING,       "HypF",                                        "HypF",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1307 */  { fnCurveFitting_T,             CF_ROOT_FITTING,             "RootF",                                       "RootF",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1308 */  { fnCurveFittingReset,          1,                           "AllF",                                        "AllF",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1309 */  { fnCurveFittingReset,          0,                           "ResetF",                                      "ResetF",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1310 */  { fnCurveFitting,               TM_VALUE,                    "BestF",                                       "BestF",                                       (0 << TAM_MAX_BITS) |   511, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_16    | HG_ENABLED         },
/* 1311 */  { itemToBeCoded,                NOPARAM,                     "1311",                                        "1311",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1312 */  { itemToBeCoded,                NOPARAM,                     "1312",                                        "1312",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },


// Menus
/* 1313 */  { itemToBeCoded,                NOPARAM,                     "ADV",                                         "ADV",                                         (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1314 */  { itemToBeCoded,                NOPARAM,                     "ANGLES",                                      "ANGLES",                                      (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1315 */  { itemToBeCoded,                NOPARAM,                     "PRINT",                                       "PRINT",                                       (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1316 */  { itemToBeCoded,                NOPARAM/*#JM#*/,             "Area:",                                       "Area:",                                       (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1317 */  { itemToBeCoded,                NOPARAM/*#JM#*/,             "BITS",                                        "BITS",                                        (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1318 */  { itemToBeCoded,                NOPARAM/*#JM#*/,             "CATALOG",                                     "CAT",                                         (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },// JM
/* 1319 */  { itemToBeCoded,                NOPARAM,                     "CHARS",                                       "CHARS",                                       (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1320 */  { itemToBeCoded,                NOPARAM,                     "CLK",                                         "CLK",                                         (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1321 */  { itemToBeCoded,                NOPARAM,                     "CLR",                                         "CLR",                                         (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1322 */  { itemToBeCoded,                NOPARAM/*#JM#*/,             "CNST",                                        "CNST",                                        (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM Keeps the same. Don't havce space for more on kjeyplate
/* 1323 */  { itemToBeCoded,                NOPARAM,                     "CPX",                                         "CPX",                                         (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1324 */  { itemToBeCoded,                NOPARAM,                     "CPXS",                                        "CPXS",                                        (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1325 */  { itemToBeCoded,                NOPARAM,                     "DATES",                                       "DATES",                                       (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1326 */  { itemToBeCoded,                NOPARAM,                     "DISP",                                        "DISP",                                        (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1327 */  { itemToBeCoded,                NOPARAM,                     "EQN",                                         "EQN",                                         (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1328 */  { itemToBeCoded,                NOPARAM,                     "EXP",                                         "EXP",                                         (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1329 */  { itemToBeCoded,                NOPARAM/*#JM#*/,             "Energy:",                                     "Energy:",                                     (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1330 */  { itemToBeCoded,                NOPARAM,                     "FCNS",                                        "FCNS",                                        (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1331 */  { itemToBeCoded,                NOPARAM,                     "FIN",                                         "FIN",                                         (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1332 */  { itemToBeCoded,                NOPARAM,                     "S.INTS",                                      "S.INTS",                                      (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1333 */  { itemToBeCoded,                NOPARAM,                     "FLAG",                                        "FLAG",                                        (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1334 */  { fnBaseMenu,                   NOPARAM,                     "MyMenu",                                      "MyM",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1335 */  { itemToBeCoded,                NOPARAM,                     "f'",                                          "f'",                                          (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1336 */  { itemToBeCoded,                NOPARAM,                     "f\"",                                         "f\"",                                         (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1337 */  { itemToBeCoded,                NOPARAM,                     "F&p:",                                        "F&p:",                                        (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1338 */  { itemToBeCoded,                NOPARAM,                     "L.INTS",                                      "L.INTS",                                      (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1339 */  { itemToBeCoded,                NOPARAM,                     "INFO",                                        "INFO",                                        (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1340 */  { itemToBeCoded,                NOPARAM,                     "INTS",                                        "INTS",                                        (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1341 */  { itemToBeCoded,                NOPARAM,                     "I/O",                                         "I/O",                                         (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1342 */  { itemToBeCoded,                NOPARAM,                     "LOOP",                                        "LOOP",                                        (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1343 */  { itemToBeCoded,                NOPARAM,                     "MATRS",                                       "MATRS",                                       (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1344 */  { itemToBeCoded,                NOPARAM,                     "MATX",                                        "MATX",                                        (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1345 */  { itemToBeCoded,                NOPARAM,                     "MENUS",                                       "MENUS",                                       (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1346 */  { itemToBeCoded,                NOPARAM,                     "MODE",                                        "MODE",                                        (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1347 */  { itemToBeCoded,                NOPARAM,                     "M_SIMQ",                                      "M_SIMQ",                                      (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1348 */  { itemToBeCoded,                NOPARAM,                     "M_EDIT",                                      "M_EDIT",                                      (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1349 */  { itemToBeCoded,                NOPARAM,                     "MyMenu",                                      "MyM",                                         (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1350 */  { itemToBeCoded,                NOPARAM,                     "My" STD_alpha,                                "My" STD_alpha,                                (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1351 */  { itemToBeCoded,                NOPARAM,                     "Mass:",                                       "Mass:",                                       (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1352 */  { itemToBeCoded,                NOPARAM,                     "ORTHOG",                                      "Orthog",                                      (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1353 */  { itemToBeCoded,                NOPARAM,                     "REAL",                                        "REAL",                                        (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1354 */  { itemToBeCoded,                NOPARAM,                     "PROB",                                        "PROB",                                        (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1355 */  { itemToBeCoded,                NOPARAM,                     "PROGS",                                       "PROGS",                                       (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1356 */  { itemToBeCoded,                NOPARAM,                     "P.FN1",                                       "P.FN1",                                       (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1357 */  { itemToBeCoded,                NOPARAM,                     "P.FN2",                                       "P.FN2",                                       (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1358 */  { itemToBeCoded,                NOPARAM,                     "Power:",                                      "Power:",                                      (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1359 */  { itemToBeCoded,                NOPARAM,                     "FFF+:",                                       "FFF+:",                                       (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1360 */  { itemToBeCoded,                NOPARAM,                     "REALS",                                       "REALS",                                       (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1361 */  { itemToBeCoded,                NOPARAM,                     "f Solve",                                     "f Solve",                                     (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1362 */  { itemToBeCoded,                NOPARAM,                     "STAT",                                        "STAT",                                        (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1363 */  { itemToBeCoded,                NOPARAM,                     "STK",                                         "STK",                                         (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1364 */  { itemToBeCoded,                NOPARAM,                     "STRINGS",                                     "STRINGS",                                     (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1365 */  { itemToBeCoded,                NOPARAM,                     "TEST",                                        "TEST",                                        (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1366 */  { itemToBeCoded,                NOPARAM,                     "TIMES",                                       "TIMES",                                       (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1367 */  { itemToBeCoded,                NOPARAM,                     "TRIG",                                        "TRIG",                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM
/* 1368 */  { itemToBeCoded,                NOPARAM,                     "TVM",                                         "TVM",                                         (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1369 */  { itemToBeCoded,                NOPARAM,                     "CONV",                                        "CONV",                                        (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM Change U> arrow to CONV. Changed again to UNIT
/* 1370 */  { itemToBeCoded,                NOPARAM,                     "VARS",                                        "VARS",                                        (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1371 */  { itemToBeCoded,                NOPARAM,                     "Volume:",                                     "Volume:",                                     (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1372 */  { itemToBeCoded,                NOPARAM,                     "X.FN",                                        "X.FN",                                        (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1373 */  { itemToBeCoded,                NOPARAM,                     "Length:",                                     "Length:",                                     (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1374 */  { itemToBeCoded,                NOPARAM,                     STD_alpha "INTL",                              STD_alpha "INTL",                              (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1375 */  { itemToBeCoded,                NOPARAM,                     STD_alpha "Math",                              STD_alpha "Math",                              (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1376 */  { itemToBeCoded,                NOPARAM,                     STD_alpha ".FN",                               STD_alpha ".FN",                               (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM Changed a.FN to STRNG
/* 1377 */  { itemToBeCoded,                NOPARAM,                     STD_ALPHA STD_ELLIPSIS STD_OMEGA,              STD_ALPHA STD_ELLIPSIS STD_OMEGA,              (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // Upper case greek letters
/* 1378 */  { itemToBeCoded,                NOPARAM,                     STD_alpha "Misc",                              STD_alpha "Misc",                              (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // Upper case intl letters
/* 1379 */  { itemToBeCoded,                NOPARAM,                     "SYS.FL",                                      "SYS.FL",                                      (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1380 */  { itemToBeCoded,                NOPARAM,                     STD_INTEGRAL "f", /*G*/                        STD_INTEGRAL "f", /*H*/                        (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1381 */  { fnIntegrate,                  TM_LBLONLY,                  STD_INTEGRAL "f" STD_SPACE_4_PER_EM "d", /*J*/ STD_INTEGRAL "f" STD_SPACE_4_PER_EM "d",/*K*/  (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1382 */  { itemToBeCoded,                NOPARAM,                     STD_ANGLE "CONV",                              STD_ANGLE "CONV",                              (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM Change to text DRG and change again to CONV
/* 1383 */  { itemToBeCoded,                NOPARAM,                     STD_alpha STD_ELLIPSIS STD_omega,              STD_alpha STD_ELLIPSIS STD_omega,              (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // Lower case greek letters
/* 1384 */  { itemToBeCoded,                NOPARAM,                     STD_alpha "intl",                              STD_alpha "intl",                              (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // lower case intl letters
/* 1385 */  { itemToBeCoded,                NOPARAM,                     "",                                            "Tam",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1386 */  { itemToBeCoded,                NOPARAM,                     "",                                            "TamCmp",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1387 */  { itemToBeCoded,                NOPARAM,                     "",                                            "TamSto",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1388 */  { itemToBeCoded,                NOPARAM,                     "f Graph",                                     "f Graph",                                     (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1389 */  { itemToBeCoded,                NOPARAM,                     "VAR",                                         "VAR",                                         (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1390 */  { itemToBeCoded,                NOPARAM,                     "",                                            "TamFlag",                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1391 */  { itemToBeCoded,                NOPARAM,                     "",                                            "TamShuffle",                                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1392 */  { itemToBeCoded,                NOPARAM,                     "PROG",                                        "PROG",                                        (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1393 */  { itemToBeCoded,                NOPARAM,                     "",                                            "TamLabel",                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1394 */  { fnDynamicMenu,                NOPARAM,                     "",                                            "DYNMNU",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1395 */  { itemToBeCoded,                NOPARAM,                     "SCATR",                                       "SCATR",                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1396 */  { itemToBeCoded,                NOPARAM,                     "ASSESS",                                      "ASSESS",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1397 */  { itemToBeCoded,                NOPARAM,                     "ELLIPT",                                      "Ellipt",                                      (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1398 */  { itemToBeCoded,                NOPARAM,                     "MVAR",                                        "MVAR",                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // solver variables
/* 1399 */  { itemToBeCoded,                NOPARAM,                     "EQ_EDIT",                                     "EQ_EDIT",                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1400 */  { itemToBeCoded,                NOPARAM,                     "STOPW",                                       "STOPW",                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1401 */  { itemToBeCoded,                NOPARAM,                     "HIST",                                        "HIST",                                        (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1402 */  { itemToBeCoded,                NOPARAM,                     "HPLOT",                                       "HPLOT",                                       (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1403 */  { itemToBeCoded,                NOPARAM,                     "P.FN",                                        "P.FN",                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM PFN


/* 1404 */  { fnIntegerMode,                SIM_1COMPL,                  "1COMPL",                                      "1COMPL",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1405 */  { fnSNAP,                       NOPARAM,                     "SNAP",                                        "SNAP",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_DISABLED        },
/* 1406 */  { fnIntegerMode,                SIM_2COMPL,                  "2COMPL",                                      "2COMPL",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1407 */  { fnMagnitude,                  NOPARAM,                     ">ABS<",                                       ">ABS<",                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1408 */  { fnAgm,                        NOPARAM,                     "AGM",                                         "AGM",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1409 */  { fnAGraph,                     TM_REGISTER,                 "AGRAPH",                                      "AGRAPH",                                      (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 1410 */  { fnDisplayFormatAll,           TM_VALUE,                    "ALL" ,                                        "ALL",                                         (0 << TAM_MAX_BITS) |DSP_MAX,CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 1411 */  { fnAssign,                     0,                           "ASSIGN",                                      "ASN",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_DISABLED        },
/* 1412 */  { fnBack,                       TM_VALUE,                    "BACK",                                        "BACK",                                        (0 << TAM_MAX_BITS) |   255, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_SKIP_BACK    | HG_ENABLED         },
/* 1413 */  { fnBatteryVoltage,             NOPARAM,                     "BATT?",                                       "BATT?",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1414 */  { fnBeep,                       NOPARAM,                     "BEEP",                                        "BEEP",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1415 */  { fnTvmBeginMode,               NOPARAM,                     "BeginP",                                      "Begin",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1416 */  { fnBn,                         NOPARAM,                     "B" STD_SUB_n,                                 "B" STD_SUB_n,                                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1417 */  { fnBnStar,                     NOPARAM,                     "B" STD_SUB_n STD_SUP_ASTERISK,                "B" STD_SUB_n STD_SUP_ASTERISK,                (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1418 */  { fnCase,                       TM_REGISTER,                 "CASE",                                        "CASE",                                        (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 1419 */  { fnClAll,                      NOT_CONFIRMED,               "DELall",                                      "DELall",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABL_XEQ | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1420 */  { fnClCVar,                     NOPARAM,                     "CLCVAR",                                      "CLCVAR",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABL_XEQ | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1421 */  { fnClFAll,                     NOT_CONFIRMED,               "CLFALL",                                      "CLFall",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABL_XEQ | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1422 */  { fnFractionType,               NOPARAM,                     "a b/c",                                       "a b/c",                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1423 */  { fnClLcd,                      NOPARAM,                     "CLLCD",                                       "CLLCD",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABL_XEQ | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1424 */  { fnClearMenu,                  NOPARAM,                     "CLMENU",                                      "CLMENU",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1425 */  { fnClP,                        TM_LBLONLY,                  "DELP",                                        "DELP",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_CANCEL    | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1426 */  { fnClPAll,                     NOT_CONFIRMED,               "DELPALL",                                     "DELPall",                                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_CANCEL    | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1427 */  { fnClearRegisters,             NOT_CONFIRMED,               "CLREGS",                                      "CLREGS",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_CANCEL    | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1428 */  { fnClearStack,                 NOPARAM,                     "CLSTK",                                       "CLSTK",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABL_XEQ | EIM_DISABLED | PTP_NONE         | HG_DISABLED        },
/* 1429 */  { fnClSigma,                    NOPARAM,                     "CL" STD_SIGMA,                                "CL" STD_SIGMA,                                (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABL_XEQ | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1430 */  { fnStoreMax,                   TM_REGISTER,                 "STO" STD_UP_ARROW,                            "STO" STD_UP_ARROW,                            (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 1431 */  { fnConjugate,                  NOPARAM,                     "CONJ",                                        "conj",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1432 */  { fnRecallMax,                  TM_REGISTER,                 "RCL" STD_UP_ARROW,                            "RCL" STD_UP_ARROW,                            (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 1433 */  { fnCoefficientDetermination,   NOPARAM,                     "CORR",                                        "r",                                           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1434 */  { fnPopulationCovariance,       NOPARAM,                     "COV",                                         "cov",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1435 */  { fnCurveFittingLR,             NOPARAM,                     "BestF?",                                      "BestF?",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1436 */  { fnCross,                      NOPARAM,                     "CROSS",                                       "cross",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1437 */  { fnCxToRe,                     NOPARAM,                     STD_COMPLEX_C STD_RIGHT_ARROW STD_REAL_R,      STD_COMPLEX_C STD_RIGHT_ARROW STD_REAL_R,      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1438 */  { fnDate,                       NOPARAM,                     "Date" STD_RIGHT_ARROW DT,                     "Date" STD_RIGHT_ARROW DT,                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1439 */  { fnDateTo,                     NOPARAM,                     DT STD_RIGHT_ARROW "zyx",                      DT STD_RIGHT_ARROW "zyx",                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1440 */  { fnDay,                        NOPARAM,                     "DAY",                                         "DAY",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1441 */  { fnDblDivideRemainder,         NOPARAM,                     "DBLR",                                        "DBLR",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1442 */  { fnDblMultiply,                NOPARAM,                     "DBL" STD_CROSS,                               "DBL" STD_CROSS,                               (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1443 */  { fnDblDivide,                  NOPARAM,                     "DBL" STD_DIVIDE,                              "DBL" STD_DIVIDE,                              (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1444 */  { fnDecomp,                     NOPARAM,                     "DECOMP",                                      "DECOMP",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1445 */  { fnAngularMode,                amDegree,                    "DEG",                                         "DEG",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1446 */  { fnLoadedFile,                 NOPARAM,                     "STATE?",                                      "STATE?",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1447 */  { fnStatSa,                     NOPARAM,                     "s(a)",                                        "s(a)",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1448 */  { itemToBeCoded,                NOPARAM,                     "BLUE47",                                      "BLUE47",                                      (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1449 */  { fnDot,                        NOPARAM,                     "DOT",                                         "dot",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1450 */  { fnDisplayStack,               TM_VALUE,                    "dSTACK",                                      "dSTACK",                                      (1 << TAM_MAX_BITS) |     4, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 1451 */  { fnAngularMode,                amDMS,                       "D.MS",                                        "D.MS",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1452 */  { fnResetTVM,                   NOPARAM,                     "CLTVM",                                       "CLTVM",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABL_XEQ | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1453 */  { fnSetDateFormat,              ITM_DMY,                     "DMY",                                         "DMY",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1454 */  { fnDateToJulian,               NOPARAM,                     DT STD_RIGHT_ARROW "J",                        DT STD_RIGHT_ARROW "J",                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1455 */  { fnDeleteVariable,             TM_DELITM,                   "DELITM",                                      "DELITM",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_CANCEL    | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 1456 */  { fnEigenvalues,                NOPARAM,                     "EIGVAL",                                      "EIGVAL",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1457 */  { fnEigenvectors,               NOPARAM,                     "EIGVEC",                                      "EIGVEC",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1458 */  { fnReturn,                     0,                           "END",                                         "END",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1459 */  { fnTvmEndMode,                 NOPARAM,                     "ENDP",                                        "End",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1460 */  { fnDisplayFormatEng,           TM_VALUE,                    "ENG",                                         "ENG",                                         (0 << TAM_MAX_BITS) | DSP_MAX, CAT_FNCT|SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 1461 */  { fnEuclideanNorm,              NOPARAM,                     "ENORM",                                       "ENORM",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1462 */  { fnRecallMin,                  TM_REGISTER,                 "RCL" STD_DOWN_ARROW,                          "RCL" STD_DOWN_ARROW,                          (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 1463 */  { fnEqDelete,                   NOPARAM,                     "EQ.DEL",                                      "DELETE",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1464 */  { fnEqEdit,                     NOPARAM,                     "EQ.EDIT",                                     "EDIT",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1465 */  { fnEqNew,                      NOPARAM,                     "EQ.NEW",                                      "NEW",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1466 */  { fnErf,                        NOPARAM,                     "erf",                                         "erf",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1467 */  { fnErfc,                       NOPARAM,                     "erfc",                                        "erfc",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1468 */  { fnRaiseError,                 TM_VALUE,                    "ERR",                                         "ERR",                            (1 << TAM_MAX_BITS) | LAST_ERROR_MESSAGE, CAT_FNCT |SLS_ENABLED    | US_CANCEL    | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 1469 */  { fnExitAllMenus,               NOPARAM,                     "EXITALL",                                     "EXITall",                                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1470 */  { fnExpt,                       NOPARAM,                     "EXPT",                                        "EXPT",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1471 */  { fnGetFirstGregorianDay,       NOPARAM,                     "J/G?",                                        "J/G?",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1472 */  { fnFib,                        NOPARAM,                     "FIB",                                         "FIB",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1473 */  { fnDisplayFormatFix,           TM_VALUE,                    "FIX",                                         "FIX",                                         (0 << TAM_MAX_BITS) | DSP_MAX,CAT_FNCT| SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 1474 */  { fnDiskInfo,                   NOPARAM,                     "DISK?",                                       "DISK?",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1475 */  { fn1stDeriv,                   TM_LBLONLY,                  "f'(x)",                                       "f'(x)",                                       (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_LABEL        | HG_ENABLED         },
/* 1476 */  { fn2ndDeriv,                   TM_LBLONLY,                  "f\"(x)",                                      "f\"(x)",                                      (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_LABEL        | HG_ENABLED         },
/* 1477 */  { fnPrimeFactors,               NOPARAM,                     "FACTORS",                                     "FACTORS",                                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1478 */  { fnGd,                         NOPARAM,                     "g" STD_SUB_d,                                 "g" STD_SUB_d,                                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1479 */  { fnInvGd,                      NOPARAM,                     "g" STD_SUB_d STD_SUP_MINUS STD_SUP_1,         "g" STD_SUB_d STD_SUP_MINUS STD_SUP_1,         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1480 */  { fnAngularMode,                amGrad,                      "GRAD",                                        "GRAD",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1481 */  { fnGetFractionDigits,          NOPARAM,                     "FDIGS?",                                      "FDIGS?",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1482 */  { fnGotoDot,                    NOPARAM,                     "GTO.",                                        "GTO.",                                        (0 << TAM_MAX_BITS) | 16383, CAT_NONE | SLS_ENABLED   | US_CANCEL    | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1483 */  { fnHermite,                    NOPARAM,                     "H" STD_SUB_n,                                 "H" STD_SUB_n,                                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1484 */  { fnHermiteP,                   NOPARAM,                     "H" STD_SUB_n STD_SUB_P,                       "H" STD_SUB_n STD_SUB_P,                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1485 */  { fnImaginaryPart,              NOPARAM,                     "Im",                                          "Im",                                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1486 */  { fnIndexMatrix,                TM_REGISTER,                 "INDEX",                                       "INDEX",                                       (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_UNCHANGED | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 1487 */  { fnIxyz,                       NOPARAM,                     "I" STD_SUB_x STD_SUB_y STD_SUB_z,             "I" STD_SUB_x STD_SUB_y STD_SUB_z,             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1488 */  { fnGammaX,                     GAMMA_P,                     "I" STD_GAMMA STD_SUB_p,                       "I" STD_GAMMA STD_SUB_p,                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1489 */  { fnGammaX,                     GAMMA_Q,                     "I" STD_GAMMA STD_SUB_q,                       "I" STD_GAMMA STD_SUB_q,                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1490 */  { fnIncDecI,                    INC_FLAG,                    "I+",                                          "I+",                                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1491 */  { fnIncDecI,                    DEC_FLAG,                    "I-",                                          "I-",                                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1492 */  { fnBesselJ,                    NOPARAM,                     "J" STD_SUB_y "(x)",                           "J" STD_SUB_y "(x)",                           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1493 */  { fnIncDecJ,                    INC_FLAG,                    "J+",                                          "J+",                                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1494 */  { fnIncDecJ,                    DEC_FLAG,                    "J-",                                          "J-",                                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1495 */  { fnSetFirstGregorianDay,       NOPARAM,                     "J/G",                                         "J/G",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1496 */  { fnErrorMessage,               TM_VALUE,                    "MSG",                                         "MSG",                      (1 << TAM_MAX_BITS) | (NUMBER_OF_ERROR_CODES-1),CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 1497 */  { fnKeyGtoXeq,                  TM_VALUE,                    "KEY",                                         "KEY",                                         (1 << TAM_MAX_BITS) |    21, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_KEYG_KEYX    | HG_ENABLED         },
/* 1498 */  { fnKeyGto,                     TM_KEY,                      "KEYG",                                        "KEYG",                                        (1 << TAM_MAX_BITS) |    21, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1499 */  { fnKeyXeq,                     TM_KEY,                      "KEYX",                                        "KEYX",                                        (1 << TAM_MAX_BITS) |    21, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1500 */  { fnSinc,                       NOPARAM,                     "sinc",                                        "sinc",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1501 */  { fnKeyType,                    TM_REGISTER,                 "KTYP?",                                       "KTYP?",                                       (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 1502 */  { fnLastX,                      NOPARAM,                     "LASTx",                                       "LASTx",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_DISABLED        },
/* 1503 */  { fnCheckLabel,                 TM_LABEL,                    "LBL?",                                        "LBL?",                                        (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_LABEL        | HG_ENABLED         },
/* 1504 */  { fnIsLeap,                     NOPARAM,                     "LEAP?",                                       "LEAP?",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1505 */  { fnLaguerre,                   NOPARAM,                     "L" STD_SUB_m ,                                "L" STD_SUB_m ,                                (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1506 */  { fnLaguerreAlpha,              NOPARAM,                     "L" STD_SUB_m STD_SUB_alpha,                   "L" STD_SUB_m STD_SUB_alpha,                   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1507 */  { fnLnBeta,                     NOPARAM/*#JM#*/,             "LN" STD_beta,                                 "LN" STD_beta,                                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1508 */  { fnLnGamma,                    NOPARAM/*#JM#*/,             "LN" STD_GAMMA,                                "LN" STD_GAMMA,                                (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1509 */  { fnLoad,                       LM_ALL,                      "LOAD",                                        "LOAD",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_CANCEL    | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1510 */  { fnLoad,                       LM_PROGRAMS,                 "LOADP",                                       "LOADP",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1511 */  { fnLoad,                       LM_REGISTERS,                "LOADR",                                       "LOADR",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1512 */  { fnLoad,                       LM_SYSTEM_STATE,             "LOADSS",                                      "LOADSS",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1513 */  { fnLoad,                       LM_SUMS,                     "LOAD" STD_SIGMA,                              "LOAD" STD_SIGMA,                              (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1514 */  { allocateLocalRegisters,       TM_VALUE,                    "LocR",                                        "LocR",                                        (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 1515 */  { fnGetLocR,                    NOPARAM,                     "LocR?",                                       "LocR?",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1516 */  { fnProcessLR,                  7,                           "L.R.",                                        "L.R.",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1517 */  { fnMant,                       NOPARAM,                     "MANT",                                        "MANT",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1518 */  { fnEditLinearEquationMatrixX,  NOPARAM,                     "Mat_X",                                       "Mat X",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1519 */  { fnFreeMemory,                 NOPARAM,                     "MEM?",                                        "MEM?",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1520 */  { fnProgrammableMenu,           NOPARAM,                     "MENU",                                        "MENU",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1521 */  { fnMonth,                      NOPARAM,                     "MONTH",                                       "MONTH",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1522 */  { fnOldItemError,               NOPARAM,                     ">MSG<",                                       ">MSG<",                                       (0 << TAM_MAX_BITS) |    99, CAT_NONE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },//Old item
/* 1523 */  { fnAngularMode,                amMultPi,                    "MUL" STD_pi,                                  "MUL" STD_pi,                                  (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1524 */  { fnNop,                        TM_VARONLY,                  "MVAR",                                        "MVAR",                                        (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 1525 */  { fnDelRow,                     NOPARAM,                     "M.DELR",                                      "DELR",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1526 */  { fnSetMatrixDimensions,        TM_M_DIM,                    "M.DIM",                                       "DIM",                                         (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_UNCHANGED | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 1527 */  { fnGetMatrixDimensions,        NOPARAM,                     "M.DIM?",                                      "DIM?",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1528 */  { fnSetDateFormat,              ITM_MDY,                     "MDY",                                         "MDY",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1529 */  { fnEditMatrix,                 NOPARAM,                     "M.EDIT",                                      "EDIT",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1530 */  { fnEditMatrix,                 TM_REGISTER,                 "M.EDITN",                                     "EDITN",                                       (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_UNCHANGED | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1531 */  { fnGetMatrix,                  NOPARAM,                     "M.GETM",                                      "GETM",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1532 */  { fnGoToElement,                NOPARAM,                     "M.GOTO",                                      "GOTO",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1533 */  { fnSetGrowMode,                ON,                          "M.GROW",                                      "GROW",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1534 */  { fnInsRow,                     NOPARAM,                     "M.INSR",                                      "INSR",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1535 */  { fnLuDecomposition,            NOPARAM,                     "M.LU",                                        "M.LU",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1536 */  { fnNewMatrix,                  NOPARAM,                     "M.NEW",                                       "NEW",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1537 */  { fnOldMatrix,                  NOPARAM,                     "M.OLD",                                       "OLD",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1538 */  { fnPutMatrix,                  NOPARAM,                     "M.PUTM",                                      "PUTM",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1539 */  { fnSwapRows,                   NOPARAM,                     "M.R" STD_RIGHT_OVER_LEFT_ARROW "R",           "R" STD_RIGHT_OVER_LEFT_ARROW "R",             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1540 */  { fnSincpi,                     NOPARAM,                     "sinc" STD_pi,                                 "sinc" STD_pi,                                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1541 */  { fnSetGrowMode,                OFF,                         "M.WRAP",                                      "WRAP",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1542 */  { fnNop,                        NOPARAM,                     "NOP",                                         "NOP",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1543 */  { fnOff,                        NOPARAM,                     "OFF",                                         "OFF",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1544 */  { fnDropY,                      NOPARAM,                     "DROPy",                                       "DROPy",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1545 */  { fnStoreMin,                   TM_REGISTER,                 "STO" STD_DOWN_ARROW,                          "STO" STD_DOWN_ARROW,                          (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 1546 */  { fnPgmInt,                     TM_LBLONLY,                  "PGMINT",                                      "PGMINT",                                      (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_LABEL        | HG_ENABLED         },
/* 1547 */  { fnPgmSlv,                     TM_SOLVE,                    "PGMSLV",                                      "PGMSLV",                                      (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_LABEL        | HG_ENABLED         },
/* 1548 */  { fnPixel,                      NOPARAM,                     "PIXEL",                                       "PIXEL",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1549 */  { fnPlotStat,                   PLOT_START,                  "SCATR",                                       "SCATR",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1550 */  { fnLegendre,                   NOPARAM,                     "P" STD_SUB_n,                                 "P" STD_SUB_n,                                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1551 */  { fnPoint,                      NOPARAM,                     "POINT",                                       "POINT",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1552 */  { fnLoad,                       LM_NAMED_VARIABLES,          "LOADV",                                       "LOADV",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1553 */  { allocateLocalRegisters,       0,                           "PopLR",                                       "PopLR",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1554 */  { fnNop,                        NOPARAM,                     "REM",                                         "REM",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REM          | HG_ENABLED         },
/* 1555 */  { fnSlvc,                       NOPARAM,                     "SLVC",                                        "SLVC",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1556 */  { fnPutKey,                     TM_REGISTER,                 "PUTK",                                        "PUTK",                                        (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABL_XEQ | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 1557 */  { fnAngularMode,                amRadian,                    "RAD",                                         "RAD",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1558 */  { fnGetDmx,                     NOPARAM,                     "DMX?",                                        "DMX?",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1559 */  { fnRandom,                     NOPARAM,                     "RAN#",                                        "RAN#",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1560 */  { registerBrowser,              NOPARAM/*#JM#*/,             "REGS",                                        "REGS",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM Changed RBR to REGS
/* 1561 */  { fnRecallConfig,               TM_REGISTER,                 "RCLCFG",                                      "RCLCFG",                                      (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 1562 */  { fnRecallElement,              NOPARAM,                     "RCLEL",                                       "RCLEL",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1563 */  { fnRecallIJ,                   NOPARAM,                     "RCLIJ",                                       "RCLIJ",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1564 */  { fnRecallStack,                TM_REGISTER,                 "RCLS",                                        "RCLS",                                        (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 1565 */  { fnRdp,                        TM_VALUE,                    "RDP",                                         "RDP",                                         (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 1566 */  { fnRealPart,                   NOPARAM,                     "Re",                                          "Re",                                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1567 */  { fnLoadProgram,                NOPARAM,                     "READP",                                       "READP",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_CANCEL    | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1568 */  { fnReset,                      NOT_CONFIRMED,               "RESET",                                       "RESET",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABL_XEQ | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1569 */  { fnReToCx,                     NOPARAM,                     STD_REAL_R STD_RIGHT_ARROW STD_COMPLEX_C,      STD_REAL_R STD_RIGHT_ARROW STD_COMPLEX_C,      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1570 */  { fnSwapRealImaginary,          NOPARAM,                     "Re" STD_RIGHT_OVER_LEFT_ARROW "Im",           "Re" STD_RIGHT_OVER_LEFT_ARROW "Im",           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1571 */  { fnClP,                        TM_DELITM,                   "DELITM",                                      "DELITM",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_CANCEL    | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1572 */  { fnDeleteMenu,                 TM_DELITM,                   "DELITM",                                      "DELITM",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_CANCEL    | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1573 */  { fnDisplayFormatDsp,           TM_VALUE,                    "DSP",                                         "DSP",                                         (0 << TAM_MAX_BITS) |DSP_MAX,CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 1574 */  { fnRowNorm,                    NOPARAM,                     "RNORM",                                       "RNORM",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1575 */  { fnExpM1,                      NOPARAM,                     STD_EulerE STD_SUP_BOLD_x "-1",                STD_EulerE STD_SUP_BOLD_x "-1",                (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1576 */  { fnExportProgram,              TM_LBLONLY,                  "XPORTP",                                      "XPORTP",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_CANCEL    | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1577 */  { fnRsd,                        TM_VALUE,                    "RSD",                                         "RSD",                                         (1 << TAM_MAX_BITS) |    34, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 1578 */  { fnRowSum,                     NOPARAM,                     "RSUM",                                        "RSUM",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1579 */  { fnReturn,                     1,                           "RTN.SKP",                                     "RTN.SKP",                                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1580 */  { fnRegClr,                     NOPARAM,                     "R-CLR",                                       "R-CLR",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1581 */  { fnRegCopy,                    NOPARAM,                     "R-COPY",                                      "R-COPY",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1582 */  { fnRegSort,                    NOPARAM,                     "R-SORT",                                      "R-SORT",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1583 */  { fnRegSwap,                    NOPARAM,                     "R-SWAP",                                      "R-SWAP",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1584 */  { fnJacobiAmplitude,            NOPARAM,                     STD_psi "(u,m)",                               STD_psi "(u,m)",                               (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1585 */  { fnSampleStdDev,               NOPARAM,                     "s",                                           "s",                                           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1586 */  { fnSave,                       SM_MANUAL_SAVE,              "SAVE",                                        "SAVE",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1587 */  { fnDisplayFormatSci,           TM_VALUE,                    "SCI",                                         "SCI",                                         (0 << TAM_MAX_BITS) |DSP_MAX,CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 1588 */  { fnGetSignificantDigits,       NOPARAM,                     "SDIGS?",                                      "SDIGS?",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1589 */  { fnSeed,                       NOPARAM,                     "SEED",                                        "SEED",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1590 */  { fnSaveProgram,                TM_LBLONLY,                  "WRITEP",                                      "WRITEP",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_CANCEL    | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1591 */  { configCommon,                 CFG_CHINA,                   "S.CHN",                                       "CHINA",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1592 */  { fnSetDate,                    NOPARAM,                     "SETDAT",                                      "SETDAT",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1593 */  { configCommon,                 CFG_EUROPE,                  "S.EUR",                                       "EUROP",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1594 */  { configCommon,                 CFG_INDIA,                   "S.IND",                                       "INDIA",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1595 */  { configCommon,                 CFG_JAPAN,                   "S.JPN",                                       "JAPAN",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1596 */  { configCommon,                 CFG_DFLT,                    "S.DFLT",                                      "DEFLT",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1597 */  { fnSetTime,                    NOPARAM,                     "SETTIM",                                      "SETTIM",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1598 */  { configCommon,                 CFG_UK,                      "S.UK",                                        "UK",                                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1599 */  { configCommon,                 CFG_USA,                     "S.USA",                                       "USA",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1600 */  { fnSign,                       NOPARAM,                     "sign",                                        "sign",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1601 */  { fnIntegerMode,                SIM_SIGNMT,                  "SIGNMT",                                      "SIGNMT",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1602 */  { fnSimultaneousLinearEquation, TM_VALUE,                    "SIM_EQ",                                      "SIM EQ",                                      (1 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 1603 */  { fnSkip,                       TM_VALUE,                    "SKIP",                                        "SKIP",                                        (0 << TAM_MAX_BITS) |   255, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_SKIP_BACK    | HG_ENABLED         },
/* 1604 */  { fnSlvq,                       NOPARAM,                     "SLVQ",                                        "SLVQ",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1605 */  { fnStandardError,              NOPARAM,                     "s" STD_SUB_m,                                 "s" STD_SUB_m,                                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1606 */  { fnGetIntegerSignMode,         NOPARAM,                     "ISM?",                                        "ISM?",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1607 */  { fnWeightedStandardError,      NOPARAM,                     "s" STD_SUB_m STD_SUB_w,                       "s" STD_SUB_m STD_SUB_w,                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1608 */  { fnSolve,                      TM_SOLVE,                    "SOLVE",                                       "SOLVE",                                       (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 1609 */  { fnGetStackSize,               NOPARAM,                     "SSIZE?",                                      "SSIZE?",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1610 */  { flagBrowser,                  STATUS_SCREEN,               "STATUS",                                      "STATUS",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1611 */  { fnStoreConfig,                TM_REGISTER,                 "STOCFG",                                      "STOCFG",                                      (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 1612 */  { fnStoreElement,               NOPARAM,                     "STOEL",                                       "STOEL",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1613 */  { fnStoreIJ,                    NOPARAM,                     "STOIJ",                                       "STOIJ",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1614 */  { fnLnP1,                       NOPARAM,                     "LN(1+x)",                                     "LN(1+x)",                                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1615 */  { fnStoreStack,                 TM_REGISTER,                 "STOS",                                        "STOS",                                        (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 1616 */  { fnSumXY,                      NOPARAM,                     "x" STD_SUB_S STD_SUB_U STD_SUB_M,             "x" STD_SUB_S STD_SUB_U STD_SUB_M,             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1617 */  { fnWeightedSampleStdDev,       NOPARAM,                     "s" STD_SUB_w,                                 "s" STD_SUB_w,                                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1618 */  { fnSampleCovariance,           NOPARAM,                     "s" STD_SUB_x STD_SUB_y,                       "s" STD_SUB_x STD_SUB_y,                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1619 */  { fnDisplayFormatTime,          TM_VALUE,                    "TDISP",                                       "TDISP",                                       (0 << TAM_MAX_BITS) |     6, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 1620 */  { fnTicks,                      NOPARAM,                     "TICKS",                                       "TICKS",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1621 */  { fnTime,                       NOPARAM,                     "Time" STD_RIGHT_ARROW TM,                     "Time" STD_RIGHT_ARROW TM,                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1622 */  { fnItemTimerApp,               NOPARAM/*#JM#*/,             "STOPW",                                       "STOPW",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1623 */  { fnChebyshevT,                 NOPARAM,                     "T" STD_SUB_n,                                 "T" STD_SUB_n,                                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1624 */  { fnTone,                       TM_VALUE,                    "TONE",                                        "TONE",                                        (0 << TAM_MAX_BITS) |     9, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 1625 */  { fnSwapT,                      TM_REGISTER,                 "t" STD_RIGHT_OVER_LEFT_ARROW,                 "t" STD_RIGHT_OVER_LEFT_ARROW,                 (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 1626 */  { fnUlp,                        NOPARAM,                     "ULP?",                                        "ULP?",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1627 */  { fnChebyshevU,                 NOPARAM,                     "U" STD_SUB_n,                                 "U" STD_SUB_n,                                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1628 */  { fnUnitVector,                 NOPARAM,                     "UNITV",                                       "UNITV",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1629 */  { fnIntegerMode,                SIM_UNSIGN,                  "UNSIGN",                                      "UNSIGN",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1630 */  { fnVarMnu,                     TM_LBLONLY,                  "VARMNU",                                      "VarMNU",                                      (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_LABEL        | HG_ENABLED         },
/* 1631 */  { fnVersion,                    NOPARAM,                     "VERS?",                                       "VERS?",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1632 */  { fnIDivR,                      NOPARAM,                     "IDIVR",                                       "IDIVR",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1633 */  { fnWday,                       NOPARAM,                     "WDAY",                                        "WDAY",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1634 */  { fnWho,                        NOPARAM,                     "WHO?",                                        "WHO?",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1635 */  { fnWnegative,                  NOPARAM,                     "W" STD_SUB_m,                                 "W" STD_SUB_m,                                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1636 */  { fnWpositive,                  NOPARAM,                     "W" STD_SUB_p,                                 "W" STD_SUB_p,                                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1637 */  { fnWinverse,                   NOPARAM,                     "W" STD_SUP_MINUS STD_SUP_1,                   "W" STD_SUP_MINUS STD_SUP_1,                   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1638 */  { fnSetWordSize,                TM_VALUE,                    "WSIZE",                                       "WSIZE",                                       (0 << TAM_MAX_BITS) |    64, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 1639 */  { fnGetWordSize,                NOPARAM,                     "WSIZE?",                                      "WSIZE?",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1640 */  { fnMeanXY,                     NOPARAM,                     STD_x_BAR,                                     STD_x_BAR,                                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1641 */  { fnGeometricMeanXY,            NOPARAM,                     STD_x_BAR STD_SUB_G,                           STD_x_BAR STD_SUB_G,                           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1642 */  { fnWeightedMeanX,              NOPARAM,                     STD_x_BAR STD_SUB_w,                           STD_x_BAR STD_SUB_w,                           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1643 */  { fnXIsFny,                     NOPARAM,                     STD_x_CIRC,                                    STD_x_CIRC,                                    (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1644 */  { fnXToDate,                    NOPARAM,                     "x" STD_RIGHT_ARROW DT,                        "x" STD_RIGHT_ARROW DT,                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1645 */  { fnXToAlpha,                   NOPARAM,                     "x" STD_RIGHT_ARROW STD_alpha,                 "x" STD_RIGHT_ARROW STD_alpha,                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1646 */  { fnQrDecomposition,            NOPARAM,                     "M.QR",                                        "M.QR",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1647 */  { fnYear,                       NOPARAM,                     "YEAR",                                        "YEAR",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1648 */  { fnYIsFnx,                     NOPARAM,                     STD_y_CIRC,                                    STD_y_CIRC,                                    (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1649 */  { fnSetDateFormat,              ITM_YMD,                     "YMD",                                         "YMD",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1650 */  { fnSwapY,                      TM_REGISTER,                 "y" STD_RIGHT_OVER_LEFT_ARROW,                 "y" STD_RIGHT_OVER_LEFT_ARROW,                 (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 1651 */  { fnSwapZ,                      TM_REGISTER,                 "z" STD_RIGHT_OVER_LEFT_ARROW,                 "z" STD_RIGHT_OVER_LEFT_ARROW,                 (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 1652 */  { fnAlphaLeng,                  TM_REGISTER,                 STD_alpha "LENG?",                             STD_alpha "LENG?",                             (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 1653 */  { fnXmax,                       NOPARAM,                     "x" STD_SUB_M STD_SUB_A STD_SUB_X,             "x" STD_SUB_M STD_SUB_A STD_SUB_X,             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1654 */  { fnXmin,                       NOPARAM,                     "x" STD_SUB_M STD_SUB_I STD_SUB_N,             "x" STD_SUB_M STD_SUB_I STD_SUB_N,             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1655 */  { fnAlphaPos,                   TM_REGISTER,                 STD_alpha "POS?",                              STD_alpha "POS?",                              (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 1656 */  { fnAlphaRL,                    TM_REGISTER,                 STD_alpha "RL",                                STD_alpha "RL",                                (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 1657 */  { fnAlphaRR,                    TM_REGISTER,                 STD_alpha "RR",                                STD_alpha "RR",                                (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 1658 */  { fnAlphaSL,                    TM_REGISTER,                 STD_alpha "SL",                                STD_alpha "SL",                                (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 1659 */  { fnAlphaSR,                    TM_REGISTER,                 STD_alpha "SR",                                STD_alpha "SR",                                (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 1660 */  { fnAlphaToX,                   TM_REGISTER,                 STD_alpha STD_RIGHT_ARROW "x",                 STD_alpha STD_RIGHT_ARROW "x",                 (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 1661 */  { fnBeta,                       NOPARAM,                     STD_beta "(x,y)",                              STD_beta "(x,y)",                              (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1662 */  { fnGammaX,                     GAMMA_XYLOWER,               STD_gamma STD_SUB_x STD_SUB_y,                 STD_gamma STD_SUB_x STD_SUB_y,                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1663 */  { fnGammaX,                     GAMMA_XYUPPER,               STD_GAMMA STD_SUB_x STD_SUB_y,                 STD_GAMMA STD_SUB_x STD_SUB_y,                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1664 */  { fnGamma,                      NOPARAM,                     STD_GAMMA "(x)",                               STD_GAMMA "(x)",                               (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1665 */  { fnBesselY,                    NOPARAM,                     "Y" STD_SUB_y "(x)",                           "Y" STD_SUB_y "(x)",                           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1666 */  { fnDeltaPercent,               NOPARAM,                     STD_DELTA "%",                                 STD_DELTA "%",                                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1667 */  { fnGeometricSampleStdDev,      NOPARAM,                     STD_epsilon,                                   STD_epsilon,                                   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1668 */  { fnGeometricStandardError,     NOPARAM,                     STD_epsilon STD_SUB_m,                         STD_epsilon STD_SUB_m,                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1669 */  { fnGeometricPopulationStdDev,  NOPARAM,                     STD_epsilon STD_SUB_p,                         STD_epsilon STD_SUB_p,                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1670 */  { fnZeta,                       NOPARAM,                     STD_zeta "(x)",                                STD_zeta "(x)",                                (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1671 */  { fnProgrammableProduct,        TM_LBLONLY,                  STD_PRODUCT STD_SUB_n,                         STD_PRODUCT STD_SUB_n,                         (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_LABEL        | HG_ENABLED         },
/* 1672 */  { fnProgrammableSum,            TM_LBLONLY,                  STD_SUM STD_SUB_n,                             STD_SUM STD_SUB_n,                             (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_LABEL        | HG_ENABLED         },
/* 1673 */  { fnPopulationStdDev,           NOPARAM,                     STD_sigma,                                     STD_sigma,                                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1674 */  { fnWeightedPopulationStdDev,   NOPARAM,                     STD_sigma STD_SUB_w,                           STD_sigma STD_SUB_w,                           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1675 */  { fnRandomI,                    NOPARAM,                     "RANI#",                                       "RANI#",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1676 */  { fnP_All_Regs,                  PRN_Xr,                     STD_PRINTER "x",                               STD_PRINTER "x",                               (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1677 */  { fnLastT,                      NOPARAM,                     "LASTT?",                                      "LASTT?",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1678 */  { fnGetRange,                   NOPARAM,                     "RANGE?",                                      "RANGE?",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1679 */  { fnM1Pow,                      NOPARAM,                     "(-1)" STD_SUP_BOLD_x,                         "(-1)" STD_SUP_BOLD_x,                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1680 */  { fnMulMod,                     NOPARAM,                     STD_CROSS "MOD",                               STD_CROSS "MOD",                               (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1681 */  { fnToDate,                     NOPARAM,                     "zyx" STD_RIGHT_ARROW DT,                      "zyx" STD_RIGHT_ARROW DT,                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1682 */  { fnJacobiSn,                   NOPARAM,                     "sn(u,m)",                                     "sn(u,m)",                                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1683 */  { fnJacobiCn,                   NOPARAM,                     "cn(u,m)",                                     "cn(u,m)",                                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1684 */  { fnJacobiDn,                   NOPARAM,                     "dn(u,m)",                                     "dn(u,m)",                                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1685 */  { fnOldItemError,               NOPARAM,                     ">" STD_RIGHT_ARROW "HR<",                     ">" STD_RIGHT_ARROW "HR<",                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1686 */  { fnHRtoTM,                     NOPARAM,                     "H" STD_RIGHT_ARROW TM,                        "H" STD_RIGHT_ARROW TM,                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM mod
/* 1687 */  { fnChangeBase,                 TM_VALUE_CHB,                STD_RIGHT_ARROW "INT",                         "#",                                           (2 << TAM_MAX_BITS) |    16, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 1688 */  { fnHMStoTM,                    NOPARAM,                     "H.MS" STD_RIGHT_ARROW TM,                     "H.MS" STD_RIGHT_ARROW TM,                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1689 */  { fnSetBaseNr,                  TM_VALUE,                    "dBASE",                                       "dBASE",                                       (0 << TAM_MAX_BITS) |    62, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 1690 */  { fnIntegrateYX,                TM_REGISTER,                 STD_INTEGRAL STD_YX, /*A*/                     STD_INTEGRAL STD_YX, /*B*/                     (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 1691 */  { fnToReal,                     NOPARAM,                     STD_RIGHT_ARROW "REAL",                        ".d",                                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1692 */  { fnPcSigmaDeltaPcXmean,        NOPARAM,                     "%" STD_SIGMA "," STD_DELTA "%" STD_x_BAR STD_SUB_1, "%" STD_SIGMA "," STD_DELTA "%" STD_x_BAR STD_SUB_1, (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1693 */  { fnDeltaPercentXmean,          NOPARAM,                     STD_DELTA "%" STD_x_BAR STD_SUB_1,             STD_DELTA "%" STD_x_BAR STD_SUB_1,             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1694 */  { fnShuffle,                    TM_SHUFFLE,                  STD_RIGHT_OVER_LEFT_ARROW,                     STD_RIGHT_OVER_LEFT_ARROW,                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_SHUFFLE      | HG_ENABLED         },
/* 1695 */  { fnPercent,                    NOPARAM,                     "%",                                           "%",                                           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1696 */  { fnPercentMRR,                 NOPARAM,                     "%MRR",                                        "%MRR",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1697 */  { fnPercentT,                   NOPARAM,                     "%T",                                          "%T",                                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1698 */  { fnPercentSigma,               NOPARAM,                     "%" STD_SIGMA,                                 "%" STD_SIGMA,                                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1699 */  { fnPercentPlusMG,              NOPARAM,                     "%+MG",                                        "%+MG",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1700 */  { fnIntegrate,                  TM_REGISTER,                 STD_INTEGRAL "f" STD_SPACE_4_PER_EM "d", /*C*/ STD_INTEGRAL, /*D*/                            (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 1701 */  { fnExpMod,                     NOPARAM,                     "^MOD",                                        "^MOD",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1702 */  { fnDeterminant,                NOPARAM,                     "|M|",                                         "|M|",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1703 */  { fnParallel,                   NOPARAM,                     STD_PARALLEL,                                  STD_PARALLEL,                                  (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM
/* 1704 */  { fnTranspose,                  NOPARAM,                     "[M]" STD_TRANSPOSED,                          "[M]" STD_TRANSPOSED,                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1705 */  { fnInvertMatrix,               NOPARAM,                     "[M]" STD_SUP_MINUS STD_SUP_1,                 "[M]" STD_SUP_MINUS STD_SUP_1,                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1706 */  { fnArg,                        NOPARAM/*#JM#*/,             STD_MEASURED_ANGLE,                            STD_MEASURED_ANGLE,                            (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1707 */  { fnP_All_Regs,                 PRN_XYr,                     STD_PRINTER "xy",                               STD_PRINTER "xy",                             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1708 */  { itemToBeCoded,                NOPARAM,                     STD_PRINTER "ADV",                             STD_PRINTER "ADV",                             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1709 */  { itemToBeCoded,                NOPARAM,                     STD_PRINTER "CHAR",                            STD_PRINTER "CHAR",                            (0 << TAM_MAX_BITS) |   127, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 1710 */  { itemToBeCoded,                NOPARAM,                     STD_PRINTER "DLAY",                            STD_PRINTER "DLAY",                            (0 << TAM_MAX_BITS) |   127, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 1711 */  { itemToBeCoded,                NOPARAM,                     STD_PRINTER "LCD",                             STD_PRINTER "LCD",                             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1712 */  { itemToBeCoded,                NOPARAM,                     STD_PRINTER "MODE",                            STD_PRINTER "MODE",                            (0 << TAM_MAX_BITS) |     3, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 1713 */  { fnExportProgram,              TM_LBLONLY,                  STD_PRINTER "PROG",                            STD_PRINTER "PROG",                            (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_CANCEL    | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1714 */  { fnP_Regs,                     TM_REGISTER,                 STD_PRINTER "r",                               STD_PRINTER "r",                               (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 1715 */  { itemToBeCoded,                NOPARAM,                     STD_PRINTER "REGS",                            STD_PRINTER "REGS",                            (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1716 */  { fnP_All_Regs,                 PRN_STK,                     STD_PRINTER "STK",                             STD_PRINTER "STK",                             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1717 */  { itemToBeCoded,                NOPARAM,                     STD_PRINTER "TAB",                             STD_PRINTER "TAB",                             (0 << TAM_MAX_BITS) |   127, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 1718 */  { itemToBeCoded,                NOPARAM,                     STD_PRINTER "USER",                            STD_PRINTER "USER",                            (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1719 */  { itemToBeCoded,                NOPARAM,                     STD_PRINTER "WIDTH",                           STD_PRINTER "WIDTH",                           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1720 */  { itemToBeCoded,                NOPARAM,                     STD_PRINTER STD_SIGMA,                         STD_PRINTER STD_SIGMA,                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1721 */  { itemToBeCoded,                NOPARAM,                     STD_PRINTER "#",                               STD_PRINTER "#",                               (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1722 */  { fontBrowser,                  NOPARAM,                     "FBR",                                         "FBR",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // Font browser
/* 1723 */  { fnUndo,                       NOPARAM,                     "UNDO",                                        STD_UNDO,                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1724 */  { fnPem,                        NOPARAM/*#JM#*/,             "PRGM",                                        "PRGM",                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_CANCEL    | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM Change P/R to PRGM
/* 1725 */  { fnRunProgram,                 NOPARAM,                     "R/S",                                         "R/S",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1726 */  { fnEllipticK,                  NOPARAM,                     "K(m)",                                        "K(m)",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1727 */  { fnEllipticE,                  NOPARAM,                     "E(m)",                                        "E(m)",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1728 */  { fnEllipticPi,                 NOPARAM,                     STD_PI "(n,m)",                                STD_PI "(n,m)",                                (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1729 */  { fnFlipFlag,                   FLAG_USER,                   "USER",                                        "USER",                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_DISABLED        },
/* 1730 */  { fnKeyCC,                      NOPARAM,                     "CC",                                          "CC",                                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1731 */  { fnSHIFTf /*JM*/,              NOPARAM,                     "f",                                           "f",                                           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1732 */  { fnSHIFTg /*JM*/,              NOPARAM,                     "g",                                           "g",                                           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1733 */  { fnKeyUp,                      NOPARAM,                     "UP",                                          STD_UP_ARROW,                                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_DISABLED        },
/* 1734 */  { fnBst,                        NOPARAM,                     "BST",                                         "BST",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1735 */  { fnKeyDown,                    NOPARAM,                     "DOWN",                                        STD_DOWN_ARROW,                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_DISABLED        },
/* 1736 */  { fnSst,                        NOPARAM,                     "SST",                                         "SST"                ,                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1737 */  { fnKeyExit,                    NOPARAM,                     "EXIT",                                        "EXIT",                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_DISABLED        },
/* 1738 */  { fnKeyBackspace,               NOPARAM/*#JM#*/,             "BKSPC",                                       STD_LEFT_ARROW,                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_DISABLED        },
/* 1739 */  { fnSetMatrixDimensionsGr,      TM_M_DIM,                    "M.DIMgr",                                     "DIMgr",                                       (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_UNCHANGED | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 1740 */  { fnAim,                        NOPARAM,                     STD_alpha,                                     STD_alpha,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_DISABLED        },
/* 1741 */  { fnKeyDotD,                    NOPARAM,                     ".d",                                          ".d",                                          (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1742 */  { fnC47Show,                    NOPARAM,                     "SHOW",                                        "SHOW",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1743 */  { fnMeanX,                      NOPARAM,                     STD_x_BAR STD_SUB_1,                           STD_x_BAR STD_SUB_1,                           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1744 */  { SetSetting,                   FLAG_FRACT,                  "FRACT",                                       "FRACT",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1745 */  { fnVectorAngle,                NOPARAM,                     STD_MEASURED_ANGLE STD_SPACE_6_PER_EM STD_v_BAR "," STD_SPACE_6_PER_EM STD_v_BAR, STD_MEASURED_ANGLE STD_SPACE_6_PER_EM STD_v_BAR "," STD_SPACE_6_PER_EM STD_v_BAR, (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1746 */  { fnHarmonicMeanXY,             NOPARAM,                     STD_x_BAR STD_SUB_H,                           STD_x_BAR STD_SUB_H,                           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1747 */  { fnRMSMeanXY,                  NOPARAM,                     STD_x_BAR STD_SUB_R STD_SUB_M STD_SUB_S,       STD_x_BAR STD_SUB_R STD_SUB_M STD_SUB_S,       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1748 */  { fnArccos,                     NOPARAM,                     ">ACOS<",                                      ">ACOS<",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1749 */  { fnArcsin,                     NOPARAM,                     ">ASIN<",                                      ">ASIN<",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1750 */  { fnArctan,                     NOPARAM,                     ">ATAN<",                                      ">ATAN<",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1751 */  { fnDeterminant,                NOPARAM,                     ">DET<",                                       ">DET<",                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1752 */  { fnInvertMatrix,               NOPARAM,                     ">INVRT<",                                     ">INVRT<",                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1753 */  { fnTranspose,                  NOPARAM,                     ">TRANS<",                                     ">TRANS<",                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1754 */  { fnProgrammableiProduct,       TM_LBLONLY,                  "i" STD_PRODUCT STD_SUB_n,                     "i" STD_PRODUCT STD_SUB_n,                     (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_LABEL        | HG_ENABLED         },
/* 1755 */  { fnProgrammableiSum,           TM_LBLONLY,                  "i" STD_SUM STD_SUB_n,                         "i" STD_SUM STD_SUB_n,                         (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_LABEL        | HG_ENABLED         },
/* 1756 */  { fnPlotStat,                   PLOT_ORTHOF,                 "CENTRL",                                      "CENTRL",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1757 */  { fnOldItemError,               NOPARAM,                     ">HIDE<",                                      ">HIDE<",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//Old item
/* 1758 */  { fnPlotCloseSmi,               NOPARAM,                     "s" STD_SUB_m STD_SUB_i,                       "s" STD_SUB_m STD_SUB_i,                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1759 */  { fnPlotStat,                   PLOT_LR,                     "ASSESS",                                      "ASSESS",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1760 */  { fnPlotStat,                   PLOT_NXT,                    "NXTFIT",                                      "NXTFIT",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1761 */  { fnPlotStat,                   PLOT_REV,                    "",                                            "",                                            (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1762 */  { fnPlotZoom,                   NOPARAM,                     "ZOOM",                                        "ZOOM",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1763 */  { fnEllipticFphi,               NOPARAM,                     "F(" STD_phi_m ",m)",                          "F(" STD_phi_m ",m)",                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1764 */  { fnEllipticEphi,               NOPARAM,                     "E(" STD_phi_m ",m)",                          "E(" STD_phi_m ",m)",                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1765 */  { fnJacobiZeta,                 NOPARAM,                     STD_ZETA "(" STD_phi_m ",m)",                  STD_ZETA "(" STD_phi_m ",m)",                  (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1766 */  { fnGetHide,                    NOPARAM,                     "HIDE?",                                       "HIDE?",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1767 */  { fnEqCalc,                     NOPARAM,                     "Calc f",                                      "Calc f",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1768 */  { fnSquareRoot,                 NOPARAM,                     ">SQRT<",                                      ">SQRT<",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1769 */  { fnOldItemError,               NOPARAM,                     ">FV<",                                        ">FV<",                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_DISABLED        },
/* 1770 */  { fnOldItemError,               NOPARAM,                     ">I/a<",                                       ">I/a<",                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_DISABLED        },
/* 1771 */  { fnOldItemError,               NOPARAM,                     ">n<",                                         ">n<",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_DISABLED        },
/* 1772 */  { fnOldItemError,               NOPARAM,                     ">pp/a<",                                      ">pp/a<",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_DISABLED        },
/* 1773 */  { fnOldItemError,               NOPARAM,                     ">PMT<",                                       ">PMT<",                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_DISABLED        },
/* 1774 */  { fnOldItemError,               NOPARAM,                     ">PV<",                                        ">PV<",                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_DISABLED        },
/* 1775 */  { fnAtan2,                      NOPARAM /*#JM#*/,            "ATAN2",                                       "ATAN2",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_ENABLED  | PTP_NONE         | HG_ENABLED         },
/* 1776 */  { fnEffToI,                     NOPARAM,                     "EFF" STD_RIGHT_ARROW "I/a",                   "EFF" STD_RIGHT_ARROW "I/a",                   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1777 */  { fnDecisecondTimerApp,         NOPARAM,                     "0.1s",                                        "0.1s",                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1778 */  { fnResetTimerApp,              NOPARAM,                     "RESET",                                       "RESET",                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1779 */  { fnRecallTimerApp,             TM_REGISTER,                 "RCL",                                         "RCL",                                         (0 << TAM_MAX_BITS) |    99, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1780 */  { fnDeleteBackup,               NOT_CONFIRMED,               "DELBkup",                                     "DELBkup",                                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1781 */  { fnEff,                        NOPARAM,                     "I" STD_RIGHT_ARROW "EFF/a",                  "I" STD_RIGHT_ARROW "EFF/a",                    (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1782 */  { fnAddTimerApp,                NOPARAM,                     "TIM" STD_RIGHT_ARROW STD_SIGMA,              "TIM" STD_RIGHT_ARROW STD_SIGMA,                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1783 */  { fnAddLapTimerApp,             NOPARAM,                     "LAP" STD_RIGHT_ARROW STD_SIGMA,              "LAP" STD_RIGHT_ARROW STD_SIGMA,                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1784 */  { fnRegAddTimerApp,             NOPARAM,                     "TIM" STD_RIGHT_ARROW "R",                    "TIM" STD_RIGHT_ARROW "R",                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1785 */  { fnRegAddLapTimerApp,          NOPARAM,                     "LAP" STD_RIGHT_ARROW "R",                    "LAP" STD_RIGHT_ARROW "R",                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1786 */  { fnStartStopTimerApp,          NOPARAM,                     "R/S",                                        "R/S",                                          (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1787 */  { fnSetNBins,                   NOPARAM,                     "nBINS",                                       "nBINS",                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1788 */  { fnSetLoBin,                   NOPARAM,                     STD_DOWN_ARROW "BIN",                          STD_DOWN_ARROW "BIN",                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1789 */  { fnSetHiBin,                   NOPARAM,                     STD_UP_ARROW "BIN",                            STD_UP_ARROW "BIN",                            (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1790 */  { fnConvertStatsToHisto,        ITM_X,                       "HISTOX",                                      "HISTOX",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1791 */  { fnConvertStatsToHisto,        ITM_Y,                       "HISTOY",                                      "HISTOY",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1792 */  { fnPlotStat,                   H_PLOT,                      "HPLOT",                                       "HPLOT",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1793 */  { fnPlotStat,                   H_NORM,                      "HNORM",                                       "HNORM",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1794 */  { fnSqrt1Px2,                   NOPARAM,                     STD_SQUARE_ROOT "(1+x" STD_SUP_2 ")",          STD_SQUARE_ROOT "(1+x" STD_SUP_2 ")",          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },



//General
/* 1795 */  { fn_cnst_op_j_pol,             NOPARAM,                     "op_" STD_op_i STD_SUB_SUN,                    STD_op_i STD_SUB_SUN,                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_DISABLED        },//JM Operator j
/* 1796 */  { SetSetting,                   FLAG_BASE_MYM,               "MyMb",                                        "MyMb",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM NOBASE MENU SETTING
/* 1797 */  { SetSetting,                   FLAG_G_DOUBLETAP,            "g.2Tp",                                       "g.2Tp",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM KEY TAP DOUBLE SETTING
/* 1798 */  { SetSetting,                   FLAG_CPXMULT,                "CPXmul",                                      "CPXmul",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1799 */  { fnP_All_Regs,                 PRN_ALL,                     STD_PRINTER "ALLr",                            STD_PRINTER "ALLr",                            (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1800 */  { fnMultiplySI,                 85,                          STD_DOT "f",                                   STD_DOT "f",                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM PRE UNIT
/* 1801 */  { fnMultiplySI,                 88,                          STD_DOT "p",                                   STD_DOT "p",                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM PRE UNIT
/* 1802 */  { fnMultiplySI,                 91,                          STD_DOT "n",                                   STD_DOT "n",                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM PRE UNIT
/* 1803 */  { fnMultiplySI,                 94,                          STD_DOT STD_mu,                                STD_DOT STD_mu,                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM PRE UNIT
/* 1804 */  { fnMultiplySI,                 97,                          STD_DOT "m",                                   STD_DOT "m",                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM PRE UNIT
/* 1805 */  { fnMultiplySI,                 103,                         STD_DOT "k",                                   STD_DOT "k",                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM PRE UNIT
/* 1806 */  { fnMultiplySI,                 106,                         STD_DOT "M",                                   STD_DOT "M",                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM PRE UNIT
/* 1807 */  { fnMultiplySI,                 109,                         STD_DOT "G",                                   STD_DOT "G",                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM PRE UNIT
/* 1808 */  { fnMultiplySI,                 112,                         STD_DOT "T",                                   STD_DOT "T",                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM PRE UNIT
/* 1809 */  { addItemToBuffer,              ITM_QOPPA,                   "",                                            STD_QOPPA,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM GREEK
/* 1810 */  { addItemToBuffer,              ITM_DIGAMMA,                 "",                                            STD_DIGAMMA,                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM GREEK
/* 1811 */  { addItemToBuffer,              ITM_SAMPI,                   "",                                            STD_SAMPI,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM GREEK
/* 1812 */  { fnJM,                         7,                           "Y" STD_SPACE_3_PER_EM STD_RIGHT_ARROW STD_SPACE_3_PER_EM STD_DELTA, "Y" STD_SPACE_3_PER_EM STD_RIGHT_ARROW STD_SPACE_3_PER_EM STD_DELTA, (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM EE
/* 1813 */  { fnJM,                         6,                           STD_DELTA STD_SPACE_3_PER_EM STD_RIGHT_ARROW STD_SPACE_3_PER_EM "Y", STD_DELTA STD_SPACE_3_PER_EM STD_RIGHT_ARROW STD_SPACE_3_PER_EM "Y", (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM EE
/* 1814 */  { fnJM,                         9,                           "AtoSYM",                                      "abc" STD_RIGHT_ARROW  "012",                  (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM EE
/* 1815 */  { fnJM,                         8,                           "SYMtoA",                                      "012" STD_RIGHT_ARROW  "abc",                  (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM EE
/* 1816 */  { fnEulersFormula,              NOPARAM,                     STD_EulerE STD_SUP_i STD_SUP_BOLD_x,           STD_EulerE STD_SUP_i STD_SUP_BOLD_x,           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1817 */  { fn3Sto,                       96,                          "3STOZ" STD_SUB_9 STD_SUB_6,                   "3STOZ" STD_SUB_9 STD_SUB_6,                   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM EE
/* 1818 */  { fn3Rcl,                       96,                          "3RCLZ" STD_SUB_9 STD_SUB_6,                   "3RCLZ" STD_SUB_9 STD_SUB_6,                   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM EE
/* 1819 */  { fn3Sto,                       90,                          "3STOV" STD_SUB_9 STD_SUB_0,                   "3STOV" STD_SUB_9 STD_SUB_0,                   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM EE
/* 1820 */  { fn3Rcl,                       90,                          "3RCLV" STD_SUB_9 STD_SUB_0,                   "3RCLV" STD_SUB_9 STD_SUB_0,                   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM EE
/* 1821 */  { fn3Sto,                       93,                          "3STOI" STD_SUB_9 STD_SUB_3,                   "3STOI" STD_SUB_9 STD_SUB_3,                   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM EE
/* 1822 */  { fn3Rcl,                       93,                          "3RCLI" STD_SUB_9 STD_SUB_3,                   "3RCLI" STD_SUB_9 STD_SUB_3,                   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM EE
/* 1823 */  { fnJM,                         17,                          "Z=V" STD_DIVIDE "I",                           "Z=V" STD_DIVIDE "I",                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM EE
/* 1824 */  { fnJM,                         18,                          "V=I" STD_CROSS  "Z",                           "V=I" STD_CROSS  "Z",                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM EE
/* 1825 */  { fnJM,                         19,                          "I=V" STD_DIVIDE "Z",                           "I=V" STD_DIVIDE "Z",                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM EE
/* 1826 */  { fnJM,                         20,                          "X" STD_SPACE_3_PER_EM STD_RIGHT_ARROW STD_SPACE_3_PER_EM "BAL", "X" STD_SPACE_3_PER_EM STD_RIGHT_ARROW STD_SPACE_3_PER_EM "BAL", (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM EE
/* 1827 */  { fn_cnst_op_A,                 ITM_MATX_A,                  "op_A",                                        "[A]",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1828 */  { fn_cnst_op_a,                 NOPARAM,                     "op_a",                                        "a",                                           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM Operator a
/* 1829 */  { fn_cnst_op_aa,                NOPARAM,                     "op_a" STD_SUP_2,                              "a" STD_SUP_2,                                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM Operator a.a
/* 1830 */  { fn_cnst_op_j,                 NOPARAM,                     "op_" STD_op_i,                                STD_op_i,                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_DISABLED        },//JM Operator j
/* 1831 */  { fnChangeBaseJM,               2,                           "BIN",                                         "BIN",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM HEX
/* 1832 */  { fnChangeBaseJM,               8,                           "OCT",                                         "OCT",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM HEX
/* 1833 */  { fnChangeBaseJM,               10,                          "DEC",                                         "DEC",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM HEX
/* 1834 */  { fnChangeBaseJM,               16,                          "HEX",                                         "HEX",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM HEX
/* 1835 */  { fnSetWordSize,                8,                           "8-BIT",                                       "8-BIT",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM HEX
/* 1836 */  { fnSetWordSize,                16,                          "16-BIT",                                      "16-BIT",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM HEX
/* 1837 */  { fnSetWordSize,                32,                          "32-BIT",                                      "32-BIT",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM HEX
/* 1838 */  { fnSetWordSize,                64,                          "64-BIT",                                      "64-BIT",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM HEX
/* 1839 */  { fnHrDeg,                      NOPARAM,                     "HOUR",                                        "HOUR",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1840 */  { fnMinute,                     NOPARAM,                     "MINUTE",                                      "MINUTE",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1841 */  { fnSecond,                     NOPARAM,                     "SECOND",                                      "SECOND",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1842 */  { fnToTime,                     NOPARAM,                     "zyx" STD_RIGHT_ARROW TM,                      "zyx" STD_RIGHT_ARROW TM,                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1843 */  { fnTimeTo,                     NOPARAM,                     TM STD_RIGHT_ARROW "zyx",                      TM STD_RIGHT_ARROW "zyx",                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1844 */  { fnSetCountDownTimerApp,       TM_REGISTER,                 "RCL" STD_HOURGLASS_WH,                        "RCL" STD_HOURGLASS_WH,                        (0 << TAM_MAX_BITS) |    99, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1845 */  { addItemToBuffer,              ITM_qoppa,                   "",                                            STD_qoppa,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM GREEK
/* 1846 */  { addItemToBuffer,              ITM_digamma,                 "",                                            STD_digamma,                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM GREEK
/* 1847 */  { addItemToBuffer,              ITM_sampi,                   "",                                            STD_sampi,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM GREEK
/* 1848 */  { fnKeyCC,                      KEY_COMPLEX,                 "COMPLEX",                                     "COMPLEX",                                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM Change CC to COMPLEX
/* 1849 */  { fnToPolar2,                   NOPARAM,                     STD_RIGHT_ARROW "POLAR",                       STD_RIGHT_ARROW "P",                           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM TEXT & point to function to add POLAR/RECT
/* 1850 */  { fnToRect2,                    NOPARAM,                     STD_RIGHT_ARROW "RECT",                        STD_RIGHT_ARROW "R",                           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//SWAPPED ARROW DIRECTION & JM TEXT & point to function to add POLAR/RECT
/* 1851 */  { SetSetting,                   FLAG_CARRY,                  "CARRY",                                       "CARRY",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1852 */  { SetSetting,                   FLAG_OVERFLOW,               "OVERFL",                                      "OVERFL",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1853 */  { SetSetting,                   FLAG_ERPN,                   "eRPN",                                        "eRPN",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM eRPN
/* 1854 */  { SetSetting,                   FLAG_HOME_TRIPLE,            "HOME" STD_fg,                                 "HOME" STD_fg,                                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM HOME.3
/* 1855 */  { SetSetting,                   FLAG_SHFT_4s,                "SH.4s",                                       "SH.4s",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM SHIFT CANCEL
/* 1856 */  { SetSetting,                   FLAG_CPXRES,                 "CPXRES",                                      "CPXRES",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//dr
/* 1857 */  { SetSetting,                   FLAG_LEAD0,                  "LEAD.0",                                      "LEAD.0",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//dr
/* 1858 */  { SetSetting,                   JC_UC,                       "CAPS",                                        "CAPS",                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM CASE
/* 1859 */  { SetSetting,                   FLAG_BASE_HOME,              "HOMEb",                                       "HOMEb",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM eRPN
/* 1860 */  { itemToBeCoded,                NOPARAM,                     "Misc:",                                       "Misc:",                                       (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1861 */  { SetSetting,                   FLAG_MYM_TRIPLE,             "MyM" STD_fg,                                  "MyM" STD_fg,                                  (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM MYM.3
/* 1862 */  { fnDateTimeToJulian,           NOPARAM,                     DT TM STD_RIGHT_ARROW "J",                     DT TM STD_RIGHT_ARROW "J",                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1863 */  { fnJulianToDateTime,           NOPARAM,                     "J" STD_RIGHT_ARROW DT TM,                     "J" STD_RIGHT_ARROW DT TM,                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1864 */  { fnDisplayFormatCycle,         NOPARAM,                     "FSE",                                         "FSE",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM UNIT
/* 1865 */  { SetSetting,                   FLAG_LARGELI,                "dLrgLI",                                      "dLrgLI",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1866 */  { fnDisplayFormatSigFig,        TM_VALUE,                    "SIG",                                         "SIG",                                         (0 << TAM_MAX_BITS) |DSP_MAX,CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },//JM SIGFIG
/* 1867 */  { fnDisplayFormatUnit,          TM_VALUE,                    "UNIT",                                        "UNIT",                                        (0 << TAM_MAX_BITS) |DSP_MAX,CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },//JM UNIT
/* 1868 */  { fnRound,                      NOPARAM,                     "ROUND",                                       "ROUND",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1869 */  { fnRoundi,                     NOPARAM,                     "ROUNDI",                                      "ROUNDI",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1870 */  { fnDumpMenus,                  NOPARAM,                     "DUMPMNU",                                     "DUMPMNU",                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1871 */  { fnJM_2SI,                     NOPARAM,                     STD_RIGHT_ARROW "I",                           STD_RIGHT_ARROW "I",                           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1872 */  { fnChangeBaseMNU,              TM_VALUE_CHB,                STD_RIGHT_ARROW "INT",                         "#",                                           (2 << TAM_MAX_BITS) |    16, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 1873 */  { fnDRG,                        NOPARAM,                     "DRG",                                         "DRG",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_DISABLED        },
/* 1874 */  { fnCla,                        NOPARAM,                     "CLA",                                         "CLA",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_DISABLED  | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//GRAPH
/* 1875 */  { fnCln,                        NOPARAM,                     "CLN",                                         "CLN",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//GRAPH
/* 1876 */  { SetSetting,                   FLAG_DENANY,                 "DENANY",                                      "DENANY",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM DEN
/* 1877 */  { SetSetting,                   FLAG_DENFIX,                 "DENFIX",                                      "DENFIX",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM DEN
/* 1878 */  { itemToBeCoded,                NOPARAM,                     "",                                            "CASE UP",                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM CASE
/* 1879 */  { itemToBeCoded,                NOPARAM,                     "",                                            "CASE DN",                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM CASE
/* 1880 */  { fnListXY,                     NOPARAM,                     "LISTXY",                                      "LISTXY",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1881 */  { itemToBeCoded,                NOPARAM,                     "BITSET",                                      "BITSET",                                      (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1882 */  { fnSysFreeMem,                 NOPARAM,                     "HEAP",                                        "HEAP",                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1883 */  { itemToBeCoded,                NOPARAM,                     "",                                            "Inl. Tst",                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//INLINE_TEST
/* 1884 */  { fnSetInlineTest,              JC_ITM_TST,                  "",                                            "Test",                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//INLINE_TEST
/* 1885 */  { fnGetInlineTestBsToX,         NOPARAM,                     "Get BS",                                      "Get BS",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//INLINE_TEST
/* 1886 */  { fnSetInlineTestXToBs,         NOPARAM,                     "Set BS",                                      "Set BS",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//INLINE_TEST
/* 1887 */  { fnInDefault,                  ID_DP,                       "i" STD_SPACE_3_PER_EM STD_REAL_R,             "i" STD_SPACE_3_PER_EM STD_REAL_R,             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM INPUT DEFAULT
/* 1888 */  { SetSetting,                   FLAG_PROPFR,                 "PROPFR",                                      "PROPFR",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM PROPFR
/* 1889 */  { fnInDefault,                  ID_CPXDP,                    "i" STD_SPACE_3_PER_EM STD_COMPLEX_C,          "i" STD_SPACE_3_PER_EM STD_COMPLEX_C,          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM INPUT DEFAULT
/* 1890 */  { fnInDefault,                  ID_43S,                      "i" STD_SPACE_3_PER_EM "LI/" STD_REAL_R,       "i" STD_SPACE_3_PER_EM "LI/" STD_REAL_R,       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM INPUT DEFAULT
/* 1891 */  { fnInDefault,                  ID_LI,                       "i" STD_SPACE_3_PER_EM "LI",                   "i" STD_SPACE_3_PER_EM "LI",                   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM INPUT DEFAULT
/* 1892 */  { fnMultiplySI,                 115,                         STD_DOT "P",                                   STD_DOT "P",                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM PRE UNIT
/* 1893 */  { fnSHIFTfg,                    NOPARAM,                     "f/g",                                         "f/g",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM Shift replacement
/* 1894 */  { fnKeysManagement,             FROM_USER,                   "U" STD_RIGHT_ARROW "N",                       "U" STD_RIGHT_ARROW "N",                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1895 */  { SetSetting,                   FLAG_HPBASE,                 "BASE" STD_SUB_H STD_SUB_P,                    "BASE" STD_SUB_H STD_SUB_P,                    (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1896 */  { SetSetting,                   FLAG_FRCYC,                  "FRCYC",                                       "FRCYC",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1897 */  { SetSetting,                   FLAG_FGGR,                   "fg.GR",                                       "fg.GR",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM NOBASE MENU SETTING
/* 1898 */  { fnSigmaAssign,                16384+ITM_AIM,               "",                                            STD_RIGHT_DASHARROW STD_SPACE_4_PER_EM STD_alpha,(0 << TAM_MAX_BITS)|    0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1899 */  { SetSetting,                   ITM_DREAL,                   "d" STD_INTEGER_Z ".0",                        "d" STD_INTEGER_Z ".0",                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM INPUT DEFAULT
/* 1900 */  { fnSigmaAssign,                16384+ITM_SHIFTg,            "",                                            STD_RIGHT_DASHARROW STD_SPACE_4_PER_EM "g",    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1901 */  { itemToBeCoded,                NOPARAM,                     "Chef:",                                       "Chef:",                                       (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1902 */  UNIT_CONV(constFactorGlusFzus, multiply,                    "gal" STD_US STD_RIGHT_ARROW "floz" STD_US,     "gal" STD_US STD_RIGHT_ARROW                    ),
/* 1903 */  UNIT_CONV(constFactorFzusGlus,   divide,                    "floz" STD_US STD_RIGHT_ARROW "gal" STD_US,     "floz" STD_US STD_RIGHT_ARROW                   ),
/* 1904 */  { fnSigmaAssign,                16384+ITM_USERMODE,          "",                                            STD_RIGHT_DASHARROW STD_SPACE_4_PER_EM "USER", (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1905 */  { fnGetLastErr,                 NOPARAM,                     "LSTERR?",                                     "LSTERR?",                                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1906 */  { fnSigmaAssign,                16384+ITM_SIGMAPLUS,         "",                                            STD_RIGHT_DASHARROW STD_SPACE_4_PER_EM STD_SIGMA "+", (0 << TAM_MAX_BITS) |0,CAT_NONE| SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1907 */  { itemToBeCoded,                NOPARAM,                     "PLSTAT",                                      "PLSTAT",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1908 */  { fnSigmaAssign,                16384+ITM_SHIFTf,            "",                                            STD_RIGHT_DASHARROW STD_SPACE_4_PER_EM "f",    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1909 */  { fnTo_ms,                      NOPARAM,                     ".ms",                                         ".ms",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM DMS HMS
/* 1910 */  { fnFrom_ms,                    NOPARAM,                     ".ms" STD_SUP_MINUS STD_SUP_1,                 ".ms" STD_SUP_MINUS STD_SUP_1,                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM DMS HMS
/* 1911 */  { fnFrom_ymd,                   NOPARAM,                     DT STD_RIGHT_ARROW "x",                        DT STD_RIGHT_ARROW "x",                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM DMS HMS
/* 1912 */  { itemToBeCoded,                NOPARAM,                     "",                                            "TamRcl",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1913 */  { itemToBeCoded,                NOPARAM,                     "",                                            "TamAlpha",                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1914 */  { fnKeysManagement,             TO_USER,                     "N" STD_RIGHT_ARROW "U",                       "N" STD_RIGHT_ARROW "U",                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1915 */  { fnSigmaAssign,                16384+ITM_NULL,              "",                                            STD_RIGHT_DASHARROW STD_SPACE_4_PER_EM "nil",  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1916 */  { fnKeysManagement,             USER_DM42,                   "DM42",                                        "DM42",                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//J=V43
/* 1917 */  { SetSetting,                   FLAG_HPRP,                   "RP" STD_SUB_H STD_SUB_P,                      "RP" STD_SUB_H STD_SUB_P,                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1918 */  { fnSigmaAssign,                16384+KEY_fg,                "",                                            STD_RIGHT_DASHARROW STD_SPACE_4_PER_EM "f/g",  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1919 */  { fnYYDflt,                     YY_OFF,                      "YYoff",                                       "YYoff",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1920 */  { itemToBeCoded,                NOPARAM,                     STD_BOX STD_SIGMA "+KEY",                      STD_BOX STD_SIGMA "+KEY",                      (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1921 */  { itemToBeCoded,                NOPARAM,                     "HOME" STD_SPACE_HAIR,                         "HOME" STD_SPACE_HAIR,                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM HOME
/* 1922 */  { itemToBeCoded,                NOPARAM,                     "ALPHA",                                       "ALPHA",                                       (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM ALPHA
/* 1923 */  { itemToBeCoded,                NOPARAM,                     "BASE",                                        "BASE",                                        (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM BASE
/* 1924 */  { SetSetting,                   FLAG_MNUp1,                  "MNUp1",                                       "MNUp1",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1925 */  { itemToBeCoded,                NOPARAM,                     "ELEC",                                        "ELEC",                                        (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM EE
/* 1926 */  { fnT_ARROW,                    ITM_T_UP_ARROW,              "",                                            STD_UP_ARROW,                                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1927 */  { itemToBeCoded,                NOPARAM,                     "KEYS",                                        "KEYS",                                        (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1928 */  { fnT_ARROW,                    ITM_T_DOWN_ARROW,            "",                                            STD_DOWN_ARROW,                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1929 */  { fnT_ARROW,                    ITM_T_HOME,                  "",                                            "HOME",                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1930 */  { fnT_ARROW,                    ITM_T_END,                   "",                                            "END",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1931 */  { fnConvertStkToMx,             1,                          "zyx" STD_RIGHT_ARROW "M",                      "zyx" STD_RIGHT_ARROW "M",                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1932 */  { itemToBeCoded,                NOPARAM,                     STD_alpha "PARSE",                             STD_alpha "PARSE",                             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1933 */  { fnSaveAllPrograms,            NOPARAM,                     "WRXPall",                                     "WRXPall",                                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_CANCEL    | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1934 */  { fnRange,                      TM_VALUE,                    ">RNG<",                                       ">RNG<",                                       (0 << TAM_MAX_BITS) |  6145, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_16    | HG_ENABLED         },
/* 1935 */  { flagBrowser,                  GLOBAL_FLAGS_SCREEN,         "FLGS",                                        "FLGS",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM Changed STATUS
/* 1936 */  { SetSetting,                   CU_I,                        "CPX" STD_op_i,                                "CPX" STD_op_i,                                (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1937 */  { SetSetting,                   CU_J,                        "CPX" STD_op_j,                                "CPX" STD_op_j,                                (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1938 */  { SetSetting,                   SS_4,                        "SSIZE4",                                      "SSIZE4",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1939 */  { SetSetting,                   SS_8,                        "SSIZE8",                                      "SSIZE8",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1940 */  { SetSetting,                   FLAG_SPCRES,                 "SPCRES",                                      "SPCRES",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1941 */  { fnCFGsettings,                NOPARAM,                     "CFLG",                                        "CFLG",                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM Replacements
/* 1942 */  { SetSetting,                   TF_H12,                      "CLK12",                                       "CLK12",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM Replacements
/* 1943 */  { SetSetting,                   TF_H24,                      "CLK24",                                       "CLK24",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM Replacements
/* 1944 */  { SetSetting,                   PS_CROSS,                    "MULT" STD_CROSS,                              "MULT" STD_CROSS,                              (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1945 */  { SetSetting,                   PS_DOT,                      "MULT" STD_DOT,                                "MULT" STD_DOT,                                (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1946 */  { SetSetting,                   CM_POLAR,                    "POLAR",                                       "POLAR",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM Replacements
/* 1947 */  { fnJM,                         48,                          "",                                            "",                                            (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//placeholder for ITM_TST
/* 1948 */  { SetSetting,                   FLAG_PRTACT,                 "PRNTR",                                       "PRNTR",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM Replacements
/* 1949 */  { SetSetting,                   CM_RECTANGULAR,              "RECT",                                        "RECT",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM Replacements
/* 1950 */  { SetSetting,                   DO_SCI,                      "SCIOVR",                                      "SCIOVR",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM Replacements
/* 1951 */  { SetSetting,                   DO_ENG,                      "ENGOVR",                                      "ENGOVR",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM Replacements
/* 1952 */  { fnT_ARROW,                    ITM_T_LEFT_ARROW,            "",                                            STD_LEFT_ARROW,                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1953 */  { fnT_ARROW,                    ITM_T_RIGHT_ARROW,           "",                                            STD_RIGHT_ARROW,                               (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1954 */  { fnT_ARROW,                    ITM_T_LLEFT_ARROW,           "",                                            STD_LEFT_DASHARROW,                            (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1955 */  { fnT_ARROW,                    ITM_T_RRIGHT_ARROW,          "",                                            STD_RIGHT_DASHARROW,                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1956 */  { fnRlc,                        1,                           "RLC",                                         "RLC",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM SHOI
/* 1957 */  { fnRrc,                        1,                           "RRC",                                         "RRC",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM SHOI
/* 1958 */  { fnAsnViewer,                  NOPARAM,                     "KEYMAP",                                      "KEYMAP",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//J=V43
/* 1959 */  { fnKeysManagement,             USER_C47,                    "C47",                                         "C47",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1960 */  { fnKeysManagement,             USER_V47,                    "V47",                                         "V47",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1961 */  { fnKeysManagement,             USER_D47,                    "D47",                                         "D47",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1962 */  { fnKeysManagement,             USER_N47,                    "N47",                                         "N47",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1963 */  { fnKeysManagement,             USER_E47,                    "E47",                                         "E47",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1964 */  { fnKeysManagement,             USER_R47f_g,                 "R47",                                         "R47",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1965 */  { itemToBeCoded,                NOPARAM,                     "1965",                                        "1965",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1966 */  { itemToBeCoded,                NOPARAM,                     "1966",                                        "1966",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1967 */  { itemToBeCoded,                NOPARAM,                     "1967",                                        "1967",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1968 */  { itemToBeCoded,                NOPARAM,                     "1968",                                        "1968",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1969 */  { itemToBeCoded,                NOPARAM,                     "1969",                                        "1969",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1970 */  { itemToBeCoded,                NOPARAM,                     "1970",                                        "1970",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1971 */  { itemToBeCoded,                NOPARAM,                     "1971",                                        "1971",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1972 */  { itemToBeCoded,                NOPARAM,                     "1972",                                        "1972",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1973 */  { itemToBeCoded,                NOPARAM,                     "1973",                                        "1973",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1974 */  { fnSigmaAssign,                16384+ITM_XFACT,             "",                               STD_RIGHT_DASHARROW STD_SPACE_4_PER_EM "x!",                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1975 */  { fnSigmaAssign,                16384+ITM_DRG,               "",                               STD_RIGHT_DASHARROW STD_SPACE_4_PER_EM "DRG",               (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1976 */  { fnSigmaAssign,                16384+ITM_Rup,               "",                               STD_RIGHT_DASHARROW STD_SPACE_4_PER_EM "R" STD_UP_ARROW ,   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1977 */  { fnSigmaAssign,                16384+ITM_op_j_pol,          "",                               STD_RIGHT_DASHARROW STD_SPACE_4_PER_EM STD_op_i STD_SUB_SUN,(0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1978 */  { fnSigmaAssign,                16384+ITM_SNAP,              "",                               STD_RIGHT_DASHARROW STD_SPACE_4_PER_EM "SNAP",              (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1979 */  { fnSigmaAssign,                16384+ITM_TIMER,             "",                               STD_RIGHT_DASHARROW STD_SPACE_4_PER_EM "STPW",              (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1980 */  { conditionalPCURVE,            NOPARAM,                     "CURVE",                                       "CURVE",                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//GRAPH
/* 1981 */  { fnToPolar_HP,                 NOPARAM,                     STD_RIGHT_ARROW "POL"  STD_SUB_h STD_SUB_p,    STD_RIGHT_ARROW "POL"  STD_SUB_h STD_SUB_p,    (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1982 */  { fnToRect_HP,                  NOPARAM,                     STD_RIGHT_ARROW "RECT" STD_SUB_h STD_SUB_p,    STD_RIGHT_ARROW "RECT" STD_SUB_h STD_SUB_p,    (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1983 */  { fnToPolar_CX,                 NOPARAM,                     STD_RIGHT_ARROW "POL"  STD_SUB_c STD_SUB_x,    STD_RIGHT_ARROW "POL"  STD_SUB_c STD_SUB_x,    (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1984 */  { fnToRect_CX,                  NOPARAM,                     STD_RIGHT_ARROW "RECT" STD_SUB_c STD_SUB_x,    STD_RIGHT_ARROW "RECT" STD_SUB_c STD_SUB_x,    (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1985 */  { SetSetting,                   FLAG_BCD,                    "BCD",                                         "BCD",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 1986 */  { fnSetBCD,                     BCD9c ,                      "9CMPL",                                       "9CMPL",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1987 */  { fnSetBCD,                     BCD10c,                      "10CMPL",                                      "10CMPL",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1988 */  { fnSetBCD,                     BCDu,                        "BCDUNS",                                      "BCDUNS",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 1989 */  { fnByteShortcutsS,             6,                           "6:2",                                         "6:2",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM SHOI
/* 1990 */  { fnByteShortcutsS,             8,                           "8:2",                                         "8:2",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM SHOI
/* 1991 */  { fnByteShortcutsS,             16,                          "16:2",                                        "16:2",                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM SHOI
/* 1992 */  { fnByteShortcutsS,             32,                          "32:2",                                        "32:2",                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM SHOI
/* 1993 */  { fnByteShortcutsS,             64,                          "64:2",                                        "64:2",                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM SHOI
/* 1994 */  { fnByteShortcutsU,             6,                           "6:u",                                         "6:u",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM SHOI
/* 1995 */  { fnByteShortcutsU,             8,                           "8:u",                                         "8:u",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM SHOI
/* 1996 */  { fnByteShortcutsU,             16,                          "16:u",                                        "16:u",                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM SHOI
/* 1997 */  { fnByteShortcutsU,             32,                          "32:u",                                        "32:u",                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM SHOI
/* 1998 */  { fnByteShortcutsU,             64,                          "64:u",                                        "64:u",                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM SHOI
/* 1999 */  { fnSl,                         1,                           "SL",                                          "SL",                                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM SHOI
/* 2000 */  { fnSr,                         1,                           "SR",                                          "SR",                                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM SHOI
/* 2001 */  { fnRl,                         1,                           "RL",                                          "RL",                                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM SHOI
/* 2002 */  { fnRr,                         1,                           "RR",                                          "RR",                                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM SHOI
/* 2003 */  { fnSwapEndian,                 16,                          "W.SWP",                                       "W.SWP",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM SHOI
/* 2004 */  { fnSwapEndian,                 8,                           "B.SWP",                                       "B.SWP",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM SHOI
/* 2005 */  { fnClrMod,                     NOPARAM,                     "CLRMOD",                                      "CLRMOD",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//clear HEX mode
/* 2006 */  { fnShoiXRepeats,               TM_VALUE,                    "dSI",                                         "dSI",                                         (0 << TAM_MAX_BITS) |     3, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },//JM SHOI
/* 2007 */  { fnScale,                      NOPARAM,                     "X:Y=1",                                       "X:Y=1",                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM GRAPHING
/* 2008 */  { SetSetting,                   FLAG_TOPHEX,                 "KEY" STD_SUB_A STD_SUB_MINUS STD_SUB_F,       "KEY" STD_SUB_A STD_SUB_MINUS STD_SUB_F,       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2009 */  { fnPline,                      NOPARAM,                     "LINE",                                        "LINE",                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//GRAPH
/* 2010 */  { fnPcros,                      NOPARAM,                     "CROSS",                                       "CROSS",                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//GRAPH
/* 2011 */  { fnPbox,                       NOPARAM,                     "BOX",                                         "BOX",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//GRAPH
/* 2012 */  { fnPvect,                      NOPARAM,                     "VECT",                                        "VECT",                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM VECTOR MODE
/* 2013 */  { fnPNvect,                     NOPARAM,                     "N.VECT",                                      "N.VECT",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM VECTOR MODE
/* 2014 */  { fnPx,                         NOPARAM,                     "Y.AXIS",                                      "Y.AXIS",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2015 */  { fnPy,                         NOPARAM,                     "X.AXIS",                                      "X.AXIS",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2016 */  { fnOldItemError,               NOPARAM,                     ">DMX<",                                       ">DMX<",                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//Old item
/* 2017 */  { fnOldItemError,               NOPARAM,                     ">SDIGS<",                                     ">SDIGS<",                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//Old item
/* 2018 */  { fnAview,                      TM_REGISTER,                 "AVIEW",                                       "AVIEW",                                       (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 2019 */  { fnGetRoundingMode,            NOPARAM,                     "RMODE?",                                      "RMODE?",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2020 */  { fnPrompt,                     TM_REGISTER,                 "PROMPT",                                      "PROMPT",                                      (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 2021 */  { fnKeysManagement,             USER_ARESET,                 "My" STD_alpha ".R",                           "My" STD_alpha ".R",                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2022 */  { fnKeysManagement,             USER_MRESET,                 "MyM.R",                                       "MyM.R",                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2023 */  { fnKeysManagement,             USER_KRESET,                 "KEYS.R",                                      "KEYS.R",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2024 */  { fnPintg,                      NOPARAM,                     STD_SIGMA STD_y_BAR STD_DELTA "x",             STD_SIGMA STD_y_BAR STD_DELTA "x",             (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//GRAPH
/* 2025 */  { fnPdiff,                      NOPARAM,                     STD_DELTA "y/" STD_DELTA "x",                  STD_DELTA "y/" STD_DELTA "x",                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//GRAPH
/* 2026 */  { fnPrms,                       NOPARAM,                     "RMS",                                         "RMS",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//GRAPH
/* 2027 */  { fnPshade,                     NOPARAM,                     STD_INTEGRAL "AREA",                           STD_INTEGRAL "AREA",                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//GRAPH
/* 2028 */  { itemToBeCoded,                NOPARAM,                     "PLFUNC",                                      "PLFUNC",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2029 */  { SetSetting,                   JC_NL,                       "NUM",                                         "NUM",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2030 */  { itemToBeCoded,                NOPARAM,                     "",                                            "NLock",                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2031 */  { itemToBeCoded,                NOPARAM,                     "",                                            "Nulock",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2032 */  { addItemToBuffer,              ITM_EEXCHR,                  "",                                            STD_SUB_E_OUTLINE,                             (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2033 */  { fnClGrf,                      NOPARAM,                     "CLGRF",                                       "CLGRF",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2034 */  { fnPMzoom,                     1,                           "-ZOOM",                                       "-ZOOM",                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//GRAPH
/* 2035 */  { fnPMzoom,                     2,                           "+ZOOM",                                       "+ZOOM",                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//GRAPH
/* 2036 */  { itemToBeCoded,                NOPARAM,                     "TRG_R47",                                     "TRG",                                         (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2037 */  { itemToBeCoded,                NOPARAM,                     "PREF",                                        "PREF",                                        (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2038 */  { fnSafeReset,                  NOPARAM,                     "SRESET",                                      "SRESET",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2039 */  { fnFlipFlag,                   FLAG_SH_LONGPRESS,           "KEY.LP",                                       "KEY.LP",                                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2040 */  { graph_stat,                   NOPARAM,                     "PLSTAT",                                      "PLSTAT",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2041 */  { fnConvertMxToStk,             1,                           "M" STD_RIGHT_ARROW "zyx",                     "M" STD_RIGHT_ARROW "zyx",                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2042 */  { fnPlotReset,                  NOPARAM,                     "PLTRST",                                      "PLTRST",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2043 */  { runDMCPmenu,                  CONFIRMED,                   "DMCP",                                        "DMCP",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2044 */  { activateUSBdisk,              CONFIRMED,                   "ActUSB",                                      "ActUSB",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2045 */  { itemToBeCoded,                NOPARAM,                     "Speed:",                                      "Speed:",                                      (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2046 */  { itemToBeCoded,                NOPARAM,                     "Angle:",                                      "Angle:",                                      (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2047 */  { itemToBeCoded,                NOPARAM,                     "2047",                                        "2047",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2048 */  { itemToBeCoded,                NOPARAM,                     "2048",                                        "2048",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2049 */  { itemToBeCoded,                NOPARAM,                     "2049",                                        "2049",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2050 */  { itemToBeCoded,                NOPARAM,                     "2050",                                        "2050",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2051 */  { itemToBeCoded,                NOPARAM,                     "2051",                                        "2051",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2052 */  { itemToBeCoded,                NOPARAM,                     "2052",                                        "2052",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2053 */  { SetSetting,                   FLAG_2TO10,                  "1024" STD_SUP_n,                              "1024" STD_SUP_n,                              (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2054 */  { fnKeysManagement,             USER_HRESET,                 "HOME.R",                                      "HOME.R",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2055 */  { fnKeysManagement,             USER_PRESET,                 "PFN.R",                                       "PFN.R",                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2056 */  { SetSetting,                   FLAG_IRFRAC,                 "IRFRAC",                                      "IRFRAC",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2057 */  { fnOldItemError,               NOPARAM,                     ">fgOFF<",                                     ">fgOFF<",                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//Old item
/* 2058 */  { SetSetting,                   FLAG_FGLNLIM,                "fg.LIM",                                      "fg.LIM",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2059 */  { SetSetting,                   FLAG_FGLNFUL,                "fg.FUL",                                      "fg.FUL",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2060 */  { fnLongPressSwitches,          RBX_M124,                    "M.124",                                       "M.124",                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2061 */  { fnLongPressSwitches,          RBX_F1234,                   "F.1234",                                      "F.1234",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2062 */  { fnLongPressSwitches,          RBX_M1234,                   "M.1234",                                      "M.1234",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2063 */  { fnLongPressSwitches,          RBX_F14,                     "F.14",                                        "F.14",                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2064 */  { fnLongPressSwitches,          RBX_M14,                     "M.14",                                        "M.14",                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2065 */  { fnLongPressSwitches,          RBX_F124,                    "F.124",                                       "F.124",                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2066 */  { itemToBeCoded,                NOPARAM,                     "REG",                                         "REG",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2067 */  { itemToBeCoded,                NOPARAM,                     "FLG",                                         "FLG",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2068 */  { itemToBeCoded,                NOPARAM,                     "",                                            "TamNonReg",                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2069 */  { addItemToBuffer,              ITM_LG_SIGN,                 "LOG",                                         "LOG",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2070 */  { addItemToBuffer,              ITM_LN_SIGN,                 "LN",                                          "LN",                                          (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2071 */  { addItemToBuffer,              ITM_SIN_SIGN,                "SIN",                                         "SIN",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2072 */  { addItemToBuffer,              ITM_COS_SIGN,                "COS",                                         "COS",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2073 */  { addItemToBuffer,              ITM_TAN_SIGN,                "TAN",                                         "TAN",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2074 */  { fnMedianXY,                   NOPARAM,                     "x" STD_SUB_M STD_SUB_E STD_SUB_D STD_SUB_N,   "x" STD_SUB_M STD_SUB_E STD_SUB_D STD_SUB_N,   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2075 */  { fnLowerQuartileXY,            NOPARAM,                     "x" STD_SUB_Q STD_SUB_1,                       "x" STD_SUB_Q STD_SUB_1,                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2076 */  { fnUpperQuartileXY,            NOPARAM,                     "x" STD_SUB_Q STD_SUB_3,                       "x" STD_SUB_Q STD_SUB_3,                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2077 */  { fnMADXY,                      NOPARAM,                     "x" STD_SUB_M STD_SUB_A STD_SUB_D,             "x" STD_SUB_M STD_SUB_A STD_SUB_D,             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2078 */  { fnIQRXY,                      NOPARAM,                     "x" STD_SUB_I STD_SUB_Q STD_SUB_R,             "x" STD_SUB_I STD_SUB_Q STD_SUB_R,             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2079 */  { fnRangeXY,                    NOPARAM,                     "x" RANGE_,                                    "x" RANGE_,                                    (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2080 */  { itemToBeCoded,                NOPARAM,                     "REGR",                                        "REGR",                                        (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2081 */  { itemToBeCoded,                NOPARAM,                     "MODEL",                                       "MODEL",                                       (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2082 */  { fnPercentileXY,               NOPARAM,                     "x%" STD_SUB_I STD_SUB_L STD_SUB_E,            "x%" STD_SUB_I STD_SUB_L STD_SUB_E,            (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2083 */  { fnLINPOL,                     NOPARAM,                     "LINPOL",                                      "LINPOL",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2084 */  UNIT_CONV(constFactorNmiKm,     multiply,   "knot" STD_RIGHT_ARROW "km/h",                      "knot" STD_RIGHT_ARROW "km/h"),
/* 2085 */  UNIT_CONV(constFactorNmiKm,       divide,   "km/h" STD_RIGHT_ARROW "knot",                      "km/h" STD_RIGHT_ARROW "knot"),
/* 2086 */  UNIT_CONV(constFactorKmphmps,   multiply,   "km/h" STD_RIGHT_ARROW "m/s",                       "km/h" STD_RIGHT_ARROW "m/s"),
/* 2087 */  UNIT_CONV(constFactorKmphmps,     divide,   "m/s" STD_RIGHT_ARROW "km/h",                       "m/s" STD_RIGHT_ARROW "km/h"),
/* 2088 */  UNIT_CONV(constFactorRpmDegps,  multiply,   "RPM" STD_RIGHT_ARROW "deg/s",                      "RPM" STD_RIGHT_ARROW "deg/s"),
/* 2089 */  UNIT_CONV(constFactorRpmDegps,    divide,   "deg/s" STD_RIGHT_ARROW "RPM",                      "deg/s" STD_RIGHT_ARROW "RPM"),
/* 2090 */  UNIT_CONV(constFactorMiKm,      multiply,   "mph" STD_RIGHT_ARROW "km/h",                       "mph" STD_RIGHT_ARROW "km/h"),
/* 2091 */  UNIT_CONV(constFactorMiKm,        divide,   "km/h" STD_RIGHT_ARROW "mph",                       "km/h" STD_RIGHT_ARROW "mph"),
/* 2092 */  UNIT_CONV(constFactorMphmps,    multiply,   "mph" STD_RIGHT_ARROW "m/s",                        "mph" STD_RIGHT_ARROW "m/s"),
/* 2093 */  UNIT_CONV(constFactorMphmps,      divide,   "m/s" STD_RIGHT_ARROW "mph",                        "m/s" STD_RIGHT_ARROW "mph"),
/* 2094 */  UNIT_CONV(constFactorRpmRadps,  multiply,   "RPM" STD_RIGHT_ARROW "rad/s",                      "RPM" STD_RIGHT_ARROW "rad/s"),
/* 2095 */  UNIT_CONV(constFactorRpmRadps,    divide,   "rad/s" STD_RIGHT_ARROW "RPM",                      "rad/s" STD_RIGHT_ARROW "RPM"),
/* 2096 */  { fnCvtDegRad,                  multiply,                    "deg" STD_RIGHT_ARROW "rad",                   "deg" STD_RIGHT_ARROW "rad",                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2097 */  { fnCvtDegRad,                  divide,                      "rad" STD_RIGHT_ARROW "deg",                   "rad" STD_RIGHT_ARROW "deg",                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2098 */  { fnCvtDegGrad,                 multiply,                    "deg" STD_RIGHT_ARROW "grad",                  "deg" STD_RIGHT_ARROW "grad",                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2099 */  { fnCvtDegGrad,                 divide,                      "grad" STD_RIGHT_ARROW "deg",                  "grad" STD_RIGHT_ARROW "deg",                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2100 */  { fnCvtGradRad,                 multiply,                    "grad" STD_RIGHT_ARROW "rad",                  "grad" STD_RIGHT_ARROW "rad",                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2101 */  { fnCvtGradRad,                 divide,                      "rad" STD_RIGHT_ARROW "grad",                  "rad" STD_RIGHT_ARROW "grad",                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2102 */  { itemToBeCoded,                NOPARAM,                     "TRG",                                         "TRG",                                         (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2103 */  { itemToBeCoded,                NOPARAM,                     "TRG" STD_ELLIPSIS,                            "TRG" STD_ELLIPSIS,                            (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2104 */  { itemToBeCoded,                NOPARAM,                     "2104",                                        "2104",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2105 */  { itemToBeCoded,                NOPARAM,                     "2105",                                        "2105",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2106 */  { itemToBeCoded,                NOPARAM,                     "2106",                                        "2106",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2107 */  { itemToBeCoded,                NOPARAM,                     "PLOT",                                        "PLOT",                                        (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },//JM Change U> arrow to CONV. Changed again to UNIT
/* 2108 */  { itemToBeCoded,                NOPARAM,                     "",                                            "TamIndirect",                                 (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2109 */  { itemToBeCoded,                NOPARAM,                     "",                                            "TamNonRegMax",                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2110 */  { fnDenMax,                     0,                           "DMXmax",                                      "DMXmax",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2111 */  { fnSetGapChar,                 ITM_DOT,                     "IDOT" STD_DOT,                                "DOT" STD_DOT,                                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2112 */  { fnSetGapChar,                 ITM_WDOT,                    "IWDOT" STD_SPACE_4_PER_EM STD_WDOT,           "WDOT" STD_SPACE_4_PER_EM STD_WDOT,            (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2113 */  { fnSetGapChar,                 ITM_PERIOD,                  "IPER.",                                       "PER.",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2114 */  { fnSetGapChar,                 ITM_WPERIOD,                 "IWPER" STD_SPACE_4_PER_EM STD_WPERIOD,        "WPER" STD_SPACE_4_PER_EM STD_WPERIOD,         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2115 */  { fnSetGapChar,                 ITM_COMMA,                   "ICOM,",                                       "COM,",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2116 */  { fnSetGapChar,                 ITM_WCOMMA,                  "IWCOM" STD_SPACE_4_PER_EM STD_WCOMMA,         "WCOM" STD_SPACE_4_PER_EM STD_WCOMMA,          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2117 */  { fnSetGapChar,                 ITM_QUOTE,                   "IWTICK" STD_SPACE_4_PER_EM "'",               "WTICK" STD_SPACE_4_PER_EM "'",                (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2118 */  { fnSetGapChar,                 ITM_NQUOTE,                  "ITICK" STD_NQUOTE,                            "TICK" STD_NQUOTE,                             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2119 */  { fnSetGapChar,                 ITM_SPACE_PUNCTUATION,       "ISPC" STD_OPEN_BOX,                           "SPC" STD_OPEN_BOX,                            (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2120 */  { fnSetGapChar,                 ITM_SPACE_4_PER_EM,          "INSPC" STD_INV_BRIDGE,                        "NSPC" STD_INV_BRIDGE,                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2121 */  { fnSetGapChar,                 ITM_SPACE_EM,                "IWSPC" STD_INV_BRIDGE STD_INV_BRIDGE,         "WSPC" STD_INV_BRIDGE STD_INV_BRIDGE,          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2122 */  { fnSetGapChar,                 ITM_UNDERSCORE,              "IUNDR" STD_UNDERSCORE,                        "UNDR" STD_UNDERSCORE,                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2123 */  { fnSetGapChar,                 ITM_NULL,                    "INONE",                                       "NONE",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2124 */  { fnSetGapChar,                 32768+ITM_DOT,               "FDOT" STD_DOT,                                "DOT" STD_DOT,                                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2125 */  { fnSetGapChar,                 32768+ITM_WDOT,              "FWDOT" STD_SPACE_4_PER_EM STD_WDOT,           "WDOT" STD_SPACE_4_PER_EM STD_WDOT,            (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2126 */  { fnSetGapChar,                 32768+ITM_PERIOD,            "FPER.",                                       "PER.",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2127 */  { fnSetGapChar,                 32768+ITM_WPERIOD,           "FWPER" STD_SPACE_4_PER_EM STD_WPERIOD,        "WPER" STD_SPACE_4_PER_EM STD_WPERIOD,         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2128 */  { fnSetGapChar,                 32768+ITM_COMMA,             "FCOM,",                                       "COM,",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2129 */  { fnSetGapChar,                 32768+ITM_WCOMMA,            "FWCOM" STD_SPACE_4_PER_EM STD_WCOMMA,         "WCOM" STD_SPACE_4_PER_EM STD_WCOMMA,          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2130 */  { fnSetGapChar,                 32768+ITM_QUOTE,             "FWTICK" STD_SPACE_4_PER_EM "'",               "WTICK" STD_SPACE_4_PER_EM "'",                (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2131 */  { fnSetGapChar,                 32768+ITM_NQUOTE,            "FTICK" STD_NQUOTE,                            "TICK" STD_NQUOTE,                             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2132 */  { fnSetGapChar,                 32768+ITM_SPACE_PUNCTUATION, "FSPC" STD_OPEN_BOX,                           "SPC" STD_OPEN_BOX,                            (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2133 */  { fnSetGapChar,                 32768+ITM_SPACE_4_PER_EM,    "FNSPC" STD_INV_BRIDGE,                        "NSPC" STD_INV_BRIDGE,                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2134 */  { fnSetGapChar,                 32768+ITM_SPACE_EM,          "FWSPC" STD_INV_BRIDGE STD_INV_BRIDGE,         "WSPC" STD_INV_BRIDGE STD_INV_BRIDGE,          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2135 */  { fnSetGapChar,                 32768+ITM_UNDERSCORE,        "FUNDR" STD_UNDERSCORE,                        "UNDR" STD_UNDERSCORE,                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2136 */  { fnSetGapChar,                 32768+ITM_NULL,              "FNONE",                                       "NONE",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2137 */  { fnSetGapChar,                 49152+ITM_DOT,               "RDOT" STD_DOT,                                "DOT" STD_DOT,                                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2138 */  { fnSetGapChar,                 49152+ITM_WDOT,              "RWDOT" STD_SPACE_4_PER_EM STD_WDOT,           "WDOT" STD_SPACE_4_PER_EM STD_WDOT,            (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2139 */  { fnSetGapChar,                 49152+ITM_PERIOD,            "RPER.",                                       "PER.",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2140 */  { fnSetGapChar,                 49152+ITM_WPERIOD,           "RWPER" STD_SPACE_4_PER_EM STD_WPERIOD,        "WPER" STD_SPACE_4_PER_EM STD_WPERIOD,         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2141 */  { fnSetGapChar,                 49152+ITM_COMMA,             "RCOM,",                                       "COM,",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2142 */  { fnSetGapChar,                 49152+ITM_WCOMMA,            "RWCOM" STD_SPACE_4_PER_EM STD_WCOMMA,         "WCOM" STD_SPACE_4_PER_EM STD_WCOMMA,          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2143 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_WDOT,                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2144 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_WPERIOD,                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2145 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_WCOMMA ,                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2146 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_NQUOTE,                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2147 */  { fnSetFirstGregorianDay,       ITM_JUL_GREG_1582,           "JG.1582",                                     "JG.1582",                                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2148 */  { fnSetFirstGregorianDay,       ITM_JUL_GREG_1752,           "JG.1752",                                     "JG.1752",                                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2149 */  { fnSetFirstGregorianDay,       ITM_JUL_GREG_1873,           "JG.1873",                                     "JG.1873",                                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2150 */  { fnSetFirstGregorianDay,       ITM_JUL_GREG_1949,           "JG.1949",                                     "JG.1949",                                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2151 */  { itemToBeCoded,                NOPARAM,                     "IPART",                                       "IPART",                                       (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2152 */  { itemToBeCoded,                NOPARAM,                     "RADIX",                                       "RADIX",                                       (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2153 */  { itemToBeCoded,                NOPARAM,                     "FPART",                                       "FPART",                                       (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2154 */  { fnSetFractionDigits,          TM_VALUE,                    "FDIGS",                                       "FDIGS",                                       (0 << TAM_MAX_BITS) |    34, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 2155 */  { fnSettingsDispFormatGrpL,     TM_VALUE,                    "IPGRP",                                       "IPGRP",                                       (2 << TAM_MAX_BITS) |     9, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 2156 */  { fnSettingsDispFormatGrp1Lo,   TM_VALUE,                    "IPGRP1x",                                     "IPGRP1x",                                     (0 << TAM_MAX_BITS) |     9, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 2157 */  { fnSettingsDispFormatGrp1L,    TM_VALUE,                    "IPGRP1",                                      "IPGRP1",                                      (0 << TAM_MAX_BITS) |     9, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 2158 */  { fnSettingsDispFormatGrpR,     TM_VALUE,                    "FPGRP",                                       "FPGRP",                                       (2 << TAM_MAX_BITS) |     9, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 2159 */  { fnMenuGapL,                   MNU_GAP_L,                   "IPART",                                       "IPART",                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2160 */  { fnMenuGapRX,                  MNU_GAP_RX,                  "RADIX",                                       "RADIX",                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2161 */  { fnMenuGapR,                   MNU_GAP_R,                   "FPART",                                       "FPART",                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2162 */  { fnPplus,                      NOPARAM,                     "PLUS",                                        "PLUS",                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2163 */  UNIT_CONV(constFactorInchCm,    multiply,   "in"  STD_RIGHT_ARROW "cm",                         "in"  STD_RIGHT_ARROW "cm"),
/* 2164 */  UNIT_CONV(constFactorInchCm,      divide,   "cm" STD_RIGHT_ARROW "in" ,                         "cm" STD_RIGHT_ARROW "in" ),
/* 2165 */  { addItemToBuffer,              ITM_op_j_SIGN,               "op_" STD_op_i,                                STD_op_i,                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2166 */  { addItemToBuffer,              ITM_poly_SIGN,               "Poly4",                                       "Poly4",                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2167 */  UNIT_CONV(constFactorNmiMi,     multiply,   "nmi" STD_RIGHT_ARROW "mi" ,                        "nmi" STD_RIGHT_ARROW "mi" ),
/* 2168 */  UNIT_CONV(constFactorNmiMi,       divide,   "mi"  STD_RIGHT_ARROW "nmi",                        "mi"  STD_RIGHT_ARROW "nmi"),
/* 2169 */  UNIT_CONV(constFactorFurtom,    multiply,   "fur" STD_RIGHT_ARROW "m",                          "fur" STD_RIGHT_ARROW "m"),
/* 2170 */  UNIT_CONV(constFactorFurtom,      divide,   "m" STD_RIGHT_ARROW "fur",                          "m" STD_RIGHT_ARROW "fur"),
/* 2171 */  UNIT_CONV(constFactorFtntos,    multiply,   "ftn" STD_RIGHT_ARROW "s",                          "ftn" STD_RIGHT_ARROW "s"),
/* 2172 */  UNIT_CONV(constFactorFtntos,      divide,   "s" STD_RIGHT_ARROW "ftn",                          "s" STD_RIGHT_ARROW "ftn"),
/* 2173 */  UNIT_CONV(constFactorFpftomps,  multiply,   "fur/ftn" STD_RIGHT_ARROW "m/s",                    "fur/ftn" STD_RIGHT_ARROW "m/s"),
/* 2174 */  UNIT_CONV(constFactorFpftomps,    divide,   "m/s" STD_RIGHT_ARROW "fur/ftn",                    "m/s" STD_RIGHT_ARROW "fur/ftn"),
/* 2175 */  UNIT_CONV(constFactorBrdstom,   multiply,   "brds" STD_RIGHT_ARROW "m",                         "brds" STD_RIGHT_ARROW "m"),
/* 2176 */  UNIT_CONV(constFactorBrdstom,     divide,   "m" STD_RIGHT_ARROW "brds",                         "m" STD_RIGHT_ARROW "brds"),
/* 2177 */  UNIT_CONV(constFactorFirtokg,   multiply,   "fir" STD_RIGHT_ARROW "kg",                         "fir" STD_RIGHT_ARROW "kg"),
/* 2178 */  UNIT_CONV(constFactorFirtokg,     divide,   "kg" STD_RIGHT_ARROW "fir",                         "kg" STD_RIGHT_ARROW "fir"),
/* 2179 */  UNIT_CONV(constFactorFpftokph,  multiply,   "fur/ftn" STD_RIGHT_ARROW "km/h",                   "fur/ftn" STD_RIGHT_ARROW "km/h"),
/* 2180 */  UNIT_CONV(constFactorFpftokph,    divide,   "km/h" STD_RIGHT_ARROW "fur/ftn",                   "km/h" STD_RIGHT_ARROW "fur/ftn"),
/* 2181 */  UNIT_CONV(constFactorBrdstoin,  multiply,   "brds" STD_RIGHT_ARROW "in" ,                       "brds" STD_RIGHT_ARROW "in" ),
/* 2182 */  UNIT_CONV(constFactorBrdstoin,    divide,   "in"  STD_RIGHT_ARROW "brds",                       "in"  STD_RIGHT_ARROW "brds"),
/* 2183 */  UNIT_CONV(constFactorFirtolb,   multiply,   "fir" STD_RIGHT_ARROW "lb" ,                        "fir" STD_RIGHT_ARROW "lb" ),
/* 2184 */  UNIT_CONV(constFactorFirtolb,     divide,   "lb"  STD_RIGHT_ARROW "fir",                        "lb"  STD_RIGHT_ARROW "fir"),
/* 2185 */  UNIT_CONV(constFactorFpftomph,  multiply,   "fur/ftn" STD_RIGHT_ARROW "mph",                    "fur/ftn" STD_RIGHT_ARROW "mph"),
/* 2186 */  UNIT_CONV(constFactorFpftomph,    divide,   "mph" STD_RIGHT_ARROW "fur/ftn",                    "mph" STD_RIGHT_ARROW "fur/ftn"),
/* 2187 */  UNIT_CONV(constFactorFpstokph,  multiply,   "ft/s" STD_RIGHT_ARROW "km/h",                      "ft/s" STD_RIGHT_ARROW "km/h"),
/* 2188 */  UNIT_CONV(constFactorFpstokph,    divide,   "km/h" STD_RIGHT_ARROW "ft/s",                      "km/h" STD_RIGHT_ARROW "ft/s"),
/* 2189 */  UNIT_CONV(constFactorFpstomps,  multiply,   "ft/s" STD_RIGHT_ARROW "m/s",                       "ft/s" STD_RIGHT_ARROW "m/s"),
/* 2190 */  UNIT_CONV(constFactorFpstomps,    divide,   "m/s" STD_RIGHT_ARROW "ft/s",                       "m/s" STD_RIGHT_ARROW "ft/s"),
/* 2191 */  { SetSetting,                   JC_SS,                       SUPSUB_,                                       SUPSUB_,                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2192 */  { itemToBeCoded,                NOPARAM,                     "",                                            "SCRNRM",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2193 */  { itemToBeCoded,                NOPARAM,                     "",                                            "SCRSUP",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2194 */  { itemToBeCoded,                NOPARAM,                     "",                                            "SCRSUB",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2195 */  { fnHide,                       TM_VALUE,                    "HIDE",                                        "HIDE",                                        (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 2196 */  { fnDenMax,                     TM_VALUE_MAX,                ">DMX<",                                       ">DMX<",                                       (0 << TAM_MAX_BITS) |MAX_DENMAX,CAT_NONE|SLS_ENABLED  | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_16    | HG_ENABLED         },
/* 2197 */  { fnSetSignificantDigits,       TM_VALUE,                    "SDIGS",                                       "SDIGS",                                       (0 << TAM_MAX_BITS) |    34, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 2198 */  { fnSetVolume,                  TM_VALUE,                    "VOL",                                         "VOL",                                         (0 << TAM_MAX_BITS) |    11, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 2199 */  { fnGetVolume,                  NOPARAM,                     "VOL?",                                        "VOL?",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2200 */  { fnVolumeUp,                   NOPARAM,                     "VOL" STD_UP_ARROW,                            "VOL" STD_UP_ARROW,                            (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2201 */  { fnVolumeDown,                 NOPARAM,                     "VOL" STD_DOWN_ARROW,                          "VOL" STD_DOWN_ARROW,                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2202 */  { fnBuzz,                       NOPARAM,                     "BUZZ",                                        "BUZZ",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2203 */  { fnPlay,                       TM_REGISTER,                 "PLAY",                                        "PLAY",                                        (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 2204 */  UNIT_CONV(constFactorL100Tokml, invert | multiply,           STD_litre "/hkm" STD_RIGHT_ARROW "km/" STD_litre,  STD_litre "/100km" STD_RIGHT_ARROW),
/* 2205 */  UNIT_CONV(constFactorL100Tokml, invert | multiply,           "km/" STD_litre STD_RIGHT_ARROW STD_litre "/hkm",  "km/" STD_litre STD_RIGHT_ARROW),
/* 2206 */  { fnKmletok100K,                multiply,                    "km/" STD_litre STD_SUB_e STD_RIGHT_ARROW "E/hkm", "km/" STD_litre STD_SUB_e STD_RIGHT_ARROW, (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2207 */  { fnKmletok100K,                divide,                      "E/hkm" STD_RIGHT_ARROW "km/" STD_litre STD_SUB_e, "kWh/100km" STD_RIGHT_ARROW,               (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2208 */  UNIT_CONV(constFactorK100Ktokmk, invert | multiply,          "E/hkm" STD_RIGHT_ARROW "km/E",                    "kWh/100km" STD_RIGHT_ARROW),
/* 2209 */  UNIT_CONV(constFactorK100Ktokmk, invert | multiply,          "km/E" STD_RIGHT_ARROW "E/hkm",                    "km/kWh" STD_RIGHT_ARROW),
/* 2210 */  { fnL100Tomgus,                 multiply,                    STD_litre "/hkm" STD_RIGHT_ARROW "mpg" STD_US,     STD_litre "/100km" STD_RIGHT_ARROW,        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2211 */  { fnL100Tomgus,                 divide,                      "mpg" STD_US STD_RIGHT_ARROW STD_litre "/hkm",     "mpg" STD_US STD_RIGHT_ARROW,              (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2212 */  { fnMgeustok100M,               multiply,                    "mge" STD_US STD_RIGHT_ARROW "E/100mi",            "mge" STD_US STD_RIGHT_ARROW,              (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2213 */  { fnMgeustok100M,               divide,                      "E/100mi" STD_RIGHT_ARROW "mge" STD_US,            "kWh/100mi" STD_RIGHT_ARROW,               (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2214 */  UNIT_CONV(constFactorK100Ktok100M, multiply,                 "E/hkm" STD_RIGHT_ARROW "E/100mi",                 "kWh/100km" STD_RIGHT_ARROW),
/* 2215 */  UNIT_CONV(constFactorK100Ktok100M,   divide,                 "E/100mi" STD_RIGHT_ARROW "E/hkm",                 "kWh/100mi" STD_RIGHT_ARROW),
/* 2216 */  { fnL100Tomguk,                 multiply,                    STD_litre "/hkm" STD_RIGHT_ARROW "mpg" STD_UK,     STD_litre "/100km" STD_RIGHT_ARROW,        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2217 */  { fnL100Tomguk,                 divide,                      "mpg" STD_UK STD_RIGHT_ARROW STD_litre "/hkm",     "mpg" STD_UK STD_RIGHT_ARROW,              (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2218 */  { fnMgeuktok100M,               multiply,                    "mge" STD_UK STD_RIGHT_ARROW "E/100mi",            "mge" STD_UK STD_RIGHT_ARROW,              (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2219 */  { fnMgeuktok100M,               divide,                      "E/100mi" STD_RIGHT_ARROW "mge" STD_UK,            "kWh/100mi" STD_RIGHT_ARROW,               (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2220 */  UNIT_CONV(constFactorK100Mtomik, invert | multiply,          "E/100mi" STD_RIGHT_ARROW "mi/E",                  "kWh/100mi" STD_RIGHT_ARROW),
/* 2221 */  UNIT_CONV(constFactorK100Mtomik, invert | multiply,          "mi/E" STD_RIGHT_ARROW "E/100mi",                  "mi/kWh" STD_RIGHT_ARROW),
/* 2222 */  { itemToBeCoded,                NOPARAM,                     "Ymmv:",                                       "Ymmv:",                                       (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2223 */  { fnExecutePlusSkip,            TM_LABEL,                    "XEQ.SKP",                                     "XEQ.SKP",                                     (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_LABEL        | HG_ENABLED         },
/* 2224 */  { fnRecallPlusSkip,             TM_STORCL,                   "RCL.SKP",                                     "RCL.SKP",                                     (0 << TAM_MAX_BITS) |    99, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_DISABLED        },
/* 2225 */  { itemToBeCoded,                NOPARAM,                     "",                                            "TamVarOnly",                                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2226 */  { itemToBeCoded,                NOPARAM,                     "",                                            "TamLblOnly",                                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2227 */  { itemToBeCoded,                NOPARAM,                     "CATe",                                        "CAT",                                         (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },// DL
/* 2228 */  { itemToBeCoded,                NOPARAM,                     "FCNSe",                                       "FCNS",                                        (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },// DL
/* 2229 */  { itemToBeCoded,                NOPARAM,                     "Prefix",                                      "PFX",                                         (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2230 */  { itemToBeCoded,                NOPARAM,                     "NUMBRS",                                      "NUMBRS",                                      (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2231 */  { itemToBeCoded,                NOPARAM,                     "CONFIGS",                                     "CONFIGS",                                     (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2232 */  { itemToBeCoded,                NOPARAM,                     "ALLVARS",                                     "ALL",                                         (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2233 */  { itemToBeCoded,                NOPARAM,                     "2233",                                        "2233",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2234 */  { itemToBeCoded,                NOPARAM,                     "RESETS",                                      "RESETS",                                      (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2235 */  { itemToBeCoded,                NOPARAM,                     "RIBBONS",                                     "RIBBONS",                                     (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },

/* 2236 */  { fnYYDflt,                     TM_VALUE_TRK,                ">YY<",                                        ">YY<",                                        (0 << TAM_MAX_BITS) |  9999, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_16    | HG_ENABLED         },
/* 2237 */  { fnYYDflt,                     YY_TRACKING,                 "YYtrack",                                     "YYtrack",                                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2238 */  { itemToBeCoded,                NOPARAM,                     "",                                            "TamNonRegTrk",                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2239 */  { fnClearUserMenus,             NOT_CONFIRMED,               "CLMall",                                      "CLMall",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABL_XEQ | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2240 */  { fnClearAllVariables,          NOT_CONFIRMED,               "CLVall",                                      "CLVall",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABL_XEQ | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2241 */  { fnDeleteUserMenus,            NOT_CONFIRMED,               "DELMall",                                     "DELMall",                                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABL_XEQ | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2242 */  { fnDeleteAllVariables,         NOT_CONFIRMED,               "DELVall",                                     "DELVall",                                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABL_XEQ | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2243 */  { itemToBeCoded,                NOPARAM,                     "DELETE",                                      "DELETE",                                      (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2244 */  { itemToBeCoded,                NOPARAM,                     "YESNO",                                       "YESNO",                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2245 */  { fnConfirmationYes,            NOPARAM,                     "YES",                                         "YES",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2246 */  { fnConfirmationNo,             NOPARAM,                     "NO",                                          "NO",                                          (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2247 */  { fnRecallVElement,             TM_VALUE,                    "RCLVEL",                                      "RCLVEL",                                      (0 << TAM_MAX_BITS) |   999, CAT_FNCT | SLS_UNCHANGED | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 2248 */  { fnStoreVElement,              TM_VALUE,                    "STOVEL",                                      "STOVEL",                                      (0 << TAM_MAX_BITS) |   999, CAT_FNCT | SLS_UNCHANGED | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 2249 */  { fnRecallElementPlus,          NOPARAM,                     "RCLSEQ",                                      "RCLSEQ",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2250 */  { fnStoreElementPlus,           NOPARAM,                     "STOSEQ",                                      "STOSEQ",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },

// 64 more system flags //!!!!!!! Remember to set NUMBER_OF_SYSTEM_FLAGS !
/* 2251 */  { fnGetSystemFlag,              FLAG_MONIT,                  "MONIT",                                       "MONIT",                                       (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2252 */  { fnGetSystemFlag,              FLAG_FRCYC,                  "FRCYC",                                       "FRCYC",                                       (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2253 */  { fnGetSystemFlag,              FLAG_HPCONV,                 "CONV" STD_SUB_H STD_SUB_P,                    "CONV" STD_SUB_H STD_SUB_P,                    (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2254 */  { fnGetSystemFlag,              FLAG_NUMLOCK,                "NUM",                                         "NUM",                                         (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2255 */  { fnGetSystemFlag,              FLAG_CPXMULT,                "CPXmul",                                      "CPXmul",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2256 */  { fnGetSystemFlag,              FLAG_ERPN,                   "eRPN",                                        "eRPN",                                        (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2257 */  { fnGetSystemFlag,              FLAG_LARGELI,                "dLrgLI",                                      "dLrgLI",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2258 */  { fnGetSystemFlag,              FLAG_IRFRAC,                 "IRFRAC",                                      "IRFRAC",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2259 */  { fnGetSystemFlag,              FLAG_IRFRQ,                  "IRFR?",                                       "IRFR?",                                       (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2260 */  { fnGetSystemFlag,              FLAG_PFX_ALL,                "PFX.All",                                     "PFX.All",                                     (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2261 */  { fnGetSystemFlag,              FLAG_DREAL,                  "d" STD_INTEGER_Z ".0",                        "d" STD_INTEGER_Z ".0",                        (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2262 */  { fnGetSystemFlag,              FLAG_CPXPLOT,                "PLcxp",                                       "PLcxp",                                       (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2263 */  { fnGetSystemFlag,              FLAG_SHOWX,                  "PLx.ax",                                      "PLx.ax",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2264 */  { fnGetSystemFlag,              FLAG_SHOWY,                  "PLy.ax",                                      "PLy.ax",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2265 */  { fnGetSystemFlag,              FLAG_PBOX,                   "PLbox",                                       "PLbox",                                       (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2266 */  { fnGetSystemFlag,              FLAG_PCROS,                  "PLcros",                                      "PLcros",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2267 */  { fnGetSystemFlag,              FLAG_PPLUS,                  "PLplus",                                      "PLplus",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2268 */  { fnGetSystemFlag,              FLAG_PLINE,                  "PLline",                                      "PLline",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2269 */  { fnGetSystemFlag,              FLAG_SCALE,                  "PLxy:1",                                      "PLxy:1",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2270 */  { fnGetSystemFlag,              FLAG_VECT,                   "PLvec",                                       "PLvec",                                       (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2271 */  { fnGetSystemFlag,              FLAG_NVECT,                  "PLnvec",                                      "PLnvec",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2272 */  { fnGetSystemFlag,              FLAG_US,                     "CONV" STD_SUB_U STD_SUB_S,                    "CONV" STD_SUB_U STD_SUB_S,                    (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2273 */  { fnGetSystemFlag,              FLAG_MNUp1,                  "MNUp1",                                       "MNUp1",                                       (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2274 */  { fnGetSystemFlag,              FLAG_SBwoy,                  "SBwoy",                                       "SBwoy",                                       (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2275 */  { fnGetSystemFlag,              FLAG_TOPHEX,                 "KEY" STD_SUB_A STD_SUB_MINUS STD_SUB_F,       "KEY" STD_SUB_A STD_SUB_MINUS STD_SUB_F,       (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2276 */  { fnGetSystemFlag,              FLAG_BCD,                    "BCD",                                         "BCD",                                         (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2277 */  { fnGetSystemFlag,              FLAG_PCURVE,                 "PLcurve",                                     "PLcurve",                                     (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2278 */  { fnGetSystemFlag,              FLAG_CLX_DROP,               STD_LEFT_ARROW "DROP",                         STD_LEFT_ARROW "DROP",                         (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2279 */  { fnGetSystemFlag,              FLAG_BASE_MYM,               "MyMb",                                        "MyMb",                                        (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2280 */  { fnGetSystemFlag,              FLAG_G_DOUBLETAP,            "g.2Tp",                                       "g.2Tp",                                       (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2281 */  { fnGetSystemFlag,              FLAG_BASE_HOME,              "HOMEb",                                       "HOMEb",                                       (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2282 */  { fnGetSystemFlag,              FLAG_MYM_TRIPLE,             "MyM" STD_fg,                                  "MyM" STD_fg,                                  (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2283 */  { fnGetSystemFlag,              FLAG_HOME_TRIPLE,            "HOME" STD_fg,                                 "HOME" STD_fg,                                 (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2284 */  { fnGetSystemFlag,              FLAG_SHFT_4s,                "SH.4s",                                       "SH.4s",                                       (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2284 */  { fnGetSystemFlag,              FLAG_FGLNLIM,                "fg.LIM",                                      "fg.LIM",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2284 */  { fnGetSystemFlag,              FLAG_FGLNFUL,                "fg.FUL",                                      "fg.FUL",                                      (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2284 */  { fnGetSystemFlag,              FLAG_FGGR   ,                "fg.GR",                                       "fg.GR",                                       (0 << TAM_MAX_BITS) |     0, CAT_SYFL | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2288 */  { itemToBeCoded,                NOPARAM,                     "2288",                                        "2288",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2289 */  { itemToBeCoded,                NOPARAM,                     "2289",                                        "2289",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2290 */  { itemToBeCoded,                NOPARAM,                     "2290",                                        "2290",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2291 */  { itemToBeCoded,                NOPARAM,                     "2291",                                        "2291",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2292 */  { itemToBeCoded,                NOPARAM,                     "2292",                                        "2292",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2293 */  { itemToBeCoded,                NOPARAM,                     "2293",                                        "2293",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2294 */  { itemToBeCoded,                NOPARAM,                     "2294",                                        "2294",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2295 */  { itemToBeCoded,                NOPARAM,                     "2295",                                        "2295",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2296 */  { itemToBeCoded,                NOPARAM,                     "2296",                                        "2296",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2297 */  { itemToBeCoded,                NOPARAM,                     "2297",                                        "2297",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2298 */  { itemToBeCoded,                NOPARAM,                     "2298",                                        "2298",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2299 */  { itemToBeCoded,                NOPARAM,                     "2299",                                        "2299",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2300 */  { itemToBeCoded,                NOPARAM,                     "2300",                                        "2300",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2301 */  { itemToBeCoded,                NOPARAM,                     "2301",                                        "2301",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2302 */  { itemToBeCoded,                NOPARAM,                     "2302",                                        "2302",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2303 */  { itemToBeCoded,                NOPARAM,                     "2303",                                        "2303",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2304 */  { itemToBeCoded,                NOPARAM,                     "2304",                                        "2304",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2305 */  { itemToBeCoded,                NOPARAM,                     "2305",                                        "2305",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2306 */  { itemToBeCoded,                NOPARAM,                     "2306",                                        "2306",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2307 */  { itemToBeCoded,                NOPARAM,                     "2307",                                        "2307",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2308 */  { itemToBeCoded,                NOPARAM,                     "2308",                                        "2308",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2309 */  { itemToBeCoded,                NOPARAM,                     "2309",                                        "2309",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2310 */  { itemToBeCoded,                NOPARAM,                     "2310",                                        "2310",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2311 */  { itemToBeCoded,                NOPARAM,                     "2311",                                        "2311",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2312 */  { itemToBeCoded,                NOPARAM,                     "2312",                                        "2312",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2313 */  { itemToBeCoded,                NOPARAM,                     "2313",                                        "2313",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2314 */  { itemToBeCoded,                NOPARAM,                     "2314",                                        "2314",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2315 */  { itemToBeCoded,                NOPARAM,                     "SHOW",                                        "SHOW",                                        (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2316 */  { fnStore,                      REGISTER_P,                  "STO" STD_SUB_P " p",                          "STO" STD_SUB_P " p",                          (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2317 */  { fnStore,                      REGISTER_N,                  "STO" STD_SUB_N " n",                          "STO" STD_SUB_N " n",                          (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2318 */  { fnStore,                      REGISTER_M,                  "STO" STD_SUB_M " x" STD_SUB_0,                "STO" STD_SUB_M " x" STD_SUB_0,                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2319 */  { fnStore,                      REGISTER_S,                  "STO" STD_SUB_S " " STD_gamma,                 "STO" STD_SUB_S " " STD_gamma,                 (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2320 */  { fnStore,                      REGISTER_M,                  "STO" STD_SUB_M " " STD_nu,                    "STO" STD_SUB_M " " STD_nu,                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2321 */  { fnStore,                      REGISTER_R,                  "STO" STD_SUB_R " " STD_lambda,                "STO" STD_SUB_R " " STD_lambda,                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2322 */  { fnStore,                      REGISTER_M,                  "STO" STD_SUB_M " d" STD_SUB_1,                "STO" STD_SUB_M " d" STD_SUB_1,                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2323 */  { fnStore,                      REGISTER_N,                  "STO" STD_SUB_N " d" STD_SUB_2,                "STO" STD_SUB_N " d" STD_SUB_2,                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2324 */  { fnStore,                      REGISTER_M,                  "STO" STD_SUB_M " N",                          "STO" STD_SUB_M " N",                          (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2325 */  { fnStore,                      REGISTER_Q,                  "STO" STD_SUB_Q " K",                          "STO" STD_SUB_Q " K",                          (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2326 */  { fnStore,                      REGISTER_M,                  "STO" STD_SUB_M " " STD_mu,                    "STO" STD_SUB_M " " STD_mu,                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2327 */  { fnStore,                      REGISTER_S,                  "STO" STD_SUB_S " " STD_sigma,                 "STO" STD_SUB_S " " STD_sigma,                 (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2328 */  { fnStore,                      REGISTER_S,                  "STO" STD_SUB_S " s",                          "STO" STD_SUB_S " s",                          (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2329 */  { fnStore,                      REGISTER_Q,                  "STO" STD_SUB_Q " " STD_alpha,                 "STO" STD_SUB_Q " " STD_alpha,                 (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2330 */  { fnStore,                      REGISTER_Q,                  "STO" STD_SUB_Q " " STD_xi,                    "STO" STD_SUB_Q " " STD_xi,                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2331 */  { fnStore,                      REGISTER_Q,                  "STO" STD_SUB_Q " " STD_k,                     "STO" STD_SUB_Q " " STD_k,                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2332 */  { fnStore,                      REGISTER_S,                  "STO" STD_SUB_S " " STD_lambda,                "STO" STD_SUB_S " " STD_lambda,                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2333 */  { fnStore,                      REGISTER_M,                  "STO" STD_SUB_M " a",                          "STO" STD_SUB_M " a",                          (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2334 */  { fnStore,                      REGISTER_N,                  "STO" STD_SUB_N " b",                          "STO" STD_SUB_N " b",                          (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2335 */  { itemToBeCoded,                NOPARAM,                     "2335",                                        "2335",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2336 */  { addItemToBuffer,              REGISTER_M,                  "M",                                           "M",                                           (0 << TAM_MAX_BITS) |     0, CAT_REGS | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // The
/* 2337 */  { addItemToBuffer,              REGISTER_N,                  "N",                                           "N",                                           (0 << TAM_MAX_BITS) |     0, CAT_REGS | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // items
/* 2338 */  { addItemToBuffer,              REGISTER_P,                  "P",                                           "P",                                           (0 << TAM_MAX_BITS) |     0, CAT_REGS | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // from
/* 2339 */  { addItemToBuffer,              REGISTER_Q,                  "Q",                                           "Q",                                           (0 << TAM_MAX_BITS) |     0, CAT_REGS | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // REGISTER_M
/* 2340 */  { addItemToBuffer,              REGISTER_R,                  "R",                                           "R",                                           (0 << TAM_MAX_BITS) |     0, CAT_REGS | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // to
/* 2341 */  { addItemToBuffer,              REGISTER_S,                  "S",                                           "S",                                           (0 << TAM_MAX_BITS) |     0, CAT_REGS | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // REGISTER_W
/* 2342 */  { addItemToBuffer,              REGISTER_E,                  "E",                                           "E",                                           (0 << TAM_MAX_BITS) |     0, CAT_REGS | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // must
/* 2343 */  { addItemToBuffer,              REGISTER_F,                  "F",                                           "F",                                           (0 << TAM_MAX_BITS) |     0, CAT_REGS | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // stay
/* 2344 */  { addItemToBuffer,              REGISTER_G,                  "G",                                           "G",                                           (0 << TAM_MAX_BITS) |     0, CAT_REGS | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // in
/* 2345 */  { addItemToBuffer,              REGISTER_H,                  "H",                                           "H",                                           (0 << TAM_MAX_BITS) |     0, CAT_REGS | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // the
/* 2346 */  { addItemToBuffer,              REGISTER_O,                  "O",                                           "O",                                           (0 << TAM_MAX_BITS) |     0, CAT_REGS | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // same
/* 2347 */  { addItemToBuffer,              REGISTER_U,                  "U",                                           "U",                                           (0 << TAM_MAX_BITS) |     0, CAT_REGS | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // order
/* 2348 */  { addItemToBuffer,              REGISTER_V,                  "V",                                           "V",                                           (0 << TAM_MAX_BITS) |     0, CAT_REGS | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // !
/* 2349 */  { addItemToBuffer,              REGISTER_W,                  "W",                                           "W",                                           (0 << TAM_MAX_BITS) |     0, CAT_REGS | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // !
/* 2350 */  { addItemToBuffer,              REGISTER_M,                  "M",                                           "M",                                           (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // The
/* 2351 */  { addItemToBuffer,              REGISTER_N,                  "N",                                           "N",                                           (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // items
/* 2352 */  { addItemToBuffer,              REGISTER_P,                  "P",                                           "P",                                           (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // from
/* 2353 */  { addItemToBuffer,              REGISTER_Q,                  "Q",                                           "Q",                                           (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // REGISTER_M
/* 2354 */  { addItemToBuffer,              REGISTER_R,                  "R",                                           "R",                                           (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // to
/* 2355 */  { addItemToBuffer,              REGISTER_S,                  "S",                                           "S",                                           (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // REGISTER_W
/* 2356 */  { addItemToBuffer,              REGISTER_E,                  "E",                                           "E",                                           (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // must
/* 2357 */  { addItemToBuffer,              REGISTER_F,                  "F",                                           "F",                                           (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // stay
/* 2358 */  { addItemToBuffer,              REGISTER_G,                  "G",                                           "G",                                           (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // in
/* 2359 */  { addItemToBuffer,              REGISTER_H,                  "H",                                           "H",                                           (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // the
/* 2360 */  { addItemToBuffer,              REGISTER_O,                  "O",                                           "O",                                           (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // same
/* 2361 */  { addItemToBuffer,              REGISTER_U,                  "U",                                           "U",                                           (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // order
/* 2362 */  { addItemToBuffer,              REGISTER_V,                  "V",                                           "V",                                           (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // !
/* 2363 */  { addItemToBuffer,              REGISTER_W,                  "W",                                           "W",                                           (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         }, // !
/* 2364 */  { fnInsCol,                     NOPARAM,                     "M.INSC",                                      "INSC",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2365 */  { fnDelCol,                     NOPARAM,                     "M.DELC",                                      "DELC",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2366 */  { fnAddRow,                     NOPARAM,                     "M.ROW+1",                                     "ROW+1",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2367 */  { fnAddCol,                     NOPARAM,                     "M.COL+1",                                     "COL+1",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2368 */  { fnEqSolvGraph,                EQ_REALSOLVE,                "realSlv" STD_YX,                              "realSlv" STD_YX,                              (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2369 */  { fnEqSolvGraph,                EQ_REALSOLVE_LU,             "realSlv",                                     "realSlv",                                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2370 */  { fnEqSolvGraph,                EQ_CPXSOLVE,                 "cpxSlv" STD_YX,                               "cpxSlv" STD_YX,                               (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2371 */  { fnEqSolvGraph,                EQ_CPXSOLVE_LU,              "cpxSlv",                                      "cpxSlv",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2372 */  { fnEqSolvGraph,                EQ_PLOT,                     "Draw" STD_YX,                                 "Draw" STD_YX,                                 (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2373 */  { fnEqSolvGraph,                EQ_PLOT_LU,                  "Draw",                                        "Draw",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2374 */  { itemToBeCoded,                NOPARAM,                     "Graphs",                                      "Graphs",                                      (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2375 */  { itemToBeCoded,                NOPARAM,                     "Tool" STD_INTEGRAL,                           "Tool" STD_INTEGRAL,                           (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2376 */  { itemToBeCoded,                NOPARAM,                     "ToolS",                                       "ToolS",                                       (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2377 */  { fn1stDerivEq,                 NOPARAM,                     "Calc f'",                                     "Calc f'",                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2378 */  { fn2ndDerivEq,                 NOPARAM,                     "Calc f\"",                                    "Calc f\"",                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2379 */  { fnTvmVar,                     RESERVED_VARIABLE_CPERONA,   "cp/a",                                        "cp/a",                                        (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2380 */  { fnOldItemError,               NOPARAM,                     ">cp/a<",                                      ">cp/a<",                                      (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_DISABLED        },
/* 2381 */  { itemToBeCoded,                NOPARAM,                     "CASHFL",                                      "CASHFL",                                      (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2382 */  { itemToBeCoded,                NOPARAM,                     "AMORT",                                       "AMORT",                                       (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2383 */  { addItemToBuffer,              ITM_x_SIGN,                  "",                                            STD_x,                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2384 */  { fnComplexPlot,                NOPARAM,                     "CXPLT",                                       "CXPLT",                                       (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//GRAPH
/* 2385 */  { fnEvPFacts,                   M_FACTORS,                   "M.FACT",                                      "M.FACT",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2386 */  { fnEvPFacts,                   M_PHI_EUL,                   STD_phi_m STD_SUB_E,                           STD_phi_m STD_SUB_E,                           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2387 */  { fnSave,                       SM_STATE_SAVE,               "SAVEST",                                      "SAVEST",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2388 */  { fnLoad,                       LM_STATE_LOAD,               "LOADST",                                      "LOADST",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_CANCEL    | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2389 */  { fnSaveAuto,                   NOPARAM,                     "SAVEAUT",                                     "SAVEAUT",                                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2390 */  { itemToBeCoded,                NOPARAM,                     "AUDIO",                                       "AUDIO",                                       (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2391 */  { fnKeysManagement,             USER_R47f_g,                 "f g",                                         "f g",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2392 */  { fnKeysManagement,             USER_R47bk_fg,               STD_BOX " " STD_fg,                            STD_BOX " " STD_fg,                            (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2393 */  { fnKeysManagement,             USER_R47fg_bk,               STD_fg " " STD_BOX,                            STD_fg " " STD_BOX,                            (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2394 */  { fnKeysManagement,             USER_R47fg_g,                STD_fg " " "g",                                STD_fg " " "g",                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2395 */  { itemToBeCoded,                NOPARAM,                     "2395",                                        "2395",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2396 */  { fnCheckAngle,                 NOPARAM,                     "ANGLE?",                                      "ANGLE?",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2397 */  { fnCheckType,                  dtDate,                      STD_DATE_D "?",                                STD_DATE_D "?",                                (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2398 */  { fnCheckType,                  dtLongInteger,               "LINT?",                                       "LINT?",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2399 */  { fnCheckNumber,                NOPARAM,                     "NUMBR?",                                      "NUMBR?",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2400 */  { fnCheckType,                  dtShortInteger,              "SINT?",                                       "SINT?",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2401 */  { fnCheckType,                  dtTime,                      STD_TIME_T "?",                                STD_TIME_T "?",                                (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2402 */  { fnGetType,                    NOPARAM,                     "TYPE?",                                       "TYPE?",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2403 */  { itemToBeCoded,                NOPARAM,                     "P.FN3",                                       "P.FN3",                                       (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2404 */  { fnEdit,                       NOPARAM,                     "EDIT",                                        "EDIT",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_DISABLED        },
/* 2405 */  { fnOpenMenu,                   TM_MENU,                     "OPENM",                                       "OPENM",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_CANCEL    | EIM_DISABLED | PTP_MENU         | HG_ENABLED         },
/* 2406 */  { itemToBeCoded,                NOPARAM,                     "",                                            "TamMenu",                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2407 */  { itemToBeCoded,                NOPARAM,                     "MENU",                                        "MENU",                                        (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2408 */  { fnGetMenu,                    NOPARAM,                     "MENU?",                                       "MENU?",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2409 */  { fnEvPFacts,                   M_SIGMA_0,                   STD_sigma STD_SUB_0,                           STD_sigma STD_SUB_0,                           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2410 */  { fnEvPFacts,                   M_SIGMA_1,                   STD_sigma STD_SUB_1,                           STD_sigma STD_SUB_1,                           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2411 */  { fnEvPFacts,                   M_SIGMA_k,                   STD_sigma STD_SUB_k,                           STD_sigma STD_SUB_k,                           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2412 */  { fnEvPFacts,                   M_SIGMA_p1,                  STD_sigma STD_SUB_1 STD_SUP_ASTERISK,          STD_sigma STD_SUB_1 STD_SUP_ASTERISK,          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2413 */  { fnEvPFacts,                   M_SIGMA_pk,                  STD_sigma STD_SUP_ASTERISK STD_SPACE_HAIR STD_SUB_k,  STD_sigma STD_SUP_ASTERISK STD_SPACE_HAIR STD_SUB_k,(0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2414 */  { itemToBeCoded,                NOPARAM,                     "NumTh",                                       "NumTh",                                       (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2415 */  { addItemToBuffer,              REGISTER_X,                  STD_RIGHT_ARROW "X",                           STD_RIGHT_ARROW "X",                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2416 */  { addItemToBuffer,              REGISTER_Z,                  STD_RIGHT_ARROW "Y",                           STD_RIGHT_ARROW "Y",                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2417 */  { addItemToBuffer,              REGISTER_Y,                  STD_RIGHT_ARROW "Z",                           STD_RIGHT_ARROW "Z",                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2418 */  { addItemToBuffer,              REGISTER_T,                  STD_RIGHT_ARROW "T",                           STD_RIGHT_ARROW "T",                           (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2419 */  { fnXSWAP,                      XSWAP,                       "X.SWAP",                                      "X.SWAP",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2420 */  { fnXSWAP,                      XEDIT,                       "X.EDIT",                                      "X.EDIT",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },

//more bufferized
/* 2421 */  { addItemToBuffer,              ITM_SUP_T,                   "",                                            STD_SUP_BOLD_T,                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2422 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_BOLD_f,                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2423 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_BOLD_g,                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2424 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_BOLD_h,                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2425 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_BOLD_r,                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2426 */  { itemToBeCoded,                NOPARAM,                     "",                                            STD_SUP_BOLD_x,                                (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2427 */  { addItemToBuffer,              ITM_HOLLOW_UP_ARROW  ,       "",                                            STD_HOLLOW_UP_ARROW  ,                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2428 */  { addItemToBuffer,              ITM_HOLLOW_DOWN_ARROW,       "",                                            STD_HOLLOW_DOWN_ARROW,                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2429 */  { addItemToBuffer,              ITM_SPHERICAL_ANGLE  ,       "",                                            STD_SPHERICAL_ANGLE  ,                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2430 */  { addItemToBuffer,              ITM_TRIPLE_INTEGRAL  ,       "",                                            STD_TRIPLE_INTEGRAL  ,                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2431 */  { addItemToBuffer,              ITM_VOLUME_INTEGRAL  ,       "",                                            STD_VOLUME_INTEGRAL  ,                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2432 */  { addItemToBuffer,              ITM_LEFT_BLOCKARROW  ,       "",                                            STD_LEFT_BLOCKARROW  ,                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2433 */  { addItemToBuffer,              ITM_UP_BLOCKARROW    ,       "",                                            STD_UP_BLOCKARROW    ,                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2434 */  { addItemToBuffer,              ITM_RIGHT_BLOCKARROW ,       "",                                            STD_RIGHT_BLOCKARROW ,                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2435 */  { addItemToBuffer,              ITM_DOWN_BLOCKARROW  ,       "",                                            STD_DOWN_BLOCKARROW  ,                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2436 */  { addItemToBuffer,              ITM_POWER_SYMBOL     ,       "",                                            STD_POWER_SYMBOL     ,                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2437 */  { addItemToBuffer,              ITM_u_BAR  ,                 "",                                            STD_u_BAR  ,                                   (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2438 */  { addItemToBuffer,              ITM_v_BAR  ,                 "",                                            STD_v_BAR  ,                                   (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2439 */  { addItemToBuffer,              ITM_w_BAR  ,                 "",                                            STD_w_BAR  ,                                   (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2440 */  { addItemToBuffer,              ITM_z_BAR  ,                 "",                                            STD_z_BAR  ,                                   (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2441 */  { addItemToBuffer,              ITM_v_CIRC ,                 "",                                            STD_v_CIRC ,                                   (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2442 */  { addItemToBuffer,              ITM_z_CIRC ,                 "",                                            STD_z_CIRC ,                                   (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2443 */  { addItemToBuffer,              ITM_u_CIRC2,                 "",                                            STD_u_CIRC2,                                   (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2444 */  { addItemToBuffer,              ITM_theta_m,                 "",                                            STD_theta_m,                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2445 */  { addItemToBuffer,              ITM_j_CIRC,                  "",                                            STD_j_CIRC,                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2446 */  { addItemToBuffer,              ITM_k_CIRC,                  "",                                            STD_k_CIRC,                                    (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2447 */  { addItemToBuffer,              ITM_phi_m,                   "",                                            STD_phi_m,                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },

/* 2448 */  { itemToBeCoded,                NOPARAM,                     "2448",                                        "2448",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2449 */  { itemToBeCoded,                NOPARAM,                     "2449",                                        "2449",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2450 */  { itemToBeCoded,                NOPARAM,                     "2450",                                        "2450",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2451 */  { itemToBeCoded,                NOPARAM,                     "2451",                                        "2451",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2452 */  { itemToBeCoded,                NOPARAM,                     "2452",                                        "2452",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2453 */  { itemToBeCoded,                NOPARAM,                     "2453",                                        "2453",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2454 */  { itemToBeCoded,                NOPARAM,                     "2454",                                        "2454",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2455 */  { itemToBeCoded,                NOPARAM,                     "2455",                                        "2455",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2456 */  { itemToBeCoded,                NOPARAM,                     "2456",                                        "2456",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2457 */  { itemToBeCoded,                NOPARAM,                     "2457",                                        "2457",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2458 */  { itemToBeCoded,                NOPARAM,                     "2458",                                        "2458",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2459 */  { itemToBeCoded,                NOPARAM,                     "2459",                                        "2459",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2460 */  { itemToBeCoded,                NOPARAM,                     "2460",                                        "2460",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2461 */  { itemToBeCoded,                NOPARAM,                     "2461",                                        "2461",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2462 */  { itemToBeCoded,                NOPARAM,                     "2462",                                        "2462",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2463 */  { itemToBeCoded,                NOPARAM,                     "2463",                                        "2463",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },

/* 2464 */  UNIT_CONV(constFactoreVJ,      multiply,                     "eV" STD_RIGHT_ARROW "J" ,                     "eV"  STD_RIGHT_ARROW                   ),
/* 2465 */  UNIT_CONV(constFactorJeV,        divide,                     "J"  STD_RIGHT_ARROW "eV",                     "J" STD_RIGHT_ARROW                     ),
/* 2466 */  UNIT_CONV(constFactorBananamm, multiply,                     "Banana"  STD_RIGHT_ARROW "mm",                "Banana" STD_RIGHT_ARROW                ),
/* 2467 */  UNIT_CONV(constFactormmBanana,  divide,                      "mm"  STD_RIGHT_ARROW "Banana",                "mm" STD_RIGHT_ARROW                    ),
/* 2468 */  UNIT_CONV(constFactorBananaInch,multiply,                    "Banana"  STD_RIGHT_ARROW "inch",              "Banana" STD_RIGHT_ARROW                ),
/* 2469 */  UNIT_CONV(constFactorInchBanana,divide,                      "inch"  STD_RIGHT_ARROW "Banana",              "inch" STD_RIGHT_ARROW                  ),

/* 2470 */  { /*V3RectoToCyl*/itemToBeCoded,                     255,                     STD_RIGHT_ARROW "CYL",                         STD_RIGHT_ARROW "CYL",        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2471 */  { /*V3RectoToSph*/itemToBeCoded,                     255,                     STD_RIGHT_ARROW "SPH",                         STD_RIGHT_ARROW "SPH",        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2472 */  { fnVectorDist,                 NOPARAM,                     "|" STD_SPACE_6_PER_EM STD_v_BAR "-" STD_v_BAR STD_SPACE_6_PER_EM "|",     "|" STD_SPACE_6_PER_EM STD_v_BAR "-" STD_v_BAR STD_SPACE_6_PER_EM "|", (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2473 */  { itemToBeCoded,                NOPARAM,                     "M.CONCT",                                     "CONCAT",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2474 */  { fnConvertStkToMx,                   2,                     "zyx" STD_RIGHT_ARROW STD_v_BAR STD_SUB_3,     "zyx" STD_RIGHT_ARROW STD_v_BAR STD_SUB_3,     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2475 */  { fnConvertStkToMx,                   6,                     "yx"  STD_RIGHT_ARROW  STD_v_BAR STD_SUB_2,    "yx"  STD_RIGHT_ARROW STD_v_BAR STD_SUB_2,     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2476 */  { /*fnConvertMxToStk*/itemToBeCoded,                   2,                     STD_v_BAR STD_SUB_3 STD_RIGHT_ARROW "zyx",STD_v_BAR STD_SUB_3 STD_RIGHT_ARROW "zyx",     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2477 */  { /*fnConvertMxToStk*/itemToBeCoded,                   6,                     STD_v_BAR STD_SUB_2 STD_RIGHT_ARROW "yx", STD_v_BAR STD_SUB_2 STD_RIGHT_ARROW "yx",      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2478 */  { /*fnConvertMxToStk*/itemToBeCoded,                0x62,                     STD_v_BAR STD_RIGHT_ARROW "stk",          STD_v_BAR STD_RIGHT_ARROW "stk",               (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2479 */  { fnConvertStkToMx,                   3,                     STD_v_BAR STD_SUB_3 "[100]",                   STD_v_BAR STD_SUB_3 "[100]",                   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2480 */  { fnConvertStkToMx,                   4,                     STD_v_BAR STD_SUB_3 "[010]",                   STD_v_BAR STD_SUB_3 "[010]",                   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2481 */  { fnConvertStkToMx,                   5,                     STD_v_BAR STD_SUB_3 "[001]",                   STD_v_BAR STD_SUB_3 "[001]",                   (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2482 */  { fnRecallVElement,                   1,                     "RCLVEL" STD_SUB_1,                            "RCLVEL" STD_SUB_1,                            (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2483 */  { fnRecallVElement,                   2,                     "RCLVEL" STD_SUB_2,                            "RCLVEL" STD_SUB_2,                            (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2484 */  { fnRecallVElement,                   3,                     "RCLVEL" STD_SUB_3,                            "RCLVEL" STD_SUB_3,                            (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2485 */  { fnStoreVElement,                    1,                     "STOVEL" STD_SUB_1,                            "STOVEL" STD_SUB_1,                            (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2486 */  { fnStoreVElement,                    2,                     "STOVEL" STD_SUB_2,                            "STOVEL" STD_SUB_2,                            (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2487 */  { fnStoreVElement,                    3,                     "STOVEL" STD_SUB_3,                            "STOVEL" STD_SUB_3,                            (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2488 */  { itemToBeCoded,                NOPARAM,                     "2488",                                        "2488",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2489 */  { itemToBeCoded,                NOPARAM,                     "2489",                                        "2489",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2490 */  { fnConvertStkToMx,                   7,                     STD_v_BAR STD_SUB_2 "[10]",                    STD_v_BAR STD_SUB_2 "[10]",                    (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2491 */  { fnConvertStkToMx,                   8,                     STD_v_BAR STD_SUB_2 "[01]",                    STD_v_BAR STD_SUB_2 "[01]",                    (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2492 */  { /*fnComplexToVector*/itemToBeCoded,            NOPARAM,                     STD_COMPLEX_C STD_LEFT_RIGHT_ARROWS STD_SPACE_6_PER_EM STD_v_BAR STD_SUB_2,STD_COMPLEX_C STD_LEFT_RIGHT_ARROWS STD_SPACE_6_PER_EM STD_v_BAR STD_SUB_2, (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2493 */  { itemToBeCoded,                NOPARAM,                     STD_COMPLEX_C STD_RIGHT_ARROW STD_SPACE_6_PER_EM STD_v_BAR STD_SUB_2,      STD_COMPLEX_C STD_RIGHT_ARROW STD_SPACE_6_PER_EM STD_v_BAR STD_SUB_2,       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2494 */  { itemToBeCoded,                NOPARAM,                     STD_v_BAR STD_SUB_2 STD_RIGHT_ARROW STD_COMPLEX_C,                         STD_v_BAR STD_SUB_2 STD_RIGHT_ARROW STD_COMPLEX_C,                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2495 */  { itemToBeCoded,                NOPARAM,                     "yx" STD_LEFT_RIGHT_ARROWS STD_SPACE_6_PER_EM STD_v_BAR STD_SUB_2,         "yx" STD_LEFT_RIGHT_ARROWS STD_SPACE_6_PER_EM STD_v_BAR STD_SUB_2,          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2496 */  { itemToBeCoded,                NOPARAM,                     "zyx" STD_LEFT_RIGHT_ARROWS STD_SPACE_6_PER_EM STD_v_BAR STD_SUB_3,        "zyx" STD_LEFT_RIGHT_ARROWS STD_SPACE_6_PER_EM STD_v_BAR STD_SUB_3,         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2497 */  { itemToBeCoded,                NOPARAM,                     "R" STD_SUB_n STD_RIGHT_ARROW "V",                                         "R" STD_SUB_n STD_RIGHT_ARROW "V",                                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2498 */  { itemToBeCoded,                NOPARAM,                     "V" STD_RIGHT_ARROW "R" STD_SUB_n,                                         "V" STD_RIGHT_ARROW "R" STD_SUB_n,                                          (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2499 */  { itemToBeCoded,                NOPARAM,                     STD_v_BAR STD_LEFT_ARROW STD_RIGHT_ARROW STD_SPACE_6_PER_EM STD_v_BAR,     STD_v_BAR STD_LEFT_ARROW STD_RIGHT_ARROW STD_SPACE_6_PER_EM STD_v_BAR,      (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },

/* 2500 */  { fnPseudoMenu,                 (2<<14) + MNU_CLK,           "CLK" STD_CR "2",                              DT TM "SET",                                   (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2501 */  { fnSetWeekOfYearRule,          ITM_WOY_ISO,                 "WOY" STD_SUB_I STD_SUB_S STD_SUB_O,           "WOY" STD_SUB_I STD_SUB_S STD_SUB_O,           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2502 */  { fnSetWeekOfYearRule,          ITM_WOY_US,                  "WOY" STD_SUB_U STD_SUB_S,                     "WOY" STD_SUB_U STD_SUB_S,                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2503 */  { fnSetWeekOfYearRule,          ITM_WOY_ME,                  "WOY" STD_SUB_M STD_SUB_E,                     "WOY" STD_SUB_M STD_SUB_E,                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2504 */  { fnGetWeekOfYearRule,          NOPARAM,                     "WOY?",                                        "WOY?",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2505 */  { fnWeekOfYear,                 NOPARAM,                     "WOY",                                         "WOY",                                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },

/* 2506 */  { fnKeysManagement,      ITM_RIBBON_CPX,                     "M.CPX",                                       "M.CPX",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2507 */  { fnKeysManagement,      ITM_RIBBON_FIN,                     "M.FIN",                                       "M.FIN",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2508 */  { fnKeysManagement,      ITM_RIBBON_SAV,                     "M.SAV",                                       "M.SAV",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2509 */  { fnKeysManagement,      ITM_RIBBON_C47,                     "M.C47",                                       "M.C47",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2510 */  { fnKeysManagement,    ITM_RIBBON_C47PL,                     "M.C47+",                                      "M.C47+",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2511 */  { fnKeysManagement,      ITM_RIBBON_R47,                     "M.R47",                                       "M.R47",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2512 */  { fnKeysManagement,    ITM_RIBBON_R47PL,                     "M.R47+",                                      "M.R47+",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2513 */  { fnKeysManagement,      ITM_RIBBON_ENG,                     "M.ENG",                                       "M.ENG",                                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2514 */  { fnKeysManagement,     ITM_RIBBON_FIN2,                     "M.FIN+",                                      "M.FIN+",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2515 */  { fnKeysManagement,     ITM_RIBBON_SAV2,                     "M.SAV+",                                      "M.SAV+",                                      (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2516 */  { itemToBeCoded,                NOPARAM,                     "M.2516",                                      "M.2516",                                      (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2517 */  { itemToBeCoded,                NOPARAM,                     "M.2517",                                      "M.2517",                                      (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2518 */  { itemToBeCoded,                NOPARAM,                     "M.2518",                                      "M.2518",                                      (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2519 */  { itemToBeCoded,                NOPARAM,                     "M.2519",                                      "M.2519",                                      (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2520 */  { itemToBeCoded,                NOPARAM,                     "M.2520",                                      "M.2520",                                      (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2521 */  { itemToBeCoded,                NOPARAM,                     "M.2521",                                      "M.2521",                                      (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2522 */  { itemToBeCoded,                NOPARAM,                     "M.2522",                                      "M.2522",                                      (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2523 */  { itemToBeCoded,                NOPARAM,                     "M.2523",                                      "M.2523",                                      (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },

/* 2524 */  { fnCheckType,                  dtReal34Matrix,              STD_REAL_R "matx?",                            STD_REAL_R "matx?",                            (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2525 */  { fnCheckType,                  dtComplex34Matrix,           STD_COMPLEX_C "matx?",                         STD_COMPLEX_C "matx?",                         (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2526 */  { fnCheckType,                  dtConfig,                    "Config?",                                     "Config?",                                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2527 */  { fnCheckReIsZero,              NOPARAM,                     "Re" STD_EQUAL "0?",                           "Re" STD_EQUAL "0?",                           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2528 */  { fnCheckImIsZero,              NOPARAM,                     "Im" STD_EQUAL "0?",                           "Im" STD_EQUAL "0?",                           (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2529 */  { fnCheckReNotZero,             NOPARAM,                     "Re" STD_NOT_EQUAL "0?",                       "Re" STD_NOT_EQUAL "0?",                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2530 */  { fnCheckImNotZero,             NOPARAM,                     "Im" STD_NOT_EQUAL "0?",                       "Im" STD_NOT_EQUAL "0?",                       (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2531 */  { fnCheckIsVect2d,              NOPARAM,                     "VECT2D?",                                     "VECT2D?",                                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2532 */  { fnCheckIsVect3d,              NOPARAM,                     "VECT3D?",                                     "VECT3D?",                                     (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2533 */  { fnMultiplySI,                 210,                         STD_DOT "Ki",                                  STD_DOT "Ki",                                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM PRE UNIT
/* 2534 */  { fnMultiplySI,                 220,                         STD_DOT "Mi",                                  STD_DOT "Mi",                                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM PRE UNIT
/* 2535 */  { fnMultiplySI,                 230,                         STD_DOT "Gi",                                  STD_DOT "Gi",                                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM PRE UNIT
/* 2536 */  { fnMultiplySI,                 240,                         STD_DOT "Ti",                                  STD_DOT "Ti",                                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM PRE UNIT
/* 2537 */  { fnMultiplySI,                 250,                         STD_DOT "Pi",                                  STD_DOT "Pi",                                  (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },//JM PRE UNIT

/* 2538 */  { itemToBeCoded,                NOPARAM,                     STD_alpha "LTRIM",                             STD_alpha "LTRIM",                              (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2539 */  { itemToBeCoded,                NOPARAM,                     STD_alpha "RTRIM",                             STD_alpha "RTRIM",                              (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2540 */  { itemToBeCoded,                NOPARAM,                     STD_alpha "MID",                               STD_alpha "MID",                                (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2541 */  { itemToBeCoded,                NOPARAM,                     STD_alpha "LEFT",                              STD_alpha "LEFT",                               (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2542 */  { itemToBeCoded,                NOPARAM,                     STD_alpha "RIGHT",                             STD_alpha "RIGHT",                              (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2543 */  { itemToBeCoded,                NOPARAM,                     STD_alpha "LOWER",                             STD_alpha "LOWER",                              (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2544 */  { itemToBeCoded,                NOPARAM,                     STD_alpha "UPPER",                             STD_alpha "UPPER",                              (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },

/* 2545 */  { fnStore,                      RESERVED_VARIABLE_UEST,      STD_UP_ARROW "Est",                            STD_UP_ARROW "Est",                            (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2546 */  { fnStore,                      RESERVED_VARIABLE_LEST,      STD_DOWN_ARROW "Est",                          STD_DOWN_ARROW "Est",                          (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2547 */  { fnStore,                      RESERVED_VARIABLE_UY,        STD_UP_ARROW "Y",                              STD_UP_ARROW "Y",                              (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2548 */  { fnStore,                      RESERVED_VARIABLE_LY,        STD_DOWN_ARROW "Y",                            STD_DOWN_ARROW "Y",                            (0 << TAM_MAX_BITS) |     0, CAT_RVAR | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },

/* 2549 */  { fnRange,                      TM_VALUE,                    "RNG",                                         "RNG",                                         (0 << TAM_MAX_BITS) |  6145, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_16    | HG_ENABLED         },
/* 2550 */  { fnYYDflt,                     TM_VALUE_TRK,                "YY",                                          "YY",                                          (0 << TAM_MAX_BITS) |  9999, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_16    | HG_ENABLED         },
/* 2551 */  { fnDenMax,                     TM_VALUE_MAX,                "DMX",                                         "DMX",                                         (0 << TAM_MAX_BITS) |MAX_DENMAX,CAT_FNCT|SLS_ENABLED  | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_16    | HG_ENABLED         },
/* 2552 */  { itemToBeCoded,                NOPARAM,                     "CAT" STD_alpha,                               "CAT",                                         (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },// JM
/* 2553 */  { fnPseudoMenu,                 (2<<14) + MNU_BITS,          "BITS" STD_CR "2",                             "BITSHFT",                                     (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },


/* 2554 */  { S18_fnXXfn,               ITM_DEG2_XFN,                    "X" STD_RIGHT_DOUBLE_ARROW "DEG",              "X" STD_RIGHT_DOUBLE_ARROW "DEG",              (0 << TAM_MAX_BITS) |     0, S18_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2555 */  { S18_fnXXfn,               ITM_RAD2_XFN,                    "X" STD_RIGHT_DOUBLE_ARROW "RAD",              "X" STD_RIGHT_DOUBLE_ARROW "RAD",              (0 << TAM_MAX_BITS) |     0, S18_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2556 */  { S18_fnXXfn_RDP,               TM_VALUE,                    "XRDP",                                        "XRDP",                                        (0 << TAM_MAX_BITS) |  1034, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_16    | HG_ENABLED         },
/* 2557 */  { S18_fnXXfn_RSD,               TM_VALUE,                    "XRSD",                                        "XRSD",                                        (0 << TAM_MAX_BITS) |  1034, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_16    | HG_ENABLED         },
/* 2558 */  { S18_fnXXfn,                ITM_sin_XFN,                    "XSIN",                                        "XSIN",                                        (0 << TAM_MAX_BITS) |     0, S18_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2559 */  { S18_fnXXfn,                ITM_cos_XFN,                    "XCOS",                                        "XCOS",                                        (0 << TAM_MAX_BITS) |     0, S18_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2560 */  { S18_fnXXfn,                ITM_tan_XFN,                    "XTAN",                                        "XTAN",                                        (0 << TAM_MAX_BITS) |     0, S18_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2561 */  { S18_fnXXfn,                 ITM_pi_XFN,                    "X" STD_pi,                                    "X" STD_pi,                                    (0 << TAM_MAX_BITS) |     0, S18_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2562 */  { S18_fnXXfn,               ITM_1ONX_XFN,                    "X1/X",                                        "X1/X",                                        (0 << TAM_MAX_BITS) |     0, S18_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2563 */  { S18_fnXXfn,              ITM_atan2_XFN,                    "XATAN2",                                      "XATAN2",                                      (0 << TAM_MAX_BITS) |     0, S18_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2564 */  { S18_fnXXfn,             ITM_arcsin_XFN,                    "XASIN",                                       "XASIN",                                       (0 << TAM_MAX_BITS) |     0, S18_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2565 */  { S18_fnXXfn,             ITM_arccos_XFN,                    "XACOS",                                       "XACOS",                                       (0 << TAM_MAX_BITS) |     0, S18_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2566 */  { S18_fnXXfn,             ITM_arctan_XFN,                    "XATAN",                                       "XATAN",                                       (0 << TAM_MAX_BITS) |     0, S18_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2567 */  { S18_fnXXfn,                 ITM_LN_XFN,                    "XLN",                                         "XLN",                                         (0 << TAM_MAX_BITS) |     0, S18_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2568 */  { S18_fnXXfn,                ITM_LOG_XFN,                    "XLOG",                                        "XLOG",                                        (0 << TAM_MAX_BITS) |     0, S18_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2569 */  { S18_fnXXfn,                ITM_EXP_XFN,                    "X" STD_EulerE "^X",                           "X" STD_EulerE "^X",                           (0 << TAM_MAX_BITS) |     0, S18_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2570 */  { S18_fnXXfn,                ITM_10X_XFN,                    "X10^X",                                       "X10^X",                                       (0 << TAM_MAX_BITS) |     0, S18_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2571 */  { S18_fnXXfn,              ITM_POWER_XFN,                    "XPOWER",                                      "XPOWER",                                      (0 << TAM_MAX_BITS) |     0, S18_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2572 */  { S18_fnXXfn,               ITM_SQRT_XFN,                    "XSQRT",                                       "XSQRT",                                       (0 << TAM_MAX_BITS) |     0, S18_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2573 */  { S18_fnXXfn,                ITM_ADD_XFN,                    "XADD",                                        "XADD",                                        (0 << TAM_MAX_BITS) |     0, S18_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2574 */  { S18_fnXXfn,                ITM_SUB_XFN,                    "XSUB",                                        "XSUB",                                        (0 << TAM_MAX_BITS) |     0, S18_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2575 */  { S18_fnXXfn,               ITM_MULT_XFN,                    "XMULT",                                       "XMULT",                                       (0 << TAM_MAX_BITS) |     0, S18_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2576 */  { S18_fnXXfn,                ITM_DIV_XFN,                    "XDIV",                                        "XDIV",                                        (0 << TAM_MAX_BITS) |     0, S18_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2577 */  { S18_fnXXfn,                ITM_MOD_XFN,                    "XMOD",                                        "XMOD",                                        (0 << TAM_MAX_BITS) |     0, S18_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2578 */  { S18_fnXXfn,             ITM_MODANG_XFN,                    "XMOD" STD_MEASURED_ANGLE,                     "XMOD" STD_MEASURED_ANGLE,                     (0 << TAM_MAX_BITS) |     0, S18_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2579 */  { S18_fnXXfn,                 ITM_TO_XFN,                    "X" STD_RIGHT_ARROW "XFN",                     "X" STD_RIGHT_ARROW "XFN",                     (0 << TAM_MAX_BITS) |     0, S18_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2580 */  { S18_fnXXfn,                ITM_CHS_XFN,                    "XCHS",                                        "XCHS",                                        (0 << TAM_MAX_BITS) |     0, S18_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2581 */  { itemToBeCoded,                 NOPARAM,                    "2581",                                        "2581",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2582 */  { S18_fnXXfn,                ITM_DRG_XFN,                    "XDRG",                                        "XDRG",                                        (0 << TAM_MAX_BITS) |     0, S18_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2583 */  { S18_fnXXfn,                ITM_SQR_XFN,                    "XSQR",                                        "XSQR",                                        (0 << TAM_MAX_BITS) |     0, S18_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2584 */  { S18_fnXXfn,            ITM_XTHROOT_XFN,                   "XXRTY",                                       "XXRTY",                                        (0 << TAM_MAX_BITS) |     0, S18_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },

/* 2585 */  { fnDupN,                              3,                    "3DUP",                                        "3DUP",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2586 */  { fnSwapN,                             3,                   "3SWAP",                                       "3SWAP",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2587 */  { fnDropN,                             3,                   "3DROP",                                       "3DROP",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2588 */  { fn3Sto,                    TM_REGISTER,                    "3STO",                                        "3STO",                                        (0 << TAM_MAX_BITS) |    97, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 2589 */  { fn3Rcl,                    TM_REGISTER,                    "3RCL",                                        "3RCL",                                        (0 << TAM_MAX_BITS) |    97, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 2590 */  { fnOldItemError,                NOPARAM,                  ">NDUP<",                                      ">NDUP<",                                       ( 1 << TAM_MAX_BITS) |     4, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2591 */  { fnOldItemError,                NOPARAM,                 ">NSWAP<",                                     ">NSWAP<",                                       ( 1 << TAM_MAX_BITS) |     4, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2592 */  { fnOldItemError,                NOPARAM,                 ">NDROP<",                                     ">NDROP<",                                       ( 1 << TAM_MAX_BITS) |     8, CAT_NONE | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2593 */  { fn2Sto,                    TM_REGISTER,                    "2STO",                                        "2STO",                                        (0 << TAM_MAX_BITS) |    97, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 2594 */  { fn2Rcl,                    TM_REGISTER,                    "2RCL",                                        "2RCL",                                        (0 << TAM_MAX_BITS) |    97, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_REGISTER     | HG_ENABLED         },
/* 2595 */  { fnSwapN,                             4,                   "4SWAP",                                       "4SWAP",                                        (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },

/* 2596 */  { itemToBeCoded,                 NOPARAM,                    "XFCNS",                                       "XFCNS",                                       (0 << TAM_MAX_BITS) |     0, S18_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2597 */  { itemToBeCoded,                 NOPARAM,                  "MULTSTK",                                     "MULTSTK",                                       (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },

/* 2598 */  { itemToBeCoded,                 NOPARAM,                    "2598",                                        "2598",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2599 */  { itemToBeCoded,                 NOPARAM,                    "2599",                                        "2599",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },

/* 2600 */  { itemToBeCoded,                NOPARAM,                     "Uniform:",                                    "Uniform:",                                    (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2601 */  { fnUniformP,                         0,                     "Unfm" STD_SUB_p,                              "Unfm" STD_SUB_p,                              (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2602 */  { fnUniformL,                         0,                     "Unfm" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,    "Unfm" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,    (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2603 */  { fnUniformU,                         0,                     "Unfm" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,    "Unfm" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,    (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2604 */  { fnUniformI,                         0,                     "Unfm" STD_SUP_MINUS STD_SUP_1,                "Unfm" STD_SUP_MINUS STD_SUP_1,                (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2605 */  { itemToBeCoded,                NOPARAM,                     "DisUni:",                                     "DisUni:",                            (0 << TAM_MAX_BITS) |     0, CAT_MENU | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2606 */  { fnUniformP,                         1,                     "DisU" STD_SUB_p,                              "DisU" STD_SUB_p,                              (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2607 */  { fnUniformL,                         1,                     "DisU" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,    "DisU" STD_GAUSS_BLACK_L STD_GAUSS_WHITE_R,    (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2608 */  { fnUniformU,                         1,                     "DisU" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,    "DisU" STD_GAUSS_WHITE_L STD_GAUSS_BLACK_R,    (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2609 */  { fnUniformI,                         1,                     "DisU" STD_SUP_MINUS STD_SUP_1,                "DisU" STD_SUP_MINUS STD_SUP_1,                (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2610 */  { itemToBeCoded,                NOPARAM,                     "2610",                                        "2610",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2611 */  { itemToBeCoded,                NOPARAM,                     "2611",                                        "2611",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2612 */  { itemToBeCoded,                NOPARAM,                     "2612",                                        "2612",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2613 */  { itemToBeCoded,                NOPARAM,                     "2613",                                        "2613",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2614 */  { itemToBeCoded,                NOPARAM,                     "2614",                                        "2614",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2615 */  { itemToBeCoded,                NOPARAM,                     "2615",                                        "2615",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2616 */  { itemToBeCoded,                NOPARAM,                     "2616",                                        "2616",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2617 */  { itemToBeCoded,                NOPARAM,                     "2617",                                        "2617",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2618 */  { itemToBeCoded,                NOPARAM,                     "2618",                                        "2618",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2619 */  { itemToBeCoded,                NOPARAM,                     "2619",                                        "2619",                                        (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2620 */  { fnDupN,                       TM_VALUE,                    "NDUP",                                        "nDUP",                                       ( 1 << TAM_MAX_BITS) |     4, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 2621 */  { fnSwapN,                      TM_VALUE,                   "NSWAP",                                       "nSWAP",                                       ( 1 << TAM_MAX_BITS) |     4, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 2622 */  { fnDropN,                      TM_VALUE,                   "NDROP",                                       "nDROP",                                       ( 1 << TAM_MAX_BITS) |     8, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NUMBER_8     | HG_ENABLED         },
/* 2623 */  { fn_cnst_op_A,             ITM_MATX_A_1,                   "op_A" STD_SUP_MINUS STD_SUP_1,                "[A" STD_SUP_MINUS STD_SUP_1 "]",               (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2624 */  { fnJM,                               21,                   "3R" STD_LEFT_RIGHT_ARROWS "3P",               "3R" STD_LEFT_RIGHT_ARROWS "3P",                (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },

/* 2625 */  { itemToBeCoded,                 NOPARAM,                    "DEV",                                         "DEV",                                         (0 << TAM_MAX_BITS) |     0, CAT_MNUH | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED    | HG_ENABLED          },
/* 2626 */  { fnSetHP35,                     NOPARAM,                    "HP35",                                       "HP35",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED    | HG_ENABLED          },
/* 2627 */  { fnSetC47,                      NOPARAM,                    "47"  ,                                       "47"  ,                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED    | HG_ENABLED          },
/* 2628 */  { fnSetJM,                       NOPARAM,                    "JM"  ,                                       "JM"  ,                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED    | HG_ENABLED          },
/* 2629 */  { fnSetRJ,                       NOPARAM,                    "RJ"  ,                                       "RJ"  ,                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED    | HG_ENABLED          },
/* 2630 */  { itemToBeCoded,                 NOPARAM,                "2630_rsv",                                   "2630_rsv",                                         (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED    | HG_ENABLED          },
/* 2631 */  { itemToBeCoded,                 NOPARAM,                "2631_rsv",                                   "2631_rsv",                                         (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED    | HG_ENABLED          },
/* 2632 */  { itemToBeCoded,                 NOPARAM,                "2632_rsv",                                   "2632_rsv",                                         (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED    | HG_ENABLED          },
/* 2633 */  { itemToBeCoded,                 NOPARAM,                "2633_rsv",                                   "2633_rsv",                                         (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED    | HG_ENABLED          },
/* 2634 */  { itemToBeCoded,                 NOPARAM,                "2634_rsv",                                   "2634_rsv",                                         (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED    | HG_ENABLED          },
/* 2635 */  { itemToBeCoded,                 NOPARAM,                "2635_rsv",                                   "2635_rsv",                                         (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED    | HG_ENABLED          },
/* 2636 */  { itemToBeCoded,                 NOPARAM,                "2636_rsv",                                   "2636_rsv",                                         (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED    | HG_ENABLED          },
/* 2637 */  { itemToBeCoded,                 NOPARAM,                "2637_rsv",                                   "2637_rsv",                                         (0 << TAM_MAX_BITS) |     0, CAT_FREE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED    | HG_ENABLED          },
/* 2638 */  { itemToBeCoded,                 NOPARAM,                        "",                                  "TamStoTvm",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2639 */  { itemToBeCoded,                 NOPARAM,                        "",                                  "TamStoTvm",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2640 */  { addItemToBuffer, RESERVED_VARIABLE_NPPER,                     "n",                                          "n",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_DISABLED        },
/* 2641 */  { addItemToBuffer, RESERVED_VARIABLE_IPONA,                   "I/a",                                        "I/a",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_DISABLED        },
/* 2642 */  { addItemToBuffer, RESERVED_VARIABLE_PV,                       "PV",                                         "PV",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_DISABLED        },
/* 2643 */  { addItemToBuffer, RESERVED_VARIABLE_FV,                       "FV",                                         "FV",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_DISABLED        },
/* 2644 */  { addItemToBuffer, RESERVED_VARIABLE_PMT,                     "PMT",                                        "PMT",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_DISABLED        },
/* 2645 */  { addItemToBuffer, RESERVED_VARIABLE_PPERONA,                "pp/a",                                       "pp/a",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_DISABLED        },
/* 2646 */  { addItemToBuffer, RESERVED_VARIABLE_CPERONA,                "cp/a",                                       "cp/a",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_DISABLED        },
/* 2647 */  { addItemToBuffer,              ITM_dddVEL1,                  "",                    STD_ELLIPSIS "VEL" STD_SUB_1,                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2648 */  { addItemToBuffer,              ITM_dddVEL2,                  "",                    STD_ELLIPSIS "VEL" STD_SUB_2,                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2649 */  { addItemToBuffer,              ITM_dddVEL3,                  "",                    STD_ELLIPSIS "VEL" STD_SUB_3,                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2450 */  { addItemToBuffer,              ITM_dddVEL,                   "",          STD_ELLIPSIS "VEL" STD_SUB_n STD_SUB_n,                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2451 */  { addItemToBuffer,              ITM_dddIX,                    "",                                         "INDEX",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2452 */  { addItemToBuffer,              ITM_dddEL,                    "",            STD_ELLIPSIS STD_SPACE_6_PER_EM "EL",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2453 */  { addItemToBuffer,              ITM_dddIJ,                    "",            STD_ELLIPSIS STD_SPACE_6_PER_EM "IJ",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2454 */  { addItemToBuffer,              ITM_Max,                      "",           STD_ELLIPSIS STD_SPACE_6_PER_EM "Max",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2455 */  { addItemToBuffer,              ITM_Min,                      "",           STD_ELLIPSIS STD_SPACE_6_PER_EM "Min",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2456 */  { addItemToBuffer,              ITM_Config,                   "",                                        "Config",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },
/* 2457 */  { addItemToBuffer,              ITM_Stack,                    "",                                         "Stack",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_UNCHANGED | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },

/* 2658 */  UNIT_CONV(constFactorErgJ,       divide,   "erg" STD_RIGHT_ARROW "J" ,  "erg" STD_RIGHT_ARROW "J" ),
/* 2659 */  UNIT_CONV(constFactorErgJ,     multiply,   "J"  STD_RIGHT_ARROW "erg",  "J"  STD_RIGHT_ARROW "erg"),
/* 2660 */  UNIT_CONV(constFactorFoeJ,       divide,   "foe" STD_RIGHT_ARROW "J" ,  "foe" STD_RIGHT_ARROW "J" ),
/* 2661 */  UNIT_CONV(constFactorFoeJ,     multiply,   "J"  STD_RIGHT_ARROW "foe",  "J"  STD_RIGHT_ARROW "foe"),

/* 2662 */  { fnProcessLR,                  1,                            "LR:a0",                                 "LR:a0",                                             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2663 */  { fnProcessLR,                  2,                            "LR:a1",                                 "LR:a1",                                             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },
/* 2664 */  { fnProcessLR,                  4,                            "LR:a2",                                 "LR:a2",                                             (0 << TAM_MAX_BITS) |     0, CAT_FNCT | SLS_ENABLED   | US_ENABLED   | EIM_DISABLED | PTP_NONE         | HG_ENABLED         },

/* 2665 */  { itemToBeCoded,                 NOPARAM,                     "",                                     "Last item",                                         (0 << TAM_MAX_BITS) |     0, CAT_NONE | SLS_ENABLED   | US_UNCHANGED | EIM_DISABLED | PTP_DISABLED     | HG_ENABLED         },

};
