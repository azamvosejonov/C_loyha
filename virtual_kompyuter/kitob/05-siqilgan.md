# 5-bob. C kengaytmasi: 16 bitli buyruqlar

> **Bu bobda nima o'rganasiz:** nega 16 bitli buyruqlar kerak (kod zichligi va kesh); "kvadrant"lar; siqilgan
> registrlar (`rd'`); siqilgan buyruqni 32 bitli ekvivalentiga **kengaytirish** usuli; teskari dekoder —
> buyruq **quruvchi** funksiyalar; taqiqlangan kodlar.
> **Oldindan nima kerak:** 1, 2, 4-boblar.
> **Mashqlar:** E7a, E7b ([emu/siqilgan.c](../emu/siqilgan.c)).   **Vaqt:** 3–4 soat.

## Bu bob nima haqida?

Linux yadrosi va deyarli hamma RISC-V dasturlar **C kengaytmasi** bilan yig'iladi: eng ko'p ishlatiladigan
buyruqlarning 16 bitli "qisqa" shakli bor. Bizning firmware'ning SBI qismi (`firmware/sbi.c`) uchun:

```console
$ llvm-size -A sbi_rv32ima.o  | grep '^.text'      # C'siz
.text                 1024      0
$ llvm-size -A sbi_rv32imac.o | grep '^.text'      # C bilan
.text                  774      0
```

**24% kichikroq**. Bu nafaqat disk joyi: kod kichik bo'lsa, **instruksiya keshiga** ko'proq sig'adi
(darslik 21-bob) — protsessor tezroq. Mikrokontrollerlarda esa flash xotira narxi bevosita pul.

Linux distributivlari va standart RISC-V profillari (RVA, RVB) C kengaytmasini **majburiy** deb hisoblaydi;
bizning yadro ham u bilan yig'iladi (`linux/vk.config` da `CONFIG_RISCV_ISA_C=y`). Shuning uchun emulyator uni
to'liq qo'llaydi.

**Hayotdan misol: SMS qisqartmalari.** "Assalomu alaykum" o'rniga "Ass.", "rahmat" o'rniga "rhmt". Faqat
**eng ko'p ishlatiladigan** so'zlarning qisqartmasi bor va o'quvchi ularni xayolan to'liq so'zga
"kengaytiradi". Protsessor ham shunday: `c.addi a0, 1` ni ko'rib, uni xayolan `addi a0, a0, 1` deb tushunadi.

## 5.1. Kengaytirish — eng oddiy va eng to'g'ri yo'l

Har bir siqilgan buyruqning **aniq bitta** 32 bitli ekvivalenti bor. Spetsifikatsiya ularni shunday
ta'riflaydi: "c.addi rd, imm **expands to** addi rd, rd, imm". Demak emulyatorga yangi "bajaruvchi" kerak
emas — faqat **tarjimon**:

```c
/* 3) DECODE: siqilgan buyruq bo'lsa — 32 bitliga aylantiramiz */
uint32_t b32 = b;
if (uz == 2) {
    b32 = c_kengaytir((uint16_t)b);
    if (b32 == 0) {
        trap_kirish(c, SABAB_NOTOGRI_BUYRUQ, b);
        return;
    }
}
```

Keyin `bajar(m, b32, uz, ...)` odatdagidek ishlaydi. Faqat bitta farq bor: `uz` (buyruq uzunligi). `c.jal`
ning qaytish manzili `pc + 2`, `jal` niki esa `pc + 4`. Shuning uchun `bajar()` `uz` ni ham oladi:
`rd_yoz(c, rd, c->pc + uz)`. Haqiqiy apparat dekoderlari ham aynan shu usulda ishlaydi — 16 bitli buyruqni
dekoder boshida kengaytirib, qolgan konveyerni o'zgartirmaydi.

## 5.2. Kvadrantlar va maydonlar

16 bitli buyruqning past 2 biti — **kvadrant** (`11` — 32 bitli buyruq):

| kvadrant | past 2 bit | asosiy buyruqlar |
|---|---|---|
| 0 | `00` | `c.addi4spn`, `c.lw`, `c.sw` (stekdagi va struct maydonlariga murojaat) |
| 1 | `01` | `c.addi`, `c.li`, `c.lui`, `c.addi16sp`, arifmetika, `c.j`, `c.jal`, `c.beqz`, `c.bnez` |
| 2 | `10` | `c.slli`, `c.lwsp`, `c.swsp`, `c.jr`, `c.jalr`, `c.mv`, `c.add`, `c.ebreak` |

Har kvadrant ichida 15..13-bitlar (`funct3`) buyruqni tanlaydi. Registr maydonlari ikki xil:

