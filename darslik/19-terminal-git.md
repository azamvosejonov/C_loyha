# 19-bob. Linux terminali va Git — dasturchining ish stoli

> **II qism boshlanishi.** I qism (00–18) — C tili. II qism (19–31) — kompyuter tizimlari va operatsion tizimlar nazariyasi: odatda ingliz tilidagi bir nechta kitobdan o'rganiladigan bilimlar
> shu yerda o'zbek tilida jamlangan. Bu bob — hamma narsaning poydevori: terminalda ishlash va Git. Agar terminalni allaqachon yaxshi bilsangiz, 19.6 dan (Git) boshlang.
>
> **Bu bobda nima o'rganasiz:** terminalda papkalar bo'ylab yurishni; fayllar bilan ishlashni; kod ichidan qidirishni (`grep`); buyruqlarni quvur (`|`) bilan ulashni; fayl ruxsatlarini;
> Git bilan "saqlash nuqtalari" yaratish va ularga qaytishni.
> **Oldindan nima kerak:** 0-bob (o'rnatish).   **Vaqt:** 5–6 soat.

> **To'liq ishlaydigan misol:** [misollar/19_terminal.sh](misollar/19_terminal.sh) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Bu bob nima haqida?

Dasturchi kuniga yuzlab marta terminalga yozadi: "bu papkaga kir", "bu so'zni hamma fayldan top", "o'zgarishni saqla". Bu bob — shu kundalik ishning **to'liq lug'ati**. Hamma buyruq bu yerda
**haqiqatan bajarilgan** (kichik "sandbox" papkada), shuning uchun natijani o'z kompyuteringizda ko'rganingiz bilan solishtirishingiz mumkin.

**Hayotdan misol: SMS bilan boshqarish.** Grafik oynalar — televizor pultidagi tugmalar: qulay, lekin faqat ishlab chiqaruvchi qo'ygan tugmalar bor. Terminal — aniq buyruqlar yozish: "fayllar ichidan
'xato' so'zi bor qatorlarni top, sanab, eng ko'pini ko'rsat". Buni tugmalar bilan qilib bo'lmaydi. Serverlarda va yadro ishida esa oyna umuman yo'q — faqat terminal.

| Televizor pulti | Terminal |
|---|---|
| tayyor tugmalar | o'zingiz buyruq yozasiz |
| bitta bosish — bitta ish | buyruqlarni zanjirga ulash (`\|`) mumkin |
| takrorlash qiyin | skript qilib saqlab, istalgan vaqt qayta ishga tushirish mumkin |

## 19.1. Nega terminal

Grafik oyna (sichqoncha bilan bosish) qulay, lekin dasturchi uchun terminal kuchliroq:

- har bir amalni **aniq yozib**, qayta takrorlash mumkin (skript);
- kichik dasturlarni **quvur** (`|`) bilan birlashtirib, murakkab ish qilish mumkin;
- serverlar, yadrolar, o'rnatilgan tizimlar — deyarli hammasida faqat terminal bor;
- MyOS'ning o'zida ham shell bor — siz uni yozgansiz (40-mashq).

Terminalda ishlaydigan dastur — **shell** (Linux'da odatda `bash`). U siz yozgan qatorni o'qiydi, so'zlarga bo'ladi, dasturni topib ishga tushiradi (`fork` + `exec`, 14-bob) va natijani ko'rsatadi.

## 19.2. Fayl tizimi bo'ylab yurish

**Hayotdan misol: shkaf, tortma va papka.** `/` — butun shkaf, `/home/ali` — Alining tortmasi, `/home/ali/loyiha` — tortmadagi papka. `pwd` — "men qaysi tortmadaman?", `cd` — boshqa tortmaga o'tish,
`ls` — "bu tortmada nima bor?". `..` — bir daraja yuqori.

Mashq qilish uchun kichik "yadro"-ga o'xshash papka daraxti yaratamiz (haqiqiy fayllarga tegmaymiz):

```console
$ mkdir -p ish/kernel/mm ish/kernel/fs ish/user
$ touch ish/kernel/mm/pmm.c ish/kernel/mm/slab.c ish/kernel/fs/pipe.c ish/user/sh.c ish/README
$ ls -R ish
ish:
README
kernel
user

ish/kernel:
fs
mm

ish/kernel/fs:
pipe.c

ish/kernel/mm:
pmm.c
slab.c

ish/user:
sh.c
```

**Qismlar:** `mkdir -p` — ichma-ich papkalarni birdaniga yaratish (`-p`: oraliqlari yo'q bo'lsa ham). `touch` — bo'sh fayl yaratish. `ls -R` — papkani ichidagilari bilan (rekursiv) ko'rsatish.

```bash
pwd                     # qaysi papkadaman (print working directory)
ls                      # papkada nima bor
ls -la                  # batafsil: ruxsatlar, hajm, sana, yashirin fayllar (. bilan boshlanadigan)
cd kernel/mm            # papkaga kirish
cd ..                   # bir daraja yuqoriga
cd ~                    # uy papkasiga (/home/ism)
cd -                    # oldingi papkaga qaytish
```

Yurish tajribasi (bir qatorda, chunki har buyruq alohida ishlaydi; manzilning boshini `...` bilan qisqartiramiz):

```console
$ cd ish/kernel/mm; pwd | sed 's#.*/ish#.../ish#'; cd ..; pwd | sed 's#.*/ish#.../ish#'; cd ../user; pwd | sed 's#.*/ish#.../ish#'; ls
.../ish/kernel/mm
.../ish/kernel
.../ish/user
sh.c
```

**Nima ko'rdik:** `cd ish/kernel/mm` → shu papkadamiz; `cd ..` → bir daraja yuqori (`kernel`); `cd ../user` → yuqoriga chiqib, `user` ga kirdik. `ls` — `user` ichidagi fayllarni ko'rsatdi.

| Belgi | Ma'nosi |
|---|---|
| `/` | ildiz papka (yoki yo'l ajratgichi) |
| `.` | joriy papka |
| `..` | ota papka |
| `~` | uy papkangiz |
| `*` | istalgan belgilar (`*.c` — hamma C fayllar) |
| `?` | bitta istalgan belgi |

**Mutlaq yo'l** `/` bilan boshlanadi (`/home/ali/C_loyha`), **nisbiy yo'l** — joriy papkadan (`kernel/mm/pmm.c`).

```console
$ ls ish/kernel/*/*.c
ish/kernel/fs/pipe.c
ish/kernel/mm/pmm.c
ish/kernel/mm/slab.c
$ ls ish/kernel/mm/?lab.c
ish/kernel/mm/slab.c
```

`*` — "istalgan belgilar": `ish/kernel/*/*.c` = "kernel ichidagi istalgan papkadagi istalgan `.c` fayl". `?` — aynan bitta belgi: `?lab.c` → `slab.c`. Shu belgilarni (`*`, `?`) **shell** ochadi,
buyruqning o'zi ularni ko'rmaydi (u tayyor fayl ro'yxatini oladi).

## 19.3. Fayllar bilan ishlash

```bash
cat fayl.txt            # mazmunini chiqarish
less fayl.txt           # sahifalab o'qish (q - chiqish, / - qidirish)
head -20 fayl.c         # birinchi 20 qator
tail -f log.txt         # oxiri + yangi qatorlarni kuzatish
cp a.txt b.txt          # nusxalash
mv a.txt papka/         # ko'chirish yoki nomini o'zgartirish
rm fayl.txt             # o'chirish (QAYTARIB BO'LMAYDI - savat yo'q!)
rm -r papka             # papkani ichidagilari bilan o'chirish - ehtiyot bo'ling
mkdir -p a/b/c          # papkalar yaratish
touch yangi.c           # bo'sh fayl yaratish
```

Amalda (har buyruq natijasi ko'rinadi):

```console
$ printf 'birinchi qator\nikkinchi qator\nuchinchi qator\nto'"'"'rtinchi qator\n' > ish/matn.txt
$ cat ish/matn.txt
birinchi qator
ikkinchi qator
uchinchi qator
to'rtinchi qator
$ head -2 ish/matn.txt
birinchi qator
ikkinchi qator
$ tail -1 ish/matn.txt
to'rtinchi qator
$ cp ish/matn.txt ish/nusxa.txt
$ mv ish/nusxa.txt ish/user/
$ ls ish ish/user
ish:
README
kernel
matn.txt
user

ish/user:
nusxa.txt
sh.c
$ rm ish/user/nusxa.txt
$ ls ish/user
sh.c
```

**Nima ko'rdik:** `printf '...\n...' > fayl` matnni faylga yozdi (`>` — yo'naltirish, 19.5). `head -2` — birinchi 2 qator; `tail -1` — oxirgi qator. `cp` nusxa yaratdi, `mv` uni `user/` ga ko'chirdi
(`ls` ikkala papkani ko'rsatdi), `rm` esa o'chirdi (undan keyin `user/` da faqat `sh.c` qoldi). `rm` **savatga tashlamaydi** — qaytarib bo'lmaydi.

> **Eslab qoling:** `rm` qaytarib bo'lmaydi. Papkani o'chirishdan oldin `ls` bilan tekshiring.

## 19.4. Qidirish — eng ko'p ishlatiladigan buyruqlar

**Bu nima?** `grep` — fayllar ichidan **matn qidiradi**; `find` — fayllarni **nomi bo'yicha** qidiradi. **Asosiy ishi:** katta kod bazasida kerakli joyni topish.

```bash
grep -rn "buddy_alloc" kernel/         # kernel/ ichida hamma fayllardan qidirish (qator raqami bilan)
grep -rn "struct process {" kernel/    # struktura qayerda ta'riflangan
grep -rl "spin_lock" kernel/ | wc -l   # nechta faylda ishlatiladi
find . -name "*.h" | head              # fayllarni nomi bo'yicha qidirish
```

Sandbox'ga mazmun solib, sinaymiz:

```console
$ echo 'void *buddy_alloc(int order);' > ish/kernel/mm/pmm.c
$ printf 'void slab_init(void);\nvoid *buddy_alloc(int);\nspin_lock(&l);\n' > ish/kernel/mm/slab.c
$ echo 'spin_lock(&pipe_lock);' > ish/kernel/fs/pipe.c
$ grep -rn "buddy_alloc" ish/kernel
ish/kernel/mm/pmm.c:1:void *buddy_alloc(int order);
ish/kernel/mm/slab.c:2:void *buddy_alloc(int);
$ grep -rl "spin_lock" ish | sort
ish/kernel/fs/pipe.c
ish/kernel/mm/slab.c
$ grep -rl "spin_lock" ish | wc -l
2
$ find ish -name "*.c" | sort
ish/kernel/fs/pipe.c
ish/kernel/mm/pmm.c
ish/kernel/mm/slab.c
ish/user/sh.c
```

**Qismlar:**

| Buyruq | Vazifasi |
|---|---|
| `grep -rn "so'z" papka` | `-r` — papka ichini **rekursiv** qidir; `-n` — **qator raqamini** ko'rsat |
| `grep -rl "so'z" papka` | `-l` — faqat **fayl nomlarini** ko'rsat (qaysi fayllarda bor) |
| `\| wc -l` | natijadagi qatorlarni sanash → "nechta faylda?" |
| `find papka -name "*.c"` | nomi `.c` bilan tugaydigan fayllarni top |

Katta kod bazasida (MyOS, keyin Linux) `grep -rn` — sizning eng yaqin do'stingiz. "Bu funksiya qayerda yozilgan va kim uni chaqiradi?" — har kuni beriladigan savol.

## 19.5. Quvurlar, yo'naltirish va jarayonlar

**Hayotdan misol: konveyer lentasi.** Zavoddagi lenta: birinchi stanok detalni kesadi, ikkinchisi teshadi, uchinchisi bo'yaydi. `cat log | grep xato | wc -l` — birinchi buyruq faylni o'qiydi, ikkinchisi
xatolarni ajratadi, uchinchisi sanaydi. Har bir buyruq kichik, lekin lenta bilan ulansa — kuchli.

```bash
ls | wc -l                      # stdout -> keyingi dasturning stdin'i
make 2>&1 | tee build.log       # xatolarni ham, ekranga ham faylga ham
./dastur > natija.txt           # chiqishni faylga
./dastur < kirish.txt           # kirishni fayldan
./dastur &                      # fonda ishga tushirish
ps aux | grep qemu              # ishlayotgan jarayonlar
kill 1234                       # jarayonga SIGTERM
Ctrl-C                          # joriy dasturni to'xtatish (SIGINT)
Ctrl-Z, keyin fg / bg           # pauza va davom ettirish
echo $?                         # oxirgi buyruqning chiqish kodi
```

Bularning **hammasi qanday ishlashini** siz bilasiz: 14-bob va 40-mashq. MyOS'ning shell'i ham aynan shularni qiladi.

```console
$ cat ish/matn.txt | grep qator | wc -l
4
$ cat ish/matn.txt | grep ikkinchi
ikkinchi qator
$ echo "salom" > ish/yozuv.txt; echo "dunyo" >> ish/yozuv.txt; cat ish/yozuv.txt
salom
dunyo
$ wc -l < ish/yozuv.txt
2
$ ls ish/yoq_fayl 2>/dev/null; echo "chiqish kodi: $?"
chiqish kodi: 2
$ ls ish/README; echo "chiqish kodi: $?"
ish/README
chiqish kodi: 0
```

**Nima ko'rdik:**

| Yozuv | Vazifasi |
|---|---|
| `a \| b` | `a` ning chiqishi `b` ga **kirish** bo'ladi (konveyer) |
| `>` | chiqishni **faylga yozish** (tozalab); `>>` — faylning **oxiriga qo'shish** |
| `<` | kirishni **fayldan olish** |
| `2>/dev/null` | xato xabarlarini (`stderr`, 2) "axlat qutisi"ga tashlash |
| `$?` | oxirgi buyruqning **chiqish kodi**: `0` — muvaffaqiyat, boshqa — xato (`ls` yo'q faylda 2 qaytardi) |

`cat ish/matn.txt | grep qator | wc -l` — to'rt qatorning to'rttasida "qator" so'zi bor → `4`; `grep ikkinchi` — faqat bitta qatorni qoldirdi.

Fonda ishga tushirish:

```console
$ sleep 0.2 & echo "fonda ishga tushdi"; wait; echo "fon ishi tugadi"
fonda ishga tushdi
fon ishi tugadi
```

`&` buyruqni **fonda** boshlaydi (shell kutmaydi); `wait` — fon ishlari tugashini kutadi.

### Ruxsatlar

**Bu nima?** Har bir faylning ruxsati bor: kim o'qishi, yozishi, bajarishi mumkin. **Asosiy ishi:** begonalardan himoya va "bu faylni ishga tushirib bo'ladimi" ni belgilash.

```console
$ printf '#!/bin/sh\necho "dastur ishladi"\n' > ish/dastur.sh
$ chmod 754 ish/dastur.sh && ls -l ish/dastur.sh | cut -c1-10
-rwxr-xr--
$ ./ish/dastur.sh
dastur ishladi
$ chmod -x ish/dastur.sh && ls -l ish/dastur.sh | cut -c1-10
-rw-r--r--
$ ./ish/dastur.sh 2>&1 | sed 's/^.*: Permission/Permission/' # xato kutiladi
Permission denied
```

`ls -l` ning birinchi 10 belgisi: `-rwxr-xr--`. Birinchi belgi — tur (`-` oddiy fayl, `d` papka), keyin uch guruh: **egasi** (`rwx`), **guruh** (`r-x`), **boshqalar** (`r--`).
`r` — o'qish, `w` — yozish, `x` — bajarish. Sakkizlik: `r=4, w=2, x=1` → `rwx`=7, `r-x`=5, `r--`=4 → **`754`**. `chmod +x` — bajariladigan qilish; `chmod -x` bilan olib tashlaganda dastur
ishga tushmay qoldi ("Permission denied").

`sudo buyruq` — administrator (root) nomidan bajarish. **Ehtiyot:** `sudo` bilan xato buyruq tizimni buzishi mumkin (masalan, `dd` bilan noto'g'ri diskka yozish — YAKUNIY.md, 5-bo'lim).

### Qo'llanma — tarjimasiz yo'l

`man ls` — ingliz tilida. Tushunish qiyin bo'lsa: `ls --help`, shu darslikdagi lug'at (31-bob) va misollar. Eng ko'p kerak bo'ladigan bayroqlar shu bobda yozilgan.

## 19.6. Git — vaqt mashinasi

**Hayotdan misol: o'yindagi saqlash nuqtasi.** Kompyuter o'yinida qiyin joydan oldin "saqlaysiz". Yutqazsangiz — shu joydan qayta boshlaysiz. `git commit` ham loyihangizning butun holatini saqlaydi.
Keyin istalgan saqlash nuqtasiga qaytish mumkin. Qancha ko'p saqlasangiz — shuncha kam narsa yo'qotasiz.

**Bu nima?** Git — kodning **har bir o'zgarishini** saqlaydigan tizim. **Asosiy ishi:** tarixni saqlash. Nega kerak:

- xato qilsangiz — istalgan oldingi holatga qaytasiz;
- "qachon buzildi?" — tarix aytib beradi (`git bisect`);
- boshqalar bilan birga ishlash (GitHub);
- MyOS'ning butun tarixi — o'quv qo'llanma: har bir commit bitta bosqich.

Asosiy tushunchalar:

| Atama | Ma'nosi |
|---|---|
| **repository (repo)** | loyiha + uning butun tarixi (`.git` papkasi) |
| **commit** | loyihaning bir lahzadagi "surati" + izoh |
| **branch (tarmoq)** | commitlarning alohida yo'nalishi (`main` — asosiysi) |
| **remote** | boshqa joydagi nusxa (masalan, GitHub'dagi `origin`) |
| **working tree** | siz tahrirlayotgan fayllar |
| **staging (index)** | keyingi commitga kiradigan o'zgarishlar |

**Hayotdan misol: `git diff` — "ikki rasm orasidagi farqni top".** Bolalar jurnallaridagi o'yin: ikki rasm deyarli bir xil, 7 ta farqni toping. `git diff` buni siz uchun qiladi: qaysi qator qo'shildi (`+`),
qaysi biri o'chirildi (`-`).

### Birinchi tajriba: kundalik va vaqt mashinasi

**Bu skript nima qiladi (umumiy):** vaqtinchalik papkada Git repo yaratadi; ikki "kun" yozib saqlaydi (commit); uchinchi kuni hamma narsani o'chirib yuboradi; `git diff` bilan nima o'zgarganini ko'radi;
xatoni bekor qilib oxirgi saqlangan holatga qaytadi; tarixni ko'radi. Skript hech narsani buzmaydi (vaqtinchalik papkada).

```sh
# kundalik.sh - terminal va git: saqlash, farq, tarix, qaytish
set -e
cd "$(mktemp -d)"
git init -q -b main
git config user.name "O'quvchi"
git config user.email "oquvchi@example.com"

echo "1-kun: C tilini boshladim" > kundalik.txt
git add kundalik.txt
git commit -q -m "1-kun"

echo "2-kun: ko'rsatkichlarni o'rgandim" >> kundalik.txt
git commit -q -am "2-kun"

echo "3-kun: hamma narsani o'chirib yubordim :(" > kundalik.txt
echo "--- git diff: nima o'zgardi?"
git diff | grep '^[-+][^-+]'

echo "--- Xatoni bekor qilish: oxirgi saqlangan holatga qaytish"
git checkout -- kundalik.txt
cat kundalik.txt

echo "--- Tarix (saqlash nuqtalari)"
git log --format='%s'

echo "--- Konveyer: nechta qatorda 'kun' so'zi bor?"
grep kun kundalik.txt | wc -l
```

```console
$ sh kundalik.sh
--- git diff: nima o'zgardi?
-1-kun: C tilini boshladim
-2-kun: ko'rsatkichlarni o'rgandim
+3-kun: hamma narsani o'chirib yubordim :(
--- Xatoni bekor qilish: oxirgi saqlangan holatga qaytish
1-kun: C tilini boshladim
2-kun: ko'rsatkichlarni o'rgandim
--- Tarix (saqlash nuqtalari)
2-kun
1-kun
--- Konveyer: nechta qatorda 'kun' so'zi bor?
2
```

**Qadam-baqadam:**

| Buyruq | Vazifasi |
|---|---|
| `git init` | bo'sh repo yaratish (`.git` papkasi) |
| `git add fayl` | faylni keyingi commitga **tayyorlash** (staging) |
| `git commit -m "izoh"` | tayyorlanganlarni tarixga **saqlash** (saqlash nuqtasi) |
| `git commit -am "izoh"` | `-a`: o'zgargan (allaqachon kuzatilayotgan) fayllarni avtomatik tayyorlab saqlash |
| `git diff` | saqlanmagan o'zgarishlar: `-` — o'chirilgan, `+` — qo'shilgan qator |
| `git checkout -- fayl` | faylni oxirgi saqlangan holatga **qaytarish** |
| `git log --format='%s'` | tarix (faqat izohlar) |

**Nima ko'rdik:** `git diff` ikki eski qator o'chirilib, bitta yangi qator paydo bo'lganini ko'rsatdi; `checkout` fayl mazmunini 2-kundagi holatga qaytardi — "hamma narsani o'chirib yubordim" xatosi yo'qoldi.

### Kundalik ish

```bash
git clone https://github.com/azamvosejonov/C_loyha     # birinchi marta yuklab olish
cd C_loyha
git pull                        # GitHub'dagi yangiliklarni olish

# ... kod yozasiz ...
git status                      # nima o'zgardi
git diff                        # aniq qaysi qatorlar o'zgardi
git add mashqlar/01_kvadratlar/yechim.c      # commitga qo'shish
git commit -m "01-mashq: kvadratlar yig'indisi"
git log --oneline               # tarix
```

### Tarmoqlar (branch) va saqlab turish (stash)

**Hayotdan misol: qoralama daftar.** Asosiy ishni buzmasdan, yangi g'oyani alohida qoralamada sinab ko'rasiz. Yaxshi chiqsa — asosiyga ko'chirasiz (**merge**), yomon chiqsa — qoralamani tashlab yuborasiz.

```sh
# shoxcha.sh - branch, merge va stash
set -e
cd "$(mktemp -d)"
git init -q -b main
git config user.name "O'quvchi"
git config user.email "oquvchi@example.com"

echo "asosiy kod" > kod.txt
git add kod.txt && git commit -q -m "boshlang'ich kod"

git checkout -q -b yangi_goya           # yangi tarmoq yaratib, unga o'tish
echo "tajriba qatori" >> kod.txt
git commit -q -am "yangi g'oya"
echo "--- yangi_goya tarmog'ida:"; cat kod.txt

git checkout -q main                     # asosiyga qaytish
echo "--- main tarmog'ida (tajriba ko'rinmaydi):"; cat kod.txt

git merge -q yangi_goya                  # g'oyani asosiyga qo'shish
echo "--- merge dan keyin main:"; cat kod.txt

echo "yarim qolgan ish" >> kod.txt
git stash -q                             # saqlanmagan ishni chetga olib qo'yish
echo "--- stash dan keyin (toza):"; cat kod.txt
git stash pop -q                         # qaytarib olish
echo "--- stash pop dan keyin:"; cat kod.txt
```

```console
$ sh shoxcha.sh
--- yangi_goya tarmog'ida:
asosiy kod
tajriba qatori
--- main tarmog'ida (tajriba ko'rinmaydi):
asosiy kod
--- merge dan keyin main:
asosiy kod
tajriba qatori
--- stash dan keyin (toza):
asosiy kod
tajriba qatori
--- stash pop dan keyin:
asosiy kod
tajriba qatori
yarim qolgan ish
```

**Nima ko'rdik:** `git checkout -b` yangi tarmoq yaratib o'tdi; `yangi_goya` tarmog'ida qo'shilgan qator `main` da **ko'rinmadi**; `git merge` uni asosiyga qo'shdi. `git stash` — saqlanmagan ishni vaqtincha chetga oldi
(fayl toza holatga qaytdi), `stash pop` — qaytardi.

### Xatolarni tuzatish

```bash
git diff                        # o'zgarishlarni ko'rish
git restore fayl.c              # fayldagi SAQLANMAGAN o'zgarishlarni bekor qilish (ehtiyot!)
git stash                       # o'zgarishlarni vaqtincha chetga olib qo'yish
git stash pop                   # qaytarish
git show 9cf0d2d                # bitta commit nimani o'zgartirgan
git checkout e5906bb            # eski holatga "vaqt safari" (MyOS v0.1)
git checkout main               # hozirgi holatga qaytish
```

### Shaxsiy nusxangiz

Mashqlarni yechib, GitHub'ga saqlash uchun: GitHub'da loyihani **fork** qiling (o'z hisobingizga nusxa), keyin o'z nusxangizni `git clone` qiling. Shunda har kuni `git commit` + `git push` bilan ishingiz
saqlanadi va rezyumega ko'rsatish mumkin bo'ladi.

### `git bisect` — xatoni topuvchi

**Bu nima?** "Bir hafta oldin ishlardi, endi yo'q" — 50 ta commitdan qaysi biri buzdi? `git bisect` **ikkilik qidiruv** (28-bob) bilan aybdorni topadi: o'rtadagi commitni ochadi, siz "yaxshi" yoki "yomon" deysiz,
Git yarimiga qisqartiradi. ~6 qadamda 50 commitdan topiladi.

Buni avtomatik bajaramiz: 6 ta commit, 4-chisi dasturni buzadi; `git bisect run` testni o'zi takrorlaydi.

```sh
# bisect.sh - aybdor commitni avtomatik topish
set -e
cd "$(mktemp -d)"
git init -q -b main
git config user.name "O'quvchi"
git config user.email "oquvchi@example.com"

for n in 1 2 3 4 5 6; do
    if [ "$n" -ge 4 ]; then
        echo "buzuq $n" > holat.txt            # 4-commitdan boshlab buzuq
    else
        echo "yaxshi $n" > holat.txt
    fi
    git add holat.txt
    git commit -q -m "commit $n"
done

git bisect start HEAD HEAD~5 >/dev/null             # HEAD - buzuq, HEAD~5 (1-commit) - yaxshi
git bisect run sh -c 'grep -q yaxshi holat.txt' > bisect.log
bad=$(grep -o '^[0-9a-f]\{40\} is the first bad commit' bisect.log | cut -c1-40)
echo "birinchi buzuq commit: $(git log -1 --format=%s "$bad")"
git bisect reset >/dev/null 2>&1
```

```console
$ sh bisect.sh
birinchi buzuq commit: commit 4
```

**Nima ko'rdik:** Git ikkilik qidiruv bilan atigi bir necha qadamda "`commit 4` — birinchi buzuq commit" ni topdi (test: fayl "yaxshi" so'zini o'z ichiga oladimi). Haqiqiy loyihada test — dasturingizni ishga tushirish yoki `make test`.

## 19.7. Muharrir

Muhim emas qaysi biri — muhimi uni tez ishlatish:

- **VS Code** — boshlovchi uchun eng qulay. "C/C++" kengaytmasi: funksiyaga sakrash (F12), hamma chaqiruvlarni topish (Shift+F12).
- **vim/nano** — terminalda, serverda. `nano fayl.c` — oddiy; `vim` — kuchli, lekin o'rganish kerak (`i` — yozish, `Esc` — buyruq rejimi, `:wq` — saqlab chiqish, `:q!` — saqlamasdan chiqish).
- **MyOS ichida**: `edit fayl` — o'zimiz yozgan muharrir (`user/bin/edit.c`).

## Hayotdan misol va to'liq dastur

Bobning to'liq namunasi — yuqoridagi uchta skript: `kundalik.sh` (saqlash/qaytish), `shoxcha.sh` (tarmoq/merge/stash), `bisect.sh` (aybdorni topish). Ular birgalikda dasturchining kundalik **vaqt mashinasi**:
saqlaysiz, tajriba qilasiz, xato bo'lsa qaytasiz, buzilganda qachon buzilganini topasiz.

**Sinab ko'ring:** skriptdagi buyruqlarni o'z papkangizda birma-bir qo'lda yozing. `git log --oneline` bilan tarixni ko'ring va `git show HEAD~1` bilan 1-kundagi saqlash nuqtasini oching.

<!-- katta:boshi -->
## Katta loyiha: veb-server jurnalini terminal bilan tahlil qilish

**Umumiy fikr.** Haqiqiy ishda dasturchi doim **jurnallar** (log) bilan ishlaydi: "server nega qulab tushdi?", "kim ko'p so'rov yubordi?", "qaysi sahifa 404 beryapti?". Buning uchun katta dastur yozilmaydi: **kichik buyruqlar** (`cut`, `sort`, `uniq`, `awk`, `grep`, `head`) **quvur** (`|`) bilan ulanadi. 19-bob (terminal) aynan shu mahoratni o'rgatadi, bu bosqichda esa uni **haqiqiy katta fayl** (3000 qator) ustida qo'llaymiz.

**Hayotiy o'xshatish:** konveyer. Birinchi ishchi faqat kerakli ustunni kesib oladi, ikkinchisi tartiblaydi, uchinchisi sanaydi, to'rtinchisi eng ko'pini tanlaydi. Har biri **bitta** ish qiladi, lekin birga — kuchli.

### Bu bosqichda nima qilamiz

Ikkita skript:

| Fayl | Vazifasi |
|---|---|
| `log_yarat.sh` | sinov uchun 3000 qatorli **veb-server jurnali** yaratadi (har safar **bir xil** natija: o'z tasodif generatorimiz) |
| `tahlil.sh` | jurnalni 9 ta hisobot bilan tahlil qiladi, har hisobot — buyruqlar zanjiri |

Jurnal qatori (nginx/apache "access log" formati):

```text
198.51.100.77 - - [05/Oct/2026:15:20:00 +0500] "GET / HTTP/1.1" 500 231
 IP manzil           vaqt                        so'rov            kod  hajm
```

`awk` maydonlari bo'sh joy bo'yicha: `$1` — IP, `$7` — sahifa (yo'l), `$(NF-1)` — holat kodi (oxiridan ikkinchi), `$NF` — hajm (oxirgi). `NF` — maydonlar soni.

**Skriptning eng muhim zanjirlari (har birini o'qiymiz):**

**«Eng faol 3 IP»:**

```bash
cut -d' ' -f1 "$LOG" | sort | uniq -c | sort -rn | head -3
```

| Bosqich | Nima qiladi |
|---|---|
| `cut -d' ' -f1` | har qatordan **1-ustunni** (IP) kesib oladi (ajratgich: bo'sh joy) |
| `sort` | tartiblaydi (bir xil IP lar **yonma-yon** keladi) |
| `uniq -c` | ketma-ket bir xil qatorlarni **sanaydi** (`uniq` faqat qo'shni takrorlarni topadi — shuning uchun oldin `sort`!) |
| `sort -rn` | sonlar bo'yicha (`-n`) **kamayish** tartibida (`-r`) |
| `head -3` | birinchi 3 qator |

Bu — eng ko'p uchraydigan **«eng ko'pi» andozasi**: `... | sort | uniq -c | sort -rn | head`.

**«Xatolar ulushi»** — `awk` o'zi hisoblaydi:

```bash
awk '{ k = $(NF-1); jami++; if (k >= 400) xato++ }
     END { printf "%d / %d = %.1f%%\n", xato, jami, 100 * xato / jami }' "$LOG"
```

`END { ... }` bloki **hamma qator o'qilgandan keyin** bir marta ishlaydi.

Hammasini ishga tushiramiz (avval jurnalni yaratamiz, boshini ko'ramiz):

```console
$ cd katta_loyiha/tizim/19_log_tahlil
$ chmod +x log_yarat.sh tahlil.sh
$ ./log_yarat.sh 3000 > access.log
$ head -3 access.log
198.51.100.77 - - [05/Oct/2026:15:20:00 +0500] "GET / HTTP/1.1" 500 231
192.168.1.20 - - [05/Oct/2026:14:48:01 +0500] "POST /api/ombor HTTP/1.1" 200 7861
203.0.113.50 - - [05/Oct/2026:21:04:02 +0500] "GET /login HTTP/1.1" 200 8049
$ ./tahlil.sh access.log
=== 1. Umumiy ===
so'rovlar soni: 3000
noyob IP lar:   8
=== 2. Eng faol 3 IP ===
    403 10.0.0.9
    384 198.51.100.77
    379 172.16.4.8
=== 3. Holat kodlari ===
   2392 200
    152 301
     57 403
    307 404
     92 500
=== 4. Xatolar (4xx va 5xx) ulushi ===
456 / 3000 = 15.2%
=== 5. Eng ko'p so'ralgan 3 sahifa ===
    405 /login
    384 /
    381 /yordam
=== 6. Faqat 404 bo'lgan sahifalar (noyob) ===
/ /admin /api/narx /api/ombor /index.html /login /rasm/logo.png /yordam 
=== 7. Soatlar bo'yicha so'rovlar (kunning birinchi 6 soati) ===
00:00  ################################### 140
01:00  ############################### 127
02:00  ################################## 137
03:00  ############################### 126
04:00  ############################# 118
05:00  ################################## 138
=== 8. Uzatilgan ma'lumot (faqat 200) ===
11937387 bayt = 11.4 MB
=== 9. 500 xatosi bergan IP lar ===
     22 203.0.113.50
     16 198.51.100.77
     12 172.16.4.8
```

**Nima ko'rdik (hisobotlar bo'yicha):**

| Hisobot | Natija | Fikr |
|---|---|---|
| 1 | 3000 so'rov, 8 ta noyob IP | `sort -u` — noyob qiymatlar |
| 2 | eng faol: `10.0.0.9` (403 ta) | bitta IP boshqalardan sezilarli ko'p so'rov yuborsa — shubhali (botmi?) |
| 3 | 200: 2392, 404: 307, 500: 92 ... | taxminan 80% muvaffaqiyatli |
| 4 | `456 / 3000 = 15.2%` | xatolar ulushi (4xx + 5xx) |
| 5 | eng ko'p: `/login` (405) | qaysi sahifaga yuk tushmoqda |
| 6 | 404 bergan 8 ta sahifa | mavjud sahifalar ham 404 bergan — tarmoq/sozlash muammosi belgisi |
| 7 | soatlar bo'yicha `#` grafigi | **matnli** diagramma: `substr("####...", 1, son/4)` |
| 8 | 11.4 MB | faqat `200` javoblar hajmi |
| 9 | 500 bergan IP lar | server xatosi qaysi mijozlarda ko'proq |

> **Eslab qoling:** terminalda **kichik, bir ishni qiladigan** buyruqlarni **quvur** bilan ulang. Eng ko'p ishlatiladigan andoza: `kes → tartibla → sana → tartibla (teskari) → bosh N`. `uniq` dan **oldin doim `sort`**. Murakkab hisob kerak bo'lsa — `awk`. Jurnalni **eng oldin** `head`/`wc -l`/`grep` bilan ko'zdan kechiring.

**O'zingiz qo'shing (yechimsiz):**

1. Hisobot qo'shing: **har IP bo'yicha jami uzatilgan bayt** (maslahat: `awk` da `bayt[$1] += $NF`, `END` da `for (ip in bayt)`).
2. Ulushi **10% dan oshgan** soatni toping (404 va 500 lar soat bo'yicha). `awk -F'[:[]'` nima uchun ishlaydi? Ajratgich qanday?
3. `./log_yarat.sh 3000 | ./tahlil.sh /dev/stdin` — shu ishlaydimi? Nega `wc -l < "$LOG"` bilan `wc -l "$LOG"` farq qiladi (fayl nomi chiqadimi)?
<!-- katta:oxiri -->

## Bob xulosasi (yodlash uchun)

1. Terminal — aniq buyruqlar; `pwd`, `ls`, `cd` (`..`, `~`, `-`), `*` va `?` — yurish va fayllarni tanlash. `rm` — **qaytarib bo'lmaydi**.
2. `grep -rn "so'z" papka` — matn qidirish; `find -name` — fayl qidirish; `|` (quvur), `>`/`>>`/`<` (yo'naltirish), `$?` (chiqish kodi).
3. Ruxsatlar: `rwx` × (egasi, guruh, boshqalar); `r=4, w=2, x=1` → `754`; `chmod +x`.
4. Git: `add` (tayyorla) → `commit` (saqla) → `log`/`diff` (ko'r) → `checkout`/`restore` (qayt); `branch` + `merge` — alohida tajriba; `stash` — vaqtincha chetga.
5. `git bisect` — ikkilik qidiruv bilan buzgan commitni topadi.

## Savol-javob

**Savol:** Nega `rm` bilan o'chirilgan fayl axlat qutisiga tushmaydi?
**Javob:** Terminalda "axlat qutisi" yo'q: `rm` nomni bevosita o'chiradi (27-bob). Shuning uchun `rm -rf` dan oldin yo'lni ikki marta tekshiring; xavfli buyruqlarda avval `ls` bilan nimani o'chirishni ko'rib oling.

**Savol:** Nega git'da avval `add`, keyin `commit` kerak?
**Javob:** `add` — "aynan **shu** o'zgarishlar keyingi commit'ga kirsin" deb tanlash bosqichi. Ko'p o'zgarish qilgan bo'lsangiz ham, ularni mantiqiy, kichik commit'larga bo'lib saqlay olasiz (19.6).

**Savol:** Nega `git pull` ba'zan to'qnashuv (conflict) beradi?
**Javob:** Siz ham, boshqa kishi ham bitta qatorni boshqacha o'zgartirgan bo'lsa, git qaysi biri to'g'ri ekanini bilmaydi. U ikkala variantni faylga yozadi va sizdan qo'lda tanlashni so'raydi (31-bobdagi `CONFLICT` xabari).

## O'zingizni tekshiring

1. `cd ..` va `cd -` farqi?
2. `rwxr-x---` sakkizlikda qanday yoziladi?
3. `kernel/` ichida `pipe_read` qayerda chaqirilishini qanday topasiz?
4. `git add` va `git commit` farqi?
5. `git bisect` qaysi algoritmga asoslangan?

<details><summary>Javoblar</summary>

1. `..` — ota papka; `-` — oldingi turgan papka.
2. `750`.
3. `grep -rn "pipe_read" kernel/`.
4. `add` — o'zgarishni keyingi commitga tayyorlaydi; `commit` — tayyorlanganlarni tarixga yozadi.
5. Ikkilik qidiruv (binary search).
</details>

## Mashq

- Loyihani o'z GitHub hisobingizga fork qiling va clone qiling. Har bir yechilgan mashqdan keyin commit qiling.
- `grep -rn` bilan MyOS'da `kmalloc` chaqirilgan 5 ta joyni toping va har biri nimaga xotira so'rayotganini aniqlang.
- `git log --oneline --reverse` — MyOS'ning birinchi 5 commitini `git show --stat` bilan ko'rib chiqing.

<!-- loyiha:boshi -->
## Loyiha: `mhead` — buyruq qatori argumentlari

**Maqsad:** terminal buyrug'i kabi ishlaydigan dastur yozish: `argc`/`argv`, bayroqlar, xato xabarlari `stderr` ga, chiqish kodi,
fayl yoki `stdin` dan o'qish. Bu — barcha Unix vositalarining umumiy tuzilishi (19-bob, 5.11).
**Bobdan ishlatiladi:** `argv` tahlili, `strtol` bilan tekshirilgan son o'qish, `stdin`/`stderr`, chiqish kodlari.

**Talab:** `mhead [-n SON] [fayl]` — faylning (yoki `stdin` ning) birinchi `SON` qatorini chiqarsin (standart: 10).
- Noto'g'ri son yoki ortiqcha argument: xabar `stderr` ga, chiqish kodi **2**.
- Fayl ochilmasa: `mhead: <fayl>: <sabab>` `stderr` ga, chiqish kodi **1**.
- Muvaffaqiyat: **0**.

```c
/* mhead.c - head buyrug'i */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void yordam(void)
{
    fprintf(stderr, "Ishlatish: mhead [-n SON] [fayl]\n");
}

int main(int argc, char **argv)
{
    long n = 10;
    const char *yol = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0) {
            if (i + 1 >= argc) {                /* "-n" dan keyin son yo'q */
                yordam();
                return 2;
            }
            char *oxir;
            n = strtol(argv[++i], &oxir, 10);
            if (oxir == argv[i] || *oxir != '\0' || n < 0) {
                fprintf(stderr, "mhead: noto'g'ri son: %s\n", argv[i]);
                return 2;
            }
        } else if (strcmp(argv[i], "--yordam") == 0) {
            yordam();
            return 0;
        } else if (!yol) {
            yol = argv[i];
        } else {
            yordam();
            return 2;
        }
    }

    FILE *f = yol ? fopen(yol, "r") : stdin;    /* fayl bermasa - standart kirish */
    if (!f) {
        fprintf(stderr, "mhead: %s: %s\n", yol, strerror(errno));
        return 1;
    }
    char qator[1024];
    for (long k = 0; k < n && fgets(qator, sizeof(qator), f); k++)
        fputs(qator, stdout);
    if (f != stdin)
        fclose(f);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g mhead.c -o mhead
$ seq 1 20 > sonlar.txt
$ ./mhead sonlar.txt | tr '\n' ' '; echo
1 2 3 4 5 6 7 8 9 10 
$ ./mhead -n 3 sonlar.txt
1
2
3
$ seq 1 100 | ./mhead -n 2
1
2
$ ./mhead yoq.txt 2>&1; echo "chiqish kodi: $?"
mhead: yoq.txt: No such file or directory
chiqish kodi: 1
$ ./mhead -n abc sonlar.txt 2>&1; echo "chiqish kodi: $?"
mhead: noto'g'ri son: abc
chiqish kodi: 2
$ ./mhead -n 0 sonlar.txt | wc -l
0
```

`|` (quvur) tufayli bir xil dastur ham fayldan, ham boshqa buyruq chiqishidan o'qiydi — chunki `stdin` ham oddiy fayl deskriptori (0).
Xato xabarlari `stdout` ga emas, `stderr` ga yozilgani uchun quvurga aralashmaydi: `./mhead yoq.txt | wc -l` faqat `0` ni beradi, xabar terminalga chiqadi.

**Kengaytiring:** `-c` bayrog'ini qo'shing (birinchi N **bayt**). Bir nechta fayl berilsa, har birining oldiga `==> fayl <==` sarlavhasini chiqaring.

## Mustaqil loyiha: mini `wc` ★★★

**Vazifa:** `wc` buyrug'ini yozing: `mwc [-l] [-w] [-c] [fayl...]`. Fayl: `mwc.c`. Kirish-chiqish uchun `stdio` (`fopen`, `fread` yoki
`getc`, `printf`) ishlatishingiz mumkin.

**Talab:**
- **Ustunlar:** `-l` — qatorlar (`\n` soni), `-w` — so'zlar (probel, tab, `\n` bilan ajratilgan bo'sh bo'lmagan ketma-ketliklar),
  `-c` — baytlar. Hech qaysi berilmasa — uchalasi. Ustunlar tartibi doim: qator, so'z, bayt.
- **Format:** har tanlangan ustun `%7ld`, oxirida probel va fayl nomi: `"%7ld%7ld%7ld %s\n"` shaklida
  (faqat tanlangan ustunlar; `stdin` dan o'qilsa nom yozilmaydi va oxirgi probel ham bo'lmaydi).
- **Bir nechta fayl** (`argv` da 1 dan ko'p fayl nomi) berilsa, oxirida `jami` qatori chiqadi (muvaffaqiyatli
  o'qilgan fayllar yig'indisi, nomi `jami`).
- **Ochilmagan fayl:** `mwc: <fayl>: <strerror>` xabari **stderr** ga, dastur davom etadi (qolgan fayllarni ham
  ishlaydi), oxirida chiqish kodi **1**. Bunday fayl `jami` ga kirmaydi.
- Fayl nomi berilmasa, `stdin` dan o'qiladi.

**Kirish fayllari** (`darslik/loyihalar/19_mini_wc/`):

`a.txt` (4 qator, oxirida `\n` bor):

```text
salom dunyo
ikkinchi qator bor

uchinchi
```

`b.txt` (**oxirida `\n` yo'q!**):

```text
bir
ikki uch
to'rt besh olti yetti
```

**1-sinov:** `./dastur a.txt`

```text
      4      6     41 a.txt
```

**2-sinov:** `./dastur -l -w a.txt b.txt`

```text
      4      6 a.txt
      2      7 b.txt
      6     13 jami
```

**3-sinov:** `./dastur -c a.txt yoq.txt b.txt 2>/dev/null; echo "chiqish kodi: $?"` va `./dastur -w < a.txt`

```text
     41 a.txt
     34 b.txt
     75 jami
chiqish kodi: 1
      6
```

**Maslahat** (yechim emas):
- Har baytni o'qib (`getc`), holatni yuriting: `bayt++`; `c == '\n'` bo'lsa `qator++`; bo'shliqdan harfga o'tishda `soz++`.
- Bo'shliq: `' '`, `'\t'`, `'\n'` (va istasangiz `'\r'`). "Oldingi belgi bo'shliqmi?" bayrog'i so'zlarni sanaydi.
- `b.txt` oxirgi qatori `\n` siz — `-l` uni sanamaydi (`wc` ham shunday), lekin `-w` va `-c` hisobga oladi.
- Ustunlarni chiqarishda "tanlanganmi" bayroqlarini tekshiring, yig'indilarni `long` da saqlang.
- `stderr` ga `fprintf(stderr, ...)` yozing — `2>/dev/null` shu xabarni yashiradi.

**Tekshirish:**

```bash
D=~/C_loyha/darslik/loyihalar/19_mini_wc
cp $D/a.txt $D/b.txt .                      # kirish fayllarini o'z papkangizga oling
gcc -Wall -Wextra -g -fsanitize=address,undefined mwc.c -o dastur
./dastur a.txt | diff - $D/kutilgan.txt && echo "1: TO'G'RI"
./dastur -l -w a.txt b.txt | diff - $D/kutilgan_2.txt && echo "2: TO'G'RI"
(./dastur -c a.txt yoq.txt b.txt 2>/dev/null; echo "chiqish kodi: $?"; ./dastur -w < a.txt) | diff - $D/kutilgan_3.txt && echo "3: TO'G'RI"
```
<!-- loyiha:oxiri -->

Keyingi bob: [20-bob. Sonlar kompyuterda](20-sonlar.md)
