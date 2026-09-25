; =============================================================================
;  arch/trampoline.asm - AP (qo'shimcha CPU yadrolari) uchun boshlang'ich kod
; =============================================================================
;
;  Kompyuter yoqilganda faqat BITTA yadro (BSP - Bootstrap Processor) ishlaydi.
;  Qolganlari (AP - Application Processors) "uxlab" yotadi. BSP ularni
;  uyg'otish uchun LAPIC orqali INIT va SIPI (Startup IPI) yuboradi. SIPI da
;  sahifa raqami beriladi: vektor 0x08 -> AP 0x8000 manzilidan boshlaydi.
;
;  MUAMMO: AP 1978-yildagidek 16-BITLI REAL MODE'da uyg'onadi! Shuning uchun
;  u ham BSP bosib o'tgan yo'lni takrorlashi kerak:
;       16-bit real -> 32-bit protected -> 64-bit long mode -> yadro (ap_main)
;
;  Bu kod yadro ichida ma'lumot sifatida saqlanadi va smp.c uni fizik 0x8000
;  ga NUSXALAYDI. Shuning uchun barcha manzillar "0x8000 + (belgi - boshi)"
;  shaklida hisoblanadi - kod qayerga link qilinganidan qat'i nazar ishlashi uchun.
;  Oxiridagi ma'lumot maydonlarini (CR3, stek, kirish nuqtasi) smp.c to'ldiradi.
; =============================================================================

TRAMP_BASE equ 0x8000
%define ADDR(x) (TRAMP_BASE + ((x) - trampoline_start))

section .rodata
global trampoline_start
global trampoline_end
global tramp_cr3, tramp_stack, tramp_entry, tramp_cpu, tramp_efer

bits 16
trampoline_start:
    cli
    cld
    xor ax, ax
    mov ds, ax                          ; DS = 0: ADDR() lar to'g'ridan-to'g'ri fizik manzil
    lgdt [ADDR(tr_gdt_ptr)]             ; vaqtinchalik GDT
    mov eax, cr0
    or eax, 1                           ; PE - protected mode
    mov cr0, eax
    jmp dword 0x08:ADDR(tr_pm32)        ; 32-bitli kod segmentiga uzoq sakrash

bits 32
tr_pm32:
    mov ax, 0x10                        ; 32-bitli ma'lumot segmenti
    mov ds, ax
    mov es, ax
    mov ss, ax

    mov eax, cr4
    or eax, 1 << 5                      ; PAE
    mov cr4, eax

    mov eax, [ADDR(tramp_cr3)]          ; BSP tayyorlagan jadval (identity 0..2MB + yadro)
    mov cr3, eax

    mov ecx, 0xC0000080                 ; EFER
    rdmsr
    or eax, [ADDR(tramp_efer)]          ; LME (+ NXE agar CPU qo'llasa) - BSP qiymati
    wrmsr

    mov eax, cr0
    or eax, (1 << 31) | (1 << 16)       ; paging + WP
    mov cr0, eax
    jmp 0x18:ADDR(tr_lm64)              ; 64-bitli kod segmenti

bits 64
tr_lm64:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov rsp, [ADDR(tramp_stack)]        ; AP ning yadro steki (vmalloc, yuqori yarimda)
    xor rbp, rbp
    mov rdi, [ADDR(tramp_cpu)]          ; ap_main(struct cpu *)
    mov rax, [ADDR(tramp_entry)]
    jmp rax                             ; >>> yuqori yarimdagi C kodiga <<<

align 8
tr_gdt:
    dq 0                                ; null
    dq 0x00CF9A000000FFFF               ; 0x08: 32-bitli kod, 4 GB
    dq 0x00CF92000000FFFF               ; 0x10: ma'lumot, 4 GB
    dq 0x00209A0000000000               ; 0x18: 64-bitli kod
tr_gdt_ptr:
    dw tr_gdt_ptr - tr_gdt - 1
    dd ADDR(tr_gdt)

align 8
tramp_cr3:   dq 0
tramp_stack: dq 0
tramp_entry: dq 0
tramp_cpu:   dq 0
tramp_efer:  dd 0
trampoline_end:
