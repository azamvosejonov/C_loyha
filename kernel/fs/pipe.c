/* =============================================================================
 *  fs/pipe.c - PIPE: jarayonlar orasidagi "quvur"
 * =============================================================================
 *
 *  `ls | grep txt` - shell ikkita jarayon yaratadi va birinchisining stdout ini
 *  ikkinchisining stdin iga ulaydi. Ulagich - pipe: yadro ichidagi halqali
 *  bufer (16 KB) va ikkita fayl: o'qish uchi va yozish uchi.
 *
 *  QOIDALAR (POSIX):
 *    * Bufer bo'sh bo'lsa, o'quvchi UXLAYDI (yozuvchi yozguncha).
 *    * Bufer to'la bo'lsa, yozuvchi UXLAYDI (o'quvchi o'qiguncha) - shu tufayli
 *      tez ishlab chiqaruvchi sekin iste'molchini "bosib" ketmaydi (backpressure).
 *    * Barcha yozuvchilar yopilsa va bufer bo'sh - o'quvchi 0 oladi (EOF).
 *      Shuning uchun shell ORTIQCHA yozish uchlarini yopishi SHART, aks holda
 *      `grep` hech qachon EOF ko'rmaydi va abadiy kutadi (klassik xato!).
 *    * O'quvchilar yo'q bo'lsa, yozuvchiga SIGPIPE yuboriladi va yozish -EPIPE qaytaradi.
 * ============================================================================= */
#include "fs/pipe.h"

#include "lib/common.h"
#include "lib/spinlock.h"
#include "lib/string.h"
#include "mm/slab.h"
#include "proc/process.h"
#include "proc/signal.h"

#define PIPE_SIZE 16384

struct pipe {
    spinlock_t lock;
    uint8_t *buf;
    size_t head;                        /* keyingi o'qiladigan joy */
    size_t count;                       /* buferdagi baytlar */
    int readers, writers;               /* ochiq uchlar */
};

static int64_t pipe_read(struct file *f, void *dst, size_t len, uint64_t off)
{
    (void)off;
    struct pipe *p = f->priv;
    if (len == 0)
        return 0;
    spin_lock(&p->lock);
    while (p->count == 0) {
        if (p->writers == 0) {          /* hech kim yozmaydi - EOF */
            spin_unlock(&p->lock);
            return 0;
        }
        if (signal_interrupted(current)) {
            spin_unlock(&p->lock);
            return -EINTR;
        }
        proc_sleep(&p->count, &p->lock);
    }
    size_t n = MIN(len, p->count);
    for (size_t i = 0; i < n; i++)
        ((uint8_t *)dst)[i] = p->buf[(p->head + i) % PIPE_SIZE];
    p->head = (p->head + n) % PIPE_SIZE;
    p->count -= n;
    proc_wakeup(&p->writers);           /* joy bo'shadi - yozuvchilarni uyg'otamiz */
    spin_unlock(&p->lock);
    return (int64_t)n;
}

static int64_t pipe_write(struct file *f, const void *src, size_t len, uint64_t off)
{
    (void)off;
    struct pipe *p = f->priv;
    size_t done = 0;
    spin_lock(&p->lock);
    while (done < len) {
        if (p->readers == 0) {
            spin_unlock(&p->lock);
            if (done)
                return (int64_t)done;
            /* O'quvchi yo'q: SIGPIPE (standart amal - tugash). Shu tufayli
             * `seq 1000000 | head -1` da seq head tugashi bilan to'xtaydi. */
            spin_lock(&proc_lock);
            signal_send_locked(current, SIGPIPE);
            spin_unlock(&proc_lock);
            return -EPIPE;
        }
        if (p->count == PIPE_SIZE) {
            if (signal_interrupted(current)) {
                spin_unlock(&p->lock);
                return done ? (int64_t)done : -EINTR;
            }
            proc_wakeup(&p->count);
            proc_sleep(&p->writers, &p->lock);
            continue;
        }
        size_t n = MIN(len - done, PIPE_SIZE - p->count);
        size_t tail = (p->head + p->count) % PIPE_SIZE;
        for (size_t i = 0; i < n; i++)
            p->buf[(tail + i) % PIPE_SIZE] = ((const uint8_t *)src)[done + i];
        p->count += n;
        done += n;
        proc_wakeup(&p->count);         /* ma'lumot bor - o'quvchilarni uyg'otamiz */
    }
    spin_unlock(&p->lock);
    return (int64_t)done;
}

static void pipe_release(struct file *f)
{
    struct pipe *p = f->priv;
    spin_lock(&p->lock);
    if ((f->flags & O_ACCMODE) == O_WRONLY)
        p->writers--;
    else
        p->readers--;
    proc_wakeup(&p->count);             /* EOF yoki EPIPE ni ko'rsinlar */
    proc_wakeup(&p->writers);
    bool dead = p->readers == 0 && p->writers == 0;
    spin_unlock(&p->lock);
    if (dead) {
        kfree(p->buf);
        kfree(p);
    }
}

static const struct file_ops pipe_fops = {
    .read = pipe_read,
    .write = pipe_write,
    .release = pipe_release,
    .seekable = false,
};

int pipe_create(struct file **rf, struct file **wf)
{
    struct pipe *p = kzalloc(sizeof(*p));
    uint8_t *buf = kmalloc(PIPE_SIZE);
    struct inode *in = inode_alloc(NULL);   /* hech qaysi fayl tizimiga tegishli emas */
    struct file *r = NULL, *w = NULL;
    if (p && buf && in) {
        in->mode = S_IFIFO | 0600;
        in->nlink = 0;
        r = file_alloc(in, O_RDONLY);
        w = file_alloc(iget(in), O_WRONLY);
    }
    if (!r || !w) {
        if (r)
            file_close(r);
        else if (in)
            iput(in);
        if (w)
            file_close(w);
        kfree(buf);
        kfree(p);
        return -ENOMEM;
    }
    p->lock = (spinlock_t)SPINLOCK_INIT("pipe");
    p->buf = buf;
    p->readers = p->writers = 1;
    r->fops = w->fops = &pipe_fops;
    r->priv = w->priv = p;
    *rf = r;
    *wf = w;
    return 0;
}
