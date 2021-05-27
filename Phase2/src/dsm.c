#include "dsm.h"
#include "common.h"
#include "gestionMessage.h"


int DSM_NODE_NUM; /* nombre de processus dsm */
int DSM_NODE_ID;  /* rang (= numero) du processus */
volatile int CHAT_ON = 1;

/* indique l'adresse de debut de la page de numero numpage */
char *num2address( int numpage ){
   char *pointer = (char *)(BASE_ADDR+(numpage*(PAGE_SIZE)));

   if( pointer >= (char *)TOP_ADDR ){
      fprintf(stderr,"[%i] Invalid address !\n", DSM_NODE_ID);
      return NULL;
   }
   else return pointer;
}

int address2num( char *addr ){
  return (((long int)(addr - BASE_ADDR))/(PAGE_SIZE));
}

/* fonctions pouvant etre utiles */
void dsm_change_info( int numpage, dsm_page_state_t state, dsm_page_owner_t owner){
   if ((numpage >= 0) && (numpage < PAGE_NUMBER)) {
	if (state != NO_CHANGE )
	table_page[numpage].status = state;
      if (owner >= 0 )
	table_page[numpage].owner = owner;
      return;
   }else {
	    fprintf(stderr,"[%i] Invalid page number !\n", DSM_NODE_ID);
   return;
   }
}

dsm_page_owner_t get_owner( int numpage){
   return table_page[numpage].owner;
}

dsm_page_state_t get_status( int numpage){
   return table_page[numpage].status;
}

