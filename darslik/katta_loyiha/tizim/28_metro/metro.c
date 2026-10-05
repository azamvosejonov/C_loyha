/* metro.c - vaznli graf: Dijkstra (eng tez yo'l), Kruskal (eng arzon tarmoq), trie (nom bo'yicha qidirish) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAKS 32
#define INF 1000000

static char nom[MAKS][24];
static int n;                                   /* stansiyalar soni */
static int vazn[MAKS][MAKS];                    /* qo'shnilik matritsasi: yo'l vaqti (daqiqa) yoki INF */

struct qirra {
    int a, b, vazn;
};
static struct qirra qirralar[MAKS * MAKS];
static int qirra_soni;

static int id_ol(const char *s)                 /* nom -> raqam; yangi bo'lsa qo'shadi */
{
    for (int i = 0; i < n; i++)
        if (strcmp(nom[i], s) == 0)
            return i;
    if (n == MAKS)
        return -1;
    snprintf(nom[n], sizeof(nom[n]), "%s", s);
    return n++;
}

static int id_top(const char *s)                /* nom -> raqam; yo'q bo'lsa -1 */
{
    for (int i = 0; i < n; i++)
        if (strcmp(nom[i], s) == 0)
            return i;
    return -1;
}

/* --- Dijkstra: bir stansiyadan hammasigacha eng qisqa vaqt. Har qadamda eng yaqin ko'rilmagan tugunni tanlaymiz --- */
static void yol_top(int dan, int ga)
{
    int masofa[MAKS], oldingi[MAKS], tayyor[MAKS];
    for (int i = 0; i < n; i++) {
        masofa[i] = INF;
        oldingi[i] = -1;
        tayyor[i] = 0;
    }
    masofa[dan] = 0;
    for (int qadam = 0; qadam < n; qadam++) {
        int u = -1;
        for (int i = 0; i < n; i++)             /* hali ko'rilmaganlar ichida eng yaqini */
            if (!tayyor[i] && (u < 0 || masofa[i] < masofa[u]))
                u = i;
        if (u < 0 || masofa[u] == INF)
            break;                              /* qolganlariga yetib bo'lmaydi */
        tayyor[u] = 1;
        for (int v = 0; v < n; v++)             /* "bo'shashtirish" (relaxation): u orqali yo'l qisqaroqmi? */
            if (vazn[u][v] < INF && masofa[u] + vazn[u][v] < masofa[v]) {
                masofa[v] = masofa[u] + vazn[u][v];
                oldingi[v] = u;
            }
    }
    if (masofa[ga] == INF) {
        printf("%s -> %s: yo'l yo'q\n", nom[dan], nom[ga]);
        return;
    }
    int yol[MAKS], k = 0;
    for (int v = ga; v >= 0; v = oldingi[v])    /* oxiridan boshiga qarab tiklaymiz */
        yol[k++] = v;
    printf("%s -> %s: %d daqiqa, %d ta bekat\n  ", nom[dan], nom[ga], masofa[ga], k);
    while (k--)
        printf("%s%s", nom[yol[k]], k ? " -> " : "\n");
}

/* --- Kruskal: hamma stansiyani bog'laydigan eng arzon (eng qisqa jami vaqt) tarmoq. Union-find bilan sikl tekshiramiz --- */
static int ota[MAKS];

static int ildiz(int x)
{
    while (ota[x] != x) {
        ota[x] = ota[ota[x]];                   /* yo'lni qisqartirish: keyingi qidiruv tezroq */
        x = ota[x];
    }
    return x;
}

static int solishtir(const void *p, const void *q)
{
    const struct qirra *a = p, *b = q;
    return a->vazn != b->vazn ? a->vazn - b->vazn : (a->a != b->a ? a->a - b->a : a->b - b->b);
}

