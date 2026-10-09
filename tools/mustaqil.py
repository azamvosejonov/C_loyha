#!/usr/bin/env python3
# =============================================================================
#  tools/mustaqil.py - O'Z YADROINGIZNI (noldan, MyOS'ga qaramasdan) tekshirish
# =============================================================================
#
#  G'OYA: labs/ da funksiya imzosi, ma'lumot tuzilmalari va testlar tayyor edi.
#  Mustaqil yadroda esa hech narsa tayyor emas: dizaynni SIZ tanlaysiz. Shuning
#  uchun bu vosita yadroingizning ichiga qaramaydi - u faqat TASHQARIDAN
#  kuzatiladigan narsalarni tekshiradi ("qora quti" testi):
#
#    1. serial port (COM1) ga chiqqan qatorlar   - protokol: mustaqil/README.md
#    2. QEMU'ning `-d int` logi                  - haqiqatan istisno/uzilish bo'ldimi,
#                                                  qaysi rejimda (cpl), CR2, xato kodi
#    3. QEMU monitori (`info registers`)         - EFER.LMA, CR0.PG, IDT, TR ...
#    4. QEMU monitori (`sendkey`)                - klaviaturaga tugma bosish (B8)
#
#  Xato bo'lsa, vosita logni tahlil qilib MASLAHAT beradi: qaysi istisno, qaysi
#  funksiyangizda (addr2line), eng ehtimoliy sabablar. Lekin yechimni bermaydi.
#
#  Buyruqlar:
#      tools/mustaqil.py royxat                        bosqichlar ro'yxati
#      tools/mustaqil.py tekshir B4 --elf yol/yadro.elf   bitta bosqich (elf yo'li eslab qolinadi)
#      tools/mustaqil.py tekshir B4                    oxirgi --elf bilan
#      tools/mustaqil.py hammasi                       B0 dan boshlab birinchi xatogacha
#      tools/mustaqil.py log                           oxirgi ishga tushirish logining tahlili
#      tools/mustaqil.py selftest                      vositaning o'zini tekshirish (QEMU'siz, CI)
#
#  Qo'shimcha bayroqlar (tekshir uchun):
#      --vaqt 40        kutish chegarasi, soniya (sukut: 30)
#      --gdb            QEMU gdb'ni kutadi (:1234), vaqt chegarasi yo'q
#      --oyna           QEMU oynasini ko'rsatish (VGA ekran)
#      --korsat         QEMU buyrug'i va grub.cfg ni chop etish (qo'lda takrorlash uchun)
#      --abi syscall    B12 dasturi `int 0x80` o'rniga `syscall` buyrug'ini ishlatsin
#
#  Kerak: qemu-system-x86_64, grub-mkrescue (+ xorriso, mtools), nasm, ld, addr2line.
#  MyOS'ni yig'a olsangiz - hammasi bor.
# =============================================================================
import argparse
import json
import os
import re
import shutil
import socket
import subprocess
import sys
import tempfile
import threading
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SOZLAMA = os.path.join(ROOT, '.mustaqil.json')     # oxirgi --elf yo'li (git'ga kirmaydi)
MUSTAQIL = os.path.join(ROOT, 'mustaqil')
OXIRGI = os.path.join(ROOT, 'build', 'mustaqil')    # oxirgi ishga tushirish: serial, log
RAM_MB = 128
USER_BAZA = 0x8000000000
LOG_CHEGARA = 32 * 1024 * 1024      # cheksiz takrorlanuvchi istisno logni yuzlab MB ga to'ldiradi
SERIAL_CHEGARA = 20000              # qator

RANG = sys.stdout.isatty() and os.environ.get('NO_COLOR') is None


def rang(kod, s):
    return f'\033[{kod}m{s}\033[0m' if RANG else s


def yashil(s):
    return rang('32', s)


def qizil(s):
    return rang('31;1', s)


def sariq(s):
    return rang('33', s)


def qalin(s):
    return rang('1', s)


# =============================================================================
#  1. QEMU `-d int` logini o'qish
# =============================================================================
#
#  QEMU har bir istisno/uzilishni shunday yozadi (faqat himoyalangan rejimda):
#     2: v=0e e=0007 i=0 cpl=3 IP=0023:0000000000400137 pc=0000000000400137 SP=001b:00007fffffffef50 CR2=00007fffffffef54
#  v - vektor, e - xato kodi, i=1 - `int N` buyrug'i (dasturiy), cpl - qaysi halqada edi.
#  Undan keyin registrlar (RAX=..., CR0=..., EFER=...). Triple fault'da "Triple fault" qatori.

VOQEA_RE = re.compile(
    r'^\s*\d+: v=([0-9a-f]+) e=([0-9a-f]+) i=(\d) cpl=(\d) IP=([0-9a-f]+):([0-9a-f]+) '
    r'pc=([0-9a-f]+) SP=([0-9a-f]+):([0-9a-f]+)(?: CR2=([0-9a-f]+))?')

VEKTOR = {
    0: '#DE - nolga bo\'lish (Divide Error)',
    1: '#DB - debug',
    2: 'NMI',
    3: '#BP - int3 (Breakpoint)',
    4: '#OF - overflow',
    5: '#BR - bound',
    6: '#UD - noma\'lum buyruq (Invalid Opcode)',
    7: '#NM - FPU/SSE yo\'q (Device Not Available)',
    8: '#DF - double fault (istisno ishlovchisini chaqirib bo\'lmadi)',
    10: '#TS - noto\'g\'ri TSS',
    11: '#NP - segment yo\'q (Segment Not Present)',
    12: '#SS - stek segmenti xatosi',
    13: '#GP - umumiy himoya xatosi (General Protection)',
    14: '#PF - sahifa xatosi (Page Fault)',
    16: '#MF - x87 FPU xatosi',
    17: '#AC - tekislash (Alignment Check)',
    18: '#MC - Machine Check',
    19: '#XM - SIMD (SSE) xatosi',
}


def vektor_nomi(v, i=0):
    if v in VEKTOR and not (i == 0 and v >= 32):
        return VEKTOR[v]
    if i == 1:
        return f'int 0x{v:x} (dasturiy uzilish)'
    if 0x20 <= v < 0x30:
        return f'IRQ{v - 0x20} (PIC qayta raqamlangan bo\'lsa)'
    return f'vektor 0x{v:x}'


def pf_kodi(e):
    """#PF xato kodini (Intel SDM 3A, "Page-Fault Exceptions") o'zbekchaga aylantirish."""
    q = ['sahifa BOR, ruxsat buzildi' if e & 1 else 'sahifa YO\'Q (P=0)',
         'YOZISH' if e & 2 else 'O\'QISH',
         'user rejimida (U=1)' if e & 4 else 'yadro rejimida']
    if e & 8:
        q.append('jadvalda zahiradagi bit o\'rnatilgan (RSVD)')
    if e & 16:
        q.append('buyruq olishda (bajarish, NX?)')
    return ', '.join(q)


class Voqea:
    def __init__(self, m):
        self.v = int(m.group(1), 16)
        self.e = int(m.group(2), 16)
        self.i = int(m.group(3))
        self.cpl = int(m.group(4))
        self.cs = int(m.group(5), 16)
        self.pc = int(m.group(7), 16)
        self.sp = int(m.group(9), 16)
        self.cr2 = int(m.group(10), 16) if m.group(10) else None
        self.ichki = []                       # undan keyingi registr qatorlari

    @property
    def istisnomi(self):
        """Haqiqiy CPU istisnosimi (apparat IRQ yoki `int N` emas)?"""
        return self.v < 32 and self.i == 0 and not (self.v == 8 and self.e == 0 and self._irq_ehtimoli)

    _irq_ehtimoli = False

    def reg(self, nom):
        for q in self.ichki:
            m = re.search(r'\b' + re.escape(nom) + r'=\s*([0-9a-f]+)', q)
            if m:
                return int(m.group(1), 16)
        return None

    def tavsif(self, belgi=None):
        s = f'v=0x{self.v:02x} {vektor_nomi(self.v, self.i)}, cpl={self.cpl}, rip=0x{self.pc:x}'
        if belgi:
            s += f' ({belgi})'
        if self.v in (8, 10, 11, 12, 13, 14, 17) and self.i == 0:
            s += f', xato kodi=0x{self.e:x}'
        if self.v == 14 and self.i == 0:
            s += f', CR2=0x{self.cr2 or 0:x} [{pf_kodi(self.e)}]'
        if self.v == 13 and self.e and self.i == 0:
            idx = self.e >> 3
            if self.e & 2:
                s += f' [IDT ning {idx} (0x{idx:x})-yozuvi sabab]'
            else:
                s += f' [selektor 0x{self.e & ~7:x} sabab]'
        return s


class Log:
    def __init__(self, matn):
        self.voqealar = []
        self.triple = False
        self.reset_soni = 0
        joriy = None
        for q in matn.splitlines():
            m = VOQEA_RE.match(q)
            if m:
                joriy = Voqea(m)
                self.voqealar.append(joriy)
                continue
            if q.startswith('Triple fault'):
                self.triple = True
                joriy = None
            elif q.startswith('CPU Reset'):
                self.reset_soni += 1
                joriy = None
            elif q.startswith('SMM: enter') or q.startswith('check_exception'):
                joriy = None
            elif joriy is not None:
                joriy.ichki.append(q)

    def istisnolar(self, cpl=None):
        return [v for v in self.voqealar if v.istisnomi and (cpl is None or v.cpl == cpl)]

    def vektor(self, v, i=None, cpl=None):
        return [x for x in self.voqealar
                if x.v == v and (i is None or x.i == i) and (cpl is None or x.cpl == cpl)]


# =============================================================================
#  2. Yordamchilar: ELF belgilar (addr2line), monitor, ISO yig'ish
# =============================================================================

def belgi(elf, manzil):
    """Manzil -> 'funksiya (fayl:qator)' - sizning yadroingiz ichida qayerda."""
    if not elf or not shutil.which('addr2line'):
        return None
    try:
        r = subprocess.run(['addr2line', '-f', '-e', elf, hex(manzil)],
                           capture_output=True, text=True, timeout=5)
    except (OSError, subprocess.TimeoutExpired):
        return None
    q = r.stdout.split('\n')
    if len(q) < 2 or q[0] == '??':
        return None
    joy = os.path.basename(q[1]) if not q[1].startswith('??') else ''
    return f'{q[0]} {joy}'.strip()


