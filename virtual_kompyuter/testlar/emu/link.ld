/* testlar uchun bog'lovchi skript: kod RAM boshidan (0x8000_0000) */
ENTRY(_start)
SECTIONS
{
    . = 0x80000000;
    .text : { *(.text.boshi) *(.text*) }
    .rodata : { *(.rodata*) }
    .data : { *(.data*) }
    .bss : { *(.bss*) }
}
