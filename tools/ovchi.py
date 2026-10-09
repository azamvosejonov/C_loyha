#!/usr/bin/env python3
# =============================================================================
#  tools/ovchi.py - XATO OVCHISI: yadroga yashirin xato kiritiladi, siz topasiz
# =============================================================================
#
#  G'OYA: noldan yadro yozganda eng ko'p vaqt kod YOZISHGA emas, xato QIDIRISHGA
#  ketadi - va yadro xatosi ko'pincha jim: ekran qotadi, QEMU qayta yuklanadi,
#  sabab esa xato ko'ringan joydan ancha uzoqda. Bu ko'nikmani faqat mashq bilan
#  o'stirish mumkin.
#
#  Har bir "ov" MyOS'ning bitta faylida 1-2 qatorni haqiqiy, ko'p uchraydigan
#  xatoga almashtiradi (qaysi fayl - aytilmaydi). Siz faqat ALOMATNI bilasiz.
#  Vositalar: serial log, `-d int`, QEMU monitori, gdb, addr2line, git bisect
#  emas (o'zgarish commit qilinmagan!) - va bosh.
#
#  Buyruqlar:
#      tools/ovchi.py royxat            ovlar ro'yxati va holati
#      tools/ovchi.py boshla <nom>      xatoni kiritish (asl fayl .ovchi/ ga saqlanadi)
#      tools/ovchi.py maslahat          faol ov uchun keyingi maslahat (har chaqiruvda bittadan)
#      tools/ovchi.py tekshir           yig'ish + QEMU testi: xato tuzatildimi?
#      tools/ovchi.py javob <nom>       tuzatgandan keyin: nima edi, nega shunday alomat berdi
#      tools/ovchi.py tiklash           asl faylni qaytarish (taslim bo'lish yoki tugatgach)
#      tools/ovchi.py selfcheck         har bir ov hozirgi kodga qo'llanishini tekshirish (CI)
#
#  Qoidalar:
#   1. `git diff` va `git status` - TAQIQLANGAN (bu "javobni ko'rish"). Xuddi shu
#      sababli kiritilgan qatorlar bu faylda base64 bilan yashirilgan.
#   2. Avval GIPOTEZA, keyin tajriba. Har ovni `xatolar.md` daftaringizga yozing:
#      alomat -> gipotezalar -> qaysi tajriba nimani ko'rsatdi -> sabab.
#   3. Ov faol turganda commit qilmang; lab'lar bilan bir vaqtda ishlatmang.
# =============================================================================
import base64
import difflib
import glob
import json
import os
import re
import shutil
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
HOLAT_DIR = os.path.join(ROOT, '.ovchi')
HOLAT = os.path.join(HOLAT_DIR, 'holat.json')

RANG = sys.stdout.isatty() and os.environ.get('NO_COLOR') is None


def rang(kod, s):
    return f'\033[{kod}m{s}\033[0m' if RANG else s


