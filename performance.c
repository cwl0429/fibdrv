#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define FIB_DEV "/dev/fibonacci"
#define FIB_NAIVE 0
#define FIB_FAST_DOUBLING 1

int main()
{
    char write_buf[] = "testing writing";
    int offset = 1000;

    int fd = open(FIB_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        exit(1);
    }

    for (int i = 0; i <= offset; i++) {
        lseek(fd, i, SEEK_SET);
        long long f0, f1;
        f1 = write(fd, write_buf, FIB_FAST_DOUBLING);
        f0 = write(fd, write_buf, FIB_NAIVE);

        printf("%d %lld %lld\n", i, f0, f1);
    }
    close(fd);
    return 0;
}
