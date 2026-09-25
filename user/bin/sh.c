/* =============================================================================
 *  user/bin/sh.c - SHELL (buyruqlar interpretatori)
 * =============================================================================
 *
 *  Shell - oddiy user dasturi, hech qanday maxsus huquqi yo'q. Uning butun
 *  kuchi to'rtta syscall'da: fork, exec, wait, va eng muhimi - dup2/pipe.
 *
 *  QATORNING YO'LI:
 *      "ls -l /bin | grep sh > f.txt && cat f.txt"
 *         │
 *         ▼ 1. LEKSER (lex)        so'zlar va operatorlar; qo'shtirnoqlar ochiladi,
 *         │                        $VAR almashtiriladi
 *      [ls][-l][/bin] | [grep][sh] > [f.txt] && [cat][f.txt]
 *         │
 *         ▼ 2. PARSER (parse_pipeline)   buyruqlar + yo'naltirishlar, '|' bilan
 *         │                              zanjir ("pipeline")
 *      pipeline{ ls -l /bin  |  grep sh (stdout -> f.txt) }  &&  pipeline{ cat f.txt }
 *         │
 *         ▼ 3. BAJARISH (run_pipeline)   har bir buyruq uchun fork; bolada
 *                                        dup2 bilan stdin/stdout ulanadi, keyin exec
 *
 *  QO'LLANADIGAN SINTAKSIS:
 *      a | b | c          pipe: a ning stdout -> b ning stdin ...
 *      < f   > f   >> f   stdin fayldan / stdout faylga (qayta yozish / qo'shish)
 *      2> f   2>&1        stderr faylga / stderr -> stdout
 *      a ; b              ketma-ket
 *      a && b   a || b    b faqat a muvaffaqiyatli (0) / muvaffaqiyatsiz bo'lsa
 *      a &                fonda (shell kutmaydi)
 *      'matn'  "matn"     qo'shtirnoqlar ("..." ichida $VAR ochiladi)
 *      $?  $$  $NOM       oxirgi chiqish kodi, shell pid'i, o'zgaruvchi
 *      NOM=qiymat         shell o'zgaruvchisi
 *      *.txt  ?           glob: mos fayl nomlari bilan almashtiriladi
 *      # izoh
 *
 *  JOB CONTROL (interaktiv rejimda):
 *      Har bir pipeline - alohida JARAYON GURUHI (job). Terminal faqat
 *      "oldingi plandagi" guruhga tegishli: Ctrl-C/Ctrl-Z faqat unga boradi,
 *      shell esa o'zi SIGINT/SIGTSTP ni e'tiborsiz qoldiradi.
 *      Ctrl-Z   - job to'xtaydi         jobs  - ro'yxat
 *      fg [%n]  - oldingi planga        bg [%n] - fonda davom ettirish
 *
 *  Ishga tushirish:  sh (interaktiv) | sh skript.sh | sh -c "buyruqlar"
 * ============================================================================= */
#include <ctype.h>
#include <signal.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include "myos.h"

#define LINE_MAX   1024
#define TOKENS_MAX 128
#define ARGS_MAX   64
#define REDIR_MAX  8
#define PIPE_MAX   16
#define VARS_MAX   64

/* ============================================================================
 *  Shell o'zgaruvchilari
 * ========================================================================== */

static struct {
    char *name, *value;
} vars[VARS_MAX];

static int last_status;                 /* $? */

/* ---- Job control holati ---- */
#define JOBS_MAX 16

struct job {
    int id;                             /* [1], [2] ... (0 - bo'sh slot) */
    int pgid;
    int pids[PIPE_MAX];
    int npids;
    int alive;                          /* hali tugamagan jarayonlar */
    bool stopped;
    int status;                         /* oxirgi buyruqning holat so'zi */
    char cmd[128];
};

static struct job jobs[JOBS_MAX];
static bool job_control;                /* interaktiv terminal: guruhlar, fg/bg */
static int shell_pgid;
static volatile sig_atomic_t got_sigint;

struct pipeline;
static struct job *job_by_pid(int pid, int *index);
static void job_update(struct job *j, int k, int status);
static int job_foreground(struct job *j, bool resume);
static struct job *job_find(const char *spec);
static void jobs_list(void);
static void reap_background(void);

static const char *var_get(const char *name)
{
    for (int i = 0; i < VARS_MAX; i++)
        if (vars[i].name && strcmp(vars[i].name, name) == 0)
            return vars[i].value;
    return NULL;
}

static void var_set(const char *name, const char *value)
{
    int free_slot = -1;
    for (int i = 0; i < VARS_MAX; i++) {
        if (vars[i].name && strcmp(vars[i].name, name) == 0) {
            free(vars[i].value);
            vars[i].value = strdup(value);
            return;
        }
        if (!vars[i].name && free_slot < 0)
            free_slot = i;
    }
    if (free_slot < 0) {
        fprintf(stderr, "sh: o'zgaruvchilar juda ko'p\n");
        return;
    }
    vars[free_slot].name = strdup(name);
    vars[free_slot].value = strdup(value);
}

static bool is_name_char(char c, bool first)
{
    return c == '_' || isalpha((unsigned char)c) || (!first && isdigit((unsigned char)c));
}

/* ============================================================================
 *  1. LEKSER: qatorni tokenlarga ajratish
 * ========================================================================== */

enum tok {
    T_WORD,
    T_PIPE,         /* |  */
    T_OR,           /* || */
    T_AMP,          /* &  */
    T_AND,          /* && */
    T_SEMI,         /* ;  */
    T_IN,           /* <  */
    T_OUT,          /* >  */
    T_APPEND,       /* >> */
    T_ERR_OUT,      /* 2> */
    T_ERR_APPEND,   /* 2>> */
    T_ERR_TO_OUT,   /* 2>&1 */
    T_END,
};

struct token {
    enum tok type;
    char *raw;                          /* T_WORD: XOM matn (qo'shtirnoqlar hali bor) */
};

/* So'z yig'iladigan dinamik bufer. */
struct wbuf {
    char data[LINE_MAX];
    size_t len;
    bool overflow;
};

static void wb_putc(struct wbuf *w, char c)
{
    if (w->len < sizeof(w->data) - 1)
        w->data[w->len++] = c;
    else
        w->overflow = true;
}

static void wb_puts(struct wbuf *w, const char *s)
{
    while (*s)
        wb_putc(w, *s++);
}

