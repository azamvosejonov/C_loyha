# Loyihalar: har bir bobda bitta tizim

Fikr oddiy: kod yozishni faqat **o'qib** o'rganib bo'lmaydi. Har bir bobning oxirida ikkita loyiha bor:

1. **Loyiha** — bobdagi vositalar bilan quriladigan kichik tizim. Men **ko'rsataman**: talab, ma'lumotlar tuzilmasi, funksiyalar
   ro'yxati, to'liq kod va ishga tushirilgan natija. Siz uni yozib, ishga tushirib, o'zgartirib ko'rasiz.
2. **Mustaqil loyiha** — boshqa tizim; uni **siz yozasiz**. Yechim berilmaydi: talab, kutilgan natija va maslahatlar (yechim emas) beriladi.
   Natijangizni `diff` bilan kutilgan natija bilan solishtirasiz.

Loyihalar sodda boshlanib, asta-sekin murakkablashadi: 0-bobda chek chop etishdan, 31-bobda `hexdump` yozishgacha. Ularning ko'pi
haqiqiy dasturlarning kichik nusxasi: `grep`, `wc`, `readelf`, kesh simulyatori, sahifa jadvali, fayl tizimi jurnali, rejalashtiruvchi.

## Har yangi loyihani shunday boshlang (7 qadam)

1. **Talabni o'qing** va kutilgan natijani diqqat bilan ko'zdan kechiring. Har qatorning **qayerdan** kelishini o'zingizga tushuntiring.
2. **Eng kichik misolni qo'lda hisoblang** (qog'ozda). Qo'lda hisoblay olmasangiz, kodda ham yoza olmaysiz.
3. **Ma'lumotlarni aniqlang:** qaysi qiymatlar o'zgaradi? `struct`, massiv yoki oddiy o'zgaruvchi?
4. **Funksiyalar ro'yxatini yozing** (nomi, nima oladi, nima qaytaradi) — kodsiz, faqat qog'ozda. Ular loyihaning "menyusi".
5. **Har funksiyani alohida yozing va darhol sinang** (`main` da bitta chaqiruv va `printf`). Hammasini birdaniga yozib, keyin
   xatoni qidirmang.
6. **Butun dasturni yig'ing** va kutilgan natija bilan `diff` qiling.
7. **Sanitizer bilan yig'ing** (`-fsanitize=address,undefined`) — natija to'g'ri bo'lsa ham, yashirin xato bo'lishi mumkin.

## Tekshirish

Har mustaqil loyihada tayyor buyruq bor. Umumiy shakli:

```bash
gcc -Wall -Wextra -g -fsanitize=address,undefined dasturim.c -o dastur
./dastur | diff - ~/C_loyha/darslik/loyihalar/NN_nomi/kutilgan.txt && echo "TO'G'RI"
```

- Buyruqlardagi `~/C_loyha` — repozitoriyni klon qilgan papka. Boshqa joyda bo'lsa, o'z yo'lingizni qo'ying.
- `diff` **hech narsa chiqarmasa** va `TO'G'RI` yozilsa — natija harfma-harf bir xil.
- Farq bo'lsa, `diff` qaysi qator boshqacha ekanini ko'rsatadi: `<` — sizniki, `>` — kutilgani.
- Ba'zi loyihalarda kirish fayli (`kirish.txt`) yoki bir necha sinov (`kutilgan_2.txt`, `kutilgan_3.txt`) bor — ular shu papkada.
- Chiqishdagi **probellar ham** hisobga olinadi. Format loyihada aniq yozilgan.

## Qotib qolsangiz

