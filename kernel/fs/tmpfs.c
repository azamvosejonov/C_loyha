/* =============================================================================
 *  fs/tmpfs.c - TMPFS: butunlay RAM'da yashaydigan fayl tizimi
 * =============================================================================
 *
 *  Linux'da /tmp, /dev/shm va boot paytidagi initramfs ham shunday ishlaydi.
 *  Diskka yozilmaydi - kompyuter o'chsa yo'qoladi. Lekin juda tez va sodda,
 *  shuning uchun ROOT (/) fayl tizimimiz: boot paytida initrd.tar shu yerga
 *  ochiladi (fs/initrd.c).
 *
 *  TUZILMA:
 *    Papka  - bolalar ro'yxati: { nom -> inode } (tmpfs_dirent).
 *    Fayl   - sahifalar massivi: pages[i] = faylning i-chi 4 KB bo'lagi.
 *             NULL = "teshik" (hech narsa yozilmagan) - o'qilganda nollar.
 *             Shunday fayllar "sparse" (siyrak) deyiladi.
 *
 *  REFCOUNT: papka yozuvi inode'ga bitta referens ushlab turadi. unlink
 *  yozuvni o'chiradi va referensni qaytaradi - agar fayl hali ochiq bo'lsa,
 *  u yopilguncha yashaydi (VFS iput).
 * ============================================================================= */
#include "fs/tmpfs.h"

#include "drivers/rtc.h"
#include "lib/common.h"
#include "lib/string.h"
#include "mm/page.h"
#include "mm/pmm.h"
#include "mm/slab.h"

struct tmpfs_dirent {
    char name[NAME_MAX + 1];
    struct inode *inode;
    struct list_head node;
};

struct tmpfs_node {
    struct list_head children;          /* papka uchun */
    struct inode *parent;               /* ".." uchun (papka) */
    struct page **pages;                /* fayl uchun */
    size_t pages_cap;
};

static const struct inode_ops tmpfs_dir_ops;
static const struct inode_ops tmpfs_file_iops;
static const struct file_ops tmpfs_file_ops;
static const struct super_ops tmpfs_sops;
static uint64_t next_ino = 1;
static uint32_t next_dev = 1;

static struct tmpfs_node *NODE(struct inode *in)
{
    return in->priv;
}

static struct inode *tmpfs_new_inode(struct super_block *sb, uint32_t mode)
{
    struct inode *in = inode_alloc(sb);
    struct tmpfs_node *n = kzalloc(sizeof(*n));
    if (!in || !n) {
        if (in)
            iput(in);
        kfree(n);
        return NULL;
    }
    list_init(&n->children);
    in->priv = n;
    in->ino = __atomic_fetch_add(&next_ino, 1, __ATOMIC_RELAXED);
    in->mode = mode;
    in->nlink = 1;
    if (S_ISDIR(mode)) {
        in->iops = &tmpfs_dir_ops;
        in->nlink = 2;                  /* "." va ota papkadagi nom */
    } else {
        in->iops = &tmpfs_file_iops;
        in->fops = &tmpfs_file_ops;
    }
    return in;
}

/* ---- Fayl ma'lumoti ---- */

static void free_pages_from(struct tmpfs_node *n, size_t first)
{
    for (size_t i = first; i < n->pages_cap; i++) {
        if (n->pages[i]) {
            free_pages(n->pages[i], 0);
            n->pages[i] = NULL;
        }
    }
}

static int ensure_cap(struct tmpfs_node *n, size_t need)
{
    if (need <= n->pages_cap)
        return 0;
    size_t cap = n->pages_cap ? n->pages_cap : 4;
    while (cap < need)
        cap *= 2;
    struct page **np = kzalloc(cap * sizeof(*np));
    if (!np)
        return -ENOMEM;
    if (n->pages)
        memcpy(np, n->pages, n->pages_cap * sizeof(*np));
    kfree(n->pages);
    n->pages = np;
    n->pages_cap = cap;
    return 0;
}

static int64_t tmpfs_read(struct file *f, void *buf, size_t len, uint64_t off)
{
    struct inode *in = f->inode;
    mutex_lock(&in->lock);
    struct tmpfs_node *n = NODE(in);
    if (off >= in->size) {
        mutex_unlock(&in->lock);
        return 0;                       /* fayl oxiri */
    }
    len = MIN(len, in->size - off);
    size_t done = 0;
    while (done < len) {
        uint64_t pos = off + done;
        size_t idx = pos / PAGE_SIZE, inpage = pos % PAGE_SIZE;
        size_t chunk = MIN(len - done, PAGE_SIZE - inpage);
        struct page *pg = idx < n->pages_cap ? n->pages[idx] : NULL;
        if (pg)
            memcpy((uint8_t *)buf + done, (uint8_t *)page_to_virt(pg) + inpage, chunk);
        else
            memset((uint8_t *)buf + done, 0, chunk);    /* teshik - nollar */
        done += chunk;
    }
    in->atime = time_now();
    mutex_unlock(&in->lock);
    return (int64_t)done;
}