/* $... ni ochish. p '$' dan keyingi belgiga ko'rsatadi; qaytaradi: yangi p. */
static const char *expand_var(const char *p, struct wbuf *w)
{
    char num[24];
    if (*p == '?') {
        snprintf(num, sizeof(num), "%d", last_status);
        wb_puts(w, num);
        return p + 1;
    }
    if (*p == '$') {
        snprintf(num, sizeof(num), "%d", getpid());
        wb_puts(w, num);
        return p + 1;
    }
    bool braces = *p == '{';
    if (braces)
        p++;
    if (!is_name_char(*p, true)) {      /* yolg'iz '$' - oddiy belgi */
        wb_putc(w, '$');
        return braces ? p - 1 : p;
    }
    char name[64];
    size_t n = 0;
    while (is_name_char(*p, n == 0) && n < sizeof(name) - 1)
        name[n++] = *p++;
    name[n] = '\0';
    if (braces && *p == '}')
        p++;
    const char *v = var_get(name);
    if (v)
        wb_puts(w, v);
    return p;
}

static bool is_op_char(char c)
{
    return c == '|' || c == '&' || c == ';' || c == '<' || c == '>';
}

/* Qaytaradi: tokenlar soni (oxirida T_END), yoki -1 (sintaksis xatosi). */
static int lex(const char *p, struct token *toks)
{
    int n = 0;
    for (;;) {
        if (n == TOKENS_MAX - 1) {
            fprintf(stderr, "sh: qator juda uzun\n");
            return -1;
        }
        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
            p++;
        if (!*p || *p == '#') {
            toks[n++] = (struct token){ T_END, NULL };
            return n;
        }
        struct token *t = &toks[n];
        *t = (struct token){ T_WORD, NULL };

        /* --- Operatorlar (eng uzunini birinchi tekshiramiz) --- */
        if (p[0] == '2' && p[1] == '>' && p[2] == '&' && p[3] == '1') {
            t->type = T_ERR_TO_OUT, p += 4;
        } else if (p[0] == '2' && p[1] == '>' && p[2] == '>') {
            t->type = T_ERR_APPEND, p += 3;
        } else if (p[0] == '2' && p[1] == '>') {
            t->type = T_ERR_OUT, p += 2;
        } else if (p[0] == '|' && p[1] == '|') {
            t->type = T_OR, p += 2;
        } else if (p[0] == '&' && p[1] == '&') {
            t->type = T_AND, p += 2;
        } else if (p[0] == '>' && p[1] == '>') {
            t->type = T_APPEND, p += 2;
        } else if (*p == '|') {
            t->type = T_PIPE, p++;
        } else if (*p == '&') {
            t->type = T_AMP, p++;
        } else if (*p == ';') {
            t->type = T_SEMI, p++;
        } else if (*p == '<') {
            t->type = T_IN, p++;
        } else if (*p == '>') {
            t->type = T_OUT, p++;
        }
        if (t->type != T_WORD) {
            n++;
            continue;
        }

        /* --- So'z: faqat CHEGARASINI topamiz. Qo'shtirnoqlarni ochish va $ ni
         * almashtirish KEYINROQ - buyruq bajarilishidan oldin (expand_word).
         * Nega? "false || echo $?" da $? false TUGAGANDAN keyin hisoblanishi kerak. */
        const char *start = p;
        while (*p && *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r' && !is_op_char(*p)) {
            if (*p == '\'' || *p == '"') {
                const char *end = strchr(p + 1, *p);
                if (!end) {
                    fprintf(stderr, "sh: yopilmagan %c qo'shtirnoq\n", *p);
                    return -1;
                }
                p = end + 1;
            } else if (*p == '\\' && p[1]) {
                p += 2;
            } else {
                p++;
            }
        }
        size_t len = (size_t)(p - start);
        t->raw = malloc(len + 1);
        memcpy(t->raw, start, len);
        t->raw[len] = '\0';
        n++;
    }
}

/* So'zni ochish: qo'shtirnoqlar olib tashlanadi, $... almashtiriladi.
 * *glob = qo'shtirnoqsiz * yoki ? bor. Qaytaradi: malloc qilingan satr, yoki
 * NULL - so'z yo'qoldi (qo'shtirnoqsiz bo'sh $VAR, sh qoidasi). */
static char *expand_word(const char *p, bool *glob)
{
    static struct wbuf w;
    w.len = 0;
    w.overflow = false;
    bool quoted = false;
    *glob = false;
    while (*p) {
        if (*p == '\'') {              /* '...' - hech narsa ochilmaydi */
            quoted = true;
            for (p++; *p && *p != '\''; p++)
                wb_putc(&w, *p);
            if (*p)
                p++;
        } else if (*p == '"') {         /* "..." - faqat $ va \ maxsus */
            quoted = true;
            for (p++; *p && *p != '"';) {
                if (*p == '\\' && (p[1] == '"' || p[1] == '\\' || p[1] == '$')) {
                    wb_putc(&w, p[1]);
                    p += 2;
                } else if (*p == '$') {
                    p = expand_var(p + 1, &w);
                } else {
                    wb_putc(&w, *p++);
                }
            }
            if (*p)
                p++;
        } else if (*p == '\\' && p[1]) {  /* \x - x ni o'zicha */
            quoted = true;
            wb_putc(&w, p[1]);
            p += 2;
        } else if (*p == '$') {
            p = expand_var(p + 1, &w);
        } else {
            if (*p == '*' || *p == '?')
                *glob = true;
            wb_putc(&w, *p++);
        }
    }
    if (w.overflow)
        fprintf(stderr, "sh: so'z juda uzun (qirqildi)\n");
    if (w.len == 0 && !quoted)
        return NULL;
    w.data[w.len] = '\0';
    return strdup(w.data);
}

static void free_tokens(struct token *toks, int n)
{
    for (int i = 0; i < n; i++)
        free(toks[i].raw);
}

/* ============================================================================
 *  Glob: "*.txt" -> mos fayl nomlari (faqat oxirgi komponentda)
 * ========================================================================== */

/* '*' - istalgan (bo'sh ham) ketma-ketlik, '?' - bitta belgi. Rekursiv backtracking. */
static bool glob_match(const char *pat, const char *s)
{
    /* >>> LAB glob_match - vazifa: labs/README.md */
    for (; *pat; pat++, s++) {
        if (*pat == '*') {
            while (pat[1] == '*')
                pat++;
            for (const char *t = s;; t++) {
                if (glob_match(pat + 1, t))
                    return true;
                if (!*t)
                    return false;
            }
        }
        if (!*s || (*pat != '?' && *pat != *s))
            return false;
    }
    return *s == '\0';
    /* <<< LAB glob_match */
}

static int cmp_str(const void *a, const void *b)
{
    return strcmp(*(char *const *)a, *(char *const *)b);
}

