/** Name: Steven Cruz
 ** Date: 11/28/2018
 ** Description: keygenerator
 ** Version: Passed first draft, complete?
**/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>






int main(int argc, char* argv[]){

  if (argc < 2){                                                    // Make sure an arg was passed with keygen
    fprintf(stderr, "Error: A valid second argument not found. Exiting keygen.\n");
    exit(0);
  }

  srand(time(NULL));
  int randoCharInt;
  int i;
  int length = atoi(argv[1]);


                                                                    // random formula is: rand() % (max_number + 1 - minimum_number) + minimum_number
  for (i = 0; i < length; i++){
    if ((rand() % (10) + 1) == 8){
      randoCharInt = 32;                                           // ASCII FOR SPACE
    } else {
      randoCharInt = rand() % (90 + 1 - 65) + 65;                  // 65 to 90 are the capital letters.
    }
    fprintf(stdout, "%c", randoCharInt);
  }
  fprintf(stdout, "\n");
  return 0;
} // END MAIN
