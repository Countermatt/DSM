#ifndef COMMON_H_
#define COMMON_H_


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <poll.h>

#include "dsm.h"

#define SOCKFD (3);      // fd socket de dsmexec
#define SERV_FD (4)    // fd socket serveur TCP
pthread_mutex_t verrou;
void myprintf(char *txt);
int handle_connect(const char* addr,const	char* port);

#endif
