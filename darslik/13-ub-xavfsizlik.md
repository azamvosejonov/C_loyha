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

<!-- katta:boshi -->
## Katta loyiha: Fuzzer — o'z xatolaringizni mashina bilan toping

**Umumiy fikr.** Oldingi boblarda xatolarni **o'zimiz** topdik: dasturni ishga tushirdik, noto'g'ri natijani ko'rdik. Haqiqiy xatolar esa odatda **kutilmagan kirishda** chiqadi — kimdir 200 belgili nom yoki `99999999999` kiritganda. Bunday kirishlarni qo'lda o'ylab topish qiyin. **Fuzzer** — shuni mashinaning o'ziga topshiradigan dastur: u minglab **tasodifiy buzilgan** kirishlarni parserga yuboradi, sanitizer (13-bob) esa birinchi xatoda dasturni to'xtatib, **qayerda** va **nega** deb aytadi.

**Hayotiy o'xshatish:** yangi eshik qulfini sinash uchun uni million marta turli kalitlar bilan ochib ko'rasiz — qaysidir kalit "noto'g'ri" bo'lsa-yu, eshik ochilib ketsa, qulf nuqsonli.

### Bu bosqichda nima qilamiz

`nom:narx:soni` ko'rinishidagi qatorni (masalan `non:4000:120`) o'qiydigan kichik parser yozamiz. Uni **ikki xil** yozamiz:

| Variant | Fayl | Qanday |
|---|---|---|
| **zaif** | `parser_zaif.c` | ataylab 4 ta xato bilan (hech qachon bunday yozmang!) |
| **xavfsiz** | `parser_xavfsiz.c` | har xatoga qarshi chora ko'rilgan |

Ikkalasi ham **bitta interfeys** (`parser.h`) orqali ishlaydi — shuning uchun **bitta fuzzer** ikkalasini sinay oladi (1-bobdagi "e'lon va ta'rif" g'oyasi, 11-bobdagi ko'p fayl).

**Birinchi — interfeys (`parser.h`):**

```c
/* parser.h - "nom:narx:soni" qatorlarini o'qiydigan kutubxona interfeysi (ikki ichki variant bor: zaif va xavfsiz) */
#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

struct yozuv {
    char nom[16];                               /* eng ko'pi bilan 15 belgi + '\0' */
    int narx;                                   /* so'mda */
    int soni;
};

/* "non:4000:120" ni y ga o'qiydi. 0 - muvaffaqiyat, -1 - format noto'g'ri */
int yozuv_oqi(const char *satr, struct yozuv *y);

/* hamma yozuvlar narx * soni yig'indisi. 0 - muvaffaqiyat, -1 - toshib ketdi */
int yozuvlar_jami(const struct yozuv *y, size_t n, long *jami);

#endif
```

**Ikkinchi — zaif variant.** Har xato izohda ko'rsatilgan:

```c
/* parser_zaif.c - ZAIF variant: ataylab xatolar bilan (hech qachon bunday yozmang!) */
#include <stdlib.h>
#include <string.h>

#include "parser.h"

int yozuv_oqi(const char *satr, struct yozuv *y)
{
    char nusxa[64];
    strcpy(nusxa, satr);                        /* XATO 1: satr 64 baytdan uzun bo'lsa stek buziladi */

    char *birinchi = strchr(nusxa, ':');
    if (!birinchi)
        return -1;
    *birinchi = '\0';
    strcpy(y->nom, nusxa);                      /* XATO 2: nom 15 belgidan uzun bo'lsa y->nom dan chiqib ketadi */

    char *ikkinchi = strchr(birinchi + 1, ':');
    if (!ikkinchi)
        return -1;
    *ikkinchi = '\0';
    y->narx = atoi(birinchi + 1);               /* XATO 3: atoi xatoni bildirmaydi, katta sonda UB */
    y->soni = atoi(ikkinchi + 1);
    return 0;
}

int yozuvlar_jami(const struct yozuv *y, size_t n, long *jami)
{
    int yig = 0;
    for (size_t i = 0; i < n; i++)
        yig += y[i].narx * y[i].soni;           /* XATO 4: int toshishi (aniqlanmagan xatti-harakat) */
    *jami = yig;
    return 0;
}
```

**Uchinchi — xavfsiz variant.** Bu yerda barcha xatolar yopilgan:

