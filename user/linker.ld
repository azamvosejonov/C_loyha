/* =============================================================================
 *  user/linker.ld - user dasturlari uchun linker skripti
 * =============================================================================
 *
 *  Har bir user dastur 0x400000 (4 MB) manzilga link qilinadi - bu yadro
 *  belgilagan USER_SPACE_START (kernel/mm/vmm.h). Virtual xotira tufayli
 *  BARCHA dasturlar bir xil manzilda "yashaydi", lekin har biri o'z fizik
 *  freymlariga ega.
 *
 *  Har bir bo'lim alohida sahifadan boshlanadi: kod (R-X), o'zgarmaslar (R),
 *  ma'lumotlar (RW) turli sahifalarda bo'lsa, yadro ularga turli ruxsatlar
 *  bera oladi (masalan, kodni yozib bo'lmaydigan qilish).
 * ============================================================================= */
ENTRY(_start)

SECTIONS
{
    . = 0x400000;

    .text : ALIGN(4K)
    {
        *(.text.start)                  /* crt0 ning _start i birinchi */
        *(.text .text.*)
    }

    .rodata : ALIGN(4K)
    {
        *(.rodata .rodata.*)
    }

    .data : ALIGN(4K)
    {
        *(.data .data.*)
    }

    .bss : ALIGN(4K)
    {
        *(COMMON)
        *(.bss .bss.*)
    }

    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
        *(.note .note.*)
    }
}
