#!/bin/sh
# =============================================================================
#  19_terminal.sh - terminal va Git mashqi                    (darslik 19-bob)
# =============================================================================
#  Bu skript faqat XAVFSIZ buyruqlarni bajaradi (vaqtinchalik papkada) va har birini
#  oldin ko'rsatadi. Ishga tushirish:  sh 19_terminal.sh
#  Keyin har bir buyruqni O'ZINGIZ qo'lda terminalga yozib takrorlang.
#
#  Sinab ko'ring:
#    1) Shu buyruqlarni o'z papkangizda (masalan ~/mashq) qo'lda bajaring.
#    2) Faylni o'zgartiring -> git diff -> git add -> git commit -> git log --oneline.
#    3) Faylni buzib, `git checkout -- fayl` bilan qaytaring.
#    4) grep -rn "int main" ~/C_loyha/darslik/misollar - qaysi fayllarda main bor?
#    5) man ls, ls --help - har qanday buyruqning yordamnomasi.
# =============================================================================
set -e
korsat() { printf '\n$ %s\n' "$*"; sh -c "$*"; }

ISH=$(mktemp -d)
cd "$ISH"
echo "Vaqtinchalik papka: $ISH"

korsat 'pwd'
korsat 'mkdir -p loyiha/src && ls'
korsat 'printf "#include <stdio.h>\nint main(void) { puts(\"salom\"); return 0; }\n" > loyiha/src/salom.c'
korsat 'cat loyiha/src/salom.c'
korsat 'ls -la loyiha/src'
korsat 'grep -rn "puts" loyiha'
korsat 'find . -name "*.c"'
korsat 'seq 1 100 | grep 7 | wc -l'
korsat 'gcc loyiha/src/salom.c -o salom && ./salom; echo "chiqish kodi: $?"'
korsat 'ls -l salom'
korsat 'chmod 750 salom && ls -l salom'

if command -v git >/dev/null; then
    korsat 'cd loyiha && git init -q && git -c user.name=Talaba -c user.email=t@t add . && git -c user.name=Talaba -c user.email=t@t commit -q -m "birinchi commit" && git log --oneline'
    korsat 'cd loyiha && echo "/* izoh */" >> src/salom.c && git status --short && git diff'
fi

cd /
rm -rf "$ISH"
echo
echo "Tayyor. Vaqtinchalik papka o'chirildi."
