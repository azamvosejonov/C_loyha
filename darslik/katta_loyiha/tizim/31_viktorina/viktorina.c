/* viktorina.c - savollar bazasini fayldan o'qib, interaktiv test o'tkazadi va natija bo'yicha qaysi boblarni qayta o'qishni aytadi */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MAKS_SAVOL 64
#define MAKS_QATOR 512

struct savol {
    int bob;
    char matn[160];
    char variant[4][80];
    int javob;                      /* 0..3 */
    char izoh[160];
};

static struct savol baza[MAKS_SAVOL];
static int soni;

/* "a|b|c" kabi qatorni '|' bo'yicha bo'ladi (strtok ISHLATMAYMIZ: bo'sh maydonni yutib yuboradi) */
static int bol(char *qator, char *maydon[], int maks)
{
    int k = 0;
    maydon[k++] = qator;
    for (char *p = qator; *p && k < maks; p++)
        if (*p == '|') {
            *p = '\0';
            maydon[k++] = p + 1;
        }
    return k;
}

static int yukla(const char *yol)
{
    FILE *f = fopen(yol, "r");
    if (!f) {
        fprintf(stderr, "%s ochilmadi: %s\n", yol, strerror(errno));
        return -1;
    }
    char qator[MAKS_QATOR];
    int raqam = 0;
    while (fgets(qator, sizeof(qator), f)) {
        raqam++;
        qator[strcspn(qator, "\n")] = '\0';
        if (qator[0] == '#' || qator[0] == '\0')
            continue;
        char *m[8];
        if (bol(qator, m, 8) != 8 || soni >= MAKS_SAVOL) {
            fprintf(stderr, "%s:%d: noto'g'ri qator\n", yol, raqam);
            fclose(f);
            return -1;
        }
        struct savol *s = &baza[soni++];
        s->bob = atoi(m[0]);
        snprintf(s->matn, sizeof(s->matn), "%s", m[1]);
        for (int i = 0; i < 4; i++)
            snprintf(s->variant[i], sizeof(s->variant[i]), "%s", m[2 + i]);
        s->javob = atoi(m[6]);
        snprintf(s->izoh, sizeof(s->izoh), "%s", m[7]);
        if (s->javob < 0 || s->javob > 3) {
            fprintf(stderr, "%s:%d: javob 0..3 bo'lishi kerak\n", yol, raqam);
            fclose(f);
            return -1;
        }
    }
    fclose(f);
    return 0;
}

/* a/b/c/d harfini 0..3 ga aylantiradi; yaroqsiz bo'lsa -1 */
static int harf_raqam(const char *s)
{
    if (s[0] >= 'a' && s[0] <= 'd' && (s[1] == '\0' || s[1] == '\n'))
        return s[0] - 'a';
    return -1;
}

int main(int argc, char **argv)
{
    if (yukla(argc > 1 ? argv[1] : "savollar.txt") != 0)
        return 1;

    int togri = 0, xato = 0;
    int xato_bob[32] = {0};
    char javob[32];

    for (int i = 0; i < soni; i++) {
        struct savol *s = &baza[i];
        printf("\n[%d/%d] (%d-bob) %s\n", i + 1, soni, s->bob, s->matn);
        for (int v = 0; v < 4; v++)
            printf("  %c) %s\n", 'a' + v, s->variant[v]);
        printf("javob (a-d): ");
        fflush(stdout);
        int tanlov = -1;
        if (fgets(javob, sizeof(javob), stdin))
            tanlov = harf_raqam(javob);
        else
            printf("\n");
        if (tanlov == s->javob) {
            printf("TO'G'RI\n");
            togri++;
        } else {
            printf("XATO. To'g'ri javob: %c) %s\n  izoh: %s\n", 'a' + s->javob, s->variant[s->javob], s->izoh);
            xato++;
            if (s->bob >= 0 && s->bob < 32)
                xato_bob[s->bob]++;
        }
    }

    printf("\n===== NATIJA =====\nto'g'ri: %d, xato: %d, ball: %d%%\n", togri, xato, soni ? togri * 100 / soni : 0);
    if (xato) {
        printf("qayta o'qing:");
        for (int b = 0; b < 32; b++)
            if (xato_bob[b])
                printf(" %d-bob", b);
        printf("\n");
    } else {
        printf("hammasi to'g'ri - zo'r!\n");
    }
    return 0;
}
