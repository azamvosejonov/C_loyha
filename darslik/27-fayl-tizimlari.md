# 27-bob. Qurilmalar va fayl tizimlari

> **Bu bobda nima o'rganasiz:** OS qurilmalar bilan qanday gaplashishini (polling, uzilish, DMA, drayver); HDD va SSD qanday ishlashini; fayl tizimi **ichida** nima borligini (inode, bitmap, katalog, havolalar);
> faylning baytlari diskda qayerda ekanini topish usulini; va eng muhimi — **tok o'chsa nima bo'ladi** (fsck, jurnal, atomik saqlash).
> (OSTEP "Persistence" qismi.)
> **Oldindan nima kerak:** 7-, 14-, 16-boblar (ko'rsatkichlar, fayl syscall'lari, qurilma registrlari).   **Vaqt:** 8–10 soat.
> Mashqlar: 25, 26, 37.

> **To'liq ishlaydigan misol:** [misollar/27_fayllar.c](misollar/27_fayllar.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Bu bob nima haqida?

Dastur xotirada ishlaydi, lekin xotira tok o'chsa **yo'qoladi**. Doimiy saqlash uchun disk kerak. Bu bobda ikki katta savolga javob topamiz:

1. **Disk bilan qanday gaplashiladi?** (qurilma, drayver, kutish usullari, HDD/SSD farqi)
2. **Disk ustida "fayl va papka" degan qulay dunyo qanday quriladi?** Disk — shunchaki raqamlangan bloklar to'plami; "kundalik.txt" degan nom, hajm, ruxsatlar, tuzilma — hammasini fayl tizimi o'zi o'ylab topadi.

Va uchinchi, eng nozik savol: **diskka yozish paytida tok o'chsa, fayl tizimi buzilmaydimi?**

**Hayotdan misol: ulkan kutubxona.** Kitoblar — fayllar. Javonlar — disk bloklari. Katalog kartochkasi — **inode** (kitobning hajmi, egasi, sahifalari qaysi javonlarda — lekin **nomi emas**). Mundarija — **katalog** ("nom → kartochka raqami").
Kutubxonachi (OS) siz so'raganda mundarijadan nomni topadi, kartochkani oladi, javonlardan sahifalarni keltiradi.

| Kutubxonada | Fayl tizimida |
|---|---|
| kitob | fayl |
| javon | disk bloki |
| katalog kartochkasi (hajm, egasi, sahifalar qayerda) | **inode** |
| mundarija ("nom → kartochka #") | **katalog** (papka) |
| javon band/bo'sh xaritasi | **bitmap** |
| kutubxona qoidalari va tuzilishi tavsifi | **superblok** |
| bir kitobga ikki nom | qattiq havola (hard link) |
| "bu kitobni falon nom bilan qidiring" qog'ozi | ramziy havola (symlink) |
| bank xodimining daftari ("men shuni qilmoqchiman") | jurnal |

## 27.1. Qurilma bilan gaplashish: polling, uzilish, DMA

### Qurilma OS'ga qanday ko'rinadi

**Oddiy qilib aytganda:** disk, tarmoq kartasi, klaviatura — protsessorga **registrlar** to'plami bo'lib ko'rinadi (16-bob): "holat" registri (band/tayyor), "buyruq" registri, "ma'lumot" registri. OS shu registrlarga yozib
qurilmaga buyruq beradi. Eng oddiy protokol:

```text
while (holat == BAND) ;            /* 1. qurilma bo'shaguncha kutish */
ma'lumotni yozish;                 /* 2. */
buyruqni yozish;                   /* 3. qurilma ishni boshlaydi */
while (holat == BAND) ;            /* 4. tugashini kutish */
```

Bu protokolda ikki muammo bor va ikkalasining ham yechimi bor:

| Muammo | Yechim |
|---|---|
| Kutish paytida CPU bo'sh sikl aylanib **behuda yonadi** (polling) | **Uzilish** (interrupt): buyruq berib, jarayon **uxlaydi**; qurilma tugagach CPU'ga signal yuboradi |
| Ma'lumotni CPU **o'zi** nusxalaydi (PIO) — CPU band | **DMA**: CPU "xotiraning shu fizik manzilidan N bayt ol" deydi, nusxani **qurilmaning o'zi** qiladi |

### Polling va uyqu: farqni o'lchaymiz

**Hayotdan misol: kuryer.** Buyurtma berdingiz. *Polling* — har soniya eshikka chiqib qarash. *Uzilish* — o'tirib o'z ishingizni qilasiz, kuryer kelsa qo'ng'iroq qiladi.