# Ovlar. Har birida:
#   alomat     - nimani ko'rasiz (sinab ko'rilgan)
#   qayta      - xatoni qanday takrorlash
#   maslahat   - zinapoya: avval umumiy usul, oxirida deyarli javob
#   tekshiruv  - ('test', [tools/test.sh dagi tekshiruv nomlari - hammasi OK bo'lishi kerak])
#                ('demo', APPEND, regex - shu chiqishi kerak)
#   sabab      - `javob` ko'rsatadigan tushuntirish
#   o          - [fayl, eski (base64), yangi (base64)]
OVLAR = [
    {
        'nom': 'cpl',
        'sarlavha': '"Yadroda" xato - lekin RIP user dasturida',
        'qiyinlik': 2,
        'alomat': 'Selftest o\'tadi, keyin birinchi user dasturida: "Yadroda qayta ishlanmagan exception (Page '
                  'Fault)" va KERNEL PANIC. Lekin xabarning o\'zida: "user rejimida", CS=23, RIP=0x400...',
        'qayta': 'make run-nographic',
        'maslahat': [
            'Xabardagi qarama-qarshilikni toping: panic "yadroda" deydi, Sabab qatori va CS esa boshqa narsani. '
            'CS=0x23 ning qaysi bitlari CPL (imtiyoz darajasi)? (SDM: "Segment Selectors")',
            'User rejimidagi page fault normal holat (demand paging, COW, stek o\'sishi) - u mm_handle_fault ga '
            'borishi kerak edi. kernel/arch/interrupts.c: istisno user\'dan kelganini qanday aniqlaydi?',
        ],
        'tekshiruv': ('test', ['shell user rejimida ishga tushdi (/etc/motd)', 'fork + copy-on-write + exec + demand paging',
                               'NULL ga yozish ushlandi', 'yadro panic bo\'lmasligi kerak']),
        'sabab': 'Segment selektori = indeks (3-15 bitlar) + TI (2-bit) + RPL (0-1 bitlar). User kod selektori '
                 '0x23 = indeks 4, RPL 3. "User\'danmi?" degan savolga faqat RPL bitlari javob beradi: '
                 '(cs & 3) == 3. Butun selektorni 3 bilan solishtirish hech qachon rost bo\'lmaydi - shuning '
                 'uchun har bir user page fault (oddiy demand paging ham) "yadro xatosi" deb panic\'ga '
                 'olib boriladi. Saboq: apparat maydonlarini bitlar bilan o\'qing - spetsifikatsiyadagi '
                 'rasmni ochib, qaysi bit nima ekanini tekshiring.',
        'o': ['kernel/arch/interrupts.h',
              'ICAgIHJldHVybiAoZnJhbWUtPmNzICYgMykgPT0gMzsK',
              'ICAgIHJldHVybiBmcmFtZS0+Y3MgPT0gMzsK'],
    },
    {
        'nom': 'eoi',
        'sarlavha': 'Tizim yuklanishda qotib qoladi',
        'qiyinlik': 2,
        'alomat': 'Yadro "[int] Uzilishlar yoqilmoqda" gacha yuklanadi va... qotadi (selftest bilan - selftest '
                  'o\'rtasida). Panic yo\'q, triple fault yo\'q, QEMU ishlab turibdi.',
        'qayta': 'make run-nographic   (chiqish: Ctrl-A, X)',
        'maslahat': [
            'Qotgan tizimda birinchi savol: CPU NIMA qilyapti? QEMU monitori (make run da Ctrl-Alt-2, yoki '
            '-monitor stdio): `info registers` - RIP qayerda (addr2line -f -e build/kernel.elf <RIP>), '
            'IF yoqilganmi (RFL ning 9-biti), HLT=1 mi?',
            'CPU hlt da uxlayapti va IF=1 - demak u UZILISHNI kutyapti, lekin uzilish kelmayapti. Kim '
            'kelishi kerak edi? Oxirgi chiqqan xabar yonidagi kod nimani kutyapti (taymer tiki? disk?)',
            '`-d int -D int.log` bilan ishga tushiring: taymer uzilishi (LAPIC taymer vektori - '
            'kernel/arch/interrupts.h) logda necha marta bor? Bir martami?',
            'LAPIC keyingi uzilishni qachon yuboradi? docs/10-smp.md, "EOI". Har bir IRQ yo\'li EOI ga '
            'yetib boradimi - shartlarni diqqat bilan o\'qing.',
        ],
        'tekshiruv': ('test', ['shell user rejimida ishga tushdi (/etc/motd)', 'poweroff']),
        'sabab': 'LAPIC (va PIC) har bir apparat uzilishidan keyin EOI ("ishlov tugadi") ni kutadi. EOI '
                 'kelmaguncha shu va undan past ustuvorlikdagi uzilishlarni yubormaydi. Shart teskari '
                 'yozilgan: EOI faqat syscall (0x80) uchun yuborilardi - aslida unga EOI KERAK EMAS '
                 '(dasturiy uzilish), qolgan hammasiga kerak. Natija: birinchi taymer tikidan keyin '
                 'jimlik. Saboq: "qotdi" - bu ko\'pincha "uzilish kelmayapti"; logda uzilishlarni sanang.',
        'o': ['kernel/arch/interrupts.c',
              'ICAgICAgICBpZiAodiAhPSBWRUNUT1JfU1lTQ0FMTCkKICAgICAgICAgICAgbGFwaWNfZW9pKCk7Cg==',
              'ICAgICAgICBpZiAodiA9PSBWRUNUT1JfU1lTQ0FMTCkKICAgICAgICAgICAgbGFwaWNfZW9pKCk7Cg=='],
    },
    {
        'nom': 'ist',
        'sarlavha': 'Stek to\'lishi xabarsiz qayta yuklanishga aylandi',
        'qiyinlik': 3,
        'alomat': '`APPEND=demo=stack` (cheksiz rekursiya) avval chiroyli "Double Fault" panic xabarini '
                  'berardi. Endi QEMU shunchaki qayta yuklanadi (yoki -no-reboot bilan to\'xtaydi) - hech '
                  'qanday xabarsiz.',
        'qayta': 'make run-nographic APPEND=demo=stack   (yoki QEMU\'ga -no-reboot -d int,cpu_reset -D int.log)',
        'maslahat': [
            '`-d int,cpu_reset -D int.log -no-reboot`: oxirgi 3 ta istisno qaysilar (v=0e, v=08, ...)? '
            'Har birida SP qayerda?',
            'Double fault ishlovchisi ishga tushishi uchun CPU stekka freym yozishi kerak. Stek to\'lib '
            'bo\'lgan bo\'lsa-chi? CPU qaysi stekni ishlatadi? (SDM: "Interrupt Stack Table")',
            'IDT ning 8-yozuvi qanday sozlangan va TSS dagi IST stekining manzili-chi? (idt.c, gdt.c)',
        ],
        'tekshiruv': ('demo', 'demo=stack', r'Double Fault'),
        'sabab': 'Yadro steki to\'lganda #PF bo\'ladi, lekin ishlovchi uchun freymni ham o\'sha (to\'lgan) '
                 'stekka yozib bo\'lmaydi -> #DF. #DF ham o\'sha stekni ishlatsa - yana xato -> triple fault '
                 '-> CPU reset. Shuning uchun #DF ga IDT da IST=1 beriladi: CPU TSS.ist[0] dagi alohida, '
                 'toza stekka o\'tadi. IST maydoni 0 qilib qo\'yilgan edi. Saboq: istisno ishlovchisining '
                 'o\'zi ishlashi uchun nima kerakligini (stek, kod, IDT) doim so\'rang.',
        'o': ['kernel/arch/idt.c',
              'ICAgIGlkdF9zZXRfZ2F0ZSg4LCBpc3Jfc3R1Yl90YWJsZVs4XSwgMSwgSURUX0lOVEVSUlVQVF9HQVRFKTsK',
              'ICAgIGlkdF9zZXRfZ2F0ZSg4LCBpc3Jfc3R1Yl90YWJsZVs4XSwgMCwgSURUX0lOVEVSUlVQVF9HQVRFKTsK'],
    },
    {
        'nom': 'ext2',
        'sarlavha': 'e2fsck yadro yozgan diskni yoqtirmaydi',
        'qiyinlik': 3,
        'alomat': 'Hamma narsa ishlayotganga o\'xshaydi - fayllar yoziladi va o\'qiladi. Lekin testdan keyin '
                  '`e2fsck` disk tuzilmasida xatolar topadi; katta fayllar ba\'zan buziladi.',
        'qayta': 'make test  (oxirida e2fsck natijasi; disk: build/test-disk-bios.img)',
        'maslahat': [
            'e2fsck nima deydi? `e2fsck -fn build/test-disk-bios.img` - xabarlarni o\'qing: qaysi '
            'tuzilma (blok bitmap? inode? papka?) va qanday farq (band, lekin belgilanmagan / aksincha)?',
            '`debugfs -R "stat /yangi/seq.txt" build/test-disk-bios.img` - fayl bloklari ro\'yxati va '
            '`debugfs -R "testb <blok>"` - shu bloklar bitmap\'da band deb belgilanganmi?',
            'Blok ajratish: bitmap\'dagi bit raqami -> blok raqami formulasi. Qo\'lda bitta misolni hisoblang '
            '(docs/13-disk-ext2.md, s_first_data_block).',
        ],
        'tekshiruv': ('test', ['e2fsck: yadro yozgan ext2 buzilmagan', 'yadro panic bo\'lmasligi kerak']),
        'sabab': 'Ajratuvchi bitmap\'da N-bitni band qiladi, lekin chaqiruvchiga N+1-blokni qaytaradi '
                 '(off-by-one). Natija: fayl belgilanmagan blokni ishlatadi (keyin boshqa fayl ham uni '
                 'oladi - ikki fayl bitta blokda), belgilangan blok esa hech kimniki emas. e2fsck aynan shu '
                 'ikki xil nomuvofiqlikni ko\'radi. Saboq: tashqi "hakam" (e2fsck, fsck, valgrind) - '
                 'o\'z kodingizni tekshirishning eng ishonchli yo\'li.',
        'o': ['kernel/fs/ext2.c',
              'ICAgICAgICAgICAgcmVzdWx0ID0gZmlyc3QgKyBnICogYnBnICsgKHVpbnQzMl90KWJpdDsK',
              'ICAgICAgICAgICAgcmVzdWx0ID0gZmlyc3QgKyBnICogYnBnICsgKHVpbnQzMl90KWJpdCArIDE7Cg=='],
    },
    {
        'nom': 'rsp0',
        'sarlavha': 'Birinchi user dasturida Double Fault, RSP=0',
        'qiyinlik': 3,
        'alomat': 'Yadro to\'liq yuklanadi (selftest ham o\'tadi), lekin birinchi user dasturi ishga tushishi '
                  'bilan: "EXCEPTION 8: Double Fault", RSP=0, keyin KERNEL PANIC.',
        'qayta': 'make run-nographic',
        'maslahat': [
            'Double fault - oqibat, sabab emas. Undan OLDINGI istisno qaysi? QEMU\'ni `-d int -D int.log '
            '-no-reboot` bilan ishga tushiring va v=08 dan oldingi 2-3 voqeani o\'qing: cpl nechchi, SP qayerda?',
            'RSP=0: CPU yadro stekini qayerdan oldi? Ring 3 da ishlayotgan kodga uzilish kelganda (cpl=3 -> 0) '
            'CPU yangi stek manzilini QAYERDAN o\'qiydi? (docs/07, SDM: "Task Management in 64-bit Mode")',
            'QEMU monitori: `info registers` -> TR= qatori: TSS manzili. `x/4gx <TSS manzili>` - rsp0 maydoni '
            'nechaga teng? Uni kim, qachon yozishi kerak edi (gdb: `break tss_set_kernel_stack`)?',
        ],
        'tekshiruv': ('test', ['shell user rejimida ishga tushdi (/etc/motd)', 'fork + copy-on-write + exec + demand paging',
                               'yadro panic bo\'lmasligi kerak']),
        'sabab': 'Ring 3 da ishlayotgan jarayonga uzilish (taymer, page fault) kelganda CPU yadro stekini '
                 'TSS.rsp0 dan oladi. Jarayonga o\'tishda rsp0 yozilmagan - u 0 bo\'lib qolgan: CPU freymni 0 '
                 'dan pastga yozmoqchi bo\'ladi -> #PF -> uni ham yozib bo\'lmaydi -> #DF (u IST stekida ishlaydi, '
                 'shuning uchun xabar chiqdi). Agar rsp0 boshqa jarayonniki bo\'lib qolganda edi - xato yana '
                 'ham yomon bo\'lardi: o\'sha jarayonning yadro steki jimgina buzilardi. Saboq: "per-jarayon" '
                 'apparat holati (CR3, rsp0, FS bazasi) almashish paytida HAMMASI yangilanishi kerak; '
                 'syscall yo\'li (gs:CPU_KERNEL_RSP) va uzilish yo\'li (TSS.rsp0) - ikki xil manba.',
        'o': ['kernel/proc/process.c',
              'ICAgICAgICAgICAgdHNzX3NldF9rZXJuZWxfc3RhY2soa3N0YWNrX3RvcChwKSk7ICAgIC8qIHJpbmczIC0+IHJpbmcwIHZhIHN5c2NhbGwgc3Rla2kgKi8K',
              ''],
    },
    {
        'nom': 'switch',
        'sarlavha': 'Uzilishlar yoqilishi bilan RIP=0',
        'qiyinlik': 3,
        'alomat': 'Yadro "[int] Uzilishlar yoqilmoqda" gacha yuklanadi, keyin darhol: EXCEPTION 14 (Page Fault), '
                  'CR2=0, RIP=0, "instruksiya o\'qishda". Backtrace deyarli bo\'sh.',
        'qayta': 'make run-nographic',
        'maslahat': [
            'RIP == CR2 == 0: CPU 0 manzilidagi kodni bajarmoqchi bo\'lgan - kimdir 0 ga SAKRATGAN (ret, '
            'jmp reg, call reg). Uzilishlar yoqilgandan keyin birinchi bo\'ladigan muhim voqea nima? (taymer -> '
            'scheduler -> ...)',
            'Yangi oqim/jarayon qanday boshlanadi: stekda nima tayyorlanadi va birinchi marta unga '
            'qanday "qaytiladi"? (docs/06, kernel/proc/process.c dagi stek tayyorlash va switch.asm)',
            'gdb: `break kthread_start` va `break new_proc_start` - u yerga kelgandagi registrlar '
            'tayyorlangan qiymatlarga mosmi? `info registers r12 r13 r14`',
        ],
        'tekshiruv': ('test', ['yadro ichki testlari (buddy, slab, vmalloc, vmm, scheduler)',
                               'shell user rejimida ishga tushdi (/etc/motd)', 'yadro panic bo\'lmasligi kerak']),
        'sabab': 'context_switch registrlarni stekdan push tartibiga TESKARI tartibda pop qilishi kerak. '
                 'Ikki pop o\'rin almashgan: r12 va r13 qiymatlari almashib qoladi. Yangi oqimda r12 = '
                 'argument, r13 = kirish manzili (process.c shunday tayyorlaydi) - almashganda oqim '
                 'noto\'g\'ri argument oladi, jarayon esa `jmp r13` bilan noto\'g\'ri manzilga sakraydi. '
                 'Saboq: assembly\'dagi "shartnoma" (qaysi registr nima) - C struct kabi: ikki tomon bir xil '
                 'tartibni kutadi; bir tomonni o\'zgartirsangiz, ikkinchisi jim buziladi.',
        'o': ['kernel/proc/switch.asm',
              'ICAgIHBvcCByMTMKICAgIHBvcCByMTIK',
              'ICAgIHBvcCByMTIKICAgIHBvcCByMTMK'],
    },
    {
        'nom': 'swapgs',
        'sarlavha': 'Panic xabari sababni emas, oqibatni ko\'rsatadi',
        'qiyinlik': 3,
        'alomat': 'Selftest o\'tadi, birinchi user dasturi ishga tushadi va - yadroda Page Fault: RIP mavjud '
                  'bo\'lmagan, tushunarsiz manzil, "instruksiya o\'qishda". Backtrace hech narsa bermaydi.',
        'qayta': 'make run-nographic',
        'maslahat': [
            'Panic\'dagi RIP - oxirgi OQIBAT. Birinchi istisnoni toping: QEMU\'ni `-d int -D int.log -no-reboot` '
            'bilan ishga tushiring, `grep -m3 "v=0e" int.log` - birinchisining RIP ini addr2line qiling, '
            'CR2 va SP ga qarang.',
            'Birinchi #PF da CR2 kichik son (0x10) - qaysidir "baza + siljish" da baza 0. SP esa USER steki '
            '(0x7fff...) - yadro hali o\'z stekiga o\'tmagan. Yadro per-CPU ma\'lumotiga GS orqali kiradi; '
            'GS bazasi kirish va chiqish yo\'llarida necha marta almashtiriladi? Har yo\'lda juft bo\'lishi kerak.',
            'gdb: `break syscall_entry`, 1- va 2-marta to\'xtaganda QEMU monitorida `info registers` -> GS= '
            'qatori. Birinchi syscall\'dan QAYTISHNING ikki yo\'li bor (sysret va iretq) - qaysi biri ishlatildi?',
        ],
        'tekshiruv': ('test', ['shell user rejimida ishga tushdi (/etc/motd)', 'yadro panic bo\'lmasligi kerak']),
        'sabab': '`syscall` kirishda `swapgs` qiladi (GS -> yadro per-CPU), tez qaytish yo\'lida `sysret` '
                 'oldidan ikkinchi `swapgs` bo\'lishi kerak (GS -> user). U olib tashlangan edi: user dasturi '
                 'yadro GS bazasi bilan qaytadi, keyingi `syscall` dagi `swapgs` esa bazani USER qiymatiga '
                 '(0) almashtiradi - `[gs:...]` 0 atrofidagi manzilga murojaat qiladi (hali user stekida!). #PF '
                 'ishlovchisi ham per-CPU ma\'lumotni GS orqali o\'qiydi - yana #PF, va hokazo: stek yeb '
                 'boriladi, oxirida CPU axlat manzilga sakraydi. Panic xabari shu OXIRGI qadamni ko\'rsatadi; '
                 '`-d int` logi esa BIRINCHISINI. Saboq: simmetrik '
                 'amallar (lock/unlock, swapgs/swapgs, push/pop) har bir yo\'lda juft bo\'lishi kerak - '
                 'ayniqsa ikki xil qaytish yo\'li bo\'lganda (sysret va iretq).',
        'o': ['kernel/arch/syscall_entry.asm',
              'ICAgIG1vdiByc3AsIFtyc3AgKyA0MF0gICAgICAgICAgICAgICAgIDsgdXNlciBzdGVraQogICAgc3dhcGdzICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgOyBHUyAtPiB1c2VyCg==',
              'ICAgIG1vdiByc3AsIFtyc3AgKyA0MF0gICAgICAgICAgICAgICAgIDsgdXNlciBzdGVraQo='],
    },
    {
        'nom': 'cow',
        'sarlavha': 'Shell hech ochilmaydi',
        'qiyinlik': 4,
        'alomat': 'Yadro yuklanadi, selftest o\'tadi, panic yo\'q - lekin shell ochilmaydi: init cheksiz '
                  '"[init] shell tugadi (kod ...) - yangisi ochilmoqda" deb yozadi.',
        'qayta': 'make run-nographic',
        'maslahat': [
            'Shell signal bilan emas, oddiy exit bilan tugayapti (init shunday deydi). Qayerda? gdb: '
            '`break proc_exit` (yoki sys_exit) - `bt` va chiqish kodi. Shell umuman main ga yetadimi '
            '(`break` user manzilida qiyin - avval exec natijasini tekshiring)?',
            'init: fork -> bola exec("/bin/sh"). Bola exec qilganda eski manzil maydoni (otaniki bilan '
            'bo\'lishilgan sahifalar!) bo\'shatiladi. Ota (init) sahifalari bunga qanday "omon qoladi"?',
            'fork + copy-on-write: ota va bola bir xil fizik sahifani bo\'lishadi. Sahifa qachon bo\'shatiladi? '
            'Kim buni hisoblaydi? (docs/11-fork-cow.md)',
            'Gipoteza: sahifa hali kimdir ishlatayotganda bo\'shatilyapti. Tekshirish: bola exit qilganda '
            'bo\'shatilgan sahifalar sonini va ota hali xaritalagan sahifalarni solishtiring (kprintf).',
            'struct page dagi hisoblagichni fork qanday o\'zgartiradi va COW page fault / exit qanday '
            'kamaytiradi? Oshirish va kamaytirish soni teng bo\'lishi kerak.',
        ],
        'tekshiruv': ('test', ['fork + copy-on-write + exec + demand paging', 'yadro panic bo\'lmasligi kerak']),
        'sabab': 'fork sahifani bolaga ham xaritalaydi, lekin refcount\'ni oshirmaydi. Birinchi bo\'lib exit '
                 'qilgan (yoki COW nusxa olgan) jarayon refcount\'ni 0 ga tushiradi va sahifani buddy\'ga '
                 'qaytaradi - ikkinchisi esa hali undan foydalanyapti. Sahifa boshqa maqsadga beriladi va '
                 'ikki egasi bir-birining ma\'lumotini buzadi (use-after-free, lekin fizik sahifa '
                 'darajasida). Saboq: umumiy resursning har bir "egasi" hisoblagichda aks etishi kerak.',
        'o': ['kernel/mm/mm.c',
              'ICAgICAgICAgICAgICAgICAgICBzdHJ1Y3QgcGFnZSAqcGcgPSBwaHlzX3RvX3BhZ2UocHRlICYgUFRFX0FERFJfTUFTSyk7CiAgICAgICAgICAgICAgICAgICAgZ2V0X3BhZ2UocGcpOwo=',
              'ICAgICAgICAgICAgICAgICAgICBzdHJ1Y3QgcGFnZSAqcGcgPSBwaHlzX3RvX3BhZ2UocHRlICYgUFRFX0FERFJfTUFTSyk7Cg=='],
    },
]


