/* =============================================================================
 *  fs/ext2.c - EXT2: diskdagi haqiqiy fayl tizimi (o'qish va yozish)
 * =============================================================================
 *
 *  ext2 - 1993 yilda Linux uchun yaratilgan. ext3 = ext2 + jurnal, ext4 esa
 *  ext3 ning kengaytmasi (extentlar). Asosiy g'oyalar hamon shu yerda va
 *  ko'plab boshqa FS'lar (UFS, FFS) ham shunga o'xshaydi.
 *
 *  DISK TUZILISHI (1 KB bloklarda):
 *
 *   blok 0      1           2..         keyin        keyin       keyin
 *   ┌──────┬──────────┬─────────────┬────────────┬────────────┬──────────────┬──────────┐
 *   │ boot │ SUPERBLOK│ guruh       │ blok       │ inode      │ inode        │ ma'lumot │ ...
 *   │      │ (1 KB)   │ deskriptorl.│ bitmap     │ bitmap     │ jadvali      │ bloklari │
 *   └──────┴──────────┴─────────────┴────────────┴────────────┴──────────────┴──────────┘
 *   └─────────────────────── 0-BLOK GURUHI ─────────────────────────────────────────┘ 1-guruh ...
 *
 *  Disk BLOK GURUHLARIGA bo'lingan (1 KB blokda har biri 8 MB). Har bir
 *  guruhning o'z bitmap'lari va inode jadvali bor - fayl ma'lumotlari o'z
 *  inode'iga YAQIN joylashadi (disk kallagi kamroq yuradi - HDD uchun muhim).
 *
 *    superblok      - butun FS haqida: blok hajmi, nechta inode/blok, bo'sh joy
 *    blok bitmap    - har bir bit = bitta blok (1 = band)
 *    inode bitmap   - har bir bit = bitta inode
 *    inode jadvali  - inode'lar massivi (har biri 128 yoki 256 bayt)
 *
 *  INODE -> MA'LUMOT: i_block[15] massivi
 *
 *    i_block[0..11]  ──► ma'lumot bloki           (12 KB gacha - to'g'ridan-to'g'ri)
 *    i_block[12]     ──► [256 ko'rsatkich] ──► ma'lumot       (+256 KB, "bilvosita")
 *    i_block[13]     ──► [256] ──► [256] ──► ma'lumot         (+64 MB, "ikki karra")
 *    i_block[14]     ──► [256] ──► [256] ──► [256] ──► ma'lumot (+16 GB, "uch karra")
 *
 *  Kichik fayllar tez (bitta o'qish), katta fayllar ham mumkin. 0 = "teshik".
 *
 *  PAPKA - oddiy fayl, ichida yozuvlar ro'yxati:
 *     [inode 4][rec_len 2][name_len 1][tur 1][nom ...]  -> keyingi yozuv rec_len dan keyin
 *  O'chirilgan yozuv oldingisining rec_len iga "yutiladi".
 *
 *  YOZISH SIYOSATI: har bir o'zgarish (bitmap, inode, papka bloki) buffer
 *  cache orqali DARHOL diskka yoziladi (write-through). Jurnal yo'q, shuning
 *  uchun ikki yozish ORASIDA tok o'chsa, nomuvofiqlik qolishi mumkin (masalan,
 *  blok band, lekin hech kimga tegishli emas) - e2fsck buni tuzatadi.
 *  Tartib shunday tanlangan: avval resurs AJRATILADI, keyin ishlatiladi,
 *  keyin havola qilinadi - eng yomon holatda "yo'qolgan" joy qoladi, lekin
 *  ikki fayl bitta blokni ulashmaydi.
 * ============================================================================= */
#include "fs/ext2.h"

#include "drivers/rtc.h"
#include "fs/block.h"
#include "fs/vfs.h"
#include "lib/common.h"
#include "lib/kprintf.h"
#include "lib/spinlock.h"
#include "lib/string.h"
#include "mm/slab.h"

#define EXT2_MAGIC       0xEF53
#define EXT2_ROOT_INO    2
#define EXT2_NDIR_BLOCKS 12
#define EXT2_IND_BLOCK   12
#define EXT2_DIND_BLOCK  13
#define EXT2_TIND_BLOCK  14

/* Xususiyat (feature) bayroqlari. INCOMPAT - tushunmasak, umuman ulab bo'lmaydi;
 * RO_COMPAT - tushunmasak, faqat o'qish uchun ulash mumkin; COMPAT - e'tiborsiz. */
#define INCOMPAT_FILETYPE     0x0002    /* papka yozuvida fayl turi bor */
#define RO_COMPAT_SPARSE      0x0001    /* superblok nusxalari faqat ba'zi guruhlarda */
#define RO_COMPAT_LARGE_FILE  0x0002    /* 2 GB dan katta fayllar (i_size_high) */

/* Papka yozuvidagi tur */
#define FT_UNKNOWN 0
#define FT_REG     1
#define FT_DIR     2
#define FT_CHR     3
#define FT_BLK     4
#define FT_FIFO    5
#define FT_SOCK    6
#define FT_SYMLINK 7

/* ---- Diskdagi tuzilmalar (little-endian, x86 bilan bir xil) ---- */

struct ext2_super {
    uint32_t s_inodes_count;
    uint32_t s_blocks_count;
    uint32_t s_r_blocks_count;          /* root uchun zaxira */
    uint32_t s_free_blocks_count;
    uint32_t s_free_inodes_count;
    uint32_t s_first_data_block;        /* 1 KB blokda 1, aks holda 0 */
    uint32_t s_log_block_size;          /* blok hajmi = 1024 << shu */
    uint32_t s_log_frag_size;
    uint32_t s_blocks_per_group;
    uint32_t s_frags_per_group;
    uint32_t s_inodes_per_group;
    uint32_t s_mtime, s_wtime;          /* oxirgi mount / yozish vaqti */
    uint16_t s_mnt_count;
    int16_t s_max_mnt_count;
    uint16_t s_magic;                   /* 0xEF53 */
    uint16_t s_state;                   /* 1 = toza */
    uint16_t s_errors;
    uint16_t s_minor_rev_level;
    uint32_t s_lastcheck, s_checkinterval;
    uint32_t s_creator_os;
    uint32_t s_rev_level;               /* 0 = eski (128 baytli inode), 1 = dinamik */
    uint16_t s_def_resuid, s_def_resgid;
    /* rev 1 */
    uint32_t s_first_ino;               /* birinchi oddiy inode (11); oldingilari zaxira */
    uint16_t s_inode_size;
    uint16_t s_block_group_nr;
    uint32_t s_feature_compat;
    uint32_t s_feature_incompat;
    uint32_t s_feature_ro_compat;
    uint8_t s_uuid[16];
    char s_volume_name[16];
    char s_last_mounted[64];
    uint8_t rest[1024 - 200];
} __attribute__((packed));

struct ext2_group_desc {
    uint32_t bg_block_bitmap;
    uint32_t bg_inode_bitmap;
    uint32_t bg_inode_table;
    uint16_t bg_free_blocks_count;
    uint16_t bg_free_inodes_count;
    uint16_t bg_used_dirs_count;
    uint16_t bg_pad;
    uint8_t bg_reserved[12];
} __attribute__((packed));

