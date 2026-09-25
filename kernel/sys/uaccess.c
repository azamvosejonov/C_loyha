/* =============================================================================
 *  sys/uaccess.c - user xotirasi bilan xavfsiz ishlash
 * =============================================================================
 *
 *  ENG MUHIM XAVFSIZLIK QOIDASI: user bergan ko'rsatkich - DUSHMAN qiymati.
 *    read(fd, 0xFFFFFFFF80100000, 100)  - yadro kodining ustiga yozdirishga urinish
 *    write(fd, 0xFFFF800000000000, 1e6) - yadro xotirasini o'qib olishga urinish
 *    open(ko'rsatkich xaritalanmagan sahifaga cho'zilgan satr)
 *  Har bir ko'rsatkich ishlatilishidan OLDIN tekshiriladi: user hududidami,
 *  jarayonning VMA sidami, ruxsatlari mosmi. mm_prefault sahifalarni hozir
 *  yaratadi (demand paging) va yozish uchun COW ni buzadi - shundan keyin
 *  yadro to'g'ridan-to'g'ri, page fault'siz murojaat qila oladi.
 * ============================================================================= */
#include "sys/uaccess.h"

#include "lib/string.h"
#include "mm/layout.h"
#include "mm/mm.h"
#include "myos/abi.h"
#include "proc/process.h"

bool user_ok(uint64_t addr, size_t len, bool write)
{
    return mm_prefault(current->mm, addr, len, write);
}

long copy_string_from_user(char *dst, uint64_t src, size_t max)
{
    for (size_t i = 0; i < max; i++) {
        /* Har bir yangi sahifada qayta tekshiramiz: satr keyingi (xaritalanmagan)
         * sahifaga "cho'zilib" ketishi mumkin. */
        if (i == 0 || ((src + i) & (PAGE_SIZE - 1)) == 0)
            if (!user_ok(src + i, 1, false))
                return -EFAULT;
        dst[i] = *(const char *)(uintptr_t)(src + i);
        if (dst[i] == '\0')
            return (long)i;
    }
    return -ENAMETOOLONG;
}

int copy_from_user(void *dst, uint64_t src, size_t len)
{
    if (!user_ok(src, len, false))
        return -EFAULT;
    memcpy(dst, (const void *)(uintptr_t)src, len);
    return 0;
}

int copy_to_user(uint64_t dst, const void *src, size_t len)
{
    if (!user_ok(dst, len, true))
        return -EFAULT;
    memcpy((void *)(uintptr_t)dst, src, len);
    return 0;
}
