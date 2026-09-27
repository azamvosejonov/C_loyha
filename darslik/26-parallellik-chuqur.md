# 26-bob. Parallellik chuqur: qulflarni qurish, semaforlar, klassik masalalar, deadlock

> **Bu bobdan keyin:** qulflar noldan qanday quriladi (Peterson, test-and-set, CAS, ticket), shart
> o'zgaruvchilari va semaforlarni, klassik masalalarni (ishlab chiqaruvchi-iste'molchi, o'quvchilar-yozuvchilar,
> ovqatlanayotgan faylasuflar), deadlock'ning 4 shartini va real dasturlardagi parallellik xatolari
> turlarini bilasiz. (OSTEP "Concurrency" qismi.) 15-bobning davomi. Mashqlar: 29, 34, 38, 44, 45, 48.

> **To'liq ishlaydigan misol:** [misollar/26_faylasuflar.c](misollar/26_faylasuflar.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

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

Keyingi bob: [27-bob. Qurilmalar va fayl tizimlari](27-fayl-tizimlari.md)
