#include "common_dsmwrap.h"


int creer_socket(int *port){

   int sockfd;
   struct sockaddr_in servaddr;

   // socket create and verification
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   if (sockfd == -1) {
       perror("socket creation failed...\n");
       exit(0);
   }


   memset(&servaddr,0,sizeof(servaddr));

   // assign IP, PORT
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
   servaddr.sin_port = 0;
   socklen_t len =  sizeof(servaddr);
   // Binding newly created socket to given IP and verification
   if ((bind(sockfd, (struct sockaddr*)&servaddr, len) ) != 0) {
       perror("socket bind failed...\n");
       exit(0);
   }

   memset(&servaddr,0,sizeof(servaddr));
   getsockname(sockfd,(struct sockaddr*)&servaddr,&len);
   *port = ntohs(servaddr.sin_port);
   return sockfd;
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

int accept_interproc(int nb_proc, int rang,struct pollfd *fds, int serv_fd){
  if ((listen(serv_fd, rang)) != 0) {
    perror("Listen failed...\n");
    exit(0);
  }

  int i;
  for(i = 0; i < rang; i++){
    fds[i].fd = -1;
    while (fds[i].fd == -1){ //correction des problèmes de synchro
      fds[i].fd = accept(serv_fd, NULL, NULL);
    }
  }
  return EXIT_SUCCESS;
}

int connection_interproc(int nb_proc, int rang,struct pollfd *fds,dsm_proc_t *proc_array){
  int i;
  char port[10];
  for (i = rang+1; i < nb_proc; i++) {
      memset(port,0,10*sizeof(*port));
      sprintf(port,"%d",proc_array[i].connect_info.port);
      fds[i].fd = -1;
    while(fds[i].fd == -1){ //correction des problèmes de synchro
      fds[i].fd = handle_connect( proc_array[i].connect_info.hostname, port);
    }
  }
  return EXIT_SUCCESS;
}
