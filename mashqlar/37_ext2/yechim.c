/* =============================================================================
 *  37 - ext2 fayl tizimini o'qish                       [6-modul: yadro mexanizmlari]
 * =============================================================================
 *
 *  VAZIFA - disk tasviri faylidan (Linux'ning mke2fs yaratgan haqiqiy ext2) fayllarni
 *  o'qiydigan kichik "drayver". Tuzilmalar mashq.h da.
 *
 *    ext2_och(tasvir)      - faylni ochib superblokni o'qing (1024-baytdan). s_magic
 *                            0xEF53 bo'lmasa yoki fayl yo'q bo'lsa -> NULL.
 *                            Blok hajmi = 1024 << s_log_block_size; inode hajmi =
 *                            rev 0 bo'lsa 128, aks holda s_inode_size.
 *    ext2_inode(fs, n, &i) - n-inode'ni o'qish (1 dan boshlanadi!): 0 / -1
 *    ext2_qidir(fs, yol)   - "/papka/fayl" yo'lini inode raqamiga: ildiz (2) dan
 *                            boshlab, har bir qism uchun papka yozuvlarini qidirish.
 *                            "/" -> 2. Topilmasa yoki oraliq qism papka emas -> -1.
 *    ext2_oqi(fs, yol, buf, n) - fayl mazmunidan ko'pi bilan n baytni buf ga:
 *                            o'qilgan baytlar soni; topilmadi -> -1; papka -> -2.
 *    ext2_yop(fs)          - hamma resurslarni ozod qilish.
 *
 *  QANDAY ISHLAYDI:
 *    1) Inode n qayerda: guruh = (n - 1) / s_inodes_per_group,
 *       indeks = (n - 1) % s_inodes_per_group. Guruh deskriptorlari jadvali
 *       (s_first_data_block + 1)-blokda; undan bg_inode_table ni olib:
 *       joy = bg_inode_table * blok_hajmi + indeks * inode_hajmi.
 *    2) Fayl ma'lumoti: i_block[0..11] - ma'lumot bloklari raqamlari.
 *       i_block[12] - "bilvosita" blok: uning ichida blok_hajmi/4 ta raqam.
 *       i_block[13] - "ikki karra bilvosita": raqamlar bloklarining raqamlari.
 *       (Test 300 KB faylni o'qiydi - ikki karra bilvosita KERAK bo'ladi.)
 *       Raqam 0 - "teshik": o'sha blok nollardan iborat.
 *    3) Papka - oddiy fayl, mazmuni ext2_yozuv lar ketma-ketligi. Yozuvlar blok
 *       chegarasidan o'tmaydi; keyingisi = joriy + rec_len. inode == 0 - o'tkazib yuborish.
 *
 *  NEGA:
 *    Bu - MyOS'dagi kernel/fs/ext2.c ning o'qish qismi (bmap, block_path, dir
 *    lookup - lab'lar). Haqiqiy diskdagi haqiqiy formatni o'qish: spetsifikatsiyani
 *    kodga aylantirish mahorati - drayver dasturchisining asosiy ishi.
 *
 *  MASLAHAT:
 *    * "Blokni o'qi" yordamchi funksiyasi: pread(fd, buf, bs, (off_t)raqam * bs).
 *    * "Faylning k-blokining disk raqami" (bmap) - alohida funksiya. Keyin fayl
 *      o'qish - oddiy sikl.
 *    * Papka nomi '\0' bilan tugamaydi: strncmp emas, uzunlik + memcmp.
 *    * Debug: `debugfs -R "stat /salom.txt" build/mashq/t37.img` - Linux vositasi
 *      inode'ning hamma maydonlarini ko'rsatadi.
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 37   (mke2fs kerak: sudo apt install e2fsprogs)
 * ============================================================================= */
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mashq.h"

struct ext2 {
    int hali_bosh;      /* TODO: fd, blok hajmi, inode hajmi, guruhlar... */
};

struct ext2 *ext2_och(const char *tasvir)
{
    /* TODO */
    (void)tasvir;
    return NULL;
}

int ext2_inode(struct ext2 *fs, uint32_t raqam, struct ext2_inode *out)
{
    /* TODO */
    (void)fs; (void)raqam; (void)out;
    return -1;
}

long ext2_qidir(struct ext2 *fs, const char *yol)
{
    /* TODO */
    (void)fs; (void)yol;
    return -1;
}

long ext2_oqi(struct ext2 *fs, const char *yol, void *buf, size_t n)
{
    /* TODO */
    (void)fs; (void)yol; (void)buf; (void)n;
    return -1;
}

void ext2_yop(struct ext2 *fs)
{
    /* TODO */
    (void)fs;
}
