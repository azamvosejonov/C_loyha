/* =============================================================================
 *  user/bin/edit.c - MATN MUHARRIRI (antirez'ning "kilo" g'oyasi asosida)
 * =============================================================================
 *
 *  Ishlatish:  edit <fayl>
 *
 *      strelkalar, Home/End, PgUp/PgDn   - harakat
 *      Ctrl-S  - saqlash       Ctrl-Q - chiqish       Ctrl-F - qidirish
 *      Ctrl-G  - qatorga o'tish
 *
 *  QANDAY ISHLAYDI:
 *    1. Terminal XOM rejimga o'tadi (tcsetattr): har bir tugma darhol keladi,
 *       echo yo'q, Ctrl-C/Ctrl-Z signal emas - oddiy tugma (ISIG o'chiq).
 *    2. Fayl xotirada QATORLAR MASSIVI sifatida turadi: struct row { chars, len }.
 *       Qo'shish/o'chirish - qator ichida memmove.
 *    3. Har bir tugmadan keyin ekran QAYTA CHIZILADI: butun kadr bitta
 *       buferga yig'iladi (escape ketma-ketliklari bilan) va BITTA write() bilan
 *       yuboriladi - aks holda ekran "miltillaydi".
 *    4. Muqobil ekran (ESC[?1049h): chiqqanda shell ekrani qaytib keladi.
 *
 *  Muharrir faqat terminal tilini (VT100) biladi - xuddi shu dastur Linux
 *  terminalida ham, bizning framebuffer konsolimizda ham ishlaydi.
 * ============================================================================= */
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define TAB_STOP   4
#define QUIT_TIMES 3                    /* saqlanmagan o'zgarish bo'lsa - necha marta Ctrl-Q */
#define CTRL(k)    ((k) & 0x1F)

/* Maxsus tugmalar (1000 dan katta - oddiy belgilar bilan to'qnashmasin). */
enum key {
    BACKSPACE = 127,
    ARROW_LEFT = 1000, ARROW_RIGHT, ARROW_UP, ARROW_DOWN,
    DEL_KEY, HOME_KEY, END_KEY, PAGE_UP, PAGE_DOWN,
};

/* Sintaksis ranglari (har bir ko'rsatiladigan belgi uchun). */
enum hl { HL_NORMAL, HL_COMMENT, HL_KEYWORD, HL_TYPE, HL_STRING, HL_NUMBER, HL_MATCH, HL_PREPROC };

struct row {
    int len;
    char *chars;                        /* fayldagi haqiqiy matn */
    int rlen;
    char *render;                       /* ekrandagi ko'rinish (tab -> bo'shliqlar) */
    unsigned char *hl;                  /* render dagi har bir belgining rangi */
    bool open_comment;                  /* qator ochiq ko'p qatorli izoh ichida tugadimi */
};

static struct {
    int cx, cy;                         /* kursor: fayldagi (ustun, qator) */
    int rx;                             /* kursor render ichida (tab'lar hisobga olingan) */
    int rowoff, coloff;                 /* aylantirish: ekrandagi birinchi qator/ustun */
    int screenrows, screencols;
    int numrows;
    struct row *rows;
    int dirty;                          /* saqlanmagan o'zgarishlar soni */
    char *filename;
    bool c_syntax;                      /* .c / .h fayl - ranglar */
    char status[128];
    time_t status_time;
    struct termios orig;
} E;

/* ---- Terminal --------------------------------------------------------------- */

static void die(const char *msg)
{
    write(STDOUT_FILENO, "\033[?1049l", 8);
    tcsetattr(STDIN_FILENO, TCSANOW, &E.orig);
    perror(msg);
    exit(1);
}

static void raw_off(void)
{
    write(STDOUT_FILENO, "\033[?1049l", 8);    /* muqobil ekrandan chiqish */
    tcsetattr(STDIN_FILENO, TCSANOW, &E.orig);
}

