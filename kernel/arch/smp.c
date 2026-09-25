/* =============================================================================
 *  arch/smp.c - SMP: qo'shimcha CPU'larni ishga tushirish va ular bilan aloqa
 * =============================================================================
 *
 *  AP NI UYG'OTISH (Intel "MP Initialization Protocol"):
 *    1. trampolin kodini fizik 0x8000 ga nusxalaymiz, ma'lumot maydonlarini
 *       (CR3, stek, kirish nuqtasi) to'ldiramiz
 *    2. INIT IPI   - AP ni "reset" holatiga keltiradi
 *    3. 10 ms kutamiz
 *    4. SIPI (vektor 0x08) - AP 0x8000 dan real mode'da boshlaydi
 *    5. javob kelmasa - SIPI ni takrorlaymiz (spetsifikatsiya shunday deydi)
 *    6. AP o'zini "online" deb belgilaguncha kutamiz (100 ms gacha)
 *  Har bir AP ketma-ket uyg'otiladi (trampolin ma'lumoti umumiy).
 *
 *  TLB SHOOTDOWN:
 *    Har bir CPU'ning o'z TLB keshi bor. Yadro sahifasini (masalan, vfree)
 *    o'chirsak, BOSHQA CPU'larda eski tarjima qolishi mumkin -> ular bo'shatilgan
 *    (va boshqa maqsadga berilgan!) sahifaga yozib qo'yishi mumkin. Shuning
 *    uchun IPI yuborib, hamma CPU'ni invlpg qilishga majbur qilamiz.
 *
 *    Deadlock'siz dizayn: har bir CPU'da "pochta qutisi" (har bir yuboruvchi
 *    uchun alohida katak). Yuboruvchi javob kutayotganda O'Z pochta qutisini
 *    ham ko'rib turadi - ikki CPU bir-biriga bir vaqtda so'rov yuborsa ham
 *    osilib qolmaydi (hatto uzilishlar o'chiq bo'lsa ham).
 * ============================================================================= */
#include "arch/smp.h"

#include "acpi/acpi.h"
#include "arch/apic.h"
#include "arch/cpu.h"
#include "arch/gdt.h"
#include "arch/idt.h"
#include "arch/interrupts.h"
#include "arch/percpu.h"
#include "arch/tsc.h"
#include "lib/kprintf.h"
#include "lib/string.h"
#include "mm/pmm.h"
#include "mm/slab.h"
#include "mm/vmalloc.h"
#include "mm/vmm.h"
#include "proc/process.h"
#include "sys/syscall.h"

#define TRAMP_PHYS 0x8000
#define AP_STACK_SIZE (16 * 1024)

extern const uint8_t trampoline_start[], trampoline_end[];
extern uint64_t tramp_cr3, tramp_stack, tramp_entry, tramp_cpu;
extern uint32_t tramp_efer;

/* ---- TLB shootdown ---- */

struct tlb_req {
    uint64_t virt;
    uint64_t pages;
    volatile int pending;               /* hali javob bermagan CPU'lar */
};

/* inbox[qabul qiluvchi][yuboruvchi] */
static struct tlb_req *volatile inbox[MAX_CPUS][MAX_CPUS];

static void flush_local(uint64_t virt, uint64_t pages)
{
    for (uint64_t i = 0; i < pages; i++)
        cpu_invlpg(virt + i * PAGE_SIZE);
}

static void process_inbox(void)
{
    int me = cpu_id();
    for (int s = 0; s < ncpus; s++) {
        struct tlb_req *r = __atomic_exchange_n(&inbox[me][s], NULL, __ATOMIC_ACQ_REL);
        if (r) {
            flush_local(r->virt, r->pages);
            __atomic_sub_fetch(&r->pending, 1, __ATOMIC_RELEASE);
        }
    }
}

static void tlb_ipi(struct interrupt_frame *f)
{
    (void)f;
    process_inbox();
}

void tlb_shootdown(uint64_t virt, uint64_t pages)
{
    flush_local(virt, pages);
    if (ncpus <= 1)
        return;
    push_off();                         /* shu CPU'dan ko'chib ketmaylik */
    int me = cpu_id();
    struct tlb_req req = { .virt = virt, .pages = pages, .pending = 0 };
    for (int i = 0; i < ncpus; i++) {
        if (i == me || !cpus[i] || !cpus[i]->online)
            continue;
        __atomic_add_fetch(&req.pending, 1, __ATOMIC_RELAXED);
        __atomic_store_n(&inbox[i][me], &req, __ATOMIC_RELEASE);
        lapic_send_ipi(cpus[i]->apic_id, VECTOR_TLB_SHOOT);
    }
    while (__atomic_load_n(&req.pending, __ATOMIC_ACQUIRE) > 0) {
        process_inbox();                /* boshqalar ham bizdan so'rayotgan bo'lishi mumkin */
        cpu_pause();
    }
    pop_off();
}

/* ---- To'xtatish (panic) ---- */

static void stop_ipi(struct interrupt_frame *f)
{
    (void)f;
    for (;;)
        __asm__ volatile("cli; hlt");
}

void smp_stop_others(void)
{
    if (ncpus > 1 && apic_active)
        lapic_send_ipi_all_but_self(VECTOR_STOP);
}

/* ---- AP larni ishga tushirish ---- */

