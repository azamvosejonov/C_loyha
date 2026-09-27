# 29-bob. Debug va profiling vositalari

> **Bu bobdan keyin:** xatoni tizimli ravishda topish usulini, gdb'ning chuqur imkoniyatlarini
> (watchpoint, shartli to'xtash, core dump), sanitizer'lar, valgrind, strace, ltrace, perf, objdump,
> addr2line va QEMU monitor'ini bilasiz. Dasturchi vaqtining yarmi — debug; bu bob o'sha yarmini tezlashtiradi.

> **To'liq ishlaydigan misol:** [misollar/29_xatoli.c](misollar/29_xatoli.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## 29.1. Debug usuli — asbobdan oldin fikr

1. **Takrorlang.** Xatoni har safar chiqadigan qiling (kirish, buyruq, qadamlar). Takrorlanmaydigan xato
   tuzatilmaydi — faqat yashiriladi.
2. **Kichraytiring.** Xatoni ko'rsatadigan eng kichik kirish/kodni toping (ikkilik bo'lish usuli: yarmini
   olib tashlang — xato qoldimi?).
3. **Faraz qiling va tekshiring.** "Menimcha x bu yerda NULL" → `print` yoki `assert` bilan tasdiqlang.
   Farazlarsiz tasodifiy o'zgartirishlar — vaqtni yo'qotish.
4. **Birinchi noto'g'ri narsani toping**, oxirgisini emas. Qulash — oqibat; sabab ancha oldinda
   (masalan, bufer to'lishi 1000 qator oldin bo'lgan).
