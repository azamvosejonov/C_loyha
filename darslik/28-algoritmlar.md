# 28-bob. Algoritmlar va ma'lumotlar tuzilmalari — yadro dasturchisi uchun

> **Bu bobda nima o'rganasiz:** dastur tezligini "soniyada" emas, "ma'lumot ko'payganda ish qanchaga o'sadi" deb baholashni (O-belgi); asosiy tuzilmalarni (massiv, bog'langan ro'yxat, stek, navbat, xesh jadval, daraxt, heap, graf);
> qidirish va saralash algoritmlarini; va ularning har biri **yadroda qayerda** ishlatilishini.
> **Oldindan nima kerak:** 5-, 7-, 8-boblar (funksiya, ko'rsatkich, malloc).   **Vaqt:** 10–14 soat.
> Mashqlar: 13, 16, 17, 19, 21, 22, 32, 42, 46, 47, 48.

> **To'liq ishlaydigan misol:** [misollar/28_algoritmlar.c](misollar/28_algoritmlar.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Bu bob nima haqida?

Dastur yozish ikki qismdan iborat: (1) **ma'lumotni qanday saqlash** (tuzilma), (2) **u bilan qanday ishlash** (algoritm). Bir xil masalani yomon tuzilma bilan yechsangiz, dastur 1000 marta sekin bo'lishi mumkin. Yadroda bu ayniqsa muhim:
scheduler sekundiga minglab marta ishlaydi, sahifa xatosi ishlovchisi — undan ham tez-tez. Sekin algoritm butun kompyuterni to'xtatadi.

Bu bob — "lug'at": har bir tuzilmaning g'oyasi, kuchli va kuchsiz tomoni, yadroda qayerda uchrashi. Hammasini yodlash shart emas; **har birini kamida bir marta o'zingiz C'da yozing** (mashqlar).

**Hayotdan misol: kutubxona.** Kitoblarni qanday saqlashingiz qidirish tezligini belgilaydi:

| Saqlash usuli | Qidirish | Kompyuterdagi nomi |
|---|---|---|
| uyum bo'lib to'plangan | har birini ko'rib chiqish | massiv/ro'yxat, **O(n)** |
| alifbo tartibida javonda | o'rtasini och, "oldinmi-keyinmi?", yarmini tashla | **ikkilik qidiruv**, O(log n) |
| har kitob uchun raqamli ilmoq (raqamcha bo'yicha) | to'g'ridan-to'g'ri olish | **xesh jadval**, O(1) |
| mavzu → bo'lim → javon → kitob | bosqichma-bosqich tushish | **daraxt** |
| metro xaritasi (bekatlar va yo'llar) | "eng kam bekat bilan qanday yetaman?" | **graf**, BFS |

## 28.1. Murakkablik: O-belgi

**Oddiy qilib aytganda:** algoritm tezligini soniyalarda o'lchab bo'lmaydi (kompyuterga bog'liq). Shuning uchun: **kirish ma'lumoti n marta ko'payganda, ish qancha ko'payadi?** — shuni baholaymiz. Bu **O-belgi** (katta O).

**Hayotdan misol:**

- **O(1)** — garderob: raqamcha bo'yicha paltoni darhol olasiz. 10 ta palto ham, 10 000 ta ham bir xil.
- **O(log n)** — lug'atda so'z qidirish: o'rtasini ochasiz, "S" dan oldinmi-keyinmi, yana yarmi... Million so'zli lug'atda ~20 qadam.
- **O(n)** — tartibsiz qog'ozlar uyumida bitta hujjatni qidirish: har birini ko'rib chiqasiz.
- **O(n log n)** — kartalarni yaxshi usul bilan saralash.
- **O(n²)** — sinfdagi har o'quvchi har biri bilan qo'l berib ko'rishadi: 30 kishi — 435 ta qo'l berish, 300 kishi — 44 850 ta. Odam 10 barobar ko'paydi, ish **100** barobar.

| O(...) | Nomi | n = 1 000 000 da taxminiy qadamlar | Misol |
|---|---|---|---|
| O(1) | o'zgarmas | 1 | massiv indeksi, xesh jadval (o'rtacha) |
| O(log n) | logarifmik | 20 | ikkilik qidiruv, muvozanatli daraxt |
| O(n) | chiziqli | 10⁶ | massivni aylanish |
| O(n log n) | | 2·10⁷ | yaxshi saralash |
| O(n²) | kvadratik | 10¹² — soatlar! | ichma-ich sikl, oddiy saralash |
| O(2ⁿ) | eksponensial | koinot yoshidan uzoq | hamma to'plamostilarni sanash |

**Bu dastur nima qiladi (umumiy):** uchta ish uchun **qadamlarni sanaydi**: (1) tartibsiz kabi boshidan qidirish (chiziqli), (2) saralangan massivda ikkilik qidiruv, (3) massivdagi barcha juftliklarni sanash (ichma-ich sikl). n ni 10 dan million gacha oshirib, har birida ish qanday o'sishini ko'rsatadi.

```c
/* murakkablik.c - qadamlarni sanaymiz: O(n), O(log n), O(n^2) */
#include <stdio.h>

static long chiziqli(const int *a, int n, int x, long *qadam)      /* O(n) */
{
    *qadam = 0;
    for (int i = 0; i < n; i++) {
        (*qadam)++;
        if (a[i] == x)
            return i;
    }
    return -1;
}

static long ikkilik(const int *a, int n, int x, long *qadam)       /* O(log n) */
{
    int chap = 0, ong = n;
    *qadam = 0;
    while (chap < ong) {
        (*qadam)++;
        int orta = chap + (ong - chap) / 2;
        if (a[orta] < x)
            chap = orta + 1;
        else if (a[orta] > x)
            ong = orta;
        else
            return orta;
    }
    return -1;
}

static long juftlar(int n)                                          /* O(n^2) */
{
    if (n > 1000)                                       /* sikl juda uzoq ishlaydi: formula n(n-1)/2 */
        return (long)n * (n - 1) / 2;
    long qadam = 0;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            qadam++;
    return qadam;
}

int main(void)
{
    static int a[1000000];
    for (int i = 0; i < 1000000; i++)
        a[i] = i * 2;                           /* saralangan: 0, 2, 4, ... */

    printf("%10s | %14s | %16s | %14s\n", "n", "chiziqli qidiruv", "ikkilik qidiruv", "barcha juftlar");
    int olchamlar[] = { 10, 1000, 100000, 1000000 };
    for (int i = 0; i < 4; i++) {
        int n = olchamlar[i];
        long q1, q2;
        chiziqli(a, n, a[n - 1], &q1);          /* eng yomon holat: oxirgi element */
        ikkilik(a, n, a[n - 1], &q2);
        printf("%10d | %17ld | %16ld | %14ld\n", n, q1, q2, juftlar(n));
    }
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 murakkablik.c -o murakkablik
$ ./murakkablik
         n | chiziqli qidiruv |  ikkilik qidiruv | barcha juftlar
        10 |                10 |                3 |             45
      1000 |              1000 |                9 |         499500
    100000 |            100000 |               16 |     4999950000
   1000000 |           1000000 |               19 |   499999500000
```

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `chiziqli` | massivni boshidan oxirigacha ko'radi; har ko'rishda `qadam++` |
| `ikkilik` | oraliqni har qadamda yarmiga qisqartiradi (28.5) |
| `juftlar` | `i` va `j > i` juftliklari: ichma-ich sikl → n(n−1)/2 |
| `a[n - 1]` ni qidirish | **eng yomon holat**: element oxirida |

**Nima ko'rdik:** n 10 dan 1 000 000 gacha (10⁵ marta) o'sganda: chiziqli qidiruv qadamlari 10⁵ marta o'sdi; ikkilik qidiruv — atigi 3 dan 19 gacha; juftliklar — 45 dan 5·10¹¹ gacha (10¹⁰ marta!).
Million elementda ikkilik qidiruv 19 qadam, chiziqli — million qadam, kvadratik — yarim trillion.

**Qoidalar:** o'zgarmas ko'paytuvchilar tashlanadi (O(3n) = O(n)); eng tez o'sadigan had qoladi (O(n² + n) = O(n²)). **Eng yomon** va **o'rtacha** holat alohida baholanadi (xesh jadval: o'rtacha O(1), eng yomon O(n)).
**Amortizatsiyalangan:** dinamik massivda `push` ba'zan O(n) (qayta ajratish), lekin hajm 2 barobar oshgani uchun o'rtacha O(1) (13-mashq).

**Yadroda nega muhim:** O(n) algoritm 10 jarayonda sezilmaydi, 10 000 da tizimni to'xtatadi. Linux'da O(1) scheduler, keyin CFS (O(log n)) aynan shu sababdan yaratilgan. **Xotira murakkabligi** ham hisobga olinadi: yadro stekida rekursiya chuqurligi O(n) bo'lishi mumkin emas.

> **Eslab qoling:** O-belgi = "n ko'paysa ish qanday o'sadi". O(1) ≪ O(log n) ≪ O(n) ≪ O(n log n) ≪ O(n²). Katta n da kvadratik algoritm — falokat.

## 28.2. Chiziqli tuzilmalar: massiv, ro'yxat, stek, navbat

| Tuzilma | Kirish (i-element) | Qidirish | Qo'shish/o'chirish | Yadroda |
|---|---|---|---|---|
| **Massiv** | O(1) | O(n) (saralangan: O(log n)) | oxiriga O(1)*, o'rtaga O(n) | jadvallar, fd jadvali |
| **Bog'langan ro'yxat** | O(n) | O(n) | tugun ma'lum bo'lsa **O(1)** | `list_head` hamma joyda (23-mashq) |
| **Stek** (LIFO) | tepa O(1) | — | O(1) | chaqiruvlar steki, 24-mashq |
| **Navbat** (FIFO) | boshi O(1) | — | O(1) | scheduler, halqa bufer (22), I/O navbatlari |

### Bog'langan ro'yxat

**Oddiy qilib aytganda:** massivda elementlar yonma-yon; ro'yxatda har element **o'zidan keyingisining manzilini** saqlaydi. **Hayotdan misol: xazina qidirish o'yini** — har yozuvda "keyingi yozuv qayerda" deb yozilgan. 5-yozuvga yetish uchun
1, 2, 3, 4 dan o'tish kerak. Lekin o'rtaga yangi yozuv qo'shish oson: faqat ikkita "keyingi" ni o'zgartirasiz.

**Bu dastur nima qiladi (umumiy):** 4 ta tugunli ro'yxat yaratadi (`10→20→30→40`), 20 dan keyin 25 ni **faqat ikkita ko'rsatkichni** o'zgartirib qo'shadi, keyin o'chiradi, va oxirgi elementga necha qadamda yetishni sanaydi.

```c
/* royxat.c - bog'langan ro'yxat: o'rtaga qo'shish O(1), n-elementga borish O(n) */
#include <stdio.h>
#include <stdlib.h>

struct tugun {
    int qiymat;
    struct tugun *keyingi;
};

static struct tugun *yangi(int q, struct tugun *keyingi)
{
    struct tugun *t = malloc(sizeof(*t));
    t->qiymat = q;
    t->keyingi = keyingi;
    return t;
}

static void chiqar(const char *izoh, const struct tugun *bosh)
{
    printf("%-28s", izoh);
    for (const struct tugun *t = bosh; t; t = t->keyingi)
        printf("%d%s", t->qiymat, t->keyingi ? " -> " : "\n");
}

int main(void)
{
    struct tugun *bosh = yangi(10, yangi(20, yangi(30, yangi(40, NULL))));
    chiqar("boshida:", bosh);

    struct tugun *ikkinchi = bosh->keyingi;             /* 20 ning manzili ma'lum */
    ikkinchi->keyingi = yangi(25, ikkinchi->keyingi);   /* 20 dan keyin 25: FAQAT 2 ta ko'rsatkich */
    chiqar("20 dan keyin 25 qo'shildi:", bosh);

    struct tugun *olinadi = ikkinchi->keyingi;          /* 25 ni o'chiramiz */
    ikkinchi->keyingi = olinadi->keyingi;
    free(olinadi);
    chiqar("25 o'chirildi:", bosh);

    int qadam = 0;                                      /* 4-elementga yetish uchun yurish kerak */
    const struct tugun *t = bosh;
    while (t->keyingi) {
        t = t->keyingi;
        qadam++;
    }
    printf("oxirgi element (%d) ga %d qadamda yetdik (massivda a[3] - bitta amal)\n", t->qiymat, qadam);

    while (bosh) {
        struct tugun *keyingi = bosh->keyingi;
        free(bosh);
        bosh = keyingi;
    }
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 royxat.c -o royxat
$ ./royxat
boshida:                    10 -> 20 -> 30 -> 40
20 dan keyin 25 qo'shildi:  10 -> 20 -> 25 -> 30 -> 40
25 o'chirildi:              10 -> 20 -> 30 -> 40
oxirgi element (40) ga 3 qadamda yetdik (massivda a[3] - bitta amal)
```

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `struct tugun { qiymat; keyingi }` | bitta ro'yxat elementi: qiymat + keyingi tugun manzili (`NULL` — oxiri) |
| `yangi(q, keyingi)` | `malloc` bilan tugun yaratadi va uni `keyingi` ga bog'laydi |
| `ikkinchi->keyingi = yangi(25, ikkinchi->keyingi)` | yangi tugun eski "keyingi" ga ulanadi, so'ng 20 yangi tugunga ishora qiladi — **ikki ko'rsatkich**, hech narsa surilmaydi |
| `ikkinchi->keyingi = olinadi->keyingi; free(olinadi)` | o'chirish: atrofidagilar bir-biriga bog'lanadi, o'zi `free` |
| `while (t->keyingi)` | oxirgi elementga yetish uchun **hamma tugunlardan yurish** kerak |

Trace — 20 dan keyin 25 qo'shish:

| Qadam | Holat |
|---|---|
| boshida | `20 → 30` |
| 1) `yangi(25, 20.keyingi)` | yangi tugun: `25 → 30` (20 hali 30 ga ishora qilmoqda) |
| 2) `20.keyingi = yangi` | `20 → 25 → 30` |

**Nima ko'rdik:** o'rtaga qo'shish/o'chirish — O(1) (tugun manzili ma'lum bo'lsa), lekin 4-elementga yetish uchun 3 qadam yurish kerak; massivda `a[3]` — bitta amal. Kichik `n` da massiv deyarli doim yutadi: elementlar yonma-yon turgani uchun
kesh (21-bob) yaxshi ishlaydi, ro'yxat tugunlari esa xotirada tarqoq.

### Stek va navbat

**Oddiy qilib aytganda:** stek — **likopchalar ustuni**: oxirgi qo'yilgan birinchi olinadi (**LIFO**). Navbat — **do'kon kassasi**: birinchi kelgan birinchi ketadi (**FIFO**).

**Bu dastur nima qiladi (umumiy):** 1, 2, 3 ni stekka va navbatga kiritadi va chiqarilish tartibini ko'rsatadi. Keyin navbat **halqa** (aylanma) buferda ishlashini ko'rsatadi: indeks oxiriga yetgach boshiga qaytadi.

```c
/* stek_navbat.c - stek (LIFO) va navbat (FIFO) massivda */
#include <stdio.h>

#define N 4

static int stek[N], tepa = 0;

static void stek_push(int x) { stek[tepa++] = x; }
static int stek_pop(void) { return stek[--tepa]; }

static int navbat[N], bosh = 0, soni = 0;               /* halqa bufer: oxiri boshiga ulangan */

static void nav_qosh(int x) { navbat[(bosh + soni++) % N] = x; }

static int nav_ol(void)
{
    int x = navbat[bosh];
    bosh = (bosh + 1) % N;
    soni--;
    return x;
}

int main(void)
{
    printf("stek:   kiritdik 1 2 3, chiqdi:");
    stek_push(1);
    stek_push(2);
    stek_push(3);
    while (tepa)
        printf(" %d", stek_pop());

    printf("\nnavbat: kiritdik 1 2 3, chiqdi:");
    nav_qosh(1);
    nav_qosh(2);
    nav_qosh(3);
    while (soni)
        printf(" %d", nav_ol());

    nav_qosh(7);                                        /* halqa: indeks aylanib boshiga qaytadi */
    nav_qosh(8);
    nav_qosh(9);
    printf("\nhalqa:  7 8 9 ni qo'shdik, bosh = %d, chiqdi:", bosh);
    while (soni)
        printf(" %d", nav_ol());
    printf("\n");
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 stek_navbat.c -o stek_navbat
$ ./stek_navbat
stek:   kiritdik 1 2 3, chiqdi: 3 2 1
navbat: kiritdik 1 2 3, chiqdi: 1 2 3
halqa:  7 8 9 ni qo'shdik, bosh = 3, chiqdi: 7 8 9
```

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `stek[]`, `tepa` | `tepa` — keyingi bo'sh katak; `push`: `stek[tepa++] = x`; `pop`: `stek[--tepa]` |
| `navbat[]`, `bosh`, `soni` | `bosh` — eng eski element; yangi element o'rni `(bosh + soni) % N` |
| `% N` | indeks `N` ga yetsa 0 ga qaytadi — **halqa bufer** (22-mashq) |

**Nima ko'rdik:** stek `3 2 1` (teskari), navbat `1 2 3` (kiritilgan tartibda). Halqada 7, 8, 9 buferning boshiga qaytib aylangan (bosh = 3) holda ham to'g'ri tartibda chiqdi. Yadroda: stek — funksiya chaqiruvlari; navbat — scheduler, klaviatura/tarmoq buferlari.

> **Eslab qoling:** massiv — tez kirish; ro'yxat — tez qo'shish/o'chirish (joyi ma'lum bo'lsa); stek — oxirgi birinchi; navbat — birinchi birinchi.

## 28.3. Xesh jadval

**Oddiy qilib aytganda:** qidirishni O(1) ga tushirish usuli. **Hayotdan misol: garderob.** Paltoni ilmoqqa ilasiz, raqamcha olasiz; qaytib kelganda raqamcha bo'yicha **darhol** topasiz — hamma paltolarni ko'rib chiqmaysiz.

Xesh jadvalda raqamchaning o'rnini **xesh funksiya** bajaradi: u kalitdan (masalan, so'zdan) massiv indeksini (**cho'ntak**) **hisoblaydi**:

```text
kalit ("yadro")  --xesh funksiya-->  son (281247332)  --% 8-->  cho'ntak raqami (4)
```

**To'qnashuv:** ikki xil kalit bir xil cho'ntakka tushishi mumkin (ilmoqqa ikki palto). Yechimlar:

- **Zanjir** (chaining): har cho'ntakda ro'yxat (17-mashq). Linux: `hlist_head` massivlari.
- **Ochiq adreslash**: to'qnashsa, keyingi bo'sh joyni qidirish. Keshga mosroq.

**Bu dastur nima qiladi (umumiy):** 10 ta so'zni `djb2` xesh funksiyasi bilan 8 ta cho'ntakka taqsimlaydi: har so'zning xesh qiymati va cho'ntagi, so'ng cho'ntaklardagi so'zlar soni, to'qnashuvlar va yuklama koeffitsienti.

```c
/* xesh_demo.c - xesh funksiya so'zni cho'ntak raqamiga aylantiradi; to'qnashuvlar */
#include <stdio.h>
#include <string.h>

#define KOVAKLAR 8

static unsigned xesh(const char *s)                     /* djb2 */
{
    unsigned h = 5381;
    while (*s)
        h = h * 33 + (unsigned char)*s++;
    return h;
}

int main(void)
{
    const char *sozlar[] = { "yadro", "xotira", "jarayon", "oqim", "qulf", "disk", "fayl", "kesh", "stek", "heap" };
    int n = 10, soni[KOVAKLAR] = { 0 };

    printf("%-8s %12s   %s\n", "so'z", "xesh", "cho'ntak = xesh % 8");
    for (int i = 0; i < n; i++) {
        unsigned h = xesh(sozlar[i]);
        printf("%-8s %12u   %u\n", sozlar[i], h, h % KOVAKLAR);
        soni[h % KOVAKLAR]++;
    }

    printf("\ncho'ntaklar: ");
    int band = 0, eng = 0, tonqnash = 0;
    for (int k = 0; k < KOVAKLAR; k++) {
        printf("[%d]=%d ", k, soni[k]);
        band += soni[k] > 0;
        eng = soni[k] > eng ? soni[k] : eng;
        tonqnash += soni[k] > 1 ? soni[k] - 1 : 0;
    }
    printf("\n%d so'z, %d ta cho'ntak band, eng uzun zanjir %d, to'qnashuvlar %d\n", n, band, eng, tonqnash);
    printf("yuklama koeffitsienti = %d / %d = %.2f\n", n, KOVAKLAR, (double)n / KOVAKLAR);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 xesh_demo.c -o xesh_demo
$ ./xesh_demo
so'z             xesh   cho'ntak = xesh % 8
yadro       281247332   4
xotira      669260252   4
jarayon    3454490105   1
oqim       2090589243   3
qulf       2090665565   5
disk       2090185552   0
fayl       2090248913   1
kesh       2090432752   0
stek       2090736124   4
heap       2090324355   3

cho'ntaklar: [0]=2 [1]=2 [2]=0 [3]=2 [4]=3 [5]=1 [6]=0 [7]=0 
10 so'z, 5 ta cho'ntak band, eng uzun zanjir 3, to'qnashuvlar 5
yuklama koeffitsienti = 10 / 8 = 1.25
```

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `xesh(s)` (**djb2**) | `h = 5381`, har belgi uchun `h = h*33 + belgi` — belgilarni sonlarga "aralashtiradi" |
| `h % KOVAKLAR` | xesh sonini 0…7 oralig'iga keltiradi (cho'ntak raqami) |
| `soni[k]` | `k`-cho'ntakka nechta so'z tushdi |
| `tonqnash` | "ortiqcha" so'zlar: har cho'ntakda 1 tadan ortig'i |

**Nima ko'rdik:** 10 so'z 8 cho'ntakka tushdi; 3 ta cho'ntak bo'sh qoldi (2, 6, 7), 4-cho'ntakda 3 ta so'z (`yadro`, `xotira`, `stek`) — eng uzun zanjir 3. Yuklama koeffitsienti = 10/8 = 1.25: elementlar cho'ntaklardan ko'p, shuning uchun to'qnashuv muqarrar.

**Yuklama koeffitsienti** α = elementlar / cho'ntaklar. α oshsa — zanjirlar uzayadi va jadval sekinlashadi. α > 1 (zanjirda) yoki α > 0.7 (ochiq adreslashda) bo'lganda jadval **2 barobar kattalashtiriladi va hamma element qayta joylashtiriladi** (*rehash*) — Python `dict` ham shunday.
Bobning "Loyiha" qismida xesh jadvalni to'liq o'zingiz qurasiz.

**Yaxshi xesh funksiya** bitlarni yaxshi aralashtiradi: FNV-1a, MurmurHash; Linux'da `jhash`, `hash_long`. **Xavfsizlik:** hujumchi ataylab bir cho'ntakka tushadigan kalitlar yuborsa — qidirish O(n) ga tushadi (DoS). Yechim — tasodifiy "tuz" (seed).

Yadroda: PID → jarayon, inode keshi, dentry keshi, tarmoq ulanishlari jadvali.

> **Eslab qoling:** xesh jadval = massiv + xesh funksiya. O'rtacha O(1), eng yomon O(n). Yuklama oshsa — kengaytiriladi.

## 28.4. Ikkilik qidiruv

**Oddiy qilib aytganda:** **saralangan** massivda elementni topish: o'rtasiga qarang. Izlanayotgan son o'rtadagidan kichikmi? — o'ng yarmini tashlang. Kattami? — chap yarmini tashlang. Har qadamda ish ikki barobar kamayadi → O(log n).

**Hayotdan misol:** 1–100 oralig'ida o'ylangan sonni topish: "50 dan kattami?" — 7 savolda topasiz.

**Bu dastur nima qiladi (umumiy):** 16 elementli saralangan massivda (`0 3 6 ... 45`) 27 ni qidiradi — har qadamda qaysi oraliq qolganini va qaysi tomonga ketishini ko'rsatadi; keyin massivda yo'q 28 ni qidiradi. Oxirida mashhur xatoni (`(chap+ong)/2` toshishi) 8 bitli indekslarda ko'rsatadi.

```c
/* ikkilik_qidiruv.c - iz bilan ikkilik qidiruv va "(chap+ong)/2" xatosi */
#include <stdint.h>
#include <stdio.h>

static long qidir(const int *a, int n, int x)
{
    int chap = 0, ong = n;                              /* [chap, ong) - yarim ochiq oraliq */
    int qadam = 0;
    while (chap < ong) {
        int orta = chap + (ong - chap) / 2;
        printf("  qadam %d: oraliq [%2d,%2d) o'rtasi a[%2d]=%2d  ", ++qadam, chap, ong, orta, a[orta]);
        if (a[orta] < x) {
            printf("< %d -> o'ngga\n", x);
            chap = orta + 1;
        } else if (a[orta] > x) {
            printf("> %d -> chapga\n", x);
            ong = orta;
        } else {
            printf("== %d -> TOPILDI\n", x);
            return orta;
        }
    }
    printf("  oraliq bo'sh -> topilmadi\n");
    return -1;
}

int main(void)
{
    int a[16];
    for (int i = 0; i < 16; i++)
        a[i] = i * 3;                                   /* 0 3 6 9 ... 45 */

    printf("a[] = 0 3 6 9 12 ... 45;  qidiramiz: 27\n");
    printf("natija indeks: %ld\n", qidir(a, 16, 27));
    printf("qidiramiz: 28 (massivda yo'q)\n");
    printf("natija indeks: %ld\n", qidir(a, 16, 28));

    /* "(chap + ong) / 2" xatosi: indekslar 8 bitli sig'imga mos kelmasa */
    uint8_t chap = 200, ong = 250;
    uint8_t xato = (uint8_t)(chap + ong) / 2;           /* 450 -> 194 (toshdi!) -> 97 */
    uint8_t togri = (uint8_t)(chap + (ong - chap) / 2); /* 200 + 25 = 225 */
    printf("\n8 bitli indekslar: chap=200, ong=250\n  (chap+ong)/2         = %u  <- XATO: oraliqdan tashqarida\n  chap+(ong-chap)/2    = %u  <- to'g'ri\n",
           xato, togri);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 ikkilik_qidiruv.c -o ikkilik_qidiruv
$ ./ikkilik_qidiruv
a[] = 0 3 6 9 12 ... 45;  qidiramiz: 27
  qadam 1: oraliq [ 0,16) o'rtasi a[ 8]=24  < 27 -> o'ngga
  qadam 2: oraliq [ 9,16) o'rtasi a[12]=36  > 27 -> chapga
  qadam 3: oraliq [ 9,12) o'rtasi a[10]=30  > 27 -> chapga
  qadam 4: oraliq [ 9,10) o'rtasi a[ 9]=27  == 27 -> TOPILDI
natija indeks: 9
qidiramiz: 28 (massivda yo'q)
  qadam 1: oraliq [ 0,16) o'rtasi a[ 8]=24  < 28 -> o'ngga
  qadam 2: oraliq [ 9,16) o'rtasi a[12]=36  > 28 -> chapga
  qadam 3: oraliq [ 9,12) o'rtasi a[10]=30  > 28 -> chapga
  qadam 4: oraliq [ 9,10) o'rtasi a[ 9]=27  < 28 -> o'ngga
  oraliq bo'sh -> topilmadi
natija indeks: -1

8 bitli indekslar: chap=200, ong=250
  (chap+ong)/2         = 97  <- XATO: oraliqdan tashqarida
  chap+(ong-chap)/2    = 225  <- to'g'ri
```

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `[chap, ong)` | **yarim ochiq** oraliq: `chap` kiradi, `ong` kirmaydi; `ong = n` dan boshlanadi |
| `orta = chap + (ong - chap) / 2` | oraliq o'rtasi — toshmaydigan shakl |
| `a[orta] < x` → `chap = orta + 1` | o'rta kichik: qidirilayotgani o'ngda; `orta` ni ham tashlaymiz |
| `a[orta] > x` → `ong = orta` | o'rta katta: chapda; `ong` kirmagani uchun `orta` ham tashlanadi |
| `chap >= ong` | oraliq bo'sh → topilmadi |

**Nima ko'rdik:** 16 element — eng ko'pi bilan 4–5 qadam. Qidirish 27 → 4 qadamda topildi. 28 yo'q: 9-indeksga yetib, oraliq bo'shadi → `-1`.

**Mashhur xato.** `(chap + ong) / 2` — katta indekslarda yig'indi **toshadi** va noto'g'ri o'rta chiqadi. Bu xato Java standart kutubxonasida ~9 yil yashagan (2006-yilda topilgan). Dasturdagi 8 bitli misol buni ko'rsatadi: 200 va 250 uchun
`(200+250)` 8 bitga sig'maydi (450 → 194), natijada 97 chiqadi — oraliqdan tashqarida. `chap + (ong - chap) / 2` doim xavfsiz.

Ikkilik qidiruv g'oyasi faqat massiv uchun emas: `git bisect` (19-bob), "eng kichik yaroqli qiymat" topish (javob bo'yicha ikkilik qidiruv).

