/* mfs.c - disk tasviri (oddiy fayl) ustida ishlaydigan mini fayl tizimi: mkfs, put, ls, cat, rm, fsck */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOK 512                                /* bir blok - 512 bayt */
#define BLOKLAR 256                             /* butun tasvir: 256 blok = 128 KB */
#define INODELAR 32
#define BITMAP_BLOK 1                           /* 1-blok: band bloklar xaritasi */
#define INODE_BOSH 2                            /* 2..5: inode jadvali (har blokda 8 ta inode) */
#define DATA_BOSH 6                             /* 6 dan: fayl ma'lumotlari */
#define TOGRI 12                                /* inode da 12 ta bevosita blok raqami: eng ko'pi 12 * 512 = 6144 bayt */
#define MAGIK 0x4D465331u                       /* "MFS1" */

struct super {                                  /* 0-blok */
    uint32_t magik, bloklar, inodelar, data_bosh;
};

struct inode {                                  /* 64 bayt: 512 / 64 = 8 ta inode bir blokda */
    uint16_t tur;                               /* 0 - bo'sh, 1 - fayl, 2 - katalog */
    uint16_t nlink;
    uint32_t hajm;
    uint32_t blok[TOGRI];
    uint8_t to_ldiruvchi[8];
};

struct yozuv {                                  /* katalogdagi bitta nom: (nom -> inode raqami), 16 bayt */
    uint16_t inode;                             /* 0 - bo'sh yozuv (inode 0 - ildiz katalog, nom emas) */
    char nom[14];
};

static FILE *tasvir;
static uint8_t bitmap[BLOK];

/* --- eng past daraja: blok o'qish/yozish --- */
static void blok_oqi(unsigned n, void *b)
{
    fseek(tasvir, (long)n * BLOK, SEEK_SET);
    if (fread(b, BLOK, 1, tasvir) != 1) {
        fprintf(stderr, "mfs: blok %u o'qilmadi\n", n);
        exit(1);
    }
}

static void blok_yoz(unsigned n, const void *b)
{
    fseek(tasvir, (long)n * BLOK, SEEK_SET);
    if (fwrite(b, BLOK, 1, tasvir) != 1) {
        fprintf(stderr, "mfs: blok %u yozilmadi\n", n);
        exit(1);
    }
}

static int bm_bor(unsigned i) { return (bitmap[i / 8] >> (i % 8)) & 1; }
static void bm_yoq(unsigned i) { bitmap[i / 8] |= (uint8_t)(1u << (i % 8)); }
static void bm_och(unsigned i) { bitmap[i / 8] &= (uint8_t)~(1u << (i % 8)); }

static void inode_oqi(unsigned i, struct inode *x)
{
    uint8_t b[BLOK];
    blok_oqi(INODE_BOSH + i / 8, b);
    memcpy(x, b + (i % 8) * sizeof(*x), sizeof(*x));
}

static void inode_yoz(unsigned i, const struct inode *x)
{
    uint8_t b[BLOK];
    blok_oqi(INODE_BOSH + i / 8, b);
    memcpy(b + (i % 8) * sizeof(*x), x, sizeof(*x));
    blok_yoz(INODE_BOSH + i / 8, b);
}

static int blok_ajrat(void)                     /* birinchi bo'sh ma'lumot blokini band qiladi */
{
    for (unsigned i = DATA_BOSH; i < BLOKLAR; i++)
        if (!bm_bor(i)) {
            bm_yoq(i);
            return (int)i;
        }
    return -1;
}

/* --- tasvirni ochish va bitmapni yuklash/saqlash --- */
static void och(const char *yol, const char *rejim)
{
    tasvir = fopen(yol, rejim);
    if (!tasvir) {
        perror(yol);
        exit(1);
    }
}

static void yukla(void)
{
    uint8_t b[BLOK];
    blok_oqi(0, b);
    const struct super *s = (const struct super *)b;
    if (s->magik != MAGIK) {
        fprintf(stderr, "mfs: bu mfs tasviri emas (sehrli son noto'g'ri)\n");
        exit(1);
    }
    blok_oqi(BITMAP_BLOK, bitmap);
}

static void saqla(void)
{
    blok_yoz(BITMAP_BLOK, bitmap);
    fflush(tasvir);
}

/* --- katalog (ildiz, inode 0): bitta blokda 32 ta yozuv --- */
static struct yozuv katalog[BLOK / sizeof(struct yozuv)];
static unsigned katalog_blok;

static void katalog_yukla(void)
{
    struct inode ildiz;
    inode_oqi(0, &ildiz);
    katalog_blok = ildiz.blok[0];
    blok_oqi(katalog_blok, katalog);
}

static int topish(const char *nom)              /* katalog indeksini yoki -1 */
{
    for (size_t i = 0; i < sizeof(katalog) / sizeof(katalog[0]); i++)
        if (katalog[i].inode != 0 && strncmp(katalog[i].nom, nom, sizeof(katalog[i].nom)) == 0)
            return (int)i;
    return -1;
}

