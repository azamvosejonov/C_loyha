/*
 * plic.c — tashqi uzilishlar kontrolleri (PLIC).
 *
 * ISHLASH TARTIBI (yadro tomondan qaralganda):
 *   1) Boshida: har kerakli manbaga ustuvorlik (masalan UART=1), o'z kontekstida yoqish, chegara = 0.
 *   2) Qurilma signal beradi -> PLIC manbani "kutmoqda" deb belgilaydi -> protsessorga SEIP (yoki MEIP).
 *   3) Yadroning trap ishlovchisi CLAIM registrini o'qiydi: eng ustuvor kutayotgan manba raqami qaytadi
 *      (masalan 10 — UART) va u "ishlanmoqda" holatiga o'tadi (shu paytda qayta uzilish bermaydi).
 *   4) Yadro qurilmaga xizmat ko'rsatadi (UART dan baytlarni o'qiydi).
 *   5) Yadro shu raqamni COMPLETE ga yozadi. Qurilma signal berishda davom etsa — yana kutmoqda bo'ladi.
 *
 * "Daraja bo'yicha" (level-triggered) uzilish: qurilma signali yoniq turar ekan, uzilish takrorlanadi.
 * Shuning uchun drayver sababni YO'Q QILISHI kerak (UART buferini bo'shatish), aks holda abadiy uzilish.
 */
#include "qurilmalar.h"

#define PLIC_KUTMOQDA 0x1000u
#define PLIC_YOQILGAN 0x2000u
#define PLIC_KONTEKST 0x200000u

void plic_signal(struct plic *p, int manba, int daraja)
{
    uint32_t bit = 1u << manba;
    if (daraja && !(p->ishlanmoqda & bit))
        p->kutmoqda |= bit;                     /* ishlanayotgan manba qayta belgilanmaydi (complete kutiladi) */
    else if (!daraja)
        p->kutmoqda &= ~bit;                    /* signal o'chdi: kutish ham bekor */
}

/* kontekst uchun eng ustuvor, yoqilgan, chegaradan yuqori kutayotgan manba (0 — yo'q) */
static uint32_t eng_ustuvor(const struct plic *p, int k)
{
    /*
     * TODO(P1) — O'ZINGIZ YOZING: PLIC: kontekst k uchun eng ustuvor kutayotgan manba raqamini toping (yo'q bo'lsa 0).
     *   - nomzod: kutmoqda VA yoqilgan[k] da biti bor, ustuvorlik[i] > chegara[k]
     *   - 1 dan PLIC_MANBALAR-1 gacha (0 — 'hech kim'); teng ustuvorlikda KICHIK raqam yutadi (> bilan solishtiring)
     * Tekshirish: make test  (birlik testida 'P1' qatori)
     */
    (void)p;
    (void)k;
    return 0;
}

int plic_kutyapti(const struct plic *p, int kontekst)
{
    return eng_ustuvor(p, kontekst) != 0;
}

uint32_t plic_oqi(struct plic *p, uint32_t s)
{
    if (s < 4 * PLIC_MANBALAR)
        return p->ustuvorlik[s / 4];
    if (s == PLIC_KUTMOQDA)
        return p->kutmoqda;
    if (s >= PLIC_YOQILGAN && s < PLIC_YOQILGAN + 0x80 * PLIC_KONTEKSTLAR && (s & 0x7F) == 0)
        return p->yoqilgan[(s - PLIC_YOQILGAN) / 0x80];
    if (s >= PLIC_KONTEKST && s < PLIC_KONTEKST + 0x1000 * PLIC_KONTEKSTLAR) {
        int k = (int)((s - PLIC_KONTEKST) / 0x1000);
        uint32_t ichki = (s - PLIC_KONTEKST) % 0x1000;
        if (ichki == 0)
            return p->chegara[k];
        if (ichki == 4) {                       /* CLAIM: eng ustuvorini beramiz va "ishlanmoqda" qilamiz */
            uint32_t m = eng_ustuvor(p, k);
            if (m) {
                p->kutmoqda &= ~(1u << m);
                p->ishlanmoqda |= 1u << m;
            }
            return m;
        }
    }
    return 0;
}

void plic_yoz(struct plic *p, uint32_t s, uint32_t q)
{
    if (s < 4 * PLIC_MANBALAR) {
        p->ustuvorlik[s / 4] = q & 7u;
        return;
    }
    if (s >= PLIC_YOQILGAN && s < PLIC_YOQILGAN + 0x80 * PLIC_KONTEKSTLAR && (s & 0x7F) == 0) {
        p->yoqilgan[(s - PLIC_YOQILGAN) / 0x80] = q & ~1u;     /* 0-manba yo'q */
        return;
    }
    if (s >= PLIC_KONTEKST && s < PLIC_KONTEKST + 0x1000 * PLIC_KONTEKSTLAR) {
        int k = (int)((s - PLIC_KONTEKST) / 0x1000);
        uint32_t ichki = (s - PLIC_KONTEKST) % 0x1000;
        if (ichki == 0)
            p->chegara[k] = q & 7u;
        else if (ichki == 4 && q > 0 && q < PLIC_MANBALAR)
            p->ishlanmoqda &= ~(1u << q);       /* COMPLETE: manba yana uzilish bera oladi */
    }
}