> **Eslab qoling:** ikkilik qidiruv — faqat **saralangan** ma'lumotda, O(log n); o'rtani `chap + (ong - chap) / 2` deb hisoblang; oraliqni `[chap, ong)` ko'rinishida saqlang.

## 28.5. Saralash

**Oddiy qilib aytganda:** elementlarni tartibga solish. Ko'p algoritm bor; ularning farqi **qancha taqqoslash** kerakligida va qo'shimcha xotira talab qilishida.

| Algoritm | O'rtacha | Eng yomon | Qo'shimcha xotira | Barqaror | Qachon |
|---|---|---|---|---|---|
| Qo'yish (insertion) | O(n²) | O(n²) | O(1) | ha | n < ~20, deyarli saralangan |
| Birlashtirish (merge) | O(n log n) | O(n log n) | O(n) | ha | kafolat kerak, ro'yxatlar (19-mashq) |
| Tez (quick) | O(n log n) | O(n²) | O(log n) | yo'q | amalda eng tez |
| Heap | O(n log n) | O(n log n) | O(1) | yo'q | kafolat + xotirasiz — **Linux `sort()` shuni ishlatadi** |
| Sanash (counting/radix) | O(n + k) | | O(k) | ha | kichik butun kalitlar |

"Barqaror" — teng kalitlarning nisbiy tartibi saqlanadi. Taqqoslashga asoslangan saralash O(n log n) dan tez bo'lishi mumkin emas (isbotlangan).

