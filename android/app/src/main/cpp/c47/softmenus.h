// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file softmenus.h
 * List of softmenus and related functions.
 */
#if !defined(SOFTMENUS_H)
#define SOFTMENUS_H

int16_t currentMenu(void);
bool_t  isAlphaSubmenu(uint8_t n);



uint8_t *getNthString           (uint8_t *ptr, int16_t n); // Starting with string 0 (the 1st string is returned for n=0)
void     fnDynamicMenu          (uint16_t unusedButMandatoryParameter);

void     fnExitAllMenus         (uint16_t unusedButMandatoryParameter);
void     fnOpenMenu             (uint16_t menu);
int16_t  findMenu               (char *buffer);
void     fnGetMenu              (uint16_t unusedButMandatoryParameter);

/**
 * Displays one softkey.
 *
 * \param[in] label      Text to display
 * \param[in] xSoftkey   x location of softkey: from 0 (left) to 5 (right)
 * \param[in] ySoftKey   y location of softkey: from 0 (bottom) to 2 (top)
 * \param[in] videoMode  Video mode normal or reverse
 * \param[in] topLine    Draw a top line
 * \param[in] bottomLine Draw a bottom line
 */

bool_t maxfgLines             (int16_t y);
void   showKey                (const char *label, int16_t x1, int16_t x2, int16_t y1, int16_t y2, videoMode_t videoMode, bool_t topLine, bool_t bottomLine, int8_t showCb, int16_t showValue, const char *showText);

/**
 * Displays the current part of the displayed softmenu.
 */
bool_t isFunctionItemAMenu(int16_t item);
void   showSoftmenuCurrentPart(void);

/**
 * Displays a softmenu.
 *
 * \param[in] id ID of softmenu
 */
void    showSoftmenu           (int16_t id);
int16_t menu                   (uint8_t n);
void    changeToALPHA(void);
void    changeToHOME(void);
void    changeToPFN(void);

bool_t  setCurrentUserMenu     (int16_t item, char* funcParam);
bool_t  createHOME(void);
bool_t  createPFN(void);

/**
 * Pops a softmenu from the softmenu stack.
 */
void   popSoftmenu             (void);

/**
 * Remove a User menu from a softmenu stack.
 */
void removeUserMenuFromStack   (int16_t userMenuId);
void removeMenuFromStack       (int16_t userMenuId);
void extractPFNMenus           (void);

void   setCatalogLastPos       (void);
bool_t currentSoftmenuScrolls  (void);
bool_t isAlphabeticSoftmenu    (void);
bool_t isJMAlphaSoftmenu       (int16_t menuId);             //JM
bool_t isJMAlphaOnlySoftmenu   (void);                       //JM

void fnPseudoMenu              (uint16_t target);


int16_t mm(int16_t id);                                     //JM
extern TO_QSPI const int16_t menu_HOME[];                //JM

void   fnBaseMenu                (uint16_t unusedButMandatoryParameter);
char    *dynmenuGetLabel         (int16_t menuitem);
char    *dynmenuGetLabelWithDup  (int16_t menuitem, int16_t *dupNum);
void    fnDumpMenus              (uint16_t unusedButMandatoryParameter);  //JM
extern  bool_t BASE_OVERRIDEONCE;
#endif // !SOFTMENUS_H
