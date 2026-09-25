/* =============================================================================
 *  fs/vfs.h - VIRTUAL FILE SYSTEM: barcha fayl tizimlari uchun umumiy qatlam
 * ============================================================================= */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "lib/list.h"
#include "lib/mutex.h"
#include "myos/abi.h"

struct inode;
struct file;
struct super_block;
struct block_device;

/* Qurilma raqami: major (drayver turi, 12 bit) + minor (nusxa, 20 bit) - Linux kabi. */
#define MKDEV(ma, mi) (((uint32_t)(ma) << 20) | (uint32_t)(mi))
#define MAJOR(d)      ((d) >> 20)
#define MINOR(d)      ((d) & 0xFFFFF)

/* Inode ustidagi amallar (papka va nom bilan bog'liq). */
struct inode_ops {
    /* dir ichida name ni topish. Qaytaradi: REFERENCE OLINGAN inode yoki NULL. */
    struct inode *(*lookup)(struct inode *dir, const char *name);
    int (*create)(struct inode *dir, const char *name, uint32_t mode, struct inode **out);
    int (*mkdir)(struct inode *dir, const char *name, uint32_t mode);
    int (*mknod)(struct inode *dir, const char *name, uint32_t mode, uint32_t rdev);
    int (*unlink)(struct inode *dir, const char *name);
    int (*rmdir)(struct inode *dir, const char *name);
    int (*rename)(struct inode *odir, const char *oname, struct inode *ndir, const char *nname);
    int (*truncate)(struct inode *inode, uint64_t size);
    /* *pos - ichki pozitsiya. 1 = yozuv berildi, 0 = tugadi, <0 = xato. */
    int (*readdir)(struct inode *dir, uint64_t *pos, struct myos_dirent *out);
};

/* Ochiq fayl ustidagi amallar (o'qish/yozish). */
struct file_ops {
    int (*open)(struct inode *inode, struct file *f);
    void (*release)(struct file *f);
    int64_t (*read)(struct file *f, void *buf, size_t len, uint64_t off);
    int64_t (*write)(struct file *f, const void *buf, size_t len, uint64_t off);
    int (*ioctl)(struct file *f, uint64_t cmd, uint64_t arg);
    bool seekable;                      /* pipe va terminal - yo'q */
};

struct super_ops {
    void (*free_inode)(struct inode *inode);    /* nlink = 0 va refcount = 0 bo'lganda */
    int (*sync)(struct super_block *sb);
    void (*write_inode)(struct inode *inode);   /* metama'lumotni diskka */
};

struct super_block {
    const char *fs_name;
    uint32_t dev;
    struct inode *root;
    const struct super_ops *ops;
    struct block_device *bdev;
    void *priv;
    bool read_only;
};

struct inode {
    uint64_t ino;
    uint32_t mode;                      /* S_IFxxx | ruxsatlar */
    uint32_t nlink;
    uint64_t size;
    uint32_t uid, gid;
    uint64_t atime, mtime, ctime;
    uint32_t rdev;                      /* qurilma fayli uchun */
    int refcount;
    struct super_block *sb;
    const struct inode_ops *iops;
    const struct file_ops *fops;
    void *priv;                         /* fayl tizimining o'z ma'lumoti */
    struct mutex lock;
    struct inode *mounted;              /* bu papkaga boshqa FS ulangan: uning ildizi */
    struct inode *covered;              /* bu FS ildizi: qaysi papkani "yopib" turibdi */
};

struct file {
    struct inode *inode;
    const struct file_ops *fops;
    uint64_t offset;
    uint32_t flags;                     /* O_RDONLY / O_WRONLY / O_RDWR | O_APPEND ... */
    int refcount;                       /* fork/dup: bir nechta fd bitta file'ga */
    void *priv;
    struct mutex pos_lock;              /* offset ni himoya qiladi */
};

/* Fayl tizimi turi (tmpfs, ext2 ...). */
struct fs_type {
    const char *name;
    int (*mount)(struct block_device *bdev, struct super_block **out);
    struct list_head node;
};

/* ---- Inode ---- */
struct inode *inode_alloc(struct super_block *sb);
struct inode *iget(struct inode *inode);        /* refcount++ */
void iput(struct inode *inode);                 /* refcount-- (0 va nlink=0 bo'lsa - o'chirish) */

/* ---- Fayl ---- */
struct file *file_alloc(struct inode *inode, uint32_t flags);
struct file *file_dup(struct file *f);
void file_close(struct file *f);
int64_t file_read(struct file *f, void *buf, size_t len);
int64_t file_write(struct file *f, const void *buf, size_t len);
int64_t file_lseek(struct file *f, int64_t off, int whence);

/* ---- Yo'llar (path) ---- */
/* Yo'lni inode'ga aylantirish. cwd - nisbiy yo'llar uchun (NULL = ildiz).
 * Qaytaradi: 0 va *out (reference olingan), yoki -errno. */
int vfs_lookup(const char *path, struct inode *cwd, struct inode **out);
/* Oxirgi komponentsiz: /a/b/c -> *parent = /a/b, name = "c". */
int vfs_lookup_parent(const char *path, struct inode *cwd, struct inode **parent, char *name);

int vfs_open(const char *path, struct inode *cwd, uint32_t flags, uint32_t mode, struct file **out);
int vfs_mkdir(const char *path, struct inode *cwd, uint32_t mode);
int vfs_mknod(const char *path, struct inode *cwd, uint32_t mode, uint32_t rdev);
int vfs_unlink(const char *path, struct inode *cwd);
int vfs_rmdir(const char *path, struct inode *cwd);
int vfs_rename(const char *from, const char *to, struct inode *cwd);
int vfs_stat(struct inode *inode, struct myos_stat *st);
int vfs_readdir(struct file *f, struct myos_dirent *out);
/* Butun faylni yadroga o'qish (exec uchun). vfree bilan qaytaring. */
int vfs_read_whole(const char *path, struct inode *cwd, void **data, size_t *size);

/* ---- Mount ---- */
void vfs_register_fs(struct fs_type *fs);
int vfs_mount(const char *source, const char *target, const char *fstype);
int vfs_umount(const char *target);
/* Ildiz fayl tizimini o'rnatish (boot paytida). */
void vfs_mount_root(struct super_block *sb);
struct inode *vfs_root(void);
int vfs_sync(void);

/* ---- Belgili qurilmalar (char device) ---- */
void chrdev_register(uint32_t major, const struct file_ops *fops, const char *name);
const struct file_ops *chrdev_get(uint32_t major);

/* Yo'lni kanonik ko'rinishga keltirish (".", "..", "//" larni olib tashlash).
 * cwd_path + path -> out. */
int path_normalize(const char *cwd_path, const char *path, char *out, size_t size);
