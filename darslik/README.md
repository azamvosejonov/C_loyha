# C tili darsligi — noldan yadro dasturchisigacha

> Bu darslik **internetdan qidirmasdan** C tilini to'liq o'rganish uchun yozilgan: har bir belgi, har
> bir tushuncha va boshlovchi beradigan har bir "nega?" savoli shu yerda javoblangan. Har bir bob
> [mashqlar](../mashqlar/README.md) bilan bog'langan, oxirgi boblar esa sizni MyOS yadrosiga olib boradi.
>
> **Kim uchun:** dasturlashni biladigan (masalan, Python), lekin C'ni endi boshlayotgan odam uchun.
> Umuman dasturlash bilmasangiz ham bo'ladi — faqat sekinroq o'qing va har bir misolni yozib ko'ring.

## Qanday o'qish kerak

1. **Tartib bilan.** Har bir bob oldingilariga tayanadi.
   Har bir bob **"Hayotdan misollar"** bo'limi bilan boshlanadi: bobning g'oyalari kundalik hayotdagi
   narsalar (bankomat, pochta, metro, kinoteatr...) bilan tushuntiriladi va har birida o'ziga xos,
   to'liq ishlaydigan dastur bor — ekranda nima chiqishi ham yozilgan.
2. **Har bir misolni qo'lda yozing** (ko'chirmang) va ishga tushiring. Keyin o'zgartirib, nima bo'lishini kuzating.
   Har bir bob uchun **to'liq ishlaydigan dastur** [misollar/](misollar/README.md) papkasida bor (bob boshida
   havolasi). Bobdagi kod parchasini (masalan, faqat `#define N 10`) qanday sinashni ham o'sha yerda o'qing.
3. Har bob oxirida **kichik loyiha** bor: avval men ko'rsatgan tizimni yozing, keyin **mustaqil loyihani** o'zingiz yarating
   va natijani `diff` bilan tekshiring. Tafsilot va ro'yxat: [loyihalar/](loyihalar/README.md).
4. Bob oxiridagi **"O'zingizni tekshiring"** savollariga avval o'zingiz javob bering, keyin javobni oching.
5. Bobga tegishli **mashqlarni** bajaring: `tools/mashq.py vazifa`, `tools/mashq.py tekshir`.
6. Tushunmagan joyni **qayta o'qing**, keyin kichik dastur yozib sinang. Tushunmasdan keyingi bobga o'tmang.
7. **Notanish belgi** (`|`, `&`, `->`, `%`, `<<` ...) uchrasa — [belgilar.md](belgilar.md). **`#include <...>`** nima ekani va qaysi ish uchun
   qaysi sarlavha kerakligi — [sarlavhalar.md](sarlavhalar.md) (masalan, `<stdint.h>` nima qilishi).

**Boblar qanday yozilgan:** har bo'lim "Oddiy qilib aytganda → Hayotdan misol → Qadam-baqadam → Kod (har qatori tushuntirilgan, qiymatlar
jadvali bilan) → Nega shunday? → **Eslab qoling**" tartibida. Har bir kod blokining natijasi **haqiqatan ishga tushirib** olingan.

## Katta loyihalar: Ombor (0–12) va tizim dasturlari (13–31)

Boblarni bitta **o'sib boradigan dastur** bog'laydi: **Ombor** (mahsulotlar ombori). Har bobning oxirida (0–12) "**Katta loyiha: Ombor — N-bosqich**" bo'limi bor: yangi tushuncha oldingi versiyadagi muammoni hal qiladi. To'liq tavsif va tayyor kodlar — [katta_loyiha/](katta_loyiha/README.md).

10–16-boblarda esa yana bitta o'sib boruvchi dastur — **Kadrlar tizimi** (makrolardan binar faylgacha, 7 bosqich). 13–31-boblarning har birida ham o'zining **katta loyihasi** bor (fuzzer, shell, fayl tizimi, mini-yadro, `malloc` kuzatuvchisi va h.k. — jadval: [katta_loyiha/README.md](katta_loyiha/README.md)). Har bobda: **umumiy fikr → kod → haqiqiy chiqish → tushuntirish → o'zingiz qo'shing (yechimsiz)**.

## Birlashtiruvchi loyihalar

