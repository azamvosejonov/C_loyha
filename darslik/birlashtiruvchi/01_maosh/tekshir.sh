#!/bin/sh
# tekshir.sh - "Maosh hisobchisi" bosqichini tekshiradi.
# Ishlatish:  ./tekshir.sh BOSQICH [PAPKA]      (BOSQICH: 1, 2, 3 yoki 4; PAPKA: sizning fayllaringiz, sukut - joriy papka)
# Misol:      ~/C_loyha/darslik/birlashtiruvchi/01_maosh/tekshir.sh 3 ~/maosh/b3

B=${1:?"foydalanish: $0 BOSQICH [PAPKA]"}
P=${2:-.}
D=$(cd "$(dirname "$0")" && pwd)
cd "$P" || exit 1

xato() { echo "XATO: $1"; exit 1; }

# 1) kerakli fayllar
case $B in
1) FAYLLAR="maosh.c" ;;
2) FAYLLAR="main.c hisob.c hisob.h" ;;
3 | 4) FAYLLAR="main.c hisob.c hisob.h pul.c pul.h" ;;
*) xato "bosqich 1..4 bo'lishi kerak" ;;
esac
for f in $FAYLLAR; do [ -f "$f" ] || xato "$f fayli yo'q (bu papkada: $(pwd))"; done

# 2) har .c fayl ALOHIDA kompilyatsiya qilinadi (1-bob: .c -> .o), ogohlantirish ham xato hisoblanadi
rm -f /tmp/maosh_*.o
for c in *.c; do
    OUT=$(gcc -Wall -Wextra -g -c "$c" -o "/tmp/maosh_${c%.c}.o" 2>&1)
    [ -z "$OUT" ] || { echo "$OUT"; xato "$c kompilyatsiyada ogohlantirish/xato berdi"; }
done

# 3) 2-bosqichdan: sarlavha fayllarida "include guard" bo'lishi shart (1-bob)
if [ "$B" -ge 2 ]; then
    for h in *.h; do grep -q '#ifndef' "$h" || xato "$h da include guard (#ifndef ... #define ... #endif) yo'q"; done
fi

# 4) bog'lash va ishga tushirish
gcc /tmp/maosh_*.o -o /tmp/maosh_dastur 2>&1 || xato "bog'lashda (linker) xato"
/tmp/maosh_dastur > /tmp/maosh_chiqish.txt || xato "dastur 0 dan boshqa kod bilan tugadi"

# 5) natijani solishtirish
if diff /tmp/maosh_chiqish.txt "$D/kutilgan_$B.txt" > /tmp/maosh_farq.txt; then
    echo "$B-bosqich: TO'G'RI"
else
    echo "$B-bosqich: natija farq qiladi (< sizniki, > kutilgan):"
    head -20 /tmp/maosh_farq.txt
    exit 1
fi
