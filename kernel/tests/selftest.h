/* =============================================================================
 *  tests/selftest.h - yadro ichidagi o'z-o'zini tekshirish testlari
 * ============================================================================= */
#pragma once

/* Barcha testlarni ishga tushiradi. Yadro buyruq qatorida "selftest" bo'lsa
 * kmain() uni chaqiradi:  make run APPEND=selftest */
void selftest_run(void);