/* pattern ni argv ga ochadi. Qaytaradi: qo'shilgan so'zlar soni (0 - mos yo'q). */
static int glob_expand(const char *pattern, char **argv, int room)
{
    char dir[PATH_MAX], prefix[PATH_MAX];
    const char *slash = strrchr(pattern, '/');
    const char *pat = slash ? slash + 1 : pattern;
    if (slash) {
        size_t dlen = (size_t)(slash - pattern);
        snprintf(dir, sizeof(dir), "%.*s", (int)(dlen ? dlen : 1), pattern);
        snprintf(prefix, sizeof(prefix), "%.*s/", (int)dlen, pattern);
    } else {
        strcpy(dir, ".");
        prefix[0] = '\0';
    }
    DIR *d = opendir(dir);
    if (!d)
        return 0;
    int n = 0;
    struct dirent *de;
    while ((de = readdir(d)) != NULL && n < room) {
        if (de->d_name[0] == '.' && pat[0] != '.')
            continue;                   /* yashirin fayllar faqat ".*" bilan */
        if (!glob_match(pat, de->d_name))
            continue;
        char full[PATH_MAX];
        snprintf(full, sizeof(full), "%s%s", prefix, de->d_name);
        argv[n++] = strdup(full);
    }
    closedir(d);
    qsort(argv, (size_t)n, sizeof(char *), cmp_str);
    return n;
}

/* ============================================================================
 *  2. PARSER: tokenlardan buyruqlar zanjiri (pipeline)
 * ========================================================================== */

enum redir_type { R_IN, R_OUT, R_APPEND, R_DUP };

struct redir {
    enum redir_type type;
    int fd;                             /* qaysi fd ni o'zgartiramiz (0, 1, 2) */
    char *path;                         /* malloc; R_DUP da NULL */
    int dup_from;                       /* R_DUP: fd = dup2(dup_from) */
};

struct cmd {
    char *argv[ARGS_MAX + 1];           /* hammasi malloc qilingan */
    int argc;
    struct redir redirs[REDIR_MAX];
    int nredir;
};

struct pipeline {
    struct cmd cmds[PIPE_MAX];
    int n;
    bool background;
};

static void free_pipeline(struct pipeline *pl)
{
    for (int i = 0; i < pl->n; i++) {
        for (int k = 0; k < pl->cmds[i].argc; k++)
            free(pl->cmds[i].argv[k]);
        for (int k = 0; k < pl->cmds[i].nredir; k++)
            free(pl->cmds[i].redirs[k].path);
    }
}

static bool add_arg(struct cmd *c, char *s)
{
    if (c->argc == ARGS_MAX) {
        fprintf(stderr, "sh: argumentlar juda ko'p\n");
        free(s);
        return false;
    }
    c->argv[c->argc++] = s;
    c->argv[c->argc] = NULL;
    return true;
}

/* toks[*pos] dan bitta pipeline'ni o'qiydi. Qaytaradi: 0 yoki -1 (xato). */
static int parse_pipeline(struct token *toks, int *pos, struct pipeline *pl)
{
    memset(pl, 0, sizeof(*pl));
    pl->n = 1;
    struct cmd *c = &pl->cmds[0];
    int i = *pos;
    for (;; i++) {
        struct token *t = &toks[i];
        switch (t->type) {
        case T_WORD: {
            bool glob;
            char *word = expand_word(t->raw, &glob);
            if (!word)
                break;                  /* bo'sh $VAR - so'z yo'q */
            if (glob) {
                char *found[ARGS_MAX];
                int k = glob_expand(word, found, ARGS_MAX - c->argc);
                if (k > 0) {
                    free(word);
                    for (int j = 0; j < k; j++)
                        if (!add_arg(c, found[j]))
                            return -1;
                    break;
                }
                /* mos fayl yo'q - so'z o'zicha qoladi (sh an'anasi) */
            }
            if (!add_arg(c, word))
                return -1;
            break;
        }
        case T_IN:
        case T_OUT:
        case T_APPEND:
        case T_ERR_OUT:
        case T_ERR_APPEND: {
            if (toks[i + 1].type != T_WORD) {
                fprintf(stderr, "sh: yo'naltirishdan keyin fayl nomi kerak\n");
                return -1;
            }
            if (c->nredir == REDIR_MAX) {
                fprintf(stderr, "sh: yo'naltirishlar juda ko'p\n");
                return -1;
            }
            bool glob;
            char *path = expand_word(toks[++i].raw, &glob);
            if (!path) {
                fprintf(stderr, "sh: bo'sh fayl nomi\n");
                return -1;
            }
            struct redir *r = &c->redirs[c->nredir++];
            r->path = path;
            r->type = t->type == T_IN ? R_IN
                    : (t->type == T_APPEND || t->type == T_ERR_APPEND) ? R_APPEND : R_OUT;
            r->fd = t->type == T_IN ? 0 : (t->type == T_ERR_OUT || t->type == T_ERR_APPEND) ? 2 : 1;
            break;
        }
        case T_ERR_TO_OUT:
            if (c->nredir == REDIR_MAX)
                return -1;
            c->redirs[c->nredir++] = (struct redir){ R_DUP, 2, NULL, 1 };
            break;
        case T_PIPE:
            if (c->argc == 0) {
                fprintf(stderr, "sh: '|' dan oldin buyruq yo'q\n");
                return -1;
            }
            if (pl->n == PIPE_MAX) {
                fprintf(stderr, "sh: pipe zanjiri juda uzun\n");
                return -1;
            }
            c = &pl->cmds[pl->n++];
            break;
        default:                        /* ; & && || END - pipeline tugadi */
            /* Bo'sh buyruq: "a | " yoki "&& b" - xato. Yolg'iz ";" - shunchaki o'tkaziladi. */
            if (c->argc == 0 && c->nredir == 0 && (pl->n > 1 || t->type != T_SEMI)) {
                fprintf(stderr, "sh: sintaksis xatosi: buyruq yo'q\n");
                return -1;
            }
            pl->background = t->type == T_AMP;
            *pos = i;
            return 0;
        }
    }
}

/* ============================================================================
 *  3. BAJARISH
 * ========================================================================== */

/* Yo'naltirishlarni joriy jarayonga qo'llash. Xatoda -1. */
static int apply_redirs(const struct cmd *c)
{
    for (int i = 0; i < c->nredir; i++) {
        const struct redir *r = &c->redirs[i];
        if (r->type == R_DUP) {
            if (dup2(r->dup_from, r->fd) < 0) {
                perror("sh: dup2");
                return -1;
            }
            continue;
        }
        int flags = r->type == R_IN ? O_RDONLY
                  : r->type == R_APPEND ? O_WRONLY | O_CREAT | O_APPEND
                  : O_WRONLY | O_CREAT | O_TRUNC;
        int fd = open(r->path, flags, 0644);
        if (fd < 0) {
            fprintf(stderr, "sh: %s: %s\n", r->path, strerror(errno));
            return -1;
        }
        if (fd != r->fd) {
            dup2(fd, r->fd);            /* masalan: fd 3 (fayl) -> fd 1 (stdout) */
            close(fd);
        }
    }
    return 0;
}

static int run_script(const char *path);

