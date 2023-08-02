all: myElf

myElf: myElf.o 
	gcc -m32 -g -Wall -o myElf myElf.o

myElf.o: myElf.c
	gcc -g -Wall -m32 -c -o myElf.o myElf.c 

.PHONY: clean 

 #Clean the build directory 

clean:
	rm -f *.o myElf