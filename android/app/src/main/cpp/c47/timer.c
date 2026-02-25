// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"

kb_timer_t  timer[TMR_NUMBER];
#if defined(PC_BUILD)
gint64      timerLastCalled;
#endif // PC_BUILD

#if defined(DMCP_BUILD)
uint32_t    timerLastCalled;
bool_t      mutexRefreshTimer = false;
#endif // DMCP_BUILD


uint32_t getUptimeMs(void) {
  #if defined(DMCP_BUILD)
    return (uint32_t)sys_current_ms();
  #elif !defined(TESTSUITE_BUILD)
    return (uint32_t)(g_get_monotonic_time() / 1000);
  #else // TESTSUITE_BUILD
    return 0;
  #endif // DMCP_BUILD ; TESTSUITE_BUILD
}


void fnTicks(uint16_t unusedButMandatoryParameter) {
  uint32_t tim;
  longInteger_t lgInt;

  tim = getUptimeMs() / 100;

  liftStack();
  longIntegerInit(lgInt);
  uInt32ToLongInteger(tim, lgInt);
  convertLongIntegerToLongIntegerRegister(lgInt, REGISTER_X);
  longIntegerFree(lgInt);
}

void LastOpTimerReStart (uint16_t func) {
  timeLastOp0 = getUptimeMs() / 100;
  //printf("Func:%s setting %u Running:%u\n",indexOfItems[func].itemCatalogName, timeLastOp0, programRunStop == PGM_RUNNING);
}

void LastOpTimerLap (uint16_t func) {
  timeLastOp1 = getUptimeMs() / 100;
  if(timeLastOp1 >= timeLastOp0) {
    timeLastOp = timeLastOp1 - timeLastOp0;
    //printf("Func:%s setting STOP %u: %u Running:%u\n",indexOfItems[func].itemCatalogName, timeLastOp1, timeLastOp, programRunStop == PGM_RUNNING);
  }
  else {
    timeLastOp =  ((int)(0xFFFFFFFF) / 100 - timeLastOp0) + timeLastOp1; //if loop passed 2^32-1 ms, recalc offset
    //printf("setting STOP Wrapped %u: %u Running:%u\n",timeLastOp1, timeLastOp, programRunStop == PGM_RUNNING);
  }
  if(timeLastOp > 30*24*60*60*10) {                                      //More than one month is an unreasonable amount and probably is due to either timeLastOp0 or timeLastOp1 not set correctly
    timeLastOp = 0;
    //printf("setting STOP Error too long %u: %u\n",timeLastOp1, timeLastOp);
  }
}


void fnLastT (uint16_t unusedButMandatoryParameter) {
  longInteger_t lgInt;
  liftStack();
  longIntegerInit(lgInt);
  uInt32ToLongInteger(timeLastOp, lgInt);
  convertLongIntegerToLongIntegerRegister(lgInt, REGISTER_X);
  longIntegerFree(lgInt);
}



void fnRebuildTimerRefresh(void) {
  #if defined(DMCP_BUILD)
  uint32_t next;

  if(mutexRefreshTimer == false) {
    nextTimerRefresh = 0;
    for(int i = 0; i < TMR_NUMBER; i++) {
      if(timer[i].state == TMR_RUNNING) {
        next = timer[i].timer_will_expire;
        if(nextTimerRefresh != 0 && next < nextTimerRefresh) {
          nextTimerRefresh = next;
        }
        if(nextTimerRefresh == 0) {
          nextTimerRefresh = next;
        }
      }
    }
  }
  #endif // DMCP_BUILD
}


#if defined(PC_BUILD)
/********************************************//**
 * \brief Refreshes timer. This function is
 * called every 5 ms by a GTK timer.
 *
 * \param[in] data gpointer Not used
 * \return gboolean         What will happen next?
 *                          * true  = timer will call this function again
 *                          * false = timer stops calling this function
 ***********************************************/
