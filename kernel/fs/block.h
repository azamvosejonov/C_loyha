/* =============================================================================
 *  fs/block.h - blokli qurilmalar (disklar) va buffer cache
 * ============================================================================= */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "lib/list.h"
#include "lib/mutex.h"

struct block_device;
struct file_ops;

struct block_ops {
    /* lba dan count ta sektor. buf - yadro xotirasi. 0 = muvaffaqiyat. */
    int (*read)(struct block_device *bd, uint64_t lba, uint32_t count, void *buf);
    int (*write)(struct block_device *bd, uint64_t lba, uint32_t count, const void *buf);
};

struct block_device {
    char name[32];                      /* "sda", "sda1", "hda" ... */
    uint64_t sectors;                   /* 512 baytli sektorlar soni */
    uint32_t sector_size;               /* odatda 512 */
    const struct block_ops *ops;
    void *priv;                         /* drayver ma'lumoti */
    struct block_device *parent;        /* bo'lim (partition) bo'lsa - butun disk */
    uint64_t start;                     /* bo'lim boshlanishi (parent sektorlarida) */
    uint32_t dev;                       /* MKDEV(major, minor) */
    char model[41];                     /* disk modeli (IDENTIFY dan) */
    struct mutex lock;                  /* drayverga so'rovlar ketma-ket */
    struct list_head node;
};

/* Keyingi "sdX" nomini va qurilma raqamini (major 8, minor = 16 * indeks) berish.
 * ATA, AHCI va USB disklar hammasi shu nomlarni oladi (Linux libata kabi). */
void blkdev_assign_sd(struct block_device *bd);
/* Drayver diskni ro'yxatdan o'tkazadi; bo'limlar avtomatik skanerlanadi. */
int blkdev_register(struct block_device *bd);
struct block_device *blkdev_find(const char *name);
struct block_device *blkdev_get(int index);

/* Sektorlar bo'yicha o'qish/yozish (bo'lim siljishi va chegaralar tekshiriladi). */
int blk_read(struct block_device *bd, uint64_t lba, uint32_t count, void *buf);
int blk_write(struct block_device *bd, uint64_t lba, uint32_t count, const void *buf);

/* /dev/sdX kabi fayllar uchun: bayt siljishlari bilan o'qish/yozish. */
const struct file_ops *blkdev_file_ops(void);

/* ---- Buffer cache ---- */
struct buf {
    struct block_device *bd;
    uint64_t block;                     /* blok raqami (size birligida) */
    uint32_t size;                      /* blok hajmi (1024/2048/4096) */
    uint8_t *data;
    bool valid;                         /* data diskdagiga mos */
    int refcount;
    struct mutex lock;                  /* bitta jarayon ishlaydi */
    struct list_head node;              /* LRU ro'yxati */
};

void bcache_init(void);
/* Blokni o'qish (keshdan yoki diskdan). Qaytaradi: QULFLANGAN buf yoki NULL. */
struct buf *bread(struct block_device *bd, uint64_t block, uint32_t size);
/* O'zgartirilgan blokni diskka yozish (write-through). */
int bwrite(struct buf *b);
/* bread dan olingan buf ni qaytarish. */
void brelse(struct buf *b);
/* Qurilmaning keshdagi bloklarini bekor qilish (umount). */
void bcache_invalidate(struct block_device *bd);
