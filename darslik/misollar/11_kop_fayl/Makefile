CC     := gcc
CFLAGS := -Wall -Wextra -g -MMD -MP
OBJ    := main.o matematika.o

# Birinchi nishon - "make" yolg'iz yozilganda shu yig'iladi
dastur: $(OBJ)
	$(CC) $^ -o $@

# Naqsh qoidasi: har qanday X.o X.c dan ($< - birinchi bog'liqlik, $@ - nishon)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# .h o'zgarsa, uni ishlatgan .c lar qayta yig'iladi (-MMD yaratgan .d fayllar)
-include $(OBJ:.o=.d)

.PHONY: clean
clean:
	rm -f dastur *.o *.d
