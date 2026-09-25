# 12 — "Hamma narsa fayl": VFS, tmpfs, /dev, pipe, terminal, libc va shell

Kod: `kernel/fs/{vfs,tmpfs,devfs,pipe,initrd}.c`, `kernel/sys/{sys_fs,uaccess}.c`, `kernel/drivers/tty.c`,
`user/libc/*`, `user/include/*`, `user/bin/{sh,fstest,ls,...}.c`

Bu bosqichdan keyin tizim oddiy Unix kabi ishlatiladi:

```
myos:/$ ls /bin | grep s | head -3
myos:/$ echo salom > /home/a.txt ; cat /home/a.txt
myos:/$ seq 20000 | grep 7 | tail -2
myos:/$ cd /home && mkdir -p loyiha/src && ls -l
```

## 1. Qatlamlar

```
 user:   printf("%d", x)        ls, cat, sh ...
           │  stdio bufer (user/libc/stdio.c)
           ▼
         write(1, buf, n)       user/libc/unistd.c: RAX=1, syscall
─────────────────────────────── ring 3 → ring 0 ───────────────────────────
 yadro:  sys_write(fd, ...)     sys/sys_fs.c: current->files[fd] -> struct file
           │
           ▼
         file_write(f, ...)     fs/vfs.c: f->fops->write(...)  ← "virtual" chaqiruv
           │
     ┌─────┼──────────────┬──────────────┐
   tmpfs  devfs          pipe          (ext2 - keyingi qism)
   RAM    tty.c, null    halqa bufer
```

**VFS** — bu interfeys: `struct inode_ops` (nomlar bilan ishlash: `lookup`, `create`, `mkdir`, `unlink`,
`rename`, `readdir`) va `struct file_ops` (ochiq fayl: `read`, `write`, `ioctl`). Har bir fayl tizimi o'z
jadvalini beradi. C tilida bu funksiya ko'rsatkichlari orqali qilinadi; C++ dagi `virtual`, Java dagi
`interface` bilan bir xil g'oya. Linux'da aynan shu nomlar ishlatiladi.

## 2. Uchta obyekt: inode, file, fd

```
 jarayon A                          jarayon B
 files[0] ─┐                        files[3] ──► file (offset 100) ──┐
 files[1] ─┼─► file (/dev/console)                                  │
 files[2] ─┘                                                        ▼
 files[3] ──► file (offset 0, O_RDONLY) ──────────────────────► inode #12 ("a.txt" hajmi 4096)
                                                                    ▲
                                  papka /home: "a.txt" -> #12 ──────┘
```

| obyekt | nima | nechta |
|---|---|---|
| **inode** | faylning o'zi: tur, hajm, vaqtlar, ma'lumot qayerda. **Nomi yo'q.** | har bir fayl uchun 1 ta |
| **file** | *ochiq* fayl: qaysi inode, **offset**, rejim (`O_RDONLY`...) | har bir `open()` uchun 1 ta |
| **fd** | jarayonning `files[]` jadvalidagi **indeks** | bitta file'ga bir nechta fd |

- `fork()` fd jadvalini nusxalaydi, lekin **file'larni ulashadi** (`refcount++`). Shuning uchun ota va bola
  bitta offsetni siljitadi. `(echo a; echo b) > f` ikkala qatorni ketma-ket yozishi shunga bog'liq.
- `dup2(3, 1)` degani "1-slot endi 3-slot ko'rsatgan file'ni ko'rsatsin". Shell'dagi `>` shu bilan qilinadi.
- `open()` **eng kichik bo'sh** fd raqamini beradi. Bu POSIX talabi.

### Refcount va "o'chirilgan, lekin tirik" fayllar

`unlink("a.txt")` faqat **nomni** o'chiradi (`nlink--`). Inode quyidagi ikkala shart bajarilganda
bo'shatiladi: `nlink == 0` **va** `refcount == 0` (hech kim ochiq ushlab turmaydi). `fstest` buni
tekshiradi: ochiq faylni o'chiradi, keyin undan o'qiydi. Linux'da ham xuddi shunday, shuning uchun
ishlayotgan dasturning faylini yangilash xavfsiz.

## 3. Yo'l bo'ylab yurish va mount

