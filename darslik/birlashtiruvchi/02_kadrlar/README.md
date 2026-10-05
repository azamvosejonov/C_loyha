# Birlashtiruvchi loyiha 2: Kadrlar va maosh tizimi (katta loyiha, 40 fayl)

> **Bu nima:** haqiqiy ish loyihasiga o'xshash **katta dastur**: papkalar (`include/`, `src/`, `data/`, `tests/`), 10 ta modul, `Makefile`, avtomatik testlar, ma'lumot fayllari. Dasturning **ko'p qismi tayyor**; **14 ta funksiyani o'zingiz yozasiz** (`TODO`). Qolgan kodni **o'qib** o'rganasiz — haqiqiy loyihada ish ko'pincha ana shunday: **boshqa birovning kodini tushunib, o'z qismingni qo'shish**.
> **Oldindan nima kerak:** hozirning o'zida 2-bobgacha bo'lsangiz — **boshlay olasiz** (★ belgili TODO lar). Qolganlari keyingi boblar o'tilgach: loyiha **siz bilan birga o'sadi**.
> **Vaqt:** bir necha kun. Shoshilmang: bitta TODO — bitta o'tirish.
> **Yechim repoda yo'q.** Har funksiya uchun **aniq shart, misollar, maslahat** va **avtomatik test** bor.

## Dastur nima qiladi

Kichik kompaniyaning **xodimlar ro'yxati** (`data/xodimlar.txt`) va **ish davomati** (`data/davomat.txt` — kim qachon kelib-ketgan) o'qiladi. Shulardan har xodim uchun **oylik maosh** hisoblanadi: asosiy ish haqi, ustama (ortiqcha ish 1.5 barobar), toifa bonusi, **progressiv soliq** (daromad oshgan sari soliq foizi oshadi), kasaba badali. Natija — jadval, **maosh varaqasi**, reyting, umumiy hisobot.

```text
$ ./bin/kadrlar royxat
ID    Ism        Toifa             Brutto         Soliq        Qo'lga
1042  Aziza      Mutaxassis     686263.72      82351.65     597049.43
2087  Bobur      Boshlovchi     472500.00      56700.00     411075.00
...
```

Buyruqlar: `royxat`, `varaqa ID`, `top`, `jami`, `izla ISM`.

## Fayllar xaritasi

```text
02_kadrlar/
├── Makefile                 yig'ish va test buyruqlari (make, make test, make tekshir)
├── data/
│   ├── xodimlar.txt         xodimlar (ba'zi qatorlar ATAYLAB noto'g'ri - dastur ularni rad etishi kerak)
│   └── davomat.txt          kirish/chiqish vaqtlari (ba'zilari ataylab noto'g'ri)
├── include/                 SARLAVHALAR (.h): har modulning "shartnomasi" - qanday funksiyalar bor, nima qiladi
│   ├── config.h             umumiy doimiylar (chegaralar, foizlar)
│   ├── pul.h  vaqt.h  matn.h  soliq.h  xodim.h  ombor.h  davomat.h  hisobot.h  buyruq.h
├── src/                     REALIZATSIYA (.c)
│   ├── main.c               kirish nuqtasi: fayllarni yuklaydi -> buyruqni bajaradi
│   ├── buyruq.c             buyruqlarni ajratadi (royxat, varaqa...)
│   ├── pul.c  vaqt.c  matn.c  soliq.c      pastki daraja: pul, vaqt, matn, soliq
│   ├── xodim.c              bitta xodimning maoshi, varaqa chiqarish
│   ├── ombor.c              xodimlar ombori (massiv), qidirish, saralash, fayldan o'qish
│   ├── davomat.c            davomat faylini o'qib, xodimlarga ish daqiqalarini qo'shadi
│   └── hisobot.c            jadval va umumiy hisobotlar
└── tests/                   avtomatik testlar
    ├── test.h               kichik test tizimi
    ├── test_pul.c  test_vaqt.c  test_matn.c  test_soliq.c  test_ombor.c  test_hisobot.c
    ├── butun.sh             tayyor dasturni butun holda sinaydi
    └── kutilgan_*.txt       kutilgan natijalar
```

