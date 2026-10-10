# 29-bob. Debug va profiling vositalari

> **Bu bobda nima o'rganasiz:** xatoni **tizimli** topish usulini (taxmin qilib emas); `printf` izlari; **gdb** (to'xtash nuqtasi, watchpoint, chaqiruvlar zanjiri, core dump); **sanitizer**'lar; **valgrind**; **strace** (dastur OS bilan
> nima gaplashyapti); binar fayllarni ko'rish (`nm`, `objdump`, `addr2line`); `perf` va QEMU vositalari. Dasturchi vaqtining yarmi — debug; bu bob o'sha yarmini tezlashtiradi.
> **Oldindan nima kerak:** 8-, 13-, 14-boblar (xotira xatolari, UB, syscall).   **Vaqt:** 6–8 soat.

> **To'liq ishlaydigan misol:** [misollar/29_xatoli.c](misollar/29_xatoli.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Bu bob nima haqida?

Hech kim xatosiz dastur yozmaydi. Farq shundaki, tajribali dasturchi xatoni **soatlar emas, daqiqalarda** topadi. Buning siri — to'g'ri vosita va to'g'ri usul. Bu bobda:

1. **Usul** (29.1): xato qidirish — detektiv ishi; asbobdan oldin fikr.
2. **Oddiy vositalar** (29.2–29.4): `printf` izlari, gdb, sanitizer'lar — eng ko'p ishlatiladigan uchlik.
3. **Chuqurroq vositalar** (29.5–29.9): valgrind, strace, binar fayllarni ko'rish, perf, QEMU.
4. **Retsept** (29.10): "belgi → birinchi qadam" jadvali.

**Hayotdan misol: detektiv.** Detektiv tasodifiy odamlarni hibsga olmaydi. U **dalillarni** yig'adi, **gipoteza** quradi ("qotil bog' eshigidan kirgan"), uni **tekshiradi** va noto'g'ri bo'lsa — yangisini quradi.

| Detektivda | Debug'da |
|---|---|
| jinoyat belgisi | dastur noto'g'ri natija beradi / qulaydi |
| dalillarni yig'ish | takrorlash, `printf`, log |
| non ushoqlari (iz qoldirish) | `printf` / `kprintf` izlari |
| vaqtni to'xtatib, hamma cho'ntakni tekshirish | gdb: `break`, `print`, `bt` |
| seyfga teggan odamni ushlovchi signalizatsiya | watchpoint (`watch x`) |
| aeroport rentgeni | sanitizer (ASan, UBSan, TSan) |
| telefon suhbatlari yozuvi | `strace` (dastur ↔ yadro suhbati) |
| fitnes-soat (qadam, yurak urishi) | `perf` (vaqt qayerga ketyapti) |

## 29.1. Debug usuli — asbobdan oldin fikr

**Oddiy qilib aytganda:** xatoni topish — tajriba o'tkazish: taxmin qilasiz, tekshirasiz, natijaga qarab taxminni tasdiqlaysiz yoki rad etasiz. **Kodni tasodifan o'zgartirib ko'rish** — usul emas, omad o'yini.

| # | Qoida | Nima uchun |
|---|---|---|
| 1 | **Takrorlang** — xatoni har safar chiqadigan qiling (kirish, buyruq, qadamlar) | takrorlanmaydigan xato tuzatilmaydi, faqat yashiriladi |
| 2 | **Kichraytiring** — xatoni ko'rsatadigan eng kichik kirish/kodni toping (yarmini olib tashlang — xato qoldimi?) | kam kodda sabab ko'rinadi (ikkilik qidiruv g'oyasi, 28-bob) |
| 3 | **Faraz qiling va tekshiring** — "menimcha x bu yerda NULL" → `print` yoki `assert` bilan tasdiqlang | farazsiz o'zgartirishlar — vaqt yo'qotish |
| 4 | **Birinchi noto'g'ri narsani toping**, oxirgisini emas | qulash — oqibat; sabab ancha oldinda (bufer to'lishi 1000 qator oldin bo'lgan) |
| 5 | **Tuzatgach — test qo'shing** | xato qaytib kelmasin |
| 6 | **Rezina o'rdak** — muammoni kimgadir (yoki o'yinchoqqa) ovoz chiqarib tushuntiring | tushuntirish paytida o'zingiz topasiz |

> **Eslab qoling:** takrorla → kichraytir → faraz qil → tekshir → tuzat → test qo'sh. Eng muhimi: **birinchi** noto'g'ri holatni toping, qulash joyini emas.

## 29.2. `printf` izlari — hali ham eng ko'p ishlatiladigan vosita

**Oddiy qilib aytganda:** dasturga "shu yerga keldim, x = 5" deb yozuvchi qatorlar qo'shasiz va dastur qaysi yo'ldan o'tganini ko'rasiz. **Hayotdan misol:** ertakdagi bolalar o'rmonda adashmaslik uchun non ushoqlari tashlab ketgan.

```c
fprintf(stderr, "[debug] %s:%d n=%zu p=%p\n", __FILE__, __LINE__, n, (void *)p);
```

| Qism | Ma'nosi |
|---|---|
| `stderr` | **buferlanmaydi** — dastur qulashidan oldin ham chiqadi (`stdout` esa qulashda yo'qolishi mumkin, 12-bob) |
| `__FILE__`, `__LINE__` | kompilyator joylagan fayl nomi va qator raqami (10-bob) |
| `%zu`, `%p` | `size_t` va ko'rsatkich uchun |

Yadroda — `kprintf` (serial port + `dmesg`). Muhim o'zgaruvchilarni **funksiya kirish/chiqishida** chiqarish ko'pincha gdb'dan tezroq natija beradi.

**Muammo:** izlarni keyin qo'lda o'chirish noqulay, unutib qoldirsangiz — "chiqarish" qoladi. **Yechim:** izlarni makro bilan yozing — `-DDEBUG` bilan yig'ilsa chiqaradi, usiz — jim. Shu bobning to'liq dasturida (oxirida) buni ko'rasiz.

## 29.3. Bitta xato — to'rt vosita

Eng yaxshi o'rganish usuli — **bitta xatoni bir necha vosita bilan ushlash**. Quyidagi dasturda ataylab xato bor: kutilgan natija 15, lekin chiqadigan — boshqa.

**Bu dastur nima qiladi (umumiy):** talabaning 3 ta bahosini kiritadi (hammasi 5) va yig'indini hisoblaydi. Yig'indi (`jami`) 0 dan boshlanadi va 3 ta baho qo'shiladi → 15 chiqishi kerak.

```c
/* talaba.c - jami baho "o'z-o'zidan" o'zgarib qoladi (xato bor!) */
#include <stdio.h>

struct talaba {
    int baho[3];
    int jami;
};

static void baholar_kirit(struct talaba *t)
{
    for (int i = 0; i <= 3; i++)                /* XATO: i <= 3 emas, i < 3 bo'lishi kerak */
        t->baho[i] = 5;
}

int main(void)
{
    struct talaba t;
    t.jami = 0;                                 /* hisob 0 dan boshlanadi */
    baholar_kirit(&t);
    for (int i = 0; i < 3; i++)
        t.jami += t.baho[i];
    printf("jami = %d (kutilgan: 15)\n", t.jami);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g talaba.c -o talaba
$ ./talaba
jami = 20 (kutilgan: 15)
```

Kompilyator hech narsa demadi (`-Wall -Wextra` bilan ham), dastur **ishladi** — lekin natija noto'g'ri: 15 o'rniga 20.

**Kodda nimalar bor:**

| Qism | Vazifasi |
|---|---|
| `struct talaba { int baho[3]; int jami; }` | 3 ta baho va yig'indi — xotirada ketma-ket: `baho[0]`, `baho[1]`, `baho[2]`, so'ng `jami` |
| `t.jami = 0` | yig'indini nollaymiz |
| `baholar_kirit` | hamma bahoni 5 qiladi. **Xato:** sikl `i <= 3`, ya'ni `baho[3]` ga ham yozadi — bunday element yo'q! |
| ikkinchi sikl | 3 ta bahoni `jami` ga qo'shadi |

Nega 20? `baho[3]` — massivdan **keyingi** katak, xotirada bu aynan `jami`! Shuning uchun `baholar_kirit` `jami` ni 5 qilib qo'ydi (nolni yo'qotdi), keyin 3 × 5 = 15 qo'shildi → 20.

**Qaysi vosita buni topadi?** Quyida ketma-ket sinaymiz:

### 1) Faqat ASan

```console
$ gcc -Wall -Wextra -g -fsanitize=address talaba.c -o talaba_asan
$ ./talaba_asan
jami = 20 (kutilgan: 15)
```

**Hech narsa demadi!** AddressSanitizer **butun obyektdan** (struktura ichidagi `baho` + `jami`) chiqishni kuzatadi; `baho[3]` hali struktura ichida bo'lgani uchun u buni "yaxshi xotira" deb hisoblaydi. **Dars:** hech bir vosita hammasini topmaydi.

### 2) UBSan

```console
$ gcc -Wall -Wextra -g -fsanitize=undefined talaba.c -o talaba_ub
$ ./talaba_ub 2>&1
talaba.c:12:16: runtime error: index 3 out of bounds for type 'int [3]'
jami = 20 (kutilgan: 15)
```

