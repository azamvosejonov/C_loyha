/* =============================================================================
 *  drivers/tty.c - TERMINAL: kanonik rejim, echo, backspace, EOF
 * =============================================================================
 *
 *  Klaviatura drayveri bizga XOM belgilarni beradi: 'l', 's', '\b', '\r' ...
 *  Lekin dastur (masalan `cat`) "bitta qator" o'qishni xohlaydi va foydalanuvchi
 *  xatosini backspace bilan tuzata olishi kerak. Bu ishni har bir dastur
 *  o'zi qilmasligi uchun Unix uni YADROGA qo'ygan: "LINE DISCIPLINE".
 *
 *      klaviatura IRQ ──► console input bufer (xom) ──► tty_read:
 *                                                       ├─ echo (ekranga qaytarish)
 *                                                       ├─ backspace: qatordan o'chirish
 *                                                       ├─ Ctrl-U: butun qatorni o'chirish
 *                                                       ├─ Ctrl-D: fayl oxiri (EOF)
 *                                                       └─ Enter: qator TAYYOR -> dasturga
 *
 *  Ikki rejim (termios.c_lflag dagi ICANON):
 *    kanonik - read() faqat butun qator tayyor bo'lganda qaytadi.
 *    xom     - read() birinchi kelgan belgi bilan qaytadi (muharrir, o'yin).
 *  Dastur rejimni ioctl(TCGETS/TCSETS) bilan o'qiydi/o'zgartiradi - Linux'dagi
 *  tcgetattr/tcsetattr aynan shu ioctl'lar.
 *
 *  SODDALASHTIRISH (Linux'dan farq): belgilar o'quvchi jarayon kontekstida
 *  qayta ishlanadi. Ya'ni hech kim read() qilmayotganda terilgan belgilar
 *  darhol ekranda ko'rinmaydi - o'qilganda chiqadi. Linux buni uzilish
 *  kontekstida (n_tty_receive_buf) qiladi. Mashqlarga qarang.
 *
 *  MAXSUS TUGMALAR (ISIG yoqiq bo'lsa) signalga aylanadi:
 *      Ctrl-C  -> SIGINT    Ctrl-\ -> SIGQUIT    Ctrl-Z -> SIGTSTP
 *  Ular OLDINGI PLANDAGI jarayon guruhiga (fg_pgrp) yuboriladi - shell buni
 *  har bir buyruqdan oldin TIOCSPGRP bilan o'rnatadi. Bu tugmalar klaviatura
 *  UZILISHIDA qayta ishlanadi (tty_input_signal): cheksiz tsikldagi dastur
 *  terminaldan hech narsa o'qimasa ham Ctrl-C uni to'xtatadi.
 *
 *  JOB CONTROL: fon (background) guruhidagi jarayon terminaldan o'qimoqchi
 *  bo'lsa, SIGTTIN oladi va to'xtaydi - aks holda ikki dastur bitta
 *  klaviatura uchun "urishardi".
 * ============================================================================= */
#include "drivers/tty.h"

#include "drivers/console.h"
#include "lib/common.h"
#include "lib/mutex.h"
#include "lib/string.h"
#include "proc/process.h"
#include "proc/signal.h"
#include "sys/uaccess.h"

#define LINE_MAX 1024

#define CTRL(c) ((c) & 0x1F)

static struct {
    struct mutex lock;                  /* bir vaqtda faqat bitta o'quvchi qatorni yig'adi */
    struct myos_termios tio;
    char line[LINE_MAX];
    size_t len;                         /* qatordagi belgilar */
    size_t ready;                       /* shundan nechtasi dasturga berishga tayyor */
    size_t rpos;                        /* tayyor qismning qanchasi allaqachon o'qildi */
    bool eof;                           /* Ctrl-D bo'sh qatorda: keyingi read() = 0 */
    int fg_pgrp;                        /* oldingi plan guruhi (0 - job control yo'q) */
} tty = {
    .lock = MUTEX_INIT("tty"),
    .tio = {
        .c_iflag = ICRNL,
        .c_lflag = ISIG | ICANON | ECHO | ECHOE,
        .c_cc = {
            [VINTR] = CTRL('C'),
            [VQUIT] = 0x1C,             /* Ctrl-\ */
            [VSUSP] = CTRL('Z'),
            [VERASE] = 0x7F,
            [VKILL] = CTRL('U'),
            [VEOF] = CTRL('D'),
            [VMIN] = 1,
        },
    },
};

