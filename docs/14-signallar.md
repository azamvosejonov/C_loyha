# 14 — Signallar, jarayon guruhlari va job control

Kod: `kernel/proc/signal.c`, `kernel/proc/process.c` (to'xtatish, `wait`), `kernel/drivers/tty.c` (Ctrl-C),
`kernel/arch/syscall_entry.asm` (iretq yo'li), `user/libc/{signal.c,sigreturn.asm}`, `user/bin/{sh,sigtest,kill}.c`

```
myos:/$ spin a 1000
[a] 1/1000
^C                              <- SIGINT: faqat spin o'ldi, shell emas
myos:/$ sleep 100
^Z                              <- SIGTSTP: to'xtatildi
[1]+  To'xtatildi       sleep 100
myos:/$ bg                      <- SIGCONT: fonda davom etadi
myos:/$ seq 1000000 | head -2   <- head tugadi -> seq SIGPIPE oladi va darhol to'xtaydi
myos:/$ crash null
Segmentation fault              <- SIGSEGV, $? = 139 (128 + 11)
```

## 1. Signal — dasturiy uzilish

| | apparat uzilishi | signal |
|---|---|---|
| kimni to'xtatadi | CPU'ni | jarayonni |
| handler | yadrodagi IDT | dasturdagi funksiya (`sigaction`) |
| qaytish | `iretq` | `sigreturn` syscall'i |
| bloklash | `cli` | `sigprocmask` |

Raqamlar, `struct sigaction` joylashuvi va `wait()` holat so'zining kodlanishi **Linux bilan bir xil**
(`include/myos/abi.h`).

## 2. Yuborish → kutish → yetkazish

```
kill(pid, SIGTERM) ──► signal_send_locked: sig_pending |= bit
                                  │ jarayon uxlayotgan bo'lsa → uyg'otish (read → -EINTR)
                                  ▼
              jarayon yadrodan USER rejimiga qaytayotganda (syscall/uzilish oxiri)
                                  ▼
                           signal_deliver
               ┌──────────────────┼──────────────────────┐
           SIG_IGN            SIG_DFL                  handler
          (tashlash)   tugash / to'xtash /       user stekiga freym,
                       e'tiborsiz (SIGCHLD)      RIP = handler
```

**Nega darhol emas?** Signal kelgan paytda jarayon yadroda qulf ushlab turgan bo'lishi mumkin. User
rejimiga qaytish nuqtasida esa u hech narsa ushlamaydi. Cheksiz tsikldagi dastur ham signalni oladi,
chunki taymer uzilishi har 10 ms da yadroga kiradi va qaytishda signallar tekshiriladi.

## 3. Handler'ga borish va qaytish

```
 user steki:
   ...dastur ma'lumotlari...   <- to'xtagan RSP
   128 bayt red zone           <- ABI: funksiya bu yerni ogohlantirishsiz ishlatadi, tegmaymiz!
   sigframe.ctx: RAX..R15, RIP, RFLAGS, RSP, eski niqob
   __restore_rt manzili        <- handler kirishidagi RSP (% 16 == 8, xuddi `call` dan keyingidek)
```

1. Yadro `RIP = handler`, `RDI = signal raqami` qilib qaytadi.
2. Handler `ret` qiladi va `__restore_rt` ga (libc, `sigreturn.asm`) tushadi. U `sigreturn` syscall'ini
   chaqiradi.
3. `sys_sigreturn` freymni o'qiydi va **tekshiradi**: freym user xotirasida, dastur uni o'zgartirgan
   bo'lishi mumkin. RIP va RSP user manzili bo'lishi, RFLAGS da esa faqat arifmetik bayroqlar qolishi
   kerak. Aks holda dastur `IOPL=3` ni o'rnatib, portlarga to'g'ridan-to'g'ri murojaat qila olardi.
4. **sysret emas, iretq**: `sysret` RCX va R11 ni buzadi, to'xtatilgan kod esa ularning asl qiymatini
   kutadi. Shuning uchun sigreturn freymdagi "vektor" ni `0x81` qiladi va `syscall_entry.asm` sekinroq,
   lekin to'liq `iretq` yo'lini tanlaydi.

`sigtest` ning 12-testi buni tekshiradi: bola registrlarni ko'p ishlatadigan hisob-kitob qiladi, ota esa
unga har 10 ms da signal yuboradi. Natija signalsiz hisoblangan bilan bir xil chiqishi kerak.

## 4. Uzilgan syscall'lar: EINTR va SA_RESTART

`read()` klaviaturani kutib uxlab turganda signal keldi, deylik. Ikki yo'l bor:

- **-EINTR**: `read` xato bilan qaytadi, dastur o'zi qaror qiladi. Shell Ctrl-C da aynan shunday qiladi:
  terilayotgan qatorni tashlab, yangi so'rov chiqaradi.
- **SA_RESTART**: yadro syscall'ni **qayta bajaradi**: `RIP -= 2` (`syscall` instruksiyasi `0F 05` ikki
  bayt) va `RAX = asl raqam`. Handler tugagach, dastur go'yo hech narsa bo'lmagandek `read` da davom etadi.

Ctrl-Z bilan to'xtatilgan `read`/`sleep` har doim qayta boshlanadi. `sleep` bunda **qolgan** vaqtni
argumentga yozib qo'yadi, shuning uchun `fg` dan keyin 30 soniya qaytadan boshlanmaydi.

## 5. Exception → signal

| CPU | signal |
|---|---|
| #PF, #GP | SIGSEGV |
| #DE (0 ga bo'lish) | SIGFPE |
| #UD | SIGILL |
| int3 | SIGTRAP |

Dastur SIGSEGV handler'ini o'rnatgan bo'lsa, xatoni o'zi hal qilishi mumkin. JVM va ba'zi GC'lar null
tekshiruvini aynan shunday qiladi. Handler bo'lmasa, yadro xabar chiqaradi va jarayon **signal bilan**
tugaydi (`WTERMSIG = 11`). Xato signalini e'tiborsiz qoldirib yoki bloklab bo'lmaydi: yadro uni majburan
standart holatga qaytaradi (`signal_force`). Aks holda xato qilgan instruksiya cheksiz qayta bajarilardi.

## 6. wait() holat so'zi

```
exit(3)          ->  0x0300   WIFEXITED,   WEXITSTATUS = 3
SIGSEGV          ->  0x000B   WIFSIGNALED, WTERMSIG    = 11
Ctrl-Z (SIGTSTP) ->  0x147F   WIFSTOPPED,  WSTOPSIG    = 20   (faqat WUNTRACED bilan)
```

Shell `$?` ni bash qoidasi bilan hisoblaydi: `WEXITSTATUS` yoki `128 + signal`.

## 7. Jarayon guruhlari, sessiyalar, job control

```
sessiya 2 (init setsid qilgan)
├── guruh 2:  init
├── guruh 6:  sh                    <- terminal "egasi" navbatdagi buyruqqacha
├── guruh 14: seq 100000 | grep 7   <- bitta job = bitta guruh (ikki jarayon)
└── guruh 17: sleep 100 (fonda)
```

- **Shell** har bir pipeline uchun yangi guruh ochadi: `setpgid(child, pgid)`. Buni ota ham, bola ham
  qiladi. Qaysi biri birinchi ulgurishidan qat'i nazar natija bir xil bo'ladi, shuning uchun poyga holati
  yuzaga kelmaydi.
- **Terminalning oldingi plan guruhi** (`tcsetpgrp` → `TIOCSPGRP`): Ctrl-C/Ctrl-Z faqat unga boradi.
  Buyruq tugagach, shell terminalni o'ziga qaytaradi.
- **Ctrl-C uzilish kontekstida** qayta ishlanadi (`tty_input_signal`). Dastur terminaldan hech narsa
  o'qimasa ham (masalan, `spin`), signal baribir unga yetib boradi.
- **Fondagi jarayon terminaldan o'qisa**, SIGTTIN oladi va to'xtaydi (`cat &` → `[1]+ To'xtatildi`).
  Aks holda ikki dastur bitta klaviatura uchun talashardi.
- Shell o'zi SIGTSTP/SIGTTIN/SIGTTOU/SIGQUIT ni e'tiborsiz qoldiradi, bolalarda esa ularni **SIG_DFL** ga
  qaytaradi. `exec` e'tiborsiz qoldirilgan signallarni saqlab qoladi (POSIX, `nohup` shunga tayanadi),
  shuning uchun bu qadam zarur.

## Sinab ko'ring

```
sigtest                         17 ta tekshiruv
spin a 1000     -> Ctrl-C
sleep 100       -> Ctrl-Z, jobs, bg, fg, Ctrl-C
cat &           -> jobs (SIGTTIN bilan to'xtagan)
kill -l ; kill -STOP %1 ; kill %1
seq 1000000 | head -2
```

## Mashqlar

1. **Oson:** `timeout 5 spin a 1000` dasturi: `alarm` + `SIGALRM` bilan bolani o'ldirsin.
2. **Oson:** `nohup`: SIGHUP ni e'tiborsiz qoldirib, keyin `exec` qilsin. Nega bu `exec` dan keyin ham
   ishlaydi?
3. **O'rta:** `SA_SIGINFO`: handler'ga `siginfo_t` (kim yubordi, qaysi manzilda xato) uzating. SIGSEGV
   handler'i CR2 ni ko'ra olsin.
4. **O'rta:** `sigaltstack`: stek to'lishini (SIGSEGV) ushlash uchun alohida signal steki. Usiz nega
   handler ishlamaydi?
5. **O'rta:** tty'da **TOSTOP** va SIGTTOU: fondagi jarayonning terminalga yozishini ham to'xtating.
6. **Qiyin:** real-time signallar (32–64): navbatga qo'yiladi (har biri alohida), bit bo'lib
   "yo'qolmaydi". `sigqueue` bilan qiymat uzating.
7. **Qiyin:** `setjmp`/`longjmp` va `sigsetjmp`/`siglongjmp` ni libc'ga qo'shing (assembly). Keyin
   SIGSEGV handler'idan `siglongjmp` bilan "xatodan qaytish"ni ko'rsating.
