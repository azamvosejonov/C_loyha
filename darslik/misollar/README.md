# Misollar: har bir bob uchun to'liq ishlaydigan dasturlar

Darslik ichidagi kod ko'pincha **parcha** (fragment) ko'rinishida: masalan, faqat `#define N 10`
yoki bitta funksiya. Parchani o'zi kompilyatsiya qilib bo'lmaydi — unga `#include` va `main` kerak.
Bu papkada esa har bir bob uchun **to'liq** dastur bor: nusxa olmasdan, shunchaki yig'ib ishga
tushirasiz va bobdagi har bir g'oyani o'z ko'zingiz bilan ko'rasiz.

Har bir fayl boshidagi izohda uchta narsa yozilgan:

1. **Ishga tushirish** — aynan qaysi buyruqni terish kerak;
2. **Kutilgan natija** — ekranda nima chiqishi kerak (o'zingiznikini solishtiring);
3. **Sinab ko'ring** — dasturni o'zgartirib, nima bo'lishini kuzatadigan kichik topshiriqlar.
   Eng ko'p narsani aynan shu yerda o'rganasiz.

## Qanday ishlatish

```bash
cd darslik/misollar
gcc -Wall -Wextra -g 10_makrolar.c -o makrolar     # yig'ish
./makrolar                                          # ishga tushirish
```

Keyin faylni muharrirda oching (`nano 10_makrolar.c`), izohlarni o'qing, **bitta narsani o'zgartiring**,
qayta yig'ing va natija qanday o'zgarganini kuzating. Faylni buzib qo'ysangiz — qaytarish oson:
`git checkout 10_makrolar.c`.

## Ro'yxat

| Bob | Misol | Nimani ko'rsatadi | Buyruq |
|---|---|---|---|
| [0](../00-kirish.md) | [00_salom.c](00_salom.c) | Birinchi dastur, `printf` | `gcc -Wall -Wextra -g 00_salom.c -o salom && ./salom` |
| [1](../01-kompilyatsiya.md) | [01_kompilyatsiya.sh](01_kompilyatsiya.sh) | `-E`, `-S`, `-c`, bog'lash: har bosqich natijasini qanday ko'rish (`.i` — matn, `.o` — `nm`/`objdump` bilan) | `sh 01_kompilyatsiya.sh` |
| [2](../02-turlar.md) | [02_turlar.c](02_turlar.c) | `sizeof`, toshish, butun bo'lish, sakkizlik son | `gcc -Wall -Wextra -g 02_turlar.c -o turlar && ./turlar` |
| [3](../03-operatorlar.md) | [03_bitlar.c](03_bitlar.c) | Bayroqlar (`\|`, `&`, `~`), sahifaga tekislash, bitlarni sanash | `gcc -Wall -Wextra -g 03_bitlar.c -o bitlar && ./bitlar` |
| [4](../04-boshqaruv.md) | [04_boshqaruv.c](04_boshqaruv.c) | FizzBuzz, `switch`, `size_t` bilan teskari sikl | `gcc -Wall -Wextra -g 04_boshqaruv.c -o boshqaruv && ./boshqaruv` |
| [5](../05-funksiyalar.md) | [05_funksiyalar.c](05_funksiyalar.c) | Qiymat va ko'rsatkich orqali uzatish, `static`, rekursiya | `gcc -Wall -Wextra -g 05_funksiyalar.c -o funksiyalar && ./funksiyalar` |
| [6](../06-massivlar-satrlar.md) | [06_satrlar.c](06_satrlar.c) | `sizeof` va `strlen`, `strcmp`, `snprintf` kesishi | `gcc -Wall -Wextra -g 06_satrlar.c -o satrlar && ./satrlar` |
| [7](../07-korsatkichlar.md) | [07_korsatkichlar.c](07_korsatkichlar.c) | `&`, `*`, arifmetika, `->`, funksiya ko'rsatkichi, `argv` | `gcc -Wall -Wextra -g 07_korsatkichlar.c -o korsatkich && ./korsatkich` |
| [8](../08-xotira.md) | [08_xotira.c](08_xotira.c) | `malloc`/`realloc`/`free`, o'suvchi massiv | `gcc -Wall -Wextra -g -fsanitize=address 08_xotira.c -o xotira && ./xotira` |
| [9](../09-struct.md) | [09_struct.c](09_struct.c) | Padding, `packed`, `_Static_assert`, `union`, `enum` | `gcc -Wall -Wextra -g 09_struct.c -o struct && ./struct` |
| [10](../10-preprotsessor.md) | [10_makrolar.c](10_makrolar.c) | `#define N`, makro tuzoqlari, `do{}while(0)`, `#ifdef` | `gcc -Wall -Wextra -g 10_makrolar.c -o makrolar && ./makrolar` |
| [11](../11-kop-fayl-make.md) | [11_kop_fayl/](11_kop_fayl/main.c) | `.h`/`.c`, `static`, `extern`, Makefile | `cd 11_kop_fayl && make && ./dastur` |
| [12](../12-standart-kutubxona.md) | [12_stdlib.c](12_stdlib.c) | `printf` formatlari, fayllar, `errno`, `qsort`, `va_list` | `gcc -Wall -Wextra -g 12_stdlib.c -o stdlib && ./stdlib` |
| [13](../13-ub-xavfsizlik.md) | [13_ub.c](13_ub.c) | Toshishni tekshirish UB tufayli yo'qoladi; sanitizer ushlaydi | fayl boshida — 3 xil yig'ish |
| [14](../14-tizim-chaqiruvlari.md) | [14_jarayonlar.c](14_jarayonlar.c) | `fork`/`wait`, `pipe`+`dup2`+`exec`, signal | `gcc -Wall -Wextra -g 14_jarayonlar.c -o jarayonlar && ./jarayonlar` |
| [15](../15-parallellik.md) | [15_oqimlar.c](15_oqimlar.c) | Poyga holati: qulfsiz / mutex / atomic | `gcc -Wall -Wextra -O2 -pthread 15_oqimlar.c -o oqimlar && ./oqimlar` |
| [16](../16-bitlar-apparat.md) | [16_bitlar_apparat.c](16_bitlar_apparat.c) | Endianness, registr maydonlari, bitmap | `gcc -Wall -Wextra -g 16_bitlar_apparat.c -o apparat && ./apparat` |
| [17](../17-assembly.md) | [17_assembly.c](17_assembly.c) | Inline asm: `rdtsc`, `cpuid`, to'g'ridan-to'g'ri `syscall` | `gcc -Wall -Wextra -g 17_assembly.c -o asm && ./asm` |
| [18](../18-yadroga-koprik.md) | [18_libcsiz.c](18_libcsiz.c) | libc'siz dastur: `_start`, xom syscall'lar | fayl boshida (maxsus bayroqlar) |
| [19](../19-terminal-git.md) | [19_terminal.sh](19_terminal.sh) | Terminal va Git buyruqlari xavfsiz vaqtinchalik papkada | `sh 19_terminal.sh` |
| [20](../20-sonlar.md) | [20_sonlar.c](20_sonlar.c) | Ishora kengayishi, kesish, `float` bitlari, `0.1+0.2` | `gcc -Wall -Wextra -g 20_sonlar.c -o sonlar && ./sonlar` |
| [21](../21-kesh.md) | [21_kesh.c](21_kesh.c) | Qator/ustun bo'yicha o'tish, false sharing — vaqtlar | `gcc -Wall -Wextra -O2 -pthread 21_kesh.c -o kesh && ./kesh` |
| [22](../22-boglash.md) | [22_boglash.c](22_boglash.c) | Bo'limlar (`.text`, `.data`, `.bss`, `.rodata`), `nm`, `readelf` | fayl boshida |
| [23](../23-jarayonlar-scheduling.md) | [23_jarayonlar.c](23_jarayonlar.c) | Scheduler: majburiy kontekst almashishlar soni | `gcc -Wall -Wextra -O2 23_jarayonlar.c -o jarayonlar && ./jarayonlar` |
| [24](../24-virtual-xotira.md) | [24_virtual_xotira.c](24_virtual_xotira.c) | `mmap`, demand paging, COW — page fault'lar soni | `gcc -Wall -Wextra -O2 24_virtual_xotira.c -o vx && ./vx` |
| [25](../25-xotira-ajratish.md) | [25_malloc_ichi.c](25_malloc_ichi.c) | glibc `malloc` ichidan: sarlavha, qayta ishlatish, `sbrk`/`mmap` | `gcc -Wall -Wextra -g 25_malloc_ichi.c -o malloc_ichi && ./malloc_ichi` |
| [26](../26-parallellik-chuqur.md) | [26_faylasuflar.c](26_faylasuflar.c) | Ovqatlanayotgan faylasuflar: deadlock va uni tuzatish | fayl boshida |
| [27](../27-fayl-tizimlari.md) | [27_fayllar.c](27_fayllar.c) | inode, `link`/`unlink`, `rename` bilan atomar saqlash, `fsync` | `gcc -Wall -Wextra -g 27_fayllar.c -o fayllar && ./fayllar` |
| [28](../28-algoritmlar.md) | [28_algoritmlar.c](28_algoritmlar.c) | Heapsort, chiziqli va ikkilik qidiruv tezligi | `gcc -Wall -Wextra -O2 28_algoritmlar.c -o algo && ./algo` |
| [29](../29-debug-vositalari.md) | [29_xatoli.c](29_xatoli.c) | **Ataylab xatoli** dastur: gdb, ASan, valgrind bilan toping | fayl boshida |
| [30](../30-yadro-arxitekturasi.md) | [30_yadro_moduli/](30_yadro_moduli/salom_modul.c) | Haqiqiy Linux yadro moduli (`insmod`, `dmesg`) | faqat virtual mashinada, fayl boshida |

31-bob (lug'at) uchun misol yo'q — u ma'lumotnoma.

## Darslikdagi parchani o'zingiz qanday sinaysiz

Bobda shunday parcha ko'rdingiz deylik:

```c
#define N 10
int massiv[N];
```

Bu — to'liq dastur emas, faqat **g'oya**. Uni sinash uchun doim bitta shablonga qo'yasiz:

```c
#include <stdio.h>      /* printf uchun */

/* <-- parchadagi #define, struct, funksiyalar shu yerga (main dan TASHQARIGA) */
#define N 10

int main(void)
{
    /* <-- parchadagi o'zgaruvchilar va buyruqlar shu yerga */
    int massiv[N];
    for (int i = 0; i < N; i++)
        massiv[i] = i * i;

    /* <-- natijani ko'rish uchun printf qo'shing */
    for (int i = 0; i < N; i++)
        printf("massiv[%d] = %d\n", i, massiv[i]);
    return 0;
}
```

Qoida oddiy:

- `#include`, `#define`, `struct`, funksiyalar — **`main` dan tashqarida**, fayl boshida;
- o'zgaruvchilar va buyruqlar — **`main` ichida**;
- natijani ko'rish uchun — **`printf`** (Python'dagi `print`);
- kompilyator "implicit declaration of function 'strlen'" desa — kerakli `#include` yetishmayapti
  (`strlen` → `<string.h>`, `malloc` → `<stdlib.h>`; qaysi biri kerakligini `man 3 strlen` aytadi).

Keyin: `gcc -Wall -Wextra -g sinov.c -o sinov && ./sinov`.

**Eng yaxshi odat:** har bir bob uchun alohida `sinov.c` ochib, bobdagi har bir parchani shu shablonga
qo'yib ishga tushiring, keyin bir narsani o'zgartirib "nima bo'larkin?" deb taxmin qiling va tekshiring.
Taxminingiz noto'g'ri chiqqan joy — aynan siz hali tushunmagan joy.
