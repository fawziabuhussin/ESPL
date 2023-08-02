#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <elf.h>
#include <stdlib.h>

FILE *file;
extern int startup(int argc, char **argv, void (*start)());




int cntFlag(int flg);
char *getFlag(int flag);
char *getType(int type);
int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg);
void printingPhdr(Elf32_Phdr *phdr, int i);
void printing(Elf32_Phdr *phdr, int i);
void *mapReturn(char *nameOfStr);
void load_phdr(Elf32_Phdr *phdr, int fd);
int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Error: there is no required data.\n");
        return 1;
    }

    void *map_start = mapReturn(argv[1]);
    if (map_start == NULL)
    {
        printf("Failed to map the file.\n");
        return 1;
    }
    // printf("Type\tFlag\tOffset\tVirtAddr\tPhysAddr\tFileSiz\tMemSiz\tFlg\tAlign\n");
    int fd = fileno(file);
    printf("task0: \n");

    foreach_phdr(map_start, printing, fd);
    printf("task 1 : \n");
    foreach_phdr(map_start, printingPhdr, fd);
    printf("task 2 : \n");
    foreach_phdr(map_start, load_phdr, fd);
    startup(argc-1, argv+1, (void *)(((Elf32_Ehdr *) map_start)->e_entry));
    return 0;
}

void *mapReturn(char *nameOfStr)
{
    file = fopen(nameOfStr, "r");
    if (!file)
    {
        fprintf(stderr, "An error occurred while trying to open the file.\n");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long fs = ftell(file);
    fseek(file, 0, SEEK_SET);

    void *map_start = mmap(NULL, fs, PROT_READ, MAP_PRIVATE, fileno(file), 0);
    if (map_start == MAP_FAILED)
    {
        printf("Failed to map the file.\n");
        fclose(file);
        return NULL;
    }

    return map_start;
}

void printing(Elf32_Phdr *phdr, int i)
{
    printf("Program header number %d at address %p\n", i, phdr);
}

// Type Offset VirtAddr PhysAddr FileSiz MemSiz Flg Align
// PHDR 0x000034 0x04048034 0x04048034 0x00100 0x00100 R E 0x4
// INTERP 0x000134 0x04048134 0x04048134 0x00013 0x00013 R 0x1
// LOAD 0x000000 0x04048000 0x04048000 0x008a4 0x008a4 R E 0x1000
// LOAD 0x0008a4 0x040498a4 0x040498a4 0x0011c 0x00120 RW 0x1000
// DYNAMIC 0x0008b0 0x040498b0 0x040498b0 0x000c8 0x000c8 RW 0x4
// NOTE 0x000148 0x04048148 0x04048148 0x00020 0x00020 R 0x4

void printingPhdr(Elf32_Phdr *phdr, int i)
{
    printf("%s\t", getType(phdr->p_type));
    printf("%s\t", getFlag(phdr->p_flags));
    printf("0x%06x\t", phdr->p_offset);
    printf("0x%08x\t", phdr->p_vaddr);
    printf("0x%08x\t", phdr->p_paddr);
    printf("0x%06x\t", phdr->p_filesz);
    printf("0x%06x\t", phdr->p_memsz);
    printf("%c%c%c\t", phdr->p_flags & PF_R ? 'R' : '-', phdr->p_flags & PF_W ? 'W' : '-', phdr->p_flags & PF_X ? 'E' : '-');
    printf("0x%x\n", phdr->p_align);
}

int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg)
{
    int i;
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)map_start;
    Elf32_Phdr *phdr = (Elf32_Phdr *)(map_start + ehdr->e_phoff);

    int phnum = ehdr->e_phnum;
    for (i = 0; i < phnum; i++)
    {
        func(&phdr[i], arg);
    }
    return 0;
}

char *getType(int type)
{
    switch (type)
    {
    case PT_NULL:
        return "NULL";
    case PT_LOAD:
        return "LOAD";
    case PT_DYNAMIC:
        return "DYNAMIC";
    case PT_INTERP:
        return "INTERP";
    case PT_NOTE:
        return "NOTE";
    case PT_SHLIB:
        return "SHLIB";
    case PT_PHDR:
        return "PHDR";
    case PT_TLS:
        return "TLS";
    case PT_NUM:
        return "NUM";
    case PT_LOOS:
        return "LOOS";
    case PT_GNU_EH_FRAME:
        return "GNU_EH_FRAME";
    case PT_GNU_STACK:
        return "GNU_STACK";
    case PT_GNU_RELRO:
        return "GNU_RELRO";
    case PT_SUNWBSS:
        return "SUNWBSS";
    case PT_SUNWSTACK:
        return "SUNWSTACK";
    case PT_HIOS:
        return "HIOS";
    default:
        return "UNKNOWN";
    }
}

// 1.B
char *getFlag(int flag)
{
    switch (flag)
    {
    case PF_R:
        return "Read-only";
    case PF_R | PF_X:
        return "Read-execute";
    case PF_R | PF_W:
        return "Read-write";
    default:
        return "Unknown";
    }
}

void load_phdr(Elf32_Phdr *phdr, int fd)
{
    if (phdr->p_type == PT_LOAD)
    {
        /*
        vaddr = phdr.p_vaddr&0xfffff000;
        offset = phdr.p_offset&0xfffff000;
        padding = phdr.p_vaddr & 0xfff;
         */
        void *vaddr = (void*) (phdr->p_vaddr & 0xfffff000);
        int offset = phdr->p_offset & 0xfffff000;
        int padding = phdr->p_vaddr & 0xfff;
        int flags = cntFlag(phdr->p_flags);
        void *map_start = mmap(vaddr, phdr->p_memsz + padding, flags, MAP_FIXED | MAP_PRIVATE, fd, offset);
        if (map_start == MAP_FAILED)
        {
            printf("Failed to map the file.\n");
            fclose(file);
            exit(1);
        }
    }
    else
        return; // type is not load.
}

int cntFlag(int flg)
{
    switch (flg){
        case 0x000: return 0;
        case 0x001: return PROT_EXEC;
        case 0x002: return PROT_WRITE;
        case 0x003: return PROT_EXEC | PROT_EXEC;
        case 0x004: return PROT_READ;
        case 0x005: return PROT_READ | PROT_EXEC;
        case 0x006: return PROT_READ | PROT_WRITE;
        case 0x007: return PROT_READ | PROT_WRITE | PROT_EXEC;
        default:return -1;
    }
}


 