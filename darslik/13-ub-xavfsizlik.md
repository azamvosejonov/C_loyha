# 13-bob. Aniqlanmagan xatti-harakat (UB) va xavfsiz kod

> **Bu bobda nima o'rganasiz:** "undefined behavior" (UB, aniqlanmagan xatti-harakat) nima ekanini; nega u shunchaki "noto'g'ri natija" emas, balki kompilyator kodingizni
> **o'zgartirib yuborishiga** sabab bo'lishini; eng ko'p uchraydigan UB turlarini; ulardan qanday himoyalanishni.
> **Oldindan nima kerak:** 2-, 3-, 6-, 7-, 8-boblar.   **Vaqt:** 5–6 soat.
> Yadro muhandisi uchun bu bob majburiy. Mashqlar: 01, 03, 04, 09, 12, 18.

> **To'liq ishlaydigan misol:** [misollar/13_ub.c](misollar/13_ub.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Bu bob nima haqida?

Oldingi boblarda "bu UB" degan so'zni ko'p ko'rdingiz: ishorali toshish, massivdan chiqish, `NULL` ni o'qish. Bu bobda **UB nima ekanini** va **nega u eng xavfli** ekanini ochamiz.

**Hayotdan misol: sug'urta shartnomasidagi istisno.** Avtomobil sug'urtasida yozilgan: "Mast holda haydaganda yuz bergan hodisalar uchun kompaniya **hech qanday** javobgarlikni o'z zimmasiga olmaydi".
Bu "kamroq to'laymiz" degani emas — **umuman hech narsa kafolatlanmaydi**. C standarti ham shunday: "massiv chegarasidan chiqsangiz, ishorali son toshsa, NULL ni o'qisangiz — nima bo'lishi haqida hech narsa
va'da qilmayman". Dastur ishlashi ham, qulashi ham, jim turib noto'g'ri natija berishi ham mumkin.

| Sug'urtada | C'da |
|---|---|
| "qoidani buzdingiz" (mast haydash) | UB sodir bo'ldi (toshish, chegaradan chiqish...) |
| "kompaniya hech narsa kafolatlamaydi" | dastur nima qilishi **noma'lum** |
| ehtiyotkor haydovchi | xavfsiz kod: UB ni **oldindan** oldini olish |

## 13.1. UB nima

C standarti ba'zi holatlar uchun natijani **aniqlamaydi**: "bunday qilmang — agar qilsangiz, har qanday narsa bo'lishi mumkin". Masalan:

- ishorali butun sonning toshishi;
- massiv chegarasidan tashqariga murojaat;
- `NULL` yoki `free` qilingan ko'rsatkichni dereference qilish;
- boshlang'ich qiymatsiz o'zgaruvchini o'qish;
- `1 << 32` (surish tur kengligidan katta), nolga bo'lish;
- bitta ifodada bir o'zgaruvchini ikki marta o'zgartirish (`i = i++`).

**Nega standart shunday?** Tezlik uchun: har bir qo'shishdan keyin toshishni, har bir `a[i]` da chegarani tekshirish — sekin. C "dasturchi xato qilmaydi" deb faraz qiladi va buning evaziga
tez kod beradi.

### Sanitizer bilan UB ni "ko'rish"

UB odatda **jim** o'tadi. Uni ko'rish uchun `-fsanitize=undefined` (UBSan) bilan yig'ish mumkin. Quyidagi dastur buyruq qatoridagi raqamga qarab **turli UB**ni sodir qiladi:

