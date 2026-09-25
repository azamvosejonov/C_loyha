# 15 — Terminal: VT100 emulyatori, klaviatura, qator muharriri va `edit`

Kod: `kernel/drivers/{vt,fbcon,vga,keyboard,tty}.c`, `kernel/drivers/font8x16.c` (`tools/psf2c.py`),
`user/bin/sh.c` (`edit_line`), `user/bin/edit.c`

## 1. Terminal — bu "til"

Dastur ekranni to'g'ridan-to'g'ri boshqarmaydi, u faqat **baytlar** yozadi. Rang, kursor va tozalash
buyruqlari matn orasiga qo'shilgan **escape ketma-ketliklari** orqali beriladi:

| ketma-ketlik | ma'nosi |
|---|---|
| `ESC[2J` | butun ekranni tozalash |
| `ESC[5;10H` | kursorni 5-qator, 10-ustunga qo'yish |
| `ESC[K` | qatorning qolganini o'chirish |
| `ESC[1;32m` / `ESC[0m` | qalin yashil / oddiy rang |
| `ESC[?25l` / `ESC[?25h` | kursorni yashirish / ko'rsatish |
| `ESC[?1049h` | muqobil ekran (muharrir chiqqanda shell ekrani qaytadi) |
| `ESC[6n` | "kursor qayerda?" — javob klaviaturadan keladi: `ESC[12;40R` |

Bu 1978 yildagi DEC VT100 tili (ECMA-48). Shuning uchun `edit` bizning konsolda ham, QEMU'ning serial porti
orqali ulangan Linux terminalida ham bir xil ishlaydi.

## 2. vt.c — emulyator

```
console_write ─► vt_putc ─┬─ UTF-8 dekoder (1-4 bayt → bitta belgi)
                          └─ holat mashinasi:  NORMAL ─ESC─► ESC ─[─► CSI "5;10" ─H─► bajarish
                                   │
                    katakchalar massivi  { belgi, rang } × (ustunlar × qatorlar)   ← "haqiqat manbai"
                                   │
                    screen_ops->draw(x, y, belgi, rang)   ← fbcon: piksellar,  vga: 0xB8000
```

- **Nima uchun katakchalar RAM'da saqlanadi?** Framebuffer write-combining xotira: yozish tez, o'qish esa
  juda sekin. Aylantirish (scroll) paytida ekrandan o'qib bo'lmaydi, shuning uchun RAM'dagi nusxa suriladi
  va ekran shu nusxadan qayta chiziladi.
- **Kechiktirilgan o'tish (deferred wrap):** oxirgi ustunga belgi yozilganda kursor darhol keyingi qatorga
  o'tmaydi, faqat **keyingi** belgi kelganda o'tadi (xterm qoidasi). Aks holda muharrir chizgan to'liq
  kenglikdagi har bir qatordan keyin bo'sh qator paydo bo'lardi.
- **Aylantirish hududi** (`ESC[t;br`) va qator qo'shish/o'chirish (`ESC[L`, `ESC[M`) muharrirlar uchun
  kerak.
- **Unicode:** Spleen shriftida ~650 ta belgi bor (chegara chiziqlari `─│┌`, bloklar `░▒▓█`, strelkalar,
  lotin diakritikalari). `font_glyph()` saralangan jadvalda ikkilik qidiruv qiladi. O'zbekcha `oʻ` dagi
  `ʻ` (U+02BB) shriftda yo'q, shuning uchun `‘` glifi ko'rsatiladi (`psf2c.py`: aliases).
- VGA matn rejimida har bir katak 1 bayt, shuning uchun belgilar IBM CP437 jadvaliga aylantiriladi
  (`vga.c: to_cp437`). Kursorni esa apparat o'zi chizadi.

## 3. Klaviatura: tugma → bayt → ketma-ketlik

| tugma | PS/2 scancode | dasturga keladi |
|---|---|---|
| A | `1E` | `a` |
| ↑ | `E0 48` | `ESC [ A` |
| Delete | `E0 53` | `ESC [ 3 ~` |
| Ctrl-C | `1D` + `2E` | `0x03` → tty: **SIGINT** |

