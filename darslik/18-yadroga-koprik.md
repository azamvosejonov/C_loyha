# 18-bob. Freestanding C: yadroga ko'prik

> **Bu bobdan keyin:** operatsion tizimsiz ("yalang'och" apparatda) C qanday ishlashini, yadro
> kompilyatsiya bayroqlarini, linker skriptini va MyOS kodini qayerdan va qanday o'qishni bilasiz.
> Bu bob darslikni MyOS bilan bog'laydi — bundan keyin siz yadro ichida ishlaysiz.

## 18.1. Hosted va freestanding

Siz shu paytgacha yozgan dasturlar — **hosted** muhitda: ostida OS bor, `main` ni kimdir chaqiradi,
`printf`, `malloc`, fayllar mavjud.

Yadro esa **freestanding** muhitda: ostida hech narsa yo'q. Yadroning o'zi — boshqalar uchun "ost".

| | Hosted (oddiy dastur) | Freestanding (yadro) |
|---|---|---|
| Kirish nuqtasi | `main` (libc'ning `_start` i chaqiradi) | o'zingiz belgilaysiz (MyOS: `boot.asm` → `kmain`) |
| Standart kutubxona | bor | **yo'q** — faqat `<stdint.h>`, `<stddef.h>`, `<stdbool.h>`, `<stdarg.h>`, `<limits.h>` |
| Xotira | `malloc` | o'zingiz yozasiz (buddy, slab) |
| Chiqish | `printf` → `write` | to'g'ridan-to'g'ri ekran xotirasiga / serial portga |
| Xato | segfault → OS dasturni o'ldiradi | page fault → **o'zingizning** ishlovchingiz; ishlovchi yo'q bo'lsa — triple fault, kompyuter qayta yuklanadi |
| Stek | 8 MB, avtomatik o'sadi | 8–16 KB, o'zingiz ajratasiz, to'lsa — qulash |

## 18.2. Yadro kompilyatsiya bayroqlari

MyOS `Makefile`'idagi `KERNEL_CFLAGS` — har biri nega kerak (Makefile'da ham izohlangan):

| Bayroq | Nega |
|---|---|
| `-ffreestanding` | Kompilyator libc borligini taxmin qilmaydi (masalan, `printf` → `puts` almashtirmaydi) |
| `-nostdlib` (link) | Hech qanday standart kutubxona va `crt0` ulanmaydi |
| `-fno-stack-protector` | Stek himoyasi glibc'dagi `__stack_chk_fail` ga tayanadi — u yo'q |
| `-mno-red-zone` | Uzilishlar red zone'ni buzadi (17-bob) |
| `-mgeneral-regs-only` | SSE/AVX registrlari ishlatilmaydi — ularni har uzilishda saqlash kerak bo'lardi |
| `-mcmodel=kernel` | Kod manzilning eng yuqori 2 GB ida (`0xFFFFFFFF80000000+`) |
| `-fno-pic -fno-pie` | Yadro aniq manzilga yuklanadi — "joyi o'zgaruvchan" kod kerak emas |
| `-Wall -Wextra -Werror` | Yadroda xato — butun tizimning qulashi |

**Muhim nozik joy:** `-ffreestanding` bo'lsa ham GCC struct nusxalash yoki katta massivni nollash uchun
`memcpy`/`memset` chaqiruvini **o'zi** qo'shishi mumkin. Shuning uchun yadro ularni albatta ta'riflashi
kerak (MyOS: `kernel/lib/string.c`).

## 18.3. Linker skripti — yadro xotirada qayerda turadi

Oddiy dasturni OS joylashtiradi. Yadroni esa siz aytgan manzilga yuklovchi (GRUB) qo'yadi. Buni
**linker skripti** belgilaydi (MyOS: `kernel/linker.ld`, soddalashtirilgan):

