; =============================================================================
;  mustaqil/dastur.asm - B12 bosqichi uchun TAYYOR user dasturi
; =============================================================================
;
;  Buni siz yozmaysiz: tools/mustaqil.py uni yig'adi (nasm + ld, dastur.ld) va
;  GRUB orqali yadroingizga Multiboot2 MODULI sifatida beradi ("dastur" nomi bilan).
;  Sizning vazifangiz - ELF yuklovchi: modulni topish, PT_LOAD segmentlarini
;  xaritalash, ring 3 ga o'tish va quyidagi ABI bo'yicha syscall'larga javob berish.
;
;  ABI (Linux x86-64 raqamlari bilan bir xil - keyin Linux'ni o'qish osonroq):
;      rax = syscall raqami, argumentlar: rdi, rsi, rdx; natija: rax
;      1  = write(fd, buf, len)  -> yozilgan baytlar soni (len)
;      60 = exit(kod)            -> qaytmaydi
;  Sukut bo'yicha syscall `int 0x80` bilan qilinadi. `syscall` buyrug'ini
;  (MSR: EFER.SCE, STAR, LSTAR, SFMASK) qilgan bo'lsangiz:
;      tools/mustaqil.py tekshir B12 --abi syscall
;
;  Dastur nimani tekshiradi (sizning yuklovchingiz xatosini ushlash uchun):
;      1) write qaytargan qiymat len ga tengmi       -> syscall natijasi rax da qaytdimi
;      2) .bss 512 bayti nolmi                        -> p_memsz > p_filesz qismi tozalanganmi
;      3) .data dagi 41 ni 42 ga oshirish             -> data segmenti nusxalanganmi VA yoziladimi
;      4) push/pop                                     -> user steki ishlaydimi
;      5) exit(7)                                      -> yadro "B12 exit=7" deb chiqarishi kerak
; =============================================================================
bits 64

%ifdef SYSCALL_ABI
    %define SYS syscall
%else
    %define SYS int 0x80
%endif

; YOZ satr, uzunlik - write(1, satr, uzunlik)
%macro YOZ 2
    mov rax, 1
    mov rdi, 1
    lea rsi, [rel %1]
    mov rdx, %2
    SYS
%endmacro

section .text
global _start
_start:
    ; 1) salom va write natijasi
    YOZ salom, salom_len
    cmp rax, salom_len
    je .write_ok
    YOZ write_xato, write_xato_len
.write_ok:

    ; 2) .bss nolmi?
    lea rsi, [rel bss_boshi]
    mov rcx, BSS_HAJMI
    xor eax, eax
.bss_sikl:
    or al, [rsi]
    inc rsi
    dec rcx
    jnz .bss_sikl
    test al, al
    jnz .bss_yomon
    YOZ bss_ok, bss_ok_len
    jmp .hisob_tekshir
.bss_yomon:
    YOZ bss_xato, bss_xato_len

    ; 3) .data: 41 -> 42 (segment yoziladigan bo'lishi kerak)
.hisob_tekshir:
    inc qword [rel hisob]
    cmp qword [rel hisob], 42
    jne .data_yomon
    YOZ data_ok, data_ok_len
    jmp .exit
.data_yomon:
    YOZ data_xato, data_xato_len

    ; 4) stek va 5) exit(7)
.exit:
    push 7
    pop rdi
    mov rax, 60
    SYS
    ; exit qaytmasligi kerak edi
    YOZ exit_xato, exit_xato_len
.osil:
    jmp .osil

section .data
salom:       db "B12 ELF dan salom", 10
salom_len    equ $ - salom
write_xato:  db "B12 write noto'g'ri qiymat qaytardi", 10
write_xato_len equ $ - write_xato
bss_ok:      db "B12 bss=0", 10
bss_ok_len   equ $ - bss_ok
bss_xato:    db "B12 bss=xato", 10
bss_xato_len equ $ - bss_xato
data_ok:     db "B12 data=42", 10
data_ok_len  equ $ - data_ok
data_xato:   db "B12 data=xato", 10
data_xato_len equ $ - data_xato
exit_xato:   db "B12 exit qaytib keldi", 10
exit_xato_len equ $ - exit_xato
align 8
hisob:       dq 41

section .bss
BSS_HAJMI    equ 512
bss_boshi:   resb BSS_HAJMI
