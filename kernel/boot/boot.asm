; =============================================================================
;  boot.asm - yadroning ENG BIRINCHI bajariladigan kodi
; =============================================================================
;
;  KOMPYUTER YOQILGANDA NIMA BO'LADI:
;    1. CPU 16-bitli "real mode"da uyg'onadi (1978-yilgi 8086 bilan moslik uchun).
;    2. BIOS apparatni tekshiradi va yuklovchini (GRUB yoki QEMU'ning ichki
;       yuklovchisini) ishga tushiradi.
;    3. Yuklovchi bizning yadro faylimizda Multiboot sarlavhasini qidiradi, yadroni
;       1 MB manzilga yuklaydi, CPU'ni 32-bitli "protected mode"ga o'tkazadi va
;       _start ga sakraydi.
;
;  BU FAYL NIMA QILADI (32-bit -> 64-bit):
;    a) CPU 64-bitni qo'llab-quvvatlashini tekshiradi (CPUID)
;    b) Sahifa jadvallarini (page tables) quradi - 64-bitli rejimda paging MAJBURIY
;    c) PAE, Long Mode va Paging'ni yoqadi
;    d) 64-bitli GDT yuklaydi va 64-bitli kodga "uzoq sakrash" (far jump) qiladi
;    e) C tilidagi kmain() funksiyasini chaqiradi
;
;  NEGA ASSEMBLY:
;    C kompilyatori 32-bit va 64-bit kodni bitta funksiyada aralashtira olmaydi,
;    CR0/CR3/CR4 registrlari va MSR'larga to'g'ridan-to'g'ri murojaat ham kerak.
;    Shuning uchun bu kichik qismni qo'lda yozamiz.
; =============================================================================

; ---- Multiboot 1 sarlavhasi konstantalari ----------------------------------------
MB_MAGIC    equ 0x1BADB002              ; Yuklovchi yadroni shu "sehrli son" orqali taniydi
MB_FLAGS    equ (1 << 0) | (1 << 1)     ; bit0: modullarni 4 KB chegarasiga tekislash
                                        ; bit1: bizga xotira xaritasini (memory map) berish
MB_CHECKSUM equ -(MB_MAGIC + MB_FLAGS)  ; magic + flags + checksum = 0 bo'lishi SHART

; Yuklovchi bizga EAX registrida shu qiymatni beradi - "men Multiboot yuklovchiman".
MB_BOOTLOADER_MAGIC equ 0x2BADB002

