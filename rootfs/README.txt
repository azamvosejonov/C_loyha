MyOS - noldan yozilgan x86-64 yadrosi
=====================================

Bu fayl initrd.tar arxivida keldi: yadro boot paytida uni xotiradagi
fayl tizimiga (tmpfs) ochdi. Fayl tizimi daraxti:

  /bin      dasturlar (ls /bin)
  /dev      qurilmalar: console, null, zero, random, kmsg, disklar
  /etc      sozlamalar (motd - kirishdagi xabar)
  /home     sizning fayllaringiz
  /tmp      vaqtinchalik fayllar
  /mnt      boshqa fayl tizimlarini ulash uchun

Sinab ko'ring:

  ls -l /bin                      fayllar ro'yxati
  echo salom > /home/a.txt        faylga yozish
  cat /home/a.txt                 o'qish
  ls /bin | wc -l                 pipe: nechta dastur bor?
  ls /bin | grep s | head -3      uch bosqichli pipe
  cd /home ; pwd                  papkani almashtirish
  seq 1000 | tail -3              katta ma'lumot pipe orqali
  ps ; free ; uname ; date        tizim haqida
  fstest ; forktest               testlar
  crash null                      himoya: faqat shu dastur o'ladi

DIQQAT: hozircha hamma narsa XOTIRADA (tmpfs). Qayta yuklashda yo'qoladi.
Disk (ext2) keyingi qadamda qo'shiladi.
