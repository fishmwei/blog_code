#include <sys/mman.h>

#include <sys/types.h>

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

typedef struct {
	char name[256];
	int  from_id;
} data_t;

int main(int argc, char** argv)
{
    int fd, i;
	data_t *p_map;
	register pid_t pid;

    pid = getpid();
	fd=open(argv[1],O_CREAT|O_RDWR|O_TRUNC,00777);

    ftruncate(fd, sizeof(data_t)); // need a size... 
	//lseek(fd,sizeof(data_t)-1,SEEK_SET);

	//write(fd,"",1);

	p_map = (data_t*)mmap( NULL, sizeof(data_t), PROT_READ|PROT_WRITE,MAP_SHARED,fd,0 );

	close(fd);


    char command[256];
    while (1) {
        printf("\r\nchoice(%d):\r\n 1: show info, 0: input info\r\n", pid);
        scanf("%d", &i);
        switch (i) {
            case 0:
            {
                printf("enter data:\n");
                scanf("%s", p_map->name);
                p_map->from_id = pid;
                printf("\r\nyou input %s in process %d\r\n", p_map->name, pid);
                break;
            }
            case 1:
            {
                printf("content : %s\r\n from %d\r\n", p_map->name, p_map->from_id);
                break;
            }
            case 2:
            {
                break;
            }
            default:
            {
                printf("wrong choice\r\n");
                break;
            }
        }

        if (i==2) {
            break;
        }
    }

	printf(" initialize over \n ");

	//sleep(10);

	munmap( p_map, sizeof(data_t) );

	printf( "umap ok \n" );
    return 0;
}