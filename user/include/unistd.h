/* unistd.h - POSIX tizim chaqiruvlari */
#pragma once
#include <stddef.h>
#include <stdint.h>
#include "myos/abi.h"

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

typedef long ssize_t;
typedef long off_t;
typedef int pid_t;

ssize_t read(int fd, void *buf, size_t len);
ssize_t write(int fd, const void *buf, size_t len);
int close(int fd);
off_t lseek(int fd, off_t off, int whence);
int dup(int fd);
int dup2(int oldfd, int newfd);
int pipe(int fds[2]);
pid_t fork(void);
int execv(const char *path, char *const argv[]);
pid_t getpid(void);
pid_t getppid(void);
int chdir(const char *path);
char *getcwd(char *buf, size_t size);
int unlink(const char *path);
int rmdir(const char *path);
void *sbrk(long increment);
unsigned sleep(unsigned seconds);
int usleep(unsigned long usec);
void sync(void);
int ftruncate(int fd, off_t size);
__attribute__((noreturn)) void _exit(int code);
int isatty(int fd);
