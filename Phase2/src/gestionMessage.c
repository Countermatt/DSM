#include "gestionMessage.h"
#include "common.h"
#include <string.h>
#include <unistd.h>
#include <dsm.h>

struct message messageCreate(enum msg_type type,int page_number){
  struct message msg;
  memset(&msg,0,sizeof(msg));
  msg.type        = type;
  msg.page_number = page_number;
  msg.rank        = DSM_NODE_ID;
  return msg;
}

void send_LOCALISATION_YES(int fd,struct message msg){
    write(fd,&msg,sizeof(msg));
}

void send_QUIT(struct message msg){
  int i;
  for ( i = 1; i < DSM_NODE_NUM; i++) {
    write(fds[i].fd,&msg,sizeof(msg));
  }
}

void messageSend(int fd,struct message msg){
     switch (msg.type) {
       case LOCALISATION_YES:
        send_LOCALISATION_YES(fd,msg);
        break;

       case QUIT:
       send_QUIT(msg);
       break;

       default:
       break;
     }
}



/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

static int dsm_send(int dest,int numero_page){
  char* pointer = num2address( numero_page );
  int toRecv = PAGE_SIZE;
  int recv = 0;
  do {
    recv += write(dest,pointer+recv,toRecv);
    toRecv -= recv;
  } while(toRecv!=0);
   return 0;
}

int recv_STATUS_ACCEPT(){
  return 1;
}

int recv_STATUS_REFUSE(){
  return -1;
}

int get_STATUS(int fd){
  struct message msg;
  memset(&msg,0,sizeof(msg));
  read(fd,&msg,sizeof(msg));
  if (msg.type == STATUS_ACCEPT) {
    return 1;
  }
  return 0;
}

void recv_WRITE_REQUEST(int fd,struct message msg) {
  if ( get_owner(msg.page_number)==DSM_NODE_ID ) {
    struct message send_msg =  messageCreate(LOCALISATION_YES,msg.page_number);
    messageSend(fd,send_msg);
    if (get_STATUS(fd) == 1) {
      int port = proc_array[msg.rank].connect_info.port;
      char sport[10];
      sprintf(sport,"%d",port);
      int page_recver = handle_connect(proc_array[msg.rank].connect_info.hostname,sport);
      dsm_protect_page( msg.page_number ,PROT_READ);
      dsm_send(page_recver,msg.page_number);
      dsm_change_info(msg.page_number,INVALID,msg.rank);
      dsm_free_page(msg.page_number);
      close(page_recver);
    }
  }
}


void recv_QUIT() {
  nb_proc_finish += 1;
}

int recv_LOCALISATION_YES(int fd,struct message msg){
  struct message send_msg =  messageCreate(STATUS_REFUSE,msg.page_number);
  return write(fd,&send_msg,sizeof(send_msg));
}

int traitementMsg(struct message msg,int fd){
  switch (msg.type) {
    case WRITE_REQUEST:
    // printf("[%d]messageRecv WRITE_REQUEST %d\n", DSM_NODE_ID,msg.rank);
    recv_WRITE_REQUEST(fd,msg);
    break;

    case LOCALISATION_YES:
    // printf("[%d]messageRecv LOCALISATION_YES %d\n", DSM_NODE_ID,msg.rank);
    recv_LOCALISATION_YES(fd,msg);
    break;

    case STATUS_REFUSE:
    // printf("[%d]messageRecv STATUS_REFUSE %d\n", DSM_NODE_ID,msg.rank);
    break;

    case QUIT:
    recv_QUIT();
    break;

    default:
    break;
  }
}

int messageRecv(int fd) {
  struct message msg;
  memset(&msg,0,sizeof(msg));
  int n = read(fd,&msg,sizeof(msg));
  traitementMsg(msg,fd);
  return 0;
}