```c
/* ub_katalog.c - turli UB lar: raqam bilan tanlanadi */
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int tanlov = argc > 1 ? atoi(argv[1]) : 0;
    int bir = argc;                         /* ish vaqtida 2 (argc = 2): kompilyator oldindan bilmaydi */

    switch (tanlov) {
    case 1: {
        int a = INT_MAX;
        a = a + bir;                        /* 1) ishorali toshish */
        printf("%d\n", a);
        break;
    }
    case 2: {
        int n = 30 + bir;                   /* n = 32 */
        unsigned v = 1u << n;               /* 2) surish miqdori kenglikka teng */
        printf("%u\n", v);
        break;
    }
    case 3: {
        int m = INT_MIN;
        int d = -(bir - 1);                 /* d = -1 */
        printf("%d\n", m / d);              /* 3) INT_MIN / -1 */
        break;
    }
    case 4: {
        unsigned char buf[16] = { 0 };
        uint32_t *p = (uint32_t *)(buf + bir - 1);   /* buf + 1: tekislanmagan */
        printf("%u\n", *p);                 /* 4) tekislanmagan murojaat */
        break;
    }
    case 5: {
        int nol = bir - 2;                  /* nol */
        printf("%d\n", 10 / nol);           /* 5) nolga bo'lish */
        break;
    }
    }
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g -fsanitize=undefined ub_katalog.c -o ub_katalog
$ ./ub_katalog 1 2>&1 | grep 'runtime error' | sed -E 's/0x[0-9a-f]+/0x.../'
ub_katalog.c:15:11: runtime error: signed integer overflow: 2147483647 + 2 cannot be represented in type 'int'
$ ./ub_katalog 2 2>&1 | grep 'runtime error' | sed -E 's/0x[0-9a-f]+/0x.../'
ub_katalog.c:21:25: runtime error: shift exponent 32 is too large for 32-bit type 'unsigned int'
$ ./ub_katalog 3 2>&1 | grep 'runtime error' | sed -E 's/0x[0-9a-f]+/0x.../'
ub_katalog.c:28:26: runtime error: division of -2147483648 by -1 cannot be represented in type 'int'
$ ./ub_katalog 4 2>&1 | grep 'runtime error' | sed -E 's/0x[0-9a-f]+/0x.../'
ub_katalog.c:34:9: runtime error: load of misaligned address 0x... for type 'uint32_t', which requires 4 byte alignment
$ ./ub_katalog 5 2>&1 | grep 'runtime error' | sed -E 's/0x[0-9a-f]+/0x.../'
ub_katalog.c:39:27: runtime error: division by zero
```

**Bu dastur nima qiladi (umumiy):** `./ub_katalog 1` ishga tushirilganda birinchi argument (`1`) qaysi UB bajarilishini tanlaydi. Bunda `argc` = 2 bo'ladi (dastur nomi + `1`); dastur shu qiymatdan foydalanib UB sharoitini ish vaqtida yasaydi.
Sanitizer har birini **nomi, qatori va qiymatlari bilan** ushlaydi. Oddiy yig'ishda hech qanday xabar bo'lmasdi.

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `int tanlov = atoi(argv[1])` | qaysi UB ni sinash — buyruq qatoridan |
| `int bir = argc` | **ish vaqtida** ma'lum qiymat (2); shuning uchun kompilyator UB ni oldindan ko'rib, o'zgartirib yubormaydi |
| `switch (tanlov)` | beshta holatdan birini bajarish |
| 1–5 | har biri bitta UB: toshish, surish, `INT_MIN / -1`, tekislanmagan o'qish, nolga bo'lish |

## 13.2. UB — "noto'g'ri natija" emas, "hamma narsa mumkin"

Eng muhim tushuncha: kompilyator **"UB hech qachon sodir bo'lmaydi"** deb faraz qiladi va shu farazga tayanib optimallashtiradi. Natijada kod siz o'ylagandan butunlay boshqacha ishlaydi.

**Hayotdan misol: yashil chiroqdagi haydovchi.** Yashil chiroqda haydovchi chorrahaga sekinlamasdan kiradi: "qoida bo'yicha boshqalar to'xtagan". Kimdir qizilda o'tsa — halokat.
Kompilyator ham "dasturchi UB qilmaydi" deb ishonadi va shunga qarab tezlashtiradi.

### Misol 1: toshishni tekshirish yo'qoladi

```c
/* toshadimi.c - "x + 1 < x" tekshiruvi */
#include <limits.h>
#include <stdio.h>

int toshadimi(int x)
{
    if (x + 1 < x)          /* "x + 1 toshsa, kichikroq bo'ladi" degan fikr */
        return 1;
    return 0;
}

int main(void)
{
    volatile int v = INT_MAX;               /* qiymat ish vaqtida */
    printf("toshadimi(INT_MAX) = %d\n", toshadimi(v));
    return 0;
}
```

