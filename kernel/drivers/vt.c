/* =============================================================================
 *  drivers/vt.c - TERMINAL EMULYATORI (VT100 / xterm'ning asosiy qismi)
 * =============================================================================
 *
 *  Dasturlar ekranni to'g'ridan-to'g'ri boshqarmaydi - ular faqat BAYTLAR
 *  yozadi. Kursorni siljitish, rang, tozalash - bular "escape ketma-ketliklari":
 *
 *      "\033[2J"        - butun ekranni tozalash
 *      "\033[5;10H"     - kursorni 5-qator, 10-ustunga
 *      "\033[1;31m"     - qalin qizil matn
 *      "\033[K"         - qatorning qolganini o'chirish
 *
 *  Bu til 1978 yildagi DEC VT100 terminalidan qolgan va hamon standart
 *  (ECMA-48). Linux konsoli, xterm, Windows Terminal, macOS Terminal -
 *  hammasi uni tushunadi. Shuning uchun `vi`, `top`, `less` har qanday
 *  terminalda ishlaydi. Bizning muharrir (`edit`) ham shu tilda "gapiradi" va
 *  xuddi shunday serial port orqali Linux terminalida ham ishlaydi.
 *
 *  ARXITEKTURA:
 *      console_write ──► vt_putc: UTF-8 dekoder + escape parser (holat mashinasi)
 *                          │  katakchalar buferi (belgi + rang) - "haqiqat manbai"
 *                          ▼
 *                    screen_ops->draw(x, y, belgi, rang)   (fbcon: piksellar, vga: 0xB8000)
 *
 *  Ekran drayverlari faqat "shu katakni chiz" ni biladi. Terminal mantig'i
 *  bitta joyda - Linux'da ham vt.c + fbcon/vgacon shunday bo'lingan.
 *
 *  PARSER HOLATLARI:
 *      NORMAL ─ESC─► ESC ─'['─► CSI (parametrlar: "5;10") ─yakuniy harf─► bajarish
 *                        └─']'─► OSC (oyna nomi va h.k. - e'tiborsiz, BEL gacha)
 * ============================================================================= */
#include "drivers/vt.h"

#include "drivers/console.h"
#include "lib/common.h"
#include "lib/kprintf.h"
#include "lib/string.h"
#include "mm/slab.h"

#define MAX_PARAMS 16

struct vt_cell {
    uint16_t cp;                        /* Unicode belgi (BMP) */
    uint8_t attr;                       /* past 4 bit - harf rangi, yuqori 4 - fon */
    uint8_t pad;
};

enum state { S_NORMAL, S_ESC, S_CSI, S_OSC, S_OSC_ESC };

static const struct screen_ops *scr;
static struct vt_cell *cells, *alt_cells;   /* asosiy va muqobil (alternate) ekran */
static bool alt_active;
static unsigned cols, rows;
static unsigned cx, cy;                     /* kursor */
static bool wrap_pending;                   /* oxirgi ustunga yozildi - keyingi belgi yangi qatorda */
static unsigned saved_x, saved_y, main_x, main_y;
static bool cursor_visible = true;
static unsigned top, bottom;                /* aylantirish (scroll) hududi: [top, bottom] */

/* Joriy atributlar (SGR) */
static uint8_t fg = 7, bg = 0;
static bool bold, reverse;

/* Parser */
static enum state st;
static unsigned params[MAX_PARAMS];
static unsigned nparams;
static bool param_started;
static char priv;                           /* '?' - DEC maxsus rejimlari */

/* UTF-8 */
static uint32_t utf_cp;
static int utf_need;

/* Terminal javoblari (DSR) */
static char response[32];
static size_t response_len;

static uint8_t cur_attr(void)
{
    uint8_t f = fg, b = bg;
    if (bold && f < 8)
        f += 8;                             /* qalin = yorqin rang (klassik konsol) */
    if (reverse) {
        uint8_t t = f;
        f = b;
        b = t;
    }
    return (uint8_t)(f | (b << 4));
}

