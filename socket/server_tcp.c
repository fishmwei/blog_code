#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <getopt.h>

const struct option long_options[] =
{
    {"port", required_argument, NULL, 'p'},
    {"ip", required_argument, NULL, 's'},
}; 

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


int main(int argc, char *argv[])
{
    int opt;
    uint16_t port = 65535;
    char *ip_arg = "0.0.0.0";

    while((opt =getopt_long_only(argc,argv, "p:s:",long_options, NULL))!= -1)
    {
        switch(opt) {
        case 'p':
            port = atoi(optarg);
            
            break;
        case 's':
            {
                ip_arg = optarg;
            }
            break;
        default:
                break;
        }
    }

    if (port == 65535) {
        printf("invalid argument\r\n");
        return -1;
    }


    in_addr_t ip = inet_addr("0.0.0.0");
    if (ip == INADDR_NONE)
    {
        perror("parse ip");
        return -1;
    }

    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0)
    {
        perror("socket error");
        return -1;
    }


    printf("get socket %d\r\n", fd);
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = ip;
    server_addr.sin_port = htons(port);
    if (bind(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind error");
        return -1;
    }

    if (listen(fd, 100) < 0)
    {
        perror("listen error");
        return -1;
    }

    while (1)
    {
        printf("ready to accept\r\n");
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int accept_fd = accept(fd, (struct sockaddr *)&client_addr, &len);
        if (accept_fd < 0)
        {
            perror("accept error");
            continue;
        }
        
        printf("accept client %d, addr: %s（%d）\r\n", accept_fd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        thread_args_t *thread_args = (thread_args_t *)malloc(sizeof(thread_args_t));
        thread_args->accept_fd = accept_fd;
        memcpy(&thread_args->client_addr, &client_addr, len);

        pthread_t tid;
        pthread_create(&tid, NULL, thread_func, thread_args);
        //pthread_join(tid, NULL);
    }

    close(fd);

    return 0;
}