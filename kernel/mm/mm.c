/* =============================================================================
 *  mm/mm.c - JARAYON XOTIRASI: VMA, DEMAND PAGING, COPY-ON-WRITE
 * =============================================================================
 *
 *  1) VMA - "bu manzillar hududida nima bo'lishi KERAK" degan tavsif:
 *
 *       0x400000  ┌─────────────┐ VMA_ELF  r-x  (kod)
 *                 ├─────────────┤ VMA_ELF  rw-  (ma'lumotlar)
 *                 ├─────────────┤ VMA_HEAP rw-  (brk - malloc shu yerdan)
 *                 │     ...     │
 *       mmap      ├─────────────┤ VMA_ANON rw-  (mmap(NULL, ...))
 *                 │     ...     │
 *       stek      ├─────────────┤ VMA_STACK rw- (pastga o'sadi, 8 MB gacha)
 *
 *     Sahifa jadvali esa "hozir haqiqatan nima bor"ni ko'rsatadi. Ikkalasi
 *     farq qilishi mumkin - va bu ataylab!
 *
 *  2) DEMAND PAGING (talab bo'yicha sahifalash): malloc(100 MB) darhol 100 MB
 *     RAM ni EGALLAMAYDI - faqat VMA yaratiladi. Dastur sahifaga birinchi
 *     marta tegganda page fault bo'ladi, handler VMA ni topadi va FAQAT O'SHA
 *     sahifani ajratadi. Ishlatilmagan xotira hech qachon ajratilmaydi.
 *
 *  3) COPY-ON-WRITE (fork): fork() 100 MB li jarayonni nusxalashi kerak. Barcha
 *     sahifalarni nusxalash sekin va isrofgarchilik (bola odatda darhol exec
 *     qiladi). Yechim: sahifalarni UMUMIY qoldiramiz, lekin ikkala tomonda
 *     "faqat o'qish" + COW belgisi qo'yamiz. Kimdir YOZSA - page fault, handler
 *     O'SHA sahifaning nusxasini yaratadi. struct page->refcount nechta jarayon
 *     sahifani ulashayotganini sanaydi; oxirgisi nusxalamasdan o'zlashtiradi.
 *
 *  4) STEK O'SISHI: stek VMA sidan PASTDAGI manzilga murojaat (8 MB chegarasida)
 *     - VMA avtomatik kengayadi. Chegaradan pastda - Segmentation fault.
 * ============================================================================= */
#include "mm/mm.h"

#include "arch/cpu.h"
#include "lib/common.h"
#include "lib/kprintf.h"
#include "lib/panic.h"
#include "lib/string.h"
#include "mm/layout.h"
#include "mm/page.h"
#include "mm/pmm.h"
#include "mm/slab.h"
#include "mm/vmm.h"

static struct kmem_cache *vma_cache;
static struct kmem_cache *mm_cache;

uint64_t prot_to_pte(uint32_t prot)
{
    uint64_t f = PTE_USER;
    if (prot & PROT_WRITE)
        f |= PTE_WRITABLE;
    if (!(prot & PROT_EXEC))
        f |= pte_nx;                    /* bajarib bo'lmaydi (W^X user dasturlar uchun ham) */
    return f;
}

static void caches_init(void)
{
    if (!vma_cache) {
        vma_cache = kmem_cache_create("vma", sizeof(struct vma), 8, NULL);
        mm_cache = kmem_cache_create("mm", sizeof(struct mm), 8, NULL);
    }
}

struct mm *mm_create(void)
{
    caches_init();
    struct mm *mm = kmem_cache_alloc(mm_cache);
    if (!mm)
        return NULL;
    memset(mm, 0, sizeof(*mm));
    mm->pml4 = vmm_create_address_space();
    if (!mm->pml4) {
        kmem_cache_free(mm_cache, mm);
        return NULL;
    }
    list_init(&mm->vmas);
    mm->lock = (spinlock_t)SPINLOCK_INIT("mm");
    return mm;
}

void mm_destroy(struct mm *mm)
{
    struct vma *v, *tmp;
    list_for_each_entry_safe(v, tmp, &mm->vmas, node) {
        list_del(&v->node);
        kmem_cache_free(vma_cache, v);
    }
    vmm_destroy_address_space(mm->pml4);        /* sahifalar put_page orqali (COW xavfsiz) */
    kmem_cache_free(mm_cache, mm);
}

/* ---- VMA ro'yxati bilan ishlash (mm->lock ushlangan) ---- */

