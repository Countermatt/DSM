#include "common_dsmexec.h"

#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
int creer_socket(int *port){

   int sockfd;
   struct sockaddr_in servaddr;

   // socket create and verification
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   if (sockfd == -1) {
       perror("socket creation failed...");
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
       perror("socket bind failed...");
       exit(0);
   }

   memset(&servaddr,0,sizeof(servaddr));
   getsockname(sockfd,(struct sockaddr*)&servaddr,&len);
   *port = ntohs(servaddr.sin_port);
   return sockfd;
}

void pipe_redirection(int pipenumber, int pipe_stdout[][2] ,int pipe_stderr[][2]){

  int k;
  for ( k = 0; k < pipenumber; k++) {
    close(pipe_stdout[k][0]);
    close(pipe_stderr[k][0]);
  }
  /* redirection stderr */
  close(pipe_stderr[pipenumber][0]);
  close(STDERR_FILENO);
  dup(pipe_stderr[pipenumber][1]);
  close(pipe_stderr[pipenumber][1]);


  close(pipe_stdout[pipenumber][0]);
  close(STDOUT_FILENO);
  dup(pipe_stdout[pipenumber][1]);
  close(pipe_stdout[pipenumber][1]);

}

void ssh_arg_config(char *exec, char **list_machine, char *argv[], int argc, int ind, char char_port[100]){
  int k;
//Path de l'executable-------------------------------------------
  char str[1024];
  char *path = NULL;
  path = getcwd(str,1024);

//Hostname du père-----------------------------------------------
  char hostname[256] ;
  gethostname(hostname,256);


//clé ssh privé du père------------------------------------------
  char id_rsa[strlen(path)+strlen("/..")+strlen("/.ssh/id_rsa")+10];
  memset(id_rsa,0,sizeof(id_rsa));
  strcat(id_rsa, path);
  strcat(id_rsa, "/..");
  strcat(id_rsa, "/.ssh/id_rsa");


//commande à lancer----------------------------------------------
  char script[strlen(path)+strlen("/bin/dsmwrap")+10];
  memset(script,0,sizeof(script));
  strcat(script, path);
  strcat(script, "/bin/dsmwrap");


//chemin du script à lancer--------------------------------------
  char full_path_exec[strlen(path)+strlen(exec)+10];
  memset(full_path_exec,0,sizeof(full_path_exec));
  if (exec[0] == '/'){
    strcat(full_path_exec, path);
    strcat(full_path_exec, exec);
  }
  else{
    strcat(full_path_exec, path);
    strcat(full_path_exec, exec +1);
  }



//argument à passer au script lancer-----------------------------
  int args_len = 0;
  for (k = 3; k<argc; k++){
    args_len += strlen(argv[k]);
  }

  char args[args_len + argc + 1];
  memset(args,0,sizeof(args));
  for (k = 3; k<argc; k++){
    strcat(args, argv[k]);
    strcat(args, " ");
  }

int pid = getpid();
char spid[100];
memset(spid,0,sizeof(spid));
sprintf(spid, "%d", pid);
//argument final de la commande ssh------------------------------

char *ssh_arg[11];
ssh_arg[0] = "ssh";
ssh_arg[1] = list_machine[ind];
ssh_arg[2] =  "-i";
ssh_arg[3] = id_rsa;
ssh_arg[4] = script; // ./bin/dsmwrap
ssh_arg[5] = hostname;
ssh_arg[6] = char_port;
ssh_arg[7] = spid;
ssh_arg[8] = full_path_exec;
ssh_arg[9] = args;
ssh_arg[10] = NULL;
  // char *ssh_arg[] = {"ssh", list_machine[i], "-i", id_rsa, script, hostname, char_port, full_path_exec, args, NULL};

  // execvp(script,ssh_arg+4);
  execvp("ssh",ssh_arg);
}

void recuperation_info_tcp(int nb_proc, int fd_socket, dsm_proc_t *proc_array){
  int sockFdProc[nb_proc];
  memset(sockFdProc,-1,nb_proc*sizeof(*sockFdProc));
  int i;
  for(i = 0; i < nb_proc ; i++){/* recuperation des donnes*/

    /* on accepte les connexions des processus dsm */
    int len;
    struct sockaddr_in proc_addr;
    len = sizeof(proc_addr);
    sockFdProc[i] = accept(fd_socket,(struct sockaddr*)&proc_addr,(socklen_t *)&len);
    if (sockFdProc[i] < 0) {
        perror("server acccept failed...\n");
        exit(0);
    }

   /*  On recupere le nom de la machine distante */
   /* 1- d'abord la taille de la chaine */
   int hostname_len;
   char hostname[LENNAME];
   memset(hostname,0,LENNAME*sizeof(*hostname));
   read(sockFdProc[i],&hostname_len,sizeof(hostname_len));
   read(sockFdProc[i],hostname,hostname_len*sizeof(*hostname));

   /* 2- puis la chaine elle-meme */
   /* On recupere le pid du processus distant  */

   /* On recupere le numero de port de la socket */
   /* d'ecoute des processus distants */

   int proc_pid,proc_port;
   read(sockFdProc[i],&proc_pid, sizeof(proc_pid));
   read(sockFdProc[i],&proc_port, sizeof(proc_port));

   int k;
   for ( k = 0; k < nb_proc; k++) {
     if ( proc_array[k].pid == proc_pid) {
       strcpy(proc_array[k].connect_info.hostname,hostname);
       proc_array[k].connect_info.port = proc_port;
       break;
     }
   }
  }

  /* envoi du nombre de processus aux processus dsm*/
  /* envoi des rangs aux processus dsm */
  /* envoi des infos de connexion aux processus */
  for (i = 0; i < nb_proc; i++) {
    if ( sockFdProc[i] != -1 ) {
      write( sockFdProc[i],&nb_proc,sizeof(nb_proc) );
      write( sockFdProc[i],proc_array,nb_proc*sizeof(*proc_array) );
    }
  }
}

/* Vous pouvez ecrire ici toutes les fonctions */
/* qui pourraient etre utilisees par le lanceur */
/* et le processus intermediaire. N'oubliez pas */
/* de declarer le prototype de ces nouvelles */
/* fonctions dans common_impl.h */
