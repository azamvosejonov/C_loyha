# LABORATORIYALAR — kodni yopib, o'zingiz yozish

Bu yerda yadro va libc'ning **22 ta muhim funksiyasi** bor. Har birini o'chirib, o'zingiz qayta yozasiz,
avtomatik testlar esa natija to'g'ri yoki noto'g'ri ekanini aytadi. Bu — o'qigan narsani **o'zlashtirilgan**
bilimga aylantirishning yagona yo'li.

## Qanday ishlaydi

```bash
tools/lab.py royxat               # barcha laboratoriyalar va holati
tools/lab.py boshla memmove       # asl kod yashiriladi, o'rniga TODO izohi qo'yiladi
edit user/libc/string.c           # (yoki o'z muharriringizda) funksiyani yozing
tools/lab.py tekshir memmove      # yig'ish + testlar
tools/lab.py yechim memmove       # O'TGANDAN keyin: asl kod bilan solishtiring
tools/lab.py tiklash memmove      # asl kodni qaytarish (qaytadan boshlash uchun ham)
```

Kodda funksiya tanasi quyidagi belgilar bilan o'ralgan:
```c
    /* >>> LAB memmove - vazifa: labs/README.md */
    ...
    /* <<< LAB memmove */
```
Belgilarni **o'chirmang**: `tools/lab.py` ular bo'yicha ishlaydi.

**Tekshirish qanday o'tadi:**
- **libc laboratoriyalari:** avval `tools/host_libctest.sh` ishga tushadi. U sizning funksiyangizni Linux'ning
  o'zida, 1 soniyada, **aniq** xabar bilan tekshiradi (masalan, `memmove chapga (ustma-ust)`). Keyin butun
  tizim QEMU'da sinaladi.
- **Yadro laboratoriyalari:** `tools/test.sh bios`. Bunda yadro selftest'lari (buddy, slab, spinlock),
  fstest, sigtest, forktest, ext2 + e2fsck va shell'ning 43 ta tekshiruvi ishga tushadi (~1 daqiqa).
  Yadro qulasa, `build/test-output-bios.log` da panic xabari va backtrace bo'ladi.

## Qoidalar

1. **Avval o'qing, keyin yozing.** Har bir lab tegishli `docs/NN-*.md` ga havola beradi.
2. **Asl kodga qaramang** (`git show`, `.lab/`). Qiyin bo'lsa: hujjatni qayta o'qing, qog'ozda chizing,
   kichik misolni qo'lda bajarib ko'ring.
3. Test o'tgach, **`yechim`** bilan solishtiring. Farq bo'lsa, qaysi biri yaxshiroq va nima uchun ekanini
   o'ylang. Farq — xato degani emas.
4. Ishlaganiga ishonch hosil qiling va **`tiklash`** qiling. Keyin yana bir marta, tezroq yozing. Maqsad —
   funksiyani 10 daqiqada, xatosiz yozish.
