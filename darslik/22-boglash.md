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

<!-- katta:boshi -->
## Katta loyiha: ELF belgilar jadvalini o'qish (mini `nm`)

**Umumiy fikr.** `gcc -c` natijasi — **obyekt fayl** (`.o`). U oddiy baytlar emas, balki **tuzilgan** ELF fayli: sarlavha, bo'limlar (`.text` — kod, `.data` — boshlang'ich qiymatli o'zgaruvchilar, `.bss` — nollangan) va **belgilar jadvali** (`.symtab`): "qaysi nom qayerda va qanday ko'rinadi". Linker (22-bob) aynan shu jadval orqali fayllarni **bir-biriga ulaydi**. Bu bosqichda jadvalni **o'zimiz** `nm` kabi o'qiymiz.

**Hayotiy o'xshatish:** kitob mundarijasi. "Funksiya `qosh` — 0-bet, hajmi 4" degan yozuvlar: kerakli narsani **darrov topish** uchun.

### ELF faylning tuzilishi (qisqa)

```text
+------------------+
| ELF sarlavhasi   |  <- "bo'limlar jadvali qayerda" (e_shoff), nechta (e_shnum)
+------------------+
| .text  .data ... |  <- bo'limlar (kod, ma'lumot)
+------------------+
| .symtab          |  <- belgilar: har biri Elf64_Sym (nom, qiymat, hajm, tur, bog'lanish, bo'lim)
| .strtab          |  <- nomlar matni (belgi faqat NOM INDEKSini saqlaydi)
+------------------+
| bo'limlar jadvali|  <- har bo'lim haqida: tur, siljish, hajm
+------------------+
```

Bizning dastur shu zanjir bo'yicha yuradi: **sarlavha → bo'limlar jadvali → `.symtab` → `.strtab` (nomlar)**.

### Tekshiriladigan obyekt fayl

`demo.s` — assembly bilan yozilgan kichik fayl. U ataylab **turli belgilarni** o'z ichiga oladi: global va lokal funksiyalar, ma'lumot, BSS va **tashqi** (`puts`) chaqiruv:

```text
# demo.s - ELF belgilar jadvalini o'rganish uchun kichik obyekt fayl (as bilan yig'iladi: natijasi har safar bir xil)
        .intel_syntax noprefix
        .text

        .globl  qosh                    # GLOBAL funksiya: boshqa fayllar ko'radi
        .type   qosh, @function
qosh:
        lea     eax, [rdi + rsi]
        ret
        .size   qosh, .-qosh

        .globl  kopaytir
        .type   kopaytir, @function
kopaytir:
        mov     eax, edi
        imul    eax, esi
        ret
        .size   kopaytir, .-kopaytir

        .type   yordamchi, @function    # LOCAL funksiya (.globl yo'q): faqat shu faylda ko'rinadi
yordamchi:
        xor     eax, eax
        ret
        .size   yordamchi, .-yordamchi

        .globl  tashqi_chaqir
        .type   tashqi_chaqir, @function
tashqi_chaqir:
        sub     rsp, 8
        call    puts@PLT                # puts shu faylda YO'Q: UNDEFINED belgi (linker keyin topadi)
        add     rsp, 8
        ret
        .size   tashqi_chaqir, .-tashqi_chaqir

        .data
        .globl  hisoblagich
        .type   hisoblagich, @object
        .size   hisoblagich, 4
hisoblagich:
        .long   7

        .bss
        .lcomm  bufer, 64               # BSS dagi LOCAL o'zgaruvchi (nollar bilan boshlanadi)

        .section .note.GNU-stack, "", @progbits
```

| Belgi | Tur | Bog'lanish | Nega qiziq |
|---|---|---|---|
| `qosh`, `kopaytir`, `tashqi_chaqir` | FUNC | **GLOBAL** | boshqa fayllar ko'radi (`.globl`) |
| `yordamchi` | FUNC | **LOCAL** | `.globl` yo'q — faqat shu faylda ko'rinadi (C da `static` funksiya) |
| `hisoblagich` | OBJECT | GLOBAL | `.data` bo'limida (boshlang'ich qiymati 7) |
| `bufer` | OBJECT | LOCAL | `.bss` (nollar bilan boshlanadi, faylda joy egallamaydi) |
| `puts` | NOTYPE | GLOBAL, **UND** | shu faylda **yo'q** — linker keyin libc dan topadi (undefined) |

### Dastur: sarlavhadan belgigacha

Birinchi qism — faylni xotiraga o'qib, **ELF sarlavhasi orqali** `.symtab` ni topish:

