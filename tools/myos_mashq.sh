#!/usr/bin/env bash
# =============================================================================
#  tools/myos_mashq.sh - MyOS mashqlari M1-M4 (printf, kprintf, malloc) ni QEMU'siz tekshirish (~1 s)
# =============================================================================
#  Darslik: darslik/32-printf-malloc.md. Har mashq alohida guruh, xato bo'lsa - aniq kirish/chiqish.
#    tools/myos_mashq.sh         hammasi; biror guruh yiqilsa chiqish kodi 1
#    tools/myos_mashq.sh --ci    CI uchun: faqat yig'ilish va testlar ishga tushishini tekshiradi
#                                (ochiq mashqlar [XATO] bo'lishi normal - yechim repoda yo'q)
#  Hamma guruh [ OK ] bo'lgach: make test (butun tizim QEMU'da).
# =============================================================================
set -e
cd "$(dirname "$0")/.."
OUT=build/host/mashq
mkdir -p $OUT
CI=0
[ "${1:-}" = "--ci" ] && CI=1

ICHKI="-nostdinc -isystem $(gcc -print-file-name=include)"
FLAGS="-c -O1 -g -Wall -Wextra -Werror -ffreestanding -fno-builtin -fno-stack-protector"
gcc $FLAGS $ICHKI -Iuser/include -Iinclude user/libc/printf.c -o $OUT/printf.o
gcc $FLAGS $ICHKI -Iuser/include -Iinclude user/libc/malloc.c -o $OUT/malloc.o
gcc $FLAGS $ICHKI -Ikernel -Iinclude kernel/lib/kprintf.c -o $OUT/kprintf.o
for f in printf malloc kprintf; do
    objcopy --prefix-symbols=lab_ $OUT/$f.o $OUT/lab_$f.o
done
gcc -O1 -g -Wall -Wextra -Wno-format -c tests/host/mashq_test.c -o $OUT/mashq_test.o
gcc -O1 -g -c tests/host/mashq_shim.c -o $OUT/mashq_shim.o
gcc -o $OUT/mashq_test $OUT/mashq_test.o $OUT/mashq_shim.o $OUT/lab_printf.o $OUT/lab_malloc.o $OUT/lab_kprintf.o \
    -Wl,--unresolved-symbols=ignore-all

echo "MyOS mashqlari (darslik/32-printf-malloc.md):"
xato=0
for g in m1 m2 m3 m4; do                # har guruh alohida jarayonda: malloc holati toza boshlanadi
    if [ $CI -eq 1 ]; then              # CI: faqat guruh natijalari (tafsilotlarsiz)
        kod=0; timeout 20 $OUT/mashq_test $g > $OUT/$g.txt || kod=$?
        grep '^  \[' $OUT/$g.txt || { echo "  $g: test ishlamadi (kod $kod)"; exit 1; }
        [ $kod -eq 0 ] || xato=1
    else
        timeout 20 $OUT/mashq_test $g || xato=1
    fi
done
if [ $xato -ne 0 ]; then
    echo "  ochiq mashqlar: $(grep -l 'TODO(M[1-4])' user/libc/printf.c user/libc/malloc.c kernel/lib/kprintf.c 2>/dev/null | tr '\n' ' ')"
    [ $CI -eq 1 ] && { echo "  (--ci: yig'ilish va testlar ishladi; ochiq mashqlar [XATO] bo'lishi kutilgan)"; exit 0; }
    exit 1
fi
echo "  hammasi o'tdi. Endi butun tizim: make test"
