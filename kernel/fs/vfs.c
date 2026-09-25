/* =============================================================================
 *  fs/vfs.c - VFS: inode'lar, fayllar, yo'llar, mount
 * =============================================================================
 *
 *  VFS NIMA UCHUN KERAK:
 *    Dastur open("/mnt/disk/a.txt") qiladi. U fayl qaysi fayl tizimida (tmpfs,
 *    ext2, FAT...) ekanini bilmaydi va bilishi SHART EMAS. VFS - bu "adapter":
 *    umumiy amallar (open, read, mkdir...) va har bir fayl tizimi o'zi beradigan
 *    funksiyalar jadvali (inode_ops, file_ops). C'dagi "polimorfizm".
 *
 *       syscall open/read/write
 *              │
 *            VFS  ── yo'l bo'ylab yurish, mount'lar, fd'lar, refcount
 *        ┌─────┼──────────┬───────────┐
 *      tmpfs  ext2     devfs(/dev)  pipe
 *       RAM   disk     drayverlar   xotira
 *
 *  ASOSIY OBYEKTLAR:
 *    inode - fayl/papkaning O'ZI (hajm, turi, ma'lumot qayerda). Nomi YO'Q!
 *            Nom papkada saqlanadi ("a.txt" -> inode #12). Shu tufayli bitta
 *            faylning bir nechta nomi (hard link) bo'lishi mumkin.
 *    file  - OCHIQ fayl: qaysi inode, qayerdan o'qiyapmiz (offset), rejim.
 *            Ikki jarayon bir faylni alohida ochsa - ikkita file, bitta inode.
 *    fd    - jarayonning files[] massividagi indeks -> file.
 *
 *  REFCOUNT: inode'ni kim ushlab turgan bo'lsa (ochiq fayl, joriy papka,
 *  papka yozuvi) - refcount++. Oxirgisi qo'yib yuborganda va hech bir papkada
 *  nomi qolmaganda (nlink == 0) - fayl haqiqatan o'chiriladi. Shuning uchun
 *  Unix'da ochiq faylni o'chirsangiz, u yopilguncha ishlashda davom etadi.
 * ============================================================================= */
#include "fs/vfs.h"

#include "drivers/rtc.h"
#include "fs/block.h"
#include "lib/common.h"
#include "lib/kprintf.h"
#include "lib/panic.h"
#include "lib/string.h"
#include "mm/slab.h"
#include "mm/vmalloc.h"

static struct kmem_cache *inode_cache;
static struct kmem_cache *file_cache;
static struct inode *root_inode;
static LIST_HEAD(fs_types);
static const struct file_ops *chrdevs[256];

struct mount {
    char target[PATH_MAX];
    struct super_block *sb;
    struct inode *mountpoint;
    struct list_head node;
};
static LIST_HEAD(mounts);
static struct mutex mount_lock = MUTEX_INIT("mount");

/* ---- Inode ------------------------------------------------------------------ */

struct inode *inode_alloc(struct super_block *sb)
{
    if (!inode_cache)
        inode_cache = kmem_cache_create("inode", sizeof(struct inode), 8, NULL);
    struct inode *in = kmem_cache_alloc(inode_cache);
    if (!in)
        return NULL;
    memset(in, 0, sizeof(*in));
    in->refcount = 1;
    in->sb = sb;
    in->atime = in->mtime = in->ctime = time_now();
    mutex_init(&in->lock, "inode");
    return in;
}

struct inode *iget(struct inode *inode)
{
    if (inode)
        __atomic_add_fetch(&inode->refcount, 1, __ATOMIC_RELAXED);
    return inode;
}

void iput(struct inode *inode)
{
    if (!inode)
        return;
    int n = __atomic_sub_fetch(&inode->refcount, 1, __ATOMIC_ACQ_REL);
    if (n > 0)
        return;
    if (n < 0)
        panic("iput: inode %lu refcount manfiy", inode->ino);
    /* Hech kim ishlatmayapti. Fayl tizimi qaror qiladi: nlink == 0 bo'lsa
     * ma'lumotni o'chiradi, aks holda faqat xotiradagi nusxani (keshdan). */
    if (inode->sb && inode->sb->ops && inode->sb->ops->free_inode)
        inode->sb->ops->free_inode(inode);
    kmem_cache_free(inode_cache, inode);
}

/* ---- Belgili qurilmalar ----------------------------------------------------- */

