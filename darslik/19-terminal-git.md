# 19-bob. Linux terminali va Git — dasturchining ish stoli

> **II qism boshlanishi.** I qism (00–18) — C tili. II qism (19–31) — kompyuter tizimlari va
> operatsion tizimlar nazariyasi: odatda ingliz tilidagi bir nechta kitobdan o'rganiladigan bilimlar
> shu yerda o'zbek tilida jamlangan. Bu bob — hamma narsaning poydevori: terminalda ishlash va Git.
> Agar terminalni allaqachon yaxshi bilsangiz, 19.6 dan (Git) boshlang.

> **To'liq ishlaydigan misol:** [misollar/19_terminal.sh](misollar/19_terminal.sh) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Hayotdan misollar

**Terminal — SMS bilan boshqarish (19.1).** Grafik oynalar — televizor pultidagi tugmalar: qulay, lekin
faqat ishlab chiqaruvchi qo'ygan tugmalar bor. Terminal — aniq buyruqlar yozish: "fayllar ichidan
'xato' so'zi bor qatorlarni top, sanab, eng ko'pini ko'rsat". Buni tugmalar bilan qilib bo'lmaydi.
Serverlarda va yadro ishida esa oyna umuman yo'q — faqat terminal.

**Fayl tizimi — shkaf, tortma va papka (19.2).** `/` — butun shkaf, `/home/ali` — Alining tortmasi,
`/home/ali/loyiha` — tortmadagi papka. `pwd` — "men qaysi tortmadaman?", `cd` — boshqa tortmaga o'tish,
`ls` — "bu tortmada nima bor?". `..` — bir daraja yuqori.

**Quvur `|` — konveyer lentasi (19.5).** Zavoddagi lenta: birinchi stanok detalni kesadi, ikkinchisi
teshadi, uchinchisi bo'yaydi. `cat log | grep xato | wc -l` — birinchi buyruq faylni o'qiydi, ikkinchisi
xatolarni ajratadi, uchinchisi sanaydi. Har bir buyruq kichik, lekin lenta bilan ulansa — kuchli.

**Git commit — o'yindagi saqlash nuqtasi (19.6).** Kompyuter o'yinida qiyin joydan oldin "saqlaysiz".
Yutqazsangiz — shu joydan qayta boshlaysiz. `git commit` ham loyihangizning butun holatini saqlaydi.
Keyin istalgan saqlash nuqtasiga qaytish mumkin. Qancha ko'p saqlasangiz — shuncha kam narsa yo'qotasiz.

**`git diff` — "ikki rasm orasidagi farqni top" (19.6).** Bolalar jurnallaridagi o'yin: ikki rasm
deyarli bir xil, 7 ta farqni toping. `git diff` buni siz uchun qiladi: qaysi qator qo'shildi (`+`),
qaysi biri o'chirildi (`-`).

**Branch — qoralama daftar (19.6).** Asosiy ishni buzmasdan, yangi g'oyani alohida qoralamada sinab
ko'rasiz. Yaxshi chiqsa — asosiyga ko'chirasiz (merge), yomon chiqsa — qoralamani tashlab yuborasiz.

### To'liq skript: kundalik va vaqt mashinasi

Bu skript vaqtinchalik papkada ishlaydi va hech narsani buzmaydi. Har bir buyruqni keyin o'zingiz
qo'lda takrorlang.

