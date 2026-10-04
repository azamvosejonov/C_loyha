# Belgilar lug'ati: kodda uchraydigan har bir belgi nima qiladi

> **Bu sahifada:** C kodida uchraydigan **belgilar** (`|`, `&`, `*`, `->`, `%`, `<<` ...) nima ekanini
> bilib olasiz. Notanish belgi ko'rsangiz, shu yerga qarang.
> **Qanday foydalaniladi:** belgini toping → "nima qiladi" ustunini o'qing → kichik misolni ko'ring →
> keyingi ustundagi bobga o'ting. Bu sahifa **tez qidirish** uchun; tafsilot — boblarda.

## 1. Hisoblash belgilari (3-bob)

| Belgi | Nomi | Nima qiladi | Misol | Natija |
|---|---|---|---|---|
| `+` `-` `*` | qo'shish, ayirish, ko'paytirish | maktabdagidek | `7 * 3` | `21` |
| `/` | bo'lish | **butun** sonlarni bo'lsa **butun** chiqadi (kasr tashlanadi) | `7 / 2` | `3` |
| `%` | qoldiq | bo'lgandan keyin **ortib qolgan** | `7 % 2` | `1` |

**Eslab qoling:** `/` Python'dagi `//` kabi (butun sonlar bilan). Haqiqiy bo'lish uchun bittasini kasr qiling: `7 / 2.0` = `3.5`.

## 2. Solishtirish belgilari (3-bob)

| Belgi | Ma'nosi | Misol | Natija |
|---|---|---|---|
| `==` | **teng** | `5 == 5` | `1` (rost) |
| `!=` | teng emas | `5 != 5` | `0` (yolg'on) |
| `<` `>` `<=` `>=` | kichik, katta, kichik yoki teng, katta yoki teng | `3 < 5` | `1` |

C'da "rost" — `1`, "yolg'on" — `0`. `True` / `False` so'zlari yo'q (`<stdbool.h>` ni qo'shsangiz bor).

> **Eng mashhur xato:** `if (x = 5)` yozish. Bu **tenglik emas**, `x` ga 5 **berish**! Tenglik uchun **ikkita** `==`.

## 3. Mantiq belgilari (3-bob)

| Belgi | O'qilishi | Python'da | Misol |
|---|---|---|---|
| `&&` | **va** (ikkalasi ham rost) | `and` | `a > 0 && a < 10` |
| `\|\|` | **yoki** (kamida bittasi rost) | `or` | `a < 0 \|\| a > 10` |
| `!` | **emas** (teskari) | `not` | `!tayyor` |

## 4. Bit belgilari — sonning har bir biti bilan ishlash (3-bob)

Bu belgilar sonning **ichki** (0 va 1 lardan iborat) ko'rinishiga ishlaydi.

| Belgi | Nomi | Bir gapda | Misol (4 bit) | Natija |
|---|---|---|---|---|
| `&` | VA | har bitni solishtiradi: ikkalasida **1** bo'lsa 1 | `1100 & 1010` | `1000` |
| `\|` | YOKI | **kamida** bittasida 1 bo'lsa 1 | `1100 \| 1010` | `1110` |
| `^` | XOR | **faqat bittasida** 1 bo'lsa 1 (farq bo'lsa 1) | `1100 ^ 1010` | `0110` |
| `~` | teskari | har bitni **aylantiradi** (0↔1) | `~1100` | `0011` |
| `<<` | chapga surish | bitlarni chapga suradi (**×2** har qadamda) | `0011 << 2` | `1100` |
| `>>` | o'ngga surish | bitlarni o'ngga suradi (**÷2** har qadamda) | `1100 >> 2` | `0011` |

**Ko'p adashtiriladigan juftliklar:**

| Bir belgili (bit) | Ikki belgili (mantiq) | Farqi |
|---|---|---|
| `&` | `&&` | `&` — **bitlar** bilan; `&&` — "ikkalasi rost-mi?" degan **savol** |
| `\|` | `\|\|` | xuddi shunday |

