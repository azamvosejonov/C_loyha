#!/usr/bin/env bash
# =============================================================================
#  tools/host_libctest.sh - libc funksiyalarini QEMU'SIZ, Linux'ning o'zida tekshirish
# =============================================================================
#  Lab tizimi uchun tezkor sinov (~1 soniya): user/libc dagi string.c, stdlib.c,
#  time.c, printf.c kompyuterda kompilyatsiya qilinadi, barcha belgilariga "lab_"
#  prefiksi qo'shiladi (glibc bilan to'qnashmasligi uchun) va user/bin/libctest.c
#  (xuddi tizim ichidagi test) ular bilan ishga tushiriladi. malloc bu yerda
#  tekshirilmaydi - u sbrk() ga tayanadi (tizim ichidagi memtest ishlatiladi).
# =============================================================================
set -e
cd "$(dirname "$0")/.."
OUT=build/host
mkdir -p $OUT
INC="-nostdinc -isystem $(gcc -print-file-name=include) -Iuser/include -Iinclude"
for f in string stdlib time printf; do
    gcc -c -O1 -g -ffreestanding -fno-builtin -fno-stack-protector $INC \
        user/libc/$f.c -o $OUT/$f.o
    objcopy --prefix-symbols=lab_ $OUT/$f.o $OUT/lab_$f.o
done
# Test o'zi: bizning sarlavhalar + prefiks makrolari; printf/puts - glibc'niki.
gcc -c -O1 -g -ffreestanding -fno-builtin -include tests/host/lab_prefix.h $INC user/bin/libctest.c -o $OUT/libctest.o
gcc -c -O1 tests/host/shim.c -o $OUT/shim.o
# Bizning obyektlar ishlatmaydigan, lekin eslatib o'tgan belgilar (lab_write, lab_exit ...)
# test davomida chaqirilmaydi - ularni hal qilmaymiz.
gcc -o $OUT/libctest $OUT/libctest.o $OUT/lab_*.o $OUT/shim.o -Wl,--unresolved-symbols=ignore-all
timeout 20 $OUT/libctest
