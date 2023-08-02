section .data
    hworld db 'hello world', 10 

section .text
    global _start

    _start:
        ; write message to stdout
        mov eax, 4          
        mov ebx, 1          
        mov ecx, hworld        
        mov edx, 12         
        int 0x80            ; call write

        
        mov eax, 1          
        xor ebx, ebx        
        int 0x80            ; call exit.