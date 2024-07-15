# Lab 5 - Static ELF Loader

## Introduction

This lab involves writing a static loader to load and execute 32-bit ELF formatted executables that do not use dynamic libraries. The loader will utilize the `system_call` interface. The code should be compiled with the `-m32` flag.


## Used :

### Elf32_Ehdr: 
    Represents the ELF file header with metadata about the ELF file.
### Elf32_Phdr: 
    Represents individual program headers that describe segments to be loaded into memory.


## Work flow : 

```
1- Checks if the program was given a filename as an argument.
2- Calls map_executable to map the executable file into memory.
3- Calls foreach_phdr with different functions (print_program_header, printProgramHeaderInfo, load_phdr) to handle each program header.
4- Calls startup with the program's entry point.
```

Essensial function for opening and maping the file to the memory: 

`void *map_executable(char *nameOfStr);`

### Purpose: 
    Maps the executable file into memory.
### Steps: 

    1- Opens the file for reading.
    2- Determines the file size.
    3- Maps the file into memory using mmap.
    3- Returns the mapped memory address.
    


#              Task 0: Iterator Over Program Headers


## Description
Write a program that receives a single command-line argument, which is the filename of a 32-bit ELF executable. Implement a function `foreach_phdr` to iterate over the program headers in the file and apply a given function to each header.


## Function Signature
`int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg);`

### Steps:
```
1- Gets the ELF header from the start of the mapped memory.
2- Gets the program header table from the ELF header.
3- Iterates over each program header and calls the provided function.
```

### Verification (output) for load me:
```
1- Program header 3 at address 0xf7f7f97e
2- Program header 3 at address 0xf7f7f99e
3- Program header 3 at address 0xf7f7f9be
4- Program header 3 at address 0xf7f7f9de
```

`void print_program_header(Elf32_Phdr *phdr, int index)`

### Purpose: Prints the program header address.
#### Steps:
    Prints the index and address of the given program header.



#                    Task 1: Readelf -l Functionality


## Task 1a: Display Program Headers

### Description:
Using the iterator from Task 0, the function displays all information in the Elf32_Phdr structure for each program header, similar to readelf -l.
Example Output: 

### Expected output :
``` Type Offset VirtAddr PhysAddr FileSiz MemSiz Flg Align
PHDR 0x000034 0x04048034 0x04048034 0x00100 0x00100 R E 0x4
INTERP 0x000134 0x04048134 0x04048134 0x00013 0x00013 R 0x1
LOAD 0x000000 0x04048000 0x04048000 0x008a4 0x008a4 R E 0x1000
LOAD 0x0008a4 0x040498a4 0x040498a4 0x0011c 0x00120 RW 0x1000
DYNAMIC 0x0008b0 0x040498b0 0x040498b0 0x000c8 0x000c8 RW 0x4
NOTE 0x000148 0x04048148 0x04048148 0x00020 0x00020 R 0x4
```

`void printProgramHeaderInfo(Elf32_Phdr *phdr, int i)`

### Purpose: Prints detailed information about the program header.
### Steps:
``` 
1) Prints the type of the program header using getType.
2) Prints the flags of the program header using getFlag.
3) Prints the offset, virtual address, physical address, file size, memory size, and alignment of the program header.
4) Prints the flags in a readable format (R, W, E).
```

## Task 1b: Prepare Data for Memory Mapping

For each program header, print the appropriate protection and mapping flags for mmap. Ensure that the protection flags are correctly translated from the program header flags.

`char *getType(int type)`

### Purpose: Returns a string representation of the program header type.
### Steps:
        1) Returns the corresponding string for each program header type.

`char *getFlag(int flag)`

### Purpose: Returns a string representation of the program header flags.
### Steps:
        Returns the corresponding string for each set of flags.

`int cntFlag(int flg)`

### Purpose: Converts program header flags to mmap protection flags.
### Steps:
        Returns the corresponding protection flags for each set of program header flags.


#                    Task 2: Implement the Loader


## Task 2a: Linking and Mapping

### Description:
Use the provided linking script and start.o to compile your loader. Ensure that the program is linked without standard libraries but with the system_call interface.


## Task 2b: Mapping Segments to Memory

