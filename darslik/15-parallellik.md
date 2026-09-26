# 15-bob. Parallellik: oqimlar, poyga holati, qulflar, atomiklar

> **Bu bobdan keyin:** poyga holati (race condition) nima ekanini, mutex va spinlock farqini,
> deadlock'ni, atomik amallarni va xotira tartibi (memory ordering) g'oyasini bilasiz. Bu — yadro
> dasturlashning **eng qiyin** mavzusi: MyOS 64 tagacha yadroda bir vaqtda ishlaydi va har bir umumiy
> ma'lumot himoyalangan bo'lishi kerak. Mashqlar: 29, 34.

## 15.1. Oqim (thread) nima

Jarayon — alohida xotira maydoni. **Oqim** — bitta jarayon ichidagi alohida "bajaruvchi": o'z steki va
registrlari bor, lekin **xotirani boshqa oqimlar bilan bo'lishadi**. Ko'p yadroli CPU'da oqimlar
**haqiqatan bir vaqtda** ishlaydi.

```c
#include <pthread.h>

static void *ish(void *arg)
{
    int n = *(int *)arg;
    printf("oqim %d ishlayapti\n", n);
    return NULL;
}

int main(void)
{
    pthread_t t[4];
    int id[4];
    for (int i = 0; i < 4; i++) {
        id[i] = i;                          /* har biriga O'Z argumenti */
        pthread_create(&t[i], NULL, ish, &id[i]);
    }
    for (int i = 0; i < 4; i++)
        pthread_join(t[i], NULL);           /* tugashini kutish */
    return 0;
}
```

`gcc -pthread fayl.c`. Chiqish tartibi har safar boshqacha bo'lishi mumkin — oqimlar qachon ishlashini
scheduler hal qiladi.

**Tuzoq:** hamma oqimga bitta `&i` berish — `i` sikl davomida o'zgaradi, oqimlar noto'g'ri qiymatni ko'radi.

## 15.2. Poyga holati (race condition)

```c
long hisob = 0;

void *oshir(void *arg)
{
    for (int i = 0; i < 1000000; i++)
        hisob++;
    return NULL;
}
/* 4 ta oqim -> kutilgan 4000000, olingan: 1834521, 2210987, ... har safar boshqacha */
```

`hisob++` — **uchta** amal: xotiradan registrga o'qish, oshirish, xotiraga yozish.

```text
oqim A: o'qidi 5
oqim B: o'qidi 5
oqim A: yozdi 6
oqim B: yozdi 6        <- bitta oshirish YO'QOLDI
```

Poyga holatlari — eng yomon xatolar: ular **kamdan-kam** va **tasodifiy** chiqadi, debugger bilan
kuzatganda yo'qolib qoladi (vaqt o'zgaradi). Yadroda oqibati — buzilgan ro'yxat, ikki marta berilgan
sahifa, qulash — bir necha kunda bir marta.

**Kritik seksiya** — umumiy ma'lumotga murojaat qiladigan kod qismi. Bir vaqtda faqat **bitta** oqim
unda bo'lishi kerak (o'zaro istisno — mutual exclusion).

## 15.3. Mutex

```c
static pthread_mutex_t qulf = PTHREAD_MUTEX_INITIALIZER;

void *oshir(void *arg)
{
    for (int i = 0; i < 1000000; i++) {
        pthread_mutex_lock(&qulf);      /* bo'sh bo'lguncha kutadi (uxlaydi) */
        hisob++;                        /* kritik seksiya */
        pthread_mutex_unlock(&qulf);
    }
    return NULL;
}
```

Qoidalar:
1. Har bir umumiy ma'lumotning **aniq bitta** himoyachi qulfi bo'lsin — va buni izohda yozing
   (`/* disks_lock himoya qiladi */`). MyOS'da har bir global ro'yxat yonida shunday izoh bor.