static int64_t tmpfs_write(struct file *f, const void *buf, size_t len, uint64_t off)
{
    struct inode *in = f->inode;
    if (off + len < off || off + len > (1UL << 36))
        return -EFBIG;
    mutex_lock(&in->lock);
    struct tmpfs_node *n = NODE(in);
    int err = ensure_cap(n, (off + len + PAGE_SIZE - 1) / PAGE_SIZE);
    if (err) {
        mutex_unlock(&in->lock);
        return err;
    }
    size_t done = 0;
    while (done < len) {
        uint64_t pos = off + done;
        size_t idx = pos / PAGE_SIZE, inpage = pos % PAGE_SIZE;
        size_t chunk = MIN(len - done, PAGE_SIZE - inpage);
        if (!n->pages[idx]) {
            n->pages[idx] = alloc_pages(0, GFP_ZERO);
            if (!n->pages[idx])
                break;                  /* xotira tugadi - qisman yozildi */
        }
        memcpy((uint8_t *)page_to_virt(n->pages[idx]) + inpage, (const uint8_t *)buf + done, chunk);
        done += chunk;
    }
    if (off + done > in->size)
        in->size = off + done;
    in->mtime = in->ctime = time_now();
    mutex_unlock(&in->lock);
    return done ? (int64_t)done : -ENOSPC;
}

/* inode->lock ushlangan holda (VFS). */
static int tmpfs_truncate(struct inode *in, uint64_t size)
{
    struct tmpfs_node *n = NODE(in);
    if (size < in->size) {
        free_pages_from(n, (size + PAGE_SIZE - 1) / PAGE_SIZE);
        /* Oxirgi sahifaning qolgan qismini nollaymiz (keyin kattalashsa - nol ko'rinsin). */
        size_t idx = size / PAGE_SIZE;
        if (size % PAGE_SIZE && idx < n->pages_cap && n->pages[idx])
            memset((uint8_t *)page_to_virt(n->pages[idx]) + size % PAGE_SIZE, 0,
                   PAGE_SIZE - size % PAGE_SIZE);
    }
    in->size = size;
    in->mtime = in->ctime = time_now();
    return 0;
}

/* ---- Papkalar (dir->lock ushlangan) ---- */

static struct tmpfs_dirent *find(struct inode *dir, const char *name)
{
    struct tmpfs_dirent *d;
    list_for_each_entry(d, &NODE(dir)->children, node)
        if (strcmp(d->name, name) == 0)
            return d;
    return NULL;
}

static struct inode *tmpfs_lookup(struct inode *dir, const char *name)
{
    if (strcmp(name, "..") == 0)
        return iget(NODE(dir)->parent ? NODE(dir)->parent : dir);
    struct tmpfs_dirent *d = find(dir, name);
    return d ? iget(d->inode) : NULL;
}

static int add_entry(struct inode *dir, const char *name, struct inode *child)
{
    struct tmpfs_dirent *d = kzalloc(sizeof(*d));
    if (!d)
        return -ENOMEM;
    strlcpy(d->name, name, sizeof(d->name));
    d->inode = child;                   /* yozuv referensni o'zlashtiradi */
    list_add_tail(&d->node, &NODE(dir)->children);
    dir->mtime = dir->ctime = time_now();
    return 0;
}

static int tmpfs_create(struct inode *dir, const char *name, uint32_t mode, struct inode **out)
{
    struct inode *in = tmpfs_new_inode(dir->sb, mode);
    if (!in)
        return -ENOMEM;
    int err = add_entry(dir, name, in);
    if (err) {
        in->nlink = 0;
        iput(in);
        return err;
    }
    *out = iget(in);                    /* chaqiruvchiga alohida referens */
    return 0;
}

static int tmpfs_mkdir(struct inode *dir, const char *name, uint32_t mode)
{
    struct inode *in = tmpfs_new_inode(dir->sb, mode);
    if (!in)
        return -ENOMEM;
    NODE(in)->parent = dir;             /* ota papka bola papkadan uzoqroq yashaydi */
    int err = add_entry(dir, name, in);
    if (err) {
        in->nlink = 0;
        iput(in);
        return err;
    }
    dir->nlink++;                       /* bolaning ".." si */
    return 0;
}

static int tmpfs_mknod(struct inode *dir, const char *name, uint32_t mode, uint32_t rdev)
{
    struct inode *in = tmpfs_new_inode(dir->sb, mode);
    if (!in)
        return -ENOMEM;
    in->rdev = rdev;
    in->fops = NULL;                    /* qurilma drayveri open paytida tanlanadi */
    int err = add_entry(dir, name, in);
    if (err) {
        in->nlink = 0;
        iput(in);
    }
    return err;
}

static int tmpfs_unlink(struct inode *dir, const char *name)
{
    struct tmpfs_dirent *d = find(dir, name);
    if (!d)
        return -ENOENT;
    list_del(&d->node);
    struct inode *in = d->inode;
    in->nlink--;
    in->ctime = time_now();
    kfree(d);
    iput(in);                           /* yozuvning referensi. Ochiq bo'lsa - yashaydi */
    return 0;
}

