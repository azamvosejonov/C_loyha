/* tests/host/lab_prefix.h - host testida bizning libc funksiyalarimiz "lab_" prefiksi
 * bilan chaqiriladi (objcopy --prefix-symbols=lab_), shunda ular kompyuterdagi
 * glibc funksiyalari bilan to'qnashmaydi. printf/puts esa glibc'niki bo'lib qoladi. */
#define strlen lab_strlen
#define strnlen lab_strnlen
#define memmove lab_memmove
#define memcmp lab_memcmp
#define memset lab_memset
#define memcpy lab_memcpy
#define strcmp lab_strcmp
#define strcpy lab_strcpy
#define strtok_r lab_strtok_r
#define strtoul lab_strtoul
#define strtol lab_strtol
#define atoi lab_atoi
#define gmtime_r lab_gmtime_r
#define snprintf lab_snprintf
#define qsort lab_qsort
#define errno lab_errno
#define HOST_TEST 1
