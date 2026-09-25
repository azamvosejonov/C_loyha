# 06 — Jarayonlar, kontekst almashtirish va scheduler

**Kod:** `kernel/proc/process.c`, `kernel/proc/switch.asm`, `kernel/drivers/console.c` (uxlash)

## Jarayon nima? (yadro nuqtai nazaridan)

Yadro uchun jarayon — bu `struct process`:

| Maydon | Nima uchun |
|---|---|
| `kernel_rsp` | to'xtatilganda yadro stekining qayeri — **butun holat shu stekda** |
| `kstack_base` | har bir jarayonning **o'z** 16 KB yadro steki |
| `pml4` | manzil maydoni (CR3) |
| `state` | UNUSED / EMBRYO / READY / RUNNING / BLOCKED / ZOMBIE |
| `wait_channel` | nimani kutyapti |
| `parent`, `exit_code` | wait/exit uchun |

## Kontekst almashtirish — 12 qator assembly

```nasm
context_switch:          ; (uint64_t *old_rsp, uint64_t new_rsp)
    push rbx / rbp / r12 / r13 / r14 / r15    ; eski jarayonning callee-saved registrlari
    mov [rdi], rsp                            ; eski RSP ni saqlash
    mov rsp, rsi                              ; ► boshqa jarayonning stekiga o'tish ◄
    pop r15 / r14 / r13 / r12 / rbp / rbx     ; uning registrlari
    ret                                       ; uning schedule() chaqiruvidan "qaytish"
```

**Nega faqat 6 ta registr?** `context_switch` oddiy C funksiyasi kabi chaqiriladi. System V ABI bo'yicha
chaqiruvchi RAX, RCX, RDX, RSI, RDI, R8–R11 buzilishini **kutadi** (caller-saved). Faqat RBX, RBP, R12–R15
(callee-saved) saqlanishi shart. RIP esa `call` qo'ygan qaytish manzili sifatida stekda turibdi.

**Asosiy fikr:** har bir jarayon `schedule()` → `context_switch()` ichida "muzlab" turadi. Unga qaytib
o'tilganda u go'yo oddiy funksiyadan qaytgandek davom etadi.

### Yangi oqim qanday boshlanadi?

Yangi oqim hech qachon `context_switch` ichida bo'lmagan. Shuning uchun uning stekini **qo'lda** shunday
tayyorlaymiz: go'yo u o'sha yerda to'xtab turgan bo'lsin:

```
yuqori  │ 0                    │
        │ kthread_trampoline   │ ← `ret` shu yerga sakraydi
        │ RBX=0                │
        │ RBP=0                │
        │ R12=arg              │ ← trampolin fn(arg) ni chaqiradi
        │ R13=fn               │
        │ R14=0                │
past    │ R15=0                │ ← kernel_rsp
```

7-bosqichda user jarayon uchun ham xuddi shu hiyla ishlatiladi: qaytish manzili = `interrupt_return`, uning
ustida esa soxta `interrupt_frame` turadi (`iretq` → ring 3).

## Scheduler: round-robin + preemption

```
taymer IRQ (har 10 ms) → sched_tick() → quantum_left-- → 0 bo'lsa: state=READY, schedule()
```

- `pick_next()`: joriydan keyingisidan boshlab aylana bo'ylab birinchi `READY`.
- Hech kim tayyor bo'lmasa → **idle** (pid 0 = `kmain`) → `sti; hlt`: CPU keyingi uzilishgacha uxlaydi.
- Kvant = 5 tik = 50 ms.

**Yagona qoida:** `schedule()` faqat `IF=0` holda chaqiriladi va `IF=0` holda qaytadi. Bitta protsessorli
tizimda jarayonlar jadvalini himoya qilish uchun shu yetarli.

### EOI nega handlerdan oldin?

Taymer handleri `schedule()` orqali boshqa jarayonga o'tib ketadi va bu uzilishdan ancha vaqt "qaytmaydi".
Agar EOI yuborilmagan bo'lsa, PIC keyingi taymer uzilishlarini yubormaydi va tizim qotadi
(`interrupts.c` ga qarang).

## Uxlash va uyg'otish (Unix "kanal" modeli)

```c
// o'quvchi (console_getc)                // yozuvchi (klaviatura IRQ)
flags = irq_save();                        buffer[head++] = c;
while (bufer_bo'sh)                        proc_wakeup(&input_head);
    proc_sleep_on(&input_head);
c = buffer[tail++];
irq_restore(flags);
```

- **Kanal** — istalgan manzil, "nima kutilayotgani" belgisi.
- Shart **IF=0 holda** tekshiriladi va uxlash ham IF=0 da boshlanadi, shuning uchun "lost wakeup"
  bo'lmaydi (02-bosqichdagi `sti; hlt` bilan bir xil g'oya).
- `while` (`if` emas!): uyg'onganimizda shart hali ham bajarilmagan bo'lishi mumkin (boshqa jarayon belgini
  bizdan oldin olib ketgan). Bu **spurious wakeup** deyiladi; pthread'dagi condition variable bilan ham
  aynan shunday ishlanadi.

## Exit, zombie va wait

1. `proc_exit()`: user manzil maydoni **darhol** qaytariladi, jarayon esa `ZOMBIE` holatiga o'tadi.
2. Yadro steki qaytarilmaydi: biz hozir **o'sha stekda** turibmiz.
3. Ota-ona `proc_wait()` da o'z manzili ustida uxlaydi va bola uni uyg'otadi.
4. Ota-ona zombie'ni topib, chiqish kodini oladi va stekni bo'shatadi (`proc_free`).
5. Yetim zombie'larni (ota-onasi o'lgan) **idle** tozalaydi.

## Kill — nega darhol o'ldirib bo'lmaydi?

Boshqa jarayon hozir yadro ichida turgan, masalan `pmm_alloc_frame()` o'rtasida, bitmap bilan ishlayotgan
bo'lishi mumkin. Uni shu yerda to'xtatsak, tizim buzilgan holatda qoladi. Shuning uchun `proc_kill()` faqat
**belgi** qo'yadi (`killed = true`). Jarayon uni xavfsiz nuqtada — user rejimiga qaytishdan oldin — ko'rib,
o'zi `exit` qiladi (7-bosqich). Linux'dagi signallar ham shunday yetkaziladi.

## Testlar (`APPEND=selftest`)

| Test | Nimani isbotlaydi |
|---|---|
| 3 oqim × 1000 marta `counter++` = 3000 | kontekst almashtirish registrlarni buzmaydi, `irq_save` atomarlikni ta'minlaydi |
| sleeper ≥ 10 tik | taymer bo'yicha uyg'otish |
| spinner (hech qachon yield qilmaydi) boshqa oqim o'rnatgan flag'ni ko'radi | **preemption** ishlaydi |
| `wait(-1)` bolasiz → -1 | abadiy osilib qolmaslik |
| PMM hisoblagichi o'zgarmagan | yadro steklari oqib ketmaydi |

## Sinab ko'ring

1. `counter_thread` dagi `irq_save/irq_restore` ni olib tashlang. Test yiqiladimi? Nega ba'zan yiqilmaydi?
   (Maslahat: `shared_counter++` qanday instruksiyalarga aylanadi? `objdump -d` qiling.)
2. `SCHED_QUANTUM` ni 1 va 100 qilib, ticker chiqishini solishtiring.
3. **Mashq:** ustuvorlik (priority) qo'shing. **Katta mashq:** SMP — ikkinchi CPU yadrosini ishga tushiring
   (spinlock, per-CPU `current`).