```ld
ENTRY(_start)                                   /* birinchi bajariladigan belgi (boot.asm) */
KERNEL_VMA = 0xFFFFFFFF80000000;                /* yadro ishlaydigan virtual manzil */

SECTIONS
{
    . = 1M;                                     /* "." - joriy manzil: fizik 1 MB dan */
    .boot : ALIGN(8) { KEEP(*(.multiboot2)) ... }   /* yuklovchi sarlavhasi + 32 bitli kod */

    . = ALIGN(4K);
    . += KERNEL_VMA;                            /* bundan keyingi hammasi yuqori yarmida */
    __kernel_start = .;
    .text   : AT(ADDR(.text) - KERNEL_VMA)   ALIGN(4K) { ... *(.text*) ... }
    .rodata : AT(ADDR(.rodata) - KERNEL_VMA) ALIGN(4K) { ... }
    .data   : AT(ADDR(.data) - KERNEL_VMA)   ALIGN(4K) { ... }
    .bss    : AT(ADDR(.bss) - KERNEL_VMA)    ALIGN(4K) { ... }
    . = ALIGN(4K);
    __kernel_end = .;                           /* C kodidan: extern char __kernel_end[]; */
}
```

- `.` — joriy manzil hisoblagichi; `. = 1M;` uni o'rnatadi (`;` — buyruq oxiri, linker skriptida ham).
- `ALIGN(4K)` — har bir bo'lim yangi sahifadan: shunda `.text` ni "faqat o'qish + bajarish", `.data` ni
  "o'qish + yozish, bajarib bo'lmaydi" qilib xaritalash mumkin (W^X himoyasi).
- `AT(...)` — **yuklash** manzili (fizik) va **ishlash** manzili (virtual) farqli: GRUB fizik 1 MB ga
  yuklaydi, kod esa yuqori yarmida ishlaydi. `boot.asm` sahifa jadvalini shunday sozlaydiki, ikkalasi
  ham bir xil fizik xotiraga ko'rsatadi.
- Skriptdagi belgilar (`__kernel_end`, `__text_start`...) C'dan manzil sifatida o'qiladi — "yadro qayerda tugaydi, bo'sh
  xotira qayerdan boshlanadi".

To'liq variantni `kernel/linker.ld` da o'qing — har bir qator izohlangan.

## 18.4. Birinchi C funksiyasigacha

CPU yoqilganda C ishlay olmaydi: stek yo'q, sahifalar yo'q, hatto 64 bitli rejim ham yo'q.
`kernel/boot/boot.asm` shularni tayyorlaydi:

```text
GRUB: 32 bitli himoyalangan rejim, sahifalashsiz, Multiboot2 ma'lumoti ebx da
  → boot.asm: vaqtinchalik stek
  → boot.asm: sahifa jadvallari (identity + yuqori yarmi)
  → boot.asm: CR3, PAE, EFER.LME, CR0.PG → 64 bitli rejim
  → boot.asm: yuqori yarmidagi manzilga sakrash, yangi stek
  → call kmain(magic, mbi_phys)           ← birinchi C funksiyasi (kernel/main.c)
```

