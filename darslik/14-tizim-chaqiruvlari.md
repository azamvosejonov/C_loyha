# 14-bob. Tizim chaqiruvlari: fayllar, jarayonlar, pipe, signallar

> **Bu bobdan keyin:** dastur yadro bilan qanday gaplashishini, fayl deskriptorlarini, `open/read/write`,
> `fork/exec/wait`, `pipe/dup2` va signallarni bilasiz — ya'ni shell yozish uchun kerak bo'lgan hamma
> narsani. Bu bob — **yadroning tashqi eshigi**: keyingi qadamda (MyOS) shu eshikning ichkarisini yozasiz.
> Mashqlar: 25–28.

## 14.1. Tizim chaqiruvi (syscall) nima

Oddiy dastur (user rejimi, CPU'ning 3-halqasi) apparatga tega olmaydi: diskni o'qiy olmaydi, ekranga
to'g'ridan-to'g'ri yoza olmaydi, boshqa dasturning xotirasini ko'ra olmaydi. Bularning hammasi uchun
u **yadrodan so'raydi**:

```text
dastur:  write(1, "salom\n", 6)
libc:    rax = 1 (SYS_write), rdi = 1, rsi = buf, rdx = 6;  syscall     <- maxsus CPU buyrug'i
CPU:     yadro rejimiga (0-halqa) o'tadi, yadroning kirish nuqtasiga sakraydi
yadro:   argumentlarni tekshiradi -> fd 1 qaysi fayl? -> terminal drayveri -> ekran
         natija (6 yoki -EBADF) -> rax;  sysret
dastur:  write() 6 qaytardi
```

MyOS'da har bir qadamni o'qishingiz mumkin:
`user/libc/unistd.c` (write) → `user/libc/syscall.h` (`__syscall3` — inline asm) →
`kernel/arch/syscall_entry.asm` (kirish) → `kernel/sys/syscall.c` (`switch (nr)`) →
`kernel/sys/sys_fs.c` → `kernel/fs/vfs.c` → `kernel/drivers/tty.c`.

Syscall **qimmat** (rejim almashishi, tekshiruvlar) — shuning uchun libc buferlaydi (12-bob).

`man 2 write` — syscall hujjati (2-bo'lim), `man 3 printf` — kutubxona funksiyasi (3-bo'lim).

## 14.2. Fayl deskriptorlari

Fayl deskriptori (fd) — kichik butun son, jarayonning **ochiq fayllar jadvalidagi indeks**:

```text
jarayon:  fd 0 -> terminal (stdin)
          fd 1 -> terminal (stdout)
          fd 2 -> terminal (stderr)
          fd 3 -> /home/ali/ma'lumot.txt (offset 120)
```

"Unix'da hamma narsa — fayl": oddiy fayl, terminal, pipe, disk qurilmasi (`/dev/sda`), tasodifiy son
generatori (`/dev/urandom`) — hammasi bir xil `read`/`write` bilan ishlaydi. Yadro ichida buni VFS va
`file_ops` jadvali ta'minlaydi (7-bob).

## 14.3. `open`, `read`, `write`, `close`, `lseek`

```c
#include <fcntl.h>
#include <unistd.h>

int fd = open("ma'lumot.txt", O_RDONLY);
if (fd < 0) { perror("open"); return 1; }

char buf[4096];
ssize_t n;
while ((n = read(fd, buf, sizeof(buf))) > 0) {
    /* buf[0..n-1] - o'qilgan baytlar ('\0' YO'Q - bu satr emas!) */
}
if (n < 0) perror("read");
close(fd);
```

| Chaqiruv | Qaytaradi | E'tibor |
|---|---|---|
| `open(yol, bayroqlar, rejim)` | yangi fd (eng kichik bo'sh raqam) yoki −1 | `O_RDONLY/O_WRONLY/O_RDWR`, `O_CREAT`, `O_TRUNC`, `O_APPEND`; `O_CREAT` bilan rejim: `0644` |
| `read(fd, buf, n)` | o'qilgan baytlar soni, **0 = fayl oxiri**, −1 = xato | **n dan kam** qaytishi normal (pipe, terminal, fayl oxiri) |
| `write(fd, buf, n)` | yozilgan baytlar soni yoki −1 | **n dan kam** yozishi mumkin — takrorlash kerak (25-mashq) |
| `close(fd)` | 0 / −1 | Yopilmasa — "fd sizib chiqadi" (leak) |
| `lseek(fd, offset, qayerdan)` | yangi pozitsiya | `SEEK_SET/CUR/END` |

**"Qisman yozish" to'g'ri ishlovi:**

```c
ssize_t hammasini_yoz(int fd, const char *buf, size_t n)
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
```

`0644` — **sakkizlik** son (2-bob): egasi o'qiydi+yozadi (6), guruh va boshqalar faqat o'qiydi (4).

## 14.4. Jarayonlar: `fork`, `exec`, `wait`

Unix'ning eng mashhur g'oyasi — yangi dastur ishga tushirish ikki qadamga bo'lingan:

```c
pid_t pid = fork();                 /* 1) o'zimning NUSXAMNI yaratish */
if (pid < 0) {
    perror("fork");
} else if (pid == 0) {
    /* BOLA jarayon - fork() 0 qaytardi */
    char *argv[] = { "ls", "-l", NULL };
    execvp("ls", argv);             /* 2) dasturimni "ls" ga almashtirish */
    perror("execvp");               /* bu yerga faqat exec XATO bo'lsa yetib kelinadi */
    _exit(127);
} else {
    /* OTA jarayon - fork() bolaning pid'ini qaytardi */
    int status;
    waitpid(pid, &status, 0);       /* bola tugashini kutish */
    if (WIFEXITED(status))
        printf("chiqish kodi: %d\n", WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        printf("signal bilan o'ldi: %d\n", WTERMSIG(status));
}
```

- **`fork()` bir marta chaqiriladi, ikki marta qaytadi** — otada va bolada. Farqi — qaytish qiymati.
- Bola otaning **to'liq nusxasi**: xotira, o'zgaruvchilar, ochiq fayllar. Lekin keyin ular mustaqil —
  bolada o'zgaruvchini o'zgartirish otaga ta'sir qilmaydi.
  (Yadro butun xotirani darhol nusxalamaydi — **copy-on-write**: sahifalar faqat birinchi yozishda
  nusxalanadi. MyOS: `kernel/proc/process.c` → `proc_fork`, `docs/11-fork-cow.md`.)
- **`exec`** joriy jarayonning dasturini almashtiradi: kod, ma'lumot, stek — yangi; pid va ochiq fd'lar —
  o'sha. Muvaffaqiyatli bo'lsa, **hech qachon qaytmaydi**. (`execvp` — PATH bo'yicha qidiradi.)
- **`wait`** — bola tugagach, uning chiqish holatini oladi. Olinmagan tugagan bola — **"zombi"** (jarayon
  jadvalida qolib ketadi). Otasi o'lgan bola — "yetim", uni `init` (PID 1) asrab oladi.

**Nega ikki qadam?** `fork` va `exec` orasida bola **o'zini sozlay oladi**: fd'larni almashtirish
(`>` yo'naltirish, pipe), papkani o'zgartirish, signallarni sozlash. Shell aynan shu oraliqda ishlaydi.

## 14.5. `pipe` va `dup2` — shell'dagi `a | b`

```c
int fds[2];
pipe(fds);                  /* fds[0] - o'qish uchi, fds[1] - yozish uchi */
```

`dup2(eski, yangi)` — `yangi` fd ni `eski` ning nusxasiga aylantiradi. `ls | wc -l` shunday bajariladi:

```c
int fds[2];
pipe(fds);
if (fork() == 0) {                  /* 1-bola: ls */
    dup2(fds[1], 1);                /* stdout -> pipe'ning yozish uchi */
    close(fds[0]); close(fds[1]);
    execlp("ls", "ls", NULL);
    _exit(127);
}
if (fork() == 0) {                  /* 2-bola: wc -l */
    dup2(fds[0], 0);                /* stdin <- pipe'ning o'qish uchi */
    close(fds[0]); close(fds[1]);
    execlp("wc", "wc", "-l", NULL);
    _exit(127);
}
close(fds[0]); close(fds[1]);       /* OTA ham yopishi SHART */
wait(NULL); wait(NULL);
```

**Nega hamma joyda `close`?** Pipe'ning o'qiydigan tomoni **EOF** (`read` → 0) ni faqat yozish uchining
**hamma** nusxalari yopilganda oladi. Ota `fds[1]` ni yopmasa, `wc` abadiy kutadi. 28-mashqning
asosiy tuzog'i shu. MyOS: `user/bin/sh.c` (pipeline), `kernel/fs/pipe.c` (yadro tomoni — halqa bufer + uxlash/uyg'otish).

`>` yo'naltirish ham xuddi shunday: bolada `int fd = open("f.txt", O_WRONLY|O_CREAT|O_TRUNC, 0644); dup2(fd, 1); close(fd);`.

## 14.6. Signallar

Signal — jarayonga yuboriladigan asinxron "xabar":

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
#include <signal.h>

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
    sigaction(SIGINT, &sa, NULL);
    while (!toxta)
        pause();                /* signal kelguncha uxlash */
    printf("Ctrl-C bosildi, chiroyli tugatyapman\n");
    return 0;
}
```

**Handler ichida xavfli:** `printf`, `malloc` — ular qulf ushlab turgan paytda signal kelsa, deadlock.
Faqat "async-signal-safe" funksiyalar (`write`, `_exit`...) va `volatile sig_atomic_t` bayroq.

Yadro signalni qanday yetkazadi: foydalanuvchi stekiga "signal kadri" yozib, `rip` ni handler'ga
o'zgartiradi; handler tugagach `sigreturn` syscall'i eski holatni tiklaydi. MyOS: `kernel/proc/signal.c`
(`setup_frame`, `sys_sigreturn` — lab'lar), `docs/14-signallar.md`.

## 14.7. Katalog va fayl ma'lumotlari

```c
struct stat st;
if (stat("fayl.txt", &st) == 0)
    printf("hajm: %ld bayt\n", (long)st.st_size);

DIR *d = opendir(".");
struct dirent *e;
while ((e = readdir(d)) != NULL)
    printf("%s\n", e->d_name);
closedir(d);
```

`mkdir`, `rmdir`, `unlink` (faylni o'chirish), `rename`, `chdir`, `getcwd` — shell utilitalari
(`ls`, `rm`, `mv`...) shular ustida qurilgan. MyOS'dagi `user/bin/*.c` — har biri 50–150 qatorli misol.

## 14.8. Savol-javob

**`printf` va `write(1, ...)` farqi?**
`printf` — kutubxona funksiyasi: formatlaydi va buferlaydi. `write` — to'g'ridan-to'g'ri syscall.

**Nega `read` 4096 so'ralganda 100 qaytarishi mumkin?**
Terminal — foydalanuvchi Enter bosgan qatorni beradi; pipe — hozir bor ma'lumotni; fayl — oxirigacha
qolganini. Doim qaytish qiymatiga qarang.

**`exit` va `_exit` farqi?**
`exit` — stdio buferlarini `fflush` qiladi va `atexit` funksiyalarini chaqiradi, keyin `_exit`. `fork`
qilingan bolada exec xatosidan keyin `_exit` — aks holda otaning buferlari ikki marta chiqadi.

**Zombi jarayonlarni qanday ko'raman?**
Linux'da `ps aux` → holat `Z`. MyOS'da `ps` → `zombie`.

## 14.9. O'zingizni tekshiring

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

## 14.10. Mashqlar

- **25** (`cp`), **26** (`wc`), **27** (`fork`/`exec`/`wait`), **28** (`pipe`/`dup2`).
- Qo'shimcha loyiha: **o'z mini-shell'ingiz** — qatorni o'qish, bo'shliq bo'yicha ajratish (15-mashq),
  `fork`+`execvp`+`waitpid` (27), ichki `cd` va `exit`, keyin `>` va `|` (28). Tayyor bo'lgach, MyOS
  `user/bin/sh.c` bilan solishtiring.

Keyingi bob: [15-bob. Parallellik: oqimlar, qulflar, atomiklar](15-parallellik.md)
