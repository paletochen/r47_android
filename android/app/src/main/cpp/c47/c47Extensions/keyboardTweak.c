// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file keyboardTweak.c
 ***********************************************/

#include "c47.h"

void fnSHIFTf(uint16_t unusedButMandatoryParameter) {
  shiftF = true;
  shiftG = false;
}


void fnSHIFTg(uint16_t unusedButMandatoryParameter) {
  shiftF = false;
  shiftG = true;
}


void fnSHIFTfg(uint16_t unusedButMandatoryParameter) {
  #if !defined(TESTSUITE_BUILD)
    btnClicked(NULL, "27");
  #endif // !TESTSUITE_BUILD
}


//Length in ms, frequency in Hz
void _keyClick(uint8_t lengthMs, uint32_t f) {  //Debugging on scope, a millisecond input pulse length after every key edge. !!!!! Destroys the prior volume setting
  #if defined(DMCP_BUILD)
    #if (defined(DM42_KEYCLICK) || defined(CLICK_REFRESHSCR) || defined(DM42_POWERMARKS) || defined(DM42_POWERMARK_KEYPRESS))
      while(get_beep_volume() < 11) {
        beep_volume_up();
      }
      start_buzzer_freq(f*1000); //Click 1kHz for 1 ms
      sys_delay((uint32_t)lengthMs);
      stop_buzzer();
    #endif // DM42_KEYCLICK
  #endif // DMCP_BUILD
}

void keyClick(uint8_t lengthMs) {  //Debugging on scope, a millisecond input pulse length after every key edge. !!!!! Destroys the prior volume setting
  #if (defined(DM42_KEYCLICK) || defined(CLICK_REFRESHSCR) || defined(DM42_POWERMARKS) || defined(DM42_POWERMARK_KEYPRESS))
   _keyClick(lengthMs, 1000);
  #endif
}


void powerMarkerMsF(uint8_t lengthMs, uint32_t f) {
  #if (defined(DM42_KEYCLICK) || defined(CLICK_REFRESHSCR) || defined(DM42_POWERMARKS) || defined(DM42_POWERMARK_KEYPRESS))
    _keyClick(lengthMs, f);
  #endif
}


void showAlphaModeonGui(void) {
  #if defined(PC_BUILD)
    char tmp[200];
    sprintf(tmp, "^^^^showAlphaModeonGui\n");
    jm_show_comment(tmp);
  #endif // PC_BUILD

  if(calcMode == CM_AIM || calcMode == CM_EIM || tam.mode) {                      //vv dr JM
    #if !defined(TESTSUITE_BUILD)
      showHideAlphaMode();
    #endif // !TESTSUITE_BUILD
    calcModeAimGui();
  }                                                         //^^
  screenUpdatingMode &= ~SCRUPD_MANUAL_MENU;
  screenUpdatingMode &= ~SCRUPD_SKIP_MENU_ONE_TIME;
}


void showShiftState(void) {
  #if !defined(TESTSUITE_BUILD)
    #if defined(PC_BUILD_TELLTALE)
      printf("    >>> showShiftState: calcMode=%d\n", calcMode);
    #endif // PC_BUILD_TELLTALE

    if(!SHOWMODE && temporaryInformation != TI_SHOW_REGISTER) {
      if(shiftF) {                        //SEE screen.c:refreshScreen
        showShiftStateF();
        show_f_jm();
        showHideAlphaMode();
      }
      else if(shiftG) {                   //SEE screen.c:refreshScreen
        showShiftStateG();
        show_g_jm();
        showHideAlphaMode();
      }
      else {
        clearShiftState();
        clear_fg_jm();
        showHideAlphaMode();
        cleanupAfterShift = true;
      }
    }
  #endif // !TESTSUITE_BUILD
}


/********************************************//**  //JM Retain this because of convenience
 * \brief Resets shift keys status and clears the
 * corresponding area on the screen
 *
 * \param void
 * \return void
 *
 ***********************************************/
void resetShiftState(void) {
  fnTimerStop(TO_FG_LONG);
  fnTimerStop(TO_FG_TIMR);

  fnTimerStop(TO_3S_CTFF);     //to make sure a repeated key does not restart the f shift which just reset
  fnTimerStop(TO_AUTO_REPEAT);

  if(shiftF || shiftG) {
    shiftF = false;
    shiftG = false;
    screenUpdatingMode &= ~SCRUPD_MANUAL_SHIFT_STATUS;
    showShiftState();
    screenUpdatingMode |= (SCRUPD_SKIP_STACK_ONE_TIME);                         // | SCRUPD_SKIP_MENU_ONE_TIME); //JMNEWSPEEDUP; removed the MENU skip again, as the fglines do not get deleted in PEM AIM

//    refreshScreen(100);
    #if !defined(TESTSUITE_BUILD)
      force_refresh(timed);
    #endif //TESTSUITE_BUILD

    refreshModeGui();
  }
  lastshiftF = shiftF;
  lastshiftG = shiftG;
}


void resetKeytimers(void) {
  resetShiftState();
  //fnTimerStop(TO_FG_LONG);
  //fnTimerStop(TO_FG_TIMR);

  fnTimerStop(TO_CL_LONG);

  //additionally
  fnTimerStop(TO_FN_LONG);
}