static void raw_on(void)
{
    if (tcgetattr(STDIN_FILENO, &E.orig) < 0)
        die("edit: terminal emas");
    atexit(raw_off);
    struct termios t = E.orig;
    t.c_iflag &= ~(uint32_t)ICRNL;              /* Enter = '\r' (Ctrl-M dan ajratish uchun) */
    t.c_lflag &= ~(uint32_t)(ECHO | ICANON | ISIG);
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
    write(STDOUT_FILENO, "\033[?1049h", 8);    /* muqobil ekran */
}

/* Bitta tugmani o'qish; escape ketma-ketliklarini maxsus kodlarga aylantirish. */
static int read_key(void)
{
    char c;
    ssize_t n;
    while ((n = read(STDIN_FILENO, &c, 1)) != 1)
        if (n < 0 && errno != EINTR)
            die("read");
    if (c != '\033')
        return (unsigned char)c;
    char seq[3];
    if (read(STDIN_FILENO, &seq[0], 1) != 1 || read(STDIN_FILENO, &seq[1], 1) != 1)
        return '\033';
    if (seq[0] == '[') {
        if (seq[1] >= '0' && seq[1] <= '9') {
            if (read(STDIN_FILENO, &seq[2], 1) != 1 || seq[2] != '~')
                return '\033';
            switch (seq[1]) {
            case '1': case '7': return HOME_KEY;
            case '4': case '8': return END_KEY;
            case '3': return DEL_KEY;
            case '5': return PAGE_UP;
            case '6': return PAGE_DOWN;
            }
            return '\033';
        }
        switch (seq[1]) {
        case 'A': return ARROW_UP;
        case 'B': return ARROW_DOWN;
        case 'C': return ARROW_RIGHT;
        case 'D': return ARROW_LEFT;
        case 'H': return HOME_KEY;
        case 'F': return END_KEY;
        }
    } else if (seq[0] == 'O') {
        if (seq[1] == 'H')
            return HOME_KEY;
        if (seq[1] == 'F')
            return END_KEY;
    }
    return '\033';
}

static void window_size(void)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) < 0 || ws.ws_col == 0) {
        ws.ws_row = 24;
        ws.ws_col = 80;
    }
    E.screenrows = ws.ws_row - 2;       /* pastki 2 qator: holat va xabar satrlari */
    E.screencols = ws.ws_col;
}

/* ---- Sintaksis ranglari (C tili) -------------------------------------------- */

static const char *const keywords[] = {
    "if", "else", "for", "while", "do", "switch", "case", "default", "break", "continue",
    "return", "goto", "sizeof", "struct", "union", "enum", "typedef", "static", "extern",
    "const", "volatile", "inline", NULL,
};
static const char *const types[] = {
    "int", "char", "void", "long", "short", "unsigned", "signed", "bool", "float", "double",
    "size_t", "ssize_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t", "int8_t", "int16_t",
    "int32_t", "int64_t", "uintptr_t", NULL,
};

static bool is_separator(int c)
{
    return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];{}&|!^?:", c) != NULL;
}

static bool match_word(const char *s, const char *w, int left)
{
    int n = (int)strlen(w);
    return n <= left && strncmp(s, w, (size_t)n) == 0 && is_separator(s[n]);
}

/* Qatorni ranglash. prev_open - oldingi qator ochiq ko'p qatorli izoh bilan tugaganmi.
 * Qator holati o'zgarsa - keyingi qatorlar ham qayta ranglanadi. */
