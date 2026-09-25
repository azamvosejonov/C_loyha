/* =============================================================================
 *  arch/cpu.h - protsessorning maxsus instruksiyalari uchun kichik yordamchilar
 * =============================================================================
 *
 *  C tilida "uzilishlarni o'chir" yoki "CR3 registrini o'qi" degan operator yo'q.
 *  Shuning uchun ularni inline assembly bilan o'raymiz va oddiy C funksiyasidek
 *  ishlatamiz.
 * ============================================================================= */
#pragma once

#include <stdbool.h>
#include <stdint.h>

/* RFLAGS registridagi IF (Interrupt Flag) biti - 9-bit.
 * IF = 1 bo'lsa CPU tashqi uzilishlarni qabul qiladi. */
#define RFLAGS_IF (1ULL << 9)

/* Uzilishlarni o'chirish (Clear Interrupt flag). */
static inline void cpu_cli(void)
{
    __asm__ volatile("cli" ::: "memory");
}

/* Uzilishlarni yoqish (Set Interrupt flag). */
static inline void cpu_sti(void)
{
    __asm__ volatile("sti" ::: "memory");
}

/* CPU'ni keyingi uzilishgacha "uxlatish". Bo'sh turganda protsessorni
 * 100% yuklamaslik uchun ishlatiladi (energiya tejaydi). */
static inline void cpu_hlt(void)
{
    __asm__ volatile("hlt" ::: "memory");
}

/* RFLAGS registrini o'qish. pushfq - flaglarni stekka, pop - registrga. */
static inline uint64_t cpu_read_rflags(void)
{
    uint64_t rflags;
    __asm__ volatile("pushfq; pop %0" : "=r"(rflags) : : "memory");
    return rflags;
}

/* Uzilishlar yoqilganmi? */
static inline bool cpu_interrupts_enabled(void)
{
    return (cpu_read_rflags() & RFLAGS_IF) != 0;
}

/* ---------------------------------------------------------------------------
 *  irq_save / irq_restore - "kritik bo'lim" (critical section) uchun juftlik.
 *
 *  MUAMMO: bitta protsessorli tizimda ikki kod bir vaqtda bitta ma'lumotni
 *  o'zgartirishining yagona yo'li - uzilish. Masalan, biz ro'yxatga element
 *  qo'shayotganda taymer uzilishi kelib, boshqa jarayonga o'tsa va u ham shu
 *  ro'yxatni o'zgartirsa - ro'yxat buziladi.
 *
 *  YECHIM: kritik bo'lim davomida uzilishlarni o'chiramiz. Lekin oddiy cli/sti
 *  yetarli emas: agar chaqiruvchi allaqachon uzilishlarni o'chirgan bo'lsa,
 *  bizning sti uni kutilmaganda yoqib yuboradi. Shuning uchun AVVALGI holatni
 *  saqlab, keyin aynan o'shani tiklaymiz:
 *
 *      uint64_t flags = irq_save();
 *      ... kritik bo'lim ...
 *      irq_restore(flags);
 * --------------------------------------------------------------------------- */
static inline uint64_t irq_save(void)
{
    uint64_t flags = cpu_read_rflags();
    cpu_cli();
    return flags;
}

static inline void irq_restore(uint64_t flags)
{
    if (flags & RFLAGS_IF)
        cpu_sti();
}

/* CR2 - sahifa xatosi (page fault) sodir bo'lgan virtual manzil. */
static inline uint64_t cpu_read_cr2(void)
{
    uint64_t value;
    __asm__ volatile("mov %%cr2, %0" : "=r"(value));
    return value;
}

/* CR3 - joriy PML4 jadvalining fizik manzili (ya'ni joriy "manzil maydoni"). */
static inline uint64_t cpu_read_cr3(void)
{
    uint64_t value;
    __asm__ volatile("mov %%cr3, %0" : "=r"(value));
    return value;
}

/* CR3 ga yozish = boshqa manzil maydoniga o'tish. Bu TLB keshini ham tozalaydi. */
static inline void cpu_write_cr3(uint64_t value)
{
    __asm__ volatile("mov %0, %%cr3" : : "r"(value) : "memory");
}

