/* =============================================================================
 *  user/include/myos.h - faqat MyOS'ga xos funksiyalar (POSIX'da yo'q)
 * =============================================================================
 *
 *  POSIX funksiyalari standart sarlavhalarda (unistd.h, stdio.h ...). Bu
 *  yerda esa bizning yadromizning o'ziga xos syscall'lari: ps, meminfo,
 *  lspci uchun ma'lumotlar va h.k. Linux'da bular odatda /proc va /sys
 *  fayllari orqali olinadi (mashq: /proc fayl tizimini yozing!).
 * ============================================================================= */
#pragma once

#include <stddef.h>
#include <stdint.h>

#include "myos/abi.h"

int spawn(const char *path, char *const argv[]);    /* fork+exec bitta chaqiruvda */
void yield(void);
uint64_t sleep_ms(uint64_t ms);           /* != 0 - signal uzdi */
uint64_t uptime_ms(void);
int meminfo(struct myos_meminfo *out);
int ps(struct myos_proc_info *buf, int max);
int pciinfo(int index, struct myos_pci_info *out);
long dmesg(char *buf, size_t size);
int sysinfo(struct myos_sysinfo *out);
int getdents(int fd, struct myos_dirent *buf, int max);
int mount(const char *source, const char *target, const char *fstype);
int umount(const char *target);
__attribute__((noreturn)) void shutdown(void);
__attribute__((noreturn)) void reboot(void);

/* malloc ichki statistikasi (memtest uchun). */
struct malloc_stats {
    size_t heap_bytes;                  /* sbrk orqali olingan jami */
    size_t used_bytes;                  /* band bloklar (foydali yuk) */
    size_t free_bytes;                  /* bo'sh bloklar */
    size_t free_blocks;                 /* bo'sh bloklar soni (fragmentatsiya o'lchovi) */
    size_t used_blocks;
};
void malloc_get_stats(struct malloc_stats *out);
