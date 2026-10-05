# Katta loyihalar

Darslik boblari mashqlardan ham iborat, lekin **bitta katta dastur**ni bobdan bobga o'stirib borish o'rganishning eng yaxshi usuli:
har yangi tushuncha oldingi versiyadagi **haqiqiy muammoni** hal qiladi, shuning uchun "nega bu kerak?" degan savol qolmaydi.

## Ombor (0–12-boblar): bitta dastur, 13 bosqich

Mahsulotlar ombori boshqaruvi: ro'yxat, sotish, qo'shish, o'chirish, hisobot, saralash, faylga saqlash. 13 bosqich — har bobda bittadan:

| Bosqich | Bob | Papka | Yangi narsa |
|---|---|---|---|
| 0 | 0 | `ombor/00_salom` | `printf` bilan chiroyli jadval |
| 1 | 1 | `ombor/01_fayllar` | `.h` + `.c`, alohida kompilyatsiya |
| 2 | 2 | `ombor/02_turlar` | pul tiyinda (`long`), `uint16_t`, toshishdan himoya |
| 3 | 3 | `ombor/03_operatorlar` | chegirma, bit bayroqlari (holat) |
| 4 | 4 | `ombor/04_boshqaruv` | menyu (`while` + `switch`), `scanf` tekshiruvi |
| 5 | 5 | `ombor/05_funksiyalar` | funksiyalar, xato kodlari (`enum`) |
| 6 | 6 | `ombor/06_massivlar` | massivlar, satrlar, nom bo'yicha qidirish |
| 7 | 7 | `ombor/07_korsatkichlar` | ko'rsatkichlar: `sot(&zaxira)`, saralash, eng qimmat |
| 8 | 8 | `ombor/08_xotira` | `malloc`/`realloc`/`free`, sanitizer |
| 9 | 9 | `ombor/09_struct` | `struct`, `enum`, `typedef` |
| 10 | 10 | `ombor/10_makrolar` | makrolar, X-makro, `-DDEBUG` izlari |
| 11 | 11 | `ombor/11_kop_fayl` | ko'p fayl, opaque tur, `Makefile`, statik kutubxona |
| 12 | 12 | `ombor/12_stdlib` | faylga saqlash, `qsort`, `errno`, `assert` |

## Tizim loyihalari (13–31-boblar)

Ikkinchi qism boblarida katta loyiha **bitta dastur** emas, har bobga **o'zining** tizim dasturi: u bobning asosiy g'oyasini **ishlaydigan, tekshirilgan** kodda ko'rsatadi. Har biri 100–400 qator, **haqiqiy chiqishi** bobda ko'rsatilgan.

| Bob | Papka (`tizim/`) | Nima qiladi | Asosiy g'oya |
|---|---|---|---|
| 13 | `13_fuzz` | fuzzer + zaif va xavfsiz parser | sanitizer, toshish, chegara tekshiruvi |
| 14 | `14_msh` | mini shell: quvur, `<` `>` `>>`, `cd` | `fork`/`exec`/`pipe`/`dup2` |
| 15 | `15_matmul` | parallel matritsa ko'paytirish + poyga ovchisi | oqimlar, TSan |
| 16 | `16_ip_paket` | IPv4/TCP/UDP paket dekoderi | bitlar, maskalar, big-endian, nazorat yig'indisi |
| 17 | `17_asm_lab` | NASM funksiyalari C dan chaqiriladi | System V chaqirish qoidasi |
| 18 | `18_libk` | libc'siz mini kutubxona (yadro uchun) | freestanding, bitmap, halqa bufer |
| 19 | `19_log_tahlil` | veb-server jurnalini tahlil qilish | quvurlar, `awk`, `sort`/`uniq` |
| 20 | `20_float_lab` | IEEE 754 laboratoriyasi | float bitlari, ULP, Kahan |
| 21 | `21_kesh_lab` | kesh xatolarini o'lchash | cachegrind, xotira yurish tartibi |
| 22 | `22_elf_sym` | ELF belgilar jadvali o'quvchisi (mini `nm`) | ELF, linker belgilari |
| 23 | `23_sched_lab` | FIFO/SJF/RR/ustuvorlik/aging simulyatori | rejalashtirish ko'rsatkichlari |
| 24 | `24_fault_lab` | page fault sanash | demand paging, COW |
| 25 | `25_guard_alloc` | qo'riqchi sahifali ajratuvchi | `mmap`/`mprotect` |
| 26 | `26_thread_pool` | oqimlar puli | mutex, condvar |
| 27 | `27_mfs` | mini fayl tizimi + `fsck` | inode, bitmap, katalog, izchillik |
| 28 | `28_metro` | metro: Dijkstra, Kruskal, trie | graf algoritmlari |
| 29 | `29_trace` | `LD_PRELOAD` bilan `malloc` kuzatuvchisi | dinamik linker, `dlsym` |
| 30 | `30_ucontext_yadro` | foydalanuvchi maydonidagi mini-yadro | kontekst almashish, syscall jadvali |
| 31 | `31_viktorina` | savollar faylidan o'qib test o'tkazadigan dastur | fayl formati, tekshirish |

Ba'zi loyihalar qo'shimcha dastur talab qiladi (`17` — `nasm`, `21` — `valgrind`): ular `kerak.txt` da yozilgan, dastur yo'q bo'lsa tekshiruv bu bosqichni **o'tkazib yuboradi**.

## Qanday ishlash kerak

1. Bobdagi "**Katta loyiha**" bo'limini o'qing (kod + tushuntirish + natija).
2. O'zingizga papka oching (`mkdir ~/ombor`) va kodni **o'zingiz yozing** (ko'chirmang!).
3. Natijani shu papkadagi tayyor variant bilan solishtiring.
4. Bo'lim oxiridagi **"O'zingiz qo'shing"** topshiriqlarini bajaring (yechimsiz, faqat maslahat).

## Tekshirish

Har bosqich papkasida: manba fayllar, `qur.txt` (yig'ish buyrug'i), `kirish.txt` (klaviatura o'rniga) va `kutilgan.txt` (kutilgan chiqish).
Hammasini yig'ib solishtirish: `python3 tools/katta_loyiha.py` (`make lab-check` ham shuni ishga tushiradi).
