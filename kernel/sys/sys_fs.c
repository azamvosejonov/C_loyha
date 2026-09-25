/* =============================================================================
 *  sys/sys_fs.c - FAYL SYSCALL'LARI: open, read, write, pipe, dup2, stat ...
 * =============================================================================
 *
 *  FAYL DESKRIPTORLARI:
 *    current->files[fd] -> struct file (VFS). fd - shunchaki INDEKS.
 *    open() eng KICHIK bo'sh raqamni beradi - bu POSIX talabi va shell
 *    yo'naltirishlari shunga tayanadi:
 *        close(1); open("fayl", O_WRONLY);   // endi fd 1 = fayl (stdout -> fayl)
 *    dup2(old, new) - buni atomar qiladi.
 *
 *  Barcha yo'llar joriy papkaga (current->cwd) nisbatan hal qilinadi.
 * ============================================================================= */
#include "sys/sys_fs.h"

#include "fs/pipe.h"
#include "fs/vfs.h"
#include "lib/common.h"
#include "lib/string.h"
#include "mm/slab.h"
#include "proc/process.h"
#include "sys/uaccess.h"

#define IO_MAX (1024 * 1024)            /* bitta read/write uchun chegara */

int fd_install(struct file *f)
{
    for (int fd = 0; fd < MAX_FDS; fd++) {
        if (!current->files[fd]) {
            current->files[fd] = f;
            return fd;
        }
    }
    return -EMFILE;
}

static struct file *fd_get(int64_t fd)
{
    if (fd < 0 || fd >= MAX_FDS)
        return NULL;
    return current->files[fd];
}

/* User yo'lini olish (kmalloc bufer - kfree qiling). */
static char *get_path(uint64_t upath, long *err)
{
    char *p = kmalloc(PATH_MAX);
    if (!p) {
        *err = -ENOMEM;
        return NULL;
    }
    long r = copy_string_from_user(p, upath, PATH_MAX);
    if (r < 0) {
        kfree(p);
        *err = r;
        return NULL;
    }
    return p;
}

static int64_t sys_open(uint64_t upath, uint64_t flags, uint64_t mode)
{
    long err;
    char *path = get_path(upath, &err);
    if (!path)
        return err;
    struct file *f;
    err = vfs_open(path, current->cwd, (uint32_t)flags, (uint32_t)mode, &f);
    kfree(path);
    if (err)
        return err;
    int fd = fd_install(f);
    if (fd < 0)
        file_close(f);
    return fd;
}

static int64_t sys_close(int64_t fd)
{
    struct file *f = fd_get(fd);
    if (!f)
        return -EBADF;
    current->files[fd] = NULL;
    file_close(f);
    return 0;
}

static int64_t sys_read(int64_t fd, uint64_t buf, uint64_t len)
{
    struct file *f = fd_get(fd);
    if (!f)
        return -EBADF;
    len = MIN(len, (uint64_t)IO_MAX);
    if (!user_ok(buf, len, true))
        return -EFAULT;
    return file_read(f, (void *)(uintptr_t)buf, len);
}

static int64_t sys_write(int64_t fd, uint64_t buf, uint64_t len)
{
    struct file *f = fd_get(fd);
    if (!f)
        return -EBADF;
    len = MIN(len, (uint64_t)IO_MAX);
    if (!user_ok(buf, len, false))
        return -EFAULT;
    return file_write(f, (const void *)(uintptr_t)buf, len);
}

static int64_t sys_dup2(int64_t oldfd, int64_t newfd)
{
    struct file *f = fd_get(oldfd);
    if (!f || newfd < 0 || newfd >= MAX_FDS)
        return -EBADF;
    if (oldfd == newfd)
        return newfd;
    struct file *old = current->files[newfd];
    current->files[newfd] = file_dup(f);
    if (old)
        file_close(old);                /* avvalgisini yopamiz (POSIX) */
    return newfd;
}

static int64_t sys_dup(int64_t oldfd)
{
    struct file *f = fd_get(oldfd);
    if (!f)
        return -EBADF;
    int fd = fd_install(file_dup(f));
    if (fd < 0)
        file_close(f);
    return fd;
}

