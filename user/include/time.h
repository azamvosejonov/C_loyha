/* time.h - vaqt */
#pragma once
#include <stdint.h>

typedef int64_t time_t;

struct tm {
    int tm_sec, tm_min, tm_hour;
    int tm_mday, tm_mon, tm_year;       /* tm_mon: 0..11, tm_year: 1900 dan */
    int tm_wday, tm_yday;
};

time_t time(time_t *out);
struct tm *gmtime_r(const time_t *t, struct tm *out);