static void print_help(void)
{
    puts("Ichki buyruqlar: cd [papka], pwd, exit [kod], help, wait, set, . <skript>");
    puts("Job control:     jobs, fg [%n], bg [%n]   (Ctrl-C - to'xtatish, Ctrl-Z - pauza)");
    puts("Dasturlar /bin ichida ('ls /bin'):");
    puts("  fayllar:   ls [-la], cat, cp, mv, rm [-rf], mkdir [-p], rmdir, touch, stat");
    puts("  muharrir:  edit <fayl>  (Ctrl-S saqlash, Ctrl-Q chiqish, Ctrl-F qidirish)");
    puts("  matn:      echo, wc [-lwc], head, tail, grep [-ivnc], tee, seq");
    puts("  tizim:     ps, kill, free, dmesg, uname, lspci, date, sleep, mount, umount, sync");
    puts("  testlar:   hello, forktest, fstest, memtest, spin, crash");
    puts("  quvvat:    poweroff, reboot");
    puts("Sintaksis:   a | b   a > f   a >> f   a < f   a 2>&1   a ; b   a && b   a || b   a &");
    puts("Tahrir:      strelkalar, yuqori/pastga - tarix, Tab - to'ldirish, Ctrl-A/E/U/K/W/L");
    puts("             'matn'  \"$O'ZGARUVCHI\"  $?  NOM=qiymat  *.txt");
}

/* Ichki buyruq bo'lsa - bajarib true qaytaradi (*status ga natija). */
static bool run_builtin(struct cmd *c, int *status)
{
    const char *name = c->argv[0];
    if (strcmp(name, "cd") == 0) {
        /* cd SHELL ICHIDA bo'lishi SHART: bola jarayonning joriy papkasini
         * o'zgartirish otaga ta'sir qilmaydi. Shuning uchun /bin/cd yo'q. */
        const char *dir = c->argc > 1 ? c->argv[1] : (var_get("HOME") ? var_get("HOME") : "/");
        *status = 0;
        if (chdir(dir) < 0) {
            fprintf(stderr, "cd: %s: %s\n", dir, strerror(errno));
            *status = 1;
        }
        return true;
    }
    if (strcmp(name, "pwd") == 0) {
        char buf[PATH_MAX];
        *status = getcwd(buf, sizeof(buf)) ? (puts(buf), 0) : 1;
        return true;
    }
    if (strcmp(name, "exit") == 0) {
        fflush(NULL);
        exit(c->argc > 1 ? atoi(c->argv[1]) : last_status);
    }
    if (strcmp(name, "help") == 0) {
        print_help();
        *status = 0;
        return true;
    }
    if (strcmp(name, "wait") == 0) {   /* barcha fon joblarini kutish */
        int st, pid, k;
        while ((pid = waitpid(-1, &st, 0)) > 0) {
            struct job *j = job_by_pid(pid, &k);
            if (j) {
                job_update(j, k, st);
                if (j->alive == 0)
                    j->id = 0;
            }
        }
        *status = 0;
        return true;
    }
    if (strcmp(name, "jobs") == 0) {
        reap_background();              /* avval holatlarni yangilaymiz */
        jobs_list();
        *status = 0;
        return true;
    }
    if (strcmp(name, "fg") == 0 || strcmp(name, "bg") == 0) {
        struct job *j = job_find(c->argc > 1 ? c->argv[1] : NULL);
        if (!j) {
            fprintf(stderr, "%s: bunday job yo'q\n", name);
            *status = 1;
            return true;
        }
        if (name[0] == 'f') {
            printf("%s\n", j->cmd);
            *status = job_foreground(j, true);
        } else {
            j->stopped = false;
            kill(-j->pgid, SIGCONT);
            printf("[%d]  %s &\n", j->id, j->cmd);
            *status = 0;
        }
        return true;
    }
    if (strcmp(name, "set") == 0) {
        for (int i = 0; i < VARS_MAX; i++)
            if (vars[i].name)
                printf("%s=%s\n", vars[i].name, vars[i].value);
        *status = 0;
        return true;
    }
    if (strcmp(name, ".") == 0 || strcmp(name, "source") == 0) {
        if (c->argc < 2) {
            fprintf(stderr, "%s: fayl nomi kerak\n", name);
            *status = 1;
        } else {
            *status = run_script(c->argv[1]);
        }
        return true;
    }
    return false;
}

static bool is_builtin(const char *name)
{
    static const char *const names[] = { "cd", "pwd", "exit", "help", "wait", "set", ".",
                                         "source", "jobs", "fg", "bg", NULL };
    for (int i = 0; names[i]; i++)
        if (strcmp(name, names[i]) == 0)
            return true;
    return false;
}

/* "NOM=qiymat" ko'rinishidami? */
static bool is_assignment(const char *s)
{
    const char *eq = strchr(s, '=');
    if (!eq || eq == s)
        return false;
    for (const char *p = s; p < eq; p++)
        if (!is_name_char(*p, p == s))
            return false;
    return true;
}

/* BOLA jarayonda: buyruqni bajarish. Hech qachon qaytmaydi. */
/* `kill %2` -> `kill -- -<pgid>`: job raqamini guruh raqamiga aylantirish
 * (bash'da kill ichki buyruq; bizda /bin/kill, shuning uchun argumentlarni
 * shell o'zi tarjima qiladi). */
static void translate_kill_args(struct cmd *c)
{
    static char bufs[ARGS_MAX][16];
    int first_pid = 1;
    if (c->argc > 1 && c->argv[1][0] == '-' && c->argv[1][1] != '%')
        first_pid = 2;                  /* signal: -9, -STOP ... */
    bool any = false;
    for (int i = first_pid; i < c->argc; i++)
        any |= c->argv[i][0] == '%';
    if (!any || c->argc >= ARGS_MAX)
        return;
    /* "--" qo'shamiz: "-12" signal emas, guruh raqami ekanini bildiradi. */
    for (int i = c->argc; i > first_pid; i--)
        c->argv[i] = c->argv[i - 1];
    c->argv[first_pid] = "--";
    c->argc++;
    c->argv[c->argc] = NULL;
    for (int i = first_pid + 1; i < c->argc; i++) {
        if (c->argv[i][0] != '%')
            continue;
        struct job *j = job_find(c->argv[i]);
        snprintf(bufs[i], sizeof(bufs[i]), "-%d", j ? j->pgid : 999999);
        c->argv[i] = bufs[i];
        /* To'xtagan job'ga SIGTERM kabi signal SIGCONT gacha "osilib" turadi
         * (POSIX). Bash kabi avval davom ettiramiz - aks holda kill ta'sirsiz
         * ko'rinardi. To'xtatuvchi signallar uchun bu kerak emas. */
        const char *spec = first_pid == 2 ? c->argv[1] + 1 : "TERM";
        bool stopping = strstr("STOP TSTP TTIN TTOU CONT 18 19 20 21 22", spec) != NULL;
        if (j && j->stopped && !stopping)
            kill(-j->pgid, SIGCONT);
    }
}

