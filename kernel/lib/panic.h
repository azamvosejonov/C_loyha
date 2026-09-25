/* =============================================================================
 *  lib/panic.h - tuzatib bo'lmaydigan xato: tizimni to'xtatish
 * ============================================================================= */
#pragma once

/* noreturn - kompilyatorga "bu funksiyadan qaytilmaydi" deydi. U shunda
 * chaqiruvdan keyingi kodni generatsiya qilmaydi va ogohlantirishlar kamayadi. */
__attribute__((noreturn, format(printf, 1, 2)))
void panic(const char *fmt, ...);

/* ASSERT - "bu shart HAR DOIM to'g'ri bo'lishi kerak". Agar noto'g'ri bo'lsa -
 * yadroda xato bor, davom etish xavfli (ma'lumotlar buziladi). Kuchli
 * dasturchilar o'z taxminlarini ASSERT bilan kodga yozib qo'yadi: bu ham hujjat,
 * ham xatoni erta ushlash vositasi.
 * #expr - ifodani matnga aylantiradi, __FILE__ va __LINE__ - joylashuv. */
#define ASSERT(expr)                                                         \
    do {                                                                     \
        if (!(expr))                                                         \
            panic("ASSERT xato: %s\n  %s:%d", #expr, __FILE__, __LINE__);    \
    } while (0)
