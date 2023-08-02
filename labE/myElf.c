#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stddef.h>
#include <elf.h>

#define SIZED_ARRAY 7

typedef struct fun_desc
{
    char *name;
    void (*fun)();
} fun_desc;

/* Global Varibles */
bool deBugMode = false;
bool displayFlag = false;
int counter = 0;
long file_size1;
long file_size2;
FILE *file1;
FILE *file2;
Elf32_Ehdr *ehdr1;
Elf32_Ehdr *ehdr2;

// MENU FUNCTIONS
void toggleDeBugMode();
void examineElfFile();
void printSectionNames();
void printSymbols();
void CheckForMerge();
void mergeElfFiles();
void Quit();

// AST. MENU FUNCTIONS
void printSections(Elf32_Ehdr *ehdr);
void printSingleSection(int index, char *section_name, Elf32_Shdr *sec);
char *getElfDataEncoding(Elf32_Ehdr *elf_header);
void *mapReturn();
void print_elfHeader(Elf32_Ehdr *ehdr);
void printSymbolsInFile(Elf32_Ehdr *ehdr);
Elf32_Shdr *getSymbolTable(Elf32_Ehdr *ehdr);
bool rangeCheck(int input);
void printthemenu(struct fun_desc *menu);
bool checkInSymbolTable2(char *symbol_name1, Elf32_Ehdr *ehdr);

int main(int argc, char **argv)
{
    char c[100];
    fun_desc menu[] = {
        {"Toggle Debug mode", &toggleDeBugMode},
        {"Examine ELF File", &examineElfFile},
        {"Print Section Names", &printSectionNames},
        {"Print Symbols", &printSymbols},
        {"Check Files for Merge", &CheckForMerge},
        {"Merge ElF Files", &mergeElfFiles},
        {"Quit", &Quit},
        {NULL, NULL}};

    int intInput = 0;
    printthemenu(menu);
    while (true)
    {
        if (fgets(c, sizeof(c), stdin) != NULL)
        {
            intInput = atoi(c);
            if (rangeCheck(intInput))
            {
                menu[intInput].fun();
            }
            else
            {
                printf("Input is not in the bounds\n");
                exit(1);
            }
        }
        else
        {
            exit(1);
        }
        printf("\n\n ");
        printthemenu(menu);
    }
}

// function to print the menu components.
void printthemenu(struct fun_desc *menu)
{
    printf("\n******* NEW ITERATION ********\n\n\n");

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

void toggleDeBugMode()
{
    deBugMode = !deBugMode;
    if (deBugMode)
        fprintf(stderr, "\nDebug: flag now on.\n");
    else
        fprintf(stderr, "\nDebug: flag now off.\n");
}

void Quit()
{
    exit(0);
}

void *mapReturn(FILE **file, long *fileSize)
{
    printf("\n> insert a new file name: ");
    char newfilename[100];
    fgets(newfilename, sizeof(newfilename), stdin);

    if (newfilename[strlen(newfilename) - 1] == '\n')
        newfilename[strlen(newfilename) - 1] = '\0';

    *file = fopen(newfilename, "r");

    if (!(*file))
    {
        fprintf(stderr, "An error occurred while trying to open the file.\n");
        return NULL;
    }

    // fileSize represent either file1 size or file2 size
    // depends on the given file.
    fseek(*file, 0, SEEK_END);
    *fileSize = ftell(*file);
    fseek(*file, 0, SEEK_SET);

    void *map_start = mmap(NULL, *fileSize, PROT_READ, MAP_PRIVATE, fileno(*file), 0);

    if (map_start == MAP_FAILED)
    {
        printf("Failed to map the file.\n");
        fclose(*file);
        return NULL;
    }

    return map_start;
}

void examineElfFile()
{
    counter++;
    void *map_start;

    if (counter == 1)
        map_start = mapReturn(&file1, &file_size1);
    else if (counter == 2)
        map_start = mapReturn(&file2, &file_size2);
    else
    {
        printf("error: Calling the function for 3rd time");
        return;
    }
    if (map_start == NULL)
    {
        printf("Failed to map the file.\n");
        return;
    }

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)map_start;
    if (counter == 1)
        ehdr1 = (Elf32_Ehdr *)map_start;
    else
        ehdr2 = (Elf32_Ehdr *)map_start;

    // check if ELF file
    if (strncmp((char *)ehdr->e_ident, (char *)ELFMAG, 4) == 0)
    {
        print_elfHeader(ehdr);
    }
    else
    {

        printf("Closing the file...\n");
        if (counter == 1)
        {
            munmap(map_start, file_size1);
            fclose(file1);
            file1 = NULL;
        }
        else if (counter == 2)
        {
            munmap(map_start, file_size2);
            fclose(file2);
            file2 = NULL;
        }
    }
}