__attribute__((noreturn)) static void exec_cmd(struct cmd *c)
{
    int status;
    if (strcmp(c->argv[0], "kill") == 0)
        translate_kill_args(c);         /* bola jarayonda - asl argv otada o'zgarmaydi */
    if (run_builtin(c, &status)) {      /* pipe ichidagi builtin: help | grep cd */
        fflush(NULL);
        _exit(status);
    }
    /* PATH qidiruvi: '/' bo'lmasa - /bin ichidan. */
    char path[PATH_MAX];
    if (strchr(c->argv[0], '/'))
        snprintf(path, sizeof(path), "%s", c->argv[0]);
    else
        snprintf(path, sizeof(path), "/bin/%s", c->argv[0]);
    execv(path, c->argv);
    /* exec qaytdi = xato */
    int e = errno;
    if (e == ENOENT)
        fprintf(stderr, "sh: %s: buyruq topilmadi\n", c->argv[0]);
    else
        fprintf(stderr, "sh: %s: %s\n", c->argv[0], strerror(e));
    _exit(e == ENOENT ? 127 : 126);     /* Unix an'anasi: 127 - topilmadi, 126 - bajarib bo'lmaydi */
}

/* Builtin'ni SHELL'NING O'ZIDA, yo'naltirishlar bilan: fd'larni saqlab, keyin tiklaymiz. */
static int run_builtin_here(struct cmd *c)
{
    int saved[3];
    for (int fd = 0; fd < 3; fd++)
        saved[fd] = dup(fd);
    int status = 1;
    if (apply_redirs(c) == 0) {
        if (c->argc == 0)
            status = 0;                 /* "> fayl" yolg'iz - faqat faylni yaratadi */
        else
            run_builtin(c, &status);
    }
    fflush(stdout);
    fflush(stderr);
    for (int fd = 0; fd < 3; fd++) {
        if (saved[fd] >= 0) {
            dup2(saved[fd], fd);
            close(saved[fd]);
        }
    }
    return status;
}

/* ---- Joblar ---------------------------------------------------------------- */

static struct job *job_new(const struct pipeline *pl)
{
    for (int i = 0; i < JOBS_MAX; i++) {
        if (jobs[i].id)
            continue;
        struct job *j = &jobs[i];
        memset(j, 0, sizeof(*j));
        j->id = i + 1;
        /* Buyruq matni (jobs ro'yxati uchun): "seq 100 | wc -l" */
        size_t len = 0;
        for (int c = 0; c < pl->n; c++) {
            for (int a = 0; a < pl->cmds[c].argc && len < sizeof(j->cmd) - 1; a++)
                len += (size_t)snprintf(j->cmd + len, sizeof(j->cmd) - len, "%s%s",
                                        len ? " " : "", pl->cmds[c].argv[a]);
            if (c < pl->n - 1 && len < sizeof(j->cmd) - 3)
                len += (size_t)snprintf(j->cmd + len, sizeof(j->cmd) - len, " |");
        }
        return j;
    }
    return NULL;
}

static struct job *job_by_pid(int pid, int *index)
{
    for (int i = 0; i < JOBS_MAX; i++)
        for (int k = 0; jobs[i].id && k < jobs[i].npids; k++)
            if (jobs[i].pids[k] == pid) {
                *index = k;
                return &jobs[i];
            }
    return NULL;
}

/* waitpid natijasini job holatiga yozish. */
static void job_update(struct job *j, int k, int status)
{
    if (WIFSTOPPED(status)) {
        j->stopped = true;
        return;
    }
    j->stopped = false;                 /* tugagan jarayon to'xtagan emas (tashqaridan CONT olgan bo'lishi mumkin) */
    j->pids[k] = -j->pids[k];           /* tugagan (manfiy - "o'lik" belgisi) */
    j->alive--;
    if (k == j->npids - 1)
        j->status = status;             /* pipeline natijasi - oxirgi buyruqniki */
}

/* Holat so'zidan $? qiymati (bash qoidasi). */
static int status_to_code(int st)
{
    if (WIFEXITED(st))
        return WEXITSTATUS(st);
    if (WIFSTOPPED(st))
        return 128 + WSTOPSIG(st);
    return 128 + WTERMSIG(st);
}

/* Job'ni oldingi planda kutish (buyruq yoki `fg`). Qaytaradi: $? qiymati. */
static int job_foreground(struct job *j, bool resume)
{
    if (job_control)
        tcsetpgrp(STDIN_FILENO, j->pgid);   /* klaviatura endi shu job'niki */
    if (resume) {
        j->stopped = false;
        kill(-j->pgid, SIGCONT);
    }
    while (j->alive > 0 && !j->stopped) {
        int st;
        for (int k = 0; k < j->npids && !j->stopped; k++) {
            if (j->pids[k] <= 0)
                continue;
            int r = waitpid(j->pids[k], &st, WUNTRACED);
            if (r == j->pids[k])
                job_update(j, k, st);
            else if (r < 0 && errno == ECHILD)
                job_update(j, k, 0);
        }
    }
    if (job_control)
        tcsetpgrp(STDIN_FILENO, shell_pgid);    /* terminal shell'ga qaytadi */
    if (j->stopped) {
        printf("\n[%d]+  To'xtatildi       %s\n", j->id, j->cmd);
        return 128 + SIGTSTP;
    }
    int st = j->status;
    /* Signal bilan o'lgan bo'lsa - bash kabi xabar (Ctrl-C va SIGPIPE - jim). */
    if (WIFSIGNALED(st) && WTERMSIG(st) != SIGINT && WTERMSIG(st) != SIGPIPE)
        fprintf(stderr, "%s\n", strsignal(WTERMSIG(st)));
    j->id = 0;                          /* slot bo'shadi */
    return status_to_code(st);
}

/* "%2" yoki "" (oxirgisi) -> job. */
static struct job *job_find(const char *spec)
{
    if (spec && *spec) {
        int n = atoi(spec[0] == '%' ? spec + 1 : spec);
        return (n >= 1 && n <= JOBS_MAX && jobs[n - 1].id) ? &jobs[n - 1] : NULL;
    }
    for (int i = JOBS_MAX - 1; i >= 0; i--)
        if (jobs[i].id)
            return &jobs[i];
    return NULL;
}

static void jobs_list(void)
{
    for (int i = 0; i < JOBS_MAX; i++)
        if (jobs[i].id)
            printf("[%d]  %-12s %s\n", jobs[i].id, jobs[i].stopped ? "To'xtatildi" : "Ishlayapti",
                   jobs[i].cmd);
}

