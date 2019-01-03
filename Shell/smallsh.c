/***********************************
**Name: Steven Cruz
**Date: 11/6/2018
************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <sys/types.h>
#include <fcntl.h>

#define MAX_ARGS 512



/**
    Struct to keep track of background pids
    Has a count, current number for PIDS
    An array of pid_t, maximum 100
**/
struct PIDstruct {
  int count;
  pid_t BGpid[100];
};

// Globals
int status = 0;
int FGonly = 0;
struct PIDstruct PIDstack = {0};


/** push bg pid to stack **/
void pushPID(pid_t processID){
  PIDstack.BGpid[PIDstack.count] = processID;
  PIDstack.count++;
}

/** pop it off when its done **/
void popTarPID(pid_t processID){
  int i;
  int pos;
  for (i = 0; i < PIDstack.count; i++){
    if (PIDstack.BGpid[i] == processID){
      pos = i;
      break;
    }
    for (i = pos; i < PIDstack.count; i++){
      PIDstack.BGpid[i] = PIDstack.BGpid[i + 1];
    }
  }
  PIDstack.count--;
}

/** Returns status **/
void getStatus(int status){                                                       // From Block 3 Lectures
  if (WIFEXITED(status)){
    printf("Exit value %i\n", WEXITSTATUS(status));
  } else {
    printf("Terminated by signal %i\n", status);
  }
}

/** For the ctrl+z signal **/
void trapSIGSTPsignal(int sig){
  if (FGonly == 0){
    char* string = "\nEntering foreground-only mode (& is now ignored)\n: ";
    write(STDOUT_FILENO, string, 52);
    fflush(stdout);
    FGonly = 1;
  } else {
    char* string = "\nExiting foreground-only mode\n: ";
    write(STDOUT_FILENO, string, 33);
    fflush(stdout);
    FGonly = 0;
  }
}

/** For the ctrl+c signal **/
void trapSIGINTsignal(int sig){
  char* string = "\nterminated by signal 2\n: ";
  write(STDOUT_FILENO, string, 26);
  fflush(stdout);
}

/** Will determine if the input is a background process and return its position **/
int isLastCharBG(char* arguments[MAX_ARGS]){
  int flag = 0;
  int i = 0;
  while (arguments[i] != NULL && flag != 1){
    if (strcmp(arguments[i], "&") == 0 && arguments[i+1] == NULL){
      flag = 1;
    } else {
      i++;
    }
  }
  if (flag == 1)
    return i;
  else
    return -1;
}

/** Gets a line from the user **/
char* getLineInput(){
/* Function will get the line input from a use, and return it.
   Obtained from the 3.3 lecture reading.*/
  char* lineEntered = NULL;
  size_t bufferSize = 0;
  // works but trying the other way
  //getline(&lineEntered, &bufferSize, stdin);                                      // lineEntered now contains the raw string input WITH the \n at the end.
  int valid = -5;
  while (1){
    valid = getline(&lineEntered, &bufferSize, stdin);
    if (valid == -1)
      clearerr(stdin);
    else
      break;
  }
  lineEntered[strcspn(lineEntered, "\n")] = '\0';                                 // Trim it off
  return lineEntered;
}

/** Checks if a word is a comment **/
int isComment(char* word){
  int flag = 0;
  if (word[0] == '#'){
    flag = 1;
  }
  return flag;
}


/** detects $$ in a word **/
int isArgDollar(char* word){
  int flag = 0;
  if (strstr(word, "$$") != NULL){
    flag = 1;
  }
  return flag;
}

/**
    Will determine if a redirect is required.
    If it is, it will set it up before
    passing the arguments into execvp
**/
void redirect(char* arguments[MAX_ARGS]){
  // declare
  int inputFileD = STDIN_FILENO;
  int outputFileD = STDOUT_FILENO;
  char* inputFileN = NULL;
  char* outputFileN = NULL;
  int argCount = 0;
  // get number of elements in the array
  int j;
  for (j = 0; arguments[j] != NULL; j++){
      argCount++;
  }
  // check if there are redirects to be done.
  int i;
  for (i = 0; i < argCount; i++){
    if (arguments[i] != NULL){
      if (strcmp(arguments[i], "<") == 0){
        inputFileN = arguments[i + 1];
          arguments[i] = arguments[i + 1];
          arguments[i + 1] = NULL;
      }
      if (strcmp(arguments[i], ">") == 0){                                          // want to pass only 'ls' thru exec, must set the rest of the array to null
        outputFileN = arguments[i + 1];
        arguments[i] = NULL;
        arguments[i + 1] = NULL;
      }
    }
  } // end for
  // ready them
  if (inputFileN != NULL){
    inputFileD = open(inputFileN, O_RDONLY);
    if (inputFileD == -1){
      printf("\ncannot open %s for input\n", inputFileN);
      fflush(stdout);
      _Exit(1);
    }
    dup2(inputFileD, 0);
    close(inputFileD);
  }
  if (outputFileN != NULL){
    outputFileD = open(outputFileN, O_WRONLY | O_CREAT | O_TRUNC, 0644);          // Permissions: https://support.hostgator.com/articles/cpanel/how-to-change-permissions-chmod-of-a-file
    if (outputFileD == -1){                                                       // also in the lectures
      printf("\ncannot open %s\n", outputFileN);
      fflush(stdout);
      _Exit(1);
    }
    dup2(outputFileD, 1);
    close(outputFileD);
  }
}

