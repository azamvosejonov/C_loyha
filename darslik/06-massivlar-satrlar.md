# 6-bob. Massivlar va satrlar

> **Bu bobdan keyin:** massiv xotirada qanday turishini, nega u o'z uzunligini bilmasligini,
> satr = `'\0'` bilan tugaydigan baytlar ekanini va bufer to'lishi (buffer overflow) qanday
> paydo bo'lishini bilasiz. Mashqlar: 05, 08, 09, 10, 11, 12.

## 6.1. Massiv

```c
int a[5] = { 10, 20, 30, 40, 50 };
```

Xotirada — **ketma-ket** 5 ta `int`, jami 20 bayt:

```text
manzil:  1000   1004   1008   1012   1016
         [ 10 ][ 20 ][ 30 ][ 40 ][ 50 ]
          a[0]  a[1]  a[2]  a[3]  a[4]
```

- Indeks **0 dan** boshlanadi: oxirgisi `a[4]`, `a[5]` — massivdan tashqari!
- O'lcham **qat'iy**: `a` ni 6 elementga "o'stirib" bo'lmaydi.
- `a[i]` ning manzili = `a` ning manzili + `i * sizeof(int)`. Shuning uchun murojaat O(1) va juda tez.

### Boshlash usullari

```c
int a[5];                       /* lokal bo'lsa - AXLAT qiymatlar */
int b[5] = { 0 };               /* hammasi 0 (qolganlari avtomatik 0) */
int c[] = { 1, 2, 3 };          /* o'lchamni kompilyator hisoblaydi: 3 */
int d[100] = { [10] = 5, [99] = 7 };    /* nomlangan indekslar, qolgani 0 */
```

### Massiv uzunligi

```c
size_t n = sizeof(a) / sizeof(a[0]);    /* 20 / 4 = 5 */
```

Bu **faqat massiv e'lon qilingan joyda** ishlaydi! Funksiyaga uzatilganda massiv ko'rsatkichga
aylanadi va `sizeof` ko'rsatkich hajmini (8) beradi:

```c
void f(int a[])                 /* aslida: int *a */
{
    size_t n = sizeof(a) / sizeof(a[0]);    /* 8 / 4 = 2 - XATO! */
}
```

GCC buni ogohlantiradi (`sizeof on array function parameter`). Shuning uchun C'da **massiv doim
uzunligi bilan birga uzatiladi**: `void f(const int *a, size_t n)`. Standart kutubxonada ham, yadroda
ham — `buf, len` juftligi hamma joyda (`read(fd, buf, len)`, `memcpy(dst, src, n)`).

## 6.2. Chegaradan chiqish — C'ning eng xavfli xatosi

```c
int a[5];
a[5] = 99;          /* massivdan tashqariga yozish */
a[-1] = 0;          /* bu ham */
```

