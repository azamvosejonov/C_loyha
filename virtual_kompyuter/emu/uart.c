/*
 * uart.c — konsol qurilmasi: 16550 UART ning kichik, lekin haqiqiy drayverlar bilan mos qismi.
 *
 * Haqiqiy UART — ketma-ket port: baytlarni sim orqali bitta-bitta yuboradi. Biz uni kompyuterimizning
 * terminaliga ulaymiz:
 *   - yadro THR ga bayt yozsa  -> u bizning stdout ga chiqadi;
 *   - biz klaviaturada bosgan (yoki stdin ga quvur orqali bergan) baytlar -> RBR dan o'qiladi.
 *
 * Drayver qanday ishlaydi (yadro tomonda): LSR ning 5-biti (THRE) = 1 bo'lguncha kutadi, keyin THR ga
 * yozadi. O'qish: LSR ning 0-biti (DR) = 1 bo'lsa — RBR da bayt bor. Bizning qurilma doim "yuborishga
 * tayyor" (THRE = 1), chunki stdout ga yozish darhol bo'ladi.
 *
 * KIRISH (stdin)
 *   - Terminal bo'lmasa (masalan `echo ls | ./vk ...` yoki test fayli): butun kirishni bir marta o'qib olamiz.
 *     Natija har safar bir xil — testlar uchun muhim.
 *   - Terminal bo'lsa: terminalni "xom" (raw) rejimga o'tkazamiz — har tugma darhol (Enter kutmasdan)
 *     va ekranga avtomatik chiqarilmasdan keladi (aks-sadoni yadro o'zi chiqaradi, haqiqiy kompyuterdagidek).
 */
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "qurilmalar.h"

static struct termios asl_holat;
static int xom_rejim;

int uart_interaktiv(void)
{
    return xom_rejim;
}

void uart_tugat(void)
{
    fflush(stdout);
    if (xom_rejim) {
        tcsetattr(STDIN_FILENO, TCSANOW, &asl_holat);      /* terminalni avvalgi holatiga qaytaramiz */
        xom_rejim = 0;
    }
}

void uart_tayyorla(struct uart *u)
{
    u->bosh = u->oxir = 0;
    u->stdin_tugadi = 0;
    if (isatty(STDIN_FILENO) && tcgetattr(STDIN_FILENO, &asl_holat) == 0) {
        struct termios t = asl_holat;
        t.c_lflag &= ~(tcflag_t)(ICANON | ECHO);           /* qatorlab emas, belgilab; aks-sadosiz */
        tcsetattr(STDIN_FILENO, TCSANOW, &t);
        xom_rejim = 1;
        atexit(uart_tugat);
    }
}

#define SUKUNAT 3000000ull                       /* "odam" keyingi qatorni yozishidan oldin shuncha buyruq sukunat */

/* fayl/quvur rejimi: butun kirishni bir marta o'qib, zaxiraga qo'yamiz */
static void zaxirani_toldir(struct uart *u)
{
    size_t sigim = 4096;
    u->zaxira = malloc(sigim);
    ssize_t n;
    while (u->zaxira && (n = read(STDIN_FILENO, u->zaxira + u->zaxira_uz, sigim - u->zaxira_uz)) > 0) {
        u->zaxira_uz += (size_t)n;
        if (u->zaxira_uz == sigim) {
            unsigned char *y = realloc(u->zaxira, sigim * 2);
            if (!y)
                break;
            u->zaxira = y;
            sigim *= 2;
        }
    }
    u->stdin_tugadi = 1;
}

