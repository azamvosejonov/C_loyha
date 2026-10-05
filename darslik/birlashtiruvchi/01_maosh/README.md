# Birlashtiruvchi loyiha 1: Maosh hisobchisi

> **Bu loyiha nimaga kerak:** 0, 1 va 2-boblarda o'rganganlaringizni **bitta masalada** ishlatasiz: `printf` (0-bob), ko'p fayl va alohida kompilyatsiya (1-bob), butun turlar, toshish, `char`, kasr o'rniga tiyin (2-bob).
> **Oldindan nima kerak:** 0–2-boblar (Asos bobi ham yordam beradi). `if`/`for` **kerak emas**.
> **Vaqt:** 3–5 soat (4 bosqich). Kod **yechimsiz** — yo'l-yo'riq va kutilgan natija bor; kodni **o'zingiz** yozasiz.

## Masala (hayotdan)

Kichik kompaniyada hisobchi har oy **maosh varaqasi** (payslip) tayyorlaydi. Siz shu varaqani chiqaradigan dastur yozasiz. Qoidalar oddiy:

| Nima | Qoida |
|---|---|
| **Asosiy ish haqi** | oddiy soat × soatlik tarif |
| **Ustama** | qo'shimcha soat × tarif × **1.5** (ortiqcha ish — ko'proq to'lanadi) |
| **Brutto** | asosiy + ustama |
| **Soliq** | brutto ning **12%** i |
| **Kasaba badali** | brutto ning **1%** i |
| **Qo'lga tegadi** | brutto − soliq − kasaba |

**Qaysi bobdan nima ishlatiladi:**

| Bob | Nima | Loyihada qayerda |
|---|---|---|
| 0 | `printf`, `%d`, `%s`, `%-20s`, `%10d`, `%%`, `\n` | hamma bosqichda — varaqani chiroyli chiqarish |
| 1 | `.h` + `.c`, alohida kompilyatsiya, `#include "..."`, include guard, linker | 2-bosqichdan |
| 2 | `int64_t`, `uint8_t`, `char` arifmetikasi, butun bo'lish, toshish, `(tur)` aylantirish, `static` o'zgaruvchi (scope) | 3–4-bosqichlar |

## Qanday ishlash kerak

1. **Har bosqichga alohida papka** oching: `mkdir -p ~/maosh/b1 ~/maosh/b2 ~/maosh/b3 ~/maosh/b4`.
2. Spetsifikatsiyani o'qing, **avval qog'ozda** hisoblab ko'ring, keyin yozing.
3. Tekshiring: `~/C_loyha/darslik/birlashtiruvchi/01_maosh/tekshir.sh BOSQICH PAPKA`, masalan:

```bash
~/C_loyha/darslik/birlashtiruvchi/01_maosh/tekshir.sh 1 ~/maosh/b1
```

`tekshir.sh` nima qiladi: kerakli fayllar bormi → **har `.c` ni alohida** `gcc -Wall -Wextra -c` bilan kompilyatsiya qiladi (**ogohlantirish ham xato**) → bog'laydi → ishga tushirib natijani `kutilgan_N.txt` bilan solishtiradi. Farq bo'lsa, qaysi qator farq qilishini ko'rsatadi.

> Maslahat: xato chiqsa **o'zingiz** sababini toping, so'ng "O'zingizni tekshiring" savollarini o'qing. Yechimni boshqadan ko'rib olish — o'rganishni to'xtatadi.

---

## 1-bosqich: bitta varaqa, oddiy `int` (0-bob)

**Fayl:** `maosh.c` (hamma narsa `main` ichida).

**Ma'lumot** (o'zgaruvchilar qilib yozing): xodim **Aziza**; oddiy soat **160**; qo'shimcha soat **12**; tarif **25000** so'm/soat.

