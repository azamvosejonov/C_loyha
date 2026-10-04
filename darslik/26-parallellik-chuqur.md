# 26-bob. Parallellik chuqur: qulflar qanday quriladi, semaforlar, klassik masalalar, deadlock

> **Bu bobda nima o'rganasiz:** qulf **ichida** nima borligini (oddiy bayroq nega ishlamaydi, test-and-set, ticket, CAS, Peterson); oqim uxlashi va uyg'onishini
> (shart o'zgaruvchisi, semafor); uchta klassik masalani (ishlab chiqaruvchi–iste'molchi, o'quvchilar–yozuvchilar, faylasuflar); **deadlock** nima ekanini va uni qanday oldini olishni.
> (OSTEP "Concurrency" qismi.) 15-bobning davomi.
> **Oldindan nima kerak:** 15-bob (oqim, mutex, poyga), 16-bob (xotira to'siqlari).   **Vaqt:** 8–10 soat.
> Mashqlar: 29, 34, 38, 44, 45, 48.

> **To'liq ishlaydigan misol:** [misollar/26_faylasuflar.c](misollar/26_faylasuflar.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Bu bob nima haqida?

15-bobda `pthread_mutex_lock` ni **ishlatdingiz**: "qulflaysiz — hech kim kira olmaydi, ochasiz — boshqa kiradi". Lekin qulf o'zi **qanday ishlaydi**? Axir qulf ham oddiy o'zgaruvchi — uni ikki oqim bir vaqtda o'qisa
nima bo'ladi? Yadro yozuvchisi bu savolga javob bilishi shart: yadroda `pthread_mutex` yo'q — qulflarni **o'zingiz** yozasiz.

Bob uch qismga bo'lingan:

1. **Qulf ichida** (26.1–26.6): oddiy bayroq nega yetmaydi, protsessorning bo'linmas buyruqlari, spinlock, ticket, CAS.
2. **Kutish vositalari** (26.7–26.9): oqim "kerakli narsa tayyor bo'lguncha" uxlashi uchun — shart o'zgaruvchisi va semafor. Ular bilan klassik masalalar.
3. **Deadlock** (26.10–26.12): oqimlar bir-birini abadiy kutib qolishi va bundan qanday qochish.

**Hayotdan misol: hojatxona eshigi.** Eshikda "band/bo'sh" yozuvi bor. Bir vaqtda faqat bitta odam ichkarida bo'lishi kerak. Muammo: ikki kishi bir vaqtda yozuvga qarasa, ikkalasi ham "bo'sh" ko'radi — va ikkalasi
kiradi. Bu bobning butun mazmuni — shu muammoni **bo'linmas** harakat bilan yechish.

| Hayotda | Kompyuterda |
|---|---|
| hojatxona eshigi | qulf (mutex) |
| ichkaridagi odam | kritik seksiyadagi oqim |
| "band/bo'sh" yozuvi | qulf o'zgaruvchisi (0 yoki 1) |
| yozuvga qarash va uni o'zgartirish orasidagi vaqt | poyga uchun "teshik" |
| burama qulf (qarash va qulflash bir harakat) | atomik buyruq (`xchg`, `lock cmpxchg`) |
| navbat chiptasi | ticket lock, shart o'zgaruvchisi |
| turargohdagi "bo'sh joylar: 5" tablosi | semafor |

## 26.1. Yaxshi qulf nimalarni bajarishi kerak

**Oddiy qilib aytganda:** qulf — "bir vaqtda faqat bitta oqim" qoidasini ta'minlovchi vosita. Lekin faqat shu yetarli emas — hojatxonada bitta odam hamisha ichkarida o'tirib olsa ham "bitta odam" qoidasi
bajariladi, lekin bu foydasiz. Shuning uchun uchta talab:

| Talab | Ma'nosi | Hayotiy misol |
|---|---|---|
| **O'zaro istisno** | kritik seksiyada bir vaqtda bittadan ko'p oqim yo'q | hojatxonada bitta odam |
| **Adolat** | hech kim abadiy navbatda qolmaydi ("och qolmaydi") | navbat kelgach hamma kiradi |
| **Samaradorlik** | qulf bo'sh bo'lsa — arzon; talashuvda ham CPU'ni behuda yoqmaydi | eshikni ochish-yopish tez |

> **Eslab qoling:** qulfning 3 talabi: **istisno** (bittadan), **adolat** (och qolmaslik), **samara** (tez). Birinchisi buzilsa — poyga; ikkinchisi buzilsa — starvation; uchinchisi buzilsa — sekin dastur.

## 26.2. Nega oddiy o'zgaruvchi qulf bo'la olmaydi

**Oddiy qilib aytganda:** eng birinchi fikr — `band` degan o'zgaruvchi: 0 = bo'sh, 1 = band. Kirishdan oldin "band 0 bo'lsa, 1 qil va kir". Quyidagi dastur shuni sinaydi.

**Bu dastur nima qiladi (umumiy):** ikki oqim har biri 1 000 000 marta umumiy sanagichni 1 ga oshiradi. Har oshirishdan oldin "band" bayrog'ini tekshirib, o'rnatib oladi. To'g'ri bo'lsa, natija 2 000 000 bo'lishi kerak.

```c
/* oddiy_bayroq.c - "band" bayrog'i bilan qulf yasash urinishi (XATO) */
#include <pthread.h>
#include <stdio.h>

#define TAKROR 1000000

static volatile int band = 0;                   /* 0 - bo'sh, 1 - band */
static volatile long sanagich = 0;

static void *ishchi(void *arg)
{
    (void)arg;
    for (int i = 0; i < TAKROR; i++) {
        while (band)                            /* 1) tekshirish */
            ;
        band = 1;                               /* 2) o'rnatish - orada teshik bor! */
        sanagich++;                             /* kritik seksiya */
        band = 0;
    }
    return NULL;
}

int main(void)
{
    pthread_t a, b;
    pthread_create(&a, NULL, ishchi, NULL);
    pthread_create(&b, NULL, ishchi, NULL);
    pthread_join(a, NULL);
    pthread_join(b, NULL);
    printf("kutilgan: %d, chiqdi: %ld\n", 2 * TAKROR, sanagich);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 -pthread oddiy_bayroq.c -o oddiy_bayroq
$ ./oddiy_bayroq
kutilgan: 2000000, chiqdi: 1015723
```

Bu son **har safar boshqacha** chiqadi (sizda boshqa raqam bo'ladi) va doim 2 000 000 dan kam — taxminan yarmiga yaqin: yo'qolgan yangilanishlar.

**Kodda nimalar bor:**

| Qism | Vazifasi |
|---|---|
| `band` | qulf bayrog'i: 0 = bo'sh, 1 = band |
| `volatile` | kompilyator `band` va `sanagich` ni xotiradan har safar qayta o'qisin (registrda saqlab qolmasin) — 16-bob |
| `while (band) ;` | band bo'lsa bo'sh sikl bilan **kutish** (aylanish) |
| `band = 1;` | "endi men egalladim" |
| `sanagich++` | himoyalanishi kerak bo'lgan ish (kritik seksiya) |
| `band = 0;` | qulfni ochish |

**Xato qayerda?** `while (band)` va `band = 1` — **ikki alohida qadam**. Oraliqda boshqa oqim ham ishlay oladi:

| Vaqt | Oqim A | Oqim B | `band` |
|---|---|---|---|
| 1 | `while (band)` — 0 ko'rdi, sikldan chiqdi | | 0 |
| 2 | | `while (band)` — hali 0 ko'rdi, sikldan chiqdi | 0 |
| 3 | `band = 1`, kritik seksiyaga kirdi | | 1 |
| 4 | | `band = 1`, **u ham kirdi!** | 1 |

Ikkala oqim kritik seksiyada — o'zaro istisno buzildi, `sanagich++` (o'qi → qo'sh → yoz) yo'qolgan yangilanishlar beradi, natija 2 000 000 dan kam bo'ladi.

> **Eslab qoling:** muammo — "tekshirish" va "o'rnatish" orasidagi **teshik**. Yechim: ikkalasini **bitta bo'linmas (atomik) amal** qilish — buni faqat protsessor beradi.

## 26.3. Apparat yordami: bo'linmas buyruqlar

**Oddiy qilib aytganda:** protsessorlarda maxsus buyruqlar bor — ular o'qish va yozishni **bitta bo'linmas harakat** qilib bajaradi: bu buyruq davomida hech bir boshqa yadro shu xotira katagiga tega olmaydi.

| Primitiv | Nima qiladi (hammasi bitta bo'linmas amal) | x86 buyrug'i |
|---|---|---|
| **test-and-set** | eski qiymatni qaytaradi va katakka 1 yozadi | `xchg` |
| **compare-and-swap (CAS)** | `if (*p == kutilgan) { *p = yangi; return 1; } return 0;` | `lock cmpxchg` |
| **fetch-and-add** | eski qiymatni qaytaradi va unga qo'shadi | `lock xadd` |
| load-linked / store-conditional | o'qiydi; "orada hech kim tegmagan bo'lsa" yozadi | ARM: `ldxr` / `stxr` |

Bu — oddiy qilib aytganda "burama qulf": uni burasangiz, **bir harakatda** ham qulflaysiz, ham u oldin ochiq bo'lganmi-yo'qmi bilib olasiz. Ikki kishi bir vaqtda burolmaydi.

GCC'da bular `__atomic_...` funksiyalari sifatida mavjud (C11 da `<stdatomic.h>`). Ularni qo'lda yozilgan assembler'siz ishlatamiz.

### Test-and-set spinlock

**Bu dastur nima qiladi (umumiy):** 26.2 dagi o'sha vazifa — ikki oqim 1 000 000 martadan sanagichni oshiradi — lekin endi qulf `__atomic_test_and_set` bilan qurilgan. Natija doim aniq 2 000 000.

```c
/* tas_qulf.c - test-and-set spinlock (bo'linmas test + o'rnatish) */
#include <pthread.h>
#include <stdio.h>

#define TAKROR 1000000

static volatile char band = 0;
static long sanagich = 0;

static void qulf(void)
{
    while (__atomic_test_and_set(&band, __ATOMIC_ACQUIRE))   /* eskisini qaytarib, 1 yozadi */
        ;
}

static void och(void)
{
    __atomic_clear(&band, __ATOMIC_RELEASE);                  /* 0 yozadi */
}

static void *ishchi(void *arg)
{
    (void)arg;
    for (int i = 0; i < TAKROR; i++) {
        qulf();
        sanagich++;
        och();
    }
    return NULL;
}

int main(void)
{
    pthread_t a, b;
    pthread_create(&a, NULL, ishchi, NULL);
    pthread_create(&b, NULL, ishchi, NULL);
    pthread_join(a, NULL);
    pthread_join(b, NULL);
    printf("kutilgan: %d, chiqdi: %ld\n", 2 * TAKROR, sanagich);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 -pthread tas_qulf.c -o tas_qulf
$ ./tas_qulf
kutilgan: 2000000, chiqdi: 2000000
```

**Kodda nimalar bor:**

| Qism | Vazifasi |
|---|---|
| `__atomic_test_and_set(&band, ...)` | bitta bo'linmas amalda: `band` ning **eski** qiymatini qaytaradi va `band` ga 1 yozadi |
| `while (...) ;` | eski qiymat 1 bo'lsa (boshqasi egallab turibdi) — sikl davom etadi; **0 bo'lsa** — qulf bizniki, sikldan chiqamiz |
| `__atomic_clear` | `band` ga 0 yozadi — qulf ochiladi |
| `__ATOMIC_ACQUIRE` / `__ATOMIC_RELEASE` | xotira tartibi: qulfni olgandan keyingi o'qish/yozishlar undan **oldinga** o'tib ketmasin; ochishdan oldingilari **orqaga** o'tmasin (16-bob to'siqlari) |

**Nega endi ishlaydi?** Eski dasturda "tekshirish" va "o'rnatish" ikki qadam edi. Endi bitta amal:

| Vaqt | Oqim A | Oqim B | `band` |
|---|---|---|---|
| 1 | test-and-set: eski = 0, yozdi 1 → **qulf olindi** | | 0 → 1 |
| 2 | | test-and-set: eski = **1** (A yozib bo'lgan) → qulf yo'q, aylanadi | 1 |
| 3 | `sanagich++`, keyin `band = 0` | | 0 |
| 4 | | test-and-set: eski = 0 → **qulf olindi** | 0 → 1 |

Ikki oqim bir vaqtda "eski = 0" ola olmaydi, chunki amal bo'linmas: kim birinchi ulgursa, o'sha 0 oladi.

Kamchiligi: qulf **adolatsiz** — navbat yo'q, kim tez ulgursa o'sha oladi. Bitta oqim doim yutib, boshqasi "och" qolishi mumkin.

> **Eslab qoling:** **spinlock** = bo'sh sikl + atomik test-and-set. Kutayotgan oqim CPU'ni band qilib turadi ("aylanadi", *spin*).

## 26.4. Ticket lock — adolatli qulf

**Oddiy qilib aytganda:** shifoxona yoki bank navbatini eslang: eshikda chipta oling (raqam), tablo "hozir 7-raqam" deb turadi, siz 9-raqamsiz — kutasiz. Hamma **kelish tartibida** xizmat oladi. Adolat kafolatlanadi.

Kompyuterda ikkita hisoblagich: `kelgan` (keyingi beriladigan chipta) va `xizmat` (hozir xizmat ko'rsatilayotgan raqam). Chipta olish — **fetch-and-add** (bo'linmas "ol va 1 qo'sh").

**Bu dastur nima qiladi (umumiy):** to'rtta oqim har biri 200 000 martadan sanagichni oshiradi; himoya — ticket lock. Oxirida natija 800 000 ekanini va berilgan chiptalar soni xizmat ko'rsatilganlarga teng ekanini ko'rsatadi.

```c
/* ticket_qulf.c - ticket lock: navbat raqami bilan adolatli qulf */
#include <pthread.h>
#include <stdio.h>

#define OQIMLAR 4
#define TAKROR 200000

static unsigned kelgan = 0;                     /* keyingi beriladigan chipta raqami */
static unsigned xizmat = 0;                     /* hozir xizmat ko'rsatilayotgan raqam */
static long sanagich = 0;

static void qulf(void)
{
    unsigned mening = __atomic_fetch_add(&kelgan, 1, __ATOMIC_RELAXED);   /* chipta olish */
    while (__atomic_load_n(&xizmat, __ATOMIC_ACQUIRE) != mening)           /* navbatim kelguncha */
        ;
}

static void och(void)
{
    __atomic_store_n(&xizmat, xizmat + 1, __ATOMIC_RELEASE);               /* keyingi raqam */
}

static void *ishchi(void *arg)
{
    (void)arg;
    for (int i = 0; i < TAKROR; i++) {
        qulf();
        sanagich++;
        och();
    }
    return NULL;
}

int main(void)
{
    pthread_t t[OQIMLAR];
    for (int i = 0; i < OQIMLAR; i++)
        pthread_create(&t[i], NULL, ishchi, NULL);
    for (int i = 0; i < OQIMLAR; i++)
        pthread_join(t[i], NULL);
    printf("kutilgan: %d, chiqdi: %ld\n", OQIMLAR * TAKROR, sanagich);
    printf("berilgan chiptalar: %u, xizmat ko'rsatilgan: %u\n", kelgan, xizmat);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 -pthread ticket_qulf.c -o ticket_qulf
$ ./ticket_qulf
kutilgan: 800000, chiqdi: 800000
berilgan chiptalar: 800000, xizmat ko'rsatilgan: 800000
```

**Kodda nimalar bor:**

| Qism | Vazifasi |
|---|---|
| `kelgan` | chipta avtomati: keyingi chipta raqami |
| `xizmat` | tablo: hozir kimning navbati |
| `__atomic_fetch_add(&kelgan, 1, ...)` | bo'linmas "chiptani ol va avtomatni 1 ga oshir": har oqim **noyob** raqam oladi |
| `while (xizmat != mening) ;` | tabloda mening raqamim chiqquncha aylanish |
| `och()`: `xizmat + 1` | keyingi raqamga navbat berish (faqat qulf egasi yozadi, shuning uchun bo'linmas amal shart emas) |

Oxirgi qatordagi ikki son teng — **har bir chipta bir marta xizmat ko'rdi**: hech kim o'tkazib yuborilmadi, hech kim ikki marta kirmadi.

Kamchiligi: hamma bir xil `xizmat` katagiga qarab aylanadi — yadrolar orasida kesh qatori tinmay almashadi (21-bob, "false/true sharing"), oqimlar ko'paysa sekinlashadi.

> **Eslab qoling:** **ticket lock** = chipta (fetch-and-add) + tablo. Adolatli, lekin hali ham aylanadi (CPU yoqadi).

## 26.5. CAS — "o'qi, hisobla, o'zgarmagan bo'lsa yoz"

**Oddiy qilib aytganda:** qulfsiz dasturlashning asosiy vositasi **compare-and-swap (CAS)**. G'oya: qiymatni o'qing, yangisini hisoblang, so'ng "agar hali ham eskisi turgan bo'lsa — yangisini yoz; agar kimdir o'zgartirib ulgurgan bo'lsa —
yozma, qaytadan urin" deng. Hammasi bitta bo'linmas amalda tekshiriladi.

**Hayotdan misol:** kimdir siz bilan bir vaqtda umumiy doskadagi "rekord" ni yangilamoqchi. Siz: "doskada 90 yozilgan, men 95 yozaman — **agar hali ham 90 bo'lsa**". Agar bu orada kimdir 93 yozib qo'ygan bo'lsa — yozmaysiz,
yangi qiymat (93) ni ko'rib, qaytadan hisoblaysiz (95 > 93 — baribir yozasiz).

**Bu dastur nima qiladi (umumiy):** to'rtta oqim bir vaqtda `eng_katta` o'zgaruvchisini yangilab turadi (har biri o'z sonlari ichidan eng kattasini topadi). Qulfsiz, faqat CAS bilan. Oxirida natija bitta oqimda oddiy usulda
hisoblangan javob bilan solishtiriladi.

```c
/* cas_max.c - CAS bilan atomik "maksimumni yangilash" */
#include <pthread.h>
#include <stdio.h>

#define OQIMLAR 4
#define SONLAR 100000

static long eng_katta = 0;

static void atomic_max(long *p, long v)
{
    long eski = __atomic_load_n(p, __ATOMIC_RELAXED);
    while (v > eski && !__atomic_compare_exchange_n(p, &eski, v, 0,
                                                    __ATOMIC_RELAXED, __ATOMIC_RELAXED))
        ;           /* muvaffaqiyatsiz bo'lsa, eski ga hozirgi qiymat yozildi - qayta urinish */
}

static void *ishchi(void *arg)
{
    long id = (long)arg;
    for (long i = 0; i < SONLAR; i++)
        atomic_max(&eng_katta, (i * 7919 + id * 104729) % 1000003);   /* "tasodifiy" sonlar */
    return NULL;
}

int main(void)
{
    pthread_t t[OQIMLAR];
    for (long i = 0; i < OQIMLAR; i++)
        pthread_create(&t[i], NULL, ishchi, (void *)i);
    for (int i = 0; i < OQIMLAR; i++)
        pthread_join(t[i], NULL);

    long tekshiruv = 0;                         /* bitta oqimda oddiy usulda qayta hisoblaymiz */
    for (long id = 0; id < OQIMLAR; id++)
        for (long i = 0; i < SONLAR; i++) {
            long v = (i * 7919 + id * 104729) % 1000003;
            if (v > tekshiruv)
                tekshiruv = v;
        }
    printf("oqimlar topdi: %ld, oddiy hisob: %ld, mos: %s\n", eng_katta, tekshiruv,
           eng_katta == tekshiruv ? "ha" : "yo'q");
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 -pthread cas_max.c -o cas_max
$ ./cas_max
oqimlar topdi: 1000002, oddiy hisob: 1000002, mos: ha
```

**`atomic_max` qanday ishlaydi (bu bobning eng muhim 5 qatori):**

| Qadam | Kod | Ma'nosi |
|---|---|---|
| 1 | `eski = load(p)` | hozirgi rekordni o'qiymiz |
| 2 | `v > eski` | bizning son rekorddan kattami? Yo'q bo'lsa — tamom, yozish shart emas |
| 3 | `compare_exchange(p, &eski, v)` | **bo'linmas:** agar `*p == eski` bo'lsa → `*p = v` yoz va "muvaffaqiyat" qaytar; aks holda → `eski` ga **hozirgi** qiymatni yoz va "muvaffaqiyatsiz" qaytar |
| 4 | muvaffaqiyatsiz bo'lsa sikl qaytadan | `eski` allaqachon yangilangan — 2-qadamga qaytib, qayta tekshiramiz |

Trace (ikki oqim bir vaqtda, rekord 90; A 95 yozmoqchi, B 93):

| Vaqt | A | B | `*p` |
|---|---|---|---|
| 1 | eski = 90, 95 > 90 | eski = 90, 93 > 90 | 90 |
| 2 | CAS(90→95): `*p==90` ✓, **yozildi** | | 95 |
| 3 | | CAS(90→93): `*p` endi 95 ≠ 90 ✗, **eski = 95** ga yangilandi | 95 |
| 4 | | `93 > 95`? yo'q → sikl tugadi, hech narsa yozilmadi | 95 |

Hech qanday qulf yo'q, hech kim kutib uxlamadi — shuning uchun bunday usullar **qulfsiz** (lock-free) deyiladi.

> **Eslab qoling:** CAS naqshi: **o'qi → hisobla → "o'zgarmagan bo'lsa yoz" → bo'lmasa qaytadan.** Yadrodagi ko'p tezkor tuzilmalar shunga qurilgan.

## 26.6. Peterson algoritmi — qulfni faqat mantiq bilan qurish (tarixiy)

**Oddiy qilib aytganda:** apparat yordamisiz ham qulf qurish mumkinmi? Ha — **ikki** oqim uchun. Bu 1981-yilda G. Peterson o'ylab topgan sodda va chiroyli algoritm. Amalda ishlatilmaydi (apparat primitivlari tezroq),
lekin "qulf — bu aslida kelishuv qoidasi" degan fikrni ko'rsatadi.

**Hayotdan misol:** ikki kishi tor ko'prikka bir vaqtda yaqinlashdi. Qoida: har biri bayroqchasini ko'taradi ("men o'tmoqchiman"), keyin "navbat sendan" deb **o'ziga emas, raqibga** navbat beradi. Kimning
navbati bo'lsa va raqibi ham bayroq ko'targan bo'lsa — kutadi. Oxirgi "navbat sendan" degan odam yo'l beradi.

**Bu dastur nima qiladi (umumiy):** ikki oqim Peterson qulfi bilan 200 000 martadan sanagichni oshiradi. Faqat oddiy o'qish/yozish ishlatilgan (lekin ularni **qayta tartiblanmaydigan** `SEQ_CST` atomik amallar qilib yozdik — sababi quyida).

```c
/* peterson.c - Peterson algoritmi (ketma-ket izchil atomik amallar bilan) */
#include <pthread.h>
#include <stdio.h>

#define TAKROR 200000

static int bayroq[2];                           /* bayroq[i] = 1: "i-oqim kirmoqchi" */
static int navbat;
static long sanagich = 0;

static void qulf(int men)
{
    int u = 1 - men;
    __atomic_store_n(&bayroq[men], 1, __ATOMIC_SEQ_CST);        /* "men kirmoqchiman" */
    __atomic_store_n(&navbat, u, __ATOMIC_SEQ_CST);             /* "lekin avval sen" */
    while (__atomic_load_n(&bayroq[u], __ATOMIC_SEQ_CST) &&
           __atomic_load_n(&navbat, __ATOMIC_SEQ_CST) == u)
        ;                                       /* u xohlasa va navbat unda - kutaman */
}

static void och(int men)
{
    __atomic_store_n(&bayroq[men], 0, __ATOMIC_SEQ_CST);
}

static void *ishchi(void *arg)
{
    int men = (int)(long)arg;
    for (int i = 0; i < TAKROR; i++) {
        qulf(men);
        sanagich++;
        och(men);
    }
    return NULL;
}

int main(void)
{
    pthread_t a, b;
    pthread_create(&a, NULL, ishchi, (void *)0L);
    pthread_create(&b, NULL, ishchi, (void *)1L);
    pthread_join(a, NULL);
    pthread_join(b, NULL);
    printf("kutilgan: %d, chiqdi: %ld\n", 2 * TAKROR, sanagich);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 -pthread peterson.c -o peterson
$ ./peterson
kutilgan: 400000, chiqdi: 400000
```

**Kodda nimalar bor:**

| Qism | Vazifasi |
|---|---|
| `bayroq[2]` | har oqimning "kirmoqchiman" bayrog'i (0 yoki 1) |
| `navbat` | kimga yo'l berildi (so'nggi yozgan "yo'l beruvchi") |
| `u = 1 - men` | raqib raqami (0 ↔ 1) |
| `qulf()` | bayroq ko'tar → navbatni raqibga ber → raqib **kirmoqchi VA navbat unda** bo'lsa aylan |
| `och()` | bayroqni tushir |

Nega ishlaydi: ikkalasi bir vaqtda bayroq ko'tarsa, `navbat` ga **oxirgi yozgan** qiymat qoladi — bu odam raqibga yo'l bergan, shuning uchun kutadi; boshqasi kiradi.

**Nega `SEQ_CST` kerak?** Zamonaviy CPU va kompilyator o'qish-yozishlarni qayta tartiblaydi (16-bob). Oddiy `bayroq[men] = 1; navbat = u;` ni boshqa tartibda bajarish yoki yozuvni "ushlab turish" (store buffer)
mumkin — shunda ikkalasi ham kiradi. Peterson algoritmi **ketma-ket izchil** (sequentially consistent) xotirani talab qiladi; `SEQ_CST` aynan shuni beradi. Oddiy o'zgaruvchilar bilan yozilsa, zamonaviy mashinada **ishlamaydi**.

> **Eslab qoling:** Peterson — faqat 2 oqim, faqat `SEQ_CST` bilan. Amalda **apparat primitivlari** ishlatiladi. Dars: qulf = "bayroq + navbat" kelishuvi, uni xotira tartibi buzishi mumkin.

## 26.7. Aylanish yoki uxlash

**Oddiy qilib aytganda:** spinlock'da kutayotgan oqim CPU'ni yoqib turadi. Agar kutish **juda qisqa** bo'lsa (mikrosekundlar) — bu yaxshi: uxlab-uyg'onishdan arzonroq. Lekin kutish uzoq bo'lsa — behuda.

**Hayotdan misol:** lift kelishini kutyapsiz. 5 soniya bo'lsa — tugma yonida turasiz (**aylanish**). 5 daqiqa bo'lsa — o'tirib kitob o'qiysiz, lift kelsa sizni chaqirishadi (**uxlash**).

| Usul | Qachon yaxshi | Kamchiligi |
|---|---|---|
| **Aylanish** (spin) | kutish juda qisqa; yadroda uzilishlarni o'chirib kutilganda | CPU'ni behuda yoqadi; qulf egasi to'xtatilgan bo'lsa (bitta yadro!) — butunlay isrof |
| **Uxlash** (sleep) | kutish uzoq | uxlash/uyg'otish arzon emas (kontekst almashish) |

**Ikki fazali qulf:** avval biroz aylanish (qisqa kutishlarni arzon qoplaydi), keyin uxlash. Linux'da `pthread_mutex` shunday ishlaydi: **`futex`** syscall'i yordamida — talashuv bo'lmasa qulf **butunlay user rejimida** (bitta atomik amal, yadroga
kirmaydi), talashuv bo'lsa — yadroda uxlash navbatiga o'tadi.

Yadroda ham ikkala usul bor: spinlock (juda qisqa kritik seksiyalar, uyqu mumkin bo'lmagan joy — uzilish ishlovchisi) va mutex/semafor (uyqu mumkin).

> **Eslab qoling:** qisqa kutish → aylan; uzun → uxla. Amalda: ikkalasi birgalikda (**futex**).

## 26.8. Shart o'zgaruvchilari — "tayyor bo'lguncha uxla"

**Oddiy qilib aytganda:** qulf "bir vaqtda bitta" qoidasini beradi. Lekin ko'pincha oqimga **kutish** kerak: "navbatda element paydo bo'lguncha", "bufer bo'shaguncha". Shart o'zgaruvchisi (*condition variable*)
— aynan shu uchun: oqim uxlab yotadi va kerakli narsa tayyor bo'lganda uyg'otiladi.

**Hayotdan misol: shifoxona navbat chiptasi.** Shifokor bo'shaguncha har daqiqada eshikni taqillatib so'ramaysiz. Chipta olib o'tirasiz (uxlaysiz), raqamingiz chaqirilganda (signal) uyg'onasiz.

Shart o'zgaruvchisi ikki amal beradi (mutex bilan birga ishlatiladi):

| Amal | Ma'nosi |
|---|---|
| `pthread_cond_wait(&cv, &m)` | **bo'linmas ravishda** mutex'ni qo'yib yuboradi va uxlaydi; uyg'onganda mutex'ni qayta oladi |
| `pthread_cond_signal(&cv)` | uxlayotganlardan **bittasini** uyg'otadi |
| `pthread_cond_broadcast(&cv)` | **hammasini** uyg'otadi |

Nega "bo'linmas ravishda qo'yib yuborib uxlaydi"? Agar avval mutex qo'yilib, keyin uxlansa, shu orada boshqa oqim signal berib yuborishi mumkin — va biz uni **o'tkazib yuboramiz** (signal yo'qoladi, abadiy uxlab qolamiz).
`wait` ikkalasini bitta amalda bajaradi.

**Ikki temir qoida (38-mashq):**

1. Shartni doim **mutex ostida** tekshiring va o'zgartiring. Aks holda — "yo'qolgan uyg'otish".
2. Kutishni doim **`while`** ichida yozing, `if` ichida emas:

```c
pthread_mutex_lock(&m);
while (!shart_bajarildimi)          /* if EMAS, while! */
    pthread_cond_wait(&cv, &m);
/* ... shart bajarilgan, mutex bizda ... */
pthread_mutex_unlock(&m);
```

Nega `while`? Oqim uyg'onganda shart yana yolg'on bo'lishi mumkin: boshqa oqim oldinroq ulgurib, narsani olib ketgan bo'lishi mumkin; yoki "soxta uyg'onish" (*spurious wakeup*) — ba'zan `wait` hech kim signal
bermasa ham qaytadi. `while` har uyg'onishda shartni **qayta tekshiradi**.

To'liq namuna — bobning pastki qismidagi **novvoyxona** dasturi (ishlab chiqaruvchi–iste'molchi). Yadrodagi analogi — kutish navbatlari: MyOS'da `proc_sleep(kalit, qulf)` / `proc_wakeup(kalit)`.

> **Eslab qoling:** shart o'zgaruvchisi = mutex + uxlash. **`while`** ichida kut, **mutex ostida** tekshir.

## 26.9. Semaforlar

**Oddiy qilib aytganda:** semafor — **butun son hisoblagichi** va ikki amal. 1965-yilda Dijkstra o'ylab topgan.

**Hayotdan misol: turargoh kirishidagi tablo "bo'sh joylar: 5".** Mashina kirsa — son 1 ga kamayadi; chiqsa — oshadi. Son 0 bo'lsa — shlagbaum yopiq, mashina kutadi.

| Amal | Nomi | Nima qiladi |
|---|---|---|
| `sem_wait(&s)` | P, down, wait | agar `s > 0` bo'lsa → `s--` va davom; **`s == 0`** bo'lsa → **uxlaydi** (kutadi) |
| `sem_post(&s)` | V, up, post | `s++`; kutayotgan bo'lsa, bittasini uyg'otadi |

**Boshlang'ich qiymatga qarab turli vazifa bajaradi:**

| `s` boshlang'ich | Vazifasi | Misol |
|---|---|---|
| **1** | ikkilik semafor = **qulf** | bir vaqtda bitta oqim |
| **0** | **tartiblash**: "A tugagach B boshlansin" | B `sem_wait`, A oxirida `sem_post` |
| **N** | **N ta resurs**dan foydalanishni cheklash | bir vaqtda ko'pi bilan N ta disk so'rovi |

**Bu dastur nima qiladi (umumiy):** semaforning "0 dan boshlash" holati. Ikki oqim: A ma'lumotni tayyorlaydi (100 ms), B uni ishlatishi kerak. B A tugamaguncha boshlay olmasligi uchun semafor 0 dan boshlanadi: B `sem_wait` da uxlab
turadi, A tugagach `sem_post` qiladi.

```c
/* semafor_tartib.c - semafor 0 dan boshlanadi: "A tugagach B boshlansin" */
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

static sem_t tayyor;

static void *tayyorlovchi(void *arg)
{
    (void)arg;
    usleep(100000);                             /* uzoq tayyorlanish */
    printf("A: ma'lumot tayyor\n");
    sem_post(&tayyor);                          /* V: "tayyor" */
    return NULL;
}

static void *ishlatuvchi(void *arg)
{
    (void)arg;
    printf("B: tayyor bo'lishini kutyapman...\n");
    sem_wait(&tayyor);                          /* P: 0 bo'lsa uxlaydi */
    printf("B: ma'lumotni ishlataman\n");
    return NULL;
}

int main(void)
{
    pthread_t a, b;
    sem_init(&tayyor, 0, 0);                    /* boshlang'ich qiymat 0 */
    pthread_create(&b, NULL, ishlatuvchi, NULL);
    pthread_create(&a, NULL, tayyorlovchi, NULL);
    pthread_join(a, NULL);
    pthread_join(b, NULL);
    sem_destroy(&tayyor);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 -pthread semafor_tartib.c -o semafor_tartib
$ ./semafor_tartib
B: tayyor bo'lishini kutyapman...
A: ma'lumot tayyor
B: ma'lumotni ishlataman
```

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `sem_t tayyor` | semafor o'zgaruvchisi |
| `sem_init(&tayyor, 0, 0)` | 2-argument `0` — shu jarayon ichidagi oqimlar uchun; 3-argument — **boshlang'ich qiymat** (0) |
| `sem_wait` (B) | qiymat 0 → B uxlaydi |
| `sem_post` (A) | qiymat 1 bo'ladi → B uyg'onadi, qiymatni yana 0 ga tushirib o'tadi |

`B` oqimi **birinchi** yaratilgan va birinchi yozuvini chiqargan — lekin "ma'lumotni ishlataman" faqat A tugagach chiqdi. Semafor tartibni kafolatladi. Shu bobning "Loyiha" qismida `s = N` holati (ulanishlar puli) ko'rsatilgan.

Semafor mutex + shart o'zgaruvchisidan quriladi (44-mashq) va aksincha.

> **Eslab qoling:** semafor = hisoblagich: `wait` — kamaytir/kut, `post` — oshir/uyg'ot. **1** → qulf, **0** → tartiblash, **N** → N ta resurs.

## 26.10. Klassik masalalar

Bu uch masala parallel dasturlashning "alifbosi": hamma murakkab tizim ularning kombinatsiyasi.

### 26.10.1. Ishlab chiqaruvchi – iste'molchi (bounded buffer)

**Oddiy qilib aytganda:** bir tomon narsa **ishlab chiqaradi** (novvoy non yopadi), ikkinchi tomon **iste'mol qiladi** (xaridor oladi). Ular o'rtasida cheklangan o'lchamli bufer (peshtaxta). Bufer to'lsa — ishlab chiqaruvchi kutadi;
bo'sh bo'lsa — iste'molchi kutadi.

Yechim: bitta mutex (buferni himoyalaydi) + ikkita shart o'zgaruvchisi (`joy_bor`, `non_bor`) yoki ikkita semafor (`bo'sh = N`, `to'la = 0`). Bu — yadroda eng ko'p uchraydigan naqsh: pipe, klaviatura buferi,
disk so'rovlari navbati, tarmoq buferlari. To'liq dastur bobning pastki qismida (**novvoyxona**).

### 26.10.2. O'quvchilar – yozuvchilar

**Oddiy qilib aytganda:** ma'lumotni **o'qishni** ko'p oqim bir vaqtda qila oladi (o'qish hech narsani buzmaydi), lekin **yozuvchi yolg'iz** bo'lishi kerak (yozayotganda hech kim o'qimasligi kerak).

**Hayotdan misol: muzey.** Ko'p tomoshabin rasmni birga ko'ra oladi. Restavrator esa rasm bilan ishlaganda zalda hech kim bo'lmasligi kerak.

Oddiy mutex ortiqcha qattiq: o'quvchilarni ham bittadan o'tkazadi. **Rwlock** (read-write lock) ikkita rejim beradi: "o'qish" (ko'pchilik birga) va "yozish" (yolg'iz).

**Bu dastur nima qiladi (umumiy):** uchta o'quvchi bir vaqtda `rdlock` oladi, bitta yozuvchi `wrlock` olmoqchi. Dastur ikki narsani tekshiradi: (1) o'quvchilar haqiqatan **birga** o'qiy olishdimi; (2) yozuvchi ichkarida
o'quvchi yoki boshqa yozuvchi bilan **to'qnashmadimi**.

```c
/* oquvchi_yozuvchi.c - pthread_rwlock: ko'p o'quvchi birga, yozuvchi yolg'iz */
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

static pthread_rwlock_t rw = PTHREAD_RWLOCK_INITIALIZER;
static pthread_mutex_t kalit = PTHREAD_MUTEX_INITIALIZER;
static int oquvchilar, eng_kop_oquvchi, yozuvchilar, buzilish;

static void *oquvchi(void *arg)
{
    (void)arg;
    pthread_rwlock_rdlock(&rw);
    pthread_mutex_lock(&kalit);
    oquvchilar++;
    if (oquvchilar > eng_kop_oquvchi)
        eng_kop_oquvchi = oquvchilar;
    if (yozuvchilar > 0)
        buzilish++;                             /* o'quvchi yozuvchi bilan birga bo'lmasligi kerak */
    pthread_mutex_unlock(&kalit);
    usleep(50000);
    pthread_mutex_lock(&kalit);
    oquvchilar--;
    pthread_mutex_unlock(&kalit);
    pthread_rwlock_unlock(&rw);
    return NULL;
}

static void *yozuvchi(void *arg)
{
    (void)arg;
    usleep(10000);                              /* o'quvchilar avval kirib olsin */
    pthread_rwlock_wrlock(&rw);
    pthread_mutex_lock(&kalit);
    yozuvchilar++;
    if (oquvchilar > 0 || yozuvchilar > 1)
        buzilish++;
    pthread_mutex_unlock(&kalit);
    usleep(50000);
    pthread_mutex_lock(&kalit);
    yozuvchilar--;
    pthread_mutex_unlock(&kalit);
    pthread_rwlock_unlock(&rw);
    return NULL;
}

int main(void)
{
    pthread_t o[3], y;
    for (int i = 0; i < 3; i++)
        pthread_create(&o[i], NULL, oquvchi, NULL);
    pthread_create(&y, NULL, yozuvchi, NULL);
    for (int i = 0; i < 3; i++)
        pthread_join(o[i], NULL);
    pthread_join(y, NULL);
    printf("bir vaqtda o'qigan o'quvchilar (eng ko'pi): %d\n", eng_kop_oquvchi);
    printf("yozuvchi o'quvchi/yozuvchi bilan to'qnashdi: %d marta\n", buzilish);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 -pthread oquvchi_yozuvchi.c -o oquvchi_yozuvchi
$ ./oquvchi_yozuvchi
bir vaqtda o'qigan o'quvchilar (eng ko'pi): 3
yozuvchi o'quvchi/yozuvchi bilan to'qnashdi: 0 marta
```

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `pthread_rwlock_rdlock` | **o'qish** qulfi: boshqa o'quvchilar bilan birga mumkin, yozuvchi bo'lsa — kutadi |
| `pthread_rwlock_wrlock` | **yozish** qulfi: hamma o'quvchi va yozuvchilar chiqquncha kutadi, so'ng yolg'iz kiradi |
| `kalit` (mutex) | faqat bizning hisoblagichlarni (`oquvchilar`, `yozuvchilar`) himoyalaydi — kuzatuv uchun |
| `eng_kop_oquvchi` | bir vaqtda ichkarida bo'lgan o'quvchilarning eng ko'pi |
| `buzilish` | qoida buzilgan holatlar soni (0 bo'lishi kerak) |

Natija: uchala o'quvchi birga o'qidi (3) — oddiy mutex bo'lganda bu 1 bo'lardi; yozuvchi esa hech kim bilan to'qnashmadi (0). Yozuvchi o'quvchilar chiqquncha `wrlock` da kutdi.

**Muammo — "ochlik":** yangi o'quvchilar tinimsiz kelaversa, yozuvchi hech qachon navbat ololmasligi mumkin. Yechim — kutayotgan yozuvchi bo'lsa, yangi o'quvchilarni kiritmaslik. Yadroda: `rwlock`, `rw_semaphore`. O'qish
juda ko'p, yozish kam bo'lsa — hatto rwlock ham sekin (kesh qatori almashinuvi); u yerda RCU ishlatiladi (26.13). 45-mashq.

### 26.10.3. Ovqatlanayotgan faylasuflar

**Oddiy qilib aytganda:** 5 faylasuf doira shaklidagi stolda; har ikki qo'shni orasida bittadan vilka (jami 5 vilka). Ovqatlanish uchun **ikkala** (chap va o'ng) vilka kerak. Ular o'ylaydi, vilkalarni oladi, yeydi, qaytaradi.

Bu — **qulflar tartibi** masalasi: ikkita resurs (vilka) kerak bo'lganda deadlock paydo bo'lishi mumkin. Agar hamma bir vaqtda **chap** vilkasini olsa — hamma o'ngini kutadi, hech kim yemaydi: **deadlock**.

Yechimlar: bittasi avval **o'ngini** oladi (tartib buziladi); yoki vilkalarni doim **global tartibda** olish (kichik raqamli vilka avval). Quyidagi dastur ikkinchisini qiladi.

**Bu dastur nima qiladi (umumiy):** 5 oqim (faylasuf) har biri 20 000 marta ovqatlanadi. Vilka — mutex. Har faylasuf ikki vilkasidan **kichik raqamlisini avval** oladi. Hamma tugatsa — deadlock bo'lmagan.

```c
/* faylasuflar.c - 5 faylasuf, vilkalar GLOBAL TARTIBDA olinadi: deadlock yo'q */
#include <pthread.h>
#include <stdio.h>

#define F 5
#define TAKROR 20000

static pthread_mutex_t vilka[F];
static long ovqat[F];

static void *faylasuf(void *arg)
{
    int men = (int)(long)arg;
    int chap = men, ong = (men + 1) % F;
    int birinchi = chap < ong ? chap : ong;     /* kichik raqamlisini AVVAL */
    int ikkinchi = chap < ong ? ong : chap;
    for (int i = 0; i < TAKROR; i++) {
        pthread_mutex_lock(&vilka[birinchi]);
        pthread_mutex_lock(&vilka[ikkinchi]);
        ovqat[men]++;                           /* ovqatlanyapti */
        pthread_mutex_unlock(&vilka[ikkinchi]);
        pthread_mutex_unlock(&vilka[birinchi]);
    }
    return NULL;
}

int main(void)
{
    pthread_t t[F];
    for (int i = 0; i < F; i++)
        pthread_mutex_init(&vilka[i], NULL);
    for (long i = 0; i < F; i++)
        pthread_create(&t[i], NULL, faylasuf, (void *)i);
    for (int i = 0; i < F; i++)
        pthread_join(t[i], NULL);
    for (int i = 0; i < F; i++)
        printf("faylasuf %d: %ld marta ovqatlandi\n", i, ovqat[i]);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 -pthread faylasuflar.c -o faylasuflar
$ ./faylasuflar
faylasuf 0: 20000 marta ovqatlandi
faylasuf 1: 20000 marta ovqatlandi
faylasuf 2: 20000 marta ovqatlandi
faylasuf 3: 20000 marta ovqatlandi
faylasuf 4: 20000 marta ovqatlandi
```

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `vilka[F]` | 5 ta mutex — har biri bitta vilka |
| `chap = men`, `ong = (men+1) % F` | faylasuf `men` ning ikki vilkasi (doira: 4-faylasufning o'ngi — 0-vilka) |
| `birinchi`, `ikkinchi` | **kichik raqamlisi birinchi**: hamma faylasuf vilkalarni o'sish tartibida oladi |
| `ovqat[men]++` | ovqatlanish (kritik seksiya, ikkala vilka qo'lda) |

Nega bu deadlock'ni yo'qotadi? 4-faylasuf: chap = 4, o'ng = 0 → avval **0**, keyin **4** oladi (boshqalardan farqli: ular avval chap, keyin o'ng). Natijada hamma "avval chap, keyin o'ng" qilib aylana hosil qila olmaydi — bitta faylasuf albatta teskari
tartibda oladi va **aylanma kutish** uziladi (26.11).

> **Eslab qoling:** 3 klassik masala: **ishlab chiqaruvchi–iste'molchi** (bufer: mutex + 2 shart), **o'quvchilar–yozuvchilar** (rwlock), **faylasuflar** (qulflar tartibi → deadlock yo'q).

## 26.11. Deadlock — abadiy kutish

**Oddiy qilib aytganda:** **deadlock** (o'zaro to'silib qolish) — bir nechta oqim bir-birini kutib, hech biri oldinga siljiy olmaydigan holat.

**Hayotdan misol: chorrahadagi 4 mashina.** To'rt tomondan kelgan mashinalar o'rtada tiqildi: har biri o'z joyini egallagan, keyingi joyni kutyapti, hech kim orqaga qayta olmaydi.

**Bu dastur nima qiladi (umumiy):** ikki oqim ikkita qulfni **teskari tartibda** oladi: 1-oqim avval A keyin B, 2-oqim avval B keyin A. Ikkalasi birinchi qulfni olib, ikkinchisini kutadi — abadiy. Dasturimiz kutishni 1 soniya bilan
cheklaydi (`pthread_mutex_timedlock`), shunda qotib qolmasdan deadlock'ni **aniqlab** xabar beradi.

```c
/* deadlock.c - ikki oqim qulflarni teskari tartibda oladi (XATO), kutish 1 soniya bilan cheklangan */
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

static pthread_mutex_t qulf_a = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t qulf_b = PTHREAD_MUTEX_INITIALIZER;

static int kut(pthread_mutex_t *m)              /* 1 soniyadan ko'p kutsa - deadlock deymiz */
{
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    t.tv_sec += 1;
    return pthread_mutex_timedlock(m, &t);
}

static void *oqim1(void *arg)
{
    (void)arg;
    pthread_mutex_lock(&qulf_a);                /* avval A */
    usleep(100000);                             /* ikkinchi oqim B ni olib ulgursin */
    if (kut(&qulf_b) != 0)                      /* keyin B */
        printf("oqim1: A ni ushlab, B ni 1 soniya kutdi - DEADLOCK\n");
    else
        pthread_mutex_unlock(&qulf_b);
    pthread_mutex_unlock(&qulf_a);
    return NULL;
}

static void *oqim2(void *arg)
{
    (void)arg;
    pthread_mutex_lock(&qulf_b);                /* avval B - teskari tartib! */
    usleep(100000);
    if (kut(&qulf_a) != 0)                      /* keyin A */
        printf("oqim2: B ni ushlab, A ni 1 soniya kutdi - DEADLOCK\n");
    else
        pthread_mutex_unlock(&qulf_a);
    pthread_mutex_unlock(&qulf_b);
    return NULL;
}

int main(void)
{
    pthread_t a, b;
    pthread_create(&a, NULL, oqim1, NULL);
    pthread_create(&b, NULL, oqim2, NULL);
    pthread_join(a, NULL);
    pthread_join(b, NULL);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 -pthread deadlock.c -o deadlock
$ ./deadlock | sort
oqim1: A ni ushlab, B ni 1 soniya kutdi - DEADLOCK
```

(Oddiy `pthread_mutex_lock` bilan dastur butunlay **qotib qolardi** — shuning uchun `timedlock` ishlatdik. `| sort` faqat ikki qator tartibini barqarorlashtiradi.)

**Kodda nimalar bor:**

| Qism | Vazifasi |
|---|---|
| `qulf_a`, `qulf_b` | ikkita resurs |
| `oqim1` | A → (kutish) → B tartibida oladi |
| `oqim2` | B → (kutish) → A tartibida oladi (**teskari**) |
| `usleep(100000)` | ikkala oqim birinchi qulfini olib ulgursin deb (deadlock'ni "kafolatlash" uchun) |
| `kut()` | `timedlock`: 1 soniyada ololmasa 0 dan boshqa son qaytaradi |

Trace:

| Vaqt | Oqim 1 | Oqim 2 |
|---|---|---|
| 1 | A ni oldi ✓ | B ni oldi ✓ |
| 2 | B ni so'raydi — **B 2-oqimda**, kutadi | A ni so'raydi — **A 1-oqimda**, kutadi |
| 3 | hech biri oldinga siljimaydi — **aylanma kutish**: 1 → 2 → 1 | |

### Deadlock'ning 4 sharti (Coffman, 1971)

Deadlock faqat to'rtta shart **birga** bajarilganda bo'ladi:

| # | Shart | Bizning misolda |
|---|---|---|
| 1 | **O'zaro istisno** — resursni bir vaqtda faqat bittasi egallaydi | mutex shunday |
| 2 | **Ushlab turib kutish** — bitta resursni ushlab, boshqasini kutadi | A ni ushlab B ni kutadi |
| 3 | **Tortib olib bo'lmaslik** — resursni egasidan majburan olib bo'lmaydi | mutex'ni majburan ochib bo'lmaydi |
| 4 | **Aylanma kutish** — A B ni kutadi, B A ni kutadi (yoki C orqali) | 1 → 2 → 1 |

**Birortasini buzsangiz — deadlock bo'lmaydi.** Amalda eng ko'p buziladigan — **4-shart**: qulflarni **doim bir xil global tartibda** olish (faylasuflar yechimi: kichik raqam avval). Linux'da **lockdep** qulflar tartibi grafini kuzatadi va aylana topsa
ogohlantiradi. Aylanani topish — grafdagi siklni DFS bilan qidirish (28-bob, 48-mashq).

Boshqa yondashuvlar:

| Yondashuv | Qaysi shartni buzadi | Misol |
|---|---|---|
| Qulflar global tartibi | 4 (aylanma kutish) | faylasuflar, yadro |
| `trylock` + orqaga chekinish | 2 (ushlab turmaslik) | ikkinchi qulf band bo'lsa — birinchisini ham qo'yib yuborib, qaytadan |
| Aniqlash + tiklash | 3 (tortib olish) | ma'lumotlar bazasi tranzaksiyani bekor qiladi |

> **Eslab qoling:** deadlock = 4 shart **birga**. Eng oddiy davo: **qulflarni har doim bir xil tartibda ol.**

## 26.12. Real parallellik xatolari

Tadqiqotchilar (Lu va boshq., 2008; MySQL, Apache, Mozilla, OpenOffice) real dasturlardagi parallellik xatolarini o'rgandi. Natija:

1. **Atomiklik buzilishi** — "tekshir-keyin-foydalan" ikki qadam orasida boshqa oqim holatni o'zgartiradi:
   ```c
   if (p->fayl != NULL)          /* A: tekshirdi */
       fputs(s, p->fayl);         /* B shu orada p->fayl = NULL qildi -> qulash */
   ```
   Yechim: ikkalasini **bitta qulf ostida**.
2. **Tartib buzilishi** — "A B dan oldin bo'lishi kerak" deb faraz qilingan, lekin kafolatlanmagan (oqim yaratildi, lekin u ishlatadigan tuzilma hali boshlanmagan). Yechim: shart o'zgaruvchisi yoki semafor (26.8–26.9).
3. **Deadlock** (26.11).

Topilgan xatolarning ~97% i **birinchi ikki tur** (deadlock'dan tashqari). Ya'ni asosiy xavf — poyga emas, balki "ikki qadam orasida holat o'zgardi" va "tartib kafolatlanmadi".

> **Eslab qoling:** ko'p xato deadlock emas, balki **atomiklik** ("tekshir va ishlat — bitta qulf ostida") va **tartib** ("avval A, keyin B" — shart o'zgaruvchisi/semafor bilan majburlang).

## 26.13. Qulfsiz usullar haqida qisqacha

- **Atomik hisoblagichlar**, **CAS sikllari** (26.5) — oddiy holatlar uchun.
- **Per-CPU ma'lumot** — umuman bo'lishmaslik (21-bob).
- **RCU** (Read-Copy-Update, Linux): o'quvchilar **umuman qulf olmaydi**; yozuvchi yangi nusxa yaratib, ko'rsatkichni atomik almashtiradi va eski nusxani "hamma o'quvchilar chiqib ketgach" o'chiradi. O'qish juda
  ko'p, yozish kam bo'lgan joylarda (routing jadvallari, fayl tizimi keshlari) — ulkan tezlik. Bu — Linux yadrosining eng muhim va eng murakkab mexanizmlaridan biri; hozircha g'oyasini bilish yetarli.

## Hayotdan misol va to'liq dastur

**Novvoyxona peshtaxtasi.** Novvoy non yopib peshtaxtaga qo'yadi, xaridor oladi. Peshtaxtaga faqat 3 ta non sig'adi: to'lsa — novvoy kutadi; bo'sh bo'lsa — xaridor kutadi.

**Bu dastur nima qiladi (umumiy):** ikki novvoy har biri 5 tadan non yopadi, bitta xaridor 10 ta non oladi. Peshtaxta — 3 o'rinli aylanma bufer. Mutex buferni himoyalaydi; ikkita shart o'zgaruvchisi kutishni
boshqaradi: `joy_bor` ("peshtaxtada joy bo'shadi" — novvoylarni uyg'otadi) va `non_bor` ("non qo'yildi" — xaridorni uyg'otadi). Oxirida 10 ta non olingani va peshtaxta bo'sh qolgani tekshiriladi.

```c
/* novvoyxona.c - ishlab chiqaruvchi/iste'molchi: mutex + ikkita shart o'zgaruvchisi */
#include <pthread.h>
#include <stdio.h>

#define SIGIM 3

static int peshtaxta[SIGIM];
static int soni = 0, bosh = 0;                  /* aylanma bufer */
static int kutdi_novvoy = 0, kutdi_xaridor = 0;
static pthread_mutex_t kalit = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t joy_bor = PTHREAD_COND_INITIALIZER;   /* "peshtaxtada joy bo'shadi" */
static pthread_cond_t non_bor = PTHREAD_COND_INITIALIZER;   /* "non qo'yildi" */

static void *novvoy(void *arg)
{
    int raqam = (int)(long)arg;
    for (int i = 0; i < 5; i++) {
        pthread_mutex_lock(&kalit);
        while (soni == SIGIM) {                 /* while, if emas: uyg'ongach QAYTA tekshirish */
            kutdi_novvoy++;
            pthread_cond_wait(&joy_bor, &kalit);   /* kalitni qo'yib uxlaydi */
        }
        peshtaxta[(bosh + soni) % SIGIM] = raqam * 100 + i;
        soni++;
        pthread_cond_signal(&non_bor);          /* xaridorni uyg'otish */
        pthread_mutex_unlock(&kalit);
    }
    return NULL;
}

static void *xaridor(void *arg)
{
    long *olindi = arg;
    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(&kalit);
        while (soni == 0) {
            kutdi_xaridor++;
            pthread_cond_wait(&non_bor, &kalit);
        }
        (void)peshtaxta[bosh];                  /* nonni olish */
        bosh = (bosh + 1) % SIGIM;
        soni--;
        (*olindi)++;
        pthread_cond_signal(&joy_bor);          /* novvoyni uyg'otish */
        pthread_mutex_unlock(&kalit);
    }
    return NULL;
}

int main(void)
{
    pthread_t n1, n2, x;
    long olindi = 0;
    pthread_create(&x, NULL, xaridor, &olindi);
    pthread_create(&n1, NULL, novvoy, (void *)1L);
    pthread_create(&n2, NULL, novvoy, (void *)2L);
    pthread_join(n1, NULL);
    pthread_join(n2, NULL);
    pthread_join(x, NULL);

    printf("Xaridor oldi: %ld ta non, peshtaxtada qoldi: %d ta\n", olindi, soni);
    printf("Kutishlar bo'ldimi: %s\n", kutdi_novvoy + kutdi_xaridor > 0 ? "ha (bu normal)" : "yo'q");
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 -pthread novvoyxona.c -o novvoyxona
$ ./novvoyxona
Xaridor oldi: 10 ta non, peshtaxtada qoldi: 0 ta
Kutishlar bo'ldimi: ha (bu normal)
$ gcc -Wall -Wextra -O2 -pthread -fsanitize=thread novvoyxona.c -o novvoyxona_tsan
$ ./novvoyxona_tsan
Xaridor oldi: 10 ta non, peshtaxtada qoldi: 0 ta
Kutishlar bo'ldimi: ha (bu normal)
```

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `peshtaxta[SIGIM]`, `bosh`, `soni` | aylanma bufer: `bosh` — eng eski non, `soni` — nechta non bor; yangi non o'rni `(bosh + soni) % SIGIM` |
| `kalit` | buferni himoyalovchi mutex |
| `joy_bor` | shart o'zgaruvchisi: novvoy shu yerda "joy bo'shaguncha" uxlaydi |
| `non_bor` | shart o'zgaruvchisi: xaridor shu yerda "non paydo bo'lguncha" uxlaydi |
| `while (soni == SIGIM)` | peshtaxta to'la — novvoy kutadi (`while`! uyg'onganda qayta tekshiradi) |
| `pthread_cond_signal(&non_bor)` | non qo'yildi — xaridorni uyg'ot |
| `kutdi_novvoy`, `kutdi_xaridor` | nechta marta kutishga to'g'ri kelgani (kuzatuv; mutex ostida o'zgaradi) |

Oqimlar har safar boshqa tartibda ishlaydi, lekin natija doim bir xil: 10 ta non olindi, 0 ta qoldi. ThreadSanitizer poyga topmadi.

**Sinab ko'ring:** `while (soni == SIGIM)` ni `if (soni == SIGIM)` ga almashtiring. Nega bu xavfli (ikki novvoy bir signal bilan uyg'onsa nima bo'ladi)? `SIGIM` ni 1 qiling — dastur hali ham to'g'ri ishlaydimi?

## Bob xulosasi (yodlash uchun)

1. Oddiy bayroq qulf bo'la olmaydi: "tekshirish" va "o'rnatish" orasida **teshik** bor. Yechim — apparat **bo'linmas** buyruqlari: test-and-set, **CAS**, fetch-and-add.
2. Spinlock = atomik test-and-set + aylanish; **ticket lock** = chipta (fetch-and-add) + tablo (adolatli); **CAS** naqshi: o'qi → hisobla → o'zgarmagan bo'lsa yoz, bo'lmasa qaytadan. Qisqa kutish — aylan, uzun — uxla (**futex**).
3. **Shart o'zgaruvchisi:** mutex + uxlash; `wait` ni doim `while` ichida, mutex ostida. **Semafor:** hisoblagich; 1 → qulf, 0 → tartiblash, N → N ta resurs.
4. Klassik masalalar: **ishlab chiqaruvchi–iste'molchi** (mutex + 2 shart), **o'quvchilar–yozuvchilar** (rwlock), **faylasuflar** (qulflar tartibi).
5. **Deadlock** = 4 shart birga (o'zaro istisno, ushlab kutish, tortib olmaslik, **aylanma kutish**). Davo: qulflarni **doim bir xil global tartibda** olish. Boshqa xatolar: atomiklik va tartib buzilishi.

## Savol-javob

**Savol:** Nega `volatile` qulf uchun yetmaydi?
**Javob:** `volatile` faqat kompilyatorga "xotiradan har safar o'qi" deydi. U o'qish-yozishni **bo'linmas** qilmaydi (26.2 dagi dastur `volatile` bilan ham xato qildi) va CPU tartiblashiga to'siq qo'ymaydi. Bo'linmaslik uchun atomik amal kerak.

**Savol:** Nega yadroda ham `pthread_mutex` ishlatib bo'lmaydi?
**Javob:** `pthread_mutex` user dasturlari uchun va uxlash uchun `futex` syscall'iga tayanadi — yadro o'zi syscall'ni bajaradi, uni chaqira olmaydi. Yadroda o'z spinlock/mutex/semaforlari bor (bu bobdagi g'oyalar bilan).

**Savol:** Semafor va mutex farqi nima?
**Javob:** Mutex'ni faqat uni **olgan** oqim ocha oladi (egalik bor). Semaforni istalgan oqim `post` qila oladi — shuning uchun semafor "signal berish" (tartiblash) uchun ham yaroqli.

## O'zingizni tekshiring

1. Nega Peterson algoritmi zamonaviy CPU'da `SEQ_CST`siz ishlamaydi?
2. CAS bilan atomik `x = max(x, v)` qanday yoziladi?
3. Nega `wait` doim `while` ichida bo'lishi kerak?
4. Semafor boshlang'ich qiymati 0, 1, N — har biri nimaga?
5. Deadlock'ning 4 sharti va amalda qaysi biri buziladi?

<details><summary>Javoblar</summary>

1. CPU va kompilyator yozish/o'qishlarni qayta tartiblaydi — algoritm ketma-ket izchillikka tayanadi.
2. 26.5 dagi `atomic_max`: o'qish, `v` katta bo'lsa CAS, muvaffaqiyatsiz bo'lsa yangi qiymat bilan qaytadan.
3. Uyg'onganda shart yolg'on bo'lishi mumkin (boshqa oqim oldin oldi yoki soxta uyg'onish).
4. 0 — tartiblash (kutish), 1 — qulf, N — N ta resurs.
5. O'zaro istisno, ushlab kutish, tortib olmaslik, aylanma kutish; amalda — aylanma kutish (qulflar tartibi).
</details>

## Mashq

- **29**, **34**, **38** — agar qilmagan bo'lsangiz.
- **44** (semafor), **45** (o'quvchilar-yozuvchilar qulfi), **48** (deadlock'ni graf bilan aniqlash).
- Qo'shimcha: `faylasuflar.c` da tartib qoidasini olib tashlang (hamma avval **chap**ni olsin) va ikkala vilka orasiga `usleep(1000)` qo'ying — deadlock'ni o'zingiz ko'ring (`timeout 10` bilan ishga tushiring!), keyin global tartib bilan tuzating.

<!-- loyiha:boshi -->
## Loyiha: ulanishlar puli (semafor)

**Maqsad:** **cheklangan resurs**ni ko'p oqim orasida taqsimlash: bir vaqtda ko'pi bilan 3 ta ulanish. Semafor aynan shu uchun
yaratilgan: u "nechta joy bo'sh" sanagichi (26.7).
**Bobdan ishlatiladi:** semafor (`sem_wait`/`sem_post`), mutex bilan kuzatuv, oqimlarni kutish.

**Talab:** 8 ta oqim har biri bitta ulanish oladi, biroz "ishlaydi" (20 ms) va qaytaradi. Semafor boshlang'ich qiymati = 3.
- `sem_wait` — joy bor bo'lsa sanagichni kamaytiradi, yo'q bo'lsa **uxlaydi**;
- `sem_post` — sanagichni oshiradi va kutayotganlardan birini uyg'otadi.
**Tekshiruv:** hozir necha oqim "ishlayotgani"ni mutex bilan himoyalangan sanagich orqali kuzatib, **hech qachon 3 dan oshmaganini** isbotlaymiz.

```c
/* puli.c - semafor bilan cheklangan resurs */
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

#define ULANISH 3
#define OQIMLAR 8

static sem_t joylar;
static pthread_mutex_t kalit = PTHREAD_MUTEX_INITIALIZER;
static int hozir, eng_kop, jami, buzilish;

static void *ishchi(void *arg)
{
    (void)arg;
    sem_wait(&joylar);                          /* ulanish olish (kerak bo'lsa kutamiz) */

    pthread_mutex_lock(&kalit);
    hozir++;
    if (hozir > eng_kop)
        eng_kop = hozir;
    if (hozir > ULANISH)
        buzilish++;                             /* bu hech qachon bo'lmasligi kerak */
    pthread_mutex_unlock(&kalit);

    usleep(20000);                              /* "ishlaymiz" */

    pthread_mutex_lock(&kalit);
    hozir--;
    jami++;
    pthread_mutex_unlock(&kalit);

    sem_post(&joylar);                          /* ulanishni qaytarish */
    return NULL;
}

int main(void)
{
    pthread_t t[OQIMLAR];
    sem_init(&joylar, 0, ULANISH);
    for (int i = 0; i < OQIMLAR; i++)
        pthread_create(&t[i], NULL, ishchi, NULL);
    for (int i = 0; i < OQIMLAR; i++)
        pthread_join(t[i], NULL);
    sem_destroy(&joylar);

    printf("Ishlagan oqimlar: %d\n", jami);
    printf("Bir vaqtda eng ko'pi bilan: %d (chegara %d)\n", eng_kop, ULANISH);
    printf("Chegara buzilishlari: %d\n", buzilish);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 -pthread puli.c -o puli
$ ./puli
Ishlagan oqimlar: 8
Bir vaqtda eng ko'pi bilan: 3 (chegara 3)
Chegara buzilishlari: 0
$ time ./puli > /dev/null

real	0m0.062s
user	0m0.003s
sys	0m0.000s
```

8 oqim × 20 ms, uchtadan bo'lib ishlaydi: 3 ta guruh (3+3+2) ≈ 60 ms. `time` shuni ko'rsatadi. Semafor sanagichi 1 bo'lsa (`sem_init(..., 1)`) — bu mutex bilan bir xil:
hammasi ketma-ket, ≈ 160 ms.

**Kengaytiring:** `ULANISH` ni 1, 4, 8 qiling va `time` bilan tezlikni o'lchang. `sem_post` ni unutib qo'ysangiz (ulanish qaytarilmasa) nima bo'ladi?

## Mustaqil loyiha: qayta ishlatiladigan to'siq (barrier) ★★★

**Vazifa:** **to'siq** — `N` ta oqim hammasi shu nuqtaga yetib kelmaguncha **hech biri o'tmaydi**. Fazali hisob-kitoblarda
(o'yin dvigateli, ilmiy hisob: "hamma 1-bosqichni tugatsin, keyin 2-bosqich") ishlatiladi. Sizning vazifangiz — uni **mutex + shart o'zgaruvchisi** (26.6) bilan
o'zingiz yozish. `pthread_barrier_t` **taqiqlangan**. Fayl: `tosiq.c`.

**Talab:** `struct tosiq` va uch funksiya:
- `void tosiq_yarat(struct tosiq *b, int n)`
- `void tosiq_kut(struct tosiq *b)` — `n` ta oqim yetib kelguncha uxlaydi, oxirgisi kelganda hammasini uyg'otadi
- `void tosiq_ozod(struct tosiq *b)`

**Eng nozik joyi — qayta ishlatish:** to'siq **bir necha marta** ketma-ket ishlatiladi. Tez oqim to'siqdan o'tib, keyingi fazada **yana**
to'siqqa kelishi mumkin — sekin oqimlar hali oldingisidan chiqib ulgurmagan bo'lsa ham. Sanagichni oddiy nolga qaytarish bu holda xato
(tez oqim o'tgan-o'tmaganligini hech kim bilmaydi). Yechim: har bir "avlod" (generation) uchun raqam — kutayotgan oqim **avlod o'zgarguncha** uxlaydi.

**Sinov (aniq):** `N = 4` oqim, `5` faza. Umumiy massiv `int bajarildi[5]` (har faza nechta oqim ishini tugatgani). Har oqim (`id = 0..3`):

```text
har faza f = 0..4 uchun:
    agar f > 0 va bajarildi[f-1] != 4  ->  xato++          (oldingi faza to'liq tugamagan holda o'tib ketdi!)
    usleep(((id + f) % 3) * 1000)                          (turli tezlik: poyga hosil qilish uchun)
    bajarildi[f] ni 1 ga oshiring                          (mutex yoki atomik amal bilan)
    tosiq_kut(&b)
```

`main` hamma oqimni kutadi va chiqaradi:

**Kutilgan natija** (`darslik/loyihalar/26_tosiq/kutilgan.txt`):

```text
Faza 1: 4/4 oqim bajardi
Faza 2: 4/4 oqim bajardi
Faza 3: 4/4 oqim bajardi
Faza 4: 4/4 oqim bajardi
Faza 5: 4/4 oqim bajardi
To'siq buzilmadi: 0 ta xato
```

**Maslahat** (yechim emas):
- `struct tosiq { pthread_mutex_t m; pthread_cond_t c; int n, kelgan, avlod; }`.
- `tosiq_kut`: mutexni oling → `mening_avlodim = avlod`; `kelgan++`. Agar `kelgan == n`: `avlod++`, `kelgan = 0`, `pthread_cond_broadcast`. Aks holda:
  `while (mening_avlodim == avlod) pthread_cond_wait(&c, &m);` → mutexni qo'yib yuboring.
- Nega `if` emas, `while`? Yolg'on uyg'onish (spurious wakeup) va boshqa avlod signali (26.6).
- Nega `kelgan = 0` ni oxirgi oqim qiladi va `avlod++` — kutayotganlar `avlod` o'zgarganini ko'rib chiqadi?
- Xatoni ataylab hosil qiling: `avlod` siz yozing va bir necha marta ishga tushiring — ba'zan qotib qoladi yoki `xato > 0` chiqadi
  (`timeout 20` bilan ishga tushiring!).
- `-fsanitize=thread` bilan tekshiring.

**Tekshirish:**

```bash
gcc -Wall -Wextra -O2 -g -pthread tosiq.c -o dastur && timeout 20 ./dastur | diff - ~/C_loyha/darslik/loyihalar/26_tosiq/kutilgan.txt && echo "TO'G'RI"
```
<!-- loyiha:oxiri -->

Keyingi bob: [27-bob. Qurilmalar va fayl tizimlari](27-fayl-tizimlari.md)
