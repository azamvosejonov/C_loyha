#!/bin/sh
# tekshir.sh — Linux ni virtual kompyuterimizda yuklaydi, shell ga buyruqlar "yozadi" va natijani bosqichma-bosqich
# tekshiradi. Qaysi bosqichda to'xtagani — xato QAYERDA ekaniga ishora (masalan "taymer" bosqichi — F1/F2, E9).
VK=$1; FW=$2; IMAGE=$3
KIRISH=$(mktemp); CHIQISH=$(mktemp)
trap 'rm -f "$KIRISH" "$CHIQISH"' EXIT
printf 'echo salom dunyo\nrun /bin/salom bir ikki\ncat /proc/cpuinfo\nuname\npoweroff\n' > "$KIRISH"

# -i: kirish "odamdek" — shell tayyor bo'lgandagina qatorma-qator beriladi;  -n: cheksiz aylanishdan himoya
"$VK" -i -m 64 -n 1500000000 -k "$IMAGE" "$FW" < "$KIRISH" > "$CHIQISH" 2>&1
kod=$?

xato=0
bosqich() {     # bosqich "tavsif" "kutilgan matn" "buzilsa qayerga qarash kerak"
    if grep -qF -- "$2" "$CHIQISH"; then
        echo "  [ OK ] $1"
    else
        echo "  [XATO] $1 — chiqishda yo'q: \"$2\""
        echo "         qarang: $3"
        xato=1
    fi
}
bosqich "firmware ishga tushdi (M rejim)"         "[vk-sbi] yadroga o'tyapman"        "firmware/asosiy.c (F3), emu: E1-E4"
bosqich "Linux yadrosi S rejimda gapirdi"         "Linux version 6.6"                 "F3 (mret, MPP), SBI konsol, E5 (MMU)"
bosqich "qurilmalar daraxti o'qildi (DTB)"        "Machine model: virtual-kompyuter"               "emu/dtb.c (D1-D3)"
bosqich "ISA kengaytmalari tanildi"               "riscv: base ISA extensions acim"   "dtb.c: riscv,isa-extensions"
bosqich "taymer ishlaydi (sched_clock, jiffies)"  "clocksource: Switched to clocksource riscv_clocksource" "F1, F2, E9 (uzilishlar)"
bosqich "UART konsoli (16550, PLIC orqali)"       "printk: console [ttyS0] enabled"   "emu/uart.c, P1 (PLIC)"
bosqich "init (pid 1) ishga tushdi"               "Salom! Linux ishga tushdi"         "E6 (ecall U->S), E8, E10 (atomik)"
bosqich "shell buyrug'i bajarildi (echo)"         "salom dunyo"                       "UART qabul uzilishi (E9, P1)"
bosqich "fork + execve + waitid"                  "[pid 16 tugadi, kod 7]"            "E5 (COW: sahifa xatolari), E6"
bosqich "/proc/cpuinfo: ISA satri"                "isa		: rv32imac_zicntr_zicsr_zifencei" "dtb.c"
bosqich "poweroff (SBI SRST)"                     "[vk-sbi] yadro tizimni o'chirishni so'radi" "firmware/sbi.c (SRST)"
if [ $kod -ne 0 ]; then
    echo "  [XATO] emulyator chiqish kodi $kod (kutilgan 0)"
    xato=1
fi
if [ $xato -ne 0 ]; then
    echo "  --- chiqishning oxirgi 15 qatori ---"
    tail -15 "$CHIQISH" | sed 's/^/        /'
    exit 1
fi
echo "  [ OK ] Linux to'liq ishladi: yuklandi, buyruqlarni bajardi, o'chdi"