def elf_entry(elf):
    if not elf:
        return None
    try:
        with open(elf, 'rb') as f:
            h = f.read(64)
    except OSError:
        return None
    if h[:4] != b'\x7fELF':
        return None
    if h[4] == 2:
        return int.from_bytes(h[24:32], 'little')
    return int.from_bytes(h[24:28], 'little')


ANSI_RE = re.compile(r'\x1b\[[0-9;?]*[ -/]*[@-~]|\x1b[()][0-9A-Za-z]|\x1b[=>78DEHM]')


def tozala(s):
    return ANSI_RE.sub('', s).replace('\r', '').rstrip()


class Monitor:
    """QEMU HMP monitori (unix socket): `info registers`, `sendkey`."""

    def __init__(self, yol, kutish=5.0):
        self.s = None
        oxir = time.monotonic() + kutish
        while time.monotonic() < oxir:
            try:
                s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
                s.connect(yol)
                self.s = s
                break
            except OSError:
                time.sleep(0.05)
        if self.s:
            self.s.settimeout(3.0)
            self._oqi()

    def _oqi(self):
        buf = b''
        try:
            while not buf.endswith(b'(qemu) '):
                b = self.s.recv(4096)
                if not b:
                    break
                buf += b
        except OSError:
            pass
        return buf.decode('utf-8', 'replace')

    def buyruq(self, b):
        if not self.s:
            return ''
        try:
            self.s.sendall(b.encode() + b'\n')
        except OSError:
            return ''
        javob = tozala(self._oqi())
        q = javob.split('\n')
        return '\n'.join(q[1:-1] if len(q) > 1 else q)

    def yop(self):
        if self.s:
            try:
                self.s.close()
            except OSError:
                pass


def registrlar(matn):
    """`info registers` -> {'EFER': .., 'CR0': .., 'TR': selektor, 'IDT_LIM': .., ...}"""
    r = {}
    for nom in ('EFER', 'CR0', 'CR2', 'CR3', 'CR4', 'RIP', 'RSP', 'RFL'):
        m = re.search(r'\b' + nom + r'=([0-9a-f]+)', matn)
        if m:
            r[nom] = int(m.group(1), 16)
    m = re.search(r'\bHLT=(\d)', matn)
    if m:
        r['HLT'] = int(m.group(1))
    m = re.search(r'^TR =([0-9a-f]+) [0-9a-f]+ [0-9a-f]+ [0-9a-f]+(.*)$', matn, re.M)
    if m:
        r['TR'] = int(m.group(1), 16)
        r['TR_TUR'] = m.group(2).strip()
    m = re.search(r'^IDT=\s*([0-9a-f]+) ([0-9a-f]+)', matn, re.M)
    if m:
        r['IDT_BAZA'] = int(m.group(1), 16)
        r['IDT_LIM'] = int(m.group(2), 16)
    m = re.search(r'^CS =([0-9a-f]+) .*?(CS64|CS32|CS16)?', matn, re.M)
    if m:
        r['CS'] = int(m.group(1), 16)
    return r


def dastur_yigish(ishchi, abi):
    """B12 uchun mustaqil/dastur.asm -> dastur.elf."""
    obj = os.path.join(ishchi, 'dastur.o')
    elf = os.path.join(ishchi, 'dastur.elf')
    nasm = ['nasm', '-f', 'elf64', os.path.join(MUSTAQIL, 'dastur.asm'), '-o', obj]
    if abi == 'syscall':
        nasm[1:1] = ['-DSYSCALL_ABI']
    subprocess.run(nasm, check=True)
    subprocess.run(['ld', '-static', '-nostdlib', '-z', 'max-page-size=0x1000', '-z', 'noexecstack',
                    '-T', os.path.join(MUSTAQIL, 'dastur.ld'), obj, '-o', elf], check=True)
    return elf


def iso_yigish(ishchi, elf, bosqich, kalit, abi):
    """GRUB ISO: yadroingiz + buyruq qatori `mustaqil=Bn kalit=...` (+ B12 da modul)."""
    iso_dir = os.path.join(ishchi, 'iso')
    os.makedirs(os.path.join(iso_dir, 'boot', 'grub'))
    shutil.copy(elf, os.path.join(iso_dir, 'boot', 'yadro.elf'))
    modul = ''
    if bosqich == 'B12':
        shutil.copy(dastur_yigish(ishchi, abi), os.path.join(iso_dir, 'boot', 'dastur.elf'))
        modul = '    module2 /boot/dastur.elf dastur\n'
    cfg = ('set timeout=0\n'
           'set default=0\n'
           'serial --unit=0 --speed=115200\n'
           'terminal_input serial console\n'
           'terminal_output serial\n'
           'menuentry "mustaqil" {\n'
           f'    multiboot2 /boot/yadro.elf mustaqil={bosqich} kalit={kalit}\n'
           f'{modul}'
           '    boot\n'
           '}\n')
    with open(os.path.join(iso_dir, 'boot', 'grub', 'grub.cfg'), 'w') as f:
        f.write(cfg)
    iso = os.path.join(ishchi, 'mustaqil.iso')
    r = subprocess.run(['grub-mkrescue', '-o', iso, iso_dir], capture_output=True, text=True)
    if r.returncode != 0:
        sys.exit(qizil('grub-mkrescue xato berdi:\n') + r.stderr[-2000:] +
                 '\nKerak: grub-pc-bin, xorriso, mtools (README.md, "Tez boshlash").')
    return iso, cfg


# =============================================================================
#  3. Ishga tushirish: QEMU + serialni o'qish + interaktiv qadamlar
# =============================================================================

class Natija:
    """Bitta ishga tushirishda kuzatilgan hamma narsa."""

    def __init__(self):
        self.qatorlar = []           # [(vaqt, matn)] - serialdan, tozalangan
        self.log = Log('')
        self.reg = {}                # tugaganda `info registers`
        self.chiqdi = False          # QEMU o'zi tugadi (triple fault + -no-reboot, isa-debug-exit)
        self.vaqt_tugadi = False
        self.log_toldi = False       # -d int logi LOG_CHEGARA dan oshdi -> to'xtatildi
        self.serial_toldi = False
        self.qemu_xato = ''          # QEMU'ning o'z xabarlari (stderr)
        self.elf = None
        self.kalit = ''

    def matnlar(self):
        return [q for _, q in self.qatorlar]

    def topish(self, regex):
        r = re.compile(regex)
        for t, q in self.qatorlar:
            m = r.search(q)
            if m:
                return m, t
        return None, None

    def hammasi(self, regex):
        r = re.compile(regex)
        return [(r.search(q), t) for t, q in self.qatorlar if r.search(q)]


def ishga_tushir(bosqich, elf, vaqt, gdb=False, oyna=False, korsat=False, abi='int80'):
    for v in ('qemu-system-x86_64', 'grub-mkrescue'):
        if not shutil.which(v):
            sys.exit(qizil(f'{v} topilmadi.') + ' O\'rnatish: README.md, "Tez boshlash".')
    n = Natija()
    n.elf = elf
    n.kalit = str(100000 + (int(time.time() * 1000) % 900000))
    ishchi = tempfile.mkdtemp(prefix='mq')
    try:
        iso, cfg = iso_yigish(ishchi, elf, bosqich.nom, n.kalit, abi)
        mon_yol = os.path.join(ishchi, 'mon.sock')
        log_yol = os.path.join(ishchi, 'int.log')
        qemu = ['qemu-system-x86_64', '-accel', 'tcg', '-M', 'pc', '-m', f'{RAM_MB}M',
                '-cdrom', iso, '-boot', 'd', '-serial', 'stdio',
                '-monitor', f'unix:{mon_yol},server,nowait',
                '-no-reboot', '-d', 'int,cpu_reset', '-D', log_yol,
                '-device', 'isa-debug-exit,iobase=0xf4,iosize=0x04']
        if not oyna:
            qemu += ['-display', 'none']
        if gdb:
            qemu += ['-s', '-S']
        if korsat:
            print(qalin('grub.cfg:'))
            print(cfg)
            print(qalin('QEMU:'), ' '.join(qemu), '\n')
        if gdb:
            print(sariq('QEMU gdb\'ni kutyapti. Boshqa terminalda:'))
            print(f'    gdb {elf} -ex "target remote :1234"')
            print('    (break kmain, continue ...). Vaqt chegarasi o\'chirildi.\n')

        t0 = time.monotonic()
        p = subprocess.Popen(qemu, stdin=subprocess.DEVNULL, stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE)
        qism = ['']

        def oquvchi():
            while True:
                b = p.stdout.read1(4096) if hasattr(p.stdout, 'read1') else p.stdout.read(1)
                if not b:
                    break
                qism[0] += b.decode('utf-8', 'replace')
                while '\n' in qism[0]:
                    q, qism[0] = qism[0].split('\n', 1)
                    n.qatorlar.append((time.monotonic() - t0, tozala(q)))

        th = threading.Thread(target=oquvchi, daemon=True)
        th.start()
        mon = Monitor(mon_yol)
        interaktiv_bajarildi = False
        oxir = None if gdb else t0 + vaqt
        try:
            while True:
                if p.poll() is not None:
                    n.chiqdi = True
                    break
                if bosqich.tugadimi(n):
                    time.sleep(0.3)
                    break
                if bosqich.interaktiv and not interaktiv_bajarildi and bosqich.interaktiv_sharti(n):
                    bosqich.interaktiv(mon)
                    interaktiv_bajarildi = True
                if oxir and time.monotonic() > oxir:
                    n.vaqt_tugadi = True
                    break
                if os.path.exists(log_yol) and os.path.getsize(log_yol) > LOG_CHEGARA:
                    n.log_toldi = True
                    break
                if len(n.qatorlar) > SERIAL_CHEGARA:
                    n.serial_toldi = True
                    break
                time.sleep(0.05)
            if p.poll() is None:
                n.reg = registrlar(mon.buyruq('info registers'))
                mon.buyruq('quit')
        finally:
            mon.yop()
            try:
                p.wait(timeout=3)
            except subprocess.TimeoutExpired:
                p.kill()
                p.wait()
            th.join(timeout=2)
            try:
                n.qemu_xato = p.stderr.read().decode('utf-8', 'replace')[-2000:]
            except (OSError, ValueError):
                pass
        if qism[0].strip():
            n.qatorlar.append((time.monotonic() - t0, tozala(qism[0])))
        del n.qatorlar[SERIAL_CHEGARA:]
        try:
            with open(log_yol, errors='replace') as f:
                log_matn = f.read(LOG_CHEGARA)
        except OSError:
            log_matn = ''
        n.log = Log(log_matn)
        # oxirgi ishga tushirishni saqlaymiz: `tools/mustaqil.py log` va qo'lda o'qish uchun
        os.makedirs(OXIRGI, exist_ok=True)
        with open(os.path.join(OXIRGI, 'serial.txt'), 'w') as f:
            f.write('\n'.join(n.matnlar()) + '\n')
        with open(os.path.join(OXIRGI, 'int.log'), 'w') as f:
            f.write(log_matn)
        with open(os.path.join(OXIRGI, 'grub.cfg'), 'w') as f:
            f.write(cfg)
        with open(os.path.join(OXIRGI, 'bosqich.txt'), 'w') as f:
            f.write(f'{bosqich.nom}\n{elf}\n')
        return n
    finally:
        shutil.rmtree(ishchi, ignore_errors=True)


