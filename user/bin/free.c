/* =============================================================================
 *  user/bin/free.c - xotira statistikasi (Linux'dagi `free` kabi)
 * ============================================================================= */
#include <stdio.h>
#include "myos.h"

int main(void)
{
    struct myos_meminfo m;
    if (meminfo(&m) < 0)
        return 1;
    uint64_t kb = m.page_size / 1024;
    printf("Fizik xotira (PMM):\n");
    printf("  jami:   %6lu KB (%lu sahifa)\n", m.total_pages * kb, m.total_pages);
    printf("  band:   %6lu KB\n", (m.total_pages - m.free_pages) * kb);
    printf("  bo'sh:  %6lu KB\n", m.free_pages * kb);
    printf("Yadro heap'i (kmalloc):\n");
    printf("  band:   %6lu bayt\n", m.kheap_bytes_in_use);
    printf("  ajratish/bo'shatish: %lu / %lu\n", m.kheap_allocs, m.kheap_frees);
    printf("  slab sahifalar: %lu, katta ajratma sahifalari: %lu\n", m.kheap_slab_pages,
           m.kheap_large_pages);
    return 0;
}
