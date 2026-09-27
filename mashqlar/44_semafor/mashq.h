#pragma once

struct semafor;                             /* ichki tuzilishi - yechim.c da */

struct semafor *semafor_yarat(int boshlangich);
void semafor_kut(struct semafor *s);        /* P: qiymat > 0 bo'lguncha kutib, 1 ga kamaytirish */
void semafor_ber(struct semafor *s);        /* V: 1 ga oshirish, kutayotgan bo'lsa - uyg'otish */
int semafor_urin(struct semafor *s);        /* kutmasdan urinish: oldi -> 1, yo'q -> 0 */
void semafor_yoq(struct semafor *s);
