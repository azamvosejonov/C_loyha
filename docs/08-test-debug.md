# 08 — Test qilish va debug: professional ish uslubi

Yadro yozishning 20% i kod yozish bo'lsa, 80% i **xatoni topish**. Bu bo'limda o'sha 80% ni tezlashtiradigan
vositalar bor.

## 1. Avtomatik testlar

```bash
make test
```

Ikki qatlam:

| Qatlam | Qayerda | Nimani tekshiradi |
|---|---|---|
| **Unit** (`kernel/tests/selftest.c`) | yadro ichida, `APPEND=selftest` | PMM, VMM (izolyatsiya, oqish), heap (stress), scheduler (preemption) |
| **Integratsion** (`tools/test.sh`) | QEMU tashqarisidan, serial port orqali | shell, dasturlar, himoya, `memtest`, `ps`, `shutdown` |

`test.sh` shell'ga xuddi odam kabi buyruqlar yozadi (serial port orqali), chiqishni `build/test-output.log`
ga saqlaydi va kutilgan/taqiqlangan satrlarni qidiradi. GitHub Actions (`.github/workflows/ci.yml`)
har bir push'da shuni ishga tushiradi.

**Qoida:** yangi xususiyat qo'shsangiz, unga test ham qo'shing. Xatoni tuzatsangiz, o'sha xatoni ushlaydigan
test qo'shing (**regression test**), aks holda u qaytib keladi.

## 2. Serial log — birinchi vosita

`kprintf` hamma narsani serial portga ham yozadi. Terminalda ko'rasiz, faylga saqlashingiz mumkin:

```bash
make run-nographic 2>&1 | tee boot.log
```

Muammo bo'lsa, birinchi qadam — to'g'ri joyga `kprintf` qo'yish. Bu oddiy, lekin juda samarali.

## 3. Panic va backtrace

```
*** KERNEL PANIC ***
kfree: DOUBLE FREE! 0x124040 allaqachon bo'shatilgan
Backtrace (addr2line -f -e build/kernel.elf <manzil>):
  #0  0x00000000001026fa
  #1  0x000000000010586f
```

```bash
addr2line -f -e build/kernel.elf 0x10586f
# crashdemo_run
# kernel/tests/crashdemo.c:52
```

## 4. GDB — qadamma-qadam bajarish

```bash
make debug                    # 1-terminal: QEMU to'xtab, GDB'ni kutadi
gdb -x tools/gdbinit          # 2-terminal
```

```
(gdb) break vmm_map_page
(gdb) continue
(gdb) bt                      # kim chaqirdi?
(gdb) p/x virt                # argument qiymati
(gdb) p *current              # joriy jarayon
(gdb) x/8gx $rsp              # stek tarkibi
(gdb) monitor info tlb        # QEMU: sahifa jadvali (virtual -> fizik)
(gdb) monitor info registers  # CR0/CR3/CR4, GDT, IDT
```

**Assembly bo'ylab qadamlash:** `layout asm`, `stepi`, `info registers rsp rip`. `boot.asm` ni shu
tarzda o'tib, 32-bitdan 64-bitga o'tish lahzasini ko'ring.

## 5. QEMU'ning o'z vositalari

```bash
# Har bir uzilish/exception'ni logga yozish. Triple fault sababini topish uchun eng yaxshi vosita:
qemu-system-x86_64 -kernel build/kernel32.elf -initrd build/initrd.tar \
    -d int,cpu_reset -D qemu.log -no-reboot -serial stdio

# QEMU monitori: Ctrl-A, keyin C (nographic rejimida)
(qemu) info registers
(qemu) info mem               # xaritalangan virtual hududlar
(qemu) x /10i $pc             # joriy instruksiyalar
```

`-d int` logida `v=0e` (page fault), `e=0002` (xato kodi), `cr2=...` ko'rinadi. **Triple fault**
(kompyuter o'z-o'zidan qayta yuklanadi) odatda noto'g'ri IDT/GDT yoki buzilgan stekdan kelib chiqadi. Logning oxirgi
uzilishlariga qarang.

## 6. Statik tahlil vositalari

```bash
objdump -d -M intel build/kernel.elf | less     # disassembly
readelf -S build/kernel.elf                     # bo'limlar
nm -n build/kernel.elf                          # simvollar manzil bo'yicha
size build/kernel.elf                           # text/data/bss hajmi
```

Kompilyator ogohlantirishlari (`-Wall -Wextra -Werror`) ham bepul statik tahlil. Ularni hech qachon o'chirmang.

## 7. Xatolarni topish strategiyasi

1. **Takrorlang.** Xatoni ishonchli takrorlaydigan eng qisqa buyruqlar ketma-ketligini toping.
2. **Bo'ling (bisect).** Qaysi commit'dan keyin buzildi? `git bisect` avtomatik ravishda binar qidiruv qiladi.
   Qaysi buyruqdan keyin buzildi? Kodning yarmini o'chirib ko'ring.
3. **Taxminni tekshiring.** "Bu yerda `ptr` NULL bo'lmasligi kerak" → `ASSERT(ptr)` yozing.
4. **Invariantlar.** Masalan, "bo'sh freymlar soni + band = jami". Ularni testlarda tekshiring.
5. **Birinchi xatoga qarang**, oxirgisiga emas. Buzilish (masalan, use-after-free) ko'pincha ancha oldin sodir bo'lgan.
