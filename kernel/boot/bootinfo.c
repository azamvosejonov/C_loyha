/* =============================================================================
 *  boot/bootinfo.c - Multiboot2 teglarini o'qish
 * ============================================================================= */
#include "boot/bootinfo.h"

#include "boot/multiboot2.h"
#include "lib/common.h"
#include "lib/kprintf.h"
#include "lib/panic.h"
#include "lib/string.h"
#include "mm/layout.h"

struct boot_info boot_info;

static const char *mem_type_name(uint32_t t)
{
    switch (t) {
    case MEM_USABLE:       return "RAM";
    case MEM_RESERVED:     return "band";
    case MEM_ACPI_RECLAIM: return "ACPI (qayta ishlatiladi)";
    case MEM_ACPI_NVS:     return "ACPI NVS";
    case MEM_BAD:          return "buzuq RAM";
    default:               return "noma'lum";
    }
}

static void parse_mmap(const struct mb2_tag_mmap *t)
{
    const uint8_t *p = (const uint8_t *)t->entries;
    const uint8_t *end = (const uint8_t *)t + t->size;
    /* entry_size dan foydalanamiz, sizeof dan EMAS: kelajakdagi versiyalarda
     * yozuv kattalashishi mumkin - spetsifikatsiya shunga ruxsat beradi. */
    for (; p + t->entry_size <= end; p += t->entry_size) {
        const struct mb2_mmap_entry *e = (const void *)p;
        if (boot_info.mmap_count == BOOT_MAX_MMAP) {
            kprintf("[boot] ogohlantirish: xotira xaritasi juda uzun, qolgani tashlab yuborildi\n");
            break;
        }
        if (e->len == 0)
            continue;
        struct mem_region *r = &boot_info.mmap[boot_info.mmap_count++];
        r->base = e->addr;
        r->len = e->len;
        r->type = (e->type >= 1 && e->type <= 5) ? e->type : MEM_RESERVED;
    }
}

static void parse_framebuffer(const struct mb2_tag_framebuffer *t)
{
    struct boot_framebuffer *fb = &boot_info.fb;
    fb->phys = t->addr;
    fb->pitch = t->pitch;
    fb->width = t->width;
    fb->height = t->height;
    fb->bpp = t->bpp;
    if (t->fb_type == MB2_FB_TYPE_TEXT) {
        fb->present = true;
        fb->text_mode = true;
    } else if (t->fb_type == MB2_FB_TYPE_RGB && (t->bpp == 32 || t->bpp == 24)) {
        fb->present = true;
        fb->red_pos = t->red_pos;
        fb->red_size = t->red_size;
        fb->green_pos = t->green_pos;
        fb->green_size = t->green_size;
        fb->blue_pos = t->blue_pos;
        fb->blue_size = t->blue_size;
    } else {
        kprintf("[boot] framebuffer turi %u / %u bpp qo'llab-quvvatlanmaydi\n", t->fb_type, t->bpp);
    }
}

