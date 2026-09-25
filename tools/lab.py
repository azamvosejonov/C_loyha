#!/usr/bin/env python3
# =============================================================================
#  tools/lab.py - LABORATORIYALAR: kodni yopib, o'zingiz qayta yozish
# =============================================================================
#
#  G'OYA: yadrodagi muhim funksiyalar tanasi maxsus izohlar bilan belgilangan:
#
#      size_t strlen(const char *s)
#      {
#          /* >>> LAB strlen - vazifa: labs/README.md */
#          ... asl kod ...
#          /* <<< LAB strlen */
#      }
#
#  `boshla` asl kodni .lab/ papkasiga saqlaydi va o'rniga labs/stubs/ dagi
#  "bo'sh" variantni qo'yadi (u kompilyatsiya bo'ladi, lekin ishlamaydi).
#  Siz funksiyani qayta yozasiz, `tekshir` esa butun tizimni QEMU'da sinaydi.
#
#  Buyruqlar:
#      tools/lab.py royxat              barcha laboratoriyalar va holati
#      tools/lab.py boshla  <nom>       asl kodni yashirib, vazifani ko'rsatish
#      tools/lab.py tekshir [<nom>]     yig'ish + testlar (QEMU, ~1 daqiqa)
#      tools/lab.py yechim  <nom>       sizning kodingiz va asl kod farqi
#      tools/lab.py tiklash <nom>       asl kodni qaytarish
#      tools/lab.py selfcheck           har bir stub kompilyatsiya bo'lishini tekshirish (CI)
# =============================================================================
import difflib
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SAVE_DIR = os.path.join(ROOT, '.lab')
STUB_DIR = os.path.join(ROOT, 'labs', 'stubs')

# Har bir laboratoriya: (nom, daraja, hududlar [(fayl, hudud)], test natijasidagi kalit satrlar)
# Tartib - tavsiya etilgan o'rganish tartibi (oddiydan murakkabga).
LABS = [
    ('strlen', 1, [('user/libc/string.c', 'strlen')], ['libctest']),
    ('memmove', 1, [('user/libc/string.c', 'memmove')], ['libctest']),
    ('strtok_r', 2, [('user/libc/string.c', 'strtok_r')], ['libctest']),
    ('strtoul', 2, [('user/libc/stdlib.c', 'strtoul')], ['libctest']),
    ('emit_number', 2, [('user/libc/printf.c', 'emit_number')], ['libctest', 'echo']),
    ('gmtime_r', 2, [('user/libc/time.c', 'gmtime_r')], ['libctest', 'date']),
    ('glob_match', 2, [('user/bin/sh.c', 'glob_match')], ['glob']),
    ('malloc', 3, [('user/libc/malloc.c', 'malloc')], ['malloc', 'libctest']),
    ('insert_free', 3, [('user/libc/malloc.c', 'insert_free')], ['malloc', 'libctest']),
    ('path_normalize', 2, [('kernel/fs/vfs.c', 'path_normalize')], ['mkdir -p', 'VFS']),
    ('pipe', 3, [('kernel/fs/pipe.c', 'pipe_read'), ('kernel/fs/pipe.c', 'pipe_write')],
     ['pipe', 'VFS', 'signallar']),
    ('spinlock', 3, [('kernel/lib/spinlock.c', 'spin_lock'), ('kernel/lib/spinlock.c', 'spin_unlock')],
     ['SELFTEST', 'yadro ichki']),
    ('buddy', 3, [('kernel/mm/pmm.c', 'buddy_alloc'), ('kernel/mm/pmm.c', 'buddy_free')],
     ['yadro ichki']),
    ('slab', 3, [('kernel/mm/slab.c', 'kmem_cache_alloc')], ['yadro ichki']),
    ('elf_load', 3, [('kernel/sys/elf.c', 'elf_load')], ['hello']),
    ('sleep_wakeup', 4, [('kernel/proc/process.c', 'proc_sleep'),
                         ('kernel/proc/process.c', 'wakeup_locked')], ['yadro ichki', 'pipe']),
    ('fault_page', 4, [('kernel/mm/mm.c', 'fault_page')], ['fork', 'malloc']),
    ('block_path', 2, [('kernel/fs/ext2.c', 'block_path')], ['ext2', 'e2fsck']),
    ('bmap', 4, [('kernel/fs/ext2.c', 'bmap')], ['ext2', 'e2fsck', 'debugfs']),
    ('dir_add', 3, [('kernel/fs/ext2.c', 'add_visit'), ('kernel/fs/ext2.c', 'dir_add')],
     ['ext2', 'e2fsck']),
    ('setup_frame', 4, [('kernel/proc/signal.c', 'setup_frame')], ['signallar']),
    ('sigreturn', 4, [('kernel/proc/signal.c', 'sys_sigreturn')], ['signallar']),
]
STARS = {1: '*', 2: '**', 3: '***', 4: '****'}


