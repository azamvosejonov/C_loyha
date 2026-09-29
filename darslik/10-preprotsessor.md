# 10-bob. Preprotsessor: `#include`, `#define`, makrolar

> **Bu bobdan keyin:** `#` bilan boshlanadigan har bir qatorni, makrolarning tuzoqlarini,
> `do { } while (0)` nima uchun kerakligini, `#x` va `a##b` ni, shartli kompilyatsiyani bilasiz.
> Yadro kodining taxminan 10% i makrolar — ularni o'qiy olishingiz shart. Mashqlar: 23 va barcha `test.h`.

> **To'liq ishlaydigan misol:** [misollar/10_makrolar.c](misollar/10_makrolar.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Hayotdan misollar

**Preprotsessor — "Hammasini almashtirish" tugmasi (10.1).** Word'da "Topish va almashtirish" bor:
"Tosh." so'zini hamma joyda "Toshkent" ga almashtirasiz. Preprotsessor ham shunday — u C'ni
**tushunmaydi**, faqat matnni almashtiradi. Kompilyator almashtirilgan matnni oladi.

**`#include` — kitobga boshqa sahifalarni yopishtirish (10.2).** "Bu yerga 3-ilovani qo'shing" degan
joyga ilova sahifalari **so'zma-so'z** ko'chirib qo'yiladi. `#include <stdio.h>` o'rniga 800 ga yaqin
qator paydo bo'ladi (1-bobda `gcc -E` bilan ko'rgansiz).

**`#define N 10` — shartnoma boshidagi kelishuv (10.3).** Shartnomalarda boshida yoziladi: "Bundan
buyon 'Ijarachi' deganda 'Aliyev Vali' tushuniladi". Keyin matnda faqat 'Ijarachi' yoziladi. Ijarachi
almashsa — faqat bitta qatorni o'zgartirasiz. `#define N 10` ham shunday: 10 ni 20 ga o'zgartirish
uchun faqat bitta qatorni tuzatasiz, massiv o'lchami ham, sikl ham o'zi moslashadi.

**Makro tuzog'i — so'zma-so'z almashtirish (10.4).** "Kvadratni hisoblang: x × x" degan retseptga
x o'rniga "2 + 3" ni **qavssiz** qo'ysangiz: 2 + 3 × 2 + 3 = 11. Odam buni tushunib qavs qo'yardi,
preprotsessor — yo'q. Shuning uchun makroda har bir parametr va butun ifoda qavsga olinadi.

