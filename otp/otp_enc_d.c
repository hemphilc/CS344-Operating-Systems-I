/* Corey Hemphill
 * hemphilc@oregonstate.edu
 * CS344_001 - Operating Systems I
 * Assignment 4 - otp
 * March 14, 2016
 * otp_enc_d.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define ARG_COUNT 2
#define BUFFER_MAX 2000000

/* Define Boolean Operators */
typedef int bool;
enum {
	false,
	true
};

////////////////////* Function Declarations *////////////////////

void error(const char* msg);
char intToChar(int i);
int charToInt(char c);

int main(int argc, char *argv[]) {
	int sockfd, newsockfd, portno, status;
	long i, n;
	int enable = 1; // Set opt value
	socklen_t clilen;
	pid_t pid;
	char buffer[BUFFER_MAX];
	bzero(buffer, sizeof(buffer));
	struct sockaddr_in serv_addr, cli_addr; // Declare necessary structs
	// Check for the proper number of arguments
	if(argc < ARG_COUNT) {
		fprintf(stderr,"Usage: opt_enc_d <listening_port>\n");
		fflush(stderr);
		exit(1);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	// Print error and exit if socket creation fails
	if(sockfd < 0) {
		error("Encryption Server: ERROR opening socket");
	}
	// Configure socket to allow reuse of socket address
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		error("setsockopt(SO_REUSEADDR) failed");
	}
	bzero((char *) &serv_addr, sizeof(serv_addr)); // Clean up address
	portno = atoi(argv[1]); // Convert port number to integer
	serv_addr.sin_family = AF_INET; // Set host address
	serv_addr.sin_addr.s_addr = INADDR_ANY; // Allow any client to connect
	serv_addr.sin_port = htons(portno); // Set port number
	// Bind the socket and address together
	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
              error("Encryption Server: ERROR on binding");
	}
	listen(sockfd, 5); // Listen for up to 5 new clients
	while(true) {
		clilen = sizeof(cli_addr);
		// Accept the socket connection request
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		// Check to make sure the acceptance occurred
		if(newsockfd < 0) {
			error("ERROR on accept");
		}
		pid = fork(); // Fork off a new child process
		// Check to make sure the fork occured
		if(pid < 0) {
			error("ERROR forking process");
		}
		// The fork was successful -- continue on with reading and decoding
		else if(pid == 0) {	
			bzero(buffer, sizeof(buffer)); // Zero the buffer
			long bytesLeft = sizeof(buffer); // Track the number of remaining bytes
			long bytesRead = 0; // Track how many bytes we have read so far
			int newLines = 0; // Track the number of new lines
			char *buffPtr = buffer; // Set pointer for tracking buffer position
			char *key; // The decryption key
			read(newsockfd, buffer, sizeof(buffer)); // Read from the socket
			// Check to see if the read was successful
			if(strcmp(buffer, "enc_crh") != 0) {
				char answer[] = "ERROR Invalid Code: Not Authorized";
				write(newsockfd, answer, sizeof(answer)); // Write to open socket
				_Exit(2); // Exit child with error value 2
			}
			// The read was successful -- continue
			else {
				char answer[] = "enc_d_crh"; // Return the authorized response
				write(newsockfd, answer, sizeof(answer)); // Write to open socket
			}
			bzero(buffer, sizeof(buffer)); // Zero the buffer
			while(true) {
				bytesRead = read(newsockfd, buffPtr, bytesLeft); // Read bytes
				// If no bytes were read, break the loop
				if(bytesLeft <= 0) {
					break;
				}
				// If somehow we got negative bytes, print error and exit
				else if(bytesRead < 0) {
					error("Encryption Server: ERROR reading from socket");
				}
				// Seperate the message from the key
				for(i = 0; i < bytesRead; i++) {
					// Check for new line character
					if(buffPtr[i] == '\n') {
						newLines += 1;
						// If found the first new line, its the key
						if(newLines == 1) {
							key = buffPtr + i + 1; // Save the key
						}
					}
					
				}
				// If we have found at least 2 new lines, then we are done reading
				if(newLines == 2) {
					break;
				}
				buffPtr += bytesRead; // Move buffer pointer the correct number of bytes
				bytesLeft -= bytesRead; // Decrement to track number of bytes are left
			}
			char message[BUFFER_MAX]; // Declare the  message buffer to max size
			bzero(message, sizeof(message)); // Zero the buffer
			strncpy(message, buffer, key - buffer); // Store the message segment only
			// Encode the message
			for(i = 0; message[i] != '\n'; i++) {
				n = (charToInt(message[i]) + charToInt(key[i])) % 27; // Convert using the key
				message[i] = intToChar(n); // Convert the integer value to a character
			}
			message[i] = '\0'; // Make sure the end of the message is null terminated
			long m = 0;
			m = write(newsockfd, message, strlen(message)); // Write the message to the socket
		}
		close(newsockfd); // Close the new socket we created
		// Wait for the child processes to die
		while(pid > 0) {
			pid = waitpid(-1, &status, WNOHANG);
		}
	}
	close(sockfd); // Close the original parent socket
	return 0;
}

////////////////////* Function Definitions *////////////////////

/* Prints a specified error message to the screen and exits.
    param:  msg    a pointer to a constant string error
                   message.
    pre:    the pointer to the string is not NULL.
    post:   the program has ended via error code 1.
    ret:    none
 */
void error(const char* msg) {
	perror(msg); // Print the specified error message
	exit(1); // Exit with error value 1
}


/* Converts a single integer value to a character.
    param:  i      the integer to be converted.
    pre:    none
    post:   the integer value has been converted.
    ret:    a character.
 */
char intToChar(int i) {
	// If int is equal to 26 decimal, its a space character
	if(i == 26) {
		return ' ';
	}
	// Add the value of 'A' to convert to a capital character
	else {
		return (i + 'A');
	}
}


/* Converts a single character value to an integer.
    param:  c      the character to be converted
    pre:    none
    post:   the character value has been converted.
    ret:    an integer.
 */
int charToInt(char c) {
	// If the character is a space, its value is 26 decimal
	if(c == ' ') {
		return 26;
	}
	// Subtract the value of 'A' to convert to integer decimal value
	else {
		return (c - 'A');
	}
}

