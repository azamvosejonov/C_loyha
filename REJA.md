# 6 oylik reja: C'ni noldan boshlab yadroda mustaqil ishlashgacha

> Bu reja **to'liq kunlik** o'qish uchun: haftasiga 6 kun, kuniga 6–7 soat (jami ~950 soat).
> Kuniga 2–3 soat vaqtingiz bo'lsa — xuddi shu reja, lekin taxminan **15 oy**. Tartib o'zgarmaydi.
>
> Hamma material shu loyiha ichida: [darslik](darslik/README.md) (19 bob), [mashqlar](mashqlar/README.md)
> (40 ta, avtomatik tekshiruv bilan), `docs/` (MyOS yadrosi), [labs](labs/README.md) (22 ta),
> [QOLLANMA.md](QOLLANMA.md), [YAKUNIY.md](YAKUNIY.md). Internetdan qidirish shart emas.

## Halol kutilma

6 oydan keyin, reja bo'yicha ishlasangiz:

- ✅ C'ni erkin bilasiz: ko'rsatkichlar, xotira, bitlar, UB, tizim chaqiruvlari, oqimlar;
- ✅ yadroning asosiy mexanizmlarini (sahifa jadvali, buddy, slab, qulflar, scheduler, ELF, ext2, pipe,
  signallar) **o'zingiz yozgan** bo'lasiz — avval oddiy dastur sifatida, keyin MyOS ichida;
- ✅ MyOS kodini o'qiy olasiz va o'zgartira olasiz, yadroni gdb bilan debug qila olasiz;
- ✅ noldan yadro yozishni boshlagan bo'lasiz (yuklash, xotira, uzilishlar, jarayonlar);
- ⏳ hali yo'q: katta kod bazasidagi (Linux) tajriba, haqiqiy drayverlar, ishlab chiqarishdagi debug.
  Bular — keyingi 1–2 yil (YAKUNIY.md, V qism).

Bu — "junior tizim dasturchisi"ning mustahkam darajasi va yadro sohasiga eng to'g'ri kirish yo'li.

## Kundalik tartib

| Vaqt | Nima |
|---|---|
| 1–1.5 soat | **O'qish**: darslik bobi yoki `docs/` hujjati + tegishli kod. Misollarni qo'lda yozib ko'rish |
| 3–4 soat | **Yozish**: mashq yoki lab. Avval o'zingiz; qotib qolsangiz — 30 daqiqa kurashing, keyin darslikni qayta o'qing |
| 30 daqiqa | **Ingliz tili** (spetsifikatsiyalar, Linux, suhbatlar — hammasi inglizcha) |
| 30 daqiqa | **Daftar**: bugun nimani tushundim — o'z so'zlarim bilan. Tushunmaganlarim ro'yxati |

**Haftalik:** 6-kuni — hafta takrori: o'tilgan mashqlardan 2 tasini **yopib**, boshidan qayta yozish.
7-kun — dam olish (miya yangi bilimni aynan dam olishda mustahkamlaydi).

