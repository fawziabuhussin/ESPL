#format is target-name: target dependencies 

#{-tab-}actions 

 # All Targets 


.PHONY : clean

all: task0a task0b

task0a :  start.o util.o main.o
	ld -m elf_i386 start.o main.o util.o -o task0a

task0b : task0b.o
	ld -m elf_i386 task0b.o util.o -o task0b

main.o : main.c
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector main.c -o main.o 

util.o : util.c
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o

start.o : start.s
	nasm -f elf32 start.s -o start.o 


task0b.o: task0b.s
	nasm -f elf32 task0b.s -o task0b.o

clean:
	rm -f *.o main task0a task0b
