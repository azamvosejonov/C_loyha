# 25-bob. Dinamik xotira ajratish: `malloc` ichida va yadro allocator'lari

> **Bu bobda nima o'rganasiz:** allocator'ning (xotira ajratuvchining) maqsadlari va cheklovlarini; ichki/tashqi fragmentatsiyani; yashirin va aniq bo'sh ro'yxatlarni; chegara teglarini (boundary tags);
> joylashtirish siyosatlarini (first/next/best fit); ajratilgan ro'yxatlarni; **buddy** va **slab** ni. (CS:APP 9.9 va OSTEP "Free-Space Management".)
> **Oldindan nima kerak:** 7-, 8-, 16-, 24-boblar.   **Vaqt:** 6–8 soat.
> Mashqlar: 13, 30, 32, 33.

> **To'liq ishlaydigan misol:** [misollar/25_malloc_ichi.c](misollar/25_malloc_ichi.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Bu bob nima haqida?

8-bobda `malloc` va `free` ni ishlatdingiz. Endi ularning **ichiga** qaraymiz: `malloc(100)` chaqirilganda qaysi xotira beriladi? `free` qilingan joy qanday qayta ishlatiladi? Nega ba'zan "xotira bor, lekin
`malloc` NULL qaytaradi"? Bu savollarga allocator'ning ishlash tamoyillari javob beradi. Yadro uchun esa bu bilim bevosita ish: unda `malloc` yo'q — allocator'larni **o'zingiz** yozasiz (buddy, slab).

**Hayotdan misol: avtoturargoh qo'riqchisi.** Turargohga har xil mashinalar keladi: kichik, o'rta, avtobus. Qo'riqchi har biriga joy ko'rsatadi (`malloc`) va ketganda joyni bo'sh deb belgilaydi (`free`).
Mashinani joyidan surib bo'lmaydi — dasturdagi ko'rsatkichlar uning manziliga bog'langan.

