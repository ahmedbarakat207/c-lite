[bits 32]
section .data
default_app db "sh", 0
default_argv dd default_app, 0
default_envp dd 0

section .text
global _start
extern main
extern exit
extern environ

_start:
    xor ebp, ebp            ; clear frame ptr

    ; extract argc, argv, envp 
    mov eax, [esp]          ; argc
    test eax, eax
    jle .no_args

    lea edx, [esp + 4]      ; argv
    lea ecx, [esp + eax*4 + 8] ; envp
    mov [environ], ecx

    push ecx                ; envp
    push edx                ; argv
    push eax                ; argc
    call main
    jmp .done

.no_args:
    mov dword [environ], default_envp
    push default_envp       ; envp
    push default_argv       ; argv
    push 1                  ; argc = 1
    call main

.done:
    push eax                ; exit
    call exit

.halt:
    hlt
    jmp .halt
