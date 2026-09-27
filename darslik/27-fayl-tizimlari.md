# 27-bob. Qurilmalar va fayl tizimlari

> **Bu bobdan keyin:** OS qurilmalar bilan qanday gaplashishini (polling, uzilishlar, DMA, drayverlar),
> HDD va SSD qanday ishlashini, fayl tizimini noldan qanday loyihalashni (inode, bitmap, papkalar),
> FFS g'oyasini va eng muhimi — **tok o'chsa nima bo'ladi** (fsck, journaling) masalasini bilasiz.
> (OSTEP "Persistence" qismi.) Mashqlar: 25, 26, 37.

> **To'liq ishlaydigan misol:** [misollar/27_fayllar.c](misollar/27_fayllar.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Hayotdan misollar

**Qurilma bilan gaplashish — kuryer xizmati (27.1).** Diskdan ma'lumot so'rash — kuryer chaqirishga
o'xshaydi: buyurtma berasiz va kutasiz. Kutish vaqtida boshqa ish qilasiz (protsessor boshqa jarayonni
ishlatadi). Kuryer kelganda qo'ng'iroq qiladi — bu **uzilish** (interrupt). Har daqiqada eshikka chiqib
qarash esa — **polling**.

**Disk — ulkan kutubxona binosi (27.2).** Qattiq diskda kitob izlash uchun kutubxonachi zinapoyadan
kerakli qavatga chiqadi (seek) va javon aylanib kelishini kutadi (rotation). Ketma-ket joylashgan
kitoblarni olish tez, tarqoqlarni — juda sekin. SSD'da harakatlanadigan qism yo'q, lekin u ham bloklar
bilan ishlaydi.

**Inode — kutubxona katalog kartochkasi (27.3).** Har bir kitob uchun kartochka: hajmi, qachon
qo'shilgan, kim o'qishi mumkin va **sahifalari qaysi javonlarda**. Kitobning **nomi** esa kartochkada yo'q!
Nom — katalogda.

**Katalog — mundarija (27.3).** Mundarijada faqat "nom → kartochka raqami" yozilgan. `ls -i` aynan
shu raqamlarni ko'rsatadi.

**Qattiq havola — bitta kitobga ikki nom (27.3).** Mundarijaning ikki joyida bir xil kartochka raqami:
"Sariq devni minib" va "Hoshimjon sarguzashtlari" — aslida bitta kitob. Kartochkada "nomlar soni: 2"
yozilgan. Bitta nomni o'chirsangiz, kitob yo'qolmaydi — nomlar soni 0 bo'lgandagina.

**Ramziy havola — yo'llanma qog'oz (27.3).** "Bu kitobni 'Sariq devni minib' nomi bilan qidiring" degan
qog'oz. O'sha kitob o'chirilsa, qog'oz qoladi, lekin hech qayerga olib bormaydi ("osilib qolgan" havola).

**Tok o'chsa — jurnal (27.6).** Bank xodimi pul o'tkazishdan oldin daftarga yozadi: "A hisobdan 100 ni
B ga o'tkazaman". Keyin o'tkazadi, keyin "bajarildi" deb belgilaydi. Chiroq o'chib qolsa, daftarni o'qib,
ishni oxiriga yetkazadi yoki bekor qiladi. Fayl tizimining jurnali (ext4, NTFS) aynan shunday.

**`rename` bilan saqlash — e'lonni almashtirish (27.6).** Devordagi e'lonni yangilash uchun eskisini
yirtib, yangisini yozishni boshlamaysiz (shu orada kimdir bo'sh devorni ko'radi). Yangisini oldindan tayyorlab,
**bitta harakat** bilan almashtirasiz. `rename` ham atomik: fayl yo eski, yo yangi — hech qachon yarim.

### To'liq dastur: kundalik saqlovchi

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

**Sinab ko'ring:** terminalda: `echo salom > a; ln a b; ln -s a c; ls -li a b c` — inode raqamlarini
solishtiring. `xavfsiz_saqla` ichida `rename` dan oldin `return -1;` qo'yib, eski kundalik buzilmay
qolishini tekshiring.

## 27.1. Qurilma bilan gaplashish

Qurilma (disk, tarmoq kartasi) OS'ga **registrlar** orqali ko'rinadi: holat, buyruq, ma'lumot (16-bob).
Oddiy protokol:

