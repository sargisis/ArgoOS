/* Linker script для FlowDay-OS Kernel */

ENTRY(_start)

SECTIONS
{
    /* Ядро загружается по адресу 1MB (стандарт для Multiboot) */
    . = 0x100000;
    
    /* Multiboot заголовок должен быть в первых 8KB */
    .multiboot : {
        *(.multiboot)
    }
    
    /* Код */
    .text : ALIGN(4K)
    {
        *(.text)
    }
    
    /* Только для чтения данные */
    .rodata : ALIGN(4K)
    {
        *(.rodata)
    }
    
    /* Инициализированные данные */
    .data : ALIGN(4K)
    {
        *(.data)
    }
    
    /* Неинициализированные данные */
    .bss : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    }
}
