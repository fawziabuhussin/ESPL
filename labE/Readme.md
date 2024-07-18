# <span style="color: red;">**Lab E: Linking ELF Object Files**</span>

## **Introduction**
The goal of this lab is to implement a limited pass I (merging) of a linkage editor. You begin by allowing access to ELF object files, examining, and printing out their structures (section headers and symbol table). After this part is correctly implemented and debugged, you will be implementing the part that does the merging. This assignment will be limited to merging 2 ELF files, and with additional simplifying assumptions and restrictions specified later on.

### **A little review about the ELF files**

#### **ELF Header Fields**
- **Magic Number**: The first bytes of the ELF header, used to identify the file as an ELF file.
- **Data Encoding**: Specifies the encoding of the data in the file (e.g., little-endian).
- **Entry Point Address**: The memory address at which the process starts executing.
- **Section Headers**: Information about the sections in the file, including their offsets and sizes.
- **Program Headers**: Information about the segments that need to be loaded into memory, including their offsets and sizes.

#### **ELF Header Fields**
- **e_ident**: An array of bytes that specifies how to interpret the file, including magic number and file class (32-bit or 64-bit).
- **e_type**: Identifies the object file type (e.g., relocatable, executable, shared object).
- **e_machine**: Specifies the required architecture (e.g., x86).
- **e_version**: Identifies the object file version.
- **e_entry**: Virtual address of the entry point in the process image.
- **e_phoff**: Offset to the program header table.
- **e_shoff**: Offset to the section header table.
- **e_flags**: Processor-specific flags.
- **e_ehsize**: ELF header's size in bytes.
- **e_phentsize**: Size of one entry in the program header table.
- **e_phnum**: Number of entries in the program header table.
- **e_shentsize**: Size of one entry in the section header table.
- **e_shnum**: Number of entries in the section header table.
- **e_shstrndx**: Section header table index of the section name string table.

## **What you should install from this lab?**
1. a.out
2. object file examples.

## **Output and run examples at the end of the readme**

---

# <span style="color: red;"> **Part 0 - mmap system call:** </span>
### **Functions used:**
1. `void display_elf_header(Elf32_Ehdr *ehdr)`
2. `char *get_elf_data_encoded(Elf32_Ehdr *elf_header)`
3. `void *map_executable();`


### **Steps:**
1. You should copy the main from Lab4, including the menu parts.
2. Change the menu from lab4 to the suggested menu in this Lab.


### **Expected output for F12a:**

Magic:                              ELF
Data:                               2's complement, little endian
Entry point address:                0x0
Start of program headers:           0 (bytes into file)
Start of section headers:           64 (bytes into file)
Size of program headers:            0 (bytes)
Number of program headers:          0
Size of section headers:            40 (bytes)
Number of section headers:          8

### **readelf -h F12a:**
    ELF Header:
        Magic:   7f 45 4c 46 01 01 01 00 00 00 00 00 00 00 00 00 
        Class:                             ELF32
        Data:                              2's complement, little endian
        Version:                           1 (current)
        OS/ABI:                            UNIX - System V
        ABI Version:                       0
        Type:                              EXEC (Executable file)
        Machine:                           Intel 80386
        Version:                           0x1
        Entry point address:               0x8048080
        Start of program headers:          52 (bytes into file)
        Start of section headers:          416 (bytes into file)
        Flags:                             0x0
        Size of this header:               52 (bytes)
        Size of program headers:           32 (bytes)
        Number of program headers:         2
        Size of section headers:           40 (bytes)
        Number of section headers:         7
        Section header string table index: 4


---

# <span style="color: red;"> **Part 1 - Sections:** </span>

### **Functions used:**
1. `void printSectionNames()`
2. `void displaySections(Elf32_Ehdr *elfHeader)`
3. `void displaySingleSection(int index, char *sectionName, Elf32_Shdr *sectionHeader)`

