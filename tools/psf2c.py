#!/usr/bin/env python3
# =============================================================================
#  tools/psf2c.py - PSF2 (Linux konsol shrifti) faylini C massivga aylantirish
#
#  Ishlatish:  python3 tools/psf2c.py spleen-8x16.psfu.gz > kernel/drivers/font8x16.c
#
#  PSF1 formati: 4 baytlik sarlavha (magic 0x36 0x04, rejim, glif hajmi), keyin
#  256 yoki 512 glif, keyin (ixtiyoriy) UCS-2 Unicode jadvali (0xFFFF bilan tugaydi).
#  PSF2 formati: 32 baytlik sarlavha (magic 0x864ab572, glif soni, glif hajmi,
#  balandlik, kenglik), keyin glif bitmaplari, keyin (ixtiyoriy) UTF-8 jadvali.
#  Biz glif tartibini ASCII ga moslaymiz: font[c] = 'c' belgisining rasmi.
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
 *  Tuzilishi: font8x16[c][y] - 'c' belgisining y-qatori, 8 bit = 8 piksel
 *  (eng katta bit = eng chap piksel).
 * ============================================================================= */
#include <stdint.h>

const uint8_t font8x16[128][16] = {""")
for c in range(128):
    g = glyphs[mapping.get(c, fallback)] if 32 <= c < 127 else bytes(16)
    label = repr(chr(c)) if 32 <= c < 127 else f"0x{c:02x}"
    print("    { " + ", ".join(f"0x{b:02x}" for b in g) + " }, /* " + label.replace('*/', '* /') + " */")
print("};")
