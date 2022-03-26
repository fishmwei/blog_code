
#include <stdio.h>
#include <stdlib.h> 
#include <getopt.h>
#include <string.h>
#include <errno.h>  
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <unistd.h>
#include <pthread.h> 


typedef struct thread_args {
    int accept_fd;
    struct sockaddr_in client_addr;
} thread_args_t;
 
void *thread_func(void *arg)
{
    if (!arg) 
    {
        return NULL;
    }


    thread_args_t *pthread_args = (thread_args_t *)arg;
    char recvBuf[1024];
    while (1)
    {
        size_t recvLen = recv(pthread_args->accept_fd, recvBuf, sizeof(recvBuf), 0);
        if (recvLen < 0)
        {
            perror("recv error");
            close(pthread_args->accept_fd);
            break;
        }

        if (recvLen == 0)
        {
            //perror("len error");//peer has closed
            close(pthread_args->accept_fd);
            break;
        }

        recvBuf[recvLen] = '\0';
        printf("client %d: %s\r\n", pthread_args->accept_fd, recvBuf);

        ssize_t sendLen = send(pthread_args->accept_fd, recvBuf, recvLen, 0);
        if (sendLen < 0)
        {
            perror("send error");
            close(pthread_args->accept_fd);
            break;
        }
    }

    printf("close client %d\r\n", pthread_args->accept_fd);

    free(arg);

    return NULL;
}

int do_select(int port) {
    printf("enter do_select\r\n");
    // create serverfd
    in_addr_t ip = inet_addr("0.0.0.0");
    if (ip == INADDR_NONE)
    {
        perror("parse ip");
        return -1;
    }

    int serverfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverfd < 0)
    {
        perror("socket error");
        return -1;
    }

    int yes = 1;
    if (setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) 
    { 
        perror("setsockopt");
        exit(1);
    }
    printf("get serverfd socket %d\r\n", serverfd);
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = ip;
    server_addr.sin_port = htons(port);
    if (bind(serverfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind error");
        return -1;
    }

    if (listen(serverfd, 100) < 0)
    {
        perror("listen error");
        return -1;
    }

    struct timeval tv;
    fd_set fds;
    int accept_fd, ret;
    int readfds[10];
    int readfds_num = 0;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(serverfd, &fds);
    int maxfd = serverfd + 1;
    while (1) {
        printf("select begin\r\n");
        ret = select(maxfd, &fds, NULL, NULL, NULL);
        if (ret < 1) {

             printf("select error\n");
 
             close(serverfd);
   
            return -1;
        }
        else if (ret == 0) {
            printf("time out\r\n");
        } else {
            if (FD_ISSET(serverfd, &fds)) {
                printf("ready to accept\r\n");
                struct sockaddr_in client_addr;
                socklen_t len = sizeof(client_addr);
                int accept_fd = accept(serverfd, (struct sockaddr *)&client_addr, &len);
                if (accept_fd < 0)
                {
                    perror("accept error");
                    continue;
                }
                
                thread_args_t *thread_args = (thread_args_t *)malloc(sizeof(thread_args_t));
                thread_args->accept_fd = accept_fd;
                memcpy(&thread_args->client_addr, &client_addr, len);

                pthread_t tid;
                pthread_create(&tid, NULL, thread_func, thread_args);
            } else {
                printf("unexcepted fd");
            }
        }
    }

    close(serverfd);
    for (ret = 0; ret < readfds_num; ret++) {
        close(readfds[ret]);
    }

    return 0;
}




int main(int argc, char *argv[]) {
  int next_option;
  const char* const short_options = "p:t:";
  const struct option long_options[] = {
    { "port", 1, NULL, 'p'},
    { "type", 1, NULL, 't'}, // 0 : select, 1: poll, 2: epoll
    { NULL, 0, NULL, 0 }
  };

  int len = -1;
  int port = -1;
  int type = 0;
  do {
    next_option = getopt_long (argc, argv, short_options, long_options, NULL);
    switch (next_option)
    {
      case 'p':
        port = atoi(optarg);
        break;
      case 't':
        type = atol(optarg);
        break;
      default:
        break;
    }
  }while(next_option != -1);


  if (port == -1) {
      printf("invalid port");
      return -1;
  }

  switch(type) {
      case 0: // select
        return do_select(port);
 
      case 1: // poll
      case 2: //epoll
      default:
        printf("invalid type");
      return -1;
  }
  
  return 0;
}