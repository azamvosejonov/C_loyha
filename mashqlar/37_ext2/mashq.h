#pragma once

#include <stddef.h>
#include <stdint.h>

/* ---- ext2 diskdagi tuzilmalari (spetsifikatsiya: nongnu.org/ext2-doc) ---- */

/* Superblok: diskning 1024-baytidan boshlanadi (bu yerda faqat kerakli qismi). */
struct ext2_superblok {
    uint32_t s_inodes_count;
    uint32_t s_blocks_count;
    uint32_t s_r_blocks_count;
    uint32_t s_free_blocks_count;
    uint32_t s_free_inodes_count;
    uint32_t s_first_data_block;        /* 1 KB blokda 1, kattaroqda 0 */
    uint32_t s_log_block_size;          /* blok hajmi = 1024 << shu */
    uint32_t s_log_frag_size;
    uint32_t s_blocks_per_group;
    uint32_t s_frags_per_group;
    uint32_t s_inodes_per_group;
    uint32_t s_mtime, s_wtime;
    uint16_t s_mnt_count, s_max_mnt_count;
    uint16_t s_magic;                   /* 0xEF53 */
    uint16_t s_state, s_errors, s_minor_rev_level;
    uint32_t s_lastcheck, s_checkinterval, s_creator_os;
    uint32_t s_rev_level;               /* 0 - eski (inode 128 bayt), 1 - dinamik */
    uint16_t s_def_resuid, s_def_resgid;
    uint32_t s_first_ino;
    uint16_t s_inode_size;              /* rev 1 da: inode hajmi (128, 256 ...) */
};

/* Guruh deskriptori (32 bayt). Jadval superblokdan keyingi blokda. */
struct ext2_guruh {
    uint32_t bg_block_bitmap;
    uint32_t bg_inode_bitmap;
    uint32_t bg_inode_table;            /* shu guruh inode jadvalining birinchi bloki */
    uint16_t bg_free_blocks_count;
    uint16_t bg_free_inodes_count;
    uint16_t bg_used_dirs_count;
    uint16_t bg_pad;
    uint8_t  bg_reserved[12];
};

/* Inode (asosiy 128 bayt). */
struct ext2_inode {
    uint16_t i_mode;                    /* tur (0x4000 papka, 0x8000 fayl) + ruxsatlar */
    uint16_t i_uid;
    uint32_t i_size;                    /* hajm (bayt) */
    uint32_t i_atime, i_ctime, i_mtime, i_dtime;
    uint16_t i_gid;
    uint16_t i_links_count;
    uint32_t i_blocks;
    uint32_t i_flags;
    uint32_t i_osd1;
    uint32_t i_block[15];               /* 0-11 bevosita, 12 - bilvosita, 13 - ikki karra, 14 - uch karra */
    uint32_t i_generation, i_file_acl, i_size_high, i_faddr;
    uint8_t  i_osd2[12];
};

/* Papka yozuvi (o'zgaruvchan uzunlik). */
struct ext2_yozuv {
    uint32_t inode;                     /* 0 - bo'sh yozuv */
    uint16_t rec_len;                   /* shu yozuvdan keyingisigacha masofa */
    uint8_t  name_len;
    uint8_t  file_type;
    char     name[];                    /* '\0' bilan TUGAMAYDI! uzunlik - name_len */
};

#define EXT2_ILDIZ_INODE 2
#define EXT2_S_IFMT  0xF000
#define EXT2_S_IFDIR 0x4000
#define EXT2_S_IFREG 0x8000

struct ext2;                            /* sizning tuzilmangiz (yechim.c da) */

struct ext2 *ext2_och(const char *tasvir);
int ext2_inode(struct ext2 *fs, uint32_t raqam, struct ext2_inode *out);
long ext2_qidir(struct ext2 *fs, const char *yol);
long ext2_oqi(struct ext2 *fs, const char *yol, void *buf, size_t n);
void ext2_yop(struct ext2 *fs);
