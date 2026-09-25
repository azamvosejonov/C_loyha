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
