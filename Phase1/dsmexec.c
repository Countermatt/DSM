#include "common_dsmexec.h"

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <poll.h>


#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

/* variables globales */

/* un tableau gerant les infos d'identification */
/* des processus dsm */
dsm_proc_t *proc_array = NULL;

/* le nombre de processus effectivement crees */
volatile int num_procs_creat = 0;

void usage(void){
  fprintf(stdout,"Usage : dsmexec machine_file executable arg1 arg2 ...\n");
  fflush(stdout);
  exit(EXIT_FAILURE);
}

void sigchld_handler(int sig){
   /* on traite les fils qui se terminent */
   /* pour eviter les zombies */
}

int getnb_proc(char *machine_file){
  FILE* fd_machine_file;
  fd_machine_file = fopen(machine_file,"r");
  int tampon,nb_proc;

  nb_proc = 0;
  while( (tampon = fgetc(fd_machine_file))!= EOF){
    if(tampon == '\n'){
      nb_proc++;
    }
  }
  fclose(fd_machine_file);
  return nb_proc;
}

char ** getListMachine(char *machine_file,int nb_proc){
  FILE* fd_machine_file;
  int i;
  char **list_machine = malloc(nb_proc*sizeof(*list_machine));

  for ( i = 0; i < nb_proc; i++) {
    list_machine[i] = malloc(LENNAME*sizeof(*list_machine[i]));
    memset(list_machine[i],0,LENNAME);
  }

  fd_machine_file = fopen(machine_file,"r");
  for (i = 0; i < nb_proc; i++) {
    fgets(list_machine[i],LENNAME,fd_machine_file);
    int len = strlen(list_machine[i]);
    list_machine[i][len-1] = '\0';
  }
  fclose(fd_machine_file);
  return list_machine;
}

void freeListMachine(char **machine_file,int nb_proc) {
  int i;
  for ( i = 0; i < nb_proc; i++) {
    free(machine_file[i]);
  }
  free(machine_file);
}

void init_struct_pollfd(struct pollfd* fds,int nb_proc,int pipe_stdout[][2],int pipe_stderr[][2]) {
  int i;
  memset(fds,0,2*nb_proc*sizeof(*fds));
  for ( i = 0; i < nb_proc; i++) {
     fds[2*i].fd = pipe_stdout[i][0];
     fds[2*i+1].fd = pipe_stderr[i][0];
     fds[2*i].events = POLLIN;
     fds[2*i+1].events = POLLIN;
  }
}

void ecoute_des_tubes(struct pollfd *fds,int nb_proc){
  int tmp_nb_fd = 2*nb_proc,i;
   while(1){
        int res = poll(fds, 2*nb_proc, -1);
        if (res < 0) {
         perror ("poll()");
       }else if (res == 0) {
         printf("poll() timed out \n");
       }

        char buffer[1024];
        for (i = 0; i < 2*nb_proc; i++) {
          if (fds[i].fd == -1) {
            continue;
          }

          switch (fds[i].revents){
          case POLLIN:
           memset(buffer,0,1024*sizeof(*buffer));
           read(fds[i].fd,buffer,1024*sizeof(*buffer));
           if ( ( i%2) == 0) {
             fprintf(stdout,"[Proc i:%d : stdout] %s \n",i/2,buffer);
             fflush(stdout);
           }else{
             fprintf(stdout,"[Proc i:%d : stderr] %s \n",i/2,buffer);
             fflush(stdout);
           }
           break;

         case POLLHUP:
           tmp_nb_fd--;
           fds[i].fd = -1;
           break;
          }
          if (fds[i].revents== (POLLHUP|POLLIN) ) {
            memset(buffer,0,1024*sizeof(*buffer));
            read(fds[i].fd,buffer,1024*sizeof(*buffer));
            if ( ( i%2) == 0) {
              fprintf(stdout,"[Proc i:%d : stdout] %s \n",i/2,buffer);
              fflush(stdout);
            }else{
              fprintf(stdout,"[Proc i:%d : stderr] %s \n",i/2,buffer);
              fflush(stdout);
            }
            tmp_nb_fd--;
            fds[i].fd = -1;

          }
          fds[i].revents = 0;
          }
          if (tmp_nb_fd<1) {
            break;
          }
      };
}