5. Lab faol turganda `git commit` qilmang (aks holda stub kodi commit'ga tushadi). `git status` bilan
   tekshiring.

## Tavsiya etilgan tartib

| # | lab | daraja | nimani o'rgatadi | hujjat |
|---|---|---|---|---|
| 1 | `strlen` | * | ko'rsatkichlar, satr oxiri | QOLLANMA 5.2 |
| 2 | `memmove` | * | ustma-ust xotira, nusxalash yo'nalishi | QOLLANMA 5.2 |
| 3 | `strtok_r` | ** | holatni saqlash, qayta kiriluvchanlik | 12 |
| 4 | `strtoul` | ** | parsing, chegaraviy holatlar, to'lish | 12 |
| 5 | `emit_number` | ** | printf ichki qismi: kenglik, to'ldirish | 02 |
| 6 | `gmtime_r` | ** | algoritm: kalendar arifmetikasi | 12 |
| 7 | `glob_match` | ** | rekursiya, backtracking | 12 |
| 8 | `malloc` | *** | free list, bo'lish (split), tekislash | 05, 07 |
| 9 | `insert_free` | *** | tartiblangan ro'yxat, birlashtirish (coalescing) | 05 |
| 10 | `path_normalize` | ** | `.` va `..`, stek g'oyasi | 12 |
| 11 | `pipe` | *** | halqa bufer, uxlash/uyg'otish, EOF, SIGPIPE | 12 |
| 12 | `spinlock` | *** | atomar amallar, xotira tartibi | 10 |
| 13 | `buddy` | *** | buddy allocator, egizak manzil (XOR) | 09 |
| 14 | `slab` | *** | slab kesh, ro'yxatlar holati | 09 |
| 15 | `elf_load` | *** | fayl formatini tekshirish (xavfsizlik!) | 07 |
| 16 | `sleep_wakeup` | **** | lost wakeup, qulflar almashinuvi | 06, 10 |
| 17 | `fault_page` | **** | demand paging, copy-on-write, TLB | 11 |
| 18 | `block_path` | ** | ext2 bilvosita bloklar indeksi | 13 |
| 19 | `bmap` | **** | ko'p darajali daraxt bo'ylab yurish + ajratish | 13 |
| 20 | `dir_add` | *** | papka yozuvlari, rec_len | 13 |
| 21 | `setup_frame` | **** | signal freymi, stek tekislash | 14 |
| 22 | `sigreturn` | **** | ishonchsiz ma'lumotni tekshirish | 14 |

---

## 1. `strlen` — `user/libc/string.c`

**Vazifa:** `s` satrining uzunligini qaytaring, ya'ni birinchi `'\0'` gacha nechta bayt borligini.
`'\0'` ning o'zi hisoblanmaydi.

**Chegaraviy holatlar:** bo'sh satr (`""` → 0); ichida `'\0'` bor satr (`"ab\0cd"` → 2); uzun satr.

**O'ylab ko'ring:** indeks bilan yozish (`s[n]`) va ko'rsatkich bilan yozish (`p++`, keyin `p - s`) — ikkala
usulni ham yozing. glibc `strlen` ni bir vaqtda 8 bayt tekshirib tezlashtiradi: bu qanday ishlashi mumkin?

## 2. `memmove` — `user/libc/string.c`

**Vazifa:** `n` baytni `src` dan `dst` ga nusxalang va `dst` ni qaytaring. **Bloklar ustma-ust tushishi
mumkin**. Aynan shu jihati bilan u `memcpy` dan farq qiladi.

```
src:  [A B C D E]            dst = src + 2 (o'ngga surish):
dst:      [A B C D E]        oldindan nusxalansa: A B A B A  <- XATO (C va D yo'qoldi)
```
**Maslahat:** `dst` `src` dan keyinda turib, ular ustma-ust tushsa, **orqadan** nusxalang. Aks holda
oldindan nusxalang.

**Chegaraviy holatlar:** `n == 0`; `dst == src`; chapga va o'ngga surish.
**Xato manbai:** `unsigned char *` emas, `char *` ishlatish ham to'g'ri ishlaydi, lekin nega `unsigned`
afzal? `n--` sikli `size_t` bilan qanday tugaydi?

## 3. `strtok_r` — `user/libc/string.c`

**Vazifa:** satrni `delim` dagi belgilar bo'yicha "so'z"larga ajrating. Birinchi chaqiruvda `s` beriladi,
keyingilarida esa `s == NULL` bo'lib, davom etish joyi `*save` da saqlanadi.
- So'z boshidagi ajratuvchilar o'tkazib yuboriladi.
- So'z oxiri `'\0'` bilan almashtiriladi (satr **o'zgaradi**).
- So'z qolmasa, `NULL` qaytariladi.

**Mavjud yordamchilar:** `strspn(s, delim)` — boshidagi ajratuvchilar soni, `strcspn(s, delim)` — birinchi
ajratuvchigacha bo'lgan belgilar soni.
**O'ylab ko'ring:** oddiy `strtok` global o'zgaruvchi ishlatadi. Nega u ko'p oqimli dasturlar va
signallar uchun xavfli, `_r` ("reentrant") versiyasi esa xavfsiz?

## 4. `strtoul` — `user/libc/stdlib.c`

