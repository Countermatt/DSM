#include "common_dsmwrap.h"

#include <sys/types.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


int main(int argc, char **argv)
{
   /* processus intermediaire pour "nettoyer" */
   /* la liste des arguments qu'on va passer */
   /* a la commande a executer vraiment */
   // if (argc != 3) {
   //   printf("Usage: <addr IP or hostname> <port>\n");
   //   exit(EXIT_FAILURE);
   // }
   int sockfd,serv_port,serv_fd,hostname_len;
   char *ip = argv[1];     // hostname de dswexec
   char *port = argv[2];   // port de dswexec
	 char *spid = argv[3];   // pid du processus fork qui a excuter le dsm dsmwrap
   char *exec = argv[4];   // Non du fichier a excuter
   char **arg = argv+4;    // Arguments du pour le programmes a excuter
   char hostname[LENNAME];
// int i;
// printf("*************************%s\n",pid );
// for ( i = 0; i < argc; i++) {
// 	printf("-----%s\n",argv[i] );
// }
   /* creation d'une socket pour se connecter au */
   /* au lanceur et envoyer/recevoir les infos */
   /* necessaires pour la phase dsm_init */
   sockfd = handle_connect(ip,port);

   /* Envoi du nom de machine au lanceur */
   memset(hostname,0,LENNAME*sizeof(*hostname));
   gethostname(hostname,LENNAME);
   hostname_len = strlen(hostname);
   write(sockfd,&hostname_len, sizeof(hostname_len));
   write(sockfd,hostname, hostname_len*sizeof(*hostname));


   /* Envoi du pid au lanceur */
	 int pid = atoi(spid);
   write(sockfd,&pid,sizeof(pid));

   /* Creation de la socket d'ecoute pour les */
   /* connexions avec les autres processus dsm */
   serv_fd = creer_socket(&serv_port);

   /* Envoi du numero de port au lanceur */
   /* pour qu'il le propage à tous les autres */
   /* processus dsm */
   write(sockfd,&serv_port, sizeof(serv_port));

   /* Reception de donnees  */

   int nb_proc;
   read(sockfd,&nb_proc,sizeof(nb_proc));

   dsm_proc_t proc_array[nb_proc];
   read(sockfd,proc_array,nb_proc*sizeof(*proc_array));
   /* on execute la bonne commande */

   int i;
   for (i = 0; i < nb_proc; i++) {
     printf("%d [%d] rank: %d|port :%d|name :%s\n",getpid(),i,proc_array[i].connect_info.rank,proc_array[i].connect_info.port,proc_array[i].connect_info.hostname);
   }

   int rang = get_rang(proc_array, nb_proc, pid);

   struct pollfd fds[nb_proc];
   memset(&fds,0,sizeof(fds));
   accept_interproc(nb_proc, rang, fds, serv_fd);
   connection_interproc(nb_proc, rang, fds, proc_array);
   execvp(exec,arg);
   return 0;
}
