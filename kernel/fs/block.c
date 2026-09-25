/* =============================================================================
 *  fs/block.c - BLOK QATLAMI: disklar, bo'limlar (MBR/GPT), buffer cache
 * =============================================================================
 *
 *  DISK - sektorlar massivi (odatda 512 bayt). Drayver (ATA, AHCI, NVMe) faqat
 *  "N-sektorni o'qi/yoz" ni biladi. Bu qatlam:
 *    1. Barcha disklarni ro'yxatda saqlaydi (/dev/sda, /dev/hda ...)
 *    2. BO'LIMLARNI topadi: disk boshidagi MBR yoki GPT jadvalini o'qib,
 *       har bir bo'lim uchun "virtual disk" yaratadi (/dev/sda1 ...)
 *    3. BUFFER CACHE: fayl tizimi bir xil blokni (superblok, inode jadvali)
 *       qayta-qayta o'qiydi. Diskdan o'qish ~0.1-10 ms, RAM dan ~100 ns -
 *       100 000 marta farq! Oxirgi ishlatilgan bloklarni RAM da saqlaymiz (LRU).
 *
 *  YOZISH SIYOSATI: WRITE-THROUGH - bwrite() blokni DARHOL diskka yozadi.
 *  Sekinroq, lekin kompyuter to'satdan o'chsa ham fayl tizimi buzilmaydi.
 *  (Linux write-back ishlatadi + journal (ext4) - bu "mashq" darajasi.)
 * ============================================================================= */
#include "fs/block.h"

#include "fs/devfs.h"
#include "fs/vfs.h"
#include "lib/common.h"
#include "lib/kprintf.h"
#include "lib/string.h"
#include "mm/slab.h"

static LIST_HEAD(disks);
static struct mutex disks_lock = MUTEX_INIT("blkdevs");

/* ---- Sektor darajasida o'qish/yozish ---- */

static int blk_rw(struct block_device *bd, uint64_t lba, uint32_t count, void *buf, bool write)
{
    if (count == 0)
        return 0;
    if (lba + count < lba || lba + count > bd->sectors)
        return -EINVAL;                 /* disk (bo'lim) chegarasidan tashqari */
    while (bd->parent) {                /* bo'lim -> butun disk */
        lba += bd->start;
        bd = bd->parent;
    }
    mutex_lock(&bd->lock);
    int r = write ? bd->ops->write(bd, lba, count, buf) : bd->ops->read(bd, lba, count, buf);
    mutex_unlock(&bd->lock);
    return r ? -EIO : 0;
}

int blk_read(struct block_device *bd, uint64_t lba, uint32_t count, void *buf)
{
    return blk_rw(bd, lba, count, buf, false);
}

int blk_write(struct block_device *bd, uint64_t lba, uint32_t count, const void *buf)
{
    return blk_rw(bd, lba, count, (void *)buf, true);
}

/* ---- Bo'limlar ---- */

struct mbr_entry {
    uint8_t status;
    uint8_t chs_first[3];
    uint8_t type;
    uint8_t chs_last[3];
    uint32_t lba_start;
    uint32_t sectors;
} __attribute__((packed));

struct gpt_header {
    char signature[8];                  /* "EFI PART" */
    uint32_t revision, header_size, crc32, reserved;
    uint64_t current_lba, backup_lba, first_usable, last_usable;
    uint8_t disk_guid[16];
    uint64_t entries_lba;
    uint32_t num_entries, entry_size, entries_crc32;
} __attribute__((packed));

struct gpt_entry {
    uint8_t type_guid[16];
    uint8_t part_guid[16];
    uint64_t first_lba, last_lba;
    uint64_t attrs;
    uint16_t name[36];
} __attribute__((packed));

static void add_partition(struct block_device *disk, int num, uint64_t start, uint64_t sectors)
{
    if (start >= disk->sectors || sectors == 0 || start + sectors > disk->sectors)
        return;
    struct block_device *p = kzalloc(sizeof(*p));
    if (!p)
        return;
    /* "sda" -> "sda1", lekin "nvme0n1" -> "nvme0n1p1" (raqam bilan tugasa). */
    char last = disk->name[strlen(disk->name) - 1];
    ksnprintf(p->name, sizeof(p->name), (last >= '0' && last <= '9') ? "%sp%d" : "%s%d",
              disk->name, num);
    p->sectors = sectors;
    p->sector_size = disk->sector_size;
    p->parent = disk;
    p->start = start;
    p->ops = disk->ops;
    p->dev = disk->dev + (uint32_t)num;
    mutex_init(&p->lock, "blkpart");
    mutex_lock(&disks_lock);
    list_add_tail(&p->node, &disks);
    mutex_unlock(&disks_lock);
    devfs_add_block(p);
    kprintf("[blk]  %s: bo'lim, sektor %lu, %lu MB\n", p->name, start, sectors / 2048);
}