static int run_pipeline(struct pipeline *pl)
{
    struct cmd *first = &pl->cmds[0];

    /* Oddiy holatlar shell ichida: o'zgaruvchi, builtin, yolg'iz yo'naltirish. */
    if (pl->n == 1 && !pl->background) {
        if (first->argc == 1 && first->nredir == 0 && is_assignment(first->argv[0])) {
            char *eq = strchr(first->argv[0], '=');
            *eq = '\0';
            var_set(first->argv[0], eq + 1);
            *eq = '=';
            return 0;
        }
        if (first->argc == 0 || is_builtin(first->argv[0]))
            return run_builtin_here(first);
    }

    /*  Pipeline: a | b | c
     *
     *      a ──pipe0──► b ──pipe1──► c
     *   stdout=p0[1]  stdin=p0[0]   stdin=p1[0]
     *                 stdout=p1[1]
     *
     *  ENG MUHIM QOIDA: har bir jarayon o'ziga KERAK BO'LMAGAN pipe uchlarini
     *  yopishi shart. Pipe'ning yozish uchi kamida bitta jarayonda ochiq qolsa,
     *  o'quvchi hech qachon EOF olmaydi va abadiy kutadi (masalan, `ls | wc`
     *  osilib qoladi). Shuning uchun ota ham har bir uchni darhol yopadi. */
    struct job *job = job_new(pl);
    if (!job) {
        fprintf(stderr, "sh: joblar juda ko'p\n");
        return 1;
    }
    int prev_read = -1;
    for (int i = 0; i < pl->n; i++) {
        struct cmd *c = &pl->cmds[i];
        int p[2] = { -1, -1 };
        if (i < pl->n - 1 && pipe(p) < 0) {
            perror("sh: pipe");
            break;
        }
        int pid = fork();
        if (pid < 0) {
            perror("sh: fork");
            if (p[0] >= 0) {
                close(p[0]);
                close(p[1]);
            }
            break;
        }
        if (pid == 0) {
            if (job_control) {
                /* Guruhga qo'shilish: birinchi bola - guruh yetakchisi (pgid = o'z pid'i).
                 * Ota ham xuddi shuni qiladi - kim birinchi ulgursa ham natija bir xil
                 * (poyga holati yo'q). */
                int pg = job->pgid ? job->pgid : getpid();
                setpgid(0, pg);
                if (!pl->background)
                    tcsetpgrp(STDIN_FILENO, pg);
                /* Shell e'tiborsiz qoldirgan signallar dasturda STANDART bo'lsin
                 * (exec SIG_IGN ni saqlab qoladi!). */
                signal(SIGINT, SIG_DFL);
                signal(SIGQUIT, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);
                signal(SIGTTIN, SIG_DFL);
                signal(SIGTTOU, SIG_DFL);
            }
            /* BOLA: stdin <- oldingi pipe, stdout -> keyingi pipe. */
            if (prev_read >= 0) {
                dup2(prev_read, STDIN_FILENO);
                close(prev_read);
            }
            if (p[1] >= 0) {
                dup2(p[1], STDOUT_FILENO);
                close(p[1]);
                close(p[0]);            /* o'qish uchi keyingi buyruqniki */
            }
            if (apply_redirs(c) < 0)    /* > f kabi yo'naltirishlar pipe'dan KEYIN (bash kabi) */
                _exit(1);
            if (c->argc == 0)
                _exit(0);
            exec_cmd(c);
        }
        /* OTA */
        if (job_control) {
            if (!job->pgid)
                job->pgid = pid;
            setpgid(pid, job->pgid);
        }
        if (prev_read >= 0)
            close(prev_read);
        if (p[1] >= 0)
            close(p[1]);
        prev_read = p[0];
        job->pids[job->npids++] = pid;
        job->alive++;
    }
    if (prev_read >= 0)
        close(prev_read);
    if (job->npids == 0) {
        job->id = 0;
        return 1;
    }
    if (pl->background) {
        printf("[%d] %d\n", job->id, job->pids[job->npids - 1]);
        return 0;
    }
    return job_foreground(job, false);
}

/* Bitta qatorni bajarish: pipeline'lar ; && || & bilan bog'langan. */
static void run_line(const char *line)
{
    static struct token toks[TOKENS_MAX];
    int ntok = lex(line, toks);
    if (ntok < 0) {
        last_status = 2;
        return;
    }
    int pos = 0;
    enum tok connector = T_SEMI;        /* oldingi pipeline qanday bog'langan */
    while (toks[pos].type != T_END) {
        static struct pipeline pl;
        if (parse_pipeline(toks, &pos, &pl) < 0) {
            free_pipeline(&pl);
            last_status = 2;
            break;
        }
        bool run = connector == T_SEMI || connector == T_AMP ||
                   (connector == T_AND && last_status == 0) ||
                   (connector == T_OR && last_status != 0);
        if (run && (pl.cmds[0].argc > 0 || pl.cmds[0].nredir > 0))
            last_status = run_pipeline(&pl);
        free_pipeline(&pl);
        connector = toks[pos].type;
        if (connector != T_END)
            pos++;
    }
    free_tokens(toks, ntok);
}

/* Fonda tugagan jarayonlarni "yig'ib olish" - aks holda ular zombie bo'lib qoladi. */
static void reap_background(void)
{
    int st, pid, k;
    while ((pid = waitpid(-1, &st, WNOHANG | WUNTRACED)) > 0) {
        struct job *j = job_by_pid(pid, &k);
        if (!j)
            continue;
        job_update(j, k, st);
        if (j->stopped) {
            printf("[%d]+  To'xtatildi       %s\n", j->id, j->cmd);
        } else if (j->alive == 0) {
            printf("[%d]   Tugadi (%d)       %s\n", j->id, status_to_code(j->status), j->cmd);
            j->id = 0;
        }
    }
}

/* stdin'dan bitta qator. BAYTMA-BAYT o'qiymiz: stdio buferi ishlatilsa, shell
 * keyingi qatorlarni ham "yutib" yuborishi mumkin, va keyin ishga tushgan
 * dastur (masalan, `cat` skript ichida) ularni ko'rmaydi. */
static int read_line(char *buf, size_t size)
{
    size_t len = 0;
    for (;;) {
        char c;
        ssize_t n = read(STDIN_FILENO, &c, 1);
        if (n < 0 && errno == EINTR)
            return -2;                  /* Ctrl-C: qator bekor, yangi so'rov */
        if (n <= 0)
            return len ? (int)len : -1; /* EOF (Ctrl-D) yoki xato */
        if (c == '\n')
            break;
        if (len < size - 1)
            buf[len++] = c;
    }
    buf[len] = '\0';
    return (int)len;
}