```c
    const Elf64_Ehdr *eh = (const Elf64_Ehdr *)fayl;
    if (memcmp(eh->e_ident, ELFMAG, SELFMAG) != 0 || eh->e_ident[EI_CLASS] != ELFCLASS64) {
        fprintf(stderr, "%s: 64 bitli ELF emas\n", argv[1]);
        return 1;
    }

    const Elf64_Shdr *bolimlar = (const Elf64_Shdr *)(fayl + eh->e_shoff);   /* bo'limlar jadvali */
    const char *bolim_nomlari = (const char *)(fayl + bolimlar[eh->e_shstrndx].sh_offset);

    const Elf64_Shdr *symtab = NULL;
    for (int i = 0; i < eh->e_shnum; i++)
        if (bolimlar[i].sh_type == SHT_SYMTAB)
            symtab = &bolimlar[i];
    if (!symtab) {
        fprintf(stderr, "%s: .symtab yo'q (strip qilingan)\n", argv[1]);
        return 1;
    }
    const Elf64_Sym *belgilar = (const Elf64_Sym *)(fayl + symtab->sh_offset);
    size_t soni = symtab->sh_size / sizeof(Elf64_Sym);
    const char *nomlar = (const char *)(fayl + bolimlar[symtab->sh_link].sh_offset);   /* sh_link: nomlar jadvali */
```

Muhim joylari: `fayl + eh->e_shoff` — bo'limlar jadvali **faylning o'zida** qayerdaligini sarlavha aytadi (fayl boshidan siljish — "ko'rsatkich arifmetikasi", 7-bob). `sh_link` — bu bo'lim **qaysi boshqa bo'lim bilan bog'liq** ekanini aytadi: `.symtab` uchun u — nomlar jadvali `.strtab`.

Ikkinchi qism — **manzil bo'yicha qidirish**: "bu manzil qaysi funksiya ichida?" (xuddi `addr2line` yoki yadro `oops` xabaridagi `qosh+0x5/0x20`):

```c
    unsigned long m = strtoul(argv[2], NULL, 16);
    for (size_t i = 0; i < soni; i++) {
        const Elf64_Sym *s = &belgilar[i];
        if (ELF64_ST_TYPE(s->st_info) == STT_FUNC && s->st_shndx != SHN_UNDEF && m >= s->st_value &&
            m < s->st_value + s->st_size) {
            printf("0x%lx: %s + %lu\n", m, nomlar + s->st_name, m - s->st_value);
            return 0;
        }
    }
    printf("0x%lx: hech qaysi funksiyaga tegishli emas\n", m);
```

```console
$ cd katta_loyiha/tizim/22_elf_sym
$ as demo.s -o demo.o
$ gcc -Wall -Wextra -g -fsanitize=address,undefined elf_sym.c -o elf_sym
$ ./elf_sym demo.o
#   qiymat   hajm  tur     bog'    bo'lim    nom
0   00000000 0     NOTYPE  LOCAL   UND       
1   0000000a 3     FUNC    LOCAL   .text     yordamchi
2   00000000 64    OBJECT  LOCAL   .bss      bufer
3   00000000 4     FUNC    GLOBAL  .text     qosh
4   00000004 6     FUNC    GLOBAL  .text     kopaytir
5   0000000d 14    FUNC    GLOBAL  .text     tashqi_chaqir
6   00000000 0     NOTYPE  GLOBAL  UND       puts
7   00000000 4     OBJECT  GLOBAL  .data     hisoblagich
$ echo "--- manzil qidirish ---"
--- manzil qidirish ---
$ ./elf_sym demo.o 0
0x0: qosh + 0
$ ./elf_sym demo.o 5
0x5: kopaytir + 1
$ ./elf_sym demo.o 6
0x6: kopaytir + 2
$ ./elf_sym demo.o 9
0x9: kopaytir + 5
$ ./elf_sym demo.o 100; echo "chiqish kodi: $?"
0x100: hech qaysi funksiyaga tegishli emas
chiqish kodi: 1
$ echo "--- nm bilan solishtirish ---"
--- nm bilan solishtirish ---
$ echo "bizning FUNC belgilar: $(./elf_sym demo.o | awk '$4 == "FUNC"' | wc -l)"
bizning FUNC belgilar: 4
$ echo "nm ning T/t belgilari: $(nm demo.o | awk '$2 == "T" || $2 == "t"' | wc -l)"
nm ning T/t belgilari: 4
```

**Nima ko'rdik:**