void chrdev_register(uint32_t major, const struct file_ops *fops, const char *name)
{
    if (major < 256)
        chrdevs[major] = fops;
    kprintf("[vfs]  char qurilma %u: %s\n", major, name);
}

const struct file_ops *chrdev_get(uint32_t major)
{
    return major < 256 ? chrdevs[major] : NULL;
}

/* ---- Fayl ------------------------------------------------------------------- */

struct file *file_alloc(struct inode *inode, uint32_t flags)
{
    if (!file_cache)
        file_cache = kmem_cache_create("file", sizeof(struct file), 8, NULL);
    struct file *f = kmem_cache_alloc(file_cache);
    if (!f)
        return NULL;
    memset(f, 0, sizeof(*f));
    f->inode = inode;
    f->flags = flags;
    f->refcount = 1;
    mutex_init(&f->pos_lock, "file-pos");
    if (inode) {
        if (S_ISCHR(inode->mode))
            f->fops = chrdev_get(MAJOR(inode->rdev));
        else if (S_ISBLK(inode->mode))
            f->fops = blkdev_file_ops();
        else
            f->fops = inode->fops;
    }
    return f;
}

struct file *file_dup(struct file *f)
{
    if (f)
        __atomic_add_fetch(&f->refcount, 1, __ATOMIC_RELAXED);
    return f;
}

void file_close(struct file *f)
{
    if (!f)
        return;
    if (__atomic_sub_fetch(&f->refcount, 1, __ATOMIC_ACQ_REL) > 0)
        return;
    if (f->fops && f->fops->release)
        f->fops->release(f);
    iput(f->inode);
    kmem_cache_free(file_cache, f);
}

static bool can_read(const struct file *f)
{
    return (f->flags & O_ACCMODE) != O_WRONLY;
}

static bool can_write(const struct file *f)
{
    return (f->flags & O_ACCMODE) != O_RDONLY;
}

int64_t file_read(struct file *f, void *buf, size_t len)
{
    if (!can_read(f))
        return -EBADF;
    if (f->inode && S_ISDIR(f->inode->mode))
        return -EISDIR;
    if (!f->fops || !f->fops->read)
        return -EINVAL;
    if (!f->fops->seekable)             /* pipe/terminal: offset yo'q, bloklanishi mumkin */
        return f->fops->read(f, buf, len, 0);
    mutex_lock(&f->pos_lock);
    int64_t n = f->fops->read(f, buf, len, f->offset);
    if (n > 0)
        f->offset += (uint64_t)n;
    mutex_unlock(&f->pos_lock);
    return n;
}

int64_t file_write(struct file *f, const void *buf, size_t len)
{
    if (!can_write(f))
        return -EBADF;
    if (!f->fops || !f->fops->write)
        return -EINVAL;
    if (!f->fops->seekable)
        return f->fops->write(f, buf, len, 0);
    mutex_lock(&f->pos_lock);
    if (f->flags & O_APPEND)
        f->offset = f->inode->size;     /* har doim oxiriga */
    int64_t n = f->fops->write(f, buf, len, f->offset);
    if (n > 0)
        f->offset += (uint64_t)n;
    mutex_unlock(&f->pos_lock);
    return n;
}

int64_t file_lseek(struct file *f, int64_t off, int whence)
{
    if (!f->fops || !f->fops->seekable)
        return -ESPIPE;
    mutex_lock(&f->pos_lock);
    int64_t base = whence == SEEK_SET ? 0
                 : whence == SEEK_CUR ? (int64_t)f->offset
                 : whence == SEEK_END ? (int64_t)f->inode->size : -1;
    int64_t r;
    if (base < 0 || base + off < 0) {
        r = -EINVAL;
    } else {
        f->offset = (uint64_t)(base + off);
        r = (int64_t)f->offset;
    }
    mutex_unlock(&f->pos_lock);
    return r;
}

int vfs_readdir(struct file *f, struct myos_dirent *out)
{
    struct inode *dir = f->inode;
    if (!dir || !S_ISDIR(dir->mode))
        return -ENOTDIR;
    if (!dir->iops || !dir->iops->readdir)
        return -EINVAL;
    mutex_lock(&dir->lock);
    int r = dir->iops->readdir(dir, &f->offset, out);
    mutex_unlock(&dir->lock);
    return r;
}

