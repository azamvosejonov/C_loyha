# 8-bob. Xotira: stek, heap, statik

> **Bu bobdan keyin:** dastur xotirasi qanday bo'linganini, `malloc`/`free`/`realloc` ni to'g'ri
> ishlatishni, "egalik" (kim `free` qiladi) qoidasini va xotira xatolarini (leak, use-after-free,
> double free) topishni bilasiz. Mashqlar: 13–18, 30.

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

MyOS'da bu xaritani yadro yaratadi: ELF faylidan `.text/.data/.bss` ni yuklash — `kernel/sys/elf.c`,
stek va heap uchun hududlar (VMA) — `kernel/mm/mm.c`, `sbrk` syscall'i heap'ni o'stiradi.

## 8.2. Uch xil "saqlash muddati"

| Tur | Qayerda | Qachon yaratiladi | Qachon yo'qoladi | Misol |
|---|---|---|---|---|
| **Avtomatik** | stek | funksiyaga kirganda | funksiyadan chiqqanda | `int x;` funksiya ichida |
| **Statik** | .data / .bss | dastur boshlanganda | dastur tugaganda | global, `static` o'zgaruvchi |
| **Dinamik** | heap | `malloc` chaqirilganda | `free` chaqirilganda | `malloc(100)` |

### Statik o'zgaruvchilar

```c
int hisob = 0;                  /* global: butun dastur davomida yashaydi */

void chaqir(void)
{
    static int necha_marta = 0; /* FUNKSIYA ichidagi static: qiymati chaqiruvlar orasida SAQLANADI */
    necha_marta++;
    printf("%d\n", necha_marta);    /* 1, 2, 3, ... */
}
```

- Global va `static` o'zgaruvchilar **avtomatik 0** bilan boshlanadi (lokal o'zgaruvchilardan farqli).
- `static` so'zining **ikki ma'nosi**: fayl darajasida — "faqat shu faylda ko'rinadi" (5-bob);
  funksiya ichida — "stekda emas, statik xotirada yashaydi".
- Yadroda globallar ko'p: `static struct list_head disks;`, `static uint64_t ticks;` — lekin ko'p
  yadroli tizimda ularni **qulf bilan** himoya qilish kerak (15-bob).

## 8.3. `malloc` va `free`

```c
#include <stdlib.h>

int *a = malloc(100 * sizeof(int));     /* 100 ta int uchun joy (400 bayt) */
if (a == NULL) {                        /* xotira berilmadi */
    perror("malloc");
    return -1;
}
a[0] = 5;
...
free(a);                                /* qaytarish */
a = NULL;                               /* ixtiyoriy, lekin foydali odat */
```

- `malloc(n)` — **n bayt** so'raydi. Elementlar soni emas! Doim `soni * sizeof(tur)`.
- Qaytgan xotira **nollanmagan** (axlat). Nol kerak bo'lsa — `calloc(soni, hajm)`.
- `NULL` qaytishi mumkin — **tekshiring**.
- Har bir `malloc` ga aniq **bitta** `free`. `free(NULL)` — hech narsa qilmaydi (xavfsiz).

### `realloc` — hajmni o'zgartirish

```c
int *yangi = realloc(a, 200 * sizeof(int));
if (!yangi) {
    /* a hali ham haqiqiy - uni yo'qotmaslik uchun vaqtinchalik o'zgaruvchi */
    free(a);
    return -1;
}
a = yangi;
```

`realloc` blokni joyida kattalashtiradi yoki yangi joyga **ko'chiradi** (eski mazmun saqlanadi, eski
manzil yaroqsiz bo'ladi). **Tuzoq:** `a = realloc(a, ...)` — `NULL` qaytsa, eski blok manzili yo'qoladi
(leak). 13-mashq.

### `malloc` ichida nima bor

`malloc` — sehr emas, oddiy C kodi: katta xotira hududini (OS'dan `sbrk`/`mmap` bilan olingan)
bloklarga bo'lib beradi. Har bir blok oldida kichik **sarlavha** (hajm, bo'shmi) turadi. `free` blokni
"bo'sh" deb belgilaydi va qo'shni bo'sh bloklar bilan birlashtiradi. 30-mashqda xuddi shuni o'zingiz
yozasiz, MyOS'da esa `user/libc/malloc.c` (va malloc lab'i).

Shundan muhim xulosa: `malloc` qaytargan blokdan **tashqariga** yozsangiz, keyingi blokning
sarlavhasini buzasiz. Xato keyingi `malloc`/`free` da, **boshqa joyda** chiqadi — shuning uchun bunday
xatolarni topish qiyin.

## 8.4. Egalik (ownership) — kim `free` qiladi?

C'da garbage collector yo'q, shuning uchun har bir ajratilgan xotiraning **egasi** bo'lishi kerak —
oxirida uni `free` qiladigan kod. Qoida funksiya hujjatida yoziladi:

```c
/* Yangi satr qaytaradi. Chaqiruvchi uni free() qilishi SHART. */
char *birlashtir(const char *a, const char *b);

/* s ning ichiga ko'rsatkich qaytaradi. free() QILMANG - bu s ning bir qismi. */
const char *strchr(const char *s, int c);
```

