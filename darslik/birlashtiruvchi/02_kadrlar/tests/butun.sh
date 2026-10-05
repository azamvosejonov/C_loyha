#!/bin/sh
# butun.sh - tayyor dasturni (bin/kadrlar) butun holda sinaydi: har buyruq natijasi tests/kutilgan_*.txt bilan solishtiriladi.
# Odatda "make tekshir" orqali chaqiriladi (testlar o'tgandan keyin).
cd "$(dirname "$0")/.." || exit 1
xato=0

sina() {        # sina KUTILGAN_FAYL KUTILGAN_KOD buyruq...
    fayl=$1; kod=$2; shift 2
    bin/kadrlar "$@" > /tmp/kadrlar_chiqish.txt 2>&1
    haqiqiy=$?
    if [ "$haqiqiy" = "$kod" ] && diff -q /tmp/kadrlar_chiqish.txt "tests/$fayl" > /dev/null; then
        echo "  [ OK ] ./kadrlar $*"
    else
        echo "  [XATO] ./kadrlar $*   (chiqish kodi $haqiqiy, kutilgan $kod)"
        diff /tmp/kadrlar_chiqish.txt "tests/$fayl" | head -8
        xato=1
    fi
}

echo "Butun dastur:"
sina kutilgan_royxat.txt 0 royxat
sina kutilgan_varaqa_1042.txt 0 varaqa 1042
sina kutilgan_varaqa_9999.txt 0 varaqa 9999
sina kutilgan_top.txt 0 top
sina kutilgan_jami.txt 0 jami
sina kutilgan_izla.txt 0 izla Dilnoza
sina kutilgan_topilmadi.txt 1 varaqa 1234
sina kutilgan_nomalum.txt 1 yoq

if [ $xato -eq 0 ]; then echo "BUTUN DASTUR TO'G'RI"; else echo "butun dasturda farq bor"; fi
exit $xato
