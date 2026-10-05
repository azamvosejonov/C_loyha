# Kadrlar tizimi, 11-bosqich. Buyruqlar:  make   |  make toza
CC = gcc
# -MMD: har .c uchun .d fayl (qaysi .h ga bog'liqligi) hosil qilinadi
CFLAGS = -Wall -Wextra -g -Iinclude -MMD -MP

MANBA = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c,build/%.o,$(MANBA))
# main.o dan tashqari hammasi kutubxonaga kiradi
KUTUBXONA_OBJ = $(filter-out build/main.o,$(OBJ))

bin/kadrlar: build/main.o libkadr.a
	@mkdir -p bin
	$(CC) $(CFLAGS) build/main.o libkadr.a -o $@

# statik kutubxona = .o fayllar arxivi
libkadr.a: $(KUTUBXONA_OBJ)
	ar rcs $@ $^

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

toza:
	rm -rf build bin libkadr.a

# .d fayllar bo'lsa, ularni o'qiydi (sarlavha o'zgarsa - tegishli .c qayta kompilyatsiya qilinadi)
-include $(OBJ:.o=.d)
.PHONY: toza
