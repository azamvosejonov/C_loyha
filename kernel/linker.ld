/* =============================================================================
 *  linker.ld - yadro xotirada QAYERDA va QANDAY tartibda joylashishini belgilaydi
 * =============================================================================
 *
 *  NIMA UCHUN KERAK:
 *    Oddiy dasturda bu ishni Linux va standart linker skripti qiladi. Bizda OS
 *    yo'q - yadro O'ZI OS. Shuning uchun linkerga aniq aytishimiz kerak:
 *      - kod qaysi manzildan boshlanadi (1 MB)
 *      - bo'limlar (.text, .data, .bss ...) qaysi tartibda keladi
 *      - yadro qayerda tugaydi (kernel_end) - fizik xotira menejeri (pmm.c)
 *        shu manzilgacha bo'lgan xotirani "band" deb belgilaydi.
 *
 *  NEGA 1 MB (0x100000):
 *    Birinchi 1 MB da BIOS ma'lumotlari, VGA xotirasi (0xB8000), ROM'lar bor.
 *    Multiboot yuklovchilari ham yadroni odatda 1 MB ga qo'yadi.
 * ============================================================================= */

/* Yadroning kirish nuqtasi - boot.asm dagi _start belgisi. */
ENTRY(_start)

SECTIONS
{
    /* "." - joriy manzil hisoblagichi. Yadro 1 MB dan boshlanadi. */
    . = 1M;

    /* C kodidan &kernel_start orqali yadro boshini bilib olamiz. */
    kernel_start = .;

    /* Multiboot sarlavhasi fayl boshidan 8 KB ichida bo'lishi SHART, aks holda
     * yuklovchi uni topa olmaydi. Shuning uchun uni eng birinchi qo'yamiz.
     * KEEP() - linker "ishlatilmayapti" deb o'chirib yubormasligi uchun. */
    .multiboot : ALIGN(8)
    {
        KEEP(*(.multiboot))
    }

    /* .text - bajariladigan kod. ALIGN(4K) - har bir bo'lim yangi sahifadan
     * boshlanadi, keyinchalik sahifalarga turli ruxsat (faqat o'qish va h.k.)
     * berish mumkin bo'lishi uchun. */
    .text : ALIGN(4K)
    {
        *(.text .text.*)
    }

    /* .rodata - o'zgarmas ma'lumotlar: satr literallari, const massivlar. */
    .rodata : ALIGN(4K)
    {
        *(.rodata .rodata.*)
    }

    /* .data - boshlang'ich qiymatli global o'zgaruvchilar (int x = 5;). */
    .data : ALIGN(4K)
    {
        *(.data .data.*)
    }

    /* .bss - nol bilan boshlanadigan global o'zgaruvchilar (int y;).
     * Faylda joy egallamaydi - yuklovchi bu joyni nollar bilan to'ldiradi.
     * COMMON - eski uslubdagi "umumiy" o'zgaruvchilar. */
    .bss : ALIGN(4K)
    {
        *(COMMON)
        *(.bss .bss.*)
    }

    /* Yadro oxiri, sahifa chegarasiga tekislangan. Bundan keyingi xotira bo'sh. */
    . = ALIGN(4K);
    kernel_end = .;

    /* Keraksiz bo'limlarni tashlab yuboramiz:
     *   .comment  - kompilyator versiyasi haqida matn
     *   .eh_frame - C++ exception'lar uchun jadval
     *   .note*    - GNU qo'shimcha eslatmalari (ular alohida segment yaratib,
     *               multiboot sarlavhasini fayl boshidan uzoqlashtirishi mumkin) */
    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
        *(.note .note.*)
    }
}
