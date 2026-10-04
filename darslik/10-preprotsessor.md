# 10-bob. Preprotsessor: `#include`, `#define`, makrolar

> **Bu bobda nima o'rganasiz:** `#` bilan boshlanadigan har bir qator nima qilishini; `#define N 10` ni **qanday ishlatish**ni; makrolarning tuzoqlarini;
> `do { } while (0)` nima uchun kerakligini; `#x` va `a##b` ni; shartli kompilyatsiyani (`#ifdef`, `-DDEBUG`).
> Yadro kodining taxminan 10% i makrolar — ularni o'qiy olishingiz shart.
> **Oldindan nima kerak:** 1-bob (kompilyatsiya bosqichlari), 3-bob (bitlar), 5-bob (funksiyalar).   **Vaqt:** 5–6 soat.
> Mashqlar: 23 va barcha `test.h`.

> **To'liq ishlaydigan misol:** [misollar/10_makrolar.c](misollar/10_makrolar.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Bu bob nima haqida?

1-bobda ko'rdingiz: kompilyatsiyaning **birinchi bosqichi** — preprotsessor. U **C tilini tushunmaydi**; u shunchaki **matnni** almashtiradi va `#` bilan boshlangan qatorlarni bajaradi.
Kompilyator undan **almashtirilgan** matnni oladi.

**Hayotdan misol: "Topish va almashtirish" tugmasi.** Word'da "Tosh." so'zini hamma joyda "Toshkent" ga almashtirasiz. Preprotsessor ham shunday — faqat matn,
ma'no haqida o'ylamaydi. Shuning uchun uning tuzoqlari bor (pastda).

| Buyruq | Nima qiladi | Hayotdan |
|---|---|---|
| `#include <x.h>` | `x.h` faylini shu joyga **ko'chirib qo'yadi** | "bu yerga 3-ilovani qo'shing" |
| `#define N 10` | keyingi hamma `N` ni `10` ga **almashtiradi** | shartnoma boshidagi kelishuv: "'Ijarachi' = Aliyev Vali" |
| `#ifdef`, `#if` | kodning bir qismini **shartli** qo'shadi / olib tashlaydi | kitobning ikki nashri |
| `#x` makro ichida | argumentni **qo'shtirnoqqa** oladi | so'zni "..." ichiga solish |

## 10.1. Preprotsessor nima

