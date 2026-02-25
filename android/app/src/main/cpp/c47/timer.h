// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file timer.h
 */
#if !defined(TIMER_H)
#define TIMER_H

uint32_t getUptimeMs           (void);
void     fnTicks               (uint16_t unusedButMandatoryParameter);
void     fnLastT               (uint16_t unusedButMandatoryParameter);
void     LastOpTimerReStart    (uint16_t func);
void     LastOpTimerLap        (uint16_t func);
void     fnItemTimerApp        (uint16_t unusedButMandatoryParameter); // STOPW
void     fnStopTimerApp        (void);                                 // from Off
void     fnShowTimerApp        (void);                                
void     fnUpdateTimerApp      (void);                                
void     fnUpTimerApp          (void);                                
void     fnDownTimerApp        (void);                                
void     fnDigitKeyTimerApp    (uint16_t digit);                      

void     fnAddTimerApp         (uint16_t unusedButMandatoryParameter); // F1 TIM → Σ
void     fnAddLapTimerApp      (uint16_t unusedButMandatoryParameter); // F2 LAP → Σ
void     fnRegAddTimerApp      (uint16_t unusedButMandatoryParameter); // F3 TIM → R
void     fnRegAddLapTimerApp   (uint16_t unusedButMandatoryParameter); // F4 LAP → R
void     fnStartStopTimerApp   (uint16_t unusedButMandatoryParameter); // F5 R/S
void     fnResetTimerApp       (uint16_t unusedButMandatoryParameter); // F6 RESET

void     fnRecallTimerApp      (uint16_t regist);                      // fF3 RCL⏱
void     fnSetCountDownTimerApp(uint16_t unusedButMandatoryParameter); // fF4 RCL⧖
void     fnDecisecondTimerApp  (uint16_t unusedButMandatoryParameter); // fF5 0.1s

void     fnBackspaceTimerApp  (void);
void     fnLeaveTimerApp      (void);
void     fnPollTimerApp       (void);


#if defined(PC_BUILD)
  gboolean refreshTimer         (gpointer data);
#endif

#if defined(DMCP_BUILD)
  void     refreshTimer         (void);
#endif


void     fnTimerReset         (void);
void     fnTimerDummy1        (uint16_t param);
void     fnTimerEndOfActivity (uint16_t param);
void     fnTimerConfig        (uint8_t nr, void(*func)(uint16_t), uint16_t param);
void     fnTimerStart         (uint8_t nr, uint16_t param, uint32_t time);      // Start Timer, 0..n-1
void     fnTimerStop          (uint8_t nr);                                     // Stop Timer, 0..n-1
void     fnTimerExec          (uint8_t nr);                                     // Execute Timer, 0..n-1
void     fnTimerDel           (uint8_t nr);                                     // Delete Timer, 0..n-1
uint16_t fnTimerGetParam      (uint8_t nr);
uint8_t  fnTimerGetStatus     (uint8_t nr);



/********************************************//**
 * \typedef timer_t
 * \brief Structure keeping the information for one timer
 ***********************************************/
#define TMR_UNUSED    0
#define TMR_STOPPED   1
#define TMR_RUNNING   2
#define TMR_COMPLETED 3



typedef struct {
  void     (*func)(uint16_t);   ///< Function called to execute the timer
  uint16_t param;               ///< 1st parameter to the above
  #if !defined(PC_BUILD)
    uint32_t timer_will_expire; ///<
  #else // PC_BUILD
    gint64   timer_will_expire; ///<
  #endif // !PC_BUILD
  uint8_t  state;               ///<
} kb_timer_t;
#endif // TIMER_H
