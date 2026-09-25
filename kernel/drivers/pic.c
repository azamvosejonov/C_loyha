/* =============================================================================
 *  drivers/pic.c - 8259A PIC (uzilishlar kontrolleri)
 * =============================================================================
 *
 *  NIMA BU:
 *    Qurilmalar (taymer, klaviatura, serial port...) CPU'ga to'g'ridan-to'g'ri
 *    ulanmagan. Ular PIC chipiga ulangan, PIC esa CPU'ning bitta INTR oyog'iga.
 *    Kompyuterda ikkita PIC bor: master (IRQ 0-7) va slave (IRQ 8-15), slave
 *    master'ning IRQ2 oyog'iga ulangan ("kaskad").
 *
 *      IRQ0 - taymer (PIT)      IRQ1 - klaviatura     IRQ2 - slave PIC
 *      IRQ4 - COM1 serial       IRQ8 - RTC soat       IRQ14 - disk ...
 *
 *  NEGA QAYTA XARITALASH (REMAP) KERAK:
 *    BIOS master PIC'ni 8..15 vektorlariga sozlab qo'ygan. Lekin 8..15 - CPU
 *    exception'lari (8 = double fault!). Taymer har tikda "double fault" bo'lib
 *    ko'rinardi. Shuning uchun IRQ'larni 32..47 ga ko'chiramiz.
 *
 *  EOI (End Of Interrupt):
 *    Uzilishni qayta ishlab bo'lgach, PIC'ga "tayyor" deyishimiz SHART. Aks holda
 *    u shu va undan past ustuvorlikdagi uzilishlarni boshqa YUBORMAYDI.
 *
 *  ZAMONAVIY TIZIMLAR: haqiqiy kompyuterlarda PIC o'rniga APIC/IOAPIC ishlatiladi
 *  (ko'p yadroli protsessorlar uchun). PIC sodda va QEMU'da mukammal ishlaydi -
 *  o'rganish uchun ideal. APIC ga o'tish - docs/mashqlar.md dagi mashq.
 * ============================================================================= */
#include "drivers/pic.h"

#include "arch/io.h"

#define PIC1_COMMAND 0x20               /* master buyruq porti */
#define PIC1_DATA    0x21               /* master ma'lumot (niqob) porti */
#define PIC2_COMMAND 0xA0               /* slave buyruq porti */
#define PIC2_DATA    0xA1

#define PIC_EOI      0x20               /* "uzilish tugadi" buyrug'i */
#define ICW1_INIT    0x10               /* ishga tushirish buyrug'i */
#define ICW1_ICW4    0x01               /* ICW4 ham yuboriladi */
#define ICW4_8086    0x01               /* 8086 rejimi */
#define PIC_READ_ISR 0x0B               /* "hozir qaysi uzilishga xizmat qilinyapti" */

void pic_init(void)
{
    /* ICW = Initialization Command Word. PIC 4 ta ketma-ket so'z kutadi. */
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4); io_wait();   /* ICW1: ishga tushirishni boshlash */
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4); io_wait();
    outb(PIC1_DATA, PIC_IRQ_BASE);             io_wait();   /* ICW2: master vektor bazasi = 32 */
    outb(PIC2_DATA, PIC_IRQ_BASE + 8);         io_wait();   /* ICW2: slave vektor bazasi = 40 */
    outb(PIC1_DATA, 1 << 2);                   io_wait();   /* ICW3: master'ga "slave IRQ2 da" */
    outb(PIC2_DATA, 2);                        io_wait();   /* ICW3: slave'ga "sening raqaming 2" */
    outb(PIC1_DATA, ICW4_8086);                io_wait();   /* ICW4: 8086 rejimi */
    outb(PIC2_DATA, ICW4_8086);                io_wait();

    /* Hamma IRQ'larni niqoblaymiz (o'chiramiz). Kerakli drayver o'zi yoqadi.
     * Faqat IRQ2 (kaskad) ochiq - aks holda slave'dan hech narsa kelmaydi. */
    outb(PIC1_DATA, 0xFF & ~(1 << 2));
    outb(PIC2_DATA, 0xFF);
}

void pic_send_eoi(uint8_t irq)
{
    if (irq >= 8)                       /* slave'dan kelgan bo'lsa - ikkalasiga ham EOI */
        outb(PIC2_COMMAND, PIC_EOI);
    outb(PIC1_COMMAND, PIC_EOI);
}

/* Niqob registrida bit = 1 -> IRQ o'chirilgan. */
void pic_unmask(uint8_t irq)
{
    uint16_t port = irq < 8 ? PIC1_DATA : PIC2_DATA;
    uint8_t bit = irq & 7;
    outb(port, inb(port) & ~(1 << bit));
}

void pic_mask(uint8_t irq)
{
    uint16_t port = irq < 8 ? PIC1_DATA : PIC2_DATA;
    uint8_t bit = irq & 7;
    outb(port, inb(port) | (1 << bit));
}

/* "Soxta" uzilish: elektr shovqini tufayli PIC IRQ7 (yoki IRQ15) yuborishi
 * mumkin, lekin ISR registrida tegishli bit o'rnatilmagan bo'ladi. Bunday
 * uzilishga EOI YUBORMASLIK kerak (IRQ15 bo'lsa, master'ga yuborish kerak). */
bool pic_is_spurious(uint8_t irq)
{
    if (irq != 7 && irq != 15)
        return false;
    uint16_t cmd = irq == 7 ? PIC1_COMMAND : PIC2_COMMAND;
    outb(cmd, PIC_READ_ISR);
    uint8_t isr = inb(cmd);
    if (isr & (1 << 7))                 /* bit o'rnatilgan - haqiqiy uzilish */
        return false;
    if (irq == 15)
        outb(PIC1_COMMAND, PIC_EOI);    /* master buni haqiqiy deb o'yladi */
    return true;
}
