/* =============================================================================
 *  fs/devfs.h - /dev: qurilma fayllari
 * ============================================================================= */
#pragma once

#include "fs/block.h"

#define MAJOR_MEM     1                 /* null, zero, random, kmsg */
#define MAJOR_TTY     5                 /* console */
#define MAJOR_HD      3                 /* IDE disklar (ATA PIO) */
#define MAJOR_SD      8                 /* SATA (AHCI) va USB disklar */
#define MAJOR_NVME    259

/* /dev papkasini yaratib, qurilma fayllarini (mknod) qo'shish. */
void devfs_init(void);
/* Keyinroq topilgan disk uchun /dev yozuvi. */
void devfs_add_block(struct block_device *bd);