UBSan **massiv chegarasini** biladi: `int baho[3]` ga `3`-indeks bilan murojaat qilinganini aniq **qator va ustun** bilan aytdi (`talaba.c:12`).

### 3) gdb watchpoint — "kim buzdi?"

**Watchpoint** — "bu o'zgaruvchi **o'zgarganda** dasturni to'xtat". **Hayotdan misol:** seyfga kim tegsa — darhol xabar beruvchi signalizatsiya.

```console
$ gdb -q -batch -ex 'break 19' -ex run -ex 'watch t.jami' -ex continue -ex bt -ex 'print i' ./talaba 2>&1 | grep -vE 'libthread_db|^\[Thread' | sed 's/0x[0-9a-f]\{6,\}/0xADRES/g'
Breakpoint 1 at 0x11bf: file talaba.c, line 19.

Breakpoint 1, main () at talaba.c:19
19	    baholar_kirit(&t);
Hardware watchpoint 2: t.jami

Hardware watchpoint 2: t.jami

Old value = 0
New value = 5
baholar_kirit (t=0xADRES) at talaba.c:11
11	    for (int i = 0; i <= 3; i++)                /* XATO: i <= 3 emas, i < 3 bo'lishi kerak */
#0  baholar_kirit (t=0xADRES) at talaba.c:11
#1  0xADRES in main () at talaba.c:19
$1 = 3
```

**gdb buyruqlari tahlili:**

| Buyruq | Nima qildi |
|---|---|
| `break 19` | 19-qatordan (`baholar_kirit(&t)` chaqiruvi) oldin to'xtash nuqtasi |
| `run` | dasturni ishga tushirdi — 19-qatorda to'xtadi |
| `watch t.jami` | `t.jami` ga **kuzatuvchi** qo'ydi (protsessor apparati — hardware watchpoint) |
| `continue` | davom etdi; `t.jami` o'zgargan zahoti to'xtadi |
| `bt` | chaqiruvlar zanjiri: `baholar_kirit` ichidamiz, uni `main` 19-qatordan chaqirgan |
| `print i` | sikl o'zgaruvchisi |

**Nima ko'rdik:** `t.jami` `0` dan `5` ga o'zgardi — hech kim buni kutmagan edi — va aynan **`baholar_kirit` ichida**, `i = 3` bo'lganda. (gdb yozuv **bajarilgandan keyin** to'xtaydi, shuning uchun ko'rsatilgan qator — yozuvdan keyingi qator.) Aybdor topildi: `i <= 3`.

### 4) Kompilyator ogohlantirishlari

`-Wall -Wextra` bu holatda jim edi. Shuning uchun **sanitizer + gdb** — yagona himoya emas, lekin birgalikda ishonchli.

> **Eslab qoling:** qiymat "o'z-o'zidan" o'zgarsa — **watchpoint**; massiv chegarasi — **UBSan/ASan**; hech biri hammasini ko'rmaydi, ularni birga ishlating.

## 29.4. gdb chuqur

