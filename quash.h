// #ifndef QUASH_H
// #define QUASH_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// #include "backgroundProcess.h"

typedef struct Job
{
    int pid;
    int id;
    char *cmd;
};

static int numJobs = 0;
static struct Job jobs[100];
static char *env;
static char *dir;

void setJobs(struct Job newJobs[]);

char *clearWhitespace(char *str);

void cd(char *p);

int setPath(char *input);

void show_jobs();

void execute(char **cmds);
