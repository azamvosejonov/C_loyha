/* =============================================================================
 *  include/myos/abi.h - YADRO va USER dasturlari o'rtasidagi "shartnoma" (ABI)
 * =============================================================================
 *
 *  ABI (Application Binary Interface) - dastur yadro bilan qanday "gaplashishi"
 *  kerakligini belgilaydi: syscall raqamlari, argumentlar qaysi registrlarda,
 *  strukturalar qanday ko'rinishda. Bu fayl HAM yadroga, HAM user kutubxonasiga
 *  ulanadi - shunda ikki tomon doim bir xil raqamlarni ishlatadi.
 *
 *  QOIDA: ABI ni o'zgartirish = barcha dasturlarni qayta yig'ish. Linux
 *  shuning uchun syscall raqamlarini HECH QACHON o'zgartirmaydi ("we do not
 *  break userspace").
 *
 *  SYSCALL CHAQIRUV KONVENTSIYASI (Linux x86-64 bilan bir xil):
 *      syscall                 (eski usul `int 0x80` ham qo'llanadi)
 *      RAX = syscall raqami
 *      RDI, RSI, RDX, R10, R8 = 1..5-argumentlar
 *      Natija RAX da. -4095..-1 oralig'idagi qiymat = -errno (xato).
 *  (4-argument R10 da, RCX da emas: `syscall` instruksiyasi RCX ga qaytish
 *   manzilini, R11 ga RFLAGS ni yozadi - ular BUZILADI.)
 * ============================================================================= */
#pragma once

#include <stdint.h>

#define SYS_EXIT     0                  /* exit(code)                                */
#define SYS_WRITE    1                  /* write(fd, buf, len) -> yozilgan           */
#define SYS_READ     2                  /* read(fd, buf, len) -> o'qilgan            */
#define SYS_OPEN     3                  /* open(path, flags, mode) -> fd             */
#define SYS_CLOSE    4                  /* close(fd)                                 */
#define SYS_SPAWN    5                  /* spawn(path, argv) -> pid                  */
#define SYS_WAIT     6                  /* wait(pid, &status, flags) -> pid          */
#define SYS_GETPID   7
#define SYS_YIELD    8
#define SYS_SLEEP    9                  /* sleep_ms(ms)                              */
#define SYS_SBRK     10                 /* sbrk(increment) -> eski brk               */
#define SYS_GETDENTS 11                 /* getdents(fd, buf, max) -> yozuvlar soni   */
#define SYS_MEMINFO  12
#define SYS_PS       13
#define SYS_KILL     14                 /* kill(pid, sig)                            */
#define SYS_UPTIME   15                 /* uptime() -> ms                            */
#define SYS_SHUTDOWN 16
#define SYS_PCIINFO  17
#define SYS_REBOOT   18
#define SYS_DMESG    19
#define SYS_SYSINFO  20
#define SYS_FORK     21
#define SYS_EXEC     22                 /* exec(path, argv) - faqat xatoda qaytadi   */
#define SYS_MMAP     23
#define SYS_MUNMAP   24
#define SYS_GETPPID  25
#define SYS_LSEEK    26                 /* lseek(fd, off, whence) -> yangi pozitsiya */
#define SYS_DUP      27
#define SYS_DUP2     28                 /* dup2(old, new)                            */
#define SYS_PIPE     29                 /* pipe(int fds[2])                          */
#define SYS_STAT     30                 /* stat(path, &st)                           */
#define SYS_FSTAT    31                 /* fstat(fd, &st)                            */
#define SYS_MKDIR    32                 /* mkdir(path, mode)                         */
#define SYS_RMDIR    33
#define SYS_UNLINK   34
#define SYS_RENAME   35
#define SYS_CHDIR    36
#define SYS_GETCWD   37                 /* getcwd(buf, size)                         */
#define SYS_MOUNT    38                 /* mount(source, target, fstype)             */
#define SYS_SYNC     39
#define SYS_FTRUNCATE 40
#define SYS_TIME     41                 /* time() -> Unix vaqti (soniya)             */
#define SYS_IOCTL    42
#define SYS_UMOUNT   43
#define SYS_COUNT    44

