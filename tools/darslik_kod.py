#!/usr/bin/env python3
"""Darslikdagi to'liq dasturlarni yig'ib tekshiradi (make lab-check).

Tekshiriladigan bo'limlar (har bobda):
  1) "## Hayotdan misollar" - bob boshidagi dasturlar;
  2) "<!-- loyiha:boshi -->" ... "<!-- loyiha:oxiri -->" - bob oxiridagi loyiha (faqat ko'rsatilgan tizim; mashq yechimi yo'q).

Har bo'limdagi ```c / ```make / ```sh bloklari (birinchi qatorda fayl nomi izohi bilan) vaqtinchalik papkaga yoziladi,
keyin ```console blokidagi gcc/make buyruqlari bajariladi. Ogohlantirish ham xato hisoblanadi.
"""
import glob
import os
import re
import subprocess
import sys
import tempfile

ildiz = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'darslik')
xatolar = soni = 0


def bolimlar(matn):
    m = re.search(r'^## Hayotdan misollar.*?(?=^## )', matn, re.S | re.M)
    if m:
        yield 'Hayotdan misollar', m.group(0)
    m = re.search(r'<!-- loyiha:boshi -->(.*?)<!-- loyiha:oxiri -->', matn, re.S)
    if m:
        yield 'Loyiha', m.group(1)


def tekshir(bob, nom, bolim):
    global xatolar, soni
    with tempfile.TemporaryDirectory() as wd:
        for til, tana in re.findall(r'```(\w+)\n(.*?)```', bolim, re.S):
            if til in ('c', 'make', 'sh'):
                f = re.match(r'\s*(?:/\*|#)\s*([\w.]+)', tana)
                if f and ('.' in f.group(1) or f.group(1) == 'Makefile'):
                    open(os.path.join(wd, f.group(1)), 'w').write(tana)
            elif til == 'console':
                for q in tana.splitlines():
                    if not re.match(r'\$ (gcc|make)\b', q):
                        continue
                    buyruq = q[2:].split(' && ')[0]        # faqat yig'ish qismi
                    r = subprocess.run(['bash', '-c', buyruq], cwd=wd, capture_output=True, text=True)
                    chiqish = r.stdout + r.stderr
                    soni += 1
                    if r.returncode != 0 or 'warning' in chiqish:
                        xatolar += 1
                        print(f'XATO: {os.path.basename(bob)} [{nom}]: {buyruq}\n{chiqish}')


for bob in sorted(glob.glob(os.path.join(ildiz, '[0-3][0-9]-*.md'))):
    matn = open(bob).read()
    for nom, bolim in bolimlar(matn):
        tekshir(bob, nom, bolim)

# Mustaqil loyihalar: har papkada kutilgan.txt bo'lishi kerak
for bob in sorted(glob.glob(os.path.join(ildiz, '[0-3][0-9]-*.md'))):
    nn = os.path.basename(bob)[:2]
    papkalar = glob.glob(os.path.join(ildiz, 'loyihalar', nn + '_*'))
    if len(papkalar) != 1 or not os.path.exists(os.path.join(papkalar[0], 'kutilgan.txt')):
        xatolar += 1
        print(f'XATO: {os.path.basename(bob)}: loyihalar/{nn}_*/kutilgan.txt yo\'q')

print(f"darslik kodi: {soni} ta yig'ish buyrug'i, " + ('hammasi OK' if not xatolar else f'{xatolar} ta XATO'))
sys.exit(1 if xatolar else 0)