static void update_syntax(int at)
{
    struct row *r = &E.rows[at];
    r->hl = realloc(r->hl, (size_t)r->rlen + 1);
    memset(r->hl, HL_NORMAL, (size_t)r->rlen);
    if (!E.c_syntax)
        return;
    bool in_comment = at > 0 && E.rows[at - 1].open_comment;
    char in_string = 0;
    bool prev_sep = true;
    int i = 0;
    /* # bilan boshlanadigan qator - preprotsessor (#include, #define) */
    int first = 0;
    while (first < r->rlen && isspace((unsigned char)r->render[first]))
        first++;
    bool preproc = first < r->rlen && r->render[first] == '#';
    while (i < r->rlen) {
        char c = r->render[i];
        unsigned char prev_hl = i > 0 ? r->hl[i - 1] : HL_NORMAL;
        if (in_comment) {
            r->hl[i] = HL_COMMENT;
            if (c == '*' && i + 1 < r->rlen && r->render[i + 1] == '/') {
                r->hl[i + 1] = HL_COMMENT;
                i += 2;
                in_comment = false;
                prev_sep = true;
            } else {
                i++;
            }
            continue;
        }
        if (in_string) {
            r->hl[i] = HL_STRING;
            if (c == '\\' && i + 1 < r->rlen) {
                r->hl[i + 1] = HL_STRING;
                i += 2;
                continue;
            }
            if (c == in_string)
                in_string = 0;
            i++;
            prev_sep = true;
            continue;
        }
        if (c == '/' && i + 1 < r->rlen && r->render[i + 1] == '/') {
            memset(r->hl + i, HL_COMMENT, (size_t)(r->rlen - i));
            break;
        }
        if (c == '/' && i + 1 < r->rlen && r->render[i + 1] == '*') {
            r->hl[i] = r->hl[i + 1] = HL_COMMENT;
            i += 2;
            in_comment = true;
            continue;
        }
        if (c == '"' || c == '\'') {
            in_string = c;
            r->hl[i++] = HL_STRING;
            continue;
        }
        if (preproc) {
            r->hl[i++] = HL_PREPROC;
            continue;
        }
        if ((isdigit((unsigned char)c) && (prev_sep || prev_hl == HL_NUMBER)) ||
            ((c == '.' || c == 'x' || isxdigit((unsigned char)c)) && prev_hl == HL_NUMBER)) {
            r->hl[i++] = HL_NUMBER;
            prev_sep = false;
            continue;
        }
        if (prev_sep) {
            bool found = false;
            for (int k = 0; keywords[k] && !found; k++)
                if (match_word(&r->render[i], keywords[k], r->rlen - i)) {
                    int n = (int)strlen(keywords[k]);
                    memset(r->hl + i, HL_KEYWORD, (size_t)n);
                    i += n;
                    found = true;
                }
            for (int k = 0; types[k] && !found; k++)
                if (match_word(&r->render[i], types[k], r->rlen - i)) {
                    int n = (int)strlen(types[k]);
                    memset(r->hl + i, HL_TYPE, (size_t)n);
                    i += n;
                    found = true;
                }
            if (found) {
                prev_sep = false;
                continue;
            }
        }
        prev_sep = is_separator((unsigned char)c);
        i++;
    }
    bool changed = r->open_comment != in_comment;
    r->open_comment = in_comment;
    if (changed && at + 1 < E.numrows)
        update_syntax(at + 1);          /* ko'p qatorli izoh ochildi yoki yopildi - keyingi qatorlarga ta'sir */
}

/* Rang -> ANSI rang kodi. 90-97 - YORQIN ranglar (qora fonda yaxshi o'qiladi). */
static int hl_color(unsigned char hl)
{
    switch (hl) {
    case HL_COMMENT: return 96;         /* moviy */
    case HL_KEYWORD: return 93;         /* sariq */
    case HL_TYPE:    return 92;         /* yashil */
    case HL_STRING:  return 95;         /* binafsha */
    case HL_NUMBER:  return 91;         /* qizil */
    case HL_PREPROC: return 94;         /* ko'k */
    default:         return 39;         /* standart */
    }
}

/* ---- Qatorlar --------------------------------------------------------------- */

/* Fayldagi ustun (cx) -> ekrandagi ustun (rx): tab bir necha katak egallaydi. */
static int cx_to_rx(const struct row *r, int cx)
{
    int rx = 0;
    for (int j = 0; j < cx; j++) {
        if (r->chars[j] == '\t')
            rx += (TAB_STOP - 1) - (rx % TAB_STOP);
        rx++;
    }
    return rx;
}

