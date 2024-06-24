#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <signal.h>
#include "LineParser.h"
#include <fcntl.h>
#include <time.h>

#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0
#define HISTLEN 20

typedef struct process
{
    cmdLine *cmd;         /* the parsed command line*/
    pid_t pid;            /* the process id that is running the command*/
    int status;           /* status of the process: RUNNING/SUSPENDED/TERMINATED */
    struct process *next; /* next process in chain */
} process;

typedef struct history_process
{
    cmdLine *cmd;
    int index;
    time_t time;
    struct history_process *next;
} history_process;
process **proclist = NULL;
history_process *history_list[HISTLEN];
int counter = 0;
history_process *newest = NULL;
history_process *oldest = NULL;

#define BUFFERSIZE 2048
bool debugMode = false;

// Shell commands
void execute(process **process_list, cmdLine *pCmdLine, cmdLine *pCmdLine2);
void funCd(cmdLine *pCmdLine);

pid_t child_process(cmdLine *cLine, char *const *args, int closeId, int read_write[], int refactor);
// Process Functions
void addProcess(process **process_list, cmdLine *cmd, pid_t pid, int status);
void printProcessList(process **process_list);
void freeProcessList(process **process_list);
void updateProcessList(process **process_list);
void updateProcessStatus(process *process_list, int pid, int status);
void deleteProcess(process **process_list, process *pro);
char *getLineCommand(cmdLine *cmdLineP);
// HISTORY FUNCTIONS
void addProcessHistory(cmdLine *cmd);
void deleteHistoryProcess(int mod);
void printHistoryList();
void freeHistoryList();
// PROGRAMS.
/*
    suspend <process id> - Suspend a running process (SIGTSTP).
    wake <process id> - Wake up a sleeping process (SIGCONT).
    kill <process id> - Terminate a running/sleeping process.
*/
void suspend(pid_t pid, cmdLine *pCmdLine);
void wake(pid_t pid, cmdLine *pCmdLine);
void my_kill(pid_t pid, cmdLine *pCmdLine);
void execute2(cmdLine *pCmdLine1, cmdLine *pCmdLine2);
void executeHelper(process **processesList, cmdLine *parsedCmd, cmdLine *parsedCmd2);

// DEBUGGING FUNCS.
void debug(pid_t pid, cmdLine *pCmdLine);

int main(int argc, char **argv)
{
    char path[BUFFERSIZE];
    char buffer[BUFFERSIZE];
    cmdLine *parsedCmd = NULL;
    cmdLine *parsedCmd2 = NULL;
    process **processesList = malloc(sizeof(process *));
    proclist = processesList;
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
        // printf("%s\n", buffer);
        parsedCmd = parseCmdLines(buffer);
        parsedCmd2 = parseCmdLines(buffer);
        if (strncmp(parsedCmd->arguments[0], "quit", 4) == 0)
        {
            debug(getpid(), parsedCmd);

            freeProcessList(processesList);
            freeHistoryList();

            freeCmdLines(parsedCmd);
            freeCmdLines(parsedCmd2);
            break;
        }
        if (parsedCmd == NULL)
        {
            continue;
        }
        executeHelper(processesList, parsedCmd, parsedCmd2);
        // freeCmdLines(parsedCmd);
    }
    free(processesList);
    return 0;
}

