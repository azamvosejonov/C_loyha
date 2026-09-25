/* =============================================================================
 *  sys/syscall.h - tizim chaqiruvlari (syscall)
 * ============================================================================= */
#pragma once

/* int 0x80 handlerini ro'yxatdan o'tkazish. */
void syscall_init(void);
/* Har bir CPU'da syscall MSR larini sozlash (AP lar uchun ham). */
void syscall_init_cpu(void);
