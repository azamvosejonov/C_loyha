/* =============================================================================
 *  30_yadro_moduli/salom_modul.c - Linux yadrosi uchun birinchi modul     (30-bob)
 * =============================================================================
 *  DIQQAT: modul YADRO ichida ishlaydi - undagi xato butun tizimni qulatishi mumkin.
 *  Birinchi tajribalarni VIRTUAL MASHINADA qiling (VirtualBox, QEMU yoki WSL2 emas -
 *  WSL2 yadrosiga modul yuklash qiyin).
 *
 *  Kerak: sudo apt install build-essential linux-headers-$(uname -r)
 *
 *  Yig'ish va sinash:
 *      make
 *      sudo insmod salom_modul.ko
 *      sudo dmesg | tail -3            - "salom: yadroga xush kelibsiz! jiffies=..."
 *      cat /sys/module/salom_modul/parameters/ism
 *      sudo rmmod salom_modul
 *      sudo dmesg | tail -1            - "salom: xayr, Ali!"
 *  Parametr bilan:  sudo insmod salom_modul.ko ism=Vali
 *
 *  Sinab ko'ring:
 *      1) `modinfo salom_modul.ko` - litsenziya, muallif, parametrlar.
 *      2) salom_init() dan `return -ENOMEM;` qaytaring (<linux/errno.h>). insmod nima deydi?
 *         Modul yuklandimi (lsmod | grep salom)?
 *      3) pr_info ni pr_err ga almashtiring va `dmesg --level=err` bilan ko'ring.
 *      4) `ism` parametrini 0644 bilan e'lon qilib, yuklangandan keyin
 *         `echo Vali | sudo tee /sys/module/salom_modul/parameters/ism` - rmmod'da kim bilan xayrlashadi?
 * ============================================================================= */
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/module.h>

static char *ism = "Ali";
module_param(ism, charp, 0444);                 /* insmod ... ism=Vali */
MODULE_PARM_DESC(ism, "Kimga salom berish");

static int __init salom_init(void)
{
    pr_info("salom: yadroga xush kelibsiz! jiffies=%lu, salom %s\n", jiffies, ism);
    return 0;                                   /* 0 - muvaffaqiyat; manfiy - xato kodi, modul yuklanmaydi */
}

static void __exit salom_exit(void)
{
    pr_info("salom: xayr, %s!\n", ism);
}

module_init(salom_init);
module_exit(salom_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Birinchi o'quv moduli");