static void daraxt_top(void)
{
    qsort(qirralar, (size_t)qirra_soni, sizeof(qirralar[0]), solishtir);   /* eng arzonidan boshlab */
    for (int i = 0; i < n; i++)
        ota[i] = i;
    int jami = 0, tanlandi = 0;
    printf("Eng arzon tarmoq (barcha bekatlarni bog'lovchi):\n");
    for (int i = 0; i < qirra_soni && tanlandi < n - 1; i++) {
        int ra = ildiz(qirralar[i].a), rb = ildiz(qirralar[i].b);
        if (ra == rb)
            continue;                           /* ikkalasi allaqachon bog'langan: bu qirra sikl hosil qiladi, o'tkazib yuboramiz */
        ota[ra] = rb;
        jami += qirralar[i].vazn;
        tanlandi++;
        printf("  %-16s - %-16s %d\n", nom[qirralar[i].a], nom[qirralar[i].b], qirralar[i].vazn);
    }
    printf("  tanlangan %d ta yo'l, jami %d daqiqa (hamma %d ta yo'ldan)\n", tanlandi, jami, qirra_soni);
}

/* --- Trie: nomlarni harflar bo'yicha daraxt qilib saqlaymiz; prefiks bo'yicha qidirish uzunlikka bog'liq, nomlar soniga emas --- */
struct tugun {
    int bola[128];                              /* har belgi uchun keyingi tugun (0 - yo'q) */
    int oxirgi;                                 /* bu yerda biror nom tugaydimi */
};
static struct tugun trie[1024];
static int trie_soni = 1;                       /* 0 - ildiz */

static void trie_qosh(const char *s)
{
    int t = 0;
    for (; *s; s++) {
        int c = (unsigned char)*s & 127;
        if (!trie[t].bola[c])
            trie[t].bola[c] = trie_soni++;
        t = trie[t].bola[c];
    }
    trie[t].oxirgi = 1;
}

static void trie_chiqar(int t, char *bufer, int uz)
{
    if (trie[t].oxirgi) {
        bufer[uz] = '\0';
        printf("  %s\n", bufer);
    }
    for (int c = 0; c < 128; c++)
        if (trie[t].bola[c]) {
            bufer[uz] = (char)c;
            trie_chiqar(trie[t].bola[c], bufer, uz + 1);    /* alifbo tartibida chiqadi */
        }
}

static void qidir(const char *prefiks)
{
    int t = 0;
    for (const char *s = prefiks; *s; s++) {
        t = trie[t].bola[(unsigned char)*s & 127];
        if (!t) {
            printf("  '%s' bilan boshlanuvchi bekat yo'q\n", prefiks);
            return;
        }
    }
    char bufer[64];
    snprintf(bufer, sizeof(bufer), "%s", prefiks);
    printf("'%s' bilan boshlanuvchi bekatlar:\n", prefiks);
    trie_chiqar(t, bufer, (int)strlen(prefiks));
}

int main(int argc, char **argv)
{
    FILE *f = fopen(argc > 1 ? argv[1] : "metro.txt", "r");
    if (!f) {
        perror("metro.txt");
        return 1;
    }
    for (int i = 0; i < MAKS; i++)
        for (int j = 0; j < MAKS; j++)
            vazn[i][j] = INF;
    char x[24], y[24];
    int w;
    while (fscanf(f, "%23s %23s %d", x, y, &w) == 3) {
        int a = id_ol(x), b = id_ol(y);
        if (a < 0 || b < 0)
            break;
        vazn[a][b] = vazn[b][a] = w;            /* ikki tomonlama yo'l */
        qirralar[qirra_soni++] = (struct qirra){ a, b, w };
    }
    fclose(f);
    for (int i = 0; i < n; i++)
        trie_qosh(nom[i]);
    printf("%d ta bekat, %d ta yo'l o'qildi\n\n", n, qirra_soni);

    char buyruq[24], arg1[24], arg2[24];
    while (scanf("%23s", buyruq) == 1) {
        if (strcmp(buyruq, "yol") == 0 && scanf("%23s %23s", arg1, arg2) == 2) {
            int a = id_top(arg1), b = id_top(arg2);
            if (a < 0 || b < 0)
                printf("yol %s %s: bunday bekat yo'q\n", arg1, arg2);
            else
                yol_top(a, b);
        } else if (strcmp(buyruq, "daraxt") == 0) {
            daraxt_top();
        } else if (strcmp(buyruq, "qidir") == 0 && scanf("%23s", arg1) == 1) {
            qidir(arg1);
        } else {
            printf("noma'lum buyruq: %s\n", buyruq);
        }
    }
    return 0;
}
