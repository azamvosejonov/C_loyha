/* link.ld — firmware joylashuvi: RAM boshidan 0x8000_0000. Yadro 0x8040_0000 dan boshlanadi:
   orada 4 MB — firmware kodi, ma'lumoti va steki uchun (Linux bu joyni "reserved-memory" deb biladi). */
OUTPUT_ARCH(riscv)
ENTRY(_start)
SECTIONS
{
    . = 0x80000000;
    .text : { *(.text.boshi) *(.text*) }
    .rodata : { *(.rodata*) *(.srodata*) }
    .data : { *(.data*) *(.sdata*) }
    .bss : { __bss_boshi = .; *(.bss*) *(.sbss*) *(COMMON) __bss_oxiri = .; }
    . = ALIGN(16);
    . += 16384;                                 /* 16 KB stek */
    __stek_tepasi = .;
}
