/* =============================================================================
 *  user/bin/hello.c - birinchi user-mode dastur
 * =============================================================================
 *
 *  Bu kod CPU'ning 3-halqasida (ring 3) ishlaydi: apparatga ham, yadro
 *  xotirasiga ham tega olmaydi. printf -> stdio bufer -> write() syscall ->
 *  yadro -> VFS -> /dev/console (tty.c) -> ekran + serial. Har bir harf shu yo'lni bosib o'tadi!
 * ============================================================================= */
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    printf("Salom, dunyo! Men user rejimida (ring 3) ishlayapman.\n");
    printf("Mening pid'im: %d\n", getpid());
    printf("argc = %d\n", argc);
    for (int i = 0; i < argc; i++)
        printf("  argv[%d] = \"%s\"  (manzil %p)\n", i, argv[i], (void *)argv[i]);

    /* Stek va kod qayerda? Virtual manzillarni ko'ring. */
    int local = 0;
    printf("main() funksiyasi: %p (kod)\n", (void *)main);
    printf("lokal o'zgaruvchi: %p (stek)\n", (void *)&local);
    return 0;
}
