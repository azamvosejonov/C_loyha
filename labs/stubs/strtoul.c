    /* TODO: bo'shliqlar, ishora, base = 0/8/10/16/36, 0x prefiksi, *end, to'lish (ERANGE). */
    (void)base;
    if (end)
        *end = (char *)s;
    return 0;
