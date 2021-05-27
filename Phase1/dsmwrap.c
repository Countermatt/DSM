#include "common_dsmwrap.h"

#include <sys/types.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


int main(int argc, char **argv){
   /* processus intermediaire pour "nettoyer" */
   /* la liste des arguments qu'on va passer */
   /* a la commande a executer vraiment */

   int sockfd,serv_fd,hostname_len;
   char *ip = argv[1];     // hostname de dswexec
   char *port = argv[2];   // port de dswexec
	 char *spid = argv[3];   // pid du processus fork qui a excuter le dsm dsmwrap
   char *exec = argv[4];   // Non du fichier a excuter
   char **arg = argv+4;    // Arguments du pour le programmes a excuter
   char hostname[LENNAME];

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
   int serv_port;
   serv_fd = creer_socket(&serv_port);
   /* Envoi du numero de port au lanceur */
   /* pour qu'il le propage à tous les autres */
   /* processus dsm */
   write(sockfd,&serv_port, sizeof(serv_port));
   execvp(exec,arg);
   return 0;
}
