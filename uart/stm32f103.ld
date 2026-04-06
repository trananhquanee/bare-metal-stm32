MEMORY 
{
    FLASH (rx) : ORIGIN = 0X08000000, LENGTH = 64K
    RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}

SECTIONS
{
    .text :
    {
        KEEP(*(.isr_vector)) /* bảng vector thì phải đầu tiên*/
        *(.text*)
        *(.rodata*)
    }>FLASH
    .data :
    {
        *(.data*)
    }> RAM AT > FLASH
    .bss :
    {
        *(.bss*)
    } > RAM
    /*STACK CUỐI RAM*/
    _stack_top = ORIGIN(RAM) + LENGTH(RAM);
}