`vfs_lookup("/mnt/disk/a.txt")` quyidagicha ishlaydi:

1. Boshlanish nuqtasi: `/` bilan boshlansa — ildiz, aks holda jarayonning `cwd` inode'i.
2. Har bir komponent uchun `dir->iops->lookup(dir, "mnt")` chaqiriladi.
3. Topilgan inode'ga boshqa fayl tizimi **ulangan** bo'lsa (`inode->mounted`), o'sha FS ildiziga o'tiladi.
4. `..` fayl tizimi ildizida bo'lsa, `covered` orqali **ota fayl tizimiga** qaytiladi.

`mount -t tmpfs none /mnt` shu ikki ko'rsatkichni o'rnatadi. Endi `/mnt` ichidagi hamma narsa yangi FS'da.

## 4. tmpfs — RAM'dagi fayl tizimi

Root (`/`) — tmpfs. Boot paytida GRUB `initrd.tar` ni xotiraga yuklaydi, `fs/initrd.c` esa undagi har bir
papka va faylni tmpfs'da yaratadi. Linux'ning initramfs'i ham shunday ishlaydi.

Fayl ma'lumotlari `struct page *pages[]` massivida saqlanadi. `pages[i] == NULL` bo'lsa, u **teshik**
hisoblanadi va o'qilganda nollar qaytaradi. `lseek(fd, 1 GB, SEEK_SET); write(fd, "x", 1)` 1 GB emas,
bitta sahifa xotira oladi (sparse file).

## 5. /dev — qurilmalar ham fayl

```
crw-rw-rw-  1    1,    3  null        MAJOR 1 = "mem" drayveri, MINOR 3 = null
crw-------  1    5,    1  console     MAJOR 5 = tty drayveri
```

Qurilma fayli — oddiy inode, faqat turi `S_IFCHR` va `rdev = MKDEV(major, minor)`. `open()` paytida VFS
**major** raqami bo'yicha drayverning `file_ops` jadvalini oladi (`chrdev_get`). `ls -l /dev` hajm
o'rniga "major, minor" ni ko'rsatadi.

## 6. Pipe