struct ext2_inode {
    uint16_t i_mode;
    uint16_t i_uid;
    uint32_t i_size;
    uint32_t i_atime, i_ctime, i_mtime, i_dtime;    /* dtime - o'chirilgan vaqt */
    uint16_t i_gid;
    uint16_t i_links_count;
    uint32_t i_blocks;                  /* 512 BAYTLI birliklarda (bilvosita bloklar ham) */
    uint32_t i_flags;
    uint32_t i_osd1;
    uint32_t i_block[15];
    uint32_t i_generation;
    uint32_t i_file_acl;
    uint32_t i_size_high;               /* oddiy fayl: hajmning yuqori 32 biti */
    uint32_t i_faddr;
    uint8_t i_osd2[12];
};                                      /* maydonlar tabiiy tekislangan - packed shart emas */

struct ext2_dirent {
    uint32_t inode;                     /* 0 = bo'sh yozuv */
    uint16_t rec_len;                   /* keyingi yozuvgacha masofa */
    uint8_t name_len;
    uint8_t file_type;
    char name[];                        /* '\0' bilan TUGAMAYDI */
} __attribute__((packed));

_Static_assert(sizeof(struct ext2_super) == 1024, "superblok 1024 bayt");
_Static_assert(sizeof(struct ext2_group_desc) == 32, "guruh deskriptori 32 bayt");
_Static_assert(sizeof(struct ext2_inode) == 128, "inode 128 bayt");

/* ---- Xotiradagi tuzilmalar ---- */

struct ext2_fs {
    struct super_block vsb;             /* VFS superbloki (priv = shu struct) */
    struct block_device *bd;
    struct ext2_super sb;               /* superblok nusxasi */
    struct ext2_group_desc *gd;
    uint32_t block_size;
    uint32_t groups;
    uint32_t inode_size;
    uint32_t addr_per_block;            /* bitta blokdagi ko'rsatkichlar (256 yoki 1024) */
    uint32_t gd_blocks;                 /* deskriptorlar jadvali necha blok */
    struct mutex alloc_lock;            /* bitmap'lar, sanagichlar, superblok */
    struct mutex rename_lock;           /* papkalarni ko'chirishni ketma-ket qilish */
    spinlock_t icache_lock;
    struct list_head icache;            /* xotiradagi inode'lar */
};

/* Har bir ext2 inode'ining VFS inode'iga biriktirilgan ma'lumoti. */
struct ext2_node {
    struct ext2_inode raw;              /* diskdagi nusxa (i_block, i_blocks, flags ...) */
    struct inode *vfs;
    struct list_head node;              /* icache */
};

#define FS(in)   ((struct ext2_fs *)(in)->sb->priv)
#define NODE(in) ((struct ext2_node *)(in)->priv)

static const struct inode_ops ext2_dir_iops;
static const struct inode_ops ext2_file_iops;
static const struct file_ops ext2_file_fops;
static const struct super_ops ext2_sops;

/* ============================================================================
 *  Bloklar va superblok
 * ========================================================================== */

static struct buf *fs_bread(struct ext2_fs *fs, uint32_t block)
{
    if (block == 0 || block >= fs->sb.s_blocks_count) {
        kprintf("[ext2] noto'g'ri blok raqami %u - fayl tizimi buzilgan?\n", block);
        return NULL;
    }
    return bread(fs->bd, block, fs->block_size);
}

/* Superblok va guruh deskriptorlarini diskka yozish (alloc_lock ostida).
 * Superblok DOIM baytlar 1024..2047 da: 1 KB blokda - 1-blok, kattarog'ida
 * 0-blokning ichida (1024 siljishda). Bir xil baytlarni keshda turli
 * o'lchamdagi bloklar sifatida saqlamaslik uchun doim FS blok hajmida o'qiymiz. */
static int write_super(struct ext2_fs *fs)
{
    uint32_t blk = fs->block_size == 1024 ? 1 : 0;
    uint32_t off = fs->block_size == 1024 ? 0 : 1024;
    struct buf *b = bread(fs->bd, blk, fs->block_size);
    if (!b)
        return -EIO;
    fs->sb.s_wtime = (uint32_t)time_now();
    memcpy(b->data + off, &fs->sb, sizeof(fs->sb));
    int r = bwrite(b);
    brelse(b);
    return r;
}

/* g-guruh deskriptori joylashgan blokni diskka yozish. */
static int write_gd(struct ext2_fs *fs, uint32_t g)
{
    uint32_t per_block = fs->block_size / sizeof(struct ext2_group_desc);
    uint32_t idx = g / per_block;
    struct buf *b = fs_bread(fs, fs->sb.s_first_data_block + 1 + idx);
    if (!b)
        return -EIO;
    memcpy(b->data, (uint8_t *)fs->gd + idx * fs->block_size,
           MIN(fs->block_size, fs->groups * sizeof(struct ext2_group_desc) - idx * fs->block_size));
    int r = bwrite(b);
    brelse(b);
    return r;
}

/* ============================================================================
 *  Bitmap'lar: blok va inode ajratish/bo'shatish
 * ========================================================================== */

/* bitmap blokidan birinchi 0 bitni topish, start dan boshlab. -1 - yo'q. */
static int find_zero_bit(const uint8_t *map, uint32_t nbits, uint32_t start)
{
    for (uint32_t i = start; i < nbits; i++) {
        if (map[i / 8] == 0xFF) {       /* butun bayt band - 8 tasini birdan o'tkazamiz */
            i |= 7;
            continue;
        }
        if (!(map[i / 8] & (1u << (i % 8))))
            return (int)i;
    }
    for (uint32_t i = 0; i < start && i < nbits; i++)
        if (!(map[i / 8] & (1u << (i % 8))))
            return (int)i;
    return -1;
}

/* Yangi blok ajratish. goal - "shu blokka yaqin bo'lsin" (fayl davomi).
 * Qaytaradi: blok raqami yoki 0 (joy yo'q). */
static uint32_t alloc_block(struct ext2_fs *fs, uint32_t goal)
{
    mutex_lock(&fs->alloc_lock);
    uint32_t bpg = fs->sb.s_blocks_per_group;
    uint32_t first = fs->sb.s_first_data_block;
    if (goal < first || goal >= fs->sb.s_blocks_count)
        goal = first;
    uint32_t g0 = (goal - first) / bpg;
    uint32_t result = 0;
    for (uint32_t k = 0; k < fs->groups && !result; k++) {
        uint32_t g = (g0 + k) % fs->groups;
        if (fs->gd[g].bg_free_blocks_count == 0)
            continue;
        struct buf *b = fs_bread(fs, fs->gd[g].bg_block_bitmap);
        if (!b)
            break;
        uint32_t nbits = MIN(bpg, fs->sb.s_blocks_count - first - g * bpg);
        uint32_t start = (g == g0) ? (goal - first) % bpg : 0;
        int bit = find_zero_bit(b->data, nbits, start);
        if (bit >= 0) {
            b->data[bit / 8] |= (uint8_t)(1u << (bit % 8));
            bwrite(b);
            fs->gd[g].bg_free_blocks_count--;
            fs->sb.s_free_blocks_count--;
            write_gd(fs, g);
            write_super(fs);
            result = first + g * bpg + (uint32_t)bit;
        }
        brelse(b);
    }
    mutex_unlock(&fs->alloc_lock);
    if (result) {                       /* yangi blok - eski "axlat" ko'rinmasin */
        struct buf *b = fs_bread(fs, result);
        if (b) {
            memset(b->data, 0, fs->block_size);
            bwrite(b);
            brelse(b);
        }
    }
    return result;
}

