/* Corey Hemphill
 * hemphilc@oregonstate.edu
 * CS344_001 - Operating Systems I
 * Assignment 3 - smallsh
 * February 29, 2016
 * smallsh.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define ARG_MAX 513
#define ARG_LENGTH 25

/* Define Boolean Operators */
typedef int bool;
enum {
	false,
	true
};

/* Declare sigaction struct as a global */
struct sigaction act;

////////////////////* Function Declarations *////////////////////

void runCommandShell(char* command, bool* exitShell, int* status);
void freeArgs(char** args);

int main(int argc, char *argv[]) {
	// Trapping interupt signals ensures that the parent process does
	// not get killed. Since this is a shell, it is very important we
	// do this because we only want to kill off the child processes
	act.sa_handler = SIG_IGN;
	act.sa_flags = 0;
	sigfillset(&(act.sa_mask));
	sigaction(SIGINT, &act, NULL);

	char* commandLine = NULL; // Initialize the string for command line entry
	size_t size = 0; // This variable exists purely to satisfy getline()	
	bool exitShell = false; // Initialize exit bool to false
	int status; // Current status value

	// Keep going as long as the shell has not been exited
	while(!exitShell) {	
		printf(": ");	 // Print out prompts as necessary
		fflush(stdout);	 // Don't forget to courtesy flush 
		if(!(getline(&commandLine, &size, stdin))) {
			return 0;
		}
		// If the line contains only a new line char, run again
		if(strcmp(commandLine, "\n") == 0) {
			continue;
		}
		runCommandShell(commandLine, &exitShell, &status); // Run the shell
	}
	return 0;
}

////////////////////* Function Definitions *//////////////////////