gboolean refreshTimer(gpointer data) {      // This function is called every 5 ms by a GTK timer
  gint64 now = g_get_monotonic_time();

  if(now < timerLastCalled) {
    for(int i = 0; i < TMR_NUMBER; i++) {
      if(timer[i].state == TMR_RUNNING) {
        timer[i].state = TMR_COMPLETED;
        timer[i].func(timer[i].param);      // Callback to configured function
      }
    }
  }
  else {
    for(int i = 0; i < TMR_NUMBER; i++) {
      if(timer[i].state == TMR_RUNNING) {
        if(timer[i].timer_will_expire <= now) {
          timer[i].state = TMR_COMPLETED;
          timer[i].func(timer[i].param);    // Callback to configured function
        }
      }
    }
  }

  timerLastCalled = now;

//fnRebuildTimerRefresh();

  return TRUE;
}
#endif // PC_BUILD

#if defined(DMCP_BUILD)
void refreshTimer(void) {                   // This function is called when nextTimerRefresh has been elapsed
  uint32_t now = (uint32_t)sys_current_ms();

  if(now < timerLastCalled) {
    for(int i = 0; i < TMR_NUMBER; i++) {
      if(timer[i].state == TMR_RUNNING) {
        timer[i].state = TMR_COMPLETED;
        mutexRefreshTimer = true;
        timer[i].func(timer[i].param);      // Callback to configured function
        mutexRefreshTimer = false;
      }
    }
  }
  else {
    for(int i = 0; i < TMR_NUMBER; i++) {
      if(timer[i].state == TMR_RUNNING) {
        if(timer[i].timer_will_expire <= now) {
          timer[i].state = TMR_COMPLETED;
          mutexRefreshTimer = true;
          timer[i].func(timer[i].param);    // Callback to configured function
          mutexRefreshTimer = false;
        }
      }
    }
  }

  timerLastCalled = now;

  fnRebuildTimerRefresh();
}
#endif // DMCP_BUILD


void fnTimerDummy1(uint16_t param) {
#if defined(PC_BUILD) || defined(TESTSUITE_BUILD)
  printf("fnTimerDummy1 called  %u\n", param);
#endif // PC_BUILD || TESTSUITE_BUILD
}



void fnTimerEndOfActivity(uint16_t param) {
#if defined(PC_BUILD) || defined(TESTSUITE_BUILD)
  printf("fnTimerEndOfActivity called  %u\n", param);
#endif // PC_BUILD || TESTSUITE_BUILD

#if defined(DMCP_BUILD)
                                #if defined(DM42_POWERMARKS)
                                  powerMarkerMsF(1,10000);
                                #endif //DM42_POWERMARKS

  if(skippedStackLines) {       //update screen after 1 or 2 sec timout, to restore the half-updated screen in battery mode. See refreshRegisterLine() in screen.c
    screenUpdatingMode = SCRUPD_AUTO;
    skippedStackLines = false;
    refreshScreen(32);
    //if(skippedStackLines) {
    //  print_linestr("more skippedStackLines #######", false);
    //  fnTimerStart(TO_KB_ACTV, TO_KB_ACTV, skippedStackLines ? TO_KB_ACTV_MEDIUM/5 : TO_KB_ACTV_SHORT);
    //}
  }
                                #if defined(DM42_POWERMARKS)
                                  powerMarkerMsF(1,15000);
                                #endif //DM42_POWERMARKS
#endif // DMCP_BUILD
}



void fnTimerReset(void) {
  #if !defined(TESTSUITE_BUILD)
  timerLastCalled = 0;
  #endif // !TESTSUITE_BUILD

  for(int i = 0; i < TMR_NUMBER; i++) {
    timer[i].state = TMR_UNUSED;
    timer[i].func = fnTimerDummy1;
    timer[i].param = 0;
  }

  #if defined(DMCP_BUILD)
    mutexRefreshTimer = false;
  #endif
  fnRebuildTimerRefresh();
}



void fnTimerConfig(uint8_t nr, void(*func)(uint16_t), uint16_t param) {
  if(nr < TMR_NUMBER) {
    timer[nr].func  = func;
    timer[nr].param = param;
    timer[nr].state = TMR_STOPPED;
  }
  fnRebuildTimerRefresh();
}