**Talab:**
- Hamma qiymatni **o'zgaruvchilarda** hisoblang (`asosiy`, `ustama`, `brutto`, `soliq`, `kasaba`, `sof`).
- Chiqarish: yorliq `%-20s` (chapga tekis), qiymat `%10d` (o'ngga tekis), keyin ` so'm` yoki ` soat`. Xodim nomi: `%10s`.
- `Soliq (12%):` yorlig'ida **`%` belgisini** chiqarish uchun nima yozish kerakligini 0-bobdan eslang.
- Butun bo'lish: `soliq = brutto * 12 / 100` (kasr qismi tashlab yuboriladi — hozircha shunday).

**Kutilgan natija** (`kutilgan_1.txt`):

```text
=== MAOSH VARAQASI ===
Xodim:                   Aziza
Oddiy soat:                160 soat
Qo'shimcha soat:            12 soat
Soatlik tarif:           25000 so'm
Asosiy ish haqi:       4000000 so'm
Ustama (x1.5):          450000 so'm
Brutto:                4450000 so'm
Soliq (12%):            534000 so'm
Kasaba (1%):             44500 so'm
Qo'lga tegadi:         3871500 so'm
```

**Maslahat (yechim emas):**
- **Tartib muhim!** `ustama = qosh * tarif * 3 / 2` va `qosh * tarif * (3 / 2)` **turli** natija beradi. Nega? (`3 / 2` butun sonlarda nechaga teng?)
- Avval qog'ozda: 160 × 25000 = ? 12 × 25000 × 1.5 = ?
- `printf` da `%d` ga **butun** son berasiz; `%s` ga — matn.

**O'zingizni tekshiring:**
1. `%-20s` va `%10d` dagi `20` va `10` nimani bildiradi? Ularni o'zgartirsangiz nima bo'ladi?
2. `7 * 12 / 100` va `12 / 100 * 7` natijalari nega farq qiladi?
3. `Soliq (12%):` ni `printf("...12%...")` deb yozsangiz kompilyator nima deydi? (`-Wall` bilan sinab ko'ring.)

---

## 2-bosqich: ko'p fayl (1-bob)

**Fayllar:** `main.c`, `hisob.h`, `hisob.c`. **Natija 1-bosqichdagi bilan aynan bir xil.**

**Talab:**
- Hisoblarni **`hisob.c`** ga ko'chiring: uchta funksiya (nomlari tekshirilmaydi, lekin ma'noli bo'lsin): asosiy ish haqi, ustama, foiz.
- **`hisob.h`** da funksiyalarning **e'loni** (prototip) va **include guard** bo'lsin (`#ifndef HISOB_H` / `#define HISOB_H` / `#endif`).
- `main.c` da `#include "hisob.h"` (qo'shtirnoq — **o'z** sarlavhangiz; burchakli qavs `<...>` — tizimniki).
- Qo'lda kompilyatsiya qilib ko'ring (`tekshir.sh` ham shunday qiladi):

```bash
gcc -Wall -Wextra -c main.c -o main.o
gcc -Wall -Wextra -c hisob.c -o hisob.o
gcc main.o hisob.o -o maosh
./maosh
```

**Maslahat:**
- `hisob.c` ning boshida ham `#include "hisob.h"` yozing: shunda e'lon va ta'rif **mos kelmasa**, kompilyator darrov aytadi.
- `hisob.c` dan `hisob.o` ni oling va `main.c` ni **o'zgartirib**, faqat `main.o` ni qayta yig'ing — `hisob.o` qayta kompilyatsiya **kerak emas**. Mana shu alohida kompilyatsiyaning foydasi.

**O'zingizni tekshiring:**
1. `hisob.c` dagi funksiya nomini `main.c` dagidan boshqacha yozsangiz — **kompilyatsiya** xato beradimi yoki **linker**? (Sinang va 1-bobdagi "undefined reference" bilan solishtiring.)
2. `hisob.h` dagi `#ifndef` qatorlarini olib tashlab, `main.c` da `#include "hisob.h"` ni **ikki marta** yozing. Nima bo'ladi? (Funksiya e'loni takrorlansa xato emas — shu sababli sinash uchun sarlavhaga `struct` yoki `typedef` qo'shib ko'ring.)
3. Nega `gcc main.c hisob.c -o maosh` ham ishlaydi, lekin katta loyihalarda `.o` fayllarga bo'lib kompilyatsiya qilinadi?

---

## 3-bosqich: pul tiyinda, aniq turlar, nazorat raqami (2-bob)

**Fayllar:** `main.c`, `hisob.h`, `hisob.c`, **yangi:** `pul.h`, `pul.c`.

Hozirgacha hamma narsa `int` va **so'm**da edi: tiyinlar yo'qolardi, katta summada esa `int` **toshardi**. Endi hamma narsa **tiyinda** (`1 so'm = 100 tiyin`) va **`int64_t`** da.

**Yangi talablar:**

