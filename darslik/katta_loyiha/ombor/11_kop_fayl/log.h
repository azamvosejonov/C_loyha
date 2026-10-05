/* log.h - umumiy makrolar: LOG (faqat -DDEBUG bilan), ARRAY_SIZE, BIT */
#ifndef LOG_H
#define LOG_H

#include <stdio.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define BIT(n) (1u << (n))

#ifdef DEBUG
#define LOG(fmt, ...) fprintf(stderr, "[LOG %s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define LOG(fmt, ...) do { } while (0)
#endif

#endif
