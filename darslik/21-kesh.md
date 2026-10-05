# 21-bob. Xotira ierarxiyasi va kesh — nega bir xil kod 10 marta tezroq bo'lishi mumkin

> **Bu bobda nima o'rganasiz:** registr → kesh → RAM → disk pog'onalarini; kesh qatori (cache line) va lokallik tamoyilini; keshga mos kod yozishni; TLB'ni; ko'p yadroli tizimdagi
> "soxta bo'lishish" (false sharing) ni. Yadro — tizimning eng ko'p ishlatiladigan kodi; uning tezligi shu bilimga bog'liq.
> **Oldindan nima kerak:** 6-bob (massivlar), 9-bob (struct), 15-bob (oqimlar).   **Vaqt:** 5–6 soat.

> **To'liq ishlaydigan misol:** [misollar/21_kesh.c](misollar/21_kesh.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

> **Eslatma:** bu bobdagi **vaqt o'lchovlari** sizning kompyuteringizda boshqacha chiqadi (protsessor, kesh hajmi, boshqa ishlayotgan dasturlar). Muhim narsa — **nisbat** ("necha barobar tez/sekin"), son emas.

## Bu bob nima haqida?

Ikkita dastur bir xil ishni qiladi, bir xil natija beradi — lekin biri 5–10 marta sekin. Sabab odatda algoritmda emas, **xotiraga qanday murojaat qilinishida**. CPU juda tez, RAM esa unga nisbatan sekin.
Bu farqni yopish uchun CPU ichida kichik tez xotira — **kesh** bor. Uni qanday "xursand qilish" — shu bobning mavzusi.

**Hayotdan misol: kitob qayerda turibdi.** Siz insho yozyapsiz va kitob kerak:

| Qayerda | Hayotda | Kesh/xotira |
|---|---|---|
| **Registr** | kitob qo'lingizda ochiq turibdi (0 soniya) | CPU ichidagi registr |
| **L1 kesh** | stol ustida (1 soniya) | eng tez, kichik |
| **L2/L3 kesh** | xonadagi javonda (10 soniya) | |
| **RAM** | shahar kutubxonasida (~2 daqiqa) | asosiy xotira |
| **SSD** | boshqa shahardagi arxivda (~bir kun) | |
| **Qattiq disk** | chet eldan pochta orqali (~bir necha oy) | |

Protsessor uchun RAM'ga borish — xuddi kutubxonaga borib kelishdek. Kesh shu yo'lni qisqartiradi.

## 21.1. Tezlik pog'onalari

CPU juda tez, xotira esa unga nisbatan sekin. Taxminiy raqamlar (zamonaviy kompyuter):

| Qayerda | Hajm | Kirish vaqti | CPU taktlarida |
|---|---|---|---|
| Registr | ~1 KB | 0.3 ns | 1 |
| L1 kesh | 32–48 KB (har yadroda) | ~1 ns | 4 |
| L2 kesh | 1–2 MB (har yadroda) | ~4 ns | 12–15 |
| L3 kesh | 8–64 MB (umumiy) | ~15 ns | 40–50 |
| RAM | 8–64 GB | ~80 ns | 200–300 |
| NVMe SSD | 1 TB | ~20 000 ns | 60 000 |
| HDD | 4 TB | ~5 000 000 ns | 15 000 000 |

Agar L1 ga murojaat 1 soniya bo'lsa, RAM — ~1.5 daqiqa, SSD — ~6 soat, HDD — ~2 oy.

**Kesh** — kichik, tez xotira, sekinroq xotiradagi ma'lumotning **nusxasini** saqlaydi. Har bir pog'ona keyingisi uchun kesh vazifasini bajaradi: RAM disk uchun (sahifa keshi — MyOS: buffer cache,
`kernel/fs/block.c`), L3 RAM uchun va hokazo.

O'z kompyuteringizning keshini ko'rish:

```console
$ getconf LEVEL1_DCACHE_LINESIZE
64
```

Bu — **kesh qatori** o'lchami (bayt): ko'p kompyuterlarda `64`. Hajmlarni `lscpu | grep -i cache` yoki `getconf -a | grep CACHE` ko'rsatadi (kompyuterga qarab farq qiladi).

## 21.2. Lokallik tamoyili — kesh nega ishlaydi

**Hayotdan misol: kitobning keyingi sahifasi.** Kitobning 50-sahifasini o'qigan bo'lsangiz, katta ehtimol bilan keyin 51-sahifani o'qiysiz (**fazoviy lokallik**). Bugun ishlatgan lug'atingizni ertaga yana ishlatasiz
(**vaqt lokalligi**). Kesh aynan shunga garov o'ynaydi: yaqinda kerak bo'lgan narsa va uning qo'shnilari yana kerak bo'ladi.

Dasturlar xotiraga tasodifiy murojaat qilmaydi:

- **Vaqt bo'yicha lokallik:** yaqinda ishlatilgan ma'lumot yana tez orada ishlatiladi (sikl o'zgaruvchisi, stek tepasi).
- **Joy bo'yicha lokallik:** ishlatilgan manzil yonidagilar ham tez orada ishlatiladi (massivni ketma-ket o'qish, ketma-ket buyruqlar).

Kesh shu ikkisidan foydalanadi: ma'lumot keshda qoladi (vaqt) va **butun kesh qatori** bilan olib kelinadi (joy).

## 21.3. Kesh qatori (cache line)

**Hayotdan misol: kitobni emas, butun qutini olib kelish.** Kutubxonaga borganda bitta sahifani emas, butun kitobni olasiz. Protsessor ham 1 baytni emas, 64 baytlik **qatorni** olib keladi.
Keyingi 63 bayt bepul keladi — agar ular kerak bo'lsa.

