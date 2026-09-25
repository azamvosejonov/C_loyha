/* =============================================================================
 *  sys/uaccess.h - yadrodan USER xotirasiga xavfsiz murojaat
 * ============================================================================= */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* [addr, addr+len) joriy jarayonniki va (write bo'lsa) yozish mumkinmi?
 * Sahifalar hali yaratilmagan bo'lsa - hozir yaratiladi (demand paging). */
bool user_ok(uint64_t addr, size_t len, bool write);
/* User satrini yadroga: uzunlik yoki -errno. */
long copy_string_from_user(char *dst, uint64_t src, size_t max);
int copy_from_user(void *dst, uint64_t src, size_t len);
int copy_to_user(uint64_t dst, const void *src, size_t len);
