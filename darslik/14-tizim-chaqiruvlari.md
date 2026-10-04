# 14-bob. Tizim chaqiruvlari: fayllar, jarayonlar, pipe, signallar

> **Bu bobda nima o'rganasiz:** dastur yadro bilan qanday gaplashishini; fayl deskriptorlarini; `open/read/write`; `fork/exec/wait` (yangi dastur ishga tushirish); `pipe/dup2`
> (`a | b` qanday ishlashi); signallar (Ctrl+C). Ya'ni shell yozish uchun kerak bo'lgan hamma narsa. Bu bob — **yadroning tashqi eshigi**: keyingi qadamda (MyOS) shu eshikning ichkarisini yozasiz.
> **Oldindan nima kerak:** 5-, 7-, 8-, 12-boblar.   **Vaqt:** 7–8 soat.
> Mashqlar: 25–28.

> **To'liq ishlaydigan misol:** [misollar/14_jarayonlar.c](misollar/14_jarayonlar.c) — yig'ib ishga tushiring, fayl boshidagi
> "Sinab ko'ring" topshiriqlarini bajaring. Bobdagi parchalarni qanday sinash: [misollar/README.md](misollar/README.md#darslikdagi-parchani-ozingiz-qanday-sinaysiz).

## Bu bob nima haqida?

Dasturingiz ekranga yozadi, fayl o'qiydi, boshqa dastur ishga tushiradi. Lekin dastur **o'zi** buni qila olmaydi: diskka, ekranga, tarmoqqa faqat **operatsion tizim yadrosi** tega oladi.
Dastur yadrodan **so'raydi** — bu so'rovlar **tizim chaqiruvlari** (system call, syscall) deyiladi.

**Hayotdan misol: bank kassasining oynasi.** Bankda seyfga o'zingiz kira olmaysiz. Kassa oynasiga kelib, so'rov berasiz: "hisobimdan 100 ming yeching". Kassir (yadro) tekshiradi, seyfga kiradi
va natijani oynadan beradi. Dastur ham diskka, tarmoqqa, ekranga **to'g'ridan-to'g'ri** tegolmaydi — `read`, `write`, `open` orqali yadrodan so'raydi. Oyna — `syscall` buyrug'i.

| Bankda | Kompyuterda |
|---|---|
| mijoz | dastur (user rejimi) |
| kassir | yadro (yadro rejimi) |
| kassa oynasi | `syscall` buyrug'i |
| seyf | disk, ekran, tarmoq |
| so'rov | `read`, `write`, `open`, `fork`... |

## 14.1. Tizim chaqiruvi (syscall) nima

Oddiy dastur (user rejimi, CPU'ning 3-halqasi) apparatga tega olmaydi: diskni o'qiy olmaydi, ekranga to'g'ridan-to'g'ri yoza olmaydi, boshqa dasturning xotirasini ko'ra olmaydi.
Bularning hammasi uchun u **yadrodan so'raydi**:

```text
dastur:  write(1, "salom\n", 6)
libc:    rax = 1 (SYS_write), rdi = 1, rsi = buf, rdx = 6;  syscall     <- maxsus CPU buyrug'i
CPU:     yadro rejimiga (0-halqa) o'tadi, yadroning kirish nuqtasiga sakraydi
yadro:   argumentlarni tekshiradi -> fd 1 qaysi fayl? -> terminal drayveri -> ekran
         natija (6 yoki -EBADF) -> rax;  sysret
dastur:  write() 6 qaytardi
```

Buni haqiqiy dasturda ko'ramiz:

```c
/* syscall_write.c - write() va uning ostidagi syscall */
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>

int main(void)
{
    write(1, "salom (write)\n", 14);               /* libc funksiyasi -> syscall */
    syscall(SYS_write, 1, "salom (syscall)\n", 16);   /* to'g'ridan-to'g'ri syscall raqami bilan */
    printf("SYS_write raqami: %d\n", SYS_write);
    return 0;
}
```

```console
$ gcc -Wall -Wextra syscall_write.c -o syscall_write
$ ./syscall_write
salom (write)
salom (syscall)
SYS_write raqami: 1
$ strace -o iz.txt -e trace=write ./syscall_write > /dev/null
$ cat iz.txt
write(1, "salom (write)\n", 14)         = 14
write(1, "salom (syscall)\n", 16)       = 16
write(1, "SYS_write raqami: 1\n", 20)   = 20
+++ exited with 0 +++
```

**Bu dastur nima qiladi:** ekranga uch xil usulda yozadi: (1) `write` — libc'ning tayyor funksiyasi; (2) `syscall(SYS_write, ...)` — shu chaqiruvni **raqami** bilan; (3) `printf` — formatlab chiqarish.

**Qismlar:**

| Qism | Vazifasi | Tafsilot |
|---|---|---|
| `write(1, "...", 14)` | `1` raqamli faylga (ekranga) 14 bayt yozish | 14 — matn uzunligi (`\n` bilan) |
| `syscall(SYS_write, 1, ...)` | aynan shu ishni **syscall raqami** (`SYS_write` = 1) bilan | libc'ning `write` i ichida shu ish bajariladi |
| `strace -o iz.txt -e trace=write` | dastur qilgan `write` syscall'larini `iz.txt` ga **yozadi** | uchinchi `write` — `printf` ning buferdan yuborgani |

`strace` natijasida `write(1, "salom (write)\n", 14) = 14` ko'rinishi: **nom(argumentlar) = natija**. Bu — yadro bilan dastur orasidagi haqiqiy "suhbat".

MyOS'da har bir qadamni o'qishingiz mumkin: `user/libc/unistd.c` (write) → `user/libc/syscall.h` (`__syscall3` — inline asm) → `kernel/arch/syscall_entry.asm` (kirish) → `kernel/sys/syscall.c`
(`switch (nr)`) → `kernel/sys/sys_fs.c` → `kernel/fs/vfs.c` → `kernel/drivers/tty.c`.

Syscall **qimmat** (rejim almashishi, tekshiruvlar) — shuning uchun libc buferlaydi (12-bob). `man 2 write` — syscall hujjati (2-bo'lim), `man 3 printf` — kutubxona funksiyasi (3-bo'lim).

> **Eslab qoling:** dastur apparatga **tega olmaydi**; hamma ish `syscall` orqali yadrodan so'raladi. Natija manfiy bo'lsa — xato kodi (12.4).

## 14.2. Fayl deskriptorlari

**Hayotdan misol: garderob raqamchasi.** Teatrda paltongizni topshirasiz va raqamcha olasiz: 3. Keyin paltoni raqamcha bilan so'raysiz — palto qayerda osilganini bilishingiz shart emas.
`open` ham raqam qaytaradi (`fd = 3`), keyin `read(3, ...)`, `write(3, ...)`. 0, 1, 2 raqamlari doim band: klaviatura, ekran, xatolar ekrani.

Fayl deskriptori (fd) — kichik butun son, jarayonning **ochiq fayllar jadvalidagi indeks**:

```text
jarayon:  fd 0 -> terminal (stdin)
          fd 1 -> terminal (stdout)
          fd 2 -> terminal (stderr)
          fd 3 -> /home/ali/ma'lumot.txt (offset 120)
```

"Unix'da hamma narsa — fayl": oddiy fayl, terminal, pipe, disk qurilmasi (`/dev/sda`), tasodifiy son generatori (`/dev/urandom`) — hammasi bir xil `read`/`write` bilan ishlaydi.
Yadro ichida buni VFS va `file_ops` jadvali ta'minlaydi (7-bob).

## 14.3. `open`, `read`, `write`, `close`, `lseek`

**Bu nima?** To'rt asosiy fayl amali: faylni **ochish** (raqam olish), **o'qish**, **yozish**, **yopish**. Hammasi fd (raqam) bilan.

```c
/* fd_misol.c - open, write, lseek, read, close */
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    int fd = open("fd_fayl.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);   /* yozish uchun yaratish */
    if (fd < 0) {
        perror("open");
        return 1;
    }
    printf("open qaytargan fd = %d  (0, 1, 2 band bo'lgani uchun)\n", fd);

    write(fd, "salom dunyo\n", 12);
    close(fd);

    fd = open("fd_fayl.txt", O_RDONLY);                  /* o'qish uchun qayta ochish */
    char buf[64];
    ssize_t n = read(fd, buf, sizeof(buf));              /* ko'pi bilan 64 bayt */
    printf("o'qildi %zd bayt: ", n);
    fflush(stdout);
    write(1, buf, (size_t)n);                            /* buf satr EMAS - '\0' yo'q, shuning uchun write */

    lseek(fd, 6, SEEK_SET);                              /* pozitsiyani 6-baytga o'tkazish */
    n = read(fd, buf, 5);
    printf("lseek(6) dan keyin 5 bayt: %.*s\n", (int)n, buf);

    n = read(fd, buf, sizeof(buf));                      /* faylda 1 bayt ('\n') qolgan edi */
    n = read(fd, buf, sizeof(buf));                      /* endi fayl OXIRI */
    printf("fayl oxirida read qaytardi: %zd\n", n);
    close(fd);
    return 0;
}
```

```console
$ gcc -Wall -Wextra fd_misol.c -o fd_misol
$ ./fd_misol
open qaytargan fd = 3  (0, 1, 2 band bo'lgani uchun)
o'qildi 12 bayt: salom dunyo
lseek(6) dan keyin 5 bayt: dunyo
fayl oxirida read qaytardi: 0
```

**Bu dastur nima qiladi (umumiy):** fayl yaratib, ichiga matn yozadi; uni qayta ochib o'qiydi; pozitsiyani ko'chirib qismini o'qiydi; fayl oxirida `read` nima qaytarishini ko'rsatadi.

**Qismlar (avval vazifasi):**

| Chaqiruv | Vazifasi | Qaytaradi | Tafsilot |
|---|---|---|---|
| `open(yol, bayroqlar, rejim)` | faylni ochish | yangi fd (eng kichik **bo'sh** raqam) yoki −1 | `O_RDONLY/O_WRONLY/O_RDWR`, `O_CREAT` (yo'q bo'lsa yarat), `O_TRUNC` (tozala), `O_APPEND`; `O_CREAT` bilan rejim: `0644` |
| `read(fd, buf, n)` | ko'pi bilan `n` bayt o'qish | o'qilgan baytlar soni; **0 = fayl oxiri**; −1 = xato | `n` dan **kam** qaytishi normal (pipe, terminal, fayl oxiri) |
| `write(fd, buf, n)` | `n` bayt yozish | yozilgan baytlar soni yoki −1 | `n` dan kam yozishi mumkin — takrorlash kerak (pastda) |
| `close(fd)` | yopish | 0 / −1 | yopilmasa — "fd sizib chiqadi" (leak) |
| `lseek(fd, offset, qayerdan)` | pozitsiyani ko'chirish | yangi pozitsiya | `SEEK_SET` (boshdan), `SEEK_CUR` (hozirgidan), `SEEK_END` (oxiridan) |

- `open` birinchi marta `3` qaytardi — chunki 0, 1, 2 band.
- `read` bufer **satr emas**: oxirida `'\0'` yo'q (6-bob). Shuning uchun uni `write` bilan yoki `%.*s` (uzunlik bilan) chiqardik.
- `0644` — **sakkizlik** son (2-bob): egasi o'qiydi+yozadi (6), guruh va boshqalar faqat o'qiydi (4).

### "Qisman yozish" to'g'ri ishlovi

`write` ba'zan so'ralgandan **kam** bayt yozadi (signal uzdi, pipe to'lgan). Shuning uchun to'liq yozish **siklda**:

```c
/* hammasini_yoz.c - to'liq yozish sikli */
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

static ssize_t hammasini_yoz(int fd, const char *buf, size_t n)
{
    size_t yozildi = 0;
    while (yozildi < n) {
        ssize_t k = write(fd, buf + yozildi, n - yozildi);
        if (k < 0) {
            if (errno == EINTR)
                continue;           /* signal uzdi - qayta urinish */
            return -1;
        }
        yozildi += (size_t)k;
    }
    return (ssize_t)yozildi;
}

int main(void)
{
    ssize_t k = hammasini_yoz(1, "to'liq yozildi\n", 15);
    fprintf(stderr, "yozilgan bayt: %zd\n", k);
    return 0;
}
```

```console
$ gcc -Wall -Wextra hammasini_yoz.c -o hammasini_yoz
$ ./hammasini_yoz
to'liq yozildi
yozilgan bayt: 15
```

**Qadamlar:** `yozildi` — hozirgacha yozilgan baytlar. Har gal `buf + yozildi` dan boshlab **qolganini** yozishga urinamiz; `write` nechta yozsa, shuncha `yozildi` ga qo'shamiz; `n` ga yetguncha takrorlaymiz.
`EINTR` — "signal uzdi, xato emas" — qayta urinamiz.

## 14.4. Jarayonlar: `fork`, `exec`, `wait`

Unix'ning eng mashhur g'oyasi — yangi dastur ishga tushirish **ikki qadamga** bo'lingan: `fork` (nusxa olish) + `exec` (dasturni almashtirish).

### `fork` — egizak

**Hayotdan misol: egizak.** `fork` jarayonning **aynan nusxasini** yaratadi: bir xil xotira, bir xil ochiq fayllar, hatto kodning bir xil qatorida. Farqi bitta: otaga `fork` bolaning raqamini
qaytaradi, bolaga esa 0. Shu bilan har biri o'zining kimligini biladi.

```c
/* fork_misol.c - fork: bir marta chaqiriladi, ikki marta qaytadi */
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    int son = 100;
    printf("fork dan oldin: son = %d\n", son);
    fflush(stdout);                     /* bufer ikki marta chiqmasligi uchun */

    pid_t pid = fork();                 /* BU YERDA ikki jarayon bo'ladi */
    if (pid < 0) {
        perror("fork");
        return 1;
    }
    if (pid == 0) {                     /* BOLA: fork 0 qaytardi */
        son = 200;                      /* faqat bolaning nusxasi o'zgaradi */
        printf("bola: son = %d\n", son);
        _exit(7);                       /* chiqish kodi 7 */
    }
    int holat;                          /* OTA: fork bolaning pid'ini qaytardi */
    waitpid(pid, &holat, 0);            /* bola tugashini kutish */
    printf("ota: son = %d (o'zgarmadi!)\n", son);
    if (WIFEXITED(holat))
        printf("ota: bola %d kod bilan tugadi\n", WEXITSTATUS(holat));
    return 0;
}
```

```console
$ gcc -Wall -Wextra fork_misol.c -o fork_misol
$ ./fork_misol
fork dan oldin: son = 100
ota: son = 100 (o'zgarmadi!)
ota: bola 7 kod bilan tugadi
```

**Bu dastur nima qiladi (umumiy):** o'zini ikkiga bo'ladi. Bola `son` ni o'zgartiradi va 7 kod bilan tugaydi; ota kutadi va `son` o'zgarmaganini hamda bolaning chiqish kodini ko'radi.

**Qismlar:**

| Qism | Vazifasi | Tafsilot |
|---|---|---|
| `fork()` | jarayon nusxasini yaratish | **bir marta chaqiriladi, ikki marta qaytadi** — otada va bolada |
| `pid == 0` | "men bolaman" | bolaga `fork` 0 qaytaradi |
| `pid > 0` | "men otaman" | otaga bolaning pid'i qaytadi |
| `pid < 0` | xato | masalan jarayonlar chegarasi |
| `waitpid(pid, &holat, 0)` | bola tugashini kutish va natijasini olish | olinmagan tugagan bola — "zombi" (pastda) |
| `WIFEXITED`, `WEXITSTATUS` | "normal tugadimi?", "chiqish kodi" | holatdan ma'lumot ajratish |
| `_exit(7)` | bolani darhol tugatish | `fork` qilingan bolada `exit` emas `_exit` (12.3 dagi bufer ikkilanishi) |

- Bola otaning **to'liq nusxasi**: xotira, o'zgaruvchilar, ochiq fayllar. Lekin keyin ular mustaqil — bolada `son` ni o'zgartirish otaga ta'sir qilmaydi (natijada `son = 100` qoldi).
  (Yadro butun xotirani darhol nusxalamaydi — **copy-on-write**: sahifalar faqat birinchi yozishda nusxalanadi. MyOS: `kernel/proc/process.c` → `proc_fork`, `docs/11-fork-cow.md`.)

### `exec` — aktyorning rolni almashtirishi

**Hayotdan misol: aktyor.** Aktyor (jarayon, uning raqami — PID) o'sha, lekin kiyimi va matni butunlay yangi: endi u boshqa rolni o'ynaydi. `exec` jarayonning kodini boshqa dastur bilan almashtiradi.
Shell `ls` ni aynan shunday ishga tushiradi: `fork` (egizak) + `exec` (rolni almashtir).

```c
/* exec_misol.c - fork + exec + wait: shell'ning yuragi */
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

static int ishga_tushir(char *const argv[])
{
    pid_t pid = fork();
    if (pid == 0) {                         /* BOLA */
        execvp(argv[0], argv);              /* dasturni almashtirish (PATH bo'yicha qidiradi) */
        perror("execvp");                   /* bu yerga faqat exec XATO bo'lsa yetib kelinadi */
        _exit(127);
    }
    int holat;                              /* OTA */
    waitpid(pid, &holat, 0);
    return WIFEXITED(holat) ? WEXITSTATUS(holat) : -1;
}

int main(void)
{
    char *a1[] = { "echo", "salom", "dunyo", NULL };
    printf("echo chiqish kodi: %d\n", ishga_tushir(a1));
    fflush(stdout);                         /* tartib buzilmasligi uchun (12.3) */

    char *a2[] = { "yoq_dastur_bunaqa", NULL };
    printf("yoq_dastur chiqish kodi: %d\n", ishga_tushir(a2));
    fflush(stdout);
    return 0;
}
```

```console
$ gcc -Wall -Wextra exec_misol.c -o exec_misol
$ ./exec_misol 2>&1
salom dunyo
echo chiqish kodi: 0
execvp: No such file or directory
yoq_dastur chiqish kodi: 127
```

**Qismlar:**

- `execvp(nom, argv)` — joriy jarayonning dasturini `nom` bilan almashtiradi. Kod, ma'lumot, stek — **yangi**; pid va ochiq fd'lar — **o'sha**. Muvaffaqiyatli bo'lsa **hech qachon qaytmaydi**
  (chunki eski kod endi yo'q). `v` — argumentlar massiv (`argv`) ko'rinishida, `p` — `PATH` bo'yicha qidirish.
- `argv` massivi `NULL` bilan tugaydi (7.8 dagi `char **argv`).
- Agar `exec` qaytsa — xato bo'lgan (masalan dastur topilmadi); `perror` va `_exit(127)` ("topilmadi" an'anaviy kodi).
- Natija: `echo` ishladi (`salom dunyo`, kod 0); mavjud bo'lmagan dastur uchun kod 127.

**Nega ikki qadam (`fork` + `exec`)?** `fork` va `exec` **orasida** bola o'zini sozlay oladi: fd'larni almashtirish (`>` yo'naltirish, pipe), papkani o'zgartirish, signallarni sozlash. Shell aynan shu oraliqda ishlaydi.

### `wait` va zombi

**Hayotdan misol: ota-ona bolani maktabdan kutishi.** Ota bolaning ishini tugatishini kutadi va uning natijasini (chiqish kodini) oladi. Tugagan, lekin hech kim natijasini olmagan bola —
**zombi**: ishini tugatgan, lekin ro'yxatda turibdi. Otasi o'lgan bola — "yetim", uni `init` (PID 1) asrab oladi. Shuning uchun har bir `fork` ga mos `wait` bo'lishi kerak.

> **Eslab qoling:** `fork` → ikkita jarayon (`0` — bola, `>0` — ota); `exec` — dasturni almashtiradi (qaytmaydi); `wait` — bolani kutadi (zombi qolmasin).

## 14.5. `pipe` va `dup2` — shell'dagi `a | b`

**Hayotdan misol: pnevmatik quvur.** Supermarketlarda kassadan hujjatlarni quvur orqali ofisga yuboradilar: bir tomondan solinadi, boshqa tomondan chiqadi. `pipe` ham ikki uchli: `fds[1]` ga yozilgan
narsa `fds[0]` dan o'qiladi. `ls | wc -l` — ikki jarayonni quvur bilan ulash.

**Bu nima?** `pipe(fds)` — ikkita fd yaratadi: `fds[0]` — **o'qish uchi**, `fds[1]` — **yozish uchi**. **Asosiy ishi:** bir jarayon yozganini boshqasi o'qishi uchun kanal.

`dup2(eski, yangi)` — `yangi` fd ni `eski` ning nusxasiga aylantiradi (masalan, `stdout` ni quvurning yozish uchiga ulash). `echo ... | wc -w` shunday bajariladi:

```c
/* pipe_misol.c - echo ... | wc -w */
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    int fds[2];
    if (pipe(fds) < 0) {
        perror("pipe");
        return 1;
    }

    if (fork() == 0) {                  /* 1-bola: echo (yozuvchi) */
        dup2(fds[1], 1);                /* stdout -> quvurning yozish uchi */
        close(fds[0]);
        close(fds[1]);
        execlp("echo", "echo", "bir ikki uch tort", NULL);
        _exit(127);
    }
    if (fork() == 0) {                  /* 2-bola: wc -w (o'quvchi) */
        dup2(fds[0], 0);                /* stdin <- quvurning o'qish uchi */
        close(fds[0]);
        close(fds[1]);
        execlp("wc", "wc", "-w", NULL);
        _exit(127);
    }
    close(fds[0]);                      /* OTA ham yopishi SHART */
    close(fds[1]);
    wait(NULL);
    wait(NULL);
    return 0;
}
```

```console
$ gcc -Wall -Wextra pipe_misol.c -o pipe_misol
$ ./pipe_misol
4
```

**Bu dastur nima qiladi (umumiy):** `echo "bir ikki uch tort" | wc -w` buyrug'ini qo'lda bajaradi: birinchi bola `echo` ning chiqishini quvurga yozadi, ikkinchi bola `wc -w` quvurdan o'qib so'zlarni sanaydi. Natija: `4`.

**Qadamlar:**

| Qadam | Kim | Nima |
|---|---|---|
| 1 | ota | `pipe(fds)` — quvur yaratadi (`fds[0]` o'qish, `fds[1]` yozish) |
| 2 | 1-bola | `dup2(fds[1], 1)` — endi `stdout` (1) quvurga yozadi; ortiqcha fd'larni yopadi; `echo` ga aylanadi |
| 3 | 2-bola | `dup2(fds[0], 0)` — endi `stdin` (0) quvurdan o'qiydi; `wc -w` ga aylanadi |
| 4 | ota | quvurning ikkala uchini **yopadi**; ikkala bolani kutadi |

**Nega hamma joyda `close`?** Quvurning o'qiydigan tomoni **EOF** (`read` → 0) ni faqat yozish uchining **hamma** nusxalari yopilganda oladi. Ota `fds[1]` ni yopmasa, `wc` abadiy
kutadi (hech qachon "yozuvchi qolmadi" demaydi), ota ham `wait` da qotib qoladi. 28-mashqning asosiy tuzog'i shu.
MyOS: `user/bin/sh.c` (pipeline), `kernel/fs/pipe.c` (yadro tomoni — halqa bufer + uxlash/uyg'otish).

`>` yo'naltirish ham xuddi shunday: bolada `int fd = open("f.txt", O_WRONLY|O_CREAT|O_TRUNC, 0644); dup2(fd, 1); close(fd);`.

> **Eslab qoling:** `a | b` = `pipe` + ikki `fork` + `dup2` + **ortiqcha uchlarni yopish**.

## 14.6. Signallar

**Hayotdan misol: telefonga kelgan qo'ng'iroq.** Ishlayotganingizda telefon jiringlaydi — ishni to'xtatib, javob berasiz, keyin davom etasiz. Ctrl+C bosilganda dasturga `SIGINT` "qo'ng'irog'i" keladi.
Dastur unga javob beradigan funksiya o'rnatishi (`sigaction`) yoki umuman javob bermasligi mumkin (standart javob — dastur to'xtaydi).

**Bu nima?** Signal — jarayonga yuboriladigan asinxron "xabar" (raqam bilan). **Asosiy ishi:** jarayonga tashqi voqea haqida xabar berish (Ctrl+C, bola tugadi, xato).

| Signal | Raqam | Sabab | Sukut bo'yicha |
|---|---|---|---|
| `SIGINT` | 2 | Ctrl-C | tugatish |
| `SIGKILL` | 9 | `kill -9` | tugatish (ushlab **bo'lmaydi**) |
| `SIGSEGV` | 11 | noto'g'ri xotiraga murojaat | tugatish (+core) |
| `SIGPIPE` | 13 | o'quvchisi yo'q pipe'ga yozish | tugatish |
| `SIGTERM` | 15 | `kill` (sukut) | tugatish |
| `SIGCHLD` | 17 | bola tugadi | e'tiborsiz |
| `SIGTSTP` | 20 | Ctrl-Z | to'xtatish |
| `SIGCONT` | 18 | `fg`/`bg` | davom etish |

O'z ishlovchingizni o'rnatish:

```c
/* signal_misol.c - signal ishlovchisi */
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

static volatile sig_atomic_t toxta = 0;

static void ishlovchi(int sig)
{
    (void)sig;
    toxta = 1;                  /* handler ichida faqat eng oddiy ishlar! */
}

int main(void)
{
    struct sigaction sa = { 0 };
    sa.sa_handler = ishlovchi;
    sigaction(SIGINT, &sa, NULL);           /* SIGINT kelsa - ishlovchini chaqir */

    printf("signal yuboramiz...\n");
    raise(SIGINT);                          /* o'zimizga SIGINT (Ctrl-C o'rniga) */

    if (toxta)
        printf("SIGINT ushlandi, chiroyli tugatyapman\n");
    return 0;
}
```

```console
$ gcc -Wall -Wextra signal_misol.c -o signal_misol
$ ./signal_misol
signal yuboramiz...
SIGINT ushlandi, chiroyli tugatyapman
```

**Bu dastur nima qiladi:** `SIGINT` uchun o'z ishlovchisini o'rnatadi, keyin `raise(SIGINT)` bilan signalni **o'ziga** yuboradi (Ctrl-C ni imitatsiya). Dastur to'xtamaydi — ishlovchi bayroqni ko'taradi, `main` uni ko'rib chiroyli tugaydi.

**Qismlar:**

| Qism | Vazifasi |
|---|---|
| `sigaction(SIGINT, &sa, NULL)` | "SIGINT kelsa, `sa.sa_handler` ni chaqir" |
| `ishlovchi(int sig)` | signal kelganda **avtomatik** chaqiriladigan funksiya |
| `volatile sig_atomic_t toxta` | ishlovchi va `main` bo'lishadigan bayroq (`volatile` — kompilyator keshlamasin; 16-bob) |
| `raise(SIGINT)` | o'ziga signal yuborish |

**Handler ichida xavfli:** `printf`, `malloc` — ular qulf ushlab turgan paytda signal kelsa, deadlock. Faqat "async-signal-safe" funksiyalar (`write`, `_exit`...) va `volatile sig_atomic_t` bayroq.

Yadro signalni qanday yetkazadi: foydalanuvchi stekiga "signal kadri" yozib, `rip` ni handler'ga o'zgartiradi; handler tugagach `sigreturn` syscall'i eski holatni tiklaydi. MyOS: `kernel/proc/signal.c`
(`setup_frame`, `sys_sigreturn` — lab'lar), `docs/14-signallar.md`.

## 14.7. Katalog va fayl ma'lumotlari

```c
/* katalog_misol.c - stat, mkdir, opendir */
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(void)
{
    mkdir("kat", 0755);                         /* papka yaratish (bor bo'lsa - xato, e'tibor bermaymiz) */
    FILE *f = fopen("kat/a.txt", "w");
    fprintf(f, "birinchi\n");
    fclose(f);
    f = fopen("kat/b.txt", "w");
    fprintf(f, "ikkinchi fayl matni\n");
    fclose(f);

    struct stat st;
    if (stat("kat/b.txt", &st) == 0)
        printf("kat/b.txt hajmi: %ld bayt\n", (long)st.st_size);

    DIR *d = opendir("kat");
    struct dirent *e;
    int soni = 0;
    while ((e = readdir(d)) != NULL)
        if (e->d_name[0] != '.')                /* '.' va '..' ni o'tkazamiz */
            soni++;
    closedir(d);
    printf("kat papkasida %d ta fayl\n", soni);
    return 0;
}
```

```console
$ gcc -Wall -Wextra katalog_misol.c -o katalog_misol
$ ./katalog_misol
kat/b.txt hajmi: 20 bayt
kat papkasida 2 ta fayl
```

**Qismlar:** `stat(yol, &st)` — fayl haqida ma'lumot (`st_size` — hajm bayt). `opendir`/`readdir`/`closedir` — papka ichidagi yozuvlarni birma-bir olish (tartib kafolatlanmagan; `.` va `..` doim bor).
`mkdir`, `rmdir`, `unlink` (faylni o'chirish), `rename`, `chdir`, `getcwd` — shell utilitalari (`ls`, `rm`, `mv`...) shular ustida qurilgan. MyOS'dagi `user/bin/*.c` — har biri 50–150 qatorli misol.

## Hayotdan misol va to'liq dastur

**Oilaviy hisob-kitob.** Ota uchta bola yaratadi. Har bir bola 1000 ta sonni qo'shib, natijani **quvur** orqali otaga yuboradi; ota yig'indini formula bilan solishtiradi. Dasturda: `fork`, `pipe`, `read`/`write`, `wait`.

```c
/* oila.c - fork, pipe, wait: ishni bolalarga bo'lib berish */
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    int quvur[2];
    if (pipe(quvur) < 0) {
        perror("pipe");
        return 1;
    }

    /* 1..3000 yig'indisi: har bir bola 1000 ta sonni qo'shadi */
    for (int bola = 0; bola < 3; bola++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return 1;
        }
        if (pid == 0) {                         /* BOLA: fork 0 qaytardi */
            close(quvur[0]);                    /* bola faqat yozadi */
            long qism = 0;
            for (long i = bola * 1000 + 1; i <= (bola + 1) * 1000; i++)
                qism += i;
            if (write(quvur[1], &qism, sizeof(qism)) != sizeof(qism))
                _exit(1);
            _exit(bola + 10);                   /* chiqish kodi - otaga "baho" */
        }
    }

    close(quvur[1]);                            /* OTA: faqat o'qiydi */
    long jami = 0, qism;
    while (read(quvur[0], &qism, sizeof(qism)) == sizeof(qism))
        jami += qism;                           /* hamma bola yozish uchini yopgach, read 0 qaytaradi */

    int holat, kodlar = 0;
    while (wait(&holat) > 0)                    /* har bir bolani kutish (zombie qolmasin) */
        if (WIFEXITED(holat))
            kodlar += WEXITSTATUS(holat);

    printf("1..3000 yig'indisi (3 bola hisobladi): %ld\n", jami);
    printf("Formula bo'yicha: %d\n", 3000 * 3001 / 2);
    printf("Bolalarning chiqish kodlari yig'indisi: %d (10 + 11 + 12)\n", kodlar);
    return 0;
}
```

```console
$ gcc -Wall -Wextra oila.c -o oila
$ ./oila
1..3000 yig'indisi (3 bola hisobladi): 4501500
Formula bo'yicha: 4501500
Bolalarning chiqish kodlari yig'indisi: 33 (10 + 11 + 12)
$ strace -f -e trace=fork,clone,pipe2,pipe,wait4 ./oila 2>&1 | grep -c "clone\|fork"
5
```

**Bu dastur nima qiladi (umumiy):** 1 dan 3000 gacha sonlarni qo'shish ishini 3 bolaga bo'lib beradi; har bola o'z qismini hisoblab quvurga yozadi; ota hammasini o'qib jamlaydi va bolalarni kutadi.

**Qismlar (vazifasi):**

| Qism | Vazifasi |
|---|---|
| `pipe(quvur)` | bolalar → ota yo'nalishida kanal |
| `for (bola = 0; bola < 3; ...) fork()` | uchta bola yaratadi; har biri `bola` raqamini (0, 1, 2) biladi |
| bola: `i` dan `(bola+1)*1000` gacha qo'shish | o'z oralig'ining yig'indisi: 1–1000, 1001–2000, 2001–3000 |
| bola: `write(quvur[1], &qism, sizeof(qism))` | natijani (8 bayt, `long`) quvurga yozish |
| bola: `_exit(bola + 10)` | tugash, chiqish kodi 10, 11, 12 |
| ota: `close(quvur[1])` | **o'zining** yozish uchini yopadi — aks holda `read` EOF olmaydi |
| ota: `while (read(...) == sizeof(qism))` | har bolaning natijasini o'qiydi |
| ota: `while (wait(&holat) > 0)` | hamma bolani kutadi (zombi qolmasin), kodlarni yig'adi |

**Tekshiruv:** 1+2+…+3000 = 3000·3001/2 = 4 501 500 ✓. Kodlar: 10 + 11 + 12 = 33 ✓. Oxirgi buyruq (`strace ... | grep -c`) yadroga nechta "jarayon yaratish" so'rovi yuborilganini sanaydi — **3** (strace — 29-bob).

**Sinab ko'ring:** `close(quvur[1]);` (otadagi) ni o'chiring — dastur nega abadiy kutib qoladi? (Ctrl+C bilan to'xtating.) `while (wait(...))` siklini o'chirib, dastur ishlayotganda boshqa terminalda
`ps aux | grep defunct` ni bajaring.

## Bob xulosasi (yodlash uchun)

1. **Syscall** — dasturning yadrodan so'rovi (`write`, `open`, `fork`...); dastur apparatga o'zi tega olmaydi. Xato — manfiy/−1 + `errno`.
2. **Fayl deskriptori** — kichik butun son (0, 1, 2 — stdin/stdout/stderr); `open` → `read`/`write` → `close`; `read` 0 = fayl oxiri; `read`/`write` so'ralgandan **kam** bo'lishi mumkin.
3. `fork` bir marta chaqiriladi, ikki marta qaytadi (`0` — bola, `>0` — ota); `exec` dasturni almashtiradi (qaytmaydi); `wait` — bolani kutadi.
4. `a | b` = `pipe` + ikki `fork` + `dup2` + **ortiqcha uchlarni yopish** (aks holda EOF kelmaydi).
5. Signal — asinxron xabar (`SIGINT`...); handler ichida faqat oddiy ishlar (`volatile sig_atomic_t` bayroq).

## Savol-javob

**`printf` va `write(1, ...)` farqi?**
`printf` — kutubxona funksiyasi: formatlaydi va buferlaydi. `write` — to'g'ridan-to'g'ri syscall.

**Nega `read` 4096 so'ralganda 100 qaytarishi mumkin?**
Terminal — foydalanuvchi Enter bosgan qatorni beradi; pipe — hozir bor ma'lumotni; fayl — oxirigacha qolganini. Doim qaytish qiymatiga qarang.

**`exit` va `_exit` farqi?**
`exit` — stdio buferlarini `fflush` qiladi va `atexit` funksiyalarini chaqiradi, keyin `_exit`. `fork` qilingan bolada exec xatosidan keyin `_exit` — aks holda otaning buferlari ikki marta chiqadi.

**Zombi jarayonlarni qanday ko'raman?**
Linux'da `ps aux` → holat `Z`. MyOS'da `ps` → `zombie`.

## O'zingizni tekshiring

1. `fork()` qaytish qiymatlari nimani bildiradi?
2. `execvp` dan keyingi qatorga qachon yetib kelinadi?
3. `ls | wc -l` da ota `fds[1]` ni yopmasa nima bo'ladi?
4. `read` 0 qaytarsa nimani bildiradi?
5. Nega signal handler ichida `printf` xavfli?

<details><summary>Javoblar</summary>

1. <0 — xato, 0 — bolada, >0 — otada (bolaning pid'i).
2. Faqat exec muvaffaqiyatsiz bo'lsa.
3. `wc` EOF olmaydi va abadiy kutadi (ota ham `wait` da kutadi).
4. Fayl oxiri / yozuvchilar yopilgan (EOF).
5. `printf` ichki qulf va bufer ishlatadi; signal o'sha paytda kelsa — deadlock yoki buzilish.
</details>

## Mashq

- **25** (`cp`), **26** (`wc`), **27** (`fork`/`exec`/`wait`), **28** (`pipe`/`dup2`).
- Qo'shimcha loyiha: **o'z mini-shell'ingiz** — qatorni o'qish, bo'shliq bo'yicha ajratish (15-mashq), `fork`+`execvp`+`waitpid` (27), ichki `cd` va `exit`, keyin `>` va `|` (28).
  Tayyor bo'lgach, MyOS `user/bin/sh.c` bilan solishtiring.

<!-- loyiha:boshi -->
## Loyiha: `wc` — tizim chaqiruvlari bilan

**Maqsad:** `printf`/`fopen`siz — faqat yadro bilan **to'g'ridan-to'g'ri** gaplashish: `open`, `read`, `write`,
`close`. Har bir C dasturi ichida aynan shu ish bajariladi (14.1).
**Bobdan ishlatiladi:** fayl deskriptorlari, `open` bayroqlari, `read` qaytargan bayt soni, xatolarni tekshirish.

**Talab:** fayl yozing, uni **kichik bufer** (8 bayt) bilan bo'lak-bo'lak o'qib, qator, so'z va baytlarni sanang
(`wc` kabi).
**Muhim:** `read` so'ralgandan **kam** bayt qaytarishi mumkin va faylning oxirida `0` qaytaradi. Shuning uchun
`read` har doim siklda chaqiriladi, `n` ta bayt qaytganini tekshirasiz.
**Ma'lumotlar:** `fd`, `bufer[8]`, sanagichlar, `ichida` (so'z ichidamizmi — 6-bobdagi bayroq).

```c
/* syswc.c - open / read / write / close bilan wc */
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    const char *fayl = "namuna.txt";
    const char matn[] = "salom dunyo\nikkinchi qator bor\n\nuchinchi\n";

    int fd = open(fayl, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    if (write(fd, matn, sizeof(matn) - 1) != (ssize_t)(sizeof(matn) - 1)) {
        perror("write");
        return 1;
    }
    close(fd);

    fd = open(fayl, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    char bufer[8];                              /* ataylab kichik: read() ko'p marta chaqiriladi */
    long baytlar = 0, qatorlar = 0, sozlar = 0;
    int ichida = 0, chaqiruvlar = 0;
    ssize_t n;
    while ((n = read(fd, bufer, sizeof(bufer))) > 0) {
        chaqiruvlar++;
        baytlar += n;
        for (ssize_t i = 0; i < n; i++) {
            char c = bufer[i];
            if (c == '\n')
                qatorlar++;
            if (c == ' ' || c == '\n' || c == '\t')
                ichida = 0;
            else if (!ichida) {                 /* bo'shliqdan harfga o'tish - yangi so'z */
                ichida = 1;
                sozlar++;
            }
        }
    }
    if (n < 0)
        perror("read");
    close(fd);
    unlink(fayl);

    printf("qatorlar: %ld, so'zlar: %ld, baytlar: %ld\n", qatorlar, sozlar, baytlar);
    printf("read() %d marta chaqirildi (8 baytdan)\n", chaqiruvlar);
    return 0;
}
```

```console
$ gcc -Wall -Wextra -g syswc.c -o syswc
$ ./syswc
qatorlar: 4, so'zlar: 6, baytlar: 41
read() 6 marta chaqirildi (8 baytdan)
$ printf 'salom dunyo\nikkinchi qator bor\n\nuchinchi\n' | wc
      4       6      41
$ strace -e trace=openat,read,write,close,unlink -o iz.txt ./syswc > /dev/null && sed -n '/namuna.txt/,$p' iz.txt
openat(AT_FDCWD, "namuna.txt", O_WRONLY|O_CREAT|O_TRUNC, 0644) = 3
write(3, "salom dunyo\nikkinchi qator bor\n\n"..., 41) = 41
close(3)                                = 0
openat(AT_FDCWD, "namuna.txt", O_RDONLY) = 3
read(3, "salom du", 8)                  = 8
read(3, "nyo\nikki", 8)                 = 8
read(3, "nchi qat", 8)                  = 8
read(3, "or bor\n\n", 8)                = 8
read(3, "uchinchi", 8)                  = 8
read(3, "\n", 8)                        = 1
read(3, "", 8)                          = 0
close(3)                                = 0
unlink("namuna.txt")                    = 0
write(1, "qatorlar: 4, so'zlar: 6, baytlar"..., 75) = 75
+++ exited with 0 +++
```

Sizning natijangiz haqiqiy `wc` bilan bir xil (`4 6 41`). `strace` (29-bob) dasturning yadroga qilgan
chaqiruvlarini ko'rsatadi: `openat(...) = 3` — `fd = 3` (0, 1, 2 band, 3 — birinchi bo'sh), keyin `read(3, ...)` yetti marta
(oxirgisi `0` qaytaradi — fayl tugadi), so'ng `close`, `unlink`.

**Kengaytiring:** `bufer` ni 4096 ga o'zgartiring — `read()` necha marta chaqiriladi? `open` ga mavjud bo'lmagan fayl bering va
`perror` xabarini o'qing.

## Mustaqil loyiha: mini `grep` ★★★

**Vazifa:** standart kirishdan (`fd 0`) qatorlarni o'qib, naqsh (matn) bor qatorlarni chiqaruvchi dastur.
Fayl: `mgrep.c`. **Faqat** `read()` va `write()` (fd 0, 1, 2) bilan kirish-chiqish qiling —
`printf`, `puts`, `fgets`, `fopen`, `getchar` **taqiqlanadi**. Matnni formatlash uchun `snprintf`, qidirish
uchun `strstr` mumkin.

**Buyruq qatori:** `./dastur [-n] [-c] naqsh`
- `-n` — har bir topilgan qator oldiga `<qator raqami>:` qo'shadi (`4:yana bir salom`);
- `-c` — qatorlarni chiqarmaydi, faqat topilganlar **sonini** chiqaradi;
- bayroqlar ixtiyoriy tartibda, naqsh — birinchi `-` bilan boshlanmagan argument;
- naqsh yo'q bo'lsa: `Ishlatish: mgrep [-n] [-c] naqsh` ni **fd 2** ga (stderr) yozing, chiqish kodi 2.

**Chiqish kodi:** kamida bitta qator topilsa — `0`, hech biri topilmasa — `1` (haqiqiy `grep` kabi).

**Qiyinchiliklar (testda bor):**
1. `read()` qatorni **o'rtasida** uzishi mumkin — qator bir necha `read` ga bo'linishi mumkin. Test faylida 5000 belgili qator bor.
2. Fayl oxirgi qatori `\n` **bilan tugamasligi** mumkin — u ham qator! Chiqarilganda oxiriga `\n` qo'shiladi.
3. Qidiruv katta-kichik harfga **sezgir** (`SALOM` ≠ `salom`).

**Kirish fayli** (`darslik/loyihalar/14_mini_grep/kirish.txt`): 7 qator (5-qator 5000 ta `x`, oxirgi qatorda `\n` yo'q):

```text
salom dunyo
Bugun havo yaxshi
SALOM katta harf
yana bir salom, salom!
xxxxxxxxxx...(5000 ta x)
dunyo tinch
oxirgi qator salom (yangi qatorsiz)
```

**1-sinov:** `./dastur salom < kirish.txt; echo "chiqish kodi: $?"`

```text
salom dunyo
yana bir salom, salom!
oxirgi qator salom (yangi qatorsiz)
chiqish kodi: 0
```

**2-sinov:** `./dastur -n dunyo < kirish.txt; echo "chiqish kodi: $?"`

```text
1:salom dunyo
6:dunyo tinch
chiqish kodi: 0
```

**3-sinov:** `./dastur -c salom < kirish.txt; echo "chiqish kodi: $?"` va keyin `./dastur yoqsoz < kirish.txt; echo "chiqish kodi: $?"`

```text
3
chiqish kodi: 0
chiqish kodi: 1
```

**Maslahat** (yechim emas):
- Har `read` dan keyin olingan baytlarni **qator buferiga** to'plang (kattalashadigan — `realloc`, yoki katta statik massiv
  ustiga ehtiyot bo'lib). `\n` topilganda qatorni `'\0'` bilan yopib, tekshirasiz va buferni tozalaysiz.
- `read` 0 qaytarganda (fayl tugadi) buferda yig'ilgan, `\n` siz qolgan oxirgi qatorni ham tekshiring.
- Chiqarish: `write(1, qator, uzunlik)` va alohida `write(1, "\n", 1)`. Qisman yozilishi (`write` kam qaytarishi) — oddiy
  fayl/quvur uchun kam uchraydi, lekin qanday hal qilinishini o'ylab ko'ring (loop).
- Raqam prefiksi uchun: `int m = snprintf(t, sizeof t, "%d:", raqam); write(1, t, m);`.

**Tekshirish** (uch buyruq):

```bash
D=~/C_loyha/darslik/loyihalar/14_mini_grep
gcc -Wall -Wextra -g -fsanitize=address,undefined mgrep.c -o dastur
(./dastur salom < $D/kirish.txt; echo "chiqish kodi: $?") | diff - $D/kutilgan.txt && echo "1: TO'G'RI"
(./dastur -n dunyo < $D/kirish.txt; echo "chiqish kodi: $?") | diff - $D/kutilgan_2.txt && echo "2: TO'G'RI"
(./dastur -c salom < $D/kirish.txt; echo "chiqish kodi: $?"; ./dastur yoqsoz < $D/kirish.txt; echo "chiqish kodi: $?") | diff - $D/kutilgan_3.txt && echo "3: TO'G'RI"
```
<!-- loyiha:oxiri -->

Keyingi bob: [15-bob. Parallellik: oqimlar, qulflar, atomiklar](15-parallellik.md)
