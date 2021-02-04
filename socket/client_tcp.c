#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>// close
#include <getopt.h>
#include <string.h>

const struct option long_options[] =
{
    {"port", required_argument, NULL, 'p'},
    {"server", required_argument, NULL, 's'},
}; 



int main(int argc, char *argv[])
{
    int opt;
    uint16_t port = 65535;
    char *ip_arg = NULL;

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

    if (port == 65535 || ip_arg == NULL) {
        printf("invalid argument\r\n");
        return -1;
    }

    printf("get port %d, server %s\r\n", port, ip_arg);

    in_addr_t ip = inet_addr(ip_arg);
    if (ip == INADDR_NONE)
    {
        perror("ip error");
        return -1;
    } 

    //uint16_t port = 5001;
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
    if (connect(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect error");
        return -1;
    }

    char sendBuf[1024];
    while (1) 
    {
        int len = scanf("%s", sendBuf);
        if (len != 1) 
        {
            perror("scanf error");
            break;
        }
        
        if (strcmp(sendBuf, "quit") == 0 ) {
            break;
        }

        size_t sendLen = send(fd, sendBuf, strlen(sendBuf), 0);
        if (sendLen < 0)
        {
            perror("send error");
            return -1;
        }
        
        char recvBuf[2048];
        char *pRecvBuf = recvBuf;
        size_t bufLen = sizeof(recvBuf);
        bufLen = 2048;
        size_t recvTotal = 0;
        while (recvTotal < sendLen)
        {
            ssize_t recvLen = recv(fd, recvBuf, bufLen, 0);
            if (recvLen < 0)
            {
                perror("recv error");
                close(fd);
                return -1;
            }

            if (recvLen == 0)
            {
                perror("len error");
                close(fd);
                break;
            }
            pRecvBuf += recvLen;
            bufLen -= recvLen;
            recvTotal += recvLen;
        }
        recvBuf[recvTotal] = '\0';
        printf("server response %s\r\n", recvBuf);
    }

    
    //cout << "recv len: " << uiRecvTotal << ", msg: " << szRecvBuf << endl;
    close(fd);
    printf("end\r\n");

    return 0;
}