1. **Pul — `int64_t` tiyinda.** Tarif endi **25000.50 so'm = `2500050` tiyin**. Hisob funksiyalari `int64_t` qabul qilib qaytaradi.
2. **`pul.c`:**
   - `foiz(summa, p)` — summaning `p` foizi, **tiyinga yaxlitlab** (.5 va undan yuqori — yuqoriga). Butun sonlarda yaxlitlashning mashhur usuli: bo'lishdan **oldin** bo'luvchining yarmini qo'shish. (Maslahat: `(summa * p + 50) / 100`.)
   - `pul_chiqar(yorliq, tiyin)` — shu formatda: `printf("%-20s%12lld.%02lld so'm\n", yorliq, tiyin / 100, tiyin % 100)`. `lld` — `long long` uchun; `int64_t` ni `(long long)` ga o'tkazing. `%02lld` — **2 xona, bosh nol bilan** (5 tiyin → `05`).
3. **ID va nazorat raqami.** Xodim ID si 4 ta `char` (`'1'`, `'0'`, `'4'`, `'2'`). Nazorat raqami:
   `((r1 − '0')·3 + (r2 − '0')·7 + (r3 − '0')·1 + (r4 − '0')·3) % 10`.
   `'1' − '0'` — belgidan **raqam qiymatini** olish (2-bob: `char` — son ham). Chiqarish: `1042-3`.
4. **Imzo kodi — `uint8_t`.** `(uint8_t)(r1 + r2 + r3 + r4 + 100)`. Belgilarning ASCII kodlari yig'indisi `uint8_t` ga sig'masa — **aylanib o'tadi** (255 dan keyin 0).
5. **Yillik ish haqi** = qo'lga tegadi × 12. Uni **ikki xil** chiqaring: to'g'ri (`int64_t`) va **`int32_t` ga o'tkazib** (toshish namoyishi). `int32_t` qiymatini **tiyinda** `%d` bilan chiqaring.

Hisob **1 ta xodim** uchun (Aziza, ID `1042`, tarif **2500050** tiyin).

**Kutilgan natija** (`kutilgan_3.txt`):

```text
=== MAOSH VARAQASI ===
Xodim:                        Aziza
ID:                          1042-3
Imzo kodi:                       43
Oddiy soat:                     160 soat
Qo'shimcha soat:                 12 soat
Soatlik tarif:             25000.50 so'm
Asosiy ish haqi:         4000080.00 so'm
Ustama (x1.5):            450009.00 so'm
Brutto:                  4450089.00 so'm
Soliq (12%):              534010.68 so'm
Kasaba (1%):               44500.89 so'm
Qo'lga tegadi:           3871577.43 so'm
Yillik (int64_t):       46458929.16 so'm
Yillik (int32_t):         350925620 tiyin (XATO!)

```

