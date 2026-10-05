#!/bin/sh
# tahlil.sh - jurnalni quvurlar (pipe) bilan tahlil qiladi: har bir hisobot - kichik buyruqlar zanjiri
# Ishlatish: ./tahlil.sh access.log
LOG=${1:?"foydalanish: $0 jurnal_fayli"}
export LC_ALL=C                                        # tartiblash har joyda bir xil bo'lsin

echo "=== 1. Umumiy ==="
echo "so'rovlar soni: $(wc -l < "$LOG")"
echo "noyob IP lar:   $(cut -d' ' -f1 "$LOG" | sort -u | wc -l)"

echo "=== 2. Eng faol 3 IP ==="
cut -d' ' -f1 "$LOG" | sort | uniq -c | sort -rn | head -3

echo "=== 3. Holat kodlari ==="
awk '{ print $(NF-1) }' "$LOG" | sort | uniq -c | sort -k2n

echo "=== 4. Xatolar (4xx va 5xx) ulushi ==="
awk '{ k = $(NF-1); jami++; if (k >= 400) xato++ }
     END { printf "%d / %d = %.1f%%\n", xato, jami, 100 * xato / jami }' "$LOG"

echo "=== 5. Eng ko'p so'ralgan 3 sahifa ==="
awk '{ print $7 }' "$LOG" | sort | uniq -c | sort -rn | head -3

echo "=== 6. Faqat 404 bo'lgan sahifalar (noyob) ==="
awk '$(NF-1) == 404 { print $7 }' "$LOG" | sort -u | tr '\n' ' '
echo

echo "=== 7. Soatlar bo'yicha so'rovlar (kunning birinchi 6 soati) ==="
awk -F'[:[]' '{ s[$3 + 0]++ } END { for (h = 0; h < 6; h++) printf "%02d:00  %s %d\n", h, substr("##########################################################################", 1, int(s[h] / 4)), s[h] }' "$LOG"

echo "=== 8. Uzatilgan ma'lumot (faqat 200) ==="
awk '$(NF-1) == 200 { bayt += $NF } END { printf "%d bayt = %.1f MB\n", bayt, bayt / 1048576 }' "$LOG"

echo "=== 9. 500 xatosi bergan IP lar ==="
grep ' 500 ' "$LOG" | cut -d' ' -f1 | sort | uniq -c | sort -rn | head -3
