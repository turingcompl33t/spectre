/*
 * cachetime.c
 * Observe cache performance improvements in user-level process. 
 *
 * Kyle Dotterrer
 * December, 2018 
 */

#include <stdio.h>
#include <emmintrin.h>
#include <x86intrin.h>

/* ----------------------------------------------------------------------------
	Constants
*/

#define ARRSIZE 10

/* ----------------------------------------------------------------------------
	Global Variables 
*/

uint8_t array[ARRSIZE*4096];

/* ----------------------------------------------------------------------------
	Main
*/

int main(void) {
	unsigned int junk = 0;

	register uint64_t time1, time2;
	volatile uint8_t *addr;
	int i;

	// init the array
	for (i = 0; i < ARRSIZE; i++) 
		array[i*4096] = 1;

	// flush the array from the cache 
	// really flushing the address from the cache, eh? 
	// or at least that is the API the clflush function provides 
	for (i = 0; i < ARRSIZE; i++) 
		_mm_clflush(&array[i*4096]);

	// touch some values, bring into cache?
	array[3*4096] = 100;
	array[7*4096] = 200;

	for (i = 0; i < ARRSIZE; i++) {
		addr = &array[i*4096];    // get the address of the array element
		time1 = __rdtscp(&junk);
		junk = *addr;
		time2 = __rdtscp(&junk) - time1;
		printf("access time for array[%d*4096]: %d CPU cycles\n", i, (int) time2);
	}

	exit(0); 
}
