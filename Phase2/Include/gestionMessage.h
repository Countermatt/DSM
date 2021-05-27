#ifndef GESTIONMESSAGE_H
#define GESTIONMESSAGE_H


enum msg_type {
  //localisation de la page
	LOCALISATION_REQUEST=1,
  LOCALISATION_YES,
  LOCALISATION_NO,
  //accés en lecture page
  READ_REQUEST,
  READ_ACCEPT,
  READ_REFUSE,
  //accés en ecriture page
  WRITE_REQUEST,
  WRITE_ACCEPT,
  WRITE_REFUSE,
  //gestion des droits sur une page
  STATUS_REQUEST,
  STATUS_SEND,
  STATUS_CHANGE,
  STATUS_ACCEPT,
  STATUS_REFUSE,
	//FIN
	QUIT
};

struct message {
  enum msg_type type;
	int page_number;
	int rank;
	// int port_sender;
  // int port_receiv;
  // char hostname_sender[LENNAME];
  // char hostname_receiv[LENNAME];
	// char payload[PAGE_SIZE];
};

static char* msg_type_str[] = {
	"LOCALISATION_REQUEST",
  "LOCALISATION_YES",
  "LOCALISATION_NO",
  "READ_REQUEST",
  "READ_ACCEPT",
  "READ_REFUSE",
  "WRITE_REQUEST",
  "WRITE_ACCEPT",
  "WRITE_REFUSE",
  "STATUS_REQUEST",
  "STATUS_SEND",
  "STATUS_CHANGE",
  "STATUS_ACCEPT",
  "STATUS_REFUSE",
	"QUIT"
};

struct message messageCreate(enum msg_type type,int page_number);
void messageSend(int fd,struct message msg);
int messageRecv(int fd);
int traitementMsg(struct message msg,int fd);
#endif
