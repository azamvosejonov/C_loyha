# Mashqlar: 22 ta "o'zingiz yozing" joyi

Har mashq — kodda `TODO(ID)` bilan belgilangan bo'sh funksiya (yoki funksiya qismi). Kod **kompilyatsiya
bo'ladi**, lekin vaqtinchalik javob qaytaradi; testlar `[XATO]` deydi. Qidirish:

```console
$ grep -rn "TODO(" emu/ firmware/
```

## Jadval

Qiyinlik: ★ — 15 daqiqa, ★★ — bir soatgacha, ★★★ — bir necha soat, ★★★★ — yarim kun va ko'proq
(spetsifikatsiyani o'qish bilan).

| ID | Fayl | Funksiya | Nima | Bob | Qiyinlik |
|---|---|---|---|---|---|
| E1a | `emu/dekod.c` | `imm_i` | I-tur o'zgarmas | [2](02-buyruqlar.md) | ★ |
| E1b | `emu/dekod.c` | `imm_s` | S-tur: ikki bo'lak | 2 | ★ |
| E1c | `emu/dekod.c` | `imm_b` | B-tur: aralash bitlar | 2 | ★★ |
| E1d | `emu/dekod.c` | `imm_j` | J-tur: aralash bitlar | 2 | ★★ |
| E2 | `emu/alu.c` | `shart_bajarildimi` | 6 ta sakrash sharti | [3](03-alu.md) | ★ |
| E3 | `emu/alu.c` | `yuklash_kengaytir` | lb/lh/lbu/lhu | 3 | ★ |
| E4 | `emu/alu.c` | `m_amal` | mul/mulh*/div/rem: maxsus holatlar | 3 | ★★★ |
| E10 | `emu/cpu.c` | `atomik` (qismi) | 9 ta AMO amali | [4](04-protsessor-sikli.md) | ★★ |
| E7a | `emu/siqilgan.c` | `imm_cj` | c.j/c.jal siljishi | [5](05-siqilgan.md) | ★★ |
| E7b | `emu/siqilgan.c` | `imm_cb` | c.beqz/c.bnez siljishi | 5 | ★★ |
| E6 | `emu/trap.c` | `trap_kirish` | trap + delegatsiya | [6](06-rejimlar-trap.md) | ★★★ |
| E9 | `emu/trap.c` | `uzilish_tekshir` | uzilish qabul qilish, ustuvorlik | 6 | ★★★ |
| E5 | `emu/mmu.c` | `jadval_yurish` | Sv32 sahifa jadvali bo'ylab yurish | [7](07-virtual-xotira.md) | ★★★★ |
| E8a | `emu/shina.c` | `tekis_emas_oqi` | tekis bo'lmagan o'qish | 7 | ★★ |
| E8b | `emu/shina.c` | `tekis_emas_yoz` | tekis bo'lmagan yozish (atomar) | 7 | ★★★ |
| P1 | `emu/plic.c` | `eng_ustuvor` | PLIC: eng ustuvor manba | [8](08-qurilmalar.md) | ★★ |
| D1 | `emu/dtb.c` | `be32_yoz` | big-endian yozish | [9](09-dtb.md) | ★ |
| D2 | `emu/dtb.c` | `baytlar` (qismi) | 4 ga tekislash | 9 | ★ |
| D3 | `emu/dtb.c` | `satr_siljishi` | satrlar blokida qidirish/qo'shish | 9 | ★★ |
| F1 | `firmware/sbi.c` | `taymer_qoy` | SBI set_timer: 64 bitli xavfsiz yozish | [10](10-firmware.md) | ★★★ |
| F2 | `firmware/sbi.c` | `sbi_taymer_uzilishi` | M taymer → S taymer | 10 | ★★ |
| F3 | `firmware/asosiy.c` | `firmware_asosiy` (qismi) | delegatsiya, `mret` ga tayyorgarlik | 10 | ★★★ |

## Tavsiya etilgan tartib va "bosqich nazorat nuqtalari"

Boblar tartibida boring. Har bosqich oxirida aniq bir narsa ishlay boshlaydi — bu motivatsiya uchun muhim:

| Bosqich | Boblar | Mashqlar | Nima ishlaydi (muallif tekshirgan) |
|---|---|---|---|
| 1 | 2, 3 | E1, E2, E3, E4 | `alu`, `sakrash`, `xotira`, `mul`, `csr`, `uart` testlari |
| 2 | 4, 5 | E10, E7 | `atomik`, `siqilgan`, `siqilgan_c`; **`make salom`** (clang `c.jal` ishlatadi — E7 kerak) |
| 3 | 6 | E6, E9 | `taymer`, `mrejim` |
| 4 | 7, 8 | E5, E8, P1 | `trap` (tekis bo'lmagan murojaat qismi — E8), `mmu`, `plic` — **hamma** assembly testlari yashil |
| 5 | 9, 10 | D1–D3, F1–F3 | firmware (SBI) testi |
| 6 | 11 | — | `make linux-test` — Linux! |

Bu jadval taxmin emas: muallif har bosqichni alohida (faqat shu mashqlar yechilgan holda) yig'ib tekshirgan.

Taraqqiyotni ko'rish:

```console
$ make test 2>&1 | grep -c "\[ OK \]"     # 27 tadan nechtasi
```

## Ishlash qoidalari

1. **Avval bob, keyin kod.** Har mashqning bobida qo'lda hisoblangan misol bor. Uni o'zingiz qog'ozda
   takrorlang.
2. **Bitta mashq — bitta `make test`.** Bir nechta joyni birdan o'zgartirmang: xato qayerda ekanini bilmay qolasiz.
3. **Test xabarini o'qing.** `olindi` va `kutilgan` ni ikkilikda yozing (1-bob) — qaysi bitlar farq qilyapti?
   Ko'pincha farq **bitta bo'lak** (masalan B-turdagi `imm[11]`) — shu bo'lakning kodini tekshiring.
4. **`-t` — eng yaxshi do'st.** Assembly testi buzilsa: `./build/vk -S -t testlar/emu/build/NOM.elf 2>&1 | less`.
5. **Sanitayzerlar.** Emulyatorni `-fsanitize=undefined,address` bilan yig'ib sinab ko'ring — 1.8 dagi
   tuzoqlarni avtomatik ushlaydi:
   ```console
   $ gcc -O1 -g -fsanitize=undefined,address -Iemu testlar/birlik/birlik.c $(ls emu/*.c | grep -v main.c) -o build/birlik_san
   $ ./build/birlik_san testlar/birlik/kutilgan.dtb
   ```
6. **Yechimni internetdan qidirmang.** Mashqlar ataylab "spetsifikatsiyani o'qib, o'zing yoz" uchun. Qotib
   qolsangiz — spetsifikatsiyaning tegishli bo'limini (har bob boshida ko'rsatilgan) qayta o'qing.

## Mustaqil loyihalar (bitirgandan keyin)

Bular uchun tayyor test yo'q — o'zingiz yozasiz (bu ham mashqning bir qismi). Qiyinlik bo'yicha:

1. **★★ Profilyator.** `-p` bayrog'i: har buyruq manzili bo'yicha hisoblagich; oxirida eng ko'p bajarilgan 20
   manzil. Linux `System.map` dan funksiya nomlarini olib, "Linux yuklanishida vaqtning qancha qismi qaysi
   funksiyada" jadvalini chiqaring.
2. **★★ `sfence.vma` argumentlari.** Faqat ko'rsatilgan manzil/ASID ni tozalash (7-bob). `-s` bilan TLB
   foizini o'lchang.
3. **★★★ GDB serveri.** GDB "remote serial protocol" (TCP orqali matn buyruqlari: `g` — registrlar, `m` —
   xotira, `Z0` — to'xtash nuqtasi). Shunda `gdb-multiarch build/linux-6.6.50/vmlinux` bilan Linux'ni
   **manba kodi darajasida** qadamma-qadam kuzatasiz.
4. **★★★ Zba/Zbb (bit manipulyatsiya).** `sh1add`, `clz`, `cpop`, `rev8` ... DTB'ga qo'shing, Linux'ni
   `CONFIG_RISCV_ISA_ZBB=y` bilan yig'ing — Linux ularni `strlen` kabi funksiyalarda ishlatadi.
5. **★★★ PMP.** M rejim xotira himoyasi: `pmpcfg`/`pmpaddr` haqiqatan ishlasin (hozir faqat saqlanadi).
6. **★★★★ virtio-blk.** Haqiqiy, standart disk qurilmasi (virtio MMIO + virtqueue). Linux drayveri tayyor
   (`CONFIG_VIRTIO_BLK`) — siz faqat qurilmani yasaysiz. Natija: Linux ext2 diskni `mount` qiladi.
7. **★★★★ Ikki yadro (SMP).** `struct cpu` ikkita, navbatma-navbat qadam; CLINT `msip` bilan IPI; SBI HSM
   (`hart_start`); `lr/sc` ning yadrolararo bekor bo'lishi. Linux'ni `CONFIG_SMP=y` bilan yig'ing.
8. **★★★★ F/D kengaytmalari.** Suzuvchi nuqta (IEEE 754): yaxlitlash rejimlari, NaN qoidalari — juda chuqur.
9. **★★★★★ JIT.** RISC-V buyruqlar blokini x86-64 mashina kodiga tarjima qilib, to'g'ridan-to'g'ri bajarish.
   QEMU (TCG) shunday ishlaydi: 5–20 barobar tezroq.
10. **★★★★★ MyOS → RISC-V.** Asosiy loyihadagi MyOS yadrosini (x86-64) RISC-V 32 ga ko'chirish va **o'z
    emulyatoringizda** ishga tushirish. Firmware'imiz tayyor — yadro S rejimda, SBI bilan.
