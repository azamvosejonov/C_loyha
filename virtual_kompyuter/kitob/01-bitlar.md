# 1-bob. Bitlar bilan gaplashish

> **Bu bobda nima o'rganasiz:** emulyator yozishning "alifbosi" — sondan kerakli bitlarni ajratish, bitlarni
> joyiga qo'yish, maska bilan yozish, ishorali va ishorasiz sonlar farqi, ishora bilan kengaytirish, baytlar
> tartibi (endianness) va C'dagi bit amallarining xavfli burchaklari.
> **Oldindan nima kerak:** darslikning 2-bobi (turlar), 3-bobi (bit operatorlari), 16-bobi (bitlar va apparat).
> **Namoyish:** `make bitlar` ([misollar/bitlar.c](../misollar/bitlar.c)).   **Vaqt:** 2–3 soat.

## Bu bob nima haqida?

Protsessor uchun **hamma narsa son**: buyruq — 32 bitli son, registr — 32 bitli son, sahifa jadvali
yozuvi — 32 bitli son. Lekin bu sonlarning ichida **ma'no** bor: "11..7-bitlar — qaysi registrga yozish",
"0-bit — sahifa yaroqlimi". Emulyator kodining yarmi — mana shu ma'nolarni **ajratib olish** va **joyiga
qo'yish**. Bu bobda shu ish uchun 5 ta "andoza"ni o'rganamiz; keyingi hamma boblar ulardan foydalanadi.

**Hayotdan misol: pasport seriyasi.** "AB1234567" — bitta satr, lekin ichida ikki ma'no bor: birinchi 2
belgi — seriya, qolgan 7 tasi — raqam. Pasport stolidagi xodim **joyi bo'yicha** ajratadi: "1-2 belgilar —
seriya". Protsessor buyrug'i ham shunday: `0xFFF50513` — bitta son, lekin "6..0-bitlar — opcode,
11..7-bitlar — rd, ..." deb **joyi bo'yicha** o'qiladi.

## 1.1. Ikkilik, o'n oltilik va nega hex

Bitta hex raqam = **aynan 4 bit**. Shuning uchun dasturchilar bitlarni hex'da yozadi: 32 bit = 8 ta hex raqam.

| hex | ikkilik | | hex | ikkilik |
|---|---|---|---|---|
| 0 | 0000 | | 8 | 1000 |
| 1 | 0001 | | 9 | 1001 |
| 2 | 0010 | | A | 1010 |
| 3 | 0011 | | B | 1011 |
| 4 | 0100 | | C | 1100 |
| 5 | 0101 | | D | 1101 |
| 6 | 0110 | | E | 1110 |
| 7 | 0111 | | F | 1111 |

Bu jadvalni **yod oling** — emulyator yozganda har kuni kerak bo'ladi. `0x80000000` ni ko'rganda darhol
"faqat 31-bit yoqiq" deb ko'rishingiz kerak; `0xFFF` — "past 12 bit yoqiq"; `0x1F` — "past 5 bit".

Mana `addi a0, a0, -1` buyrug'i bitlarda ([misollar/bitlar.c](../misollar/bitlar.c) chiqishi):

```console
$ make bitlar
addi a0, a0, -1                    0xfff50513 = 1111_1111_1111_0101_0000_0101_0001_0011
  opcode (6..0)  = 0x13
  rd     (11..7) = 10 (a0 = x10)
  rs1    (19..15)= 10
  imm    (31..20)= 0xfff -> ishora bilan: -1
```

`0xfff50513` ni 4 bitlik guruhlarga bo'ling — har guruh bitta hex raqam: `f f f 5 0 5 1 3`.

## 1.2. Andoza 1 — bitlarni ajratib olish: `(x >> past) & maska`

"x ning 11..7-bitlarini ber" degan so'rov ikki qadam:

1. **O'ngga suring** — kerakli bitlar 0-o'ringa tushsin: `x >> 7`.
2. **Maska bilan kesing** — faqat 5 ta past bit qolsin: `& 0x1F` (0x1F = `11111`, 5 ta bir).