# =============================================================================
#  4. Hisobot: tekshiruvlar va maslahatlar
# =============================================================================

class Hisobot:
    def __init__(self):
        self.qatorlar = []           # (tur, matn): tur = ok | xato | ixt (ixtiyoriy) | info
        self.maslahat = []

    def ok(self, s):
        self.qatorlar.append(('ok', s))

    def xato(self, s):
        self.qatorlar.append(('xato', s))

    def ixt(self, yaxshi, s):
        self.qatorlar.append(('ixt_ok' if yaxshi else 'ixt', s))

    def info(self, s):
        self.qatorlar.append(('info', s))

    def tekshir(self, shart, yaxshi, yomon=None):
        if shart:
            self.ok(yaxshi)
        else:
            self.xato(yomon or yaxshi)
        return bool(shart)

    def masl(self, s):
        if s not in self.maslahat:
            self.maslahat.append(s)

    @property
    def otdi(self):
        return not any(t == 'xato' for t, _ in self.qatorlar)


def qator_kut(n, h, regex, nom):
    """Qator chiqqanmi? Bo'lmasa - eng yaqin o'xshash qatorni ko'rsatamiz."""
    m, _ = n.topish(regex)
    if m:
        h.ok(nom)
        return m
    prefiks = regex.split(' ')[0].lstrip('^')
    oxshash = [q for q in n.matnlar() if q.startswith(prefiks)]
    if oxshash:
        h.xato(f'{nom}\n           kutilgan (regex): {regex}\n           o\'xshash qator : {oxshash[0]!r}')
    else:
        h.xato(f'{nom} - qator chiqmadi')
    return None


def banner_tekshir(n, h, gacha):
    """B0..B3 qatorlari har yuklanishda chiqadi (regressiya: eski bosqich buzilmasin)."""
    if gacha >= 0:
        qator_kut(n, h, r'^B0 salom$', 'B0: "B0 salom" (yuklandi, serial ishlaydi)')
    if gacha >= 1:
        qator_kut(n, h, r'^B1 kmain', 'B1: "B1 kmain..." (64-bit C kodi ishlayapti)')
    if gacha >= 2:
        b2_tekshir(n, h)
    if gacha >= 3:
        b3_tekshir(n, h)


def umumiy_tahlil(n, h, bosqich_raqam):
    """Har bir bosqich uchun: GRUB xatolari, triple fault, vaqt, istisnolar ro'yxati."""
    matn = '\n'.join(n.matnlar())
    grub = re.search(r'error: ([^\n]*)', matn)
    if grub:
        xabar = grub.group(1)
        h.xato(f'GRUB xatosi: {xabar}')
        if 'multiboot header' in xabar:
            h.masl('GRUB Multiboot2 sarlavhasini topmadi. Tekshiring: sarlavha faylning BIRINCHI 32 KB '
                   'ichidami (linker skriptida birinchi bo\'lim)? 8 baytga tekislanganmi? magic = '
                   '0xE85250D6, arch = 0, checksum = -(magic + arch + uzunlik) va tugatuvchi teg (type=0, '
                   'size=8) bormi? `grub-file --is-x86-multiboot2 yadro.elf` bilan tekshiring.')
        elif 'ELF' in xabar or 'arch' in xabar:
            h.masl('GRUB ELF faylni o\'qiy olmadi: `readelf -h yadro.elf` - Machine: X86-64 yoki 80386? '
                   'Segment manzillari (`readelf -l`) fizik xotirada (1 MB dan yuqori) bo\'lishi kerak.')
        elif 'unsupported tag' in xabar:
            h.masl('Multiboot2 sarlavhasida GRUB tanimaydigan teg bor: ixtiyoriy teglarga flags=1 qo\'ying '
                   'yoki keraksiz tegni olib tashlang.')
        else:
            h.masl('GRUB yadroni yuklay olmadi: `grub-file --is-x86-multiboot2 yadro.elf` va '
                   '`readelf -l yadro.elf` bilan tekshiring.')
        return

    if n.log.triple or n.log.reset_soni > 2:     # QEMU ishga tushishda 2 marta reset qiladi
        h.xato('TRIPLE FAULT: CPU qayta yuklandi (istisno ishlovchisini ham chaqirib bo\'lmadi)')
        oxirgilar = n.log.istisnolar()[-3:]
        if oxirgilar:
            h.masl('Triple fault oldidan oxirgi istisnolar (eng muhimi - BIRINCHISI, qolganlari uning '
                   'oqibati):\n' + '\n'.join('      ' + v.tavsif(belgi(n.elf, v.pc)) for v in oxirgilar))
            birinchi = oxirgilar[0]
            if birinchi.v == 14 and birinchi.cr2 is not None and birinchi.pc == birinchi.cr2:
                h.masl('RIP == CR2: CPU bajarmoqchi bo\'lgan kodning o\'zi xaritalanmagan. Paging yoqilgan '
                       'paytda (yoki CR3 almashganda) hozir bajarilayotgan kod yangi jadvalda bormi?')
        else:
            h.masl('Logda istisno yo\'q: IDT hali yo\'q paytda xato bo\'lgan (masalan, long mode ga o\'tishda). '
                   '`--gdb` bilan ishga tushirib, `stepi` bilan qaysi buyruqda qulashini toping. '
                   'Eng ko\'p sabablar: sahifa jadvali bajarilayotgan kodni identity-xaritalamagan; '
                   'GDT da 64-bit kod segmenti (L=1, D=0) yo\'q; far jump selektori noto\'g\'ri.')
    if n.log_toldi or n.serial_toldi:
        sanoq = {}
        for v in n.log.voqealar:
            sanoq[(v.v, v.i)] = sanoq.get((v.v, v.i), 0) + 1
        eng = max(sanoq.items(), key=lambda x: x[1]) if sanoq else None
        h.info(('QEMU logi 32 MB dan' if n.log_toldi else f'Serial chiqishi {SERIAL_CHEGARA} qatordan') +
               ' oshdi - ishga tushirish to\'xtatildi: nimadir TO\'XTOVSIZ takrorlanyapti' +
               (f'. Eng ko\'p: {vektor_nomi(eng[0][0], eng[0][1])} - {eng[1]} marta' if eng else ''))
        if eng and eng[0][0] < 32 and eng[0][1] == 0:
            h.masl('Bir xil istisno qayta-qayta: ishlovchi qaytgach CPU o\'sha buyruqni QAYTA bajaradi (fault - RIP '
                   'xato qilgan buyruqni ko\'rsatadi). Sababni tuzatmasdan qaytmang: yo RIP ni o\'zgartiring, yo '
                   'jarayonni to\'xtating/panic qiling.')
    if n.vaqt_tugadi:
        h.info(f'Vaqt chegarasi tugadi. Oxirgi holat: RIP=0x{n.reg.get("RIP", 0):x}'
               + (f' ({belgi(n.elf, n.reg["RIP"])})' if n.reg.get('RIP') and belgi(n.elf, n.reg['RIP']) else '')
               + (', CPU hlt da uxlayapti' if n.reg.get('HLT') else ', CPU ishlayapti (sikl?)')
               + (f', IF={1 if n.reg.get("RFL", 0) & 0x200 else 0}' if 'RFL' in n.reg else ''))
    if n.chiqdi and not n.qatorlar and n.qemu_xato.strip() and not n.log.voqealar:
        h.xato('QEMU ishga tushmadi: ' + n.qemu_xato.strip().splitlines()[-1])
        return
    if not n.qatorlar:
        h.xato('Serialdan hech narsa chiqmadi')
        h.masl('Serial: COM1 = port 0x3F8; bayt yuborishdan oldin 0x3FD ning 5-biti (THR bo\'sh) ni kuting. '
               'QEMU portni sozlashsiz ham qabul qiladi, demak birinchi navbatda: `_start` umuman '
               'ishlayaptimi? `--gdb` bilan `break *0x<entry>` qo\'yib ko\'ring. Yadro kirish nuqtasi: '
               f'{hex(elf_entry(n.elf) or 0)}.')

    kutilmagan = [v for v in n.log.istisnolar() if not bosqich_kutadimi(bosqich_raqam, v)]
    if kutilmagan:
        h.info('Kutilmagan istisnolar (QEMU logidan):\n' + '\n'.join(
            '      ' + v.tavsif(belgi(n.elf, v.pc)) for v in kutilmagan[:5])
            + (f'\n      ... yana {len(kutilmagan) - 5} ta' if len(kutilmagan) > 5 else ''))


