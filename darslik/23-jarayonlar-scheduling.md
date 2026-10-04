# 23-bob. Jarayonlar va rejalashtirish (scheduling)

> **Bu bobda nima o'rganasiz:** jarayon abstraksiyasini; uning holatlarini va yadrodagi tuzilmasini (PCB); kontekst almashishni; rejalashtirish algoritmlarini (FIFO, SJF, STCF, Round Robin, MLFQ, adolatli ulush, Linux CFS)
> va ularni baholash metrikalarini. (Mavzu "Operating Systems: Three Easy Pieces" kitobining virtualizatsiya qismidan.)
> **Oldindan nima kerak:** 14-bob (fork/exec/wait), 17-bob (registrlar), 21-bob (kesh/TLB).   **Vaqt:** 6–7 soat.
> Mashqlar: 35.

> **To'liq ishlaydigan misol:** [misollar/23_jarayonlar.c](misollar/23_jarayonlar.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Bu bob nima haqida?

Kompyuterda 4–8 ta yadro bor, lekin yuzlab dastur "bir vaqtda" ishlaydi. Qanday? Operatsion tizim CPU'ni dasturlar orasida **juda tez navbatlashtiradi** — har biri o'zini yagona ega deb his qiladi.
Bu bobda ikki savolga javob beramiz: (1) dastur "boshqa dasturga o'tish" **qanday** bajariladi (kontekst almashish)? (2) **qaysi** dasturga navbat beriladi (rejalashtirish)?

**Hayotdan misol: kompyuter klubidagi o'yinchi.** Klubda bitta kompyuter (bitta CPU yadrosi) va o'nta o'yinchi bor. Har biri navbat bilan bir necha daqiqa o'ynaydi. Almashishlar juda tez bo'lsa, har biri
"kompyuter faqat meniki" deb o'ylaydi. Jarayon — aynan shu: har bir dastur o'zini butun protsessorga ega deb his qiladi.

| Klubda | Operatsion tizimda |
|---|---|
| bitta kompyuter | bitta CPU yadrosi |
| o'yinchilar | jarayonlar |
| navbat bilan o'ynash | **vaqtni bo'lish** (time sharing) |
| "kim keyingi?" | **rejalashtirish** (scheduling) |
| o'yinchi almashishi | **kontekst almashish** |

## 23.1. Jarayon — "virtual CPU"

Kompyuterda 4–8 ta yadro bor, lekin yuzlab dastur "bir vaqtda" ishlaydi. OS har bir dasturga **o'zining alohida CPU'si va xotirasi bordek** illyuziya beradi. Bu illyuziya — **jarayon** (process):

- **manzil maydoni** — o'z xotirasi (kod, ma'lumot, heap, stek) — 24-bob;
- **CPU holati** — registrlar, `rip`, `rsp`, `rflags`;
- **OS resurslari** — ochiq fayllar, joriy papka, signal ishlovchilari, ota-bola munosabatlari.

Illyuziya **vaqtni bo'lish** (time sharing) bilan yaratiladi: har bir jarayon qisqa vaqt (kvant, ~1–10 ms) ishlaydi, keyin OS uni to'xtatib, boshqasini ishga tushiradi.

Ikki qatlam (OSTEP'ning muhim g'oyasi):

- **mexanizm** — *qanday* almashtirish (kontekst almashish, uzilishlar) — past daraja;
- **siyosat** — *qaysi* jarayonni ishga tushirish (rejalashtirish algoritmi) — yuqori daraja.

## 23.2. Jarayon holatlari

**Hayotdan misol: poliklinikadagi bemor.**

- **Tayyor (ready)** — navbatda o'tiribdi, shifokor bo'shashini kutyapti.
- **Ishlayapti (running)** — shifokor qabulida.
- **Bloklangan (blocked)** — tahlil topshirdi, natijani kutyapti. Uni chaqirishdan foyda yo'q — natija kelmaguncha u baribir hech narsa qila olmaydi. Natija kelsa — yana navbatga (ready) o'tadi.

```text
          yaratildi
              │
              ▼
   ┌──────► TAYYOR ◄────────────┐
   │          │ scheduler tanladi│ kvant tugadi (preemption)
   │          ▼                  │
   │      ISHLAYAPTI ────────────┘
   │          │ I/O kutish, sleep, qulf
   │          ▼
   └───── BLOKLANGAN (uxlayapti)
  hodisa    │
  sodir     ▼ exit()
  bo'ldi   ZOMBI ──(ota wait qildi)──► yo'q qilindi
```

MyOS'da: `enum proc_state` (`kernel/proc/process.h`): `PROC_EMBRYO`, `PROC_READY`, `PROC_RUNNING`, `PROC_BLOCKED` (uxlayapti), `PROC_STOPPED` (Ctrl-Z), `PROC_ZOMBIE`, `PROC_DEAD`.