/* ============================================================================
 *  4. QATOR MUHARRIRI (interaktiv rejim) - GNU readline'ning kichik varianti
 * ============================================================================
 *  Terminal kanonik rejimda bo'lsa, qatorni YADRO tahrirlaydi (faqat
 *  backspace). Strelkalar, tarix, Tab uchun esa shell terminalni XOM rejimga
 *  o'tkazadi (ICANON va ECHO o'chiq) va har bir tugmani o'zi qayta ishlaydi,
 *  ekranni ham o'zi yangilaydi (ANSI escape ketma-ketliklari bilan).
 *  Buyruqni ishga tushirishdan OLDIN asl (kanonik) rejim tiklanadi: `cat`
 *  kabi dasturlar odatdagi terminalni kutadi. */

#define HISTORY_MAX 64

static char *history[HISTORY_MAX];
static int history_len;
static struct termios orig_tio;
static bool have_tio;

static void raw_mode(bool on)
{
    if (!have_tio)
        return;
    struct termios t = orig_tio;
    if (on)
        t.c_lflag &= ~(uint32_t)(ICANON | ECHO);    /* ISIG qoladi: Ctrl-C/Ctrl-Z ishlaydi */
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

static void history_add(const char *line)
{
    if (!*line || (history_len && strcmp(history[history_len - 1], line) == 0))
        return;                         /* bo'sh va takroriy qatorlar saqlanmaydi */
    if (history_len == HISTORY_MAX) {   /* eng eskisini chiqarib tashlaymiz */
        free(history[0]);
        memmove(history, history + 1, (HISTORY_MAX - 1) * sizeof(char *));
        history_len--;
    }
    history[history_len++] = strdup(line);
}

static void out(const char *s)
{
    write(STDOUT_FILENO, s, strlen(s));
}

/* Qatorni qayta chizish: qator boshi (\r), so'rov, matn, qolganini o'chirish
 * (ESC[K), keyin kursorni kerakli joyga qaytarish (ESC[nD - n ta chapga). */
static void refresh(const char *prompt, const char *buf, size_t len, size_t pos)
{
    char seq[32];
    out("\r");
    out(prompt);
    write(STDOUT_FILENO, buf, len);
    out("\033[K");
    if (len > pos) {
        snprintf(seq, sizeof(seq), "\033[%luD", len - pos);
        out(seq);
    }
}

/* ---- Tab bilan to'ldirish ---- */

#define COMPL_MAX 128

static int add_candidate(char **list, int n, const char *name)
{
    for (int i = 0; i < n; i++)
        if (strcmp(list[i], name) == 0)
            return n;                   /* takror (builtin va /bin da bir xil nom) */
    if (n < COMPL_MAX)
        list[n++] = strdup(name);
    return n;
}

/* buf[0..pos) dagi oxirgi so'zni to'ldirish. Qaytaradi: yangi pos.
 * Birinchi so'z - buyruq (/bin va ichki buyruqlar), qolganlari - fayl nomlari. */
static size_t complete(const char *prompt, char *buf, size_t *len, size_t pos, size_t size,
                       bool show_all)
{
    size_t start = pos;
    while (start > 0 && !strchr(" |;&<>", buf[start - 1]))
        start--;
    size_t ws = start;
    while (ws > 0 && buf[ws - 1] == ' ')
        ws--;
    bool first_word = ws == 0 || strchr("|;&", buf[ws - 1]);

    char word[PATH_MAX];
    size_t wlen = pos - start;
    if (wlen >= sizeof(word))
        return pos;
    memcpy(word, buf + start, wlen);
    word[wlen] = '\0';

    /* "papka/qism" -> papkada "qism" bilan boshlanadigan nomlar */
    char dir[PATH_MAX];
    const char *slash = strrchr(word, '/');
    const char *part = slash ? slash + 1 : word;
    if (slash)
        snprintf(dir, sizeof(dir), "%.*s", (int)(slash == word ? 1 : slash - word), word);
    else
        strcpy(dir, first_word ? "/bin" : ".");
    size_t plen = strlen(part);

    char *list[COMPL_MAX];
    int n = 0;
    if (first_word && !slash) {
        static const char *const builtins[] = { "cd", "pwd", "exit", "help", "jobs", "fg",
                                                "bg", "wait", "set", "source", NULL };
        for (int i = 0; builtins[i]; i++)
            if (strncmp(builtins[i], part, plen) == 0)
                n = add_candidate(list, n, builtins[i]);
    }
    DIR *d = opendir(dir);
    struct dirent *de;
    while (d && (de = readdir(d)) != NULL) {
        if (de->d_name[0] == '.' && part[0] != '.')
            continue;                   /* yashirin fayllar faqat "." yozilganda */
        if (strncmp(de->d_name, part, plen) != 0)
            continue;
        char name[NAME_MAX + 2];
        snprintf(name, sizeof(name), "%s%s", de->d_name, S_ISDIR(de->d_type) ? "/" : "");
        n = add_candidate(list, n, name);
    }
    if (d)
        closedir(d);
    if (n == 0)
        return pos;

    /* Barcha variantlarning umumiy boshlanishi - shuncha qismni yozib beramiz. */
    size_t common = strlen(list[0]);
    for (int i = 1; i < n; i++) {
        size_t k = 0;
        while (k < common && list[i][k] == list[0][k])
            k++;
        common = k;
    }
    if (n > 1 && common == plen && show_all) {  /* ikkinchi Tab: variantlar ro'yxati */
        out("\n");
        for (int i = 0; i < n; i++) {
            out(list[i]);
            out(i + 1 < n ? "  " : "\n");
        }
    }
    char add[NAME_MAX + 2];
    size_t alen = common - plen;
    memcpy(add, list[0] + plen, alen);
    if (n == 1 && (alen == 0 || add[alen - 1] != '/'))
        add[alen++] = ' ';              /* yagona fayl - keyingi argument uchun bo'shliq */
    if (*len + alen < size) {
        memmove(buf + pos + alen, buf + pos, *len - pos);
        memcpy(buf + pos, add, alen);
        *len += alen;
        pos += alen;
    }
    for (int i = 0; i < n; i++)
        free(list[i]);
    refresh(prompt, buf, *len, pos);
    return pos;
}

static void delete_at(char *buf, size_t *len, size_t at)
{
    memmove(buf + at, buf + at + 1, *len - at - 1);
    (*len)--;
}

/* Qatorni tahrirlab o'qish. Qaytaradi: uzunlik, -1 (EOF), -2 (Ctrl-C). */
static int edit_line(const char *prompt, char *buf, size_t size)
{
    size_t len = 0, pos = 0;
    int hist = history_len;             /* tarixdagi joriy o'rin (history_len = yangi qator) */
    char saved[LINE_MAX] = "";          /* tarixga chiqishdan oldin yozilayotgan qator */
    bool last_tab = false;
    int result;
    got_sigint = 0;
    raw_mode(true);
    out(prompt);
    for (;;) {
        char c;
        ssize_t n = read(STDIN_FILENO, &c, 1);
        /* Ctrl-C: SIGINT handler'imiz read'ni uzdi (EINTR) YOKI signal read
         * qaytgandan keyin keldi (bayroq) - ikkala holatda ham qator bekor. */
        if ((n < 0 && errno == EINTR) || got_sigint) {
            out("^C\n");
            result = -2;
            break;
        }
        if (n <= 0) {
            result = -1;
            break;
        }
        bool tab = false;
        if (c == '\n' || c == '\r') {
            out("\n");
            result = (int)len;
            break;
        } else if (c == 4) {            /* Ctrl-D: bo'sh qatorda - chiqish, aks holda Delete */
            if (len == 0) {
                result = -1;
                break;
            }
            if (pos < len)
                delete_at(buf, &len, pos);
        } else if (c == 0x7F || c == '\b') {
            if (pos > 0)
                delete_at(buf, &len, --pos);
        } else if (c == 1) {            /* Ctrl-A: qator boshi */
            pos = 0;
        } else if (c == 5) {            /* Ctrl-E: qator oxiri */
            pos = len;
        } else if (c == 21) {           /* Ctrl-U: kursorgacha o'chirish */
            memmove(buf, buf + pos, len - pos);
            len -= pos;
            pos = 0;
        } else if (c == 11) {           /* Ctrl-K: kursordan keyin o'chirish */
            len = pos;
        } else if (c == 23) {           /* Ctrl-W: oldingi so'zni o'chirish */
            size_t p = pos;
            while (p > 0 && buf[p - 1] == ' ')
                p--;
            while (p > 0 && buf[p - 1] != ' ')
                p--;
            memmove(buf + p, buf + pos, len - pos);
            len -= pos - p;
            pos = p;
        } else if (c == 12) {           /* Ctrl-L: ekranni tozalash */
            out("\033[2J\033[H");
        } else if (c == '\t') {
            tab = true;
            pos = complete(prompt, buf, &len, pos, size - 1, last_tab);
        } else if (c == 0x1B) {         /* escape ketma-ketligi: ESC [ harf  yoki  ESC [ raqam ~ */
            char s1, s2;
            if (read(STDIN_FILENO, &s1, 1) <= 0 || read(STDIN_FILENO, &s2, 1) <= 0)
                continue;
            if (s1 != '[' && s1 != 'O')
                continue;
            if (s2 >= '0' && s2 <= '9') {
                char t;
                if (read(STDIN_FILENO, &t, 1) <= 0)
                    continue;
                if (s2 == '3' && pos < len)
                    delete_at(buf, &len, pos);      /* Delete */
                else if (s2 == '1' || s2 == '7')
                    pos = 0;                        /* Home (boshqa terminallar varianti) */
                else if (s2 == '4' || s2 == '8')
                    pos = len;                      /* End */
            } else if (s2 == 'C' && pos < len) {
                pos++;
            } else if (s2 == 'D' && pos > 0) {
                pos--;
            } else if (s2 == 'H') {
                pos = 0;
            } else if (s2 == 'F') {
                pos = len;
            } else if ((s2 == 'A' && hist > 0) || (s2 == 'B' && hist < history_len)) {
                if (hist == history_len) {  /* yangi qatorni eslab qolamiz */
                    buf[len] = '\0';
                    strcpy(saved, buf);
                }
                hist += s2 == 'A' ? -1 : 1;
                const char *h = hist == history_len ? saved : history[hist];
                len = pos = strnlen(h, size - 1);
                memcpy(buf, h, len);
            }
        } else if ((unsigned char)c >= 32 && len < size - 1) {
            memmove(buf + pos + 1, buf + pos, len - pos);
            buf[pos++] = c;
            len++;
        }
        last_tab = tab;
        if (!tab)
            refresh(prompt, buf, len, pos);
    }
    raw_mode(false);                    /* buyruqlar odatdagi terminalni ko'rsin */
    buf[len] = '\0';
    if (result >= 0)
        history_add(buf);
    return result;
}

static void on_sigint(int sig)
{
    (void)sig;
    got_sigint = 1;                     /* handler'da faqat bayroq - xavfsiz */
}

static int run_script(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "sh: %s: %s\n", path, strerror(errno));
        return 127;
    }
    char line[LINE_MAX];
    while (fgets(line, sizeof(line), f))
        run_line(line);
    fclose(f);
    return last_status;
}

