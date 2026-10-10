# 8-bob. Xotira: stek, heap, statik

> **Bu bobda nima o'rganasiz:** dastur xotirasi qanday bo'linganini (stek, heap, statik); `malloc` / `free` / `realloc` ni to'g'ri ishlatishni;
> "egalik" (kim `free` qiladi) qoidasini; xotira xatolarini (leak, use-after-free, double free) topishni.
> **Oldindan nima kerak:** 5-, 6-, 7-boblar (funksiya, massiv, ko'rsatkich).   **Vaqt:** 6–8 soat.
> Mashqlar: isitish (bob oxirida), 14, 15, 18. 13, 16, 17 — 9-bobdan, 30 — 25-bobdan keyin.

> **To'liq ishlaydigan misol:** [misollar/08_xotira.c](misollar/08_xotira.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Bu bob nima haqida?

Hozirgacha massivlar o'lchamini oldindan yozdik (`int a[100]`). Lekin dastur ishlayotganda necha mehmon kelishini, fayl necha bayt ekanini
oldindan bilmaymiz. **Dinamik xotira** — dastur ishlayotgan paytda "menga yana 400 bayt kerak" deb so'rash (`malloc`) va ishlatib bo'lgach qaytarish (`free`).
Python'da buni "axlatchi" (garbage collector) o'zi qiladi; C'da **siz** qilasiz — shuning uchun xotira xatolari C'ning eng katta og'rig'i.

**Hayotdan misol: uch xil joy.**

| Joy | Hayotdan | Xususiyati |
|---|---|---|
| **Stek** | **ish stoli** | Funksiya ishlayotganda qog'ozlarini stolga yoyadi, ish tugadi — stol **avtomatik tozalanadi**. Tez, lekin kichik va qisqa muddatli |
| **Heap** | **ijaraga olinadigan ombor** | Katta yoki uzoq saqlanadigan narsalar uchun. O'zingiz so'raysiz (`malloc`), o'zingiz qaytarasiz (`free`). Hech kim siz uchun qaytarmaydi |
| **Statik xotira** | bino devoriga o'rnatilgan shkaf | Dastur boshidan oxirigacha turadi (global va `static` o'zgaruvchilar) |

## 8.1. Jarayon xotirasining xaritasi

Linux'da (va MyOS'da) har bir dastur o'z virtual manzil maydonini ko'radi:

```text
yuqori manzillar
 ┌─────────────────────┐ 0x7fff...
 │ STEK                │  lokal o'zgaruvchilar, funksiya kadrlari; PASTGA o'sadi
 │   ↓                 │
 │                     │
 │   ↑                 │
 │ HEAP                │  malloc/free; YUQORIGA o'sadi (brk/sbrk yoki mmap)
 ├─────────────────────┤
 │ .bss                │  boshlang'ich qiymatsiz global/static (avtomatik 0)
 │ .data               │  boshlang'ich qiymatli global/static
 │ .rodata             │  o'zgarmaslar, satr literallari (faqat o'qish)
 │ .text               │  mashina kodi (faqat o'qish + bajarish)
 └─────────────────────┘ 0x400000 atrofida
quyi manzillar (0-sahifa ataylab bo'sh - NULL ushlanishi uchun)
```

Stek va heap bir-biriga qarab o'sadi: stek pastga, heap yuqoriga. Ular o'rtasidagi bo'sh joy katta (virtual manzillar maydoni juda keng).

MyOS'da bu xaritani yadro yaratadi: ELF faylidan `.text/.data/.bss` ni yuklash — `kernel/sys/elf.c`, stek va heap uchun hududlar (VMA) — `kernel/mm/mm.c`, `sbrk` syscall'i heap'ni o'stiradi.

## 8.2. Uch xil "saqlash muddati"

| Tur | Qayerda | Qachon yaratiladi | Qachon yo'qoladi | Misol |
|---|---|---|---|---|
| **Avtomatik** | stek | funksiyaga kirganda | funksiyadan chiqqanda | `int x;` funksiya ichida |
| **Statik** | .data / .bss | dastur boshlanganda | dastur tugaganda | global, `static` o'zgaruvchi |
| **Dinamik** | heap | `malloc` chaqirilganda | `free` chaqirilganda | `malloc(100)` |

### Statik o'zgaruvchilar

**Bu dastur nima qiladi (umumiy):** global, funksiya ichidagi `static` va oddiy lokal o'zgaruvchilarning yashash muddati farqini ko'rsatadi.

```c
/* statik.c - global va static o'zgaruvchi */
#include <stdio.h>

int hisob = 0;                  /* global: butun dastur davomida yashaydi, avtomatik 0 */

static void chaqir(void)
{
    static int necha_marta = 0; /* FUNKSIYA ichidagi static: qiymati chaqiruvlar orasida SAQLANADI */
    int oddiy = 0;              /* stekda: har chaqiruvda yangi */
    necha_marta++;
    oddiy++;
    hisob += 10;
    printf("chaqiruv %d: oddiy = %d, hisob = %d\n", necha_marta, oddiy, hisob);
}

int main(void)
{
    chaqir();
    chaqir();
    chaqir();
    return 0;
}
```

```console
$ gcc -Wall -Wextra statik.c -o statik
$ ./statik
chaqiruv 1: oddiy = 1, hisob = 10
chaqiruv 2: oddiy = 1, hisob = 20
chaqiruv 3: oddiy = 1, hisob = 30
```

**Nima ko'rdik:**

- `necha_marta` har chaqiruvda 1, 2, 3 ga oshdi — u **stekda emas**, statik xotirada (`.data`) yashaydi va chaqiruvlar orasida saqlanadi.
- `oddiy` har safar 1 — u stekda, funksiya tugaganda yo'qoldi.
- `hisob` (global) — hamma funksiyalarga ko'rinadi va dastur tugaguncha yashaydi.

- Global va `static` o'zgaruvchilar **avtomatik 0** bilan boshlanadi (lokal o'zgaruvchilardan farqli).
- `static` so'zining **ikki ma'nosi**: fayl darajasida — "faqat shu faylda ko'rinadi" (5-bob); funksiya ichida — "stekda emas, statik xotirada yashaydi".
- Yadroda globallar ko'p: `static struct list_head disks;`, `static uint64_t ticks;` — lekin ko'p yadroli tizimda ularni **qulf bilan** himoya qilish kerak (15-bob).

## 8.3. `malloc` va `free`

**Hayotdan misol: mehmonxona xonasi.** Qabulxonadan xona so'raysiz (`malloc`) — kalit (ko'rsatkich) olasiz. Ketayotganda kalitni qaytarasiz (`free`).
Mehmonxonada bo'sh xona qolmasa — `malloc` `NULL` qaytaradi, buni doim tekshiring.