static int rx_to_cx(const struct row *r, int rx)
{
    int cur = 0, cx;
    for (cx = 0; cx < r->len; cx++) {
        if (r->chars[cx] == '\t')
            cur += (TAB_STOP - 1) - (cur % TAB_STOP);
        cur++;
        if (cur > rx)
            return cx;
    }
    return cx;
}

static void update_row(int at)
{
    struct row *r = &E.rows[at];
    int tabs = 0;
    for (int j = 0; j < r->len; j++)
        tabs += r->chars[j] == '\t';
    free(r->render);
    r->render = malloc((size_t)r->len + (size_t)tabs * (TAB_STOP - 1) + 1);
    int idx = 0;
    for (int j = 0; j < r->len; j++) {
        if (r->chars[j] == '\t') {
            r->render[idx++] = ' ';
            while (idx % TAB_STOP)
                r->render[idx++] = ' ';
        } else {
            r->render[idx++] = r->chars[j];
        }
    }
    r->render[idx] = '\0';
    r->rlen = idx;
    update_syntax(at);
}

static void insert_row(int at, const char *s, size_t len)
{
    if (at < 0 || at > E.numrows)
        return;
    E.rows = realloc(E.rows, sizeof(struct row) * (size_t)(E.numrows + 1));
    memmove(&E.rows[at + 1], &E.rows[at], sizeof(struct row) * (size_t)(E.numrows - at));
    struct row *r = &E.rows[at];
    r->len = (int)len;
    r->chars = malloc(len + 1);
    memcpy(r->chars, s, len);
    r->chars[len] = '\0';
    r->rlen = 0;
    r->render = NULL;
    r->hl = NULL;
    r->open_comment = false;
    E.numrows++;
    update_row(at);
    E.dirty++;
}

static void delete_row(int at)
{
    if (at < 0 || at >= E.numrows)
        return;
    struct row *r = &E.rows[at];
    free(r->chars);
    free(r->render);
    free(r->hl);
    memmove(&E.rows[at], &E.rows[at + 1], sizeof(struct row) * (size_t)(E.numrows - at - 1));
    E.numrows--;
    if (at < E.numrows)
        update_syntax(at);
    E.dirty++;
}

static void row_insert_char(struct row *r, int at, int c)
{
    if (at < 0 || at > r->len)
        at = r->len;
    r->chars = realloc(r->chars, (size_t)r->len + 2);
    memmove(&r->chars[at + 1], &r->chars[at], (size_t)(r->len - at + 1));
    r->len++;
    r->chars[at] = (char)c;
}

static void row_append(struct row *r, const char *s, size_t len)
{
    r->chars = realloc(r->chars, (size_t)r->len + len + 1);
    memcpy(&r->chars[r->len], s, len);
    r->len += (int)len;
    r->chars[r->len] = '\0';
}

static void row_delete_char(struct row *r, int at)
{
    if (at < 0 || at >= r->len)
        return;
    memmove(&r->chars[at], &r->chars[at + 1], (size_t)(r->len - at));
    r->len--;
}

/* ---- Tahrir amallari -------------------------------------------------------- */

static void insert_char(int c)
{
    if (E.cy == E.numrows)
        insert_row(E.numrows, "", 0);
    row_insert_char(&E.rows[E.cy], E.cx, c);
    update_row(E.cy);
    E.cx++;
    E.dirty++;
}