static void scan_gpt(struct block_device *disk, uint8_t *sec)
{
    if (blk_read(disk, 1, 1, sec))
        return;
    struct gpt_header h;
    memcpy(&h, sec, sizeof(h));
    if (memcmp(h.signature, "EFI PART", 8) != 0 || h.entry_size < sizeof(struct gpt_entry) ||
        h.num_entries > 256)
        return;
    int num = 1;
    uint32_t per_sector = 512 / h.entry_size;
    for (uint32_t i = 0; i < h.num_entries; i++) {
        if (i % per_sector == 0 && blk_read(disk, h.entries_lba + i / per_sector, 1, sec))
            return;
        const struct gpt_entry *e = (const void *)(sec + (i % per_sector) * h.entry_size);
        bool empty = true;
        for (int b = 0; b < 16; b++)
            empty &= e->type_guid[b] == 0;
        if (!empty && e->last_lba >= e->first_lba)
            add_partition(disk, num, e->first_lba, e->last_lba - e->first_lba + 1);
        num++;
    }
}

static void scan_partitions(struct block_device *disk)
{
    uint8_t *sec = kmalloc(512);
    if (!sec)
        return;
    if (blk_read(disk, 0, 1, sec) == 0 && sec[510] == 0x55 && sec[511] == 0xAA) {
        const struct mbr_entry *e = (const void *)(sec + 446);
        if (e[0].type == 0xEE) {        /* "himoya MBR" - aslida GPT disk */
            scan_gpt(disk, sec);
        } else {
            struct mbr_entry copy[4];
            memcpy(copy, e, sizeof(copy));
            for (int i = 0; i < 4; i++)
                if (copy[i].type != 0 && copy[i].sectors)
                    add_partition(disk, i + 1, copy[i].lba_start, copy[i].sectors);
        }
    }
    kfree(sec);
}

void blkdev_assign_sd(struct block_device *bd)
{
    static int next;                    /* sda, sdb, ... */
    int idx = __atomic_fetch_add(&next, 1, __ATOMIC_RELAXED);
    ksnprintf(bd->name, sizeof(bd->name), "sd%c", 'a' + idx % 26);
    bd->dev = MKDEV(MAJOR_SD, idx * 16);    /* sda1 = 8:1, sdb = 8:16 ... */
}

int blkdev_register(struct block_device *bd)
{
    mutex_init(&bd->lock, "blkdev");
    if (!bd->sector_size)
        bd->sector_size = 512;
    mutex_lock(&disks_lock);
    list_add_tail(&bd->node, &disks);
    mutex_unlock(&disks_lock);
    kprintf("[blk]  %s: %lu MB \"%s\"\n", bd->name, bd->sectors / 2048, bd->model);
    devfs_add_block(bd);                /* /dev/sda (devfs hali tayyor bo'lmasa - keyinroq) */
    scan_partitions(bd);
    return 0;
}

struct block_device *blkdev_find(const char *name)
{
    struct block_device *bd;
    list_for_each_entry(bd, &disks, node)
        if (strcmp(bd->name, name) == 0)
            return bd;
    return NULL;
}

struct block_device *blkdev_get(int index)
{
    struct block_device *bd;
    int i = 0;
    list_for_each_entry(bd, &disks, node)
        if (i++ == index)
            return bd;
    return NULL;
}

/* ---- /dev/sdX fayli: bayt darajasida o'qish/yozish ---- */

static struct block_device *file_bdev(struct file *f)
{
    struct block_device *bd;
    list_for_each_entry(bd, &disks, node)
        if (bd->dev == f->inode->rdev)
            return bd;
    return NULL;
}

static int64_t bdev_rw(struct file *f, void *buf, size_t len, uint64_t off, bool write)
{
    struct block_device *bd = file_bdev(f);
    if (!bd)
        return -ENXIO;
    uint64_t total = bd->sectors * 512;
    if (off >= total)
        return 0;
    len = MIN(len, total - off);
    uint8_t *sec = kmalloc(512);
    if (!sec)
        return -ENOMEM;
    size_t done = 0;
    int err = 0;
    while (done < len && !err) {
        uint64_t pos = off + done;
        uint64_t lba = pos / 512;
        size_t in = pos % 512, chunk = MIN(len - done, 512 - in);
        if (write && chunk != 512)      /* qisman sektor: o'qish-o'zgartirish-yozish */
            err = blk_read(bd, lba, 1, sec);
        if (!err && !write)
            err = blk_read(bd, lba, 1, sec);
        if (err)
            break;
        if (write) {
            memcpy(sec + in, (const uint8_t *)buf + done, chunk);
            err = blk_write(bd, lba, 1, sec);
        } else {
            memcpy((uint8_t *)buf + done, sec + in, chunk);
        }
        if (!err)
            done += chunk;
    }
    kfree(sec);
    return done ? (int64_t)done : err;
}

