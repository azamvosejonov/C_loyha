/* =============================================================================
 *  boot/bootinfo.h - yuklovchidan olingan ma'lumotlarning YADRO ichidagi nusxasi
 * =============================================================================
 *
 *  NEGA NUSXA: Multiboot2 strukturasi GRUB qo'ygan joyda turadi. Xotira
 *  menejeri ishga tushgach, bu joy qayta ishlatilishi mumkin. Shuning uchun
 *  kerakli hamma narsani ENG BOSHIDA o'z statik strukturamizga ko'chiramiz va
 *  yadroning qolgan qismi faqat shu strukturani biladi - Multiboot2 haqida
 *  bilmaydi. Ertaga Limine yoki UEFI stub'ga o'tsak, faqat bootinfo.c o'zgaradi.
 * ============================================================================= */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define BOOT_MAX_MMAP     128
#define BOOT_MAX_MODULES  8
#define BOOT_CMDLINE_MAX  256

enum mem_type {
    MEM_USABLE = 1,
    MEM_RESERVED = 2,
    MEM_ACPI_RECLAIM = 3,
    MEM_ACPI_NVS = 4,
    MEM_BAD = 5,
};

struct mem_region {
    uint64_t base;
    uint64_t len;
    uint32_t type;
};

struct boot_module {
    uint64_t phys_start;
    uint64_t phys_end;
    char name[64];
};

struct boot_framebuffer {
    bool present;
    bool text_mode;                     /* EGA 80x25 matn rejimi */
    uint64_t phys;
    uint32_t pitch, width, height;
    uint8_t bpp;
    uint8_t red_pos, green_pos, blue_pos;
    uint8_t red_size, green_size, blue_size;
};

struct boot_info {
    char cmdline[BOOT_CMDLINE_MAX];
    char bootloader[64];
    struct mem_region mmap[BOOT_MAX_MMAP];
    size_t mmap_count;
    struct boot_module modules[BOOT_MAX_MODULES];
    size_t module_count;
    struct boot_framebuffer fb;
    uint8_t rsdp[36];                   /* ACPI RSDP nusxasi (v2 = 36 bayt) */
    bool rsdp_present;
    uint64_t mbi_phys, mbi_size;        /* multiboot strukturasi (memblock uni band qiladi) */
};

extern struct boot_info boot_info;

/* mbi_phys - GRUB bergan manzil. Identity/HHDM boot xaritasi hali faol. */
void bootinfo_parse(uint32_t magic, uint64_t mbi_phys);
void bootinfo_dump(void);
bool cmdline_has(const char *word);
/* "key=value" dan value ni olish. Yo'q bo'lsa NULL. */
const char *cmdline_get(const char *key, char *buf, size_t size);