/* Allocation d'une nouvelle page */
void dsm_alloc_page( int numpage ){
   char *page_addr = num2address( numpage );
   mmap(page_addr, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   return ;
}

/* Changement de la protection d'une page */
void dsm_protect_page( int numpage , int prot){
   char *page_addr = num2address( numpage );
   mprotect(page_addr, PAGE_SIZE, prot);
   return;
}

void dsm_free_page( int numpage ){
   char *page_addr = num2address( numpage );
   munmap(page_addr, PAGE_SIZE);
   return;
}

static void *dsm_comm_daemon( void *arg){
   int i;
   sigset_t masque,old;
   sigemptyset(&masque);
   sigaddset(&masque, SIGSEGV);
   pthread_sigmask(SIG_SETMASK, &masque, &old);

   while(CHAT_ON){
	    /* a modifier */
      if(poll(fds,DSM_NODE_NUM,-1)>0){
        pthread_mutex_lock(&verrou);
        if(poll(fds,DSM_NODE_NUM,0)>0){
          for (i = 0; i < DSM_NODE_NUM; i++) {
            if (fds[i].revents==0) {
              continue;
            }
            switch (fds[i].revents) {
              case POLLIN:
              fflush(stdout);
              if(messageRecv(fds[i].fd)==-1){
                fds[i].fd =-1;
              }
              break;

              case POLLHUP:
              fds[i].fd =-1;
              break;
              // case POLLIN|POLLHUP:
              // break;
            }
            fds[i].revents = 0;
          }
        }
        pthread_mutex_unlock(&verrou);
        }
     }
     fflush(stdout);
     fflush(stderr);
   return NULL;
}

static int dsm_recv(int dest,int numero_page){
  dsm_alloc_page(numero_page);
  char* pointer = num2address( numero_page );
  int toRecv = PAGE_SIZE;
  int recv = 0;
  do {
    recv += read(dest,pointer+recv,toRecv);
    toRecv -= recv;

  } while(toRecv!=0);
  dsm_change_info(numero_page,WRITE,DSM_NODE_ID);
  close(dest);
  return 0;
}

int send_WRITE_REQUEST(struct message msg){
   struct pollfd tmp_fds[1];
   struct message Nmsg;
    tmp_fds[0].fd = SERV_FD;
    tmp_fds[0].events = POLLIN;
    tmp_fds[0].revents = 0;

    int send_msg=1,i;
    int page_owner_fd;

    if(poll(fds,DSM_NODE_NUM,0)>0){
      for (i = 0; i < DSM_NODE_NUM; i++) {
        if (fds[i].revents==0) {
          continue;
        }
        switch (fds[i].revents) {
          case POLLIN:
          if(messageRecv(fds[i].fd)==-1){
            fds[i].fd =-1;
          }
          break;

          case POLLHUP:
          fds[i].fd =-1;
          break;
        }
        fds[i].revents = 0;
      }
    }

    while (send_msg) {
      int i;
      for ( i = 1; i < DSM_NODE_NUM; i++) {
        write(fds[i].fd,&msg,sizeof(msg));
      }

      if(poll(fds,DSM_NODE_NUM,100)>0){
        for (i = 0; i < DSM_NODE_NUM; i++) {
          if (fds[i].revents==0) {
            continue;
          }else{
          fds[i].revents=0;

          memset(&Nmsg,0,sizeof(Nmsg));
          if(read(fds[i].fd,&Nmsg,sizeof(Nmsg))>0){
            if (Nmsg.type == LOCALISATION_YES) {
              Nmsg = messageCreate(STATUS_ACCEPT,msg.page_number);
              write(fds[i].fd,&Nmsg,2*sizeof(Nmsg));
              if(poll(tmp_fds,1,100)>0){
                tmp_fds[0].revents = 0;
                page_owner_fd = accept(SERV_FD,NULL,NULL);
                send_msg = 0;
              }else{
              }
            }else{
              traitementMsg(Nmsg,fds[i].fd);
            }
          }else{
          }
        }
      }
    }else{
    }
    }
    return page_owner_fd;
}

static void dsm_handler(int numero_page){
   int page_owner_fd;
   struct message msg = messageCreate(WRITE_REQUEST,numero_page);
   pthread_mutex_lock(&verrou);
   page_owner_fd = send_WRITE_REQUEST(msg);
   dsm_recv(page_owner_fd,numero_page);
   close(page_owner_fd);
   pthread_mutex_unlock(&verrou);
}

/* traitant de signal adequat */
static void segv_handler(int sig, siginfo_t *info, void *context){
   /* A completer */
   /* adresse qui a provoque une erreur */
   void  *addr = info->si_addr;
  /* Si ceci ne fonctionne pas, utiliser a la place :*/
  /*
   #ifdef __x86_64__
   void *addr = (void *)(context->uc_mcontext.gregs[REG_CR2]);
   #elif __i386__
   void *addr = (void *)(context->uc_mcontext.cr2);
   #else
   void  addr = info->si_addr;
   #endif
   */
   /*
   pour plus tard (question ++):
   dsm_access_t access  = (((ucontext_t *)context)->uc_mcontext.gregs[REG_ERR] & 2) ? WRITE_ACCESS : READ_ACCESS;
  */
   /* adresse de la page dont fait partie l'adresse qui a provoque la faute */
   void  *page_addr  = (void *)(((unsigned long) addr) & ~(PAGE_SIZE-1));
   if ((addr >= (void *)BASE_ADDR) && (addr < (void *)TOP_ADDR)){
       int numero_page   = address2num(page_addr);
       dsm_handler(numero_page);
   }
   else{
	/* SIGSEGV normal : ne rien faire*/
   }
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

int accept_interproc(int nb_proc, int rang,struct pollfd *fds, int serv_fd){
  if ((listen(serv_fd, rang)) != 0) {
    perror("Listen failed...\n");
    fflush(stderr);
    fflush(stdout);
    exit(0);
  }
  int i;
  for(i = 1; i < rang+1; i++){
    fds[i].fd = -1;
    while (fds[i].fd == -1){ //correction des problèmes de synchro
      fds[i].fd = accept(serv_fd, NULL, NULL);

    }
  }
  return EXIT_SUCCESS;
}

struct pollfd *init_poll(int nb_proc) {
  struct pollfd *tmp_fds;
  tmp_fds = malloc(sizeof(*tmp_fds)*nb_proc);
  memset(tmp_fds,0,sizeof(*tmp_fds)*nb_proc);

  tmp_fds[0].fd = -1;
  tmp_fds[0].events   = POLLIN;
  tmp_fds[0].revents  = 0;

  int i;
  for ( i = 1; i < nb_proc; i++) {
    tmp_fds[i].fd = -1;
    tmp_fds[i].events   = POLLIN;
    tmp_fds[i].revents  = 0;
  }
  return tmp_fds;
}

/* Seules ces deux dernieres fonctions sont visibles et utilisables */
/* dans les programmes utilisateurs de la DSM                       */
char *dsm_init(int argc, char **argv){
   pthread_mutex_init(&verrou,NULL);
   struct sigaction act;
   int index;
   int sockfd   = SOCKFD;      // fd socket de dsmexec
   int serv_fd  = SERV_FD;    // fd socket serveur TCP


   /* reception de mon numero de processus dsm envoye */
   /* par le lanceur de programmes (DSM_NODE_ID)*/
   int rang;
   read(sockfd,&rang,sizeof(rang));
   DSM_NODE_ID = rang;

   /* reception du nombre de processus dsm envoye */
   /* par le lanceur de programmes (DSM_NODE_NUM)*/
   int nb_proc;
   read(sockfd,&nb_proc,sizeof(nb_proc));
   DSM_NODE_NUM   = nb_proc;
   nb_proc_finish = 0;

   /* reception des informations de connexion des autres */
   /* processus envoyees par le lanceur : */
   /* nom de machine, numero de port, etc. */
   proc_array = malloc(sizeof(*proc_array)*nb_proc);
   read(sockfd,proc_array,nb_proc*sizeof(*proc_array));

   // int i;
   // for (i = 0; i < nb_proc; i++) {
   //   printf("%d [%d] rank: %d|port :%d|name :%s\n",getpid(),i,proc_array[i].connect_info.rank,proc_array[i].connect_info.port,proc_array[i].connect_info.hostname);
   //   fflush(stdout);
   // }

   /* initialisation des connexions */
   /* avec les autres processus : connect/accept */
   fds = init_poll(nb_proc);
   accept_interproc(nb_proc, DSM_NODE_ID, fds, serv_fd);
   connection_interproc(nb_proc, DSM_NODE_ID, fds, proc_array);

   /* Allocation des pages en tourniquet */
   for(index = 0; index < PAGE_NUMBER; index ++){
     if ((index % DSM_NODE_NUM) == DSM_NODE_ID)
     dsm_alloc_page(index);
     dsm_change_info( index, WRITE, index % DSM_NODE_NUM);
   }

   /* mise en place du traitant de SIGSEGV */
   act.sa_flags = SA_SIGINFO;
   act.sa_sigaction = *segv_handler;
   sigaction(SIGSEGV, &act, NULL);

   /* creation du thread de communication */
   /* ce thread va attendre et traiter les requetes */
   /* des autres processus */
   pthread_create(&comm_daemon, NULL, dsm_comm_daemon, NULL);
   /* Adresse de début de la zone de mémoire partagée */
   return ((char *)BASE_ADDR);
}

void wait_to_finish() {
  struct message msg = messageCreate(QUIT,-1);
  messageSend(-1,msg);
  nb_proc_finish += 1;

  while (  nb_proc_finish != DSM_NODE_NUM){
    // printf("nb_proc_finish %d\n",nb_proc_finish );
    // myprintf("");
    sleep(1);
  }

}

void dsm_finalize(){
   /* fermer proprement les connexions avec les autres processus */
   /* terminer correctement le thread de communication */
   /* pour le moment, on peut faire : */
   printf("[%i] fin\n", DSM_NODE_ID);
   fflush(stdout);
   fflush(stderr);

   wait_to_finish();
   pthread_cancel(comm_daemon);
   int i;
   for ( i = 0; i < DSM_NODE_NUM; i++) {
     close(fds[i].fd);
   }
   free(proc_array);
  return;
}
