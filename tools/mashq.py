#!/usr/bin/env python3
# =============================================================================
#  tools/mashq.py - "Python'dan C'ga" mashqlarini tekshiruvchi
# =============================================================================
#
#  mashqlar/NN_nom/ papkasidagi har bir mashq:
#    yechim.c - SIZ yozadigan fayl (vazifa sharti uning boshida)
#    mashq.h  - funksiyalar e'lonlari (o'zgartirmang)
#    test.c   - testlar (o'zgartirmang)
#
#  Tekshirish: yechim.c + test.c kompyuteringizdagi gcc bilan yig'iladi va
#  AddressSanitizer + UndefinedBehaviorSanitizer ostida ishga tushiriladi.
#  Ular massiv chegarasidan chiqish, ozod qilingan xotiradan foydalanish,
#  xotira sizib chiqishi (leak) va butun son toshishini USHLAYDI - C'dagi eng
#  xavfli xatolarni. Yadroda bunday yordamchi yo'q, shuning uchun ularni hozir,
#  xavfsiz joyda o'rganing.
#
#  Buyruqlar:
#    tools/mashq.py                  - ro'yxat va holat
#    tools/mashq.py vazifa [NN]      - vazifa shartini ko'rsatish (sukut: keyingisi)
#    tools/mashq.py tekshir [NN]     - tekshirish (sukut: keyingi o'tilmagan mashq)
#    tools/mashq.py hammasi          - hammasini tekshirish
#    tools/mashq.py selfcheck [DIR]  - CI: hamma yechim.c kompilyatsiya bo'ladimi
#                                      (DIR berilsa - o'sha papkadagi yechimlar bilan
#                                       testlar O'TISHINI ham tekshiradi)
# =============================================================================
import json
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
MDIR = os.path.join(ROOT, 'mashqlar')
BUILD = os.path.join(ROOT, 'build', 'mashq')
HOLAT = os.path.join(ROOT, '.mashq', 'holat.json')

# Har bir bayroq nega kerak:
#   -std=gnu11 -Wall -Wextra -Werror : yadrodagi kabi - ogohlantirish = xato
#   -g -O0                            : gdb bilan qadamma-qadam ko'rish mumkin
#   -fsanitize=address                : xotira xatolari (chegara, use-after-free, leak)
#   -fsanitize=undefined              : aniqlanmagan xatti-harakat (toshish, noto'g'ri surish)
#   -fno-sanitize-recover=undefined   : UB topilsa - darhol to'xtash (yashirin qolmasin)
#   -fno-omit-frame-pointer           : xato hisobotida aniq chaqiruvlar zanjiri
#   -pthread                          : 29-mashq (oqimlar) uchun
CFLAGS = ['-std=gnu11', '-Wall', '-Wextra', '-Werror', '-g', '-O0',
          '-fsanitize=address,undefined', '-fno-sanitize-recover=undefined',
          '-fno-omit-frame-pointer', '-pthread']
TIMEOUT = 30

# Sanitizer xabarlari -> odam tilida tushuntirish.
TUSHUNTIRISH = [
    ('heap-buffer-overflow',
     "malloc bilan olingan xotira CHEGARASIDAN chiqdingiz (massiv oxiridan keyin o'qish/yozish).\n"
     "  Ko'p uchraydigan sabab: '\\0' uchun +1 bayt unutilgan yoki `i <= n` (`i < n` o'rniga)."),
    ('stack-buffer-overflow',
     "stekdagi massiv (lokal o'zgaruvchi) chegarasidan chiqdingiz."),
    ('global-buffer-overflow',
     "global yoki satr literali chegarasidan chiqdingiz (masalan, '\\0' dan keyin o'qish)."),
    ('heap-use-after-free',
     "free() qilingan xotiradan foydalandingiz. free dan keyin u xotira endi sizniki EMAS.\n"
     "  Ro'yxatni ozod qilishda: avval `keyingi` ni saqlang, keyin free qiling."),
    ('attempting double-free',
     "bitta xotirani ikki marta free() qildingiz."),
    ('detected memory leaks',
     "xotira sizib chiqdi: malloc qilingan xotira free() qilinmagan.\n"
     "  Hisobotdagi 'allocated by' qismi qaysi qatorda ajratilganini ko'rsatadi."),
    ('SEGV on unknown address 0x000000000000',
     "NULL ko'rsatkich orqali murojaat (Python'dagi None.x ga o'xshaydi, lekin C'da dastur yiqiladi)."),
    ('SEGV on unknown address',
     "ruxsat etilmagan manzilga murojaat (boshlanmagan yoki buzilgan ko'rsatkich)."),
    ('signed integer overflow',
     "ishorali butun son TOSHIB KETDI. Python'da son cheksiz o'sadi, C'da int 32 bit -\n"
     "  toshish esa aniqlanmagan xatti-harakat (UB). Kattaroq tur (long) yoki tekshiruv kerak."),
    ('shift exponent',
     "bitni noto'g'ri surish: `1 << 31` int uchun UB. `1u << n` yoki `1ull << n` yozing."),
    ('left shift of',
     "ishorali sonni chapga surish toshdi. Ishorasiz tur (`1u << n`) ishlating."),
    ('division by zero',
     "nolga bo'lish."),
    ('stack-use-after-return',
     "funksiyadan qaytgan lokal o'zgaruvchining manzilidan foydalandingiz."),
    ('load of misaligned address',
     "tekislanmagan manzildan o'qish."),
    ('load of value', "noto'g'ri qiymat (masalan, bool'da 0/1 dan boshqa son)."),
]


