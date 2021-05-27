#include "common.h"

void myprintf(char *txt){
  write(STDOUT_FILENO,txt,strlen(txt)*sizeof(char));
  fflush(stdout);
}

int handle_connect(const char* addr,const	char* port) {
	struct addrinfo hints, *result;
	int sfd;
	memset(&hints, 0, sizeof(struct addrinfo));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	int error_getaddrinfo = getaddrinfo(addr, port, &hints, &result);
	if (error_getaddrinfo != 0) {
		printf("getaddrinfo:%s\n",gai_strerror(error_getaddrinfo));
		return -1;
	}

	// Creation de la socket
	sfd = socket(result->ai_family, result->ai_socktype,result->ai_protocol);
	if (sfd == -1) {
		perror("socket");
		return -1;
	}

	//Connexion de la socket au serveur
	if (connect(sfd, result->ai_addr, result->ai_addrlen) == -1) {
		perror("connect");
		return -1;
	}
	// liberation de la memoire allouer
	freeaddrinfo(result);
	fflush(stderr);
	fflush(stdout);

	return sfd;
}

int get_rang(dsm_proc_t *proc_array, int nb_proc, int pid){
  int i = 0;
  int rang = -1;
  while(i<nb_proc){
    if(proc_array[i].pid == pid){
      rang = proc_array[i].connect_info.rank;
      break;
    }
    i++;
  }
  return rang;
}
