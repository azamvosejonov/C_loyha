# 17-bob. Assembly va C: CPU aslida nima bajaradi

> **Bu bobdan keyin:** x86-64 registrlarini, asosiy buyruqlarni, funksiya chaqirish qoidalarini
> (calling convention), stek kadrini, `gcc -S` chiqishini o'qishni va inline assembly'ni bilasiz.
> Yadroning C bilan yozib bo'lmaydigan qismlari (yuklash, uzilishlar, kontekst almashish, syscall
> kirishi) assembly'da — ularni o'qiy olishingiz kerak.

## 17.1. Registrlar

CPU ichidagi juda tez "o'zgaruvchilar". x86-64 da 16 ta umumiy 64 bitli registr:

| Registr | An'anaviy vazifa (System V ABI, Linux/MyOS) |
|---|---|
| `rax` | qaytish qiymati; syscall raqami |
| `rdi, rsi, rdx, rcx, r8, r9` | funksiyaning 1..6-argumentlari |
| `rsp` | **stek ko'rsatkichi** (stek tepasi) |
| `rbp` | kadr ko'rsatkichi (ixtiyoriy) |
| `rbx, rbp, r12–r15` | "saqlanadigan" (callee-saved): chaqirilgan funksiya ularni tiklashi shart |
| `rax, rcx, rdx, rsi, rdi, r8–r11` | "buziladigan" (caller-saved): chaqiruvdan keyin o'zgargan bo'lishi mumkin |
| `rip` | keyingi bajariladigan buyruq manzili |
| `rflags` | bayroqlar: ZF (nol), CF (ko'chirish), SF (ishora), IF (uzilishlar yoqilgan) |

Kichik qismlari: `eax` (rax ning pastki 32 biti), `ax` (16), `al` (8). **Maxsus registrlar** (faqat yadro):
`cr0`, `cr3` (sahifa jadvali manzili!), `cr4`, `gdtr`, `idtr`, MSR'lar (`rdmsr`/`wrmsr`).

## 17.2. Asosiy buyruqlar (Intel sintaksisi, NASM)

```nasm
mov rax, 5          ; rax = 5
mov rax, rbx        ; rax = rbx
mov rax, [rbx]      ; rax = *(uint64_t *)rbx         - [ ] xotira
mov [rbx+8], rax    ; *(uint64_t *)(rbx + 8) = rax
lea rax, [rbx+rcx*4]; rax = rbx + rcx*4              - manzilni HISOBLASH (o'qimaydi)
add rax, rbx        ; rax += rbx
sub rsp, 16         ; stekda 16 bayt joy
inc rcx / dec rcx
and / or / xor / not / shl / shr   ; bitli amallar
xor eax, eax        ; rax = 0 (eng qisqa usul)
cmp rax, rbx        ; rax - rbx ni hisoblab, faqat bayroqlarni o'rnatadi
je  yorliq          ; teng bo'lsa sakra (ZF=1). jne, jl, jg, jb (ishorasiz <), ja ...
jmp yorliq          ; shartsiz sakrash
call funksiya       ; qaytish manzilini stekka qo'yib, sakrash
ret                 ; stekdan manzilni olib, o'sha yerga qaytish
push rax / pop rax  ; stekka qo'yish / olish (rsp -= 8 / += 8)
```

**Ikki sintaksis:** NASM (MyOS `.asm` fayllari) — Intel: `mov QAYERGA, QAYERDAN`, `;` — izoh.
GCC `-S` chiqishi va inline asm — AT&T: `movq %rbx, %rax` (teskari tartib, `%` registrlar, `$` son,
`#` izoh). Ko'rish uchun: `gcc -S -masm=intel`.

## 17.3. C → assembly: misollar

```c
long qoshish(long a, long b) { return a + b; }
```

```nasm
qoshish:
    lea rax, [rdi+rsi]      ; a (rdi) + b (rsi) -> rax (qaytish qiymati)
    ret
```

```c
long yigindi(const long *a, long n)
{
    long s = 0;
    for (long i = 0; i < n; i++)
        s += a[i];
    return s;
}
```

```nasm
yigindi:                        ; rdi = a, rsi = n
    xor eax, eax                ; s = 0
    test rsi, rsi
    jle .tugadi                 ; n <= 0 bo'lsa - chiqish
    lea rdx, [rdi+rsi*8]        ; rdx = &a[n] (oxir)
.sikl:
    add rax, [rdi]              ; s += *a
    add rdi, 8                  ; a++ (8 bayt - long)
    cmp rdi, rdx
    jne .sikl
.tugadi:
    ret
```

(GCC 13 `-O2` chiqishi deyarli aynan shunday, faqat tartibi biroz boshqacha.) E'tibor bering: kompilyator `a[i]` ni ko'rsatkich bilan yurishga aylantirdi (7-bob: `a[i]` = `*(a + i)`)
va `i` ni umuman yo'qotdi. O'zingiz sinang: `gcc -O2 -S -masm=intel fayl.c` — va Compiler Explorer
uslubida har bir C qatori nimaga aylanganini solishtiring.

## 17.4. Chaqirish qoidalari va stek kadri

`call f`:
1. qaytish manzili (`call` dan keyingi buyruq) stekka qo'yiladi (`rsp -= 8`);
2. `rip = f`.

Funksiya ichida (optimallashtirishsiz):

```nasm
f:
    push rbp            ; eski kadr ko'rsatkichini saqlash
    mov rbp, rsp        ; yangi kadr
    sub rsp, 32         ; lokal o'zgaruvchilar uchun joy
    ...                 ; lokal o'zgaruvchilar: [rbp-8], [rbp-16] ...
    leave               ; mov rsp, rbp; pop rbp
    ret                 ; stekdan qaytish manzilini olib sakrash
```

```text
yuqori manzil
  | ...chaqiruvchining kadri |
  | qaytish manzili          |  <- call qo'ydi
  | eski rbp                 |  <- rbp shu yerga ko'rsatadi
  | lokal 1                  |  [rbp-8]
  | lokal 2                  |  [rbp-16]
  |                          |  <- rsp
quyi manzil
```

Qoidalar (System V AMD64 ABI):
- `call` paytida `rsp` **16 ga karrali** bo'lishi kerak (SSE buyruqlari uchun). MyOS'da signal kadrini
  qurishda bu qoida muhim (`setup_frame` lab'i).
- 6 tadan ortiq argument — stekda.
- **Red zone:** `rsp` ostidagi 128 bayt — user funksiyalari uni `rsp` ni surmasdan ishlatishi mumkin.
  **Yadroda bu taqiqlanadi** (`-mno-red-zone`): uzilish kelganda CPU aynan shu joyga yozadi va lokal
  o'zgaruvchilarni buzadi.

**Bufer to'lishi hujumi endi tushunarli:** stekdagi `char buf[16]` dan toshgan ma'lumot yuqoridagi
**qaytish manzilini** ustidan yozadi → `ret` hujumchi xohlagan joyga sakraydi.

## 17.5. Inline assembly (GCC)

C ichida CPU buyrug'i kerak bo'lganda (yadroda: `cli`, `hlt`, `in/out`, `rdmsr`, `invlpg`, `mov cr3`):

```c
static inline void cli(void) { __asm__ volatile("cli" ::: "memory"); }

static inline uint64_t rdtsc(void)
{
    uint32_t lo, hi;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

static inline void yoz_cr3(uint64_t pml4)
{
    __asm__ volatile("mov %0, %%cr3" : : "r"(pml4) : "memory");
}
```

Sintaksis: `__asm__ volatile("buyruqlar" : chiqishlar : kirishlar : buziladiganlar);`

| Qism | Ma'nosi |
|---|---|
| `"=a"(lo)` | chiqish: `eax` → `lo` (`=` — yoziladi) |
| `"r"(x)` | kirish: x istalgan registrda |
| `"a"`, `"D"`, `"S"`, `"d"` | aniq registr: rax, rdi, rsi, rdx |
| `"Nd"(port)` | 8 bitli o'zgarmas yoki `dx` (port I/O uchun) |
| `%0`, `%1` | operandlar tartib raqami bo'yicha |
| `%%cr3` | registr nomi (`%` ikki marta — AT&T) |
| `"memory"` | "bu buyruq xotirani o'zgartirishi mumkin" — kompilyator xotira amallarini u orqali ko'chirmaydi |
| `volatile` | buyruqni o'chirma/ko'chirma (natijasi ishlatilmasa ham) |

MyOS'da: `kernel/arch/cpu.h`, `io.h`; user syscall'lari — `user/libc/syscall.h`:

```c
static inline long __syscall1(long n, long a1)
{
    long r;
    __asm__ volatile("syscall" : "=a"(r) : "a"(n), "D"(a1) : "rcx", "r11", "memory");
    return r;
}
```

`rcx` va `r11` buziladi — chunki `syscall` buyrug'i qaytish manzilini `rcx` ga, `rflags` ni `r11` ga yozadi.

## 17.6. Yadrodagi assembly fayllari

MyOS'da faqat C bilan qilib bo'lmaydigan narsalar assembly'da — o'qish tartibida:

| Fayl | Nima uchun assembly |
|---|---|
| `kernel/boot/boot.asm` | CPU 32 bitli rejimda uyg'onadi; C ishlashi uchun stek, sahifalar va 64-bit rejim kerak |
| `kernel/arch/isr.asm` | uzilish kirishi: hamma registrlarni saqlash, C ishlovchini chaqirish, `iretq` |
| `kernel/proc/switch.asm` | kontekst almashish: bir jarayonning `rsp` idan boshqasiga o'tish |
| `kernel/arch/syscall_entry.asm` | `syscall` kirishi: user stekidan yadro stekiga o'tish |
| `kernel/arch/trampoline.asm` | qolgan CPU yadrolari 16 bitli rejimda uyg'onadi — ularni 64 bitga olib chiqish |
| `user/libc/crt0.asm` | dastur boshlanishi: stekdan `argc/argv` ni olib `main` ni chaqirish |

Har biri 50–200 qator va batafsil izohlangan. `switch.asm` dan boshlang — u eng qisqa va eng "sehrli".

## 17.7. O'zingizni tekshiring

1. Funksiyaning 1-argumenti va qaytish qiymati qaysi registrlarda?
2. `lea` va `mov` ning `[ ]` bilan farqi?
3. Nega yadro `-mno-red-zone` bilan yig'iladi?
4. Inline asm'dagi `"memory"` nima uchun?
5. Stekdagi bufer to'lishi qaytish manzilini qanday buzadi?

<details><summary>Javoblar</summary>

1. `rdi` va `rax`.
2. `mov rax, [x]` — xotiradan o'qiydi; `lea rax, [x]` — faqat manzilni hisoblaydi.
3. Uzilish kelganda CPU joriy stekka yozadi — red zone'dagi lokal ma'lumotlar buziladi.
4. Kompilyator xotira amallarini asm buyrug'i orqali ko'chirmasligi va keshlangan qiymatlarni qayta o'qishi uchun.
5. Bufer lokal o'zgaruvchi sifatida qaytish manzilidan pastda turadi; toshgan yozuv yuqoriga — qaytish manziliga yetadi.
</details>

## 17.8. Mashq

- 3–4 ta kichik funksiyani (`strlen`, `max`, massiv yig'indisi) `gcc -O0 -S -masm=intel` va `-O2` bilan
  kompilyatsiya qilib, har bir qatorni izohlang.
- `gdb` da `layout asm` va `stepi` bilan dasturni buyruqma-buyruq bajaring, `info registers` ni kuzating.
- `kernel/proc/switch.asm` ni o'qib, har bir qatorni o'z so'zingiz bilan yozing.

Keyingi bob: [18-bob. Freestanding C: yadroga ko'prik](18-yadroga-koprik.md)
