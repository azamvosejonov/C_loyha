# C tili darsligi — noldan yadro dasturchisigacha

> Bu darslik **internetdan qidirmasdan** C tilini to'liq o'rganish uchun yozilgan: har bir belgi, har
> bir tushuncha va boshlovchi beradigan har bir "nega?" savoli shu yerda javoblangan. Har bir bob
> [mashqlar](../mashqlar/README.md) bilan bog'langan, oxirgi boblar esa sizni MyOS yadrosiga olib boradi.
>
> **Kim uchun:** dasturlashni biladigan (masalan, Python), lekin C'ni endi boshlayotgan odam uchun.
> Umuman dasturlash bilmasangiz ham bo'ladi — faqat sekinroq o'qing va har bir misolni yozib ko'ring.

## Qanday o'qish kerak

1. **Tartib bilan.** Har bir bob oldingilariga tayanadi.
2. **Har bir misolni qo'lda yozing** (ko'chirmang) va ishga tushiring. Keyin o'zgartirib, nima bo'lishini kuzating.
3. Bob oxiridagi **"O'zingizni tekshiring"** savollariga avval o'zingiz javob bering, keyin javobni oching.
4. Bobga tegishli **mashqlarni** bajaring: `tools/mashq.py vazifa`, `tools/mashq.py tekshir`.
5. Tushunmagan joyni **qayta o'qing**, keyin kichik dastur yozib sinang. Tushunmasdan keyingi bobga o'tmang.

## Mundarija

| Bob | Mavzu | Mashqlar |
|---|---|---|
| [00](00-kirish.md) | **Kirish**: C nima, o'rnatish, birinchi dastur — har bir belgi (`#include`, `main`, `void`, `{}`, `;`, `return`) | — |
| [01](01-kompilyatsiya.md) | **Kompilyatsiya**: preprotsessor → kompilyator → assembler → linker; e'lon va ta'rif; gdb | 01 |
| [02](02-turlar.md) | **Turlar**: `int`, `char`, `size_t`, `uint32_t`, signed/unsigned, toshish, cast | 01–03 |
| [03](03-operatorlar.md) | **Operatorlar**: arifmetika, `&&` va `&`, bitli amallar, `++`, ustuvorlik tuzoqlari | 02–04 |
| [04](04-boshqaruv.md) | **Boshqaruv**: `if`, `for`, `while`, `switch`, `goto` (nega yadroda kerak) | 01, 02, 05 |
| [05](05-funksiyalar.md) | **Funksiyalar**: `void` ning barcha ma'nolari, nusxa bo'yicha uzatish, `static`, stek | 03, 06, 07 |
| [06](06-massivlar-satrlar.md) | **Massivlar va satrlar**: `'\0'`, bufer to'lishi, `<string.h>` | 05, 08–12 |
| [07](07-korsatkichlar.md) | **Ko'rsatkichlar**: `&`, `*`, `->`, `NULL`, arifmetika, `void *`, `char **`, funksiya ko'rsatkichlari | 06–08, 10, 16, 20, 23 |
| [08](08-xotira.md) | **Xotira**: stek, heap, statik; `malloc`/`free`; leak, use-after-free | 13–18, 30 |
| [09](09-struct.md) | **Struct, union, enum**: tekislash, `packed`, opaque turlar | 13, 16, 17, 19, 22, 23 |
| [10](10-preprotsessor.md) | **Preprotsessor**: makrolar, `do { } while (0)`, `#x`, `container_of` | 23 |
| [11](11-kop-fayl-make.md) | **Ko'p faylli dasturlar va Make**: `extern`, `static`, kutubxonalar, Makefile | — |
| [12](12-standart-kutubxona.md) | **Standart kutubxona**: `printf` to'liq, `FILE *` buferlash, `errno`, `qsort` | 11, 12, 19, 24 |
| [13](13-ub-xavfsizlik.md) | **Aniqlanmagan xatti-harakat (UB)** va xavfsiz kod | 01, 03, 04, 09, 12, 18 |
| [14](14-tizim-chaqiruvlari.md) | **Tizim chaqiruvlari**: fd, `open/read/write`, `fork/exec/wait`, `pipe`, signallar | 25–28 |
| [15](15-parallellik.md) | **Parallellik**: oqimlar, poyga holati, mutex, spinlock, atomiklar | 29, 34 |
| [16](16-bitlar-apparat.md) | **Bitlar va apparat**: endianness, `volatile`, MMIO, port I/O, bitmap | 04, 21, 22 |
| [17](17-assembly.md) | **Assembly**: registrlar, chaqirish qoidalari, stek kadri, inline asm | — |
| [18](18-yadroga-koprik.md) | **Yadroga ko'prik**: freestanding C, linker skripti, MyOS'ni o'qish | 31–40 |

