/* =============================================================================
 *  mm/vmalloc.c - VMALLOC: sahifama-sahifa xaritalanadigan yadro xotirasi
 * =============================================================================
 *
 *  kmalloc bilan farqi:
 *    kmalloc - direct map'dagi FIZIK jihatdan uzluksiz xotira. Katta blok
 *              (masalan, 1 MB) kerak bo'lsa, buddy'da shuncha ketma-ket bo'sh
 *              sahifa topilmasligi mumkin (fragmentatsiya).
 *    vmalloc - VIRTUAL jihatdan uzluksiz, fizik sahifalar esa tarqoq bo'lishi
 *              mumkin. Katta buferlar va - eng muhimi - YADRO STEKLARI uchun.
 *
 *  HIMOYA SAHIFALARI: har bir hudud atrofida xaritalanmagan sahifa qoldiramiz.
 *    [guard][ ... hudud ... ][guard][guard][ ... keyingi hudud ... ][guard]
 *  Yadro steki to'lsa (cheksiz rekursiya), u pastdagi himoya sahifasiga urilib
 *  darhol page fault beradi - boshqa jarayonning stekini jimgina buzmaydi.
 *  (Linux 4.9 dan boshlab CONFIG_VMAP_STACK aynan shu.)
 *
 *  ioremap: qurilma registrlari (APIC, AHCI, framebuffer) RAM emas - ular
 *  direct map'da yo'q. Ularni shu hududga TO'G'RI KESH TURI bilan xaritalaymiz:
 *    UC - har bir o'qish/yozish to'g'ridan-to'g'ri qurilmaga (registrlar uchun shart)
 *    WC - yozuvlar to'planib yuboriladi (framebuffer uchun tez)
 *
 *  Virtual manzillar oddiy "first-fit" bilan ajratiladi: hududlar manzil
 *  bo'yicha tartiblangan ro'yxatda.
 * ============================================================================= */
#include "mm/vmalloc.h"

#include <stdbool.h>

#include "lib/common.h"
#include "lib/kprintf.h"
#include "lib/list.h"
#include "lib/panic.h"
#include "lib/spinlock.h"
#include "mm/layout.h"
#include "mm/page.h"
#include "mm/pmm.h"
#include "mm/slab.h"
#include "mm/vmm.h"

#define VM_IOREMAP (1u << 0)            /* sahifalar bizniki emas - qaytarilmaydi */

struct vm_area {
    uint64_t start;                     /* birinchi xaritalangan sahifa */
    uint64_t size;                      /* baytda (himoya sahifalarisiz) */
    unsigned flags;
    struct list_head node;
};

static LIST_HEAD(areas);
static spinlock_t vm_lock = SPINLOCK_INIT("vmalloc");
static uint64_t vmalloc_pages;

/* [start - guard, start + size + guard) uchun bo'sh joy topish. */
static struct vm_area *alloc_area(uint64_t size, unsigned flags)
{
    struct vm_area *a = kmalloc(sizeof(*a));
    if (!a)
        return NULL;
    size = ALIGN_UP(size, PAGE_SIZE);
    uint64_t need = size + 2 * PAGE_SIZE;       /* oldida va ortida himoya */

    uint64_t f = spin_lock_irqsave(&vm_lock);
    uint64_t cursor = VMALLOC_START;
    struct list_head *insert_before = &areas;
    struct vm_area *it;
    list_for_each_entry(it, &areas, node) {
        uint64_t it_begin = it->start - PAGE_SIZE;
        if (it_begin - cursor >= need) {
            insert_before = &it->node;
            break;
        }
        cursor = it->start + it->size + PAGE_SIZE;
    }
    if (cursor + need > VMALLOC_END) {
        spin_unlock_irqrestore(&vm_lock, f);
        kfree(a);
        return NULL;
    }
    a->start = cursor + PAGE_SIZE;
    a->size = size;
    a->flags = flags;
    list_add_tail(&a->node, insert_before);     /* insert_before dan oldin */
    spin_unlock_irqrestore(&vm_lock, f);
    return a;
}

static struct vm_area *find_area(uint64_t start)
{
    struct vm_area *it;
    list_for_each_entry(it, &areas, node)
        if (it->start == start)
            return it;
    return NULL;
}

