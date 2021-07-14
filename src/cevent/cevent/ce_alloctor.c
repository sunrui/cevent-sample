/**
 * memory provider
 *
 * Copyright (C) 2012-2013 honeysense.com
 * @author rui.sun 2013-1-28
 */
#include "ce_alloctor.h"
#include "ce_assert.h"

#include <stdlib.h>
#include <string.h>

#ifdef HAVE_JEMALLOC
#include "jemalloc.h"
#endif

#if defined(__GNUC__) && (__GNUC__ > 2)
#define CE_LIKELY(x)    (__builtin_expect(!!(x), 1))
#define CE_UNLIKELY(x)  (__builtin_expect(!!(x), 0))
#else
#define CE_LIKELY(x)    (x)
#define CE_UNLIKELY(x)  (x)
#endif

static ce_alloctor_t s_alloctor = {
#ifdef HAVE_JEMALLOC
	je_malloc, je_calloc, je_free
#else
	malloc, calloc, free
#endif
};

bool ce_custom_alloctor(ce_alloctor_t * allocator)
{
	if (allocator == NULL || allocator->malloc == NULL ||
        allocator->calloc == NULL || allocator->free == NULL) {
		ce_assert(0);
		return false;
	}
	
	memcpy(&s_alloctor, allocator, sizeof(ce_alloctor_t));

	return true;
}

void * ce_malloc(int size)
{
    if (CE_UNLIKELY(size <= 0)) {
		ce_assert(0);
		return NULL;
    }

	return s_alloctor.malloc(size);
}

void * ce_calloc(int count, int size)
{
	if (CE_UNLIKELY(count <= 0 || size <= 0)) {
		ce_assert(0);
		return NULL;
    }
	
	return s_alloctor.calloc(count, size);
}

void ce_free(void * ptr)
{
    if (CE_UNLIKELY(ptr == NULL)) {
		ce_assert(0);
		return;
    }

	s_alloctor.free(ptr);
}