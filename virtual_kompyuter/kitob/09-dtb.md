# 9-bob. Qurilmalar daraxti (DTB)

> **Bu bobda nima o'rganasiz:** yadro "qaysi kompyuterda ishlayapman?" savoliga qanday javob oladi; DTS (matn)
> va DTB (binar) formatlari; big-endian kodlash; tuzilma va satrlar bloklari; tekislash; `compatible` —
> drayver tanlash kaliti; `phandle` — tugunlar orasidagi "ko'rsatkich"; DTB'ni `dtc` bilan tekshirish.
> **Oldindan nima kerak:** 1-bob (1.7 — endianness), 8-bob.
> **Mashqlar:** D1, D2, D3 ([emu/dtb.c](../emu/dtb.c)).   **Vaqt:** 3 soat.

## Bu bob nima haqida?

Linux minglab turli kompyuterlarda ishlaydi. Bizning "virtual kompyuter"imiz haqida u hech narsa bilmaydi:
RAM qayerda va qancha? UART qaysi manzilda, qaysi uzilish bilan? Protsessor qaysi kengaytmalarni biladi?
x86'da bu ma'lumotni BIOS/ACPI beradi. RISC-V va ARM'da — **qurilmalar daraxti** (device tree): kompyuterning
"pasporti", yuklanish paytida yadroga `a1` registrida beriladi.

**Hayotdan misol: yangi xodimga ofis xaritasi.** Birinchi kuni xodimga varaq beriladi: "Oshxona — 2-qavat,
printer — 305-xona, IT bo'limi — ichki raqam 1010". Xodim ofisni o'zi qidirib yurmaydi. DTB — yadro uchun
shunday varaq.

## 9.1. DTS — matn ko'rinishi

Bizning emulyator yaratgan daraxt (`-D` bilan faylga yozib, `dtc` bilan matnga aylantirilgan):

```console
$ ./build/vk -S -m 64 -D build/vk.dtb -n 1 testlar/emu/build/alu.elf
$ dtc -I dtb -O dts build/vk.dtb
```

```dts
/dts-v1/;

/ {
	#address-cells = <0x01>;
	#size-cells = <0x01>;
	compatible = "riscv-virtio";
	model = "virtual-kompyuter,rv32";

	chosen {
		stdout-path = "/soc/serial@10000000";
	};

	memory@80000000 {
		device_type = "memory";
		reg = <0x80000000 0x4000000>;
	};

	cpus {
		#address-cells = <0x01>;
		#size-cells = <0x00>;
		timebase-frequency = <0x989680>;

		cpu@0 {
			device_type = "cpu";
			reg = <0x00>;
			status = "okay";
			compatible = "riscv";
			riscv,isa = "rv32imac_zicsr_zifencei";
			riscv,isa-base = "rv32i";
			riscv,isa-extensions = "i\0m\0a\0c\0zicsr\0zifencei\0zicntr";
			mmu-type = "riscv,sv32";

			interrupt-controller {
				#interrupt-cells = <0x01>;
				interrupt-controller;
				compatible = "riscv,cpu-intc";
				phandle = <0x01>;
			};
		};
	};

	soc {
		...
		plic@c000000 {
			compatible = "sifive,plic-1.0.0\0riscv,plic0";
			reg = <0xc000000 0x400000>;
			interrupts-extended = <0x01 0x0b 0x01 0x09>;
			riscv,ndev = <0x1f>;
			phandle = <0x02>;
			...
		};

		serial@10000000 {
			compatible = "ns16550a";
			reg = <0x10000000 0x100>;
			interrupt-parent = <0x02>;
			interrupts = <0x0a>;
			...
		};
	};
};
```

O'qish kalitlari:

- **Tugun** (`cpu@0 { ... }`) — qurilma; `@` dan keyin — asosiy manzil.
- **Xossa** (`reg = <...>`) — nom va qiymat. Qiymat: 32 bitli sonlar `<...>`, satr `"..."`, satrlar ro'yxati
  `"a\0b"`, yoki bo'sh (`interrupt-controller;` — "bor" degan belgi).
- **`compatible`** — eng muhim xossa: drayver tanlash kaliti. Linux `"ns16550a"` ni ko'rib 8250 drayverini,
  `"sifive,plic-1.0.0"` ni ko'rib PLIC drayverini ulaydi. Bir nechta qiymat — "eng aniqdan umumiyga".
- **`reg = <manzil hajm>`** — nechta son ekanini ota tugundagi `#address-cells` / `#size-cells` aytadi.
  `cpus` da `#size-cells = 0` — protsessorning "hajmi" yo'q, `reg` — faqat raqami.
- **`phandle`** — tugunning raqami; boshqa tugunlar unga shu raqam bilan ishora qiladi. `interrupt-parent =
  <0x02>` — "mening uzilishlarim 2-raqamli tugunga (PLIC) boradi". `interrupts-extended = <0x01 0x0b 0x01 0x09>`
  — "PLIC'ning chiqishlari: 1-tugunning (cpu-intc) 11-kirishiga (MEI) va 9-kirishiga (SEI)". Bu — 6-bobdagi
  uzilish raqamlari!

