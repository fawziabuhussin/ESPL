section .data
    MASK: dd 0xB400        ; Mask for 16-bit Fibonacci LFSR
    STATE: dd 0x1234       ; seed
    
    x_struct: dd 5
    x_num: db 0xaa, 1,2,0x44,0x4f
    
    y_struct: dd 6
    y_num: db 0xaa, 1,2,3,0x44,0x4f

    print_struct: db "%02x", 0, 0
    newLine: db "", 10, 0 
    argc_format: db "The argc is : %s",10,0
    TESTING: db "TESTING %d", 10, 0
    randinp db "-R",0 
    stdinp db "-I", 0

    BUFFER_SIZE equ 600
    buffer      resb BUFFER_SIZE
    prompt db "Enter input: ", 0
    prompt_len equ $ - prompt

section .rodata 
    stateline: db "STATE: %x", 10, 0

section .bss
    c_struct: dd 5
    c_num db 600
    d_struct: dd 5
    d_num db 600
    

section .text
    global main
    global rand_num
    global MaxMin
    global add_multi
    global PRmulti
    extern printf
    global print_multi
    extern malloc
    extern free
    extern strncmp
    extern fgets
    extern stdin



main:
    push ebp
    mov ebp, esp

    mov esi, [ebp+12]           ; argv
    mov edi, [ebp+8]            ; argc

    
    cmp edi, 2                  ; argc = 2 
    jne defInp                  ; if more than 2 or less.

    add esi, 4
    mov edx, [esi]


    ; CHECK -R.
    pushad
    push 2                      ; check if first 2 chars are -i
    push edx                    ;  save "-R"
    push randinp
    call strncmp                ;  int strncmp(inPutString, edx, 2); 
    add esp, 12                 ;  This instruction adjusts the stack pointer to remove the arguments to strncmp from the stack.
    cmp eax, 0                  ;  eax == 0 <=> "-i".equal(edx).
    popad
    jz processRand              ; RAND.
    
    
    ; CHECK IF "-I"
    pushad
    push 2                      ; check if first 2 chars are -i
    push edx                    ;  save "-R"
    push stdinp
    call strncmp                ;  int strncmp(inPutString, edx, 2); 
    add esp, 12                 ;  This instruction adjusts the stack pointer to remove the arguments to strncmp from the stack.
    cmp eax, 0                  ;  eax == 0 <=> "-i".equal(edx).
    popad
    jz getinp           ; INPUT.
    
    ; else

    jmp finish
    
    defInp:
    
        push dword x_struct
        call print_multi
        push dword y_struct
        call print_multi
        call add_multi
        jmp finish

    getinp:
         
        call getMulti
        add esp, 8

        swap:

        
        push newLine
        call printf
        add esp, 4

         
        call getMulti
        add esp, 8

        push newLine
        call printf
        add esp, 4

            ; Swap c_struct and d_struct
        mov eax, [d_struct]
        mov ebx, [c_struct]
        mov [c_struct], eax
        mov [d_struct], ebx

        ; Swap c_num and d_num
        mov al, [d_num]
        mov bl, [c_num]
        mov [c_num], al
        mov [d_num], bl

        push dword c_struct
        push dword d_struct
        call add_multi

        jmp finish  


    processRand:

        ; get first number.
        push c_num
        push c_struct    
        call PRmulti
        add esp, 8

        push newLine
        call printf
        add esp, 4

        ; get second number.
        push d_num
        push d_struct    
        call PRmulti
        add esp, 8

        push newLine
        call printf
        add esp, 4
        ; add both

        push dword c_struct
        push dword d_struct
        call add_multi

        jmp finish


    finish:
    mov esp, ebp
    pop ebp
    ret



; PRINT MULTI

print_multi:
    push ebp
    mov ebp, esp
    mov edi, [ebp+8]            ; points to the first arg. (to the size)
    mov esi, [edi]
    mov ecx, [edi]

    mov ebx, edi
    add ebx, 4                  ; now points to first element in the array.
    go:
        add ebx, 1
        dec ecx
        cmp ecx, 1
        jnz go

    loopOver:
        cmp esi, 0
        jz finishline
        
        sub esi, 1

        mov ecx,0
        mov cl, [ebx]
        push dword ecx
        push dword print_struct
        
        call printf
        sub ebx, 1

        add esp, 8
        jmp loopOver

    finishline:
    push dword newLine
    call printf
    add esp, 4

    mov esp, ebp
    pop ebp
    ret

; ADD MULTI


