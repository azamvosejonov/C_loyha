#!/bin/sh
# tekshir.sh — firmware ni SBI testi bilan ishga tushirib, natijani tekshiradi
VK=$1; FW=$2; TEST=$3
chiqish=$("$VK" -n 20000000 "$FW" "$TEST" < /dev/null 2>&1)
kod=$?
if [ $kod -eq 0 ] && echo "$chiqish" | grep -q "SBI testlari: OK"; then
    echo "  [ OK ] firmware (SBI): base, probe, konsol, rdtime, set_timer, srst"
else
    echo "  [XATO] firmware (SBI) — chiqish kodi $kod. Chiqish:"
    echo "$chiqish" | tail -5 | sed 's/^/        /'
    exit 1
fi