; Sahifa jadvali yozuvi bayroqlari (Intel SDM, 4.5-bo'lim):
PAGE_PRESENT  equ 1 << 0                ; P  - sahifa xotirada mavjud
PAGE_WRITABLE equ 1 << 1                ; RW - yozish mumkin
PAGE_HUGE     equ 1 << 7                ; PS - "katta sahifa": PD darajasida 2 MB sahifa

; =============================================================================
;  .multiboot bo'limi - linker.ld uni fayl boshiga qo'yadi
; =============================================================================
section .multiboot
align 4                                 ; Multiboot talabi: sarlavha 4 baytga tekislangan
    dd MB_MAGIC                         ; dd = "define double word" (4 bayt)
    dd MB_FLAGS
    dd MB_CHECKSUM

; =============================================================================
;  .bss bo'limi - nol bilan to'ldirilgan xotira (fayldan joy olmaydi)
; =============================================================================
section .bss
align 4096                              ; Sahifa jadvallari 4 KB ga tekislangan bo'lishi SHART
                                        ; (CR3 va yozuvlar faqat manzilning yuqori bitlarini saqlaydi)
global boot_pml4                        ; vmm.c yadroning asosiy jadvalini bilishi uchun
global boot_pd
boot_pml4:  resb 4096                   ; 4-daraja: Page Map Level 4 (512 ta yozuv x 8 bayt)
boot_pdpt:  resb 4096                   ; 3-daraja: Page Directory Pointer Table
boot_pd:    resb 4096                   ; 2-daraja: Page Directory (bizda 2 MB lik sahifalar)

align 4096
global boot_stack_guard
boot_stack_guard:                       ; HIMOYA SAHIFASI (guard page). vmm.c uni xaritadan
    resb 4096                           ; olib tashlaydi: stek to'lib ketsa, bu yerga yozish
                                        ; darhol page fault beradi (aks holda stek jimgina
                                        ; yuqoridagi sahifa jadvallarini buzardi!)
boot_stack_bottom:                      ; (4096 ga tekis, demak 16 ga ham - ABI talabi)
    resb 16384                          ; 16 KB stek. Stek PASTGA o'sadi (top -> bottom)
global boot_stack_top
boot_stack_top:

mb_magic:   resd 1                      ; EAX dagi multiboot "magic"ni saqlab qo'yamiz
mb_info:    resd 1                      ; EBX dagi multiboot_info strukturasining manzili

; =============================================================================
;  .rodata - vaqtinchalik 64-bitli GDT (Global Descriptor Table)
; =============================================================================
;  GDT - CPU'ga "segmentlar" haqida aytadigan jadval. 64-bitli rejimda segmentlar
;  deyarli ishlatilmaydi, lekin CPU baribir kod segmenti "64-bitli" (L bit)
;  ekanini ko'rishi kerak. Keyinroq arch/gdt.c to'liq GDT (user segmentlari va TSS
;  bilan) yuklaydi; bu faqat 64-bitga o'tish uchun.
section .rodata
align 8
gdt64:
    dq 0                                ; 0-yozuv: har doim NULL (CPU talabi)
.code: equ $ - gdt64                    ; .code = 8 (selektor - jadvaldagi bayt siljishi)
    dq (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53)
                                        ; 43: bajariladigan (kod segmenti)
                                        ; 44: kod/ma'lumot segmenti (tizim emas)
                                        ; 47: mavjud (present)
                                        ; 53: L - 64-bitli kod segmenti
.data: equ $ - gdt64                    ; .data = 16
    dq (1 << 41) | (1 << 44) | (1 << 47)
                                        ; 41: yozish mumkin, 44 va 47 yuqoridagidek
.pointer:                               ; LGDT instruksiyasi o'qiydigan struktura:
    dw $ - gdt64 - 1                    ;   2 bayt: jadval hajmi - 1
    dq gdt64                            ;   8 bayt: jadval manzili

; =============================================================================
;  32-bitli kod
; =============================================================================
section .text
bits 32                                 ; NASM'ga: quyidagi kodni 32-bitli qilib yig'
global _start
extern kmain                            ; kernel/main.c da aniqlangan

_start:
    cli                                 ; Uzilishlarni o'chiramiz - hali IDT yo'q, uzilish
                                        ; kelsa CPU "triple fault" bilan qayta yuklanadi
    mov esp, boot_stack_top             ; O'z stekimizni o'rnatamiz (yuklovchining stekiga
                                        ; ishonib bo'lmaydi - u qayerdaligini bilmaymiz)

    mov [mb_magic], eax                 ; EAX va EBX ni saqlaymiz, chunki quyidagi
    mov [mb_info], ebx                  ; CPUID instruksiyasi ularni buzadi

    cmp eax, MB_BOOTLOADER_MAGIC        ; Bizni haqiqatan Multiboot yuklovchi yukladimi?
    jne .no_multiboot

    call check_cpuid                    ; CPUID instruksiyasi bormi?
    call check_long_mode                ; CPU 64-bitni qo'llaydimi?
    call setup_page_tables              ; Birinchi 1 GB ni "identity map" qilamiz
    call enable_paging                  ; PAE + Long Mode + Paging

    lgdt [gdt64.pointer]                ; 64-bitli GDT ni yuklaymiz
    jmp gdt64.code:long_mode_start      ; "Far jump": CS registriga 64-bitli kod segmentini
                                        ; yuklaydi. Shu lahzadan CPU 64-bitli rejimda!

.no_multiboot:
    mov al, 'M'                         ; Xato kodi: M = Multiboot emas
    jmp boot_error

; -----------------------------------------------------------------------------
;  check_cpuid: EFLAGS dagi 21-bit (ID) ni o'zgartirib bo'lsa, CPUID bor.
;  (Juda eski CPU'larda bu bitni o'zgartirib bo'lmaydi.)
; -----------------------------------------------------------------------------
check_cpuid:
    pushfd                              ; EFLAGS ni stekka
    pop eax                             ; ... va EAX ga
    mov ecx, eax                        ; Asl nusxasini ECX da saqlaymiz
    xor eax, 1 << 21                    ; ID bitini teskarisiga o'zgartiramiz
    push eax
    popfd                               ; O'zgartirilgan qiymatni EFLAGS ga yozamiz
    pushfd
    pop eax                             ; EFLAGS ni qayta o'qiymiz
    push ecx
    popfd                               ; Asl EFLAGS ni tiklaymiz
    cmp eax, ecx                        ; Bit o'zgardimi?
    je .no_cpuid                        ; O'zgarmagan bo'lsa - CPUID yo'q
    ret
.no_cpuid:
    mov al, 'C'
    jmp boot_error

; -----------------------------------------------------------------------------
;  check_long_mode: CPUID 0x80000001 funksiyasining EDX 29-biti = Long Mode (64-bit).
; -----------------------------------------------------------------------------
check_long_mode:
    mov eax, 0x80000000                 ; "Eng katta kengaytirilgan funksiya raqami qancha?"
    cpuid
    cmp eax, 0x80000001                 ; 0x80000001 umuman mavjudmi?
    jb .no_long_mode
    mov eax, 0x80000001                 ; Kengaytirilgan protsessor ma'lumoti
    cpuid
    test edx, 1 << 29                   ; LM (Long Mode) biti
    jz .no_long_mode
    ret
.no_long_mode:
    mov al, 'L'
    jmp boot_error

; -----------------------------------------------------------------------------
;  setup_page_tables: virtual manzil = fizik manzil ("identity mapping")
;  ko'rinishida birinchi 1 GB ni xaritalaymiz.
;
;  64-bitli manzil tarjimasi 4 darajali:
;     virtual manzil bitlari:  [47..39] [38..30] [29..21] [20..0]
;                                PML4     PDPT      PD     sahifa ichidagi siljish
;  PD yozuvida PS (huge) biti o'rnatilsa, u to'g'ridan-to'g'ri 2 MB lik sahifani
;  ko'rsatadi va 4-daraja (PT) kerak bo'lmaydi. 512 ta x 2 MB = 1 GB.
;
;  NEGA IDENTITY: paging yoqilgan zahoti CPU keyingi instruksiyani VIRTUAL manzildan
;  o'qiydi. Agar kodimiz turgan manzil o'ziga xaritalanmagan bo'lsa - darhol xato.
; -----------------------------------------------------------------------------
setup_page_tables:
    mov eax, boot_pdpt
    or eax, PAGE_PRESENT | PAGE_WRITABLE
    mov [boot_pml4], eax                ; PML4[0] -> PDPT  (0..512 GB oralig'i)

    mov eax, boot_pd
    or eax, PAGE_PRESENT | PAGE_WRITABLE
    mov [boot_pdpt], eax                ; PDPT[0] -> PD    (0..1 GB oralig'i)

    xor ecx, ecx                        ; ecx = i = 0 (hisoblagich)
.map_pd_entry:
    mov eax, 0x200000                   ; 2 MB
    mul ecx                             ; eax = i * 2 MB  (edx ham o'zgaradi, u 0 bo'ladi)
    or eax, PAGE_PRESENT | PAGE_WRITABLE | PAGE_HUGE
    mov [boot_pd + ecx * 8], eax        ; PD[i] = i-chi 2 MB sahifa (yozuv 8 bayt; yuqori
                                        ; 4 bayti .bss dan 0 - 4 GB dan past manzil)
    inc ecx
    cmp ecx, 512                        ; 512 ta yozuv = 1 GB
    jne .map_pd_entry
    ret

; -----------------------------------------------------------------------------
;  enable_paging: 64-bitli rejimga o'tish uchun Intel talab qilgan ketma-ketlik
; -----------------------------------------------------------------------------
enable_paging:
    mov eax, boot_pml4
    mov cr3, eax                        ; CR3 = eng yuqori jadval (PML4) ning fizik manzili

    mov eax, cr4
    or eax, 1 << 5                      ; CR4.PAE - 64-bitli sahifa yozuvlari (Long Mode talabi)
    mov cr4, eax

    mov ecx, 0xC0000080                 ; EFER MSR (Extended Feature Enable Register)
    rdmsr                               ; EDX:EAX = MSR[ECX]
    or eax, 1 << 8                      ; EFER.LME - Long Mode Enable
    wrmsr

    mov eax, cr0
    or eax, (1 << 31) | (1 << 16)       ; 31: PG - Paging yoqish (shu lahzada Long Mode faollashadi)
                                        ; 16: WP - yadro ham "faqat o'qish" sahifalarga yoza olmaydi
    mov cr0, eax
    ret

; -----------------------------------------------------------------------------
;  boot_error: ekranga "ERR: X" yozib, to'xtaydi. AL = xato kodi.
;  VGA matn xotirasi 0xB8000 da: har bir belgi 2 bayt (ASCII + rang).
;  0x4f = oq harf, qizil fon.
; -----------------------------------------------------------------------------
boot_error:
    mov dword [0xb8000], 0x4f524f45     ; 'E','R' (little-endian: 45='E', 52='R')
    mov dword [0xb8004], 0x4f3a4f52     ; 'R',':'
    mov dword [0xb8008], 0x4f204f20     ; ' ',' '
    mov byte  [0xb800a], al             ; xato kodi harfi
.halt:
    hlt                                 ; CPU'ni to'xtatish
    jmp .halt                           ; (NMI uni uyg'otsa, yana to'xtatamiz)

; =============================================================================
;  64-bitli kod
; =============================================================================
bits 64
long_mode_start:
    mov ax, gdt64.data                  ; Barcha ma'lumot segment registrlariga 64-bitli
    mov ss, ax                          ; GDT dagi data selektorini yuklaymiz.
    mov ds, ax                          ; (64-bitli rejimda ular deyarli e'tiborga
    mov es, ax                          ;  olinmaydi, lekin eski 32-bitli qiymatlar
    mov fs, ax                          ;  qolib ketmasligi kerak)
    mov gs, ax

    mov rsp, boot_stack_top             ; 32->64 o'tishda registrlarning yuqori 32 biti
                                        ; noaniq bo'ladi - RSP ni to'liq qayta yuklaymiz
    xor rbp, rbp                        ; RBP = 0: backtrace shu yerda to'xtaydi (zanjir oxiri)

    ; System V AMD64 ABI: birinchi argument RDI da, ikkinchisi RSI da.
    ; 32-bitli MOV yuqori 32 bitni avtomatik nollaydi.
    mov edi, [mb_magic]                 ; kmain(magic, ...)
    mov esi, [mb_info]                  ; kmain(..., multiboot_info_phys)
    call kmain                          ; C dunyosiga xush kelibsiz!

    ; kmain hech qachon qaytmasligi kerak. Qaytsa - CPU'ni abadiy to'xtatamiz.
.hang:
    cli
    hlt
    jmp .hang