```console
$ gcc -O2 toshadimi.c -o toshadimi_O2
$ ./toshadimi_O2
toshadimi(INT_MAX) = 0
$ gcc -O2 -fwrapv toshadimi.c -o toshadimi_wrapv
$ ./toshadimi_wrapv
toshadimi(INT_MAX) = 1
```

**Bir xil kod, ikki xil javob!** Birinchi qatorda kompilyator UB ga ishondi (`0`). Ikkinchisida `-fwrapv` bayrog'i "ishorali toshish **aylanadi**, UB emas" deydi — shunda `INT_MAX + 1` haqiqatan kichikroq bo'ladi
va natija `1`. Demak, natija kodga emas, **kompilyatorning farazlariga** bog'liq. Assembly'ga qaraymiz:

```console
$ gcc -O2 -S -o - toshadimi.c | sed -n '/^toshadimi:/,/ret/p' | grep -v '^\s*[.]'
toshadimi:
	endbr64
	xorl	%eax, %eax
	ret
```

Funksiya tanasi — faqat `xorl %eax, %eax` (natija = 0) va `ret`. GCC `-O2` bilan bu funksiyani **doim `return 0`** ga aylantirdi: "ishorali toshish UB, demak u bo'lmaydi, demak `x + 1 < x` hech qachon rost emas".
Tekshiruvingiz **yo'q qilindi**.

### Misol 2: NULL tekshiruvi yo'qoladi (Linux yadrosida haqiqatan bo'lgan xato, 2009)

```text
struct sock *sk = tun->sk;      /* 1) tun ni dereference qilish */
if (!tun)                       /* 2) tun NULL-mi? */
    return POLLERR;
```

Kompilyator: "1-qatorda `tun` dereference qilindi — agar u NULL bo'lganida UB bo'lardi; UB bo'lmaydi, demak `tun` NULL emas, demak 2-qatordagi tekshiruv keraksiz" — va uni **o'chirib tashladi**.
Natija — yadroda ekspluatatsiya qilinadigan teshik. Shuni kichik dasturda ko'ramiz:

```c
/* null_optim.c - dereference'dan keyingi NULL tekshiruvi */
int oqi(int *p)
{
    int v = *p;             /* 1) avval o'qiymiz */
    if (!p)                 /* 2) keyin NULL-mi deb tekshiramiz (kech!) */
        return -1;
    return v;
}
```

```console
$ gcc -O2 -S -o - null_optim.c | sed -n '/^oqi:/,/ret/p' | grep -v '^\s*[.]'
oqi:
	endbr64
	movl	(%rdi), %eax
	ret
$ gcc -O2 -fno-delete-null-pointer-checks -S -o - null_optim.c | sed -n '/^oqi:/,/ret/p' | grep -v '^\s*[.]'
oqi:
	endbr64
	testq	%rdi, %rdi
	movl	$-1, %eax
	cmovne	(%rdi), %eax
	ret
```

Birinchi natijada **hech qanday tekshiruv yo'q** (faqat `movl (%rdi), %eax` va `ret`): `if (!p)` o'chirib tashlangan. Ikkinchisida — `testq %rdi, %rdi` (NULL tekshiruvi) saqlangan.
Shundan keyin Linux `-fno-delete-null-pointer-checks` bilan yig'iladigan bo'ldi.

**Xulosa:** UB bo'lgan dastur "bugun ishlayapti" bo'lishi mumkin — boshqa kompilyator versiyasi, boshqa `-O` darajasi yoki qo'shni kodni o'zgartirish bilan u buziladi.

> **Eslab qoling:** kompilyator UB sodir bo'lmaydi deb **ishonadi**. Shuning uchun UB dan keyin yozilgan himoya (tekshiruv) **yo'q qilinishi** mumkin. Tekshiruv — **hisoblashdan oldin**.

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

**Hayotdan misol: ko'prikka chiqishdan oldin yuk og'irligini tekshirish.** Ko'prik 10 tonnaga chidaydi. Yuk mashinasi **ko'prikka chiqqandan keyin** ko'prik buzilganmi deb tekshirish befoyda.
Toshishni ham **hisoblashdan oldin** tekshirasiz: `if (a > INT_MAX - b)` — "qo'shsam sig'adimi?".

