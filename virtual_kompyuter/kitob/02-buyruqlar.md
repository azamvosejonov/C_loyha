# 2-bob. RISC-V buyruqlari va ularni dekodlash

> **Bu bobda nima o'rganasiz:** protsessorning "dasturchi ko'radigan" qismi (registrlar, pc, xotira);
> RISC-V buyruq formatlari (R, I, S, B, U, J); opcode jadvali; buyruqdan maydonlar va o'zgarmaslarni
> (immediate) ajratish; kutilgan qiymatlarni assembler yordamida o'zingiz olish.
> **Oldindan nima kerak:** 1-bob. Darslikning 17-bobi (assembly) foydali.
> **Mashqlar:** E1a–E1d ([emu/dekod.c](../emu/dekod.c)).   **Vaqt:** 3–4 soat.

## Bu bob nima haqida?

Protsessor — juda oddiy mashina. U bitta ishni qayta-qayta qiladi: **pc** (program counter) ko'rsatgan
manzildan 32 bitli sonni o'qiydi, uni **buyruq** sifatida tushunadi, bajaradi va keyingisiga o'tadi.
Bu bobda "tushunish" qismini — **dekodlash**ni yozamiz: 32 bitli sondan "bu `addi`, natija `a0` ga,
manba `a0`, o'zgarmas -1" degan ma'lumotni ajratib olish.

**Hayotdan misol: pochta indeksi.** 100011 — Toshkent, Yunusobod tumani, ma'lum bo'lim. Birinchi raqamlar
viloyatni, keyingilari tumanni bildiradi. Pochtachi butun raqamni "o'qimaydi" — **joyi bo'yicha** qismlarni
ajratadi. Protsessor dekoderi ham shunday: buyruqning qaysi bitlari nimani bildirishini **format** aytadi.

## 2.1. Protsessorning dasturchi ko'radigan holati

RV32I protsessorida dastur ko'radigan narsa juda kam:

| Narsa | Hajmi | Izoh |
|---|---|---|
| `x0` .. `x31` | 32 ta × 32 bit | umumiy registrlar. **`x0` doim 0** — unga yozish e'tiborsiz |
| `pc` | 32 bit | joriy buyruq manzili |
| xotira | 2³² bayt | bayt bo'yicha manzillanadi, little-endian |
| CSR'lar | 4096 ta (raqam) | boshqaruv registrlari — 6-bob |

Registrlarning **ABI nomlari** bor (kompilyator kelishuvi — qaysi registr nimaga ishlatiladi). Trace'da shu
nomlarni ko'rasiz ([emu/disasm.c](../emu/disasm.c) dagi `registr_nomi`):

| Registr | ABI nomi | Vazifasi |
|---|---|---|
| x0 | zero | doim 0 |
| x1 | ra | qaytish manzili (return address) |
| x2 | sp | stek ko'rsatkichi |
| x3, x4 | gp, tp | global va oqim (thread) ko'rsatkichi |
| x5–x7, x28–x31 | t0–t6 | vaqtinchalik (chaqirilgan funksiya buzishi mumkin) |
| x8, x9, x18–x27 | s0–s11 | saqlanadigan (chaqirilgan funksiya tiklashi shart) |
| x10–x17 | a0–a7 | argumentlar va natija (a0, a1). **a7 — tizim chaqiruvi raqami** |

Emulyatorda bu holat — oddiy `struct` ([emu/cpu.h](../emu/cpu.h)):

```c
struct cpu {
    uint32_t x[32];                             /* umumiy registrlar; x[0] doim 0 */
    uint32_t pc;
    enum rejim rejim;                           /* U, S yoki M (6-bob) */
    /* ... CSR maydonlari, TLB ... */
};
```

`x0` ni qanday "doim nol" qilamiz? Ikki usul bor va bizda ikkalasi ham: `rd_yoz()` `rd == 0` bo'lsa
yozmaydi, `cpu_qadam()` esa har buyruqdan keyin `c->x[0] = 0` qiladi (ehtiyot uchun). Nega x0 kerak?
Chunki u ko'p buyruqni "tekin" beradi: `mv a0, a1` aslida `addi a0, a1, 0`; `nop` — `addi x0, x0, 0`;
`j label` — `jal x0, label` (qaytish manzilini "tashlab yuborish").

## 2.2. Oltita format

