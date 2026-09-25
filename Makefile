# =============================================================================
#  Makefile - butun loyihani yig'ish (build) qoidalari
# =============================================================================
#
#  NIMA QILADI:
#    `make` buyrug'i shu fayldagi qoidalar bo'yicha:
#      1) kernel/ ichidagi har bir .c va .asm faylni obyekt (.o) faylga aylantiradi
#      2) ularni linker.ld bo'yicha bitta yadro fayliga (build/kernel.elf) bog'laydi
#      3) user/ ichidagi har bir dasturni alohida ELF qilib yig'adi
#      4) dasturlarni bitta tar arxivga (build/initrd.tar) joylaydi - bu bizning
#         "disk"imiz bo'ladi
#
#  ASOSIY BUYRUQLAR:
#    make              - hammasini yig'ish
#    make run          - QEMU oynasida ishga tushirish (VGA ekran + serial terminalda)
#    make run-nographic- faqat terminalda ishga tushirish (oyna yo'q). Chiqish: Ctrl-A, keyin X
#    make debug        - QEMU'ni GDB kutadigan holatda ishga tushirish
#    make test         - avtomatik testlar (CI ham shuni ishlatadi)
#    make clean        - build/ papkasini o'chirish
#    make V=1          - kompilyatsiya buyruqlarini to'liq ko'rsatish
#
#  NEGA MAKE:
#    Make faqat O'ZGARGAN fayllarni qayta kompilyatsiya qiladi. Katta loyihada
#    bu juda muhim: bitta faylni o'zgartirsangiz, faqat o'sha fayl qayta yig'iladi.
# =============================================================================

# ---- Vositalar ---------------------------------------------------------------
# Biz alohida "cross-compiler" o'rniga tizimdagi oddiy gcc'dan foydalanamiz.
# Bu mumkin, chunki host ham x86-64. Lekin gcc'ga maxsus bayroqlar berib,
# uning standart kutubxonasi (glibc) va Linux'ga xos narsalarni o'chiramiz.
# (Jiddiy OS loyihalarda x86_64-elf-gcc cross-compiler ishlatiladi -
#  docs/00-kirish.md ga qarang.)
CC      := gcc
AS      := nasm
LD      := ld
OBJCOPY := objcopy
QEMU    := qemu-system-x86_64

# Hamma yig'ilgan narsa shu papkaga tushadi (manba kodni iflos qilmaslik uchun).
BUILD := build

# Ixcham chiqish: sukut bo'yicha faqat "  CC  kernel/main.c" ko'rinadi.
# To'liq buyruqlarni ko'rish uchun:  make V=1
ifeq ($(V),1)
  Q :=
  say = @true
else
  Q := @
  say = @echo "  $(1)	$(2)"
endif

# ---- Yadro uchun kompilyator bayroqlari -----------------------------------------
# Har bir bayroq NEGA kerakligi:
#   -std=gnu11            : C11 standarti + GNU kengaytmalari (inline asm uchun kerak)
#   -ffreestanding        : "Bu yerda OS yo'q" degani. Kompilyator printf, malloc
#                           kabi kutubxona borligini taxmin qilmaydi.
#   -fno-stack-protector  : Stack himoyasi glibc'dagi __stack_chk_fail ga tayanadi.
#                           Bizda glibc yo'q.
#   -fno-pic -fno-pie     : Ubuntu gcc sukut bo'yicha "joyi o'zgaruvchan" kod yaratadi.
#                           Yadro aniq manzilda (1 MB) turadi, bizga bu kerak emas.
#   -mno-red-zone         : ENG MUHIMI! x86-64 ABI funksiyaga stack ostidagi 128 baytni
#                           (red zone) so'ramasdan ishlatishga ruxsat beradi. Lekin
#                           uzilish (interrupt) aynan o'sha joyga yozadi va ma'lumotni
#                           buzadi. Yadroda red zone bo'lishi mumkin emas.
#   -mgeneral-regs-only   : SSE/AVX registrlarini ishlatmaslik. Aks holda har bir
#                           uzilishda ularni ham saqlashimiz kerak bo'lardi.
#   -mcmodel=small        : Kod va ma'lumot 2 GB dan past manzilda. Bizning yadro 1 MB da.
#   -fno-omit-frame-pointer : rbp registrini har doim stack freym uchun saqlash.
#                           Shunda panic() paytida "backtrace" chiqarib bera olamiz.
#   -fno-asynchronous-unwind-tables : .eh_frame (C++ exception'lar uchun) kerak emas.
#   -fno-tree-loop-distribute-patterns : gcc bizning memset() tsiklimizni ko'rib,
#                           uni "memset() chaqiruvi"ga aylantirib qo'yishi mumkin -
#                           bu cheksiz rekursiya bo'ladi. Shuni taqiqlaymiz.
#   -O2 -g                : optimizatsiya + debug ma'lumotlari (GDB uchun).
#   -Wall -Wextra -Werror : hamma ogohlantirishlar yoqilgan va ular XATO hisoblanadi.
#                           Kuchli muhandislar ogohlantirishlarni hech qachon e'tiborsiz
#                           qoldirmaydi.
#   -Ikernel              : #include "mm/pmm.h" kabi yo'llar kernel/ dan boshlanadi.
#   -Iinclude             : yadro va user dasturlari UMUMIY sarlavhalari (myos/abi.h).
#   -MMD -MP              : har bir .c uchun .d fayl (qaysi .h larga bog'liqligi) yaratadi,
#                           shunda .h o'zgarsa, kerakli .c lar qayta yig'iladi.
KERNEL_CFLAGS := -std=gnu11 -ffreestanding -fno-stack-protector -fno-pic -fno-pie \
                 -mno-red-zone -mgeneral-regs-only -mcmodel=small \
                 -fno-omit-frame-pointer -fno-asynchronous-unwind-tables \
                 -fno-tree-loop-distribute-patterns \
                 -O2 -g -Wall -Wextra -Werror -Ikernel -Iinclude -MMD -MP

