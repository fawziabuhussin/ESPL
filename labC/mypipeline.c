#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <fcntl.h>

bool debugMode = false;
#define parent "parent_process"
#define child1 "child1"
#define child2 "child2"

pid_t child_process(char *args[], int closeId, int read_write[], int refactor);
void debug1(char *name, char *msg, int id);
void debug(char *name, char *msg);

int main(int argc, char **argv)
{
    char *args2[4] = {"tail", "-n", "2", NULL};
    char *args1[3] = {"ls", "-l", NULL};
    int read_write[2];

    pid_t pid;
    int fd;
    // printf("before for\n");

    for (int i = 0; i < argc; i++)
    {
        if (strncmp(argv[i], "-d", 2) == 0)
        {
            debugMode = true;
            // printf("debugmode on\n");
        }
    }
    // printf("after for\n");

    if (pipe(read_write) == -1)
    {
        fprintf(stderr, "Pipe failed\n");
        return 1;
    }
    // printf("after if\n");
    debug(parent, "forking...)");
    pid_t child_ls = child_process(args1, STDOUT_FILENO, read_write, 1);
    debug(parent, "closing the write end of the pipe...)");
    close(read_write[1]);
    debug1(parent, "created process with id:", pid);

    debug(parent, "forking...)");
    pid_t child_tail = child_process(args2, STDIN_FILENO, read_write, 0);
    debug(parent, "closing the read end of the pipe...)");
    close(read_write[0]);
    debug1(parent, "created process with id:", pid);

    debug(parent, "waiting for child processes to terminate...)");
    waitpid(child_ls, NULL, 0);
    waitpid(child_tail, NULL, 0);

    debug(parent, "exiting...)");

    return 0;
}
pid_t child_process(char *args[], int closeId, int read_write[], int refactor)
{
    pid_t pid;
    pid = fork();

    if (pid == 0) // child process
    {
        if (refactor)
        {
            debug(child1, "redirecting stdout to the write end of the pipe...)");
        }
        else
        {
            debug(child2, "redirecting stdin to the write end of the pipe...)");
        }
        close(closeId);
        dup(read_write[refactor]);
        close(read_write[refactor]);
        if (refactor)
        {
            debug(child1, "going to excute cmd...)");
        }
        else
        {
            debug(child2, "going to excute cmd...)");
        }
        execvp(args[0], args);
    }
    return pid;
}

void debug(char *name, char *msg)
{
    // printf("debug\n");

    if (debugMode)
    {
        // char *output = "(";
        fprintf(stderr, "%s%s%s%s\n", "(", name, ">", msg);
    }
    // printf(" affter debug\n");
}

void debug1(char *name, char *msg, int id)
{
    if (debugMode)
    {
        char *output = "(";
                fprintf(stderr, "%s%s%s%s%d%s\n", "(", name, ">", msg, id, ")");

    }
}