Linux'da bu holatlarni `ps` ko'rsatadi: **`R`** (running/runnable), **`S`** (sleeping — uxlayapti), **`T`** (stopped), **`Z`** (zombi). Quyida to'rttasini ham haqiqiy jarayonlarda ushlaymiz:

```c
/* zombi.c - ota o'lmagan, bola tugagan: zombi */
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    if (fork() == 0)
        exit(0);                        /* bola darhol tugaydi */
    sleep(2);                           /* ota esa wait() QILMAYDI - bola zombi bo'lib qoladi */
    return 0;
}
```

```console
$ gcc -Wall -Wextra zombi.c -o zombi
$ sleep 3 & p=$!; sleep 0.3; echo "uxlayotgan: $(ps -o stat= -p $p | cut -c1)"; kill -STOP $p; sleep 0.2; echo "to'xtatilgan: $(ps -o stat= -p $p | cut -c1)"; kill -CONT $p; kill $p; wait $p 2>/dev/null; true
uxlayotgan: S
to'xtatilgan: T
$ ./zombi & z=$!; sleep 0.5; echo "bola holati: $(ps -o stat= --ppid $z | cut -c1)"; wait $z
bola holati: Z
```

**Nima ko'rdik:** `sleep` — **`S`** (kutyapti), `kill -STOP` dan keyin — **`T`** (to'xtatilgan; xuddi Ctrl-Z). `zombi` ning bolasi tugagan, lekin otasi `wait` qilmagani uchun **`Z`** (zombi) holatida.
`R` (ishlayapti) holatini `ps` odatda ushlay olmaydi — jarayon juda qisqa vaqt ishlaydi, keyin uxlaydi.

## 23.3. PCB — yadrodagi jarayon tuzilmasi

**Hayotdan misol: bemorning kartasi.** Shifokor bemorni qayta chaqirganda, uning kartasini ochadi: oxirgi tashxis, dorilar. Yadro ham har bir jarayonning "kartasini" saqlaydi: registrlar, ochiq fayllar,
xotira xaritasi, holati.

Har bir jarayon uchun yadroda tuzilma ("Process Control Block"): Linux'da `task_struct` (~10 KB!), MyOS'da `struct process`. Unda: pid, holat, saqlangan registrlar (kontekst), yadro steki, manzil maydoni (`mm`),
ochiq fayllar jadvali, ota, bolalar, signal ma'lumotlari, scheduler ma'lumotlari (9.9 dagi `struct process` ga qarang).

## 23.4. Kontekst almashish — mexanizm

**Hayotdan misol: o'qituvchining sinfdan sinfga o'tishi.** O'qituvchi 5-A dan 5-B ga o'tishdan oldin jurnalga qayerda to'xtaganini yozib qo'yadi. Qaytib kelganda aynan shu joydan davom etadi.
Yadro ham jarayonni almashtirishdan oldin uning barcha registrlarini saqlaydi va keyingisinikini tiklaydi. Bu tez, lekin bepul emas — o'qituvchi har 10 soniyada sinf almashtirsa, dars o'tishga vaqt qolmaydi.

