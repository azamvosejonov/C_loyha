/* =============================================================================
 *  18 - Xotira xatolarini topish va tuzatish                   [3-modul: xotira]
 * =============================================================================
 *
 *  VAZIFA:
 *    Bu mashqda kod YOZILGAN - lekin undagi 5 ta funksiyaning HAR BIRIDA
 *    bittadan xotira xatosi bor. Ularni toping va tuzating.
 *
 *    1. `tools/mashq.py tekshir 18` ni ishga tushiring.
 *    2. Sanitizer hisobotini o'qing: xato turi (heap-buffer-overflow,
 *       heap-use-after-free, memory leak ...), '#0 ... yechim.c:QATOR' - qayerda.
 *    3. Tuzating, qayta ishga tushiring. Keyingi xato chiqadi. 5 tasi tugaguncha.
 *
 *  NEGA BU MASHQ ENG MUHIMLARIDAN:
 *    C dasturchisi vaqtining katta qismi - xato qidirish. Yadroda sanitizer
 *    (deyarli) yo'q: xato "jim" xotirani buzadi va 10 daqiqadan keyin
 *    butunlay boshqa joyda PANIC beradi. Shuning uchun bu xatolarni ko'rganda
 *    darhol TANIYDIGAN bo'lishingiz kerak. Hisobotni o'qishni o'rganing -
 *    gdb bilan birga bu sizning asosiy qurolingiz.
 *
 *  MASLAHAT:
 *    Har bir xato - 1-2 belgilik tuzatish. Funksiyani qayta yozmang, xatoni toping.
 *    Topa olmasangiz: shu mashqdagi test.c qaysi bo'limda yiqilganini ko'ring.
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 18
 * ============================================================================= */
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "mashq.h"

/* Satrning malloc qilingan nusxasi (strdup). */
char *nusxa(const char *s)
{
    size_t n = strlen(s);
    char *p = malloc(n);
    if (!p)
        return NULL;
    memcpy(p, s, n + 1);
    return p;
}

/* int massivining malloc qilingan nusxasi. */
int *massiv_nusxa(const int *a, size_t n)
{
    int *p = malloc(n);
    if (!p)
        return NULL;
    for (size_t i = 0; i < n; i++)
        p[i] = a[i];
    return p;
}

/* Massiv elementlari yig'indisi. */
long yigindi(const int *a, size_t n)
{
    long s = 0;
    for (size_t i = 0; i <= n; i++)
        s += a[i];
    return s;
}

/* Satrning katta harfli nusxasi. */
char *katta_harf(const char *s)
{
    char *vaqtincha = nusxa(s);
    if (!vaqtincha)
        return NULL;
    for (char *p = vaqtincha; *p; p++)
        *p = (char)toupper((unsigned char)*p);
    char *natija = nusxa(vaqtincha);
    return natija;
}

/* Bitta tugunni ozod qilish. (Haqiqiy kodda free ko'pincha shunday o'ram
 * funksiya ichida bo'ladi - kompilyator esa bunday xatoni ko'ra olmaydi.) */
static void tugunni_ozod(struct tugun *t)
{
    free(t);
}

/* Bog'langan ro'yxatni ozod qilish. */
void royxat_ozod(struct tugun *bosh)
{
    for (struct tugun *p = bosh; p; p = p->keyingi)
        tugunni_ozod(p);
}