static int64_t sys_pipe(uint64_t ufds)
{
    if (!user_ok(ufds, 2 * sizeof(int32_t), true))
        return -EFAULT;
    struct file *r, *w;
    int err = pipe_create(&r, &w);
    if (err)
        return err;
    int rfd = fd_install(r);
    int wfd = rfd >= 0 ? fd_install(w) : -EMFILE;
    if (rfd < 0 || wfd < 0) {
        if (rfd >= 0)
            current->files[rfd] = NULL;
        file_close(r);
        file_close(w);
        return -EMFILE;
    }
    int32_t fds[2] = { rfd, wfd };
    memcpy((void *)(uintptr_t)ufds, fds, sizeof(fds));
    return 0;
}

static int64_t sys_stat(uint64_t upath, uint64_t ust)
{
    long err;
    char *path = get_path(upath, &err);
    if (!path)
        return err;
    struct inode *in;
    err = vfs_lookup(path, current->cwd, &in);
    kfree(path);
    if (err)
        return err;
    struct myos_stat st;
    vfs_stat(in, &st);
    iput(in);
    return copy_to_user(ust, &st, sizeof(st));
}

static int64_t sys_fstat(int64_t fd, uint64_t ust)
{
    struct file *f = fd_get(fd);
    if (!f)
        return -EBADF;
    struct myos_stat st;
    vfs_stat(f->inode, &st);
    return copy_to_user(ust, &st, sizeof(st));
}

static int64_t sys_getdents(int64_t fd, uint64_t ubuf, uint64_t max)
{
    struct file *f = fd_get(fd);
    if (!f)
        return -EBADF;
    if (max == 0)
        return 0;
    if (!user_ok(ubuf, max * sizeof(struct myos_dirent), true))
        return -EFAULT;
    struct myos_dirent *out = (struct myos_dirent *)(uintptr_t)ubuf;
    uint64_t n = 0;
    while (n < max) {
        int r = vfs_readdir(f, &out[n]);
        if (r < 0)
            return n ? (int64_t)n : r;
        if (r == 0)
            break;
        n++;
    }
    return (int64_t)n;
}

/* Bitta yo'l argumentli amallar. */
static int64_t path_op(uint64_t upath, int (*op)(const char *, struct inode *))
{
    long err;
    char *path = get_path(upath, &err);
    if (!path)
        return err;
    err = op(path, current->cwd);
    kfree(path);
    return err;
}

static int64_t sys_mkdir(uint64_t upath, uint64_t mode)
{
    long err;
    char *path = get_path(upath, &err);
    if (!path)
        return err;
    err = vfs_mkdir(path, current->cwd, (uint32_t)mode & 07777);
    kfree(path);
    return err;
}

static int64_t sys_rename(uint64_t ufrom, uint64_t uto)
{
    long err;
    char *from = get_path(ufrom, &err);
    if (!from)
        return err;
    char *to = get_path(uto, &err);
    if (!to) {
        kfree(from);
        return err;
    }
    err = vfs_rename(from, to, current->cwd);
    kfree(from);
    kfree(to);
    return err;
}

static int64_t sys_chdir(uint64_t upath)
{
    long err;
    char *path = get_path(upath, &err);
    if (!path)
        return err;
    struct inode *in;
    err = vfs_lookup(path, current->cwd, &in);
    if (!err && !S_ISDIR(in->mode)) {
        iput(in);
        err = -ENOTDIR;
    }
    if (!err) {
        char *canon = kmalloc(PATH_MAX);
        if (!canon || path_normalize(current->cwd_path, path, canon, PATH_MAX)) {
            iput(in);
            err = canon ? -ENAMETOOLONG : -ENOMEM;
        } else {
            iput(current->cwd);
            current->cwd = in;
            strlcpy(current->cwd_path, canon, sizeof(current->cwd_path));
        }
        kfree(canon);
    }
    kfree(path);
    return err;
}