### To'g'ri yo'l: tekislanmagan o'qishni `memcpy` bilan

```c
/* memcpy_oqish.c - bayt massividan uint32_t xavfsiz o'qish */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    unsigned char buf[8] = { 0xAA, 0x78, 0x56, 0x34, 0x12, 0xBB, 0, 0 };

    uint32_t v;
    memcpy(&v, buf + 1, sizeof(v));         /* tekislanishdan qat'i nazar xavfsiz */
    printf("buf[1..4] -> 0x%08x\n", v);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -fsanitize=undefined memcpy_oqish.c -o memcpy_oqish
$ ./memcpy_oqish
buf[1..4] -> 0x12345678
```

**Qismlar:** `buf + 1` — 1-baytdan boshlab (tekislanmagan manzil). `memcpy` — 4 baytni `v` ga **baytma-bayt** ko'chiradi, shuning uchun UB yo'q. (Kompilyator uni bitta buyruqqa aylantiradi — tezlik yo'qolmaydi.)
Natijada `0x12345678`: x86 da kichik bayt oldin (`78 56 34 12`) — 16-bobda ko'ramiz.

### Strict aliasing — qisqacha

Kompilyator "turli turdagi ko'rsatkichlar bir xil xotiraga ko'rsatmaydi" deb faraz qiladi (`char *` bundan mustasno). `int` ni `float *` orqali o'qish — UB va optimallashtirishda noto'g'ri kod.
Bir xil baytlarni boshqa tur sifatida o'qishning to'g'ri yo'llari — `memcpy` (kompilyator uni bitta buyruqqa aylantiradi) yoki `union` (9.5). Linux yadrosi `-fno-strict-aliasing` bilan yig'iladi —
tarmoq va disk kodi bu qoidani juda ko'p buzadi.

## 13.4. Himoya vositalari

**Hayotdan misol: videoregistrator (sanitizer).** Halokat bo'lganda aniq nima bo'lganini ko'rsatadi: qaysi qator, qaysi qiymat. Mashinani biroz sekinlashtiradi, lekin o'rganish va sinov paytida bebaho.

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

Yadroda ham analoglari bor: Linux'da KASAN (AddressSanitizer yadro uchun), UBSAN, KCSAN (poygalar), lockdep (qulflar tartibi). MyOS'da soddalari: slab'dagi redzone va zahar, double-free tekshiruvi,
himoya sahifalari (`kernel/mm/slab.c`, `vmalloc.c`).

## 13.5. Xavfsiz kod yozish qoidalari (yadro uslubida)

1. **Tashqaridan kelgan hamma narsaga ishonmang.** Foydalanuvchi bergan uzunlik, indeks, ko'rsatkich, diskdan o'qilgan tuzilma, tarmoq paketi — hammasi tekshiriladi. MyOS: syscall'lardagi
   `copy_from_user` foydalanuvchi ko'rsatkichi user hududida ekanini tekshiradi (`kernel/sys/uaccess.c`), ext2 drayveri superblokdagi qiymatlarni tekshiradi (`kernel/fs/ext2.c` → `ext2_mount`).
2. **Hisoblashdan oldin tekshiring**, keyin emas (toshish).
3. **Uzunlikni doim olib yuring** — `buf, len`.
4. **Har bir xato yo'lini sinang** — `malloc` NULL qaytarsa nima bo'ladi? Xatodan keyin hamma resurslar bo'shatildimi (`goto` tozalash)?
5. **Ogohlantirishlarni hech qachon o'chirmang** — sababini tushuning va tuzating.
6. **Oddiy yozing.** Aqlli hiyla — kelajakdagi xato.

## 13.6. Implementation-defined va unspecified

**Hayotdan misol: mamlakatning yo'l qoidasi.** Qaysi tomondan yurish kerak: o'ngdanmi, chapdanmi? Har bir davlat o'zi belgilaydi, lekin **aniq belgilaydi va hujjatlaydi**. `char` ishorali yoki ishorasiz
ekani ham shunday — kompilyator tanlaydi va hujjatlaydi. Bu UB emas.

UB dan tashqari yana ikki toifa bor:

- **Implementation-defined** — kompilyator tanlaydi, lekin **hujjatlashtiradi** va doim bir xil: `char` ishorali-mi, `int` hajmi, manfiy sonni `>>` qilish. Portativ kodda tayanmang, lekin UB emas.
- **Unspecified** — bir nechta variantdan biri, har safar boshqacha bo'lishi mumkin: funksiya argumentlarining hisoblanish tartibi (`f(a(), b())` — qaysi biri oldin?).

```c
/* aniqlanmagan_tartib.c - implementation-defined va unspecified */
#include <stdio.h>

static int a(void) { printf("a chaqirildi\n"); return 1; }
static int b(void) { printf("b chaqirildi\n"); return 2; }

static int qosh(int x, int y) { return x + y; }

int main(void)
{
    char c = (char)200;                 /* char ishorali bo'lsa -56, ishorasiz bo'lsa 200 */
    printf("(char)200 = %d  (bu kompilyatorda; ARM da 200 bo'lardi)\n", c);

    printf("natija: %d\n", qosh(a(), b()));     /* qaysi biri oldin hisoblanadi? */
    return 0;
}
```

```console
$ gcc -Wall -Wextra aniqlanmagan_tartib.c -o aniqlanmagan_tartib
$ ./aniqlanmagan_tartib
(char)200 = -56  (bu kompilyatorda; ARM da 200 bo'lardi)
b chaqirildi
a chaqirildi
natija: 3
```

**Nima ko'rdik:** bu kompilyatorda `b` **oldin**, `a` keyin chaqirildi (o'ngdan chapga). Boshqa kompilyatorda teskari bo'lishi mumkin — standart tartibni **belgilamagan** (unspecified).
Dasturingiz shu tartibga tayanmasligi kerak: chaqiruvlarni alohida qatorlarga ajrating (`int x = a(); int y = b();`).