**Vazifa:** satrni ishorasiz butun songa aylantiring.
- Boshidagi bo'shliqlar (`isspace`) o'tkazib yuboriladi, keyin ixtiyoriy `+`/`-` keladi (`-` bo'lsa,
  natija `-v` bo'ladi, C standarti shunday talab qiladi).
- `base == 0`: `0x`/`0X` → 16, `0` bilan boshlansa → 8, aks holda → 10. `base == 16` bo'lsa ham `0x`
  prefiksi qabul qilinadi.
- Raqamlar: `0-9`, `a-z`/`A-Z` (10–35). Raqam qiymati `>= base` bo'lsa, o'qish to'xtaydi.
- `end != NULL` bo'lsa: `*end` = birinchi o'qilmagan belgi. **Birorta ham raqam bo'lmasa, `*end = s`**
  (bo'shliqlardan oldingi asl boshlanish).
- To'lish: `errno = ERANGE` va `0xFFFFFFFFFFFFFFFF` qaytariladi. To'lishni ko'paytirishdan **oldin**
  tekshiring: `v > (MAX - d) / base`.

**Xato manbai:** `"0x"` dan keyin hex raqam kelmasa (`"0xg"`), natija `0` bo'ladi va `end` `x` ga
ko'rsatadi.

## 5. `emit_number` — `user/libc/printf.c`

**Vazifa:** `v` sonini `base` asosida `emit(c, ctx)` orqali chiqaring. Har bir chiqarilgan belgi uchun
`(*count)++`.
- Raqamlar teskari tartibda hosil bo'ladi (`v % base`), shuning uchun avval vaqtinchalik massivga yozing.
- Ishora belgisi: `neg` → `'-'`, aks holda `sp->plus` → `'+'`, `sp->space` → `' '`.
- `sp->prec >= 0`: kamida shuncha raqam chiqariladi (oldidan nollar). **`prec == 0` va `v == 0`** bo'lsa,
  hech narsa chiqmaydi.
- `sp->width`: umumiy kenglik. `sp->left` bo'lsa, bo'shliqlar o'ngga qo'yiladi. `sp->zero` bo'lsa (va
  `left` hamda `prec` bo'lmasa), ishoradan **keyin** nollar qo'yiladi: `-0042`.
- `upper` — `A-F` yoki `a-f`. Yordamchi funksiya: `pad(emit, ctx, belgi, n, count)`.

**Test:** `snprintf("[%5d|%-5d|%05d]", 42, 42, 42)` → `[   42|42   |00042]`.

## 6. `gmtime_r` — `user/libc/time.c`

**Vazifa:** `*t` (1970-01-01 00:00:00 UTC dan beri o'tgan soniyalar, **manfiy ham bo'lishi mumkin**) ni
`struct tm` ga aylantiring: `tm_year` (1900 dan beri), `tm_mon` (0–11), `tm_mday` (1–31), `tm_hour`,
`tm_min`, `tm_sec`, `tm_wday` (0 = yakshanba), `tm_yday` (0–365).

**Maslahatlar:**
- Avval kunlar va kun ichidagi soniyalarni ajrating. **Manfiy** vaqtda qoldiq manfiy chiqadi, uni tuzating.
- 1970-01-01 — payshanba (4).
- Kabisa yili: 4 ga bo'linadi, lekin 100 ga bo'linmaydi, **yoki** 400 ga bo'linadi. 2000 — kabisa, 2100
  — kabisa emas.
- Oddiy yondashuv: yillarni birma-bir ayirib borish (365/366), keyin oylarni. Tez yondashuv: Howard
  Hinnant'ning `civil_from_days` algoritmi (400 yillik "era"lar).

## 7. `glob_match` — `user/bin/sh.c`

**Vazifa:** `pat` naqshi `s` satriga **to'liq** mos kelsa, `true` qaytaring. `*` — istalgan ketma-ketlik
(bo'sh ham), `?` — bitta istalgan belgi, qolgan belgilar faqat o'ziga mos keladi.

**Maslahat:** `*` uchragach, uni 0, 1, 2 ... belgiga "yeydirib", qolgan naqshni rekursiv tekshiring
(backtracking). Ketma-ket `**` ni bitta deb hisoblang, aks holda vaqt eksponent o'sadi.
**Test:** tizimda `cd /bin && echo c?t *grep*` → `cat grep`.

## 8. `malloc` — `user/libc/malloc.c`

**Ma'lumot tuzilmasi:** har bir blok oldida 16 baytli sarlavha `struct block { size_t size; struct block
*next; }` turadi. Bo'sh bloklar `free_list` da **manzil bo'yicha tartiblangan**. Band blokda
`next == USED_MAGIC` bo'ladi.

**Vazifa:**
1. `size == 0` yoki haddan tashqari katta → `NULL`. `size` ni 16 ga yaxlitlang (`ALIGN16`).
2. `free_list` dan `size` sig'adigan birinchi blokni toping (first-fit).
3. Qoldiq `>= size + MIN_SPLIT` bo'lsa, blokni **bo'ling**: qoldiqdan yangi bo'sh blok yasang.
4. Blokni ro'yxatdan chiqaring (oldingisini to'g'ri ulashni unutmang — `prev == NULL` holati!), belgilang
   (`next = USED_MAGIC`), statistikani yangilang (`used_bytes`, `used_blocks`).
5. **Sarlavhadan keyingi** manzilni qaytaring: `b + 1`.
6. Hech narsa topilmasa: `grow(size)`, keyin qayta urinish; `grow` 0 qaytarsa → `NULL`.

**Xato manbai:** qaytarilgan manzil 16 ga tekislanishi shart (`malloc` standarti; SSE ishlatadigan kod
aks holda qulaydi). `libctest` buni tekshiradi.

## 9. `insert_free` — `user/libc/malloc.c`

**Vazifa:** `b` blokini `free_list` ga **manzil tartibida** qo'shing, keyin qo'shnilari bilan birlashtiring:
- o'ng qo'shni: `(char *)b + HDR + b->size == (char *)cur` bo'lsa, `b` `cur` ni "yutadi";
- chap qo'shni: `(char *)prev + HDR + prev->size == (char *)b` bo'lsa, `prev` `b` ni "yutadi".

**Nega tartib muhim:** birlashtirish uchun qo'shnini tez topish kerak. Birlashtirish bo'lmasa, xotira
maydalanib ketadi (fragmentatsiya): jami bo'sh joy ko'p, lekin katta blok uchun joy topilmaydi.
`memtest` buni tekshiradi.

## 10. `path_normalize` — `kernel/fs/vfs.c`

**Vazifa:** `cwd` (joriy papka, masalan `/home/a`) va `path` dan kanonik mutlaq yo'l yasang:
- `path` `/` bilan boshlansa, u mutlaq yo'l; aks holda `cwd + "/" + path`.
- ortiqcha `/` lar tashlanadi, `.` o'tkazib yuboriladi, `..` oxirgi komponentni olib tashlaydi.
  **Ildizdan yuqoriga chiqilmaydi:** `/..` → `/`.
- Natija `size` ga sig'masa → `-ENAMETOOLONG`. Bo'sh natija → `"/"`.

**Misollar:** (`/home/a`, `../b/./c//`) → `/home/b/c`; (`/`, `../../x`) → `/x`.
**Maslahat:** `out` ni stek kabi ishlating: komponent qo'shish — oxiriga `/nom` yozish, `..` — oxirgi `/`
gacha qirqish.

## 11. `pipe` — `kernel/fs/pipe.c` (`pipe_read` va `pipe_write`)

**Tuzilma:** `struct pipe { lock; buf[PIPE_SIZE]; head; count; readers; writers; }` — halqa bufer. Keyingi
o'qiladigan bayt: `buf[head]`; bo'sh joy `(head + count) % PIPE_SIZE` dan boshlanadi.

**`pipe_read`:**
- `len == 0` → 0.
- Bufer bo'sh bo'lsa: `writers == 0` → **0 (EOF)**; `signal_interrupted(current)` → `-EINTR`; aks holda
  `proc_sleep(&p->count, &p->lock)` va qayta tekshirish (**`while`**, `if` emas!).
- `min(len, count)` baytni o'qing, `head` va `count` ni yangilang va `proc_wakeup(&p->writers)` chaqiring.

**`pipe_write`:**
- Hammasi yozilguncha: `readers == 0` → `SIGPIPE` yuboring (`proc_lock` ostida `signal_send_locked`) va
  `-EPIPE` qaytaring (lekin biror narsa allaqachon yozilgan bo'lsa, o'shani qaytaring).
- Bufer to'la bo'lsa: signal → `-EINTR` (yoki yozilgan qism); aks holda o'quvchilarni uyg'oting va
  `proc_sleep(&p->writers, ...)`.
- Yozgach, `proc_wakeup(&p->count)`.

**Qulf qoidasi:** `p->lock` ushlangan holda `proc_lock` ni olmang (tartib buziladi). Avval
`spin_unlock(&p->lock)` qiling. **Test:** `seq 20000 | grep 7 | tail -2`, `fstest` (200 KB, EOF, EPIPE),
`sigtest` (SIGPIPE).

## 12. `spinlock` — `kernel/lib/spinlock.c` (`spin_lock`, `spin_unlock`)

**`spin_lock`:**
1. `push_off()` — uzilishlarni o'chirish (sanagich bilan). **Nega kerak:** qulf ushlangan paytda uzilish
   handler'i shu qulfni so'rasa, CPU o'zini o'zi kutadi.
2. `spinlocks_busted` bo'lsa (panic paytida) — shunchaki qaytish.
3. Shu CPU qulfni allaqachon ushlab turgan bo'lsa → `panic` (deadlock'ni darhol aniqlash).
4. `__atomic_exchange_n(&l->locked, 1, __ATOMIC_ACQUIRE)` — 0 qaytarsa, qulf olindi. Aks holda qulf
   bo'shaguncha **faqat o'qib** turing (`__atomic_load_n` + `cpu_pause()`) va qayta urining
   (test-and-test-and-set).
5. `l->owner = this_cpu()`.

**`spin_unlock`:** egalikni tekshirish (`spin_holding`), `owner = 0`,
`__atomic_store_n(&l->locked, 0, __ATOMIC_RELEASE)`, `pop_off()`.

**Test:** selftest'dagi SMP stress testi — ikki CPU 60000 marta bitta sanagichni oshiradi. Xotira tartibi
xato bo'lsa, natija 60000 dan kam chiqadi.

## 13. `buddy` — `kernel/mm/pmm.c` (`buddy_alloc`, `buddy_free`)

**G'oya:** `order` o'lchamli blok = 2^order sahifa. Har bir blokning **egizagi** (buddy) bor:
`buddy_pfn = pfn ^ (1 << order)`. Ikkala egizak bo'sh bo'lsa, ular bitta katta blokka birlashadi.

**Yordamchilar:** `add_free(z, page, order)` — ro'yxatga qo'shadi va `PG_BUDDY`, `order` ni belgilaydi;
`del_free(z, page, order)` — ro'yxatdan chiqaradi; `pfn_to_page()`; `list_first_entry()`.

**`buddy_alloc(z, order)`:** `o = order, order+1, ... MAX_ORDER-1` tartibida birinchi bo'sh bo'lmagan
ro'yxatni toping, blokni oling (`del_free`). `o > order` bo'lsa, blokni ikkiga bo'lib, **yuqori** yarmini
`o-1` ro'yxatiga qaytaring va shu ish `o == order` bo'lguncha takrorlanadi. Hech narsa bo'lmasa → `NULL`.

**`buddy_free(z, pfn, order)`:** `order < MAX_ORDER - 1` ekan: egizakni hisoblang. Egizak zona
chegarasidan tashqarida bo'lsa, yoki bo'sh bo'lmasa (`PG_BUDDY` yo'q), yoki **boshqa tartibda** bo'lsa,
to'xtang. Aks holda egizakni ro'yxatdan chiqaring, `pfn &= ~(1 << order)` (birlashgan blok boshi) va
`order++`. Oxirida `add_free`.

