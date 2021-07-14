/**
 * assert
 *
 * Copyright (C) 2012-2013 honeysense.com
 * @author rui.sun 2014-5-23
 */
#include "ce_assert.h"
#include "ce_log.h"

#include <stdio.h>
#include <assert.h>

static pfn_ce_assert s_assert;
static bool s_assert_enable =
#ifdef DEBUG
true
#else
false
#endif
;

void ce_assert_custom(pfn_ce_assert assert)
{
    s_assert = assert;
}

void ce_assert_enable(bool enable)
{
    s_assert_enable = enable;
}

void ce_assert0(bool r, const char * file, const char * function, int line)
{
    if (s_assert_enable && !r) {
        ce_log_write(ce_log_level_error, file, function, line, "assert");
        
        if (s_assert != NULL) {
            s_assert(file, function, line);
        }
        
        assert(0);
    }
}
