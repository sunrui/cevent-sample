/* Fast hashing routine for a long.
 (C) 2002 William Lee Irwin III, IBM */
/*
 * Knuth recommends primes in approximately golden ratio to the maximum
 * integer representable by a machine word for multiplicative hashing.
 * Chuck Lever verified the effectiveness of this technique:
 * http://www.citi.umich.edu/techreports/reports/citi-tr-00-1.pdf
 *
 * These primes are chosen to be bit-sparse, that is operations on
 * them can use shifts and additions instead of multiplications for
 * machines where multiplications are slow.
 */

#ifndef XHASH_H
#define XHASH_H

unsigned long hash_long(unsigned long val, unsigned int bits);
unsigned long hash_ptr(void *ptr, unsigned int bits);

#endif