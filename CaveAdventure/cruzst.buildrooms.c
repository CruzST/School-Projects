/*******************************************************************************
Name: Steven Cruz

File description:
	This file contains the code required for Program 2, build rooms.
	This file will create rooms for the adventure program.
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // for getpid
#include <time.h>

// Global conditions
#define MIN_CONNECTIONS 3
#define MAX_CONNECTIONS 6
#define ROOM_NUM 7

struct room {
	char* name;
	int numOutBoundConnect;
	int size;
	struct room* outBoundConnectList[MAX_CONNECTIONS];				// List of rooms the struct is connected to. Max 6.
  char* type;
  int id;
};

/** Variable name declerations */
char directoryName[100];
char* roomNames[10] = {"Afterglo", "Roselia", "PoPiPa", "HaroHapi", "PasuPare", "GuriGuri", "TheThird", "Cyaron", "Yosoro", "ItsShiny"};
char* roomType[3] = {"START_ROOM", "END_ROOM", "MID_ROOM"};


/**************************

  Function declarations

**************************/
char* makeDirectory();
void makeFiles(struct room roomList[ROOM_NUM], char* directoryName);
void makeRooms(struct room roomList[ROOM_NUM]);
void connectRooms(struct room a, struct room b, struct room roomList[ROOM_NUM]);
struct room getRandomRoom(struct room roomList[ROOM_NUM]);
int isSameRoom(struct room a, struct room b);
int alreadyConnected(struct room a, struct room b, struct room roomList[ROOM_NUM]);
int isRoomValid(struct room a, struct room roomList[ROOM_NUM]);



/************************

  Function Definitions

*************************/

/** makeDirectory
    Inputs: nothing
    Returns: A directory name
    Function creates a directory to put the files in. */
char* makeDirectory() {
	sprintf(directoryName, "cruzst.rooms.%d", getpid());
	mkdir(directoryName, 0700);						// Only user can r-w-e
	return directoryName;
}

void makeFiles(struct room roomList[ROOM_NUM], char* directoryName){
  chdir(directoryName);
  FILE* newFile;
  int i;
  for(i = 0; i< ROOM_NUM; i++){
    newFile = fopen(roomList[i].name, "a");
    fprintf(newFile, "ROOM NAME: %s\n", roomList[i].name);

    int j;
    for (j = 0; j < roomList[i].numOutBoundConnect; j++){
      fprintf(newFile, "CONNECTION %d: %s\n", j+1, roomList[i].outBoundConnectList[j]->name);
    }
    fprintf(newFile, "ROOM TYPE: %s", roomList[i].type);
    fclose(newFile);
  }
}

/** makeFiles
    Inputs: A list of room structures, a folder directory name
    Returns: Nothing
    Turns ea struct in the array into a file, then puts it in the
    directory **/


/** makeRooms
    Inputs: an array of the room structures
    Returns: nothing
    Big lifter function that will make the rooms. */
