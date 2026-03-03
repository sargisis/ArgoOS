[global task_start_asm]
task_start_asm:
    pop eax     ; Get task entry point (it was pushed by task_create)
    sti         ; Enable interrupts
    call eax    ; Call the task function
    
    ; If the task function returns, call task_exit
    extern task_exit
    call task_exit
    
    ; Should never reach here
    cli
.hang:
    hlt
    jmp .hang