static struct vma *find_vma(struct mm *mm, uint64_t addr)
{
    struct vma *v;
    list_for_each_entry(v, &mm->vmas, node)
        if (addr >= v->start && addr < v->end)
            return v;
    return NULL;
}

static bool range_free(struct mm *mm, uint64_t start, uint64_t end)
{
    struct vma *v;
    list_for_each_entry(v, &mm->vmas, node)
        if (start < v->end && end > v->start)
            return false;
    return true;
}

static void insert_vma(struct mm *mm, struct vma *nv)
{
    struct vma *v;
    list_for_each_entry(v, &mm->vmas, node) {
        if (nv->start < v->start) {
            list_add_tail(&nv->node, &v->node);     /* v dan OLDIN */
            return;
        }
    }
    list_add_tail(&nv->node, &mm->vmas);
}

/* MMAP_TOP dan pastga qarab len uchun bo'sh joy (oraliqlarda 1 sahifa bo'shliq). */
static uint64_t find_free_area(struct mm *mm, uint64_t len)
{
    uint64_t candidate = MMAP_TOP - len;
    for (;;) {
        bool moved = false;
        struct vma *v;
        list_for_each_entry(v, &mm->vmas, node) {
            if (candidate < v->end + PAGE_SIZE && candidate + len + PAGE_SIZE > v->start) {
                if (v->start < len + 2 * PAGE_SIZE + USER_SPACE_START)
                    return 0;
                candidate = v->start - len - PAGE_SIZE;
                moved = true;
            }
        }
        if (!moved)
            return candidate >= USER_SPACE_START ? candidate : 0;
    }
}

uint64_t mm_map(struct mm *mm, uint64_t addr, uint64_t len, uint32_t prot, uint32_t flags)
{
    len = ALIGN_UP(len, PAGE_SIZE);
    if (len == 0)
        return 0;
    struct vma *nv = kmem_cache_alloc(vma_cache);
    if (!nv)
        return 0;
    spin_lock(&mm->lock);
    if (addr == 0)
        addr = find_free_area(mm, len);
    if (!addr || !IS_ALIGNED(addr, PAGE_SIZE) || addr < USER_SPACE_START || addr + len < addr ||
        addr + len > USER_SPACE_END || !range_free(mm, addr, addr + len)) {
        spin_unlock(&mm->lock);
        kmem_cache_free(vma_cache, nv);
        return 0;
    }
    nv->start = addr;
    nv->end = addr + len;
    nv->prot = prot;
    nv->flags = flags;
    insert_vma(mm, nv);
    spin_unlock(&mm->lock);
    return addr;
}

/* [start, end) oralig'idagi sahifalarni olib tashlash (mm->lock ushlangan). */
static void unmap_pages(struct mm *mm, uint64_t start, uint64_t end)
{
    for (uint64_t va = start; va < end; va += PAGE_SIZE) {
        uint64_t phys = vmm_unmap_page(mm->pml4, va);
        if (phys) {
            struct page *pg = phys_to_page(phys);
            if (pg->mapcount)
                pg->mapcount--;
            put_page(pg);
        }
    }
}

int mm_unmap(struct mm *mm, uint64_t addr, uint64_t len)
{
    len = ALIGN_UP(len, PAGE_SIZE);
    if (!IS_ALIGNED(addr, PAGE_SIZE) || len == 0 || addr + len < addr)
        return -1;
    uint64_t end = addr + len;
    struct vma *split = kmem_cache_alloc(vma_cache);    /* o'rtadan kesish uchun zaxira */
    spin_lock(&mm->lock);
    struct vma *v, *tmp;
    list_for_each_entry_safe(v, tmp, &mm->vmas, node) {
        if (end <= v->start || addr >= v->end)
            continue;
        uint64_t cut_start = MAX(addr, v->start), cut_end = MIN(end, v->end);
        unmap_pages(mm, cut_start, cut_end);
        vmm_prune_tables(mm->pml4, cut_start, cut_end);
        if (cut_start == v->start && cut_end == v->end) {       /* butunlay */
            list_del(&v->node);
            kmem_cache_free(vma_cache, v);
        } else if (cut_start == v->start) {                     /* boshidan */
            v->start = cut_end;
        } else if (cut_end == v->end) {                         /* oxiridan */
            v->end = cut_start;
        } else if (split) {                                     /* o'rtasidan: ikkiga bo'lamiz */
            *split = *v;
            split->start = cut_end;
            v->end = cut_start;
            insert_vma(mm, split);
            split = NULL;
        }
    }
    spin_unlock(&mm->lock);
    if (split)
        kmem_cache_free(vma_cache, split);
    return 0;
}