**Oddiy qilib aytganda:** gdb — dasturni **to'xtatib**, ichiga qarash vositasi. Dunyoni "pauza" qilib, har bir o'zgaruvchi (cho'ntak) ni tekshirasiz.

Tayyorgarlik: dasturni **`-g -O0`** bilan yig'ing (`-g` — nom va qator ma'lumoti; `-O0` — optimallashtirmasdan, aks holda gdb "optimized out" ko'rsatadi):

```bash
gcc -g -O0 dastur.c -o dastur
gdb --args ./dastur arg1 arg2
```

| Buyruq | Nima qiladi |
|---|---|
| `break fayl.c:42` / `b funksiya` | to'xtash nuqtasi |
| `break 42 if i == 1000` | **shartli** to'xtash — sikl ichidagi 1000-aylanishda |
| `watch x` / `watch -l p->qiymat` | **watchpoint**: qiymat o'zgarganda to'xtash |
| `rwatch`, `awatch` | o'qilganda / har qanday murojaatda |
| `run`, `continue` (`c`), `next` (`n`), `step` (`s`), `finish` | boshqarish: `next` — qatordan o'tish, `step` — funksiya ichiga kirish, `finish` — funksiyadan chiqish |
| `until 50` | 50-qatorgacha (siklni tugatish) |
| `bt` / `bt full` | chaqiruvlar zanjiri (+ lokal o'zgaruvchilar) |
| `frame 3`, `up`, `down` | zanjir bo'ylab yurish |
| `print *p`, `print a[0]@10` | qiymat; massivning 10 ta elementi |
| `print/x val`, `x/16xb p` | o'n oltilikda; xotirani baytlab ko'rish |
| `info registers`, `x/10i $rip` | registrlar; keyingi 10 buyruq |
| `display i` | har qadamda avtomatik ko'rsatish |
| `set var x = 5` | ish vaqtida o'zgaruvchini o'zgartirish |
| `layout src` / `layout asm` / `tui enable` | matnli interfeys: kod va assembly yonma-yon |
| `thread apply all bt` | hamma oqimlarning zanjiri (deadlock'da!) |

### Qulagan dasturni tekshirish (Segmentation fault)

**Bu dastur nima qiladi (umumiy):** mijozlar ro'yxatidan ism bo'yicha mijozni topadi va yoshini chiqaradi. Ro'yxatda bor mijoz ("Ali") uchun ishlaydi, yo'q mijoz ("Gulnora") uchun `topish` `NULL` qaytaradi va `yosh_ol` `NULL` ga murojaat qilib **qulaydi**.

```c
/* qulash.c - Segmentation fault: ichma-ich chaqiruvdagi NULL */
#include <stdio.h>

struct mijoz {
    const char *ism;
    int yosh;
};

static int yosh_ol(const struct mijoz *m)
{
    return m->yosh;                             /* m == NULL bo'lsa - qulaydi */
}

static const struct mijoz *topish(const struct mijoz *royxat, int n, const char *ism)
{
    for (int i = 0; i < n; i++)
        if (royxat[i].ism[0] == ism[0])         /* birinchi harf mos kelsa */
            return &royxat[i];
    return NULL;                                /* topilmadi */
}

int main(void)
{
    struct mijoz royxat[] = { { "Ali", 30 }, { "Vali", 25 } };
    printf("Ali: %d yosh\n", yosh_ol(topish(royxat, 2, "Ali")));
    printf("Gulnora: %d yosh\n", yosh_ol(topish(royxat, 2, "Gulnora")));
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g qulash.c -o qulash
$ bash -c 'stdbuf -oL ./qulash 2>&1 | head -5; echo "chiqish kodi: ${PIPESTATUS[0]}"'
Ali: 30 yosh
chiqish kodi: 139
```

`stdbuf -oL` — chiqishni qatorma-qator buferlaydi (aks holda `stdout` fayl/quvurga yo'naltirilganda qulashdan oldingi qator yo'qolishi mumkin). **Chiqish kodi 139 = 128 + 11**: jarayon 11-signal (`SIGSEGV`) bilan o'ldirildi.

Endi gdb bilan **qayerda va nega** qulaganini topamiz:

```console
$ gdb -q -batch -ex run -ex bt -ex 'frame 1' -ex 'print royxat' ./qulash 2>&1 | grep -vE 'libthread_db|^\[Thread' | sed 's/0x[0-9a-f]\{6,\}/0xADRES/g'

Program received signal SIGSEGV, Segmentation fault.
0xADRES in yosh_ol (m=0x0) at qulash.c:11
11	    return m->yosh;                             /* m == NULL bo'lsa - qulaydi */
#0  0xADRES in yosh_ol (m=0x0) at qulash.c:11
#1  0xADRES in main () at qulash.c:26
#1  0xADRES in main () at qulash.c:26
26	    printf("Gulnora: %d yosh\n", yosh_ol(topish(royxat, 2, "Gulnora")));
$1 = {{ism = 0xADRES "Ali", yosh = 30}, {ism = 0xADRES "Vali", yosh = 25}}
```

**Nima ko'rdik:**

| gdb chiqishi | Ma'nosi |
|---|---|
| `Program received signal SIGSEGV` | dastur qulagan joyda gdb to'xtadi |
| `yosh_ol (m=0x0) at qulash.c:11` | qulash `yosh_ol` ichida, 11-qatorda; argument `m = 0x0` (**NULL**) |
| `#1 ... main () at qulash.c:26` | `yosh_ol` ni `main` 26-qatordan chaqirgan — "Gulnora" qatori |
| `frame 1` + `print royxat` | `main` kadriga o'tib, uning lokal o'zgaruvchisini ko'rdik |

Sabab: `topish("Gulnora")` hech narsa topmadi va `NULL` qaytardi; chaqiruvchi tekshirmadi. **Tuzatish** — `NULL` ni tekshirish. Qulash joyi (`yosh_ol`) — oqibat; **sabab** — `main` dagi tekshirilmagan natija (4-qoida!).

### Core dump — qulagan dasturning "surati"

**Oddiy qilib aytganda:** dastur qulaganda OS uning butun xotirasini faylga yozishi mumkin (**core**). Keyin dasturni qayta ishga tushirmasdan, qulash paytidagi holatni gdb bilan ko'rish mumkin. Ayniqsa kamdan-kam uchraydigan, takrorlab bo'lmaydigan qulashlar uchun foydali.

```bash
ulimit -c unlimited              # core fayllarga ruxsat (joriy terminal uchun)
./dastur                          # Segmentation fault (core dumped)
gdb ./dastur core                 # qulagan paytdagi holat: bt, print...
```

(Zamonaviy Linux'da core'lar `systemd-coredump` ga tushishi mumkin: `coredumpctl gdb`.)

**Osilib qolgan dasturga ulanish:** `gdb -p PID`, keyin `thread apply all bt` — hamma oqim qayerda kutyapti.

**Yadroni debug qilish** (MyOS): `make debug` — QEMU gdb'ni kutadi; boshqa terminalda `gdb -x tools/gdbinit`. Xuddi shu buyruqlar yadroda ham ishlaydi: `break kmain`, `break mm_handle_fault` (page fault), `break handle_exception`, `bt`, `info registers`. `docs/08-test-debug.md` — batafsil.

> **Eslab qoling:** qulasa — `gdb` → `run` → `bt` (kim chaqirdi?) → `frame N` + `print` (o'zgaruvchilar). Qiymat buzilsa — `watch`. Tanlangan oqim/jarayon osilsa — `gdb -p`.

## 29.5. Sanitizer'lar — aeroport rentgeni

**Oddiy qilib aytganda:** sanitizer — kompilyator dasturga qo'shimcha tekshiruv kodini joylaydigan vosita. Har xotira murojaati tekshiriladi va xato **sodir bo'lgan zahoti**, aniq joyi bilan ko'rsatiladi (ancha keyin, boshqa joyda emas). Biroz sekinlashtiradi.

| Bayroq | Nima topadi | Sekinlashish |
|---|---|---|
| `-fsanitize=address` (ASan) | chegaradan chiqish, use-after-free, double free, leak | ~2x |
| `-fsanitize=undefined` (UBSan) | toshish, noto'g'ri surish, NULL, tekislanmagan, massiv chegarasi | kam |
| `-fsanitize=thread` (TSan) | poyga holatlari | 5–15x (ASan bilan birga emas) |
| `-fsanitize=memory` (MSan, faqat clang) | boshlanmagan xotirani o'qish | 3x |

Linux yadrosida analoglari: KASAN, UBSAN, KCSAN.

### ASan: heap'dan chiqish va bo'shatilgan xotira

**Bu dastur nima qiladi (umumiy):** 4 elementli massiv ajratadi. Argumentsiz ishga tushirilsa — **5-elementga yozadi** (heap overflow); argument bilan — massivni `free` qilib, keyin uning elementini **o'qiydi** (use-after-free).

```c
/* tosh.c - xotirada chegaradan chiqish va bo'shatilgan xotiradan foydalanish */
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    (void)argv;
    int *a = malloc(4 * sizeof(int));
    for (int i = 0; i < 4; i++)
        a[i] = i;
    if (argc == 1) {
        a[4] = 99;                              /* XATO 1: 4 ta element, a[4] yo'q (heap overflow) */
    } else {
        free(a);
        printf("%d\n", a[0]);                   /* XATO 2: bo'shatilgan xotirani o'qish (use-after-free) */
        return 0;
    }
    printf("%d\n", a[3]);
    free(a);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address tosh.c -o tosh   # xato kutiladi
tosh.c: In function ‘main’:
tosh.c:15:9: warning: pointer ‘a’ used after ‘free’ [-Wuse-after-free]
   15 |         printf("%d\n", a[0]);                   /* XATO 2: bo'shatilgan xotirani o'qish (use-after-free) */
      |         ^~~~~~~~~~~~~~~~~~~~
tosh.c:14:9: note: call to ‘free’ here
   14 |         free(a);
      |         ^~~~~~~
$ ./tosh 2>&1 | grep -E "ERROR|WRITE of|#0 .*tosh.c|SUMMARY" | sed 's/0x[0-9a-f]*/0xADRES/g; s/==[0-9]*==/==PID==/; s#/[^ :]*/##g'
==PID==ERROR: AddressSanitizer: heap-buffer-overflow on address 0xADRES at pc 0xADRES bp 0xADRES sp 0xADRES
WRITE of size 4 at 0xADRES thread T0
    #0 0xADRES in main tosh.c:12
SUMMARY: AddressSanitizer: heap-buffer-overflow tosh.c:12 in main
$ ./tosh x 2>&1 | grep -E "ERROR|READ of|#0 .*tosh.c|SUMMARY" | sed 's/0x[0-9a-f]*/0xADRES/g; s/==[0-9]*==/==PID==/; s#/[^ :]*/##g'
==PID==ERROR: AddressSanitizer: heap-use-after-free on address 0xADRES at pc 0xADRES bp 0xADRES sp 0xADRES
READ of size 4 at 0xADRES thread T0
    #0 0xADRES in main tosh.c:15
SUMMARY: AddressSanitizer: heap-use-after-free tosh.c:15 in main
```

**Qanday o'qiladi:**

| ASan satri | Ma'nosi |
|---|---|
| `ERROR: AddressSanitizer: heap-buffer-overflow` | **xato turi**: heap'dagi buferdan chiqish |
| `WRITE of size 4` | 4 baytlik **yozuv** (o'qish bo'lsa `READ`) |
| `#0 ... in main tosh.c:12` | aniq **qator**: 12-qator, `a[4] = 99` |
| `heap-use-after-free` | ikkinchi xato turi: bo'shatilgan xotira |
| `SUMMARY` | qisqa xulosa: tur + qator |

(Birinchi buyruqdagi `# xato kutiladi` izohi: kompilyator `use-after-free` ni o'zi ham ogohlantiradi — `-Wuse-after-free`; bu ogohlantirish ataylab qoldirilgan.)

Oldingi bobda (8) qo'lda qidirgan xatolarni ASan **soniyada, qator raqami bilan** topdi.

### TSan: poyga

**Bu dastur nima qiladi (umumiy):** ikki oqim bitta global sanagichni qulfsiz 100 000 martadan oshiradi — klassik poyga (15-bob).

```c
/* poyga.c - ikki oqim, himoyalanmagan sanagich */
#include <pthread.h>
#include <stdio.h>

static long sanagich;

static void *ishchi(void *arg)
{
    (void)arg;
    for (int i = 0; i < 100000; i++)
        sanagich++;
    return NULL;
}

int main(void)
{
    pthread_t a, b;
    pthread_create(&a, NULL, ishchi, NULL);
    pthread_create(&b, NULL, ishchi, NULL);
    pthread_join(a, NULL);
    pthread_join(b, NULL);
    printf("sanagich = %ld\n", sanagich);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g -fsanitize=thread -pthread poyga.c -o poyga_t
$ ./poyga_t 2>&1 | grep SUMMARY | sort -u | sed 's#/[^ :]*/##g'
SUMMARY: ThreadSanitizer: data race poyga.c:11 in ishchi
```

TSan poygani **qator** (`poyga.c:11`, `sanagich++`) bilan ko'rsatdi. Natijaning o'zi (`sanagich = ...`) har safar boshqacha bo'lardi, TSan esa poygani ishlash natijasiga qaramay aniqlaydi.

Mashqlardagi tekshiruvchi ASan + UBSan ishlatadi. Ko'p oqimli kodni (29, 34, 38, 44, 45-mashqlar) qo'shimcha ravishda TSan bilan sinang:

```bash
gcc -g -fsanitize=thread -pthread -Imashqlar -Imashqlar/29_oqimlar \
    mashqlar/29_oqimlar/yechim.c mashqlar/29_oqimlar/test.c -o t29 && ./t29
```

> **Eslab qoling:** xotira xatosi → **ASan**; UB (toshish, chegara) → **UBSan**; ko'p oqimli noto'g'ri natija → **TSan**. Ko'p mashqlarda `-fsanitize=address,undefined` ni doim yoqing.

## 29.6. Valgrind — sinchkov inspektor

**Oddiy qilib aytganda:** valgrind dasturni **qayta yig'masdan** tekshiradi: uni virtual protsessorda bajaradi va har xotira murojaatini kuzatadi. Sanitizer'dan **20–50 marta** sekin, lekin boshlanmagan qiymatlardan foydalanishni ham topadi.

```bash
valgrind --leak-check=full --track-origins=yes ./dastur
```

### Sizib chiqish (leak)

**Bu dastur nima qiladi (umumiy):** `f` funksiyasi 50 bayt ajratadi va ko'rsatkichni yo'qotadi — `free` yo'q. Eng oddiy xotira sizib chiqishi.

```c
/* sizish.c - eng oddiy sizib chiqish */
#include <stdlib.h>

static void f(void)
{
    void *p = malloc(50);
    (void)p;                                    /* f tugadi, p yo'qoldi, free yo'q */
}

int main(void)
{
    f();
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g sizish.c -o sizish
$ valgrind --leak-check=full ./sizish 2>&1 | grep -E "definitely lost:|by 0x|ERROR SUMMARY" | sed 's/==[0-9]*==/==PID==/; s/0x[0-9A-F]*/0xADRES/'
==PID==    by 0xADRES: f (sizish.c:6)
==PID==    by 0xADRES: main (sizish.c:12)
==PID==    definitely lost: 50 bytes in 1 blocks
==PID== ERROR SUMMARY: 1 errors from 1 contexts (suppressed: 0 from 0)
```

**Nima ko'rdik:** `definitely lost: 50 bytes in 1 blocks` — 50 bayt qaytarilmadi; ajratilgan joy `f (sizish.c:6)` va uni `main` 12-qatordan chaqirgan.

### Boshlanmagan qiymat

**Bu dastur nima qiladi (umumiy):** `chegirma` o'zgaruvchisiga qiymat berilmaydi, lekin shartda ishlatiladi. Qaysi shoxga kirishi — tasodifiy (stekdagi eski qoldiqqa bog'liq).

```c
/* boshlanmagan.c - boshlang'ich qiymat berilmagan o'zgaruvchi */
#include <stdio.h>

int main(void)
{
    int chegirma;                               /* qiymat berilmadi! */
    if (chegirma > 10)                          /* qaysi shoxga kirishi noma'lum */
        printf("chegirma katta\n");
    else
        printf("chegirma kichik\n");
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g -O0 boshlanmagan.c -o boshlanmagan   # xato kutiladi
boshlanmagan.c: In function ‘main’:
boshlanmagan.c:7:8: warning: ‘chegirma’ is used uninitialized [-Wuninitialized]
    7 |     if (chegirma > 10)                          /* qaysi shoxga kirishi noma'lum */
      |        ^
boshlanmagan.c:6:9: note: ‘chegirma’ was declared here
    6 |     int chegirma;                               /* qiymat berilmadi! */
      |         ^~~~~~~~
$ valgrind --track-origins=yes ./boshlanmagan 2>&1 | grep -E "Conditional|Uninitialised|ERROR SUMMARY" | sed 's/==[0-9]*==/==PID==/'
==PID== Conditional jump or move depends on uninitialised value(s)
==PID==  Uninitialised value was created by a stack allocation
==PID== ERROR SUMMARY: 1 errors from 1 contexts (suppressed: 0 from 0)
```

**Nima ko'rdik:** valgrind `Conditional jump or move depends on uninitialised value(s)` dedi — shart boshlanmagan qiymatga bog'liq; `--track-origins=yes` esa uning **qayerda yaratilganini** (`stack allocation`: `main` stekidagi o'zgaruvchi) ko'rsatdi. Bu holatni kompilyator ham ogohlantirgan (`-Wuninitialized`) — **ogohlantirishlarni o'qing**.

`valgrind --tool=callgrind` — qaysi funksiya qancha buyruq bajarayotgani.

> **Eslab qoling:** ASan — tez, qayta yig'ish kerak; valgrind — sekin, lekin qayta yig'masdan va boshlanmagan qiymatni ham topadi. Ikkisi bir-birini to'ldiradi.

## 29.7. strace — dastur OS bilan nima gaplashyapti

**Oddiy qilib aytganda:** hamma dastur OS bilan **syscall**'lar (14-bob) orqali gaplashadi. `strace` shu suhbatni **yozib** ko'rsatadi: qaysi faylni ochdi, nima o'qidi, qayerda xato oldi. **Hayotdan misol:** telefon suhbatlari yozuvi. Kodni o'qimasdan, dasturning "tashqi xatti-harakati"dan sabab topish mumkin.

**Bu dastur nima qiladi (umumiy):** `data` papkasidagi `sozlama.cfg` ni ochishga urinadi. Fayl yo'li `papka` va `nom` ni birlashtirib hosil qilinadi. Dastur "fayl topilmadi" deydi — lekin **qaysi** yo'lni izlaganini ko'rsatmaydi.

```c
/* fayl_oqi.c - "fayl topilmadi" - lekin qaysi fayl izlandi? */
#include <stdio.h>

int main(void)
{
    const char *papka = "data";
    const char *nom = "sozlama.cfg";
    char yol[64];
    snprintf(yol, sizeof(yol), "%s%s", papka, nom);     /* '/' unutilgan! */

    FILE *f = fopen(yol, "r");
    if (!f) {
        printf("sozlama fayli topilmadi!\n");
        return 1;
    }
    fclose(f);
    printf("sozlama o'qildi\n");
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g fayl_oqi.c -o fayl_oqi
$ ./fayl_oqi
sozlama fayli topilmadi!
$ strace -o iz.txt ./fayl_oqi > /dev/null; grep openat iz.txt | grep -v '/lib\|/etc/ld'
openat(AT_FDCWD, "datasozlama.cfg", O_RDONLY) = -1 ENOENT (No such file or directory)
```

**Qanday o'qiladi:** `openat(AT_FDCWD, "datasozlama.cfg", O_RDONLY) = -1 ENOENT (No such file or directory)`:

| Qism | Ma'nosi |
|---|---|
| `openat(...)` | chaqirilgan syscall |
| `"datasozlama.cfg"` | **aslida izlangan yo'l** — papka va nom orasida `/` yo'q! (`snprintf` formatida `/` unutilgan) |
| `= -1 ENOENT` | natija: xato "fayl yoki papka yo'q" |

Dastur "topilmadi" degan, strace esa **nima izlaganini** ko'rsatdi — xato bir qarashda ko'rindi.

```bash
strace ./dastur                  # hamma syscall'lar: openat, read, write, mmap...
strace -e trace=openat,read ls   # faqat tanlanganlari
strace -f -p PID                 # ishlayotgan jarayonga (bolalari bilan)
strace -c ./dastur               # statistika: qaysi syscall necha marta, qancha vaqt
ltrace ./dastur                  # kutubxona funksiyalari chaqiruvlari
```

"Dastur nega bu faylni topmayapti?", "qayerda osilib qoldi?" (oxirgi syscall — `read` yoki `futex` da kutyapti) kabi savollarga kodni o'qimasdan javob beradi.

**14-bobdagi tushunchalarni jonli ko'rish:** shell `ls | wc -l` ni bajarganda qaysi syscall'lar ishlaydi?

```console
$ strace -f -o iz2.txt sh -c 'ls | wc -l' > /dev/null; grep -oE '\b(pipe2|clone|dup2|execve|wait4)\(' iz2.txt | sort -u
clone(
dup2(
execve(
pipe2(
wait4(
```

Hammasi ko'rindi: `pipe2` (quvur), `clone` (yangi jarayon — `fork`), `dup2` (stdin/stdout ni quvurga ulash), `execve` (dasturni yuklash), `wait4` (bolani kutish) — 14-bobda o'zingiz yozgan mexanizmlar.

> **Eslab qoling:** dastur "fayl topilmadi" desa, qaysi fayl? — `strace`. "Osilib qoldi" — `strace -p PID` (oxirgi syscall qayerda to'xtagan).

## 29.8. Binar fayllarni ko'rish

**Oddiy qilib aytganda:** ba'zan manba kod yo'q yoki kompilyator nima hosil qilganini ko'rish kerak. Bunda binar faylni **o'qiladigan** ko'rinishga aylantirish vositalari ishlatiladi. Yadro dasturchisi uchun ayniqsa muhim: PANIC xabaridagi **manzilni** manba qatoriga aylantirish.

| Vosita | Nima qiladi |
|---|---|
| `nm -n dastur` | belgilar (funksiya/o'zgaruvchi nomlari) manzil bo'yicha tartiblangan |
| `addr2line -e dastur -f ADRES` | manzil → funksiya va `fayl:qator` |
| `objdump -d -M intel dastur` | disassembly (Intel sintaksisi) |
| `objdump -S dastur` | manba kod bilan aralash (`-g` bilan yig'ilgan bo'lsa) |
| `readelf -a dastur` | ELF'ning hammasi (22-bob) |
| `od -A x -t x1z fayl` yoki `xxd fayl` | baytlarni o'n oltilikda |

Yuqoridagi `talaba` dasturi bilan (29.3):

```console
$ nm -n talaba | grep -E ' [tT] (baholar_kirit|main)$'
0000000000001169 t baholar_kirit
000000000000119d T main
$ addr2line -f -e talaba $(nm talaba | awk '$3=="baholar_kirit"{print $1}') | sed 's#/[^ :]*/##g'
baholar_kirit
talaba.c:10
$ objdump -d -M intel --no-show-raw-insn talaba | sed -n '/<baholar_kirit>:/,/ret/p' | head -12
0000000000001169 <baholar_kirit>:
    1169:	endbr64
    116d:	push   rbp
    116e:	mov    rbp,rsp
    1171:	mov    QWORD PTR [rbp-0x18],rdi
    1175:	mov    DWORD PTR [rbp-0x4],0x0
    117c:	jmp    1193 <baholar_kirit+0x2a>
    117e:	mov    rax,QWORD PTR [rbp-0x18]
    1182:	mov    edx,DWORD PTR [rbp-0x4]
    1185:	movsxd rdx,edx
    1188:	mov    DWORD PTR [rax+rdx*4],0x5
    118f:	add    DWORD PTR [rbp-0x4],0x1
$ od -A x -t x1z -N 16 talaba
000000 7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00  >.ELF............<
000010
```

**Nima ko'rdik:**

| Buyruq | Natija |
|---|---|
| `nm -n` | `baholar_kirit` va `main` manzillari (kichik raqam = bu faylga nisbatan offset) |
| `addr2line` | shu manzil → funksiya `baholar_kirit` va `talaba.c:10` (funksiya tanasi boshlangan qator) |
| `objdump -d` | funksiyaning assembly kodi (17-bob): `push rbp`, `mov rbp, rsp`, ... (kompilyator versiyasiga qarab biroz boshqacha bo'lishi mumkin) |
| `od` | faylning birinchi baytlari: `7f 45 4c 46` = `\x7f ELF` — ELF sehrli raqami (22-bob) |

**`addr2line` — yadro PANIC'ini o'qish:** MyOS PANIC xabarida `RIP=0xffffffff8010abcd` bo'lsa: `addr2line -e build/kernel.elf -f 0xffffffff8010abcd` → funksiya va qator. Stek zanjiridagi har bir manzil uchun takrorlang.

> **Eslab qoling:** PANIC manzili → `addr2line`; "kompilyator nima qildi?" → `objdump -d`; "bu fayl nima?" → `od`/`readelf`.

## 29.9. perf va QEMU vositalari

### perf — dastur vaqti qayerga ketyapti

**Oddiy qilib aytganda:** `perf` protsessorning ichki hisoblagichlarini o'qiydi: nechta takt, nechta buyruq, nechta kesh xatosi, qaysi funksiyada qancha vaqt. **Hayotdan misol:** fitnes-soat. **Qoida:** optimallashtirishdan oldin **o'lchang** — dasturchilarning "qayer sekin" haqidagi taxminlari ko'pincha noto'g'ri.

```bash
perf stat ./dastur                          # umumiy: taktlar, buyruqlar, kesh xatolari, sakrash xatolari
perf stat -e cache-misses,cache-references ./dastur
perf record -g ./dastur && perf report      # qaysi funksiyalar eng ko'p vaqt oladi (chaqiruv zanjirlari bilan)
perf top                                    # jonli: butun tizimda hozir nima issiq
```

(Bu bobdagi sinov muhitida `perf` o'rnatilmagan; o'z mashinangizda `perf` paketini o'rnating.) 21-bobdagi matritsa tajribasini `perf stat -e cache-misses` bilan takrorlang.

### QEMU'ning o'z vositalari (yadro uchun)

| Vosita | Nima |
|---|---|
| `-d int,cpu_reset -no-reboot` | har bir uzilish/istisno va CPU qayta yuklanishi jurnali — triple fault'ni topish |
| `-d in_asm` | bajarilgan har bir blok (juda ko'p chiqish) |
| `-s -S` | gdb server, birinchi buyruqdan oldin to'xtash (`make debug`) |
| QEMU monitor (`Ctrl-A C` nographic rejimda) | `info registers`, `info mem` (sahifa xaritasi!), `info tlb`, `x/10i $pc`, `info pic` |
| `-serial stdio` / `-nographic` | serial port chiqishi terminalda |

## 29.10. Tipik holatlar uchun "retsept"lar

| Belgi | Birinchi qadam |
|---|---|
| Segmentation fault | ASan bilan qayta yig'ish yoki `gdb` → `run` → `bt` |
| Qiymat "o'z-o'zidan" o'zgaradi | `watch -l` o'sha o'zgaruvchiga (29.3) |
| "Fayl topilmadi" / noma'lum xato | `strace` |
| Dastur osilib qoldi | `gdb -p PID` → `thread apply all bt`; yoki `strace -p PID` |
| Ba'zan noto'g'ri natija (ko'p oqimli) | TSan; qulflarni tekshirish |
| Sekin | `perf record` → `perf report` |
| Xotira o'sib boradi | ASan/valgrind leak hisoboti |
| "Oldin ishlardi" | `git bisect` (19-bob) |
| Yadro qayta yuklanib turadi | QEMU `-d int,cpu_reset -no-reboot` |
| Yadro PANIC | `addr2line` bilan RIP va stek manzillari |
| Yadro osilib qoldi | `make debug`, `Ctrl-C` gdb'da, `bt`, `info registers` |

## Hayotdan misol va to'liq dastur

**Non ushoqlari.** Ikkilik qidiruv (28-bob) dasturi: `-DDEBUG` bilan yig'ilsa — dastur qaysi yo'ldan o'tganini yozadi (non ushoqlari), usiz — jim ishlaydi. `assert` esa kutilmagan holatni darhol to'xtatadi.

**Bu dastur nima qiladi (umumiy):** narxlar saralangan massivida uchta narxni (12000, 1500, 8000) ikkilik qidiruv bilan qidiradi va topilgan o'rinni yoki "topilmadi" ni chiqaradi. `-DDEBUG` bilan har qidiruv qadamini `stderr` ga yozadi.

```c
/* izlar.c - DEBUG izlari va assert: dastur qaysi yo'ldan o'tganini ko'rish */
#include <assert.h>
#include <stdio.h>

#ifdef DEBUG
#define IZ(...) fprintf(stderr, "  [iz] " __VA_ARGS__)
#else
/* if (0): chiqarilmaydi, lekin kompilyator argumentlarni baribir tekshiradi
 * (Linux'dagi no_printk ham shunday). Shuning uchun "ishlatilmagan o'zgaruvchi" ogohlantirishi yo'q. */
#define IZ(...) do { if (0) fprintf(stderr, __VA_ARGS__); } while (0)
#endif

static int qidir(const int *a, int n, int x)
{
    int chap = 0, ong = n - 1, qadam = 0;
    while (chap <= ong) {
        int orta = chap + (ong - chap) / 2;
        IZ("%d-qadam: chap=%d ong=%d orta=%d a[orta]=%d\n", ++qadam, chap, ong, orta, a[orta]);
        assert(orta >= 0 && orta < n);          /* chegaradan chiqsak - darhol to'xtaymiz */
        if (a[orta] == x)
            return orta;
        if (a[orta] < x)
            chap = orta + 1;
        else
            ong = orta - 1;
    }
    return -1;
}

int main(void)
{
    int narxlar[] = { 1500, 3000, 4200, 5000, 7800, 9900, 12000, 15000, 21000, 30000 };
    int n = sizeof(narxlar) / sizeof(narxlar[0]);
    int izlanganlar[] = { 12000, 1500, 8000 };

    for (int i = 0; i < 3; i++) {
        int j = qidir(narxlar, n, izlanganlar[i]);
        if (j >= 0)
            printf("%d so'm: %d-o'rinda\n", izlanganlar[i], j);
        else
            printf("%d so'm: topilmadi\n", izlanganlar[i]);
    }
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g izlar.c -o izlar
$ ./izlar
12000 so'm: 6-o'rinda
1500 so'm: 0-o'rinda
8000 so'm: topilmadi
$ gcc -Wall -Wextra -g -DDEBUG izlar.c -o izlar_debug
$ ./izlar_debug 2>&1 | head -6
  [iz] 1-qadam: chap=0 ong=9 orta=4 a[orta]=7800
  [iz] 2-qadam: chap=5 ong=9 orta=7 a[orta]=15000
  [iz] 3-qadam: chap=5 ong=6 orta=5 a[orta]=9900
  [iz] 4-qadam: chap=6 ong=6 orta=6 a[orta]=12000
  [iz] 1-qadam: chap=0 ong=9 orta=4 a[orta]=7800
  [iz] 2-qadam: chap=0 ong=3 orta=1 a[orta]=3000
$ gdb -q -batch -ex 'break qidir' -ex run -ex 'print n' -ex 'print x' -ex bt ./izlar 2>&1 | grep -E '^\$|^#' | sed 's/0x[0-9a-f]\{6,\}/0xADRES/g'
$1 = 10
$2 = 12000
#0  qidir (a=0xADRES, n=10, x=12000) at izlar.c:15
#1  0xADRES in main () at izlar.c:37
```

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `#ifdef DEBUG` ... `#define IZ(...)` | `-DDEBUG` bilan `IZ` — `stderr` ga yozuvchi; usiz — hech narsa qilmaydigan (lekin kompilyator argumentlarni baribir tekshiradi) |
| `__VA_ARGS__` | makroga berilgan hamma argumentlarni `fprintf` ga uzatadi (`printf` kabi o'zgaruvchan sonli argument) |
| `assert(orta >= 0 && orta < n)` | shart yolg'on bo'lsa — dastur xato xabari bilan to'xtaydi (chegaradan chiqishni darhol ushlash) |
| `IZ(...)` qatori | har qadamda `chap`, `ong`, `orta`, `a[orta]` ni ko'rsatadi |

**Nima ko'rdik:** oddiy ishga tushirishda faqat natija; `-DDEBUG` bilan har qidiruv qadami ko'rindi — `12000` uchun 4 qadam (`orta`: 4 → 7 → 5 → 6), `1500` uchun `chap` doim 0 bo'lib, chapga siljish. Oxirgi buyruq: gdb `qidir` funksiyasida to'xtadi, argumentlarni ko'rsatdi va `bt`
bilan "bu yerga `main` dan keldik" zanjirini chiqardi.

**Sinab ko'ring:** `ong = orta - 1;` ni `ong = orta;` qiling va 8000 ni qidiring — dastur nega to'xtamaydi? `-DDEBUG` bilan izlarni o'qib, sababini toping (`timeout 2 ./izlar_debug 2>&1 | head` bilan to'xtating).

<!-- katta:boshi -->
## Katta loyiha: o'z `malloc` kuzatuvchingiz (LD_PRELOAD)

**Umumiy fikr.** 29-bobdagi vositalar (valgrind, ASan) dasturni **tashqaridan** tekshiradi. Bu bosqichda **o'z vositamizni** yozamiz: dastur kodini **o'zgartirmasdan** uning `malloc`/`free` chaqiruvlarini **kuzatadigan** kutubxona. Sir shundaki: **dinamik linker** (22-bob) funksiya nomini **qidirish tartibi** bilan ishlaydi — va biz o'zimizning `malloc` ni **libc dan oldin** qidiriladigan qilib qo'yishimiz mumkin.

**Hayotiy o'xshatish:** telefon qo'ng'irog'ini **kotib** orqali o'tkazish. Siz "do'konga qo'ng'iroq qil" desangiz, kotib uni **yozib qo'yadi** va haqiqiy do'konga **ulaydi**. Siz hech narsani sezmaysiz, lekin kotibda **to'liq jurnal** bor.

### Qanday ishlaydi

```text
sinov_dastur  --malloc(100)-->  [ libtrace.so: malloc ]  --haqiqiy malloc-->  [ libc: malloc ]
                                      | yozib qo'yadi: (manzil, hajm, kim chaqirdi)
```

| Element | Vazifasi |
|---|---|
| `LD_PRELOAD=./libtrace.so` | muhit o'zgaruvchisi: "bu kutubxonani **birinchi** yukla" — shuning uchun `malloc` nomi avval **bizdan** topiladi |
| `dlsym(RTLD_NEXT, "malloc")` | "**mendan keyingi** `malloc` ni top" — ya'ni haqiqiy libc `malloc`i. Biz uni **chaqirib**, natijani qaytaramiz |
| `__builtin_return_address(0)` | `malloc` dan **qaytish manzili**: **kim chaqirgani** (qaysi funksiya ichidan) |
| `dladdr(manzil, &info)` | manzildan **funksiya nomini** topadi (shuning uchun test dasturi `-rdynamic` bilan yig'iladi) |
| `__attribute__((destructor))` | dastur **tugaganda** avtomatik chaqiriladigan funksiya: hisobotni shu yerda chiqaramiz |
| `__thread int ichkarida` | **qayta kirishdan himoya**: `printf` yoki `dlsym` o'zi `malloc` chaqirishi mumkin — uni **kuzatmaymiz** (aks holda cheksiz rekursiya) |

**Nozik joy — "tuxum va tovuq".** `dlsym` o'zi `calloc` chaqiradi, lekin biz haqiqiy `calloc` ni topish uchun **aynan `dlsym` ni** ishlatyapmiz! Yechim: `calloc` ichida **kichik statik massiv**dan vaqtinchalik xotira beramiz (`boshlangich[4096]`) — faqat shu bir marta.

### Kodning asosiy qismlari

**Yozuv va haqiqiy funksiyalarni yuklash:**

```c
struct yozuv {
    void *p;                                    /* berilgan ko'rsatkich */
    size_t hajm;
    void *chaqiruvchi;                          /* malloc dan qaytish manzili: kim chaqirgan (shu funksiya ichida) */
    int tirik;
};
```

```c
static void yukla(void)
{
    haqiqiy_malloc = dlsym(RTLD_NEXT, "malloc");        /* RTLD_NEXT: bizdan KEYINGI (libc dagi) malloc */
    haqiqiy_free = dlsym(RTLD_NEXT, "free");
    haqiqiy_realloc = dlsym(RTLD_NEXT, "realloc");
    haqiqiy_calloc = dlsym(RTLD_NEXT, "calloc");
    tayyor = 1;
}
```

**`malloc` o'rniga bizning funksiya.** Nomi **aynan `malloc`** — linker uni boshqasidan oldin topadi. U haqiqiy `malloc` ni chaqiradi, natijani **jadvalga yozadi** va qaytaradi:

```c
void *malloc(size_t n)
{
    if (!tayyor)
        yukla();
    void *p = haqiqiy_malloc(n);
    if (p && !ichkarida) {
        ichkarida = 1;
        yoz(p, n, __builtin_return_address(0));
        ichkarida = 0;
    }
    return p;
}
```

`free` ham shunday: jadvaldan **o'chiradi** va haqiqiy `free` ni chaqiradi. `realloc` — eski blokni o'chirib, yangisini yozadi.

**Hisobot** — dastur tugaganda: jadvalda **hali tirik** (free qilinmagan) bloklarni topadi va har biri uchun **kim ajratganini** chiqaradi:

```c
__attribute__((destructor)) static void hisobot(void)
{
    ichkarida = 1;                              /* hisobot chiqarishda printf ning malloc'i kuzatilmasin */
    fprintf(stderr, "\n=== [trace] malloc hisoboti ===\n");
    fprintf(stderr, "ajratishlar: %lu, bo'shatishlar: %lu, jami: %lu bayt, eng ko'pi bir vaqtda: %lu bayt\n", malloc_soni,
            free_soni, jami_bayt, eng_katta_bayt);

    static struct yozuv sizganlar[MAKS];
    int k = 0;
    for (int i = 0; i < MAKS; i++)
        if (jadval[i].tirik)
            sizganlar[k++] = jadval[i];
    qsort(sizganlar, (size_t)k, sizeof(sizganlar[0]), solishtir);

    unsigned long jami = 0, jami_baytlar = 0;
    for (int i = 0; i < k; i++) {
        Dl_info info;
        int bor = dladdr(sizganlar[i].chaqiruvchi, &info);
        if (bor && info.dli_fname && strstr(info.dli_fname, "libc.so"))
            continue;                           /* libc ning o'z ichki xotirasi (masalan stdout buferi): dastur xatosi emas */
        const char *kim = bor && info.dli_sname ? info.dli_sname : "?";
        fprintf(stderr, "  SIZIB CHIQDI: %5zu bayt, ajratgan funksiya: %s()\n", sizganlar[i].hajm, kim);
        jami++, jami_baytlar += sizganlar[i].hajm;
    }
    if (jami == 0)
        fprintf(stderr, "  sizib chiqish yo'q.\n");
    else
        fprintf(stderr, "  JAMI: %lu ta blok, %lu bayt sizib chiqdi\n", jami, jami_baytlar);
}
```

`libc.so` ichidan ajratilgan bloklar (masalan `stdout` buferi) **dastur xatosi emas**, shuning uchun filtrlanadi.

### Sinov dasturi (ataylab sizib chiqish bilan)

```c
/* sinov_dastur.c - ataylab sizib chiqish bor dastur. trace.c uni KOD O'ZGARTIRMASDAN kuzatadi.
   Funksiyalar static EMAS: dladdr faqat tashqariga ochiq (global) belgilarning nomini topa oladi (-rdynamic bilan) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *yaratish_a(void)
{
    char *p = malloc(100);                      /* free YO'Q: sizib chiqadi */
    strcpy(p, "birinchi");
    return p;
}

void yaratish_b(int n)
{
    for (int i = 0; i < n; i++) {
        char *p = malloc(24);                   /* har aylanishda 24 bayt, hech biri free qilinmaydi */
        snprintf(p, 24, "blok %d", i);
    }
}

void toza_ish(void)
{
    char *p = malloc(4000);
    memset(p, 1, 4000);
    p = realloc(p, 8000);                       /* realloc: eski bloklar hisobdan chiqadi, yangisi yoziladi */
    free(p);
}

int main(void)
{
    char *a = yaratish_a();
    printf("a = \"%s\"\n", a);
    yaratish_b(3);
    toza_ish();
    char *q = calloc(10, 16);
    free(q);                                    /* bu to'g'ri qaytarildi */
    puts("dastur tugadi");
    fflush(stdout);                             /* stdout buferi hisobotdan OLDIN chiqsin */
    return 0;
}
```

Kutiladigan sizib chiqish: `yaratish_a()` — 100 bayt (1 blok), `yaratish_b(3)` — 3 × 24 bayt. `toza_ish` va `calloc(10,16)` — to'g'ri `free` qilingan.

```console
$ cd katta_loyiha/tizim/29_trace
$ gcc -Wall -Wextra -O1 -fPIC -shared trace.c -o libtrace.so -ldl
$ gcc -Wall -Wextra -O0 -fno-inline -rdynamic -g sinov_dastur.c -o sinov_dastur
$ LD_PRELOAD=./libtrace.so ./sinov_dastur 2>&1
a = "birinchi"
dastur tugadi

=== [trace] malloc hisoboti ===
ajratishlar: 8, bo'shatishlar: 3, jami: 16428 bayt, eng ko'pi bir vaqtda: 12268 bayt
  SIZIB CHIQDI:   100 bayt, ajratgan funksiya: yaratish_a()
  SIZIB CHIQDI:    24 bayt, ajratgan funksiya: yaratish_b()
  SIZIB CHIQDI:    24 bayt, ajratgan funksiya: yaratish_b()
  SIZIB CHIQDI:    24 bayt, ajratgan funksiya: yaratish_b()
  JAMI: 4 ta blok, 172 bayt sizib chiqdi
```

**Nima ko'rdik:**

- `sinov_dastur` **qayta kompilyatsiya qilinmadi** va **kodi o'zgarmadi** — kuzatuv faqat `LD_PRELOAD` orqali ulandi. Xuddi shu usul bilan **istalgan** dasturni (`ls`, `python`...) kuzatish mumkin.
- Hisobotdagi `ajratishlar: 8, bo'shatishlar: 3` — **hamma** `malloc`/`calloc`/`realloc`/`free` chaqiruvlari, shu jumladan **libc ning o'zi** qilganlari (masalan, birinchi `printf` ochgan 4096 baytlik `stdout` buferi). Dasturingizdagi 7 ta chaqiruvdan tashqari kuzatuvchi libc ichidagilarni ham ko'radi.
- **`SIZIB CHIQDI: 100 bayt, yaratish_a()`** va **3 × 24 bayt, `yaratish_b()`** — kuzatuvchi nafaqat **qancha**, balki **qaysi funksiya** ajratganini ham topdi (`dladdr` + `__builtin_return_address`).
- Jami **172 bayt** = 100 + 3 × 24. Hammasi hisobga mos.
- `eng ko'pi bir vaqtda: 12268 bayt` — **eng yuqori nuqta** (peak): `realloc(8000)` paytida tirik bloklar: 100 + 3×24 + `stdout` buferi 4096 + 8000 = 12268. Peak — dastur **eng ko'pi bilan qancha RAM so'rashi**ni ko'rsatadi.

> **Eslab qoling:** `LD_PRELOAD` + `dlsym(RTLD_NEXT, ...)` — **istalgan** libc funksiyasini **kodni o'zgartirmasdan** kuzatish yoki almashtirish usuli. Bu `valgrind`, `ltrace`, `jemalloc`/`tcmalloc` ni ulash, `libfaketime` va xavfsizlik tadqiqotlarining asosi. **Qayta kirish** (o'z ichingdan o'zingni chaqirish) — eng ko'p uchraydigan xato manbai: bayroq (`ichkarida`) bilan himoyalang.

**O'zingiz qo'shing (yechimsiz):**

1. `sinov_dastur.c` da `yaratish_a` dagi sizib chiqishni **tuzating** (`free(a)` qo'shing, `main` da): hisobot nima deydi? Tuzatilgan dastur uchun "sizib chiqish yo'q" chiqadimi?
2. Kuzatuvchiga **double free** aniqlovchisini qo'shing: `free(p)` da `p` jadvalda **yo'q** bo'lsa (yoki allaqachon `tirik=0`) — ogohlantirish chiqaring.
3. Hisobotni **hajm bo'yicha tarqalish** bilan boyiting: "1–64 bayt: N ta, 65–1024: M ta, >1024: K ta" — qaysi o'lchamlar eng ko'p ajratilishini ko'rsatadi (ajratuvchilarni sozlashda shu kerak bo'ladi).
<!-- katta:oxiri -->

## Bob xulosasi (yodlash uchun)

1. **Usul asbobdan muhim:** takrorla → kichraytir → faraz qil → tekshir → tuzat → test qo'sh. **Birinchi** noto'g'ri holatni qidiring, qulash joyini emas.
2. **gdb:** `break`, `run`, `bt`, `print`, `frame`; **`watch`** — "buni kim buzdi?"; core dump — qulashning surati; `-g -O0` bilan yig'ing.
3. **Sanitizer'lar:** ASan (xotira), UBSan (UB, massiv chegarasi), TSan (poyga) — xatoni sodir bo'lgan zahoti qator bilan ko'rsatadi; hech biri hammasini topmaydi, birga ishlating.
4. **valgrind** (qayta yig'masdan, sekin, boshlanmagan qiymatni ham), **strace** (dastur ↔ OS: "qaysi fayl?", "qayerda osildi?"), **perf** (avval o'lchang).
5. Binar: `nm`, `objdump -d`, `addr2line` (PANIC manzili → qator), `od`/`readelf`. Yadro: QEMU `-d int,cpu_reset`, `make debug`.

## Savol-javob

**Savol:** `printf` bilan debug qilish "ibtidoiy"mi?
**Javob:** Yo'q — yadroda (`kprintf`) va ko'p production tizimlarda asosiy vosita. Gdb ishlamaydigan joylarda (boot, uzilish ishlovchisi, taymingga sezgir kod) izlar ba'zan yagona yo'l.

**Savol:** Nega xato chiqarmagan sanitizer "xato yo'q" degani emas?
**Javob:** Sanitizer faqat **bajarilgan yo'ldagi** xatolarni ko'radi va o'z toifasini biladi (29.3 da ASan struktura ichidagi chiqishni ko'rmadi). Test qamrovi va bir nechta vosita kerak.

**Savol:** `-O2` bilan gdb "optimized out" desa nima qilish kerak?
**Javob:** Debug uchun `-O0 -g` bilan qayta yig'ing; xato faqat `-O2` da bo'lsa — bu odatda UB belgisi (13-bob); `-O2 -g` + `-fsanitize=undefined` sinang.

## O'zingizni tekshiring

1. "Bu o'zgaruvchini kim o'zgartiryapti?" — qaysi gdb buyrug'i?
2. Dastur osilib qoldi — qaysi ikkita vosita bilan qayerda turganini ko'rasiz?
3. Nega ASan va TSan birga ishlatilmaydi?
4. Yadro PANIC'idagi RIP manzilini qanday qilib manba qatoriga aylantirasiz?
5. Optimallashtirishdan oldin nima qilish kerak?

<details><summary>Javoblar</summary>

1. `watch` (yoki `watch -l` manzil bo'yicha).
2. `gdb -p PID` + `thread apply all bt`; `strace -p PID`.
3. Ikkalasi ham xotirani o'zicha kuzatadi ("soya xotira") va bir-biriga zid; alohida yig'iladi.
4. `addr2line -e build/kernel.elf -f <RIP>`.
5. O'lchash (`perf`) — sekin joyni aniq topish.
</details>

## Mashq

### Isitish: xato ovchisi ★☆☆ — eng osoni, avval shuni qiling

Faqat 0–29-boblar kerak (sanitizer hisobotlarini o'qish). Bu safar skelet **ataylab xatoli**: izohlar qaysi sanitizer nimani aytishini ko'rsatadi,
xatoni o'zingiz topib tuzatasiz. Bir vaqtda bitta xato: tuzating, qayta yig'ing, keyingisini o'qing.

```c
/* isitish.c - 29-bob, isitish: XATO OVCHISI. Bu dasturda 4 ta xato bor - sanitizer'lar ularni ko'rsatadi.
 * Usul (29.1): ishga tushiring -> hisobotning BIRINCHI qatorini o'qing (xato turi) -> "#0 ... isitish.c:QATOR"
 * (qayerda) -> tuzating -> qayta ishga tushiring. Bir vaqtda bitta xato. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* XATO 1 (AddressSanitizer: heap-buffer-overflow): satr uchun joy hisobida nimadir yetishmaydi. */
static char *nusxa(const char *s)
{
    char *p = malloc(strlen(s));
    if (p)
        strcpy(p, s);
    return p;
}

/* XATO 2 (UndefinedBehaviorSanitizer: signed integer overflow): 13! int ga sig'maydi.
 *       Turini o'zgartiring (funksiya, f va main dagi printf formati ham). */
static int faktorial(int n)
{
    int f = 1;
    for (int i = 2; i <= n; i++)
        f *= i;
    return f;
}

int main(void)
{
    char *n = nusxa("xatolar ovchisi");
    if (!n)
        return 1;
    printf("nusxa: %s\n", n);
    /* XATO 3 (LeakSanitizer: detected memory leaks): dastur oxirida chiqadi - malloc qilingan narsa qaytarilmadi. */

    printf("13! = %d\n", faktorial(13));

    /* XATO 4 (UndefinedBehaviorSanitizer: index 5 out of bounds; usiz - ASan stack-buffer-overflow): sikl chegarasi. */
    int massiv[5] = {1, 2, 3, 4, 5};
    int yig = 0;
    for (int i = 0; i <= 5; i++)
        yig += massiv[i];
    printf("yig'indi: %d\n", yig);
    return 0;
}
```

**Kutilgan natija** (`darslik/loyihalar/29_xato_ovchisi/isitish.txt`):

```text
nusxa: xatolar ovchisi
13! = 6227020800
yig'indi: 15
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address,undefined isitish.c -o isitish
$ ./isitish | diff - ~/C_loyha/darslik/loyihalar/29_xato_ovchisi/isitish.txt && echo "TO'G'RI"
TO'G'RI
```

### Keyingi mashqlar

- 18-mashqni ASan'siz yig'ib, `valgrind` bilan xatolarni toping — hisobotlarni solishtiring.
- `strace -f sh -c 'ls | wc -l'` chiqishida `pipe2`, `clone`, `dup2`, `execve` qatorlarini topib, 14-bob bilan bog'lang.
- MyOS'da: `make run-nographic APPEND=demo=uaf` — slab qanday ushlashini ko'ring; `make debug` bilan `kmain` da to'xtab, `bt` va `info registers` ni sinang.

<!-- loyiha:boshi -->
## Loyiha: xotira sizib chiqishini ushlagich (mini leak detector)

**Maqsad:** o'zingiz `malloc`/`free` ni **kuzatib** boradigan kichik vosita yaratish. AddressSanitizer, valgrind va yadroning `kmemleak` i
ichida aynan shu g'oya bor: har bir ajratishni yodda tut, `free` da o'chir, oxirida qolganlarni ko'rsat (29.4–29.5).
**Bobdan ishlatiladi:** makrolar (`__FILE__`, `__LINE__`), o'z ajratuvchi qatlami, jadval, xatolarni bildirish.

**Talab:** `MALLOC(n)` va `FREE(p)` makrolari. Har ajratish qayerdan (fayl:qator) va qancha ekanini eslab qolsin. `FREE` noto'g'ri
ko'rsatkich (ajratilmagan yoki ikki marta bo'shatilgan) ni **ushlasin**. Dastur oxirida `hisobot()` sizib chiqqanlarni ro'yxat qilsin.
**Nega makro?** Funksiya ichida `__LINE__` funksiyaning o'z qatorini beradi; makro esa **chaqirilgan joyning** qatorini beradi (10-bob).

```c
/* ushlagich.c - malloc/free kuzatuvchisi */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAKS 64

struct yozuv {
    void *p;
    size_t hajm;
    const char *fayl;
    int qator;
};
static struct yozuv jadval[MAKS];
static int xato_soni;

static void *kuzat_malloc(size_t n, const char *fayl, int qator)
{
    void *p = malloc(n);
    for (int i = 0; p && i < MAKS; i++)
        if (!jadval[i].p) {
            jadval[i] = (struct yozuv){ p, n, fayl, qator };
            break;
        }
    return p;
}

static void kuzat_free(void *p, const char *fayl, int qator)
{
    for (int i = 0; i < MAKS; i++)
        if (jadval[i].p == p) {
            jadval[i].p = NULL;
            free(p);
            return;
        }
    xato_soni++;
    printf("XATO %s:%d: free() noto'g'ri yoki ikki marta chaqirildi\n", fayl, qator);
}

#define MALLOC(n) kuzat_malloc((n), __FILE__, __LINE__)
#define FREE(p) kuzat_free((p), __FILE__, __LINE__)

static void hisobot(void)
{
    size_t jami = 0;
    int soni = 0;
    for (int i = 0; i < MAKS; i++)
        if (jadval[i].p) {
            printf("  SIZIB CHIQDI: %zu bayt, ajratilgan joy %s:%d\n", jadval[i].hajm, jadval[i].fayl, jadval[i].qator);
            jami += jadval[i].hajm;
            soni++;
        }
    printf("Hisobot: %d ta sizib chiqish (%zu bayt), %d ta noto'g'ri free()\n", soni, jami, xato_soni);
}

static void sizdiruvchi(void)
{
    char *c = MALLOC(50);
    (void)c;                                    /* funksiyadan chiqdik, c yo'qoldi: SIZIB CHIQISH */
}

int main(void)
{
    char *a = MALLOC(100);
    char *b = MALLOC(200);
    strcpy(a, "salom");
    FREE(a);
    FREE(a);                                    /* XATO: ikki marta */
    FREE(b);
    int x;
    FREE(&x);                                   /* XATO: ajratilmagan manzil */
    sizdiruvchi();
    hisobot();
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g ushlagich.c -o ushlagich
$ ./ushlagich
XATO ushlagich.c:68: free() noto'g'ri yoki ikki marta chaqirildi
XATO ushlagich.c:71: free() noto'g'ri yoki ikki marta chaqirildi
  SIZIB CHIQDI: 50 bayt, ajratilgan joy ushlagich.c:58
Hisobot: 1 ta sizib chiqish (50 bayt), 2 ta noto'g'ri free()
```

Bizning kuzatuvchi **fayl:qator** bilan aytdi: 58-qatorda ajratilgan 50 bayt qaytarilmagan; 68- va 71-qatorlardagi `FREE` lar noto'g'ri.
Ikki marta `free` ni bizning qatlam **o'zi** ushladi va haqiqiy `free` ga yetkazmadi.

Endi shu xatoni boshqa vositalar qanday ko'rishini solishtiring. Eng oddiy sizib chiqish:

```c
/* sizish.c - eng oddiy sizib chiqish */
#include <stdlib.h>

static void f(void)
{
    void *p = malloc(50);
    (void)p;                                    /* f tugadi, p yo'qoldi, free yo'q */
}

int main(void)
{
    f();
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address sizish.c -o s_asan && ./s_asan 2>&1 | grep -E "SUMMARY"
SUMMARY: AddressSanitizer: 50 byte(s) leaked in 1 allocation(s).
$ gcc -Wall -Wextra -g sizish.c -o s_val && valgrind --leak-check=full ./s_val 2>&1 | grep -E "definitely lost:"
==2234==    definitely lost: 50 bytes in 1 blocks
```

AddressSanitizer (LeakSanitizer) va valgrind ikkalasi ham 50 baytni topdi. Nega ular `ushlagich.c` dagi sizishni **ko'rmadi**? Bizning kuzatuvchi jadvali ko'rsatkichni
saqlab turibdi, shuning uchun sanitizer uni "hali erishish mumkin" (still reachable) deb hisoblaydi — sizib chiqish emas. Bu — muhim saboq:
vositalar bir-birini to'ldiradi, hech biri hammasini ko'rmaydi.

**Kengaytiring:** `REALLOC(p, n)` makrosini qo'shing. `MAKS` dan ko'p ajratish bo'lsa nima bo'ladi — jimgina yo'qoladi (yozuvsiz). Buni qanday oshkor qilasiz?

## Mustaqil loyiha: 5 ta xatoni toping ★★★

**Vazifa:** sizga ataylab **5 ta xatosi bor** `ombor.c` beriladi (`darslik/loyihalar/29_xato_ovchisi/ombor.c`). Har birini **vosita yordamida** toping va tuzating —
kodni birinchi qarashda o'qib emas. Bu — haqiqiy dasturchining kundalik ishi (29-bob).

**Dastur:** mahsulotlar ombori (nom, narx, soni): qo'shish, jami qiymat, eng qimmat mahsulot, chegirmali narx, hisobot.

**Ish tartibi (har xato uchun):**
1. Yig'ing: `gcc -Wall -Wextra -g -fsanitize=address,undefined ombor.c -o ombor && ./ombor`
2. Vosita xabarini o'qing: **qaysi qator**, **qanday xato turi**?
3. Sababni o'zingiz tushuntiring (daftarga bir gap): "Bu xato nima uchun yuz beryapti?"
4. Tuzating, 1-qadamga qayting — keyingi xato chiqquncha.

**Vositalar ro'yxati** (har xato boshqasida ko'rinadi): AddressSanitizer (`-fsanitize=address`, u LeakSanitizer ni ham o'z ichiga oladi),
UBSan (`-fsanitize=undefined`), `valgrind ./ombor`, `gdb` (`bt`, `print`).

**Maqsad:** hamma xato tuzatilgach, dastur **sanitizer bilan ham jim** ishlaydi va quyidagini chiqaradi:

**Kutilgan natija** (`darslik/loyihalar/29_xato_ovchisi/kutilgan.txt`):

```text
Jami qiymat: 204000
Eng qimmat: Juda uzun nomli (50000 so'm)
Chegirmali narx (30000000 so'm, 10%): 27000000
Mahsulot turlari: 5
```

**Maslahat** (yechim emas):
- Bitta ishga tushirishda sanitizer odatda **birinchi** xatoda to'xtaydi. Tuzatib, qayta ishga tushirasiz — keyingisi chiqadi. Bu normal.
- Qator raqamlari `ombor.c:28` shaklida beriladi. `gdb ./ombor` → `run` → `bt` to'liq chaqiruvlar zanjirini ko'rsatadi.
- Beshta xato **turlicha**: bufer chegarasidan yozish, chegaradan o'qish, o'chirilgan xotiraga murojaat, sizib chiqish, ishorali toshish.
- "Tasodifan ishlab ketgan" dastur xatosiz emas: sanitizersiz yig'ib ishga tushirib ko'ring — ehtimol, hech narsa sezmaysiz. Nega bu xavfli?
- Yechim kalitlari: `darslik/loyihalar/29_xato_ovchisi/javoblar.txt` — **faqat o'zingiz urinib ko'rgach** oching.

**Tekshirish:**

```bash
D=~/C_loyha/darslik/loyihalar/29_xato_ovchisi
cp $D/ombor.c . && gcc -Wall -Wextra -g -fsanitize=address,undefined ombor.c -o ombor
./ombor            # 5 marta tuzatib, qayta ishga tushiring
./ombor | diff - $D/kutilgan.txt && echo "TO'G'RI"
```
<!-- loyiha:oxiri -->

Keyingi bob: [30-bob. Yadro arxitekturasi va Linux'ga yo'l](30-yadro-arxitekturasi.md)