1. **30 daqiqa o'zingiz kurashing.** Qotib qolish — o'rganishning bir qismi.
2. `printf` bilan oraliq qiymatlarni chiqaring. Qog'ozdagi qo'lda hisoblash bilan solishtiring — qaysi qadamda farq boshlanadi?
3. `gdb` (29-bob): `break funksiya`, `next`, `print o'zgaruvchi`.
4. Bobning tegishli bo'limini qayta o'qing (maslahatlarda bo'lim raqamlari berilgan).
5. Yordam so'rang, lekin **yechimni emas**: "shu qatorda nima xato, faqat maslahat ber" (REJA.md, "AI'dan foydalanish").

## Vaqt

| Qiyinlik | Taxminiy vaqt |
|---|---|
| ★☆☆ | 30–60 daqiqa |
| ★★☆ | 1–2 soat |
| ★★★ | 2–4 soat (ba'zisi ko'proq — bu normal) |

## Loyihalar ro'yxati

| Bob | Loyiha (ko'rsatilgan) | Mustaqil loyiha (siz yozasiz) | Qiyinlik | Fayllar |
|---|---|---|---|---|
| [0](../00-kirish.md) | do'kon cheki | yo'l xarajati kalkulyatori | ★☆☆ | [`00_chek/`](00_chek/) |
| [1](../01-kompilyatsiya.md) | geometriya moduli | vaqt moduli | ★☆☆ | [`01_geometriya/`](01_geometriya/) |
| [2](../02-turlar.md) | turlar jadvali va toshish | video hajmi hisoblagichi | ★☆☆ | [`02_tur_jadvali/`](02_tur_jadvali/) |
| [3](../03-operatorlar.md) | Unix fayl ruxsatlari (chmod) | IP manzil hisoblagichi | ★★☆ | [`03_ip_hisoblagich/`](03_ip_hisoblagich/) |
| [4](../04-boshqaruv.md) | taxmin o'yini (ikkilik qidiruv) | oy kalendari | ★★☆ | [`04_kalendar/`](04_kalendar/) |
| [5](../05-funksiyalar.md) | sonlar laboratoriyasi | Hanoy minorasi va Paskal uchburchagi | ★★☆ | [`05_hanoy_paskal/`](05_hanoy_paskal/) |
| [6](../06-massivlar-satrlar.md) | ovoz berish natijalari | palindrom, anagram va so'zlar | ★★☆ | [`06_anagram/`](06_anagram/) |
| [7](../07-korsatkichlar.md) | map / filter / reduce | massivni joyida qayta ishlash | ★★★ | [`07_joyida/`](07_joyida/) |
| [8](../08-xotira.md) | o'suvchi satr (string builder) | qavslar tekshiruvchisi | ★★★ | [`08_qavslar/`](08_qavslar/) |
| [9](../09-struct.md) | geometriya — nuqta va to'g'ri to'rtburchak | poker qo'llari | ★★★ | [`09_poker/`](09_poker/) |
| [10](../10-preprotsessor.md) | mini test kutubxonasi | log tizimi va bit makrolari | ★★☆ | [`10_log_makro/`](10_log_makro/) |
| [11](../11-kop-fayl-make.md) | bank moduli (3 fayl + Makefile) | kitob moduli | ★★☆ | [`11_kitob_moduli/`](11_kitob_moduli/) |
| [12](../12-standart-kutubxona.md) | server log tahlilchisi | CSV baholar hisoboti | ★★★ | [`12_csv_hisobot/`](12_csv_hisobot/) |
| [13](../13-ub-xavfsizlik.md) | xavfsiz butun sonlar | `parse_uint` — Linux'ning `kstrtouint` i | ★★★ | [`13_parse_uint/`](13_parse_uint/) |
| [14](../14-tizim-chaqiruvlari.md) | `wc` — tizim chaqiruvlari bilan | mini `grep` | ★★★ | [`14_mini_grep/`](14_mini_grep/) |
| [15](../15-parallellik.md) | parallel yig'indi | deadlock'siz bank | ★★★ | [`15_bank_oqimlar/`](15_bank_oqimlar/) |
| [16](../16-bitlar-apparat.md) | virtual UART va uning drayveri | ma'lumot yaxlitligi — parity, CRC-8, Gray | ★★★ | [`16_crc_gray/`](16_crc_gray/) |
| [17](../17-assembly.md) | stekli virtual mashina | 8 bitli ALU va bayroqlar | ★★★ | [`17_alu/`](17_alu/) |
| [18](../18-yadroga-koprik.md) | `kprintf` — libc'siz formatlash | `kprintf` — kenglik va to'ldirish | ★★★ | [`18_kprintf/`](18_kprintf/) |
| [19](../19-terminal-git.md) | `mhead` — buyruq qatori argumentlari | mini `wc` | ★★★ | [`19_mini_wc/`](19_mini_wc/) |
| [20](../20-sonlar.md) | sonlar konvertori | Q16.16 qat'iy nuqtali kalkulyator | ★★★ | [`20_qat_nuqta/`](20_qat_nuqta/) |
| [21](../21-kesh.md) | to'g'ridan-to'g'ri xaritalangan kesh simulyatori | 2 yo'lli to'plamli LRU kesh | ★★★ | [`21_kesh_sim/`](21_kesh_sim/) |
| [22](../22-boglash.md) | ELF sarlavha o'quvchisi (mini `readelf -h`) | bo'limlar ro'yxati (`readelf -S`) | ★★★ | [`22_elf_bolimlar/`](22_elf_bolimlar/) |
| [23](../23-jarayonlar-scheduling.md) | Round Robin rejalashtiruvchi simulyatori | SRTF (eng qisqa qolgan vaqt birinchi) | ★★★ | [`23_srtf/`](23_srtf/) |
| [24](../24-virtual-xotira.md) | ikki darajali sahifa jadvali | VMA ro'yxati — `mmap`, `munmap`, `find_vma` | ★★★ | [`24_vma/`](24_vma/) |
| [25](../25-xotira-ajratish.md) | arena (bump) ajratuvchi | ajratish siyosatlari taqqoslash | ★★★ | [`25_fit_siyosat/`](25_fit_siyosat/) |
| [26](../26-parallellik-chuqur.md) | ulanishlar puli (semafor) | qayta ishlatiladigan to'siq (barrier) | ★★★ | [`26_tosiq/`](26_tosiq/) |
| [27](../27-fayl-tizimlari.md) | xotiradagi mini fayl tizimi | jurnal va avariyadan tiklash | ★★★ | [`27_jurnal/`](27_jurnal/) |
| [28](../28-algoritmlar.md) | so'z chastotasi (xesh jadval) | labirintdan chiqish yo'li (BFS) | ★★★ | [`28_labirint/`](28_labirint/) |
| [29](../29-debug-vositalari.md) | xotira sizib chiqishini ushlagich (mini leak detector) | 5 ta xatoni toping | ★★★ | [`29_xato_ovchisi/`](29_xato_ovchisi/) |
| [30](../30-yadro-arxitekturasi.md) | kooperativ mini yadro | taymer uzilishi va uxlash navbati | ★★★ | [`30_taymer_yadro/`](30_taymer_yadro/) |
| [31](../31-lugat.md) | atamalar qidiruvchisi | `hexdump` | ★★★ | [`31_hexdump/`](31_hexdump/) |

Bu papkalarda: `kutilgan.txt` (kutilgan natija), kerak bo'lsa `kutilgan_2.txt`, `kutilgan_3.txt`, kirish fayllari (`kirish.txt`, `a.txt`, `namuna.elf`...).
Mening yechimlarim bu yerda **yo'q** — xuddi [mashqlar](../../mashqlar/README.md) dagi kabi.

## Loyihalar ishlashini kim tekshiradi

`make lab-check` ko'rsatilgan (birinchi) loyihalarning hammasini yig'adi (`-Wall -Wextra` ogohlantirishlarisiz). Mustaqil loyihalarning
kutilgan natijalari mening yashirin yechimlarim bilan olingan va ular sanitizer ostida toza ishlaydi.