```c
/* malloc_asos.c - xotira so'rash va qaytarish */
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int soni = 5;
    int *a = malloc(soni * sizeof(int));        /* 5 ta int uchun joy (20 bayt) */
    if (a == NULL) {                            /* xotira berilmadi */
        perror("malloc");
        return 1;
    }

    for (int i = 0; i < soni; i++)
        a[i] = (i + 1) * 100;                   /* oddiy massiv kabi ishlatamiz */

    for (int i = 0; i < soni; i++)
        printf("a[%d] = %d\n", i, a[i]);

    free(a);                                    /* qaytarish */
    a = NULL;                                   /* ixtiyoriy, lekin foydali odat */
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address malloc_asos.c -o malloc_asos
$ ./malloc_asos
a[0] = 100
a[1] = 200
a[2] = 300
a[3] = 400
a[4] = 500
```

**Kodda nimalar bor:**

| Qator | Nima qiladi | Nega |
|---|---|---|
| `#include <stdlib.h>` | `malloc`, `free` e'lonlarini beradi | [sarlavhalar.md](sarlavhalar.md) |
| `malloc(soni * sizeof(int))` | **bayt** so'raydi: 5 × 4 = 20 bayt | `malloc(n)` — **n bayt**, elementlar soni emas! Doim `soni * sizeof(tur)` |
| `int *a = ...` | `malloc` qaytargan manzil `a` da saqlanadi | `a` — ko'rsatkich (7-bob). Endi `a[i]` ishlaydi (`a[i]` = `*(a+i)`) |
| `if (a == NULL)` | xotira berilmagan holat | `malloc` joy topmasa `NULL` qaytaradi — **tekshiring** |
| `free(a)` | qaytarish: "kalitni topshirdim" | Har bir `malloc` ga aniq **bitta** `free` |
| `a = NULL` | endi `a` hech qayerga ko'rsatmaydi | Tasodifan `free` qilingan xotirani ishlatmaslik uchun |

- Qaytgan xotira **nollanmagan** (axlat). Nol kerak bo'lsa — `calloc(soni, hajm)`.
- `free(NULL)` — hech narsa qilmaydi (xavfsiz).
- Sanitizer hech narsa demadi — demak, hamma xotira to'g'ri qaytarilgan.

### `realloc` — hajmni o'zgartirish

**Hayotdan misol: kattaroq kvartiraga ko'chish.** Oila kattalashdi — kattaroq kvartira kerak. Narsalar yangi joyga ko'chiriladi, eski kvartira bo'shatiladi.
**Eski manzil endi yaroqsiz** — `realloc` dan keyin faqat yangi ko'rsatkichdan foydalaning.

```c
int *yangi = realloc(a, 200 * sizeof(int));
if (!yangi) {
    /* a hali ham haqiqiy - uni yo'qotmaslik uchun vaqtinchalik o'zgaruvchi */
    free(a);
    return -1;
}
a = yangi;
```

`realloc` blokni joyida kattalashtiradi yoki yangi joyga **ko'chiradi** (eski mazmun saqlanadi, eski manzil yaroqsiz bo'ladi).
**Tuzoq:** `a = realloc(a, ...)` — `NULL` qaytsa, eski blok manzili yo'qoladi (leak). Shuning uchun natijani avval **vaqtinchalik** `yangi` ga yozamiz. 13-mashq.

### `malloc` ichida nima bor