/* TLB (Translation Lookaside Buffer) - CPU'ning "virtual->fizik" tarjimalar keshi.
 * Sahifa jadvalini o'zgartirganimizda, eski tarjima keshda qolib ketishi mumkin.
 * invlpg bitta sahifa uchun keshdagi yozuvni o'chiradi. */
static inline void cpu_invlpg(uint64_t virt)
{
    __asm__ volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

/* Kompilyatorga "bu nuqtadan xotira operatsiyalarini u yoq-bu yoqqa ko'chirma"
 * deyish. CPU instruksiyasi emas, faqat kompilyator uchun to'siq. */
static inline void compiler_barrier(void)
{
    __asm__ volatile("" ::: "memory");
}

/* ---------------------------------------------------------------------------
 *  CPUID - protsessordan "nimalarni qila olasan?" deb so'rash.
 *  leaf (EAX) va subleaf (ECX) beriladi, javob EAX/EBX/ECX/EDX da.
 * --------------------------------------------------------------------------- */
static inline void cpuid(uint32_t leaf, uint32_t subleaf, uint32_t *a, uint32_t *b,
                         uint32_t *c, uint32_t *d)
{
    __asm__ volatile("cpuid" : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d)
                     : "a"(leaf), "c"(subleaf));
}

/* MSR (Model Specific Register) - CPU sozlamalari registrlari (EFER, PAT, APIC...). */
static inline uint64_t rdmsr(uint32_t msr)
{
    uint32_t lo, hi;
    __asm__ volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
    return ((uint64_t)hi << 32) | lo;
}

static inline void wrmsr(uint32_t msr, uint64_t value)
{
    __asm__ volatile("wrmsr" : : "c"(msr), "a"((uint32_t)value), "d"((uint32_t)(value >> 32))
                     : "memory");
}

#define MSR_EFER           0xC0000080
#define MSR_STAR           0xC0000081
#define MSR_LSTAR          0xC0000082
#define MSR_SFMASK         0xC0000084
#define MSR_FS_BASE        0xC0000100
#define MSR_GS_BASE        0xC0000101
#define MSR_KERNEL_GS_BASE 0xC0000102
#define MSR_PAT            0x277
#define MSR_APIC_BASE      0x1B

#define EFER_SCE (1UL << 0)             /* syscall/sysret yoqish */
#define EFER_NXE (1UL << 11)            /* NX bit yoqish */

static inline uint64_t rdtsc(void)
{
    uint32_t lo, hi;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

static inline void cpu_pause(void)
{
    __asm__ volatile("pause" ::: "memory");  /* spin-kutish tsiklida CPU'ga "dam ol" */
}

static inline uint64_t cpu_read_cr0(void)
{
    uint64_t v;
    __asm__ volatile("mov %%cr0, %0" : "=r"(v));
    return v;
}

static inline uint64_t cpu_read_cr4(void)
{
    uint64_t v;
    __asm__ volatile("mov %%cr4, %0" : "=r"(v));
    return v;
}

static inline void cpu_write_cr4(uint64_t v)
{
    __asm__ volatile("mov %0, %%cr4" : : "r"(v) : "memory");
}

/* Aniqlangan CPU imkoniyatlari (arch/cpu.c). */
struct cpu_features {
    char vendor[13];                    /* "GenuineIntel", "AuthenticAMD" ... */
    char brand[49];                     /* "Intel(R) Core(TM) i5-..." */
    bool nx;                            /* No-Execute bit */
    bool page1gb;                       /* 1 GB lik sahifalar */
    bool pat;                           /* Page Attribute Table (write-combining uchun) */
    bool pge;                           /* global sahifalar */
    bool apic;                          /* Local APIC */
    bool x2apic;
    bool tsc_invariant;                 /* TSC chastotasi o'zgarmas */
    bool smep, smap;                    /* user sahifalardan himoya */
    uint32_t max_leaf, max_ext_leaf;
};

extern struct cpu_features cpu_features;
void cpu_detect(void);
/* NX/PAT/SMEP kabi imkoniyatlarni har bir CPU'da yoqish. */
void cpu_enable_features(void);
