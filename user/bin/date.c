/* =============================================================================
 *  user/bin/date.c - joriy sana va vaqt
 *
 *  Vaqt manbai: RTC (CMOS soati, batareya bilan ishlaydi) - yadro uni boot
 *  paytida o'qiydi va taymer bilan davom ettiradi (drivers/rtc.c).
 *  RTC odatda UTC da (Linux) yoki mahalliy vaqtda (Windows) saqlanadi.
 *  Vaqt zonalari yo'q: -z N bilan UTC+N ni ko'rsatish mumkin (Toshkent: -z 5).
 * ============================================================================= */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char **argv)
{
    long offset_h = 0;
    if (argc > 2 && strcmp(argv[1], "-z") == 0)
        offset_h = atoi(argv[2]);
    time_t t = time(NULL) + offset_h * 3600;
    struct tm tm;
    gmtime_r(&t, &tm);
    static const char *days[] = { "Yakshanba", "Dushanba", "Seshanba", "Chorshanba",
                                  "Payshanba", "Juma", "Shanba" };
    static const char *months[] = { "yanvar", "fevral", "mart", "aprel", "may", "iyun",
                                    "iyul", "avgust", "sentabr", "oktabr", "noyabr", "dekabr" };
    printf("%s, %d-%s %d, %02d:%02d:%02d UTC%+ld\n", days[tm.tm_wday], tm.tm_mday,
           months[tm.tm_mon], tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec, offset_h);
    return 0;
}