Kesh xotira bilan baytma-bayt emas, **64 baytli bloklar** bilan almashadi. `a[0]` ni o'qisangiz, `a[0..15]` (16 ta int) birdan keshga keladi — keyingi 15 tasi deyarli bepul.

**Hayotdan misol: massivni qator bo'yicha aylanish.** Kitobni sahifama-sahifa o'qish tez. Har kitobdan bittadan sahifa o'qib, keyingi kitobga o'tish (ustun bo'yicha aylanish) — har safar kutubxonaga borish bilan barobar.

```c
/* qator_ustun.c - matritsani qator va ustun bo'yicha yig'ish */
#include <stdio.h>
#include <time.h>

#define N 4096

static int m[N][N];                              /* 64 MB: keshga sig'maydi */

static double hozir(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (double)t.tv_sec + (double)t.tv_nsec / 1e9;
}

int main(void)
{
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            m[i][j] = 1;

    long s1 = 0, s2 = 0;

    double t0 = hozir();
    for (int i = 0; i < N; i++)                  /* QATOR bo'yicha: m[i][0], m[i][1], ... */
        for (int j = 0; j < N; j++)
            s1 += m[i][j];
    double t1 = hozir();

    for (int j = 0; j < N; j++)                  /* USTUN bo'yicha: m[0][j], m[1][j], ... */
        for (int i = 0; i < N; i++)
            s2 += m[i][j];
    double t2 = hozir();

    printf("qator bo'yicha:  yig'indi %ld, %.3f s\n", s1, t1 - t0);
    printf("ustun bo'yicha:  yig'indi %ld, %.3f s\n", s2, t2 - t1);
    printf("ustun taxminan %.1f barobar sekin\n", (t2 - t1) / (t1 - t0));
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 qator_ustun.c -o qator_ustun
$ ./qator_ustun
qator bo'yicha:  yig'indi 16777216, 0.009 s
ustun bo'yicha:  yig'indi 16777216, 0.159 s
ustun taxminan 17.3 barobar sekin
```

**Bu dastur nima qiladi (umumiy):** 4096×4096 `int` matritsani ikki usulda yig'adi — **qator** bo'yicha va **ustun** bo'yicha — va vaqtni solishtiradi. Yig'indi **bir xil**, vaqt esa turlicha.

**Nega farq bor (qadam-baqadam):**

```text
C da m[i][j] xotirada QATORMA-QATOR yotadi (6.3):
  m[0][0] m[0][1] m[0][2] ... m[0][4095] | m[1][0] m[1][1] ...

Qator bo'yicha:  m[0][0], m[0][1], m[0][2] ...  -> qo'shni xotira -> bitta kesh qatori (64 bayt = 16 int) 16 elementga yetadi
Ustun bo'yicha:  m[0][0], m[1][0], m[2][0] ...  -> har biri 4096*4 = 16 KB narida -> HAR BIRI yangi kesh qatori
```

Qator bo'yicha 16 ta elementga **bitta** xotira murojaati, ustun bo'yicha esa **har bir** element uchun alohida murojaat. Shuning uchun ustun bo'yicha aylanish sezilarli sekin
(N katta bo'lsa 5–10 marta).

> **Eslab qoling:** ichki sikl **oxirgi indeks** bo'yicha bo'lsin (`m[i][j]` da `j` ichkarida). Xotiraga **ketma-ket** murojaat qiling.

**Ma'lumot tuzilmasini tanlashga ta'siri:**

- Massiv bog'langan ro'yxatdan ko'pincha **tezroq**, hatto nazariy murakkablik bir xil bo'lsa ham: massiv elementlari ketma-ket, ro'yxat tugunlari xotirada sochilgan — har bir tugun kesh xatosi.
- Tez-tez birga ishlatiladigan maydonlarni structda yonma-yon qo'ying; kam ishlatiladiganlarini oxiriga. Linux yadrosida muhim tuzilmalar (`struct page`, `task_struct`) aynan shunday tartiblangan.

## 21.4. Kesh qanday tashkil etilgan

Kesh qaysi qatorni qayerda saqlashini manzildan **hisoblaydi**. Manzil uch qismga bo'linadi:

```text
[      teg      |  to'plam indeksi  | qator ichidagi siljish (6 bit, 64 bayt) ]
```

- **Siljish** — qator ichida qaysi bayt (64 bayt → 6 bit).
- **Indeks** — keshning qaysi "uyasiga" tushishi.
- **Teg (tag)** — shu uyaga tushishi mumkin bo'lgan boshqa qatorlardan farqlovchi belgi.

Kichik misolda (8 uya × 16 bayt) qismlarga bo'lishni ko'ramiz:

```c
/* manzil_bolish.c - manzilni teg | indeks | siljish ga bo'lish */
#include <stdint.h>
#include <stdio.h>

#define QATOR_BAYT 16                           /* bitta qator: 16 bayt  (4 bit siljish) */
#define UYALAR 8                                /* 8 ta uya              (3 bit indeks)  */

int main(void)
{
    uint32_t manzillar[] = { 0, 15, 16, 128, 130, 256, 1000 };
    printf("manzil | qator | siljish | indeks | teg\n");
    for (int i = 0; i < 7; i++) {
        uint32_t m = manzillar[i];
        uint32_t qator = m / QATOR_BAYT;        /* nechanchi 16 baytli blok */
        uint32_t siljish = m % QATOR_BAYT;      /* blok ichida */
        uint32_t indeks = qator % UYALAR;       /* keshning qaysi uyasi */
        uint32_t teg = qator / UYALAR;          /* shu uyaga tushganlarni farqlovchi */
        printf("%6u | %5u | %7u | %6u | %3u\n", m, qator, siljish, indeks, teg);
    }
    return 0;
}
```

```console
$ gcc -Wall -Wextra manzil_bolish.c -o manzil_bolish
$ ./manzil_bolish
manzil | qator | siljish | indeks | teg
     0 |     0 |       0 |      0 |   0
    15 |     0 |      15 |      0 |   0
    16 |     1 |       0 |      1 |   0
   128 |     8 |       0 |      0 |   1
   130 |     8 |       2 |      0 |   1
   256 |    16 |       0 |      0 |   2
  1000 |    62 |       8 |      6 |   7
```

**Nima ko'rdik:**

- `0` va `15` — **bir qator** (qator 0), siljish 0 va 15 → ikkinchisi birinchisi keltirgan qatordan **bepul** keladi (fazoviy lokallik).
- `16` — keyingi qator (qator 1, indeks 1).
- `128` va `130` — ham **bir qator** (qator 8), siljishlari 0 va 2.
- `0` (teg 0) va `128` (teg 1) — ikkalasida **indeks 0**, lekin **turli teg** → ular bir uyaga tushadi va bir-birini **siqib chiqaradi**. Bu — *to'qnashuv* (conflict) xatosi.

Keshlar turlari:

- **To'g'ridan-to'g'ri xaritalangan** (direct-mapped): har bir manzil faqat bitta joyga tushadi — oddiy, lekin ikki "raqib" manzil bir-birini doim siqib chiqaradi.
- **To'plam-assotsiativ** (set-associative, masalan 8-way): har bir to'plamda 8 ta joy — zamonaviy CPU'lar shunday.
- Joy tugasa — eng kam ishlatilgan (LRU'ga yaqin) qator chiqariladi (24-bobdagi sahifa almashtirish bilan bir xil g'oya).

**Kesh xatolarining 3 turi ("3C"):** birinchi murojaat (compulsory), sig'maslik (capacity), to'qnashuv (conflict).

## 21.5. Yozish va kogerentlik

CPU yozganda ma'lumot avval keshga yoziladi (write-back), RAM'ga keyinroq. Ko'p yadroli tizimda har bir yadroning o'z L1/L2 si bor — bitta manzilning nusxasi bir nechta keshda bo'lishi mumkin.
Apparat **kogerentlik protokoli** (MESI) bilan ularni moslashtiradi: bir yadro yozsa, boshqalardagi nusxa "yaroqsiz" qilinadi.

Bu dasturchiga ikki narsani anglatadi:

1. **Kesh qatori "ping-pong"i:** ikki yadro bitta qatorga navbatma-navbat yozsa, qator ular orasida doimiy ko'chib yuradi — juda sekin. Spinlock'da "test-and-test-and-set" (15-bob, 34-mashq) shuning uchun kerak:
   kutayotganlar faqat **o'qiydi**, qator hamma keshda "umumiy" holatda turadi.
2. **Soxta bo'lishish (false sharing):** ikki yadro **turli** o'zgaruvchilarga yozadi, lekin ular bitta 64 baytli qatorda — natija xuddi bitta o'zgaruvchidek sekin.

**Hayotdan misol: bitta daftarga ikki kishi yozishi.** Ikki xodim bitta daftarning **turli** qatorlariga yozadi, lekin daftar bitta — har safar uni bir-biriga uzatishga to'g'ri keladi. Ular bir-biriga xalaqit bermaydi
deb o'ylaydi, aslida vaqtning ko'pi uzatishga ketadi. Yechim: har biriga alohida daftar.

```c
/* soxta_bolishish.c - false sharing: 4 oqim, turli o'zgaruvchilar */
#include <pthread.h>
#include <stdio.h>
#include <time.h>

#define OQIMLAR 4
#define TAKROR 10000000L

struct yaqin { volatile long qiymat; };                                      /* 8 bayt: 4 tasi BITTA qatorda */
struct uzoq  { volatile long qiymat; char bosh[56]; };                       /* 64 bayt: har biri alohida qatorda */

static struct yaqin yaqin_h[OQIMLAR];
static struct uzoq uzoq_h[OQIMLAR] __attribute__((aligned(64)));

static double hozir(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (double)t.tv_sec + (double)t.tv_nsec / 1e9;
}

static void *yaqin_ish(void *arg)
{
    long id = (long)arg;
    for (long i = 0; i < TAKROR; i++)
        __atomic_fetch_add(&yaqin_h[id].qiymat, 1, __ATOMIC_RELAXED);   /* har oqim O'Z o'zgaruvchisiga */
    return NULL;
}

static void *uzoq_ish(void *arg)
{
    long id = (long)arg;
    for (long i = 0; i < TAKROR; i++)
        __atomic_fetch_add(&uzoq_h[id].qiymat, 1, __ATOMIC_RELAXED);
    return NULL;
}

static double o_lcha(void *(*ish)(void *))
{
    pthread_t t[OQIMLAR];
    double b = hozir();
    for (long i = 0; i < OQIMLAR; i++)
        pthread_create(&t[i], NULL, ish, (void *)i);
    for (int i = 0; i < OQIMLAR; i++)
        pthread_join(t[i], NULL);
    return hozir() - b;
}

int main(void)
{
    double t_yaqin = o_lcha(yaqin_ish);
    double t_uzoq = o_lcha(uzoq_ish);
    printf("bitta qatorda (soxta bo'lishish): %.3f s\n", t_yaqin);
    printf("alohida qatorlarda:               %.3f s\n", t_uzoq);
    printf("farq: taxminan %.1f barobar\n", t_yaqin / t_uzoq);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 -pthread soxta_bolishish.c -o soxta_bolishish
$ ./soxta_bolishish
bitta qatorda (soxta bo'lishish): 0.645 s
alohida qatorlarda:               0.066 s
farq: taxminan 9.7 barobar
```

**Bu dastur nima qiladi (umumiy):** 4 oqim bor, har biri **o'z** sanagichini 10 million marta (atomik) oshiradi (poyga yo'q — turli o'zgaruvchilar). Birinchi holatda 4 ta sanagich yonma-yon (bitta 64 baytli qatorda),
ikkinchisida har biri alohida qatorda. Natija to'g'ri bo'lsa ham, vaqt turlicha.

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `struct yaqin { long qiymat; }` | 8 bayt; 4 ta ketma-ket massivda → hammasi **bir kesh qatorida** (32 bayt) |
| `struct uzoq { long qiymat; char bosh[56]; }` | 8 + 56 = **64 bayt**: har bir sanagich **o'z qatorida** |
| `aligned(64)` | massiv boshi 64 ga karrali manzilda boshlansin |
| `__atomic_fetch_add` | atomik oshirish: bu amal qatorga **yakka egalik** talab qiladi — shuning uchun false sharing aniq ko'rinadi (oddiy `x++` da yozuvlar bufer orqali yashirinishi mumkin) |
| nega sekin | yadrolar bitta qatorni navbat bilan "tortib olishadi" (MESI) — har yozish qatorni boshqa yadrodan olib kelishga majbur qiladi |

Yechim: har biri alohida qatorda (`__attribute__((aligned(64)))` yoki to'ldiruvchi bayt). MyOS'da: per-CPU ma'lumotlar (`kernel/arch/percpu.c`) — har bir CPU'ning o'z tuzilmasi, qulfsiz va bo'lishishsiz.
Linux'da `DEFINE_PER_CPU` va `____cacheline_aligned`.

## 21.6. TLB — manzil tarjimasi keshi

**Hayotdan misol: telefondagi "tez-tez qo'ng'iroq qilinganlar".** Virtual manzilni fizik manzilga tarjima qilish sekin (24-bob). Protsessor oxirgi tarjimalarni kichik ro'yxatda saqlaydi — xuddi telefon
tez-tez kerak bo'ladigan raqamlarni eng tepada ko'rsatgandek.

Har bir xotira murojaatida virtual manzilni fizikka aylantirish kerak (4 darajali sahifa jadvali — 31-mashq). Har safar 4 marta xotiraga borish — halokatli sekin. **TLB** (Translation Lookaside Buffer) —
oxirgi tarjimalarning kichik keshi (~1500 yozuv).

Oqibatlari:

- **CR3 almashtirilganda** (boshqa jarayonga o'tish) TLB tozalanadi → kontekst almashishning yashirin narxi. (PCID texnologiyasi buni qisman hal qiladi.)
- Sahifa jadvalini o'zgartirgan yadro **TLB'ni ham tozalashi** kerak: bitta sahifa uchun `invlpg`, boshqa CPU'larda — "TLB shootdown" (uzilish yuborib, ularni ham tozalatish). MyOS: `kernel/mm/vmm.c`, `docs/10-smp.md`.
- **Katta sahifalar** (2 MB, 1 GB) — bitta TLB yozuvi ko'p xotirani qamraydi. MyOS'ning direct map'i 2 MB sahifalar bilan quriladi (`[vmm] Yadro xaritasi: ... 2 MB/4 KB sahifalar`).

## 21.7. Keshga mos dasturlash qoidalari

1. Ma'lumotni ketma-ket o'qing (massiv > ro'yxat, qatorma-qator).
2. Ishchi to'plamni kichik qiling: katta ma'lumotni bloklarga bo'lib ishlang (blocking/tiling).
3. Birga ishlatiladigan maydonlarni yaqin joylashtiring; "issiq" va "sovuq" maydonlarni ajrating.
4. Ko'p yadroli yozishlarda: har bir yadro o'z ma'lumotiga (per-CPU), umumiy o'zgaruvchilarni kam yozing, turli yadrolar yozadigan o'zgaruvchilarni turli kesh qatorlariga qo'ying.
5. **Taxmin qilmang — o'lchang** (29-bob: `perf stat -e cache-misses`).

## Hayotdan misol va to'liq dastur

**Ombor hisoboti.** Omborda har bir mahsulot qutisida nomi, rasmi, tavsifi va narxi bor. Faqat **narxlar yig'indisi** kerak bo'lsa, har bir katta qutini ochish kerak. Narxlar alohida ro'yxatda bo'lsa —
bitta varaqni o'qish kifoya. Kesh uchun ham shunday: faqat kerakli maydonlar zich joylashsa, har bir 64 baytlik qatordan to'liq foydalaniladi.

Bir million mahsulot narxlarining yig'indisi — ikki xil joylashuvda.

```c
/* ombor.c - struct'lar massivi va alohida massiv: kesh farqi */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1000000

struct mahsulot {                               /* 64 bayt: bitta kesh qatori */
    char nom[40];
    long narx;
    long soni;
    long ombor_raqami;
};

static double hozir(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (double)t.tv_sec + (double)t.tv_nsec / 1e9;
}

int main(void)
{
    struct mahsulot *qutilar = malloc(N * sizeof(*qutilar));   /* 64 MB */
    long *narxlar = malloc(N * sizeof(*narxlar));              /* 8 MB */
    if (!qutilar || !narxlar)
        return 1;
    for (long i = 0; i < N; i++) {
        qutilar[i].narx = narxlar[i] = i % 1000;
        qutilar[i].soni = 1;
    }

    long s1 = 0, s2 = 0;
    double t0 = hozir();
    for (int takror = 0; takror < 10; takror++)
        for (long i = 0; i < N; i++)
            s1 += qutilar[i].narx;              /* har bir narx - alohida kesh qatorida */
    double t1 = hozir();
    for (int takror = 0; takror < 10; takror++)
        for (long i = 0; i < N; i++)
            s2 += narxlar[i];                   /* bitta kesh qatorida 8 ta narx */
    double t2 = hozir();

    printf("sizeof(struct mahsulot) = %zu bayt\n", sizeof(struct mahsulot));
    printf("Har bir qutini ochib:   yig'indi %ld, %.3f s\n", s1, t1 - t0);
    printf("Alohida narxlar ro'yxati: yig'indi %ld, %.3f s\n", s2, t2 - t1);
    printf("Farq: taxminan %.0f barobar\n", (t1 - t0) / (t2 - t1));
    free(qutilar);
    free(narxlar);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 ombor.c -o ombor
$ ./ombor
sizeof(struct mahsulot) = 64 bayt
Har bir qutini ochib:   yig'indi 4995000000, 0.030 s
Alohida narxlar ro'yxati: yig'indi 4995000000, 0.004 s
Farq: taxminan 7 barobar
```

**Bu dastur nima qiladi (umumiy):** bir million mahsulot narxining yig'indisini ikki joylashuvda hisoblaydi: (1) **struct massivi** — har bir mahsulot 64 baytlik "quti", faqat `narx` kerak; (2) **alohida `narxlar` massivi** —
faqat narxlar zich turadi. Ikkala sikl bir xil yig'indi beradi, farq faqat xotirada joylashuvda.

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `struct mahsulot` (64 bayt) | bitta mahsulot = bitta to'liq kesh qatori; `narx` uning faqat 8 baytiga |
| `qutilar[i].narx` | har bir narx uchun **butun 64 baytli qator** keshga keladi (56 bayt kerak emas) |
| `narxlar[i]` | 8 ta narx **bitta** qatorda (8 × 8 = 64) — qator to'liq foydalanilgan |
| `takror < 10` | o'lchov aniqroq bo'lishi uchun 10 marta |

Birinchi holatda xotiradan **8 barobar ko'p bayt** olib kelinadi (64 vs 8 bayt/narx) — vaqt farqi ham shu tartibda.

**Sinab ko'ring:** `char nom[40]` ni `char nom[8]` qiling (struct 32 bayt bo'ladi) — farq qanday o'zgaradi? `perf stat -e cache-misses ./ombor` bilan kesh xatolarini sanang (29-bob).

<!-- katta:boshi -->
## Katta loyiha: kesh laboratoriyasi — xotiraga qanday kirish tezlikni belgilaydi

**Umumiy fikr.** Protsessor operativ xotiradan (RAM) **juda sekin** o'qiydi: bitta kirish ~100 ta buyruq bajarish vaqtiga teng. Shuning uchun protsessor ichida **kesh** — kichik, lekin juda tez xotira bor. Kesh xotirani **bayt-bayt emas, 64 baytlik qatorlar** (cache line) bilan oladi. 21-bob shu haqida: agar siz **qo'shni** xotiraga ketma-ket murojaat qilsangiz, bitta qator yuklanishi **16 ta int** uchun yetadi; sakrab yursangiz — har kirishda yangi qator.

**Hayotiy o'xshatish:** kutubxonadan kitob olish. **Ketma-ket** o'qisangiz — bir marta borib, 16 ta kitobni savatga solib kelasiz. **Sakrab** o'qisangiz — har kitob uchun alohida yuborasiz.

### Bu bosqichda nima qilamiz

Bitta massivni **ikki xil tartibda** aylanamiz, matritsa ko'paytirishning **ikki xil tartibini** solishtiramiz. Natijani **vaqt** bilan emas, **kesh xatolari soni** bilan o'lchaymiz: bu uchun **cachegrind** (valgrind ichidagi kesh simulyatori) ishlatamiz. Valgrind protsessorni **emulyatsiya** qiladi, shuning uchun natijalar har safar **aniq bir xil** (vaqt o'lchash esa har gal o'zgaradi).

```c
/* xotira_yurish.c - bitta massivni turlicha aylanish: kesh xatolari soni valgrind (cachegrind) bilan o'lchanadi */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 512                                   /* N x N int = 1 MB: 32 KB li D1 keshga sig'maydi */
#define M 128                                   /* matritsa ko'paytirish uchun kichikroq (3 ta M x M massiv) */

static int a[N][N];
static int p[M][M], q[M][M], r[M][M];

static long satr_boyicha(void)                  /* a[i][0], a[i][1], ...: xotirada KETMA-KET */
{
    long s = 0;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            s += a[i][j];
    return s;
}

static long ustun_boyicha(void)                 /* a[0][j], a[1][j], ...: har qadam N*4 = 2048 bayt SAKRAYDI */
{
    long s = 0;
    for (int j = 0; j < N; j++)
        for (int i = 0; i < N; i++)
            s += a[i][j];
    return s;
}

static long kopaytir_ijk(void)                  /* klassik tartib: q[k][j] ustun bo'ylab o'qiladi */
{
    memset(r, 0, sizeof(r));
    for (int i = 0; i < M; i++)
        for (int j = 0; j < M; j++)
            for (int k = 0; k < M; k++)
                r[i][j] += p[i][k] * q[k][j];
    return r[M - 1][M - 1];
}

static long kopaytir_ikj(void)                  /* j ichkarida: q[k][*] va r[i][*] qator bo'ylab o'qiladi */
{
    memset(r, 0, sizeof(r));
    for (int i = 0; i < M; i++)
        for (int k = 0; k < M; k++) {
            int pik = p[i][k];
            for (int j = 0; j < M; j++)
                r[i][j] += pik * q[k][j];
        }
    return r[M - 1][M - 1];
}

int main(int argc, char **argv)
{
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            a[i][j] = (i + j) % 7;
    for (int i = 0; i < M; i++)
        for (int j = 0; j < M; j++) {
            p[i][j] = (i * 3 + j) % 5;
            q[i][j] = (i + j * 2) % 5;
        }

    const char *rejim = argc > 1 ? argv[1] : "satr";
    long natija;
    if (strcmp(rejim, "satr") == 0)
        natija = satr_boyicha();
    else if (strcmp(rejim, "ustun") == 0)
        natija = ustun_boyicha();
    else if (strcmp(rejim, "ijk") == 0)
        natija = kopaytir_ijk();
    else if (strcmp(rejim, "ikj") == 0)
        natija = kopaytir_ikj();
    else
        return 2;
    printf("%s: natija %ld\n", rejim, natija);
    return 0;
}
```

**Kodda nimalar bor:**

| Rejim | Kod | Xotira yurishi |
|---|---|---|
| `satr` | `a[i][j]`, `j` ichkarida | `a[i][0]`, `a[i][1]`, ... — **ketma-ket** (C massivlari satr bo'yicha saqlanadi) |
| `ustun` | `a[i][j]`, `i` ichkarida | har qadam **2048 bayt** (512 ta int) sakraydi — har kirish **yangi** kesh qatoriga |
| `ijk` | klassik: `r[i][j] += p[i][k] * q[k][j]` | `q[k][j]` — `k` o'zgarganda **ustun bo'ylab** yuradi |
| `ikj` | `k` o'rtada, `j` ichkarida | `q[k][*]` va `r[i][*]` **qator bo'ylab** yuradi |

Ikkala matritsa ko'paytirish usuli **bir xil natija** beradi va **bir xil sondagi** amal bajaradi — farqi faqat **tartibda**.

O'lchov skripti (`olchov.sh`) har rejimni cachegrind ostida ishga tushirib, `D1` (birinchi daraja ma'lumot keshi, 32 KB, 8 yo'lli, 64 baytli qator) bo'yicha **xatolar** (miss) sonini (mingta) va foizini jadvalga chiqaradi. Natija **ataylab yaxlitlangan**: muhit o'zgaruvchilari hajmi hisobni bir necha o'nga o'zgartirishi mumkin, farq esa **bir necha barobar**:

```bash
valgrind --tool=cachegrind --cache-sim=yes --D1=32768,8,64 --cachegrind-out-file=/dev/null ./xotira_yurish "$rejim"
```

```console
$ cd katta_loyiha/tizim/21_kesh_lab
$ gcc -Wall -Wextra -O1 -g xotira_yurish.c -o xotira_yurish
$ chmod +x olchov.sh
$ ./olchov.sh
rejim     D1 xatolar (ming)     xato %
satr                     37         6%
ustun                   282        46%
ijk                    2153        46%
ikj                     154         3%
```

**Nima ko'rdik:**

| Rejim | D1 xato % | Tushuntirish |
|---|---|---|
| `satr` | **≈6%** | 64 bayt = 16 ta `int`; ketma-ket o'qishda **16 kirishdan 1 tasi** yangi qator yuklaydi: 1/16 = 6.25% — deyarli shu |
| `ustun` | **≈46%** | har kirish **boshqa qatorga**: deyarli har safar xato. Kirishlar soni **bir xil** (taxminan 0,6 mln), lekin xatolar **~8 barobar** ko'p (37 ming ↔ 282 ming) |
| `ijk` | **≈46%** | `q[k][j]` ustun bo'ylab o'qiladi — yuqoridagi `ustun` holati |
| `ikj` | **≈3%** | hamma massiv **qator bo'ylab** — eng yaxshi. `ijk` ga nisbatan xatolar **14 barobar** kam |

Haqiqiy vaqtda bu farq ham seziladi: katta massivlarda `ikj` `ijk` dan odatda **bir necha barobar tez**. Dasturingiz ishlashi **algoritm murakkabligi** (O(n³)) bilan emas, **xotiraga kirish tartibi** bilan ham belgilanadi.

> **Eslab qoling:** protsessor xotirani **64 baytlik qatorlar** bilan oladi. **Ketma-ket** murojaat — arzon, **sakrash** — qimmat. C da ikki o'lchamli massivda **ichki sikl — oxirgi indeks** bo'lsin (`a[i][j]` da `j`). Optimallashtirishdan oldin **o'lchang** (cachegrind, `perf`), taxmin qilmang.

**O'zingiz qo'shing (yechimsiz):**

1. `N` ni 512 dan **64** ga tushiring (massiv = 16 KB, keshga **sig'adi**). `satr` va `ustun` farqi qolyaptimi? Nega?
2. `--D1=32768,8,64` ni `--D1=8192,8,64` (8 KB kesh) ga o'zgartiring: jadval qanday o'zgardi?
3. **Blokli** (tiling) matritsa ko'paytirish yozing: `M×M` ni `8×8` bloklarga bo'lib hisoblang va `ikj` bilan xatolarni solishtiring.
<!-- katta:oxiri -->

## Bob xulosasi (yodlash uchun)

1. Tezlik pog'onalari: registr → L1 → L2/L3 → RAM → SSD → HDD; har pog'ona ~10–100× sekinroq. **Kesh** — sekinroq xotira nusxasini saqlovchi kichik tez xotira.
2. **Lokallik:** yaqinda ishlatilgan (vaqt) va yaqin joylashgan (joy) ma'lumot yana kerak bo'ladi — kesh shunga tayanadi.
3. Kesh **64 baytli qator** bilan ishlaydi: ketma-ket murojaat tez; ustun bo'yicha / sochilgan murojaat sekin. Ichki sikl — oxirgi indeks.
4. Manzil = `teg | indeks | siljish`; bir uyaga tushgan turli qatorlar bir-birini siqadi (to'qnashuv).
5. **False sharing:** turli yadrolar bitta qatordagi turli o'zgaruvchilarga yozadi → sekin; yechim: alohida qatorlar (`aligned(64)`, per-CPU). **TLB** — manzil tarjimasi keshi. Doim **o'lchang**.

## Savol-javob

**Savol:** Nega massivni qator bo'ylab aylanish ustun bo'ylab aylanishdan tezroq?
**Javob:** Qator elementlari xotirada yonma-yon, bitta kesh qatori (64 bayt) bir nechta elementni birga olib keladi. Ustun bo'ylab har qadam yangi kesh qatoriga tushadi — ko'p kesh xatolari (21.2, 21.3).

**Savol:** False sharing nima?
**Javob:** Ikki oqim **turli** o'zgaruvchilarni o'zgartiradi, lekin ular bitta kesh qatorida turibdi: yadrolar qatorni tinmay bir-biridan tortib oladi va dastur sekinlashadi. Davo: o'zgaruvchilarni 64 baytga tekislab (padding) alohida qatorga qo'yish (21.5).

**Savol:** TLB va kesh bir narsami?
**Javob:** Yo'q. Kesh **ma'lumotni** tezlashtiradi; TLB esa **manzil tarjimasini** (virtual → fizik) keshlaydi, aks holda har murojaatda sahifa jadvalini o'qish kerak bo'lardi (21.6, 24-bob).

## O'zingizni tekshiring

1. Nega `int` massivini ketma-ket o'qish har bir elementni alohida olishdan tezroq?
2. False sharing nima va qanday tuzatiladi?
3. TLB nima uchun kerak va CR3 almashishi unga qanday ta'sir qiladi?
4. Nega spinlock'da kutayotgan oqim `xchg` ni takrorlamasligi kerak?

<details><summary>Javoblar</summary>

1. Kesh 64 baytli qatorlar bilan ishlaydi — bitta xatoda 16 ta int keladi; qolgan 15 tasi keshdan.
2. Turli yadrolar yozadigan turli o'zgaruvchilar bitta kesh qatorida — qator yadrolar orasida ko'chib yuradi. Har birini alohida qatorga tekislash (`aligned(64)`) yoki per-CPU ma'lumot.
3. Virtual → fizik tarjimalarni keshlaydi (aks holda har murojaatda 4 ta qo'shimcha xotira o'qishi); CR3 almashganda (PCID'siz) TLB tozalanadi.
4. `xchg` yozish amali — kesh qatorini boshqa yadrolardan tortib oladi; faqat o'qish qatorni umumiy holatda qoldiradi.
</details>

## Mashq

Qo'shimcha tajriba (natijani o'zingiz o'lchang): 4096×4096 `int` matritsani qatorma-qator va ustunma-ustun yig'ing, `clock()` bilan vaqtni solishtiring (`-O2`). Keyin 8 ta oqim bilan 21.5-dagi
ikkala `struct hisoblagich` variantini sinang.

<!-- loyiha:boshi -->
## Loyiha: to'g'ridan-to'g'ri xaritalangan kesh simulyatori

**Maqsad:** keshni **ichidan** tushunish: manzil qanday `tag | indeks | siljish` ga bo'linadi, qachon "urish" (hit)
va qachon "xato" (miss) bo'ladi. Massivni qator bo'yicha aylanish nega ustun bo'yicha aylanishdan tez ekanini **son bilan**
ko'rasiz.
**Bobdan ishlatiladi:** kesh qatori, indeks/tag, lokallik, konflikt xatolari (21.2–21.4).

**Talab:** kesh 8 qatordan iborat, har qator 16 bayt (jami 128 bayt). Xotira manziliga murojaat kelganda:
1. `qator_raqami = manzil / 16`,
2. `indeks = qator_raqami % 8` (keshning qaysi joyiga tushadi),
3. `tag = qator_raqami / 8` (o'sha joyga tushishi mumkin bo'lgan boshqa qatorlardan farqlovchi).

Shu joyda **haqiqiy** qator bor va `tag` mos bo'lsa — **urish**; aks holda **xato**: qator keshga yuklanadi (eskisi siqib chiqariladi).
**Ma'lumotlar:** `struct qator { int haqiqiy; uint32_t tag; }` massivi, `urish`/`xato` sanagichlari.

```c
/* kesh.c - to'g'ridan-to'g'ri xaritalangan kesh simulyatori */
#include <stdint.h>
#include <stdio.h>

#define QATOR_BAYT 16
#define QATORLAR 8

struct qator {
    int haqiqiy;
    uint32_t tag;
};

static struct qator kesh[QATORLAR];
static long urish, xato;

static void tozala(void)
{
    for (int i = 0; i < QATORLAR; i++)
        kesh[i].haqiqiy = 0;
    urish = xato = 0;
}

static void murojaat(uint32_t manzil)
{
    uint32_t qator_raqami = manzil / QATOR_BAYT;
    uint32_t indeks = qator_raqami % QATORLAR;
    uint32_t tag = qator_raqami / QATORLAR;
    if (kesh[indeks].haqiqiy && kesh[indeks].tag == tag) {
        urish++;
        return;
    }
    xato++;                                     /* keshda yo'q: xotiradan olib kelamiz */
    kesh[indeks].haqiqiy = 1;
    kesh[indeks].tag = tag;
}

static void hisobot(const char *nom)
{
    long jami = urish + xato;
    printf("%-30s urish %3ld, xato %3ld  (urish %5.1f%%)\n", nom, urish, xato, 100.0 * urish / jami);
}

int main(void)
{
    tozala();
    for (int i = 0; i < 64; i++)                /* 64 ta int, ketma-ket: manzil = i * 4 */
        murojaat((uint32_t)i * 4);
    hisobot("ketma-ket 64 ta int");

    tozala();
    for (int k = 0; k < 32; k++) {              /* 0 va 128 bir xil indeksga tushadi, tag boshqa */
        murojaat(0);
        murojaat(128);
    }
    hisobot("0 va 128 galma-gal (konflikt)");

    tozala();
    for (int i = 0; i < 8; i++)                 /* 8x8 int matritsa, QATOR bo'yicha */
        for (int j = 0; j < 8; j++)
            murojaat((uint32_t)(i * 8 + j) * 4);
    hisobot("matritsa 8x8, qator bo'yicha");

    tozala();
    for (int j = 0; j < 8; j++)                 /* xuddi shu matritsa, USTUN bo'yicha */
        for (int i = 0; i < 8; i++)
            murojaat((uint32_t)(i * 8 + j) * 4);
    hisobot("matritsa 8x8, ustun bo'yicha");
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g kesh.c -o kesh
$ ./kesh
ketma-ket 64 ta int            urish  48, xato  16  (urish  75.0%)
0 va 128 galma-gal (konflikt)  urish   0, xato  64  (urish   0.0%)
matritsa 8x8, qator bo'yicha   urish  48, xato  16  (urish  75.0%)
matritsa 8x8, ustun bo'yicha   urish   0, xato  64  (urish   0.0%)
```

Ketma-ket murojaatda har 16 baytlik qatordagi 4 ta `int` dan **birinchisi** xato, qolgan **uchtasi** bepul — urish 75%.
Konflikt holatida keshda joy bor-u, ikki manzil **bitta** joyni talashadi: har safar bir-birini siqib chiqaradi — urish 0%.
Ustun bo'yicha yurishda qadam 32 bayt: har murojaat yangi qator.

**Kengaytiring:** `QATORLAR` ni 16 qiling — qaysi natijalar o'zgaradi? `0` va `128` konflikti yo'qoladimi? (`128/16 = 8`, `8 % 16 = 8` — endi boshqa indeks.)

## Mustaqil loyiha: 2 yo'lli to'plamli LRU kesh ★★★

**Vazifa:** yuqoridagi simulyatorni **assotsiativ** kesh bilan almashtiring. Konflikt xatolarini kamaytirish uchun har bir indeksda
bitta emas, **bir nechta qator** ("yo'l") bor; xato bo'lganda **eng uzoq ishlatilmagan** (LRU) yo'l almashtiriladi.
Fayl: `kesh2.c`.

**Parametrlar:** qator 16 bayt; **to'plamlar** soni 4; yo'llar soni `Y` (1 yoki 2). Kesh sig'imi = 16 × 4 × Y.
- `qator_raqami = manzil / 16`; `to'plam = qator_raqami % 4`; `tag = qator_raqami / 4`.
- Murojaat: to'plamdagi barcha `Y` ta yo'lni tekshiring. Mos `tag` bo'lsa — urish (bu yo'lning "oxirgi ishlatilgan vaqti"ni yangilang).
- Xato: avval **bo'sh** yo'lni qidiring; bo'sh yo'q bo'lsa — **oxirgi ishlatilgan vaqti eng kichik** (LRU) yo'lni almashtiring.
- "Vaqt" — umumiy murojaatlar hisoblagichi.

`Y = 1` da bu — to'g'ridan-to'g'ri xaritalangan kesh.

**Uchta sinov** (barchasi `xotira manzili = bayt`):

| Nom | Murojaatlar | Yo'llar |
|---|---|---|
| **A** | 64 ta `int`, ketma-ket (`i·4`) | 2 |
| **B** | `0` va `128` galma-gal, 10 marta (jami 20 murojaat) | 1, keyin 2 |
| **C** | 16×16 `int` matritsa (`manzil = (i·16 + j)·4`): avval **qator**, keyin **ustun** bo'yicha (256 murojaatdan) | 2 |

**Chiqish shakli aniq:**

**Kutilgan natija** (`darslik/loyihalar/21_kesh_sim/kutilgan.txt`):

```text
A: ketma-ket 64 ta int             2 yo'l: xato  16, urish  48 ( 75.0%)
B: 0 va 128 galma-gal              1 yo'l: xato  20, urish   0 (  0.0%)
B: 0 va 128 galma-gal              2 yo'l: xato   2, urish  18 ( 90.0%)
C: matritsa 16x16, qator bo'yicha  2 yo'l: xato  64, urish 192 ( 75.0%)
C: matritsa 16x16, ustun bo'yicha  2 yo'l: xato 256, urish   0 (  0.0%)
```

**Maslahat** (yechim emas):
- Har yo'l uchun: `haqiqiy`, `tag`, `oxirgi_vaqt`. To'plam — shu tuzilmalar massivi (`kesh[4][2]`).
- B sinovda: `Y=1` da `0` va `128` bitta to'plamda o'z joyi uchun kurashadi (hammasi xato); `Y=2` da ikkalasi ham sig'adi (faqat 2 ta birinchi xato).
- C sinovi: ustun bo'yicha yurishda hamma qatorlar **bitta to'plamga** tushadi (nega? `qadam = 64 bayt = 4 qator`, `4 % 4 = 0`) — 2 yo'l ham qutqarmaydi. Bu — real dasturlarda
  ikkining darajali qadamning yomon ta'siri.
- Natijani foizda `%5.1f` bilan chiqaring: `100.0 * urish / jami`.

**Tekshirish:**

```bash
gcc -Wall -Wextra -g -fsanitize=address,undefined kesh2.c -o dastur && ./dastur | diff - ~/C_loyha/darslik/loyihalar/21_kesh_sim/kutilgan.txt && echo "TO'G'RI"
```
<!-- loyiha:oxiri -->

Keyingi bob: [22-bob. Bog'lash (linking) chuqur](22-boglash.md)