Har bir RV32 buyrug'i — **aniq 32 bit**. Pastki 7 bit — **opcode** (buyruq oilasi). Qolgan maydonlar
formatga bog'liq ([emu/dekod.h](../emu/dekod.h) dagi chizma):

```text
 31        25 24    20 19    15 14  12 11         7 6       0
+------------+--------+--------+------+------------+---------+
|   funct7   |  rs2   |  rs1   |funct3|     rd     | opcode  |  R: add, sub, mul ...
+------------+--------+--------+------+------------+---------+
|       imm[11:0]     |  rs1   |funct3|     rd     | opcode  |  I: addi, lw, jalr, csrrw ...
+------------+--------+--------+------+------------+---------+
| imm[11:5]  |  rs2   |  rs1   |funct3|  imm[4:0]  | opcode  |  S: sw, sh, sb
+------------+--------+--------+------+------------+---------+
|im[12|10:5] |  rs2   |  rs1   |funct3|imm[4:1|11] | opcode  |  B: beq, bne, blt ...
+------------+--------+--------+------+------------+---------+
|              imm[31:12]             |     rd     | opcode  |  U: lui, auipc
+-------------------------------------+------------+---------+
|        imm[20|10:1|11|19:12]        |     rd     | opcode  |  J: jal
+-------------------------------------+------------+---------+
```

Maydonlar uchun emulyatorda bir qatorli funksiyalar:

```c
static inline uint32_t d_opcode(uint32_t b) { return BITLAR(b, 6, 0); }
static inline uint32_t d_rd(uint32_t b) { return BITLAR(b, 11, 7); }
static inline uint32_t d_funct3(uint32_t b) { return BITLAR(b, 14, 12); }
static inline uint32_t d_rs1(uint32_t b) { return BITLAR(b, 19, 15); }
static inline uint32_t d_rs2(uint32_t b) { return BITLAR(b, 24, 20); }
static inline uint32_t d_funct7(uint32_t b) { return BITLAR(b, 31, 25); }
```

