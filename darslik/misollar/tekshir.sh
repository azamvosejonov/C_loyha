#!/bin/sh
# =============================================================================
#  tekshir.sh - barcha misollarni yig'ib, ishga tushirib ko'radi (make lab-check)
# =============================================================================
#  Ishga tushirish:  sh darslik/misollar/tekshir.sh
#  Har bir misol -Werror bilan yig'iladi (ogohlantirish = xato). Tez ishlaydiganlari
#  ishga ham tushiriladi. 30_yadro_moduli tekshirilmaydi - linux-headers kerak.
# =============================================================================
set -u
cd "$(dirname "$0")"
ISH=$(mktemp -d)
trap 'rm -rf "$ISH"' EXIT
CF="-Wall -Wextra -Werror -O2"
xato=0
ok=0

# yig <fayl> [qo'shimcha bayroqlar...]
yig() {
    f=$1; shift
    if gcc $CF "$@" "$f" -o "$ISH/${f%.c}" 2>"$ISH/log"; then
        ok=$((ok + 1))
    else
        echo "XATO: $f yig'ilmadi:"; cat "$ISH/log"; xato=$((xato + 1))
    fi
}
# ishla <fayl> [argumentlar...] - dastur 0 bilan tugashi kerak
ishla() {
    f=$1; shift
    [ -x "$ISH/${f%.c}" ] || return 0
    if ! (cd "$ISH" && timeout 60 "./${f%.c}" "$@" >/dev/null 2>&1); then
        echo "XATO: $f ishga tushganda xato bilan tugadi"; xato=$((xato + 1))
    fi
}

for f in 0*.c 1[0-4]*.c 16*.c 17*.c 20*.c 22*.c 24*.c 25*.c 27*.c 28*.c; do
    yig "$f"
done
for f in 15*.c 21*.c 23*.c 26*.c; do
    yig "$f" -pthread
done
yig 13_ub.c -Wno-error                           # ataylab UB - ogohlantirish kutiladi
yig 29_xatoli.c -O0 -Wno-error                   # ataylab xatoli - faqat yig'ilishi tekshiriladi
yig 18_libcsiz.c -ffreestanding -nostdlib -static -fno-stack-protector

for f in 00_salom.c 02_turlar.c 03_bitlar.c 04_boshqaruv.c 05_funksiyalar.c 06_satrlar.c \
         07_korsatkichlar.c 08_xotira.c 09_struct.c 10_makrolar.c 12_stdlib.c 16_bitlar_apparat.c \
         17_assembly.c 20_sonlar.c 22_boglash.c 24_virtual_xotira.c 25_malloc_ichi.c 27_fayllar.c; do
    ishla "$f"
done
ishla 26_faylasuflar.c togri
# 18: chiqish kodi 42 bo'lishi kerak
if [ -x "$ISH/18_libcsiz" ]; then
    "$ISH/18_libcsiz" >/dev/null; kod=$?
    [ "$kod" -eq 42 ] || { echo "XATO: 18_libcsiz chiqish kodi $kod (42 kutilgandi)"; xato=$((xato + 1)); }
fi
# 11: Makefile bilan
if make -s -C 11_kop_fayl CFLAGS="-Wall -Wextra -Werror -g -MMD -MP" >"$ISH/log" 2>&1 &&
   11_kop_fayl/dastur | grep -q "kvadrat(12) = 144"; then
    ok=$((ok + 1))
else
    echo "XATO: 11_kop_fayl:"; cat "$ISH/log"; xato=$((xato + 1))
fi
make -s -C 11_kop_fayl clean >/dev/null 2>&1
# Skriptlar sintaksisi
for s in 01_kompilyatsiya.sh 19_terminal.sh; do
    sh -n "$s" || { echo "XATO: $s sintaksisi"; xato=$((xato + 1)); }
done

if [ "$xato" -eq 0 ]; then
    echo "misollar: $ok ta yig'ildi, hammasi OK"
else
    echo "misollar: $xato ta XATO"
    exit 1
fi