/* --- buyruqlar --- */
static int mkfs(const char *yol)
{
    och(yol, "wb+");
    uint8_t nol[BLOK] = { 0 };
    for (int i = 0; i < BLOKLAR; i++)
        blok_yoz((unsigned)i, nol);

    struct super s = { MAGIK, BLOKLAR, INODELAR, DATA_BOSH };
    uint8_t b[BLOK] = { 0 };
    memcpy(b, &s, sizeof(s));
    blok_yoz(0, b);

    for (unsigned i = 0; i <= DATA_BOSH; i++)   /* metama'lumot bloklari (0..5) va ildiz katalog bloki (6) band */
        bm_yoq(i);
    struct inode ildiz = { 2, 1, BLOK, { DATA_BOSH }, { 0 } };
    inode_yoz(0, &ildiz);
    saqla();
    printf("mfs yaratildi: %d blok x %d bayt, %d inode\n", BLOKLAR, BLOK, INODELAR);
    return 0;
}

static int put(const char *tasvir_yol, const char *manba, const char *nom)
{
    FILE *f = fopen(manba, "rb");
    if (!f) {
        perror(manba);
        return 1;
    }
    uint8_t data[TOGRI * BLOK];
    size_t n = fread(data, 1, sizeof(data), f);
    int katta = fgetc(f) != EOF;                /* 6144 baytdan ortiq qoldimi? */
    fclose(f);
    if (katta) {
        fprintf(stderr, "mfs: %s juda katta (eng ko'pi %d bayt)\n", manba, TOGRI * BLOK);
        return 1;
    }
    och(tasvir_yol, "rb+");
    yukla();
    katalog_yukla();
    if (strlen(nom) >= sizeof(katalog[0].nom) || topish(nom) >= 0) {
        fprintf(stderr, "mfs: nom noto'g'ri yoki band: %s\n", nom);
        return 1;
    }

    int inode_raqam = -1;                       /* bo'sh inode topish */
    struct inode x;
    for (unsigned i = 1; i < INODELAR && inode_raqam < 0; i++) {
        inode_oqi(i, &x);
        if (x.tur == 0)
            inode_raqam = (int)i;
    }
    int bosh_yozuv = -1;                        /* katalogda bo'sh joy */
    for (size_t i = 0; i < sizeof(katalog) / sizeof(katalog[0]) && bosh_yozuv < 0; i++)
        if (katalog[i].inode == 0)
            bosh_yozuv = (int)i;
    if (inode_raqam < 0 || bosh_yozuv < 0) {
        fprintf(stderr, "mfs: joy yo'q (inode yoki katalog yozuvi tugagan)\n");
        return 1;
    }

    memset(&x, 0, sizeof(x));
    x.tur = 1;
    x.nlink = 1;
    x.hajm = (uint32_t)n;
    for (size_t k = 0; k * BLOK < n; k++) {     /* har blokka 512 baytdan */
        int blok = blok_ajrat();
        if (blok < 0) {
            fprintf(stderr, "mfs: disk to'lgan\n");
            return 1;                           /* (soddalik uchun ajratilgan bloklar qaytarilmaydi: fsck topadi) */
        }
        uint8_t b[BLOK] = { 0 };
        size_t qism = n - k * BLOK < BLOK ? n - k * BLOK : BLOK;
        memcpy(b, data + k * BLOK, qism);
        blok_yoz((unsigned)blok, b);
        x.blok[k] = (uint32_t)blok;
    }
    saqla();                                    /* TARTIB (27-bob, izchillik): ma'lumot -> bitmap -> inode -> katalog yozuvi. */
    inode_yoz((unsigned)inode_raqam, &x);       /* Istalgan joyda uzilsa, eng yomoni "sizib chiqish" (fsck tuzatadi); */
    katalog[bosh_yozuv].inode = (uint16_t)inode_raqam;      /* ishlatilayotgan blok "bo'sh" ko'rinib qolmaydi. */
    snprintf(katalog[bosh_yozuv].nom, sizeof(katalog[0].nom), "%s", nom);
    blok_yoz(katalog_blok, katalog);
    printf("yozildi: %s (%zu bayt, inode %d, %zu blok)\n", nom, n, inode_raqam, (n + BLOK - 1) / BLOK);
    return 0;
}

static int ls(const char *yol)
{
    och(yol, "rb");
    yukla();
    katalog_yukla();
    int soni = 0;
    unsigned band = 0;
    for (unsigned i = 0; i < BLOKLAR; i++)
        band += bm_bor(i);
    for (size_t i = 0; i < sizeof(katalog) / sizeof(katalog[0]); i++) {
        if (katalog[i].inode == 0)
            continue;
        struct inode x;
        inode_oqi(katalog[i].inode, &x);
        printf("  %-14s inode %-3u %5u bayt\n", katalog[i].nom, katalog[i].inode, x.hajm);
        soni++;
    }
    printf("%d ta fayl; %u/%d blok band\n", soni, band, BLOKLAR);
    return 0;
}