/* Enter: qatorni kursor joyida ikkiga bo'lish (chekinish saqlanadi). */
static void insert_newline(void)
{
    if (E.cx == 0 || E.cy == E.numrows) {
        insert_row(E.cy, "", 0);
        E.cy++;
        E.cx = 0;
        return;
    }
    struct row *r = &E.rows[E.cy];
    int indent = 0;                     /* avtomatik chekinish: joriy qatorning boshidagi bo'shliqlar */
    while (indent < r->len && indent < E.cx && (r->chars[indent] == ' ' || r->chars[indent] == '\t'))
        indent++;
    size_t tail = (size_t)(r->len - E.cx);
    char *buf = malloc((size_t)indent + tail + 1);
    memcpy(buf, r->chars, (size_t)indent);
    memcpy(buf + indent, &r->chars[E.cx], tail);
    insert_row(E.cy + 1, buf, (size_t)indent + tail);
    free(buf);
    r = &E.rows[E.cy];                  /* realloc tufayli ko'rsatkich o'zgargan bo'lishi mumkin! */
    r->len = E.cx;
    r->chars[r->len] = '\0';
    update_row(E.cy);
    E.cy++;
    E.cx = indent;
}

static void delete_char(void)
{
    if (E.cy == E.numrows || (E.cx == 0 && E.cy == 0))
        return;
    struct row *r = &E.rows[E.cy];
    if (E.cx > 0) {
        row_delete_char(r, E.cx - 1);
        update_row(E.cy);
        E.cx--;
    } else {                            /* qator boshida: oldingi qatorga qo'shib yuborish */
        E.cx = E.rows[E.cy - 1].len;
        row_append(&E.rows[E.cy - 1], r->chars, (size_t)r->len);
        update_row(E.cy - 1);
        delete_row(E.cy);
        E.cy--;
    }
    E.dirty++;
}

/* ---- Fayl ------------------------------------------------------------------- */

static void set_status(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(E.status, sizeof(E.status), fmt, ap);
    va_end(ap);
    E.status_time = time(NULL);
}

static void open_file(const char *name)
{
    free(E.filename);
    E.filename = strdup(name);
    const char *ext = strrchr(name, '.');
    E.c_syntax = ext && (strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0);
    FILE *f = fopen(name, "r");
    if (!f) {
        if (errno != ENOENT)
            die(name);
        set_status("Yangi fayl: %s", name);
        return;
    }
    char line[4096];
    while (fgets(line, sizeof(line), f)) {
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
            len--;
        insert_row(E.numrows, line, len);
    }
    fclose(f);
    E.dirty = 0;
}

static void save_file(void)
{
    if (!E.filename)
        return;
    /* Hammasini bitta buferga yig'amiz: har bir qator + '\n'. */
    size_t total = 0;
    for (int i = 0; i < E.numrows; i++)
        total += (size_t)E.rows[i].len + 1;
    char *buf = malloc(total ? total : 1), *p = buf;
    for (int i = 0; i < E.numrows; i++) {
        memcpy(p, E.rows[i].chars, (size_t)E.rows[i].len);
        p += E.rows[i].len;
        *p++ = '\n';
    }
    int fd = open(E.filename, O_RDWR | O_CREAT, 0644);
    /* Avval yozib, KEYIN qisqartiramiz: yozish muvaffaqiyatsiz bo'lsa, eski
     * mazmun kamida qisman saqlanib qoladi (O_TRUNC darhol o'chirib yuborardi). */
    if (fd >= 0 && (size_t)write(fd, buf, total) == total && ftruncate(fd, (off_t)total) == 0) {
        close(fd);
        free(buf);
        E.dirty = 0;
        set_status("%lu bayt yozildi: %s", total, E.filename);
        return;
    }
    if (fd >= 0)
        close(fd);
    free(buf);
    set_status("Saqlab bo'lmadi: %s", strerror(errno));
}

/* ---- Ekranni chizish -------------------------------------------------------- */

struct abuf {                           /* "append buffer": butun kadr shu yerda yig'iladi */
    char *b;
    size_t len, cap;
};

static void ab_add(struct abuf *ab, const char *s, size_t len)
{
    if (ab->len + len > ab->cap) {
        ab->cap = (ab->len + len) * 2;
        ab->b = realloc(ab->b, ab->cap);
    }
    memcpy(ab->b + ab->len, s, len);
    ab->len += len;
}

