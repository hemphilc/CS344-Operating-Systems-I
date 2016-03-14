/* Corey Hemphill 		   */
/* hemphilc@oregonstate.edu 	   */
/* CS344_001 - Operation Systems I */
/* Assignment 2 - adventure   	   */
/* February 7, 2016		   */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define NUM_ROOMS 7
#define NUM_NAMES 10
#define CONN_MAX 6
#define CONN_MIN 3
#define NAME_LENGTH 11
#define DIR_BUFFER_LENGTH 21

/* Define Unsigned Int as Unt */
typedef unsigned int Unt;

/* Define Boolean Operators */
typedef int bool;
enum {
	false,
	true
};

/* Define Room Types */
enum roomType {
	START_ROOM,
	MID_ROOM,
	END_ROOM
};

/* Define Room Names */
const char* roomNames[NUM_NAMES] = {
	"Alderaan",
	"Bespin",
	"Coruscant",
	"Dagobah",
	"Endor",
	"Geonosis",
	"Hoth",
	"Jakku",
	"Naboo",
	"Tatooine"
};

/* Define Room Struct */
typedef struct Room Room;
struct Room {
	const char* roomName;
	enum roomType type;	
	struct Room* conns[CONN_MAX];
	Unt connCap;
	Unt connCnt;
};
Room rooms[NUM_ROOMS];		// Declare array of Room structs

////////////////////* Function Declarations *////////////////////

void playGame(const char* startRoom);
void createRooms(Room roomArray[]);
void writeRooms(Room roomArray[]);
void stripChar(char* str, char c);
bool connectRooms(Unt r1, Unt r2, Room roomArray[]);
bool isConnected(Unt r1, Unt r2, Room roomArray[]);
void getDirectory(char* name);


int main(void) {
	srand(time(0)); 		// Seed random numbers
	createRooms(rooms);		// Generate Rooms
	writeRooms(rooms);		// Write Rooms to file
	playGame(rooms[0].roomName);	// Play the game
	return 0;
}


////////////////////* Function Definitions *//////////////////////