(Oxirida bitta **bo'sh qator** bor.) Xodimning ma'lumoti: `varaqa("Aziza", '1', '0', '4', '2', 160, 12, 2500050)` ko'rinishidagi **bitta funksiya**ga o'ralsin — 4-bosqichda uni qayta ishlatasiz.

**Maslahat:**
- Imzo kodi: `'1' + '0' + '4' + '2'` = 49 + 48 + 52 + 50 = **199**; +100 = 299; `uint8_t` da 299 − 256 = **43**.
- Yillik: 3 871 577.43 so'm × 12 = 46 458 929.16 so'm = **4 645 892 916 tiyin**. `int32_t` ning eng kattasi 2 147 483 647 — **sig'maydi**. 4 645 892 916 − 2³² = **350 925 620**: aynan shu chiqadi.
- `int64_t` bilan `int` ni aralashtirib ko'paytirsangiz, kompilyator `int` ni **kengaytiradi**; lekin `int * int` **avval** `int` da hisoblanadi (va toshishi mumkin), keyin kengayadi. Shuning uchun **kamida bittasini** `int64_t` qilib yozing.
- Funksiya e'lonlarida (`.h`) turlar `.c` dagi ta'rifga **aynan mos** bo'lsin.

**O'zingizni tekshiring:**
1. `int64_t` ning o'rniga `int` ishlatsangiz, qaysi qatordan boshlab natija **buziladi**? (`brutto * 12` ni hisoblang: 445 008 900 × 12 = ?)
2. `'7' - '0'` nega `7` beradi, `'7'` o'zi esa 55? (ASCII jadvali, 2-bob va Asos bob.)
3. Soliq **yaxlitlashsiz** (`brutto * 12 / 100`) bo'lsa, natija nechaga farq qiladi? Qaysi qiymatda?
4. `%02lld` ni `%2lld` ga almashtirsangiz, 5 tiyin qanday chiqadi? Nega bu pul uchun xato?

---

## 4-bosqich: to'liq tizim — to'rt xodim va umumiy hisobot (0–2 birga)

**Fayllar:** 3-bosqichdagi hammasi. `main.c` ga **4 ta xodim** va **jami** qo'shiladi.

| ID | Ism | Oddiy soat | Qo'shimcha soat | Tarif (tiyin) |
|---|---|---|---|---|
| 1042 | Aziza | 160 | 12 | 2500050 |
| 2087 | Bobur | 152 | 0 | 3150000 |
| 3150 | Dilnoza | 168 | 24 | 1875050 |
| 9999 | Sardor | 160 | 40 | 1250000075 |

**Talab:**
- `varaqa(...)` ni 4 marta chaqiring (har biridan keyin bo'sh qator).
- **Umumiy yig'indilar** — `main.c` da **fayl darajasidagi `static int64_t`** o'zgaruvchilar (2-bob: ko'rinish sohasi): `jami_brutto`, `jami_soliq`, `jami_kasaba`, `jami_sof`. `varaqa` ichida ularga qo'shiladi.
- Oxirida shu ko'rinishda:

```text
=== JAMI ===
Brutto:               2763063356.00 so'm
Soliq:                 331567602.72 so'm
Kasaba:                 27630633.56 so'm
Qo'lga tegadi:        2403865119.72 so'm
```

Qolgan natija — `kutilgan_4.txt` (to'liq 4 ta varaqa + jami, 69 qator). Sardor uchun diqqat qiling: uning yillik ish haqi **`int32_t` da juda boshqacha** chiqadi.

**Maslahat:**
- Dastur **uchta fayl guruhi**dan iborat: `main.c` (qanday chiqarish), `hisob.c` (qancha), `pul.c` (pul qanday yoziladi). Har biri **o'z ishini** qiladi — shu "mas'uliyatlarni ajratish" katta dasturlarning asosi.
- `static` o'zgaruvchi **faqat shu faylda** ko'rinadi (boshqa fayl uni **ko'ra olmaydi** — linker xato bermaydi, shunchaki mavjud emas). Bu 11-bobdagi "ichki bog'lanish".
- Aziza/Bobur/Dilnoza qiymatlari 3-bosqichdagi bilan bir xil formulalar bilan chiqadi. Faqat **bitta** qiymatni qo'lda tekshirib, qolganiga ishoning: **Bobur** uchun: 152 × 3 150 000 = 478 800 000 tiyin = 4 788 000.00 so'm.

**O'zingizni tekshiring:**
1. `jami_brutto` ni `int` qilsangiz, nechanchi xodimdan keyin `toshadi`? (`int` chegarasi ≈ 2.1 mlrd; Sardorning bruttosi 275 mlrd tiyin.)
2. `static` ni olib tashlab, boshqa faylda `extern int64_t jami_brutto;` yozsangiz nima bo'ladi? (1-bobdagi "e'lon va ta'rif" bilan bog'lang.)
3. Imzo kodlari (43, 53, 45, 72): `uint8_t` o'rniga `int` ishlatsangiz, qaysilari o'zgaradi? Nega?
4. Yangi xodim qo'shish uchun **nechta fayl** o'zgaradi? (Yaxshi dizaynda — faqat `main.c` ning bitta qatori.)

---

## Tuzoqlar ro'yxati (yakunda o'zingizga qarang)

| Tuzoq | Qaysi bob | Qayerda uchradi |
|---|---|---|
| `a * b * 3 / 2` va `a * b * (3 / 2)` | 2 (butun bo'lish) | ustama |
| `%` ni `printf` da chiqarish | 0 | `Soliq (12%):` |
| `int` toshishi | 2 (overflow) | katta summalar, yillik |
| `uint8_t` aylanishi | 2 | imzo kodi |
| `char` — bu son | 2 | nazorat raqami |
| e'lon va ta'rif, linker | 1 | `hisob.h` / `hisob.c` |
| include guard | 1 | sarlavha fayllari |
| ko'rinish sohasi (`static`) | 2 | `jami_*` |

## Tugatgach

- Hamma 4 bosqich `tekshir.sh` dan **TO'G'RI** olsa — 0–2-boblar sizniki.
- **Ixtiyoriy kengaytirish** (yechimsiz): (1) dasturni **tarif 0** bo'lgan xodim bilan sinang; (2) soliq **13%** bo'lsa, qaysi **bitta** joyni o'zgartirasiz?; (3) `-fsanitize=undefined` bilan yig'ib, qaysi qatorda `int` toshishi aniqlanishini ko'ring (`int` versiyasida).
- Keyingi birlashtiruvchi loyiha 3–5-boblar (operatorlar, `if`/`switch`, funksiyalar) o'tilgach tayyorlanadi.
