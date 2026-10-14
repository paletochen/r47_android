// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file assign.h
 */
#if !defined(ASSIGN_H)
  #define ASSIGN_H

  void fnAssign                   (uint16_t mode);

  void fnDeleteMenu               (uint16_t id);

  void updateAssignTamBuffer      (void);

  void _assignItem                (userMenuItem_t *menuItem);
  void assignToMyMenu             (uint16_t position);
  void assignToMyAlpha            (uint16_t position);
  void assignToUserMenu           (uint16_t position);
  void assignToKey                (const char *data);

  void initUserKeyArgument        (void);
  void setUserKeyArgument         (uint16_t position, const char *name);
  void createMenu                 (const char *name);

  void assignEnterAlpha           (void);
  void assignLeaveAlpha           (void);
  void assignGetName1             (void);
  void assignGetName2             (void);

  /**
   * Remove assignments for user items
   *
   * \param[in] item         Category of user items to be removed:
   *                          - ITM_XEQ for user programs,
   *                          - ITM_RCL for User variables,
   *                          - -MNU_DYNAMIC for user defined menus
   *            userItemName Pointer to the item name to be removed
   *                         If the name is an empty string, all assigned items within the category will be removed
   *                         ex: ITM_XEQ, "Prog1" will remove all assignments of the program "Prog1"
   *                             ITM_XEQ, ""      will remove all assignments of all programs
   */
  void removeUserItemAssignments  (int16_t item, char *userItemName);

  /**
   * Delete all user menus and user menus assignments
   *
   * \param[in] confirmation Current status of the confirmation of deleting all user menus
   */
  void fnDeleteUserMenus          (uint16_t confirmation);

  /**
   * Clear all user menus, removong all assignements to these menus
   *
   * \param[in] confirmation Current status of the confirmation of clearing all user menus
   */
  void fnClearUserMenus           (uint16_t confirmation);

#endif // !ASSIGN_H
