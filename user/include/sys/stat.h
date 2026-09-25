/* sys/stat.h - fayl haqida ma'lumot (stat) va papka yaratish */
#pragma once
#include "myos/abi.h"

/* POSIX nomi: struct stat. Yadro ABI'sida u struct myos_stat deb ataladi,
 * maydonlari (st_mode, st_size ...) aynan bir xil. Makro ikkalasini bog'laydi:
 * `struct stat st; stat("/a", &st);` -> `struct myos_stat st; myos_stat(...)`.
 * (struct teglari va funksiya nomlari C da alohida "nomlar fazosida" - to'qnashmaydi.) */
#define stat myos_stat

int stat(const char *path, struct stat *st);
int fstat(int fd, struct stat *st);
int mkdir(const char *path, unsigned mode);
