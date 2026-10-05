# Katta loyihalar

Darslik boblari mashqlardan ham iborat, lekin **bitta katta dastur**ni bobdan bobga o'stirib borish o'rganishning eng yaxshi usuli:
har yangi tushuncha oldingi versiyadagi **haqiqiy muammoni** hal qiladi, shuning uchun "nega bu kerak?" degan savol qolmaydi.

## Ombor (0–12-boblar)

Mahsulotlar ombori boshqaruvi: ro'yxat, sotish, qo'shish, o'chirish, hisobot, saralash, faylga saqlash. 13 bosqich — har bobda bittadan:

| Bosqich | Bob | Papka | Yangi narsa |
|---|---|---|---|
| 0 | 0 | `ombor/00_salom` | `printf` bilan chiroyli jadval |
| 1 | 1 | `ombor/01_fayllar` | `.h` + `.c`, alohida kompilyatsiya |
| 2 | 2 | `ombor/02_turlar` | pul tiyinda (`long`), `uint16_t`, toshishdan himoya |
| 3 | 3 | `ombor/03_operatorlar` | chegirma, bit bayroqlari (holat) |
| 4 | 4 | `ombor/04_boshqaruv` | menyu (`while` + `switch`), `scanf` tekshiruvi |
| 5 | 5 | `ombor/05_funksiyalar` | funksiyalar, xato kodlari (`enum`) |
| 6 | 6 | `ombor/06_massivlar` | massivlar, satrlar, nom bo'yicha qidirish |
| 7 | 7 | `ombor/07_korsatkichlar` | ko'rsatkichlar: `sot(&zaxira)`, saralash, eng qimmat |
| 8 | 8 | `ombor/08_xotira` | `malloc`/`realloc`/`free`, sanitizer |
| 9 | 9 | `ombor/09_struct` | `struct`, `enum`, `typedef` |
| 10 | 10 | `ombor/10_makrolar` | makrolar, X-makro, `-DDEBUG` izlari |
| 11 | 11 | `ombor/11_kop_fayl` | ko'p fayl, opaque tur, `Makefile`, statik kutubxona |
| 12 | 12 | `ombor/12_stdlib` | faylga saqlash, `qsort`, `errno`, `assert` |

## Qanday ishlash kerak

1. Bobdagi "**Katta loyiha**" bo'limini o'qing (kod + tushuntirish + natija).
2. O'zingizga papka oching (`mkdir ~/ombor`) va kodni **o'zingiz yozing** (ko'chirmang!).
3. Natijani shu papkadagi tayyor variant bilan solishtiring.
4. Bo'lim oxiridagi **"O'zingiz qo'shing"** topshiriqlarini bajaring (yechimsiz, faqat maslahat).

## Tekshirish

Har bosqich papkasida: manba fayllar, `qur.txt` (yig'ish buyrug'i), `kirish.txt` (klaviatura o'rniga) va `kutilgan.txt` (kutilgan chiqish).
Hammasini yig'ib solishtirish: `python3 tools/katta_loyiha.py` (`make lab-check` ham shuni ishga tushiradi).
