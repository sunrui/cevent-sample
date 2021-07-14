/**
 * memory allocator
 *
 * Copyright (C) 2012-2013 honeysense.com
 * @author rui.sun 2013-1-28
 */
#ifndef CE_ALLOCTOR_H
#define CE_ALLOCTOR_H

#include <stddef.h>
#include <stdbool.h>

#define ce_align_size(x, ALIGNED_SIZE) ((x + (ALIGNED_SIZE - 1)) & ~(ALIGNED_SIZE - 1))
#define ce_freep(ptr) do { if (ptr != NULL) { ce_free(ptr); ptr = NULL; } } while (0)

typedef struct ce_alloctor {
	void * (* malloc)(size_t size);
	void * (* calloc)(size_t count, size_t size);
	void (* free)(void * ptr);
} ce_alloctor_t;

bool ce_custom_alloctor(ce_alloctor_t * allocator);
void * ce_malloc(int size);
void * ce_calloc(int count, int size);
void ce_free(void * ptr);

#endif