static int64_t bdev_read(struct file *f, void *buf, size_t len, uint64_t off)
{
    return bdev_rw(f, buf, len, off, false);
}

static int64_t bdev_write(struct file *f, const void *buf, size_t len, uint64_t off)
{
    return bdev_rw(f, (void *)buf, len, off, true);
}

static const struct file_ops bdev_fops = {
    .read = bdev_read,
    .write = bdev_write,
    .seekable = true,
};

const struct file_ops *blkdev_file_ops(void)
{
    return &bdev_fops;
}

/* ---- Buffer cache ---- */

#define BCACHE_MAX 1024                 /* ko'pi bilan shuncha blok keshda */

static LIST_HEAD(lru);                  /* boshida - eng yaqinda ishlatilgan */
static int bcache_count;
static spinlock_t bcache_lock = SPINLOCK_INIT("bcache");

void bcache_init(void)
{
    kprintf("[blk]  buffer cache: %d blokgacha, write-through\n", BCACHE_MAX);
}

/* Keshdan topish yoki yangi/qayta ishlatiladigan buf olish (qulfsiz holatda). */
static struct buf *bget(struct block_device *bd, uint64_t block, uint32_t size)
{
    spin_lock(&bcache_lock);
    struct buf *b;
    list_for_each_entry(b, &lru, node) {
        if (b->bd == bd && b->block == block && b->size == size) {
            b->refcount++;
            list_del(&b->node);         /* LRU boshiga */
            list_add(&b->node, &lru);
            spin_unlock(&bcache_lock);
            return b;
        }
    }
    /* Yangi: limitga yetmagan bo'lsak - ajratamiz, aks holda eng eski bo'shini. */
    struct buf *victim = NULL;
    if (bcache_count >= BCACHE_MAX) {
        list_for_each_entry(b, &lru, node)
            if (b->refcount == 0)
                victim = b;             /* oxirgisi = eng eski */
    }
    if (victim && victim->size == size) {
        victim->bd = bd;
        victim->block = block;
        victim->valid = false;
        victim->refcount = 1;
        list_del(&victim->node);
        list_add(&victim->node, &lru);
        spin_unlock(&bcache_lock);
        return victim;
    }
    if (victim) {                       /* hajmi boshqa - bo'shatib, yangisini olamiz */
        list_del(&victim->node);
        bcache_count--;
    }
    spin_unlock(&bcache_lock);
    if (victim) {
        kfree(victim->data);
        kfree(victim);
    }
    b = kzalloc(sizeof(*b));
    uint8_t *data = b ? kmalloc(size) : NULL;
    if (!b || !data) {
        kfree(b);
        return NULL;
    }
    b->bd = bd;
    b->block = block;
    b->size = size;
    b->data = data;
    b->refcount = 1;
    mutex_init(&b->lock, "buf");
    spin_lock(&bcache_lock);
    /* Boshqa jarayon shu vaqtda xuddi shu blokni qo'shgan bo'lishi mumkin. */
    struct buf *dup;
    list_for_each_entry(dup, &lru, node) {
        if (dup->bd == bd && dup->block == block && dup->size == size) {
            dup->refcount++;
            spin_unlock(&bcache_lock);
            kfree(data);
            kfree(b);
            return dup;
        }
    }
    list_add(&b->node, &lru);
    bcache_count++;
    spin_unlock(&bcache_lock);
    return b;
}

struct buf *bread(struct block_device *bd, uint64_t block, uint32_t size)
{
    struct buf *b = bget(bd, block, size);
    if (!b)
        return NULL;
    mutex_lock(&b->lock);
    if (!b->valid) {                    /* keshda yo'q (yoki eskirgan) - diskdan */
        uint32_t spb = size / 512;      /* blokdagi sektorlar */
        if (blk_read(bd, block * spb, spb, b->data) != 0) {
            brelse(b);
            return NULL;
        }
        b->valid = true;
    }
    return b;
}

int bwrite(struct buf *b)
{
    uint32_t spb = b->size / 512;
    return blk_write(b->bd, b->block * spb, spb, b->data);
}

void brelse(struct buf *b)
{
    if (!b)
        return;
    mutex_unlock(&b->lock);
    spin_lock(&bcache_lock);
    b->refcount--;
    spin_unlock(&bcache_lock);
}

void bcache_invalidate(struct block_device *bd)
{
    spin_lock(&bcache_lock);
    struct buf *b;
    list_for_each_entry(b, &lru, node)
        if (b->bd == bd && b->refcount == 0)
            b->valid = false;
    spin_unlock(&bcache_lock);
}
