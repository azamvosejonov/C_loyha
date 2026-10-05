#!/usr/bin/env python3
"""Katta loyihalarni (darslik/katta_loyiha/<loyiha>/NN_nom/) yig'ib, natijasini kutilgan.txt bilan solishtiradi.

Har bosqich papkasida: manba fayllar, qur.txt (yig'ish buyrug'i), ixtiyoriy ishga.txt (ishga tushirish buyrug'i, sukut: ./ombor),
ixtiyoriy kirish.txt (stdin) va kutilgan.txt (kutilgan chiqish).
`--yangila` bilan kutilgan.txt hozirgi natijadan qayta yoziladi (faqat natijani ko'zdan kechirgach!).
`--goster NOM` - papka nomida NOM bo'lgan bosqichlarni ishga tushirib natijani chop etadi (yozish paytida qulay).
"""
import glob
import os
import shutil
import subprocess
import sys
import tempfile

ildiz = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'darslik', 'katta_loyiha')
yangila = '--yangila' in sys.argv
goster = sys.argv[sys.argv.index('--goster') + 1] if '--goster' in sys.argv else None
xato = soni = 0

for papka in sorted(glob.glob(os.path.join(ildiz, '*', '[0-9][0-9]_*'))):
    nom = os.path.relpath(papka, ildiz)
    if goster and goster not in nom:
        continue
    qur = os.path.join(papka, 'qur.txt')
    if not os.path.exists(qur):
        continue
    soni += 1
    with tempfile.TemporaryDirectory() as wd:
        shutil.copytree(papka, wd, dirs_exist_ok=True)
        buyruq = open(qur).read().strip()
        r = subprocess.run(['bash', '-c', buyruq], cwd=wd, capture_output=True, text=True)
        if r.returncode != 0 or 'warning' in r.stdout + r.stderr:
            xato += 1
            print(f'XATO (yig\'ish): {nom}\n{r.stdout}{r.stderr}')
            continue
        kirish = os.path.join(wd, 'kirish.txt')
        stdin = open(kirish) if os.path.exists(kirish) else subprocess.DEVNULL
        ishga = os.path.join(wd, 'ishga.txt')
        buyruq = open(ishga).read().strip() if os.path.exists(ishga) else './ombor'
        r = subprocess.run(['bash', '-c', buyruq], cwd=wd, stdin=stdin, capture_output=True, text=True, timeout=120)
        if goster:
            print(f'===== {nom}\n{r.stdout}{r.stderr}')
            continue
        if r.returncode != 0 or r.stderr:
            xato += 1
            print(f'XATO (ishga tushirish, kod {r.returncode}): {nom}\n{r.stderr[:500]}')
            continue
        kutilgan = os.path.join(papka, 'kutilgan.txt')
        if yangila:
            open(kutilgan, 'w').write(r.stdout)
        elif not os.path.exists(kutilgan) or open(kutilgan).read() != r.stdout:
            xato += 1
            print(f'XATO (natija farq qiladi): {nom}')

print(f"katta loyihalar: {soni} ta bosqich, " + ('hammasi OK' if not xato else f'{xato} ta XATO'))
sys.exit(1 if xato else 0)