#if !defined(TESTSUITE_BUILD)
  /********************************************//**
  * \brief Displays the f or g shift state in the
  * upper left corner of the T register line
  *
  * \param void
  * \return void
  ***********************************************/

  #define keypress_fff true
  #define keypress_long_f false
  void openHOMEorMyM(bool_t situation){
    if((getSystemFlag(FLAG_HOME_TRIPLE) || getSystemFlag(FLAG_MYM_TRIPLE)) && !GRAPHMODE && (calcMode != CM_EIM) && (calcMode != CM_MIM)) {  // f and g longpress temporarily disabled in EIM and MIM)
      #if defined(PC_BUILD)
      if(situation == keypress_fff) {
        jm_show_calc_state("keyboardtweak.c: fg_processing_jm: openHOMEorMyM");
      } else if(situation == keypress_long_f) {
        jm_show_calc_state("screen.c: Shft_handler: openHOMEorMyM");
      }
      #endif // PC_BUILD

      int16_t target_HOME = (calcMode == CM_PEM ? -MNU_PFN : -MNU_HOME);
      int16_t target_MYM  = (calcMode == CM_PEM ? -MNU_PFN : -MNU_MyMenu);
//note: restored CUST changing to HOME

//note: Removed fff clearing HOME again

//note: Add Non-USER mode below for fff


      bool_t baseOverrideOnce = false;
      BASE_OVERRIDEONCE = baseOverrideOnce;

      if(getSystemFlag(FLAG_ALPHA)) {
        leaveTamModeIfEnabled();
        if(getSystemFlag(FLAG_HOME_TRIPLE)) {
          if((currentMenu() == -MNU_MyAlpha) || (currentMenu() == -MNU_AIMCATALOG) || isAlphabeticSoftmenu()) {
            popSoftmenu();
          }
          if(tam.alpha) {
            showSoftmenu(-MNU_TAMALPHA);
          }
          else {
            showSoftmenu(-MNU_ALPHA);
          }
        }
        else if(getSystemFlag(FLAG_MYM_TRIPLE)) {
          showSoftmenu(-MNU_MyAlpha);
        }
      }
      else {
        leaveTamModeIfEnabled();

        int keyCode = (calcModel == USER_R47bk_fg) ? 11 : (calcModel == USER_R47fg_bk || calcModel == USER_R47fg_g) ? 10 : (calcModel == USER_C47 || calcModel == USER_DM42) ? 27 : 9999;
        if(keyCode != 9999) {
          calcKey_t *key = kbd_usr + keyCode;
          int16_t item = key->gShifted;
          if(calcMode == CM_NIM && getSystemFlag(FLAG_USER) && item != ITM_ms && item != ITM_CC && item != ITM_op_j && item != ITM_op_j_pol && item != ITM_dotD
               && item != ITM_HASH_JM && item != ITM_toINT && item != ITM_BACKSPACE && indexOfItems[item].func != addItemToBuffer) {
            delayCloseNim = false;
            closeNim();
            screenUpdatingMode &= ~SCRUPD_MANUAL_MENU;
          }
          if(getSystemFlag(FLAG_USER)) {    // USER mode
            if((calcMode != CM_AIM) && (calcMode != CM_EIM) && (item > 0)) {
              #if defined(LONGPRESS_CFG)   // only when allowed by LONGPRESS_CFG
                _executeItem(item,keyCode);
              #endif // LONGPRESS_CFG

              screenUpdatingMode = SCRUPD_AUTO;
              refreshScreen(1000);
            }
            else {;
              if(item < 0) {
                if(item == -MNU_DYNAMIC) {
                  char *funcParam = (char *)getNthString((uint8_t *)userKeyLabel, keyCode * 6 + 1);
                  setCurrentUserMenu(item, funcParam);
                }
                target_HOME = ((item == -MNU_HOME) && getSystemFlag(FLAG_MYM_TRIPLE)? -MNU_MyMenu : item);
                showSoftmenu(target_HOME);
              }
              else {
                if(getSystemFlag(FLAG_HOME_TRIPLE)) {
                  leaveTamModeIfEnabled();
                  showSoftmenu(target_HOME);
                }
                else if(getSystemFlag(FLAG_MYM_TRIPLE)) {
                  leaveTamModeIfEnabled();
                  if(situation == keypress_fff) {
                    BASE_OVERRIDEONCE = true;
                  }
                  showSoftmenu(target_MYM);
                } //If none selected, do not display any menu, keep the screen blank
              }
            }
          }
          else {                            // Normal mode
            if(getSystemFlag(FLAG_HOME_TRIPLE)) {
              leaveTamModeIfEnabled();
              showSoftmenu(target_HOME);
            }
            else if(getSystemFlag(FLAG_MYM_TRIPLE)) {
              if(getSystemFlag(FLAG_BASE_MYM) || getSystemFlag(FLAG_BASE_HOME)) {
                leaveTamModeIfEnabled();
                if(situation == keypress_fff) {
                  baseOverrideOnce = true;
                }
                BASE_OVERRIDEONCE = baseOverrideOnce;
                showSoftmenu(target_MYM);
              }
              else {
                baseOverrideOnce = false;
                BASE_OVERRIDEONCE = baseOverrideOnce;
                fnExitAllMenus(0);               // If MyMb and HOMEb are both clear, return to the blank base menu display
              }
            }  //If none selected, do not display any menu, keep the screen blank
          }
        }
      }
      BASE_OVERRIDEONCE = baseOverrideOnce;
      showSoftmenuCurrentPart();
      BASE_OVERRIDEONCE = baseOverrideOnce;            //for upcoming refresh*
      screenUpdatingMode = SCRUPD_AUTO;
      refreshScreen(23);
    }
  }

  void fg_processing_jm(void) {
    bool_t toExecute = false;
    if(getSystemFlag(FLAG_SHFT_4s) || (getSystemFlag(FLAG_HOME_TRIPLE) || getSystemFlag(FLAG_MYM_TRIPLE))) {
      if((getSystemFlag(FLAG_HOME_TRIPLE) || getSystemFlag(FLAG_MYM_TRIPLE)) && !GRAPHMODE) {
        if(fnTimerGetStatus(TO_3S_CTFF) == TMR_RUNNING) {
          JM_SHIFT_HOME_TIMER1++;
          if(JM_SHIFT_HOME_TIMER1 >= 3) {
            fnTimerStop(TO_FG_TIMR);      //dr
            fnTimerStop(TO_3S_CTFF);
            shiftF = false;               // Set it up, for flags to be cleared below.
            shiftG = true;
            leaveTamModeIfEnabled();
            toExecute = true;
          }
        }
        if(fnTimerGetStatus(TO_3S_CTFF) == TMR_STOPPED) {
          JM_SHIFT_HOME_TIMER1 = 1;
          fnTimerStart(TO_3S_CTFF, TO_3S_CTFF, JM_TO_3S_CTFF);                    //dr
        }
      }
    }

    if(!shiftF && !shiftG) {                                                      //JM shifts
      shiftF = true;                                                              //JM shifts
      shiftG = false;
    }                                                                             //JM shifts
    else if(shiftF && !shiftG) {                                                  //JM shifts
      shiftF = false;                                                             //JM shifts
      shiftG = true;                                                              //JM shifts
    }
    else if(!shiftF && shiftG) {                                                  //JM shifts
      shiftF = false;                                                             //JM shifts
      shiftG = false;                                                             //JM shifts
    }
    else if(shiftF && shiftG) {                                                   //JM shifts  should never be possible. included for completeness
      shiftF = false;                                                             //JM shifts
      shiftG = false;                                                             //JM shifts
    }                                                                             //JM shifts
    if(toExecute) {
      openHOMEorMyM(keypress_fff);
    }
  }


  int16_t Check_Norm_Key_00_Assigned(int16_t * result, int16_t tempkey) {
    if(!getSystemFlag(FLAG_USER) && (Norm_Key_00_key != -1) &&
       (calcMode == CM_NORMAL || calcMode == CM_NIM || calcMode == CM_PEM || calcMode == CM_TIMER || calcMode == CM_ASSIGN) &&
       !tam.alpha && tam.mode != TM_STORCL && tam.mode != TM_LABEL &&
       (!catalog || (catalog && (Norm_Key_00.func != ITM_SHIFTg && Norm_Key_00.func != ITM_SHIFTf && Norm_Key_00.func != KEY_fg))) &&
       (!(lastIntegerBase >= 2 && getSystemFlag(FLAG_TOPHEX))) &&
       (  ( ((!shiftF && !shiftG) || isR47FAM) && (tempkey == Norm_Key_00_key) && ((kbd_std + Norm_Key_00_key)->primary == *result) )  ||   //f & g allowed in R47, not allowed in C47 Σ+
          ((Norm_Key_00.func == KEY_fg) && (tempkey == Norm_Key_00_key) && ((kbd_std + Norm_Key_00_key)->primary == *result))  )
       ) {
      *result = Norm_Key_00.func;
      return (Norm_Key_00.func == ITM_SHIFTg || Norm_Key_00.func == ITM_SHIFTf || Norm_Key_00.func == KEY_fg) ? Norm_Key_00.func : 0;
    }
    return 0;
  }


  void Setup_MultiPresses(int16_t result) {                            //Setup and start double press for DROP timer, and check for second press
    JM_auto_doublepress_autodrop_enabled = 0;                          //JM TIMER CLRDROP. Autodrop means double click normal key.
    int16_t tmp = 0;
    if(calcMode == CM_NORMAL && result == ITM_BACKSPACE && tam.mode == 0 && !getSystemFlag(FLAG_CLX_DROP)) {             //Set up backspace double click to DROP
      tmp = ITM_DROP;
    }

    if(tmp != 0) {
      if(fnTimerGetStatus(TO_CL_DROP) == TMR_RUNNING) {
        JM_auto_doublepress_autodrop_enabled = tmp;                    //Signal to DROP, i.e. that the second press of the same button was received
      }
      fnTimerStart(TO_CL_DROP, TO_CL_DROP, JM_CLRDROP_TIMER);
    }
    fnTimerStop(TO_FG_LONG);                        //dr
    fnTimerStop(TO_FN_LONG);                        //dr
  }


