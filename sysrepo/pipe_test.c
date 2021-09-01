#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
// #include <asprintf.h>

int getPipePath(int number, char **path)
{
    if (asprintf(path, "/sr_evpipe%d", number) == -1) {
        return -1;
    }

    // printf("get path %s\r\n", *path);
    return 0;
}


void write_notify(int number) {
    char *path = NULL, buf[1] = {0};
    int fd  = -1, ret;

    if (getPipePath(number, &path) < 0) {
        printf("error get pipe path");
        return;
    }

    if ((fd = open(path, O_WRONLY|O_NONBLOCK)) == -1)
    {
        free(path);
        return;
    }

    
    free(path);

    do {
        ret = write(fd, buf, 1);
    } while (!ret);
    if (ret == -1) {
        printf("error write");
        return;
    }

    printf("write a notify\r\n");
    if (fd > -1) {
        close(fd);
    }
}

void do_read(int fd)
{
    int ret = 0;
    char buf[1];
    do {
        ret = read(fd, buf, 1);
    } while (ret == 1);
    if ((ret == -1) && (errno != EAGAIN)) {
        printf("read error\r\n");
        return;
    }
    printf("read ok , do something!\r\n");
}

void *pipe_listen_thread(void *arg)
{
    fd_set rfds;
    struct timeval tv;
    time_t stop_time_in = 0;
    // pipe read

    int read = *(int *)arg;
    while (1) {
        tv.tv_sec = 10;
        tv.tv_usec = 0;
        FD_ZERO(&rfds);
        FD_SET(read, &rfds);
        int ret = select(read + 1, &rfds, NULL, NULL, &tv);
        if ((ret == -1) && (errno != EINTR)) {
            break;
        } else if ((!ret || ((ret == -1)&&(errno == EINTR)))) {
            printf("time out");
            continue;
        }
        // do read 
        do_read(read);
    }
}

void main() {
    int number = 0;
    char *path = NULL;

    if (getPipePath(number, &path) < 0) {
        return;
    }
    unlink(path);
    mode_t um = umask(0);
    int ret = mkfifo(path, 00622);
    umask(um);
    if (ret == -1) {
        printf("mkfifo error");
        free(path);
        return;
    }
    int readFd = open(path, O_RDWR | O_NONBLOCK);
    if (readFd == -1) {
        printf("open error");
        free(path);
        return;
    }

    free(path);

    pthread_t tid;
    if (ret = pthread_create(&tid, NULL, pipe_listen_thread, &readFd))
    {
        close(readFd);
        return;
    }

    while (1) {
        sleep(5);
        write_notify(number);
    }

    pthread_join(tid, NULL);
    close(readFd);
    return ;
}