**Opcode jadvali** (bizga kerakli qismi; `cpu.c` dagi `bajar()` shu bo'yicha `switch` qiladi):

| opcode | Nomi | Format | Buyruqlar |
|---|---|---|---|
| 0x37 | LUI | U | `lui` |
| 0x17 | AUIPC | U | `auipc` |
| 0x6F | JAL | J | `jal` |
| 0x67 | JALR | I | `jalr` |
| 0x63 | BRANCH | B | `beq bne blt bge bltu bgeu` |
| 0x03 | LOAD | I | `lb lh lw lbu lhu` |
| 0x23 | STORE | S | `sb sh sw` |
| 0x13 | OP-IMM | I | `addi slti sltiu xori ori andi slli srli srai` |
| 0x33 | OP | R | `add sub sll slt sltu xor srl sra or and` + M: `mul ... remu` |
| 0x0F | MISC-MEM | I | `fence fence.i` |
| 0x2F | AMO | R | `lr.w sc.w amoswap.w ...` (4-bob) |
| 0x73 | SYSTEM | I | `ecall ebreak mret sret wfi sfence.vma csrrw ...` (6-bob) |

Opcode'ning past 2 biti doim `11` (binar). Nega? Chunki `00`, `01`, `10` — **16 bitli** siqilgan buyruqlar
uchun band qilingan (5-bob). Protsessor birinchi 2 bitga qarab buyruq uzunligini biladi.

## 2.3. O'zgarmaslar (immediate) — eng qiziq qism

Buyruq ichidagi son — **o'zgarmas**. Hammasi (U dan tashqari) **ishorali**: 12 bitli `addi` o'zgarmasi
-2048..2047 oralig'ida. Formatlar bo'yicha:

| Tur | Bitlar | Diapazon | Qo'llanishi |
|---|---|---|---|
| I | 12 bit, 31..20 | -2048 .. 2047 | `addi`, `lw` siljishi, `jalr` |
| S | 12 bit, ikki bo'lakda | -2048 .. 2047 | `sw` siljishi |
| B | 13 bit (0-bit doim 0), aralash | -4096 .. 4094 (juft) | shartli sakrash |
| U | yuqori 20 bit, pastki 12 = 0 | | `lui`, `auipc` |
| J | 21 bit (0-bit doim 0), aralash | ±1 MB (juft) | `jal` |

### B va J tur — nega bitlar aralash?

Bu RISC-V'ning eng "g'alati" ko'rinadigan, lekin eng aqlli qarori. Ikki qoidaga amal qilingan:

1. **Ishora biti har doim 31-bitda.** Apparat ishora kengaytirishni formatni bilmasdan **oldindan**
   boshlay oladi.
2. **Bir xil ma'noli bitlar iloji boricha bir xil joyda.** Masalan `imm[10:5]` S va B turda bir xil joyda
   (30..25), `imm[4:1]` ham (11..8). B tur S turdan faqat ikki bit bilan farq qiladi: S'ning `imm[0]` joyida
   (7-bit) B'da `imm[11]` turadi (B'da `imm[0]` doim 0 — yozishga hojat yo'q), S'ning `imm[11]` joyida
   (31-bit) esa `imm[12]`.

Natijada **apparatda** (simlar bilan) multipleksorlar kamroq kerak — mikrosxema kichikroq va tezroq.
**Dasturda** esa bu — bit jumbog'i: bo'laklarni to'g'ri joyiga yig'ish kerak. Bu sizning mashqingiz.

### Qo'lda dekodlash: `beq a0, a1, -4`

Assembler bu buyruqni `0xFEB50EE3` ga aylantiradi. Keling, o'zgarmasni **qo'lda** chiqaraylik:

```text
0xFEB50EE3 = 1111 1110 1011 0101 0000 1110 1110 0011
             ^^^^ ^^^                                     31..25 = 1111111   -> imm[12] = 1 (31-bit), imm[10:5] = 111111
                                         ^^^^ ^           11..7  = 11101     -> imm[4:1] = 1110 (11..8), imm[11] = 1 (7-bit)
```

Yig'amiz (imm[0] = 0):

```text
imm[12]=1 imm[11]=1 imm[10:5]=111111 imm[4:1]=1110 imm[0]=0
 -> 1 1 111111 1110 0  = 13 bit: 1_1111_1111_1100
 -> ishora bilan 32 bitga: 0xFFFFFFFC = -4   ✓
```

Shu jarayonni kod bilan qilish — E1c mashqi. **Maslahat:** kodni yozishdan oldin bitta misolni shunday qog'ozda
bajaring. Keyin kodingizdagi har `<<` ni qog'ozdagi chizma bilan solishtiring.

## 2.4. Test vektorlarini o'zingiz oling

To'g'ri javobni qayerdan bilamiz? **Assembler**dan — u spetsifikatsiyaga aniq amal qiladi. Har qanday
buyruqning kodini olish:

```console
$ cat > enc.S <<'E'
.option norvc
add a2, a0, a1
addi a0, a0, -1
lw a0, 8(sp)
sw a1, 8(sp)
beq a0, a1, .-4
lui a0, 0x12345
jal ra, .+2048
E
$ clang --target=riscv32 -march=rv32im -mno-relax -c enc.S -o enc.o
$ llvm-objdump -d -M no-aliases enc.o
       0: 33 06 b5 00  	add	a2, a0, a1
       4: 13 05 f5 ff  	addi	a0, a0, -0x1
       8: 03 25 81 00  	lw	a0, 0x8(sp)
       c: 23 24 b1 00  	sw	a1, 0x8(sp)
      10: e3 0e b5 fe  	beq	a0, a1, 0xc <.text+0xc>
      14: 37 55 34 12  	lui	a0, 0x12345
      18: ef 00 10 00  	jal	ra, 0x818 <.text+0x818>
```

Baytlar little-endian tartibda chiqadi: `e3 0e b5 fe` → son `0xFEB50EE3`. `.-4` — "shu buyruqdan 4 bayt
oldinga" (nisbiy manzil). `-mno-relax` va `.` (joriy manzil) bilan assembler siljishni **darhol** hisoblaydi.

> **Diqqat — haqiqiy xato hikoyasi:** muallif birinchi marta siqilgan sakrashlar uchun vektorlarni tayyorlaganda
> `c.j label` ko'rinishidagi buyruqlarni yig'ib, faqat `.o` faylga qaradi. `.o` faylda manzil hali **noma'lum**
> — assembler o'zgarmas o'rniga 0 qo'yib, "relokatsiya" yozuvini qoldiradi (bog'lovchi keyin to'ldiradi,
> darslik 22-bob). Natijada test "c.j 1000 → 0x0000006f" ni kutgan — ya'ni noto'g'ri yechimni ham "to'g'ri"
> deb qabul qilgan! Xato faqat bo'sh (stub) versiya ham testdan o'tib ketganda ma'lum bo'ldi. Saboq:
> **testingizni ham tekshiring** — u noto'g'ri kodni rad etayotganiga ishonch hosil qiling.