```text
while (holat == BAND) ;            /* 1. qurilma bo'shaguncha kutish (polling) */
ma'lumotni yozish;                 /* 2. */
buyruqni yozish;                   /* 3. qurilma ishni boshlaydi */
while (holat == BAND) ;            /* 4. tugashini kutish */
```

Muammolar va yechimlar:

- **Polling CPU'ni behuda yoqadi** → **uzilishlar**: buyruq berib, jarayon uxlaydi; qurilma tugagach
  uzilish yuboradi, ishlovchi kutayotgan jarayonni uyg'otadi. Lekin juda tez qurilmada uzilish xarajati
  foydadan katta — shuning uchun gibrid: avval biroz polling, keyin uzilish (NVMe, tarmoqda — NAPI).
- **Ma'lumotni CPU nusxalashi (PIO) sekin** → **DMA**: CPU qurilmaga "xotiraning shu fizik manzilidan
  N bayt ol" deydi; nusxani qurilmaning o'zi qiladi. MyOS'da ikkalasi ham bor: ATA PIO (`drivers/ata.c`)
  va AHCI DMA (`drivers/ahci.c`) — solishtiring.

**Drayver** — qurilmaga xos kod; yuqorida esa umumiy interfeys: blok qurilma (`read_block`/`write_block`),
belgi qurilma (`read`/`write` oqimi). Linux kodining ~70% i drayverlar. MyOS: `kernel/fs/block.c` —
disk qanday bo'lmasin (ATA, AHCI, keyinchalik NVMe), fayl tizimi bir xil interfeysni ko'radi.

## 27.2. Disklar

**HDD (qattiq disk):** aylanuvchi plastinalar, magnit kallak. Sektorni o'qish vaqti:
`izlash (kallakni yo'lakka surish, ~4–10 ms) + aylanish kutish (~2–4 ms) + uzatish`.
Tasodifiy o'qish — sekundiga ~100–200 ta; ketma-ket o'qish — 100–200 MB/s. **1000 marta farq!**
Shuning uchun fayl tizimlari tarixi — "tasodifiy murojaatni ketma-ketga aylantirish" tarixi.

**SSD (flesh):** harakatlanuvchi qism yo'q; tasodifiy o'qish ~10–100 mikrosekund. O'ziga xosliklari:
yozish sahifalar bilan (4–16 KB), o'chirish katta bloklar bilan; har bir blokni cheklangan marta o'chirish
mumkin — SSD kontrolleri ichida o'z "fayl tizimi" (FTL) ishlaydi (yozishlarni tarqatish, axlat yig'ish).
OS unga `TRIM` bilan "bu bloklar endi bo'sh" deb aytadi.

**Disk rejalashtirish** (HDD uchun): so'rovlarni kallak harakatini kamaytiradigan tartibda bajarish —
SSTF (eng yaqini), SCAN/elevator (lift kabi bir yo'nalishda). SSD'larda ahamiyati kam.

## 27.3. Fayl tizimi abstraksiyasi

Foydalanuvchi uchun: **fayllar** (baytlar ketma-ketligi) va **papkalar** (nom → fayl). Ichkarida har bir
faylning past darajadagi nomi — **inode raqami**. Papka — shunchaki (nom, inode raqami) juftliklari ro'yxati.

Tizim chaqiruvlari (14-bob): `open`, `read`, `write`, `lseek`, `fsync`, `rename`, `link`, `unlink`,
`mkdir`, `stat`. Bir nechta nozik joy:

- **`fsync(fd)`** — "ma'lumotni hozir diskka yoz". `write` odatda faqat xotiradagi keshga yozadi;
  tok o'chsa — yo'qoladi. Ma'lumotlar bazalari har tranzaksiyadan keyin `fsync` qiladi.
- **`rename` atomik** — "yangi faylni yozib, eski ustiga rename qilish" — faylni xavfsiz yangilashning
  standart usuli (muharrirlar shunday saqlaydi).
- **Qattiq havola** (`link`): bitta inode'ga ikkinchi nom. `unlink` — nomni o'chiradi; inode esa havolalar
  soni 0 bo'lganda **va** hech kim ochib turmaganda o'chadi (ochiq faylni o'chirish mumkin — ishlashda davom etadi!).
- **Ramziy havola** (`symlink`): ichida yo'l yozilgan maxsus fayl (MyOS disk tasvirida `test/havola`).

## 27.4. Oddiy fayl tizimini loyihalash (vsfs — OSTEP'dagi "juda oddiy fayl tizimi")

Disk bloklarga bo'lingan (masalan 4 KB). Joylashuv:

```text
[ superblok | inode bitmap | ma'lumot bitmap | inode jadvali ........ | ma'lumot bloklari .......... ]
```

- **Superblok** — butun fayl tizimi haqida: bloklar soni, inode'lar soni, jadvallar qayerda, sehrli son.
- **Bitmap'lar** — qaysi inode va qaysi ma'lumot bloki band (21-mashq).
- **Inode** — fayl metama'lumoti: tur, hajm, egasi, ruxsatlar, vaqtlar, havolalar soni va — eng muhimi —
  **ma'lumot bloklari qayerda**.

### Ma'lumot bloklarini ko'rsatish: ko'p darajali indeks

Inode'ga cheksiz ko'rsatkich sig'maydi. ext2 yechimi (37-mashq):
- 12 ta **bevosita** ko'rsatkich — kichik fayllar (ko'pchilik fayllar kichik!) uchun tez;
- 1 ta **bilvosita** — ko'rsatkichlar bloki (4 KB / 4 = 1024 blok = 4 MB qo'shimcha);
- 1 ta **ikki karra bilvosita** — 1024² blok (4 GB);
- 1 ta **uch karra bilvosita** — 1024³ blok (4 TB).

