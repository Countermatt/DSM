#include <poll.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <string.h>
#include <unistd.h>

int main(int argc, char const *argv[]) {
  printf("%d\n", POLLWRBAND);
  printf("%d\n", POLLWRNORM);
  printf("%d\n", POLLRDBAND);
  printf("%d\n", POLLRDNORM);
  printf("%d\n", POLLNVAL);
  printf("%d\n", POLLHUP);
  printf("%d\n", POLLERR);
  // printf("%d\n", POLLRDHUP);
  printf("%d\n", POLLOUT);
  printf("%d\n", POLLPRI);
  printf("%d\n", POLLIN);

  int fd,len;
  char txt[5][10] = {"a","ab","abc","abcd","abdce"};
  int i;
  fd =  open("./test.txt",O_CREAT|O_WRONLY,S_IRWXU);

  for ( i = 0; i < 5; i++) {
    printf("ptr %d:%p valeur:%s \n",i, txt+i, *(txt+i) );
    // printf("ptr %d:%p valeur:%s \n",i, txt+i, *(txt+i) );
    printf("\n");
  }

  if ( fd < 0 ) {
    perror("open");
    exit(EXIT_FAILURE);
  }

  len = sizeof(txt);
  printf("table len: %d %d \n", len, len/sizeof(char*));
  write(fd,txt, 5*10*sizeof(char));
  close(fd);

  /* ----------------------------------------------*/
  printf("\n" );
  fd =  open("./test.txt",O_RDONLY);
  char buffer_int[5][10];
  memset(buffer_int,0,5*10*sizeof(char));
  read(fd,buffer_int,5*10*sizeof(char) );
  len = sizeof(buffer_int)/sizeof(char);

  for ( i = 0; i < 10; i++) {
    printf("%s\n",buffer_int[i] );
  }
  
  char str[1024];
  char *wd_ptr = NULL;
  wd_ptr = getcwd(str,1024);
  printf("wd: %s\n",wd_ptr);

  return 0;


}
