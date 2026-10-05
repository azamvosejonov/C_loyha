/* poyga_misol.c - XATOLI variant: hamma oqim BITTA umumiy o'zgaruvchiga qulfsiz yozadi (ThreadSanitizer bilan sinang) */
#include <pthread.h>
#include <stdio.h>

#define OQIMLAR 4
#define QADAM 100000

static long yigindi;                            /* HAMMA oqim shuni o'zgartiradi: poyga! */

static void *ishchi(void *arg)
{
    (void)arg;
    for (int i = 0; i < QADAM; i++)
        yigindi++;                              /* o'qi - qo'sh - yoz: ikki oqim bir vaqtda bajarsa, qo'shish yo'qoladi */
    return NULL;
}

int main(void)
{
    pthread_t t[OQIMLAR];
    for (int i = 0; i < OQIMLAR; i++)
        pthread_create(&t[i], NULL, ishchi, NULL);
    for (int i = 0; i < OQIMLAR; i++)
        pthread_join(t[i], NULL);
    printf("kutilgan %d, chiqdi %ld\n", OQIMLAR * QADAM, yigindi);
    return 0;
}
