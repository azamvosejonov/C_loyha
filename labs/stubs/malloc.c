    /* TODO: size ni 16 ga tekislang, free_list dan birinchi sig'adigan blokni toping
     * (kerak bo'lsa bo'ling - split), band deb belgilang (next = USED_MAGIC) va
     * sarlavhadan KEYINGI manzilni qaytaring. Joy bo'lmasa - grow() va qayta urinish. */
    (void)size;
    (void)grow;
    return NULL;
