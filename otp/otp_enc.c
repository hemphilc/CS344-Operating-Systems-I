/* Corey Hemphill
 * hemphilc@oregonstate.edu
 * CS344_001 - Operating Systems I
 * Assignment 4 - otp
 * March 14, 2016
 * otp_enc.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

#define ARG_COUNT 4
#define BUFFER_MAX 2000000

/* Define Boolean Operators */
typedef int bool;
enum {
        false,
        true
};

////////////////////* Function Declarations *////////////////////

void error(const char* msg);
long getLength(char* fileName);
void transmit(char* fileName, int socket, long fileLength);

int main(int argc, char *argv[]) {
	int sockfd, portno;
	long n;
	int enable = 1; // Set opt value
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[BUFFER_MAX];
	bzero(buffer, sizeof(buffer));
        // Check for the proper number of arguments
	if(argc < ARG_COUNT) {
                fprintf(stderr,"Usage: opt_dec <plaintext> <key> <port>\n");
                fflush(stderr);
                exit(1);
        }
	sockfd = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	// Print error and exit if socket creation fails 
	if(sockfd < 0) {
		error("ERROR opening socket");
	}
	server = gethostbyname("localhost"); // Get server host name "localhost"
	// Check to make sure server name exists
	if(server == NULL) {
		error("ERROR no such host");
	}
	// Configure socket to allow reuse of socket address
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		error("setsockopt(SO_REUSEADDR) failed");
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[3]); // Retrieve port number from argv array
	serv_addr.sin_family = AF_INET; // Set host address
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno); // Set port number
	// Attempt to connect to the specified server socket
	if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		error("ERROR cannot connect");
	}
	char authorization[] = "enc_crh";
	write(sockfd, authorization, sizeof(authorization)); // Send authorization
	read(sockfd, buffer, sizeof(buffer)); // Read response into the buffer
	// Check to see if the transmission was successful
	if(strcmp(buffer, "enc_d_crh") != 0) {
		fprintf(stderr, "Error: could not contact otp_dec_d on port %d\n", portno);
		fflush(stderr);
		exit(2);
	}
	long msgFileLength = getLength(argv[1]); // Get the length of the message file
	long keyLength = getLength(argv[2]); // Get the length of the key
	// Check to make sure that the message file and key are at least the same size
	if(msgFileLength > keyLength) {
		fprintf(stderr, "Error: key '%s' is too short\n", argv[2]);
		fflush(stderr);
		exit(1);
	}
	// Check for invalid/bad characters
	long vd = open(argv[1], 'r');
	// Read a single byte from the buffer
	while(read(vd, buffer, 1) != 0) {
		// Is it a space?
		if(buffer[0] != ' ') {
			// Is it a capitalized character?
			if(buffer[0] < 'A' || buffer[0] > 'Z') {
				// Is it a new line character?
				if(buffer[0] != '\n') {
					// The input must be bad -- print error and exit
					fprintf(stderr, "otp_enc_d error: '%s' contains bad characters\n", argv[1]);
					fflush(stderr);
					exit(1);
				}
			}
		}	
	}
	transmit(argv[1], sockfd, msgFileLength); // Transmit the plaintext file
	transmit(argv[2], sockfd, keyLength); // Transmit the key	
	n = read(sockfd, buffer, sizeof(buffer)); // Read from the open socket
	printf("%s\n", buffer); // Print the decoded message
	close(sockfd); // Close the socket
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


/* Returns the length of a specified file in bytes.
    param:  fileName  a pointer to a string containing a
                      filename.
    pre:    the pointer to the string is not NULL.
    post:   none
    ret:    a long integer.
*/
long getLength(char* fileName) {
	FILE* fptr = fopen(fileName, "r"); // Open the specified file
	fpos_t pos;
	long length;
	fgetpos(fptr, &pos); // Save the current position of the file pointer
	// Find the end of the file and determine the length in bytes
	if(fseek(fptr, 0, SEEK_END) || (length = ftell(fptr)) == -1) {
		perror("ERROR unable to get length"); // Print an error if unable to get length
	}
	fsetpos(fptr, &pos); // Restore the file pointer to its original position
	return length;
}


/* Transmits the contents of a specified file through an open socket.
    param:  fileName    a pointer to a string containing a
                        filename.
    param:  socket      a socket file descriptor.
    param:  fileLength  the length of the file being
			transmitted.
    pre:    the fileName pointer is not NULL.
    pre:    the socket file descriptor is not < 0.
    pre:    fileLength is not <= 0.
    post:   a file has been transmitted through the socket.
    ret:    none
*/
void transmit(char* fileName, int socket, long fileLength) {
	long fd = open(fileName, 'r'); // Open the specified file read only
	char buffer[BUFFER_MAX];
	bzero(buffer, sizeof(buffer)); // Zero the buffer
	long bytesRead = 0; // Initialize variable for tracking bytes read
	long bytesWritten = 0; // Initialize variable for tracking bytes written
	// Read from the socket until there is nothing else to read
	while(fileLength > 0) {
		bytesRead = read(fd, buffer, sizeof(buffer)); // Read in from socket
		// If there was nothing to read, end the loop
		if(bytesRead == 0) {
			break;
		}
		// If for some reason the number of bytes read is negative, error and exit
		if(bytesRead < 0) {	
			error("ERROR reading file");
		}
		fileLength -= bytesRead; // Deduct number of bytes read from total file length
	}
	char *buffPtr = buffer; // Initialize pointer to track buffer position
	// Write back into the socket until bytesRead variable is zeroed
	while(bytesRead > 0) {
		bytesWritten = write(socket, buffPtr, bytesRead); // Write to the socket
		// If for some reason the number of bytes written is negative, error and exit
		if(bytesWritten < 0) {
			error("ERROR writing to socket");
		}
		bytesRead -= bytesWritten; // Decrement the number of bytes read in
		buffPtr += bytesWritten; // Move the buffer pointer by number of bytes written
	}
	return; // We are done transmitting -- return to main program
}