/* Initializes the main game loop.
  	param:  an array of Room structs.
	pre:	boredom.
  	post:	happiness.
 	ret:	none
*/
void playGame(const char* startRoom) {
	// Allocate the memory for the game.
	char** visitedRooms = malloc((NUM_ROOMS * 2) * sizeof(char*));
	char* currName = malloc(NAME_LENGTH * sizeof(char));
	char* dirName = malloc(DIR_BUFFER_LENGTH * sizeof(char));
	Room currRoom;
	Unt visits = 0;
	char strBuffer[NAME_LENGTH];
	char strBuffer2[NAME_LENGTH];
	char strBuffer3[NAME_LENGTH];
	Unt i;
	// Allocate the memory for each string pointer in the visitedRooms array.
	for(i = 0; i < (NUM_ROOMS * 2); i++) {
		visitedRooms[i] = (char*)malloc(NAME_LENGTH * sizeof(char));
	}
	sprintf(currName, "%s", startRoom);
	getDirectory(dirName);
	assert(chdir(dirName) == 0);	
	while(true) {
		bool doAgain;
		do {
			doAgain = false;	
			// Read in the game data from file.
			FILE* fptr;
			fptr = fopen(currName, "r");
			fscanf(fptr, "ROOM NAME: %s\n", strBuffer);
			Unt reader, connNum;
			while((reader =	fscanf(fptr, "CONNECTION %d: %s\n", &connNum, strBuffer2)) != 0 && reader != EOF) {
				for(i = 0; i < NUM_ROOMS; i++) {
					if(strcmp(strBuffer2, rooms[i].roomName) == 0) {
						currRoom.conns[connNum - 1] = &rooms[i];
						break;
					}
				}
			}	
			currRoom.connCnt = connNum;
			fscanf(fptr, "ROOM TYPE: %s\n", strBuffer3);
			if(strcmp(strBuffer3, "START_ROOM") == 0) {
				currRoom.type = START_ROOM;
			}
			else if(strcmp(strBuffer3, "END_ROOM") == 0) {
				currRoom.type = END_ROOM;
			}
			else {
				currRoom.type = MID_ROOM;
			}
			fclose(fptr);
			// If the current room is the END_ROOM, end the game and clean up.
			if(currRoom.type == END_ROOM) {
				printf("\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
				printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", visits);
				for(i = 0; i < visits; i++) {
					printf("%s\n", visitedRooms[i]);
				}
				assert(chdir("..") == 0);
				// Free the visitedRooms array pointers.
				for(i = 0; i < (NUM_ROOMS * 2); i++) {
					free(visitedRooms[i]);
				}
				free(visitedRooms);
				free(dirName);
				free(currName);
				return;
			}
			// Print out the user interface.
			printf("\nCURRENT LOCATION: %s\n", currName);
			printf("POSSIBLE CONNECTIONS: ");
			for(i = 0; i < currRoom.connCnt - 1; i++) {
				printf("%s, ", currRoom.conns[i]->roomName);
			}
			printf("%s.\n", currRoom.conns[currRoom.connCnt - 1]->roomName);
			printf("WHERE TO? >");	
			// Get the input from the user.
			fgets(strBuffer, NAME_LENGTH, stdin);
			// Clean up the input
			stripChar(strBuffer, ' ');
			stripChar(strBuffer, '\n');
			// Compare the user input to the connection room names.
			for(i = 0; i < currRoom.connCnt; i++) {
				if(strncmp(strBuffer, currRoom.conns[i]->roomName, NAME_LENGTH) == 0) {
					strncpy(currName, strBuffer, NAME_LENGTH);
					strncpy(visitedRooms[visits], strBuffer, NAME_LENGTH);
					visits++;
					doAgain = true;
					break;
				}
			}
		} while(doAgain == true);
	printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n");	// Input was bad.
	}
}


/* Removes a specified character from a string.
  	param:  str  a pointer to a string.
	param:  c    character instance to remove.
	pre:	str is not NULL.
  	post:	all instances of c in str are gone.
 	ret:	none
*/
void stripChar(char* str, char c){
	char *pr = str, *pw = str;
	while(*pr) {
		*pw = *pr++;
		pw += (*pw != c);
	}
	*pw = '\0';	// Make sure to null terminate.
}


/* Generate an array of Room structs randomly.
  	param:  roomArray[]  reference to an array of Rooms.
	pre:	none
  	post:	roomArray[] is filled with randomly chosen Room 
		structs by reference.
 	ret:	none
*/
void createRooms(Room roomArray[]) {
	bool namesInUse[NUM_NAMES];
	// Define memory for saving used room names.
	memset(&namesInUse, 0, NUM_NAMES * sizeof(bool));
	Unt i;
	// Randomize connection counts for the rooms.
	for(i = 0; i < NUM_ROOMS; i++) {
		roomArray[i].connCnt = 0;
		Unt connCap = (rand() % (CONN_MIN + 1)) + (rand() % (CONN_MIN - 2));
		if(connCap < CONN_MIN) {
			connCap = CONN_MIN;
		}
		else if(connCap > CONN_MAX) {
			connCap = CONN_MAX;
		}
		roomArray[i].connCap = connCap;
		while(true) {
			Unt idx = rand() % NUM_NAMES;
			if(!namesInUse[idx]) {
				namesInUse[idx] = true;
				roomArray[i].roomName = roomNames[idx];
				break;
			}
		}
		roomArray[i].type = MID_ROOM;		// Middle room.
	}
	// Connect each of the rooms with random rooms.
	for(i = 0; i < NUM_ROOMS; i++) {
		Unt j;
		for(j = 0; j < roomArray[i].connCap; j++) {
			Unt randRoom = rand() % NUM_ROOMS;
			while(!connectRooms(i, randRoom, roomArray)) {
				randRoom = rand() % NUM_ROOMS;
			}
		}
	}
	roomArray[0].type = START_ROOM;			// Start room.
	roomArray[NUM_ROOMS - 1].type = END_ROOM;	// End room.
}


/* Writes formatted Room data to files in a specified directory.
  	param:  roomArray[]  reference to an array of Rooms.
	pre:	roomArray[] must not be NULL or empty.
	post:	none
  	ret:	Room data is printed to respective files within
		the specified directory.
*/
void writeRooms(Room roomArray[]) {
	// Allocate memory
	char* dirName = malloc(DIR_BUFFER_LENGTH * sizeof(char));
	getDirectory(dirName);
	// Make a new directory.
	if(mkdir(dirName, 0777) != 0) {
		perror("Error creating directory!\n");
		exit(1);
	}
	assert(chdir(dirName) == 0);
	Unt i;
	// Print data to each respective room file.
	for(i = 0; i < NUM_ROOMS; i++) {
		FILE* fptr = fopen(roomArray[i].roomName, "w");
		fprintf(fptr, "ROOM NAME: %s\n", roomArray[i].roomName);
		Unt j;
		for(j = 0; j < roomArray[i].connCnt; j++) {
			fprintf(fptr, "CONNECTION %d: %s\n", (j + 1), roomArray[i].conns[j]->roomName);
		}
		if(roomArray[i].type == START_ROOM) {
			fprintf(fptr, "ROOMTYPE: START_ROOM\n");
		}
		else if(roomArray[i].type == END_ROOM) {
			fprintf(fptr, "ROOMTYPE: END_ROOM\n");
		}
		else {
			fprintf(fptr, "ROOMTYPE: MID_ROOM\n");
		}
		fclose(fptr);
	}
	assert(chdir("..") == 0);
	free(dirName);		// Clean up
}


/* Creates a connection between two Rooms.
 	param:  r1   first array index.
	param:  r2   second array index.
	param:  roomArray[]  reference to an array of Rooms.
	pre:	roomArray[] must not be NULL or empty.
		r1 cannot be equal to r2.
		either room cannot have max connections.
  	post:	the two Rooms are connected.
 	ret:	true if connection succeeded, false otherwise.
*/
bool connectRooms(Unt r1, Unt r2, Room roomArray[]) {
	Room* rA = &roomArray[r1];	// Room A
	Room* rB = &roomArray[r2];	// Room B
	// If Room A has full connections.
	if(rA->connCnt == CONN_MAX) {
		return true;
	}
	// If the Rooms are already connected.
	else if(isConnected(r1, r2, roomArray)) {
		return false;
	}
	// If either Rooms have full connections.
	else if(rA->connCnt >= CONN_MAX || rB->connCnt >= CONN_MAX) {
		return false;
	}
	// Connect the rooms to each other.
	else {
		rA->conns[rA->connCnt] = rB;
		rB->conns[rB->connCnt] = rA;
		rA->connCnt++;
		rB->connCnt++;
		return true;
	}
}


/* Checks to see if two rooms are already connected.
	param:  r1   first array index.
	param:  r2   second array index.
	param:  roomArray[]  reference to an array of Rooms.
	pre:	roomArray[] must not be NULL or empty.
	post:	none
	ret:	true if rooms are connected, false otherwise.
*/
bool isConnected(Unt r1, Unt r2, Room roomArray[]) {
	// If indexs are the same, its the same room.
	if(r1 == r2) {
		return true;
	}
	// If a connection already exists between the rooms
	else {
		Unt i;
		for(i = 0; i < roomArray[r1].connCnt; i++) {
			if(roomArray[r1].conns[i] != NULL) {
				if(roomArray[r1].conns[i] == &roomArray[r2]) {
					return true;
				}
			}
		}
		return false;	// There must be no connection.
	}
}


/* Assigns a directory name to the name pointer.
	param:  name   a pointer to a string.
	pre:	name must not be NULL.
	post:	none
	ret:	none
*/
void getDirectory(char* name) {
	assert(name != NULL);
	pid_t pid = getpid();		// Process ID
	sprintf(name, "hemphilc.rooms.%d", pid);
}

