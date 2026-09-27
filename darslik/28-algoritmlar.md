# 28-bob. Algoritmlar va ma'lumotlar tuzilmalari — yadro dasturchisi uchun

> **Bu bobdan keyin:** murakkablikni (O-belgi) baholay olasiz, asosiy tuzilmalarni (massiv, ro'yxat,
> stek, navbat, xesh jadval, ikkilik qidiruv daraxti, muvozanatli daraxtlar, heap, trie/radix, graflar)
> va algoritmlarni (saralash, ikkilik qidiruv, BFS/DFS, sikl topish, topologik tartib) C'da yozishni,
> ularning har biri yadroda qayerda ishlatilishini va suhbatdagi masalalarga qanday tayyorlanishni bilasiz.
> Mashqlar: 13, 16, 17, 19, 21, 22, 32, 42, 46, 47, 48.

> **To'liq ishlaydigan misol:** [misollar/28_algoritmlar.c](misollar/28_algoritmlar.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Hayotdan misollar

**O-belgi — ish qanchalik tez o'sadi (28.1).**
- **O(1)** — garderob: raqamcha bo'yicha paltoni darhol topasiz. 10 ta palto bo'lsa ham, 10 000 ta bo'lsa ham.
- **O(log n)** — lug'atda so'z qidirish: o'rtasini ochasiz, "S" dan oldinmi-keyinmi, yana yarmini...
  Million so'zli lug'atda ~20 qadam.
- **O(n)** — tartibsiz qog'ozlar uyumida bitta hujjatni qidirish: har birini ko'rib chiqasiz.
- **O(n log n)** — kartalarni yaxshi usul bilan saralash.
- **O(n²)** — sinfdagi har bir o'quvchi har biri bilan qo'l berib ko'rishishi: 30 kishi — 435 ta qo'l
  berish, 300 kishi — 44 850 ta. Odam 10 barobar ko'payganda ish 100 barobar ko'payadi.

