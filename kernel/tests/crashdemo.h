/* =============================================================================
 *  tests/crashdemo.h - xotira xatolarini ATAYLAB yaratib ko'rsatish
 * ============================================================================= */
#pragma once

/* name: "null", "uaf", "doublefree", "badfree", "stack". Noma'lum bo'lsa hech
 * narsa qilmaydi. Yadro buyruq qatori orqali: make run APPEND=demo=uaf */
void crashdemo_run(const char *name);
