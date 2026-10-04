# 17-bob. Assembly va C: CPU aslida nima bajaradi

> **Bu bobda nima o'rganasiz:** x86-64 registrlarini; asosiy assembly buyruqlarini; funksiya chaqirish qoidalarini (calling convention); stek kadrini; `gcc -S` chiqishini o'qishni;
> inline assembly'ni. Yadroning C bilan yozib bo'lmaydigan qismlari (yuklash, uzilishlar, kontekst almashish, syscall kirishi) assembly'da — ularni o'qiy olishingiz kerak.
> **Oldindan nima kerak:** 1-, 3-, 5-, 7-, 14-, 16-boblar.   **Vaqt:** 7–8 soat.

> **To'liq ishlaydigan misol:** [misollar/17_assembly.c](misollar/17_assembly.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Bu bob nima haqida?

C — "ko'chma assembly" (0-bob). Bu bobda uning ostiga tushamiz: kompilyator C kodini **qanday CPU buyruqlariga** aylantiradi? Nega funksiyaga argumentlar aynan `rdi`, `rsi` da beriladi? Stek kadri nima?
Assembly yozmoqchi emassiz — **o'qiy olishingiz** kerak: yadro dasturchisi xatoni ko'pincha assembly'da topadi.

**Hayotdan misol: retsept oddiy harakatlarga bo'linadi.** Murakkab taom ("osh") oddiy harakatlarga bo'linadi: "piyozni to'g'ra", "yog'ni qizdir", "go'shtni sol". CPU ham shunday: har qanday C dasturi
o'nlab oddiy buyruqlarga bo'linadi: "xotiradan ol", "qo'sh", "solishtir", "sakra".

| Oshxonada | CPU'da |
|---|---|
| qo'lingizdagi narsalar (pichoq, qoshiq) | **registrlar** (juda tez, juda kam) |
| stol ustidagilar | kesh |
| shkafdagilar | RAM |
| retseptdagi qadam | **buyruq** (`mov`, `add`, `jmp`) |

## 17.1. Registrlar

**Hayotdan misol: qo'lingizdagi narsalar.** Oshxonada ishlayotganda eng kerakli narsalar qo'lingizda (pichoq, qoshiq) — registrlar. Stol ustidagilar — kesh. Shkafdagilar — RAM. Qo'lda ushlash eng tez,
lekin qo'l atigi ikkita (x86-64 da 16 ta umumiy registr). Protsessor hisoblashni faqat **registrlarda** qiladi: xotiradagi sonni avval registrga olib keladi, keyin qo'shadi.

**Bu nima?** Registr — CPU ichidagi juda tez "o'zgaruvchi" (64 bit). x86-64 da 16 ta umumiy registr:

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

Kichik qismlari: `eax` (rax ning pastki 32 biti), `ax` (16), `al` (8). **Maxsus registrlar** (faqat yadro): `cr0`, `cr3` (sahifa jadvali manzili!), `cr4`, `gdtr`, `idtr`, MSR'lar (`rdmsr`/`wrmsr`).

## 17.2. Asosiy buyruqlar (Intel sintaksisi, NASM)

**Hayotdan misol: ishchining oddiy buyruqlari.** "Qutidan olib qo'lingga ol" (`mov`), "qo'shib qo'y" (`add`), "solishtir" (`cmp`), "agar teng bo'lsa, 5-qadamga o't" (`je`).
Har qanday murakkab dastur shu kabi o'nlab oddiy buyruqlarga bo'linadi.

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

**Ikki sintaksis:** NASM (MyOS `.asm` fayllari) — Intel: `mov QAYERGA, QAYERDAN`, `;` — izoh. GCC `-S` chiqishi (standart) va inline asm — AT&T: `movq %rbx, %rax` (teskari tartib, `%` registrlar, `$` son,
`#` izoh). Ko'rish uchun: `gcc -S -masm=intel`.

> **Eslab qoling:** Intel: `mov QAYERGA, QAYERDAN`. `[ ]` — xotira. `lea` — faqat manzilni hisoblaydi (xotiraga tegmaydi). `cmp` + `je/jne/jl` — shart.

## 17.3. C → assembly: haqiqiy misollar

Quyidagi uch funksiya kompilyator bilan assembly'ga aylantiriladi (`-O2`, Intel sintaksisi).