/* bufer bo'sh bo'lsa, stdin dan yangi baytlarni olishga urinadi */
static void kirishni_yangila(struct uart *u)
{
    if (u->bosh != u->oxir)
        return;
    if (!xom_rejim && u->vaqt) {                /* "odamdek yozish" rejimi */
        if (!u->zaxira && !u->stdin_tugadi)
            zaxirani_toldir(u);
        if (u->zaxira_bosh >= u->zaxira_uz || *u->vaqt - u->oxirgi_chiqish < SUKUNAT)
            return;
        if (!(u->registr[1] & UART_IER_RX))
            return;                             /* drayver hali qabul qilishga tayyor emas (terminal ochilmagan) */
        size_t n = 0;                           /* keyingi QATORni (\n gacha, u ham kiradi) beramiz */
        while (u->zaxira_bosh < u->zaxira_uz && n < sizeof(u->kirish)) {
            unsigned char c = u->zaxira[u->zaxira_bosh++];
            u->kirish[n++] = c;
            if (c == '\n')
                break;
        }
        u->bosh = 0;
        u->oxir = n;
        u->oxirgi_chiqish = *u->vaqt;           /* keyingi qator ham sukunatdan keyin */
        return;
    }
    if (u->stdin_tugadi)
        return;
    if (xom_rejim) {
        static unsigned sanoq;
        if (++sanoq % 64)                       /* tizim chaqiruvi qimmat: har 64-murojaatda bir marta tekshiramiz */
            return;
        struct pollfd p = { STDIN_FILENO, POLLIN, 0 };
        if (poll(&p, 1, 0) <= 0)                /* 0 ms: kutmasdan tekshiramiz — emulyator to'xtab qolmasin */
            return;
    }
    ssize_t n = read(STDIN_FILENO, u->kirish, sizeof(u->kirish));
    if (n <= 0) {
        if (!xom_rejim)
            u->stdin_tugadi = 1;                /* fayl/quvur tugadi: boshqa bayt kelmaydi */
        return;
    }
    u->bosh = 0;
    u->oxir = (size_t)n;
}

uint32_t uart_oqi(struct uart *u, uint32_t siljish)
{
    switch (siljish) {
    case 0:                                     /* RBR: navbatdagi baytni beradi */
        kirishni_yangila(u);
        if (u->bosh == u->oxir)
            return 0;
        return u->kirish[u->bosh++];
    case 2: {                                   /* IIR: qaysi uzilish? Ustuvorlik: "bayt keldi" > "yuborishga tayyor" */
        kirishni_yangila(u);
        if ((u->registr[1] & UART_IER_RX) && u->bosh != u->oxir)
            return 0x04;
        if ((u->registr[1] & UART_IER_TX) && u->tx_uzilish) {
            u->tx_uzilish = 0;                  /* 16550 qoidasi: IIR o'qilgach "tayyor" uzilishi o'chadi */
            return 0x02;
        }
        return 0x01;                            /* uzilish yo'q */
    }
    case 5:                                     /* LSR */
        kirishni_yangila(u);
        return UART_LSR_THRE | UART_LSR_TEMT | (u->bosh != u->oxir ? UART_LSR_DR : 0);
    default:
        return (uint32_t)u->registr[siljish & 7];
    }
}

void uart_yoz(struct uart *u, uint32_t siljish, uint32_t qiymat)
{
    if (siljish == 0) {                         /* THR: baytni ekranga chiqaramiz */
        putchar((int)(qiymat & 0xFF));
        if (u->vaqt)
            u->oxirgi_chiqish = *u->vaqt;
        if ((qiymat & 0xFF) == '\n' || xom_rejim)
            fflush(stdout);
        u->tx_uzilish = 1;                      /* bayt "yuborildi": yana tayyormiz */
        return;
    }
    if (siljish == 1 && (qiymat & UART_IER_TX) && !(u->registr[1] & UART_IER_TX))
        u->tx_uzilish = 1;                      /* TX uzilishi endi yoqildi: darhol "tayyor" */
    u->registr[siljish & 7] = (int)(qiymat & 0xFF);
}

/* PLIC ga uzilish signali: yoqilgan sabablardan biri bormi */
int uart_uzilish(struct uart *u)
{
    if ((u->registr[1] & UART_IER_RX) && u->bosh != u->oxir)
        return 1;
    if ((u->registr[1] & UART_IER_TX) && u->tx_uzilish)
        return 1;
    if (u->registr[1] & UART_IER_RX) {
        kirishni_yangila(u);                    /* yangi bayt kelganmi — tekshiramiz */
        return u->bosh != u->oxir;
    }
    return 0;
}
