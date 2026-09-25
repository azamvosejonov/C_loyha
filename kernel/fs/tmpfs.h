/* =============================================================================
 *  fs/tmpfs.h - RAM'dagi fayl tizimi
 * ============================================================================= */
#pragma once

#include "fs/vfs.h"

void tmpfs_init(void);
struct super_block *tmpfs_create_sb(void);
