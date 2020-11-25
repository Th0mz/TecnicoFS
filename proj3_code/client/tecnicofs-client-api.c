#include "tecnicofs-client-api.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>

#define ERROR -1
#define SUCCESS 0

int sockfd;

socklen_t clientAddrLen;
struct sockaddr_un client_address; 

socklen_t serverAddrLen;
struct sockaddr_un server_address;

int setSocketAddrUn(char *path, struct sockaddr_un *address) {
    
    if (address == NULL) 
        return 0;
    
    bzero((char *) address, sizeof(struct sockaddr_un));
    address->sun_family = AF_UNIX;
    strcpy(address->sun_path, path);
    
    return SUN_LEN(address);
}

void sendCommand(char *command, int commandLen) {
  sendto(sockfd, command, commandLen, 0, (struct sockaddr *) &server_address, serverAddrLen);
}

int receiveStatus() {
  int status, statusLen;

  statusLen = recvfrom(sockfd, &status, sizeof(status), 0, 0, 0);

  if (statusLen < 0)
      return ERROR;

  return status;
}

int tfsCreate(char *filename, char nodeType) {
  char command[MAX_INPUT_SIZE];
  int commandLen;

  /*
    Concatenates all requirements to a commandline
    to execute the "create" Operation
  */
  sprintf(command, "c %s %c", filename, nodeType);
  commandLen = strlen(command);

  sendCommand(command, commandLen);
  return receiveStatus();
}

int tfsDelete(char *path) {
  char command[MAX_INPUT_SIZE];
  int commandLen;

  /*
    Concatenates all requirements to a commandline
    to execute the "delete" Operation
  */
  sprintf(command, "d %s", path);
  commandLen = strlen(command);
  
  sendCommand(command, commandLen);
  return receiveStatus();
}

int tfsMove(char *from, char *to) {
  char command[MAX_INPUT_SIZE];
  int commandLen;

  /*
    Concatenates all requirements to a commandline
    to execute the "move" Operation
  */
  sprintf(command, "m %s %s", from, to);
  commandLen = strlen(command);
  
  sendCommand(command, commandLen);
  return receiveStatus();
}

int tfsLookup(char *path) {
  char command[MAX_INPUT_SIZE];
  int commandLen;

  /*
    Concatenates all requirements to a commandline
    to execute the "lookup" Operation
  */
  sprintf(command, "l %s", path);
  commandLen = strlen(command);
  
  sendCommand(command, commandLen);
  return receiveStatus();
}

int tfsPrintTree(char *outputfile) {
  char command[MAX_INPUT_SIZE];
  int commandLen;

  /*
    Concatenates all requirements to a commandline
    to execute the "printTree" Operation
  */
  sprintf(command, "p %s", outputfile);
  commandLen = strlen(command);
  
  sendCommand(command, commandLen);
  return receiveStatus();
}


int tfsMount(char * sockPath) {
  char clientTempPath[MAX_FILE_NAME];

  if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
    return ERROR;
  }

  /* Create a temporary path for client */
  sprintf(clientTempPath, "/tmp/%d", getpid());
  unlink(clientTempPath);  

  clientAddrLen = setSocketAddrUn(clientTempPath, &client_address);
  if (clientAddrLen == 0) {
    return ERROR;
  }


  if (bind(sockfd, (struct sockaddr *) &client_address, clientAddrLen) < 0) {
    return ERROR;
  }

  serverAddrLen = setSocketAddrUn(sockPath, &server_address);
  if (serverAddrLen == 0) {
    return ERROR;
  }

  return SUCCESS;
}

int tfsUnmount() {
  return ERROR;
}