### **Steps:**
1. Open the elf sections using `readelf -S F12a`.
2. Try making the same output using it.
3. If you manage you will get the same output as the next section.

### **Expected output for F12a:**


``` 
Section Headers:
[0]  0x0 0x0 0 0
[1] .text 0x8048080 0x80 166 1
[2] .rodata 0x8048128 0x128 34 1
[3] .data 0x804914c 0x14c 34 1
[4] .shstrtab 0x0 0x16e 47 3
[5] .symtab 0x0 0x2b8 352 2
[6] .strtab 0x0 0x418 122 3
```

### **readelf -S F12a:**

```
Section Headers:
  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
  [ 0]                   NULL            00000000 000000 000000 00      0   0  0
  [ 1] .text             PROGBITS        08048080 000080 0000a6 00  AX  0   0 16
  [ 2] .rodata           PROGBITS        08048128 000128 000022 00   A  0   0  4
  [ 3] .data             PROGBITS        0804914c 00014c 000022 00  WA  0   0  4
  [ 4] .shstrtab         STRTAB          00000000 00016e 00002f 00      0   0  1
  [ 5] .symtab           SYMTAB          00000000 0002b8 000160 10      6  11  4
  [ 6] .strtab           STRTAB          00000000 000418 00007a 00      0   0  1
```

### **Run example:**

```
> make clean all
> ./myElf 
> 1
> F12a
> 2

```


---

# <span style="color: red;"> **Part 2 - Symbols** </span>

### **Functions used:**
1. `void display_symbol_tables();`
2. `void display_symbols_in_file(Elf32_Ehdr *elfHeader);`

### **Steps:**
1. Open the elf sections using `readelf -S F12a`.
2. Try making the same output using it.
3. If you manage you will get the same output as the next section.

### **Expected output for F12a:**

``` 
Option: 3
Num:    Value  Size Type    Bind   Vis      Ndx Name
Symbol table:
[0] 0x0          0       UND     
[1] 0x8048080    1       a.s     
[2] 0x8048128    2       p1      
[3] 0x804914c    3       t       
[4] 0x0          65521   ABS     F1a.s
[5] 0x8048128    2       p1      OutStr
[6] 0x804914c    3       t       Dummy
[7] 0x80480c0    1       a.s     main
[8] 0x0          65521   ABS     F2a.s
[9] 0x0          65521   ABS     
[10] 0x0         65521   ABS     
[11] 0x8048105   1       a.s     rep1
[12] 0x8048080   1       a.s     _start
[13] 0x804916e   3       t       __bss_start
[14] 0x804810e   1       a.s     end_it
[15] 0x804810f   1       a.s     print_it
[16] 0x8049150   3       t       OtherStr
[17] 0x804916e   3       t       _edata
[18] 0x8049170   3       t       _end
[19] 0x804809f   1       a.s     system_call
[20] 0x804811e   1       a.s     my_exit
[21] 0x8048100   1       a.s     my_strlen
```


### **readelf -S F12a:**

