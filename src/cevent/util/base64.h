//
// base64.h
//
// Created by 孙瑞 on 15/1/9.
// Copyright (c) 2015年 honeysense.com. All rights reserved.
//

#ifndef BASE64_H
#define BASE64_H

#include <stdio.h>

typedef	unsigned char u_char;
typedef intptr_t ngx_int_t;
typedef uintptr_t ngx_uint_t;

typedef struct {
 size_t len;
 u_char *data;
} ngx_str_t;

#define NGX_OK 0
#define NGX_ERROR -1

#define ngx_base64_encoded_length(len) (((len + 2) / 3) * 4)
#define ngx_base64_decoded_length(len) (((len + 3) / 4) * 3)

void
ngx_encode_base64(ngx_str_t *dst, ngx_str_t *src);

void
ngx_encode_base64url(ngx_str_t *dst, ngx_str_t *src);

ngx_int_t
ngx_decode_base64(ngx_str_t *dst, ngx_str_t *src);

ngx_int_t
ngx_decode_base64url(ngx_str_t *dst, ngx_str_t *src);


#endif