```text
x            = 1111 1111 1111 0101 0000 0101 0001 0011
x >> 7       = 0000 0001 1111 1111 1110 1010 0000 1010      (pastdagi 7 bit "tushib ketdi")
& 0x1F       = 0000 0000 0000 0000 0000 0000 0000 1010  = 10
```

Maska — "**n ta bir**": `(1 << n) - 1`. Masalan `(1 << 5) - 1 = 32 - 1 = 31 = 0x1F`. Buni har safar yozmaslik
uchun emulyatorda makro bor ([emu/turlar.h](../emu/turlar.h)):

```c
#define BITLAR(x, yuqori, past) \
    ((uint32_t)(((x) >> (past)) & (uint32_t)((1ull << ((yuqori) - (past) + 1)) - 1)))

#define BIT(x, n) (((x) >> (n)) & 1u)
```

Nega `1ull` (64 bitli bir), `1u` emas? Chunki kenglik 32 bo'lishi mumkin: `BITLAR(x, 31, 0)`. `1u << 32` — C'da
**aniqlanmagan xatti-harakat** (UB, darslik 13-bob): 32 bitli sonni 32 yoki undan ko'p siljitish mumkin emas.
x86 da u ko'pincha `1u << 0 = 1` beradi (protsessor siljish miqdorining faqat past 5 bitini oladi) — natija
**jimgina** noto'g'ri. 64 bitli `1ull << 32` esa to'g'ri: `0x100000000`, undan 1 ayirsak — `0xFFFFFFFF`.

> **Eslab qoling:** siljish miqdori har doim turning kengligidan **kichik** bo'lishi shart. `x << 32` (x — 32 bit)
> — xato. Bu xato kompilyator ogohlantirishisiz o'tib ketadi va faqat ma'lum kirishlarda "otadi".

## 1.3. Andoza 2 — bitlarni joyiga qo'yish: `(qiymat & maska) << joy`

Teskari amal: "5 bitli sonni 11..7-o'rinlarga qo'y" — `(rd & 0x1F) << 7`. Bir nechta maydonni **OR** (`|`)
bilan birlashtiramiz. Mana C kengaytmasi kodidan (5-bob) 32 bitli buyruqni yig'uvchi funksiya:

```c
static uint32_t r_tur(uint32_t f7, uint32_t rs2, uint32_t rs1, uint32_t f3, uint32_t rd, uint32_t op)
{
    return f7 << 25 | rs2 << 20 | rs1 << 15 | f3 << 12 | rd << 7 | op;
}
```

Har maydon o'z joyiga suriladi va ular **ustma-ust tushmaydi** — shuning uchun `|` ularni "yopishtiradi".
(Agar ustma-ust tushsa — `|` ikkalasini aralashtirib yuboradi: bu xatoni tutish qiyin. Shuning uchun har
maydon to'g'ri kenglikda ekaniga ishonch hosil qiling.)

