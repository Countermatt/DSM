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

void pipe_redirection(int pipenumber, int pipe_stdout[][2] ,int pipe_stderr[][2]);

void ssh_arg_config(char *exec, char **list_machine, char *argv[], int argc, int i, char char_port[100]);

void recuperation_info_tcp(int nb_proc, int fd_socket, dsm_proc_t *proc_array);
