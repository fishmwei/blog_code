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
#include <string.h>

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

#define CONNECT_ENABLE (1)
int main(int argc, char *argv[])
{
    int opt;
    uint16_t port = 65535;
    char *ip_arg = NULL;

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

    if (port == 65535 || ip_arg == NULL) {
        printf("invalid argument\r\n");
        return -1;
    }

    in_addr_t ip = inet_addr(ip_arg);
    if (ip == INADDR_NONE)
    {
        perror("parse ip");
        return -1;
    }

    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
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


    struct sockaddr_in other_addr;
    memset(&other_addr, 0, sizeof(other_addr));
    other_addr.sin_family = AF_INET;
    other_addr.sin_addr.s_addr = 0;
    other_addr.sin_port = htons(port+1);

#if CONNECT_ENABLE
    // 随机端口号
    if (connect(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect error");
        return -1;
    }
#else
    // 绑定端口号
    if (bind(fd, (struct sockaddr *)&other_addr, sizeof(other_addr)) < 0)
    {
        perror("bind error");
        return -1;
    }
#endif

    char recvBuf[1024];
    socklen_t other_addr_len = sizeof(other_addr);

    printf("ready to enter msg:\r\n");
    while (1)
    {
        int ret = scanf("%s", recvBuf);
        if (ret != 1) 
        {
            perror("scanf error");
            break;
        }

        if (strcmp(recvBuf, "quit") == 0 ) 
        {
            break;
        }

        #if CONNECT_ENABLE
        // 使用了connect， 只能用send发送了
        int send_len = send(fd, recvBuf, strlen(recvBuf), 0);
        ssize_t recv_len = recv(fd, recvBuf, 1024, 0);

        #else
        
        int send_len = sendto(fd, recvBuf, strlen(recvBuf), 0, (struct sockaddr *)&server_addr, other_addr_len);
        ssize_t recv_len = recvfrom(fd, recvBuf, 1024, 0, (struct sockaddr *)&server_addr, &other_addr_len);
        
        #endif
        if (recv_len < 0) 
        {
            perror("recv error");
            continue;
        }
        recvBuf[recv_len] = 0;
        printf("recv msg from %s : %s\r\n", inet_ntoa(server_addr.sin_addr), recvBuf);
        
    }

    close(fd);

    return 0;
}