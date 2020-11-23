#include "tecnicofs-client-api.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>

#define ERROR -1
#define SUCCESS 0

int sockfd, ServerAddrLen, ClientAddrLen;
struct sockaddr_un server_address, client_address;

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

  unlink("client");    
  ClientAddrLen = setSocketAddrUn("client", &client_address);
  if (ClientAddrLen == 0) {
    return ERROR;
  }
  
  if (bind(sockfd, (struct sockaddr *) &client_address, ClientAddrLen) < 0) {
    return ERROR;
  }

  ServerAddrLen = setSocketAddrUn(sockPath, &server_address);
  if (ServerAddrLen == 0) {
    return ERROR;
  }

  return SUCCESS;
}

int mandaMensagem() {
  sendto(sockfd, "ohhhh nice", sizeof("ohhhh nice") + 1, 0, (struct sockaddr *) &server_address, ServerAddrLen);
  return 0;
}

int tfsUnmount() {
  return ERROR;
}
