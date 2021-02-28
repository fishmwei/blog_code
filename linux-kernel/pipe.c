
#include <unistd.h>
#include <stdio.h>

#define MAXLINE 1024
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