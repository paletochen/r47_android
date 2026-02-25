// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file config.h
 ***********************************************/
#if !defined(CONFIG_H)
#define CONFIG_H

enum {
    CFG_DFLT,
    CFG_CHINA, CFG_EUROPE, CFG_INDIA, CFG_JAPAN, CFG_UK, CFG_USA
};

bool_t   isConfigCommon        (uint16_t id);
void     configCommon          (uint16_t idx);
void     showSoftmenu          (int16_t id);  //JM repeat here to make DMCP compile
void     fnShowVersion         (uint16_t option);
extern const enum rounding roundingModeTable[7];

/********************************************//**
 * \brief Sets the calc to meet Chinese standards
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 ***********************************************/
void     fnConfigChina         (uint16_t unusedButMandatoryParameter);

/********************************************//**
 * \brief Sets the calc to meet European standards
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 ***********************************************/
void     fnConfigEurope        (uint16_t unusedButMandatoryParameter);

/********************************************//**
 * \brief Sets the calc to meet Indian standards
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 ***********************************************/
void     fnConfigIndia         (uint16_t unusedButMandatoryParameter);

/********************************************//**
 * \brief Sets the calc to meet Japanese standards
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 ***********************************************/
void     fnConfigJapan         (uint16_t unusedButMandatoryParameter);

/********************************************//**
 * \brief Sets the calc to meet English standards
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 ***********************************************/
void     fnConfigUk            (uint16_t unusedButMandatoryParameter);

/********************************************//**
 * \brief Sets the calc to meet American standards
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 ***********************************************/
void     fnConfigUsa                (uint16_t unusedButMandatoryParameter);

void     fnSetHP35                  (uint16_t unusedButMandatoryParameter);
void     fnSetJM                    (uint16_t unusedButMandatoryParameter);
void     fnSetRJ                    (uint16_t unusedButMandatoryParameter);
void     fnSetC47                   (uint16_t unusedButMandatoryParameter);
void     fnClrMod                   (uint16_t unusedButMandatoryParameter);

void     fnMenuGapL                 (uint16_t unusedButMandatoryParameter);
void     fnMenuGapRX                (uint16_t unusedButMandatoryParameter);
void     fnMenuGapR                 (uint16_t unusedButMandatoryParameter);
void     fnSetGapChar               (uint16_t charParam);
void     fnSettingsDispFormatGrpL   (uint16_t param);
void     fnSettingsDispFormatGrp1Lo (uint16_t param);
void     fnSettingsDispFormatGrp1L  (uint16_t param);
void     fnSettingsDispFormatGrpR   (uint16_t param);





/********************************************//**
 * \brief Sets the calc to back to default standards
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 ***********************************************/
void     fnConfigDefault       (uint16_t unusedButMandatoryParameter);

/********************************************//**
 * \brief Sets the integer mode
 *
 * \param[in] mode uint16_t Integer mode
 ***********************************************/
void     fnIntegerMode         (uint16_t mode);

/********************************************//**
 * \brief Displays credits to the brave men who
 * made this project work.
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 ***********************************************/
void     fnWho                 (uint16_t unusedButMandatoryParameter);

/********************************************//**
 * \brief Displays the version of this software
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 ***********************************************/
void     fnVersion             (uint16_t unusedButMandatoryParameter);

/********************************************//**
 * \brief Sets X to the amount of free RAM
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 ***********************************************/
void     fnFreeMemory          (uint16_t unusedButMandatoryParameter);

/********************************************//**
 * \brief Sets X to the value of the rounding mode
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 ***********************************************/
void     fnGetDmx              (uint16_t unusedButMandatoryParameter);

/********************************************//**
 * \brief Sets X to the value of the rounding mode
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 ***********************************************/
void     fnGetRoundingMode     (uint16_t unusedButMandatoryParameter);

/********************************************//**
 * \brief Sets the rounding mode
 *
 * \param[in] RM uint16_t
 ***********************************************/
void     fnSetRoundingMode     (uint16_t RM);

/********************************************//**
 * \brief Sets X to the value of the integer mode
 ***********************************************/
void     fnGetIntegerSignMode  (uint16_t unusedButMandatoryParameter);

/********************************************//**
 * \brief Gets the word size
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 ***********************************************/
void     fnGetWordSize         (uint16_t unusedButMandatoryParameter);

/********************************************//**
 * \brief Sets the word size
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 ***********************************************/
void     fnSetWordSize         (uint16_t WS);

/********************************************//**
 * \brief Sets X to the amount of free flash memory
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 ***********************************************/
void     fnFreeFlashMemory     (uint16_t unusedButMandatoryParameter);

/********************************************//**
 * \brief Sets X to the battery voltage
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 ***********************************************/
void     fnBatteryVoltage      (uint16_t unusedButMandatoryParameter);

/********************************************//**
 * \brief Retrives the amount of free flash memory
 *
 * \param void
 * \return uint32_t Number of bytes free
 ***********************************************/
uint32_t getFreeFlash          (void);

/********************************************//**
 * \brief Sets X to the number of signficant digits
 * rounding after each operation
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 ***********************************************/
void     fnGetSignificantDigits(uint16_t unusedButMandatoryParameter);
void     fnGetFractionDigits(uint16_t unusedButMandatoryParameter);

/********************************************//**
 * \brief Sets the number of signficant digits
 * rounding after each operation to X
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 ***********************************************/
void     fnSetBaseNr(uint16_t S);
void     fnSetSignificantDigits(uint16_t S);
void     fnSetFractionDigits(uint16_t S);

/********************************************//**
 * \brief Sets the rounding mode
 *
 * \param[in] RM uint16_t Rounding mode
 ***********************************************/
void     fnRoundingMode        (uint16_t RM);

/********************************************//**
 * \brief Sets the angular mode
 *
 * \param[in] am uint16_t Angular mode
 ***********************************************/
void     fnAngularMode         (uint16_t angularMode);

/********************************************//**
 * \brief Activates fraction display and toggles which of improper (like 3/2) or mixed (like 1 1/2) fractions is preferred.
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 ***********************************************/
void     fnFractionType        (uint16_t unusedButMandatoryParameter);


#define  loadAutoSav           true
#define  doNotLoadAutoSav      false
void     defaultStatusBar      (void);
void     resetOtherConfigurationStuff(bool_t allowUserKeys);
void     setLongPressFg        (int calcModel0, int16_t menuItem);
void     fnReset               (uint16_t confirmation);
void     doFnReset             (uint16_t confirmation, bool_t autoSav);

void     restoreStats          (void);
void     setConfirmationMode   (void (*func)(uint16_t));
void     fnConfirmationYes     (uint16_t unusedButMandatoryParameter);
void     fnConfirmationNo      (uint16_t unusedButMandatoryParameter);
uint16_t getConfirmationTiId   (void);

void     dmcpResetAutoOff      (void);
int      updateVbatIntegrated  (bool_t minutePulse);
void     checkBattery          (void);
void     fnClAll               (uint16_t confirmation);
void     backToSystem          (uint16_t confirmation);
void     runDMCPmenu           (uint16_t confirmation);
void     activateUSBdisk       (uint16_t confirmation);
void     fnRange               (uint16_t R);
void     fnGetRange            (uint16_t unusedButMandatoryParameter);
void     fnHide                (uint16_t H);
void     fnGetHide             (uint16_t unusedButMandatoryParameter);
void     fnGetLastErr          (uint16_t unusedButMandatoryParameter);
void     fnKeysManagement      (uint16_t choice);
void     initSimEqMatABX       (void);
void     fnResetTVM            (uint16_t unusedButMandatoryParameter);



#endif // !CONFIG_H
