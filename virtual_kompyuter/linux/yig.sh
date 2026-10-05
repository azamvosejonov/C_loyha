#!/bin/sh
# yig.sh — Linux yadrosini virtual kompyuterimiz uchun yuklab olish va yig'ish (bir marta, ~5-15 daqiqa).
#
# Kerak: clang, ld.lld, make, flex, bison, bc, rsync, curl (Ubuntu: sudo apt install clang lld make flex bison bc rsync curl)
# Natija: build/Image (Linux yadrosi, ichida initramfs: /init shell va /bin/salom)
set -e
VERSIYA=6.6.50
BU=$(cd "$(dirname "$0")" && pwd)
BUILD=$(cd "$BU/.." && pwd)/build
mkdir -p "$BUILD"
cd "$BUILD"

if [ ! -d linux-$VERSIYA ]; then
    echo ">>> Linux $VERSIYA manba kodini yuklab olyapman (~140 MB)"
    curl -L -o linux.tar.xz https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-$VERSIYA.tar.xz
    tar xf linux.tar.xz
    rm linux.tar.xz
fi
L=$BUILD/linux-$VERSIYA

echo ">>> foydalanuvchi dasturlari uchun yadro sarlavhalari (headers_install)"
make -C "$L" ARCH=riscv LLVM=1 headers_install INSTALL_HDR_PATH="$BUILD/hdr" > /dev/null

echo ">>> /init (shell) va /bin/salom — nolibc bilan (libc'siz, to'g'ridan-to'g'ri tizim chaqiruvlari)"
R=$(clang -print-resource-dir)
for p in init salom; do
    clang --target=riscv32-unknown-linux-gnu -march=rv32imac -mabi=ilp32 -Os -fno-builtin -nostdlib -nostdinc \
        -isystem "$R/include" -static -fno-stack-protector -fno-asynchronous-unwind-tables -Wno-unknown-attributes \
        -include "$L/tools/include/nolibc/nolibc.h" -I "$L/tools/include/nolibc" -I "$BUILD/hdr/include" \
        "$BU/$p.c" -o "$BUILD/$p" -fuse-ld=lld
done

echo ">>> sozlash: tinyconfig + vk.config"
cp "$BU/initramfs.list" "$L/usr/initramfs.list"
sed -i "s#\.\./\.\./build/#$BUILD/#" "$L/usr/initramfs.list"
make -C "$L" ARCH=riscv LLVM=1 tinyconfig > /dev/null
sed "s#^CONFIG_INITRAMFS_SOURCE=.*#CONFIG_INITRAMFS_SOURCE=\"usr/initramfs.list\"#" "$BU/vk.config" > "$BUILD/vk.config"
(cd "$L" && ./scripts/kconfig/merge_config.sh -m .config "$BUILD/vk.config" > /dev/null)
make -C "$L" ARCH=riscv LLVM=1 olddefconfig > /dev/null

echo ">>> yig'ish (bir necha daqiqa)"
make -C "$L" ARCH=riscv LLVM=1 -j"$(nproc)" Image
cp "$L/arch/riscv/boot/Image" "$BUILD/Image"
echo ">>> TAYYOR: $BUILD/Image   (ishga tushirish: make linux-ishga)"