16 KB halqa bufer va ikkita `struct file` (o'qish uchi, yozish uchi):

- Bufer bo'sh → o'quvchi **uxlaydi**. To'la → yozuvchi **uxlaydi** (backpressure). `seq 20000 | grep 7`
  da `seq` `grep` dan tezroq ishlaydi, lekin xotirani to'ldirib yubora olmaydi.
- **Barcha** yozish uchlari yopilsa, o'quvchi `read() == 0` (EOF) oladi.
- O'quvchi qolmasa, yozuvchi `SIGPIPE` oladi va yozish `-EPIPE` qaytaradi.

### Eng mashhur pipe xatosi

```c
pipe(p);
if (fork() == 0) { dup2(p[1], 1); exec("ls"); }
if (fork() == 0) { dup2(p[0], 0); exec("wc"); }   // <- wc HECH QACHON tugamaydi!
wait(); wait();
```

`wc` EOF'ni kutadi. EOF esa pipe'ning **barcha** yozish uchlari yopilgandagina keladi. Bu kodda yozish uchi
hali uchta joyda ochiq: otada, `wc` ning o'zida (fork orqali meros qolgan) va `ls` da. To'g'ri qoida: har bir
jarayon o'ziga kerak bo'lmagan uchlarni **darhol** yopishi kerak. `sh.c` dagi `run_pipeline()` ga qarang.

## 7. Terminal: line discipline (`drivers/tty.c`)

Klaviatura "xom" belgilar beradi. `cat` ga esa **tahrirlangan qator** kerak. Bu ish har bir dasturda
emas, yadroda bajariladi:

| tugma | kanonik rejimda |
|---|---|
| harf | qatorga qo'shiladi va ekranga qaytariladi (echo) |
| Backspace | oxirgi belgi o'chadi (`\b \b`) |
| Ctrl-U | butun qator o'chadi |
| Enter | qator dasturga beriladi (`read()` qaytadi) |
| Ctrl-D | bo'sh qatorda **EOF** (`read() == 0`), aks holda qatorni `\n` siz yuboradi |
| Ctrl-C / Ctrl-Z | oldingi plan guruhiga SIGINT / SIGTSTP ([14-signallar.md](14-signallar.md)) |

Muharrir va o'yinlar **xom rejimga** o'tadi: `tcgetattr` → `c_lflag &= ~(ICANON|ECHO)` → `tcsetattr`.
Bular `ioctl(fd, TCGETS/TCSETS)` chaqiruvlari bo'lib, raqamlar va `struct termios` Linux bilan bir xil.
`isatty(fd)` ham shu ioctl'dan foydalanadi: faqat terminal `TCGETS` ni qabul qiladi.

```
myos:/$ cat > /tmp/x
birinchi qator
ikkinchi^D                    <- Ctrl-D: cat EOF oladi va tugaydi
```

## 8. Xatolar: -errno → errno

Yadro xato bo'lsa **manfiy kod** qaytaradi (`-ENOENT` = -2). libc esa POSIX an'anasiga moslab uni `errno`
ga yozadi va `-1` qaytaradi (`user/libc/syscall.h: __sysret`). Raqamlar Linux bilan **aynan bir xil**
(`include/myos/abi.h`), `strerror()` ularni o'zbekchaga tarjima qiladi:

```
myos:/$ cat /yoq
cat: /yoq: Bunday fayl yoki papka yo'q
myos:/$ rmdir /bin
rmdir: /bin: Papka bo'sh emas
```

## 9. libc — o'z standart kutubxonamiz

```
user/include/   stdio.h unistd.h fcntl.h string.h stdlib.h dirent.h sys/stat.h termios.h ...
user/libc/      crt0.asm  unistd.c (syscall'lar)  stdio.c (FILE*)  printf.c  string.c
                stdlib.c  malloc.c  dirent.c  time.c
build/user/libc.a   <- statik kutubxona: linker undan faqat kerakli .o larni oladi
```

- Dasturlar endi `#include <stdio.h>` yozadi va xuddi Linux'dagidek ko'rinadi. `-nostdinc` bayrog'i
  kompyuterdagi glibc sarlavhalari **tasodifan** ulanib qolmasligini ta'minlaydi.
- **stdio buferlash:** terminalga yozishda bufer qator bo'yicha bo'shatiladi, fayl yoki pipe'ga yozishda
  4 KB to'lganda, stderr esa umuman buferlanmaydi. `isatty()` qaysi holat ekanini aniqlaydi.
- `exit()` buferlarni bo'shatadi, `_exit()` bo'shatmaydi. Shuning uchun `fork` dan keyin bolada
  `_exit` ishlatiladi. Bizning `fork()` o'zi ham avval `fflush(NULL)` qiladi, aks holda bufer ikki marta
  chiqadi.
- `fgets` EOF'da buferga **tegmasligi** kerak (C standarti). Bu qoida buzilganda `tail` oxirgi qatorni
  yo'qotgan edi va xato test orqali topildi.

## 10. Shell

`sh.c` uch bosqichda ishlaydi: **lekser** (so'zlar va operatorlar) → **parser** (pipeline: buyruqlar va
yo'naltirishlar) → **bajarish** (fork, dup2, exec, wait).

Muhim tafsilotlar:

- `$?` va `$X` qatorni o'qish paytida emas, har bir buyruq **bajarilishidan oldin** ochiladi. Aks holda
  `false || echo $?` `0` chiqaradi.
- `cd` **ichki** buyruq bo'lishi shart. Bola jarayon o'z papkasini o'zgartirsa, bu otaga ta'sir qilmaydi.
- Ichki buyruq yo'naltirish bilan (`pwd > f`) chaqirilsa, shell fd'larni `dup` bilan saqlaydi, yo'naltiradi,
  buyruqni bajaradi va fd'larni qaytaradi.
- Shell stdin'ni **baytma-bayt** o'qiydi. Bufer bilan o'qisa, skriptdagi keyingi qatorlarni "yutib"
  yuborishi va ular keyingi ishga tushgan dasturga yetib bormasligi mumkin edi.

## 11. Haqiqiy xato: o'lgan jarayonning CR3'i

Bu bosqichda sinov paytida tizim ba'zan **o'z-o'zidan qayta yuklanardi** (triple fault). `-d cpu_reset,int`
bilan olingan QEMU logi quyidagilarni ko'rsatdi:

```
v=40 (taymer)  CPU scheduler'da uxlayapti, CR3=0fa75000
v=0e e=0010    RIP=interrupt_dispatch  CR2=interrupt_dispatch    <- YADRO KODI "yo'q"!
... (stek to'lguncha takrorlanadi) ... v=08 (double fault) → triple fault
```

Voqealar ketma-ketligi:

1. A jarayon CPU0'da ishlaydi (CR3 = A ning jadvali), taymer uni to'xtatadi va CPU0 scheduler'ga qaytadi.
   Bunda CR3 o'zgarmaydi.
2. A endi CPU1'da ishlaydi, `exit()` yoki `exec()` qiladi va eski jadvallarini **bo'shatadi**.
3. CPU0 hali ham o'sha CR3 bilan `hlt` da uxlaydi. Bo'shatilgan sahifa boshqa narsaga ishlatiladi.
4. Taymer uzilishi keladi. Yadro kodi TLB'da bo'lmasa, CPU bo'shatilgan va endi axlat bilan to'lgan
   xotirani sahifa jadvali sifatida o'qiydi. Natijada #PF, keyin #DF, keyin triple fault yuz beradi.

**Tuzatish** (`scheduler_loop`): jarayon CPU'ni qaytarishi bilan darhol yadro jadvaliga o'tiladi
(`vmm_switch(vmm_kernel_pml4())`). xv6 buni `switchkvm()` deb ataydi. Linux esa "lazy TLB" va
`mm_count` hisoblagichidan foydalanadi: bo'sh CPU begona jadvalni ishlatib turadi, lekin jadval u CPU
undan chiqmaguncha bo'shatilmaydi.

**Saboq:** SMP'da "kim bu xotiradan hali foydalanyapti?" degan savol har doim bor. Xato 6 urinishdan
taxminan 2 tasida chiqdi. Shuning uchun testlarni bir necha marta ishga tushiring.

## Sinab ko'ring

```
fstest                            VFS'ning 53 ta tekshiruvi
ls -la /dev ; stat /dev/console
seq 100000 | wc                   ~600 KB pipe orqali
X=5 ; echo "X=$X" '$X'
ls /yoq 2> /tmp/xato ; cat /tmp/xato
echo 'echo skriptdan salom' > /tmp/s.sh ; sh /tmp/s.sh
```

## Mashqlar

1. **Oson:** `/dev/full` qurilmasini qo'shing: yozish `-ENOSPC` qaytarsin, o'qish nollarni bersin.
2. **Oson:** `utimes` syscall'ini qo'shing va `touch` fayl vaqtini ham yangilasin.
3. **O'rta:** `ln` (hard link): `inode_ops->link` va `nlink++`. Keyin `fstest` ga "ikki nom, bitta inode"
   testini qo'shing.
4. **O'rta:** `O_CLOEXEC`: exec paytida shu bayroqli fd'lar yopilsin. Nega bu xavfsizlik uchun muhim?
5. **O'rta:** tty'da belgilarni o'quvchi kontekstida emas, **uzilish** kontekstida qayta ishlang, shunda
   dastur hech narsa o'qimayotganda ham echo darhol ko'rinadi. Qaysi qulf kerak bo'ladi?
6. **Qiyin:** `/proc` fayl tizimi: `/proc/meminfo`, `/proc/<pid>/status`. Keyin `ps` va `free` ni
   syscall'lar o'rniga shu fayllardan o'qiydigan qilib qayta yozing (Linux shunday qiladi).
7. **Qiyin:** grep'ga haqiqiy regex qo'shing (`.`, `*`, `[a-z]`). Russ Cox'ning "Regular Expression
   Matching Can Be Simple And Fast" maqolasidagi Thompson NFA usulidan foydalaning.
8. **Qiyin:** "lazy TLB": scheduler'da CR3'ni almashtirmang, lekin `mm` ga `active_users` hisoblagichini
   qo'shing va jadvallarni faqat hech bir CPU ishlatmayotganda bo'shating. Samaradorlikni o'lchang.
