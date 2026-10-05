/* ombor.h - xodimlar ombori: massivda saqlanadi (6-bob), struct (9-bob), ko'rsatkichlar (7-bob) */
#ifndef OMBOR_H
#define OMBOR_H

#include "xodim.h"

struct ombor {
    struct xodim a[MAKS_XODIM];
    int soni;                               /* hozir nechta xodim bor (0..MAKS_XODIM) */
};

/* omborni bo'shatadi */
void ombor_boshla(struct ombor *o);

/* xodim qo'shadi. 0 - OK; -1 - ombor to'la; -2 - bu ID allaqachon bor (ombor_top dan foydalanadi) */
int ombor_qosh(struct ombor *o, const struct xodim *x);

/* [TODO T10 ★★★ 7-bob] ID bo'yicha xodimni topadi: omborning shu xodimiga KO'RSATKICH (nusxa emas!), topilmasa NULL.
   Qaytgan ko'rsatkich orqali xodimni o'zgartirsa bo'ladi (davomat.c shunday qiladi). */
struct xodim *ombor_top(struct ombor *o, int id);

/* [TODO T11 ★★★ 6-bob] ism bo'yicha topadi (nom_teng() dan foydalaning). Topilmasa NULL. */
struct xodim *ombor_ism_bilan_top(struct ombor *o, const char *ism);

/* [TODO T12 ★★★ 6-bob] xodimlarni tarif bo'yicha KAMAYISH tartibida saralaydi (eng yuqori tarif birinchi).
   O'zingiz yozing (pufakchali yoki tanlab saralash); qsort() ishlatmang. Tarif teng bo'lsa - tartib ahamiyatsiz.
   Eslatma: struct larni massivda almashtirish oddiy tenglik bilan bo'ladi: tmp = a[i]; a[i] = a[j]; a[j] = tmp; */
void ombor_saralash_tarif(struct ombor *o);

/* [TODO T13 ★★★★ 12-bob] "xodimlar.txt" fayldan o'qiydi. Har qator: id ism toifa tarif  (probel bilan ajratilgan).
   '#' bilan boshlangan va bo'sh qatorlar o'tkaziladi. Noto'g'ri qator (maydonlar yetishmaydi, ID 1000..9999 emas, toifa 1..4 emas,
   tarif <= 0, ID takrori, ombor to'la) o'tkazib yuboriladi va *rad ni oshiradi.
   Qaytaradi: muvaffaqiyatli o'qilgan xodimlar soni, fayl ochilmasa -1.
   Maslahat: fopen, fgets (yoki fscanf), sscanf("%d %23s %d %lld"), ombor_qosh. Namuna uchun davomat.c ga qarang - u xuddi shunday ishlaydi. */
int ombor_yukla(struct ombor *o, const char *fayl, int *rad);

#endif