```c
/* parser_xavfsiz.c - XAVFSIZ variant: har xatoga qarshi chora */
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

/* matnni butun songa o'giradi: faqat raqamlar, chegarada. 0 - OK */
static int son_oqi(const char *s, size_t uzunlik, int *natija)
{
    if (uzunlik == 0 || uzunlik > 9)            /* bo'sh yoki juda uzun: int ga sig'maydi */
        return -1;
    long v = 0;
    for (size_t i = 0; i < uzunlik; i++) {
        if (s[i] < '0' || s[i] > '9')
            return -1;                          /* raqam bo'lmagan belgi */
        v = v * 10 + (s[i] - '0');
    }
    *natija = (int)v;                           /* 9 raqam <= 999999999 < INT_MAX */
    return 0;
}

int yozuv_oqi(const char *satr, struct yozuv *y)
{
    const char *a = strchr(satr, ':');
    if (!a)
        return -1;
    const char *b = strchr(a + 1, ':');
    if (!b)
        return -1;

    size_t nom_uz = (size_t)(a - satr);
    if (nom_uz == 0 || nom_uz >= sizeof(y->nom))
        return -1;                              /* nom bo'sh yoki joyga sig'maydi */

    int narx, soni;
    if (son_oqi(a + 1, (size_t)(b - a - 1), &narx) != 0)
        return -1;
    const char *oxir = b + 1;
    if (son_oqi(oxir, strlen(oxir), &soni) != 0)
        return -1;

    memcpy(y->nom, satr, nom_uz);               /* uzunlik tekshirilgan: chegaradan chiqmaydi */
    y->nom[nom_uz] = '\0';
    y->narx = narx;
    y->soni = soni;
    return 0;
}

int yozuvlar_jami(const struct yozuv *y, size_t n, long *jami)
{
    long yig = 0;
    for (size_t i = 0; i < n; i++) {
        long p;
        if (__builtin_mul_overflow((long)y[i].narx, (long)y[i].soni, &p) || __builtin_add_overflow(yig, p, &yig))
            return -1;                          /* toshishni TEKSHIRIB aniqlaymiz */
    }
    *jami = yig;
    return 0;
}
```

**To'rtinchi — fuzzer o'zi.** Uning ishi: kirishni tasodifiy **buzish** va parserga berish.

```c
/* fuzz.c - fuzzer: tasodifiy buzilgan kirishlarni parserga yuboradi; xato bo'lsa sanitizer to'xtatadi */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

static unsigned long holat;                     /* tasodif generatori holati (bir xil urug' = bir xil ketma-ketlik) */

static unsigned tasodif(void)
{
    holat = holat * 6364136223846793005UL + 1442695040888963407UL;
    return (unsigned)(holat >> 33);
}

/* haqiqiy kirishni olib, tasodifiy "buzamiz": uzaytirish, belgi almashtirish, ikki nuqta qo'shish */
static void buz(char *bufer, size_t sigim)
{
    static const char *urug[] = { "non:4000:120", "sut:12000:45", "guruch:18000:8", "shakar:15000:60" };
    snprintf(bufer, sigim, "%s", urug[tasodif() % 4]);
    size_t uz = strlen(bufer);

    switch (tasodif() % 5) {
    case 0:                                     /* nomni uzun qilamiz */
        for (unsigned k = tasodif() % 100; k && uz + 1 < sigim; k--)
            memmove(bufer + 1, bufer, ++uz), bufer[0] = 'a' + (char)(tasodif() % 26);
        break;
    case 1:                                     /* ko'p raqam qo'shamiz */
        for (unsigned k = tasodif() % 30; k && uz + 1 < sigim; k--)
            bufer[uz++] = '0' + (char)(tasodif() % 10);
        bufer[uz] = '\0';
        break;
    case 2:                                     /* tasodifiy belgi almashtiramiz */
        if (uz)
            bufer[tasodif() % uz] = (char)(33 + tasodif() % 90);
        break;
    case 3:                                     /* ':' belgilarini ko'paytiramiz yoki yo'qotamiz */
        for (size_t i = 0; i < uz; i++)
            if (bufer[i] == ':' && tasodif() % 2)
                bufer[i] = 'x';
        break;
    default:                                    /* hech narsa: to'g'ri kirish */
        break;
    }
}

int main(int argc, char **argv)
{
    unsigned long urug = argc > 1 ? strtoul(argv[1], NULL, 10) : 1;
    long n = argc > 2 ? atol(argv[2]) : 1000;
    holat = urug;

    /* 1) aniq sinov: 12 ta juda katta yozuvning yig'indisi long ga ham sig'maydi */
    struct yozuv katta[12];
    for (int i = 0; i < 12; i++) {
        strcpy(katta[i].nom, "x");
        katta[i].narx = 999999999;
        katta[i].soni = 999999999;
    }
    long yig;
    int r = yozuvlar_jami(katta, 12, &yig);
    printf("toshish sinovi: yozuvlar_jami() = %d (%s)\n", r, r ? "toshish aniqlandi" : "toshish ANIQLANMADI!");

    /* 2) fuzz: n ta tasodifiy buzilgan kirish */
    long togri = 0, rad = 0, toshdi = 0;
    for (long i = 0; i < n; i++) {
        char kirish[256];
        buz(kirish, sizeof(kirish));
        FILE *f = fopen("oxirgi_kirish.txt", "w");  /* qulash bo'lsa, qaysi kirish sababchi ekanini bilamiz */
        if (f) {
            fputs(kirish, f);
            fclose(f);
        }

        struct yozuv y[2];
        long jami;
        if (yozuv_oqi(kirish, &y[0]) == 0) {
            togri++;
            y[1] = y[0];                        /* ikkita yozuv: yig'indini tekshirish uchun */
            if (yozuvlar_jami(y, 2, &jami) != 0)
                toshdi++;
        } else {
            rad++;
        }
    }
    printf("urug %lu, %ld ta kirish: %ld ta to'g'ri o'qildi, %ld ta rad etildi, %ld ta yig'indi toshdi (aniqlandi)\n",
           urug, n, togri, rad, toshdi);
    return 0;
}
```

