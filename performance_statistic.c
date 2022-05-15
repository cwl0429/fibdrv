#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>


#define FIB_DEV "/dev/fibonacci"
#define FIB_NAIVE 0
#define FIB_FAST_DOUBLING 1
#define TEST_CASE_CNT 1001

int main()
{
    char write_buf[] = "testing writing";
    int offset = 1000;
    int test_cnt = 1000;
    int fib_tmp[TEST_CASE_CNT];
    int fib_fast_tmp[TEST_CASE_CNT];
    int fd = open(FIB_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        exit(1);
    }

    for (int i = 0; i <= offset; i++) {
        for (int j = 0; j <= test_cnt; j++) {
            lseek(fd, i, SEEK_SET);
            int f0, f1;
            f0 = write(fd, write_buf, FIB_NAIVE);
            f1 = write(fd, write_buf, FIB_FAST_DOUBLING);
            fib_tmp[j] = f0;
            fib_fast_tmp[j] = f1;
        }
        /* find outliner */
        float fib_sd = 0, fib_fast_sd = 0;
        int fib_mean = 0, fib_fast_mean = 0;
        /* calculate mean */
        for (int j = 0; j <= test_cnt; j++) {
            fib_mean += fib_tmp[j];
            fib_fast_mean += fib_fast_tmp[j];
        }
        fib_mean /= test_cnt;
        fib_fast_mean /= test_cnt;
        /* calculate sd */
        for (int j = 0; j <= test_cnt; j++) {
            fib_sd += (fib_tmp[j] - fib_mean);
            fib_fast_sd += (fib_fast_tmp[j] - fib_fast_mean);
        }
        fib_sd = sqrt(fib_sd);
        fib_fast_sd = sqrt(fib_fast_sd);

        for (int j = 0; j <= test_cnt; j++) {
            if ((fib_tmp[j] > (fib_mean + 3 * fib_sd)) ||
                (fib_tmp[j] < (fib_mean - 3 * fib_sd))) {
                fib_tmp[j] = 0;
            }
            if ((fib_fast_tmp[j] > (fib_fast_mean + 3 * fib_fast_sd)) ||
                (fib_fast_tmp[j] < (fib_fast_mean - 3 * fib_fast_sd))) {
                fib_fast_tmp[j] = 0;
            }
        }

        int fib_cnt = 0, fib_fast_cnt = 0;
        fib_mean = 0;
        fib_fast_mean = 0;
        for (int j = 0; j <= test_cnt; j++) {
            if (fib_tmp[j] != 0) {
                fib_mean += fib_tmp[j];
                fib_cnt++;
            }
            if (fib_fast_tmp[j] != 0) {
                fib_fast_mean += fib_fast_tmp[j];
                fib_fast_cnt++;
            }
        }
        fib_mean /= fib_cnt;
        fib_fast_mean /= fib_fast_cnt;
        printf("%d %d %d\n", i, fib_mean, fib_fast_mean);
    }
    close(fd);
    return 0;
}
