#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <signal.h>
#include "LineParser.h"
#include <fcntl.h>

#define BUFFERSIZE 2048
bool debugMode = false;

// Shell commands
void execute(cmdLine *pCmdLine);
void funCd(cmdLine *pCmdLine);

// PROGRAMS.
/*
    suspend <process id> - Suspend a running process (SIGTSTP).
    wake <process id> - Wake up a sleeping process (SIGCONT).
    kill <process id> - Terminate a running/sleeping process.
*/

void suspend(pid_t pid, cmdLine *pCmdLine);
void wake(pid_t pid, cmdLine *pCmdLine);
void my_kill(pid_t pid, cmdLine *pCmdLine);

// DEBUGGING FUNCS.
void debug(pid_t pid, cmdLine *pCmdLine);

int main(int argc, char **argv)
{
    char path[BUFFERSIZE];
    char buffer[BUFFERSIZE];
    cmdLine *parsedCmd = NULL;

    for (int i = 0; i < argc; i++)
    {
        if (strncmp(argv[i], "-d", 2) == 0)
        {
            debugMode = true;
        }
    }
    while (1)
    {
        if (getcwd(path, BUFFERSIZE) != NULL)
        {
            printf("The path is %s\n > ", path);
        }
        fgets(buffer, BUFFERSIZE, stdin);
        parsedCmd = parseCmdLines(buffer);
        if (parsedCmd == NULL)
        {
            continue;
        }
        // QUIT
        if (strncmp(parsedCmd->arguments[0], "quit", 4) == 0)
        {
            debug(getpid(), parsedCmd);
            break;
        }

        // CD
        // TASK 1c - CD COMMAND (Shell)
        else if (strncmp(parsedCmd->arguments[0], "cd", 2) == 0)
        {
            debug(getpid(), parsedCmd);
            funCd(parsedCmd);
        }
        // OTHERWISE.
        else
        {
            execute(parsedCmd);
        }
        freeCmdLines(parsedCmd); // TODO
    }
    return 0;
}

void execute(cmdLine *pCmdLine)
{
    int pid = fork(); // TASK 1a - FORKING.
    int status;
    int fd;

    if (pid == 0)
    {
        pid_t p_id = getpid();
        // TASK 3 :
        // Redirect input
        if (pCmdLine->inputRedirect != NULL)
        {
            printf("INPUT ETNERED \n");
            fd = open(pCmdLine->inputRedirect, O_RDWR | O_CREAT, 0777);
            printf("%s", pCmdLine->inputRedirect);
            if (fd == -1)
            {
                perror("open failed\n");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        // Redirect output
        if (pCmdLine->outputRedirect != NULL)
        {
            printf("OUTPUT ETNERED \n");
            fd = open(pCmdLine->outputRedirect, O_RDWR | O_CREAT, 0777);
            if (fd == -1)
            {
                perror("open failed\n");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        if (strncmp(pCmdLine->arguments[0], "suspend", 7) == 0)
        {
            suspend(p_id, pCmdLine);
        }

        else if (strncmp(pCmdLine->arguments[0], "wake", 4) == 0)
        {
            wake(p_id, pCmdLine);
        }
        else if (strncmp(pCmdLine->arguments[0], "kill", 4) == 0)
        {
            my_kill(p_id, pCmdLine);
        }
        else
        {
            if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1)
            {
                perror("execvp failed\n");
                _exit(EXIT_FAILURE);
            }
        }
        /*
        TASK 2 REDIRECTIONS FOR CHILD ONLY.
        */
    }
    else if (pid > 0)
    {
        // TASK1b - WAITING MODE.

        if (pCmdLine->blocking)
        {
            waitpid(pid, &status, 0);
        }
    }
    debug(getpid(), pCmdLine);
}

void debug(pid_t pid, cmdLine *pCmdLine)
{
    if (debugMode)
    {
        fprintf(stderr, "PID : %d.\nExecuting command : %s.\n", pid, pCmdLine->arguments[0]);
    }
}

void funCd(cmdLine *pCmdLine)
{
    if (chdir(pCmdLine->arguments[1]) == -1)
    {
        perror("chdir failed\n");
    }
}
void suspend(pid_t pid, cmdLine *pCmdLine)
{
        if (kill(atoi(pCmdLine->arguments[1]), SIGTSTP) == -1)
        {
            perror("suspend failed\n");
            _exit(EXIT_FAILURE);
        }
}
void wake(pid_t pid, cmdLine *pCmdLine)
{
    if (kill(atoi(pCmdLine->arguments[1]), SIGCONT) == -1)
    {
        perror("wake failed\n");
        _exit(EXIT_FAILURE);
    }
}
void my_kill(pid_t pid, cmdLine *pCmdLine)
{
    int i = kill(atoi(pCmdLine->arguments[1]), SIGTERM);
    if (i == -1)
    {
        perror("kill failed\n");
        _exit(EXIT_FAILURE);
    }
}