**Fuzzer qanday ishlaydi (umumiy):**

1. `holat` — tasodif generatori (LCG). **Bir xil urug' = bir xil ketma-ketlik**: shuning uchun topilgan xatoni **qayta ishlab chiqarish** mumkin (`./fuzz_zaif 12345`).
2. `buz()` — to'rtta haqiqiy qatordan birini oladi va **tasodifiy buzadi**: nomni uzaytiradi, raqam qo'shadi, bitta belgini almashtiradi yoki `:` ni yo'qotadi.
3. Har kirishdan **oldin** `oxirgi_kirish.txt` ga yoziladi: dastur qulasa, shu fayl **aynan qaysi kirish** sababchi ekanini ko'rsatadi.
4. Parser kirishni qabul qilsa — `yozuvlar_jami()` bilan yig'indisi tekshiriladi; rad etsa — sanab qo'yiladi.

Avval **zaif** variantni sinaymiz. Sanitizer (`-fsanitize=address,undefined`) xatoni topgach xabar beradi. Chiqish juda uzun, shuning uchun faqat muhim qatorlarni olamiz (`grep`), manzillarni esa `sed` bilan yashiramiz:

```console
$ cd katta_loyiha/tizim/13_fuzz
$ gcc -Wall -Wextra -g -fsanitize=address,undefined fuzz.c parser_zaif.c -o fuzz_zaif
$ ./fuzz_zaif 12345 200000 2>&1 | grep -E 'runtime error|ERROR|SUMMARY|toshish sinovi' | sed -E 's/==[0-9]+==//; s/0x[0-9a-f]+/0x.../g' # xato kutiladi
parser_zaif.c:31:26: runtime error: signed integer overflow: 999999999 * 999999999 cannot be represented in type 'int'
parser_zaif.c:31:13: runtime error: signed integer overflow: 1616697346 + 808348673 cannot be represented in type 'int'
ERROR: AddressSanitizer: stack-buffer-overflow on address 0x... at pc 0x... bp 0x... sp 0x...
SUMMARY: AddressSanitizer: stack-buffer-overflow ../../../../src/libsanitizer/asan/asan_interceptors.cpp:563 in strcpy
```

**Nima ko'rdik:**

- `signed integer overflow` — UBSan `parser_zaif.c` ning **31-qatorida** `int` toshganini topdi (`yig += narx * soni`, XATO 4).
- `stack-buffer-overflow ... in strcpy` — ASan **64 baytlik** `nusxa` massivi chegarasidan chiqilganini topdi (XATO 1): fuzzer **uzun nomli** kirish yaratgan edi.
- Sanitizer **qator raqamini** ham, **qaysi funksiya** (`yozuv_oqi`) ekanini ham aytdi — qidirish kerak emas.
- Qaysi kirish sabab bo'ldi? `cat oxirgi_kirish.txt` — 90 ga yaqin belgidan iborat uzun nom.

Endi **xavfsiz** variantni ayni shu fuzzer bilan sinaymiz:

