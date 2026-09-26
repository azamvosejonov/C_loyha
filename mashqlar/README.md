# Mashqlar — C tilini noldan yadro darajasigacha

> Bu papka — [darslik](../darslik/README.md) bilan birga ishlatiladigan **amaliy mashqlar**.
> Har bir mashqda siz bitta faylni (`yechim.c`) yozasiz, avtomatik test esa natijani
> tekshiradi. Internetdan qidirish shart emas: vazifa sharti, Python'dagi o'xshashi,
> tuzoqlar va maslahatlar `yechim.c` ning boshida yozilgan.

## Qanday ishlash kerak

```bash
tools/mashq.py                 # ro'yxat: qaysilari o'tilgan, keyingisi qaysi
tools/mashq.py vazifa          # keyingi mashqning sharti
# mashqlar/NN_nom/yechim.c ni muharrirda ochib, TODO o'rniga kodingizni yozing
tools/mashq.py tekshir         # tekshirish (yoki: tools/mashq.py tekshir 07)
```

Kerak: `gcc` va `python3` (Linux yoki Windows'da WSL). QEMU va yadro kerak emas.

Har bir mashq papkasida:

| Fayl | Nima | O'zgartirasizmi |
|---|---|---|
| `yechim.c` | Vazifa sharti (boshidagi izohda) + siz yozadigan funksiyalar | **Ha** — faqat shu fayl |
| `mashq.h` | Funksiyalar e'loni (prototiplar) | Yo'q |
| `test.c` | Testlar | Yo'q (lekin o'qing — nima kutilayotgani aniq ko'rinadi) |

## Tekshiruvchi nimani ushlaydi

Kod `-Wall -Wextra -Werror` bilan (yadrodagi kabi — ogohlantirish = xato) va ikki
"qo'riqchi" ostida ishlaydi:

- **AddressSanitizer** — massiv chegarasidan chiqish, `free` qilingan xotiradan foydalanish,
  ikki marta `free`, xotira sizib chiqishi (leak).
- **UndefinedBehaviorSanitizer** — butun son toshishi, `1 << 31`, nolga bo'lish.

Xato topilsa, tekshiruvchi hisobotni ko'rsatadi va **o'zbekcha tushuntiradi**: bu qanday xato
va odatda nimadan kelib chiqadi. Hisobotdagi `#0 ... yechim.c:QATOR` — xato aynan shu qatorda.

## Qoidalar

1. **Ko'chirmang.** Yechimni internetdan yoki AI'dan olsangiz, test o'tadi, lekin siz o'rganmaysiz.
   Qotib qolsangiz: darslikdagi tegishli bobni qayta o'qing, `test.c` ni o'qing, qog'ozda chizing.
2. **Tartib bilan boring.** Har bir mashq oldingilariga tayanadi.
3. **O'tgandan keyin yaxshilang.** Test o'tdi — endi kodni qayta o'qing: soddaroq, tezroq
   qilish mumkinmi? Chegaraviy holatlarni o'zingiz qo'shing.
4. **Yadroga bog'lang.** Ko'p mashqlarda "MyOS'da: ..." havolasi bor — o'sha kodni oching va
   o'zingiz yozgan narsa bilan solishtiring.

## Mashqlar xaritasi

| # | Mashq | Darslik bobi | Nimani o'rgatadi | Yadroda qayerda kerak |
|---|---|---|---|---|
| | **1-modul: turlar va sikllar** | | | |
| 01 | Kvadratlar yig'indisi | 02–04 | `int`/`long`, `for`, toshish | hamma joyda |
| 02 | Tub sonlar | 03–04 | `bool`, `unsigned`, toshmaydigan shart | — |
| 03 | Toshishsiz arifmetika | 03, 13 | UB, `INT_MAX`, chegarani oldindan tekshirish | bufer hajmlarini hisoblash |
| 04 | Bitlar | 03, 16 | `& \| ^ ~ << >>`, tekislash | sahifa jadvali, registrlar |
| 05 | Massivlar | 06 | massiv + uzunlik, `size_t` tuzog'i | hamma joyda |
| | **2-modul: ko'rsatkichlar** | | | |
| 06 | Almashtirish | 07 | `&`, `*`, manzil orqali o'zgartirish | hamma joyda |
| 07 | Chiqish parametrlari | 05, 07 | ko'p natija, bufer hajmi | `stat`, `read`, `getdents` |
| 08 | Satrlar va ko'rsatkich arifmetikasi | 06–07 | `'\0'`, `p++`, `unsigned char` | `user/libc/string.c` |
| 09 | Xavfsiz nusxalash | 06, 13 | bufer to'lishi | `ksnprintf`, `getcwd` |
| 10 | Teskari satr, palindrom | 06–07 | ikki ko'rsatkich, `<ctype.h>` | — |
| 11 | Son → satr | 06, 12 | `printf`ning yuragi, `LONG_MIN` | `kernel/lib/kprintf.c` |
| 12 | Satr → son | 06, 13 | kirishni qat'iy tekshirish | yadro parametrlari, `kill` |
| | **3-modul: xotira** | | | |
| 13 | Dinamik massiv (list) | 08 | `malloc`/`realloc`/`free`, o'sish strategiyasi | — |
| 14 | Satr yasash | 08 | `+1` uchun `'\0'`, egalik | — |
| 15 | Split | 08 | `char **`, ikki darajali `free` | shell, `argv` |
| 16 | Bog'langan ro'yxat | 07–09 | `->`, use-after-free, `**pp` usuli | jarayonlar, navbatlar |
| 17 | Xesh jadval (dict) | 08–09 | opaque tur, rehash | inode keshi, PID jadvali |
| 18 | Xotira xatolarini topish | 08, 13 | sanitizer hisobotini o'qish | **debug** |
| | **4-modul: yadro uslubidagi C** | | | |
| 19 | Saralash va struct | 09 | merge sort, `qsort`, taqqoslash tuzog'i | — |
| 20 | Funksiya jadvali | 07, 09 | funksiya ko'rsatkichlari | VFS `file_ops`, drayverlar |
| 21 | Bitmap | 16 | bitlar xaritasi, `__builtin_ctzll` | sahifalar, ext2 bloklari |
| 22 | Halqa bufer | 06, 09 | FIFO, indekslar aylanishi | pipe, klaviatura, NVMe navbati |
| 23 | Ichki ro'yxat (`list_head`) | 07, 09, 10 | `container_of`, `offsetof` | Linux'ning har bir qismi |
| 24 | RPN kalkulyator | 06, 12 | parsing, stek | shell, ACPI AML |
| | **5-modul: tizim chaqiruvlari** | | | |
| 25 | Faylni nusxalash | 14 | `open/read/write/close`, qisman yozish | VFS |
| 26 | `wc` | 14 | holat mashinasi, bo'lak chegarasi | terminal emulyatori |
| 27 | `fork` + `exec` + `wait` | 14 | jarayonlar, chiqish holati | `proc/process.c`, `exec.c` |
| 28 | `pipe` + `dup2` | 14 | quvurlar, deadlock | `fs/pipe.c`, shell |
| 29 | Oqimlar va mutex | 15 | poyga holati | `lib/spinlock.c`, `mutex.c` |
| 30 | O'z `malloc`'ingiz | 08, 16 | bloklar, bo'lish, birlashtirish | `libc/malloc.c`, `mm/slab.c` |

**Keyingi bosqich:** 30 tasi tugagach — [labs/README.md](../labs/README.md) (MyOS yadrosining
ichida, xuddi shu usulda: funksiyani o'chirib, qayta yozasiz).
