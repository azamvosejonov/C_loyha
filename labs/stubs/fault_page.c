    /* TODO: (1) sahifa yo'q -> demand paging: nollangan sahifa; (2) yozish + PTE_COW ->
     * copy-on-write (refcount == 1 bo'lsa o'zlashtirish, aks holda nusxa). TLB! */
    (void)mm;
    (void)v;
    (void)page;
    (void)write;
    (void)prot_to_pte;
    return false;
