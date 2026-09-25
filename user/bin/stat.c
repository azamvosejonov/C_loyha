/* user/bin/stat.c - inode haqidagi hamma ma'lumot (struct stat maydonlari). */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

static const char *type_name(unsigned mode)
{
    switch (mode & S_IFMT) {
    case S_IFREG:  return "oddiy fayl";
    case S_IFDIR:  return "papka";
    case S_IFCHR:  return "belgili qurilma";
    case S_IFBLK:  return "blokli qurilma";
    case S_IFIFO:  return "pipe (FIFO)";
    case S_IFLNK:  return "ramziy havola";
    default:       return "noma'lum";
    }
}

static void print_time(const char *label, uint64_t t)
{
    struct tm tm;
    time_t tt = (time_t)t;
    gmtime_r(&tt, &tm);
    printf("%s %04d-%02d-%02d %02d:%02d:%02d UTC\n", label, tm.tm_year + 1900, tm.tm_mon + 1,
           tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "ishlatish: stat <yo'l>...\n");
        return 1;
    }
    int status = 0;
    for (int i = 1; i < argc; i++) {
        struct stat st;
        if (stat(argv[i], &st) < 0) {
            fprintf(stderr, "stat: %s: %s\n", argv[i], strerror(errno));
            status = 1;
            continue;
        }
        printf("  Fayl: %s\n", argv[i]);
        printf(" Hajmi: %-10lu Bloklar: %-6lu Tur: %s\n", st.st_size, st.st_blocks,
               type_name(st.st_mode));
        printf("Qurilma: %u:%u  Inode: %-8lu Havolalar: %u", st.st_dev >> 20,
               st.st_dev & 0xFFFFF, st.st_ino, st.st_nlink);
        if (S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode))
            printf("  Qurilma turi: %u,%u", st.st_rdev >> 20, st.st_rdev & 0xFFFFF);
        printf("\nRuxsat: %04o  Uid: %u  Gid: %u\n", st.st_mode & 07777, st.st_uid, st.st_gid);
        print_time("Kirish:   ", st.st_atime);
        print_time("O'zgarish:", st.st_mtime);
        print_time("Holat:    ", st.st_ctime);
    }
    return status;
}
