#!/bin/sh
# =============================================================================
#  01_kompilyatsiya.sh - kompilyatsiyaning 4 bosqichini ko'rish      (darslik 1-bob)
# =============================================================================
#  Ishga tushirish:  sh 01_kompilyatsiya.sh
#  Har bir bosqich natijasi QANDAY ko'rilishini ko'rsatadi (.i va .s - matn,
#  .o - binar: maxsus vositalar bilan). Keyin buyruqlarni o'zingiz qo'lda takrorlang.
#
#  Sinab ko'ring (qo'lda, o'z papkangizda):
#    1) salom.c ga `#define XABAR "salom"` qo'shib, printf'da ishlating. salom.i da XABAR
#       so'zi qolmaganini ko'ring: grep XABAR salom.i
#    2) gcc -S -O0 va gcc -S -O2 natijalarini solishtiring (wc -l salom.s).
#    3) `chmod +x salom.o && ./salom.o` - "Exec format error": .o hali dastur emas, uni
#       linker (4-bosqich) dasturga aylantiradi. .i va .s esa umuman ishga tushirilmaydi -
#       ular matn, cat yoki less bilan o'qiladi.
#    4) #include <stdio.h> ni o'chirib, gcc -c qiling - "implicit declaration of printf".
# =============================================================================
set -e
cd "$(dirname "$0")"
ISH=$(mktemp -d)
cp 00_salom.c "$ISH/salom.c"
cd "$ISH"

echo "=== 1. Preprotsessor: gcc -E salom.c -o salom.i  (natija - MATN, cat/less bilan o'qiladi)"
gcc -E salom.c -o salom.i
echo "salom.i: $(wc -l < salom.i) qator (siz 20 qatorga yaqin yozdingiz - qolgani stdio.h dan)"
echo "--- oxirgi 8 qator (sizning kodingiz, izohlarsiz):"
tail -8 salom.i
echo "--- printf ning E'LONI (kodi emas!):"
grep -n "printf (const char" salom.i | head -2

echo
echo "=== 2. Kompilyator: gcc -S -O1 salom.c -o salom.s  (natija - assembly MATNI; -O1 - qisqaroq bo'lishi uchun)"
gcc -S -O1 salom.c -o salom.s
grep -v "^\s*\.\(cfi\|file\|ident\|section\|type\|size\|globl\|p2align\|text\|align\)" salom.s | head -20

echo
echo "=== 3. Assembler: gcc -c salom.c -o salom.o  (natija - BINAR, cat bilan o'qib bo'lmaydi)"
gcc -c salom.c -o salom.o
file salom.o
echo "--- nm salom.o  (T - shu yerda aniqlangan, U - tashqaridan kerak):"
nm salom.o
echo "--- objdump -d salom.o  (mashina kodi baytlari + assembly):"
objdump -d -M intel salom.o | sed -n '/<main>:/,$p' | head -12

echo
echo "=== 4. Linker: gcc salom.o -o salom  (natija - DASTUR, ./ bilan ishga tushiriladi)"
gcc salom.o -o salom
file salom
./salom

cd /
rm -rf "$ISH"
