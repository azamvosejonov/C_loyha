# 21-bob. Xotira ierarxiyasi va kesh — nega bir xil kod 10 marta tezroq bo'lishi mumkin

> **Bu bobdan keyin:** registr → kesh → RAM → disk pog'onalarini, kesh qatori (cache line) va lokallik
> tamoyilini, keshga mos kod yozishni, TLB'ni va ko'p yadroli tizimdagi "soxta bo'lishish" (false sharing)
> ni bilasiz. Yadro — tizimning eng ko'p ishlatiladigan kodi; uning tezligi shu bilimga bog'liq.

> **To'liq ishlaydigan misol:** [misollar/21_kesh.c](misollar/21_kesh.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Hayotdan misollar

**Tezlik pog'onalari — kitob qayerda turibdi (21.1).** Siz insho yozyapsiz va kitob kerak:
- **Registr** — kitob qo'lingizda ochiq turibdi (0 soniya).
- **L1 kesh** — stol ustida (1 soniya).
- **L2/L3 kesh** — xonadagi javonda (10 soniya).
- **RAM** — shahar kutubxonasida (~2 daqiqa).
- **SSD** — boshqa shahardagi arxivda (~bir kun).
- **Qattiq disk** — chet eldan pochta orqali (~bir necha oy).

Protsessor uchun RAM'ga borish — xuddi kutubxonaga borib kelishdek. Kesh shu yo'lni qisqartiradi.

**Lokallik — kitobning keyingi sahifasi (21.2).** Kitobning 50-sahifasini o'qigan bo'lsangiz, katta
ehtimol bilan keyin 51-sahifani o'qiysiz (**fazoviy lokallik**). Bugun ishlatgan lug'atingizni ertaga yana
ishlatasiz (**vaqt lokalligi**). Kesh aynan shunga garov o'ynaydi: yaqinda kerak bo'lgan narsa va uning
qo'shnilari yana kerak bo'ladi.

**Kesh qatori — kitobni emas, butun qutini olib kelish (21.3).** Kutubxonaga borganda bitta sahifani
emas, butun kitobni olasiz. Protsessor ham 1 baytni emas, 64 baytlik **qatorni** olib keladi. Keyingi
63 bayt bepul keladi — agar ular kerak bo'lsa.

