# 02 — Uzilishlar: GDT, TSS, IDT, exception'lar va apparat IRQ'lari

**Kod:** `kernel/arch/{gdt.c,gdt_load.asm,idt.c,isr.asm,interrupts.c}`,
`kernel/drivers/{pic.c,pit.c,keyboard.c,console.c}`

## Uzilish nima va nega u OS'ning markazida turadi

CPU instruksiyalarni ketma-ket bajaradi. **Uzilish** bu oqimni to'xtatib, CPU'ni boshqa kodga
(handler) yuboradi, keyin esa aynan to'xtagan joyiga qaytaradi. Uch turi bor:

| Tur | Kim yaratadi | Misol | Vektor |
|---|---|---|---|
| **Exception** | CPU o'zi, instruksiya bajarishda | 0 ga bo'lish, page fault, noto'g'ri instruksiya | 0–31 |
| **Apparat IRQ** | qurilma, PIC orqali | taymer, klaviatura, serial port | 32–47 (biz ko'chirdik) |
| **Dasturiy** | `int N` instruksiyasi | bizning syscall'imiz: `int 0x80` | 0x80 |

Yadroning deyarli hamma ishi uzilishdan boshlanadi: taymer → scheduler, klaviatura → kiritish,
`int 0x80` → syscall, page fault → xotira boshqaruvi.

## GDT va TSS (`gdt.c`)

64-bitli rejimda segmentlar deyarli o'lik, lekin GDT hali ham ikki narsani hal qiladi:

1. **Imtiyoz darajasi.** CS registridagi segment ring 0 (yadro) yoki ring 3 (user) ekanini bildiradi.
   User dasturni ishga tushirish = `iretq` orqali CS ga `0x23` (user kodi, RPL=3) yuklash.
2. **TSS.** User rejimida uzilish kelsa, CPU yadro stekini `TSS.rsp0` dan oladi. User stekiga
   ishonib bo'lmaydi — u ataylab buzilgan bo'lishi mumkin. Scheduler har bir jarayonga o'tganda
   `tss_set_kernel_stack()` chaqiradi.

**IST (Interrupt Stack Table):** double fault uchun alohida stek. Agar yadro steki to'lib ketsa
(cheksiz rekursiya), page fault handleri ham shu to'lgan stekka yoza olmaydi → double fault. IST bo'lmasa
bu **triple fault**ga, ya'ni kompyuterning qayta yuklanishiga olib keladi. IST bilan esa hech bo'lmaganda
xabar chiqara olamiz.

## IDT va stub'lar (`idt.c`, `isr.asm`)

IDT 256 ta darvozadan iborat. Har bir vektor uchun `isr.asm` kichik stub generatsiya qiladi (NASM `%rep`):

```
isr_stub_14:            ; page fault - CPU xato kodini o'zi qo'yadi
    push 14             ; vektor
    jmp isr_common
isr_stub_32:            ; taymer - xato kodi yo'q
    push 0              ; soxta xato kodi (stek bir xil ko'rinishi uchun)
    push 32
    jmp isr_common
```

`isr_common` 15 ta registrni saqlaydi va `interrupt_dispatch(frame)` ni chaqiradi. Stek tuzilishi
`struct interrupt_frame` bilan **aynan** mos bo'lishi kerak — `interrupts.h` va `isr.asm` ni yonma-yon
o'qing.

### Stek tekisligi (alignment) — kichik, lekin halokatli tafsilot

System V ABI: `call` qilishdan oldin RSP 16 ga karrali bo'lishi kerak. CPU uzilishda RSP ni 16 ga
tekislab 40 bayt qo'yadi; stub va `isr_common` yana 136 bayt qo'shadi: jami 176 = 16×11. Tekis.
Agar bitta `push` qo'shsangiz, ba'zi SSE instruksiyalar (agar ishlatilsa) kutilmaganda qulaydi —
bunday xatolarni topish juda qiyin.

## PIC (`pic.c`)

BIOS IRQ'larni 8–15 vektorlariga qo'ygan, ular esa CPU exception'lari bilan to'qnashadi (IRQ0 = taymer
→ vektor 8 = double fault!). Shuning uchun PIC'ni qayta dasturlab, IRQ'larni 32–47 ga ko'chiramiz.

**EOI** (End Of Interrupt): har bir IRQ'dan keyin PIC'ga "tayyor" deymiz. Biz uni handlerdan **oldin**
yuboramiz — taymer handleri boshqa jarayonga o'tib ketishi mumkin (6-bosqich), va EOI kechiksa,
tizim qotadi.

## Taymer (`pit.c`)

PIT 1 193 182 Hz dan bo'lib, sekundiga 100 marta IRQ0 yuboradi. Bu **preemptive multitasking**ning
asosi: cheksiz tsiklga tushgan dastur ham har 10 ms da yadroga boshqaruvni qaytarishga majbur.

## Klaviatura va serial kiritish (`keyboard.c`, `console.c`)

Klaviatura **scancode** beradi (tugmaning raqami), drayver uni Shift/Caps Lock'ni hisobga olib ASCII'ga
aylantiradi. Serial port (IRQ4) terminaldan kelgan baytlarni beradi — shu tufayli avtomatik testlar
shell'ga buyruq "yoza" oladi.

Ikkala manba ham **halqali buferga** (ring buffer) yozadi. `console_getc()` bufer bo'sh bo'lsa kutadi.

### "Lost wakeup" muammosi va `sti; hlt`

```c
cli();
if (bufer_bo'sh) {
    // <- agar uzilish shu yerda kelsa va biz uni sezmasak...
    sti(); hlt();   // ... abadiy uxlab qolishimiz mumkin
}
```

x86'da `sti` ning ta'siri **keyingi instruksiyadan keyin** boshlanadi. Shuning uchun `sti; hlt`
atomar: uzilish ular orasiga tusha olmaydi. Bu klassik muammo; 6-bosqichda jarayonlarni uxlatish va
uyg'otishda ham aynan shu fikrni qo'llaymiz.

## Sinab ko'ring

1. `kmain` ga `volatile int z = 0; kprintf("%d", 5 / z);` yozing — "Divide Error" va registrlar dump'ini ko'ring.
2. `*(volatile int *)0x40000000 = 1;` — page fault. CR2 va xato kodi tushuntirishini o'qing.
3. `pic_send_eoi` chaqiruvini o'chiring. Taymer necha marta tik qiladi? Nega?
4. `isr.asm` da `push qword 0` ni olib tashlang (faqat taymer uchun). Nima buziladi?
