#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/mman.h>
#include <signal.h>
#include <termios.h>
#include <readline/readline.h>
#include <readline/history.h>
#define true 1
#define false 0

// #include "backgroundProcess.h"

void process(int numJobs, char* command)
{
    int status;
    pid_t pid, sid;
    pid = fork();
    if (pid == 0)
    {
        sid = setsid();
        if (sid < 0)
        {
            fprintf(stderr, "Unable to create new process\n");
            exit(0);
        }
        printf("New process with pid %d running out of %d processes\n", getpid(), numJobs + 1);
        parse_cmd(command);
        printf("Process %d is done\n", getpid());
        kill(getpid(), -9);
        exit(0);
    }
    else
    {
        struct Job new_job = {.pid = pid, .id = numJobs, .cmd = command};
        struct Job jobs[100] = getJobs();
        jobs[numJobs] = new_job;
        numJobs++;
        setNumJobs(numJobs);
        while (waitpid(pid, NULL, WNOHANG | WEXITED) > 0)
        {
        }
    }
}