**Stek va navbat (28.2).** Stek — likopchalar ustuni (oxirgi qo'yilgan birinchi olinadi). Navbat —
do'kon kassasi (birinchi kelgan birinchi ketadi).

**Bog'langan ro'yxat — xazina qidirish o'yini (28.2).** Har bir yozuvda keyingi yozuv qayerdaligi
yozilgan. 5-yozuvga yetish uchun 1, 2, 3, 4 dan o'tish kerak — to'g'ridan-to'g'ri borib bo'lmaydi.
Lekin o'rtaga yangi yozuv qo'shish oson: faqat ikkita "keyingisi" ni o'zgartirasiz.

**Xesh jadval — garderob (28.3).** Paltoni ilmoqqa ilasiz, raqamcha olasiz. Xesh funksiya — kalitdan
(ismdan) ilmoq raqamini hisoblaydi. Ikki kalit bitta ilmoqqa tushsa (to'qnashuv) — bir ilmoqqa ikkita
palto ilinadi (zanjir).

**Daraxt — oila shajarasi yoki tashkilot tuzilmasi (28.6).** Direktor → bo'lim boshliqlari → xodimlar.
Ikkilik qidiruv daraxti — har bir tugunda "kichiklar chapda, kattalar o'ngda" qoidasi. Muvozanatlangan
daraxt (AVL, qizil-qora) shoxlar bir tomonga og'ib ketmasligini ta'minlaydi.

**Graf va BFS — metro xaritasi (28.7).** Bekatlar — tugunlar, yo'llar — qirralar. "Eng kam bekat bilan
qanday yetaman?" savoliga BFS javob beradi: avval 1 bekatlik masofadagilarni, keyin 2 bekatlik... —
suvga tashlangan toshdan tarqalgan to'lqinlar kabi.

### To'liq dastur: Toshkent metrosi

Metro tarmog'ining bir qismi (soddalashtirilgan). BFS bilan eng kam bekatli yo'lni topamiz.

```c
/* metro.c - graf, navbat va BFS: eng qisqa yo'l (bekatlar soni bo'yicha) */
#include <stdio.h>
#include <string.h>

#define B 10
static const char *bekat[B] = {
    "Chilonzor", "Novza", "Mustaqillik maydoni", "Amir Temur xiyoboni", "Paxtakor",
    "Alisher Navoiy", "Toshkent", "Oybek", "Kosmonavtlar", "Yunus Rajabiy",
};
static int yol[B][B];                           /* qo'shnilik matritsasi: 1 - to'g'ridan-to'g'ri yo'l */

static void ula(int a, int b) { yol[a][b] = yol[b][a] = 1; }

static void eng_qisqa(int dan, int ga)
{
    int oldingi[B], navbat[B], bosh = 0, oxir = 0;
    memset(oldingi, -1, sizeof(oldingi));
    oldingi[dan] = dan;
    navbat[oxir++] = dan;
    while (bosh < oxir) {                       /* navbat bo'sh bo'lmaguncha */
        int u = navbat[bosh++];
        for (int v = 0; v < B; v++)
            if (yol[u][v] && oldingi[v] == -1) {
                oldingi[v] = u;                 /* v ga u orqali keldik */
                navbat[oxir++] = v;
            }
    }

    int yol_teskari[B], n = 0;                  /* oxiridan boshiga qarab tiklash */
    for (int v = ga; v != dan; v = oldingi[v])
        yol_teskari[n++] = v;
    yol_teskari[n++] = dan;
    printf("%s -> %s: %d bekat\n  ", bekat[dan], bekat[ga], n - 1);
    for (int i = n - 1; i >= 0; i--)
        printf("%s%s", bekat[yol_teskari[i]], i ? " -> " : "\n");
}

int main(void)
{
    ula(0, 1);                                  /* Chilonzor liniyasi */
    ula(1, 2);
    ula(2, 3);
    ula(3, 4);
    ula(4, 5);                                  /* Paxtakor <-> Alisher Navoiy (o'tish) */
    ula(5, 7);                                  /* Alisher Navoiy -> Oybek */
    ula(3, 9);                                  /* Amir Temur <-> Yunus Rajabiy (o'tish) */
    ula(7, 8);                                  /* Oybek -> Kosmonavtlar */
    ula(8, 6);                                  /* Kosmonavtlar -> Toshkent */
    ula(9, 6);                                  /* Yunus Rajabiy -> Toshkent */

    eng_qisqa(0, 6);                            /* Chilonzor -> Toshkent vokzali */
    eng_qisqa(0, 7);
    eng_qisqa(4, 9);
    return 0;
}
```

```console
$ gcc -Wall -Wextra metro.c -o metro
$ ./metro
Chilonzor -> Toshkent: 5 bekat
  Chilonzor -> Novza -> Mustaqillik maydoni -> Amir Temur xiyoboni -> Yunus Rajabiy -> Toshkent
Chilonzor -> Oybek: 6 bekat
  Chilonzor -> Novza -> Mustaqillik maydoni -> Amir Temur xiyoboni -> Paxtakor -> Alisher Navoiy -> Oybek
Paxtakor -> Yunus Rajabiy: 2 bekat
  Paxtakor -> Amir Temur xiyoboni -> Yunus Rajabiy
```

BFS Chilonzordan Toshkentgacha ikki yo'lni ko'rdi: Yunus Rajabiy orqali va Oybek orqali. Birinchisi
qisqaroq bo'lgani uchun u tanlandi — BFS bekatlarni masofasi bo'yicha qatlam-qatlam ochadi.

**Sinab ko'ring:** `ula(9, 6);` ni o'chiring — endi yo'l qaysi bekatlar orqali o'tadi? Bekatlar soni
`B` ni 1000 ga oshirsak, qo'shnilik matritsasi necha bayt egallaydi? Nega katta graflarda qo'shnilar
ro'yxati ishlatiladi?

## 28.1. Murakkablik: O-belgi

Algoritm tezligini soniyalarda emas, **kirish o'lchami n o'sganda ish qanday o'sishi** bilan baholaymiz:

| O(...) | Nomi | n = 1 000 000 da taxminiy qadamlar | Misol |
|---|---|---|---|
| O(1) | o'zgarmas | 1 | massiv indeksi, xesh jadval (o'rtacha) |
| O(log n) | logarifmik | 20 | ikkilik qidiruv, muvozanatli daraxt |
| O(n) | chiziqli | 10⁶ | massivni aylanish |
| O(n log n) | | 2·10⁷ | yaxshi saralash |
| O(n²) | kvadratik | 10¹² — soatlar! | ichma-ich sikl, oddiy saralash |
| O(2ⁿ) | eksponensial | koinot yoshidan uzoq | hamma to'plamostilarni sanash |

Qoidalar: o'zgarmas ko'paytuvchilar tashlanadi (O(3n) = O(n)), eng tez o'sadigan had qoladi
(O(n² + n) = O(n²)). **Eng yomon holat** va **o'rtacha holat** alohida baholanadi (xesh jadval:
o'rtacha O(1), eng yomon O(n)). **Amortizatsiyalangan:** dinamik massivda `push` ba'zan O(n) (qayta
ajratish), lekin o'rtacha O(1) — chunki hajm 2 barobar oshadi (13-mashq).

**Yadroda nega muhim:** scheduler har millisekundda ishlaydi, page fault ishlovchisi sekundiga minglab
marta. O(n) algoritm 10 jarayonda sezilmaydi, 10 000 da tizimni to'xtatadi. Linux'da O(1) scheduler
va keyin CFS (O(log n)) aynan shu sababdan yaratilgan.

**Xotira murakkabligi** ham hisobga olinadi — yadro stekida rekursiya chuqurligi O(n) bo'lishi mumkin emas.

## 28.2. Chiziqli tuzilmalar

| Tuzilma | Kirish | Qidirish | Qo'shish/o'chirish | Yadroda |
|---|---|---|---|---|
| Massiv | O(1) | O(n) (saralangan: O(log n)) | oxiriga O(1)*, o'rtaga O(n) | jadvallar, fd jadvali |
| Bog'langan ro'yxat | O(n) | O(n) | tugun ma'lum bo'lsa O(1) | `list_head` hamma joyda (23-mashq) |
| Stek (LIFO) | tepa O(1) | — | O(1) | chaqiruvlar steki, 24-mashq |
| Navbat (FIFO) | boshi O(1) | — | O(1) | scheduler, halqa bufer (22), I/O navbatlari |

Massiv va ro'yxat tanlovida kesh (21-bob) ko'pincha nazariyadan muhimroq: kichik n da massiv doim yutadi.

## 28.3. Xesh jadval

Kalitni xesh funksiya bilan songa aylantirib, massiv indeksini topish. To'qnashuvlar (ikki kalit bitta
indeksga) ikki usulda hal qilinadi:
- **Zanjir** (chaining): har bir chelakda ro'yxat (17-mashq). Linux: `hlist_head` massivlari.
- **Ochiq adreslash**: to'qnashsa, keyingi bo'sh joyni qidirish (chiziqli, kvadratik). Keshga mosroq.

