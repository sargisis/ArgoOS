; Context Switch Assembly
; Low-level context switching between tasks

[global task_switch_asm]

task_switch_asm:
    ; Save old task's context
    push ebp
    push ebx
    push esi
    push edi
    
    ; Save old ESP
    mov eax, [esp + 20]  ; Get pointer to old_esp
    mov [eax], esp       ; Save current ESP
    
    ; Load new task's ESP
    mov esp, [esp + 24]  ; Get new_esp parameter
    
    ; Restore new task's context
    pop edi
    pop esi
    pop ebx
    pop ebp
    
    ; Return to new task (this will jump to the new task's EIP)
    ret
