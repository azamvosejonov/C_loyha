; =============================================================================
;  boot.asm - Multiboot2 kirish nuqtasi: 32-bit -> 64-bit -> HIGHER HALF
; =============================================================================
;
;  GRUB (BIOS yoki UEFI rejimida) bizni shunday holatda qoldiradi:
;    - CPU 32-bitli protected mode, paging O'CHIQ
;    - EAX = 0x36D76289 (Multiboot2 "magic"), EBX = boot ma'lumotlari fizik manzili
;
;  Bizning vazifa:
;    1. CPU 64-bit (Long Mode) ni qo'llashini tekshirish
;    2. VAQTINCHALIK sahifa jadvallarini qurish (uchta xarita, rasmga qarang)
;    3. PAE, (NX), Long Mode, Paging ni yoqish
;    4. 64-bitli kodga o'tish va YUQORI YARIMDAGI (0xFFFFFFFF8...) kodga sakrash
;    5. kmain(magic, mbi_phys) ni chaqirish
;
;  VAQTINCHALIK XARITA (2 MB lik sahifalar bilan):
;
;     virtual                               fizik        nega
;     0x0000000000000000 - 1 GB       ->   0 - 1 GB     identity: paging yoqilgan
;                                                       lahzada shu kod ishlashda davom etishi uchun
;     0xFFFF800000000000 - +4 GB      ->   0 - 4 GB     direct map (HHDM): C kodi
;                                                       fizik xotirani shu orqali o'qiydi
;     0xFFFFFFFF80000000 - +1 GB      ->   0 - 1 GB     yadro tasviri (linker.ld)
;
;  vmm.c keyinroq ANIQ va XAVFSIZ jadvallarni quradi (4 KB aniqlik, W^X,
;  butun RAM) va identity xaritani butunlay olib tashlaydi.
; =============================================================================

MB2_MAGIC         equ 0xE85250D6        ; Multiboot2 sarlavha magic
MB2_ARCH_I386     equ 0                 ; 32-bitli protected mode'da chaqiring
MB2_BOOT_MAGIC    equ 0x36D76289        ; GRUB EAX ga shuni qo'yadi

KERNEL_VMA        equ 0xFFFFFFFF80000000
PTE_P             equ 1 << 0            ; present
PTE_W             equ 1 << 1            ; writable
PTE_PS            equ 1 << 7            ; 2 MB lik "katta" sahifa

