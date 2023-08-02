#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define SIZED_ARRAY 5

char my_get(char c);
char cprt(char c);
char encrypt(char c);
char decrypt(char c);
char xprt(char c);
char *map(char *array, int array_length, char (*f)(char));
bool rangeCheck(int input);

typedef struct fun_desc
{
    char *name;
    char (*fun)(char);
} fundesc;

int main()
{
    char c[100];
    char *carray = (char *)malloc(SIZED_ARRAY * sizeof(char));
    char *carryHolder = NULL;
    fundesc menu[] = {
        {"Get string", &my_get},
        {"Print string", &cprt},
        {"Print hex", &xprt},
        {"Encrypt", &encrypt},
        {"Decrypt", &decrypt},
        {NULL, NULL}};
    int i = 0;
    int intInput = 0;

    while (true)
    {
        printf("Select operation from the following menu:\n");
        while (menu[i].fun != NULL)
        {
            printf("%d) %s\n", i, menu[i].name);
            i++;
        }
        i = 0;
        printf("Option : ");
        if (fgets(c, sizeof(c), stdin) != NULL)
        {
            intInput = atoi(c);
            if (rangeCheck(intInput))
            {
                printf("Within Bounds\n");
                carryHolder = map(carray, SIZED_ARRAY, menu[intInput].fun);
                free(carray);
                carray = carryHolder;
                printf("DONE.\n\n");
            }
            else
            {
                printf("Input is not in the bounds");
                free(carray);
                exit(1);
            }
        }
        else {
            exit(1);
        }
    }
}

bool rangeCheck(int input)
{
    return input >= 0 && input <= 4;
}

char my_get(char c)
{
    // Ignores c, reads and returns a character from stdin using fgetc.
    return (char)fgetc(stdin);
}

char cprt(char c)
{
    // If c is a number between 0x20 and 0x7E, cprt prints the character of ASCII value c followed by a new line. Otherwise, cprt prints the dot ('.') character. After printing, cprt returns the value of c unchanged.
    if ((c >= 0x20) && (c <= 0x7E))
        printf("%c\n", c);
    else
        printf(".\n");
    return c;
}

char encrypt(char c)
{
    /* Gets a char c and returns its encrypted form by adding 1 to its value. If c is not between 0x20 and 0x7E it is returned unchanged */
    if ((c >= 0x20) && (c <= 0x7E))
        return c + 1;
    else
        return c;
}
char decrypt(char c)
{
    /* Gets a char c and returns its decrypted form by reducing 1 from its value. If c is not between 0x20 and 0x7E it is returned unchanged */
    if ((c >= 0x20) && (c <= 0x7E))
        return c - 1;
    else
        return c;
}

char xprt(char c)
{
    if ((c >= 0x20) && (c <= 0x7E))
        printf("%x\n", c);
    else
        printf(".\n");
    return c;
}

char *map(char *array, int array_length, char (*f)(char))
{
    char *mapped_array = (char *)(malloc(array_length * sizeof(char)));
    /* TODO: Complete during task 2.a */
    for (int i = 0; i < array_length; i++)
    {
        mapped_array[i] = (*f)(array[i]);
    }
    return mapped_array;
}