**Murakkabroq holat — bo'laklarni yig'ish.** RISC-V'ning ba'zi buyruqlarida son **bo'laklarga bo'lib**
yozilgan (2-bobda nima uchunligini ko'ramiz). Masalan S-tur (saqlash) buyrug'ida 12 bitli siljish:
yuqori 7 bit (`imm[11:5]`) buyruqning 31..25-bitlarida, pastki 5 bit (`imm[4:0]`) — 11..7-bitlarida.
Yig'ish: har bo'lakni ajratib (andoza 1), o'z joyiga qo'yamiz (andoza 2):

```text
imm = (BITLAR(b, 31, 25) << 5) | BITLAR(b, 11, 7)
         ^^^^^^^^^^^^^^^^^ imm[11:5]       ^^^^^^^^^^^^^^^ imm[4:0]
```

Bu — 2-bobdagi E1 mashqining kaliti.

## 1.4. Andoza 3 — maska bilan yozish: `(eski & ~maska) | (yangi & maska)`

Registrning **ba'zi** bitlarini o'zgartirish, qolganlarini tegmaslik kerak. Masalan `sstatus` registriga
yozish aslida `mstatus` ning faqat S rejimga ruxsat etilgan bitlarini o'zgartiradi (6-bob). Andoza:

1. `eski & ~maska` — eski qiymatdan maskadagi bitlarni **o'chiramiz** (`~` — hamma bitlarni teskari qiladi);
2. `yangi & maska` — yangi qiymatdan **faqat** maskadagi bitlarni olamiz;
3. `|` — birlashtiramiz.

```console
eski                               0x0000a0a0 = 0000_0000_0000_0000_1010_0000_1010_0000
yangi                              0x00000002 = 0000_0000_0000_0000_0000_0000_0000_0010
maska                              0x00000022 = 0000_0000_0000_0000_0000_0000_0010_0010
(eski & ~maska) | (yangi & maska)  0x0000a082 = 0000_0000_0000_0000_1010_0000_1000_0010
```

Natijada: 1-bit yangi qiymatdan (1 bo'ldi), 5-bit yangi qiymatdan (0 bo'ldi — o'chdi), qolgan hamma bitlar —
eskidan (7, 13, 15-bitlar saqlandi). [emu/csr.c](../emu/csr.c) da bu andoza alohida funksiya:

```c
static uint32_t maskali(uint32_t eski, uint32_t yangi, uint32_t maska)
{
    return (eski & ~maska) | (yangi & maska);
}
```

Uchta kichik "qarindoshi" ham bor — ularni ham yod oling:

| Ish | C | Misol |
|---|---|---|
| n-bitni yoqish | `x \|= 1u << n` | `mstatus \|= MSTATUS_SPP` |
| n-bitni o'chirish | `x &= ~(1u << n)` | `mstatus &= ~MSTATUS_SIE` |
| n-bitni teskari qilish | `x ^= 1u << n` | — |
| n-bit yoqiqmi | `(x >> n) & 1u` yoki `x & (1u << n)` | `if (pte & PTE_V)` |
| bit "ko'chirish": A ning qiymatini B ga | `if (x & A) x \|= B; else x &= ~B;` | trap'da `SPIE <- SIE` |

Oxirgi qator — 6-bobdagi E6 mashqining yarmi.

## 1.5. Ishorali va ishorasiz: bir xil bitlar, ikki xil ma'no

`0xFFFFFFFF` — bu 4 294 967 295 mi yoki -1 mi? **Ikkalasi ham**: bitlar bir xil, ma'no — siz qaysi turda
o'qishingizga bog'liq (ikkiga to'ldirish, darslik 2-bob). Protsessor uchun ham shunday: registrda shunchaki
32 bit turadi. **Buyruq** qaysi ma'noda o'qishni hal qiladi:

| Buyruq | Ma'nosi | C'da qanday yozamiz |
|---|---|---|
| `blt a, b` | a < b, **ishorali** | `(int32_t)a < (int32_t)b` |
| `bltu a, b` | a < b, **ishorasiz** | `a < b` (a, b — `uint32_t`) |
| `sra` | o'ngga arifmetik siljitish (ishora bitini ko'paytiradi) | `(uint32_t)((int32_t)a >> n)` |
| `srl` | o'ngga mantiqiy siljitish (chapdan nollar) | `a >> n` |

```text
a = 0xFFFFFFFF, b = 1
  blt:  -1 < 1          -> ROST  (sakraydi)
  bltu: 4294967295 < 1  -> YOLG'ON
```

Emulyatorda registrlar **doim** `uint32_t`. Ishorali ma'no kerak bo'lgan joyda **aniq** `(int32_t)` ga
o'tkazamiz. Bu qoida xatolarni kamaytiradi: "bu yerda qaysi ma'no?" degan savol har safar ko'z oldida.

> **Diqqat — C tuzog'i:** `int` va `unsigned` aralashsa, C **ishorasizga** o'tkazadi: `-1 < 1u` — **yolg'on**!
> Shuning uchun `(int32_t)a < (int32_t)b` da **ikkalasini** ham o'tkazing.

## 1.6. Andoza 4 — ishora bilan kengaytirish

Buyruq ichidagi son ko'pincha 12 bit. 12 bitda `0xFFF` = -1. Uni 32 bitli registrga qo'shish uchun 32 bitli
-1 = `0xFFFFFFFF` kerak, `0x00000FFF` (= 4095) emas. Qoida: **eng yuqori (ishora) bit 1 bo'lsa, yuqoridagi hamma
bitlarni 1 bilan to'ldir; 0 bo'lsa — 0 bilan**.

Bunda C'ning bitta xususiyatidan foydalanamiz: ishorali sonni o'ngga siljitish ishora bitini **ko'paytiradi**.

```c
static inline uint32_t ishora_kengaytir(uint32_t qiymat, int bitlar)
{
    int siljish = 32 - bitlar;
    return (uint32_t)((int32_t)(qiymat << siljish) >> siljish);
}
```

Qadamlar (bitlar = 12):

```console
12 bit: 0xFFF                      0x00000fff = 0000_0000_0000_0000_0000_1111_1111_1111
<< 20                              0xfff00000 = 1111_1111_1111_0000_0000_0000_0000_0000
(int32_t) >> 20                    0xffffffff = 1111_1111_1111_1111_1111_1111_1111_1111
(uint32_t) >> 20 (xato!)           0x00000fff = 0000_0000_0000_0000_0000_1111_1111_1111
```

1. Chapga 20 ga: 12 bitli sonning ishora biti (11-bit) 31-o'ringa chiqdi.
2. `int32_t` sifatida o'ngga 20 ga: 31-bit (1) bo'shagan joylarga ko'paytirildi.
3. Oxirgi qator — xato varianti: ishorasiz siljitish chapdan **nol** kiritadi — ishora yo'qoldi.

> **Eslatma:** manfiy sonni o'ngga siljitish C standartida "implementatsiyaga bog'liq" (C23 gacha). GCC va
> Clang buni **hujjatlashtirilgan** holda arifmetik siljitish qiladi; loyiha shunga tayanadi. Ehtiyotkor
> muqobil: `(qiymat ^ (1u << (bitlar-1))) - (1u << (bitlar-1))` — faqat ishorasiz arifmetika bilan.
> Qog'ozda tekshirib ko'ring: nega bu ham ishlaydi?

## 1.7. Andoza 5 — baytlar tartibi (endianness)

32 bitli son xotirada 4 ta baytda turadi. Qaysi bayt birinchi (kichik manzilda)?

- **Little-endian** (x86, ARM, RISC-V): **past** bayt birinchi.
- **Big-endian** (tarmoq protokollari, DTB formati — 9-bob): **yuqori** bayt birinchi.

```console
0x12345678 xotirada: [78] [56] [34] [12]  (kichik manzildan)
```

Emulyatorning RAM — oddiy `uint8_t` massivi. Undan 4 baytli son o'qish ([emu/shina.c](../emu/shina.c)):

```c
const uint8_t *p = m->ram + (fiz - RAM_BOSH);
uint32_t v = 0;
for (int i = hajm - 1; i >= 0; i--)
    v = (v << 8) | p[i];                /* little-endian: oxirgi (yuqori) baytdan boshlab yig'amiz */
```

Nega `*(uint32_t *)p` deb yozmadik? Uch sabab:

1. Emulyator **big-endian** kompyuterda ham to'g'ri ishlashi kerak — siljitish bilan yozilgan kod
   endianness'dan mustaqil.
2. `p` 4 ga tekis bo'lmasligi mumkin — ba'zi protsessorlarda (va C standartida) bu **UB**.
3. "Strict aliasing" qoidasi (darslik 13-bob): `uint8_t` massivni `uint32_t *` orqali o'qish — UB.

Kompilyator bu tsiklni x86 da baribir **bitta** `mov` buyrug'iga aylantiradi — tezlik yo'qotilmaydi.

## 1.8. Xavfli burchaklar ro'yxati

Emulyatorda uchraydigan C tuzoqlari — hammasi haqiqiy xatolardan olingan:

| Tuzoq | Nima bo'ladi | To'g'risi |
|---|---|---|
| `1u << 32` | UB: x86 da odatda 1 | `1ull << 32` yoki holatni alohida tekshirish |
| `INT32_MIN / -1` | UB: x86 da dastur **qulaydi** (SIGFPE) | avval tekshirish (3-bob, E4) |
| `x / 0` | UB: qulaydi | RISC-V'da natija aniqlangan — avval tekshirish |
| `int32_t` toshishi (`0x7FFFFFFF + 1`) | UB | arifmetikani `uint32_t` da qiling (toshish aniqlangan: mod 2³²) |
| `-1 < 1u` | yolg'on! (ishorasizga o'tadi) | ikkala tomonni bir turga o'tkazing |
| `(uint8_t)x << 24` | `int` ga o'tadi; 0x80 bo'lsa — ishora bitiga tushadi (UB) | `(uint32_t)x << 24` |
| `char` ishoralimi? | platformaga bog'liq | baytlar uchun doim `uint8_t` |

Oxirgisidan oldingisini batafsil: C'da `uint8_t` arifmetikadan oldin **`int` ga ko'tariladi** (integer
promotion). `int` esa ishorali. `(uint8_t)0x80 << 24 = 0x80000000` — `int` uchun bu toshish. Shuning uchun
emulyatordagi `be32_yoz`/`shina_oqi` kabi joylarda baytni avval `uint32_t` ga o'tkazamiz.

## 1.9. O'zingizni tekshiring

Qog'ozda hisoblang, keyin kichik C dastur bilan tekshiring (javoblar pastda).

1. `BITLAR(0x12345678, 15, 8)` = ?
2. `BITLAR(0x80000000, 31, 31)` = ? Va `BIT(0x80000000, 31)` = ?
3. `ishora_kengaytir(0x800, 12)` = ? `ishora_kengaytir(0x7FF, 12)` = ?
4. `0x0000ABCD` ning 7..4-bitlarini `0x3` ga almashtiring (andoza 3). Natija = ?
5. `(int32_t)0xFFFFFFFE < (int32_t)3` va `0xFFFFFFFEu < 3u` — qaysi biri rost?
6. 0xCAFEBABE little-endian'da xotiraga qanday tushadi? Big-endian'da-chi?
7. Nega `BITLAR(x, 31, 0)` makrosida `1u` ishlatilsa xato bo'lardi? Qaysi qiymat qaytgan bo'lardi (x86 da)?

<details><summary>Javoblar</summary>

1. `0x56`. 2. `1` va `1`. 3. `0xFFFFF800` (-2048) va `0x000007FF` (2047).
4. maska = `0xF0`; `(0xABCD & ~0xF0) | (0x3 << 4)` = `0xAB0D | 0x30` = `0xAB3D`.
5. Birinchisi rost (-2 < 3), ikkinchisi yolg'on (4294967294 < 3 emas).
6. LE: `BE BA FE CA`; BE: `CA FE BA BE`.
7. `1u << 32` — UB; x86 da siljish miqdori 32 mod 32 = 0 bo'lib, `(1 << 0) - 1 = 0` — maska nol, natija **doim 0**.

</details>

## Savol-javob

**Savol:** Bitfield'lar (`struct { unsigned rd : 5; }`) bilan qilsa bo'lmaydimi — qulayroq-ku?
**Javob:** C standarti bitfield'larning **xotiradagi tartibini** belgilamaydi: qaysi bit birinchi —
kompilyatorga bog'liq. Apparat formatini aniq ifodalash uchun ular ishonchsiz. Shuning uchun Linux ham,
QEMU ham, bizning emulyator ham — siljitish va maska bilan ishlaydi.

**Savol:** Shuncha makro, `inline` funksiyalar — sekinlashtirmaydimi?
**Javob:** Yo'q. `-O2` da `BITLAR(b, 11, 7)` bitta `shr` va bitta `and` buyrug'iga aylanadi. Ishonmasangiz:
`gcc -O2 -S emu/dekod.c` va `.s` faylga qarang (darslik 17-bob).