def b64(s):
    return base64.b64decode(s).decode()


def ov_top(nom):
    for o in OVLAR:
        if o['nom'] == nom:
            return o
    sys.exit(f'Noma\'lum ov: {nom}. Ro\'yxat: tools/ovchi.py royxat')


def holat_oqi():
    try:
        with open(HOLAT) as f:
            return json.load(f)
    except (OSError, ValueError):
        return {'faol': None, 'maslahat': 0, 'yechilgan': []}


def holat_yoz(h):
    os.makedirs(HOLAT_DIR, exist_ok=True)
    with open(HOLAT, 'w') as f:
        json.dump(h, f, indent=1)


def asl_yol(o):
    return os.path.join(HOLAT_DIR, o['nom'] + '.asl')


def cmd_royxat(_a):
    h = holat_oqi()
    print('Xato ovchisi - yadroga yashirin xato kiritiladi, siz topasiz (mustaqil/README.md)\n')
    for o in OVLAR:
        if o['nom'] == h['faol']:
            belgi = rang('33', '[FAOL]')
        elif o['nom'] in h['yechilgan']:
            belgi = rang('32', '[ OK ]')
        else:
            belgi = '[    ]'
        print(f'  {belgi} {o["nom"]:8} {"*" * o["qiyinlik"]:5} {o["sarlavha"]}')
    print('\nBoshlash: tools/ovchi.py boshla <nom>   (tavsiya: cpl, eoi, ist, ext2, rsp0, switch, swapgs, cow)')
    return 0