## Hayotdan misol va to'liq dastur

**Xavfsiz bank o'tkazmasi.** Hisobga pul qo'shamiz; `int` chegarasi (≈ 2.1 mlrd) yaqin. Yig'indi sig'maydigan bo'lsa — oldindan sezib, **rad etamiz** (UB siz). Uch xil to'g'ri usul bir dasturda.

```c
/* otkazma.c - toshishni OLDINDAN tekshirish: uch xil to'g'ri usul */
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>

/* 1-usul: qo'shishdan OLDIN chegarani tekshirish */
static bool qosh_xavfsiz(int balans, int summa, int *natija)
{
    if (summa > 0 && balans > INT_MAX - summa)
        return false;                           /* sig'maydi */
    if (summa < 0 && balans < INT_MIN - summa)
        return false;
    *natija = balans + summa;
    return true;
}

int main(void)
{
    int balans = 2000000000;                    /* 2 mlrd - int chegarasiga yaqin */
    int kelgan[] = { 100000000, 100000000, 100000000 };

    printf("1-usul (oldindan tekshirish):\n");
    for (int i = 0; i < 3; i++) {
        int yangi;
        if (qosh_xavfsiz(balans, kelgan[i], &yangi)) {
            balans = yangi;
            printf("  +%d -> balans %d\n", kelgan[i], balans);
        } else {
            printf("  +%d RAD ETILDI: hisob chegarasidan oshadi\n", kelgan[i]);
        }
    }

    /* 2-usul: kompilyatorning o'rnatilgan funksiyasi (Linux yadrosi check_add_overflow shu) */
    int natija;
    if (__builtin_add_overflow(balans, 500000000, &natija))
        printf("2-usul: %d + 500000000 toshadi - rad etildi\n", balans);

    /* 3-usul: kattaroq tur bilan hisoblash */
    long long katta = (long long)balans + 500000000;
    printf("3-usul: long long bilan aniq natija %lld (int ga sig'maydi: %s)\n",
           katta, katta > INT_MAX ? "ha" : "yo'q");
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 -fsanitize=undefined otkazma.c -o otkazma
$ ./otkazma
1-usul (oldindan tekshirish):
  +100000000 -> balans 2100000000
  +100000000 RAD ETILDI: hisob chegarasidan oshadi
  +100000000 RAD ETILDI: hisob chegarasidan oshadi
2-usul: 2100000000 + 500000000 toshadi - rad etildi
3-usul: long long bilan aniq natija 2600000000 (int ga sig'maydi: ha)
```

