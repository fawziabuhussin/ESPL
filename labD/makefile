
compC = gcc -no-pie -m32 -g -Wall -c -o
complC = gcc -m32 -g -Wall -o
complS = nasm -g -f elf32 -w+all -o

.PHONY : clean

all: multi

multi :  multi.o
	gcc -m32 multi.o -o multi 

multi.o : multi.s
	nasm -g -f elf32 multi.s -o multi.o

clean:
	rm -f *.o multi