| Turargohda | Allocator'da |
|---|---|
| qo'riqchi | allocator (`malloc`/`free`) |
| mashina | so'ralgan blok |
| joy raqami | manzil (ko'rsatkich) |
| mashinani surib bo'lmaydi | blokni ko'chirib bo'lmaydi |
| "avtobusga joy yo'q, bo'sh joylar tarqoq" | tashqi fragmentatsiya |

## 25.1. Vazifa va cheklovlar

Allocator katta xotira hududini (heap) oladi va so'rovlarga bo'laklab beradi. Cheklovlari:

- so'rovlar **istalgan tartibda** keladi va ketadi;
- berilgan blokni **ko'chirib bo'lmaydi** (dasturda uning manzili saqlangan);
- javob **darhol** berilishi kerak;
- har bir blok **tekislangan** (16 bayt, x86-64 da).

Ikki maqsad bir-biriga zid: **tezlik** (so'rov/sekund) va **xotira samaradorligi** (isrof kam).

Haqiqiy `malloc` shu cheklovlarni qanday bajaradi — o'z kompyuteringizda ko'ramiz:

```c
/* malloc_ichi.c - haqiqiy malloc: tekislash va ichki fragmentatsiya */
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    size_t sizes[] = { 1, 17, 24, 25, 100, 1000 };
    for (int i = 0; i < 6; i++) {
        void *p = malloc(sizes[i]);
        size_t haqiqiy = malloc_usable_size(p);
        printf("so'raldi %4zu bayt -> foydali joy %4zu bayt (ortiqcha %3zu), 16 ga tekis: %s\n",
               sizes[i], haqiqiy, haqiqiy - sizes[i], (uintptr_t)p % 16 == 0 ? "ha" : "yo'q");
    }

    char *a = malloc(1);
    char *b = malloc(1);
    char *c = malloc(1);
    printf("ketma-ket malloc(1) manzillari orasidagi masofa: %ld va %ld bayt\n", (long)(b - a), (long)(c - b));
    return 0;
}
```

```console
$ gcc -Wall -Wextra malloc_ichi.c -o malloc_ichi
$ ./malloc_ichi
so'raldi    1 bayt -> foydali joy   24 bayt (ortiqcha  23), 16 ga tekis: ha
so'raldi   17 bayt -> foydali joy   24 bayt (ortiqcha   7), 16 ga tekis: ha
so'raldi   24 bayt -> foydali joy   24 bayt (ortiqcha   0), 16 ga tekis: ha
so'raldi   25 bayt -> foydali joy   40 bayt (ortiqcha  15), 16 ga tekis: ha
so'raldi  100 bayt -> foydali joy  104 bayt (ortiqcha   4), 16 ga tekis: ha
so'raldi 1000 bayt -> foydali joy 1000 bayt (ortiqcha   0), 16 ga tekis: ha
ketma-ket malloc(1) manzillari orasidagi masofa: 32 va 32 bayt
```

**Bu dastur nima qiladi (umumiy):** har xil o'lchamda xotira so'raydi va `malloc_usable_size` bilan allocator **haqiqatan qancha** ajratganini so'raydi; manzillar 16 ga tekis ekanini va ketma-ket
kichik so'rovlar orasidagi masofani ko'rsatadi.

**Nima ko'rdik:** `malloc(1)` aslida 24 bayt foydali joy beradi (`ortiqcha 23`) — **ichki fragmentatsiya**: allocator har bir blok uchun minimal o'lchamga ega (sarlavha + tekislash). Ketma-ket `malloc(1)`
lar orasidagi masofa — 32 bayt (blok sarlavhasi + yuk + tekislash). Hamma manzil 16 ga tekis. Bu — keyingi bo'limlardagi tamoyillarning bevosita natijasi.

## 25.2. Fragmentatsiya

**Hayotdan misol: bo'sh joylar bor, lekin avtobusga yetmaydi.** Turargohda 6 ta bo'sh joy bor, lekin ular tarqoq: 2 ta bu yerda, 1 ta u yerda, 3 ta narida. 5 joy kerak bo'ladigan avtobus sig'maydi —
**tashqi fragmentatsiya**. **Ichki fragmentatsiya** — kichik mashinaga katta joy berilgani: joy band, lekin yarmi ishlatilmayapti.

- **Ichki:** blok so'ralganidan katta (tekislash, sarlavha, minimal hajm). 17 bayt so'ralsa, 32 bayt berilishi mumkin.
- **Tashqi:** jami bo'sh xotira yetarli, lekin **uzluksiz** bo'lak yo'q: `[band][bo'sh 8][band][bo'sh 8]` — 16 bayt bo'sh, lekin 16 baytli so'rovni qondirib bo'lmaydi.

Tashqi fragmentatsiyani to'liq yo'q qilib bo'lmaydi (bloklarni ko'chirib bo'lmaydi), faqat kamaytirish mumkin: yaxshi joylashtirish siyosati va qo'shni bo'sh bloklarni **birlashtirish**.

Turargoh simulyatori — fragmentatsiyani ko'z bilan ko'rish uchun (20 ta joy; `.` — bo'sh, harf — mashina):

```c
/* turargoh.c - first fit ajratish, bo'shatish va tashqi fragmentatsiya */
#include <stdio.h>
#include <string.h>

#define JOYLAR 20
static char joy[JOYLAR + 1];                    /* '.' - bo'sh, harf - mashina */

static int ajrat(char mashina, int kerak)       /* first fit */
{
    for (int i = 0; i + kerak <= JOYLAR; i++) {
        int bosh = 1;
        for (int j = i; j < i + kerak; j++)
            if (joy[j] != '.') {
                bosh = 0;
                break;
            }
        if (bosh) {
            memset(&joy[i], mashina, kerak);
            return i;
        }
    }
    return -1;
}

static void bosh_qil(char mashina)
{
    for (int i = 0; i < JOYLAR; i++)
        if (joy[i] == mashina)
            joy[i] = '.';
}

static void holat(const char *izoh)
{
    int bosh = 0, eng_uzun = 0, joriy = 0;
    for (int i = 0; i < JOYLAR; i++) {
        joriy = joy[i] == '.' ? joriy + 1 : 0;
        bosh += joy[i] == '.';
        if (joriy > eng_uzun)
            eng_uzun = joriy;
    }
    printf("%-24s [%s]  bo'sh: %2d, eng uzun bo'sh qator: %2d\n", izoh, joy, bosh, eng_uzun);
}

int main(void)
{
    memset(joy, '.', JOYLAR);
    holat("Boshida:");
    ajrat('A', 2);
    ajrat('B', 3);
    ajrat('C', 3);
    ajrat('D', 2);
    ajrat('E', 3);
    ajrat('F', 2);
    ajrat('I', 5);
    holat("7 mashina keldi:");

    bosh_qil('B');
    bosh_qil('D');
    bosh_qil('F');
    holat("B, D, F ketdi:");

    int r = ajrat('G', 6);
    printf("6 joyli avtobus G: %s\n", r < 0 ? "SIG'MADI (fragmentatsiya!)" : "joylashdi");

    r = ajrat('H', 2);
    holat("2 joyli H keldi:");
    printf("H %d-joyga qo'yildi - birinchi sig'adigan joy (first fit)\n", r);
    return 0;
}
```

```console
$ gcc -Wall -Wextra turargoh.c -o turargoh
$ ./turargoh
Boshida:                 [....................]  bo'sh: 20, eng uzun bo'sh qator: 20
7 mashina keldi:         [AABBBCCCDDEEEFFIIIII]  bo'sh:  0, eng uzun bo'sh qator:  0
B, D, F ketdi:           [AA...CCC..EEE..IIIII]  bo'sh:  7, eng uzun bo'sh qator:  3
6 joyli avtobus G: SIG'MADI (fragmentatsiya!)
2 joyli H keldi:         [AAHH.CCC..EEE..IIIII]  bo'sh:  5, eng uzun bo'sh qator:  2
H 2-joyga qo'yildi - birinchi sig'adigan joy (first fit)
```

**Bu dastur nima qiladi (umumiy):** 20 joyli turargohga 7 mashina keladi, uchtasi ketadi, keyin 6 joyli avtobus va 2 joyli mashina keladi. Dastur **first fit** siyosatida har birini qayerga qo'yishini va
bo'sh joylar qanday tarqalishini ko'rsatadi.

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `joy[]` | turargoh xaritasi (heap): `.` bo'sh, harf — band |
| `ajrat(mashina, kerak)` | **first fit**: chapdan boshlab `kerak` ta ketma-ket bo'sh joy qidiradi, birinchi topilganini beradi |
| `bosh_qil(mashina)` | mashina ketdi — joylarni bo'sh qiladi (`free`) |
| `holat` | jami bo'sh joy va **eng uzun bo'sh qator** (fragmentatsiya ko'rsatkichi) |

B, D, F ketgandan keyin jami 7 ta bo'sh joy bor, lekin **eng uzun bo'sh qator — 3**: avtobus (6) sig'madi — bo'sh joylar uch bo'lakka bo'lingan (3 + 2 + 2). Bu — tashqi fragmentatsiya. H esa birinchi sig'adigan
bo'shliqqa (2–4-joylar, 3 joylik) qo'yildi va uning oxirida 1 joylik foydasiz bo'lak qoldi (`AAHH.CCC`: 4-joy).

**Sinab ko'ring:** `ajrat` ni **best fit** qiling: barcha bo'sh qatorlardan `kerak` ga sig'adigan eng qisqasini tanlang. H endi qayerga qo'yiladi va foydasiz bo'lak qoladimi? Avtobusdan oldin `bosh_qil('E')` qo'shing —
D, E, F joylari birlashib 7 joy bo'ladi. Avtobus endi sig'adimi?

> **Eslab qoling:** ichki fragmentatsiya — blok ichidagi isrof; tashqi — bo'sh joy bor, lekin uzluksiz emas. Qo'shni bo'sh bloklarni **birlashtirish** tashqi fragmentatsiyani kamaytiradi.

## 25.3. Blok tuzilmasi va yashirin bo'sh ro'yxat

Eng oddiy dizayn (30-mashq): har bir blok oldida sarlavha — hajm va "band/bo'sh" biti. Hajm doim 16 ga karrali, shuning uchun pastki 4 bit bo'sh — ulardan birini bayroq sifatida ishlatish mumkin:

```text
[ hajm | band ] [ foydali yuk ............ ] [ hajm | band ] [ ... ]
  8 bayt           (hajm - 8)
```

Keyingi blok = joriy + hajm. Bo'sh blokni topish — boshidan hamma bloklarni aylanish ("yashirin ro'yxat", implicit list). Oddiy, lekin sekin: O(jami bloklar).

## 25.4. Joylashtirish siyosatlari

**Hayotdan misol: joylashtirish siyosatlari.**

- **First fit** — birinchi sig'adigan joy. Tez, lekin boshida mayda bo'laklar to'planadi.
- **Best fit** — eng mos (eng kichik sig'adigan) joy. Katta joylarni saqlaydi, lekin qidirish uzoq.
- **Next fit** — oxirgi qoldirilgan joydan davom etib qidirish.
- **Worst fit** — eng katta blok (qolgan bo'lak ham foydali bo'lsin degan g'oya) — amalda yomon.

Topilgan blok kattaroq bo'lsa — **bo'lish** (split): kerakli qismi beriladi, qolgani yangi bo'sh blok (agar minimal hajmdan katta bo'lsa).

## 25.5. Birlashtirish va chegara teglari

**Hayotdan misol: qo'shni bo'sh joylar.** Yonma-yon ikki mashina ketdi. Ikkita kichik bo'sh joy emas — bitta katta joy bo'ldi. Allocator buni sezishi uchun har bir blokning **chegara tegi** bor —
qo'shni blok bo'sh yoki band ekanini tez bilish uchun.

`free` qilinganda qo'shni bo'sh bloklar bilan birlashtirish kerak. **Keyingi** blok oson topiladi (joriy + hajm). **Oldingisi-chi?** Chegara tegi (boundary tag, Knuth): har bir blok **oxirida** ham sarlavhaning
nusxasi (footer) turadi:

```text
[ sarlavha | yuk ........ | footer ][ sarlavha | yuk ... | footer ]
                             ▲         ▲
                    oldingi blokning   joriy blok
                    footer'i - joriy sarlavhadan 8 bayt oldin
```

Endi `free` O(1) da ikkala qo'shnini tekshiradi — 4 holat: ikkalasi band / faqat keyingisi bo'sh / faqat oldingisi bo'sh / ikkalasi bo'sh. Optimallashtirish: footer faqat **bo'sh** bloklarda kerak —
band blokning sarlavhasida "oldingisi band" biti.

## 25.6. Aniq bo'sh ro'yxat (explicit free list)

Faqat **bo'sh** bloklarni ikki tomonlama bog'langan ro'yxatda saqlash. Ro'yxat ko'rsatkichlari bo'sh blokning **foydali yuk joyida** turadi (u baribir ishlatilmayapti — 33-mashqdagi slab hiylasi bilan bir xil).
Qidiruv endi O(bo'sh bloklar). Yangi bo'sh blokni qayerga qo'yish: boshiga (LIFO, tez) yoki manzil tartibida (fragmentatsiya kamroq).

## 25.7. Ajratilgan ro'yxatlar (segregated lists) — amaldagi allocator'lar

Har bir o'lcham sinfi uchun alohida bo'sh ro'yxat: {16}, {32}, {48–64}, {65–128}, ..., {4097–∞}. `malloc(n)` — mos sinfdan olish; bo'sh bo'lsa, kattaroq sinfdan bo'lib olish. Tez (deyarli O(1)) va best fit'ga yaqin.
glibc (`ptmalloc`), jemalloc, tcmalloc — shu oila; qo'shimcha ravishda har bir oqimga o'z keshi (qulfsiz tezlik uchun).

## 25.8. Buddy tizimi — yadro sahifalari uchun

**Hayotdan misol: shokolad plitkasi.** 16 bo'lakli plitkadan 3 bo'lak kerak. Yarmiga bo'lasiz (8 + 8), yana yarmiga (4 + 4) — 4 bo'lak olasiz (3 dan katta eng kichik 2 ning darajasi). Qaytarilganda
"juftingiz" (buddy) ham bo'sh bo'lsa — yana birlashadi. Hisoblash juda tez, lekin 3 o'rniga 4 beriladi (ichki fragmentatsiya).

Bloklar faqat 2ᵏ o'lchamda, 2ᵏ ga tekislangan. Ajratish — kattaroq blokni teng ikkiga bo'lish; bo'shatish — "jufti" (buddy) ham bo'sh bo'lsa birlashtirish. Juft manzili bitta XOR bilan topiladi:
`juft = blok ^ (1 << k)`.

```c
/* buddy_xor.c - buddy: o'lcham sinfi va juftni XOR bilan topish */
#include <stdio.h>

static int tartib(unsigned n)                   /* n ta sahifa uchun eng kichik 2^k >= n */
{
    int k = 0;
    while ((1u << k) < n)
        k++;
    return k;
}

int main(void)
{
    printf("1) so'rov -> beriladigan blok:\n");
    unsigned so_rovlar[] = { 1, 2, 3, 5, 9, 33 };
    for (int i = 0; i < 6; i++) {
        int k = tartib(so_rovlar[i]);
        printf("   %2u sahifa -> tartib %d (%2u sahifa beriladi, isrof %2u)\n",
               so_rovlar[i], k, 1u << k, (1u << k) - so_rovlar[i]);
    }

    printf("2) juftni XOR bilan topish (blok = sahifa raqami):\n");
    struct { unsigned blok; int k; } bloklar[] = { { 0, 0 }, { 4, 0 }, { 8, 2 }, { 12, 2 }, { 16, 3 } };
    for (int i = 0; i < 5; i++) {
        unsigned juft = bloklar[i].blok ^ (1u << bloklar[i].k);
        unsigned ota = bloklar[i].blok < juft ? bloklar[i].blok : juft;
        printf("   blok %2u (tartib %d): jufti %2u, birlashsa -> blok %2u (tartib %d)\n",
               bloklar[i].blok, bloklar[i].k, juft, ota, bloklar[i].k + 1);
    }
    return 0;
}
```

```console
$ gcc -Wall -Wextra buddy_xor.c -o buddy_xor
$ ./buddy_xor
1) so'rov -> beriladigan blok:
    1 sahifa -> tartib 0 ( 1 sahifa beriladi, isrof  0)
    2 sahifa -> tartib 1 ( 2 sahifa beriladi, isrof  0)
    3 sahifa -> tartib 2 ( 4 sahifa beriladi, isrof  1)
    5 sahifa -> tartib 3 ( 8 sahifa beriladi, isrof  3)
    9 sahifa -> tartib 4 (16 sahifa beriladi, isrof  7)
   33 sahifa -> tartib 6 (64 sahifa beriladi, isrof 31)
2) juftni XOR bilan topish (blok = sahifa raqami):
   blok  0 (tartib 0): jufti  1, birlashsa -> blok  0 (tartib 1)
   blok  4 (tartib 0): jufti  5, birlashsa -> blok  4 (tartib 1)
   blok  8 (tartib 2): jufti 12, birlashsa -> blok  8 (tartib 3)
   blok 12 (tartib 2): jufti  8, birlashsa -> blok  8 (tartib 3)
   blok 16 (tartib 3): jufti 24, birlashsa -> blok 16 (tartib 4)
```

**Bu dastur nima qiladi:** (1) har so'rov uchun buddy qaysi **2ᵏ** blokni berishini va qancha isrof bo'lishini; (2) bir blokning **jufti**ni bitta XOR bilan topishni va birlashgan blokni ko'rsatadi.

**Nima ko'rdik:** 5 sahifa so'ralsa — 8 beriladi (3 isrof); 33 so'ralsa — 64 (31 isrof) — ichki fragmentatsiya. Juft: `blok ^ (1 << tartib)`: 4-blok (tartib 0) jufti 5; 8-blok (tartib 2) jufti 12, ikkalasi
birlashsa — 8-blok (tartib 3). Nega XOR: ikkilikda juftlar faqat `k`-bitda farq qiladi (3-bobdagi XOR `^=` bilan bitni almashtirish).

- ✅ Birlashtirish juda arzon, tashqi fragmentatsiya nazorat ostida, katta uzluksiz bloklar olish oson (DMA uchun muhim — 16-bob).
- ❌ Ichki fragmentatsiya: 33 sahifa so'ralsa — 64 beriladi.

Linux va MyOS fizik sahifalar uchun buddy ishlatadi (`kernel/mm/pmm.c`, `buddy` lab'i, 32-mashq).

## 25.9. Slab — yadro obyektlari uchun

**Hayotdan misol: tuxum kartoni.** Tuxum uchun maxsus karton: har bir uyacha aynan bitta tuxum o'lchamida. Qidirish yo'q, bo'laklash yo'q — bo'sh uyachani olasiz. Yadroda bir xil o'lchamdagi obyektlar
(jarayon tuzilmasi, inode) juda ko'p yaratiladi — ularning har biri uchun alohida "karton" (kesh).

Yadro bir xil o'lchamdagi obyektlarni juda ko'p yaratadi (inode, fayl, jarayon, tarmoq paketi). Slab: har bir obyekt turi uchun kesh; kesh buddy'dan sahifa olib, uni teng obyektlarga bo'ladi.
Bo'sh obyektlar **bog'langan ro'yxat** hosil qiladi — ko'rsatkich bo'sh obyektning **o'zi ichida** saqlanadi (25.6 dagi hiyla):

```c
/* slab_misol.c - bitta sahifani 64 baytli obyektlarga bo'lish */
#include <stdint.h>
#include <stdio.h>

#define OBYEKT 64
#define SONI 8

static uint8_t sahifa[OBYEKT * SONI];           /* "buddy'dan olingan sahifa" */
static void *bosh;                              /* bo'sh obyektlar ro'yxati boshi */

static void slab_boshla(void)
{
    bosh = NULL;
    for (int i = SONI - 1; i >= 0; i--) {       /* hamma obyektni ro'yxatga bog'laymiz */
        void *obj = &sahifa[i * OBYEKT];
        *(void **)obj = bosh;                   /* ko'rsatkich - obyektning O'Z ICHIDA */
        bosh = obj;
    }
}

static void *slab_ajrat(void)
{
    void *p = bosh;
    if (p)
        bosh = *(void **)p;                     /* ro'yxatning boshidan olish: O(1) */
    return p;
}

static void slab_bosh_qil(void *p)
{
    *(void **)p = bosh;                         /* ro'yxat boshiga qaytarish: O(1) */
    bosh = p;
}

static long offset(void *p) { return p ? (long)((uint8_t *)p - sahifa) : -1; }

int main(void)
{
    slab_boshla();
    void *a = slab_ajrat();
    void *b = slab_ajrat();
    void *c = slab_ajrat();
    printf("a=%ld b=%ld c=%ld (offsetlar)\n", offset(a), offset(b), offset(c));

    slab_bosh_qil(b);
    void *d = slab_ajrat();
    printf("b qaytarildi, d = slab_ajrat() -> %ld  (LIFO: yaqinda bo'shatilgan qayta beriladi)\n", offset(d));

    int n = 3;                                  /* a, c, d band; 5 ta bo'sh */
    while (slab_ajrat())
        n++;
    printf("jami %d ta obyekt ajratildi, keyingisi: %ld (NULL - slab to'ldi)\n", n, offset(slab_ajrat()));
    return 0;
}
```

```console
$ gcc -Wall -Wextra slab_misol.c -o slab_misol
$ ./slab_misol
a=0 b=64 c=128 (offsetlar)
b qaytarildi, d = slab_ajrat() -> 64  (LIFO: yaqinda bo'shatilgan qayta beriladi)
jami 8 ta obyekt ajratildi, keyingisi: -1 (NULL - slab to'ldi)
```

**Bu dastur nima qiladi:** bitta "sahifa"ni 8 ta 64 baytli obyektga bo'ladi. Bo'sh obyektlar bir-biriga bog'langan (ko'rsatkich obyektning birinchi 8 baytida). Ajratish — ro'yxat boshidan olish; qaytarish —
boshiga qo'yish: ikkalasi **O(1)**.

**Nima ko'rdik:** `a`, `b`, `c` — 0, 64, 128 offsetlarda. `b` qaytarilgach, keyingi `slab_ajrat()` **aynan o'sha** `b` ni (offset 64) qayta berdi — LIFO (yaqinda ishlatilgan obyekt keshda "issiq", 21-bob).
Barcha 8 obyekt tugagach — `NULL`. Qidirish yo'q, bo'laklash yo'q, fragmentatsiya yo'q.

- ✅ O(1) ajratish/bo'shatish, fragmentatsiya deyarli yo'q (hamma obyekt bir xil).
- ✅ Obyekt manzilidan slabni topish — manzilni maskalash (33-mashq).
- ✅ Keshga mos: yaqinda bo'shatilgan obyekt (hali keshda "issiq") birinchi qayta beriladi (LIFO).
- ✅ Debug imkoniyatlari: "qizil zonalar" (redzone) va zahar (poison) bilan chegaradan chiqish va use-after-free'ni ushlash — MyOS slab'i shuni qiladi (`demo=uaf`).

Linux tarixi: SLAB (1994, Bonwick'ning Solaris uchun g'oyasi) → SLUB (2007, sodda va tez, hozir asosiy). `kmalloc(n)` — turli o'lchamlar uchun umumiy keshlar to'plami (16, 32, 64, ... 8192 bayt).

> **Eslab qoling:** buddy — **sahifalar** uchun (2ᵏ bloklar, juft = `blok ^ (1<<k)`); slab — **kichik bir xil obyektlar** uchun (bo'sh ro'yxat, O(1)). Slab buddy'dan sahifa oladi.

## 25.10. Qatlamlar birgalikda (MyOS va Linux)

```text
kmalloc / kmem_cache_alloc   <- slab: kichik obyektlar
        │ (yangi slab kerak bo'lsa)
alloc_pages(order)           <- buddy: 2^order ta fizik sahifa
        │
memblock                     <- boot paytida, buddy tayyor bo'lguncha
        │
fizik RAM (BIOS/UEFI xotira xaritasidan)
```

User dasturda: `malloc` → (kichik) o'z ro'yxatlari / (katta) `mmap` → `brk`/`mmap` syscall → yadro VMA yaratadi → page fault'da buddy'dan sahifa.

## Hayotdan misol va to'liq dastur

**Avtoturargoh.** Bobning to'liq dasturi — 25.2 dagi `turargoh.c`: first fit siyosati, ajratish/bo'shatish va tashqi fragmentatsiya. Qolgan misollar (haqiqiy `malloc` o'lchovi, buddy XOR hisobi, slab) — shu
g'oyaning boshqa qatlamlari: `malloc` (ro'yxatlar), buddy (sahifalar), slab (obyektlar).

## Bob xulosasi (yodlash uchun)

1. Allocator katta hududni bo'laklab beradi; bloklarni **ko'chirib bo'lmaydi**; tezlik va isrof — zid maqsadlar. Haqiqiy `malloc(1)` ham 24 bayt beradi (ichki fragmentatsiya).
2. **Fragmentatsiya:** ichki (blok ichidagi isrof) va tashqi (bo'sh joy bor, lekin uzluksiz emas); yechim — to'g'ri siyosat + **birlashtirish** (chegara teglari bilan O(1)).
3. Siyosatlar: first/next/best fit; ro'yxatlar: yashirin → aniq → **ajratilgan** (o'lcham sinflari bo'yicha; glibc shunday).
4. **Buddy:** bloklar 2ᵏ, juft = `blok ^ (1 << k)`, birlashtirish arzon, lekin ichki fragmentatsiya — fizik **sahifalar** uchun.
5. **Slab:** bir xil obyektlar uchun kesh; bo'sh obyektlar ro'yxati obyektning o'zida; O(1), LIFO, fragmentatsiya yo'q. Qatlam: slab → buddy → memblock.

## O'zingizni tekshiring

1. Ichki va tashqi fragmentatsiya farqi, har biriga misol.
2. Chegara tegi nima muammoni hal qiladi?
3. Nega aniq bo'sh ro'yxat uchun qo'shimcha xotira kerak emas?
4. Buddy'da 5 sahifa so'ralsa nechta beriladi? Qanday isrof bu?
5. Nega yadroga slab kerak, buddy yetmaydimi?

<details><summary>Javoblar</summary>

1. Ichki — blok ichidagi foydalanilmagan joy (17 bayt uchun 32); tashqi — bo'sh joy sochilgan, katta uzluksiz bo'lak yo'q.
2. `free` da oldingi blokni O(1) da topish (birlashtirish uchun).
3. Ko'rsatkichlar bo'sh blokning ishlatilmayotgan foydali yuk joyiga yoziladi.
4. 8 ta; ichki fragmentatsiya (3 sahifa isrof).
5. Buddy eng kichik birlik — sahifa (4 KB); 64 baytli obyektlar uchun juda isrofli va sekin. Slab sahifani ko'p obyektga bo'ladi.
</details>

## Mashq

- **30** (mini malloc) — endi chegara teglari yoki aniq ro'yxat bilan qayta yozib, tezligini solishtiring.
- **32** (buddy), **33** (slab).
- MyOS: `user/libc/malloc.c` ni o'qing — qaysi dizayn tanlangan va nega?

<!-- loyiha:boshi -->
## Loyiha: arena (bump) ajratuvchi

**Maqsad:** eng oddiy va eng tez xotira ajratuvchini yozish. Yadro boshlanishida (`kmalloc` hali tayyor emas) va kompilyatorlarda
(bir necha ming kichik obyektni birdaniga tashlab yuborish kerak bo'lganda) ko'p ishlatiladi (25.1).
**Bobdan ishlatiladi:** tekislash (alignment), ajratish siyosatining eng sodda ko'rinishi, xotira tugaganda `NULL`.

**G'oya:** katta bufer + bitta "hozirgi joy" ko'rsatkichi. Ajratish — ko'rsatkichni oldinga surish. Bo'shatish **yo'q**: butun arenani birdaniga tozalaymiz
(`reset`) yoki belgigacha qaytamiz (`qayt`).
**Tekislash:** `int` 4 ga, `double` 8 ga bo'linadigan manzilda turishi kerak. Formula: `(joy + t - 1) & ~(t - 1)` (`t` — 2 ning darajasi).

```c
/* arena.c - bump ajratuvchi */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static uint8_t xotira[64];                      /* kichik arena, tugashini ko'rish oson bo'lsin */
static size_t joy;                              /* keyingi bo'sh bayt raqami */

static void *ajrat(size_t n, size_t tekis)
{
    size_t bosh = (joy + tekis - 1) & ~(tekis - 1);     /* tekislash: keyingi bo'linadigan manzil */
    if (bosh + n > sizeof(xotira))
        return NULL;                            /* joy yo'q */
    joy = bosh + n;
    return &xotira[bosh];
}

static size_t belgi(void) { return joy; }
static void qayt(size_t b) { joy = b; }
static void tozala(void) { joy = 0; }

static void korsat(const char *nom, const void *p)
{
    if (p)
        printf("%-22s offset %2zu, keyingi joy %2zu\n", nom, (size_t)((const uint8_t *)p - xotira), joy);
    else
        printf("%-22s NULL (joy yetmadi), keyingi joy %2zu\n", nom, joy);
}

int main(void)
{
    char *s = ajrat(5, 1);                      /* 5 bayt, tekislash shart emas */
    korsat("char[5]", s);
    int *son = ajrat(sizeof(int), _Alignof(int));       /* 4 ga tekis: 5 -> 8 */
    korsat("int", son);
    double *d = ajrat(sizeof(double), _Alignof(double));/* 8 ga tekis: 12 -> 16 */
    korsat("double", d);

    size_t b = belgi();                         /* shu joyni eslab qolamiz */
    char *vaqtincha = ajrat(30, 1);
    korsat("vaqtincha[30]", vaqtincha);
    qayt(b);                                    /* vaqtinchani "bo'shatdik" */
    korsat("qaytdik", ajrat(0, 1));

    void *katta = ajrat(100, 8);
    korsat("katta[100]", katta);                /* sig'maydi: 64 baytlik arena */

    strcpy(s, "abcd");                          /* ajratilgan xotiradan foydalanish */
    *son = 42;
    *d = 3.5;
    printf("s = %s, son = %d, d = %.1f\n", s, *son, *d);

    tozala();
    korsat("tozalangandan keyin", ajrat(1, 1));
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address,undefined arena.c -o arena
$ ./arena
char[5]                offset  0, keyingi joy  5
int                    offset  8, keyingi joy 12
double                 offset 16, keyingi joy 24
vaqtincha[30]          offset 24, keyingi joy 54
qaytdik                offset 24, keyingi joy 24
katta[100]             NULL (joy yetmadi), keyingi joy 24
s = abcd, son = 42, d = 3.5
tozalangandan keyin    offset  0, keyingi joy  1
```

`int` 8-baytdan boshlandi (5-bayt o'rniga), chunki 5 ga 4 bo'linmaydi — orada 3 bayt **bo'sh joy** (padding, 9-bob). Bu — ajratuvchining bepul
narxi: tez (bitta qo'shish), lekin bo'sh joyni qayta ishlatib bo'lmaydi.

**Kengaytiring:** arena o'lchamini `sizeof(xotira)` dan parametrga aylantiring va bir nechta arena yarating. `ajrat` ga `0` uzunlik berilsa nima bo'ladi?

## Mustaqil loyiha: ajratish siyosatlari taqqoslash ★★★

**Vazifa:** 25.4-bo'limdagi uch siyosat — **first-fit**, **best-fit**, **worst-fit** — bir xil ish yukida qanday **fragmentatsiya** berishini o'lchang.
Fayl: `fit.c`. Hech qanday `malloc` yo'q — xotira shunchaki **bo'sh oraliqlar ro'yxati** ustida hisoblanadi.

**Model:** xotira 2048 birlik (`[0, 2048)`). Bo'sh joy — **manzil bo'yicha tartiblangan** oraliqlar `(boshi, uzunligi)`. Boshida bitta oraliq: `(0, 2048)`.

**Siyosatlar** (yetarli, ya'ni `uzunlik ≥ o'lcham` oraliqlar orasidan):
- **first-fit** — manzili eng past;
- **best-fit** — uzunligi eng kichik (teng bo'lsa manzili past);
- **worst-fit** — uzunligi eng katta (teng bo'lsa manzili past).

**Ajratish:** tanlangan oraliqning **boshidan** `o'lcham` birlik olinadi; qolgan qismi (bo'lsa) oraliq bo'lib qoladi. Mos oraliq yo'q bo'lsa — ajratish
**muvaffaqiyatsiz** (sanaladi, blok qo'shilmaydi).
**Bo'shatish:** blok qaytariladi va qo'shni bo'sh oraliqlar bilan **birlashtiriladi** (chapdagi va o'ngdagi).

**Ish yuki** (har siyosat uchun **aynan** shu ketma-ketlik): tasodifiy son generatori

```text
uint32_t davlat = 12345;
sonni_ol():  davlat = davlat * 1103515245u + 12345u;  return (davlat >> 16) & 0x7fff;
```

400 qadam. Har qadamda `r = sonni_ol() % 100`:
- agar `r < 60` **yoki** tirik bloklar yo'q → **ajratish**: `o'lcham = 1 + sonni_ol() % 40`;
- aks holda → **bo'shatish**: `k = sonni_ol() % tirik_soni`; tirik bloklar ro'yxatining `k`-elementi bo'shatiladi, ro'yxatdan
  o'chirish — **oxirgi element `k` o'rniga qo'yiladi** (swap-remove). Tirik ro'yxat ajratish tartibida to'ldiriladi.

**Chiqish** (har siyosat uchun bitta qator; `%-9s` va qolganlari aniq — kutilgan natijaga qarang). Ma'lumotlar: ajratish urinishlari, muvaffaqiyatsizlari, oxirida tirik
bloklar soni, jami bo'sh joy `T`, eng katta bo'sh oraliq `L`, **tashqi fragmentatsiya** `(T − L) · 100 / T` (butun bo'lish; `T = 0` bo'lsa `0`).

**Kutilgan natija** (`darslik/loyihalar/25_fit_siyosat/kutilgan.txt`):

```text
first-fit ajratish 247, muvaffaqiyatsiz   4, tirik  90, bo'sh  156, eng katta   22, fragmentatsiya 85%
best-fit  ajratish 247, muvaffaqiyatsiz   4, tirik  90, bo'sh   90, eng katta   12, fragmentatsiya 86%
worst-fit ajratish 247, muvaffaqiyatsiz  15, tirik  79, bo'sh  437, eng katta   20, fragmentatsiya 95%
```

**Maslahat** (yechim emas):
- Bitta `simulyatsiya(siyosat)` funksiyasi; siyosat — `int` kod (0, 1, 2) yoki funksiya ko'rsatkichi. Generator holatini har siyosat uchun **qayta** `12345` ga qo'ying.
- Bo'sh oraliqlar — `struct oraliq { long boshi, uzunlik; }` massivi, tartiblangan. Ajratish oraliqni qisqartiradi (yoki nol uzunlik bo'lsa o'chiradi).
- Bo'shatishda o'rin toping (tartib bo'yicha), chap qo'shni `chap.boshi + chap.uzunlik == blok.boshi` bo'lsa qo'shing, o'ng qo'shni ham.
- Qadamlar generatordan **bir xil tartibda** son oladi — ajratish `2` ta, bo'shatish `2` ta son ishlatadi (`r`, keyin `o'lcham` yoki `k`). Tartibni buzmang.
- Natijalarni tahlil qiling: qaysi siyosat eng kam fragmentatsiya beradi? Nega best-fit har doim g'olib emas (25.4)?

**Tekshirish:**

```bash
gcc -Wall -Wextra -g -fsanitize=address,undefined fit.c -o dastur && ./dastur | diff - ~/C_loyha/darslik/loyihalar/25_fit_siyosat/kutilgan.txt && echo "TO'G'RI"
```
<!-- loyiha:oxiri -->

Keyingi bob: [26-bob. Parallellik chuqur](26-parallellik-chuqur.md)
