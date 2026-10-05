/*
 * sbi.c — SBI (Supervisor Binary Interface): yadroning firmware'ga murojaatlari.
 *
 * Chaqirish qoidasi (SBI spetsifikatsiyasi v2.0):
 *   a7 = EID (kengaytma: masalan 0x54494D45 = "TIME"),  a6 = FID (shu kengaytmadagi funksiya),
 *   a0..a5 = argumentlar.  Javob: a0 = xato kodi (0 — muvaffaqiyat, manfiy — xato), a1 = qiymat.
 * Eski ("legacy", v0.1) chaqiruvlar: a7 = 0..8, natija faqat a0 da. Linux erta konsolida ular ishlatiladi.
 *
 * EID lar ko'pincha ASCII matn: 0x54494D45 = 'T' 'I' 'M' 'E'. Bitlar bilan "gaplashish"ning yana bir misoli.
 */
#include "fw.h"

#define SBI_OK 0
#define SBI_ERR_NOT_SUPPORTED (-2)
#define SBI_ERR_INVALID_PARAM (-3)

#define EID_LEGACY_SET_TIMER 0x00
#define EID_LEGACY_PUTCHAR 0x01
#define EID_LEGACY_GETCHAR 0x02
#define EID_LEGACY_SHUTDOWN 0x08
#define EID_BASE 0x10
#define EID_TIME 0x54494D45u                    /* "TIME" */
#define EID_IPI 0x735049u                       /* "sPI" */
#define EID_RFENCE 0x52464E43u                  /* "RFNC" */
#define EID_HSM 0x48534Du                       /* "HSM" */
#define EID_SRST 0x53525354u                    /* "SRST" */
#define EID_DBCN 0x4442434Eu                    /* "DBCN" — debug konsol */

#define MIP_STIP (1u << 5)
#define MIE_MTIE (1u << 7)
#define MIP_SSIP (1u << 1)

void quvvatni_och(uint32_t kod)
{
    mmio_yoz32(QUVVAT, kod == 0 ? 0x5555u : (kod << 16) | 0x3333u);
    for (;;)
        __asm__ volatile("wfi");
}

/*
 * TAYMER — eng muhim xizmat. Yadro "t vaqtda meni uyg'ot" deydi (set_timer). Lekin S rejimda mtimecmp ga
 * yozib bo'lmaydi (u M ning qurilmasi). Shuning uchun:
 *   set_timer(t):  mtimecmp <- t;  mip.STIP <- 0 (eski uzilish bekor);  mie.MTIE <- 1
 *   M taymer uzilishi kelganda (sbi_taymer_uzilishi): mip.STIP <- 1 (yadroga S taymer uzilishi "uzatiladi"),
 *                                                     mie.MTIE <- 0 (keyingi set_timer gacha takrorlanmasin)
 * Natijada yadro o'zini go'yo o'z taymeriga ega deb his qiladi.
 */
static void taymer_qoy(uint32_t past, uint32_t yuqori)
{
    /*
     * TODO(F1) — O'ZINGIZ YOZING: set_timer: mtimecmp <- (yuqori:past), eski S taymer uzilishini o'chiring, M taymer uzilishini yoqing.
     *   - 64 bitli qiymatni 32 bitli yozuvlar bilan: avval past <- 0xFFFFFFFF, keyin yuqori, keyin past (nega? izohni o'qing)
     *   - mmio_yoz32(CLINT_MTIMECMP, ...), CLINT_MTIMECMP + 4 — yuqori yarmi
     *   - csr_bit_och(mip, MIP_STIP);  csr_bit_yoq(mie, MIE_MTIE)
     * Tekshirish: make test  (birlik testida 'F1' qatori)
     */
    (void)past;
    (void)yuqori;
}

void sbi_taymer_uzilishi(void)
{
    /*
     * TODO(F2) — O'ZINGIZ YOZING: M taymer uzilishi keldi: uni yadroga S taymer uzilishi sifatida 'uzating'.
     *   - csr_bit_yoq(mip, MIP_STIP) — yadro STI ni ko'radi
     *   - csr_bit_och(mie, MIE_MTIE) — keyingi set_timer gacha M taymer jim
     * Tekshirish: make test  (birlik testida 'F2' qatori)
     */
}

/* BASE kengaytmasi: yadro birinchi bo'lib "qanday firmware? qaysi kengaytmalar bor?" deb so'raydi */
static int kengaytma_bormi(uint32_t eid)
{
    switch (eid) {
    case EID_LEGACY_SET_TIMER:
    case EID_LEGACY_PUTCHAR:
    case EID_LEGACY_GETCHAR:
    case EID_LEGACY_SHUTDOWN:
    case EID_BASE:
    case EID_TIME:
    case EID_IPI:
    case EID_RFENCE:
    case EID_HSM:
    case EID_SRST:
    case EID_DBCN:
        return 1;
    default:
        return 0;
    }
}

