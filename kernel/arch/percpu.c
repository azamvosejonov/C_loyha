/* =============================================================================
 *  arch/percpu.c - per-CPU ma'lumotlar
 * ============================================================================= */
#include "arch/percpu.h"

#include "arch/cpu.h"
#include "lib/panic.h"
#include "lib/string.h"
#include "mm/slab.h"

struct cpu *cpus[MAX_CPUS];
int ncpus = 1;

/* BSP nikini statik qilamiz: u kmalloc paydo bo'lishidan OLDIN kerak
 * (spinlock'lar push_off ni, push_off esa this_cpu() ni ishlatadi). */
static struct cpu bsp_cpu;

void percpu_install(struct cpu *c)
{
    c->self = c;
    wrmsr(MSR_GS_BASE, (uint64_t)c);
    wrmsr(MSR_KERNEL_GS_BASE, 0);       /* user GS bazasi (swapgs dan keyin) */
}

void percpu_init_bsp(void)
{
    memset(&bsp_cpu, 0, sizeof(bsp_cpu));
    bsp_cpu.id = 0;
    bsp_cpu.online = true;
    cpus[0] = &bsp_cpu;
    percpu_install(&bsp_cpu);
}

struct cpu *percpu_alloc(int id, uint32_t apic_id)
{
    struct cpu *c = kzalloc(sizeof(*c));
    if (!c)
        return NULL;
    c->id = id;
    c->apic_id = apic_id;
    cpus[id] = c;
    return c;
}

void push_off(void)
{
    bool was_enabled = cpu_interrupts_enabled();
    cpu_cli();
    struct cpu *c = this_cpu();
    if (c->ncli == 0)
        c->intena = was_enabled;
    c->ncli++;
}

void pop_off(void)
{
    struct cpu *c = this_cpu();
    if (cpu_interrupts_enabled())
        panic("pop_off: uzilishlar yoqilgan holda chaqirildi");
    if (c->ncli < 1)
        panic("pop_off: push_off siz");
    c->ncli--;
    if (c->ncli == 0 && c->intena)
        cpu_sti();
}
