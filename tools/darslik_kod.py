#!/usr/bin/env python3
"""Darslikdagi "Hayotdan misollar" dasturlarini yig'ib tekshiradi (make lab-check).

Har bir bobdagi ```c / ```make / ```sh bloklari (birinchi qatorda fayl nomi izohi bilan)
vaqtinchalik papkaga yoziladi, keyin ```console blokidagi gcc/make buyruqlari bajariladi.
Ogohlantirish ham xato hisoblanadi.
"""
import glob, os, re, subprocess, sys, tempfile

ildiz = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'darslik')
xatolar = soni = 0
for bob in sorted(glob.glob(os.path.join(ildiz, '[0-3][0-9]-*.md'))):
    matn = open(bob).read()
    m = re.search(r'^## Hayotdan misollar.*?(?=^## )', matn, re.S | re.M)
    if not m:
        continue
    with tempfile.TemporaryDirectory() as wd:
        for til, tana in re.findall(r'```(\w+)\n(.*?)```', m.group(0), re.S):
            if til in ('c', 'make', 'sh'):
                f = re.match(r'\s*(?:/\*|#)\s*([\w.]+)', tana)
                if f and ('.' in f.group(1) or f.group(1) == 'Makefile'):
                    open(os.path.join(wd, f.group(1)), 'w').write(tana)
            elif til == 'console':
                for q in tana.splitlines():
                    if not re.match(r'\$ (gcc|make)\b', q):
                        continue
                    buyruq = q[2:].split(' && ./')[0]      # faqat yig'ish
                    r = subprocess.run(['bash', '-c', buyruq], cwd=wd, capture_output=True, text=True)
                    chiqish = r.stdout + r.stderr
                    soni += 1
                    if r.returncode != 0 or 'warning' in chiqish:
                        xatolar += 1
                        print(f'XATO: {os.path.basename(bob)}: {buyruq}\n{chiqish}')
print(f"darslik kodi: {soni} ta yig'ish buyrug'i, " + ('hammasi OK' if not xatolar else f'{xatolar} ta XATO'))
sys.exit(1 if xatolar else 0)