```console
$ cd katta_loyiha/tizim/13_fuzz
$ gcc -Wall -Wextra -g -fsanitize=address,undefined fuzz.c parser_xavfsiz.c -o fuzz_xavfsiz
$ ./fuzz_xavfsiz 12345 200000
toshish sinovi: yozuvlar_jami() = -1 (toshish aniqlandi)
urug 12345, 200000 ta kirish: 80734 ta to'g'ri o'qildi, 119266 ta rad etildi, 0 ta yig'indi toshdi (aniqlandi)
```

**Nima ko'rdik:**

- **Hech qanday sanitizer xabari yo'q** — 200 000 ta buzilgan kirishdan keyin ham dastur tinch tugadi.
- `toshish sinovi: ... = -1 (toshish aniqlandi)`: 12 ta juda katta yozuvning yig'indisi `long` ga ham sig'maydi, xavfsiz variant buni **-1** bilan **bildirdi** (zaif variant jimgina noto'g'ri son qaytarardi).
- Taxminan 40% kirish to'g'ri o'qildi, 60% rad etildi: parser **buzilgan kirishni to'g'ri rad etyapti**, qulamayapti — xavfsizlikning asosiy ma'nosi shu.

**To'rtta xato va ularning davosi:**

| # | Zaif kod | Muammo | Xavfsiz kod |
|---|---|---|---|
| 1 | `strcpy(nusxa, satr)` | satr 64 baytdan uzun bo'lsa — **stek buziladi** | nusxa olmaymiz; uzunlikni **avval tekshirib**, `memcpy` qilamiz |
| 2 | `strcpy(y->nom, nusxa)` | nom 15 belgidan uzun bo'lsa — struct dan chiqib ketadi | `nom_uz >= sizeof(y->nom)` bo'lsa **rad etamiz** |
| 3 | `atoi(...)` | xatoni bildirmaydi: `"abc"` ham `0` bo'ladi, katta sonda UB | `son_oqi()`: faqat `'0'..'9'`, uzunlik ≤ 9 |
| 4 | `int yig += a * b` | **signed overflow** — UB (2-bob) | `__builtin_mul_overflow` / `__builtin_add_overflow` — toshishni **aniqlaydi** |

> **Eslab qoling:** tashqaridan kelgan **har bir** kirish (fayl, tarmoq, foydalanuvchi) — **dushman** deb hisoblang. Parser: (1) uzunlikni tekshiradi, (2) belgilarni tekshiradi, (3) chegaradan chiqmaydi, (4) toshishni aniqlaydi. Fuzzer + sanitizer — buni isbotlashning eng arzon yo'li.

**O'zingiz qo'shing (yechimsiz):**

1. `parser_xavfsiz.c` dan `nom_uz >= sizeof(y->nom)` tekshiruvini **vaqtincha** olib tashlab, fuzzer bilan sinang: sanitizer endi qaysi xatoni topadi? Fuzzer buni **tez** topdimi?
2. `buz()` ga **beshinchi usul** qo'shing: kirishning o'rtasiga `\0` (null bayt) kiriting. Parser buni qanday qabul qiladi?
3. Fuzzerni kengaytiring: har 1000 kirishdan keyin **progress** (`... 1000 ta tekshirildi`) chiqaring. Urug'ni (`argv[1]`) o'zgartirib turli xatolar topiladimi?
<!-- katta:oxiri -->

<!-- kadrlar:boshi -->
## Katta loyiha: Kadrlar tizimi — 13-bosqich: xavfsizlik va fuzzer

**Oldingi bosqichdan:** 12-bosqichda parser fayldan o'qirdi va noto'g'ri qatorlarni rad etardi. Lekin ikki muammo qoldi: (1) tekshiruv fayl o'qish kodi **ichida** aralashgan — uni **alohida sinab bo'lmaydi**; (2) biz faqat **o'zimiz o'ylagan** noto'g'ri qatorlarni sinadik. Haqiqiy dushman esa **ko'pgina kutilmagan** kirishni yuboradi: 300 harfli ism, `99999999999999999999` tarif, manfiy ishora...