**Xato manbai:** egizakning `order` ini tekshirmaslik — u bo'lingan va qisman band bo'lishi mumkin.

## 14. `slab` — `kernel/mm/slab.c` (`kmem_cache_alloc`)

**Tuzilma:** har bir kesh uchta ro'yxat saqlaydi: `partial` (qisman band), `empty`, `full`. Slab — sahifa
(`struct page`) bo'lib, uning ichidagi bo'sh obyektlar zanjiri `freelist` da turadi (keyingi bo'sh obyekt
manzili obyektning birinchi 8 baytida). Obyektning ikkinchi 8 bayti bo'sh holatda `FREE_MAGIC` bo'ladi.

**Vazifa:** `spin_lock_irqsave(&c->lock)`. Slab tanlash tartibi: `partial` → `empty` (uni `partial` ga
ko'chiring) → `new_slab(c)` (`NULL` bo'lsa, qulfni ochib, `NULL` qaytaring; aks holda `partial` ga
qo'shing). Keyin:
- `obj = slab->freelist`, `check_free_object(c, obj)` (use-after-free tekshiruvi);
- `slab->freelist = ((void **)obj)[0]`, `((uint64_t *)obj)[1] = 0` (endi "band");
- `inuse++`; slab to'lsa (`inuse == objs_per_slab`) → `full` ga ko'chiring;
- `active_objs++`, `allocs++`, qulfni oching, `ctor` bo'lsa uni chaqiring va `obj` ni qaytaring.

## 15. `elf_load` — `kernel/sys/elf.c`

**Vazifa:** ELF faylni `mm` manzil maydoniga yuklang. **Fayl ishonchsiz**: foydalanuvchi uni ataylab
buzgan bo'lishi mumkin, shuning uchun har bir maydon tekshiriladi (har bir xato — alohida manfiy kod):
- hajm `>= sizeof(ehdr)`, magic `\x7fELF`, `ELFCLASS64`, little-endian, `ET_EXEC`, `EM_X86_64`,
  `e_phentsize` to'g'ri, program header'lar **fayl ichida**;
- har bir `PT_LOAD` (`p_memsz > 0`): `filesz <= memsz`, ma'lumot fayl ichida, `[vaddr, vaddr+memsz)`
  **user hududida** (`USER_SPACE_START .. MMAP_TOP`) va to'lish yo'q (`end < start`!);
- ruxsatlar: `PF_W` → `PROT_WRITE`, `PF_X` → `PROT_EXEC`; sahifa chegaralariga tekislangan VMA:
  `mm_map(mm, vstart, len, prot, VMA_ELF)`;
- `filesz > 0` bo'lsa: `mm_populate` va `vmm_copy_to_space`. `.bss` qismini (memsz > filesz) demand
  paging nollaydi;
- `e_entry` bajariladigan segment ichida bo'lishi kerak. `*entry`, `*image_end` (eng yuqori manzil).

**O'ylab ko'ring:** `vaddr` ni tekshirmasangiz, hujumchi yadro manziliga yozadigan ELF yasay oladimi?

## 16. `sleep_wakeup` — `kernel/proc/process.c` (`proc_sleep`, `wakeup_locked`)

**`proc_sleep(channel, lk)`** — chaqiruvchi `lk` ni ushlab turibdi ("shart bajarilmadi, uxlayman"):
1. `lk != &proc_lock` bo'lsa: **avval** `proc_lock` ni oling, **keyin** `lk` ni qo'yib yuboring.
2. `wait_channel = channel`, `state = PROC_BLOCKED`, `sched()`.
3. Uyg'ongach: `wait_channel = NULL`; `lk != &proc_lock` bo'lsa, `proc_lock` ni qo'yib, `lk` ni qayta oling.

**`wakeup_locked(channel)`** (`proc_lock` ushlangan): shu kanalda `BLOCKED` turgan hamma jarayonni
`READY` qiling.

**Eng muhim savol — LOST WAKEUP:** 1-qadamdagi tartibni teskari qilsangiz (`lk` ni qo'yib, keyin
`proc_lock` ni olsangiz), oraliqda boshqa CPU'dagi `wakeup` uyg'otishi kerak bo'lgan jarayonni topmaydi
(u hali `BLOCKED` emas). Jarayon esa abadiy uxlaydi. Buni chizib tushuntiring. (xv6 kitobi, 7-bob.)

## 17. `fault_page` — `kernel/mm/mm.c`

**Vazifa** (`mm->lock` ushlangan): `page` manzili uchun page fault'ni hal qiling.
1. `pte = vmm_get_pte(mm->pml4, page, true)` (`NULL` → `false`).
2. **Sahifa yo'q** (`!(*pte & PTE_PRESENT)`) — **demand paging**: `alloc_pages(0, GFP_ZERO)`,
   `mapcount = 1`, `*pte = fizik | prot_to_pte(v->prot) | PTE_PRESENT`.
3. **Yozish va `PTE_COW`** — **copy-on-write**: eski sahifaning `refcount` i 1 bo'lsa (boshqa hech kim
   ulashmayapti), shunchaki `COW` ni olib, `WRITABLE` qiling. Aks holda yangi sahifa oling, 4096 baytni
   nusxalang, PTE ni yangi sahifaga yo'naltiring va eskisidan `put_page` (`mapcount` ni ham kamaytiring).
   Ikkala holatda ham **`cpu_invlpg(page)`**: TLB'da eski "faqat o'qish" yozuvi qolmasin.
