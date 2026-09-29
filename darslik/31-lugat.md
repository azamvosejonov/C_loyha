# 31-bob. Lug'at: ingliz texnik atamalari va xabarlar

> Kod, kompilyator xabarlari, Linux va hujjatlar — hammasi ingliz tilida. Bu bob ingliz tilini bilmasdan
> ham ular bilan ishlash uchun: **atamalar**, **koddagi qisqartmalar**, **kompilyator va dastur xabarlari**
> tarjimasi. Kerak bo'lganda qidiring (`Ctrl-F`). Ingliz tilini asta-sekin o'rganish uchun ham eng
> foydali 300 so'z — shu yerda.

## Hayotdan misollar: so'zlarning asl ma'nosi

Inglizcha atamalarning ko'pi oddiy, kundalik so'zlardan olingan. Asl ma'nosini bilsangiz, atama
yodda o'z-o'zidan qoladi.

| Atama | So'zma-so'z | Nega shunday atalgan |
|---|---|---|
| **kernel** | yong'oq mag'zi | Tizimning ichki, eng muhim qismi. Uni o'rab turgan qism — **shell** (po'choq) |
| **shell** | po'choq | Yadroni o'rab turgan va foydalanuvchi bilan gaplashadigan qatlam |
| **boot** | etik | "Pull oneself up by one's bootstraps" — o'zini etigining ipidan tortib ko'tarish. Kompyuter o'zini o'zi ishga tushiradi |
| **fork** | ayri, vilka; yo'l ayrilishi | Bitta jarayon ikkiga ayriladi — xuddi yo'l ayrilgandek |
| **pipe** | quvur | Bir tomondan kiradi, boshqa tomondan chiqadi |
| **thread** | ip | Bitta dastur ichidagi bajarilishning alohida "ipi" |
| **lock** | qulf | Eshikni qulflash kabi — bir vaqtda bitta kishi |
| **stack** | taxlam | Likopchalar taxlami |
| **heap** | uyum | Tartibsiz uyum — bo'sh joy istalgan joydan olinadi |
| **cache** | yashirin zaxira | Fransuzcha "yashirmoq": tez olish uchun yaqinda yashirib qo'yilgan narsa |
| **page** | sahifa | Xotira kitob kabi teng sahifalarga bo'lingan |
| **bus** | avtobus (shina) | Ma'lumotlarni hamma "bekatlarga" tashiydigan umumiy yo'l |
| **bug** | qo'ng'iz | 1947-yilda Harvard Mark II kompyuterining relesiga haqiqiy kuya qamalib qolgan va u jurnalga yopishtirib qo'yilgan |
| **patch** | yamoq | Kiyimdagi teshikni yamash kabi — kodning kichik tuzatmasi |
| **daemon** | ruh, jin (yunoncha: yordamchi ruh) | Ko'rinmasdan, fonda ishlab yuradigan yordamchi dastur |
| **zombie** | tirik o'lik | Jarayon tugagan (o'lgan), lekin hali ro'yxatda "yuribdi" |
| **orphan** | yetim | Otasi tugagan jarayon — uni `init` "asrab oladi" |
| **spin** | aylanmoq | Qulf bo'shashini aylanib-aylanib kutish |
| **dump** | to'kib tashlamoq | Xotiraning hammasini faylga "to'kib" tashlash (core dump) |
| **flush** | yuvib yubormoq | Bufer ichidagini bir yo'la chiqarib yuborish |
| **hook** | ilgak | Boshqa kod "ilinishi" mumkin bo'lgan joy |
| **handler** | ishlovchi, boshqaruvchi | Hodisa kelganda uni "qo'lga oladigan" funksiya |
| **mount** | minmoq, o'rnatmoq | Diskni katalog daraxtiga "mindirish" |
| **sandbox** | qumdon | Bolalar faqat qumdon ichida o'ynaydi — tashqariga zarar yo'q |
| **firmware** | "qattiq" dastur | Apparatga yozib qo'yilgan, kam o'zgaradigan dastur (hard + soft oralig'i) |

**Sinab ko'ring:** 31.2-bo'limdagi atamalardan 10 tasini tanlab, har biri uchun o'zingizning hayotiy
misolingizni o'ylab toping va daftaringizga yozing. O'zingiz o'ylab topgan o'xshatish boshqasinikidan
yaxshiroq eslab qolinadi.

## 31.1. Koddagi qisqartmalar (nomlarda tez-tez uchraydi)

| Qisqartma | To'liq | Ma'nosi |
|---|---|---|
| `alloc` | allocate | ajratish (xotira) |
| `free` | free | ozod qilish, bo'shatish |
| `init` | initialize | boshlash, boshlang'ich holatga keltirish |
| `buf` | buffer | bufer, vaqtinchalik xotira |
| `len` | length | uzunlik |
| `sz`, `size` | size | hajm |
| `cnt`, `count` | count | soni |
| `idx` | index | indeks, tartib raqami |
| `ptr` | pointer | ko'rsatkich |
| `src` / `dst` | source / destination | manba / nishon |
| `tmp` | temporary | vaqtincha |
| `ret` | return (value) | qaytish qiymati |
| `err` | error | xato |
| `ctx` | context | kontekst, holat |
| `prev` / `next` | previous / next | oldingi / keyingi |
| `cur`, `curr` | current | joriy |
| `max` / `min` | maximum / minimum | eng katta / eng kichik |
| `num` | number | son |
| `str` | string | satr |
| `arg`, `argv`, `argc` | argument (vector, count) | argument (massivi, soni) |
| `fd` | file descriptor | fayl deskriptori |
| `pid` | process ID | jarayon raqami |
| `uid` / `gid` | user / group ID | foydalanuvchi / guruh raqami |
| `mm` | memory management | xotira boshqaruvi |
| `vm`, `vma` | virtual memory (area) | virtual xotira (hududi) |
| `pte` | page table entry | sahifa jadvali yozuvi |
| `phys`, `virt` | physical, virtual | fizik, virtual |
| `addr` | address | manzil |
| `off`, `offset` | offset | siljish |
| `irq` | interrupt request | uzilish so'rovi |
| `isr` | interrupt service routine | uzilish ishlovchisi |
| `dev` | device | qurilma |
| `drv` | driver | drayver |
| `blk` | block | blok |
| `sb` | superblock | superblok |
| `ino` | inode | inode |
| `dir` | directory | papka |
| `ops`, `fops` | operations (file operations) | amallar jadvali |
| `priv` | private | shaxsiy (drayverning o'z ma'lumoti) |
| `ref`, `refcount` | reference (count) | havola (sanog'i) |
| `lock` / `unlock` | lock | qulflash / ochish |
| `sem` | semaphore | semafor |
| `mtx`, `mutex` | mutual exclusion | o'zaro istisno qulfi |
| `cb` | callback | qayta chaqiriladigan funksiya |
| `cfg`, `conf` | configuration | sozlama |
| `ver` | version | versiya |
| `hdr` | header | sarlavha |
| `msg` | message | xabar |
| `req` / `resp` | request / response | so'rov / javob |
| `rx` / `tx` | receive / transmit | qabul qilish / yuborish |
| `wr` / `rd` | write / read | yozish / o'qish |
| `en`, `enable` / `dis`, `disable` | enable / disable | yoqish / o'chirish |
| `sched` | scheduler | rejalashtiruvchi |
| `proc` | process | jarayon |
| `thr`, `thread` | thread | oqim |
| `sig` | signal | signal |
| `fs` | file system | fayl tizimi |
| `ctl` | control | boshqaruv |
| `stat` | status / statistics | holat / statistika |
| `util` | utility | yordamchi |
| `impl` | implementation | amalga oshirish |
| `dbg` | debug | debug, xato izlash |

## 31.2. Asosiy atamalar (alifbo tartibida)

| Inglizcha | O'zbekcha | Izoh / bob |
|---|---|---|
| abstraction | abstraksiya | murakkablikni yashiruvchi oddiy interfeys |
| address space | manzil maydoni | jarayon ko'radigan virtual xotira (24) |
| alignment | tekislash | manzil N ga karrali (9, 16) |
| allocator | ajratuvchi | xotira beruvchi kod (25) |
| argument | argument | funksiyaga beriladigan qiymat |
| array | massiv | (6) |
| assembly | assembler tili | (17) |
| assertion | tasdiq | "bu rost bo'lishi shart" tekshiruvi (12) |
| atomic | atomik, bo'linmas | (15, 26) |
| backtrace | chaqiruvlar zanjiri | gdb `bt` (29) |
| barrier | to'siq | xotira tartibini saqlash (15) |
| binary | ikkilik; bajariladigan fayl | (20) |
| bit / byte | bit / bayt | 8 bit = 1 bayt |
| bitmap | bitlar xaritasi | (16, 21-mashq) |
| block | blok | disk yoki xotira bo'lagi |
| boot / bootloader | yuklash / yuklovchi | GRUB (18) |
| branch | tarmoq; sakrash | git / CPU |
| breakpoint | to'xtash nuqtasi | gdb (1, 29) |
| buffer overflow | bufer to'lishi | (6, 13) |
| bug | xato | |
| build | yig'ish | kompilyatsiya + bog'lash |
| cache | kesh | (21) |
| cache line | kesh qatori | 64 bayt (21) |
| call stack | chaqiruvlar steki | (5, 17) |
| callee / caller | chaqiriluvchi / chaqiruvchi | |
| cast | tur o'zgartirish | `(int)x` (2) |
| clone | klonlash; nusxa | git clone |
| commit | commit (saqlash) | git; tranzaksiyani tasdiqlash |
| compiler | kompilyator | (1) |
| concurrency | parallellik | (15, 26) |
| condition variable | shart o'zgaruvchisi | (26) |
| context switch | kontekst almashish | (23) |
| copy-on-write (COW) | yozishda nusxalash | (24) |
| core | yadro (CPU); core dump | |
| critical section | kritik seksiya | (15) |
| deadlock | o'zaro qotish | (15, 26) |
| declaration / definition | e'lon / ta'rif | (1, 5) |
| dereference | ko'rsatkich orqali murojaat | `*p` (7) |
| descriptor | deskriptor | fd; GDT yozuvi |
| device | qurilma | |
| directory | papka, katalog | |
| dirty | "iflos" (o'zgartirilgan) | diskka yozilmagan (24) |
| driver | drayver | (27) |
| dynamic linking | dinamik bog'lash | (22) |
| endianness | bayt tartibi | (16, 20) |
| entry point | kirish nuqtasi | `main`, `_start` |
| exception | istisno | CPU: #PF, #GP (17, 24) |
| executable | bajariladigan fayl | |
| fault | xato; istisno | page fault |
| file system | fayl tizimi | (27) |
| flag | bayroq | bitli sozlama |
| fork | ayirish | jarayon nusxasi (14) |
| fragmentation | fragmentatsiya | (25) |
| frame | kadr; freym | stek kadri; fizik sahifa |
| freestanding | mustaqil (OS'siz) | (18) |
| handler | ishlovchi | uzilish/signal ishlovchisi |
| hash table | xesh jadval | (28) |
| header | sarlavha | `.h` fayl; paket sarlavhasi |
| heap | heap (dinamik xotira); uyum | (8, 28) |
| hosted | OS ustida | (18) |
| implementation | amalga oshirish | |
| inode | inode | fayl metama'lumoti (27) |
| instruction | buyruq (CPU) | |
| interrupt | uzilish | (16, 23) |
| kernel | yadro | |
| kernel mode / user mode | yadro rejimi / user rejimi | 0-halqa / 3-halqa |
| latency | kechikish | |
| leak | sizib chiqish | xotira/fd (8) |
| library | kutubxona | (11) |
| linker | bog'lovchi | (1, 22) |
| lock | qulf | (15) |
| lock-free | qulfsiz | (26) |
| macro | makro | (10) |
| memory-mapped I/O | xotiraga xaritalangan kiritish-chiqarish | (16) |
| mount | ulash (fayl tizimini) | |
| mutex | mutex (o'zaro istisno) | (15) |
| namespace | nomlar maydoni | |
| null pointer | nol ko'rsatkich | (7) |
| object file | obyekt fayl | `.o` (1, 22) |
| offset | siljish | |
| overflow | toshish | (2, 20) |
| page | sahifa | 4 KB (24) |
| page fault | sahifa xatosi | (24) |
| page table | sahifa jadvali | (24, 31-mashq) |
| panic | panika (halokatli xato) | yadro to'xtashi |
| parameter | parametr | |
| parser / parsing | tahlilchi / tahlil | (24-mashq) |
| patch | tuzatma (o'zgarishlar fayli) | (30) |
| permission | ruxsat | |
| pipe | quvur | (14) |
| pointer | ko'rsatkich | (7) |
| polling | so'rab turish | (27) |
| preemption | majburiy to'xtatish | (23) |
| preprocessor | preprotsessor | (10) |
| priority | ustuvorlik | |
| process | jarayon | (14, 23) |
| race condition | poyga holati | (15) |
| register | registr | (17) |
| relocation | relokatsiya | (22) |
| repository | repozitoriy (ombor) | git (19) |
| return value | qaytish qiymati | |
| root | ildiz; administrator | `/`; root foydalanuvchi |
| runtime | ish vaqti | |
| scheduler | rejalashtiruvchi | (23) |
| scope | ko'rinish sohasi | (2, 4) |
| section / segment | bo'lim / segment | ELF (22) |
| semaphore | semafor | (26) |
| shell | shell (buyruq qobig'i) | (14, 19) |
| signal | signal | (14) |
| signed / unsigned | ishorali / ishorasiz | (2, 20) |
| spinlock | aylanuvchi qulf | (15) |
| stack | stek | (5, 17) |
| starvation | och qolish | (23, 26) |
| statement | buyruq (C'da) | `;` bilan tugaydi |
| static | statik | (5, 8, 11) |
| struct | struktura | (9) |
| symbol | belgi (nom) | (22) |
| syscall (system call) | tizim chaqiruvi | (14) |
| thread | oqim | (15) |
| throughput | o'tkazuvchanlik | |
| timer | taymer | |
| TLB | manzil tarjimasi keshi | (21) |
| trap | tuzoq; ushlash (istisno) | |
| two's complement | ikkiga to'ldirish | (20) |
| undefined behavior | aniqlanmagan xatti-harakat | (13) |
| union | birlashma | (9) |
| user space | foydalanuvchi maydoni | |
| virtual memory | virtual xotira | (24) |
| volatile | o'zgaruvchan (keshlanmaydigan) | (16) |
| wait queue | kutish navbati | (26, 38-mashq) |
| warning | ogohlantirish | |
| zombie | zombi jarayon | (14) |

## 31.3. Kompilyator (gcc) xabarlari tarjimasi

| Xabar | Tarjima | Odatda nima qilish kerak |
|---|---|---|
| `expected ';' before 'X'` | X dan oldin `;` kutilgan edi | bir qator yuqorida `;` qo'ying |
| `expected ')' before ...` | `)` kutilgan | qavslarni sanang |
| `expected declaration or statement at end of input` | fayl oxirida e'lon yoki buyruq kutilgan | `}` yetishmayapti |
| `'x' undeclared (first use in this function)` | `x` e'lon qilinmagan | nom xato yozilgan yoki e'lon yo'q |
| `implicit declaration of function 'f'` | `f` funksiyasi e'lonsiz ishlatilgan | `#include` qo'shing |
| `undefined reference to 'f'` | `f` ga havola aniqlanmagan (linker) | `.c`/kutubxona ulanmagan |
| `multiple definition of 'x'` | `x` bir necha marta ta'riflangan | `.h` da ta'rif bor — `extern` qiling |
| `conflicting types for 'f'` | `f` uchun turlar zid | e'lon va ta'rif mos emas |
| `incompatible pointer type` | mos kelmaydigan ko'rsatkich turi | tur yoki `&` ni tekshiring |
| `makes integer from pointer without a cast` | ko'rsatkichdan son yasaldi | `*` yoki `&` unutilgan |
| `assignment to 'x' from incompatible pointer type` | | |
| `passing argument 1 of 'f' makes pointer from integer` | 1-argument: sondan ko'rsatkich yasaldi | `&x` kerak bo'lishi mumkin |
| `too few / too many arguments to function` | argument kam / ko'p | |
| `control reaches end of non-void function` | qiymat qaytaruvchi funksiya oxiriga `return` siz yetib keldi | `return` qo'shing |
| `unused variable 'x'` / `unused parameter 'x'` | `x` ishlatilmagan | o'chiring yoki `(void)x;` |
| `'x' is used uninitialized` | `x` boshlang'ich qiymatsiz ishlatilgan | qiymat bering |
| `comparison of integer expressions of different signedness` | ishorali va ishorasiz taqqoslanmoqda | turlarni moslang (2-bob) |
| `suggest parentheses around assignment used as truth value` | shartda `=` ishlatilgan | `==` kerakmi? |
| `format '%d' expects argument of type 'int', but argument 2 has type 'long'` | format va tur mos emas | `%ld` |
| `array subscript is above array bounds` | indeks massiv chegarasidan katta | |
| `this 'if' clause does not guard...` (misleading indentation) | chekinish aldamchi | `{ }` qo'ying (4-bob) |
| `function returns address of local variable` | lokal o'zgaruvchi manzili qaytarilmoqda | heap yoki chaqiruvchi buferi (5, 8) |
| `dereferencing pointer to incomplete type` | to'liq ta'riflanmagan turga murojaat | struct ta'rifi ko'rinmaydi (`#include`) |
| `storage size of 'x' isn't known` | `x` hajmi noma'lum | xuddi shu |
| `"/*" within comment` | izoh ichida `/*` | (10-bob) |
| `missing separator` (make) | ajratgich yo'q | Makefile'da TAB o'rniga bo'shliq |
| `No such file or directory` | bunday fayl yoki papka yo'q | yo'lni tekshiring |
| `Permission denied` | ruxsat berilmadi | `chmod +x` yoki `sudo` |
| `command not found` | buyruq topilmadi | o'rnatilmagan yoki `./` unutilgan |

## 31.4. Dastur ishlaganda chiqadigan xabarlar

| Xabar | Tarjima | Sabab |
|---|---|---|
| `Segmentation fault (core dumped)` | segmentatsiya xatosi | ruxsatsiz xotiraga murojaat (7, 8) |
| `Bus error` | shina xatosi | tekislanmagan murojaat yoki `mmap` fayl chegarasi |
| `Floating point exception` | kasr son istisnosi | aslida ko'pincha butun **nolga bo'lish** |
| `Aborted` | to'xtatildi | `abort()`, `assert` buzildi |
| `Killed` | o'ldirildi | SIGKILL (ko'pincha xotira tugadi — OOM) |
| `double free or corruption` | ikki marta free yoki buzilish | (8) |
| `free(): invalid pointer` | noto'g'ri ko'rsatkich free qilindi | |
| `malloc(): corrupted top size` | heap buzilgan | oldingi chegaradan chiqish |
| `stack smashing detected` | stek buzilishi aniqlandi | stekdagi bufer to'ldi |
| `Broken pipe` | uzilgan quvur | o'quvchisi yo'q pipe'ga yozish (SIGPIPE) |
| `Resource temporarily unavailable` | resurs vaqtincha mavjud emas | EAGAIN |
| `Interrupted system call` | tizim chaqiruvi uzildi | EINTR (signal) |
| `Too many open files` | ochiq fayllar juda ko'p | fd sizib chiqishi |
| `Bad file descriptor` | noto'g'ri fayl deskriptori | yopilgan fd ishlatildi |
| `Address already in use` | manzil band | port boshqa dasturda |
| `Device or resource busy` | qurilma band | ulangan diskni ajratish |

AddressSanitizer xabarlari — `tools/mashq.py` ularni o'zi tushuntiradi (8-bob, 18-mashq).

## 31.5. Git xabarlari

| Xabar | Tarjima / ma'nosi |
|---|---|
| `nothing to commit, working tree clean` | commit qiladigan narsa yo'q, hammasi saqlangan |
| `Changes not staged for commit` | o'zgarishlar bor, lekin `git add` qilinmagan |
| `Untracked files` | git kuzatmaydigan yangi fayllar |
| `Your branch is ahead of 'origin/main' by 2 commits` | sizda GitHub'da yo'q 2 ta commit bor — `git push` |
| `Your branch is behind ... can be fast-forwarded` | GitHub'da yangiliklar bor — `git pull` |
| `CONFLICT (content): Merge conflict in fayl.c` | ikki o'zgarish to'qnashdi — faylda `<<<<<<<` belgilarni topib, qo'lda hal qiling |
| `detached HEAD` | tarmoqsiz eski commitdasiz — `git checkout main` bilan qayting |
| `rejected ... (fetch first)` | push rad etildi — avval `git pull` |

## 31.6. Kodda ko'p uchraydigan so'zlar (izohlar va nomlar)

**Fe'llar:** get (olish), set (o'rnatish), add (qo'shish), remove/del (o'chirish), create (yaratish),
destroy (yo'q qilish), open/close (ochish/yopish), read/write (o'qish/yozish), send/recv (yuborish/qabul),
find/lookup/search (topish/qidirish), insert (qo'yish), update (yangilash), check/validate (tekshirish),
handle (ishlov berish), map/unmap (xaritalash/olib tashlash), load/store (yuklash/saqlash),
flush (tozalab yozish), reset (qayta boshlash), wait (kutish), wake (uyg'otish), sleep (uxlash),
yield (navbatni berish), spawn (yangi jarayon yaratish), exit (chiqish), return (qaytish),
copy (nusxalash), move (ko'chirish), fill (to'ldirish), clear (tozalash), parse (tahlil qilish),
dump (to'kib chiqarish, ko'rsatish), trace (kuzatish), probe (tekshirib topish — drayverda), register (ro'yxatdan o'tkazish).

**Sifatlar/holatlar:** valid/invalid (yaroqli/yaroqsiz), empty/full (bo'sh/to'la), busy/idle (band/bo'sh),
ready (tayyor), pending (kutilayotgan), active (faol), enabled/disabled (yoqilgan/o'chirilgan),
present (mavjud), missing (yetishmayotgan), dirty/clean (o'zgargan/toza), shared/private (umumiy/shaxsiy),
readonly (faqat o'qish), writable (yozish mumkin), locked (qulflangan), aligned (tekislangan),
unused (ishlatilmagan), deprecated (eskirgan), legacy (eski, meros), generic (umumiy), default (sukut bo'yicha).

**Izohlardagi iboralar:** `TODO` (qilish kerak), `FIXME` (tuzatish kerak), `XXX` (diqqat, shubhali joy),
`HACK` (vaqtinchalik yechim), `NOTE` (eslatma), `must` (shart), `should` (kerak), `may` (mumkin),
`never` (hech qachon), `always` (doim), `assume` (faraz qilamiz), `caller` (chaqiruvchi),
`on success / on failure` (muvaffaqiyatda / xatoda), `returns` (qaytaradi), `if any` (agar bo'lsa).

## 31.7. Ingliz tilini asta-sekin o'rganish maslahati

Bu darslik bilan ingliz tili shart emas. Lekin kelajakda (Linux, ish, spetsifikatsiyalar) kerak bo'ladi.
Eng samarali yo'l — **texnik ingliz tili**, umumiy emas:
1. Shu bobdagi so'zlarni har kuni 10 tadan takrorlang — ular kodda doim uchraydi.
2. Kod o'qiganda nomlarni ovoz chiqarib tarjima qiling: `alloc_pages` → "sahifalarni ajrat".
3. Kompilyator xabarlarini tarjima jadvalisiz tushunishga harakat qiling.
4. Keyinchalik: Linux `Documentation/` dan qisqa hujjatlarni lug'at bilan o'qish.

Kuniga 20–30 daqiqa — REJA.md dagi kundalik tartibda shunga joy qoldirilgan.

<!-- loyiha:boshi -->
## Loyiha: atamalar qidiruvchisi

**Maqsad:** saralangan ma'lumotdan **ikkilik qidiruv** bilan tez topish, va prefiks bo'yicha qidirish. Bu lug'at (31-bob) uchun ham, yadro ichida
saralangan jadvallar (`bsearch`, belgilar jadvali, `extable`) uchun ham ishlaydi (28.4).
**Bobdan ishlatiladi:** `struct` massivi, `bsearch`, `strcasecmp` (katta-kichik harfga sezgir emas solishtirish), prefiks tekshirish.

**Talab:** alifbo bo'yicha **saralangan** atamalar massivi. `qidir(nom)` — katta-kichik harfni farqlamasdan aniq nom bo'yicha; `prefiks(p)` — `p`
bilan boshlanuvchi hamma atamalar.
**Muhim:** `bsearch` faqat **saralangan** massivda to'g'ri ishlaydi — massiv tartibi `strcasecmp` bilan mos bo'lishi shart.

```c
/* lugat.c - atamalar qidiruvchisi */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

struct atama {
    const char *nom;
    const char *izoh;
};

static const struct atama lugat[] = {           /* alifbo tartibida! */
    { "bug", "dasturdagi xato" },
    { "cache", "tez yashirin xotira" },
    { "daemon", "fonda ishlovchi dastur" },
    { "fork", "jarayon nusxasini yaratish" },
    { "heap", "dinamik xotira sohasi" },
    { "kernel", "operatsion tizim yadrosi" },
    { "mutex", "o'zaro istisno qulfi" },
    { "pipe", "jarayonlar orasidagi quvur" },
    { "shell", "buyruq qobig'i" },
    { "stack", "chaqiruvlar steki" },
    { "thread", "bajarilish ipi" },
    { "zombie", "tugagan, lekin kutilmagan jarayon" },
};
#define SONI (sizeof(lugat) / sizeof(lugat[0]))

static int taqqosla(const void *kalit, const void *elem)
{
    return strcasecmp((const char *)kalit, ((const struct atama *)elem)->nom);
}

static void qidir(const char *nom)
{
    const struct atama *a = bsearch(nom, lugat, SONI, sizeof(lugat[0]), taqqosla);
    if (a)
        printf("%-8s -> %s\n", a->nom, a->izoh);
    else
        printf("%-8s -> topilmadi\n", nom);
}

static void prefiks(const char *p)
{
    printf("'%s' bilan boshlanuvchilar:", p);
    int topildi = 0;
    for (size_t i = 0; i < SONI; i++)
        if (strncasecmp(lugat[i].nom, p, strlen(p)) == 0) {
            printf(" %s", lugat[i].nom);
            topildi++;
        }
    printf("%s\n", topildi ? "" : " (yo'q)");
}

int main(void)
{
    qidir("kernel");
    qidir("MUTEX");                             /* katta harf ham topiladi */
    qidir("zombie");
    qidir("linux");
    prefiks("s");
    prefiks("TH");
    prefiks("x");
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address,undefined lugat.c -o lugat
$ ./lugat
kernel   -> operatsion tizim yadrosi
mutex    -> o'zaro istisno qulfi
zombie   -> tugagan, lekin kutilmagan jarayon
linux    -> topilmadi
's' bilan boshlanuvchilar: shell stack
'TH' bilan boshlanuvchilar: thread
'x' bilan boshlanuvchilar: (yo'q)
```

12 atamada ko'pi bilan 4 solishtirish (2⁴ = 16 ≥ 12) kifoya; million atamada ham 20 ta. Ketma-ket qidirish esa o'rtacha yarmini ko'rar edi.
Massivni saralamasdan `bsearch` chaqirilsa — natija **noto'g'ri** (yoki topmaydi), lekin xato ham bermaydi: bunday xatolar eng ayyor.

**Kengaytiring:** `"stack"` va `"shell"` o'rnini almashtirib (saralanmagan qilib) `qidir("thread")` ni chaqiring. Prefiks qidiruvini `bsearch` kabi tezlashtiring
(birinchi mosini ikkilik qidirish bilan toping, keyin oldinga yuring).

## Mustaqil loyiha: `hexdump` ★★★

**Vazifa:** yadro dasturchisi har kuni **baytlarni ko'zi bilan ko'radi**: disk tasviri, tarmoq paketi, ELF fayl, xotira dumpi. Buning uchun `hexdump -C`
(yoki `xxd`) kerak. Siz shunday vositani yozing: standart kirishdan baytlarni o'qib, `hexdump -C -v` formatida chiqaring. Fayl: `hexdump.c`.

**Format (aniq).** Har qator — 16 bayt:

```text
00000000  53 61 6c 6f 6d 2c 20 79  61 64 72 6f 21 0a 00 01  |Salom, yadro!...|
```

1. **Offset:** 8 xonali kichik harfli o'n oltilik (`%08x`), keyin **ikki probel**.
2. **Baytlar:** har bayt `%02x` va bitta probel. **8-baytdan keyin qo'shimcha bitta probel** (ya'ni ikkita guruh orasida ikki probel).
3. Oxirgi qator to'liq bo'lmasa, yetishmagan baytlar o'rniga **3 ta probel** (`"   "`) qo'yiladi (8-baytdan keyingi qo'shimcha probel ham saqlanadi).
4. So'ng **bitta probel**, `|`, ASCII ko'rinishi, `|`. ASCII: kodi 32..126 — belgining o'zi, qolganlari — `.`. Oxirgi qatorda faqat mavjud baytlar.
5. Hamma qatordan keyin **oxirgi qator**: jami baytlar soni `%08x` ko'rinishida (bo'sh kirish uchun `00000000`).
6. Takrorlanuvchi qatorlar `*` bilan **qisqartirilmaydi** (bu `hexdump -v` kabi).

**Kirish fayli** (`darslik/loyihalar/31_hexdump/kirish.bin`, 56 bayt: matn + 0x00–0x13 baytlar + matn + `ff fe 7f 80`).

**1-sinov:** `./dastur < kirish.bin`

```text
00000000  53 61 6c 6f 6d 2c 20 79  61 64 72 6f 21 0a 00 01  |Salom, yadro!...|
00000010  02 03 04 05 06 07 08 09  0a 0b 0c 0d 0e 0f 10 11  |................|
00000020  12 13 48 65 78 20 64 75  6d 70 20 74 65 73 74 20  |..Hex dump test |
00000030  30 31 32 33 ff fe 7f 80                           |0123....|
00000038
```

**2-sinov:** bo'sh kirish — `./dastur < /dev/null`

```text
00000000
```

**3-sinov:** aniq 16 bayt — `printf 'ABCDEFGHIJKLMNOP' | ./dastur`

```text
00000000  41 42 43 44 45 46 47 48  49 4a 4b 4c 4d 4e 4f 50  |ABCDEFGHIJKLMNOP|
00000010
```

**Maslahat** (yechim emas):
- Kirishni `fread(bufer, 1, 16, stdin)` bilan 16 baytdan o'qing — qaytgan son `n` (oxirida 16 dan kam bo'lishi mumkin, 0 bo'lsa tugadi).
- Bo'lakni chiqarish: `for i in 0..15`: `i < n` bo'lsa `"%02x "`, aks holda `"   "`; `i == 7` dan keyin qo'shimcha `" "`. Keyin `" |"`...
- Baytni `unsigned char` sifatida qayta ishlang: `isprint((unsigned char)c)` yoki `c >= 32 && c <= 126`.
- Jami baytlarni yig'ing va oxirida `%08x` bilan chiqaring.
- Sinab ko'rish: sizda `hexdump -C -v fayl` yoki `xxd fayl` bo'lsa, natijani solishtiring; yo'q bo'lsa `od -A x -t x1z -v fayl` ham o'xshash (format biroz boshqacha).
- Bu dastur bilan MyOS disk tasviri, ELF sarlavhasi (22-bob) va o'z `.o` fayllaringizga qarang: `./dastur < namuna.elf | head`.

**Tekshirish:**

```bash
D=~/C_loyha/darslik/loyihalar/31_hexdump
gcc -Wall -Wextra -g -fsanitize=address,undefined hexdump.c -o dastur
./dastur < $D/kirish.bin | diff - $D/kutilgan.txt && echo "1: TO'G'RI"
./dastur < /dev/null | diff - $D/kutilgan_2.txt && echo "2: TO'G'RI"
printf 'ABCDEFGHIJKLMNOP' | ./dastur | diff - $D/kutilgan_3.txt && echo "3: TO'G'RI"
```
<!-- loyiha:oxiri -->
