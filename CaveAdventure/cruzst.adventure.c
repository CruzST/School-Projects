/*************************************************
* Name: Steve Cruz
* Date: 10/25/2018
* Description: File contains the code required
*  for program 2. Adventure
**************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

// Global conditions
#define MIN_CONNECTIONS 3
#define MAX_CONNECTIONS 6
#define ROOM_NUM 7


struct room {
	char* name;
	int outBoundNum;
	struct room* outBoundList[MAX_CONNECTIONS];				// List of rooms the struct is connected to. Max 6.
  char* type;
  int id;
};


/** Global Variable name declerations */
char* roomNames[10] = {"Afterglo", "Roselia", "PoPiPa", "HaroHapi", "PasuPare", "GuriGuri", "TheThird", "Cyaron", "Yosoro", "ItsShiny"};
pthread_mutex_t mainMutex = PTHREAD_MUTEX_INITIALIZER;
char* roomType[3] = {"START_ROOM", "END_ROOM", "MID_ROOM"};


/** Function declarations **/
int playGame(char startRoom[20], struct room roomList[ROOM_NUM]);

int getNewestDirectory(char word[256]);
int startEndRoom(char word[20], char word2[20], char latestDir[256]);
void assignRoomDefaults(char latestDir[256], struct room roomList[ROOM_NUM]);
int assignRoomConnections(char latestDir[256], struct room roomList[ROOM_NUM]);
int assignStartEndRoom(char startRoom[20], char endRoom[20], struct room roomList[ROOM_NUM]);
int getRoomPos(char name[20], struct room roomList[ROOM_NUM]);
int winCondition(struct room roomStruct);
int validInput(char input[100], struct room roomParent, struct room roomList[ROOM_NUM]);
void makeThread();
void* getTime();
void writeTime();
int readTime();

/** FUNCTION DEFINITIONS **/
/* getNewestDirectory
   Inputs: A string array
   Returns: 0 or 1 depending if it is working
   Function referenced from 2.4 of the CS344 notes
   Function will determine the latest directory with
   target prefix. */
int getNewestDirectory(char word[256]){
  time_t newTime = -1;
  char targetDirPrefix[32] = "cruzst.rooms.";
  char newestDirName[256];
  memset(newestDirName, '\0', sizeof(newestDirName));                           // Memset will set the array to null terminators.

  DIR* dirToCheck;
  struct dirent* fileInDir;
  struct stat dirAttributes;

  dirToCheck = opendir(".");
  if (dirToCheck != NULL){                                                      // Function copied from 2.4 lecture.  TA Referenced on properly
    while ((fileInDir = readdir(dirToCheck)) != NULL){                          // modifying strings.
      if (strstr(fileInDir->d_name, targetDirPrefix) != NULL){
        stat(fileInDir->d_name, &dirAttributes);
        if ((dirAttributes.st_mtime > newTime)){
          newTime = dirAttributes.st_mtime;
          memset(newestDirName, '\0', sizeof(newestDirName));
          strcpy(newestDirName, fileInDir->d_name);
        }
      }
    }
  } else {
      fprintf(stderr, "FUNCTION: ERROR Directory was bad.\n");
      return -1;
  }
  closedir(dirToCheck);
  strcpy(word, newestDirName);
  return 0;
}



/* startingRoom
   Inputs: 2 string arrays, directory string
   Returns: 0 or 1 depending if it works.
   Function will determine which room within the target
   directory is the starting and end room */
int startEndRoom(char start[20], char end[20], char latestDir[256]){

  DIR* directory = opendir(latestDir);
  struct dirent* fileInDir;
  char fileLine[100];
  char filePath[100];
  memset(fileLine, '\0', sizeof(fileLine));

  FILE* entryFile;

  if (directory != NULL){
    while ((fileInDir = readdir(directory)) != NULL){

      if (!strcmp(fileInDir->d_name, "."))                                                  // Skip current and parent directory
        continue;                                                                           // obtained from stack overflow.
      if (!strcmp(fileInDir->d_name, ".."))
        continue;

      sprintf(filePath, "%s/%s", latestDir, fileInDir->d_name);                             // create the right filepath to open. does not work without the above statement (BUG 2)
      entryFile = fopen(filePath, "r");

      if (entryFile == NULL){ fprintf(stderr, "FUCTION: ERROR FILE WAS NULL \n"); return -1;}         // Check if null
      while (fgets(fileLine, 100, entryFile) != NULL){                                                // check each line, look for START_ROOM and assign it to the variable
        if (strstr(fileLine, "START_ROOM")){                                                          // passed, like in the directory function
          strcpy(start, fileInDir->d_name);
        } else if (strstr(fileLine, "END_ROOM")){
          strcpy(end, fileInDir->d_name);
        }
        else {
          memset(fileLine, '\0', sizeof(fileLine));
        }
      }
      fclose(entryFile);
    }
    closedir(directory);
  }

  if (start != NULL && end != NULL){
    return 0;
  } else {
    fprintf(stderr, "FUCTION: ERROR START/END ROOM NOT FOUND. \n");                           // if it made it this far, no start room was found
    return -1;
  }

}

