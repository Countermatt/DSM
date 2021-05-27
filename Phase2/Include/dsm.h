#ifndef DSM_H_
#define DSM_H_

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include <string.h>


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

/* fin des includes */

#define TOP_ADDR    (0x40000000)
#define PAGE_NUMBER (100)
#define PAGE_SIZE   (sysconf(_SC_PAGE_SIZE))
#define BASE_ADDR   (TOP_ADDR - (PAGE_NUMBER * PAGE_SIZE))
int nb_proc_finish;
/* Reception de donnees  */
dsm_proc_t *proc_array;
typedef enum
{
   NO_ACCESS,
   READ_ACCESS,
   WRITE_ACCESS,
   UNKNOWN_ACCESS
}dsm_page_access_t;

typedef enum
{
   INVALID,
   READ_ONLY,
   WRITE,
   NO_CHANGE
}dsm_page_state_t;

typedef int dsm_page_owner_t;

typedef struct
{
  dsm_page_state_t status;
  dsm_page_owner_t owner;
} dsm_page_info_t;

dsm_page_info_t table_page[PAGE_NUMBER];
struct pollfd *fds;

pthread_t comm_daemon;
extern int DSM_NODE_ID;
extern int DSM_NODE_NUM;

char *dsm_init( int argc, char **argv);
void  dsm_finalize(void);

dsm_page_owner_t get_owner( int numpage);
dsm_page_state_t get_status( int numpage);
void dsm_change_info( int numpage, dsm_page_state_t state, dsm_page_owner_t owner);
void dsm_protect_page( int numpage , int prot);
void dsm_alloc_page( int numpage );
void dsm_free_page( int numpage );
char *num2address( int numpage );

int connection_interproc(int nb_proc, int rang,struct pollfd *fds,dsm_proc_t *proc_array);
int accept_interproc(int nb_proc, int rang,struct pollfd *fds, int serv_fd);
#endif