/* ---- Chizish ---------------------------------------------------------------- */

static void draw(unsigned x, unsigned y)
{
    const struct vt_cell *c = &cells[y * cols + x];
    scr->draw(x, y, c->cp, c->attr);
}

/* Kursor: apparat kursori bo'lsa (VGA) - uni siljitamiz, bo'lmasa (framebuffer)
 * kursor turgan katakni teskari rangda chizamiz. Har bir vt_putc boshida
 * yashiriladi va oxirida qayta ko'rsatiladi. */
static void show_cursor(void)
{
    unsigned x = cx < cols ? cx : cols - 1;
    if (scr->cursor) {
        scr->cursor(x, cy, cursor_visible);
    } else if (cursor_visible) {
        const struct vt_cell *c = &cells[cy * cols + x];
        scr->draw(x, cy, c->cp, (uint8_t)((c->attr >> 4) | (c->attr << 4)));
    }
}

static void hide_cursor(void)
{
    if (!scr->cursor && cursor_visible)
        draw(cx < cols ? cx : cols - 1, cy);
}

static void redraw(void)
{
    for (unsigned y = 0; y < rows; y++)
        for (unsigned x = 0; x < cols; x++)
            draw(x, y);
}

static void clear_cells(unsigned from, unsigned to)     /* [from, to) indekslar */
{
    /* O'chirilgan joy JORIY fon rangini oladi (xterm "background color erase"). */
    struct vt_cell blank = { ' ', (uint8_t)((bg << 4) | fg), 0 };
    for (unsigned i = from; i < to; i++) {
        cells[i] = blank;
        scr->draw(i % cols, i / cols, ' ', blank.attr);
    }
}

/* [t, b] hududini n qator yuqoriga surish (pastdan bo'sh qatorlar). */
static void scroll_up(unsigned t, unsigned b, unsigned n)
{
    if (n > b - t + 1)
        n = b - t + 1;
    memmove(&cells[t * cols], &cells[(t + n) * cols], (size_t)(b + 1 - t - n) * cols * sizeof(*cells));
    for (unsigned y = t; y <= b - n; y++)
        for (unsigned x = 0; x < cols; x++)
            draw(x, y);
    clear_cells((b + 1 - n) * cols, (b + 1) * cols);
}

/* [t, b] hududini n qator pastga surish (tepadan bo'sh qatorlar). */
static void scroll_down(unsigned t, unsigned b, unsigned n)
{
    if (n > b - t + 1)
        n = b - t + 1;
    memmove(&cells[(t + n) * cols], &cells[t * cols], (size_t)(b + 1 - t - n) * cols * sizeof(*cells));
    for (unsigned y = t + n; y <= b; y++)
        for (unsigned x = 0; x < cols; x++)
            draw(x, y);
    clear_cells(t * cols, (t + n) * cols);
}

static void linefeed(void)
{
    if (cy == bottom)
        scroll_up(top, bottom, 1);
    else if (cy < rows - 1)
        cy++;
}

/* ---- Belgi chiqarish -------------------------------------------------------- */

static void put_char(uint32_t cp)
{
    if (wrap_pending) {                 /* xterm qoidasi: oxirgi ustundan keyin darhol */
        wrap_pending = false;           /* o'tmaymiz - faqat YANGI belgi kelganda */
        cx = 0;
        linefeed();
    }
    cells[cy * cols + cx] = (struct vt_cell){ (uint16_t)(cp > 0xFFFF ? '?' : cp), cur_attr(), 0 };
    scr->draw(cx, cy, cells[cy * cols + cx].cp, cells[cy * cols + cx].attr);
    if (cx == cols - 1)
        wrap_pending = true;            /* aks holda to'liq kenglikdagi qator bo'sh qator qo'shardi */
    else
        cx++;
}

/* ---- CSI buyruqlari --------------------------------------------------------- */

static unsigned param(unsigned i, unsigned def)
{
    return (i < nparams && params[i]) ? params[i] : def;
}

