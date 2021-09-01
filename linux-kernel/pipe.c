
#include <unistd.h>
#include <stdio.h>

#define MAXLINE 1024

#if 0
int main()
{
    int n;
    int fd[2];
    pid_t pid;
    char line[MAXLINE];
    if (pipe(fd) < 0)
    {
        printf("pipe error");
    }
    pid=fork();
    if (pid == 0)
    {
        close(fd[1]);
        //sleep(5);

        n=read(fd[0],line,MAXLINE);
        write(STDOUT_FILENO,line,n);
        
    }
    else
    {
        close(fd[0]);
        write(fd[1],"hello world\n",12);
    }

    return 0;
}

#endif


#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[])
{
  int fds[2];
  if (pipe(fds) == -1)
    perror("pipe error");

  pid_t pid;
  pid = fork();
  if (pid == -1)
    perror("fork error");

  if (pid == 0){
    // child
    dup2(fds[1], STDOUT_FILENO);
    close(fds[1]);
    close(fds[0]);
    execlp("ps", "ps", "-ef", NULL);
  } else {
    // father

    pid = fork();
    if (pid == 0) {
        dup2(fds[0], STDIN_FILENO);
        close(fds[0]);
        close(fds[1]);
        execlp("grep", "grep", "systemd", NULL);
    }
    else
    {
        sleep(5);
    }
  }
  

  
  return 0;
}