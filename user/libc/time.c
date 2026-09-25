/* =============================================================================
 *  user/libc/time.c - Unix vaqtini sana-soatga aylantirish
 * =============================================================================
 *
 *  Yadro vaqtni bitta son sifatida beradi: 1970-01-01 00:00:00 UTC dan beri
 *  o'tgan soniyalar ("Unix epoch"). Uni yil/oy/kunga aylantirish - sof
 *  arifmetika (kabisa yillari bilan).
 *
 *  Algoritm: Howard Hinnant'ning "days_from_civil" teskarisi - yillarni 400
 *  yillik "era"larga bo'ladi (har bir era aynan 146097 kun), shunda kabisa
 *  qoidalari (4, 100, 400) oddiy bo'lish amallari bilan hisoblanadi.
 * ============================================================================= */
#include <time.h>

struct tm *gmtime_r(const time_t *t, struct tm *out)
{
    /* >>> LAB gmtime_r - vazifa: labs/README.md */
    int64_t secs = *t;
    int64_t days = secs / 86400;
    int64_t rem = secs % 86400;
    if (rem < 0) {                      /* 1970 dan oldingi vaqt */
        rem += 86400;
        days--;
    }
    out->tm_hour = (int)(rem / 3600);
    out->tm_min = (int)(rem % 3600 / 60);
    out->tm_sec = (int)(rem % 60);
    out->tm_wday = (int)((days % 7 + 11) % 7);  /* 1970-01-01 - payshanba (4) */

    /* Mart-dan boshlanadigan yil: kabisa kuni (29-fevral) yil OXIRIDA bo'ladi. */
    int64_t z = days + 719468;
    int64_t era = (z >= 0 ? z : z - 146096) / 146097;
    int64_t doe = z - era * 146097;                                 /* [0, 146096] */
    int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;   /* [0, 399] */
    int64_t y = yoe + era * 400;
    int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);          /* [0, 365] */
    int64_t mp = (5 * doy + 2) / 153;                               /* [0, 11], 0 = mart */
    int64_t d = doy - (153 * mp + 2) / 5 + 1;
    int64_t m = mp < 10 ? mp + 3 : mp - 9;                          /* [1, 12] */
    y += m <= 2;

    out->tm_year = (int)(y - 1900);
    out->tm_mon = (int)(m - 1);
    out->tm_mday = (int)d;
    static const int cum[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
    int leap = (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
    out->tm_yday = cum[m - 1] + (int)d - 1 + (leap && m > 2);
    return out;
    /* <<< LAB gmtime_r */
}
