/* link.ld — dastur RAM boshiga (0x80000000) joylashadi; _start eng birinchi */
ENTRY(_start)
SECTIONS
{
    . = 0x80000000;
    .text : { *(.text.boshlash) *(.text*) }
    .rodata : { *(.rodata*) }
    .data : { *(.data*) *(.sdata*) }
    .bss : { *(.bss*) *(.sbss*) }
}
