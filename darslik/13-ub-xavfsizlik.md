# 13-bob. Aniqlanmagan xatti-harakat (UB) va xavfsiz kod

> **Bu bobdan keyin:** "undefined behavior" nima ekanini, nega u shunchaki "xato natija" emas, balki
> kompilyator kodingizni **o'zgartirishiga** sabab bo'lishini, eng ko'p uchraydigan UB turlarini va
> ulardan himoyalanishni bilasiz. Yadro muhandisi uchun bu bob majburiy. Mashqlar: 01, 03, 04, 09, 12, 18.

## 13.1. UB nima

C standarti ba'zi holatlar uchun natijani **aniqlamaydi**: "bunday qilmang — agar qilsangiz, har qanday
narsa bo'lishi mumkin". Masalan:

- ishorali butun sonning toshishi;
- massiv chegarasidan tashqariga murojaat;
- NULL yoki `free` qilingan ko'rsatkichni dereference qilish;
- boshlang'ich qiymatsiz o'zgaruvchini o'qish;
- `1 << 32` (surish tur kengligidan katta), nolga bo'lish;
- bitta ifodada bir o'zgaruvchini ikki marta o'zgartirish (`i = i++`).

Nega standart shunday? Tezlik uchun: har bir qo'shishdan keyin toshishni, har bir `a[i]` da chegarani
tekshirish — sekin. C "dasturchi xato qilmaydi" deb faraz qiladi va buning evaziga tez kod beradi.

## 13.2. UB — "noto'g'ri natija" emas, "hamma narsa mumkin"

Eng muhim tushuncha: kompilyator **"UB hech qachon sodir bo'lmaydi"** deb faraz qiladi va shu farazga
tayanib optimallashtiradi. Natijada kod siz o'ylagandan butunlay boshqacha ishlaydi:

```c
int toshadimi(int x)
{
    if (x + 1 < x)          /* "x + 1 toshsa, kichikroq bo'ladi" degan fikr */
        return 1;
    return 0;
}
```

GCC `-O2` bilan bu funksiyani **doim `return 0`** ga aylantiradi: "ishorali toshish UB, demak u
bo'lmaydi, demak `x + 1 < x` hech qachon rost emas". Tekshiruvingiz **yo'q qilindi**.