void printSectionNames()
{
    if (ehdr1)
    {
        printSections(ehdr1);
    }
    if (ehdr2)
        printSections(ehdr2);
    if (!ehdr1 && !ehdr2)
    {
        printf("There is no files, please enter file and call me again!\n");
        return;
    }
}

void printSections(Elf32_Ehdr *ehdr)
{
    // section table add : file + header table offset
    Elf32_Shdr *sectionsTable = (Elf32_Shdr *)((void *)ehdr + ehdr->e_shoff);
    // string_table_entry  :  sections_table + (index of string table * size of the sections).
    Elf32_Shdr *stringTableEntry = &sectionsTable[ehdr->e_shstrndx];
    // array of strings of the sections' names.
    char *sectionsname = (char *)((void *)ehdr + stringTableEntry->sh_offset);
    printf("\nSection Headers:\n");

    for (int i = 0; i < ehdr->e_shnum; i++)
    {
        Elf32_Shdr *section_entry = &sectionsTable[i];
        char *section_name = &sectionsname[section_entry->sh_name];
        printSingleSection(i, section_name, section_entry);
    }
}

void printSingleSection(int index, char *section_name, Elf32_Shdr *sec)
{
    printf("[%d] %s 0x%x 0x%x %d %u\n",
           index,
           section_name,
           sec->sh_addr,
           sec->sh_offset,
           sec->sh_size,
           sec->sh_type);
    // printf("%#06x \n", gettype(sec->sh_type));
}

void printSymbols()
{
    printf("Num:    Value  Size Type    Bind   Vis      Ndx Name");
    if (ehdr1 == NULL && ehdr2 == NULL)
    {
        printf("No ELF files currently mapped.\n");
        return;
    }

    printf("\nSymbol table:\n");

    if (ehdr1 != NULL)
    {
        printSymbolsInFile(ehdr1);
    }

    printf("\n");

    if (ehdr2 != NULL)
    {
        printSymbolsInFile(ehdr2);
    }
}

void printSymbolsInFile(Elf32_Ehdr *ehdr)
{
    Elf32_Shdr *sec_table = (Elf32_Shdr *)((void *)ehdr + ehdr->e_shoff);

    // Find the symbol table section and the associated string table
    Elf32_Shdr *SymbolTable = NULL;
    Elf32_Shdr *stringTable = NULL;
    for (int i = 0; i < ehdr->e_shnum; i++)
    {
        Elf32_Shdr *sec = &sec_table[i];
        if (sec->sh_type == SHT_SYMTAB) // Find Symbol table.
        {
            SymbolTable = sec;
            stringTable = &sec_table[sec->sh_link];
            break;
        }
    }

    // Check if symbol table is found
    if (SymbolTable == NULL)
    {
        printf("No symbol table found.\n");
        return;
    }

    // Retrieve symbol and string table data
    Elf32_Sym *sym_table = (Elf32_Sym *)((void *)ehdr + SymbolTable->sh_offset);
    char *strtab = (char *)((void *)ehdr + stringTable->sh_offset);

    // Iterate over symbols and print their information
    int num_symbols = SymbolTable->sh_size / SymbolTable->sh_entsize;
    for (int i = 0; i < num_symbols; i++)
    {
        Elf32_Sym *symbol = &sym_table[i];

        char *symbol_name = &strtab[symbol->st_name];

        char *section_name;

        if (symbol->st_shndx == 0xFFF1)
            section_name = "ABS";
        else if (symbol->st_shndx == 0x0)
            section_name = "UND";
        else
        {
            Elf32_Shdr *defining_sec = &sec_table[symbol->st_shndx];
            section_name = &strtab[defining_sec->sh_name];
        }
        printf("[%d] 0x%x \t %d \t %s \t %s\n", i, symbol->st_value, symbol->st_shndx, section_name, symbol_name);
    }
}

void CheckForMerge()
{

    if (ehdr1 && ehdr2)
    {
        Elf32_Shdr *SYMTAB1 = getSymbolTable(ehdr1);
        Elf32_Shdr *sec_table1 = (Elf32_Shdr *)((void *)ehdr1 + ehdr1->e_shoff);
        Elf32_Shdr *stringTable1 = &sec_table1[SYMTAB1->sh_link];
        int num_symbols1 = SYMTAB1->sh_size / SYMTAB1->sh_entsize;
        char *strtab1 = (char *)((void *)ehdr1 + stringTable1->sh_offset);

        Elf32_Sym *symtbl_enrty1 = (Elf32_Sym *)((void *)ehdr1 + SYMTAB1->sh_offset);

        // loop over the symbol table 1
        for (int i = 0; i < num_symbols1; i++)
        {
            Elf32_Sym *symbol1 = &symtbl_enrty1[i];
            char *symbol_name1 = &strtab1[symbol1->st_name];

            printf("section name : %s \n", symbol_name1);
            bool found = checkInSymbolTable2(symbol_name1, ehdr2);

            // check if symbol in symbol table 1 is undefined
            if (symbol1->st_shndx == SHN_UNDEF)
            {
                if (!found)
                    printf("Symbol sym undefined\n");
            }
            else
            {
                if (found && symbol1->st_name != 0)
                    printf("Symbol sym multiply defined\n");
            }
        }
    }
    else
    {
        printf("There is no files, please enter file and call me again!\n");
        return;
    }
}