**Modullar bir-biriga qanday bog'langan** (o'q — "chaqiradi"):

```text
main ──> ombor_yukla ─────────────┐        (xodimlar.txt)
   │                              v
   ├──> davomat_yukla ──> ombor_top, ish_daqiqalari
   │
   └──> buyruq_bajar ──> hisobot / xodim_varaqa
                                │
                    xodim_hisobla  ──>  pul_vaqt_haqi, foiz, toifa_bonusi, soliq_hisobla
```

## Chalkashib ketmaslik uchun: qanday o'qish kerak

Katta kodni **boshidan oxirigacha** o'qimang! Quyidagi tartibda:

1. **`main.c`** — dastur nimadan boshlanadi? (30 qator)
2. **`include/*.h`** — har sarlavha = modulning "menyusi". Funksiya **nima qilishi** (izohda), **qanday ishlashi** emas.
3. **`src/xodim.c`** dagi `xodim_hisobla` — maosh qanday yig'iladi? Bu yerda hamma modul **birlashadi**.
4. Faqat keyin — o'zingiz yozadigan funksiya joylashgan faylni oching.

**Qoida:** bitta funksiyani yozayotganda **boshqa fayllarni o'zgartirmang** (tayyor kod to'g'ri ishlaydi). TODO joyda `/* TODO ... */` izohi turibdi; o'sha izohni o'chirib, o'rniga kod yozasiz.

## Ishlash tartibi

```bash
cd ~/C_loyha/darslik/birlashtiruvchi/02_kadrlar
make                 # dasturni yig'adi (TODO lar bo'sh bo'lsa ham yig'iladi, lekin natijalar noto'g'ri)
make test            # har TODO ning holatini ko'rsatadi
make run ARGS="royxat"
make tekshir         # hamma test o'tganda: butun dasturni kutilgan natija bilan solishtiradi
```