static void echo(const char *s, size_t n)
{
    if (tty.tio.c_lflag & ECHO)
        console_write(s, n);
}

/* Qatordan oxirgi belgini o'chirish (ekrandan ham). */
static void erase_one(void)
{
    if (tty.len == tty.ready)
        return;                         /* tayyor (yuborilgan) qismni o'chirib bo'lmaydi */
    tty.len--;
    if (tty.tio.c_lflag & ECHOE)
        echo("\b \b", 3);
}

/* Bitta xom belgini kanonik rejimda qayta ishlash. */
static void canon_input(char c)
{
    const uint8_t *cc = tty.tio.c_cc;
    if (c == '\r' && (tty.tio.c_iflag & ICRNL))
        c = '\n';

    if (c == (char)cc[VERASE] || c == '\b') {
        erase_one();
    } else if (c == (char)cc[VKILL]) {
        while (tty.len > tty.ready)
            erase_one();
    } else if ((tty.tio.c_lflag & ISIG) &&
               (c == (char)cc[VINTR] || c == (char)cc[VQUIT] || c == (char)cc[VSUSP])) {
        /* Signal allaqachon yuborilgan va "^C" chiqarilgan (tty_input_signal). */
        tty.len = tty.ready;            /* joriy (yarim terilgan) qatorni bekor qilamiz */
    } else if (c == (char)cc[VEOF]) {
        /* Ctrl-D: qatorda nimadir bo'lsa - uni '\n' siz yuboramiz, bo'lmasa - EOF. */
        if (tty.len == tty.ready)
            tty.eof = true;
        tty.ready = tty.len;
    } else if (c == '\n') {
        if (tty.len < LINE_MAX)
            tty.line[tty.len++] = '\n';
        tty.ready = tty.len;            /* qator tugadi - dastur olishi mumkin */
        echo("\n", 1);
    } else if ((unsigned char)c >= 32 || c == '\t') {
        if (tty.len < LINE_MAX - 1) {   /* '\n' uchun doim joy qoldiramiz */
            tty.line[tty.len++] = c;
            echo(&c, 1);
        }
    }
    /* Boshqa boshqaruv belgilari (masalan, strelkalar ESC ketma-ketligi) - tashlanadi. */
}

bool tty_input_signal(char c)
{
    uint32_t lflag = tty.tio.c_lflag;   /* qulfsiz o'qish - uzilish kontekstida mutex yo'q */
    if (!(lflag & ISIG))
        return false;
    const uint8_t *cc = tty.tio.c_cc;
    int sig;
    const char *shown;
    if (c == (char)cc[VINTR])
        sig = SIGINT, shown = "^C\n";
    else if (c == (char)cc[VQUIT])
        sig = SIGQUIT, shown = "^\\\n";
    else if (c == (char)cc[VSUSP])
        sig = SIGTSTP, shown = "^Z\n";
    else
        return false;
    if (lflag & ECHO)
        console_write(shown, strlen(shown));
    int pg = __atomic_load_n(&tty.fg_pgrp, __ATOMIC_RELAXED);
    if (pg > 0)
        signal_send_pgrp(pg, sig);
    return true;
}

/* Fon guruhidagi jarayon o'qimoqchi: SIGTTIN (standart amal - to'xtash). */
static int background_read_check(void)
{
    struct process *me = current;
    int pg = __atomic_load_n(&tty.fg_pgrp, __ATOMIC_RELAXED);
    if (pg <= 0 || !me->is_user || me->pgid == pg)
        return 0;
    if (me->sig_actions[SIGTTIN].sa_handler == SIG_IGN || (me->sig_blocked & (1u << SIGTTIN)))
        return -EIO;                    /* POSIX: to'xtatib bo'lmasa - xato */
    signal_send_pgrp(me->pgid, SIGTTIN);
    return -EINTR;                      /* to'xtaydi; SIGCONT dan keyin read qayta boshlanadi */
}