; =============================================================================
;  Multiboot2 sarlavhasi. Tuzilishi: 4 ta majburiy maydon + "teglar" ro'yxati.
;  Har bir teg 8 baytga tekislangan: [u16 tur][u16 bayroq][u32 hajm][ma'lumot].
; =============================================================================
section .multiboot2 progbits alloc noexec nowrite align=8
align 8
mb2_start:
    dd MB2_MAGIC
    dd MB2_ARCH_I386
    dd mb2_end - mb2_start                              ; sarlavha hajmi
    dd 0x100000000 - (MB2_MAGIC + MB2_ARCH_I386 + (mb2_end - mb2_start))   ; yig'indi = 0

    ; --- Framebuffer tegi (5): "grafik rejim ber, iloji bo'lsa 1024x768x32" ---
    ; bayroq 1 = IXTIYORIY: bajarib bo'lmasa ham yuklanaver (matn rejimi qoladi).
    ; UEFI kompyuterlarda VGA matn rejimi YO'Q - faqat framebuffer.
align 8
    dw 5, 1
    dd 20
    dd 1024, 768, 32

    ; --- Modullarni sahifa chegarasiga tekislash tegi (6) ---
align 8
    dw 6, 0
    dd 8

    ; --- Oxirgi teg ---
align 8
    dw 0, 0
    dd 8
mb2_end:

; =============================================================================
;  Past xotiradagi .bss: vaqtinchalik jadvallar va 32-bitli qism uchun stek.
;  Bu yerga faqat FIZIK manzil orqali murojaat qilinadi.
; =============================================================================
section .boot.bss nobits alloc noexec write align=4096
align 4096
boot_pml4:       resb 4096
boot_pdpt_low:   resb 4096              ; PML4[0]   -> identity
boot_pdpt_hhdm:  resb 4096              ; PML4[256] -> direct map
boot_pdpt_high:  resb 4096              ; PML4[511] -> yadro
boot_pd:         resb 4096 * 4          ; 4 ta PD = 4 GB (2 MB x 512 x 4)
boot_stack32:    resb 4096
boot_stack32_top:
boot_magic:      resd 1
boot_mbi:        resd 1

; =============================================================================
;  Past xotiradagi GDT (32-bitli LGDT faqat 32-bitli baza manzilini o'qiydi,
;  shuning uchun u past manzilda bo'lishi kerak).
; =============================================================================
section .boot.rodata progbits alloc noexec nowrite align=8
align 8
boot_gdt:
    dq 0                                                ; null
    dq (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53)    ; 0x08: 64-bitli kod
    dq (1 << 41) | (1 << 44) | (1 << 47)                ; 0x10: ma'lumot
boot_gdt_ptr:
    dw $ - boot_gdt - 1
    dq boot_gdt

; =============================================================================
;  32-bitli kod (past manzil)
; =============================================================================
section .boot.text progbits alloc exec nowrite align=16
bits 32
global _start
_start:
    cli
    mov esp, boot_stack32_top
    mov [boot_magic], eax
    mov [boot_mbi], ebx

    cmp eax, MB2_BOOT_MAGIC
    jne .no_multiboot

    ; --- CPUID bormi? (EFLAGS.ID bitini o'zgartirib ko'ramiz) ---
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 1 << 21
    push eax
    popfd
    pushfd
    pop eax
    push ecx
    popfd
    cmp eax, ecx
    je .no_cpuid

    ; --- Long Mode bormi? ---
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_long_mode
    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29                   ; LM
    jz .no_long_mode
    mov esi, edx                        ; EDX 20-bit = NX. Keyinroq kerak - saqlaymiz

    call setup_page_tables

    ; --- PAE ---
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; --- CR3 ---
    mov eax, boot_pml4
    mov cr3, eax

    ; --- EFER: LME (Long Mode) + NXE (agar CPU qo'llasa) ---
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8                      ; LME
    test esi, 1 << 20                   ; CPU NX ni qo'llaydimi?
    jz .no_nx
    or eax, 1 << 11                     ; NXE - "bajarib bo'lmaydi" bitiga ruxsat
.no_nx:
    wrmsr

    ; --- Paging + Write Protect ---
    mov eax, cr0
    or eax, (1 << 31) | (1 << 16)
    mov cr0, eax

    lgdt [boot_gdt_ptr]
    jmp 0x08:long_mode_low              ; 64-bitli kod segmentiga uzoq sakrash

.no_multiboot:
    mov al, 'M'
    jmp boot_error
.no_cpuid:
    mov al, 'C'
    jmp boot_error
.no_long_mode:
    mov al, 'L'
    jmp boot_error

; -----------------------------------------------------------------------------
;  setup_page_tables - yuqoridagi uch xaritani qurish
; -----------------------------------------------------------------------------
setup_page_tables:
    ; PML4 yozuvlari (har biri 8 bayt; yuqori 4 bayt .bss dan 0)
    mov eax, boot_pdpt_low
    or eax, PTE_P | PTE_W
    mov [boot_pml4 + 0 * 8], eax
    mov eax, boot_pdpt_hhdm
    or eax, PTE_P | PTE_W
    mov [boot_pml4 + 256 * 8], eax
    mov eax, boot_pdpt_high
    or eax, PTE_P | PTE_W
    mov [boot_pml4 + 511 * 8], eax

    ; identity: PDPT_low[0] -> PD 0 (0..1 GB)
    mov eax, boot_pd
    or eax, PTE_P | PTE_W
    mov [boot_pdpt_low], eax
    ; yadro: 0xFFFFFFFF80000000 = PML4[511], PDPT[510] -> PD 0
    mov [boot_pdpt_high + 510 * 8], eax

    ; direct map: PDPT_hhdm[0..3] -> PD 0..3 (0..4 GB)
    mov ecx, 0
.hhdm_loop:
    mov eax, ecx
    shl eax, 12                         ; i * 4096
    add eax, boot_pd
    or eax, PTE_P | PTE_W
    mov [boot_pdpt_hhdm + ecx * 8], eax
    inc ecx
    cmp ecx, 4
    jne .hhdm_loop

    ; 4 ta PD ni to'ldirish: 2048 ta yozuv, i-chi = i * 2 MB.
    ; 4 GB dan yuqori bit (32-bit) 4-yozuvning yuqori dword'iga tushadi - bizda
    ; eng katta manzil 4 GB - 2 MB, ya'ni 32 bitga sig'adi.
    mov ecx, 0
.pd_loop:
    mov eax, ecx
    shl eax, 21                         ; i * 2 MB
    or eax, PTE_P | PTE_W | PTE_PS
    mov [boot_pd + ecx * 8], eax
    inc ecx
    cmp ecx, 2048
    jne .pd_loop
    ret

; -----------------------------------------------------------------------------
;  boot_error: VGA matn ekraniga "ERR: X" (faqat BIOS rejimida ko'rinadi).
; -----------------------------------------------------------------------------
boot_error:
    mov dword [0xb8000], 0x4f524f45
    mov dword [0xb8004], 0x4f3a4f52
    mov dword [0xb8008], 0x4f204f20
    mov byte  [0xb800a], al
.halt:
    hlt
    jmp .halt

; =============================================================================
;  64-bitli kod, hali PAST manzilda (identity xarita tufayli ishlaydi)
; =============================================================================
bits 64
long_mode_low:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax
    mov rax, higher_half_entry          ; 64-bitli ABSOLYUT manzil (0xFFFFFFFF8...)
    jmp rax                             ; >>> yuqori yarimga sakrash <<<

; =============================================================================
;  Yuqori yarimdagi kod (oddiy .text bo'limi - KERNEL_VMA da link qilingan)
; =============================================================================
section .text
extern kmain
global boot_stack_guard
global boot_stack_top

higher_half_entry:
    mov rsp, boot_stack_top             ; yuqori yarimdagi yadro steki
    xor rbp, rbp                        ; backtrace zanjiri oxiri
    mov edi, [boot_magic]               ; 32-bitli MOV yuqori qismini nollaydi
    mov esi, [boot_mbi]
    call kmain
.hang:
    cli
    hlt
    jmp .hang

section .bss
align 4096
boot_stack_guard:
    resb 4096                           ; HIMOYA SAHIFASI: vmm.c uni xaritalamaydi
boot_stack_bottom:
    resb 16384                          ; 16 KB boot steki (keyinchalik idle jarayonniki)
boot_stack_top:
