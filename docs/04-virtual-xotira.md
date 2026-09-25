# 04 — Virtual xotira: sahifa jadvallari va manzil maydonlari

> **Eslatma (v0.2):** bu bo'lim loyihaning birinchi, **kichik** versiyasini tasvirlaydi
> (`git checkout e5906bb`). O'sha versiyada yadro 1 MB da, identity paging bilan ishlagan.
> Hozirgi yadro **higher-half**, **buddy + slab** allocatorlar va GRUB/Multiboot2 bilan ishlaydi:
> [09-yangi-arxitektura.md](09-yangi-arxitektura.md) ga qarang. G'oyalarni tushunish uchun avval
> shu bo'limni, keyin 09-bo'limni o'qing: oddiydan murakkabga.


**Kod:** `kernel/mm/vmm.c`, `kernel/mm/vmm.h`

## Virtual xotira nimani hal qiladi?

Virtual xotirasiz barcha dasturlar bitta fizik RAM'ni to'g'ridan-to'g'ri ko'radi. Bu uchta muammoga olib keladi:

1. **Himoya yo'q.** Xato qilgan dastur boshqasining (yoki yadroning) xotirasini buzadi.
2. **Manzil to'qnashuvi.** Ikki dastur bir xil manzilga link qilingan bo'lsa, ikkalasini bir vaqtda yuklab bo'lmaydi.
3. **Fragmentatsiya.** Dasturga 1 MB uzluksiz joy kerak, RAM'da esa faqat tarqoq bo'sh freymlar qolgan.

Sahifalash (paging) uchalasini hal qiladi: har bir jarayon **o'z sahifa jadvaliga** ega va CPU har bir
xotira murojaatini shu jadval orqali tarjima qiladi.

```
 Jarayon A (CR3 = 0x7f000)              Fizik RAM                 Jarayon B (CR3 = 0x7e000)
 0x40000000 ───────────────────────►  freym 0x0234000
                                       freym 0x0512000  ◄────────── 0x40000000
 0x40001000 ───────────────────────►  freym 0x0101000  (bir xil virtual manzil - har xil freym!)
```

## Tarjima qanday ishlaydi

```
virt = 0x0000_0000_4000_5123
        │
        ├─ bitlar 47..39 = 0   → PML4[0]   → PDPT jadvali
        ├─ bitlar 38..30 = 1   → PDPT[1]   → PD jadvali
        ├─ bitlar 29..21 = 0   → PD[0]     → PT jadvali
        ├─ bitlar 20..12 = 5   → PT[5]     → freym 0x0234000
        └─ bitlar 11..0  = 0x123           → fizik = 0x0234123
```

`vmm.c` dagi `walk()` funksiyasi aynan shu yo'lni bosib o'tadi, `next_table()` esa yo'qolgan jadvallarni
yaratadi. `vmm_translate()` esa xuddi CPU kabi tarjimani dasturiy ravishda bajaradi.

### Sahifa yozuvi bayroqlari

| Bit | Nomi | Ma'nosi |
|---|---|---|
| 0 | P | sahifa mavjud (0 bo'lsa → page fault) |
| 1 | W | yozish mumkin |
| 2 | U | ring 3 murojaat qila oladi |
| 7 | PS | katta sahifa (PD darajasida 2 MB) |
| 63 | NX | bajarib bo'lmaydi (bizda o'chiq — mashq) |

**Muhim:** samarali ruxsat — barcha 4 darajadagi bayroqlarning **mantiqiy VA** (AND) si. Agar PD yozuvida
U=0 bo'lsa, PT'da U=1 bo'lsa ham user kirolmaydi.

## Bizning manzil maydoni dizayni

```
0x0000_0000_0000_0000  0-sahifa: XARITALANMAGAN (NULL → darhol #PF)
0x0000_0000_0000_1000  yadro: 0..1 GB identity (U=0)  ← barcha jarayonlarda UMUMIY (boot_pd)
0x0000_0000_4000_0000  user kodi (ELF), heap ↓
                       ...
0x0000_0000_8000_0000  ↑ user steki
0x0000_0080_0000_0000  user hududi oxiri
```

**Asosiy hiyla:** har bir jarayonning `PDPT[0]` yozuvi **bitta umumiy** `boot_pd` jadvaliga ko'rsatadi.
Shu tufayli:
- CR3 almashtirilganda yadro kodi, steki va heap'i "yo'qolib qolmaydi" (aks holda keyingi instruksiyada qulardik);
- yadro istalgan fizik freymga `(void *)phys` orqali murojaat qila oladi (`phys_to_virt`).

### NULL himoyasi

`boot.asm` birinchi 2 MB ni bitta katta sahifa bilan xaritalagan edi — demak `*(int *)0 = 1` xato
**bermasdi**, jimgina BIOS ma'lumotlarini buzardi. `vmm_init()` birinchi 2 MB ni 512 ta 4 KB sahifaga
bo'ladi va 0-sahifani xaritalamaydi. Endi:

```
!!! EXCEPTION 14: Page Fault (sahifa xatosi)
  Manzil (CR2) = 0x0000000000000000
  Sabab: sahifa mavjud emas, YOZISH, yadro rejimida
```

## TLB — tarjima keshi

Har bir xotira murojaatida 4 ta jadvalni o'qish juda sekin bo'lardi. CPU tayyor tarjimalarni **TLB**da
saqlaydi. Jadvalni o'zgartirsangiz, TLB'da **eski** tarjima qolishi mumkin! Shuning uchun:

- bitta sahifani o'zgartirganda: `invlpg` (`cpu_invlpg`);
- CR3 ga yozish butun TLB'ni tozalaydi (global sahifalardan tashqari).

TLB bilan bog'liq xatolar eng qiyin topiladigan xatolar qatoriga kiradi: kod "ba'zan" ishlaydi.

## Xavfsizlik: user ko'rsatkichlarini tekshirish

7-bosqichda user dastur `write(1, buf, len)` chaqiradi. Agar `buf` yadro manzilini ko'rsatsa-chi? Yadro
ring 0 da ishlaydi va **hamma narsani o'qiy oladi** — tekshirmasak, user dastur yadro xotirasini o'qib olishi mumkin.
`vmm_user_range_ok()` quyidagilarni tekshiradi:
- manzil user hududidami;
- `virt + len` butun son to'lib ketishi (overflow) bilan aylanib qolmaganmi;
- har bir sahifa xaritalanganmi, U=1 va (kerak bo'lsa) W=1 mi.

## Xotira oqishi (memory leak) testi

`selftest` manzil maydonini yaratib, sahifalarni bog'lab, keyin yo'q qiladi va **PMM hisoblagichi
boshlang'ich qiymatga qaytganini** tekshiradi. Bu oddiy, lekin juda kuchli usul: bitta PT jadvalini
bo'shatishni unutsangiz ham test yiqiladi.

## Sinab ko'ring

1. `vmm_init()` da `low_pt[0] = 0` o'rniga 0-sahifani xaritalang. NULL yozish endi nima qiladi?
2. `vmm_map_page` dagi `cpu_invlpg` ni o'chiring va testni ishga tushiring. Nega hali ham o'tadi?
   (Maslahat: test sahifani xaritalashdan **oldin** unga murojaat qilmaydi. Qachon buziladi?)
3. **Mashq:** NX bitini yoqing (`EFER.NXE`) va user stekini bajarib bo'lmaydigan qiling.
4. **Katta mashq:** yadroni higher-half'ga (`0xFFFFFFFF80000000`) ko'chiring.