int main(int argc, char **argv)
{
    var_set("HOME", "/home");
    if (argc >= 3 && strcmp(argv[1], "-c") == 0) {
        run_line(argv[2]);
        return last_status;
    }
    if (argc >= 2)
        return run_script(argv[1]);

    bool interactive = isatty(STDIN_FILENO);
    if (interactive) {
        /* Job control: o'z guruhimiz, terminal bizniki. Shell Ctrl-Z / Ctrl-\\ dan
         * o'lmasligi kerak; Ctrl-C esa faqat terilayotgan qatorni bekor qiladi
         * (SA_RESTART YO'Q - read() -EINTR bilan qaytishi uchun). */
        job_control = true;
        setpgid(0, 0);
        shell_pgid = getpgrp();
        tcsetpgrp(STDIN_FILENO, shell_pgid);
        signal(SIGTSTP, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGQUIT, SIG_IGN);
        struct sigaction sa = { .sa_handler = on_sigint };
        sigaction(SIGINT, &sa, NULL);

        int fd = open("/etc/motd", O_RDONLY);  /* "message of the day" */
        if (fd >= 0) {
            char buf[1024];
            ssize_t n;
            while ((n = read(fd, buf, sizeof(buf))) > 0)
                write(STDOUT_FILENO, buf, (size_t)n);
            close(fd);
        }
    }
    char line[LINE_MAX];
    if (interactive)
        have_tio = tcgetattr(STDIN_FILENO, &orig_tio) == 0;
    for (;;) {
        reap_background();
        int n;
        if (interactive) {
            char cwd[PATH_MAX], prompt[PATH_MAX + 48];
            if (!getcwd(cwd, sizeof(cwd)))
                strcpy(cwd, "?");
            /* Yashil "myos", ko'k papka: ANSI SGR ranglari (1 - qalin, 32 - yashil, 34 - ko'k). */
            snprintf(prompt, sizeof(prompt), "\033[1;32mmyos\033[0m:\033[1;34m%s\033[0m$ ", cwd);
            fflush(stdout);
            if (have_tio) {
                n = edit_line(prompt, line, sizeof(line));
            } else {
                out(prompt);
                n = read_line(line, sizeof(line));
            }
        } else {
            n = read_line(line, sizeof(line));
        }
        if (n == -2)
            continue;
        if (n < 0)
            break;
        run_line(line);
    }
    if (interactive)
        puts("exit");
    return last_status;
}