Bir necha bobni **bitta masalada** ishlatish uchun: [birlashtiruvchi/](birlashtiruvchi/README.md). Birinchisi — **Maosh hisobchisi** (0–2-boblar), ikkinchisi — **Kadrlar tizimi** (40 fayllik katta loyiha, 14 ta funksiyani o'zingiz yozasiz).

## Mundarija

| Bob | Mavzu | Mashqlar |
|---|---|---|
| [Asos](asos-kompyuter-ichi.md) | **Kompyuter ichi (0-bobdan OLDIN o'qing)**: bit, bayt, ikkilik, o'n oltilik (`0x`, `0b`), oldidagi nollar, toshish, ma'no va tur, matn, xotira, qo'shish kalitlar bilan, jismoniy saqlash; katta loyiha — `konv` | Isitish |
| [00](00-kirish.md) | **Kirish**: C nima, o'rnatish, birinchi dastur — har bir belgi (`#include`, `main`, `void`, `{}`, `;`, `return`) | Isitish |
| [01](01-kompilyatsiya.md) | **Kompilyatsiya**: preprotsessor → kompilyator → assembler → linker; e'lon va ta'rif; gdb | Isitish |
| [02](02-turlar.md) | **Turlar**: `int`, `char`, `size_t`, `uint32_t`, signed/unsigned, toshish, cast | Isitish |
| [03](03-operatorlar.md) | **Operatorlar**: arifmetika, `&&` va `&`, bitli amallar, `++`, ustuvorlik tuzoqlari | Isitish, 04 |
| [04](04-boshqaruv.md) | **Boshqaruv**: `if`, `for`, `while`, `switch`, `goto` (nega yadroda kerak) | Isitish, 01, 02, 04 |
| [05](05-funksiyalar.md) | **Funksiyalar**: `void` ning barcha ma'nolari, nusxa bo'yicha uzatish, `static`, stek | Isitish, 03, 06 |
| [06](06-massivlar-satrlar.md) | **Massivlar va satrlar**: `'\0'`, bufer to'lishi, `<string.h>` | Isitish, 05, 07–12 |
| [07](07-korsatkichlar.md) | **Ko'rsatkichlar**: `&`, `*`, `->`, `NULL`, arifmetika, `void *`, `char **`, funksiya ko'rsatkichlari | Isitish, 06–08, 10 |
| [08](08-xotira.md) | **Xotira**: stek, heap, statik; `malloc`/`free`; leak, use-after-free | Isitish, 14, 15, 18 |
| [09](09-struct.md) | **Struct, union, enum**: tekislash, `packed`, opaque turlar | Isitish, 13, 16, 17, 19, 20, 22 |
| [10](10-preprotsessor.md) | **Preprotsessor**: makrolar, `do { } while (0)`, `#x`, `container_of` | Isitish, 23 |
| [11](11-kop-fayl-make.md) | **Ko'p faylli dasturlar va Make**: `extern`, `static`, kutubxonalar, Makefile | Isitish |
| [12](12-standart-kutubxona.md) | **Standart kutubxona**: `printf` to'liq, `FILE *` buferlash, `errno`, `qsort` | Isitish, 11, 12, 19, 24 |
| [13](13-ub-xavfsizlik.md) | **Aniqlanmagan xatti-harakat (UB)** va xavfsiz kod | Isitish, 01, 03, 04, 09, 12, 18 |
| [14](14-tizim-chaqiruvlari.md) | **Tizim chaqiruvlari**: fd, `open/read/write`, `fork/exec/wait`, `pipe`, signallar | Isitish, 25–28 |
| [15](15-parallellik.md) | **Parallellik**: oqimlar, poyga holati, mutex, spinlock, atomiklar | Isitish, 29, 34 |
| [16](16-bitlar-apparat.md) | **Bitlar va apparat**: endianness, `volatile`, MMIO, port I/O, bitmap | Isitish, 04, 21, 22 |
| [17](17-assembly.md) | **Assembly**: registrlar, chaqirish qoidalari, stek kadri, inline asm | Isitish |
| [18](18-yadroga-koprik.md) | **Yadroga ko'prik**: freestanding C, linker skripti, MyOS'ni o'qish | Isitish, 34, 39, 40 |

**Isitish** — har bobdagi eng oson mashq: izohli kod skeleti, `TODO` larni o'zingiz yozasiz; faqat shu va oldingi boblarda
o'tilgan narsa kerak, natija `diff` bilan tekshiriladi. Mashq raqami esa o'sha bobda **to'liq** yechish mumkin bo'lgan joyda turibdi (oldin bobda tilga olinsa ham).

### II qism: kompyuter tizimlari va operatsion tizimlar

| Bob | Mavzu | Mashqlar |
|---|---|---|
| [19](19-terminal-git.md) | **Terminal va Git**: fayllar, qidirish, quvurlar, ruxsatlar, commit, `git bisect` | Isitish |
| [20](20-sonlar.md) | **Sonlar**: ikkilik/o'n oltilik, ikkiga to'ldirish, ishora kengayishi, IEEE 754 float | Isitish, 41 |
| [21](21-kesh.md) | **Xotira ierarxiyasi va kesh**: lokallik, kesh qatori, soxta bo'lishish, TLB | Isitish |
| [22](22-boglash.md) | **Bog'lash va ELF**: belgilar, relokatsiya, statik/dinamik, PLT/GOT, `exec` | Isitish, 36 |
| [23](23-jarayonlar-scheduling.md) | **Jarayonlar va rejalashtirish**: holatlar, kontekst almashish, FIFO/SJF/RR/MLFQ/CFS | Isitish, 35 |
| [24](24-virtual-xotira.md) | **Virtual xotira**: sahifalash, page fault, almashtirish algoritmlari, COW, `mmap` | Isitish, 31, 43 |
| [25](25-xotira-ajratish.md) | **Xotira ajratish**: fragmentatsiya, bo'sh ro'yxatlar, chegara teglari, buddy, slab | Isitish, 30, 32, 33 |
| [26](26-parallellik-chuqur.md) | **Parallellik chuqur**: qulflarni qurish, semaforlar, klassik masalalar, deadlock | Isitish, 44, 45, 48 |
| [27](27-fayl-tizimlari.md) | **Qurilmalar va fayl tizimlari**: DMA, disklar, inode, FFS, jurnal | Isitish, 37 |
| [28](28-algoritmlar.md) | **Algoritmlar**: O-belgi, xesh, daraxtlar, heap, graflar, saralash | Isitish, 42, 46, 47, 48 |
| [29](29-debug-vositalari.md) | **Debug vositalari**: gdb chuqur, sanitizer, valgrind, strace, perf | Isitish |
| [30](30-yadro-arxitekturasi.md) | **Yadro arxitekturasi**: monolit/mikroyadro, Linux xaritasi, modul, patch | Isitish |
| [31](31-lugat.md) | **Lug'at**: ingliz atamalari, koddagi qisqartmalar, xato xabarlari tarjimasi | Isitish |

### III qism: amaliyot — MyOS kodini o'zingiz yozasiz

| Bob | Mavzu | Mashqlar |
|---|---|---|
| [32](32-printf-malloc.md) | **`printf` va `malloc` ni yozish**: format dvigateli, `kprintf`, heap, free list, split, coalescing — kod repoda bo'sh, testlar tayyor | Isitish, M1–M4 |

### Bu darslik qaysi kitoblar o'rnini bosadi

Bu darslik quyidagi mashhur (ingliz tilidagi) kitoblarning **asosiy mavzularini** o'zbek tilida, o'z
tushuntirishlarimiz va MyOS kodi misollari bilan qamraydi — ularni tarjima qilmaydi, balki shu bilimlarni
beradi:

| Kitob (asl nomi) | Mavzu | Darslikda |
|---|---|---|
| K. N. King, *C Programming: A Modern Approach*; Kernighan & Ritchie, *The C Programming Language* | C tili | 00–18 |
| Bryant & O'Hallaron, *Computer Systems: A Programmer's Perspective* (CS:APP) | sonlar, assembly, kesh, bog'lash, jarayonlar, virtual xotira, `malloc`, I/O, parallellik | 13–17, 20–22, 24–26 |
| Arpaci-Dusseau, *Operating Systems: Three Easy Pieces* (OSTEP) | jarayonlar, scheduling, virtual xotira, parallellik, fayl tizimlari | 14, 15, 23–27 |
| Cormen va boshq., *Introduction to Algorithms* (asosiy qismlari) | algoritmlar va tuzilmalar | 28 |
| Love, *Linux Kernel Development*; Bovet & Cesati, *Understanding the Linux Kernel* (umumiy manzara) | yadro arxitekturasi | 18, 30 va `docs/` |
| Intel SDM (kerakli qismlari), OSDev Wiki | x86-64, yuklash, uzilishlar, APIC | 16, 17 va `docs/01–15` |

Chuqurlik jihatidan bu kitoblar kattaroq — lekin yadro dasturchisi bo'lish uchun **kerakli yadrosi** shu
yerda, amaliy mashqlar va haqiqiy yadro kodi bilan bog'langan holda.

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
darslik 00-18 → mashqlar 01-30 → darslik 19-31 → mashqlar 31-48 → docs/ + MyOS kodi → labs + ovchi → mustaqil (B0-B12) → virtual_kompyuter
```

- [mashqlar/README.md](../mashqlar/README.md) — 48 ta mashq, avtomatik tekshiruv bilan.
- [QOLLANMA.md](../QOLLANMA.md) — loyihaning to'liq qo'llanmasi va noldan yadro yozish rejasi.
- [mustaqil/](../mustaqil/README.md) — **o'z yadroingiz noldan**: B0–B12 bosqichlari, `tools/mustaqil.py` ularni QEMU'da
  tekshiradi. `tools/ovchi.py` — xato ovchisi: yadrodagi yashirin xatoni alomatdan topish.
- [labs/README.md](../labs/README.md) — yadro ichidagi 19 ta laboratoriya.
- [YAKUNIY.md](../YAKUNIY.md) — haqiqiy kompyuterda ishlatish va tizimni kengaytirish.
- [virtual_kompyuter/](../virtual_kompyuter/README.md) — **eng chuqur loyiha**: RISC-V kompyuterni (protsessor, MMU,
  uzilishlar, firmware) o'zingiz yozib, unda haqiqiy Linux'ni yuklaysiz. O'z kitobi (12 bob) va 22 ta mashq bilan.
  Kerak: 16-bob (bitlar), 17 (assembly), 24 (virtual xotira).
- [REJA.md](../REJA.md) — 6 oylik kunma-kun reja.