/* ---- Yo'l bo'ylab yurish ---------------------------------------------------- */

/* s dan keyingi komponentni name ga olish. Qaytaradi: keyingi pozitsiya yoki NULL. */
static const char *next_component(const char *s, char *name, int *err)
{
    while (*s == '/')
        s++;
    if (!*s)
        return NULL;
    size_t n = 0;
    while (s[n] && s[n] != '/') {
        if (n >= NAME_MAX) {
            *err = -ENAMETOOLONG;
            return NULL;
        }
        name[n] = s[n];
        n++;
    }
    name[n] = '\0';
    return s + n;
}

/* Mount nuqtasidan o'tish: papkaga FS ulangan bo'lsa, uning ildiziga. */
static struct inode *follow_mounts(struct inode *in)
{
    while (in->mounted) {
        struct inode *m = iget(in->mounted);
        iput(in);
        in = m;
    }
    return in;
}

static int step(struct inode **cur, const char *name)
{
    struct inode *dir = *cur;
    if (!S_ISDIR(dir->mode))
        return -ENOTDIR;
    if (strcmp(name, ".") == 0)
        return 0;
    if (strcmp(name, "..") == 0) {
        if (dir == root_inode)
            return 0;                   /* ildizdan yuqori yo'q */
        if (dir->covered) {             /* ulangan FS ildizidan chiqish */
            struct inode *c = iget(dir->covered);
            iput(dir);
            *cur = dir = c;
        }
    }
    if (!dir->iops || !dir->iops->lookup)
        return -ENOTDIR;
    mutex_lock(&dir->lock);
    struct inode *child = dir->iops->lookup(dir, name);
    mutex_unlock(&dir->lock);
    if (!child)
        return -ENOENT;
    iput(dir);
    *cur = follow_mounts(child);
    return 0;
}

static struct inode *start_inode(const char *path, struct inode *cwd)
{
    if (path[0] == '/' || !cwd)
        return iget(root_inode);
    return iget(cwd);
}

int vfs_lookup(const char *path, struct inode *cwd, struct inode **out)
{
    if (!path || !*path)
        return -ENOENT;
    if (strnlen(path, PATH_MAX) >= PATH_MAX)
        return -ENAMETOOLONG;
    struct inode *cur = start_inode(path, cwd);
    char name[NAME_MAX + 1];
    const char *s = path;
    int err = 0;
    while ((s = next_component(s, name, &err)) != NULL) {
        err = step(&cur, name);
        if (err) {
            iput(cur);
            return err;
        }
    }
    if (err) {
        iput(cur);
        return err;
    }
    *out = cur;
    return 0;
}

int vfs_lookup_parent(const char *path, struct inode *cwd, struct inode **parent, char *last)
{
    if (!path || !*path)
        return -ENOENT;
    if (strnlen(path, PATH_MAX) >= PATH_MAX)
        return -ENAMETOOLONG;
    struct inode *cur = start_inode(path, cwd);
    char name[NAME_MAX + 1];
    const char *s = path;
    int err = 0;
    last[0] = '\0';
    while ((s = next_component(s, name, &err)) != NULL) {
        /* Oxirgi komponentmi? (keyin faqat '/' lar qolgan) */
        const char *rest = s;
        while (*rest == '/')
            rest++;
        if (!*rest) {
            strlcpy(last, name, NAME_MAX + 1);
            break;
        }
        err = step(&cur, name);
        if (err) {
            iput(cur);
            return err;
        }
    }
    if (err || !last[0] || strcmp(last, ".") == 0 || strcmp(last, "..") == 0) {
        iput(cur);
        return err ? err : -EINVAL;
    }
    if (!S_ISDIR(cur->mode)) {
        iput(cur);
        return -ENOTDIR;
    }
    *parent = cur;
    return 0;
}

/* ---- Yuqori darajadagi amallar ---------------------------------------------- */