#define LongpressEXIT1 (calcModel == USER_C47 ? (calcMode == CM_AIM ? -MNU_MyAlpha : ITM_BASEMENU) : ITM_SNAP)   //LongpressEXIT1 : C47: MyAlpha or MyMenu; R47: SNAP
//Note:
// ITM_BASEMENU is used by the LongpressEXIT1 macro, which does some logic to determine what EXIT must do.
// LongpressEXIT1 has an item number as output, which is fed into the longpress/command preview/runFunction systems, to open a menu. The menu is opened by fnBaseMenu(),
// and this is the only way fnBaseMenu() can be run, via the item ITM_BASEMENU which is created for that purpose.
// fnBaseMenu() opens MNU_MyMenu, and due to the reasons given, MNU_MyMenu cannot be used directly, as it needs to first set some flags, hence in a way it is like an invisible pseudo-menu, opened by an item.

  void Check_MultiPresses(int16_t *result, int8_t key_no) { //Set up longpress
    int16_t longpressDelayedkey1 = 0;                                   //To Setup the timer locally for the next timed stage
            longpressDelayedkey2 = 0;                                   //To Store the next timed stage
            longpressDelayedkey3 = 0;                                   //To Store the next timed stage


    int16_t           tmpp_ = getSystemFlag(FLAG_USER) ? kbd_usr[key_no].primary  : kbd_std[key_no].primary;
    int16_t tmpf = 0, tmpf_ = getSystemFlag(FLAG_USER) ? kbd_usr[key_no].fShifted : kbd_std[key_no].fShifted;
    int16_t tmpg = 0, tmpg_ = getSystemFlag(FLAG_USER) ? kbd_usr[key_no].gShifted : kbd_std[key_no].gShifted;
    if((calcMode == CM_NORMAL || calcMode == CM_NIM) && tam.mode==0) {  //longpress yellow math functions on the first two rows, menus allowed provided it is within keys 00-14
      if(   ((key_no >= 0 && key_no < 15) && (LongPressM == RBX_M1234 || LongPressM == RBX_M124))  //any mathkeys
         || (/*(key_no >= 0 && key_no < 15) && (LongPressM == RBX_M14) && */(tmpp_ == ITM_DRG && tmpf_ == ITM_USERMODE ) ) //DRG anywhere mathkeys
         || (tmpp_ == ITM_XEQ && tmpf_ == ITM_AIM)                                               //anywhere
        ) {
        if(!shiftF && !shiftG && !(lastIntegerBase >= 2 && getSystemFlag(FLAG_TOPHEX) && key_no >= 0 && key_no <= 5)) { //accept NIM but do not react, stay on default 0 0 0
          longpressDelayedkey1 = tmpf_;
          tmpf = tmpf_;
          if(LongPressM == RBX_M1234) {
            longpressDelayedkey3 = tmpg_;
            tmpg = tmpg_;
          }
        }
      }
    }                                                                   //yellow and blue function keys ^^


    else if((calcMode == CM_AIM || calcMode == CM_EIM || (calcMode == CM_PEM && getSystemFlag(FLAG_ALPHA))) && tam.mode==0) {
      tmpp_ = getSystemFlag(FLAG_USER) ? kbd_usr[key_no].primaryAim  : kbd_std[key_no].primaryAim;
      tmpg_ = getSystemFlag(FLAG_USER) ? kbd_usr[key_no].gShiftedAim : kbd_std[key_no].gShiftedAim;
      if(   ((key_no != 32 && tmpp_ != ITM_SHIFTf && tmpp_ != ITM_SHIFTg && tmpp_ != KEY_fg && tmpp_ != ITM_BACKSPACE) && (LongPressM == RBX_M1234 || LongPressM == RBX_M124))  //any mathkeys
        ) {
        if(!shiftF && !shiftG) {
            longpressDelayedkey1 = tmpg_;
            tmpg = tmpg_;
        }
      }
    }                                                                   //yellow and blue function keys ^^
    #define TAMALPHA_f //select the yellow or blue text labels
    else if(tam.alpha) {
      tmpp_ = getSystemFlag(FLAG_USER) ? kbd_usr[key_no].primaryAim  : kbd_std[key_no].primaryAim;
      #if defined(TAMALPHA_f)
        tmpf_ = getSystemFlag(FLAG_USER) ? kbd_usr[key_no].fShiftedAim : kbd_std[key_no].fShiftedAim;
      #else //TAMALPHA_f
        tmpg_ = getSystemFlag(FLAG_USER) ? kbd_usr[key_no].gShiftedAim : kbd_std[key_no].gShiftedAim;
      #endif //TAMALPHA_f
      if(   ((key_no != 32 && tmpp_ != ITM_SHIFTf && tmpp_ != ITM_SHIFTg && tmpp_ != KEY_fg && tmpp_ != ITM_BACKSPACE) && (LongPressM == RBX_M1234 || LongPressM == RBX_M124))  //any mathkeys
        ) {
        if(!shiftF && !shiftG) {
          #if defined(TAMALPHA_f)
            longpressDelayedkey1 = tmpf_;
            tmpg = tmpf_;
          #else //TAMALPHA_f
            longpressDelayedkey1 = tmpg_;
            tmpg = tmpg_;
          #endif //TAMALPHA_f
        }
      }
    }

    char *funcParam = (char *)getNthString((uint8_t *)userKeyLabel, key_no); //keyCode * 6 + g ? 2 : f ? 1 : 0);
    //printf("\n\n >>>> ## result=%i key_no=%i *funcParam=%s  [0]=%u\n", *result, key_no, (char*)funcParam, ((char*)funcParam)[0]);

    if(calcMode == CM_NORMAL && *result == ITM_UP1) {
      longpressDelayedkey1 = ITM_NOP;
    }
    else if(calcMode == CM_NORMAL && *result == ITM_DOWN1) {
      longpressDelayedkey1 = ITM_NOP;
    }

    else if(calcMode == CM_ASSIGN && *result == ITM_EXIT1) {
      longpressDelayedkey1 = -MNU_MyMenu;
      longpressDelayedkey2 = -MNU_HOME;
      longpressDelayedkey3 = -MNU_PFN;
    }

    else if(calcMode == CM_NORMAL && *result >= ITM_A && *result <= ITM_F && lastIntegerBase >= 2 && getSystemFlag(FLAG_TOPHEX)) {
       longpressDelayedkey1 = getSystemFlag(FLAG_USER) ? kbd_usr[*result - ITM_A].primary  : kbd_std[*result - ITM_A].primary;
       longpressDelayedkey2 = getSystemFlag(FLAG_USER) ? kbd_usr[*result - ITM_A].fShifted : kbd_std[*result - ITM_A].fShifted;
       longpressDelayedkey3 = getSystemFlag(FLAG_USER) ? kbd_usr[*result - ITM_A].gShifted : kbd_std[*result - ITM_A].gShifted;
    }

    else if(  (( (calcMode == CM_NORMAL || calcMode == CM_NIM) && *result == ITM_XEQ) && tam.mode == 0 && ((char*)funcParam)[0] == 0 && (getSystemFlag(FLAG_USER) ? kbd_usr[key_no].primary == kbd_std[key_no].primary : true)) ) { //If XEQ (always primary) is not the standard position, or if XEQ has a parameter then do not inject it into the long press cycle
      if(getSystemFlag(FLAG_SH_LONGPRESS)) {
        if(tmpp_ == ITM_XEQ && tmpf == ITM_AIM) {

          if(LongPressM == RBX_M14) {
            longpressDelayedkey1 = ITM_AIM;
            longpressDelayedkey2 = 0;
            longpressDelayedkey3 = 0;
          }
          else if(LongPressM == RBX_M124) {
            longpressDelayedkey1 = ITM_AIM;
            longpressDelayedkey2 = 0;
            longpressDelayedkey3 = 0;
          }
          else if(LongPressM == RBX_M1234) {
            longpressDelayedkey1 = ITM_AIM;
            longpressDelayedkey2 = 0;
            longpressDelayedkey3 = tmpg;
          }
        }
        else {
          longpressDelayedkey1 = tmpf_;
          longpressDelayedkey2 = 0;
          longpressDelayedkey3 = tmpg_;
        }
      }
    }

    else if(calcMode == CM_NORMAL && *result == ITM_BACKSPACE && tam.mode == 0) {
      longpressDelayedkey1 = ITM_CLSTK;    //backspace longpress to CLSTK
      longpressDelayedkey2 = 0; //longpressDelayedkey1;
      longpressDelayedkey3 = ITM_EDIT;
    }

    else if((calcMode == CM_NORMAL || calcMode == CM_NIM) && *result == ITM_EXIT1) {
      longpressDelayedkey1 = LongpressEXIT1; // LongpressEXIT1 : C47: MyAlpha or MyMenu; R47: SNAP
      longpressDelayedkey2 = 0;     // EXIT longpress DOES CLRMOD
      longpressDelayedkey3 = ITM_CLRMOD;              // forcefully prevent the 3rd slot to trigger when EXIT is assigned elsewhere
    }

    else if((calcMode == CM_NORMAL || calcMode == CM_NIM) && *result == ITM_DRG) {
      longpressDelayedkey1 = 0;
      longpressDelayedkey2 = 0;
      longpressDelayedkey3 = 0;
      if(tmpp_ == ITM_DRG) {
        if(LongPressM == RBX_M14) {
          longpressDelayedkey1 = tmpf == ITM_USERMODE && getSystemFlag(FLAG_SH_LONGPRESS) ? ITM_USERMODE : 0;
          //longpressDelayedkey2 = 0;
          //longpressDelayedkey3 = 0;
        }
        else if(LongPressM == RBX_M124) {
          longpressDelayedkey1 = ITM_USERMODE;
          //longpressDelayedkey2 = 0;
          //longpressDelayedkey3 = 0 ;
        }
        else if(LongPressM == RBX_M1234) {
          longpressDelayedkey1 = ITM_USERMODE;
          //longpressDelayedkey2 = 0;
          longpressDelayedkey3 = tmpg;
        }
      }
    }

    else {
      switch(calcMode) {
        case CM_NIM : {
          if(*result == ITM_BACKSPACE && tam.mode == 0) {
            longpressDelayedkey1 = ITM_CLN;      //BACKSPACE longpress clears input buffer
          }
          break;
        }

        case CM_AIM : {
          switch(*result) {
            case ITM_BACKSPACE:
              if(tam.mode == 0) {
                  longpressDelayedkey1 = ITM_CLA;      //BACKSPACE longpress clears input buffer
                  longpressDelayedkey2 = 0;
                  longpressDelayedkey3 = ITM_EDIT;
                }
              break;
            case ITM_EXIT1:
              longpressDelayedkey1 = -MNU_MyAlpha;//  LongpressEXIT1; // LongpressEXIT1 : C47: MyAlpha or MyMenu; R47: SNAP
              longpressDelayedkey2 = 0;
              longpressDelayedkey3 = ITM_CLRMOD;     // EXIT longpress DOES CLRMOD
              break;

            case ITM_ENTER:
              if(tam.mode == 0) {
                longpressDelayedkey1 = ITM_XEDIT;
                longpressDelayedkey2 = 0;
                longpressDelayedkey3 = ITM_CR;
              }
              break;
            default:;
          }
          break;
        }

        case CM_EIM : {
          switch(*result) {
            case ITM_BACKSPACE:
              if(tam.mode == 0) {
                longpressDelayedkey1 = ITM_CLA;      //BACKSPACE longpress clears input buffer
                longpressDelayedkey3 = ITM_EDIT;
              }
              break;
            case ITM_EXIT1:
              longpressDelayedkey1 = -MNU_MyAlpha;
              longpressDelayedkey2 = 0;
              longpressDelayedkey3 = ITM_CLRMOD;   //EXIT longpress DOES CLRMOD
              break;
            case ITM_ENTER:
              if(tam.mode == 0) {
                longpressDelayedkey1 = ITM_XEDIT;
                longpressDelayedkey2 = 0;
                longpressDelayedkey3 = ITM_CR;
              }
              break;
            default:;
          }
          break;
        }

        case CM_PEM : {
          switch(*result) {
            case ITM_BACKSPACE:
                longpressDelayedkey1 = ITM_EDIT;
              break;
            case ITM_EXIT1:
              longpressDelayedkey1 = -MNU_PFN;
              longpressDelayedkey2 = LongpressEXIT1; // LongpressEXIT1 : C47: MyAlpha or MyMenu; R47: SNAP
              longpressDelayedkey3 = ITM_CLRMOD;     // EXIT longpress DOES CLRMOD
              break;
            default:;
          }
          break;
        }

        default : {
          switch(*result) {
            case ITM_EXIT1    :                     // This is for modes not handles: ASSING, NORMAL, NIM, AIM, EIM handled above
              if(calcModel == USER_C47) {
                longpressDelayedkey1 = ITM_CLRMOD;   //EXIT longpress DOES CLRMOD
                longpressDelayedkey2 = 0;
                longpressDelayedkey3 = 0;
              } else {
                longpressDelayedkey1 = ITM_SNAP;
                longpressDelayedkey2 = 0;
                longpressDelayedkey3 = ITM_CLRMOD;   //EXIT longpress DOES CLRMOD
              }
              break;
            default:;
          }
          break;
        }

      }
    }

//  ----------

    if(calcMode == CM_NIM) {
      if( (*result == ITM_ms       || longpressDelayedkey1 == ITM_ms       || longpressDelayedkey2 == ITM_ms       || longpressDelayedkey3 == ITM_ms   )   || //.ms needs NIM mode to be open if the user intends it to be open.
          (*result == ITM_CC       || longpressDelayedkey1 == ITM_CC       || longpressDelayedkey2 == ITM_CC       || longpressDelayedkey3 == ITM_CC   )   ||
          (*result == ITM_dotD     || longpressDelayedkey1 == ITM_dotD     || longpressDelayedkey2 == ITM_dotD     || longpressDelayedkey3 == ITM_dotD )   ||
          (*result == ITM_HASH_JM  || longpressDelayedkey1 == ITM_HASH_JM  || longpressDelayedkey2 == ITM_HASH_JM  || longpressDelayedkey3 == ITM_HASH_JM )||
          (*result == ITM_toINT    || longpressDelayedkey1 == ITM_toINT    || longpressDelayedkey2 == ITM_toINT    || longpressDelayedkey3 == ITM_toINT   )||
          (*result == ITM_op_j     || longpressDelayedkey1 == ITM_op_j     || longpressDelayedkey2 == ITM_op_j     || longpressDelayedkey3 == ITM_op_j )   ||
          (*result == ITM_op_j_pol || longpressDelayedkey1 == ITM_op_j_pol || longpressDelayedkey2 == ITM_op_j_pol || longpressDelayedkey3 == ITM_op_j_pol )) {
        delayCloseNim = true;
      }
    }

    if(longpressDelayedkey1 != 0) {       //if activated key pressed
      JM_auto_longpress_enabled = longpressDelayedkey1;
      fnTimerStart(TO_CL_LONG, TO_CL_LONG, JM_TO_CL_LONG);    //dr

      //JM TIMER CLRDROP. Autodrop means double click normal key.
      if(JM_auto_doublepress_autodrop_enabled != 0) {
        hideFunctionName();
//        undo(); Removed undo. I cannot figure why I had it in here not detected in many years - it only started to give issue now with the undo stack worked on! 2024-04-20 jm
        showFunctionName(JM_auto_doublepress_autodrop_enabled, FUNCTION_NOPTIME, "SF:M");            //JM CLRDROP
        *result = JM_auto_doublepress_autodrop_enabled;
        fnTimerStop(TO_CL_DROP);          //JM TIMER CLRDROP ON DOUBLE BACKSPACE
        setSystemFlag(FLAG_ASLIFT);       //JM TIMER CLRDROP ON DOUBLE BACKSPACE
      }
    }
  }


  //******************* JM LONGPRESS & JM DOUBLE CLICK START *******************************
  int16_t nameFunction(int16_t fn, bool_t shiftF, bool_t shiftG) { //JM LONGPRESS vv
    int16_t func = 0;

    char str[3];
    str[0] = '0' + fn;
    str[1] = 0;

    func = determineFunctionKeyItem_C47(str, shiftF, shiftG);
    lastKeyItemDetermined = func;

    #if defined(PC_BUILD)
      //printf(">>> nameFunction fn=%i shifts=%u %u: %s %s\n", fn, shiftF, shiftG, indexOfItems[abs(func)].itemCatalogName, indexOfItems[abs(func)].itemSoftmenuName);
    #endif // PC_BUILD

    return abs(func);
  }


  //CONCEPT - actual timing was changed:

  /* Switching profile, to jump to g[FN]:
                    set g
  ____-----_______--------->
      P    R      P

    |>              btnFnPressed:  mark time and ignore if FN_double_click is not set
          |>         btnFnReleased: if t<t0 then set FN_double_click AND prevent normal MAIN keypress
                |>  btnFnPressed:  if t<t1 AND if FN_double_click then jump start to g[FN]


  Standard procedure LONGPRESS:

    | 800 | 800 | 800 |    ms     Monitored while LONGPRESSED
  ___+-----|-----|-----|--> NOP    Press and time out to NOP
  ___+---x_|_____|_____|___        Press and release within Fx time. Fx selected upon release
  ___+-----|--x__|_____|___        Press and hold, release within f(Fx) time. f(Fx) selected upon release
  ___+-----|-----|--___|___        Press and hold and hold, release within g(Fx) time. g(Fx) selected upon release
          |     |     |

  At x, at time 0, wait 75 ms to see if a press happens, i.e. double click.
  - After 75 ms, execute function.
  - If before 75 ms, key is pressed, jump to the beginning of g(Fx) state.

      STATE #1 #2 #3 #4
  ___________---___---_____       STATES indicated

  ----------x1 x2 x3 x4           X's indicate transitions X


  Detail for x (transition press/release):

  STATE1  | STATE2  | STATE3
          |t=0      |t=75 ms
  --------E______   |             Showing release and immediate execution at "E" (previous system, no LONGPRESS no DOUBLE click)
  --------x_________E_____        Showing release at x and delayed execution "E" (if double click is to be detected)
          #########              ## shows DEAD TIME where C47 does not react to the key release that already happened
                    ^             ^ shows the point where the x command is executed if no double press recognised.

  --------x_______G-|-----        Starting with key already pressed, release at x, re-press shown at 60 ms, double click registered, g-shift activated, COMMAND displayed, timing started to NOP if not released.

  --------x_________E             Starting with key already pressed, release at x, no repress within 75 ms execute Fx.
  --------x_________|_C---        Starting with key already pressed, release at x, re-press shown at 85 ms, timing cycle C re-started on Fx.


  Double click release:
  Timing is reset to start 800 ms from G again
  --------x_______G---E__        Starting with key already pressed, release at x, re-press shown at 60 ms, double click registered, g-shift activated, COMMAND displayed, timing started, key released

  //Summary
    //Process starts in btnFnReleased, with a FN key released.
    //  do timecheck, i.e. zero the timer
    //At the next press, i.e. btnFnPressed, check for a 5 ms < re-press < 75 ms gap, which, indicates a double press. (5 ms = debounce check).
    //Set a flag here, to let the refresh cyclic check for 75 ms and, if exceeded, execute the function

  */


  //**************JM DOUBLE CLICK SUPPORT vv **********************************
  void FN_cancel() {
    FN_key_pressed = 0;
    FN_timeouts_in_progress = false;
    fnTimerStop(TO_FN_LONG);                                  //dr
  }


  //*************----------*************------- FN KEY PRESSED -------***************-----------------
  void btnFnPressed_StateMachine
    #if defined(PC_BUILD)                                                           //JM LONGPRESS FN
      (GtkWidget *unused, gpointer data)
    #endif // PC_BUILD
    #if defined(DMCP_BUILD)
      (void *unused, void *data)
    #endif // DMCP_BUILD
    {

    bool_t exexute_double_g;
    bool_t double_click_detected;

    //#if define(INLINE_TEST)
    //  if(testEnabled) {
    //    fnSwStart(1);
    //  }
    //#endif // INLINE_TEST

    FN_timed_out_to_RELEASE_EXEC = false;

    //Change states according to PRESS/RELEASE incoming sequence
    if(FN_state == ST_2_REL1) {
      FN_state = ST_3_PRESS2;
    }
    else {
      FN_state =  ST_1_PRESS1;
    }

    //FN_key_pressed = *((char *)data) - '0' + 37;  //to render 38-43, as per original keypress

    if(FN_state == ST_3_PRESS2 && fnTimerGetStatus(TO_FN_EXEC) != TMR_RUNNING) {  //JM BUGFIX (INVERTED) The first  usage did not work due to the timer which was in stopped mode, not in expired mode.
      //----------------Copied here
      underline_softkey(1<<(FN_key_pressed-38), 3);   //Purposely in row 3 which does not exist, just to activate the clear previous line

      hideFunctionName();

      //IF 2-->3 is longer than double click time, then move back to state 1
      FN_timeouts_in_progress = false;
      FN_state = ST_1_PRESS1;
    }



    if(fnTimerGetStatus(TO_FN_EXEC) == TMR_RUNNING) {         //vv dr new try
      if(FN_key_pressed_last != FN_key_pressed) {       //first press
        fnTimerExec(TO_FN_EXEC);
        FN_handle_timed_out_to_EXEC = true;
        exexute_double_g = false;
      }
      else {                                            //second press
        FN_handle_timed_out_to_EXEC = false;
        exexute_double_g = true;
        fnTimerStop(TO_FN_EXEC);
      }
      if(menu(0) == -MNU_EQ_EDIT) {
        exexute_double_g = false;
      }
      //PLACE ANY CONDITION PREVENTING DOUBLE CLICK HERE
      //  old:if(softmenuStack[0].softmenuId == 0) exexute_double_g = false; //JM prevent double click from executing nothing and showing a line, if no menu is showing
    }
    else {
      FN_handle_timed_out_to_EXEC = true;
      exexute_double_g = false;
    }                                                         //^^

    if(FN_state == ST_1_PRESS1) {
      FN_key_pressed_last = FN_key_pressed;
    }

    //printf("^^^^ softmenu=%d -MNU_ALPHA=%d currentFirstItem=%d\n", softmenu[softmenuStack[0].softmenuId].menuItem, -MNU_ALPHA, softmenuStack[0].firstItem);
    //**************JM DOUBLE CLICK DETECTION ******************************* // JM FN-DOUBLE
    double_click_detected = false;                                            //JM FN-DOUBLE - Dip detection flag
    if((getSystemFlag(FLAG_G_DOUBLETAP) && !BLOCK_DOUBLEPRESS_MENU(currentMenu(),FN_key_pressed-38,0)  )) {
      if(exexute_double_g) {
        if(FN_key_pressed !=0 && FN_key_pressed == FN_key_pressed_last) {     //Identified valid double press dip, the same key in rapid succession
          shiftF = false;                                                     //JM
          shiftG = true;                                                      //JM
          double_click_detected = true;                                       //JM --> FORCE INTO LONGPRESS
          FN_handle_timed_out_to_EXEC = false;                //dr
        }
      }
      else {
        FN_timeouts_in_progress = false;         //still in no shift mode
        fnTimerStop(TO_FN_LONG);                              //dr
      }
    }

    //STAGE 1 AND 3 GO HERE
    //**************JM LONGPRESS ****************************************************
    if((FN_state == ST_1_PRESS1 || FN_state == ST_3_PRESS2) && (!FN_timeouts_in_progress || double_click_detected) && FN_key_pressed != 0) {
      FN_timeouts_in_progress = true;
      fnTimerStart(TO_FN_LONG, TO_FN_LONG,  FN_state == ST_1_PRESS1 ? TIME_FN_12XX_TO_F : TIME_FN_DOUBLE_G_TO_NOP);    //dr
      FN_timed_out_to_NOP_or_Executed = false;


      if(!shiftF && !shiftG) {
        char *varCatalogItem = "SF:U";
        int16_t Dyn = nameFunction(FN_key_pressed-37, shiftF, shiftG);
        //printf("dynamicMenuItem=%i %i\n", dynamicMenuItem, Dyn);
        if(dynamicMenuItem > -1 && !DEBUGSFN) {
          varCatalogItem = dynmenuGetLabel(dynamicMenuItem);
        }
        showFunctionName(Dyn, 0, varCatalogItem);
        underline_softkey(1<<(FN_key_pressed-38), 0);
      }


      else if(shiftF && !shiftG) {
        char *varCatalogItem = "SF:V";
        int16_t Dyn = nameFunction(FN_key_pressed-37, shiftF, shiftG);
        //printf("dynamicMenuItem=%i %i\n", dynamicMenuItem, Dyn);
        if(dynamicMenuItem > -1 && !DEBUGSFN) {
          varCatalogItem = dynmenuGetLabel(dynamicMenuItem);
        }
        showFunctionName(Dyn, 0,  varCatalogItem);
        underline_softkey(1<<(FN_key_pressed-38), 1);
      }


      else if(!shiftF && shiftG) {
        char *varCatalogItem = "SF:W";
        int16_t Dyn = nameFunction(FN_key_pressed-37, shiftF, shiftG);
        //printf("dynamicMenuItem=%i\n", dynamicMenuItem);
        if(dynamicMenuItem > -1 && !DEBUGSFN) {
          varCatalogItem = dynmenuGetLabel(dynamicMenuItem);
        }
        showFunctionName(Dyn, 0,  varCatalogItem);
        underline_softkey(1<<(FN_key_pressed-38), 2);
      }                                                                       //further shifts are done within FN_handler
    }
    //#if defined(INLINE_TEST)
    //  if(testEnabled) {
    //    fnSwStop(1);
    //  }
    //#endif // INLINE_TEST
  }


  //*************----------*************------- FN KEY RELEASED -------***************-----------------
  void btnFnReleased_StateMachine
    #if defined(PC_BUILD)                                                             //JM LONGPRESS FN
      (GtkWidget *unused, gpointer data)
    #endif // PC_BUILD
    #if defined(DMCP_BUILD)
      (void *unused, void *data)
    #endif // DMCP_BUILD
    {
    //#if defined(INLINE_TEST)
    //  if(testEnabled) {
    //    fnSwStart(2);
    //  }
    //#endif // INLINE_TEST

    #if defined(VERBOSEKEYS)
      printf(">>>>Z 0050A btnFnReleased_StateMachine ------------------ FN_state=%d\n", FN_state);
    #endif // VERBOSEKEYS

    if(FN_state == ST_3_PRESS2) {
      FN_state = ST_4_REL2;
    }
    else {
      FN_state = ST_2_REL1;
    }

    #if defined(VERBOSEKEYS)
      printf(">>>>Z 0050B btnFnReleased_StateMachine ------------------ FN_state=%d\n", FN_state);
    #endif // VERBOSEKEYS

    if(getSystemFlag(FLAG_G_DOUBLETAP) && !BLOCK_DOUBLEPRESS_MENU(currentMenu(),FN_key_pressed-38,0) && FN_state == ST_2_REL1 && FN_handle_timed_out_to_EXEC) {
      uint8_t offset =  0;
      if(shiftF && !shiftG) {
        offset =  6;
      }
      else if(!shiftF && shiftG) {
        offset = 12;
      }
      fnTimerStart(TO_FN_EXEC, FN_key_pressed + offset, TIME_FN_DOUBLE_RELEASE); // if it times out, it goes to execFnTimeout

      #if defined(VERBOSEKEYS)
        printf(">>>>Z 0050 btnFnReleased_StateMachine ------------------ Start TO_FN_EXEC\n          data=|%s| data[0]=%d (Global) FN_key_pressed=%d +offset=%d\n",(char*)data,((char*)data)[0], FN_key_pressed, offset);
      #endif // VERBOSEKEYS

      //FN_key_pressed = *((char *)data) - '0' + 37;  //to render 38-43, as per original keypress
      //This parameter of the timer is non-standard: 38-43 for unshifted, +6 for f, +12 for g.
    }

      // **************JM LONGPRESS EXECUTE****************************************************
    char charKey[3];
    charKey[0]=0;
    bool_t EXEC_pri;
    EXEC_pri = (FN_timeouts_in_progress && (FN_key_pressed != 0));
    // EXEC_FROM_LONGPRESS_RELEASE     EXEC_FROM_LONGPRESS_TIMEOUT  EXEC FN primary
    if((FN_timed_out_to_RELEASE_EXEC || FN_timed_out_to_NOP_or_Executed || EXEC_pri ))  {                  //JM DOUBLE: If slower ON-OFF than half the limit (250 ms)
      underline_softkey(1<<(FN_key_pressed-38), 3);   //Purposely in row 3 which does not exist, just to activate the clear previous line
      charKey[1]=0;
      charKey[0]=FN_key_pressed + (-37+48);

      hideFunctionName();

      if(!FN_timed_out_to_NOP_or_Executed && fnTimerGetStatus(TO_FN_EXEC) != TMR_RUNNING) {
        #if defined(VERBOSEKEYS)
          printf(">>>>Z RRR2 LONGPRESS EXECUTE              ------------------       TO_FN_EXEC\n          charKey=|%s| charkey[0]=%d \n", charKey, charKey[0]);
        #endif // VERBOSEKEYS
        btnFnClicked(unused, charKey);                                             //Execute
      }

      uint8_t screenUpdatingModeMeM = screenUpdatingMode;
      resetShiftState();
      screenUpdatingMode = screenUpdatingModeMeM;      //clear skip that was set in resetShiftState()
      screenUpdatingMode &= ~SCRUPD_MANUAL_STATUSBAR;

      if(!(calcMode == CM_REGISTER_BROWSER || calcMode == CM_FLAG_BROWSER || calcMode == CM_ASN_BROWSER || calcMode == CM_FONT_BROWSER || GRAPHMODE || calcMode == CM_LISTXY)) {
        if((calcMode == CM_ASSIGN && itemToBeAssigned == 0) || FN_timed_out_to_NOP_or_Executed) { //Clear any possible underline residues
          showSoftmenuCurrentPart();
        }
      }

      FN_cancel();
    }

    //**************JM LONGPRESS AND JM DOUBLE ^^ *********************************************   // JM FN-DOUBLE
    //#if defined(INLINE_TEST)
    //  if(testEnabled) {
    //    fnSwStop(2);
    //  }
    //#endif // INLINE_TEST
  }


  void execFnTimeout(uint16_t key) {                          //dr - delayed call of the primary function key
    char charKey[3];
    charKey[1] = 0;
    charKey[0] = key + (-37+48);

    #if defined(VERBOSEKEYS)
      printf(">>>>Z RRR3 execFnTimeout              ------------------       TO_FN_EXEC\n          charKey=|%s| charkey[0]=%d key+11=%d \n", charKey, charKey[0], key+11);
    #endif // VERBOSEKEYS

    if(!FN_timed_out_to_NOP_or_Executed) {
      btnFnClicked(NULL, (char *)charKey);
    }
  }


  void shiftCutoff(uint16_t unusedButMandatoryParameter) {        //dr - press shift three times within one second to call HOME timer
    fnTimerStop(TO_3S_CTFF);
  }
  //---------------------------------------------------------------