void makeRooms(struct room roomList[ROOM_NUM]){
  /* This block will do the following:
     Assign the rooms
     Names and id
     The MID_ROOM type to start. (start and end will be determined after)
     The number of connections to each room. (this is variable but gives the program a start)
     The size to 0
     The connection List.
  */
  int j;
  for (j = 0; j < ROOM_NUM; j++){						              // j is the increment for the roomList holding the structures
    int rando;
    int dupeFlag = 1;
    while (dupeFlag){								                      // Set flag to 0, get a number 0 thru 9.
      rando = rand() % 10;
      dupeFlag = 0;
      int k;									                            // k is the increment for the roomList again
      for (k = 0; k < j+1; k++){
        if (roomList[k].name == roomNames[rando])				  // if there is a dupe, it will repeat.  // THIS WORKS. but improper?
          dupeFlag = 1;
      }
    } // End While
    roomList[j].name = roomNames[rando];					        // When it exits while, a valid room name is assigned and added.
    roomList[j].type = roomType[2];						            // Assign all to MID_ROOM for now
    roomList[j].size = 0;                                 // Assign the size to 0.
    roomList[j].id = j;

    int numConnect = rand() % (MAX_CONNECTIONS + 1 -  MIN_CONNECTIONS) + MIN_CONNECTIONS;
    roomList[j].numOutBoundConnect = numConnect;
  } // End outer if

  /* This block determines the starting and ending rooms */
  int dupeStartEnd = 1;
  int startRoom;
  int endRoom;
  while (dupeStartEnd){                                   // While loop prevents the same start/end room
    startRoom = rand() % ROOM_NUM;
    endRoom = rand() % ROOM_NUM;
    dupeStartEnd = 0;
    if (startRoom == endRoom)
      dupeStartEnd = 1;
  }
  roomList[startRoom].type = roomType[0];
  roomList[endRoom].type = roomType[1];

 /* This block will make the connections from room to room */
  int i;
  for (i = 0; i < ROOM_NUM; i++){                                                           // Outer loop, each room(outer) will have
    int k;                                                                                  // an inner loop that will iterate
    for (k = 0; k < roomList[i].numOutBoundConnect; k++){                                   // over the room's(outer) subarray of rooms
      struct room roomA = roomList[i];
      struct room roomB = getRandomRoom(roomList);
      int sameFlag = isSameRoom(roomA, roomB);                                              // Flags to check if the potential connect is
      int alreadyFlag = alreadyConnected(roomA, roomB, roomList);                           // valid.
      int roomAvalid = isRoomValid(roomA, roomList);

      if (roomAvalid) {
        while (sameFlag || alreadyFlag){                                                    // Get a new roomB until you find a valid one
          roomB = getRandomRoom(roomList);
          sameFlag = isSameRoom(roomA, roomB);
          alreadyFlag = alreadyConnected(roomA, roomB, roomList);
        }
        connectRooms(roomA, roomB, roomList);                                               // Upon exiting the while, a valid roomB
      } // End roomAvalid loop                                                              // has been found
    } // End k loop
  } // End i loop

/*This block will reassign a connection cap to the rooms*/
  int f;                                                                                    // Fix the size vs connection cap
  for (f = 0; f < ROOM_NUM; f++){
    int newCap = roomList[f].size;
    roomList[f].numOutBoundConnect = newCap;
  }
}

/** connectRooms
    Inputs: 2 room structures, room structure array
    Returns: Nothing
    Takes in the 2 arrays, identifies their ID and assigns the rooms to each other
    within the roomList array **/
void connectRooms(struct room a, struct room b, struct room roomList[ROOM_NUM]){
  int aPos = a.id;
  int bPos = b.id;
  int aSize = roomList[aPos].size;
  int bSize = roomList[bPos].size;
  roomList[aPos].outBoundConnectList[aSize] = &roomList[bPos];
  roomList[bPos].outBoundConnectList[bSize] = &roomList[aPos];
  roomList[aPos].size++;
  roomList[bPos].size++;
}

/** getRandomROom
    Inputs: an array of room structures
    Returns: a room structure
    Function will return a random room. **/
struct room getRandomRoom(struct room roomList[ROOM_NUM]){
  int rando = rand() % ROOM_NUM;
  return roomList[rando];
}

/** isSameRoom
    Inputs: 2 room structures
    Returns: 0 or 1
    Determines if the room structs are the same
    base on their name. **/
int isSameRoom(struct room a, struct room b){
 return (a.name == b.name);
}

/** alreadyConnected
    Inputs: 2 room structures
    Returns: 0 or 1
    Determines if the second arg exists in the
    first args list of connected rooms. **/
int alreadyConnected(struct room a, struct room b, struct room roomList[ROOM_NUM]){
  int connected = 0;
  int aPos = a.id;
  int i;
  for (i = 0; i < roomList[aPos].size; i++){
    if (roomList[aPos].outBoundConnectList[i] != NULL){
      if (strcmp(roomList[aPos].outBoundConnectList[i]->name, b.name)==0)
        connected = 1;
    }
  }
  return connected;
}

/** Name: isConnectionsFilled
    Inputs: a room structure
    Returns: 0 or 1
    Determines if the room's
    connections are maxed out */
int isRoomValid(struct room a, struct room roomList[ROOM_NUM]){
  int status = 1;
  int aPos = a.id;
  if ((roomList[aPos].size >= roomList[aPos].numOutBoundConnect))
    status = 0;
  return status;
}



/*************** BEGIN MAIN SCRIPT *****************/
int main() {

  /* delcare seed and the new directory */
  srand(time(NULL));
  char* newDic = makeDirectory();						// Works, makes a new directory.
  struct room roomList[ROOM_NUM];

  makeRooms(roomList);
  makeFiles(roomList,newDic);

  return 0;
}

// END MAIN SCRIPT
