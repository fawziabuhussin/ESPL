
# Lab 4 :

-----------------------------------------------------------------
#                        TASK 0 
-----------------------------------------------------------------


## Task0a: 
`What shall we do?`
```
readelf -h abc
Entry point address:               0x80483b0
Number of section headers:         29
```


## Task0b: 
is already in Task1, which is :
```
 ./task1 
* 1 => abc
* 2 => 2
```


-----------------------------------------------------------------
 #                       TASK 1 
-----------------------------------------------------------------


Task1(a-c):
./task1
Option: 1 
    abc
Option: 2
    2
Option: 3
    12F 10
Option: 5
    0 5


Task1d :
> ./task1
Option : 1 => abc
Option : 2 => 2
Option : 3 => 12F 10
Option : 6 => 960c170 33 4
> hexedit abc: look at bytes from 33 to 38 (10 bytes) it will be changed to : 70  C1 60 09 00

                                                            it was before : 00 06 00 00 00


Task1e :

Option : 1 => abc
Option : 2 => 2
Option : 3 => 12F 10
OPtion : 7 => 40 804808a


-----------------------------------------------------------------
                    TASK 2  :  Reading ELF
-----------------------------------------------------------------


Task2:


1) Run the file.
2) Which "function" should precede main( ) in execution ? 
ANSWER : _start.
3) What is the virtual address to which this function is loaded :  
ANSWER : 08048350


How to fix?
set file name (1): deep_thought 
save into file (6):  18 1 

Explanation: 

Why the file is bugged (is not working or hacked), file is corrupted because
the offset in (readelf -h deep_thought) is not correct, is not equal to _start. (wrong entry point address: 0x8048464)
to adjust it we need to modify the file by the right offset which is 08048350 (the offset of _start)

Lets do this, step by step!
> ./deep_thought (NO PERMISSIONS!)
> chmod u+wx deep_thought
> ./deep_thought (it is not working!)
> ./task1
Option: 1
deep_thought 
Option: 3
18 1 (Explain: it is the location of the entry in the head (readelf -h deep_thought))
Option: 4
Option: 5 (result is 0x8048350)
0 1 (0: in the mem_buf, length: 1 - 1*4 bytes only (because unit is 4).)
Option: 7
0 08048350 /* setting start as the entry point */
Option: 6
0 18 4  (0 : read from mem_buf ; 18 from where to write; 1 until where)
 

-----------------------------------------------------------------
            TASK 3 : Delving Deeper into the ELF Structure
-----------------------------------------------------------------

*** NOTICE: THESE INFORMATION ARE BASED ON DEEP_THOGUHT, THE NEXT PARAGRAPH IS ABOUT OFFENIVE ******
Doing required steps : 
1) readelf -s deep_thought :- 64: 0804844d    23 FUNC    GLOBAL DEFAULT   13 main
2) Virtual addr : 0804844d, size : 23
3) readelf -S deep_thought :- [13] .text    PROGBITS    08048350 000350 0001b2 00  AX  0   0 16
4) Virtual addr : 08048350, offset : 000350


1) what is main offset?
main offset = (addres main) - (address .text) + (offset .text) = 0x44D
    the .text address is  08048350 (readelf -S deep_thought [Section 13] => addr)
    the .text offset is 000350 (readelf -S deep_thought => [SECTION 13] -> offset)
    the main address 0804844d (readelf -s deep_thought => [61 => main => addr])

    2) what is the size of fun main? 23. (readelf -s deep_thought => num 64[main] => size[2])
    
    3) set file name (1): deep_thought 
        set unit size (2): 1 
        display memory (5): 44d 23



Option: 1
deep_thought
Option: 2
1
Option: 3
44d 23 (Offset of the main, save from the offset to the size of the main).
Option: 4
Option: 5
0 23 (0: because we stored in mem_buf ; 23 is the size of the main (we stored)..)


// How to hack? adding RET at the begining.
... (execute from 131 to 139 and continue here)
Now change the buffer using 7: 
Option : 7
0 c3   (C3 = return)
Option: 5
0 23 (0: because we stored in mem_buf ; 23 is the size of the main (we stored)..)
Option :  6
0 44d 4   (use the number that you calculate)


******** NOTICE : THIS IS THE INFORMATION ABOUT OFFENSIVE **********
> readelf -s offensive :- 62: 0804841d    23 FUNC    GLOBAL DEFAULT   13 main
> readelf -S offensive :-  [13] .text             PROGBITS        08048320 000320 000192 00  AX  0   0 16

Doing this for offensive :

main virtual address : 0804841d
main size : 23
text virtual address : 08048320
text offset : 000320

main offset = (addres main) - (address .text) + (offset .text) = 41D
    the .text address is  08048320 (readelf -S deep_thought [Section 13] => addr)
    the .text offset is 000320 (readelf -S deep_thought => [SECTION 13] -> offset)
    the main address 0804841d (readelf -s deep_thought => [61 => main => addr])
Option: 1
offensive
Option: 2
1
Option: 3
41D 23 (Offset of the main, save from the offset to the size of the main).
Option: 4
Option: 5
0 23 (0: because we stored in mem_buf ; 23 is the size of the main (we stored)..)
Option : 7
0 c3   (C3 = return)
Option: 5
0 23 (0: because we stored in mem_buf ; 23 is the size of the main (we stored)..)
Option :  6
0 44d 1   (use the number that you calculate)



-----------------------------------------------------------------
                        TASK 4 
-----------------------------------------------------------------


Explanation:   
    ** task4 **
    > (readelf -s task4)
    Virtual address:   0000056d
    size: 80
    section: 14
    > (readelf -S task4)
    Virtual address:   00000470
    offset: 00000470

    ** ntsc **
    > (readelf -s ntsc)
    Virtual address: 0804847d
    size:  93
    section: 13
    > (readelf -S ntsc)
    txt Virtual address:   00000410
    txt offset: 00000410

Error occurs in ntsc because it does not count 0's nor 9's.
Task4: 
Option 1: 
task4.
Option 2: 
1
Option 3: 
56d 80 
Option 4:

Option 5: 
0 80 

Option 1:
ntsc.
Option 6:
0 47D 80 


047D               =  0804847d                -      08048380                  + 000380  
Offset-Count-Digit = Virtual-Add-Count-Digits - Virtual-Addr-Text-Section + Offset-Txt-Section.
