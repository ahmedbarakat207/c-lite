[bits 32]
section .text
global setjmp
global longjmp

setjmp:
    mov edx, [esp + 4]          ; jmp_buf pointer
    mov [edx + 0], ebx
    mov [edx + 4], esi
    mov [edx + 8], edi
    mov [edx + 12], ebp
    lea ecx, [esp + 4]          ; caller's esp
    mov [edx + 16], ecx
    mov ecx, [esp]              ; return address
    mov [edx + 20], ecx
    xor eax, eax
    ret

longjmp:
    mov edx, [esp + 4]          ; jmp_buf pointer
    mov eax, [esp + 8]          ; return value
    test eax, eax
    jnz .val_ok
    mov eax, 1
.val_ok:
    mov ebx, [edx + 0]
    mov esi, [edx + 4]
    mov edi, [edx + 8]
    mov ebp, [edx + 12]
    mov esp, [edx + 16]         ; restore esp
    mov ecx, [edx + 20]         ; return address
    jmp ecx
