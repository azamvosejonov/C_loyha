#!/bin/sh
# birlashtiruvchi.sh - birlashtiruvchi loyihalar skeleti ogohlantirishsiz yig'ilishini va testlar ishga tushishini tekshiradi.
# (Yechim repoda yo'q, shuning uchun testlar o'tmaydi - bu normal; muhimi: kompilyatsiya toza va testlar qulamaydi.)
set -e
ILDIZ=$(cd "$(dirname "$0")/.." && pwd)
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT
cp -r "$ILDIZ/darslik/birlashtiruvchi/02_kadrlar" "$T/k"
cd "$T/k"
CHIQ=$(make 2>&1) || { echo "$CHIQ"; echo "XATO: 02_kadrlar yig'ilmadi"; exit 1; }
echo "$CHIQ" | grep -q "warning" && { echo "$CHIQ"; echo "XATO: 02_kadrlar ogohlantirish beryapti"; exit 1; }
SONI=$(make test 2>&1 | grep -c '^  \[')
[ "$SONI" -eq 14 ] || { echo "XATO: 02_kadrlar: 14 ta test guruhi kutilgandi, $SONI ta chiqdi"; exit 1; }
echo "birlashtiruvchi: 02_kadrlar skeleti toza yig'iladi, 14 ta test guruhi ishlaydi"
