#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

#define ll long long
#define min(a, b) ((a) < (b) ? (a) : (b))

#define MAX_PROCESS 16
#define MAX_N 1000
#define AVERAGE_TIME 10

ll a[MAX_N][MAX_N], b[MAX_N][MAX_N];

void matrixProduct(ll *c, int i, int processes) {
    int start = MAX_N / processes * i + min(i, MAX_N % processes);
    int end = start + MAX_N / processes + (i < (MAX_N % processes));
    for (int i = start; i < end; i++)
        for (int j = 0; j < MAX_N; j++)
            for (int k = 0; k < MAX_N; k++)
                c[i * MAX_N + j] += a[i][k] * b[k][j];
    return;
}

pid_t pids[MAX_PROCESS];
double createProcesses(ll* c, int processes) {
    memset(c, 0, sizeof(c) * MAX_N * MAX_N);

    struct timeval start;
    gettimeofday(&start, 0);

    for (int i = 0; i < processes; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            matrixProduct(c, i, processes);
            shmdt(c);
            exit(0);
        }
    }
    for (int i = 0; i < processes; i++)
        waitpid(pids[i], NULL, 0);

    struct timeval end;
    gettimeofday(&end, 0);

    return end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec) / 1000000.0 ;
}

int main() {
    for (int i = 0; i < MAX_N; i++)
        for (int j = 0; j < MAX_N; j++)
            a[i][j] = b[i][j] = (i * MAX_N + j) % 1000;

    int shmid = shmget(IPC_PRIVATE, sizeof(ll) * MAX_N * MAX_N, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("Failed to get shared memory");
        return -1;
    }
    
    ll *c = shmat(shmid, NULL, 0);
    for (int i = 1; i <= MAX_PROCESS; i++) {
        double sum = 0;
        for (int j = 0; j < AVERAGE_TIME; j++)
            sum += createProcesses(c, i);
        printf("%2d processes use %6.4lf seconds\n", i, sum / AVERAGE_TIME);
    }

    return 0;
}