void fnTimerStart(uint8_t nr, uint16_t param, uint32_t time) {//time is in ms
  #if defined(DMCP_BUILD)
  uint32_t now = (uint32_t)sys_current_ms();                  //DMCP time is in ms
  #endif // DMCP_BUILD

  #if defined(PC_BUILD)
  gint64 now = g_get_monotonic_time();                        //PC time is in us
  #endif // PC_BUILD

  if(nr < TMR_NUMBER) {
    timer[nr].param = param;
    #if defined(DMCP_BUILD)
    timer[nr].timer_will_expire = (uint32_t)(now + time);     // is in ms
    #endif // DMCP_BUILD
    #if defined(PC_BUILD)
    timer[nr].timer_will_expire = (gint64)(now + time *1000); // time * 1000 is in us
    if(timer[nr].timer_will_expire < 0) {
      timer[nr].timer_will_expire = time *1000;
    }
    #endif // PC_BUILD
    timer[nr].state = TMR_RUNNING;
  }
  fnRebuildTimerRefresh();
}



void fnTimerStop(uint8_t nr) {
  if(nr < TMR_NUMBER && timer[nr].state != TMR_UNUSED) {
    timer[nr].state = TMR_STOPPED;
  }
  fnRebuildTimerRefresh();
}


void fnTimerExec(uint8_t nr) {
  if(nr < TMR_NUMBER && timer[nr].state == TMR_RUNNING) {
    timer[nr].state = TMR_COMPLETED;
    timer[nr].func(timer[nr].param);        // Callback to configured function
  }
}



void fnTimerDel(uint8_t nr) {
  if(nr < TMR_NUMBER) {
    timer[nr].state = TMR_UNUSED;
  }
  fnRebuildTimerRefresh();
}



uint16_t fnTimerGetParam(uint8_t nr) {
  uint16_t result = 0;

  if(nr < TMR_NUMBER) {
    result = timer[nr].param;
  }

  return result;
}



uint8_t fnTimerGetStatus(uint8_t nr) {
  uint8_t result = TMR_UNUSED;

  if(nr < TMR_NUMBER) {
    result = timer[nr].state;
  }

  return result;
}


// Timer application

#if !defined(TESTSUITE_BUILD)
static uint32_t _currentTime(void) {
    #if defined(DMCP_BUILD)
    tm_t timeInfo;
    dt_t dateInfo;

    rtc_read(&timeInfo, &dateInfo);
    return (uint32_t)timeInfo.hour * 3600000u +
           (uint32_t)timeInfo.min * 60000u +
           (uint32_t)timeInfo.sec * 1000u +
           (uint32_t)timeInfo.csec * 10u;
  #else // !DMCP_BUILD
    return (uint32_t)(g_get_real_time() % 86400000000uLL / 1000uLL);
  #endif // DMCP_BUILD
}

static void _antirewinder(uint32_t currTime) {
  if(currTime < timerStartTime) {
    timerValue += 86400000u - timerStartTime;
      if(timerTotalTime > 0) {
        timerTotalTime += 86400000u - timerStartTime;
      }
    timerStartTime = 0u;
  }
  else if(currTime >= timerStartTime + 3600000u) {
    timerValue += 3600000u;
      if(timerTotalTime > 0) {
        timerTotalTime += 3600000u;
      }
    timerStartTime += 3600000u;
  }
}

static uint32_t _getTimerValue(void) {
  uint32_t currTime = _currentTime();

  if(timerStartTime == TIMER_APP_STOPPED) {
    return timerValue;
  }

  _antirewinder(currTime);
  return currTime - timerStartTime + timerValue;
}
#endif // TESTSUITE_BUILD

//#if defined(PC_BUILD)
//  static gboolean _updateTimer(gpointer unusedData) {
//    if(calcMode != CM_TIMER) {
//      return FALSE;
//    }
//    fnUpdateTimerApp();
//    return timerStartTime != TIMER_APP_STOPPED;
//  }
//#endif // PC_BUILD

