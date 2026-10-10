# 32-bob. Amaliyot: MyOS'ning `printf` va `malloc` ini o'zingiz yozasiz

> **Bu bobda nima o'rganasiz:** `printf` ichidan qanday ishlaydi (o'zgaruvchan argumentlar, format satri,
> "emit" arxitekturasi); sonni matnga aylantirish va kenglik/to'ldirish/aniqlik qoidalari; yadroning o'z
> `kprintf` i; `malloc` va `free` — heap, blok sarlavhasi, bo'sh ro'yxat, "first fit", bo'lish (split),
> birlashtirish (coalescing), xotirani yadroga qaytarish.
> **Oldindan nima kerak:** 5 (funksiyalar), 7 (ko'rsatkichlar), 8 (xotira), 9 (struct), 12 (standart
> kutubxona), 25-bob (xotira ajratish nazariyasi). MyOS kodini yig'a olish (asosiy README).
> **Mashqlar:** M1, M2, M3, M4 — kodlari repoda **yo'q**, siz yozasiz.   **Vaqt:** 8–12 soat.

## Bu bob nima haqida?

Siz hozirgacha `printf` va `malloc` dan **foydalandingiz**. Ular har bir C dasturining asosi — lekin
"sehrli" emas, oddiy C kodi. MyOS'da ular **o'zimizniki** (`user/libc/`), yadrodagi `kprintf` ham. Bu bobda
ularning **eng muhim qismlari repodan olib tashlangan** — siz yozasiz:

| Mashq | Fayl | Funksiya | Nima | Qiyinlik |
|---|---|---|---|---|
| **M1** | `user/libc/printf.c` | `emit_number` | sonni matnga: asos, ishora, kenglik, `0`, `-`, aniqlik | ★★ |
| **M2** | `kernel/lib/kprintf.c` | `format_core` (qismi) | format satrini o'qish: `%-08.3s` dagi bayroqlar, kenglik, aniqlik | ★★ |
| **M3** | `user/libc/malloc.c` | `malloc` | first fit, bo'lish, band belgisi, heap'ni o'stirish | ★★★ |
| **M4** | `user/libc/malloc.c` | `insert_free` | tartiblangan ro'yxatga qo'shish, qo'shnilar bilan birlashtirish | ★★★ |

Yechimlar repoda **yo'q** — ataylab (boshqa laboratoriyalardagidek "yashirin asl kod" ham yo'q). Testlar
yechimingiz to'g'riligini to'liq tekshiradi.

**Hayotdan misol: avtomobil ustaxonasi.** Mashina (MyOS) yuradi, lekin ikkita detal vaqtinchalik:
spidometr o'rniga "?" ko'rsatkichi (sonlar chiqmaydi) va bak o'rniga kanistr (yoqilg'i qaytib ishlatilmaydi).
Mashina yuradi — lekin to'liq emas. Siz haqiqiy detallarni yasab, joyiga qo'yasiz.

## 32.1. Hozir nima bo'ladi: vaqtinchalik variantlar

Har bo'sh joyda **vaqtinchalik** kod bor, shuning uchun MyOS yig'iladi va ishlaydi:

- **M1 ochiq:** foydalanuvchi dasturlarida **har bir son `?`** bo'lib chiqadi.
- **M2 ochiq:** yadro sonlarni chiqaradi, lekin **kenglik va to'ldirishsiz** (`%08x` → shunchaki `x`).
- **M3 ochiq:** `malloc` har safar yadrodan **yangi** joy oladi (`sbrk`), bo'shatilgan xotira qayta ishlatilmaydi.
- **M4 ochiq:** `free` blokni ro'yxat **boshiga** qo'yadi — tartib ham, birlashtirish ham yo'q.

Haqiqiy natija (`make test` logidan). Yadro xotira xaritasi — M2 ochiq va M2 yozilgandan keyin:

```text
[boot] Xotira xaritasi:                              [boot] Xotira xaritasi:
         0 - 9fbff  RAM                                       0000000000000000 - 000000000009fbff  RAM
         9fc00 - 9ffff  band                                  000000000009fc00 - 000000000009ffff  band
[rtc]  Sana: 2026-10-5 17:10:38 (UTC)                [rtc]  Sana: 2026-10-05 17:08:40 (UTC)
```

