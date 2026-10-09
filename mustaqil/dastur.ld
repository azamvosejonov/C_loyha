/* mustaqil/dastur.ld - B12 test dasturining manzillari.
 * 0x8000000000 = 512 GB: PML4 ning 1-yozuvi. Yadroingiz 0-yozuvda (identity xarita) yoki
 * yuqori yarmida (511-yozuv) bo'lsa ham, user dasturi alohida joyda turadi. */
ENTRY(_start)
SECTIONS
{
    . = 0x8000000000;
    .text : { *(.text) }
    . = ALIGN(0x1000);
    .data : { *(.data) }
    .bss  : { *(.bss) }
}
