/* =============================================================================
 *  09 - Buferga xavfsiz nusxalash                         [2-modul: ko'rsatkichlar]
 * =============================================================================
 *
 *  VAZIFA (BSD'dagi strlcpy / strlcat):
 *    xavfsiz_nusxa(dst, src, hajm)
 *        dst - `hajm` baytli bufer. src ni unga nusxalang, lekin ko'pi bilan
 *        hajm - 1 bayt, va hajm > 0 bo'lsa oxiriga DOIM '\0' qo'ying.
 *        Qaytish: strlen(src) (qisqartirilganini chaqiruvchi shundan biladi:
 *        natija >= hajm bo'lsa - satr sig'madi).
 *        hajm == 0 bo'lsa: dst ga hech narsa yozmang.
 *
 *    xavfsiz_ulash(dst, src, hajm)
 *        dst da allaqachon satr bor (u hajm ichida '\0' bilan tugaydi).
 *        src ni uning oxiriga qo'shing, jami hajm - 1 baytdan oshirmasdan.
 *        Qaytish: strlen(dst_boshlang'ich) + strlen(src).
 *
 *  PYTHON'DA:
 *    Muammo yo'q - satrlar o'zi o'sadi.
 *
 *  NEGA BU MUHIM:
 *    `strcpy(buf, foydalanuvchi_satri)` - tarixdagi eng ko'p hujumlar (buffer
 *    overflow) aynan shunday boshlangan. Yadroda foydalanuvchi bergan HAR BIR
 *    uzunlik tekshiriladi. MyOS'da: ksnprintf, path_normalize, getcwd.
 *
 *  MASLAHAT:
 *    Test buferlarni aniq hajmda malloc qiladi - bitta ortiqcha bayt yozsangiz,
 *    sanitizer "heap-buffer-overflow" deydi. Qog'ozda hajm = 1, 2, 6 uchun sinang.
 *
 *  TEKSHIRISH:  tools/mashq.py tekshir 09
 * ============================================================================= */
#include "mashq.h"

size_t xavfsiz_nusxa(char *dst, const char *src, size_t hajm)
{
    /* TODO */
    (void)dst; (void)src; (void)hajm;
    return 0;
}

size_t xavfsiz_ulash(char *dst, const char *src, size_t hajm)
{
    /* TODO */
    (void)dst; (void)src; (void)hajm;
    return 0;
}