Taymer uzilishi keldi (yoki jarayon o'zi uxladi):

```text
1. CPU uzilishni qabul qiladi: user rejimidan yadroga, jarayonning yadro stekiga
   (rip, rsp, rflags avtomatik saqlanadi)
2. Uzilish kirishi (isr.asm): qolgan registrlar stekka
3. Yadro: "vaqt tugadi" -> scheduler() -> keyingi jarayon tanlanadi
4. switch_to(eski, yangi): yadro rsp ni almashtiradi (switch.asm) - endi YANGI jarayonning
   yadro stekidamiz
5. CR3 = yangi jarayonning sahifa jadvali (manzil maydoni almashdi, TLB tozalandi - 21-bob)
6. Stekdan registrlar tiklanadi, iretq -> yangi jarayon user rejimida davom etadi
```

Kontekst almashish **qimmat**: ~1–5 mikrosekund, lekin asosiy yashirin narx — kesh va TLB'ning "sovushi". Shuning uchun kvant juda kichik bo'lishi mumkin emas. Narxini o'lchaymiz:

```c
/* kontekst.c - kontekst almashish narxini o'lchash (pipe ping-pong) */
#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define ALMASH 200000

static double hozir(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (double)t.tv_sec + (double)t.tv_nsec / 1e9;
}

/* bitta bayt yozish / o'qish; xatoda dasturni to'xtatish */
static void yoz1(int fd, char c)
{
    if (write(fd, &c, 1) != 1)
        _exit(1);
}

static void oqi1(int fd)
{
    char c;
    if (read(fd, &c, 1) != 1)
        _exit(1);
}

int main(void)
{
    cpu_set_t bir_cpu;
    CPU_ZERO(&bir_cpu);
    CPU_SET(0, &bir_cpu);
    sched_setaffinity(0, sizeof(bir_cpu), &bir_cpu);    /* ikkala jarayon BITTA yadroda */

    int ota_bola[2], bola_ota[2];
    if (pipe(ota_bola) < 0 || pipe(bola_ota) < 0)
        return 1;

    if (fork() == 0) {                          /* BOLA: kelganni qaytarib yuboradi */
        for (int i = 0; i < ALMASH; i++) {
            oqi1(ota_bola[0]);
            yoz1(bola_ota[1], 'x');
        }
        _exit(0);
    }
    double b = hozir();
    for (int i = 0; i < ALMASH; i++) {          /* OTA: yozadi, javobni kutadi */
        yoz1(ota_bola[1], 'x');
        oqi1(bola_ota[0]);
    }
    double t = hozir() - b;
    wait(NULL);
    printf("%d marta oldinga-orqaga: %.3f s\n", ALMASH, t);
    printf("bitta kontekst almashish ~ %.2f mikrosekund\n", t / (2.0 * ALMASH) * 1e6);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 kontekst.c -o kontekst
$ ./kontekst
200000 marta oldinga-orqaga: 0.641 s
bitta kontekst almashish ~ 1.60 mikrosekund
```

**Bu dastur nima qiladi (umumiy):** ikki jarayon (ota va bola) **bitta CPU yadrosida** bir bayt "ping-pong" o'ynaydi: ota yozadi va javob kutadi (bloklanadi) → bola uyg'onib o'qiydi, yozadi, kutadi → ota uyg'onadi...
Har bir yo'nalish — **bitta kontekst almashish**. Umumiy vaqtni almashishlar soniga bo'lib, narxini topamiz.

**Qismlar:** `sched_setaffinity` — ikkalasini bitta yadroga "mixlaydi" (aks holda ikki yadroda parallel ishlab, almashish bo'lmasdi); ikkita `pipe` — ikki yo'nalishdagi kanal (14.5); `read` bo'sh quvurda **bloklanadi** →
yadro boshqa jarayonga o'tadi. Natija: bir necha mikrosekund (sizning kompyuteringizda boshqa chiqadi) — shuning uchun scheduler kvantni millisekundlarda tanlaydi (aks holda vaqtning ko'pi almashishga ketardi).

MyOS'da: `kernel/arch/isr.asm`, `kernel/proc/switch.asm` (~30 qator — o'qing!), `kernel/proc/process.c` (`sched_tick`, `scheduler_loop`), `docs/06-jarayonlar.md`.

## 23.5. Rejalashtirish metrikalari

Algoritmlarni qanday solishtiramiz? To'rtta o'lchov:

- **Aylanish vaqti** (turnaround): `tugash − kelish` — ish qancha vaqtda bajarildi (fon vazifalari uchun muhim).
- **Javob vaqti** (response): `birinchi_ishga_tushish − kelish` — interaktivlik (klaviatura, ekran uchun muhim).
- **Adolat** — hamma o'z ulushini oladimi.
- **O'tkazuvchanlik** (throughput) — vaqt birligida bajarilgan ishlar.

Bular bir-biriga zid: aylanish vaqti uchun eng yaxshi algoritm javob vaqti uchun yomon va aksincha.

## 23.6. Algoritmlar

**Hayotdan misol: kassadagi navbat.**

- **FCFS** — oddiy navbat: kim birinchi kelsa. 100 ta mahsulotli arava oldinda bo'lsa, bitta non ko'targan odam uzoq kutadi.
- **SJF** — "10 tagacha mahsulot" kassasi: qisqa ishlar oldin. O'rtacha kutish kamayadi.
- **Round Robin** — bolalar arg'imchoqda navbat bilan: har biriga 1 daqiqa, keyin navbat oxiriga. Hech kim abadiy kutmaydi.
- **Ustuvorlik** — tez yordam mashinasi: navbatsiz o'tadi.
- **Linux CFS** — "eng kam o'ynagan bola keyingi uchadi": har bir jarayonning olgan vaqti hisoblab boriladi, eng kam olgani tanlanadi.

Misol uchun uchta ish: **A (100 ms), B (10 ms), C (10 ms)**, hammasi 0-vaqtda keldi. Algoritmlarni dastur bilan hisoblaymiz:

```c
/* algoritmlar.c - FIFO, SJF va Round Robin: aylanish va javob vaqti */
#include <stdio.h>

#define N 3
static const char *nom[N] = { "A", "B", "C" };
static const int davom[N] = { 100, 10, 10 };

/* ketma-ket bajarish tartibi bo'yicha (FIFO yoki SJF) */
static void ketma_ket(const char *nomi, const int *tartib)
{
    int vaqt = 0, aylanish = 0, javob = 0;
    for (int k = 0; k < N; k++) {
        int i = tartib[k];
        javob += vaqt;                          /* birinchi marta ishga tushgan vaqt (kelish = 0) */
        vaqt += davom[i];
        aylanish += vaqt;                       /* tugash vaqti */
    }
    printf("%-12s aylanish %6.2f   javob %5.2f\n", nomi, (double)aylanish / N, (double)javob / N);
}

static void round_robin(int kvant)
{
    int qoldi[N], boshlandi[N], tugadi[N], vaqt = 0, qolgan = N;
    for (int i = 0; i < N; i++) {
        qoldi[i] = davom[i];
        boshlandi[i] = -1;
    }
    while (qolgan > 0)
        for (int i = 0; i < N; i++) {
            if (qoldi[i] == 0)
                continue;
            if (boshlandi[i] < 0)
                boshlandi[i] = vaqt;
            int ish = qoldi[i] < kvant ? qoldi[i] : kvant;
            vaqt += ish;
            qoldi[i] -= ish;
            if (qoldi[i] == 0) {
                tugadi[i] = vaqt;
                qolgan--;
            }
        }
    int aylanish = 0, javob = 0;
    for (int i = 0; i < N; i++) {
        aylanish += tugadi[i];
        javob += boshlandi[i];
    }
    char t[32];
    snprintf(t, sizeof(t), "RR (kvant %d)", kvant);
    printf("%-12s aylanish %6.2f   javob %5.2f\n", t, (double)aylanish / N, (double)javob / N);
}

int main(void)
{
    int fifo[N] = { 0, 1, 2 };                  /* A, B, C */
    int sjf[N] = { 1, 2, 0 };                   /* B, C, A: eng qisqasi birinchi */
    printf("A=%d, B=%d, C=%d (hammasi 0 da keldi)\n", davom[0], davom[1], davom[2]);
    ketma_ket("FIFO", fifo);
    ketma_ket("SJF", sjf);
    round_robin(10);
    round_robin(1);
    (void)nom;
    return 0;
}
```

```console
$ gcc -Wall -Wextra algoritmlar.c -o algoritmlar
$ ./algoritmlar
A=100, B=10, C=10 (hammasi 0 da keldi)
FIFO         aylanish 110.00   javob 70.00
SJF          aylanish  50.00   javob 10.00
RR (kvant 10) aylanish  56.67   javob 10.00
RR (kvant 1) aylanish  59.67   javob  1.00
```

**Bu dastur nima qiladi (umumiy):** bir xil uch ishni to'rt xil usulda rejalashtiradi va har birining **o'rtacha aylanish** va **o'rtacha javob** vaqtini hisoblaydi. Shunda "algoritm tanlovi natijaga qanday ta'sir qiladi" raqamda ko'rinadi.

**Natijani o'qish:**

| Algoritm | Aylanish | Javob | Xulosa |
|---|---|---|---|
| **FIFO** (A, B, C) | 110 | 70 | **konvoy effekti**: qisqa B va C uzun A ni kutdi |
| **SJF** (B, C, A) | 50 | 10 | aylanish uchun **optimal** (hammasi 0 da kelganda) |
| **RR** (kvant 10) | 56.67 | 10 | javob vaqti ajoyib (hamma tez boshlaydi), aylanish SJF'dan yomon |
| **RR** (kvant 1) | 59.67 | 1 | javob eng yaxshi, lekin amalda ko'p kontekst almashish narxi |

### FIFO (birinchi kelgan — birinchi)

Tartib A, B, C: tugash 100, 110, 120 → o'rtacha aylanish **110**. Muammo — **konvoy effekti**: qisqa ishlar uzun ishning orqasida kutib qoladi (do'kondagi navbatda to'la aravacha orqasida).

### SJF (eng qisqa ish birinchi)