add_multi:
    push    ebp  
    mov     ebp, esp  

    mov eax, [ebp+8]        ;get sturct_x
    mov ebx, [ebp+12]       ;get struct_y
    
    call MaxMin
    
    ; push malloc args
    push dword ebx
    push dword ecx
    push dword edi

    call malloc

    pop dword edi
    pop dword ecx
    pop dword ebx

      ; ebx = struct of the longer one 
    sub edi, 4
    push dword eax              ; save the returned value form malloc
    mov dword [eax], edi        


     ; edi = max length          esi = lower length
    mov esi, [ebx]
    mov edi, [ecx]
    add eax, 4
    add ebx, 4
    add ecx, 4
    sub esi, edi

    CLC                         ;clears carry flag

    inc edi
    inc esi
    
    loop:
        dec edi
        jz over_reached

        mov dl,byte [ecx]
        adc dl,byte [ebx]
        mov byte[eax], dl

        inc ecx
        inc eax
        inc ebx

        jmp loop

    over_reached:
        dec esi
        jz ended

        mov dl,byte [ebx]
        adc dl, 0 ; for the first time
        mov byte[eax], dl ; add element with previous carry
        inc ebx
        inc eax

        jmp over_reached

    ended:

    call print_multi
    pop eax
    ;exit func
    mov     esp, ebp
    pop     ebp
    ret  



; max min

MaxMin:
    push    ebp                 
    mov     ebp, esp  
    ;get the maximum length
    mov ecx, [eax]      ; size of struct_x
    mov edx,[ebx]       ;size of struct_y

    cmp ecx,edx 
    jg dont_swap

    mov ecx, ebx
    mov ebx, eax
    mov eax, ecx

    dont_swap:

    ; malloc
    mov ecx, ebx
    mov ebx, eax
    mov edi, [ebx]
    add edi, 4

    mov     esp, ebp
    pop     ebp
    ret    




; RAND NUM


rand_num:
    push ebp
    mov ebp, esp
    
    mov ax, [STATE]

    xor ax, [MASK]    
    jpe parity_even             ; Jump if parity is even (Parity flag is already set)
    jmp parity_odd
    
    parity_even:
    shr ax, 1
    mov [STATE], ax
    mov eax, [STATE] 
    jmp randfinish
    
    parity_odd:
    STC 
    RCR ax ,1 
    mov [STATE], ax
    mov eax, [STATE]  

    randfinish:
    mov esp, ebp
    pop ebp
    ret


PRmulti:
    push ebp
    mov ebp, esp

    ; Function prologue
    generate_length:
        call rand_num
        test al, al
        jz generate_length

    ; Save the length in x_struct
    mov ecx, [ebp + 8]     ; Get x_struct parameter (first function argument)
    mov edi, [ebp + 12]
    push ecx
    movzx edx, al
    mov [ecx], edx

    
    ; Allocate memory for the number
    movzx edx, al
    shl edx, 3               ; Multiply n by 8
    push edx
    call malloc
    add esp, 4

    mov [edi], eax            ; Store the base address of x_num in ebx

    pop ecx
    mov esi, [ecx]            ; Counter


generate_number:
    push esi
    push edi
    call rand_num
    pop edi
    mov [edi + esi], al     ; Access x_num using ebx as the base address

    pop esi
    dec esi                 ; Decrement the loop counter
    cmp esi, 0
    jnz generate_number

    ; Print
    push dword [ebp + 8]    ; Pass x_struct as an argument
    call print_multi
    add esp, 4

    ; Clean up

end:
    mov esp, ebp
    pop ebp
    ret



getMulti:
        push ebp
        mov ebp, esp    
    
    
        mov eax, 4          
        mov ebx, 1          
        mov ecx, prompt     
        mov edx, prompt_len 
        int 0x80            

        push    dword [stdin]
        push    dword BUFFER_SIZE 
        push    buffer            
        call    fgets
        add     esp, 12



    ; Process input and store in x_num
        mov esi, buffer
        mov edi, c_num

        mov bl, [esi]       
        cmp bl, 0           ; check if end of input
        je end_input


    process_input:
        mov bh, [esi]   
        shl bl, 4           
        or bl, bh           
        mov [edi], bl       ; store in x_num
        inc edi             
        
        add esi, 2          

        mov bl, [esi]       ; Read the next character
        cmp bl, 0           ; Check if end of input
        je end_input

        jmp process_input

        
    end_input:
        ; Calculate the size of x_num
        sub edi, c_num
        
        push dword edi
        call printf
        add esp, 8
        
        mov [c_struct], edi
        push dword c_struct
        call print_multi

        finishgetMulti:
            mov esp, ebp
            pop ebp
            ret
