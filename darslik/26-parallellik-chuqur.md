# 26-bob. Parallellik chuqur: qulflarni qurish, semaforlar, klassik masalalar, deadlock

> **Bu bobdan keyin:** qulflar noldan qanday quriladi (Peterson, test-and-set, CAS, ticket), shart
> o'zgaruvchilari va semaforlarni, klassik masalalarni (ishlab chiqaruvchi-iste'molchi, o'quvchilar-yozuvchilar,
> ovqatlanayotgan faylasuflar), deadlock'ning 4 shartini va real dasturlardagi parallellik xatolari
> turlarini bilasiz. (OSTEP "Concurrency" qismi.) 15-bobning davomi. Mashqlar: 29, 34, 38, 44, 45, 48.

> **To'liq ishlaydigan misol:** [misollar/26_faylasuflar.c](misollar/26_faylasuflar.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Hayotdan misollar

**Qulfga talablar — hojatxona eshigi (26.1).** Yaxshi qulf uchta shartni bajaradi: bir vaqtda faqat
bitta odam ichkarida (**o'zaro istisno**), hech kim abadiy navbatda qolib ketmaydi (**adolat**), va
qulflash-ochish tez (**unumdorlik**).

**Oddiy o'zgaruvchi nega yetmaydi (26.2).** Eshikda "band/bo'sh" yozuvi bor. Ikki kishi bir vaqtda qaraydi:
"bo'sh". Ikkalasi ham yozuvni "band" qiladi va ikkalasi ham kiradi. Qarash va yozish orasida vaqt bor —
muammo shu oraliqda.

**Test-and-set — bitta harakatda qarash va aylantirish (26.4).** Eshikdagi burama qulf: uni burasangiz,
bir vaqtning o'zida ham qulflaysiz, ham u oldin ochiq bo'lganmi — sezasiz. Ikki kishi bir vaqtda burolmaydi.
Protsessorning atomik buyruqlari (`xchg`, `lock cmpxchg`) aynan shunday bo'linmas.

**Aylanish yoki uxlash (26.5).** Lift kelishini kutyapsiz: 5 soniya bo'lsa — tugma yonida turasiz
(aylanish, spin). 5 daqiqa bo'lsa — o'tirib kitob o'qiysiz, lift kelsa chaqirishadi (uxlash).

**Shart o'zgaruvchisi — shifoxona navbat chiptasi (26.6).** Shifokor bo'shaguncha har daqiqada eshikni
taqillatib so'ramaysiz. Chipta olib o'tirasiz (`pthread_cond_wait` — uxlaysiz) va raqamingiz chaqirilganda
(`pthread_cond_signal`) uyg'onasiz.

**Semafor — turargoh kirishidagi tablo "bo'sh joylar: 5" (26.7).** Har kirgan mashina sonni bittaga
kamaytiradi, chiqqani oshiradi. 0 bo'lsa — shlagbaum yopiq, kutasiz.

**Ishlab chiqaruvchi va iste'molchi — novvoyxona peshtaxtasi (26.8).** Novvoy non yopib, peshtaxtaga
qo'yadi. Xaridor peshtaxtadan oladi. Peshtaxtaga faqat 3 ta non sig'adi: to'lsa — novvoy kutadi; bo'sh
bo'lsa — xaridor kutadi. Bu — yadroda eng ko'p uchraydigan naqsh (pipe, klaviatura buferi, disk navbati).

**O'quvchilar-yozuvchilar — muzey (26.8).** Ko'p tomoshabin rasmni birga ko'ra oladi (o'qish). Restavrator
esa rasm bilan ishlaganda zalda hech kim bo'lmasligi kerak (yozish).

**Deadlock'ning 4 sharti — chorrahadagi 4 mashina (26.9).** To'rt tomondan kelgan mashinalar o'rtada
tiqildi: har biri o'z joyini egallagan (**o'zaro istisno**), joyini bo'shatmay keyingisini kutyapti
(**ushlab turib kutish**), hech kimni majburan chiqarib bo'lmaydi (**tortib olish yo'q**), va kutish
aylana bo'lib yopilgan (**aylanma kutish**). Bittasini buzsangiz — tiqilinch bo'lmaydi.

### To'liq dastur: novvoyxona

Ikki novvoy har biri 5 tadan non yopadi, bitta xaridor 10 ta non oladi. Peshtaxtaga 3 ta non sig'adi.

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

