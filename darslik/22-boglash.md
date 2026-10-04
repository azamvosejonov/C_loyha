# 22-bob. Bog'lash (linking) va ELF — dastur fayli ichida nima bor

> **Bu bobda nima o'rganasiz:** obyekt fayl (`.o`) va dastur fayli ichida nima borligini (bo'limlar, belgilar jadvali, relokatsiyalar); linker aslida nima qilishini; statik va dinamik bog'lash farqini;
> PLT/GOT'ni; dastur qanday yuklanib ishga tushishini. `nm`, `readelf`, `objdump` bilan istalgan dasturning ichini ko'ra olasiz. (Odatda CS:APP 7-bobidan o'rganiladi.)
> **Oldindan nima kerak:** 1-, 11-, 17-, 18-boblar.   **Vaqt:** 6–7 soat.
> Mashqlar: 36 (ELF tahlilchisi).

> **To'liq ishlaydigan misol:** [misollar/22_boglash.c](misollar/22_boglash.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Bu bob nima haqida?

1-bobda ko'rdingiz: `.c` → `.o` → dastur. Endi **dastur fayli ichiga** qaraymiz: u shunchaki "kod baytlari" emas, **tuzilgan format** (ELF). Uni o'qish bilan siz "nega `undefined reference`?",
"nega statik dastur katta?", "dastur qanday ishga tushadi?" savollariga aniq javob olasiz.

**Hayotdan misol: ELF fayl — yuk konteyneri va yuk xati.** Portga kelgan konteyner ichida yuk bor, eshigida esa **yuk xati**: nima bor, qancha, qayerga tushirish kerak. ELF faylning boshida ham sarlavha bor:
"bu x86-64 uchun dastur, kod shu yerda, ma'lumotlar bu yerda, ishni shu manzildan boshla". Yuklovchi (yadrodagi `exec`) shu xatni o'qib, yukni xotiraga tushiradi.

| Portda | ELF faylda |
|---|---|
| yuk xati | ELF sarlavhasi |
| konteyner bo'limlari | bo'limlar / segmentlar |
| "qayerga tushirish" | yuklash manzillari |
| yuk | kod va ma'lumot baytlari |

## 22.1. Ikki xil ELF: obyekt va bajariladigan

Linux'da (va MyOS'da) hamma dastur fayllari **ELF** (Executable and Linkable Format) formatida:

| Tur | Fayl | Kim uchun |
|---|---|---|
| Relocatable (`ET_REL`) | `.o` | linker uchun: bo'limlar, belgilar, "hali to'ldirilmagan joylar" |
| Executable (`ET_EXEC`) | `a.out`, `kernel.elf` | OS/yuklovchi uchun: aniq manzillar |
| Shared object / PIE (`ET_DYN`) | `.so`, zamonaviy dasturlar | istalgan manzilga yuklanadi |

ELF'da ikki "ko'rinish" bor:

- **Bo'limlar (sections)** — linker uchun: `.text`, `.data`, `.bss`, `.rodata`, `.symtab`, `.rela.text`...
- **Segmentlar (program headers)** — yuklovchi uchun: "fayldagi shu qismni xotiraning shu manziliga shu ruxsatlar bilan qo'y" (`PT_LOAD`). 36-mashqda siz aynan shularni o'qidingiz;
  MyOS `exec` ham faqat segmentlarga qaraydi (`kernel/sys/elf.c`).

```bash
readelf -h dastur        # sarlavha: tur, mashina, kirish nuqtasi
readelf -S dastur.o      # bo'limlar
readelf -l dastur        # segmentlar
readelf -s dastur.o      # belgilar jadvali
objdump -d dastur.o      # mashina kodi
nm dastur.o              # belgilar qisqacha
```

## 22.2. Asosiy bo'limlar

**Hayotdan misol: chamadonning bo'linmalari.**

- `.text` — **kod**: faqat o'qish va bajarish mumkin, o'zgartirib bo'lmaydi.
- `.rodata` — **muhrlangan konvertlar**: o'zgarmas satrlar va jadvallar.
- `.data` — **to'ldirilgan idishlar**: qiymati bor global o'zgaruvchilar. Faylda joy egallaydi.
- `.bss` — **"bo'sh 10 ta quti kerak" degan yozuv**: nol bilan boshlanadigan o'zgaruvchilar. Faylda faqat o'lchami yoziladi — 1 MB lik nol massiv faylni 1 MB ga kattalashtirmaydi. Qutilarni yuklovchi joyida yaratadi.

