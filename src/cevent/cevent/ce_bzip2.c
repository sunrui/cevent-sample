/**
 * bzip2
 *
 * Copyright (C) 2012-2013 honeysense.com
 * @author rui.sun 2013-8-13
 */

#include "ce_bzip2.h"
#include "ce_alloctor.h"
#include "ce_assert.h"

#include "bzlib.h"

char * ce_bzip2_compress(const char * src, uint32_t src_len, uint32_t * dest_len, int level)
{
    char * dest;
    uint32_t alloced;
    int ret;
    
    if (src == NULL || src_len <= 0 || dest_len == NULL) {
        ce_assert(0);
        return NULL;
    }
    
    alloced = ce_align_size((uint32_t)(src_len * 1.01) + 600, 16);
    dest = (char *)ce_malloc(alloced);
    if (dest == NULL) {
        ce_assert(0);
        return NULL;
    }
    
    *dest_len = alloced;
    
    ret = BZ2_bzBuffToBuffCompress(dest, dest_len, (char *)src, src_len, level, 0, 0);
    if (ret != BZ_OK) {
        ce_free(dest);
        return NULL;
    }
    
    return dest;
}

char * ce_bzip2_decompress(const char * src, uint32_t src_len, uint32_t dest_len)
{
    uint32_t dec_len;
    char * dest;
    int ret;
    
    ce_assert(src != NULL && src_len > 0 && dest_len > 0);
    dest = (char *)ce_malloc(ce_align_size(dest_len, 16));
    if (dest == NULL) {
        ce_assert(0);
        return NULL;
    }
    
    dec_len = dest_len;
    ret = BZ2_bzBuffToBuffDecompress(dest, (uint32_t *)&dec_len, (char *)src, src_len, 0, 0);
    if (ret != BZ_OK || dest_len != dec_len) {
        ce_assert(0);
        ce_free(dest);
        return NULL;
    }
    
    return dest;
}