static int tmpfs_rmdir(struct inode *dir, const char *name)
{
    struct tmpfs_dirent *d = find(dir, name);
    if (!d)
        return -ENOENT;
    if (!list_empty(&NODE(d->inode)->children))
        return -ENOTEMPTY;
    list_del(&d->node);
    d->inode->nlink = 0;
    dir->nlink--;
    iput(d->inode);
    kfree(d);
    return 0;
}

static int tmpfs_rename(struct inode *odir, const char *oname, struct inode *ndir, const char *nname)
{
    struct tmpfs_dirent *d = find(odir, oname);
    if (!d)
        return -ENOENT;
    struct tmpfs_dirent *target = find(ndir, nname);
    if (target == d)
        return 0;
    if (target) {
        if (S_ISDIR(target->inode->mode) != S_ISDIR(d->inode->mode))
            return S_ISDIR(target->inode->mode) ? -EISDIR : -ENOTDIR;
        if (S_ISDIR(target->inode->mode) && !list_empty(&NODE(target->inode)->children))
            return -ENOTEMPTY;
        /* Mavjud nishonni almashtiramiz (POSIX: rename atomar ravishda ustidan yozadi). */
        list_del(&target->node);
        target->inode->nlink = S_ISDIR(target->inode->mode) ? 0 : target->inode->nlink - 1;
        iput(target->inode);
        kfree(target);
    }
    /* Papkani o'zining ichiga ko'chirishni oldini olish: ndir d ning avlodimi? */
    if (S_ISDIR(d->inode->mode)) {
        for (struct inode *p = ndir; p; p = NODE(p)->parent)
            if (p == d->inode)
                return -EINVAL;
    }
    list_del(&d->node);
    strlcpy(d->name, nname, sizeof(d->name));
    list_add_tail(&d->node, &NODE(ndir)->children);
    if (S_ISDIR(d->inode->mode) && odir != ndir) {
        NODE(d->inode)->parent = ndir;
        odir->nlink--;
        ndir->nlink++;
    }
    odir->mtime = ndir->mtime = time_now();
    return 0;
}

static int tmpfs_readdir(struct inode *dir, uint64_t *pos, struct myos_dirent *out)
{
    uint64_t i = 0;
    /* 0 - ".", 1 - "..", keyin bolalar. */
    if (*pos == 0) {
        out->d_ino = dir->ino;
        out->d_type = S_IFDIR;
        strlcpy(out->d_name, ".", sizeof(out->d_name));
        (*pos)++;
        return 1;
    }
    if (*pos == 1) {
        struct inode *p = NODE(dir)->parent ? NODE(dir)->parent : dir;
        out->d_ino = p->ino;
        out->d_type = S_IFDIR;
        strlcpy(out->d_name, "..", sizeof(out->d_name));
        (*pos)++;
        return 1;
    }
    struct tmpfs_dirent *d;
    list_for_each_entry(d, &NODE(dir)->children, node) {
        if (i++ == *pos - 2) {
            out->d_ino = d->inode->ino;
            out->d_type = d->inode->mode & S_IFMT;
            strlcpy(out->d_name, d->name, sizeof(out->d_name));
            (*pos)++;
            return 1;
        }
    }
    return 0;
}

static void tmpfs_free_inode(struct inode *in)
{
    struct tmpfs_node *n = NODE(in);
    if (!n)
        return;
    if (n->pages) {
        free_pages_from(n, 0);
        kfree(n->pages);
    }
    kfree(n);
    in->priv = NULL;
}

static const struct inode_ops tmpfs_dir_ops = {
    .lookup = tmpfs_lookup,
    .create = tmpfs_create,
    .mkdir = tmpfs_mkdir,
    .mknod = tmpfs_mknod,
    .unlink = tmpfs_unlink,
    .rmdir = tmpfs_rmdir,
    .rename = tmpfs_rename,
    .readdir = tmpfs_readdir,
};

static const struct inode_ops tmpfs_file_iops = {
    .truncate = tmpfs_truncate,
};

static const struct file_ops tmpfs_file_ops = {
    .read = tmpfs_read,
    .write = tmpfs_write,
    .seekable = true,
};

static const struct super_ops tmpfs_sops = {
    .free_inode = tmpfs_free_inode,
};

static int tmpfs_mount(struct block_device *bdev, struct super_block **out)
{
    (void)bdev;
    *out = tmpfs_create_sb();
    return *out ? 0 : -ENOMEM;
}

struct super_block *tmpfs_create_sb(void)
{
    struct super_block *sb = kzalloc(sizeof(*sb));
    if (!sb)
        return NULL;
    sb->fs_name = "tmpfs";
    sb->ops = &tmpfs_sops;
    sb->dev = MKDEV(0, __atomic_fetch_add(&next_dev, 1, __ATOMIC_RELAXED));
    sb->root = tmpfs_new_inode(sb, S_IFDIR | 0755);
    if (!sb->root) {
        kfree(sb);
        return NULL;
    }
    return sb;
}

static struct fs_type tmpfs_type = { .name = "tmpfs", .mount = tmpfs_mount };

void tmpfs_init(void)
{
    vfs_register_fs(&tmpfs_type);
}
