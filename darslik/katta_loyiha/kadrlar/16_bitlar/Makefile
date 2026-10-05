# Kadrlar tizimi, 16-bosqich. Buyruqlar:  make   |  make fuzz   |  make tsan  |  make toza
CC = gcc
# -MMD: har .c uchun .d fayl (qaysi .h ga bog'liqligi) hosil qilinadi
CFLAGS = -Wall -Wextra -g -Iinclude -MMD -MP -pthread

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

# fuzzer: parserni sanitizer bilan kompilyatsiya qilib, minglab buzilgan qatorni yuboradi
FUZZ_MANBA = fuzz/fuzz.c src/yukla.c src/ombor.c src/vaqt.c src/toifa.c
FUZZ_BAYROQ = -Wall -Wextra -g -O1 -Iinclude -fsanitize=address,undefined

fuzz: bin/fuzz_xavfsiz bin/fuzz_zaif

bin/fuzz_xavfsiz: $(FUZZ_MANBA)
	@mkdir -p bin
	$(CC) $(FUZZ_BAYROQ) $(FUZZ_MANBA) -o $@

bin/fuzz_zaif: $(FUZZ_MANBA) fuzz/parser_zaif.c
	@mkdir -p bin
	$(CC) $(FUZZ_BAYROQ) -DZAIF fuzz/fuzz.c fuzz/parser_zaif.c src/yukla.c src/ombor.c src/vaqt.c src/toifa.c -o $@

# ThreadSanitizer bilan alohida yig'ish: poygalarni aniqlaydi (ASan bilan BIRGA ishlatib bo'lmaydi)
bin/kadrlar_tsan: $(MANBA)
	@mkdir -p bin
	$(CC) -Wall -Wextra -g -O1 -Iinclude -pthread -fsanitize=thread $^ -o $@

tsan: bin/kadrlar_tsan

toza:
	rm -rf build bin libkadr.a oxirgi_kirish.txt

# .d fayllar bo'lsa, ularni o'qiydi (sarlavha o'zgarsa - tegishli .c qayta kompilyatsiya qilinadi)
-include $(OBJ:.o=.d)
.PHONY: toza fuzz tsan