`malloc` — sehr emas, oddiy C kodi: katta xotira hududini (OS'dan `sbrk`/`mmap` bilan olingan) bloklarga bo'lib beradi. Har bir blok oldida kichik **sarlavha** (hajm, bo'shmi)
turadi. `free` blokni "bo'sh" deb belgilaydi va qo'shni bo'sh bloklar bilan birlashtiradi. 30-mashqda xuddi shuni o'zingiz yozasiz, MyOS'da esa `user/libc/malloc.c` (va malloc lab'i).

Shundan muhim xulosa: `malloc` qaytargan blokdan **tashqariga** yozsangiz, keyingi blokning sarlavhasini buzasiz. Xato keyingi `malloc`/`free` da, **boshqa joyda** chiqadi —
shuning uchun bunday xatolarni topish qiyin.

> **Eslab qoling:** `p = malloc(soni * sizeof(*p));` → `NULL` ni tekshir → ishlat → `free(p);`. `realloc` natijasini avval vaqtinchalik o'zgaruvchiga yozing.

## 8.4. Egalik (ownership) — kim `free` qiladi?

**Hayotdan misol: kim kalitni qaytaradi.** Xonani kim olgan bo'lsa, o'sha qaytaradi. Do'stingizga kalitni berib yuborsangiz, kim qaytarishini aniq kelishib oling.
Aks holda yo ikkalangiz ham qaytarmaysiz (leak), yo ikkalangiz ham qaytarasiz (double free).

C'da garbage collector yo'q, shuning uchun har bir ajratilgan xotiraning **egasi** bo'lishi kerak — oxirida uni `free` qiladigan kod. Qoida funksiya hujjatida yoziladi:

```text
/* Yangi satr qaytaradi. Chaqiruvchi uni free() qilishi SHART. */
char *birlashtir(const char *a, const char *b);

/* s ning ichiga ko'rsatkich qaytaradi. free() QILMANG - bu s ning bir qismi. */
const char *strchr(const char *s, int c);
```

Tipik uslublar:

- **"Yaratuvchi-yo'q qiluvchi" juftligi:** `xesh_yarat` / `xesh_ozod`, `fopen` / `fclose`, `ajrat` / `ajrat_ozod` (15, 17-mashqlar).
- **Havolalar sanog'i (refcount):** obyektni bir nechta joy ishlatsa, har biri sanoqni oshiradi, tugatganda kamaytiradi; 0 bo'lganda obyekt yo'q qilinadi. MyOS'da: `struct file`
  dagi `refcount` (`fork` va `dup` bitta faylni bo'lishadi), inode keshi. Linux'da `kref`.

## 8.5. Xotira xatolari — to'liq katalog

**Hayotdan misol: mehmonxonadagi tartibbuzarliklar.**

- **Leak (sizib chiqish)** — xonadan chiqib ketdingiz, kalitni qaytarmadingiz. Xona abadiy band. Serverda har soniyada bitta shunday xona — bir necha kundan keyin mehmonxonada joy qolmaydi.
- **Use-after-free** — kalitni qaytarib, keyin yashirin nusxasi bilan yana xonaga kirdingiz. U yerda endi boshqa mehmon yashaydi — uning narsalarini buzasiz.
- **Double free** — bitta kalitni ikki marta qaytarish. Qabulxona chalkashib, bitta xonani ikki mehmonga beradi.

| Xato | Kod | Oqibat | Sanitizer xabari |
|---|---|---|---|
| **Leak** (sizib chiqish) | `malloc` → `free` yo'q | xotira asta-sekin tugaydi | `detected memory leaks` |
| **Use-after-free** | `free(p); p->x = 1;` | boshqa obyektni buzish, xavfsizlik teshigi | `heap-use-after-free` |
| **Double free** | `free(p); free(p);` | allocator tuzilmalari buziladi | `attempting double-free` |
| **Heap overflow** | `malloc(n)` va `p[n] = 0` | qo'shni blok/sarlavha buziladi | `heap-buffer-overflow` |
| **Stack overflow (bufer)** | `char b[8]; strcpy(b, uzun);` | qaytish manzili buziladi | `stack-buffer-overflow` |
| **Boshlanmagan o'qish** | `int *p = malloc(4); printf("%d", *p);` | tasodifiy natija | (MemorySanitizer/Valgrind) |
| **`free` noto'g'ri manzilga** | `free(p + 1);` yoki stek manziliga | qulash | `attempting free on address which was not malloc()-ed` |
| **Noto'g'ri hajm** | `malloc(n)` o'rniga `malloc(n * sizeof(int))` kerak edi | overflow | `heap-buffer-overflow` |

Har birini amalda ko'ramiz (sanitizer qaysi qatorni ko'rsatishiga e'tibor bering):

**Bu dastur nima qiladi (umumiy):** `malloc` bilan olingan xotirani `free` qilmaydi — xotira sizib chiqishi (ataylab xatoli).

```c
/* leak.c - kalit qaytarilmadi */
#include <stdlib.h>

int main(void)
{
    int *p = malloc(40);                /* 40 bayt olindi */
    p[0] = 1;
    return 0;                           /* free(p) YO'Q */
}
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address leak.c -o leak
$ ./leak 2>&1 | grep -E 'ERROR|SUMMARY|leak of' | sed -E 's/==[0-9]+==//'
ERROR: LeakSanitizer: detected memory leaks
Direct leak of 40 byte(s) in 1 object(s) allocated from:
SUMMARY: AddressSanitizer: 40 byte(s) leaked in 1 allocation(s).
```

**Bu dastur nima qiladi (umumiy):** `free` dan keyin xotiradan o'qiydi (use-after-free) — ataylab xatoli.

```c
/* uaf.c - free dan keyin ishlatish */
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int *p = malloc(sizeof(int));
    *p = 7;
    free(p);
    printf("%d\n", *p);                 /* XATO: free qilingan xotira */
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address uaf.c -o uaf # xato kutiladi
uaf.c: In function ‘main’:
uaf.c:10:5: warning: pointer ‘p’ used after ‘free’ [-Wuse-after-free]
   10 |     printf("%d\n", *p);                 /* XATO: free qilingan xotira */
      |     ^~~~~~~~~~~~~~~~~~
uaf.c:9:5: note: call to ‘free’ here
    9 |     free(p);
      |     ^~~~~~~
$ ./uaf 2>&1 | grep -E 'ERROR|READ of size|#0' | head -3 | sed -E 's/==[0-9]+==//; s/0x[0-9a-f]+/0x.../g; s/ in main .*uaf/ in main uaf/'
ERROR: AddressSanitizer: heap-use-after-free on address 0x... at pc 0x... bp 0x... sp 0x...
READ of size 4 at 0x... thread T0
    #0 0x... in main uaf.c:10
```

**Bu dastur nima qiladi (umumiy):** bitta ko'rsatkichni ikki marta `free` qiladi (double free) — ataylab xatoli.

```c
/* ikki_free.c - bitta kalit ikki marta */
#include <stdlib.h>

int main(void)
{
    int *p = malloc(sizeof(int));
    free(p);
    free(p);                            /* XATO: double free */
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address ikki_free.c -o ikki_free # xato kutiladi
ikki_free.c: In function ‘main’:
ikki_free.c:8:5: warning: pointer ‘p’ used after ‘free’ [-Wuse-after-free]
    8 |     free(p);                            /* XATO: double free */
      |     ^~~~~~~
ikki_free.c:7:5: note: call to ‘free’ here
    7 |     free(p);
      |     ^~~~~~~
$ ./ikki_free 2>&1 | grep -E 'ERROR' | sed -E 's/==[0-9]+==//; s/0x[0-9a-f]+/0x.../g'
ERROR: AddressSanitizer: attempting double-free on 0x... in thread T0:
```

**Bu dastur nima qiladi (umumiy):** 4 elementli heap massivning 5-elementiga yozadi (`p[4]`) — ataylab xatoli.

```c
/* heap_toshish.c - malloc(n) va p[n] */
#include <stdlib.h>

int main(int argc, char **argv)
{
    (void)argv;
    int *p = malloc(4 * sizeof(int));   /* p[0]..p[3] */
    p[3 + argc] = 1;                    /* XATO: p[4] mavjud emas */
    free(p);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address heap_toshish.c -o heap_toshish
$ ./heap_toshish 2>&1 | grep -E 'ERROR|WRITE of size' | sed -E 's/==[0-9]+==//; s/0x[0-9a-f]+/0x.../g'
ERROR: AddressSanitizer: heap-buffer-overflow on address 0x... at pc 0x... bp 0x... sp 0x...
WRITE of size 4 at 0x... thread T0
```

**Nima ko'rdik:** `uaf.c` va `ikki_free.c` ni yig'ishda GCC o'zi ogohlantirdi (`used after 'free'`) — oddiy holatlarni kompilyator ham ko'radi. Har bir xato turi sanitizer'da **o'z nomi** bilan ushlandi. Sanitizer yig'ish vaqtida qo'shimcha tekshiruv kodini qo'shadi — shuning uchun o'rganishda doim yoqing.

### Leak'lar yadroda nega o'ldiradi

User dasturdagi leak dastur tugaganda yo'qoladi (OS hamma xotirani qaytarib oladi). **Yadro esa hech qachon tugamaydi**: har bir syscall'da 64 bayt sizib chiqsa,
bir necha soatda xotira tugaydi. Shuning uchun yadroda xatodan keyin tozalash (`goto` — 4-bob) juda muhim.

### Use-after-free yadroda nega eng xavfli

Bo'shatilgan xotira tezda boshqa obyektga beriladi. Eski ko'rsatkich orqali yozish — **boshqa** obyektni (masalan, boshqa jarayonning credentials'ini) o'zgartiradi.
Zamonaviy yadro hujumlarining katta qismi — use-after-free. MyOS'dagi slab allocator shuning uchun `free` qilingan obyektni "zahar" bilan to'ldiradi va keyingi `alloc` da tekshiradi
(`kernel/mm/slab.c`; `APPEND=demo=uaf` bilan ko'ring).

## 8.6. Xatolarni topish vositalari

**Hayotdan misol: mehmonxona inspektori.** Har bir kalit berilishi va qaytarilishini yozib boradi. Dastur tugaganda "3-xona kaliti qaytarilmadi, 12-qatorda olingan" deb hisobot beradi.

**AddressSanitizer** (mashqlarda doim yoqilgan):

```bash
gcc -g -fsanitize=address,undefined dastur.c -o dastur && ./dastur
```

Hisobotni o'qish (kesilgan namuna):

```text
==1234==ERROR: AddressSanitizer: heap-use-after-free on address 0x602000000010   <- XATO TURI
READ of size 8 at 0x602000000010 thread T0
    #0 0x55d1 in royxat_ozod yechim.c:80            <- XATO SHU YERDA (eng muhim qator)
    #1 0x55d2 in main test.c:45                     <- kim chaqirgan
freed by thread T0 here:
    #0 ... in free
    #1 0x55d3 in royxat_ozod yechim.c:81            <- qayerda free qilingan
previously allocated by thread T0 here:
    #1 0x55d4 in main test.c:40                     <- qayerda ajratilgan
```

Uch savolga javob beradi: **nima** xato (`heap-use-after-free`), **qayerda** (birinchi `#0` qatori), **kim** `free` qilgan va **kim** ajratgan. 18-mashq — aynan shu hisobotlarni o'qib, 5 ta xatoni topish.

**Valgrind** (qayta kompilyatsiyasiz): `valgrind --leak-check=full ./dastur`.

## 8.7. Stek va heap — qachon qaysi biri

| | Stek | Heap |
|---|---|---|
| Tezlik | juda tez (faqat `rsp` ni surish) | sekinroq (allocator ishlaydi) |
| Hajm | cheklangan (user: 8 MB, **yadro: 8–16 KB**) | katta |
| Yashash muddati | funksiya tugaguncha | `free` gacha |
| O'lcham | kompilyatsiya paytida ma'lum bo'lishi afzal | ish vaqtida istalgan |
| Xato | stek to'lishi | leak, UAF |

> **Qoida:** kichik va qisqa muddatli — stekda; katta, o'lchami noma'lum yoki funksiyadan tashqarida yashashi kerak — heap'da.

## 8.8. Yadroda xotira qanday ajratiladi

Yadroda `malloc` yo'q — uning o'rnida bir necha qatlam (MyOS'da hammasi bor):

```text
kmalloc(n) / kfree(p)       kernel/mm/slab.c    - kichik obyektlar (16 B .. 8 KB), slab keshlari
vmalloc(n)                  kernel/mm/vmalloc.c - katta, virtual jihatdan ketma-ket hududlar
alloc_pages(order)          kernel/mm/pmm.c     - fizik sahifalar (4 KB x 2^order), buddy allocator
memblock                    kernel/mm/memblock.c - eng boshida, boshqa hech narsa tayyor bo'lmaganda
```

Buni `docs/05-heap.md` va 11–12-bosqich hujjatlari tushuntiradi. 30-mashq (mini malloc) — bularga birinchi qadam, 32–33-mashqlar (buddy, slab) — ikkinchi qadam.

## Hayotdan misol va to'liq dastur

**To'y mehmonlari ro'yxati.** Mehmonlar soni oldindan noma'lum — ro'yxat kerakli paytda kattalashadi (`realloc`). Har bir ism alohida `malloc` bilan saqlanadi.

```c
/* mehmonlar.c - malloc, realloc, free: o'sib boradigan ro'yxat */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct royxat {
    char **ismlar;                              /* har bir ism - alohida ajratilgan satr */
    int soni;
    int sigim;                                  /* nechta joy ajratilgan */
};

static int qosh(struct royxat *r, const char *ism)
{
    if (r->soni == r->sigim) {                  /* joy tugadi - kattaroq "kvartira"ga ko'chish */
        int yangi_sigim = r->sigim ? r->sigim * 2 : 2;
        char **yangi = realloc(r->ismlar, yangi_sigim * sizeof(char *));
        if (!yangi)
            return -1;                          /* eski ro'yxat butun qoladi */
        r->ismlar = yangi;
        r->sigim = yangi_sigim;
        printf("  (joy kengaytirildi: %d ta)\n", r->sigim);
    }
    r->ismlar[r->soni] = malloc(strlen(ism) + 1);   /* +1 - '\0' uchun */
    if (!r->ismlar[r->soni])
        return -1;
    strcpy(r->ismlar[r->soni], ism);
    r->soni++;
    return 0;
}

static void ozod_qil(struct royxat *r)
{
    for (int i = 0; i < r->soni; i++)
        free(r->ismlar[i]);                     /* avval har bir ism */
    free(r->ismlar);                            /* keyin ro'yxatning o'zi */
    r->ismlar = NULL;                           /* eski kalitni ham yo'q qilamiz */
    r->soni = r->sigim = 0;
}

int main(void)
{
    struct royxat r = { NULL, 0, 0 };
    const char *kelganlar[] = { "Aziz", "Malika", "Bobur", "Nigora", "Sardor" };

    for (int i = 0; i < 5; i++) {
        if (qosh(&r, kelganlar[i]) != 0) {
            printf("Xotira yetmadi!\n");
            break;
        }
        printf("%s qo'shildi\n", kelganlar[i]);
    }

    printf("Jami %d mehmon:", r.soni);
    for (int i = 0; i < r.soni; i++)
        printf(" %s", r.ismlar[i]);
    printf("\n");

    ozod_qil(&r);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address mehmonlar.c -o mehmonlar
$ ./mehmonlar
  (joy kengaytirildi: 2 ta)
Aziz qo'shildi
Malika qo'shildi
  (joy kengaytirildi: 4 ta)
Bobur qo'shildi
Nigora qo'shildi
  (joy kengaytirildi: 8 ta)
Sardor qo'shildi
Jami 5 mehmon: Aziz Malika Bobur Nigora Sardor
```

**Kodda nimalar bor:**

| Nom | Tur | Boshlang'ich | Nima uchun |
|---|---|---|---|
| `r.ismlar` | `char **` | `NULL` | massiv: har bir element — bitta ismning manzili (`char *`) |
| `r.soni` | `int` | 0 | hozir nechta ism bor |
| `r.sigim` | `int` | 0 | massivda nechta **joy** ajratilgan |

**`qosh` qadamlari:**

1. `soni == sigim`? Joy tugagan → yangi sig'im = eski × 2 (boshida 2). `realloc` ro'yxatni kattaroq blokka ko'chiradi. **Natija avval `yangi` ga yoziladi**, `NULL` bo'lmasa `r->ismlar` ga o'tkaziladi.
2. `malloc(strlen(ism) + 1)` — ism uchun alohida joy: harflar + `'\0'` (shuning uchun `+ 1`!).
3. `strcpy` — ismni ko'chirish (joy aynan yetarli bo'lgani uchun xavfsiz).

**Sig'im qanday o'sadi:**

| Qo'shilgan | `soni` | `sigim` | Nima bo'ldi |
|---|---|---|---|
| Aziz | 1 | 2 | birinchi `realloc`: 0 → 2 |
| Malika | 2 | 2 | joy bor |
| Bobur | 3 | 4 | joy tugadi: 2 → 4 |
| Nigora | 4 | 4 | joy bor |
| Sardor | 5 | 8 | joy tugadi: 4 → 8 |

**Nega sig'imni 2 baravar oshiramiz?** Har qo'shishda `realloc` qilish sekin (ko'chirish qimmat). Ikki baravar oshirsak, `realloc` soni juda kam bo'ladi (amortizatsiyalangan tezlik).

**`ozod_qil` tartibi:** avval **har bir ism** (ichki), keyin **ro'yxatning o'zi** (tashqi). Teskari tartibda qilsangiz, ismlarning manzillarini yo'qotib, leak qilasiz. Sanitizer hech narsa demadi — demak, har bir kalit qaytarilgan.

**Sinab ko'ring:** `ozod_qil(&r);` ni o'chiring va qayta ishga tushiring — sanitizer "memory leak" deb har bir unutilgan xonani ko'rsatadi. `malloc(strlen(ism) + 1)` dan `+ 1` ni o'chiring — nima deydi?

<!-- katta:boshi -->
## Katta loyiha: Ombor — 8-bosqich: dinamik xotira (`malloc`, `realloc`, `free`)

**Oldingi bosqichdan:** massivlar **doimiy o'lchamda** (`MAKS = 8`): sakkizinchi mahsulotdan keyin "ombor to'lgan". Haqiqiy ombor esa o'sishi kerak.

### Bu bosqichda nima qilamiz

Massivlarni **heap** da (dinamik xotirada) ajratamiz: dastur boshida ular **bo'sh** (`NULL`), mahsulot qo'shilganda **o'zi kengayadi**. Qoida: joy tugasa — sig'imni **ikki barobar** oshiramiz (2 → 4 → 8 → 16 ...). Bu **amortizatsiyalangan O(1)** usuli (28-bob): har qo'shish o'rtacha arzon.

**Yangi tushunchalar:**

| Funksiya | Vazifasi |
|---|---|
| `malloc(n)` | heap dan `n` bayt so'raydi; xotira bo'lmasa `NULL` qaytaradi |
| `realloc(p, n)` | mavjud blokni `n` baytga **kattalashtiradi** (kerak bo'lsa ko'chiradi); xato bo'lsa `NULL` — **eski blok saqlanib qoladi** |
| `free(p)` | xotirani qaytaradi |
| `strdup(s)` | satrning heap dagi **nusxasini** yaratadi (`malloc` + `strcpy`); keyin `free` kerak |

**Uchta qat'iy qoida (8-bob):**

1. Har `malloc`/`realloc`/`strdup` dan keyin natijani **tekshiring** (`NULL`?).
2. Har olingan blokni **bir marta** `free` qiling — ko'p emas (`double free`), oldin emas (`use-after-free`), umuman unutmay ham (`leak`).
3. `realloc` natijasini **avval boshqa o'zgaruvchiga** oling: `a = realloc(a, ...)` yozsangiz va u `NULL` qaytarsa, eski ko'rsatkichni **yo'qotib qo'yasiz** (sizib chiqish).

```c
/* ombor.c - Ombor, 8-bosqich: xotira - massivlar o'zi kengayadi (malloc/realloc), hammasi free qilinadi */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ombor_chop.h"

enum { OK = 0, XOTIRA_YOQ = -1, NOM_BAND = -2, TOPILMADI = -3, NOTOGRI_MIQDOR = -4, YETARLI_EMAS = -5 };

/* HEAP dagi massivlar: boshida bo'sh (NULL), kerak bo'lsa kengayadi */
static char **nom;                              /* nomlar: har biri alohida ajratilgan satr */
static long *narx;
static uint16_t *soni;
static int n;                                   /* nechta mahsulot bor */
static int sig;                                 /* nechtaga joy ajratilgan (sig'im) */

/* sig'imni 2 barobar oshiradi (birinchi marta 2 ga). 0 - muvaffaqiyat */
static int sigim_oshir(void)
{
    int yangi = sig ? sig * 2 : 2;
    char **a = realloc(nom, (size_t)yangi * sizeof(*a));
    if (!a)
        return XOTIRA_YOQ;
    nom = a;                                    /* eskisi realloc ichida bo'shatildi yoki ko'chirildi */
    long *b = realloc(narx, (size_t)yangi * sizeof(*b));
    if (!b)
        return XOTIRA_YOQ;
    narx = b;
    uint16_t *c = realloc(soni, (size_t)yangi * sizeof(*c));
    if (!c)
        return XOTIRA_YOQ;
    soni = c;
    printf("  [xotira] sig'im %d -> %d\n", sig, yangi);
    sig = yangi;
    return OK;
}

static int topish(const char *qidirilgan)
{
    for (int i = 0; i < n; i++)
        if (strcmp(nom[i], qidirilgan) == 0)
            return i;
    return -1;
}

static int qosh(const char *yangi_nom, long yangi_narx, uint16_t yangi_soni)
{
    if (topish(yangi_nom) >= 0)
        return NOM_BAND;
    if (n == sig && sigim_oshir() != OK)
        return XOTIRA_YOQ;
    char *nusxa = strdup(yangi_nom);            /* nomning o'z nusxasi: malloc + strcpy */
    if (!nusxa)
        return XOTIRA_YOQ;
    nom[n] = nusxa;
    narx[n] = yangi_narx;
    soni[n] = yangi_soni;
    n++;
    return OK;
}

static void ochir(int i)
{
    free(nom[i]);                               /* nom nusxasini qaytaramiz */
    memmove(&nom[i], &nom[i + 1], (size_t)(n - i - 1) * sizeof(*nom));      /* qolganlarni chapga suramiz */
    memmove(&narx[i], &narx[i + 1], (size_t)(n - i - 1) * sizeof(*narx));
    memmove(&soni[i], &soni[i + 1], (size_t)(n - i - 1) * sizeof(*soni));
    n--;
}

static void hammasini_qaytar(void)
{
    for (int i = 0; i < n; i++)
        free(nom[i]);
    free(nom);
    free(narx);
    free(soni);
    nom = NULL;
    narx = NULL;
    soni = NULL;
    n = sig = 0;
}

static int sot(uint16_t *zaxira, int miqdor)
{
    if (miqdor <= 0)
        return NOTOGRI_MIQDOR;
    if (miqdor > *zaxira)
        return YETARLI_EMAS;
    *zaxira -= miqdor;
    return OK;
}

static void royxat(void)
{
    long jami = 0;
    chop_sarlavha();
    for (int i = 0; i < n; i++) {
        chop_qator(nom[i], narx[i], soni[i]);
        jami += narx[i] * soni[i];
    }
    chop_jami(jami, 12);
}

static const char *xato_matni(int kod)
{
    switch (kod) {
    case XOTIRA_YOQ: return "xotira yetmadi";
    case NOM_BAND: return "bunday nomli mahsulot allaqachon bor";
    case TOPILMADI: return "bunday mahsulot topilmadi";
    case NOTOGRI_MIQDOR: return "miqdor musbat bo'lishi kerak";
    case YETARLI_EMAS: return "omborda yetarli emas";
    default: return "noma'lum xato";
    }
}

int main(void)
{
    qosh("non", 400000, 120);
    qosh("sut", 1200000, 45);
    qosh("guruch", 1800000, 8);

    int tanlov;
    while (printf("\n1) ro'yxat  2) sotish  3) qo'shish  4) o'chirish  0) chiqish\nTanlov:\n"),
           scanf("%d", &tanlov) == 1 && tanlov != 0) {
        char s[24];
        long so_m;
        int miqdor, i, r;
        switch (tanlov) {
        case 1:
            royxat();
            break;
        case 2:
            printf("Mahsulot nomi va necha dona?\n");
            if (scanf("%23s %d", s, &miqdor) != 2)
                break;
            i = topish(s);
            r = i < 0 ? TOPILMADI : sot(&soni[i], miqdor);
            if (r == OK)
                printf("  Sotildi: %d dona %s. Qoldi: %u dona\n", miqdor, nom[i], soni[i]);
            else
                printf("  XATO: %s\n", xato_matni(r));
            break;
        case 3:
            printf("Nom, narx (so'mda) va soni?\n");
            if (scanf("%23s %ld %d", s, &so_m, &miqdor) != 3)
                break;
            r = qosh(s, so_m * 100, (uint16_t)miqdor);
            if (r == OK)
                printf("  Qo'shildi: %s (jami %d ta)\n", s, n);
            else
                printf("  XATO: %s\n", xato_matni(r));
            break;
        case 4:
            printf("Qaysi mahsulot o'chirilsin?\n");
            if (scanf("%23s", s) != 1)
                break;
            i = topish(s);
            if (i < 0) {
                printf("  XATO: %s\n", xato_matni(TOPILMADI));
            } else {
                ochir(i);
                printf("  O'chirildi: %s (qoldi %d ta)\n", s, n);
            }
            break;
        default:
            printf("  XATO: menyuda bunday band yo'q\n");
        }
    }
    hammasini_qaytar();                         /* chiqishdan oldin hamma xotirani qaytarish */
    printf("\nXayr!\n");
    return 0;
}
```

Bu safar dasturni **sanitizer'lar bilan** yig'amiz — ular xotira xatolarini va sizib chiqishni ushlaydi (8-bob, 29-bob):

```console
$ cd katta_loyiha/ombor/08_xotira
$ printf '3\nshakar 15000 60\n3\nyog 30000 20\n3\nun 8000 100\n3\nolma 5000 40\n4\nsut\n4\nsut\n1\n0\n' > kirish.txt
$ gcc -Wall -Wextra -g -fsanitize=address,undefined ombor.c ombor_chop.c -o ombor
$ ./ombor < kirish.txt
  [xotira] sig'im 0 -> 2
  [xotira] sig'im 2 -> 4

1) ro'yxat  2) sotish  3) qo'shish  4) o'chirish  0) chiqish
Tanlov:
Nom, narx (so'mda) va soni?
  Qo'shildi: shakar (jami 4 ta)

1) ro'yxat  2) sotish  3) qo'shish  4) o'chirish  0) chiqish
Tanlov:
Nom, narx (so'mda) va soni?
  [xotira] sig'im 4 -> 8
  Qo'shildi: yog (jami 5 ta)

1) ro'yxat  2) sotish  3) qo'shish  4) o'chirish  0) chiqish
Tanlov:
Nom, narx (so'mda) va soni?
  Qo'shildi: un (jami 6 ta)

1) ro'yxat  2) sotish  3) qo'shish  4) o'chirish  0) chiqish
Tanlov:
Nom, narx (so'mda) va soni?
  Qo'shildi: olma (jami 7 ta)

1) ro'yxat  2) sotish  3) qo'shish  4) o'chirish  0) chiqish
Tanlov:
Qaysi mahsulot o'chirilsin?
  O'chirildi: sut (qoldi 6 ta)

1) ro'yxat  2) sotish  3) qo'shish  4) o'chirish  0) chiqish
Tanlov:
Qaysi mahsulot o'chirilsin?
  XATO: bunday mahsulot topilmadi

1) ro'yxat  2) sotish  3) qo'shish  4) o'chirish  0) chiqish
Tanlov:
================ OMBOR ================
Mahsulot         Narx   Soni          Summa
---------------------------------------
non           4000.00    120      480000.00
guruch       18000.00      8      144000.00
shakar       15000.00     60      900000.00
yog          30000.00     20      600000.00
un            8000.00    100      800000.00
olma          5000.00     40      200000.00
---------------------------------------
Jami qiymat:                3124000.00
QQS stavkasi:               12%
QQS summasi:                374880.00

1) ro'yxat  2) sotish  3) qo'shish  4) o'chirish  0) chiqish
Tanlov:

Xayr!
```

**Nima ko'rdik:**

- `[xotira] sig'im 0 -> 2`, `2 -> 4`, `4 -> 8` — massiv **o'zi kengaydi**: birinchi mahsulotda 2 ga, uchinchisida (3 > 2) 4 ga, beshinchisida (5 > 4) 8 ga.
- Mahsulotlar 3 tadan 7 tagacha ko'paydi — "ombor to'lgan" xatosi yo'q.
- `sut` o'chirildi: qolgan mahsulotlar **chapga surildi** (`memmove`), ro'yxatda tartib saqlandi; ikkinchi urinishda "topilmadi".
- **Sanitizer hech narsa demadi** — hamma `strdup`/`realloc` xotirasi `hammasini_qaytar()` da `free` qilindi. Agar `hammasini_qaytar();` qatorini o'chirsangiz, LeakSanitizer chiqishda **qancha bayt sizib chiqqanini** ko'rsatadi — sinab ko'ring.

**Kodda nimalar bor:**

| Qism | Vazifasi |
|---|---|
| `static char **nom;` | "`char *` lar massivi" ga ko'rsatkich: har nom **alohida** ajratilgan satr |
| `static int sig;` | ajratilgan joy (sig'im); `n` — band joy; `n <= sig` doim |
| `realloc(nom, (size_t)yangi * sizeof(*a))` | `*a` turining o'lchami × soni — `sizeof(*a)` tur o'zgarsa ham to'g'ri qoladi |
| `char *a = realloc(...); if (!a) return ...; nom = a;` | `NULL` tekshiruvi **va** eski ko'rsatkichni xavfsiz almashtirish |
| `strdup(yangi_nom)` | nomning o'z nusxasi: foydalanuvchi buferi o'zgarsa ham saqlanadi |
| `memmove(&nom[i], &nom[i+1], k * sizeof(*nom))` | bloklarni **chapga suradi**; ustma-ust tushsa `memcpy` emas, `memmove` kerak |
| `hammasini_qaytar()` | har nomni, keyin massivlarni `free` qiladi va ko'rsatkichlarni `NULL` ga qaytaradi |

> **Eslab qoling:** `malloc`/`realloc` — **tekshiring**; har blokni **bir marta** `free` qiling; `realloc` natijasini avval vaqtinchalik o'zgaruvchiga oling. Sanitizer (`-fsanitize=address`) bilan **har doim** sinang.

**O'zingiz qo'shing (yechimsiz):**

1. `hammasini_qaytar()` chaqiruvini olib tashlab yig'ing va LeakSanitizer xabarini o'qing: nechta bayt, nechta blok sizdi?
2. `ochir` ichida `free(nom[i])` ni olib tashlang: endi ASan nima deydi?
3. Sig'imni ikki barobar emas, **+1** oshiring: nechta `realloc` chaqiriladi? Nega bu sekin (28-bob)?
<!-- katta:oxiri -->

## Bob xulosasi (yodlash uchun)

1. Xotira uch xil: **stek** (avtomatik, tez, kichik), **statik** (butun dastur davomida), **heap** (`malloc`/`free`, qo'lda).
2. `malloc(n)` — **n bayt** so'raydi (`soni * sizeof(tur)`), `NULL` ni **tekshiring**; har `malloc` ga aniq bitta `free`.
3. `realloc` natijasini avval **vaqtinchalik** o'zgaruvchiga oling (aks holda `NULL` da eski blok yo'qoladi); undan keyin eski manzil yaroqsiz.
4. Har bir blokning **egasi** bor — u `free` qiladi. Xatolar: leak, use-after-free, double free, overflow.
5. O'rganishda doim `-fsanitize=address`; hisobotda birinchi `#0` qatori — xato joyi.

## Savol-javob

**`free` dan keyin xotira OS'ga qaytadimi?**
Odatda yo'q — `malloc` uni keyingi so'rovlar uchun o'zida saqlaydi. Katta bloklar (`mmap` bilan olinganlari) esa darhol qaytariladi.

**`free` qilingan ko'rsatkich nega "yaroqsiz" bo'ladi, qiymati o'zgarmagan-ku?**
Manzil o'zgarmaydi — lekin u endi **sizniki emas**. O'sha joyni allocator o'z sarlavhasi uchun yoki boshqa `malloc` uchun ishlatadi. `free(p); p = NULL;` odati keyingi xatoni
NULL dereference'ga aylantiradi — u darhol va aniq qulaydi (jim buzilishdan yaxshi).

**`malloc(0)` nima qaytaradi?**
Standart bo'yicha — `NULL` yoki `free` qilinadigan noyob ko'rsatkich (implementatsiyaga bog'liq). MyOS libc'si va 30-mashq — `NULL`.

**Nega Python'da bularning hech biri yo'q?**
Python obyektlar sonini o'zi sanaydi va keraksiz bo'lganda o'zi tozalaydi. Bu qulay, lekin qo'shimcha xotira va vaqt oladi va vaqti oldindan noma'lum. Yadroda esa har bayt va har mikrosekund hisobda — shuning uchun qo'lda.

## O'zingizni tekshiring

1. Funksiya ichidagi `static int n;` va oddiy `int n;` farqi (ikki jihat)?
2. `int *a = malloc(10);` — nechta `int` sig'adi?
3. `p = realloc(p, n)` nima uchun xavfli?
4. Nega yadrodagi leak user dasturdagi leak'dan xavfliroq?
5. Sanitizer hisobotida xato qatorini qayerdan topasiz?

<details><summary>Javoblar</summary>

1. static: qiymati chaqiruvlar orasida saqlanadi va avtomatik 0 bilan boshlanadi; oddiy: har chaqiruvda yangi, axlat qiymat.
2. 2 ta (10 bayt / 4) — 10 ta uchun `malloc(10 * sizeof(int))`.
3. `realloc` NULL qaytarsa, eski blok manzili yo'qoladi (leak).
4. Yadro hech qachon tugamaydi — xotira qaytarilmaydi va tizim oxir-oqibat to'xtaydi.
5. Birinchi `#0 ... fayl.c:QATOR` qatori (sizning faylingizdagi birinchisi).
</details>

## Mashq

### Isitish: o'suvchi massiv va satr nusxasi ★☆☆ — eng osoni, avval shuni qiling

Faqat 0–8-boblar kerak (`malloc`, `realloc`, `free`).
Skeletni `isitish.c` ga **qo'lda** yozing (ko'chirmang), izohlarni o'qing va `TODO` joylarini to'ldiring.
"Namuna" qismlar tayyor — qolganini qanday yozishni ko'rsatadi. Skelet hozir ham ogohlantirishsiz yig'iladi:
har `TODO` dan keyin yig'ib, ishga tushirib boring.

```c
/* isitish.c - 8-bob, isitish: o'suvchi massiv va satr nusxasi. struct kerak emas. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* const - "o'qiyman, o'zgartirmayman" (7.6). Massiv o'z uzunligini bilmaydi - n alohida beriladi. */
static void chiqar(const int *a, int n)
{
    printf("%d ta:", n);
    for (int i = 0; i < n; i++)
        printf(" %d", a[i]);
    printf("\n");
}

int main(void)
{
    int n = 5;

    /* 1) Heap'dan 5 ta int: n * sizeof(int) BAYT so'raymiz (8.3). Har malloc - NULL tekshiruvi. (Namuna - tayyor.) */
    int *a = malloc(n * sizeof(int));
    if (!a)
        return 1;
    for (int i = 0; i < n; i++)
        a[i] = (i + 1) * (i + 1);       /* 1, 4, 9, 16, 25 */
    chiqar(a, n);

    /* 2) TODO: realloc bilan 2 * n ta elementga kattalashtiring.
     *    a = realloc(a, ...) DEMANG: realloc NULL qaytarsa, eski blok manzili yo'qoladi - leak (8.5).
     *    To'g'ri:  int *yangi = realloc(a, ...);  if (!yangi) { free(a); return 1; }  a = yangi;
     *    Keyin a[5]..a[9] ni ham kvadratlar bilan to'ldiring, n *= 2 va chiqar(a, n).
     *    Natija: 10 ta: 1 4 9 16 25 36 49 64 81 100 */

    /* 3) TODO: n ta elementning yig'indisi long da -> "yig'indi: %ld\n", keyin free(a).
     *    free dan keyin a[0] ni o'qish - use-after-free (8.5): sanitizer darhol ushlaydi.
     *    Natija: yig'indi: 385 */

    /* 4) Satr nusxasi: strlen + 1 - '\0' ham joy oladi! (Namuna - tayyor.) */
    const char *asl = "salom dunyo";
    char *nusxa = malloc(strlen(asl) + 1);
    if (!nusxa)
        return 1;
    strcpy(nusxa, asl);
    nusxa[0] = 'S';                     /* nusxa - bizniki, o'zgartirsa bo'ladi; asl - satr literali, yo'q */
    printf("asl: %s, nusxa: %s (%zu belgi)\n", asl, nusxa, strlen(nusxa));

    /* 5) TODO: free(nusxa). Hozir (2, 3, 5 yozilmaguncha) dastur oxirida LeakSanitizer
     *    "detected memory leaks" deydi - hisobotni o'qing: qaysi qatorda ajratilgan xotira qaytarilmagan? */
    return 0;
}
```

**Kutilgan natija** (`darslik/loyihalar/08_qavslar/isitish.txt`):

```text
5 ta: 1 4 9 16 25
10 ta: 1 4 9 16 25 36 49 64 81 100
yig'indi: 385
asl: salom dunyo, nusxa: Salom dunyo (11 belgi)
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address,undefined isitish.c -o isitish
$ ./isitish | diff - ~/C_loyha/darslik/loyihalar/08_qavslar/isitish.txt && echo "TO'G'RI"
TO'G'RI
```

### Keyingi mashqlar

- **14** satr yasash, **15** split — `malloc`/`realloc`/`free`.
- **18** — xotira xatolarini topish (bu bobning asosiy mashqi). Unda bog'langan ro'yxat bor (`struct tugun` va `->`, 7.5) —
  9.1 ni ko'z yugurtirib chiqing.
- **13** dinamik massiv, **16** bog'langan ro'yxat, **17** xesh jadval — `struct` (9-bob) kerak: 9-bobdan keyin.
- **30** — o'z `malloc`'ingiz: `struct`, tekislash (9-bob) va xotira ajratish nazariyasi (25-bob) kerak: 25-bobdan keyin.

<!-- loyiha:boshi -->
## Loyiha: o'suvchi satr (string builder)

**Maqsad:** Python'dagi `s += "..."` ning C'dagi ichki tuzilishini yozish: bufer to'lganda **ikki barobar**
kattaroq joy so'rash (`realloc`). Bu yadroning `kmalloc` + o'suvchi buferlarining kichik nusxasi.
**Bobdan ishlatiladi:** `malloc`/`realloc`/`free`, `NULL` tekshiruvi, egalik, sanitizer.

**Talab:** `qosh(s)` funksiyasi satrni oxiriga qo'shsin; kerak bo'lsa bufer o'zi o'ssin.
**Ma'lumotlar:** `satr` (ko'rsatkich), `uzunlik` (ishlatilgan), `sigim` (ajratilgan).
**Qoida:** har doim `uzunlik + 1 <= sigim` (`'\0'` uchun joy!). Sig'im tugasa — 2 barobar oshiramiz.

```c
/* quruvchi.c - o'suvchi satr */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *satr;                              /* dinamik satr (heap'da) */
static size_t uzunlik, sigim;

static void qosh(const char *s)
{
    size_t k = strlen(s);
    if (uzunlik + k + 1 > sigim) {              /* sig'maydi: kattalashtiramiz */
        size_t yangi = sigim ? sigim : 8;
        while (yangi < uzunlik + k + 1)
            yangi *= 2;
        char *p = realloc(satr, yangi);         /* natijani ALOHIDA o'zgaruvchiga oling */
        if (!p) {
            perror("realloc");
            exit(1);
        }
        satr = p;
        sigim = yangi;
        printf("  [sig'im %zu bayt ga oshdi]\n", sigim);
    }
    memcpy(satr + uzunlik, s, k + 1);           /* '\0' ham ko'chadi */
    uzunlik += k;
}

int main(void)
{
    for (int i = 1; i <= 8; i++) {
        char raqam[16];
        snprintf(raqam, sizeof(raqam), "%d", i * i);
        if (i > 1)
            qosh(",");
        qosh(raqam);
    }
    printf("natija: \"%s\"\n(uzunlik %zu, sig'im %zu)\n", satr, uzunlik, sigim);
    free(satr);                                 /* egasi - biz */
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address,undefined quruvchi.c -o quruvchi
$ ./quruvchi
  [sig'im 8 bayt ga oshdi]
  [sig'im 16 bayt ga oshdi]
  [sig'im 32 bayt ga oshdi]
natija: "1,4,9,16,25,36,49,64"
(uzunlik 20, sig'im 32)
```

Nega ikki barobar? Har `qosh` da 8 baytdan oshirsak, 1000 marta qo'shishda 1000 marta `realloc` bo'lardi.
Ikki barobarda — atigi ~10 marta. Bu **amortizatsiyalangan O(1)** (28-bob).
`p` ni alohida oldik: `satr = realloc(satr, ...)` yozsak va `realloc` `NULL` qaytarsa, eski `satr` yo'qoladi (leak).

**Kengaytiring:** `free(satr);` qatorini o'chirib, sanitizer xabarini o'qing. `qosh` ga `%d` formatli `qosh_son(int)` qo'shing.

## Mustaqil loyiha: qavslar tekshiruvchisi ★★★

**Vazifa:** matndagi `()`, `[]`, `{}` qavslar to'g'ri joylashganini tekshiring. Kompilyator ham, JSON o'qigich
ham shunday qiladi. **Stek** kerak — va u **cheksiz o'sishi** kerak (oldindan 100 ta deb bo'lmaydi). Fayl: `qavslar.c`.

**Talab:**
1. Dinamik stek: `malloc`/`realloc` bilan o'suvchi `char` massivi (sig'im 4 dan boshlab, kerak bo'lsa 2 barobar).
   Funksiyalar: `stek_qosh(char)`, `stek_ol()`, `stek_bosh_bormi()`; oxirida hammasi `free` qilinsin.
2. `int tekshir(const char *s)` — quyidagi to'rt natijadan birini bildirsin va **xato joyini** chiqarsin:
   - to'g'ri;
   - yopuvchi qavs turi **mos emas** (`([)]`: 3-belgi `)` ga `[` ochiq turibdi);
   - yopuvchi qavs **ortiqcha** (stek bo'sh);
   - oxirida ba'zi qavslar **yopilmagan**.
3. Qavs bo'lmagan belgilar (harflar, probellar) e'tiborga olinmaydi, lekin belgi tartib raqami ularni ham sanaydi (1 dan).
4. **Uzun sinov:** `n = 100000` ta `(` keyin `n` ta `)`. Qator `malloc` bilan yig'iladi (200000 belgi). Natija to'g'ri
   va stekning **eng katta chuqurligi** chiqarilsin.

**Kutilgan natija** (`darslik/loyihalar/08_qavslar/kutilgan.txt`):

```text
"([]{})": to'g'ri
"{[()()]}": to'g'ri
"([)]": xato, 3-belgi ')' mos emas
"())": xato, 3-belgi ')' ortiqcha
"((": xato, 2 ta qavs yopilmagan
"a(b[c]{d}e)f": to'g'ri
"": to'g'ri
Uzun sinov (200000 belgi): to'g'ri, eng katta chuqurlik 100000
```

Sinov satrlari, tartib bilan: `([]{})`, `{[()()]}`, `([)]`, `())`, `((`, `a(b[c]{d}e)f` va bo'sh satr.
Aynan shu chiqish shakli kerak, shu jumladan qo'shtirnoq va `-belgi` so'zi.

**Maslahat** (yechim emas):
- Ochuvchi qavsni stekka qo'ying; yopuvchi kelganda stek tepasidagi mos juftmi? Mos kelmasa — xato, bo'sh bo'lsa — ortiqcha.
- Stek o'sganda `realloc` natijasini **alohida** o'zgaruvchiga oling (leak bo'lmasin).
- `-fsanitize=address` chiqishi jim bo'lishi kerak — hech qanday leak yo'q.
- 100000 chuqurlik uchun oddiy rekursiya ishlamaydi (stek to'lib ketadi, 5-bob) — nega dinamik stek buni hal qiladi?

**Tekshirish:**

```bash
gcc -Wall -Wextra -g -fsanitize=address,undefined qavslar.c -o dastur && ./dastur | diff - ~/C_loyha/darslik/loyihalar/08_qavslar/kutilgan.txt && echo "TO'G'RI"
```
<!-- loyiha:oxiri -->

Keyingi bob: [9-bob. Struct, union, enum](09-struct.md)
