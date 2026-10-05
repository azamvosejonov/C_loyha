/* pool.h - oqimlar puli (thread pool): ishchilar navbatdan topshiriq oladi. Navbat CHEGARALANGAN: to'lsa yuboruvchi kutadi */
#ifndef POOL_H
#define POOL_H

typedef void (*topshiriq_fn)(void *arg);

struct pool;

struct pool *pool_yarat(int ishchilar, int navbat_sigimi);
int pool_yubor(struct pool *p, topshiriq_fn f, void *arg);    /* 0 - OK; navbat to'la bo'lsa joy bo'shaguncha KUTADI; -1 - pool yopilgan */
void pool_tugat(struct pool *p);                              /* navbatdagi hamma ishni tugatadi, ishchilarni to'xtatadi, xotirani qaytaradi */

#endif
