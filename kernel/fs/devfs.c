/* =============================================================================
 *  fs/devfs.c - "HAMMA NARSA FAYL": qurilmalar /dev ichida
 * =============================================================================
 *
 *  Unix'ning eng chiroyli g'oyalaridan biri: qurilma - bu FAYL. Dastur
 *  terminaldan o'qish uchun ham, diskni to'g'ridan-to'g'ri o'qish uchun ham,
 *  tasodifiy sonlar olish uchun ham oddiy open/read/write ishlatadi:
 *
 *     /dev/null     - "qora tuynuk": yozilgan hamma narsa yo'qoladi, o'qish = EOF
 *     /dev/zero     - cheksiz nollar
 *     /dev/random   - tasodifiy baytlar
 *     /dev/console  - klaviatura + ekran (terminal, drivers/tty.c)
 *     /dev/kmsg     - yadro logi (dmesg)
 *     /dev/sda, /dev/sda1 ... - disklar va bo'limlar (bayt darajasida)
 *
 *  Qurilma fayli - maxsus inode: turi S_IFCHR/S_IFBLK va qurilma raqami
 *  (major:minor). open() paytida VFS major bo'yicha drayverning file_ops
 *  jadvalini oladi (chrdev_get). Linux ham aynan shunday qiladi (mknod).
 * ============================================================================= */
#include "fs/devfs.h"

#include "arch/cpu.h"
#include "drivers/console.h"
#include "drivers/tty.h"
#include "fs/vfs.h"
#include "lib/common.h"
#include "lib/kprintf.h"
#include "lib/string.h"
#include "proc/process.h"

#define MINOR_NULL    3
#define MINOR_ZERO    5
#define MINOR_RANDOM  8
#define MINOR_URANDOM 9
#define MINOR_KMSG    11

static bool devfs_ready;

/* ---- /dev/null, zero, random, kmsg ---- */

static uint64_t rng_state;

/* xorshift64* - tez psevdo-tasodifiy generator. Urug' - TSC (har boot'da boshqa).
 * DIQQAT: kriptografik jihatdan xavfsiz EMAS (parol/kalit uchun yaramaydi). */
static uint64_t rng_next(void)
{
    if (!rng_state)
        rng_state = rdtsc() | 1;
    rng_state ^= rng_state >> 12;
    rng_state ^= rng_state << 25;
    rng_state ^= rng_state >> 27;
    return rng_state * 0x2545F4914F6CDD1DULL;
}

static int64_t mem_read(struct file *f, void *buf, size_t len, uint64_t off)
{
    (void)off;
    switch (MINOR(f->inode->rdev)) {
    case MINOR_NULL:
        return 0;                       /* doim fayl oxiri */
    case MINOR_ZERO:
        memset(buf, 0, len);
        return (int64_t)len;
    case MINOR_RANDOM:
    case MINOR_URANDOM:
        for (size_t i = 0; i < len; i++) {
            if (i % 8 == 0)
                rng_state ^= rdtsc();   /* ozgina qo'shimcha "entropiya" */
            ((uint8_t *)buf)[i] = (uint8_t)(rng_next() >> 56);
        }
        return (int64_t)len;
    case MINOR_KMSG:
        return off ? 0 : (int64_t)console_read_log(buf, len);
    default:
        return -ENXIO;
    }
}

static int64_t mem_write(struct file *f, const void *buf, size_t len, uint64_t off)
{
    (void)off;
    if (MINOR(f->inode->rdev) == MINOR_KMSG)
        console_write(buf, len);        /* dastur yadro logiga yozishi mumkin */
    return (int64_t)len;                /* null/zero/random: yutib yuboradi */
}

static const struct file_ops mem_fops = {
    .read = mem_read,
    .write = mem_write,
    .seekable = true,
};

void devfs_add_block(struct block_device *bd)
{
    if (!devfs_ready)
        return;                         /* devfs_init o'zi hammasini qo'shadi */
    char path[64];
    ksnprintf(path, sizeof(path), "/dev/%s", bd->name);
    vfs_mknod(path, NULL, S_IFBLK | 0660, bd->dev);
}

void devfs_init(void)
{
    chrdev_register(MAJOR_MEM, &mem_fops, "mem (null, zero, random, kmsg)");
    chrdev_register(MAJOR_TTY, &tty_fops, "tty");

    vfs_mkdir("/dev", NULL, 0755);
    vfs_mknod("/dev/null", NULL, S_IFCHR | 0666, MKDEV(MAJOR_MEM, MINOR_NULL));
    vfs_mknod("/dev/zero", NULL, S_IFCHR | 0666, MKDEV(MAJOR_MEM, MINOR_ZERO));
    vfs_mknod("/dev/random", NULL, S_IFCHR | 0666, MKDEV(MAJOR_MEM, MINOR_RANDOM));
    vfs_mknod("/dev/urandom", NULL, S_IFCHR | 0666, MKDEV(MAJOR_MEM, MINOR_URANDOM));
    vfs_mknod("/dev/kmsg", NULL, S_IFCHR | 0644, MKDEV(MAJOR_MEM, MINOR_KMSG));
    vfs_mknod("/dev/console", NULL, S_IFCHR | 0600, MKDEV(MAJOR_TTY, 1));

    devfs_ready = true;
    struct block_device *bd;
    for (int i = 0; (bd = blkdev_get(i)) != NULL; i++)
        devfs_add_block(bd);
}