## Tez savollar — boshlovchilar eng ko'p so'raydigan narsalar

Qisqa javob va batafsil tushuntirish qaysi bobda ekani.

### Sintaksis

| Savol | Qisqa javob | Bob |
|---|---|---|
| Nega har bir qator oxirida `;` qo'yiladi? | C uchun yangi qator hech narsa anglatmaydi; buyruq qayerda tugashini kompilyator faqat `;` dan biladi | [00](00-kirish.md) |
| Qayerda `;` qo'yilMAYdi? | `#include`/`#define` oxirida, funksiya tanasining `}` idan keyin, `if/for/while` bloklaridan keyin | [00](00-kirish.md), [04](04-boshqaruv.md) |
| Nega `struct {...}` dan keyin `;` kerak? | `}` dan keyin shu turdagi o'zgaruvchi e'lon qilish mumkin — `;` "e'lon tugadi" degani | [09](09-struct.md) |
| `{ }` nima qiladi? | Bir nechta buyruqni bitta blok qiladi va yangi ko'rinish sohasi ochadi | [04](04-boshqaruv.md) |
| Chekinish (indent) muhimmi? | Kompilyator uchun yo'q, faqat odam uchun. `if` dan keyin `{}` bo'lmasa, faqat bitta buyruq unga tegishli | [04](04-boshqaruv.md) |
| `#` bilan boshlanadigan qatorlar nima? | Preprotsessor buyruqlari — kompilyatsiyadan oldingi matn almashtirish | [10](10-preprotsessor.md) |
| `#include <...>` va `#include "..."` farqi? | `< >` — tizim papkalari, `" "` — avval loyiha papkasi | [10](10-preprotsessor.md) |
| `.h` va `.c` farqi? | `.h` — e'lonlar (nima bor), `.c` — ta'riflar (kodning o'zi) | [01](01-kompilyatsiya.md) |
| `'a'` va `"a"` farqi? | `'a'` — bitta belgi (son 97), `"a"` — satr: `{'a', '\0'}` | [02](02-turlar.md), [06](06-massivlar-satrlar.md) |
| `\n`, `\0`, `\\` nima? | Maxsus belgilar: yangi qator, nol bayt (satr oxiri), `\` ning o'zi | [00](00-kirish.md), [06](06-massivlar-satrlar.md) |
| `/* */` va `//`? | Izohlar — kompilyator ularni butunlay tashlab yuboradi | [00](00-kirish.md) |
| `0x`, `0` bilan boshlanadigan sonlar? | `0x1F` — o'n oltilik, `017` — **sakkizlik** (=15)! | [02](02-turlar.md) |

### Kalit so'zlar

| Savol | Qisqa javob | Bob |
|---|---|---|
| `void` nima? | "Hech narsa": `void f()` — qaytarmaydi; `f(void)` — argument olmaydi; `void *` — turi noma'lum manzil; `(void)x;` — ataylab tashlash | [05](05-funksiyalar.md) |
| `int main(void)` nima? | Dastur kirish nuqtasi; `int` — chiqish kodi (0 = muvaffaqiyat) | [00](00-kirish.md) |
| `return 0;` nima uchun? | `main` dan chiqish kodi — shell `$?` va `&&` shunga qaraydi | [00](00-kirish.md) |
| `static` nima? | Fayl darajasida — "faqat shu faylda"; funksiya ichida — "qiymati chaqiruvlar orasida saqlanadi" | [05](05-funksiyalar.md), [08](08-xotira.md) |
| `extern` nima? | "Bu nom boshqa faylda ta'riflangan" | [11](11-kop-fayl-make.md) |
| `const` nima? | "O'zgartirmayman" degan va'da, kompilyator tekshiradi | [02](02-turlar.md), [07](07-korsatkichlar.md) |
| `volatile` nima? | "Bu xotiraga har murojaatni aynan bajar" — qurilma registrlari uchun. Qulf EMAS | [16](16-bitlar-apparat.md) |
| `inline` nima? | "Chaqirmasdan, joyiga ko'chirishing mumkin" | [05](05-funksiyalar.md) |
| `typedef` nima? | Turga yangi nom | [09](09-struct.md) |
| `sizeof` nima? | Tur/o'zgaruvchi hajmi baytda, kompilyatsiya paytida | [02](02-turlar.md) |
| `unsigned` nima? | Ishorasiz: manfiy bo'lmaydi, 0 dan pastga tushsa eng katta songa aylanadi | [02](02-turlar.md) |
| `size_t` nima? | Hajm va indekslar uchun ishorasiz tur (`sizeof`, `strlen` natijasi) | [02](02-turlar.md) |
| `uint32_t` nima uchun? | Aniq 32 bit — apparat va disk tuzilmalari uchun (`int` hajmi platformaga bog'liq) | [02](02-turlar.md) |
| `goto` yomon emasmi? | Umuman — ha, lekin xatodan keyin tozalash uchun eng toza yo'l; yadroda ko'p | [04](04-boshqaruv.md) |

### Ko'rsatkichlar va xotira

| Savol | Qisqa javob | Bob |
|---|---|---|
| `&x` nima? | x ning manzili ("x qayerda?") | [07](07-korsatkichlar.md) |
| `*p` nima? | E'londa — "p ko'rsatkich"; ifodada — "p ko'rsatgan joydagi qiymat" | [07](07-korsatkichlar.md) |
| `.` va `->` farqi? | `s.x` — struct qiymati orqali; `p->x` — struct ko'rsatkichi orqali (= `(*p).x`) | [07](07-korsatkichlar.md), [09](09-struct.md) |
| `NULL` nima? | "Hech qayerga ko'rsatmaydi" (0 manzil); dereference qilish — qulash | [07](07-korsatkichlar.md) |
| `char **argv` nima? | Satrlar (ko'rsatkichlar) massivi | [07](07-korsatkichlar.md) |
| Nega funksiya argumentimni o'zgartira olmaydi? | Argumentlar nusxa sifatida uzatiladi — manzilini bering | [05](05-funksiyalar.md), [07](07-korsatkichlar.md) |
| `a[i]` aslida nima? | `*(a + i)` — ko'rsatkich arifmetikasi | [07](07-korsatkichlar.md) |
| Nega massiv o'z uzunligini bilmaydi? | Funksiyaga faqat birinchi element manzili boradi — uzunlikni alohida uzating | [06](06-massivlar-satrlar.md) |
| `malloc` va `free`? | Heap'dan xotira olish va qaytarish; har bir `malloc` ga aniq bitta `free` | [08](08-xotira.md) |
| Segmentation fault nima? | Ruxsat etilmagan manzilga murojaat (NULL, free qilingan, chegaradan tashqari) | [07](07-korsatkichlar.md), [08](08-xotira.md) |
| Nega `if (s == "exit")` ishlamaydi? | Manzillar solishtiriladi; mazmun uchun `strcmp(s, "exit") == 0` | [06](06-massivlar-satrlar.md) |

### Xatolar va vositalar

| Savol | Qisqa javob | Bob |
|---|---|---|
| `expected ';' before ...` | Bir qator **yuqorida** `;` unutilgan | [00](00-kirish.md) |
| `implicit declaration of function` | E'lon yo'q — `#include` unutilgan | [01](01-kompilyatsiya.md) |
| `undefined reference to` | Ta'rif yo'q — `.c` fayl yoki kutubxona (`-lm`, `-pthread`) ulanmagan | [01](01-kompilyatsiya.md) |
| `-Wall -Wextra` nima? | Ogohlantirishlarni yoqish — xatolarning katta qismini oldindan topadi | [00](00-kirish.md), [01](01-kompilyatsiya.md) |
| Aniqlanmagan xatti-harakat (UB) nima? | Standart natijani belgilamagan holat — kompilyator kodingizni o'zgartirishi mumkin | [13](13-ub-xavfsizlik.md) |
| Sanitizer hisobotini qanday o'qiyman? | Xato turi birinchi qatorda, joyi — birinchi `#0 ... fayl.c:QATOR` | [08](08-xotira.md) |
| `gdb` bilan qanday ishlayman? | `break`, `run`, `next`, `step`, `print`, `bt` | [01](01-kompilyatsiya.md) |

## Darslikdan keyin

```text
darslik 00-18  →  mashqlar 01-30  →  mashqlar 31-40  →  docs/ + MyOS kodi  →  labs  →  noldan yadro
```

- [mashqlar/README.md](../mashqlar/README.md) — 40 ta mashq, avtomatik tekshiruv bilan.
- [QOLLANMA.md](../QOLLANMA.md) — loyihaning to'liq qo'llanmasi va noldan yadro yozish rejasi.
- [labs/README.md](../labs/README.md) — yadro ichidagi 22 ta laboratoriya.
- [YAKUNIY.md](../YAKUNIY.md) — haqiqiy kompyuterda ishlatish va tizimni kengaytirish.
- [REJA.md](../REJA.md) — 6 oylik kunma-kun reja.
