#!/usr/bin/env bash
# =============================================================================
#  tools/screenshot.sh - QEMU ekranini PNG ga saqlash (oynasiz muhitda ham)
#  Ishlatish: tools/screenshot.sh <chiqish.png> <soniya> [qo'shimcha qemu bayroqlari]
# =============================================================================
set -eu
OUT=$1; WAIT=$2; shift 2
SOCK=$(mktemp -u /tmp/qemu-mon.XXXXXX)
qemu-system-x86_64 -cdrom build/myos.iso -m 256M -smp 2 -no-reboot -display none \
    -serial null -monitor unix:$SOCK,server,nowait "$@" &
PID=$!
sleep "$WAIT"
PPM=${OUT%.png}.ppm
printf 'screendump %s\n' "$PPM" | python3 -c "
import socket,sys,time
s=socket.socket(socket.AF_UNIX); s.connect('$SOCK'); time.sleep(0.3)
s.sendall(sys.stdin.read().encode()); time.sleep(1); s.close()"
kill $PID 2>/dev/null || true
python3 - "$PPM" "$OUT" <<'PY'
import sys, zlib, struct
data = open(sys.argv[1], 'rb').read()
parts = data.split(b'\n', 3)
w, h = map(int, parts[1].split()); px = parts[3]
raw = b''.join(b'\x00' + px[y*w*3:(y+1)*w*3] for y in range(h))
def chunk(t, d): return struct.pack('>I', len(d)) + t + d + struct.pack('>I', zlib.crc32(t + d) & 0xffffffff)
png = b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 2, 0, 0, 0)) \
    + chunk(b'IDAT', zlib.compress(raw, 6)) + chunk(b'IEND', b'')
open(sys.argv[2], 'wb').write(png)
PY
rm -f "$PPM"
echo "saqlandi: $OUT"
