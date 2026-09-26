# 10-bob. Preprotsessor: `#include`, `#define`, makrolar

> **Bu bobdan keyin:** `#` bilan boshlanadigan har bir qatorni, makrolarning tuzoqlarini,
> `do { } while (0)` nima uchun kerakligini, `#x` va `a##b` ni, shartli kompilyatsiyani bilasiz.
> Yadro kodining taxminan 10% i makrolar — ularni o'qiy olishingiz shart. Mashqlar: 23 va barcha `test.h`.

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

Keyingi bob: [11-bob. Ko'p faylli dasturlar va Make](11-kop-fayl-make.md)
