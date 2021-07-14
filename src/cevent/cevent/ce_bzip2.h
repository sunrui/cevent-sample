/**
 * bzip2
 *
 * Copyright (C) 2012-2013 honeysense.com
 * @author rui.sun 2013-8-13
 */

#ifndef CE_BZIP2_H
#define CE_BZIP2_H

#include <stdint.h>

char * ce_bzip2_compress(const char * src, uint32_t src_len, uint32_t * dest_len, int level);
char * ce_bzip2_decompress(const char * src, uint32_t src_len, uint32_t dest_len);

#endif