void fnItemTimerApp(uint16_t unusedButMandatoryParameter) {
#if !defined(TESTSUITE_BUILD) && !defined(SAVE_SPACE_DM42_20_TIMER)
  calcMode = CM_TIMER;
  rbr1stDigit = true;
  if(timerStartTime != TIMER_APP_STOPPED) {
    fnTimerStart(TO_TIMER_APP, TO_TIMER_APP, TIMER_APP_PERIOD);
      //#if defined(PC_BUILD)
      //  gdk_threads_add_timeout(100, _updateTimer, NULL);
      //#endif // PC_BUILD
  }
  #endif // !TESTSUITE_BUILD
}

void fnDecisecondTimerApp(uint16_t unusedButMandatoryParameter) {
  #if !defined(TESTSUITE_BUILD)
  timerCraAndDeciseconds ^= 0x80u;
  #endif // !TESTSUITE_BUILD
}

uint32_t remainingMsecCountdown = 0;


static void _realToUInt32(const real_t *re, enum rounding mode, uint32_t *value32, bool_t *overflow) {
  uint8_t bcd[76], sign;
  real_t real;
  longInteger_t lgInt;

  realToIntegralValue(re, &real, mode, &ctxtReal75);
  sign = real.bits & 0x80;
  realGetCoefficient(&real, bcd);

  longIntegerInit(lgInt);
  uInt32ToLongInteger(bcd[0], lgInt);

  for(int i=1; i<real.digits; i++) {
    longIntegerMultiplyUInt(lgInt, 10, lgInt);
    longIntegerAddUInt(lgInt, bcd[i], lgInt);
  }

  while(real.exponent > 0) {
    longIntegerMultiplyUInt(lgInt, 10, lgInt);
    real.exponent--;
  }

  *overflow = false;

  #if defined(OS32BIT) // 32 bit
    *value32 = (lgInt->_mp_size == 0 ? 0 : lgInt->_mp_d[0]);
    if(sign || lgInt->_mp_size > 1) {
      *overflow = true;
    }
  #else // 64 bit
    *value32 = (lgInt->_mp_size == 0 ? 0 : lgInt->_mp_d[0] & 0x00000000ffffffffULL);
    if(sign || lgInt->_mp_size > 1 || lgInt->_mp_d[0] & 0xffffffff00000000ULL) {
      *overflow = true;
    }
  #endif // OS32BIT

  longIntegerFree(lgInt);
}


bool_t inputHelper(uint16_t regist, uint32_t *val, bool_t *overflow) {
  real_t tmp;

  switch(getRegisterDataType(regist)) {
    case dtTime: {
      real34ToReal(REGISTER_REAL34_DATA(regist), &tmp);
      tmp.exponent += 3;
      _realToUInt32(&tmp, DEC_ROUND_DOWN, val, overflow);
      break;
    }
    case dtReal34: {
      real34ToReal(REGISTER_REAL34_DATA(regist), &tmp);
      realMultiply(&tmp, const_3600, &tmp, &ctxtReal39);
      tmp.exponent += 3;
      _realToUInt32(&tmp, DEC_ROUND_HALF_EVEN, val, overflow);
      break;
    }
    case dtLongInteger: {
      convertLongIntegerRegisterToReal(regist, &tmp, &ctxtReal39);
      realMultiply(&tmp, const_3600, &tmp, &ctxtReal39);
      tmp.exponent += 3;
      _realToUInt32(&tmp, DEC_ROUND_HALF_EVEN, val, overflow);
      break;
    }
    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "cannot recall %s to the stopwatch", getRegisterDataTypeName(regist, true, false));
        moreInfoOnError("In function inputHelper:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
  }
  return true;
}


void fnSetCountDownTimerApp(uint16_t regist) {
  #if !defined(TESTSUITE_BUILD)
    bool_t overflow;
    uint32_t input;
    if(!inputHelper(regist, &input, &overflow)) return;
    if(overflow){
      remainingMsecCountdown = 0;
    } else {
      fnResetTimerApp(NOPARAM);
      fnStopTimerApp();
      remainingMsecCountdown = input;
    }
  #endif // !TESTSUITE_BUILD
}