```c
/* qoshish.c - uchta oddiy funksiya */
long qoshish(long a, long b)
{
    return a + b;
}

long yigindi(const long *a, long n)
{
    long s = 0;
    for (long i = 0; i < n; i++)
        s += a[i];
    return s;
}

long yetti(long a, long b, long c, long d, long e, long f, long g)
{
    return a + b + c + d + e + f + g;
}
```

```console
$ gcc -O2 -S -masm=intel -fcf-protection=none -o - qoshish.c | grep -vE '^\s+\.' | grep -vE '^\.LF|^$'
qoshish:
	lea	rax, [rdi+rsi]
	ret
yigindi:
	test	rsi, rsi
	jle	.L6
	lea	rdx, [rdi+rsi*8]
	xor	eax, eax
.L5:
	add	rax, QWORD PTR [rdi]
	add	rdi, 8
	cmp	rdi, rdx
	jne	.L5
	ret
.L6:
	xor	eax, eax
	ret
yetti:
	add	rdi, rsi
	add	rdi, rdx
	add	rdi, rcx
	add	rdi, r8
	lea	rax, [rdi+r9]
	add	rax, QWORD PTR 8[rsp]
	ret
```

**`qoshish`:** (1-funksiya)

```nasm
qoshish:
    lea rax, [rdi+rsi]      ; a (rdi) + b (rsi) -> rax (qaytish qiymati)
    ret
```

Nega `lea`, `add` emas? `lea rax, [rdi+rsi]` "manzilni hisoblash" buyrug'i, lekin hisobi qo'shish bilan bir xil — bir buyruqda natijani yangi registrga yozadi. **Ikki argument** `rdi` va `rsi` da keldi,
**natija** `rax` da qaytadi — bu chaqirish qoidasi (17.4).

**`yigindi`:** (2-funksiya; `rdi` = massiv, `rsi` = n)

```nasm
yigindi:
    test rsi, rsi           ; n bilan n ni VA (nol-mi tekshirish)
    jle .L6                 ; n <= 0 bo'lsa - chiqish
    lea rdx, [rdi+rsi*8]    ; rdx = &a[n] (massiv OXIRI manzili)
    xor eax, eax            ; s = 0
.L5:
    add rax, QWORD PTR [rdi]; s += *a
    add rdi, 8              ; a++ (8 bayt - long)
    cmp rdi, rdx
    jne .L5                 ; oxirga yetmagan bo'lsa - takror
    ret
.L6:
    xor eax, eax            ; natija 0
    ret
```

Kompilyator `a[i]` ni **ko'rsatkich bilan yurishga** aylantirdi (7-bob: `a[i]` = `*(a + i)`) va `i` ni umuman yo'qotdi: o'rniga `rdi` ni 8 baytga siljitib, `rdx` (oxir) ga yetgunicha aylanadi.
Siz yozgan `for (i...)` sikli → `.L5:` yorlig'i + `jne .L5`.

**`yetti`:** (3-funksiya; 7 ta argument)

```nasm
yetti:
    add rdi, rsi            ; a + b
    add rdi, rdx            ; + c
    add rdi, rcx            ; + d
    add rdi, r8             ; + e
    lea rax, [rdi+r9]       ; + f  -> rax
    add rax, QWORD PTR 8[rsp]   ; + g  <- 7-ARGUMENT STEKDA (registr yetmadi)
    ret
```

Birinchi 6 argument registrlarda (`rdi, rsi, rdx, rcx, r8, r9`), **7-chisi stekda** (`[rsp+8]` — `rsp` ostida qaytish manzili turibdi, undan keyin argument).

> **Eslab qoling:** assembly'ni o'qish — C bilan **juftlab** qarash: har bir C qatori qaysi buyruqlarga aylandi? `gcc -O2 -S -masm=intel` shuni ko'rsatadi.

## 17.4. Chaqirish qoidalari va stek kadri

**Hayotdan misol: pochta qoidasi.** Pochta qat'iy qoidaga ega: indeks — yuqori o'ng burchakda, manzil — o'rtada. Hamma shu qoidaga amal qilgani uchun har qanday pochtachi har qanday xatni yetkazadi.
Funksiya chaqiruvi ham: 1-argument doim `rdi` da, 2-si `rsi` da, natija `rax` da. Shuning uchun GCC'da yozilgan funksiyani assembly'dan yoki boshqa tildan chaqirish mumkin.

`call f`:

1. qaytish manzili (`call` dan keyingi buyruq) stekka qo'yiladi (`rsp -= 8`);
2. `rip = f`.