def bosqich_kutadimi(raqam, v):
    """Bu istisno shu bosqich testining bir qismimi (B4: 3/6/0, B6: #PF, B11: #GP cpl=3)."""
    if raqam == 4 and v.v in (0, 3, 6) and v.cpl == 0:
        return True
    if raqam == 6 and v.v == 14 and v.cr2 == 0x500000000000:
        return True
    if raqam == 11 and v.v == 13 and v.cpl == 3 and v.e == 0:
        return True
    return False


# ---------- B2: kprintf ----------

B2_KUTILGAN = [
    ('B2 d=-42 u=3000000000 x=beef s=satr c=Z p=0xffffffff80001000 foiz=%', True),
    ('B2 min=-2147483648 max=4294967295 lx=123456789abcdef', True),
    ('B2 [   42] [42   ] [0000beef]', False),                     # ixtiyoriy: kenglik
]

B2_MASLAHAT = {
    'd': '%d manfiy son: avval "-" ni chiqaring, keyin musbat qismini.',
    'u': '%u: argumentni `unsigned` sifatida oling (va_arg(ap, unsigned)); int sifatida olsangiz '
         '3000000000 manfiy bo\'lib qoladi.',
    'x': '%x: kichik harflar (beef), 0x prefiksisiz, oldida nol yo\'q.',
    's': '%s: va_arg(ap, const char *) va \\0 gacha chiqarish.',
    'c': '%c: va_arg(ap, int) - char emas! (variadik argumentlarda char int ga ko\'tariladi).',
    'p': '%p: "0x" + kichik harfli hex, 64 bit: (uintptr_t)va_arg(ap, void *).',
    'foiz': '%%: bitta % chiqadi, argument olinmaydi.',
    'min': 'INT_MIN: -(-2147483648) int da sig\'maydi (UB)! Musbat qismini unsigned da hisoblang: '
           '0u - (unsigned)v.',
    'max': '%u da UINT_MAX = 4294967295; int sifatida o\'qilsa -1 chiqadi.',
    'lx': '%lx: "l" modifikatori - va_arg(ap, unsigned long). Unsiz yuqori 32 bit yo\'qoladi.',
}


def b2_tekshir(n, h):
    for kutilgan, majburiy in B2_KUTILGAN:
        tokenlar = kutilgan.split(' ')
        bor = kutilgan in n.matnlar()
        if bor:
            (h.ok if majburiy else (lambda s: h.ixt(True, s)))(f'B2 kprintf: {kutilgan!r}')
            continue
        if not majburiy:
            h.ixt(False, f'B2 kenglik/to\'ldirish (ixtiyoriy ★): kutilgan {kutilgan!r}')
            continue
        oxshash = [q for q in n.matnlar() if q.startswith(' '.join(tokenlar[:2]).split('=')[0])]
        if not oxshash:
            h.xato(f'B2 kprintf: {kutilgan!r} - qator chiqmadi')
            h.masl('B2: kprintf qatorlari mustaqil/README.md dagi aniq chaqiruvlar bilan chiqishi kerak.')
            continue
        olindi = oxshash[0]
        h.xato(f'B2 kprintf:\n           kutilgan: {kutilgan!r}\n           olindi  : {olindi!r}')
        olindi_t = olindi.split(' ')
        for i, tok in enumerate(tokenlar):
            if i >= len(olindi_t) or olindi_t[i] != tok:
                kalit = tok.split('=')[0]
                if kalit in B2_MASLAHAT:
                    h.masl(f'B2 ({tok} o\'rniga {olindi_t[i] if i < len(olindi_t) else "hech narsa"!r}): '
                           + B2_MASLAHAT[kalit])


# ---------- B3: Multiboot2 ----------

def b3_tekshir(n, h):
    m = qator_kut(n, h, r'^B3 cmdline=', 'B3: "B3 cmdline=..." qatori')
    if m:
        q = [x for x in n.matnlar() if x.startswith('B3 cmdline=')][0]
        if not h.tekshir(f'kalit={n.kalit}' in q, f'B3: buyruq qatorida kalit={n.kalit} bor',
                         f'B3: buyruq qatorida kalit={n.kalit} yo\'q: {q!r}'):
            h.masl('B3: Multiboot2 ma\'lumotida 1-turdagi teg (cmdline) ni toping: teglar 8 baytga '
                   'tekislangan, har biri {u32 type, u32 size, ...}; keyingisi = (size + 7) & ~7. '
                   'Ma\'lumot manzili _start da EBX da keladi - uni saqlab, kmain ga uzating.')
    m = qator_kut(n, h, r'^B3 ram=(\d+) KB$', 'B3: "B3 ram=<N> KB" qatori')
    if m:
        kb = int(m.group(1))
        yaxshi = 120000 <= kb <= RAM_MB * 1024
        if not h.tekshir(yaxshi, f'B3: RAM = {kb} KB (QEMU -m {RAM_MB}M uchun to\'g\'ri)',
                         f'B3: RAM = {kb} KB - kutilgan 120000..{RAM_MB * 1024} KB'):
            if kb > RAM_MB * 1024:
                h.masl('B3: RAM juda katta: faqat type == 1 (available) yozuvlarni qo\'shing; band '
                       '(reserved, ACPI) hududlar RAM emas.')
            elif kb < 1024:
                h.masl('B3: RAM juda kichik: faqat birinchi yozuvni (0..640 KB) hisoblayapsiz. Teg 6 '
                       '(memory map) ichida entry_size qadami bilan hamma yozuvlarni aylaning.')
            else:
                h.masl('B3: uzunlik 64 bitli (u64 base_addr, u64 length, u32 type, u32 reserved). '
                       'Yozuvlar orasidagi qadam - teg ichidagi entry_size, sizeof(struct) emas.')


# =============================================================================
#  5. Bosqichlar
# =============================================================================

class Bosqich:
    def __init__(self, nom, sarlavha, tavsif, tekshiruv, tugash=None, interaktiv=None,
                 interaktiv_sharti=None):
        self.nom = nom
        self.raqam = int(nom[1:])
        self.sarlavha = sarlavha
        self.tavsif = tavsif
        self.tekshiruv = tekshiruv
        self._tugash = tugash
        self.interaktiv = interaktiv
        self.interaktiv_sharti = interaktiv_sharti

    def tugadimi(self, n):
        if self._tugash:
            return self._tugash(n)
        return any(q == f'{self.nom} TUGADI' for q in n.matnlar())


def t_b0(n, h):
    banner_tekshir(n, h, 0)


def t_b1(n, h):
    banner_tekshir(n, h, 1)
    efer, cr0, cr4 = n.reg.get('EFER'), n.reg.get('CR0'), n.reg.get('CR4')
    if efer is None:
        h.xato('B1: QEMU monitoridan registrlarni o\'qib bo\'lmadi')
        return
    h.tekshir(efer & (1 << 10), f'B1: EFER.LMA = 1 (long mode faol), EFER=0x{efer:x}',
              f'B1: EFER.LMA = 0 - CPU long mode da EMAS (EFER=0x{efer:x})')
    h.tekshir(cr0 & (1 << 31), 'B1: CR0.PG = 1 (sahifalash yoqilgan)', f'B1: CR0.PG = 0 (CR0=0x{cr0:x})')
    h.tekshir(cr4 & (1 << 5), 'B1: CR4.PAE = 1', f'B1: CR4.PAE = 0 (CR4=0x{cr4:x})')
    if not efer & (1 << 10):
        h.masl('B1: long mode tartibi (Intel SDM 3A, "Initializing IA-32e Mode"): PAE yoqish (CR4.PAE) -> CR3 = PML4 '
               'manzili -> EFER.LME = 1 (MSR 0xC0000080, 8-bit) -> CR0.PG = 1 -> 64-bit kod segmentiga '
               'far jump. LME bor-u LMA yo\'q bo\'lsa - paging yoqilmagan.')


def t_b2(n, h):
    banner_tekshir(n, h, 2)


def t_b3(n, h):
    banner_tekshir(n, h, 3)