void fnResetTimerApp(uint16_t unusedButMandatoryParameter) {
  #if !defined(TESTSUITE_BUILD)
  timerValue = 0;
  timerTotalTime = 0;
  remainingMsecCountdown = 0;
  if(timerStartTime != TIMER_APP_STOPPED) {
    timerStartTime = _currentTime();
    fnTimerStart(TO_TIMER_APP, TO_TIMER_APP, TIMER_APP_PERIOD);
  }
  rbr1stDigit = true;
  #endif // !TESTSUITE_BUILD
}

void fnStartStopTimerApp(uint16_t unusedButMandatoryParameter) {
  #if !defined(TESTSUITE_BUILD) && !defined(SAVE_SPACE_DM42_20_TIMER)
  if(timerStartTime == TIMER_APP_STOPPED) {
    setSystemFlag(FLAG_RUNTIM);
    timerStartTime = _currentTime();
    fnTimerStart(TO_TIMER_APP, TO_TIMER_APP, TIMER_APP_PERIOD);
      //#if defined(PC_BUILD)
      //  gdk_threads_add_timeout(100, _updateTimer, NULL);
      //#endif // PC_BUILD
  }
  else {
    fnStopTimerApp();
  }
  rbr1stDigit = true;
#endif // TESTSUITE_BUILD
}

void fnStopTimerApp(void) {
  #if !defined(TESTSUITE_BUILD) && !defined(SAVE_SPACE_DM42_20_TIMER)
  if(timerStartTime != TIMER_APP_STOPPED) {
    const uint32_t msec = _currentTime();
    timerValue += msec - timerStartTime;
      if(timerTotalTime > 0) {
        timerTotalTime += msec - timerStartTime;
      }
    timerStartTime = TIMER_APP_STOPPED;
    fnTimerStop(TO_TIMER_APP);
  }
  clearSystemFlag(FLAG_RUNTIM);
  if(watchIconEnabled) {
    setSystemFlagChanged(SETTING_WATCHICON);
    watchIconEnabled = false;
  }
#endif // TESTSUITE_BUILD
}

