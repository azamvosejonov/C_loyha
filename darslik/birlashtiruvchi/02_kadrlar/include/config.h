/* config.h - butun loyiha uchun umumiy doimiylar (bir joyda: o'zgartirish oson) */
#ifndef CONFIG_H
#define CONFIG_H

#define MAKS_XODIM 32                       /* omborda eng ko'pi bilan nechta xodim */
#define ISM_UZ 24                           /* ism uchun joy (23 belgi + '\0') */

#define TANAFFUS_DAQ 60                     /* har ish kunidagi tushlik tanaffusi (daqiqa) */
#define NORMA_KUN_DAQ 480                   /* kunlik oddiy norma: 8 soat. Undan ortig'i - qo'shimcha (ustama) */

/* progressiv soliq (tiyinda: 1 so'm = 100 tiyin) */
#define SOLIQ_CHEGARA1 300000000LL          /* 3 000 000 so'm */
#define SOLIQ_CHEGARA2 800000000LL          /* 8 000 000 so'm */
#define SOLIQ_FOIZ1 12                      /* 1-chegaragacha */
#define SOLIQ_FOIZ2 15                      /* 1- va 2-chegara orasidagi qism */
#define SOLIQ_FOIZ3 20                      /* 2-chegaradan oshgan qism */

#define KASABA_FOIZ 1

#endif