**G'oyalar:**

- **Qo'yish:** har elementni olib, chap tomondagi saralangan qismdagi o'z o'rniga "suqib" qo'yish.
- **Birlashtirish:** massivni teng ikkiga bo'l, har birini sarala, ikki saralangan qismni birlashtir.
- **Tez (quicksort):** tayanch (pivot) tanla; undan kichiklarni chapga, kattalarni o'ngga o'tkaz; ikkala qismni rekursiv sarala.

**Bu dastur nima qiladi (umumiy):** uchta algoritmni (2000 ta tasodifiy son va saralangan kirish bilan) ishga tushiradi va har birida **taqqoslashlar sonini** sanaydi; natija to'g'ri saralanganini ham tekshiradi. Quicksort'da tayanch sifatida **birinchi element** olingan.

```c
/* saralash.c - saralash algoritmlarining taqqoslashlar soni */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 2000

static long taqqos;                                     /* taqqoslashlar hisoblagichi */

static int kichik(int a, int b)
{
    taqqos++;
    return a < b;
}

static void qoyish(int *a, int n)                       /* insertion sort: O(n^2) */
{
    for (int i = 1; i < n; i++) {
        int x = a[i], j = i - 1;
        while (j >= 0 && kichik(x, a[j])) {
            a[j + 1] = a[j];
            j--;
        }
        a[j + 1] = x;
    }
}

static void birlash(int *a, int *t, int n)              /* merge sort: O(n log n) */
{
    if (n < 2)
        return;
    int m = n / 2;
    birlash(a, t, m);
    birlash(a + m, t, n - m);
    int i = 0, j = m, k = 0;
    while (i < m && j < n)
        t[k++] = kichik(a[j], a[i]) ? a[j++] : a[i++];
    while (i < m)
        t[k++] = a[i++];
    while (j < n)
        t[k++] = a[j++];
    memcpy(a, t, (size_t)n * sizeof(int));
}

static void tez(int *a, int n)                          /* quicksort, pivot = BIRINCHI element */
{
    if (n < 2)
        return;
    int pivot = a[0], i = 0;
    for (int j = 1; j < n; j++)
        if (kichik(a[j], pivot)) {
            i++;
            int t = a[i]; a[i] = a[j]; a[j] = t;
        }
    int t = a[0]; a[0] = a[i]; a[i] = t;
    tez(a, i);
    tez(a + i + 1, n - i - 1);
}

static unsigned tasodif = 12345;
static int tas(void)
{
    tasodif = tasodif * 1103515245u + 12345u;
    return (int)(tasodif >> 8) % 100000;
}

static int saralanganmi(const int *a, int n)
{
    for (int i = 1; i < n; i++)
        if (a[i - 1] > a[i])
            return 0;
    return 1;
}

int main(void)
{
    static int asl[N], a[N], t[N];
    for (int i = 0; i < N; i++)
        asl[i] = tas();

    printf("%d ta element\n%-30s %12s %s\n", N, "algoritm / kirish", "taqqoslash", "to'g'ri?");

    memcpy(a, asl, sizeof(a)); taqqos = 0; qoyish(a, N);
    printf("%-30s %12ld %s\n", "qo'yish, tasodifiy", taqqos, saralanganmi(a, N) ? "ha" : "YO'Q");
    memcpy(a, asl, sizeof(a)); taqqos = 0; birlash(a, t, N);
    printf("%-30s %12ld %s\n", "birlashtirish, tasodifiy", taqqos, saralanganmi(a, N) ? "ha" : "YO'Q");
    memcpy(a, asl, sizeof(a)); taqqos = 0; tez(a, N);
    printf("%-30s %12ld %s\n", "tez (pivot=birinchi), tasodifiy", taqqos, saralanganmi(a, N) ? "ha" : "YO'Q");

    taqqos = 0; tez(a, N);                              /* a endi saralangan - quicksort uchun eng yomon kirish */
    printf("%-30s %12ld %s\n", "tez (pivot=birinchi), SARALANGAN", taqqos, saralanganmi(a, N) ? "ha" : "YO'Q");
    taqqos = 0; birlash(a, t, N);
    printf("%-30s %12ld %s\n", "birlashtirish, saralangan", taqqos, saralanganmi(a, N) ? "ha" : "YO'Q");
    taqqos = 0; qoyish(a, N);
    printf("%-30s %12ld %s\n", "qo'yish, saralangan", taqqos, saralanganmi(a, N) ? "ha" : "YO'Q");
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 saralash.c -o saralash
$ ./saralash
2000 ta element
algoritm / kirish                taqqoslash to'g'ri?
qo'yish, tasodifiy                  1009561 ha
birlashtirish, tasodifiy              19402 ha
tez (pivot=birinchi), tasodifiy        24425 ha
tez (pivot=birinchi), SARALANGAN      1999000 ha
birlashtirish, saralangan             10864 ha
qo'yish, saralangan                    1999 ha
```

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `kichik(a, b)` | taqqoslash + hisoblagich: har taqqoslash `taqqos++` qiladi |
| `qoyish` | `x = a[i]` ni chapga siljitib, o'rniga qo'yadi |
| `birlash` | yarmini rekursiv saralab, `t[]` ga birlashtirib, qaytib nusxalaydi |
| `tez` | birinchi element tayanch; kichiklarni chapga yig'ib, tayanchni o'rtaga qo'yadi, ikki tomonni rekursiv saralaydi |
| `tas()` | oddiy tasodifiy son generatori (natija har gal bir xil chiqishi uchun) |