static void free_block(struct ext2_fs *fs, uint32_t block)
{
    if (block < fs->sb.s_first_data_block || block >= fs->sb.s_blocks_count)
        return;
    mutex_lock(&fs->alloc_lock);
    uint32_t g = (block - fs->sb.s_first_data_block) / fs->sb.s_blocks_per_group;
    uint32_t bit = (block - fs->sb.s_first_data_block) % fs->sb.s_blocks_per_group;
    struct buf *b = fs_bread(fs, fs->gd[g].bg_block_bitmap);
    if (b) {
        if (!(b->data[bit / 8] & (1u << (bit % 8)))) {
            kprintf("[ext2] OGOHLANTIRISH: bo'sh blok %u qayta bo'shatilmoqda\n", block);
        } else {
            b->data[bit / 8] &= (uint8_t)~(1u << (bit % 8));
            bwrite(b);
            fs->gd[g].bg_free_blocks_count++;
            fs->sb.s_free_blocks_count++;
            write_gd(fs, g);
            write_super(fs);
        }
        brelse(b);
    }
    mutex_unlock(&fs->alloc_lock);
}

/* Yangi inode raqami. Papkalar bo'sh inode'i eng ko'p guruhga (tarqatish),
 * fayllar - otasi turgan guruhga (yaqinlik). 0 - joy yo'q. */
static uint32_t alloc_inode_nr(struct ext2_fs *fs, uint32_t parent_ino, bool is_dir)
{
    mutex_lock(&fs->alloc_lock);
    uint32_t ipg = fs->sb.s_inodes_per_group;
    uint32_t g0 = (parent_ino - 1) / ipg;
    if (is_dir) {
        uint32_t best = g0;
        for (uint32_t g = 0; g < fs->groups; g++)
            if (fs->gd[g].bg_free_inodes_count > fs->gd[best].bg_free_inodes_count)
                best = g;
        g0 = best;
    }
    uint32_t result = 0;
    for (uint32_t k = 0; k < fs->groups && !result; k++) {
        uint32_t g = (g0 + k) % fs->groups;
        if (fs->gd[g].bg_free_inodes_count == 0)
            continue;
        struct buf *b = fs_bread(fs, fs->gd[g].bg_inode_bitmap);
        if (!b)
            break;
        /* 0-guruhda zaxira inode'lar (1..10) - ularni hech qachon bermaymiz. */
        uint32_t start = g == 0 ? fs->sb.s_first_ino - 1 : 0;
        int bit = find_zero_bit(b->data, ipg, start);
        if (bit >= 0 && (g != 0 || (uint32_t)bit >= start)) {
            b->data[bit / 8] |= (uint8_t)(1u << (bit % 8));
            bwrite(b);
            fs->gd[g].bg_free_inodes_count--;
            if (is_dir)
                fs->gd[g].bg_used_dirs_count++;
            fs->sb.s_free_inodes_count--;
            write_gd(fs, g);
            write_super(fs);
            result = g * ipg + (uint32_t)bit + 1;
        }
        brelse(b);
    }
    mutex_unlock(&fs->alloc_lock);
    return result;
}

static void free_inode_nr(struct ext2_fs *fs, uint32_t ino, bool is_dir)
{
    mutex_lock(&fs->alloc_lock);
    uint32_t g = (ino - 1) / fs->sb.s_inodes_per_group;
    uint32_t bit = (ino - 1) % fs->sb.s_inodes_per_group;
    struct buf *b = fs_bread(fs, fs->gd[g].bg_inode_bitmap);
    if (b) {
        if (b->data[bit / 8] & (1u << (bit % 8))) {
            b->data[bit / 8] &= (uint8_t)~(1u << (bit % 8));
            bwrite(b);
            fs->gd[g].bg_free_inodes_count++;
            if (is_dir)
                fs->gd[g].bg_used_dirs_count--;
            fs->sb.s_free_inodes_count++;
            write_gd(fs, g);
            write_super(fs);
        }
        brelse(b);
    }
    mutex_unlock(&fs->alloc_lock);
}

/* ============================================================================
 *  Inode'larni o'qish/yozish va inode keshi
 * ========================================================================== */

/* ino ning diskdagi joyi: qaysi blok, blok ichida qaysi siljish. */
static void inode_location(struct ext2_fs *fs, uint32_t ino, uint32_t *block, uint32_t *off)
{
    uint32_t g = (ino - 1) / fs->sb.s_inodes_per_group;
    uint32_t idx = (ino - 1) % fs->sb.s_inodes_per_group;
    uint64_t byte = (uint64_t)idx * fs->inode_size;
    *block = fs->gd[g].bg_inode_table + (uint32_t)(byte / fs->block_size);
    *off = (uint32_t)(byte % fs->block_size);
}

/* VFS maydonlaridan (hajm, rejim, vaqtlar) diskdagi nusxani yangilab, yozish.
 * Faqat birinchi 128 bayt - 256 baytli inode'ning qolgan qismiga (ext4
 * qo'shimchalari) tegmaymiz. */
static int write_inode(struct inode *in)
{
    struct ext2_fs *fs = FS(in);
    struct ext2_inode *r = &NODE(in)->raw;
    r->i_mode = (uint16_t)in->mode;
    r->i_uid = (uint16_t)in->uid;
    r->i_gid = (uint16_t)in->gid;
    r->i_links_count = (uint16_t)in->nlink;
    r->i_size = (uint32_t)in->size;
    if (S_ISREG(in->mode))
        r->i_size_high = (uint32_t)(in->size >> 32);
    r->i_atime = (uint32_t)in->atime;
    r->i_mtime = (uint32_t)in->mtime;
    r->i_ctime = (uint32_t)in->ctime;
    uint32_t block, off;
    inode_location(fs, (uint32_t)in->ino, &block, &off);
    struct buf *b = fs_bread(fs, block);
    if (!b)
        return -EIO;
    memcpy(b->data + off, r, sizeof(*r));
    int err = bwrite(b);
    brelse(b);
    return err;
}

/* refcount 0 bo'lmasa, uni oshirish (0 bo'lsa - inode yo'q qilinmoqda). */
static bool get_unless_zero(struct inode *in)
{
    int v = __atomic_load_n(&in->refcount, __ATOMIC_RELAXED);
    while (v > 0) {
        if (__atomic_compare_exchange_n(&in->refcount, &v, v + 1, false, __ATOMIC_ACQ_REL,
                                        __ATOMIC_RELAXED))
            return true;
    }
    return false;
}

static struct inode *icache_find(struct ext2_fs *fs, uint32_t ino)
{
    struct ext2_node *n;
    list_for_each_entry(n, &fs->icache, node)
        if (n->vfs->ino == ino && get_unless_zero(n->vfs))
            return n->vfs;
    return NULL;
}

static void setup_ops(struct inode *in)
{
    if (S_ISDIR(in->mode)) {
        in->iops = &ext2_dir_iops;
        in->fops = NULL;
    } else if (S_ISREG(in->mode)) {
        in->iops = &ext2_file_iops;
        in->fops = &ext2_file_fops;
    } else {
        in->iops = NULL;                /* ramziy havola, qurilma - hozircha faqat ro'yxatda */
        in->fops = NULL;
    }
}

/* ino raqamli inode'ni olish (REFERENS bilan). Bir inode uchun xotirada
 * BITTA struct inode bo'lishi shart - aks holda ikki nusxa bir-birining
 * o'zgarishlarini ko'rmaydi. Shuning uchun avval keshdan qidiramiz. */
static struct inode *ext2_iget(struct ext2_fs *fs, uint32_t ino)
{
    if (ino < 1 || ino > fs->sb.s_inodes_count)
        return NULL;
    spin_lock(&fs->icache_lock);
    struct inode *in = icache_find(fs, ino);
    spin_unlock(&fs->icache_lock);
    if (in)
        return in;

