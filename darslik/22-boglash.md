# 22-bob. Bog'lash (linking) va ELF — dastur fayli ichida nima bor

> **Bu bobdan keyin:** obyekt faylning tuzilishini (bo'limlar, belgilar jadvali, relokatsiyalar), linker
> nima qilishini, statik va dinamik bog'lash farqini, PLT/GOT'ni va dastur qanday yuklanishini bilasiz.
> `nm`, `readelf`, `objdump` bilan istalgan dasturning ichini ko'ra olasiz. (Odatda CS:APP 7-bobidan
> o'rganiladi.) Mashqlar: 36 (ELF tahlilchisi).

> **To'liq ishlaydigan misol:** [misollar/22_boglash.c](misollar/22_boglash.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## 22.1. Ikki xil ELF: obyekt va bajariladigan

Linux'da (va MyOS'da) hamma dastur fayllari **ELF** (Executable and Linkable Format) formatida:

| Tur | Fayl | Kim uchun |
|---|---|---|
| Relocatable (`ET_REL`) | `.o` | linker uchun: bo'limlar, belgilar, "hali to'ldirilmagan joylar" |
| Executable (`ET_EXEC`) | `a.out`, `kernel.elf` | OS/yuklovchi uchun: aniq manzillar |
| Shared object / PIE (`ET_DYN`) | `.so`, zamonaviy dasturlar | istalgan manzilga yuklanadi |

ELF'da ikki "ko'rinish" bor:
- **Bo'limlar (sections)** — linker uchun: `.text`, `.data`, `.bss`, `.rodata`, `.symtab`, `.rela.text`...
- **Segmentlar (program headers)** — yuklovchi uchun: "fayldagi shu qismni xotiraning shu manziliga shu
  ruxsatlar bilan qo'y" (`PT_LOAD`). 36-mashqda siz aynan shularni o'qidingiz; MyOS `exec` ham faqat
  segmentlarga qaraydi (`kernel/sys/elf.c`).

```bash
readelf -h dastur        # sarlavha: tur, mashina, kirish nuqtasi
readelf -S dastur.o      # bo'limlar
readelf -l dastur        # segmentlar
readelf -s dastur.o      # belgilar jadvali
objdump -d dastur.o      # mashina kodi
nm dastur.o              # belgilar qisqacha
```

## 22.2. Asosiy bo'limlar

| Bo'lim | Nima | Misol |
|---|---|---|
| `.text` | mashina kodi | funksiyalar |
| `.rodata` | faqat o'qiladigan ma'lumot | satr literallari, `const` globallar, `switch` jadvallari |
| `.data` | boshlang'ich qiymatli global/static | `int n = 5;` |
| `.bss` | nol qiymatli global/static | `int buf[1000];` — faylda **joy egallamaydi**, faqat hajmi yoziladi |
| `.symtab` | belgilar jadvali | funksiya va o'zgaruvchi nomlari |
| `.rela.text` | relokatsiyalar | "bu joyga X ning manzilini yoz" |
| `.debug_*` | debug ma'lumoti (`-g`) | gdb uchun qatorlar, turlar |

`.bss` hiylasi: `static char buf[1 << 20];` faylni 1 MB ga kattalashtirmaydi — yuklovchi o'sha joyni nollar
bilan to'ldiradi. ELF segmentida bu `memsz > filesz` bo'lib ko'rinadi (36-mashq: `memsz >= filesz` sharti).

## 22.3. Belgilar (symbols)

Har bir global nom — belgi: nomi, qaysi bo'limda, qiymati (siljish), turi, ko'rinishi.

```bash
$ nm main.o
                 U kvadrat          # Undefined - boshqa joyda, linker topishi kerak
0000000000000000 T main             # T - .text da, global
$ nm matematika.o
0000000000000000 T kvadrat
0000000000000010 t yordamchi        # kichik t - LOKAL (static funksiya!)
0000000000000000 D hisob            # D - .data, global
0000000000000000 b bufer            # kichik b - .bss, lokal (static)
```

Katta harf — global (boshqa fayllar ko'radi), kichik — lokal (`static`, 5 va 11-boblar).

**Linker qoidalari:**
- Har bir `U` belgi aynan **bitta** global ta'rifga ulanishi kerak. Yo'q — `undefined reference`;
  ikkita — `multiple definition`.
- Kutubxonalar (`.a`) chapdan o'ngga ko'riladi, arxivdan faqat kerakli `.o` olinadi (11-bob).

## 22.4. Relokatsiya — "manzilni keyin yozaman"

Kompilyator `main.c` ni kompilyatsiya qilganda `kvadrat` qayerda bo'lishini bilmaydi. U `call` buyrug'iga
vaqtincha 0 yozadi va relokatsiya yozuvi qoldiradi:

```bash
$ objdump -dr main.o
  e:  e8 00 00 00 00      call  13 <main+0x13>
          f: R_X86_64_PLT32   kvadrat-0x4        # "f siljishiga kvadrat manzilini (nisbiy) yoz"
```

Linker: 1) hamma `.o` larning bo'limlarini birlashtiradi (hamma `.text` lar bitta `.text` ga), 2) har bir
belgiga yakuniy manzil beradi, 3) har bir relokatsiyani bajarib, bo'sh joylarga haqiqiy manzillarni yozadi.

Linker skripti (18-bob) aynan 1-2-qadamni boshqaradi: qaysi bo'lim qaysi manzilga.