`#` bilan boshlanadigan qatorlar uning buyruqlari; ular `;` bilan tugamaydi, **qator oxiri** bilan tugaydi (uzun buyruq qatorlarini `\` bilan davom ettirish mumkin).
Natijani ko'rish: `gcc -E fayl.c`.

**Bu dastur nima qiladi (umumiy):** preprotsessor `#define N 10` ni kodga qanday qo'yishini ko'rsatadi (so'zma-so'z almashtirish).

```c
/* almashtir.c - preprotsessor nima qilishini ko'rish */
#define N 10                            /* bundan keyin N so'zi 10 bo'ladi */

int massiv[N];                          /* int massiv[10]; */

int yig(void)
{
    int s = 0;
    for (int i = 0; i < N; i++)         /* i < 10 */
        s += i;
    return s;
}
```

```console
$ gcc -E almashtir.c | tail -9
int massiv[10];

int yig(void)
{
    int s = 0;
    for (int i = 0; i < 10; i++)
        s += i;
    return s;
}
```

**Nima ko'rdik:** `gcc -E` preprotsessordan **keyingi** matnni ko'rsatadi. `#define N 10` qatori yo'qoldi, `N` hamma joyda `10` bo'lib qoldi.
Kompilyator `N` degan so'zni **hech qachon ko'rmaydi** — u faqat `10` ni ko'radi.

> **Eslab qoling:** preprotsessor — **matn almashtirgich**. Natijani tekshirish uchun `gcc -E`.

## 10.2. `#include`

```c
#include <stdio.h>          /* tizim sarlavhasi: -isystem papkalari, /usr/include */
#include "fs/vfs.h"         /* loyiha sarlavhasi: avval joriy papka, keyin -I papkalari */
```

Fayl mazmuni aynan shu joyga ko'chiriladi. Shuning uchun `.h` faylda faqat **e'lonlar**, makrolar, tur ta'riflari va `static inline` funksiyalar bo'lishi kerak —
oddiy funksiya ta'riflari emas (1-bob). Qaysi sarlavha nima uchun — [sarlavhalar.md](sarlavhalar.md).

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

Ma'nosi: "agar `VFS_H` hali aniqlanmagan bo'lsa (`#ifndef`) — uni aniqla va fayl mazmunini qo'sh; ikkinchi marta kelganda `VFS_H` allaqachon bor, shuning uchun hamma narsa o'tkazib yuboriladi".

## 10.3. `#define` — o'zgarmaslar

**Hayotdan misol: shartnoma boshidagi kelishuv.** Shartnomalarda boshida yoziladi: "Bundan buyon 'Ijarachi' deganda 'Aliyev Vali' tushuniladi". Keyin matnda faqat 'Ijarachi' yoziladi.
Ijarachi almashsa — faqat bitta qatorni o'zgartirasiz.

```c
#define PAGE_SIZE 4096
#define PAGE_MASK (~(PAGE_SIZE - 1))
```

Preprotsessor keyingi hamma `PAGE_SIZE` so'zlarini `4096` ga almashtiradi. Endi **qanday ishlatish**ni ko'rsatamiz — `#define N 10` ning to'liq hayoti:

```c
/* define_asos.c - #define N 10 ni ishlatish */
#include <stdio.h>

#ifndef N                               /* agar buyruq qatoridan berilmagan bo'lsa */
#define N 10                            /* massiv o'lchami - FAQAT SHU YERDA */
#endif

int main(void)
{
    int kvadrat[N];                     /* N ta element */

    for (int i = 0; i < N; i++)         /* N marta takrorlash */
        kvadrat[i] = i * i;

    printf("N = %d, oxirgi element: kvadrat[%d] = %d\n", N, N - 1, kvadrat[N - 1]);
    printf("massiv hajmi: %zu bayt (%d x %zu)\n", sizeof(kvadrat), N, sizeof(int));
    return 0;
}
```

```console
$ gcc -Wall -Wextra define_asos.c -o define_asos
$ ./define_asos
N = 10, oxirgi element: kvadrat[9] = 81
massiv hajmi: 40 bayt (10 x 4)
$ gcc -Wall -Wextra -DN=5 define_asos.c -o define_asos5
$ ./define_asos5
N = 5, oxirgi element: kvadrat[4] = 16
massiv hajmi: 20 bayt (5 x 4)
```

**Kodda nimalar bor:**

| Qator | Nima qiladi | Nega |
|---|---|---|
| `#ifndef N` ... `#endif` | "agar `N` hali berilmagan bo'lsa, quyidagini bajar" | `-DN=5` bilan boshqa qiymat berish imkoni |
| `#define N 10` | N ni 10 deb belgilaydi | **bitta joyda** o'zgartirasiz |
| `int kvadrat[N];` | `int kvadrat[10];` bo'ladi | massiv o'lchami |
| `i < N` | `i < 10` | sikl chegarasi — massiv bilan **doim mos** |
| `kvadrat[N - 1]` | oxirgi element | `N - 1` — oxirgi indeks (6.1) |
| `-DN=5` | buyruq qatoridan `#define N 5` ni berish | qayta yozmasdan o'zgartirish |

**Nega bu foydali?** `10` raqami kodda 20 joyda bo'lsa, o'zgartirishda birini unutasiz → massiv 5 ta, sikl 10 marta → **chegaradan chiqish** (6.2). `#define N` bilan hammasi
**bir joydan** boshqariladi. Natija: `N = 5` ni bergach, massiv ham, sikl ham, chop etish ham o'zi moslashdi.

Qoidalar:

- Nomlar KATTA harflarda (makro ekanini ko'rsatish uchun).
- Ifodali makroni **doim qavsga** oling. Nega:

**Bu dastur nima qiladi (umumiy):** qavssiz va qavsli makroning natijasi farqini ko'rsatadi: `1 + 1 * 3` va `(1 + 1) * 3`.

```c
/* qavs_tuzoq.c - qavssiz makro */
#include <stdio.h>

#define IKKI_YOMON 1 + 1
#define IKKI (1 + 1)

int main(void)
{
    printf("IKKI_YOMON * 3 = %d   (1 + 1 * 3 = 4, 6 emas!)\n", IKKI_YOMON * 3);
    printf("IKKI * 3       = %d\n", IKKI * 3);
    return 0;
}
```

```console
$ gcc -Wall -Wextra qavs_tuzoq.c -o qavs_tuzoq
$ ./qavs_tuzoq
IKKI_YOMON * 3 = 4   (1 + 1 * 3 = 4, 6 emas!)
IKKI * 3       = 6
```

Preprotsessor `IKKI_YOMON * 3` ni so'zma-so'z `1 + 1 * 3` ga almashtirdi — ko'paytirish oldin bajarildi. Odam qavs qo'yardi, preprotsessor — yo'q.

- Oxirida **`;` qo'ymang**:

**Bu dastur nima qiladi (umumiy):** `#define` oxiriga `;` qo'yilsa kod buzilishini ko'rsatadi — ataylab xatoli.

```c
/* nuqtali_define.c - #define oxirida ; */
#define N 10;

int a[N];                               /* int a[10;]; -> xato */

int main(void)
{
    return 0;
}
```

```console
$ gcc -Wall -Wextra -c nuqtali_define.c # xato kutiladi
nuqtali_define.c:2:13: error: expected ‘]’ before ‘;’ token
    2 | #define N 10;
      |             ^
nuqtali_define.c:4:7: note: in expansion of macro ‘N’
    4 | int a[N];                               /* int a[10;]; -> xato */
      |       ^
```

Xato **ishlatilgan** joyda (`int a[N];`) chiqadi, `#define` qatorida emas — chunki almashtirishdan keyin `int a[10;];` bo'ldi.

`const` va `enum` bilan solishtirish: `enum { PAGE_SIZE = 4096 };` debuggerda ko'rinadi va tur tekshiriladi; lekin `#define` `#if` da ishlatilishi mumkin va istalgan turdagi (masalan, `64 bit`)
o'zgarmas bo'la oladi. Yadroda ikkalasi ham ishlatiladi.

## 10.4. Funksiyaga o'xshash makrolar

```c
#define KVADRAT(x) ((x) * (x))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
```

Parametrli makro — "matn shabloni": `KVADRAT(5)` → `((5) * (5))`. Har bir parametrni va butun ifodani **qavsga** oling:

**Bu dastur nima qiladi (umumiy):** funksiyaga o'xshash makrolarning ikki tuzog'i: argument atrofida qavs yo'qligi va argumentning ikki marta hisoblanishi.

```c
/* makro_tuzoq.c - qavs va ikki marta hisoblash */
#include <stdio.h>

#define YOMON_KVADRAT(x) x * x
#define KVADRAT(x) ((x) * (x))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

int main(void)
{
    int a = 2;
    printf("YOMON_KVADRAT(a + 1) = %d  (a + 1 * a + 1 = 5, to'g'ri 9 emas)\n", YOMON_KVADRAT(a + 1));
    printf("KVADRAT(a + 1)       = %d\n", KVADRAT(a + 1));

    int i = 5;
    int m = MIN(i++, 10);               /* ((i++) < (10) ? (i++) : (10)) */
    printf("MIN(i++, 10) = %d, i = %d  (i ikki marta oshdi!)\n", m, i);
    return 0;
}
```

```console
$ gcc -Wall -Wextra makro_tuzoq.c -o makro_tuzoq
$ ./makro_tuzoq
YOMON_KVADRAT(a + 1) = 5  (a + 1 * a + 1 = 5, to'g'ri 9 emas)
KVADRAT(a + 1)       = 9
MIN(i++, 10) = 6, i = 7  (i ikki marta oshdi!)
```

**Qadamlar:**

- `YOMON_KVADRAT(a + 1)` → `a + 1 * a + 1` → `2 + 2 + 1` = **5** (9 kutilgan edi).
- `MIN(i++, 10)` → `((i++) < (10) ? (i++) : (10))`: shart tekshirilganda `i++` (5 < 10 rost, `i` = 6), keyin natija uchun yana `i++` → `m = 6`, `i = 7`. `i` **ikki marta** oshdi.

**Ikki marta hisoblash tuzog'i:** makro — **matn almashtirish**, funksiya emas: argument har ishlatilgan joyda qayta hisoblanadi. `MIN(sekin_funksiya(), 5)` — funksiya **ikki marta** chaqiriladi.
Shuning uchun imkon bo'lsa `static inline` funksiya yozing (5-bob). Makro faqat funksiya qila olmaydigan narsa uchun: tur bo'yicha umumiy kod, `__FILE__`/`__LINE__`, `#x`, `container_of`.

> **Eslab qoling:** makro parametrini va butun natijani **qavsga** oling: `((x) * (x))`. Argumentda `i++` yoki funksiya chaqiruvi bo'lmasin.

## 10.5. `do { ... } while (0)` — ko'p buyruqli makro

Bir nechta buyruqdan iborat makro yozamiz:

```c
#define XATO(msg) printf("xato: %s\n", msg); xatolar++
```

`if (x < 0) XATO("manfiy");` ochilganda: `if (x < 0) printf(...); xatolar++;` — `xatolar++` `if` ga **tegishli emas**, doim bajariladi (4-bobdagi `{ }` tuzog'i).

`{ }` bilan o'rash ham to'liq yechim emas:

**Bu dastur nima qiladi (umumiy):** `{ }` bilan o'ralgan makro `if/else` ichida `;` tufayli sintaksis xatosi berishini ko'rsatadi — ataylab xatoli.

```c
/* makro_skobka.c - { } bilan o'ralgan makro va else */
#include <stdio.h>

#define XATO(msg) { printf("xato: %s\n", msg); }

int main(void)
{
    int x = -1;
    if (x < 0)
        XATO("manfiy");                 /* { ... }; - oxiridagi ; bo'sh buyruq */
    else
        printf("musbat\n");
    return 0;
}
```

```console
$ gcc -Wall -Wextra makro_skobka.c -o makro_skobka # xato kutiladi
makro_skobka.c: In function ‘main’:
makro_skobka.c:11:5: error: ‘else’ without a previous ‘if’
   11 |     else
      |     ^~~~
```

`{ ... };` dan keyingi `;` — `if` ni tugatadi, keyingi `else` esa "yolg'iz" qoladi (`'else' without a previous 'if'`).

To'g'ri yechim — `do { ... } while (0)`:

**Bu dastur nima qiladi (umumiy):** `do { ... } while (0)` bilan o'ralgan makro `if/else` ichida to'g'ri ishlashini ko'rsatadi.

```c
/* makro_dowhile.c - do { } while (0) */
#include <stdio.h>

static int xatolar;

#define XATO(msg)                          \
    do {                                   \
        printf("xato: %s\n", msg);         \
        xatolar++;                         \
    } while (0)

int main(void)
{
    int x = -1;
    if (x < 0)
        XATO("manfiy");                    /* bitta buyruq, oxiridagi ; bilan to'g'ri */
    else
        printf("musbat\n");
    printf("xatolar = %d\n", xatolar);
    return 0;
}
```

```console
$ gcc -Wall -Wextra makro_dowhile.c -o makro_dowhile
$ ./makro_dowhile
xato: manfiy
xatolar = 1
```

Bu **bitta buyruq**: `do { ... } while (0);` — tana aniq bir marta bajariladi, oxiridagi `;` buyruqni tugatadi va `if/else` ichida to'g'ri ishlaydi. Linux yadrosida va mashqlardagi
`test.h` da (`CHECK`, `CHECK_INT`) hamma ko'p qatorli makrolar shunday. `\` — "makro keyingi qatorda davom etadi" (undan keyin bo'shliq ham bo'lmasligi kerak).

> **Eslab qoling:** ko'p buyruqli makro = `do { ... } while (0)`.

## 10.6. `#` va `##` operatorlari

**Hayotdan misol: so'zni qo'shtirnoqqa olish.** Makro ifodani ham hisoblaydi, ham uning **yozilishini** satr sifatida chiqaradi: `TEKSHIR(a > 0)` xato bo'lsa "a > 0 bajarilmadi" deb yoza oladi.

```c
/* stringlash.c - #x, __FILE__, __LINE__ va a##b */
#include <stdio.h>

#define SATR(x) #x                      /* argumentni SATRGA aylantirish */
#define CHECK(shart) \
    do { if (!(shart)) printf("XATO: %s (%s:%d)\n", #shart, __FILE__, __LINE__); } while (0)
#define YOPISHTIR(a, b) a##b            /* ikki so'zni BITTA so'zga yopishtirish */

int main(void)
{
    printf("SATR(a + b) = %s\n", SATR(a + b));

    int x = -3;
    CHECK(x > 0);                       /* shart yolg'on: matni va joyi chiqadi */

    int YOPISHTIR(ozgaruvchi, 1) = 5;   /* int ozgaruvchi1 = 5; */
    printf("ozgaruvchi1 = %d\n", ozgaruvchi1);
    return 0;
}
```

```console
$ gcc -Wall -Wextra stringlash.c -o stringlash
$ ./stringlash
SATR(a + b) = a + b
XATO: x > 0 (stringlash.c:14)
ozgaruvchi1 = 5
```

- `#shart` — shart **matnini** `"x > 0"` ga aylantiradi: funksiya buni qila olmaydi (u faqat qiymatni ko'radi).
- `__FILE__`, `__LINE__` — makro **chaqirilgan** joyning fayli va qatori. Funksiya ichida `__LINE__` funksiyaning qatorini bergan bo'lardi.
- `a##b` — ikki tokenni bittaga yopishtiradi (`ozgaruvchi` + `1` → `ozgaruvchi1`).

Mashqlardagi `test.h` va MyOS'ning `kernel/tests/selftest.c` xato bo'lganda shartning **matnini** aynan `#shart` bilan chiqaradi.

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

**Bu dastur nima qiladi (umumiy):** oldindan aniqlangan makrolarni (`__FILE__`, `__LINE__`, `__func__`, `__x86_64__`) chiqaradi.

```c
/* oldindan.c - oldindan aniqlangan makrolar */
#include <stdio.h>

static void qayerdaman(void)
{
    printf("fayl: %s, qator: %d, funksiya: %s\n", __FILE__, __LINE__, __func__);
}

int main(void)
{
    qayerdaman();
#ifdef __x86_64__
    printf("platforma: x86-64\n");
#endif
#ifdef __linux__
    printf("tizim: Linux\n");
#endif
    return 0;
}
```

```console
$ gcc -Wall -Wextra oldindan.c -o oldindan
$ ./oldindan
fayl: oldindan.c, qator: 6, funksiya: qayerdaman
platforma: x86-64
tizim: Linux
```

## 10.8. Shartli kompilyatsiya

**Hayotdan misol: kitobning ikki nashri.** Darslikning o'qituvchi nashrida javoblar bor, o'quvchi nashrida yo'q. Matn bitta, faqat ba'zi sahifalar bitta nashrga kiradi.
`-DDEBUG` bilan yig'sangiz — "o'qituvchi nashri" (qo'shimcha xabarlar bilan), usiz — oddiy nashr.

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

**Bu dastur nima qiladi (umumiy):** `#error` bilan qo'llab-quvvatlanmaydigan platformada yig'ishni to'xtatadi — ataylab xatoli (bu mashinada).

```c
/* arxitektura.c - qo'llab-quvvatlanmaydigan platforma */
#if defined(__aarch64__)
int arxitektura = 1;
#else
#error "Bu arxitektura qo'llab-quvvatlanmaydi"
#endif

int main(void)
{
    return 0;
}
```

```console
$ gcc -Wall -Wextra -c arxitektura.c # xato kutiladi
arxitektura.c:5:2: error: #error "Bu arxitektura qo'llab-quvvatlanmaydi"
    5 | #error "Bu arxitektura qo'llab-quvvatlanmaydi"
      |  ^~~~~
```

Mashqlarda: `libctest.c` dagi `#ifndef HOST_TEST` — bir xil test faylini yadroda ham, kompyuterda ham ishlatish uchun (`tools/host_libctest.sh` `-DHOST_TEST` bilan kompilyatsiya qiladi).

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

`!!x` — istalgan sonni 0 yoki 1 ga aylantirish (ikki marta "EMAS"). MyOS'da bularning ko'pi `kernel/lib/common.h` da (u yerda `ALIGN_UP` `ALIGN_DOWN` orqali yozilgan va `__typeof__` ishlatadi) — ochib o'qing.

**Bu dastur nima qiladi (umumiy):** yadro dasturlaridagi tipik makrolarni (`ARRAY_SIZE`, `ALIGN_UP`, `ALIGN_DOWN`, `BIT`) ishlatib, natijalarini chiqaradi.

```c
/* yadro_makrolar.c - yadro makrolari amalda */
#include <stdio.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define ALIGN_UP(x, a)   (((x) + (a) - 1) & ~((a) - 1))
#define ALIGN_DOWN(x, a) ((x) & ~((a) - 1))
#define BIT(n) (1UL << (n))

int main(void)
{
    int sonlar[7];
    printf("ARRAY_SIZE = %zu\n", ARRAY_SIZE(sonlar));

    printf("ALIGN_UP(5000, 4096)   = %d  (keyingi sahifa chegarasi)\n", ALIGN_UP(5000, 4096));
    printf("ALIGN_DOWN(5000, 4096) = %d  (sahifa boshi)\n", ALIGN_DOWN(5000, 4096));
    printf("ALIGN_UP(4096, 4096)   = %d  (allaqachon tekis)\n", ALIGN_UP(4096, 4096));
    printf("BIT(3) = %lu, BIT(12) = %lu\n", BIT(3), BIT(12));
    return 0;
}
```

```console
$ gcc -Wall -Wextra yadro_makrolar.c -o yadro_makrolar
$ ./yadro_makrolar
ARRAY_SIZE = 7
ALIGN_UP(5000, 4096)   = 8192  (keyingi sahifa chegarasi)
ALIGN_DOWN(5000, 4096) = 4096  (sahifa boshi)
ALIGN_UP(4096, 4096)   = 4096  (allaqachon tekis)
BIT(3) = 8, BIT(12) = 4096
```

**`ALIGN_UP(5000, 4096)` qadamlari:** `5000 + 4095 = 9095`; `~(4095)` = pastki 12 bit nol (3-bobdagi niqob); `9095 & ~4095` = **8192** (4096 ga karrali eng yaqin katta son). Bu — xotirani
sahifalarga yaxlitlash (24-bob).

**Ro'yxat bo'ylab yurish makrosi** (MyOS: `kernel/lib/list.h`) — makro yangi "sikl sintaksisi" yaratadi:

```text
#define list_for_each_entry(pos, head, member)                                   \
    for (pos = list_entry((head)->next, __typeof__(*pos), member);               \
         &pos->member != (head);                                                 \
         pos = list_entry(pos->member.next, __typeof__(*pos), member))

struct vm_area *it;
list_for_each_entry(it, &areas, node) {     /* kernel/mm/vmalloc.c */
    ...                                     /* it - navbatdagi element */
}
```

`list_entry` — `container_of` ning boshqa nomi, `__typeof__(*pos)` — GCC kengaytmasi: "pos ko'rsatgan tur". Bitta makro istalgan turdagi obyektlar ro'yxatini aylanadi —
C'da shablon (template) yo'q, uning o'rnini makrolar bosadi.

## Hayotdan misol va to'liq dastur

**Narxlar jadvali.** Do'kon narxlarini QQS bilan chiqaramiz. Soliq stavkasi o'zgarsa — **bitta qator**. Dasturda: `#define` o'zgarmas, parametrli makrolar, `#x`, `do-while(0)`, `-DDEBUG`.

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

**Kodda nimalar bor:**

| Makro | Nima | Nega |
|---|---|---|
| `QQS_FOIZ` | `12` | stavka bitta joyda |
| `QQS_BILAN(narx)` | `((narx) + (narx) * 12 / 100)` | narxga soliqni qo'shadi; har `narx` qavsda |
| `SONI(massiv)` | massiv elementlari soni (6.1 formulasi) | `dokon` ga yangi mahsulot qo'shsangiz, sikl o'zi moslashadi |
| `TEKSHIR(shart)` | shart yolg'on bo'lsa, uning **matnini** chiqaradi | `#shart`: "dokon[i].narx > 0" |
| `LOG(xabar)` | `-DDEBUG` bilan — xabar; usiz — **hech narsa** (`do { } while (0)`) | debug xabarlari tayyor dasturda iz qoldirmaydi |

Oxirgi buyruq (`gcc -E ... | grep`) `QQS_BILAN(dokon[i].narx)` preprotsessordan keyin nimaga aylanganini ko'rsatadi — kompilyator aynan shu matnni oladi. `Choy` ning narxi manfiy bo'lgani uchun `TEKSHIR` ogohlantirdi.

**Sinab ko'ring:** `QQS_FOIZ` ni 15 qiling — faqat bitta qatorni o'zgartirdingiz. `QQS_BILAN` dan qavslarni olib tashlang: `#define QQS_BILAN(narx) narx + narx * QQS_FOIZ / 100` va uni `QQS_BILAN(1000) * 2` bilan
chaqiring — natija nega noto'g'ri?

## Bob xulosasi (yodlash uchun)

1. Preprotsessor — **kompilyatordan oldin** ishlaydigan matn almashtirgich; natijani `gcc -E` ko'rsatadi.
2. `#define NOM qiymat` — o'zgarmas (qiymatni bitta joyda saqlash); ifodani **qavsga**, oxiriga `;` **qo'ymang**; `-DNOM=5` bilan buyruq qatoridan berish mumkin.
3. Parametrli makro — matn shabloni: parametr va natijani qavsga oling; `i++` yoki funksiyani argument qilmang (**ikki marta hisoblanadi**).
4. Ko'p buyruqli makro — `do { ... } while (0)`; `#x` — qo'shtirnoqqa olish; `a##b` — yopishtirish; `__FILE__`/`__LINE__` — chaqirilgan joy.
5. `#ifdef` / `#if` / `#error` — shartli kompilyatsiya (`-DDEBUG`); imkon bo'lsa makro o'rniga `static inline`.

## Savol-javob

**Makro va `inline` funksiya — qaysi biri?**
Imkon bo'lsa — `static inline` (tur tekshiruvi, bir marta hisoblash, debuggerda ko'rinadi). Makro — faqat funksiya qila olmaydigan narsa uchun.

**Makro ochilganini qanday ko'raman?**
`gcc -E fayl.c | grep -A3 "qidirilgan_joy"`. Chalkash makro xatolarida bu eng yaxshi yo'l.

**Nega kompilyator xatosi makro ichidagi qatorni ko'rsatadi?**
Chunki xato ochilgan kodda. GCC `note: in expansion of macro 'X'` bilan qaysi makrodan kelganini aytadi.

**`const int N = 10;` bilan `#define N 10` farqi?**
`const` — haqiqiy o'zgaruvchi (xotirada joyi bor, turi bor), lekin C'da massiv o'lchami sifatida ishlatish har doim ham mumkin emas va `#if` da ishlamaydi. `#define` — faqat matn, joy olmaydi, hamma joyda ishlaydi.

## O'zingizni tekshiring

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

## Mashq

- `mashqlar/test.h` ni to'liq o'qing: har bir makro nima qilishini va nega `do { } while (0)` ichida ekanini tushuntiring.
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