**Bu dastur nima qiladi (umumiy):** hisobda 2 000 000 000 so'm bor. Uch marta 100 000 000 qo'shishga urinadi: birinchisi sig'adi (2.1 mlrd < 2 147 483 647), keyingilari yo'q — rad etiladi. Keyin
yana ikki usul bilan toshishni oldindan aniqlashni ko'rsatadi.

**Qismlar:**

| Qism | Vazifasi | Tafsilot |
|---|---|---|
| `qosh_xavfsiz(balans, summa, &natija)` | qo'shishdan **oldin** "sig'adimi?" ni tekshiradi | `balans > INT_MAX - summa` — qo'shsak chegaradan oshadimi? Hech qanday toshish ro'y bermaydi, chunki ayirma **sig'adi** |
| `bool` qaytishi | `true` — qo'shildi, `false` — rad etildi | natija `*natija` ga yoziladi (chiqish parametri, 5.5) |
| `__builtin_add_overflow(a, b, &r)` | GCC: qo'shadi va toshgan bo'lsa **`true`** qaytaradi | Linux yadrosining `check_add_overflow` shu |
| `(long long)balans + ...` | kattaroq turda hisoblash | 64 bit sig'adi (2.5) |

`-fsanitize=undefined` bilan yig'ildi, lekin sanitizer bitta ham xabar bermadi: hech qayerda UB yo'q.

**Sinab ko'ring:** `qosh_xavfsiz` ichidagi ikkala `if` ni o'chirib, faqat `*natija = balans + summa;` qoldiring. Dasturni `-fsanitize=undefined` bilan ishga tushiring — sanitizer qaysi qatorni ko'rsatadi?

## Bob xulosasi (yodlash uchun)