#endif // !TESTSUITE_BUILD


bool_t caseReplacements(uint8_t id, bool_t lowerCaseSelected, int16_t item, int16_t *itemOut) {
    *itemOut = item;
    if(lowerCaseSelected && (ITM_A <= item && item <= ITM_Z)) {
      *itemOut = item + (ITM_a - ITM_A);
      goto returnTrue;
    }
    else if(!lowerCaseSelected && (ITM_A <= item && item <= ITM_Z)) {  //JM
      goto returnTrue;
    }
    else if(!lowerCaseSelected && (ITM_a <= item && item <= ITM_z)) {  //JM
      *itemOut = item - (ITM_a - ITM_A);
      goto returnTrue;
    }
    else if(lowerCaseSelected && (ITM_a <= item && item <= ITM_z)) {  //JM
      goto returnTrue;
    }
    goto returnFalse;

returnTrue:
    #if defined(VERBOSEKEYS) || defined(PAIMDEBUG)
      printf("Translating upper/lower case: lowercaseselected=%i item=%i itemOut=%i\n",lowerCaseSelected, item, *itemOut);
      switch(id){
        case 0: printf("keyboard.c processAimInput() %i\n",id); break;
        case 1: printf("manage.c   pemAlpha()        %i\n",id); break;
        case 2: printf("gtkGui.c   sendKey()         %i\n",id); break;
        default:;
      }
    #endif //VERBOSEKEYS || PAIMDEBUG
    return true;

returnFalse:
    #if defined(VERBOSEKEYS) || defined(PAIMDEBUG)
      printf("Translating upper/lower case: No translation: lowercaseselected=%i item=%i itemOut=%i\n",lowerCaseSelected, item, *itemOut);
    #endif //VERBOSEKEYS || PAIMDEBUG
    return false;
  }