uint64_t mm_sbrk(struct mm *mm, int64_t inc)
{
    spin_lock(&mm->lock);
    uint64_t old = mm->brk;
    uint64_t new_brk = old + (uint64_t)inc;
    struct vma *heap = NULL, *v;
    list_for_each_entry(v, &mm->vmas, node)
        if (v->flags & VMA_HEAP)
            heap = v;
    if (!heap || (inc > 0 && new_brk < old) || (inc < 0 && new_brk < mm->brk_start) ||
        new_brk > MMAP_TOP) {
        spin_unlock(&mm->lock);
        return (uint64_t)-1;
    }
    uint64_t new_end = MAX(ALIGN_UP(new_brk, PAGE_SIZE), heap->start + PAGE_SIZE);
    if (new_end > heap->end) {
        /* Kengaytirish: faqat VMA - sahifalar birinchi murojaatda (demand paging). */
        struct vma *next = list_entry(heap->node.next, struct vma, node);
        if (&next->node != &mm->vmas && new_end > next->start) {
            spin_unlock(&mm->lock);
            return (uint64_t)-1;        /* keyingi hudud bilan to'qnashuv */
        }
        heap->end = new_end;
    } else if (new_end < heap->end) {
        unmap_pages(mm, new_end, heap->end);    /* xotirani tizimga QAYTARAMIZ */
        vmm_prune_tables(mm->pml4, new_end, heap->end);
        heap->end = new_end;
    }
    mm->brk = new_brk;
    spin_unlock(&mm->lock);
    return old;
}

/* ---- Page fault ---- */

/* Bitta sahifani hal qilish (mm->lock ushlangan). */
static bool fault_page(struct mm *mm, struct vma *v, uint64_t page, bool write)
{
    uint64_t *pte = vmm_get_pte(mm->pml4, page, true);
    if (!pte)
        return false;                   /* jadval uchun xotira yo'q */

    if (!(*pte & PTE_PRESENT)) {
        /* DEMAND PAGING: birinchi murojaat - yangi nollangan sahifa. */
        struct page *pg = alloc_pages(0, GFP_ZERO);
        if (!pg)
            return false;               /* xotira tugadi (OOM) */
        pg->mapcount = 1;
        *pte = page_to_phys(pg) | prot_to_pte(v->prot) | PTE_PRESENT;
        return true;
    }

    if (write && (*pte & PTE_COW)) {
        /* COPY-ON-WRITE */
        uint64_t old_phys = *pte & PTE_ADDR_MASK;
        struct page *old = phys_to_page(old_phys);
        if (__atomic_load_n(&old->refcount, __ATOMIC_ACQUIRE) == 1) {
            /* Boshqa hech kim ulashmayapti - nusxalash shart emas, o'zlashtiramiz. */
            *pte = (*pte & ~PTE_COW) | PTE_WRITABLE;
        } else {
            struct page *np = alloc_pages(0, 0);
            if (!np)
                return false;
            memcpy(page_to_virt(np), phys_to_virt(old_phys), PAGE_SIZE);
            np->mapcount = 1;
            *pte = page_to_phys(np) | prot_to_pte(v->prot) | PTE_PRESENT;
            if (old->mapcount)
                old->mapcount--;
            put_page(old);              /* eski sahifadan bitta ulushimizni qaytaramiz */
        }
        cpu_invlpg(page);               /* eski "faqat o'qish" yozuvi TLB da qolmasin */
        return true;
    }
    return write ? (*pte & PTE_WRITABLE) != 0 : true;
}

/* Stek VMA sini kengaytirish (addr uning ostida, 8 MB chegarasida bo'lsa). */
static struct vma *grow_stack(struct mm *mm, uint64_t addr)
{
    struct vma *v;
    list_for_each_entry(v, &mm->vmas, node) {
        if (!(v->flags & VMA_STACK) || addr >= v->start)
            continue;
        uint64_t limit = USER_STACK_TOP - USER_STACK_MAX;
        if (addr < limit)
            return NULL;                /* stek to'ldi (8 MB) */
        uint64_t new_start = ALIGN_DOWN(addr, PAGE_SIZE);
        struct vma *prev = list_entry(v->node.prev, struct vma, node);
        if (&prev->node != &mm->vmas && prev->end + PAGE_SIZE > new_start)
            return NULL;                /* boshqa hududga urilib qolmaylik */
        v->start = new_start;
        return v;
    }
    return NULL;
}