- **To'liq** (5 bit, 11..7 yoki 6..2) — `c.addi`, `c.mv`, `c.lwsp` kabi buyruqlarda istalgan registr.
- **Siqilgan** (3 bit, `rd'`/`rs1'`/`rs2'`) — faqat **x8..x15** (`s0, s1, a0..a5`). Haqiqiy raqam = 8 + maydon.
  Nega aynan bular? Statistika: kompilyator chiqaradigan kodda eng ko'p ishlatiladigan 8 ta registr.

```c
uint32_t rd = BITLAR(c, 11, 7), rs2 = BITLAR(c, 6, 2);              /* to'liq (5 bitli) registr maydonlari */
uint32_t rdq = 8 + BITLAR(c, 4, 2), rs1q = 8 + BITLAR(c, 9, 7);     /* siqilgan registrlar: x8..x15 */
```

## 5.3. Buyruq quruvchilar — dekoderning teskarisi

2-bobda 32 bitli buyruqdan maydonlarni **ajratdik**. Bu yerda teskari ish: maydonlardan 32 bitli buyruqni
**yig'amiz**. Har format uchun kichik funksiya:

```c
static uint32_t i_tur(uint32_t imm, uint32_t rs1, uint32_t f3, uint32_t rd, uint32_t op)
{
    return (imm & 0xFFFu) << 20 | rs1 << 15 | f3 << 12 | rd << 7 | op;
}

static uint32_t b_tur(uint32_t imm, uint32_t rs2, uint32_t rs1, uint32_t f3)
{
    return BIT(imm, 12) << 31 | BITLAR(imm, 10, 5) << 25 | rs2 << 20 | rs1 << 15 | f3 << 12 |
           BITLAR(imm, 4, 1) << 8 | BIT(imm, 11) << 7 | 0x63u;
}
```

`b_tur` ni 2-bobdagi B-format chizmasi bilan solishtiring: `imm_b` (E1c) bo'laklarni **yig'adi**, `b_tur`
esa ularni **tarqatadi**. Agar ikkalasi to'g'ri bo'lsa, `imm_b(b_tur(x, ...)) == x` (juft x uchun, ±4 KB
ichida). Bu — ajoyib **xossa testi** (property test): bir funksiya ikkinchisini tekshiradi. Mustaqil mashq
sifatida yozib ko'ring.

Shunda, masalan, `c.beqz` ning kengaytmasi bitta qator:

```c
case 6:                                 /* c.beqz rs1', offset  ->  beq rs1', x0, offset */
    return b_tur(imm_cb(c), 0, rs1q, 0);
```

## 5.4. O'zgarmaslar — yana ham aralash

16 bitda joy juda kam, shuning uchun o'zgarmas bitlari yanada "chalkash" joylashgan. Spetsifikatsiya ularni
qavs ichidagi ro'yxat bilan yozadi, masalan `c.lw`:

```text
 15  13 12      10 9    7 6   5 4    2 1  0
+------+----------+-------+-----+-------+----+
| 010  | uimm[5:3]| rs1'  |uimm |  rd'  | 00 |      uimm[2|6] -> 6..5-bitlar: 6-bit = uimm[2], 5-bit = uimm[6]
+------+----------+-------+[2|6]+-------+----+
```

O'qish qoidasi: **qavs ichidagi ro'yxat chapdan o'ngga — bit o'rinlari yuqoridan pastga**. `uimm[2|6]`
6..5-bitlar ustida: 6-bit → `uimm[2]`, 5-bit → `uimm[6]`. Kod:

```c
/* c.lw / c.sw: uimm[5:3] -> 12..10, uimm[2|6] -> 6..5 */
static uint32_t imm_lw(uint32_t c)
{
    return BITLAR(c, 12, 10) << 3 | BIT(c, 6) << 2 | BIT(c, 5) << 6;
}
```

E'tibor bering: `c.lw` siljishi **ishorasiz** va 4 ga karrali (past 2 bit doim 0 — yozilmaydi). Shuning uchun
`c.lw` 0..124 bayt oralig'ida ishlaydi — struct maydonlari uchun yetarli.

## 5.5. E7 — `c.j`/`c.jal` va `c.beqz`/`c.bnez` siljishlari

Ikkita eng chalkash o'zgarmas sizga qoldirilgan:

**E7a — `imm_cj`** (`c.j`, `c.jal`): `offset[11|4|9:8|10|6|7|3:1|5]` → 12..2-bitlar. Ya'ni:

| buyruq biti | 12 | 11 | 10..9 | 8 | 7 | 6 | 5..3 | 2 |
|---|---|---|---|---|---|---|---|---|
| offset biti | 11 | 4 | 9:8 | 10 | 6 | 7 | 3:1 | 5 |

Natija 12 bitli ishorali son (±2 KB).

**E7b — `imm_cb`** (`c.beqz`, `c.bnez`): `offset[8|4:3]` → 12..10, `offset[7:6|2:1|5]` → 6..2:

| buyruq biti | 12 | 11..10 | 6..5 | 4..3 | 2 |
|---|---|---|---|---|---|
| offset biti | 8 | 4:3 | 7:6 | 2:1 | 5 |