static void set_cursor(unsigned x, unsigned y)
{
    cx = MIN(x, cols - 1);
    cy = MIN(y, rows - 1);
    wrap_pending = false;
}

static const uint8_t ansi_to_vga[8] = { 0, 4, 2, 6, 1, 5, 3, 7 };  /* ANSI tartibi -> VGA palitra */

static void sgr(void)
{
    if (nparams == 0)
        nparams = 1, params[0] = 0;
    for (unsigned i = 0; i < nparams; i++) {
        unsigned p = params[i];
        if (p == 0) {
            fg = 7, bg = 0, bold = reverse = false;
        } else if (p == 1) {
            bold = true;
        } else if (p == 22) {
            bold = false;
        } else if (p == 7) {
            reverse = true;
        } else if (p == 27) {
            reverse = false;
        } else if (p >= 30 && p <= 37) {
            fg = ansi_to_vga[p - 30];
        } else if (p == 39) {
            fg = 7;
        } else if (p >= 40 && p <= 47) {
            bg = ansi_to_vga[p - 40];
        } else if (p == 49) {
            bg = 0;
        } else if (p >= 90 && p <= 97) {
            fg = ansi_to_vga[p - 90] + 8;
        } else if (p >= 100 && p <= 107) {
            bg = ansi_to_vga[p - 100] + 8;
        } else if ((p == 38 || p == 48) && i + 1 < nparams) {
            /* 256 rang (5;n) yoki RGB (2;r;g;b): 16 rangga yaqinlashtiramiz. */
            unsigned col = 7;
            if (params[i + 1] == 5 && i + 2 < nparams) {
                unsigned n = params[i + 2];
                col = n < 8 ? ansi_to_vga[n] : n < 16 ? ansi_to_vga[n - 8] + 8u : 7u;
                i += 2;
            } else if (params[i + 1] == 2 && i + 4 < nparams) {
                unsigned r = params[i + 2], g = params[i + 3], b = params[i + 4];
                col = (r > 127 ? 4u : 0u) | (g > 127 ? 2u : 0u) | (b > 127 ? 1u : 0u);
                col = ansi_to_vga[col] + ((r + g + b) > 500 ? 8u : 0u);
                i += 4;
            }
            if (p == 38)
                fg = (uint8_t)col;
            else
                bg = (uint8_t)col;
        }
        /* 4 (tagiga chizish), 5 (miltillash) va boshqalar - e'tiborsiz */
    }
}

static void set_alt_screen(bool on)
{
    if (on == alt_active || !alt_cells)
        return;
    if (on) {                           /* muharrir ochildi: asosiy ekranni saqlaymiz */
        memcpy(alt_cells, cells, (size_t)cols * rows * sizeof(*cells));
        main_x = cx, main_y = cy;
        clear_cells(0, cols * rows);
    } else {                            /* muharrir yopildi: shell ekrani qaytadi */
        memcpy(cells, alt_cells, (size_t)cols * rows * sizeof(*cells));
        set_cursor(main_x, main_y);
        redraw();
    }
    alt_active = on;
}

