// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"


#if !defined(TESTSUITE_BUILD)

TO_QSPI static const char bugScreenNonexistentMenu[] = "In function determineFunctionKeyItem: nonexistent menu specified!";
TO_QSPI static const char bugScreenItemNotDetermined[] = "In function determineItem: item was not determined!";

static void executeFunction(const char *data, int16_t item_);

  int16_t determineFunctionKeyItem_C47(const char *data, bool_t shiftF, bool_t shiftG) { //Added itemshift param JM
    int16_t item = ITM_NOP;
    dynamicMenuItem = -1;
    int16_t itemShift = (shiftF ? 6 : (shiftG ? 12 : 0));
    int16_t fn = *(data) - '0' - 1;
    const softmenu_t *sm;
    int16_t row, menuId = softmenuStack[0].softmenuId;
    int16_t firstItem = softmenuStack[0].firstItem;

                    #if defined(VERBOSEKEYS)
                      printf(">>>>Z 0090 determineFunctionKeyItem_C47(fn=%d):    data=|%s| data[0]=%d item=%d itemShift=%d (Global) FN_key_pressed=%d\n", fn, data, data[0], item, itemShift, FN_key_pressed);
                    #endif // VERBOSEKEYS
                    #if defined(PC_BUILD)
                      char tmp[200];
                      sprintf(tmp,"^^^^determineFunctionKeyItem_C47(fn=%d): itemShift=%d menuId=%d menuItem=%d", fn, itemShift, menuId, -softmenu[menuId].menuItem);
                      jm_show_comment(tmp);
                    #endif // PC_BUILD

    if(IS_BASEBLANK_(menuId)) {
      return item;
    }

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wimplicit-fallthrough"

                    #if defined(VERBOSEKEYS)
                    printf(">>>>Z 0090a determineFunctionKeyItem       -softmenu[menuId].menuItem=%i\n",-softmenu[menuId].menuItem);
                    #endif // VERBOSEKEYS

    switch(-softmenu[menuId].menuItem) {
      case MNU_MyMenu: {
        dynamicMenuItem = firstItem + itemShift + fn;
        item = userMenuItems[dynamicMenuItem].item;
        setCurrentUserMenu(item, userMenuItems[dynamicMenuItem].argumentName);
        break;
      }

      case MNU_MyAlpha: {
        dynamicMenuItem = firstItem + itemShift + fn;
        item = userAlphaItems[dynamicMenuItem].item;
                    #if defined(VERBOSEKEYS)
                    printf(">>>>Z 0091   case MNU_MyAlpha             data=|%s| data[0]=%d item=%d itemShift=%d (Global) FN_key_pressed=%d\n",data,data[0],item,itemShift, FN_key_pressed);
                    printf(">>>>  0092     dynamicMenuItem=%d\n",dynamicMenuItem);
                    printf(">>>>  0093     firstItem=%d itemShift=%d fn=%d",firstItem, itemShift, fn);
                    #endif //VERBOSEKEYS
        break;
      }

      case MNU_DYNAMIC: {
        dynamicMenuItem = firstItem + itemShift + fn;
        item = userMenus[currentUserMenu].menuItem[dynamicMenuItem].item;
        // currentUserMenu update is managed later on in executeFunction
        break;
      }

      case MNU_PROG: {
        dynamicMenuItem = firstItem + itemShift + fn;
        if(tam.function == ITM_GTOP) {
          item = (dynamicMenuItem >= dynamicSoftmenu[menuId].numItems ? ITM_NOP : ITM_GTOP);
        }
        else {
          item = (dynamicMenuItem >= dynamicSoftmenu[menuId].numItems ? ITM_NOP : MNU_DYNAMIC);
        }
        break;
      }

      case MNU_VAR: {
        dynamicMenuItem = firstItem + itemShift + fn;
        item = (dynamicMenuItem >= dynamicSoftmenu[menuId].numItems ? ITM_NOP : MNU_DYNAMIC);
        break;
      }

      case MNU_MVAR: {
        dynamicMenuItem = firstItem + itemShift + fn;
        if(tam.mode) {
          item = (dynamicMenuItem >= dynamicSoftmenu[menuId].numItems ? ITM_NOP : MNU_DYNAMIC);
        }
        else if(currentMvarLabel != INVALID_VARIABLE) {
          item = (dynamicMenuItem >= dynamicSoftmenu[menuId].numItems ? ITM_NOP : ITM_SOLVE_VAR);
        }

// These items below are all in the primary function key row, and should not be decoded from actual operating modes, but will be better to be decoded from the visible currentmenu,
//   It cannot be changed to menu lookups, because when the MVAR is open, we do not know anymore which menu or function actually opened the MVAR, hence the method of decoding from operating modes.
//   This is not ideal. It probably would still work better if we check which menu called the MVAR, i.e. the menu in currentmenu() == MVAR && menu(1) == whatever.

//Grapher
        else if((currentSolverStatus & SOLVER_STATUS_USES_FORMULA) && (currentSolverStatus & SOLVER_STATUS_INTERACTIVE) && ((currentSolverStatus & SOLVER_STATUS_EQUATION_MODE) == SOLVER_STATUS_EQUATION_GRAPHER) && dynamicMenuItem == 5) {
          item = ITM_DRAW;
        }
        else if((currentSolverStatus & SOLVER_STATUS_USES_FORMULA) && (currentSolverStatus & SOLVER_STATUS_INTERACTIVE) && ((currentSolverStatus & SOLVER_STATUS_EQUATION_MODE) == SOLVER_STATUS_EQUATION_GRAPHER) && dynamicMenuItem == 4) {
          item = -MNU_GRAPHS;
        }

//Solver
        else if((currentSolverStatus & SOLVER_STATUS_USES_FORMULA) && (currentSolverStatus & SOLVER_STATUS_INTERACTIVE) && ((currentSolverStatus & SOLVER_STATUS_EQUATION_MODE) == SOLVER_STATUS_EQUATION_SOLVER) && dynamicMenuItem == 5) {
          item = ITM_CALC;
        }
        else if((currentSolverStatus & SOLVER_STATUS_USES_FORMULA) && (currentSolverStatus & SOLVER_STATUS_INTERACTIVE) && ((currentSolverStatus & SOLVER_STATUS_EQUATION_MODE) == SOLVER_STATUS_EQUATION_SOLVER) && dynamicMenuItem == 4) {
          item = -MNU_Solver_TOOL;
        }
        else if((currentSolverStatus & SOLVER_STATUS_USES_FORMULA) && (currentSolverStatus & SOLVER_STATUS_INTERACTIVE) && *getNthString(dynamicSoftmenu[softmenuStack[0].softmenuId].menuContent, dynamicMenuItem) == 0) {
          item = ITM_NOP;
        }

//f'
        else if(IS_EQN_1STDER && dynamicMenuItem == 5) {
          item = ITM_FPHERE;
        }
        else if(IS_EQN_1STDER && dynamicMenuItem == 4) {
          item = -MNU_GRAPHS;
        }

//f''
        else if(IS_EQN_2NDDER && dynamicMenuItem == 5) {
          item = ITM_FPPHERE;
        }
        else if(IS_EQN_2NDDER && dynamicMenuItem == 4) {
          item = -MNU_GRAPHS;
        }


        else if(dynamicMenuItem >= dynamicSoftmenu[menuId].numItems) {
          item = ITM_NOP;
        }
        else if(!(currentSolverStatus & SOLVER_STATUS_INTERACTIVE)) {
          item = MNU_DYNAMIC;
        }

//integral MNU_Sf
        else if( (IS_EQN_INTEGRATE) && dynamicMenuItem == 4) {
          item = -MNU_Sf_TOOL;
        }

//integral y to x
        else if( (IS_EQN_INTEGRATE) && dynamicMenuItem == 5) {
          item = ITM_INTEGRAL_YX;
        }


        else if(IS_EQN_INTEGRATE) {
          item = ITM_Sfdx_VAR;
        }
        else {
          item = ITM_SOLVE_VAR;
        }
        break;
      }

      //VARS MENU
      case MNU_CONFIGS:
      case MNU_MATRS:
      case MNU_DATES:
      case MNU_TIMES:
      case MNU_SINTS:
      case MNU_STRINGS:
      case MNU_NUMBRS:
      case MNU_CPXS:
      case MNU_REALS:
      case MNU_ANGLES:
      case MNU_LINTS:
      case MNU_ALLVARS:
      {
        dynamicMenuItem = firstItem + itemShift + fn;
        item = (dynamicMenuItem >= dynamicSoftmenu[menuId].numItems ? ITM_NOP : (tam.mode == TM_DELITM) ? MNU_DYNAMIC : ITM_RCL);
        break;
      }

      case MNU_PROGS: {
        dynamicMenuItem = firstItem + itemShift + fn;
        item = (dynamicMenuItem >= dynamicSoftmenu[menuId].numItems ? ITM_NOP : (tam.mode == TM_DELITM) ? MNU_DYNAMIC : ITM_XEQ);
        break;
      }

      case MNU_MENU:
      case MNU_MENUS: {
        dynamicMenuItem = firstItem + itemShift + fn;
        item = ITM_NOP;
        if(dynamicMenuItem < dynamicSoftmenu[menuId].numItems) {
          for(uint16_t i = 0; softmenu[i].menuItem < 0; ++i) {
                    #if defined(VERBOSEKEYS)
                    printf(">>>>Z 0093b determineFunctionKeyItem  case MNU_MENUS:     i=%u softmenu[i].menuItem=%i name:=%s\n",i,softmenu[i].menuItem, indexOfItems[-softmenu[i].menuItem].itemCatalogName);
                    #endif //VERBOSEKEYS
            if(compareString((char *)getNthString(dynamicSoftmenu[menuId].menuContent, dynamicMenuItem), indexOfItems[-softmenu[i].menuItem].itemCatalogName, CMP_NAME) == 0) {
              if(tam.mode == TM_DELITM) {
                item = MNU_DYNAMIC;
                tam.value = numberOfUserMenus;
              }
              else {
                item = softmenu[i].menuItem;
                    #if defined(VERBOSEKEYS)
                    printf(">>>>Z 0093c determineFunctionKeyItem  item = %i:   name:=%s\n",item, indexOfItems[-item].itemCatalogName);
                    #endif //VERBOSEKEYS
              }
            }
          }
          for(uint32_t i = 0; i < numberOfUserMenus; ++i) {
            if(compareString((char *)getNthString(dynamicSoftmenu[menuId].menuContent, dynamicMenuItem), userMenus[i].menuName, CMP_NAME) == 0) {
              if(tam.mode == TM_DELITM) {
                item = MNU_DYNAMIC;
                tam.value = i;
              }
              else {
                item = -MNU_DYNAMIC;
                if(calcMode != CM_ASSIGN) {
                  currentUserMenu = i;
                }
              }
            }
          }
        }
        break;
      }

      case ITM_MENU: {
        dynamicMenuItem = firstItem + itemShift + fn;
        item = ITM_MENU;
        break;
      }

      case MNU_EQN: {
        if(numberOfFormulae == 0 && (firstItem + itemShift + fn) > 0) {
          break;
        }
        /* fallthrough */
      }

      default: {
        sm = &softmenu[menuId];
        row = min(3, (sm->numItems + modulo(firstItem - sm->numItems, 6))/6 - firstItem/6) - 1;
        if(itemShift/6 <= row && firstItem + itemShift + fn < sm->numItems) {
          item = (sm->softkeyItem)[firstItem + itemShift + fn] % 10000;

          if(item == ITM_PROD_SIGN) {
            item = (getSystemFlag(FLAG_MULTx) ? ITM_DOT : ITM_CROSS);
          }

          if(softmenu[menuId].menuItem == -MNU_ALPHA && calcMode == CM_PEM && item == ITM_ASSIGN) {
            item = ITM_NULL;
          }
        }
      #if defined(VERBOSEKEYS)
        printf(">>>>Z 0094 Fallthrough item=%d \n",item);
      #endif //VERBOSEKEYS
      }
    }

                    #if defined(VERBOSEKEYS)
                    printf(">>>>Z 0094B    calcMode == %u  data=|%s| data[0]=%d item=%d itemShift=%d (Global) FN_key_pressed=%d\n",calcMode, data,data[0],item,itemShift, FN_key_pressed);
                    printf(">>>>  0095     dynamicMenuItem=%d\n",dynamicMenuItem);
                    printf(">>>>  0096     firstItem=%d itemShift=%d fn=%d\n",firstItem, itemShift, fn);
                    #endif //VERBOSEKEYS

  #pragma GCC diagnostic pop
    if(calcMode == CM_ASSIGN && item != ITM_NOP && item != ITM_NULL) {
      switch(-softmenu[menuId].menuItem) {
        case MNU_PROG:
        case MNU_PROGS: {
                    #if defined(VERBOSEKEYS)
                    printf("0096a PROG or PROGS: registerno:%s\n", (char *)getNthString(dynamicSoftmenu[menuId].menuContent, dynamicMenuItem) );
                    printf("0096b %d %d %d\n",findNamedLabel((char *)getNthString(dynamicSoftmenu[menuId].menuContent, dynamicMenuItem)), - FIRST_LABEL, + ASSIGN_LABELS);
                    #endif //VERBOSEKEYS
          return findNamedLabel((char *)getNthString(dynamicSoftmenu[menuId].menuContent, dynamicMenuItem)) - FIRST_LABEL + ASSIGN_LABELS;
        }

        case MNU_VAR:

        //VARS MENU
        case MNU_CONFIGS:
        case MNU_MATRS:
        case MNU_DATES:
        case MNU_TIMES:
        case MNU_SINTS:
        case MNU_STRINGS:
        case MNU_NUMBRS:
        case MNU_CPXS:
        case MNU_REALS:
        case MNU_ANGLES:
        case MNU_LINTS:
        case MNU_ALLVARS:


         {
          return findNamedVariable((char *)getNthString(dynamicSoftmenu[menuId].menuContent, dynamicMenuItem)) - FIRST_NAMED_VARIABLE + ASSIGN_NAMED_VARIABLES;
        }
        case MNU_MENUS: {
          if(item == -MNU_DYNAMIC) {
            for(int32_t i = 0; i < numberOfUserMenus; ++i) {
              if(compareString((char *)getNthString(dynamicSoftmenu[menuId].menuContent, dynamicMenuItem), userMenus[i].menuName, CMP_NAME) == 0) {
                return ASSIGN_USER_MENU - i;
              }
            }
            displayBugScreen(bugScreenNonexistentMenu);
                    #if defined(VERBOSEKEYS)
                    printf(">>>>  0086 item=%d \n",item);
                    #endif //VERBOSEKEYS
            return item;
          }
          else {
                    #if defined(VERBOSEKEYS)
                    printf(">>>>  0087 item=%d \n",item);
                    #endif //VERBOSEKEYS
            return item;
          }
        }
        default: {
                    #if defined(VERBOSEKEYS)
                    printf(">>>>  0088 item=%d \n",item);
                    #endif //VERBOSEKEYS
          return item;
        }
      }
    }
    else {
                    #if defined(VERBOSEKEYS)
                    printf(">>>>  0089 item=%d \n",item);
                    #endif //VERBOSEKEYS
      if(item == 0) return ITM_NOP;
      return item;
    }
  }



  #if defined(PC_BUILD)
    void btnFnClicked(GtkWidget *notUsed, gpointer data) {
                    #if defined(VERBOSEKEYS)
                    printf(">>>>Z 0070 btnFnClicked data=|%s| data[0]=%d\n",(char*)data, ((char*)data)[0]);
                    #endif //VERBOSEKEYS
      executeFunction(data, 0);
    }
  #endif // PC_BUILD



  #if defined(DMCP_BUILD)
    void btnFnClicked(void *unused, void *data) {
      executeFunction(data, 0);
    }
  #endif // DMCP_BUILD



  #if defined(PC_BUILD)
    void btnFnClickedP(GtkWidget *notUsed, gpointer data) { //JM Added this portion to be able to go to NOP on emulator
      GdkEvent mouseButton;
      mouseButton.button.button = 1;
      mouseButton.type = 0;
      btnFnPressed(notUsed, &mouseButton, data);
    }

    void btnFnClickedR(GtkWidget *notUsed, gpointer data) { //JM Added this portion to be able to go to NOP on emulator
      GdkEvent mouseButton;
      mouseButton.button.button = 1;
      mouseButton.type = 0;
      btnFnReleased(notUsed, &mouseButton, data);
    }
  #endif // PC_BUILD


    void execAutoRepeat(uint16_t key) {
    #if defined(DMCP_BUILD)
      char charKey[6];
      bool_t f = shiftF;
      bool_t g = shiftG;
      uint8_t origScreenUpdatingMode = screenUpdatingMode;
      sprintf(charKey, "%02d", key -1);

      fnTimerStart(TO_AUTO_REPEAT, key, KEY_AUTOREPEAT_PERIOD);

      btnClicked(NULL, (char *)charKey);
      screenUpdatingMode = origScreenUpdatingMode;
      //btnPressed(charKey);
      shiftF = f;
      shiftG = g;
      refreshLcd();
      lcd_refresh_dma();
    #endif // DMCP_BUILD
    }


    //Closing catalog menu only if it is a real catalog entry.
    //  The previous system closes a normal menu if the CAT main menu happens to be in one of the older menu slots in the menu stack
    //    and that happens if you call CAT and then say CLK or any other menu and runn a command from there. Then CAT is present and the menu closed after the command executed.

    TO_QSPI const int16_t CatalogMenus[] = {
      //          MNU_CATALOG,  //option to include if we need to close the actual CAT menu too, i.e. jump back to before CAT (like 42S does
      MNU_ALPHA_OMEGA,
      MNU_ALPHAMISC,
      MNU_ALPHA,
      //CAT MENU
      MNU_FCNS,
      MNU_CONST,
      MNU_CHARS,
      MNU_PROGS,
      MNU_VARS,
      MNU_MENUS,
      //VARS MENU
      MNU_CONFIGS,
      MNU_MATRS,
      MNU_DATES,
      MNU_TIMES,
      MNU_SINTS,
      MNU_STRINGS,
      MNU_NUMBRS,
      MNU_CPXS,
      MNU_REALS,
      MNU_ANGLES,
      MNU_LINTS,
      MNU_ALLVARS
    };

    static void closeAllCatalogMenus(void) {
      int16_t menu = -currentMenu();
      for(uint_fast16_t i = 0; i < nbrOfElements(CatalogMenus); i++) {
        if(menu == CatalogMenus[i]) {
          popSoftmenu();
          //         closeAllCatalogMenus(); //Option to recurse and close more than one menu level until all the CAT related menus are out
          break;
        }
      }
    }


    static void _closeCatalog(void) {
      bool_t inCatalog = false;
      for(int i = 0; i < SOFTMENU_STACK_SIZE; ++i) {
        if(softmenu[softmenuStack[i].softmenuId].menuItem == -MNU_CATALOG) {
          inCatalog = true;
          break;
        }
      }
      if(inCatalog || currentMenu() == -MNU_CONST) {
        switch(-currentMenu()) {
          case MNU_TAM:
          case MNU_TAMVARONLY:
          case MNU_TAMNONREG:
          case MNU_TAMCMP:
          case MNU_TAMSTO:
          case MNU_TAMRCL:
          case MNU_TAMFLAG:
          case MNU_TAMSHUFFLE:
          case MNU_TAMLABEL:
          case MNU_TAMLBLONLY:
          case ITM_DELITM: {
            // TAM menus are processed elsewhere
            break;
          }
          default: {
            leaveAsmMode();
            //popSoftmenu();        //removed in favour of the more selective closing
            closeAllCatalogMenus();

        }
      }
    }
  }



