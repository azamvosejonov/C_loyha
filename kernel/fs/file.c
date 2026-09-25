/* =============================================================================
 *  fs/file.c - "hamma narsa - fayl" (Unix falsafasi)
 * =============================================================================
 *
 *  User dastur uchun klaviaturadan o'qish ham, initrd dagi faylni o'qish ham
 *  BIR XIL ko'rinadi: read(fd, buf, len). Farqni shu qatlam yashiradi.
 *
 *  FAYL DESKRIPTORI (fd) - jarayonning files[] massividagi INDEKS, xolos.
 *  files[fd] -> struct file (turi + joriy o'qish pozitsiyasi).
 *  Sukut bo'yicha: 0 = stdin, 1 = stdout, 2 = stderr - hammasi konsol.
 *
 *  Haqiqiy yadrolarda struct file ortida VFS (Virtual File System) turadi:
 *  har bir fayl tizimi (ext4, FAT, /proc...) o'z read/write funksiyalarini
 *  beradi (funksiya ko'rsatkichlari jadvali). Bu - mashq.
 * ============================================================================= */
#include "fs/file.h"

#include "drivers/console.h"
#include "lib/string.h"
#include "mm/slab.h"

static struct file console_file = { .type = FILE_CONSOLE };

struct file *file_console(void)
{
    return &console_file;
}

struct file *file_open(const char *path)
{
    const struct tar_file *tf = tarfs_find(path);
    if (!tf)
        return NULL;
    struct file *f = kzalloc(sizeof(*f));
    if (!f)
        return NULL;
    f->type = FILE_TAR;
    f->refcount = 1;
    f->tar = tf;
    f->offset = 0;
    return f;
}

void file_close(struct file *f)
{
    if (!f || f == &console_file)       /* konsol umumiy - bo'shatilmaydi */
        return;
    if (__atomic_sub_fetch(&f->refcount, 1, __ATOMIC_ACQ_REL) == 0)
        kfree(f);                       /* oxirgi foydalanuvchi */
}

struct file *file_dup(struct file *f)
{
    if (f && f != &console_file)
        __atomic_add_fetch(&f->refcount, 1, __ATOMIC_RELAXED);
    return f;
}

long file_read(struct file *f, void *buf, size_t len)
{
    if (len == 0)
        return 0;

    if (f->type == FILE_CONSOLE) {
        /* Kamida 1 belgini KUTAMIZ (jarayon uxlaydi), keyin tayyor turganlarini
         * kutmasdan olamiz. Shell har bir belgini alohida o'qib, o'zi echo qiladi. */
        char *out = buf;
        int c = console_getc();
        if (c < 0)
            return -1;                  /* kill() tufayli uzildi */
        out[0] = (char)c;
        size_t n = 1;
        while (n < len && console_input_available()) {
            c = console_getc();
            if (c < 0)
                break;
            out[n++] = (char)c;
        }
        return (long)n;
    }

    /* FILE_TAR: xotiradagi ma'lumotdan nusxalash. */
    size_t remaining = f->tar->size - f->offset;
    size_t n = len < remaining ? len : remaining;
    memcpy(buf, f->tar->data + f->offset, n);
    f->offset += n;
    return (long)n;                     /* 0 = fayl oxiri (EOF) */
}

long file_write(struct file *f, const void *buf, size_t len)
{
    if (f->type == FILE_CONSOLE) {
        console_write(buf, len);
        return (long)len;
    }
    return -1;                          /* initrd - faqat o'qish uchun */
}