static int cat(const char *yol, const char *nom)
{
    och(yol, "rb");
    yukla();
    katalog_yukla();
    int k = topish(nom);
    if (k < 0) {
        fprintf(stderr, "mfs: %s: bunday fayl yo'q\n", nom);
        return 1;
    }
    struct inode x;
    inode_oqi(katalog[k].inode, &x);
    for (size_t i = 0; i * BLOK < x.hajm; i++) {
        uint8_t b[BLOK];
        blok_oqi(x.blok[i], b);
        size_t qism = x.hajm - i * BLOK < BLOK ? x.hajm - i * BLOK : BLOK;
        fwrite(b, 1, qism, stdout);
    }
    return 0;
}

static int rm(const char *yol, const char *nom)
{
    och(yol, "rb+");
    yukla();
    katalog_yukla();
    int k = topish(nom);
    if (k < 0) {
        fprintf(stderr, "mfs: %s: bunday fayl yo'q\n", nom);
        return 1;
    }
    struct inode x;
    inode_oqi(katalog[k].inode, &x);
    unsigned inode_raqam = katalog[k].inode;    /* TARTIB teskari: katalog yozuvi -> inode -> bitmap (uzilsa - faqat sizib chiqish) */
    katalog[k].inode = 0;
    blok_yoz(katalog_blok, katalog);
    struct inode bosh = { 0 };
    inode_yoz(inode_raqam, &bosh);
    for (size_t i = 0; i * BLOK < x.hajm; i++)
        bm_och(x.blok[i]);
    saqla();
    printf("o'chirildi: %s\n", nom);
    return 0;
}

/* fsck: bitmap haqiqiy ishlatilishga mos keladimi? tuzatish=1 bo'lsa bitmapni tuzatadi */
static int fsck(const char *yol, int tuzatish)
{
    och(yol, tuzatish ? "rb+" : "rb");
    yukla();
    uint8_t ishlatiladi[BLOKLAR] = { 0 };
    for (unsigned i = 0; i <= DATA_BOSH - 1; i++)
        ishlatiladi[i] = 1;                     /* metama'lumot bloklari */
    for (unsigned i = 0; i < INODELAR; i++) {
        struct inode x;
        inode_oqi(i, &x);
        if (x.tur == 0)
            continue;
        size_t blok_soni = (x.hajm + BLOK - 1) / BLOK;           /* fayl necha blokni egallaydi */
        for (size_t k = 0; k < blok_soni && k < TOGRI; k++) {
            unsigned bl = x.blok[k];
            if (bl < DATA_BOSH || bl >= BLOKLAR) {
                printf("  XATO: inode %u: noto'g'ri blok raqami %u\n", i, bl);
                continue;
            }
            if (ishlatiladi[bl])
                printf("  XATO: blok %u ikki inode tomonidan ishlatilgan (inode %u)\n", bl, i);
            ishlatiladi[bl] = 1;
        }
    }
    int muammo = 0;
    for (unsigned i = DATA_BOSH; i < BLOKLAR; i++) {
        if (bm_bor(i) && !ishlatiladi[i]) {
            printf("  SIZIB CHIQISH: blok %u bitmapda band, lekin hech bir inode ishlatmaydi%s\n", i, tuzatish ? " -> bo'shatildi" : "");
            if (tuzatish)
                bm_och(i);
            muammo++;
        } else if (!bm_bor(i) && ishlatiladi[i]) {
            printf("  XAVFLI: blok %u ishlatiladi, lekin bitmapda bo'sh (boshqa faylga berilib ketishi mumkin!)%s\n", i, tuzatish ? " -> band qilindi" : "");
            if (tuzatish)
                bm_yoq(i);
            muammo++;
        }
    }
    if (tuzatish && muammo)
        saqla();
    printf("fsck: %s\n", muammo == 0 ? "fayl tizimi izchil (muammo yo'q)" : (tuzatish ? "muammolar topildi va TUZATILDI" : "muammolar topildi"));
    return muammo != 0 && !tuzatish;
}

static int hack(const char *yol, unsigned blok)  /* DEMO: bitmapni ataylab buzamiz (avariyani modellash) */
{
    och(yol, "rb+");
    yukla();
    bm_yoq(blok);
    saqla();
    printf("bitmap buzildi: blok %u 'band' deb belgilandi\n", blok);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc >= 3 && strcmp(argv[1], "mkfs") == 0)
        return mkfs(argv[2]);
    if (argc >= 5 && strcmp(argv[1], "put") == 0)
        return put(argv[2], argv[3], argv[4]);
    if (argc >= 3 && strcmp(argv[1], "ls") == 0)
        return ls(argv[2]);
    if (argc >= 4 && strcmp(argv[1], "cat") == 0)
        return cat(argv[2], argv[3]);
    if (argc >= 4 && strcmp(argv[1], "rm") == 0)
        return rm(argv[2], argv[3]);
    if (argc >= 3 && strcmp(argv[1], "fsck") == 0)
        return fsck(argv[2], argc >= 4 && strcmp(argv[3], "-t") == 0);
    if (argc >= 4 && strcmp(argv[1], "hack") == 0)
        return hack(argv[2], (unsigned)atoi(argv[3]));
    fprintf(stderr, "foydalanish: mfs mkfs|put|ls|cat|rm|fsck|hack TASVIR [...]\n");
    return 2;
}
