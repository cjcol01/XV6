#include "kernel/types.h"
#include "user/user.h"

#include "user.h"

int main(void)
{
  int p2c[2], c2p[2];
  char ping = 'P', pong = 'R';
  char buf;
  int pid;

  // Creating Pipes
  if (pipe(p2c) < 0 || pipe(c2p) < 0)
  {
    printf("Pipe creation failed\n");
    exit(-1);
  }

  // Forking a Child Process
  pid = fork();

  if (pid < 0)
  {
    printf("Fork failed\n");
    exit(-1);
  }

  if (pid == 0)
  {
    // Child process
    read(p2c[0], &buf, 1);
    printf("%d: Received %c\n", getpid(), buf);
    write(c2p[1], &pong, 1);
    exit(0);
  }
  else
  {
    // Parent process
    write(p2c[1], &ping, 1);
    read(c2p[0], &buf, 1);
    printf("%d: Received %c\n", getpid(), buf);
    exit(0);
  }
  exit(0);
};
