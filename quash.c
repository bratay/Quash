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

struct Job
{
    int pid;
    int id;
    char *cmd;
};

static int numJobs = 0;
static struct Job jobs[100];
static char *env;
static char *dir;

void setJobs(struct Job newJobs[])
{
    for (int i = 0; i < 100; i++)
    {
        jobs[i] = newJobs[i];
    }
}

char *clearWhitespace(char *str)
{
    char *end;

    while (isspace(*str))
        str++;
    if (*str == 0)
        return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace(*end))
        end--;
    *(end + 1) = 0;

    return str;
}

void cd(char *p)
{
    if (p == NULL)
    {
        chdir(getenv("HOME"));
        dir = getcwd(NULL, 1024);
    }
    else
    {
        if (chdir(p) == -1)
        {
            printf(" %s: No such file or directory\n", strerror(errno));
        }
        dir = getcwd(NULL, 1024);
    }
}

int setPath(char *input)
{
    char *setter = strtok(input, "=");
    char *ptype = setter;
    setter = strtok(NULL, "\0");
    char *path = setter;

    if ((setenv(ptype, path, 1)) == -1)
    {
        printf("%s not set correctly.\n", ptype);
    }
    return 1;
}

void show_jobs()
{
    int i;
    printf("\nActive jobs:\n");
    printf("| %7s  | %7s | %7s |\n", "Job ID", "PID  ", "Command");
    for (i = 0; i < numJobs; i++)
    {
        if (kill(jobs[i].pid, 0) == 0)
        {
            printf("|  [%7d] | %7d | %7s |\n", jobs[i].id, jobs[i].pid,
                   jobs[i].cmd);
        }
    }
}

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
        // processNewCommandFunct(command);
        printf("Process %d is done\n", getpid());
        kill(getpid(), -9);
        exit(0);
    }
    else
    {
        struct Job new_job = {.pid = pid, .id = numJobs, .cmd = command};
        // struct Job jobs[100] = getJobs();
        jobs[numJobs] = new_job;
        numJobs++;
        // setNumJobs(numJobs);
        while (waitpid(pid, NULL, WNOHANG | WEXITED) > 0)
        {
        }
    }
}

void execute(char **cmds)
{
    int status;
    pid_t pid;
    pid = fork();
    if (pid == 0)
    {
        if (strlen(cmds[0]) > 0)
        {
            if (execvp(cmds[0], cmds) < 0)
            {
                fprintf(stderr, "Invalid command\n");
                exit(0);
            }
        }
        else
        {
            if (execvp(cmds[0], NULL) < 0)
            {
                fprintf(stderr, "Invalid command\n");
                exit(0);
            }
        }
    }
    else
    {
        waitpid(pid, &status, 0);
        if (status == 1)
        {
            fprintf(stderr, "%s\n", "Status == 1\n");
        }
    }
}

void withoutArgs(){
    int status;
    pid_t pid;
    pid = fork();
    if (pid == 0)
    {
        if (strlen(cmds[0]) > 0)
        {
            waitpid(pid, &status, 0);
        if (status == 1)
        {
            fprintf(stderr, "%s\n", "Status == 1\n");
        }
 
        }
        else
        {
            fprintf(stderr, "Invalid command\n");
            exit(0);
        }
    }
    else
    {
        waitpid(pid, &status, 0);
        if (status == 1)
        {
            fprintf(stderr, "%s\n", "Status == 1\n");
        }
    }
}

int main(int argc, char **argv, char **envp)
{
    printf("\n\nQUASH\n\n");
    rl_bind_key('\t', rl_complete);
    char *input, prompt[128];
    env = getenv("USER");
    dir = getcwd(NULL, 1024);
    numJobs = 0;
    while (free)
    {
        snprintf(prompt, sizeof(prompt), "[%s:%s]$ ", env, dir);
        input = readline(prompt);
        input = clearWhitespace(input);
        if (strcmp("exit", input) != 0 && strcmp("quit", input) != 0)
        {
            if (strlen(input) > 1)
            {
                input = clearWhitespace(input);
            }
        }
        else
        {
            break;
        }
        free(input);
    }

    return 0;
}