def faol_lablar():
    return glob.glob(os.path.join(ROOT, '.lab', '*.yechim'))


def cmd_boshla(a):
    o = ov_top(a.nom)
    h = holat_oqi()
    if h['faol']:
        sys.exit(f'"{h["faol"]}" ovi faol. Avval: tools/ovchi.py tekshir  yoki  tools/ovchi.py tiklash')
    if faol_lablar():
        sys.exit('Faol lab bor (.lab/): u tizimni boshqacha buzadi. Avval: tools/lab.py tiklash <nom>')
    fayl, eski, yangi = o['o'][0], b64(o['o'][1]), b64(o['o'][2])
    yol = os.path.join(ROOT, fayl)
    with open(yol) as f:
        matn = f.read()
    if matn.count(eski) != 1:
        sys.exit(f'Bu ovni hozirgi kodga qo\'llab bo\'lmadi ({fayl} o\'zgargan). tools/ovchi.py selfcheck')
    os.makedirs(HOLAT_DIR, exist_ok=True)
    shutil.copy(yol, asl_yol(o))
    with open(yol, 'w') as f:
        f.write(matn.replace(eski, yangi))
    h.update(faol=o['nom'], fayl=fayl, maslahat=0)
    holat_yoz(h)
    print(rang('1', f'Ov boshlandi: {o["sarlavha"]}') + f'  ({"*" * o["qiyinlik"]})\n')
    print('Alomat:   ' + o['alomat'])
    print('Takrorlash: ' + o['qayta'])
    print('\nYadroda bitta joy buzildi (qaysi fayl - aytilmaydi). `git diff` - taqiqlangan.')
    print('Maslahat kerak bo\'lsa: tools/ovchi.py maslahat   Tuzatdim deb o\'ylasangiz: tools/ovchi.py tekshir')
    return 0


