    /* TODO: cwd + path -> kanonik yo'l out ga: "//" -> "/", "." tashlanadi,
     * ".." oldingi komponentni olib tashlaydi, ildizdan yuqoriga chiqilmaydi.
     * Natija sig'masa -ENAMETOOLONG. Bo'sh natija = "/". */
    (void)cwd;
    (void)path;
    (void)size;
    out[0] = '\0';
    return -ENOSYS;
