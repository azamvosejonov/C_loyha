/* SBI testi "yadro" o'rnida: firmware uni 0x8040_0000 da S rejimda ishga tushiradi */
OUTPUT_ARCH(riscv)
ENTRY(_start)
SECTIONS
{
    . = 0x80400000;
    .text : { *(.text*) }
    .data : { *(.data*) *(.rodata*) }
}