**Bu dastur nima qiladi (umumiy):** "qurilma" (boshqa jarayon) 300 ms o'ylab, keyin bitta bayt yuboradi. Dastur javobni ikki usulda kutadi: (1) **polling** — `read` ni "bloklamasdan" tinmay chaqiradi; (2) **uxlash** — oddiy
bloklanuvchi `read`: yadro jarayonni uxlatadi, ma'lumot kelganda uyg'otadi (bu — uzilish mexanizmining dasturdagi ko'rinishi). Har ikkala usulda dastur **CPU'da qancha ishlagani** o'lchanadi.

```c
/* polling_uyqu.c - kutishning ikki usuli: band kutish (polling) va uxlash (uzilish kabi) */
#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static long cpu_ms(void)                        /* shu jarayon CPU'da ishlagan vaqt */
{
    return (long)(clock() * 1000L / CLOCKS_PER_SEC);
}

static pid_t qurilma_ishga_tushir(int yoz_fd)   /* "qurilma": 300 ms o'ylaydi, keyin javob beradi */
{
    pid_t p = fork();
    if (p == 0) {
        usleep(300000);
        if (write(yoz_fd, "!", 1) != 1)
            _exit(1);
        _exit(0);
    }
    return p;
}

int main(void)
{
    char c;
    int q[2];

    /* 1) POLLING: o'qish "bloklanmaydi" - tayyormi deb tinmay so'raymiz */
    if (pipe(q) != 0)
        return 1;
    fcntl(q[0], F_SETFL, O_NONBLOCK);
    pid_t p1 = qurilma_ishga_tushir(q[1]);
    long boshi = cpu_ms();
    long urinish = 0;
    while (read(q[0], &c, 1) != 1)
        urinish++;
    long polling_cpu = cpu_ms() - boshi;
    waitpid(p1, NULL, 0);

    /* 2) UXLASH: o'qish bloklanadi - yadro bizni uxlatadi, tayyor bo'lganda uyg'otadi */
    int r[2];
    if (pipe(r) != 0)
        return 1;
    pid_t p2 = qurilma_ishga_tushir(r[1]);
    boshi = cpu_ms();
    if (read(r[0], &c, 1) != 1)
        return 1;
    long uyqu_cpu = cpu_ms() - boshi;
    waitpid(p2, NULL, 0);

    printf("polling: 300 ms kutdi, CPU yondi >= 200 ms: %s, so'rovlar soni > 1000: %s\n",
           polling_cpu >= 200 ? "ha" : "yo'q", urinish > 1000 ? "ha" : "yo'q");
    printf("uxlash : 300 ms kutdi, CPU yondi < 20 ms:   %s\n", uyqu_cpu < 20 ? "ha" : "yo'q");
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 polling_uyqu.c -o polling_uyqu
$ ./polling_uyqu
polling: 300 ms kutdi, CPU yondi >= 200 ms: ha, so'rovlar soni > 1000: ha
uxlash : 300 ms kutdi, CPU yondi < 20 ms:   ha
```

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `pipe(q)` | "qurilma" bilan aloqa kanali; `q[0]` — o'qish uchi, `q[1]` — yozish uchi |
| `fork()` + `usleep(300000)` + `write` | "qurilma": 300 ms kutib, bitta bayt yuboradi |
| `fcntl(q[0], F_SETFL, O_NONBLOCK)` | o'qishni **bloklamaydigan** qiladi: ma'lumot yo'q bo'lsa `read` darhol xato qaytaradi |
| `while (read(...) != 1) urinish++` | **polling**: javob kelguncha tinmay so'rash |
| oddiy `read(r[0], ...)` | **bloklanuvchi**: ma'lumot yo'q bo'lsa yadro jarayonni uxlatadi |
| `clock()` | shu jarayon CPU'da **haqiqatan ishlagan** vaqti (uxlagan vaqt hisoblanmaydi) |

**Nima ko'rdik:** ikkala usulda ham javobni 300 ms kutdik, lekin polling shu vaqtning deyarli hammasida CPU'ni yondirdi (minglab, odatda yuz minglab foydasiz so'rov), uxlash esa deyarli nol CPU sarfladi — shu vaqtda CPU boshqa jarayonlarni ishlata oladi.

**Nega ikkalasi ham kerak?** Uzilishning o'z narxi bor (uyg'otish, kontekst almashish). Juda tez qurilmada (NVMe, 100 Gb/s tarmoq) javob mikrosekundlarda keladi — uzilishga ketadigan vaqt javob vaqtidan ko'p bo'lib qoladi.
Shuning uchun **gibrid**: avval biroz polling, kutish cho'zilsa uzilishga o'tish (NVMe drayveri, tarmoqda NAPI).

> **Eslab qoling:** **polling** — tinmay so'rash (CPU yonadi, lekin javob tez); **uzilish** — uxlab, qurilma uyg'otsin (CPU bo'sh); **DMA** — nusxani CPU emas qurilma qiladi. MyOS'da ikkalasi ham bor: ATA PIO (`drivers/ata.c`) va AHCI DMA (`drivers/ahci.c`) — solishtiring.

### Drayver

**Drayver** — aniq qurilmaga xos kod. Uning ustida OS **umumiy interfeys** ko'radi: blok qurilma (`read_block` / `write_block`) yoki belgi qurilma (`read` / `write` oqimi). Fayl tizimi disk ATA yoki AHCI yoki NVMe ekanini
bilmaydi — faqat "blokni o'qi" deydi. Linux kodining ~70% i shunday drayverlardir. MyOS: `kernel/fs/block.c`.

## 27.2. Disklar: HDD va SSD

**HDD (qattiq disk).** Aylanuvchi plastinalar, ustida magnit kallak. Sektorni o'qish uchun ikki jismoniy harakat kerak:

| Qadam | Nima | Taxminan vaqt |
|---|---|---|
| izlash (seek) | kallakni kerakli yo'lakka surish | 4–10 ms |
| aylanish kutish | kerakli sektor kallak ostiga kelguncha | 2–4 ms |
| uzatish | baytlarni o'qish | juda qisqa |

**Hayotdan misol:** kutubxonachi zinapoyadan kerakli qavatga chiqadi (izlash), keyin javon aylanib kelishini kutadi (aylanish). Ketma-ket joylashgan kitoblarni olish tez, tarqoqlarini — juda sekin.

**Bu dastur nima qiladi (umumiy):** HDD'ning oddiy hisob modeli: 1 GB ma'lumotni (a) ketma-ket, (b) 4 KB'lik tasodifiy bo'laklar bilan o'qish uchun qancha vaqt ketishini hisoblaydi.

```c
/* disk_model.c - HDD modeli: ketma-ket va tasodifiy o'qish */
#include <stdio.h>

int main(void)
{
    double izlash_ms = 8.0, aylanish_ms = 4.0;  /* kallakni surish + javon aylanishi */
    double tezlik_mb_s = 150.0;                 /* ketma-ket uzatish tezligi */
    double hajm_mb = 1024.0;                    /* 1 GB o'qiymiz */
    double blok_kb = 4.0;                       /* tasodifiy o'qishda blok o'lchami */

    double ketma_s = hajm_mb / tezlik_mb_s;
    long bloklar = (long)(hajm_mb * 1024.0 / blok_kb);
    double bir_blok_ms = izlash_ms + aylanish_ms + blok_kb / 1024.0 / tezlik_mb_s * 1000.0;
    double tasodifiy_s = bloklar * bir_blok_ms / 1000.0;

    printf("1 GB ketma-ket o'qish:   %7.1f soniya (%.0f MB/s)\n", ketma_s, tezlik_mb_s);
    printf("1 GB tasodifiy (4 KB):   %7.1f soniya (%ld ta murojaat, har biri %.2f ms)\n",
           tasodifiy_s, bloklar, bir_blok_ms);
    printf("tasodifiy tezlik:        %7.2f MB/s, sekundiga %.0f ta murojaat\n",
           hajm_mb / tasodifiy_s, 1000.0 / bir_blok_ms);
    printf("farq:                    %7.0f marta\n", tasodifiy_s / ketma_s);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 disk_model.c -o disk_model
$ ./disk_model
1 GB ketma-ket o'qish:       6.8 soniya (150 MB/s)
1 GB tasodifiy (4 KB):    3152.6 soniya (262144 ta murojaat, har biri 12.03 ms)
tasodifiy tezlik:           0.32 MB/s, sekundiga 83 ta murojaat
farq:                        462 marta
```

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `izlash_ms`, `aylanish_ms` | har bir tasodifiy murojaatda ketadigan mexanik vaqt (8 + 4 ms) |
| `ketma_s` | 1024 MB / 150 MB/s: kallak bir marta suriladi, keyin oqim bo'ylab o'qiydi |
| `bloklar` | 1 GB necha ta 4 KB blokdan iborat (262 144) |
| `bir_blok_ms` | bitta tasodifiy murojaat vaqti ≈ 12 ms (izlash + aylanish + oz uzatish) |
| `tasodifiy_s` | 262 144 ta murojaat × 12 ms |

**Nima ko'rdik:** ketma-ket o'qish — taxminan 7 soniya; xuddi shu 1 GB tasodifiy bo'laklar bilan — **ellik minutdan ko'p** (modelda ≈ 460 marta sekin; haqiqiy disklarda 100–1000 marta). Shuning uchun **fayl tizimlari tarixi —
"tasodifiy murojaatni ketma-ketga aylantirish" tarixi**: bog'liq narsalarni diskda yonma-yon joylashtirish (27.6), keshlash, bir nechta yozuvni birlashtirish.

**SSD (flesh xotira).** Harakatlanadigan qism yo'q; tasodifiy o'qish ~10–100 mikrosekund (HDD'dan ~100 marta tez). Lekin o'ziga xos qoidalari bor:

- yozish **sahifalar** bilan (4–16 KB), o'chirish esa ancha katta **bloklar** bilan;
- har bir blokni cheklangan marta o'chirish mumkin (yeyilish);
- shuning uchun SSD ichida o'z "fayl tizimi" ishlaydi — **FTL**: yozishlarni tekis taqsimlaydi, "axlat yig'adi";
- OS unga **TRIM** buyrug'i bilan "bu bloklar endi bo'sh" deb aytadi.

**Disk rejalashtirish** (HDD uchun): so'rovlarni kallak harakatini kamaytiradigan tartibda bajarish — **SSTF** (eng yaqinini), **SCAN / elevator** (lift kabi bir yo'nalishda, orqaga qaytguncha). SSD'da bu deyarli ahamiyatsiz.

> **Eslab qoling:** HDD: izlash (ms) + aylanish (ms) → ketma-ket tez, tasodifiy juda sekin. SSD: mexanika yo'q, lekin yozish/o'chirish bloklari bor (FTL, TRIM).

## 27.3. Fayl nima: fayl va katalog

**Oddiy qilib aytganda:** foydalanuvchi uchun fayl — baytlar ketma-ketligi, papka — "nom → fayl" ro'yxati. Ichkarida ikkalasi ham **inode** bilan ifodalanadi, har bir inode'ning raqami bor.

| Tushuncha | Nima | Nimani saqlaydi |
|---|---|---|
| **Inode** | fayl haqida hamma narsa, **nomidan tashqari** | tur (fayl/papka/...), hajm, egasi, ruxsatlar, vaqtlar, **nomlar soni**, **ma'lumot bloklari qayerda** |
| **Katalog** (papka) | oddiy fayl, lekin mazmuni maxsus | (nom, inode raqami) juftliklari ro'yxati |
| **Nom** | katalogdagi yozuv | faqat "bu nom — falon inode" |

Eng muhim g'oya: **nom faylning ichida emas, katalogda turadi.** Shuning uchun bitta faylga bir nechta nom berish mumkin.

**Bu dastur nima qiladi (umumiy):** katalog yaratadi, unda 4 ta nom hosil qiladi: ikkita oddiy fayl, bitta **qattiq havola** (`nusxa.txt` → `kitob.txt` ga) va bitta **ramziy havola**. Keyin katalog tarkibini o'qib, har bir
nomning inode guruhini, nomlar sonini va hajmini ko'rsatadi.

```c
/* katalog_inode.c - katalog = (nom, inode raqami) juftliklari; havolalar */
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static void yarat(const char *nom, const char *matn)
{
    int fd = open(nom, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0 || write(fd, matn, strlen(matn)) < 0)
        exit(1);
    close(fd);
}

static int solishtir(const void *a, const void *b)
{
    return strcmp(*(char *const *)a, *(char *const *)b);
}

int main(void)
{
    if (system("rm -rf kutubxona && mkdir kutubxona") != 0 || chdir("kutubxona") != 0)
        return 1;
    yarat("kitob.txt", "salom");
    yarat("boshqa.txt", "yana bir fayl");
    if (link("kitob.txt", "nusxa.txt") != 0 || symlink("kitob.txt", "yollanma.txt") != 0)
        return 1;

    char *nomlar[16];
    int n = 0;
    DIR *d = opendir(".");
    struct dirent *e;
    while ((e = readdir(d)) != NULL)
        if (e->d_name[0] != '.')
            nomlar[n++] = strdup(e->d_name);
    closedir(d);
    qsort(nomlar, n, sizeof(char *), solishtir);

    unsigned long korilgan[16];                 /* ko'rilgan inode'lar -> guruh raqami */
    int guruhlar = 0;
    printf("%-14s %-8s %-12s %s\n", "NOM", "INODE", "NOMLAR SONI", "HAJM");
    for (int i = 0; i < n; i++) {
        struct stat st;
        lstat(nomlar[i], &st);
        int g = -1;
        for (int j = 0; j < guruhlar; j++)
            if (korilgan[j] == (unsigned long)st.st_ino)
                g = j;
        if (g < 0) {
            korilgan[guruhlar] = st.st_ino;
            g = guruhlar++;
        }
        printf("%-14s #%-7d %-12ld %ld bayt%s\n", nomlar[i], g + 1, (long)st.st_nlink, (long)st.st_size,
               S_ISLNK(st.st_mode) ? "  (ichida yo'l matni)" : "");
    }
    printf("(#N - bir xil raqam = bir xil inode, ya'ni bitta fayl)\n");

    if (chdir("..") != 0 || system("rm -rf kutubxona") != 0)
        return 1;
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 katalog_inode.c -o katalog_inode
$ ./katalog_inode
NOM            INODE    NOMLAR SONI  HAJM
boshqa.txt     #1       1            13 bayt
kitob.txt      #2       2            5 bayt
nusxa.txt      #2       2            5 bayt
yollanma.txt   #3       1            9 bayt  (ichida yo'l matni)
(#N - bir xil raqam = bir xil inode, ya'ni bitta fayl)
```

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `link("kitob.txt", "nusxa.txt")` | **qattiq havola**: mavjud inode'ga ikkinchi **nom** qo'shadi |
| `symlink("kitob.txt", "yollanma.txt")` | **ramziy havola**: ichida "kitob.txt" matni yozilgan **alohida** fayl (o'z inode'i bor) |
| `readdir` | katalogdagi (nom, inode) juftliklarini o'qiydi |
| `lstat` | inode ma'lumotlarini oladi (havolaning o'zini, ko'rsatgan faylni emas) |
| `st_ino` | inode raqami (bu yerda `#1`, `#2`, `#3` deb tartiblab ko'rsatdik — haqiqiy raqamlar mashinaga bog'liq) |
| `st_nlink` | shu inode'ga nechta **nom** ishora qilyapti |

**Nima ko'rdik:**

- `kitob.txt` va `nusxa.txt` — **bir xil inode (#2)**, nomlar soni 2: bitta fayl, ikki nom. Biriga yozsangiz, ikkinchisida ham ko'rinadi.
- `yollanma.txt` — **boshqa inode (#3)**, nomlar soni 1, hajmi 9 bayt — bu "kitob.txt" matnining uzunligi: ramziy havola ichida faqat yo'l yozilgan.

| | Qattiq havola | Ramziy havola |
|---|---|---|
| Inode | **xuddi shu** | o'zining alohida inode'i |
| Ichida nima | — (shunchaki yana bir nom) | yo'l matni |
| Asl nom o'chirilsa | fayl yashaydi (boshqa nom bor) | havola **osilib qoladi** (hech qayerga olib bormaydi) |
| Boshqa diskka | bo'lmaydi | bo'ladi |

**Hayotdan misol:** qattiq havola — bitta kitobga mundarijada ikki joyda bir xil kartochka raqami. Ramziy havola — "bu kitobni falon nom bilan qidiring" degan qog'oz.

> **Eslab qoling:** nom — katalogda, hamma boshqa narsa — **inode**'da. `ls -i` inode raqamini ko'rsatadi; `ls -l` dagi ikkinchi ustun — nomlar soni.

### Nom o'chirilsa fayl o'chadimi?

`unlink` fayl emas, **nomni** o'chiradi. Inode (va bloklar) nomlar soni **0 ga tushganda VA hech kim ochib turmaganda** o'chadi.

**Bu dastur nima qiladi (umumiy):** faylni ochadi, keyin uning nomini o'chiradi va fayl ochiq turguncha hali yashayotganini ko'rsatadi.

```c
/* ochiq_ochirish.c - ochiq faylni o'chirish: nom ketadi, fayl ochiq turguncha yashaydi */
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main(void)
{
    int fd = open("vaqtincha.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0 || write(fd, "maxfiy ma'lumot", 15) != 15)
        return 1;

    struct stat st;
    fstat(fd, &st);
    printf("ochiq, nom bor:   nomlar soni = %ld\n", (long)st.st_nlink);

    unlink("vaqtincha.txt");                    /* nomni o'chiramiz */
    fstat(fd, &st);
    printf("nom o'chirildi:   nomlar soni = %ld, hajm = %ld bayt\n", (long)st.st_nlink, (long)st.st_size);
    printf("katalogda bormi:  %s\n", access("vaqtincha.txt", F_OK) == 0 ? "ha" : "yo'q");

    char b[32] = "";
    lseek(fd, 0, SEEK_SET);
    ssize_t n = read(fd, b, sizeof(b) - 1);
    printf("lekin o'qish mumkin: [%.*s]\n", (int)(n < 0 ? 0 : n), b);

    close(fd);                                  /* oxirgi ochiq tutqich yopildi - endi disk bo'shaydi */
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 ochiq_ochirish.c -o ochiq_ochirish
$ ./ochiq_ochirish
ochiq, nom bor:   nomlar soni = 1
nom o'chirildi:   nomlar soni = 0, hajm = 15 bayt
katalogda bormi:  yo'q
lekin o'qish mumkin: [maxfiy ma'lumot]
```

**Nima ko'rdik:** nom o'chdi (nomlar soni 0, katalogda yo'q), lekin ochiq tutqich (`fd`) orqali fayl hajmi 15 bayt va mazmuni hali o'qiladi. Faqat `close(fd)` dan keyin disk joyi bo'shaydi. Shuning uchun
"fayl o'chirilgan, lekin disk joyi bo'shamadi" holatiga duch kelasiz: kimdir uni ochib turibdi (`lsof` buni ko'rsatadi).

## 27.4. Tizim chaqiruvlari: `fsync` va `rename`

Fayl bilan ishlash syscall'lari (14-bob): `open`, `read`, `write`, `lseek`, `fsync`, `rename`, `link`, `unlink`, `mkdir`, `stat`. Ikkitasi tok o'chishi masalasida alohida muhim:

| Syscall | Nima qiladi | Nega muhim |
|---|---|---|
| **`fsync(fd)`** | "ma'lumotni **hozir** diskka yoz va tugagach qayt" | `write` odatda faqat xotiradagi **keshga** yozadi — tok o'chsa yo'qoladi. Ma'lumotlar bazalari har tranzaksiyadan keyin `fsync` qiladi |
| **`rename(eski, yangi)`** | nomni almashtiradi — **atomik** | fayl yo eski, yo yangi holda ko'rinadi — hech qachon "yarmi" emas |

Ular birgalikda faylni **xavfsiz yangilashning** standart usulini beradi (muharrirlar shunday saqlaydi): 1) yangi mazmunni **vaqtinchalik faylga** yoz; 2) `fsync`; 3) `rename` bilan eski fayl ustiga qo'y.

**Hayotdan misol: devordagi e'lon.** Eskisini yirtib, yangisini yozishni boshlamaysiz — shu orada kimdir bo'sh devorni ko'radi. Yangisini oldindan tayyorlab, **bitta harakat** bilan almashtirasiz.

**Bu dastur nima qiladi (umumiy):** ikki saqlash usulini solishtiradi va ikkalasida ham "yozish paytida tok o'chdi" holatini modellaydi: (1) oddiy usul — avval faylni **tozalaydi**, keyin yozadi; (2) xavfsiz usul — yangisini boshqa faylga yozadi,
keyin `rename`.

```c
/* saqlash_xavfi.c - tok o'chsa: oddiy saqlash va tmp+rename (avariyani modellaymiz) */
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void yoz(const char *nom, const char *matn)
{
    int fd = open(nom, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0 || write(fd, matn, strlen(matn)) < 0)
        perror("yoz");
    close(fd);
}

static void korsat(const char *izoh, const char *nom)
{
    char b[128] = "";
    int fd = open(nom, O_RDONLY);
    ssize_t n = fd < 0 ? 0 : read(fd, b, sizeof(b) - 1);
    if (fd >= 0)
        close(fd);
    if (n < 0)
        n = 0;
    b[n] = '\0';
    printf("%-34s [%s]\n", izoh, b);
}

int main(void)
{
    const char *eski = "ESKI: 100 so'm";
    const char *yangi = "YANGI: 250 so'm";

    /* 1) oddiy saqlash: avval faylni TOZALAYDI (O_TRUNC), keyin yozadi. Tozalash va yozish orasida "tok o'chdi" */
    yoz("hisob1.txt", eski);
    int fd = open("hisob1.txt", O_WRONLY | O_TRUNC);   /* 1-qadam: tozalandi */
    (void)fd;                                           /* 2-qadam (yozish) ULGURMADI: tok o'chdi */
    close(fd);
    korsat("oddiy saqlash, avariyadan keyin:", "hisob1.txt");

    /* 2) tmp + rename: yangi mazmun AVVAL boshqa faylga yoziladi */
    yoz("hisob2.txt", eski);
    yoz("hisob2.txt.tmp", yangi);                       /* 1-qadam: tmp yozildi */
    /* 2-qadam (rename) ULGURMADI: tok o'chdi */
    korsat("tmp+rename, avariyadan keyin:", "hisob2.txt");
    if (rename("hisob2.txt.tmp", "hisob2.txt") != 0)    /* agar avariya bo'lmaganda: */
        perror("rename");
    korsat("tmp+rename, rename bajarilsa:", "hisob2.txt");

    unlink("hisob1.txt");
    unlink("hisob2.txt");
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 saqlash_xavfi.c -o saqlash_xavfi
$ ./saqlash_xavfi
oddiy saqlash, avariyadan keyin:   []
tmp+rename, avariyadan keyin:      [ESKI: 100 so'm]
tmp+rename, rename bajarilsa:      [YANGI: 250 so'm]
```

**Qismlar va natija:**

| Usul | Qadamlar | Avariyadan keyin (2-qadam ulgurmasa) |
|---|---|---|
| Oddiy | 1) `O_TRUNC` bilan ochish (fayl **bo'shaydi**) → 2) yozish | fayl **bo'sh** `[]` — eski ma'lumot ham, yangisi ham yo'q! |
| `tmp` + `rename` | 1) `hisob2.txt.tmp` ga yozish → 2) `rename` | fayl **eski** holda `[ESKI: 100 so'm]` — ma'lumot sog'; `rename` bajarilsa — to'liq **yangi** |

> **Eslab qoling:** faylni joyida o'zgartirmang. **Yangisini yoz → `fsync` → `rename`.** Natija har doim yo eski, yo yangi.

### To'liq dastur: kundalik saqlovchi

Quyidagi dastur shu bo'limlarning hammasini birlashtiradi (pastda "Hayotdan misol va to'liq dastur" bo'limida batafsil tahlili): xavfsiz saqlash (`tmp` + `fsync` + `rename`), qattiq va ramziy havola, inode taqqoslash, o'chirilgan nomning oqibati.

## 27.5. Oddiy fayl tizimini loyihalash (vsfs)

**Oddiy qilib aytganda:** fayl tizimi — diskni bloklarga (masalan 4 KB) bo'lib, ularning har biriga **vazifa** beradi. OSTEP'dagi "juda oddiy fayl tizimi" (vsfs) shunday joylashadi:

```text
[ superblok | inode bitmap | ma'lumot bitmap | inode jadvali ........ | ma'lumot bloklari .......... ]
```

| Qism | Nima | Hayotiy o'xshatish |
|---|---|---|
| **Superblok** | butun fayl tizimi haqida: bloklar soni, inode'lar soni, jadvallar qayerda, "sehrli son" | kutubxona tavsifi va qoidalari |
| **inode bitmap** | qaysi inode'lar band (har inode uchun 1 bit) | kartochkalar qutisi: qaysi kartochka to'ldirilgan |
| **ma'lumot bitmap** | qaysi ma'lumot bloklari band | javonlar: band/bo'sh xaritasi |
| **inode jadvali** | hamma inode'lar ketma-ket | kartochkalar qutisi |
| **ma'lumot bloklari** | fayllarning haqiqiy baytlari | javonlardagi kitoblar |

### Faylning bloklari qayerda? Ko'p darajali indeks

Inode'ga fayl bloklarining **hammasini** yozib bo'lmaydi (fayl katta bo'lishi mumkin). ext2 yechimi (37-mashq) — "nomutanosib daraxt": kichik fayllar uchun tez, kattalar uchun ham sig'adigan.

| Daraja | Inode'da nima | 4 KB blokda qancha ma'lumotni ko'rsatadi |
|---|---|---|
| **Bevosita** (direct) | 12 ta blok raqami | 12 × 4 KB = **48 KB** |
| **Bilvosita** (indirect) | 1 ta ko'rsatkich → u 1024 ta blok raqamli blokka ishora qiladi | 1024 × 4 KB = **4 MB** |
| **Ikki karra** (double) | 1 ta ko'rsatkich → 1024 ta ko'rsatkich bloki → har biri 1024 ta ma'lumot blokiga | 1024² × 4 KB = **4 GB** |
| **Uch karra** (triple) | yana bir daraja | 1024³ × 4 KB = **4 TB** |

(Bir blokda 4 KB / 4 bayt = **1024** ta blok raqami sig'adi.)

Nega bunday? Kuzatuv: **ko'p fayllar kichik, bir nechtasi juda katta.** Kichik fayl (≤ 48 KB) uchun qo'shimcha blok o'qish shart emas; katta fayl uchun esa daraxt chuqurlashadi.

**Bu dastur nima qiladi (umumiy):** fayl ichidagi ofsetni (nechanchi bayt) beramiz — dastur ext2 usulida bu bayt uchun **qaysi daraja va qaysi indekslar** ishlatilishini topadi (`bmap` funksiyasining g'oyasi), va har daraja maksimal necha KB/MB/GB/TB ko'rsatishini hisoblaydi.

```c
/* bmap_yol.c - ext2 uslubida: fayl ofseti -> qaysi ko'rsatkich darajasi? */
#include <stdio.h>

#define BLOK 4096L
#define KO 1024L                                /* bir blokdagi ko'rsatkichlar soni: 4096 / 4 */

static void yol(long ofset)
{
    long k = ofset / BLOK;                      /* fayl ichidagi blok raqami */
    printf("ofset %13ld (blok %9ld): ", ofset, k);
    if (k < 12) {
        printf("bevosita [%ld]\n", k);
        return;
    }
    k -= 12;
    if (k < KO) {
        printf("bilvosita -> [%ld]\n", k);
        return;
    }
    k -= KO;
    if (k < KO * KO) {
        printf("ikki karra -> [%ld] -> [%ld]\n", k / KO, k % KO);
        return;
    }
    k -= KO * KO;
    printf("uch karra -> [%ld] -> [%ld] -> [%ld]\n", k / (KO * KO), k / KO % KO, k % KO);
}

int main(void)
{
    long ofsetlar[] = { 0, 5000, 11 * BLOK, 12 * BLOK, (12 + 1023) * BLOK, (12 + 1024) * BLOK,
                        (12 + 1024 + 5000) * BLOK, (12 + 1024 + 1024L * 1024) * BLOK };
    for (int i = 0; i < 8; i++)
        yol(ofsetlar[i]);

    printf("\nBlok 4 KB bo'lsa, fayl eng ko'pi bilan:\n");
    printf("  bevosita:   %5ld KB\n", 12 * BLOK / 1024);
    printf("  bilvosita:  %5ld KB (%ld MB)\n", KO * BLOK / 1024, KO * BLOK / 1024 / 1024);
    printf("  ikki karra: %5ld MB (%ld GB)\n", KO * KO * BLOK / 1024 / 1024, KO * KO * BLOK / 1024 / 1024 / 1024);
    printf("  uch karra:  %5ld GB (%ld TB)\n", KO * KO * KO * BLOK / 1024 / 1024 / 1024,
           KO * KO * KO * BLOK / 1024 / 1024 / 1024 / 1024);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 bmap_yol.c -o bmap_yol
$ ./bmap_yol
ofset             0 (blok         0): bevosita [0]
ofset          5000 (blok         1): bevosita [1]
ofset         45056 (blok        11): bevosita [11]
ofset         49152 (blok        12): bilvosita -> [0]
ofset       4239360 (blok      1035): bilvosita -> [1023]
ofset       4243456 (blok      1036): ikki karra -> [0] -> [0]
ofset      24723456 (blok      6036): ikki karra -> [4] -> [904]
ofset    4299210752 (blok   1049612): uch karra -> [0] -> [0] -> [0]

Blok 4 KB bo'lsa, fayl eng ko'pi bilan:
  bevosita:      48 KB
  bilvosita:   4096 KB (4 MB)
  ikki karra:  4096 MB (4 GB)
  uch karra:   4096 GB (4 TB)
```

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `k = ofset / BLOK` | fayl ichidagi blok raqami (baytni blok o'lchamiga bo'lish) |
| `k < 12` | bevosita: inode'dagi `blok[k]` |
| `k -= 12; k < KO` | bilvosita: ko'rsatkichlar blokining `[k]` elementi |
| `k -= KO; k < KO*KO` | ikki karra: birinchi blokda `k / KO`, ikkinchisida `k % KO` |
| `k / KO % KO`, `k % KO` | uch karrada uch indeks |

**Nima ko'rdik:** 0–11-bloklar (49 152 baytgacha) — bevosita; 12-blokdan bilvosita (`[0]` dan `[1023]` gacha); 1036-blokdan ikki karra. Kichik faylning bloki manzili inode'ning o'zida turadi — qo'shimcha disk murojaati kerak emas.
4 MB dan keyin esa avval **ko'rsatkichlar bloki** ham o'qilishi kerak (bitta qo'shimcha murojaat), 4 GB dan keyin — ikkita. Shuning uchun katta fayl oxiriga yetish "qimmatroq" va keshlash muhim.

Boshqa yondashuv — **ekstentlar** (ext4, XFS): "shu blokdan boshlab 1000 ta ketma-ket blok" — bitta yozuv bilan katta oraliqni ko'rsatadi; katta fayllar uchun ixchamroq.

### Faylni o'qish yo'li

`open("/papka/fayl.txt")` nima qiladi:

| Qadam | Nima | Qayerdan |
|---|---|---|
| 1 | ildiz katalogning inode'ini oladi (raqami ma'lum — ext2'da **2**) | inode jadvali |
| 2 | ildiz katalogning bloklarida **"papka"** nomini qidiradi → uning inode raqami | ma'lumot bloki |
| 3 | "papka" inode'ini o'qiydi → uning bloklarida **"fayl.txt"** ni qidiradi → inode raqami | inode + blok |
| 4 | "fayl.txt" inode'ini o'qiydi va `fd` ga bog'laydi | inode jadvali |

`read`: inode'dan kerakli blok raqamini topish (**bmap**, yuqoridagi dastur) → blokni o'qish. Chuqur yo'l = ko'p disk murojaati — shuning uchun **keshlar** hal qiluvchi: sahifa keshi (fayl mazmuni), dentry keshi (nom → inode),
inode keshi. MyOS: `kernel/fs/ext2.c` (inode keshi), `kernel/fs/block.c` (buffer cache), `kernel/fs/vfs.c` (yo'lni aylanish).

> **Eslab qoling:** superblok + bitmap'lar + inode jadvali + ma'lumot bloklari. Inode bloklarni **ko'p darajali indeks** bilan ko'rsatadi: 12 bevosita + 1 + 1 + 1 bilvosita. Yo'lni ochish = har komponent uchun katalog o'qish.

## 27.6. FFS — diskni hisobga oluvchi joylashtirish

Birinchi Unix fayl tizimi sodda edi va disk o'tkazuvchanligining atigi ~2% ini ishlatardi: fayl bloklari va inode'lar diskning turli chekkalarida sochilgan edi — har o'qishda uzoq izlash (27.2 dagi model!).

**FFS** (Fast File System, 1984) g'oyasi: diskni **silindr guruhlariga** bo'lish va **bog'liq narsalarni bir guruhga qo'yish**: papka va undagi fayllar, faylning inode'i va uning bloklari. Natija — 10 barobardan ko'proq tezlik.

ext2'dagi **blok guruhlari** — aynan shu g'oya (37-mashqdagi guruh deskriptorlari): har bir guruhning o'z bitmap'lari va inode jadvali bor.

## 27.7. Tok o'chsa nima bo'ladi — izchillik (crash consistency)

**Oddiy qilib aytganda:** fayl tizimi diskda ko'p joyga yozadi, lekin disk ularni **bittadan** yozadi. Tok **orada** o'chsa, ba'zilari yozilgan, ba'zilari yozilmagan bo'ladi — fayl tizimi o'zi bilan **nomuvofiq** bo'lib qoladi.

Misol: faylga bitta blok qo'shish uchun diskka **uch** narsa yozish kerak:

1. **ma'lumot** blokining o'zi;
2. **inode** (yangi blok ko'rsatkichi va yangi hajm);
3. **bitmap** (bu blok endi band).

**Bu dastur nima qiladi (umumiy):** uch yozuvning har bir kombinatsiyasi (jami 2³ = 8 ta) uchun "tok o'chganda faqat shular diskka ulgurgan bo'lsa nima bo'ladi?" ni chiqaradi. Kombinatsiya 3 ta **bit** bilan kodlangan: har bit — bitta yozuv (3-bob, bit operatorlari).

```c
/* yozish_tartibi.c - faylga blok qo'shishda 3 ta yozuvdan qaysilari diskka ulgurdi? */
#include <stdio.h>

int main(void)
{
    const char *nomlar[3] = { "ma'lumot", "inode", "bitmap" };
    printf("%-26s | natija\n", "diskka ulgurgan yozuvlar");
    printf("---------------------------+---------------------------------------------\n");
    for (int ulgurdi = 0; ulgurdi < 8; ulgurdi++) {         /* 3 bit: har bit - bitta yozuv */
        int data = ulgurdi & 1, inode = (ulgurdi >> 1) & 1, bitmap = (ulgurdi >> 2) & 1;

        char nom[64] = "";                                  /* ulgurganlar nomini yig'amiz */
        int uzunlik = 0;
        for (int i = 0; i < 3; i++)
            if (ulgurdi & (1 << i))
                uzunlik += snprintf(nom + uzunlik, sizeof(nom) - uzunlik, "%s%s", uzunlik ? "+" : "", nomlar[i]);
        if (!uzunlik)
            snprintf(nom, sizeof(nom), "(hech narsa)");

        const char *natija;
        if (!inode && !bitmap)
            natija = "izchil (eski holat; ma'lumot yo'qolishi mumkin, fayl buzilmaydi)";
        else if (data && inode && bitmap)
            natija = "izchil (yangi holat) - hammasi yozildi";
        else if (inode && bitmap)
            natija = "metama'lumot izchil, lekin blokda AXLAT o'qiladi";
        else if (inode && data)
            natija = "fayl to'g'ri, lekin bitmap \"bo'sh\" deydi - blok ikkinchi faylga berilishi mumkin";
        else if (inode)
            natija = "inode blokka ko'rsatadi, bitmap \"bo'sh\" deydi, blokda AXLAT - eng yomoni";
        else
            natija = "blok \"band\", lekin hech kim ko'rsatmaydi - SIZIB CHIQISH";
        printf("%-26s | %s\n", nom, natija);
    }
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 yozish_tartibi.c -o yozish_tartibi
$ ./yozish_tartibi
diskka ulgurgan yozuvlar   | natija
---------------------------+---------------------------------------------
(hech narsa)               | izchil (eski holat; ma'lumot yo'qolishi mumkin, fayl buzilmaydi)
ma'lumot                   | izchil (eski holat; ma'lumot yo'qolishi mumkin, fayl buzilmaydi)
inode                      | inode blokka ko'rsatadi, bitmap "bo'sh" deydi, blokda AXLAT - eng yomoni
ma'lumot+inode             | fayl to'g'ri, lekin bitmap "bo'sh" deydi - blok ikkinchi faylga berilishi mumkin
bitmap                     | blok "band", lekin hech kim ko'rsatmaydi - SIZIB CHIQISH
ma'lumot+bitmap            | blok "band", lekin hech kim ko'rsatmaydi - SIZIB CHIQISH
inode+bitmap               | metama'lumot izchil, lekin blokda AXLAT o'qiladi
ma'lumot+inode+bitmap      | izchil (yangi holat) - hammasi yozildi
```

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `for (ulgurdi = 0; ulgurdi < 8; ...)` | 8 ta kombinatsiya: 0 = hech narsa, 7 = hammasi |
| `ulgurdi & 1`, `(ulgurdi >> 1) & 1`, `(ulgurdi >> 2) & 1` | mos ravishda ma'lumot, inode, bitmap yozilganmi (bitni ajratib olish) |
| `snprintf(...)` sikli | yozilganlar nomlarini "ma'lumot+inode" ko'rinishida yig'adi |
| `if ... else if ...` | har kombinatsiya uchun oqibatni aytadi |

**Nima ko'rdik** (har qator — bitta tok o'chish ssenariysi):

| Faqat shu ulgurdi | Oqibat | Og'irligi |
|---|---|---|
| hech narsa / faqat ma'lumot | eski holat qoladi (ma'lumot yo'qoldi, lekin fayl tizimi **izchil**) | muammo emas |
| faqat bitmap | blok "band", hech kim ishlatmaydi — **sizib chiqish** | joy yo'qoladi |
| faqat inode | inode axlat blokka ko'rsatadi, bitmap esa "bo'sh" deydi | **eng yomoni** |
| inode + bitmap | metama'lumot izchil, lekin blokda **axlat** | eski ma'lumot/axlat o'qiladi |
| ma'lumot + inode | fayl to'g'ri, lekin bitmap "bo'sh" deydi — blok **ikkinchi faylga** berilishi mumkin | keyinroq buzilish |
| hammasi | yangi izchil holat | muammo emas |

### Yechim 1: fsck

Yuklanishda **butun diskni tekshirib**, nomuvofiqliklarni tuzatish (Linux `e2fsck` — MyOS testlari uni ishlatadi!). To'g'ri ishlaydi, lekin **juda sekin**: disk qancha katta bo'lsa, shuncha uzoq (terabayt disklarda soatlar).

### Yechim 2: jurnal (write-ahead logging)

**Hayotdan misol: bank xodimining daftari.** Pul o'tkazishdan oldin daftarga yozadi: "A hisobdan 100 ni B ga o'tkazaman". Keyin o'tkazadi, keyin "bajarildi" deb belgilaydi. Chiroq o'chib qolsa, daftarni o'qib, ishni oxiriga yetkazadi yoki bekor qiladi.

Ma'lumotlar bazalaridan olingan g'oya: diskka yozishdan **oldin**, nima qilmoqchi ekaningizni alohida **jurnal** hududiga yozing:

```text
1. Jurnalga: [TxB - boshlanish] [inode] [bitmap] [ma'lumot]          <- yozish
2. Jurnalga: [TxE - tugadi]   (bitta sektor - atomik yoziladi)        <- "commit"
3. Asl joylarga yozish (checkpoint)
4. Jurnaldagi tranzaksiyani bo'shatish
```

Tok o'chsa, yuklanishda jurnal ko'riladi: `TxE` bor tranzaksiyalar **qayta bajariladi** (redo), `TxE` yo'qlari tashlanadi. Tiklash **soniyalar** oladi (fsck'da soatlar o'rniga). ext3/ext4, NTFS, XFS — hammasi jurnalli.

Tezlik uchun ko'pincha faqat **metama'lumot** jurnallanadi (ext4 `data=ordered`: ma'lumot blokini metama'lumotdan **oldin** asl joyiga yozish — shunda inode hech qachon axlatga ko'rsatmaydi).

Jurnalning mini-modelini bobning oxiridagi **mustaqil loyiha**da o'zingiz yozasiz.

### Yechim 3: copy-on-write fayl tizimlari

Hech narsani joyida o'zgartirmaslik: yangi versiyani bo'sh joyga yozib, oxirida ildiz ko'rsatkichini **atomik** almashtirish (ZFS, btrfs, LFS g'oyasi). Bu 27.4 dagi `tmp` + `rename` ning butun disk miqyosidagi ko'rinishi; 24-bobdagi COW va 26-bobdagi RCU bilan ham bir xil g'oya.

> **Eslab qoling:** tok o'chganda nomuvofiqlik bo'ladi. 3 davo: **fsck** (tekshirish, sekin), **jurnal** (avval yozib qo'y, keyin bajar — tez tiklanadi), **COW** (joyida o'zgartirma).

## 27.8. VFS — ko'p fayl tizimini birlashtirish

**Oddiy qilib aytganda:** Linux (va MyOS) bir vaqtda turli fayl tizimlarini ko'rsatadi: diskdagi ext4, xotiradagi tmpfs, `/proc` (yadro holati), `/dev` (qurilmalar). Dastur esa hammasini bir xil `open`/`read` bilan ishlatadi. Buni **VFS**
(virtual file system) qatlami ta'minlaydi: u yo'lga qarab **qaysi fayl tizimi** javob berishini aniqlaydi va uning funksiyalar jadvalidan (7-bobdagi funksiya ko'rsatkichlar) kerakli funksiyani chaqiradi.

**Bu dastur nima qiladi (umumiy):** uchta "fayl tizimi" (`ext2`, `tmpfs`, `procfs`), har birining o'z `oqi` funksiyasi bor. `vfs_oqi` yo'lga qarab (mount nuqtasi bo'yicha) kerakli tizimni tanlaydi. Dasturchi esa bitta `vfs_oqi` ni chaqiradi.

```c
/* vfs_misol.c - VFS g'oyasi: bitta interfeys, turli fayl tizimlari (funksiyalar jadvali) */
#include <stdio.h>
#include <string.h>

struct fs_amallar {                             /* "har bir fayl tizimi shularni biladi" */
    const char *nom;
    int (*oqi)(const char *yol, char *bufer, int hajm);
};

static int disk_oqi(const char *yol, char *b, int h)
{
    return snprintf(b, h, "[ext2] diskdan blok o'qildi: %s", yol);
}

static int proc_oqi(const char *yol, char *b, int h)
{
    return snprintf(b, h, "[procfs] yadro holatidan hosil qilindi: %s", yol);
}

static int tmp_oqi(const char *yol, char *b, int h)
{
    return snprintf(b, h, "[tmpfs] xotiradan olindi: %s", yol);
}

static struct { const char *mount; struct fs_amallar fs; } ulangan[] = {
    { "/proc", { "procfs", proc_oqi } },
    { "/tmp",  { "tmpfs",  tmp_oqi } },
    { "/",     { "ext2",   disk_oqi } },        /* ildiz - oxirida: eng uzun mos kelgani tanlanadi */
};

static int vfs_oqi(const char *yol, char *bufer, int hajm)
{
    for (unsigned i = 0; i < sizeof(ulangan) / sizeof(ulangan[0]); i++) {
        size_t m = strlen(ulangan[i].mount);
        if (strncmp(yol, ulangan[i].mount, m) == 0 && (yol[m] == '/' || yol[m] == '\0' || m == 1))
            return ulangan[i].fs.oqi(yol + (m == 1 ? 0 : m), bufer, hajm);
    }
    return -1;
}

int main(void)
{
    const char *yollar[] = { "/etc/hostname", "/proc/meminfo", "/tmp/a.txt", "/home/men/kundalik" };
    char b[128];
    for (int i = 0; i < 4; i++) {
        vfs_oqi(yollar[i], b, sizeof(b));
        printf("open(\"%s\")\n   -> %s\n", yollar[i], b);
    }
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 vfs_misol.c -o vfs_misol
$ ./vfs_misol
open("/etc/hostname")
   -> [ext2] diskdan blok o'qildi: /etc/hostname
open("/proc/meminfo")
   -> [procfs] yadro holatidan hosil qilindi: /meminfo
open("/tmp/a.txt")
   -> [tmpfs] xotiradan olindi: /a.txt
open("/home/men/kundalik")
   -> [ext2] diskdan blok o'qildi: /home/men/kundalik
```

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `struct fs_amallar` | "har bir fayl tizimi biladigan amallar" — bu yerda faqat `oqi` (funksiya ko'rsatkichi) |
| `ulangan[]` | mount jadvali: yo'l boshlanishi → fayl tizimi |
| `vfs_oqi(yol, ...)` | jadvalni aylanadi, yo'l qaysi mount bilan boshlansa, o'sha tizimning `oqi` ini chaqiradi |
| `"/"` oxirda | ildiz hamma yo'lga mos keladi, shuning uchun eng oxirida tekshiriladi (aniqroq mount avval) |

**Nima ko'rdik:** `/proc/meminfo` → procfs, `/tmp/a.txt` → tmpfs (mount nuqtasi olib tashlangan yo'l uzatiladi), qolganlari → ext2. Chaqiruvchi hech qaysi tizim ekanini bilmaydi. MyOS: `kernel/fs/vfs.c`, `docs/12-vfs.md`.

## Hayotdan misol va to'liq dastur

**Kundalik saqlovchi.** Bobda ko'rilgan hamma narsani birlashtiruvchi dastur: xavfsiz saqlash (`tmp` + `fsync` + `rename`), havolalar va inode.

**Bu dastur nima qiladi (umumiy):** `kundalik.txt` ni uch marta xavfsiz usulda saqlaydi (har safar yangi mazmun), keyin unga qattiq va ramziy havola yaratadi, hamma nomlarning holatini ko'rsatadi; keyin asl nomni o'chiradi va oqibatini ko'rsatadi.

```c
/* saqlovchi.c - atomik saqlash (tmp + fsync + rename), havolalar va inode */
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* Faylni xavfsiz saqlash: hech qachon yarim yozilgan holatda qolmaydi */
static int xavfsiz_saqla(const char *nom, const char *matn)
{
    char tmp[256];
    snprintf(tmp, sizeof(tmp), "%s.tmp", nom);
    int fd = open(tmp, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
        return -1;
    size_t n = strlen(matn);
    if (write(fd, matn, n) != (ssize_t)n || fsync(fd) != 0) {   /* fsync - haqiqatan diskka */
        close(fd);
        unlink(tmp);
        return -1;
    }
    close(fd);
    return rename(tmp, nom);                    /* bitta harakat: eski -> yangi */
}

static void korsat(const char *nom)
{
    struct stat st;
    if (lstat(nom, &st) != 0) {
        printf("  %-14s yo'q\n", nom);
        return;
    }
    printf("  %-14s nomlar soni %ld, hajm %2ld bayt%s\n", nom, (long)st.st_nlink, (long)st.st_size,
           S_ISLNK(st.st_mode) ? " (yo'llanma qog'oz)" : "");
}

int main(void)
{
    for (int kun = 1; kun <= 3; kun++) {
        char matn[64];
        snprintf(matn, sizeof(matn), "%d-kun: hammasi yaxshi\n", kun);
        if (xavfsiz_saqla("kundalik.txt", matn) != 0)
            perror("saqlash");
    }

    unlink("zaxira.txt");
    unlink("yollanma.txt");
    if (link("kundalik.txt", "zaxira.txt") != 0)          /* bitta kitobga ikkinchi nom */
        perror("link");
    if (symlink("kundalik.txt", "yollanma.txt") != 0)     /* yo'llanma qog'oz */
        perror("symlink");

    struct stat a, b;
    stat("kundalik.txt", &a);
    stat("zaxira.txt", &b);
    printf("Uch marta saqlandi. Havolalar yaratildi:\n");
    korsat("kundalik.txt");
    korsat("zaxira.txt");
    korsat("yollanma.txt");
    printf("kundalik.txt va zaxira.txt bitta inode'mi: %s\n", a.st_ino == b.st_ino ? "ha" : "yo'q");

    unlink("kundalik.txt");                     /* asl nomni o'chiramiz */
    printf("\nkundalik.txt o'chirildi:\n");
    korsat("zaxira.txt");
    printf("  yollanma.txt orqali ochish: %s\n",
           open("yollanma.txt", O_RDONLY) < 0 ? "bo'lmadi - yo'llanma hech qayerga olib bormaydi" : "bo'ldi");

    unlink("zaxira.txt");
    unlink("yollanma.txt");
    return 0;
}
```

```console
$ gcc -Wall -Wextra saqlovchi.c -o saqlovchi
$ ./saqlovchi
Uch marta saqlandi. Havolalar yaratildi:
  kundalik.txt   nomlar soni 2, hajm 22 bayt
  zaxira.txt     nomlar soni 2, hajm 22 bayt
  yollanma.txt   nomlar soni 1, hajm 12 bayt (yo'llanma qog'oz)
kundalik.txt va zaxira.txt bitta inode'mi: ha

kundalik.txt o'chirildi:
  zaxira.txt     nomlar soni 1, hajm 22 bayt
  yollanma.txt orqali ochish: bo'lmadi - yo'llanma hech qayerga olib bormaydi
```

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `xavfsiz_saqla` | `.tmp` ga yozadi → `fsync` (haqiqatan diskka) → `rename` (atomik almashtirish). Xato bo'lsa `.tmp` o'chiriladi, eski fayl daxlsiz |
| `korsat` | `lstat` bilan nomlar soni va hajmni ko'rsatadi; havolani (`S_ISLNK`) belgilaydi |
| `link` / `symlink` | qattiq va ramziy havola |
| `a.st_ino == b.st_ino` | ikki nom bitta inode'mi? |
| `unlink("kundalik.txt")` | asl nomni o'chiradi |

**Nima ko'rdik:** uch saqlashdan keyin mazmun 22 bayt (`3-kun: hammasi yaxshi\n`). `kundalik.txt` va `zaxira.txt` — bir inode, nomlar soni 2. `yollanma.txt` — alohida kichik fayl (12 bayt = "kundalik.txt" matni). Asl nom
o'chirgach: `zaxira.txt` hali yashaydi (nomlar soni 1) — fayl yo'qolmadi; `yollanma.txt` esa osilib qoldi — ochilmadi.

**Sinab ko'ring:** terminalda: `echo salom > a; ln a b; ln -s a c; ls -li a b c` — inode raqamlarini solishtiring. `xavfsiz_saqla` ichida `rename` dan oldin `return -1;` qo'yib, eski kundalik buzilmay qolishini tekshiring.

<!-- katta:boshi -->
## Katta loyiha: mfs — o'zimizning fayl tizimi (disk tasviri ustida)

**Umumiy fikr.** Disk — shunchaki **bloklar massivi** (har biri 512 yoki 4096 bayt). "Fayl", "papka", "nom" tushunchalari diskda **yo'q**: ularni **fayl tizimi** (ext4, FAT, NTFS) o'ylab topgan va bloklar ichiga **tartibli yozib** qo'ygan. Bu bosqichda mini fayl tizimini noldan yozamiz: u **oddiy fayl ichida** (disk tasviri, `disk.img`) yashaydi, shuning uchun hech narsani buzmaymiz. Yozish, o'qish, o'chirish **va avariyadan keyin tekshirish (`fsck`)** — 27-bobning hammasi.

**Hayotiy o'xshatish:** katta kutubxona. **Mundarija kartochkalari** (inode) har kitob qaysi javonlarda ekanini aytadi; **katalog** — "kitob nomi → kartochka raqami"; **bitmap** — "qaysi javon band" jadvali. Kitob topish uchun avval katalog, keyin kartochka, keyin javonlar.

### Disk tuzilishi

```text
blok 0       blok 1     bloklar 2..5         blok 6        bloklar 7..255
+-----------+----------+--------------------+-------------+----------------+
| superblok | bitmap   | inode jadvali      | ildiz       | fayl           |
| (magik)   | (band?)  | (32 ta inode)      | katalog     | ma'lumotlari   |
+-----------+----------+--------------------+-------------+----------------+
```

| Element | Vazifasi |
|---|---|
| **superblok** | fayl tizimi **sehrli soni** (`MFS1`) va o'lchamlari: bu tasvir haqiqatan mfs ekanini tekshirish |
| **bitmap** | 1 bit = 1 blok: 1 — band, 0 — bo'sh (16-bob, bitmap) |
| **inode** (64 bayt) | **fayl haqida hamma narsa** (tur, hajm, **blok raqamlari**) — faqat **nomi** yo'q |
| **katalog** | `(nom → inode raqami)` juftliklari jadvali |
| **blok** | 512 bayt: diskka o'qish/yozishning eng kichik birligi |

**Muhim fikr:** fayl nomi **inode ichida emas**, katalogda saqlanadi. Shuning uchun bitta faylga **ikki nom** (hard link) berish mumkin, va `rm` — aslida "katalogdan yozuvni olib tashlash".

**Strukturalar:**

```c
struct super {                                  /* 0-blok */
    uint32_t magik, bloklar, inodelar, data_bosh;
};
```

```c
struct inode {                                  /* 64 bayt: 512 / 64 = 8 ta inode bir blokda */
    uint16_t tur;                               /* 0 - bo'sh, 1 - fayl, 2 - katalog */
    uint16_t nlink;
    uint32_t hajm;
    uint32_t blok[TOGRI];
    uint8_t to_ldiruvchi[8];
};
```

```c
struct yozuv {                                  /* katalogdagi bitta nom: (nom -> inode raqami), 16 bayt */
    uint16_t inode;                             /* 0 - bo'sh yozuv (inode 0 - ildiz katalog, nom emas) */
    char nom[14];
};
```

`struct inode` da **12 ta to'g'ridan-to'g'ri blok raqami** bor: fayl eng ko'pi 12 × 512 = 6144 bayt (haqiqiy ext2 da yana bilvosita bloklar bor).

### Qanday ishlaydi: eng past daraja

Hammasi **blok o'qish/yozish** ustiga quriladi: `fseek(n * 512)` + `fread`/`fwrite`. Ko'p baytli inode ni blokdan olish uchun `memcpy` ishlatiladi:

```c
static void blok_oqi(unsigned n, void *b)
{
    fseek(tasvir, (long)n * BLOK, SEEK_SET);
    if (fread(b, BLOK, 1, tasvir) != 1) {
        fprintf(stderr, "mfs: blok %u o'qilmadi\n", n);
        exit(1);
    }
}
```

```c
static void inode_oqi(unsigned i, struct inode *x)
{
    uint8_t b[BLOK];
    blok_oqi(INODE_BOSH + i / 8, b);
    memcpy(x, b + (i % 8) * sizeof(*x), sizeof(*x));
}
```

### Fayl yozish: tartib muhim!

`put` (fayl qo'shish) **beshta ish** qiladi: (1) bo'sh inode topish, (2) bloklar ajratish, (3) ma'lumotni yozish, (4) inode yozish, (5) katalogga nom qo'shish. Tartib **tasodifiy emas** — kompyuter **istalgan paytda o'chib qolishi** mumkin:

| Tartib | Uzilsa nima bo'ladi |
|---|---|
| ma'lumot → bitmap → inode → **katalog (oxirida)** | yarim yozilgan fayl **ko'rinmaydi** (katalogda yo'q). Eng yomoni — **sizib chiqish**: bitmapda band blok, lekin hech kim ishlatmaydi |
| (teskari) katalog **oldin** → keyin ma'lumot | nom ko'rinadi, ichida esa **axlat** yoki bo'sh |

Mfs shu **xavfsiz tartibni** qo'llaydi: ma'lumot → bitmap → inode → katalog. `rm` esa **teskari**: katalog → inode → bitmap (uzilsa ham faqat sizib chiqish bo'ladi). Bu — **izchillik** (consistency) tamoyili; haqiqiy fayl tizimlarida **jurnal** (journaling) buni yanada ishonchli qiladi.

```c
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
```

### fsck: izchillikni tekshirish

`fsck` hamma inode lar **haqiqatan ishlatayotgan** bloklarni yig'adi va bitmap bilan **solishtiradi**:

| Holat | Ma'nosi | Xavfi |
|---|---|---|
| bitmapda **band**, hech kim ishlatmaydi | **SIZIB CHIQISH** (leak) | joy yo'qoladi, xavfsiz |
| bitmapda **bo'sh**, lekin fayl ishlatadi | **XAVFLI** | blok boshqa faylga berilib, **ma'lumot buziladi** |
| bitta blokni **ikki** inode ishlatadi | **XATO** | ikki fayl bir-birining ustiga yozadi |

```c
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
```

Ishga tushiramiz: tasvirni yaratamiz, ikki fayl yozamiz, o'qiymiz, bittasini o'chiramiz, keyin **avariyani modellaymiz** (`hack` bitmapni ataylab buzadi) va `fsck` ni ishlatamiz:

```console
$ cd katta_loyiha/tizim/27_mfs
$ gcc -Wall -Wextra -g -fsanitize=address,undefined mfs.c -o mfs
$ ./mfs mkfs disk.img
mfs yaratildi: 256 blok x 512 bayt, 32 inode
$ echo "salom, fayl tizimi" > a.txt
$ head -c 1500 /dev/zero | tr '\0' 'x' > katta.txt
$ ./mfs put disk.img a.txt salom
yozildi: salom (19 bayt, inode 1, 1 blok)
$ ./mfs put disk.img katta.txt katta
yozildi: katta (1500 bayt, inode 2, 3 blok)
$ ./mfs ls disk.img
  salom          inode 1      19 bayt
  katta          inode 2    1500 bayt
2 ta fayl; 11/256 blok band
$ ./mfs cat disk.img salom
salom, fayl tizimi
$ ./mfs fsck disk.img
fsck: fayl tizimi izchil (muammo yo'q)
$ ./mfs rm disk.img salom
o'chirildi: salom
$ ./mfs ls disk.img
  katta          inode 2    1500 bayt
1 ta fayl; 10/256 blok band
$ ./mfs hack disk.img 100
bitmap buzildi: blok 100 'band' deb belgilandi
$ ./mfs fsck disk.img; echo "fsck chiqish kodi: $?"
  SIZIB CHIQISH: blok 100 bitmapda band, lekin hech bir inode ishlatmaydi
fsck: muammolar topildi
fsck chiqish kodi: 1
$ ./mfs fsck disk.img -t
  SIZIB CHIQISH: blok 100 bitmapda band, lekin hech bir inode ishlatmaydi -> bo'shatildi
fsck: muammolar topildi va TUZATILDI
$ ./mfs fsck disk.img
fsck: fayl tizimi izchil (muammo yo'q)
```

**Nima ko'rdik:**

- `salom` — **1 blok** (19 bayt), `katta` — **3 blok** (1500 bayt: 1500/512 → yuqoriga yaxlitlash = 3). Band: **11 / 256** blok = 6 (metama'lumot) + 1 (ildiz katalog) + 1 + 3.
- `salom` o'chirilgach: 10 blok band — bitmapdagi bit **qaytarildi**.
- `hack disk.img 100` — blok 100 "band" deb belgilandi, lekin **hech bir inode uni ishlatmaydi**: bu kutilmagan uzilishdan keyingi **sizib chiqish** holatining aynan o'zi.
- `fsck` (tekshirish rejimi) muammoni **topdi**, lekin **tuzatmadi** (chiqish kodi **1**: skriptlar buni ishlatadi).
- `fsck -t` (tuzatish rejimi) blokni **bo'shatdi**; keyingi `fsck`: **"izchil"**.

> **Eslab qoling:** fayl tizimi = **bloklar + metama'lumot** (superblok, bitmap, inode, katalog). **Nom — katalogda**, fayl haqida hamma narsa — **inode da**. Diskka yozish **tartibi** uzilishdan keyingi holatni belgilaydi: **ma'lumot oldin, "e'lon qilish" (katalog) oxirida**. `fsck` — metama'lumotlarni **bir-biri bilan solishtirib** buzilishni topadi.

**O'zingiz qo'shing (yechimsiz):**

1. **Hard link:** `mfs ln disk.img salom ikkinchi_nom` — bitta inode ga ikkinchi katalog yozuvi. `nlink` ni oshiring; `rm` da faqat `nlink == 0` bo'lganda bloklarni qaytaring.
2. `put` ichida tartibni **buzing** (katalogni inode dan **oldin** yozing) va ichida `exit(1)` bilan **uzilishni modellang**: `fsck` nimani topadi?
3. `fsck` ga yangi tekshiruv qo'shing: katalogda **mavjud bo'lmagan** (tur = 0) inode ga ishora qilgan yozuv — qanday xato nomi bering va uni tuzatish usulini o'ylab toping.
<!-- katta:oxiri -->

## Bob xulosasi (yodlash uchun)

1. Qurilma registrlar orqali boshqariladi. **Polling** — tinmay so'rash (CPU yonadi), **uzilish** — uxlab, qurilma uyg'otsin, **DMA** — nusxani qurilma qiladi. **Drayver** — qurilmaga xos kod; ustida umumiy blok interfeysi.
2. **HDD:** izlash + aylanish (ms) — tasodifiy murojaat ketma-ketdan yuzlab marta sekin; shuning uchun fayl tizimlari ketma-ketlikni qidiradi. **SSD:** mexanika yo'q, FTL va TRIM.
3. Nom **katalogda**, hamma boshqa narsa **inode**'da. Qattiq havola — yana bir nom (bir inode); ramziy — yo'l matni yozilgan fayl. Fayl nomlar soni 0 **va** ochiq tutqich yo'q bo'lganda o'chadi.
4. Tuzilma: superblok + bitmap'lar + inode jadvali + ma'lumot bloklari; inode bloklarni 12 bevosita + bilvosita/ikki/uch karra bilan ko'rsatadi; `open` yo'li = har komponent uchun katalog o'qish; FFS/blok guruhlari bog'liq narsalarni yonma-yon qo'yadi.
5. **Tok o'chsa** nomuvofiqlik: davo — `tmp`+`fsync`+`rename` (dasturda), **fsck** (sekin), **jurnal** (avval yozib qo'y), **COW**. **VFS** — turli fayl tizimlarini bitta interfeys ostida birlashtiradi.

## Savol-javob

**Savol:** `write` qaytdi — ma'lumot diskdami?
**Javob:** Yo'q. `write` odatda faqat OS keshiga yozadi. Diskka haqiqatan tushganini `fsync` kafolatlaydi.

**Savol:** Nega `rm` katta faylni o'chirgach disk joyi bo'shamadi?
**Javob:** Kimdir faylni hali ochib turibdi (27.3). Nom ketgan, inode va bloklar ochiq tutqich yopilguncha yashaydi.

**Savol:** Qattiq havola nega papkalarga ruxsat etilmaydi?
**Javob:** Katalog daraxtida sikl hosil bo'lishi mumkin (papka o'z ichidagi papkaga havola) — o'tish dasturlari abadiy aylanib qoladi.

## O'zingizni tekshiring

1. Polling va uzilish — qachon qaysi biri yaxshi?
2. DMA nima va nega kerak?
3. HDD'da tasodifiy va ketma-ket o'qish nega yuzlab marta farq qiladi?
4. 4 KB blok bilan bevosita ko'rsatkichlar qancha ma'lumotni qamraydi? Bilvosita-chi?
5. Faylga blok qo'shishda tok o'chdi: faqat inode yozilgan. Nima muammo? Jurnal buni qanday hal qiladi?

<details><summary>Javoblar</summary>

1. Tez qurilma/qisqa kutish — polling; sekin qurilma — uzilishlar (CPU boshqa ish qiladi).
2. Qurilma xotiraga CPU'siz, fizik manzil bo'yicha nusxalaydi — CPU vaqti tejaladi.
3. Tasodifiyda har murojaat uchun kallak surilishi va aylanish kutiladi (millisekundlar), ketma-ketda — faqat uzatish.
4. 12 × 4 KB = 48 KB; bilvosita + 1024 blok = 4 MB.
5. Inode axlat blokka ko'rsatadi, bitmap esa "bo'sh" deydi. Jurnalda uchala yozuv `TxE` bilan birga turadi — tiklashda hammasi qayta bajariladi yoki hech biri.
</details>

## Mashq

- **37** (ext2 o'qish) — agar hali qilmagan bo'lsangiz. Keyin unga faylni yozishni qo'shishni o'ylab ko'ring: qaysi tartibda yozish xavfsizroq?
- MyOS: `docs/13-disk-ext2.md` va `tools/test.sh` dagi `e2fsck` tekshiruvi — yadro yozgan diskni Linux qanday tekshiradi.

<!-- loyiha:boshi -->
## Loyiha: xotiradagi mini fayl tizimi

**Maqsad:** fayl tizimining ichki tuzilmasini o'z qo'lingiz bilan yasash: **inode** (fayl haqida ma'lumot), **bloklar**, blok **bitmap**i va nomlar.
Disk o'rniga oddiy massiv ishlatamiz — g'oya bir xil (27.3–27.4).
**Bobdan ishlatiladi:** inode, blok bitmap, blok ajratish/qaytarish, fayl hajmi va bloklar.

**Talab:** 16 ta blok (har biri 16 bayt); har fayl ko'pi bilan 4 blok. 8 ta inode. Funksiyalar:
- `fs_yarat(nom)` — bo'sh inode oladi (0 = muvaffaqiyat, −1 = joy yo'q yoki nom band);
- `fs_yoz(nom, matn)` — faylga **qo'shib** yozadi, kerak bo'lganda bloklar ajratadi (bitmap orqali);
- `fs_oqi(nom, bufer, hajm)` — mazmunni o'qiydi;
- `fs_ochir(nom)` — inode ni ozod qiladi va bloklarni bitmap'ga qaytaradi;
- `fs_holat()` — bitmap va fayllarni ko'rsatadi.

**Ma'lumotlar:** `struct inode { int band; char nom[12]; int hajm; int blok[4]; }`, `char disk[16][16]`, `uint16_t bitmap` (1 = band blok).

```c
/* mfs.c - xotiradagi mini fayl tizimi */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define BLOKLAR 16
#define BLOK_HAJM 16
#define INODELAR 8
#define FAYL_BLOK 4

struct inode {
    int band;
    char nom[12];
    int hajm;
    int blok[FAYL_BLOK];
};

static char disk[BLOKLAR][BLOK_HAJM];
static uint16_t bitmap;                         /* i-bit = 1: i-blok band */
static struct inode inodelar[INODELAR];

static int blok_ol(void)                        /* birinchi bo'sh blok yoki -1 */
{
    for (int i = 0; i < BLOKLAR; i++)
        if (!(bitmap & (1u << i))) {
            bitmap |= (uint16_t)(1u << i);
            return i;
        }
    return -1;
}

static struct inode *top(const char *nom)
{
    for (int i = 0; i < INODELAR; i++)
        if (inodelar[i].band && strcmp(inodelar[i].nom, nom) == 0)
            return &inodelar[i];
    return NULL;
}

static int fs_yarat(const char *nom)
{
    if (top(nom))
        return -1;
    for (int i = 0; i < INODELAR; i++)
        if (!inodelar[i].band) {
            memset(&inodelar[i], 0, sizeof(inodelar[i]));
            inodelar[i].band = 1;
            snprintf(inodelar[i].nom, sizeof(inodelar[i].nom), "%s", nom);
            return 0;
        }
    return -1;
}

static int fs_yoz(const char *nom, const char *matn)
{
    struct inode *f = top(nom);
    if (!f)
        return -1;
    for (const char *p = matn; *p; p++) {
        int k = f->hajm / BLOK_HAJM;            /* fayl ichidagi blok raqami */
        if (k >= FAYL_BLOK)
            return -2;                          /* fayl hajmi chegarasi */
        if (f->hajm % BLOK_HAJM == 0) {         /* yangi blok kerak */
            int b = blok_ol();
            if (b < 0)
                return -3;                      /* disk to'ldi */
            f->blok[k] = b;
        }
        disk[f->blok[k]][f->hajm % BLOK_HAJM] = *p;
        f->hajm++;
    }
    return 0;
}

static int fs_oqi(const char *nom, char *bufer, int hajm)
{
    struct inode *f = top(nom);
    if (!f)
        return -1;
    int n = f->hajm < hajm - 1 ? f->hajm : hajm - 1;
    for (int i = 0; i < n; i++)
        bufer[i] = disk[f->blok[i / BLOK_HAJM]][i % BLOK_HAJM];
    bufer[n] = '\0';
    return n;
}

static void fs_ochir(const char *nom)
{
    struct inode *f = top(nom);
    if (!f)
        return;
    for (int k = 0; k * BLOK_HAJM < f->hajm; k++)
        bitmap &= (uint16_t)~(1u << f->blok[k]);        /* bloklarni qaytaramiz */
    f->band = 0;
}

static void fs_holat(const char *izoh)
{
    printf("%s\n  bitmap: ", izoh);
    for (int i = 0; i < BLOKLAR; i++)
        putchar(bitmap & (1u << i) ? '1' : '0');
    printf("\n");
    for (int i = 0; i < INODELAR; i++)
        if (inodelar[i].band) {
            printf("  %-8s hajm %2d, bloklar:", inodelar[i].nom, inodelar[i].hajm);
            for (int k = 0; k * BLOK_HAJM < inodelar[i].hajm; k++)
                printf(" %d", inodelar[i].blok[k]);
            printf("\n");
        }
}

int main(void)
{
    char bufer[128];
    fs_yarat("salom");
    fs_yarat("yadro");
    fs_yoz("salom", "Salom, dunyo!");                   /* 13 bayt: 1 blok */
    fs_yoz("yadro", "Yadro yozish oson emas, lekin qiziq.");    /* 36 bayt: 3 blok */
    fs_holat("Ikki fayl yaratildi:");

    fs_yoz("salom", " Xush kelibsiz!");                /* 13+15=28: yana 1 blok kerak */
    fs_oqi("salom", bufer, sizeof(bufer));
    printf("salom mazmuni: \"%s\"\n", bufer);
    fs_holat("salom kengaydi:");

    fs_ochir("salom");
    fs_holat("salom o'chirildi (bloklar qaytdi):");

    fs_yarat("yangi");
    fs_yoz("yangi", "abc");
    fs_holat("yangi fayl bo'sh blokni (0-blok) qayta ishlatdi:");

    printf("64 baytdan katta yozish: kod %d\n", fs_yoz("yadro", "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"));
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address,undefined mfs.c -o mfs
$ ./mfs
Ikki fayl yaratildi:
  bitmap: 1111000000000000
  salom    hajm 13, bloklar: 0
  yadro    hajm 36, bloklar: 1 2 3
salom mazmuni: "Salom, dunyo! Xush kelibsiz!"
salom kengaydi:
  bitmap: 1111100000000000
  salom    hajm 28, bloklar: 0 4
  yadro    hajm 36, bloklar: 1 2 3
salom o'chirildi (bloklar qaytdi):
  bitmap: 0111000000000000
  yadro    hajm 36, bloklar: 1 2 3
yangi fayl bo'sh blokni (0-blok) qayta ishlatdi:
  bitmap: 1111000000000000
  yangi    hajm  3, bloklar: 0
  yadro    hajm 36, bloklar: 1 2 3
64 baytdan katta yozish: kod -2
```

Bitmap nomerlar: `1` — blok band. `salom` o'chirilgach 0- va 4-bloklar bo'shadi; yangi fayl birinchi bo'sh blokni (0-blok) oldi.
Haqiqiy fayl tizimlari (ext2, 27.4) ham xuddi shu tarzda ishlaydi — faqat katta o'lchamda va diskda.

**Kengaytiring:** `fs_royxat()` (nomlar va hajmlar), `fs_qisqartir(nom)`. Fayl 4 blokdan katta bo'lolmasligi — real tizimda qanday hal qilinadi? (Bilvosita bloklar, 27.4.)

## Mustaqil loyiha: jurnal va avariyadan tiklash ★★★

**Vazifa:** tok o'chsa nima bo'ladi? Pul o'tkazmasi ikki yozuvdan iborat (bir hisobdan ayirish, ikkinchisiga qo'shish) — orada tok
o'chsa, pul **yo'qoladi**. Fayl tizimlari (ext4, NTFS) buni **jurnal** (write-ahead log) bilan hal qiladi (27.6). Siz shu mexanizmni
kichik modelda yozasiz. Fayl: `jurnal.c`.

**Model:** "disk" — 4 ta hisob `hisob[4]` (boshida `100, 50, 0, 0`) va **jurnal**: `struct jurnal { int haqiqiy, commit; int dan, ga; int dan_yangi, ga_yangi; }`.
Har "diskka yozuv" — bitta qadam; avariya har qadamdan **keyin** bo'lishi mumkin.

**O'tkazma `otkazma(dan, ga, summa, avariya_qadami)`** — quyidagi **5 qadam** aynan shu tartibda; `avariya_qadami = k` bo'lsa, `k` ta qadam
bajarilgach dastur "qulaydi" (funksiya darhol qaytadi):

| Qadam | Nima qilinadi |
|---|---|
| 1 | jurnalga yozish: `haqiqiy=1, commit=0`, `dan`, `ga`, ikkala hisobning **yangi** qiymatlari |
| 2 | `commit = 1` (tranzaksiya "qat'iy") |
| 3 | `hisob[dan] = dan_yangi` |
| 4 | `hisob[ga] = ga_yangi` |
| 5 | jurnalni tozalash (`haqiqiy=0, commit=0`) |

**Tiklash `tiklash()`** (qayta yoqilganda): 
- jurnal `haqiqiy` emas → hech narsa qilmaydi (kod **0**);
- `haqiqiy` bo'lsa, lekin `commit=0` → tashlab yuboradi (kod **1**) — tranzaksiya "bo'lmagan" deb hisoblanadi;
- `commit=1` → jurnaldagi yangi qiymatlarni hisoblarga **qayta yozadi** (bu amal **idempotent**: ikki marta qilinsa ham zarar yo'q) va jurnalni tozalaydi (kod **2**).

**Sinov (aniq).** Har `k = 0..5` uchun: disk boshlang'ich holatga qaytariladi, `otkazma(0, 1, 30, k)` bajariladi, `tiklash()` chaqiriladi va
holat chiqariladi. Keyin **jurnalsiz** variant: ikki yozuv (`hisob[dan] -= s`, `hisob[ga] += s`) — 1-yozuvdan keyin avariya.

**Kutilgan natija** (`darslik/loyihalar/27_jurnal/kutilgan.txt`):

```text
Jurnal bilan (otkazma 0 -> 1, 30 so'm; boshida 100, 50):
  avariya 0 qadamdan keyin: A=100 B=50 jami=150 (tiklash kerak emas)
  avariya 1 qadamdan keyin: A=100 B=50 jami=150 (jurnal tashlandi)
  avariya 2 qadamdan keyin: A=70 B=80 jami=150 (jurnal qayta o'ynaldi)
  avariya 3 qadamdan keyin: A=70 B=80 jami=150 (jurnal qayta o'ynaldi)
  avariya 4 qadamdan keyin: A=70 B=80 jami=150 (jurnal qayta o'ynaldi)
  avariya 5 qadamdan keyin: A=70 B=80 jami=150 (tiklash kerak emas)
Jurnalsiz, 1-yozuvdan keyin avariya: A=70 B=50 jami=120 (BUZILDI)
```

**Maslahat** (yechim emas):
- `k` qadamdan keyin to'xtash: har qadam oldidan `if (bajarildi == avariya_qadami) return;` yoki qadamlar `switch` ichida `case` dan `case` ga o'tsin.
- O'zgarmas (invariant): **yig'indi doim 150**. Jurnal bilan har `k` uchun bu to'g'ri: yo eski holat (`100, 50`), yo yangi (`70, 80`) — hech qachon oraliq.
- `tiklash` qayta o'ynashda jurnaldagi **yangi qiymatlarni** yozing, o'zgarish (`-=`, `+=`) emas! Nega? (Idempotentlik: ikki marta tiklash yoki tiklash paytida yana avariya.)
- Jurnalsiz variantda yig'indini chiqaring va `BUZILDI` ni ko'ring.
- Bu — real fayl tizimidagi "commit" bloki, "replay" va `fsck` ning modeli.

**Tekshirish:**

```bash
gcc -Wall -Wextra -g -fsanitize=address,undefined jurnal.c -o dastur && ./dastur | diff - ~/C_loyha/darslik/loyihalar/27_jurnal/kutilgan.txt && echo "TO'G'RI"
```
<!-- loyiha:oxiri -->

Keyingi bob: [28-bob. Algoritmlar va ma'lumotlar tuzilmalari](28-algoritmlar.md)