static void ab_str(struct abuf *ab, const char *s)
{
    ab_add(ab, s, strlen(s));
}

static void scroll(void)
{
    E.rx = E.cy < E.numrows ? cx_to_rx(&E.rows[E.cy], E.cx) : 0;
    if (E.cy < E.rowoff)
        E.rowoff = E.cy;
    if (E.cy >= E.rowoff + E.screenrows)
        E.rowoff = E.cy - E.screenrows + 1;
    int text_cols = E.screencols - 5;   /* 5 ustun - qator raqamlari */
    if (E.rx < E.coloff)
        E.coloff = E.rx;
    if (E.rx >= E.coloff + text_cols)
        E.coloff = E.rx - text_cols + 1;
}

static void draw_rows(struct abuf *ab)
{
    int text_cols = E.screencols - 5;
    for (int y = 0; y < E.screenrows; y++) {
        int filerow = y + E.rowoff;
        if (filerow >= E.numrows) {
            if (E.numrows == 0 && y == E.screenrows / 3) {
                char w[80];
                int n = snprintf(w, sizeof(w), "MyOS muharriri -- Ctrl-S saqlash, Ctrl-Q chiqish");
                int pad = (E.screencols - n) / 2;
                ab_str(ab, "\033[90m~");
                for (int i = 1; i < pad; i++)
                    ab_str(ab, " ");
                ab_add(ab, w, (size_t)n);
                ab_str(ab, "\033[0m");
            } else {
                ab_str(ab, "\033[90m~\033[0m");     /* fayl oxiridan keyin (vi an'anasi) */
            }
        } else {
            char num[16];
            snprintf(num, sizeof(num), "\033[90m%4d\033[0m ", filerow + 1);
            ab_str(ab, num);
            struct row *r = &E.rows[filerow];
            int len = r->rlen - E.coloff;
            if (len < 0)
                len = 0;
            if (len > text_cols)
                len = text_cols;
            const char *s = &r->render[E.coloff];
            const unsigned char *hl = &r->hl[E.coloff];
            int cur = -1;
            for (int j = 0; j < len; j++) {
                if (hl[j] == HL_NORMAL) {
                    if (cur != -1) {
                        ab_str(ab, "\033[39m");
                        cur = -1;
                    }
                } else if (hl[j] != cur) {
                    char seq[16];
                    /* Qidiruv mosligi - teskari rang (7), qolganlari - o'z rangi. */
                    snprintf(seq, sizeof(seq), "\033[%dm", hl[j] == HL_MATCH ? 7 : hl_color(hl[j]));
                    ab_str(ab, seq);
                    cur = hl[j];
                }
                if (hl[j] == HL_MATCH) {
                    ab_add(ab, &s[j], 1);
                    if (j + 1 >= len || hl[j + 1] != HL_MATCH) {
                        ab_str(ab, "\033[27m");
                        cur = -1;
                    }
                } else {
                    ab_add(ab, &s[j], 1);
                }
            }
            ab_str(ab, "\033[39m");
        }
        ab_str(ab, "\033[K\r\n");        /* qatorning qolgan qismini tozalash */
    }
}

static void draw_status(struct abuf *ab)
{
    char left[128], right[64];
    ab_str(ab, "\033[7m");              /* teskari ranglar: oq fon */
    int l = snprintf(left, sizeof(left), " %.40s%s - %d qator",
                     E.filename ? E.filename : "[nomsiz]", E.dirty ? " (o'zgargan)" : "",
                     E.numrows);
    int r = snprintf(right, sizeof(right), "%s | %d:%d ", E.c_syntax ? "C" : "matn", E.cy + 1,
                     E.rx + 1);
    if (l > E.screencols)
        l = E.screencols;
    ab_add(ab, left, (size_t)l);
    while (l < E.screencols) {
        if (E.screencols - l == r) {
            ab_add(ab, right, (size_t)r);
            break;
        }
        ab_str(ab, " ");
        l++;
    }
    ab_str(ab, "\033[m\r\n\033[K");
    if (E.status[0] && time(NULL) - E.status_time < 5)
        ab_add(ab, E.status, strnlen(E.status, (size_t)E.screencols));
}