Tipik uslublar:
- **"Yaratuvchi-yo'q qiluvchi" juftligi:** `xesh_yarat` / `xesh_ozod`, `fopen` / `fclose`,
  `ajrat` / `ajrat_ozod` (15, 17-mashqlar).
- **Havolalar sanog'i (refcount):** obyektni bir nechta joy ishlatsa, har biri sanoqni oshiradi,
  tugatganda kamaytiradi; 0 bo'lganda obyekt yo'q qilinadi. MyOS'da: `struct file` dagi `refcount`
  (`fork` va `dup` bitta faylni bo'lishadi), inode keshi. Linux'da `kref`.

## 8.5. Xotira xatolari — to'liq katalog

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

### Leak'lar yadroda nega o'ldiradi

User dasturdagi leak dastur tugaganda yo'qoladi (OS hamma xotirani qaytarib oladi). **Yadro esa
hech qachon tugamaydi**: har bir syscall'da 64 bayt sizib chiqsa, bir necha soatda xotira tugaydi.
Shuning uchun yadroda xatodan keyin tozalash (`goto` — 4-bob) juda muhim.

### Use-after-free yadroda nega eng xavfli

Bo'shatilgan xotira tezda boshqa obyektga beriladi. Eski ko'rsatkich orqali yozish — **boshqa**
obyektni (masalan, boshqa jarayonning credentials'ini) o'zgartiradi. Zamonaviy yadro hujumlarining
katta qismi — use-after-free. MyOS'dagi slab allocator shuning uchun `free` qilingan obyektni "zahar"
bilan to'ldiradi va keyingi `alloc` da tekshiradi (`kernel/mm/slab.c`; `APPEND=demo=uaf` bilan ko'ring).

## 8.6. Xatolarni topish vositalari

**AddressSanitizer** (mashqlarda doim yoqilgan):

```bash
gcc -g -fsanitize=address,undefined dastur.c -o dastur && ./dastur
```

Hisobotni o'qish:

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

**Valgrind** (qayta kompilyatsiyasiz): `valgrind --leak-check=full ./dastur`.

18-mashq — aynan shu hisobotlarni o'qib, 5 ta xatoni topish.

## 8.7. Stek va heap — qachon qaysi biri

| | Stek | Heap |
|---|---|---|
| Tezlik | juda tez (faqat `rsp` ni surish) | sekinroq (allocator ishlaydi) |
| Hajm | cheklangan (user: 8 MB, **yadro: 8–16 KB**) | katta |
| Yashash muddati | funksiya tugaguncha | `free` gacha |
| O'lcham | kompilyatsiya paytida ma'lum bo'lishi afzal | ish vaqtida istalgan |
| Xato | stek to'lishi | leak, UAF |

Qoida: kichik va qisqa muddatli — stekda; katta, o'lchami noma'lum yoki funksiyadan tashqarida
yashashi kerak — heap'da.

## 8.8. Yadroda xotira qanday ajratiladi

Yadroda `malloc` yo'q — uning o'rnida bir necha qatlam (MyOS'da hammasi bor):

```text
kmalloc(n) / kfree(p)       kernel/mm/slab.c    - kichik obyektlar (16 B .. 8 KB), slab keshlari
vmalloc(n)                  kernel/mm/vmalloc.c - katta, virtual jihatdan ketma-ket hududlar
alloc_pages(order)          kernel/mm/pmm.c     - fizik sahifalar (4 KB x 2^order), buddy allocator
memblock                    kernel/mm/memblock.c - eng boshida, boshqa hech narsa tayyor bo'lmaganda
```

Buni `docs/05-heap.md` va 11–12-bosqich hujjatlari tushuntiradi. 30-mashq (mini malloc) — bularga
birinchi qadam, 32–33-mashqlar (buddy, slab) — ikkinchi qadam.

## 8.9. Savol-javob

**`free` dan keyin xotira OS'ga qaytadimi?**
Odatda yo'q — `malloc` uni keyingi so'rovlar uchun o'zida saqlaydi. Katta bloklar (`mmap` bilan
olinganlari) esa darhol qaytariladi.

**`free` qilingan ko'rsatkich nega "yaroqsiz" bo'ladi, qiymati o'zgarmagan-ku?**
Manzil o'zgarmaydi — lekin u endi **sizniki emas**. O'sha joyni allocator o'z sarlavhasi uchun yoki
boshqa `malloc` uchun ishlatadi. `free(p); p = NULL;` odati keyingi xatoni NULL dereference'ga
aylantiradi — u darhol va aniq qulaydi (jim buzilishdan yaxshi).

**`malloc(0)` nima qaytaradi?**
Standart bo'yicha — `NULL` yoki `free` qilinadigan noyob ko'rsatkich (implementatsiyaga bog'liq).
MyOS libc'si va 30-mashq — `NULL`.

## 8.10. O'zingizni tekshiring

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

## 8.11. Mashqlar

- **13** dinamik massiv, **14** satr yasash, **15** split — `malloc`/`realloc`/`free`.
- **16** bog'langan ro'yxat, **17** xesh jadval — egalik va tozalash.
- **18** — xotira xatolarini topish (bu bobning asosiy mashqi).
- **30** — o'z `malloc`'ingiz.

Keyingi bob: [9-bob. Struct, union, enum](09-struct.md)
