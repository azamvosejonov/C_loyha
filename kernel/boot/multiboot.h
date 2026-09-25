/* =============================================================================
 *  boot/multiboot.h - Multiboot 1 spetsifikatsiyasi strukturalari
 * =============================================================================
 *
 *  Yuklovchi (QEMU yoki GRUB) bizga EBX registrida "multiboot_info" strukturasi
 *  manzilini beradi. Unda:
 *    - RAM xaritasi: qaysi fizik manzillar bo'sh, qaysilari band (BIOS, ACPI...)
 *    - buyruq qatori (cmdline): yadroga berilgan parametrlar
 *    - modullar: yuklovchi yadro bilan birga yuklagan fayllar (bizning initrd.tar)
 *
 *  Rasmiy hujjat: https://www.gnu.org/software/grub/manual/multiboot/multiboot.html
 *
 *  __attribute__((packed)) - kompilyator a'zolar orasiga "tekislash" uchun bo'sh
 *  bayt qo'shmasin. Struktura aniq spetsifikatsiyadagidek bo'lishi SHART.
 * ============================================================================= */
#pragma once

#include <stdint.h>

#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002

/* multiboot_info.flags bitlari: qaysi maydonlar to'ldirilgan. */
#define MB_INFO_MEMORY  (1 << 0)        /* mem_lower / mem_upper */
#define MB_INFO_CMDLINE (1 << 2)        /* cmdline */
#define MB_INFO_MODS    (1 << 3)        /* mods_count / mods_addr */
#define MB_INFO_MMAP    (1 << 6)        /* mmap_length / mmap_addr */

struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;                 /* KB, 0 dan boshlab (odatda 640) */
    uint32_t mem_upper;                 /* KB, 1 MB dan boshlab */
    uint32_t boot_device;
    uint32_t cmdline;                   /* buyruq qatori satrining fizik manzili */
    uint32_t mods_count;                /* modullar soni */
    uint32_t mods_addr;                 /* multiboot_module massivining manzili */
    uint32_t syms[4];
    uint32_t mmap_length;               /* xotira xaritasi hajmi (baytda) */
    uint32_t mmap_addr;                 /* xotira xaritasining manzili */
    /* ... undan keyingi maydonlar bizga kerak emas */
} __attribute__((packed));

/* Xotira xaritasi yozuvi. DIQQAT: "size" maydoni o'zini hisobga OLMAYDI -
 * keyingi yozuv (entry + size + 4) da. */
struct multiboot_mmap_entry {
    uint32_t size;
    uint64_t addr;                      /* hudud boshi (fizik manzil) */
    uint64_t len;                       /* hudud uzunligi */
    uint32_t type;                      /* 1 = bo'sh RAM, boshqasi = band */
} __attribute__((packed));

#define MULTIBOOT_MEMORY_AVAILABLE 1

struct multiboot_module {
    uint32_t mod_start;                 /* modul boshi (fizik) */
    uint32_t mod_end;                   /* modul oxiri (shu baytdan oldin tugaydi) */
    uint32_t cmdline;                   /* modul nomi satri */
    uint32_t reserved;
} __attribute__((packed));
