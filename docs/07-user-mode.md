# 07 — User mode: ring 3, syscall'lar, ELF, fayl tizimi va shell

**Yadro kodi:** `kernel/proc/exec.c`, `kernel/sys/{syscall.c,elf.c}`, `kernel/fs/{tarfs.c,file.c}`,
`kernel/arch/interrupts.c` (user exception'lari), `include/myos/abi.h`
**User kodi:** `user/lib/{crt0.asm,ulib.c,printf.c,malloc.c}`, `user/bin/*.c`, `user/linker.ld`

Shu bosqichda tizim haqiqiy **operatsion tizim**ga aylanadi. Dasturlar himoyalangan muhitda ishlaydi,
yadrodan faqat syscall orqali xizmat so'raydi, xato qilsa esa faqat o'zi o'ladi.

## 1. Katta rasm: `hello` qanday ishga tushadi

```
 shell (ring 3)                     yadro (ring 0)
 ─────────────                      ──────────────
 spawn("hello", argv) ──int 0x80──► sys_spawn: path va argv ni XAVFSIZ nusxalash
                                    proc_spawn:
                                      1. tarfs_find("hello")         ← initrd.tar ichidan
                                      2. proc_alloc()                ← struct process + 16 KB yadro steki
                                      3. vmm_create_address_space()  ← yangi PML4 (yadro qismi umumiy)
                                      4. elf_load()                  ← PT_LOAD segmentlari → user sahifalari
                                      5. user steki + argv
                                      6. yadro stekida SOXTA uzilish freymi (RIP=_start, CS=0x23)
                                      7. READY
 wait(pid) ─────────int 0x80──────► proc_wait → uxlaydi
                                    ... taymer → schedule() → context_switch → interrupt_return → iretq
                                                                                        │
 hello: _start → main() ◄───────────────────────────────────────────────────── ring 3 ──┘
        printf → write(1,..) ─int 0x80─► sys_write → console_write → VGA + serial
        return 0 → exit(0) ───int 0x80─► proc_exit → ZOMBIE → shell'ni uyg'otish
 wait qaytadi ◄──────────────────────── proc_wait → yadro stekini bo'shatish
```

## 2. Ring 3 ga qanday o'tamiz? — "soxta uzilishdan qaytish"

CPU'da "ring 3 ga o't" degan to'g'ridan-to'g'ri instruksiya yo'q. Lekin `iretq` stekdan **CS** va **SS** ni
oladi. Agar CS = `0x23` (user kodi, RPL=3) bo'lsa, CPU **o'zi** ring 3 ga tushadi.

Shuning uchun `exec.c` yangi jarayonning yadro stekida go'yo u user rejimida uzilgandek freym quradi:

```
yadro steki yuqorisi
┌──────────────────────┐
│ SS     = 0x1B        │ user ma'lumot segmenti
│ RSP    = user steki  │
│ RFLAGS = 0x202       │ IF=1: uzilishlar yoqiq (aks holda taymer bu jarayonni to'xtata olmasdi!)
│ CS     = 0x23        │ ← ring 3
│ RIP    = _start      │ ← ELF e_entry
│ err, vector = 0      │
│ RAX..R15 (RDI=argc,  │
│           RSI=argv)  │
├──────────────────────┤
│ interrupt_return     │ ← context_switch'ning `ret` i shu yerga sakraydi
│ RBX..R15 = 0         │ ← kernel_rsp
└──────────────────────┘
```

Scheduler bu jarayonni birinchi marta tanlaganda: `context_switch` → `ret` → `interrupt_return` →
registrlarni "tiklaydi" → `iretq` → **ring 3, `_start`**. Mavjud mexanizmlar qayta ishlatiladi, yangi kod deyarli yo'q.

## 3. Syscall'lar (`sys/syscall.c`, `include/myos/abi.h`)

| # | Nomi | Nima qiladi |
|---|---|---|
| 0 | exit | jarayonni tugatish |
| 1/2 | write/read | fd orqali chiqarish/o'qish |
| 3/4 | open/close | initrd faylini ochish |
| 5 | spawn | yangi dastur |
| 6 | wait | bolani kutish (`WAIT_NOHANG` bilan ham) |
| 10 | sbrk | heap'ni o'stirish/kichraytirish |
| 11..16 | readdir, meminfo, ps, kill, uptime, shutdown | |

**Konventsiya:** `RAX` = raqam, `RDI, RSI, RDX, R10, R8` = argumentlar, natija `RAX` da. IDT'da `0x80`
darvozasi DPL=3, shuning uchun user uni chaqira oladi. Qolgan vektorlar DPL=0: user `int 14` qilib soxta
page fault yarata olmaydi.

### Xavfsizlik — yadro dasturchisining eng muhim odati

User bergan **har bir** qiymat dushman qiymati deb qaraladi:

```c
sys_read(fd, buf, len):
    fd diapazonda va ochiqmi?                  → get_file()
    len juda katta emasmi?                     → IO_CHUNK_MAX
    [buf, buf+len) user hududida, xaritalangan,
      U=1 va YOZISH mumkinmi? overflow yo'qmi?  → vmm_user_range_ok(..., write=true)
```

Shu tekshiruvlarning birortasi bo'lmasa, nima bo'lishini tasavvur qiling: `read(0, 0x100000, 100)` → yadro
**o'z kodining ustiga** klaviaturadan kelgan baytlarni yozardi. Satrlar (`copy_string_from_user`) har bir
yangi sahifada qayta tekshiriladi, chunki satr xaritalanmagan sahifaga "cho'zilib" ketishi mumkin.

## 4. ELF yuklovchi (`sys/elf.c`)

```bash
readelf -h build/initrd/hello     # sarlavha: Entry point address: 0x40000000
readelf -l build/initrd/hello     # PT_LOAD segmentlari
```

```
LOAD  offset 0x1000  vaddr 0x40000000  filesz 0x1300  memsz 0x1300  R E   ← .text
LOAD  offset 0x3000  vaddr 0x40002000  filesz 0x268   memsz 0x268   R     ← .rodata
LOAD  offset 0x0000  vaddr 0x40003000  filesz 0x0     memsz 0x20    RW    ← .bss (faylda yo'q!)
```

Har bir `PT_LOAD` uchun sahifalar ajratiladi (nollangan: bu tayyor `.bss`) va `filesz` bayt nusxalanadi.
ELF — **ishonchsiz kiritma**: har bir siljish va manzil tekshiriladi (fayl ichidami, user hududidami,
overflow yo'qmi). Real yadrolardagi ko'plab zaifliklar aynan parser'lardan chiqqan.

## 5. Fayl tizimi: tarfs + fayl deskriptorlari

- `initrd.tar` — Makefile yig'adigan oddiy USTAR arxiv. Yuklovchi uni Multiboot moduli sifatida xotiraga
  qo'yadi, biz esa sarlavhalarni o'qiymiz (hajm **sakkizlik** sanoqda yozilgan!).
- **"Hamma narsa — fayl":** `read(0, ...)` klaviaturadan, `read(3, ...)` initrd faylidan o'qiydi. Farqni
  `struct file` yashiradi. fd bu shunchaki `current->files[]` massividagi indeks.

## 6. Himoya amalda (`crash` dasturi)

```
myos$ crash kernel
Yadro kodini (0x100000) o'qiyapman...
[kernel] 'crash' (pid 5) o'ldirildi: Page Fault (sahifa xatosi), RIP=0x400000b8
  Manzil (CR2) = 0x0000000000100000
  Sabab: ruxsat buzildi (sahifa bor), O'QISH, user rejimida
myos$ _                                    ← shell va tizim ishlashda davom etadi
```

| Buyruq | Qaysi himoya ishladi |
|---|---|
| `crash null` | 0-sahifa xaritalanmagan |
| `crash kernel` | yadro sahifalarida U=0 |
| `crash cli` | imtiyozli instruksiya ring 3 da → #GP |
| `crash stack` | user steki ostidagi xaritalanmagan sahifa |
| `crash div0` | #DE ham faqat shu jarayonni o'ldiradi |

`interrupts.c`: exception **user rejimida** bo'lsa (`CS & 3 == 3`), `proc_exit(128 + vektor)` chaqiriladi,
**yadroda** bo'lsa esa `panic` (bu bizning bug'imiz).

## 7. User kutubxonasi — mini libc

- **`crt0.asm`**: haqiqiy kirish nuqtasi `_start` → `main(argc, argv)` → `exit(qaytgan qiymat)`.
- **`printf.c`**: 256 baytlik bufer bilan ishlaydi. Har bir belgi uchun syscall qilish juda qimmat
  (ring 3 → 0 → 3 o'tishi).
- **`malloc.c`**: manzil bo'yicha tartiblangan bo'sh bloklar ro'yxati, **first-fit**, **split**,
  **coalesce** va `sbrk(-n)` bilan xotirani yadroga qaytarish. Yadrodagi slab bilan solishtiring:

| | Yadro slab | User free-list |
|---|---|---|
| O'lchamlar | qat'iy sinflar | istalgan |
| Tezlik | O(1) | O(bo'sh bloklar soni) |
| Fragmentatsiya | ichki (sinfgacha yaxlitlash) | tashqi (teshiklar) → coalesce kerak |

### Haqiqiy bug hikoyasi

Birinchi versiyada `grow()` yangi olingan xotirani `free()` orqali ro'yxatga qo'shardi. `free()` esa
oxirida `trim_top()` ni chaqirib, heap tepasidagi katta bo'sh blokni **darhol yadroga qaytarib yuborardi**.
`malloc(1 MB)` natijada abadiy "o'stir → qaytar → o'stir" tsikliga tushardi. `memtest` shu bug'ni topdi.
Tuzatish: ro'yxatga qo'shish (`insert_free`) va qaytarish siyosati (`trim_top`) alohida funksiyalarga
ajratildi. **Saboq:** bitta funksiya bir nechta vazifani bajarsa, uni boshqa kontekstda qayta ishlatish xavfli.

## 8. Shell

`sh.c` — oddiy user dasturi: `readline` → so'zlarga ajratish → `spawn` → `wait`. Qator oxiridagi `&` fonda
ishga tushiradi. Fondagi dasturlar tugagach zombie bo'lib qolmasligi uchun har bir prompt oldidan
`wait(-1, ..., WAIT_NOHANG)` bilan yig'ib olinadi.

Preemption'ni ko'rish uchun:

```
myos$ spin a 20 &
myos$ spin b 20        ← [a] va [b] chiqishlari aralashadi, garchi hech biri yield qilmasa ham
```

## Sinab ko'ring

1. `vmm_user_range_ok` chaqiruvini `sys_write` dan olib tashlang va `write(1, (void*)0x100000, 64)` qiluvchi
   dastur yozing. Nima chiqadi? Bu nega xavfli?
2. `exec.c` da `f->rflags = 0x202` o'rniga `0x002` yozing va `spin a 20 &`, keyin `spin b 20` qiling. Nima bo'ladi?
3. **Mashq:** `fork()` syscall'ini qo'shing (manzil maydonini nusxalash). Keyin **copy-on-write** bilan
   optimallashtiring (bu page fault handler'da ishlash demakdir!).
4. **Mashq:** `int 0x80` o'rniga `syscall/sysret` instruksiyalarini qo'llang (MSR: STAR, LSTAR, SFMASK).
