#!/usr/bin/env python3
# =============================================================================
#  tools/psf2c.py - PSF2 (Linux konsol shrifti) faylini C massivga aylantirish
#
#  Ishlatish:  python3 tools/psf2c.py spleen-8x16.psfu.gz > kernel/drivers/font8x16.c
#  (Ubuntu: apt install fonts-spleen yoki console-setup; /usr/share/consolefonts/)
#
#  PSF1 formati: 4 baytlik sarlavha (magic 0x36 0x04, rejim, glif hajmi), keyin
#  256 yoki 512 glif, keyin (ixtiyoriy) UCS-2 Unicode jadvali (0xFFFF bilan tugaydi).
#  PSF2 formati: 32 baytlik sarlavha (magic 0x864ab572, glif soni, glif hajmi,
#  balandlik, kenglik), keyin glif bitmaplari, keyin (ixtiyoriy) UTF-8 jadvali.
#  Biz BARCHA glif'larni va Unicode -> glif jadvalini (saralangan) chiqaramiz.
# =============================================================================
import gzip, struct, sys

path = sys.argv[1]
data = gzip.open(path).read() if path.endswith('.gz') else open(path, 'rb').read()
mapping = {}
if data[0:2] == b'\x36\x04':                      # ---- PSF1 ----
    mode, charsize = data[2], data[3]
    count = 512 if mode & 1 else 256
    hdrsize, width, height = 4, 8, charsize
    glyphs = [data[hdrsize + i * charsize: hdrsize + (i + 1) * charsize] for i in range(count)]
    if mode & 2:                                    # Unicode jadvali bor
        pos = hdrsize + count * charsize
        for g in range(count):
            while True:
                (u,) = struct.unpack_from('<H', data, pos)
                pos += 2
                if u == 0xFFFF:
                    break
                if u != 0xFFFE:
                    mapping.setdefault(u, g)
    else:
        mapping = {i: i for i in range(count)}
else:                                               # ---- PSF2 ----
    magic, version, hdrsize, flags, count, charsize, height, width = struct.unpack('<8I', data[:32])
    assert magic == 0x864ab572, "PSF emas"
    glyphs = [data[hdrsize + i * charsize: hdrsize + (i + 1) * charsize] for i in range(count)]
    if flags & 1:
        pos = hdrsize + count * charsize
        for g in range(count):
            end = data.index(b'\xff', pos)
            for ch in data[pos:end].split(b'\xfe')[0].decode('utf-8', 'ignore'):
                mapping.setdefault(ord(ch), g)
            pos = end + 1
    else:
        mapping = {i: i for i in range(count)}
assert width == 8 and height == 16, "faqat 8x16 qo'llab-quvvatlanadi"

fallback = mapping.get(ord('?'), 0)
# Spleen'da yo'q, lekin matnlarda ko'p uchraydigan belgilar - o'xshash glifga:
#   ʻ ʼ (o'zbek lotin alifbosidagi oʻ, gʻ tutuq belgilari) -> ‘ ’,  — -> –
aliases = {0x02BB: 0x2018, 0x02BC: 0x2019, 0x2014: 0x2013, 0x25BA: 0x25B6, 0x25C4: 0x25C0}
for src, dst in aliases.items():
    if src not in mapping and dst in mapping:
        mapping[src] = mapping[dst]
entries = sorted((cp, g) for cp, g in mapping.items() if cp >= 128)

print("""/* =============================================================================
 *  drivers/font8x16.c - 8x16 bitmap shrift (AVTOMATIK GENERATSIYA QILINGAN)
 *  Generator: tools/psf2c.py  |  Manba: Spleen 8x16 (github.com/fcambus/spleen)
 *
 *  Copyright (c) 2018-2024, Frederic Cambus. All rights reserved.
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES ARE DISCLAIMED. (to'liq matn: docs/LICENSES.md)
 *
 *  Tuzilishi:
 *    font8x16_glyphs[g][y] - g-glifning y-qatori, 8 bit = 8 piksel (katta bit - chap)
 *    font8x16_ascii[c]     - ASCII belgi -> glif raqami
 *    font8x16_map[]        - Unicode kod nuqtasi -> glif (kod bo'yicha SARALANGAN:
 *                            ikkilik qidiruv, drivers/vt.c: font_glyph)
 * ============================================================================= */
#include "drivers/font.h"
""")
print(f"const unsigned font8x16_count = {len(glyphs)};")
print(f"const unsigned font8x16_map_len = {len(entries)};")
print(f"const uint16_t font8x16_fallback = {fallback};\n")
print(f"const uint8_t font8x16_glyphs[{len(glyphs)}][16] = {{")
for i, g in enumerate(glyphs):
    print("    { " + ", ".join(f"0x{b:02x}" for b in g) + f" }}, /* {i} */")
print("};\n")
print("const uint16_t font8x16_ascii[128] = {")
row = []
for c in range(128):
    row.append(str(mapping.get(c, fallback) if 32 <= c < 127 else fallback))
for i in range(0, 128, 16):
    print("    " + ", ".join(row[i:i + 16]) + ",")
print("};\n")
print("const struct font_map font8x16_map[] = {")
for cp, g in entries:
    ch = chr(cp)
    label = ch if ch.isprintable() and cp not in (0x2A, 0x2F) else ''
    print(f"    {{ 0x{cp:04X}, {g} }}, /* {label} */")
print("};")