static void csi_dispatch(char f)
{
    unsigned n = param(0, 1);
    if (priv == '?') {
        if (f == 'h' || f == 'l') {
            for (unsigned i = 0; i < nparams; i++) {
                if (params[i] == 25)
                    cursor_visible = f == 'h';
                else if (params[i] == 1049 || params[i] == 47 || params[i] == 1047)
                    set_alt_screen(f == 'h');
            }
        }
        return;
    }
    switch (f) {
    case 'A': set_cursor(cx, cy >= n ? cy - n : 0); break;
    case 'B': set_cursor(cx, cy + n); break;
    case 'C': set_cursor(cx + n, cy); break;
    case 'D': set_cursor(cx >= n ? cx - n : 0, cy); break;
    case 'E': set_cursor(0, cy + n); break;
    case 'F': set_cursor(0, cy >= n ? cy - n : 0); break;
    case 'G': set_cursor(n - 1, cy); break;
    case 'd': set_cursor(cx, n - 1); break;
    case 'H':
    case 'f': set_cursor(param(1, 1) - 1, param(0, 1) - 1); break;
    case 'J': {
        unsigned mode = param(0, 0), pos = cy * cols + cx;
        if (mode == 0)
            clear_cells(pos, cols * rows);
        else if (mode == 1)
            clear_cells(0, pos + 1);
        else
            clear_cells(0, cols * rows);
        break;
    }
    case 'K': {
        unsigned mode = param(0, 0), line = cy * cols, x = MIN(cx, cols - 1);
        if (mode == 0)
            clear_cells(line + x, line + cols);
        else if (mode == 1)
            clear_cells(line, line + x + 1);
        else
            clear_cells(line, line + cols);
        break;
    }
    case 'L':                           /* qator qo'shish (muharrir: yangi qator) */
        if (cy >= top && cy <= bottom)
            scroll_down(cy, bottom, n);
        break;
    case 'M':                           /* qator o'chirish */
        if (cy >= top && cy <= bottom)
            scroll_up(cy, bottom, n);
        break;
    case 'S': scroll_up(top, bottom, n); break;
    case 'T': scroll_down(top, bottom, n); break;
    case '@': {                         /* belgilar qo'shish (o'ngga surish) */
        unsigned line = cy * cols;
        n = MIN(n, cols - cx);
        memmove(&cells[line + cx + n], &cells[line + cx], (cols - cx - n) * sizeof(*cells));
        for (unsigned x = cx + n; x < cols; x++)
            draw(x, cy);
        clear_cells(line + cx, line + cx + n);
        break;
    }
    case 'P': {                         /* belgilar o'chirish (chapga surish) */
        unsigned line = cy * cols;
        n = MIN(n, cols - cx);
        memmove(&cells[line + cx], &cells[line + cx + n], (cols - cx - n) * sizeof(*cells));
        for (unsigned x = cx; x < cols - n; x++)
            draw(x, cy);
        clear_cells(line + cols - n, line + cols);
        break;
    }
    case 'X': clear_cells(cy * cols + cx, cy * cols + MIN(cx + n, cols)); break;
    case 'm': sgr(); break;
    case 'r': {                         /* aylantirish hududi */
        unsigned t = param(0, 1) - 1, b = param(1, rows) - 1;
        if (t < b && b < rows) {
            top = t;
            bottom = b;
            set_cursor(0, 0);
        }
        break;
    }
    case 's': saved_x = cx, saved_y = cy; break;
    case 'u': set_cursor(saved_x, saved_y); break;
    case 'n':
        if (param(0, 0) == 6)           /* kursor qayerda? -> ESC[qator;ustunR */
            response_len = ksnprintf(response, sizeof(response), "\033[%u;%uR", cy + 1,
                                     MIN(cx, cols - 1) + 1);
        break;
    default:
        break;                          /* noma'lum - e'tiborsiz (xato emas) */
    }
}

/* ---- Asosiy kirish nuqtasi -------------------------------------------------- */

static void control(char c)
{
    switch (c) {
    case '\n':
        /* Unix'da '\n' = yangi qator + qator boshi (tty'ning ONLCR vazifasi -
         * bizda chiqarishni qayta ishlash yo'q, shuning uchun shu yerda). */
        cx = 0;
        wrap_pending = false;
        linefeed();
        break;
    case '\r':
        cx = 0;
        wrap_pending = false;
        break;
    case '\b':
        if (cx > 0)
            cx--;
        wrap_pending = false;
        break;
    case '\t':
        cx = MIN((cx + 8) & ~7u, cols - 1);
        break;
    case 0x1B:
        st = S_ESC;
        break;
    default:
        break;                          /* BEL va boshqalar */
    }
}