**Hayotdan misol: stek kadri — har topshiriq uchun alohida varaq.** Funksiya chaqirilganda stekda unga "varaq" ajratiladi: qaytish manzili (qayerga qaytish kerak), lokal o'zgaruvchilar. Funksiya tugaganda varaq yirtib tashlanadi.

Optimallashtirishsiz (`-O0`) kompilyator **haqiqiy stek kadri** quradi. Mana kichik funksiya:

```c
/* kadr.c - lokal o'zgaruvchilar bilan funksiya */
long kvadratlar_yigindisi(long a, long b)
{
    long t = a * a;
    long u = b * b;
    return t + u;
}
```

```console
$ gcc -O0 -S -masm=intel -fcf-protection=none -fno-asynchronous-unwind-tables -o - kadr.c | grep -vE '^\s+\.' | grep -v '^$'
kvadratlar_yigindisi:
	push	rbp
	mov	rbp, rsp
	mov	QWORD PTR -24[rbp], rdi
	mov	QWORD PTR -32[rbp], rsi
	mov	rax, QWORD PTR -24[rbp]
	imul	rax, rax
	mov	QWORD PTR -16[rbp], rax
	mov	rax, QWORD PTR -32[rbp]
	imul	rax, rax
	mov	QWORD PTR -8[rbp], rax
	mov	rdx, QWORD PTR -16[rbp]
	mov	rax, QWORD PTR -8[rbp]
	add	rax, rdx
	pop	rbp
	ret
```

**Qatorma-qator (nima uchun):**

```nasm
kvadratlar_yigindisi:
    push rbp                      ; 1) eski kadr ko'rsatkichini SAQLASH
    mov rbp, rsp                  ; 2) yangi kadr: rbp = hozirgi stek tepasi
    mov QWORD PTR -24[rbp], rdi   ; 3) argument 'a' ni stekka saqlash (lokal nusxa)
    mov QWORD PTR -32[rbp], rsi   ;    argument 'b'
    mov rax, QWORD PTR -24[rbp]   ; 4) t = a * a
    imul rax, rax
    mov QWORD PTR -16[rbp], rax   ;    t -> [rbp-16]
    mov rax, QWORD PTR -32[rbp]   ; 5) u = b * b
    imul rax, rax
    mov QWORD PTR -8[rbp], rax    ;    u -> [rbp-8]
    mov rdx, QWORD PTR -16[rbp]   ; 6) t + u
    mov rax, QWORD PTR -8[rbp]
    add rax, rdx                  ;    natija rax da
    pop rbp                       ; 7) eski rbp ni tiklash
    ret                           ; 8) qaytish manzilini stekdan olib sakrash
```

```text
yuqori manzil
  | ...chaqiruvchining kadri |
  | qaytish manzili          |  <- call qo'ydi
  | eski rbp                 |  <- rbp shu yerga ko'rsatadi
  | u                        |  [rbp-8]
  | t                        |  [rbp-16]
  | a (nusxa)                |  [rbp-24]
  | b (nusxa)                |  [rbp-32]
  quyi manzil
```