int vfs_open(const char *path, struct inode *cwd, uint32_t flags, uint32_t mode, struct file **out)
{
    struct inode *inode = NULL;
    int err;
    if (flags & O_CREAT) {
        struct inode *dir;
        char name[NAME_MAX + 1];
        err = vfs_lookup_parent(path, cwd, &dir, name);
        if (err)
            return err;
        if (dir->sb && dir->sb->read_only) {
            iput(dir);
            return -EROFS;
        }
        mutex_lock(&dir->lock);
        inode = dir->iops->lookup(dir, name);
        if (inode && (flags & O_EXCL)) {
            iput(inode);
            inode = NULL;
            err = -EEXIST;
        } else if (!inode) {
            err = dir->iops->create ? dir->iops->create(dir, name, S_IFREG | (mode & 07777), &inode)
                                    : -EPERM;
        }
        mutex_unlock(&dir->lock);
        iput(dir);
        if (err)
            return err;
        inode = follow_mounts(inode);
    } else {
        err = vfs_lookup(path, cwd, &inode);
        if (err)
            return err;
    }

    bool writing = (flags & O_ACCMODE) != O_RDONLY;
    if (S_ISDIR(inode->mode) && writing) {
        iput(inode);
        return -EISDIR;
    }
    if ((flags & O_DIRECTORY) && !S_ISDIR(inode->mode)) {
        iput(inode);
        return -ENOTDIR;
    }
    if (writing && inode->sb && inode->sb->read_only && S_ISREG(inode->mode)) {
        iput(inode);
        return -EROFS;
    }
    if ((flags & O_TRUNC) && writing && S_ISREG(inode->mode) && inode->iops &&
        inode->iops->truncate) {
        mutex_lock(&inode->lock);
        err = inode->iops->truncate(inode, 0);
        mutex_unlock(&inode->lock);
        if (err) {
            iput(inode);
            return err;
        }
    }
    struct file *f = file_alloc(inode, flags);  /* inode referensi endi file'niki */
    if (!f) {
        iput(inode);
        return -ENOMEM;
    }
    if (S_ISCHR(inode->mode) && !f->fops) {
        file_close(f);
        return -ENXIO;                  /* bunday drayver yo'q */
    }
    if (f->fops && f->fops->open) {
        err = f->fops->open(inode, f);
        if (err) {
            f->fops = NULL;             /* release chaqirilmasin */
            file_close(f);
            return err;
        }
    }
    *out = f;
    return 0;
}

/* Parent papkada amal bajarish uchun umumiy yordamchi. */
static int with_parent(const char *path, struct inode *cwd, struct inode **dir, char *name)
{
    int err = vfs_lookup_parent(path, cwd, dir, name);
    if (err)
        return err;
    if ((*dir)->sb && (*dir)->sb->read_only) {
        iput(*dir);
        return -EROFS;
    }
    return 0;
}

int vfs_mkdir(const char *path, struct inode *cwd, uint32_t mode)
{
    struct inode *dir;
    char name[NAME_MAX + 1];
    int err = with_parent(path, cwd, &dir, name);
    if (err)
        return err;
    mutex_lock(&dir->lock);
    struct inode *ex = dir->iops->lookup(dir, name);
    if (ex) {
        iput(ex);
        err = -EEXIST;
    } else {
        err = dir->iops->mkdir ? dir->iops->mkdir(dir, name, S_IFDIR | (mode & 07777)) : -EPERM;
    }
    mutex_unlock(&dir->lock);
    iput(dir);
    return err;
}

int vfs_mknod(const char *path, struct inode *cwd, uint32_t mode, uint32_t rdev)
{
    struct inode *dir;
    char name[NAME_MAX + 1];
    int err = with_parent(path, cwd, &dir, name);
    if (err)
        return err;
    mutex_lock(&dir->lock);
    struct inode *ex = dir->iops->lookup(dir, name);
    if (ex) {
        iput(ex);
        err = -EEXIST;
    } else {
        err = dir->iops->mknod ? dir->iops->mknod(dir, name, mode, rdev) : -EPERM;
    }
    mutex_unlock(&dir->lock);
    iput(dir);
    return err;
}

int vfs_unlink(const char *path, struct inode *cwd)
{
    struct inode *dir;
    char name[NAME_MAX + 1];
    int err = with_parent(path, cwd, &dir, name);
    if (err)
        return err;
    mutex_lock(&dir->lock);
    struct inode *victim = dir->iops->lookup(dir, name);
    if (!victim)
        err = -ENOENT;
    else if (S_ISDIR(victim->mode))
        err = -EISDIR;
    else if (victim->mounted)
        err = -EBUSY;
    else
        err = dir->iops->unlink ? dir->iops->unlink(dir, name) : -EPERM;
    mutex_unlock(&dir->lock);
    iput(victim);
    iput(dir);
    return err;
}