**Nima ko'rdik** (n = 2000):

| Kirish | Qo'yish | Birlashtirish | Tez (pivot = birinchi) |
|---|---|---|---|
| tasodifiy | ≈ 1 000 000 (n²/4) | ≈ 19 000 (n log n) | ≈ 24 000 (n log n) |
| **saralangan** | 1 999 (juda tez!) | ≈ 11 000 | **1 999 000** (n²/2 — falokat!) |

- Qo'yish saralash tasodifiy kirishda ~50 marta ko'p ish qiladi, lekin **saralangan** kirishda eng tez (faqat n−1 taqqoslash).
- Quicksort tasodifiy kirishda juda tez, lekin **allaqachon saralangan** massivda birinchi elementni tayanch qilsa — O(n²)! Sabab: tayanch eng kichik element bo'lib chiqadi → bo'laklash 1 va n−1 ga bo'linadi.
- Birlashtirish saralash kirishga befarq: doim ≈ n log n.

Yechim: tayanchni o'rtadan/tasodifiy/"uchtaning medianasi" qilib tanlash. Standart kutubxonalar gibrid: **introsort** (quick → chuqurlik oshsa heap → kichik qismlarda insertion).

**Yadroda nega heapsort:** eng yomon holat kafolati, qo'shimcha xotira yo'q, rekursiya yo'q (yadro steki kichik!).

> **Eslab qoling:** ishonchli tez = n log n. Quicksort — odatda tez, lekin yomon tayanchda O(n²); merge/heap — doim n log n; qo'yish — kichik yoki deyarli saralangan kirishda eng yaxshi.

## 28.6. Daraxtlar

### Ikkilik qidiruv daraxti (BST)

**Oddiy qilib aytganda:** har tugunda qoida: **chap tomondagilar kichik, o'ng tomondagilar katta.** Qidirishda har tugunda bir tomonga ketasiz → O(daraxt balandligi).

