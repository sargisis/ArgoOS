; FlowDay-OS Bootloader Entry Point
; Multiboot 1 спецификация

section .multiboot
align 4
    ; Multiboot заголовок
    dd 0x1BADB002              ; Magic number
    dd 0x00000007              ; Flags: align(1) | meminfo(2) | video(4)
    dd -(0x1BADB002 + 0x00000007)  ; Checksum
    
    ; a.out kludge fields (заполняем нулями, так как у нас ELF)
    dd 0
    dd 0
    dd 0
    dd 0
    dd 0
    
    ; Video mode request (800x600, 32-bit color)
    dd 0                       ; 0 = linear graphics mode
    dd 800                     ; width
    dd 600                     ; height
    dd 32                      ; depth

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
