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

void sendCommand(char *command) {
  sendto(sockfd, command, sizeof(command) + 1, 0, (struct sockaddr *) &server_address, serverAddrLen);
}

int receiveStatus() {
  char status[MAX_INPUT_SIZE];
  int statusLen;

  printf("waiting for server to respond\n");
  statusLen = recvfrom(sockfd, status, sizeof(status) - 1, 0, 0, 0);
  printf("parou o wait\n");
  
  if (statusLen < 0)
      return ERROR;

  status[statusLen] = '\0';
  printf("client : %s\n", status);
  if (strcmp(status, "0") == 0)
    return SUCCESS;
  else 
    return ERROR;

}

int tfsCreate(char *filename, char nodeType) {
  char command[MAX_INPUT_SIZE];

  /*
    Concatenates all requirements to a commandline
    to execute the "create" Operation
  */
  sprintf(command, "c %s %c", filename, nodeType);
  
  sendCommand(command);
  return receiveStatus();
}

int tfsDelete(char *path) {
  return ERROR;
}

int tfsMove(char *from, char *to) {
  return ERROR;
}

int tfsLookup(char *path) {
  return ERROR;
}

int tfsMount(char * sockPath) {
  if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
    return ERROR;
  }

  unlink("/tmp/client");    
  clientAddrLen = setSocketAddrUn("/tmp/client", &client_address);
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