    uint32_t block, off;
    inode_location(fs, ino, &block, &off);
    struct buf *b = fs_bread(fs, block);
    if (!b)
        return NULL;
    struct ext2_node *n = kzalloc(sizeof(*n));
    in = n ? inode_alloc(&fs->vsb) : NULL;
    if (!in) {
        kfree(n);
        brelse(b);
        return NULL;
    }
    memcpy(&n->raw, b->data + off, sizeof(n->raw));
    brelse(b);

    struct ext2_inode *r = &n->raw;
    in->ino = ino;
    in->mode = r->i_mode;
    in->uid = r->i_uid;
    in->gid = r->i_gid;
    in->nlink = r->i_links_count;
    in->size = r->i_size;
    if (S_ISREG(in->mode))
        in->size |= (uint64_t)r->i_size_high << 32;
    in->atime = r->i_atime;
    in->mtime = r->i_mtime;
    in->ctime = r->i_ctime;
    if (S_ISCHR(in->mode) || S_ISBLK(in->mode)) {
        /* Qurilma raqami i_block[0] (eski) yoki i_block[1] (yangi format) da. */
        uint32_t d = r->i_block[0] ? r->i_block[0] : r->i_block[1];
        in->rdev = r->i_block[0] ? MKDEV((d >> 8) & 0xFF, d & 0xFF)
                                 : MKDEV((d >> 8) & 0xFFF, (d & 0xFF) | ((d >> 12) & 0xFFF00));
    }
    in->priv = n;
    n->vfs = in;
    setup_ops(in);

    /* Biz diskdan o'qiyotganda boshqa jarayon ham xuddi shu inode'ni yuklagan
     * bo'lishi mumkin - qayta tekshiramiz. */
    spin_lock(&fs->icache_lock);
    struct inode *other = icache_find(fs, ino);
    if (!other)
        list_add(&n->node, &fs->icache);
    spin_unlock(&fs->icache_lock);
    if (other) {
        in->priv = NULL;
        kfree(n);
        iput(in);                       /* free_inode chaqirilmaydi: priv = NULL */
        return other;
    }
    return in;
}

/* ============================================================================
 *  Blok xaritasi: fayldagi N-blok -> diskdagi blok
 * ========================================================================== */

/* Fayl bloki fblock ga yo'l: har bir darajadagi indeks. Qaytaradi: chuqurlik
 * (1 = to'g'ridan-to'g'ri, 2 = bilvosita, ...), 0 - juda katta. */
static int block_path(struct ext2_fs *fs, uint64_t fblock, uint32_t path[4])
{
    /* >>> LAB block_path - vazifa: labs/README.md */
    uint64_t apb = fs->addr_per_block;
    if (fblock < EXT2_NDIR_BLOCKS) {
        path[0] = (uint32_t)fblock;
        return 1;
    }
    fblock -= EXT2_NDIR_BLOCKS;
    if (fblock < apb) {
        path[0] = EXT2_IND_BLOCK;
        path[1] = (uint32_t)fblock;
        return 2;
    }
    fblock -= apb;
    if (fblock < apb * apb) {
        path[0] = EXT2_DIND_BLOCK;
        path[1] = (uint32_t)(fblock / apb);
        path[2] = (uint32_t)(fblock % apb);
        return 3;
    }
    fblock -= apb * apb;
    if (fblock < apb * apb * apb) {
        path[0] = EXT2_TIND_BLOCK;
        path[1] = (uint32_t)(fblock / (apb * apb));
        path[2] = (uint32_t)(fblock / apb % apb);
        path[3] = (uint32_t)(fblock % apb);
        return 4;
    }
    return 0;
    /* <<< LAB block_path */
}

/* fblock uchun disk blokini topish. alloc = true bo'lsa, yetishmayotgan
 * bloklar (bilvosita ham) ajratiladi. *out = 0 - teshik (alloc = false da).
 * inode qulfi ushlangan bo'lishi kerak. */
static int bmap(struct inode *in, uint64_t fblock, bool alloc, uint32_t *out)
{
    /* >>> LAB bmap - vazifa: labs/README.md */
    struct ext2_fs *fs = FS(in);
    struct ext2_node *n = NODE(in);
    uint32_t path[4];
    int depth = block_path(fs, fblock, path);
    if (!depth)
        return -EFBIG;
    uint32_t sectors_per_block = fs->block_size / 512;
    bool inode_dirty = false;

    /* 1-daraja: inode ichidagi i_block[] */
    uint32_t cur = n->raw.i_block[path[0]];
    if (!cur) {
        if (!alloc) {
            *out = 0;
            return 0;
        }
        /* Yaqin joy: inode guruhining boshi. */
        uint32_t goal = fs->sb.s_first_data_block +
               ((uint32_t)(in->ino - 1) / fs->sb.s_inodes_per_group) * fs->sb.s_blocks_per_group;
        cur = alloc_block(fs, goal);
        if (!cur)
            return -ENOSPC;
        n->raw.i_block[path[0]] = cur;
        n->raw.i_blocks += sectors_per_block;
        inode_dirty = true;
    }
    /* Keyingi darajalar: bilvosita bloklar ichidagi ko'rsatkichlar. */
    for (int level = 1; level < depth; level++) {
        struct buf *b = fs_bread(fs, cur);
        if (!b)
            return -EIO;
        uint32_t *ptrs = (uint32_t *)b->data;
        uint32_t next = ptrs[path[level]];
        if (!next) {
            if (!alloc) {
                brelse(b);
                *out = 0;
                goto done;
            }
            next = alloc_block(fs, cur + 1);
            if (!next) {
                brelse(b);
                if (inode_dirty)
                    write_inode(in);
                return -ENOSPC;
            }
            ptrs[path[level]] = next;
            bwrite(b);
            n->raw.i_blocks += sectors_per_block;
            inode_dirty = true;
        }
        brelse(b);
        cur = next;
    }
    *out = cur;
done:
    if (inode_dirty)
        write_inode(in);
    return 0;
    /* <<< LAB bmap */
}

/* ============================================================================
 *  Fayl o'qish/yozish/qisqartirish
 * ========================================================================== */

static int64_t ext2_read(struct file *f, void *buf, size_t len, uint64_t off)
{
    struct inode *in = f->inode;
    struct ext2_fs *fs = FS(in);
    mutex_lock(&in->lock);
    if (off >= in->size) {
        mutex_unlock(&in->lock);
        return 0;
    }
    len = MIN(len, in->size - off);
    size_t done = 0;
    int err = 0;
    while (done < len) {
        uint64_t pos = off + done;
        uint32_t in_block = (uint32_t)(pos % fs->block_size);
        size_t chunk = MIN(len - done, (size_t)(fs->block_size - in_block));
        uint32_t blk;
        if ((err = bmap(in, pos / fs->block_size, false, &blk)) != 0)
            break;
        if (!blk) {
            memset((uint8_t *)buf + done, 0, chunk);    /* teshik - nollar */
        } else {
            struct buf *b = fs_bread(fs, blk);
            if (!b) {
                err = -EIO;
                break;
            }
            memcpy((uint8_t *)buf + done, b->data + in_block, chunk);
            brelse(b);
        }
        done += chunk;
    }
    mutex_unlock(&in->lock);
    return done ? (int64_t)done : err;
}

