#include <poll.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <string.h>
#include <unistd.h>
#include <errno.h>


int main(int argc, char const *argv[]) {
  int i;
  int read_err[]={
    EAGAIN,
    EAGAIN,
    EWOULDBLOCK,
    EBADF,
    EFAULT,
    EINTR,
    EINVAL,
    EINVAL,
    EIO,
    EISDIR
  };
  char *char_read_err[]={
    "EAGAIN",
    "EAGAIN",
    "EWOULDBLOCK",
    "EBADF",
    "EFAULT",
    "EINTR",
    "EINVAL",
    "EINVAL",
    "EIO",
    "EISDIR"
  };
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
  printf("\n" );
  for (i = 0; i < 10; i++) {
    printf("%s:%d\n",char_read_err[i], read_err[i]);
  }
  return 0;


}
