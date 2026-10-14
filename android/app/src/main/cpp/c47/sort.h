// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file sort.h
 * Comparing 2 strings, sorting strings.
 */

#if !defined(SORT_H)
  #define SORT_H

  int32_t compareChar  (const char *char1, const char *char2);
  int32_t compareString(const char *stra, const char *strb, int32_t comparisonType);
#endif // !SORT_H
