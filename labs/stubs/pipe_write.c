    /* TODO: halqa buferga yozish. O'quvchi yo'q -> SIGPIPE + -EPIPE. To'la bo'lsa -
     * o'quvchilarni uyg'otib, uxlash. Hammasi yozilguncha davom eting. */
    (void)f;
    (void)src;
    (void)len;
    (void)off;
    return -ENOSYS;