Natija 9 bitli ishorali son (±256 bayt).

**Qo'lda bitta misol (E7b):** `c.bnez a5, +100` = `0xE3B5`.

```text
0xE3B5 = 1110 0011 1011 0101
          bit 15..13 = 111 (funct3=7: c.bnez)   bit 1..0 = 01 (kvadrant 1)
          bit 12     = 0          -> offset[8]   = 0
          bit 11..10 = 00         -> offset[4:3] = 00
          bit 9..7   = 111        -> rs1' = 7 -> x15 = a5  ✓
          bit 6..5   = 01         -> offset[7:6] = 01
          bit 4..3   = 10         -> offset[2:1] = 10
          bit 2      = 1          -> offset[5]   = 1
offset = 0 01 1 00 10 0 (8..0) = 0b001100100 = 100  ✓
```

Endi siz shuni kodga aylantirasiz. 32 bitli natija `bne a5, x0, 100` = `0x06079263` bo'lishi kerak.

**Tekshirish:** birlik testi 56 ta juftlikni tekshiradi ([testlar/birlik/c_juftlar.h](../testlar/birlik/c_juftlar.h)
— hammasi clang assembler'idan olingan) va 5 ta taqiqlangan kodni:

```console
$ make test 2>&1 | grep -B3 "E7 "
        c_kengaytir(c_juftlar[i].c)                    c.j 1000                     -> olindi 0x0000006f, kutilgan 0x3e80006f
  [XATO] E7 c_kengaytir: 16 bitli buyruqlar (siqilgan.c): 44/56
```

`olindi 0x0000006f` — `jal x0, 0`: opcode va rd to'g'ri, siljish nol. Demak `imm_cj` hali 0 qaytaryapti.

## 5.6. Taqiqlangan kodlar — "noto'g'ri buyruq" ham to'g'ri javob

`c_kengaytir` 0 qaytarsa — "bunday buyruq yo'q" (32 bitli 0 ham haqiqiy buyruq emas, shuning uchun 0 xavfsiz
belgi). Qaysilar taqiqlangan?

| kod | nega |
|---|---|
| `0x0000` | ataylab: nollangan xotiraga sakrash darhol ushlanadi |
| `c.addi4spn` imm = 0 | spetsifikatsiyada "reserved" |
| `c.lwsp rd = x0` | reserved |
| `c.jr x0` | reserved |
| `c.addi16sp` imm = 0 | reserved |
| `c.lui` imm = 0 | reserved |
| `c.slli`/`c.srli`/`c.srai` 12-bit = 1 | RV32 da 32+ siljish yo'q (RV64 uchun) |
| `c.subw`, `c.addw` | faqat RV64 |

Nega taqiqlangan kodlarni aniq rad etish kerak? Kelajakda spetsifikatsiya ularga **yangi** ma'no berishi
mumkin. Agar eski protsessor ularni "nimadir" sifatida bajarsa, yangi dastur eski protsessorda **jimgina noto'g'ri**
ishlaydi. "Noto'g'ri buyruq" istisnosi esa yadroga "bu protsessor buni bilmaydi" deb aytadi — yadro uni
dasturiy taqlid qilishi (emulate) mumkin. Shuning uchun `0x0000` ham "noto'g'ri".

## 5.7. Disassembler

Trace'da siqilgan buyruqlar 4 xonali hex va `c.` prefiksi bilan chiqadi:

```text
[M] 80000004: 2011      c.jal ra, 0x80000008
```

[emu/disasm.c](../emu/disasm.c) siqilgan buyruqni ham avval `c_kengaytir` bilan kengaytiradi, keyin oddiy
disassembler bilan matnga aylantiradi va oldiga `c.` qo'yadi. Ya'ni trace'dagi manzil (`0x80000008`) —
**sizning** `imm_cj` natijangiz. Shunchaki `-t` bilan ishlatib, xatoni ko'z bilan ham topish mumkin.

## Savol-javob

**Savol:** 16 bitli buyruq 4 ga tekis bo'lmagan manzilda bo'lishi mumkinmi?
**Javob:** Ha — C kengaytmasi bilan buyruqlar **2 ga** tekis bo'lsa yetarli. Shuning uchun `sakrash_tekis()`
faqat 0-bitni tekshiradi va 32 bitli buyruq 2 ga tekis (lekin 4 ga emas) manzilda turishi mumkin — fetch
16+16 qilib o'qiydi (4.3).

**Savol:** Nega `c.jal` faqat RV32 da?
**Javob:** RV64 da shu kod `c.addiw` ga berilgan (64 bitli protsessorda 32 bitli qo'shish juda ko'p
ishlatiladi). Bir xil bitlar — arxitekturaga qarab boshqa ma'no. Emulyator yozganda spetsifikatsiyaning
**RV32** ustunini o'qing.