/* getRoomDefaults
   Inputs: directory, roomList structure array
   Returns: Nothing
   Function will scan thru the roomList array and assign it basic
   information, ie : id number, name, connections = 0
   and type to MID_ROOM to start.
*/
void assignRoomDefaults(char latestDir[256], struct room roomList[ROOM_NUM]){
  DIR* directory = opendir(latestDir);
  struct dirent* fileInDir;
  //char fileName[100];

  //FILE* entryFile;

  if (directory != NULL){
    int i = 0;
    while ((fileInDir = readdir(directory)) != NULL){
      if (!strcmp(fileInDir->d_name, "."))                                                  // Skip current and parent directory
        continue;                                                                           // obtained from stack overflow.
      if (!strcmp(fileInDir->d_name, ".."))
        continue;

      roomList[i].name = fileInDir->d_name;
      roomList[i].type = roomType[2];
      roomList[i].id = i;
      roomList[i].outBoundNum = 0;
      i++;
    }
  }
}


/* assignStartEndRoom
   Inputs: startroom name, endroom name, roomList structure array
   Returns: 0 or 1 depending if it was successful
   Function will scan thru the structure array and check which matches
   the start room and end room.  It will assign them accordingly
*/
int assignStartEndRoom(char startRoom[20], char endRoom[20], struct room roomList[ROOM_NUM]){
  int i;
  int flag = 0;
  for (i = 0; i < ROOM_NUM; i++){
    if (strcmp(startRoom, roomList[i].name) == 0){
      roomList[i].type = roomType[0];
      flag++;                                                                                       // Flag must be 2 for it to succeed.
    }
    if (strcmp(endRoom, roomList[i].name) == 0){
      roomList[i].type = roomType[1];
      flag++;
    }
  }
  if (flag == 2){
    return 0;
  } else {
    fprintf(stderr, "ERROR SOMETHING WENT WRONG");
    return -1;
  }
}

/* assignRoomConnections
   Inputs: directory name, room structure array
   Returns 0 or 1 depending.
   Function will scan thru the room list and assign the connections
   as they appear in the file.  The connections will be held in
   the array of room structures
*/
int assignRoomConnections(char latestDir[256], struct room roomList[ROOM_NUM]){
  char filePath[100];
  char fileLine[100];
  char targetRoom[20];
  memset(fileLine, '\0', sizeof(fileLine));

  FILE* entryFile;

  int i;
  for (i = 0; i < ROOM_NUM; i++){

    memset(filePath, '\0', sizeof(filePath));
    sprintf(filePath, "%s/%s", latestDir, roomList[i].name);
    entryFile = fopen(filePath, "r");

    int j = -1;
    if (entryFile == NULL){ fprintf(stderr, "FUNCTION: ERROR FILE WAS NULL\n"); return -1;}
    while (fgets(fileLine, 100, entryFile) != NULL){                                                // Open each file and get the connections
      if (strstr(fileLine, "CONNECTION")){
        memset(targetRoom, '\0', sizeof(targetRoom));
        sscanf(fileLine, "%*s %*s %s", targetRoom);
        int targetPos = getRoomPos(targetRoom, roomList);
        j++;
        roomList[i].outBoundList[j] = &roomList[targetPos];                                         // add the structure and update the connection count
        roomList[i].outBoundNum++;
      }
      memset(fileLine, '\0', sizeof(fileLine));                                                     // Reset the file line
    }
    fclose(entryFile);
  }
  return 0;
}


/* getRoomPos
   Inputs: room name, room structure array
   Returns an int
   function takes in a room name, compares it against the room list
   and returns its position in the array, or its ID.
*/
int getRoomPos(char name[20], struct room roomList[ROOM_NUM]){
  int i;
  for (i = 0; i < ROOM_NUM; i++){
    if (strcmp(roomList[i].name, name) == 0)
      return roomList[i].id;
  }
  fprintf(stderr, "ERROR (getRoomPos)\n");
  return -1;
}

/* winCondition
   inputs room structure
   returns 0 or 1
   function checks the room type vs the
   win condition phrase AKA END_ROOM
*/
int winCondition(struct room roomStruct){
  if (strcmp(roomStruct.type, roomType[1]) == 0){
    return 1;
  } else {
    return 0;
  }
}

