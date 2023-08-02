#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define BUFFERSIZE 10000

typedef struct virus
{
    unsigned short SigSize;
    char virusName[16];
    unsigned char *sig;
} virus;

typedef struct link link;

struct link
{
    link *nextVirus;
    virus *vir;
};

struct fun_desc
{
    char *name;
    link *(*fun)(link *);
};

// MENU FUNCTIONS
void printthemenu(struct fun_desc *menu);
link *loadSig(link *virus_list);
link *printSig(link *virus_list);
link *detectViruses(link *virus_list);
link *fixFile(link *virus_list);
link *Quit(link *virus_list);

// VIRUS FUNCTIONS
virus *readVirus(FILE *fp, int flag);
void printVirus(virus *virus, FILE *output);
void detect_virus(char *buffer, unsigned int size, link *virus_list);
void neutralize_virus(char *fileName, int signatureOffset);
void PrintHex(char *buffer, size_t length, FILE *output);

//LIST FUNCTIONS
void list_print(link *virus_list, FILE *output);
link *list_append(link *virus_list, virus *data);
void list_free(link *virus_list);


// GLOBAL VARIABLES
char **argv;
int argc;

int main(int argc_temp, char **argv_temp)
{
    argc = argc_temp;
    argv = argv_temp;
    int id = 0;
    char value[50];
    FILE *out_file = stdout;
    link *virus_list = NULL;
    struct fun_desc menu_fun[] = {
        {"Load signatures ", &loadSig},
        {"Print signatures", &printSig},
        {"Detect viruses", &detectViruses},
        {"Fix file", &fixFile},
        {"Quit ", &Quit},
        {NULL, NULL}};

    printthemenu(menu_fun);

    while (true)
    {
        fgets(value, sizeof(value), stdin);
        id = atoi(value) - 1;
        if (id >= 0 && id <= 4)
        {
            printf("\nWithin Bounds\n");
            virus_list = menu_fun[id].fun(virus_list);
            printf("DONE.\n\n");
        }
        else
        {
            printf("Input is not in the bounds");
            Quit(virus_list);
        }
        printthemenu(menu_fun);
    }

    return 0;
}

// function to print the menu components.
void printthemenu(struct fun_desc *menu)
{
    int i = 0;
    printf("Select operation from the following menu:\n");
    while (menu[i].fun != NULL)
    {
        printf("%d)  %s\n", i + 1, menu[i].name);
        i++;
    }
    printf("Option: ");
}

virus *readVirus(FILE *fp, int flag)
{
    virus *v = malloc(sizeof(virus));
    fread(v, sizeof(short) + 16 * sizeof(char), 1, fp);
    // check if Big endian
    if (flag == 2)
    {
        v->SigSize = (v->SigSize >> 8) | (v->SigSize << 8);
    }
    v->sig = malloc((v->SigSize) * (sizeof(char)));
    fread(v->sig, v->SigSize, 1, fp);
    return v;
}

void PrintHex(char *buffer, size_t length, FILE *output)
{
    for (int i = 0; i < length; i++)
    {
        fprintf(output, "%02X ", buffer[i] & 0xFF);
    }
    fprintf(output, "\n");
}

void printVirus(virus *virus, FILE *output)
{
    /*
         Virus name:
         Virus size:
         signature:
     */
    fprintf(output, "Virus name: %s\n", virus->virusName);
    fprintf(output, "Virus size: %d\n", virus->SigSize);
    fprintf(output, "signature:\n");
    PrintHex(virus->sig, virus->SigSize, output);
    fprintf(output, "\n");
}

void list_print(link *virus_list, FILE *output)
{
    while (virus_list)
    {
        printVirus(virus_list->vir, output);
        fprintf(output, "\n");
        virus_list = virus_list->nextVirus;
    }
}

link *list_append(link *virus_list, virus *data)
{
    link *newLink = malloc(sizeof(link));
    newLink->vir = data;
    newLink->nextVirus = virus_list;
    return newLink;
}

void list_free(link *virus_list)
{
    if (virus_list != 0)
    {
        list_free(virus_list->nextVirus);
        free(virus_list->vir->sig);
        free(virus_list->vir);
        free(virus_list);
    }
}