//numlock replacements require the case to be upper case
uint16_t numlockReplacements(uint16_t id, int16_t item, bool_t NL, bool_t FSHIFT, bool_t GSHIFT) {
  int16_t item1 = 0;
  //printf("####A>>%u %u %u\n", id, item, item1);

  if(keyReplacements(item, &item1, NL, FSHIFT, GSHIFT)) {
    return max(item1, -item1);
  }
  else {
    return max(item, -item);
  }
}


 //Note item1 MUST be set to 0 prior to calling.
 bool_t keyReplacements(int16_t item, int16_t *item1, bool_t NL, bool_t FSHIFT, bool_t GSHIFT) {
   //printf("####B>> %d %d\n", item, *item1);
   if(calcMode == CM_AIM || calcMode == CM_EIM || calcMode == CM_PEM || (tam.mode && tam.alpha) || (calcMode == CM_ASSIGN && itemToBeAssigned == 0)) {

    if(GSHIFT) {        //ensure that sigma and delta stays uppercase
      switch(item) {
        case ITM_sigma:         *item1 = ITM_SIGMA;        break;
        case ITM_delta:         *item1 = ITM_DELTA;        break;
        case ITM_NULL:          *item1 = ITM_SPACE;        break;
        default:                                           break;
      }
    }

    else if(NL) {       //JMvv Numlock translation: Assumes lower case is NOT active

      item -= (ITM_A + 26 <= item && item <= ITM_Z + 26) ? -26 : 0; //Ensures lower case is NOT active
      uint16_t ix = 15; //from EEX to the bottom of the keyboard, last key 37
      while(ix < 37) {
        if(kbd_std[ix].primaryAim != ITM_EXIT1 && kbd_std[ix].primaryAim != ITM_UP1 && kbd_std[ix].primaryAim != ITM_DOWN1 && kbd_std[ix].primaryAim != ITM_BACKSPACE) {
          if(!FSHIFT && item == kbd_std[ix].primaryAim) {
            *item1 = getSystemFlag(FLAG_USER) ? kbd_usr[ix].gShiftedAim : kbd_std[ix].gShiftedAim;
            break;
          }
          if(FSHIFT && ix >= 31 && item == kbd_std[ix].gShiftedAim) {                            //Originally for C47: ITM_MINUS, ITM_PLUS, ITM_SLASH, ITM_PERIOD, ITM_0
            *item1 = getSystemFlag(FLAG_USER) ? kbd_usr[ix].primaryAim : kbd_std[ix].primaryAim;
            break;
          }
        }
        ix++;
      }

    }
  }
  return *item1 != 0;
}