bool checkInSymbolTable2(char *symbol_name1, Elf32_Ehdr *ehdr2)
{
    Elf32_Shdr *SYMTAB2 = getSymbolTable(ehdr2);
    Elf32_Sym *symtbl_enrty2 = (Elf32_Sym *)((void *)ehdr2 + SYMTAB2->sh_offset);
    Elf32_Shdr *sec_table2 = (Elf32_Shdr *)((void *)ehdr2 + ehdr2->e_shoff);
    Elf32_Shdr *stringTable2 = &sec_table2[SYMTAB2->sh_link];
    char *strtab2 = (char *)((void *)ehdr2 + stringTable2->sh_offset);

    int num_symbols2 = SYMTAB2->sh_size / SYMTAB2->sh_entsize;

    for (int i = 1; i < num_symbols2; i++)
    {

        Elf32_Sym *symbol2 = &symtbl_enrty2[i];
        char *symbol_name2 = &strtab2[symbol2->st_name];

        if (strncmp(symbol_name1, symbol_name2, strlen(symbol_name1)) == 0)
        {
            if (symbol2->st_shndx == SHN_UNDEF)
            {
                return false;
            }
            else
            {
                return true;
            }
        }
    }
    return false;
}

Elf32_Shdr *getSymbolTable(Elf32_Ehdr *ehdr)
{
    Elf32_Shdr *sec_table = (Elf32_Shdr *)((void *)ehdr + ehdr->e_shoff);

    // Find the symbol table section and the associated string table
    Elf32_Shdr *SymbolTable = NULL;
    for (int i = 1; i < ehdr->e_shnum; i++)
    {
        Elf32_Shdr *sec = &sec_table[i];
        if (sec->sh_type == SHT_SYMTAB) // Find Symbol table.
        {
            SymbolTable = sec;
            return SymbolTable;
        }
    }
    // Check if symbol table is found
    if (SymbolTable == NULL)
    {
        printf("No symbol table found.\n");
        return NULL;
    }
    return NULL;
}