void detect_virus(char *buffer, unsigned int size, link *virus_list)
{
    for (int byte = 0; byte < size; byte++)
    {
        link *curr = virus_list;
        while (curr)
        {
            size_t sigSize = curr->vir->SigSize;
            // check bounds
            if (byte + sigSize < size)
            {
                if (memcmp(buffer + byte, curr->vir->sig, sigSize) == 0)
                {
                    fprintf(stdout, "The starting byte location in the suspected file is: %d\n", byte);
                    fprintf(stdout, "The virus name is: %s\n", curr->vir->virusName);
                    fprintf(stdout, "The size of the virus signature is: %d\n", sigSize);
                }
            }
            curr = curr->nextVirus;
        }
    }
}

void neutralize_virus(char *fileName, int signatureOffset)
{
    char RET[] = {0xC3};

    FILE *in_file = fopen(fileName, "r+");
    if (in_file == NULL)
    {
        fprintf(stderr, "Error: Couldn't open file\n");
        exit(1);
    }
    fseek(in_file, signatureOffset, SEEK_SET);
    fwrite(RET, sizeof(char), 1, in_file);
    fclose(in_file);
}

link *loadSig(link *virus_list)
{
    int flag = 0;
    char temp_name[100];
    char filename[100];
    virus *virus = NULL;

    fgets(temp_name, 100, stdin);
    sscanf(temp_name, "%s", filename);

    FILE *in_file = fopen(filename, "r");
    if (in_file == NULL)
    {
        fprintf(stderr, "Error: Couldn't open file\n");
        Quit(virus_list);
    }

    char magicNumber[4];
    fread(magicNumber, 4 * sizeof(char), 1, in_file);
    if (strncmp(magicNumber, "VISL", 4) == 0)
    {
        flag = 1;
    }
    else if (strncmp(magicNumber, "VISB", 4) == 0)
    {
        flag = 2;
    }
    else
    {
        fprintf(stderr, "Invalid magic number");
        fclose(in_file);
        Quit(virus_list);
    }

    while (in_file != NULL && fgetc(in_file) != EOF)
    {
        fseek(in_file, -1, SEEK_CUR);
        virus = readVirus(in_file, flag);
        virus_list = list_append(virus_list, virus);
    }

    if (ferror(in_file))
    {
        fprintf(stderr, "Error: Read error in file , record \n");
        fclose(in_file);
        Quit(virus_list);
    }

    fclose(in_file);
    return virus_list;
}

link *printSig(link *virus_list)
{
    list_print(virus_list, stdout);
    return virus_list;
}

link *detectViruses(link *virus_list)
{
    char buffer[BUFFERSIZE];
    size_t bytes_read;
    if (argc == 1)
        Quit(virus_list);

    FILE *in_file = fopen(argv[1], "r");
    if (in_file == NULL)
    {
        fprintf(stderr, "Error: Couldn't open file\n");
        exit(1);
    }
    fseek(in_file, 0L, SEEK_END);
    bytes_read = ftell(in_file);
    fseek(in_file, 0L, SEEK_SET);
    fread(buffer, bytes_read, 1, in_file);

    detect_virus(buffer, bytes_read > BUFFERSIZE ? BUFFERSIZE : bytes_read, virus_list);

    fclose(in_file);
    return virus_list;
}

link *fixFile(link *virus_list)
{
    char buffer[BUFFERSIZE];
    size_t bytes_read;
    size_t size;
    if (argc == 1)
        Quit(virus_list);

    FILE *in_file = fopen(argv[1], "r");
    if (in_file == NULL)
    {
        fprintf(stdout, "Error: Couldn't open file\n");
        exit(1);
    }
    fseek(in_file, 0L, SEEK_END);
    bytes_read = ftell(in_file);
    fseek(in_file, 0L, SEEK_SET);
    fread(buffer, bytes_read, 1, in_file);
    size = bytes_read > BUFFERSIZE ? BUFFERSIZE : bytes_read;
    for (int byte = 0; byte < size; byte++)
    {
        link *curr = virus_list;
        while (curr)
        {
            size_t sigSize = curr->vir->SigSize;
            // check bounds
            if (byte + sigSize < size)
            {
                if (memcmp(buffer + byte, curr->vir->sig, sigSize) == 0)
                {
                    neutralize_virus(argv[1], byte);
                }
            }
            curr = curr->nextVirus;
        }
    }
    fclose(in_file);
    return virus_list;
}

link *Quit(link *virus_list)
{
    if (virus_list)
    {
        list_free(virus_list);
    }
    exit(1);
    return virus_list;
}