**Qoida (13-bob):** tashqaridan kelgan **hamma** narsa — dushman. Tekshiruvni **chegarada** (ma'lumot tizimga kirgan joyda) qiling; undan keyin ichkaridagi kod **ishonib** ishlay oladi.

### Bu bosqichda nima qilamiz

1. **Parserni toza funksiyaga ajratamiz:** `xodim_tahlil(qator, &x, &sabab)` — bitta qatorni oladi va faylga, ekranga **tegmaydi**. Shuning uchun uni **test va fuzz** qilish oson.
2. **Hamma chegarani bir joyga** yig'amiz (`config.h`: `TARIF_MAKS`, `OY_MAKS_DAQ`, `QATOR_UZ`) va **toshishning mumkin emasligini kompilyatsiya vaqtida isbotlaymiz** (`_Static_assert`).
3. **Juda uzun qatorni** to'g'ri qayta ishlaymiz (qolgan qismini tashlab yuboramiz).
4. **Fuzzer** yozamiz: tasodifiy buzilgan **ikki yuz mingta** qator parserga yuboriladi, sanitizer xotira xatolarini, biz esa **invariantlarni** (qabul qilingan natija **yaroqli** bo'lishi shart) tekshiramiz.
5. Taqqoslash uchun **ataylab zaif** parser yozamiz — fuzzer uni topib olishini ko'ramiz.

### Chegaralar — bir joyda

```c
/* KIRISH CHEGARALARI: tashqaridan kelgan ma'lumotga ISHONMAYMIZ. Hamma chegara shu yerda, parser va hisob ular bilan mos */
#define QATOR_UZ 256                            /* fayl qatorining eng uzun hajmi */
#define TARIF_MAKS 100000000000LL               /* eng yuqori tarif: 1 mlrd so'm/soat (tiyinda) */
#define OY_MAKS_DAQ (31 * 24 * 60)              /* bir oyda bo'lishi mumkin bo'lgan eng ko'p daqiqa */
```

`pul.c` da shu chegaralar **toshib ketmasligi** kafolatlanadi — tekshiruv **kompilyatsiya vaqtida**:

```c
_Static_assert(TARIF_MAKS <= INT64_MAX / OY_MAKS_DAQ / 2, "tarif * daqiqa * 3 int64_t dan oshib ketishi mumkin");
```

Ma'nosi: eng katta tarif × eng ko'p daqiqa × 3 (ustama koeffitsiyenti) `int64_t` ga **sig'adi**. Kimdir `TARIF_MAKS` ni 100 barobar oshirsa — dastur **yig'ilmaydi**, jimgina noto'g'ri ishlamaydi.

### Toza parser

Xavfsiz parserning to'rt qoidasi: **uzunlikni tekshir** → **belgilarni tekshir** → **oraliqni tekshir** → **chegaradan chiqma**. Mana u:

```c
int xodim_tahlil(const char *qator, struct xodim *x, const char **sabab)
{
    char nusxa[QATOR_UZ];
    if (strlen(qator) >= sizeof(nusxa)) {       /* strcpy EMAS: avval uzunlikni tekshiramiz */
        *sabab = "qator juda uzun";
        return -1;
    }
    memcpy(nusxa, qator, strlen(qator) + 1);

    char *soz[8];
    if (bol(nusxa, soz, 4) != 4) {
        *sabab = "maydonlar soni 4 emas";
        return -1;
    }
    long long id, toifa, tarif;
    if (son_oqi(soz[0], &id) || id < 1000 || id > 9999) {
        *sabab = "ID 1000..9999 oralig'ida son emas";
        return -1;
    }
    if (strlen(soz[1]) >= ISM_UZ) {
        *sabab = "ism juda uzun";
        return -1;
    }
    if (son_oqi(soz[2], &toifa) || toifa < 1 || toifa > T_SONI) {
        *sabab = "toifa 1..4 oralig'ida son emas";
        return -1;
    }
    if (son_oqi(soz[3], &tarif) || tarif <= 0 || tarif > TARIF_MAKS) {
        *sabab = "tarif 1..TARIF_MAKS oralig'ida son emas";
        return -1;
    }
    memset(x, 0, sizeof(*x));
    x->id = (int)id;
    x->toifa = (enum toifa)(toifa - 1);
    x->tarif = tarif;
    memcpy(x->ism, soz[1], strlen(soz[1]) + 1);         /* uzunlik yuqorida tekshirilgan */
    return 0;
}
```

Nimalar o'zgardi:

| Eski usul | Yangi usul | Nega |
|---|---|---|
| `strcpy(nusxa, qator)` | `strlen` tekshiruvi + `memcpy` | uzun qator buferdan chiqib ketmasin |
| `sscanf("%s", ism)` | so'zni `strtok_r` bilan ajratib, `strlen(soz[1]) >= ISM_UZ` tekshiruvi | ism `ism[24]` ga sig'sin |
| `atoi` | `strtoll` + `son_oqi` (12-bosqich) | harf, toshish aniqlansin |
| oraliqsiz | `1000 ≤ id ≤ 9999`, `1 ≤ toifa ≤ 4`, `0 < tarif ≤ TARIF_MAKS` | keyingi kod "yaroqli" deb ishonsin |

**Uzun qator** — alohida muammo: `fgets` bufer to'lguncha o'qiydi va **qatorning qolgan qismi** keyingi `fgets` ga "yangi qator" bo'lib kirib qoladi. Shuning uchun uzun qatorni **oxirigacha o'qib tashlaymiz**:

```c
static int qator_ol(FILE *f, char *qator, size_t hajm)
{
    if (!fgets(qator, (int)hajm, f))
        return 0;
    if (strchr(qator, '\n') == NULL && !feof(f)) {
        int c;
        while ((c = fgetc(f)) != '\n' && c != EOF)
            ;
        return -1;
    }
    return 1;
}
```

### Fuzzer

**Fuzzing** — dasturga **minglab tasodifiy buzilgan** kirishni yuborib, qulashini yoki noto'g'ri natija berishini kutish. Bizning fuzzerimiz: (1) haqiqiy qatorlardan **urug'** oladi, (2) `buz()` bilan buzadi (uzun ism, ko'p raqam, bitta belgini almashtirish, manfiy ishora, juda katta son), (3) parserga beradi, (4) agar parser **qabul qilsa** — natija **invariantlarga** mos bo'lishini tekshiradi (ID, toifa, tarif, ism uzunligi). **Bir xil urug' = bir xil ketma-ketlik**: topilgan xatoni qayta ishlab chiqarish mumkin. Har qatordan **oldin** `oxirgi_kirish.txt` ga yoziladi — dastur qulasa, aynan qaysi qator sababchi ekanini bilamiz.

```c
int main(int argc, char **argv)
{
    unsigned long urug_son = argc > 1 ? strtoul(argv[1], NULL, 10) : 1;
    long n = argc > 2 ? atol(argv[2]) : 1000;
    holat = urug_son;

    static const char *const xodim_urug[] = { "1042 Aziza 2 2500050", "2087 Bobur 1 3150000", "9999 Sardor 4 1250000075" };
    long qabul = 0, rad = 0;
#ifndef ZAIF
    static const char *const davomat_urug[] = { "1042 1 0900 1800", "3150 2 1000 1800", "9999 31 0000 2359" };
    long d_qabul = 0, d_rad = 0;
#endif
    for (long i = 0; i < n; i++) {
        char qator[QATOR_UZ * 2];
        struct xodim x;
        const char *sabab;

        buz(qator, sizeof(qator), xodim_urug, 3);
        FILE *f = fopen("oxirgi_kirish.txt", "w");      /* qulasa, qaysi qator sababchi ekanini bilamiz */
        if (f) {
            fputs(qator, f);
            fclose(f);
        }
        if (XODIM_TAHLIL(qator, &x, &sabab) == 0) {
            qabul++;
            int yaroqli = x.id >= 1000 && x.id <= 9999 && x.toifa >= 0 && x.toifa < T_SONI && x.tarif > 0 &&
                          x.tarif <= TARIF_MAKS && strlen(x.ism) < ISM_UZ;
            if (!yaroqli) {
                printf("INVARIANT BUZILDI: qabul qilingan xodim noto'g'ri: id=%d toifa=%d tarif=%lld  (qator: %.60s)\n", x.id,
                       (int)x.toifa, (long long)x.tarif, qator);
                return 1;
            }
        } else {
            rad++;
        }

#ifndef ZAIF
        buz(qator, sizeof(qator), davomat_urug, 3);
        int id, kirish, chiqish;
        if (davomat_tahlil(qator, &id, &kirish, &chiqish, &sabab) == 0) {
            d_qabul++;
            if (id < 1000 || id > 9999 || kirish < 0 || kirish > 2359 || chiqish < 0 || chiqish > 2359) {
                printf("INVARIANT BUZILDI: davomat: id=%d kirish=%d chiqish=%d\n", id, kirish, chiqish);
                return 1;
            }
        } else {
            d_rad++;
        }
#endif
    }
    printf("urug %lu, %ld ta kirish: xodim qabul %ld / rad %ld", urug_son, n, qabul, rad);
#ifndef ZAIF
    printf("; davomat qabul %ld / rad %ld", d_qabul, d_rad);
#endif
    printf(". Invariant buzilmadi, sanitizer jim.\n");
    return 0;
}
```

**Zaif parser** — taqqoslash uchun ataylab to'rtta xato bilan (hech qachon bunday yozmang):

```c
/* parser_zaif.c - ATAYLAB ZAIF variant (hech qachon bunday yozmang!). Faqat fuzzer unga qarshi ishlashini ko'rsatish uchun. */
#include <stdio.h>
#include <string.h>

#include "yukla.h"

int xodim_tahlil_zaif(const char *qator, struct xodim *x, const char **sabab)
{
    char nusxa[64];
    strcpy(nusxa, qator);                       /* XATO 1: qator 64 baytdan uzun bo'lsa stek buziladi */

    int toifa;
    long long tarif;
    if (sscanf(nusxa, "%d %s %d %lld", &x->id, x->ism, &toifa, &tarif) != 4) {   /* XATO 2: %s uzunlik chegarasiz: ism[24] dan chiqib ketadi */
        *sabab = "format noto'g'ri";
        return -1;
    }
    x->toifa = (enum toifa)(toifa - 1);         /* XATO 3: toifa oralig'i tekshirilmagan */
    x->tarif = tarif;                           /* XATO 4: tarif musbatmi, juda kattami - tekshirilmagan */
    x->oddiy_daq = x->qosh_daq = 0;
    return 0;
}
```

### Ishga tushirish

Fuzzerlar **sanitizer** (ASan + UBSan) bilan yig'iladi: `make fuzz`.

```console
$ cd katta_loyiha/kadrlar/13_xavfsizlik
$ make -s
$ make -s fuzz
$ ./bin/fuzz_xavfsiz 12345 200000
urug 12345, 200000 ta kirish: xodim qabul 60681 / rad 139319; davomat qabul 44363 / rad 155637. Invariant buzilmadi, sanitizer jim.
$ ./bin/fuzz_zaif 1 200000 2>&1 | head -1
INVARIANT BUZILDI: qabul qilingan xodim noto'g'ri: id=9999 toifa=3 tarif=9223372036854775807  (qator: 9999 Sardor 4 1250000075045029632204205257760984218788542686)
$ ./bin/fuzz_zaif 3 200000 2>&1 | grep -c "overflow\|AddressSanitizer"
1
```

**Nima ko'rdik:**

- **Xavfsiz parser:** 200 000 ta buzilgan qator — **60 681 ta qabul**, **139 319 ta rad**; davomat parseri ham shunday. **Sanitizer jim**, invariantlar **buzilmadi**: qabul qilingan har natija **yaroqli**.
- **Zaif parser, urug' 1:** `INVARIANT BUZILDI` — parser qatorni **qabul qildi**, lekin `tarif = 9223372036854775807` (eng katta `int64_t`): `sscanf` juda katta sonni jimgina **kesib qo'ydi**. Bunday tarif keyin ko'paytirishda **toshadi** — "yaroqli ko'rinadigan, lekin zaharli" ma'lumot.
- **Zaif parser, urug' 3:** qator **64 baytdan uzun** (`oxirgi_kirish.txt` da uzun ism) — `strcpy` **steki buzdi** va tizim dasturni to'xtatdi (`*** buffer overflow detected ***` yoki ASan xabari). Ikkala holatda ham fuzzer xatoni **soniyalarda** topdi — qo'lda sinasangiz, balki **hech qachon**.

**Zaif parserdagi 4 xato:**

| # | Zaif kod | Muammo | Davosi |
|---|---|---|---|
| 1 | `strcpy(nusxa, qator)` | uzun qator **stek**ni buzadi | `strlen` tekshiruvi |
| 2 | `sscanf("%s", x->ism)` | `ism[24]` dan chiqib ketadi | so'z uzunligini tekshirish |
| 3 | `toifa - 1` tekshiruvsiz | `enum` oralig'idan chiqadi → jadvaldan **tashqariga** o'qish | `1..T_SONI` oralig'i |
| 4 | tarif chegarasiz | keyin **toshish** | `0 < tarif ≤ TARIF_MAKS` |

> **Eslab qoling:** tashqi kirish = dushman. **Chegarada** tekshiring: uzunlik → belgilar → oraliq. Tekshiruvni **toza funksiyaga** ajrating (fayl/ekransiz) — shunda uni **fuzz** qilish mumkin. Chegaralarni bir joyga yig'ib, **`_Static_assert`** bilan "toshish mumkin emas"ligini isbotlang. **Invariant** — "qabul qilingan har natija shu shartlarni bajaradi": fuzzer shuni tekshiradi. Sanitizer + fuzzer — eng arzon xavfsizlik testi.

**O'zingiz qo'shing (yechimsiz):**

1. `xodim_tahlil` dan `strlen(soz[1]) >= ISM_UZ` tekshiruvini **vaqtincha** olib tashlab, `make fuzz && ./bin/fuzz_xavfsiz 1 200000` ni ishga tushiring: **qaysi** sanitizer xabari chiqadi va fuzzer uni **tez** topdimi?
2. `fuzz.c` dagi `buz()` ga **yangi usul** qo'shing: qator o'rtasiga **`\0`** (null bayt) kiritish. (Maslahat: `snprintf` bilan emas, `memset` bilan.) Parser bunga qanday javob beradi?
3. `config.h` da `TARIF_MAKS` ni **100 barobar** oshiring (`10000000000000LL` ga) va `make` qiling: `_Static_assert` nima deydi? Nega bu jimgina noto'g'ri ishlashdan **yaxshi**?
<!-- kadrlar:oxiri -->

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

### Isitish: UB'siz yozish ★☆☆ — eng osoni, avval shuni qiling

Faqat 0–13-boblar kerak (toshishni oldindan tekshirish, chegara, `memcpy`).
Skeletni `isitish.c` ga **qo'lda** yozing (ko'chirmang), izohlarni o'qing va `TODO` joylarini to'ldiring.
"Namuna" qismlar tayyor — qolganini qanday yozishni ko'rsatadi. Skelet hozir ham ogohlantirishsiz yig'iladi:
har `TODO` dan keyin yig'ib, ishga tushirib boring.

