#!/usr/bin/env bash
# =============================================================================
#  tools/test.sh - to'liq avtomatik test (make test va GitHub Actions CI)
# =============================================================================
#
#  NIMA QILADI:
#    1. QEMU'ni oynasiz (-display none) ishga tushiradi. Yadro buyruq qatorida
#       "selftest" bor, shuning uchun avval yadro ichki testlari ishlaydi.
#    2. Serial port orqali shell'ga buyruqlarni "yozadi" (xuddi odam
#       klaviaturada yozgandek) - har bir buyruq orasida pauza bilan.
#    3. Butun chiqishni faylga yozadi va unda kutilgan satrlarni qidiradi.
#    4. Oxirida `poweroff` bilan QEMU ni yopadi.
#
#  NEGA BU MUHIM:
#    Har bir o'zgarishdan keyin "hamma narsa hali ham ishlaydimi?" savoliga
#    10 soniyada javob olasiz. Kuchli jamoalarda hech bir kod testlardan
#    o'tmasdan asosiy branch'ga tushmaydi (CI - Continuous Integration).
# =============================================================================
set -u

cd "$(dirname "$0")/.."
QEMU=${QEMU:-qemu-system-x86_64}
MODE=${1:-bios}                     # bios | uefi
LOG=build/test-output-$MODE.log
OVMF=${OVMF:-/usr/share/OVMF/OVMF_CODE_4M.fd}

# Yadro buyruq qatorida "selftest" bo'lgan ISO ni yig'amiz.
make -s APPEND=selftest || exit 1

# Har bir test TOZA disk nusxasida ishlaydi (tizim diskka yozadi).
# BIOS testi: "pc" mashinasi - disk IDE kontrollerda (drivers/ata.c, PIO).
# UEFI testi: "q35" mashinasi - disk SATA/AHCI da (drivers/ahci.c, DMA).
DISK=build/test-disk-$MODE.img
cp build/disk.img "$DISK" || exit 1
MACHINE=pc
EXTRA=()
if [ "$MODE" = uefi ]; then
    MACHINE=q35
    EXTRA=(-drive "if=pflash,format=raw,readonly=on,file=$OVMF")
fi

# Shell'ga yuboriladigan buyruqlar. Har biri alohida qatorda.
COMMANDS=(
    "hello birinchi ikkinchi"
    "ls /mnt/test"
    "cksum /mnt/test/katta.bin"
    "fstest /mnt"
    "mkdir /mnt/yangi && seq 30000 > /mnt/yangi/seq.txt && mv /mnt/test/salom.txt /mnt/yangi/"
    "ls /"
    "cat /README.txt"
    "echo salom dunyo"
    "fstest"
    "ls /bin | wc -l > /tmp/n ; cat /tmp/n"
    "seq 20000 | grep 7 | tail -2"
    "X=MyOS ; false || echo \"yiqildi: \$? \$X\""
    "mkdir -p /home/a/b && cd /home/a/b && pwd && cd /"
    "memtest"
    "forktest"
    "crash null"
    "crash kernel"
    "crash cli"
    "crash stack"
    "mavjud_emas"
    "spin fon 3 &"
    "ps"
    "free"
    "lspci"
    "uname"
    "date"
    "poweroff"
)

# Buyruqlarni pauzalar bilan yuboruvchi funksiya. Yadro bootlanishi va
# selftest'lar tugashi uchun boshida ko'proq kutamiz.
feed_commands() {
    sleep 6
    for cmd in "${COMMANDS[@]}"; do
        printf '%s\n' "$cmd"
        sleep 1
    done
}

echo "==> QEMU ($MODE) ishga tushirilmoqda (log: $LOG)"
feed_commands | timeout 180 "$QEMU" \
    -machine $MACHINE -boot d -cdrom build/myos.iso -m 256M -smp 2 -no-reboot \
    -display none -serial stdio \
    -drive "file=$DISK,format=raw,if=none,id=disk0" -device ide-hd,drive=disk0,bus=ide.0 \
    "${EXTRA[@]}" > "$LOG" 2>&1
QEMU_STATUS=$?
# Serial port qatorlarni "\r\n" bilan tugatadi - '$' bilan tekshirish uchun \r ni olib tashlaymiz.
tr -d '\r' < "$LOG" > "$LOG.tmp" && mv "$LOG.tmp" "$LOG"