static int64_t ext2_write(struct file *f, const void *buf, size_t len, uint64_t off)
{
    struct inode *in = f->inode;
    struct ext2_fs *fs = FS(in);
    if (fs->vsb.read_only)
        return -EROFS;
    mutex_lock(&in->lock);
    size_t done = 0;
    int err = 0;
    while (done < len) {
        uint64_t pos = off + done;
        uint32_t in_block = (uint32_t)(pos % fs->block_size);
        size_t chunk = MIN(len - done, (size_t)(fs->block_size - in_block));
        uint32_t blk;
        if ((err = bmap(in, pos / fs->block_size, true, &blk)) != 0)
            break;
        struct buf *b = fs_bread(fs, blk);
        if (!b) {
            err = -EIO;
            break;
        }
        memcpy(b->data + in_block, (const uint8_t *)buf + done, chunk);
        err = bwrite(b);
        brelse(b);
        if (err)
            break;
        done += chunk;
    }
    if (done) {
        if (off + done > in->size)
            in->size = off + done;
        in->mtime = in->ctime = time_now();
        write_inode(in);
    }
    mutex_unlock(&in->lock);
    return done ? (int64_t)done : err;
}

/* Diskdagi blok daraxtini (depth = 0 - ma'lumot bloki) butunlay bo'shatish. */
static void free_tree(struct inode *in, uint32_t blk, int depth)
{
    struct ext2_fs *fs = FS(in);
    if (!blk)
        return;
    if (depth > 0) {
        struct buf *b = fs_bread(fs, blk);
        if (b) {
            /* Ko'rsatkichlarni nusxalab, buferni darhol qaytaramiz (chuqur rekursiyada
             * ko'p bufer ushlab turmaslik uchun). */
            uint32_t *copy = kmalloc(fs->block_size);
            if (copy)
                memcpy(copy, b->data, fs->block_size);
            brelse(b);
            if (copy) {
                for (uint32_t i = 0; i < fs->addr_per_block; i++)
                    free_tree(in, copy[i], depth - 1);
                kfree(copy);
            }
        }
    }
    free_block(fs, blk);
    NODE(in)->raw.i_blocks -= fs->block_size / 512;
}

/* *ptr ildizli daraxtda fayl bloklari [base, base + span) joylashgan. Indeksi
 * keep dan katta yoki teng bo'lgan hamma narsani bo'shatish. */
static void trunc_tree(struct inode *in, uint32_t *ptr, int depth, uint64_t base, uint64_t keep)
{
    struct ext2_fs *fs = FS(in);
    if (!*ptr)
        return;
    uint64_t child_span = 1;
    for (int i = 1; i < depth; i++)
        child_span *= fs->addr_per_block;
    uint64_t span = depth ? child_span * fs->addr_per_block : 1;
    if (base >= keep) {                 /* butun daraxt kerak emas */
        free_tree(in, *ptr, depth);
        *ptr = 0;
        return;
    }
    if (depth == 0 || base + span <= keep)
        return;                         /* butunlay saqlanadi */
    struct buf *b = fs_bread(fs, *ptr);
    if (!b)
        return;
    uint32_t *ptrs = (uint32_t *)b->data;
    bool dirty = false;
    for (uint32_t i = 0; i < fs->addr_per_block; i++) {
        uint64_t cb = base + i * child_span;
        if (cb + child_span <= keep || !ptrs[i])
            continue;
        uint32_t p = ptrs[i];
        trunc_tree(in, &p, depth - 1, cb, keep);
        if (p != ptrs[i]) {
            ptrs[i] = p;
            dirty = true;
        }
    }
    if (dirty)
        bwrite(b);
    brelse(b);
}

/* inode qulfi ushlangan holda. */
static int ext2_truncate(struct inode *in, uint64_t size)
{
    struct ext2_fs *fs = FS(in);
    if (fs->vsb.read_only)
        return -EROFS;
    if (size < in->size) {
        uint64_t keep = (size + fs->block_size - 1) / fs->block_size;
        uint64_t apb = fs->addr_per_block;
        struct ext2_inode *r = &NODE(in)->raw;
        for (int i = 0; i < EXT2_NDIR_BLOCKS; i++)
            trunc_tree(in, &r->i_block[i], 0, (uint64_t)i, keep);
        trunc_tree(in, &r->i_block[EXT2_IND_BLOCK], 1, 12, keep);
        trunc_tree(in, &r->i_block[EXT2_DIND_BLOCK], 2, 12 + apb, keep);
        trunc_tree(in, &r->i_block[EXT2_TIND_BLOCK], 3, 12 + apb + apb * apb, keep);
        /* Oxirgi (qisman) blokning qolgan qismini nollaymiz - keyin fayl
         * kattalashtirilsa, eski ma'lumot "qaytib chiqmasin". */
        uint32_t tail = (uint32_t)(size % fs->block_size);
        uint32_t blk;
        if (tail && bmap(in, size / fs->block_size, false, &blk) == 0 && blk) {
            struct buf *b = fs_bread(fs, blk);
            if (b) {
                memset(b->data + tail, 0, fs->block_size - tail);
                bwrite(b);
                brelse(b);
            }
        }
    }
    in->size = size;
    in->mtime = in->ctime = time_now();
    return write_inode(in);
}

/* ============================================================================
 *  Papkalar
 * ========================================================================== */

static uint8_t mode_to_ft(uint32_t mode)
{
    switch (mode & S_IFMT) {
    case S_IFREG: return FT_REG;
    case S_IFDIR: return FT_DIR;
    case S_IFCHR: return FT_CHR;
    case S_IFBLK: return FT_BLK;
    case S_IFIFO: return FT_FIFO;
    case S_IFLNK: return FT_SYMLINK;
    default:      return FT_UNKNOWN;
    }
}

static uint32_t ft_to_mode(uint8_t ft)
{
    static const uint32_t map[] = { 0, S_IFREG, S_IFDIR, S_IFCHR, S_IFBLK, S_IFIFO, 0140000,
                                    S_IFLNK };
    return ft < 8 ? map[ft] : 0;
}

/* Yozuvning kerakli hajmi: 8 bayt sarlavha + nom, 4 ga tekislangan. */
static uint32_t rec_size(uint32_t name_len)
{
    return (8 + name_len + 3) & ~3u;
}

/* Papka yozuvi yaroqlimi? (Buzilgan disk yadroni qulatmasligi kerak!) */
static bool dirent_ok(struct ext2_fs *fs, const struct ext2_dirent *d, uint32_t off)
{
    return d->rec_len >= 8 && (d->rec_len & 3) == 0 && off + d->rec_len <= fs->block_size &&
           rec_size(d->name_len) <= d->rec_len;
}

/* Papkani aylanib chiqish. fn true qaytarsa - to'xtaymiz. Bufer fn ga
 * QULFLANGAN holda beriladi (o'zgartirish mumkin - keyin bwrite). */
typedef bool (*dir_visit_fn)(struct buf *b, struct ext2_dirent *d, struct ext2_dirent *prev,
                             uint32_t off, void *ctx);

static int dir_iterate(struct inode *dir, dir_visit_fn fn, void *ctx)
{
    struct ext2_fs *fs = FS(dir);
    uint64_t nblocks = dir->size / fs->block_size;
    for (uint64_t i = 0; i < nblocks; i++) {
        uint32_t blk;
        int err = bmap(dir, i, false, &blk);
        if (err)
            return err;
        if (!blk)
            continue;
        struct buf *b = fs_bread(fs, blk);
        if (!b)
            return -EIO;
        struct ext2_dirent *prev = NULL;
        for (uint32_t off = 0; off < fs->block_size;) {
            struct ext2_dirent *d = (struct ext2_dirent *)(b->data + off);
            if (!dirent_ok(fs, d, off)) {
                kprintf("[ext2] inode %lu: buzilgan papka yozuvi (blok %u, siljish %u)\n",
                        dir->ino, blk, off);
                break;
            }
            if (fn(b, d, prev, off, ctx)) {
                brelse(b);
                return 1;
            }
            prev = d;
            off += d->rec_len;
        }
        brelse(b);
    }
    return 0;
}

