; =============================================================================
;  proc/switch.asm - KONTEKST ALMASHTIRISH (context switch)
; =============================================================================
;
;  MULTITASKING SIRI SHU 12 QATORDA.
;
;  Har bir jarayonning O'Z YADRO STEKI bor. Jarayon to'xtaganda uning butun
;  holati (qayerda to'xtagani, registrlari) shu stekda qoladi. Boshqa jarayonga
;  o'tish = RSP ni boshqa stekka o'zgartirish.
;
;  void context_switch(uint64_t *old_rsp, uint64_t new_rsp);
;     RDI = &eski_jarayon->kernel_rsp  (eski RSP qayerga saqlansin)
;     RSI = yangi_jarayon->kernel_rsp  (yangi RSP)
;
;  NEGA FAQAT 6 TA REGISTR:
;    System V ABI bo'yicha funksiya chaqiruvida RBX, RBP, R12-R15 "callee-saved":
;    chaqirilgan funksiya ularni saqlab qaytarishi SHART. Qolganlari (RAX, RCX,
;    RDX, RSI, RDI, R8-R11) "caller-saved": chaqiruvchi ular buzilishini kutadi.
;    context_switch oddiy C funksiyasi kabi chaqiriladi, shuning uchun faqat
;    callee-saved registrlarni saqlash yetarli! RIP esa `call` qo'ygan qaytish
;    manzili sifatida stekda allaqachon turibdi.
;
;  STEK KO'RINISHI (to'xtatilgan jarayonda):
;       ...
;       qaytish manzili  <- context_switch ni kim chaqirgan (schedule ichida)
;       RBX
;       RBP
;       R12
;       R13
;       R14
;       R15              <- saqlangan kernel_rsp shu yerga ko'rsatadi
;
;  "Sehr": `ret` yangi stekdagi qaytish manzilini oladi - ya'ni biz BOSHQA
;  jarayonning schedule() chaqiruvidan "qaytamiz". Har bir jarayon o'zini
;  go'yo oddiy funksiyadan qaytgandek his qiladi.
; =============================================================================

bits 64
section .text

global context_switch
context_switch:
    push rbx                    ; callee-saved registrlarni ESKI stekka
    push rbp
    push r12
    push r13
    push r14
    push r15
    mov [rdi], rsp              ; eski RSP ni saqlaymiz
    mov rsp, rsi                ; >>> SHU YERDA BOSHQA JARAYONGA O'TDIK <<<
    pop r15                     ; YANGI stekdan uning registrlarini tiklaymiz
    pop r14
    pop r13
    pop r12
    pop rbp
    pop rbx
    ret                         ; yangi jarayon to'xtagan joyga qaytish

; -----------------------------------------------------------------------------
;  kthread_trampoline - yangi yadro oqimining birinchi instruksiyasi.
;
;  proc_create_kernel_thread() yangi stekni shunday tayyorlaydi: go'yo bu oqim
;  context_switch ichida to'xtab turgan, qaytish manzili esa shu trampolin.
;  R12 = argument, R13 = funksiya.
; -----------------------------------------------------------------------------
extern proc_exit
global kthread_trampoline
kthread_trampoline:
    sti                         ; schedule() uzilishlar o'chiq holda ishlaydi - yangi
                                ; oqim ularni o'zi yoqadi
    mov rdi, r12                ; fn(arg) - birinchi argument RDI da
    and rsp, -16                ; ABI: call oldidan RSP 16 ga karrali
    call r13                    ; oqim funksiyasi
    mov edi, eax                ; qaytgan qiymat = chiqish kodi
    call proc_exit              ; oqim tugadi (bu funksiya qaytmaydi)
.hang:
    hlt
    jmp .hang
