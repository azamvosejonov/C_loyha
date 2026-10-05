# Virtual kompyuter — yig'ish va tekshirish
#
#   make               emulyator (build/vk) va firmware (build/firmware.elf)
#   make test          hamma testlar: birlik (aniq xabar bilan), assembly testlari, SBI testi
#   make linux         Linux yadrosini yuklab olib yig'ish (bir marta, ~5-15 daqiqa): build/Image
#   make linux-ishga   Linux ni ishga tushirish (interaktiv shell). Chiqish: shell'da `poweroff`
#   make linux-test    Linux ni skriptli kirish bilan yuklab, natijani tekshirish
#   make bitlar        bitlar bilan ishlash namoyishi (misollar/bitlar.c) — 1-bob
#   make salom         eng kichik 'bare metal' dastur (misollar/salom.c) — 0-bob
#   make toza
#
# Kerak: gcc, clang + lld (RISC-V uchun kross-kompilyatsiya), make. Linux uchun yana: flex bison bc rsync curl

CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
RVCC = clang --target=riscv32 -march=rv32imac_zicsr_zifencei -mabi=ilp32 -mcmodel=medany \
       -ffreestanding -nostdlib -fno-builtin -O2 -Wall -Wextra

EMU_C = $(wildcard emu/*.c)
EMU_H = $(wildcard emu/*.h)
FW = firmware/boshlash.S firmware/trap.S firmware/konsol.c firmware/sbi.c firmware/asosiy.c

all: build/vk build/firmware.elf

build/vk: $(EMU_C) $(EMU_H)
	@mkdir -p build
	$(CC) $(CFLAGS) $(EMU_C) -o $@

build/firmware.elf: $(FW) firmware/fw.h firmware/link.ld
	@mkdir -p build
	$(RVCC) -T firmware/link.ld -fuse-ld=lld $(FW) -o $@

# birlik testlari: emulyator funksiyalarini (main.c dan tashqari) to'g'ridan-to'g'ri chaqiradi
build/birlik: testlar/birlik/birlik.c testlar/birlik/c_juftlar.h $(filter-out emu/main.c,$(EMU_C)) $(EMU_H)
	@mkdir -p build
	$(CC) $(CFLAGS) -Iemu testlar/birlik/birlik.c $(filter-out emu/main.c,$(EMU_C)) -o $@

build/sbi_test.elf: testlar/sbi/sbi_test.S testlar/sbi/link.ld
	@mkdir -p build
	$(RVCC) -T testlar/sbi/link.ld -fuse-ld=lld testlar/sbi/sbi_test.S -o $@

test: build/vk build/firmware.elf build/birlik build/sbi_test.elf
	@echo "== birlik testlari (har mashq funksiyasi alohida) =="
	@./build/birlik testlar/birlik/kutilgan.dtb
	@echo "== assembly testlari (butun protsessor) =="
	@sh testlar/emu/yigish.sh $(CURDIR)/build/vk
	@echo "== firmware (SBI) testi =="
	@sh testlar/sbi/tekshir.sh $(CURDIR)/build/vk $(CURDIR)/build/firmware.elf $(CURDIR)/build/sbi_test.elf

build/salom.elf: misollar/salom.c misollar/link.ld
	@mkdir -p build
	$(RVCC) -T misollar/link.ld -fuse-ld=lld misollar/salom.c -o $@

bitlar: misollar/bitlar.c emu/turlar.h
	@mkdir -p build
	$(CC) $(CFLAGS) -Iemu misollar/bitlar.c -o build/bitlar
	./build/bitlar

salom: build/vk build/salom.elf
	./build/vk -s -n 1000000 build/salom.elf

linux:
	sh linux/yig.sh

linux-ishga: build/vk build/firmware.elf
	@test -f build/Image || { echo "avval: make linux"; exit 1; }
	./build/vk -m 64 -k build/Image build/firmware.elf

linux-test: build/vk build/firmware.elf
	@test -f build/Image || { echo "avval: make linux"; exit 1; }
	@sh testlar/linux/tekshir.sh $(CURDIR)/build/vk $(CURDIR)/build/firmware.elf $(CURDIR)/build/Image

toza:
	rm -rf build/vk build/firmware.elf build/birlik build/sbi_test.elf build/salom.elf build/bitlar testlar/emu/build

.PHONY: all test bitlar salom linux linux-ishga linux-test toza
