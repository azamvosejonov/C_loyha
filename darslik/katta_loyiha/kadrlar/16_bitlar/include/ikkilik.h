/* ikkilik.h - xodimlar ombori uchun BINAR fayl formati (16-bob). Matn faylidan kichik va tez o'qiladi, lekin odam o'qiy olmaydi.

   Fayl tuzilishi (hamma ko'p baytli son LITTLE-ENDIAN: past bayt birinchi; protsessorga bog'liq EMAS):

     sarlavha (8 bayt):   [0..3] "KDR1" (sehrli bayt)  [4..5] versiya = 1 (u16)  [6..7] xodimlar soni (u16)
     har xodim (48 bayt): [0..1] id u16   [2] toifa u8   [3] bayroqlar u8   [4..11] tarif u64
                          [12..15] oddiy_daq u32   [16..19] qosh_daq u32   [20..43] ism (24 bayt, '\0' bilan to'ldirilgan)
                          [44..45] nazorat yig'indisi u16 (Fletcher-16, [0..43] baytlar uchun)   [46..47] zaxira = 0
*/
#ifndef IKKILIK_H
#define IKKILIK_H

#include <stddef.h>
#include <stdint.h>

#include "ombor.h"
#include "yukla.h"

#define IKKILIK_SEHR "KDR1"
#define IKKILIK_SARLAVHA 8
#define IKKILIK_YOZUV 48

/* tasodifiy baytlar uchun oddiy 16 bitli nazorat yig'indisi (Fletcher-16). Bitta bayt o'zgarsa - deyarli har doim boshqa natija */
uint16_t nazorat16(const uint8_t *b, size_t uzunlik);

/* bitta xodimni 48 baytga yozadi / 48 baytdan o'qiydi (fayl ishlatmaydi: sinash oson) */
void yozuv_yoz(const struct xodim *x, uint8_t chiqish[IKKILIK_YOZUV]);
int yozuv_oqi(const uint8_t kirish[IKKILIK_YOZUV], struct xodim *x, const char **sabab);

/* butun omborni yangi faylga saqlaydi (mavjud faylni bosmaydi). 0 - OK, -1 - xato (errno) */
int ikkilik_saqla(const struct ombor *o, const char *yol);

/* faylni o'qib, xodimlarni omborga qo'shadi. Buzilgan yozuvlar rad etiladi (h->rad), sabab stderr ga. 0 - o'qildi, -1 - fayl yaroqsiz */
int ikkilik_yukla(struct ombor *o, const char *yol, struct yuklash *h);

/* faylning boshidagi n baytni hexdump ko'rinishida chiqaradi: siljish, 16 bayt hex, ASCII */
int hex_chiqar(const char *yol, size_t n);

#endif