struct find_ctx {
    const char *name;
    size_t len;
    uint32_t ino;
    uint8_t ft;
};

static bool find_visit(struct buf *b, struct ext2_dirent *d, struct ext2_dirent *prev,
                       uint32_t off, void *ctx)
{
    (void)b, (void)prev, (void)off;
    struct find_ctx *c = ctx;
    if (d->inode && d->name_len == c->len && memcmp(d->name, c->name, c->len) == 0) {
        c->ino = d->inode;
        c->ft = d->file_type;
        return true;
    }
    return false;
}

static uint32_t dir_find(struct inode *dir, const char *name)
{
    struct find_ctx c = { name, strlen(name), 0, 0 };
    return dir_iterate(dir, find_visit, &c) == 1 ? c.ino : 0;
}

static struct inode *ext2_lookup(struct inode *dir, const char *name)
{
    uint32_t ino = dir_find(dir, name);
    return ino ? ext2_iget(FS(dir), ino) : NULL;
}

struct add_ctx {
    const char *name;
    uint32_t len;
    uint32_t ino;
    uint8_t ft;
    bool done;
};

static bool add_visit(struct buf *b, struct ext2_dirent *d, struct ext2_dirent *prev,
                      uint32_t off, void *ctx)
{
    /* >>> LAB add_visit - vazifa: labs/README.md */
    (void)prev, (void)off;
    struct add_ctx *c = ctx;
    uint32_t need = rec_size(c->len);
    uint32_t used = d->inode ? rec_size(d->name_len) : 0;
    if (d->rec_len - used < need)
        return false;
    struct ext2_dirent *nd = d;
    if (d->inode) {                     /* mavjud yozuvni bo'lamiz: uning "ortiqcha" joyiga */
        nd = (struct ext2_dirent *)((uint8_t *)d + used);
        nd->rec_len = (uint16_t)(d->rec_len - used);
        d->rec_len = (uint16_t)used;
    }
    nd->inode = c->ino;
    nd->name_len = (uint8_t)c->len;
    nd->file_type = c->ft;
    memcpy(nd->name, c->name, c->len);
    bwrite(b);
    c->done = true;
    return true;
    /* <<< LAB add_visit */
}

/* Papkaga yangi yozuv qo'shish (dir qulfi ushlangan). */
static int dir_add(struct inode *dir, const char *name, uint32_t ino, uint8_t ft)
{
    /* >>> LAB dir_add - vazifa: labs/README.md */
    struct ext2_fs *fs = FS(dir);
    struct add_ctx c = { name, (uint32_t)strlen(name), ino, ft, false };
    if (c.len > NAME_MAX)
        return -ENAMETOOLONG;
    int err = dir_iterate(dir, add_visit, &c);
    if (err < 0)
        return err;
    if (c.done)
        return 0;
    /* Joy yo'q - papkaga yangi blok qo'shamiz. */
    uint32_t blk;
    err = bmap(dir, dir->size / fs->block_size, true, &blk);
    if (err)
        return err;
    struct buf *b = fs_bread(fs, blk);
    if (!b)
        return -EIO;
    struct ext2_dirent *d = (struct ext2_dirent *)b->data;
    d->inode = ino;
    d->rec_len = (uint16_t)fs->block_size;
    d->name_len = (uint8_t)c.len;
    d->file_type = ft;
    memcpy(d->name, name, c.len);
    bwrite(b);
    brelse(b);
    dir->size += fs->block_size;
    dir->mtime = dir->ctime = time_now();
    return write_inode(dir);
    /* <<< LAB dir_add */
}

static bool remove_visit(struct buf *b, struct ext2_dirent *d, struct ext2_dirent *prev,
                         uint32_t off, void *ctx)
{
    (void)off;
    struct find_ctx *c = ctx;
    if (!d->inode || d->name_len != c->len || memcmp(d->name, c->name, c->len) != 0)
        return false;
    if (prev)
        prev->rec_len = (uint16_t)(prev->rec_len + d->rec_len);    /* oldingisi "yutadi" */
    else
        d->inode = 0;                   /* blokdagi birinchi yozuv - bo'sh deb belgilaymiz */
    bwrite(b);
    c->ino = 1;
    return true;
}

static int dir_remove(struct inode *dir, const char *name)
{
    struct find_ctx c = { name, strlen(name), 0, 0 };
    int r = dir_iterate(dir, remove_visit, &c);
    if (r < 0)
        return r;
    if (!c.ino)
        return -ENOENT;
    dir->mtime = dir->ctime = time_now();
    return write_inode(dir);
}

static bool empty_visit(struct buf *b, struct ext2_dirent *d, struct ext2_dirent *prev,
                        uint32_t off, void *ctx)
{
    (void)b, (void)prev, (void)off;
    if (!d->inode)
        return false;
    if ((d->name_len == 1 && d->name[0] == '.') ||
        (d->name_len == 2 && d->name[0] == '.' && d->name[1] == '.'))
        return false;
    *(bool *)ctx = false;
    return true;
}

static bool dir_is_empty(struct inode *dir)
{
    bool empty = true;
    dir_iterate(dir, empty_visit, &empty);
    return empty;
}

/* ".." yozuvini yangilash (papka boshqa joyga ko'chirilganda). */
static bool dotdot_visit(struct buf *b, struct ext2_dirent *d, struct ext2_dirent *prev,
                         uint32_t off, void *ctx)
{
    (void)prev, (void)off;
    if (d->name_len == 2 && d->name[0] == '.' && d->name[1] == '.') {
        d->inode = *(uint32_t *)ctx;
        bwrite(b);
        return true;
    }
    return false;
}

static int ext2_readdir(struct inode *dir, uint64_t *pos, struct myos_dirent *out)
{
    struct ext2_fs *fs = FS(dir);
    while (*pos < dir->size) {
        uint32_t off = (uint32_t)(*pos % fs->block_size);
        uint32_t blk;
        int err = bmap(dir, *pos / fs->block_size, false, &blk);
        if (err)
            return err;
        if (!blk) {                     /* teshik - keyingi blokka */
            *pos = (*pos / fs->block_size + 1) * fs->block_size;
            continue;
        }
        struct buf *b = fs_bread(fs, blk);
        if (!b)
            return -EIO;
        struct ext2_dirent *d = (struct ext2_dirent *)(b->data + off);
        if (!dirent_ok(fs, d, off)) {
            brelse(b);
            *pos = (*pos / fs->block_size + 1) * fs->block_size;
            continue;
        }
        *pos += d->rec_len;
        if (d->inode) {
            out->d_ino = d->inode;
            out->d_type = ft_to_mode(d->file_type);
            memcpy(out->d_name, d->name, d->name_len);
            out->d_name[d->name_len] = '\0';
            brelse(b);
            if (!out->d_type) {         /* eski FS (filetype xususiyatisiz) - inode'dan */
                struct inode *c = ext2_iget(fs, out->d_ino);
                if (c) {
                    out->d_type = c->mode & S_IFMT;
                    iput(c);
                }
            }
            return 1;
        }
        brelse(b);
    }
    return 0;
}

/* ============================================================================
 *  Yaratish va o'chirish
 * ========================================================================== */