void vt_putc(char ch)
{
    if (!cells)
        return;
    uint8_t c = (uint8_t)ch;
    hide_cursor();

    switch (st) {
    case S_NORMAL:
        if (utf_need) {                 /* UTF-8 davomi: 10xxxxxx */
            if ((c & 0xC0) == 0x80) {
                utf_cp = (utf_cp << 6) | (c & 0x3F);
                if (--utf_need == 0)
                    put_char(utf_cp);
                break;
            }
            utf_need = 0;
            put_char('?');              /* buzilgan ketma-ketlik */
        }
        if (c < 0x20 || c == 0x7F)
            control((char)c);
        else if (c < 0x80)
            put_char(c);
        else if ((c & 0xE0) == 0xC0)
            utf_cp = c & 0x1F, utf_need = 1;
        else if ((c & 0xF0) == 0xE0)
            utf_cp = c & 0x0F, utf_need = 2;
        else if ((c & 0xF8) == 0xF0)
            utf_cp = c & 0x07, utf_need = 3;
        else
            put_char('?');
        break;
    case S_ESC:
        st = S_NORMAL;
        if (c == '[') {
            st = S_CSI;
            nparams = 0;
            param_started = false;
            priv = 0;
            memset(params, 0, sizeof(params));
        } else if (c == ']') {
            st = S_OSC;
        } else if (c == '7') {
            saved_x = cx, saved_y = cy;
        } else if (c == '8') {
            set_cursor(saved_x, saved_y);
        } else if (c == 'M') {          /* teskari indeks: yuqoriga, kerak bo'lsa pastga aylantirish */
            if (cy == top)
                scroll_down(top, bottom, 1);
            else if (cy > 0)
                cy--;
        } else if (c == 'D') {
            linefeed();
        } else if (c == 'E') {
            cx = 0;
            linefeed();
        } else if (c == 'c') {          /* to'liq reset */
            fg = 7, bg = 0, bold = reverse = false;
            top = 0, bottom = rows - 1;
            cursor_visible = true;
            clear_cells(0, cols * rows);
            set_cursor(0, 0);
        }
        break;
    case S_CSI:
        if (c >= '0' && c <= '9') {
            if (nparams == 0)
                nparams = 1;
            params[nparams - 1] = params[nparams - 1] * 10 + (c - '0');
            param_started = true;
        } else if (c == ';') {
            if (nparams == 0)
                nparams = 1;
            if (nparams < MAX_PARAMS)
                nparams++;
        } else if (c == '?' || c == '>' || c == '=') {
            priv = (char)c;
        } else if (c >= 0x40 && c <= 0x7E) {
            csi_dispatch((char)c);
            st = S_NORMAL;
        } else if (c < 0x20) {
            control((char)c);           /* CSI ichidagi boshqaruv belgilari ham bajariladi */
        }
        break;
    case S_OSC:                         /* ESC ] ... BEL yoki ESC \ */
        if (c == 0x07)
            st = S_NORMAL;
        else if (c == 0x1B)
            st = S_OSC_ESC;
        break;
    case S_OSC_ESC:
        st = c == '\\' ? S_NORMAL : S_OSC;
        break;
    }
    show_cursor();
}

void vt_set_color(uint8_t f, uint8_t b)
{
    fg = f & 0xF;
    bg = b & 0xF;
    bold = reverse = false;
}

void vt_clear(void)
{
    if (!cells)
        return;
    clear_cells(0, cols * rows);
    set_cursor(0, 0);
    show_cursor();
}

size_t vt_take_response(char *buf, size_t size)
{
    size_t n = MIN(response_len, size);
    memcpy(buf, response, n);
    response_len = 0;
    return n;
}

bool vt_attach(const struct screen_ops *ops)
{
    unsigned c, r;
    ops->get_size(&c, &r);
    struct vt_cell *nc = kmalloc((size_t)c * r * sizeof(*nc));
    if (!nc)
        return false;
    alt_cells = kmalloc((size_t)c * r * sizeof(*alt_cells));  /* bo'lmasa - alt ekransiz */
    scr = ops;
    cells = nc;
    cols = c, rows = r;
    top = 0, bottom = r - 1;
    cx = cy = 0;
    clear_cells(0, cols * rows);
    show_cursor();
    return true;
}
