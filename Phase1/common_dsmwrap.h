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


/* autres includes (eventuellement) */
#define LENNAME 256
#define ERROR_EXIT(str) {perror(str);exit(EXIT_FAILURE);}

/* definition du type des infos */
/* de connexion des processus dsm */
struct dsm_proc_conn  {
   int rank;
   char hostname[LENNAME];
   int port;
   /* a completer */
};
typedef struct dsm_proc_conn dsm_proc_conn_t;

/* definition du type des infos */
/* d'identification des processus dsm */
struct dsm_proc {
  pid_t pid;
  dsm_proc_conn_t connect_info;
};
typedef struct dsm_proc dsm_proc_t;

int creer_socket(int *port);

int handle_connect(const char* addr,const	char* port);

int get_rang(dsm_proc_t *proc_array, int nb_proc, int pid);

int accept_interproc(int nb_proc, int rang,struct pollfd *fds, int serv_fd);

int connection_interproc(int nb_proc, int rang,struct pollfd *fds,dsm_proc_t *proc_array);
