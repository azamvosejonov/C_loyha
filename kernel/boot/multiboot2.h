/* =============================================================================
 *  boot/multiboot2.h - Multiboot2 spetsifikatsiyasi strukturalari
 * =============================================================================
 *
 *  GRUB bizga EBX da "boot information" strukturasining fizik manzilini beradi:
 *
 *     u32 total_size | u32 reserved | teg | teg | ... | teg(type=0, oxiri)
 *
 *  Har bir teg: u32 type, u32 size, ma'lumot... va keyingi teg 8 baytga
 *  tekislangan manzilda boshlanadi. Bu "TLV" (Type-Length-Value) formati -
 *  kengaytiriladigan binar formatlarning klassik usuli (ELF, PNG, USB ham shunday).
 *
 *  Rasmiy hujjat: https://www.gnu.org/software/grub/manual/multiboot2/
 * ============================================================================= */
#pragma once

#include <stdint.h>

#define MB2_BOOTLOADER_MAGIC 0x36D76289

#define MB2_TAG_END          0
#define MB2_TAG_CMDLINE      1
#define MB2_TAG_BOOTLOADER   2
#define MB2_TAG_MODULE       3
#define MB2_TAG_BASIC_MEM    4
#define MB2_TAG_MMAP         6
#define MB2_TAG_FRAMEBUFFER  8
#define MB2_TAG_ACPI_OLD     14         /* RSDP v1 nusxasi */
#define MB2_TAG_ACPI_NEW     15         /* RSDP v2 (XSDT bilan) nusxasi */

struct mb2_info {
    uint32_t total_size;
    uint32_t reserved;
} __attribute__((packed));

struct mb2_tag {
    uint32_t type;
    uint32_t size;
} __attribute__((packed));

struct mb2_tag_string {                 /* cmdline, bootloader nomi */
    uint32_t type, size;
    char string[];
} __attribute__((packed));

struct mb2_tag_module {
    uint32_t type, size;
    uint32_t mod_start;                 /* fizik */
    uint32_t mod_end;
    char cmdline[];
} __attribute__((packed));

struct mb2_mmap_entry {
    uint64_t addr;
    uint64_t len;
    uint32_t type;                      /* 1 = RAM, 3 = ACPI reclaimable, 4 = ACPI NVS, 5 = buzuq */
    uint32_t zero;
} __attribute__((packed));

struct mb2_tag_mmap {
    uint32_t type, size;
    uint32_t entry_size;                /* har bir yozuv hajmi (kelajakda kattalashishi mumkin!) */
    uint32_t entry_version;
    struct mb2_mmap_entry entries[];
} __attribute__((packed));

#define MB2_FB_TYPE_INDEXED 0
#define MB2_FB_TYPE_RGB     1
#define MB2_FB_TYPE_TEXT    2           /* EGA matn rejimi (0xB8000) */

struct mb2_tag_framebuffer {
    uint32_t type, size;
    uint64_t addr;                      /* framebuffer fizik manzili */
    uint32_t pitch;                     /* bitta qatordagi baytlar soni (width*bpp/8 dan katta bo'lishi mumkin!) */
    uint32_t width;
    uint32_t height;
    uint8_t bpp;                        /* piksel uchun bitlar */
    uint8_t fb_type;
    uint16_t reserved;
    /* RGB uchun: har bir rang komponentining o'rni va uzunligi */
    uint8_t red_pos, red_size;
    uint8_t green_pos, green_size;
    uint8_t blue_pos, blue_size;
} __attribute__((packed));

struct mb2_tag_acpi {
    uint32_t type, size;
    uint8_t rsdp[];                     /* RSDP strukturasining to'liq nusxasi */
} __attribute__((packed));