static void refresh_screen(void)
{
    scroll();
    struct abuf ab = { 0 };
    ab_str(&ab, "\033[?25l\033[H");     /* kursorni yashirib, chap yuqoriga */
    draw_rows(&ab);
    draw_status(&ab);
    char pos[32];
    snprintf(pos, sizeof(pos), "\033[%d;%dH", E.cy - E.rowoff + 1, E.rx - E.coloff + 6);
    ab_str(&ab, pos);
    ab_str(&ab, "\033[?25h");
    write(STDOUT_FILENO, ab.b, ab.len);
    free(ab.b);
}

/* ---- Pastki satrda so'rov (qidirish, qatorga o'tish) ------------------------ */

typedef void (*prompt_cb)(const char *buf, int key);

static char *prompt(const char *fmt, prompt_cb cb)
{
    size_t cap = 128, len = 0;
    char *buf = malloc(cap);
    buf[0] = '\0';
    for (;;) {
        set_status(fmt, buf);
        refresh_screen();
        int c = read_key();
        if (c == DEL_KEY || c == CTRL('h') || c == BACKSPACE) {
            if (len)
                buf[--len] = '\0';
        } else if (c == '\033') {
            set_status("");
            if (cb)
                cb(buf, c);
            free(buf);
            return NULL;
        } else if (c == '\r' || c == '\n') {
            if (len) {
                set_status("");
                if (cb)
                    cb(buf, c);
                return buf;
            }
        } else if (c < 128 && isprint(c)) {
            if (len == cap - 1)
                buf = realloc(buf, cap *= 2);
            buf[len++] = (char)c;
            buf[len] = '\0';
        }
        if (cb)
            cb(buf, c);
    }
}

/* Qidiruv: har bir tugmada yangilanadi; strelkalar - keyingi/oldingi moslik. */
static void find_cb(const char *query, int key)
{
    static int last_match = -1, direction = 1;
    static int saved_line = -1;
    static unsigned char *saved_hl;
    if (saved_hl) {                     /* oldingi moslik ranglarini tiklash */
        memcpy(E.rows[saved_line].hl, saved_hl, (size_t)E.rows[saved_line].rlen);
        free(saved_hl);
        saved_hl = NULL;
    }
    if (key == '\r' || key == '\n' || key == '\033') {
        last_match = -1;
        direction = 1;
        return;
    }
    if (key == ARROW_RIGHT || key == ARROW_DOWN) {
        direction = 1;
    } else if (key == ARROW_LEFT || key == ARROW_UP) {
        direction = -1;
    } else {
        last_match = -1;
        direction = 1;
    }
    if (last_match == -1)
        direction = 1;
    int cur = last_match;
    for (int i = 0; i < E.numrows; i++) {
        cur += direction;
        if (cur == -1)
            cur = E.numrows - 1;
        else if (cur == E.numrows)
            cur = 0;
        struct row *r = &E.rows[cur];
        char *m = strstr(r->render, query);
        if (m) {
            last_match = cur;
            E.cy = cur;
            E.cx = rx_to_cx(r, (int)(m - r->render));
            E.rowoff = E.numrows;       /* scroll() mos qatorni ekran tepasiga olib chiqadi */
            saved_line = cur;
            saved_hl = malloc((size_t)r->rlen);
            memcpy(saved_hl, r->hl, (size_t)r->rlen);
            memset(&r->hl[m - r->render], HL_MATCH, strlen(query));
            break;
        }
    }
}

static void find(void)
{
    int cx = E.cx, cy = E.cy, coloff = E.coloff, rowoff = E.rowoff;
    char *q = prompt("Qidirish: %s  (Esc - bekor, strelkalar - keyingi)", find_cb);
    if (q) {
        free(q);
    } else {                            /* bekor qilindi - joyimizga qaytamiz */
        E.cx = cx, E.cy = cy, E.coloff = coloff, E.rowoff = rowoff;
    }
}

