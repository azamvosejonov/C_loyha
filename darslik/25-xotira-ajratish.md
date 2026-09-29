# 25-bob. Dinamik xotira ajratish: `malloc` ichida va yadro allocator'lari

> **Bu bobdan keyin:** allocator'ning maqsadlari va cheklovlarini, ichki/tashqi fragmentatsiyani,
> yashirin va aniq bo'sh ro'yxatlarni, chegara teglarini (boundary tags), joylashtirish siyosatlarini
> (first/next/best fit), ajratilgan ro'yxatlarni, buddy va slab'ni bilasiz. (CS:APP 9.9 va OSTEP
> "Free-Space Management".) Mashqlar: 13, 30, 32, 33.

> **To'liq ishlaydigan misol:** [misollar/25_malloc_ichi.c](misollar/25_malloc_ichi.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Hayotdan misollar

**Allocator — avtoturargoh qo'riqchisi (25.1).** Turargohga har xil mashinalar keladi: kichik, o'rta,
avtobus. Qo'riqchi har biriga joy ko'rsatadi (`malloc`) va ketganda joyni bo'sh deb belgilaydi (`free`).
Mashinani joyidan surib bo'lmaydi — dasturdagi ko'rsatkichlar uning manziliga bog'langan.

**Fragmentatsiya — bo'sh joylar bor, lekin avtobusga yetmaydi (25.2).** Turargohda 6 ta bo'sh joy bor,
lekin ular tarqoq: 2 ta bu yerda, 1 ta u yerda, 3 ta narida. 5 joy kerak bo'ladigan avtobus sig'maydi —
**tashqi fragmentatsiya**. **Ichki fragmentatsiya** — kichik mashinaga katta joy berilgani: joy band,
lekin yarmi ishlatilmayapti.

**Joylashtirish siyosatlari (25.4).**
- **First fit** — birinchi sig'adigan joy. Tez, lekin boshida mayda bo'laklar to'planadi.
- **Best fit** — eng mos (eng kichik sig'adigan) joy. Katta joylarni saqlaydi, lekin qidirish uzoq.
- **Next fit** — oxirgi qoldirilgan joydan davom etib qidirish.

**Birlashtirish — qo'shni bo'sh joylar (25.5).** Yonma-yon ikki mashina ketdi. Ikkita kichik bo'sh joy
emas — bitta katta joy bo'ldi. Allocator buni sezishi uchun har bir blokning **chegara tegi** bor —
qo'shni blok bo'sh yoki band ekanini tez bilish uchun.

**Buddy tizimi — shokolad plitkasi (25.8).** 16 bo'lakli plitkadan 3 bo'lak kerak. Yarmiga bo'lasiz
(8 + 8), yana yarmiga (4 + 4) — 4 bo'lak olasiz (3 dan katta eng kichik 2 ning darajasi). Qaytarilganda
"juftingiz" (buddy) ham bo'sh bo'lsa — yana birlashadi. Hisoblash juda tez, lekin 3 o'rniga 4 beriladi
(ichki fragmentatsiya).

**Slab — tuxum kartoni (25.9).** Tuxum uchun maxsus karton: har bir uyacha aynan bitta tuxum o'lchamida.
Qidirish yo'q, bo'laklash yo'q — bo'sh uyachani olasiz. Yadroda bir xil o'lchamdagi obyektlar (jarayon
tuzilmasi, inode) juda ko'p yaratiladi — ularning har biri uchun alohida "karton" (kesh).

### To'liq dastur: avtoturargoh (first fit va fragmentatsiya)

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

B, D, F ketgandan keyin jami 7 ta bo'sh joy bor, lekin avtobus sig'madi: bo'sh joylar uch bo'lakka
bo'lingan (3 + 2 + 2). H esa birinchi sig'adigan 3 joylik bo'shliqqa qo'yildi va 1 joylik foydasiz
bo'lak qoldi.