void fnShowTimerApp(void) {
  #if !defined(TESTSUITE_BUILD) && !defined(SAVE_SPACE_DM42_20_TIMER)
  if(calcMode == CM_TIMER) {
    const uint32_t msec = _getTimerValue();
    clearRegisterLine(REGISTER_T, true, true);

    int64_t remainingMsec = 0;
    if(remainingMsecCountdown > 0) {
      remainingMsec = (int64_t)remainingMsecCountdown - (int64_t)msec; //allow it to run negative to be sure to catch it has run out, hence the int64
      clearRegisterLine(REGISTER_Z, true, true);
    }

    tmpString[0] = 0;

    if(timerTotalTime > 0) {
      const uint32_t tmsec = msec - timerValue + timerTotalTime;

      if(remainingMsecCountdown > 0) {
        remainingMsec = (int64_t)remainingMsecCountdown - (int64_t)tmsec;
      }

      if(timerCraAndDeciseconds & 0x80u) {
        sprintf(tmpString, "%2" PRIu32 ":%02" PRIu32 ":%02" PRIu32 ".%" PRIu32 STD_SUP_BOLD_T "  ", tmsec / 3600000u, tmsec % 3600000u / 60000u, tmsec % 60000u / 1000u, tmsec % 1000u / 100u);
      }
      else {
        sprintf(tmpString, "%2" PRIu32 ":%02" PRIu32 ":%02" PRIu32 STD_SUP_BOLD_T STD_SPACE_PUNCTUATION STD_SPACE_FIGURE "  ", tmsec / 3600000u, tmsec % 3600000u / 60000u, tmsec % 60000u / 1000u);
      }
    }

    if(timerCraAndDeciseconds & 0x80u) {
      sprintf(tmpString + stringByteLength(tmpString), "%2" PRIu32 ":%02" PRIu32 ":%02" PRIu32 ".%" PRIu32 " ", msec / 3600000u, msec % 3600000u / 60000u, msec % 60000u / 1000u, msec % 1000u / 100u);
    }
    else {
      sprintf(tmpString + stringByteLength(tmpString), "%2" PRIu32 ":%02" PRIu32 ":%02" PRIu32 STD_SPACE_PUNCTUATION STD_SPACE_FIGURE " ", msec / 3600000u, msec % 3600000u / 60000u, msec % 60000u / 1000u);
    }

    if(rbr1stDigit) {
      sprintf(tmpString + stringByteLength(tmpString), "[%02" PRIu32 "]", (uint32_t)(timerCraAndDeciseconds & 0x7fu));
    }
    else if(aimBuffer[AIM_BUFFER_LENGTH / 2] == 0) {
      sprintf(tmpString + stringByteLength(tmpString), "[" STD_CURSOR STD_SPACE_FIGURE "]");
    }
    else {
      sprintf(tmpString + stringByteLength(tmpString), "[%" PRId32 STD_CURSOR "]", (int32_t)(aimBuffer[AIM_BUFFER_LENGTH / 2] - '0'));
    }
    showString(tmpString, &numericFont, 1, Y_POSITION_OF_REGISTER_T_LINE, vmNormal, true, true);


    if(remainingMsecCountdown > 0) {
      if(remainingMsec > 0) {
        tmpString[0] = 0;
        if(timerCraAndDeciseconds & 0x80u) {
          sprintf(tmpString, "%2" PRIu32 ":%02" PRIu32 ":%02" PRIu32 ".%" PRIu32 STD_HOURGLASS_WH "  ", (uint32_t)remainingMsec / 3600000u, (uint32_t)remainingMsec % 3600000u / 60000u, (uint32_t)remainingMsec % 60000u / 1000u, (uint32_t)remainingMsec % 1000u / 100u);
        }
        else {
          sprintf(tmpString, "%2" PRIu32 ":%02" PRIu32 ":%02" PRIu32 STD_HOURGLASS_WH STD_SPACE_PUNCTUATION STD_SPACE_FIGURE "  ", (uint32_t)remainingMsec / 3600000u, (uint32_t)remainingMsec % 3600000u / 60000u, (uint32_t)remainingMsec % 60000u / 1000u);
        }
        showString(tmpString, &numericFont, 1, Y_POSITION_OF_REGISTER_Z_LINE, vmNormal, true, true);
      }
      else if(remainingMsec < 0) {
        remainingMsec = 0;
        remainingMsecCountdown = 0;
        fnBeep(NOPARAM);
      }
    }


      bool_t timerMenu = false;
      for(int i = 0; i < SOFTMENU_STACK_SIZE; i++) {
        if(softmenu[softmenuStack[i].softmenuId].menuItem == -MNU_TIMERF) {
          timerMenu = true;
          break;
        }
      }
      if(!timerMenu) {
        showSoftmenu(-MNU_TIMERF);
      }
      if(softmenu[softmenuStack[0].softmenuId].menuItem == -MNU_TIMERF) {
        calcModeNormalGui();
      }
    }
  #endif // !TESTSUITE_BUILD
}

void fnUpdateTimerApp(void) {
  #if !defined(TESTSUITE_BUILD) && !defined(SAVE_SPACE_DM42_20_TIMER)
  if(calcMode == CM_TIMER) {
    fnShowTimerApp();
    displayShiftAndTamBuffer();
      #if defined(DMCP_BUILD)
      refreshLcd();
      lcd_refresh();
    #else // !DMCP_BUILD
      refreshLcd(NULL);
    #endif // DMCP_BUILD
  }
  #endif // !TESTSUITE_BUILD
}

void fnRegAddTimerApp(uint16_t unusedButMandatoryParameter) {  //ENTER
printf("fnRegAddTimerApp\n");
  #if !defined(TESTSUITE_BUILD) && !defined(SAVE_SPACE_DM42_20_TIMER)
  if(rbr1stDigit) {
    real_t tmp;
    uInt32ToReal(_getTimerValue() / 100u, &tmp);
    tmp.exponent -= 1;
    reallocateRegister(timerCraAndDeciseconds & 0x7fu, dtTime, 0, amNone);
    realToReal34(&tmp, REGISTER_REAL34_DATA(timerCraAndDeciseconds & 0x7fu));
    fnUpTimerApp();
  }
  else if(aimBuffer[AIM_BUFFER_LENGTH / 2] == 0) {
    rbr1stDigit = true;
  }
  else {
    timerCraAndDeciseconds = (timerCraAndDeciseconds & 0x80u) + (uint8_t)(aimBuffer[AIM_BUFFER_LENGTH / 2] - '0');
    rbr1stDigit = true;
  }
  #endif // !TESTSUITE_BUILD
}