bool mm_handle_fault(struct mm *mm, uint64_t addr, uint64_t err)
{
    bool present = err & 1, write = err & 2, ifetch = err & 16;
    (void)present;
    if (!mm || !is_user_address(addr))
        return false;
    spin_lock(&mm->lock);
    struct vma *v = find_vma(mm, addr);
    if (!v)
        v = grow_stack(mm, addr);
    bool ok = v && !(write && !(v->prot & PROT_WRITE)) && !(ifetch && !(v->prot & PROT_EXEC)) &&
              fault_page(mm, v, ALIGN_DOWN(addr, PAGE_SIZE), write);
    spin_unlock(&mm->lock);
    return ok;
}

bool mm_prefault(struct mm *mm, uint64_t addr, uint64_t len, bool write)
{
    if (len == 0)
        return true;
    uint64_t end = addr + len;
    if (!mm || end < addr || !is_user_address(addr) || end > USER_SPACE_END)
        return false;
    spin_lock(&mm->lock);
    for (uint64_t page = ALIGN_DOWN(addr, PAGE_SIZE); page < end; page += PAGE_SIZE) {
        struct vma *v = find_vma(mm, page);
        if (!v)
            v = grow_stack(mm, page);
        if (!v || (write && !(v->prot & PROT_WRITE)) || !fault_page(mm, v, page, write)) {
            spin_unlock(&mm->lock);
            return false;
        }
    }
    spin_unlock(&mm->lock);
    return true;
}

bool mm_populate(struct mm *mm, uint64_t addr, uint64_t len)
{
    return mm_prefault(mm, addr, len, false);
}

/* ---- fork ---- */

/* Pastki yarimdagi har bir mavjud sahifa: ikkala tomonda ham COW. */
static bool copy_user_tables(uint64_t parent_pml4, uint64_t child_pml4)
{
    uint64_t *l4 = phys_to_virt(parent_pml4);
    for (int i = 0; i < 256; i++) {
        if (!(l4[i] & PTE_PRESENT))
            continue;
        uint64_t *l3 = phys_to_virt(l4[i] & PTE_ADDR_MASK);
        for (int j = 0; j < 512; j++) {
            if (!(l3[j] & PTE_PRESENT))
                continue;
            uint64_t *l2 = phys_to_virt(l3[j] & PTE_ADDR_MASK);
            for (int k = 0; k < 512; k++) {
                if (!(l2[k] & PTE_PRESENT))
                    continue;
                uint64_t *l1 = phys_to_virt(l2[k] & PTE_ADDR_MASK);
                for (int m = 0; m < 512; m++) {
                    uint64_t pte = l1[m];
                    if (!(pte & PTE_PRESENT))
                        continue;
                    uint64_t va = ((uint64_t)i << 39) | ((uint64_t)j << 30) |
                                  ((uint64_t)k << 21) | ((uint64_t)m << 12);
                    if (pte & PTE_WRITABLE) {
                        pte = (pte & ~PTE_WRITABLE) | PTE_COW;
                        l1[m] = pte;            /* ota ham endi yozsa - nusxa oladi */
                    }
                    struct page *pg = phys_to_page(pte & PTE_ADDR_MASK);
                    get_page(pg);
                    pg->mapcount++;
                    uint64_t *cpte = vmm_get_pte(child_pml4, va, true);
                    if (!cpte) {
                        put_page(pg);
                        return false;
                    }
                    *cpte = pte;
                }
            }
        }
    }
    return true;
}

struct mm *mm_fork(struct mm *parent)
{
    struct mm *child = mm_create();
    if (!child)
        return NULL;
    spin_lock(&parent->lock);
    child->brk_start = parent->brk_start;
    child->brk = parent->brk;
    struct vma *v;
    bool ok = true;
    list_for_each_entry(v, &parent->vmas, node) {
        struct vma *nv = kmem_cache_alloc(vma_cache);
        if (!nv) {
            ok = false;
            break;
        }
        *nv = *v;
        list_add_tail(&nv->node, &child->vmas);
    }
    if (ok)
        ok = copy_user_tables(parent->pml4, child->pml4);
    spin_unlock(&parent->lock);
    /* Otaning yozish ruxsatlari olib tashlandi - TLB dagi eski yozuvlar ketsin.
     * (CR3 ni qayta yuklash global bo'lmagan barcha yozuvlarni tozalaydi.) */
    if (cpu_read_cr3() == parent->pml4)
        cpu_write_cr3(parent->pml4);
    if (!ok) {
        mm_destroy(child);
        return NULL;
    }
    return child;
}