#if defined(DMCP_BUILD)                                           //vv dr - internal keyBuffer POC
  //#define JMSHOWCODES_KB0   // top line left    Key value from DM42. Not necessarily pushed to buffer.
  //#define JMSHOWCODES_KB1   // top line middle  Key and  dT value
  //#define JMSHOWCODES_KB2   // main screen      Telltales keys, times, etc.
  //#define JMSHOWCODES_KB3   // top line right   Single Double Triple

  void keyBuffer_pop(void) {
    int tmpKey;

    do {
      tmpKey = -1;
      if(!fullKeyBuffer()) {
        tmpKey = key_pop();
        #if CALCMODEL == USER_R47
          tmpKey = convertKeyCode(tmpKey);
        #endif
        if(tmpKey >= 0) {
          inKeyBuffer(tmpKey);
        }
      }
    } while(tmpKey >= 0);
  }

  #if defined(JMSHOWCODES_KB0)
    uint16_t tmpxx = 1;
  #endif // JMSHOWCODES_KB0

  #if defined(BUFFER_CLICK_DETECTION)
    kb_buffer_t buffer = {{}, {}, 0, 0};
  #else // !BUFFER_CLICK_DETECTION
    kb_buffer_t buffer = {{}, 0, 0};
  #endif //BUFFER_CLICK_DETECTION

  // Stores 1 Byte in the ring buffer
  //
  // Returns:
  //     BUFFER_FAIL       ring buffer is full. No further byte can be stored
  //     BUFFER_SUCCESS    the byte was stored
  uint8_t inKeyBuffer(uint8_t byte) {
    #if defined(BUFFER_CLICK_DETECTION)
      uint32_t now  = (uint32_t)sys_current_ms();
    #endif // BUFFER_CLICK_DETECTION

    uint8_t  next = ((buffer.write + 1) & BUFFER_MASK);

    if(buffer.read == next) {
      return BUFFER_FAIL;  // full
    }

    // EXPERIMENT Do not allow the same key to be stored multiple times. Only key changes stored.
    if(buffer.data[(buffer.write - 1) & BUFFER_MASK] == byte) {
      #if defined(JMSHOWCODES_KB0)
        char aaa[16];
        sprintf   (aaa,"%2d ",byte);
        showString(aaa,&standardFont, tmpxx++, 1, vmNormal, true, true);
      #endif // JMSHOWCODES_KB0

      return BUFFER_FAIL;  // duplicate
    }
    // END EXPERIMENT

    #if defined(JMSHOWCODES_KB0)
      tmpxx = 1;
    #endif // JMSHOWCODES_KB0

    buffer.data[buffer.write & BUFFER_MASK] = byte;

    #if defined(BUFFER_CLICK_DETECTION)
      buffer.time[buffer.write & BUFFER_MASK] = now;
    #endif // BUFFER_CLICK_DETECTION

    buffer.write = next;

    return BUFFER_SUCCESS;
  }


