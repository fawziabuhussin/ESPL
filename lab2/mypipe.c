#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>

int main()
{
    char buffer[sizeof("hello")];
    int read_write[2];
    pid_t pid;

    if (pipe(read_write) == -1)
    {
        fprintf(stderr, "Pipe failed\n");
        return 1;
    }

    pid = fork();

    if (pid == 0) // child process
    {
        close(read_write[0]);
        write(read_write[1], "hello", sizeof("hello"));
        close(read_write[1]);
    }
    else
    {
        close(read_write[1]);
        read(read_write[0], buffer, sizeof(buffer));
        printf("%s\n", buffer);
        close(read_write[0]);
    }

    return 0;
}
