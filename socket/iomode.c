

 
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
#include <fcntl.h>  
#include <poll.h>

#define MACOS 1

#if !MACOS
#include <sys/epoll.h>


///////////////////////////////////////////////// epoll  /////////////////////////////////

int do_epoll(int port) {
    printf("enter do_epoll\r\n");

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

     int flags = fcntl(serverfd, F_GETFL, 0);
    fcntl(serverfd, F_SETFL, flags|O_NONBLOCK);

    if (listen(serverfd, 100) < 0)
    {
        perror("listen error");
        return -1;
    }

#define EPOLL_SIZE 1024

    int epoll_fd = epoll_create(EPOLL_SIZE);
    struct epoll_event epoll_events[EPOLL_SIZE];
    struct epoll_event temp_event;

    // add server fd
    temp_event.events = POLLIN;
    temp_event.data.fd = serverfd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serverfd, &temp_event);
    int epoll_count = 1;
    int ret, i;
    while (1) {
        ret = epoll_wait(epoll_fd, epoll_events, EPOLL_SIZE, 5000);
        if (ret == 0) {
            printf("time out \r\n");
        } else if (ret < 0) {
            printf("epoll wait error \r\n");
        } else {
            for (i = 0; i < ret; i++) {
                if (epoll_events[i].data.fd == serverfd) {
                    // accept 
                    printf("ready to accept\r\n");
                    struct sockaddr_in client_addr;
                    socklen_t len = sizeof(client_addr);
                    int accept_fd = accept(epoll_events[i].data.fd, (struct sockaddr *)&client_addr, &len);
                    if (accept_fd < 0)
                    {
                        perror("accept error"); 
                    }
                    else {
                        if (epoll_count < EPOLL_SIZE) {
                            temp_event.events = POLLIN;
                            temp_event.data.fd = accept_fd;
                            epoll_ctl(epoll_fd, EPOLL_CTL_ADD, accept_fd, &temp_event);
                            epoll_count++;
                        } else {
                            printf("reach max connect\r\n");
                            close(accept_fd);
                        }
                    }
                } else {
                    if (epoll_events[i].events&POLLIN) {
                        char recvBuf[1024];
                        {
                            size_t recvLen = recv(epoll_events[i].data.fd, recvBuf, sizeof(recvBuf), 0);
                            if (recvLen < 0)
                            {
                                perror("recv error");
                                close(epoll_events[i].data.fd);
                                temp_event.events = POLLIN;
                                temp_event.data.fd = epoll_events[i].data.fd;
                                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, epoll_events[i].data.fd, &temp_event);
                                epoll_count--; 
                                continue;
                            }

                            if (recvLen == 0)
                            {
                                perror("len error");//peer has closed
                                close(epoll_events[i].data.fd);
                                temp_event.events = POLLIN;
                                temp_event.data.fd = epoll_events[i].data.fd;
                                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, epoll_events[i].data.fd, &temp_event);
                                epoll_count--; 
                                continue;
                            }

                            recvBuf[recvLen] = '\0';
                            printf("client %d: %s\r\n", epoll_events[i].data.fd, recvBuf);

                            ssize_t sendLen = send(epoll_events[i].data.fd, recvBuf, recvLen, 0);
                            if (sendLen < 0)
                            {
                                perror("send error");
                                close(epoll_events[i].data.fd);
                                temp_event.events = POLLIN;
                                temp_event.data.fd = epoll_events[i].data.fd;
                                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, epoll_events[i].data.fd, &temp_event);
                                epoll_count--;  
                                continue;
                            }
                        } 
                    }
                }
            }
        }
    }

    return 0;
}


#endif


///////////////////////////////////////////////// poll  /////////////////////////////////
void clean_fd(int *fds, int num, int fd) {
    int i;
    for (i = 0; i < num; i++) {
        if (fds[i] == fd) {
            fds[i] = 0;
            printf("fd %d quit\r\n", fd);
            break;
        }
    }
}