5. **Tuzatgach — test qo'shing.** Xato qaytib kelmasin.
6. **"Rezina o'rdak":** muammoni kimgadir (yoki o'yinchoqqa) ovoz chiqarib tushuntiring — ko'pincha
   tushuntirish paytida o'zingiz topasiz.

## 29.2. `printf` / `kprintf` — hali ham eng ko'p ishlatiladigan vosita

```c
fprintf(stderr, "[debug] %s:%d n=%zu p=%p\n", __FILE__, __LINE__, n, (void *)p);
```

`stderr` — buferlanmaydi (qulashdan oldin ham chiqadi). Yadroda — `kprintf` (serial port + dmesg).
Muhim o'zgaruvchilarni **funksiya kirish/chiqishida** chiqarish ko'pincha gdb'dan tezroq natija beradi.

## 29.3. gdb chuqur

```bash
gcc -g -O0 dastur.c -o dastur
gdb --args ./dastur arg1 arg2
```

| Buyruq | Nima |
|---|---|
| `break fayl.c:42` / `b funksiya` | to'xtash nuqtasi |
| `break 42 if i == 1000` | **shartli** to'xtash — sikl ichidagi 1000-aylanishda |
| `watch x` / `watch -l p->qiymat` | **watchpoint**: o'zgaruvchi O'ZGARGANDA to'xtash — "buni kim buzdi?" savolining javobi |
| `rwatch`, `awatch` | o'qilganda / har qanday murojaatda |
| `run`, `continue` (`c`), `next` (`n`), `step` (`s`), `finish` | boshqarish |
| `until 50` | 50-qatorgacha (siklni tugatish) |
| `bt` / `bt full` | chaqiruvlar zanjiri (+ lokal o'zgaruvchilar) |
| `frame 3`, `up`, `down` | zanjir bo'ylab yurish |
| `print *p`, `print a[0]@10` | qiymat; massivning 10 ta elementi |
| `print/x val`, `x/16xb p` | o'n oltilikda; xotirani baytlab ko'rish |
| `info registers`, `x/10i $rip` | registrlar; keyingi 10 buyruq |
| `display i` | har qadamda avtomatik ko'rsatish |
| `set var x = 5` | ish vaqtida o'zgaruvchini o'zgartirish |
| `layout src` / `layout asm` / `tui enable` | matnli interfeys: kod va assembly yonma-yon |
| `thread apply all bt` | hamma oqimlarning zanjiri (deadlock'da!) |

**Core dump** — qulagan dasturning xotira "surati":

```bash
ulimit -c unlimited              # core fayllarga ruxsat
./dastur                          # Segmentation fault (core dumped)
gdb ./dastur core                 # qulagan paytdagi holat: bt, print...
```

(Zamonaviy Linux'da core'lar `systemd-coredump` ga tushishi mumkin: `coredumpctl gdb`.)

**Osilib qolgan dasturga ulanish:** `gdb -p PID`, keyin `thread apply all bt` — hamma oqim qayerda kutyapti.

**Yadroni debug qilish** (MyOS): `make debug` — QEMU gdb'ni kutadi; boshqa terminalda `gdb -x tools/gdbinit`.
Xuddi shu buyruqlar yadroda ham ishlaydi: `break kmain`, `break mm_handle_fault` (page fault), `break handle_exception`, `bt`, `info registers`.
`docs/08-test-debug.md` — batafsil.

## 29.4. Sanitizer'lar

| Bayroq | Nima topadi | Sekinlashish |
|---|---|---|
| `-fsanitize=address` (ASan) | chegaradan chiqish, use-after-free, double free, leak | ~2x |
| `-fsanitize=undefined` (UBSan) | toshish, noto'g'ri surish, NULL, tekislanmagan | kam |
| `-fsanitize=thread` (TSan) | poyga holatlari | 5–15x (ASan bilan birga emas) |
| `-fsanitize=memory` (MSan, faqat clang) | boshlanmagan xotirani o'qish | 3x |

Mashqlardagi tekshiruvchi ASan + UBSan ishlatadi. Ko'p oqimli kodni (29, 34, 38, 44, 45-mashqlar)
qo'shimcha ravishda TSan bilan sinang:

```bash
gcc -g -fsanitize=thread -pthread -Imashqlar -Imashqlar/29_oqimlar \
    mashqlar/29_oqimlar/yechim.c mashqlar/29_oqimlar/test.c -o t29 && ./t29
```

Linux yadrosida analoglari: KASAN, UBSAN, KCSAN.

## 29.5. Valgrind

```bash
valgrind --leak-check=full --track-origins=yes ./dastur
```

Qayta kompilyatsiyasiz ishlaydi (dasturni virtual CPU'da bajaradi — 20–50x sekin). Boshlanmagan
qiymatlardan foydalanishni ham topadi (`--track-origins` — qayerdan kelgan). `valgrind --tool=callgrind`
— qaysi funksiya qancha buyruq bajarayotgani.

## 29.6. strace va ltrace — dastur OS bilan nima gaplashyapti

```bash
strace ./dastur                  # hamma syscall'lar: open, read, write, mmap...
strace -e trace=openat,read ls   # faqat tanlanganlari
strace -f -p PID                 # ishlayotgan jarayonga (bolalari bilan)
strace -c ./dastur               # statistika: qaysi syscall necha marta, qancha vaqt
ltrace ./dastur                  # kutubxona funksiyalari chaqiruvlari
```

"Dastur nega bu faylni topmayapti?", "qayerda osilib qoldi?" (oxirgi syscall — `read` yoki `futex` da kutyapti)
kabi savollarga kodni o'qimasdan javob beradi. 14-bobdagi tushunchalarni jonli ko'rish uchun ajoyib:
`strace -f sh -c 'ls | wc -l'` — `pipe`, `clone`, `dup2`, `execve`, `wait4` ni ko'rasiz.

## 29.7. Binar fayllarni ko'rish

```bash
objdump -d -M intel dastur        # disassembly (Intel sintaksisi)
objdump -S dastur                 # manba kod bilan aralash (-g bilan kompilyatsiya qilingan bo'lsa)
readelf -a dastur                 # ELF'ning hammasi (22-bob)
nm -n dastur                      # belgilar manzil bo'yicha tartiblangan
addr2line -e dastur -f 0x401136   # manzil -> funksiya va fayl:qator
xxd fayl | head                   # baytlarni o'n oltilikda
```

**addr2line — yadro PANIC'ini o'qish:** MyOS PANIC xabarida `RIP=0xffffffff8010abcd` bo'lsa:
`addr2line -e build/kernel.elf -f 0xffffffff8010abcd` → funksiya va qator. Stek zanjiridagi har bir
manzil uchun takrorlang.

## 29.8. perf — dastur vaqti qayerga ketyapti

```bash
perf stat ./dastur                          # umumiy: taktlar, buyruqlar, kesh xatolari, sakrash xatolari
perf stat -e cache-misses,cache-references ./dastur
perf record -g ./dastur && perf report      # qaysi funksiyalar eng ko'p vaqt oladi (chaqiruv zanjirlari bilan)
perf top                                    # jonli: butun tizimda hozir nima issiq
```

**Qoida:** optimallashtirishdan oldin o'lchang. Dasturchilarning "qayer sekin" haqidagi taxminlari
ko'pincha noto'g'ri. 21-bobdagi matritsa tajribasini `perf stat -e cache-misses` bilan takrorlang.

## 29.9. QEMU'ning o'z vositalari (yadro uchun)

| Vosita | Nima |
|---|---|
| `-d int,cpu_reset -no-reboot` | har bir uzilish/istisno va CPU qayta yuklanishi jurnali — triple fault'ni topish |
| `-d in_asm` | bajarilgan har bir blok (juda ko'p chiqish) |
| `-s -S` | gdb server, birinchi buyruqdan oldin to'xtash (`make debug`) |
| QEMU monitor (`Ctrl-A C` nographic rejimda) | `info registers`, `info mem` (sahifa xaritasi!), `info tlb`, `x/10i $pc`, `info pic` |
| `-serial stdio` / `-nographic` | serial port chiqishi terminalda |

## 29.10. Tipik holatlar uchun "retsept"lar

| Belgi | Birinchi qadam |
|---|---|
| Segmentation fault | ASan bilan qayta yig'ish yoki `gdb` → `run` → `bt` |
| Qiymat "o'z-o'zidan" o'zgaradi | `watch -l` o'sha o'zgaruvchiga |
| Dastur osilib qoldi | `gdb -p PID` → `thread apply all bt`; yoki `strace -p PID` |
| Ba'zan noto'g'ri natija (ko'p oqimli) | TSan; qulflarni tekshirish |
| Sekin | `perf record` → `perf report` |
| Xotira o'sib boradi | ASan/valgrind leak hisoboti |
| "Oldin ishlardi" | `git bisect` (19-bob) |
| Yadro qayta yuklanib turadi | QEMU `-d int,cpu_reset -no-reboot` |
| Yadro PANIC | `addr2line` bilan RIP va stek manzillari |
| Yadro osilib qoldi | `make debug`, `Ctrl-C` gdb'da, `bt`, `info registers` |

## 29.11. O'zingizni tekshiring

1. "Bu o'zgaruvchini kim o'zgartiryapti?" — qaysi gdb buyrug'i?
2. Dastur osilib qoldi — qaysi ikkita vosita bilan qayerda turganini ko'rasiz?
3. Nega ASan va TSan birga ishlatilmaydi?
4. Yadro PANIC'idagi RIP manzilini qanday qilib manba qatoriga aylantirasiz?
5. Optimallashtirishdan oldin nima qilish kerak?

<details><summary>Javoblar</summary>

1. `watch` (yoki `watch -l` manzil bo'yicha).
2. `gdb -p PID` + `thread apply all bt`; `strace -p PID`.
3. Ikkalasi ham xotirani o'zicha kuzatadi ("soya xotira") va bir-biriga zid; alohida yig'iladi.
4. `addr2line -e build/kernel.elf -f <RIP>`.
5. O'lchash (`perf`) — sekin joyni aniq topish.
</details>

## 29.12. Mashq

- 18-mashqni ASan'siz yig'ib, `valgrind` bilan xatolarni toping — hisobotlarni solishtiring.
- `strace -f sh -c 'ls | wc -l'` chiqishida `pipe`, `clone`, `dup2`, `execve` qatorlarini topib, 14-bob bilan bog'lang.
- MyOS'da: `make run-nographic APPEND=demo=uaf` — slab qanday ushlashini ko'ring; `make debug` bilan `kmain` da to'xtab, `bt` va `info registers` ni sinang.

Keyingi bob: [30-bob. Yadro arxitekturasi va Linux'ga yo'l](30-yadro-arxitekturasi.md)