/* Proccesses a string command line received from user input.
    param:  command    a pointer to a string of arguments.
    param:  exitShell  a pointer to a boolean for determining 
		       whether or not to exit the shell.
    param:  status     a pointer to an int for storing current
                       shell status.
    pre:    command is not NULL or empty.
    post:   the shell has executed a command from the user.
    ret:    none
*/
void runCommandShell(char* command, bool* exitShell, int* status) {
	// Allocate memory for an array of strings to hold command line arguments
	char** args = malloc(ARG_MAX * sizeof(char*));
	unsigned int i;
	// Allocate space for each pointer in the args pointer array
	for(i = 0; i < ARG_MAX; i++) {
		args[i] = malloc(ARG_LENGTH * sizeof(char));
	}
	unsigned int argCount = 0;	// Argument counter variable
	char* inputFile = NULL; 	// Input file name variable
	char* outputFile = NULL; 	// Output file name variable
	char* lineSeg; 			// Line segment variable for storing argument tokens
	bool isBackground = false; 	// Initialize background to false
	pid_t cpid; 			// Child process ID
	int file; 			// File descriptor variable
	lineSeg = strtok(command, " \n"); // Get the initial command segment
	// Break the command line into segments until it is NULL
	while(lineSeg != NULL) {
		// The process is background. Set isBackground boolean to true
		if(strcmp(lineSeg, "&") == 0) {
			isBackground = true;
		}
		// The process requires input redirection
		else if(strcmp(lineSeg, "<") == 0) {
			lineSeg = strtok(NULL, " \n"); // Get the segment following redirection
			inputFile = strdup(lineSeg); // Save it as the inputFile name
		}
		// The process requires output redirection
		else if(strcmp(lineSeg, ">") == 0) {
			lineSeg = strtok(NULL, " \n"); // Get the segment following redirection
			outputFile = strdup(lineSeg); // Save it as the outputFile name
		}
		// Store the command line argument into the args array
		else {
			strcpy(args[argCount], lineSeg); // Save the argument into the args array
			argCount += 1; // Count the number of arguments
		}
		lineSeg = strtok(NULL, " \n"); // Get the next segment
	}
	args[argCount] = NULL; // Set the pointer after the last argument to NULL
	if((strcmp(args[0], "#") == 0) || args[0] == NULL) {
		/* Its a comment--Don't do anything */
		return;
	}
	// If the first argument is "exit", cleanup and exit the shell
	if(strcmp(args[0], "exit") == 0) {
		*exitShell = true; // Set exit boolean to true in case we make it to main	
		free(inputFile); // Clear the inputFile
		free(outputFile); //Clear the outputFile
		free(lineSeg); // Clear the line segment
		freeArgs(args); // Cleanup
		exit(0); // Exit normally
	}
	// If the first argument is "cd", change the current working directory
	else if(strcmp(args[0], "cd") == 0) {
		// There is not a second argument, change directory to HOME
		if(args[1] == NULL) {
			chdir(getenv("HOME"));
		}
		// The second argument exists, change to the specified directory
		else {
			chdir(args[1]);
		}
	}
	// If the first argument is "status", print out the most recent status
	else if(strcmp(args[0], "status") == 0) {
		// If child process exited, display the exit value
		if(WIFEXITED(*status)) {
			printf("exit value %d\n", WEXITSTATUS(*status));
			fflush(stdout);
		}
		// If child process was terminated, display the signal value
		else {
			printf("terminated by signal %d\n", WTERMSIG(*status));
			fflush(stdout);
		}	
	}
	// The command is not built in, fork() and exec()
	else {
		cpid = fork(); // Fork process
		// I am the child process
		if (cpid == 0) {
			// If the process is not background, allow the interuption of foreground commands
			if(!isBackground) {
				act.sa_handler = SIG_DFL;
				act.sa_flags = 0;
				sigaction(SIGINT, &act, NULL);
			}
			// If input redirection is required, open the input file
			if(inputFile != NULL) {
				file = open(inputFile, O_RDONLY);
				// If opening failed, print error, cleanup, and return to main
				if(file < 0) {
					fprintf(stderr, "smallsh: cannot open %s for input\n", inputFile);
					fflush(stderr);
					freeArgs(args); // Cleanup
					exit(1);
				}
				// The file opened successfully, handle the input redirection
				else if(dup2(file, 0) < 0) {
					// If redirection fails, print error, cleanup, and return to main
					fprintf(stderr, "dup2 error\n");
					fflush(stderr);
					freeArgs(args); // Cleanup
					exit(1);
				}
				close(file); // Close the open file
			}
			// If input file not provided and background process, open "/dev/null"
			else if(isBackground) {
				file = open("/dev/null", O_RDONLY);	
				// If opening failed, print error, cleanup, and return to main
				if(file < 0) {
					fprintf(stderr, "open error\n");
					fflush(stderr);
					freeArgs(args); // Cleanup
					exit(1);
				}
				// The file opened successfully, handle the input redirection
				else if(dup2(file, 0) < 0) {
					// If redirection fails, print error, cleanup, and return to main
					fprintf(stderr, "dup2 error\n");
					fflush(stderr);
					freeArgs(args); // Cleanup
					exit(1);
				}
				close(file); // Close the open file
			}
			// If output redirection is required, open the output file
			else if(outputFile != NULL) {
				file = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0777);	
				// If opening failed, print error, cleanup, and return to main
				if(file < 0) {
					fprintf(stderr, "smallsh: cannot open %s for output\n", outputFile);
					fflush(stderr);
					freeArgs(args); // Cleanup
					exit(1);
				}
				// The file opened successfully, handle the output redirection
				if(dup2(file, 1) < 0) {
					// If redirection fails, print error, cleanup, and return to main
					fprintf(stderr, "dup2 error\n");
					fflush(stderr);
					freeArgs(args); // Cleanup
					exit(1);
				}
				close(file); // Close the open file
			}
			// Execute the command line
			if(execvp(args[0], args)) {
				// If execution fails, print error, cleanup, and return to main
				fprintf(stderr, "%s: no such file or directory\n", args[0]);
				fflush(stderr);
				freeArgs(args); // Cleanup
				exit(1);
			}
		}
		// I am the parent process
		else {
			// If the process is foreground, wait for it to die
			if(!isBackground) {
				waitpid(cpid, status, 0);
			}
			// Otherwise, the process is background, don't wait, just print the ID and move on
			else {
				printf("background pid is %d\n", cpid);
				fflush(stdout);
			}
		}
	}
	cpid = waitpid(-1, status, WNOHANG); // Check to see if any children have died
	while(cpid > 0) {
		// A background process has finished
		printf("background pid %d is done: ", cpid);
		fflush(stdout);
		// If child process exited, display the exit value
		if(WIFEXITED(*status)) {
			printf("exit value %d\n", WEXITSTATUS(*status));
			fflush(stdout);
		}
		// If child process was terminated, display the signal value
		else {
			printf("terminated by signal %d\n", WTERMSIG(*status));
			fflush(stdout);
		}
		cpid = waitpid(-1, status, WNOHANG); // Check again to see if anyone died
	}
	free(inputFile); // Clear the inputFile
	free(outputFile); //Clear the outputFile
	free(lineSeg); // Clear the line segment
	freeArgs(args); // Clear the args pointer array
}


/* Frees the args pointer array and each individual pointer within.
    param:  args    a pointer to an array of string
		    pointers.
    pre:    the pointer array is not NULL.
    post:   the pointer array memory has been freed.
    ret:    none
*/
void freeArgs(char** args) {
	unsigned int i;
	for(i = 0; i < ARG_MAX; i++) {
		free(args[i]); // Free each string pointer
	}
	free(args); // Free the array of pointers
}

