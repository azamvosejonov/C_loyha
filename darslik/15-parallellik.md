# 15-bob. Parallellik: oqimlar, poyga holati, qulflar, atomiklar

> **Bu bobda nima o'rganasiz:** oqim (thread) nima ekanini; **poyga holati** (race condition) nima va nega xavfli ekanini; mutex va spinlock farqini; deadlock'ni; atomik amallarni;
> xotira tartibi (memory ordering) g'oyasini. Bu — yadro dasturlashning **eng qiyin** mavzusi: MyOS 64 tagacha yadroda bir vaqtda ishlaydi va har bir umumiy ma'lumot himoyalangan bo'lishi kerak.
> **Oldindan nima kerak:** 5-, 7-, 8-, 14-boblar.   **Vaqt:** 8–10 soat.
> Mashqlar: 29, 34.

> **To'liq ishlaydigan misol:** [misollar/15_oqimlar.c](misollar/15_oqimlar.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Bu bob nima haqida?

Zamonaviy kompyuterda 4, 8, 64 ta yadro bor — ular **bir vaqtda** ishlay oladi. Dasturni ham bir vaqtda ishlaydigan bir necha "bajaruvchi"ga (**oqim**) bo'lsak, tezroq bo'ladi.
Lekin oqimlar **bitta xotirani bo'lishadi** — shu yerdan eng g'alati xatolar chiqadi: natija har safar boshqacha, debuggerda yo'qoladi.

**Hayotdan misol: bitta ofisdagi xodimlar.** Jarayon — ofis: o'z xonasi, o'z hujjatlari. Oqimlar — shu ofisdagi xodimlar. Ular bitta xonada ishlaydi va **bir xil** shkaf, printer, doskadan foydalanadi
(umumiy xotira). Bu qulay — hujjatni uzatish shart emas. Lekin xavfli — ikki xodim bir hujjatga bir vaqtda yozishi mumkin.

