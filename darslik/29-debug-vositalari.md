# 29-bob. Debug va profiling vositalari

> **Bu bobdan keyin:** xatoni tizimli ravishda topish usulini, gdb'ning chuqur imkoniyatlarini
> (watchpoint, shartli to'xtash, core dump), sanitizer'lar, valgrind, strace, ltrace, perf, objdump,
> addr2line va QEMU monitor'ini bilasiz. Dasturchi vaqtining yarmi — debug; bu bob o'sha yarmini tezlashtiradi.

> **To'liq ishlaydigan misol:** [misollar/29_xatoli.c](misollar/29_xatoli.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Hayotdan misollar

**Debug — detektiv ishi (29.1).** Detektiv tasodifiy odamlarni hibsga olmaydi. U **dalillarni** yig'adi,
**gipoteza** quradi ("qotil bog' eshigidan kirgan"), uni **tekshiradi** va noto'g'ri bo'lsa — yangisini
quradi. Xatoni qidirish ham shunday: kodni tasodifan o'zgartirish emas, balki "xato shu funksiyada,
chunki..." deb taxmin qilib, uni tajriba bilan tasdiqlash yoki rad etish.

**`printf` — non ushoqlari (29.2).** Ertakdagi bolalar o'rmonda adashmaslik uchun yo'lga non ushoqlari
tashlab ketgan. Dasturda "shu yerga keldim, x = 5" degan izlar qoldirasiz va dastur qaysi yo'ldan
o'tganini ko'rasiz. Oddiy, lekin hali ham eng ko'p ishlatiladigan usul.

**gdb — vaqtni to'xtatish (29.3).** Dunyoni "pauza" qilib, har bir odamning cho'ntagini tekshirish
mumkin bo'lsa-chi? `break` — "shu joyga kelganda to'xtat", `print` — cho'ntakni tekshirish, `next` —
bitta qadam oldinga, `bt` — "bu yerga qanday kelding?" (chaqiruvlar zanjiri).

**Watchpoint — xonadagi signalizatsiya (29.3).** "Kim bu seyfga tegsa — darhol xabar ber". `watch x` —
`x` qayerda o'zgarsa, gdb dasturni aynan o'sha qatorda to'xtatadi. "Bu o'zgaruvchini kim buzyapti?"
degan savolga eng tez javob.

**Sanitizer — aeroport rentgeni (29.4).** Har bir yukni tekshiradi va taqiqlangan narsani darhol, aniq
joyi bilan ko'rsatadi. Biroz sekinlashtiradi, lekin xatoni sodir bo'lgan zahoti ushlaydi — ancha keyin
boshqa joyda emas.

**Valgrind — sinchkov inspektor (29.5).** Dasturni qayta kompilyatsiya qilmasdan tekshiradi, lekin juda
sekin (20–50 barobar).

**strace — telefon suhbatlari yozuvi (29.6).** Dastur yadro bilan nima gaplashyapti: qaysi faylni ochdi,
nima o'qidi, qayerda xato oldi. Dastur "fayl topilmadi" deb qulasa, strace **qaysi** faylni izlaganini ko'rsatadi.

**perf — fitnes-soat (29.8).** Kun davomida qadamlar, yurak urishi. perf ham dastur vaqti qaysi
funksiyalarga ketayotganini o'lchaydi. Taxmin qilmang — o'lchang.

**Ikkiga bo'lib qidirish — kitobdagi xatoni topish (29.10).** 1000 sahifali qo'lyozmada qayerdadir
xato bor. 500-sahifani tekshirasiz — xato undan oldinmi yoki keyinmi? 10 qadamda topasiz. `git bisect`
xuddi shunday: 1000 ta commit ichidan xatoni kiritganini 10 qadamda topadi.

### To'liq dastur: iz qoldiruvchi ikkilik qidiruv

`-DDEBUG` bilan yig'ilsa — non ushoqlarini qoldiradi, usiz — jim ishlaydi. `assert` esa kutilmagan
holatni darhol to'xtatadi.

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
$ gdb -q -batch -ex 'break qidir' -ex run -ex 'print n' -ex 'print x' -ex bt ./izlar 2>&1 | grep -E '^\$|^#'
$1 = 10
$2 = 12000
#0  qidir (a=0x7fffffffc8f0, n=10, x=12000) at izlar.c:15
#1  0x000055555555530c in main () at izlar.c:37
```

Oxirgi buyruq: gdb `qidir` funksiyasida to'xtadi, argumentlarni ko'rsatdi va `bt` bilan "bu yerga
`main` dan keldik" degan zanjirni chiqardi.

**Sinab ko'ring:** `ong = orta - 1;` ni `ong = orta;` qiling va 8000 ni qidiring — dastur nega to'xtamaydi?
`-DDEBUG` bilan izlarni o'qib, sababini toping. (Ctrl+C bilan to'xtating.)

## 29.1. Debug usuli — asbobdan oldin fikr

1. **Takrorlang.** Xatoni har safar chiqadigan qiling (kirish, buyruq, qadamlar). Takrorlanmaydigan xato
   tuzatilmaydi — faqat yashiriladi.
2. **Kichraytiring.** Xatoni ko'rsatadigan eng kichik kirish/kodni toping (ikkilik bo'lish usuli: yarmini
   olib tashlang — xato qoldimi?).
3. **Faraz qiling va tekshiring.** "Menimcha x bu yerda NULL" → `print` yoki `assert` bilan tasdiqlang.
   Farazlarsiz tasodifiy o'zgartirishlar — vaqtni yo'qotish.
4. **Birinchi noto'g'ri narsani toping**, oxirgisini emas. Qulash — oqibat; sabab ancha oldinda
   (masalan, bufer to'lishi 1000 qator oldin bo'lgan).
5. **Tuzatgach — test qo'shing.** Xato qaytib kelmasin.
6. **"Rezina o'rdak":** muammoni kimgadir (yoki o'yinchoqqa) ovoz chiqarib tushuntiring — ko'pincha
   tushuntirish paytida o'zingiz topasiz.

## 29.2. `printf` / `kprintf` — hali ham eng ko'p ishlatiladigan vosita

```c
fprintf(stderr, "[debug] %s:%d n=%zu p=%p\n", __FILE__, __LINE__, n, (void *)p);
```

`stderr` — buferlanmaydi (qulashdan oldin ham chiqadi). Yadroda — `kprintf` (serial port + dmesg).
Muhim o'zgaruvchilarni **funksiya kirish/chiqishida** chiqarish ko'pincha gdb'dan tezroq natija beradi.

## 29.3. gdb chuqur

```bash
gcc -g -O0 dastur.c -o dastur
gdb --args ./dastur arg1 arg2
```

| Buyruq | Nima |
|---|---|
| `break fayl.c:42` / `b funksiya` | to'xtash nuqtasi |
| `break 42 if i == 1000` | **shartli** to'xtash — sikl ichidagi 1000-aylanishda |
| `watch x` / `watch -l p->qiymat` | **watchpoint**: o'zgaruvchi O'ZGARGANDA to'xtash — "buni kim buzdi?" savolining javobi |
| `rwatch`, `awatch` | o'qilganda / har qanday murojaatda |
| `run`, `continue` (`c`), `next` (`n`), `step` (`s`), `finish` | boshqarish |
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

**Core dump** — qulagan dasturning xotira "surati":

```bash
ulimit -c unlimited              # core fayllarga ruxsat
./dastur                          # Segmentation fault (core dumped)
gdb ./dastur core                 # qulagan paytdagi holat: bt, print...
```

(Zamonaviy Linux'da core'lar `systemd-coredump` ga tushishi mumkin: `coredumpctl gdb`.)

**Osilib qolgan dasturga ulanish:** `gdb -p PID`, keyin `thread apply all bt` — hamma oqim qayerda kutyapti.

**Yadroni debug qilish** (MyOS): `make debug` — QEMU gdb'ni kutadi; boshqa terminalda `gdb -x tools/gdbinit`.
Xuddi shu buyruqlar yadroda ham ishlaydi: `break kmain`, `break mm_handle_fault` (page fault), `break handle_exception`, `bt`, `info registers`.
`docs/08-test-debug.md` — batafsil.

## 29.4. Sanitizer'lar

| Bayroq | Nima topadi | Sekinlashish |
|---|---|---|
| `-fsanitize=address` (ASan) | chegaradan chiqish, use-after-free, double free, leak | ~2x |
| `-fsanitize=undefined` (UBSan) | toshish, noto'g'ri surish, NULL, tekislanmagan | kam |
| `-fsanitize=thread` (TSan) | poyga holatlari | 5–15x (ASan bilan birga emas) |
| `-fsanitize=memory` (MSan, faqat clang) | boshlanmagan xotirani o'qish | 3x |

Mashqlardagi tekshiruvchi ASan + UBSan ishlatadi. Ko'p oqimli kodni (29, 34, 38, 44, 45-mashqlar)
qo'shimcha ravishda TSan bilan sinang:

```bash
gcc -g -fsanitize=thread -pthread -Imashqlar -Imashqlar/29_oqimlar \
    mashqlar/29_oqimlar/yechim.c mashqlar/29_oqimlar/test.c -o t29 && ./t29
```

Linux yadrosida analoglari: KASAN, UBSAN, KCSAN.

## 29.5. Valgrind

```bash
valgrind --leak-check=full --track-origins=yes ./dastur
```

Qayta kompilyatsiyasiz ishlaydi (dasturni virtual CPU'da bajaradi — 20–50x sekin). Boshlanmagan
qiymatlardan foydalanishni ham topadi (`--track-origins` — qayerdan kelgan). `valgrind --tool=callgrind`
— qaysi funksiya qancha buyruq bajarayotgani.

## 29.6. strace va ltrace — dastur OS bilan nima gaplashyapti

```bash
strace ./dastur                  # hamma syscall'lar: open, read, write, mmap...
strace -e trace=openat,read ls   # faqat tanlanganlari
strace -f -p PID                 # ishlayotgan jarayonga (bolalari bilan)
strace -c ./dastur               # statistika: qaysi syscall necha marta, qancha vaqt
ltrace ./dastur                  # kutubxona funksiyalari chaqiruvlari
```

"Dastur nega bu faylni topmayapti?", "qayerda osilib qoldi?" (oxirgi syscall — `read` yoki `futex` da kutyapti)
kabi savollarga kodni o'qimasdan javob beradi. 14-bobdagi tushunchalarni jonli ko'rish uchun ajoyib:
`strace -f sh -c 'ls | wc -l'` — `pipe`, `clone`, `dup2`, `execve`, `wait4` ni ko'rasiz.

## 29.7. Binar fayllarni ko'rish

```bash
objdump -d -M intel dastur        # disassembly (Intel sintaksisi)
objdump -S dastur                 # manba kod bilan aralash (-g bilan kompilyatsiya qilingan bo'lsa)
readelf -a dastur                 # ELF'ning hammasi (22-bob)
nm -n dastur                      # belgilar manzil bo'yicha tartiblangan
addr2line -e dastur -f 0x401136   # manzil -> funksiya va fayl:qator
xxd fayl | head                   # baytlarni o'n oltilikda
```

**addr2line — yadro PANIC'ini o'qish:** MyOS PANIC xabarida `RIP=0xffffffff8010abcd` bo'lsa:
`addr2line -e build/kernel.elf -f 0xffffffff8010abcd` → funksiya va qator. Stek zanjiridagi har bir
manzil uchun takrorlang.

## 29.8. perf — dastur vaqti qayerga ketyapti

```bash
perf stat ./dastur                          # umumiy: taktlar, buyruqlar, kesh xatolari, sakrash xatolari
perf stat -e cache-misses,cache-references ./dastur
perf record -g ./dastur && perf report      # qaysi funksiyalar eng ko'p vaqt oladi (chaqiruv zanjirlari bilan)
perf top                                    # jonli: butun tizimda hozir nima issiq
```

**Qoida:** optimallashtirishdan oldin o'lchang. Dasturchilarning "qayer sekin" haqidagi taxminlari
ko'pincha noto'g'ri. 21-bobdagi matritsa tajribasini `perf stat -e cache-misses` bilan takrorlang.

## 29.9. QEMU'ning o'z vositalari (yadro uchun)

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
| Qiymat "o'z-o'zidan" o'zgaradi | `watch -l` o'sha o'zgaruvchiga |
| Dastur osilib qoldi | `gdb -p PID` → `thread apply all bt`; yoki `strace -p PID` |
| Ba'zan noto'g'ri natija (ko'p oqimli) | TSan; qulflarni tekshirish |
| Sekin | `perf record` → `perf report` |
| Xotira o'sib boradi | ASan/valgrind leak hisoboti |
| "Oldin ishlardi" | `git bisect` (19-bob) |
| Yadro qayta yuklanib turadi | QEMU `-d int,cpu_reset -no-reboot` |
| Yadro PANIC | `addr2line` bilan RIP va stek manzillari |
| Yadro osilib qoldi | `make debug`, `Ctrl-C` gdb'da, `bt`, `info registers` |

## 29.11. O'zingizni tekshiring

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

## 29.12. Mashq

- 18-mashqni ASan'siz yig'ib, `valgrind` bilan xatolarni toping — hisobotlarni solishtiring.
- `strace -f sh -c 'ls | wc -l'` chiqishida `pipe`, `clone`, `dup2`, `execve` qatorlarini topib, 14-bob bilan bog'lang.
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
