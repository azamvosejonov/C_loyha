/* sistema.h - operatsion tizim bilan to'g'ridan-to'g'ri ishlash (14-bob): open/write/close, fork/exec/pipe/wait.
   Hamma funksiya xatoda -1 qaytaradi va errno ni qoldiradi (sababini strerror(errno) bilan olish mumkin). */
#ifndef SISTEMA_H
#define SISTEMA_H

#include <stddef.h>

/* fd ga HAMMA baytni yozadi. write() ba'zan KAMROQ yozadi (yarim yozish) yoki signal bilan uziladi (EINTR): ikkalasi ham shu yerda hal qilinadi */
int yoz_hammasi(int fd, const void *malumot, size_t uzunlik);

/* YANGI fayl yaratib, ma'lumotni yozadi. Fayl allaqachon bor bo'lsa YOZMAYDI (O_EXCL: errno = EEXIST) - mavjud hisobotni tasodifan yo'qotmaymiz */
int fayl_yarat_yoz(const char *yol, const void *malumot, size_t uzunlik);

/* argv[0] dasturni ishga tushiradi (PATH bo'yicha qidiradi), uning kirishiga (stdin) ma'lumotni quvur (pipe) orqali beradi.
   Dasturning chiqishi bizning stdout ga tushadi. Tugashini kutadi. Qaytaradi: dasturning chiqish kodi (0..255),
   signal bilan o'lgan bo'lsa 128 + signal raqami, ishga tushmasa 127; tizim xatosida -1 */
int dastur_ishlat(char *const argv[], const void *kirish, size_t uzunlik);

#endif