```c
/* isitish.c - 13-bob, isitish: UB'siz yozish - toshishni OLDINDAN tekshirish, chegara, memcpy. */
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* a + b ni hisoblashdan OLDIN tekshiramiz: toshgandan keyin tekshirish kech - signed toshish UB,
 * kompilyator tekshiruvni butunlay olib tashlashi mumkin (13.2). 0 - muvaffaqiyat, -1 - toshadi.
 * TODO: b > 0 bo'lsa a > INT_MAX - b da, b < 0 bo'lsa a < INT_MIN - b da -1 qaytaring;
 *       aks holda *natija = a + b; return 0; */
static int xavfsiz_qosh(int a, int b, int *natija)
{
    (void)a;
    (void)b;
    (void)natija;
    return -1;
}

/* Chegara tekshiruvi bilan o'qish: i >= n bo'lsa -1. size_t - manfiy bo'lmaydi, shuning uchun i < 0 tekshiruvi kerak emas.
 * (Namuna - tayyor.) */
static int ol(const int *a, size_t n, size_t i, int *natija)
{
    if (i >= n)
        return -1;
    *natija = a[i];
    return 0;
}

int main(void)
{
    int r;

    /* 1) Toshadigan holat - funksiya -1 qaytaradi, r ga hech narsa yozilmaydi. (Namuna - tayyor.) */
    if (xavfsiz_qosh(INT_MAX, 1, &r) != 0)
        printf("xavfsiz_qosh(INT_MAX, 1): toshadi\n");

    /* 2) TODO: xavfsiz_qosh(100, 23, &r) == 0 bo'lsa - "xavfsiz_qosh(100, 23): %d\n" va r.
     *    Natija: xavfsiz_qosh(100, 23): 123 */

    int massiv[5] = {10, 20, 30, 40, 50};
    if (ol(massiv, 5, 7, &r) != 0)
        printf("indeks 7: chegaradan tashqari (uzunlik 5)\n");
    if (ol(massiv, 5, 4, &r) == 0)
        printf("indeks 4: %d\n", r);

    /* 3) TODO: baytlar + 1 dan 4 bayt - uint32_t. *(uint32_t *)(baytlar + 1) DEMANG: tekislanmagan murojaat
     *    va strict aliasing - UB (13.3). To'g'ri: memcpy(&qiymat, baytlar + 1, sizeof(qiymat));
     *    x86 little-endian: kichik bayt birinchi -> 0x04030201 (16-bob).
     *    Natija: memcpy bilan o'qildi: 0x04030201 */
    unsigned char baytlar[] = {0xAA, 0x01, 0x02, 0x03, 0x04};
    (void)baytlar;                      /* 3-qadamni yozgach, bu qatorni o'chiring */
    return 0;
}
```

**Kutilgan natija** (`darslik/loyihalar/13_parse_uint/isitish.txt`):

```text
xavfsiz_qosh(INT_MAX, 1): toshadi
xavfsiz_qosh(100, 23): 123
indeks 7: chegaradan tashqari (uzunlik 5)
indeks 4: 50
memcpy bilan o'qildi: 0x04030201
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address,undefined isitish.c -o isitish
$ ./isitish | diff - ~/C_loyha/darslik/loyihalar/13_parse_uint/isitish.txt && echo "TO'G'RI"
TO'G'RI
```

### Keyingi mashqlar

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