Maxsus tugmalar ham serial terminaldagi kabi escape ketma-ketliklariga aylanadi. Shu sababli dastur
klaviaturaning PS/2, USB yoki tarmoq orqali (ssh) ulanganini bilmaydi va bilishi shart ham emas.

Kanonik rejimda `tty.c` bu ketma-ketliklarni "yutadi", aks holda `cat` ga `[A` tushib qolardi.
Ctrl-C/Z bosilganda Linux kabi **kiritish navbati tozalanadi**: oldindan terilgan buyruqlar bajarilmaydi.

## 4. Kanonik va xom rejim

```
                 kanonik (cat, sh skript)          xom (sh prompt, edit)
tahrir           yadro (backspace, Ctrl-U)         dasturning o'zi
read() qaytadi   Enter bosilganda                  har bir tugmada
echo             yadro                             dastur o'zi chizadi
Ctrl-C           SIGINT (ISIG)                     sh: SIGINT; edit: oddiy tugma (ISIG o'chiq)
```

Shell prompt paytida terminalni xom rejimga o'tkazadi (`edit_line`): strelkalar, ↑/↓ tarix, **Tab**
bilan to'ldirish (birinchi so'z uchun `/bin` dagi buyruqlar, qolganlari uchun fayllar), Ctrl-A/E/U/K/W/L.
Buyruqni ishga tushirishdan **oldin** kanonik rejim tiklanadi, chunki `cat` odatdagi terminalni kutadi.
Bash + GNU readline ham aynan shunday ishlaydi.

## 5. `edit` muharriri

antirez'ning *kilo* muharriri (1000 qator) g'oyasiga asoslangan:

1. **Xom rejim**: `tcsetattr` (ICANON, ECHO, ISIG o'chiq) va muqobil ekran.
2. **Model**: fayl qatorlar massivi ko'rinishida saqlanadi (`struct row { chars, render, hl }`). `render`
   — ekrandagi ko'rinish (tab → bo'shliqlar), `hl` — har bir belgining rangi.
3. **Chizish**: butun kadr bitta buferga (`struct abuf`) yig'iladi va **bitta** `write()` bilan
   yuboriladi. Bir necha kichik yozish ekran "miltillashiga" olib kelardi.
4. **Sintaksis ranglari** (C): kalit so'zlar, turlar, satrlar, sonlar, `//` va ko'p qatorli izohlar.
   Ko'p qatorli izoh holati keyingi qatorga uzatiladi (`open_comment`).
5. **Xavfsiz saqlash**: fayl avval yoziladi, keyin `ftruncate` qilinadi. `O_TRUNC` bilan ochilsa va yozish
   yarim yo'lda to'xtasa, fayl bo'sh qolardi.

```
myos:/$ edit /mnt/salom.c       ← diskda: qayta yuklashdan keyin ham saqlanadi
```

## Mashqlar

1. **Oson:** `edit` ga `Ctrl-D` — joriy qatorni nusxalash (duplicate line) buyrug'ini qo'shing.
2. **Oson:** shell tarixini `$HOME/.sh_history` ga saqlab boring va boshlanishda o'qing.
3. **O'rta:** `vt.c` da **SIGWINCH**: ekran o'lchami o'zgarganda oldingi plan guruhiga signal yuboring,
   `edit` esa qayta chizilsin.
4. **O'rta:** `edit` ga **undo** (Ctrl-Z): har bir o'zgarishni stekka yozing (qaysi qator, eski matn).
5. **O'rta:** tty'da ECHO'ni o'qish kontekstidan **uzilish** kontekstiga ko'chiring. Shunda oldindan
   terilgan harflar darhol ekranda ko'rinadi.
6. **Qiyin:** bir nechta virtual konsol (Alt-F1..F4) qo'shing: har biri o'z `vt` holati va `tty` si bilan.
   Linux'dagi `/dev/tty1..6` aynan shunday tuzilgan.
7. **Qiyin:** `less` (sahifalab ko'rish): muqobil ekran, PgUp/PgDn va `/` bilan qidirish.