2. Kritik seksiya **qisqa** bo'lsin — qulf ichida uxlamang, katta ish qilmang.
3. Har bir `lock` ga aniq bitta `unlock` — xato yo'llarida ham (`goto` tozalash, 4-bob).

## 15.4. Deadlock (o'zaro qotish)

```text
oqim A: lock(X) ...  lock(Y) <- kutadi (Y B da)
oqim B: lock(Y) ...  lock(X) <- kutadi (X A da)       -> ikkalasi ham abadiy kutadi
```

Oldini olish — **qulflar tartibi**: hamma kod qulflarni bir xil tartibda oladi (avval X, keyin Y).
Linux'da `lockdep` bu tartibni avtomatik tekshiradi. Boshqa deadlock turi: o'zingiz ushlab turgan qulfni
qayta olish (rekursiv), yoki qulf ushlagan holda uxlab qolish.

## 15.5. Atomik amallar

Oddiy hisoblagich uchun mutex og'ir. CPU'da **atomik** buyruqlar bor — bo'linmas o'qish-o'zgartirish-yozish:

```c
#include <stdatomic.h>
atomic_long hisob = 0;
atomic_fetch_add(&hisob, 1);            /* bitta bo'linmas amal (x86: lock xadd) */

/* GCC builtin'lari (MyOS shularni ishlatadi): */
__atomic_add_fetch(&n, 1, __ATOMIC_RELAXED);
__atomic_load_n(&ticks, __ATOMIC_ACQUIRE);
__atomic_compare_exchange_n(&x, &kutilgan, yangi, 0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
```

**Compare-and-swap (CAS)** — barcha qulflarning asosi: "agar x hali ham `kutilgan` ga teng bo'lsa,
uni `yangi` qil — atomik ravishda; bo'lmasa, menga hozirgi qiymatni ayt". x86'da `lock cmpxchg`.

## 15.6. Spinlock — yadroning asosiy qulfi

Mutex kutganda **uxlaydi** (scheduler boshqa ishni beradi). Yadroda esa ba'zi joylarda uxlab bo'lmaydi
(uzilish ishlovchisi ichida, scheduler'ning o'zida) — u yerda **spinlock**: bo'shaguncha aylanib turadi.

```c
typedef struct { volatile int band; } spinlock_t;

void spin_lock(spinlock_t *l)
{
    while (__atomic_exchange_n(&l->band, 1, __ATOMIC_ACQUIRE))  /* 1 yozib, eskisini olish */
        while (__atomic_load_n(&l->band, __ATOMIC_RELAXED))     /* band - faqat o'qib kutish */
            __builtin_ia32_pause();                             /* CPU'ga "aylanyapman" ishorasi */
}

void spin_unlock(spinlock_t *l)
{
    __atomic_store_n(&l->band, 0, __ATOMIC_RELEASE);
}
```

