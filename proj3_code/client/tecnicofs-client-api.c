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

int tfsCreate(char *filename, char nodeType) {
  return ERROR;
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

int mandaMensagem() {
  sendto(sockfd, "ohh nice", sizeof("ohh nice") + 1, 0, (struct sockaddr *) &server_address, serverAddrLen);
  return 0;
}