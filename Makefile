# =============================================================================
#  Makefile - butun loyihani yig'ish (build) qoidalari
# =============================================================================
#
#  NIMA QILADI:
#    `make` buyrug'i shu fayldagi qoidalar bo'yicha:
#      1) kernel/ ichidagi har bir .c va .asm faylni obyekt (.o) faylga aylantiradi
#      2) ularni linker.ld bo'yicha bitta yadro fayliga (build/kernel.elf) bog'laydi
#      3) user/libc dan statik kutubxona (libc.a) va user/bin dagi har bir
#         dasturni alohida ELF qilib yig'adi
#      4) root fayl tizimini (build/rootfs: /bin, /etc ...) tar arxivga
#         (build/initrd.tar) joylaydi - yadro uni boot paytida tmpfs ga ochadi
#      5) GRUB bilan yuklanadigan ISO tasvir yaratadi (build/myos.iso). Uni USB
#         fleshkaga yozib, HAQIQIY kompyuterda yuklash mumkin (docs/real-apparat.md)
#
#  ASOSIY BUYRUQLAR:
#    make              - yadro + dasturlar + yuklanadigan ISO (build/myos.iso)
#    make run          - QEMU oynasida, BIOS rejimida (ekran + serial terminalda)
#    make run-uefi     - QEMU oynasida, UEFI rejimida (OVMF firmware)
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
QEMU    := qemu-system-x86_64
GRUB_MKRESCUE := grub-mkrescue

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
#   -mcmodel=kernel       : Kod va ma'lumot manzilning ENG YUQORI 2 GB ida
#                           (0xFFFFFFFF80000000 dan yuqori). Kompilyator manzillarni
#                           32-bitli ishorali son sifatida kodlaydi - kod ixcham va tez.
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
                 -mno-red-zone -mgeneral-regs-only -mcmodel=kernel \
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
#   * -nostdinc : kompyuterdagi glibc sarlavhalari (/usr/include) ISHLATILMASIN -
#     aks holda <stdio.h> bizniki emas, Linux'niki bo'lib qoladi va dastur
#     noto'g'ri tuzilmalar bilan yig'iladi. Faqat GCC'ning o'z "freestanding"
#     sarlavhalari (stdint.h, stddef.h, stdarg.h, stdbool.h) qoladi (-isystem).
#   * -Iuser/include : bizning libc sarlavhalarimiz (stdio.h, unistd.h ...)
GCC_INCLUDE := $(shell $(CC) -print-file-name=include)
USER_CFLAGS := -std=gnu11 -ffreestanding -fno-stack-protector -fno-pic -fno-pie \
               -mgeneral-regs-only -fno-asynchronous-unwind-tables \
               -fno-tree-loop-distribute-patterns \
               -nostdinc -isystem $(GCC_INCLUDE) -Iuser/include -Iinclude \
               -O2 -g -Wall -Wextra -Werror -MMD -MP
USER_ASFLAGS := -f elf64 -g -F dwarf
USER_LDFLAGS := -T user/linker.ld -nostdlib -z max-page-size=0x1000 -z noexecstack

# ---- Manba fayllarni topish -------------------------------------------------------
# `find` bilan kernel/ ichidagi barcha .c va .asm fayllarni avtomatik yig'amiz.
# Yangi fayl qo'shsangiz Makefile'ni o'zgartirish shart emas.
KERNEL_C   := $(shell find kernel -name '*.c')
KERNEL_ASM := $(shell find kernel -name '*.asm')
# kernel/drivers/vga.c -> build/kernel/drivers/vga.c.o
KERNEL_OBJ := $(patsubst %,$(BUILD)/%.o,$(KERNEL_C) $(KERNEL_ASM))

