/* config.h - hamma modul uchun umumiy doimiylar */
#ifndef CONFIG_H
#define CONFIG_H

#define ISM_UZ 24                               /* ism uchun joy: 23 belgi + '\0' */
#define TANAFFUS_DAQ 60
#define NORMA_KUN_DAQ 480                       /* kunlik oddiy norma (8 soat); ortig'i - qo'shimcha */
#define KASABA_FOIZ 1

/* KIRISH CHEGARALARI: tashqaridan kelgan ma'lumotga ISHONMAYMIZ. Hamma chegara shu yerda, parser va hisob ular bilan mos */
#define QATOR_UZ 256                            /* fayl qatorining eng uzun hajmi */
#define TARIF_MAKS 100000000000LL               /* eng yuqori tarif: 1 mlrd so'm/soat (tiyinda) */
#define OY_MAKS_DAQ (31 * 24 * 60)              /* bir oyda bo'lishi mumkin bo'lgan eng ko'p daqiqa */

#define SOLIQ_CHEGARA1 300000000LL              /* tiyinda: 3 000 000 so'm */
#define SOLIQ_CHEGARA2 800000000LL
#define SOLIQ_FOIZ1 12
#define SOLIQ_FOIZ2 15
#define SOLIQ_FOIZ3 20

#endif