# NASM bayroqlari: -f elf64 = 64-bitli ELF obyekt fayl, -g -F dwarf = debug ma'lumot.
KERNEL_ASFLAGS := -f elf64 -g -F dwarf

# Linker bayroqlari:
#   -T kernel/linker.ld : bo'limlarni qaysi manzilga qo'yishni linker skriptimiz aytadi
#   -nostdlib           : hech qanday standart kutubxonani ulamaslik
#   -z max-page-size=0x1000 : ELF segmentlarini 4 KB ga tekislash (sukut 2 MB - fayl
#                         juda katta bo'lib ketadi va multiboot sarlavhasi 8 KB dan
#                         uzoqqa tushib qolishi mumkin)
#   -z noexecstack      : "stack bajariladimi?" degan ogohlantirishni o'chiradi
KERNEL_LDFLAGS := -T kernel/linker.ld -nostdlib -z max-page-size=0x1000 -z noexecstack

# ---- Foydalanuvchi (user mode) dasturlari uchun bayroqlar ------------------------
# Yadro bayroqlariga o'xshash, lekin:
#   * red zone MUMKIN - user dasturda uzilish kelsa, CPU avtomatik ravishda yadro
#     stekiga o'tadi (TSS.rsp0), shuning uchun user stekiga tegilmaydi.
#   * -Iuser/lib : user kutubxonamiz sarlavhalari
USER_CFLAGS := -std=gnu11 -ffreestanding -fno-stack-protector -fno-pic -fno-pie \
               -mgeneral-regs-only -fno-asynchronous-unwind-tables \
               -fno-tree-loop-distribute-patterns \
               -O2 -g -Wall -Wextra -Werror -Iuser/lib -Iinclude -MMD -MP
USER_ASFLAGS := -f elf64 -g -F dwarf
USER_LDFLAGS := -T user/linker.ld -nostdlib -z max-page-size=0x1000 -z noexecstack

# ---- Manba fayllarni topish -------------------------------------------------------
# `find` bilan kernel/ ichidagi barcha .c va .asm fayllarni avtomatik yig'amiz.
# Yangi fayl qo'shsangiz Makefile'ni o'zgartirish shart emas.
KERNEL_C   := $(shell find kernel -name '*.c')
KERNEL_ASM := $(shell find kernel -name '*.asm')
# kernel/drivers/vga.c -> build/kernel/drivers/vga.c.o
KERNEL_OBJ := $(patsubst %,$(BUILD)/%.o,$(KERNEL_C) $(KERNEL_ASM))