B, C, A: tugash 10, 20, 120 → o'rtacha **50**. Hamma ishlar bir vaqtda kelsa — aylanish vaqti uchun **optimal**. Muammo: ish uzunligini oldindan bilish kerak; A yolg'iz boshlagan bo'lsa, B va C keyin
kelsa — baribir kutadi (preemption yo'q).

### STCF (eng kam qolgani birinchi, preemptive SJF)

Yangi ish kelganda — agar uning qolgan vaqti joriy ishnikidan kam bo'lsa, joriy ish to'xtatiladi. Aylanish vaqti uchun eng yaxshi. Lekin javob vaqti yomon va uzun ishlar "och qolishi" mumkin.

### Round Robin (RR)

Har bir ish kvant (masalan 10 ms) ishlaydi, keyin navbat oxiriga. Javob vaqti ajoyib (hamma tez boshlaydi), aylanish vaqti yomon (hamma ish cho'ziladi). Kvant tanlash — muvozanat: kichik kvant — yaxshi javob,
lekin kontekst almashish xarajati ko'p. 35-mashqda aynan RR'ni simulyatsiya qildingiz; MyOS'ning scheduler'i ham RR (kvant = 5 tik = 50 ms).

### I/O ni hisobga olish

Ish I/O kutganda (disk, klaviatura) — u bloklanadi va CPU boshqasiga beriladi. Interaktiv jarayonlar kichik CPU bo'laklari + ko'p kutishdan iborat; ularni tez-tez ishga tushirish kerak.

### MLFQ (ko'p darajali qayta aloqali navbat)

Uzunlikni bilmasdan SJF'ga yaqinlashish — **xatti-harakatdan o'rganish**:

1. Bir nechta ustuvorlik navbati. Yuqoridagisi doim birinchi ishlaydi; bir darajada — RR.
2. Yangi ish — eng yuqori darajada.
3. Ish o'z darajasidagi vaqt ulushini to'liq ishlatsa — bir daraja **pastga** (demak, u uzun, CPU talab).
4. Kvant tugashidan oldin CPU'ni o'zi bo'shatsa (I/O) — darajasida qoladi (interaktiv).
5. Vaqti-vaqti bilan hammani **yuqoriga ko'tarish** (priority boost) — past darajadagilar och qolmasin va xatti-harakati o'zgargan ishlar qayta baholansin.
6. Har bir darajada **jami** ishlatilgan vaqt hisoblanadi (aks holda dastur kvant tugashidan oldin ataylab I/O qilib, yuqorida qolishi mumkin — "o'yin").

Windows, eski Linux, macOS scheduler'lari MLFQ oilasidan.

### Adolatli ulush: lotereya va stride

Har bir jarayonga "chipta" beriladi; har kvantda tasodifiy chipta tortiladi — ko'p chiptali ko'proq yutadi. Oddiy, holatsiz, ulushlarni aniq nazorat qiladi. Deterministik varianti — **stride scheduling**.

### Linux CFS (Completely Fair Scheduler) — g'oya

Har bir jarayonning **virtual ishlash vaqti** (`vruntime`) bor. Scheduler doim eng **kichik** `vruntime` li jarayonni tanlaydi. Ishlagan jarayonning `vruntime` i oshadi — og'irligiga (nice qiymatiga) qarab:
yuqori ustuvorlik — sekinroq oshadi. Natija: hamma adolatli ulush oladi, uxlab qaytgan interaktiv jarayon kichik `vruntime` bilan tez tanlanadi. Eng kichikni tez topish uchun jarayonlar
**qizil-qora daraxtda** saqlanadi (O(log n), 28-bob). (2023-yildan Linux EEVDF algoritmiga o'tdi — CFS'ning takomillashgan davomi.)

Linux'da ustuvorlikni **nice** qiymati bilan o'zgartirasiz (−20 … 19; katta son = "yaxshi, boshqalarga yo'l beraman" = past ustuvorlik):

```console
$ ps -o ni= -p $$
  0
$ nice -n 10 sh -c 'ps -o ni= -p $$'
 10
```

Oddiy shell'ning nice qiymati `0`; `nice -n 10` bilan boshlangan jarayonniki — `10`: scheduler uning `vruntime` ini tezroq oshiradi, shuning uchun u kamroq CPU oladi.

## 23.7. Ko'p yadroli rejalashtirish

- **Bitta umumiy navbat:** oddiy, lekin qulf talashuvi va kesh yaqinligi (affinity) yo'qoladi — jarayon har safar boshqa yadroda, keshi sovuq.
- **Har bir yadroga o'z navbati** (Linux, MyOS SMP): tez, kesh yaqinligi saqlanadi, lekin **yuklamani muvozanatlash** kerak — bo'sh yadro band yadrodan ish "o'g'irlaydi" (work stealing).

## Hayotdan misol va to'liq dastur

**Rejalashtiruvchi simulyatori.** Uchta jarayon bir vaqtda keldi. FCFS va Round Robin'da kim qachon ishlashini va qancha kutishini hisoblaymiz.

```c
/* rejalashtiruvchi.c - FCFS va Round Robin: vaqt chizig'i va o'rtacha kutish */
#include <stdio.h>

#define J 3

static const char *nom[J] = { "A", "B", "C" };
static const int kerak[J] = { 8, 2, 4 };        /* har biriga kerakli CPU vaqti */

static void fcfs(void)
{
    int vaqt = 0, jami_kutish = 0;
    printf("FCFS:        |");
    for (int i = 0; i < J; i++) {
        jami_kutish += vaqt;                    /* shu paytgacha navbatda turdi */
        for (int t = 0; t < kerak[i]; t++)
            printf("%s", nom[i]);
        vaqt += kerak[i];
    }
    printf("|  o'rtacha kutish: %.2f\n", (double)jami_kutish / J);
}

static void round_robin(int kvant)
{
    int qoldi[J], tugadi[J], vaqt = 0, tugaganlar = 0;
    for (int i = 0; i < J; i++)
        qoldi[i] = kerak[i];
    printf("RR (kvant %d): |", kvant);
    while (tugaganlar < J) {
        for (int i = 0; i < J; i++) {
            if (qoldi[i] == 0)
                continue;
            int ish = qoldi[i] < kvant ? qoldi[i] : kvant;
            for (int t = 0; t < ish; t++)
                printf("%s", nom[i]);
            vaqt += ish;
            qoldi[i] -= ish;
            if (qoldi[i] == 0) {
                tugadi[i] = vaqt;
                tugaganlar++;
            }
        }
    }
    int jami_kutish = 0;
    for (int i = 0; i < J; i++)
        jami_kutish += tugadi[i] - kerak[i];    /* kutish = tugash vaqti - ishlagan vaqti */
    printf("|  o'rtacha kutish: %.2f\n", (double)jami_kutish / J);
}

int main(void)
{
    printf("Jarayonlar: A=%d, B=%d, C=%d vaqt birligi\n", kerak[0], kerak[1], kerak[2]);
    fcfs();
    round_robin(2);
    round_robin(1);
    return 0;
}
```

```console
$ gcc -Wall -Wextra rejalashtiruvchi.c -o rejalashtiruvchi
$ ./rejalashtiruvchi
Jarayonlar: A=8, B=2, C=4 vaqt birligi
FCFS:        |AAAAAAAABBCCCC|  o'rtacha kutish: 6.00
RR (kvant 2): |AABBCCAACCAAAA|  o'rtacha kutish: 4.67
RR (kvant 1): |ABCABCACACAAAA|  o'rtacha kutish: 5.00
```

**Bu dastur nima qiladi (umumiy):** uch jarayon (A=8, B=2, C=4 vaqt birligi) bir vaqtda keldi. Dastur ikki usulda **vaqt chizig'ini** chizadi (har harf — bir vaqt birligida kim ishlagani) va o'rtacha kutishni hisoblaydi.

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `kerak[J]` | har jarayonga kerak CPU vaqti |
| `fcfs()` | tartib bilan: har jarayon oldingilarning vaqtini **kutadi** (`jami_kutish += vaqt`) |
| `round_robin(kvant)` | har biriga ko'pi bilan `kvant` vaqt; tugamagan bo'lsa — navbat oxiriga; `kutish = tugash − ishlagan vaqti` |

Har bir harf — bitta vaqt birligida kim ishlagani. FCFS'da qisqa B uzun A ni kutib qoldi. Round Robin'da B tez tugadi va o'rtacha kutish kamaydi. Lekin kvant juda kichik bo'lsa (1), almashishlar ko'payadi —
haqiqiy tizimda har bir almashish vaqt oladi.

**Sinab ko'ring:** `kerak` massivini `{ 2, 4, 8 }` qiling (qisqasi birinchi) — FCFS natijasi qanday o'zgaradi? Bu aynan SJF. `kerak` ga to'rtinchi jarayon qo'shing (`J` ni ham o'zgartiring).

## Bob xulosasi (yodlash uchun)

1. **Jarayon** — "virtual CPU": o'z xotirasi + CPU holati + OS resurslari; illyuziya **vaqtni bo'lish** (kvant ~1–10 ms) bilan yaratiladi. **Mexanizm** (qanday almashtirish) va **siyosat** (qaysi birini) farqlanadi.
2. Holatlar: **tayyor → ishlayapti → bloklangan** (`S`), to'xtatilgan (`T`), **zombi** (`Z`). Yadroda har jarayon uchun **PCB** (`task_struct` / `struct process`).
3. **Kontekst almashish:** registrlarni saqlash → boshqasini tiklash (+ `CR3`); ~mikrosekundlar + kesh/TLB sovishi — shuning uchun kvant kichik bo'lmaydi.
4. Metrikalar: **aylanish** (`tugash − kelish`), **javob** (`birinchi ishga tushish − kelish`), adolat, o'tkazuvchanlik — ular bir-biriga **zid**.
5. Algoritmlar: FIFO (konvoy), SJF/STCF (aylanish uchun optimal), RR (javob uchun yaxshi), MLFQ (xatti-harakatdan o'rganadi), CFS (eng kichik `vruntime`, qizil-qora daraxt), lotereya/stride.

## Savol-javob

**Nega Round Robin'da aylanish vaqti yomon?**
Hamma ish bir-birini to'xtatib, cho'ziladi: hamma bir vaqtda tugashga yaqinlashadi (23.6 jadvalida RR aylanish 56.67 > SJF 50).

**Zombi nima uchun kerak?**
Bola tugagach, uning **chiqish kodi** otaga yetkazilishi kerak. Ota `wait` qilguncha yadro bu ma'lumotni PCB'da saqlaydi — shu zombi (14.4).

**Nega kvant 1 ms emas, 1 mikrosekund emas?**
Juda kichik kvant — vaqtning ko'pi kontekst almashishga ketadi (23.4 da o'lchagan narx); juda katta — interaktivlik yomonlashadi. 1–10 ms — muvozanat.

## O'zingizni tekshiring

1. Mexanizm va siyosat farqi?
2. A=100, B=10, C=10 (hammasi 0 da): FIFO va SJF o'rtacha aylanish vaqtlari?
3. RR'da kvantni juda kichik qilsak nima bo'ladi?
4. MLFQ uzunlikni bilmasdan qisqa ishlarni qanday "taniydi"?
5. CFS qaysi jarayonni tanlaydi va nega qizil-qora daraxt?

<details><summary>Javoblar</summary>

1. Mexanizm — qanday almashtirish (kontekst saqlash/tiklash); siyosat — qaysi jarayonni tanlash.
2. FIFO: (100+110+120)/3 = 110; SJF: (10+20+120)/3 = 50.
3. Kontekst almashish va kesh/TLB sovushi xarajati foydali ishdan oshib ketadi.
4. Kvantni to'liq ishlatgan ish pastga tushadi; CPU'ni tez bo'shatganlar yuqorida qoladi.
5. Eng kichik vruntime'li; daraxt eng kichikni O(log n) da topish va qo'shish/o'chirishni ta'minlaydi.
</details>

## Mashq

- **35** (Round Robin simulyatsiyasi).
- Qo'shimcha: 35-mashq kodingizga SJF va STCF variantlarini qo'shing va 23.6-dagi misol bilan tekshiring.
- MyOS: `kernel/proc/process.c` dagi `sched_tick` va `scheduler_loop` ni o'qib, kvant qanday hisoblanishini toping; `spin` dasturidan ikkitasini ishga tushirib `ps` bilan kuzating.

<!-- loyiha:boshi -->
## Loyiha: Round Robin rejalashtiruvchi simulyatori

**Maqsad:** yadro rejalashtiruvchisining ichki mantig'i: kim qachon ishlaydi, kim qancha kutadi. Metrikalar
(aylanish vaqti, kutish, javob) rejalashtirish algoritmlarini taqqoslashning asosiy o'lchovi (23.5–23.6).
**Bobdan ishlatiladi:** tayyor navbat, vaqt kvanti, jarayon holati, metrikalar.

**Talab:** 4 ta jarayon: kelish vaqti va kerakli CPU vaqti berilgan. Kvant = 3. Har vaqt birligida kim ishlagani
(Gantt chizig'i) va har jarayon uchun metrikalar chiqarilsin.

| Jarayon | Kelish | CPU vaqti |
|---|---|---|
| A | 0 | 5 |
| B | 1 | 3 |
| C | 2 | 8 |
| D | 3 | 2 |

**Qoida:** kvant tugaganda jarayon navbat **oxiriga** qaytadi. Agar aynan shu paytda yangi jarayon kelsa, u **oldin**
navbatga qo'yiladi (kvant davomida kelganlar hammasi avval, keyin tugatgan jarayonning o'zi).
**Metrikalar:** `aylanish = tugash − kelish`; `kutish = aylanish − CPU vaqti`; `javob = birinchi marta ishlagan vaqt − kelish`.

```c
/* rr.c - Round Robin simulyatori */
#include <stdio.h>

#define N 4
#define KVANT 3

int main(void)
{
    const char nom[N] = { 'A', 'B', 'C', 'D' };
    int kelish[N] = { 0, 1, 2, 3 };
    int davomiylik[N] = { 5, 3, 8, 2 };
    int qoldi[N], tugash[N], boshlash[N];
    char gantt[128];
    int navbat[64], bosh = 0, oxir = 0;
    int t = 0, tugaganlar = 0, uz = 0;

    for (int i = 0; i < N; i++) {
        qoldi[i] = davomiylik[i];
        boshlash[i] = -1;
    }
    for (int i = 0; i < N; i++)                 /* t = 0 da kelganlar */
        if (kelish[i] == 0)
            navbat[oxir++] = i;

    while (tugaganlar < N) {
        if (bosh == oxir) {                     /* navbat bo'sh: CPU bo'sh turadi */
            gantt[uz++] = '.';
            t++;
            for (int i = 0; i < N; i++)
                if (kelish[i] == t)
                    navbat[oxir++] = i;
            continue;
        }
        int p = navbat[bosh++];
        if (boshlash[p] < 0)
            boshlash[p] = t;
        int ish = qoldi[p] < KVANT ? qoldi[p] : KVANT;
        for (int k = 0; k < ish; k++) {
            gantt[uz++] = nom[p];
            t++;
            for (int i = 0; i < N; i++)         /* ish davomida kelganlar avval navbatga */
                if (kelish[i] == t)
                    navbat[oxir++] = i;
        }
        qoldi[p] -= ish;
        if (qoldi[p] > 0) {
            navbat[oxir++] = p;                 /* keyin o'zi */
        } else {
            tugash[p] = t;
            tugaganlar++;
        }
    }
    gantt[uz] = '\0';

    printf("Gantt: %s\n", gantt);
    printf("Nom  Kelish  CPU  Tugash  Aylanish  Kutish  Javob\n");
    double jami_aylanish = 0, jami_kutish = 0;
    for (int i = 0; i < N; i++) {
        int aylanish = tugash[i] - kelish[i];
        int kutish = aylanish - davomiylik[i];
        int javob = boshlash[i] - kelish[i];
        printf("%c    %6d  %3d  %6d  %8d  %6d  %5d\n", nom[i], kelish[i], davomiylik[i], tugash[i], aylanish, kutish, javob);
        jami_aylanish += aylanish;
        jami_kutish += kutish;
    }
    printf("O'rtacha aylanish: %.2f, o'rtacha kutish: %.2f\n", jami_aylanish / N, jami_kutish / N);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address,undefined rr.c -o rr
$ ./rr
Gantt: AAABBBCCCDDAACCCCC
Nom  Kelish  CPU  Tugash  Aylanish  Kutish  Javob
A         0    5      13        13       8      0
B         1    3       6         5       2      2
C         2    8      18        16       8      4
D         3    2      11         8       6      6
O'rtacha aylanish: 10.50, o'rtacha kutish: 6.00
```

Gantt chizig'ida har harf — bir vaqt birligi. Kvant kichik bo'lsa — javob tez (hamma tezda birinchi marta ishlaydi), lekin
kontekst almashishlar ko'p; katta bo'lsa — FCFS ga yaqinlashadi. `KVANT` ni 1 va 100 qiling va farqni ko'ring.

**Kengaytiring:** kontekst almashishlar sonini sanang (Gantt'da qo'shni harflar o'zgargan joylar). Jarayonlarni `A(0,4) B(20,2)` qiling — `.` (bo'sh CPU) qanday ko'rinadi?

## Mustaqil loyiha: SRTF (eng qisqa qolgan vaqt birinchi) ★★★

**Vazifa:** **preemptiv** SJF (SRTF) ni simulyatsiya qiling. Har vaqt birligida, kelib bo'lgan va tugamagan jarayonlar ichidan
**qolgan vaqti eng kam** bo'lganini ishlating. Yangi jarayon kelganda hozirgisidan qisqa bo'lsa — uni **to'xtatib** o'tadi. Fayl: `srtf.c`.

| Jarayon | Kelish | CPU vaqti |
|---|---|---|
| A | 0 | 7 |
| B | 2 | 4 |
| C | 4 | 1 |
| D | 5 | 4 |

**Tanlash qoidasi (aniq):** har vaqt birligi boshida, tayyorlar orasidan `qoldi` eng kichigini oling. Teng bo'lsa:
1) o'tgan birlikda ishlagan jarayonni saqlang (keraksiz almashish bo'lmasin); 2) u ham bo'lmasa — **kelishi oldinroq**, keyin ro'yxatdagi tartib.
Hech kim tayyor bo'lmasa — Gantt'da `.`.

