#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/mman.h> // for mmap, munmap
#include <sys/stat.h> // for open, fstat
#include <fcntl.h>    // for open
#include <unistd.h>   // for close

#define SIZED_ARRAY 9

typedef struct
{
    char debug_mode;
    char file_name[128];
    int unit_size;
    unsigned char mem_buf[10000];
    size_t mem_count;
    /*
     .
     .
     Any additional fields you deem necessary
    */
} state;

typedef struct hexEditPlus
{
    char *name;
    void (*fun)(state *);
} hexEditPlus;

/* Global Varibles */
bool deBugMode = false;
bool displayFlag = false;

// MENU FUNCTIONS
void Quit(state *c);
void toggleDeBugMode(state *s);
void setFileName(state *s);
void setUnitSize(state *s);
void LoadIntoMemory(state *s);
void toggleDisplayMode(state *s);
void memoryDisplay(state *s);
void saveIntoFile(state *s);
void memoryModify(state *s);

// AST. MENU FUNCTIONS
char *print_units(state *s);
void print_debugs(state *s);
bool rangeCheck(int input);
void printthemenu(struct hexEditPlus *menu, state *s);

int main(int argc, char **argv)
{
    state *s = (state *)malloc(sizeof(state));
    char c[100];
    hexEditPlus menu[] = {
        {"Toggle debug mode", &toggleDeBugMode},
        {"Set file name", &setFileName},
        {"Set unit size", &setUnitSize},
        {"Load into memory", &LoadIntoMemory},
        {"Toggle display mode", &toggleDisplayMode},
        {"Memory display", &memoryDisplay},
        {"Save into file", &saveIntoFile},
        {"Memory modify", &memoryModify},
        {"Quit", &Quit},
        {NULL, NULL}};
    int intInput = 0;
    printthemenu(menu, s);
    while (true)
    {
        if (fgets(c, sizeof(c), stdin) != NULL)
        {
            intInput = atoi(c);
            if (rangeCheck(intInput))
            {
                menu[intInput].fun(s);
            }
            else
            {
                printf("Input is not in the bounds");
                exit(1);
            }
        }
        else
        {
            exit(1);
        }
        printf("\n\n ");
        printthemenu(menu, s);
    }
}

// function to print the menu components.
void printthemenu(struct hexEditPlus *menu, state *s)
{
    printf("\n******* NEW ITERATION ********\n\n\n");
    if (deBugMode)
    {
        fprintf(stderr, "Debug: Unit size: %d\n", s->unit_size);
        fprintf(stderr, "Debug: file name: %s\n", s->file_name);
        fprintf(stderr, "Debug: mem count: %d\n\n", s->mem_count);
    }

    int i = 0;
    printf("Choose action:\n");
    while (menu[i].name != NULL)
    {
        printf("%d)  %s\n", i, menu[i].name);
        i++;
    }
    printf("\nOption: ");
}

bool rangeCheck(int input)
{
    return input >= 0 && input < SIZED_ARRAY;
}

void toggleDeBugMode(state *s)
{
    deBugMode = !deBugMode;
    if (deBugMode)
        fprintf(stderr, "\nDebug: flag now on.\n");
    else
        fprintf(stderr, "\nDebug: flag now off.\n");
}

void setFileName(state *s)
{
    // printf("insert a new file name\n");
    // char newfilename[100];
    // scanf("%s", newfilename);
    // strcpy(s->file_name, newfilename);
    // if (deBugMode)
    //     printf("Debug: file name set to %s\n", s->file_name);
    printf("\ninsert a new file name: ");
    char newfilename[100];

    fgets(newfilename, sizeof(newfilename), stdin);
    if (newfilename[strlen(newfilename) - 1] == '\n')
        newfilename[strlen(newfilename) - 1] = '\0';

    strcpy(s->file_name, newfilename);
    if (deBugMode)
        printf("Debug: file name set to %s\n", s->file_name);
}

void Quit(state *s)
{
    if (deBugMode)
    {
        fprintf(stderr, "quitting\n");
    }
    free(s);
    exit(0);
}

void setUnitSize(state *s)
{
    printf("\nPlease, enter the unit size: ");
    char input[100];
    int unitscaf = 0;
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%d", &unitscaf);
    if ((unitscaf == 1) || (unitscaf == 2) || (unitscaf == 4))
    {
        if (deBugMode)
        {
            fprintf(stderr, "Debug: set size to %d.\n", unitscaf);
        }
        s->unit_size = unitscaf;
    }
    else
    {
        fprintf(stderr, "size is not valid\n");
    }
}

