; asm_funk.asm - assembly funksiyalari (NASM, Intel sintaksisi, x86-64 System V chaqirish qoidasi)
; Argumentlar: rdi, rsi, rdx, rcx, r8, r9. Natija: rax. Saqlanishi shart registrlar: rbx, rbp, r12-r15 (biz ularga tegmaymiz).

global yig_massiv
global satr_uzunligi
global popcount64
global bayt_almashtir32

section .text

; long yig_massiv(const int *a, long n)  - int massiv elementlari yig'indisi
yig_massiv:
    xor eax, eax                    ; rax = 0 (yig'indi); xor o'zi bilan - registrni nolga tushirishning tez yo'li
    test rsi, rsi                   ; n == 0 ?
    jle .tugadi
.sikl:
    movsxd rdx, dword [rdi]         ; int ni 64 bitga ishora bilan kengaytirib o'qiymiz
    add rax, rdx
    add rdi, 4                      ; keyingi element (int = 4 bayt)
    dec rsi
    jnz .sikl
.tugadi:
    ret

; long satr_uzunligi(const char *s)  - '\0' gacha bayt soni (strlen)
satr_uzunligi:
    xor eax, eax
.sikl:
    cmp byte [rdi + rax], 0
    je .tugadi
    inc rax
    jmp .sikl
.tugadi:
    ret

; int popcount64(unsigned long x)  - yoniq bitlar soni: x & (x - 1) eng pastki yoniq bitni o'chiradi
popcount64:
    xor eax, eax
.sikl:
    test rdi, rdi
    jz .tugadi
    lea rdx, [rdi - 1]              ; rdx = x - 1
    and rdi, rdx                    ; x &= x - 1
    inc eax
    jmp .sikl
.tugadi:
    ret

; unsigned bayt_almashtir32(unsigned x)  - baytlar tartibini teskari qiladi (little <-> big endian)
bayt_almashtir32:
    mov eax, edi
    bswap eax
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