int vfs_rmdir(const char *path, struct inode *cwd)
{
    struct inode *dir;
    char name[NAME_MAX + 1];
    int err = with_parent(path, cwd, &dir, name);
    if (err)
        return err;
    mutex_lock(&dir->lock);
    struct inode *victim = dir->iops->lookup(dir, name);
    if (!victim)
        err = -ENOENT;
    else if (!S_ISDIR(victim->mode))
        err = -ENOTDIR;
    else if (victim->mounted)
        err = -EBUSY;                   /* mount nuqtasini o'chirib bo'lmaydi */
    else
        err = dir->iops->rmdir ? dir->iops->rmdir(dir, name) : -EPERM;
    mutex_unlock(&dir->lock);
    iput(victim);
    iput(dir);
    return err;
}

int vfs_rename(const char *from, const char *to, struct inode *cwd)
{
    struct inode *odir, *ndir;
    char oname[NAME_MAX + 1], nname[NAME_MAX + 1];
    int err = with_parent(from, cwd, &odir, oname);
    if (err)
        return err;
    err = with_parent(to, cwd, &ndir, nname);
    if (err) {
        iput(odir);
        return err;
    }
    if (odir->sb != ndir->sb) {
        err = -EXDEV;                   /* fayl tizimlari orasida - faqat nusxalash */
    } else if (!odir->iops->rename) {
        err = -EPERM;
    } else {
        /* Ikkita qulf: deadlock bo'lmasligi uchun doim MANZIL tartibida olamiz. */
        struct inode *a = odir < ndir ? odir : ndir, *b = odir < ndir ? ndir : odir;
        mutex_lock(&a->lock);
        if (b != a)
            mutex_lock(&b->lock);
        err = odir->iops->rename(odir, oname, ndir, nname);
        if (b != a)
            mutex_unlock(&b->lock);
        mutex_unlock(&a->lock);
    }
    iput(odir);
    iput(ndir);
    return err;
}

int vfs_stat(struct inode *in, struct myos_stat *st)
{
    memset(st, 0, sizeof(*st));
    st->st_ino = in->ino;
    st->st_mode = in->mode;
    st->st_nlink = in->nlink;
    st->st_size = in->size;
    st->st_atime = in->atime;
    st->st_mtime = in->mtime;
    st->st_ctime = in->ctime;
    st->st_dev = in->sb ? in->sb->dev : 0;
    st->st_rdev = in->rdev;
    st->st_uid = in->uid;
    st->st_gid = in->gid;
    st->st_blksize = 4096;
    st->st_blocks = (in->size + 511) / 512;
    return 0;
}

int vfs_read_whole(const char *path, struct inode *cwd, void **data, size_t *size)
{
    struct file *f;
    int err = vfs_open(path, cwd, O_RDONLY, 0, &f);
    if (err)
        return err;
    if (!S_ISREG(f->inode->mode)) {
        file_close(f);
        return -EACCES;
    }
    size_t sz = f->inode->size;
    if (sz > 64 * 1024 * 1024) {
        file_close(f);
        return -EFBIG;
    }
    uint8_t *buf = vmalloc(sz ? sz : 1);
    if (!buf) {
        file_close(f);
        return -ENOMEM;
    }
    size_t got = 0;
    while (got < sz) {
        int64_t n = f->fops->read(f, buf + got, sz - got, got);
        if (n <= 0)
            break;
        got += (size_t)n;
    }
    file_close(f);
    if (got != sz) {
        vfree(buf);
        return -EIO;
    }
    *data = buf;
    *size = sz;
    return 0;
}

/* ---- Mount ------------------------------------------------------------------ */

void vfs_register_fs(struct fs_type *fs)
{
    list_add_tail(&fs->node, &fs_types);
}

void vfs_mount_root(struct super_block *sb)
{
    root_inode = iget(sb->root);
    struct mount *m = kzalloc(sizeof(*m));
    strlcpy(m->target, "/", sizeof(m->target));
    m->sb = sb;
    list_add_tail(&m->node, &mounts);
}

struct inode *vfs_root(void)
{
    return root_inode;
}