| Bo'lim | Nima | Misol |
|---|---|---|
| `.text` | mashina kodi | funksiyalar |
| `.rodata` | faqat o'qiladigan ma'lumot | satr literallari, `const` globallar, `switch` jadvallari |
| `.data` | boshlang'ich qiymatli global/static | `int n = 5;` |
| `.bss` | nol qiymatli global/static | `int buf[1000];` — faylda **joy egallamaydi**, faqat hajmi yoziladi |
| `.symtab` | belgilar jadvali | funksiya va o'zgaruvchi nomlari |
| `.rela.text` | relokatsiyalar | "bu joyga X ning manzilini yoz" |
| `.debug_*` | debug ma'lumoti (`-g`) | gdb uchun qatorlar, turlar |

Quyida bitta fayl (`kv.c`) ning har bir qismi qaysi bo'limga tushishini ko'ramiz:

```c
/* kv.c - turli xil global narsalar */
int hisob = 3;                                  /* .data  : qiymati bor global */
static char bufer[100];                         /* .bss   : nol, static (lokal) */
static int yordamchi(int x) { return x * 2; }   /* .text  : static funksiya */

int kvadrat(int x)                              /* .text  : global funksiya */
{
    static int chaqiruvlar = 0;                 /* .bss   : funksiya ichidagi static */
    chaqiruvlar++;
    bufer[0] = (char)x;
    return yordamchi(x) * x / 2 + chaqiruvlar;
}

const char nom[] = "kvadrat";                   /* .rodata: o'zgarmas */
```

```console
$ gcc -Wall -Wextra -c kv.c -o kv.o
$ readelf -SW kv.o | grep -E '\.(text|data|bss|rodata|symtab|rela\.text) ' | sed 's/^ *\[ *[0-9]*\] //' | awk '{print $1, $2}'
.text PROGBITS
.rela.text RELA
.data PROGBITS
.bss NOBITS
.rodata PROGBITS
.symtab SYMTAB
$ nm kv.o
0000000000000000 b bufer
0000000000000064 b chaqiruvlar.0
0000000000000000 D hisob
0000000000000012 T kvadrat
0000000000000000 R nom
0000000000000000 t yordamchi
```

**Nima ko'rdik:** `readelf -S` bo'limlar ro'yxati: `.text` (kod), `.data`, `.bss`, `.rodata`, `.rela.text` (relokatsiyalar), `.symtab` (belgilar). `nm` har bir nomni qaysi bo'limda ekanini harf bilan ko'rsatdi (22.3).

`.bss` hiylasi: `static char buf[1 << 20];` faylni 1 MB ga kattalashtirmaydi — yuklovchi o'sha joyni nollar bilan to'ldiradi. ELF segmentida bu `memsz > filesz` bo'lib ko'rinadi (36-mashq: `memsz >= filesz` sharti).

## 22.3. Belgilar (symbols)

**Hayotdan misol: telefon kitobidagi ismlar.** Har bir funksiya va global o'zgaruvchi — kitobdagi yozuv: nom va manzil. `nm` — shu kitobni ko'rsatadi. `U` — "bu odamning raqamini hali bilmayman, kimdir berishi kerak".

Har bir global nom — **belgi**: nomi, qaysi bo'limda, qiymati (siljish), turi, ko'rinishi. `nm` harflari:

