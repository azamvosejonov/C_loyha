#!/bin/sh
# olchov.sh - har rejimni cachegrind (kesh simulyatori) ostida ishga tushirib, D1 (birinchi daraja ma'lumot keshi) xatolarini yig'adi.
# Valgrind PROTSESSORNI emulyatsiya qiladi, shuning uchun natijalar har safar ANIQ bir xil (vaqt o'lchashdan farqli).
# Natijalar ATAYLAB yaxlitlanadi (ming, butun foiz): muhit o'zgaruvchilari hajmi bir necha o'nlab hisobni o'zgartirishi mumkin.
# D1 hisobiga massivni to'ldirish (bir xil, hamma rejimda) ham kiradi. Kesh: 32 KB, 8 yo'lli, 64 baytli qator (zamonaviy x86 ning D1 keshi)
export LC_ALL=C
printf '%-8s %18s %10s\n' rejim "D1 xatolar (ming)" "xato %"
for rejim in satr ustun ijk ikj; do
    valgrind --tool=cachegrind --cache-sim=yes --D1=32768,8,64 --cachegrind-out-file=/dev/null ./xotira_yurish "$rejim" 2>&1 >/dev/null |
    awk -v rejim="$rejim" '
        /D refs:/    { gsub(",", "", $4); okish = $4 }
        /D1  misses:/  { gsub(",", "", $4); xato = $4 }
        END { printf "%-8s %18d %9d%%\n", rejim, int(xato / 1000 + 0.5), int(100 * xato / okish + 0.5) }'
done