/********************************
** The heavy lifting function
** Function will get input,
** parse it, and execute it.
** it depending on input, it will
** return 0 or 1 to main
/********************************/
int parseAndExecuteLine(){
  char* arguments[MAX_ARGS];                                                           // Array of MAX_ARGS string literals
  char* input;
  pid_t pid;

  char* token;
  int count = 0;
  int bgProcess = -1;                                                             // Check for the input, if it is a background process.

  int z;
  for (z = 0; z < MAX_ARGS; z++){
    arguments[z] = NULL;
  }


  // Init Signals
  struct sigaction STPsignal = {0};
  struct sigaction INTsignal = {0};
  //struct sigaction ignore_action = {0};

    // crtl+z
    STPsignal.sa_handler = trapSIGSTPsignal;
    sigfillset(&STPsignal.sa_mask);
    STPsignal.sa_flags = 0;//SA_RESTART;

    // ctrl+c
    INTsignal.sa_handler = SIG_IGN;
    sigfillset(&INTsignal.sa_mask);
    INTsignal.sa_flags = 0;//SA_RESTART;

    // Set the signals
    sigaction(SIGTSTP, &STPsignal, NULL);
    sigaction(SIGINT, &INTsignal, NULL);

  // Get Input
  fflush(stdout);
  fflush(stdin);
  printf(": ");
  fflush(stdout);
  fflush(stdin);
  input = getLineInput();

  // Parse the line an put it into an arguments array
  token = strtok(input, " ");
  while (token != NULL){
    arguments[count] = strdup(token);
    /* DOES NOT WORK PROPERLY
    if (isArgDollar(arguments[count])){
      char* wordPartA = strtok(arguments[count], "$$");
      printf("%s\n", wordPartA);
      if (wordPartA != NULL){
        sprintf(arguments[count], "%s%d", wordPartA, getpid());
      } else {
        sprintf(arguments[count],"%d",getpid());
      }
      //printf("%s\n", arguments[count]);
    }
    */
    if (isArgDollar(arguments[count])){
      sprintf(arguments[count],"%d",getpid());
    }
    token = strtok(NULL, " ");
    count++;
  }

  // Check if it is a background process.
    bgProcess = isLastCharBG(arguments);
    if (bgProcess)
      arguments[bgProcess] = NULL;

  // Detect if the string is empty, or it is a comment strcmp(arguments[0], "#") == 0
  if (arguments[0] == NULL || isComment(arguments[0])){
    // Dont care if its empty or a comment exit the parse loop, no execute
    ;
  }
  else if (strcmp(arguments[0], "exit") == 0 && arguments[1] == NULL){
    return 0;
  }
  else if (strcmp(arguments[0], "cd") == 0){
    if (arguments[1] == NULL){
      chdir(getenv("HOME"));
    } else {
      if (chdir(arguments[1]) != 0){
       printf("Error directory: %s not found.\n", arguments[1]);
      }
    }
  }
  else if (strcmp(arguments[0], "status") == 0 && arguments[1] == NULL){
    //printf("status stuff here \n");
    getStatus(status);
  }
  else {
    // ATTEMPT TO EXECUTE IT
    pid = fork();
    switch(pid){
      case -1:
        printf("ERROR Something went wrong with the fork.\n");
        status = 1;
        exit(1);
        break;
      case 0:
        if (bgProcess == -1){
          //printf("debug\n");
          INTsignal.sa_handler = trapSIGINTsignal;
          sigaction(SIGINT, &INTsignal, NULL);
        }
        redirect(arguments);
        execvp(arguments[0], arguments);
        printf("%s: no such file or directory.\n", arguments[0]);
        exit(1);
        break;

      default:
        if ((bgProcess != -1) && (FGonly == 0)){
          printf("Background PID: %i\n", pid);
          pushPID(pid);
        } else {
          waitpid(pid, &status, 0);
          break;
        }
    } // End switch
  } // end else
  // wait for background processes
      pid = waitpid(-1, &status, WNOHANG);
      while (pid > 0){
        printf("background process, %d, is done: ", pid);
        getStatus(status);
        popTarPID(pid);
        pid = waitpid(-1, &status, WNOHANG);
      }

  // reset the array to null values where needed.
  int c;
  for (c = 0; arguments[c] != NULL; c++){
    arguments[c] = NULL;
  }
  return 1;                                                          // If it made it this far, the loop in main will continue and this function will be called again
}

/*****************   BEGIN MAIN ***************************************************************************************************************/
int main(){
  int loopStatus = 1;

  // PIDstack is a global, init it here
  PIDstack.count = 0;
  int z;
  for (z = 0; z < 100; z++){
    PIDstack.BGpid[z] = -1;
  }

  // The actually program.
  while (loopStatus == 1){
    loopStatus = parseAndExecuteLine();
    // Before exiting, clear any BG processes
    if (loopStatus == 0){
      printf("\nDoing exit stuff, closing all processes\n");
      int k;
      for (k = 0; k < PIDstack.count; k++){
        printf("Killing Background PID: %d\n", PIDstack.BGpid[k]);
        kill(PIDstack.BGpid[k], SIGINT);
      }
    }
  } // end while

  printf("===== END PROGRAM MAIN =====\n");
  return 0;
}
/***************************************************************************************************************************************************/
