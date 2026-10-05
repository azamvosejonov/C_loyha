#!/bin/sh
# virtual_kompyuter.sh - RISC-V emulyatori (o'quvchi versiyasi) ogohlantirishsiz yig'ilishini va birlik testlari
# 12 ta guruhni aniq xabar bilan chiqarishini tekshiradi. (Yechim repoda yo'q - testlar [XATO] deydi, bu normal.)
# clang/lld bo'lmasa, RISC-V qismi (firmware) o'tkazib yuboriladi.
set -e
ILDIZ=$(cd "$(dirname "$0")/.." && pwd)
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT
cp -r "$ILDIZ/virtual_kompyuter" "$T/vk"
cd "$T/vk"
CHIQ=$(make build/vk build/birlik 2>&1) || { echo "$CHIQ"; echo "XATO: emulyator yig'ilmadi"; exit 1; }
echo "$CHIQ" | grep -q "warning" && { echo "$CHIQ"; echo "XATO: emulyator ogohlantirish beryapti"; exit 1; }
SONI=$(./build/birlik testlar/birlik/kutilgan.dtb | grep -c '^  \[')
[ "$SONI" -eq 12 ] || { echo "XATO: virtual_kompyuter: 12 ta birlik test guruhi kutilgandi, $SONI ta chiqdi"; exit 1; }
if command -v clang > /dev/null && command -v ld.lld > /dev/null; then
    CHIQ=$(make build/firmware.elf 2>&1) || { echo "$CHIQ"; echo "XATO: firmware yig'ilmadi"; exit 1; }
    echo "$CHIQ" | grep -q "warning" && { echo "$CHIQ"; echo "XATO: firmware ogohlantirish beryapti"; exit 1; }
    echo "virtual_kompyuter: emulyator va firmware toza yig'iladi, 12 ta birlik test guruhi ishlaydi"
else
    echo "virtual_kompyuter: emulyator toza yig'iladi, 12 ta birlik test guruhi ishlaydi (clang yo'q: firmware o'tkazildi)"
fi