def t_b4(n, h):
    banner_tekshir(n, h, 3)
    m3 = qator_kut(n, h, r'^B4 vektor=3 rip=0x([0-9a-f]+)$', 'B4: int3 ishlovchisi "B4 vektor=3 rip=0x..."')
    q3 = qator_kut(n, h, r'^B4 int3 dan qaytdi$', 'B4: int3 dan keyin iretq bilan qaytish')
    m6 = qator_kut(n, h, r'^B4 vektor=6 rip=0x([0-9a-f]+)$', 'B4: #UD ishlovchisi "B4 vektor=6 rip=0x..."')
    q6 = qator_kut(n, h, r'^B4 ud2 dan qaytdi$', 'B4: ud2 dan keyin qaytish (RIP += 2)')
    qator_kut(n, h, r'^B4 vektor=0 rip=0x([0-9a-f]+)$', 'B4: #DE ishlovchisi "B4 vektor=0 rip=0x..."')
    qator_kut(n, h, r'^B4 TUGADI$', 'B4: "B4 TUGADI"')
    # log: haqiqiy istisnolarmi?
    l3, l6, l0 = n.log.vektor(3, cpl=0), n.log.vektor(6, i=0, cpl=0), n.log.vektor(0, i=0, cpl=0)
    h.tekshir(l3 and l6 and l0, 'B4: QEMU logida haqiqatan #BP, #UD va #DE bo\'ldi',
              f'B4: QEMU logida #BP={len(l3)}, #UD={len(l6)}, #DE={len(l0)} ta - har biri kamida 1 bo\'lishi kerak')
    if m6 and l6:
        h.tekshir(int(m6.group(1), 16) == l6[0].pc,
                  'B4: #UD da chiqarilgan rip = ud2 buyrug\'ining manzili',
                  f'B4: #UD rip=0x{int(m6.group(1), 16):x}, lekin CPU 0x{l6[0].pc:x} da xato qildi')
    if m3 and l3:
        rip = int(m3.group(1), 16)
        h.tekshir(rip in (l3[0].pc, l3[0].pc + 1),
                  'B4: int3 da chiqarilgan rip to\'g\'ri (int3 - "trap": rip KEYINGI buyruqni ko\'rsatadi)',
                  f'B4: int3 rip=0x{rip:x}, logda 0x{l3[0].pc:x}')
    idt_lim = n.reg.get('IDT_LIM')
    if idt_lim is not None:
        h.ixt(idt_lim >= 16 * 32 - 1, f'B4: IDT chegarasi 0x{idt_lim:x} ({(idt_lim + 1) // 16} ta yozuv, '
              'kamida 32 ta istisno uchun joy)')
    # maslahatlar
    if m3 and not q3:
        gp = [v for v in n.log.istisnolar() if v.v in (13, 8)]
        h.masl('B4: int3 ishlovchisi ishladi, lekin qaytolmadi. Stekdagi freym iretq kutgan '
               'ko\'rinishda emas: (1) ishlovchi oxirida push qilingan hamma narsa pop qilindimi? '
               '(2) xato kodi faqat 8, 10-14, 17, 21, 29, 30 vektorlarda CPU tomonidan qo\'yiladi - '
               'qolganlarida stub o\'zi 0 qo\'yishi kerak, aks holda `add rsp, 16` noto\'g\'ri; '
               '(3) `iret` emas, `iretq` (64-bit).' +
               (f' Logda: {gp[0].tavsif(belgi(n.elf, gp[0].pc))}' if gp else ''))
    if m6 and not q6:
        if len(n.log.vektor(6, i=0)) > 3:
            h.masl(f'B4: #UD {len(n.log.vektor(6, i=0))} marta takrorlandi: ishlovchi RIP ni o\'zgartirmagan, '
                   'shuning uchun CPU yana o\'sha ud2 ni bajaryapti. ud2 = 2 bayt (0F 0B): saqlangan '
                   'freymdagi rip ni 2 ga oshiring.')
    if not m3:
        df = n.log.vektor(8, i=0)
        gp = n.log.vektor(13, i=0)
        if gp and gp[0].e & 2:
            h.masl(f'B4: #GP xato kodi 0x{gp[0].e:x}: IDT ning {gp[0].e >> 3}-yozuvi noto\'g\'ri. '
                   'IDT yozuvi 16 bayt (SDM 3A, "64-Bit Mode IDT"): offset 3 bo\'lakda '
                   '(15:0, 31:16, 63:32), selektor = yadro kod segmenti (masalan 0x08), type_attr = 0x8E '
                   '(P=1, DPL=0, 64-bit interrupt gate). Yoki `lidt` umuman bajarilmagan / IDTR chegarasi '
                   'kichik: `--gdb` bilan to\'xtatib, QEMU monitorida `info registers` -> IDT= qatori.')
        elif df:
            h.masl('B4: double fault: CPU birinchi istisno ishlovchisini ishga tushira olmadi - IDT '
                   'manzili/chegarasi (`lidt` operandi: 2 bayt limit + 8 bayt baza, packed) yoki '
                   'yozuvdagi selektor noto\'g\'ri.')


def t_b5(n, h):
    banner_tekshir(n, h, 3)
    m = qator_kut(n, h, r'^B5 birinchi=(\d+) ikkinchi=(\d+) takror=(\d+) tekis_emas=(\d+)$',
                  'B5: "B5 birinchi=.. ikkinchi=.. takror=.. tekis_emas=.." qatori')
    qator_kut(n, h, r'^B5 TUGADI$', 'B5: "B5 TUGADI"')
    if not m:
        return
    n1, n2, tak, tek = (int(x) for x in m.groups())
    kb = n1 * 4
    h.tekshir(100000 <= kb <= RAM_MB * 1024, f'B5: {n1} ta kadr = {kb} KB bo\'sh xotira',
              f'B5: {n1} ta kadr = {kb} KB - kutilgan 100000..{RAM_MB * 1024} KB')
    if not h.tekshir(n1 == n2, 'B5: hammasini bo\'shatgandan keyin yana shuncha kadr olindi (oqish yo\'q)',
                     f'B5: birinchi={n1}, ikkinchi={n2} - bo\'shatish hamma kadrni qaytarmadi'):
        h.masl('B5: free() kadrni qaytarmayapti yoki noto\'g\'ri bitni tozalayapti: bitmap indeksi = manzil / 4096; '
               'ro\'yxatli PMM da - qaytarilgan kadr ro\'yxat boshiga qo\'shiladimi?')
    h.tekshir(tak == 0, 'B5: bitta kadr ikki marta berilmadi', f'B5: takror={tak} - bitta kadr ikki marta berildi')
    h.tekshir(tek == 0, 'B5: hamma kadr 4096 ga tekislangan', f'B5: tekis_emas={tek}')
    if kb > RAM_MB * 1024:
        h.masl('B5: mavjud bo\'lmagan xotira berildi: faqat mmap dagi type=1 hududlar va ular ichida '
               'to\'liq sig\'adigan 4 KB kadrlar (boshini yuqoriga, oxirini pastga yaxlitlang).')
    elif kb < 100000:
        h.masl('B5: kadrlar kam: (1) 1 MB dan yuqoridagi katta hududni qo\'shdingizmi? (2) yadro, Multiboot2 '
               'ma\'lumoti va modullardan tashqari joylarni band qilib qo\'ymadingizmi?')
    if tak:
        h.masl('B5: takror kadr - ko\'pincha yadro tasviri yoki bitmap/ro\'yxatning o\'zi joylashgan xotira '
               'bo\'sh deb belgilangan. `_kernel_end` (linker skripti) gacha va Multiboot2 ma\'lumotini band qiling.')


def t_b6(n, h):
    banner_tekshir(n, h, 3)
    a = qator_kut(n, h, r'^B6 alias=([0-9a-f]+)$', 'B6: "B6 alias=..." (ikki virtual manzil - bitta kadr)')
    if a:
        h.tekshir(int(a.group(1), 16) == 0x1122334455667788, 'B6: alias=1122334455667788 - xaritalash ishlaydi',
                  f'B6: alias={a.group(1)} - B orqali A ga yozilgan qiymat ko\'rinmadi')
    m = qator_kut(n, h, r'^B6 #PF cr2=0x([0-9a-f]+) err=(?:0x)?([0-9a-f]+)$', 'B6: "B6 #PF cr2=0x... err=..."')
    qator_kut(n, h, r'^B6 TUGADI$', 'B6: "B6 TUGADI"')
    if m:
        cr2, err = int(m.group(1), 16), int(m.group(2), 16)
        h.tekshir(cr2 == 0x500000000000, 'B6: CR2 = 0x500000000000 (xato manzili to\'g\'ri o\'qildi)',
                  f'B6: cr2=0x{cr2:x}, kutilgan 0x500000000000')
        lpf = [v for v in n.log.vektor(14, i=0) if v.cr2 == 0x500000000000]
        h.tekshir(lpf and lpf[0].e == 2, 'B6: QEMU logida ham shu #PF (CR2, e=0002) bor',
                  'B6: QEMU logida CR2=0x500000000000, e=0002 bo\'lgan #PF yo\'q')
        h.tekshir(err == 2, 'B6: err=0x2 (sahifa yo\'q, YOZISH, yadro rejimi)',
                  f'B6: err=0x{err:x} [{pf_kodi(err)}], kutilgan 0x2 [{pf_kodi(2)}]')
        if err & 1:
            h.masl('B6: err ning 0-biti 1: sahifa hali BOR. unmap yozuvni (PTE) nolga tushirdimi? Balki '
                   'boshqa yozuvni tozalayapsiz (indeks hisobi: (va >> 12) & 511).')
        if cr2 != 0x500000000000:
            h.masl('B6: CR2 ni `mov %%cr2, %0` bilan ISHLOVCHI BOSHIDA o\'qing (keyingi #PF uni almashtiradi).')
    lp = [v for v in n.log.vektor(14, i=0) if v.cr2 == 0x500000000000]
    if a and not m and not lp:
        h.masl('B6: unmap dan keyin yozish xatosiz o\'tdi: TLB eski tarjimani eslab qolgan! PTE ni '
               'o\'chirgandan keyin `invlpg [va]` qiling (yoki CR3 ni qayta yuklang). QEMU TLB ni ham '
               'emulyatsiya qiladi - shuning uchun bu xato bu yerda ham ko\'rinadi.')
    if not a:
        pf = n.log.vektor(14, i=0)
        if pf:
            h.masl('B6: xaritalashning o\'zida #PF: ' + pf[0].tavsif(belgi(n.elf, pf[0].pc)) +
                   '. Yangi jadval kadrlarini ishlatishdan oldin ularga yozish mumkinmi (identity yoki '
                   'HHDM orqali)? Jadval yozuvida fizik manzil + P|W bitlari bormi? Yangi jadvalni '
                   'nol bilan to\'ldirdingizmi?')
    if lp and lp[0].e != 2 and not m:
        h.masl('B6: logda #PF bor, lekin qator chiqmadi - #PF ishlovchingiz (14-vektor, xato kodi BILAN) qatorni '
               'chiqarishi kerak.')


def t_b7(n, h):
    banner_tekshir(n, h, 3)
    tiklar = n.hammasi(r'^B7 tik=(\d+)$')
    qiymatlar = [int(m.group(1)) for m, _ in tiklar]
    h.tekshir(qiymatlar[:10] == list(range(10, 101, 10)), 'B7: tik=10, 20, ..., 100 ketma-ket chiqdi',
              f'B7: tik qatorlari: {qiymatlar[:12]} - kutilgan [10, 20, ..., 100]')
    qator_kut(n, h, r'^B7 TUGADI$', 'B7: "B7 TUGADI"')
    if len(tiklar) >= 10 and qiymatlar[0] == 10 and qiymatlar[9] == 100:
        dt = tiklar[9][1] - tiklar[0][1]
        h.tekshir(0.3 <= dt <= 5.0, f'B7: 90 tik {dt:.2f} s da (100 Hz da ~0.9 s kutiladi)',
                  f'B7: 90 tik {dt:.2f} s da - 100 Hz da ~0.9 s bo\'lishi kerak (PIT bo\'luvchisi = 1193182 / 100)')
    irq0 = [v for v in n.log.voqealar if v.v == 0x20 and v.i == 0]
    if len(qiymatlar) < 10:
        if len(irq0) == 1:
            h.masl('B7: taymer uzilishi faqat BIR MARTA keldi: ishlovchi oxirida PIC ga EOI (`outb(0x20, 0x20)`) '
                   'yuborilmagan. EOI bo\'lmasa PIC keyingi uzilishni yubormaydi.')
        elif not irq0:
            sakkiz = [v for v in n.log.voqealar if v.v == 8 and v.i == 0]
            if sakkiz:
                h.masl('B7: logda 8-vektor bor: PIC qayta raqamlanmagan (sukut bo\'yicha IRQ0 = vektor 8 = '
                       'double fault bilan bir xil!). ICW1..ICW4 bilan master PIC ni 0x20 ga, slave ni 0x28 ga ko\'chiring.')
            else:
                h.masl('B7: birorta taymer uzilishi kelmadi: (1) `sti` qildingizmi? (2) PIC niqobi (port 0x21) '
                       'IRQ0 ni ochadimi (0-bit = 0)? (3) PIT: port 0x43 ga 0x36, keyin 0x40 ga bo\'luvchining '
                       'past va yuqori bayti. (4) IDT da 0x20-yozuv bormi?')
        elif len(irq0) > 5 and not tiklar:
            h.masl(f'B7: {len(irq0)} ta taymer uzilishi keldi, lekin "B7 tik=" chiqmadi - hisoblagich yoki chiqarish '
                   'mantiqini tekshiring (hisoblagich `volatile` mi?).')