`-O0` bilan kod sekin, lekin **debugger uchun ideal**: har bir o'zgaruvchi stekda o'z joyida turadi (`gdb` `print t` shu yerdan o'qiydi). `-O2` bularning hammasini registrlarda qiladi (17.3 dagi `qoshish` — bitta `lea`).

Qoidalar (System V AMD64 ABI):

- `call` paytida `rsp` **16 ga karrali** bo'lishi kerak (SSE buyruqlari uchun). MyOS'da signal kadrini qurishda bu qoida muhim (`setup_frame` lab'i).
- 6 tadan ortiq argument — stekda (17.3 dagi `yetti`).
- **Red zone:** `rsp` ostidagi 128 bayt — user funksiyalari uni `rsp` ni surmasdan ishlatishi mumkin. **Yadroda bu taqiqlanadi** (`-mno-red-zone`): uzilish kelganda CPU aynan shu joyga yozadi va lokal
  o'zgaruvchilarni buzadi.

**Bufer to'lishi hujumi endi tushunarli:** stekdagi `char buf[16]` dan toshgan ma'lumot yuqoridagi **qaytish manzilini** ustidan yozadi → `ret` hujumchi xohlagan joyga sakraydi.

> **Eslab qoling:** funksiya argumentlari: `rdi, rsi, rdx, rcx, r8, r9` (keyingilari — stekda); natija — `rax`; `call` qaytish manzilini stekka qo'yadi, `ret` uni oladi.

## 17.5. Inline assembly (GCC)

**Hayotdan misol: o'zbekcha gapda bitta inglizcha so'z.** Ba'zan o'zbekchada aniq so'z yo'q — inglizchasini qo'shib yuborasiz. C'da ham ba'zi buyruqlar yo'q (`cpuid`, `rdtsc`, `cli`) — ularni `__asm__` bilan
qo'shib yozasiz. Faqat kompilyatorga aniq aytish kerak: qaysi registrlarni ishlatdingiz va nimani buzdingiz.

**Bu nima?** Inline assembly — C kodi ichiga to'g'ridan-to'g'ri CPU buyrug'ini qo'shish. Yadroda kerak: `cli`, `hlt`, `in/out`, `rdmsr`, `invlpg`, `mov cr3`.

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

Avval oddiy hisob-kitob buyruqlari bilan (user rejimida ishlaydi):

```c
/* bitlar_asm.c - inline assembly: qo'shish, ko'paytirish va eng katta bitni topish */
#include <stdio.h>

static long asm_qosh(long a, long b)
{
    long natija;
    __asm__("mov %1, %0\n\t"                    /* natija = a */
            "add %2, %0"                        /* natija += b */
            : "=&r"(natija)                     /* chiqish: istalgan registr */
            : "r"(a), "r"(b));                  /* kirish: istalgan registrlar */
    return natija;
}

static long asm_kopaytir(long a, long b)
{
    __asm__("imul %1, %0" : "+r"(a) : "r"(b));  /* a *= b ("+" - ham kirish, ham chiqish) */
    return a;
}

/* bsr - "bit scan reverse": eng katta 1 bitning raqami (0 ga berilmasin) */
static int eng_katta_bit(unsigned long x)
{
    unsigned long r;
    __asm__("bsr %1, %0" : "=r"(r) : "rm"(x));
    return (int)r;
}

int main(void)
{
    printf("asm_qosh(40, 2) = %ld\n", asm_qosh(40, 2));
    printf("asm_kopaytir(12, 12) = %ld\n", asm_kopaytir(12, 12));

    unsigned long sonlar[] = { 1, 5, 255, 256, 1000000, 4000000000ul };
    for (int i = 0; i < 6; i++)
        printf("%10lu -> eng katta bit %2d, demak %2d bitli son\n",
               sonlar[i], eng_katta_bit(sonlar[i]), eng_katta_bit(sonlar[i]) + 1);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 bitlar_asm.c -o bitlar_asm
$ ./bitlar_asm
asm_qosh(40, 2) = 42
asm_kopaytir(12, 12) = 144
         1 -> eng katta bit  0, demak  1 bitli son
         5 -> eng katta bit  2, demak  3 bitli son
       255 -> eng katta bit  7, demak  8 bitli son
       256 -> eng katta bit  8, demak  9 bitli son
   1000000 -> eng katta bit 19, demak 20 bitli son
4000000000 -> eng katta bit 31, demak 32 bitli son
$ gcc -O2 -S bitlar_asm.c -o - | grep -m3 "bsr\|imul"
	imul %rdx, %rdx
	bsr %rdx, %rcx
```

**Bu dastur nima qiladi (umumiy):** uchta kichik funksiya — qo'shish, ko'paytirish, eng katta 1-bit — C emas, **to'g'ridan-to'g'ri CPU buyruqlari** bilan yozilgan. `main` ularni chaqirib natijani ko'rsatadi.
Oxirgi buyruq kompilyator yaratgan assembly'dan sizning buyruqlaringizni topadi — ular C kodi ichiga aynan siz yozgandek qo'yilgan.

**Qismlar (vazifasi → tafsilot):**

| Funksiya | Buyruq | Vazifasi | Operandlar |
|---|---|---|---|
| `asm_qosh` | `mov %1, %0` + `add %2, %0` | `natija = a; natija += b` | `%0` = natija (chiqish), `%1` = a, `%2` = b. `"=&r"` — chiqish registri kirishlar bilan **ustma-ust tushmasin** (`&`) |
| `asm_kopaytir` | `imul %1, %0` | `a *= b` | `"+r"(a)` — `a` ham kirish, ham chiqish |
| `eng_katta_bit` | `bsr %1, %0` | eng katta 1-bit raqami | `"rm"` — registr yoki xotira |

Natija: 255 → eng katta bit 7 (8 bitli son); 256 → 8 (9 bitli) — 3-bobdagi bit raqamlash bilan mos.

### Yadro funksiyalari (ular faqat yadroda ishlaydi)

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

`cli` — uzilishlarni o'chirish; `rdtsc` — CPU taymeri (`edx:eax` ga yozadi, shuning uchun `"=a"(lo), "=d"(hi)`); `mov cr3` — sahifa jadvalini almashtirish. Ulardan `rdtsc` oddiy dasturda ham ishlaydi — keyingi misolda.

### `syscall` buyrug'i: 14-bob va 17-bobning uchrashuvi

14.1 da `write` chaqiruvi `rax = 1, rdi = 1, rsi = buf, rdx = n; syscall` ga aylanishini ko'rgan edik. Endi uni o'zimiz **libc siz** bajaramiz:

```c
/* syscall_asm.c - write ni libc'siz, inline assembly bilan */
#include <stdint.h>
#include <stdio.h>

static long my_write(long fd, const void *buf, long n)
{
    long r;
    __asm__ volatile("syscall"
                     : "=a"(r)                          /* natija rax da */
                     : "a"(1L), "D"(fd), "S"(buf), "d"(n)   /* rax=1 (SYS_write), rdi=fd, rsi=buf, rdx=n */
                     : "rcx", "r11", "memory");          /* syscall bularni buzadi */
    return r;
}

static inline uint64_t rdtsc(void)
{
    uint32_t lo, hi;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

int main(void)
{
    long k = my_write(1, "salom asm dan\n", 14);
    printf("my_write qaytardi: %ld\n", k);

    uint64_t t1 = rdtsc();
    uint64_t t2 = rdtsc();
    printf("rdtsc: ikkinchi o'qish kattaroq-mi? %d\n", t2 > t1);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -O2 syscall_asm.c -o syscall_asm
$ ./syscall_asm
salom asm dan
my_write qaytardi: 14
rdtsc: ikkinchi o'qish kattaroq-mi? 1
```

**Qismlar:** `"a"(1L)` — `rax` ga 1 (`SYS_write`); `"D"(fd)` — `rdi`; `"S"(buf)` — `rsi`; `"d"(n)` — `rdx` (kirish registrlarini aniq tanlash). `"rcx", "r11"` buziladiganlar ro'yxatida —
chunki `syscall` buyrug'i qaytish manzilini `rcx` ga, `rflags` ni `r11` ga yozadi. Natija `rax` da: yozilgan baytlar soni (14).

MyOS'da: `kernel/arch/cpu.h`, `io.h`; user syscall'lari — `user/libc/syscall.h`:

```text
static inline long __syscall1(long n, long a1)
{
    long r;
    __asm__ volatile("syscall" : "=a"(r) : "a"(n), "D"(a1) : "rcx", "r11", "memory");
    return r;
}
```

> **Eslab qoling:** inline asm = buyruq + chiqishlar + kirishlar + **nima buzilishi**. Kompilyatorga halol ayting — aks holda u registrlar qiymatini buzib yuboradi.

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

## Hayotdan misol va to'liq dastur

**Son necha bitli?** Yuqoridagi `bitlar_asm.c` aynan shu bobning to'liq dasturi: ichida inline assembly bilan qo'shish, ko'paytirish va `bsr` bor, natijasi — sonning bit uzunligi (1 → 1 bit, 255 → 8 bit, 256 → 9 bit).
Uni `gcc -O0 -S` va `gcc -O2 -S` bilan solishtiring — kompilyator inline asm buyruqlarini aynan siz yozgandek, atrofini esa o'zi optimallashtirib qo'yadi.

**Sinab ko'ring:** `asm_qosh` ga o'xshash `asm_ayir` yozing (`sub` buyrug'i). `gcc -O0 -S` va `gcc -O2 -S` bilan `main` ni solishtiring — `-O2` qancha qisqa?

## Bob xulosasi (yodlash uchun)

1. **Registr** — CPU ichidagi tez "o'zgaruvchi" (16 ta umumiy); CPU hisoblashni faqat registrlarda qiladi. `rip` — joriy buyruq, `rsp` — stek tepasi.
2. Chaqirish qoidasi: argumentlar `rdi, rsi, rdx, rcx, r8, r9` (keyingilari — stek); natija `rax`; `call` qaytish manzilini stekka qo'yadi, `ret` oladi.
3. Stek kadri: `push rbp; mov rbp, rsp; sub rsp, N` ... `pop rbp; ret`; lokal o'zgaruvchilar `[rbp-8]`, `[rbp-16]`...; `-O2` ularni registrlarga ko'chiradi.
4. Intel: `mov QAYERGA, QAYERDAN`; `[ ]` — xotira; `lea` — faqat manzilni hisoblash; `cmp` + `jXX` — shart.
5. Inline assembly: `__asm__ volatile("buyruq" : chiqish : kirish : buziladiganlar)`; `"memory"`; `syscall` `rcx` va `r11` ni buzadi.

## Savol-javob

**Savol:** Men C yozaman, nega assembly o'rganishim kerak?
**Javob:** Yadroda assembly'siz bo'lmaydigan joylar bor (kirish nuqtasi, kontekst almashish, uzilish kirishi), va xatoni topishda `objdump -d`/gdb orqali kompilyator **nima qilganini** ko'ra bilish kerak. Hamma narsani assembly'da yozmaysiz, lekin **o'qiy olishingiz** shart (17.3).

**Savol:** `caller-saved` va `callee-saved` registrlar farqi nima?
**Javob:** Funksiyani chaqiruvchi o'zi saqlashi kerak bo'lgan registrlar — `caller-saved` (chaqirilgan funksiya ularni buzishi mumkin). Chaqirilgan funksiya qaytishdan oldin tiklab berishi shart bo'lganlari — `callee-saved` (17.4). Bu qoida bo'lmasa, funksiyalar bir-birining qiymatlarini buzib yuborardi.

**Savol:** Inline assembly qachon ishlatiladi?
**Javob:** C'da ifodalab bo'lmaydigan bitta buyruq kerak bo'lganda (`rdtsc`, `cli`/`sti`, port I/O). Xato qilsangiz kompilyator sizning registr o'zgarishingizni bilmay qoladi — shuning uchun `clobber` ro'yxati to'g'ri yozilishi shart (17.5).

## O'zingizni tekshiring

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

## Mashq

- 3–4 ta kichik funksiyani (`strlen`, `max`, massiv yig'indisi) `gcc -O0 -S -masm=intel` va `-O2` bilan kompilyatsiya qilib, har bir qatorni izohlang.
- `gdb` da `layout asm` va `stepi` bilan dasturni buyruqma-buyruq bajaring, `info registers` ni kuzating.
- `kernel/proc/switch.asm` ni o'qib, har bir qatorni o'z so'zingiz bilan yozing.

<!-- loyiha:boshi -->
## Loyiha: stekli virtual mashina

**Maqsad:** protsessor ichidagi tsiklni — **chaqir → dekodla → bajar** — o'z qo'lingiz bilan yozish. Assembly'da
`push`, `add`, `jnz` nima qilishini bu mashinada C bilan ko'rasiz.
**Bobdan ishlatiladi:** buyruq (opcode) va operand, stek, dastur hisoblagichi (`pc`), shartli sakrash (17.1–17.2).

**Talab:** kichik "dasturlash tili" (bayt-kod): `PUSH n`, `POP`, `ADD`, `SUB`, `MUL`, `DUP`, `PRINT`, `JNZ manzil`, `HALT`.
- **Stek** hisoblash joyi: `ADD` yuqoridagi ikkitasini olib, yig'indisini qo'yadi.
- **`JNZ manzil`** stek tepasini oladi; **nol emas** bo'lsa `pc = manzil`. Sikl shu bilan quriladi.
- Har bir xato (stek bo'shab qolishi yoki to'lishi, noma'lum buyruq, cheksiz sikl) aniqlansin.

**Ma'lumotlar:** `struct komanda { enum op op; int arg; }`, `int stek[32]`, `sp` (stek ko'rsatkichi), `pc`.

```c
/* vm.c - stekli virtual mashina */
#include <stdio.h>

enum op { PUSH, POP, ADD, SUB, MUL, DUP, PRINT, JNZ, HALT };
static const char *nomlar[] = { "PUSH", "POP", "ADD", "SUB", "MUL", "DUP", "PRINT", "JNZ", "HALT" };

struct komanda {
    enum op op;
    int arg;
};

static void stek_chiqar(const int *stek, int sp)
{
    printf("stek:");
    for (int i = 0; i < sp; i++)
        printf(" %d", stek[i]);
    printf("\n");
}

static int yur(const struct komanda *dastur, int iz)
{
    int stek[32], sp = 0, pc = 0;
    for (int qadam = 0; qadam < 1000; qadam++) {        /* qadam chegarasi: cheksiz siklga qarshi */
        struct komanda k = dastur[pc++];
        int a, b;
        switch (k.op) {
        case PUSH:
            if (sp == 32) { printf("XATO: stek to'ldi\n"); return -1; }
            stek[sp++] = k.arg;
            break;
        case POP:
            if (sp < 1) { printf("XATO: stek bo'sh\n"); return -1; }
            sp--;
            break;
        case ADD: case SUB: case MUL:
            if (sp < 2) { printf("XATO: stekda 2 ta qiymat yo'q\n"); return -1; }
            b = stek[--sp];
            a = stek[--sp];
            stek[sp++] = k.op == ADD ? a + b : k.op == SUB ? a - b : a * b;
            break;
        case DUP:
            if (sp < 1 || sp == 32) { printf("XATO: DUP\n"); return -1; }
            stek[sp] = stek[sp - 1];
            sp++;
            break;
        case PRINT:
            if (sp < 1) { printf("XATO: stek bo'sh\n"); return -1; }
            printf("  => %d\n", stek[--sp]);
            break;
        case JNZ:
            if (sp < 1) { printf("XATO: stek bo'sh\n"); return -1; }
            if (stek[--sp] != 0)
                pc = k.arg;
            break;
        case HALT:
            return 0;
        default:
            printf("XATO: noma'lum buyruq\n");
            return -1;
        }
        if (iz) {
            printf("  %-5s %-3d | ", nomlar[k.op], k.arg);
            stek_chiqar(stek, sp);
        }
    }
    printf("XATO: qadamlar chegarasi (cheksiz sikl?)\n");
    return -1;
}

int main(void)
{
    /* (2 + 3) * 4 */
    struct komanda hisob[] = { { PUSH, 2 }, { PUSH, 3 }, { ADD, 0 }, { PUSH, 4 }, { MUL, 0 },
                               { PRINT, 0 }, { HALT, 0 } };
    printf("Dastur 1: (2 + 3) * 4, izlash bilan\n");
    yur(hisob, 1);

    /* 5 dan 1 gacha sanash: sikl JNZ bilan */
    struct komanda sanoq[] = { { PUSH, 5 },                  /* 0 */
                               { DUP, 0 },                   /* 1  <- sikl boshi */
                               { PRINT, 0 },                 /* 2 */
                               { PUSH, 1 }, { SUB, 0 },      /* 3, 4 */
                               { DUP, 0 },                   /* 5 */
                               { JNZ, 1 },                   /* 6: nol emas bo'lsa 1 ga qayt */
                               { HALT, 0 } };                /* 7 */
    printf("Dastur 2: 5 dan 1 gacha sanash\n");
    yur(sanoq, 0);

    struct komanda xato[] = { { ADD, 0 }, { HALT, 0 } };
    printf("Dastur 3: xato\n");
    yur(xato, 0);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g -fsanitize=address,undefined vm.c -o vm
$ ./vm
Dastur 1: (2 + 3) * 4, izlash bilan
  PUSH  2   | stek: 2
  PUSH  3   | stek: 2 3
  ADD   0   | stek: 5
  PUSH  4   | stek: 5 4
  MUL   0   | stek: 20
  => 20
  PRINT 0   | stek:
Dastur 2: 5 dan 1 gacha sanash
  => 5
  => 4
  => 3
  => 2
  => 1
Dastur 3: xato
XATO: stekda 2 ta qiymat yo'q
```

Bu — protsessorning **modeli**: `switch` — dekodlovchi, `stek[]` — registrlar/stek, `pc` — dastur hisoblagichi, `JNZ` — xuddi x86 dagi
shartli sakrash (`jnz`). JVM va Python bayt-kodi ham aynan shu tuzilishda.

**Kengaytiring:** `JZ` (nol bo'lsa sakra) va `SWAP` buyruqlarini qo'shing. 1 dan 5 gacha faktorial dasturini yozing.

## Mustaqil loyiha: 8 bitli ALU va bayroqlar ★★★

**Vazifa:** protsessorning arifmetik qurilmasi (ALU) natijaning o'zini ham, **bayroqlarni** ham beradi. Har
shartli sakrash buyrug'i (`jz`, `jc`, `jo`, `js`) shu bayroqlarga qaraydi. Sizning vazifangiz — 8 bitli
`add`, `sub`, `adc` ni bayroqlar bilan yozish. Fayl: `alu.c`.

**Tuzilma:** `struct natija { uint8_t q; int z, c, n, v; };` — `q` natija; bayroqlar `0` yoki `1`:

| Bayroq | Ma'nosi |
|---|---|
| `Z` (zero) | natija `0` |
| `C` (carry) | qo'shishda: 8-bitdan **ortiqcha tashish** bor. Ayirishda: **qarz** (borrow) bor, ya'ni `a < b` (ishorasiz) |
| `N` (negative) | natijaning eng katta biti (7-bit) `1` |
| `V` (overflow) | ishorali (`int8_t`) hisobda **toshish**: musbat+musbat=manfiy yoki manfiy+manfiy=musbat (ayirishda mos qoida) |

**Funksiyalar:**
- `struct natija add8(uint8_t a, uint8_t b)`
- `struct natija sub8(uint8_t a, uint8_t b)` (`a − b`)
- `struct natija adc8(uint8_t a, uint8_t b, int tashish)` — `a + b + tashish` (16 bitli qo'shish uchun)

**16 bitli qo'shish:** `add8` bilan pastki baytlar, keyin `adc8` bilan yuqori baytlar (pastkidan chiqqan `C` ni uzating).

**Chiqish shakli aniq:**

**Kutilgan natija** (`darslik/loyihalar/17_alu/kutilgan.txt`):

```text
Qo'shish:
add8(0x7F, 0x01) = 0x80  Z=0 C=0 N=1 V=1
add8(0xFF, 0x01) = 0x00  Z=1 C=1 N=0 V=0
add8(0x80, 0x80) = 0x00  Z=1 C=1 N=0 V=1
add8(0x10, 0x20) = 0x30  Z=0 C=0 N=0 V=0
add8(0xC8, 0x64) = 0x2C  Z=0 C=1 N=0 V=0
Ayirish:
sub8(0x05, 0x03) = 0x02  Z=0 C=0 N=0 V=0
sub8(0x03, 0x05) = 0xFE  Z=0 C=1 N=1 V=0
sub8(0x80, 0x01) = 0x7F  Z=0 C=0 N=0 V=1
sub8(0x00, 0x00) = 0x00  Z=1 C=0 N=0 V=0
sub8(0x7F, 0xFF) = 0x80  Z=0 C=1 N=1 V=1
16 bitli qo'shish:
  0x12FF + 0x0001 = 0x1300  (chiqish tashishi C=0)
  0xFFFF + 0x0001 = 0x0000  (chiqish tashishi C=1)
```

Sinovlar: `add8` — `(0x7F,0x01)`, `(0xFF,0x01)`, `(0x80,0x80)`, `(0x10,0x20)`, `(0xC8,0x64)`;
`sub8` — `(0x05,0x03)`, `(0x03,0x05)`, `(0x80,0x01)`, `(0x00,0x00)`, `(0x7F,0xFF)`;
16 bitli — `0x12FF + 0x0001` va `0xFFFF + 0x0001`.

**Maslahat** (yechim emas):
- Natijani 9 bitda hisoblang: `unsigned t = a + b;` — 8-bitdan ortiqcha bit (`t > 0xFF`) — bu `C`. `q = (uint8_t)t`.
- `V` qoidasi qo'shishda: `a` va `b` **bir xil** ishorali, natijaning ishorasi ularnikidan **boshqa**:
  `((a ^ q) & (b ^ q) & 0x80) != 0`. Qog'ozda `0x7F + 0x01` da tekshiring.
- Ayirishda `V`: `a` va `b` **turli** ishorali va natijaning ishorasi `a` nikidan boshqa: `((a ^ b) & (a ^ q) & 0x80) != 0`.
- `adc8` qo'shishga o'xshash, faqat uchinchi qo'shiluvchi. `C` ni to'g'ri hisoblash uchun `unsigned t = a + b + tashish`.
- Bayroqlarni qo'lda tasdiqlash: `int8_t` ga cast qilib ishorali qiymatni o'ylang: `0x7F` = 127, `0x80` = −128.

**Tekshirish:**

```bash
gcc -Wall -Wextra -g -fsanitize=address,undefined alu.c -o dastur && ./dastur | diff - ~/C_loyha/darslik/loyihalar/17_alu/kutilgan.txt && echo "TO'G'RI"
```
<!-- loyiha:oxiri -->

Keyingi bob: [18-bob. Freestanding C: yadroga ko'prik](18-yadroga-koprik.md)
