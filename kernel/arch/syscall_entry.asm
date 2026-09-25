; =============================================================================
;  arch/syscall_entry.asm - `syscall` INSTRUKSIYASI UCHUN KIRISH NUQTASI
; =============================================================================
;
;  `int 0x80` o'rniga zamonaviy `syscall` instruksiyasi (Linux x86-64 shuni
;  ishlatadi). Farqi - TEZLIK: `int` IDT ni o'qiydi, stekni TSS dan oladi,
;  5 ta qiymatni stekka yozadi, ruxsatlarni tekshiradi... `syscall` esa
;  minimal ish qiladi:
;       RCX    <- qaytish manzili (RIP)
;       R11    <- RFLAGS
;       RIP    <- LSTAR MSR (shu fayldagi syscall_entry)
;       CS, SS <- STAR MSR dan
;       RFLAGS &= ~SFMASK (biz IF ni o'chiramiz)
;  DIQQAT: STEK ALMASHMAYDI! RSP hali ham USER stekiga ko'rsatadi. Yadro
;  stekiga o'zimiz o'tishimiz kerak - buning uchun per-CPU (gs:8) kerak.
;
;  Biz interrupt_frame bilan AYNAN BIR XIL stek tuzilishini quramiz. Shunda
;  syscall C kodi, fork, signallar va hk. `int 0x80` bilan ham, `syscall`
;  bilan ham bir xil ishlaydi.
; =============================================================================

bits 64
section .text

%define CPU_KERNEL_RSP 8
%define CPU_USER_RSP   16

; frame ichidagi siljishlar (arch/interrupts.h: r15..rax, vector, err, rip, cs, rflags, rsp, ss)
%define F_VECTOR 120
%define F_RIP    136
%define F_CS     144
%define F_RFLAGS 152
%define F_RSP    160

extern syscall_dispatch
extern interrupt_return

global syscall_entry
syscall_entry:
    swapgs                              ; GS -> yadro per-CPU
    mov [gs:CPU_USER_RSP], rsp          ; user RSP ni vaqtincha saqlaymiz
    mov rsp, [gs:CPU_KERNEL_RSP]        ; joriy jarayonning yadro steki (TSS.rsp0 bilan bir xil)

    ; --- int 0x80 bilan bir xil freym ---
    push qword 0x1B                     ; SS  (user ma'lumoti | 3)
    push qword [gs:CPU_USER_RSP]        ; RSP
    push r11                            ; RFLAGS (syscall uni R11 ga qo'ygan)
    push qword 0x23                     ; CS  (user kodi | 3)
    push rcx                            ; RIP (syscall uni RCX ga qo'ygan)
    push qword 0                        ; xato kodi
    push qword 0x80                     ; "vektor" - int 0x80 bilan bir xil
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

    mov rdi, rsp                        ; struct interrupt_frame *
    cld
    call syscall_dispatch               ; (u ichida uzilishlar yoqiladi)

    ; --- Qaytish ---
    cli                                 ; swapgs va sysret orasida uzilish kelmasin!

    ; SYSRET faqat "oddiy" holatda ishlatiladi. Aks holda (exec yoki signal
    ; RIP ni o'zgartirgan bo'lsa) xavfsiz, lekin sekinroq iretq yo'li.
    ;
    ; MASHHUR ZAIFLIK (CVE-2012-0217): Intel CPU'larda KANONIK BO'LMAGAN RIP
    ; bilan sysret qilinsa, #GP xatosi RING 0 da, lekin USER stekida sodir
    ; bo'ladi -> hujumchi yadro kodini bajarishi mumkin edi. FreeBSD, Xen,
    ; Windows shundan zarar ko'rgan. Shuning uchun RIP ni tekshiramiz.
    ; sigreturn barcha registrlarni (RCX va R11 ni ham) tiklashi kerak - sysret
    ; esa ularni buzadi. Shunday holatda C kodi "vektor"ni 0x81 qilib qo'yadi.
    cmp qword [rsp + F_VECTOR], 0x80
    jne .slow_path
    mov rcx, [rsp + F_RIP]
    mov r11, 0x00007FFFFFFFFFFF
    cmp rcx, r11
    ja .slow_path
    cmp qword [rsp + F_CS], 0x23
    jne .slow_path

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11                             ; (keyin RFLAGS bilan almashtiriladi)
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx                             ; (keyin RIP bilan almashtiriladi)
    pop rbx
    pop rax                             ; syscall natijasi
    ; Endi [rsp] = vektor, +8 xato, +16 RIP, +24 CS, +32 RFLAGS, +40 RSP
    mov rcx, [rsp + 16]                 ; sysret RIP ni RCX dan oladi
    mov r11, [rsp + 32]                 ; va RFLAGS ni R11 dan
    mov rsp, [rsp + 40]                 ; user steki
    swapgs                              ; GS -> user
    o64 sysret                          ; ring 3 ga qaytish

.slow_path:
    jmp interrupt_return                ; iretq yo'li (u o'zi swapgs qiladi)