**`#ifdef` — kitobning ikki nashri (10.8).** Darslikning o'qituvchi nashrida javoblar bor, o'quvchi
nashrida yo'q. Matn bitta, faqat ba'zi sahifalar bitta nashrga kiradi. `-DDEBUG` bilan yig'sangiz —
"o'qituvchi nashri" (qo'shimcha xabarlar bilan), usiz — oddiy nashr.

**`#x` — so'zni qo'shtirnoqqa olish (10.6).** Makro ifodani ham hisoblaydi, ham uning **yozilishini**
satr sifatida chiqaradi: `TEKSHIR(a > 0)` xato bo'lsa "a > 0 bajarilmadi" deb yoza oladi.

### To'liq dastur: narxlar jadvali

```c
/* narxlar.c - #define o'zgarmaslar, makro funksiyalar, #x, shartli kompilyatsiya */
#include <stdio.h>

#define QQS_FOIZ 12                             /* soliq stavkasi o'zgarsa - faqat shu qator */
#define QQS_BILAN(narx) ((narx) + (narx) * QQS_FOIZ / 100)
#define SONI(massiv) (sizeof(massiv) / sizeof((massiv)[0]))
#define TEKSHIR(shart)                                                        \
    do {                                                                      \
        if (!(shart))                                                         \
            printf("  OGOHLANTIRISH: \"%s\" bajarilmadi\n", #shart);          \
    } while (0)

#ifdef DEBUG
#define LOG(xabar) printf("  [debug] %s\n", xabar)
#else
#define LOG(xabar) do { } while (0)             /* oddiy nashrda - hech narsa */
#endif

struct mahsulot {
    const char *nom;
    long narx;
};

int main(void)
{
    struct mahsulot dokon[] = {
        { "Non", 4000 }, { "Sut (1 l)", 12000 }, { "Guruch (1 kg)", 18000 }, { "Choy", -500 },
    };

    LOG("jadval chiqarilmoqda");
    printf("%-15s %10s %12s\n", "Mahsulot", "Narx", "QQS bilan");
    for (size_t i = 0; i < SONI(dokon); i++) {
        printf("%-15s %10ld %12ld\n", dokon[i].nom, dokon[i].narx, QQS_BILAN(dokon[i].narx));
        TEKSHIR(dokon[i].narx > 0);
    }
    printf("Jami %zu ta mahsulot, QQS %d%%\n", SONI(dokon), QQS_FOIZ);
    LOG("tayyor");
    return 0;
}
```

```console
$ gcc -Wall -Wextra narxlar.c -o narxlar
$ ./narxlar
Mahsulot              Narx    QQS bilan
Non                   4000         4480
Sut (1 l)            12000        13440
Guruch (1 kg)        18000        20160
Choy                  -500         -560
  OGOHLANTIRISH: "dokon[i].narx > 0" bajarilmadi
Jami 4 ta mahsulot, QQS 12%
$ gcc -Wall -Wextra -DDEBUG narxlar.c -o narxlar_debug
$ ./narxlar_debug | head -3
  [debug] jadval chiqarilmoqda
Mahsulot              Narx    QQS bilan
Non                   4000         4480
$ gcc -E narxlar.c | grep 'dokon\[i\].nom'
        printf("%-15s %10ld %12ld\n", dokon[i].nom, dokon[i].narx, ((dokon[i].narx) + (dokon[i].narx) * 12 / 100));
```

Oxirgi buyruq `QQS_BILAN(dokon[i].narx)` preprotsessordan keyin nimaga aylanganini ko'rsatadi —
kompilyator aynan shu matnni oladi.

**Sinab ko'ring:** `QQS_FOIZ` ni 15 qiling — faqat bitta qatorni o'zgartirdingiz. `QQS_BILAN` dan
qavslarni olib tashlang: `#define QQS_BILAN(narx) narx + narx * QQS_FOIZ / 100` va uni
`QQS_BILAN(1000) * 2` bilan chaqiring — natija nega noto'g'ri?

## 10.1. Preprotsessor nima

Kompilyatsiyaning **birinchi** bosqichi (1-bob). U C'ni tushunmaydi — faqat **matnni** almashtiradi.
`#` bilan boshlanadigan qatorlar uning buyruqlari; ular `;` bilan tugamaydi, **qator oxiri** bilan tugaydi
(uzun buyruq qatorlarini `\` bilan davom ettirish mumkin).

Natijani ko'rish: `gcc -E fayl.c`.

## 10.2. `#include`

```c
#include <stdio.h>          /* tizim sarlavhasi: -isystem papkalari, /usr/include */
#include "fs/vfs.h"         /* loyiha sarlavhasi: avval joriy papka, keyin -I papkalari */
```

Fayl mazmuni aynan shu joyga ko'chiriladi. Shuning uchun `.h` faylda faqat **e'lonlar**, makrolar,
tur ta'riflari va `static inline` funksiyalar bo'lishi kerak — oddiy funksiya ta'riflari emas (1-bob).

### Bir marta qo'shish

```c
#pragma once                /* zamonaviy usul - GCC/Clang/MSVC qo'llaydi (MyOS shuni ishlatadi) */
```

Eski (standart) usul — "include guard":

```c
#ifndef VFS_H
#define VFS_H
/* ... fayl mazmuni ... */
#endif
```

## 10.3. `#define` — o'zgarmaslar

```c
#define PAGE_SIZE 4096
#define PAGE_MASK (~(PAGE_SIZE - 1))
```

Preprotsessor keyingi hamma `PAGE_SIZE` so'zlarini `4096` ga almashtiradi. Qoidalar:

- Nomlar KATTA harflarda (makro ekanini ko'rsatish uchun).
- Ifodali makroni **doim qavsga** oling. Nega:
  ```c
  #define IKKI 1 + 1
  int x = IKKI * 3;       /* 1 + 1 * 3 = 4, 6 emas! */
  #define IKKI (1 + 1)    /* to'g'ri */
  ```
- Oxirida `;` qo'ymang: `#define N 10;` → `int a[N];` → `int a[10;];` → xato.

`const` va `enum` bilan solishtirish: `enum { PAGE_SIZE = 4096 };` debuggerda ko'rinadi va tur
tekshiriladi; lekin `#define` `#if` da ishlatilishi mumkin va istalgan turdagi (masalan, `64 bit`)
o'zgarmas bo'la oladi. Yadroda ikkalasi ham ishlatiladi.

## 10.4. Funksiyaga o'xshash makrolar

```c
#define KVADRAT(x) ((x) * (x))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
```

Har bir parametrni va butun ifodani **qavsga** oling:

```c
#define YOMON_KVADRAT(x) x * x
YOMON_KVADRAT(a + 1)        /* a + 1 * a + 1  =  2a + 1 !!! */
KVADRAT(a + 1)              /* ((a + 1) * (a + 1)) - to'g'ri */
```

**Ikki marta hisoblash tuzog'i:**

```c
MIN(i++, 10)                /* ((i++) < (10) ? (i++) : (10)) - i ikki marta oshishi mumkin! */
MIN(sekin_funksiya(), 5)    /* funksiya ikki marta chaqiriladi */
```

Makro — **matn almashtirish**, funksiya emas: argument har ishlatilgan joyda qayta hisoblanadi.
Shuning uchun imkon bo'lsa `static inline` funksiya yozing (5-bob). Makro faqat funksiya qila
olmaydigan narsa uchun: tur bo'yicha umumiy kod, `__FILE__`/`__LINE__`, `#x`, `container_of`.

## 10.5. `do { ... } while (0)` — ko'p buyruqli makro

Bir nechta buyruqdan iborat makro:

```c
#define XATO(msg) printf("xato: %s\n", msg); xatolar++
```

```c
if (x < 0)
    XATO("manfiy");         /* ochilganda: if (x < 0) printf(...); xatolar++;  */
                            /* xatolar++ if ga TEGISHLI EMAS - doim bajariladi! */
```

`{ }` bilan o'rash ham to'liq yechim emas:

```c
#define XATO(msg) { printf("xato: %s\n", msg); xatolar++; }
if (x < 0)
    XATO("manfiy");         /* { ... };  <- bu ; bo'sh buyruq */
else                        /* XATO: else dan oldin if tugab qoldi */
    ...
```

To'g'ri yechim — `do { ... } while (0)`:

```c
#define XATO(msg)                          \
    do {                                   \
        printf("xato: %s\n", msg);         \
        xatolar++;                         \
    } while (0)
```

Bu **bitta buyruq**, oxiridagi `;` ni "yutadi" va `if/else` ichida to'g'ri ishlaydi. Tana aniq bir marta
bajariladi. Linux yadrosida va mashqlardagi `test.h` da (`CHECK`, `CHECK_INT`) hamma ko'p qatorli
makrolar shunday. `\` — "makro keyingi qatorda davom etadi" (undan keyin bo'shliq ham bo'lmasligi kerak).

## 10.6. `#` va `##` operatorlari

```c
#define SATR(x) #x                  /* argumentni SATRGA aylantirish */
SATR(a + b)                         /* "a + b" */

#define CHECK(shart) \
    do { if (!(shart)) printf("XATO: %s (%s:%d)\n", #shart, __FILE__, __LINE__); } while (0)
CHECK(x > 0);                       /* XATO: x > 0 (test.c:15) */

#define YOPISHTIR(a, b) a##b        /* ikki so'zni BITTA so'zga yopishtirish */
int YOPISHTIR(ozgaruvchi, 1) = 5;   /* int ozgaruvchi1 = 5; */
```

Mashqlardagi `test.h` va MyOS'ning `kernel/tests/selftest.c` xato bo'lganda shartning **matnini**
aynan `#shart` bilan chiqaradi.

## 10.7. Oldindan aniqlangan makrolar

| Makro | Qiymati |
|---|---|
| `__FILE__` | joriy fayl nomi (`"test.c"`) |
| `__LINE__` | joriy qator raqami |
| `__func__` | joriy funksiya nomi (aslida makro emas, lekin xuddi shunday ishlatiladi) |
| `__DATE__`, `__TIME__` | kompilyatsiya sanasi/vaqti |
| `__x86_64__`, `__linux__`, `__GNUC__` | platforma va kompilyator |

```c
#define PANIC(msg) panic("%s:%d: %s: %s", __FILE__, __LINE__, __func__, msg)
```

## 10.8. Shartli kompilyatsiya

```c
#ifdef DEBUG
    kprintf("[debug] x = %d\n", x);
#endif

#if defined(__x86_64__)
    /* x86-64 ga xos kod */
#elif defined(__aarch64__)
    /* ARM64 */
#else
#error "Bu arxitektura qo'llab-quvvatlanmaydi"
#endif
```

- `gcc -DDEBUG ...` — buyruq qatoridan makro berish.
- `#error` — kompilyatsiyani xabar bilan to'xtatish.
- `#if 0 ... #endif` — kod blokini vaqtincha "o'chirish" (izohdan yaxshiroq: ichida `/* */` bo'lsa ham ishlaydi).

Mashqlarda: `libctest.c` dagi `#ifndef HOST_TEST` — bir xil test faylini yadroda ham, kompyuterda
ham ishlatish uchun (`tools/host_libctest.sh` `-DHOST_TEST` bilan kompilyatsiya qiladi).

## 10.9. Yadroda uchraydigan makrolar

```c
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define ALIGN_UP(x, a)   (((x) + (a) - 1) & ~((a) - 1))
#define ALIGN_DOWN(x, a) ((x) & ~((a) - 1))
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))
#define likely(x)   __builtin_expect(!!(x), 1)      /* "bu shart deyarli doim rost" - optimallashtirishga maslahat */
#define unlikely(x) __builtin_expect(!!(x), 0)
#define BIT(n) (1UL << (n))
```

`!!x` — istalgan sonni 0 yoki 1 ga aylantirish (ikki marta "EMAS"). MyOS'da bularning ko'pi
`kernel/lib/common.h` da (u yerda `ALIGN_UP` `ALIGN_DOWN` orqali yozilgan va `__typeof__` ishlatadi) — ochib o'qing.

**Ro'yxat bo'ylab yurish makrosi** (MyOS: `kernel/lib/list.h`) — makro yangi "sikl sintaksisi" yaratadi:

```c
#define list_for_each_entry(pos, head, member)                                   \
    for (pos = list_entry((head)->next, __typeof__(*pos), member);               \
         &pos->member != (head);                                                 \
         pos = list_entry(pos->member.next, __typeof__(*pos), member))

struct vm_area *it;
list_for_each_entry(it, &areas, node) {     /* kernel/mm/vmalloc.c */
    ...                                     /* it - navbatdagi element */
}
```

`list_entry` — `container_of` ning boshqa nomi, `__typeof__(*pos)` — GCC kengaytmasi: "pos ko'rsatgan
tur". Bitta makro istalgan turdagi obyektlar ro'yxatini aylanadi — C'da shablon (template) yo'q, uning
o'rnini makrolar bosadi.

## 10.10. Savol-javob

**Makro va `inline` funksiya — qaysi biri?**
Imkon bo'lsa — `static inline` (tur tekshiruvi, bir marta hisoblash, debuggerda ko'rinadi). Makro — faqat
funksiya qila olmaydigan narsa uchun.

**Makro ochilganini qanday ko'raman?**
`gcc -E fayl.c | grep -A3 "qidirilgan_joy"`. Chalkash makro xatolarida bu eng yaxshi yo'l.

**Nega kompilyator xatosi makro ichidagi qatorni ko'rsatadi?**
Chunki xato ochilgan kodda. GCC `note: in expansion of macro 'X'` bilan qaysi makrodan kelganini aytadi.

## 10.11. O'zingizni tekshiring

1. `#define KVADRAT(x) x*x` — `KVADRAT(2+3)` nechaga ochiladi va natija qancha?
2. `do { } while (0)` nima uchun?
3. `#x` va `a##b` nima qiladi?
4. `#define N 10;` dagi xato qayerda chiqadi?
5. `MIN(f(), 3)` makrosi bilan qanday muammo bo'lishi mumkin?

<details><summary>Javoblar</summary>

1. `2+3*2+3` = 11 (25 emas).
2. Ko'p buyruqli makroni `if/else` ichida bitta buyruq kabi ishlashi va `;` ni to'g'ri qabul qilishi uchun.
3. `#x` — argumentni satrga aylantiradi; `a##b` — ikki tokenni bittaga yopishtiradi.
4. `N` ishlatilgan joyda (masalan `int a[10;];`) — makro ta'rifida emas.
5. `f()` ikki marta chaqirilishi mumkin (yon ta'sirlar, sekinlik).
</details>

## 10.12. Mashqlar

- `mashqlar/test.h` ni to'liq o'qing: har bir makro nima qilishini va nega `do { } while (0)` ichida
  ekanini tushuntiring.
- **23** — `container_of` va `offsetof`.
- `kernel/lib/common.h` va `kernel/lib/list.h` ni o'qing — har bir makroni o'zingiz ochib yozing.

<!-- loyiha:boshi -->
## Loyiha: mini test kutubxonasi

**Maqsad:** makrolar bilan o'z test tizimingizni yozish: xato bo'lsa **fayl nomi va qator raqami** ko'rsatilsin,
o'tgan/yiqilgan testlar sanalsin. Yadro va boshqa tizim loyihalarida testlar aynan shunday makrolar bilan yoziladi
(mashqlardagi `test.h` ham).
**Bobdan ishlatiladi:** `#define`, `do { } while (0)`, `#x` (stringlash), `__FILE__`, `__LINE__`.

**Talab:** `TEKSHIR_TENG(haqiqat, kutilgan)` — teng bo'lsa `otdi++`, bo'lmasa xato xabarini chiqaradi.
`TEKSHIR_ROST(shart)` — shart rost bo'lishi kerak. Oxirida hisobot.
**Nega makro (funksiya emas)?** `__LINE__` funksiya ichida funksiyaning qatorini beradi, makro esa
**chaqirilgan** joyning qatorini beradi. `#haqiqat` ifodaning matnini satr qilib beradi.

```c
/* testlar.c - mini test kutubxonasi */
#include <stdio.h>

static int otdi, yiqildi;

#define TEKSHIR_TENG(haqiqat, kutilgan)                                                 \
    do {                                                                                \
        long _h = (haqiqat), _k = (kutilgan);                                           \
        if (_h == _k) {                                                                 \
            otdi++;                                                                     \
        } else {                                                                        \
            yiqildi++;                                                                  \
            printf("  XATO %s:%d: %s = %ld, kutilgan %ld\n", __FILE__, __LINE__, #haqiqat, _h, _k); \
        }                                                                               \
    } while (0)

#define TEKSHIR_ROST(shart)                                                             \
    do {                                                                                \
        if (shart) {                                                                    \
            otdi++;                                                                     \
        } else {                                                                        \
            yiqildi++;                                                                  \
            printf("  XATO %s:%d: '%s' rost emas\n", __FILE__, __LINE__, #shart);       \
        }                                                                               \
    } while (0)

/* sinaladigan funksiyalar */
static int modul(int x) { return x < 0 ? -x : x; }
static int eng_katta3(int a, int b, int c)
{
    int m = a;
    if (b > m) m = b;
    if (c > m) m = c;
    return m;
}
static int tub_mi(int n)
{
    if (n < 2) return 0;
    for (int d = 2; d * d <= n; d++)
        if (n % d == 0) return 0;
    return 1;
}

int main(void)
{
    printf("modul:\n");
    TEKSHIR_TENG(modul(-5), 5);
    TEKSHIR_TENG(modul(7), 7);
    TEKSHIR_TENG(modul(0), 0);

    printf("eng_katta3:\n");
    TEKSHIR_TENG(eng_katta3(1, 9, 4), 9);
    TEKSHIR_TENG(eng_katta3(-3, -1, -2), -1);
    TEKSHIR_TENG(eng_katta3(5, 5, 5), 4);       /* ATAYLAB XATO: kutilgan noto'g'ri */

    printf("tub_mi:\n");
    TEKSHIR_ROST(tub_mi(13));
    TEKSHIR_ROST(!tub_mi(15));
    TEKSHIR_ROST(tub_mi(1));                    /* ATAYLAB XATO: 1 tub emas */

    printf("\nnatija: %d o'tdi, %d yiqildi\n", otdi, yiqildi);
    return yiqildi != 0;
}
```

```console
$ gcc -Wall -Wextra -g testlar.c -o testlar
$ ./testlar; echo "chiqish kodi: $?"
modul:
eng_katta3:
  XATO testlar.c:54: eng_katta3(5, 5, 5) = 5, kutilgan 4
tub_mi:
  XATO testlar.c:59: 'tub_mi(1)' rost emas

natija: 7 o'tdi, 2 yiqildi
chiqish kodi: 1
```

Chiqish kodi 1 — CI tizimlari (GitHub Actions ham) shu kod bo'yicha "test yiqildi" deb belgilaydi.
`_h`, `_k` — makro ichidagi lokal o'zgaruvchilar: argument ifodasi (`i++` kabi) faqat **bir marta** hisoblanadi (10.4-bo'lim tuzog'i).

**Kengaytiring:** `TEKSHIR_MATN(a, b)` (`strcmp` bilan) qo'shing. Yiqilgan testlar ro'yxatini oxirida qayta chiqaring.

## Mustaqil loyiha: log tizimi va bit makrolari ★★☆

**Vazifa:** kompilyatsiya vaqtida sozlanadigan log tizimini va yadroda ko'p uchraydigan yordamchi makrolarni yozing.
Hammasi **bitta fayl**: `makro.c`. Hech qanday funksiya kerak emas (faqat `main`).

**1. Log darajalari.** `LOG_DARAJA` (0..4) buyruq qatoridan beriladi: `gcc -DLOG_DARAJA=2 ...`
(berilmasa 3 bo'lsin: `#ifndef`). Makrolar:

| Makro | Chiqishi | Qachon kodga kiradi |
|---|---|---|
| `LOG_XATO(...)` | `[XATO] <matn>` | daraja ≥ 1 |
| `LOG_OGOH(...)` | `[OGOH] <matn>` | daraja ≥ 2 |
| `LOG_INFO(...)` | `[INFO] <matn>` | daraja ≥ 3 |
| `LOG_DEBUG(...)` | `[DEBUG] <matn>` | daraja ≥ 4 |

`...` — `printf` formati va argumentlari: `LOG_INFO("son = %d", 5)`. Daraja yetmasa makro **hech narsaga
aylanmasin** (`do { } while (0)`), kod ham qolmasin. Buning uchun `#if LOG_DARAJA >= 2` kabi shartli kompilyatsiya.
Prefiks bilan matnni bitta `printf` ga birlashtiring; oxirida `\n`. (`__VA_ARGS__` — 10.4.)

**2. Bit makrolari:**
- `BIT(n)` — `1UL << n`;
- `MASKA(yuqori, past)` — `past` dan `yuqori` gacha (ikkalasi ham kiradi) bitlari 1 bo'lgan maska (Linux'da `GENMASK`);
- `ARRAY_SIZE(a)`;
- `MIN(a, b)` va `MAX(a, b)` — argument **faqat bir marta** hisoblansin (GCC `({ ... })` va `__typeof__` yordamida).

**3. `main`** quyidagi tartibda ishlasin:
1. Har to'rt darajadagi log makrosini chaqiradi (mos xabarlar bilan: `xato yuz berdi`, `ogohlantirish`, `ma'lumot: 42`, `tuzatish: 7`),
2. `BIT(5)` = 32, `BIT(0)`, `MASKA(7, 4)` = 240 (`0xF0`), `MASKA(31, 0)`,
3. `int t[] = {4, 8, 15, 16, 23, 42}` uchun `ARRAY_SIZE`,
4. `int i = 5; int m = MIN(i++, 10);` — `m` va `i` ni chiqaring (funksiya kabi bir marta hisoblanishini ko'rsatadi).

Format aniq — natija `-DLOG_DARAJA=2` bilan (`darslik/loyihalar/10_log_makro/kutilgan.txt`):

```text
[XATO] xato yuz berdi
[OGOH] ogohlantirish
BIT(5) = 32, BIT(0) = 1
MASKA(7, 4) = 240 (0xF0)
MASKA(31, 0) = 4294967295
ARRAY_SIZE(t) = 6
MIN(i++, 10) = 5, i = 6
MAX(3, 9) = 9
```

`-DLOG_DARAJA=4` bilan:

```text
[XATO] xato yuz berdi
[OGOH] ogohlantirish
[INFO] ma'lumot: 42
[DEBUG] tuzatish: 7
BIT(5) = 32, BIT(0) = 1
MASKA(7, 4) = 240 (0xF0)
MASKA(31, 0) = 4294967295
ARRAY_SIZE(t) = 6
MIN(i++, 10) = 5, i = 6
MAX(3, 9) = 9
```

`-DLOG_DARAJA=0` bilan log qatorlari **umuman chiqmaydi** (faqat bit va massiv qatorlari):

```text
BIT(5) = 32, BIT(0) = 1
MASKA(7, 4) = 240 (0xF0)
MASKA(31, 0) = 4294967295
ARRAY_SIZE(t) = 6
MIN(i++, 10) = 5, i = 6
MAX(3, 9) = 9
```

**Maslahat** (yechim emas):
- Aynan bir xil shablonli makro: `#define LOG_INFO(...) printf("[INFO] " __VA_ARGS__)` — qo'shni satr literallari birlashadi. Oxirgi `"\n"` ni qanday qo'shasiz?
  (`printf("[INFO] " fmt "\n", ...)` uchun birinchi argumentni ajrating yoki `printf("[INFO] "); printf(...); putchar('\n')` ni `do{}while(0)` ichiga oling.)
- `MIN(a, b)` uchun: `({ __typeof__(a) _a = (a); __typeof__(b) _b = (b); _a < _b ? _a : _b; })`.
- `MASKA(h, l)`: `(BIT(h+1) - BIT(l))` — `h = 31` da `1UL << 32` `unsigned long` (64 bit) da xavfsiz.
- `gcc -E makro.c | tail -30` — makrolaringiz nimaga aylanganini ko'ring.

**Tekshirish** (uch xil yig'ish):

```bash
D=~/C_loyha/darslik/loyihalar/10_log_makro
gcc -Wall -Wextra -DLOG_DARAJA=2 makro.c -o d && ./d | diff - $D/kutilgan.txt && echo "2: TO'G'RI"
gcc -Wall -Wextra -DLOG_DARAJA=4 makro.c -o d && ./d | diff - $D/kutilgan_2.txt && echo "4: TO'G'RI"
gcc -Wall -Wextra -DLOG_DARAJA=0 makro.c -o d && ./d | diff - $D/kutilgan_3.txt && echo "0: TO'G'RI"
```
<!-- loyiha:oxiri -->

Keyingi bob: [11-bob. Ko'p faylli dasturlar va Make](11-kop-fayl-make.md)
