# 23-bob. Jarayonlar va rejalashtirish (scheduling)

> **Bu bobdan keyin:** jarayon abstraksiyasini, uning holatlarini va yadrodagi tuzilmasini (PCB),
> kontekst almashishni, rejalashtirish algoritmlarini (FIFO, SJF, STCF, Round Robin, MLFQ, adolatli
> ulush, Linux CFS) va ularni baholash metrikalarini bilasiz. (Mavzu "Operating Systems: Three Easy
> Pieces" kitobining virtualizatsiya qismidan.) Mashqlar: 35.

> **To'liq ishlaydigan misol:** [misollar/23_jarayonlar.c](misollar/23_jarayonlar.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Hayotdan misollar

**Jarayon — kompyuter klubidagi o'yinchi (23.1).** Klubda bitta kompyuter (bitta CPU yadrosi) va o'nta
o'yinchi bor. Har biri navbat bilan bir necha daqiqa o'ynaydi. Almashishlar juda tez bo'lsa, har biri
"kompyuter faqat meniki" deb o'ylaydi. Jarayon — aynan shu: har bir dastur o'zini butun protsessorga
ega deb his qiladi.

**Jarayon holatlari — poliklinikadagi bemor (23.2).**
- **Tayyor (ready)** — navbatda o'tiribdi, shifokor bo'shashini kutyapti.
- **Ishlayapti (running)** — shifokor qabulida.
- **Bloklangan (blocked)** — tahlil topshirdi, natijani kutyapti. Uni chaqirishdan foyda yo'q — natija
  kelmaguncha u baribir hech narsa qila olmaydi. Natija kelsa — yana navbatga (ready) o'tadi.

**PCB — bemorning kartasi (23.3).** Shifokor bemorni qayta chaqirganda, uning kartasini ochadi: oxirgi
tashxis, dorilar. Yadro ham har bir jarayonning "kartasini" saqlaydi: registrlar, ochiq fayllar,
xotira xaritasi, holati.

**Kontekst almashish — o'qituvchining sinfdan sinfga o'tishi (23.4).** O'qituvchi 5-A dan 5-B ga
o'tishdan oldin jurnalga qayerda to'xtaganini yozib qo'yadi. Qaytib kelganda aynan shu joydan davom
etadi. Yadro ham jarayonni almashtirishdan oldin uning barcha registrlarini saqlaydi va keyingisinikini
tiklaydi. Bu tez, lekin bepul emas — o'qituvchi har 10 soniyada sinf almashtirsa, dars o'tishga vaqt qolmaydi.

**Rejalashtirish algoritmlari (23.6).**
- **FCFS** — oddiy navbat: kim birinchi kelsa. 100 ta mahsulotli arava oldinda bo'lsa, bitta non
  ko'targan odam uzoq kutadi.
- **SJF** — "10 tagacha mahsulot" kassasi: qisqa ishlar oldin. O'rtacha kutish kamayadi.
- **Round Robin** — bolalar arg'imchoqda navbat bilan: har biriga 1 daqiqa, keyin navbat oxiriga.
  Hech kim abadiy kutmaydi.
- **Ustuvorlik** — tez yordam mashinasi: navbatsiz o'tadi.
- **Linux CFS** — "eng kam o'ynagan bola keyingi uchadi": har bir jarayonning olgan vaqti hisoblab
  boriladi, eng kam olgani tanlanadi.

### To'liq dastur: rejalashtiruvchi simulyatori

Uchta jarayon bir vaqtda keldi. FCFS va Round Robin'da kim qachon ishlashini va qancha kutishini hisoblaymiz.

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

Har bir harf — bitta vaqt birligida kim ishlagani. FCFS'da qisqa B uzun A ni kutib qoldi. Round Robin'da
B tez tugadi va o'rtacha kutish kamaydi. Lekin kvant juda kichik bo'lsa (1), almashishlar ko'payadi —
haqiqiy tizimda har bir almashish vaqt oladi.

**Sinab ko'ring:** `kerak` massivini `{ 2, 4, 8 }` qiling (qisqasi birinchi) — FCFS natijasi qanday
o'zgaradi? Bu aynan SJF. `kerak` ga to'rtinchi jarayon qo'shing (`J` ni ham o'zgartiring).