**Hayotdan misol:** tashkilot tuzilmasi yoki oila shajarasi (direktor → bo'lim boshliqlari → xodimlar), lekin har bir tugunda "kichiklar chapda, kattalar o'ngda".

**Muammo:** agar kalitlar **tartib bilan** qo'shilsa (1, 2, 3, ...), daraxt bir tomonga cho'zilib **ro'yxatga aylanadi** — balandlik n, qidirish O(n).

**Bu dastur nima qiladi (umumiy):** 1000 ta kalitni BST'ga (a) 0, 1, 2, ... tartibida, (b) aralashtirib kiritadi va ikkala daraxtning **balandligini** o'lchaydi. Ideal balandlik bilan solishtiradi.

```c
/* bst_balans.c - ikkilik qidiruv daraxti balandligi: tartibli va aralash kiritish */
#include <stdio.h>
#include <stdlib.h>

struct tugun {
    int kalit;
    struct tugun *chap, *ong;
};

static struct tugun *qosh(struct tugun *t, int k)
{
    if (!t) {
        t = calloc(1, sizeof(*t));
        t->kalit = k;
    } else if (k < t->kalit) {
        t->chap = qosh(t->chap, k);
    } else {
        t->ong = qosh(t->ong, k);
    }
    return t;
}

static int balandlik(const struct tugun *t)
{
    if (!t)
        return 0;
    int c = balandlik(t->chap), o = balandlik(t->ong);
    return 1 + (c > o ? c : o);
}

static void ochir(struct tugun *t)
{
    if (t) {
        ochir(t->chap);
        ochir(t->ong);
        free(t);
    }
}

int main(void)
{
    enum { N = 1000 };
    static int kalit[N];

    struct tugun *a = NULL;
    for (int i = 0; i < N; i++)
        a = qosh(a, i);                                 /* 0, 1, 2, ... tartibida */
    printf("%d ta kalit TARTIB bilan kiritildi: balandlik = %d  (daraxt ro'yxatga aylandi)\n", N, balandlik(a));
    ochir(a);

    for (int i = 0; i < N; i++)
        kalit[i] = i;
    unsigned s = 7;
    for (int i = N - 1; i > 0; i--) {                   /* aralashtirish (Fisher-Yates) */
        s = s * 1103515245u + 12345u;
        int j = (int)((s >> 8) % (unsigned)(i + 1));
        int t = kalit[i]; kalit[i] = kalit[j]; kalit[j] = t;
    }
    struct tugun *b = NULL;
    for (int i = 0; i < N; i++)
        b = qosh(b, kalit[i]);
    printf("%d ta kalit ARALASH kiritildi: balandlik = %d\n", N, balandlik(b));
    printf("ideal muvozanatli daraxt balandligi = 10  (2^10 = 1024 >= %d)\n", N);
    ochir(b);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 bst_balans.c -o bst_balans
$ ./bst_balans
1000 ta kalit TARTIB bilan kiritildi: balandlik = 1000  (daraxt ro'yxatga aylandi)
1000 ta kalit ARALASH kiritildi: balandlik = 22
ideal muvozanatli daraxt balandligi = 10  (2^10 = 1024 >= 1000)
```

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `qosh(t, k)` | rekursiv: bo'sh joyga yetguncha kichik bo'lsa chapga, aks holda o'ngga tushadi |
| `balandlik(t)` | `1 + max(chap balandligi, o'ng balandligi)` |
| aralashtirish (Fisher–Yates) | kalitlar tartibini tasodifiy qiladi |
| `ochir` | daraxtni tubidan `free` qiladi (xotira oqmasin) |

**Nima ko'rdik:** tartibli kiritishda balandlik = 1000 (daraxt ro'yxat), aralash kiritishda — 22. Ideal muvozanatli daraxtda 1000 element uchun balandlik 10 bo'lardi. Aralash kiritish "taxminan yaxshi" (22), lekin ishonchli emas.

### Muvozanatli daraxtlar

Balandlikni **kafolatli** O(log n) da ushlab turish uchun qo'shish/o'chirishdan keyin **aylantirishlar** (rotations) qilinadi:

```text
     y                x
    / \   o'ngga     / \
   x   C  ------>   A   y
  / \     <------      / \
 A   B    chapga      B   C
```

Aylantirish tartibni (A < x < B < y < C) saqlaydi, lekin balandliklarni o'zgartiradi.

| Daraxt | G'oya | Qayerda |
|---|---|---|
| **AVL** | har tugunda chap va o'ng balandliklar farqi ≤ 1; qat'iy muvozanat — qidirish tez | 47-mashqda yozasiz |
| **Qizil-qora** | har tugun qizil yoki qora; qoidalar eng uzun yo'l eng qisqasidan 2 barobardan oshmasligini kafolatlaydi; kamroq aylantirish | **Linux'da eng ko'p ishlatiladigan daraxt** (`lib/rbtree.c`): CFS scheduler, hrtimer, ext4 ekstentlar keshi; VMA uchun 6.1 dan boshlab "maple tree" |
| **B-daraxt** | tugunda ko'p kalit (masalan, 100 ta); balandlik juda kichik | disk uchun ideal (tugun = disk bloki): ma'lumotlar bazalari, btrfs, XFS, ext4 papka indeksi |

### Heap (uyum) va ustuvorlik navbati

**Oddiy qilib aytganda:** **eng kichik** (yoki eng katta) elementni tez olish uchun tuzilma. **Min-heap**: har ota o'z bolalaridan kichik → eng kichik element doim tepada (`a[0]`).

Heap — to'liq ikkilik daraxt, lekin **massivda** saqlanadi, ko'rsatkichsiz. `i`-element uchun: ota `(i-1)/2`, chap bola `2i+1`, o'ng bola `2i+2`.

**Bu dastur nima qiladi (umumiy):** 7 elementli massivning har bir elementi uchun ota va bola indekslarini chiqaradi, so'ng massiv min-heap qoidasini bajarishini tekshiradi (va qoidasi buzilgan massivni aniqlaydi).

```c
/* heap_indeks.c - heap massivda: ota/bola indekslari va heap qoidasini tekshirish */
#include <stdio.h>

static int heap_mi(const int *a, int n)                 /* min-heap: har ota bolalaridan kichik */
{
    for (int i = 1; i < n; i++)
        if (a[(i - 1) / 2] > a[i])
            return 0;
    return 1;
}

int main(void)
{
    int h[] = { 1, 3, 2, 7, 4, 5, 9 };
    int n = 7;

    printf("indeks | ota | chap bola (2i+1) | o'ng bola (2i+2) | qiymat\n");
    for (int i = 0; i < n; i++) {
        int ota = i ? (i - 1) / 2 : -1;
        int chap = 2 * i + 1 < n ? 2 * i + 1 : -1;      /* -1: bunday bola yo'q */
        int ong = 2 * i + 2 < n ? 2 * i + 2 : -1;
        printf("  %2d   | %3d | %9d        | %9d        | %d\n", i, ota, chap, ong, h[i]);
    }

    printf("\n[1 3 2 7 4 5 9] min-heap'mi? %s\n", heap_mi(h, n) ? "ha" : "yo'q");
    int buzuq[] = { 1, 3, 2, 7, 0, 5, 9 };              /* 4-indeksdagi 0 otasidan (3) kichik */
    printf("[1 3 2 7 0 5 9] min-heap'mi? %s  (a[4]=0 < otasi a[1]=3)\n", heap_mi(buzuq, 7) ? "ha" : "yo'q");
    printf("eng kichik element doim a[0] = %d\n", h[0]);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 heap_indeks.c -o heap_indeks
$ ./heap_indeks
indeks | ota | chap bola (2i+1) | o'ng bola (2i+2) | qiymat
   0   |  -1 |         1        |         2        | 1
   1   |   0 |         3        |         4        | 3
   2   |   0 |         5        |         6        | 2
   3   |   1 |        -1        |        -1        | 7
   4   |   1 |        -1        |        -1        | 4
   5   |   2 |        -1        |        -1        | 5
   6   |   2 |        -1        |        -1        | 9

[1 3 2 7 4 5 9] min-heap'mi? ha
[1 3 2 7 0 5 9] min-heap'mi? yo'q  (a[4]=0 < otasi a[1]=3)
eng kichik element doim a[0] = 1
```

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `(i - 1) / 2` | `i` ning otasi |
| `2 * i + 1`, `2 * i + 2` | chap/o'ng bola; `n` dan katta bo'lsa — bola yo'q (`-1` deb ko'rsatildi) |
| `heap_mi` | har `i ≥ 1` uchun `a[ota] <= a[i]` ekanini tekshiradi |

**Nima ko'rdik:** `[1 3 2 7 4 5 9]` — daraxtga qo'ysak: tepada 1; uning bolalari 3 va 2; 3 ning bolalari 7 va 4; 2 ning bolalari 5 va 9. Har ota bolalaridan kichik → heap. `a[4]=0` qo'yilsa, otasi (`a[1]=3`) dan kichik → qoida buzildi.

