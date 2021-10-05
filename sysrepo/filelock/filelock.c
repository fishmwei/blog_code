#include <stdio.h>
#include<fcntl.h> 
#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

#define SHM_FILE "/tmp/shmfile"
#define SR_MAIN_SHM_LOCK "/tmp/lockfile"
#define SR_MAIN_SHM_PERM 00666
#define SR_UMASK 0


//========================== file map =======================

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
    int ret = asprintf(path, "%s", SHM_FILE);
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



//========================== file lock =======================

typedef struct shm_main_s {
    int value;
}shm_main_t;


int 
sr_shmmain_createlock(int shm_lock)
{
    struct flock fl;
    int ret;

    memset(&fl, 0, sizeof fl);
    fl.l_type = F_WRLCK;
    do {
        ret = fcntl(shm_lock, F_SETLKW, &fl);
    } while ((ret == -1) && (errno == EINTR));
    if (ret == -1) {
        printf("lock fail\r\n");
        return -1;
    }

    return 0;
}

void
sr_shmmain_createunlock(int shm_lock)
{
    struct flock fl;

    memset(&fl, 0, sizeof fl);
    fl.l_type = F_UNLCK;
    if (fcntl(shm_lock, F_SETLK, &fl) == -1) {
        //assert(0);
    }
}


int
sr_shmmain_createlock_open(int *shm_lock)
{
    char *path;
    mode_t um;

    if (asprintf(&path, "%s", SR_MAIN_SHM_LOCK) == -1) {
        printf("get file path error\r\n"); 
        return -1;
    }

    /* set umask so that the correct permissions are really set */
    um = umask(SR_UMASK);

    *shm_lock = open(path, O_RDWR | O_CREAT, SR_MAIN_SHM_PERM);
    free(path);
    umask(um);
    if (*shm_lock == -1) {
        printf("open file error\r\n");
        return -1;
    }

    return 0;
}


int main(int argc, char** argv)
{
    if (argc != 3) {
        printf("argc is %d\r\n", argc);
        return -1;
    }

    int i = 0;
    int step = atoi(argv[1]);
    int count = atoi(argv[2]);

    printf("get step %d, count %d\r\n", step, count);

    shm_t shm;
    size_t struct_size = sizeof(struct shm_main_s);
    memset(&shm, 0, sizeof(shm_t));
    int ret = open_map("", &shm, struct_size);
    if (ret < 0) {
        return -1;
    }

    int filelock = -1;
    if (sr_shmmain_createlock_open(&filelock) < 0) {
        clear_map(&shm);
        return -1;
    }

    shm_main_t *ptr = shm.addr;
    printf("init value is %d\r\n", ptr->value);
    while (count--) {
        sleep(1);
        sr_shmmain_createlock(filelock);
        printf("current value %d\r\n", ptr->value);
        ptr->value += step;
        sr_shmmain_createunlock(filelock);
    }
    printf("step %d done ... \r\n", step);
    sleep(10);
    sr_shmmain_createlock(filelock);
    printf("result value %d\r\n", ptr->value);
    sr_shmmain_createunlock(filelock);
    
    clear_map(&shm);
    close(filelock);


    return 0;
}