void ap_main(struct cpu *c);
void ap_main(struct cpu *c)
{
    percpu_install(c);                  /* gs:0 -> shu CPU */
    gdt_init_cpu(c);                    /* o'z GDT va TSS */
    idt_load();                         /* IDT umumiy */
    cpu_enable_features();              /* NX, PAT, PGE, SMEP */
    vmm_switch(vmm_kernel_pml4());      /* trampolin jadvalidan asosiy jadvalga */
    lapic_init_ap();
    syscall_init_cpu();
    proc_init_cpu(c);
    c->apic_id = lapic_id();
    __atomic_store_n(&c->online, true, __ATOMIC_RELEASE);   /* BSP kutyapti */
    lapic_timer_start();
    scheduler_loop();                   /* shu CPU ham jarayonlarni bajara boshlaydi */
}

/* AP uchun vaqtinchalik sahifa jadvali: yadro yuqori yarmi + 0..2 MB identity
 * (trampolin paging yoqilgan zahoti 0x8000 da ishlashda davom etishi uchun).
 * 32-bitli kod CR3 ga yozadi, shuning uchun jadval 4 GB dan past (DMA32). */
static uint64_t build_tramp_pml4(uint64_t *pages, int *npages)
{
    uint64_t pml4 = pmm_alloc_page(GFP_ZERO | GFP_DMA32);
    uint64_t pdpt = pmm_alloc_page(GFP_ZERO | GFP_DMA32);
    uint64_t pd = pmm_alloc_page(GFP_ZERO | GFP_DMA32);
    if (!pml4 || !pdpt || !pd)
        return 0;
    uint64_t *l4 = phys_to_virt(pml4);
    memcpy(l4 + 256, (uint64_t *)phys_to_virt(vmm_kernel_pml4()) + 256, 256 * 8);
    l4[0] = pdpt | PTE_PRESENT | PTE_WRITABLE;
    ((uint64_t *)phys_to_virt(pdpt))[0] = pd | PTE_PRESENT | PTE_WRITABLE;
    ((uint64_t *)phys_to_virt(pd))[0] = 0 | PTE_PRESENT | PTE_WRITABLE | PTE_HUGE;
    pages[0] = pml4, pages[1] = pdpt, pages[2] = pd;
    *npages = 3;
    return pml4;
}

static void ipi_init_handlers(void)
{
    interrupt_register(VECTOR_TLB_SHOOT, tlb_ipi);
    interrupt_register(VECTOR_STOP, stop_ipi);
}

void smp_init(void)
{
    ipi_init_handlers();
    if (!apic_active || acpi.cpu_count <= 1) {
        kprintf("[smp]  Bitta CPU\n");
        return;
    }

    /* Trampolinni 0x8000 ga (memblock birinchi 1 MB ni band qilgan - bo'sh). */
    size_t tsize = (size_t)(trampoline_end - trampoline_start);
    uint8_t *tramp = phys_to_virt(TRAMP_PHYS);
    memcpy(tramp, trampoline_start, tsize);
#define TRAMP_FIELD(sym) ((void *)(tramp + ((const uint8_t *)&(sym) - trampoline_start)))

    uint64_t pt_pages[3];
    int npt = 0;
    uint64_t cr3 = build_tramp_pml4(pt_pages, &npt);
    if (!cr3) {
        kprintf("[smp]  xotira yo'q - AP lar ishga tushirilmadi\n");
        return;
    }
    *(uint64_t *)TRAMP_FIELD(tramp_cr3) = cr3;
    *(uint64_t *)TRAMP_FIELD(tramp_entry) = (uint64_t)ap_main;
    *(uint32_t *)TRAMP_FIELD(tramp_efer) = (uint32_t)(rdmsr(MSR_EFER) & (EFER_NXE | (1u << 8)));

    uint32_t bsp_apic = this_cpu()->apic_id;
    int started = 0;
    for (int i = 0; i < acpi.cpu_count && ncpus < MAX_CPUS; i++) {
        uint32_t apic_id = acpi.cpus[i].apic_id;
        if (apic_id == bsp_apic)
            continue;
        struct cpu *c = percpu_alloc(ncpus, apic_id);
        uint8_t *stack = vmalloc(AP_STACK_SIZE);
        uint8_t *ist = kmalloc(8192);
        if (!c || !stack || !ist) {
            kprintf("[smp]  CPU (APIC %u) uchun xotira yo'q\n", apic_id);
            break;
        }
        c->ist_stack = ist;
        *(uint64_t *)TRAMP_FIELD(tramp_stack) = (uint64_t)stack + AP_STACK_SIZE;
        *(uint64_t *)TRAMP_FIELD(tramp_cpu) = (uint64_t)c;
        __atomic_thread_fence(__ATOMIC_SEQ_CST);

        lapic_send_init(apic_id);
        mdelay(10);
        for (int attempt = 0; attempt < 2 && !c->online; attempt++) {
            lapic_send_sipi(apic_id, TRAMP_PHYS >> 12);
            for (int t = 0; t < 1000 && !__atomic_load_n(&c->online, __ATOMIC_ACQUIRE); t++)
                udelay(100);            /* 100 ms gacha */
        }
        if (!c->online) {
            kprintf("[smp]  CPU APIC %u javob bermadi\n", apic_id);
            cpus[ncpus] = NULL;
            continue;
        }
        ncpus++;
        started++;
    }
    for (int i = 0; i < npt; i++)
        pmm_free_page(pt_pages[i]);     /* trampolin jadvali endi kerak emas */
    kprintf("[smp]  %d ta CPU ishlayapti (BSP + %d AP)\n", ncpus, started);
}