**Taraqqiyotni kuzatish:** `tools/mashq.py` (mashqlar), `tools/lab.py royxat` (lab'lar), `git log` (kodingiz).

## 1-oy: C asoslari va xotira (1–4-haftalar)

| Hafta | O'qish | Mashqlar | Hafta oxirida qila olishingiz kerak |
|---|---|---|---|
| 1 | Darslik 00–03 | 01, 02, 03, 04 | Dasturni kompilyatsiya qilish, xatoni o'qish, gdb'da qadamma-qadam yurish; bitlar bilan ishlash |
| 2 | Darslik 04–06 | 05, 08, 09, 11, 12 | Massiv/satr funksiyalarini yozish; `'\0'` va bufer hajmini doim hisobga olish |
| 3 | Darslik 07 (**3–4 kun**, eng muhim bob) | 06, 07, 10, 16 | Har qanday ko'rsatkich ifodasini qog'ozda chizib tushuntirish |
| 4 | Darslik 08 | 13, 14, 15, 17, 18 | `malloc`/`free` ni xatosiz ishlatish; sanitizer hisobotidan xatoni topish |

**1-nazorat** (kodga qaramasdan, 3 soat ichida): dinamik massiv, bog'langan ro'yxat (qo'shish, teskari,
o'chirish, ozod qilish) va satr `split` ni yozing — hammasi `-fsanitize=address` bilan toza.

## 2-oy: C to'liq va tizim dasturlash (5–8-haftalar)

| Hafta | O'qish | Mashqlar | Maqsad |
|---|---|---|---|
| 5 | Darslik 09–10 | 19, 20, 21, 22, 23, 24 | Struct/union/makrolar; `container_of` ni tushuntira olish |
| 6 | Darslik 11–13 | Ko'p faylli loyiha + Makefile (11-bob mashqi); 13-bob UB tajribalari | O'z Makefile'ingiz; UB'ni ko'rganda tanish |
| 7 | Darslik 14 | 25, 26, 27, 28 | `fork/exec/wait/pipe/dup2` — erkin |
| 8 | Darslik 15–17 | 29, 30; 17-bob assembly mashqlari | Poyga holatini tushuntirish; `gcc -S` chiqishini o'qish |

**2-nazorat:** o'z mini-shell'ingiz (kodga qaramasdan): buyruqlar, `|`, `>`, `<`, `cd`, `exit`.

## 3-oy: yadro mexanizmlari — oddiy dastur sifatida (9–12-haftalar)

| Hafta | O'qish | Mashqlar |
|---|---|---|
| 9 | Darslik 18; `docs/04-virtual-xotira.md` | 31 (sahifa jadvali), 32 (buddy) |
| 10 | `docs/03-fizik-xotira.md`, `docs/05-heap.md` | 33 (slab), 34 (spinlock), 35 (scheduler) |
| 11 | `docs/13-disk-ext2.md` (ext2 qismi) | 36 (ELF), 37 (ext2) |
| 12 | `docs/06-jarayonlar.md` | 38 (kutish navbati), 39 (kichik CPU), 40 (mini shell) |

**3-nazorat:** `tools/mashq.py hammasi` → 40/40. Keyin 32 (buddy) va 31 (sahifa jadvali) ni yopib, 2 soat
ichida qayta yozing.

## 4-oy: MyOS'ni o'qish va birinchi lab'lar (13–16-haftalar)

| Hafta | O'qish | Amaliyot |
|---|---|---|
| 13 | README, QOLLANMA 0–5-bo'limlar; `docs/00-kirish.md`–`02-uzilishlar.md` (v0.1: `git checkout e5906bb`) | `make run`, `make debug` + gdb bilan yuklanishni kuzatish |
| 14 | `docs/03`–`05` (fizik xotira, virtual xotira, heap) + kod | Har bir hujjatning "Sinab ko'ring" bo'limi |
| 15 | `docs/06`–`08` (jarayonlar, user rejimi, test/debug) + kod | Ataylab xato kiritib, gdb bilan topish |
| 16 | QOLLANMA 6–7-bo'limlar | Lab'lar 1–2-daraja: `strlen`, `memmove`, `strtok_r`, `strtoul`, `emit_number`, `gmtime_r`, `glob_match`, `malloc`, `insert_free`, `path_normalize`, `pipe` |

**4-nazorat:** `kernel/main.c` ni ochib, har bir `*_init()` nima qilishini va nega aynan shu tartibda
chaqirilishini og'zaki tushuntiring (o'zingizga yoki kimgadir).

## 5-oy: hozirgi yadro va qolgan lab'lar (17–20-haftalar)

| Hafta | O'qish (`git checkout -` bilan hozirgi versiyaga qayting) | Lab'lar |
|---|---|---|
| 17 | `docs/09-yangi-arxitektura.md`, `docs/10-smp.md` | `spinlock`, `buddy` |
| 18 | `docs/11-fork-cow.md` | `slab`, `fault_page`, `sleep_wakeup` |
| 19 | `docs/12-vfs.md`, `docs/13-disk-ext2.md` | `elf_load`, `block_path`, `bmap`, `dir_add` |
| 20 | `docs/14-signallar.md`, `docs/15-terminal.md` | `setup_frame`, `sigreturn` |

**5-nazorat:** `tools/lab.py royxat` — 22/22. `make test` — hammasi OK.

## 6-oy: noldan yadro + haqiqiy apparat (21–26-haftalar)

Yangi, bo'sh papkada, **MyOS'ga qaramasdan** ([QOLLANMA.md](QOLLANMA.md), 11-bo'lim). Qotib qolsangiz —
avval darslik, `docs/`, Intel qo'llanmasi; MyOS kodi — oxirgi chora.

| Hafta | Maqsad |
|---|---|
| 21 | Multiboot2 + GRUB yuklash, ekranga matn, serial port, `kprintf` |
| 22 | GDT, IDT, istisno ishlovchilari (page fault xabari bilan) |
| 23 | Fizik xotira (memblock → buddy), sahifalash, yuqori yarmi |
| 24 | Heap (`kmalloc`), taymer, klaviatura |
| 25 | Yadro oqimlari va kontekst almashish; **MyOS'ni haqiqiy kompyuterda yuklash** ([YAKUNIY.md](YAKUNIY.md), I qism) |
| 26 | User rejimi + birinchi syscall (agar ulgurmasangiz — keyingi oyga). Takror, xulosa, keyingi reja |

**6-nazorat (yakuniy):** o'z yadroingiz QEMU'da yuklanadi, page fault'ni ushlaydi, taymer va klaviatura
uzilishlari ishlaydi, `kmalloc` bor. Buni 6 oy oldin qila olmasdingiz.

## 6 oydan keyin

1. O'z yadroingizni davom ettirish: jarayonlar, syscall'lar, VFS, shell ([QOLLANMA.md](QOLLANMA.md) 11-bo'lim, qolgan qadamlar).
2. MyOS'ni kengaytirish: [YAKUNIY.md](YAKUNIY.md), IV qism — `/proc`, NVMe, USB klaviatura, tarmoq.
3. Linux: yadroni yig'ish, modul yozish, birinchi patch (YAKUNIY.md, 31-bo'lim).
4. Ingliz tili va algoritmlar — ishga topshirishdan 2–3 oy oldin maqsadli tayyorgarlik.

## Python haqida

Python'dan **voz kechish shart emas** — faqat unga yangi vaqt sarflamang. Python yadro dasturchisi uchun
ham foydali vosita: bu loyihaning o'zida `tools/lab.py`, `tools/mashq.py`, `tools/mkdisk.py` (ext2 disk
yaratish) — Python'da. Test skriptlari, log tahlili, disk tasvirlari — Python'da tez yoziladi.
Asosiy tilingiz — C, yordamchi tilingiz — Python va shell.

## AI'dan (menden) qanday foydalanish

O'rganish davrida **yechimni so'ramang** — test o'tadi, lekin siz o'rganmaysiz. So'rang:

- "Bu kodimda xato qayerda? Tuzatma, faqat qatorni ayt."
- "`volatile` nega kerak? Misol bilan tushuntir."
- "Menga maslahat ber, yechimni aytma."
- "7-bob bo'yicha menga 10 ta savol ber, javoblarimni bahola."
- "Kodimni Linux code review'dagi kabi qattiq tanqid qil."

## Qoidalar (qisqacha)

1. Har kuni kod yozing — o'qishning o'zi yetmaydi.
2. Tushunmasdan keyingisiga o'tmang.
3. Ko'chirmang — xotiradan yozing.
4. Har bir xatoni daftarga yozing: nima bo'ldi, nega, qanday topdim.
5. Har hafta bitta narsani "yopib, qayta yozing".