void LoadIntoMemory(state *s)
{
    int location = 0;
    int length = 0;
    char input[100];
    if (strlen(s->file_name) == 0)
    {
        fprintf(stderr, "file name is empty.\n");
    }
    else
    {
        FILE *fd = fopen(s->file_name, "r+");
        if (fd == NULL)
        {
            perror("open failed\n");
            exit(1);
        }

        printf("\nPlease enter location (hexadecimal) and length (decimal): ");
        fgets(input, sizeof(input), stdin);
        sscanf(input, "%x %d", &location, &length);

        fseek(fd, location, SEEK_SET);
        fread(s->mem_buf, s->unit_size, length, fd);
        s->mem_count = s->unit_size * length;
        printf("Loaded %d units into memory\n", length);

        if (deBugMode)
        {
            fprintf(stderr, "\nDebug: file name: %s\n", s->file_name);
            fprintf(stderr, "Debug: file location: %d\n", location);
            fprintf(stderr, "Debug: file len: %d", length);
        }

        fclose(fd);
    }
}

void toggleDisplayMode(state *s)
{
    displayFlag = !displayFlag;
    if (displayFlag)
        fprintf(stderr, "\nDisplay flag now on, hexadecimal representation.\n");
    else
        fprintf(stderr, "\nDisplay flag now off, decimal representation.\n");
}

void memoryDisplay(state *s)
{
    char input[100];
    off_t address;
    int u;
    printf("\nEnter address and length: (please seperate using spacebar): ");
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%lx %d", &address, &u);

    // Special case 0:`
    void *pointer;
    if (address == 0)
        pointer = &(s->mem_buf);
    else
        pointer = &address;

    if (displayFlag)
        printf("\nHexadecimal\n===========\n");
    else
        printf("\nDecimal\n=======\n");

    void *end = pointer + s->unit_size * u;
    char *formatshexa[] = {"%#hhX\n", "%#hX\n", "No such unit", "%#X\n"};
    char *formatsdec[] = {"%hhu\n", "%hu\n", "No such unit", "%u\n"};

    while (pointer < end)
    {
        // print ints
        int var = *((int *)(pointer));
        displayFlag ? printf(formatshexa[s->unit_size - 1], var) : printf(formatsdec[s->unit_size - 1], var);
        pointer += s->unit_size;
    }
}

char *print_units(state *s)
{
    if (displayFlag)
    {
        static char *formats[] = {"%#hhX\n", "%#hX\n", "No such unit", "%#X\n"};
        return formats[s->unit_size - 1];
    }
    else
    {
        static char *formats[] = {"%hhu\n", "%hu\n", "No such unit", "%u\n"};
        return formats[s->unit_size - 1];
    }
}

void saveIntoFile(state *s)
{
    char input[100];
    int source_address = 0;
    int target_location = 0;
    int length = 0;
    s->unit_size = 1; // reseting the unit size if not reseted.

    FILE *file = fopen((s->file_name), "r+");
    if (file != NULL)
    {
        printf("\nPlease enter <source-address> <target-location> <length>:\n");
        fgets(input, sizeof(input), stdin);
        sscanf(input, "%x %x %d", &source_address, &target_location, &length);
        fseek(file, 0L, SEEK_SET);
        fseek(file, 0L, SEEK_END);

        if (target_location > ftell(file))
        {
            perror("target location is greater than the size of filename");
        }
        else
        {
            printf("\n\n %ld\n", ftell(file));
            fseek(file, 0L, SEEK_SET);
            printf("\n\n %ld\n", ftell(file));
            fseek(file, target_location, SEEK_SET);
            printf("\n\n %ld\n", ftell(file));

            if (source_address != 0)
            {
                fwrite(&source_address, 1, length, file);
            }
            else
                fwrite(&(s->mem_buf), 1, length, file);
        }
        fclose(file);
    }
    else
    {
        perror("Unable to open the file\n");
        exit(1);
    }
}

void memoryModify(state *s)
{
    int location = 0;
    int value = 0;
    char input[100];
    printf("Please enter <location> <val>:\n");
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%x %x", &location, &value);
    if (deBugMode)
    {
        fprintf(stderr, "Debug: location: %x\n", location);
        fprintf(stderr, "Debug: value: %x\n", value);
    }
    // Copy unit_size bytes of value to mem_buf.
    // FILE *file1 = fopen("fawzioutput", "wb");

    // if (file1 != NULL)
    // {
    //     fwrite(s->mem_buf, s->mem_count, 1, file1);
    //     fclose(file1);
    //     printf("Memory buffer saved to file: %s\n", "fawzioutput");
    // }
    // else
    // {
    //     printf("Error opening the file\n");
    // }
    memcpy(&s->mem_buf[location], &value, s->unit_size);

    // // FOR DEBUGGING!!
    // FILE *file = fopen("fawzioutput", "wb");

    // if (file != NULL)
    // {
    //     fwrite(s->mem_buf, s->mem_count, 1, file);
    //     fclose(file);
    //     printf("Memory buffer saved to file: %s\n", "fawzioutput");
    // }
    // else
    // {
    //     printf("Error opening the file\n");
    // }
}
