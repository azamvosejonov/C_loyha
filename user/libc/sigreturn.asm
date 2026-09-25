; =============================================================================
;  user/libc/sigreturn.asm - signal handler'dan qaytish "trampolini"
; =============================================================================
;
;  Yadro handler'ni chaqirganda user stekiga qaytish manzili sifatida shu
;  funksiyani qo'yadi (sa_restorer). Handler `ret` qilganda bu yerga keladi va
;  sigreturn syscall'i yadroga "saqlangan registrlarni tikla" deydi.
;  Oddiy C funksiya bo'la olmaydi: u stekka hech narsa qo'ymasligi kerak -
;  RSP aynan yadro yozgan freymga ko'rsatib turishi shart.
; =============================================================================
bits 64
section .text

SYS_SIGRETURN equ 46

global __restore_rt
__restore_rt:
    mov eax, SYS_SIGRETURN
    syscall
    ud2                                 ; bu yerga hech qachon kelmaymiz