**Sinab ko'ring:** `ajrat` ni **best fit** qiling: barcha bo'sh qatorlardan `kerak` ga sig'adigan eng
qisqasini tanlang. H endi qayerga qo'yiladi va foydasiz bo'lak qoladimi? Avtobusdan oldin
`bosh_qil('E')` qo'shing — D, E, F joylari birlashib 7 joy bo'ladi. Avtobus endi sig'adimi?

## 25.1. Vazifa va cheklovlar

Allocator katta xotira hududini (heap) oladi va so'rovlarga bo'laklab beradi. Cheklovlari:
- so'rovlar **istalgan tartibda** keladi va ketadi;
- berilgan blokni **ko'chirib bo'lmaydi** (dasturda uning manzili saqlangan);
- javob **darhol** berilishi kerak;
- har bir blok **tekislangan** (16 bayt, x86-64 da).

Ikki maqsad bir-biriga zid: **tezlik** (so'rov/sekund) va **xotira samaradorligi** (isrof kam).

## 25.2. Fragmentatsiya

- **Ichki:** blok so'ralganidan katta (tekislash, sarlavha, minimal hajm). 17 bayt so'ralsa, 32 bayt berilishi mumkin.
- **Tashqi:** jami bo'sh xotira yetarli, lekin **uzluksiz** bo'lak yo'q: `[band][bo'sh 8][band][bo'sh 8]` —
  16 bayt bo'sh, lekin 16 baytli so'rovni qondirib bo'lmaydi.

Tashqi fragmentatsiyani to'liq yo'q qilib bo'lmaydi (bloklarni ko'chirib bo'lmaydi), faqat
kamaytirish mumkin: yaxshi joylashtirish siyosati va qo'shni bo'sh bloklarni **birlashtirish**.

## 25.3. Blok tuzilmasi va yashirin bo'sh ro'yxat

Eng oddiy dizayn (30-mashq): har bir blok oldida sarlavha — hajm va "band/bo'sh" biti. Hajm doim
16 ga karrali, shuning uchun pastki 4 bit bo'sh — ulardan birini bayroq sifatida ishlatish mumkin:

```text
[ hajm | band ] [ foydali yuk ............ ] [ hajm | band ] [ ... ]
  8 bayt           (hajm - 8)
```

Keyingi blok = joriy + hajm. Bo'sh blokni topish — boshidan hamma bloklarni aylanish ("yashirin
ro'yxat", implicit list). Oddiy, lekin sekin: O(jami bloklar).

## 25.4. Joylashtirish siyosatlari

- **First fit** — birinchi mos bo'sh blok. Tez; ro'yxat boshida mayda bo'laklar to'planadi.
- **Next fit** — oxirgi to'xtagan joydan davom etish. Tezroq, lekin fragmentatsiya ko'proq.
- **Best fit** — eng kichik mos blok. Kam isrof, lekin hammasini ko'rish kerak (agar tuzilma yordam bermasa).
- **Worst fit** — eng katta blok (qolgan bo'lak ham foydali bo'lsin degan g'oya) — amalda yomon.

Topilgan blok kattaroq bo'lsa — **bo'lish** (split): kerakli qismi beriladi, qolgani yangi bo'sh blok
(agar minimal hajmdan katta bo'lsa).

## 25.5. Birlashtirish va chegara teglari

`free` qilinganda qo'shni bo'sh bloklar bilan birlashtirish kerak. **Keyingi** blok oson topiladi
(joriy + hajm). **Oldingisi-chi?** Chegara tegi (boundary tag, Knuth): har bir blok **oxirida** ham
sarlavhaning nusxasi (footer) turadi:

```text
[ sarlavha | yuk ........ | footer ][ sarlavha | yuk ... | footer ]
                             ▲         ▲
                    oldingi blokning   joriy blok
                    footer'i - joriy sarlavhadan 8 bayt oldin
```

Endi `free` O(1) da ikkala qo'shnini tekshiradi — 4 holat: ikkalasi band / faqat keyingisi bo'sh /
faqat oldingisi bo'sh / ikkalasi bo'sh. Optimallashtirish: footer faqat **bo'sh** bloklarda kerak —
band blokning sarlavhasida "oldingisi band" biti.

## 25.6. Aniq bo'sh ro'yxat (explicit free list)

Faqat **bo'sh** bloklarni ikki tomonlama bog'langan ro'yxatda saqlash. Ro'yxat ko'rsatkichlari bo'sh
blokning **foydali yuk joyida** turadi (u baribir ishlatilmayapti — 33-mashqdagi slab hiylasi bilan bir xil).
Qidiruv endi O(bo'sh bloklar). Yangi bo'sh blokni qayerga qo'yish: boshiga (LIFO, tez) yoki manzil
tartibida (fragmentatsiya kamroq).

## 25.7. Ajratilgan ro'yxatlar (segregated lists) — amaldagi allocator'lar

Har bir o'lcham sinfi uchun alohida bo'sh ro'yxat: {16}, {32}, {48–64}, {65–128}, ..., {4097–∞}.
`malloc(n)` — mos sinfdan olish; bo'sh bo'lsa, kattaroq sinfdan bo'lib olish. Tez (deyarli O(1)) va
best fit'ga yaqin. glibc (`ptmalloc`), jemalloc, tcmalloc — shu oila; qo'shimcha ravishda har bir
oqimga o'z keshi (qulfsiz tezlik uchun).

## 25.8. Buddy tizimi — yadro sahifalari uchun

Bloklar faqat 2ᵏ o'lchamda, 2ᵏ ga tekislangan. Ajratish — kattaroq blokni teng ikkiga bo'lish;
bo'shatish — "jufti" (buddy) ham bo'sh bo'lsa birlashtirish. Juft manzili bitta XOR bilan topiladi:
`juft = blok ^ (1 << k)`.

- ✅ Birlashtirish juda arzon, tashqi fragmentatsiya nazorat ostida, katta uzluksiz bloklar olish oson
  (DMA uchun muhim — 16-bob).
- ❌ Ichki fragmentatsiya: 33 sahifa so'ralsa — 64 beriladi.

Linux va MyOS fizik sahifalar uchun buddy ishlatadi (`kernel/mm/pmm.c`, `buddy` lab'i, 32-mashq).

## 25.9. Slab — yadro obyektlari uchun

Yadro bir xil o'lchamdagi obyektlarni juda ko'p yaratadi (inode, fayl, jarayon, tarmoq paketi). Slab:
har bir obyekt turi uchun kesh; kesh buddy'dan sahifa olib, uni teng obyektlarga bo'ladi.

- ✅ O(1) ajratish/bo'shatish, fragmentatsiya deyarli yo'q (hamma obyekt bir xil).
- ✅ Obyekt manzilidan slabni topish — manzilni maskalash (33-mashq).
- ✅ Keshga mos: yaqinda bo'shatilgan obyekt (hali keshda "issiq") birinchi qayta beriladi (LIFO).
- ✅ Debug imkoniyatlari: "qizil zonalar" (redzone) va zahar (poison) bilan chegaradan chiqish va
  use-after-free'ni ushlash — MyOS slab'i shuni qiladi (`demo=uaf`).

Linux tarixi: SLAB (1994, Bonwick'ning Solaris uchun g'oyasi) → SLUB (2007, sodda va tez, hozir asosiy).
`kmalloc(n)` — turli o'lchamlar uchun umumiy keshlar to'plami (16, 32, 64, ... 8192 bayt).

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

User dasturda: `malloc` → (kichik) o'z ro'yxatlari / (katta) `mmap` → `brk`/`mmap` syscall → yadro VMA
yaratadi → page fault'da buddy'dan sahifa.

## 25.11. O'zingizni tekshiring

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

## 25.12. Mashqlar

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