static void unmap_area(struct vm_area *a)
{
    uint64_t pml4 = vmm_kernel_pml4();
    for (uint64_t off = 0; off < a->size; off += PAGE_SIZE) {
        uint64_t phys = vmm_unmap_page(pml4, a->start + off);
        if (phys && !(a->flags & VM_IOREMAP)) {
            pmm_free_page(phys);
            __atomic_sub_fetch(&vmalloc_pages, 1, __ATOMIC_RELAXED);
        }
    }
}

void *vmalloc(size_t size)
{
    if (size == 0)
        return NULL;
    struct vm_area *a = alloc_area(size, 0);
    if (!a)
        return NULL;
    uint64_t pml4 = vmm_kernel_pml4();
    for (uint64_t off = 0; off < a->size; off += PAGE_SIZE) {
        uint64_t phys = pmm_alloc_page(GFP_ZERO);
        if (!phys || !vmm_map_page(pml4, a->start + off, phys,
                                   PTE_WRITABLE | pte_nx | PTE_GLOBAL)) {
            if (phys)
                pmm_free_page(phys);
            a->size = off;              /* shu paytgacha xaritalanganini qaytaramiz */
            unmap_area(a);
            uint64_t f = spin_lock_irqsave(&vm_lock);
            list_del(&a->node);
            spin_unlock_irqrestore(&vm_lock, f);
            kfree(a);
            return NULL;
        }
        __atomic_add_fetch(&vmalloc_pages, 1, __ATOMIC_RELAXED);
    }
    return (void *)a->start;
}

static void release(void *addr, bool io)
{
    if (!addr)
        return;
    uint64_t start = ALIGN_DOWN((uint64_t)addr, PAGE_SIZE);
    uint64_t f = spin_lock_irqsave(&vm_lock);
    struct vm_area *a = find_area(start);
    if (a)
        list_del(&a->node);
    spin_unlock_irqrestore(&vm_lock, f);
    if (!a)
        panic("%s: %p vmalloc hududi emas", io ? "iounmap" : "vfree", addr);
    if (!!(a->flags & VM_IOREMAP) != io)
        panic("%s: %p noto'g'ri funksiya bilan qaytarilyapti", io ? "iounmap" : "vfree", addr);
    unmap_area(a);
    kfree(a);
}

void vfree(void *addr)
{
    release(addr, false);
}

static void *ioremap_flags(uint64_t phys, size_t size, uint64_t cache)
{
    uint64_t offset = phys & (PAGE_SIZE - 1);
    uint64_t base = phys - offset;
    uint64_t len = ALIGN_UP(size + offset, PAGE_SIZE);
    struct vm_area *a = alloc_area(len, VM_IOREMAP);
    if (!a)
        return NULL;
    uint64_t pml4 = vmm_kernel_pml4();
    for (uint64_t off = 0; off < len; off += PAGE_SIZE)
        if (!vmm_map_page(pml4, a->start + off, base + off,
                          PTE_WRITABLE | pte_nx | PTE_GLOBAL | cache))
            panic("ioremap: sahifa jadvali uchun xotira yo'q");
    return (void *)(a->start + offset);
}

void *ioremap(uint64_t phys, size_t size)
{
    return ioremap_flags(phys, size, PTE_CACHE_UC);
}

void *ioremap_wc(uint64_t phys, size_t size)
{
    return ioremap_flags(phys, size, cpu_features.pat ? PTE_CACHE_WC : PTE_CACHE_UC);
}

void iounmap(void *addr)
{
    release(addr, true);
}

void vmalloc_init(void)
{
    kprintf("[vmalloc] hudud %p - %p, har bir ajratma atrofida himoya sahifalari\n",
            (void *)VMALLOC_START, (void *)VMALLOC_END);
}

void vmalloc_dump(void)
{
    uint64_t f = spin_lock_irqsave(&vm_lock);
    struct vm_area *it;
    list_for_each_entry(it, &areas, node)
        kprintf("  %p - %p  %6lu KB  %s\n", (void *)it->start, (void *)(it->start + it->size),
                it->size / 1024, (it->flags & VM_IOREMAP) ? "ioremap" : "vmalloc");
    spin_unlock_irqrestore(&vm_lock, f);
    kprintf("  vmalloc sahifalari: %lu\n", vmalloc_pages);
}
