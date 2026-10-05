# 3-bob. ALU: arifmetika, shartlar, yuklash, ko'paytirish va bo'lish

> **Bu bobda nima o'rganasiz:** protsessorning hisoblash qismi (ALU); ishorali/ishorasiz amallar qayerda farq
> qiladi; shartli sakrashlar; xotiradan o'qilgan baytni registrga kengaytirish; 64 bitli ko'paytmaning yuqori
> qismi; nolga bo'lish va toshishning RISC-V qoidalari (va nega C'da ular xavfli).
> **Oldindan nima kerak:** 1-bob (1.5, 1.6, 1.8), 2-bob.
> **Mashqlar:** E2, E3, E4 ([emu/alu.c](../emu/alu.c)).   **Vaqt:** 3 soat.

## Bu bob nima haqida?

ALU (arithmetic-logic unit) — protsessorning "kalkulyatori". U registrlar va xotira haqida hech narsa
bilmaydi: ikki son oladi, bitta son qaytaradi. Shuning uchun emulyatorda ALU **sof funksiyalar** sifatida
alohida faylda ([emu/alu.c](../emu/alu.c)) — ularni boshqa hech narsasiz sinash oson (birlik testlari
aynan shunday qiladi).

**Hayotdan misol: kassadagi kalkulyator.** Kassir mahsulotni skanerlaydi (fetch), narxini aniqlaydi (decode),
kalkulyatorga kiritadi (execute). Kalkulyator mahsulot nima ekanini bilmaydi — faqat sonlar. Lekin
"chegirma foizini qo'llash" yoki "qaytimni hisoblash" — turli tugmalar. ALU'da tugmalar — `funct3` va `funct7`.

## 3.1. Asosiy amallar — tayyor namuna

`alu_asosiy()` tayyor. Uni o'qing — keyingi mashqlarning uslubi xuddi shunday:

```c
uint32_t alu_asosiy(uint32_t funct3, int alt, int registr, uint32_t a, uint32_t b, int *xato)
{
    uint32_t siljish = b & 31;                  /* RV32: siljish miqdori — faqat pastki 5 bit (0..31) */
    *xato = 0;
    switch (funct3) {
    case 0:                                     /* add / sub (sub faqat R-turda, alt=1) */
        if (registr && alt)
            return a - b;
        return a + b;                           /* ishorasiz qo'shish: toshish 2^32 bo'yicha "aylanadi" — bu RISC-V da to'g'ri */
    case 1:                                     /* sll: chapga mantiqiy siljitish */
        return a << siljish;
    case 2:                                     /* slt: ishorali solishtirish -> 1/0 */
        return (int32_t)a < (int32_t)b;
    case 3:                                     /* sltu: ishorasiz solishtirish */
        return a < b;
    /* ... xor, srl/sra, or, and ... */
```

Uchta nozik joyga e'tibor bering:

1. **`b & 31`.** `sll a0, a0, a1` da a1 = 33 bo'lsa nima bo'ladi? Spetsifikatsiya: faqat pastki 5 bit
   ishlatiladi → 33 & 31 = 1. Bu C'ning UB'sidan (1.2) ham himoya qiladi: `a << 33` — UB, `a << (33 & 31)` — yo'q.
2. **`a + b` — ishorasiz.** `0x7FFFFFFF + 1` int32_t'da toshish (UB), uint32_t'da esa `0x80000000` —
   aniqlangan, va aynan RISC-V `add` natijasi. Shuning uchun registrlar `uint32_t`.
3. **`registr && alt`.** `funct7` ning 5-biti (`0x20`) R-turda `add`/`sub` ni ajratadi. I-turda esa
   o'sha bitlar o'zgarmasning bir qismi — `addi` da "subi" yo'q (manfiy son qo'shish yetarli).

## 3.2. E2 — shartli sakrash

```c
int shart_bajarildimi(uint32_t funct3, uint32_t a, uint32_t b);
/* 1 — sakrash kerak, 0 — kerak emas, -1 — bunday funct3 yo'q */
```

| funct3 | buyruq | shart |
|---|---|---|
| 0 | beq | a == b |
| 1 | bne | a != b |
| 2, 3 | — | **mavjud emas** |
| 4 | blt | a < b (ishorali) |
| 5 | bge | a >= b (ishorali) |
| 6 | bltu | a < b (ishorasiz) |
| 7 | bgeu | a >= b (ishorasiz) |

Nega `bgt` (katta) yo'q? Chunki `bgt a, b` = `blt b, a` — assembler registrlarni almashtirib qo'yadi.
RISC falsafasi: apparatda faqat zarur narsa.

Natija qaytgach `cpu.c` nima qiladi ([emu/cpu.c](../emu/cpu.c)):

