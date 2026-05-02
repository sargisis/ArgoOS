[BITS 32]

global gdt_flush

gdt_flush:
    mov eax, [esp + 4]  ; Get the pointer to the GDT pointer from the stack
    lgdt [eax]          ; Load the new GDT

    ; Reload segment registers
    mov ax, 0x10        ; 0x10 is the offset in the GDT to our data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Far jump to reload CS (Code Segment)
    ; 0x08 is the offset in the GDT to our code segment
    jmp 0x08:.flush

.flush:
    ret