## 9.2. DTB — binar format

Yadro matnni emas, **binar** shaklni oladi (FDT — flattened device tree). Hamma sonlar **big-endian**:

```text
+------------------+  0
| sarlavha (40 B)  |  sehrli son 0xD00DFEED, o'lchamlar, bloklar siljishi
+------------------+
| xotira zaxirasi  |  (bizda bo'sh: 16 ta nol bayt — ro'yxat oxiri)
+------------------+
| TUZILMA bloki    |  tokenlar: BEGIN_NODE(1) nom | PROP(3) uzunlik nom_siljishi qiymat | END_NODE(2) | END(9)
+------------------+
| SATRLAR bloki    |  xossa NOMLARI ("reg\0compatible\0..."), har nom bir marta
+------------------+
```

Haqiqiy baytlar (`od -A x -t x1z`):

```text
000000 d0 0d fe ed 00 00 05 a5 00 00 00 38 00 00 04 8c   sehr | jami 0x5a5 | tuzilma @0x38 | satrlar @0x48c
000010 00 00 00 28 00 00 00 11 00 00 00 10 00 00 00 00   zaxira @0x28 | versiya 17 | mos 16 | boot cpu 0
000020 00 00 01 19 00 00 04 54 00 00 00 00 00 00 00 00   satrlar hajmi 0x119 | tuzilma hajmi 0x454 | (zaxira...)
000030 00 00 00 00 00 00 00 00 00 00 00 01 00 00 00 00   ...zaxira | BEGIN_NODE | nom "" (+ 3 bayt to'ldirish)
000040 00 00 00 03 00 00 00 04 00 00 00 00 00 00 00 01   PROP | uzunlik 4 | nom @0 ("#address-cells") | <1>
000050 00 00 00 03 00 00 00 04 00 00 00 0f 00 00 00 01   PROP | 4 | nom @0x0f ("#size-cells") | <1>
000060 00 00 00 03 00 00 00 0d 00 00 00 1b 72 69 73 63   PROP | 13 | nom @0x1b ("compatible") | "risc
000070 76 2d 76 69 72 74 69 6f 00 00 00 00               v-virtio\0" + 3 bayt to'ldirish
```

Uchta qoidaga e'tibor bering — ular sizning uchta mashqingiz:

1. **D1 — big-endian.** `1` soni `00 00 00 01` bo'lib yoziladi, `01 00 00 00` emas. Bizning protsessor
   little-endian bo'lsa ham — format shunday kelishilgan (tarmoq protokollaridagi kabi).
2. **D2 — 4 baytga tekislash.** `"riscv-virtio\0"` — 13 bayt; keyingi token 4 ga karrali joydan boshlanishi
   kerak — 3 ta nol bayt qo'shiladi. Ildiz tugun nomi `""` (1 bayt `\0`) — yana 3 bayt.
3. **D3 — nomlar bir marta.** PROP xossa **nomini** emas, uning satrlar blokidagi **siljishini** saqlaydi.
   `"compatible"` daraxtda 7 marta uchraydi, lekin satrlar blokida **bir marta** — hamma PROP'lar bitta
   siljishga (0x1b) ishora qiladi. Bu — "string interning", kompilyatorlar va ma'lumotlar bazalarida keng
   tarqalgan usul.

## 9.3. Quruvchi — tayyor qism

[emu/dtb.c](../emu/dtb.c) daraxtni "yuqoridan pastga" yozadi — xuddi XML yoki JSON yozuvchi kabi:

```c
static void xossa(struct fdt *f, const char *nom, const void *qiymat, uint32_t uzunlik)
{
    uint8_t b[8];
    token(f, FDT_PROP);
    be32_yoz(b, uzunlik);
    be32_yoz(b + 4, satr_siljishi(f, nom));
    baytlar(f, b, 8);
    if (uzunlik)
        baytlar(f, qiymat, uzunlik);
}
```

Va daraxtning o'zi:

```c
tugun(&f, "cpu@0");
xossa_matn(&f, "device_type", "cpu");
xossa_son(&f, "reg", 0);
...
XOSSA_MATNLAR(&f, "riscv,isa-extensions", "i\0m\0a\0c\0zicsr\0zifencei\0zicntr");
xossa_matn(&f, "mmu-type", "riscv,sv32");
```

Sizning uchta funksiyangiz — eng past daraja: `be32_yoz` (D1), `baytlar` dagi tekislash (D2) va
`satr_siljishi` (D3). Ular ishlamasa — butun daraxt buzuq.

### `XOSSA_MATNLAR` va `sizeof` — haqiqiy xato hikoyasi