4. Aks holda: yozish bo'lsa `PTE_WRITABLE` bor-yo'qligini, o'qish bo'lsa `true` ni qaytaring.

**Test:** `forktest` (COW, 20 bola, 32 MB mmap), `memtest`, butun tizim. **Xato manbai:** `invlpg` ni
unutish — bu xato faqat ba'zan chiqadi, chunki TLB yozuvi har doim ham qolavermaydi.

## 18. `block_path` — `kernel/fs/ext2.c`

**Vazifa:** fayldagi `fblock` bloki `i_block[]` daraxtining qayerida ekanini hisoblang. `apb =
fs->addr_per_block` (1 KB blokda 256).
- `fblock < 12` → `path[0] = fblock`, chuqurlik **1**;
- keyingi `apb` ta → `path = {12, i}`, chuqurlik **2**;
- keyingi `apb²` ta → `path = {13, i / apb, i % apb}`, chuqurlik **3**;
- keyingi `apb³` ta → `path = {14, ...}` (3 ta indeks), chuqurlik **4**;
- undan katta → **0**.

**Test:** 300 KB'li `katta.bin` (1 KB blokda ikki karra bilvosita blok) CRC'si, `fstest /mnt`, e2fsck.
Blokni ~12 va ~268 chegaralarida qo'lda hisoblab chiqing.

