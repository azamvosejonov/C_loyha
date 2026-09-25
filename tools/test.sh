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
#    4. Oxirida `shutdown` bilan QEMU ni yopadi.
#
#  NEGA BU MUHIM:
#    Har bir o'zgarishdan keyin "hamma narsa hali ham ishlaydimi?" savoliga
#    10 soniyada javob olasiz. Kuchli jamoalarda hech bir kod testlardan
#    o'tmasdan asosiy branch'ga tushmaydi (CI - Continuous Integration).
# =============================================================================
set -u

cd "$(dirname "$0")/.."
LOG=build/test-output.log
QEMU=${QEMU:-qemu-system-x86_64}

# Shell'ga yuboriladigan buyruqlar. Har biri alohida qatorda.
COMMANDS=(
    "hello birinchi ikkinchi"
    "ls"
    "cat README.txt"
    "echo salom dunyo"
    "memtest"
    "crash null"
    "crash kernel"
    "crash cli"
    "crash stack"
    "mavjud_emas"
    "spin fon 3 &"
    "ps"
    "free"
    "shutdown"
)

# Buyruqlarni pauzalar bilan yuboruvchi funksiya. Yadro bootlanishi va
# selftest'lar tugashi uchun boshida ko'proq kutamiz.
feed_commands() {
    sleep 3
    for cmd in "${COMMANDS[@]}"; do
        printf '%s\n' "$cmd"
        sleep 1
    done
}

echo "==> QEMU ishga tushirilmoqda (log: $LOG)"
feed_commands | timeout 90 "$QEMU" \
    -kernel build/kernel32.elf -initrd build/initrd.tar \
    -m 128M -no-reboot -display none -serial stdio \
    -append "selftest" > "$LOG" 2>&1
QEMU_STATUS=$?

# ---- Tekshiruvlar -------------------------------------------------------------
# Har bir element: "kutilgan satr|tavsif"
EXPECT=(
    "SELFTEST: .* hammasi PASSED|yadro ichki testlari (pmm, vmm, heap, scheduler)"
    "MyOS shell'iga xush kelibsiz|shell user rejimida ishga tushdi"
    "Men user rejimida \(ring 3\)|hello dasturi ishladi"
    "argv\[2\] = \"ikkinchi\"|argv to'g'ri uzatildi"
    "README.txt|ls initrd ni o'qidi"
    "MyOS diskiga xush kelibsiz|cat faylni o'qidi"
    "^salom dunyo|echo ishladi"
    "memtest: PASSED|user malloc/free stress testi"
    "Sabab: sahifa mavjud emas, YOZISH, user rejimida|NULL ga yozish ushlandi"
    "Sabab: ruxsat buzildi \(sahifa bor\), O'QISH, user rejimida|yadro xotirasi himoyalangan"
    "General Protection Fault|imtiyozli instruksiya (cli) ushlandi"
    "Manzil \(CR2\) = 0x000000007ffe|user stek to'lishi ushlandi"
    "'mavjud_emas' topilmadi|mavjud bo'lmagan dastur xatosi"
    "\[fon\] pid [0-9]+ ishga tushdi|fon rejimi"
    "HOLAT|ps ishladi"
    "Fizik xotira \(PMM\)|free ishladi"
    "Tizim o'chirilmoqda|shutdown"
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

if [ "$QEMU_STATUS" -eq 124 ]; then
    echo "  [FAIL] QEMU vaqt chegarasida o'z-o'zidan to'xtamadi (osilib qoldimi?)"
    fail=$((fail + 1))
fi

echo "==> Natija: $pass OK, $fail FAIL"
if [ "$fail" -ne 0 ]; then
    echo "==> Log oxiri ($LOG):"
    tail -40 "$LOG"
    exit 1
fi
exit 0