1. **UB** — standart natijani **kafolatlamagan** holat (toshish, chegaradan chiqish, NULL...). Dastur ishlashi ham, qulashi ham, jim noto'g'ri bo'lishi ham mumkin.
2. Kompilyator **UB bo'lmaydi** deb ishonadi va tekshiruvlarni **o'chirib** yuborishi mumkin (`x + 1 < x`, dereference'dan keyingi `if (!p)`).
3. Himoya: tekshiruvni **hisoblashdan oldin** qiling (`a > INT_MAX - b`), `__builtin_*_overflow`, kattaroq tur, `memcpy` bilan o'qish.
4. Vositalar: `-Wall -Wextra`, `-fsanitize=undefined,address`, testlar — o'rganishda doim yoqilgan.
5. Implementation-defined (hujjatlangan, masalan `char` ishorasi) va unspecified (tartib) — UB emas, lekin ularga tayanmang.

## Savol-javob

**Nega kompilyator UB ni o'zi ko'rsatmaydi?**
Ba'zan ko'rsatadi (ogohlantirishlar), lekin umuman UB ni oldindan aniqlash mumkin emas (ish vaqtidagi qiymatlarga bog'liq). Shuning uchun sanitizerlar bor.

**UB bo'lgan dastur testlardan o'tsa, u to'g'rimi?**
Yo'q — boshqa kompilyator, optimallashtirish yoki kontekstda buziladi.

**Unsigned toshish UB mi?**
Yo'q — ishorasiz arifmetika modulli va **aniqlangan** (2.5).

## O'zingizni tekshiring

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

## Mashq

- **01, 03, 04** — toshish va surish UB'lari (sanitizer ushlaydi).
- **09, 12** — tashqi kirishni qat'iy tekshirish.
- **18** — xotira UB'larini hisobotdan topish.
- Qo'shimcha: 13.2-dagi `toshadimi` funksiyasini `gcc -O2` va `gcc -O2 -fwrapv` bilan kompilyatsiya qilib, `toshadimi(INT_MAX)` natijasini solishtiring. Keyin `gcc -O2 -S` bilan assembly'ni ko'ring.

<!-- loyiha:boshi -->
## Loyiha: xavfsiz butun sonlar

**Maqsad:** aniqlanmagan xatti-harakatga (UB) yo'l qo'ymaydigan kichik kutubxona: qo'shish, ko'paytirish va
matnni songa aylantirish — **hammasi toshishni oldindan sezadi**.
**Bobdan ishlatiladi:** toshishni tekshirish (13.3), `__builtin_*_overflow`, `strtol` + `errno` + `endptr`, xato kodlari.

**Talab:** har bir funksiya `0` (muvaffaqiyat) yoki **manfiy xato kodi** (`-ERANGE` — sig'maydi,
`-EINVAL` — noto'g'ri kirish) qaytarsin; natija chiqish parametri orqali.
**Nima uchun manfiy kod?** Yadro (Linux) ham shunday: `return -ENOMEM;` (12-bob, 30-bob).

```c
/* xavfsiz.c - toshmaydigan hisob-kitob */
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

static int qosh(int a, int b, int *natija)
{
    return __builtin_add_overflow(a, b, natija) ? -ERANGE : 0;     /* GCC: toshishni o'zi aniqlaydi */
}

static int kopaytir(int a, int b, int *natija)
{
    return __builtin_mul_overflow(a, b, natija) ? -ERANGE : 0;
}

static int matndan_int(const char *s, int *natija)
{
    char *oxir;
    errno = 0;
    long v = strtol(s, &oxir, 10);
    if (oxir == s || *oxir != '\0')             /* hech narsa o'qilmadi yoki oxirida ortiqcha belgi */
        return -EINVAL;
    if (errno == ERANGE || v > INT_MAX || v < INT_MIN)
        return -ERANGE;
    *natija = (int)v;
    return 0;
}

static const char *nomi(int kod)
{
    return kod == 0 ? "ok" : kod == -ERANGE ? "ERANGE (sig'maydi)" : "EINVAL (noto'g'ri)";
}

int main(void)
{
    int r, kod;
    char yorliq[64];

    kod = qosh(2000000000, 100000000, &r);
    printf("%-32s: %s", "qosh(2000000000, 100000000)", nomi(kod));
    if (kod == 0)
        printf(" = %d", r);
    printf("\n");

    kod = qosh(2000000000, 200000000, &r);
    printf("%-32s: %s\n", "qosh(2000000000, 200000000)", nomi(kod));

    kod = kopaytir(65536, 32767, &r);
    printf("%-32s: %s", "kopaytir(65536, 32767)", nomi(kod));
    if (kod == 0)
        printf(" = %d", r);
    printf("\n");

    kod = kopaytir(65536, 32768, &r);
    printf("%-32s: %s\n", "kopaytir(65536, 32768)", nomi(kod));

    const char *matnlar[] = { "12345", "-2147483648", "2147483648", "12x", "", "  42" };
    for (int i = 0; i < 6; i++) {
        kod = matndan_int(matnlar[i], &r);
        snprintf(yorliq, sizeof(yorliq), "matndan_int(\"%s\")", matnlar[i]);
        printf("%-32s: %s", yorliq, nomi(kod));
        if (kod == 0)
            printf(" = %d", r);
        printf("\n");
    }
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g -fsanitize=undefined xavfsiz.c -o xavfsiz
$ ./xavfsiz
qosh(2000000000, 100000000)     : ok = 2100000000
qosh(2000000000, 200000000)     : ERANGE (sig'maydi)
kopaytir(65536, 32767)          : ok = 2147418112
kopaytir(65536, 32768)          : ERANGE (sig'maydi)
matndan_int("12345")            : ok = 12345
matndan_int("-2147483648")      : ok = -2147483648
matndan_int("2147483648")       : ERANGE (sig'maydi)
matndan_int("12x")              : EINVAL (noto'g'ri)
matndan_int("")                 : EINVAL (noto'g'ri)
matndan_int("  42")             : ok = 42
```

Sanitizer **jim** — chunki hech qayerda UB yo'q. Endi xavfli variantni yozing: `return a + b;` (tekshiruvsiz) va
`qosh(INT_MAX, 1)` ni chaqiring: `-fsanitize=undefined` "signed integer overflow" deydi.
`strtol` ning `"  42"` (boshida probel) ni qabul qilishiga e'tibor bering: u boshidagi probelni o'tkazib yuboradi.

**Kengaytiring:** `ayir_xavfsiz` (`__builtin_sub_overflow`) qo'shing. `matndan_int` `"+7"` ni qabul qiladimi? Tekshiring.

## Mustaqil loyiha: `parse_uint` — Linux'ning `kstrtouint` i ★★★

**Vazifa:** matnni `unsigned int` ga **qo'lda** (harfma-harf) aylantiring. `strtol`/`strtoul`/`sscanf` **ishlatmang** —
maqsad: har bir chegaraviy holatni o'zingiz hal qilish. Yadroda libc yo'q, `kstrtouint` aynan shunday yozilgan.
Fayl: `parse.c`.

**Imzo:** `int parse_uint(const char *s, unsigned *chiqish, int asos)` — `asos` 10 yoki 16. `0` yoki `-EINVAL` yoki `-ERANGE`.

**Qoidalar:**
1. Bo'sh satr → `-EINVAL`. Raqam bo'lmagan belgi (harf, `+`, `-`, boshida probel) → `-EINVAL`.
2. Oxirida **bitta** `\n` ruxsat etiladi (fayldan o'qilgan satr uchun), ikkita — yo'q.
3. `asos == 16` da ixtiyoriy `0x`/`0X` old qo'shimchasi; lekin faqat `0x` (raqamsiz) → `-EINVAL`.
4. Qiymat `UINT_MAX` (4294967295) dan oshsa → `-ERANGE`. **Toshishni ko'paytirishdan OLDIN tekshiring**
   (`v * asos + raqam` allaqachon toshgan bo'lmasin — 13.3).
5. Oldingi nollar mumkin: `"007"` = 7.
6. Xato bo'lsa `*chiqish` o'zgarmasin.

`main` quyidagi jadvalni **aynan shu shaklda** chiqarsin: `"<matn>" -> ok <qiymat>` yoki `EINVAL` yoki `ERANGE`.
Ikki maxsus qator (`\n` bilan) `main` da qo'lda yoziladi: `"99\n"` va `"99\n\n"`.

**Kutilgan natija** (`darslik/loyihalar/13_parse_uint/kutilgan.txt`):

```text
Asos 10:
  "0" -> ok 0
  "42" -> ok 42
  "4294967295" -> ok 4294967295
  "4294967296" -> ERANGE
  "" -> EINVAL
  "abc" -> EINVAL
  "12abc" -> EINVAL
  "-5" -> EINVAL
  "+5" -> EINVAL
  " 7" -> EINVAL
  "007" -> ok 7
  "123456789012345678901234567890" -> ERANGE
  "99\n" -> ok 99
  "99\n\n" -> EINVAL
Asos 16:
  "ff" -> ok 255
  "0xFF" -> ok 255
  "0x" -> EINVAL
  "FFFFFFFF" -> ok 4294967295
  "100000000" -> ERANGE
  "xyz" -> EINVAL
  "0x1G" -> EINVAL
```

Sinov satrlari, tartib bilan. Asos 10: `0`, `42`, `4294967295`, `4294967296`, bo'sh satr, `abc`, `12abc`, `-5`, `+5`,
` 7` (boshida probel), `007`, `123456789012345678901234567890`. Asos 16: `ff`, `0xFF`, `0x`, `FFFFFFFF`,
`100000000`, `xyz`, `0x1G`.

**Maslahat** (yechim emas):
- Toshish sharti: `v > (UINT_MAX - raqam) / asos` bo'lsa, `v * asos + raqam` `UINT_MAX` dan oshadi. Nega bo'lish bilan yozilgan?
- `0x` old qo'shimchani faqat `asos == 16` bo'lsa va keyingi belgi mavjud bo'lsa o'tkazing.
- Raqam qiymati: `'0'..'9'` → 0..9, `'a'..'f'`/`'A'..'F'` → 10..15; `asos` dan kichik bo'lishi shart.
- `-fsanitize=undefined` bilan yig'ing: hech qanday xabar bo'lmasligi kerak.

**Tekshirish:**

```bash
gcc -Wall -Wextra -g -fsanitize=address,undefined parse.c -o dastur && ./dastur | diff - ~/C_loyha/darslik/loyihalar/13_parse_uint/kutilgan.txt && echo "TO'G'RI"
```
<!-- loyiha:oxiri -->

Keyingi bob: [14-bob. Tizim chaqiruvlari: fayllar va jarayonlar](14-tizim-chaqiruvlari.md)
