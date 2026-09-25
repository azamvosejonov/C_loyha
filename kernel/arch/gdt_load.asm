; =============================================================================
;  arch/gdt_load.asm - yangi GDT ni yuklash va segment registrlarini yangilash
; =============================================================================
;
;  NEGA ASSEMBLY: CS registrini oddiy MOV bilan o'zgartirib bo'lmaydi. Uni faqat
;  "uzoq" o'tish (far jump / far return) yangilaydi. C da bunday operator yo'q.
; =============================================================================

bits 64
section .text

; void gdt_load(const struct gdt_pointer *ptr, uint64_t code_sel, uint64_t data_sel)
;   RDI = ptr, RSI = code_sel, RDX = data_sel  (System V ABI tartibi)
global gdt_load
gdt_load:
    lgdt [rdi]                  ; GDTR registriga yangi jadval manzili va hajmi

    mov ax, dx                  ; data selektori (0x10)
    mov ds, ax                  ; barcha ma'lumot segment registrlarini yangilaymiz
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; CS ni yangilash hiylasi: stekka (CS, RIP) juftligini qo'yib, "far return"
    ; (retfq) qilamiz. retfq stekdan avval RIP ni, keyin CS ni oladi.
    push rsi                    ; yangi CS (0x08)
    lea rax, [rel .reload_cs]   ; qaytish manzili = pastdagi belgi
    push rax                    ; RIP
    retfq                       ; RIP <- .reload_cs, CS <- 0x08
.reload_cs:
    ret                         ; endi oddiy qaytish - C kodiga

; void tss_load(uint64_t tss_sel)
;   LTR - "Load Task Register": CPU'ga TSS qayerdaligini aytadi.
global tss_load
tss_load:
    ltr di                      ; TR <- 0x28
    ret
