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
            perror("len error");
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

    while((opt = getopt_long_only(argc,argv, "p:s:",long_options, NULL))!= -1)
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

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        perror("socket error");
        return -1;
    }

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

    char recvBuf[1024];
    struct sockaddr_in other_addr;
    socklen_t other_addr_len = sizeof(other_addr);
    while (1)
    {
        ssize_t recv_len = recvfrom(fd, recvBuf, 1024, 0, (struct sockaddr *)&other_addr, &other_addr_len);
        if (recv_len < 0) 
        {
            perror("recv error");
            continue;
        }
        recvBuf[recv_len] = 0;
        printf("recv msg from %s(%u) : %s\r\n", inet_ntoa(other_addr.sin_addr), ntohs(other_addr.sin_port), recvBuf);
        
        int send_len = sendto(fd, "ok", 2, 0, (struct sockaddr *)&other_addr, other_addr_len);
    }

    close(fd);

    return 0;
}