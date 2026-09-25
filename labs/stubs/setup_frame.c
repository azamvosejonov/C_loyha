    /* TODO: user stekiga sigframe yozing (red zone'dan pastda, tekislash!), keyin
     * RIP = handler, RDI = sig, RSP = freym. Niqobni yangilang (sa_mask, NODEFER, RESETHAND). */
    (void)f;
    (void)sig;
    (void)act;
    return -1;
