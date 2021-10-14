#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <string.h>

#define ll long long
#define min(a, b) ((a) < (b) ? (a) : (b))

#define MAX_THREAD 16
#define MAX_N 1000
#define AVERAGE_TIME 10

ll a[MAX_N][MAX_N], b[MAX_N][MAX_N], c[MAX_N][MAX_N];

struct parameter {
    int i, tot;
};

void *matrixProduct(void *para) {
    struct parameter *p = para;
    int start = MAX_N / p->tot * p->i + min((p->i), (MAX_N % p->tot));
    int end = start + MAX_N / p->tot + (p->i < (MAX_N % p->tot));
    for (int i = start; i < end; i++)
        for (int j = 0; j < MAX_N; j++)
            for (int k = 0; k < MAX_N; k++)
                c[i][j] += a[i][k] * b[k][j];
    return ((void *)0);
}

pthread_t tids[MAX_THREAD];
struct parameter para[MAX_THREAD];
double createThreads(int threads) {
    memset(c, 0, sizeof(c));

    struct timeval start;
    gettimeofday(&start, 0);

    for (int i = 0; i < threads; i++) {
        para[i].i = i;
        para[i].tot = threads;
        if (pthread_create(tids + i, NULL, matrixProduct, para + i) != 0) {
            perror("Failed to create thread");
            return -1;
        }
    }
    for (int i = 0; i < threads; i++)
        if (pthread_join(tids[i], NULL) != 0) {
            perror("Failed to join thread");
            return -1;
        }

    struct timeval end;
    gettimeofday(&end, 0);
    return end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec) / 1000000.0 ;
}

int main() {
    for (int i = 0; i < MAX_N; i++)
        for (int j = 0; j < MAX_N; j++)
            a[i][j] = b[i][j] = (i * MAX_N + j) % 1000;

    for (int i = 1; i <= MAX_THREAD; i++) {
        double sum = 0;
        for (int j = 0; j < AVERAGE_TIME; j++)
            sum += createThreads(i);
        printf("%2d Threads use %6.4lf seconds\n", i, (double)sum / AVERAGE_TIME);
    }

    return 0;
}
