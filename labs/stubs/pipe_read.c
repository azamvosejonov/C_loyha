    /* TODO: halqa buferdan o'qish. Bo'sh bo'lsa: yozuvchi yo'q -> 0 (EOF), signal ->
     * -EINTR, aks holda uxlash. O'qigach yozuvchilarni uyg'oting. */
    (void)f;
    (void)dst;
    (void)len;
    (void)off;
    return -ENOSYS;