void executeHelper(process **processesList, cmdLine *parsedCmd, cmdLine *parsedCmd2)
{
    // QUIT

    if (strncmp(parsedCmd->arguments[0], "procs", 5) == 0)
    {
        addProcessHistory(parsedCmd2);
        printProcessList(processesList);
    }
    else if (strncmp(parsedCmd->arguments[0], "!!", 2) == 0)
    {
        int len = 0;
        if (newest)
        {
            char *str = getLineCommand(newest->cmd);
            cmdLine *ast = parseCmdLines(str);
            cmdLine *ast1 = parseCmdLines(str);

            // printf("\n RESULT IS %s : \n ", str);
            executeHelper(processesList, ast, ast1);
        }
    }
    else if (parsedCmd->arguments[0][0] == '!')
    {
        int num = atoi(&(parsedCmd->arguments[0][1]));
        if (num <= 0 || num > counter || !history_list[(num % HISTLEN) - 1])
            fprintf(stdout, "Command does not exist yet, try again please.\n");
        else
        {
            history_process *hp = history_list[(num % HISTLEN) - 1];
            char *str = getLineCommand(hp->cmd);
            cmdLine *ast = parseCmdLines(str);
            cmdLine *ast1 = parseCmdLines(str);
            executeHelper(processesList, ast, ast1);
        }
    }
    // CD
    // TASK 1c - CD COMMAND (Shell)
    else if (strncmp(parsedCmd->arguments[0], "cd", 2) == 0)
    {
        addProcessHistory(parsedCmd2);
        debug(getpid(), parsedCmd);
        funCd(parsedCmd);
    }
    else if (strncmp(parsedCmd->arguments[0], "suspend", 7) == 0)
    {
        addProcessHistory(parsedCmd2);
        suspend(getpid(), parsedCmd);
        updateProcessStatus(*processesList, atoi(parsedCmd->arguments[1]), SUSPENDED);
    }

    else if (strncmp(parsedCmd->arguments[0], "wake", 4) == 0)
    {
        addProcessHistory(parsedCmd2);
        wake(getpid(), parsedCmd);
        updateProcessStatus(*processesList, atoi(parsedCmd->arguments[1]), RUNNING);
    }
    else if (strncmp(parsedCmd->arguments[0], "kill", 4) == 0)
    {
        addProcessHistory(parsedCmd2);
        my_kill(getpid(), parsedCmd);
        updateProcessStatus(*processesList, atoi(parsedCmd->arguments[1]), TERMINATED);
    }
    else if (strncmp(parsedCmd->arguments[0], "history", 6) == 0)
    {
        addProcessHistory(parsedCmd2);
        printHistoryList(history_list);
    }
    // OTHERWISE.
    else
    {
        if (parsedCmd->next) // First Cmd -> SecondCmd -> Null. otherwise continue.
        {
            if (!parsedCmd->next->next) // ! cmd | cmd | cmd..
            {
                addProcessHistory(parseCmdLines(getLineCommand(parsedCmd)));
                execute2(parsedCmd, parsedCmd->next);
            }
        }
        else
        {
            execute(processesList, parsedCmd, parsedCmd2);
        }
    }
}