/* Yangi inode (diskda ham, xotirada ham). Referens bilan qaytadi. */
static struct inode *new_inode(struct inode *dir, uint32_t mode)
{
    struct ext2_fs *fs = FS(dir);
    uint32_t ino = alloc_inode_nr(fs, (uint32_t)dir->ino, S_ISDIR(mode));
    if (!ino)
        return NULL;
    /* Diskdagi eski inode'ni nollaymiz (oldingi faylning i_block'lari qolmasin). */
    uint32_t block, off;
    inode_location(fs, ino, &block, &off);
    struct buf *b = fs_bread(fs, block);
    if (b) {
        memset(b->data + off, 0, fs->inode_size);
        /* 256 baytli inode: 128 dan keyingi qism "qo'shimcha maydonlar" (ext4 dagi
         * nanosekund vaqtlar va h.k.). Ularning hajmi (i_extra_isize) - Linux kabi 32. */
        if (fs->inode_size > 128)
            *(uint16_t *)(b->data + off + 128) = 32;
        bwrite(b);
        brelse(b);
    }
    struct inode *in = ext2_iget(fs, ino);
    if (!in) {
        free_inode_nr(fs, ino, S_ISDIR(mode));
        return NULL;
    }
    in->mode = mode;
    in->nlink = 1;
    in->size = 0;
    in->uid = in->gid = 0;
    in->atime = in->mtime = in->ctime = time_now();
    setup_ops(in);
    write_inode(in);
    return in;
}

static int ext2_create(struct inode *dir, const char *name, uint32_t mode, struct inode **out)
{
    struct inode *in = new_inode(dir, mode);
    if (!in)
        return -ENOSPC;
    int err = dir_add(dir, name, (uint32_t)in->ino, mode_to_ft(mode));
    if (err) {
        in->nlink = 0;                  /* iput inode'ni diskdan o'chiradi */
        iput(in);
        return err;
    }
    *out = in;
    return 0;
}

static int ext2_mkdir(struct inode *dir, const char *name, uint32_t mode)
{
    struct ext2_fs *fs = FS(dir);
    struct inode *in = new_inode(dir, mode);
    if (!in)
        return -ENOSPC;
    /* Birinchi blok: "." (o'zi) va ".." (ota). */
    uint32_t blk;
    mutex_lock(&in->lock);
    int err = bmap(in, 0, true, &blk);
    if (!err) {
        struct buf *b = fs_bread(fs, blk);
        if (b) {
            struct ext2_dirent *d = (struct ext2_dirent *)b->data;
            d->inode = (uint32_t)in->ino;
            d->rec_len = 12;
            d->name_len = 1;
            d->file_type = FT_DIR;
            d->name[0] = '.';
            d = (struct ext2_dirent *)(b->data + 12);
            d->inode = (uint32_t)dir->ino;
            d->rec_len = (uint16_t)(fs->block_size - 12);
            d->name_len = 2;
            d->file_type = FT_DIR;
            d->name[0] = d->name[1] = '.';
            bwrite(b);
            brelse(b);
        } else {
            err = -EIO;
        }
    }
    if (!err) {
        in->size = fs->block_size;
        in->nlink = 2;                  /* ota papkadagi nom + o'zining "." */
        write_inode(in);
    }
    mutex_unlock(&in->lock);
    if (!err)
        err = dir_add(dir, name, (uint32_t)in->ino, FT_DIR);
    if (err) {
        mutex_lock(&in->lock);
        ext2_truncate(in, 0);
        in->nlink = 0;
        mutex_unlock(&in->lock);
        iput(in);
        return err;
    }
    dir->nlink++;                       /* bolaning ".." si otaga havola */
    write_inode(dir);
    iput(in);
    return 0;
}

static int ext2_unlink(struct inode *dir, const char *name)
{
    struct inode *in = ext2_lookup(dir, name);
    if (!in)
        return -ENOENT;
    int err = dir_remove(dir, name);
    if (!err) {
        mutex_lock(&in->lock);
        in->nlink--;
        in->ctime = time_now();
        write_inode(in);
        mutex_unlock(&in->lock);
    }
    iput(in);                           /* oxirgi referens bo'lsa - fayl haqiqatan o'chadi */
    return err;
}

static int ext2_rmdir(struct inode *dir, const char *name)
{
    struct inode *in = ext2_lookup(dir, name);
    if (!in)
        return -ENOENT;
    int err = 0;
    mutex_lock(&in->lock);
    if (!dir_is_empty(in))
        err = -ENOTEMPTY;
    mutex_unlock(&in->lock);
    if (!err)
        err = dir_remove(dir, name);
    if (!err) {
        mutex_lock(&in->lock);
        in->nlink = 0;                  /* "." va ota papkadagi nom - ikkalasi ham yo'q */
        write_inode(in);
        mutex_unlock(&in->lock);
        dir->nlink--;                   /* bolaning ".." si yo'qoldi */
        write_inode(dir);
    }
    iput(in);
    return err;
}

/* anc papka dir ning ajdodimi (yoki o'zimi)? ".." bo'ylab ildizgacha yuramiz. */
static bool is_ancestor(struct ext2_fs *fs, struct inode *anc, struct inode *dir)
{
    uint32_t ino = (uint32_t)dir->ino;
    for (int guard = 0; guard < 4096; guard++) {
        if (ino == anc->ino)
            return true;
        if (ino == EXT2_ROOT_INO)
            return false;
        struct inode *d = ext2_iget(fs, ino);
        if (!d)
            return false;
        uint32_t parent = dir_find(d, "..");
        iput(d);
        if (!parent || parent == ino)
            return false;
        ino = parent;
    }
    return true;                        /* juda chuqur - xavfsiz tomonga */
}

static int ext2_rename(struct inode *odir, const char *oname, struct inode *ndir,
                       const char *nname)
{
    struct ext2_fs *fs = FS(odir);
    struct inode *src = ext2_lookup(odir, oname);
    if (!src)
        return -ENOENT;
    bool is_dir = S_ISDIR(src->mode);
    int err = 0;
    mutex_lock(&fs->rename_lock);
    struct inode *dst = ext2_lookup(ndir, nname);
    if (dst == src)
        goto out;                       /* o'ziga - hech narsa qilmaymiz */
    if (is_dir && odir != ndir && is_ancestor(fs, src, ndir)) {
        err = -EINVAL;                  /* papkani o'z ichiga ko'chirib bo'lmaydi */
        goto out;
    }
    if (dst) {
        if (S_ISDIR(dst->mode) != is_dir) {
            err = S_ISDIR(dst->mode) ? -EISDIR : -ENOTDIR;
            goto out;
        }
        if (is_dir) {
            mutex_lock(&dst->lock);
            bool empty = dir_is_empty(dst);
            mutex_unlock(&dst->lock);
            if (!empty) {
                err = -ENOTEMPTY;
                goto out;
            }
        }
        if ((err = dir_remove(ndir, nname)) != 0)
            goto out;
        mutex_lock(&dst->lock);
        dst->nlink = is_dir ? 0 : dst->nlink - 1;
        write_inode(dst);
        mutex_unlock(&dst->lock);
        if (is_dir) {
            ndir->nlink--;
            write_inode(ndir);
        }
    }
    /* Avval YANGI nom, keyin eskisini o'chiramiz: oraliqda tok o'chsa, fayl
     * ikki nomga ega bo'ladi (e2fsck tuzatadi), lekin yo'qolmaydi. */
    if ((err = dir_add(ndir, nname, (uint32_t)src->ino, mode_to_ft(src->mode))) != 0)
        goto out;
    if ((err = dir_remove(odir, oname)) != 0)
        goto out;
    if (is_dir && odir != ndir) {
        uint32_t parent = (uint32_t)ndir->ino;
        mutex_lock(&src->lock);
        dir_iterate(src, dotdot_visit, &parent);
        mutex_unlock(&src->lock);
        odir->nlink--;
        ndir->nlink++;
        write_inode(odir);
        write_inode(ndir);
    }
    src->ctime = time_now();
    write_inode(src);
out:
    mutex_unlock(&fs->rename_lock);
    iput(dst);
    iput(src);
    return err;
}

