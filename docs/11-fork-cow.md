# 11 — Unix jarayon modeli: VMA, demand paging, fork + copy-on-write, exec, mmap

Kod: `kernel/mm/mm.c`, `kernel/proc/exec.c`, `kernel/sys/elf.c`, `kernel/arch/interrupts.c` (page fault),
`kernel/sys/syscall.c`, `user/bin/{sh.c,forktest.c}`

## 1. VMA — "qayerda nima bo'lishi KERAK"

Har bir jarayon endi `struct mm` ga ega: sahifa jadvali va **VMA'lar ro'yxati**.

```
0x400000        ┌──────────────┐  VMA_ELF   r-x   kod          ← ELF segmentlari (elf.c)
                ├──────────────┤  VMA_ELF   rw-   ma'lumotlar
brk_start       ├──────────────┤  VMA_HEAP  rw-   heap (sbrk)
                │      ...     │
MMAP_TOP pastga ├──────────────┤  VMA_ANON  rw-   mmap(NULL, ...)
                │      ...     │
                ├──────────────┤  VMA_STACK rw-   stek (8 MB gacha o'sadi)
USER_STACK_TOP  └──────────────┘
```

**VMA va sahifa jadvali bir xil narsa emas.** VMA ruxsat etilgan hududni tasvirlaydi, sahifa jadvali esa
hozir RAM'da bor sahifalarni ko'rsatadi. Ular orasidagi farqni **page fault handler** to'ldiradi.

## 2. Page fault — endi xato emas, ish vositasi

```
user dastur manzilga tegdi ──► #PF ──► mm_handle_fault(cr2, err)
                                        ├─ VMA yo'q? stek ostidami? → stekni kengaytir
                                        ├─ VMA yo'q                  → Segmentation fault
                                        ├─ ruxsat yo'q (W/X)         → Segmentation fault
                                        ├─ sahifa yo'q               → DEMAND PAGING: nollangan sahifa
                                        └─ yozish + COW bayrog'i     → COPY-ON-WRITE: nusxa
```

Muvaffaqiyatli bo'lsa, CPU **aynan o'sha instruksiyani qaytadan bajaradi** va dastur hech narsani sezmaydi.

### Demand paging

```
mmap 32 MB dan keyin:           0 KB ishlatildi (faqat VMA yaratildi)
32 ta sahifaga tekkandan keyin: 200 KB (128 KB sahifalar + sahifa jadvallari)
```

`malloc(100 MB)` darhol 100 MB RAM egallamaydi. Linux ham shunday ishlaydi, shuning uchun `top` da **VIRT**
va **RES** ustunlari alohida ko'rsatiladi. Shell endi 80 KB o'rniga 20 KB ishlatadi.

## 3. fork + copy-on-write

`fork()` jarayonning to'liq nusxasini yaratishi kerak. Barcha sahifalarni nusxalash sekin, bundan tashqari
foydasiz: shell'da bola odatda darhol `exec` qiladi. Shuning uchun:

1. Bola **o'sha fizik sahifalarni** oladi (`get_page` → `refcount++`).
2. Ota va bolada yoziladigan sahifalar **faqat o'qish + `PTE_COW`** qilib belgilanadi (9-bit — dasturiy bit,
   CPU unga e'tibor bermaydi).
3. Kimdir yozsa, #PF yuz beradi. Agar `refcount == 1` bo'lsa (boshqa hech kim ulashmayapti), sahifa
   o'zlashtiriladi, aks holda **nusxa** olinadi va eskisiga `put_page` qilinadi.
4. Otaning TLB'si tozalanadi (CR3 qayta yuklanadi), chunki uning ruxsatlari o'zgardi.

`forktest` tekshiradi: bola `global_value = 999` yozadi, ota esa hali ham `100` ni ko'radi.

### Bola qayerdan davom etadi?

Otaning syscall freymi bolaning yadro stekiga **nusxalanadi**, bolada `RAX = 0` qilinadi. Bola birinchi marta
ishga tushganda `iretq` orqali otasi bilan **aynan bir xil joyga** qaytadi, faqat `fork()` natijasi 0 bo'ladi.

## 4. exec

1. Yangi `mm` **to'liq** quriladi: ELF segmentlari VMA'larga yoziladi, keyin heap, stek va argv. **Eski
   manzil maydoniga hali tegilmaydi**, shuning uchun xato bo'lsa `exec` shunchaki `-1` qaytaradi.
2. Muvaffaqiyatli bo'lsa: `mm` almashtiriladi (`proc_lock` ostida, `ps` jadvallarni yurayotgan bo'lishi
   mumkin), CR3 yangilanadi va eski `mm` yo'q qilinadi.
3. Syscall freymi qayta yoziladi: `RIP = entry`, `RSP = yangi stek`, `RDI = argc`, `RSI = argv`. Syscall'dan
   "qaytish" yangi dasturning boshiga olib boradi.

**Muhim:** argv **exec'dan oldin** yadroga nusxalanadi, chunki keyin eski xotira bo'lmaydi.

## 5. Shell — klassik Unix

```c
pid = fork();
if (pid == 0) {                 // bola
    exec(args[0], args);        // o'zini dasturga aylantiradi
    exit(127);                  // exec qaytdi = topilmadi
}
wait(pid, &status, 0);          // ota
```

Nega ikki qadam? Fork va exec orasida bola o'z muhitini sozlaydi: fayllarni yo'naltiradi (`ls > f`), pipe
ulaydi (`ls | cat`). Ota jarayonga esa tegilmaydi. Keyingi bosqichda aynan shu ishlatiladi.

## 6. Syscall'lar va user xotirasi

Demand paging tufayli yangi muammo paydo bo'ldi: `read(fd, buf, n)` chaqirilganda `buf` sahifalari **hali
yaratilmagan** bo'lishi mumkin. `user_ok()` endi `mm_prefault()` ni chaqiradi: u VMA'ni tekshiradi, kerakli
sahifalarni yaratadi va yozish kerak bo'lsa COW'ni buzadi. Shundan keyin yadro user xotirasiga page fault'siz
murojaat qila oladi. (Linux'dagi `get_user_pages` / exception fixup jadvallari shu muammoning boshqacha
yechimlari.)

## 7. Sahifa jadvallarini qaytarish

`munmap` sahifalarni qaytargach, **bo'shab qolgan PT/PD/PDPT jadvallari** ham buddy'ga qaytariladi
(`vmm_prune_tables`). Bu qilinmasa, har bir `mmap/munmap` tsikli 4–72 KB xotira "yo'qotadi". `forktest` buni
tekshiradi.

## Sinab ko'ring

```
myos$ forktest
myos$ ps                  ← XOTIRA ustuni: faqat haqiqatan ishlatilgan sahifalar
myos$ crash stack         ← stek 8 MB gacha o'sadi, keyin Segmentation fault
```

1. `fault_page` dagi `refcount == 1` optimizatsiyasini olib tashlang. Nima o'zgaradi? (`free` bilan o'lchang.)
2. `mm_fork` dan keyin CR3 qayta yuklanmasa, nima bo'lishi mumkin?
3. **Mashq:** `mprotect()` syscall'ini qo'shing (VMA'ni bo'lish kerak bo'ladi).
4. **Mashq:** fayl bilan `mmap` (VFS'dan keyin) va `MAP_SHARED`.
5. **Katta mashq:** swap: kam ishlatiladigan sahifalarni diskka chiqarish (LRU, `PTE_ACCESSED` biti).
