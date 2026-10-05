#!/bin/sh
# yigish.sh — emulyator testlarini yig'adi va ishga tushiradi. Ishlatish: sh yigish.sh EMULYATOR
VK=${1:?"emulyator yo'li kerak"}
cd "$(dirname "$0")" || exit 1
mkdir -p build
xato=0
for f in alu sakrash xotira mul csr trap mmu taymer uart mrejim atomik siqilgan siqilgan_c plic; do
    if [ -f $f.c ]; then
        clang --target=riscv32 -march=rv32imac -mabi=ilp32 -Os -ffreestanding -nostdlib -c $f.c -o build/$f.o || { xato=1; continue; }
    else
        clang --target=riscv32 -march=rv32imac_zicsr -mabi=ilp32 -nostdlib -c $f.S -o build/$f.o || { xato=1; continue; }
    fi
    ld.lld -T link.ld build/$f.o -o build/$f.elf || { xato=1; continue; }
    if [ $f = uart ]; then
        chiqish=$(printf 'abc' | "$VK" -S -n 100000 build/$f.elf)
        kod=$?
        [ "$chiqish" = "$(printf 'salom, RISC-V!\nABC')" ] || { echo "  [XATO] $f: chiqish: $chiqish"; xato=1; continue; }
    elif [ $f = plic ]; then
        printf 'abc' | "$VK" -n 1000000 build/$f.elf
        kod=$?
    elif [ $f = mrejim ] || [ $f = siqilgan_c ]; then
        "$VK" -n 10000000 build/$f.elf < /dev/null     # M rejimdan boshlanadi (firmware kabi)
        kod=$?
    else
        "$VK" -S -n 1000000 build/$f.elf < /dev/null
        kod=$?
    fi
    if [ $kod -eq 0 ]; then
        echo "  [ OK ] $f"
    else
        echo "  [XATO] $f: $kod-tekshiruv buzildi"
        xato=1
    fi
done
exit $xato