| Harf | Ma'nosi |
|---|---|
| `T` / `t` | `.text` da (funksiya): katta — **global**, kichik — **lokal** (`static`) |
| `D` / `d` | `.data` da (qiymatli o'zgaruvchi) |
| `B` / `b` | `.bss` da (nol o'zgaruvchi) |
| `R` / `r` | `.rodata` da (o'zgarmas) |
| `U` | **U**ndefined — boshqa joyda, linker topishi kerak |

Yuqoridagi `nm kv.o` ga qaytsak: `hisob` — `D` (global, `.data`); `kvadrat` — `T` (global funksiya); `yordamchi` — `t` (**static** funksiya, kichik harf); `bufer` — `b` (static, `.bss`); `nom` — `R` (global `.rodata`).
`chaqiruvlar.0` — funksiya ichidagi `static`: kompilyator nomiga raqam qo'shadi (nomlar to'qnashmasin).

Katta harf — global (boshqa fayllar ko'radi), kichik — lokal (`static`, 5 va 11-boblar).

**Linker qoidalari:**

- Har bir `U` belgi aynan **bitta** global ta'rifga ulanishi kerak. Yo'q — `undefined reference`; ikkita — `multiple definition`.
- Kutubxonalar (`.a`) chapdan o'ngga ko'riladi, arxivdan faqat kerakli `.o` olinadi (11-bob).

### `weak` belgi — "kuchli ta'rif bo'lmasa, shuni ishlat"

```c
/* zaif.c - weak (zaif) belgi */
#include <stdio.h>

__attribute__((weak)) void salom(void)          /* sukut bo'yicha amalga oshirish */
{
    puts("sukut bo'yicha salom");
}

int main(void)
{
    salom();
    return 0;
}
```

```c
/* kuchli.c - zaif ta'rifni almashtiradi */
#include <stdio.h>

void salom(void)
{
    puts("kuchli ta'rif ishladi");
}
```

```console
$ gcc -Wall -Wextra zaif.c -o zaif_yolg
$ ./zaif_yolg
sukut bo'yicha salom
$ gcc -Wall -Wextra zaif.c kuchli.c -o zaif_kuchli
$ ./zaif_kuchli
kuchli ta'rif ishladi
```

**Nima ko'rdik:** `kuchli.c` bo'lmaganda `zaif.c` dagi ta'rif ishladi; `kuchli.c` qo'shilganda **u** g'alaba qildi (`multiple definition` xatosi **chiqmadi** — chunki bittasi `weak`). Kutubxonalarda sukut bo'yicha
amalga oshirishni almashtirish uchun shunday qilinadi; yadroda ham keng qo'llanadi.

## 22.4. Relokatsiya — "manzilni keyin yozaman"

**Hayotdan misol: taklifnomadagi bo'sh joy.** To'y taklifnomasi oldindan chop etilgan, lekin restoran manzili uchun bo'sh joy qoldirilgan: "manzil keyin yoziladi". Kompilyator `kvadrat` ning manzilini
bilmaydi — `.o` faylda bo'sh joy va "bu yerga `kvadrat` manzilini yoz" degan eslatma qoldiradi. Linker bo'sh joylarni to'ldiradi.

```c
/* asosiy.c - boshqa faylda ta'riflangan funksiyani chaqiradi */
int kvadrat(int x);

int main(void)
{
    return kvadrat(7);
}
```

```console
$ gcc -Wall -Wextra -c asosiy.c -o asosiy.o
$ objdump -dr asosiy.o | grep -E 'call|R_X86'
   d:	e8 00 00 00 00       	call   12 <main+0x12>
			e: R_X86_64_PLT32	kvadrat-0x4
```

**Nima ko'rdik:**

```text
  d:  e8 00 00 00 00      call  12 <main+0x12>          <- call buyrug'i; manzil joyi (4 bayt) - NOLLAR
          e: R_X86_64_PLT32   kvadrat-0x4                <- relokatsiya: "e siljishiga kvadrat manzilini yoz"
```

`call` (`e8`) dan keyingi 4 bayt hozircha `00 00 00 00` — **bo'sh joy**. Quyidagi qator (`R_X86_64_PLT32 kvadrat`) — eslatma: "shu joyga `kvadrat` ning (nisbiy) manzilini yozing".

Linker: 1) hamma `.o` larning bo'limlarini birlashtiradi (hamma `.text` lar bitta `.text` ga), 2) har bir belgiga yakuniy manzil beradi, 3) har bir relokatsiyani bajarib, bo'sh joylarga haqiqiy manzillarni yozadi.
Linker skripti (18-bob) aynan 1-2-qadamni boshqaradi: qaysi bo'lim qaysi manzilga.

## 22.5. Statik va dinamik bog'lash

**Hayotdan misol: kitobni sotib olish va kutubxonadan olish.** Statik: kerakli kutubxona kodini dasturingiz ichiga **nusxalaysiz** — dastur katta, lekin mustaqil (kitobni sotib olish). Dinamik: dastur faqat
"menga libc kerak" deb yozadi, ishga tushganda umumiy nusxadan foydalanadi — dastur kichik, xotirada bitta libc hamma dasturlar uchun (kutubxonadan olish).

**Statik:** kutubxona kodi dastur fayliga **ko'chiriladi**. Dastur mustaqil, lekin katta; kutubxona yangilansa — qayta bog'lash kerak. MyOS dasturlari shunday (`libc.a`).

**Dinamik:** dasturda faqat "menga `libc.so.6` dagi `puts` kerak" degan yozuv. Ishga tushganda **dinamik yuklovchi** (`ld-linux.so`) kutubxonani xotiraga yuklaydi va manzillarni ulaydi.
Afzalliklari: kutubxona xotirada bir marta (hamma dasturlar bo'lishadi), yangilash oson.

```c
/* salom_bog.c - ikki xil bog'lash uchun bir xil dastur */
#include <stdio.h>

int main(void)
{
    puts("salom");
    return 0;
}
```

```console
$ gcc -Wall -Wextra salom_bog.c -o dinamik
$ gcc -Wall -Wextra -static salom_bog.c -o statik
$ ls -l dinamik statik | awk '{print $9 ": " $5 " bayt"}'
dinamik: 15968 bayt
statik: 785360 bayt
$ ldd dinamik | sed -E 's/\(0x[0-9a-f]+\)//'
	linux-vdso.so.1 
	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 
	/lib64/ld-linux-x86-64.so.2 
$ ldd statik
	not a dynamic executable
```

**Nima ko'rdik:** bir xil dastur — ikki o'lcham: **dinamik** ~16 KB (kod faqat `puts` ni so'raydi), **statik** ~780 KB (libc'ning kerakli qismi ichiga ko'chirildi). `ldd` dinamik dastur bog'liq
kutubxonalarni (`libc.so.6`, dinamik yuklovchi) ko'rsatdi; statik uchun — "not a dynamic executable" (hech kimga bog'liq emas).

### PLT va GOT — dinamik chaqiruv qanday ishlaydi

Kutubxona har safar boshqa manzilga yuklanishi mumkin, lekin dastur kodi (`.text`) faqat o'qiladi va bo'lishiladi — uni o'zgartirib bo'lmaydi. Yechim — bilvosita chaqiruv:

```text
call puts@PLT   ->  PLT (kichik "trambolin" kod)  ->  jmp *GOT[puts]  ->  puts
```

- **GOT** (Global Offset Table) — manzillar jadvali, **yoziladigan** ma'lumot bo'limida. Yuklovchi unga haqiqiy manzillarni yozadi.
- **PLT** (Procedure Linkage Table) — har bir tashqi funksiya uchun kichik kod: GOT'dagi manzilga sakraydi. "Dangasa bog'lash": birinchi chaqiruvda GOT yuklovchiga ko'rsatadi, u manzilni topib GOT'ga yozadi,
  keyingi chaqiruvlar to'g'ridan-to'g'ri boradi.

```console
$ objdump -d dinamik | grep -E 'call.*puts|jmp.*puts'
    1054:	ff 25 76 2f 00 00    	jmp    *0x2f76(%rip)        # 3fd0 <puts@GLIBC_2.2.5>
    115b:	e8 f0 fe ff ff       	call   1050 <puts@plt>
```

**Nima ko'rdik:** `main` ichida `call ... <puts@plt>` (bevosita `puts` emas — PLT trambolini), PLT ichida esa `jmp *...(%rip) # <puts@GLIBC...>` — GOT yozuvi orqali sakrash.

Xavfsizlik jihati: GOT yoziladigan — hujumchilar uni o'zgartirishga urinadi. "RELRO" himoyasi yuklashdan keyin GOT'ni faqat o'qiladigan qiladi.

> **Eslab qoling:** statik — kutubxona ichingda (katta, mustaqil); dinamik — alohida `.so` (kichik, umumiy); dinamik chaqiruv PLT → GOT orqali boradi.

## 22.6. PIC va PIE — joyi o'zgaruvchan kod

**Hayotdan misol: g'ildirakli uy.** Oddiy uy bitta joyga qurilgan. G'ildirakli uyni istalgan joyga qo'yish mumkin — ichidagi hamma narsa ishlayveradi, chunki manzillar "uyning boshidan 5 metr" kabi nisbiy yozilgan.
Xavfsizlik uchun Linux dasturni har safar boshqa manzilga yuklaydi (**ASLR**).

**PIC** (position-independent code) — istalgan manzilda ishlaydigan kod: global ma'lumotga `rip`ga nisbiy murojaat qiladi (`lea rax, [rip + 0x1234]`). **PIE** — butun dastur shunday; OS uni har safar tasodifiy
manzilga yuklaydi (**ASLR** — hujumchi manzillarni oldindan bilmasin).

```c
/* aslr.c - har safar boshqa manzil */
#include <stdio.h>

int main(void)
{
    int x;
    printf("main=%p stek=%p\n", (void *)main, (void *)&x);
    return 0;
}
```

```console
$ gcc -Wall -Wextra aslr.c -o aslr
$ a=$(./aslr); b=$(./aslr); [ "$a" != "$b" ] && echo "ikki ishga tushirishda manzillar FARQ qildi (ASLR)"
ikki ishga tushirishda manzillar FARQ qildi (ASLR)
$ readelf -h aslr | grep Type
  Type:                              DYN (Position-Independent Executable file)
```

**Nima ko'rdik:** ikki ishga tushirishda `main` va stek manzillari **boshqa-boshqa** chiqdi (ASLR). `readelf -h` turi `DYN` ("Position-Independent Executable") — zamonaviy dasturlar shunday.

Yadro esa odatda aniq manzilda (`-fno-pic -fno-pie -mcmodel=kernel`, 18-bob), lekin Linux ham yadro uchun KASLR (tasodifiy joylashtirish) qiladi.

## 22.7. Dastur qanday ishga tushadi (`exec` dan `main` gacha)

```text
1. Shell: fork() + execve("/bin/ls", argv, envp)
2. Yadro (exec):
   - faylni ochadi, ELF sarlavhasini TEKSHIRADI (36-mashq)
   - eski manzil maydonini tashlab, yangisini yaratadi
   - har bir PT_LOAD segmentni xaritalaydi (kod: R-X, ma'lumot: RW-, .bss: nollar)
   - stek yaratadi, unga argv, envp, auxv ni yozadi
   - dinamik bo'lsa: ld-linux.so ni ham yuklaydi va rip = uning kirishi
   - aks holda rip = ELF kirish nuqtasi (_start)
3. _start (crt0): stekdan argc/argv ni olib, libc'ni boshlab, main(argc, argv) ni chaqiradi
4. main qaytgach: exit(qiymat) -> buferlarni tozalash -> _exit syscall
```

Buni `strace` bilan o'z ko'zingiz bilan ko'ramiz:

```console
$ strace -o iz.txt -e trace=execve,openat ./dinamik > /dev/null
$ grep -o 'execve("[^"]*"' iz.txt
execve("./dinamik"
$ grep -c 'libc.so.6' iz.txt
1
$ grep -o 'openat([^,]*, "[^"]*libc.so.6"' iz.txt | sed 's/^openat([^,]*, //'
"/lib/x86_64-linux-gnu/libc.so.6"
```

**Nima ko'rdik:** birinchi syscall — `execve("./dinamik", ...)`; keyin dinamik yuklovchi `libc.so.6` ni **ochdi** (`openat`) — 22.5 dagi "dinamik bog'lash" ishga tushish paytida shunday bajariladi.

MyOS'da bu yo'lning hammasi o'qish uchun ochiq: `kernel/proc/exec.c`, `kernel/sys/elf.c`, `user/libc/crt0.asm`, `user/linker.ld`.

## Hayotdan misol va to'liq dastur

**`.data` va `.bss` ning fayl hajmiga ta'siri.** Bir xil million baytlik massiv: biri nol (`.bss`), biri qiymatli (`.data`). Faylning haqiqiy hajmi qanchaga farq qiladi?

```c
/* chamadon.c - bo'limlar: bir xil massiv .bss da va .data da */
#include <stdio.h>

/* Global (static emas): kompilyator uni "ishlatilmaydi" deb olib tashlay olmaydi */
#ifdef TOLDIRILGAN
char quti[1000000] = { 1 };                     /* qiymati bor -> .data: faylda 1 MB */
#else
char quti[1000000];                             /* nol -> .bss: faylda faqat o'lchami */
#endif

static const char xabar[] = "chamadon tayyor";  /* .rodata */
int yuklar_soni = 3;                            /* .data */

int main(void)
{
    quti[999999] = 7;
    printf("%s: %d ta yuk, oxirgi quti = %d\n", xabar, yuklar_soni, quti[999999]);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 chamadon.c -o bss_bilan
$ gcc -Wall -Wextra -O2 -DTOLDIRILGAN chamadon.c -o data_bilan
$ ./bss_bilan
chamadon tayyor: 3 ta yuk, oxirgi quti = 7
$ size bss_bilan data_bilan
   text	   data	    bss	    dec	    hex	filename
   1484	    604	1000032	1002120	  f4a88	bss_bilan
   1484	1000648	      8	1002140	  f4a9c	data_bilan
$ ls -l bss_bilan data_bilan | awk '{print $9 ": " $5 " bayt"}'
bss_bilan: 16072 bayt
data_bilan: 1016112 bayt
$ nm bss_bilan | grep "quti\|xabar\|yuklar_soni"
0000000000004040 B quti
0000000000002030 r xabar
0000000000004010 D yuklar_soni
$ nm data_bilan | grep "quti"
0000000000004040 D quti
```

**Bu dastur nima qiladi (umumiy):** bitta dastur ikki xil yig'iladi: (1) `quti` massivi nol → `.bss`; (2) `-DTOLDIRILGAN` bilan massivning birinchi elementi 1 → butun massiv `.data` ga o'tadi. Ikkala dastur bir xil ishlaydi,
lekin **fayl hajmi** keskin farq qiladi.

**Nima ko'rdik:** `size` natijasida: birinchi dasturda million bayt `bss` ustunida (faylda joy yo'q), ikkinchisida — `data` ustunida va fayl ~1 MB ga katta (`ls -l`: ~16 KB va ~1 MB). `nm` da: `B` — `.bss`, `D` — `.data`,
`r` — `.rodata` (kichik harf — `static`, faqat shu faylda ko'rinadi). Bitta `quti` birinchi dasturda `B`, ikkinchisida `D`.

**Sinab ko'ring:** `readelf -S bss_bilan | grep -A1 "\.bss"` bilan `.bss` bo'limining o'lchamini toping. `ldd bss_bilan` — dastur qaysi dinamik kutubxonalarga bog'liq? `-static` bilan yig'ib, hajmini solishtiring.

## Bob xulosasi (yodlash uchun)

1. **ELF** — dastur fayl formati: `.o` (REL, linker uchun), bajariladigan/PIE (yuklovchi uchun). **Bo'limlar** — linker uchun (`.text`, `.data`, `.bss`, `.rodata`), **segmentlar** — yuklovchi uchun.
2. `.bss` fayl hajmini oshirmaydi (faqat o'lcham yoziladi; yuklovchi nollar bilan to'ldiradi); `.data` — oshiradi.
3. Belgilar: `nm` harflari — `T/t` (kod), `D/d` (ma'lumot), `B/b` (`.bss`), `R/r` (`.rodata`), `U` (tashqarida); katta harf — global, kichik — `static`.
4. **Relokatsiya** — `.o` dagi bo'sh joy + "bu yerga X manzilini yoz"; linker hamma bo'limlarni birlashtirib, manzillarni to'ldiradi. `weak` — "kuchli ta'rif bo'lmasa, shuni ishlat".
5. Statik (kutubxona ichingda, katta) va dinamik (`.so`, kichik; PLT → GOT orqali chaqiriladi); PIE + ASLR — dastur har safar boshqa manzilga yuklanadi; `exec` ELF segmentlarini xaritalaydi.

## Savol-javob

**`static` global o'zgaruvchi va funksiya ichidagi `static` — belgilar jadvalida qanday?**
Ikkalasi ham lokal belgi (`b`/`d`/`t`). Funksiya ichidagisining nomiga kompilyator raqam qo'shadi (`chaqiruvlar.0`) — nomlar to'qnashmasin.

**Nega `-g` fayl hajmini oshiradi, lekin dastur tezligiga ta'sir qilmaydi?**
Debug ma'lumot alohida bo'limlarda; yuklovchi ularni xotiraga yuklamaydi. MyOS diskka `--strip-debug` qilingan nusxani qo'yadi, to'liq ELF esa gdb uchun `build/` da qoladi.

**Nega statik dastur dinamikdan shuncha katta?**
Statik dastur libc'ning kerakli qismlarini **ichiga ko'chiradi** (ba'zan `printf`, `malloc` va hokazo butun mexanizmi); dinamikda esa ular alohida `.so` da va hamma dasturlar bo'lishadi.

## O'zingizni tekshiring

1. `.bss` nega fayl hajmini oshirmaydi?
2. `nm` chiqishidagi `T`, `t`, `U` nimani bildiradi?
3. Relokatsiya nima uchun kerak?
4. PLT va GOT qanday ishlaydi, nega bevosita `call puts` emas?
5. ELF bo'limlari va segmentlari farqi?

<details><summary>Javoblar</summary>

1. Unda faqat hajm yoziladi; yuklovchi xotirani nollar bilan to'ldiradi (`memsz > filesz`).
2. `T` — global funksiya, `t` — lokal (static), `U` — aniqlanmagan (tashqaridan kerak).
3. Kompilyator yakuniy manzillarni bilmaydi; linker ularni keyin joyiga yozadi.
4. Kod faqat o'qiladi va bo'lishiladi; manzil yoziladigan GOT'da turadi, PLT undagi manzilga sakraydi.
5. Bo'limlar — linker uchun mantiqiy qismlar; segmentlar — yuklovchi uchun "xotiraga nimani qayerga".
</details>

## Mashq

- Uch faylli dasturingiz (1-bob) uchun `nm`, `readelf -S`, `objdump -dr` chiqishlarini o'qing.
- MyOS'da: `readelf -l build/kernel.elf` — segmentlar manzillari (`0xffffffff80...`) va linker skripti bilan solishtiring.
- **36-mashq** (ELF tahlilchisi) — agar hali qilmagan bo'lsangiz.

<!-- loyiha:boshi -->
## Loyiha: ELF sarlavha o'quvchisi (mini `readelf -h`)

**Maqsad:** har bir dastur va `.o` fayl ichida nima borligini **o'zingiz o'qish**. ELF — Linux'dagi barcha bajariladigan
fayllarning formati; yadro `exec` da aynan shu sarlavhani o'qiydi (22.1, 22.7).
**Bobdan ishlatiladi:** ELF sarlavhasi, `e_type` (REL/EXEC/DYN), `e_entry`, bo'limlar va segmentlar soni.

**Talab:** `elfbosh [fayl]` — fayl (yoki o'zining bajariladigan fayli, `/proc/self/exe`) ning ELF sarlavhasini o'qib, asosiy
maydonlarni chiqarsin. Fayl ELF bo'lmasa — xabar va chiqish kodi 1.
**Asosiy g'oya:** fayl boshidagi `Elf64_Ehdr` tuzilmasi (`<elf.h>` da tayyor) — 64 bayt; uni `fread` bilan shunchaki **tuzilmaga** o'qiymiz.
Birinchi 4 bayt — sehrli (magic): `0x7F 'E' 'L' 'F'`.

```c
/* elfbosh.c - ELF sarlavhasini o'qish */
#include <elf.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    const char *yol = argc > 1 ? argv[1] : "/proc/self/exe";
    FILE *f = fopen(yol, "rb");
    if (!f) {
        perror(yol);
        return 1;
    }
    Elf64_Ehdr h;
    size_t o = fread(&h, sizeof(h), 1, f);
    fclose(f);
    if (o != 1 || memcmp(h.e_ident, ELFMAG, SELFMAG) != 0) {
        printf("%s: ELF emas\n", yol);
        return 1;
    }

    printf("Sehrli belgilar : %02x %c%c%c\n", h.e_ident[0], h.e_ident[1], h.e_ident[2], h.e_ident[3]);
    printf("Sinf            : %s\n", h.e_ident[EI_CLASS] == ELFCLASS64 ? "ELF64" : "ELF32");
    printf("Bayt tartibi    : %s\n", h.e_ident[EI_DATA] == ELFDATA2LSB ? "little-endian" : "big-endian");
    const char *tur = h.e_type == ET_REL ? "REL (obyekt fayl, .o)"
                      : h.e_type == ET_EXEC ? "EXEC (bajariladigan)"
                      : h.e_type == ET_DYN ? "DYN (PIE yoki .so)" : "boshqa";
    printf("Turi            : %s\n", tur);
    const char *mashina = h.e_machine == EM_X86_64 ? "x86-64"
                          : h.e_machine == EM_AARCH64 ? "AArch64"
                          : h.e_machine == EM_RISCV ? "RISC-V" : "boshqa";
    printf("Mashina         : %s\n", mashina);
    printf("Kirish nuqtasi  : 0x%lx\n", (unsigned long)h.e_entry);
    printf("Segmentlar soni : %u (yuklovchi uchun)\n", h.e_phnum);
    printf("Bo'limlar soni  : %u (linker uchun)\n", h.e_shnum);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g elfbosh.c -o elfbosh
$ ./elfbosh
Sehrli belgilar : 7f ELF
Sinf            : ELF64
Bayt tartibi    : little-endian
Turi            : DYN (PIE yoki .so)
Mashina         : x86-64
Kirish nuqtasi  : 0x1120
Segmentlar soni : 13 (yuklovchi uchun)
Bo'limlar soni  : 37 (linker uchun)
$ readelf -h ./elfbosh | grep -E "Class|Data:|Type|Machine|Entry"
  Class:                             ELF64
  Data:                              2's complement, little endian
  Type:                              DYN (Position-Independent Executable file)
  Machine:                           Advanced Micro Devices X86-64
  Entry point address:               0x1120
$ gcc -c elfbosh.c -o e.o && ./elfbosh e.o | grep -E "Turi|Segment|Bo'lim"
Turi            : REL (obyekt fayl, .o)
Segmentlar soni : 0 (yuklovchi uchun)
Bo'limlar soni  : 14 (linker uchun)
$ ./elfbosh elfbosh.c; echo "chiqish kodi: $?"
elfbosh.c: ELF emas
chiqish kodi: 1
```

Sizning dasturingiz `readelf -h` bilan bir xil ma'lumot beradi. `.o` faylda **segmentlar 0** (yuklanmaydi), bo'limlar bor;
tayyor dasturda esa ikkalasi ham bor (22.1). `Kirish nuqtasi` `main` emas, `_start` — `main` ni libc chaqiradi (22.7).

**Kengaytiring:** `e_shoff` (bo'limlar jadvali offseti) va `e_shstrndx` ni ham chiqaring. `/bin/ls` bilan sinang.

## Mustaqil loyiha: bo'limlar ro'yxati (`readelf -S`) ★★★

**Vazifa:** ELF faylning **bo'limlar jadvalini** o'qib, har bir bo'lim nomi, turi, bayrog'i, offseti va hajmini chiqaring.
Fayl: `bolimlar.c`. Aniq, o'zgarmaydigan natija uchun sizga tayyor **kichik ELF fayl** beriladi:
`darslik/loyihalar/22_elf_bolimlar/namuna.elf` (kompilyatorga bog'liq emas — qo'lda yasalgan, 6 ta bo'lim, taxminan 500 bayt).

**Bilimingiz kerak:**
- `Elf64_Ehdr.e_shoff` — bo'limlar jadvali fayl boshidan qancha uzoqda; `e_shnum` — nechta; `e_shentsize` — har biri necha bayt;
  `e_shstrndx` — **nomlar jadvali** (`.shstrtab`) bo'limining indeksi.
- Har bir yozuv — `Elf64_Shdr`: `sh_name` (nomning `.shstrtab` ichidagi **offseti**), `sh_type`, `sh_flags`, `sh_offset`, `sh_size`.
- Nom — `.shstrtab` ma'lumotining `sh_name` baytidan boshlanuvchi `'\0'` bilan tugaydigan satr.

**Talab:**
1. `argv[1]` — fayl. Sehrli belgilarni, `ELFCLASS64` va little-endian ni tekshiring; bo'lmasa aynan `ELF emas` deb chiqarib, kod **1** bilan chiqing.
2. Sarlavha qatorlari (aniq shakl kutilgan natijada), keyin **har bo'lim uchun bitta qator**:
   `"[%2d] %-10s %-9s %-5s offset=%-5lu hajm=%lu\n"` — tartib raqami, nomi, turi, bayroqlari, offseti, hajmi.
3. Tur nomlari: `NULL`, `PROGBITS`, `NOBITS`, `STRTAB`, `SYMTAB` (boshqasi `BOSHQA`).
4. Bayroq harflari (`sh_flags`): `W` (yozish, `SHF_WRITE`), `A` (xotiraga yuklanadi, `SHF_ALLOC`), `X` (bajariladi, `SHF_EXECINSTR`) —
   shu tartibda; yo'q bayroqlar tushirib qoldiriladi.
5. Sarlavha qatorlari: `Sinf: ELF64, little-endian`, `Turi: REL` (yoki `EXEC`/`DYN`), `Mashina: x86-64`,
   `Bo'limlar soni: N`, `Nomlar jadvali: K-bo'lim`.

**1-sinov:** `./dastur namuna.elf`

```text
Sinf: ELF64, little-endian
Turi: REL
Mashina: x86-64
Bo'limlar soni: 6
Nomlar jadvali: 5-bo'lim
[ 0]            NULL            offset=0     hajm=0
[ 1] .text      PROGBITS  AX    offset=64    hajm=16
[ 2] .data      PROGBITS  WA    offset=80    hajm=8
[ 3] .bss       NOBITS    WA    offset=88    hajm=4096
[ 4] .rodata    PROGBITS  A     offset=88    hajm=12
[ 5] .shstrtab  STRTAB          offset=100   hajm=36
```

**2-sinov:** `./dastur emas.txt; echo "chiqish kodi: $?"` (matnli fayl, `darslik/loyihalar/22_elf_bolimlar/emas.txt`)

```text
ELF emas
chiqish kodi: 1
```

**Maslahat** (yechim emas):
- Avval sarlavhani `Elf64_Ehdr` ga o'qing. `fseek(f, e_shoff, SEEK_SET)` bilan jadvalga o'ting va `e_shnum` ta `Elf64_Shdr` ni massivga o'qing.
- Nomlar: `shstr = bo'limlar[e_shstrndx]`; uning `sh_offset` iga `fseek` qilib, `sh_size` bayt o'qing; keyin `nomlar + sh_name`.
- `NOBITS` (`.bss`) bo'limining `sh_size` i bor, lekin faylda joy yo'q — bu yozuvda ko'rasiz (22.2).
- Natijani `readelf -S namuna.elf` bilan solishtiring — nomlari va hajmlari mos kelishi kerak.
- Har `fread` natijasini tekshiring; buzuq fayl uchun ham dastur qulamasin.

**Tekshirish:**

```bash
D=~/C_loyha/darslik/loyihalar/22_elf_bolimlar
gcc -Wall -Wextra -g -fsanitize=address,undefined bolimlar.c -o dastur
./dastur $D/namuna.elf | diff - $D/kutilgan.txt && echo "1: TO'G'RI"
(./dastur $D/emas.txt; echo "chiqish kodi: $?") | diff - $D/kutilgan_2.txt && echo "2: TO'G'RI"
readelf -S $D/namuna.elf        # qiyoslash uchun
```
<!-- loyiha:oxiri -->

Keyingi bob: [23-bob. Jarayonlar va rejalashtirish (scheduling)](23-jarayonlar-scheduling.md)