int vfs_mount(const char *source, const char *target, const char *fstype)
{
    struct fs_type *fs = NULL, *it;
    list_for_each_entry(it, &fs_types, node)
        if (strcmp(it->name, fstype) == 0)
            fs = it;
    if (!fs)
        return -ENODEV;
    struct block_device *bdev = NULL;
    if (source && *source && strcmp(source, "none") != 0) {
        const char *name = strncmp(source, "/dev/", 5) == 0 ? source + 5 : source;
        bdev = blkdev_find(name);
        if (!bdev)
            return -ENXIO;
    }
    struct inode *mp;
    int err = vfs_lookup(target, NULL, &mp);
    if (err)
        return err;
    if (!S_ISDIR(mp->mode)) {
        iput(mp);
        return -ENOTDIR;
    }
    mutex_lock(&mount_lock);
    if (mp->mounted || mp == root_inode) {
        mutex_unlock(&mount_lock);
        iput(mp);
        return -EBUSY;
    }
    struct super_block *sb;
    err = fs->mount(bdev, &sb);
    if (err) {
        mutex_unlock(&mount_lock);
        iput(mp);
        return err;
    }
    struct mount *m = kzalloc(sizeof(*m));
    strlcpy(m->target, target, sizeof(m->target));
    m->sb = sb;
    m->mountpoint = mp;                 /* referens mount'da qoladi */
    mp->mounted = iget(sb->root);
    sb->root->covered = iget(mp);
    list_add_tail(&m->node, &mounts);
    mutex_unlock(&mount_lock);
    kprintf("[vfs]  %s (%s) -> %s ulandi\n", source ? source : "none", fstype, target);
    return 0;
}

int vfs_umount(const char *target)
{
    struct inode *in;
    int err = vfs_lookup(target, NULL, &in);
    if (err)
        return err;
    mutex_lock(&mount_lock);
    struct mount *m, *found = NULL;
    list_for_each_entry(m, &mounts, node)
        if (m->mountpoint && m->sb->root == in)
            found = m;
    if (!found) {
        mutex_unlock(&mount_lock);
        iput(in);
        return -EINVAL;
    }
    /* Faqat bizning va mount'ning referenslari qolgan bo'lishi kerak. */
    if (in->refcount > 3) {
        mutex_unlock(&mount_lock);
        iput(in);
        return -EBUSY;
    }
    if (found->sb->ops && found->sb->ops->sync)
        found->sb->ops->sync(found->sb);
    found->mountpoint->mounted = NULL;
    iput(in);                           /* mounted referensi */
    in->covered = NULL;
    iput(found->mountpoint);            /* covered referensi */
    iput(found->mountpoint);            /* mount'ning referensi */
    list_del(&found->node);
    mutex_unlock(&mount_lock);
    iput(in);                           /* lookup referensi */
    kfree(found);
    return 0;
}

int vfs_sync(void)
{
    mutex_lock(&mount_lock);
    struct mount *m;
    list_for_each_entry(m, &mounts, node)
        if (m->sb->ops && m->sb->ops->sync)
            m->sb->ops->sync(m->sb);
    mutex_unlock(&mount_lock);
    return 0;
}

/* ---- Yo'lni normallashtirish ------------------------------------------------ */

int path_normalize(const char *cwd, const char *path, char *out, size_t size)
{
    /* >>> LAB path_normalize - vazifa: labs/README.md */
    char buf[PATH_MAX * 2];
    if (path[0] == '/')
        strlcpy(buf, path, sizeof(buf));
    else
        ksnprintf(buf, sizeof(buf), "%s/%s", cwd, path);

    /* Komponentlar stekini quramiz: "." - tashlaymiz, ".." - oxirgisini olib tashlaymiz. */
    size_t len = 0;
    out[0] = '\0';
    const char *s = buf;
    while (*s) {
        while (*s == '/')
            s++;
        if (!*s)
            break;
        const char *e = s;
        while (*e && *e != '/')
            e++;
        size_t n = (size_t)(e - s);
        if (n == 1 && s[0] == '.') {
            /* joriy papka */
        } else if (n == 2 && s[0] == '.' && s[1] == '.') {
            while (len > 0 && out[len - 1] != '/')
                len--;
            if (len > 0)
                len--;                  /* '/' ni ham */
            out[len] = '\0';
        } else {
            if (len + 1 + n + 1 > size)
                return -ENAMETOOLONG;
            out[len++] = '/';
            memcpy(out + len, s, n);
            len += n;
            out[len] = '\0';
        }
        s = e;
    }
    if (len == 0) {
        out[0] = '/';
        out[1] = '\0';
    }
    return 0;
    /* <<< LAB path_normalize */
}