## 19. `bmap` — `kernel/fs/ext2.c`

**Vazifa:** `fblock` ning disk bloki raqamini `*out` ga yozing (inode qulfi ushlangan).
1. `depth = block_path(...)`; 0 bo'lsa → `-EFBIG`.
2. `cur = raw.i_block[path[0]]`. 0 bo'lsa: `alloc` bo'lmasa `*out = 0` (teshik), `return 0`. `alloc`
   bo'lsa: `alloc_block(fs, goal)` (0 → `-ENOSPC`), `i_block` ga yozing, `i_blocks += blok/512`.
3. Har bir keyingi daraja uchun: `fs_bread(fs, cur)` → `uint32_t` ko'rsatkichlar massivi →
   `next = ptrs[path[level]]`. 0 bo'lsa, xuddi yuqoridagidek ajrating, **`bwrite`** qiling va
   `i_blocks` ni oshiring. Har doim `brelse` ni unutmang!
4. Oxirida `*out = cur`. Inode o'zgargan bo'lsa → `write_inode(in)`.

**Test:** `fstest /mnt` (300 KB fayl, teshiklar, truncate), e2fsck (`i_blocks` noto'g'ri bo'lsa, e2fsck
aniq aytadi!), debugfs.

## 20. `dir_add` — `kernel/fs/ext2.c` (`add_visit`, `dir_add`)