void bootinfo_parse(uint32_t magic, uint64_t mbi_phys)
{
    if (magic != MB2_BOOTLOADER_MAGIC)
        panic("Multiboot2 magic noto'g'ri: %x (GRUB orqali yuklang)", magic);
    if (mbi_phys & 7)
        panic("Multiboot2 ma'lumoti 8 ga tekislanmagan");

    /* Boot xaritasi 0..4 GB ni HHDM ga xaritalagan - shu orqali o'qiymiz. */
    const struct mb2_info *info = phys_to_virt(mbi_phys);
    boot_info.mbi_phys = mbi_phys;
    boot_info.mbi_size = info->total_size;

    const uint8_t *p = (const uint8_t *)info + 8;
    const uint8_t *end = (const uint8_t *)info + info->total_size;
    while (p + sizeof(struct mb2_tag) <= end) {
        const struct mb2_tag *tag = (const void *)p;
        if (tag->type == MB2_TAG_END)
            break;
        switch (tag->type) {
        case MB2_TAG_CMDLINE:
            strlcpy(boot_info.cmdline, ((const struct mb2_tag_string *)tag)->string,
                    sizeof(boot_info.cmdline));
            break;
        case MB2_TAG_BOOTLOADER:
            strlcpy(boot_info.bootloader, ((const struct mb2_tag_string *)tag)->string,
                    sizeof(boot_info.bootloader));
            break;
        case MB2_TAG_MODULE: {
            const struct mb2_tag_module *m = (const void *)tag;
            if (boot_info.module_count < BOOT_MAX_MODULES) {
                struct boot_module *bm = &boot_info.modules[boot_info.module_count++];
                bm->phys_start = m->mod_start;
                bm->phys_end = m->mod_end;
                strlcpy(bm->name, m->cmdline, sizeof(bm->name));
            }
            break;
        }
        case MB2_TAG_MMAP:
            parse_mmap((const void *)tag);
            break;
        case MB2_TAG_FRAMEBUFFER:
            parse_framebuffer((const void *)tag);
            break;
        case MB2_TAG_ACPI_OLD:
        case MB2_TAG_ACPI_NEW: {
            /* Yangisi (v2) bo'lsa, eskisining ustidan yozamiz. */
            if (tag->type == MB2_TAG_ACPI_OLD && boot_info.rsdp_present)
                break;
            const struct mb2_tag_acpi *a = (const void *)tag;
            size_t n = MIN(tag->size - 8, sizeof(boot_info.rsdp));
            memcpy(boot_info.rsdp, a->rsdp, n);
            boot_info.rsdp_present = true;
            break;
        }
        default:
            break;                      /* bizga kerak bo'lmagan teg */
        }
        p += ALIGN_UP(tag->size, 8);    /* keyingi teg 8 ga tekislangan */
    }

    if (boot_info.mmap_count == 0)
        panic("Yuklovchi xotira xaritasini bermadi");
}

void bootinfo_dump(void)
{
    kprintf("[boot] Yuklovchi: %s\n", boot_info.bootloader[0] ? boot_info.bootloader : "?");
    kprintf("[boot] Buyruq qatori: \"%s\"\n", boot_info.cmdline);
    kprintf("[boot] Xotira xaritasi:\n");
    for (size_t i = 0; i < boot_info.mmap_count; i++) {
        const struct mem_region *r = &boot_info.mmap[i];
        kprintf("         %016lx - %016lx  %s\n", r->base, r->base + r->len - 1,
                mem_type_name(r->type));
    }
    for (size_t i = 0; i < boot_info.module_count; i++)
        kprintf("[boot] Modul: %p - %p \"%s\"\n", (void *)boot_info.modules[i].phys_start,
                (void *)boot_info.modules[i].phys_end, boot_info.modules[i].name);
    const struct boot_framebuffer *fb = &boot_info.fb;
    if (fb->present && fb->text_mode)
        kprintf("[boot] Ekran: VGA matn rejimi\n");
    else if (fb->present)
        kprintf("[boot] Framebuffer: %ux%u, %u bpp, pitch %u, fizik %p\n", fb->width,
                fb->height, fb->bpp, fb->pitch, (void *)fb->phys);
    else
        kprintf("[boot] Ekran yo'q - faqat serial port\n");
    kprintf("[boot] ACPI RSDP: %s\n", boot_info.rsdp_present ? "bor" : "YO'Q");
}

bool cmdline_has(const char *word)
{
    size_t n = strlen(word);
    const char *s = boot_info.cmdline;
    /* Butun so'z sifatida qidiramiz: "test" so'zi "selftest" ichida topilmasin. */
    while ((s = strstr(s, word)) != NULL) {
        bool start_ok = (s == boot_info.cmdline || s[-1] == ' ');
        bool end_ok = (s[n] == '\0' || s[n] == ' ' || s[n] == '=');
        if (start_ok && end_ok)
            return true;
        s += n;
    }
    return false;
}

const char *cmdline_get(const char *key, char *buf, size_t size)
{
    size_t klen = strlen(key);
    const char *s = boot_info.cmdline;
    while ((s = strstr(s, key)) != NULL) {
        if ((s == boot_info.cmdline || s[-1] == ' ') && s[klen] == '=') {
            s += klen + 1;
            size_t n = 0;
            while (s[n] && s[n] != ' ' && n + 1 < size) {
                buf[n] = s[n];
                n++;
            }
            buf[n] = '\0';
            return buf;
        }
        s += klen;
    }
    return NULL;
}
