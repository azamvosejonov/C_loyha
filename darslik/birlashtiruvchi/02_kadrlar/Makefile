# Kadrlar tizimi - yig'ish va tekshirish
#   make            - dasturni yig'adi:  bin/kadrlar
#   make test       - har TODO uchun avtomatik testlar (qaysi funksiya tayyor, qaysi yo'q)
#   make tekshir    - testlar + butun dasturni kutilgan natija bilan solishtirish
#   make run ARGS="royxat"
#   make toza       - yig'ish natijalarini o'chiradi

CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude

MANBA = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c,build/%.o,$(MANBA))
OBJ_TESTLAR_UCHUN = $(filter-out build/main.o,$(OBJ))      # testlar o'zining main() iga ega
TESTLAR = $(addprefix bin/test_,pul vaqt matn soliq ombor hisobot)     # T-raqamlar tartibida

all: bin/kadrlar

bin/kadrlar: $(OBJ)
	@mkdir -p bin
	$(CC) $(CFLAGS) $^ -o $@

build/%.o: src/%.c $(wildcard include/*.h)
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

bin/test_%: tests/test_%.c tests/test.h $(OBJ_TESTLAR_UCHUN)
	@mkdir -p bin
	@$(CC) $(CFLAGS) -Itests $< $(OBJ_TESTLAR_UCHUN) -o $@

test: $(TESTLAR)
	@fail=0; for t in $(TESTLAR); do ./$$t || fail=1; done; \
	if [ $$fail -eq 0 ]; then echo "HAMMA TESTLAR O'TDI"; else echo "ba'zi testlar o'tmadi (yuqoridagi [XATO] qatorlarga qarang)"; fi; exit $$fail

tekshir: bin/kadrlar test
	@sh tests/butun.sh

run: bin/kadrlar
	@./bin/kadrlar $(ARGS)

toza:
	rm -rf build bin

.PHONY: all test tekshir run toza