## 2.5. E1 mashqi

[emu/dekod.c](../emu/dekod.c) da to'rtta funksiya bo'sh: `imm_i` (E1a), `imm_s` (E1b), `imm_b` (E1c),
`imm_j` (E1d). `imm_u` tayyor — namuna sifatida:

```c
uint32_t imm_u(uint32_t b)
{
    return b & 0xFFFFF000u;                     /* yuqori 20 bit o'z joyida, pastki 12 bit — nol */
}
```

Har funksiyaning izohida qadamlar bor. Tartib: I → S → B → J (osondan qiyinga).

**Tekshirish:**

```console
$ make test 2>&1 | grep -A6 "E1 "
  [ OK ] E1 imm_i/s/b/u/j (dekod.c): 14/14
```

Test chegaraviy holatlarni ham tekshiradi: eng katta musbat (`2047`, `+4094`, `+1048574`) va eng kichik manfiy
(`-2048`, `-4096`, `-1 MB`) qiymatlar. Ko'pincha xato aynan chegarada: masalan ishora bitini olishni
unutsangiz, `-4` to'g'ri chiqishi mumkin (agar kodingiz tasodifan uni boshqa yo'l bilan tiklasa), lekin `-4096`
— yo'q.

**E1 ishlagach**, butun protsessorning assembly testlaridan birinchilari o'ta boshlaydi (ular ALU'ga ham
bog'liq — 3-bob). Trace bilan o'z dekoderingizni kuzating:

```console
$ ./build/vk -S -t -n 6 testlar/emu/build/alu.elf
[S] 80000000: 00500513  addi a0, zero, 5
[S] 80000004: 00700593  addi a1, zero, 7
[S] 80000008: 00b50633  add a2, a0, a1
[S] 8000000c: 00100193  addi gp, zero, 1
[S] 80000010: 00c00f93  addi t6, zero, 12
[S] 80000014: 1ff61663  bne a2, t6, 0x80000200
```

Oxirgi qatorda sakrash manzilini (`0x80000200`) disassembler **sizning** `imm_b` funksiyangiz bilan
hisoblaydi. U noto'g'ri bo'lsa — trace'da ham noto'g'ri manzil ko'rinadi.

## 2.6. Disassembler — dekoderning ikkinchi iste'molchisi

[emu/disasm.c](../emu/disasm.c) — buyruqni matnga aylantiradi. Uni o'qing: u aynan siz yozgan `imm_*` va
`d_*` funksiyalaridan foydalanadi va `bajar()` dagi `switch` ning "oynadagi aksi". Ikki xil iste'molchi bitta
dekoderdan foydalanishi — yaxshi dizayn: xato bitta joyda, tuzatish ham bitta joyda.

## Savol-javob

**Savol:** Nega `lui` + `addi` bilan 32 bitli son yuklaganda ba'zan `lui` qiymati "bittaga ko'p" bo'ladi?
**Javob:** `addi` o'zgarmasi **ishorali**. `0x12345FFF` ni yuklash: `addi` ga `0xFFF` = **-1** tushadi, shuning
uchun `lui` ga `0x12346` (bittaga ko'p) yoziladi: `0x12346000 + (-1) = 0x12345FFF`. Assembler buni `%hi/%lo`
bilan avtomatik qiladi. `./build/vk -S -t -n 12 testlar/emu/build/alu.elf` trace'ining oxirida
`lui a3, 0x80000` + `addi a3, a3, -1` = `0x7FFFFFFF` juftligini ko'ring.

**Savol:** Buyruq 32 bit bo'lsa, 32 bitli sonni bitta buyruqda yuklab bo'lmaydimi?
**Javob:** Yo'q — buyruqda opcode va registr uchun ham joy kerak. Shuning uchun ikki buyruq (`lui` + `addi`)
yoki xotiradan yuklash. Bu RISC ("kamaytirilgan buyruqlar to'plami") falsafasi: har buyruq oddiy va bir xil
uzunlikda — dekoder sodda va tez.