int do_poll(int port) {
    printf("enter do_poll\r\n");

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

     int flags = fcntl(serverfd, F_GETFL, 0);
    fcntl(serverfd, F_SETFL, flags|O_NONBLOCK);

    if (listen(serverfd, 100) < 0)
    {
        perror("listen error");
        return -1;
    }

    struct pollfd fds[1024];
    int pollfdnum;
    int clientfds[1023];
    int clientnumm = 0;

    fds[0].fd = serverfd;
    fds[0].events = POLLIN;
    pollfdnum = 1;
    int changed = 0;
    int ret;
    int i, j;
    while (1) {
        if (changed) {
            fds[0].fd = serverfd;
            fds[0].events = POLLIN;
            j=1;
            for (i = 0; i < clientnumm; i++) {
                if (clientfds[i] != 0) {
                    fds[j].fd = clientfds[i];
                    fds[j].events = POLLIN;
                    j++;
                }
            }
            pollfdnum = j;
        }

        ret = poll(fds, pollfdnum, -1);
        if (ret <= 0) {
            printf("poll error \r\n");
            return 1;
        }

        // accept
        if (fds[0].revents) {
            printf("ready to accept\r\n");
            struct sockaddr_in client_addr;
            socklen_t len = sizeof(client_addr);
            int accept_fd = accept(fds[0].fd, (struct sockaddr *)&client_addr, &len);
            if (accept_fd < 0)
            {
                perror("accept error"); 
            }
            else {
                if (clientnumm < 1023) {
                    clientfds[clientnumm++] = accept_fd;
                    changed = 1;
                }
                else
                {
                    for (i = 0; i < clientnumm; i++) {
                        if (clientfds[i] == 0) {
                            clientfds[i] = accept_fd;
                            changed = 1;
                        }
                    }
                    if (i >= clientnumm) {
                        printf("reach max connections\r\n");
                        close(accept_fd); 
                    }
                }
            }
        }

        for (i = 1; i < pollfdnum; i++) {
            if (fds[i].revents & POLLIN) {
                char recvBuf[1024];
                {
                    size_t recvLen = recv(fds[i].fd, recvBuf, sizeof(recvBuf), 0);
                    if (recvLen < 0)
                    {
                        perror("recv error");
                        close(fds[i].fd);
                        clean_fd(clientfds, clientnumm, fds[i].fd);
                        changed = 1; 
                        continue;
                    }

                    if (recvLen == 0)
                    {
                        perror("len error");//peer has closed
                        close(fds[i].fd);
                        clean_fd(clientfds, clientnumm, fds[i].fd);
                        changed = 1; 
                        continue;
                    }

                    recvBuf[recvLen] = '\0';
                    printf("client %d: %s\r\n", fds[i].fd, recvBuf);

                    ssize_t sendLen = send(fds[i].fd, recvBuf, recvLen, 0);
                    if (sendLen < 0)
                    {
                        perror("send error");
                        close(fds[i].fd);
                        clean_fd(clientfds, clientnumm, fds[i].fd);
                        changed = 1; 
                        continue;
                    }
                } 
            }
        }
    }

}