/*
   validInput
   Inputs: string array, parent room, Roomlist
   returns: 0 or 1
   Function scans thru the current room(room parent)'s outbound list and checks if the input
   is in that list.
*/
int validInput(char input[100], struct room roomParent, struct room roomList[ROOM_NUM]){
  // get the right array of structures.
  int i;
  for (i = 0; i < roomParent.outBoundNum; i++){
    if (strcmp(input, roomList[roomParent.id].outBoundList[i]->name) == 0){
      return 1;
    }
  }
  return 0;
}



/* time stuff
   Function will init the main mutex, lock it, and then create a new thread that will
   call the gettime function.  it will then unlock the main mutex, and destroy the
   created thread.
   outline for code structure followed: https://www.thegeekstuff.com/2012/05/c-mutex-examples/?refcom%3B
*/
void makeThread(){
  pthread_t threadID;
  int init = pthread_mutex_init(&mainMutex, NULL);

  if (init != 0)
    fprintf(stderr, "\nError when trying to init mainMutex.\n\n");

  pthread_mutex_lock(&mainMutex);
  int err = pthread_create(&threadID, NULL, getTime, NULL);

  if (err != 0)
    fprintf(stderr, "\nError when trying to create new thread.\n\n");

  pthread_mutex_unlock(&mainMutex);
  pthread_join(threadID, NULL);
  pthread_mutex_destroy(&mainMutex);
}

/* Function will get the time, and write it to the file with the
   proper format.
*/
void* getTime(){
  FILE* myFile;
  myFile = fopen("currentTime.txt", "w+");
  char fileLine[100];
  struct tm* timeStruct;

  time_t currentTime = time(NULL);                                                              // Gets current time in seconds.
  timeStruct = localtime(&currentTime);                                                         // Fills the time struct based on the time got in the prev line.
  strftime(fileLine, sizeof(fileLine), "%I:%M%P %A, %B %e, %Y", timeStruct);                    // Breaks down the tm Struct into the elements we need.
  fputs(fileLine, myFile);
  fclose(myFile);
}

/* Function will read the time from the file and print it */
int readTime(){
  FILE* myFile;
  myFile = fopen("currentTime.txt", "r+");
  if (myFile == NULL){ fprintf(stderr, "FUNCTION: ERROR FILE WAS NULL\n"); return -1;}

  char fileLine[100];
  fgets(fileLine, 100, myFile);
  printf("\n%s\n\n", fileLine);

  fclose(myFile);
  return 0;
}


/** GAME PLAY FUNCTION **/
int playGame(char startRoom[20], struct room roomList[ROOM_NUM]){
  int stepCount = 0;
  int gameEnd = 0;
  char targetRoom[100];
  char* roomPath[200];
  int roomPos = getRoomPos(startRoom, roomList);
  struct room roomToCheck = roomList[roomPos];

  do{
      printf("CURRENT LOCATION: %s\n", roomToCheck.name);
      printf("POSSIBLE CONNECTIONS: ");
      int i;
      for (i = 0; i < roomToCheck.outBoundNum; i++){
        if (i == roomToCheck.outBoundNum - 1){
          printf("%s.", roomToCheck.outBoundList[i]->name);
        } else {
          printf("%s, ", roomToCheck.outBoundList[i]->name);
        }
      }
      printf("\n");

      printf("WHERE TO? >");
      memset(targetRoom, '\0', sizeof(targetRoom));
      scanf("%s", targetRoom);

      if (strcmp(targetRoom, "time") == 0){
        makeThread();
        readTime();

      }
      else if (validInput(targetRoom, roomToCheck, roomList)){
        printf("\n");
        roomPos = getRoomPos(targetRoom, roomList);
        roomToCheck = roomList[roomPos];
        roomPath[stepCount] = roomList[roomPos].name;
        stepCount++;
      } else {
        printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
      }
      gameEnd = winCondition(roomToCheck);

  } while (!gameEnd);
  printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
  printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", stepCount);
  int p;
  for (p = 0; p < stepCount; p++)
    printf("%s\n", roomPath[p]);
  return 0;
};


/******* START MAIN *********/
int main(){
  /* Declare the directory variable and pass it
     to the function to modify the variable */
  char newestDir[256];
  memset(newestDir, '\0', sizeof(newestDir));
  getNewestDirectory(newestDir);


  /* This block will find the start and end
     rooms and assign them to the respective
     variables */
  char startRoom[20];
  char endRoom[20];
  memset(startRoom, '\0', sizeof(startRoom));
  memset(endRoom, '\0', sizeof(endRoom));
  startEndRoom(startRoom, endRoom, newestDir);

  /* This Block will recreate the room structure array
     that was created by the build rooms program*/
  struct room roomList[ROOM_NUM];
  assignRoomDefaults(newestDir, roomList);
  assignStartEndRoom(startRoom, endRoom, roomList);
  assignRoomConnections(newestDir, roomList);

  /* Run the game */
  playGame(startRoom, roomList);

  return 0;
}
// end main