```sh
# kundalik.sh - terminal va git: saqlash, farq, tarix, qaytish
set -e
cd "$(mktemp -d)"
git init -q
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

**Sinab ko'ring:** skriptdagi buyruqlarni o'z papkangizda birma-bir qo'lda yozing. `git log --oneline`
bilan tarixni ko'ring va `git show HEAD~1` bilan 1-kundagi saqlash nuqtasini oching.

## 19.1. Nega terminal

Grafik oyna (sichqoncha bilan bosish) qulay, lekin dasturchi uchun terminal kuchliroq:
- har bir amalni **aniq yozib**, qayta takrorlash mumkin (skript);
- kichik dasturlarni **quvur** (`|`) bilan birlashtirib, murakkab ish qilish mumkin;
- serverlar, yadrolar, o'rnatilgan tizimlar — deyarli hammasida faqat terminal bor;
- MyOS'ning o'zida ham shell bor — siz uni yozgansiz (40-mashq).

Terminalda ishlaydigan dastur — **shell** (Linux'da odatda `bash`). U siz yozgan qatorni o'qiydi,
so'zlarga bo'ladi, dasturni topib ishga tushiradi (`fork` + `exec`, 14-bob) va natijani ko'rsatadi.

## 19.2. Fayl tizimi bo'ylab yurish

```bash
pwd                     # qaysi papkadaman (print working directory)
ls                      # papkada nima bor
ls -la                  # batafsil: ruxsatlar, hajm, sana, yashirin fayllar (. bilan boshlanadigan)
cd kernel/mm            # papkaga kirish
cd ..                   # bir daraja yuqoriga
cd ~                    # uy papkasiga (/home/ism)
cd -                    # oldingi papkaga qaytish
```

| Belgi | Ma'nosi |
|---|---|
| `/` | ildiz papka (yoki yo'l ajratgichi) |
| `.` | joriy papka |
| `..` | ota papka |
| `~` | uy papkangiz |
| `*` | istalgan belgilar (`*.c` — hamma C fayllar) |
| `?` | bitta istalgan belgi |

**Mutlaq yo'l** `/` bilan boshlanadi (`/home/ali/C_loyha`), **nisbiy yo'l** — joriy papkadan
(`kernel/mm/pmm.c`).

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

## 19.4. Qidirish — eng ko'p ishlatiladigan buyruqlar

```bash
grep -rn "buddy_alloc" kernel/         # kernel/ ichida hamma fayllardan qidirish (qator raqami bilan)
grep -rn "struct process {" kernel/    # struktura qayerda ta'riflangan
grep -rl "spin_lock" kernel/ | wc -l   # nechta faylda ishlatiladi
find . -name "*.h" | head              # fayllarni nomi bo'yicha qidirish
```

Katta kod bazasida (MyOS, keyin Linux) `grep -rn` — sizning eng yaqin do'stingiz. "Bu funksiya qayerda
yozilgan va kim uni chaqiradi?" — har kuni beriladigan savol.

## 19.5. Quvurlar, yo'naltirish va jarayonlar

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

Bularning **hammasi qanday ishlashini** siz bilasiz: 14-bob va 40-mashq. MyOS'ning shell'i ham aynan
shularni qiladi.

### Ruxsatlar

```bash
ls -l dastur.sh
-rwxr-xr-- 1 ali ali 120 ... dastur.sh
```

`rwx` uch guruh: **egasi**, **guruh**, **boshqalar**. `r` — o'qish, `w` — yozish, `x` — bajarish.
Sakkizlik: `r=4, w=2, x=1` → `rwxr-xr--` = `754`. `chmod +x skript.sh` — bajariladigan qilish.
`sudo buyruq` — administrator (root) nomidan bajarish. **Ehtiyot:** `sudo` bilan xato buyruq
tizimni buzishi mumkin (masalan, `dd` bilan noto'g'ri diskka yozish — YAKUNIY.md, 5-bo'lim).

### Qo'llanma — tarjimasiz yo'l

`man ls` — ingliz tilida. Tushunish qiyin bo'lsa: `ls --help`, shu darslikdagi lug'at (31-bob) va
misollar. Eng ko'p kerak bo'ladigan bayroqlar shu bobda yozilgan.

## 19.6. Git — vaqt mashinasi

Git — kodning **har bir o'zgarishini** saqlaydigan tizim. Nega kerak:
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

Mashqlarni yechib, GitHub'ga saqlash uchun: GitHub'da loyihani **fork** qiling (o'z hisobingizga nusxa),
keyin o'z nusxangizni `git clone` qiling. Shunda har kuni `git commit` + `git push` bilan ishingiz
saqlanadi va rezyumega ko'rsatish mumkin bo'ladi.

### `git bisect` — xatoni topuvchi

"Bir hafta oldin ishlardi, endi yo'q" — 50 ta commitdan qaysi biri buzdi?

```bash
git bisect start
git bisect bad                  # hozirgi holat - buzuq
git bisect good 9cf0d2d         # bu commit - yaxshi edi
# git o'rtadagi commitni ochadi: sinaysiz va "git bisect good" yoki "git bisect bad" deysiz
# ~6 qadamda aybdor commit topiladi (ikkilik qidiruv - 28-bob!)
git bisect reset
```

## 19.7. Muharrir

Muhim emas qaysi biri — muhimi uni tez ishlatish:
- **VS Code** — boshlovchi uchun eng qulay. "C/C++" kengaytmasi: funksiyaga sakrash (F12),
  hamma chaqiruvlarni topish (Shift+F12).
- **vim/nano** — terminalda, serverda. `nano fayl.c` — oddiy; `vim` — kuchli, lekin o'rganish kerak
  (`i` — yozish, `Esc` — buyruq rejimi, `:wq` — saqlab chiqish, `:q!` — saqlamasdan chiqish).
- **MyOS ichida**: `edit fayl` — o'zimiz yozgan muharrir (`user/bin/edit.c`).

## 19.8. O'zingizni tekshiring

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

## 19.9. Mashq

- Loyihani o'z GitHub hisobingizga fork qiling va clone qiling. Har bir yechilgan mashqdan keyin commit qiling.
- `grep -rn` bilan MyOS'da `kmalloc` chaqirilgan 5 ta joyni toping va har biri nimaga xotira so'rayotganini aniqlang.
- `git log --oneline --reverse` — MyOS'ning birinchi 5 commitini `git show --stat` bilan ko'rib chiqing.

Keyingi bob: [20-bob. Sonlar kompyuterda](20-sonlar.md)