Satrlar ro'yxati ichida `\0` bor, shuning uchun `strlen` ishlamaydi (birinchi `\0` da to'xtaydi). Birinchi
versiyada uzunlik **qo'lda** yozilgan edi — `34`. Haqiqiy uzunlik esa `30` edi. Natija: DTB'ga satrdan
**keyingi** 4 bayt ham ko'chirildi — xotirada tasodifan yonida turgan boshqa ma'lumot. Hech narsa qulamadi,
xato faqat `dtc` chiqishida ko'rindi: ro'yxat oxirida g'alati "kengaytma" paydo bo'lgan edi. (C nuqtai nazaridan
bu — massiv chegarasidan tashqarini o'qish, ya'ni UB.) Yechim — uzunlikni kompilyator hisoblasin:

```c
#define XOSSA_MATNLAR(f, nom, literal) xossa(f, nom, literal, sizeof(literal))
```

Satr literali uchun `sizeof` — oxirgi `\0` bilan birga hajm. Saboq: **qo'lda yozilgan "sehrli son" — kelajakdagi
xato**.

### Linux nima o'qiydi — yana bir hikoya

Daraxtda `riscv,isa = "rv32imac..."` bor edi. Lekin Linux 6.6 boot log'ida:

```text
riscv: base ISA extensions
```

— **bo'sh**! Linux kodini ochamiz (`arch/riscv/kernel/cpufeature.c`, `riscv_fill_hwcap`):

```c
ret = riscv_fill_hwcap_from_ext_list(isa2hwcap);       /* yangi format: riscv,isa-extensions */
if (ret && riscv_isa_fallback) {
    pr_info("Falling back to deprecated \"riscv,isa\"\n");
    riscv_fill_hwcap_from_isa_string(isa2hwcap);       /* eski format: faqat ruxsat bo'lsa */
}
```

`riscv_isa_fallback` — `CONFIG_RISCV_ISA_FALLBACK` sozlamasiga bog'liq. Biz Linux'ni eng kichik sozlamadan
(`tinyconfig`, 11-bob) yig'amiz va u bu sozlamani o'chirib qo'ygan:

```console
$ grep ISA_FALLBACK build/linux-6.6.50/.config
# CONFIG_RISCV_ISA_FALLBACK is not set
```

Ya'ni eski `riscv,isa` umuman o'qilmagan. Ikkala formatni (`riscv,isa-base` + `riscv,isa-extensions`) qo'shgach:

```text
[    0.000000] riscv: base ISA extensions acim
```

`acim` — A, C, I, M. Saboq: spetsifikatsiya o'zgaradi; "nega ishlamayapti?" savoliga eng ishonchli javob —
**haqiqiy iste'molchining manba kodi**. Linux manbasi `build/linux-6.6.50/` da — `grep -rn` bilan qidiring.

## 9.4. D1–D3 mashqlari

Izohlardagi ko'rsatmalarga amal qiling. Eslatmalar:

- **D1:** har bayt — `(uint8_t)(q >> 24)`, `(uint8_t)(q >> 16)` ... Siljitishni `uint32_t` ustida bajaring (1.8).
- **D2:** "4 ga karrali" ⇔ past 2 bit nol ⇔ `(n & 3) == 0`. Tsikl yoki formula — ikkalasi ham bo'ladi.
- **D3:** satrlar blokini boshidan **har satr bo'yicha** yuring: `i += strlen(f->satrlar + i) + 1`. Topilmasa
  — oxiriga `\0` bilan qo'shing va eski uzunlikni (yangi satrning siljishini) qaytaring.

**Tekshirish:** test sehrli sonni, tekislashni, `"compatible"` bir marta ekanini va oxirida butun DTB'ni
**bayt-bayt** kutilgan fayl ([testlar/birlik/kutilgan.dtb](../testlar/birlik/kutilgan.dtb)) bilan solishtiradi:

```console
$ make test 2>&1 | grep -B3 "D1-D3"
        farq    birinchi farq qiluvchi bayt siljishi (kutilgan = hajm: farq yo'q) -> olindi 0x00000058, kutilgan 0x000005a5
  [XATO] D1-D3 qurilmalar daraxti (dtb.c): 5/6
```

`0x58` — birinchi farq shu baytda. `od -A x -t x1z -j 0x50 -N 32` bilan ikkala faylni solishtiring
(o'zingiznikini `-D` bilan oling).

## Savol-javob

**Savol:** DTB'ni nega emulyator yaratadi, tayyor fayl emas?
**Javob:** Chunki u o'zgaruvchan: `-m 128` bilan RAM hajmi o'zgaradi va `memory` tugunining `reg` i ham
o'zgarishi kerak. QEMU ham DTB'ni har ishga tushishda yaratadi. Haqiqiy platalarda DTB odatda firmware (U-Boot)
ichida yoki diskda turadi.

**Savol:** DTB qayerga joylanadi va Linux uni qanday topadi?
**Javob:** Biz uni RAM'ning oxirgi 64 KB iga qo'yamiz va manzilini `a1` da beramiz (firmware `mret` dan oldin
`a1` ni saqlaydi — 10-bob). Linux boot qoidasi (`Documentation/arch/riscv/boot.rst`): `a0` — hart raqami,
`a1` — DTB manzili. Linux uni darhol o'z xotirasiga ko'chiradi — keyin bu joy oddiy RAM bo'ladi.