Misol: `6 & 1` = `0` (110 va 001 da umumiy 1 yo'q), lekin `6 && 1` = `1` (ikkalasi ham 0 emas).
Tafsilot: 3-bobning 3.4 bo'limi.

## 5. Berish belgilari (3-bob)

| Yozilishi | Bilan bir xil | Nima qiladi |
|---|---|---|
| `x = 5` | | `x` ga 5 **berish** (tenglik emas!) |
| `x += 3` | `x = x + 3` | 3 qo'shish |
| `x -= 3` `x *= 3` `x /= 3` `x %= 3` | shunga o'xshash | |
| `x \|= 4` | `x = x \| 4` | bitni **yoqish** |
| `x &= ~4` | `x = x & ~4` | bitni **o'chirish** |
| `x ^= 4` | `x = x ^ 4` | bitni **almashtirish** |
| `x <<= 2` `x >>= 2` | | surib, natijani `x` ga yozish |
| `x++` / `x--` | `x = x + 1` / `x = x - 1` | bittaga oshirish / kamaytirish |

## 6. Manzil va ko'rsatkich belgilari (7-bob)

| Belgi | O'qilishi | Misol | Ma'nosi |
|---|---|---|---|
| `&x` | "x ning **manzili**" | `int *p = &x;` | `x` qayerda turibdi |
| `*p` (ifodada) | "p ko'rsatgan joydagi qiymat" | `*p = 5;` | manzil bo'yicha borib, o'sha yerni o'zgartirish |
| `int *p` (e'londa) | "p — `int` ga **ko'rsatkich**" | | `*` bu yerda turning bir qismi |
| `p->nom` | "p ko'rsatgan **struktura**ning `nom` maydoni" | | `(*p).nom` ning qisqasi |
| `s.nom` | "`s` strukturasining `nom` maydoni" | | `s` oddiy struktura (ko'rsatkich emas) |
| `a[i]` | "`a` massivining `i`-elementi" | `a[0]` | sanash **0 dan** boshlanadi |
| `a[i][j]` | ikki o'lchamli massiv | | `i`-qator, `j`-ustun |

`*` ning **uch** ma'nosi bor: `a * b` — ko'paytirish; `int *p` — e'londa ko'rsatkich; `*p` — ifodada "manzil bo'yicha bor".
Qaysi ekanini **joyiga** qarab bilasiz.

## 7. Qavslar va tinish belgilari

| Belgi | Nima | Misol |
|---|---|---|
| `( )` | guruhlash, funksiya chaqirish | `(a + b) * c`, `printf("...")` |
| `{ }` | **blok**: bir nechta buyruqni birlashtiradi | `if (x) { a(); b(); }` |
| `[ ]` | massiv, indeks | `int a[5];` `a[2]` |
| `;` | buyruq **oxiri** (gapning nuqtasi) | `x = 5;` |
| `,` | ro'yxatda ajratgich | `f(a, b)` |
| `? :` | qisqa `if` (3 qismli) | `x > 0 ? 1 : -1` |
| `//`, `/* */` | **izoh** (kompilyator o'qimaydi) | `x = 5; // beshga tenglash` |
| `\n`, `\t`, `\0`, `\\`, `\"` | maxsus belgilar (`\` bilan boshlanadi) | `"salom\n"` |

**Maxsus belgilar:** `\n` — yangi qator; `\t` — tab; `\0` — satr tugash belgisi; `\\` — haqiqiy `\`; `\"` — haqiqiy `"`.

## 8. `printf` formatlari — `%` bilan boshlanadi (0.8, 12-bob)

`printf("...%d...", son)` da `%d` — "shu yerga son qo'y".

| Format | Nima chiqaradi | Misol |
|---|---|---|
| `%d` | butun son (`int`) | `42` |
| `%u` | ishorasiz butun | `4000000000` |
| `%ld` / `%lld` | uzun butun (`long` / `long long`) | |
| `%zu` | `sizeof` natijasi (`size_t`) | `8` |
| `%f` / `%.2f` | kasr son / **2 xonali** kasr | `3.14` |
| `%c` | bitta belgi | `A` |
| `%s` | satr (matn) | `salom` |
| `%x` / `%X` | o'n oltilik (kichik / katta harf) | `ff` / `FF` |
| `%o` | sakkizlik | `755` |
| `%p` | manzil (ko'rsatkich) | `0x7ffd...` |
| `%%` | haqiqiy `%` belgisi | `%` |
| `%5d`, `%-5d`, `%05d` | kenglik 5: o'ngga tekis / **chapga** tekis / nol bilan to'ldirilgan | `   42`, `42   `, `00042` |

## 9. `#` bilan boshlangan qatorlar — preprotsessor (10-bob)

Bu qatorlar C buyruqlari emas — **kompilyatordan oldin** ishlaydigan **matn almashtirish** buyruqlari.

| Qator | Nima qiladi |
|---|---|
| `#include <x.h>` | `x.h` faylini shu joyga ko'chiradi ([sarlavhalar](sarlavhalar.md)) |
| `#define N 10` | kodda `N` ni hamma joyda `10` bilan **almashtiradi** |
| `#ifdef`, `#ifndef`, `#if`, `#endif` | kodning bir qismini **shartli** qo'shadi / olib tashlaydi |
| `#pragma once` | sarlavha faylni ikki marta ko'chirmaslik |

## 10. Tez-tez uchraydigan **so'zlar** (belgi emas, lekin dastlab sirli ko'rinadi)

| So'z | Oddiy ma'nosi |
|---|---|
| `int main(void)` | dasturning **kirish eshigi**; dastur shu yerdan boshlanadi |
| `return 0;` | "hammasi joyida" deb tugatish |
| `void` | "hech narsa" (hech narsa qaytarmaydi / olmaydi) |
| `static` | "faqat shu fayl uchun" yoki "chaqiruvlar orasida eslab qol" (5-bob) |
| `const` | "o'zgartirib bo'lmaydi" |
| `sizeof(x)` | `x` xotirada necha **bayt** joy olishi |
| `NULL` | "hech qayerga ko'rsatmaydi" degan ko'rsatkich |
| `struct`, `enum`, `union`, `typedef` | o'z turingni yasash (9-bob) |

## Xulosa

1. Notanish belgi — bu sahifadan toping; nomi va bir gaplik ma'nosi yoziladi.
2. Bir xil ko'rinadigan juftliklarni ajrating: `=` / `==`, `&` / `&&`, `|` / `||`, `*` (3 ma'no).
3. Bit belgilarini (`& | ^ ~ << >>`) ko'proq mashq qiling — yadro dasturlashda eng ko'p kerak bo'ladi (3-bobdagi "bitlar" bo'limi).
4. `%` printf'da ham, qoldiqda ham bor — joyiga qarab farqlanadi.
5. Hammasini bir yo'la yodlash shart emas: kod yozganingiz sayin o'zi eslab qoladi.
