/**
 * crypt
 *
 * Copyright (C) 2012-2013 honeysense.com
 * @author rui.sun 2013-4-15
 */
#include "ce_crypt.h"
#include "ce_alloctor.h"
#include "ce_assert.h"

#include "openssl/crypto.h"
#include "openssl/bio.h"
#include "openssl/aes.h"
#include "openssl/rsa.h"
#include "openssl/pem.h"
#include "openssl/md5.h"
#include "openssl/evp.h"

#include <pthread.h>
#include <string.h>

static pthread_mutex_t * lock_array;
static int inited;

void ce_crypt_md5(unsigned char md[16], const char * buffer, int size)
{
    MD5_CTX  ctx;
    
    ce_assert(buffer != NULL && size > 0);
    
    MD5_Init(&ctx);
    MD5_Update(&ctx, buffer, size);
    MD5_Final(md, &ctx);
}

void lock_callback(int mode, int type, char * file, int line)
{
    (void)file;
    (void)line;
    
    if (mode & CRYPTO_LOCK) {
        pthread_mutex_lock(&(lock_array[type]));
    } else {
        pthread_mutex_unlock(&(lock_array[type]));
    }
}

unsigned long thread_id(void)
{
    return (unsigned long)pthread_self();
}

void ce_crypt_safe_init()
{
    if (!inited) {
        int i;
        
        lock_array = (pthread_mutex_t *)OPENSSL_malloc(CRYPTO_num_locks() * sizeof(pthread_mutex_t));
        for (i = 0; i < CRYPTO_num_locks(); i++) {
            pthread_mutex_init(&(lock_array[i]), NULL);
        }
        
        CRYPTO_set_id_callback((unsigned long (*)(void))thread_id);
        CRYPTO_set_locking_callback((void (*)(int, int, const char *, int))lock_callback);
        
        inited = 1;
    }
}

void ce_crypt_safe_uninit()
{
    if (inited) {
        int i;
        
        CRYPTO_set_locking_callback(NULL);
        for (i = 0; i < CRYPTO_num_locks(); i++) {
            pthread_mutex_destroy(&(lock_array[i]));
        }
        OPENSSL_free(lock_array);
        
        inited = 0;
    }
}

unsigned char * ce_crypt_aes(const unsigned char * in,
                             int in_size,
                             int * out_len,
                             const unsigned char key[16],
                             unsigned char vec[16],
                             bool enc)
{
    unsigned char * out;
    AES_KEY aes;
    int len;
    
    if ((in_size + 1) % AES_BLOCK_SIZE == 0) {
        len = in_size + 1;
    } else {
        len = ((in_size + 1) / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE;
    }
    
    enc == AES_ENCRYPT ?  AES_set_encrypt_key(key, 128, &aes) : AES_set_decrypt_key(key, 128, &aes);
    out = (unsigned char *)ce_malloc(len);
    if (out == NULL) {
        *out_len = 0;
        ce_assert(0);
        return NULL;
    }
    
    AES_cbc_encrypt(in, out, len, &aes, vec, enc ? AES_ENCRYPT : AES_DECRYPT);
    *out_len = len;
    
    return out;
}

bool ce_crypt_rsa(const char * key_buffer,
                  int key_size,
                  const char * in_buffer,
                  int in_size,
                  char ** out_buffer,
                  int * out_size,
                  bool enc)
{
    RSA * key;
    int r;
    
    if (key_buffer == NULL || key_size <= 0 ||
        in_buffer == NULL || in_size <= 0 ||
        out_buffer == NULL || out_size == NULL) {
        ce_assert(0);
        return false;
    }
    
    {
        BIO * bio;
        
        bio = BIO_new_mem_buf((void *)key_buffer, key_size);
        if (bio == NULL) {
            ce_assert(0);
            return false;
        }
        
        key = enc ? PEM_read_bio_RSA_PUBKEY(bio, NULL, NULL, NULL) : PEM_read_bio_RSAPrivateKey(bio, NULL, NULL, NULL);
        BIO_free(bio);
        
        if (key == NULL) {
            ce_assert(0);
            return false;
        }
    }
    
    *out_size = RSA_size(key);
    *out_buffer = (char *)ce_malloc(*out_size);
    if (*out_buffer == NULL) {
        *out_size = 0;
        ce_assert(0);
        return false;
    }
    
    r = enc ?
    RSA_public_encrypt(in_size, (const unsigned char *)in_buffer, (unsigned char *)*out_buffer, key, RSA_PKCS1_PADDING) :
    RSA_private_decrypt(in_size, (const unsigned char *)in_buffer, (unsigned char *)*out_buffer, key, RSA_PKCS1_PADDING);
    RSA_free(key);
    
    if (r == -1) {
        *out_size = 0;
        ce_freep(*out_buffer);
        ce_assert(0);
        return false;
    }
    
    *out_size = r;
    
    return true;
}
