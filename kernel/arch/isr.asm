; =============================================================================
;  arch/isr.asm - 256 ta uzilish "kirish eshigi" (stub) va umumiy handler
; =============================================================================
;
;  UZILISH KELGANDA CPU NIMA QILADI (64-bitli rejimda):
;    1. Agar user rejimda (ring 3) bo'lsak - TSS.rsp0 dagi yadro stekiga o'tadi.
;       (IST ko'rsatilgan bo'lsa - o'sha stekka.)
;    2. RSP ni 16 baytga tekislaydi.
;    3. Stekka 5 ta qiymat qo'yadi:  SS, RSP, RFLAGS, CS, RIP
;       (qayerda to'xtaganimizni eslab qolish uchun)
;    4. Ba'zi exception'lar (8, 10-14, 17, 21, 29, 30) qo'shimcha "xato kodi" ham qo'yadi.
;    5. IDT dan handler manzilini oladi va unga sakraydi. Interrupt gate bo'lsa,
;       IF=0 qiladi (boshqa uzilishlar handler ichida kelmaydi).
;
;  MUAMMO: C funksiyasi qaysi uzilish kelganini bilmaydi va stek tuzilishi xato
;  kodi bor-yo'qligiga qarab turlicha. YECHIM: har bir vektor uchun kichik "stub":
;    - xato kodi bo'lmasa, soxta 0 qo'yadi (stek har doim bir xil ko'rinishda bo'lsin)
;    - vektor raqamini qo'yadi
;    - umumiy isr_common ga sakraydi
;
;  isr_common barcha umumiy registrlarni saqlaydi va C dagi interrupt_dispatch()
;  ga "struct interrupt_frame *" beradi. Stek oxirida shunday ko'rinadi:
;
;     yuqori manzil   SS
;                     RSP        <- CPU qo'ygan
;                     RFLAGS
;                     CS
;                     RIP
;                     xato kodi  <- CPU yoki stub (0)
;                     vektor     <- stub
;                     RAX ... R15 <- isr_common
;     past manzil     (RSP shu yerga ko'rsatadi = struct boshi)
;
;  Bu tartib arch/interrupts.h dagi struct interrupt_frame bilan AYNAN mos
;  bo'lishi SHART. Birini o'zgartirsangiz, ikkinchisini ham o'zgartiring!
; =============================================================================

bits 64
section .text

extern interrupt_dispatch

; ---- Xato kodi QO'YMAYDIGAN exception/uzilish uchun stub ----
%macro ISR_NO_ERROR 1
isr_stub_%1:
    push qword 0                ; soxta xato kodi
    push qword %1               ; vektor raqami
    jmp isr_common
%endmacro

; ---- CPU o'zi xato kodi qo'yadigan exception uchun stub ----
%macro ISR_WITH_ERROR 1
isr_stub_%1:
    push qword %1               ; faqat vektor (xato kodi allaqachon stekda)
    jmp isr_common
%endmacro

; ---- 256 ta stubni NASM preprotsessori bilan generatsiya qilamiz ----
%assign i 0
%rep 256
%if i == 8 || i == 10 || i == 11 || i == 12 || i == 13 || i == 14 || i == 17 || i == 21 || i == 29 || i == 30
    ISR_WITH_ERROR i
%else
    ISR_NO_ERROR i
%endif
%assign i i+1
%endrep

; ---- Umumiy qism ----
isr_common:
    ; Barcha umumiy registrlarni saqlaymiz. C kodi ularni buzadi, lekin uzilgan
    ; kod qaytganda hech narsa sezmasligi kerak - go'yo uzilish bo'lmagandek.
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rdi, rsp                ; 1-argument: struct interrupt_frame * (stek boshi)
    cld                         ; ABI talabi: C funksiyaga kirishda DF (yo'nalish bayrog'i) = 0
    ; Stek tekisligi: CPU RSP ni 16 ga tekislab 5*8=40 bayt qo'ydi, keyin
    ; 2*8 (xato+vektor) + 15*8 (registrlar) = 136. Jami 176 = 16*11 - tekis.
    ; call qaytish manzilini qo'yadi - ABI aynan shuni kutadi.
    call interrupt_dispatch

; interrupt_return - bu belgi GLOBAL: proc/process.c yangi user jarayonini
; "go'yo uzilishdan qaytayotgandek" shu yerdan boshlaydi (06-jarayonlar.md).
global interrupt_return
interrupt_return:
    pop r15                     ; teskari tartibda tiklaymiz
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    add rsp, 16                 ; vektor va xato kodini tashlab yuboramiz
    iretq                       ; RIP, CS, RFLAGS, RSP, SS ni tiklash - uzilgan joyga qaytish.
                                ; CS ichidagi daraja 3 bo'lsa, CPU avtomatik ring 3 ga qaytadi!

; ---- Stublar manzillari jadvali (idt.c undan IDT ni to'ldiradi) ----
section .rodata
global isr_stub_table
isr_stub_table:
%assign i 0
%rep 256
    dq isr_stub_ %+ i
%assign i i+1
%endrep