### Description:
Implement the load_phdr function to map segments with the PT_LOAD flag to memory using mmap.

`void load_phdr(Elf32_Phdr *phdr, int fd);`

### Purpose:
    load_phdr maps program segments marked with PT_LOAD type from an ELF file into memory using mmap.

### Steps:

    1. Check Segment Type:    Checks if the program header (phdr) type (p_type) is PT_LOAD, indicating it should be loaded into memory.

    2. Calculate Addresses:
        2.1. Calculates the virtual address (vaddr) and file offset (offset) aligned to page boundaries.
        2.2. Computes any necessary padding based on the segment's virtual address.

    3. Set Memory Protection:
        Converts the segment's flags (p_flags) into memory protection flags (flags) for mmap.

    4. Map Segment:
        Uses mmap to map the segment into memory at the calculated virtual address with the specified size and flags.

## Task 2c: Passing Control to Loaded Program

Use the provided startup function to pass control to the loaded program. Test with programs that do not require command-line arguments.
Function Signature

`int startup(int argc, char **argv, void (*start)());`
#### note : 
    startup is a function used from lab3.

## Task 2d: Command-Line Arguments

Ensure your loader can pass command-line arguments to the loaded program.
Example Command

my_loader my_test_program arg1 arg2 ...


## Example of run time :


## Fisrt run time example without args:

```
> make clean all
> ./loader loadme
```

### Should print :

```
Program header 3 at address 0xf7f5c97e
Program header 3 at address 0xf7f5c99e
Program header 3 at address 0xf7f5c9be
Program header 3 at address 0xf7f5c9de
LOAD    Unknown 0x000040        0x08048040      0x08048040      0x001475   0x001475        RWE     0x1000
LOAD    Unknown 0x0014c0        0x0804a4c0      0x0804a4c0      0x000441   0x000441        RWE     0x1000
LOAD    Unknown 0x001920        0x0804b920      0x0804b920      0x000120   0x000120        RWE     0x1000
LOAD    Unknown 0x001a40        0x0804ca40      0x0804ca40      0x000000   0x000000        RWE     0x1000
(:^..^:) starting up the code
SHALOM RAV SHOVECH TZIPORA NECHMEDET

```

_____________________
_____________________

## Second run time example with args:

```
> make clean all
> ./loader encoder
```

### Should print :

```
Program header 3 at address 0xf7ed8034
Program header 3 at address 0xf7ed8054
Program header 3 at address 0xf7ed8074
Program header 3 at address 0xf7ed8094
LOAD    Read-only       0x000000        0x08048000      0x08048000      0x0000b4        0x0000b4        R--0x1000
LOAD    Read-execute    0x001000        0x08049000      0x08049000      0x000195        0x000195        R-E0x1000
LOAD    Read-only       0x002000        0x0804a000      0x0804a000      0x000032        0x000032        R--0x1000
LOAD    Read-write      0x002034        0x0804b034      0x0804b034      0x000010        0x000014        RW-0x1000
(:^..^:) starting up the code
1234
1234
abcde
abcde
[cntl + D]

```

_________
_________

## Third run time example with args:

```
> make clean all
> ./loader encoder +e12345
```

### Should print :

```
Program header 3 at address 0xf7ed3034
Program header 3 at address 0xf7ed3054
Program header 3 at address 0xf7ed3074
Program header 3 at address 0xf7ed3094
LOAD    Read-only       0x000000        0x08048000      0x08048000      0x0000b4        0x0000b4        R--     0x1000
LOAD    Read-execute    0x001000        0x08049000      0x08049000      0x000195        0x000195        R-E     0x1000
LOAD    Read-only       0x002000        0x0804a000      0x0804a000      0x000032        0x000032        R--     0x1000
LOAD    Read-write      0x002034        0x0804b034      0x0804b034      0x000010        0x000014        RW-     0x1000
(:^..^:) starting up the code
abcdez
bdfhj{
```

### Questions:

1. You should be able to explain to the lab instructor why the linking script is needed and how you verified that it worked. 
what is the answer?

    Answer: the linking script is needed to specify custom memory layout and ensure proper execution flow for the loader without standard library dependencies. Verification involves compiling the loader with the script and confirming correct program execution and memory mapping alignment.