`make test` natijasi (boshlang'ich holat):

```text
  [XATO] T1 foiz: 1/6 to'g'ri
  [XATO] T2 daqiqaga: 1/4 to'g'ri
  ...
```

Har yozgan funksiyangizdan keyin `make test` — `[XATO]` `[ OK ]` ga aylanadi. Xato bo'lsa, qaysi chaqiruv **nima qaytarganini va nima kutilganini** ko'rsatadi:

```text
        foiz(1050, 1)  ->  olindi 10, kutilgan 11   (tests/test_pul.c:9)
```

## TODO jadvali — o'zingiz yozadigan 14 funksiya

| # | Daraja | Fayl → funksiya | Nima | Qaysi boblar |
|---|---|---|---|---|
| T1 | ★ | `src/pul.c` → `foiz` | summaning p foizi, tiyinga yaxlitlab | 2 |
| T2 | ★ | `src/vaqt.c` → `daqiqaga` | `0930` → 570 daqiqa | 2 |
| T3 | ★★ | `src/vaqt.c` → `ish_daqiqalari` | sof ish vaqti; noto'g'ri vaqt → -1 | 4 (`if`) |
| T4 | ★ | `src/vaqt.c` → `soat_yuzdan` | daqiqa → soatning yuzdan bir ulushi | 2 |
| T5 | ★ | `src/matn.c` → `id_nazorat` | ID ning nazorat raqami | 2 |
| T6 | ★★★ | `src/matn.c` → `nom_uzunligi` | satr uzunligi (o'zingiz, `strlen`siz) | 6 |
| T7 | ★★★ | `src/matn.c` → `nom_teng` | ikki satr tengmi (`strcmp`siz) | 6 |
| T8 | ★★ | `src/soliq.c` → `toifa_bonusi` | toifa → bonus foizi | 4 (`switch`) |
| T9 | ★★ | `src/soliq.c` → `soliq_hisobla` | progressiv soliq (3 bo'lak) | 4 |
| T10 | ★★★ | `src/ombor.c` → `ombor_top` | ID bo'yicha topish, **ko'rsatkich** qaytarish | 6, 7 |
| T11 | ★★★ | `src/ombor.c` → `ombor_ism_bilan_top` | ism bo'yicha topish | 6, 7 |
| T12 | ★★★ | `src/ombor.c` → `ombor_saralash_tarif` | tarif bo'yicha saralash (o'zingiz) | 6, 9 |
| T13 | ★★★★ | `src/ombor.c` → `ombor_yukla` | fayldan xodimlarni o'qish va tekshirish | 12 |
| T14 | ★★★ | `src/hisobot.c` → `hisobot_jami` | barcha xodimlar bo'yicha yig'indi | 6, 9 |

**Hozir nimani boshlash mumkin (2-bobgacha):** T1, T2, T4, T5. Aytib o'tilgan boblar o'tilgach keyingilariga o'ting. Hamma TODO tayyor bo'lmaguncha ham dastur **yig'iladi va ishlaydi** — faqat to'liq emas (masalan, T13 yozilmaguncha xodimlar o'qilmaydi).

**Tartib tavsiyasi:** T1 → T2 → T4 → T5 (hozir) → T8 → T3 → T9 → T6 → T7 → T10 → T11 → T14 → T12 → T13.

Har TODO uchun **talab, misollar va maslahat** tegishli `.h` faylida (`include/`) yozilgan — **avval o'qing**.

## Maslahat va tuzoqlar

- **T1:** butun sonlarda yaxlitlash — bo'lishdan **oldin** bo'luvchining yarmini qo'shish. Natijani kasrga o'tkazmang. `int64_t` bilan ko'paytirganda `int` toshishi mumkin.
- **T2/T4:** `/` va `%` — butun bo'lish va qoldiq (2-bob). `570 * 100 / 60` va `570 / 60 * 100` — turli natija! Nega?
- **T3:** avval **noto'g'ri** holatlarni (`-1`) tekshiring, keyin hisoblang. `daqiqaga()` ni qayta ishlating — **kodni takrorlamang**.
- **T5:** raqamni ajratish: `id % 10` — oxirgi raqam, `id / 10` — oxirgisini tashlab yuboradi.
- **T9:** soliq **bo'laklarga** bo'linadi: 1-chegaragacha, chegaralar orasi, 2-chegaradan oshgan qism. Har bo'lak uchun `foiz()` ni **alohida** chaqiring.
- **T10:** qaytariladigan narsa — `&o->a[i]` (massiv elementining **manzili**), `o->a[i]` ning nusxasi **emas**. Test shuni tekshiradi (`p == &o.a[1]`).
- **T12:** struct ni to'liq almashtirasiz (`tmp = a[j]; a[j] = a[j+1]; a[j+1] = tmp;`), faqat tarifni emas.
- **T13:** `davomat.c` dagi `davomat_yukla` — **xuddi shunday** vazifa; uni namuna qilib oling.

## Tugatgach

`make tekshir` → **BUTUN DASTUR TO'G'RI**. Bu — haqiqiy kichik loyihaning hamma qismi: modullar, sarlavhalar, test, Makefile, fayl o'qish, massiv va struct, ko'rsatkichlar.

**Ixtiyoriy kengaytirishlar (yechimsiz):**
1. Yangi buyruq: `./kadrlar eng_kam` — eng kam maosh oluvchini chiqaring (`buyruq.c` ga yangi `else if`, hisob uchun yangi funksiya).
2. `data/xodimlar.txt` ga o'zingizning xodimlaringizni qo'shing, yangi **toifa** (5) qo'shing (`config.h`, `toifa_bonusi`, `toifa_nomi` — **nechta fayl** o'zgardi?).
3. `make test` ga **yangi test** qo'shing (`tests/test_...c`) — o'z funksiyangiz uchun chegaraviy holatlar.
4. `gcc -fsanitize=address,undefined` bilan yig'ing: hamma narsa tozami?