**Chiqish:** Gantt chizig'i va jadval (formati RR loyihasidagi kabi, sarlavha va ustunlar o'sha yerdagidek),
keyin o'rtacha aylanish va kutish (`%.2f`). Bundan tashqari **kontekst almashishlar soni** (Gantt'da qo'shni harflar farq qilgan
joylar, `.` ham hisobga kiradi). Sarlavhalar va ustun kengliklari kutilgan natijada.

**Kutilgan natija** (`darslik/loyihalar/23_srtf/kutilgan.txt`):

```text
Gantt: AABBCBBDDDDAAAAA
Nom  Kelish  CPU  Tugash  Aylanish  Kutish  Javob
A         0    7      16        16       9      0
B         2    4       7         5       1      0
C         4    1       5         1       0      0
D         5    4      11         6       2      2
O'rtacha aylanish: 7.00, o'rtacha kutish: 3.00
Kontekst almashishlar: 5
```

**Maslahat** (yechim emas):
- Sikl `t = 0, 1, 2, ...` — har `t` da bitta birlik: kim ishlaydi? `qoldi[p]--`. `qoldi` 0 bo'lsa `tugash[p] = t + 1`.
- Tanlashda `oldingi` (o'tgan birlikdagi jarayon) o'zgaruvchisini saqlang va tenglikda uni afzal ko'ring.
- Kutish `aylanish − CPU vaqti`, javob `birinchi ishlagan vaqt − kelish`.
- Natijani qo'lda tekshiring: A(0,7) dan boshlanadi; `t=2` da B(4) kelganda A da 5 qolgan — B ustun, A to'xtaydi.
- SRTF o'rtacha kutishni **minimal** qiladi (23.6) — qo'lda hisoblang va taqqoslang.

**Tekshirish:**

```bash
gcc -Wall -Wextra -g -fsanitize=address,undefined srtf.c -o dastur && ./dastur | diff - ~/C_loyha/darslik/loyihalar/23_srtf/kutilgan.txt && echo "TO'G'RI"
```
<!-- loyiha:oxiri -->

Keyingi bob: [24-bob. Virtual xotira nazariyasi](24-virtual-xotira.md)