## 23.1. Jarayon — "virtual CPU"

Kompyuterda 4–8 ta yadro bor, lekin yuzlab dastur "bir vaqtda" ishlaydi. OS har bir dasturga
**o'zining alohida CPU'si va xotirasi bordek** illyuziya beradi. Bu illyuziya — **jarayon** (process):

- **manzil maydoni** — o'z xotirasi (kod, ma'lumot, heap, stek) — 24-bob;
- **CPU holati** — registrlar, `rip`, `rsp`, `rflags`;
- **OS resurslari** — ochiq fayllar, joriy papka, signal ishlovchilari, ota-bola munosabatlari.

Illyuziya **vaqtni bo'lish** (time sharing) bilan yaratiladi: har bir jarayon qisqa vaqt (kvant, ~1–10 ms)
ishlaydi, keyin OS uni to'xtatib, boshqasini ishga tushiradi.

Ikki qatlam (OSTEP'ning muhim g'oyasi):
- **mexanizm** — *qanday* almashtirish (kontekst almashish, uzilishlar) — past daraja;
- **siyosat** — *qaysi* jarayonni ishga tushirish (rejalashtirish algoritmi) — yuqori daraja.

## 23.2. Jarayon holatlari

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

MyOS'da: `enum proc_state` (`kernel/proc/process.h`): `PROC_EMBRYO`, `PROC_READY`, `PROC_RUNNING`,
`PROC_BLOCKED` (uxlayapti), `PROC_STOPPED` (Ctrl-Z), `PROC_ZOMBIE`, `PROC_DEAD`.

## 23.3. PCB — yadrodagi jarayon tuzilmasi

Har bir jarayon uchun yadroda tuzilma ("Process Control Block"): Linux'da `task_struct` (~10 KB!),
MyOS'da `struct process`. Unda: pid, holat, saqlangan registrlar (kontekst), yadro steki, manzil
maydoni (`mm`), ochiq fayllar jadvali, ota, bolalar, signal ma'lumotlari, scheduler ma'lumotlari.

## 23.4. Kontekst almashish — mexanizm

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

Kontekst almashish **qimmat**: ~1–5 mikrosekund, lekin asosiy yashirin narx — kesh va TLB'ning "sovushi".
Shuning uchun kvant juda kichik bo'lishi mumkin emas.

MyOS'da: `kernel/arch/isr.asm`, `kernel/proc/switch.asm` (~30 qator — o'qing!), `kernel/proc/process.c`
(`sched_tick`, `scheduler_loop`), `docs/06-jarayonlar.md`.

## 23.5. Rejalashtirish metrikalari

- **Aylanish vaqti** (turnaround): `tugash − kelish` — ish qancha vaqtda bajarildi (fon vazifalari uchun muhim).
- **Javob vaqti** (response): `birinchi_ishga_tushish − kelish` — interaktivlik (klaviatura, ekran uchun muhim).
- **Adolat** — hamma o'z ulushini oladimi.
- **O'tkazuvchanlik** (throughput) — vaqt birligida bajarilgan ishlar.

Bular bir-biriga zid: aylanish vaqti uchun eng yaxshi algoritm javob vaqti uchun yomon va aksincha.

## 23.6. Algoritmlar

Misol uchun uchta ish: A (100 ms), B (10 ms), C (10 ms), hammasi 0-vaqtda keldi.

### FIFO (birinchi kelgan — birinchi)

Tartib A, B, C: tugash 100, 110, 120 → o'rtacha aylanish **110**. Muammo — **konvoy effekti**:
qisqa ishlar uzun ishning orqasida kutib qoladi (do'kondagi navbatda to'la aravacha orqasida).

### SJF (eng qisqa ish birinchi)