def b8_tugmalar(mon):
    for t in ('s', 'a', 'l', 'o', 'm', 'ret'):
        mon.buyruq(f'sendkey {t}')
        time.sleep(0.08)


def t_b8(n, h):
    banner_tekshir(n, h, 3)
    if not qator_kut(n, h, r'^B8 tayyor$', 'B8: "B8 tayyor" (klaviatura uzilishi yoqildi)'):
        return
    m = qator_kut(n, h, r'^B8 satr=(.*)$', 'B8: "B8 satr=..." (Enter bosilganda)')
    qator_kut(n, h, r'^B8 TUGADI$', 'B8: "B8 TUGADI"')
    irq1 = [v for v in n.log.voqealar if v.v == 0x21 and v.i == 0]
    if m:
        satr = m.group(1)
        if not h.tekshir(satr == 'salom', 'B8: satr=salom (s, a, l, o, m, Enter bosildi)',
                         f'B8: satr={satr!r}, kutilgan \'salom\''):
            if len(satr) == 10 or (satr and len(set(satr)) < len(satr) and 'ss' in satr):
                h.masl('B8: har harf ikki marta: tugma QO\'YIB YUBORILGANDA ham kod keladi (bit 7 = 1, '
                       '"break code"). Ularni o\'tkazib yuboring.')
            else:
                h.masl('B8: skan-kodlar jadvali: QEMU PS/2 klaviaturasi "set 1" kodlarini beradi '
                       '(s=0x1F, a=0x1E, l=0x26, o=0x18, m=0x32, Enter=0x1C). Jadval indeksi = skan-kod.')
    elif len(irq1) == 1:
        h.masl('B8: klaviatura uzilishi bir marta keldi: (1) EOI yuborilmagan, yoki (2) port 0x60 dan '
               'skan-kod O\'QILMAGAN - o\'qilmaguncha kontroller yangi bayt bermaydi.')
    elif not irq1:
        h.masl('B8: klaviatura uzilishi kelmadi: PIC niqobida IRQ1 (1-bit) ochiqmi? IDT da 0x21-yozuv bormi? `sti`?')


def t_b9(n, h):
    banner_tekshir(n, h, 3)
    m = qator_kut(n, h, r'^B9 p1=0x([0-9a-f]+) p2=0x([0-9a-f]+) p3=0x([0-9a-f]+)$', 'B9: "B9 p1=.. p2=.. p3=.."')
    if m:
        p = [int(x, 16) for x in m.groups()]
        h.tekshir(all(x % 16 == 0 for x in p), 'B9: kmalloc manzillari 16 ga tekislangan',
                  f'B9: tekislanmagan manzil: {[hex(x) for x in p]} - x86-64 ABI 16 bayt talab qiladi')
        oraliq = sorted(zip(p, (1, 24, 100)))
        kesishmaydi = all(a + s <= b for (a, s), (b, _) in zip(oraliq, oraliq[1:]))
        h.tekshir(kesishmaydi and all(p), 'B9: p1, p2, p3 bir-biri bilan kesishmaydi',
                  f'B9: bloklar kesishadi yoki NULL: {[hex(x) for x in p]}')
    s = qator_kut(n, h, r'^B9 ajratish=(\d+) xato=(\d+)$', 'B9: "B9 ajratish=N xato=0" (stress test)')
    if s:
        h.tekshir(int(s.group(2)) == 0 and int(s.group(1)) >= 600, f'B9: {s.group(1)} ta ajratish, buzilish yo\'q',
                  f'B9: ajratish={s.group(1)} xato={s.group(2)} - kamida 600 ta, xato=0 bo\'lishi kerak')
        if int(s.group(2)):
            h.masl('B9: bloklar bir-birini bosib ketdi: ajratishda sarlavha hajmini qo\'shdingizmi? Bo\'lishda '
                   '(split) yangi blok boshi = eski blok + sarlavha + so\'ralgan hajm (tekislangan).')
    qator_kut(n, h, r'^B9 katta=ok$', 'B9: "B9 katta=ok" (1 MB ajratish)')
    qator_kut(n, h, r'^B9 double-free ushlandi$', 'B9: "B9 double-free ushlandi"')
    qator_kut(n, h, r'^B9 TUGADI$', 'B9: "B9 TUGADI"')


def t_b10(n, h):
    banner_tekshir(n, h, 3)
    ab = [(m.group(1), int(m.group(2))) for m, _ in n.hammasi(r'^B10 ([AB])(\d)$')]
    a = [i for t, i in ab if t == 'A']
    b = [i for t, i in ab if t == 'B']
    h.tekshir(a == list(range(5)), 'B10: A oqimi 0..4 ni tartib bilan chiqardi', f'B10: A oqimi: {a}')
    h.tekshir(b == list(range(5)), 'B10: B oqimi 0..4 ni tartib bilan chiqardi', f'B10: B oqimi: {b}')
    almashish = sum(1 for x, y in zip(ab, ab[1:]) if x[0] != y[0])
    if not h.tekshir(almashish >= 3, f'B10: oqimlar {almashish} marta almashdi (preemption ishlaydi)',
                     f'B10: oqimlar faqat {almashish} marta almashdi - taymer oqimni majburan to\'xtatmayapti'):
        h.masl('B10: oqimlar yield QILMAYDI (faol kutish) - demak almashtirishni TAYMER uzilishi qilishi kerak: '
               'taymer ishlovchisida EOI dan keyin schedule() ni chaqiring.')
    qator_kut(n, h, r'^B10 TUGADI$', 'B10: "B10 TUGADI" (ikkala oqim tugagach)')
    aralash = [q for q in n.matnlar() if q.count('B10') > 1 or (q.startswith('B10') and not re.match(
        r'^B10 ([AB]\d|TUGADI)$', q))]
    if aralash:
        h.xato(f'B10: qatorlar aralashib ketgan: {aralash[0]!r}')
        h.masl('B10: ikki oqim bir vaqtda serialga yozdi: kprintf o\'rtasida taymer oqimni almashtirdi. '
               'Bitta qatorni chiqarishni atomar qiling (uzilishlarni o\'chirib/qulf bilan).')
    if not ab:
        tf = n.log.istisnolar()
        if tf:
            h.masl('B10: birinchi kontekst almashishda xato: ' + tf[0].tavsif(belgi(n.elf, tf[0].pc)) +
                   '. Yangi oqim stekini switch funksiyangiz POP qiladigan tartibda tayyorladingizmi? '
                   'Oxirgi `ret` oqim funksiyasiga (yoki trampolinga) borishi kerak. Yangi oqim uzilish '
                   'ichidan boshlanadi - IF=0: trampolinda `sti` kerak.')


def t_b11(n, h):
    banner_tekshir(n, h, 3)
    qator_kut(n, h, r'^B11 ring3 salom$', 'B11: ring 3 dan syscall orqali "B11 ring3 salom"')
    qator_kut(n, h, r'^B11 #GP cpl=3', 'B11: ring 3 dagi `cli` -> #GP, "B11 #GP cpl=3"')
    qator_kut(n, h, r'^B11 TUGADI$', 'B11: "B11 TUGADI"')
    gp3 = [v for v in n.log.vektor(13, i=0, cpl=3) if v.e == 0]
    ring3 = any(v.cpl == 3 for v in n.log.voqealar)
    h.tekshir(gp3, 'B11: QEMU logida cpl=3 dagi #GP bor - kod haqiqatan ring 3 da ishladi',
              'B11: QEMU logida `cli` dan #GP (cpl=3, xato kodi 0) yo\'q - ' +
              ('kod ring 3 ga yetdi, lekin `cli` gacha bormadi' if ring3 else 'kod ring 3 ga yetmadi'))
    tr = n.reg.get('TR')
    if tr is not None:
        h.tekshir(tr != 0 and 'TSS64' in n.reg.get('TR_TUR', ''), f'B11: TSS yuklangan (TR=0x{tr:x})',
                  'B11: TR = 0 - `ltr` qilinmagan: ring 3 dan uzilishda CPU yadro stekini (rsp0) TSS dan oladi')
    ist = n.log.istisnolar()
    for v in ist:
        if v.v == 13 and v.e == 0x80 * 8 + 2:
            h.masl('B11: `int 0x80` ring 3 dan #GP berdi: IDT ning 0x80-yozuvida DPL=3 bo\'lishi kerak '
                   '(type_attr = 0xEE), aks holda user uni chaqira olmaydi.')
            break
        if v.v == 13 and v.cpl == 0:
            h.masl('B11: ring 3 ga o\'tishda (iretq) #GP: ' + v.tavsif(belgi(n.elf, v.pc)) +
                   '. iretq freymi: SS, RSP, RFLAGS, CS, RIP (shu tartibda push). CS = user kod selektori | 3, '
                   'SS = user data selektori | 3; GDT dagi bu segmentlarda DPL=3, kodda L=1.')
            break
        if v.v == 14 and v.cpl == 3:
            h.masl('B11: ring 3 da #PF: ' + v.tavsif() + '. User sahifasida U bit (2-bit) BARCHA '
                   'darajalarda (PML4E, PDPTE, PDE, PTE) bo\'lishi kerak. Bajarilayotgan kod/stek '
                   'xaritalanganmi?')
            break
        if v.v in (8, 10):
            h.masl('B11: ring 3 dan yadroga o\'tishda xato (' + vektor_nomi(v.v) + '): TSS.rsp0 to\'g\'ri yadro '
                   'steki tepasini ko\'rsatadimi? GDT dagi TSS deskriptori 16 bayt (64-bit), turi 0x9.')
            break