void myprintf(char *txt){
  write(STDOUT_FILENO,txt,strlen(txt)*sizeof(char));
  fflush(stdout);
}

int main(int argc, char *argv[]){
  int i;
  if (argc < 3)
    usage();

  char str[1024];
  getcwd(str,1024);

   /* Mise en place d'un traitant pour recuperer les fils zombies*/
   /* XXX.sa_handler = sigchld_handler; */

   /* lecture du fichier de machines */
   char *machine_file = argv[1];
   char *exec = argv[2];

   /* 1- on recupere le nombre de processus a lancer */
   int nb_proc = 0;
   myprintf("on recupere le nombre de processus a lancer ...");
   nb_proc = getnb_proc(machine_file);
   myprintf("ok\n");

   /* 2- on recupere les noms des machines : le nom de */
   /* la machine est un des elements d'identification */
   myprintf("on recupere les noms des machines ...");
   char **list_machine = getListMachine(machine_file,nb_proc);
   myprintf("ok\n");

   /* creation de la socket d'ecoute */
   myprintf("creation de la socket d'ecoute...");
   fd_socket = creer_socket(&port);
   myprintf("ok\n");

   /* + ecoute effective */
   myprintf("ecoute effective de la socket" );
   if ((listen(fd_socket, nb_proc)) != 0) {
       perror("Listen failed...\n");
       exit(0);
   }
   myprintf("ok\n" );

   /* creation des fils */

   int pipe_stdout[nb_proc][2];
   int pipe_stderr[nb_proc][2];
   myprintf("creation des fils...");
   pid_t pid;

   memset(pipe_stdout,0,2*nb_proc*sizeof(*pipe_stdout));
   memset(pipe_stderr,0,2*nb_proc*sizeof(*pipe_stderr));

   proc_array = malloc(nb_proc*sizeof(*proc_array));
   memset(proc_array,0,nb_proc*sizeof(*proc_array));

   for(i = 0; i < nb_proc ; i++) {/* creation des pipes*/
      /* creation du tube pour rediriger stdout */
      /* creation du tube pour rediriger stderr */
      pipe(pipe_stdout[i]);
      pipe(pipe_stderr[i]);
      pid = fork();

      if(pid == -1) ERROR_EXIT("fork");
      if (pid == 0) { /* fils */
         /* redirection stdout */
        free(proc_array);
        close(fd_socket);
        pipe_redirection(i, pipe_stdout, pipe_stderr);

 //recuperation hostname père --------------------------------------------------
         char char_port[100];
         memset(char_port,0,100*sizeof(*char_port));
         sprintf(char_port,"%d",port);
 //ssh -------------------------------------------------------------------------

         connexion_ssh(exec, list_machine, argv, argc, i, char_port);
         /* jump to new prog : */

      } else


      if(pid > 0) { /* pere */
         /* fermeture des extremites des tubes non utiles */
         close(pipe_stdout[i][1]);
         close(pipe_stderr[i][1]);

         proc_array[i].pid = pid;
         proc_array[i].connect_info.rank = i;
         num_procs_creat++;
      }
   }
   myprintf("ok\n" );

   myprintf("recuperation de infos des client...");
   recuperation_info_tcp(nb_proc, fd_socket, proc_array);
   myprintf("ok\n");

   /* gestion des E/S : on recupere les caracteres */
   /* sur les tubes de redirection de stdout/stderr */
   struct pollfd fds[2*nb_proc];
   myprintf("initialisation du poll..." );
   init_struct_pollfd(fds,nb_proc,pipe_stdout,pipe_stderr);
   myprintf("ok\n" );

   myprintf("ecoute des tubes...\n" );
   ecoute_des_tubes(fds,nb_proc);

   /* on attend les processus fils */
   /* on ferme les descripteurs proprement */
   /* on ferme la socket d'ecoute */
   freeListMachine(list_machine,nb_proc);
   free(proc_array);
   for ( i = 0; i < nb_proc; i++) {
     wait(NULL);
   }
   exit(EXIT_SUCCESS);
}
