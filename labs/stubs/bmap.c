    /* TODO: block_path bo'yicha i_block va bilvosita bloklar orqali yurib, disk blokini
     * toping. alloc = true: yetishmayotganlarini ajrating (i_blocks, write_inode). */
    (void)in;
    (void)fblock;
    (void)alloc;
    (void)block_path;
    (void)alloc_block;
    *out = 0;
    return -EIO;