Yana bir mashhur misol (Linux yadrosida haqiqatan bo'lgan xato, 2009):

```c
struct sock *sk = tun->sk;      /* 1) tun ni dereference qilish */
if (!tun)                       /* 2) tun NULL-mi? */
    return POLLERR;
```

Kompilyator: "1-qatorda `tun` dereference qilindi — agar u NULL bo'lganida UB bo'lardi; UB bo'lmaydi,
demak `tun` NULL emas, demak 2-qatordagi tekshiruv keraksiz" — va uni **o'chirib tashladi**. Natija —
yadroda ekspluatatsiya qilinadigan teshik. Shundan keyin Linux `-fno-delete-null-pointer-checks` bilan
yig'iladigan bo'ldi.

**Xulosa:** UB bo'lgan dastur "bugun ishlayapti" bo'lishi mumkin — boshqa kompilyator versiyasi,
boshqa `-O` darajasi yoki qo'shni kodni o'zgartirish bilan u buziladi.

## 13.3. Eng ko'p uchraydigan UB va to'g'ri yozuvlar

| UB | Xato | To'g'ri |
|---|---|---|
| Ishorali toshish | `if (a + b > MAX)` | `if (a > MAX - b)` — hisoblashdan **oldin** tekshirish (03-mashq) |
| Katta ko'paytma | `int * int` natija `long` ga | `(long)a * b` (01-mashq) |
| `-INT_MIN`, `INT_MIN / -1` | | oldindan tekshirish (20-mashq) |
| Surish | `1 << 31`, `x << 32` | `1u << 31`, `1ull << 40`, surishni `< kenglik` bilan cheklash (04) |
| Chegara | `a[n]`, `strcpy` | uzunlikni uzatish, `snprintf`, chegara tekshiruvi (09) |
| NULL | `p->x` tekshiruvsiz | `if (!p) return -EINVAL;` |
| Use-after-free | `free(p); p->x` | `free(p); p = NULL;`, egalik qoidalari (8-bob) |
| Boshlanmagan | `int s; s += x;` | `int s = 0;` |
| Tekislanmagan murojaat | `*(uint32_t *)(buf + 1)` | `memcpy(&v, buf + 1, 4)` |
| Strict aliasing | `*(float *)&int_ozgaruvchi` | `memcpy` yoki `union` |
| Tartibsiz yon ta'sir | `a[i] = i++;` | ikki qatorga bo'lish |
| `char` ni ctype'ga | `isalpha(c)` (c manfiy) | `isalpha((unsigned char)c)` (10) |

### Strict aliasing — qisqacha

Kompilyator "turli turdagi ko'rsatkichlar bir xil xotiraga ko'rsatmaydi" deb faraz qiladi
(`char *` bundan mustasno). `int` ni `float *` orqali o'qish — UB va optimallashtirishda noto'g'ri kod.
Bir xil baytlarni boshqa tur sifatida o'qishning to'g'ri yo'llari — `memcpy` (kompilyator uni bitta
buyruqqa aylantiradi) yoki `union`. Linux yadrosi `-fno-strict-aliasing` bilan yig'iladi — tarmoq va disk
kodi bu qoidani juda ko'p buzadi.

## 13.4. Himoya vositalari

| Vosita | Nima qiladi | Qachon |
|---|---|---|
| `-Wall -Wextra -Werror` | kompilyatsiya paytidagi ogohlantirishlar | **doim** |
| `-fsanitize=undefined` | UB ni ish vaqtida ushlaydi | testlarda, o'rganishda |
| `-fsanitize=address` | xotira xatolari | testlarda |
| `-fsanitize=thread` | poyga holatlari (15-bob) | ko'p oqimli kod |
| `valgrind` | xotira xatolari, qayta kompilyatsiyasiz | |
| statik analizatorlar (`gcc -fanalyzer`, `clang --analyze`, `cppcheck`) | kodni ishga tushirmasdan tahlil | |
| `_Static_assert` | kompilyatsiya paytidagi tekshiruv | tuzilma hajmlari |
| Testlar | chegaraviy holatlar | **doim** |

Yadroda ham analoglari bor: Linux'da KASAN (AddressSanitizer yadro uchun), UBSAN, KCSAN (poygalar),
lockdep (qulflar tartibi). MyOS'da soddalari: slab'dagi redzone va zahar, double-free tekshiruvi,
himoya sahifalari (`kernel/mm/slab.c`, `vmalloc.c`).

## 13.5. Xavfsiz kod yozish qoidalari (yadro uslubida)

1. **Tashqaridan kelgan hamma narsaga ishonmang.** Foydalanuvchi bergan uzunlik, indeks, ko'rsatkich,
   diskdan o'qilgan tuzilma, tarmoq paketi — hammasi tekshiriladi. MyOS: syscall'lardagi `copy_from_user`
   foydalanuvchi ko'rsatkichi user hududida ekanini tekshiradi (`kernel/sys/uaccess.c`), ext2 drayveri
   superblokdagi qiymatlarni tekshiradi (`kernel/fs/ext2.c` → `ext2_mount`).
2. **Hisoblashdan oldin tekshiring**, keyin emas (toshish).
3. **Uzunlikni doim olib yuring** — `buf, len`.
4. **Har bir xato yo'lini sinang** — `malloc` NULL qaytarsa nima bo'ladi? Xatodan keyin hamma resurslar
   bo'shatildimi (`goto` tozalash)?
5. **Ogohlantirishlarni hech qachon o'chirmang** — sababini tushuning va tuzating.
6. **Oddiy yozing.** Aqlli hiyla — kelajakdagi xato.

## 13.6. Implementation-defined va unspecified

UB dan tashqari yana ikki toifa bor:

- **Implementation-defined** — kompilyator tanlaydi, lekin **hujjatlashtiradi** va doim bir xil:
  `char` ishorali-mi, `int` hajmi, manfiy sonni `>>` qilish. Portativ kodda tayanmang, lekin UB emas.
- **Unspecified** — bir nechta variantdan biri, har safar boshqacha bo'lishi mumkin: funksiya
  argumentlarining hisoblanish tartibi (`f(a(), b())` — qaysi biri oldin?).

## 13.7. O'zingizni tekshiring

1. Nega `if (x + 1 < x)` toshishni tekshirish uchun yaroqsiz?
2. `unsigned` toshish UB mi?
3. `int *p = ...; int v = *p; if (!p) return;` da nima xato?
4. Bayt massividan `uint32_t` ni xavfsiz qanday o'qiysiz?
5. UB bo'lgan dastur testlardan o'tsa, u to'g'rimi?

<details><summary>Javoblar</summary>

1. Ishorali toshish UB; kompilyator shartni doim yolg'on deb olib tashlashi mumkin.
2. Yo'q — ishorasiz arifmetika modulli va aniqlangan.
3. Tekshiruv dereference'dan keyin; kompilyator uni o'chirishi mumkin. Tekshiruv oldin bo'lishi kerak.
4. `uint32_t v; memcpy(&v, buf + i, sizeof(v));`
5. Yo'q — boshqa kompilyator, optimallashtirish yoki kontekstda buziladi.
</details>

## 13.8. Mashqlar

- **01, 03, 04** — toshish va surish UB'lari (sanitizer ushlaydi).
- **09, 12** — tashqi kirishni qat'iy tekshirish.
- **18** — xotira UB'larini hisobotdan topish.
- Qo'shimcha: 13.2-dagi `toshadimi` funksiyasini `gcc -O0` va `gcc -O2` bilan kompilyatsiya qilib,
  `toshadimi(INT_MAX)` natijasini solishtiring. Keyin `gcc -O2 -S` bilan assembly'ni ko'ring.

Keyingi bob: [14-bob. Tizim chaqiruvlari: fayllar va jarayonlar](14-tizim-chaqiruvlari.md)
