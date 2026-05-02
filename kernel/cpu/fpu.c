#include "cpu.h"
#include "printf.h"

void cpu_init_fpu(void) {
    uint32_t cr0, cr4;

    // 1. Enable FPU
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= (1 << 1);    // Set MP (Monitor Coprocessor)
    cr0 &= ~(1 << 2);   // Clear EM (Emulation) - we want real FPU
    cr0 |= (1 << 5);    // Set NE (Numeric Error)
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
    
    // Initialize FPU
    asm volatile("finit");

    // 2. Enable SSE
    asm volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= (1 << 9);    // Set OSFXSR (OS Support for FXSAVE/FXRSTOR)
    cr4 |= (1 << 10);   // Set OSXMMEXCPT (OS Support for Unmasked SIMD Exceptions)
    asm volatile("mov %0, %%cr4" :: "r"(cr4));

    kprintf("FPU and SSE support enabled\n");
}