Foydalanuvchi dasturlari — M1/M3/M4 ochiq:

```text
memtest: user heap stress testi (? blok)
  boshida:               heap=? band=? (? blok) bo'sh=? (? blok)
  ajratilgandan keyin:   heap=? band=? (? blok) bo'sh=? (? blok)
  [FAIL] bo'sh bloklar birlashmadi (coalesce ishlamadi)
memtest: ? ta FAIL
```

Va eng qiziq "yon ta'sir" — fayl tizimi testi:

```text
fstest: /mnt/fst
  [FAIL] 300 ta fayl yaratildi (errno=? Noto'g'ri fayl deskriptori)
```

Fayl tizimi buzilmagan! `fstest` fayl nomlarini `snprintf(name, ..., "fayl_uzunroq_nom_bilan_%03d", i)` bilan
yasaydi. M1 ochiq bo'lsa, 300 ta faylning **hammasi** `fayl_uzunroq_nom_bilan_?` deb ataladi. Ya'ni `printf`
— faqat "ekranga chiqarish" emas: sonni matnga aylantiradigan **hamma joyda** u bor.

`make test` (butun tizim, QEMU'da) hozir: **28 OK, 15 FAIL**. Hammasini yozganingizdan keyin: **43 OK, 0 FAIL**.

## 32.2. `printf` qanday ishlaydi

### O'zgaruvchan argumentlar

`printf("%d %s", 42, "salom")` — argumentlar soni oldindan noma'lum. C buni `...` va `<stdarg.h>` bilan hal qiladi:

```c
int printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);              /* fmt dan KEYINGI argumentlarni olishni boshlash */
    int n = vfprintf(stdout, fmt, ap);
    va_end(ap);
    return n;
}
```

`va_arg(ap, int)` — navbatdagi argumentni **siz aytgan tur** sifatida oladi. Tur xato bo'lsa (masalan `%d` ga
`long` berildi) — kompilyator tekshirmaydi, natija noto'g'ri. Shuning uchun format satri bilan argumentlar
mos bo'lishi shart (`gcc -Wall` `__attribute__((format(printf, 1, 2)))` orqali ogohlantiradi).

### Bitta dvigatel, ko'p "chiqish"

`printf`, `fprintf`, `snprintf`, `dprintf` — hammasi bitta `__format()` funksiyasini chaqiradi. Farqi faqat
"tayyor belgini **qayerga** qo'yish" ([user/libc/printf.c](../user/libc/printf.c)):

```c
typedef void (*emit_fn)(char c, void *ctx);          /* bitta belgini "qayergadir" yozadi */

int __format(emit_fn emit, void *ctx, const char *fmt, va_list ap);

/* snprintf uchun emit: satrga yozadi, sig'masa ham SANAYDI (C standarti talab qiladi) */
static void emit_str(char c, void *ctx)
{
    struct str_buf *s = ctx;
    if (s->pos + 1 < s->size)
        s->buf[s->pos] = c;
    s->pos++;
}
```

Bu — **funksiya ko'rsatkichi** orqali "strategiya" (darslik 7-bob): format mantig'i bir marta yoziladi,
chiqish joyi (ekran, satr, fayl, yadro konsoli) — parametr.

### Format satri grammatikasi

```text
%  [bayroqlar]  [kenglik]  [.aniqlik]  [uzunlik]  tur
    - 0 + ' '     12 yoki *   .3 yoki .*  l ll z     d i u x X o p s c %
```

`__format` (tayyor) bu qismlarni o'qib, `struct spec` ga yozadi va `tur` bo'yicha ish qiladi. Sonlar uchun
u **modul** va **ishorani** hisoblab, `emit_number` ni chaqiradi:

```c
case 'd':
case 'i': {
    int64_t v = is_long ? va_arg(ap, int64_t) : va_arg(ap, int);
    uint64_t mag = v < 0 ? (uint64_t)0 - (uint64_t)v : (uint64_t)v;
    emit_number(emit, ctx, mag, v < 0, 10, false, &sp, &count);
    break;
}
```