def t_b12(n, h):
    banner_tekshir(n, h, 3)
    qator_kut(n, h, r'^B12 ELF dan salom$', 'B12: dastur ishga tushdi "B12 ELF dan salom"')
    for q in ('B12 bss=0', 'B12 data=42'):
        qator_kut(n, h, '^' + re.escape(q) + '$', f'B12: "{q}"')
    qator_kut(n, h, r'^B12 exit=7$', 'B12: exit(7) -> yadro "B12 exit=7" chiqardi')
    qator_kut(n, h, r'^B12 TUGADI$', 'B12: "B12 TUGADI"')
    matn = n.matnlar()
    if 'B12 bss=xato' in matn:
        h.masl('B12: .bss nol emas: p_memsz > p_filesz bo\'lsa, faylda yo\'q qismini NOL bilan to\'ldiring '
               '(PMM bergan kadr toza bo\'lishi shart emas).')
    if 'B12 data=xato' in matn:
        h.masl('B12: .data noto\'g\'ri: segment fayldagi p_offset dan nusxalanadimi? '
               '(p_vaddr % 4096 ni sahifa ichidagi siljish sifatida hisobga oling).')
    if any('write noto\'g\'ri' in q for q in matn):
        h.masl('B12: write natijasi user ning rax iga qaytishi kerak: saqlangan freymdagi rax ni o\'zgartiring.')
    for v in n.log.istisnolar(cpl=3):
        if v.v == 14:
            if v.e & 2 and v.e & 1:
                h.masl('B12: user dasturi o\'z .data siga yoza olmadi: PF_W (p_flags & 2) bo\'lgan segmentga '
                       'W bit bering.')
            else:
                h.masl(f'B12: user #PF: {v.tavsif()}. Segment manzili {hex(USER_BAZA)} atrofida - '
                       'PT_LOAD larni (p_vaddr, p_memsz) to\'liq xaritaladingizmi, U bit bilan?')
            break
    if not any(q.startswith('B12 ELF') for q in matn):
        h.masl('B12: dastur ishga tushmadi. Modul: Multiboot2 da 3-turdagi teg (mod_start, mod_end, '
               'satr "dastur"). DIQQAT: PMM modul xotirasini bo\'sh deb bermasligi kerak - aks holda ELF '
               'o\'qilishidan oldin ustiga yozilib ketadi. Kirish nuqtasi: e_entry.')


BOSQICHLAR = [
    Bosqich('B0', 'Yuklash va serial', 'GRUB yadroingizni yuklaydi, serialga "B0 salom" chiqadi', t_b0,
            tugash=lambda n: any(q == 'B0 salom' for q in n.matnlar())),
    Bosqich('B1', 'Long mode va C', '64-bitli kmain: "B1 kmain"; EFER.LMA, CR0.PG, CR4.PAE monitor orqali', t_b1,
            tugash=lambda n: any(q.startswith('B1 kmain') for q in n.matnlar())),
    Bosqich('B2', 'kprintf', '%d %u %x %s %c %p %% %lx, INT_MIN, (★) kenglik', t_b2,
            tugash=lambda n: any(q.startswith('B2 min=') or q.startswith('B3 ') for q in n.matnlar())),
    Bosqich('B3', 'Multiboot2 ma\'lumoti', 'buyruq qatori va xotira xaritasi (RAM hajmi)', t_b3,
            tugash=lambda n: any(q.startswith('B3 ram=') for q in n.matnlar())),
    Bosqich('B4', 'IDT va istisnolar', 'int3 -> qaytish, ud2 -> RIP+2, #DE', t_b4),
    Bosqich('B5', 'Fizik xotira (PMM)', 'hamma kadrlarni olish, qaytarish, takrorsiz', t_b5),
    Bosqich('B6', 'Sahifalash (VMM)', 'map/unmap, alias, invlpg, #PF (CR2, xato kodi)', t_b6),
    Bosqich('B7', 'PIC + PIT taymer', '100 Hz, EOI, vaqtni o\'lchash', t_b7),
    Bosqich('B8', 'Klaviatura', 'IRQ1, skan-kodlar; vosita "salom" ni o\'zi teradi', t_b8,
            interaktiv=b8_tugmalar,
            interaktiv_sharti=lambda n: any(q == 'B8 tayyor' for q in n.matnlar())),
    Bosqich('B9', 'Heap (kmalloc)', 'tekislash, stress, 1 MB, double free', t_b9),
    Bosqich('B10', 'Oqimlar va preemption', 'ikki oqim, kontekst almashish, taymer bilan', t_b10),
    Bosqich('B11', 'User rejimi', 'TSS, ring 3, syscall, ring 3 da #GP', t_b11),
    Bosqich('B12', 'ELF yuklovchi', 'Multiboot2 modulidagi dasturni yuklash va ishga tushirish', t_b12),
]


def bosqich_top(nom):
    nom = nom.upper()
    if not nom.startswith('B'):
        nom = 'B' + nom
    for b in BOSQICHLAR:
        if b.nom == nom:
            return b
    sys.exit(f'Noma\'lum bosqich: {nom}. Ro\'yxat: tools/mustaqil.py royxat')


# =============================================================================
#  6. Buyruqlar
# =============================================================================

def sozlama_oqi():
    try:
        with open(SOZLAMA) as f:
            return json.load(f)
    except (OSError, ValueError):
        return {}


def elf_yol(args):
    s = sozlama_oqi()
    if args.elf:
        elf = os.path.abspath(args.elf)
        s['elf'] = elf
        with open(SOZLAMA, 'w') as f:
            json.dump(s, f)
    else:
        elf = s.get('elf')
    if not elf:
        sys.exit('Yadro ELF fayli ko\'rsatilmagan: tools/mustaqil.py tekshir B0 --elf ~/yadrom/build/yadro.elf')
    if not os.path.isfile(elf):
        sys.exit(f'{elf} topilmadi - avval yadroingizni yig\'ing.')
    return elf


def hisobot_chiqar(bosqich, n, h):
    print(qalin(f'=== {bosqich.nom}: {bosqich.sarlavha} ==='))
    for tur, s in h.qatorlar:
        belgi_ = {'ok': yashil('[ OK ]'), 'xato': qizil('[XATO]'), 'ixt': sariq('[ ★  ]'),
                  'ixt_ok': yashil('[ ★OK]'), 'info': sariq('[info]')}[tur]
        print(f'  {belgi_} {s}')
    if h.maslahat:
        print(qalin('\n  Maslahatlar:'))
        for m in h.maslahat:
            print('   - ' + m)
    if not h.otdi:
        print(f'\n  Serial chiqishi va QEMU logi: {os.path.relpath(OXIRGI, os.getcwd())}/'
              ' (serial.txt, int.log). Tahlil: tools/mustaqil.py log')
    print('\n  Natija: ' + (yashil(qalin('O\'TDI')) if h.otdi else qizil('XATO')))
    keyingi = [b for b in BOSQICHLAR if b.raqam == bosqich.raqam + 1]
    if h.otdi and keyingi:
        print(f'  Keyingi: {keyingi[0].nom} - {keyingi[0].sarlavha} (mustaqil/README.md). '
              f'Avval: git tag {bosqich.nom}, va MyOS\'dagi mos kod bilan solishtiring.')
    print()


def baholash(bosqich, n):
    h = Hisobot()
    umumiy_tahlil(n, h, bosqich.raqam)
    if not any(t == 'xato' and s.startswith(('GRUB', 'QEMU ishga')) for t, s in h.qatorlar):
        bosqich.tekshiruv(n, h)
    return h


def cmd_tekshir(args):
    elf = elf_yol(args)
    bosqich = bosqich_top(args.bosqich)
    n = ishga_tushir(bosqich, elf, args.vaqt, gdb=args.gdb, oyna=args.oyna, korsat=args.korsat, abi=args.abi)
    h = baholash(bosqich, n)
    hisobot_chiqar(bosqich, n, h)
    return 0 if h.otdi else 1


def cmd_hammasi(args):
    elf = elf_yol(args)
    holat = []
    for b in BOSQICHLAR:
        print(f'{b.nom} ...', flush=True)
        n = ishga_tushir(b, elf, args.vaqt, abi=args.abi)
        h = baholash(b, n)
        holat.append((b, h.otdi))
        if not h.otdi:
            hisobot_chiqar(b, n, h)
            break
    print(qalin('Mustaqil yadro - holat:'))
    for b in BOSQICHLAR:
        r = next((o for x, o in holat if x is b), None)
        belgi_ = yashil('[ OK ]') if r else (qizil('[XATO]') if r is False else '[    ]')
        print(f'  {belgi_} {b.nom:4} {b.sarlavha}')
    otgan = sum(1 for _, o in holat if o)
    print(f'\n  {otgan}/{len(BOSQICHLAR)} bosqich.')
    return 0 if otgan == len(BOSQICHLAR) else 1


def cmd_royxat(_args):
    print(qalin('Mustaqil yadro bosqichlari') + ' (protokol va o\'qish: mustaqil/README.md)\n')
    for b in BOSQICHLAR:
        print(f'  {b.nom:4} {b.sarlavha:24} {b.tavsif}')
    print('\nTekshirish: tools/mustaqil.py tekshir B0 --elf yol/yadro.elf')
    return 0


