/* =============================================================================
 *  lib/klog.h - yadro logi (dmesg) halqali buferi
 * ============================================================================= */
#pragma once

#include <stddef.h>

void klog_putc(char c);
/* Butun logni fn orqali qayta chiqarish (yangi ekran ishga tushganda). */
void klog_replay(void (*fn)(char c));
/* Log nusxasini buf ga (dmesg syscall uchun). Qaytaradi: nusxalangan baytlar. */
size_t klog_read(char *buf, size_t size);