B, C, A: tugash 10, 20, 120 → o'rtacha **50**. Hamma ishlar bir vaqtda kelsa — aylanish vaqti uchun
**optimal**. Muammo: ish uzunligini oldindan bilish kerak; A yolg'iz boshlagan bo'lsa, B va C keyin
kelsa — baribir kutadi (preemption yo'q).

### STCF (eng kam qolgani birinchi, preemptive SJF)

Yangi ish kelganda — agar uning qolgan vaqti joriy ishnikidan kam bo'lsa, joriy ish to'xtatiladi.
Aylanish vaqti uchun eng yaxshi. Lekin javob vaqti yomon va uzun ishlar "och qolishi" mumkin.

### Round Robin (RR)

Har bir ish kvant (masalan 10 ms) ishlaydi, keyin navbat oxiriga. Javob vaqti ajoyib (hamma tez
boshlaydi), aylanish vaqti yomon (hamma ish cho'ziladi). Kvant tanlash — muvozanat: kichik kvant —
yaxshi javob, lekin kontekst almashish xarajati ko'p. 35-mashqda aynan RR'ni simulyatsiya qildingiz;
MyOS'ning scheduler'i ham RR (kvant = 5 tik = 50 ms).

### I/O ni hisobga olish

Ish I/O kutganda (disk, klaviatura) — u bloklanadi va CPU boshqasiga beriladi. Interaktiv jarayonlar
kichik CPU bo'laklari + ko'p kutishdan iborat; ularni tez-tez ishga tushirish kerak.

### MLFQ (ko'p darajali qayta aloqali navbat)

Uzunlikni bilmasdan SJF'ga yaqinlashish — **xatti-harakatdan o'rganish**:

1. Bir nechta ustuvorlik navbati. Yuqoridagisi doim birinchi ishlaydi; bir darajada — RR.
2. Yangi ish — eng yuqori darajada.
3. Ish o'z darajasidagi vaqt ulushini to'liq ishlatsa — bir daraja **pastga** (demak, u uzun, CPU talab).
4. Kvant tugashidan oldin CPU'ni o'zi bo'shatsa (I/O) — darajasida qoladi (interaktiv).
5. Vaqti-vaqti bilan hammani **yuqoriga ko'tarish** (priority boost) — past darajadagilar och qolmasin
   va xatti-harakati o'zgargan ishlar qayta baholansin.
6. Har bir darajada **jami** ishlatilgan vaqt hisoblanadi (aks holda dastur kvant tugashidan oldin
   ataylab I/O qilib, yuqorida qolishi mumkin — "o'yin").

Windows, eski Linux, macOS scheduler'lari MLFQ oilasidan.

### Adolatli ulush: lotereya va stride

Har bir jarayonga "chipta" beriladi; har kvantda tasodifiy chipta tortiladi — ko'p chiptali ko'proq yutadi.
Oddiy, holatsiz, ulushlarni aniq nazorat qiladi. Deterministik varianti — **stride scheduling**.

### Linux CFS (Completely Fair Scheduler) — g'oya

Har bir jarayonning **virtual ishlash vaqti** (`vruntime`) bor. Scheduler doim eng **kichik** `vruntime`
li jarayonni tanlaydi. Ishlagan jarayonning `vruntime` i oshadi — og'irligiga (nice qiymatiga) qarab:
yuqori ustuvorlik — sekinroq oshadi. Natija: hamma adolatli ulush oladi, uxlab qaytgan interaktiv
jarayon kichik `vruntime` bilan tez tanlanadi.
Eng kichikni tez topish uchun jarayonlar **qizil-qora daraxtda** saqlanadi (O(log n), 28-bob).
(2023-yildan Linux EEVDF algoritmiga o'tdi — CFS'ning takomillashgan davomi.)

## 23.7. Ko'p yadroli rejalashtirish

- **Bitta umumiy navbat:** oddiy, lekin qulf talashuvi va kesh yaqinligi (affinity) yo'qoladi —
  jarayon har safar boshqa yadroda, keshi sovuq.
- **Har bir yadroga o'z navbati** (Linux, MyOS SMP): tez, kesh yaqinligi saqlanadi, lekin **yuklamani
  muvozanatlash** kerak — bo'sh yadro band yadrodan ish "o'g'irlaydi" (work stealing).

## 23.8. O'zingizni tekshiring

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

## 23.9. Mashqlar

- **35** (Round Robin simulyatsiyasi).
- Qo'shimcha: 35-mashq kodingizga SJF va STCF variantlarini qo'shing va 23.6-dagi misol bilan tekshiring.
- MyOS: `kernel/proc/process.c` dagi `sched_tick` va `scheduler_loop` ni o'qib, kvant qanday
  hisoblanishini toping; `spin` dasturidan ikkitasini ishga tushirib `ps` bilan kuzating.

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
