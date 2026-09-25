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
;  new_proc_start - YANGI jarayonning birinchi instruksiyasi.
;
;  Scheduler yangi jarayonga proc_lock ni USHLAB TURGAN holda o'tadi (xv6
;  modeli). Oddiy jarayon uni schedule() dan qaytgach qo'yib yuboradi; yangi
;  jarayon esa hech qachon schedule() da bo'lmagan - shuning uchun qulfni shu
;  yerda qo'yib yuboramiz, keyin R13 dagi davomga sakraymiz:
;     user jarayon  -> interrupt_return (iretq bilan ring 3 ga)
;     yadro oqimi   -> kthread_start
; -----------------------------------------------------------------------------
extern proc_first_run
extern proc_exit
global new_proc_start
new_proc_start:
    call proc_first_run                 ; proc_lock ni qo'yib yuborish (uzilishlar yoqiladi)
    jmp r13

; R12 = argument, R14 = funksiya
global kthread_start
kthread_start:
    sti
    mov rdi, r12                        ; fn(arg)
    and rsp, -16                        ; ABI: call oldidan RSP 16 ga karrali
    call r14
    mov edi, eax                        ; qaytgan qiymat = chiqish kodi
    call proc_exit                      ; qaytmaydi
.hang:
    hlt
    jmp .hang