# Bizning libc: crt0.o (dastur boshlanishi) + libc.a (STATIK KUTUBXONA - .o
# fayllar arxivi). Linker arxivdan faqat KERAKLI .o fayllarni oladi: `true`
# dasturiga printf kodi qo'shilmaydi. glibc'ning libc.a si ham shunday ishlaydi.
LIBC_OBJ := $(patsubst %,$(BUILD)/%.o,$(wildcard user/libc/*.c) \
              $(filter-out user/libc/crt0.asm,$(wildcard user/libc/*.asm)))
CRT0     := $(BUILD)/user/libc/crt0.asm.o
LIBC_A   := $(BUILD)/user/libc.a

# ROOT FAYL TIZIMI build/rootfs/ da yig'iladi va initrd.tar ga aylanadi:
#   user/bin/X.c      -> /bin/X       (dasturlar)
#   rootfs/etc/motd   -> /etc/motd    (rootfs/ dagi oddiy fayllar - o'z joyida)
USER_PROGS := $(patsubst user/bin/%.c,$(BUILD)/rootfs/bin/%,$(wildcard user/bin/*.c))
ROOTFS_FILES := $(patsubst rootfs/%,$(BUILD)/rootfs/%,$(shell find rootfs -type f))
# Bo'sh papkalar (git ularni saqlamaydi).
ROOTFS_DIRS := bin etc home mnt tmp

# ---- QEMU sozlamalari ------------------------------------------------------------
#   -cdrom       : ISO tasvirdan yuklash - xuddi haqiqiy kompyuterdagi kabi GRUB orqali
#   -boot d      : avval CD dan yuklash (diskda ham MBR imzosi bor, lekin yuklovchi kodi yo'q)
#   -m 256M      : 256 MB RAM
#   -smp 2       : 2 ta CPU yadrosi (SMP qo'llab-quvvatlash uchun)
#   -no-reboot   : triple fault bo'lsa qayta yuklanmasdan to'xtash (xatoni ko'rish uchun)
#   -device isa-debug-exit : 0xf4 portiga yozilsa QEMU chiqib ketadi (testlar uchun)
#   -machine     : pc  = i440fx chipset, disk IDE kontrollerda (drivers/ata.c)
#                  q35 = zamonaviyroq chipset, disk SATA/AHCI da (drivers/ahci.c)
#   -drive/-device ide-hd : disk tasviri birinchi portda (/dev/sda). "bus=ide.0"
#                  ikkala mashinada ham ishlaydi (q35 da AHCI portlari ham ide.N deb ataladi)
QEMU_MEM  ?= 256M
QEMU_SMP  ?= 2
MACHINE   ?= pc
DISK      ?= $(BUILD)/disk.img
QEMU_FLAGS := -machine $(MACHINE) -boot d -cdrom $(BUILD)/myos.iso -m $(QEMU_MEM) -smp $(QEMU_SMP) \
              -no-reboot -device isa-debug-exit,iobase=0xf4,iosize=0x04 \
              -drive file=$(DISK),format=raw,if=none,id=disk0 -device ide-hd,drive=disk0,bus=ide.0
# UEFI firmware (Ubuntu: `apt install ovmf`)
OVMF ?= /usr/share/OVMF/OVMF_CODE_4M.fd
UEFI_FLAGS := -drive if=pflash,format=raw,readonly=on,file=$(OVMF)

# Yadroga beriladigan buyruq qatori. Masalan: make run APPEND=selftest
APPEND ?=

# =============================================================================
#  QOIDALAR
# =============================================================================
.PHONY: all disk run run-uefi run-nographic debug test lab-check clean FORCE

# Make pattern zanjiridagi "oraliq" fayllarni (user .o va .elf lari) yig'ishdan keyin
# o'chirib yuboradi. .PRECIOUS ularni saqlab qoladi (gdb uchun .elf kerak), lekin
# .SECONDARY dan farqli ravishda o'chirilgan fayl QAYTA yig'iladi.
.PRECIOUS: $(BUILD)/user/%.o $(BUILD)/user/bin/%.elf

all: $(BUILD)/myos.iso $(BUILD)/disk.img

# ---- Yadroni bog'lash (link) ----
$(BUILD)/kernel.elf: $(KERNEL_OBJ) kernel/linker.ld
	$(call say,LD,$@)
	$(Q)$(LD) $(KERNEL_LDFLAGS) -o $@ $(KERNEL_OBJ)

# ---- Kompilyator bayroqlari o'zgarganini kuzatish ----
# Make faqat fayl vaqtlariga qaraydi: CFLAGS o'zgarsa, eski .o fayllar qoladi
# va ular yangi bayroqlarsiz yig'ilgan bo'ladi (masalan, -mcmodel almashtirilganda
# "relocation truncated" xatosi). Yechim: bayroqlarni faylga yozamiz va har bir
# .o shu faylga bog'liq. Bayroqlar o'zgarsa - fayl o'zgaradi - hammasi qayta yig'iladi.
FLAGS_STAMP := $(BUILD)/.flags
$(FLAGS_STAMP): FORCE
	@mkdir -p $(BUILD)
	@echo '$(KERNEL_CFLAGS) | $(USER_CFLAGS) | $(KERNEL_ASFLAGS)' > $@.tmp
	@cmp -s $@.tmp $@ || mv $@.tmp $@
	@rm -f $@.tmp

# ---- Yadro C fayllari ----
$(BUILD)/kernel/%.c.o: kernel/%.c $(FLAGS_STAMP)
	@mkdir -p $(dir $@)
	$(call say,CC,$<)
	$(Q)$(CC) $(KERNEL_CFLAGS) -c $< -o $@

# ---- Yadro assembly fayllari ----
$(BUILD)/kernel/%.asm.o: kernel/%.asm $(FLAGS_STAMP)
	@mkdir -p $(dir $@)
	$(call say,AS,$<)
	$(Q)$(AS) $(KERNEL_ASFLAGS) $< -o $@

# ---- User C fayllari ----
$(BUILD)/user/%.c.o: user/%.c $(FLAGS_STAMP)
	@mkdir -p $(dir $@)
	$(call say,CC,$<)
	$(Q)$(CC) $(USER_CFLAGS) -c $< -o $@

# ---- User assembly fayllari ----
$(BUILD)/user/%.asm.o: user/%.asm $(FLAGS_STAMP)
	@mkdir -p $(dir $@)
	$(call say,AS,$<)
	$(Q)$(AS) $(USER_ASFLAGS) $< -o $@

# ---- libc.a: ar (archiver) - .o fayllarni bitta arxivga yig'adi ----
#   r - qo'shish/almashtirish, c - arxivni yaratish, s - belgilar indeksi (linker tez topsin)
$(LIBC_A): $(LIBC_OBJ)
	$(call say,AR,$@)
	$(Q)rm -f $@ && ar rcs $@ $^

# ---- Har bir user dasturi: crt0.o + o'z .o fayli + libc.a ----
# To'liq ELF (debug ma'lumoti bilan, gdb uchun) build/user/bin/X.elf da qoladi.
# Diskka esa --strip-debug qilingan nusxa ketadi: ~4 barobar kichik.
# TARTIB MUHIM: libc.a eng oxirida - linker arxivdan faqat OLDINGI fayllarda
# yetishmayotgan belgilarni qidiradi.
$(BUILD)/user/bin/%.elf: $(BUILD)/user/bin/%.c.o $(CRT0) $(LIBC_A) user/linker.ld
	$(call say,LD,$@)
	$(Q)$(LD) $(USER_LDFLAGS) -o $@ $(CRT0) $< $(LIBC_A)

$(BUILD)/rootfs/bin/%: $(BUILD)/user/bin/%.elf
	@mkdir -p $(dir $@)
	$(Q)objcopy --strip-debug $< $@

# ---- rootfs/ dagi oddiy fayllarni nusxalash ----
$(BUILD)/rootfs/%: rootfs/%
	@mkdir -p $(dir $@)
	$(Q)cp $< $@

# ---- Boshlang'ich root fayl tizimi: tar arxiv (USTAR formati) ----
# Yadro (fs/initrd.c) uni boot paytida tmpfs ildiziga ochadi.
# --format=ustar   : eng oddiy, yaxshi hujjatlashtirilgan tar formati
# --owner/--group  : fayl egasi - root (0), kompyuterdagi foydalanuvchimiz emas
# --sort=name      : arxiv har safar bir xil bo'lsin (takrorlanuvchan yig'ish)
# -C dir + ro'yxat : nomlar "./" siz saqlansin ("bin/sh", "./bin/sh" emas)
$(BUILD)/initrd.tar: $(USER_PROGS) $(ROOTFS_FILES)
	@mkdir -p $(addprefix $(BUILD)/rootfs/,$(ROOTFS_DIRS))
	$(call say,TAR,$@)
	$(Q)tar --format=ustar --owner=0 --group=0 --sort=name -cf $@ \
		-C $(BUILD)/rootfs $$(ls $(BUILD)/rootfs)

# ---- GRUB konfiguratsiyasi ----
# APPEND o'zgarsa, grub.cfg ham o'zgarishi kerak. FORCE + "faqat farq qilsa yozish"
# hiylasi: fayl mazmuni o'zgarmasa, uning vaqti ham o'zgarmaydi va ISO qayta yig'ilmaydi.
$(BUILD)/iso/boot/grub/grub.cfg: FORCE
	@mkdir -p $(dir $@)
	@printf '%s\n' \
		'# Avtomatik yaratilgan (Makefile). all_video - UEFI (GOP) va BIOS (VBE)' \
		'# video drayverlari: ularsiz UEFI kompyuterda framebuffer bo'"'"'lmaydi.' \
		'insmod all_video' \
		'set timeout=0' \
		'set default=0' \
		'menuentry "MyOS" {' \
		'    multiboot2 /boot/kernel.elf $(APPEND)' \
		'    module2 /boot/initrd.tar initrd' \
		'    boot' \
		'}' > $@.tmp
	@cmp -s $@.tmp $@ || mv $@.tmp $@
	@rm -f $@.tmp

# ---- Yuklanadigan ISO (BIOS + UEFI) ----
# grub-mkrescue ISO ichiga BIOS uchun (El Torito) va UEFI uchun (EFI tizim
# bo'limi) yuklovchilarni qo'yadi. Bitta fayl ikkala turdagi kompyuterda ishlaydi.
$(BUILD)/myos.iso: $(BUILD)/kernel.elf $(BUILD)/initrd.tar $(BUILD)/iso/boot/grub/grub.cfg
	$(call say,ISO,$@)
	$(Q)cp $(BUILD)/kernel.elf $(BUILD)/initrd.tar $(BUILD)/iso/boot/
	$(Q)$(GRUB_MKRESCUE) -o $@ $(BUILD)/iso > $(BUILD)/grub-mkrescue.log 2>&1 || \
		(cat $(BUILD)/grub-mkrescue.log; false)

# ---- Disk tasviri: MBR + ext2 (tools/mkdisk.py) ----
# Ichida: docs/ (hujjatlarni tizim ichida o'qish uchun) va test fayllari.
# DIQQAT: tizim diskka YOZADI. Toza diskni qayta yaratish: make disk
DISK_SIZE_MB ?= 64
DISK_BLOCK   ?= 1024
$(BUILD)/disk.img: tools/mkdisk.py $(wildcard docs/*.md)
	@rm -rf $(BUILD)/diskroot && mkdir -p $(BUILD)/diskroot/test $(BUILD)/diskroot/lost+found
	@cp -r docs $(BUILD)/diskroot/docs
	@echo "Salom, men ext2 diskdaman!" > $(BUILD)/diskroot/test/salom.txt
	@python3 -c "import sys; sys.stdout.buffer.write(bytes((i * 7 + i // 4096) & 255 for i in range(300000)))" \
		> $(BUILD)/diskroot/test/katta.bin
	@ln -s ../docs/12-vfs.md $(BUILD)/diskroot/test/havola
	$(Q)python3 tools/mkdisk.py $@ $(DISK_SIZE_MB) $(BUILD)/diskroot $(DISK_BLOCK)

disk:
	@rm -f $(BUILD)/disk.img
	@$(MAKE) -s $(BUILD)/disk.img

# ---- Ishga tushirish ----
run: all $(DISK)
	$(QEMU) $(QEMU_FLAGS) -serial stdio

run-uefi: all $(DISK)
	$(QEMU) $(QEMU_FLAGS) $(UEFI_FLAGS) -serial stdio

run-nographic: all $(DISK)
	$(QEMU) $(QEMU_FLAGS) -nographic

# -s : GDB serverini :1234 portida ochish, -S : birinchi instruksiyadan oldin to'xtab turish.
# Boshqa terminalda:  gdb -x tools/gdbinit
debug: all $(DISK)
	$(QEMU) $(QEMU_FLAGS) -nographic -s -S

test:
	./tools/test.sh bios
	./tools/test.sh uefi
	@$(MAKE) -s APPEND=    # oddiy ISO ni tiklash

# Laboratoriyalar (labs/README.md): har bir "bo'sh" variant kompilyatsiya bo'lishini
# va libc'ning host testi o'tishini tekshirish. CI ham shuni ishga tushiradi.
lab-check:
	@tools/host_libctest.sh
	@tools/lab.py selfcheck

clean:
	rm -rf $(BUILD)

# .d fayllarni ulash (sarlavha bog'liqliklari). "-" = fayl bo'lmasa xato bermaslik.
-include $(shell find $(BUILD) -name '*.d' 2>/dev/null)