/* ---- Xato kodlari (errno) - Linux bilan AYNAN bir xil qiymatlar ----
 * Yadro xatoda MANFIY kod qaytaradi (-ENOENT), libc uni errno ga yozib -1
 * qaytaradi (POSIX an'anasi). */
#define EPERM         1                 /* ruxsat yo'q */
#define ENOENT        2                 /* fayl yoki papka yo'q */
#define ESRCH         3                 /* jarayon yo'q */
#define EINTR         4                 /* signal bilan uzildi */
#define EIO           5                 /* kiritish/chiqarish xatosi */
#define ENXIO         6
#define E2BIG         7                 /* argumentlar juda uzun */
#define ENOEXEC       8                 /* bajariladigan fayl formati noto'g'ri */
#define EBADF         9                 /* noto'g'ri fayl deskriptori */
#define ECHILD        10                /* bola jarayon yo'q */
#define EAGAIN        11                /* vaqtincha imkonsiz, qayta urinib ko'ring */
#define ENOMEM        12                /* xotira yetmadi */
#define EACCES        13                /* ruxsat berilmagan */
#define EFAULT        14                /* noto'g'ri manzil */
#define EBUSY         16                /* band */
#define EEXIST        17                /* allaqachon mavjud */
#define EXDEV         18                /* boshqa qurilma (rename) */
#define ENODEV        19                /* qurilma yo'q */
#define ENOTDIR       20                /* papka emas */
#define EISDIR        21                /* bu papka */
#define EINVAL        22                /* noto'g'ri argument */
#define ENFILE        23
#define EMFILE        24                /* ochiq fayllar juda ko'p */
#define ENOTTY        25                /* terminal emas */
#define EFBIG         27                /* fayl juda katta */
#define ENOSPC        28                /* diskda joy yo'q */
#define ESPIPE        29                /* pipe'da lseek qilib bo'lmaydi */
#define EROFS         30                /* faqat o'qish uchun fayl tizimi */
#define EMLINK        31
#define EPIPE         32                /* o'quvchisiz pipe'ga yozish */
#define ERANGE        34
#define ENAMETOOLONG  36                /* nom juda uzun */
#define ENOSYS        38                /* bunday syscall yo'q */
#define ENOTEMPTY     39                /* papka bo'sh emas */

/* ---- open() bayroqlari (Linux qiymatlari) ---- */
#define O_RDONLY      0
#define O_WRONLY      1
#define O_RDWR        2
#define O_ACCMODE     3
#define O_CREAT       0100
#define O_EXCL        0200
#define O_TRUNC       01000
#define O_APPEND      02000
#define O_NONBLOCK    04000
#define O_DIRECTORY   0200000
#define O_CLOEXEC     02000000

/* ---- mmap ---- */
#define PROT_NONE     0
#define PROT_READ     1
#define PROT_WRITE    2
#define PROT_EXEC     4
#define MAP_PRIVATE   0x02
#define MAP_FIXED     0x10
#define MAP_ANONYMOUS 0x20
#define MAP_FAILED    ((void *)-1)

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