Oqimlar har safar boshqa tartibda ishlaydi, lekin natija doim bir xil: 10 ta non olindi, 0 ta qoldi.
ThreadSanitizer poyga topmadi.

**Sinab ko'ring:** `while (soni == SIGIM)` ni `if (soni == SIGIM)` ga almashtiring. Nega bu xavfli
(ikki novvoy bir signal bilan uyg'onsa nima bo'ladi)? `SIGIM` ni 1 qiling — dastur hali ham to'g'ri ishlaydimi?

## 26.1. Qulfga talablar

Yaxshi qulf: 1) **o'zaro istisno** — kritik seksiyada bir vaqtda bittadan ko'p oqim yo'q;
2) **adolat** — hech kim abadiy kutmaydi (och qolmaydi); 3) **samaradorlik** — qulf bo'sh bo'lsa arzon,
talashuvda ham CPU'ni behuda yoqmaydi.

## 26.2. Nega oddiy o'zgaruvchi yetmaydi

```c
while (band) ;      /* 1) tekshirish  */
band = 1;           /* 2) o'rnatish   */
```

Ikki oqim bir vaqtda 1-qatordan o'tadi (ikkalasi `band == 0` ni ko'rgan) va ikkalasi kiradi.
Tekshirish va o'rnatish orasida "teshik" bor. Yechim — ularni **bo'linmas** qilish.

## 26.3. Faqat o'qish/yozish bilan: Peterson algoritmi (tarixiy)

```c
int bayroq[2], navbat;

void lock(int men)            /* men = 0 yoki 1 */
{
    int u = 1 - men;
    bayroq[men] = 1;          /* "men kirmoqchiman" */
    navbat = u;               /* "lekin avval sen" (xushmuomalalik) */
    while (bayroq[u] && navbat == u)
        ;                     /* u ham xohlasa va navbat unda bo'lsa - kutaman */
}
void unlock(int men) { bayroq[men] = 0; }
```

Ikki oqim uchun to'g'ri — **ketma-ket izchil xotira** faraz qilinganda. Zamonaviy CPU'lar xotira
amallarini qayta tartiblaydi (15-bob), shuning uchun to'siqlarsiz bu kod **ishlamaydi**. Tarixiy va
o'qitish ahamiyati bor: qulfni faqat mantiq bilan qurish mumkinligini ko'rsatadi. Amalda — apparat yordami.

## 26.4. Apparat primitivlari

| Primitiv | Ma'nosi (bitta bo'linmas amal) | x86 |
|---|---|---|
| test-and-set | eski qiymatni qaytarib, 1 yozish | `xchg` |
| compare-and-swap | `if (*p == kutilgan) { *p = yangi; return 1; } return 0;` | `lock cmpxchg` |
| fetch-and-add | eski qiymatni qaytarib, qo'shish | `lock xadd` |
| load-linked / store-conditional | o'qish; "hech kim tegmagan bo'lsa" yozish | ARM `ldxr/stxr` |

- **test-and-set spinlock** — oddiy, adolatsiz (34-mashq).
- **ticket lock** (fetch-and-add) — adolatli: navbat raqami (34-mashq).
- **CAS** — qulfsiz tuzilmalarning asosi: "o'qi → yangisini hisobla → hech kim o'zgartirmagan bo'lsa yoz,
  aks holda qaytadan".

```c
/* CAS bilan atomik "maksimumni yangilash" */
void atomic_max(long *p, long v)
{
    long eski = __atomic_load_n(p, __ATOMIC_RELAXED);
    while (v > eski && !__atomic_compare_exchange_n(p, &eski, v, 0,
                                                    __ATOMIC_RELAXED, __ATOMIC_RELAXED))
        ;           /* muvaffaqiyatsiz bo'lsa, eski ga hozirgi qiymat yozildi - qayta urinish */
}
```

## 26.5. Aylanish yoki uxlash

Spinlock bitta yadroli tizimda yoki qulf egasi to'xtatilganda **behuda** CPU yoqadi (34-mashqdagi
ticket lock sekinligi — aynan shu). Yechim: kutish uzoqqa cho'zilsa — **uxlash**. Linux'da `futex`
syscall'i: talashuv bo'lmasa qulf butunlay user rejimida (atomik amal), bo'lsa — yadroda uxlash navbati.
**Ikki fazali qulf:** avval biroz aylanish (qisqa kutish arzon), keyin uxlash. `pthread_mutex` shunday.