Keyin `kmain` qatlamma-qatlam tizimni quradi — `kernel/main.c` ni oching: har bir qator
izohlangan, tartib esa muhim (xotirasiz hech narsa ishlamaydi, uzilishlarsiz taymer yo'q...).

## 18.5. Yadroda "libc'siz" yashash — qo'llanma

| Ehtiyoj | MyOS'da |
|---|---|
| `printf` | `kprintf` (`kernel/lib/kprintf.c`) — ekran, serial va `dmesg` buferiga |
| `malloc` | `kmalloc`/`kfree` (slab), `alloc_pages` (buddy), `vmalloc` |
| `assert`/`abort` | `panic("...")` — hamma CPU'larni to'xtatib, xabarni chiqaradi |
| `errno` | yo'q — funksiyalar `-ENOMEM` kabi manfiy kod qaytaradi |
| mutex | `spinlock_t`, `struct mutex` |
| ro'yxatlar | `struct list_head` (`kernel/lib/list.h`) — 23-mashqdagi aynan o'sha |
| `sleep` | `proc_sleep`/`proc_wakeup` — kutish navbatlari |
| vaqt | `timer_ticks()`, `udelay()` (TSC) |
| `memcpy`/`strlen` | `kernel/lib/string.c` |

## 18.6. Yadroda xato qilish qanday ko'rinadi

- **Page fault** (noto'g'ri manzil) → `kernel/arch/interrupts.c` ishlovchisi: user dasturda bo'lsa —
  SIGSEGV, yadroda bo'lsa — PANIC: RIP, CR2 (xato manzili), registrlar va stek.
- **Triple fault** — istisno ishlovchisining o'zida istisno → CPU qayta yuklanadi (ekranda hech narsa
  yo'q, kompyuter shunchaki qayta yonadi). QEMU'da: `-d int,cpu_reset -no-reboot`.
- **Qotib qolish** — cheksiz sikl yoki deadlock. `make debug` + gdb bilan to'xtatib, `bt`.

Debug: `docs/08-test-debug.md` — gdb bilan yadroni qadamma-qadam bajarish, `addr2line` bilan PANIC
manzilini qatorga aylantirish.

## 18.7. MyOS kodini qanday o'qish kerak

1. **Kichikdan boshlang.** `git checkout e5906bb` — v0.1: ~6800 qatorli (izohlar bilan) sodda yadro (`docs/01`–`08`).
   Uni tushunmasdan hozirgi versiyaga o'tmang. Tugatgach: `git checkout -` bilan qayting.
2. **Bosqichma-bosqich.** `git log --oneline --reverse` — har bir commit bitta bosqich. Har birining
   `git show --stat <commit>` i qaysi fayllar qo'shilganini ko'rsatadi.
3. **Hujjat + kod birga.** `docs/09`–`15` — har biri qaysi faylni o'qishni aytadi.
4. **Faylni yuqoridan pastga.** Har bir fayl boshida katta izoh: nima qiladi, qanday ishlaydi, nega shunday.
5. **Buzing.** Har bir hujjat oxiridagi "Sinab ko'ring" — kodni ataylab buzib, natijani kuzating.
6. **Lab'lar.** `tools/lab.py` — funksiyani o'chirib, qayta yozish (labs/README.md).

## 18.8. Darslikdan keyin — to'liq yo'l

```text
darslik 00-18 + mashqlar 01-30      C tili, xotira, tizim chaqiruvlari
      ↓
mashqlar 31-40                      yadro mexanizmlari ODDIY DASTUR sifatida:
                                    sahifa jadvali, buddy, slab, spinlock, scheduler, ELF, ext2
      ↓
docs/00-15 + MyOS kodi              haqiqiy yadroni o'qish
      ↓
labs (22 ta)                        yadro ICHIDA funksiyalarni qayta yozish
      ↓
QOLLANMA.md 11-bo'lim               yadroni NOLDAN yozish
      ↓
YAKUNIY.md IV qism                  yangi drayverlar va quyi tizimlar (NVMe, USB, tarmoq)
```

## 18.9. O'zingizni tekshiring

1. Nega yadroda `printf` yo'q, lekin `<stdint.h>` bor?
2. `-ffreestanding` bo'lsa ham nega yadro `memcpy` ni ta'riflashi shart?
3. Linker skriptidagi `.` va `ALIGN(4K)` nima?
4. Yadro kodida page fault bo'lsa nima bo'ladi? Istisno ishlovchisining o'zida-chi?
5. MyOS'ni o'qishni qaysi versiyadan boshlash kerak?

<details><summary>Javoblar</summary>

1. `printf` — kutubxona kodi (OS'ga tayanadi); `<stdint.h>` — faqat tur ta'riflari, kod yo'q.
2. GCC struct nusxalash va nollash uchun `memcpy`/`memset` chaqiruvlarini o'zi qo'shishi mumkin.
3. `.` — joriy manzil hisoblagichi; `ALIGN(4K)` — uni 4096 ga karrali qilib surish.
4. Yadroning page fault ishlovchisi PANIC beradi; ishlovchining o'zida xato → double/triple fault → qayta yuklanish.
5. v0.1 (`git checkout e5906bb`), `docs/01`–`08` bilan.
</details>

## 18.10. Mashqlar

- **31–40** (yadro mexanizmlari oddiy dastur sifatida) — [mashqlar/README.md](../mashqlar/README.md).
- MyOS: `kernel/main.c` ni boshidan oxirigacha o'qing va har bir `*_init()` chaqiruvi qaysi faylga olib
  borishini daftaringizga yozing. Bu — yadroning "mundarijasi".

**Tabriklayman — C darsligi tugadi.** Endi siz yadro kodini o'qiy oladigan darajadasiz. Qolgan hamma
narsa — amaliyot: har kuni kod yozish va o'qish.