def cmd_maslahat(_a):
    h = holat_oqi()
    if not h['faol']:
        sys.exit('Faol ov yo\'q.')
    o = ov_top(h['faol'])
    i = h.get('maslahat', 0)
    for k in range(min(i + 1, len(o['maslahat']))):
        print(f'{k + 1}. {o["maslahat"][k]}\n')
    if i + 1 >= len(o['maslahat']):
        print('(Maslahatlar tugadi. Taslim bo\'lsangiz: tools/ovchi.py javob ' + o['nom'] + ' --taslim)')
    h['maslahat'] = min(i + 1, len(o['maslahat']))
    holat_yoz(h)
    return 0


def test_natijalari(log):
    """tools/test.sh chiqishidan '[OK]   nom' / '[FAIL] nom' qatorlari."""
    r = {}
    for q in log.splitlines():
        m = re.match(r'^\s*\[(OK|FAIL)\]\s+(.*?)(\s{3}\(.*\))?$', q)
        if m:
            r[m.group(2).strip()] = m.group(1) == 'OK'
    return r


def demo_ishga(append):
    subprocess.run(['make', '-s', f'APPEND={append}'], cwd=ROOT, check=True)
    serial = os.path.join(ROOT, 'build', 'ovchi-demo.txt')
    try:
        os.remove(serial)
    except OSError:
        pass
    try:
        subprocess.run(['qemu-system-x86_64', '-m', '256M', '-cdrom', 'build/myos.iso', '-boot', 'd',
                        '-display', 'none', '-serial', f'file:{serial}', '-no-reboot'],
                       cwd=ROOT, timeout=40, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    except subprocess.TimeoutExpired:
        pass
    subprocess.run(['make', '-s', 'APPEND='], cwd=ROOT)     # oddiy ISO ni tiklash
    try:
        with open(serial, errors='replace') as f:
            return f.read()
    except OSError:
        return ''


def cmd_tekshir(_a):
    h = holat_oqi()
    if not h['faol']:
        sys.exit('Faol ov yo\'q. Boshlash: tools/ovchi.py boshla <nom>')
    o = ov_top(h['faol'])
    tur = o['tekshiruv'][0]
    print('Yig\'ish va QEMU testi (bir necha daqiqa)...', flush=True)
    if tur == 'demo':
        chiqish = demo_ishga(o['tekshiruv'][1])
        otdi = re.search(o['tekshiruv'][2], chiqish) is not None
        print(f'  {"[ OK ]" if otdi else "[XATO]"} {o["tekshiruv"][1]}: "{o["tekshiruv"][2]}" chiqishi kerak')
    else:
        r = subprocess.run(['tools/test.sh', 'bios'], cwd=ROOT, capture_output=True)
        nat = test_natijalari(r.stdout.decode('utf-8', 'replace'))   # qulagan yadro axlat bayt chiqarishi mumkin
        otdi = True
        for nom in o['tekshiruv'][1]:
            ok = nat.get(nom)
            otdi &= bool(ok)
            print(f'  {"[ OK ]" if ok else "[XATO]"} {nom}' + ('' if ok is not None else ' (natija yo\'q)'))
        print('  (Boshqa tekshiruvlar - masalan ochiq M1-M4 mashqlari sababli - bu ovga taalluqli emas.)')
    if not otdi:
        log = 'build/ovchi-demo.txt' if tur == 'demo' else 'build/test-output-bios.log'
        print(rang('31', '\nHali emas.') + f' Log: {log}. Maslahat: tools/ovchi.py maslahat')
        return 1
    h['yechilgan'] = sorted(set(h['yechilgan']) | {o['nom']})
    h['faol'] = None
    holat_yoz(h)
    print(rang('32', f'\nTOPDINGIZ! "{o["sarlavha"]}"'))
    print('Endi: (1) xatolar.md ga yozing - alomat, gipotezalar, qaysi tajriba hal qildi;')
    print(f'      (2) tools/ovchi.py javob {o["nom"]}  - sizning tuzatishingiz asl kod bilan bir xilmi?')
    print('      (3) asl kodni qaytarish: tools/ovchi.py tiklash ' + o['nom'])
    return 0


def cmd_javob(a):
    o = ov_top(a.nom)
    h = holat_oqi()
    if o['nom'] not in h['yechilgan'] and not a.taslim:
        sys.exit('Avval toping (tools/ovchi.py tekshir). Taslim bo\'lsangiz: --taslim')
    fayl, eski, yangi = o['o'][0], b64(o['o'][1]), b64(o['o'][2])
    print(rang('1', f'{o["sarlavha"]} - {fayl}\n'))
    for q in difflib.unified_diff(eski.splitlines(), yangi.splitlines(), 'asl', 'kiritilgan_xato', lineterm=''):
        print('  ' + q)
    print('\n' + o['sabab'])
    return 0


def cmd_tiklash(a):
    h = holat_oqi()
    nom = a.nom or h['faol']
    if not nom:
        sys.exit('Qaysi ov? tools/ovchi.py tiklash <nom>')
    o = ov_top(nom)
    if not os.path.exists(asl_yol(o)):
        sys.exit(f'{nom}: saqlangan asl fayl yo\'q (.ovchi/{nom}.asl).')
    shutil.copy(asl_yol(o), os.path.join(ROOT, o['o'][0]))
    os.remove(asl_yol(o))
    if h['faol'] == nom:
        h['faol'] = None
    holat_yoz(h)
    print(f'{o["o"][0]} asl holiga qaytarildi.')
    return 0


def cmd_selfcheck(_a):
    xato = 0
    nomlar = set()
    for o in OVLAR:
        for k in ('nom', 'sarlavha', 'qiyinlik', 'alomat', 'qayta', 'maslahat', 'tekshiruv', 'sabab', 'o'):
            if k not in o:
                print(f'  {o.get("nom")}: "{k}" yo\'q')
                xato = 1
        if o['nom'] in nomlar:
            print(f'  {o["nom"]}: takror nom')
            xato = 1
        nomlar.add(o['nom'])
        fayl, eski, yangi = o['o'][0], b64(o['o'][1]), b64(o['o'][2])
        try:
            with open(os.path.join(ROOT, fayl)) as f:
                n = f.read().count(eski)
        except OSError:
            n = -1
        if n != 1 or eski == yangi:
            print(f'  [XATO] {o["nom"]}: {fayl} da asl qator {n} marta topildi (1 bo\'lishi kerak)')
            xato = 1
    test_sh = open(os.path.join(ROOT, 'tools', 'test.sh')).read()
    for o in OVLAR:
        if o['tekshiruv'][0] == 'test':
            for nom in o['tekshiruv'][1]:
                if nom not in test_sh:
                    print(f'  [XATO] {o["nom"]}: tools/test.sh da "{nom}" tekshiruvi yo\'q')
                    xato = 1
    print(f'ovchi selfcheck: {"XATO" if xato else "OK"} ({len(OVLAR)} ta ov)')
    return xato


def main():
    import argparse
    ap = argparse.ArgumentParser(description='Xato ovchisi: yadrodagi yashirin xatoni toping')
    sub = ap.add_subparsers(dest='b')
    sub.add_parser('royxat')
    p = sub.add_parser('boshla')
    p.add_argument('nom')
    sub.add_parser('maslahat')
    sub.add_parser('tekshir')
    p = sub.add_parser('javob')
    p.add_argument('nom')
    p.add_argument('--taslim', action='store_true')
    p = sub.add_parser('tiklash')
    p.add_argument('nom', nargs='?')
    sub.add_parser('selfcheck')
    a = ap.parse_args()
    f = {'royxat': cmd_royxat, 'boshla': cmd_boshla, 'maslahat': cmd_maslahat, 'tekshir': cmd_tekshir,
         'javob': cmd_javob, 'tiklash': cmd_tiklash, 'selfcheck': cmd_selfcheck}.get(a.b)
    if not f:
        ap.print_help()
        return 1
    return f(a)


if __name__ == '__main__':
    sys.exit(main())