## 26.6. Shart o'zgaruvchilari (condition variables)

Qulf "bir vaqtda bitta" ni beradi; **shart kutish** uchun esa boshqa vosita kerak: "navbat bo'sh
bo'lmaguncha kut". Shart o'zgaruvchisi — kutish navbati + ikkita amal:

- `wait(cv, mutex)` — mutex'ni **atomik ravishda** qo'yib yuborib uxlaydi; uyg'onganda uni qayta oladi;
- `signal(cv)` — bitta kutuvchini uyg'otadi; `broadcast(cv)` — hammasini.

**Ikki temir qoida (38-mashq):**
1. Shartni doim **mutex ostida** tekshiring va o'zgartiring (aks holda — yo'qolgan uyg'otish).
2. **`while`**, `if` emas: `while (!shart) wait(cv, m);` — uyg'onganda shart yana yolg'on bo'lishi mumkin
   (boshqa oqim oldinroq ulgurgan yoki "soxta uyg'onish").

Yadrodagi analogi — kutish navbatlari: MyOS'da `proc_sleep(kalit, qulf)` / `proc_wakeup(kalit)`.

## 26.7. Semaforlar

Dijkstra (1965): butun son hisoblagichi + ikki amal:
- `P(s)` (wait, down): `s--`; agar `s < 0` bo'lsa — uxlash;
- `V(s)` (post, up): `s++`; kutayotgan bo'lsa — bittasini uyg'otish.

Boshlang'ich qiymat bilan turli vazifalar:
- `s = 1` — **ikkilik semafor** = qulf;
- `s = 0` — **tartiblash**: "A tugagach B boshlansin" (B `P`, A oxirida `V`);
- `s = N` — **N ta resursdan** foydalanishni cheklash (masalan, bir vaqtda ko'pi bilan 4 ta disk so'rovi).

Semafor mutex + shart o'zgaruvchisi bilan quriladi (44-mashq) va aksincha.

## 26.8. Klassik masalalar

### Ishlab chiqaruvchi — iste'molchi (bounded buffer)

N o'rinli bufer: ishlab chiqaruvchilar qo'yadi (to'la bo'lsa kutadi), iste'molchilar oladi (bo'sh bo'lsa
kutadi). Ikki shart o'zgaruvchisi (`bo'sh_joy_bor`, `element_bor`) yoki ikki semafor (`bo'sh = N`,
`to'la = 0`) + mutex. Pipe, disk so'rovlari navbati, tarmoq buferlari — hammasi shu. 38-mashq.

### O'quvchilar — yozuvchilar

