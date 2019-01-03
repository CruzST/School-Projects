/***************************************************
** Name: Steven Cruz
** Date: 11/30/2018
** Description: file for the encoding portion
**              of the project, server side
** Version: Final Draft
*****************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_SIZE 200000

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues


// convert a char to an int
int charToInt(char letter){
  // 0 thru 25 are a thru z, space is 26 for 27chars total
  int asciiNum = letter;
  if (asciiNum == 32){
    return 26;
  }
  int letterToInt = asciiNum - 65;
  return letterToInt;
}

// convert an int to a char
char intToChar(int number){
  // 0 thru 25 are a thru z, space is 26 for 27chars total
  if (number == 26){
    return ' ';
  }
  char numToChar = number + 65;
  return numToChar;
}

// encryption function
void encryptText(char text[], char key[]){
    int i;
    int textInt;
    int keyInt;
    int encryptedTextInt;
    int length = (strlen(text)-1);

    for (i = 0; i < length; i++){
        textInt = charToInt(text[i]);
        keyInt = charToInt(key[i]);

        encryptedTextInt = (textInt + keyInt) % 27;
        text[i] = intToChar(encryptedTextInt);
    }
    text[i] = '\0';
    return;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]){
    int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
    socklen_t sizeOfClientInfo;
    char buffer[MAX_SIZE];
    char clientType;
    char serverResponse;
    struct sockaddr_in serverAddress, clientAddress;
    int value = 1;
    pid_t pid;


    if(argc != 2){
        fprintf(stderr, "Error: number of arguments invalid\n");
        exit(1);
    }

    // Set up the socket
    listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocketFD < 0) error("ERROR opening socket");


    setsockopt(listenSocketFD, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(int));                        // From class notes

    memset((char *)&serverAddress, '\0', sizeof(serverAddress));                                      // Clear out the address struct
    portNumber = atoi(argv[1]);                                                                       // Get the port number, convert to an integer from a string
    serverAddress.sin_family = AF_INET;                                                               // Create a network-capable socket
    serverAddress.sin_port = htons(portNumber);                                                       // Store the port number
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // bind
    if(bind(listenSocketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0){
        error("ERROR on binding");
        exit(1);
    }

    // listen
    listen(listenSocketFD, 5);

    // begin the process
    while(1){
        // Accept a connection, blocking if one is not available until one connects
        sizeOfClientInfo = sizeof(clientAddress);
        establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *) &clientAddress, &sizeOfClientInfo);
        if(establishedConnectionFD < 0){
            error("ERROR on accept");
            exit(1);
        }
        pid = fork();
        if(pid < 0){
            error("ERROR on fork");
            exit(1);
        }
        if(pid == 0){
            memset(buffer, '\0', sizeof(buffer));
            int itr;
            int flag = 0;
            char* key;
            int nleft = sizeof(buffer);
            int nread = 0;


            // receive check from client and return response
            recv(establishedConnectionFD, buffer, sizeof(buffer) - 1, 0);
            if(strcmp(buffer, "e") != 0){
                char output[] = "n";
                send(establishedConnectionFD, output, sizeof(output), 0);
                exit(2);
            }
            else{
                char output[] = "y";
                send(establishedConnectionFD, output, sizeof(output), 0);
            }
            memset(buffer, '\0', sizeof(buffer));
            char* buffPtr = buffer;
            // https://stackoverflow.com/questions/7011165/how-do-i-send-10-000-20-000-bytes-data-over-tcp
            //printf("attempting to read from client\n");
            while(1){
                nread = recv(establishedConnectionFD, buffPtr, nleft, 0);
                if (nread < 0){
                    error("error reading from client\n");
                }
                //printf("nleft: %d\n", nleft);
                if(nleft == 0){
                    error("Buffer not large enough.");
                }
                /*
                if (nread == 0){
                    printf("0 bytes read.\n");
                }
                */
                for(itr = 0; itr < nread; itr++){
                    if(buffPtr[itr] == '\n'){
                        flag++;
                        if(flag == 1){
                            key = buffPtr + itr + 1;
                        }
                    }
                }
                if(flag == 2){
                    //printf("keysize: %d\n", (buffPtr + itr - key));
                    break;
                }
                nleft -= nread;
                buffPtr += nread;
            }

            // Prepare the message for response to the client
            int responseSent;
            //printf("attempting to send response to client\n");
            char message[MAX_SIZE];
            //printf("calling memset\n");
            memset(message, '\0', sizeof(message));
            //printf("calling strncpy\n");
            //printf("size of buffer: %d\n", sizeof(buffer));
            //printf("size of key-buffer: %d\n", key-buffer);
            strncpy(message, buffer, key-buffer);
            //printf("calling encryptText\n");
            encryptText(message, key);
            //printf("calling sending\n");
            responseSent = send(establishedConnectionFD, message, sizeof(message), 0);
            // printf("value of responseSent: %d\n", responseSent);
            //printf("after sending\n");
            if (responseSent < 0){
                error("SERVER ERROR: sending response error.");
            }
            //printf("response sent\n");
        }
        close(establishedConnectionFD);
    }
    close(listenSocketFD);
    return 0;
}