/* VFS: oxirgi referens yo'qoldi. nlink = 0 bo'lsa - diskdan ham o'chiramiz. */
static void ext2_free_inode(struct inode *in)
{
    struct ext2_node *n = NODE(in);
    if (!n)
        return;
    struct ext2_fs *fs = FS(in);
    spin_lock(&fs->icache_lock);
    list_del(&n->node);
    spin_unlock(&fs->icache_lock);
    if (in->nlink == 0 && !fs->vsb.read_only) {
        bool is_dir = S_ISDIR(in->mode);
        mutex_lock(&in->lock);
        if (S_ISREG(in->mode) || is_dir)
            ext2_truncate(in, 0);
        n->raw.i_dtime = (uint32_t)time_now();  /* e2fsck: "o'chirilgan inode" belgisi */
        write_inode(in);
        mutex_unlock(&in->lock);
        free_inode_nr(fs, (uint32_t)in->ino, is_dir);
    }
    kfree(n);
    in->priv = NULL;
}

static int ext2_sync(struct super_block *vsb)
{
    struct ext2_fs *fs = vsb->priv;
    if (vsb->read_only)
        return 0;
    mutex_lock(&fs->alloc_lock);
    int r = write_super(fs);            /* qolgan hamma narsa allaqachon yozilgan */
    mutex_unlock(&fs->alloc_lock);
    return r;
}

static const struct inode_ops ext2_dir_iops = {
    .lookup = ext2_lookup,
    .create = ext2_create,
    .mkdir = ext2_mkdir,
    .unlink = ext2_unlink,
    .rmdir = ext2_rmdir,
    .rename = ext2_rename,
    .readdir = ext2_readdir,
};

static const struct inode_ops ext2_file_iops = {
    .truncate = ext2_truncate,
};

static const struct file_ops ext2_file_fops = {
    .read = ext2_read,
    .write = ext2_write,
    .seekable = true,
};

static const struct super_ops ext2_sops = {
    .free_inode = ext2_free_inode,
    .sync = ext2_sync,
};

/* ============================================================================
 *  Ulash (mount)
 * ========================================================================== */

static int ext2_mount(struct block_device *bd, struct super_block **out)
{
    if (!bd)
        return -ENXIO;
    struct ext2_fs *fs = kzalloc(sizeof(*fs));
    if (!fs)
        return -ENOMEM;
    /* Superblok: bayt 1024 dan (2-3 sektorlar). Hali blok hajmini bilmaymiz -
     * keshni chetlab to'g'ridan-to'g'ri o'qiymiz. */
    if (blk_read(bd, 2, 2, &fs->sb)) {
        kfree(fs);
        return -EIO;
    }
    struct ext2_super *s = &fs->sb;
    int err = -EINVAL;
    if (s->s_magic != EXT2_MAGIC) {
        kprintf("[ext2] %s: ext2 emas (magic 0x%x)\n", bd->name, s->s_magic);
        goto fail;
    }
    fs->bd = bd;
    fs->block_size = 1024u << s->s_log_block_size;
    fs->inode_size = s->s_rev_level >= 1 ? s->s_inode_size : 128;
    if (fs->block_size > 4096 || fs->inode_size < 128 || fs->inode_size > fs->block_size ||
        !s->s_blocks_per_group || !s->s_inodes_per_group) {
        kprintf("[ext2] %s: qo'llanmaydigan parametrlar (blok %u, inode %u)\n", bd->name,
                fs->block_size, fs->inode_size);
        goto fail;
    }
    uint32_t incompat = s->s_rev_level >= 1 ? s->s_feature_incompat : 0;
    uint32_t ro_compat = s->s_rev_level >= 1 ? s->s_feature_ro_compat : 0;
    if (incompat & ~(uint32_t)INCOMPAT_FILETYPE) {
        /* Masalan ext4 extentlari (0x40) yoki ext3 jurnali tiklanishi kerak (0x4). */
        kprintf("[ext2] %s: noma'lum INCOMPAT xususiyatlar 0x%x - ulab bo'lmaydi "
                "(ext3/ext4?)\n", bd->name, incompat);
        goto fail;
    }
    fs->vsb.read_only = (ro_compat & ~(uint32_t)(RO_COMPAT_SPARSE | RO_COMPAT_LARGE_FILE)) != 0;
    if (fs->vsb.read_only)
        kprintf("[ext2] %s: RO_COMPAT 0x%x - faqat o'qish uchun\n", bd->name, ro_compat);
    if (s->s_rev_level == 0)
        s->s_first_ino = 11;
    fs->addr_per_block = fs->block_size / 4;
    fs->groups = (s->s_blocks_count - s->s_first_data_block + s->s_blocks_per_group - 1) /
                 s->s_blocks_per_group;
    size_t gd_bytes = fs->groups * sizeof(struct ext2_group_desc);
    fs->gd_blocks = (uint32_t)((gd_bytes + fs->block_size - 1) / fs->block_size);
    fs->gd = kmalloc(fs->gd_blocks * fs->block_size);
    if (!fs->gd) {
        err = -ENOMEM;
        goto fail;
    }
    for (uint32_t i = 0; i < fs->gd_blocks; i++) {
        struct buf *b = fs_bread(fs, s->s_first_data_block + 1 + i);
        if (!b) {
            err = -EIO;
            goto fail;
        }
        memcpy((uint8_t *)fs->gd + i * fs->block_size, b->data, fs->block_size);
        brelse(b);
    }
    mutex_init(&fs->alloc_lock, "ext2-alloc");
    mutex_init(&fs->rename_lock, "ext2-rename");
    fs->icache_lock = (spinlock_t)SPINLOCK_INIT("ext2-icache");
    list_init(&fs->icache);
    fs->vsb.fs_name = "ext2";
    fs->vsb.dev = bd->dev;
    fs->vsb.bdev = bd;
    fs->vsb.ops = &ext2_sops;
    fs->vsb.priv = fs;
    fs->vsb.root = ext2_iget(fs, EXT2_ROOT_INO);
    if (!fs->vsb.root || !S_ISDIR(fs->vsb.root->mode)) {
        kprintf("[ext2] %s: ildiz papka o'qilmadi\n", bd->name);
        err = -EIO;
        goto fail;
    }
    if (s->s_state != 1)
        kprintf("[ext2] %s: OGOHLANTIRISH - toza o'chirilmagan, e2fsck tavsiya etiladi\n",
                bd->name);
    if (!fs->vsb.read_only) {
        s->s_mnt_count++;
        s->s_mtime = (uint32_t)time_now();
        mutex_lock(&fs->alloc_lock);
        write_super(fs);
        mutex_unlock(&fs->alloc_lock);
    }
    kprintf("[ext2] %s: \"%.16s\", %u KB blok, %u guruh, %u/%u inode, %u MB bo'sh\n", bd->name,
            s->s_volume_name, fs->block_size / 1024, fs->groups,
            s->s_inodes_count - s->s_free_inodes_count, s->s_inodes_count,
            (uint32_t)((uint64_t)s->s_free_blocks_count * fs->block_size >> 20));
    *out = &fs->vsb;
    return 0;
fail:
    kfree(fs->gd);
    kfree(fs);
    return err;
}

static struct fs_type ext2_type = {
    .name = "ext2",
    .mount = ext2_mount,
};

void ext2_init(void)
{
    vfs_register_fs(&ext2_type);
}
