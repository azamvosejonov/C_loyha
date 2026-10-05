# demo.s - ELF belgilar jadvalini o'rganish uchun kichik obyekt fayl (as bilan yig'iladi: natijasi har safar bir xil)
        .intel_syntax noprefix
        .text

        .globl  qosh                    # GLOBAL funksiya: boshqa fayllar ko'radi
        .type   qosh, @function
qosh:
        lea     eax, [rdi + rsi]
        ret
        .size   qosh, .-qosh

        .globl  kopaytir
        .type   kopaytir, @function
kopaytir:
        mov     eax, edi
        imul    eax, esi
        ret
        .size   kopaytir, .-kopaytir

        .type   yordamchi, @function    # LOCAL funksiya (.globl yo'q): faqat shu faylda ko'rinadi
yordamchi:
        xor     eax, eax
        ret
        .size   yordamchi, .-yordamchi

        .globl  tashqi_chaqir
        .type   tashqi_chaqir, @function
tashqi_chaqir:
        sub     rsp, 8
        call    puts@PLT                # puts shu faylda YO'Q: UNDEFINED belgi (linker keyin topadi)
        add     rsp, 8
        ret
        .size   tashqi_chaqir, .-tashqi_chaqir

        .data
        .globl  hisoblagich
        .type   hisoblagich, @object
        .size   hisoblagich, 4
hisoblagich:
        .long   7

        .bss
        .lcomm  bufer, 64               # BSS dagi LOCAL o'zgaruvchi (nollar bilan boshlanadi)

        .section .note.GNU-stack, "", @progbits
