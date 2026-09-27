/* =============================================================================
 *  27_fayllar.c - inode, qattiq havola, unlink, xavfsiz saqlash (rename)  (27-bob)
 * =============================================================================
 *  Ishga tushirish:
 *      gcc -Wall -Wextra -g 27_fayllar.c -o fayllar && ./fayllar
 *
 *  Kutilgan natija (inode raqamlari boshqacha):
 *      a.txt: inode 1234567, havolalar 1, hajm 6
 *      link("a.txt", "b.txt") dan keyin: a.txt inode 1234567, b.txt inode 1234567, havolalar 2
 *      unlink("a.txt") dan keyin b.txt hali bor: "salom\n"
 *      ochiq fayl o'chirildi, lekin hali o'qiladi: "salom\n"
 *      xavfsiz saqlash: yangi.tmp yozildi, fsync, rename -> sozlama.txt = "versiya 2\n"
 *
 *  Sinab ko'ring:
 *      1) Terminalda: `echo salom > x; ln x y; ln -s x z; ls -li x y z` - x va y ning inode'i
 *         bir xil, z niki boshqa (ramziy havola - alohida fayl).
 *      2) link() ni symlink() ga almashtiring. unlink("a.txt") dan keyin b.txt ochilmaydi -
 *         "osilib qolgan" havola. open() natijasini tekshirishni qo'shing.
 *      3) Xavfsiz saqlash nega kerak: sozlama.txt ni to'g'ridan-to'g'ri O_TRUNC bilan ochib,
 *         write() dan OLDIN abort() chaqiring. Fayl bo'sh qoldi! rename usulida esa eski
 *         versiya butun qoladi.
 *      4) `stat 27_fayllar.c` buyrug'i - inode, havolalar, bloklar.
 * ============================================================================= */
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static void yoz(const char *yol, const char *s)
{
    int fd = open(yol, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0 || write(fd, s, strlen(s)) != (ssize_t)strlen(s))
        perror(yol);                            /* tizim chaqiruvi natijasini DOIM tekshiring */
    fsync(fd);                                  /* haqiqatan diskka (27.3) */
    close(fd);
}

static long ino(const char *yol, long *havola)
{
    struct stat st;
    if (stat(yol, &st) != 0)
        return -1;
    if (havola)
        *havola = (long)st.st_nlink;
    return (long)st.st_ino;
}

int main(void)
{
    long h = 0;
    yoz("a.txt", "salom\n");
    struct stat st;
    stat("a.txt", &st);
    printf("a.txt: inode %ld, havolalar %ld, hajm %ld\n", (long)st.st_ino, (long)st.st_nlink, (long)st.st_size);

    unlink("b.txt");
    if (link("a.txt", "b.txt") != 0)            /* bitta inode'ga IKKINCHI NOM */
        perror("link");
    long ia = ino("a.txt", NULL), ib = ino("b.txt", &h);
    printf("link(\"a.txt\", \"b.txt\") dan keyin: a.txt inode %ld, b.txt inode %ld, havolalar %ld\n", ia, ib, h);

    unlink("a.txt");                            /* nom o'chdi - ma'lumot b.txt orqali yashaydi */
    char buf[32] = { 0 };
    int fd = open("b.txt", O_RDONLY);
    if (read(fd, buf, sizeof(buf) - 1) < 0)
        perror("read");
    printf("unlink(\"a.txt\") dan keyin b.txt hali bor: \"%.*s\\n\"\n", (int)strcspn(buf, "\n"), buf);

    unlink("b.txt");                            /* oxirgi nom ham o'chdi, lekin fd hali ochiq */
    memset(buf, 0, sizeof(buf));
    lseek(fd, 0, SEEK_SET);
    if (read(fd, buf, sizeof(buf) - 1) < 0)
        perror("read");
    printf("ochiq fayl o'chirildi, lekin hali o'qiladi: \"%.*s\\n\"\n", (int)strcspn(buf, "\n"), buf);
    close(fd);                                  /* endi inode haqiqatan bo'shatiladi */

    yoz("sozlama.txt", "versiya 1\n");
    yoz("yangi.tmp", "versiya 2\n");            /* 1) yangi mazmunni vaqtinchalik faylga */
    rename("yangi.tmp", "sozlama.txt");         /* 2) atomik almashtirish: yo eski, yo yangi */
    fd = open("sozlama.txt", O_RDONLY);
    memset(buf, 0, sizeof(buf));
    if (read(fd, buf, sizeof(buf) - 1) < 0)
        perror("read");
    close(fd);
    printf("xavfsiz saqlash: yangi.tmp yozildi, fsync, rename -> sozlama.txt = \"%.*s\\n\"\n",
           (int)strcspn(buf, "\n"), buf);
    unlink("sozlama.txt");
    return 0;
}