#define lowercaseselected  (bool_t)((alphaCase == AC_LOWER && !lastshiftF) || (alphaCase == AC_UPPER && lastshiftF /*&& !numLock*/)) // //JM remove last !numlock if you want the shift, during numlock, to produce lower case

  void processAimInput(int16_t item) {
    int16_t item1 = 0;
                    #if defined(PC_BUILD)
                      char tmp[200]; sprintf(tmp,"^^^^processAimInput:AIM %d nextChar=%d",item,nextChar); jm_show_comment(tmp);
                      #if defined(PAIMDEBUG)
                        printf("%s, |%s|\n",tmp,aimBuffer);
                      #endif //PAIMDEBUG
                    #endif //PC_BUILD

    if(scrLock != NC_NORMAL) {
      nextChar = scrLock;
    }

    int16_t itemOut = item;
    if(keyReplacements(item, &item1, getSystemFlag(FLAG_NUMLOCK), lastshiftF, lastshiftG) > 0) {  //JMvv
      if(item1 > 0) {
        addItemToBuffer(item1);
                    #if defined(PAIMDEBUG)
                      printf("---#K %d\n",keyActionProcessed);
                    #endif //PAIMDEBUG
        keyActionProcessed = true;
      }
    }

    else if(caseReplacements(0, lowercaseselected, item, &itemOut)) {
      addItemToBuffer(itemOut);
                    #if defined(PAIMDEBUG)
                      printf("---#H %d\n",keyActionProcessed);
                    #endif //PAIMDEBUG
      keyActionProcessed = true;
    }

    else if(item == ITM_COLON || item == ITM_COMMA || item == ITM_QUESTION_MARK || item == ITM_SPACE || item == ITM_UNDERSCORE) {  //JM vv DIRECT LETTERS
      addItemToBuffer(item);
                    #if defined(PAIMDEBUG)
                      printf("---#F %d\n",keyActionProcessed);
                    #endif //PAIMDEBUG
      keyActionProcessed = true;
    }
    else if(item == ITM_DOWN_ARROW) {
      if(nextChar == NC_NORMAL) nextChar = NC_SUBSCRIPT; else if(nextChar == NC_SUPERSCRIPT) nextChar = NC_NORMAL; //JM stack the SUP/NORMAL/SUB
                    #if defined(PAIMDEBUG)
                      printf("---#C %d\n",keyActionProcessed);
                    #endif //PAIMDEBUG
      keyActionProcessed = true;
    }

    else if(item == ITM_UP_ARROW) {
      if(nextChar == NC_NORMAL) nextChar = NC_SUPERSCRIPT; else if(nextChar == NC_SUBSCRIPT) nextChar = NC_NORMAL; //JM stack the SUP/NORMAL/SUB
                    #if defined(PAIMDEBUG)
                      printf("---#B %d\n",keyActionProcessed);
                    #endif //PAIMDEBUG
      keyActionProcessed = true;
    }

    else if(indexOfItems[item].func == addItemToBuffer) {
      addItemToBuffer(item);
                    #if defined(PAIMDEBUG)
                      printf("---#A %d\n",keyActionProcessed);
                      printf("###---> 3, |%s|\n",aimBuffer);
                    #endif //PAIMDEBUG
      keyActionProcessed = true;
    }

    if(keyActionProcessed) {
                    #if defined(PC_BUILD) && defined(MONITOR_CLRSCR)
                      printf("refreshScreen(): calcMode=%u End of processAimInput\n", calcMode);
                    #endif //PC_BUILD

      screenUpdatingMode &= ~SCRUPD_MANUAL_STACK;
        // refreshScreen(101);
    }

      showHideAlphaMode();
                    #if defined(PC_BUILD)
                      sprintf(tmp,"^^^^processAimInput:AIM:end %d, processed %d",item,keyActionProcessed); jm_show_comment(tmp);
                      #if defined(PAIMDEBUG)
                        printf("%s, |%s|\n",tmp,aimBuffer);
                      #endif //PAIMDEBUG
                    #endif //PC_BUILD
  }






  uint8_t asnKey[4] = {0, 0, 0, 0};
  bool_t releaseOverride = false;

  #if defined(PC_BUILD)
    void btnFnPressed(GtkWidget *notUsed, GdkEvent *event, gpointer data) {
      if(event->type == GDK_DOUBLE_BUTTON_PRESS || event->type == GDK_TRIPLE_BUTTON_PRESS) { // return unprocessed for double or triple click
        return;
      }
      if(event->button.button == 2) { // Middle click
        shiftF = true;
        shiftG = false;
      }
      if(event->button.button == 3) { // Right click
        shiftF = false;
        shiftG = true;
      }
  #endif // PC_BUILD
  #if defined(DMCP_BUILD)
    void btnFnPressed(void *data) {
  #endif // DMCP_BUILD

                    #if defined(VERBOSEKEYS)
                      printf(">>>>Z 0010 btnFnPressed SET FN_key_pressed            ; data=|%s| data[0]=%d shiftF=%d shiftG=%d\n",(char*)data, ((char*)data)[0],shiftF, shiftG);
                    #endif //VERBOSEKEYS
      if(SHOWMODE || currentMenu() == -MNU_SHOW) {
        closeShowMenu();
      }

      FN_timed_out_to_NOP_or_Executed = false;
      releaseOverride = false;
      temporaryInformation = TI_NO_INFO;
      FN_key_pressed = *((char *)data) - '0' + 37;  //to render 38-43, as per original keypress

      asnKey[0] = ((uint8_t *)data)[0];
      asnKey[1] = 0;

      if(programRunStop == PGM_RUNNING || programRunStop == PGM_PAUSED) {
        setLastKeyCode((*((char *)data) - '0') + 37);
      }
      else {
        lastKeyCode = 0;
      }

      if(programRunStop == PGM_PAUSED) {
        programRunStop = PGM_KEY_PRESSED_WHILE_PAUSED;
        return;
      }

      //Exception, to activate the primary functions of the timer menu, without allowing longpresses and double presses, in order to have quicker activation
      if(!shiftF && /*!shiftG &&*/ currentMenu() == -MNU_TIMERF ){   //do not check for g, to enable the g-line fast response on press when keyboard shortcuts help line is pressed
        const int16_t *softkeyItem = softmenu[softmenuStack[0].softmenuId].softkeyItem;
        int16_t _item = softkeyItem[asnKey[0]-'1'];
        //printf("WWWWWWWW-0 %i %i\n",currentMenu(), softkeyItem[asnKey[0]-'1']);
        reallyRunFunction(_item,NOPARAM);
        hourGlassIconEnabled = false;
        //printf("WWWWWWWW-1 %i %i\n",currentMenu(), softkeyItem[asnKey[0]-'1']);
        if(_item == ITM_TIMER_R_S) {
          screenUpdatingMode |= SCRUPD_SKIP_STACK_ONE_TIME;
        } else {
          screenUpdatingMode &= ~SCRUPD_MANUAL_STACK;
        }
        refreshScreen(136);
        releaseOverride = true;
        return;
      }

      lastshiftF = shiftF;
      lastshiftG = shiftG;


      if(tam.mode == TM_KEY && !tam.keyInputFinished) {
        // not processed here
        return;
      }
      if(calcMode == CM_ASSIGN && itemToBeAssigned != 0 && !(tam.alpha && tam.mode != TM_NEWMENU)) {

                    #if defined(VERBOSEKEYS)
                      printf(">>>>Z 0011 btnFnPressed >>determineFunctionKeyItem_C47; data=|%s| data[0]=%d shiftF=%d shiftG=%d\n", (char*)data, ((char*)data)[0], shiftF, shiftG);
                    #endif //VERBOSEKEYS

      int16_t item = determineFunctionKeyItem_C47((char *)data, shiftF, shiftG);

                    #if defined(VERBOSEKEYS)
                      printf(">>>>Z 011a btnFnPressed >>determineFunctionKeyItem_C47; data=|%s| data[0]=%d shiftF=%d shiftG=%d\n", (char*)data, ((char*)data)[0], shiftF, shiftG);
                    #endif //VERBOSEKEYS

      #pragma GCC diagnostic push
      #pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
        switch(-currentMenu()) {
        case MNU_MENUS: {
            if(item <= ASSIGN_USER_MENU) {
              currentUserMenu = ASSIGN_USER_MENU - item;
              item = -MNU_DYNAMIC;
            }
            /* fallthrough */
        }
          case MNU_CATALOG:
          case MNU_CHARS:
          case MNU_PROGS:
          case MNU_VARS: {
              showFunctionNameItem = item;
            break;
        }
        default: {
            updateAssignTamBuffer();
        }
      }
      #pragma GCC diagnostic pop
        _closeCatalog();
      }
      else if(calcMode != CM_REGISTER_BROWSER && calcMode != CM_FLAG_BROWSER && calcMode != CM_ASN_BROWSER && calcMode != CM_FONT_BROWSER) {

                    #if defined(VERBOSEKEYS)
                    printf(">>>>Z 0012 btnFnPressed >>determineFunctionKeyItem_C47; data=|%s| data[0]=%d shiftF=%d shiftG=%d\n",(char*)data, ((char*)data)[0],shiftF, shiftG);
                    #endif //VERBOSEKEYS

//      int16_t item = determineFunctionKeyItem_C47((char *)data, shiftF, shiftG);
/*
        if(shiftF || shiftG) {
          screenUpdatingMode &= ~SCRUPD_MANUAL_SHIFT_STATUS;
          clearShiftState();
        }

        shiftF = false;
        shiftG = false;
*/

        lastErrorCode = 0;
        btnFnPressed_StateMachine(NULL, data);    //never allow a function key to directly enter into a buffer - always via the key detection btnFnPressed_StateMachine, to pick up longpress or double press conditions
                    #if defined(VERBOSEKEYS)
                    printf(">>>>Z 0013 btnFnPressed >>btnFnPressed_StateMachine; data=|%s| data[0]=%d shiftF=%d shiftG=%d\n",(char*)data, ((char*)data)[0],shiftF, shiftG);
                    #endif //VERBOSEKEYS

/*
          if(calcMode != CM_ASSIGN && indexOfItems[item].func == addItemToBuffer) {
            // If we are in the catalog then a normal key press should affect the Alpha Selection Buffer to choose
            // an item from the catalog, but a function key press should put the item in the AIM (or TAM) buffer
            // Use this variable to distinguish between the two
            if(calcMode == CM_PEM && !tam.mode) {
              if(getSystemFlag(FLAG_ALPHA)) {
                pemAlpha(item);
              }
              else {
                addStepInProgram(item);
              }
              hourGlassIconEnabled = false;
            }
            else {
              fnKeyInCatalog = 1;
              addItemToBuffer(item);                //this PEM TAM entry moved to keyFnRelease, to pick up the long presses
              fnKeyInCatalog = 0;
            }
            if(calcMode == CM_EIM && !tam.mode) {   //this EIM portion moved to after release, to allow longpress and double press
              while(currentMenu() != -MNU_EQ_EDIT) {
                popSoftmenu();
              }
            }
            _closeCatalog();
            refreshScreen();
          }

          else {
            showFunctionNameItem = item;
          }
*/
      }
    }





  static bool_t _assignToMenu(uint8_t *data) {
    switch(-currentMenu()) {
      case MNU_MyMenu: {
        assignToMyMenu((*data - '1') + (shiftG ? 12 : shiftF ? 6 : 0));
        goto endReturnTrue;
      }
      case MNU_MyAlpha: {
        assignToMyAlpha((*data - '1') + (shiftG ? 12 : shiftF ? 6 : 0));
        goto endReturnTrue;
      }
      case MNU_DYNAMIC: {
        assignToUserMenu((*data - '1') + (shiftG ? 12 : shiftF ? 6 : 0));
        goto endReturnTrue;
      }
      case MNU_HOME: {
        if(!setCurrentUserMenu(-MNU_DYNAMIC,"HOME")) {
          #if defined(PC_BUILD)
            printf("Not done!\n");
          #endif //PC_BUILD
          return false;
        }
        assignToUserMenu((*data - '1') + (shiftG ? 12 : shiftF ? 6 : 0));
        goto endReturnTrue;
      }
      case MNU_PFN: {
        if(!setCurrentUserMenu(-MNU_DYNAMIC,"P.FN")) {
          #if defined(PC_BUILD)
            printf("Not done!\n");
          #endif //PC_BUILD
          return false;
        }
        assignToUserMenu((*data - '1') + (shiftG ? 12 : shiftF ? 6 : 0));
        goto endReturnTrue;
      }
      case MNU_CATALOG:
      case MNU_ALPHA: //JM
      case MNU_CHARS:
      case MNU_PROGS:
      case MNU_VARS:
      case MNU_MENUS: {
        screenUpdatingMode &= ~SCRUPD_ONE_TIME_FLAGS;
        return false;
      }
      default: {
        displayCalcErrorMessage(ERROR_CANNOT_ASSIGN_HERE, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
        #if defined(PC_BUILD)
          moreInfoOnError("In function _assignToMenu:", "the menu", indexOfItems[-currentMenu()].itemCatalogName, "is write-protected.");
        #endif // PC_BUILD
endReturnTrue:
        calcMode = previousCalcMode;
        shiftF = shiftG = false;
        _closeCatalog();
        screenUpdatingMode &= ~SCRUPD_MANUAL_MENU;
        screenUpdatingMode &= ~SCRUPD_MANUAL_STACK;
        refreshScreen(103);
        screenUpdatingMode &= ~SCRUPD_ONE_TIME_FLAGS;
        return true;
      }
    }
  }


  #if defined(PC_BUILD)
    void btnFnReleased(GtkWidget *notUsed, GdkEvent *event, gpointer data) {
  #endif // PC_BUILD

  #if defined(DMCP_BUILD)
    void btnFnReleased(void *data) {
  #endif // DMCP_BUILD

    if(catalog) {
      resetAlphaSelectionBuffer();
    }

    if(programRunStop == PGM_KEY_PRESSED_WHILE_PAUSED) {
      programRunStop = PGM_RESUMING;
      screenUpdatingMode &= ~SCRUPD_ONE_TIME_FLAGS;
      return;
    }

    if(calcMode != CM_REGISTER_BROWSER && calcMode != CM_FLAG_BROWSER && calcMode != CM_ASN_BROWSER && calcMode != CM_FONT_BROWSER) {
      if(tam.mode == TM_KEY && !tam.keyInputFinished) {
        if(tam.digitsSoFar == 0) {
          switch(((char *)data)[0]) {
            case '1': {
              tamProcessInput(shiftG ? ITM_1 :                  ITM_0);
              tamProcessInput(shiftG ? ITM_3 : shiftF ? ITM_7 : ITM_1);
              break;
            }
            case '2': {
              tamProcessInput(shiftG ? ITM_1 :                  ITM_0);
              tamProcessInput(shiftG ? ITM_4 : shiftF ? ITM_8 : ITM_2);
              break;
            }
            case '3': {
              tamProcessInput(shiftG ? ITM_1 :                  ITM_0);
              tamProcessInput(shiftG ? ITM_5 : shiftF ? ITM_9 : ITM_3);
              break;
            }
            case '4': {
              tamProcessInput(     (shiftG || shiftF) ? ITM_1 : ITM_0);
              tamProcessInput(shiftG ? ITM_6 : shiftF ? ITM_0 : ITM_4);
              break;
            }
            case '5': {
              tamProcessInput(     (shiftG || shiftF) ? ITM_1 : ITM_0);
              tamProcessInput(shiftG ? ITM_7 : shiftF ? ITM_1 : ITM_5);
              break;
            }
            case '6': {
              tamProcessInput(     (shiftG || shiftF) ? ITM_1 : ITM_0);
              tamProcessInput(shiftG ? ITM_8 : shiftF ? ITM_2 : ITM_6);
              break;
            }
          }
          shiftF = shiftG = false;
          refreshScreen(107);
        }
        screenUpdatingMode &= ~SCRUPD_ONE_TIME_FLAGS;
        return;
      }

      if(calcMode == CM_ASSIGN && itemToBeAssigned != 0 && !(tam.alpha && tam.mode != TM_NEWMENU)) {
//Put section in if shifts are only allowed on the primary menu line
//        if( (  (itemToBeAssigned == ITM_SHIFTf || itemToBeAssigned == ITM_SHIFTg || itemToBeAssigned == KEY_fg) &&
//                (previousCalcMode == CM_NORMAL) &&
//                (shiftF || shiftG) &&
//                ((uint8_t *)data)[0] >= '1' &&
//                ((uint8_t *)data)[0] <= '6'
//                )  ) {       //prevent "shifts on rows f and g on F6 to be overwritten //Allow any normal mode menu HOME PFN MyM, except shifts not in f or g line
//          return;
//        }
//        else


////prevent "ALPHA" on F6 to be overwritten
//      if(!(previousCalcMode == CM_AIM && (!shiftG && !shiftF) && ((uint8_t *)data)[0] == '6')) {       //prevent "ALPHA" on F6 to be overwritten
          if(_assignToMenu((uint8_t *)data)) {
            if(previousCalcMode == CM_AIM) {         //vvJM btnFnReleased
              showSoftmenu(-MNU_ALPHA);              //
              screenUpdatingMode &= ~SCRUPD_MANUAL_STACK;
              refreshScreen(108);                       //
            }                                        //^^JM
            return;
          }
//      }
//      else {
//        return;
//      }
        }


      if(!releaseOverride) {
        btnFnReleased_StateMachine(NULL, data);            //This function does the longpress differentiation, and calls ExecuteFunctio below, via fnbtnclicked
        releaseOverride = false;
      }

      if(calcMode == CM_AIM) {
        refreshRegisterLine(REGISTER_T);
      }

      if(tam.alpha) {
        displayShiftAndTamBuffer();
      }
    }
    screenUpdatingMode &= ~SCRUPD_MANUAL_STATUSBAR; //Ensure status bar is always checked after fn key release. The new statusbar code does not clear the entire statusbar, it only updates if a setting changed

    fnTimerStop(TO_3S_CTFF);      //dr
    fnTimerStop(TO_CL_LONG);      //dr
  }






  /********************************************//**
   * \brief Executes one function from a softmenu
   * \return void
   ***********************************************/
  static void executeFunction(const char *data, int16_t item_) {
    int16_t item = ITM_NOP;
    FN_timed_out_to_NOP_or_Executed = true;

                    #if defined(VERBOSEKEYS)
                      printf("keyboard.c: executeFunction %i (beginning of executeFunction): %i, %s tam.mode=%i calcMode=%u aimBuffer=%s\n", item, currentMenu(), indexOfItems[-currentMenu()].itemSoftmenuName, tam.mode, calcMode, aimBuffer);
                    #endif //VERBOSEKEYS

    if(calcMode != CM_REGISTER_BROWSER && calcMode != CM_FLAG_BROWSER && calcMode != CM_ASN_BROWSER && calcMode != CM_FONT_BROWSER) {

      if(data[0] == 0) {
        item = item_;
      }
      else {
                    #if defined(VERBOSEKEYS)
                    printf(">>>> R000AA >>determineFunctionKeyItem_C47 %d |%s| shiftF=%d, shiftG=%d tam.mode=%i\n",item, data, shiftF, shiftG, tam.mode);
                    #endif //VERBOSEKEYS

        item = determineFunctionKeyItem_C47((char *)data, shiftF, shiftG);

                    #if defined(VERBOSEKEYS)
                    printf(">>>> R000AB >>determineFunctionKeyItem_C47 %d |%s| shiftF=%d, shiftG=%d tam.mode=%i\n",item, data, shiftF, shiftG, tam.mode);
                    #endif //VERBOSEKEYS

      if(calcMode == CM_NIM && (item == ITM_HASH_JM || item == ITM_toINT)) {
        addItemToNimBuffer(item);
        item = ITM_NOP;
      }



        lastKeyItemDetermined = item;
      }

      // in graph plot menu, wanting to change Normal Mode items, so open the correct menu first and return to Normal Mode, and stop the processing.
      if(calcMode == CM_GRAPH && currentMenu() == -MNU_PLOT_FUNC && (item == VAR_LX || item == VAR_UX)) {
        calcMode = CM_NORMAL;
        screenUpdatingMode = SCRUPD_AUTO;
        clearScreen(234);
        refreshScreen(127);
        showSoftmenu(-MNU_GRAPHS);
        item = 0;
      }

      // Update currentUserMenu for user defined menus selected in an existing function
      if((currentMenu() == -MNU_DYNAMIC) || (currentMenu() == -MNU_HOME) || (currentMenu() == -MNU_PFN)) {
        setCurrentUserMenu(item, userMenus[currentUserMenu].menuItem[dynamicMenuItem].argumentName);
      }


      // If specific pseudo menus are accessed in PEM, the special function code won't run and instead the associated menu must opne
      if(calcMode == CM_PEM && isFunctionItemAMenu(item)) {
        switch(item) {
          case ITM_GAP_R : item = -MNU_GAP_R;  break;
          case ITM_GAP_L : item = -MNU_GAP_L;  break;
          case ITM_GAP_RX: item = -MNU_GAP_RX; break;
          default:;
        }
      }


                    #if defined(VERBOSEKEYS)
                    printf(">>>> R000B                                %d |%s| shiftF=%d, shiftG=%d tam.mode=%i\n",item, data, shiftF, shiftG, tam.mode);
                    #endif //VERBOSEKEYS
                    #if defined(PC_BUILD) && defined(VERBOSE_MINIMUM)
                      printf(">>>Function selected: executeFunction data=|%s| f=%d g=%d tam.mode=%i\n",(char *)data, shiftF, shiftG, tam.mode);
                      printf("    item %s 0: calcMode=%u item=%d=%s f=%d g=%d\n",(item < 0 ? "< " : ">="),calcMode, item, getItemCatalogName(item), shiftF, shiftG);
                      fflush(stdout);
                    #endif //PC_BUILD

      resetShiftState();                               //shift cancelling delayed to this point after state machine

                    #if defined(VERBOSEKEYS)
                    printf(">>>> R000C                                %d |%s| shiftF=%d, shiftG=%d tam.mode=%i\n",item, data, shiftF, shiftG, tam.mode);
                    #endif //VERBOSEKEYS


      if(/*showFunctionNameItem*/item != 0) {
/* //JM vv Rmove the possibility for error by removing code that may conflict with the state machine
        item = showFunctionNameItem;
*/
        showFunctionNameItem = 0;
        if(calcMode != CM_CONFIRMATION && data[0] != 0) { //JM data is used if operation is from the real keyboard. item is used directly if called from XEQM
          lastErrorCode = 0;

          if(calcMode != CM_PEM && item == -MNU_Sfdx) {
            tamEnterMode(MNU_Sfdx);
            refreshScreen(109);
            screenUpdatingMode &= ~SCRUPD_ONE_TIME_FLAGS;
            return;
          }
          else if(calcMode != CM_PEM && (item == ITM_INTEGRAL || item == ITM_INTEGRAL_YX)) {
            switch(calcMode) {
              case CM_NIM: {
                closeNim();
                break;
              }
              case CM_AIM: {
                closeAim();
                break;
              }
              default: {
                // do nothing
              }
            }

            reallyRunFunction(item, currentSolverVariable);
            refreshScreen(110);
            screenUpdatingMode &= ~SCRUPD_ONE_TIME_FLAGS;
            return;
          }
          else if(item < 0) { // softmenu
            if(calcMode == CM_ASSIGN && itemToBeAssigned == 0 && currentMenu() == -MNU_MENUS) {
              itemToBeAssigned = item;
              leaveAsmMode();
              popSoftmenu();
            }
            else if(calcMode == CM_ASSIGN && itemToBeAssigned != 0 && (item == -MNU_HOME || item == -MNU_PFN || item == -MNU_MyMenu)) {
              itemToBeAssigned = item;
              leaveAsmMode();
              showSoftmenu(item);
            }
            else if((tam.mode == TM_MENU) && (item != -MNU_MENU) && !tam.alpha) {
              if ((currentMenu() ==  -MNU_TAMINDIRECT) && ((item == -MNU_VAR) || (item == -MNU_REG))) {
                showSoftmenu(item);
              }
              else {
                fnKeyInCatalog = 1;
                if(item < 0) {
                  item = -item;
                }
                addItemToBuffer(item);
              }
            }
            else {
                    #if defined(VERBOSEKEYS)
                      printf(">>>Function: executeFunction: calcMode=%u showSoftmenu(%d)\n",calcMode, item);
                    #endif //VERBOSEKEYS
                    #if defined(PC_BUILD) && defined(MONITOR_CLRSCR)
                      printf("BB1 screenUpdatingMode=%u temporaryInformation=%u\n", screenUpdatingMode, temporaryInformation);
                    #endif // PC_BUILD &&MONITOR_CLRSCR

              showSoftmenu(item);

                    #if defined(PC_BUILD) && defined(MONITOR_CLRSCR)
                      printf("BB2 screenUpdatingMode=%u temporaryInformation=%u\n", screenUpdatingMode, temporaryInformation);
                    #endif // PC_BUILD &&MONITOR_CLRSCR

              if(GRAPHMODE) {
                screenUpdatingMode = SCRUPD_AUTO;
                if(item == -MNU_GRAPHS) {
                  calcMode = CM_NORMAL;
                  fnUndo(NOPARAM);
                }
              }

              if(item == -MNU_ALPHA) {
                fnAim(0);
              }

              if((item == -MNU_Solver || item == -MNU_Grapher || item == -MNU_Sf || item == -MNU_1STDERIV || item == -MNU_2NDDERIV || item == -MNU_Sf_TOOL || item == -MNU_Solver_TOOL) && lastErrorCode != 0) {
                popSoftmenu();
                currentSolverStatus &= ~SOLVER_STATUS_INTERACTIVE;
                currentSolverStatus &= ~SOLVER_STATUS_EQUATION_MODE;
              }
            }

            refreshScreen(111);
            screenUpdatingMode &= ~SCRUPD_ONE_TIME_FLAGS;
            return;
          }

          if(tam.mode && catalog && (tam.digitsSoFar || isFunctionOldParam16(tam.function) || (!tam.indirect && (tam.mode == TM_VALUE || tam.mode == TM_VALUE_CHB || (tam.mode == TM_KEY && !tam.keyInputFinished))))) {
            // disabled
          }
          else if(tam.function == ITM_GTOP && catalog == CATALOG_PROG) {
            runFunction(item);
            leaveTamModeIfEnabled();
            hourGlassIconEnabled = false;
            _closeCatalog();
            refreshScreen(112);
            screenUpdatingMode &= ~SCRUPD_ONE_TIME_FLAGS;
            return;
          }

          else if((tam.mode || getItemFunc(item) != addItemToBuffer)               //skip if not label name (TAM) AND a bufferized letter
                   && calcMode == CM_PEM && !catalog &&        //allow only in case of PEM, and a CAT
                   (tam.mode == TM_FLAGR || tam.mode == TM_FLAGW) &&
                   !(tam.mode && tam.function == ITM_DELP)) { // TODO: is that correct   //don't allow DELP
            //printf("tam.function=%d indexOfItems[tam.function].cat=%s  item=%d indexOfItems[item].cat=%s (indexOfItems[item].param & 0xff)=%d \n",tam.function, indexOfItems[tam.function].itemCatalogName, item, indexOfItems[item].itemCatalogName , (indexOfItems[item].param & 0xff));
            if((tam.mode == TM_FLAGR || tam.mode == TM_FLAGW) && (item != ITM_INDIRECTION) && !tam.indirect) {
              tam.value = (indexOfItems[item].param & 0xff);
              addStepInProgram(tamOperation());
              leaveTamModeIfEnabled();
            }
          }

          else if((tam.mode || getItemFunc(item) != addItemToBuffer)               //skip if not label name (TAM) AND a bufferized letter
                   && calcMode == CM_PEM && catalog && catalog != CATALOG_MVAR &&        //allow only in case of PEM, and a CAT
                   !(tam.mode && tam.function == ITM_DELP)) { // TODO: is that correct   //don't allow DELP

            fnKeyInCatalog = 1;
            if(indexOfItems[item].func == fnGetSystemFlag && (tam.mode == TM_FLAGR || tam.mode == TM_FLAGW) && !tam.indirect) {
              tam.value = (indexOfItems[item].param & 0xff);
              tam.alpha = true;
              addStepInProgram(tamOperation());
              leaveTamModeIfEnabled();
            }
            else  if(indexOfItems[item].func == addItemToBuffer) {   //this section is added, it was commented out in btnFnPressed line 760, it is moved here, as longpress works on release.
              //Here we deal with PEM TAM mode menu entry, i.e. item's sent to buffer. See issue #454 context.
              if(getSystemFlag(FLAG_ALPHA)) {
                processAimInput(item); // sets keyActionProcessed
                if(tam.mode) {
                  //printf("cccc tam.mode=%i tam.f=%i Popping menu\n",tam.mode, tam.function);
                  popSoftmenu();
                }
              }
              else {
                addStepInProgram(item);    // I am not sure if this can actually be needed: It was in the btnFnPressed section in line 760
              }
              hourGlassIconEnabled = false;
            }
            else if(tam.mode) {
              const char *itmLabel = dynmenuGetLabel(dynamicMenuItem);
              uint16_t nameLength = stringByteLength(itmLabel);
              xcopy(aimBuffer, itmLabel, nameLength + 1);
              tam.alpha = true;
              addStepInProgram(tamOperation());
              leaveTamModeIfEnabled();
            }
            else {
                    #if defined(VERBOSEKEYS)
                      printf(">>>Function: executeFunction runFunction(%d)\n",item);
                    #endif //VERBOSEKEYS
              runFunction(item);
            }
            hourGlassIconEnabled = false;
            _closeCatalog();
            fnKeyInCatalog = 0;
            refreshScreen(113);
            screenUpdatingMode &= ~SCRUPD_ONE_TIME_FLAGS;
            return;
          }
          else if(calcMode == CM_PEM && !getSystemFlag(FLAG_ALPHA) && (item == ITM_toINT || item == ITM_HASH_JM)) {
            if(aimBuffer[0]!=0) {
              pemAddNumber(ITM_toINT, true);
              screenUpdatingMode &= ~SCRUPD_MANUAL_STACK;
              refreshScreen(115);
              return;
            }
            else {
              //no action, continue to insert the command
            }
          }

                    #if defined(VERBOSEKEYS)
                    printf(">>>> R000D                                %d |%s| shiftF=%d, shiftG=%d tam.mode=%i\n",item, data, shiftF, shiftG, tam.mode);
                    #endif //VERBOSEKEYS

          // If we are in the catalog then a normal key press should affect the Alpha Selection Buffer to choose
          // an item from the catalog, but a function key press should put the item in the AIM (or TAM) buffer
          // Use this variable to distinguish between the two
          fnKeyInCatalog = 1;
          if(tam.mode && catalog && (tam.digitsSoFar || isFunctionOldParam16(tam.function) || (!tam.indirect && (tam.mode == TM_VALUE || tam.mode == TM_VALUE_CHB)))) {
            // disabled
          }
          else if(tam.mode && (!tam.alpha || isAlphabeticSoftmenu()) && !(tam.mode == TM_VALUE && (item == ITM_TAMMAX || item == ITM_YY_TRACK || item == ITM_YY_OFF))) {
            bool_t isInConfig = tam.mode == TM_FLAGW && currentMenu() == -MNU_SYSFL;   //JM Do not drop out of SYSFLG

            //This section to auto-drop out of alpha submenu.
             if(menu(1) == -MNU_TAMALPHA && isAlphaSubmenu(0)) {
               popSoftmenu();
               --numberOfTamMenusToPop;
             }

            addItemToBuffer(item);

            if((currentMenu() == -MNU_MODE || currentMenu() == -MNU_PREF) && isInConfig && item != ITM_EXIT1 && item != ITM_BACKSPACE) { //JM do not drop out of SYSFLG
              fnCFGsettings(0);       //JM
            }                         //JM
          }
  //          else if((calcMode == CM_NORMAL || calcMode == CM_AIM) && isAlphabeticSoftmenu()) {
  //            if(calcMode == CM_NORMAL) {
  //              fnAim(NOPARAM);
  //            }
  //                          fnAim(NOPARAM);(item);
  //JM this is handled later
  //          }

          else if(calcMode == CM_EIM && ((catalog && catalog != CATALOG_MVAR) ||
                  ( currentMenu() == -MNU_EQ_EDIT &&                       //Allow only EIM enabled functions through to be added as text
                    item != ITM_EQ_LEFT && item != ITM_EQ_RIGHT && item != CHR_num && item != CHR_case &&
                    (indexOfItems[item].status & EIM_STATUS) == EIM_ENABLED
                    )
                  ))  {
            if(currentMenu() == -MNU_CONST) {  // Add # prefix for constants in equations
              addItemToBuffer(ITM_NUMBER_SIGN);
            }
            addItemToBuffer(item);
            while(currentMenu() != -MNU_EQ_EDIT) {
              popSoftmenu();
            }
          }
          else if((calcMode == CM_NORMAL || calcMode == CM_NIM) && (ITM_0<=item && item<=ITM_F) && (!catalog || catalog == CATALOG_MVAR)) {
            if(lastIntegerBase == 0) {
              lastIntegerBase = 16;
            }
            addItemToNimBuffer(item);
          }
          else if((calcMode == CM_NIM) && ((/*item==ITM_DRG ||*/ item == ITM_DMS2 || item == ITM_dotD) && !catalog)) {   //JM Remove DRG from here, there seems to be no need to send DRG to the buffer
            addItemToNimBuffer(item);
          }                                                                                      //JM
          else if(calcMode == CM_MIM && currentMenu() != -MNU_M_EDIT && (item != ITM_CC && item != ITM_op_j && item != ITM_op_j_pol)) { //JM added ITM_CC to let it work in matrix edit
            addItemToBuffer(item);
          }
          else if(item > 0) { // function
            if(calcMode == CM_NORMAL && (                          //switch off BASE mode using the HEX/DEC... buttons
                (lastIntegerBase ==  2 && item == ITM_2BIN) ||
                (lastIntegerBase ==  8 && item == ITM_2OCT) ||
                (lastIntegerBase == 10 && item == ITM_2DEC) ||
                (lastIntegerBase == 16 && item == ITM_2HEX)    )) {
              setLastintegerBasetoZero();
              screenUpdatingMode &= ~SCRUPD_MANUAL_MENU;
              goto noMoreToDo;
            }
            if(calcMode == CM_NIM && item != ITM_CC && item != ITM_op_j && item != ITM_op_j_pol && item != ITM_HASH_JM && item != ITM_toINT && item != ITM_ms) {  //JMNIM Allow NIM not closed, so that JMNIM can change the bases without ierrors thrown; also change from # to i
              closeNim();
              if(calcMode != CM_NIM) {
                if(indexOfItems[item].func == fnConstant) {
                  setSystemFlag(FLAG_ASLIFT);
                }
              }
            }
            if(item == ITM_KEYMAP) {
              cursorEnabled = false;               // cursor is re-activated automatically elsewhere, after button release
            }
            if(calcMode == CM_AIM && !(isAlphabeticSoftmenu() || isJMAlphaOnlySoftmenu() || item == ITM_KEYMAP)) {
              closeAim();
            }
            if(tam.mode && tam.alpha) {
              if(item == ITM_T_LEFT_ARROW) {
                fnAlphaCursorLeft(NOPARAM);
                tamProcessInput(ITM_T_LEFT_ARROW);      // To update the tam buffer
                goto noMoreToDo;
              }
              else if(item == ITM_T_RIGHT_ARROW) {
                fnAlphaCursorRight(NOPARAM);
                tamProcessInput(ITM_NOP);      // To update the tam buffer
                goto noMoreToDo;
              }
              else if(item == ITM_NOP) {
                tamProcessInput(ITM_NOP);      // To update the tam buffer
                goto noMoreToDo;
              }
            }
            if(tam.alpha && calcMode != CM_ASSIGN && tam.mode != TM_NEWMENU &&
              !( (tam.mode==TM_STORCL || tam.mode==TM_LABEL || tam.mode == TM_LBLONLY || tam.mode == TM_KEY || tam.mode == TM_M_DIM || tam.mode == TM_REGISTER || tam.mode == TM_CMP)
                  && (item == CHR_num || item == CHR_case || item == ITM_SCR || item == ITM_USERMODE) )
              ) {
              if(calcMode != CM_PEM || item != ITM_NOP) { // Here we left TAM in the context of issue #454
                leaveTamModeIfEnabled();
              }
            }
            else if(tam.mode == TM_VALUE && (item == ITM_TAMMAX || item == ITM_YY_TRACK || item == ITM_YY_OFF)) {
              leaveTamModeIfEnabled();
            }

                    #if defined(VERBOSEKEYS)
                    printf(">>>> R000E                                %d |%s| shiftF=%d, shiftG=%d tam.mode=%i\n",item, data, shiftF, shiftG, tam.mode);
                    #endif //VERBOSEKEYS

            if(lastErrorCode == 0) {
              if(temporaryInformation == TI_VIEW_REGISTER) {
                temporaryInformation = TI_NO_INFO;
                updateMatrixHeightCache();
              }
              else {
                temporaryInformation = TI_NO_INFO;
              }

              if(programRunStop == PGM_WAITING) {
                programRunStop = PGM_STOPPED;
              }
              if(calcMode == CM_ASSIGN && itemToBeAssigned == 0 && item != ITM_NOP) {
                if(item == CHR_case) {
                  SetSetting(JC_UC);
                }
                else if(item == CHR_num) {
                  SetSetting(JC_NL);
                }
                else if(tam.alpha) {
                  processAimInput(item); // sets keyActionProcessed
                  if(stringGlyphLength(aimBuffer) > 6) {
                    assignLeaveAlpha();
                    assignGetName1();
                  }
                }
                else if(item == ITM_AIM) { // in case α is already assigned
                  assignEnterAlpha();
                  keyActionProcessed = true;
                }
                else {
                  if(item == ITM_XEQ && dynamicMenuItem > -1) {
                    char *varCatalogItem = dynmenuGetLabel(dynamicMenuItem);
                    if (strcmp(varCatalogItem, "XEQ") != 0) {
                      calcRegister_t regist = findNamedLabel(varCatalogItem);
                      if(regist != INVALID_VARIABLE) {
                        item = regist - FIRST_LABEL + ASSIGN_LABELS;
                      }
                      else {
                        displayCalcErrorMessage(ERROR_LABEL_NOT_FOUND, ERR_REGISTER_LINE, REGISTER_X);
                        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
                          sprintf(errorMessage, "string '%s' is not a named label", varCatalogItem);
                          moreInfoOnError("In function executeFunction:", errorMessage, NULL, NULL);
                        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
                      }
                    }
                  }
                  else if(item == ITM_RCL && dynamicMenuItem > -1) {
                    char *varCatalogItem = dynmenuGetLabel(dynamicMenuItem);
                    if (strcmp(varCatalogItem, "RCL") != 0) {
                      calcRegister_t regist = findNamedVariable(varCatalogItem);
                      if(regist != INVALID_VARIABLE) {
                        item = regist - FIRST_NAMED_VARIABLE + ASSIGN_NAMED_VARIABLES;
                      }
                      else {
                        displayCalcErrorMessage(ERROR_LABEL_NOT_FOUND, ERR_REGISTER_LINE, REGISTER_X);
                        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
                          sprintf(errorMessage, "string '%s' is not a named variable", varCatalogItem);
                          moreInfoOnError("In function executeFunction:", errorMessage, NULL, NULL);
                        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
                      }
                    }
                  }
                  itemToBeAssigned = item;

                  if(previousCalcMode == CM_AIM && isAlphaSubmenu(0)) {                            //JMvv close menu to allow only one charac
                        popSoftmenu();
                        showSoftmenu(-MNU_MyAlpha); //push MyAlpha in case ALPHA is up (likely)
                      }
                }
              }
              else if(calcMode == CM_ASSIGN && tam.alpha && tam.mode != TM_NEWMENU && item != ITM_NOP) {
                processAimInput(item); // sets keyActionProcessed
                if(stringGlyphLength(aimBuffer) > 6) {
                  assignLeaveAlpha();
                  assignGetName2();
                }
              }
              else {
                    #if defined(VERBOSEKEYS)
                      printf("keyboard.c: executeFunction calcmode=%u %i (before runfunction): %i, %s tam.mode=%i\n", calcMode, item, currentMenu(), indexOfItems[-currentMenu()].itemSoftmenuName, tam.mode);
                    #endif //VERBOSEKEYS

                runFunction(item);

                if(calcMode == CM_EIM && !tam.mode) {
                  if(isAlphaSubmenu(0)) {
                    while(currentMenu() != -MNU_EQ_EDIT) {
                        popSoftmenu();
                      }
                  }
                }
                else if(((calcMode == CM_PEM && !tam.mode && getSystemFlag(FLAG_ALPHA)) || calcMode == CM_AIM) && indexOfItems[item].func == addItemToBuffer) {
                  popSoftmenu();
                }
                    #if defined(VERBOSEKEYS)
                      printf("keyboard.c: executeFunction calcmode=%u %i (after runfunction): %i, %s tam.mode=%i\n", calcMode, item, currentMenu(), indexOfItems[-currentMenu()].itemSoftmenuName, tam.mode);
                    #endif //VERBOSEKEYS
              }
            }
          }

          noMoreToDo:

                    #if defined(VERBOSEKEYS)
                    printf(">>>> R000F                                %d |%s| shiftF=%d, shiftG=%d tam.mode=%i\n",item, data, shiftF, shiftG, tam.mode);
                    #endif //VERBOSEKEYS

          _closeCatalog();
          fnKeyInCatalog = 0;
        }
        else if((calcMode == CM_CONFIRMATION) && (item == ITM_YES || item == ITM_NO)) {
          runFunction(item);
        }
      }
      else if(calcMode == CM_CONFIRMATION) {
        temporaryInformation = TI_ARE_YOU_SURE;      // Keep confirmation message on screen
      }
    }
                    #if defined(PC_BUILD) && defined(MONITOR_CLRSCR)
                      printf(">>>  refreshScreen3 from keyboard.c executeFunction calcMode=%u screenUpdatingMode=%u\n",calcMode, screenUpdatingMode);
                    #endif

    refreshScreen(114);
    //TODO 2023-04-15 check here. It needs to be changed not to always refresh the screen.

                    #if defined(PC_BUILD) && defined(MONITOR_CLRSCR)
                      printf(">>>  refreshScreen4 from keyboard.c executeFunction calcMode=%u screenUpdatingMode=%u\n",calcMode, screenUpdatingMode);
                    #endif

    screenUpdatingMode &= ~SCRUPD_ONE_TIME_FLAGS;
                    #if defined(VERBOSEKEYS)
                      printf("keyboard.c: executeFunction (end): calcmode=%u %i, %s\n", calcMode, currentMenu(), indexOfItems[-currentMenu()].itemSoftmenuName);
                    #endif //VERBOSEKEYS
  }


  bool_t shiftKeyClearsError = false;
  #define stringToKeyNumber(data)         ((*((char *)data) - '0')*10 + *(((char *)data)+1) - '0')    // input string = "28", keynumber = 28  (keys 00-36)


  static int16_t determineItem(const char *data) {
    delayCloseNim = false;
    int16_t result;
    const calcKey_t *key;

    dynamicMenuItem = -1;

    int8_t key_no = stringToKeyNumber(data);

                    #if defined(PC_BUILD)
                      char tmp[200]; sprintf(tmp,"^^^^^^^keyboard.c: determineitem: key_no: %d:", key_no); jm_show_comment(tmp);
                    #endif //PC_BUILD

    //.    if(kbd_usr[36].primaryTam == ITM_EXIT1) //opposite keyboard V43 LT, 43S, V43 RT
    key = getSystemFlag(FLAG_USER) ? (kbd_usr + key_no) : (kbd_std + key_no);
    //.    else
    //.      key = getSystemFlag(FLAG_USER) && ((calcMode == CM_NORMAL) || (calcMode == CM_AIM) || (calcMode == CM_NIM) || (calcMode == CM_EIM) || (calcMode == CM_PLOT_STAT) || (calcMode == CM_GRAPH) || (calcMode == CM_LISTXY)) ? (kbd_usr + key_no) : (kbd_std + key_no);    //JM Added (calcMode == CM_NORMAL) to prevent user substitution in AIM and TAM

    fnTimerExec(TO_FN_EXEC);                                  //dr execute queued fn

                    #if defined(PC_BUILD)
                      sprintf(tmp,"^^^^^^^keyboard.c: determineitem: key_no: %u, key->primary1: %d:", key_no, key->primary); jm_show_comment(tmp);
                    #endif //PC_BUILD

    if( (key->primary != ITM_SHIFTf) && (key->primary != KEY_fg) && ( !SHOWMODE || !(
                           key->primary == ITM_RCL
                           || key->primary == ITM_RS
                           || key->primary == ITM_UP1
                           || key->primary == ITM_DOWN1
                           || (allowShowDigits && key->primary >= ITM_0 && key->primary <= ITM_9))
                         ) ) {
      showRegis = 9999;                                      //clear showmode register
    }

    int16_t ShiftOverride = 0;
    result = Norm_Key_00_item_in_layout;
    ShiftOverride = Check_Norm_Key_00_Assigned(&result, key_no);
    #if defined(PC_BUILD) && defined(VERBOSE_DETERMINEITEM)
      printf("**[DL]** determineItem = %d\n",result);
    #endif //VERBOSE_DETERMINEITEM


    if(ShiftOverride == 0) {                              //disable long and double press if Sigma+ is shift g
      Setup_MultiPresses( key->primary );
    }

                    #if defined(PC_BUILD)
                      sprintf(tmp,"^^^^^^^keyboard.c: determineitem: key->primary2: %d:",key->primary); jm_show_comment(tmp);
                    #endif //PC_BUILD


    if(SHOWMODE && (key->primary == KEY_fg || key->primary == ITM_SHIFTf)) { //before going into shift handling, send EXIT over to the key release
      shiftF = true;
      shiftG = false;
      lastItem = key->primary;
      resetKeytimers();
      screenUpdatingMode = SCRUPD_MANUAL_STATUSBAR | SCRUPD_MANUAL_STACK | SCRUPD_MANUAL_MENU | SCRUPD_MANUAL_SHIFT_STATUS;
      return ITM_NOP;
    }



    // Shift f pressed and JM REMOVED shift g not active
    if((key->primary == ITM_SHIFTf || ShiftOverride == ITM_SHIFTf) && (calcMode == CM_NORMAL || calcMode == CM_AIM || calcMode == CM_NIM  || calcMode == CM_MIM || calcMode == CM_EIM || calcMode == CM_PEM || calcMode == CM_PLOT_STAT || calcMode == CM_GRAPH || calcMode == CM_ASSIGN || calcMode == CM_ASN_BROWSER || calcMode == CM_REGISTER_BROWSER || calcMode == CM_FLAG_BROWSER || calcMode == CM_FONT_BROWSER || calcMode == CM_TIMER)) {   //JM shifts
      if(temporaryInformation == TI_SHOW_REGISTER || SHOWMODE) shiftKeyClearsError = true; //JM
      Shft_LongPress_f_g = true;
      if(Shft_LongPress_f_g && getSystemFlag(FLAG_SH_LONGPRESS)) {
        fnTimerStart(TO_FG_LONG, TO_FG_LONG, JM_TO_FG_LONG * 1.5);    //vv dr
      }
      if(temporaryInformation == TI_VIEW_REGISTER) {
        temporaryInformation = TI_NO_INFO;
        updateMatrixHeightCache();
      }
      else {
        //reconsider
        temporaryInformation = TI_NO_INFO;     //reconsider: Temporary commented out. This clears SHOW (and other TI's) when fg is pressed. That means SNAP and shiftEXP are not possible with SHOW
      }
      if(lastErrorCode != 0) shiftKeyClearsError = true;                                                                                         //JM shifts
      if(programRunStop == PGM_WAITING) {
        programRunStop = PGM_STOPPED;
      }
      lastErrorCode = 0;

      shiftF = !shiftF;
      shiftG = false;
      lastshiftF = shiftF;
      lastshiftG = shiftG;
      if(SHOWMODE || currentMenu() == -MNU_SHOW) {
        closeShowMenu();
      }
      showShiftState();
      refreshModeGui();

      screenUpdatingMode &= ~SCRUPD_MANUAL_SHIFT_STATUS;
      return ITM_NOP;
    }
    // Shift g pressed and JM REMOVED shift f not active
    else if((key->primary == ITM_SHIFTg || ShiftOverride == ITM_SHIFTg) && (calcMode == CM_NORMAL || calcMode == CM_AIM || calcMode == CM_NIM  || calcMode == CM_MIM || calcMode == CM_EIM || calcMode == CM_PEM || calcMode == CM_PLOT_STAT || calcMode == CM_GRAPH || calcMode == CM_ASSIGN || calcMode == CM_ASN_BROWSER || calcMode == CM_REGISTER_BROWSER || calcMode == CM_FLAG_BROWSER || calcMode == CM_FONT_BROWSER || calcMode == CM_TIMER)) {   //JM shifts
      if(temporaryInformation == TI_SHOW_REGISTER || SHOWMODE) shiftKeyClearsError = true; //JM
      Shft_LongPress_f_g = true;
      if(Shft_LongPress_f_g && getSystemFlag(FLAG_SH_LONGPRESS)) {
        fnTimerStart(TO_FG_LONG, TO_FG_LONG, JM_TO_FG_LONG * 1.5);    //vv dr
      }
      if(temporaryInformation == TI_VIEW_REGISTER) {
        temporaryInformation = TI_NO_INFO;
        updateMatrixHeightCache();
      }
      else {
        //reconsider
        temporaryInformation = TI_NO_INFO;     //reconsider: Temporary commented out. This clears SHOW (and other TI's) when fg is pressed. That means SNAP and shiftEXP are not possible with SHOW
      }
      if(lastErrorCode != 0) shiftKeyClearsError = true;                                                                                         //JM shifts
      if(programRunStop == PGM_WAITING) {
        programRunStop = PGM_STOPPED;
      }
      lastErrorCode = 0;

      shiftF = false;
      shiftG = !shiftG;
      lastshiftF = shiftF;
      lastshiftG = shiftG;
      if(SHOWMODE || currentMenu() == -MNU_SHOW) {
        closeShowMenu();
      }
      showShiftState();
      refreshModeGui();

      screenUpdatingMode &= ~SCRUPD_MANUAL_SHIFT_STATUS;
      return ITM_NOP;
    }

    // JM Shift fg pressed  //JM shifts change f/g to a single function key toggle to match DM42 keyboard
    else if((key->primary == KEY_fg || ShiftOverride == KEY_fg) && (calcMode == CM_NORMAL || calcMode == CM_AIM || calcMode == CM_NIM  || calcMode == CM_MIM || calcMode == CM_EIM || calcMode == CM_PEM || calcMode == CM_PLOT_STAT || calcMode == CM_GRAPH || calcMode == CM_ASSIGN || calcMode == CM_ASN_BROWSER || calcMode == CM_REGISTER_BROWSER || calcMode == CM_FLAG_BROWSER || calcMode == CM_FONT_BROWSER || calcMode == CM_TIMER)) {   //JM shifts
      Shft_LongPress_f_g = false;
      Shft_timeouts = true;
      fnTimerStart(TO_FG_LONG, TO_FG_LONG, JM_TO_FG_LONG);    //vv dr
      if(getSystemFlag(FLAG_SHFT_4s)) {
        fnTimerStart(TO_FG_TIMR, TO_FG_TIMR, JM_SHIFT_TIMER); //^^
      }
      if(temporaryInformation == TI_VIEW_REGISTER || SHOWMODE) shiftKeyClearsError = true; //JM
      if(temporaryInformation != TI_NO_INFO) {
        screenUpdatingMode &= ~SCRUPD_MANUAL_STACK;
        if(temporaryInformation == TI_VIEW_REGISTER) {
          temporaryInformation = TI_NO_INFO;
          updateMatrixHeightCache();
        }
        else {
          //reconsider
          temporaryInformation = TI_NO_INFO;     //reconsider: Temporary commented out. This clears SHOW (and other TI's) when fg is pressed. That means SNAP and shiftEXP are not possible with SHOW
        }
      }

      if(lastErrorCode != 0) shiftKeyClearsError = true;
      if(programRunStop == PGM_WAITING) {
        programRunStop = PGM_STOPPED;
      }
      lastErrorCode = 0;

      fg_processing_jm();

      lastshiftF = shiftF;
      lastshiftG = shiftG;
      if(SHOWMODE || currentMenu() == -MNU_SHOW) {
        closeShowMenu();
      }
      showShiftState();
      refreshModeGui();

      screenUpdatingMode &= ~SCRUPD_MANUAL_SHIFT_STATUS;
      return ITM_NOP;
    }

    else if((key->primary == KEY_fg || key->primary == ITM_SHIFTf || key->primary == ITM_SHIFTg) && (calcMode == CM_PLOT_STAT || calcMode == CM_LISTXY)) {   //JM shifts
      temporaryInformation = TI_NO_INFO;     //reconsider: Temporary commented out. This clears SHOW (and other TI's) when fg is pressed. That means SNAP and shiftEXP are not possible with SHOW

      if(lastErrorCode != 0) shiftKeyClearsError = true;                                                                                         //JM shifts
      if(programRunStop == PGM_WAITING) {
        programRunStop = PGM_STOPPED;
      }
      lastErrorCode = 0;                                                                                                      //JM shifts
      return ITM_NOP;
    }

                    #if defined(PC_BUILD)
                      sprintf(tmp,"^^^^^^^keyboard.c: determineitem: key->primary3: %d:",key->primary); jm_show_comment(tmp);
                    #endif //PC_BUILD
                                                                                                                         //JM shifts
    if( !tam.mode && (calcMode == CM_NIM || calcMode == CM_NORMAL) && (lastIntegerBase >= 2 && getSystemFlag(FLAG_TOPHEX)) && (key_no >= 0 && key_no <= 5 )) {               //JMNIM vv Added direct A-F for hex entry
      result = shiftF ? key->fShifted :
               shiftG ? key->gShifted :
                        key->primaryAim;
      switch(result){
        case ITM_SHIFTf:
        case ITM_SHIFTg:
        case KEY_fg:
          result = ITM_NOP;
          break;
        default:break;
      }
      //printf(">>> ±±±§§§ keys key:%d result:%d Calmode:%d, nimbuffer:%s, lastbase:%d, nimnumberpart:%d\n",key_no, result, calcMode,nimBuffer,lastIntegerBase, nimNumberPart);
      Check_MultiPresses(&result, key_no);        //JM
      return result;
    }
    else                                                                                                                        //JM^^

    if(calcMode == CM_AIM || (catalog && catalog != CATALOG_MVAR && calcMode != CM_NIM) || calcMode == CM_EIM || tam.alpha || (calcMode == CM_ASSIGN && (previousCalcMode == CM_AIM || previousCalcMode == CM_EIM)) || (calcMode == CM_PEM && getSystemFlag(FLAG_ALPHA))) {
      result = shiftF ? key->fShiftedAim :
               shiftG ? key->gShiftedAim :
                        key->primaryAim;
      if(calcMode == CM_PEM && getSystemFlag(FLAG_ALPHA)) {
        if(result == ITM_DOWN_ARROW || scrLock == NC_SUBSCRIPT) {
          nextChar = NC_SUBSCRIPT;
        }
        else if(result == ITM_UP_ARROW || scrLock == NC_SUPERSCRIPT) {
          nextChar = NC_SUPERSCRIPT;
        }
      }
      else if((result == ITM_COMMA || result == ITM_PERIOD) && (calcMode == CM_EIM || calcMode == CM_AIM) && getSystemFlag(FLAG_ALPHA) ) {
        switch((shiftG ? 2 : 0) + (getSystemFlag(FLAG_NUMLOCK) ? 1 : 0)) {                // gSHIFTED  numLock
        //case 0: result = key->primaryAim;break;           //                                   0        0      key->primaryAim
          case 1: result = RADIX34_MARK_DEC_ITM; break;     //                                   0        1      decimal
        //case 2: result = RADIX34_MARK_DEC_ITM; break;     //                                   2        0      decimal
          case 3: result = RADIX34_MARK_NOT_DEC_ITM; break; //                                   2        1      not the decimal
          default:;
        }
      }
      if ((calcMode == CM_EIM) && (result == -MNU_AIMCATALOG)) {
        result = -MNU_EIMCATALOG;
      }
    }
    else if(tam.mode) {
      result = key->primaryTam; // No shifted function in TAM
    }
    else if(calcMode == CM_NORMAL || calcMode == CM_NIM || calcMode == CM_MIM || calcMode == CM_FONT_BROWSER || calcMode == CM_FLAG_BROWSER || calcMode == CM_ASN_BROWSER || calcMode == CM_REGISTER_BROWSER || calcMode == CM_BUG_ON_SCREEN || calcMode == CM_CONFIRMATION || calcMode == CM_PEM || GRAPHMODE || calcMode == CM_ASSIGN || calcMode == CM_TIMER  || calcMode == CM_LISTXY) {
      result = shiftF ? key->fShifted :
               shiftG ? key->gShifted :
                        key->primary;
      if(calcMode == CM_REGISTER_BROWSER) {
        if (shiftF && key->primaryAim >= ITM_A && key->primaryAim <= ITM_Z) {
          result = key->primaryAim;
        }
      }
    }
    else {
      displayBugScreen(bugScreenItemNotDetermined);
      result = 0;
    }

                    #if defined(PC_BUILD)
                      sprintf(tmp,"^^^^^^^keyboard.c: determineitem: result1: %d:",result); jm_show_comment(tmp);
                    #endif //PC_BUILD

    if(Check_Norm_Key_00_Assigned(&result, key_no) == 0) {
      Check_MultiPresses(&result, key_no);        //JM
    }

                    #if defined(PC_BUILD)
                      sprintf(tmp,"^^^^^^^keyboard.c: determineitem: result3: %d:",result); jm_show_comment(tmp);
                    #endif //PC_BUILD

    if(result == ITM_PROD_SIGN) {
      result = (getSystemFlag(FLAG_MULTx) ? ITM_CROSS : ITM_DOT);
    }

//    if((shiftF || shiftG) && result != ITM_SNAP) {
//      screenUpdatingMode &= ~SCRUPD_MANUAL_SHIFT_STATUS;
//      clearShiftState();
//    }

    if(result != ITM_SNAP) {    //not using the above, the timers need stopping and if the shifts are cleared from screen the flags must follow.
      resetShiftState();
    }

    if(calcMode == CM_ASSIGN && itemToBeAssigned != 0 && (result == ITM_NOP || result == ITM_NULL)) {
      result = ITM_LBL;
    }

    if(calcMode == CM_REGISTER_BROWSER) {
      switch(key->primary) {
        case ITM_0:
        case ITM_1:
        case ITM_2:
        case ITM_3:
        case ITM_4:
        case ITM_5:
        case ITM_6:
        case ITM_7:
        case ITM_8:
        case ITM_9:
        // case ITM_PERIOD:  // None of these keys correspond to letters
        // case ITM_RS:
        // case ITM_UP1:
        // case ITM_DOWN1:
        // case ITM_EXIT1:
        // case ITM_ENTER:
        case ITM_RCL: {
          break;
        }
        default: {
          if (key->primaryAim >= ITM_A && key->primaryAim <= ITM_Z) {
            result = key->primaryAim;
          }
        }
      }
    }
    #if defined(PC_BUILD) && defined(VERBOSE_DETERMINEITEM)
      printf("**[DL]** determineItem = %d\n",result);
    #endif //VERBOSE_DETERMINEITEM
    return result;
  }



  #if defined(PC_BUILD)
    void btnClicked(GtkWidget *notUsed, gpointer data) {
      GdkEvent mouseButton;
      mouseButton.button.button = 1;
      mouseButton.type = 0;

      btnPressed(notUsed, &mouseButton, data);
      btnReleased(notUsed, &mouseButton, data);
  }
  #endif // PC_BUILD

  #if defined(DMCP_BUILD)
    void btnClicked(void *unused, void *data) {
      btnPressed(data);
      btnReleased(data);
    }
  #endif // DMCP_BUILD

bool_t nimWhenButtonPressed = false;                  //PHM eRPN 2021-07


#if defined(PC_BUILD)
  void btnClickedP(GtkWidget *w, gpointer data) {                          //JM PRESSED FOR KEYBOARD F REPEAT
    GdkEvent mouseButton;
    mouseButton.button.button = 1;
    mouseButton.type = 0;
    btnPressed(w, &mouseButton, data);
  }

  void btnClickedR(GtkWidget *w, gpointer data) {                          //JM PRESSED FOR KEYBOARD F REPEAT
    GdkEvent mouseButton;
    mouseButton.button.button = 1;
    btnReleased(w, &mouseButton, data);
  }
#endif //PC_BUILD




#if defined(PC_BUILD)
    void btnPressed(GtkWidget *notUsed, GdkEvent *event, gpointer data) {
#endif //PC_BUILD
  #if defined(DMCP_BUILD)
    void btnPressed(void *data) {
  #endif //DMCP_BUILD

      reDraw = false;
      nimWhenButtonPressed = (calcMode == CM_NIM);                  //PHM eRPN 2021-07
      lastT_cursorPos = T_cursorPos;

      int16_t item;
      int keyCode = (*((char *)data) - '0')*10 + *(((char *)data) + 1) - '0';
      currentKeyCode = keyCode;

      asnKey[0] = ((uint8_t *)data)[0];
      asnKey[1] = ((uint8_t *)data)[1];
      asnKey[2] = 0;

     if(programRunStop == PGM_RUNNING || programRunStop == PGM_PAUSED) {
        #if defined(PC_BUILD)
          setLastKeyCode(keyCode + 1);
        #elif defined(DMCP_BUILD)
          lastKeyCode = keyCode;
        #endif
      }
      else {
        lastKeyCode = 0;
      }

      #if defined(PC_BUILD)
        if(event->type == GDK_DOUBLE_BUTTON_PRESS || event->type == GDK_TRIPLE_BUTTON_PRESS) { // return unprocessed for double or triple click
          return;
        }
        if(event->button.button == 2) { // Middle click
          shiftF = true;
          shiftG = false;
        }
        if(event->button.button == 3) { // Right click
          shiftF = false;
          shiftG = true;
        }
      #endif //PC_BUILD

      bool_t f = shiftF;
      bool_t g = shiftG;
      bool_t ff = lastshiftF;
      bool_t gg = lastshiftG;
      lastshiftF = shiftF;
      lastshiftG = shiftG;

      #if defined(DMCP_BUILD)
        //      if(keyAutoRepeat) {            // AUTOREPEAT
        //        //beep(880, 50);
        //        item = previousItem;
        //      }
        //      else {
      #endif //DMCP_BUILD

      item = determineItem((char *)data);
      lastKeyItemDetermined = item;

      #if defined(DMCP_BUILD)
        //  previousItem = item;
        //}
      #endif //DMCP_BUILD
      #if defined(PC_BUILD)
                    #if defined(VERBOSEKEYS)
                      printf(">>>>Z 1001 btnPressed       data=|%s| data[0]=%u item=%d calcMode=%u\n", (char *)data, ((char *)data)[0], item, calcMode);
                    #endif // VERBOSEKEYS
        if(programRunStop == PGM_RUNNING || programRunStop == PGM_PAUSED) {
          if((item == ITM_RS || item == ITM_EXIT1) && !getSystemFlag(FLAG_INTING) && !getSystemFlag(FLAG_SOLVING)) {
            programRunStop = PGM_WAITING;
            showFunctionNameItem = 0;
          }
          else if(programRunStop == PGM_PAUSED) {
            programRunStop = PGM_KEY_PRESSED_WHILE_PAUSED;
          }
          return;
        }
      #elif defined(DMCP_BUILD)
        if(calcMode == CM_PEM && (item == ITM_SST || item == ITM_BST)) {
          shiftF = f;
          shiftG = g;
        }
      #endif //DMCP_BUILD

      char *funcParam = "";

      keyStateCode = (getSystemFlag(FLAG_ALPHA) ? 3 : 0) + (g ? 2 : f ? 1 : 0);
      if(getSystemFlag(FLAG_USER)) {
        funcParam = (char *)getNthString((uint8_t *)userKeyLabel, keyCode * 6 + keyStateCode);
        xcopy(tmpString, funcParam, stringByteLength(funcParam) + 1);
      }
      else if((keyCode == Norm_Key_00_key) && (keyStateCode == 0) && Norm_Key_00.used && !(lastIntegerBase >= 2 && getSystemFlag(FLAG_TOPHEX))) {
        funcParam = Norm_Key_00.funcParam;
        xcopy(tmpString, funcParam, stringByteLength(funcParam) + 1);
      }
      else {
        *tmpString = 0;
      }

      showFunctionNameItem = 0;
                    #if defined(PC_BUILD)
                      char tmp[200]; sprintf(tmp,"^^^^btnPressed START item=%d data=\'%s\'",item,(char *)data); jm_show_comment(tmp);
                    #endif //PC_BUILD

      if(item != ITM_NOP && item != ITM_NULL) {

                    #if defined(PC_BUILD_TELLTALE)
                      sprintf(tmp,"keyboard.c: btnPressed 1--> processKeyAction(%d) which is str:%s\n",item,(char *)data); jm_show_calc_state(tmp);
                    #endif //PC_BUILD_TELLTALE

        processKeyAction(item);

                    #if defined(PC_BUILD_TELLTALE)
                      sprintf(tmp,"keyboard.c: btnPressed 2--> processKeyAction(%d) which is str:%s; keyActionProcessed=%u\n",item,(char *)data, keyActionProcessed); jm_show_calc_state(tmp);
                    #endif //PC_BUILD_TELLTALE

        if(!keyActionProcessed) {
          showFunctionName(item, 1000, funcParam);// "SF:B"); // 1000ms = 1s
        }
      }
      else if(calcMode == CM_REGISTER_BROWSER){
        screenUpdatingMode = SCRUPD_AUTO;
        screenUpdatingMode |= SCRUPD_SKIP_STATUSBAR_ONE_TIME;
        refreshScreen(126);
      }

      switch(tam.function) {  // ensure TAM input update the status bar
        //case ITM_SETSDIGS:
        //case ITM_RNG   :
        case ITM_DENMAX2 :
        case ITM_WSIZE   :
        case ITM_SETFDIGS: {
          screenUpdatingMode &= ~SCRUPD_MANUAL_STATUSBAR;
          refreshStatusBar();
        }
      }


      if(calcMode == CM_ASSIGN && itemToBeAssigned != 0 && tamBuffer[0] == 0) {
        shiftF = f;
        shiftG = g;
        lastshiftF = ff;
        lastshiftG = gg;
      }

//      if(calcMode != CM_LISTXY) {
//        refreshScreen(140);
//      }
                    #if defined(PC_BUILD)
                      sprintf(tmp,"^^^^btnPressed End item=%d:\'%s\' showFunctionNameItem=%d\n",item,(char *)data,showFunctionNameItem); jm_show_comment(tmp);
                    #endif //PC_BUILD
    }


#if defined(PC_BUILD)

    char key[3] = {0, 0, 0};
    static void convertXYToKey(int x, int y) {
      int xMin, xMax, yMin, yMax;
      key[0] = 0;
      key[1] = 0;
      key[2] = 0;

      for(int i=0; i<43; i++) {
        xMin = calcKeyboard[i].x;
        yMin = calcKeyboard[i].y;
        if(i == 10 && currentBezel == 2 && (tam.mode == TM_LABEL || tam.mode == TM_LBLONLY || (tam.mode == TM_SOLVE && (tam.function != ITM_SOLVE || calcMode != CM_PEM)) || (tam.mode == TM_KEY && tam.keyInputFinished))) {
          xMax = xMin + calcKeyboard[10].width[3];
          yMax = yMin + calcKeyboard[10].height[3];
        }
        else {
          xMax = xMin + calcKeyboard[i].width[currentBezel];
          yMax = yMin + calcKeyboard[i].height[currentBezel];
        }

        if(   xMin <= x && x <= xMax
           && yMin <= y && y <= yMax) {
          if(i < 6) { // Function key
            key[0] = '1' + i;
          }
          else {
            key[0] = '0' + (i - 6)/10;
            key[1] = '0' + (i - 6)%10;
          }
          break;
        }
      }
    }

    void frmCalcMouseButtonPressed(GtkWidget *notUsed, GdkEvent *event, gpointer data) {
      if(key[0] == 0) { // The previous click must be released
        convertXYToKey((int)event->button.x, (int)event->button.y);
        if(key[0] == 0) {
          return;
        }

        if(key[1] == 0) { // Soft function key
          btnFnPressed(NULL, event, (gpointer)key);
        }
        else { // Not a soft function key
          btnPressed(NULL, event, (gpointer)key);
        }
      }
    }

    void frmCalcMouseButtonReleased(GtkWidget *notUsed, GdkEvent *event, gpointer data) {
      if(key[0] == 0) {
        return;
      }

      if(key[1] == 0) { // Soft function key
        btnFnReleased(NULL, event, (gpointer)key);
      }
      else { // Not a soft function key
        btnReleased(NULL, event, (gpointer)key);
      }

      key[0] = 0;
    }
  #endif // PC_BUILD



  bool_t checkKeyShifts(const char *data) {
    const calcKey_t *key;

    int8_t key_no = stringToKeyNumber(data);

    key = getSystemFlag(FLAG_USER) && ((calcMode == CM_NORMAL) || (calcMode == CM_AIM) || (calcMode == CM_EIM) || (calcMode == CM_NIM)) ? (kbd_usr + key_no) : (kbd_std + key_no);

    if(key->primary == ITM_SHIFTf || key->primary == ITM_SHIFTg || key->primary == KEY_fg) {
      return true;
    }
    else {
      return false;
    }
  }




  #if defined(PC_BUILD)
    void btnReleased(GtkWidget *notUsed, GdkEvent *event, gpointer data) {
                    #if defined(PC_BUILD) && defined(MONITOR_CLRSCR)
                      printf(">>> btnReleased showFunctionNameItem=%i screenUpdatingMode=%d temporaryInformation=%u tam=%i getSystemFlag(FLAG_ALPHA)=%i\n", showFunctionNameItem, screenUpdatingMode, temporaryInformation, tam.mode, getSystemFlag(FLAG_ALPHA));
                    #endif // PC_BUILD &&MONITOR_CLRSCR
                    jm_show_calc_state("##### keyboard.c: btnReleased begin: showFunctionNameItem");
  #endif // PC_BUILD
  #if defined(DMCP_BUILD)
    void btnReleased(void *data) {
  #endif // DMCP_BUILD
      int keyCode = (*((char *)data) - '0')*10 + *(((char *)data) + 1) - '0';

      if(SHOWMODE && (lastItem == KEY_fg || lastItem == ITM_SHIFTf) && lastItem != SCREENDUMP) {
        //f is delayed in SHOW to release. fg and f both will perform the f-function. F-DISP will be screen dump.
        fg_processing_jm();
        shiftF = true;
        shiftG = false;
        lastshiftF = shiftF;
        lastshiftG = shiftG;
        lastItem = 0;
        if(SHOWMODE || currentMenu() == -MNU_SHOW) {
          closeShowMenu();
        }
        showShiftState();
        refreshModeGui();
        screenUpdatingMode &= ~SCRUPD_MANUAL_SHIFT_STATUS;
      }
      if(SHOWMODE) {
        lastItem = 0;
      }




      if(temporaryInformation == TI_SHOWNOTHING) return;

      int16_t item;
      Shft_timeouts = false;                         //JM SHIFT NEW
      Shft_LongPress_f_g = false;
      JM_auto_longpress_enabled = 0;                 //JM TIMER CLRCLSTK ON LONGPRESS

      if(programRunStop == PGM_KEY_PRESSED_WHILE_PAUSED) {
        programRunStop = PGM_RESUMING;
        screenUpdatingMode &= ~SCRUPD_ONE_TIME_FLAGS;
        return;
      }

      if(calcMode == CM_ASN_BROWSER && lastItem == ITM_PERIOD) {
        fnAsnDisplayUSER = true;
        lastItem = 0;
        goto RELEASE_END;
      }

      if(calcMode == CM_LISTXY) {
        return;
      }

      //printf("release: showFunctionNameItem=%i calcMode=%i lastItem = %i keyActionProcessed=%i showFunctionNameItem=%i releaseOverride=%i tam.mode=%i tamBuffer=%s tamBuffer[0]=%u\n", showFunctionNameItem, calcMode, lastItem, keyActionProcessed, showFunctionNameItem, releaseOverride, tam.mode, tamBuffer, tamBuffer[0]);

      screenUpdatingMode |= SCRUPD_MANUAL_MENU;
      screenUpdatingMode &= ~SCRUPD_SKIP_MENU_ONE_TIME;

      if(calcMode == CM_ASSIGN && itemToBeAssigned != 0 && tamBuffer[0] == 0) {
        assignToKey((char *)data);
        if(previousCalcMode == CM_AIM) {             //vv JM RETURN TO AIM MODE
          showSoftmenu(-MNU_ALPHA);                  //
        }                                            //^^ JM
        calcMode = previousCalcMode;
        shiftF = shiftG = false;
        screenUpdatingMode = SCRUPD_AUTO;
        screenUpdatingMode |= SCRUPD_SKIP_STATUSBAR_ONE_TIME;
        refreshScreen(116);
      }
      else if(showFunctionNameItem != 0) {
        item = showFunctionNameItem;
                    #if defined(PC_BUILD)
                      char tmp[200]; sprintf(tmp,"^^^^btnReleased %d:\'%s\'",item,(char *)data); jm_show_comment(tmp);
                    #endif //PC_BUILD

        if(calcMode == CM_NIM && delayCloseNim && item != ITM_ms && item != ITM_CC && item != ITM_op_j && item != ITM_op_j_pol && item != ITM_dotD && item != ITM_HASH_JM && item != ITM_toINT) {
          delayCloseNim = false;
          closeNim();                 //JM moved here, from bufferize see JMCLOSE, to retain NIM if needed for .ms. Only a problem due to longpress.
                    #if defined(PC_BUILD)
                      printf("btnReleased 1: Closed NIM (delayed) delayCloseNim=%u\n",delayCloseNim);
                    #endif
          screenUpdatingMode &= ~SCRUPD_MANUAL_MENU;
        }

        fnTimerStop(TO_3S_CTFF);
        hideFunctionName();

        bool_t Norm_Key_00_released = !getSystemFlag(FLAG_USER) && (keyStateCode == 0) && (keyCode == Norm_Key_00_key) && Norm_Key_00.used && (!(lastIntegerBase >= 2 && getSystemFlag(FLAG_TOPHEX)));

        char *funcParam = (Norm_Key_00_released ? Norm_Key_00.funcParam : (char *)getNthString((uint8_t *)userKeyLabel, keyCode * 6 + keyStateCode));
                    #if defined(PC_BUILD) && defined(VERBOSE_DETERMINEITEM)
                      printf("**[DL]** btnReleased1 - item %d showFunctionNameArg %s funcParam %s\n",item,showFunctionNameArg,funcParam);
                    #endif //VERBOSE_DETERMINEITEM
        if(showFunctionNameArg != NULL) {
          funcParam = showFunctionNameArg;       // Needed when executing a user menu from a long pressed key
                    #if defined(PC_BUILD) && defined(VERBOSE_DETERMINEITEM)
                      printf("**[DL]** btnReleased2 - item %d showFunctionNameArg %s funcParam %s\n",item,showFunctionNameArg,funcParam);
                    #endif //VERBOSE_DETERMINEITEM
        }

        if(item < 0) {
          setCurrentUserMenu(item, funcParam);
          showSoftmenu(item);
          screenUpdatingMode &= ~SCRUPD_MANUAL_MENU;
            //printf("AA2 allowShiftsToClearError=%u !checkShifts=%u screenUpdatingMode=%u temporaryInformation=%u\n",allowShiftsToClearError, !checkShifts((char *)data), screenUpdatingMode, temporaryInformation);
        }
        else {
        #if defined(PC_BUILD)
          if(item == ITM_RS || item == ITM_XEQ) {
            key[0] = 0;
          }
        #endif // PC_BUILD

          if(item != ITM_NOP && tam.alpha && indexOfItems[item].func != addItemToBuffer && aimBuffer[0] == 0) {
            // We are in TAM mode so need to cancel first (equivalent to EXIT)
            if(item == ITM_EXIT1) {
              if(currentMenu() == -MNU_TAMALPHA){
                popSoftmenu();
                numberOfTamMenusToPop--;
                tam.alpha = false;
              }
            }
            else if(item != ITM_BACKSPACE) {          // [DL] to ensure backspace will be processed in tamProcessInput
              leaveTamModeIfEnabled();
            }
          }
          if(item == ITM_EXIT1 && tam.alpha && aimBuffer[0] != 0)  {
            if(currentMenu() == -MNU_TAMALPHA){
              popSoftmenu();
              numberOfTamMenusToPop--;
              aimBuffer[0] = 0;
              tam.alpha = false;
            }
          }
          if(item == ITM_RCL && (getSystemFlag(FLAG_USER) || Norm_Key_00_released) && funcParam[0] != 0) {
            calcRegister_t var = findNamedVariable(funcParam);
            if(var != INVALID_VARIABLE) {
              if(calcMode == CM_PEM) {  // Insert user variable recall in program
                #if defined(PC_BUILD) && defined(VERBOSE_DETERMINEITEM)
                  printf("**[DL]** insertUserItemInProgram(item=%d, funcParam=%s)\n",item,funcParam);
                #endif //VERBOSE_DETERMINEITEM
                insertUserItemInProgram(item, funcParam);
              }
              else {                    // Execute item
                #if defined(PC_BUILD) && defined(VERBOSE_DETERMINEITEM)
                  printf("**[DL]** reallyRunFunction(item=%d, var=%d, funcParam=%s)\n",item,var,funcParam);
                #endif //VERBOSE_DETERMINEITEM
                reallyRunFunction(item, var);
              }
            }
            else {
              displayCalcErrorMessage(ERROR_UNDEF_SOURCE_VAR, ERR_REGISTER_LINE, REGISTER_X);
              #if (EXTRA_INFO_ON_CALC_ERROR == 1)
                sprintf(errorMessage, "string '%s' is not a named variable", funcParam);
                moreInfoOnError("In function btnReleased:", errorMessage, NULL, NULL);
              #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
            }
          }
          else if(item == ITM_XEQ && (getSystemFlag(FLAG_USER) || Norm_Key_00_released) && funcParam[0] != 0) {
            calcRegister_t label = findNamedLabel(funcParam);
            if(label != INVALID_VARIABLE) {
              if(calcMode == CM_PEM) {  // Insert user program call in program
                #if defined(PC_BUILD) && defined(VERBOSE_DETERMINEITEM)
                  printf("**[DL]** insertUserItemInProgram(item=%d, funcParam=%s)\n",item,funcParam);
                #endif //VERBOSE_DETERMINEITEM
                insertUserItemInProgram(item, funcParam);
              }
              else {                    // Execute item
                #if defined(PC_BUILD) && defined(VERBOSE_DETERMINEITEM)
                  printf("**[DL]** reallyRunFunction(item=%d, label=%d, funcParam=%s)\n",item,label,funcParam);
                #endif //VERBOSE_DETERMINEITEM
                reallyRunFunction(item, label);
              }
            }
            else {
              displayCalcErrorMessage(ERROR_LABEL_NOT_FOUND, ERR_REGISTER_LINE, REGISTER_X);
              #if (EXTRA_INFO_ON_CALC_ERROR == 1)
                sprintf(errorMessage, "string '%s' is not a named label", funcParam);
                moreInfoOnError("In function btnReleased:", errorMessage, NULL, NULL);
              #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
            }
          }
          else {
                    #if defined(PC_BUILD) && defined(MONITOR_CLRSCR)
                      printf("btnReleased 2: Closed NIM (delayed) delayCloseNim=%u, ",delayCloseNim);
                      printf("runfunction (%d) tam=%i getSystemFlag(FLAG_ALPHA)=%i \n",item, tam.mode, getSystemFlag(FLAG_ALPHA));
                      printf(">>> btnReleased runfunction(%i) calcMode=%d previousCalcMode=%d screenUpdatingMode=%d\n", item, calcMode, previousCalcMode, screenUpdatingMode);    //JMYY
                    #endif // PC_BUILD &&MONITOR_CLRSCR

            if(item == ITM_SNAP) {
//              screenUpdatingMode = SCRUPD_AUTO;
              refreshScreen(137);
            }

            if(CM_NIM && (item == ITM_AIM || item == ITM_XEQ || item == ITM_GTO) && nimNumberPart == NP_INT_BASE && aimBuffer[strlen(aimBuffer) - 1] == '#') {  //do not allow shortinteger base change input to exit when alpha is pressed
              goto RELEASE_END;
            }

            if(item == ITM_BASEMENU) {
              leaveTamModeIfEnabled();
            }

              runFunction(item);

                    #if defined(PC_BUILD) && defined(MONITOR_CLRSCR)
                      printf(">>> btnReleased ran(%i) calcMode=%d previousCalcMode=%d screenUpdatingMode=%d\n", item, calcMode, previousCalcMode, screenUpdatingMode);    //JMYY
                      printf("    runfunction (%d) tam=%i getSystemFlag(FLAG_ALPHA)=%i \n",item, tam.mode, getSystemFlag(FLAG_ALPHA));
                    #endif // PC_BUILD &&MONITOR_CLRSCR
          }
        }
      }

      if(programRunStop == PGM_SINGLE_STEP) {     // Key pressed was SST
        programRunStop = PGM_STOPPED;
        runProgram(true, INVALID_VARIABLE);       // Execute one program step after key released
      }

//  #if defined(DMCP_BUILD)
//      else if(keyAutoRepeat) {         // AUTOREPEAT
//        btnPressed(data);
//      }
//  #endif // DMCP_BUILD

RELEASE_END:

      //printf("BB allowShiftsToClearError=%u !checkShifts=%u screenUpdatingMode=%u\n",allowShiftsToClearError, !checkShifts((char *)data), screenUpdatingMode);

      switch(last_CM) {
        case CM_REGISTER_BROWSER:
        case CM_FLAG_BROWSER:
        case CM_ASN_BROWSER:
        case CM_FONT_BROWSER: {
          screenUpdatingMode = SCRUPD_AUTO;
          break;
        }
        default: {
          if(PROBMENU) {
            screenUpdatingMode &= ~(SCRUPD_MANUAL_STACK | SCRUPD_SKIP_STACK_ONE_TIME);
          }
          screenUpdatingMode |= SCRUPD_SKIP_STATUSBAR_ONE_TIME;
          break;
        }
      }

    bool_t preventRefreshAtTheEndOfReleasedKey =
      ( !shiftKeyClearsError && checkKeyShifts((char *)data) ) ||
      ( (lastKeyItemDetermined == ITM_UP1 || lastKeyItemDetermined == ITM_DOWN1) && calcMode == CM_GRAPH );

    if(!preventRefreshAtTheEndOfReleasedKey) {
                    #if defined(PC_BUILD) && defined(MONITOR_CLRSCR)
                      printf(">>> END of btnReleased after (117) calcMode=%d previousCalcMode=%d screenUpdatingMode=%d\n", calcMode, previousCalcMode, screenUpdatingMode);    //JMYY
                    #endif // PC_BUILD &&MONITOR_CLRSCR
      refreshScreen(117);
    }

    screenUpdatingMode &= ~SCRUPD_ONE_TIME_FLAGS;
    shiftKeyClearsError = false;

    fnTimerStop(TO_CL_LONG);

    }


#endif //!TESTSUITE_BUILD
  void leavePem(void) {
    if(freeProgramBytes >= 4) { // Push the programs to the end of RAM
      uint32_t newProgramSize = (uint32_t)((uint8_t *)(ram + RAM_SIZE_IN_BLOCKS) - beginOfProgramMemory) - (freeProgramBytes & 0xfffc);
      uint16_t localStepNumber = currentLocalStepNumber;
      uint16_t programNumber = currentProgramNumber;
      uint16_t fdLocalStepNumber = firstDisplayedLocalStepNumber;
      bool_t inRam = (programList[currentProgramNumber - 1].step > 0);
      if(inRam) { // Not in flash
        currentStep           += (freeProgramBytes & 0xfffc);
        firstDisplayedStep    += (freeProgramBytes & 0xfffc);
        beginOfCurrentProgram += (freeProgramBytes & 0xfffc);
        endOfCurrentProgram   += (freeProgramBytes & 0xfffc);
      }
      freeProgramBytes &= 0x03;
      resizeProgramMemory(TO_BLOCKS(newProgramSize));
      scanLabelsAndPrograms();
      if(inRam) {
        currentLocalStepNumber = localStepNumber;
        currentProgramNumber = programNumber;
        firstDisplayedLocalStepNumber = fdLocalStepNumber;
        defineCurrentStep();
        defineFirstDisplayedStep();
        defineCurrentProgramFromCurrentStep();
      }
    }
  }

#if !defined(TESTSUITE_BUILD)
  void processKeyAction(int16_t item) {

                    #if defined(PC_BUILD) && defined(MONITOR_CLRSCR)
                      printf(">>>> processKeyAction: calcMode=%u item=%d  programRunStop=%d lastErrorCode=%u SHOWMODE=%u screenUpdatingMode=%i\n",calcMode, item, programRunStop, lastErrorCode, SHOWMODE, screenUpdatingMode);
                    #endif // PC_BUILD &&MONITOR_CLRSCR

    keyActionProcessed = false;

    if(lastErrorCode != 0 && item != ITM_EXIT1 && item != ITM_BACKSPACE) {
      lastErrorCode = 0;
      screenUpdatingMode = SCRUPD_AUTO;
      screenUpdatingMode |= SCRUPD_SKIP_STATUSBAR_ONE_TIME;
      refreshScreen(138);
    }

    if(temporaryInformation == TI_VIEW_REGISTER) {
      temporaryInformation = TI_NO_INFO;
      updateMatrixHeightCache();
      if(item == ITM_UP1 || item == ITM_DOWN1 || item == ITM_EXIT1 || item == ITM_BACKSPACE) {
        temporaryInformation = TI_VIEW_REGISTER;
      }
    }
    else if(temporaryInformation != TI_NO_INFO && item != ITM_UP1 && item != ITM_DOWN1 && item != ITM_EXIT1 && item != ITM_BACKSPACE &&
           !(  (  item == ITM_RCL || item == ITM_RS || (item >= ITM_0 && item <= ITM_9 && allowShowDigits)  ) && SHOWMODE  ) ) {
      if(SHOWMODE) {
        closeShowMenu();
      }
      temporaryInformation = TI_NO_INFO;
      screenUpdatingMode = SCRUPD_AUTO;    //cannot use MENU & STACK update due to being in NIM, and NIM prevents clearing individually
      screenUpdatingMode |= SCRUPD_SKIP_STATUSBAR_ONE_TIME;
    }

    if(calcMode == CM_GRAPH && currentMenu() == -MNU_PLOT_FUNC && ((item >= ITM_0 && item <= ITM_9) || item == ITM_PERIOD)) { //incoming digit, change modes and go to GRAPHS input page
      calcMode = CM_NORMAL;
      showSoftmenu(-MNU_GRAPHS);
      screenUpdatingMode &= SCRUPD_MANUAL_MENU;
      refreshScreen(125);
    }

    if(programRunStop == PGM_WAITING) {
      programRunStop = PGM_STOPPED;
    }

    #if (REAL34_WIDTH_TEST == 1)
      longInteger_t lgInt;
      longIntegerInit(lgInt);
    #endif // (REAL34_WIDTH_TEST == 1)

    if(item == KEY_COMPLEX && calcMode == CM_MIM) {   //JM Allow COMPLEX to function as CC if in Matrix
      item = ITM_CC;
    }

    if(calcMode == CM_NORMAL && SHOWMODE && currentMenu() != -MNU_EQN) {
      switch(item) {
        case ITM_UP1:
        case ITM_DOWN1:
        case ITM_RS:
        {
          fnC47Show(item);
          keyActionProcessed = true;
//        refreshScreen(00);
          return;
          break;
        }

        default:break;
      }
    }

    if(GRAPHMODE) {                         //disregard any TI that may be up, as it is not possible to show in Graph Mode
      temporaryInformation = TI_NO_INFO;
    }

    if(GRAPHMODE && item != ITM_BACKSPACE && item != ITM_EXIT1 && item != ITM_UP1 && item != ITM_DOWN1 && item != ITM_SNAP ) {
      keyActionProcessed = true;
    }
    else if(calcMode == CM_ASN_BROWSER && item != ITM_PERIOD && item != ITM_USERMODE && item != ITM_BACKSPACE && item != ITM_EXIT1 && item != ITM_UP1 && item != ITM_DOWN1) {
      keyActionProcessed = true;
    }
    else {
      switch(item) {
        case ITM_BACKSPACE: {
          if(calcMode == CM_NIM || calcMode == CM_AIM || calcMode == CM_EIM) {
            temporaryInformation = TI_NO_INFO;
            refreshRegisterLine(NIM_REGISTER_LINE); }
          else if(tam.mode) {
            screenUpdatingMode &= ~SCRUPD_MANUAL_STACK;
          }
          else if(calcMode == CM_PEM) {
            //convert to Release action: let backspace run through to Release and bypass the next fnKeyBackspace()
            break;
          }
          else {
            //JM No if needed, it does nothing if not in NIM. TO DISPLAY NUMBER KEYPRESS DIRECTLY AFTER PRESS, NOT ONLY UPON RELEASE          break;
            keyActionProcessed = true;   //JM move this to before fnKeyBackspace to allow fnKeyBackspace to cancel it if needed to allow this function via timing out to NOP, and this is incorporated with the CLRDROP
            fnKeyBackspace(NOPARAM);
            if(calcMode != CM_CONFIRMATION) {
              temporaryInformation = TI_NO_INFO;
            }
          }
          break;
        }

        case ITM_UP1: {
          if(calcMode != CM_CONFIRMATION) {
            keyActionProcessed = true;   //JM swapped to before fnKeyUp to be able to check if key was processed below. Chose to process it here, as fnKeyUp does not have access to item.
            fnKeyUp(NOPARAM);
            if(!keyActionProcessed) {    //JMvv
              addItemToBuffer(ITM_UP_ARROW);    //Let the arrows produce arrow up and arrow down in ALPHA mode
            }                            //JM^^
            if(calcMode != CM_LISTXY && (currentSoftmenuScrolls() || !(calcMode == CM_NORMAL || calcMode == CM_PEM) || temporaryInformation != TI_NO_INFO)) {
              screenUpdatingMode &= ~(SCRUPD_MANUAL_MENU | SCRUPD_MANUAL_STACK);
              refreshScreen(118);
            }
            keyActionProcessed = true;
            #if (REAL34_WIDTH_TEST == 1)
              if(++largeur > SCREEN_WIDTH) {
                largeur--;
              }
              uInt32ToLongInteger(largeur, lgInt);
              convertLongIntegerToLongIntegerRegister(lgInt, REGISTER_Z);
            #endif // (REAL34_WIDTH_TEST == 1)
          }
          else {
            keyActionProcessed = true;
          }
          break;
        }

        case ITM_DOWN1: {
          if(calcMode != CM_CONFIRMATION) {
            keyActionProcessed = true;   //swapped to before fnKeyUp to be able to check if key was processed below. Chose to process it here, as fnKeyUp does not have access to item.
            fnKeyDown(NOPARAM);
            if(!keyActionProcessed){     //JM
              addItemToBuffer(ITM_DOWN_ARROW);    //Let the arrows produce arrow up and arrow down in ALPHA mode
            }                            //JM^^
            if(calcMode != CM_LISTXY && (currentSoftmenuScrolls() || !(calcMode == CM_NORMAL || calcMode == CM_PEM) || temporaryInformation != TI_NO_INFO)) {
              screenUpdatingMode &= ~(SCRUPD_MANUAL_MENU | SCRUPD_MANUAL_STACK);
              refreshScreen(119);
            }
            keyActionProcessed = true;
            #if (REAL34_WIDTH_TEST == 1)
              if(--largeur < 20) {
                largeur++;
              }
              uInt32ToLongInteger(largeur, lgInt);
              convertLongIntegerToLongIntegerRegister(lgInt, REGISTER_Z);
            #endif // (REAL34_WIDTH_TEST == 1)
          }
          else {
            keyActionProcessed = true;
          }
          break;
        }

        case ITM_EXIT1: {
          if(SHOWMODE || calcMode == CM_LISTXY) {    //do action on press, instead of release
            fnKeyExit(NOPARAM);
            keyActionProcessed = true;            //Removed to force EXIT on the RELEASE cycle to make it do fnKeyExit later to allow NOP
          }
          else if(calcMode == CM_PEM) {
            if(getSystemFlag(FLAG_ALPHA)) {          //close AIM in PEM
              fnKeyExit(NOPARAM);
              keyActionProcessed = true;
            }
          }
          if((temporaryInformation != TI_NO_INFO) && (calcMode != CM_CONFIRMATION)) {
            temporaryInformation = TI_NO_INFO;
            keyActionProcessed = true;
            screenUpdatingMode &= ~SCRUPD_MANUAL_STACK;
            refreshScreen(120);
          }
          else if(lastErrorCode != 0) {
            lastErrorCode = 0;
            screenUpdatingMode = SCRUPD_AUTO;
            refreshRegisterLine(ERR_REGISTER_LINE);   //[DL] added to force error line refresh
            refreshScreen(139);
            keyActionProcessed = true;
          }
          else if(temporaryInformation == TI_NO_INFO && softmenuStack[0].softmenuId == 0) {
            //Test to see if the statusbar flicker speed is hig enough. Exit only refreshes the statusbar when MyM is up and no TI
            screenUpdatingMode &= ~(SCRUPD_MANUAL_STATUSBAR | SCRUPD_SKIP_STATUSBAR_ONE_TIME);
            refreshScreen(140);
          }
          break;
        }

        case ITM_op_j_pol:
        case ITM_op_j:
        case ITM_CC:
        {
          #if defined(PC_BUILD)
            //printf("Monitor: item=%i calcMode=%i nimNumberPart=%i char=%c lastIntegerBase=%i\n",item, calcMode, nimNumberPart, aimBuffer[strlen(aimBuffer) - 1], lastIntegerBase);
          #endif
          if(calcMode == CM_ASSIGN) {
            if(itemToBeAssigned == 0) {
              itemToBeAssigned = item;
            }
            else {
              tamBuffer[0] = 0;
            }
            keyActionProcessed = true;
          }
          else if(calcMode == CM_REGISTER_BROWSER || calcMode == CM_FLAG_BROWSER || calcMode == CM_ASN_BROWSER || calcMode == CM_FONT_BROWSER || calcMode == CM_TIMER) {
            keyActionProcessed = true;
          }
          break;
        }

        case ITM_ENTER: {
          if(calcMode == CM_ASSIGN) {
            if(itemToBeAssigned == 0) {
              if(tam.alpha) {
                assignLeaveAlpha();
                assignGetName1();
                if(softmenu[softmenuStack[1].softmenuId].menuItem == -MNU_ALPHA) {
                  popSoftmenu();
                }
                if(softmenu[softmenuStack[0].softmenuId].menuItem == -MNU_ALPHA) {
                  popSoftmenu();
                }
              }
              else {
                itemToBeAssigned = ASSIGN_CLEAR;
                if(previousCalcMode == CM_AIM) {
                  showSoftmenu(-MNU_MyAlpha); //JM push MyAlpha in case ALPHA is up (likely)
                }
              }
            }
            else {
              if(tam.alpha && tam.mode != TM_NEWMENU) {
                assignLeaveAlpha();
                assignGetName2();
              }
              else if(tam.alpha) {
                tamBuffer[0] = 0;
              }
            }
            keyActionProcessed = true;
          }
          else if(calcMode == CM_REGISTER_BROWSER || calcMode == CM_FLAG_BROWSER || calcMode == CM_ASN_BROWSER || calcMode == CM_FONT_BROWSER) {
            keyActionProcessed = true;
          }
          else if(calcMode == CM_CONFIRMATION) {
            temporaryInformation = TI_ARE_YOU_SURE;      // Keep confirmation message on screen
            keyActionProcessed = true;
          }
          else if(tam.mode) {

            // To TEST and investigate 2023-10-02
            // User menu name create: ASN + USER 'DDD' has a problem by exiting to MyAlpha

            tamProcessInput(ITM_ENTER);
            keyActionProcessed = true;
          }
          else if(calcMode == CM_NIM) {                             //JMvv
            addItemToBuffer(item);
            keyActionProcessed = true;
          }                                                         //JM^^
          break;
        }

        case CHR_caseUP: {                                                   //From keyboard: logic for Up/Dn case/num
          if(getSystemFlag(FLAG_NUMLOCK))  {}
          else if(alphaCase == AC_LOWER) {
            processKeyAction(CHR_case);
          }
          else if(alphaCase == AC_UPPER) {
            processKeyAction(CHR_numL);
          }
          nextChar = NC_NORMAL;
          keyActionProcessed = true;
          break;
        }

        case CHR_caseDN: {                                                   //From keyboard: logic for Up/Dn case/num
          if(getSystemFlag(FLAG_NUMLOCK)) {
            alphaCase = AC_UPPER;
            processKeyAction(CHR_numU);
          }
          else if(alphaCase == AC_UPPER) {
            processKeyAction(CHR_case);
          }
          nextChar = NC_NORMAL;
          keyActionProcessed = true;
          break;
        }

        case CHR_numL: {
          if(!getSystemFlag(FLAG_NUMLOCK)) {
            processKeyAction(CHR_num);
          }
          keyActionProcessed = true;
          break;
        }
        case CHR_numU: {
          if(getSystemFlag(FLAG_NUMLOCK)) {
            processKeyAction(CHR_num);
          }
          keyActionProcessed = true;
          break;
        }
        case CHR_num: {                                                      //Toggle numlock from shifted arrow shortcut
          alphaCase = AC_UPPER;
          fnFlipFlag(FLAG_NUMLOCK);
          if(!getSystemFlag(FLAG_NUMLOCK)) {
            nextChar = NC_NORMAL;
          }
          showAlphaModeonGui(); //dr JM, see keyboardtweaks
          keyActionProcessed = true;
          break;
        }

        case CHR_case: {                                                      //Toggle capslock from shifted arrow shortcut
          clearSystemFlag(FLAG_NUMLOCK);
          int16_t sm = currentMenu();
          nextChar = NC_NORMAL;
          if(alphaCase == AC_LOWER) {
            alphaCase = AC_UPPER;
            if(sm == -MNU_alpha_omega || sm == -MNU_ALPHAintl) {
              softmenuStack[0].softmenuId--; // Switch case menu
            }
          }
          else {
            alphaCase = AC_LOWER;
            if(sm == -MNU_ALPHA_OMEGA || sm == -MNU_ALPHAINTL) {
              softmenuStack[0].softmenuId++; // Switch case menu
            }
          }
          showAlphaModeonGui(); //dr JM, see keyboardtweaks
          keyActionProcessed = true;
          break;
        }


        default: {
                    #if defined(PC_BUILD) && ((defined VERBOSEKEYS) || (defined MONITOR_CLRSCR))
                      printf("Switch - default: processKeyAction: calcMode=%d itemToBeAssigned=%d item=%d SHOWMODE=%u\n",calcMode, itemToBeAssigned, item, SHOWMODE);
                    #endif //PC_BUILD
          if(calcMode == CM_ASSIGN && itemToBeAssigned != 0 && item == ITM_USERMODE) {
            while(softmenuStack[0].softmenuId > 1) {
              popSoftmenu();
            }
            if(previousCalcMode == CM_AIM) {
              softmenuStack[0].softmenuId = 1;
              calcModeAimGui();
            }
            else {
              leaveAsmMode();
            }
            keyActionProcessed = true;
          }
          else if(calcMode == CM_ASSIGN && itemToBeAssigned == 0 && item == ITM_USERMODE) {
            tamEnterMode(ITM_USERMODE);
            calcMode = previousCalcMode;
            keyActionProcessed = true;
          }
          else if(calcMode == CM_ASSIGN && item == ITM_AIM) {
            assignEnterAlpha();
            keyActionProcessed = true;
          }
          else if((calcMode != CM_PEM || !getSystemFlag(FLAG_ALPHA)) && catalog && catalog != CATALOG_MVAR) {
            if(ITM_A <= item && item <= ITM_Z && lowercaseselected) {
              addItemToBuffer(item + (ITM_a - ITM_A));
              keyActionProcessed = true;
            }
            else if(item == ITM_DOWN_ARROW || item == ITM_UP_ARROW) {
              addItemToBuffer(item);
              keyActionProcessed = true;
            }
            break;
          }
          else if(tam.mode) {
            if(tam.alpha) {
              if(indexOfItems[item].func == addItemToBuffer || item < 0) {
                processAimInput(item); // sets keyActionProcessed
              }
              else {
                keyActionProcessed = true;
              }
            }
            else {
              #if defined(DMCP_BUILD)
                  wait_for_key_release(0);
                  key_pop();
              #endif //DMCP_BUILD
              addItemToBuffer(item);
              #if defined(DMCP_BUILD)
                  key_push(0);
              #endif //DMCP_BUILD
              keyActionProcessed = true;
            }
            break;
          }

          else if(item == ITM_SNAP) {
            switch(calcMode) { //place modes here which should not work with SNAP
              //case CM_REGISTER_BROWSER:
              //case CM_FLAG_BROWSER:
              //case CM_FONT_BROWSER:
                break;
              default: {
                runFunction(item);
                keyActionProcessed = true;
                break;
              }
            }
          }

          else {
            switch(calcMode) {
              case CM_NORMAL: {
                #if defined(PC_BUILD_VERBOSE0)
                   #if defined(PC_BUILD)
                     printf("$"); //####
                   #endif
                #endif
                if(SHOWMODE) {
                  //printf("XXXXXXXX @@@@@@ temporaryInformation=%u calcmode=%u showRegis=%u\n", temporaryInformation, calcMode, showRegis);
                  if(item == ITM_RCL) {
                    keyActionProcessed = true;
                    fnRecall(showRegis);
                    setSystemFlag(FLAG_ASLIFT);
                    temporaryInformation = TI_COPY_FROM_SHOW;
                    closeShowMenu();
                  }
                  else if(ITM_0 <= item && item <= ITM_9 && allowShowDigits) {
                    keyActionProcessed = true;
                    if(showRegis%10 == 0 && showRegis <=90) {
                      showRegis += (item - ITM_0);
                    }
                    else {
                      showRegis = (item - ITM_0)*10;
                    }
                    fnC47Show(ITM_NOP);
                    //refreshScreen(139);
                  }
                }

                else if(item == ITM_EXPONENT || item == ITM_PERIOD || (ITM_0 <= item && item <= ITM_9)) {
                  addItemToNimBuffer(item);
                  refreshRegisterLine(REGISTER_X);
                  keyActionProcessed = true;
                }
                // Following commands do not timeout to NOP
                else if(item == ITM_UNDO || item == ITM_BST || item == ITM_SST || item == ITM_PR || item == ITM_AIM || item == ITM_SNAP) {
                  runFunction(item);
                  keyActionProcessed = true;
                }
                break;
              }

              //also AIM Longpress cycle
              case CM_AIM: {
                //JM In AIM, BST and SST is not reaching here, as it is reconfigured for CAPS lock and NUM lock
                if(item == ITM_BST || item == ITM_SST) {
                  closeAim();
                  runFunction(item);
                  keyActionProcessed = true;
                }
                else {
                  screenUpdatingMode &= ~(SCRUPD_MANUAL_STACK | SCRUPD_SKIP_STACK_ONE_TIME);
                  processAimInput(item); // sets keyActionProcessed
                  refreshRegisterLine(AIM_REGISTER_LINE);   //TO DISPLAY KEYPRESS DIRECTLY AFTER PRESS, NOT ONLY UPON RELEASE
                }
                break;
              }

              case CM_EIM: {
                    #if defined(PC_BUILD_VERBOSE0)
                       #if defined(PC_BUILD)
                         printf("^^^^^ screenUpdatingMode=%u\n",screenUpdatingMode); //####
                       #endif
                    #endif

                processAimInput(item); // sets keyActionProcessed
                screenUpdatingMode &= ~(SCRUPD_MANUAL_MENU | SCRUPD_SKIP_MENU_ONE_TIME);
                refreshScreen(130);
                break;
              }

              case CM_NIM: {
                    #if defined(PC_BUILD_VERBOSE0)
                      #if defined(PC_BUILD)
                        printf("&"); //####
                      #endif
                    #endif

                if(item == ITM_BST || item == ITM_SST) {
                  closeNim();
                  runFunction(item);
                  keyActionProcessed = true;
                }
                else {
                  keyActionProcessed = true;
                  if(item == ITM_toINT || item == ITM_HASH_JM) {
                    //printf("0. NIM Resetstate\n");
                    resetShiftState();
                  }

                  if(calcMode == CM_NIM && (item == ITM_RI || item == ITM_dotD) && (nimNumberPart == NP_INT_10 || nimNumberPart == NP_INT_16) && lastIntegerBase > 0) {
                    //printf("1. Change NIM to LI\n");
                    lastIntegerBase = 0;
                    nimNumberPart = (hexDigits == 0 ? NP_INT_10 : NP_INT_16);
                    screenUpdatingMode &= ~SCRUPD_MANUAL_STATUSBAR;
                    resetShiftState();
                    screenUpdatingMode &= ~SCRUPD_MANUAL_STACK;
                    screenUpdatingMode &= ~SCRUPD_SKIP_STACK_ONE_TIME;
                    keyActionProcessed = true;
                  }
                  else if(calcMode == CM_NIM && (item == ITM_RI || item == ITM_dotD) && nimNumberPart == NP_INT_BASE && aimBuffer[strlen(aimBuffer) - 1] == '#') {
                    //printf("2. NIM remove base # to LI B\n");
                    lastIntegerBase = 0;
                    screenUpdatingMode &= ~SCRUPD_MANUAL_STATUSBAR;
                    resetShiftState();
                    addItemToNimBuffer(ITM_BACKSPACE);
                    keyActionProcessed = true;
                  }
                  else if(calcMode == CM_NIM && item == ITM_HASH_JM && nimNumberPart == NP_INT_BASE && aimBuffer[strlen(aimBuffer) - 1] == '#') {
                    //printf("3. NIM remove base # to LI A\n");
                    screenUpdatingMode &= ~SCRUPD_MANUAL_STATUSBAR;
                    resetShiftState();
                    screenUpdatingMode &= ~SCRUPD_MANUAL_STACK;
                    screenUpdatingMode &= ~SCRUPD_SKIP_STACK_ONE_TIME;
                    addItemToNimBuffer(ITM_BACKSPACE);
                    keyActionProcessed = true;
                  }
                  else if(calcMode == CM_NIM && item == ITM_PERIOD && nimNumberPart == NP_INT_BASE && aimBuffer[strlen(aimBuffer) - 1] == '#') {
                    //printf("4. NIM replace base # with .\n");
                    lastIntegerBase = 0;
                    screenUpdatingMode &= ~SCRUPD_MANUAL_STATUSBAR;
                    resetShiftState();
                    addItemToNimBuffer(ITM_BACKSPACE);
                    addItemToNimBuffer(ITM_PERIOD);
                    refreshRegisterLine(REGISTER_X);
                    keyActionProcessed = true;
                  }
                  else if(calcMode == CM_NIM && item == ITM_HASH_JM && nimNumberPart == NP_REAL_FLOAT_PART && aimBuffer[strlen(aimBuffer) - 1] == '.') {
                    //printf("5. NIM replace . with #\n");
                    lastIntegerBase = 0;
                    screenUpdatingMode &= ~SCRUPD_MANUAL_STATUSBAR;
                    resetShiftState();
                    addItemToNimBuffer(ITM_BACKSPACE);
                    addItemToNimBuffer(ITM_toINT);
                    refreshRegisterLine(REGISTER_X);
                    keyActionProcessed = true;
                  }

                  else {
                    //printf("6. NIM Addtobuffer\n");
                    addItemToNimBuffer(item);
                  }

                  if( ((ITM_0 <= item && item <= ITM_9) || item == ITM_toINT || item == ITM_HASH_JM || item == ITM_ms || ((ITM_A <= item && item <= ITM_F) && (lastIntegerBase >= 2) && getSystemFlag(FLAG_TOPHEX)) ) || item == ITM_CHS || item == ITM_EXPONENT || item == ITM_PERIOD) {   //JMvv Direct keypresses; //JMNIM Added direct A-F for hex entry
                    //printf("7. NIM Refresh\n");
                    refreshRegisterLine(REGISTER_X);
                  }                                                                                   //JM^^
                }
                break;
              }

              case CM_MIM: {
                addItemToBuffer(item);
                keyActionProcessed = true;
                break;
              }

              case CM_REGISTER_BROWSER: {
                if(item == ITM_PERIOD) {
                  rbr1stDigit = true;
                  if(rbrMode == RBR_GLOBAL) {
                    if(currentNumberOfLocalRegisters != 0) {
                      rbrMode = RBR_LOCAL;
                      currentRegisterBrowserScreen = FIRST_LOCAL_REGISTER;
                    }
                    else {
                      rbrMode = RBR_NAMED;
                      currentRegisterBrowserScreen = FIRST_NAMED_VARIABLE;
                    }
                  }
                  else if(rbrMode == RBR_LOCAL) {
                    rbrMode = RBR_NAMED;
                    currentRegisterBrowserScreen = FIRST_NAMED_VARIABLE;
                  }
                  else if(rbrMode == RBR_NAMED) {
                    rbrMode = RBR_GLOBAL;
                    currentRegisterBrowserScreen = REGISTER_X;
                  }
                }
                else if(item == ITM_RS) {
                  rbr1stDigit = true;
                  showContent = !showContent;
                }
                else if(item == ITM_RCL) {
                  rbr1stDigit = true;
                  calcMode = previousCalcMode;
                  if(rbrMode == RBR_GLOBAL || rbrMode == RBR_LOCAL) {
                    fnRecall(currentRegisterBrowserScreen);
                    screenUpdatingMode = SCRUPD_AUTO;
                    refreshScreen(128);
                  }
                  else if(rbrMode == RBR_NAMED) {
                    if(currentRegisterBrowserScreen >= FIRST_NAMED_VARIABLE + numberOfNamedVariables) { // Reserved variables
                      currentRegisterBrowserScreen -= FIRST_NAMED_VARIABLE + numberOfNamedVariables;
                      currentRegisterBrowserScreen += FIRST_RESERVED_VARIABLE + NUMBER_OF_LETTERED_VARIABLES;
                    }
                    fnRecall(currentRegisterBrowserScreen);
                  }
                  setSystemFlag(FLAG_ASLIFT);
                  temporaryInformation = TI_STORCL;
                  lastParam = currentRegisterBrowserScreen;
                }
                else if(ITM_0 <= item && item <= ITM_9) {
                  if(rbr1stDigit) {
                    rbr1stDigit = false;
                    rbrRegister = item - ITM_0;
                  }
                  else {
                    rbr1stDigit = true;
                    rbrRegister = rbrRegister*10 + item - ITM_0;

                    switch(rbrMode) {
                      case RBR_GLOBAL: {
                        currentRegisterBrowserScreen = rbrRegister;
                        break;
                      }
                      case RBR_LOCAL: {
                        rbrRegister = (rbrRegister >= currentNumberOfLocalRegisters ? 0 : rbrRegister);
                        currentRegisterBrowserScreen = FIRST_LOCAL_REGISTER + rbrRegister;
                        break;
                      }
                      case RBR_NAMED: {
                        rbrMode = RBR_GLOBAL;
                        currentRegisterBrowserScreen = rbrRegister;
                        break;
                      }
                    }
                  }
                }
                else if(ITM_X <= item && item <= ITM_Z) {
                  rbrMode = RBR_GLOBAL;
                  rbr1stDigit = true;
                  currentRegisterBrowserScreen = item - ITM_X + REGISTER_X;
                }
                else if(item == ITM_T) {
                  rbrMode = RBR_GLOBAL;
                  rbr1stDigit = true;
                  currentRegisterBrowserScreen = REGISTER_T;
                }
                else if(ITM_A <= item && item <= ITM_D) {
                  rbrMode = RBR_GLOBAL;
                  rbr1stDigit = true;
                  currentRegisterBrowserScreen = item - ITM_A + REGISTER_A;
                }
                else if(item == ITM_L) {
                  rbrMode = RBR_GLOBAL;
                  rbr1stDigit = true;
                  currentRegisterBrowserScreen = REGISTER_L;
                }
                else if(ITM_I <= item && item <= ITM_K) {
                  rbrMode = RBR_GLOBAL;
                  rbr1stDigit = true;
                  currentRegisterBrowserScreen = item - ITM_I + REGISTER_I;
                }
                else if(item == ITM_M) {
                  rbrMode = RBR_GLOBAL;
                  rbr1stDigit = true;
                  currentRegisterBrowserScreen = REGISTER_M;
                }
                else if(item == ITM_N) {
                  rbrMode = RBR_GLOBAL;
                  rbr1stDigit = true;
                  currentRegisterBrowserScreen = REGISTER_N;
                }
                else if(ITM_P <= item && item <= ITM_S) {
                  rbrMode = RBR_GLOBAL;
                  rbr1stDigit = true;
                  currentRegisterBrowserScreen = item - ITM_P + REGISTER_P;
                }
                else if(ITM_E <= item && item <= ITM_H) {
                  rbrMode = RBR_GLOBAL;
                  rbr1stDigit = true;
                  currentRegisterBrowserScreen = item - ITM_E + REGISTER_E;
                }
                else if(item == ITM_O) {
                  rbrMode = RBR_GLOBAL;
                  rbr1stDigit = true;
                  currentRegisterBrowserScreen = REGISTER_O;
                }
                else if(ITM_U <= item && item <= ITM_W) {
                  rbrMode = RBR_GLOBAL;
                  rbr1stDigit = true;
                  currentRegisterBrowserScreen = item - ITM_U + REGISTER_U;
                }

                keyActionProcessed = true;
                break;
              }

              case CM_ASN_BROWSER: {
                lastItem = 0;
                lastUserMode = false;
                if(item == ITM_PERIOD) {
                  fnAsnDisplayUSER = false;
                  keyActionProcessed = true;
                  lastItem = item;
                  refreshScreen(121);
                }
                else if(item == ITM_USERMODE) {
                  runFunction(item);
                  keyActionProcessed = true;
                }
                break;
              }

              case CM_FLAG_BROWSER:
              case CM_FONT_BROWSER:
              case CM_ERROR_MESSAGE:
              case CM_BUG_ON_SCREEN: {
                keyActionProcessed = true;
                break;
              }

              case CM_GRAPH:
              case CM_PLOT_STAT:
              case CM_LISTXY: {
                break;
              }

              case CM_CONFIRMATION: {
                temporaryInformation = TI_ARE_YOU_SURE;      // Keep confirmation message on screen
                keyActionProcessed = true;
                break;
              }

              case CM_PEM: {
                if(item == ITM_PR) {
                  leavePem();
                  calcModeNormal();
                  //exit menus immediately when coming out of PEM
                  extractPFNMenus();
                  keyActionProcessed = true;
                  screenUpdatingMode = SCRUPD_AUTO;
                }
                else if(item == ITM_OFF) {
                  fnOff(NOPARAM);
                  keyActionProcessed = true;
                }
                else if(item == ITM_SST) {
                  fnSst(NOPARAM);
                  keyActionProcessed = true;
                  refreshScreen(122);
                }
                else if(item == ITM_BST) {
                  fnBst(NOPARAM);
                  keyActionProcessed = true;
                  refreshScreen(123);
                }
                else if(aimBuffer[0] != 0 && !getSystemFlag(FLAG_ALPHA) && (item == ITM_HASH_JM || item == ITM_toINT || (nimNumberPart == NP_INT_BASE && item == ITM_RCL))) {
                  if(item == ITM_HASH_JM ) {
                    item = ITM_toINT;
                  }
                  pemAddNumber(item, true);
                  keyActionProcessed = true;
                  if(item == ITM_RCL) {
                    currentStep = findPreviousStep(currentStep);
                    --currentLocalStepNumber;
                    if(!programListEnd) {
                      scrollPemBackwards();
                    }
                  }
                }
                else if(item == ITM_RS) {
                  addStepInProgram(ITM_STOP);
                  keyActionProcessed = true;
                }
                else if(item == ITM_dotD && aimBuffer[0] == 0) {
                  addStepInProgram(ITM_toREAL);
                  keyActionProcessed = true;
                }
                break;
              }

              case CM_ASSIGN: {
                if(item > 0 && itemToBeAssigned == 0) {
                  if(tam.alpha) {
                    processAimInput(item); // sets keyActionProcessed
                    if(stringGlyphLength(aimBuffer) > 6) {
                      assignLeaveAlpha();
                      assignGetName1();
                    }
                  }
                  else {
                    if(item == ITM_XEQ && tmpString[0] != 0 && (getSystemFlag(FLAG_USER) || ((currentKeyCode == Norm_Key_00_key) && (keyStateCode == 0) && Norm_Key_00.used))) {
                      char label[15];
                      xcopy(label,tmpString, stringByteLength(tmpString) + 1);
                      calcRegister_t regist = findNamedLabel(label);
                      if(regist != INVALID_VARIABLE) {
                        item = regist - FIRST_LABEL + ASSIGN_LABELS;
                      }
                      else {
                        displayCalcErrorMessage(ERROR_LABEL_NOT_FOUND, ERR_REGISTER_LINE, REGISTER_X);
                        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
                          sprintf(errorMessage, "string '%s' is not a named label", label);
                          moreInfoOnError("In function processKeyAction:", errorMessage, NULL, NULL);
                        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
                      }
                    }
                    else if(item == ITM_RCL && tmpString[0] != 0 && (getSystemFlag(FLAG_USER) || ((currentKeyCode == Norm_Key_00_key) && (keyStateCode == 0) && Norm_Key_00.used))) {
                      char var[15];
                      xcopy(var,tmpString, stringByteLength(tmpString) + 1);
                      calcRegister_t regist = findNamedVariable(var);
                      if(regist != INVALID_VARIABLE) {
                        item = regist - FIRST_NAMED_VARIABLE + ASSIGN_NAMED_VARIABLES;
                      }
                      else {
                        displayCalcErrorMessage(ERROR_LABEL_NOT_FOUND, ERR_REGISTER_LINE, REGISTER_X);
                        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
                          sprintf(errorMessage, "string '%s' is not a named variable", var);
                          moreInfoOnError("In function processKeyAction:", errorMessage, NULL, NULL);
                        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
                      }
                    }

                    itemToBeAssigned = numlockReplacements(100,item,getSystemFlag(FLAG_NUMLOCK),false,false);
                    if(ITM_A <= itemToBeAssigned && itemToBeAssigned <= ITM_Z && lowercaseselected) {
                      itemToBeAssigned += (ITM_a - ITM_A);
                    }
                    #if defined(PC_BUILD) && defined(VERBOSE_DETERMINEITEM)
                      printf("**[DL]** itemToBeAssigned = %d %s\n",itemToBeAssigned,indexOfItems[itemToBeAssigned].itemSoftmenuName);
                    #endif //VERBOSE_DETERMINEITEM

                    if(previousCalcMode == CM_AIM) softmenuStack[0].softmenuId = 1;     //JM change ALPHA to MyAlpha to be able to write ASN target
                  }
                  keyActionProcessed = true;
                }
                else if(item != 0 && itemToBeAssigned != 0) {
                  if(tam.alpha && tam.mode != TM_NEWMENU) {
                    if(item > 0) {
                      processAimInput(item); // sets keyActionProcessed
                      if(stringGlyphLength(aimBuffer) > 6) {
                        assignLeaveAlpha();
                        assignGetName2();
                      }
                      keyActionProcessed = true;
                    }
                  }
                  else {
                    switch(item) {
                      case ITM_ENTER:
                      case ITM_SHIFTf:
                      case ITM_SHIFTg:
                      case ITM_USERMODE:
                      case ITM_EXIT1:
                      case KEY_fg:        //JM
                      case ITM_BACKSPACE: {
                        break;
                      }
                      default: { //any other item
                        //printf("AAA: calcMode=%i lastItem = %i keyActionProcessed=%i showFunctionNameItem=%i releaseOverride=%i tam.mode=%i tamBuffer=%s tamBuffer[0]=%u\n", calcMode, lastItem, keyActionProcessed, showFunctionNameItem, releaseOverride, tam.mode, tamBuffer, tamBuffer[0]);
                        //enable this code to let the HOME and P.FN menus on the keyboard be active
                        #if defined(HOME_AND_PFN_KEYS)
                        if((item == -MNU_HOME || item == -MNU_PFN || item == -MNU_MyMenu) && tamBuffer[0] != 0) {
                          showSoftmenu(item);
                          shiftG = false;
                          shiftF = false;
                        } else
                        #endif //HOME_AND_PFN_KEYS
                        {
                          tamBuffer[0] = 0;
                          keyActionProcessed = true;
                        }
                      }
                    }
                  }
                }
                break;
              }

              case CM_TIMER: {

                    #if defined(PC_BUILD)
                      printf("ITEM: %d\n", item);
                    #endif // PC_BUILD

                switch(item) {
                  case ITM_TIMER_R_S:
                  case ITM_RS: {
                    fnStartStopTimerApp(NOPARAM);
                    break;
                  }
                  case ITM_0:
                  case ITM_1:
                  case ITM_2:
                  case ITM_3:
                  case ITM_4:
                  case ITM_5:
                  case ITM_6:
                  case ITM_7:
                  case ITM_8:
                  case ITM_9: {
                    fnDigitKeyTimerApp(item - ITM_0);
                    break;
                  }
                  case ITM_PERIOD: {
                    fnRegAddLapTimerApp(NOPARAM);      //dot
                    break;
                  }
                  case ITM_SIGMAPLUS: {         // Σ+ =  ADD_T_Σ
                    fnAddTimerApp(NOPARAM);
                    break;
                  }
                  case ITM_ADD: {               // Σ+ =  ADD_L_Σ
                    fnAddLapTimerApp(NOPARAM);
                    break;
                  }
                  case ITM_RCL: {
                    runFunction(ITM_TIMER_RCL);
                    break;
                  }
                }
                keyActionProcessed = true;
                break;
              }

              default: {
                sprintf(errorMessage, "In function processKeyAction: %" PRIu8 " is an unexpected value while processing calcMode!", calcMode);
                displayBugScreen(errorMessage);
              }
            }
          }
        }
      }
    }
    #if (REAL34_WIDTH_TEST == 1)
      longIntegerFree(lgInt);
    #endif // (REAL34_WIDTH_TEST == 1)
  }


  static void menuUp(void) {
    int16_t menuId = softmenuStack[0].softmenuId;
    int16_t sm = softmenu[menuId].menuItem;

    screenUpdatingMode &= ~SCRUPD_MANUAL_MENU;
    if(temporaryInformation == TI_NO_INFO && lastErrorCode == ERROR_NONE) {
      screenUpdatingMode |= SCRUPD_SKIP_STACK_ONE_TIME;
    }
    if((sm == -MNU_alpha_omega || sm == -MNU_ALPHAintl) && alphaCase == AC_LOWER) {
      alphaCase = AC_UPPER;
      showAlphaModeonGui(); //dr JM, see keyboardtweaks
      softmenuStack[0].softmenuId--; // Switch to the upper case menu
    }
    else if((sm == -MNU_ALPHAMISC || sm == -MNU_ALPHAMATH || sm == -MNU_ALPHA) && alphaCase == AC_LOWER && arrowCasechange) {  //JMcase
      alphaCase = AC_UPPER;
      showAlphaModeonGui(); //dr JM, see keyboardtweaks
    }
    else {
      int16_t itemShift = (catalog == CATALOG_NONE ? 18 : 6);
      if((softmenuStack[0].firstItem + itemShift) < (menuId < NUMBER_OF_DYNAMIC_SOFTMENUS ? dynamicSoftmenu[menuId].numItems : softmenu[menuId].numItems)) {
        softmenuStack[0].firstItem += itemShift;
      }
      else {
        softmenuStack[0].firstItem = 0;
      }

      setCatalogLastPos();
    }
    doRefreshSoftMenu = true;
                    #if defined(PC_BUILD) && defined(MONITOR_CLRSCR)
                      printf("#### menuUp: screenUpdatingMode=%u\n",screenUpdatingMode);
                    #endif // PC_BUILD &&MONITOR_CLRSCR
  }



  static void menuDown(void) {
    int16_t menuId = softmenuStack[0].softmenuId;
    int16_t sm = softmenu[menuId].menuItem;

    screenUpdatingMode &= ~SCRUPD_MANUAL_MENU;
    if(temporaryInformation == TI_NO_INFO && lastErrorCode == ERROR_NONE) {
      screenUpdatingMode |= SCRUPD_SKIP_STACK_ONE_TIME;
    }

    if((sm == -MNU_ALPHA_OMEGA || sm == -MNU_ALPHAINTL) && alphaCase == AC_UPPER) {
      alphaCase = AC_LOWER;
      showAlphaModeonGui(); //dr JM, see keyboardtweaks
      softmenuStack[0].softmenuId++; // Switch to the lower case menu
    }
    else if((sm == -MNU_ALPHAMISC || sm == -MNU_ALPHAMATH || sm == -MNU_ALPHA) && alphaCase == AC_UPPER && arrowCasechange) {  //JMcase
      alphaCase = AC_LOWER;
      showAlphaModeonGui(); //dr JM, see keyboardtweaks
    }
    else {
      int16_t itemShift = (catalog == CATALOG_NONE ? 18 : 6);

      if((softmenuStack[0].firstItem - itemShift) >= 0) {
        softmenuStack[0].firstItem -= itemShift;
      }
      else if((softmenuStack[0].firstItem - itemShift) >= -5) {
        softmenuStack[0].firstItem = 0;
      }
      else {
        if(menuId < NUMBER_OF_DYNAMIC_SOFTMENUS) {
          softmenuStack[0].firstItem = ((dynamicSoftmenu[menuId].numItems - 1)/6) / (itemShift/6) * itemShift;
        }
        else {
          softmenuStack[0].firstItem = ((       softmenu[menuId].numItems - 1)/6) / (itemShift/6) * itemShift;
        }
      }
      setCatalogLastPos();
    }
    doRefreshSoftMenu = true;
                    #if defined(PC_BUILD) && defined(MONITOR_CLRSCR)
                      printf("#### menuDown: screenUpdatingMode=%u\n",screenUpdatingMode);
                    #endif // PC_BUILD &&MONITOR_CLRSCR
  }
#endif // !TESTSUITE_BUILD



void fnKeyEnter(uint16_t unusedButMandatoryParameter) {
  doRefreshSoftMenu = true;     //dr
  #if !defined(TESTSUITE_BUILD)
    switch(calcMode) {
      case CM_NORMAL: {

        if(!getSystemFlag(FLAG_ERPN) || (!nimWhenButtonPressed && programRunStop != PGM_RUNNING) || (getSystemFlag(FLAG_ERPN) && programRunStop == PGM_RUNNING )) {     //vv PHM eRPN 2021-07;   JM corrected eRPN on 2024-03-19 on master 86fd2a5
          setSystemFlag(FLAG_ASLIFT);
                    #if defined(DEBUGUNDO)
                      printf(">>> saveForUndo from fnKeyEnterA\n");
                    #endif // DEBUGUNDO
          saveForUndo();
          if(lastErrorCode == ERROR_RAM_FULL) {
          goto undo_disabled;
          }

          liftStack();
          if(lastErrorCode == ERROR_RAM_FULL) {
            goto ram_full;
          }
          copySourceRegisterToDestRegister(REGISTER_Y, REGISTER_X);
          if(lastErrorCode == ERROR_RAM_FULL) {
            goto ram_full;
          }
        }

        if(getSystemFlag(FLAG_ERPN)) {
          setSystemFlag(FLAG_ASLIFT);
        }
        else {                                               //^^ PHM eRPN 2021-07
          clearSystemFlag(FLAG_ASLIFT);
        }                                                    //PHM eRPN 2021-07
        break;
      }

      case CM_AIM: {
          if(softmenuStack[0].softmenuId <= 1 || menu(1) == -MNU_ALPHA) {
            popSoftmenu();
          }
          if(currentMenu() == -MNU_ALPHA) {  //JM get out of the ALPHA menu and go to MyM fto satisfy the old enter routines
            softmenuStack[0].softmenuId = 1;                                  //JM
          }                                                                   //JM

        calcModeNormal();
        popSoftmenu();

        if(aimBuffer[0] == 0) {
                    #if defined(DEBUGUNDO)
                      printf(">>> undo from fnKeyEnter\n");
                    #endif // DEBUGUNDO
          undo();
        }
        else {
          int16_t lenInBytes = stringByteLength(aimBuffer) + 1;

          reallocateRegister(REGISTER_X, dtString, TO_BLOCKS(lenInBytes), amNone);
          xcopy(REGISTER_STRING_DATA(REGISTER_X), aimBuffer, lenInBytes);

          if(!getSystemFlag(FLAG_ERPN)) {                                  //PHM eRPN 2021-07
                    #if defined(DEBUGUNDO)
                      printf(">>> saveForUndo from fnKeyEnterB\n");
                    #endif // DEBUGUNDO
            setSystemFlag(FLAG_ASLIFT);
            saveForUndo();
            if(lastErrorCode == ERROR_RAM_FULL) {
              goto undo_disabled;
            }
            liftStack();
            if(lastErrorCode == ERROR_RAM_FULL) {
              goto ram_full;
            }
            clearSystemFlag(FLAG_ASLIFT);

            copySourceRegisterToDestRegister(REGISTER_Y, REGISTER_X);
            if(lastErrorCode == ERROR_RAM_FULL) {
              goto ram_full;
            }
            aimBuffer[0] = 0;              //PHM JM Keeping the structure like 43S, to be able to pick up changes from their side easier
          }
          else {
            setSystemFlag(FLAG_ASLIFT);
            aimBuffer[0] = 0;              //PHM JM Keeping the structure like 43S, to be able to pick up changes from their side easier
          }
        }
        break;
      }

      case CM_MIM: {
        mimEnter(false);
        break;
      }

//       43S code not in use: PHM.
//       JM Keeping the structure like in 43S, to be able to pick up changes
//JM: 2022-09-04:
//JM: This code does not seem to be "not-used" See bug report Gitlab #80. code seems needed.
//JM: PHM is not active in the project anymore. Restored this code:
      case CM_NIM: {
        closeNim();

        if(calcMode != CM_NIM && lastErrorCode == 0) {
          setSystemFlag(FLAG_ASLIFT);
                    #if defined(DEBUGUNDO)
                      printf(">>> saveForUndo from fnKeyEnterC\n");
                    #endif // DEBUGUNDO
          saveForUndo();
          if(lastErrorCode == ERROR_RAM_FULL) {
            goto undo_disabled;
          }
          liftStack();
          if(lastErrorCode == ERROR_RAM_FULL) {
            goto ram_full;
          }
          clearSystemFlag(FLAG_ASLIFT);
          copySourceRegisterToDestRegister(REGISTER_Y, REGISTER_X);
          if(lastErrorCode == ERROR_RAM_FULL) {
            goto ram_full;
          }
        }
        break;
      }
//JM ^^ ---2022-09-04

      case CM_EIM: {
        if(aimBuffer[0] != 0) {
          setEquation(currentFormula, aimBuffer);
          parseEquation(currentFormula, EQUATION_PARSER_MVAR, aimBuffer, tmpString);;
          if(lastErrorCode != 0) {  // Stay in Edit mode for the current equation
            const char *equationString = TO_PCMEMPTR(allFormulae[currentFormula].pointerToFormulaData);
            if(equationString) {
              xcopy(aimBuffer, equationString, stringByteLength(equationString) + 1);
            }
            else {
              aimBuffer[0] = 0;
            }
            refreshRegisterLine(ERR_REGISTER_LINE);   //[DL] added to force error line refresh
            break;
          }
        }
        if(currentMenu() == -MNU_EQ_EDIT) {
          calcModeNormal();
          if(allFormulae[currentFormula].pointerToFormulaData == C47_NULL) {
            deleteEquation(currentFormula);
          }
        }
        popSoftmenu();
        break;
      }

      case CM_REGISTER_BROWSER:
      case CM_FLAG_BROWSER:
      case CM_ASN_BROWSER:
      case CM_FONT_BROWSER:
      case CM_ERROR_MESSAGE:
      case CM_BUG_ON_SCREEN:
      case CM_PLOT_STAT:
      case CM_LISTXY:                     //JM
      case CM_GRAPH: {                     //JM
        break;
      }

      case CM_TIMER: {
        fnRegAddTimerApp(NOPARAM);     //ENTER
        break;
      }

      case CM_CONFIRMATION: {
        temporaryInformation = TI_ARE_YOU_SURE;      // Keep confirmation message on screen
        break;
      }

      default: {
        sprintf(errorMessage, commonBugScreenMessages[bugMsgCalcModeWhileProcKey], "fnKeyEnter", calcMode, "ENTER");
        displayBugScreen(errorMessage);
    }
    }
    return;

undo_disabled:
    temporaryInformation = TI_UNDO_DISABLED;
    return;

ram_full:
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
                    #if defined(DEBUGUNDO)
                      printf(">>> Undo from fnKeyEnterD\n");
                    #endif // DEBUGUNDO
    fnUndo(NOPARAM);
    return;
  #endif // !TESTSUITE_BUILD
}



#if !defined(TESTSUITE_BUILD)
  static void stayInAIM(void) {
    if(calcMode == CM_AIM && (currentMenu() != -MNU_ALPHA && currentMenu() != -MNU_MyAlpha) ) {   //JM
      changeToALPHA();
      setSystemFlag(FLAG_ALPHA);                     //JM
    }                                                //JM ^^

    if(calcMode != CM_AIM && (currentMenu() == -MNU_ALPHA ||
                              menu(1) == -MNU_ALPHA)) { //JMvv : If ALPHA, switch back to AIM
      setSystemFlag(FLAG_ALPHA);                                          //JM
      calcMode = CM_AIM;
    }                                                                     //JM ^^

    refreshModeGui(); //JM refreshModeGui
  }
#endif // !TESTSUITE_BUILD



void fnKeyExit(uint16_t unusedButMandatoryParameter) {
  #if !defined(TESTSUITE_BUILD)
    if(tam.mode == TM_KEY && !tam.keyInputFinished) {
      if(tam.digitsSoFar == 0) {
        tamProcessInput(ITM_2);
        tamProcessInput(ITM_1);
        shiftF = shiftG = false;
        refreshScreen(124);
      }
      return;
    }
    if(lastErrorCode == 0 && currentMenu() == -MNU_MVAR) {
      currentSolverStatus &= ~SOLVER_STATUS_INTERACTIVE;
    }

    doRefreshSoftMenu = true;     //dr
                    #if defined(PC_BUILD)
                      jm_show_calc_state("fnKeyExit");
                    #endif

    if(getSystemFlag(FLAG_INTING) || getSystemFlag(FLAG_SOLVING)) {
      displayCalcErrorMessage(ERROR_SOLVER_ABORT, ERR_REGISTER_LINE, REGISTER_X);
      return; // Done elsewhere
    }

    switch(calcMode) {                           //if in Catalog
        case CM_REGISTER_BROWSER:
        case CM_FLAG_BROWSER:
        case CM_ASN_BROWSER:
        case CM_FONT_BROWSER:
        case CM_CONFIRMATION:
        case CM_ERROR_MESSAGE:
        case CM_BUG_ON_SCREEN: {
            // Browser or message should be closed first
            break;
          }

        default: {
            if(catalog && (catalog != CATALOG_MVAR || !tam.mode)) {
                if(lastErrorCode != 0) {
                    lastErrorCode = 0;
                }
                else {
                    if(currentMenu() == -MNU_SYSFL) {                                                       //JM auto recover out of SYSFL
                      numberOfTamMenusToPop = 2;                                                   //JM
                      leaveTamModeIfEnabled();                                                     //JM
                      return;                                                                      //JM
                    }                                                                              //JM
                    leaveAsmMode();
                    popSoftmenu();
                    if(tam.mode ) {
                        numberOfTamMenusToPop--;
                    }
                }
                return;
            }
      }
    }

    if(tam.mode) {                               //if in TAM mode
      // [DL] : TM_LABEL specific Alpha Exit logic below replaced by TM generic exit logic, so it should not be needed anymore
        /*if((tam.mode == TM_LABEL || tam.mode == TM_LBLONLY) && (calcMode == CM_NORMAL || calcMode == CM_PEM) && getSystemFlag(FLAG_ALPHA) && menu(1) == -MNU_TAMALPHA && isAlphaSubmenu(0)) {     //MODJM
            popSoftmenu();
            keyActionProcessed = true;
            return;
          }
          if((tam.mode == TM_LABEL || tam.mode == TM_LBLONLY) && (calcMode == CM_NORMAL || calcMode == CM_PEM) && getSystemFlag(FLAG_ALPHA)) {     //MODJM
            if(menu(0) == -MNU_TAMALPHA) {
              popSoftmenu();
              if(isAlphaSubmenu(0)) {
                popSoftmenu();
              }
            }
          tamProcessInput(ITM_ENTER);
          keyActionProcessed = true;
          aimBuffer[0] = 0;
          return;
          }*/

      if((numberOfTamMenusToPop > 1) && (currentMenu() != -MNU_TAMALPHA)) {
        popSoftmenu();
        numberOfTamMenusToPop--;
      }
      else {
        if(calcMode == CM_PEM) {
          aimBuffer[0] = 0;
        }
        leaveTamModeIfEnabled();
        if(calcMode == CM_PEM) {
          scrollPemBackwards();
        }
      }
      return;
    }

    switch(calcMode) {
      case CM_REGISTER_BROWSER:
      case CM_FLAG_BROWSER:
      case CM_ASN_BROWSER:
      case CM_FONT_BROWSER:
      case CM_CONFIRMATION:
      case CM_ERROR_MESSAGE:
      case CM_BUG_ON_SCREEN: {
        // Browser or message should be closed first
        break;
      }

      case CM_NORMAL: {                                                     //If in Custom Menu
        if(currentMenu() == -ITM_MENU) {
          dynamicMenuItem = 20;
          fnProgrammableMenu(NOPARAM);
          return;
        }
      }

      default: {
      }
    }

    switch(calcMode) {                           //Inside another menu like MyMenu or My Alpha or PEM
      case CM_NORMAL: {
        if(temporaryInformation == TI_VIEW_REGISTER) {
          temporaryInformation = TI_NO_INFO;
          screenUpdatingMode = SCRUPD_AUTO;
          screenUpdatingMode |= SCRUPD_SKIP_STATUSBAR_ONE_TIME;
        }
        else if(temporaryInformation == TI_SHOW_REGISTER || SHOWMODE) {
          temporaryInformation = TI_NO_INFO;
          closeShowMenu();
         }
        else if(lastErrorCode != 0) {
          lastErrorCode = 0;
        }
        else {
          if(currentMenu() == -MNU_GRAPHS && menu(1) == -MNU_PLOT_FUNC) {
            calcMode = CM_GRAPH;
            fnEqSolvGraph(EQ_PLOT_LU);
            screenUpdatingMode = SCRUPD_AUTO;
          }
          else if(softmenuStack[0].softmenuId <= 1) { // MyMenu or MyAlpha is displayed
            currentInputVariable = INVALID_VARIABLE;
            if(getSystemFlag(FLAG_BASE_HOME)) {
               showSoftmenu(-MNU_HOME);
            }
            else if(getSystemFlag(FLAG_BASE_MYM)) {
              BASE_OVERRIDEONCE = true;
              showSoftmenu(-MNU_MyMenu);
            }                             //If none selected, do not display any menu, keep the screen blank
          }
          else {                  //jm: this is where 43S cleared an error
            popSoftmenu();
            if(currentMenu() == -MNU_MVAR && ((currentSolverStatus & SOLVER_STATUS_EQUATION_MODE) == SOLVER_STATUS_EQUATION_INTEGRATE) && (currentSolverStatus & SOLVER_STATUS_SINGLE_VARIABLE)) {
              popSoftmenu();
              currentSolverStatus &= ~SOLVER_STATUS_EQUATION_MODE;
              currentSolverStatus &= ~SOLVER_STATUS_INTERACTIVE;
            }
            stayInAIM(); //JM
          }
          screenUpdatingMode &= ~SCRUPD_MANUAL_MENU;

          if(getRegisterDataType(REGISTER_X) == dtShortInteger) {
            screenUpdatingMode &= ~SCRUPD_MANUAL_MENU;
          }
          else if(temporaryInformation == TI_NO_INFO) {
            screenUpdatingMode |= SCRUPD_SKIP_STACK_ONE_TIME;
          }
        }
        break;
      }

      case CM_AIM: {
        if(currentMenu() == -MNU_ALPHA) {  //JM get out of the ALPHA menu and go to MyM fto satisfy the old exit routines
          softmenuStack[0].softmenuId = 1;                                  //JM
        }                                                                   //JM

        if(softmenuStack[0].softmenuId <= 1 && menu(1) != -MNU_ALPHA) { // MyMenu or MyAlpha is displayed
          closeAim();
                    #if defined(DEBUGUNDO)
                      printf(">>> saveForUndo from fnKeyExitA\n");
                    #endif  // DEBUGUNDO
          updateMatrixHeightCache();
          saveForUndo();
          if(lastErrorCode == ERROR_RAM_FULL) {
            goto undo_disabled;
          }
        }
        else {
          popSoftmenu();
          stayInAIM();
        }
        break;
      }

      case CM_NIM: {
        addItemToNimBuffer(ITM_EXIT1);
        updateMatrixHeightCache();
        break;
      }

      case CM_MIM: {
        if(lastErrorCode != 0) {
          lastErrorCode = 0;
        }
        else if(temporaryInformation == TI_SHOW_REGISTER) {
          temporaryInformation = TI_NO_INFO;
        }
        else {
          if(currentMenu() == -MNU_M_EDIT) {
            mimEnter(true);
            if(matrixIndex == findNamedVariable(statMx)) {
              calcSigma(0);
            }
            mimFinalize();
            calcModeNormal();
            updateMatrixHeightCache();
          }
          screenUpdatingMode = SCRUPD_AUTO;
          screenUpdatingMode |= SCRUPD_SKIP_STATUSBAR_ONE_TIME;
          popSoftmenu(); // close softmenu dedicated for the MIM
        }
        break;
      }

      case CM_PEM: {
        if(lastErrorCode != 0) {
          lastErrorCode = 0;
          break;
        }
        if(getSystemFlag(FLAG_ALPHA) && !tam.mode) {
          if(isAlphaSubmenu(0)) {
            popSoftmenu();           // Current menu is an Alpha sub-menu: just pop it
            break;
          }
        }
        if(getSystemFlag(FLAG_ALPHA) && aimBuffer[0] == 0 && !tam.mode) {
          pemAlpha(ITM_BACKSPACE);
          fnBst(NOPARAM); // Set the PGM pointer to the original position
          break;
        }
        if(aimBuffer[0] != 0 && !tam.mode) {
          if(getSystemFlag(FLAG_ALPHA)) {
            pemCloseAlphaInput();
          }
          else if(nimNumberPart == NP_INT_BASE) {
            break;
          }
          else {
            pemCloseNumberInput();
          }
          aimBuffer[0] = 0;
          fnBst(NOPARAM); // Set the PGM pointer to the original position
          break;
        }
        if(softmenuStack[0].softmenuId > 1 && currentMenu() != -MNU_PFN) { // not MyMenu and not MyAlpha
          popSoftmenu();
          break;
        }
        else if(currentMenu() == -MNU_PFN){
          //exit menus immediately when coming out of PEM
          extractPFNMenus();
        }


        aimBuffer[0] = 0;
        leavePem();
        calcModeNormal();
                    #if defined(DEBUGUNDO)
                      printf(">>> saveForUndo from fnKeyExitB\n");
                    #endif // DEBUGUNDO
        saveForUndo();
        if(lastErrorCode == ERROR_RAM_FULL) {
          goto undo_disabled;
        }
        break;
      }

      case CM_EIM: {
        if(lastErrorCode != 0) {
          lastErrorCode = 0;
        }
        else {
          if(currentMenu() == -MNU_EQ_EDIT) {
            if(allFormulae[currentFormula].pointerToFormulaData != C47_NULL) {
              parseEquation(currentFormula, EQUATION_PARSER_MVAR, aimBuffer, tmpString);
              if(lastErrorCode != 0) {
                deleteEquation(currentFormula);
                lastErrorCode = 0;
              }
            }
            else {
              deleteEquation(currentFormula);
            }
            calcModeNormal();
          }
          popSoftmenu();
        }
        break;
      }

      case CM_REGISTER_BROWSER:
      case CM_FLAG_BROWSER:
      case CM_FONT_BROWSER: {
        rbr1stDigit = true;
        calcMode = previousCalcMode;
        if(calcMode == CM_TIMER) {
          previousCalcMode = CM_NORMAL;
        }
        break;
      }

      case CM_ASN_BROWSER: {
        if(previousCalcMode == CM_AIM || tam.alpha) {
          if(currentMenu() == -MNU_AIMCATALOG) {
            popSoftmenu();
          }
          showSoftmenu(-MNU_ALPHA);
          screenUpdatingMode = SCRUPD_AUTO;
        } else {
          if(previousCalcMode == CM_EIM) {
            if(currentMenu() == -MNU_EIMCATALOG) {
              popSoftmenu();
            }
            showSoftmenu(-MNU_EQ_EDIT);
          }
          screenUpdatingMode = SCRUPD_AUTO;
          calcMode = CM_NORMAL; //without changing the refresh scheme: change to normal mode to get the stack back, then switch back to CM_EIM
          refreshScreen(0);
        }
        calcMode = previousCalcMode;
        break;
      }

      case CM_TIMER: {
        screenUpdatingMode = SCRUPD_AUTO;
        if(lastErrorCode != 0) {
          lastErrorCode = 0;
        }
        else {
          fnLeaveTimerApp();
        }
        break;
      }

      case CM_BUG_ON_SCREEN: {
        calcMode = previousCalcMode;
        break;
      }

      case CM_LISTXY: {
        calcMode = CM_GRAPH;
        reDraw = true;
        keyActionProcessed = true;
        fnRefreshState();                //jm
        break;
      }

      case CM_GRAPH:
      case CM_PLOT_STAT: {
        if(calcMode == CM_PLOT_STAT) {
          for(int16_t ii = 0; ii < 3; ii++) {
            if( (softmenuStack[0].softmenuId > 1) && !(
              (-currentMenu() == MNU_HIST) ||
              (-currentMenu() == MNU_PLOTTING) ||
              (-currentMenu() == MNU_MODEL) ||
              (-currentMenu() == MNU_REGR)
               )) {
              popSoftmenu();
            }
          }
        }
        else {
          if(currentMenu() == -MNU_PLOT_FUNC && menu(1) == -MNU_GRAPHS) {
            popSoftmenu();
          }
          popSoftmenu();
        }

        if(currentMenu() == -MNU_TIMERF) {
          clearScreen(5);
          fnItemTimerApp(NOPARAM);
          return;
        }


        lastPlotMode = PLOT_NOTHING;
        plotSelection = 0;

        calcModeNormal();
                    #if defined(DEBUGUNDO)
                      printf(">>> Undo from fnKeyExit\n");
                    #endif // DEBUGUNDO
        SAVED_SIGMA_lastAddRem = SIGMA_NONE;
        uint64_t sf0 = systemFlags0;
        uint64_t sf1 = systemFlags1;
        fnUndo(NOPARAM);
        systemFlags0 = sf0;
        systemFlags1 = sf1;
        fnClDrawMx(1);
        if(statMx[0]!='S') {
          printStatus(0, errorMessages[RESTORING_STATS],force);
          restoreStats();
        }
        screenUpdatingMode = SCRUPD_AUTO;
        forceSBupdate();
        break;
      }

      case CM_CONFIRMATION: {
        calcMode = previousCalcMode;
        popSoftmenu();                    // Pop MNU_YESNO
        temporaryInformation = TI_NO_INFO;
        if(programRunStop == PGM_WAITING) {
          programRunStop = PGM_STOPPED;
        }
        break;
      }

      case CM_ASSIGN: {
        if((softmenuStack[0].softmenuId <= 1 && softmenuStack[1].softmenuId <= 1) || (previousCalcMode == CM_EIM && currentMenu() == -MNU_EQ_EDIT)) { // MyMenu or MyAlpha is displayed
          calcMode = previousCalcMode;
          if(tam.alpha) {
            assignLeaveAlpha();
          }
        }
        else {
          popSoftmenu();
          if(previousCalcMode == CM_AIM) { //JM
            stayInAIM();                   //JM
          }                                //JM
        }
        break;
      }

      default: {
        sprintf(errorMessage, commonBugScreenMessages[bugMsgCalcModeWhileProcKey], "fnKeyExit", calcMode, "EXIT");
        displayBugScreen(errorMessage);
      }
    }

    last_CM = calcMode; //ignore this method of prioritising refreshes. This method is sunsetting.
    screenUpdatingMode &= ~SCRUPD_MANUAL_STATUSBAR;
//    refreshScreen(127);
    return;

undo_disabled:
    temporaryInformation = TI_UNDO_DISABLED;
//    screenUpdatingMode &= ~SCRUPD_MANUAL_MENU;
//    refreshScreen(128);
    return;
  #endif // !TESTSUITE_BUILD
}



void fnKeyCC(uint16_t complex_Type) {    //JM Using 'unusedButMandatoryParameter' complex_Type=KEY_COMPLEX
    doRefreshSoftMenu = true;     //dr
    #if !defined(TESTSUITE_BUILD)
    bool_t polarOk, rectOk;
    // The switch statement is broken up here, due to multiple conditions.                      //JM
    if((calcMode == CM_NIM) && (complex_Type == KEY_COMPLEX)) {
      addItemToNimBuffer(ITM_EXIT1);
    }    //JM Allow COMPLEX to be used from NIM

    if(calcMode == CM_NORMAL || ((calcMode == CM_NIM) && (complex_Type == KEY_COMPLEX))) {      //JM

      uint8_t sdataTypeX = getRegisterDataType(REGISTER_X);
      uint8_t sdataAtagX = getRegisterAngularMode(REGISTER_X);
      uint8_t sdataTypeY = getRegisterDataType(REGISTER_Y);
      uint8_t sdataAtagY = getRegisterAngularMode(REGISTER_Y);
      bool_t toClearPolar = false;
      #define isAngle(typ,tag) (typ == dtReal34 && tag != amNone)
      #define isValidAngle(typ,tag) (typ == dtLongInteger || typ == dtReal34)
      #define isRadius(typ,tag) (typ == dtLongInteger || (typ == dtReal34 && tag == amNone))
      if(getSystemFlag(FLAG_POLAR) && isAngle(sdataTypeY,sdataAtagY) && isRadius(sdataTypeX,sdataAtagX)) {
        fnSwapXY(0);
      }
      else if(!getSystemFlag(FLAG_POLAR) && isAngle(sdataTypeY,sdataAtagY) && isRadius(sdataTypeX,sdataAtagX)) {
        fnSwapXY(0);
        setSystemFlag(FLAG_POLAR);
        toClearPolar = true;
      }
      else if(!getSystemFlag(FLAG_POLAR) && isAngle(sdataTypeX,sdataAtagX) && isRadius(sdataTypeY,sdataAtagY)) {
        setSystemFlag(FLAG_POLAR);
        toClearPolar = true;
      }

      sdataTypeX = getRegisterDataType(REGISTER_X);
      sdataTypeY = getRegisterDataType(REGISTER_Y);
      sdataAtagX = getRegisterAngularMode(REGISTER_X);
      sdataAtagY = getRegisterAngularMode(REGISTER_Y);

      polarOk = isRadius(sdataTypeY, sdataAtagY) && isValidAngle(sdataTypeX, sdataAtagX) &&  getSystemFlag(FLAG_POLAR);
      rectOk  = isRadius(sdataTypeY, sdataAtagY) && isRadius    (sdataTypeX, sdataAtagX) && !getSystemFlag(FLAG_POLAR);

      //CC needs in POLAR mode Y=r, X=ϑ;
      //CC needs in RECT mode, Y=real, X=imag
      if(polarOk || rectOk) {  //imag not allowed to be an angle if rect entry
        fnReToCx(0);
      }
      else if(sdataTypeX == dtComplex34) {
        fnCxToRe(0);
      }
      else if(sdataTypeX == dtReal34Matrix && sdataTypeY == dtReal34Matrix) {
        fnReToCx(0);
      }
      else if(sdataTypeX == dtComplex34Matrix) {
        fnCxToRe(0);
      }
      else {
        if( (!polarOk && getSystemFlag(FLAG_POLAR)) || (!rectOk && !getSystemFlag(FLAG_POLAR))) {
          displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_POLAR_RECT, ERR_REGISTER_LINE, REGISTER_X); // Invalid input data type for this operation
        }
        else {
          displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X); // Invalid input data type for this operation
        }
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        if(!polarOk && getSystemFlag(FLAG_POLAR)) {
          sprintf(errorMessage, "You cannot use CC or COMPLEX to create a Polar complex number with %s(%s) in X and %s(%s) in Y!",       getDataTypeName(getRegisterDataType(REGISTER_X), true, false), getRegisterTagName((REGISTER_X), 0), getDataTypeName(getRegisterDataType(REGISTER_Y), true, false), getRegisterTagName((REGISTER_Y), 0));
        }
        else if(!rectOk && !getSystemFlag(FLAG_POLAR)) {
          sprintf(errorMessage, "You cannot use CC or COMPLEX to create a Rectangular complex number with %s(%s) in X and %s(%s) in Y!", getDataTypeName(getRegisterDataType(REGISTER_X), true, false), getRegisterTagName((REGISTER_X), 0), getDataTypeName(getRegisterDataType(REGISTER_Y), true, false), getRegisterTagName((REGISTER_Y), 0));
        }
        else {
          sprintf(errorMessage, "You cannot use CC or COMPLEX with %s in X and %s in Y!", getDataTypeName(getRegisterDataType(REGISTER_X), true, false), getDataTypeName(getRegisterDataType(REGISTER_Y), true, false));
          moreInfoOnError("In function fnKeyCC:", errorMessage, NULL, NULL);
        }
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
      if(toClearPolar) clearSystemFlag(FLAG_POLAR);
      return;
    }

    if(complex_Type == ITM_op_j) {
      temporaryFlagRect = true;
      temporaryFlagPolar = false;
    }
    else if(complex_Type == ITM_op_j_pol) {
      temporaryFlagRect = false;
      temporaryFlagPolar = true;
    }
    else {
      temporaryFlagRect = false;
      temporaryFlagPolar = false;
    }

    switch(calcMode) {                     //JM
      case CM_NIM: {
        addItemToNimBuffer(ITM_CC);
        break;
      }

      case CM_MIM: {
        mimAddNumber(ITM_CC);
        break;
      }

      case CM_PEM: {
        if(aimBuffer[0] != 0 && !getSystemFlag(FLAG_ALPHA)) {
          pemAddNumber(ITM_CC, true);
        }
        break;
      }

      case CM_EIM:
      case CM_REGISTER_BROWSER:
      case CM_FLAG_BROWSER:
      case CM_ASN_BROWSER:
      case CM_FONT_BROWSER:
      case CM_PLOT_STAT:
      case CM_TIMER:
      case CM_LISTXY:
      case CM_GRAPH: {
        break;
      }

      default: {
        sprintf(errorMessage, commonBugScreenMessages[bugMsgCalcModeWhileProcKey], "fnKeyCC", calcMode, "CC");
        displayBugScreen(errorMessage);
      }
    }
  #endif // !TESTSUITE_BUILD
}



void fnKeyBackspace(uint16_t unusedButMandatoryParameter) {
  #if !defined(TESTSUITE_BUILD)
    uint16_t lg;
  #if !defined(SAVE_SPACE_DM42_10)
    uint8_t *nextStep;
  #endif //SAVE_SPACE_DM42_10

    if(tam.mode) {
      tamProcessInput(ITM_BACKSPACE);
      return;
    }

    switch(calcMode) {
      case CM_NORMAL: {
        if(temporaryInformation == TI_VIEW_REGISTER) {
          temporaryInformation = TI_NO_INFO;
          keyActionProcessed = true;
          screenUpdatingMode = SCRUPD_AUTO;
          screenUpdatingMode |= SCRUPD_SKIP_STATUSBAR_ONE_TIME;
          return;
        }
        else
        if(SHOWMODE) {
          temporaryInformation = TI_NO_INFO;
          keyActionProcessed = true;
          closeShowMenu();
          return;
        }
        else
        if(lastErrorCode != 0) {
          lastErrorCode = 0;
          screenUpdatingMode &= ~SCRUPD_MANUAL_STACK;
          return;
        }
        else {
          if(temporaryInformation != TI_NO_INFO) {
            temporaryInformation = TI_NO_INFO;
            keyActionProcessed = true;
            screenUpdatingMode = SCRUPD_AUTO;
            screenUpdatingMode |= SCRUPD_SKIP_STATUSBAR_ONE_TIME;
            if(lastErrorCode != 0) {
              lastErrorCode = 0;
            }
          }
          if(getSystemFlag(FLAG_CLX_DROP)) {
            showFunctionName(ITM_DROP, 1000, ""); //JM 1000ms = 1s
          } else {
            showFunctionName(ITM_CLX, 1000, ""); //JM 1000ms = 1s
          }
        }
        break;
      }

      case CM_AIM: {
        if(catalog && catalog != CATALOG_MVAR) {
          if(stringByteLength(aimBuffer) > 0) {
            lg = stringLastGlyph(aimBuffer);
            aimBuffer[lg] = 0;
            xCursor = showString(aimBuffer, &standardFont, 1, Y_POSITION_OF_AIM_LINE + 6, vmNormal, true, true);
          }
        }
        else if(stringByteLength(aimBuffer) > 0) {

#if defined(TEXT_MULTILINE_EDIT)
//JMCURSORvv SPLIT STRING AT CURSOR POSITION
          uint8_t T_cursorPos_tmp;
          T_cursorPos_tmp = aimBuffer[T_cursorPos];
          aimBuffer[T_cursorPos] = 0;                  //break it at the current cursor
          lg = stringLastGlyph(aimBuffer);             //find beginning of last glyoh, to delete
          aimBuffer[lg] = 0;                           //delete it
          aimBuffer[T_cursorPos] = T_cursorPos_tmp;    //Restore broken glyph in middle at break point
          uint16_t ix = 0;
          while(aimBuffer[ix+T_cursorPos] != 0) {      //copy second part to append to first part
            aimBuffer[ix+lg] = aimBuffer[ix+T_cursorPos];
            ix++;
          }
          aimBuffer[ix+lg]=0;                          //end new buffer
          //printf("newXCursor=%d  T_cursorPos=%d  stringLastGlyph(aimBuffer)=%d\n",newXCursor,T_cursorPos,stringLastGlyph(aimBuffer));
          if(T_cursorPos <= 1 + stringLastGlyph(aimBuffer)) {
            fnT_ARROW(ITM_T_LEFT_ARROW);                               //move cursor one left
          }
//JMCURSOR^^
#else // !TEXT_MULTILINE_EDIT
          lg = stringLastGlyph(aimBuffer);
          aimBuffer[lg] = 0;
#endif // TEXT_MULTILINE_EDIT

        }
        break;
      }

      case CM_NIM: {
        addItemToNimBuffer(ITM_BACKSPACE);
        screenUpdatingMode &= ~(SCRUPD_MANUAL_STACK | SCRUPD_SKIP_STACK_ONE_TIME);
        break;
      }

      case CM_MIM: {
        if(lastErrorCode != 0) {
          lastErrorCode = 0;
        }
        else {
          mimAddNumber(ITM_BACKSPACE);
        }
        break;
      }

      case CM_EIM: {
        if(xCursor > 0) {
          char *srcPos = aimBuffer;
          char *dstPos = aimBuffer;
          char *lstPos = aimBuffer + stringNextGlyph(aimBuffer, stringLastGlyph(aimBuffer));
          --xCursor;
          for(uint32_t i = 0; i < xCursor; ++i) {
            dstPos += (*dstPos & 0x80) ? 2 : 1;
          }
          srcPos = dstPos + ((*dstPos & 0x80) ? 2 : 1);
          for(; srcPos <= lstPos;) {
            *(dstPos++) = *(srcPos++);
          }
        }
        break;
      }

      case CM_REGISTER_BROWSER:
      case CM_FLAG_BROWSER:
      case CM_FONT_BROWSER: {
        calcMode = previousCalcMode;
        break;
      }

      case CM_ASN_BROWSER: {
        fnKeyExit(NOPARAM);                          // Rather use the Exit routine as the code is the same
        break;
      }

      case CM_BUG_ON_SCREEN:
      case CM_LISTXY:
      case CM_GRAPH:
      case CM_PLOT_STAT:
      case CM_CONFIRMATION: {
        temporaryInformation = TI_ARE_YOU_SURE;      // Keep confirmation message on screen
        if(programRunStop == PGM_WAITING) {
          programRunStop = PGM_STOPPED;
        }
        break;
      }

      case CM_PEM: {
        #if !defined(SAVE_SPACE_DM42_10)

        if(lastErrorCode != 0) {
          lastErrorCode = 0;
          return;
        }
        if(getSystemFlag(FLAG_ALPHA)) {
          pemAlpha(ITM_BACKSPACE);
          if(aimBuffer[0] == 0 && getSystemFlag(FLAG_ALPHA)) {
            // close if no characters left
            pemAlpha(ITM_BACKSPACE);
          }
          if(aimBuffer[0] == 0 && !getSystemFlag(FLAG_ALPHA)) {
            if(currentLocalStepNumber > 1) {
              --currentLocalStepNumber;
              defineCurrentStep();
              if(!programListEnd) {
                scrollPemBackwards();
            }
            }
            else {
              pemCursorIsZerothStep = true;
            }
          }
        }
        else if(aimBuffer[0] == 0) {
          if(currentLocalStepNumber > 1) {
            pemCursorIsZerothStep = false;
          }
          if(!pemCursorIsZerothStep) {
            nextStep = findNextStep(currentStep);
            if(*currentStep != 255 || *(currentStep + 1) != 255) { // Not the last END
              deleteStepsFromTo(currentStep, nextStep);
            }
            if(currentLocalStepNumber > 1) {
              --currentLocalStepNumber;
              defineCurrentStep();
            }
            else {
              pemCursorIsZerothStep = true;
            }
            scrollPemBackwards();
          }
        }
        else {
          pemAddNumber(ITM_BACKSPACE, true);
          if(aimBuffer[0] == 0 && currentLocalStepNumber > 1) {
            currentStep = findPreviousStep(currentStep);
            --currentLocalStepNumber;
            if(!programListEnd) {
              scrollPemBackwards();
            }
          }
        }
        #endif // !SAVE_SPACE_DM42_10
        break;
      }

      case CM_ASSIGN: {
        if(itemToBeAssigned == 0) {
          if(!tam.alpha) {
            calcMode = previousCalcMode;
            showFunctionName(ITM_CLX, 1000, ""); //JM 1000ms = 1s
          }
          else if(stringByteLength(aimBuffer) != 0) {
            // Delete the character before the cursor
            if(alphaCursor > 0) {
              deleteAlphaCharacter(&alphaCursor);
            }
          }
          else {
            assignLeaveAlpha();
            itemToBeAssigned = ITM_BACKSPACE;
          }
        }
        else {
          if(!tam.alpha) {
            itemToBeAssigned = 0;
          }
          else if(stringByteLength(aimBuffer) != 0) {
            // Delete the character before the cursor
            if(T_cursorPos > 0) {
              deleteAlphaCharacter(&T_cursorPos);
            }
          }
          else {
            assignLeaveAlpha();
            if(asnKey[1] != 0) {;
              assignToKey((char *)asnKey);
            }
            else {
              _assignToMenu(asnKey);
            }
            calcMode = previousCalcMode;
            shiftF = shiftG = false;
            refreshScreen(129);
          }
        }
        break;
      }

      case CM_TIMER: {
        if(lastErrorCode != 0) {
          lastErrorCode = 0;
        }
        else {
          fnBackspaceTimerApp();
        }
        break;
      }

      default: {
        sprintf(errorMessage, commonBugScreenMessages[bugMsgCalcModeWhileProcKey], "fnKeyBackspace", calcMode, "BACKSPACE");
        displayBugScreen(errorMessage);
    }
    }
  #endif // !TESTSUITE_BUILD
}




void fnKeyUp(uint16_t unusedButMandatoryParameter) {
  #if !defined(TESTSUITE_BUILD)
    int16_t menuId = softmenuStack[0].softmenuId; //JM


    if(tam.mode && tam.alpha && currentMenu() == -MNU_TAMALPHA) {
      fnAlphaCursorHome(NOPARAM);
      tamProcessInput(ITM_NOP);      // To update the tam buffer
      return;
    }
    if(tam.mode == TM_KEY && !tam.keyInputFinished) {
      if(tam.digitsSoFar == 0) {
        tamProcessInput(ITM_1);
        tamProcessInput(ITM_9);
        shiftF = shiftG = false;
        refreshScreen(131);
      }
      return;
    }
    if(softmenu[menuId].menuItem != -MNU_REG && softmenu[menuId].menuItem != -MNU_FLG && tam.mode && !catalog) {
      if(tam.alpha) {
        resetAlphaSelectionBuffer();
        if(currentSoftmenuScrolls()) {
          menuUp();
        }
        else {
          alphaCase = AC_UPPER;
        }
      }
      else {
        addItemToBuffer(ITM_Max);
      }
      return;
    }

    if((calcMode == CM_NORMAL || calcMode == CM_AIM || calcMode == CM_NIM) && currentMenu() == -ITM_MENU) {
      dynamicMenuItem = 18;
      fnProgrammableMenu(NOPARAM);
      return;
    }

    switch(calcMode) {
      case CM_NORMAL:
      case CM_AIM:
      case CM_NIM:
      case CM_EIM:
      case CM_PLOT_STAT:
      case CM_GRAPH: {
        doRefreshSoftMenu = true;     //jm
        resetAlphaSelectionBuffer();

        //JM Arrow up and down if no menu other than AHOME of MyA       //JMvv
        if(!arrowCasechange && calcMode == CM_AIM && isJMAlphaSoftmenu(menuId)) {
          screenUpdatingMode = SCRUPD_AUTO;
          fnT_ARROW(ITM_UP1);
        }

              // make this keyActionProcessed = false; to have arrows up and down placed in bufferize
              // make arrowCasechnage true
                                                                       //JM^^
        else

        if(currentSoftmenuScrolls()) {
          menuUp();
        }
        else if((calcMode == CM_NORMAL || calcMode == CM_AIM || calcMode == CM_NIM) && (numberOfFormulae < 2 || currentMenu() != -MNU_EQN)) {
          screenUpdatingMode = SCRUPD_AUTO;
          screenUpdatingMode |= SCRUPD_SKIP_STATUSBAR_ONE_TIME;
          if(calcMode == CM_NIM) {
            closeNim();
          }
          if(calcMode == CM_AIM) {
            closeAim();
          }
          fnBst(NOPARAM);
          #if defined(DMCP_BUILD)
            lcd_refresh();
          #else // !DMCP_BUILD
            refreshLcd(NULL);
          #endif // DMCP_BUILD
        }
        if(currentMenu() == -MNU_PLOT_ASSESS){
          strcpy(plotStatMx, "STATS");
          fnPlotStat(PLOT_NXT);
        }
        else if(currentMenu() == -MNU_EQN){
          if(currentFormula == 0) {
            currentFormula = numberOfFormulae;
          }
          --currentFormula;
          if(numberOfFormulae > 1) {
            currentSolverVariable = INVALID_VARIABLE;
            graphVariabl1 = 0;
          }
          screenUpdatingMode &= ~SCRUPD_MANUAL_MENU;
        }
        else {
          //alphaCase = AC_UPPER;
        }
        break;
      }

      case CM_REGISTER_BROWSER: {
        rbr1stDigit = true;
        if(rbrMode == RBR_GLOBAL) {
          currentRegisterBrowserScreen = modulo(currentRegisterBrowserScreen + RBR_INCDEC1, LAST_GLOBAL_REGISTER_SCREEN + RBR_INCDEC1);
        }
        else if(rbrMode == RBR_LOCAL) {
          currentRegisterBrowserScreen = modulo(currentRegisterBrowserScreen - FIRST_LOCAL_REGISTER + 1, currentNumberOfLocalRegisters) + FIRST_LOCAL_REGISTER;
        }
        else if(rbrMode == RBR_NAMED) {
          currentRegisterBrowserScreen = modulo(currentRegisterBrowserScreen - FIRST_NAMED_VARIABLE + 1, numberOfNamedVariables + LAST_RESERVED_VARIABLE - FIRST_RESERVED_VARIABLE - (NUMBER_OF_LETTERED_VARIABLES-1)) + FIRST_NAMED_VARIABLE;
        }
        else {
          sprintf(errorMessage, commonBugScreenMessages[bugMsgRbrMode], "fnKeyUp", "UP", rbrMode);
          displayBugScreen(errorMessage);
        }
        break;
      }

      case CM_FLAG_BROWSER: {
        currentFlgScr++;                          //[DL] reverse order
        break;
      }

      case CM_ASN_BROWSER: {
        currentAsnScr++;                          //JM removed the 3-x part
        if(currentAsnScr == 0 ||currentAsnScr >= 7) {
          currentAsnScr = (previousCalcMode == CM_AIM || previousCalcMode == CM_EIM || tam.alpha) ? 4 : 1;
        }
        break;
      }

      case CM_FONT_BROWSER: {
        if(currentFntScr >= 2) {
          currentFntScr--;
        }
        break;
      }

      case CM_PEM: {
        resetAlphaSelectionBuffer();
        if(getSystemFlag(FLAG_ALPHA) && alphaCase == AC_LOWER) {
          alphaCase = AC_UPPER;
          if(currentMenu() == -MNU_alpha_omega || currentMenu() == -MNU_ALPHAintl) {
            softmenuStack[0].softmenuId--; // Switch to the upper case menu
          }
        }
        else if(currentSoftmenuScrolls()) {
          menuUp();
        }
        else {
          if(getSystemFlag(FLAG_ALPHA) && aimBuffer[0] == 0 && !tam.mode) {
            pemAlpha(ITM_BACKSPACE);
          }
          fnBst(NOPARAM);
        }
        break;
      }

      case CM_LISTXY: {
        ListXYposition += 10;
        keyActionProcessed = true;
        break;
      }

      case CM_MIM: {
        #if defined(NOMATRIXCURSORS)
          if(currentSoftmenuScrolls() && (catalog || (currentMenu() != -MNU_TAMSTO && currentMenu() != -MNU_TAMRCL))) {   //JM remove to allow normal arrows to work as cursors
            menuUp();
          }
        #else  // !NOMATRIXCURSORS
          if(currentSoftmenuScrolls() && catalog) {
            menuUp();
          } else {
            keyActionProcessed = false;
          }
        #endif // NOMATRIXCURSORS
        break;
      }

      case CM_ASSIGN: {
        if(currentSoftmenuScrolls()) {
          menuUp();
        }
        else if(tam.alpha && alphaCase == AC_LOWER) {
          alphaCase = AC_UPPER;
        }
        else if(tam.alpha && itemToBeAssigned == 0 && aimBuffer[0] == 0) {
          assignLeaveAlpha();
          itemToBeAssigned = ITM_UP1;
        }
        else if(tam.alpha && aimBuffer[0] == 0) {
          assignLeaveAlpha();
          if(asnKey[1] != 0) {
            assignToKey((char *)asnKey);
          }
          else {
            _assignToMenu(asnKey);
          }
          calcMode = previousCalcMode;
          shiftF = shiftG = false;
          refreshScreen(132);
        }
        break;
      }

      case CM_TIMER: {
        fnUpTimerApp();
        break;
      }

      default: {
        sprintf(errorMessage, commonBugScreenMessages[bugMsgCalcModeWhileProcKey], "fnKeyUp", calcMode, "UP");
        displayBugScreen(errorMessage);
    }
    }
  #endif // !TESTSUITE_BUILD
}



void fnKeyDown(uint16_t unusedButMandatoryParameter) {
  #if !defined(TESTSUITE_BUILD)
    int16_t menuId = softmenuStack[0].softmenuId; //JM

//--     if(SHOWMODE && currentMenu() != -MNU_EQN && !tam.mode) { //JMSHOW vv
//--       if(temporaryInformation == TI_SHOW_REGISTER_TINY) {
//--         fnShow_SCROLL(12);
//--       } else {
//--         fnShow_SCROLL(2);
//-- //      refreshScreen(133);
//--       }
//--       return;
//--     }                             //JMSHOW ^^


    if(tam.mode && tam.alpha && currentMenu() == -MNU_TAMALPHA) {
      fnAlphaCursorEnd(NOPARAM);
      tamProcessInput(ITM_NOP);      // To update the tam buffer
      return;
    }
    if(tam.mode == TM_KEY && !tam.keyInputFinished) {
      if(tam.digitsSoFar == 0) {
        tamProcessInput(ITM_2);
        tamProcessInput(ITM_0);
        shiftF = shiftG = false;
        refreshScreen(134);
      }
      return;
    }
    if(softmenu[menuId].menuItem != -MNU_REG && softmenu[menuId].menuItem != -MNU_FLG && tam.mode && !catalog) {
      if(tam.alpha) {
        resetAlphaSelectionBuffer();
        if(currentSoftmenuScrolls()) {
          menuDown();
        }
        else {
          alphaCase = AC_LOWER;
        }
      }
      else {
        addItemToBuffer(ITM_Min);
      }
      return;
    }

    if((calcMode == CM_NORMAL || calcMode == CM_AIM || calcMode == CM_NIM) && currentMenu() == -ITM_MENU) {
      dynamicMenuItem = 19;
      fnProgrammableMenu(NOPARAM);
      return;
    }

    switch(calcMode) {
      case CM_NORMAL:
      case CM_AIM:
      case CM_NIM:
      case CM_EIM:
      case CM_PLOT_STAT:
      case CM_GRAPH: {
        doRefreshSoftMenu = true;     //jm
        resetAlphaSelectionBuffer();

        //JM Arrow up and down if AHOME of MyA       //JMvv
        if(!arrowCasechange && calcMode == CM_AIM && isJMAlphaSoftmenu(menuId)) {
          screenUpdatingMode = SCRUPD_AUTO;
          fnT_ARROW(ITM_DOWN1);
        }

              // make this keyActionProcessed = false; to have arrows up and down placed in bufferize
              // make arrowCasechnage true
                                                                       //JM^^
        else

        if(currentSoftmenuScrolls()) {
          menuDown();
        }
        else if((calcMode == CM_NORMAL || calcMode == CM_AIM || calcMode == CM_NIM) && (numberOfFormulae < 2 || currentMenu() != -MNU_EQN)) {
          screenUpdatingMode = SCRUPD_AUTO;
          screenUpdatingMode |= SCRUPD_SKIP_STATUSBAR_ONE_TIME;
          if(calcMode == CM_NIM) {
            closeNim();
          }
          if(calcMode == CM_AIM) {
            closeAim();
          }
          fnSst(NOPARAM);
          #if defined(DMCP_BUILD)
            lcd_refresh();
          #else // !DMCP_BUILD
            refreshLcd(NULL);
          #endif // DMCP_BUILD
        }
        if(currentMenu() == -MNU_PLOT_ASSESS){
          strcpy(plotStatMx, "STATS");
          fnPlotStat(PLOT_REV); //REVERSE
        }
        else if(currentMenu() == -MNU_EQN){
          ++currentFormula;
          if(currentFormula == numberOfFormulae) {
            currentFormula = 0;
          }
          if(numberOfFormulae > 1) {
            currentSolverVariable = INVALID_VARIABLE;
            graphVariabl1 = 0;
          }
          screenUpdatingMode &= ~SCRUPD_MANUAL_MENU;
        }
        else {
          //alphaCase = AC_LOWER;
        }
        break;
      }

      case CM_REGISTER_BROWSER: {
        rbr1stDigit = true;
        if(rbrMode == RBR_GLOBAL) {
          currentRegisterBrowserScreen = modulo(currentRegisterBrowserScreen - RBR_INCDEC1, LAST_GLOBAL_REGISTER_SCREEN + RBR_INCDEC1);
        }
        else if(rbrMode == RBR_LOCAL) {
          currentRegisterBrowserScreen = modulo(currentRegisterBrowserScreen - FIRST_LOCAL_REGISTER - 1, currentNumberOfLocalRegisters) + FIRST_LOCAL_REGISTER;
        }
        else if(rbrMode == RBR_NAMED) {
          currentRegisterBrowserScreen = modulo(currentRegisterBrowserScreen - FIRST_NAMED_VARIABLE - 1, numberOfNamedVariables + LAST_RESERVED_VARIABLE - FIRST_RESERVED_VARIABLE - (NUMBER_OF_LETTERED_VARIABLES-1)) + FIRST_NAMED_VARIABLE;
        }
        else {
          sprintf(errorMessage, commonBugScreenMessages[bugMsgRbrMode], "fnKeyDown", "DOWN", rbrMode);
          displayBugScreen(errorMessage);
        }
        break;
      }

      case CM_FLAG_BROWSER: {
        currentFlgScr--;                          //[DL] reverse order
        break;
      }

      case CM_ASN_BROWSER: {
        currentAsnScr--;
        if(currentAsnScr == 0 || currentAsnScr >= 7 || ((previousCalcMode == CM_AIM || previousCalcMode == CM_EIM || tam.alpha) && currentAsnScr < 4)) {
          currentAsnScr = 6;
        }
        break;
      }

      case CM_FONT_BROWSER: {
        if(currentFntScr < numScreensNumericFont + numScreensStandardFont + numScreensTinyFont) {
          currentFntScr++;
        }
        break;
      }

      case CM_PEM: {
        resetAlphaSelectionBuffer();
        if(getSystemFlag(FLAG_ALPHA) && alphaCase == AC_UPPER) {
          alphaCase = AC_LOWER;
          if(currentMenu() == -MNU_ALPHA_OMEGA || currentMenu() == -MNU_ALPHAINTL) {
            softmenuStack[0].softmenuId++; // Switch to the lower case menu
          }
        }
        else if(currentSoftmenuScrolls()) {
          menuDown();
        }
        else {
          if(getSystemFlag(FLAG_ALPHA) && aimBuffer[0] == 0 && !tam.mode) {
            pemAlpha(ITM_BACKSPACE);
            fnBst(NOPARAM); // Set the PGM pointer to the original position
          }
          fnSst(NOPARAM);
        }
        break;
      }

      case CM_LISTXY: {
        ListXYposition -= 10;
        keyActionProcessed = true;
        break;
      }

      case CM_MIM: {
        #if defined(NOMATRIXCURSORS)
          if(currentSoftmenuScrolls() && (catalog || (currentMenu() != -MNU_TAMSTO && currentMenu() != -MNU_TAMRCL))) {   //JM remove to allow normal arrows to work as cursors
            menuDown();
          }
        #else  // !NOMATRIXCURSORS
          if(currentSoftmenuScrolls() && catalog) {
            menuDown();
          } else {
            keyActionProcessed = false;
          }
        #endif // NOMATRIXCURSORS
        break;
      }

      case CM_ASSIGN: {
        if(currentSoftmenuScrolls()) {
          menuDown();
        }
        else if(tam.alpha && (itemToBeAssigned == 0 || tam.mode == TM_NEWMENU) && alphaCase == AC_UPPER) {
          alphaCase = AC_LOWER;
        }
        else if(tam.alpha && itemToBeAssigned == 0 && aimBuffer[0] == 0) {
          assignLeaveAlpha();
          itemToBeAssigned = ITM_DOWN1;
        }
        else if(tam.alpha && aimBuffer[0] == 0) {
          assignLeaveAlpha();
          if(asnKey[1] != 0) {
            assignToKey((char *)asnKey);
          }
          else {
            _assignToMenu(asnKey);
          }
          calcMode = previousCalcMode;
          shiftF = shiftG = false;
          refreshScreen(135);
        }
        break;
      }

      case CM_TIMER: {
        fnDownTimerApp();
        break;
      }

      default: {
        sprintf(errorMessage, commonBugScreenMessages[bugMsgCalcModeWhileProcKey], "fnKeyDown", calcMode, "DOWN");
        displayBugScreen(errorMessage);
    }
    }
  #endif // !TESTSUITE_BUILD
}



void fnKeyDotD(uint16_t unusedButMandatoryParameter) {
  #if !defined(TESTSUITE_BUILD)
    switch(calcMode) {
      case CM_NORMAL: {
        int32_t flag = getSystemFlag(FLAG_IRFRQ) ? FLAG_IRFRAC : FLAG_FRACT ;
        if(getSystemFlag(flag)) {
          clearSystemFlag(flag);
        }
        else {
          runFunction(ITM_toREAL);
        }
        break;
      }

      case CM_NIM: {
        addItemToNimBuffer(ITM_dotD);
        break;
      }

      case CM_REGISTER_BROWSER:
      case CM_FLAG_BROWSER:
      case CM_ASN_BROWSER:
      case CM_FONT_BROWSER:
      case CM_PLOT_STAT:
      case CM_GRAPH:
      case CM_MIM:
      case CM_EIM:
      case CM_TIMER:
      case CM_LISTXY: {
        break;
      }

      default: {
        sprintf(errorMessage, commonBugScreenMessages[bugMsgCalcModeWhileProcKey], "fnKeyDotD", calcMode, ".d!");
        displayBugScreen(errorMessage);
      }
    }
  #endif // !TESTSUITE_BUILD
}


void setLastKeyCode(int key) {             // key = 10      setLastKeyCode = 10-6 +30 = 34 row 3, col 4
  if(1 <= key && key <= 43) {              // key = 27      setLastKeyCode = 27-22+60 = 65 row 6, col 5
    if(key <=  6) {                        // key = 28 EXIT setLastKeyCode = 28-27+70 = 71 row 7, col 1
      lastKeyCode = key      + 20;
    }
    else if(key <= 12) {
      lastKeyCode = key -  6 + 30;
    }
    else if(key <= 17) {
      lastKeyCode = key - 12 + 40;
    }
    else if(key <= 22) {
      lastKeyCode = key - 17 + 50;
    }
    else if(key <= 27) {
      lastKeyCode = key - 22 + 60;
    }
    else if(key <= 32) {
      lastKeyCode = key - 27 + 70;
    }
    else if(key <= 37) {
      lastKeyCode = key - 32 + 80;
    }
    else if(key <= 43) {
      lastKeyCode = key - 37 + 10; // function keys
    }
  }
}
