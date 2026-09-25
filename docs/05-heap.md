# 05 — Yadro heap'i: slab allocator (`kmalloc` / `kfree`)

> **Eslatma (v0.2):** bu bo'lim loyihaning birinchi, **kichik** versiyasini tasvirlaydi
> (`git checkout e5906bb`). O'sha versiyada yadro 1 MB da, identity paging bilan ishlagan.
> Hozirgi yadro **higher-half**, **buddy + slab** allocatorlar va GRUB/Multiboot2 bilan ishlaydi:
> [09-yangi-arxitektura.md](09-yangi-arxitektura.md) ga qarang. G'oyalarni tushunish uchun avval
> shu bo'limni, keyin 09-bo'limni o'qing: oddiydan murakkabga.


**Kod:** `kernel/mm/heap.c`, `kernel/tests/crashdemo.c`, `kernel/tests/selftest.c`

## Muammo

PMM faqat 4 KB lik butun freymlar beradi. Yadroga esa ko'pincha kichik obyektlar kerak:
jarayon strukturasi (~200 bayt), fayl deskriptori (~32 bayt), satr buferi... 40 baytlik obyektga 4 KB berish
xotiraning **99%** ini isrof qiladi.

## Slab g'oyasi

Jeff Bonwick (SunOS, 1994) taklif qilgan va hozir Linux'da (SLUB) ishlatiladigan g'oya:

1. Obyektlarni **o'lcham sinflari**ga ajratamiz: 16, 32, 64, 128, 256, 512, 1024.
2. Har bir sinf uchun 4 KB lik sahifalar ("slab") olib, **bir xil** katakchalarga bo'lamiz.
3. Bo'sh katakchalar **zanjir** (free list) hosil qiladi. Zanjir ko'rsatkichi bo'sh obyektning **ichida**
   saqlanadi, shuning uchun qo'shimcha xotira kerak emas.

```
slab sahifasi (sinf = 64 bayt):
┌──────────┬───────┬───────┬───────┬───────┬─────┐
│ sarlavha │ obj 0 │ obj 1 │ obj 2 │ obj 3 │ ... │     sarlavha: magic, sinf, in_use,
│  64 B    │ band  │ bo'sh │ band  │ bo'sh │     │               free_list, prev/next
└──────────┴───────┴───┬───┴───────┴───▲───┴─────┘
      free_list ───────┘   next ────────┘
```

| Amal | Qanday | Murakkablik |
|---|---|---|
| `kmalloc(50)` | 64 baytlik sinf → bo'sh joyi bor slab → free list boshidan olish | **O(1)** |
| `kfree(p)` | `p & ~0xFFF` = sahifa boshi = sarlavha → sinfni bilamiz → free list boshiga | **O(1)** |

Nega `kfree` hajmni so'ramaydi? Chunki obyekt qaysi sahifada ekani manzilning o'zidan ma'lum, sahifa
sarlavhasi esa sinfni biladi. Bu **manzil arifmetikasi** hiylasi yadro dasturlashida juda ko'p uchraydi.

1024 baytdan katta so'rovlar PMM'dan ketma-ket freymlar oladi va birinchi 64 baytga o'z sarlavhasini yozadi.

## Bo'sh slab siyosati

Slab butunlay bo'shasa, sahifani PMM'ga qaytaramiz — **agar sinfda boshqa slab bo'lsa**. Bittasini
zaxirada qoldiramiz: aks holda "1 ta obyekt ajrat — bo'shat" tsiklida har safar sahifani olib-qaytarib
yurardik (**thrashing**).

## Xotira xatolarini aniqlash

Bu allocator'ning eng qimmatli qismi. C'da xotira xatolari **jim** bo'ladi: noto'g'ri yozuv darhol
qulashga olib kelmaydi, balki boshqa ma'lumotni buzadi va dastur ancha keyin, butunlay boshqa joyda qulaydi.
Biz ularni **sodir bo'lgan joyiga yaqin** ushlaymiz:

| Xato | Qanday aniqlanadi | Namoyish |
|---|---|---|
| **Use-after-free** | bo'shatilgan obyekt `0x6B` bilan "zaharlanadi"; `kmalloc` uni qayta berishdan oldin zahar butunligini tekshiradi | `make run APPEND=demo=uaf` |
| **Double free** | bo'sh obyektning 8..15 baytlarida `FREE_MAGIC` | `APPEND=demo=doublefree` |
| **Noto'g'ri `kfree`** | sahifa sarlavhasida `SLAB_MAGIC` yoki `LARGE_MAGIC` yo'q | `APPEND=demo=badfree` |
| **Obyekt o'rtasiga ko'rsatkich** | `(p - base) % obj_size != 0` | — |
| **NULL** | 0-sahifa xaritalanmagan (4-bosqich) | `APPEND=demo=null` |
| **Stek to'lishi** | stek ostida **guard page** → #PF → #DF → IST stek | `APPEND=demo=stack` |

Namuna:

```
[demo] 'uaf' namoyishi: ataylab xato qilamiz...
*** KERNEL PANIC ***
heap: USE-AFTER-FREE! bo'shatilgan obyekt 0x124040 ning 20-baytiga yozilgan (sinf 64 bayt)
```

Bu Linux'dagi `SLUB_DEBUG`, `KASAN` va user-space'dagi AddressSanitizer bilan bir xil g'oya.

### Guard page va double fault — batafsil

Yadro steki `.bss` da, sahifa jadvallaridan keyin turadi. Guard page bo'lmasa, stek to'lganda sahifa
jadvallarining ustiga yozib yuborardi. Natijada CPU butunlay tushunarsiz holatga tushib, qayta yuklanardi.
Guard page bilan quyidagi zanjir bo'ladi:

1. stek guard page'ga yetadi → **#PF**;
2. CPU #PF freymini **o'sha** stekka qo'ymoqchi bo'ladi → yana xato → **#DF** (double fault);
3. #DF IDT'da IST1 bilan belgilangan → CPU **toza** stekka o'tadi → handler xabar chiqaradi.

Backtrace'da bitta manzil 16 marta takrorlanadi. Bu cheksiz rekursiyaning aniq belgisi.

## Stress test

`selftest` 500 ta tasodifiy o'lchamdagi (1..3000 bayt) blok ajratadi, har biriga o'z naqshini yozadi, hammasini
tekshiradi va **tasodifiy tartibda** bo'shatadi. Oxirida `bytes_in_use` boshlang'ich qiymatga qaytishi kerak.
Tasodifiy sonlar uchun **LCG** ishlatiladi, doim bir xil urug' (seed) bilan: test har safar bir xil ishlashi,
ya'ni **takrorlanadigan** bo'lishi kerak.

## Sinab ko'ring

1. `POISON_FREE` tekshiruvini o'chiring va `demo=uaf` ni ishga tushiring. Endi nima bo'ladi?
2. `HEADER_SIZE` ni 40 qiling. Qaysi `_Static_assert` yoki test yiqiladi va nega? (Tekislash!)
3. **Mashq:** `krealloc()` yozing.
4. **Mashq:** har bir ajratma oxiriga "redzone" (masalan, 8 bayt `0xBB`) qo'shib, **buffer overflow**ni ham
   aniqlang.