**Papka yozuvi:** `[inode 4][rec_len 2][name_len 1][file_type 1][nom]`. Kerakli hajm:
`rec_size(len) = (8 + len + 3) & ~3`.

**`add_visit`** (`dir_iterate` har bir yozuv uchun chaqiradi): `used = d->inode ? rec_size(d->name_len) : 0`.
`d->rec_len - used >= rec_size(c->len)` bo'lsa, joy topildi. Agar `d->inode != 0` bo'lsa, yozuvni
**bo'ling**: yangi yozuv `d + used` dan boshlanadi va `rec_len = d->rec_len - used` bo'ladi, `d->rec_len =
used`. Maydonlarni to'ldiring, `bwrite(b)`, `c->done = true`, `return true`.

**`dir_add`:** nom uzunligini tekshiring (`-ENAMETOOLONG`), `dir_iterate(dir, add_visit, &c)` ni
chaqiring. Joy topilmasa: `bmap(dir, dir->size / blok, true, &blk)` bilan papkaga yangi blok qo'shing,
yozuvni blok boshiga yozing (`rec_len = blok hajmi`), `dir->size += blok`, vaqt va `write_inode`.

**Test:** `fstest /mnt` — 300 ta fayl (papka ~15 blokka o'sadi), yarmini o'chirib, 100 ta yangisini
qo'shish; e2fsck.

## 21. `setup_frame` — `kernel/proc/signal.c`

**Vazifa:** handler'ni chaqirish uchun user stekini tayyorlang.
1. `SA_RESTORER` yoki `sa_restorer` bo'lmasa → -1.
2. `struct sigframe` ni to'ldiring: `retaddr = sa_restorer`, `ctx` ga freymdagi **barcha** registrlar,
   `rip`, `rflags`, `rsp`, joriy `sig_blocked` va `sig`.
3. Joy: `sp = f->rsp - 128 - sizeof(fr)` (128 — **red zone**!), keyin shunday tekislang: handler
   kirishida `RSP % 16 == 8` bo'lsin (go'yo `call` qilingandek). `copy_to_user` (xato → -1).