**Massivni qator bo'yicha aylanish — kitobni ketma-ket o'qish (21.7).** Kitobni sahifama-sahifa o'qish
tez. Har kitobdan bittadan sahifa o'qib, keyingi kitobga o'tish (ustun bo'yicha aylanish) — har safar
kutubxonaga borish bilan barobar.

**Struct'lar massivi va alohida massivlar — ombor (21.7).** Omborda har bir mahsulot qutisida nomi,
rasmi, tavsifi va narxi bor. Faqat **narxlar yig'indisi** kerak bo'lsa, har bir katta qutini ochish
kerak. Narxlar alohida ro'yxatda bo'lsa — bitta varaqni o'qish kifoya. Kesh uchun ham shunday: faqat
kerakli maydonlar zich joylashsa, har bir 64 baytlik qatordan to'liq foydalaniladi.

**False sharing — bitta daftarga ikki kishi yozishi (21.5).** Ikki xodim bitta daftarning **turli**
qatorlariga yozadi, lekin daftar bitta — har safar uni bir-biriga uzatishga to'g'ri keladi. Ular bir-biriga
xalaqit bermaydi deb o'ylaydi, aslida vaqtning ko'pi uzatishga ketadi. Yechim: har biriga alohida daftar.

**TLB — telefondagi "tez-tez qo'ng'iroq qilinganlar" (21.6).** Virtual manzilni fizik manzilga tarjima
qilish sekin (24-bob). Protsessor oxirgi tarjimalarni kichik ro'yxatda saqlaydi — xuddi telefon tez-tez
kerak bo'ladigan raqamlarni eng tepada ko'rsatgandek.

### To'liq dastur: ombor hisoboti

Bir million mahsulot narxlarining yig'indisi — ikki xil joylashuvda. Vaqtlar kompyuterga qarab farq qiladi.

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
Har bir qutini ochib:   yig'indi 4995000000, 0.028 s
Alohida narxlar ro'yxati: yig'indi 4995000000, 0.004 s
Farq: taxminan 8 barobar
```

Ikkala sikl bir xil ishni qiladi va bir xil yig'indini beradi. Farq faqat ma'lumot xotirada qanday
joylashganida: birinchi holatda xotiradan 8 barobar ko'p bayt olib kelinadi.

**Sinab ko'ring:** `char nom[40]` ni `char nom[8]` qiling (struct 32 bayt bo'ladi) — farq qanday
o'zgaradi? `perf stat -e cache-misses ./ombor` bilan kesh xatolarini sanang (29-bob).

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

**Kesh** — kichik, tez xotira, sekinroq xotiradagi ma'lumotning **nusxasini** saqlaydi. Har bir pog'ona
keyingisi uchun kesh vazifasini bajaradi: RAM disk uchun (sahifa keshi — MyOS: buffer cache,
`kernel/fs/block.c`), L3 RAM uchun va hokazo.

## 21.2. Lokallik tamoyili — kesh nega ishlaydi

Dasturlar xotiraga tasodifiy murojaat qilmaydi:

- **Vaqt bo'yicha lokallik:** yaqinda ishlatilgan ma'lumot yana tez orada ishlatiladi (sikl
  o'zgaruvchisi, stek tepasi).
- **Joy bo'yicha lokallik:** ishlatilgan manzil yonidagilar ham tez orada ishlatiladi (massivni ketma-ket
  o'qish, ketma-ket buyruqlar).

Kesh shu ikkisidan foydalanadi: ma'lumot keshda qoladi (vaqt) va **butun kesh qatori** bilan olib
kelinadi (joy).

## 21.3. Kesh qatori (cache line)

Kesh xotira bilan baytma-bayt emas, **64 baytli bloklar** bilan almashadi. `a[0]` ni o'qisangiz,
`a[0..15]` (16 ta int) birdan keshga keladi — keyingi 15 tasi deyarli bepul.

```c
/* Tez: qatorma-qator (xotirada ketma-ket) */
for (int i = 0; i < N; i++)
    for (int j = 0; j < N; j++)
        s += m[i][j];

/* Sekin (N katta bo'lsa 5-10 marta): ustunma-ustun - har bir murojaat yangi kesh qatori */
for (int j = 0; j < N; j++)
    for (int i = 0; i < N; i++)
        s += m[i][j];
```

C'da ko'p o'lchamli massiv xotirada qatorma-qator (6-bob). Ichki sikl oxirgi indeks bo'yicha bo'lishi kerak.

**Ma'lumot tuzilmasini tanlashga ta'siri:**
- Massiv bog'langan ro'yxatdan ko'pincha **tezroq**, hatto nazariy murakkablik bir xil bo'lsa ham:
  massiv elementlari ketma-ket, ro'yxat tugunlari xotirada sochilgan — har bir tugun kesh xatosi.
- Tez-tez birga ishlatiladigan maydonlarni structda yonma-yon qo'ying; kam ishlatiladiganlarini
  oxiriga. Linux yadrosida muhim tuzilmalar (`struct page`, `task_struct`) aynan shunday tartiblangan.

## 21.4. Kesh qanday tashkil etilgan

Manzil uch qismga bo'linadi:

```text
[      teg      |  to'plam indeksi  | qator ichidagi siljish (6 bit, 64 bayt) ]
```

- **To'g'ridan-to'g'ri xaritalangan** (direct-mapped): har bir manzil faqat bitta joyga tushadi — oddiy,
  lekin ikki "raqib" manzil bir-birini doim siqib chiqaradi.
- **To'plam-assotsiativ** (set-associative, masalan 8-way): har bir to'plamda 8 ta joy — zamonaviy CPU'lar shunday.
- Joy tugasa — eng kam ishlatilgan (LRU'ga yaqin) qator chiqariladi (24-bobdagi sahifa almashtirish
  bilan bir xil g'oya).

**Kesh xatolarining 3 turi ("3C"):** birinchi murojaat (compulsory), sig'maslik (capacity), to'qnashuv (conflict).

## 21.5. Yozish va kogerentlik

CPU yozganda ma'lumot avval keshga yoziladi (write-back), RAM'ga keyinroq. Ko'p yadroli tizimda har bir
yadroning o'z L1/L2 si bor — bitta manzilning nusxasi bir nechta keshda bo'lishi mumkin. Apparat
**kogerentlik protokoli** (MESI) bilan ularni moslashtiradi: bir yadro yozsa, boshqalardagi nusxa
"yaroqsiz" qilinadi.

Bu dasturchiga ikki narsani anglatadi:

1. **Kesh qatori "ping-pong"i:** ikki yadro bitta qatorga navbatma-navbat yozsa, qator ular orasida
   doimiy ko'chib yuradi — juda sekin. Spinlock'da "test-and-test-and-set" (15-bob, 34-mashq) shuning
   uchun kerak: kutayotganlar faqat **o'qiydi**, qator hamma keshda "umumiy" holatda turadi.
2. **Soxta bo'lishish (false sharing):** ikki yadro **turli** o'zgaruvchilarga yozadi, lekin ular bitta
   64 baytli qatorda — natija xuddi bitta o'zgaruvchidek sekin.

```c
struct hisoblagich { long qiymat; } h[8];         /* 8 ta yadro uchun - hammasi 1-2 qatorda: SEKIN */

struct hisoblagich {
    long qiymat;
} __attribute__((aligned(64))) h[8];             /* har biri alohida qatorda: TEZ */
```

MyOS'da: per-CPU ma'lumotlar (`kernel/arch/percpu.c`) — har bir CPU'ning o'z tuzilmasi, qulfsiz va
bo'lishishsiz. Linux'da `DEFINE_PER_CPU` va `____cacheline_aligned`.

## 21.6. TLB — manzil tarjimasi keshi

Har bir xotira murojaatida virtual manzilni fizikka aylantirish kerak (4 darajali sahifa jadvali —
31-mashq). Har safar 4 marta xotiraga borish — halokatli sekin. **TLB** (Translation Lookaside Buffer) —
oxirgi tarjimalarning kichik keshi (~1500 yozuv).

Oqibatlari:
- **CR3 almashtirilganda** (boshqa jarayonga o'tish) TLB tozalanadi → kontekst almashishning yashirin
  narxi. (PCID texnologiyasi buni qisman hal qiladi.)
- Sahifa jadvalini o'zgartirgan yadro **TLB'ni ham tozalashi** kerak: bitta sahifa uchun `invlpg`,
  boshqa CPU'larda — "TLB shootdown" (uzilish yuborib, ularni ham tozalatish). MyOS: `kernel/mm/vmm.c`,
  `docs/10-smp.md`.
- **Katta sahifalar** (2 MB, 1 GB) — bitta TLB yozuvi ko'p xotirani qamraydi. MyOS'ning direct map'i
  2 MB sahifalar bilan quriladi (`[vmm] Yadro xaritasi: ... 2 MB/4 KB sahifalar`).

## 21.7. Keshga mos dasturlash qoidalari

1. Ma'lumotni ketma-ket o'qing (massiv > ro'yxat, qatorma-qator).
2. Ishchi to'plamni kichik qiling: katta ma'lumotni bloklarga bo'lib ishlang (blocking/tiling).
3. Birga ishlatiladigan maydonlarni yaqin joylashtiring; "issiq" va "sovuq" maydonlarni ajrating.
4. Ko'p yadroli yozishlarda: har bir yadro o'z ma'lumotiga (per-CPU), umumiy o'zgaruvchilarni kam yozing,
   turli yadrolar yozadigan o'zgaruvchilarni turli kesh qatorlariga qo'ying.
5. **Taxmin qilmang — o'lchang** (29-bob: `perf stat -e cache-misses`).

## 21.8. O'zingizni tekshiring

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

## 21.9. Mashq

Qo'shimcha tajriba (natijani o'zingiz o'lchang): 4096×4096 `int` matritsani qatorma-qator va
ustunma-ustun yig'ing, `clock()` bilan vaqtni solishtiring (`-O2`). Keyin 8 ta oqim bilan 21.5-dagi
ikkala `struct hisoblagich` variantini sinang.

Keyingi bob: [22-bob. Bog'lash (linking) chuqur](22-boglash.md)