`push` — oxiriga qo'yib "yuqoriga suzdirish"; `pop` — oxirgisini tepaga qo'yib "pastga cho'ktirish". Ikkalasi O(log n) (46-mashqda o'zingiz yozasiz). Yadroda: taymerlar (eng yaqin muddatli), scheduler'lar, Dijkstra algoritmi.

### Trie va radix daraxt

Kalitni **qismlarga** (belgilar, bitlar guruhlari) bo'lib, daraxt bo'ylab tushish. Kalit uzunligi bo'yicha O(k), taqqoslashsiz. **Sahifa jadvali — aslida radix daraxt!** (9 bitlik 4 daraja, 31-mashq; 24-bob.) Linux'da `xarray` (sahifa keshi: fayldagi siljish → sahifa), IP marshrutlash jadvallari (eng uzun prefiks).

> **Eslab qoling:** BST oddiy, lekin tartibli kiritishda ro'yxatga aylanadi → muvozanatli daraxtlar (AVL, qizil-qora). Disk uchun — B-daraxt. Eng kichikni olish uchun — heap (`2i+1`, `2i+2`). Kalitni qismlab tushish — trie/radix (sahifa jadvali).

## 28.7. Graflar

**Oddiy qilib aytganda:** graf — **tugunlar** (nuqtalar) va **qirralar** (ularni bog'lovchi chiziqlar). **Hayotdan misol: metro xaritasi** — bekatlar tugunlar, ular orasidagi yo'llar qirralar. Savollar: "A dan B ga eng kam bekat bilan qanday yetaman?", "hamma bekat bir-biriga bog'langanmi?".

**Tasvirlash:** **qo'shnilik ro'yxati** (har tugun uchun qo'shnilar ro'yxati — siyrak graflar uchun, odatda shu) yoki **qo'shnilik matritsasi** (n×n jadval, `yol[a][b] = 1`: zich graflar uchun, tekshirish O(1)).

**Yurish usullari:**

| Usul | Qanday | Nima uchun |
|---|---|---|
| **BFS** (kenglik bo'yicha) | navbat bilan: avval 1 qadam masofadagilar, keyin 2... (suvga tashlangan tosh to'lqinlari kabi) | **eng qisqa yo'l** (vaznsiz graflarda) |
| **DFS** (chuqurlik bo'yicha) | stek/rekursiya bilan: bir yo'l bilan oxirigacha, keyin qaytish | sikl topish, topologik tartib |

### Topologik tartib — bog'liqliklarni hisobga olib tartiblash

**Oddiy qilib aytganda:** ba'zi ishlar boshqalardan **keyin** bo'lishi kerak: `dastur` fayli `main.o` va `kutubxona.a` dan keyin yig'iladi; `main.o` esa `mat.h` dan keyin. Qaysi tartibda yig'ish kerak? `make` (11-bob) aynan shuni hal qiladi. Yadroda — modullarni ishga tushirish tartibi.

**Bu dastur nima qiladi (umumiy):** 6 ta tugun (fayl) va ular orasidagi "avval–keyin" qirralarni oladi va **Kahn algoritmi** bilan to'g'ri yig'ish tartibini topadi. Algoritm: hech narsani kutmaydigan (kirish qirrasi 0) tugunlarni navbatga qo'yadi, ularni "bajarib", ularga bog'liq tugunlarning kutish sonini kamaytiradi.

```c
/* topologik.c - topologik tartib (Kahn algoritmi): qaysi faylni avval yig'ish kerak */
#include <stdio.h>

#define N 6

static const char *nom[N] = { "main.o", "mat.o", "satr.o", "kutubxona.a", "dastur", "mat.h" };
/* (a, b): "a avval tayyor bo'lishi kerak, keyin b" */
static const int qirra[][2] = {
    { 5, 0 }, { 5, 1 },                                 /* mat.h -> main.o, mat.o */
    { 1, 3 }, { 2, 3 },                                 /* mat.o, satr.o -> kutubxona.a */
    { 0, 4 }, { 3, 4 },                                 /* main.o, kutubxona.a -> dastur */
};
#define Q (int)(sizeof(qirra) / sizeof(qirra[0]))

int main(void)
{
    int kirish[N] = { 0 }, navbat[N], bosh = 0, oxir = 0;
    for (int i = 0; i < Q; i++)
        kirish[qirra[i][1]]++;                          /* nechta shart bu tugunni kutyapti */

    for (int v = 0; v < N; v++)
        if (kirish[v] == 0)
            navbat[oxir++] = v;                         /* hech narsani kutmaydiganlar */

    printf("yig'ish tartibi:\n");
    while (bosh < oxir) {
        int u = navbat[bosh++];
        printf("  %d) %s\n", bosh, nom[u]);
        for (int i = 0; i < Q; i++)
            if (qirra[i][0] == u && --kirish[qirra[i][1]] == 0)
                navbat[oxir++] = qirra[i][1];           /* barcha shartlari bajarildi */
    }
    printf("%s\n", oxir == N ? "hammasi tartiblandi (sikl yo'q)" : "SIKL bor: hammasini tartiblab bo'lmadi");
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 topologik.c -o topologik
$ ./topologik
yig'ish tartibi:
  1) satr.o
  2) mat.h
  3) main.o
  4) mat.o
  5) kutubxona.a
  6) dastur
hammasi tartiblandi (sikl yo'q)
```

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `qirra[][2]` | (a, b) juftliklari: "a avval, b keyin" |
| `kirish[v]` | `v` yana nechta shartni kutyapti |
| `navbat` | tayyor (hech narsa kutmayotgan) tugunlar |
| `--kirish[...] == 0` | oxirgi sharti bajarilgan tugun navbatga qo'shiladi |
| `oxir == N` | hamma tugun tartiblandimi? Agar yo'q — graf'da **sikl** bor |

**Nima ko'rdik:** `mat.h` `main.o` va `mat.o` dan oldin, ular `kutubxona.a` va `dastur` dan oldin keldi; `satr.o` hech narsani kutmagani uchun birinchi chiqdi. Har qadam tartibi `make` qiladigan tartibga mos.
Agar A B ni, B A ni kutsa (sikl) — hech biri hech qachon tayyor bo'lmaydi va `oxir < N` bo'lib qoladi.

### Sikl topish va deadlock

**Sikl topish (DFS, uch rang):** har tugun — oq (ko'rilmagan), kulrang (hozir yurilayotgan yo'lda), qora (tugagan). Yurish paytida **kulrang** tugunga qaytuvchi qirra uchrasa — bu **sikl**.

Bu — deadlock aniqlashning asosi: "kutish grafi"da (A → B: "A, B ushlab turgan resursni kutyapti") sikl = deadlock (26-bob). Linux `lockdep` qulflar tartibi grafida aynan shuni qiladi. 48-mashqda siz shuni yozasiz.

### Metro: BFS bilan eng qisqa yo'l

**Bu dastur nima qiladi (umumiy):** metro xaritasi graf sifatida beriladi; BFS bilan ikki bekat orasidagi **eng kam bekatli yo'l** topiladi (pastda "Hayotdan misol va to'liq dastur" bo'limida tahlili).

> **Eslab qoling:** graf = tugunlar + qirralar. **BFS** — eng qisqa yo'l (navbat); **DFS** — sikl, topologik tartib (stek). Sikl = deadlock; topologik tartib = `make`.

## 28.8. Bitli hiylalar va maxsus tuzilmalar

- **Bitmap** (21-mashq) — to'plam elementlari uchun 1 bit (blok band/bo'sh).
- **Bloom filtri** — "albatta yo'q" yoki "ehtimol bor" deydigan ixcham to'plam.
- **Halqa bufer** (22-mashq) — qulfsiz ham qilish mumkin (bitta yozuvchi + bitta o'quvchi).
- **Buddy** (32), **slab** (33) — xotira allocator'lari (25-bob) o'zi ham algoritmlar.

## 28.9. Usullar (masala yechish naqshlari)

Quyidagi dastur ikkita eng sodda va foydali usulni ko'rsatadi.

**Bu dastur nima qiladi (umumiy):** (1) **ikki ko'rsatkich**: saralangan massivda yig'indisi 20 bo'lgan juftni topadi; (2) **siljuvchi oyna**: massivda ketma-ket 3 sonning eng katta yig'indisini topadi.

```c
/* ikki_korsatkich.c - ikki ko'rsatkich va siljuvchi oyna usullari */
#include <stdio.h>

int main(void)
{
    /* 1) ikki ko'rsatkich: saralangan massivda yig'indisi 20 bo'lgan juft */
    int a[] = { 1, 3, 4, 6, 8, 11, 14, 17 };
    int n = 8, hedef = 20, chap = 0, ong = n - 1, qadam = 0;
    while (chap < ong) {
        qadam++;
        int yig = a[chap] + a[ong];
        if (yig == hedef) {
            printf("juft topildi: a[%d]=%d + a[%d]=%d = %d  (%d qadamda)\n", chap, a[chap], ong, a[ong], yig, qadam);
            break;
        }
        if (yig < hedef)
            chap++;                                     /* yig'indi kichik - kattaroq chap kerak */
        else
            ong--;                                      /* katta - kichikroq o'ng kerak */
    }

    /* 2) siljuvchi oyna: ketma-ket 3 ta sonning eng katta yig'indisi */
    int b[] = { 2, 1, 5, 1, 3, 2, 9, 1, 4 };
    int m = 9, k = 3, oyna = 0, eng = 0, eng_bosh = 0;
    for (int i = 0; i < k; i++)
        oyna += b[i];
    eng = oyna;
    for (int i = k; i < m; i++) {
        oyna += b[i] - b[i - k];                        /* yangisini qo'shib, eskisini olib tashlaymiz: O(1) */
        if (oyna > eng) {
            eng = oyna;
            eng_bosh = i - k + 1;
        }
    }
    printf("eng katta %d ketma-ket yig'indi: %d (b[%d..%d])\n", k, eng, eng_bosh, eng_bosh + k - 1);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 ikki_korsatkich.c -o ikki_korsatkich
$ ./ikki_korsatkich
juft topildi: a[1]=3 + a[7]=17 = 20  (2 qadamda)
eng katta 3 ketma-ket yig'indi: 14 (b[4..6])
```

**Qismlar:**

| Usul | G'oya | Murakkablik |
|---|---|---|
| **Ikki ko'rsatkich** | `chap` boshdan, `ong` oxirdan; yig'indi kichik bo'lsa `chap++`, katta bo'lsa `ong--` | O(n) (sodda usul: ichma-ich sikl O(n²)) |
| **Siljuvchi oyna** | yig'indini har safar qayta hisoblamaymiz: yangi element qo'shib, eskisini olib tashlaymiz | O(n) (qayta hisoblasak: O(n·k)) |

**Nima ko'rdik:** juft 2 qadamda topildi (`3 + 17`); eng katta 3 ketma-ket yig'indi 14 (`3 + 2 + 9`, `b[4..6]`).

Boshqa usullar: **bo'l va hukmronlik qil** (merge sort, ikkilik qidiruv); **dinamik dasturlash** (takrorlanadigan qism masalalar natijasini saqlash; yadroda kam, suhbatlarda ko'p); **ochko'z** (har qadamda eng yaxshi mahalliy tanlov; SJF scheduler — 23-bob).

## 28.10. Suhbatlarga tayyorgarlik (keyinchalik)

Katta kompaniyalar tizim dasturchisini ham algoritm masalalari bilan tekshiradi. Usul:

1. **Masalani o'z so'zingiz bilan qayta ayting**, misollar va chegaraviy holatlarni so'rang (bo'sh kirish, bitta element, takrorlar, toshish).
2. Avval **oddiy (sekin) yechimni** ayting, murakkabligini baholang.
3. Keyin yaxshilang: qaysi tuzilma yordam beradi? (Qidiruv ko'p — xesh; tartib kerak — daraxt/heap; "eng yaqin"/"eng kichik" — heap; yo'llar — BFS/DFS.)
4. Kodni yozing, keyin **qo'lda misol bilan yurib chiqing**.

Tayyorgarlik uchun: ushbu bobdagi hamma tuzilmani C'da noldan yozing (mashqlar 13, 16, 17, 19, 21, 22, 42, 46, 47, 48) — keyin masalalar to'plamlaridan (ko'pchiligi ingliz tilida, lekin masala shartlari qisqa — 31-bob lug'ati yordam beradi) kuniga 1–2 ta.

## Hayotdan misol va to'liq dastur

**Toshkent metrosi.** Metro tarmog'ining bir qismi (soddalashtirilgan): bekatlar — graf tugunlari, yo'llar — qirralar. BFS bilan eng kam bekatli yo'lni topamiz.

**Bu dastur nima qiladi (umumiy):** 10 bekatli xarita yaratadi (`ula` bilan yo'llar qo'shadi), so'ng uch juft bekat orasidagi **eng kam bekatli yo'lni** BFS bilan topib chiqaradi. Dastur ikki qismdan iborat: BFS (yo'lni qidiradi va har bekatning "otasi"ni yozadi) va yo'lni oxiridan boshiga qarab tiklash.

```c
/* metro.c - graf, navbat va BFS: eng qisqa yo'l (bekatlar soni bo'yicha) */
#include <stdio.h>
#include <string.h>

#define B 10
static const char *bekat[B] = {
    "Chilonzor", "Novza", "Mustaqillik maydoni", "Amir Temur xiyoboni", "Paxtakor",
    "Alisher Navoiy", "Toshkent", "Oybek", "Kosmonavtlar", "Yunus Rajabiy",
};
static int yol[B][B];                           /* qo'shnilik matritsasi: 1 - to'g'ridan-to'g'ri yo'l */

static void ula(int a, int b) { yol[a][b] = yol[b][a] = 1; }

static void eng_qisqa(int dan, int ga)
{
    int oldingi[B], navbat[B], bosh = 0, oxir = 0;
    memset(oldingi, -1, sizeof(oldingi));
    oldingi[dan] = dan;
    navbat[oxir++] = dan;
    while (bosh < oxir) {                       /* navbat bo'sh bo'lmaguncha */
        int u = navbat[bosh++];
        for (int v = 0; v < B; v++)
            if (yol[u][v] && oldingi[v] == -1) {
                oldingi[v] = u;                 /* v ga u orqali keldik */
                navbat[oxir++] = v;
            }
    }

    int yol_teskari[B], n = 0;                  /* oxiridan boshiga qarab tiklash */
    for (int v = ga; v != dan; v = oldingi[v])
        yol_teskari[n++] = v;
    yol_teskari[n++] = dan;
    printf("%s -> %s: %d bekat\n  ", bekat[dan], bekat[ga], n - 1);
    for (int i = n - 1; i >= 0; i--)
        printf("%s%s", bekat[yol_teskari[i]], i ? " -> " : "\n");
}

int main(void)
{
    ula(0, 1);                                  /* Chilonzor liniyasi */
    ula(1, 2);
    ula(2, 3);
    ula(3, 4);
    ula(4, 5);                                  /* Paxtakor <-> Alisher Navoiy (o'tish) */
    ula(5, 7);                                  /* Alisher Navoiy -> Oybek */
    ula(3, 9);                                  /* Amir Temur <-> Yunus Rajabiy (o'tish) */
    ula(7, 8);                                  /* Oybek -> Kosmonavtlar */
    ula(8, 6);                                  /* Kosmonavtlar -> Toshkent */
    ula(9, 6);                                  /* Yunus Rajabiy -> Toshkent */

    eng_qisqa(0, 6);                            /* Chilonzor -> Toshkent vokzali */
    eng_qisqa(0, 7);
    eng_qisqa(4, 9);
    return 0;
}
```

```console
$ gcc -Wall -Wextra metro.c -o metro
$ ./metro
Chilonzor -> Toshkent: 5 bekat
  Chilonzor -> Novza -> Mustaqillik maydoni -> Amir Temur xiyoboni -> Yunus Rajabiy -> Toshkent
Chilonzor -> Oybek: 6 bekat
  Chilonzor -> Novza -> Mustaqillik maydoni -> Amir Temur xiyoboni -> Paxtakor -> Alisher Navoiy -> Oybek
Paxtakor -> Yunus Rajabiy: 2 bekat
  Paxtakor -> Amir Temur xiyoboni -> Yunus Rajabiy
```

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `bekat[B]` | bekat nomlari (tugun raqami = massiv indeksi) |
| `yol[B][B]` | **qo'shnilik matritsasi**: `yol[a][b] = 1` — to'g'ridan-to'g'ri yo'l bor |
| `ula(a, b)` | ikki tomonlama yo'l qo'shadi |
| `oldingi[v]` | `v` ga qaysi bekatdan kelganimiz (`-1` — hali ko'rilmagan) |
| `navbat[]`, `bosh`, `oxir` | BFS navbati |
| `if (yol[u][v] && oldingi[v] == -1)` | `u` ning ko'rilmagan qo'shnisi bo'lsa — uni belgilab navbatga qo'shamiz |
| `for (v = ga; v != dan; v = oldingi[v])` | yo'lni oxiridan boshiga qarab tiklash (`oldingi` zanjiri bo'ylab) |

BFS Chilonzordan Toshkentgacha ikki yo'lni ko'rdi: Yunus Rajabiy orqali va Oybek orqali. Birinchisi qisqaroq bo'lgani uchun u tanlandi — BFS bekatlarni **masofasi bo'yicha qatlam-qatlam** ochadi, shuning uchun birinchi topilgan yo'l eng qisqa.

**Sinab ko'ring:** `ula(9, 6);` ni o'chiring — endi yo'l qaysi bekatlar orqali o'tadi? Bekatlar soni `B` ni 1000 ga oshirsak, qo'shnilik matritsasi necha bayt egallaydi? Nega katta graflarda qo'shnilar ro'yxati ishlatiladi?

## Bob xulosasi (yodlash uchun)

1. **O-belgi:** n ko'paysa ish qanday o'sadi. O(1) ≪ O(log n) ≪ O(n) ≪ O(n log n) ≪ O(n²). Yadroda sekin algoritm butun tizimni to'xtatadi.
2. **Tuzilmalar:** massiv (tez kirish), ro'yxat (tez qo'shish), stek (LIFO), navbat (FIFO, halqa bufer), **xesh jadval** (O(1) o'rtacha; yuklama oshsa kengaytiriladi).
3. **Ikkilik qidiruv:** saralangan ma'lumotda O(log n); o'rtani `chap + (ong - chap) / 2` deb hisoblang. **Saralash:** n log n (merge/heap kafolatli, quick odatda tez), kichik kirishda qo'yish.
4. **Daraxtlar:** BST tartibli kiritishda ro'yxatga aylanadi → AVL/qizil-qora (Linux), B-daraxt (disk); **heap** — eng kichik doim tepada (`2i+1`, `2i+2`); trie/radix — sahifa jadvali.
5. **Graflar:** BFS (navbat) — eng qisqa yo'l; DFS (stek) — sikl (deadlock) va topologik tartib (`make`). Masala yechish: ikki ko'rsatkich, siljuvchi oyna, bo'l va hukmronlik qil.

## Savol-javob

**Savol:** Nega kichik n da massiv ro'yxatdan yaxshiroq, nazariyada ikkalasi teng bo'lsa ham?
**Javob:** Massiv elementlari xotirada yonma-yon, protsessor keshi ularni birga oladi (21-bob). Ro'yxat tugunlari tarqoq — har o'tish kesh xatosi bo'lishi mumkin. O-belgi doimiy ko'paytuvchilarni yashiradi, kesh esa ularni sezilarli qiladi.

**Savol:** Xesh jadval o'rniga har doim saralangan massiv + ikkilik qidiruv ishlatsam bo'ladimi?
**Javob:** Qidirishda O(log n) kamroq tez, qo'shishda esa massivni siljitish O(n). Xesh jadval ikkalasida o'rtacha O(1), lekin tartibni (masalan, "eng kichigi") saqlamaydi — shuning uchun tartib kerak bo'lsa daraxt/heap.

**Savol:** Nega Linux'da ham xesh jadval, ham daraxt bor?
**Javob:** Xesh — "aniq kalit bo'yicha tez topish" (PID, inode). Daraxt — "tartib kerak" (eng yaqin taymer, adres oralig'i). Masalaga mos tuzilma tanlanadi.

## O'zingizni tekshiring

1. Dinamik massivda `push` nega amortizatsiyalangan O(1)?
2. `(chap + ong) / 2` nima uchun xavfli?
3. Nega Linux'da heapsort, quicksort emas?
4. Sahifa jadvali qaysi tuzilma turiga kiradi?
5. Deadlock'ni graf yordamida qanday topasiz?

<details><summary>Javoblar</summary>

1. Hajm 2 barobar oshadi: n ta push uchun jami nusxalash 1+2+4+...+n < 2n — har biriga o'rtacha O(1).
2. Katta indekslarda yig'indi toshadi; `chap + (ong - chap) / 2` xavfsiz.
3. Eng yomon holat O(n log n) kafolati, qo'shimcha xotira va rekursiya yo'q (kichik yadro steki).
4. Radix daraxt (trie): manzil bitlari guruhlari bo'yicha tushish.
5. Kutish grafini qurib, DFS bilan sikl qidirish (kulrang tugunga qaytish).
</details>

## Mashq

- **42** (LRU kesh: xesh + ikki tomonlama ro'yxat), **46** (min-heap), **47** (AVL daraxt), **48** (deadlock: grafda sikl topish).
- Takrorlash: **13, 16, 17, 19, 21, 22, 32**.

<!-- loyiha:boshi -->
## Loyiha: so'z chastotasi (xesh jadval)

**Maqsad:** xesh jadvalni noldan qurish va u **nega tez** ekanini ko'rish: kalit → xesh → "cho'ntak" (bucket) → qisqa zanjir. Python'dagi `dict` va
`collections.Counter` ning ichi aynan shunday (28.3).
**Bobdan ishlatiladi:** xesh funksiya, zanjirlash (chaining), bog'langan ro'yxat, `qsort` bilan saralash.

**Talab:** matndagi har so'z necha marta uchrashini sanang va eng ko'p uchraydigan 5 tasini chiqaring; xesh jadval statistikasini
(nechta cho'ntak band, eng uzun zanjir) ham ko'rsating.
**Ma'lumotlar:** `struct tugun { char *soz; int soni; struct tugun *keyingi; }`; `jadval[16]` — zanjirlar boshlari.
**Xesh:** `djb2`: `h = 5381; har belgi uchun h = h*33 + belgi`. Cho'ntak = `h % 16`.

```c
/* chastota.c - so'z chastotasi xesh jadval bilan */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KOVAKLAR 16

struct tugun {
    char *soz;
    int soni;
    struct tugun *keyingi;
};
static struct tugun *jadval[KOVAKLAR];
static int noyob;

static unsigned xesh(const char *s)
{
    unsigned h = 5381;
    while (*s)
        h = h * 33 + (unsigned char)*s++;
    return h;
}

static void qosh(const char *soz)
{
    unsigned k = xesh(soz) % KOVAKLAR;
    for (struct tugun *t = jadval[k]; t; t = t->keyingi)
        if (strcmp(t->soz, soz) == 0) {         /* zanjirda bor: sanagichni oshiramiz */
            t->soni++;
            return;
        }
    struct tugun *yangi = malloc(sizeof(*yangi));
    yangi->soz = strdup(soz);
    yangi->soni = 1;
    yangi->keyingi = jadval[k];                 /* zanjir boshiga qo'yamiz */
    jadval[k] = yangi;
    noyob++;
}

static int taqqosla(const void *pa, const void *pb)
{
    const struct tugun *a = *(struct tugun *const *)pa, *b = *(struct tugun *const *)pb;
    if (a->soni != b->soni)
        return b->soni - a->soni;               /* ko'pi oldin */
    return strcmp(a->soz, b->soz);              /* teng bo'lsa alifbo */
}

int main(void)
{
    const char *matn = "Yadro dasturchisi yadro bilan ishlaydi. Yadro xotirani boshqaradi, yadro jarayonlarni "
                       "rejalashtiradi. Dasturchi xotirani tushunishi kerak, jarayon esa xotirani ishlatadi.";
    char soz[32];
    int n = 0;
    for (const char *p = matn;; p++) {
        if (isalpha((unsigned char)*p) && n < 31) {
            soz[n++] = (char)tolower((unsigned char)*p);
        } else {
            if (n > 0) {
                soz[n] = '\0';
                qosh(soz);
                n = 0;
            }
            if (*p == '\0')
                break;
        }
    }

    struct tugun **hammasi = malloc((size_t)noyob * sizeof(*hammasi));
    int k = 0, band = 0, eng_uzun = 0;
    for (int i = 0; i < KOVAKLAR; i++) {
        int uz = 0;
        for (struct tugun *t = jadval[i]; t; t = t->keyingi) {
            hammasi[k++] = t;
            uz++;
        }
        band += uz > 0;
        if (uz > eng_uzun)
            eng_uzun = uz;
    }
    qsort(hammasi, (size_t)noyob, sizeof(*hammasi), taqqosla);

    printf("Noyob so'zlar: %d\n", noyob);
    printf("Eng ko'p uchraganlar:\n");
    for (int i = 0; i < 5 && i < noyob; i++)
        printf("  %-14s %d\n", hammasi[i]->soz, hammasi[i]->soni);
    printf("Xesh jadval: %d/%d cho'ntak band, eng uzun zanjir %d\n", band, KOVAKLAR, eng_uzun);

    for (int i = 0; i < KOVAKLAR; i++)
        for (struct tugun *t = jadval[i], *keyingi; t; t = keyingi) {
            keyingi = t->keyingi;
            free(t->soz);
            free(t);
        }
    free(hammasi);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address,undefined chastota.c -o chastota
$ ./chastota
Noyob so'zlar: 14
Eng ko'p uchraganlar:
  yadro          4
  xotirani       3
  bilan          1
  boshqaradi     1
  dasturchi      1
Xesh jadval: 8/16 cho'ntak band, eng uzun zanjir 3
```

Qidirish `O(zanjir uzunligi)` — jadval to'lmasa, `O(1)`. Cho'ntaklar soni oshsa zanjirlar qisqaradi; shuning uchun real xesh jadval (Python `dict`, Linux `hlist`)
to'lish darajasi oshganda **o'zi kengayadi** (rehash). `KOVAKLAR` ni 2 va 256 qiling — "eng uzun zanjir" qanday o'zgaradi?

**Kengaytiring:** `qidir(soz)` funksiyasi va `ochir(soz)` yozing. Bog'langan ro'yxat o'rniga *ochiq manzillash* (bo'sh cho'ntakni qidirish) ni sinab ko'ring.

## Mustaqil loyiha: labirintdan chiqish yo'li (BFS) ★★★

**Vazifa:** labirintda `S` (boshlanish) dan `E` (chiqish) gacha **eng qisqa yo'l**ni toping va uni `*` bilan belgilab ko'rsating.
Bu — graflarda **kenglik bo'yicha qidirish** (BFS, 28.7). Fayl: `labirint.c`.

**Kirish** (`stdin`): bir yoki bir nechta labirint; ular **bo'sh qator** bilan ajratilgan. Belgilar: `#` — devor, `.` — yo'l, `S`, `E`.
Har labirintda qatorlar bir xil uzunlikda, o'lcham 60×60 dan oshmaydi.

**Kirish fayli** (`darslik/loyihalar/28_labirint/kirish.txt`):

```text
###########
#S....#...#
#.###.#.#.#
#.#...#.#.#
#.#.###.#.#
#.#.....#E#
###########

#######
#S#...#
#.#.#.#
###.#E#
#######

#####
#S..#
#...#
#..E#
#####
```

**Qoidalar:**
- Harakat — faqat **to'rt tomonga** (diagonal yo'q), har qadam 1.
- **Tanlov tartibi aniq:** qo'shnilarni har doim **yuqori, o'ng, past, chap** tartibida ko'ring (BFS navbatida shu tartibda qo'shing).
  Bir necha teng qisqa yo'l bo'lsa, natija shu tartibga bog'liq. Katak birinchi marta ko'rilganda uning "otasi" yoziladi (keyin o'zgarmaydi).
- Natija: `Labirint K: eng qisqa yo'l N qadam` va labirintning o'zi, yo'l katakchalari (`S` va `E` dan tashqari) `*` bilan.
  Yo'l yo'q bo'lsa: `Labirint K: yo'l yo'q` (labirint chiqarilmaydi). Har labirint natijasidan keyin **bitta bo'sh qator**.

**Kutilgan natija** (`./dastur < kirish.txt`) (`darslik/loyihalar/28_labirint/kutilgan.txt`):

```text
Labirint 1: eng qisqa yo'l 24 qadam
###########
#S****#***#
#.###*#*#*#
#.#***#*#*#
#.#*###*#*#
#.#*****#E#
###########

Labirint 2: yo'l yo'q

Labirint 3: eng qisqa yo'l 4 qadam
#####
#S**#
#..*#
#..E#
#####
```

**Maslahat** (yechim emas):
- Navbat — massiv (`navbat[3600]`), `bosh`/`oxir` indekslari (8-bob emas, 5-bobdagi oddiy massiv). Har katak uchun `ota[qator][ustun]` va `masofa`.
- BFS: `S` ni navbatga qo'ying. Sikl: navbat boshidan oling; 4 ta qo'shnini **U, R, D, L** tartibida tekshiring: chegara ichidami, devor emasmi, ko'rilmaganmi?
  Ko'rilmagan bo'lsa — `ota` va `masofa` ni yozib navbatga qo'shing. `E` topilganda to'xtashingiz mumkin.
- Yo'lni `E` dan `ota` zanjiri bo'ylab `S` gacha yuring, har katakni `*` qiling (`S`, `E` ni qoldirib).
- Nega BFS eng qisqa yo'lni **kafolatlaydi**? (Har qadamda faqat bir xil masofadagi kataklar birinchi bo'lib ko'riladi.) DFS bilan bo'lmasdi.
- Qatorlarni `fgets` bilan o'qing, oxiridagi `\\n` ni olib tashlang. Bo'sh qator — yangi labirint boshlanishi (yoki fayl oxiri).

**Tekshirish:**

```bash
D=~/C_loyha/darslik/loyihalar/28_labirint
gcc -Wall -Wextra -g -fsanitize=address,undefined labirint.c -o dastur && ./dastur < $D/kirish.txt | diff - $D/kutilgan.txt && echo "TO'G'RI"
```
<!-- loyiha:oxiri -->

Keyingi bob: [29-bob. Debug va profiling vositalari](29-debug-vositalari.md)