Nega `(uint64_t)0 - (uint64_t)v`, `-v` emas? `INT64_MIN` = −2⁶³ ning musbati (2⁶³) `int64_t` ga **sig'maydi**:
`-v` — toshish, UB (13-bob). Ishorasiz arifmetikada esa ayirish aniqlangan (mod 2⁶⁴) va to'g'ri modulni beradi.

## 32.3. M1 — `emit_number`

```c
static void emit_number(emit_fn emit, void *ctx, uint64_t v, bool neg, unsigned base,
                        bool upper, const struct spec *sp, int *count);
```

Natija **besh qismdan** iborat, aynan shu tartibda:

```text
[ chap bo'shliqlar ][ ishora ][ '0' to'ldirish ][ aniqlik nollari ][ raqamlar ][ o'ng bo'shliqlar ]
   (width, !left,     - + ' '   (width, zero,      (prec - len)                   (width, left)
    !zero yoki prec)            prec berilmagan)
```

Har qism ixtiyoriy. Misollar — chap ustun format, o'ng ustun natija (`[` `]` — chegara):

| Format | Qiymat | Natija | Nima ishladi |
|---|---|---|---|
| `%d` | 0 | `[0]` | v = 0 bo'lsa ham bitta raqam |
| `%5d` | 42 | `[   42]` | chap bo'shliqlar |
| `%-5d` | 42 | `[42   ]` | o'ng bo'shliqlar |
| `%05d` | -42 | `[-0042]` | ishora **oldin**, nollar **keyin** |
| `%5d` | -42 | `[  -42]` | bo'shliqlar ishoradan **oldin** |
| `%+d` | 5 | `[+5]` | `+` bayrog'i |
| `% d` | 5 | `[ 5]` | bo'shliq bayrog'i |
| `%.3d` | -7 | `[-007]` | aniqlik: kamida 3 raqam |
| `%08.3d` | 7 | `[     007]` | aniqlik berilsa `0` bayrog'i **e'tiborsiz** |
| `%.0d` | 0 | `[]` | aniqlik 0 va qiymat 0 — **hech narsa** |
| `%x` / `%X` | 0xbeef | `beef` / `BEEF` | `upper` |
| `%lu` | 2⁶⁴−1 | `18446744073709551615` | 20 raqam |

### Qo'lda bitta misol: `%-8.3d`, qiymat 7

```text
raqamlar:          "7"      (len = 1)
aniqlik nollari:   3 - 1 = 2 ta  -> "007"
ishora:            yo'q
jami:              3
to'ldirish:        8 - 3 = 5 ta, left -> o'ngda
natija:            "007     "
```

### Maslahatlar