4. Freymni o'zgartiring: `rip = handler`, `rsp = sp`, `rdi = sig`, `rsi = rdx = rax = 0`, RFLAGS'dan TF va
   DF ni o'chiring.
5. `proc_lock` ostida: `sig_blocked |= sa_mask` (SIGKILL/SIGSTOP dan tashqari); `SA_NODEFER` bo'lmasa,
   `sig` ning o'zi ham bloklanadi; `SA_RESETHAND` bo'lsa → `SIG_DFL`.

**Test:** `sigtest` (17 ta tekshiruv). **Tekislash haqida:** ABI talabi buzilsa, SSE ishlatadigan kod
(oddiy Linux dasturlarida printf ham shunday) `movaps` da #GP beradi. Bizning user dasturlari
`-mgeneral-regs-only` bilan yig'iladi, shuning uchun bu xato bizda **jim** qolishi mumkin. Tekislashni
qo'lda tekshiring: handler'da `(uintptr_t)__builtin_frame_address(0)` 16 ga karrali bo'lishi kerak.

## 22. `sigreturn` — `kernel/proc/signal.c` (`sys_sigreturn`)

**Vazifa:** handler `ret` qilgach, `f->rsp` `struct sigctx` ga ko'rsatadi. Uni `copy_from_user` bilan
o'qing (xato → jarayon SIGSEGV bilan tugaydi: `proc_exit_signal`).
- **Tekshiring:** `rip` va `rsp` `< 0x0000800000000000` bo'lishi kerak (user hududi).
- Barcha registrlarni freymga qaytaring. `rflags = (saqlangan & 0xCD5) | IF | 2`: faqat arifmetik bayroqlar
  va DF (0xCD5 = CF, PF, AF, ZF, SF, DF, OF bitlari). **IOPL, IF va TF ni dasturga bermang!**
- `f->vector = FORCE_IRET_VECTOR` — `sysret` RCX/R11 ni buzadi, shuning uchun `iretq` yo'li kerak.
- `proc_lock` ostida: `sig_blocked = saqlangan niqob` (SIGKILL/SIGSTOP'siz).
- `return c.rax` — dispetcher uni `f->rax` ga yozadi (aks holda to'xtatilgan kodning RAX i yo'qoladi).

**Xavfsizlik savoli:** RFLAGS'ni tekshirmasangiz, dastur `IOPL=3` ni o'rnatib, `cli` va portlarga
to'g'ridan-to'g'ri murojaat qila oladi. Bu butun tizim ustidan nazoratni qo'lga olish degani. Haqiqiy
yadrolarda aynan shunday zaifliklar bo'lgan.

---

## Keyingi qadam

22 ta lab'ni tugatganingizdan keyin `QOLLANMA.md` ning 11-bo'limiga o'ting va butun yadroni yangi papkada
**noldan** yozing. Endi har bir qatlamning eng qiyin qismini qo'lingiz o'zi biladi.
