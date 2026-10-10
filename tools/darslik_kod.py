#!/usr/bin/env python3
"""Darslikdagi to'liq dasturlarni yig'ib tekshiradi (make lab-check).

Har bob bo'yicha: matn bloklari TARTIB BILAN o'qiladi. ```c / ```make / ```sh bloki birinchi qatorida fayl nomi izohi bo'lsa
(masalan `/* salom.c - ... */`) vaqtinchalik papkaga yoziladi; ```console blokidagi `$ gcc ...` va `$ make ...` buyruqlari
shu papkada bajariladi. Ogohlantirish ham xato hisoblanadi. Loyiha bloki (<!-- loyiha:... -->) alohida papkada tekshiriladi.
Bir xil nomli fayl keyin yozilsa - oldingisining ustiga yoziladi (bobda ketma-ket ishlaydi).
"""
import glob
import os
import re
import subprocess
import sys
import tempfile

ildiz = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'darslik')
xatolar = soni = 0
B, E = '<!-- loyiha:boshi -->', '<!-- loyiha:oxiri -->'


def tekshir(bob, nom, bolim):
    global xatolar, soni
    with tempfile.TemporaryDirectory() as wd:
        for til, tana in re.findall(r'```(\w+)\n(.*?)```', bolim, re.S):
            if til in ('c', 'make', 'sh'):
                f = re.match(r'\s*(?:/\*|#)\s*([\w.]+)', tana)
                if f and ('.' in f.group(1) or f.group(1) == 'Makefile') and not re.fullmatch(r'[\d.]+', f.group(1)):
                    open(os.path.join(wd, f.group(1)), 'w').write(tana)
            elif til == 'console':
                for q in tana.splitlines():
                    if q.startswith('$ cd katta_loyiha'):
                        break                              # katta loyiha: tools/katta_loyiha.py tekshiradi
                    if not re.match(r'\$ (gcc|make|ar)\b', q) or '# xato kutiladi' in q:
                        continue
                    buyruq = q[2:].split(' && ')[0]        # faqat yig'ish qismi
                    r = subprocess.run(['bash', '-c', buyruq], cwd=wd, capture_output=True, text=True)
                    chiqish = r.stdout + r.stderr
                    soni += 1
                    if r.returncode != 0 or 'warning' in chiqish:
                        xatolar += 1
                        print(f'XATO: {os.path.basename(bob)} [{nom}]: {buyruq}\n{chiqish}')


for bob in sorted(glob.glob(os.path.join(ildiz, 'asos-*.md'))) + sorted(glob.glob(os.path.join(ildiz, '[0-3][0-9]-*.md'))):
    matn = open(bob).read()
    m = re.search(re.escape(B) + '(.*?)' + re.escape(E), matn, re.S)
    asosiy = matn.replace(m.group(0), '') if m else matn
    tekshir(bob, 'bob', asosiy)
    if m:
        tekshir(bob, 'Loyiha', m.group(1))

# Bobda havola qilingan kutilgan natija fayllari (masalan isitish.txt) mavjud bo'lishi kerak
for bob in sorted(glob.glob(os.path.join(ildiz, '[0-3][0-9]-*.md'))):
    for yol in sorted(set(re.findall(r'darslik/loyihalar/(\w+/[\w.]+\.txt)', open(bob).read()))):
        if not os.path.exists(os.path.join(ildiz, 'loyihalar', yol)):
            xatolar += 1
            print(f"XATO: {os.path.basename(bob)}: loyihalar/{yol} yo'q")
# Mustaqil loyihalar: har papkada kutilgan.txt bo'lishi kerak
for bob in sorted(glob.glob(os.path.join(ildiz, '[0-3][0-9]-*.md'))):
    nn = os.path.basename(bob)[:2]
    papkalar = glob.glob(os.path.join(ildiz, 'loyihalar', nn + '_*'))
    if len(papkalar) != 1 or not os.path.exists(os.path.join(papkalar[0], 'kutilgan.txt')):
        xatolar += 1
        print(f"XATO: {os.path.basename(bob)}: loyihalar/{nn}_*/kutilgan.txt yo'q")

print(f"darslik kodi: {soni} ta yig'ish buyrug'i, " + ('hammasi OK' if not xatolar else f'{xatolar} ta XATO'))
sys.exit(1 if xatolar else 0)
