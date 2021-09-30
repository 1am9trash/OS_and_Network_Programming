#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>

#define BUF_SIZE 4096
#define max(a,b) ((a) > (b) ? (a) : (b))

const char split[2] = " ";

char buf[BUF_SIZE];
char *argv[BUF_SIZE];
int cmds[BUF_SIZE];
int cmdsCnt;

bool nonSync;

bool processCmds() {
    char *p;
    int pos = 0;
    bool fst = true;

    nonSync = false;
    cmdsCnt = 1;
    cmds[0] = 0;

    fgets(buf, BUF_SIZE, stdin);
    buf[max(0, strlen(buf) - 1)] = '\0';

    p = strtok(buf, split);
    while (p != NULL) {
        if (strlen(p) == 0)
            continue;
        if (fst && strcmp(p, "exit") == 0)
            return 0;
        fst = false;
        if (strcmp(p, "&") == 0) {
            nonSync = true;
            break;
        }
        if (strcmp(p, "|") == 0) {
            argv[pos++] = NULL;
            cmds[cmdsCnt++] = pos;
        } else 
            argv[pos++] = p;
        p = strtok (NULL, split);
    }    

    argv[pos] = NULL;
    if (pos == 0)
        cmdsCnt = 0;

    return 1;
}

void execCmds(int cur, int in) {
    if (cur == cmdsCnt) {
        if (in != -1)
            close(in);
        return;
    }

    int fds[2];
    if (pipe(fds) == -1) {
        perror("pipe() error");
        return;
    }

    pid_t childPid = fork();
    switch(childPid) {
        case -1:
            perror("execCmd fork() error");
            break;
        case 0:
            if (cur != cmdsCnt - 1)
                dup2(fds[1], STDOUT_FILENO);
            if (cur != 0)
                dup2(in, STDIN_FILENO);
            close(fds[0]);
            close(fds[1]);
            if (in != -1)
                close(in);
            if (execvp(argv[cmds[cur]], argv + cmds[cur]) < 0)
                perror("command error");
            break;
        default:
            if (nonSync)
                waitpid(childPid, NULL, WNOHANG);
            else
                waitpid(childPid, NULL, 0);
            close(fds[1]);
            if (in != 1)
                close(in);
            execCmds(cur + 1, fds[0]);
            break;
    }
    return;
}

int main() {
    while (1) {
        printf("shell> ");
        if (!processCmds())
            break;
        
        pid_t pid = fork();
        switch(pid) {
            case -1: 
                perror("main fork() error");
                break;
            case 0:
                execCmds(0, -1);
                return 0;
            default:
                waitpid(pid, NULL, 0);
                break;
        }
    }
    return 0;
}
