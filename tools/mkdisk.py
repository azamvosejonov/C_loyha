#!/usr/bin/env python3
# =============================================================================
#  tools/mkdisk.py - MBR bo'lim jadvali + ext2 fayl tizimli disk tasviri
# =============================================================================
#
#  Ishlatish:  tools/mkdisk.py <chiqish.img> <hajm_MB> <papka> [blok_hajmi]
#
#  Natija - haqiqiy qattiq disk qanday ko'rinsa, xuddi shunday:
#
#    sektor 0            MBR: yuklovchi kodi (bizda bo'sh) + 4 ta bo'lim yozuvi + 55 AA
#    sektor 1..2047      bo'sh (an'ana: bo'limlar 1 MB chegarasidan boshlanadi -
#                        SSD va 4K sektorli disklar uchun tekislash)
#    sektor 2048..oxiri  1-bo'lim (turi 0x83 = "Linux"), ichida ext2
#
#  ext2 ni Linux'ning o'z vositasi mke2fs yaratadi (-d: papka tarkibini ichiga
#  nusxalaydi). Shunday qilib yadromiz "begona" - haqiqiy Linux yaratgan fayl
#  tizimini o'qishi kerak bo'ladi. Keyin esa e2fsck bizning yozganlarimizni
#  tekshiradi. Bu - eng yaxshi test: standartga mos kelish.
# =============================================================================
import os
import struct
import subprocess
import sys
import tempfile

SECTOR = 512
PART_START = 2048                       # 1 MB


def main():
    if len(sys.argv) < 4:
        sys.exit("ishlatish: mkdisk.py <chiqish.img> <hajm_MB> <papka> [blok_hajmi]")
    out, size_mb, src = sys.argv[1], int(sys.argv[2]), sys.argv[3]
    block = sys.argv[4] if len(sys.argv) > 4 else "1024"
    total = size_mb * 1024 * 1024 // SECTOR
    part_sectors = total - PART_START

    with tempfile.TemporaryDirectory() as tmp:
        part = os.path.join(tmp, "part.img")
        # -t ext2        : jurnalsiz, eng sodda ext oilasi
        # -b             : blok hajmi (1024 - ko'p bilvosita bloklar, testlar uchun yaxshi)
        # -O ^dir_index  : papkalar oddiy ro'yxat (htree indekslari yo'q)
        # -E root_owner  : ildiz egasi root (bizning kompyuterdagi foydalanuvchi emas)
        # -d             : papka tarkibini fayl tizimiga nusxalash
        subprocess.run(["mke2fs", "-q", "-F", "-t", "ext2", "-b", block, "-L", "myos",
                        "-O", "^dir_index", "-E", "root_owner=0:0", "-d", src,
                        part, str(part_sectors * SECTOR // 1024) + "k"], check=True,
                       stdout=subprocess.DEVNULL)
        with open(part, "rb") as f:
            data = f.read()

    # MBR bo'lim yozuvi (16 bayt):
    #   holat(1) CHS_boshi(3) tur(1) CHS_oxiri(3) LBA_boshi(4) sektorlar_soni(4)
    # CHS - eski "silindr/kallak/sektor" manzillari; zamonaviy tizimlar LBA ni
    # ishlatadi, CHS o'rniga "juda katta" belgisi (0xFE 0xFF 0xFF) qo'yiladi.
    entry = struct.pack("<B3sB3sII", 0x00, b"\xfe\xff\xff", 0x83, b"\xfe\xff\xff",
                        PART_START, part_sectors)
    mbr = bytearray(SECTOR)
    mbr[440:444] = struct.pack("<I", 0x4D594F53)   # disk imzosi ("MYOS")
    mbr[446:462] = entry
    mbr[510:512] = b"\x55\xaa"

    with open(out, "wb") as f:
        f.write(mbr)
        f.write(bytes((PART_START - 1) * SECTOR))
        f.write(data)
        f.truncate(total * SECTOR)
    print(f"  DISK\t{out}: {size_mb} MB, sda1 = ext2 ({block} baytli bloklar)")


if __name__ == "__main__":
    main()