- **Jadval:** `qosh` — `0x0` da, hajmi 4; `kopaytir` — `0x4` da, hajmi 6; `yordamchi` — `0xa` da, **LOCAL**; `puts` — **UND** (undefined): bu fayl uni ishlatadi, lekin **o'zida yo'q**.
- **Manzil qidirish:** `0x5` — `kopaytir + 1` (kopaytir `0x4` da boshlanadi, 5-bayt = 1-bayt ichkarida); `0x9` — `kopaytir + 5`, `0x6` — `kopaytir + 2`. `0x100` — hech qaysi funksiyaga tegishli emas → chiqish kodi 1. Aynan shu g'oya yadro "panic" xabarlarida ishlatiladi.
- **`nm` bilan solishtirish:** bizning 4 ta `FUNC` belgimiz `nm` ning `T`/`t` (kod bo'limidagi) belgilari soniga **teng** — dastur to'g'ri o'qiyapti.
- Nomlar `.strtab` da, belgining o'zida esa **nom indeksi** (`st_name`) bor: shuning uchun `nomlar + s->st_name`.

> **Eslab qoling:** ELF — **tuzilgan** fayl: sarlavha → jadvallar → ma'lumot. Hamma narsa **siljishlar** (offset) orqali topiladi. `.symtab` belgilari: **nom**, **qiymat** (manzil), **hajm**, **tur** (FUNC/OBJECT), **bog'lanish** (LOCAL/GLOBAL), **bo'lim**. **UND** belgi — "kimdir menga bu nomni bersin" (linkerning vazifasi). `strip` shu jadvalni o'chiradi — shuning uchun qaytarilgan fayllarda funksiya nomlari yo'q.

**O'zingiz qo'shing (yechimsiz):**

1. `elf_sym.c` ga **bo'limlar ro'yxati** rejimini qo'shing (`readelf -S` kabi): nom, siljish, hajm. `.text` hajmi `qosh+kopaytir+yordamchi+tashqi_chaqir` yig'indisiga tengmi?
2. `strip demo.o` dan keyin dasturni ishga tushiring — nima xabar chiqadi? (`readelf` yoki `nm` nima deydi?)
3. C faylda `static int x; int y = 5; extern int z;` yozing, `gcc -c` bilan yig'ing va `./elf_sym` bilan **har o'zgaruvchi qaysi bo'lim va qaysi bog'lanishda** ekanini tekshiring. Bashoratingiz to'g'rimi?
<!-- katta:oxiri -->

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

### Isitish: o'z ELF sarlavhangiz ★☆☆ — eng osoni, avval shuni qiling

Faqat 0–22-boblar kerak (ELF sarlavhasi, PIE).
Skeletni `isitish.c` ga **qo'lda** yozing (ko'chirmang), izohlarni o'qing va `TODO` joylarini to'ldiring.
"Namuna" qismlar tayyor — qolganini qanday yozishni ko'rsatadi. Skelet hozir ham ogohlantirishsiz yig'iladi:
har `TODO` dan keyin yig'ib, ishga tushirib boring.

```c
/* isitish.c - 22-bob, isitish: o'z ELF sarlavhangizni o'qing (/proc/self/exe - ishlayotgan dasturning fayli). */
#include <stdio.h>

int main(void)
{
    unsigned char h[64];                /* ELF64 sarlavhasi - 64 bayt (22.1; man 5 elf) */
    FILE *f = fopen("/proc/self/exe", "rb");
    if (!f || fread(h, 1, sizeof(h), f) != sizeof(h))
        return 1;
    fclose(f);

    /* 1) Birinchi 4 bayt: 0x7f 'E' 'L' 'F'. (Namuna - tayyor.) */
    printf("sehrli raqam: %02x %02x %02x %02x (\\x7fELF)\n", h[0], h[1], h[2], h[3]);

    /* 2) TODO: h[4] - sinf (1 = 32-bit, 2 = 64-bit), h[5] - bayt tartibi (1 = little-endian, 2 = big-endian).
     *    printf("sinf: %s, bayt tartibi: %s\n", ...) - ternar operator bilan matn tanlang.
     *    Natija: sinf: 64-bit, bayt tartibi: little-endian */

    /* 3) TODO: e_type - 16-baytdan 2 bayt, little-endian: tur = h[16] | h[17] << 8.
     *    2 = EXEC (qat'iy manzilli), 3 = DYN (PIE - istalgan manzilga yuklanadi, 22.6). -pie bilan yig'amiz.
     *    printf("tur: %u (%s)\n", tur, tur == 2 ? "EXEC" : tur == 3 ? "DYN - PIE" : "boshqa");
     *    Natija: tur: 3 (DYN - PIE) */

    /* 4) TODO: e_machine - 18-baytdan 2 bayt: 62 = x86-64.
     *    Natija: mashina: 62 (x86-64) */

    /* 5) e_entry - 24-baytdan 8 bayt: kirish nuqtasi (_start). Baytlarni oxiridan yig'amiz. (Namuna - tayyor.) */
    unsigned long kirish = 0;
    for (int i = 7; i >= 0; i--)
        kirish = kirish << 8 | h[24 + i];
    printf("kirish nuqtasi 0 emasmi: %d\n", kirish != 0);
    return 0;
}
```

**Kutilgan natija** (`darslik/loyihalar/22_elf_bolimlar/isitish.txt`):

```text
sehrli raqam: 7f 45 4c 46 (\x7fELF)
sinf: 64-bit, bayt tartibi: little-endian
tur: 3 (DYN - PIE)
mashina: 62 (x86-64)
kirish nuqtasi 0 emasmi: 1
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address,undefined -fPIE -pie isitish.c -o isitish
$ ./isitish | diff - ~/C_loyha/darslik/loyihalar/22_elf_bolimlar/isitish.txt && echo "TO'G'RI"
TO'G'RI
```

### Keyingi mashqlar

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
