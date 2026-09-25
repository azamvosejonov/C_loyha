/* =============================================================================
 *  fs/initrd.c - INITRD: boot arxivini tmpfs ga ochish
 * =============================================================================
 *
 *  GRUB yadro bilan birga initrd.tar ni xotiraga yuklaydi (Multiboot2 modul).
 *  Unda boshlang'ich dasturlar (/bin/sh, /bin/ls ...) va sozlamalar (/etc).
 *  Biz arxivni o'qib, har bir papka va faylni tmpfs ildiziga YARATAMIZ. Keyin
 *  arxiv xotirasi kerak emas. Linux initramfs'ni aynan shunday ochadi (cpio).
 *
 *  USTAR SARLAVHASI (512 bayt):
 *      0  nom (100)      100 ruxsatlar, sakkizlik (8)     124 hajm, sakkizlik (12)
 *    156  tur: '0' fayl, '5' papka                         257 "ustar"
 *    345  prefiks (155) - uzun yo'llar uchun: to'liq nom = prefiks + "/" + nom
 * ============================================================================= */
#include "fs/initrd.h"

#include "fs/vfs.h"
#include "lib/kprintf.h"
#include "lib/string.h"
#include "mm/layout.h"

#define TAR_BLOCK 512

static uint64_t parse_octal(const char *s, size_t len)
{
    uint64_t v = 0;
    for (size_t i = 0; i < len && s[i] >= '0' && s[i] <= '7'; i++)
        v = v * 8 + (uint64_t)(s[i] - '0');
    return v;
}

int initrd_unpack(const struct boot_info *bi)
{
    if (bi->module_count == 0) {
        kprintf("[initrd] modul yo'q!\n");
        return 0;
    }
    const uint8_t *base = phys_to_virt(bi->modules[0].phys_start);
    size_t size = bi->modules[0].phys_end - bi->modules[0].phys_start;
    int files = 0, dirs = 0;
    size_t off = 0;
    while (off + TAR_BLOCK <= size) {
        const char *h = (const char *)(base + off);
        if (h[0] == '\0')
            break;                      /* arxiv oxiri */
        if (memcmp(h + 257, "ustar", 5) != 0) {
            kprintf("[initrd] %zu da buzilgan sarlavha\n", off);
            break;
        }
        uint64_t fsize = parse_octal(h + 124, 12);
        uint32_t mode = (uint32_t)parse_octal(h + 100, 8) & 07777;
        char type = h[156];
        if (off + TAR_BLOCK + fsize > size)
            break;

        /* To'liq yo'l: "/" + prefiks + "/" + nom (oxiridagi "/" siz). */
        char path[PATH_MAX], name[101], prefix[156];
        memcpy(name, h, 100);
        name[100] = '\0';
        memcpy(prefix, h + 345, 155);
        prefix[155] = '\0';
        if (prefix[0])
            ksnprintf(path, sizeof(path), "/%s/%s", prefix, name);
        else
            ksnprintf(path, sizeof(path), "/%s", name);
        size_t pl = strlen(path);
        while (pl > 1 && path[pl - 1] == '/')
            path[--pl] = '\0';

        if (type == '5') {
            if (strcmp(path, "/") != 0 && strcmp(path, "/.") != 0 && vfs_mkdir(path, NULL, mode) == 0)
                dirs++;
        } else if (type == '0' || type == '\0') {
            struct file *f;
            if (vfs_open(path, NULL, O_WRONLY | O_CREAT | O_TRUNC, mode, &f) == 0) {
                file_write(f, base + off + TAR_BLOCK, fsize);
                file_close(f);
                files++;
            } else {
                kprintf("[initrd] %s yaratilmadi\n", path);
            }
        }
        off += TAR_BLOCK + ((fsize + TAR_BLOCK - 1) / TAR_BLOCK) * TAR_BLOCK;
    }
    kprintf("[initrd] %zu KB arxiv: %d ta fayl, %d ta papka -> tmpfs (/)\n", size / 1024, files,
            dirs);
    return files;
}