def mashqlar():
    out = []
    if not os.path.isdir(MDIR):
        return out
    for d in sorted(os.listdir(MDIR)):
        m = re.match(r'^(\d\d)_', d)
        if m and os.path.isdir(os.path.join(MDIR, d)):
            out.append((m.group(1), d))
    return out


def sarlavha(d):
    try:
        with open(os.path.join(MDIR, d, 'yechim.c'), encoding='utf-8') as f:
            for line in f:
                m = re.match(r'^ \*  \d\d - (.*?)\s*$', line)
                if m:
                    return m.group(1)
    except OSError:
        pass
    return d


def holat_ol():
    try:
        with open(HOLAT, encoding='utf-8') as f:
            return json.load(f)
    except (OSError, ValueError):
        return {}


def holat_saqla(h):
    os.makedirs(os.path.dirname(HOLAT), exist_ok=True)
    with open(HOLAT, 'w', encoding='utf-8') as f:
        json.dump(h, f, indent=1)


def top(nn):
    for n, d in mashqlar():
        if n == nn.zfill(2) or d == nn:
            return n, d
    sys.exit(f"'{nn}' mashqi topilmadi. Ro'yxat: tools/mashq.py")


def keyingisi():
    h = holat_ol()
    for n, d in mashqlar():
        if not h.get(d):
            return n, d
    return None


def yigish(d, yechim=None):
    os.makedirs(BUILD, exist_ok=True)
    src = os.path.join(MDIR, d)
    exe = os.path.join(BUILD, d)
    cmd = ['gcc'] + CFLAGS + ['-I', MDIR, '-I', src,
                              yechim or os.path.join(src, 'yechim.c'),
                              os.path.join(src, 'test.c'), '-o', exe]
    r = subprocess.run(cmd, capture_output=True, text=True)
    return r.returncode == 0, r.stderr, exe


def ishlatish(exe):
    env = dict(os.environ)
    env.setdefault('ASAN_OPTIONS', 'detect_leaks=1:abort_on_error=0')
    env.setdefault('UBSAN_OPTIONS', 'print_stacktrace=1')
    try:
        r = subprocess.run([exe], capture_output=True, text=True, timeout=TIMEOUT,
                           env=env, cwd=os.path.dirname(exe))
        return r.returncode, r.stdout, r.stderr, False
    except subprocess.TimeoutExpired as e:
        out = e.stdout.decode(errors='replace') if isinstance(e.stdout, bytes) else (e.stdout or '')
        return -1, out, '', True