/* ---- Fayl turlari (mode ning yuqori bitlari) ---- */
#define S_IFMT   0170000
#define S_IFREG  0100000                /* oddiy fayl */
#define S_IFDIR  0040000                /* papka */
#define S_IFCHR  0020000                /* belgili qurilma (terminal, /dev/null) */
#define S_IFBLK  0060000                /* blokli qurilma (disk) */
#define S_IFIFO  0010000                /* pipe */
#define S_IFLNK  0120000                /* ramziy havola */
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m) (((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m) (((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)

#define NAME_MAX 255
#define PATH_MAX 1024

/* Nomlar POSIX'dagi `struct stat` bilan bir xil (st_ prefiksi) - libc'ning
 * sys/stat.h fayli shu strukturani `struct stat` deb ataydi. */
struct myos_stat {
    uint64_t st_ino;                    /* inode raqami (fayl tizimi ichida yagona) */
    uint32_t st_mode;                   /* tur + ruxsatlar (rwxrwxrwx) */
    uint32_t st_nlink;                  /* nechta nom shu faylga ko'rsatadi */
    uint64_t st_size;
    uint64_t st_atime, st_mtime, st_ctime;  /* Unix vaqti (soniya) */
    uint32_t st_dev;                    /* fayl tizimi qurilmasi */
    uint32_t st_rdev;                   /* qurilma fayli bo'lsa: major/minor */
    uint32_t st_blksize;
    uint32_t st_uid, st_gid;
    uint64_t st_blocks;                 /* 512 baytli bloklar */
};

struct myos_dirent {                    /* libc'da: struct dirent */
    uint64_t d_ino;
    uint32_t d_type;                    /* S_IFREG, S_IFDIR ... */
    uint32_t d_reserved;
    char d_name[NAME_MAX + 1];
};

/* ---- Terminal (TTY) - ioctl buyruqlari va termios (Linux qiymatlari) ----
 * Terminal ikki rejimda ishlaydi:
 *   KANONIK (ICANON)  - yadro qatorni o'zi yig'adi: backspace, Ctrl-U ishlaydi,
 *                       read() faqat Enter bosilganda qaytadi. Shell, cat.
 *   XOM (raw)         - har bir tugma darhol dasturga. Matn muharriri, o'yinlar. */
#define TCGETS      0x5401              /* ioctl(fd, TCGETS, &termios) */
#define TCSETS      0x5402              /* ioctl(fd, TCSETS, &termios) */
#define TIOCGWINSZ  0x5413              /* ioctl(fd, TIOCGWINSZ, &winsize) */

#define ISIG    0000001                 /* Ctrl-C va boshqalar signal yuboradi */
#define ICANON  0000002                 /* kanonik (qatorli) rejim */
#define ECHO    0000010                 /* kiritilgan belgilarni ekranga qaytarish */
#define ECHOE   0000020                 /* backspace ekrandan ham o'chiradi */

#define ICRNL   0000400                 /* kiritishda \r -> \n (iflag) */

#define VINTR   0                       /* c_cc indekslari */
#define VERASE  2
#define VKILL   3
#define VEOF    4
#define VMIN    6
#define NCCS    19

struct myos_termios {                   /* Linux yadrosidagi struct termios bilan bir xil joylashuv */
    uint32_t c_iflag;
    uint32_t c_oflag;
    uint32_t c_cflag;
    uint32_t c_lflag;
    uint8_t c_line;
    uint8_t c_cc[NCCS];
};

struct myos_winsize {
    uint16_t ws_row, ws_col, ws_xpixel, ws_ypixel;
};

/* wait() bayroqlari */
#define WAIT_NOHANG  1                  /* bola hali tugamagan bo'lsa kutmasdan 0 qaytar */

#define MYOS_NAME_MAX 32

struct myos_meminfo {
    uint64_t page_size;
    uint64_t total_pages;               /* PMM boshqaradigan RAM */
    uint64_t free_pages;
    uint64_t kheap_bytes_in_use;        /* yadro heap'i (kmalloc) */
    uint64_t kheap_allocs;
    uint64_t kheap_frees;
    uint64_t kheap_slab_pages;
    uint64_t kheap_large_pages;
};

/* Jarayon holatlari (proc/process.h dagi enum bilan bir xil tartibda). */
#define MYOS_PROC_EMBRYO  1
#define MYOS_PROC_READY   2
#define MYOS_PROC_RUNNING 3
#define MYOS_PROC_BLOCKED 4
#define MYOS_PROC_ZOMBIE  5

struct myos_pci_info {
    uint8_t bus, dev, func, class_code;
    uint8_t subclass, prog_if, irq_line, reserved;
    uint16_t vendor, device;
    char class_name[40];
    char driver[24];                    /* "" - drayver yo'q */
};

struct myos_sysinfo {
    uint32_t ncpus;
    uint32_t timer_hz;
    uint64_t uptime_ms;
    uint64_t tsc_khz;
    char cpu_vendor[16];
    char cpu_brand[52];
    char bootloader[64];
};

struct myos_proc_info {
    int32_t pid;
    int32_t ppid;
    int32_t state;
    int32_t is_user;
    uint64_t cpu_ticks;
    uint64_t mem_pages;
    char name[MYOS_NAME_MAX];
};