void execute(process **process_list, cmdLine *pCmdLine, cmdLine *pCmdLine2)
{
    int pid = fork(); // TASK 1a - FORKING.
    int status;
    int fd;

    if (pid == 0)
    {
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

        if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1)
        {
            perror("execvp failed\n");
            _exit(EXIT_FAILURE);
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
    addProcess(process_list, pCmdLine, pid, RUNNING);
    if (pCmdLine2)
        addProcessHistory(pCmdLine2);
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
bool flag = true;

void execute2(cmdLine *pCmdLine1, cmdLine *pCmdLine2)
{
    // addProcessHistory(pCmdLine1);
    int read_write[2];
    pid_t pid;
    int fd = 0;

    if (pipe(read_write) == -1)
    {
        fprintf(stderr, "Pipe failed\n");
        exit(1);
    }

    addProcess(proclist, pCmdLine1, pid, RUNNING);

    char * const* args_child1 = pCmdLine1->arguments;
    flag = true;
    pid_t child_ls = child_process(pCmdLine1, args_child1, STDOUT_FILENO, read_write, 1);
    close(read_write[1]);
    flag = false;
    char *const *args_child2 = pCmdLine2->arguments;
    pid_t child_tail = child_process(pCmdLine2, args_child2, STDIN_FILENO, read_write, 0);
    close(read_write[0]);

    waitpid(child_ls, NULL, 0);
    waitpid(child_tail, NULL, 0);
    flag = true;
}

pid_t child_process(cmdLine *cLine, char *const *args, int closeId, int read_write[], int refactor)
{
    pid_t pid;
    pid = fork();
    int fd = 0;
    if (pid == 0) // child process
    {
        if (cLine->inputRedirect != NULL && flag)
        {
            fd = fclose(stdin);

            if (fd < 0 || fopen(cLine->inputRedirect, "r") == NULL)
            {
                perror("open failed\n");
                exit(1);
            }
            flag = false;
        }
        // Redirect output
        if (cLine->outputRedirect != NULL && !flag)
        {
            fd = fclose(stdout);
            if (fd < 0 || fopen(cLine->outputRedirect, "w") == NULL)
            {
                perror("open failed\n");
                exit(1);
            }
            flag = false;
        }
        close(closeId);
        dup(read_write[refactor]);
        close(read_write[refactor]);
        execvp(args[0], args);
    }
    return pid;
}

void addProcess(process **process_list, cmdLine *cmd, pid_t pid, int status)
{
    process *new_process = (process *)malloc(sizeof(process));
    new_process->cmd = cmd;
    new_process->pid = pid;
    new_process->status = status;
    new_process->next = NULL;

    if (*process_list == NULL)
    {
        *process_list = new_process;
        return;
    }
    process *current = *process_list;
    while (current->next != NULL)
    {
        current = current->next;
    }
    current->next = new_process;
}

void printProcessList(process **process_list)
{
    updateProcessList(process_list);
    int counter = 1;
    fprintf(stdout, "id \t PID \t\t Command \t STATUS\n");
    process *current = *process_list;

    while (current != NULL)
    {
        fprintf(stdout, "(%d) \t %d \t\t %s \t\t", counter++, current->pid, current->cmd->arguments[0]);
        switch (current->status)
        {
        case RUNNING:
            printf("Running\n");
            break;
        case SUSPENDED:
            printf("Suspended\n");
            break;
        case TERMINATED:
            printf("Terminated\n");
            break;
        default:
            printf("WRONG STATUS\n");
        }
        if (current->status == TERMINATED)
        {
            deleteProcess(process_list, current);
        }
        current = current->next;
    }
}

void freeProcessList(process **process_list)
{
    if (*process_list != 0)
    {
        freeProcessList(&(*process_list)->next);
        freeCmdLines((*process_list)->cmd);
        free(*process_list);
    }
}

void updateProcessList(process **process_list)
{
    process *current = *process_list;
    while (current != NULL)
    {

        waitpid(current->pid, &(current->status), WNOHANG);
        current = current->next;
    }
}

void updateProcessStatus(process *process_list, int pid, int status)
{
    process *current = process_list;
    while (current != NULL)
    {
        if (current->pid == pid)
        {
            current->status = status;
        }
        current = current->next;
    }
}

void deleteProcess(process **process_list, process *pro)
{
    process *current = *process_list;
    process *previous = NULL;

    while (current != NULL && current->pid != pro->pid)
    {
        previous = current;
        current = current->next;
    }

    if (current == NULL)
    {
        return;
    }

    if (previous == NULL)
    {
        *process_list = current->next;
    }
    else
    {
        previous->next = current->next;
    }

    freeCmdLines(current->cmd);
    free(current);
}

void printHistoryList(history_process **history_list)
{
    char time_string[9];
    int j = 0;
    // printf("INDEX TIME COMMAND ARGS\n");
    for (int i = 0; i < HISTLEN; i++)
    {
        if (history_list[i])
        {
            struct tm *timeinfo = localtime(&history_list[i]->time);
            strftime(time_string, sizeof(time_string), "%H:%M:%S", timeinfo);
            fprintf(stdout, "%d %s %s ", history_list[i]->index, time_string, history_list[i]->cmd->arguments[0]);
            for (int j = 1; j < history_list[i]->cmd->argCount; j++)
            {
                fprintf(stdout, "%s ", history_list[i]->cmd->arguments[j]);
            }

            if ((history_list[i]->cmd->next))
            {
                fprintf(stdout, "| %s ", history_list[i]->cmd->next->arguments[0]);
                for (int j = 1; j < history_list[i]->cmd->next->argCount; j++)
                {
                    fprintf(stdout, "%s ", history_list[i]->cmd->next->arguments[j]);
                }
            }
            fprintf(stdout, "\n");
        }
    }
}

void addProcessHistory(cmdLine *cmd)
{
    if (counter >= HISTLEN)
        deleteHistoryProcess(counter);

    history_process *new_process = malloc(sizeof(history_process));
    new_process->cmd = cmd;
    new_process->index = (counter % HISTLEN) + 1;
    new_process->time = time(NULL);
    new_process->next = NULL;
    history_list[(counter % HISTLEN)] = new_process;
    newest = new_process;
    counter++;
}

void deleteHistoryProcess(int mod)
{
    if (history_list[mod % HISTLEN] != NULL)
    {
        if (history_list[mod % HISTLEN]->cmd)
            freeCmdLines(history_list[mod % HISTLEN]->cmd);
        free(history_list[mod % HISTLEN]);
    }
}

void freeHistoryList()
{
    for (int i = 0; i < HISTLEN; i++)
    {
        deleteHistoryProcess(i);
    }
}

char *getLineCommand(cmdLine *cmdLineP)
{
    int len = 0;
    for (int i = 0; i < cmdLineP->argCount; i++)
    {
        len += strlen(cmdLineP->arguments[i]);
    }
    if (cmdLineP->next)
    {
        for (int i = 0; i < cmdLineP->next->argCount; i++)
            len += strlen(cmdLineP->next->arguments[i]);
        len += cmdLineP->next->argCount + 100;
    }
    char *str = (char *)malloc(len + cmdLineP->argCount + 1);
    strcpy(str, cmdLineP->arguments[0]);
    if (cmdLineP->blocking)
        strcat(str, "&");
    for (int i = 1; i < cmdLineP->argCount; i++)
    {
        strcat(str, " ");
        strcat(str, cmdLineP->arguments[i]);
    }
    if (cmdLineP->next)
    {
        strcat(str, " | ");
        strcat(str, cmdLineP->next->arguments[0]);
        if (!cmdLineP->next->blocking)
            strcat(str, "&");
        for (int i = 1; i < cmdLineP->next->argCount; i++)
        {
            strcat(str, " ");
            strcat(str, cmdLineP->next->arguments[i]);
        }
    }
    return str;
}