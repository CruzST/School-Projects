/***************************************************
** Name: Steven Cruz
** Date: 11/30/2018
** Description: file for the encoding portion
**              of the project, client side
** Version: Final Draft
*****************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include<sys/stat.h>

#define MAX_SIZE 200000


// Function was provided with the client.c base
void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues


// Function to send packet to server.
// Originally was 2 differentfunction, but caused unknown errors and would
// not send 100% of the time
void openAndSend(char* filename, int socketFD, int length){
    FILE* fileToSend = fopen(filename, "r");
    char buffer[MAX_SIZE];
    bzero(buffer, MAX_SIZE);
    int nread;
    //https://stackoverflow.com/questions/7011165/how-do-i-send-10-000-20-000-bytes-data-over-tcp
    // freads grabs the \n in the file as well?
    // //https://stackoverflow.com/questions/13656702/sending-and-receiving-strings-over-tcp-socket-separately/13656795#13656795
    //printf("OPEN AND SEND: Starting while loop in openandsend\n");
    while((length = fread(buffer, sizeof(char), MAX_SIZE, fileToSend)) > 0){
        if((nread = send(socketFD, buffer, length, 0)) < 0){
        	perror("Error in sending");
            break;
        }
        memset(buffer, '\0', sizeof(buffer));
    }
    if(nread == MAX_SIZE){
        send(socketFD, "0", 1, 0);              // If max is reached
    }
    fclose(fileToSend);
    //printf("OPEN AND SEND: done with while, file closed\n");
    return;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]){
    int socketFD, portNumber, keyWritten, textWritten, status, value;
    struct sockaddr_in serverAddress;
    struct hostent* serverHostInfo;
    char buffer[MAX_SIZE];
    char clientType[] = "e";
    value = 1;
    memset(buffer, '\0', sizeof(buffer));

    if (argc != 4) { fprintf(stderr,"Error: Four Agruments were not provided\n"); exit(0); }           // Check 4 args were passed.

    // Set up the server address struct
    memset((char*)&serverAddress, '\0', sizeof(serverAddress));                                       // Clear out the address struct
    portNumber = atoi(argv[3]);                                                                       // Get the port number, convert to an integer from a string
    serverAddress.sin_family = AF_INET;                                                               // Create a network-capable socket
    serverAddress.sin_port = htons(portNumber);                                                       // Store the port number
    serverHostInfo = gethostbyname("localhost");                                                      // Convert the machine name into a special form of address
    if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
    memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

    // Set up the socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
    if (socketFD < 0) error("CLIENT: ERROR opening socket");

    setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(int));

    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)               // Connect socket to address
        error("CLIENT: ERROR connecting");

    //Authentication
    send(socketFD, clientType, sizeof(clientType), 0);
    recv(socketFD, buffer, sizeof(buffer), 0);
    if(strcmp(buffer, "y") != 0){
        fprintf(stderr, "Error, could not contact otp_enc_d on port %d\n", portNumber);
        exit(2);
    }

    // Get the keys size from the file arg[2]                                                         // https://stackoverflow.com/questions/23306552/getting-a-files-size-in-c-with-lseek
    char* keyName = argv[2];
    int keyfd = open(keyName, O_RDONLY);                                                              // create a file descriptor to pass into lseek
    if (keyfd < 0) { fprintf(stderr,"Error: cannot open %s\n", keyName); exit(1); }
    int keySize = lseek(keyfd, 0, SEEK_END);                                                          // INCLUDES THE '\0' at the end.
    close(keyfd);

    // Get the text size from the file arg[1]
    char* textName = argv[1];
    int textfd = open(textName, O_RDONLY);
    if (textfd < 0) { fprintf(stderr,"Error: cannot open %s\n", textName); exit(1); }
    int textSize = lseek(textfd, 0, SEEK_END);
    close(textfd);

    // Make sure the key is big enough.
    if (textSize > keySize){
        fprintf(stderr, "Error: key '%s' is too short.\n", argv[2]);
        exit(1);
    }

    // Check symbol validity
    int letter = open(textName, 'r');
    while(read(letter, buffer, 1) != 0){                                                            // Read until end of file
        if(isspace(buffer[0]) || isupper(buffer[0])){
        }
        else{
            fprintf(stderr, "Error: input contains bad characters.\n");
    		exit(1);
        }
    }

    memset(buffer, '\0', sizeof(buffer));

    openAndSend(textName, socketFD, textSize);  //Send text and key
    openAndSend(keyName, socketFD, keySize);

    status = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
    if(status < 0){ error("Error from reading socket.");}
    fprintf(stdout, "%s\n", buffer);

    close(socketFD);
    return 0;
}
