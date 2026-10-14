// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/* ADDITIONAL C47 functions and routines */

/********************************************//**
 * \file keyboardTweak.h
 ***********************************************/

#if !defined(KEYBOARDTWEAK_H)
#define KEYBOARDTWEAK_H

//#if defined(DMCP_BUILD)
//extern uint32_t nextTimerRefresh;
//#endif // DMCP_BUILD

void     keyClick(uint8_t lengthMs);
void     powerMarkerMsF(uint8_t lengthMs, uint32_t f);

void     showAlphaModeonGui   (void);
void     resetShiftState      (void);
void     showShiftState       (void);
void     fnSHIFTf(uint16_t unusedButMandatoryParameter);
void     fnSHIFTg(uint16_t unusedButMandatoryParameter);
void     fnSHIFTfg(uint16_t unusedButMandatoryParameter);

#define  keypress_fff true
#define  keypress_long_f false
void     openHOMEorMyM(bool_t situation);
void     fg_processing_jm    (void);
int16_t  Check_Norm_Key_00_Assigned(int16_t  * result, int16_t tempkey);

bool_t   func_lookup         (int16_t  fn, int16_t itemShift, int16_t *funk);
void     execFnTimeout       (uint16_t key                    );                         //dr - delayed call of the primary function key
void     shiftCutoff         (uint16_t unusedButMandatoryParameter);     //dr - press shift three times within one second to call HOME timer
void     Check_MultiPresses  (int16_t  * result, int8_t key_no);
void     Setup_MultiPresses  (int16_t  result                 );
int16_t  nameFunction        (int16_t fn, bool_t shiftF, bool_t shiftG);   //JM LONGPRESS FN
void     resetKeytimers      (void);

uint16_t numlockReplacements(uint16_t id, int16_t item, bool_t NL, bool_t SHFT, bool_t GSHFT);
bool_t caseReplacements(uint8_t id, bool_t lowerCaseSelected, int16_t item, int16_t *itemOut);
bool_t keyReplacements(int16_t item, int16_t * item1, bool_t NL, bool_t SHFT, bool_t GSHFT);

#if defined(PC_BUILD)
void     btnFnPressed_StateMachine (GtkWidget *unused, gpointer data);
void     btnFnReleased_StateMachine(GtkWidget *unused, gpointer data);
#endif // PC_BUILD


#if defined(DMCP_BUILD)
#define BUFFER_FAIL     0                                   //vv dr - internal keyBuffer POC
#define BUFFER_SUCCESS  1
#define BUFFER_MASK (BUFFER_SIZE-1) // Klammern auf keinen Fall vergessen

typedef struct {
  uint8_t   data[BUFFER_SIZE];
  #if defined(BUFFER_CLICK_DETECTION)
  uint32_t  time[BUFFER_SIZE];
  #endif // BUFFER_CLICK_DETECTION
  uint8_t   read;   // zeigt auf das Feld mit dem ältesten Inhalt
  uint8_t   write;  // zeigt immer auf leeres Feld
} kb_buffer_t;

void     keyBuffer_pop        ();
uint8_t  inKeyBuffer          (uint8_t byte);
#if defined(BUFFER_CLICK_DETECTION)
#if defined(BUFFER_KEY_COUNT)
uint8_t  outKeyBuffer         (uint8_t *pByte, uint8_t *pByteCount, uint32_t *pTime, uint32_t *pTimeSpan_1, uint32_t *pTimeSpan_B);
#else // !BUFFER_KEY_COUNT
uint8_t  outKeyBuffer         (uint8_t *pByte, uint32_t *pTime, uint32_t *pTimeSpan_1, uint32_t *pTimeSpan_B);
#endif // BUFFER_KEY_COUNT
#else // !BUFFER_CLICK_DETECTION
#if defined(BUFFER_KEY_COUNT)
uint8_t  outKeyBuffer         (uint8_t *pByte, uint8_t *pByteCount);
#else // !BUFFER_KEY_COUNT
uint8_t  outKeyBuffer         (uint8_t *pByte);
#endif // BUFFER_KEY_COUNT
#endif // BUFFER_CLICK_DETECTION
uint8_t  outKeyBufferDoubleClick();
bool_t   fullKeyBuffer        ();
bool_t   emptyKeyBuffer       ();
void     clearKeyBuffer       ();


void     btnFnPressed_StateMachine (void *unused, void *data);
void     btnFnReleased_StateMachine(void *unused, void *data);
#endif // DMCP_BUILD

void fnCla                   (uint16_t unusedButMandatoryParameter);
void fnCln                   (uint16_t unusedButMandatoryParameter);

void fnT_ARROW(uint16_t command);

void     refreshModeGui       (void);

#endif // !KEYBOARDTWEAK_H