```c
case 0x63: {                                /* BRANCH: shart bajarilsa pc <- pc + imm */
    int shart = shart_bajarildimi(f3, a, bq);
    if (shart < 0)
        return istisno(SABAB_NOTOGRI_BUYRUQ, b);
    if (shart) {
        uint32_t manzil = c->pc + imm_b(b);
        if (!sakrash_tekis(manzil))
            return istisno(SABAB_BUYRUQ_TEKIS_EMAS, manzil);
        *keyingi_pc = manzil;
    }
    return OK;
}
```

Muhim: `-1` qaytarish shart. Agar funct3=2 uchun 0 qaytarsangiz, noto'g'ri buyruq **jimgina** "sakramaydigan
shart" bo'lib bajariladi. Bunday buyruq haqiqiy protsessorda istisno beradi — yadro dasturni `SIGILL` bilan
to'xtatadi. Jim xato — eng yomon xato.

## 3.3. E3 — yuklangan qiymatni kengaytirish

`lb` (load byte) xotiradan **1 bayt** o'qiydi, lekin registr 32 bit. Qolgan 24 bitga nima yoziladi?

| buyruq | funct3 | o'qiladi | kengaytirish | `0x80` baytidan natija |
|---|---|---|---|---|
| lb | 0 | 1 bayt | ishora bilan | `0xFFFFFF80` (-128) |
| lh | 1 | 2 bayt | ishora bilan | — |
| lw | 2 | 4 bayt | kerak emas | — |
| lbu | 4 | 1 bayt | nol bilan | `0x00000080` (128) |
| lhu | 5 | 2 bayt | nol bilan | — |

C'da `char c = ...; int x = c;` yozganingizda kompilyator aynan `lb` (yoki `lbu` — `char` ishoralimi,
platformaga bog'liq, 1.8) ishlatadi. `uint8_t` → `lbu`, `int8_t` → `lb`.

Diqqat: funksiyaga `qiymat` ning **yuqori** bitlarida "axlat" kelishi mumkin — test buni tekshiradi
(`lb faqat past bayt`: `0x12345680` → `0xFFFFFF80`). Avval maska, keyin kengaytirish.

## 3.4. E4 — M kengaytmasi: ko'paytirish va bo'lish

