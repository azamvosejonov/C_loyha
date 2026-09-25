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
 *  Ctrl-C (VINTR) hozircha faqat joriy qatorni bekor qiladi. Signallar
 *  qo'shilgach, u oldingi plandagi jarayonga SIGINT yuboradi.
 * ============================================================================= */
#include "drivers/tty.h"

#include "drivers/console.h"
#include "lib/common.h"
#include "lib/mutex.h"
#include "lib/string.h"
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
} tty = {
    .lock = MUTEX_INIT("tty"),
    .tio = {
        .c_iflag = ICRNL,
        .c_lflag = ISIG | ICANON | ECHO | ECHOE,
        .c_cc = {
            [VINTR] = CTRL('C'),
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
    } else if (c == (char)cc[VINTR] && (tty.tio.c_lflag & ISIG)) {
        tty.len = tty.ready;            /* joriy qatorni bekor qilamiz */
        echo("^C\n", 3);
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

static int64_t tty_read(struct file *f, void *buf, size_t len, uint64_t off)
{
    (void)f, (void)off;
    if (len == 0)
        return 0;
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
