; Optimized String Functions in Assembly
; Fast implementations of memcpy and memset

[global memcpy_asm]
[global memset_asm]

; Optimized memcpy using REP MOVSB
; void* memcpy_asm(void* dest, const void* src, size_t n)
memcpy_asm:
    push ebp
    mov ebp, esp
    push edi
    push esi
    
    mov edi, [ebp + 8]   ; dest
    mov esi, [ebp + 12]  ; src
    mov ecx, [ebp + 16]  ; n
    
    ; Check if size is 0
    test ecx, ecx
    jz .done
    
    ; Copy direction (forward)
    cld
    
    ; For small copies, use byte-by-byte
    cmp ecx, 4
    jb .byte_copy
    
    ; Align destination to 4 bytes for faster copying
    mov eax, edi
    and eax, 3
    jz .aligned
    
    ; Copy unaligned bytes first
    mov edx, 4
    sub edx, eax
    cmp ecx, edx
    jb .byte_copy
    
    sub ecx, edx
    rep movsb
    mov ecx, edx
    
.aligned:
    ; Copy 4 bytes at a time
    mov edx, ecx
    shr ecx, 2      ; Divide by 4
    rep movsd       ; Copy 4 bytes at a time
    
    ; Copy remaining bytes
    mov ecx, edx
    and ecx, 3
    jz .done
    
.byte_copy:
    rep movsb
    
.done:
    mov eax, [ebp + 8]   ; Return dest
    pop esi
    pop edi
    pop ebp
    ret

; Optimized memset using REP STOSB
; void* memset_asm(void* s, int c, size_t n)
memset_asm:
    push ebp
    mov ebp, esp
    push edi
    
    mov edi, [ebp + 8]   ; s
    mov eax, [ebp + 12]  ; c (only low byte matters)
    mov ecx, [ebp + 16]  ; n
    
    ; Check if size is 0
    test ecx, ecx
    jz .done
    
    ; Fill direction (forward)
    cld
    
    ; Replicate byte in all 4 bytes of eax
    mov ah, al
    mov edx, eax
    shl eax, 16
    or eax, edx
    
    ; For small fills, use byte-by-byte
    cmp ecx, 4
    jb .byte_fill
    
    ; Align destination to 4 bytes
    mov edx, edi
    and edx, 3
    jz .aligned
    
    ; Fill unaligned bytes first
    ; Calculate number of unaligned bytes: 4 - (edi & 3)
    push eax
    mov eax, edi
    and eax, 3
    mov edx, 4
    sub edx, eax
    pop eax
    
    cmp ecx, edx
    jb .byte_fill
    
    sub ecx, edx
    rep stosb
    mov ecx, edx
    
.aligned:
    ; Fill 4 bytes at a time
    mov edx, ecx
    shr ecx, 2      ; Divide by 4
    rep stosd       ; Fill 4 bytes at a time
    
    ; Fill remaining bytes
    mov ecx, edx
    and ecx, 3
    jz .done
    
.byte_fill:
    rep stosb
    
.done:
    mov eax, [ebp + 8]   ; Return s
    pop edi
    pop ebp
    ret