| funct3 | buyruq | natija |
|---|---|---|
| 0 | mul | a × b ning **past** 32 biti |
| 1 | mulh | a × b ning **yuqori** 32 biti; a, b — ishorali |
| 2 | mulhsu | yuqori 32 bit; a — ishorali, b — **ishorasiz** |
| 3 | mulhu | yuqori 32 bit; ikkalasi ishorasiz |
| 4 | div | a / b, ishorali, **nolga qarab** yaxlitlanadi |
| 5 | divu | a / b, ishorasiz |
| 6 | rem | a % b, ishorali (ishora — **bo'linuvchiniki**) |
| 7 | remu | a % b, ishorasiz |

### Yuqori 32 bit qayerdan?

32 × 32 bitli ko'paytma — **64 bit**gacha. `mul` past yarmini beradi; `mulh*` yuqori yarmini. Kompilyator
ularni `int64_t` ko'paytirishda **juftlikda** ishlatadi. Emulyatorda: 64 bitli turda ko'paytirib, `>> 32`.

**Asosiy qiyinlik — ishorani to'g'ri kengaytirish.** `mulhsu` da a ishorali: `(int64_t)(int32_t)a` — avval
`int32_t` (32 bitli ishorali talqin), keyin 64 bitga (ishora kengayadi). b ishorasiz: `(int64_t)(uint64_t)b` —
nol bilan kengayadi. Qadamlarni almashtirsangiz (`(int64_t)a` — a `uint32_t` bo'lsa) ishora **yo'qoladi**.

Qo'lda tekshiramiz: `mulh(0x80000000, 0x80000000)` = ?

```text
0x80000000 ishorali = -2^31
(-2^31) × (-2^31) = 2^62 = 0x4000_0000_0000_0000
yuqori 32 bit = 0x40000000           (test: "mulh (-2^31)^2")
```

`mulhsu(-1, 0xFFFFFFFF)`: `-1 × 4294967295 = -4294967295 = 0xFFFFFFFF_00000001` (64 bitli ikkiga to'ldirish)
→ yuqori yarmi `0xFFFFFFFF`.

### Nolga bo'lish: RISC-V'da istisno YO'Q

x86 da nolga bo'lish — istisno (#DE), Linux uni `SIGFPE` ga aylantiradi. RISC-V dizaynerlari boshqacha
qaror qildi: natija **aniqlangan**, istisno yo'q (agar dastur tekshirmoqchi bo'lsa — bitta `beqz` yetarli).

| amal | b = 0 | a = INT_MIN, b = -1 (toshish) |
|---|---|---|
| div | -1 (`0xFFFFFFFF`) | INT_MIN (`0x80000000`) |
| divu | `0xFFFFFFFF` (max) | — |
| rem | a | 0 |
| remu | a | — |

Bu qiymatlar tasodifiy emas: ular "bo'linma × b + qoldiq = a" tengligini saqlaydi (qo'lda tekshiring:
`div(x,0) = -1`, `rem(x,0) = x` → `(-1)·0 + x = x` ✓).

> **Diqqat — emulyator ichidagi xavf:** bu qoidalarni bajarish uchun C'ning `/` operatorini ishlatamiz. Lekin
> C'da `x / 0` va `INT_MIN / -1` — **UB**, x86 da emulyatorning o'zi `SIGFPE` bilan **qulaydi**! Ya'ni
> yomon niyatli (yoki shunchaki xato) RISC-V dastur bitta `div` buyrug'i bilan **emulyatorni** o'ldira
> oladi. Bu — haqiqiy xavfsizlik zaifligi turi (emulyatordan "qochish"ning birinchi qadami). Shuning uchun
> ikkala maxsus holatni `/` dan **oldin** tekshirish shart.

Ishonmasangiz, sinab ko'ring:

```console
$ cat > qulash.c <<'E'
#include <stdint.h>
#include <stdio.h>
int main(int argc, char **argv) { (void)argv; int32_t a = INT32_MIN, b = -argc; printf("%d\n", a / b); }
E
$ gcc qulash.c -o qulash && ./qulash
Floating point exception (core dumped)
```

(`b = -argc` — kompilyator natijani oldindan hisoblab qo'ymasligi uchun; `argc = 1`.)

### Yaxlitlash: nolga qarab

C99 dan beri `/` — nolga qarab yaxlitlaydi: `-7 / 2 = -3` (Python'da `-7 // 2 = -4`!). `%` ishorasi —
bo'linuvchiniki: `-7 % 2 = -1` (Python'da `1`). RISC-V `div`/`rem` aynan C kabi. Python'dan kelganlar uchun
bu klassik tuzoq — test `div -7/2` va `rem -7%2` holatlarini tekshiradi.

## 3.5. Tekshirish

```console
$ make test 2>&1 | grep -E "E2|E3|E4"
  [ OK ] E2 shart_bajarildimi (alu.c): 10/10
  [ OK ] E3 yuklash_kengaytir (alu.c): 8/8
  [ OK ] E4 m_amal: mul/div/rem (alu.c): 15/15
```

E1–E4 tayyor bo'lgach, assembly testlari: `alu`, `sakrash`, `xotira`, `mul` o'tishi kerak. Ular haqiqiy
RISC-V dasturlar ([testlar/emu/alu.S](../testlar/emu/alu.S) va boshqalar). Har tekshiruv
`SINA(raqam, registr, kutilgan)` makrosi ([testlar/emu/test_makro.h](../testlar/emu/test_makro.h)): raqam `gp`
registriga yoziladi; natija kutilganidan farq qilsa, emulyator shu raqam bilan chiqadi:

```console
$ sh testlar/emu/yigish.sh $PWD/build/vk
  [ OK ] alu
  [ OK ] sakrash
  [XATO] xotira: 7-tekshiruv buzildi
```

"7-tekshiruv" — `xotira.S` faylida `SINA(7, ...)` qatorini qidiring: `lbu a1, 0(a2)` — `sw` bilan yozilgan
`0x11223344` ning past bayti `0x44` bo'lishi kerak (little-endian). Keyin `-t` bilan o'sha joyni kuzating.

## Savol-javob

**Savol:** `slt` va `blt` — bir xil solishtirish. Nega ikkita buyruq?
**Javob:** `blt` — boshqaruv (pc ni o'zgartiradi), `slt` — qiymat (registrga 0/1 yozadi). C'dagi
`x = (a < b);` — `slt`; `if (a < b) ...` — `blt`. Emulyatorda ikkalasi ham bitta C ifodasiga tushadi, lekin
apparatda ular turli yo'llardan (biri pc ga, biri registr fayliga) o'tadi.

**Savol:** `mulh` ni 64 bitli turlarsiz qilish mumkinmi?
**Javob:** Ha — maktabdagi "ustun bo'yicha ko'paytirish": har sonni 16 bitli yarmlarga bo'lib, 4 ta qisman
ko'paytma va o'tkazmalar (carry). 64 bitli protsessorlarda `mulh` (128 bitli natija) uchun aynan shu usul
kerak bo'ladi. Qiziqarli qo'shimcha mashq.