void mergeElfFiles()
{
    if (!ehdr1 || !ehdr2)
    {
        printf("Didn't receive the two files, please enter the files.\n");
        return;
    }

    char *mergedFile_name = "out.ro";
    int fd = open(mergedFile_name, O_RDWR | O_CREAT, 0777);

    if (fd == -1)
    {
        perror("open failed\n");
        exit(1);
    }

    FILE *mergedFile = fdopen(fd, "w+b");

    if (!mergedFile)
    {
        perror("Error handling a file.");
        close(fd);
        return;
    }

    Elf32_Shdr *secHds1 = NULL;
    Elf32_Shdr *secHds2 = NULL;

    secHds1 = (Elf32_Shdr *)malloc(sizeof(Elf32_Shdr) * ehdr1->e_shnum);
    fseek(file1, ehdr1->e_shoff, SEEK_SET);
    fread(secHds1, sizeof(Elf32_Shdr), ehdr1->e_shnum, file1);

    fwrite(ehdr1, sizeof(Elf32_Ehdr), 1, mergedFile);
    fwrite(secHds1, sizeof(Elf32_Shdr), ehdr1->e_shnum, mergedFile);

    secHds2 = (Elf32_Shdr *)malloc(sizeof(Elf32_Shdr) * ehdr2->e_shnum);
    fseek(file2, ehdr2->e_shoff, SEEK_SET);
    fread(secHds2, sizeof(Elf32_Shdr), ehdr2->e_shnum, file2);

    Elf32_Shdr *stringTableEntry1 = &secHds1[ehdr1->e_shstrndx];
    char *secnames1 = (char *)((void *)ehdr1 + stringTableEntry1->sh_offset);

    Elf32_Shdr *stringTableEntry2 = &secHds2[ehdr2->e_shstrndx];
    char *secnames2 = (char *)((void *)ehdr2 + stringTableEntry2->sh_offset);

    Elf32_Off offset = 0;

    for (int i = 0; i < ehdr1->e_shnum; i++)
    {
        Elf32_Shdr *sectionHeader1 = &secHds1[i];
        char *sectionName1 = &secnames1[sectionHeader1->sh_name];


        if (strcmp(sectionName1, ".text") == 0 || strcmp(sectionName1, ".data") == 0 || strcmp(sectionName1, ".rodata") == 0)
        {
            Elf32_Shdr *sectionHeader2 = NULL;

            for (int j = 0; j < ehdr2->e_shnum; j++)
            {
                Elf32_Shdr *sectionHeader2_ast = &secHds2[j];
                char *sectionName2 = &secnames2[sectionHeader2_ast->sh_name];

                if (strcmp(sectionName1, sectionName2) == 0)
                {
                    sectionHeader2 = &secHds2[j];

                    break;
                }
            }

            if (sectionHeader2)
            {

                sectionHeader1->sh_size += sectionHeader2->sh_size;
                fseek(file1, sectionHeader1->sh_offset, SEEK_SET);
                char *sectionData1 = (char *)malloc(sectionHeader1->sh_size - sectionHeader2->sh_size);
                fread(sectionData1, sizeof(char), sectionHeader1->sh_size - sectionHeader2->sh_size, file1);
                fwrite(sectionData1, sizeof(char), sectionHeader1->sh_size - sectionHeader2->sh_size, mergedFile);
                free(sectionData1);

                // sectionHeader1->sh_offset += offset;
                //  offset += sectionHeader2->sh_size;

                fseek(file2, sectionHeader2->sh_offset, SEEK_SET);
                char *sectionData2 = (char *)malloc(sectionHeader2->sh_size);
                fread(sectionData2, sizeof(char), sectionHeader2->sh_size, file2);
                fwrite(sectionData2, sizeof(char), sectionHeader2->sh_size, mergedFile);
                fseek(mergedFile, 0, SEEK_END);
                offset = ftell(mergedFile);
                secHds1->sh_offset = offset;
                free(sectionData2);
            }
            else
            {
                fseek(file1, sectionHeader1->sh_offset, SEEK_SET);
                char *sectionData1 = (char *)malloc(sectionHeader1->sh_size);
                fread(sectionData1, sizeof(char), sectionHeader1->sh_size, file1);
                fwrite(sectionData1, sizeof(char), sectionHeader1->sh_size, mergedFile);
                fseek(mergedFile, 0, SEEK_END);
                offset = ftell(mergedFile);
                secHds1->sh_offset = offset;
                free(sectionData1);
                // sectionHeader1->sh_offset += offset;
            }
        }
    }

    // Update the section header offset in the ELF header
    fseek(mergedFile, offsetof(Elf32_Ehdr, e_shoff), SEEK_SET);
    Elf32_Off offset2 = ehdr1->e_shoff;
    fwrite(&offset2, sizeof(Elf32_Off), 1, mergedFile);

    fseek(mergedFile, offsetof(Elf32_Ehdr, e_shoff), SEEK_SET);
    fwrite(&offset2, sizeof(Elf32_Off), 1, mergedFile);

    // Write the section headers
    fseek(mergedFile, ehdr1->e_shoff, SEEK_SET);
    fwrite(secHds1, sizeof(Elf32_Shdr), ehdr1->e_shnum, mergedFile);

    fclose(mergedFile);
    free(secHds1);
    free(secHds2);

    printf("Merge completed successfully.\n");


}







char *getElfDataEncoding(Elf32_Ehdr *elf_header)
{
    char *data_encoding;
    switch (elf_header->e_ident[EI_DATA])
    {
    case ELFDATA2LSB:
        data_encoding = "2's complement, little-endian";
        break;
    case ELFDATA2MSB:
        data_encoding = "2's complement, big-endian";
        break;
    case ELFDATANONE:
        data_encoding = "Unknown data encoding";
        break;
    default:
        data_encoding = "Invalid data encoding";
        break;
    }
    return data_encoding;
}

void print_elfHeader(Elf32_Ehdr *ehdr)
{
    printf("\nELF Header:\n");

    printf("  Magic:   %x %x %x\n", ehdr->e_ident[EI_MAG1], ehdr->e_ident[EI_MAG2], ehdr->e_ident[EI_MAG3]);

    printf("  Data:\t\t\t\t\t\t%s\n", getElfDataEncoding(ehdr));

    printf("  Entry point address:\t\t\t\t0x%x\n", ehdr->e_entry);

    printf("  Start of section headers:\t\t\t%d (bytes into file)\n", ehdr->e_shoff);

    printf("  Number of section headers:\t\t\t%d\n", ehdr->e_shnum);

    printf("  Size of section headers:\t\t\t%d (bytes)\n", ehdr->e_shentsize);

    printf("  Start of program headers:\t\t\t%d (bytes into file)\n", ehdr->e_phoff);

    printf("  Number of program headers:\t\t\t%d\n", ehdr->e_phnum);

    printf("  Size of program headers:\t\t\t%d (bytes)\n", ehdr->e_phentsize);
}