```

Symbol table '.symtab' contains 22 entries:
   Num:    Value  Size Type    Bind   Vis      Ndx Name
     0: 00000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 08048080     0 SECTION LOCAL  DEFAULT    1 
     2: 08048128     0 SECTION LOCAL  DEFAULT    2 
     3: 0804914c     0 SECTION LOCAL  DEFAULT    3 
     4: 00000000     0 FILE    LOCAL  DEFAULT  ABS F1a.s
     5: 08048128     0 NOTYPE  LOCAL  DEFAULT    2 OutStr
     6: 0804914c     0 NOTYPE  LOCAL  DEFAULT    3 Dummy
     7: 080480c0     0 NOTYPE  LOCAL  DEFAULT    1 main
     8: 00000000     0 FILE    LOCAL  DEFAULT  ABS F2a.s
     9: 00000000     0 FILE    LOCAL  DEFAULT  ABS 
    10: 00000000     0 FILE    LOCAL  DEFAULT  ABS 
    11: 08048105     0 NOTYPE  GLOBAL DEFAULT    1 rep1
    12: 08048080     0 NOTYPE  GLOBAL DEFAULT    1 _start
    13: 0804916e     0 NOTYPE  GLOBAL DEFAULT    3 __bss_start
    14: 0804810e     0 NOTYPE  GLOBAL DEFAULT    1 end_it
    15: 0804810f     0 NOTYPE  GLOBAL DEFAULT    1 print_it
    16: 08049150     0 NOTYPE  GLOBAL DEFAULT    3 OtherStr
    17: 0804916e     0 NOTYPE  GLOBAL DEFAULT    3 _edata
    18: 08049170     0 NOTYPE  GLOBAL DEFAULT    3 _end
    19: 0804809f     0 NOTYPE  GLOBAL DEFAULT    1 system_call
    20: 0804811e     0 NOTYPE  GLOBAL DEFAULT    1 my_exit
    21: 08048100     0 NOTYPE  GLOBAL DEFAULT    1 my_strlen
```

### Run example :
```
> make clean all
> ./myElf 
> 1
> F12a
> 3

```

---
# <span style="color: red;"> **Part 3.1: Check Files for Merge** </span>


### **Functions used:**
1. `void check_for_merge();`
2. `Elf32_Shdr *exclude_symbol_table(Elf32_Ehdr *ehdr)`
3. `bool check_symbol_duplication(char *symbol_name1, Elf32_Ehdr *ehdr)`


## Steps:
## Check Files for Merge Implementation Steps

1. **Check ELF Files Loaded**:
   - Verify that exactly two ELF files are opened and mapped.
   - If not, print an error message and return.

2. **Verify Single Symbol Table**:
   - Ensure each ELF file contains exactly one symbol table.
   - If not, print "feature not supported" and return.

3. **Loop Through Symbols in SYMTAB1**:
   - For each ELF file, loop through all symbols in its symbol table (excluding the first dummy symbol).

4. **Handle Undefined Symbols**:
   - If a symbol in SYMTAB1 is undefined, search for it in SYMTAB2.
   - If not found or also undefined in SYMTAB2, print "Symbol sym undefined".

5. **Handle Defined Symbols**:
   - If a symbol in SYMTAB1 is defined, search for it in SYMTAB2.
   - If found and also defined in SYMTAB2, print "Symbol sym multiply defined".

6. **Continue Scanning After Errors**:
   - Continue scanning symbols even if errors are found.

7. **Run Tests**:
   - Use the provided object files (e.g., "F1a.o" and "F2a.o" for no errors, "F1b.o" and "F2b.o" for errors) to verify the implementation.

    

### Run example 1:
```
> ./myElf 
> 1
> F1a.o
> 1
> F2a.o
> 5
```

### **Expected output for merged F1a.o and F2a.o:**

> It should not result an error, it should pass.

```
Option: 4
section name :  
section name : F1a.s 
section name :  
section name :  
section name :  
section name : OutStr 
section name : Dummy 
section name : main 
section name : print_it 
section name : my_strlen 
section name : my_exit 
section name : OtherStr 
section name : rep1 
section name : end_it 
section name : _start 
section name : system_call 
```

### Run example 2:
```
> ./myElf 
> 1
> F1b.o
> 1
> F2b.o
> 5
```


### **Expected output for merged F1b.o and F2b.o:**

> It should result an error.

```
Option: 4
section name :  
section name : F1b.s 
section name :  
section name :  
section name :  
section name : OutStr 
section name : Dummy 
section name : main 
section name : my_strlen 
section name : my_exit 
Symbol sym undefined
section name : OtherStr 
section name : rep1 
section name : end_it 
section name : _start 
section name : system_call 
section name : print_it 
Symbol sym multiply defined
```