**Yuklama koeffitsienti** α = elementlar / chelaklar. α oshsa — sekinlashadi; α > 1 (zanjirda) yoki
α > 0.7 (ochiq adreslashda) bo'lganda — **2 barobar kattalashtirib, qayta joylashtirish** (rehash).

Yaxshi xesh funksiya bitlarni yaxshi aralashtiradi: FNV-1a, MurmurHash, Linux'da `jhash`, `hash_long`
(oltin nisbat bilan ko'paytirish). **Xavfsizlik:** hujumchi ataylab bir chelakka tushadigan kalitlar
yuborsa — O(n) ga tushadi (DoS). Yechim — tasodifiy "tuz" (seed).

Yadroda: PID → jarayon, inode keshi, dentry keshi, tarmoq ulanishlari jadvali.

## 28.4. Ikkilik qidiruv

Saralangan massivda O(log n):

```c
/* x ni topsa indeksini, topmasa -1 */
long ikkilik_qidiruv(const int *a, size_t n, int x)
{
    size_t chap = 0, ong = n;               /* [chap, ong) oralig'ida qidiramiz */
    while (chap < ong) {
        size_t orta = chap + (ong - chap) / 2;   /* (chap+ong)/2 TOSHISHI mumkin! */
        if (a[orta] < x)
            chap = orta + 1;
        else if (a[orta] > x)
            ong = orta;
        else
            return (long)orta;
    }
    return -1;
}
```

`(chap + ong) / 2` — mashhur xato: Java kutubxonasida 9 yil yashagan (2006-yilda topilgan). Oraliqni
`[chap, ong)` (yarim ochiq) ko'rinishida saqlash chegaraviy xatolarni kamaytiradi.

Ikkilik qidiruv g'oyasi faqat massiv uchun emas: `git bisect` (19-bob), "eng kichik yaroqli qiymat"
(javob bo'yicha ikkilik qidiruv).

## 28.5. Saralash

| Algoritm | O'rtacha | Eng yomon | Qo'shimcha xotira | Barqaror | Qachon |
|---|---|---|---|---|---|
| Qo'yish (insertion) | O(n²) | O(n²) | O(1) | ha | n < ~20, deyarli saralangan |
| Birlashtirish (merge) | O(n log n) | O(n log n) | O(n) | ha | kafolat kerak, ro'yxatlar (19-mashq) |
| Tez (quick) | O(n log n) | O(n²) | O(log n) | yo'q | amalda eng tez |
| Heap | O(n log n) | O(n log n) | O(1) | yo'q | kafolat + xotirasiz — **Linux `sort()` shuni ishlatadi** |
| Sanash (counting/radix) | O(n + k) | | O(k) | ha | kichik butun kalitlar |

"Barqaror" — teng kalitlarning nisbiy tartibi saqlanadi. Taqqoslashga asoslangan saralash O(n log n) dan
tez bo'lishi mumkin emas (isbotlangan).

**Quicksort g'oyasi:** tayanch (pivot) tanlash, undan kichiklarni chapga, kattalarni o'ngga, ikkala
qismni rekursiv saralash. Yomon pivot (masalan, saralangan massivda birinchi element) → O(n²).
Yechim: o'rtadagi/tasodifiy/"uchtaning medianasi". Standart kutubxonalar gibrid: introsort (quick →
chuqurlik oshsa heap → kichik qismlarda insertion).

**Yadroda nega heapsort:** eng yomon holat kafolati va qo'shimcha xotira yo'q; rekursiya ham yo'q (stek kichik!).

## 28.6. Daraxtlar

### Ikkilik qidiruv daraxti (BST)

Har bir tugunda: chap qism daraxtidagilar kichik, o'ngdagilar katta. Qidirish/qo'shish/o'chirish —
O(balandlik). Muammo: saralangan tartibda qo'shilsa — daraxt ro'yxatga aylanadi, balandlik n.

### Muvozanatli daraxtlar

Balandlikni O(log n) da ushlab turish uchun qo'shish/o'chirishdan keyin **aylantirishlar** (rotations):

```text
     y                x
    / \   o'ngga     / \
   x   C  ------>   A   y
  / \     <------      / \
 A   B    chapga      B   C
```

Aylantirish tartibni (A < x < B < y < C) saqlaydi, lekin balandliklarni o'zgartiradi.

- **AVL** — har bir tugunda chap va o'ng balandliklar farqi ≤ 1. Qat'iy muvozanat — qidirish tez.
  47-mashqda yozasiz.
- **Qizil-qora daraxt** — har bir tugun qizil yoki qora; qoidalar eng uzun yo'l eng qisqasidan 2 barobardan
  oshmasligini kafolatlaydi. Kamroq aylantirish — qo'shish/o'chirish tezroq. **Linux'da eng ko'p ishlatiladigan
  daraxt** (`lib/rbtree.c`): CFS scheduler, yuqori aniqlikdagi taymerlar (hrtimer), ext4 ekstentlar
  keshi, uzoq yillar virtual xotira hududlari (VMA) ham (6.1 versiyadan ular uchun B-daraxtga o'xshash
  "maple tree" ishlatiladi).
- **B-daraxt** — har bir tugunda ko'p kalit (masalan, 100 ta); balandlik juda kichik. Disk uchun ideal
  (bitta tugun = bitta disk bloki): ma'lumotlar bazalari, btrfs, XFS, ext4 papka indeksi (htree).

### Heap (uyum) va ustuvorlik navbati

To'liq ikkilik daraxt, massivda saqlanadi: `i` ning bolalari `2i+1`, `2i+2`, otasi `(i-1)/2`.
Min-heap: har bir ota bolalaridan kichik → eng kichik element doim `a[0]`.
`push` — oxiriga qo'yib, "yuqoriga suzdirish"; `pop` — oxirgisini tepaga qo'yib, "pastga cho'ktirish".
Ikkalasi O(log n). Yadroda: taymerlar (eng yaqin muddatli), scheduler'lar, Dijkstra algoritmi. 46-mashq.

### Trie va radix daraxt

Kalitni **qismlarga** (belgilar, bitlar guruhlari) bo'lib, daraxt bo'ylab tushish. Kalit uzunligi bo'yicha
O(k), taqqoslashsiz. **Sahifa jadvali — aslida radix daraxt!** (9 bitlik 4 daraja, 31-mashq.) Linux'da
`xarray` (sahifa keshi: fayldagi siljish → sahifa), IP marshrutlash jadvallari (eng uzun prefiks).

## 28.7. Graflar

Graf — tugunlar va qirralar. Tasvirlash: **qo'shnilik ro'yxati** (siyrak graflar uchun — odatda shu) yoki
**qo'shnilik matritsasi** (zich graflar, O(1) tekshiruv).

**BFS** (kenglik bo'yicha) — navbat bilan; eng qisqa yo'l (vaznsiz graflarda).
**DFS** (chuqurlik bo'yicha) — stek/rekursiya bilan; sikl topish, topologik tartib.

**Sikl topish (DFS, uch rang):** oq — ko'rilmagan, kulrang — hozir stekda, qora — tugagan. Kulrang
tugunga qaytuvchi qirra = **sikl**. Bu — deadlock aniqlashning asosi: "kutish grafi"da (A → B: "A B ushlab
turgan resursni kutyapti") sikl = deadlock (26-bob, 48-mashq). Linux `lockdep` qulflar tartibi grafida shuni qiladi.

**Topologik tartib:** bog'liqliklarni hisobga olib tartiblash (A B dan oldin bo'lishi kerak). `make`
aynan shuni qiladi — qaysi faylni avval yig'ish kerak (11-bob). Yadroda — modullarni ishga tushirish tartibi.

## 28.8. Bitli hiylalar va maxsus tuzilmalar

- **Bitmap** (21-mashq) — to'plam elementlari uchun 1 bit.
- **Bloom filtri** — "albatta yo'q" yoki "ehtimol bor" deydigan ixcham to'plam.
- **Halqa bufer** (22-mashq) — qulfsiz ham qilish mumkin (bitta yozuvchi + bitta o'quvchi).
- **Buddy** (32), **slab** (33) — xotira allocator'lari o'zi ham algoritmlar.

## 28.9. Usullar

- **Ikki ko'rsatkich** — saralangan massivda juft topish, teskari aylantirish (10-mashq), takrorlarni o'chirish (05).
- **Siljuvchi oyna** — ketma-ket qismlarda yig'indi/maksimum.
- **Bo'l va hukmronlik qil** — merge sort, ikkilik qidiruv.
- **Dinamik dasturlash** — takrorlanadigan qism masalalar natijasini saqlash (yadroda kam, suhbatlarda ko'p).
- **Ochko'z (greedy)** — har qadamda eng yaxshi mahalliy tanlov (SJF scheduler — 23-bob).

## 28.10. Suhbatlarga tayyorgarlik (keyinchalik)

Katta kompaniyalar tizim dasturchisini ham algoritm masalalari bilan tekshiradi. Usul:
1. **Masalani o'z so'zingiz bilan qayta ayting**, misollar va chegaraviy holatlarni so'rang (bo'sh kirish, bitta element, takrorlar, toshish).
2. Avval **oddiy (sekin) yechimni** ayting, murakkabligini baholang.
3. Keyin yaxshilang: qaysi tuzilma yordam beradi? (Qidiruv ko'p — xesh; tartib kerak — daraxt/heap;
   "eng yaqin"/"eng kichik" — heap; yo'llar — BFS/DFS.)
4. Kodni yozing, keyin **qo'lda misol bilan yurib chiqing**.

Tayyorgarlik uchun: ushbu bobdagi hamma tuzilmani C'da noldan yozing (mashqlar 13, 16, 17, 19, 21, 22, 42,
46, 47, 48) — keyin masalalar to'plamlaridan (ko'pchiligi ingliz tilida, lekin masala shartlari qisqa —
31-bob lug'ati yordam beradi) kuniga 1–2 ta.

## 28.11. O'zingizni tekshiring

1. Dinamik massivda `push` nega amortizatsiyalangan O(1)?
2. `(chap + ong) / 2` nima uchun xavfli?
3. Nega Linux'da heapsort, quicksort emas?
4. Sahifa jadvali qaysi tuzilma turiga kiradi?
5. Deadlock'ni graf yordamida qanday topasiz?

<details><summary>Javoblar</summary>

1. Hajm 2 barobar oshadi: n ta push uchun jami nusxalash 1+2+4+...+n < 2n — har biriga o'rtacha O(1).
2. Katta indekslarda yig'indi toshadi; `chap + (ong - chap) / 2` xavfsiz.
3. Eng yomon holat O(n log n) kafolati, qo'shimcha xotira va rekursiya yo'q (kichik yadro steki).
4. Radix daraxt (trie): manzil bitlari guruhlari bo'yicha tushish.
5. Kutish grafini qurib, DFS bilan sikl qidirish (kulrang tugunga qaytish).
</details>

## 28.12. Mashqlar

- **42** (LRU kesh: xesh + ikki tomonlama ro'yxat), **46** (min-heap), **47** (AVL daraxt),
  **48** (deadlock: grafda sikl topish).
- Takrorlash: **13, 16, 17, 19, 21, 22, 32**.

Keyingi bob: [29-bob. Debug va profiling vositalari](29-debug-vositalari.md)
