/* =============================================================================
 *  linker.ld - HIGHER-HALF yadro tasviri
 * =============================================================================
 *
 *  IKKI XIL MANZIL:
 *    VMA (Virtual Memory Address) - kod ISHLAGANDA qaysi manzilda turadi
 *    LMA (Load Memory Address)    - yuklovchi uni FIZIK xotiraning qayeriga qo'yadi
 *
 *  Yadro kodi 0xFFFFFFFF80100000 (VMA) da ishlaydi, lekin fizik jihatdan
 *  0x100000 (LMA, 1 MB) ga yuklanadi. AT(...) aynan LMA ni belgilaydi.
 *
 *  Muammo: GRUB bizni 32-bitli rejimda, paging O'CHIQ holda chaqiradi - bu
 *  paytda 0xFFFFFFFF80... manzili umuman mavjud emas! Shuning uchun boot.asm
 *  ning birinchi qismi (.boot.*) PAST manzilga (VMA = LMA = 1 MB) link qilinadi.
 *  U sahifa jadvallarini quradi, 64-bitga o'tadi va yuqori yarimga sakraydi.
 *
 *    fizik:   1 MB                                            kernel_end
 *             ├── .boot (past, VMA=LMA) ──┼── .text .rodata .data .bss ──┤
 *    virtual: 0x100000                    0xFFFFFFFF801xxxxx
 * ============================================================================= */

ENTRY(_start)

KERNEL_VMA = 0xFFFFFFFF80000000;

SECTIONS
{
    . = 1M;
    __kernel_phys_start = .;

    /* ---- Past manzildagi boot qismi ---- */
    .boot : ALIGN(8)
    {
        KEEP(*(.multiboot2))            /* Multiboot2 sarlavhasi fayl boshida (32 KB ichida) bo'lishi shart */
        *(.boot.text)
        *(.boot.rodata)
        *(.boot.data)
    }
    .boot.bss (NOLOAD) : ALIGN(4K)
    {
        *(.boot.bss)                    /* boot sahifa jadvallari, kichik 32-bitli stek */
    }

    /* ---- Yuqori yarimga o'tamiz: VMA = LMA + KERNEL_VMA ---- */
    . = ALIGN(4K);
    . += KERNEL_VMA;
    __kernel_start = .;

    /* Har bir bo'lim alohida sahifadan boshlanadi: vmm.c ularga turli ruxsat
     * beradi (kod: bajarish mumkin, yozish mumkin emas; ma'lumot: aksincha).
     * Bu W^X (write XOR execute) tamoyili - zamonaviy himoyaning asosi. */
    .text : AT(ADDR(.text) - KERNEL_VMA) ALIGN(4K)
    {
        __text_start = .;
        *(.text .text.*)
        __text_end = .;
    }

    .rodata : AT(ADDR(.rodata) - KERNEL_VMA) ALIGN(4K)
    {
        __rodata_start = .;
        *(.rodata .rodata.*)
        __rodata_end = .;
    }

    .data : AT(ADDR(.data) - KERNEL_VMA) ALIGN(4K)
    {
        __data_start = .;
        *(.data .data.*)
    }

    .bss : AT(ADDR(.bss) - KERNEL_VMA) ALIGN(4K)
    {
        *(COMMON)
        *(.bss .bss.*)
        __data_end = .;
    }

    . = ALIGN(4K);
    __kernel_end = .;
    __kernel_phys_end = . - KERNEL_VMA;

    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
        *(.note .note.*)
    }
}