C **tekshirmaydi**. Kompilyator ham (ko'pincha), ish vaqtida ham. Natija:
- yonidagi o'zgaruvchi jim buziladi;
- funksiyaning qaytish manzili ustidan yoziladi → dastur boshqa joyga "sakraydi";
- yoki hech narsa bo'lmaydi — bugun. Ertaga boshqa kompilyator bilan qulaydi.

Tarixdagi eng ko'p xavfsizlik hujumlari (Morris qurti, 1988-yildan beri) aynan shundan boshlangan:
foydalanuvchi bergan uzun satr stekdagi buferdan toshib, qaytish manzilini o'zgartiradi.

**Himoya:** `-fsanitize=address` (o'rganishda), har bir indeksni tekshirish, uzunlikni doim uzatish.
Yadroda sanitizer yo'q — faqat intizom.

## 6.3. Ko'p o'lchamli massivlar

```c
int m[3][4];                    /* 3 qator, 4 ustun */
m[1][2] = 7;
```

Xotirada — qatorma-qator ketma-ket (12 ta `int`): `m[1][2]` manzili = `m + (1 * 4 + 2) * 4`.
Katta matritsalarni qatorma-qator aylanish (`for i` tashqarida, `for j` ichkarida) keshga mos va tez.

## 6.4. Satr — `'\0'` bilan tugaydigan `char` massivi

C'da alohida "satr" turi **yo'q**. Satr — oxirida **nol bayt** (`'\0'`) bo'lgan `char` massivi:

```c
char s[] = "salom";
```

```text
s[0] s[1] s[2] s[3] s[4] s[5]
 's'  'a'  'l'  'o'  'm'  '\0'      -> jami 6 bayt!
```

- `sizeof(s)` = 6, `strlen(s)` = 5 (`'\0'` sanalmaydi).
- `'\0'` — son **0** (belgi `'0'` emas, u 48). Uni "terminator" deyiladi.
- Satr uzunligi hech qayerda saqlanmaydi — uni bilish uchun `'\0'` gacha **sanash** kerak (O(n)).
  Python satri esa uzunligini ichida saqlaydi.

**Savol: nega shunday qilingan?**
1970-yillarda xotira juda qimmat edi: 1 bayt terminator uzunlik maydonidan tejamliroq. Bugun bu
qaror "milliard dollarlik xato" deb ataladi — son-sanoqsiz bufer to'lishlari shu yerdan. Lekin C
va unga asoslangan barcha tizimlar (Linux syscall'lari ham) shunday ishlaydi, shuning uchun uni
mukammal bilishingiz shart.

### Satr literali va massiv farqi

```c
char s[] = "salom";         /* MASSIV: 6 baytli nusxa stekda - o'zgartirish mumkin */
char *p = "salom";          /* KO'RSATKICH: faqat o'qiladigan xotiradagi literalga */

s[0] = 'S';                 /* OK */
p[0] = 'S';                 /* UB - odatda Segmentation fault (literal .rodata da) */
```

Literalga ko'rsatkichni doim `const char *p = "salom";` deb yozing — shunda kompilyator `p[0] = ...` ni
darhol xato deydi.

## 6.5. `<string.h>` — asosiy funksiyalar

| Funksiya | Nima qiladi | Tuzoq |
|---|---|---|
| `strlen(s)` | `'\0'` gacha uzunlik | `'\0'` bo'lmasa — xotira bo'ylab "yuguradi" |
| `strcpy(d, s)` | s ni d ga nusxalash | **d ning hajmini tekshirmaydi** — bufer to'lishi. Ishlatmang |
| `strncpy(d, s, n)` | ko'pi bilan n bayt | s uzun bo'lsa, `'\0'` **qo'ymaydi**! Chalkash |
| `snprintf(d, n, "%s", s)` | xavfsiz nusxa/format | ✅ tavsiya etiladi |
| `strcmp(a, b)` | solishtirish: <0, 0, >0 | `a == b` satrlarni emas, **manzillarni** solishtiradi! |
| `strncmp(a, b, n)` | birinchi n bayt | prefiks tekshirish uchun |
| `strchr(s, c)` | c ning birinchi o'rni yoki NULL | |
| `strstr(s, t)` | t ning s ichidagi o'rni | |
| `memcpy(d, s, n)` | n baytni nusxalash | d va s ustma-ust tushmasligi kerak |
| `memmove(d, s, n)` | ustma-ust tushsa ham to'g'ri | |
| `memset(d, c, n)` | n baytni c bilan to'ldirish | |
| `memcmp(a, b, n)` | n baytni solishtirish | |

**`mem*` va `str*` farqi:** `str*` — `'\0'` gacha ishlaydi (matn uchun). `mem*` — aniq n bayt
(har qanday ma'lumot: struct, rasm, disk sektori). Yadroda `mem*` ko'proq ishlatiladi.

MyOS'da bularning hammasi **o'zimiz yozgan**: `user/libc/string.c` (user dasturlar uchun) va
`kernel/lib/string.c` (yadro uchun). 08-mashqda ulardan uchtasini o'zingiz yozasiz, lab'larda esa
yadrodagisini.

## 6.6. Satrni belgima-belgi aylanish

```c
size_t unli_soni(const char *s)
{
    size_t n = 0;
    for (size_t i = 0; s[i] != '\0'; i++)       /* yoki: s[i] */
        if (strchr("aeiouAEIOU", s[i]))
            n++;
    return n;
}
```

Ko'rsatkich bilan (7-bobdan keyin shunday yozasiz):

```c
for (const char *p = s; *p; p++)
    ...
```

## 6.7. `<ctype.h>` — belgilarni tekshirish

`isdigit(c)`, `isalpha(c)`, `isalnum(c)`, `isspace(c)`, `isupper(c)`, `tolower(c)`, `toupper(c)`.

**Tuzoq:** ularga faqat `unsigned char` qiymati yoki `EOF` berish mumkin. x86'da `char` ishorali:
UTF-8 dagi `'é'` baytlari (0xC3 0xA9) manfiy son bo'lib qoladi → **UB**. To'g'ri yozuv:

```c
if (isalpha((unsigned char)s[i]))
```

(10-mashq aynan shuni tekshiradi.)

## 6.8. Satrlarni solishtirish va nusxalash — to'g'ri usullar

```c
if (strcmp(buyruq, "exit") == 0) { ... }        /* teng */

char buf[64];
snprintf(buf, sizeof(buf), "%s/%s", papka, fayl);   /* hech qachon 64 dan oshmaydi */
```

`snprintf` qaytaradi: **to'liq natija uchun kerak bo'lgan** uzunlikni. Agar u `>= sizeof(buf)` bo'lsa —
natija qisqartirilgan. Bu "joy — n, lekin haqiqiy kerakli sonni qaytaraman" uslubi (09-mashq,
BSD `strlcpy`) C'ning xavfsiz satr funksiyalari uchun standart.

## 6.9. Satrni o'qish (klaviaturadan)

```c
char qator[256];
if (fgets(qator, sizeof(qator), stdin)) {       /* ko'pi bilan 255 belgi + '\0' */
    qator[strcspn(qator, "\n")] = '\0';          /* oxiridagi '\n' ni olib tashlash */
}
```

**Hech qachon `gets` ishlatmang** — u bufer hajmini bilmaydi va standartdan olib tashlangan.
`scanf("%s", buf)` ham xuddi shunday xavfli (kenglik ko'rsatilmasa).

## 6.10. Satr ↔ son

```c
long x = strtol("123", &end, 10);   /* satr -> son, xatolarni tekshirish mumkin */
int y = atoi("123");                /* oddiy, lekin xatoni bildirmaydi: atoi("abc") == 0 */
snprintf(buf, sizeof(buf), "%d", 42);   /* son -> satr */
```

11 va 12-mashqlarda bularni **o'zingiz** yozasiz — `printf` ning yuragi (`kernel/lib/kprintf.c`)
aynan shunday ishlaydi.

## 6.11. Savol-javob

**`char s[10] = "salom";` — qolgan 4 bayt nima?**
Nol. Boshlang'ich qiymat qisman berilsa, qolgani 0 bilan to'ldiriladi.

**`char s[5] = "salom";` — xato bo'ladimi?**
C'da **yo'q** (C++'da xato): 5 ta harf sig'adi, lekin `'\0'` uchun joy yo'q — `s` satr emas, oddiy
massiv bo'lib qoladi. `strlen(s)` xotira bo'ylab yuguradi. Klassik jim xato.

**UTF-8 (o'zbekcha `o'`, `g'`, kirill) qanday saqlanadi?**
Lotin o'zbek alifbosi ASCII'da (apostrof ham). Kirill yoki `é` — bir belgi 2–4 bayt. `strlen`
**baytlarni** sanaydi, belgilarni emas: `strlen("дом")` = 6. MyOS terminal emulyatori UTF-8 ni
qanday dekodlashini `kernel/drivers/vt.c` da ko'rishingiz mumkin.

## 6.12. O'zingizni tekshiring

1. `int a[10];` — `a[10]` ga yozish nima?
2. `char s[] = "abc";` — `sizeof(s)` va `strlen(s)`?
3. `if (s == "exit")` nima uchun deyarli doim yolg'on?
4. Funksiya ichida massiv uzunligini `sizeof` bilan bilsa bo'ladimi?
5. `strcpy` o'rniga nimani ishlatish kerak?

<details><summary>Javoblar</summary>

1. Chegaradan tashqariga yozish — UB (indekslar 0..9).
2. 4 va 3.
3. Ikki manzil solishtiriladi (massiv va literal manzili), mazmun emas. `strcmp(s, "exit") == 0` kerak.
4. Yo'q — parametr ko'rsatkichga aylanadi; uzunlikni alohida uzating.
5. `snprintf(d, sizeof(d), "%s", s)` yoki o'z `strlcpy` (09-mashq).
</details>

## 6.13. Mashqlar

- **05** — massivlar. **08** — `strlen`, `strchr`, `strcmp` ni o'zingiz yozish.
- **09** — xavfsiz nusxalash. **10** — ikki ko'rsatkich usuli.
- **11** — son → satr. **12** — satr → son.

Keyingi bob: [7-bob. Ko'rsatkichlar](07-korsatkichlar.md)