| Ofisda | Dasturda |
|---|---|
| ofis | jarayon |
| xodim | oqim (thread) |
| umumiy shkaf, doska | umumiy xotira (global o'zgaruvchilar, heap) |
| xodimning o'z stoli | oqimning o'z steki (lokal o'zgaruvchilar) |
| hujjat kaliti | mutex |

## 15.1. Oqim (thread) nima

Jarayon — alohida xotira maydoni. **Oqim** — bitta jarayon ichidagi alohida "bajaruvchi": o'z steki va registrlari bor, lekin **xotirani boshqa oqimlar bilan bo'lishadi**.
Ko'p yadroli CPU'da oqimlar **haqiqatan bir vaqtda** ishlaydi.

```c
/* oqim_asos.c - oqimlarni yaratish va kutish */
#include <pthread.h>
#include <stdio.h>

static long natija[4];                      /* har oqim O'Z katagiga yozadi */

static void *ish(void *arg)
{
    int n = *(int *)arg;                    /* argumentni int ga qaytarish */
    natija[n] = (long)n * n;                /* "hisob-kitob" */
    return NULL;
}

int main(void)
{
    pthread_t t[4];
    int id[4];
    for (int i = 0; i < 4; i++) {
        id[i] = i;                          /* har biriga O'Z argumenti */
        pthread_create(&t[i], NULL, ish, &id[i]);
    }
    for (int i = 0; i < 4; i++)
        pthread_join(t[i], NULL);           /* tugashini kutish */

    for (int i = 0; i < 4; i++)
        printf("oqim %d natijasi: %ld\n", i, natija[i]);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -pthread oqim_asos.c -o oqim_asos
$ ./oqim_asos
oqim 0 natijasi: 0
oqim 1 natijasi: 1
oqim 2 natijasi: 4
oqim 3 natijasi: 9
```

**Bu dastur nima qiladi (umumiy):** 4 ta oqim yaratadi; har biri `n * n` ni hisoblab, o'z katagiga yozadi; `main` hammasini kutadi va natijalarni chiqaradi.

**Qismlar:**

| Qism | Vazifasi | Tafsilot |
|---|---|---|
| `pthread_create(&t[i], NULL, ish, &id[i])` | yangi oqim yaratish | uchinchi argument — oqim bajaradigan **funksiya** (manzili); to'rtinchi — unga beriladigan `void *` argument |
| `ish(void *arg)` | oqim funksiyasi | imzo aynan shunday: `void *` oladi, `void *` qaytaradi |
| `pthread_join(t[i], NULL)` | oqim tugashini kutish | `join` siz `main` oqimlardan oldin tugab ketishi mumkin |
| `natija[n]` | har oqim **faqat o'z katagiga** yozadi | shuning uchun bu yerda poyga **yo'q** |
| `-pthread` | kompilyatsiya bayrog'i | oqimlar kutubxonasini ulaydi |

Chiqish tartibi (agar oqimlar ichida `printf` bo'lganda) har safar boshqacha bo'lishi mumkin — oqimlar qachon ishlashini scheduler hal qiladi.

**Tuzoq:** hamma oqimga bitta `&i` berish — `i` sikl davomida o'zgaradi, oqimlar noto'g'ri qiymatni ko'radi. Shuning uchun har oqimga alohida `id[i]` berdik.

## 15.2. Poyga holati (race condition)

**Hayotdan misol: bitta hisobdan ikki kishi pul yechishi.** Hisobda 100 ming bor. Er va xotin ikki bankomatdan **bir vaqtda** 80 mingdan yechmoqchi. Ikkala bankomat balansni o'qiydi: "100 ming — yetarli".
Ikkalasi ham pul beradi va yozadi: "20 ming qoldi". Natija: bank 160 ming berdi, hisobda 20 ming. `balans -= summa` bitta amal ko'rinadi, lekin aslida uch qadam: **o'qi, ayir, yoz**.
Oqimlar shu qadamlar orasida almashib qoladi.

Xuddi shu xato, oddiy hisoblagichda:

```c
/* poyga.c - qulfsiz umumiy hisoblagich */
#include <pthread.h>
#include <stdio.h>

static volatile long hisob = 0;             /* umumiy o'zgaruvchi (volatile: aniq o'qish/yozish ko'rsatilsin) */

static void *oshir(void *arg)
{
    (void)arg;
    for (int i = 0; i < 1000000; i++)
        hisob++;                            /* o'qi - oshir - yoz: 3 qadam! */
    return NULL;
}

int main(void)
{
    pthread_t t[4];
    for (int i = 0; i < 4; i++)
        pthread_create(&t[i], NULL, oshir, NULL);
    for (int i = 0; i < 4; i++)
        pthread_join(t[i], NULL);
    printf("kutilgan: 4000000, olingan: %ld\n", hisob);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -pthread poyga.c -o poyga
$ ./poyga
kutilgan: 4000000, olingan: 1325526
```

**Bu dastur nima qiladi (umumiy):** 4 oqim bir xil `hisob` o'zgaruvchisini 1 000 000 martadan oshiradi. To'g'ri javob — 4 000 000. Olingan natija esa **kamroq** va **har safar boshqacha**
(sizda boshqa raqam chiqadi) — chunki ba'zi oshirishlar "yo'qoladi".

`hisob++` — **uchta** amal: xotiradan registrga o'qish, oshirish, xotiraga yozish.

```text
oqim A: o'qidi 5
oqim B: o'qidi 5
oqim A: yozdi 6
oqim B: yozdi 6        <- bitta oshirish YO'QOLDI
```

Poyga holatlari — eng yomon xatolar: ular **kamdan-kam** va **tasodifiy** chiqadi, debugger bilan kuzatganda yo'qolib qoladi (vaqt o'zgaradi). Yadroda oqibati — buzilgan ro'yxat,
ikki marta berilgan sahifa, qulash — bir necha kunda bir marta.

Poygani **topish** uchun **ThreadSanitizer** (`-fsanitize=thread`) bor:

```console
$ gcc -g -pthread -fsanitize=thread poyga.c -o poyga_tsan
$ ./poyga_tsan 2>&1 | grep -E 'WARNING|Location' | head -3 | sed -E 's/pid=[0-9]+/pid=.../; s/0x[0-9a-f]+/0x.../g'
WARNING: ThreadSanitizer: data race (pid=...)
  Location is global 'hisob' of size 8 at 0x... (poyga_tsan+0x...)
WARNING: ThreadSanitizer: data race (pid=...)
```

ThreadSanitizer aniq ko'rsatdi: "**data race**" va **qaysi o'zgaruvchi** (`Location is global 'hisob'`). Oddiy ishga tushirishda xato ko'rinmay qolishi mumkin; sanitizer esa har safar ushlaydi.

**Kritik seksiya** — umumiy ma'lumotga murojaat qiladigan kod qismi. Bir vaqtda faqat **bitta** oqim unda bo'lishi kerak (o'zaro istisno — *mutual exclusion*).

> **Eslab qoling:** umumiy ma'lumotni **bir vaqtda o'zgartirish** — poyga. `x++` bitta amal emas, uchta. Natija tasodifiy va noto'g'ri.

## 15.3. Mutex

**Hayotdan misol: bittalik hojatxonaning kaliti.** Kalit bitta. Kim olsa — ichkariga kiradi, qolganlar navbatda kutadi. Chiqqanda kalitni qaytaradi va keyingisi kiradi. `pthread_mutex_lock` — kalitni olish
(yoki kutish), `unlock` — qaytarish. Kalit bilan himoyalangan kod — **kritik bo'lim**.

```c
/* mutex_misol.c - poygani mutex bilan tuzatish */
#include <pthread.h>
#include <stdio.h>

static long hisob = 0;
static pthread_mutex_t qulf = PTHREAD_MUTEX_INITIALIZER;

static void *oshir(void *arg)
{
    (void)arg;
    for (int i = 0; i < 1000000; i++) {
        pthread_mutex_lock(&qulf);      /* bo'sh bo'lguncha kutadi (uxlaydi) */
        hisob++;                        /* kritik seksiya */
        pthread_mutex_unlock(&qulf);
    }
    return NULL;
}

int main(void)
{
    pthread_t t[4];
    for (int i = 0; i < 4; i++)
        pthread_create(&t[i], NULL, oshir, NULL);
    for (int i = 0; i < 4; i++)
        pthread_join(t[i], NULL);
    printf("kutilgan: 4000000, olingan: %ld\n", hisob);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -pthread mutex_misol.c -o mutex_misol
$ ./mutex_misol
kutilgan: 4000000, olingan: 4000000
```

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `pthread_mutex_t qulf = PTHREAD_MUTEX_INITIALIZER` | qulfni yaratish va boshlash |
| `pthread_mutex_lock(&qulf)` | qulfni olish; band bo'lsa — **kutadi** |
| `hisob++` | kritik seksiya: bir vaqtda faqat bitta oqim shu yerda |
| `pthread_mutex_unlock(&qulf)` | qulfni qaytarish (keyingi oqim kiradi) |

Natija: **doim 4 000 000**. Qulf oshirishlarni navbatma-navbat qilishga majbur qildi.

Qoidalar:
1. Har bir umumiy ma'lumotning **aniq bitta** himoyachi qulfi bo'lsin — va buni izohda yozing (`/* disks_lock himoya qiladi */`). MyOS'da har bir global ro'yxat yonida shunday izoh bor.
2. Kritik seksiya **qisqa** bo'lsin — qulf ichida uxlamang, katta ish qilmang.
3. Har bir `lock` ga aniq bitta `unlock` — xato yo'llarida ham (`goto` tozalash, 4-bob).

## 15.4. Deadlock (o'zaro qotish)

**Hayotdan misol: tor ko'prikda ikki mashina.** Bir qatorli ko'prikka ikki tomondan mashina kirdi va o'rtada yuzma-yuz to'xtadi. Har biri ikkinchisining orqaga qaytishini kutadi — abadiy.
Ikki oqim ikki qulfni **teskari tartibda** olsa, aynan shunday bo'ladi.

```text
oqim A: lock(X) ...  lock(Y) <- kutadi (Y B da)
oqim B: lock(Y) ...  lock(X) <- kutadi (X A da)       -> ikkalasi ham abadiy kutadi
```

```c
/* deadlock.c - qulflarni teskari tartibda olish */
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

static pthread_mutex_t X = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t Y = PTHREAD_MUTEX_INITIALIZER;

static void *oqim_a(void *arg)
{
    (void)arg;
    pthread_mutex_lock(&X);
    usleep(100000);                         /* ikkinchi oqimga X ni... Y ni olishga vaqt beramiz */
    pthread_mutex_lock(&Y);                 /* B da Y bor - kutamiz */
    pthread_mutex_unlock(&Y);
    pthread_mutex_unlock(&X);
    return NULL;
}

static void *oqim_b(void *arg)
{
    (void)arg;
    pthread_mutex_lock(&Y);
    usleep(100000);
    pthread_mutex_lock(&X);                 /* A da X bor - kutamiz */
    pthread_mutex_unlock(&X);
    pthread_mutex_unlock(&Y);
    return NULL;
}

int main(void)
{
    pthread_t a, b;
    pthread_create(&a, NULL, oqim_a, NULL);
    pthread_create(&b, NULL, oqim_b, NULL);
    pthread_join(a, NULL);
    pthread_join(b, NULL);
    printf("tugadi\n");
    return 0;
}
```

```console
$ gcc -Wall -Wextra -pthread deadlock.c -o deadlock
$ bash -c 'timeout 2 ./deadlock; echo "chiqish kodi: $?"'
chiqish kodi: 124
```

**Nima ko'rdik:** dastur "tugadi" demadi — `timeout` uni 2 soniyadan keyin majburan to'xtatdi (chiqish kodi 124). A `X` ni, B `Y` ni oldi, keyin har biri ikkinchisining qulfini kutdi — abadiy.

**Oldini olish — qulflar tartibi:** hamma kod qulflarni **bir xil tartibda** oladi (avval X, keyin Y). Shunda "X ni ushlab Y ni kutayotgan" va "Y ni ushlab X ni kutayotgan" holat bo'lishi mumkin emas.
Linux'da `lockdep` bu tartibni avtomatik tekshiradi. Boshqa deadlock turi: o'zingiz ushlab turgan qulfni qayta olish (rekursiv), yoki qulf ushlagan holda uxlab qolish.

> **Eslab qoling:** bir nechta qulf kerak bo'lsa — **doim bir xil tartibda** oling.

## 15.5. Atomik amallar

**Hayotdan misol: turniket.** Turniket bir vaqtda bitta odamni o'tkazadi va hisoblagichni bittaga oshiradi — bu amalni ikkiga bo'lib bo'lmaydi. `atomic_fetch_add` ham: "o'qi-qo'sh-yoz" bitta,
**bo'linmas** qadam. Oddiy hisoblagich uchun qulfdan ko'ra ancha yengil.

```c
/* atomik_misol.c - atomik hisoblagich va compare-and-swap */
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>

static atomic_long hisob = 0;

static void *oshir(void *arg)
{
    (void)arg;
    for (int i = 0; i < 1000000; i++)
        atomic_fetch_add(&hisob, 1);        /* bitta bo'linmas amal (x86: lock xadd) */
    return NULL;
}

int main(void)
{
    pthread_t t[4];
    for (int i = 0; i < 4; i++)
        pthread_create(&t[i], NULL, oshir, NULL);
    for (int i = 0; i < 4; i++)
        pthread_join(t[i], NULL);
    printf("atomik hisoblagich: %ld\n", atomic_load(&hisob));

    /* Compare-and-swap: "agar x hali ham kutilgan bo'lsa - yangi qil" */
    int x = 10;
    int kutilgan = 10;
    int ok1 = __atomic_compare_exchange_n(&x, &kutilgan, 99, 0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
    printf("1-CAS: %s, x = %d\n", ok1 ? "muvaffaqiyatli" : "rad", x);

    kutilgan = 10;                           /* lekin x endi 99! */
    int ok2 = __atomic_compare_exchange_n(&x, &kutilgan, 55, 0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
    printf("2-CAS: %s, x = %d, haqiqiy qiymat kutilgan ga yozildi: %d\n", ok2 ? "muvaffaqiyatli" : "rad", x, kutilgan);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -pthread atomik_misol.c -o atomik_misol
$ ./atomik_misol
atomik hisoblagich: 4000000
1-CAS: muvaffaqiyatli, x = 99
2-CAS: rad, x = 99, haqiqiy qiymat kutilgan ga yozildi: 99
```

**Qismlar:**

- `atomic_long` / `atomic_fetch_add(&h, 1)` — "bo'linmas qo'shish": o'qish-oshirish-yozish orasida boshqa oqim aralasha olmaydi → doim 4 000 000.
- **Compare-and-swap (CAS)** — barcha qulflarning asosi: "agar `x` hali ham `kutilgan` ga teng bo'lsa, uni `yangi` qil — atomik ravishda; bo'lmasa, menga hozirgi qiymatni ayt". x86'da `lock cmpxchg`.
  1-CAS: `x` (10) == `kutilgan` (10) → `x = 99`. 2-CAS: `x` (99) != `kutilgan` (10) → **rad**, `kutilgan` ga haqiqiy qiymat (99) yozildi.
- GCC builtin'lari (MyOS shularni ishlatadi): `__atomic_add_fetch(&n, 1, __ATOMIC_RELAXED)`, `__atomic_load_n(&ticks, __ATOMIC_ACQUIRE)`.

> **Eslab qoling:** bitta o'zgaruvchi (hisoblagich, bayroq) → **atomik**. Bir nechta bog'liq o'zgaruvchi → **qulf**.

## 15.6. Spinlock — yadroning asosiy qulfi

**Hayotdan misol: kutishning ikki usuli.** Svetoforda 10 soniya kutish kerak bo'lsa, motorni o'chirmaysiz (**spinlock** — aylanib kutish, CPU band). Temir yo'l o'tish joyida 10 daqiqa kutish kerak
bo'lsa, motorni o'chirasiz (**mutex** — uxlab kutish, CPU boshqalarga beriladi). Yadroda qisqa kutish uchun spinlock, uzoq kutish uchun uxlash.

Mutex kutganda **uxlaydi** (scheduler boshqa ishni beradi). Yadroda esa ba'zi joylarda uxlab bo'lmaydi (uzilish ishlovchisi ichida, scheduler'ning o'zida) — u yerda **spinlock**: bo'shaguncha aylanib turadi.

```c
/* spinlock_misol.c - atomiklardan o'zimizning spinlock */
#include <pthread.h>
#include <stdio.h>

typedef struct { volatile int band; } spinlock_t;

static void spin_lock(spinlock_t *l)
{
    while (__atomic_exchange_n(&l->band, 1, __ATOMIC_ACQUIRE))  /* 1 yozib, eskisini olish */
        while (__atomic_load_n(&l->band, __ATOMIC_RELAXED))     /* band - faqat o'qib kutish */
            __builtin_ia32_pause();                             /* CPU'ga "aylanyapman" ishorasi */
}

static void spin_unlock(spinlock_t *l)
{
    __atomic_store_n(&l->band, 0, __ATOMIC_RELEASE);
}

static spinlock_t qulf;
static long hisob = 0;

static void *oshir(void *arg)
{
    (void)arg;
    for (int i = 0; i < 500000; i++) {
        spin_lock(&qulf);
        hisob++;
        spin_unlock(&qulf);
    }
    return NULL;
}

int main(void)
{
    pthread_t t[4];
    for (int i = 0; i < 4; i++)
        pthread_create(&t[i], NULL, oshir, NULL);
    for (int i = 0; i < 4; i++)
        pthread_join(t[i], NULL);
    printf("spinlock bilan: %ld (kutilgan 2000000)\n", hisob);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 -pthread spinlock_misol.c -o spinlock_misol
$ ./spinlock_misol
spinlock bilan: 2000000 (kutilgan 2000000)
```

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `band` | 0 = qulf bo'sh, 1 = band |
| `__atomic_exchange_n(&l->band, 1, ...)` | atomik: `band` ga **1 yozib, eski qiymatni olish**. Eski qiymat 0 bo'lsa — qulfni **biz oldik**; 1 bo'lsa — kimdir ushlab turibdi |
| ichki `while (load)` | band bo'lsa, faqat **o'qib** kutish (yozmaymiz — boshqa CPU'larning keshini bezovta qilmaslik uchun) |
| `__builtin_ia32_pause()` | CPU'ga "aylanib kutyapman" ishorasi (energiya va tezlik) |
| `spin_unlock` | `band = 0` (RELEASE bilan) |

| | Mutex | Spinlock |
|---|---|---|
| Kutganda | uxlaydi | aylanadi (CPU yonadi) |
| Qachon | uzoq kutish, uxlash mumkin bo'lgan joy | juda qisqa kritik seksiya, uxlab bo'lmaydigan joy |
| Yadroda | `kernel/lib/mutex.c` | `kernel/lib/spinlock.c` (spinlock lab'i) |

Yadroda spinlock ushlanganda **uzilishlar ham o'chiriladi** — aks holda uzilish ishlovchisi o'sha qulfni olishga harakat qilib, o'z CPU'sida abadiy aylanadi (deadlock). MyOS'ning `spin_lock` i shuni qiladi.

## 15.7. Xotira tartibi (memory ordering) — g'oya

Zamonaviy CPU va kompilyator xotira amallarini **tartibini o'zgartirishi** mumkin (tezlik uchun):

```text
/* oqim A */                     /* oqim B */
data = 42;                       while (!tayyor) ;
tayyor = 1;                      printf("%d", data);    /* 0 chiqishi mumkin! */
```

Kompilyator yoki CPU `tayyor = 1` ni `data = 42` dan **oldin** bajarishi mumkin — bitta oqim nuqtai nazaridan farq yo'q. Yechim — **to'siqlar** (barriers) yoki **acquire/release** semantikasi:

- `__ATOMIC_RELEASE` bilan yozish — "bundan **oldingi** hamma yozuvlar bundan **keyin** ko'rinmaydi" (ya'ni avval `data`, keyin `tayyor`);
- `__ATOMIC_ACQUIRE` bilan o'qish — "bundan **keyingi** o'qishlar bundan **oldin** bajarilmaydi".

```c
/* tartib_misol.c - release/acquire: ma'lumot va "tayyor" bayrog'i */
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>

static int data = 0;
static atomic_int tayyor = 0;

static void *yozuvchi(void *arg)
{
    (void)arg;
    data = 42;                                                  /* 1) ma'lumotni tayyorla */
    atomic_store_explicit(&tayyor, 1, memory_order_release);    /* 2) "tayyor" - RELEASE */
    return NULL;
}

static void *oquvchi(void *arg)
{
    (void)arg;
    while (!atomic_load_explicit(&tayyor, memory_order_acquire))  /* ACQUIRE: kutish */
        ;
    printf("o'quvchi ko'rdi: data = %d\n", data);               /* kafolat: 42 */
    return NULL;
}

int main(void)
{
    pthread_t a, b;
    pthread_create(&b, NULL, oquvchi, NULL);
    pthread_create(&a, NULL, yozuvchi, NULL);
    pthread_join(a, NULL);
    pthread_join(b, NULL);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 -pthread tartib_misol.c -o tartib_misol
$ ./tartib_misol
o'quvchi ko'rdi: data = 42
```

**Qismlar:** yozuvchi avval `data = 42`, keyin `tayyor = 1` (RELEASE). O'quvchi `tayyor` 1 bo'lguncha kutadi (ACQUIRE), keyin `data` ni o'qiydi. RELEASE/ACQUIRE juftligi **kafolatlaydi**: o'quvchi
`tayyor = 1` ni ko'rgan bo'lsa, `data = 42` ni ham ko'radi. Spinlock'dagi `ACQUIRE`/`RELEASE` aynan shu: qulf ichidagi amallar qulfdan "tashqariga sizib" chiqmaydi.

Qulflarni to'g'ri ishlatsangiz, ular tartibni o'zi ta'minlaydi. Qulfsiz (lock-free) kod yozish — mutaxassis darajasi; avval qulflarni mukammal o'rganing.

`volatile` **qulf emas** va tartibni kafolatlamaydi — u faqat "kompilyator bu o'qishni o'chirmasin" degani (16-bob). Ko'p oqimli sinxronlash uchun atomiklar kerak.

## 15.8. Yadroda parallellik manbalari

Bitta yadroli kompyuterda ham yadroda "parallellik" bor:

1. **Uzilishlar** — istalgan ikki buyruq orasida uzilish ishlovchisi ishlashi mumkin.
2. **Preemption** — taymer jarayonni to'xtatib, boshqasini ishga tushiradi.
3. **Ko'p yadro (SMP)** — haqiqiy parallellik (MyOS: `kernel/arch/smp.c`, `docs/10-smp.md`).

## Hayotdan misol va to'liq dastur

**Oilaviy bank hisobi.** To'rt a'zo bir vaqtda umumiy hisobga pul qo'yadi va yechadi. Mutex tufayli natija doim to'g'ri; atomik hisoblagich esa amallar sonini sanaydi.

```c
/* hisob.c - poyga holati va uning mutex hamda atomik bilan yechimi */
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>

static long balans = 1000000;
static pthread_mutex_t kalit = PTHREAD_MUTEX_INITIALIZER;
static atomic_long amallar_soni = 0;           /* oddiy hisoblagich - atomik yetarli */

static void *oila_azosi(void *arg)
{
    long summa = (long)arg;
    for (int i = 0; i < 100000; i++) {
        pthread_mutex_lock(&kalit);             /* kalitni olish */
        balans += summa;                        /* o'qi - qo'sh - yoz: endi bo'linmas */
        balans -= summa;
        pthread_mutex_unlock(&kalit);           /* kalitni qaytarish */
        atomic_fetch_add(&amallar_soni, 2);
    }
    return NULL;
}

int main(void)
{
    pthread_t azolar[4];
    long summalar[4] = { 5000, 12000, 700, 30000 };

    for (int i = 0; i < 4; i++)
        pthread_create(&azolar[i], NULL, oila_azosi, (void *)summalar[i]);
    for (int i = 0; i < 4; i++)
        pthread_join(azolar[i], NULL);          /* hammasi tugashini kutish */

    printf("Amallar soni: %ld\n", atomic_load(&amallar_soni));
    printf("Balans: %ld (boshida 1000000 edi - har bir qo'yilgan pul yechildi)\n", balans);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 -pthread hisob.c -o hisob
$ ./hisob
Amallar soni: 800000
Balans: 1000000 (boshida 1000000 edi - har bir qo'yilgan pul yechildi)
$ gcc -Wall -Wextra -O2 -pthread -fsanitize=thread hisob.c -o hisob_tsan
$ ./hisob_tsan
Amallar soni: 800000
Balans: 1000000 (boshida 1000000 edi - har bir qo'yilgan pul yechildi)
```

**Bu dastur nima qiladi (umumiy):** 4 oila a'zosi (4 oqim) umumiy `balans` ga 100 000 martadan: avval summani qo'shadi, keyin ayiradi. Oxirida balans **aynan boshidagidek** bo'lishi kerak (1 000 000), amallar soni — 800 000.

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `balans` | **umumiy** ma'lumot — poyga xavfi shu yerda |
| `kalit` (mutex) | `balans` ni himoya qiladi; `lock` … `unlock` orasi — kritik seksiya |
| `amallar_soni` (`atomic_long`) | oddiy hisoblagich — qulf shart emas, atomik yetarli |
| `(void *)summalar[i]` | oqimga son uzatish (oddiylik uchun `long` ni `void *` ga sig'dirdik) |

`-fsanitize=thread` (ThreadSanitizer) poyga holatlarini qidiradi. Hech qanday "WARNING: ThreadSanitizer: data race" chiqmadi — poyga yo'q.

**Sinab ko'ring:** `pthread_mutex_lock` va `unlock` qatorlarini o'chiring (izohga oling). Dasturni bir necha marta ishga tushiring — balans har safar boshqacha chiqadi. `-fsanitize=thread` bilan yig'ing — ThreadSanitizer poygani qaysi qatorda ko'rsatadi?

## Bob xulosasi (yodlash uchun)

1. **Oqim** — jarayon ichidagi bajaruvchi; **xotirani bo'lishadi** (shuning uchun tez va xavfli). `pthread_create` / `pthread_join`; `-pthread`.
2. **Poyga holati** — umumiy ma'lumotni bir vaqtda o'zgartirish; `x++` = o'qi + oshir + yoz (3 qadam); natija tasodifiy. `-fsanitize=thread` ushlaydi.
3. **Mutex** — kritik seksiyani himoyalaydi (kutganda uxlaydi); **spinlock** — qisqa joylar uchun (aylanib kutadi). Har ma'lumotning aniq bitta qulfi.
4. **Deadlock** — teskari tartibdagi qulflar; yechim: qulflarni **bir xil tartibda** olish.
5. Bitta o'zgaruvchi → **atomik** (`atomic_fetch_add`, CAS); xotira tartibi → acquire/release; `volatile` qulf **emas**.

## Savol-javob

**Nega Python'da bu muammolar kamroq?**
GIL (Global Interpreter Lock) — bir vaqtda faqat bitta oqim Python kodini bajaradi. Bu poygalarning bir qismini yashiradi (lekin hammasini emas!), buning evaziga CPU ishida parallellik yo'q.

**Poyga holatini qanday topaman?**
`gcc -fsanitize=thread` (ThreadSanitizer) — ish vaqtida umumiy xotiraga himoyasiz murojaatni ushlaydi. Yadroda — KCSAN, lockdep, va eng muhimi — ehtiyotkor kod o'qish.

**Qachon atomik, qachon qulf?**
Bitta o'zgaruvchi (hisoblagich, bayroq) — atomik. Bir nechta bog'liq o'zgaruvchi (ro'yxat + uning uzunligi) — qulf.

## O'zingizni tekshiring

1. Nega `hisob++` ko'p oqimda xato beradi?
2. Mutex va spinlock farqi? Yadroda qaysi birini uzilish ishlovchisida ishlatish mumkin?
3. Deadlock'ni qanday oldini olish mumkin?
4. `volatile` ko'p oqimli hisoblagichni himoya qiladimi?
5. `pthread_create` ga sikl o'zgaruvchisining manzilini berish nega xato?

<details><summary>Javoblar</summary>

1. U uch amaldan iborat (o'qish, oshirish, yozish) va oqimlar ular orasida aralashadi.
2. Mutex uxlaydi, spinlock aylanadi. Uzilish ishlovchisida faqat spinlock (uxlab bo'lmaydi).
3. Qulflarni doim bir xil tartibda olish; qulf ushlab uxlamaslik; qayta kirishdan qochish.
4. Yo'q — atomiklik va tartibni ta'minlamaydi.
5. O'zgaruvchi keyingi aylanishda o'zgaradi; oqimlar bir xil (yoki noto'g'ri) qiymatni ko'radi.
</details>

## Mashq

- **29** — oqimlar va mutex (poyga holatini o'z ko'zingiz bilan ko'rish).
- **34** — o'z spinlock'ingiz (atomiklar bilan).
- MyOS: `kernel/lib/spinlock.c` ni o'qing, keyin spinlock lab'ini bajaring (`tools/lab.py boshla spinlock`).

<!-- loyiha:boshi -->
## Loyiha: parallel yig'indi

**Maqsad:** ishni oqimlarga bo'lish va **umumiy holat** (umumiy o'zgaruvchi) qanchalik qimmatligini o'z ko'zingiz bilan ko'rish.
**Bobdan ishlatiladi:** `pthread_create`/`join`, mutex, "har oqim o'z qismini hisoblasin" naqshi.

**Talab:** 1 dan 4 000 000 gacha har son uchun `qiymat(i)` ni hisoblab, yig'indisini 4 oqimda toping. Ikki usul:
1. **Qulf bilan:** har qo'shishda umumiy `umumiy += i` (mutex ichida). To'g'ri, lekin har qo'shishda 4 oqim navbat kutadi.
2. **Qismiy:** har oqim o'z qismini **lokal** o'zgaruvchida yig'adi va oxirida bitta marta natijasini yozadi. Qulf kerak emas —
   har oqim faqat o'z `qismiy[id]` katagiga yozadi.

**Asosiy saboq:** parallellikda eng tez yechim — **bo'lishmaslik**. Umumiy narsani kamroq ishlating.

```c
/* parallel.c - parallel yig'indi: qulf bilan va qismiy */
#include <pthread.h>
#include <stdio.h>
#include <time.h>

#define OQIMLAR 4
#define N 4000000L

static long umumiy;
static pthread_mutex_t kalit = PTHREAD_MUTEX_INITIALIZER;
static long qismiy[OQIMLAR];

/* oddiy bo'lmagan qiymat: kompilyator siklni formulaga aylantira olmasin */
static long qiymat(long i)
{
    return (i ^ (i >> 3)) % 1000;
}

static double hozir(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (double)t.tv_sec + (double)t.tv_nsec / 1e9;
}

static void *qulf_bilan(void *arg)              /* 1-usul */
{
    long id = (long)arg;
    for (long i = id * (N / OQIMLAR) + 1; i <= (id + 1) * (N / OQIMLAR); i++) {
        pthread_mutex_lock(&kalit);
        umumiy += qiymat(i);
        pthread_mutex_unlock(&kalit);
    }
    return NULL;
}

static void *qismiy_bilan(void *arg)            /* 2-usul */
{
    long id = (long)arg;
    long s = 0;                                 /* lokal - boshqa oqim ko'rmaydi */
    for (long i = id * (N / OQIMLAR) + 1; i <= (id + 1) * (N / OQIMLAR); i++)
        s += qiymat(i);
    qismiy[id] = s;                             /* bitta marta yozish */
    return NULL;
}

static void ishga_tush(void *(*ish)(void *))
{
    pthread_t t[OQIMLAR];
    for (long i = 0; i < OQIMLAR; i++)
        pthread_create(&t[i], NULL, ish, (void *)i);
    for (int i = 0; i < OQIMLAR; i++)
        pthread_join(t[i], NULL);
}

int main(void)
{
    double t0 = hozir();
    long kutilgan = 0;
    for (long i = 1; i <= N; i++)               /* taqqoslash uchun: bitta oqimda */
        kutilgan += qiymat(i);
    double t1 = hozir();
    ishga_tush(qulf_bilan);
    double t2 = hozir();
    ishga_tush(qismiy_bilan);
    double t3 = hozir();

    long jami = 0;
    for (int i = 0; i < OQIMLAR; i++)
        jami += qismiy[i];

    printf("bitta oqim  : %ld, %.3f s\n", kutilgan, t1 - t0);
    printf("qulf bilan  : %ld, %.3f s\n", umumiy, t2 - t1);
    printf("qismiy      : %ld, %.3f s\n", jami, t3 - t2);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 -pthread parallel.c -o parallel
$ ./parallel
bitta oqim  : 1997978112, 0.004 s
qulf bilan  : 1997978112, 0.255 s
qismiy      : 1997978112, 0.002 s
$ gcc -Wall -Wextra -O2 -pthread -fsanitize=thread parallel.c -o parallel_tsan && ./parallel_tsan | head -1
bitta oqim  : 1997978112, 0.004 s
```

Uchala yig'indi bir xil. Qulfli usul **bitta oqimdan ham sekin** (har qo'shishda qulf uchun kurash), qismiy usul esa
yadrolar soniga yaqin barobar tez (vaqtlar kompyuterga qarab farq qiladi).
ThreadSanitizer hech narsa demadi — `qismiy[id]` katagiga har oqim faqat o'zi yozadi, poyga yo'q.

**Kengaytiring:** `OQIMLAR` ni 1, 2, 8 qiling. `qismiy` ni `static long qismiy[OQIMLAR]` dan har oqimda alohida
`long` ga o'zgartirib bo'lmaydi-mi? (Yadro ham shu g'oya bilan "per-CPU" o'zgaruvchilar ishlatadi, 15.8-bo'lim.)

## Mustaqil loyiha: deadlock'siz bank ★★★

**Vazifa:** 4 ta hisob, 4 ta oqim. Har oqim bir-biriga qarama-qarshi yo'nalishda pul o'tkazadi. Naive yozilsa dastur
**qotib qoladi (deadlock)**. Sizning vazifangiz — uni to'g'ri yozish. Fayl: `bank.c`.

**Ma'lumotlar:** `balans[4]`, boshlang'ich har biri 1000. Har hisobda **o'z mutex**i.

**`otkazma(dan, ga, summa)`:** `balans[dan] -= summa; balans[ga] += summa;` — ikkala hisobni **bir vaqtda**
qulflab. Balans yetmasa ham o'tkaziladi (manfiy bo'lishi mumkin) — mantiq oddiy bo'lsin.

**Oqim `t` (t = 0..3)** `K = 50000` marta takrorlaydi (i = 0, 1, 2, ...):
- `i` juft bo'lsa: `otkazma(t, (t+1)%4, t+1)`;
- `i` toq bo'lsa: `otkazma((t+1)%4, t, 1)`.

**Qulflash tartibi:** deadlock'ni oldini olish uchun ikkita qulfni **doim bir xil (raqami kichik birinchi) tartibda**
oling (26.9: aylanma kutishni buzish). Aks holda oqim `t` avval `t` ni, oqim `t+1` avval `t+1` ni ushlab, hammasi bir-birini kutadi.

**Chiqarish:** har hisob balansi va jami:

**Kutilgan natija** (`darslik/loyihalar/15_bank_oqimlar/kutilgan.txt`):

```text
Hisob 0: 76000
Hisob 1: -24000
Hisob 2: -24000
Hisob 3: -24000
Jami: 4000 (o'zgarmadi)
```

**Maslahat** (yechim emas):
- Avval qog'ozda `t = 0` uchun ikki iteratsiyani yozing: qaysi hisob nimaga o'zgaradi?
- `int a = dan < ga ? dan : ga;` va `int b = dan < ga ? ga : dan;` — birinchi `a` ni, keyin `b` ni qulflang. Qulfni qaytarish tartibi muhim emas, lekin odatda teskari.
- "Jami 4000 (o'zgarmadi)" — oqimlar to'g'ri ishlaganining belgisi (poyga bo'lsa yig'indi buziladi).
- Avval **ataylab noto'g'ri** yozib ko'ring (`dan` ni birinchi, `ga` ni ikkinchi qulflab) va dastur qotib qolishini ko'ring
  (`timeout 10` bilan ishga tushiring). Keyin tuzating.
- `-fsanitize=thread` bilan yig'ing: poyga bo'lmasligi kerak.

**Tekshirish:**

```bash
gcc -Wall -Wextra -O2 -g -pthread bank.c -o dastur && timeout 20 ./dastur | diff - ~/C_loyha/darslik/loyihalar/15_bank_oqimlar/kutilgan.txt && echo "TO'G'RI"
```
<!-- loyiha:oxiri -->

Keyingi bob: [16-bob. Bitlar va apparat](16-bitlar-apparat.md)
