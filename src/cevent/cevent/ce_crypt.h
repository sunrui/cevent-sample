/**
 * crypt
 *
 * Copyright (C) 2012-2013 honeysense.com
 * @author rui.sun 2013-4-15
 */
#ifndef CE_CRYPT_H
#define CE_CRYPT_H

#include <stdbool.h>

void ce_crypt_safe_init();
void ce_crypt_safe_uninit();

void ce_crypt_md5(unsigned char md5[16], const char * buffer, int size);

unsigned char * ce_crypt_aes(const unsigned char * in,
	int in_size,
	int * out_len,
	const unsigned char key[16], 
	unsigned char vec[16],
	bool enc);

bool ce_crypt_rsa(const char * key_buffer,
	int key_size,
	const char * in_buffer,
	int in_size,
	char ** out_buffer,
	int * out_size,
	bool enc);

#endif