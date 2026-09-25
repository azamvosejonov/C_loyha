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
 *  SYSCALL CHAQIRUV KONVENTSIYASI (bizning, Linux'ga o'xshash):
 *      int 0x80
 *      RAX = syscall raqami
 *      RDI, RSI, RDX, R10, R8 = 1..5-argumentlar
 *      Natija RAX da. Manfiy qiymat = xato.
 *  (R10, RCX emas - kelajakda `syscall` instruksiyasiga o'tish uchun: u RCX ni
 *   qaytish manzili sifatida buzadi.)
 * ============================================================================= */
#pragma once

#include <stdint.h>

#define SYS_EXIT     0                  /* exit(code)                       */
#define SYS_WRITE    1                  /* write(fd, buf, len) -> yozilgan  */
#define SYS_READ     2                  /* read(fd, buf, len) -> o'qilgan   */
#define SYS_OPEN     3                  /* open(path) -> fd                 */
#define SYS_CLOSE    4                  /* close(fd)                        */
#define SYS_SPAWN    5                  /* spawn(path, argv) -> pid         */
#define SYS_WAIT     6                  /* wait(pid, &status, flags) -> pid */
#define SYS_GETPID   7                  /* getpid() -> pid                  */
#define SYS_YIELD    8                  /* yield()                          */
#define SYS_SLEEP    9                  /* sleep_ms(ms)                     */
#define SYS_SBRK     10                 /* sbrk(increment) -> eski brk      */
#define SYS_READDIR  11                 /* readdir(index, &dirent) -> 0/-1  */
#define SYS_MEMINFO  12                 /* meminfo(&info)                   */
#define SYS_PS       13                 /* ps(buf, max) -> soni             */
#define SYS_KILL     14                 /* kill(pid)                        */
#define SYS_UPTIME   15                 /* uptime() -> millisekund          */
#define SYS_SHUTDOWN 16                 /* shutdown()                       */
#define SYS_PCIINFO  17                 /* pciinfo(index, &info) -> 0/-1    */
#define SYS_REBOOT   18                 /* reboot()                         */
#define SYS_DMESG    19                 /* dmesg(buf, size) -> baytlar      */
#define SYS_SYSINFO  20                 /* sysinfo(&info)                   */
#define SYS_COUNT    21

/* wait() bayroqlari */
#define WAIT_NOHANG  1                  /* bola hali tugamagan bo'lsa kutmasdan 0 qaytar */

#define MYOS_NAME_MAX 32

struct myos_dirent {
    char name[100];
    uint64_t size;
};

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