static int64_t tty_read(struct file *f, void *buf, size_t len, uint64_t off)
{
    (void)f, (void)off;
    if (len == 0)
        return 0;
    int bg = background_read_check();
    if (bg)
        return bg;
    mutex_lock(&tty.lock);
    int64_t ret;

    if (!(tty.tio.c_lflag & ICANON)) {
        /* XOM REJIM: kamida 1 belgini kutamiz, keyin tayyor turganlarini olamiz. */
        char *out = buf;
        int c = console_getc();
        if (c < 0) {
            ret = -EINTR;               /* kill() tufayli uzildi */
            goto out;
        }
        size_t n = 0;
        out[n++] = (char)c;
        while (n < len && console_input_available() && (c = console_getc()) >= 0)
            out[n++] = (char)c;
        echo(out, n);
        ret = (int64_t)n;
        goto out;
    }

    /* KANONIK REJIM: tayyor qator yo'q ekan - belgilarni yig'amiz. */
    while (tty.rpos == tty.ready && !tty.eof) {
        int c = console_getc();
        if (c < 0) {
            ret = -EINTR;
            goto out;
        }
        canon_input((char)c);
    }
    if (tty.rpos == tty.ready) {        /* eof va hech narsa yo'q */
        tty.eof = false;                /* keyingi read() yana kutadi (bash kabi) */
        ret = 0;
        goto out;
    }
    size_t n = MIN(len, tty.ready - tty.rpos);
    memcpy(buf, tty.line + tty.rpos, n);
    tty.rpos += n;
    if (tty.rpos == tty.ready) {
        /* Tayyor qism to'liq berildi: qolgan (yarim terilgan) qismni boshga suramiz. */
        memmove(tty.line, tty.line + tty.ready, tty.len - tty.ready);
        tty.len -= tty.ready;
        tty.ready = tty.rpos = 0;
    }
    ret = (int64_t)n;
out:
    mutex_unlock(&tty.lock);
    return ret;
}

static int64_t tty_write(struct file *f, const void *buf, size_t len, uint64_t off)
{
    (void)f, (void)off;
    console_write(buf, len);
    return (int64_t)len;
}

static int tty_ioctl(struct file *f, uint64_t cmd, uint64_t arg)
{
    (void)f;
    switch (cmd) {
    case TCGETS: {
        mutex_lock(&tty.lock);
        struct myos_termios t = tty.tio;
        mutex_unlock(&tty.lock);
        return copy_to_user(arg, &t, sizeof(t)) ? -EFAULT : 0;
    }
    case TCSETS: {
        struct myos_termios t;
        if (copy_from_user(&t, arg, sizeof(t)))
            return -EFAULT;
        mutex_lock(&tty.lock);
        bool was_canon = tty.tio.c_lflag & ICANON;
        tty.tio = t;
        if (was_canon && !(t.c_lflag & ICANON)) {
            tty.len = tty.ready = tty.rpos = 0; /* yarim qator tashlanadi */
            tty.eof = false;
        }
        mutex_unlock(&tty.lock);
        return 0;
    }
    case TIOCGPGRP: {
        int32_t pg = tty.fg_pgrp;
        return copy_to_user(arg, &pg, sizeof(pg)) ? -EFAULT : 0;
    }
    case TIOCSPGRP: {                   /* tcsetpgrp: shell buyruqni oldingi planga o'tkazadi */
        int32_t pg;
        if (copy_from_user(&pg, arg, sizeof(pg)))
            return -EFAULT;
        if (pg < 0)
            return -EINVAL;
        __atomic_store_n(&tty.fg_pgrp, pg, __ATOMIC_RELAXED);
        return 0;
    }
    case TIOCGWINSZ: {
        unsigned cols, rows;
        console_get_size(&cols, &rows);
        struct myos_winsize ws = { (uint16_t)rows, (uint16_t)cols, 0, 0 };
        return copy_to_user(arg, &ws, sizeof(ws)) ? -EFAULT : 0;
    }
    default:
        return -ENOTTY;
    }
}

const struct file_ops tty_fops = {
    .read = tty_read,
    .write = tty_write,
    .ioctl = tty_ioctl,
    .seekable = false,
};