void fnRegAddLapTimerApp(uint16_t unusedButMandatoryParameter) {   //dot
printf("fnRegAddLapTimerApp\n");
  #if !defined(TESTSUITE_BUILD) && !defined(SAVE_SPACE_DM42_20_TIMER)
  const uint32_t msec = _getTimerValue();
  real_t tmp;

  uInt32ToReal(msec / 100u, &tmp);
  tmp.exponent -= 1;
  reallocateRegister(timerCraAndDeciseconds & 0x7fu, dtTime, 0, amNone);
  realToReal34(&tmp, REGISTER_REAL34_DATA(timerCraAndDeciseconds & 0x7fu));

  fnUpTimerApp();

  if(timerTotalTime > 0) {
    timerTotalTime += msec - timerValue;
  }
  else {
    timerTotalTime = msec;
  }
  timerValue = 0;
  if(timerStartTime != TIMER_APP_STOPPED) {
    timerStartTime = _currentTime();
    fnTimerStart(TO_TIMER_APP, TO_TIMER_APP, TIMER_APP_PERIOD);
  }
  #endif // !TESTSUITE_BUILD
}


void fnAddTimerApp(uint16_t unusedButMandatoryParameter) {           //Send TIM to STATS
  #if !defined(TESTSUITE_BUILD)
printf("fnAddTimerApp\n");
  real_t tmp;

  uInt32ToReal(_getTimerValue() / 100u, &tmp);
  tmp.exponent -= 1;
  realDivide(&tmp, const_3600, &tmp, &ctxtReal39);

  fnStatSum(0);
  if(lastErrorCode != ERROR_NONE) {
    liftStack();
    clearRegister(REGISTER_X);
    lastErrorCode = ERROR_NONE;
  }
  real34Add(REGISTER_REAL34_DATA(REGISTER_X), const34_1, REGISTER_REAL34_DATA(REGISTER_X));
  liftStack();
  realToReal34(&tmp, REGISTER_REAL34_DATA(REGISTER_X));
  fnSwapXY(NOPARAM);                                       //swapped around to be able to plot correctly
  fnSigmaAddRem(SIGMA_PLUS);

  refreshScreen(30);
  #endif // !TESTSUITE_BUILD
}


void fnAddLapTimerApp(uint16_t unusedButMandatoryParameter) {
printf("fnAddLapTimerApp\n");
  #if !defined(TESTSUITE_BUILD) && !defined(SAVE_SPACE_DM42_20_TIMER)
  const uint32_t msec = _getTimerValue();
  real_t tmp;

  uInt32ToReal(msec / 100u, &tmp);
  tmp.exponent -= 1;
//  reallocateRegister(timerCraAndDeciseconds & 0x7fu, dtTime, 0, amNone);
//  realToReal34(&tmp, REGISTER_REAL34_DATA(timerCraAndDeciseconds & 0x7fu));
  realDivide(&tmp, const_3600, &tmp, &ctxtReal39);

  fnStatSum(0);
  if(lastErrorCode != ERROR_NONE) {
    liftStack();
    clearRegister(REGISTER_X);
    lastErrorCode = ERROR_NONE;
  }
  real34Add(REGISTER_REAL34_DATA(REGISTER_X), const34_1, REGISTER_REAL34_DATA(REGISTER_X));
  liftStack();
  realToReal34(&tmp, REGISTER_REAL34_DATA(REGISTER_X));
  fnSwapXY(NOPARAM);                                       //swapped around to be able to plot correctly
  fnSigmaAddRem(SIGMA_PLUS);

//  fnUpTimerApp();

  if(timerTotalTime > 0) {
    timerTotalTime += msec - timerValue;
  }
  else {
    timerTotalTime = msec;
  }
  timerValue = 0;
  if(timerStartTime != TIMER_APP_STOPPED) {
    timerStartTime = _currentTime();
    fnTimerStart(TO_TIMER_APP, TO_TIMER_APP, TIMER_APP_PERIOD);
  }

  refreshScreen(31);
  #endif // !TESTSUITE_BUILD
}


