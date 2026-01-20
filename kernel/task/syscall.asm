; System Call Entry Point
; INT 0x80 handler

[extern syscall_handler]

[global syscall_entry]

syscall_entry:
    ; Save all registers
    push edi
    push esi
    push ebp
    push esp
    push ebx
    push edx
    push ecx
    push eax
    
    ; Push arguments (in reverse order)
    push edx  ; arg4
    push ecx  ; arg3
    push ebx  ; arg2
    push eax  ; arg1 (syscall number)
    
    ; Call C handler
    call syscall_handler
    
    ; Clean up stack (remove arguments)
    add esp, 16
    
    ; Restore registers
    pop eax
    pop ecx
    pop edx
    pop ebx
    pop esp
    pop ebp
    pop esi
    pop edi
    
    ; Return value is in EAX
    iret
