/* matn.h - ID va matn (satr) yordamchilari. Satr - '\0' bilan tugaydigan char massivi (6-bob) */
#ifndef MATN_H
#define MATN_H

/* [TODO T5 ★ 2-bob] ID ning nazorat raqami. ID - 4 xonali son (1000..9999). Raqamlar chapdan o'ngga 3, 7, 1, 3 ga ko'paytirilib
   qo'shiladi, yig'indining 10 ga bo'lingandagi QOLDIG'I nazorat raqami. Noto'g'ri ID (1000 dan kichik yoki 9999 dan katta) -> -1.
   Misol: id_nazorat(1042) = 3 (1*3 + 0*7 + 4*1 + 2*3 = 13); id_nazorat(2087) = 5; id_nazorat(9999) = 6; id_nazorat(99) = -1.
   Maslahat: raqamlarni "% 10" va "/ 10" bilan ajrating. */
int id_nazorat(int id);

/* [TODO T6 ★★★ 6-bob] satr uzunligi ('\0' hisobga olinmaydi). strlen() dan foydalanmang - o'zingiz yozing.
   Misol: nom_uzunligi("") = 0; nom_uzunligi("Aziza") = 5. */
int nom_uzunligi(const char *s);

/* [TODO T7 ★★★ 6-bob] ikki satr aynan tengmi? Teng bo'lsa 1, aks holda 0. strcmp() dan foydalanmang.
   Katta-kichik harf farqlanadi: nom_teng("Ali", "ali") = 0. Misol: nom_teng("Bobur", "Bobur") = 1; nom_teng("Bob", "Bobur") = 0. */
int nom_teng(const char *a, const char *b);

#endif
