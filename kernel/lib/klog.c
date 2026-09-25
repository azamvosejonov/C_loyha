/* =============================================================================
 *  lib/klog.c - yadro logi
 * =============================================================================
 *
 *  Yadro ishga tushishining ilk lahzalarida ekran hali yo'q: framebuffer'ni
 *  xaritalash uchun virtual xotira kerak, virtual xotira uchun esa fizik xotira
 *  menejeri... Lekin log yo'qolmasligi kerak! Shuning uchun HAMMA chiqish
 *  avval shu 64 KB lik halqali buferga yoziladi. Ekran tayyor bo'lgach,
 *  klog_replay() butun tarixni unga "qayta o'ynaydi". Linux'dagi `dmesg`
 *  buyrug'i aynan shunday bufer (printk ring buffer) ni o'qiydi.
 * ============================================================================= */
#include "lib/klog.h"

#include <stdint.h>


#define KLOG_SIZE (64 * 1024)           /* 2 ning darajasi */

static char klog_buf[KLOG_SIZE];
static uint64_t klog_head;              /* jami yozilgan belgilar (hech qachon kamaymaydi) */

void klog_putc(char c)
{
    klog_buf[klog_head & (KLOG_SIZE - 1)] = c;
    klog_head++;
}

void klog_replay(void (*fn)(char c))
{
    uint64_t start = klog_head > KLOG_SIZE ? klog_head - KLOG_SIZE : 0;
    for (uint64_t i = start; i < klog_head; i++)
        fn(klog_buf[i & (KLOG_SIZE - 1)]);
}

/* Chaqiruvchi console_lock ni ushlab turadi (console_read_log). */
size_t klog_read(char *buf, size_t size)
{
    uint64_t start = klog_head > KLOG_SIZE ? klog_head - KLOG_SIZE : 0;
    if (klog_head - start > size)
        start = klog_head - size;       /* eng oxirgi qismini beramiz */
    size_t n = 0;
    for (uint64_t i = start; i < klog_head; i++)
        buf[n++] = klog_buf[i & (KLOG_SIZE - 1)];
    return n;
}
