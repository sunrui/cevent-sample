/**
 * assert
 *
 * Copyright (C) 2012-2013 honeysense.com
 * @author rui.sun 2014-5-23
 */
#ifndef CE_ASSERT_H
#define CE_ASSERT_H

#include <stdbool.h>

#ifdef DEBUG
#define ce_assert(r) ce_assert0(r, __FILE__, __FUNCTION__, __LINE__)
#else
#define ce_assert(r)
#endif

typedef void (* pfn_ce_assert)(const char * file, const char * function, int line);

void ce_assert_custom(pfn_ce_assert assert);
void ce_assert_enable(bool enable);
void ce_assert0(bool r, const char * file, const char * function, int line);

#endif