static void goto_line(void)
{
    char *q = prompt("Qator raqami: %s", NULL);
    if (!q)
        return;
    int n = atoi(q);
    free(q);
    if (n >= 1 && n <= E.numrows) {
        E.cy = n - 1;
        E.cx = 0;
    }
}

/* ---- Tugmalar --------------------------------------------------------------- */

static void move_cursor(int key)
{
    struct row *r = E.cy < E.numrows ? &E.rows[E.cy] : NULL;
    switch (key) {
    case ARROW_LEFT:
        if (E.cx > 0) {
            E.cx--;
        } else if (E.cy > 0) {          /* qator boshidan chapga - oldingi qator oxiriga */
            E.cy--;
            E.cx = E.rows[E.cy].len;
        }
        break;
    case ARROW_RIGHT:
        if (r && E.cx < r->len) {
            E.cx++;
        } else if (r && E.cy < E.numrows - 1) {
            E.cy++;
            E.cx = 0;
        }
        break;
    case ARROW_UP:
        if (E.cy > 0)
            E.cy--;
        break;
    case ARROW_DOWN:
        if (E.cy < E.numrows - 1)
            E.cy++;
        break;
    }
    r = E.cy < E.numrows ? &E.rows[E.cy] : NULL;
    int len = r ? r->len : 0;
    if (E.cx > len)                     /* qisqa qatorga o'tganda kursor oxirga */
        E.cx = len;
}

static void process_key(void)
{
    static int quit_times = QUIT_TIMES;
    int c = read_key();
    switch (c) {
    case '\r':
    case '\n':                          /* PS/2 drayverimiz Enter uchun '\n', serial terminal '\r' */
        insert_newline();
        break;
    case CTRL('q'):
        if (E.dirty && quit_times > 0) {
            set_status("DIQQAT: saqlanmagan o'zgarishlar! Chiqish uchun Ctrl-Q ni yana %d marta bosing",
                       quit_times);
            quit_times--;
            return;
        }
        write(STDOUT_FILENO, "\033[2J\033[H", 7);
        exit(0);
    case CTRL('s'):
        save_file();
        break;
    case CTRL('f'):
        find();
        break;
    case CTRL('g'):
        goto_line();
        break;
    case HOME_KEY:
        E.cx = 0;
        break;
    case END_KEY:
        if (E.cy < E.numrows)
            E.cx = E.rows[E.cy].len;
        break;
    case BACKSPACE:
    case CTRL('h'):
        delete_char();
        break;
    case DEL_KEY:
        move_cursor(ARROW_RIGHT);
        delete_char();
        break;
    case PAGE_UP:
    case PAGE_DOWN:
        if (c == PAGE_UP) {
            E.cy = E.rowoff;
        } else {
            E.cy = E.rowoff + E.screenrows - 1;
            if (E.cy > E.numrows)
                E.cy = E.numrows;
        }
        for (int n = E.screenrows; n--;)
            move_cursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
        break;
    case ARROW_UP:
    case ARROW_DOWN:
    case ARROW_LEFT:
    case ARROW_RIGHT:
        move_cursor(c);
        break;
    case CTRL('l'):
    case '\033':
        break;
    default:
        if (c == '\t' || (c >= 32 && c < 256))
            insert_char(c);
        break;
    }
    quit_times = QUIT_TIMES;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "ishlatish: edit <fayl>\n");
        return 1;
    }
    if (!isatty(STDIN_FILENO)) {
        fprintf(stderr, "edit: terminal kerak\n");
        return 1;
    }
    raw_on();
    window_size();
    open_file(argv[1]);
    if (!E.status[0])
        set_status("Ctrl-S saqlash | Ctrl-Q chiqish | Ctrl-F qidirish | Ctrl-G qatorga o'tish");
    for (;;) {
        refresh_screen();
        process_key();
    }
}