def tekshir(n, d, jim=False, yechim=None):
    if not jim:
        print(f"==> {n}: {sarlavha(d)}")
    ok, err, exe = yigish(d, yechim)
    if not ok:
        if not jim:
            print(err.rstrip())
            print("\n[KOMPILYATSIYA XATOSI] Eng BIRINCHI xatoni o'qing - qolganlari ko'pincha "
                  "undan kelib chiqadi.\n  'fayl:qator:ustun: error: ...' - xato aynan o'sha joyda "
                  "(yoki bir qator yuqorida: `;` unutilgan bo'lishi mumkin).")
        return False
    code, out, err, vaqt = ishlatish(exe)
    sanitizer = [(k, t) for k, t in TUSHUNTIRISH if k in err]
    otdi = code == 0 and not vaqt and not sanitizer
    if jim:
        return otdi
    print(out.rstrip())
    if vaqt:
        print(f"\n[VAQT TUGADI] Dastur {TIMEOUT} soniyada tugamadi: cheksiz sikl yoki deadlock "
              "(masalan, pipe'dan o'qimasdan bolani kutish).")
    if err.strip() and (sanitizer or code != 0):
        # Sanitizer hisobotining eng muhim qismi - boshidagi qatorlar.
        lines = err.rstrip().splitlines()
        print('\n' + '\n'.join(lines[:40]))
        if len(lines) > 40:
            print(f"   ... (yana {len(lines) - 40} qator)")
    if sanitizer:
        print("\n[TUSHUNTIRISH]")
        seen = set()
        for k, t in sanitizer:
            if t not in seen:
                seen.add(t)
                print(f"  * {t}")
        print("  Hisobotdagi '#0 ... yechim.c:QATOR' - xato aynan shu qatorda.")
    if otdi:
        print(f"\n*** {n} O'TDI! ***")
        h = holat_ol()
        h[d] = True
        holat_saqla(h)
        k = keyingisi()
        if k:
            print(f"Keyingisi: {k[0]} - {sarlavha(k[1])}   (tools/mashq.py vazifa)")
        else:
            print("Hamma 30 ta mashq tugadi! Endi REJA.md dagi keyingi bosqichga o'ting.")
    elif not vaqt and not sanitizer and code != 0 and 'NATIJA' not in out:
        print(f"\n[YIQILDI] Dastur {code} kodi bilan tugadi.")
    return otdi


def vazifa(d):
    with open(os.path.join(MDIR, d, 'yechim.c'), encoding='utf-8') as f:
        text = f.read()
    end = text.find('*/')
    print(text[:end + 2] if end >= 0 else text[:2000])
    print(f"\nFayl: mashqlar/{d}/yechim.c   (funksiyalar e'loni: mashq.h, testlar: test.c)")


def royxat():
    h = holat_ol()
    k = keyingisi()
    jami = 0
    modul = None
    for n, d in mashqlar():
        with open(os.path.join(MDIR, d, 'yechim.c'), encoding='utf-8') as f:
            m = re.search(r'\[(\d-modul: [^\]]*)\]', f.read(600))
        if m and m.group(1) != modul:
            modul = m.group(1)
            print(f"\n  {modul}")
        belgi = '[x]' if h.get(d) else '[ ]'
        jami += bool(h.get(d))
        strelka = '  <- keyingisi' if k and k[1] == d else ''
        print(f"    {belgi} {n}  {sarlavha(d)}{strelka}")
    print(f"\n  O'tildi: {jami}/{len(mashqlar())}.  Boshlash: tools/mashq.py vazifa, "
          "keyin yechim.c ni yozing va tools/mashq.py tekshir")


def main():
    args = sys.argv[1:]
    cmd = args[0] if args else 'royxat'
    if cmd in ('royxat', 'list'):
        royxat()
    elif cmd in ('vazifa', 'tekshir'):
        if len(args) > 1:
            n, d = top(args[1])
        else:
            k = keyingisi()
            if not k:
                print("Hamma mashqlar o'tilgan! Qayta tekshirish: tools/mashq.py tekshir NN")
                return
            n, d = k
        if cmd == 'vazifa':
            vazifa(d)
        else:
            sys.exit(0 if tekshir(n, d) else 1)
    elif cmd == 'hammasi':
        otdi = 0
        for n, d in mashqlar():
            r = tekshir(n, d, jim=True)
            otdi += r
            print(f"  {'[x]' if r else '[ ]'} {n}  {sarlavha(d)}")
            if r:
                h = holat_ol()
                h[d] = True
                holat_saqla(h)
        print(f"\n  {otdi}/{len(mashqlar())} o'tdi")
    elif cmd == 'selfcheck':
        ydir = args[1] if len(args) > 1 else None
        bad = 0
        for n, d in mashqlar():
            if ydir:
                y = os.path.join(ydir, d, 'yechim.c')
                r = tekshir(n, d, jim=True, yechim=y)
                what = "namunaviy yechim testdan o'tadi"
            else:
                r, err, _ = yigish(d)
                if not r:
                    print(err)
                what = "yechim.c kompilyatsiya bo'ladi"
            bad += not r
            print(f"  [{'OK' if r else 'FAIL'}] {n} {d}: {what}")
        sys.exit(1 if bad else 0)
    else:
        print(__doc__ or '', "Buyruqlar: royxat, vazifa [NN], tekshir [NN], hammasi, selfcheck")
        sys.exit(2)


if __name__ == '__main__':
    main()
