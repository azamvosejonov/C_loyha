/*
 * test_makro.h — emulyator testlari uchun assembly makrolari (C preprotsessori orqali, .S fayllarda).
 *
 * G'oya (riscv-tests loyihasidagi kabi): har tekshiruvning RAQAMI bor. Natija kutilganidan farq qilsa —
 * test raqami bilan "xato" kodi quvvat qurilmasiga yoziladi va emulyator SHU KOD bilan chiqadi.
 * Hammasi to'g'ri bo'lsa — 0x5555 (chiqish kodi 0).
 *   gp — joriy test raqami (xato bo'lsa qaysi tekshiruv buzilganini bilish uchun).
 *   t6 — kutilgan qiymat uchun vaqtinchalik registr (testlar uni boshqa maqsadda ishlatmaydi).
 */
#define QUVVAT 0x00100000

#define BOSHLASH                \
    .option norvc;              \
    .section .text.boshi;       \
    .globl _start;              \
_start:

/* SINA(raqam, registr, kutilgan): registr != kutilgan bo'lsa — test xato bilan tugaydi */
#define SINA(raqam, reg, kutilgan)  \
    li gp, raqam;               \
    li t6, kutilgan;            \
    bne reg, t6, test_xato

/* testning oxiri: muvaffaqiyat va xato yo'llari */
#define TUGATISH                \
test_otdi:                      \
    li t5, QUVVAT;              \
    li t6, 0x5555;              \
    sw t6, 0(t5);               \
1:  j 1b;                       \
test_xato:                      \
    li t5, QUVVAT;              \
    slli t6, gp, 16;            \
    ori t6, t6, 0x333;          \
    li t4, 0x3000;              \
    or t6, t6, t4;              \
    sw t6, 0(t5);               \
2:  j 2b;