void fnUpTimerApp(void) {
  #if !defined(TESTSUITE_BUILD) && !defined(SAVE_SPACE_DM42_20_TIMER)
  if((timerCraAndDeciseconds & 0x7fu) >= 99u) {
    timerCraAndDeciseconds &= 0x80u;
  }
  else {
    ++timerCraAndDeciseconds;
  }
  rbr1stDigit = true;
  #endif // !TESTSUITE_BUILD
}

void fnDownTimerApp(void) {
  #if !defined(TESTSUITE_BUILD)
  if((timerCraAndDeciseconds & 0x7fu) == 0u) {
    timerCraAndDeciseconds |= 99u;
  }
  else {
    --timerCraAndDeciseconds;
  }
  rbr1stDigit = true;
  #endif // !TESTSUITE_BUILD
}

void fnDigitKeyTimerApp(uint16_t digit) {
  #if !defined(TESTSUITE_BUILD) && !defined(SAVE_SPACE_DM42_20_TIMER)
  if(rbr1stDigit || aimBuffer[AIM_BUFFER_LENGTH / 2] == 0) {
    aimBuffer[AIM_BUFFER_LENGTH / 2    ] = digit + '0';
    aimBuffer[AIM_BUFFER_LENGTH / 2 + 1] = 0;
    rbr1stDigit = false;
  }
  else {
    timerCraAndDeciseconds = (timerCraAndDeciseconds & 0x80u) + (uint8_t)(aimBuffer[AIM_BUFFER_LENGTH / 2] - '0') * 10u + digit;
    rbr1stDigit = true;
  }
  #endif // !TESTSUITE_BUILD
}


void fnRecallTimerApp(uint16_t regist) {
  #if !defined(TESTSUITE_BUILD)
  bool_t overflow;
  uint32_t val;
  if(!inputHelper(regist, &val, &overflow)) return;
  if(overflow) {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "the %s does not fit to uint32_t", getRegisterDataTypeName(regist, true, false));
      moreInfoOnError("In function fnRecallTimerApp:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else {
    timerValue = val;
    if(timerStartTime != TIMER_APP_STOPPED) {
      timerStartTime = _currentTime();
      fnTimerStart(TO_TIMER_APP, TO_TIMER_APP, TIMER_APP_PERIOD);
    }
  }
  #endif // !TESTSUITE_BUILD
}

void fnBackspaceTimerApp(void) {
  #if !defined(TESTSUITE_BUILD)
  if(rbr1stDigit) {
    fnResetTimerApp(NOPARAM);
  }
  else if(aimBuffer[AIM_BUFFER_LENGTH / 2] == 0) {
    rbr1stDigit = true;
  }
  else {
    aimBuffer[AIM_BUFFER_LENGTH / 2] = 0;
  }
  #endif // !TESTSUITE_BUILD
}

void fnLeaveTimerApp(void) {
  #if !defined(TESTSUITE_BUILD)
  popSoftmenu();
  rbr1stDigit = true;
  calcMode = previousCalcMode;
  if(watchIconEnabled != (timerStartTime != TIMER_APP_STOPPED)) {
    setSystemFlagChanged(SETTING_WATCHICON);
    watchIconEnabled = !watchIconEnabled;
  }
  #endif // !TESTSUITE_BUILD
}

void fnPollTimerApp(void) { // poll every minute not to rewind the timer
  #if !defined(TESTSUITE_BUILD)
  if(calcMode != CM_TIMER && timerStartTime != TIMER_APP_STOPPED) {
    _antirewinder(_currentTime());
  }
  #endif // !TESTSUITE_BUILD
}