static void base(struct kadr *k, uint32_t fid)
{
    int32_t xato = SBI_OK;
    uint32_t q = 0;
    switch (fid) {
    case 0: q = 0x02000000u; break;             /* spetsifikatsiya versiyasi 2.0: (asosiy << 24) | kichik */
    case 1: q = 0x564B; break;                  /* firmware ID: "VK" */
    case 2: q = 1; break;                       /* firmware versiyasi */
    case 3: q = (uint32_t)kengaytma_bormi(k->x[A0]); break;
    case 4:                                     /* mvendorid */
    case 5:                                     /* marchid */
    case 6: q = 0; break;                       /* mimpid */
    default: xato = SBI_ERR_NOT_SUPPORTED;
    }
    k->x[A0] = (uint32_t)xato;
    k->x[A1] = q;
}

/* DBCN: yadro buferini konsolga yozish. a0 = uzunlik, a1/a2 = FIZIK manzil (past/yuqori). */
static void dbcn(struct kadr *k, uint32_t fid)
{
    if (fid == 0 || fid == 1) {                 /* 0 — write, 1 — read */
        uint32_t n = k->x[A0];
        volatile uint8_t *p = (volatile uint8_t *)k->x[A1];
        if (k->x[A2] != 0) {                    /* 4 GB dan yuqori manzil bizda yo'q */
            k->x[A0] = (uint32_t)SBI_ERR_INVALID_PARAM;
            return;
        }
        uint32_t i = 0;
        if (fid == 0) {
            for (; i < n; i++)
                konsol_belgi(p[i]);
        } else {
            int c;
            while (i < n && (c = konsol_oqi()) >= 0)
                p[i++] = (uint8_t)c;
        }
        k->x[A0] = SBI_OK;
        k->x[A1] = i;
    } else if (fid == 2) {                      /* write_byte */
        konsol_belgi((int)(k->x[A0] & 0xFF));
        k->x[A0] = SBI_OK;
        k->x[A1] = 0;
    } else {
        k->x[A0] = (uint32_t)SBI_ERR_NOT_SUPPORTED;
    }
}

void sbi_ecall(struct kadr *k)
{
    uint32_t eid = k->x[A7], fid = k->x[A6];
    k->mepc += 4;                               /* ecall dan KEYINGI buyruqqa qaytamiz (aks holda abadiy ecall) */

    switch (eid) {
    case EID_LEGACY_SET_TIMER:
        taymer_qoy(k->x[A0], k->x[A1]);         /* RV32: 64 bitli vaqt ikki registrda */
        k->x[A0] = 0;
        return;
    case EID_LEGACY_PUTCHAR:
        konsol_belgi((int)(k->x[A0] & 0xFF));
        k->x[A0] = 0;
        return;
    case EID_LEGACY_GETCHAR:
        k->x[A0] = (uint32_t)konsol_oqi();
        return;
    case EID_LEGACY_SHUTDOWN:
        quvvatni_och(0);
    case EID_BASE:
        base(k, fid);
        return;
    case EID_TIME:
        if (fid == 0) {
            taymer_qoy(k->x[A0], k->x[A1]);
            k->x[A0] = SBI_OK;
        } else {
            k->x[A0] = (uint32_t)SBI_ERR_NOT_SUPPORTED;
        }
        return;
    case EID_IPI:                               /* bitta yadro: o'ziga IPI = S dasturiy uzilish */
        if (fid == 0 && (k->x[A0] & 1u) && k->x[A1] == 0)
            csr_bit_yoq(mip, MIP_SSIP);
        k->x[A0] = SBI_OK;
        return;
    case EID_RFENCE:                            /* boshqa yadrolarga TLB tozalash: bitta yadroda — o'zimizda */
        __asm__ volatile("fence.i\n\tsfence.vma" ::: "memory");
        k->x[A0] = SBI_OK;
        return;
    case EID_HSM:
        if (fid == 2) {                         /* hart_get_status: 0 — ishlayapti */
            k->x[A0] = SBI_OK;
            k->x[A1] = 0;
        } else {
            k->x[A0] = (uint32_t)SBI_ERR_NOT_SUPPORTED;
        }
        return;
    case EID_SRST:                              /* system_reset: bizda — o'chirish */
        konsol_matn("\n[vk-sbi] yadro tizimni o'chirishni so'radi\n");
        quvvatni_och(k->x[A1] == 0 ? 0 : 1);
    case EID_DBCN:
        dbcn(k, fid);
        return;
    default:
        k->x[A0] = (uint32_t)SBI_ERR_NOT_SUPPORTED;
        return;
    }
}
