# 23-bob. Jarayonlar va rejalashtirish (scheduling)

> **Bu bobdan keyin:** jarayon abstraksiyasini, uning holatlarini va yadrodagi tuzilmasini (PCB),
> kontekst almashishni, rejalashtirish algoritmlarini (FIFO, SJF, STCF, Round Robin, MLFQ, adolatli
> ulush, Linux CFS) va ularni baholash metrikalarini bilasiz. (Mavzu "Operating Systems: Three Easy
> Pieces" kitobining virtualizatsiya qismidan.) Mashqlar: 35.

> **To'liq ishlaydigan misol:** [misollar/23_jarayonlar.c](misollar/23_jarayonlar.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

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

Keyingi bob: [24-bob. Virtual xotira nazariyasi](24-virtual-xotira.md)
