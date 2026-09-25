/* dirent.h - papka ichidagi yozuvlarni o'qish (opendir/readdir) */
#pragma once
#include "myos/abi.h"

#define dirent myos_dirent              /* struct dirent == struct myos_dirent (d_name, d_type) */

#define DT_UNKNOWN 0
typedef struct DIR DIR;

DIR *opendir(const char *path);
struct dirent *readdir(DIR *d);
int closedir(DIR *d);