- Raqamlar `v % base` bilan **teskari** chiqadi: 1234 → 4, 3, 2, 1. Vaqtinchalik massivga yig'ib, oxiridan
  chiqaring (`char tmp[24]` — 2⁶⁴ o'nlikda 20 raqam).
- `"0123456789abcdef"[d]` — raqamni belgiga aylantirishning eng qisqa yo'li.
- Har chiqarilgan belgi uchun `(*count)++` — `printf` qaytaradigan son shu.
- Tayyor yordamchi: `pad(emit, ctx, belgi, n, count)`.
- Yozib bo'lgach `emit_number_vaqtinchalik` funksiyasini va uning chaqiruvini **o'chiring**.

## 32.4. M2 — yadroning `kprintf` i: format satrini o'qish

Yadroda libc yo'q — `kprintf` alohida, o'zining sodda dvigateli bilan
([kernel/lib/kprintf.c](../kernel/lib/kprintf.c)). Ikki qiziq farqi bor:

1. **Qator buferi.** Ko'p yadroli tizimda ikki CPU bir vaqtda `kprintf` qilsa, harflar aralashib ketadi
   (`[cpu0] sa[cpu1] xlomayr...`). Shuning uchun xabar avval **stekdagi buferga** formatlanadi, keyin bitta
   `console_write()` bilan (bitta qulf ostida) chiqariladi:

```c
int kvprintf(const char *fmt, va_list ap)
{
    struct line_buf b;
    b.len = 0;
    int n = format_core(emit_line, &b, fmt, ap);
    if (b.len)
        console_write(b.data, b.len);
    return n;
}
```

2. **Faqat kerakli formatlar:** `%d %i %u %x %X %p %s %c %%`, `l`/`ll`/`z`, bayroqlar `-` va `0`, kenglik,
   `%s` uchun aniqlik (`%.4s` — ACPI jadval imzolari 4 harf, oxirida `\0` yo'q!).

M2 — `format_core` ichidagi **o'qish** qismi: `%` dan keyin bayroqlar, kenglik, aniqlik. Natijada to'rtta
o'zgaruvchi to'ldirilishi kerak: `left`, `zero_pad`, `width`, `precision`.

```text
"%-08.3s"
  ^^^^^^ fmt shu yerdan boshlanadi ('%' dan keyin)
  -      bayroq: left = true
   0     bayroq: zero_pad = true
    8    kenglik: width = 8
     .3  aniqlik: precision = 3
       s konversiya — tayyor kod davom ettiradi
```

**Tuzoq:** `%08x` da `0` — bayroq, `8` — kenglik. `%10d` da esa `1` va `0` — **ikkalasi ham** kenglik! Farq:
`0` bayroq faqat bayroqlar qismida (raqamlar boshlanishidan **oldin**) bo'lishi mumkin. Shuning uchun avval
bayroqlar tsikli (`-` yoki `0` ekan — davom et), keyin raqamlar tsikli.

Raqamlar ketma-ketligidan sonni yig'ish — klassik usul (Horner sxemasi):

```c
width = width * 10 + (*fmt - '0');      /* "12": 0*10+1 = 1, 1*10+2 = 12 */
```

`*fmt - '0'` — belgini raqamga aylantirish: ASCII'da `'0'..'9'` ketma-ket (darslik 2-bob).

## 32.5. `malloc` — nazariya

### Heap va `sbrk`

Jarayonning xotirasida **heap** — dastur ishlayotganda o'sadigan hudud. Uning chegarasi — "break". `sbrk(n)`
tizim chaqiruvi uni `n` baytga suradi va **eski** chegarani qaytaradi (yadro yangi sahifalarni "talab
bo'yicha" beradi — darslik 24-bob). `sbrk(-n)` — xotirani yadroga qaytarish.

`sbrk` sekin (tizim chaqiruvi) va faqat **oxiridan** o'sadi/qisqaradi. `malloc` uning ustidagi qatlam: yadrodan
katta bo'lak oladi va uni mayda bo'laklarga bo'lib beradi, qaytganlarini **qayta ishlatadi**.

### Blok sarlavhasi

Har blok oldida 16 baytlik sarlavha ([user/libc/malloc.c](../user/libc/malloc.c)):

```c
struct block {
    size_t size;                        /* foydali yuk hajmi (sarlavhasiz), 16 ga karrali */
    struct block *next;                 /* bo'sh: keyingi bo'sh blok; band: USED_MAGIC */
};
```

```text
          malloc(100) qaytargan manzil (b + 1)
                 v
+-------+--------+-----------------------------+-------+--------+------------
| size  | next   |  foydali yuk (112 bayt)      | size  | next   |  ...
| = 112 | =MAGIC |  (100 -> 16 ga yaxlitlangan) |       |        |
+-------+--------+-----------------------------+-------+--------+------------
^ b (sarlavha)                                  ^ keyingi blok = (char *)b + HDR + b->size
```

- `free(p)` sarlavhani qanday topadi? `(struct block *)p - 1` — ko'rsatkich arifmetikasi: 1 ta `struct block`
  (16 bayt) orqaga.
- **Nega 16 ga tekislash?** C standarti: `malloc` istalgan tur uchun mos tekislangan manzil berishi kerak.
  x86-64 da SSE ko'rsatmalari (masalan `long double`, `__int128`) 16 ga tekis manzilni talab qiladi — aks
  holda dastur qulaydi.
- **`USED_MAGIC`** — band blokning `next` iga yoziladigan maxsus qiymat. `free()` uni tekshiradi:
  yo'q bo'lsa — "bu ko'rsatkich malloc'dan emas yoki ikki marta bo'shatilgan" (double free). glibc'ning
  `free(): double free detected` xabari shu g'oyada.

### Bo'sh ro'yxat (free list)

Bo'sh bloklar **manzil bo'yicha tartiblangan** bog'langan ro'yxatda (`free_list`). Nega tartiblangan?
Birlashtirish (M4) uchun: xotirada yonma-yon turgan bloklar ro'yxatda ham yonma-yon bo'ladi.

**First fit** — ro'yxatdan sig'adigan **birinchi** blokni olish. Muqobillar: best fit (eng mos o'lchamli —
sekinroq, lekin maydaroq qoldiqlar), next fit, segregated lists (glibc, jemalloc — o'lchamlar bo'yicha
alohida ro'yxatlar). Darslik 25-bobida ularning solishtirmasi bor.

### Tayyor qismlar

`free`, `grow` (yadrodan kamida 64 KB olish), `trim_top` (heap tepasidagi katta bo'sh blokni yadroga
qaytarish), `calloc`, `realloc`, `malloc_get_stats` — tayyor, o'qing:

```c
void free(void *ptr)
{
    if (!ptr)
        return;
    struct block *b = (struct block *)ptr - 1;
    if (b->next != USED_MAGIC)
        malloc_abort("free(): noto'g'ri ko'rsatkich yoki DOUBLE FREE", ptr);

    used_bytes -= b->size;
    used_blocks--;
    insert_free(b);
    trim_top();                         /* faqat haqiqiy free() da - grow() da EMAS! */
}
```

`calloc` dagi xavfsizlik tekshiruviga ham qarang: `count * size` toshib ketsa (`calloc(2^33, 2^33)`), kichik
blok ajratilib, dastur uning chegarasidan tashqariga yozib yuborardi — klassik zaiflik.

## 32.6. M3 — `malloc`

Qadamlar (kodda ham bor):

1. `size == 0` yoki > 1 GB → `NULL`. `size = ALIGN16(size)`.
2. **First fit:** `free_list` bo'ylab yuring, `prev` ni ham eslab qoling (ro'yxatdan chiqarish uchun kerak).
3. **Split:** blok ancha katta bo'lsa (`b->size >= size + MIN_SPLIT`) — boshini beramiz, qolganidan yangi
   bo'sh blok:

```text
OLDIN:   prev -> [ b: size = 4000                                      ] -> b->next
KEYIN:   prev -> [ rest: size = 4000 - 112 - 16 ] -> b->next
                 ^ (char *)b + HDR + 112
         [ b: size = 112, next = USED_MAGIC ]  -> foydalanuvchiga b + 1
```

   `MIN_SPLIT` (32 bayt) dan kichik qoldiq qilmaymiz — undan foydasiz "chang" bo'laklar paydo bo'ladi.
4. `b` ni ro'yxatdan chiqaring: `prev` bo'lsa `prev->next = ...`, aks holda `free_list = ...` — **`prev == NULL`
   holatini unutmang** (birinchi blok). Band belgisi, statistika, `return b + 1`.
5. Topilmasa: `grow(size)` va qaytadan; `grow` 0 qaytarsa — `NULL`.

Yozib bo'lgach `malloc_vaqtinchalik` funksiyasini **o'chiring**.

## 32.7. M4 — `insert_free`: tartib va birlashtirish

```c
static void insert_free(struct block *b);
```

1. **Joyini toping:** `prev < b < cur`. Ko'rsatkichlarni `<` bilan solishtirish C'da faqat **bitta massiv**
   (bitta obyekt) ichida aniqlangan — heap aynan shunday bitta uzluksiz hudud.
2. **Ulang.**
3. **O'ng qo'shni:** `b` ning oxiri `cur` ning boshiga tegsa — `b` `cur` ni "yutadi".
4. **Chap qo'shni:** `prev` ning oxiri `b` ning boshiga tegsa — `prev` `b` ni "yutadi".

```text
free(b) dan oldin:   [a: bo'sh][b: band][c: bo'sh]
                      free_list -> a -> c
ulash:               free_list -> a -> b -> c
o'ng (b + c):        free_list -> a -> [b+c]
chap (a + b+c):      free_list -> [a+b+c]          <- bitta katta blok
```

Nega avval o'ng, keyin chap? Agar avval chap bilan birlashsangiz, `b` endi `prev` ning bir qismi — keyin
"`b` ning o'ng qo'shnisi" ni tekshirish uchun `prev` dan foydalanish kerak bo'ladi; ko'p xatolar shu yerda.
O'ng → chap tartibida har qadam sodda qoladi.

**Birlashtirish bo'lmasa nima bo'ladi?** Fragmentatsiya: jami 1 MB bo'sh bo'lsa ham, hammasi 64 baytlik
bo'laklar — 1 KB so'rov uchun joy yo'q, `malloc` yana yadrodan oladi, heap cheksiz o'sadi. `memtest` aynan
shuni tekshiradi: "hammasi bo'shatilgach — bo'sh bloklar soni 1".

**Tartiblangan ro'yxat yana nimaga kerak?** `trim_top` ro'yxatning **oxirgi** elementini heap tepasidagi blok
deb hisoblaydi. Tartib buzilsa — xotira yadroga qaytmaydi.

## 32.8. Tekshirish

### Tez: `tools/myos_mashq.sh` (QEMU'siz, ~1 soniya)

Fayllaringiz kompyuteringizda kompilyatsiya qilinadi va glibc'ning `snprintf` i "hakam" sifatida ishlatiladi
(standart formatlar uchun u to'g'ri javobni biladi). `malloc` esa 64 MB lik statik "heap" ustida sinaladi
(`sbrk` o'rinbosari — [tests/host/mashq_shim.c](../tests/host/mashq_shim.c)). Hozirgi holat:

```console
$ tools/myos_mashq.sh
MyOS mashqlari (darslik/32-printf-malloc.md):
        lab_snprintf("%d")                 -> olindi "?", kutilgan "0"
        lab_snprintf("%d")                 -> olindi "?", kutilgan "42"
        ...
  [XATO] M1 printf: emit_number (user/libc/printf.c): 0/28
        lab_ksnprintf("[%5d]")             -> olindi "[42]", kutilgan "[   42]"
        lab_ksnprintf("[%08x]")            -> olindi "[beef]", kutilgan "[0000beef]"
        ...
  [XATO] M2 kprintf: bayroqlar/kenglik/aniqlik (kernel/lib/kprintf.c): 3/15
        birinchi malloc: grow() yadrodan kamida 64 KB oldi
        split: 64 KB dan 16 bayt berildi, QOLGANI bitta bo'sh blok (heap - 16 - 2 sarlavha)
        free(x); malloc(xuddi shu hajm) -> x qaytadi (free list ishlatildi)
  [XATO] M3 malloc: first fit, split, qayta ishlatish (user/libc/malloc.c): 10/13
        manzil bo'yicha tartib: free(a), free(c) dan keyin malloc(64) -> a (eng past manzil)
        free(b): chap (a) va o'ng (c) qo'shnilar bilan birlashdi -> 1 blok
        ...
  [XATO] M4 insert_free: tartib va birlashtirish (user/libc/malloc.c): 2/7
```

Maqsad:

```console
  [ OK ] M1 printf: emit_number (user/libc/printf.c): 28/28
  [ OK ] M2 kprintf: bayroqlar/kenglik/aniqlik (kernel/lib/kprintf.c): 15/15
  [ OK ] M3 malloc: first fit, split, qayta ishlatish (user/libc/malloc.c): 13/13
  [ OK ] M4 insert_free: tartib va birlashtirish (user/libc/malloc.c): 7/7
  hammasi o'tdi. Endi butun tizim: make test
```

M4 testlari ishlaydigan `malloc` ga tayanadi — tartib: M1, M2, M3, M4.

### To'liq: `make test` (QEMU'da, ~1 daqiqa)

```text
$ make test
==> DIQQAT: ochiq mashqlar: M1 M2 M3 M4 - sonlar '?' bo'lib chiqadi, malloc xotirani qayta ishlatmaydi.
...
==> Natija (bios): 28 OK, 15 FAIL
```

Hammasi yozilgach, `memtest` (300 blok stress testi) shunday chiqishi kerak:

```text
memtest: user heap stress testi (300 blok)
  boshida:               heap=     0 band=     0 (  0 blok) bo'sh=     0 (0 blok)
  ajratilgandan keyin:   heap=327680 band=286336 (300 blok) bo'sh= 36528 (1 blok)
  yarmi bo'shatilgach:   heap=327680 band=147408 (150 blok) bo'sh=175456 (151 blok)
  hammasi bo'shatilgach: heap= 69632 band=     0 (  0 blok) bo'sh= 69616 (1 blok)
  1 MB ajratilgach:      heap=1122304 band=1048576 (  1 blok) bo'sh= 73696 (1 blok)
  1 MB bo'shatilgach:    heap= 69632 band=     0 (  0 blok) bo'sh= 69616 (1 blok)
memtest: PASSED
```

O'qing: "yarmi bo'shatilgach — 151 bo'sh blok" (har ikkinchi blok bo'sh — qo'shnisi band, birlashmaydi);
"hammasi bo'shatilgach — **1** blok" (M4 hammasini birlashtirdi) va `heap` 327680 → 69632 (`trim_top`
ortiqchani yadroga qaytardi). Oxirida `==> Natija (bios): 43 OK, 0 FAIL`.

CI (GitHub Actions) mashqlar ochiq turganda integratsion testlarni o'tkazib yuboradi va faqat
`tools/myos_mashq.sh` ni ishlatadi; hammasi yozilgach — to'liq `make test`.

## 32.9. Kengaytirish (ixtiyoriy)

1. **`%#x`** (`0x` prefiksi) va **`%f`** (suzuvchi nuqta: butun va kasr qismini alohida chiqarish, yaxlitlash —
   juda chuqur mavzu, darslik 20-bob).
2. **Best fit** — `malloc` ni o'zgartiring va `memtest` dagi `heap` va bo'sh bloklar sonini first fit bilan
   solishtiring.
3. **Segregated lists:** 16, 32, 64, ... baytlik bloklar uchun alohida ro'yxatlar — kichik `malloc` O(1).
4. **`realloc` ni yaxshilash:** o'ngdagi qo'shni bo'sh bo'lsa — nusxa ko'chirmasdan joyida kattalashtirish.
5. **Xavfsizlik:** sarlavhaga "kanareyka" qo'shing (yuk oxiridan keyin maxsus qiymat) va `free()` da
   tekshiring — bufer toshishini ushlaydi (yadrodagi slab `redzone` i kabi, darslik 29-bob).

## Mustaqil loyiha: MyOS'ning printf va malloc'i ★★★

**Vazifa:** M1, M2, M3, M4 — hammasini yozing (32.3–32.7). Vaqtinchalik funksiyalarni (`emit_number_vaqtinchalik`,
`malloc_vaqtinchalik`) va vaqtinchalik qatorlarni o'chiring.

**Kutilgan natija** (`darslik/loyihalar/32_myos_mashq/kutilgan.txt`):

```text
MyOS mashqlari (darslik/32-printf-malloc.md):
  [ OK ] M1 printf: emit_number (user/libc/printf.c): 28/28
  [ OK ] M2 kprintf: bayroqlar/kenglik/aniqlik (kernel/lib/kprintf.c): 15/15
  [ OK ] M3 malloc: first fit, split, qayta ishlatish (user/libc/malloc.c): 13/13
  [ OK ] M4 insert_free: tartib va birlashtirish (user/libc/malloc.c): 7/7
  hammasi o'tdi. Endi butun tizim: make test
```

**Tekshirish:**

```bash
tools/myos_mashq.sh | diff - darslik/loyihalar/32_myos_mashq/kutilgan.txt && echo "TO'G'RI"
make test          # oxirida: ==> Natija (bios): 43 OK, 0 FAIL
```

**Maslahat** (yechim emas):
- Har mashqni alohida yozing va darhol `tools/myos_mashq.sh` bilan tekshiring. Xato qatoridagi formatni o'zingiz
  qog'ozda 32.3 dagi "besh qism" sxemasi bo'yicha yoyib chiqing.
- `malloc` uchun qog'ozda manzillar bilan chizing: 64 KB lik blok, undan 16, keyin 32 bayt — sarlavhalar qayerda?
- Sanitizer: `tools/myos_mashq.sh` dagi `gcc` qatorlariga `-fsanitize=address,undefined` qo'shib ko'ring
  (faqat test binarisi uchun) — ko'rsatkich xatolari darhol ko'rinadi.

## Mashq

### Isitish: printf'siz son chiqarish ★☆☆ — eng osoni, avval shuni qiling

Faqat 0–32-boblar kerak (32.3 dagi raqamlarni teskari yig'ish).
Skeletni `isitish.c` ga **qo'lda** yozing (ko'chirmang), izohlarni o'qing va `TODO` joylarini to'ldiring.
"Namuna" qismlar tayyor — qolganini qanday yozishni ko'rsatadi. Skelet hozir ham ogohlantirishsiz yig'iladi:
har `TODO` dan keyin yig'ib, ishga tushirib boring.

```c
/* isitish.c - 32-bob, isitish: printf'siz son chiqarish - faqat putchar. M1 (emit_number) ning soddasi. */
#include <stdio.h>

static void chiqar_satr(const char *s)
{
    while (*s)
        putchar(*s++);
}

/* Ishorasiz son -> istalgan asosdagi raqamlar (2..16). Raqamlar teskari chiqadi (x % asos - ENG KICHIK raqam),
 * shuning uchun buferni oxiridan to'ldiramiz (32.3). 64 ta ikkilik raqam + '\0' = 65. (Namuna - tayyor.) */
static void chiqar_son(unsigned long x, unsigned asos)
{
    char buf[65];
    int i = 64;
    buf[i] = '\0';
    do {                                /* do-while: x == 0 bo'lsa ham bitta '0' chiqsin */
        buf[--i] = "0123456789abcdef"[x % asos];
        x /= asos;
    } while (x);
    chiqar_satr(&buf[i]);
}

/* TODO: x < 0 bo'lsa putchar('-') va chiqar_son(0UL - (unsigned long)x, 10); aks holda chiqar_son((unsigned long)x, 10).
 *       -x DEMANG: x = LONG_MIN bo'lsa -x long ga sig'maydi (UB, 13-bob). Ishorasizda ayirish - aniqlangan. */
static void chiqar_ishorali(long x)
{
    (void)x;
    chiqar_satr("?");
}

int main(void)
{
    chiqar_son(0, 10); putchar('\n');
    chiqar_son(42, 10); putchar('\n');
    chiqar_son(18446744073709551615UL, 10); putchar('\n');
    chiqar_satr("0x"); chiqar_son(0xdeadbeef, 16); putchar('\n');
    chiqar_son(5, 2); putchar('\n');
    chiqar_ishorali(-9223372036854775807L - 1); putchar('\n');     /* LONG_MIN */
    return 0;
}
```

**Kutilgan natija** (`darslik/loyihalar/32_myos_mashq/isitish.txt`):

```text
0
42
18446744073709551615
0xdeadbeef
101
-9223372036854775808
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address,undefined isitish.c -o isitish
$ ./isitish | diff - ~/C_loyha/darslik/loyihalar/32_myos_mashq/isitish.txt && echo "TO'G'RI"
TO'G'RI
```

### Keyingi mashqlar

- **M1–M4** (32.3–32.7) — `emit_number`, `kprintf` format o'qish, `malloc`, `insert_free`: haqiqiy MyOS kodi.
  Isitishdagi `chiqar_son` — M1 ning yuragi: endi unga kenglik, to'ldirish va ishorani qo'shasiz.

## Savol-javob

**Savol:** `printf("%d", x)` da `x` `char` bo'lsa, `va_arg(ap, int)` to'g'rimi?
**Javob:** Ha. O'zgaruvchan argumentlarda `char` va `short` avtomatik `int` ga, `float` esa `double` ga
**ko'tariladi** (default argument promotions). Shuning uchun `va_arg(ap, char)` — xato (UB), har doim `int`.

**Savol:** Nega `malloc(0)` `NULL` qaytaradi? glibc kichik blok qaytaradi-ku.
**Javob:** Standart ikkalasiga ham ruxsat beradi ("implementation-defined"). Biz soddaroq va xavfsizroq
variantni tanladik: `NULL` — "foydalanib bo'lmaydigan ko'rsatkich" ekanini aniq bildiradi.

**Savol:** Yadroning `kmalloc` i ham shunday ishlaydimi?
**Javob:** Yo'q — yadroda **slab** allocator ([kernel/mm/slab.c](../kernel/mm/slab.c)): bir xil o'lchamli
obyektlar uchun O(1), fragmentatsiyasiz. Foydalanuvchi dasturlari esa istalgan o'lchamni so'raydi — shuning
uchun "free list". Ikkalasini solishtirish — darslik 25-bob.
