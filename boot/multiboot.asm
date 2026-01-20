; FlowDay-OS Bootloader Entry Point
; Multiboot 1 спецификация

section .multiboot
align 4
    ; Multiboot заголовок
    dd 0x1BADB002              ; Magic number
    dd 0x00000003              ; Flags: align modules on page boundaries, provide memory map
    dd -(0x1BADB002 + 0x00000003)  ; Checksum

section .bss
align 16
stack_bottom:
    resb 16384                 ; 16 KB стек
stack_top:

section .text
global _start
extern kernel_main

_start:
    ; Настройка стека
    mov esp, stack_top
    
    ; Сохранение указателя на Multiboot структуру
    push ebx                    ; Multiboot info structure
    push eax                    ; Multiboot magic number
    
    ; Вызов основного ядра
    call kernel_main
    
    ; Если kernel_main вернется (не должно быть)
    cli
.hang:
    hlt
    jmp .hang
