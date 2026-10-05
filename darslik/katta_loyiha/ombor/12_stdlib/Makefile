# Makefile - Ombor: qaysi fayl nimaga bog'liq va qanday yig'iladi
CC = gcc
CFLAGS = -Wall -Wextra -g
OBJ = main.o ombor.o pul.o

ombor: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -MMD -c $< -o $@

-include $(OBJ:.o=.d)

libombor.a: ombor.o pul.o
	ar rcs $@ $^

debug: CFLAGS += -fsanitize=address,undefined -DDEBUG
debug: clean ombor

run: ombor
	./ombor < kirish.txt

clean:
	rm -f *.o *.d ombor libombor.a ombor.txt

.PHONY: debug run clean