def cmd_log(_args):
    try:
        with open(os.path.join(OXIRGI, 'int.log'), errors='replace') as f:
            log = Log(f.read())
        with open(os.path.join(OXIRGI, 'bosqich.txt')) as f:
            nom, elf = f.read().split('\n')[:2]
    except OSError:
        sys.exit('Hali ishga tushirilmagan: tools/mustaqil.py tekshir ...')
    print(qalin(f'Oxirgi ishga tushirish: {nom}, {elf}\n'))
    sanoq = {}
    for v in log.voqealar:
        k = (v.v, v.i, v.cpl)
        sanoq[k] = sanoq.get(k, 0) + 1
    print('Voqealar (vektor bo\'yicha):')
    for (v, i, cpl), s in sorted(sanoq.items()):
        print(f'  {s:6} x  v=0x{v:02x} cpl={cpl}  {vektor_nomi(v, i)}')
    ist = log.istisnolar()
    if ist:
        print('\nIstisnolar (birinchi 10 ta, vaqt tartibida):')
        for v in ist[:10]:
            print('  ' + v.tavsif(belgi(elf, v.pc)))
    if log.triple:
        print(qizil('\nTRIPLE FAULT') + ' - CPU qayta yuklandi.')
    print(f'\nXom log: {os.path.join(OXIRGI, "int.log")}  (har voqeadan keyin to\'liq registrlar)')
    return 0


# =============================================================================
#  7. selftest: vositaning o'zi to'g'ri ishlaydimi (QEMU'siz - CI uchun)
# =============================================================================

NAMUNA_LOG = """\
SMM: enter
EAX=00000000 EBX=00000000 ECX=02000000 EDX=02000628
     0: v=03 e=0000 i=1 cpl=0 IP=0008:0000000000101234 pc=0000000000101234 SP=0010:0000000000107f80 env->regs[R_EAX]=0000000000000000
RAX=0000000000000000 RBX=0000000000000000 RCX=0000000000000000 RDX=0000000000000000
CR0=80000011 CR2=0000000000000000 CR3=0000000000102000 CR4=00000020
EFER=0000000000000500
     1: v=06 e=0000 i=0 cpl=0 IP=0008:0000000000101240 pc=0000000000101240 SP=0010:0000000000107f80 env->regs[R_EAX]=0000000000000000
check_exception old: 0xffffffff new 0xe
     2: v=0e e=0002 i=0 cpl=0 IP=0008:0000000000101300 pc=0000000000101300 SP=0010:0000000000107f80 CR2=0000500000000000
     3: v=20 e=0000 i=0 cpl=0 IP=0008:0000000000101400 pc=0000000000101400 SP=0010:0000000000107f80 env->regs[R_EAX]=0000000000000000
     4: v=0d e=0402 i=0 cpl=3 IP=001b:0000008000000010 pc=0000008000000010 SP=0023:000000ffffffefe8 env->regs[R_EAX]=0000000000000001
check_exception old: 0xd new 0xb
     5: v=08 e=0000 i=0 cpl=0 IP=0008:0000000000101500 pc=0000000000101500 SP=0010:0000000000107f80 env->regs[R_EAX]=0000000000000000
check_exception old: 0x8 new 0xd
Triple fault
CPU Reset (CPU 0)
"""

NAMUNA_REG = """\
RAX=0000000000000000 RBX=0000000000000000 RCX=0000000000000000 RDX=0000000000000000
RIP=0000000000101abc RFL=00000202 [-------] CPL=0 II=0 A20=1 SMM=0 HLT=1
CS =0008 0000000000000000 ffffffff 00af9a00 DPL=0 CS64 [-R-]
TR =0028 0000000000108000 00000067 00008b00 DPL=0 TSS64-busy
GDT=     0000000000105000 00000037
IDT=     0000000000106000 00000fff
CR0=80000011 CR2=0000500000000000 CR3=0000000000102000 CR4=00000020
EFER=0000000000000500
"""


def cmd_selftest(_args):
    xatolar = []

    def kut(shart, nom):
        if not shart:
            xatolar.append(nom)

    log = Log(NAMUNA_LOG)
    kut(len(log.voqealar) == 6, f'voqealar soni {len(log.voqealar)} != 6')
    kut(log.triple, 'triple fault topilmadi')
    kut([v.v for v in log.istisnolar()] == [6, 14, 13, 8], f'istisnolar: {[v.v for v in log.istisnolar()]}')
    kut(log.vektor(3, i=1) and not log.vektor(3, i=1)[0].istisnomi, 'int3 (i=1) istisno deb hisoblandi')
    pf = log.vektor(14)[0]
    kut(pf.cr2 == 0x500000000000 and pf.e == 2, 'CR2/xato kodi o\'qilmadi')
    kut(log.voqealar[0].reg('EFER') == 0x500, 'voqeadan keyingi registr o\'qilmadi')
    kut(log.vektor(13, cpl=3)[0].sp == 0xffffffefe8, 'SP o\'qilmadi')
    kut('IDT ning 128' in log.vektor(13)[0].tavsif(), '#GP IDT indeksi dekodlanmadi')
    kut('YOZISH' in pf_kodi(2) and 'YO\'Q' in pf_kodi(2) and 'yadro' in pf_kodi(2), 'pf_kodi(2)')
    kut('user' in pf_kodi(7) and 'BOR' in pf_kodi(7), 'pf_kodi(7)')
    r = registrlar(NAMUNA_REG)
    kut(r.get('EFER') == 0x500 and r.get('CR0') == 0x80000011, f'registrlar: {r}')
    kut(r.get('TR') == 0x28 and 'TSS64' in r.get('TR_TUR', ''), f'TR: {r}')
    kut(r.get('IDT_LIM') == 0xfff and r.get('HLT') == 1 and r.get('RIP') == 0x101abc, f'IDT/HLT/RIP: {r}')
    kut(tozala('\x1b[2J\x1b[HB0 salom\r') == 'B0 salom', 'ANSI tozalash')

    # B2 hisoboti: INT_MIN xatosi to'g'ri maslahat beradimi?
    n = Natija()
    n.qatorlar = [(0, 'B0 salom'), (0, 'B1 kmain'), (0, B2_KUTILGAN[0][0]),
                  (0, 'B2 min=-0 max=4294967295 lx=123456789abcdef'), (0, f'B3 cmdline=mustaqil=B3 kalit={n.kalit}'),
                  (0, 'B3 ram=130687 KB')]
    h = Hisobot()
    b2_tekshir(n, h)
    kut(not h.otdi and any('INT_MIN' in m for m in h.maslahat), 'B2 INT_MIN maslahati')
    h = Hisobot()
    b3_tekshir(n, h)
    kut(h.otdi, f'B3: {h.qatorlar}')

    # B7: EOI maslahati
    n = Natija()
    n.log = Log('     0: v=20 e=0000 i=0 cpl=0 IP=0008:0000000000101400 pc=0000000000101400 SP=0010:00000000001'
                '07f80 env->regs[R_EAX]=0000000000000000\n')
    h = Hisobot()
    t_b7(n, h)
    kut(any('EOI' in m for m in h.maslahat), 'B7 EOI maslahati')

    # B10: almashish soni
    n = Natija()
    n.qatorlar = [(0, f'B10 {t}{i}') for i in range(5) for t in 'AB'] + [(0, 'B10 TUGADI')]
    h = Hisobot()
    t_b10(n, h)
    kut(any('almashdi (preemption' in s for _, s in h.qatorlar), f'B10: {h.qatorlar}')

    # hamma bosqichning tekshiruvi bo'sh natijada qulamaydi
    for b in BOSQICHLAR:
        try:
            baholash(b, Natija())
        except Exception as e:   # noqa: BLE001 - selftest hamma xatoni ko'rsatishi kerak
            xatolar.append(f'{b.nom}: bo\'sh natijada istisno: {e!r}')

    for f in ('dastur.asm', 'dastur.ld'):
        kut(os.path.isfile(os.path.join(MUSTAQIL, f)), f'mustaqil/{f} yo\'q')
    if shutil.which('nasm') and shutil.which('ld'):
        with tempfile.TemporaryDirectory() as d:
            for abi in ('int80', 'syscall'):
                try:
                    elf = dastur_yigish(d, abi)
                    kut(elf_entry(elf) == USER_BAZA, f'dastur.elf ({abi}) kirish nuqtasi {hex(elf_entry(elf) or 0)}')
                except subprocess.CalledProcessError as e:
                    xatolar.append(f'dastur.asm ({abi}) yig\'ilmadi: {e}')

    if xatolar:
        print(qizil('selftest: XATO'))
        for x in xatolar:
            print('  - ' + x)
        return 1
    print(yashil('selftest: OK') + f' (log tahlili, registrlar, maslahatlar, {len(BOSQICHLAR)} bosqich, dastur.asm)')
    return 0


def main():
    ap = argparse.ArgumentParser(
        description='O\'z yadroingizni bosqichma-bosqich tekshirish (mustaqil/README.md)')
    sub = ap.add_subparsers(dest='buyruq')
    t = sub.add_parser('tekshir', help='bitta bosqichni tekshirish')
    t.add_argument('bosqich')
    for p in (t, sub.add_parser('hammasi', help='B0 dan birinchi xatogacha')):
        p.add_argument('--elf', help='yadroingizning Multiboot2 ELF fayli (eslab qolinadi)')
        p.add_argument('--vaqt', type=float, default=30.0, help='kutish chegarasi, soniya')
        p.add_argument('--abi', choices=('int80', 'syscall'), default='int80', help='B12 dasturining syscall usuli')
    t.add_argument('--gdb', action='store_true', help='QEMU gdb ni kutadi (:1234)')
    t.add_argument('--oyna', action='store_true', help='QEMU oynasini ko\'rsatish')
    t.add_argument('--korsat', action='store_true', help='QEMU buyrug\'i va grub.cfg ni chop etish')
    sub.add_parser('royxat', help='bosqichlar ro\'yxati')
    sub.add_parser('log', help='oxirgi ishga tushirish logining tahlili')
    sub.add_parser('selftest', help='vositaning o\'zini tekshirish (CI)')
    args = ap.parse_args()
    buyruqlar = {'tekshir': cmd_tekshir, 'hammasi': cmd_hammasi, 'royxat': cmd_royxat,
                 'log': cmd_log, 'selftest': cmd_selftest}
    if args.buyruq not in buyruqlar:
        ap.print_help()
        return 1
    return buyruqlar[args.buyruq](args)


if __name__ == '__main__':
    sys.exit(main())
