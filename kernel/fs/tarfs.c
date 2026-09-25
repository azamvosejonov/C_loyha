/* =============================================================================
 *  fs/tarfs.c - TAR arxivini fayl tizimi sifatida o'qish
 * =============================================================================
 *
 *  NEGA TAR:
 *    Haqiqiy fayl tizimi (ext2, FAT) disk drayveri, bloklar keshi, inode'lar
 *    va ancha murakkab tuzilma talab qiladi. TAR esa eng sodda "fayllar
 *    ketma-ketligi": har bir fayl oldidan 512 baytlik sarlavha, keyin
 *    ma'lumot (512 ga yaxlitlangan). Uni Linux'dagi `tar` buyrug'i yaratadi
 *    (Makefile ga qarang). Linux ham yuklanishda "initramfs" (cpio arxivi)
 *    dan xuddi shunday foydalanadi.
 *
 *  USTAR SARLAVHASI (512 bayt, bizga kerakli maydonlar):
 *    siljish  hajm  maydon
 *      0      100   fayl nomi
 *    124       12   hajm - SAKKIZLIK (octal) sanoq tizimidagi ASCII satr!
 *    156        1   tur: '0' yoki '\0' = oddiy fayl, '5' = papka
 *    257        6   "ustar" - format belgisi
 *
 *    ┌─────────┬──────────────────┬─────────┬────────┬───────────┐
 *    │sarlavha │ ma'lumot (hello) │sarlavha │ ma'lum.│ 2x nol    │
 *    │ 512 B   │ 512 ga yaxlitl.  │ 512 B   │  ...   │ blok=oxiri│
 *    └─────────┴──────────────────┴─────────┴────────┴───────────┘
 *
 *  Yuklovchi initrd.tar ni xotiraga "modul" sifatida yuklagan (Multiboot2).
 *  Biz uni NUSXALAMAYMIZ - fayllar ma'lumotiga to'g'ridan-to'g'ri o'sha
 *  xotiradan murojaat qilamiz (pmm.c bu hududni band deb belgilagan).
 * ============================================================================= */
#include "fs/tarfs.h"

#include "lib/kprintf.h"
#include "lib/string.h"
#include "mm/layout.h"

#define TAR_BLOCK   512
#define MAX_FILES   64

static struct tar_file files[MAX_FILES];
static size_t file_count;

/* Sakkizlik ASCII satrni songa aylantirish: "0000000644" -> 420. */
static size_t parse_octal(const char *s, size_t len)
{
    size_t value = 0;
    for (size_t i = 0; i < len && s[i] >= '0' && s[i] <= '7'; i++)
        value = value * 8 + (size_t)(s[i] - '0');
    return value;
}

static void parse_archive(const uint8_t *base, size_t size)
{
    size_t off = 0;
    while (off + TAR_BLOCK <= size && file_count < MAX_FILES) {
        const char *hdr = (const char *)(base + off);
        if (hdr[0] == '\0')             /* nol blok - arxiv oxiri */
            break;
        if (memcmp(hdr + 257, "ustar", 5) != 0) {
            kprintf("[tarfs] %zu-siljishda noto'g'ri sarlavha - to'xtatamiz\n", off);
            break;
        }
        size_t fsize = parse_octal(hdr + 124, 12);
        char type = hdr[156];
        if (off + TAR_BLOCK + fsize > size) {   /* buzilgan arxivdan himoya */
            kprintf("[tarfs] fayl arxivdan tashqariga chiqadi - to'xtatamiz\n");
            break;
        }

        if (type == '0' || type == '\0') {      /* faqat oddiy fayllar */
            struct tar_file *f = &files[file_count++];
            const char *name = hdr;
            if (name[0] == '.' && name[1] == '/')
                name += 2;                      /* "./hello" -> "hello" */
            size_t n = strnlen(name, 99);       /* nom 100 bayt, '\0' bo'lmasligi mumkin */
            memcpy(f->name, name, n);
            f->name[n] = '\0';
            f->data = base + off + TAR_BLOCK;
            f->size = fsize;
        }
        /* Keyingi sarlavha: joriy sarlavha + ma'lumot (512 ga yuqoriga yaxlitlangan). */
        off += TAR_BLOCK + ((fsize + TAR_BLOCK - 1) / TAR_BLOCK) * TAR_BLOCK;
    }
}

void tarfs_init(const struct boot_info *bi)
{
    if (bi->module_count == 0) {
        kprintf("[tarfs] initrd moduli yo'q! (grub.cfg: module2 /boot/initrd.tar)\n");
        return;
    }
    /* Modul - RAM, direct map orqali o'qiymiz. memblock uni band qilgan. */
    const uint8_t *base = phys_to_virt(bi->modules[0].phys_start);
    size_t size = bi->modules[0].phys_end - bi->modules[0].phys_start;
    parse_archive(base, size);
    kprintf("[tarfs] initrd: %p, %zu KB, %zu ta fayl\n", (void *)base, size / 1024, file_count);
}

const struct tar_file *tarfs_find(const char *name)
{
    if (name[0] == '/')                 /* "/hello" ham "hello" ham ishlaydi */
        name++;
    for (size_t i = 0; i < file_count; i++)
        if (strcmp(files[i].name, name) == 0)
            return &files[i];
    return NULL;
}

const struct tar_file *tarfs_get(size_t index)
{
    return index < file_count ? &files[index] : NULL;
}

size_t tarfs_count(void)
{
    return file_count;
}
