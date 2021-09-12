#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>


typedef struct {
    int request_id;
    int event;
    unsigned int pripority;
    unsigned int count;
} content_t;


typedef struct {
    int request_id;
    int event;
    unsigned int pripority;
    unsigned int count;
    char data[4];
} excontent_t;


typedef struct {
    int fd;
    size_t size;
    void *addr;
} shm_t;

int file_get_size(int fd, size_t *size) 
{
    struct stat st;
    if (fstat(fd, &st) == -1) {
        printf("fstat error\r\n");
        return -1;
    }

    *size = st.st_size;
    return 0;
}

int get_file_path(const char *name, char **path)
{
    int ret = asprintf(path, "/sub2_%s", name);
    if (ret == -1) {
        return -1;
    }

    return 0;
}


int remap(shm_t *shm, size_t new_shm_size)
{
    size_t file_size;
    if (!new_shm_size && (file_get_size(shm->fd, &file_size)))
    {
        return -1;
    }

    // size not changed.
    if ((!new_shm_size && (new_shm_size == file_size)) || (new_shm_size &&new_shm_size == file_size)) {
        return 0;
    }

    if (shm->addr) {
        munmap(shm->addr, shm->size);
        printf("munmap\r\n");
    }

    // resize file 
    if (new_shm_size && (ftruncate(shm->fd, new_shm_size) == -1)) {
        shm->addr = NULL;
        printf("ftruncate error\r\n");
        return -1;
    }

    shm->size = new_shm_size ? new_shm_size : file_size;

    shm->addr = mmap(NULL, shm->size, PROT_READ | PROT_WRITE, MAP_SHARED, shm->fd, 0);
    if (shm->addr == MAP_FAILED) {
        shm->addr = NULL;
        printf("mmap error\r\n");
        return -1;
    }
    printf("get addr %p\r\n", shm->addr);
    return 0;
}

void clear_map(shm_t *shm)
{
    if (shm->addr)
    {
        munmap(shm->addr, shm->size);
        shm->addr = NULL;
    }

    if (shm->fd > -1) {
        close(shm->fd);
        shm->fd = -1;
    }
    shm->size =  0;
}

int open_map(const char *name, shm_t *shm, size_t struct_size)
{
    int created = 1;
    char *path;

    if (get_file_path(name, &path)) {
        printf("get_file_path error\r\n");
        return -1;
    }

    mode_t um = umask(0);
    shm->fd = shm_open(path, O_RDWR | O_CREAT | O_EXCL, 00666);
    umask(um);
    if ((shm->fd == -1) && (errno == EEXIST)) {
        created = 0;
        shm->fd = shm_open(path, O_RDWR, 00666);
        printf("file exist %s, open agine\r\n", path);
    }
    free(path);
    if (shm->fd == -1) {
        printf("open map file error\r\n");
        return -1;
    }

    if (created) {
        // remap struct_size
        if (remap(shm, struct_size)) {
            return -1;
        }

    } else {
        // remap size 0 
        if (remap(shm, 0)) {
            return -1;
        }
    }

    return 0;
}

void main()
{
    shm_t first, second;
    size_t struct_size = sizeof(content_t);
    memset(&first, 0, sizeof(shm_t));
    memset(&second, 0, sizeof(shm_t));
    
    int ret = open_map("hello", &first, struct_size);
    if (ret < 0) {
        return;
    }

    content_t *pData = first.addr;
    pData->count = 100;

    ret = open_map("hello", &second, struct_size);
    if (ret < 0) {
        clear_map(&first);
        return;
    }

    pData = second.addr;
    printf("count is %d\r\n", pData->count);

    ret = remap(&second, sizeof(excontent_t));
    if (0 == ret) {
        excontent_t *pex = second.addr;
        printf("ext count is %d\n", pex->count);
    }

    sleep(10);
    clear_map(&first);
    clear_map(&second);
}



/*
gcc map_test.c -o map -lrt


strace ./map 

shm_open will create file in /dev/shm 

*/