# User kutubxonasi (har bir dasturga ulanadi): crt0.asm + boshqa .c fayllar.
ULIB_C   := $(wildcard user/lib/*.c)
ULIB_ASM := $(wildcard user/lib/*.asm)
ULIB_OBJ := $(patsubst %,$(BUILD)/%.o,$(ULIB_C) $(ULIB_ASM))

# Har bir user/bin/X.c -> build/initrd/X (alohida bajariladigan fayl).
USER_PROGS := $(patsubst user/bin/%.c,$(BUILD)/initrd/%,$(wildcard user/bin/*.c))

# initrd/ papkasidagi oddiy fayllar (masalan, README.txt) ham diskka qo'shiladi.
INITRD_EXTRA := $(patsubst initrd/%,$(BUILD)/initrd/%,$(wildcard initrd/*))

# ---- QEMU sozlamalari ------------------------------------------------------------
#   -kernel      : QEMU'ning ichki Multiboot yuklovchisi yadroni to'g'ridan-to'g'ri yuklaydi
#                  (GRUB kerak emas). U faqat 32-bitli ELF qabul qiladi, shuning uchun
#                  kernel32.elf ni beramiz (pastga qarang).
#   -initrd      : Multiboot "modul" sifatida initrd.tar ni xotiraga yuklaydi
#   -m 128M      : 128 MB RAM
#   -no-reboot   : triple fault bo'lsa qayta yuklanmasdan to'xtash (xatoni ko'rish uchun)
#   -device isa-debug-exit : 0xf4 portiga yozilsa QEMU chiqib ketadi (testlar va
#                  `shutdown` buyrug'i uchun)
QEMU_FLAGS := -kernel $(BUILD)/kernel32.elf -initrd $(BUILD)/initrd.tar \
              -m 128M -no-reboot \
              -device isa-debug-exit,iobase=0xf4,iosize=0x04

# Yadroga beriladigan buyruq qatori (Multiboot cmdline). Masalan: make run APPEND=selftest
APPEND ?=

# =============================================================================
#  QOIDALAR
# =============================================================================
.PHONY: all run run-nographic debug test clean

# Make zanjirdagi "oraliq" fayllarni (user .o lari) avtomatik o'chirib yuboradi.
# .SECONDARY ularni saqlab qoladi - keyingi `make` hech narsani qayta yig'maydi.
.SECONDARY:

all: $(BUILD)/kernel32.elf $(BUILD)/initrd.tar

# ---- Yadroni bog'lash (link) ----
$(BUILD)/kernel.elf: $(KERNEL_OBJ) kernel/linker.ld
	$(call say,LD,$@)
	$(Q)$(LD) $(KERNEL_LDFLAGS) -o $@ $(KERNEL_OBJ)

# QEMU -kernel faqat 32-bitli ELF'ni tushunadi. objcopy faqat ELF SARLAVHASINI
# 32-bitga o'zgartiradi, ichidagi baytlar (64-bitli kod) o'zgarmaydi. Bizning
# boot.asm baribir 32-bitli rejimda boshlanadi, keyin o'zi 64-bitga o'tadi.
# kernel.elf (64-bit) esa GDB uchun saqlanib qoladi - unda hamma simvollar bor.
$(BUILD)/kernel32.elf: $(BUILD)/kernel.elf
	$(call say,OBJCOPY,$@)
	$(Q)$(OBJCOPY) -O elf32-i386 $< $@

# ---- Yadro C fayllari ----
$(BUILD)/kernel/%.c.o: kernel/%.c
	@mkdir -p $(dir $@)
	$(call say,CC,$<)
	$(Q)$(CC) $(KERNEL_CFLAGS) -c $< -o $@

# ---- Yadro assembly fayllari ----
$(BUILD)/kernel/%.asm.o: kernel/%.asm
	@mkdir -p $(dir $@)
	$(call say,AS,$<)
	$(Q)$(AS) $(KERNEL_ASFLAGS) $< -o $@

# ---- User C fayllari ----
$(BUILD)/user/%.c.o: user/%.c
	@mkdir -p $(dir $@)
	$(call say,CC,$<)
	$(Q)$(CC) $(USER_CFLAGS) -c $< -o $@

# ---- User assembly fayllari ----
$(BUILD)/user/%.asm.o: user/%.asm
	@mkdir -p $(dir $@)
	$(call say,AS,$<)
	$(Q)$(AS) $(USER_ASFLAGS) $< -o $@

# ---- Har bir user dasturi: o'z .o fayli + user kutubxonasi ----
$(BUILD)/initrd/%: $(BUILD)/user/bin/%.c.o $(ULIB_OBJ) user/linker.ld
	@mkdir -p $(dir $@)
	$(call say,LD,$@)
	$(Q)$(LD) $(USER_LDFLAGS) -o $@ $< $(ULIB_OBJ)

# ---- initrd/ dagi oddiy fayllarni nusxalash ----
$(BUILD)/initrd/%: initrd/%
	@mkdir -p $(dir $@)
	$(Q)cp $< $@

# ---- Disk tasviri: oddiy tar arxiv (USTAR formati) ----
# --format=ustar : eng oddiy, yaxshi hujjatlashtirilgan tar formati. fs/tarfs.c uni o'qiydi.
# -C dir         : fayl nomlari "./" siz saqlanishi uchun papka ichidan arxivlaymiz.
$(BUILD)/initrd.tar: $(USER_PROGS) $(INITRD_EXTRA)
	@mkdir -p $(BUILD)/initrd
	$(call say,TAR,$@)
	$(Q)tar --format=ustar -cf $@ -C $(BUILD)/initrd $(notdir $(USER_PROGS) $(INITRD_EXTRA))

# ---- Ishga tushirish ----
run: all
	$(QEMU) $(QEMU_FLAGS) -serial stdio -append "$(APPEND)"

run-nographic: all
	$(QEMU) $(QEMU_FLAGS) -nographic -append "$(APPEND)"

# -s : GDB serverini :1234 portida ochish, -S : birinchi instruksiyadan oldin to'xtab turish.
# Boshqa terminalda:  gdb build/kernel.elf -ex "target remote :1234"
debug: all
	$(QEMU) $(QEMU_FLAGS) -nographic -s -S -append "$(APPEND)"

test: all
	./tools/test.sh

clean:
	rm -rf $(BUILD)

# .d fayllarni ulash (sarlavha bog'liqliklari). "-" = fayl bo'lmasa xato bermaslik.
-include $(shell find $(BUILD) -name '*.d' 2>/dev/null)
