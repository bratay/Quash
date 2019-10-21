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
static char *curAction;

void performAction();

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

int setPath(char *action)
{
    char *setter = strtok(action, "=");
    char *ptype = setter;
    setter = strtok(NULL, "\0");
    char *path = setter;

    if ((setenv(ptype, path, 1)) == -1)
    {
        printf("%s not set correctly.\n", ptype);
    }
    return 1;
}

void showJobs()
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

void process()
{
    int status;
    pid_t pid, sid;
    pid = fork();
    if (pid == 0)
    {
        sid = setsid();
        if (sid < 0)
        {
            fprintf(stderr, "Process creation fail\n");
            exit(0);
        }
        printf("Process %d running out of %d processes\n", getpid(), numJobs + 1);

        char* backGroundAction = strdup(curAction);
        backGroundAction[strlen(curAction) - 1] = 0;
        curAction = backGroundAction;

        performAction();
        printf("Process %d is done\n", getpid());
        kill(getpid(), -9);
        exit(0);
    }
    else
    {
        struct Job new_job = {.pid = pid, .id = numJobs, .cmd = curAction};
        jobs[numJobs] = new_job;
        numJobs++;
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
                fprintf(stderr, "Invalid action\n");
                exit(0);
            }
        }
        else
        {
            if (execvp(cmds[0], NULL) < 0)
            {
                fprintf(stderr, "Invalid action\n");
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

void makePipe()
{
    char *part = strtok(curAction, "|\0");
    char *command = part;
    part = strtok(NULL, "\0");
    char *nextCommand = part;
    int spipe[2];
    int status;
    pipe(spipe);
    pid_t pid, pid2;
    pid = fork();
    if (pid == 0)
    {
        dup2(spipe[1], STDOUT_FILENO);
        curAction = clearWhitespace(command);
        performAction();
        close(spipe[0]);
        close(spipe[1]);
        exit(0);
    }
    pid2 = fork();
    if (pid2 == 0)
    {
        dup2(spipe[0], STDIN_FILENO);
        curAction = clearWhitespace(command);
        performAction();
        close(spipe[0]);
        close(spipe[1]);
        exit(0);
    }
}

void fileIN(char *args){
    char* cmdFile = NULL;

    size_t len = 0;
    ssize_t read;
    FILE* file_d;
    int status;
    pid_t pid;
    pid = fork();
    if (strcmp("quash", args[0]) == 0) {
        file_d = fopen(args[2], "r");
        do {
            read = getline(&cmdFile, &len, file_d);
            if (pid == 0) {
                performAction(clearWhitespace(cmdFile));
            } else {
                waitpid(pid, &status, 0);
                if(status == 1) {
                    fprintf(stderr, "%s\n", "ERROR\n");
                }
            }
        } while (read != -1);
        fclose(file_d);
    } else {
        file_d = fopen(args[2], "r");
        int x = 0;
        int position = 0;
        while (position == 0) {
            if (strcmp("<",args[x]) == 0) {
                position = x;
            }
            x++;
        }
        while ((read = getline(&cmdFile, &len, file_d)) == -1) {
            char* temp_i = strtok(cmdFile, " \0");
            args[x] = temp_i;
            x++;
            printf("%s\n",args[x]);
        }
        if (pid == 0) {
            execute(args);
        } else {
            waitpid(pid, &status, 0);
            if(status == 1) {
                fprintf(stderr, "%s\n", "ERROR\n");
            }
        }
    }
}

void fileOUT(char *args){
    int i, out;
    int position = 0;
    while (position == 0) {
        if (strcmp(">",args[i]) == 0) {
            position = i;
        }
        i++;
    }
    int status;
    pid_t pid;
    pid = fork();
    if (pid == 0) {
        char* filename = args[position + 1];
        out = open(filename, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
        dup2(out, STDOUT_FILENO);
        char* in = strtok(curAction, ">");
        performAction(clearWhitespace(in));
        close(out);
        exit(0);
    } else {
        waitpid(pid, &status, 0);
        if(status == 1) {
            fprintf(stderr, "%s\n", "ERROR\n");
        }
    }
}

void performAction()
{
    char *command;
    char *args[20];
    for (int i = 0; i < 20; i++)
    {
        args[i] = NULL;
    }
    int numArgs = 0;
    char *input = strdup(curAction);
    command = strtok(curAction, " ");
    while (command != NULL)
    {
        args[numArgs] = command;
        command = strtok(NULL, " ");
        numArgs++;
    }
    char *Args[19];
    for (int i = 0; i < 19; i++)
    {
        Args[i] = NULL;
    }
    for (int i = 1; i < 20; i++)
    {
        if (args[i] != NULL)
        {
            Args[i - 1] = args[i];
        }
    }

    if (strcmp("cd", args[0]) == 0)//CD
    {
        cd(args[1]);
    }
    else if (strcmp(args[0], "set") == 0)// Set Path
    {
        setPath(args[1]);
    }
    else if (strcmp(args[0], "jobs") == 0)//Show all jobs
    {
        showJobs();
    }
    else if (strchr(input, '|') != NULL)//Pipe creation
    {
        makePipe();
    }
    else if (strchr(input, '&') != NULL)//Background Process
    {
        process();
    }
    else if (strstr(input, "kill") != NULL)//Kill Job
    {
    }
    else if (strchr(input, '<') != NULL)//file in
    {
        fileIN(args);
    }
    else if (strchr(input, '>') != NULL)//file out
    {
        fileOUT(args);
    }
    else//Run executables
    {
        execute(args);
    }
}

int main(int argc, char **argv, char **envp)
{
    printf("\n\nQUASH\n\n");
    rl_bind_key('\t', rl_complete);
    char *action, prompt[128];
    env = getenv("USER");
    dir = getcwd(NULL, 1024);
    numJobs = 0;
    while(true)
    {
        snprintf(prompt, sizeof(prompt), "[%s:%s]$ ", env, dir);
        action = readline(prompt);
        action = clearWhitespace(action);
        if (strcmp("exit", action) != 0 && strcmp("quit", action) != 0)
        {
            if (strlen(action) > 1)
            {
                action = clearWhitespace(action);
                curAction = action;
                performAction();
            }
        }
        else
        {
            break;
        }
        free(action);
    }

    return 0;
}