///////////////////////////////////////////////// select  /////////////////////////////////
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

     int flags = fcntl(serverfd, F_GETFL, 0);
    fcntl(serverfd, F_SETFL, flags|O_NONBLOCK);

    if (listen(serverfd, 100) < 0)
    {
        perror("listen error");
        return -1;
    }

    struct timeval tv;
    fd_set fds, waitfds;
    int accept_fd, ret;
    int readfds[100];
    int readfds_num = 0;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(serverfd, &fds);
    int maxfd = serverfd + 1;
    int changed = 0;
    while (1) {
        #if 0
        if (0) { // must set every time.
            changed = 0;
            printf("socket chagned------------, must set every time\r\n");
            FD_ZERO(&fds);
            FD_SET(serverfd, &fds);
            maxfd = serverfd + 1;
            int i = 0;
            for (i = 0; i < readfds_num; i++) {
                if (readfds[i] != 0 ) {
                    FD_SET(readfds[i], &fds);
                    if (maxfd < readfds[i] + 1) {
                        maxfd = readfds[i] + 1;
                    }
                }
            }
        }
        #endif
        waitfds = fds; // fds is all fds which we want to watch
        printf("select begin\r\n");
        ret = select(maxfd, &waitfds, NULL, NULL, NULL);
        if (ret < 0) {

             printf("select error ret %d\n", ret);
 
             close(serverfd);
   
            return -1;
        }
        else if (ret == 0) {
            printf("time out\r\n");
        } else {
            if (FD_ISSET(serverfd, &waitfds)) {
                printf("ready to accept\r\n");
                struct sockaddr_in client_addr;
                socklen_t len = sizeof(client_addr);
                int accept_fd = accept(serverfd, (struct sockaddr *)&client_addr, &len);
                if (accept_fd < 0)
                {
                    perror("accept error");
                    continue;
                }
                if (readfds_num < 100) {
                    readfds[readfds_num++] = accept_fd;
                    changed = 1;
                } else {
                    int i = 0;
                    for (i = 0; i < readfds_num; i++) {
                        if (readfds[i] == 0) {
                            readfds[i] = accept_fd;
                            changed = 1;
                        }
                    }
                    if (i >= readfds_num) {
                        printf("reach max connections\r\n");
                        close(accept_fd);
                        continue;
                    }
                }

                if (changed) {
                    flags = fcntl(accept_fd, F_GETFL, 0);
                    fcntl(accept_fd, F_SETFL, flags|O_NONBLOCK);
                    FD_SET(accept_fd, &fds);
                    if (maxfd < accept_fd + 1) {
                        maxfd = accept_fd + 1;
                        printf("change max fd ===========\r\n");
                    }
                    printf("accept new fd %d\r\n", accept_fd);
                }
            } else {
                int i;
                char recvBuf[1024];
                for (i = 0; i < readfds_num; i++) {
                    if (FD_ISSET(readfds[i], &fds)) {
                        size_t recvLen = recv(readfds[i], recvBuf, sizeof(recvBuf), 0);
                        if (recvLen < 0)
                        {
                            perror("recv error");
                            close(readfds[i]);
                            changed = 1;
                            FD_CLR(readfds[i], &fds);
                            readfds[i] = 0;
                            continue;
                        }

                        if (recvLen == 0)
                        {
                            perror("len error");//peer has closed
                            close(readfds[i]);
                            FD_CLR(readfds[i], &fds);
                            changed = 1;
                            readfds[i] = 0;
                            continue;
                        }

                        recvBuf[recvLen] = '\0';
                        printf("client %d: %s\r\n", readfds[i], recvBuf);

                        ssize_t sendLen = send(readfds[i], recvBuf, recvLen, 0);
                        if (sendLen < 0)
                        {
                            perror("send error");
                            close(readfds[i]);
                            changed = 1;
                            FD_CLR(readfds[i], &fds);
                            readfds[i] = 0;
                            continue;
                        }
                    }
                } 
            }
        }
    }

    close(serverfd);
    for (ret = 0; ret < readfds_num; ret++) {
        if (readfds[ret] != 0)
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
        return do_poll(port);
      case 2: //epoll
      #if !MACOS
        return do_epoll(port);
        #else
        printf("No epoll in macos");
        #endif
      default:
        printf("invalid type");
      return -1;
  }
  
  return 0;
}


/*
Usage:
gcc iomode.c -o iomode

./iomode -t 0 -p 10001


参考资料：
https://www.cnblogs.com/orlion/p/6142838.html

https://www.cnblogs.com/lxmhhy/p/6214113.html

-------------------

行动，才不会被动!

欢迎关注个人公众号 微信 -> 搜索 -> fishmwei

个人博客： https://fishmwei.gitee.io/

-------------------

*/