def die(msg):
    print(msg, file=sys.stderr)
    sys.exit(1)


def find_lab(name):
    for lab in LABS:
        if lab[0] == name:
            return lab
    die(f"'{name}' degan laboratoriya yo'q. Ro'yxat: tools/lab.py royxat")


def read_lines(path):
    with open(os.path.join(ROOT, path)) as f:
        return f.read().split('\n')


def write_lines(path, lines):
    with open(os.path.join(ROOT, path), 'w') as f:
        f.write('\n'.join(lines))


def region_bounds(lines, region):
    """Belgilar orasidagi qatorlar: [start, end) - belgilarning o'zi kirmaydi."""
    begin = [i for i, l in enumerate(lines) if re.match(rf'\s*/\* >>> LAB {region}\b', l)]
    end = [i for i, l in enumerate(lines) if re.match(rf'\s*/\* <<< LAB {region}\b', l)]
    if len(begin) != 1 or len(end) != 1 or end[0] < begin[0]:
        die(f"'{region}' hududining belgilari topilmadi yoki buzilgan")
    return begin[0] + 1, end[0]


def save_path(region):
    return os.path.join(SAVE_DIR, region + '.yechim')


def is_active(lab):
    return all(os.path.exists(save_path(r)) for _, r in lab[2])


def start(name, quiet=False):
    lab = find_lab(name)
    if is_active(lab):
        die(f"'{name}' allaqachon boshlangan. Qaytadan: tools/lab.py tiklash {name}")
    os.makedirs(SAVE_DIR, exist_ok=True)
    for path, region in lab[2]:
        lines = read_lines(path)
        a, b = region_bounds(lines, region)
        with open(save_path(region), 'w') as f:
            f.write('\n'.join(lines[a:b]))
        with open(os.path.join(STUB_DIR, region + '.c')) as f:
            stub = f.read().rstrip('\n').split('\n')
        write_lines(path, lines[:a] + stub + lines[b:])
    if quiet:
        return
    print(f"==> '{name}' laboratoriyasi boshlandi ({STARS[lab[1]]})")
    for path, region in lab[2]:
        print(f"    {path}: {region}() - asl kod olib tashlandi, TODO izohini o'qing")
    print(f"    Vazifa sharti: labs/README.md -> '{name}' bo'limi")
    print(f"    Yozib bo'lgach:  tools/lab.py tekshir {name}")
    print(f"    Taslim bo'lsangiz: tools/lab.py yechim {name}  (avval o'zingiz urinib ko'ring!)")


def restore(name, quiet=False):
    lab = find_lab(name)
    for path, region in lab[2]:
        sp = save_path(region)
        if not os.path.exists(sp):
            die(f"'{name}' boshlanmagan")
        lines = read_lines(path)
        a, b = region_bounds(lines, region)
        with open(sp) as f:
            orig = f.read().split('\n')
        write_lines(path, lines[:a] + orig + lines[b:])
        os.remove(sp)
    if not quiet:
        print(f"==> '{name}': asl kod qaytarildi")


def solution(name):
    lab = find_lab(name)
    for path, region in lab[2]:
        sp = save_path(region)
        if not os.path.exists(sp):
            die(f"'{name}' boshlanmagan - asl kod faylning o'zida: {path}")
        lines = read_lines(path)
        a, b = region_bounds(lines, region)
        with open(sp) as f:
            orig = f.read().split('\n')
        print(f"==> {path}: {region}()  ('-' sizniki, '+' asl kod)")
        for l in difflib.unified_diff(lines[a:b], orig, 'sizning', 'asl', lineterm='', n=2):
            print(l)