# ---- Tekshiruvlar -------------------------------------------------------------
# Har bir element: "kutilgan satr|tavsif"
EXPECT=(
    "SELFTEST: .* hammasi PASSED|yadro ichki testlari (buddy, slab, vmalloc, vmm, scheduler)"
    "Ekran: framebuffer|framebuffer konsoli"
    "MyOS'ga xush kelibsiz|shell user rejimida ishga tushdi (/etc/motd)"
    "Men user rejimida \(ring 3\)|hello dasturi ishladi"
    "\[rc\] /dev/sda1 \(ext2\) -> /mnt|/bin/init + /etc/rc diskni uladi"
    "^katta.bin|ext2: papkani o'qish"
    "^811540523 300000 /mnt/test/katta.bin|ext2: 300 KB fayl (bilvosita bloklar) baytma-bayt to'g'ri"
    "fstest: PASSED \(60 tekshiruv\)|fstest /mnt: ext2 ga yozish (60 tekshiruv)"
    "argv\[2\] = \"ikkinchi\"|argv to'g'ri uzatildi"
    "^home/|ls ildiz papkani o'qidi"
    "noldan yozilgan x86-64 yadrosi|cat faylni o'qidi"
    "^salom dunyo|echo ishladi"
    "fstest: PASSED|VFS: fayllar, papkalar, unlink, xatolar, pipe, dup2, /dev"
    "^ +4[0-9]$|pipe + yo'naltirish: ls /bin | wc -l > fayl"
    "^19997$|3 bosqichli pipe (seq | grep | tail) - 100 KB dan ortiq ma'lumot"
    "yiqildi: 1 MyOS|shell: || , \$? va o'zgaruvchilar"
    "^/home/a/b$|mkdir -p, cd, pwd, &&"
    "memtest: PASSED|user malloc/free stress testi"
    "forktest: PASSED|fork + copy-on-write + exec + demand paging"
    "Sabab: sahifa mavjud emas, YOZISH, user rejimida|NULL ga yozish ushlandi"
    "Sabab: ruxsat buzildi \(sahifa bor\), O'QISH, user rejimida|yadro xotirasi himoyalangan"
    "General Protection Fault|imtiyozli instruksiya (cli) ushlandi"
    "Manzil \(CR2\) = 0x00007fffff7|user stek 8 MB gacha o'sib, keyin to'lishi ushlandi"
    "mavjud_emas: buyruq topilmadi|mavjud bo'lmagan dastur xatosi"
    "^\[[0-9]+\]|fon rejimi (&)"
    "HOLAT|ps ishladi"
    "Fizik xotira \(PMM\)|free ishladi"
    "idle/1|ikkinchi CPU yadrosi (SMP) ishlayapti"
    "Host ko'prik|lspci (PCI skaneri)"
    "2 yadro|uname (sysinfo)"
    "UTC\+0|date (RTC soati)"
    "Tizim o'chirilmoqda|poweroff"
)
# Bo'lmasligi kerak bo'lgan satrlar:
FORBID=(
    "KERNEL PANIC|yadro panic bo'lmasligi kerak"
    "FAIL|hech bir test yiqilmasligi kerak"
    "Bu qator chiqmasligi kerak edi|himoya chetlab o'tilmasligi kerak"
)

pass=0
fail=0
for item in "${EXPECT[@]}"; do
    pattern=${item%%|*}
    desc=${item#*|}
    if grep -Eq -- "$pattern" "$LOG"; then
        echo "  [OK]   $desc"
        pass=$((pass + 1))
    else
        echo "  [FAIL] $desc   (kutilgan: /$pattern/)"
        fail=$((fail + 1))
    fi
done
for item in "${FORBID[@]}"; do
    pattern=${item%%|*}
    desc=${item#*|}
    if grep -Eq -- "$pattern" "$LOG"; then
        echo "  [FAIL] $desc   (topildi: /$pattern/)"
        fail=$((fail + 1))
    else
        echo "  [OK]   $desc"
        pass=$((pass + 1))
    fi
done

# ---- Disk: Linux'ning o'z vositalari bilan tekshirish ----
# Tizim o'chgandan keyin disk tasviridan bo'limni ajratib olamiz (1 MB siljish)
# va e2fsck (fayl tizimi to'g'riligi) va debugfs (mazmun) bilan tekshiramiz.
if command -v e2fsck > /dev/null; then
    PART=build/test-part-$MODE.img
    dd if="$DISK" of="$PART" bs=1M skip=1 status=none
    if e2fsck -fn "$PART" > "$LOG.fsck" 2>&1; then
        echo "  [OK]   e2fsck: yadro yozgan ext2 buzilmagan"
        pass=$((pass + 1))
    else
        echo "  [FAIL] e2fsck xato topdi:"
        cat "$LOG.fsck"
        fail=$((fail + 1))
    fi
    want=$(seq 30000 | cksum | cut -d' ' -f1)
    got=$(debugfs -R "cat /yangi/seq.txt" "$PART" 2> /dev/null | cksum | cut -d' ' -f1)
    if [ "$want" = "$got" ] && debugfs -R "cat /yangi/salom.txt" "$PART" 2> /dev/null | grep -q "ext2 diskdaman"; then
        echo "  [OK]   debugfs: yozilgan fayllar Linux'da o'qiladi va mos keladi"
        pass=$((pass + 1))
    else
        echo "  [FAIL] debugfs: /yangi/seq.txt yoki salom.txt mos kelmadi"
        fail=$((fail + 1))
    fi
fi

if [ "$QEMU_STATUS" -eq 124 ]; then
    echo "  [FAIL] QEMU vaqt chegarasida o'z-o'zidan to'xtamadi (osilib qoldimi?)"
    fail=$((fail + 1))
fi

echo "==> Natija ($MODE): $pass OK, $fail FAIL"
if [ "$fail" -ne 0 ]; then
    echo "==> Log oxiri ($LOG):"
    tail -40 "$LOG"
    exit 1
fi
exit 0
