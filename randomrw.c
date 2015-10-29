#define _GNU_SOURCE
#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <time.h>
#include <locale.h>
#include <assert.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <stdint.h>

#define	TEST_NUM	(1)
#define	REQUEST_SIZE	(4*1024)
#define ALIGN_SIZE	(4*1024)
#define	FILE_SIZE	(32*1024*1024)

unsigned char *buf;
const char *file_path="./test_random_io";

struct frand_state{
        unsigned int s1, s2, s3;
};

static inline unsigned int __rand(struct frand_state *state)
{
#define TAUSWORTHE(s,a,b,c,d) ((s&c)<<d) ^ (((s <<a) ^ s)>>b)

        state->s1 = TAUSWORTHE(state->s1, 13, 19, 4294967294UL, 12);
        state->s2 = TAUSWORTHE(state->s2, 2, 25, 4294967288UL, 4);
        state->s3 = TAUSWORTHE(state->s3, 3, 11, 4294967280UL, 17);

        return (state->s1 ^ state->s2 ^ state->s3);
}

void random_read(void)
{
        unsigned int rand_num;
        int fd;
        int j = 0;
        int ret;
	    struct frand_state fs;

        fd = open(file_path, O_RDONLY | O_DIRECT, 0755);
	if (fd < 0) {
		perror("open file failed!\n");
		return;
	}

        do {
		rand_num = __rand(&fs);
		rand_num %= FILE_SIZE;
		rand_num = (rand_num + REQUEST_SIZE -1) & (~(REQUEST_SIZE -1));
                lseek(fd, rand_num, SEEK_SET);
                ret = read(fd, buf, REQUEST_SIZE);
                if (ret < 0) {
                        perror("read failed");
                }
                j++;
        } while (j < (FILE_SIZE/REQUEST_SIZE));
        close(fd);
        unlink(file_path);
	    return;
}

void random_write(void)
{
        unsigned int rand_num;
        int fd;
        int j = 0;
        int ret;
	    struct frand_state fs;

        fd = open(file_path, O_WRONLY | O_CREAT | O_DIRECT, 0755);
	if (fd < 0) {
		perror("open file failed!\n");
		return;
	}

        do {
                rand_num = __rand(&fs);
		rand_num %= FILE_SIZE;
		rand_num = (rand_num + REQUEST_SIZE -1) & (~(REQUEST_SIZE -1));
                lseek(fd, rand_num, SEEK_SET);
                ret = write(fd, buf, REQUEST_SIZE);
                if (ret < 0) {
                        perror("write failed");
                }
                j++;
        } while (j < (FILE_SIZE/REQUEST_SIZE));
        close(fd);
}


void do_random_io(void)
{
	random_write();
	random_read();
}

int main(int argc, char *argv[])
{
        int ret;
        int i;
        int sum;
        int timeuse[TEST_NUM];
	    int len;
        char *file_name;
        struct timeval start, end;
	    struct frand_state fs;	

        file_name = argv[0];
        ret = argc;

        ret = posix_memalign((void **)&buf, ALIGN_SIZE, REQUEST_SIZE);
        if (ret) {
                perror("posix_memalign failed");
                exit(1);
        }
        memset(buf, 'c', REQUEST_SIZE);

        for (i = 0; i < TEST_NUM; i++){
                gettimeofday(&start, NULL);
                do_random_io();
                gettimeofday(&end, NULL);

                timeuse[i] = 1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;
        }
        free(buf);
	    buf = NULL;

        sum = 0;
        for (i = 0; i < TEST_NUM; i++){
                sum += timeuse[i];
        }

        printf("Time comsumes:\n");
        printf("*****************\n");
        printf("%d ms\n", sum/(1000 * TEST_NUM));
        printf("*****************\n");
        return 0;
}
