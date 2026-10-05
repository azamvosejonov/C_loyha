#!/bin/sh
# log_yarat.sh - sinov uchun veb-server jurnali (access log) yaratadi. Natija har safar bir xil (o'zimiz yozgan tasodif generatori)
# Ishlatish: ./log_yarat.sh [qatorlar_soni] > access.log
SONI=${1:-3000}
awk -v soni="$SONI" '
BEGIN {
    s = 12345                                          # generator holati (Park-Miller: s = s * 16807 mod (2^31 - 1))
    nip = split("10.0.0.5 10.0.0.9 192.168.1.20 192.168.1.33 172.16.4.7 172.16.4.8 203.0.113.50 198.51.100.77", ip, " ")
    nyol = split("/ /index.html /login /api/narx /api/ombor /rasm/logo.png /yordam /admin", yol, " ")
    nmet = split("GET GET GET GET POST", met, " ")
    for (i = 0; i < soni; i++) {
        s = (s * 16807) % 2147483647; a = s % nip + 1
        s = (s * 16807) % 2147483647; y = s % nyol + 1
        s = (s * 16807) % 2147483647; m = s % nmet + 1
        s = (s * 16807) % 2147483647; r = s % 100                     # 0..99: holat kodini tanlash
        kod = r < 80 ? 200 : (r < 90 ? 404 : (r < 95 ? 301 : (r < 98 ? 500 : 403)))
        s = (s * 16807) % 2147483647; hajm = (kod == 200 ? 500 + s % 9000 : 100 + s % 300)
        s = (s * 16807) % 2147483647; soat = s % 24
        s = (s * 16807) % 2147483647; daqiqa = s % 60
        printf "%s - - [05/Oct/2026:%02d:%02d:%02d +0500] \"%s %s HTTP/1.1\" %d %d\n", ip[a], soat, daqiqa, i % 60, met[m], yol[y], kod, hajm
    }
}'
