; =============================================================================
;  user/lib/crt0.asm - "C runtime 0": har bir user dasturning HAQIQIY boshlanishi
; =============================================================================
;
;  main() - bu C dasturchisi uchun boshlanish. CPU uchun esa boshlanish - ELF
;  sarlavhasidagi e_entry, ya'ni shu _start. Yadro (proc/exec.c) bizni shunday
;  holatda qoldiradi:
;      RDI = argc, RSI = argv, RSP = 16 ga tekislangan user steki
;
;  Vazifamiz: main(argc, argv) ni chaqirish va u qaytargan qiymat bilan
;  exit() qilish. Linux'da glibc'ning crt1.o si xuddi shu ishni qiladi (yana
;  atexit, TLS, stdio buferlarini sozlash kabi ko'p narsalar bilan).
; =============================================================================

bits 64
section .text.start                     ; linker.ld uni .text ning eng boshiga qo'yadi

extern main
extern exit

global _start
_start:
    xor rbp, rbp                        ; freym zanjiri oxiri (debugger/backtrace uchun)
    call main                           ; RDI, RSI allaqachon argc, argv - tegmaymiz
    mov edi, eax                        ; main ning qaytgan qiymati = chiqish kodi
    call exit                           ; hech qachon qaytmaydi
.hang:
    jmp .hang