| | Mutex | Spinlock |
|---|---|---|
| Kutganda | uxlaydi | aylanadi (CPU yonadi) |
| Qachon | uzoq kutish, uxlash mumkin bo'lgan joy | juda qisqa kritik seksiya, uxlab bo'lmaydigan joy |
| Yadroda | `kernel/lib/mutex.c` | `kernel/lib/spinlock.c` (spinlock lab'i) |

Yadroda spinlock ushlanganda **uzilishlar ham o'chiriladi** — aks holda uzilish ishlovchisi o'sha qulfni
olishga harakat qilib, o'z CPU'sida abadiy aylanadi (deadlock). MyOS'ning `spin_lock` i shuni qiladi.

## 15.7. Xotira tartibi (memory ordering) — g'oya

Zamonaviy CPU va kompilyator xotira amallarini **tartibini o'zgartirishi** mumkin (tezlik uchun):

```c
/* oqim A */                     /* oqim B */
data = 42;                       while (!tayyor) ;
tayyor = 1;                      printf("%d", data);    /* 0 chiqishi mumkin! */
```

Kompilyator yoki CPU `tayyor = 1` ni `data = 42` dan **oldin** bajarishi mumkin — bitta oqim nuqtai
nazaridan farq yo'q. Yechim — **to'siqlar** (barriers) yoki acquire/release semantikasi:

- `__ATOMIC_RELEASE` bilan yozish — "bundan oldingi hamma yozuvlar undan keyin ko'rinmaydi";
- `__ATOMIC_ACQUIRE` bilan o'qish — "bundan keyingi o'qishlar undan oldin bajarilmaydi".

Spinlock'dagi `ACQUIRE`/`RELEASE` aynan shu: qulf ichidagi amallar qulfdan "tashqariga sizib" chiqmaydi.
Qulflarni to'g'ri ishlatsangiz, ular tartibni o'zi ta'minlaydi. Qulfsiz (lock-free) kod yozish — mutaxassis
darajasi; avval qulflarni mukammal o'rganing.

`volatile` **qulf emas** va tartibni kafolatlamaydi — u faqat "kompilyator bu o'qishni o'chirmasin"
degani (16-bob). Ko'p oqimli sinxronlash uchun atomiklar kerak.

## 15.8. Yadroda parallellik manbalari

Bitta yadroli kompyuterda ham yadroda "parallellik" bor:
1. **Uzilishlar** — istalgan ikki buyruq orasida uzilish ishlovchisi ishlashi mumkin.
2. **Preemption** — taymer jarayonni to'xtatib, boshqasini ishga tushiradi.
3. **Ko'p yadro (SMP)** — haqiqiy parallellik (MyOS: `kernel/arch/smp.c`, `docs/10-smp.md`).

## 15.9. Savol-javob

**Nega Python'da bu muammolar kamroq?**
GIL (Global Interpreter Lock) — bir vaqtda faqat bitta oqim Python kodini bajaradi. Bu poygalarning
bir qismini yashiradi (lekin hammasini emas!), buning evaziga CPU ishida parallellik yo'q.

**Poyga holatini qanday topaman?**
`gcc -fsanitize=thread` (ThreadSanitizer) — ish vaqtida umumiy xotiraga himoyasiz murojaatni ushlaydi.
Yadroda — KCSAN, lockdep, va eng muhimi — ehtiyotkor kod o'qish.

**Qachon atomik, qachon qulf?**
Bitta o'zgaruvchi (hisoblagich, bayroq) — atomik. Bir nechta bog'liq o'zgaruvchi (ro'yxat + uning
uzunligi) — qulf.

## 15.10. O'zingizni tekshiring

1. Nega `hisob++` ko'p oqimda xato beradi?
2. Mutex va spinlock farqi? Yadroda qaysi birini uzilish ishlovchisida ishlatish mumkin?
3. Deadlock'ni qanday oldini olish mumkin?
4. `volatile` ko'p oqimli hisoblagichni himoya qiladimi?
5. `pthread_create` ga sikl o'zgaruvchisining manzilini berish nega xato?

<details><summary>Javoblar</summary>

1. U uch amaldan iborat (o'qish, oshirish, yozish) va oqimlar ular orasida aralashadi.
2. Mutex uxlaydi, spinlock aylanadi. Uzilish ishlovchisida faqat spinlock (uxlab bo'lmaydi).
3. Qulflarni doim bir xil tartibda olish; qulf ushlab uxlamaslik; qayta kirishdan qochish.
4. Yo'q — atomiklik va tartibni ta'minlamaydi.
5. O'zgaruvchi keyingi aylanishda o'zgaradi; oqimlar bir xil (yoki noto'g'ri) qiymatni ko'radi.
</details>

## 15.11. Mashqlar

- **29** — oqimlar va mutex (poyga holatini o'z ko'zingiz bilan ko'rish).
- **34** — o'z spinlock'ingiz (atomiklar bilan).
- MyOS: `kernel/lib/spinlock.c` ni o'qing, keyin spinlock lab'ini bajaring (`tools/lab.py boshla spinlock`).

Keyingi bob: [16-bob. Bitlar va apparat](16-bitlar-apparat.md)