def status():
    print(f"{'#':>2}  {'nom':<15} {'daraja':<7} {'holat':<13} fayllar")
    for i, lab in enumerate(LABS, 1):
        state = 'BOSHLANGAN' if is_active(lab) else '-'
        files = ', '.join(sorted({p for p, _ in lab[2]}))
        print(f"{i:>2}  {lab[0]:<15} {STARS[lab[1]]:<7} {state:<13} {files}")
    print("\nBoshlash: tools/lab.py boshla <nom>     Tavsiflar: labs/README.md")


def run(cmd):
    return subprocess.run(cmd, cwd=ROOT, shell=True, capture_output=True, text=True)


def check(name=None):
    lab = find_lab(name) if name else None
    if lab and 'libctest' in lab[3]:
        # libc funksiyasi: avval QEMU'siz, 1 soniyalik tezkor test - xato aniq qayerda.
        print("==> 1-bosqich: host testi (libc funksiyalari Linux'ning o'zida, ~1 s)...")
        r = run('tools/host_libctest.sh 2>&1')
        print(r.stdout.rstrip())
        if r.returncode != 0:
            die("==> Host testi yiqildi - avval shularni tuzating (QEMU testi o'tkazib yuborildi)")
        print("==> 2-bosqich: butun tizim QEMU'da")
    r = run('make -s 2>&1')
    errors = [l for l in r.stdout.split('\n') if 'error' in l or 'Error' in l]
    if r.returncode != 0:
        print("\n".join(errors[:20]) or r.stdout[-3000:])
        die("==> KOMPILYATSIYA XATOSI - yuqoridagi xabarlarni o'qing")
    print("==> QEMU'da testlar (tools/test.sh bios, ~1 daqiqa)...")
    r = run('tools/test.sh bios 2>&1')
    lines = [l for l in r.stdout.split('\n') if '[OK]' in l or '[FAIL]' in l or 'Natija' in l]
    if lab:
        keys = [k.lower() for k in lab[3]]
        focus = [l for l in lines if any(k in l.lower() for k in keys)]
        print(f"==> '{name}' ga tegishli tekshiruvlar:")
        for l in focus:
            print(l)
    fails = [l for l in lines if '[FAIL]' in l]
    if fails:
        print("==> yiqilgan tekshiruvlar:")
        for l in fails:
            print(l)
        print("    Log: build/test-output-bios.log (yadro xabarlari, panic, backtrace)")
    print(lines[-1] if lines else "==> natija yo'q (QEMU ishga tushmadimi? log'ni ko'ring)")
    if r.returncode == 0:
        print(f"==> TABRIKLAYMIZ! {'`' + name + '` ' if name else ''}hamma testdan o'tdi."
              + (f" Asl kod bilan solishtiring: tools/lab.py yechim {name}" if name else ''))
    sys.exit(r.returncode)


def selfcheck():
    """Har bir stub bilan kod kompilyatsiya bo'lishini tekshirish (CI uchun)."""
    ok = True
    for lab in LABS:
        name = lab[0]
        if is_active(lab):
            die(f"'{name}' boshlangan holatda - avval tiklang")
        start(name, quiet=True)
        try:
            r = run('make -s build/kernel.elf build/initrd.tar 2>&1')
            good = r.returncode == 0
        finally:
            restore(name, quiet=True)
        print(f"  [{'OK' if good else 'FAIL'}] {name}")
        if not good:
            print(r.stdout[-2000:])
            ok = False
    run('make -s')
    sys.exit(0 if ok else 1)


def main():
    args = sys.argv[1:]
    if not args or args[0] in ('royxat', 'list'):
        status()
    elif args[0] == 'boshla' and len(args) == 2:
        start(args[1])
    elif args[0] == 'tiklash' and len(args) == 2:
        restore(args[1])
    elif args[0] == 'yechim' and len(args) == 2:
        solution(args[1])
    elif args[0] == 'tekshir':
        check(args[1] if len(args) > 1 else None)
    elif args[0] == 'selfcheck':
        selfcheck()
    else:
        print(__doc__ or '', end='')
        die("ishlatish: tools/lab.py [royxat | boshla NOM | tekshir [NOM] | yechim NOM | "
            "tiklash NOM | selfcheck]")


if __name__ == '__main__':
    main()