## 22.5. Statik va dinamik bog'lash

**Statik:** kutubxona kodi dastur fayliga **ko'chiriladi**. Dastur mustaqil, lekin katta; kutubxona
yangilansa — qayta bog'lash kerak. MyOS dasturlari shunday (`libc.a`).

**Dinamik:** dasturda faqat "menga `libc.so.6` dagi `printf` kerak" degan yozuv. Ishga tushganda
**dinamik yuklovchi** (`ld-linux.so`) kutubxonani xotiraga yuklaydi va manzillarni ulaydi.
Afzalliklari: kutubxona xotirada bir marta (hamma dasturlar bo'lishadi), yangilash oson.

```bash
ldd /bin/ls              # qaysi dinamik kutubxonalarga bog'liq
```

### PLT va GOT — dinamik chaqiruv qanday ishlaydi

Kutubxona har safar boshqa manzilga yuklanishi mumkin, lekin dastur kodi (`.text`) faqat o'qiladi va
bo'lishiladi — uni o'zgartirib bo'lmaydi. Yechim — bilvosita chaqiruv:

```text
call printf@PLT   ->  PLT (kichik "trambolin" kod)  ->  jmp *GOT[printf]  ->  printf
```

- **GOT** (Global Offset Table) — manzillar jadvali, **yoziladigan** ma'lumot bo'limida. Yuklovchi unga
  haqiqiy manzillarni yozadi.
- **PLT** (Procedure Linkage Table) — har bir tashqi funksiya uchun kichik kod: GOT'dagi manzilga sakraydi.
  "Dangasa bog'lash": birinchi chaqiruvda GOT yuklovchiga ko'rsatadi, u manzilni topib GOT'ga yozadi,
  keyingi chaqiruvlar to'g'ridan-to'g'ri boradi.

Xavfsizlik jihati: GOT yoziladigan — hujumchilar uni o'zgartirishga urinadi. "RELRO" himoyasi yuklashdan
keyin GOT'ni faqat o'qiladigan qiladi.

## 22.6. PIC va PIE — joyi o'zgaruvchan kod

**PIC** (position-independent code) — istalgan manzilda ishlaydigan kod: global ma'lumotga `rip`ga nisbiy
murojaat qiladi (`lea rax, [rip + 0x1234]`). **PIE** — butun dastur shunday; OS uni har safar tasodifiy
manzilga yuklaydi (**ASLR** — hujumchi manzillarni oldindan bilmasin).

Yadro esa odatda aniq manzilda (`-fno-pic -fno-pie -mcmodel=kernel`, 18-bob), lekin Linux ham yadro
uchun KASLR (tasodifiy joylashtirish) qiladi.

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

MyOS'da bu yo'lning hammasi o'qish uchun ochiq: `kernel/proc/exec.c`, `kernel/sys/elf.c`,
`user/libc/crt0.asm`, `user/linker.ld`.

## 22.8. Savol-javob

**`static` global o'zgaruvchi va funksiya ichidagi `static` — belgilar jadvalida qanday?**
Ikkalasi ham lokal belgi (`b`/`d`/`t`). Funksiya ichidagisining nomiga kompilyator raqam qo'shadi
(`hisob.0`) — nomlar to'qnashmasin.

**Nega `-g` fayl hajmini oshiradi, lekin dastur tezligiga ta'sir qilmaydi?**
Debug ma'lumot alohida bo'limlarda; yuklovchi ularni xotiraga yuklamaydi. MyOS diskka `--strip-debug`
qilingan nusxani qo'yadi, to'liq ELF esa gdb uchun `build/` da qoladi.

**`weak` belgi nima?**
"Boshqa kuchli ta'rif bo'lmasa — shuni ishlat". `__attribute__((weak))`. Kutubxonalarda sukut bo'yicha
amalga oshirishni almashtirish uchun.

## 22.9. O'zingizni tekshiring

1. `.bss` nega fayl hajmini oshirmaydi?
2. `nm` chiqishidagi `T`, `t`, `U` nimani bildiradi?
3. Relokatsiya nima uchun kerak?
4. PLT va GOT qanday ishlaydi, nega bevosita `call printf` emas?
5. ELF bo'limlari va segmentlari farqi?

<details><summary>Javoblar</summary>

1. Unda faqat hajm yoziladi; yuklovchi xotirani nollar bilan to'ldiradi (`memsz > filesz`).
2. `T` — global funksiya, `t` — lokal (static), `U` — aniqlanmagan (tashqaridan kerak).
3. Kompilyator yakuniy manzillarni bilmaydi; linker ularni keyin joyiga yozadi.
4. Kod faqat o'qiladi va bo'lishiladi; manzil yoziladigan GOT'da turadi, PLT undagi manzilga sakraydi.
5. Bo'limlar — linker uchun mantiqiy qismlar; segmentlar — yuklovchi uchun "xotiraga nimani qayerga".
</details>

## 22.10. Mashq

- Uch faylli dasturingiz (1-bob) uchun `nm`, `readelf -S`, `objdump -dr` chiqishlarini o'qing.
- MyOS'da: `readelf -l build/kernel.elf` — segmentlar manzillari (`0xffffffff80...`) va linker skripti bilan solishtiring.
- **36-mashq** (ELF tahlilchisi) — agar hali qilmagan bo'lsangiz.

Keyingi bob: [23-bob. Jarayonlar va rejalashtirish (scheduling)](23-jarayonlar-scheduling.md)