// Fetches 1 byte from the ring buffer if at least one is ready to be fetched
//
// Returns:
//     BUFFER_FAIL       Fetches 1 byte from the ring buffer if at least one is ready to be fetched
//     BUFFER_SUCCESS    1 byte was fetched
#if defined(BUFFER_CLICK_DETECTION)
  #if defined(BUFFER_KEY_COUNT)
    uint8_t outKeyBuffer(uint8_t *pKey, uint8_t *pKeyCount, uint32_t *pTime, uint32_t *pTimeSpan_1, uint32_t *pTimeSpan_B)
  #else // !BUFFER_KEY_COUNT
    uint8_t outKeyBuffer(uint8_t *pKey, uint32_t *pTime, uint32_t *pTimeSpan_1, uint32_t *pTimeSpan_B)
  #endif // BUFFER_KEY_COUNT
#else // !BUFFER_CLICK_DETECTION
  #if defined(BUFFER_KEY_COUNT)
    uint8_t outKeyBuffer(uint8_t *pKey, uint8_t *pKeyCount)
  #else // !BUFFER_KEY_COUNT
    uint8_t outKeyBuffer(uint8_t *pKey)
  #endif // BUFFER_KEY_COUNT
#endif // BUFFER_CLICK_DETECTION
{
  if(buffer.read == buffer.write) {
    return BUFFER_FAIL;  // empty
  }

  uint8_t actKey = buffer.data[buffer.read];
  *pKey = actKey;
  #if defined(BUFFER_KEY_COUNT)
    uint8_t keyCount = 0;
    if(actKey > 0) {
      keyCount++;
      if(buffer.data[(buffer.read - 2) & BUFFER_MASK] == actKey) {
        keyCount++;
        if(buffer.data[(buffer.read - 4) & BUFFER_MASK] == actKey) {
          keyCount++;
        }
      }
    }
    *pKeyCount = keyCount;
  #endif // BUFFER_KEY_COUNT

  #if defined(BUFFER_CLICK_DETECTION)
    uint32_t actTime = buffer.time[buffer.read];
    *pTime = actTime;
    uint32_t oldTime;

    oldTime = buffer.time[(buffer.read - 1) & BUFFER_MASK];
    if(actTime >= oldTime) {
      *pTimeSpan_1 = actTime - oldTime;
    }
    else {            // protect uint overflow
      *pTimeSpan_1 = actTime;
    }

    #if defined(BUFFER_KEY_COUNT)
      if(keyCount > 2) {
        oldTime = buffer.time[(buffer.read - 4) & BUFFER_MASK];
      }
      else {
    #else // !BUFFER_KEY_COUNT
           {
    #endif // BUFFER_KEY_COUNT
      oldTime = buffer.time[(buffer.read - 2) & BUFFER_MASK];
    }
    if(actTime >= oldTime) {
      *pTimeSpan_B = actTime - oldTime;
    }
    else {            // protect uint overflow
      *pTimeSpan_B = actTime;
    }
  #endif // BUFFER_CLICK_DETECTION

  buffer.read = (buffer.read + 1) & BUFFER_MASK;

  #if defined(BUFFER_CLICK_DETECTION)
    uint8_t detectionResult = outKeyBufferDoubleClick();
    #if defined(JMSHOWCODES_KB1)
      fnDisplayStack(2);
      char aaa[50];
      sprintf(aaa, "k=%2d dT=%5lu:%d", *pByte, *pTimeSpan, (uint8_t)tmpTime);
      showString(aaa, &standardFont, 220, 1, vmNormal, true, true);
    #endif // JMSHOWCODES_KB1
  #endif // BUFFER_CLICK_DETECTION

  return BUFFER_SUCCESS;
}


/* Switching profile, single press, double press and triple press:

SINGLEPRESS:
unknown  waiting   pres    rel
---D3--x_____D2____x---D1--x_
      t-3         t-2     t-1


DOUBLEPRESS:
unknown  waiting   pres    rel    pres
-------x___________x-------x______x-
      t-4    D3   t-3  D2 t-2 D1 t-1


TRIPLEPRESS:
unknown  waiting   pres    rel    pres   rel    pres
-------x___________x-------x______x------x______x-
                          t-4 D3 t-3 D2 t-2 D1 t-1



Circular key buffer:

 Pointers are ready to read and ready to write.
 Pointers are incremented after action.
 Allow writing of 3 keys ahead, into: R+0, R+1, R+2. R+3 is blocked.
 Allow reading of 3 keys up to R=W, which is not read.

 +------------------------------------+
 |   1                                | emptyKeyBuffer, not used
 | R 2 W   R=W: Empty buffer, cannot  | outKeyBuffer
 |   3          read.                 |
 |   4                                |    (Will not pop a key which is not pushed. Can look at 4 places back directly at the stack. )
 +------------------------------------+
 |   1 W   W+1 NOT= R: Space for 1+   | isMoreBufferSpace, used by keyBuffer_pop writing to the buffer only when there is space
 | R 2                                |
 |   3                                |
 |   4                                |
 +------------------------------------+
 |   1                                | inKeyBuffer
 | R 2                                |
 |   3 W   R+1=W: Write buffer full.  |    (must avoid this one, because the key is lost then. Must rather test first and keep the key in the DM42 buffer)
 |   4            Failed. Return      |
 +------------------------------------+

 +-------------------------------------------------------------------------------+
 |   1 W    X     iv.block access to inKeyBuffer, i.e. keep key in DM42 buffer.  |
 | R 2 ov      i.write o and move down v                                         |
 |   3 o v      ii.write o and move down v                                       |
 |   4 o  v      iii.write o and move down v                                     |
 +-------------------------------------------------------------------------------+
*/


// Returns: true if double click
uint8_t outKeyBufferDoubleClick(void) {
  #if defined(BUFFER_CLICK_DETECTION)
    //WARNING! this triggers conseq double click to be 'triple' click but does not check it is the SAME key.
    //Buffer of 4 to short for that. Buffer of 8 sufficient.
    //Can be fixed by having a single byte added to the rolling stack catching the key which was rolled out

    int16_t dTime_1, dTime_2, dTime_3;
    bool_t  doubleclicked, tripleclicked;
    uint8_t outDoubleclick;

    //note Delta Time 1 is the most recent, Delta time 3 is the oldest
    dTime_1 = (uint16_t) buffer.time[(buffer.read-1) & BUFFER_MASK] - (uint16_t) buffer.time[(buffer.read-2) & BUFFER_MASK];
    dTime_2 = (uint16_t) buffer.time[(buffer.read-2) & BUFFER_MASK] - (uint16_t) buffer.time[(buffer.read-3) & BUFFER_MASK];
    dTime_3 = (uint16_t) buffer.time[(buffer.read-3) & BUFFER_MASK] - (uint16_t) buffer.time[(buffer.read-4) & BUFFER_MASK];

    #define D1 150 //400 //space before last press, released time
    #define D2 200 //length of first press, pressed down time

  doubleclicked =
         buffer.data[(buffer.read-1) & BUFFER_MASK] != 0   //check that the last incoming keys was a press, not a release
      && buffer.data[(buffer.read-1) & BUFFER_MASK] == buffer.data[(buffer.read-3) & BUFFER_MASK]   //check that the two last keys are the same, otherwise itis a glisando
      && (dTime_1 > 10 ) && (dTime_1 < D1)                //check no chatter > 10 ms & released width is not longer than limit
      && (dTime_2 > 10 ) && (dTime_2 < D2);               //check no chatter > 10 ms & pressed width is not longer than limit

  if(dTime_1+dTime_2 > D1+D2) {
    doubleclicked = false;
  }

  #define TD1 150 //space before last press, released time
  #define TD2 200 //length of middle press, pressed down time
  #define TD3 150 //space before middle press, i.e. released time after first press

  tripleclicked =
         buffer.data[(buffer.read-1) & BUFFER_MASK] != 0   //check that the last incoming keys was a press, not a release
      && buffer.data[(buffer.read-1) & BUFFER_MASK] == buffer.data[(buffer.read-3) & BUFFER_MASK]   //check that the two last keys are the same, otherwise itis a glisando
      && buffer.data[(buffer.read-1) & BUFFER_MASK] == buffer.data[(buffer.read-5) & BUFFER_MASK]   //check that the previous key is the same. If buffer is 4 long only, it will wrap and not check the first triple press
      && (dTime_1 > 10 ) && (dTime_1 < TD1)                //check no chatter > 10 ms & released width is not longer than limit
      && (dTime_2 > 10 ) && (dTime_2 < TD2)                //check no chatter > 10 ms & pressed width is not longer than limit
      && (dTime_3 > 10 ) && (dTime_3 < TD3);               //check no chatter > 10 ms & pressed width is not longer than limit

  if(dTime_1+dTime_2+dTime_3 > TD1+TD2+TD3) {
    tripleclicked = false;
  }

  if(tripleclicked) {
    outDoubleclick = 3;
    }
  else if(doubleclicked) {
    outDoubleclick = 2;
  }
  else if(buffer.data[(buffer.read-1) & BUFFER_MASK] != 0) {
    outDoubleclick = 1;
  }
  else {
    outDoubleclick = 0;
  }

  #if defined(JMSHOWCODES_KB2)
    char line[50], line1[100];
    fnDisplayStack(2);
    sprintf(line1,"R%-1dW%-1d -1:%-5d -2:%-5d -3:%-5d -4:%-5d  ", (uint16_t)buffer.read,(uint16_t)buffer.write,
      (uint16_t)buffer.time[(buffer.read-1) & BUFFER_MASK], (uint16_t)buffer.time[(buffer.read-2) & BUFFER_MASK],(uint16_t)buffer.time[(buffer.read-3) & BUFFER_MASK],(uint16_t)buffer.time[(buffer.read-4) & BUFFER_MASK]);
    showString(line1, &standardFont, 1, Y_POSITION_OF_REGISTER_X_LINE - REGISTER_LINE_HEIGHT*(REGISTER_T - REGISTER_X), vmNormal, true, true);
    sprintf(line, "B-1:%d %d %d %d", (uint16_t)buffer.data[(buffer.read-1) & BUFFER_MASK], (uint16_t)buffer.data[(buffer.read-2) & BUFFER_MASK], (uint16_t)buffer.data[(buffer.read-3) & BUFFER_MASK], (uint16_t)buffer.data[(buffer.read-4) & BUFFER_MASK]);
    sprintf(line1, "%-16s   D1:%d D2:%d D3:%d    ", line,dTime_1, dTime_2, dTime_3);
    showString(line1, &standardFont, 1, Y_POSITION_OF_REGISTER_X_LINE - REGISTER_LINE_HEIGHT*(REGISTER_Z - REGISTER_X), vmNormal, true, true);
  #endif // JMSHOWCODES_KB2

  #if defined(JMSHOWCODES_KB3)
    char line2[10];
    line2[0] = 0;
    if( outDoubleclick == 1) {
      strcat(line2, "S");
      showString(line2, &standardFont, SCREEN_WIDTH-11, 0, vmNormal, true, true);
    }
    else
    if( outDoubleclick == 2) {
      strcat(line2, "D");
      showString(line2, &standardFont, SCREEN_WIDTH-11, 0, vmNormal, true, true);
    }
    else
    if( outDoubleclick == 3) {
      strcat(line2, "T");
      showString(line2, &standardFont, SCREEN_WIDTH-11, 0, vmNormal, true, true);
    }
    else {
      strcat(line2, " ");
      showString(line2, &standardFont, SCREEN_WIDTH-11, 0, vmNormal, true, true);
    }
  #endif // JMSHOWCODES_KB3

  return outDoubleclick;
#else // !BUFFER_CLICK_DETECTION
  return 255;
#endif // BUFFER_CLICK_DETECTION
}


// Returns:
//     true              ring buffer is full
bool_t fullKeyBuffer(void) {
  return buffer.read == ((buffer.write + 1) & BUFFER_MASK);
}


// Returns:
//     true              ring buffer is empty
bool_t emptyKeyBuffer(void) {
  return buffer.read == buffer.write;
}

void clearKeyBuffer(void) {
  buffer.read = buffer.write;
}
#endif // DMCP_BUILD                                                    //^^



//########################################
void fnT_ARROW(uint16_t command) {
  #if !defined(TESTSUITE_BUILD)
    #if defined(TEXT_MULTILINE_EDIT)
      uint16_t ixx;
      uint16_t current_cursor_x_old;
      uint16_t current_cursor_y_old;

      #if defined(PC_BUILD)
        char tmp[200]; sprintf(tmp,"^^^^fnT_ARROW: command=%d current_cursor_x=%d current_cursor_y=%d \n",command,current_cursor_x, current_cursor_y); jm_show_comment(tmp);
      #endif //PC_BUILD

      switch(command) {
        case ITM_T_LEFT_ARROW: /*STD_LEFT_ARROW */
          T_cursorPos = stringPrevGlyph(aimBuffer, T_cursorPos);
          if(T_cursorPos < displayAIMbufferoffset && displayAIMbufferoffset > 0) {
            displayAIMbufferoffset = stringPrevGlyph(aimBuffer, displayAIMbufferoffset);
          }
          break;

        case ITM_T_RIGHT_ARROW: /*STD_RIGHT_ARROW*/
          ixx = T_cursorPos;
          T_cursorPos = stringNextGlyph(aimBuffer, T_cursorPos);
          incOffset();
          break;

        case ITM_T_LLEFT_ARROW: /*STD_FARLEFT_ARROW */
          ixx = 0;
          while(ixx<10) {
            fnT_ARROW(ITM_T_LEFT_ARROW);
            ixx++;
          }
          break;

        case ITM_T_RRIGHT_ARROW: /*STD_FARRIGHT_ARROW*/
          ixx = 0;
          while(ixx<10) {
            fnT_ARROW(ITM_T_RIGHT_ARROW);
            ixx++;
          }
          break;

        case ITM_T_UP_ARROW: /*UP */                       //JMCURSOR try make the cursor upo be more accurate. Add up the char widths...
          ixx = 0;
          current_cursor_x_old = current_cursor_x;
          current_cursor_y_old = current_cursor_y;
          fnT_ARROW(ITM_T_RIGHT_ARROW);
          while(ixx < 75 && (current_cursor_x >= current_cursor_x_old+5 || current_cursor_y == current_cursor_y_old)) {
            fnT_ARROW(ITM_T_LEFT_ARROW);
            showStringEdC47(multiEdLines ,displayAIMbufferoffset, T_cursorPos, aimBuffer, 1, -100, vmNormal, true, true, true);  //display up to the cursor
            ixx++;
          }
          break;

        case ITM_T_DOWN_ARROW: /*DN*/
          ixx = 0;
          current_cursor_x_old = current_cursor_x;
          current_cursor_y_old = current_cursor_y;
          fnT_ARROW(ITM_T_LEFT_ARROW);
          while(ixx < 75 && (current_cursor_x+5 <= current_cursor_x_old || current_cursor_y == current_cursor_y_old)) {
            fnT_ARROW(ITM_T_RIGHT_ARROW);
            showStringEdC47(multiEdLines ,displayAIMbufferoffset, T_cursorPos, aimBuffer, 1, -100, vmNormal, true, true, true);  //display up to the cursor
            ixx++;

            //printf("###^^^ %d %d %d %d %d\n",ixx,current_cursor_x, current_cursor_x_old, current_cursor_y, current_cursor_y_old);
          }
          break;

        case ITM_UP1: /*HOME */
            T_cursorPos = 0;
            displayAIMbufferoffset = 0;
            break;

        case ITM_DOWN1: /*END*/
            T_cursorPos = stringByteLength(aimBuffer) - 1;
            T_cursorPos = stringNextGlyph(aimBuffer, T_cursorPos);
            fnT_ARROW(ITM_T_RIGHT_ARROW);
            findOffset();
            break;


        default: break;
      }

      //printf(">>> T_cursorPos %d", T_cursorPos);
      if(T_cursorPos > stringNextGlyph(aimBuffer, stringLastGlyph(aimBuffer))) {
        T_cursorPos = stringNextGlyph(aimBuffer, stringLastGlyph(aimBuffer));
      }
      if(T_cursorPos < 0) {
        T_cursorPos = 0;
      }
      //printf(">>> T_cursorPos limits %d\n",T_cursorPos);
    #endif // TEXT_MULTILINE_EDIT
  #endif // !TESTSUITE_BUILD
}


void fnCla(uint16_t unusedButMandatoryParameter) {
  if(calcMode == CM_AIM) {
    //Not using calcModeAim becose some modes are reset which should not be
    aimBuffer[0]=0;
    T_cursorPos = 0;
    nextChar = scrLock;
    xCursor = 1;
    yCursor = Y_POSITION_OF_AIM_LINE + 6;
    cursorFont = &standardFont;
    cursorEnabled = true;
    #if !defined(TESTSUITE_BUILD)
      clearRegisterLine(AIM_REGISTER_LINE, true, true);
      refreshRegisterLine(AIM_REGISTER_LINE);        //JM Execute here, to make sure that the 5/2 line check is done
    #endif // !TESTSUITE_BUILD
    screenUpdatingMode &= ~SCRUPD_MANUAL_STACK;
  }
  else if(calcMode == CM_EIM) {
    fnEqCla();
    refreshRegisterLine(NIM_REGISTER_LINE);
    screenUpdatingMode &= ~SCRUPD_MANUAL_STACK;
  }
}


void fnCln(uint16_t unusedButMandatoryParameter) {
  #if !defined(TESTSUITE_BUILD)
   strcpy(aimBuffer,"+0");
   fnKeyBackspace(0);
   setSystemFlag(FLAG_ASLIFT);
   screenUpdatingMode = SCRUPD_AUTO;
   screenUpdatingMode |= SCRUPD_SKIP_STATUSBAR_ONE_TIME;
//   refreshScreen();
  #endif // !TESTSUITE_BUILD
}


void refreshModeGui(void) {
  if(!tam.mode) {
    switch(calcMode) {
      case CM_AIM:
      case CM_EIM:
        calcModeAimGui();
        break;

      case CM_NORMAL:
        calcModeNormalGui();
        break;

      case CM_PEM:
        if(getSystemFlag(FLAG_ALPHA)) {
          calcModeAimGui();
        }
        else {
          calcModeNormalGui();
        }
        break;
    }
  }
}