static int64_t sys_getcwd(uint64_t ubuf, uint64_t size)
{
    size_t len = strlen(current->cwd_path) + 1;
    if (size < len)
        return -ERANGE;
    return copy_to_user(ubuf, current->cwd_path, len) ? -EFAULT : (int64_t)len;
}

static int64_t sys_mount(uint64_t usrc, uint64_t utarget, uint64_t utype)
{
    long err;
    char *src = get_path(usrc, &err);
    if (!src)
        return err;
    char *target = get_path(utarget, &err);
    char *type = target ? get_path(utype, &err) : NULL;
    if (target && type) {
        char *canon = kmalloc(PATH_MAX);
        err = canon ? path_normalize(current->cwd_path, target, canon, PATH_MAX) : -ENOMEM;
        if (!err)
            err = vfs_mount(src, canon, type);
        kfree(canon);
    }
    kfree(src);
    kfree(target);
    kfree(type);
    return err;
}

static int64_t sys_umount(uint64_t utarget)
{
    long err;
    char *target = get_path(utarget, &err);
    if (!target)
        return err;
    char *canon = kmalloc(PATH_MAX);
    err = canon ? path_normalize(current->cwd_path, target, canon, PATH_MAX) : -ENOMEM;
    if (!err)
        err = vfs_umount(canon);
    kfree(canon);
    kfree(target);
    return err;
}

static int64_t sys_ftruncate(int64_t fd, uint64_t size)
{
    struct file *f = fd_get(fd);
    if (!f)
        return -EBADF;
    struct inode *in = f->inode;
    if (!S_ISREG(in->mode) || (f->flags & O_ACCMODE) == O_RDONLY)
        return -EINVAL;
    if (!in->iops || !in->iops->truncate)
        return -EPERM;
    mutex_lock(&in->lock);
    int r = in->iops->truncate(in, size);
    mutex_unlock(&in->lock);
    return r;
}

static int64_t sys_ioctl(int64_t fd, uint64_t cmd, uint64_t arg)
{
    struct file *f = fd_get(fd);
    if (!f)
        return -EBADF;
    if (!f->fops || !f->fops->ioctl)
        return -ENOTTY;
    return f->fops->ioctl(f, cmd, arg);
}

int64_t sys_fs_dispatch(uint64_t nr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4,
                        bool *handled)
{
    (void)a4;
    *handled = true;
    switch (nr) {
    case SYS_OPEN:      return sys_open(a1, a2, a3);
    case SYS_CLOSE:     return sys_close((int64_t)a1);
    case SYS_READ:      return sys_read((int64_t)a1, a2, a3);
    case SYS_WRITE:     return sys_write((int64_t)a1, a2, a3);
    case SYS_LSEEK: {
        struct file *f = fd_get((int64_t)a1);
        return f ? file_lseek(f, (int64_t)a2, (int)a3) : -EBADF;
    }
    case SYS_DUP:       return sys_dup((int64_t)a1);
    case SYS_DUP2:      return sys_dup2((int64_t)a1, (int64_t)a2);
    case SYS_PIPE:      return sys_pipe(a1);
    case SYS_STAT:      return sys_stat(a1, a2);
    case SYS_FSTAT:     return sys_fstat((int64_t)a1, a2);
    case SYS_GETDENTS:  return sys_getdents((int64_t)a1, a2, a3);
    case SYS_MKDIR:     return sys_mkdir(a1, a2);
    case SYS_RMDIR:     return path_op(a1, vfs_rmdir);
    case SYS_UNLINK:    return path_op(a1, vfs_unlink);
    case SYS_RENAME:    return sys_rename(a1, a2);
    case SYS_CHDIR:     return sys_chdir(a1);
    case SYS_GETCWD:    return sys_getcwd(a1, a2);
    case SYS_MOUNT:     return sys_mount(a1, a2, a3);
    case SYS_UMOUNT:    return sys_umount(a1);
    case SYS_SYNC:      return vfs_sync();
    case SYS_FTRUNCATE: return sys_ftruncate((int64_t)a1, a2);
    case SYS_IOCTL:     return sys_ioctl((int64_t)a1, a2, a3);
    default:
        *handled = false;
        return -ENOSYS;
    }
}
