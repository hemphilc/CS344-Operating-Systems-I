/* Corey Hemphill
 * hemphilc@oregonstate.edu
 * CS344_001 - Operating Systems I
 * Assignment 4 - otp
 * March 14, 2016
 * keygen.c
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#define ARG_COUNT 2
#define ALPHA_COUNT 27

int main(int argc, char *argv[]) {
	// Check for the proper number of arguments
	if(argc < ARG_COUNT) {
		fprintf(stderr, "Usage: keygen <keylength>\n"); // Print proper usage statement
		fflush(stderr); // Courtesy flush
		exit(1); // Exit with error code
	}
	srand(time(0)); // Seed the random numbers
	long length = atoi(argv[1]); // Convert the char from argv to a long int
	// Generate key with capital letters and spaces
	const char alpha[ALPHA_COUNT] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
	long i;
	// Generate the key with the specified number of random characters and print them
	for(i = 0; i < length; i++) {
		printf("%c", alpha[rand() % ALPHA_COUNT]); // Print a random char
	}
	printf("\n"); // Print a new line character to so signify end of the key
	return 0;
}