Ko'p oqim bir vaqtda **o'qishi** mumkin, lekin yozuvchi yolg'iz bo'lishi kerak. Oddiy yechim: o'quvchilar
soni; birinchi o'quvchi yozuvchi qulfini oladi, oxirgisi qo'yib yuboradi. Muammo — yozuvchi "och qolishi"
mumkin (o'quvchilar oqimi tugamasa). Yechim — kutayotgan yozuvchi bo'lsa, yangi o'quvchilarni kiritmaslik.
Yadroda: `rwlock`, `rw_semaphore`; o'qish juda ko'p bo'lsa — RCU (26.11). 45-mashq.

### Ovqatlanayotgan faylasuflar

5 faylasuf, 5 vilka (har birining chap va o'ngida). Hamma avval chap vilkani olsa — **deadlock**
(hamma o'ng vilkani kutadi). Yechimlar: bittasi avval o'ngni oladi (tartibni buzish), yoki vilkalarni
**global tartibda** olish (kichik raqamlisini avval) — 26.9 dagi qulflar tartibi qoidasi.

## 26.9. Deadlock — to'rt shart (Coffman, 1971)

Deadlock faqat to'rtta shart **birga** bajarilganda bo'ladi:

1. **O'zaro istisno** — resursni bir vaqtda faqat bitta egallaydi.
2. **Ushlab turib kutish** — oqim bitta resursni ushlab, boshqasini kutadi.
3. **Tortib olib bo'lmaslik** — resursni egasidan majburan olib bo'lmaydi.
4. **Aylanma kutish** — A B ni kutadi, B C ni, C A ni.

Birortasini buzsangiz — deadlock bo'lmaydi. Amalda eng ko'p buziladigan — 4-shart: **qulflarni doim
bir xil global tartibda olish**. Linux'da `lockdep` qulflar tartibi grafini kuzatadi va aylana topsa
ogohlantiradi. Aylanani topish — grafdagi siklni DFS bilan qidirish (28-bob, 48-mashq).

Boshqa yondashuvlar: aniqlash va tiklash (ma'lumotlar bazalari tranzaksiyani bekor qiladi), `trylock`
bilan orqaga chekinish (4 → ushlab turmaslik), bankir algoritmi (nazariy).

## 26.10. Real parallellik xatolari (tadqiqotlar bo'yicha eng ko'plari)

1. **Atomiklik buzilishi:** "tekshir-keyin-foydalan" ikki qadam orasida boshqa oqim holatni o'zgartiradi.
   ```c
   if (p->fayl != NULL)          /* A: tekshirdi */
       fputs(s, p->fayl);         /* B shu orada p->fayl = NULL qildi -> qulash */
   ```
   Yechim: ikkalasini bitta qulf ostida.
2. **Tartib buzilishi:** "A B dan oldin bo'lishi kerak" deb faraz qilingan, lekin kafolatlanmagan
   (oqim yaratildi, lekin u ishlatadigan tuzilma hali boshlanmagan). Yechim: shart o'zgaruvchisi/semafor.
3. **Deadlock** (26.9).

Mashhur tadqiqotda (Lu va boshq., 2008; MySQL, Apache, Mozilla, OpenOffice) topilgan parallellik
xatolarining ~70% i deadlock **emas** edi, ularning esa deyarli hammasi (~97%) — shu ikki turdan.

## 26.11. Qulfsiz usullar haqida qisqacha

- **Atomik hisoblagichlar**, **CAS sikllari** — oddiy holatlar uchun.
- **Per-CPU ma'lumot** — umuman bo'lishmaslik (21-bob).
- **RCU** (Read-Copy-Update, Linux): o'quvchilar **umuman qulf olmaydi**; yozuvchi yangi nusxa yaratib,
  ko'rsatkichni atomik almashtiradi va eski nusxani "hamma o'quvchilar chiqib ketgach" o'chiradi.
  O'qish juda ko'p, yozish kam bo'lgan joylarda (routing jadvallari, fayl tizimi keshlari) — ulkan tezlik.
  Bu — Linux yadrosining eng muhim va eng murakkab mexanizmlaridan biri; hozircha g'oyasini bilish yetarli.

## 26.12. O'zingizni tekshiring

1. Nega Peterson algoritmi zamonaviy CPU'da to'siqsiz ishlamaydi?
2. CAS bilan atomik `x = max(x, v)` qanday yoziladi?
3. Nega `wait` doim `while` ichida bo'lishi kerak?
4. Semafor boshlang'ich qiymati 0, 1, N — har biri nimaga?
5. Deadlock'ning 4 sharti va amalda qaysi biri buziladi?

<details><summary>Javoblar</summary>

1. CPU va kompilyator yozish/o'qishlarni qayta tartiblaydi — algoritm ketma-ket izchillikka tayanadi.
2. 26.4 dagi `atomic_max`: o'qish, v katta bo'lsa CAS, muvaffaqiyatsiz bo'lsa yangi qiymat bilan qaytadan.
3. Uyg'onganda shart yolg'on bo'lishi mumkin (boshqa oqim oldin oldi yoki soxta uyg'onish).
4. 0 — tartiblash (kutish), 1 — qulf, N — N ta resurs.
5. O'zaro istisno, ushlab kutish, tortib olmaslik, aylanma kutish; amalda — aylanma kutish (qulflar tartibi).
</details>

## 26.13. Mashqlar

- **29**, **34**, **38** — agar qilmagan bo'lsangiz.
- **44** (semafor), **45** (o'quvchilar-yozuvchilar qulfi), **48** (deadlock'ni graf bilan aniqlash).
- Qo'shimcha: ovqatlanayotgan faylasuflarni 5 ta pthread va 5 ta mutex bilan yozing; avval deadlock'ni
  ko'ring (hammasi chap vilkani olsin + `usleep`), keyin global tartib bilan tuzating.

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
