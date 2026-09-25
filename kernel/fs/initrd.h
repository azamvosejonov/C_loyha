/* =============================================================================
 *  fs/initrd.h - boot paytidagi tar arxivni ildiz fayl tizimiga ochish
 * ============================================================================= */
#pragma once

#include "boot/bootinfo.h"

/* initrd.tar ni "/" ga ochadi. Qaytaradi: fayllar soni. */
int initrd_unpack(const struct boot_info *bi);