Nomutanosib daraxt — "ko'p fayllar kichik, bir nechtasi juda katta" kuzatuviga moslangan.
Boshqa yondashuv — **ekstentlar** (ext4, XFS): "shu blokdan boshlab 1000 ta ketma-ket blok" — katta
fayllar uchun ixchamroq.

### Faylni o'qish yo'li

`open("/papka/fayl.txt")`:
1. ildiz inode (raqami ma'lum — ext2'da 2) → uning ma'lumot bloklarida "papka" ni qidirish;
2. "papka" ning inode'i → uning bloklarida "fayl.txt" ni qidirish;
3. "fayl.txt" inode'i → fd'ga bog'lash.

`read`: inode'dan kerakli blok raqamini topish (bmap) → blokni o'qish. Chuqur yo'l = ko'p disk murojaati —
shuning uchun **keshlar** hal qiluvchi: sahifa keshi (fayl mazmuni), dentry keshi (nom → inode), inode keshi.
MyOS: `kernel/fs/ext2.c` (inode keshi), `kernel/fs/block.c` (buffer cache), `kernel/fs/vfs.c` (yo'lni aylanish).

## 27.5. FFS — diskni hisobga oluvchi joylashtirish

Birinchi Unix fayl tizimi sodda edi va disk o'tkazuvchanligining ~2% ini ishlatardi: fayl bloklari va
inode'lar diskning turli chekkalarida sochilgan (uzoq izlashlar). **FFS** (Fast File System, 1984) g'oyasi:
diskni **silindr guruhlariga** bo'lish; bog'liq narsalarni bir guruhga qo'yish — papka va undagi fayllar,
faylning inode'i va uning bloklari. Natija — 10 barobardan ko'proq tezlik.

ext2'dagi **blok guruhlari** — aynan shu g'oya (37-mashqdagi guruh deskriptorlari): har bir guruhning o'z
bitmap'lari va inode jadvali bor.

## 27.6. Tok o'chsa nima bo'ladi — izchillik (crash consistency)

Faylga bitta blok qo'shish uchun diskka **uch** narsa yozish kerak:
1. ma'lumot blokining o'zi;
2. inode (yangi blok ko'rsatkichi va hajm);
3. ma'lumot bitmap'i (blok band).

Disk ularni bittadan yozadi. Tok orasida o'chsa:

| Faqat shu yozildi | Oqibat |
|---|---|
| ma'lumot | ma'lumot yo'qoldi, lekin fayl tizimi izchil — muammo emas |
| inode | inode axlat blokka ko'rsatadi, bitmap "bo'sh" deydi — **nomuvofiqlik** + axlat o'qiladi |
| bitmap | blok "band", lekin hech kim ishlatmaydi — **sizib chiqish** |
| inode + bitmap | metama'lumot izchil, lekin blokda axlat |
| inode + ma'lumot | bitmap noto'g'ri — keyinchalik blok ikki faylga berilishi mumkin! |

### Yechim 1: fsck

Yuklanishda butun diskni tekshirib, nomuvofiqliklarni tuzatish (Linux `e2fsck` — MyOS testlari uni
ishlatadi!). To'g'ri ishlaydi, lekin **juda sekin**: disk qancha katta bo'lsa, shuncha uzoq (terabayt
disklarda soatlar).

### Yechim 2: jurnal (write-ahead logging)

Ma'lumotlar bazalaridan olingan g'oya: diskka yozishdan **oldin**, nima qilmoqchi ekaningizni alohida
**jurnal** hududiga yozing:

```text
1. Jurnalga: [TxB - boshlanish] [inode] [bitmap] [ma'lumot]          <- yozish
2. Jurnalga: [TxE - tugadi]   (bitta sektor - atomik yoziladi)        <- "commit"
3. Asl joylarga yozish (checkpoint)
4. Jurnaldagi tranzaksiyani bo'shatish
```

Tok o'chsa, yuklanishda jurnal ko'riladi: TxE bor tranzaksiyalar **qayta bajariladi** (redo), TxE
yo'qlari tashlanadi. Tiklash soniyalar oladi. ext3/ext4, NTFS, XFS — hammasi jurnalli.
Tezlik uchun ko'pincha faqat **metama'lumot** jurnallanadi (ext4 `data=ordered`: ma'lumot blokini
metama'lumotdan **oldin** asl joyiga yozish — shunda inode hech qachon axlatga ko'rsatmaydi).

### Yechim 3: copy-on-write fayl tizimlari

Hech narsani joyida o'zgartirmaslik: yangi versiyani bo'sh joyga yozib, oxirida ildiz ko'rsatkichini
atomik almashtirish (ZFS, btrfs, LFS g'oyasi). 24-bobdagi COW va 26-bobdagi RCU bilan bir xil g'oya.

## 27.7. VFS — ko'p fayl tizimlarini birlashtirish

Linux (va MyOS) bir vaqtda turli fayl tizimlarini ko'rsatadi: ext4, tmpfs, `/proc`, `/dev`. **VFS**
umumiy qatlam: `open`/`read` yo'lni aylanadi, mount nuqtalarini kesib o'tadi va aniq fayl tizimining
funksiyalar jadvalini chaqiradi (7-bob: `file_ops`). MyOS: `kernel/fs/vfs.c`, `docs/12-vfs.md`.

## 27.8. O'zingizni tekshiring

1. Polling va uzilishlar — qachon qaysi biri yaxshi?
2. DMA nima va nega kerak?
3. HDD'da tasodifiy va ketma-ket o'qish nega 1000 marta farq qiladi?
4. ext2'da 1 KB bloklar bilan bevosita ko'rsatkichlar qancha ma'lumotni qamraydi? Bilvosita-chi?
5. Faylga blok qo'shishda tok o'chdi: faqat inode yozilgan. Nima muammo? Jurnal buni qanday hal qiladi?

<details><summary>Javoblar</summary>

1. Tez qurilma/qisqa kutish — polling; sekin qurilma — uzilishlar (CPU boshqa ish qiladi).
2. Qurilma xotiraga CPU'siz, fizik manzil bo'yicha nusxalaydi — CPU vaqti tejaladi.
3. Tasodifiyda har murojaat uchun kallak surilishi va aylanish kutiladi (ms), ketma-ketda — faqat uzatish.
4. 12 KB; bilvosita + 256 blok = 256 KB.
5. Inode bitmap'da bo'sh deb turgan (axlatli) blokka ko'rsatadi. Jurnalda uchala yozuv TxE bilan birga turadi — tiklashda hammasi qayta bajariladi yoki hech biri.
</details>

## 27.9. Mashqlar

- **37** (ext2 o'qish) — agar hali qilmagan bo'lsangiz. Keyin unga faylni yozishni qo'shishni
  o'ylab ko'ring: qaysi tartibda yozish xavfsizroq?
- MyOS: `docs/13-disk-ext2.md` va `tools/test.sh` dagi `e2fsck` tekshiruvi — yadro yozgan diskni Linux
  qanday tekshiradi.

Keyingi bob: [28-bob. Algoritmlar va ma'lumotlar tuzilmalari](28-algoritmlar.md)
